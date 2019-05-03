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


#include "gc_es_context.h"
#include "gc_hal_user.h"
#include "gc_chip_context.h"


#define _GC_OBJ_ZONE    gcdZONE_ES30_PROFILER


#if VIVANTE_PROFILER
#define PRO_NODE_SIZE 8
typedef struct _program_list{
    gctUINT32 program_id[PRO_NODE_SIZE];
    gctUINT8  dirty_flag[PRO_NODE_SIZE];
    struct _program_list * next;
} program_list;

program_list *PGM;

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
static void _pro_destroy() {
    program_list *pPGM, *freenode;
    pPGM = PGM;
    while (pPGM != gcvNULL)
    {
        freenode = pPGM;
        pPGM = pPGM->next;
        gcoOS_Free(gcvNULL, freenode);
    }
    PGM = gcvNULL;
    return;
}

gceSTATUS
gcChipProfilerInitialize(
    IN __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR *env = gcvNULL;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    glsPROFILER * profiler = &gc->profiler;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&gc->profiler, gcmSIZEOF(glsPROFILER));

    switch (__glesApiProfileMode)
    {
    case -1:
        profiler->enable = gcvFALSE;
        gcmFOOTER();
        return gcvSTATUS_OK;
    case 0:
        gcoPROFILER_Disable();
        profiler->enable = gcvFALSE;
        gcmFOOTER();
        return gcvSTATUS_OK;
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
        gcmFOOTER();
        return status;
    }
    gcmONERROR(gcoPROFILER_Construct(&chipCtx->profiler));

    profiler->useGlfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_GLFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        profiler->useGlfinish = gcvTRUE;
        chipCtx->profiler->bufferCount = NumOfPerDrawBuf;
    }

    profiler->perDrawMode = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_PERDRAW_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
    {
        profiler->perDrawMode =
        chipCtx->profiler->perDrawMode = gcvTRUE;
        chipCtx->profiler->bufferCount = NumOfPerDrawBuf;
    }

    chipCtx->profiler->profilerClient = gcvCLIENT_OPENGLES;

    if (gcoPROFILER_Initialize(chipCtx->profiler) != gcvSTATUS_OK)
    {
        profiler->enable = gcvFALSE;
        gcmFOOTER();
        return status;
    }

    profiler->enable = gcvTRUE;
    profiler->curFrameNumber = 0;
    profiler->frameNumber = 0;
    profiler->frameBegun = gcvFALSE;
    profiler->finishNumber = 0;
    profiler->drawCount = 0;
    profiler->writeDrawable = gcvFALSE;

    gcoOS_GetTime(&profiler->frameStartTimeusec);

    gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_HEADER);

OnError:
    /* Return the error. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipProfilerDestroy(
    IN __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    glsPROFILER * profiler = &gc->profiler;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (profiler->enable)
    {
        _pro_destroy();
        profiler->enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(chipCtx->profiler));
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
gcChipProfilerWrite(
    IN __GLcontext *gc,
    IN GLuint Enum
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT rev;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcoPROFILER Profiler = chipCtx->profiler;
    gctSTRING infoCompany = "Vivante Corporation";
    gctSTRING infoVersion = "1.3";
    gctCHAR infoRevision[255] = { '\0' };   /* read from hw */
    gctSTRING infoRenderer = gc->constants.renderer;
    gctSTRING infoDriver = "OpenGL ES 3.0";
    gctUINT offset = 0;
    gctUINT32 totalCalls = 0;
    gctUINT32 totalDrawCalls = 0;
    gctUINT32 totalStateChangeCalls = 0;
    gctUINT32 maxrss, ixrss, idrss, isrss;
    gctINT32 i;

    gcmHEADER_ARG("gc=0x%x, Enum=%d", gc, Enum);

    switch (Enum)
    {
    case GL3_PROFILER_WRITE_HEADER:
        /* Write Generic Info. */
        rev = chipCtx->chipRevision;
#define BCD(digit)      ((rev >> (digit * 4)) & 0xF)
        gcoOS_MemFill(infoRevision, 0, gcmSIZEOF(infoRevision));
        if (BCD(3) == 0)

        {

            /* Old format. */

            gcmONERROR(gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),

                &offset, "revision=\"%d.%d\" ", BCD(1), BCD(0)));

        }

        else

        {

            /* New format. */

            gcmONERROR(gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),

                &offset, "revision=\"%d.%d.%d_rc%d\" ",

                BCD(3), BCD(2), BCD(1), BCD(0)));

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

    case GL3_PROFILER_WRITE_FRAME_BEGIN:
        if (!gc->profiler.writeDrawable && gc->drawablePrivate)
        {
            gctUINT offset = 0;
            gctCHAR  infoScreen[255] = { '\0' };
            gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
            gcmONERROR(gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),

                                          &offset, "%d x %d", gc->drawablePrivate->width, gc->drawablePrivate->height));
            gcmWRITE_CONST(VPC_INFOSCREENSIZE);
            gcmWRITE_STRING(infoScreen);

            gcmWRITE_CONST(VPG_END);
            gc->profiler.writeDrawable = gcvTRUE;
        }
        if (!gc->profiler.frameBegun && gc->profiler.need_dump)

        {

            gcmWRITE_COUNTER(VPG_FRAME, gc->profiler.frameNumber);

            gc->profiler.frameBegun = gcvTRUE;

        }

        break;


    case GL3_PROFILER_WRITE_FRAME_END:

        /*write time*/
        if (gc->profiler.need_dump)
        {
            gcmWRITE_CONST(VPG_TIME);
            gcmWRITE_COUNTER(VPC_ELAPSETIME, (gctINT32)(gc->profiler.frameEndTimeusec - gc->profiler.frameStartTimeusec));
            gcmWRITE_COUNTER(VPC_CPUTIME, (gctINT32)gc->profiler.totalDriverTime);
            gcmWRITE_CONST(VPG_END);

            gcoOS_GetMemoryUsage(&maxrss, &ixrss, &idrss, &isrss);

            gcmWRITE_CONST(VPG_MEM);

            gcmWRITE_COUNTER(VPC_MEMMAXRES, maxrss);
            gcmWRITE_COUNTER(VPC_MEMSHARED, ixrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDDATA, idrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDSTACK, isrss);

            gcmWRITE_CONST(VPG_END);

            /* write api time counters */
            gcmWRITE_CONST(VPG_ES30_TIME);
            for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
            {
                if (gc->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES30_TIME + 1 + i, (gctINT32)gc->profiler.apiTimes[i]);
                }
            }
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_ES30);


            for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
            {
                if (gc->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES30 + 1 + i, gc->profiler.apiCalls[i]);

                    totalCalls += gc->profiler.apiCalls[i];

                    switch (GLES3_APICALLBASE + i)
                    {
                    case GLES3_DRAWARRAYS:
                    case GLES3_DRAWELEMENTS:
                    case GLES3_DRAWRANGEELEMENTS:
                    case GLES3_DRAWARRAYSINSTANCED:
                    case GLES3_DRAWELEMENTSINSTANCED:
                    case GLES3_MULTIDRAWARRAYSEXT:
                    case GLES3_MULTIDRAWELEMENTSEXT:
                    case GLES31_DRAWARRAYSINDIRECT:
                    case GLES31_DRAWELEMENTSINDIRECT:
                    case GLES31_MULTIDRAWARRAYSINDIRECTEXT:
                    case GLES31_MULTIDRAWELEMENTSINDIRECTEXT:
                    case GLES31_DRAWELEMENTSBASEVERTEX:
                    case GLES31_DRAWRANGEELEMENTSBASEVERTEX:
                    case GLES31_DRAWELEMENTSINSTANCEDBASEVERTEX:
                    case GLES31_MULTIDRAWELEMENTSBASEVERTEXEXT:
                        totalDrawCalls += gc->profiler.apiCalls[i];
                        break;

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
                        totalStateChangeCalls += gc->profiler.apiCalls[i];
                        break;

                    default:
                        break;
                    }
                }
            }

            gcmWRITE_COUNTER(VPC_ES30CALLS, totalCalls);
            gcmWRITE_COUNTER(VPC_ES30DRAWCALLS, totalDrawCalls);
            gcmWRITE_COUNTER(VPC_ES30STATECHANGECALLS, totalStateChangeCalls);

            gcmWRITE_COUNTER(VPC_ES30POINTCOUNT, gc->profiler.drawPointCount);
            gcmWRITE_COUNTER(VPC_ES30LINECOUNT, gc->profiler.drawLineCount);
            gcmWRITE_COUNTER(VPC_ES30TRIANGLECOUNT, gc->profiler.drawTriangleCount);
            gcmWRITE_CONST(VPG_END);

            gcmWRITE_CONST(VPG_END);
        }
        break;

    case GL3_PROFILER_WRITE_FRAME_RESET:
        for (i = 0; i < GLES3_NUM_API_CALLS; ++i)
        {
            gc->profiler.apiCalls[i] = 0;
            gc->profiler.apiTimes[i] = 0;
            gc->profiler.totalDriverTime = 0;
        }

        /* Clear variables for next frame. */
        gc->profiler.drawPointCount = 0;
        gc->profiler.drawLineCount = 0;
        gc->profiler.drawTriangleCount = 0;
        gc->profiler.drawCount = 0;
        gc->profiler.totalDriverTime = 0;


        gcmONERROR(gcoOS_GetTime(&gc->profiler.frameStartTimeusec));


        break;
    default:
        GL_ASSERT(0);
        break;
    }

OnError:

    gcmFOOTER_NO();
    return status;
}

GLboolean
__glChipProfilerSet(
    IN __GLcontext *gc,
    IN GLuint Enum,
    IN gctHANDLE Value
)
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    glsPROFILER * profiler = &gc->profiler;
    gcoPROFILER Profiler;
    static gctBOOL dump_program = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x, Enum=%d, Value=0x%x", gc, Enum, Value);
    GL_ASSERT(gc);

    if (!profiler->enable)
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    Profiler = chipCtx->profiler;

    switch (__glesApiProfileMode)
    {
    case 1:
        if (profiler->frameCount == 0 || profiler->frameNumber < profiler->frameCount)
        {
            Profiler->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Profiler->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    case 2:
        if (profiler->enableOutputCounters)
        {
            Profiler->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Profiler->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    case 3:
        if ((profiler->frameStartNumber == 0 && profiler->frameEndNumber == 0) ||
            (profiler->curFrameNumber >= profiler->frameStartNumber && profiler->curFrameNumber <= profiler->frameEndNumber))
        {
            Profiler->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Profiler->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    default:
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    switch (Enum)
    {
    case GL3_PROFILER_FRAME_END:



        gcmONERROR(gcoOS_GetTime(&profiler->frameEndTimeusec));

        profiler->drawCount = 0;

        profiler->curFrameNumber++;

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_BEGIN));



        gcmONERROR(gcoPROFILER_End(Profiler, gcvCOUNTER_OP_FRAME, profiler->frameNumber));

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_END));

        gcmONERROR(gcoPROFILER_Flush(Profiler));

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_RESET));
        /* Next frame. */
        if (profiler->need_dump)
        {
            profiler->frameNumber++;
        }
        profiler->frameBegun = gcvFALSE;
        break;

    case GL3_PROFILER_FINISH_BEGIN:



        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_BEGIN));

        gcmONERROR(gcoPROFILER_EnableCounters(chipCtx->profiler, gcvCOUNTER_OP_FINISH));

        profiler->drawCount = 0;



        break;



    case GL3_PROFILER_FINISH_END:



        gcmONERROR(gcoOS_GetTime(&profiler->frameEndTimeusec));

        gcmONERROR(gcoPROFILER_End(Profiler, gcvCOUNTER_OP_FINISH, profiler->finishNumber));

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_END));

        gcmONERROR(gcoPROFILER_Flush(Profiler));

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_RESET));
        /* Next frame. */
        if (profiler->need_dump)
        {
            profiler->finishNumber++;
            profiler->frameNumber++;
        }
        profiler->frameBegun = gcvFALSE;
        break;

    case GL3_PROFILER_PRIMITIVE_TYPE:
        profiler->primitiveType = gcmPTR2INT32(Value);
        break;

    case GL3_PROFILER_PRIMITIVE_COUNT:
        profiler->primitiveCount = gcmPTR2INT32(Value);
        switch (profiler->primitiveType)
        {
        case GL_POINTS:
            profiler->drawPointCount += gcmPTR2INT32(Value);
            break;

        case GL_LINES:
        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            profiler->drawLineCount += gcmPTR2INT32(Value);
            break;

        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            profiler->drawTriangleCount += gcmPTR2INT32(Value);
            break;
        }
        break;

    case GL3_PROFILER_DRAW_BEGIN:

        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(gcoPROFILER_EnableCounters(chipCtx->profiler, gcvCOUNTER_OP_DRAW));
        break;

    case GL3_PROFILER_DRAW_END:

        gcmONERROR(gcoPROFILER_End(Profiler, gcvCOUNTER_OP_DRAW, profiler->drawCount));
        profiler->drawCount++;
        break;

        /* Print program info immediately as we do not save it. */
    case GL3_PROGRAM_IN_USE_BEGIN:
        if (!profiler->need_dump)
            break;
        dump_program = _pro_dirty(gcmPTR2INT32(Value));
        gcmONERROR(gcChipProfilerWrite(gc, GL3_PROFILER_WRITE_FRAME_BEGIN));
        if (!profiler->perDrawMode)
        {
            gcmONERROR(gcoPROFILER_EnableCounters(chipCtx->profiler, gcvCOUNTER_OP_FRAME));
        }
        gcmWRITE_CONST(VPG_PROG);
        gcmWRITE_COUNTER(VPC_PROGRAMHANDLE, gcmPTR2INT32(Value));
        break;

    case GL3_PROGRAM_IN_USE_END:
        if (!profiler->need_dump)
            break;
        gcmWRITE_CONST(VPG_END);
        break;

    case GL3_PROGRAM_VERTEX_SHADER:
        if (!profiler->need_dump)
            break;
        if (dump_program && GetShaderSourceCode((gcSHADER)Value))
        {
            gcmWRITE_CONST(VPG_PVS);
            gcmWRITE_CONST(VPC_PVSSOURCE);
            gcmWRITE_STRING(GetShaderSourceCode((gcSHADER)Value));
            gcmWRITE_CONST(VPG_END);
        }
        break;

    case GL3_PROGRAM_FRAGMENT_SHADER:
        if (!profiler->need_dump)
            break;
        if (dump_program && GetShaderSourceCode((gcSHADER)Value))
        {
            gcmWRITE_CONST(VPG_PPS);
            gcmWRITE_CONST(VPC_PPSSOURCE);
            gcmWRITE_STRING(GetShaderSourceCode((gcSHADER)Value));
            gcmWRITE_CONST(VPG_END);
        }
        break;

    default:
        break;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
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

    if (gc->profiler.enable && gc->profiler.useGlfinish)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_FINISH_BEGIN, 0);
    }

    ret = __glChipFlush(gc);

    if (gc->profiler.enable && gc->profiler.useGlfinish)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_FINISH_END, 0);
    }

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

    if (gc->profiler.enable && gc->profiler.useGlfinish)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_FINISH_BEGIN, 0);
    }

    ret = __glChipFinish(gc);

    if (gc->profiler.enable && gc->profiler.useGlfinish)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_FINISH_END, 0);
    }

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
    GLboolean layered,
    __GLfboAttachPoint *preAttach
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
                                      layered,
                                      preAttach);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

GLvoid
__glChipProfile_FramebufferRenderbuffer(
    __GLcontext *gc,
    __GLframebufferObject *fbo,
    GLint attachIndex,
    __GLrenderbufferObject *rbo,
     __GLfboAttachPoint *preAttach
    )
{
    __GLCHIP_PROFILER_HEADER();
    __glChipFramebufferRenderbuffer(gc, fbo, attachIndex, rbo, preAttach);
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
    GLuint64 count
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

GLboolean
__glChipProfile_ProfilerSet(
    IN __GLcontext *gc,
    IN GLuint Enum,
    IN gctHANDLE Value
)
{
    GLboolean ret;
    __GLCHIP_PROFILER_HEADER();
    ret = __glChipProfilerSet(gc, Enum, Value);
    __GLCHIP_PROFILER_FOOTER();
    return ret;
}

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
    gc->dp.profiler = __glChipProfile_ProfilerSet;
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

    gc->dp.blendBarrier = __glChipProfile_BlendBarrier;

    gcmFOOTER_NO();
}

#endif
