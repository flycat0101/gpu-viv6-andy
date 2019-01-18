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


/*******************************************************************************
**  Profiler for OpenGL ES 1.1 driver.
*/

#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE    glvZONE_TRACE

#if VIVANTE_PROFILER

gctINT _glffProfileMode = -1;

void
_glffProfilerInitialize(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR *env = gcvNULL;
    glsPROFILER * profiler = &Context->profiler;

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
        gcoPROFILER_Disable();
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
    gcmONERROR(gcoPROFILER_Construct(&Context->profilerObj));

    profiler->useGlfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_GLFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        profiler->useGlfinish = gcvTRUE;
        Context->profilerObj->bufferCount = NumOfPerDrawBuf;
    }

    profiler->perDrawMode = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_PERDRAW_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        profiler->perDrawMode =
        Context->profilerObj->perDrawMode = gcvTRUE;
        Context->profilerObj->bufferCount = NumOfPerDrawBuf;
    }

    Context->profilerObj->profilerClient = gcvCLIENT_OPENGLES11;

    if (gcoPROFILER_Initialize(Context->profilerObj) != gcvSTATUS_OK)
    {
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    }
    profiler->enable = gcvTRUE;
    profiler->curFrameNumber = 0;
    profiler->frameNumber = 0;
    profiler->frameBegun = gcvFALSE;
    profiler->finishNumber = 0;
    profiler->drawCount = 0;
    profiler->writeDrawable = gcvFALSE;

    gcoOS_GetTime(&profiler->frameStartTimeusec);
    _glffProfilerWrite(Context, GL1_PROFILER_WRITE_HEADER);

OnError:
    /* Return the error. */
    gcmFOOTER_NO();
}

void
_glffProfilerDestroy(
    glsCONTEXT_PTR Context
    )
{
    glsPROFILER * profiler = &Context->profiler;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (profiler->enable)
    {
        profiler->enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(Context->profilerObj));
    }

    /* Success. */
    gcmFOOTER_NO();
}

gceSTATUS
_glffProfilerWrite(
    glsCONTEXT_PTR Context,
    GLuint Enum
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT rev;
    gcoPROFILER Profiler = Context->profilerObj;
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
#if gcdNULL_DRIVER
        {
            char* infoDiverMode = "NULL Driver";
            gcmWRITE_CONST(VPC_INFODRIVERMODE);
            gcmWRITE_STRING(infoDiverMode);
        }
#endif
        break;

    case GL1_PROFILER_WRITE_FRAME_BEGIN:

        if (!Context->profiler.writeDrawable && Context->draw)
        {
            gctUINT width, height;
            gctUINT offset = 0;
            gctCHAR  infoScreen[255] = { '\0' };
            gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
            gcmONERROR(gcoSURF_GetSize(Context->draw, &width, &height, gcvNULL));
            gcmONERROR(gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),
                &offset, "%d x %d", width, height));
            gcmWRITE_CONST(VPC_INFOSCREENSIZE);
            gcmWRITE_STRING(infoScreen);

            gcmWRITE_CONST(VPG_END);
            Context->profiler.writeDrawable = gcvTRUE;
        }

        if (!Context->profiler.frameBegun && Context->profiler.need_dump)
        {
            gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);
            Context->profiler.frameBegun = gcvTRUE;
        }
        break;

    case GL1_PROFILER_WRITE_FRAME_END:
        /*write time*/
        if (Context->profiler.need_dump)
        {
            gcmWRITE_CONST(VPG_TIME);
            gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32)(Context->profiler.frameEndTimeusec - Context->profiler.frameStartTimeusec));
            gcmWRITE_COUNTER(VPC_CPUTIME, (gctINT32)Context->profiler.totalDriverTime);
            gcmWRITE_CONST(VPG_END);

            gcoOS_GetMemoryUsage(&maxrss, &ixrss, &idrss, &isrss);

            gcmWRITE_CONST(VPG_MEM);

            gcmWRITE_COUNTER(VPC_MEMMAXRES, maxrss);
            gcmWRITE_COUNTER(VPC_MEMSHARED, ixrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDDATA, idrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDSTACK, isrss);

            gcmWRITE_CONST(VPG_END);

            /* write api time counters */
            gcmWRITE_CONST(VPG_ES11_TIME);
            for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES11_TIME + 1 + i, (gctINT32)Context->profiler.apiTimes[i]);
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

            gcmWRITE_COUNTER(VPC_ES11CALLS, totalCalls_11);
            gcmWRITE_COUNTER(VPC_ES11DRAWCALLS, totalDrawCalls_11);
            gcmWRITE_COUNTER(VPC_ES11STATECHANGECALLS, totalStateChangeCalls_11);

            gcmWRITE_COUNTER(VPC_ES11POINTCOUNT, Context->profiler.drawPointCount_11);
            gcmWRITE_COUNTER(VPC_ES11LINECOUNT, Context->profiler.drawLineCount_11);
            gcmWRITE_COUNTER(VPC_ES11TRIANGLECOUNT, Context->profiler.drawTriangleCount_11);
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_END);
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
        Context->profiler.drawCount = 0;
        Context->profiler.totalDriverTime = 0;

        gcoOS_GetTime(&Context->profiler.frameStartTimeusec);

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
_glffProfilerSet(
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
        profiler->drawCount = 0;
        profiler->curFrameNumber++;
        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_BEGIN));

        gcmONERROR(gcoPROFILER_End(Context->profilerObj, gcvCOUNTER_OP_FRAME, Context->profiler.frameNumber));
        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_END));
        gcmONERROR(gcoPROFILER_Flush(Context->profilerObj));

        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_RESET));
        /* Next frame. */
        if (profiler->need_dump)
        {
            profiler->frameNumber++;
        }
        profiler->frameBegun = gcvFALSE;
        break;

    case GL1_PROFILER_FINISH_BEGIN:

        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(gcoPROFILER_EnableCounters(Context->profilerObj, gcvCOUNTER_OP_FINISH));
        profiler->drawCount = 0;

        break;

    case GL1_PROFILER_FINISH_END:

        gcmONERROR(gcoOS_GetTime(&profiler->frameEndTimeusec));
        gcmONERROR(gcoPROFILER_End(Context->profilerObj, gcvCOUNTER_OP_FINISH, Context->profiler.finishNumber));
        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_END));
        gcmONERROR(gcoPROFILER_Flush(Context->profilerObj));

        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_RESET));
        /* Next frame. */
        if (profiler->need_dump)
        {
            profiler->finishNumber++;
            profiler->frameNumber++;
        }
        profiler->frameBegun = gcvFALSE;
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

        gcmONERROR(_glffProfilerWrite(Context, GL1_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(gcoPROFILER_EnableCounters(Context->profilerObj, gcvCOUNTER_OP_DRAW));

        break;

    case GL1_PROFILER_DRAW_END:
        gcmONERROR(gcoPROFILER_End(Context->profilerObj, gcvCOUNTER_OP_DRAW, profiler->drawCount));
        profiler->drawCount++;
        break;

    default:
        break;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}
#endif
