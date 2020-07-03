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


#include "gc_es_context.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    gcdZONE_GL40_TRACE

/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/

GLboolean
__glChipBeginQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipQueryObject *chipQuery = gcvNULL;
    __GLchipQueryHeader *queryHeader = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    __GLprogramObject *fsProgObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_FS);
    __GLchipSLProgram *fsProgram = fsProgObj ? (__GLchipSLProgram*)fsProgObj->privateData : gcvNULL;
    gctUINT32 physical;

    gcmHEADER_ARG("gc=0x%x queryObj=0x%x", gc, queryObj);

    /* Allocate on first use. */
    if (gcvNULL == queryObj->privateData)
    {
        chipQuery = (__GLchipQueryObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipQueryObject));

        if (!chipQuery)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        /* Create signal to use for OQ */
        gcmONERROR(gcoOS_CreateSignal(chipCtx->os,
                                      gcvFALSE,
                                      &chipQuery->querySignal));

        chipQuery->queryHeader = gcvNULL;
        chipQuery->type  = gcvQUERY_MAX_NUM;

        queryObj->privateData = chipQuery;
    }
    else
    {
        chipQuery = (__GLchipQueryObject *)queryObj->privateData;
    }


    if ((queryObj->target == GL_SAMPLES_PASSED)||
        (queryObj->target == GL_ANY_SAMPLES_PASSED) ||
        (queryObj->target == GL_ANY_SAMPLES_PASSED_CONSERVATIVE))
    {
        if ((!gc->imports.conformGLSpec) && (fsProgram && fsProgram->progFlags.msaaOQ) &&
            !chipCtx->chipFeature.hwFeature.hasBugFixes18)
        {
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }

        if (chipQuery->queryHeader == gcvNULL)
        {
            gctUINT32 gpuCount = 0;
            gctUINT32 clusterIDWidth = 0;

            gcmONERROR(gcoHAL_Query3DCoreCount(chipCtx->hal, &gpuCount));
            gcmONERROR(gcoHAL_QueryCluster(chipCtx->hal, gcvNULL, gcvNULL, gcvNULL, &clusterIDWidth));

            queryHeader = (__GLchipQueryHeader*)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipQueryHeader));
            queryHeader->headerSize = 64 * gpuCount * gcmSIZEOF(gctUINT32) * (gctUINT32)(1<< clusterIDWidth);
            queryHeader->headerIndex = -1;
            queryHeader->headerSurfType = gcvSURF_INDEX;
            chipQuery->type = gcvQUERY_OCCLUSION;
            chipQuery->queryHeader = queryHeader;
        }
        else
        {
            queryHeader = chipQuery->queryHeader;
        }
    }
    else if ((queryObj->target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN) ||
             (queryObj->target == GL_PRIMITIVES_GENERATED_EXT))
    {
        if (!chipCtx->chipFeature.hwFeature.hasHwTFB)
        {
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }

        if (chipQuery->queryHeader == gcvNULL)
        {
            queryHeader = (__GLchipQueryHeader*)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipQueryHeader));
            queryHeader->headerSize = 64;
            queryHeader->headerIndex = -1;
            queryHeader->headerSurfType = gcvSURF_TFBHEADER;
            chipQuery->type = (queryObj->target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN) ?
                                        gcvQUERY_XFB_WRITTEN: gcvQUERY_PRIM_GENERATED;
            chipQuery->queryHeader = queryHeader;
        }
        else
        {
            queryHeader = chipQuery->queryHeader;
        }
    }

    if (gcvNULL == queryHeader->headerLocked)
    {
        gcmONERROR(gcsSURF_NODE_Construct(&queryHeader->headerNode,
                                          queryHeader->headerSize,
                                          64,
                                          queryHeader->headerSurfType,
                                          0,
                                          gcvPOOL_DEFAULT));

        gcmONERROR(gcoSURF_LockNode(&queryHeader->headerNode, gcvNULL, &queryHeader->headerLocked));
    }

    gcoOS_ZeroMemory(queryHeader->headerLocked, queryHeader->headerSize);
    gcmGETHARDWAREADDRESS(queryHeader->headerNode, physical);

    gcmDUMP_BUFFER(gcvNULL,
                   gcvDUMP_BUFFER_MEMORY,
                   physical,
                   queryHeader->headerLocked,
                   0,
                   queryHeader->headerSize);
    if (gc->imports.conformGLSpec)
    {
        gcmONERROR(gco3D_SetQuery(chipCtx->engine, physical, chipQuery->type, gcvTRUE, queryObj->index));
    }
    else
    {
        gcmONERROR(gco3D_SetQuery(chipCtx->engine, physical, chipQuery->type, gcvTRUE, 0));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    if (chipQuery)
    {
        gcmOS_SAFE_FREE(gcvNULL, chipQuery);
    }
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipEndQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipQueryObject *chipQuery = (__GLchipQueryObject *)queryObj->privateData;
    __GLprogramObject *fsProgObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_FS);
    __GLchipSLProgram *fsProgram = fsProgObj ? (__GLchipSLProgram*)fsProgObj->privateData : gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface;

    gcmHEADER_ARG("gc=0x%x queryObj=0x%x", gc, queryObj);

    gcmASSERT(queryObj);

    if ((queryObj->target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ||
         queryObj->target == GL_PRIMITIVES_GENERATED_EXT) &&
         !chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        queryObj->resultAvailable = GL_TRUE;
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    if ((!gc->imports.conformGLSpec) && (fsProgram && fsProgram->progFlags.msaaOQ) &&
        ((queryObj->target == GL_ANY_SAMPLES_PASSED) ||
        (queryObj->target == GL_ANY_SAMPLES_PASSED_CONSERVATIVE)) &&
        !chipCtx->chipFeature.hwFeature.hasBugFixes18)
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    gcmASSERT(chipQuery);

    if (chipQuery->querySignal == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Send value to data addr reg to end QUERY mode. */
    if (gc->imports.conformGLSpec)
    {
        gcmONERROR(gco3D_SetQuery(chipCtx->engine, 0, chipQuery->type, gcvFALSE, queryObj->index));
    }
    else
    {
        gcmONERROR(gco3D_SetQuery(chipCtx->engine, 0, chipQuery->type, gcvFALSE, 0));
    }

    /* Send an event to signal that the data is in the buffer. */
    iface.command            = gcvHAL_SIGNAL;
    iface.engine             = gcvENGINE_RENDER;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(chipQuery->querySignal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
    iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

    /* Schedule the event. */
    gcmONERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipGetQueryObject(
    __GLcontext *gc,
    GLenum pname,
    __GLqueryObject *queryObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipQueryHeader *queryHeader;
    __GLchipQueryObject *chipQuery ;
    gctUINT32 timeout;
    gctINT i;
    __GLprogramObject *fsProgObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_FS);
    __GLchipSLProgram *fsProgram = fsProgObj ? (__GLchipSLProgram*)fsProgObj->privateData : gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x pname=0x%04x queryObj=0x%x", gc, pname, queryObj);
    GL_ASSERT(queryObj);

    if ((queryObj->target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ||
         queryObj->target == GL_PRIMITIVES_GENERATED_EXT) &&
         !chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        queryObj->resultAvailable = GL_TRUE;
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    if ((!gc->imports.conformGLSpec) && (fsProgram && fsProgram->progFlags.msaaOQ) &&
        ((queryObj->target == GL_ANY_SAMPLES_PASSED) ||
        (queryObj->target == GL_ANY_SAMPLES_PASSED_CONSERVATIVE)) &&
        !chipCtx->chipFeature.hwFeature.hasBugFixes18)
    {
        gctSIZE_T num = chipCtx->drawRTWidth * chipCtx->drawRTHeight;
        GLubyte *pixels = (GLubyte*)gc->imports.malloc(gc, 4 * num);

        __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

        if (__glChipReadPixels(gc, 0, 0, (GLsizei)chipCtx->drawRTWidth, (GLsizei)chipCtx->drawRTHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels))
        {
            gctSIZE_T i = 0;
            queryObj->count = 0;
            for (i = 0; i < num; ++i)
            {
                if (pixels[i * 4] != 0)
                {
                    queryObj->count++;
                    break;
                }
            }
        }

        gc->imports.free(gc, pixels);
        queryObj->resultAvailable = GL_TRUE;
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    chipQuery = (__GLchipQueryObject *)queryObj->privateData;
    timeout = ((pname == GL_QUERY_RESULT) ? gcvINFINITE : 0);

    gcmASSERT(chipQuery);

    if (chipQuery->querySignal == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));

    status = gcoOS_WaitSignal(chipCtx->os, chipQuery->querySignal, timeout);

    if (gcmIS_SUCCESS(status))
    {
        __GLattribute *cState = &gc->commitState;

        queryHeader = chipQuery->queryHeader;

        gcmASSERT(queryHeader->headerLocked != gcvNULL);

        gcmONERROR(gco3D_GetQuery(chipCtx->engine,
                                  chipQuery->type,
                                  &queryHeader->headerNode,
                                  queryHeader->headerSize,
                                  queryHeader->headerLocked,
                                  queryObj->index,
                                  &queryHeader->headerIndex));

        for (i = 0; i < queryHeader->headerIndex; i++)
        {
            queryObj->count += *((GLint64*)queryHeader->headerLocked + i);
        }

        if (fsProgram &&
            fsProgram->progFlags.msaaOQ &&
            chipCtx->drawRTSamples > 1 &&
            chipCtx->drawStencilView.surf &&
            cState->enables.stencilTest &&
            cState->stencil.front.testFunc == GL_EQUAL &&
            cState->stencil.back.testFunc == GL_EQUAL &&
            cState->stencil.front.reference == 0 &&
            cState->stencil.back.reference == 0 &&
            queryObj->count != 0)
        {
            gctSIZE_T num = chipCtx->drawRTWidth * chipCtx->drawRTHeight;
            GLubyte *pixels = (GLubyte*)gc->imports.malloc(gc, 4 * num);

            __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

            if (__glChipReadPixels(gc, 0, 0, (GLsizei)chipCtx->drawRTWidth, (GLsizei)chipCtx->drawRTHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels))
            {
                gctSIZE_T i;
                gctBOOL occuled = GL_FALSE;
                for (i = 0; i < num; ++i)
                {
                    if (pixels[i * 4] != 0)
                    {
                        occuled = GL_TRUE;
                        break;
                    }
                }

                if (!occuled)
                {
                    queryObj->count = 0;
                }
            }

            gc->imports.free(gc, pixels);
        }

#if gcdDUMP
        {
            gctUINT32 physical = 0;
            gcmGETHARDWAREADDRESS(queryHeader->headerNode, physical);
            gcmDUMP(gcvNULL, "#[info: verify occlusion/xfb/prim query]");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_VERIFY,
                           physical,
                           queryHeader->headerLocked,
                           0,
                           queryHeader->headerIndex * sizeof(gctUINT64)
                           );
        }
#endif

        queryObj->resultAvailable = GL_TRUE;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLvoid
__glChipDeleteQuery(
    __GLcontext *gc,
    __GLqueryObject *queryObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipQueryObject *chipQuery = (__GLchipQueryObject *)queryObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;

    if (chipQuery)
    {
        if (chipQuery->querySignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_DestroySignal(chipCtx->os, chipQuery->querySignal));
            chipQuery->querySignal = gcvNULL;
        }

        if (chipQuery->queryHeader)
        {
            __GLchipQueryHeader *queryHeader = chipQuery->queryHeader;

            if (queryHeader->headerLocked)
            {
                gcmONERROR(gcoSURF_UnLockNode(&queryHeader->headerNode, queryHeader->headerSurfType));
                queryHeader->headerLocked = gcvNULL;
            }

            gcmONERROR(gcsSURF_NODE_Destroy(&queryHeader->headerNode));

            (*gc->imports.free)(gc, queryHeader);
            chipQuery->queryHeader = gcvNULL;
        }

        (*gc->imports.free)(gc, chipQuery);

        queryObj->privateData = gcvNULL;
    }
OnError:

    return;

}

gceSTATUS
gcChipSetImageSrc(
    void * EGLImage,
    gcoSURF Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    khrEGL_IMAGE_PTR image = (khrEGL_IMAGE_PTR) EGLImage;

    if ((image == gcvNULL) ||
        (image->magic != KHR_EGL_IMAGE_MAGIC_NUM))
    {
        return gcvSTATUS_OK;
    }

    /* Lock the image mutex. */
    gcoOS_AcquireMutex(gcvNULL, image->mutex, gcvINFINITE);

    if (image->srcSurface != Surface)
    {
        if (image->srcSurface != gcvNULL)
        {
            /* Dereference old surface. */
            gcmVERIFY_OK((gcoSURF_Destroy(image->srcSurface)));
            image->srcSurface = gcvNULL;
        }

        if (image->surface != Surface)
        {
            image->srcSurface = Surface;

            if (Surface != gcvNULL)
            {
                /* Reference latest surface. */
                gcmONERROR(gcoSURF_ReferenceSurface(Surface));
            }
        }
    }

OnError:
    /* Release the image mutex. */
    gcoOS_ReleaseMutex(gcvNULL, image->mutex);
    return status;
}

GLboolean
__glChipSyncImage(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    gcmONERROR(gcChipFboSyncFromShadow(gc, gc->frameBuffer.drawFramebufObj));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipCreateSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    )
{
    gcsHAL_INTERFACE iface;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x syncObject=0x%x", gc, syncObject);

    gcoOS_CreateSignal(chipCtx->os, GL_FALSE, &syncObject->privateData);

    __glChipSyncImage(gc);

    iface.command            = gcvHAL_SIGNAL;
    iface.engine             = gcvENGINE_RENDER;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(syncObject->privateData);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = gcmPTR_TO_UINT64(gcoOS_GetCurrentProcessID());
    iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

    /* Send event. */
    gcmONERROR(gcoHAL_ScheduleEvent(gcvNULL, &iface));

    /* Commit any command buffer. Any commands after
       this point should not be related to this fence.
     */
    gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipDeleteSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x syncObject=0x%x", gc, syncObject);

    gcmONERROR(gcoOS_DestroySignal(chipCtx->os, syncObject->privateData));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLenum
__glChipWaitSync(
    __GLcontext *gc,
    __GLsyncObject *syncObject,
    GLuint64 timeout
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLenum retVal;
    gcmHEADER_ARG("gc=0x%x syncObject=0x%x timeout=%lld", gc, syncObject, timeout);

    /* Use timeout= 0 to test the signal whether be signaled or not */
    status = gcoOS_WaitSignal(chipCtx->os,
                              (gctSIGNAL)syncObject->privateData,
                              0);

    if (status == gcvSTATUS_OK)
    {
        syncObject->status = GL_SIGNALED;
        retVal = GL_ALREADY_SIGNALED;
        gcmFOOTER_ARG("return=0x%04x", retVal);
        return retVal;
    }
    else if ((timeout == 0) && (status == gcvSTATUS_TIMEOUT))
    {
        retVal = GL_TIMEOUT_EXPIRED;
        gcmFOOTER_ARG("return=0x%04x", retVal);
        return retVal;
    }
    else if (status == gcvSTATUS_TIMEOUT)
    {
        /* Wait the real timeout */
        status = gcoOS_WaitSignal(chipCtx->os,
                                  (gctSIGNAL)syncObject->privateData,
                                  (gctUINT32)(timeout/1000000));

        if (status == gcvSTATUS_OK)
        {
            syncObject->status = GL_SIGNALED;
            retVal = GL_CONDITION_SATISFIED;
            gcmFOOTER_ARG("return=0x%04x", retVal);
            return retVal;
        }
        else if (status == gcvSTATUS_TIMEOUT)
        {
            retVal = GL_TIMEOUT_EXPIRED;
            gcmFOOTER_ARG("return=0x%04x", retVal);
            return retVal;
        }
        else
        {
            retVal = GL_WAIT_FAILED;
            gcmFOOTER_ARG("return=0x%04x", retVal);
            return retVal;
        }
    }
    else
    {
        retVal = GL_WAIT_FAILED;
        gcmFOOTER_ARG("return=0x%04x", retVal);
        return retVal;
    }
}

GLvoid
__glChipBindXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLchipXfbHeader *chipXfb = (__GLchipXfbHeader *)xfbObj->privateData;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x xfbObj=0x%x", gc, xfbObj);

    if (chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        if (chipXfb == NULL)
        {
            chipXfb = (__GLchipXfbHeader *)(gc->imports.calloc)(gc, 1, sizeof(__GLchipXfbHeader));

            gcsSURF_NODE_Construct(&chipXfb->headerNode, 64, 64, gcvSURF_TFBHEADER, 0, gcvPOOL_DEFAULT);

            gcoSURF_LockNode(&chipXfb->headerNode, gcvNULL, &chipXfb->headerLocked);

            gcoOS_ZeroMemory(chipXfb->headerLocked, 64);

            gcoSURF_UnLockNode(&chipXfb->headerNode, gcvSURF_TFBHEADER);

            chipXfb->headerLocked = gcvNULL;

            xfbObj->privateData = (GLvoid*) chipXfb;
        }
    }

    gcmFOOTER_NO();
    return;
}

GLvoid
__glChipDeleteXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLchipXfbHeader *chipXfb = (__GLchipXfbHeader *)xfbObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x xfbObj=0x%x", gc, xfbObj);

    if (chipXfb)
    {
        if (chipXfb->headerLocked)
        {
            gcmONERROR(gcoSURF_UnLockNode(&chipXfb->headerNode, gcvSURF_TFBHEADER));
            chipXfb->headerLocked = gcvNULL;
        }

        gcmONERROR(gcsSURF_NODE_Destroy(&chipXfb->headerNode));

        (*gc->imports.free)(gc, chipXfb);
        xfbObj->privateData = gcvNULL;
    }

OnError:
    gcmFOOTER();
    return;
}


GLvoid
__glChipBeginXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    if (chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        gco3D_SetXfbCmd(chipCtx->engine, gcvXFBCMD_BEGIN);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid
__glChipEndXFB(
    __GLcontext *gc,
    __GLxfbObject *xfbObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x xfbObj=0x%x", gc, xfbObj);

    if (chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        gcmONERROR(gco3D_SetXfbCmd(chipCtx->engine, gcvXFBCMD_END));
    }
    else
    {
        gcmONERROR(gco3D_FlushSHL1Cache(chipCtx->engine));
    }

    gcmONERROR(gco3D_Semaphore(chipCtx->engine,
                               chipCtx->chipFeature.hwFeature.hasCommandPrefetch ? gcvWHERE_COMMAND_PREFETCH : gcvWHERE_COMMAND,
                               gcvWHERE_PIXEL,
                               gcvHOW_SEMAPHORE));

    if (gc->xfb.boundXfbObj)
    {
        __GLBufBindPoint *pXfbBindingPoints = gc->xfb.boundXfbObj->boundBufBinding;
        __GLprogramObject *progObj = gc->xfb.boundXfbObj->programObj;
        GLuint i;
        __GLchipVertexBufferInfo *chipBufInfo;
#if gcdDUMP
        gctPOINTER buffer;
        gctUINT32 physical;
        gctSIZE_T size;
        /* Flush the cache. */
        gcmONERROR(gcoSURF_Flush(gcvNULL));

        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
#endif

        if (progObj->bindingInfo.xfbMode == GL_INTERLEAVED_ATTRIBS)
        {
            GL_ASSERT(pXfbBindingPoints[0].boundBufObj);
            chipBufInfo = (__GLchipVertexBufferInfo *)pXfbBindingPoints[0].boundBufObj->privateData;
            gcmONERROR(gcoBUFOBJ_GetFence(chipBufInfo->bufObj, gcvFENCE_TYPE_WRITE));
#if gcdDUMP
            gcmVERIFY_OK(gcoBUFOBJ_Lock(chipBufInfo->bufObj,
                                        &physical,
                                        &buffer));
            gcmVERIFY_OK(gcoBUFOBJ_GetSize(chipBufInfo->bufObj, &size));

            gcmDUMP(gcvNULL, "#[info: verify xfb buffer when endxfb]");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_VERIFY,
                           physical,
                           buffer,
                           0,
                           size);

            gcmDUMP(gcvNULL, "#[info: upload stream with xfb out in case 2nd pass rendering]");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_STREAM,
                           physical,
                           buffer,
                           0,
                           size);

            gcmVERIFY_OK(gcoBUFOBJ_Unlock(chipBufInfo->bufObj));
#endif
        }
        else
        {
            GL_ASSERT(progObj->bindingInfo.xfbMode == GL_SEPARATE_ATTRIBS);
            for (i = 0; i < progObj->bindingInfo.numActiveXFB; i++)
            {
                GL_ASSERT(pXfbBindingPoints[i].boundBufObj);
                chipBufInfo = (__GLchipVertexBufferInfo *)pXfbBindingPoints[i].boundBufObj->privateData;
                gcmONERROR(gcoBUFOBJ_GetFence(chipBufInfo->bufObj, gcvFENCE_TYPE_WRITE));
#if gcdDUMP
                gcmVERIFY_OK(gcoBUFOBJ_Lock(chipBufInfo->bufObj,
                                            &physical,
                                            &buffer));
                gcmVERIFY_OK(gcoBUFOBJ_GetSize(chipBufInfo->bufObj, &size));

                gcmDUMP(gcvNULL, "#[info: verify xfb buffer when endxfb]");
                gcmDUMP_BUFFER(gcvNULL,
                               gcvDUMP_BUFFER_VERIFY,
                               physical,
                               buffer,
                               0,
                               size);

                gcmDUMP(gcvNULL, "#[info: upload stream with xfb out in case 2nd pass rendering]");
                gcmDUMP_BUFFER(gcvNULL,
                               gcvDUMP_BUFFER_STREAM,
                               physical,
                               buffer,
                               0,
                               size);

                gcmVERIFY_OK(gcoBUFOBJ_Unlock(chipBufInfo->bufObj));
#endif
            }
        }
    }

    gcmFOOTER();
    return;
OnError:
   gcChipSetError(chipCtx, status);
   gcmFOOTER();
   return;
}


GLvoid
__glChipPauseXFB(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    if (chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        gco3D_SetXfbCmd(chipCtx->engine, gcvXFBCMD_PAUSE);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid
__glChipResumeXFB(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    if (chipCtx->chipFeature.hwFeature.hasHwTFB)
    {
        gco3D_SetXfbCmd(chipCtx->engine, gcvXFBCMD_RESUME);
    }

    gcmFOOTER_NO();
    return;
}


GLvoid
__glChipGetXFBVarying(
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
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x index=%u, bufSize=%d, length=0x%x, size=0x%0x, type=%u, name=%s",
                  gc, programObject, index, bufSize, length, size, type, name);

    if (index < program->xfbCount)
    {
        gctSIZE_T nameLen = 0;
        __GLchipSLXfbVarying *xfb = &program->xfbVaryings[index];

        /* According to spec, the returned string should be null-terminated,
        ** but "length" returned should exclude null-terminator.
        */
        if (name && bufSize > 0)
        {
            nameLen = __GL_MIN(xfb->nameLen, (gctSIZE_T)bufSize - 1);
            if (nameLen > 0)
            {
                gcoOS_MemCopy(name, xfb->name, nameLen);
            }
            name[nameLen] = '\0';
        }

        if (length)
        {
            *length = (GLsizei)nameLen;
        }
        if (size)
        {
            *size = (GLsizei)xfb->arraySize;
        }
        if (type)
        {
            *type = g_typeInfos[xfb->type].glType;
        }
    }
    else
    {
        __GL_ERROR(GL_INVALID_VALUE);
    }

    gcmFOOTER_NO();
}

GLboolean
__glChipCheckXFBBufSizes(
    __GLcontext *gc,
    __GLxfbObject *xfbObj,
    GLsizei count
    )
{
    __GLprogramObject *programObj = xfbObj->programObj;
    __GLBufBindPoint *pXfbBindingPoints;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObj->privateData;
    GLboolean ret = GL_TRUE;

    /* The last bytes a XFB buffer will be written, plus 1. */
    gctUINT32 endBytes = 0;
    gctUINT32 bufSize = 0;

    gcmHEADER_ARG("gc=0x%x xfbObj=0x%x count=%d", gc, xfbObj, count);

    pXfbBindingPoints = xfbObj->boundBufBinding;

    if (programObj->bindingInfo.xfbMode == GL_INTERLEAVED_ATTRIBS)
    {
        GLuint index;

        for (index = 0; index < programObj->nextBufferCount + 1; ++index)
        {
            endBytes = program->xfbStride[index] * (count + xfbObj->offset);

            bufSize = (gctUINT)pXfbBindingPoints[index].bufSize;
            /* (bufsize == 0) indicates the whole buffer */
            if (!bufSize)
            {
                bufSize = (gctUINT)pXfbBindingPoints[index].boundBufObj->size;
            }

            if (bufSize < endBytes)
            {
                ret = GL_FALSE;
            }
        }
    }
    else
    {
        GLuint index;

        GL_ASSERT(programObj->bindingInfo.xfbMode == GL_SEPARATE_ATTRIBS);
        for (index = 0; index < program->xfbCount; ++index)
        {
            endBytes = program->xfbVaryings[index].stride * (count + xfbObj->offset);

            bufSize = (gctUINT)pXfbBindingPoints[index].bufSize;
            /* (bufsize == 0) indicates the whole buffer */
            if (!bufSize)
            {
                bufSize = (gctUINT)pXfbBindingPoints[index].boundBufObj->size;
            }

            if (bufSize < endBytes)
            {
                ret = GL_FALSE;
            }
        }
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLvoid
__glChipGetSampleLocation(
    __GLcontext *gc,
     GLuint index,
     GLfloat *val
     )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLuint i;

    gcmHEADER_ARG("gc=0x%x index=%d val=0x%x", gc, index, val);

    i = (chipCtx->drawYInverted) ? 1 : 0;

    __GL_MEMCOPY(val, &chipCtx->sampleLocations[i][index][0], sizeof(GLfloat) * 2);

    gcmFOOTER_NO();
    return;
}

GLvoid
__glChipMemoryBarrier(
    __GLcontext *gc,
    GLbitfield barriers
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x barriers=0x%x", gc, barriers);


    if (chipCtx->chipFeature.haltiLevel >= __GL_CHIP_HALTI_LEVEL_6)
    {
        if (barriers & (GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT |
                        GL_ELEMENT_ARRAY_BARRIER_BIT       |
                        GL_TEXTURE_FETCH_BARRIER_BIT       |
                        GL_PIXEL_BUFFER_BARRIER_BIT        |
                        GL_TEXTURE_UPDATE_BARRIER_BIT      |
                        GL_BUFFER_UPDATE_BARRIER_BIT       |
                        GL_FRAMEBUFFER_BARRIER_BIT         |
                        GL_UNIFORM_BARRIER_BIT             |
                        GL_TRANSFORM_FEEDBACK_BARRIER_BIT  |
                        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                        GL_ATOMIC_COUNTER_BARRIER_BIT      |
                        GL_SHADER_STORAGE_BARRIER_BIT))
        {
            gcmONERROR(gco3D_FlushSHL1Cache(chipCtx->engine));
            gcmONERROR(gco3D_Semaphore(chipCtx->engine,
                                       gcvWHERE_COMMAND,
                                       gcvWHERE_PIXEL,
                                       gcvHOW_SEMAPHORE));
        }

        if (barriers & GL_COMMAND_BARRIER_BIT)
        {
            gcmONERROR(gco3D_FlushSHL1Cache(chipCtx->engine));

            if (chipCtx->chipFeature.hwFeature.hasCommandPrefetch)
            {
                gcmONERROR(gco3D_Semaphore(chipCtx->engine,
                                           gcvWHERE_COMMAND_PREFETCH,
                                           gcvWHERE_PIXEL,
                                           gcvHOW_SEMAPHORE));
            }
            else
            {
                gcmONERROR(gco3D_Semaphore(chipCtx->engine,
                                           gcvWHERE_COMMAND,
                                           gcvWHERE_PIXEL,
                                           gcvHOW_SEMAPHORE));
            }
        }
    }
    else
    {
        gcmONERROR(gco3D_FlushSHL1Cache(chipCtx->engine));
        gcmONERROR(gco3D_Semaphore(chipCtx->engine,
                                   gcvWHERE_COMMAND,
                                   gcvWHERE_PIXEL,
                                   gcvHOW_SEMAPHORE));
    }

    gcmFOOTER_NO();
    return;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_NO();
    return;
}

GLvoid
__glChipBlendBarrier(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsSURF_VIEW *rtView0 = &chipCtx->drawRtViews[0];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (gcvNULL == chipCtx->rtTexture)
    {
        gceSURF_FORMAT format;
        gcmASSERT(rtView0->surf);
        gcmONERROR(gcoSURF_GetFormat(rtView0->surf, gcvNULL, &format));
        gcmONERROR(gcoTEXTURE_ConstructSized(chipCtx->hal,
                                             format,
                                             (gctUINT)chipCtx->drawRTWidth,
                                             (gctUINT)chipCtx->drawRTHeight,
                                             1, 1, 1, gcvPOOL_DEFAULT, &chipCtx->rtTexture));
    }

    if (chipCtx->rtTexture && rtView0->surf)
    {
        gcsSURF_VIEW texView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcmONERROR(gcoTEXTURE_GetMipMap(chipCtx->rtTexture, 0, &texView.surf));

        /* Flush all cache in pipe */
        gcmONERROR(gcoSURF_Flush(rtView0->surf));

        /* Sync texture surface from current RT */
        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted  = gcoSURF_QueryFlags(rtView0->surf, gcvSURF_FLAG_CONTENT_YINVERTED);
        rlvArgs.uArgs.v2.rectSize.x = (gctINT)chipCtx->drawRTWidth;
        rlvArgs.uArgs.v2.rectSize.y = (gctINT)chipCtx->drawRTHeight;
        rlvArgs.uArgs.v2.numSlices  = 1;
        gcmONERROR(gcoSURF_ResolveRect(rtView0, &texView, &rlvArgs));
        gcmONERROR(gcoTEXTURE_Flush(chipCtx->rtTexture));
        gcmONERROR(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));
    }

OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
    }
    gcmFOOTER_NO();
    return;
}

GLboolean
gcChipCheckRecompileEnable(
    __GLcontext *gc,
    gceSURF_FORMAT format
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsSURF_FORMAT_INFO_PTR formatInfo = gcvNULL;

    gcmHEADER_ARG("gc=0x%x", gc);

    gcmONERROR(gcoSURF_QueryFormat(format, &formatInfo));

    if ((formatInfo->fakedFormat) ||
        (formatInfo->fmtDataType != gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED &&
         formatInfo->fmtDataType != gcvFORMAT_DATATYPE_FLOAT16))
    {
        gcmFOOTER();
        return gcvTRUE;
    }

OnError:
    gcmFOOTER_NO();
    return gcvFALSE;
}


