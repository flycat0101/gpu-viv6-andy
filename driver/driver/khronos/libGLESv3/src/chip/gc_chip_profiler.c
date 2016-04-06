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
#include "gc_hal_user.h"
#include "gc_chip_context.h"


#define _GC_OBJ_ZONE    __GLES3_ZONE_PROFILER


extern GLint __glesApiProfileMode;

#if VIVANTE_PROFILER

#if VIVANTE_PROFILER_CONTEXT
#define GLFFPROFILER_HAL (CHIP_CTXINFO(Context))->phal
#else
#define GLFFPROFILER_HAL (CHIP_CTXINFO(Context))->hal
#endif

gctBOOL __glesIsSyncMode = gcvTRUE;

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

#if VIVANTE_PROFILER_PROBE
#define gcmWRITE_XML_STRING(String) \
    do \
        { \
    gceSTATUS status; \
    gctINT32 length; \
    length = (gctINT32) gcoOS_StrLen((gctSTRING) String, gcvNULL); \
    gcmERR_BREAK(gcoPROFILER_Write(GLFFPROFILER_HAL, length, String)); \
        } \
            while (gcvFALSE)

#define gcmWRITE_XML_Value(Counter,Value,Dec) \
    do \
        { \
    char buffer[256]; \
    gctUINT offset = 0; \
    gceSTATUS status; \
    gcmERR_BREAK(gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), \
    &offset, \
    Dec?"<%s>%d</%s>\n":"<%s>%x</%s>\n", \
    Counter, \
    Value, \
    Counter)); \
    gcmWRITE_XML_STRING(buffer); \
        } \
            while (gcvFALSE)
#endif

#define PRO_NODE_SIZE 8
typedef struct _program_list{
    gctUINT32 program_id[PRO_NODE_SIZE];
    gctUINT8  dirty_flag[PRO_NODE_SIZE];
    struct _program_list * next;
} program_list;

program_list *PGM;

/*******************************************************************************
**    _pro_modify
**
**    Set program is changed or not in user API level.
**
*/
/*
static void _pro_modify(gctUINT32 program){
    gctINT ii;
    program_list *pPGM;

    if(PGM == gcvNULL){ return;}
    pPGM = PGM;
    while(pPGM != gcvNULL)
    {
        for(ii = 0; ii < PRO_NODE_SIZE; ii ++)
        {
             if(pPGM->program_id[ii] == program)
             {
                 pPGM->dirty_flag[ii] = 1;
                 return;
             }
             if(pPGM->program_id[ii] == 0)
                 return;
        }
        if(ii == PRO_NODE_SIZE)
            pPGM =pPGM->next;
    }
    return;
}*/

/*******************************************************************************
**    _pro_dirty
**
**    Check program is changed or not in user API level.
**
**        gctUINT32 program
**            Pointer value of a program object.
*/
static gctBOOL _pro_dirty(gctUINT32 program){
    program_list *pPGM = gcvNULL,*pPrvPGM = gcvNULL;
    program_list *newpPGM = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctINT ii;
    if (PGM == gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,sizeof(program_list),&pointer)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            return gcvFALSE;
        }

        PGM = pointer;
        PGM->next=gcvNULL;
        gcoOS_ZeroMemory(PGM->program_id, PRO_NODE_SIZE * gcmSIZEOF(gctUINT32));
        PGM->program_id[0] = program;PGM->dirty_flag[0] = 0;
        return gcvTRUE;
    }
    pPGM = PGM;pPrvPGM = PGM;
    while (pPGM != gcvNULL)
    {
        for (ii = 0; ii < PRO_NODE_SIZE; ii ++)
        {
            if (pPGM->program_id[ii] == program && pPGM->dirty_flag[ii] == 1) {pPGM->dirty_flag[ii] = 0 ;return gcvTRUE;}
            if (pPGM->program_id[ii] == program && pPGM->dirty_flag[ii] == 0) {pPGM->dirty_flag[ii] = 0 ;return gcvFALSE;}
            if (pPGM->program_id[ii] == 0) { pPGM->program_id[ii] = program;pPGM->dirty_flag[ii] = 0; return gcvTRUE;}
        }
        pPrvPGM = pPGM;
        pPGM =pPGM->next;
    }

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(program_list),&pointer)))
    {
        gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
        return gcvFALSE;
    }
    newpPGM = pointer;
    pPrvPGM->next = newpPGM;
    newpPGM->next=gcvNULL;
    gcoOS_ZeroMemory(newpPGM->program_id,PRO_NODE_SIZE * gcmSIZEOF(gctUINT32));
    newpPGM->program_id[0] = program;
    newpPGM->dirty_flag[0]=0;

    return gcvTRUE;
}
/*******************************************************************************
**    _pro_destroy
**
**    Free memory data in each frame.
**
*/
static void _pro_destroy(){
    program_list *pPGM,*freenode;
    pPGM=PGM;
    while(pPGM != gcvNULL)
    {
        freenode= pPGM;
        pPGM=pPGM->next;
        gcoOS_Free(gcvNULL,freenode);
    }
    PGM=gcvNULL;
    return;
}

/*******************************************************************************
**    __glInitializeProfiler
**
**    Initialize the profiler for the context provided.
**
**    Arguments:
**
**        GLContext Context
**            Pointer to a new GLContext object.
*/
void
gcChipInitializeProfiler(
    IN __GLcontext *Context
    )
{
    gceSTATUS status;
    gctUINT rev;
    char *env = gcvNULL;
    __GLchipContext *chipCtx = CHIP_CTXINFO(Context);

    if (__glesApiProfileMode == -1)
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
                return;
            }
            gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
            GLFFPROFILER_HAL = (gcoHAL) pointer;
            gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvFALSE);
        }
#endif
        return;
    }

    if (__glesApiProfileMode == 0)
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
                return;
            }
            gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
            GLFFPROFILER_HAL = (gcoHAL) pointer;
            gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvFALSE);
        }
#else
        gcoPROFILER_Initialize(gcvNULL, gcvFALSE);
#endif
#endif
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
            return;
        }
        gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
        GLFFPROFILER_HAL = (gcoHAL) pointer;
    }
#endif

    status = gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvTRUE);

    switch (status)
    {
        case gcvSTATUS_OK:
            break;
        case gcvSTATUS_MISMATCH:
        case gcvSTATUS_NOT_SUPPORTED:/*fall through*/
        default:
            Context->profiler.enable = gcvFALSE;
#if VIVANTE_PROFILER_CONTEXT
            if(GLFFPROFILER_HAL != gcvNULL)
                gcoOS_Free(gcvNULL,GLFFPROFILER_HAL);
#endif
            return;
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&Context->profiler, gcmSIZEOF(Context->profiler));

    Context->profiler.enable = gcvTRUE;
    if (__glesApiProfileMode == 1)
    {
        Context->profiler.enableOutputCounters = gcvTRUE;
    }
    else if (__glesApiProfileMode == 2 || __glesApiProfileMode == 3)
    {
        Context->profiler.enableOutputCounters = gcvFALSE;
    }

    gcoOS_GetEnv(gcvNULL, "VP_SYNC_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
    {
        __glesIsSyncMode = gcvFALSE;
    }

    gcoOS_GetEnv(gcvNULL, "VP_COUNTER_FILTER", &env);
    if ((env == gcvNULL) || (env[0] ==0))
    {
        Context->profiler.timeEnable =
            Context->profiler.drvEnable =
            Context->profiler.memEnable =
            Context->profiler.progEnable = gcvTRUE;
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
        if (bitsLen > 6)
        {
            Context->profiler.drvEnable = (env[6] == '1');
        }
        else
        {
            Context->profiler.drvEnable = gcvTRUE;
        }
        if (bitsLen > 1)
        {
            Context->profiler.memEnable = (env[1] == '1');
        }
        else
        {
            Context->profiler.memEnable = gcvTRUE;
        }
        if (bitsLen > 7)
        {
            Context->profiler.progEnable = (env[7] == '1');
        }
        else
        {
            Context->profiler.progEnable = gcvTRUE;
        }
    }

    Context->profiler.frameCount = 0;
    Context->profiler.frameStartNumber = 0;
    Context->profiler.frameEndNumber = 0;
    Context->profiler.drawCount = 0;
    Context->profiler.perDraw = gcvFALSE;
    Context->profiler.perFrame = gcvFALSE;
    Context->profiler.useFBO = gcvFALSE;

    Context->profiler.useGlfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_GLFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        Context->profiler.useGlfinish = gcvTRUE;
    }

    if (__glesApiProfileMode == 1)
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

    if (__glesApiProfileMode == 3)
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

    {
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char* infoRenderer = Context->constants.renderer;
        char* infoDriver = "OpenGL ES 3.0";
        gctUINT offset = 0;
        rev = chipCtx->chipRevision;
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

    gcoOS_GetTime(&Context->profiler.frameStart);
    Context->profiler.frameStartTimeusec     = Context->profiler.frameStart;
    Context->profiler.primitiveStartTimeusec = Context->profiler.frameStart;
    gcoOS_GetCPUTime(&Context->profiler.frameStartCPUTimeusec);
}

/*******************************************************************************
**    __glDestroyProfiler
**
**    Initialize the profiler for the context provided.
**
**    Arguments:
**
**        GLContext Context
**            Pointer to a new GLContext object.
*/
void
gcChipDestroyProfiler(
    IN __GLcontext *Context
    )
{
    if (Context->profiler.enable)
    {
        _pro_destroy();
        Context->profiler.enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(GLFFPROFILER_HAL));
#if VIVANTE_PROFILER_CONTEXT
        if(GLFFPROFILER_HAL != gcvNULL)
            gcoOS_Free(gcvNULL, GLFFPROFILER_HAL);
#endif
    }
    else
    {
#if VIVANTE_PROFILER_PROBE
        gcmVERIFY_OK(gcoPROFILER_Destroy(GLFFPROFILER_HAL));
#if VIVANTE_PROFILER_CONTEXT
        if (GLFFPROFILER_HAL != gcvNULL)
            gcoOS_Free(gcvNULL, GLFFPROFILER_HAL);
#endif
#endif
    }
}

/* Function for printing frame number only once */
static void
beginFrame(
    IN __GLcontext *Context
    )
{
    if (Context->profiler.enable)
    {
        if (!Context->profiler.frameBegun)
        {
            /* write screen size */
             if (Context->profiler.frameNumber == 0 && Context->drawablePrivate)
             {
                 gctUINT offset = 0;
                 char  infoScreen[255] = {'\0'};
                 gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
                 gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),
                         &offset, "%d x %d", Context->drawablePrivate->width, Context->drawablePrivate->height);
                 gcmWRITE_CONST(VPC_INFOSCREENSIZE);
                 gcmWRITE_STRING(infoScreen);

                 gcmWRITE_CONST(VPG_END);
             }

            gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);

            Context->profiler.frameBegun = 1;
        }
    }
    else
    {
#if VIVANTE_PROFILER_PROBE
        if (!Context->profiler.frameBegun)
        {
            /* write screen size */
            if (Context->profiler.frameNumber == 0 && Context->drawablePrivate)
            {
                gceCHIPMODEL chipModel;
                gctUINT32 chipRevision;
                gcmVERIFY_OK(gcoHAL_QueryChipIdentity(gcvNULL, &chipModel, &chipRevision, gcvNULL, gcvNULL));
                gcmWRITE_XML_STRING("<DrawCounter>\n");
                gcmWRITE_XML_STRING("\t"); gcmWRITE_XML_Value("ChipID", chipModel, 0);
                gcmWRITE_XML_STRING("\t"); gcmWRITE_XML_Value("ChipRevision", chipRevision, 0);
            }

            Context->profiler.frameBegun = 1;
        }
#endif
    }
}

static void
endFrame(
    IN __GLcontext *Context
    )
{
    int i;
    gctUINT32 totalCalls = 0;
    gctUINT32 totalDrawCalls = 0;
    gctUINT32 totalStateChangeCalls = 0;
    gctUINT32 maxrss, ixrss, idrss, isrss;

    if (Context->profiler.enable)
    {
        beginFrame(Context);

        if (Context->profiler.timeEnable)
        {
            gcmWRITE_CONST(VPG_TIME);

            gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32) (Context->profiler.frameEndTimeusec
                - Context->profiler.frameStartTimeusec));
            gcmWRITE_COUNTER(VPC_CPUTIME, (gctINT32) Context->profiler.totalDriverTime);

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
            gcmWRITE_CONST(VPG_ES30_TIME);
            for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES30_TIME + 1 + i, (gctINT32) Context->profiler.apiTimes[i]);
                }
            }
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_ES30);


            for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
            {
                if (i >= VPC_ES30CALLS && i <= VPC_ES30TRIANGLECOUNT)
                    break;

                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES30 + 1 + i, Context->profiler.apiCalls[i]);

                    totalCalls += Context->profiler.apiCalls[i];

                    switch (GLES3_APICALLBASE + i)
                    {
                    case GLES3_DRAWARRAYS:
                    case GLES3_DRAWELEMENTS:
                    case GLES3_DRAWRANGEELEMENTS:
                    case GLES3_DRAWARRAYSINSTANCED:
                    case GLES3_DRAWELEMENTSINSTANCED:
                    case GLES3_MULTIDRAWARRAYSEXT:
                    case GLES3_MULTIDRAWELEMENTSEXT:
                        totalDrawCalls += Context->profiler.apiCalls[i];
                        break;

                    /* TODO: deal with es3.0 APIs */
                    case GLES3_ATTACHSHADER:
                    case GLES3_BLENDCOLOR:
                    case GLES3_BLENDEQUATION:
                    case GLES3_BLENDEQUATIONSEPARATE:
                    case GLES3_BLENDFUNC:
                    case GLES3_BLENDFUNCSEPARATE:
                    case GLES3_CLEARCOLOR:
                    case GLES3_COLORMASK:
                    case GLES3_DEPTHFUNC:
                    case GLES3_DEPTHMASK:
                    case GLES3_DEPTHRANGEF:
                    case GLES3_STENCILFUNC:
                    case GLES3_STENCILFUNCSEPARATE:
                    case GLES3_STENCILMASK:
                    case GLES3_STENCILMASKSEPARATE:
                    case GLES3_STENCILOP:
                    case GLES3_STENCILOPSEPARATE:
                    case GLES3_UNIFORM1F:
                    case GLES3_UNIFORM1FV:
                    case GLES3_UNIFORM1I:
                    case GLES3_UNIFORM1IV:
                    case GLES3_UNIFORM2F:
                    case GLES3_UNIFORM2FV:
                    case GLES3_UNIFORM2I:
                    case GLES3_UNIFORM2IV:
                    case GLES3_UNIFORM3F:
                    case GLES3_UNIFORM3FV:
                    case GLES3_UNIFORM3I:
                    case GLES3_UNIFORM3IV:
                    case GLES3_UNIFORM4F:
                    case GLES3_UNIFORM4FV:
                    case GLES3_UNIFORM4I:
                    case GLES3_UNIFORM4IV:
                    case GLES3_UNIFORMMATRIX2FV:
                    case GLES3_UNIFORMMATRIX3FV:
                    case GLES3_UNIFORMMATRIX4FV:
                    case GLES3_USEPROGRAM:
                    case GLES3_CULLFACE:
                    case GLES3_DISABLE:
                    case GLES3_ENABLE:
                    case GLES3_DISABLEVERTEXATTRIBARRAY:
                    case GLES3_ENABLEVERTEXATTRIBARRAY:
                    case GLES3_FRONTFACE:
                    case GLES3_HINT:
                    case GLES3_LINEWIDTH:
                    case GLES3_PIXELSTOREI:
                    case GLES3_POLYGONOFFSET:
                    case GLES3_SAMPLECOVERAGE:
                    case GLES3_SCISSOR:
                    case GLES3_TEXIMAGE2D:
                    case GLES3_TEXPARAMETERF:
                    case GLES3_TEXPARAMETERFV:
                    case GLES3_TEXPARAMETERI:
                    case GLES3_TEXPARAMETERIV:
                    case GLES3_TEXSUBIMAGE2D:
                    case GLES3_VERTEXATTRIB1F:
                    case GLES3_VERTEXATTRIB1FV:
                    case GLES3_VERTEXATTRIB2F:
                    case GLES3_VERTEXATTRIB2FV:
                    case GLES3_VERTEXATTRIB3F:
                    case GLES3_VERTEXATTRIB3FV:
                    case GLES3_VERTEXATTRIB4F:
                    case GLES3_VERTEXATTRIB4FV:
                    case GLES3_VERTEXATTRIBPOINTER:
                    case GLES3_VIEWPORT:
                    case GLES3_BINDATTRIBLOCATION:
                    case GLES3_BINDBUFFER:
                    case GLES3_BINDFRAMEBUFFER:
                    case GLES3_BINDRENDERBUFFER:
                    case GLES3_BINDTEXTURE:
                    case GLES3_COMPRESSEDTEXIMAGE2D:
                    case GLES3_COMPRESSEDTEXSUBIMAGE2D:
                    case GLES3_DETACHSHADER:
                    case GLES3_SHADERBINARY:
                    case GLES3_SHADERSOURCE:
                    case GLES3_BUFFERDATA:
                    case GLES3_BUFFERSUBDATA:
                    case GLES3_CREATEPROGRAM:
                    case GLES3_CREATESHADER:
                    case GLES3_DELETEBUFFERS:
                    case GLES3_DELETEFRAMEBUFFERS:
                    case GLES3_DELETEPROGRAM:
                    case GLES3_DELETERENDERBUFFERS:
                    case GLES3_DELETESHADER:
                    case GLES3_DELETETEXTURES:
                    case GLES3_FRAMEBUFFERRENDERBUFFER:
                    case GLES3_FRAMEBUFFERTEXTURE2D:
                    case GLES3_GENBUFFERS:
                    case GLES3_GENERATEMIPMAP:
                    case GLES3_GENFRAMEBUFFERS:
                    case GLES3_GENRENDERBUFFERS:
                    case GLES3_GENTEXTURES:
                    case GLES3_RELEASESHADERCOMPILER:
                    case GLES3_RENDERBUFFERSTORAGE:
                    case GLES3_PROGRAMBINARYOES:
                        totalStateChangeCalls += Context->profiler.apiCalls[i];
                        break;

                    default:
                        break;
                    }
                }

                /* Clear variables for next frame. */
                Context->profiler.apiCalls[i] = 0;
                Context->profiler.apiTimes[i] = 0;
            }

            gcmWRITE_COUNTER(VPC_ES30CALLS, totalCalls);
            gcmWRITE_COUNTER(VPC_ES30DRAWCALLS, totalDrawCalls);
            gcmWRITE_COUNTER(VPC_ES30STATECHANGECALLS, totalStateChangeCalls);

            gcmWRITE_COUNTER(VPC_ES30POINTCOUNT, Context->profiler.drawPointCount);
            gcmWRITE_COUNTER(VPC_ES30LINECOUNT, Context->profiler.drawLineCount);
            gcmWRITE_COUNTER(VPC_ES30TRIANGLECOUNT, Context->profiler.drawTriangleCount);
            gcmWRITE_CONST(VPG_END);

        }

        gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
        gcmWRITE_CONST(VPG_END);

        gcoPROFILER_Flush(GLFFPROFILER_HAL);

        /* Clear variables for next frame. */
        Context->profiler.drawPointCount      = 0;
        Context->profiler.drawLineCount       = 0;
        Context->profiler.drawTriangleCount   = 0;
        Context->profiler.textureUploadSize   = 0;

        Context->profiler.shaderCompileTime   = 0;
        Context->profiler.shaderStartTimeusec = 0;
        Context->profiler.shaderEndTimeusec   = 0;

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
        beginFrame(Context);
        gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
        gcoPROFILER_Flush(GLFFPROFILER_HAL);

        /* Next frame. */
        Context->profiler.frameNumber++;
        Context->profiler.frameBegun = 0;
    }
#endif
}

#if !VIVANTE_PROFILER_PROBE
static void
resetFrameData(
    IN __GLcontext *Context
    )
{
    gctUINT32 i;

    GLFFPROFILER_HAL->profiler.disableOutputCounter = gcvTRUE;
    gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
    GLFFPROFILER_HAL->profiler.disableOutputCounter = gcvFALSE;

    for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
    {
        if (i >= VPC_ES30CALLS && i <= VPC_ES30TRIANGLECOUNT)
            break;
        Context->profiler.apiCalls[i] = 0;
        Context->profiler.apiTimes[i] = 0;
        Context->profiler.totalDriverTime = 0;
    }
    /* Clear variables for next frame. */
    Context->profiler.drawPointCount      = 0;
    Context->profiler.drawLineCount       = 0;
    Context->profiler.drawTriangleCount   = 0;
    Context->profiler.drawVertexCount     = 0;
    Context->profiler.textureUploadSize   = 0;
    Context->profiler.drawCount = 0;

    Context->profiler.shaderCompileTime   = 0;
    Context->profiler.shaderStartTimeusec = 0;
    Context->profiler.shaderEndTimeusec   = 0;

    Context->profiler.totalDriverTime = 0;

    if (Context->profiler.timeEnable)
    {
        Context->profiler.frameStartCPUTimeusec =
            Context->profiler.frameEndCPUTimeusec;
        gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
    }
}
#endif

/* Function for printing draw number only once */
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
static void beginDraw(IN __GLcontext *Context)
 {
#if VIVANTE_PROFILER_PERDRAW
     if (Context->profiler.enable)
     {
         gcmWRITE_CONST(VPG_ES30_DRAW);
         gcmWRITE_COUNTER(VPC_ES30_DRAW_NO, Context->profiler.drawCount);
     }
#endif
     gcoPROFILER_BeginDraw(GLFFPROFILER_HAL);
}

static void endDraw(IN __GLcontext *Context){
    gcoPROFILER_EndDraw(GLFFPROFILER_HAL, (gctBOOL)(Context->profiler.drawCount == 0));
}
#endif


#define gcmCHECK_VP_MODE(need_dump) \
    do \
    { \
        switch (__glesApiProfileMode) \
        { \
        case 1: \
            if (Context->profiler.frameCount == 0 || (Context->profiler.frameNumber < Context->profiler.frameCount)) \
                need_dump = gcvTRUE; \
            break; \
        case 2: \
            if (Context->profiler.enableOutputCounters ) \
                need_dump = gcvTRUE; \
            else \
                need_dump = gcvFALSE; \
            break; \
        case 3: \
            if (Context->profiler.frameStartNumber == 0 && Context->profiler.frameEndNumber == 0) \
            { \
                need_dump = gcvTRUE; \
                break; \
            } \
            if (cur_frame_num >= Context->profiler.frameStartNumber && cur_frame_num <= Context->profiler.frameEndNumber) \
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

GLboolean
__glChipProfiler(
    IN gctPOINTER Profiler,
    IN GLuint Enum,
    IN gctHANDLE Value
    )
{
    __GLcontext *Context = __glGetGLcontext();
    static gctBOOL dump_program = gcvFALSE;
    static gctUINT cur_frame_num = 0;
    gctBOOL need_dump = gcvFALSE;

    GL_ASSERT(Context);
#if !VIVANTE_PROFILER_PROBE
    if (! Context->profiler.enable)
    {
        return GL_FALSE;
    }
#endif

    switch (Enum)
    {
    case GL3_PROFILER_FRAME_END:
        if (Context->profiler.useGlfinish && gcmPTR2INT32(Value) != 1)
            break;

        if (Context->profiler.timeEnable)
        {
            gcoOS_GetTime(&Context->profiler.frameEndTimeusec);
            gcoOS_GetCPUTime(&Context->profiler.frameEndCPUTimeusec);
        }
        Context->profiler.perFrame = gcvTRUE;
        Context->profiler.perDraw = gcvFALSE;
        Context->profiler.drawCount = 0;
#if VIVANTE_PROFILER_PROBE
        cur_frame_num++;
        endFrame(Context);
        break;
#else
        gcmCHECK_VP_MODE(need_dump);
        cur_frame_num++;
        if (need_dump)
            endFrame(Context);
        else
            resetFrameData(Context);
        break;
#endif
    case GL3_PROFILER_PRIMITIVE_TYPE:
        if (!Context->profiler.drvEnable)
            break;
        Context->profiler.primitiveType = gcmPTR2INT32(Value);
        break;
    case GL3_PROFILER_PRIMITIVE_COUNT:
        if (!Context->profiler.drvEnable)
            break;
        Context->profiler.primitiveCount = gcmPTR2INT32(Value);
        switch (Context->profiler.primitiveType)
        {
        case GL_POINTS:
            Context->profiler.drawPointCount += gcmPTR2INT32(Value);
            break;

        case GL_LINES:
        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            Context->profiler.drawLineCount += gcmPTR2INT32(Value);
            break;

        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            Context->profiler.drawTriangleCount += gcmPTR2INT32(Value);
            break;
        }
        break;
    case GL3_PROFILER_DRAW_BEGIN:
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
        if(Context->profiler.perDraw == gcvFALSE)
        {
            Context->profiler.perDraw = gcvTRUE;
            Context->profiler.perFrame = gcvFALSE;
            Context->profiler.drawCount = 0;
        }
#if VIVANTE_PROFILER_PERDRAW
        beginFrame(Context);
#elif defined VIVANTE_PROFILER_PROBE
        beginDraw(Context);
#endif
#endif
        break;
    case GL3_PROFILER_DRAW_END:
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
        endDraw(Context);
        Context->profiler.drawCount++;
#endif
        break;
    /* Print program info immediately as we do not save it. */
    case GL3_PROGRAM_IN_USE_BEGIN:
        gcmCHECK_VP_MODE(need_dump);
        if (!Context->profiler.progEnable || !need_dump)
            break;
        dump_program = _pro_dirty(gcmPTR2INT32(Value));
        beginFrame(Context);
        gcmWRITE_CONST(VPG_PROG);
        gcmWRITE_COUNTER(VPC_PROGRAMHANDLE, gcmPTR2INT32(Value));
        break;

    case GL3_PROGRAM_IN_USE_END:
        gcmCHECK_VP_MODE(need_dump);
        if (!Context->profiler.progEnable || !need_dump)
            break;
        gcmWRITE_CONST(VPG_END);
        break;

    case GL3_PROGRAM_VERTEX_SHADER:
        gcmCHECK_VP_MODE(need_dump);
        if (!Context->profiler.progEnable || !need_dump)
            break;
        if (dump_program) gcoPROFILER_ShaderVS(GLFFPROFILER_HAL, (gcSHADER)Value);
        break;

    case GL3_PROGRAM_FRAGMENT_SHADER:
        gcmCHECK_VP_MODE(need_dump);
        if (!Context->profiler.progEnable || !need_dump)
            break;
        if (dump_program) gcoPROFILER_ShaderFS(GLFFPROFILER_HAL, (gcSHADER)Value);
        break;
    case GL3_PROFILER_SYNC_MODE:
        return (GLboolean)__glesIsSyncMode;
    default:
        break;
    }
    return GL_TRUE;
}


#define __GLCHIP_PROFILER_HEADER()
#define __GLCHIP_PROFILER_FOOTER()


GLboolean
__glChipProfile_MakeCurrent(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipMakeCurrent(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_LoseCurrent(
    __GLcontext *gc,
    GLboolean bkickoffcmd
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipLoseCurrent(gc, bkickoffcmd);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DestroyContext(
    __GLcontext *gc
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();
    ret = __glChipDestroyContext(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_QueryFormatInfo(
    __GLcontext *gc,
    __GLformat drvformat,
    GLint *numSamples,
    GLint *samples,
    GLint bufsize
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipQueryFormatInfo(gc, drvformat, numSamples, samples, bufsize);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_ReadPixelsBegin(
    __GLcontext *gc
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();
     ret = __glChipReadPixelsBegin(gc);
     __GLCHIP_PROFILER_FOOTER();
     return ret;
}

GLvoid
__glChipProfile_ReadPixelsValidateState(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipReadPixelsValidateState(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_ReadPixelsEnd(
    __GLcontext *gc
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();
    ret = __glChipReadPixelsEnd(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ReadPixels(
    __GLcontext *gc,
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLenum type,
    GLubyte *buf
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();
    ret = __glChipReadPixels(gc, x, y, width, height, format, type, buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DrawBegin(
    __GLcontext* gc,
    GLenum mode
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();

    ret = __glChipDrawBegin(gc, mode);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DrawValidateState(
    __GLcontext *gc
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();

    ret = __glChipDrawValidateState(gc);

    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DrawEnd(
    __GLcontext *gc
    )
{
    GLboolean ret;

    __GLCHIP_PROFILER_HEADER();

    ret = __glChipDrawEnd(gc);

    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_Flush(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();

    ret = __glChipFlush(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_Finish(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipFinish(gc);

    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_BindTexture(
    __GLcontext* gc,
    __GLtextureObject *texObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBindTexture(gc, texObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_DeleteTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteTexture(gc, texObj);
    __GLCHIP_PROFILER_FOOTER();
}


GLvoid
__glChipProfile_DetachTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDetachTexture(gc, texObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_TexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexImage2D(gc, texObj, face, level, buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid *buf
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexImage3D(gc, texObj, level, buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexSubImage2D(
        __GLcontext *gc,
        __GLtextureObject *texObj,
        GLint face,
        GLint level,
        GLint xoffset,
        GLint yoffset,
        GLint width,
        GLint height,
        const GLvoid* buf
        )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexSubImage2D(gc, texObj, face, level, xoffset, yoffset, width, height, buf);

    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid* buf
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexSubImage3D(gc,
                                 texObj,
                                 level,
                                 xoffset,
                                 yoffset,
                                 zoffset,
                                 width,
                                 height,
                                 depth,
                                 buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CopyTexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x,
    GLint y
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyTexImage2D(gc, texObj, face, level, x, y);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CopyTexSubImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x, GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyTexSubImage2D(gc,
                                     texObj,
                                     face,
                                     level,
                                     x,
                                     y,
                                     width,
                                     height,
                                     xoffset,
                                     yoffset);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CopyTexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint x,
    GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyTexSubImage3D(gc,
                                     texObj,
                                     level,
                                     x,
                                     y,
                                     width,
                                     height,
                                     xoffset,
                                     yoffset,
                                     zoffset);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CompressedTexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCompressedTexImage2D(gc, texObj, face, level, buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CompressedTexSubImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint width,
    GLint height,
    const GLvoid *buf,
    GLsizei size
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCompressedTexSubImage2D(gc,
                                           texObj,
                                           face,
                                           level,
                                           xoffset,
                                           yoffset,
                                           width,
                                           height,
                                           buf,
                                           size);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CompressedTexImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid *buf
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCompressedTexImage3D(gc, texObj, level, buf);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CompressedTexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid *buf,
    GLsizei size
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCompressedTexSubImage3D(gc,
                                           texObj,
                                           level,
                                           xoffset,
                                           yoffset,
                                           zoffset,
                                           width,
                                           height,
                                           depth,
                                           buf,
                                           size);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GenerateMipMap(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint faces,
    GLint *maxLevel
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGenerateMipMap(gc, texObj, faces, maxLevel);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CopyTexBegin(
    __GLcontext* gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyTexBegin(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_CopyTexValidateState(
    __GLcontext* gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipCopyTexValidateState(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_CopyTexEnd(
    __GLcontext* gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipCopyTexEnd(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_CopyImageSubData(
    __GLcontext *gc,
    GLvoid * srcObject,
    GLint srcType,
    GLint srcLevel,
    GLint srcX,
    GLint srcY,
    GLint srcZ,
    GLvoid * dstObject,
    GLint dstType,
    GLint dstLevel,
    GLint dstX,
    GLint dstY,
    GLint dstZ,
    GLsizei width,
    GLsizei height,
    GLsizei depth
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyImageSubData(gc, srcObject, srcType, srcLevel, srcX, srcY, srcZ,
                                       dstObject, dstType, dstLevel, dstX, dstY, dstZ,
                                       width, height, depth);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BindTexImage(
    IN  __GLcontext *gc,
    IN  __GLtextureObject *texObj,
    IN  GLint level,
    IN  void * surface,
    OUT void ** pBinder
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBindTexImage(gc, texObj, level, surface, pBinder);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_FreeTexImage(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipFreeTexImage(gc, texObj, face, level);
    __GLCHIP_PROFILER_FOOTER();
}

GLenum
__glChipProfile_CreateEglImageTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint depth,
    GLvoid * image
    )
{
    GLenum ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCreateEglImageTexture(gc, texObj, face, level, depth, image);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLenum
__glChipProfile_CreateEglImageRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo, GLvoid *image
    )
{
    GLenum ret;
    __GLCHIP_PROFILER_HEADER();
    ret =
__glChipCreateEglImageRenderbuffer(gc, rbo, image);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_EglImageTargetTexture2DOES(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLvoid *eglImage
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipEglImageTargetTexture2DOES(gc, texObj, target, eglImage);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_EglImageTargetRenderbufferStorageOES(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo,
    GLenum target,
    GLvoid *eglImage
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipEglImageTargetRenderbufferStorageOES(gc, rbo, target, eglImage);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GetTextureAttribFromImage(
    __GLcontext     *gc,
    GLvoid          *eglImage,
    GLint           *width,
    GLint           *height,
    GLint           *stride,
    gceSURF_FORMAT  *format,
    GLint           *glFormat,
    GLint           *glInternalFormat,
    GLint           *glType,
    GLint           *level,
    GLuint          *sliceIndex,
    GLvoid          **pixel
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetTextureAttribFromImage(gc,
                                             eglImage,
                                             width,
                                             height,
                                             stride,
                                             format,
                                             glFormat,
                                             glInternalFormat,
                                             glType,
                                             level,
                                             sliceIndex,
                                             pixel);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexDirectVIV(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint width,
    GLint height,
    GLenum format,
    GLvoid **pixels
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexDirectVIV(gc, texObj, width, height, format, pixels);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexDirectInvalidateVIV(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexDirectInvalidateVIV(gc, texObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_TexDirectVIVMap(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLvoid **logical,
    const GLuint *physical,
    GLboolean tiled
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipTexDirectVIVMap(gc,
                                   texObj,
                                   target,
                                   width,
                                   height,
                                   format,
                                   logical,
                                   physical,
                                   tiled);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ChangeDrawBuffers(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipChangeDrawBuffers(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ChangeReadBuffers(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipChangeReadBuffers(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_DetachDrawable(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDetachDrawable(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_ClearBegin(
    __GLcontext *gc,
    GLbitfield *mask
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipClearBegin(gc, mask);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ClearValidateState(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipClearValidateState(gc, mask);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ClearEnd(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipClearEnd(gc, mask);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_Clear(
    __GLcontext * gc,
    GLuint mask
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipClear(gc, mask);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ClearBuffer(
    __GLcontext *gc,
    GLenum buffer,
    GLint drawbuffer,
    GLvoid *value,
    GLenum type
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();

    ret = __glChipClearBuffer(gc, buffer, drawbuffer, value, type);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ClearBufferfi(
    __GLcontext *gc,
    GLfloat depth,
    GLint stencil
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipClearBufferfi(gc, depth, stencil);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CompileShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCompileShader(gc, shaderObject);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_DeleteShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteShader(gc, shaderObject);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_CreateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCreateProgram(gc, programObject);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_DeleteProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteProgram(gc, programObject);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_LinkProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipLinkProgram(gc, programObject);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_UseProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean *valid
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipUseProgram(gc, programObject, valid);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ValidateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean callFromDraw
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipValidateProgram(gc, programObject, callFromDraw);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GetProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei bufSize,
    GLsizei *length,
    GLenum *binaryFormat,
    GLvoid *binary
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetProgramBinary_V1(gc,
                                       programObject,
                                       bufSize,
                                       length,
                                       binaryFormat,
                                       binary);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLvoid *binary,
    GLsizei length
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipProgramBinary_V1(gc, programObject, binary, length);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_ShaderBinary(
    __GLcontext *gc,
    GLsizei n,
    __GLshaderObject **shaderObjects,
    GLenum binaryformat,
    const GLvoid *binary,
    GLsizei length
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipShaderBinary(gc, n, shaderObjects, binaryformat, binary, length);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BindAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    const GLchar *name
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret =  __glChipBindAttributeLocation(gc, programObject, index, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GetActiveAttribute(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei *length,
    GLint *size,
    GLenum *type,
    char *name
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetActiveAttribute(gc, programObject, index, bufsize, length, size, type, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLint
__glChipProfile_GetAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLint ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetAttributeLocation(gc, programObject, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLint
__glChipProfile_GetFragDataLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLint ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetFragDataLocation(gc, programObject, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLint
__glChipProfile_GetUniformLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLint ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetUniformLocation(gc, programObject, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_GetActiveUniform(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei *length,
    GLint *size,
    GLenum *type,
    GLchar *name
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetActiveUniform(gc,
                             programObject,
                             index,
                             bufsize,
                             length,
                             size,
                             type,
                             name);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_GetActiveUniformsiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLuint *uniformIndices,
    GLenum pname,
    GLint *params
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetActiveUniformsiv(gc, programObject, uniformCount, uniformIndices, pname, params);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_GetUniformIndices(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLchar* const * uniformNames,
    GLuint *uniformIndices
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetUniformIndices(gc, programObject, uniformCount, uniformNames, uniformIndices);
    __GLCHIP_PROFILER_FOOTER();
}

GLuint
__glChipProfile_GetUniformBlockIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *uniformBlockName
    )
{
    GLuint ret;
    __GLCHIP_PROFILER_HEADER();
    ret =
__glChipGetUniformBlockIndex(gc, programObject, uniformBlockName);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_GetActiveUniformBlockiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLenum pname,
    GLint *params
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetActiveUniformBlockiv(gc, programObject, uniformBlockIndex, pname, params);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_ActiveUniformBlockName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *uniformBlockName
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipActiveUniformBlockName(gc,
                                   programObject,
                                   uniformBlockIndex,
                                   bufSize,
                                   length,
                                   uniformBlockName);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_UniformBlockBinding(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLuint uniformBlockBinding
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipUniformBlockBinding(gc, programObject, uniformBlockIndex, uniformBlockBinding);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_SetUniformData(
    __GLcontext       *gc,
    __GLprogramObject *programObject,
    GLint              location,
    GLint              type,
    GLsizei            count,
    const GLvoid      *values,
    GLboolean          transpose
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipSetUniformData(gc, programObject, location, type, count, values, transpose);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GetUniformData(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location,
    GLint type,
    GLvoid *values
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetUniformData(gc, programObject, location, type, values);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLsizei
__glChipProfile_GetUniformSize(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location
    )
{
    GLint ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetUniformSize(gc, programObject, location);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_BuildTexEnableDim(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBuildTexEnableDim(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLuint
__glChipProfile_GetProgramResourceIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    const GLchar *name
    )
{
    GLuint ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetProgramResourceIndex(gc, programObject, progInterface, name);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_GetProgramResourceName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    GLuint index,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *name
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetProgramResourceName(gc, programObject, progInterface, index, bufSize, length, name);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_GetProgramResourceiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    GLuint index,
    GLsizei propCount,
    const GLenum *props,
    GLsizei bufSize,
    GLsizei *length,
    GLint *params
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetProgramResourceiv(gc, programObject, progInterface, index, propCount, props, bufSize, length, params);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_ValidateProgramPipeline(
    __GLcontext *gc,
    __GLprogramPipelineObject *ppObj,
    GLboolean callFromDraw
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipValidateProgramPipeline(gc, ppObj, callFromDraw);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BindBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBindBufferObject(gc, bufObj, targetIndex);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DeleteBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipDeleteBufferObject(gc, bufObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid*
__glChipProfile_MapBufferRange(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length,
    GLbitfield access
    )
{
    GLvoid *ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipMapBufferRange(gc, bufObj, targetIndex, offset, length, access);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_FlushMappedBufferRange(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipFlushMappedBufferRange(gc, bufObj, targetIndex, offset, length);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_UnMapBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipUnMapBufferObject(gc, bufObj, targetIndex);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BufferData(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    const void *data
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBufferData(gc, bufObj, targetIndex, data);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BufferSubData(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr size,
    const void* data
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBufferSubData(gc, bufObj, targetIndex, offset, size, data);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_CopyBufferSubData(
    __GLcontext* gc,
    GLuint readTargetIndex,
    __GLbufferObject* readBufObj,
    GLuint writeTargetIndex,
    __GLbufferObject* writeBufObj,
    GLintptr readOffset,
    GLintptr writeOffset,
    GLsizeiptr size
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCopyBufferSubData(gc,
                                     readTargetIndex,
                                     readBufObj,
                                     writeTargetIndex,
                                     writeBufObj,
                                     readOffset,
                                     writeOffset,
                                     size);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BindDrawFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBindDrawFramebuffer(gc, preFBO, curFBO);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_BindReadFramebuffer(
    __GLcontext *gc,
    __GLframebufferObject *preFBO,
    __GLframebufferObject *curFBO
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBindReadFramebuffer(gc, preFBO, curFBO);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_BindRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *renderbuf
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBindRenderbuffer(gc, renderbuf);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_DeleteRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteRenderbuffer(gc, rbo);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_DetachRenderbuffer(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDetachRenderbuffer(gc, rbo);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_RenderbufferStorage(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipRenderbufferStorage(gc, rbo);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_CleanTextureShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipCleanTextureShadow(gc, texObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_CleanRenderbufferShadow(
    __GLcontext *gc,
    __GLrenderbufferObject *rbo
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipCleanRenderbufferShadow(gc, rbo);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_BlitFramebufferBegin(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBlitFramebufferBegin(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BlitFramebufferValidateState(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBlitFramebufferValidateState(gc, mask);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BlitFramebufferEnd(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBlitFramebufferEnd(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid __glChipProfile_BlitFramebuffer(__GLcontext *gc,
                               GLint srcX0,
                               GLint srcY0,
                               GLint srcX1,
                               GLint srcY1,
                               GLint dstX0,
                               GLint dstY0,
                               GLint dstX1,
                               GLint dstY1,
                               GLbitfield mask,
                               GLboolean  xReverse,
                               GLboolean  yReverse,
                               GLenum     filter
                               )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBlitFramebuffer(gc,
                                   srcX0,
                                   srcY0,
                                   srcX1,
                                   srcY1,
                                   dstX0,
                                   dstY0,
                                   dstX1,
                                   dstY1,
                                   mask,
                                   xReverse,
                                   yReverse,
                                   filter);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_FramebufferTexture(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    __GLtextureObject *texObj,
    GLint level,
    GLint face,
    GLsizei samples,
    GLint zoffset,
    GLboolean layered
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipFramebufferTexture(gc,
                                      fbo,
                                      attachIndex,
                                      texObj,
                                      level,
                                      face,
                                      samples,
                                      zoffset,
                                      layered);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_FramebufferRenderbuffer(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    __GLrenderbufferObject *rbo
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipFramebufferRenderbuffer(gc, fbo, attachIndex, rbo);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_IsFramebufferComplete(
    __GLcontext *gc,
    __GLframebufferObject *framebufferObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipIsFramebufferComplete(gc, framebufferObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_BeginQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipBeginQuery(gc, queryObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_EndQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipEndQuery(gc, queryObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_GetQueryObject(
    __GLcontext *gc,
    GLenum pname,
    __GLqueryObject *queryObj
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetQueryObject(gc, pname, queryObj);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_DeleteQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteQuery(gc, queryObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_CreateSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCreateSync(gc, syncObject);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_DeleteSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipDeleteSync(gc, syncObject);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLenum
__glChipProfile_WaitSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject,
    GLuint64 timeout
    )
{
    GLenum ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipWaitSync(gc, syncObject, timeout);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLboolean
__glChipProfile_SyncImage(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipSyncImage(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_BindXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBindXFB(gc, xfbObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_DeleteXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipDeleteXFB(gc, xfbObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_BeginXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBeginXFB(gc, xfbObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_EndXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipEndXFB(gc, xfbObj);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_PauseXFB(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipPauseXFB(gc);
    __GLCHIP_PROFILER_FOOTER();
}

GLvoid
__glChipProfile_ResumeXFB(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipResumeXFB(gc);
    __GLCHIP_PROFILER_FOOTER();
}


GLvoid
__glChipProfile_GetXFBVarying(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufSize,
    GLsizei* length,
    GLsizei* size,
    GLenum* type,
    GLchar* name
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetXFBVarying(gc,
                          programObject,
                          index,
                          bufSize,
                          length,
                          size,
                          type,
                          name);
    __GLCHIP_PROFILER_FOOTER();
}

GLboolean
__glChipProfile_CheckXFBBufSizes(
    __GLcontext *gc,
    __GLxfbObject *xfbObj,
    GLsizei count
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipCheckXFBBufSizes(gc, xfbObj, count);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLenum
__glChipProfile_GetGraphicsResetStatus(
    __GLcontext *gc
    )
{
    GLenum ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetGraphicsResetStatus(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

#if VIVANTE_PROFILER
GLboolean
__glChipProfile_Profiler(
    IN gctPOINTER Profiler,
    IN GLuint Enum,
    IN gctHANDLE Value
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipProfiler(Profiler, Enum, Value);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}
#endif

#if __GL_CHIP_PATCH_ENABLED
void
__glChipProfile_PatchBlend(
    IN __GLcontext *gc,
    IN gctBOOL bEnable
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipPatchBlend(gc, bEnable);
    __GLCHIP_PROFILER_FOOTER();
}

#endif

GLenum
__glChipProfile_GetError(
    __GLcontext *gc
    )
{
    GLenum ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipGetError(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_GetSampleLocation(
    __GLcontext * gc,
    GLuint index,
    GLfloat * val
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipGetSampleLocation(gc, index, val);
    __GLCHIP_PROFILER_FOOTER();
    return;
}


GLboolean
__glChipProfile_ComputeBegin(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipComputeBegin(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}


GLboolean
__glChipProfile_ComputeValidateState(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipComputeValidateState(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;

}

GLvoid
__glChipProfile_ComputeEnd(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipComputeEnd(gc);
    __GLCHIP_PROFILER_FOOTER();
    return;
}


GLboolean
__glChipProfile_DispatchCompute(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipDispatchCompute(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}


GLvoid
__glChipProfile_MemoryBarrier(
    __GLcontext *gc,
    GLbitfield barriers
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipMemoryBarrier(gc, barriers);
    __GLCHIP_PROFILER_FOOTER();
    return;

}

GLvoid
__glChipProfile_BlendBarrier(
    __GLcontext *gc
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipBlendBarrier(gc);
    __GLCHIP_PROFILER_FOOTER();
    return;

}


GLboolean
__glChipProfile_MultiDrawElementsIndirect(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipMultiDrawElementsIndirect(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}


GLboolean
__glChipProfile_MultiDrawArraysIndirect(
    __GLcontext *gc
    )
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipMultiDrawArraysIndirect(gc);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

/* Init DP interface to chip specific function */
GLvoid
gcChipInitProfileDevicePipeline(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);
    gc->dp.makeCurrent = __glChipProfile_MakeCurrent;
    gc->dp.loseCurrent = __glChipProfile_LoseCurrent;
    gc->dp.destroyPrivateData = __glChipProfile_DestroyContext;
    gc->dp.queryFormatInfo = __glChipProfile_QueryFormatInfo;

    gc->dp.readPixelsBegin= __glChipProfile_ReadPixelsBegin;
    gc->dp.readPixelsValidateState = __glChipProfile_ReadPixelsValidateState;
    gc->dp.readPixelsEnd = __glChipProfile_ReadPixelsEnd;
    gc->dp.readPixels = __glChipProfile_ReadPixels;

    gc->dp.drawBegin = __glChipProfile_DrawBegin;
    gc->dp.drawValidateState = __glChipProfile_DrawValidateState;
    gc->dp.drawEnd = __glChipProfile_DrawEnd;

     /* Flush, Finish */
    gc->dp.flush = __glChipProfile_Flush;
    gc->dp.finish = __glChipProfile_Finish;

    /* Texture related functions */
    gc->dp.bindTexture = __glChipProfile_BindTexture;
    gc->dp.deleteTexture = __glChipProfile_DeleteTexture;
    gc->dp.detachTexture = __glChipProfile_DetachTexture;
    gc->dp.texImage2D = __glChipProfile_TexImage2D;
    gc->dp.texImage3D = __glChipProfile_TexImage3D;
    gc->dp.texSubImage2D = __glChipProfile_TexSubImage2D;
    gc->dp.texSubImage3D = __glChipProfile_TexSubImage3D;
    gc->dp.copyTexImage2D = __glChipProfile_CopyTexImage2D;
    gc->dp.copyTexSubImage2D = __glChipProfile_CopyTexSubImage2D;
    gc->dp.copyTexSubImage3D = __glChipProfile_CopyTexSubImage3D;
    gc->dp.compressedTexImage2D = __glChipProfile_CompressedTexImage2D;
    gc->dp.compressedTexSubImage2D = __glChipProfile_CompressedTexSubImage2D;
    gc->dp.compressedTexImage3D = __glChipProfile_CompressedTexImage3D;
    gc->dp.compressedTexSubImage3D = __glChipProfile_CompressedTexSubImage3D;
    gc->dp.generateMipmaps = __glChipProfile_GenerateMipMap;

    gc->dp.copyTexBegin = __glChipProfile_CopyTexBegin;
    gc->dp.copyTexValidateState = __glChipProfile_CopyTexValidateState;
    gc->dp.copyTexEnd = __glChipProfile_CopyTexEnd;

    gc->dp.copyImageSubData = __glChipProfile_CopyImageSubData;

    /* EGL image */
    gc->dp.bindTexImage = __glChipProfile_BindTexImage;
    gc->dp.freeTexImage = __glChipProfile_FreeTexImage;
    gc->dp.createEglImageTexture = __glChipProfile_CreateEglImageTexture;
    gc->dp.createEglImageRenderbuffer = __glChipProfile_CreateEglImageRenderbuffer;
    gc->dp.eglImageTargetTexture2DOES = __glChipProfile_EglImageTargetTexture2DOES;
    gc->dp.eglImageTargetRenderbufferStorageOES = __glChipProfile_EglImageTargetRenderbufferStorageOES;
    gc->dp.getTextureAttribFromImage = __glChipProfile_GetTextureAttribFromImage;

    /* VIV_texture_direct */
    gc->dp.texDirectVIV = __glChipProfile_TexDirectVIV;
    gc->dp.texDirectInvalidateVIV = __glChipProfile_TexDirectInvalidateVIV;
    gc->dp.texDirectVIVMap = __glChipProfile_TexDirectVIVMap;

    /* Toggle buffer change */
    gc->dp.changeDrawBuffers = __glChipProfile_ChangeDrawBuffers;
    gc->dp.changeReadBuffers = __glChipProfile_ChangeReadBuffers;
    gc->dp.detachDrawable = __glChipProfile_DetachDrawable;

    /* Clear buffer */
    gc->dp.clearBegin = __glChipProfile_ClearBegin;
    gc->dp.clearValidateState = __glChipProfile_ClearValidateState;
    gc->dp.clearEnd = __glChipProfile_ClearEnd;
    gc->dp.clear = __glChipProfile_Clear;
    gc->dp.clearBuffer = __glChipProfile_ClearBuffer;
    gc->dp.clearBufferfi = __glChipProfile_ClearBufferfi;


    /* GLSL */
    gc->dp.compileShader = __glChipProfile_CompileShader;
    gc->dp.deleteShader = __glChipProfile_DeleteShader;
    gc->dp.createProgram = __glChipProfile_CreateProgram;
    gc->dp.deleteProgram = __glChipProfile_DeleteProgram;
    gc->dp.linkProgram = __glChipProfile_LinkProgram;
    gc->dp.useProgram = __glChipProfile_UseProgram;
    gc->dp.validateProgram = __glChipProfile_ValidateProgram;
    gc->dp.getProgramBinary = __glChipProfile_GetProgramBinary_V1;
    gc->dp.programBinary = __glChipProfile_ProgramBinary_V1;
    gc->dp.shaderBinary = __glChipProfile_ShaderBinary;
    gc->dp.bindAttributeLocation = __glChipProfile_BindAttributeLocation;
    gc->dp.getActiveAttribute = __glChipProfile_GetActiveAttribute;
    gc->dp.getAttributeLocation = __glChipProfile_GetAttributeLocation;
    gc->dp.getFragDataLocation = __glChipProfile_GetFragDataLocation;
    gc->dp.getUniformLocation = __glChipProfile_GetUniformLocation;
    gc->dp.getActiveUniform = __glChipProfile_GetActiveUniform;
    gc->dp.getActiveUniformsiv = __glChipProfile_GetActiveUniformsiv;
    gc->dp.getUniformIndices = __glChipProfile_GetUniformIndices;
    gc->dp.getUniformBlockIndex = __glChipProfile_GetUniformBlockIndex;
    gc->dp.getActiveUniformBlockiv = __glChipProfile_GetActiveUniformBlockiv;
    gc->dp.getActiveUniformBlockName = __glChipProfile_ActiveUniformBlockName;
    gc->dp.uniformBlockBinding = __glChipProfile_UniformBlockBinding;
    gc->dp.setUniformData = __glChipProfile_SetUniformData;
    gc->dp.getUniformData = __glChipProfile_GetUniformData;
    gc->dp.getUniformSize = __glChipProfile_GetUniformSize;
    gc->dp.buildTexEnableDim = __glChipProfile_BuildTexEnableDim;
    gc->dp.getProgramResourceIndex = __glChipProfile_GetProgramResourceIndex;
    gc->dp.getProgramResourceName = __glChipProfile_GetProgramResourceName;
    gc->dp.getProgramResourceiv = __glChipProfile_GetProgramResourceiv;
    gc->dp.validateProgramPipeline = __glChipProfile_ValidateProgramPipeline;

    /* Buffer object */
    gc->dp.bindBuffer = __glChipProfile_BindBufferObject;
    gc->dp.deleteBuffer = __glChipProfile_DeleteBufferObject;
    gc->dp.mapBufferRange = __glChipProfile_MapBufferRange;
    gc->dp.flushMappedBufferRange = __glChipProfile_FlushMappedBufferRange;
    gc->dp.unmapBuffer = __glChipProfile_UnMapBufferObject;
    gc->dp.bufferData = __glChipProfile_BufferData;
    gc->dp.bufferSubData = __glChipProfile_BufferSubData;
    gc->dp.copyBufferSubData = __glChipProfile_CopyBufferSubData;

    /* FBO */
    gc->dp.bindDrawFramebuffer = __glChipProfile_BindDrawFramebuffer;
    gc->dp.bindReadFramebuffer = __glChipProfile_BindReadFramebuffer;
    gc->dp.bindRenderbuffer = __glChipProfile_BindRenderbuffer;
    gc->dp.deleteRenderbuffer = __glChipProfile_DeleteRenderbuffer;
    gc->dp.detachRenderbuffer = __glChipProfile_DetachRenderbuffer;
    gc->dp.renderbufferStorage = __glChipProfile_RenderbufferStorage;
    gc->dp.blitFramebufferBegin = __glChipProfile_BlitFramebufferBegin;
    gc->dp.blitFramebufferValidateState = __glChipProfile_BlitFramebufferValidateState;
    gc->dp.blitFramebufferEnd = __glChipProfile_BlitFramebufferEnd;
    gc->dp.blitFramebuffer = __glChipProfile_BlitFramebuffer;
    gc->dp.frameBufferTexture = __glChipProfile_FramebufferTexture;
    gc->dp.framebufferRenderbuffer = __glChipProfile_FramebufferRenderbuffer;
    gc->dp.isFramebufferComplete = __glChipProfile_IsFramebufferComplete;
    if (gcoHAL_GetOption(gcvNULL, gcvOPTION_FBO_PREFER_MEM))
    {
        gc->dp.cleanTextureShadow = __glChipProfile_CleanTextureShadow;
        gc->dp.cleanRenderbufferShadow = __glChipProfile_CleanRenderbufferShadow;
    }

    /*
    gc->dp.invalidateFramebuffer = NULL;
    gc->dp.invalidateDrawable = NULL;
    */

    /* Query */
    gc->dp.beginQuery = __glChipProfile_BeginQuery;
    gc->dp.endQuery = __glChipProfile_EndQuery;
    gc->dp.getQueryObject = __glChipProfile_GetQueryObject;
    gc->dp.deleteQuery = __glChipProfile_DeleteQuery;

    /* Sync */
    gc->dp.createSync = __glChipProfile_CreateSync;
    gc->dp.deleteSync = __glChipProfile_DeleteSync;
    gc->dp.waitSync = __glChipProfile_WaitSync;
    gc->dp.syncImage = __glChipProfile_SyncImage;

    /* XFB */
    gc->dp.bindXFB = __glChipProfile_BindXFB;
    gc->dp.deleteXFB = __glChipProfile_DeleteXFB;
    gc->dp.beginXFB  = __glChipProfile_BeginXFB;
    gc->dp.endXFB  = __glChipProfile_EndXFB;
    gc->dp.pauseXFB = __glChipProfile_PauseXFB;
    gc->dp.resumeXFB = __glChipProfile_ResumeXFB;
    gc->dp.getXfbVarying = __glChipProfile_GetXFBVarying;
    gc->dp.checkXFBBufSizes = __glChipProfile_CheckXFBBufSizes;

    gc->dp.getGraphicsResetStatus = __glChipProfile_GetGraphicsResetStatus;

    /* profiler */
#if VIVANTE_PROFILER
    gc->dp.profiler = __glChipProfile_Profiler;
#else
    gc->dp.profiler = NULL;
#endif

    /* Patches. */
#if __GL_CHIP_PATCH_ENABLED
    gc->dp.patchBlend = __glChipProfile_PatchBlend;
#else
    gc->dp.patchBlend = NULL;
#endif

    gc->dp.getError = __glChipProfile_GetError;

    gc->dp.getSampleLocation = __glChipProfile_GetSampleLocation;

    gc->dp.computeBegin = __glChipProfile_ComputeBegin;
    gc->dp.computeValidateState = __glChipProfile_ComputeValidateState;
    gc->dp.computeEnd = __glChipProfile_ComputeEnd;
    gc->dp.dispatchCompute = __glChipProfile_DispatchCompute;

    gc->dp.memoryBarrier = __glChipProfile_MemoryBarrier;

    gc->dp.multiDrawArraysIndirectEXT = __glChipProfile_MultiDrawArraysIndirect;

    gc->dp.multiDrawElementsIndirectEXT = __glChipProfile_MultiDrawElementsIndirect;;

    gc->dp.blendBarrier = __glChipProfile_BlendBarrier;

    gcmFOOTER_NO();
}

#endif
