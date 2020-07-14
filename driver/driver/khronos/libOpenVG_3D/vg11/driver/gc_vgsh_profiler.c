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


/*******************************************************************************
**    Profiler for OpenVG 1.1 driver.
*/

#include "gc_vgsh_precomp.h"
#include "gc_hal_driver.h"
#include "gc_hal_user.h"

#define _GC_OBJ_ZONE                gcdZONE_VG3D_PROFILER

#if VIVANTE_PROFILER
gctINT _vgshProfileMode = -1;
/*******************************************************************************
**    InitializeVGProfiler
**
**    Initialize the profiler for the context provided.
**
**    Arguments:
**
**        VGContext Context
**            Pointer to a new VGContext object.
*/
void
_vgshProfilerInitialize(
    _VGContext * Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    char *env = gcvNULL;
    _VGProfiler * profiler = &Context->profiler;

    gcmHEADER_ARG("Context=0x%x", Context);

    _vgshProfileMode = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &env)) && env)
    {
         if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
        {
            _vgshProfileMode = 0;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
        {
            _vgshProfileMode = 1;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "2")))
        {
            _vgshProfileMode = 2;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "3")))
        {
            _vgshProfileMode = 3;
        }
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&Context->profiler, gcmSIZEOF(_VGProfiler));
    profiler->useVGfinish = gcvFALSE;

    switch (_vgshProfileMode)
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
        profiler->useVGfinish = gcvTRUE;
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

    profiler->useVGfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_VGFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        profiler->useVGfinish = gcvTRUE;
    }

    profiler->perDrawMode = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_PERDRAW_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        profiler->perDrawMode =
        Context->profilerObj->perDrawMode = gcvTRUE;
    }

    Context->profilerObj->profilerClient = gcvCLIENT_OPENVG;

    if (gcoPROFILER_Initialize(Context->profilerObj) != gcvSTATUS_OK)
    {
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    }

    profiler->enable = gcvTRUE;
    profiler->curFrameNumber = 0;
    profiler->drawCount = 0;

    gcoOS_GetTime(&profiler->frameStartTimeusec);

    _vgshProfilerWrite(Context, VG_PROFILER_WRITE_HEADER);

OnError:
    /* Return the error. */
    gcmFOOTER_NO();
}

/*******************************************************************************
**    _DestroyVGProfiler
**
**    Initialize the profiler for the context provided.
**
**    Arguments:
**
**        VGContext Context
**            Pointer to a new VGContext object.
*/
void
_vgshProfilerDestroy(
    _VGContext * Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->profiler.enable)
    {
        Context->profiler.enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(Context->profilerObj));
    }

    /* Success. */
    gcmFOOTER_NO();
}

gceSTATUS
_vgshProfilerWrite(
    _VGContext * Context,
    VGuint Enum
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT rev;
    gcoPROFILER Profiler = Context->profilerObj;
    char* infoCompany = "Vivante Corporation";
    char* infoVersion = "1.3";
    char  infoRevision[255] = {'\0'};   /* read from hw */
    char* infoRenderer = Context->chipName;
    char* infoDriver = "OpenVG 1.1";
    gctUINT offset = 0;
    gctUINT32 totalVGCalls = 0;
    gctUINT32 totalVGDrawCalls = 0;
    gctUINT32 totalVGStateChangeCalls = 0;
    gctUINT32 maxrss, ixrss, idrss, isrss;
    gctINT32 i;

    gcmHEADER_ARG("Context=0x%x, Enum=%d", Context, Enum);

    switch (Enum)
    {
    case VG_PROFILER_WRITE_HEADER:
        /* Write Generic Info. */
        rev = Context->revision;
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
        break;

    case VG_PROFILER_WRITE_FRAME_BEGIN:
        if (!Context->profiler.frameBegun && Context->profiler.need_dump)
        {
            if (Context->profiler.frameNumber == 0 && Context->targetImage.surface)
            {
                gctUINT width, height;
                gctUINT offset = 0;
                gctCHAR  infoScreen[255] = { '\0' };
                gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
                gcoSURF_GetSize(Context->targetImage.surface, &width, &height, gcvNULL);
                gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),
                    &offset, "%d x %d", width, height);
                gcmWRITE_CONST(VPC_INFOSCREENSIZE);
                gcmWRITE_STRING(infoScreen);

                gcmWRITE_CONST(VPG_END);
            }
            gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);
            Context->profiler.frameBegun = 1;
        }
        break;

    case VG_PROFILER_WRITE_FRAME_END:
        gcmONERROR(gcoPROFILER_End(Profiler, gcvCOUNTER_OP_FRAME, Context->profiler.frameNumber));

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
            gcmWRITE_CONST(VPG_VG11_TIME);
            for (i = 0; i < NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_VG11_TIME + 1 + i, (gctINT32)Context->profiler.apiTimes[i]);
                }
            }
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_VG11);

            for (i = 0; i < NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_VG11 + 1 + i, Context->profiler.apiCalls[i]);
                    totalVGCalls += Context->profiler.apiCalls[i];

                    switch (i + APICALLBASE)
                    {
                    case VGDRAWPATH:
                    case VGDRAWIMAGE:
                    case VGDRAWGLYPH:
                        totalVGDrawCalls += Context->profiler.apiCalls[i];
                        break;

                    case VGFILLMASKLAYER:
                    case VGLOADIDENTITY:
                    case VGLOADMATRIX:
                    case VGMASK:
                    case VGMULTMATRIX:
                    case VGRENDERTOMASK:
                    case VGROTATE:
                    case VGSCALE:
                    case VGSETCOLOR:
                    case VGSETF:
                    case VGSETFV:
                    case VGSETI:
                    case VGSETIV:
                    case VGSETPAINT:
                    case VGSETPARAMETERF:
                    case VGSETPARAMETERFV:
                    case VGSETPARAMETERI:
                    case VGSETPARAMETERIV:
                    case VGSHEAR:
                    case VGTRANSLATE:
                        totalVGStateChangeCalls += Context->profiler.apiCalls[i];
                        break;

                    default:
                        break;
                    }
                }

                /* Clear variables for next frame. */
                Context->profiler.apiCalls[i] = 0;
                Context->profiler.apiTimes[i] = 0;
            }

            gcmWRITE_COUNTER(VPC_VG11CALLS, totalVGCalls);
            gcmWRITE_COUNTER(VPC_VG11DRAWCALLS, totalVGDrawCalls);
            gcmWRITE_COUNTER(VPC_VG11STATECHANGECALLS, totalVGStateChangeCalls);

            gcmWRITE_COUNTER(VPC_VG11FILLCOUNT, Context->profiler.drawFillCount);
            gcmWRITE_COUNTER(VPC_VG11STROKECOUNT, Context->profiler.drawStrokeCount);


            gcmWRITE_CONST(VPG_END);
            gcmWRITE_CONST(VPG_END);
        }
        break;

    case VG_PROFILER_WRITE_FRAME_RESET:
        for (i = 0; i < NUM_API_CALLS; ++i)
        {
            Context->profiler.apiCalls[i] = 0;
            Context->profiler.apiTimes[i] = 0;
            Context->profiler.totalDriverTime = 0;
        }

        /* Clear variables for next frame. */
        Context->profiler.drawFillCount = 0;
        Context->profiler.drawStrokeCount = 0;
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

VGboolean
_vgshProfilerSet(
    IN _VGContext * Context,
    IN VGuint Enum,
    IN gctHANDLE Value
)
{
    _VGProfiler * profiler = &Context->profiler;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x, Enum=%d, Value=0x%x", Context, Enum, Value);

    if (Context == gcvNULL || !profiler->enable)
    {
        gcmFOOTER_ARG("return=%d", VG_FALSE);
        return VG_FALSE;
    }

    switch (_vgshProfileMode)
    {
    case 1:
        if (profiler->frameCount == 0 || profiler->frameNumber < profiler->frameCount)
        {
            Context->profilerObj->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Context->profilerObj->needDump = profiler->need_dump = VG_FALSE;
        }
        break;
    case 2:
        if (profiler->enableOutputCounters)
        {
            Context->profilerObj->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Context->profilerObj->needDump = profiler->need_dump = VG_FALSE;
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
            Context->profilerObj->needDump = profiler->need_dump = VG_FALSE;
        }
        break;
    default:
        gcmFOOTER_ARG("return=%d", VG_FALSE);
        return VG_FALSE;
    }

    switch (Enum)
    {
    case VG_PROFILER_FRAME_END:

        gcmONERROR(gcoOS_GetTime(&profiler->frameEndTimeusec));
        profiler->drawCount = 0;
        profiler->curFrameNumber++;
        gcmONERROR(_vgshProfilerWrite(Context, VG_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(_vgshProfilerWrite(Context, VG_PROFILER_WRITE_FRAME_END));
        gcmONERROR(gcoPROFILER_Flush(Context->profilerObj));

        gcmONERROR(_vgshProfilerWrite(Context, VG_PROFILER_WRITE_FRAME_RESET));

        /* Next frame. */
        if (profiler->need_dump)
        {
            profiler->frameNumber++;
            profiler->frameBegun = 0;
        }
        break;

    case VG_PROFILER_PRIMITIVE_TYPE:
        profiler->primitiveType = gcmPTR2INT32(Value);
        break;

    case VG_PROFILER_PRIMITIVE_COUNT:
        profiler->primitiveCount = gcmPTR2INT32(Value);
        switch (profiler->primitiveType)
        {
        case VG_PATH:
            profiler->drawPathCount += gcmPTR2INT32(Value);
            break;
        case VG_GLYPH:
            profiler->drawGlyphCount += gcmPTR2INT32(Value);
            break;
        case VG_IMAGE:
            profiler->drawImageCount += gcmPTR2INT32(Value);
            break;
        }
        break;

   case VG_PROFILER_STROKE:
        profiler->drawStrokeCount += gcmPTR2INT32(Value);
        break;

    case VG_PROFILER_FILL:
        profiler->drawFillCount += gcmPTR2INT32(Value);
        break;

    case VG_PROFILER_DRAW_BEGIN:

        gcmONERROR(_vgshProfilerWrite(Context, VG_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(gcoPROFILER_EnableCounters(Context->profilerObj, gcvCOUNTER_OP_DRAW));

        break;

    case VG_PROFILER_DRAW_END:
        gcmONERROR(gcoPROFILER_End(Context->profilerObj, gcvCOUNTER_OP_DRAW, profiler->drawCount));
        profiler->drawCount++;
        break;

    default:
        if ((Enum > APICALLBASE) && (Enum - APICALLBASE < NUM_API_CALLS))
        {
            profiler->apiCalls[Enum - APICALLBASE]++;
        }
        break;
    }

    gcmFOOTER_ARG("return=%d", VG_TRUE);
    return VG_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", VG_FALSE);
    return VG_FALSE;
}

#endif
