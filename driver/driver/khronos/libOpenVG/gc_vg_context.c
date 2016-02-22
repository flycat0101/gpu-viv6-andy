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


#include "gc_vg_precomp.h"
#include <string.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gc_egl_common.h>

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _OPENVG_VERSION = "\n\0$VERSION$"
                               gcmTXT2STR(gcvVERSION_MAJOR) "."
                               gcmTXT2STR(gcvVERSION_MINOR) "."
                               gcmTXT2STR(gcvVERSION_PATCH) ":"
                               gcmTXT2STR(gcvVERSION_BUILD)
                               "$\n";

static gltCONTEXT_FUNCTION veglGetCurVGCtxFunc = gcvNULL;

/*******************************************************************************
**
** _ConstructChipName
**
** Construct chip name string.
**
** INPUT:
**
**  vgsCONTEXT_PTR Context
**
**
** OUTPUT:
**
**    Nothing.
*/

static void _ConstructChipName(
    vgsCONTEXT_PTR Context
    )
{

    gctSTRING ChipName = Context->chipName;
    gctSTRING productName = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

    gcoOS_ZeroMemory(ChipName, vgdCHIP_NAME_LEN);

    /* Append Vivante GC to the string. */
    *ChipName++ = 'V';
    *ChipName++ = 'i';
    *ChipName++ = 'v';
    *ChipName++ = 'a';
    *ChipName++ = 'n';
    *ChipName++ = 't';
    *ChipName++ = 'e';
    *ChipName++ = ' ';
    gcmONERROR(gcoHAL_GetProductName(Context->hal, &productName));

    gcoOS_StrCatSafe(Context->chipName, vgdCHIP_NAME_LEN , productName);

    gcmOS_SAFE_FREE(Context->os, productName);

OnError:

    gcmFOOTER_NO();
    return;

}

void _WriteAPITimeInfo(
    vgsCONTEXT_PTR context,
    gctSTRING functionName,
    gctUINT64 Value)
{
    gctUINT offset = 0;
    gctSIZE_T length;
    gctCHAR savedValue[256] = {'\0'};
#if gcdGC355_PROFILER
    gctSTRING p;
    gctSTRING subString = "vgDraw";
    gctCHAR s[256] = "";

    p = strstr(functionName,subString);
#endif

    if(context == gcvNULL)
        return;
    if(context->apiTimeFile == gcvNULL)
        return;

#if gcdGC355_PROFILER
    if(context->pathName && (p != gcvNULL))
    {
        if (context->TreeDepth)
        {
            memset(s,'-',(context->TreeDepth-context->varTreeDepth)*4+1);
            s[0] = '|';
            gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%s][%d]%sAPI Time for %s = %llu(microsec) \n",context->pathName,context->TreeDepth-context->varTreeDepth,s,functionName,Value);
        }
        else
        {
            gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%s] API Time for %s = %llu(microsec) \n",context->pathName,functionName,Value);
        }
    }
    else
    {
        if (context->TreeDepth)
        {
            memset(s,'-',(context->TreeDepth-context->varTreeDepth)*4+1);
            s[0] = '|';
            gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%d]%sAPI Time for %s = %llu(microsec) \n",context->TreeDepth-context->varTreeDepth,s,functionName,Value);
        }
        else
        {
            gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "API Time for %s = %llu(microsec) \n",functionName,Value);
        }
    }
#else
    gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "API Time for %s = %llu(microsec) \n",functionName,Value);
#endif
    length = gcoOS_StrLen((gctSTRING)savedValue, gcvNULL);
    gcoOS_Write(gcvNULL,context->apiTimeFile,length, savedValue);
}

#if gcdGC355_PROFILER
void _WriteProfileElapsedTimeInfo(
    vgsCONTEXT_PTR context,
    gctSTRING functionName)
{
    gctUINT64 endTime = 0;
    gctUINT64 deltaValue = 0;
    gctUINT offset = 0;
    gctSIZE_T length;
    gctCHAR savedValue[256] = {'\0'};
    gctCHAR Tag = '-';

    if(context == gcvNULL)
        return;
    if(context->apiTimeFile == gcvNULL)
        return;

    gcoOS_GetTime(&endTime);
    deltaValue = (endTime - context->appStartTime);
    gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%-10llu us]", deltaValue);

    if(context->pathName)
    {
        gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%s] ",context->pathName);
    }
    else
    {
        gcoOS_PrintStrSafe(savedValue, gcmSIZEOF(savedValue), &offset, "[%c] ",Tag);
    }

    length = gcoOS_StrLen((gctSTRING)savedValue, gcvNULL);
    gcoOS_Write(gcvNULL,context->apiTimeFile,length, savedValue);
}
#endif

VGboolean vivProfilerEnableDisable(gctBOOL enable, gctSTRING functionName)
{
    VGboolean error =  VG_TRUE ;
#if gcdGC355_PROFILER
    vgsCONTEXT_PTR Context;
    vgsTHREADDATA_PTR Thread;

    Thread = vgfGetThreadData(gcvFALSE);

    if (Thread == gcvNULL)
    {
        Context = gcvNULL;
        return VG_FALSE ;
    }

    Context = Thread->context;
    Context->enableGetAPITimes = enable;
    _WriteProfileElapsedTimeInfo(Context,functionName);
    gcoVG_ProfilerEnableDisable(Context->vg, Context->enableGetAPITimes, Context->apiTimeFile);
#endif
    return error;
}

VG_API_CALL VGboolean VG_API_ENTRY vivProfilerEnable(void)
{
    VGboolean error = VG_TRUE;
#if gcdGC355_PROFILER
    error = vivProfilerEnableDisable(gcvTRUE,"vivProfilerEnable");
#endif
    return error;
}

VG_API_CALL VGboolean VG_API_ENTRY vivProfilerDisable(void)
{
    VGboolean error = VG_TRUE;
#if gcdGC355_PROFILER
    error = vivProfilerEnableDisable(gcvFALSE,"vivProfilerDisable");
#endif
    return error;
}

VG_API_CALL VGboolean VG_API_ENTRY vivProfilerTag(char* pathName)
{
    VGboolean error = VG_TRUE;
#if gcdGC355_PROFILER
    vgsCONTEXT_PTR Context;
    vgsTHREADDATA_PTR Thread;

    Thread = vgfGetThreadData(gcvFALSE);

    if (Thread == gcvNULL)
    {
        Context = gcvNULL;
        return VG_FALSE;
    }

    Context = Thread->context;
    Context->pathName = (gctSTRING)(pathName);
    _WriteProfileElapsedTimeInfo(Context,"vivProfilerTag");
#endif
    return error;
}

VG_API_CALL VGboolean VG_API_ENTRY vivSetProfilerTreeDepth(gctUINT layer)
{
    VGboolean error = VG_TRUE;
#if gcdGC355_PROFILER
    vgsCONTEXT_PTR Context;
    vgsTHREADDATA_PTR Thread;

    Thread = vgfGetThreadData(gcvFALSE);

    if (Thread == gcvNULL)
    {
        Context = gcvNULL;
        return VG_FALSE;
    }

    Context = Thread->context;
    Context->TreeDepth = layer;
    Context->saveLayerTreeDepth = layer;
    Context->varTreeDepth = layer;
    _WriteProfileElapsedTimeInfo(Context,"vivSetProfilerTreeDepth");
    gcoVG_ProfilerTreeDepth(Context->vg, Context->TreeDepth);
#endif
    return error;
}


/*******************************************************************************
**
** vgfFlushPipe
**
** Flush and stall the hardware if needed.
**
** INPUT:
**
**    Context
**       Pointer to the context.
**
**    Finish
**       If not zero, makes sure the hardware is idle.
**
** OUTPUT:
**
**    Nothing.
*/

gceSTATUS vgfFlushPipe(
    IN vgsCONTEXT_PTR Context,
    IN gctBOOL Finish
    )
{
    gceSTATUS status;
    vgmENTERSUBAPI(vgfFlushPipe);

    do
    {
#if gcdGC355_PROFILER
        /* Flush PE cache. */
        gcmERR_BREAK(gcoHAL_Flush(Context->hal
            , Context->vg,
            Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth
            ));
#else
        gcmERR_BREAK(gcoHAL_Flush(Context->hal
            ));
#endif

        /* Need to stall the hardware as well? */
        if (Finish)
        {
            /* Make sure the hardware is idle. */
            gcmERR_BREAK(gcoHAL_Commit(Context->hal, gcvTRUE));

            /* Reset the states. */
            Context->imageDirty = vgvIMAGE_READY;
        }
        else
        {
            /* Make sure the hardware is idle. */
            gcmERR_BREAK(gcoHAL_Commit(Context->hal, gcvFALSE));

            /* Mark the states as not finished. */
            Context->imageDirty = vgvIMAGE_NOT_FINISHED;
        }
    }
    while (gcvFALSE);

    vgmLEAVESUBAPI(vgfFlushPipe);
    /* Return state. */
    return status;
}


/*******************************************************************************
**
** veglCreateContext
**
** Create and initialize a context object.
**
** INPUT:
**
**    Os
**       Pointer to a gcoOS object.
**
**    Hal
**       Pointer to a gcoHAL object.
**
**    SharedContext
**       Pointer to an existing OpenVG context. Any VGPath and VGImage objects
**       defined in SharedContext will be accessible from the new context, and
**       vice versa. If no sharing is desired, the value EGL_NO_CONTEXT should
**       be used.
**
**     getCurAPICtx
**         veglGetCurAPICtxFunc function pointer.
**
** OUTPUT:
**
**    vgsCONTEXT_PTR
**       Pointer to the new context object.
*/

static void *
veglCreateContext(
    gcoOS Os,
    gcoHAL Hal,
#if gcdGC355_PROFILER
    gctUINT64 appStartTime,
    gctFILE apiTimeFile,
#endif
    gctINT ClientVersion,
    VEGLimports *Imports,
    void * SharedContext
    )
{
    gceSTATUS status;
    vgsCONTEXT_PTR context = gcvNULL;
    gctCHAR* env = gcvNULL;
    gctCHAR dumpAPITimeFileName[256] = {'\0'};

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_PARAMETERS,
        "%s(0x%08X, 0x%08X, 0x%08X);\n",
        __FUNCTION__,
        Os, Hal, SharedContext
        );

    do
    {
        /* Set veglGetCurAPICtxFunc function pointer. */
        if (veglGetCurVGCtxFunc == gcvNULL && Imports && Imports->getCurContext)
        {
            veglGetCurVGCtxFunc = Imports->getCurContext;
        }

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG));

        /* Allocate the context structure. */
        gcmERR_BREAK(gcoOS_Allocate(
            Os,
            gcmSIZEOF(vgsCONTEXT),
            (gctPOINTER *) &context
            ));

        /* Init context structure. */
        gcoOS_ZeroMemory(context, gcmSIZEOF(vgsCONTEXT));

        /* Set API pointers. */
        context->hal = Hal;
        context->os  = Os;
#if gcdGC355_PROFILER
        context->enableGetAPITimes = gcvFALSE;
        context->appStartTime = appStartTime;
        context->apiTimeFile = apiTimeFile;
        context->pathName = gcvNULL;
        context->TreeDepth = 0;
        context->saveLayerTreeDepth = 0;
        context->varTreeDepth = 0;
#else
        context->enableGetAPITimes = gcvFALSE;
        context->apiTimeFile = gcvNULL;
#endif
        /* Query chip identity. */
        gcmERR_BREAK(gcoHAL_QueryChipIdentity(
            Hal,
            &context->chipModel,
            &context->chipRevision,
            gcvNULL,
            gcvNULL
            ));

        /* Test chip ID. */
        if (context->chipModel == 0)
        {
            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        /* Construct chip name. */
        _ConstructChipName(context);

        /* Initialize driver strings. */
        context->chipInfo.vendor     = (VGubyte*) "Vivante Corporation";
        context->chipInfo.renderer   = (VGubyte*) context->chipName;
        context->chipInfo.version    = (VGubyte*) "1.1";
        context->chipInfo.extensions = (VGubyte*) "";

        /* Query VG engine version. */
        context->vg20 = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_VG20
            ) == gcvSTATUS_TRUE;

        /* Query filter support. */
        context->filterSupported = gcoHAL_IsFeatureAvailable(
            Hal, gcvFEATURE_VG_FILTER
            ) == gcvSTATUS_TRUE;

        /* Get the 3D engine pointer. */
        gcmERR_BREAK(gcoHAL_GetVGEngine(
            context->hal, &context->vg
            ));

        /* Create a signal for waiting on. */
        gcmERR_BREAK(gcoOS_CreateSignal(
            context->os, gcvTRUE, &context->waitSignal
            ));

        /* Initialize the object cache. */
        gcmERR_BREAK(vgfObjectCacheStart(
            context, (vgsCONTEXT_PTR) SharedContext
            ));

#if gcdMOVG
        /* Load the customizable ts buffer size. */
        {
            gctFILE tf = gcvNULL;
            gctUINT length = 0;
            gctSIZE_T read = 0;
            gctCHAR buf[20];
            gctCHAR *strArg[4];
            gctUINT32   argCount;
            gctBOOL     argValid;
            gcoOS_ZeroMemory(buf, 10);
            status = gcoOS_Open(gcvNULL, "ts.cfg", gcvFILE_READTEXT, &tf);
            if (tf != gcvNULL)
            {
                gctINT i = 0;
                gctINT j = 0;
                gcoOS_Seek(gcvNULL, tf, 0, gcvFILE_SEEK_END);
                gcoOS_GetPos(gcvNULL, tf, &length);
                gcoOS_Seek(gcvNULL, tf, 0, gcvFILE_SEEK_SET);
                gcoOS_Read(gcvNULL, tf, gcmMIN(length, 20), buf, &read);

                argValid = gcvTRUE;
                argCount = 0;
                strArg[argCount++] = &buf[0];
                for (i = 0; i < (gctINT)read; i++)
                {
                    if (buf[i] == ',')
                    {
                        buf[i] = '\0';
                        if (i + 1 < (gctINT)length)
                        {
                            strArg[argCount++] = &buf[i+1];
                            j++;
                        }
                        else
                        {
                            argValid = gcvFALSE;
                            break;
                        }
                    }
                }
                if (argValid && (j == 3))
                {
                    gcoOS_StrToInt(strArg[0], (gctINT*)&context->tsWidth);
                    gcoOS_StrToInt(strArg[1], (gctINT*)&context->tsHeight);
                    gcoOS_StrToInt(strArg[2], (gctINT*)&context->pCache);
                    gcoOS_StrToInt(strArg[3], (gctINT*)&context->sCache);
                }
                else
                {
                    /* Invalid config file read. */
                    context->tsWidth = 256;
                    context->tsHeight = 256;
                    context->pCache  = 1;
                    context->sCache  = 1;
                }

                gcoOS_Close(gcvNULL, tf);
            }
            else
            {
                context->tsWidth = 256;
                context->tsHeight = 256;
                context->pCache  = 1;
                context->sCache  = 1;
            }
        }
        /* Construct the path storage manager. */
        gcmERR_BREAK(vgsPATHSTORAGE_Construct(
            context,
            gcmKB2BYTES(context->pCache),
            0,
            &context->pathStorage
            ));

        /* Construct the stroke storage manager. */
        gcmERR_BREAK(vgsPATHSTORAGE_Construct(
            context,
            gcmKB2BYTES(context->sCache),
            gcmMB2BYTES(2),
            &context->strokeStorage
            ));
#else
        /* Construct the path storage manager. */
        gcmERR_BREAK(vgsPATHSTORAGE_Construct(
            context,
            gcmKB2BYTES(64),
            0,
            &context->pathStorage
            ));

        /* Construct the stroke storage manager. */
        gcmERR_BREAK(vgsPATHSTORAGE_Construct(
            context,
            gcmKB2BYTES(64),
            gcmMB2BYTES(2),
            &context->strokeStorage
            ));
#endif

        /* Construct a memory manager object for ARC coordinates. */
        gcmERR_BREAK(vgsMEMORYMANAGER_Construct(
            context,
            Os,
            gcmSIZEOF(vgsARCCOORDINATES),
            100,
            &context->arcCoordinates
            ));

        /* Initialize the context states. */
        vgfSetDefaultStates(context);

        /* Create default paint object. */
        gcmERR_BREAK(vgfReferencePaint(context, &context->defaultPaint));
        context->defaultPaint->object.userValid = VG_FALSE;

        /* Set fill and stroke paints to the default paint. */
        context->strokePaint = context->fillPaint = context->defaultPaint;
        context->strokeDefaultPaint = context->fillDefaultPaint = VG_TRUE;

        /* Report new context. */
        vgmREPORT_NEW_CONTEXT(context);

        gcoOS_GetEnv(gcvNULL, "VG_APITIME", &env);
        if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
        {
            context->enableGetAPITimes = gcvTRUE;
            {
                /*generate file name for each context*/
                static gctUINT8 num = 1;
                gctHANDLE pid = gcoOS_GetCurrentProcessID();
                gctHANDLE tid = gcoOS_GetCurrentThreadID();
                gctUINT offset = 0;
#if defined(ANDROID)
                gcoOS_PrintStrSafe(dumpAPITimeFileName, gcmSIZEOF(dumpAPITimeFileName), &offset, "/data/data/APITimes_pid%d_tid%d.log",(gctUINTPTR_T)pid, (gctUINTPTR_T)tid);
#else
                gcoOS_PrintStrSafe(dumpAPITimeFileName, gcmSIZEOF(dumpAPITimeFileName), &offset, "APITimes_pid%d_tid%d_context%d.log",(gctUINTPTR_T)pid, (gctUINTPTR_T)tid, num);
#endif
                gcoOS_Open(gcvNULL,dumpAPITimeFileName,gcvFILE_CREATE,&context->apiTimeFile);
                if(context->apiTimeFile == gcvNULL)
                {
                    context->enableGetAPITimes = gcvFALSE;
                }
                num++;
            }
        }

        /* Image objects. */
        context->maskImage      = gcvNULL;
        context->targetChanged  = gcvTRUE;
    }
    while (gcvFALSE);

    /* Free the context buffer if error. */
    if (gcmIS_ERROR(status) && (context != gcvNULL))
    {
        /* Delete default paint. */
        gcmVERIFY_OK(vgfDereferenceObject(
            context, (vgsOBJECT_PTR *) &context->defaultPaint
            ));

        /* Destroy the stroke storage mamager. */
        if (context->strokeStorage != gcvNULL)
        {
            gcmVERIFY_OK(vgsPATHSTORAGE_Destroy(context, context->strokeStorage));
        }

        /* Destroy the path storage mamager. */
        if (context->pathStorage != gcvNULL)
        {
            gcmVERIFY_OK(vgsPATHSTORAGE_Destroy(context, context->pathStorage));
        }

        /* Delete object cache. */
        gcmVERIFY_OK(vgfObjectCacheStop(context));

        /* Destroy the ARC coordinates memory manager. */
        if (context->arcCoordinates != gcvNULL)
        {
            gcmVERIFY_OK(vgsMEMORYMANAGER_Destroy(context, context->arcCoordinates));
        }

        /* Destroy the wait signal. */
        if (context->waitSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DestroySignal(context->os, context->waitSignal));
        }

        /* Free the context. */
        gcmVERIFY_OK(gcoOS_Free(Os, context));

        /* Reset the pointer. */
        context = gcvNULL;
    }

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_PARAMETERS,
        "%s() = 0x%08X;\n",
        __FUNCTION__,
        context
        );

    return context;
}

static void *
veglCreateContextEx(
    void * Thread,
#if gcdGC355_PROFILER
    gctUINT64 appStartTime,
    gctFILE apiFileTime,
#endif
    gctINT ClientVersion,
    VEGLimports *Imports,
    void * SharedContext
    )
{
#if gcdGC355_PROFILER
    return veglCreateContext(gcvNULL, gcvNULL,appStartTime,apiFileTime,
                ClientVersion, Imports, SharedContext);
#else
    return veglCreateContext(gcvNULL, gcvNULL,
                ClientVersion, Imports, SharedContext);
#endif
}

/*******************************************************************************
**
** veglDestroyContext
**
** Destroy a context object.
**
** INPUT:
**
**    Context
**       Pointer to the current context.
**
** OUTPUT:
**
**    gctBOOL
**       Not zero if successfully destroyed.
*/

static EGLBoolean
veglDestroyContext(
    void * Thread,
    void * Context
    )
{
    gceSTATUS status;
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR) Context;
    gctCHAR* env = gcvNULL;

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_PARAMETERS,
        "%s(0x%08X);\n",
        __FUNCTION__,
        Context
        );

    do
    {
        vgsTHREADDATA_PTR thread;

        /* Cannot be NULL. */
        if (Context == gcvNULL)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

#if gcdGC355_PROFILER
        context->enableGetAPITimes = gcvFALSE;
        context->pathName = gcvNULL;
        context->TreeDepth = 0;
        context->saveLayerTreeDepth = 0;
        context->varTreeDepth = 0;
#endif

        /* Get the thread information. */
        thread = vgfGetThreadData(gcvFALSE);

        /* Flush the pipe. */
        gcmERR_BREAK(vgfFlushPipe(context, gcvTRUE));

        /* Delete the temporary buffer. */
        if (context->tempBuffer != gcvNULL)
        {
            gcmERR_BREAK(gcoOS_Free(
                context->os, context->tempBuffer
                ));

            context->tempBuffer = gcvNULL;
        }

        /* Reset the current context. */
        if ((thread != gcvNULL) && (thread->context == context))
        {
            if(context->targetImage.surface != gcvNULL)
            {
#if gcdGC355_PROFILER
                gcmERR_BREAK(gcoVG_UnsetTarget(
                    context->vg,
                    context->TreeDepth,
                    context->saveLayerTreeDepth,
                    context->varTreeDepth,
                    context->targetImage.surface
                    ));
#else
                gcmERR_BREAK(gcoVG_UnsetTarget(
                    context->vg,
                    context->targetImage.surface
                    ));
#endif
            }

            if(context->maskImage != gcvNULL)
            {
                gcmASSERT(context->maskImage->surface != gcvNULL);
#if gcdGC355_PROFILER
                gcmERR_BREAK(gcoVG_UnsetMask(
                    context->vg,
                    context->TreeDepth,
                    context->saveLayerTreeDepth,
                    context->varTreeDepth,
                    context->maskImage->surface
                    ));
#else
                gcmERR_BREAK(gcoVG_UnsetMask(
                    context->vg,
                    context->maskImage->surface
                    ));
#endif
            }

            thread->context = gcvNULL;
        }

        /* Delete temporary surface. */
        if (context->tempNode != 0)
        {
#if gcdGC355_PROFILER
            /* Schedule the current buffer for deletion. */
            gcmERR_BREAK(gcoHAL_ScheduleVideoMemory(
                context->hal,
                context->vg,
                context->TreeDepth, context->saveLayerTreeDepth, context->varTreeDepth,
                context->tempNode
                ));
#else
            gcmERR_BREAK(gcoHAL_ScheduleVideoMemory(
                context->hal,
                context->tempNode
                ));
#endif
            /* Reset temporary surface. */
            context->tempNode     = 0;
            context->tempPhysical = ~0;
            context->tempLogical  = gcvNULL;
            context->tempSize     = 0;
        }

        /* Release images. */
        {
            gcoSURF surface = context->targetImage.surface;
            gcmERR_BREAK(vgfReleaseImage(context, &context->targetImage));

            if (surface != gcvNULL)
            {
                /* deference the target surface */
                gcmVERIFY_OK(gcoSURF_Destroy(surface));
            }

        }

        if (context->maskImage)
        {
            gcmERR_BREAK(vgfReleaseImage(context, context->maskImage));
            context->maskImage = gcvNULL;
        }
        gcmERR_BREAK(vgfReleaseImage(context, &context->tempMaskImage));
        gcmERR_BREAK(vgfReleaseImage(context, &context->wrapperImage));
        gcmERR_BREAK(vgfReleaseImage(context, &context->tempImage));

        /* Delete stroke paint. */
        if ((context->strokePaint != gcvNULL) && (context->strokePaint != context->defaultPaint))
        {
            gcmERR_BREAK(vgfDereferenceObject(
                context, (vgsOBJECT_PTR *) &context->strokePaint
                ));
        }
        /* Delete fill paint. */
        if ((context->fillPaint != gcvNULL) && (context->fillPaint != context->defaultPaint))
        {
            gcmERR_BREAK(vgfDereferenceObject(
                context, (vgsOBJECT_PTR *) &context->fillPaint
                ));
        }

        /* Delete default paint. */
        gcmERR_BREAK(vgfDereferenceObject(
            context, (vgsOBJECT_PTR *) &context->defaultPaint
            ));

        /* Delete the object cache. */
        gcmERR_BREAK(vgfObjectCacheStop(context));

#if gcvSTROKE_KEEP_MEMPOOL
        /* Destroy the stroke conversion object. */
        if (context->strokeConversion != gcvNULL)
        {
            gcmERR_BREAK(vgfDestroyStrokeConversion(context, context->strokeConversion));
            context->strokeConversion = gcvNULL;
        }
#endif

        /* Destroy the stroke storage mamager. */
        if (context->strokeStorage != gcvNULL)
        {
            gcmERR_BREAK(vgsPATHSTORAGE_Destroy(context, context->strokeStorage));
            context->strokeStorage = gcvNULL;
        }

        /* Destroy the path storage mamager. */
        if (context->pathStorage != gcvNULL)
        {
            gcmERR_BREAK(vgsPATHSTORAGE_Destroy(context, context->pathStorage));
            context->pathStorage = gcvNULL;
        }

        /* Destroy the ARC coordinates memory manager. */
        if (context->arcCoordinates != gcvNULL)
        {
            gcmERR_BREAK(vgsMEMORYMANAGER_Destroy(context, context->arcCoordinates));
            context->arcCoordinates = gcvNULL;
        }

        /* Destroy the wait signal. */
        if (context->waitSignal != gcvNULL)
        {
            gcmERR_BREAK(gcoOS_DestroySignal(context->os, context->waitSignal));
            context->waitSignal = gcvNULL;
        }

        gcoOS_GetEnv(gcvNULL, "VG_APITIME", &env);
        if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
        {
            if (context->apiTimeFile != gcvNULL)
            {
                gcoOS_Flush(gcvNULL,context->apiTimeFile);
                gcoOS_Close(gcvNULL, context->apiTimeFile);
                context->apiTimeFile = gcvNULL;
                context->enableGetAPITimes = gcvFALSE;
            }
        }

#if gcdGC355_MEM_PRINT
        /* Print the memory info when exiting. */
        gcmPRINT("== Memory profiling results in bytes ==\n");
        gcmPRINT("01) VGImage objects: %d \n", context->maxMemImage);
        gcmPRINT("02) VGPath objects : %d \n", context->maxMemPath);
#endif
        /* Destroy the context object. */
        gcmERR_BREAK(gcoOS_Free(context->os, Context));
    }
    while (gcvFALSE);

    /* Return result. */
    return gcmIS_SUCCESS(status) ? EGL_TRUE : EGL_FALSE;
}


/*******************************************************************************
**
** veglSetContext
**
** Set current context to the specified one.
**
** INPUT:
**
**    Context
**       Pointer to the context to be set as current.
**
**    Draw
**       Pointer to the surface to be used for drawing.
**
**    Read
**       Pointer to the surface to be used for reading.
**
**    Depth
**       Pointer to the surface to be used as depth buffer.
**
** OUTPUT:
**
**    Nothing.
*/

static EGLBoolean
veglSetContext(
    void       * Thread,
    void       * Context,
    VEGLDrawable Drawable,
    VEGLDrawable Readable
    )
{
    gceSTATUS status;
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR) Context;
    gcoSURF Draw  = Drawable ? (gcoSURF)Drawable->rtHandle : gcvNULL;
    /*gcoSURF Read  = Readable ? (gcoSURF)Readable->rtHandle : gcvNULL;
    gcoSURF Depth = Drawable ? (gcoSURF)Drawable->depthHandle : gcvNULL;*/

    gcmTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_PARAMETERS,
        "%s(0x%08X, 0x%08X, 0x%08X, 0x%08X);\n",
        __FUNCTION__,
        Context, Draw
        );

    do
    {
        vgsTHREADDATA_PTR thread;
        vgsIMAGE_PTR targetImage;
        /* Get the thread information. */
        thread = vgfGetThreadData(gcvTRUE);

        if (thread == gcvNULL)
        {
            status = gcvSTATUS_OUT_OF_MEMORY;
            break;
        }

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG));

        /* If there was a context already set before, release it. */
        if (thread->context != gcvNULL)
        {
            if (thread->context->targetImage.surface != gcvNULL)
            {
#if gcdGC355_PROFILER
                /* Unset the current target. */
                gcmERR_BREAK(gcoVG_UnsetTarget(
                    thread->context->vg,
                    thread->context->TreeDepth,
                    thread->context->saveLayerTreeDepth,
                    thread->context->varTreeDepth,
                    thread->context->targetImage.surface
                    ));
#else
                gcmERR_BREAK(gcoVG_UnsetTarget(
                    thread->context->vg,
                    thread->context->targetImage.surface
                    ));
#endif
            }

            /* Reset the context. */
            thread->context = gcvNULL;
        }

        /* Reset the context. */
        if (context == gcvNULL)
        {
            gcmASSERT(Draw  == gcvNULL);
            /*gcmASSERT(Read  == gcvNULL);
            gcmASSERT(Depth == gcvNULL);*/

            /* Success. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Advance the API counter. */
        vgmADVANCE_SET_CONTEXT_COUNT(context);

        /* Cannot have a context already set. */
        gcmASSERT(thread->context == gcvNULL);

        /* Set the context. */
        thread->context = context;

        /* Make shortcuts to the images. */
        targetImage = &context->targetImage;

        /* Determine whether the draw surface has changed. */
        if (targetImage->surface != Draw)
        {
            gcoSURF surface = targetImage->surface;
            context->targetChanged = gcvTRUE;
            /* Release previously initialized target image. */
            gcmERR_BREAK(vgfReleaseImage(
                context, targetImage
                ));

            if (surface != gcvNULL)
            {
                /* deference the target surface */
                gcmVERIFY_OK(gcoSURF_Destroy(surface));
            }

            /* Initialize new target image. */
            if (Draw != gcvNULL)
            {
                gcmERR_BREAK(vgfInitializeImage(
                    context, targetImage, Draw
                    ));
            }
            else
            {
                break;
            }

            gcmERR_BREAK(gcoSURF_ReferenceSurface(Draw));
        }

        /* Assume conformance test for 64x64 render targets. */
        context->conformance
            =  (targetImage->size.width  == 64)
            && (targetImage->size.height == 64);

        /* For the conformance test force software tesselation
           for 64x64 surfaces. */
#if vgvFORCE_SW_TS
        context->useSoftwareTS = gcvTRUE;
#elif vgvFORCE_HW_TS
        context->useSoftwareTS = gcvFALSE;
#else
        context->useSoftwareTS = context->conformance;
#endif

        /* Print target info. */
        gcmTRACE_ZONE(
            gcvLEVEL_INFO,
            gcvZONE_CONTEXT,
            "%s(%d) DRAW SURFACE SET:\n"
            "  %dx%d, %s%s R%dG%dB%dA%d (%d bits per pixel), format=%d\n",
            __FUNCTION__, __LINE__,
            targetImage->size.width,
            targetImage->size.height,
            targetImage->surfaceFormat->linear
                ? "lRGBA" : "sRGBA",
            targetImage->surfaceFormat->premultiplied
                ? "_PRE" : "_NONPRE",
            targetImage->surfaceFormat->r.width & gcvCOMPONENT_WIDTHMASK,
            targetImage->surfaceFormat->g.width & gcvCOMPONENT_WIDTHMASK,
            targetImage->surfaceFormat->b.width & gcvCOMPONENT_WIDTHMASK,
            targetImage->surfaceFormat->a.width & gcvCOMPONENT_WIDTHMASK,
            targetImage->surfaceFormat->bitsPerPixel,
            targetImage->surfaceFormat->internalFormat
            );
    }
    while (gcvFALSE);

    /* Return result. */
    return gcmIS_SUCCESS(status);
}

/*******************************************************************************
**
** veglUnsetContext
**
** Unset current context for the specified one.
**
** INPUT:
**
**    Context
**       Pointer to the context to be unset from current.
**/
static EGLBoolean
veglUnsetContext(
    void * Thread,
    void * Context
    )
{
    /*
     * No surfaceless context supported for openvg. veglSetContext function
     * can handle lose current.
     */
    return veglSetContext(Thread, Context, gcvNULL, gcvNULL);
}


/*******************************************************************************
**
** veglSetBuffer
**
** Set render target (multi-buffer support).
**
** INPUT:
**
**    Draw
**       Pointer to the surface to be used for drawing.
**
** OUTPUT:
**
**    Nothing.
*/

static gceSTATUS
veglSetBuffer(
    gcoSURF Draw
    )
{
    gceSTATUS status = gcvSTATUS_GENERIC_IO;
    vgsTHREADDATA_PTR Thread;
    vgsCONTEXT_PTR Context;

    Thread = vgfGetThreadData(gcvFALSE);
    Context = Thread->context;
    do
    {
        /* Draw surface cannot be NULL. */
        gcmASSERT(Draw != gcvNULL);

        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s(0x%08X);\n",
            __FUNCTION__,
            Draw
            );

        /* Same as the current? */
        if (Draw == Context->targetImage.surface)
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* Unlock the current buffer. */
        if (Context->targetImage.buffer != gcvNULL)
        {
            gcmERR_BREAK(gcoSURF_Unlock(
                Context->targetImage.surface,
                Context->targetImage.buffer
                ));

            Context->targetImage.buffer = gcvNULL;
        }

        /* Lock the new buffer. */
        gcmERR_BREAK(gcoSURF_Lock(
            Draw,
            gcvNULL,
            (gctPOINTER *) &Context->targetImage.buffer
            ));

        if (Context->targetImage.surface != gcvNULL)
        {

#if gcdENABLE_3D
            gcsTLS_PTR tls;
            gcmERR_BREAK(gcoOS_GetTLS(&tls));

            /* Unset VG target. */
            if (tls->engineVG != gcvNULL)
            {
#if gcdGC355_PROFILER
                gcmERR_BREAK(
                    gcoVG_UnsetTarget(tls->engineVG,
                    0,0,0,
                    Context->targetImage.surface));
#else
                gcmERR_BREAK(
                    gcoVG_UnsetTarget(tls->engineVG,
                    Context->targetImage.surface));
#endif
            }
#endif

            gcmERR_BREAK(gcoSURF_Destroy(Context->targetImage.surface));
        }
        /* Set the surface object. */
        Context->targetImage.surface = Draw;

        gcmVERIFY_OK(gcoSURF_ReferenceSurface(Draw));

    }
    while(gcvFALSE);
    /* Return status. */
    return status;
}


/*******************************************************************************
**
** vgGetError
**
** vgGetError returns the oldest error code provided by an API call on the
** current context since the previous call to vgGetError on that context
** (or since the creation of the context). No error is indicated by a return
** value of 0 (VG_NO_ERROR). After the call, the error code is cleared to 0.
** If no context is current at the time vgGetError is called, the error code
** VG_NO_CONTEXT_ERROR is returned. Pending error codes on existing contexts
** are not affected by the call.
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    VGErrorCode
**       OpenVL error code.
*/

VG_API_CALL VGErrorCode VG_API_ENTRY vgGetError(
    void
    )
{
    vgmENTERAPI(vgGetError)
    {
        /* Set the return error. */
        error = Context->error;

        /* Reset the current error. */
        Context->error = VG_NO_ERROR;
    }
    vgmLEAVEAPI(vgGetError);

    /* Return result. */
    return error;
}


/*******************************************************************************
**
** vgGetString
**
** The vgGetString function returns information about the OpenVG implemen-
** tation, including extension information. The values returned may vary
** according to the display (e.g., the EGLDisplay when using EGL) associated
** with the current context. If no context is current, vgGetString returns
** gcvNULL. The combination of VG_VENDOR and VG_RENDERER may be used together as
** a platform identifier by applications that wish to recognize a particular
** platform and adjust their algorithms based on prior knowledge of platform
** bugs and performance characteristics.
**
** If name is VG_VENDOR, the name of company responsible for this OpenVG
** implementation is returned. This name does not change from release to
** release.
**
** If name is VG_RENDERER, the name of the renderer is returned. This name is
** typically specific to a particular configuration of a hardware platform,
** and does not change from release to release.
**
** If name is VG_VERSION, the version number of the specification implemented
** by the renderer is returned as a string in the form
** major_number.minor_number. For this specification, "1.1" is returned.
**
** If name is VG_EXTENSIONS, a space-separated list of supported extensions to
** OpenVG is returned.
**
** For other values of name, gcvNULL is returned.
**
** INPUT:
**
**    Name
**       Specifies a symbolic constant, one of:
**          VG_VENDOR
**          VG_RENDERER
**          VG_VERSION
**          VG_EXTENSIONS
**
** OUTPUT:
**
**    const VGubyte *
**       String describing the current OpenVL connection.
*/

VG_API_CALL const VGubyte * VG_API_ENTRY vgGetString(
    VGStringID Name
    )
{
    VGubyte* pointer = gcvNULL;

    vgmENTERAPI(vgGetString)
    {
        switch (Name)
        {
        case VG_VENDOR:
            pointer = Context->chipInfo.vendor;
            break;

        case VG_RENDERER:
            pointer = Context->chipInfo.renderer;
            break;

        case VG_VERSION:
            pointer = Context->chipInfo.version;
            break;

        case VG_EXTENSIONS:
            pointer = Context->chipInfo.extensions;
            break;

        default:
            pointer = gcvNULL;
        }
    }
    vgmLEAVEAPI(vgGetString);

    return pointer;
}


/*******************************************************************************
**
** vgHardwareQuery
**
** The vgHardwareQuery function returns a value indicating whether a given
** setting of a property of a type given by Key is generally accelerated in
** hardware on the currently running OpenVG implementation.
**
** The return value will be one of the values VG_HARDWARE_ACCELERATED or
** VG_HARDWARE_UNACCELERATED, taken from the VGHardwareQueryResult enumeration.
** The legal values for the setting parameter depend on the value of the key
** parameter.
**
** INPUT:
**
**    Key
**       One of:
**          VG_IMAGE_FORMAT_QUERY
**          VG_PATH_DATATYPE_QUERY
**
**    Setting
**       Vallues from VGImageFormat or VGPathDatatype enumerations depending
**       on Key value.
**
** OUTPUT:
**
**    VGHardwareQueryResult
**       VG_HARDWARE_ACCELERATED or VG_HARDWARE_UNACCELERATED value.
*/

VG_API_CALL VGHardwareQueryResult VG_API_ENTRY vgHardwareQuery(
    VGHardwareQueryType Key,
    VGint Setting
    )
{
    VGHardwareQueryResult result = VG_HARDWARE_UNACCELERATED;

    vgmENTERAPI(vgHardwareQuery)
    {
        switch (Key)
        {
        case VG_IMAGE_FORMAT_QUERY:
            if (((Setting >= VG_sRGBX_8888) && (Setting <= VG_A_4) &&
                 (Setting != VG_BW_1) && (Setting != VG_A_1)) ||
                ((Setting >= VG_sXRGB_8888) && (Setting <= VG_lARGB_8888_PRE)) ||
                ((Setting >= VG_sBGRX_8888) && (Setting <= VG_lBGRA_8888_PRE)) ||
                ((Setting >= VG_sXBGR_8888) && (Setting <= VG_lABGR_8888_PRE)))
            {
                result = VG_HARDWARE_ACCELERATED;
            }
            else
            {
                vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            }
            break;

        case VG_PATH_DATATYPE_QUERY:
            if ((Setting >= VG_PATH_DATATYPE_S_8) &&
                (Setting <= VG_PATH_DATATYPE_F))
            {
                result = VG_HARDWARE_ACCELERATED;
            }
            else
            {
                vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
            }
            break;

        default:
            vgmERROR(VG_ILLEGAL_ARGUMENT_ERROR);
        }
    }
    vgmLEAVEAPI(vgHardwareQuery);

    return result;
}


/*******************************************************************************
**
** veglFlushContext/veglFlush/veglFinish
**
** Context flushing functions.
**
** INPUT:
**
**    Context
**       Pointer to the current context.
**
** OUTPUT:
**
**    Nothing.
*/

static EGLBoolean
veglFlushContext(
    void * Context
    )
{
#if gcdGC355_PROFILER
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR)Context;

    gcoHAL_Flush(context->hal, context->vg, gcvNULL, context->TreeDepth, context->saveLayerTreeDepth, context->varTreeDepth);
    /* Make sure the hardware is idle. */
    gcoHAL_Commit(context->hal, gcvFALSE);
    /* Mark the states as not finished. */
    context->imageDirty = vgvIMAGE_NOT_FINISHED;
#else
    vgFlush();
#endif
    return gcvTRUE;
}

static void
veglFlush(
    void
    )
{
#if gcdGC355_PROFILER
    vgsTHREADDATA_PTR Thread;
    vgsCONTEXT_PTR Context;
    Thread = vgfGetThreadData(gcvFALSE);

    if (Thread == gcvNULL)
    {
        Context = gcvNULL;
        gcoOS_Print("Thread is NULL!");
        return;
    }

    Context = Thread->context;
    if (Context == gcvNULL)
    {
        gcoOS_Print("Conetxt is NULL!");
        return;
    }

    gcoHAL_Flush(Context->hal, Context->vg, gcvNULL, Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth);
    /* Make sure the hardware is idle. */
    gcoHAL_Commit(Context->hal, gcvFALSE);
    /* Mark the states as not finished. */
    Context->imageDirty = vgvIMAGE_NOT_FINISHED;
#else
    vgFlush();
#endif
}

static void
veglFinish(
    void
    )
{
#if gcdGC355_PROFILER
    vgsTHREADDATA_PTR Thread;
    vgsCONTEXT_PTR Context;
    Thread = vgfGetThreadData(gcvFALSE);

    if (Thread == gcvNULL)
    {
        Context = gcvNULL;
        gcoOS_Print("Thread is NULL!");
        return;
    }

    Context = Thread->context;
    if (Context == gcvNULL)
    {
        gcoOS_Print("Conetxt is NULL!");
        return;
    }

    gcoHAL_Flush(Context->hal, Context->vg, gcvNULL, Context->TreeDepth, Context->saveLayerTreeDepth, Context->varTreeDepth);
    /* Make sure the hardware is idle. */
    gcoHAL_Commit(Context->hal, gcvTRUE);
    /* Reset the states. */
    Context->imageDirty = vgvIMAGE_READY;
#else
    vgFinish();
#endif
}

/*******************************************************************************
**
** veglGetClientBuffer
**
** Returns the handle to surface of the specified VG image object.
**
** INPUT:
**
**    Context
**       Pointer to the current context.
**
**    Buffer
**       Valid VG image handle.
**
** OUTPUT:
**
**    Suface handle.
*/

static gcoSURF
veglGetClientBuffer(
    void * Context,
    gctPOINTER Buffer
    )
{
    gcoSURF surface;
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR) Context;

    do
    {
        vgsIMAGE_PTR image;

        /* Check the context pointer. */
        if (context == gcvNULL)
        {
            surface = gcvNULL;
            break;
        }

        /* Validate the path object. */
        if (!vgfVerifyUserObject(context, (VGHandle) Buffer, vgvOBJECTTYPE_IMAGE))
        {
            surface = gcvNULL;
            break;
        }

        /* Cast the object. */
        image = (vgsIMAGE_PTR) Buffer;

        /* Get the surface handle. */
        surface = image->surface;

        if (gcmIS_ERROR(gcoSURF_ReferenceSurface(image->surface)))
        {
            surface = gcvNULL;
            break;
        }
    }
    while (gcvFALSE);

    /* Return the handle. */
    return surface;
}

static EGLBoolean veglQueryHWVG(void)
{
    return EGL_TRUE;
}

/* VG Resolve */
static void veglAppendVGResolve(
    void * Context,
    gcoSURF Target
    )
{
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR) Context;
    gceSTATUS status;
    do
    {
        gcsPOINT src_orig, tgt_orig;

        src_orig.x = src_orig.y = 0;
        tgt_orig.x = tgt_orig.y = 0;

#if gcdGC355_PROFILER
        /* Set target. */
        gcmERR_BREAK(gcoVG_SetTarget(
            context->vg,
            context->TreeDepth,
            context->saveLayerTreeDepth,
            context->varTreeDepth,
            Target,
            gcvORIENTATION_TOP_BOTTOM
            ));
#else
        gcmERR_BREAK(gcoVG_SetTarget(
            context->vg,
            Target,
            gcvORIENTATION_TOP_BOTTOM
            ));
#endif

        /* Update states. */
        gcmERR_BREAK(vgfUpdateStates(
            context,
            gcvVG_IMAGE_NORMAL,
            gcvVG_BLEND_SRC,
            gcvFALSE,
            gcvFALSE,
            gcvFALSE,
            gcvTRUE
            ));

#if gcdGC355_PROFILER
        /* Draw the image. */
        gcmERR_BREAK(gcoVG_DrawImage(
            context->vg,
            context->TreeDepth,
            context->saveLayerTreeDepth,
            context->varTreeDepth,
            context->targetImage.surOrientation,
            context->targetImage.surface,
            &src_orig,
            &tgt_orig,
            &context->targetImage.size,
            0, 0,
            0, 0,
            context->targetImage.size.width,
            context->targetImage.size.height,
            gcvFALSE,
            gcvTRUE
            ));
#else
        gcmERR_BREAK(gcoVG_DrawImage(
            context->vg,
            context->targetImage.surOrientation,
            context->targetImage.surface,
            &src_orig,
            &tgt_orig,
            &context->targetImage.size,
            0, 0,
            0, 0,
            context->targetImage.size.width,
            context->targetImage.size.height,
            gcvFALSE,
            gcvTRUE
            ));
#endif
    }
    while (gcvFALSE);
}

/* Find level-1 children images of an image. */
static int
FindChildImages(
    vgsCONTEXT  *Context,
    vgsIMAGE    *Image,
    VGImage     **Children)
{
    int count = 0;
    int i;
    vgsIMAGE_PTR    image;
    vgsOBJECT_CACHE_PTR objectCache;
    vgsOBJECT_LIST_PTR objectList;
    vgsOBJECT_PTR head;

    gcmHEADER_ARG("context=0x%x image=0x%x children=0x%x",
                  Context, Image, Children);

    /* Count all image children. */
    count = Image->childrenCount;

    /* Get a shortcut to the object cache. */
    objectCache = Context->objectCache;

    /* Get a shortcut to the object list. */
    objectList = &objectCache->cache[Image->object.type];

    /* Capture all image children. */
    if (count > 0 && Children != gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(Context->os, sizeof(VGImage) * count, (gctPOINTER*) *Children)))
        {
            /* out-of-memory */
            gcmFOOTER_ARG("return=%d", count);
            return count;
        }

        count = 0;
        for (i = 0; i < (int)(sizeof(objectList->head) / sizeof(objectList->head[0])); i++)
        {
            head = objectList->head[i];
            while (head != gcvNULL)
            {
                image = (vgsIMAGE_PTR)head;
                if ((image != Image) &&
                    (image->parent == Image))
                {
                    (*Children)[count++] = (VGImage)image;
                }
                head = head->next;
            }
        }
    }

    /* Return the image children count. */
    gcmFOOTER_ARG("return=%d", count);
    return count;
}

/* Find and Count all child images, including child's child's child ... */
static int
FindAllImageDescents(
    vgsCONTEXT  *context,
    vgsIMAGE    *root,
    VGImage     **descents
    )
{
    VGImage     *children = gcvNULL;
    VGImage     *currentSet = gcvNULL;
    vgsIMAGE    *child_obj = gcvNULL;
    int         count = 0;
    int         currCount = 0;
    int         currImage = 0;

    gcmHEADER_ARG("context=0x%x root=0x%x descents=0x%x",
                  context, root, descents);

    /* Find the first level children. */
    count = FindChildImages(context, root, &children);
    if (children == gcvNULL)
    {
        *descents = gcvNULL;
        gcmFOOTER_ARG("return=%d", count);
        return count;
    }

    /* Access all found images to search for their children. */
    currImage = 0;
    do{
        child_obj = (vgsIMAGE_PTR)children[currImage];
        currCount = FindChildImages(context, child_obj, &currentSet);

        /* If this image has children, add them to the children array. */
        if (currCount > 0)
        {
            VGImage *temp;
            if (gcmIS_ERROR(gcoOS_Allocate(context->os,
                                        sizeof(VGImage) * (count + currCount),
                                        (gctPOINTER*) &temp)))
            {
                gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("return=%d", count);
                return count;
            }
            gcoOS_MemCopy(temp, children, sizeof(VGImage) * count);     /*Copy the existing children.*/
            gcoOS_MemCopy(temp + count, currentSet, sizeof(VGImage) * currCount);   /*Copy the newly found children.*/
            gcmOS_SAFE_FREE(context->os, children);
            children = temp;
            count += currCount;
        }

        /* Move to the next image. */
        currImage++;
    }while( currImage < count);

    /* Copy the result and return the count. */
    if (descents != gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(context->os,
                                       sizeof(VGImage) * count,
                                       (gctPOINTER*) descents)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            gcmFOOTER_ARG("return=%d", count);
            return count;
        }

        if (*descents != gcvNULL)
        {
            gcoOS_MemCopy(*descents, children, sizeof(VGImage) * count);
        }
    }
    gcmOS_SAFE_FREE(context->os, children);

    gcmFOOTER_ARG("return=%d", count);
    return count;
}

static EGLenum
veglCreateImageParentImage(
    void * Context,
    unsigned int Image,
    void ** Images,
    int * Count
    )
{
    VGImage         *vgimages = gcvNULL;
    vgsIMAGE        *vgimage_obj = gcvNULL;
    vgsIMAGE        *child_obj = gcvNULL;
    khrEGL_IMAGE    *khImage = gcvNULL;
    int             childCount;
    int             i;
    gctINT32        referenceCount = 0;
    vgsCONTEXT_PTR context = (vgsCONTEXT_PTR) Context;

    gcmHEADER_ARG("Context=%p vgImage=%d Images=0x%x Count=0x%x",
                  Context, Image, Images, Count);

    if (!vgfVerifyUserObject(context, Image, vgvOBJECTTYPE_IMAGE))
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }


    /* Test for VgImage validation. */
    vgimage_obj = (vgsIMAGE_PTR)Image;

    if  ((vgimage_obj == gcvNULL) ||
         (vgimage_obj->parent != vgimage_obj))
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }

    gcoSURF_QueryReferenceCount(vgimage_obj->surface, &referenceCount);
    /* Test if surface is a sibling of any eglImage. */
    if (referenceCount > 1)
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }

    /* Get all child images. */
    childCount = FindAllImageDescents(context, vgimage_obj, &vgimages);

    /* Fill the results. */
    *Count = childCount + 1;
    if (gcmIS_ERROR(gcoOS_Allocate(context->os, sizeof(khrEGL_IMAGE) * (*Count), (gctPOINTER*)Images)))
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ALLOC);
        return EGL_BAD_ALLOC;
    }

    /* Fill the root image. */
    {
        child_obj = (vgsIMAGE_PTR)Image;
        khImage = (khrEGL_IMAGE *) *Images;

        khImage->magic = KHR_EGL_IMAGE_MAGIC_NUM;
        khImage->type  = KHR_IMAGE_VG_IMAGE;
        khImage->surface = vgimage_obj->surface;
        khImage->u.vgimage.texSurface = gcvNULL;
        /*Inc reference to surface. The egl's caller will do this.*/
        /*gcoSURF_ReferenceSurface(khImage->surface);   */
        khImage->u.vgimage.format = (gctUINT)vgimage_obj->format;
        khImage->u.vgimage.allowedQuality = (gctUINT)vgimage_obj->allowedQuality;
        khImage->u.vgimage.dirty = vgimage_obj->imageDirty;
        khImage->u.vgimage.dirtyPtr = (gctINT32_PTR)vgimage_obj->imageDirtyPtr;
        khImage->u.vgimage.rootOffsetX = vgimage_obj->origin.x;
        khImage->u.vgimage.rootOffsetY = vgimage_obj->origin.y;
        khImage->u.vgimage.rootWidth = vgimage_obj->size.width;
        khImage->u.vgimage.rootHeight = vgimage_obj->size.height;

        if (child_obj != gcvNULL)
        {
            khImage->u.vgimage.width = child_obj->size.width;
            khImage->u.vgimage.height = child_obj->size.height;
            khImage->u.vgimage.offset_x = child_obj->origin.x;
            khImage->u.vgimage.offset_y = child_obj->origin.y;
        }
        else
        {
            khImage->u.vgimage.width = 0;
            khImage->u.vgimage.height = 0;
            khImage->u.vgimage.offset_x = 0;
            khImage->u.vgimage.offset_y = 0;
        }
    }

    /* Fill the child images. */
    for (i = 1; i <= childCount; i++)
    {
        child_obj = (vgsIMAGE_PTR)vgimages[i - 1];
        khImage = ((khrEGL_IMAGE *) *Images) + i;

        khImage->magic = KHR_EGL_IMAGE_MAGIC_NUM;
        khImage->type  = KHR_IMAGE_VG_IMAGE;
        khImage->surface = vgimage_obj->surface;
        khImage->u.vgimage.texSurface = vgimage_obj->surface;
        khImage->u.vgimage.format = (gctUINT)vgimage_obj->format;
        khImage->u.vgimage.allowedQuality = (gctUINT)vgimage_obj->allowedQuality;
        khImage->u.vgimage.dirty = vgimage_obj->imageDirty;
        khImage->u.vgimage.dirtyPtr = &khImage->u.vgimage.dirty;
        khImage->u.vgimage.rootWidth = vgimage_obj->size.width;
        khImage->u.vgimage.rootHeight = vgimage_obj->size.height;

        if (child_obj != gcvNULL)
        {
            khImage->u.vgimage.width = child_obj->size.width;
            khImage->u.vgimage.height = child_obj->size.height;
            khImage->u.vgimage.offset_x = child_obj->origin.x;
            khImage->u.vgimage.offset_y = child_obj->origin.y;
        }
        else
        {
            khImage->u.vgimage.width = 0;
            khImage->u.vgimage.height = 0;
            khImage->u.vgimage.offset_x = 0;
            khImage->u.vgimage.offset_y = 0;
        }
    }

    /* Free temporary resources. */
    if (vgimages != gcvNULL)
        gcmVERIFY_OK(gcmOS_SAFE_FREE(context->os, vgimages));

    gcmFOOTER_ARG("return=0x%x", EGL_SUCCESS);
    return EGL_SUCCESS;
}

/*******************************************************************************
**
** vgFlush
**
** The vgFlush function ensures that all outstanding requests on the current
** context will complete in finite time. vgFlush may return prior to the actual
** completion of all requests.
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgFlush(
    void
    )
{
    vgmENTERAPI(vgFlush)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s();\n",
            __FUNCTION__
            );

        gcmVERIFY_OK(vgfFlushPipe(Context, gcvFALSE));

        vgmADVANCE_FRAME();
    }
    vgmLEAVEAPI(vgFlush);
}


/*******************************************************************************
**
** vgFinish
**
** The vgFinish function forces all outstanding requests on the current context
** to complete, returning only when the last request has completed.
**
** INPUT:
**
**    Nothing.
**
** OUTPUT:
**
**    Nothing.
*/

VG_API_CALL void VG_API_ENTRY vgFinish(
    void
    )
{
    vgmENTERAPI(vgFinish)
    {
        gcmTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_PARAMETERS,
            "%s();\n",
            __FUNCTION__
            );

        gcmVERIFY_OK(vgfFlushPipe(Context, gcvTRUE));
    }
    vgmLEAVEAPI(vgFinish);
}

/* Dispatch table. */
#ifdef _WIN32
VG_API_CALL
#endif
veglDISPATCH
OpenVG_DISPATCH_TABLE =
{
    /* createContext            */  veglCreateContextEx,
    /* destroyContext           */  veglDestroyContext,
    /* makeCurrent              */  veglSetContext,
    /* loseCurrent              */  veglUnsetContext,
    /* setDrawable              */  veglSetContext,
    /* flushContext             */  veglFlushContext,

    /* flush                    */  veglFlush,
    /* finish                   */  veglFinish,

    /* setBuffer                */  veglSetBuffer,
    /* getClientBuffer          */  veglGetClientBuffer,

    /* createImageTexture       */  gcvNULL,
    /* createImageRenderbuffer  */  gcvNULL,
    /* createImageParentImage   */  veglCreateImageParentImage,
    /* bindTexImage             */  gcvNULL,

    /* profiler                 */  gcvNULL,
    /* getProcAddr;             */  gcvNULL,

    /* queryHWVG                */  veglQueryHWVG,

    /* renderThread             */  gcvNULL,

    /* swapbuffers              */  gcvNULL,
    /* VG Resolve */                veglAppendVGResolve,

    /* syncImage                */  gcvNULL,
};

/* Make sure the maskImage is ready to use. */
gceSTATUS vgfGetMaskImage(
    vgsCONTEXT_PTR Context)
{
    gceSTATUS   status = gcvSTATUS_OK;

    vgmENTERSUBAPI(vgfGetMaskImage);
    do
    {
        /* If nothing changed and mask exists, just do nothing. */
        if ((Context->maskImage != gcvNULL) &&
            !Context->targetChanged)
        {
            break;
        }

        /* Destroy the current maskImage if existing. */
        if (Context->maskImage != gcvNULL)
        {
            /* First, unset the mask target. */
            if (Context->maskImage->surface != gcvNULL)
            {
#if gcdGC355_PROFILER
                /* Unset the current mask. */
                gcmERR_BREAK(gcoVG_UnsetMask(
                    Context->vg,
                    Context->TreeDepth,
                    Context->saveLayerTreeDepth,
                    Context->varTreeDepth,
                    Context->maskImage->surface
                    ));
#else
                gcmERR_BREAK(gcoVG_UnsetMask(
                    Context->vg,
                    Context->maskImage->surface
                    ));
#endif
            }

            /* Then, derefer the maskImage object. */
            vgfDereferenceObject(Context, (vgsOBJECT_PTR*)&Context->maskImage);

            /* Set NULL. Here its counter should be 0 and the object is freed. */
            Context->maskImage = gcvNULL;
        }

        gcmERR_BREAK(vgfCreateImage(
            Context,
            VG_A_8,
            Context->targetImage.size.width,
            Context->targetImage.size.height,
            vgvIMAGE_QUALITY_ALL,
            &Context->maskImage,
            gcvNULL
            ));

        /* Set default mask. */
        vgfFillColor(
            Context,
            Context->maskImage,
            0, 0,
            Context->targetImage.size.width,
            Context->targetImage.size.height,
            vgvFLOATCOLOR0001,
            vgvBYTECOLOR0001,
            gcvFALSE
            );

#if gcdGC355_PROFILER
        /* By default the mask is disabled. */
        gcmERR_BREAK(gcoVG_EnableMask(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            gcvFALSE
            ));
#else
        gcmERR_BREAK(gcoVG_EnableMask(
            Context->vg,
            gcvFALSE
            ));
#endif

#if gcdGC355_PROFILER
        /* Set new mask. */
        gcmERR_BREAK(gcoVG_SetMask(
            Context->vg,
            Context->TreeDepth,
            Context->saveLayerTreeDepth,
            Context->varTreeDepth,
            Context->maskImage->surface
            ));
#else
        gcmERR_BREAK(gcoVG_SetMask(
            Context->vg,
            Context->maskImage->surface
            ));
#endif

        Context->targetChanged = gcvFALSE;
    } while (0);

    vgmLEAVESUBAPI(vgfGetMaskImage);
    return status;
}
