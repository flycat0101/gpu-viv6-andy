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


#include "gc_es_context.h"
#include "gc_chip_context.h"
#include "gc_es_object_inline.c"

#define _GC_OBJ_ZONE    __GLES3_ZONE_DRAW

#if gcdFRAMEINFO_STATISTIC
GLbitfield g_dbgDumpImagePerDraw = __GL_PERDRAW_DUMP_NONE;
GLboolean  g_dbgPerDrawKickOff = GL_FALSE;
GLboolean  g_dbgSkipDraw = GL_FALSE;
GLint      g_dbgReleasePhony = 0;
#endif

extern __GLSLStage __glChipHALShaderStageToGL[];

extern GLvoid
gcChipProgFreeCmdInstance(
    __GLcontext *gc,
    GLvoid *cmdInstance
    );

gceSTATUS
gcChipValidateXFB(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    );

/**********************************************************************************************/
/*Implementation of internal functions                                                        */
/**********************************************************************************************/
#if gcdUSE_WCLIP_PATCH

#define readDataForWLimit(Logical, result)                               \
{                                                                        \
    gctUINT8* ptr;                                                       \
    gctUINT32 data;                                                      \
    ptr = (gctUINT8*)(Logical);                                          \
    data = ((ptr[0]) | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3]) << 24); \
    result = *((gctFLOAT*)(&data));                                      \
}

__GL_INLINE void
gcChipUtilMultiplyMatrices(
    gctFLOAT *matrix,
    gctFLOAT *matrix2
   )
{
    gctINT i, j;
    gctFLOAT copyMatrix[16];
    gcmHEADER_ARG("matrix=0x%x matrix2=0x%x", matrix, matrix2);

    gcoOS_MemCopy(copyMatrix, matrix, 16 * gcmSIZEOF(gctFLOAT));

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            matrix[(i * 4) + j] =
                  copyMatrix[(i * 4) + 0] * matrix2[(0 * 4) + j]
                + copyMatrix[(i * 4) + 1] * matrix2[(1 * 4) + j]
                + copyMatrix[(i * 4) + 2] * matrix2[(2 * 4) + j]
                + copyMatrix[(i * 4) + 3] * matrix2[(3 * 4) + j];
        }
    }
    gcmFOOTER_NO();
}

/* Invert 3x3 block of row-major matrix, into invMatrix. */
__GL_INLINE GLboolean
gcChipUtilInvertMatrix(
    gctFLOAT *matrix,
    gctFLOAT *invMatrix
    )
{
    gctFLOAT determinant = 0;
    gctINT i, j;
    gctFLOAT TransposeMatrix[3][3];
    gcmHEADER_ARG("matrix=0x%x invMatrix=0x%x", matrix, invMatrix);

    /* Copy transpose of top 3x3 block in matrix (4x4). */
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 3; j++)
        {
            TransposeMatrix[i][j] = matrix[(j * 4) + i];
        }
    }

    /* Compute determinant of TransposeMatrix. */
    for(i = 0, j = 0; j < 3; j++)
    {
        if (j == 2)
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][0]
                           * TransposeMatrix[i+2][1];
        }
        else if (j == 1)
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][j+1]
                           * TransposeMatrix[i+2][0];
        }
        else
        {
            determinant += TransposeMatrix[i][j]
                           * TransposeMatrix[i+1][j+1]
                           * TransposeMatrix[i+2][j+2];
        }
    }

    for(i = 2, j = 0; j < 3; j++)
    {
        if (j == 2)
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][0]
                           * TransposeMatrix[i-2][1];
        }
        else if (j == 1)
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][j+1]
                           * TransposeMatrix[i-2][0];
        }
        else
        {
            determinant -= TransposeMatrix[i][j]
                           * TransposeMatrix[i-1][j+1]
                           * TransposeMatrix[i-2][j+2];
        }
    }

    if (determinant == 0)
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    determinant = 1/determinant;

    invMatrix[0*3 + 0] = determinant *
                        ((TransposeMatrix[1][1] * TransposeMatrix[2][2])
                         - (TransposeMatrix[2][1] * TransposeMatrix[1][2]));

    invMatrix[0*3 + 1] = determinant *
                        ((TransposeMatrix[2][0] * TransposeMatrix[1][2])
                         - (TransposeMatrix[2][2] * TransposeMatrix[1][0]));

    invMatrix[0*3 + 2] = determinant *
                        ((TransposeMatrix[1][0] * TransposeMatrix[2][1])
                         - (TransposeMatrix[2][0] * TransposeMatrix[1][1]));

    invMatrix[1*3 + 0] = determinant *
                        ((TransposeMatrix[2][1] * TransposeMatrix[0][2])
                         - (TransposeMatrix[0][1] * TransposeMatrix[2][2]));

    invMatrix[1*3 + 1] = determinant *
                        ((TransposeMatrix[0][0] * TransposeMatrix[2][2])
                         - (TransposeMatrix[2][0] * TransposeMatrix[0][2]));

    invMatrix[1*3 + 2] = determinant *
                        ((TransposeMatrix[2][0] * TransposeMatrix[0][1])
                         - (TransposeMatrix[0][0] * TransposeMatrix[2][1]));

    invMatrix[2*3 + 0] = determinant *
                        ((TransposeMatrix[0][1] * TransposeMatrix[1][2])
                         - (TransposeMatrix[1][1] * TransposeMatrix[0][2]));

    invMatrix[2*3 + 1] = determinant *
                        ((TransposeMatrix[1][0] * TransposeMatrix[0][2])
                         - (TransposeMatrix[0][0] * TransposeMatrix[1][2]));

    invMatrix[2*3 + 2] = determinant *
                        ((TransposeMatrix[0][0] * TransposeMatrix[1][1])
                         - (TransposeMatrix[1][0] * TransposeMatrix[0][1]));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

__GL_INLINE void
gcChipComputeWlimitZnear(
    __GLcontext *gc
    )
{
    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);
    gctFLOAT * matrix = chipCtx->wLimitVIVMVP;
    gctFLOAT zNear = 0;
    gctFLOAT invMatrix[9];
    gctINT i, j;
    gctFLOAT matrixTran[16];

    gcmHEADER_ARG("gc=0x%x", gc);

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            matrixTran[j*4 + i] = matrix[i*4 + j];
        }
    }

    if (!gcChipUtilInvertMatrix(matrixTran, invMatrix))
    {
        zNear = 0;
    }
    else
    {
        gctFLOAT Xmv, Ymv, Zmv;

        Xmv = -1 * (invMatrix[0*3 + 0]*matrixTran[0*4 + 3] + invMatrix[0*3 + 1]*matrixTran[1*4 + 3] + invMatrix[0*3 + 2]*matrixTran[2*4 + 3]);
        Ymv = -1 * (invMatrix[1*3 + 0]*matrixTran[0*4 + 3] + invMatrix[1*3 + 1]*matrixTran[1*4 + 3] + invMatrix[1*3 + 2]*matrixTran[2*4 + 3]);
        Zmv = -1 * (invMatrix[2*3 + 0]*matrixTran[0*4 + 3] + invMatrix[2*3 + 1]*matrixTran[1*4 + 3] + invMatrix[2*3 + 2]*matrixTran[2*4 + 3]);

        zNear = (matrixTran[3*4 + 0] * Xmv) + (matrixTran[3*4 + 1] * Ymv) + (matrixTran[3*4 + 2] * Zmv) + matrixTran[3*4 + 3];
    }

    chipCtx->wLimitZNear = zNear;

    gcmFOOTER_NO();
    return;
}

__GL_INLINE gceSTATUS
gcChipComputeWlimitArg(
    __GLcontext *gc,
    __GLchipInstantDraw* instantDraw
    )
{
    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gctFLOAT *matrix = chipCtx->wLimitVIVMVP;
    gctINT i;
    gctUINT j, mat4x4Found = 0;
    __GLchipSLProgram *vsProgram;

    gcmHEADER_ARG("gc=0x%x", gc);

    chipCtx->wLimitSettled = gcvFALSE;

#if __GL_CHIP_PATCH_ENABLED
    if (chipCtx->patchInfo.patchFlags.clipW == 1)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.4f));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        chipCtx->patchInfo.patchFlags.clipW = 2;
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }
#endif

    if (chipCtx->clipW)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, chipCtx->clipWValue));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    if(chipCtx->patchId == gcvPATCH_GMMY16MAPFB && instantDraw->count == 4)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 1.0f));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    if (chipCtx->patchId == gcvPATCH_F18 && instantDraw->count == 3558)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.01f));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    if(chipCtx->patchId == gcvPATCH_ROCKSTAR_MAXPAYNE)
    {
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE));
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    /* Get the ModelViewProjectionMatrix. */
    vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
    if (!vsProgram)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.f));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE));
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    if (vsProgram->masterPgInstance->programState.hints->WChannelEqualToZ)
    {
        gco3D_SetWPlaneLimitF(chipCtx->engine, 10.0f);
        gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE);
        chipCtx->wLimitSettled = gcvTRUE;
        return status;
    }

    /* Check how many mat4x4 exist. */
    for (i = 0; i < vsProgram->userDefUniformCount; i++)
    {
        gcUNIFORM vsUniform = vsProgram->uniforms[i].halUniform[__GLSL_STAGE_VS];
        gctUINT mvpIdx = vsUniform ? gcUNIFORM_GetModelViewProjMatrix(vsUniform) : 0;
        mat4x4Found = gcmMAX(mat4x4Found, mvpIdx);
    }

    if (mat4x4Found < 1)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.0f));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE));
        if(chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
        {
            gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.1f));
            gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        }
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return status;
    }

    /* Compute the product of all the matrices,
    starting from the lowest id 1. */
    for (j = mat4x4Found; j >= 1; j--)
    {
        for (i = 0; i < vsProgram->userDefUniformCount; i++)
        {
            gcUNIFORM vsUniform = vsProgram->uniforms[i].halUniform[__GLSL_STAGE_VS];

            if (vsUniform && gcUNIFORM_GetModelViewProjMatrix(vsUniform) == j)
            {
                if (j == mat4x4Found)
                {
                    if (vsProgram->uniforms[i].data != gcvNULL )
                    {
                        /* Get the column-major matrix from the uniform. */
                        gcoOS_MemCopy(matrix, vsProgram->uniforms[i].data, 16 * gcmSIZEOF(gctFLOAT));
                    }
                    else
                    {
                        /* Set default value, if shader uses UBOs. */
                        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.01f));
                        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
                        chipCtx->wLimitSettled = gcvTRUE;
                        gcmFOOTER();
                        return status;
                    }
                }
                else
                {
                    gctFLOAT tempMatrix[16];

                    if (vsProgram->uniforms[i].data != gcvNULL )
                    {
                        /* Get the column-major matrix from the uniform. */
                        gcoOS_MemCopy(tempMatrix, vsProgram->uniforms[i].data, 16 * gcmSIZEOF(gctFLOAT));
                    }
                    else
                    {
                        /* Set default value, if shader uses UBOs. */
                        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, 0.01f));
                        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
                        chipCtx->wLimitSettled = gcvTRUE;
                        gcmFOOTER();
                        return status;
                    }

                    gcChipUtilMultiplyMatrices(matrix, tempMatrix);
                }
            }
        }
    }

    /* Convert to vivante matrix. */
    matrix[ 2] = (matrix[ 2] + matrix[ 3]) / 2.f;
    matrix[ 6] = (matrix[ 6] + matrix[ 7]) / 2.f;
    matrix[10] = (matrix[10] + matrix[11]) / 2.f;
    matrix[14] = (matrix[14] + matrix[15]) / 2.f;

    gcChipComputeWlimitZnear(gc);

OnError:
    gcmFOOTER();
    return status;
}

/* Compute the wLimit from the given column-major matrix. */
__GL_INLINE gctFLOAT
gcChipComputeWlimit(
    __GLcontext *gc,
    gctFLOAT * matrix
    )
{
    gctFLOAT wLimit = 0;
    gctFLOAT zNear = 0;
    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x", gc);

    /*zNearOrig = zNear;*/
    zNear = gcoMATH_Absolute(chipCtx->wLimitZNear);

    if (zNear != 0)
    {
        gctFLOAT temp = 0;
        gctFLOAT wMinX, wMinY, wMin;
        gctFLOAT xMax, yMax;
        gctFLOAT MaxCoordClip;
        __GLchipSLProgram *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
        gctBOOL vsPositionZDependsOnW = vsProgram
                                      ? vsProgram->curPgInstance->programState.hints->vsPositionZDependsOnW
                                      : gcvFALSE;

        /* Compute wLimit for X. */
        xMax = gcoMATH_Absolute(matrix[0])
            + gcoMATH_Absolute(matrix[4])
            + gcoMATH_Absolute(matrix[8]);

        MaxCoordClip = xMax / zNear;
        wMinX = (gc->state.viewport.width / 2) * MaxCoordClip;

        /* Compute wLimit for Y. */
        yMax = gcoMATH_Absolute(matrix[1])
            + gcoMATH_Absolute(matrix[5])
            + gcoMATH_Absolute(matrix[9]);

        MaxCoordClip = yMax / zNear;

        /* Compute max wMin. */
        wMinY = (gc->state.viewport.height / 2) * MaxCoordClip;

        wMin = gcmMAX(wMinX, wMinY);

        temp = wMin * zNear / ((1 << 21) - 1);

        if (chipCtx->wLimitRmsDirty == gcvTRUE)
        {
            temp *= chipCtx->wLimitRms;
            chipCtx->wLimitRmsDirty = gcvFALSE;
        }

        /* Check for overflow before setting wLimit. */
        if (vsPositionZDependsOnW
            || ((wMin > ((1 << 20) - 1)) && (temp > 0.1f))
            || chipCtx->clipW
            )
        {
            wLimit = temp;
        }
    }
    else
    {
        /* Special case: When zNear is 0, set a small wLimit to clip against as a fallback. */
        wLimit = 0.01f;
    }

    gcmFOOTER_ARG("return=%f", wLimit);
    return wLimit;
}

/* wClipping Patch Calculation. */
__GL_INLINE gceSTATUS
gcChipFixWlimit(
    __GLcontext *gc
    )
{
    gceSTATUS status;
    gctFLOAT wLimit = 0;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcmHEADER_ARG("gc=0x%x", gc);

    wLimit = gcChipComputeWlimit(gc, chipCtx->wLimitVIVMVP);

    /* Set wLimit. */
    if (wLimit > 0)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, wLimit));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
    }
    else
    {
        /* Special case: When zNear is 0, set a small wLimit to clip against as a fallback. */
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE));
    }

OnError:
    gcmFOOTER();
    return status;
}
__GL_INLINE gctFLOAT computeSpecailWlimit(__GLcontext * gc, __GLchipInstantDraw * instantDraw, gctINT loopIndex)
{
    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);
    gctFLOAT wlimit = 0.0f, limit = 0.0f;
    gctINT v1 = 0, v2 = 0, v3 = 0;
    gctINT vertex1 = 0, vertex2 = 0, vertex3 = 0;
    gctPOINTER indexPtr, vertexPtr;
    gctFLOAT_PTR vertexPtrs[3];
    gctINT i = 0, j = 0;
    gctINT cullMask[3] = {0, 0, 0};
    gctFLOAT fBound = 8388608.0f;
    gctFLOAT xMax = 0.0f, yMax = 0.0f;
    gctFLOAT vectors[3][4] = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    gcsATTRIBUTE_PTR attrib;
    gctFLOAT * mvp, x, y, z, w, t;
    gceSTATUS status = gcvSTATUS_OK;

    mvp = chipCtx->wLimitVIVMVP;
    attrib = &instantDraw->attributes[instantDraw->positionIndex];
    switch(instantDraw->primMode)
    {
    case gcvPRIMITIVE_TRIANGLE_LIST:
        {
            v1 = loopIndex;
            switch (loopIndex % 3)
            {
            case 2:
                v2 = v1 - 2;
                v3 = v1 - 1;
                break;
            case 1:
                v2 = v1 - 1;
                v3 = v1 + 1;
                break;
            case 0:
                v2 = v1 + 1;
                v3 = v1 + 2;
                break;
            }
            if((v3 >= (gctINT)instantDraw->count) || (v2 < 0))
            {
                return 0.0f;
            }
        }
        break;
    case gcvPRIMITIVE_TRIANGLE_STRIP:
        {
            v1 = loopIndex;
            v2 = loopIndex - 1;
            v3 = loopIndex + 1;

            if((v3 >= (gctINT)instantDraw->count) || (v2 < 0))
            {
                return  0.0f;
            }
        }
        break;
    default: /* TBD */

        return  0.0f;
    }

    if (chipCtx->indexLoops)
    {
        if (instantDraw->indexBuffer)
        {
            gcmONERROR(gcoBUFOBJ_FastLock(instantDraw->indexBuffer,gcvNULL, &indexPtr));
            indexPtr = (gctPOINTER)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(indexPtr) + gcmPTR_TO_UINT64(instantDraw->indexMemory)));
        }
        else
        {
            indexPtr = instantDraw->indexMemory;
        }

        switch(instantDraw->indexType)
        {
        case gcvINDEX_8:
            vertex1 = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + v1 * sizeof(GLubyte));
            vertex2 = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + v2 * sizeof(GLubyte));
            vertex3 = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + v3 * sizeof(GLubyte));
            break;

        case gcvINDEX_16:
            vertex1 = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + v1 * sizeof(GLushort));
            vertex2 = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + v2 * sizeof(GLushort));
            vertex3 = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + v3 * sizeof(GLushort));
            break;

        case gcvINDEX_32:
            vertex1 = *(gctUINT *)((gctUINT8_PTR)indexPtr + v1 * sizeof(GLuint));
            vertex2 = *(gctUINT *)((gctUINT8_PTR)indexPtr + v2 * sizeof(GLuint));
            vertex3 = *(gctUINT *)((gctUINT8_PTR)indexPtr + v3 * sizeof(GLuint));
            break;

        default:

            return 0.0f;
        }
    }
    else
    {
        vertex1 = v1;
        vertex2 = v2;
        vertex3 = v3;
    }

    if (attrib->stream)
    {
        gcmONERROR(gcoBUFOBJ_FastLock(attrib->stream, gcvNULL, (gctPOINTER *)&vertexPtr));
        vertexPtrs[0] = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(vertexPtr) + gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex1) * attrib->stride));
        vertexPtrs[1] = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(vertexPtr) + gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex2) * attrib->stride));
        vertexPtrs[2] = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(vertexPtr) + gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex3) * attrib->stride));
    }
    else
    {
        vertexPtrs[0]  =(gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex1) * attrib->stride));
        vertexPtrs[1]  =(gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex2) * attrib->stride));
        vertexPtrs[2]  =(gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex3) * attrib->stride));
    }

    /* Get Value */
    for(i = 0; i< 3; i++)
    {
        if((((gcmPTR2SIZE(vertexPtrs[i])) & 3) == 0))
        {
            for(j = 0; j < attrib->size; j++)
            {
                vectors[i][j] = vertexPtrs[i][j];
            }
        }
        else
        {
            for(j = 0; j < attrib->size; j++)
            {
                readDataForWLimit(vertexPtrs[i] + j, vectors[i][j])
            }
        }

        x = mvp[ 0] * vectors[i][0]
        + mvp[ 4] * vectors[i][1]
        + mvp[ 8] * vectors[i][2]
        + mvp[12] * vectors[i][3];

        y = mvp[ 1] * vectors[i][0]
        + mvp[ 5] * vectors[i][1]
        + mvp[ 9] * vectors[i][2]
        + mvp[13] * vectors[i][3];

        z = mvp[ 2] * vectors[i][0]
        + mvp[ 6] * vectors[i][1]
        + mvp[10] * vectors[i][2]
        + mvp[14] * vectors[i][3];

        w = mvp[ 3] * vectors[i][0]
        + mvp[ 7] * vectors[i][1]
        + mvp[11] * vectors[i][2]
        + mvp[15] * vectors[i][3];

        vectors[i][0] = x;
        vectors[i][1] = y;
        vectors[i][2] = z;
        vectors[i][3] = w;
        cullMask[i] = (x > w)
                      | ((x < -w) << 1)
                      | ((y > w)  << 2)
                      | ((y < -w) << 3)
                      | ((z < 0)  << 4)
                      | ((z > w)  << 5) ;

    }

    /*Interpolate between v1 v2 and v1 v3 */

    if(cullMask[0] & cullMask[1] & cullMask[2])  /* Triangle is been culled */
    {
        return 0.0f;
    }

    for(i = 1;i <= 2; i++)
    {

        if(cullMask[i] & 0x10)  /* Skip if z<0, interpolate should between v0 (z<0) and vi(z>0)*/
        {
            limit = 0.0f;
            continue;
        }
        t =  vectors[0][2] / (vectors[0][2] - vectors[i][2]);
        x = vectors[0][0] + (vectors[i][0] - vectors[0][0]) * t;
        y = vectors[0][1] + (vectors[i][1] - vectors[0][1]) * t;
        z = 0.0f;
        w = vectors[0][3] + (vectors[i][3] - vectors[0][3]) * t;

        xMax = gcoMATH_Absolute(x ) * gc->state.viewport.width / 2.0f;
        yMax = gcoMATH_Absolute(y ) * gc->state.viewport.height / 2.0f;

        if(xMax > fBound * gcoMATH_Absolute(w) || yMax > fBound * gcoMATH_Absolute(w))
        {
            /* If use wlimit value, the interpolate is
                 t = (w0 -wL)/(w0 -w1);
                 x = (x1 - x0)*t + x0;
                 y = (y1 - y0)*t + y0;
                 w = wl;

                so, x/wl * width/ 2 < fbound
                     y/wl * height/2 < fbound.
            */
            gctFLOAT a,b;
            a = (vectors[i][0] - vectors[0][0]) / (vectors[0][3] - vectors[i][3]);
            b = vectors[0][0] + a * vectors[0][3];
            xMax = b / (fBound * 2.0f / gc->state.viewport.width + a);

            a = (vectors[i][1] - vectors[0][1])/(vectors[0][3] - vectors[i][3]);
            b = vectors[0][1] + a * vectors[0][3];

            yMax = b / (fBound * 2.0f / gc->state.viewport.height + a);
            limit = xMax > yMax ? xMax : yMax;

            if(limit > wlimit)
                wlimit = limit;
        }
    }

OnError:

    return wlimit;
}

__GL_INLINE gceSTATUS
gcChipComputeWlimitByVertex(
    __GLcontext *gc,
    __GLchipInstantDraw* instantDraw
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
    gctFLOAT limit = 0.0f;
    gctFLOAT wlimit = 0.0f;
    gctFLOAT fBound = 8388608.0f;
    gctSIZE_T i;
    gctINT j;
    gctSIZE_T sampleCount, sampleStep;
    gctFLOAT * vertexPtr = gcvNULL;
    gctFLOAT zNear = gcoMATH_Absolute(chipCtx->wLimitZNear);
    gctBOOL ok = gcvFALSE;
    gctBOOL disableWlimit = gcvTRUE;
    gcsATTRIBUTE_PTR attrib;
    gctPOINTER indexPtr = gcvNULL;
    gctFLOAT * mvp;
    gctFLOAT refZ = 0.0f;
    gctFLOAT refW = 0.0f;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x instantDraw=%d", gc, instantDraw);

    attrib = &instantDraw->attributes[instantDraw->positionIndex];

    /* Float vertex attribute only, not for divisor */
    if (chipCtx->wLimitPSC)
    {
        gco3D_SetWPlaneLimitF(chipCtx->engine, 0.5f);
        gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE);
        chipCtx->wLimitSettled = gcvTRUE;
        return gcvSTATUS_OK;
    }

    /* Float vertex attribute only, not for divisor */
    if (attrib->divisor || attrib->format != gcvVERTEX_FLOAT || !attrib->enable || zNear == 0.0f)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (attrib->size <= 2)
    {
        gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE);
        chipCtx->wLimitSettled = gcvTRUE;
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (vsProgram &&
        !vsProgram->masterPgInstance->programState.hints->strictWClipMatch &&
        (chipCtx->patchId !=  gcvPATCH_AFTERBURNER)
       )
    {
         /* Try old method if glPosition is not equal MVP x vpos */
        gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE);
        chipCtx->wLimitSettled = gcvFALSE;

        if(chipCtx->patchId == gcvPATCH_RIPTIDEGP2 || chipCtx->patchId == gcvPATCH_NAMESGAS)
        {
            gco3D_SetWPlaneLimitF(chipCtx->engine, 0.01f);
            gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE);
            chipCtx->wLimitSettled = gcvTRUE;
        }
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    sampleCount = chipCtx->wLimitSampleCount > instantDraw->count ? instantDraw->count : chipCtx->wLimitSampleCount;
    sampleStep =  instantDraw->count / sampleCount;

    for(i=0; i < instantDraw->count; i += sampleStep)
    {
        gctFLOAT vector[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        gctFLOAT x, y, z, w,absX,absY,absW;
        gctSIZE_T vertex;

        if (chipCtx->indexLoops)
        {
            if (instantDraw->indexBuffer)
            {
                gcmONERROR(gcoBUFOBJ_FastLock(instantDraw->indexBuffer,gcvNULL, &indexPtr));
                indexPtr = (gctPOINTER)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(indexPtr) + gcmPTR_TO_UINT64(instantDraw->indexMemory)));
            }
            else
            {
                indexPtr = instantDraw->indexMemory;
            }

            switch(instantDraw->indexType)
            {
            case gcvINDEX_8:
                vertex = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + i * sizeof(GLubyte));
                break;

            case gcvINDEX_16:
                vertex = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + i * sizeof(GLushort));
                break;

            case gcvINDEX_32:
                vertex = *(gctUINT *)((gctUINT8_PTR)indexPtr + i * sizeof(GLuint));
                break;

            default:
                vertex = 0;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                break;
            }
        }
        else
        {
             vertex = (gctINT)i;
        }

        if (attrib->stream)
        {
            gcmONERROR(gcoBUFOBJ_FastLock(attrib->stream, gcvNULL, (gctPOINTER *)&vertexPtr));
            vertexPtr = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(vertexPtr) + gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex) * attrib->stride));
        }
        else
        {
            vertexPtr =(gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex) * attrib->stride));
        }


        /* Get Value */
        if((((gcmPTR2SIZE(vertexPtr)) & 3) == 0))
        {
            for(j=0 ; j < attrib->size; j++)
            {
                vector[j] = vertexPtr[j];
            }
        }
        else
        {
            for(j=0 ; j < attrib->size; j++)
            {

                readDataForWLimit(vertexPtr + j, vector[j])
            }

        }

        mvp = chipCtx->wLimitVIVMVP;

        x = mvp[ 0] * vector[0]
          + mvp[ 4] * vector[1]
          + mvp[ 8] * vector[2]
          + mvp[12] * vector[3];

        y = mvp[ 1] * vector[0]
          + mvp[ 5] * vector[1]
          + mvp[ 9] * vector[2]
          + mvp[13] * vector[3];

        z = mvp[ 2] * vector[0]
          + mvp[ 6] * vector[1]
          + mvp[10] * vector[2]
          + mvp[14] * vector[3];

        w = mvp[ 3] * vector[0]
          + mvp[ 7] * vector[1]
          + mvp[11] * vector[2]
          + mvp[15] * vector[3];

        if(i == 0)
        {
            refZ = z;
            refW = w;
        }
        else if(!(refZ == z && refW == w))
        {
            disableWlimit = gcvFALSE;
        }

        absX = gcoMATH_Absolute(x);
        absY = gcoMATH_Absolute(y);
        absW = gcoMATH_Absolute(w);

        if (absX < absW && absY < absW && z > 0.0f)
        {
            /* Do nothing */
        }
        else if (z > 0.0f && w > 0)
        {
            gctFLOAT xmax = absX  * gc->state.viewport.width / 2.0f;
            gctFLOAT ymax = absY  * gc->state.viewport.height / 2.0f;

            if(xmax > fBound * absW || ymax > fBound * absW)
            {
                gctFLOAT max = xmax > ymax ? xmax : ymax;

                limit = max  / ((1 << 22) - 1);
            }
        }
        else if (z < 0.0f)
        {
            gctFLOAT xmax = absX / zNear * gc->state.viewport.width / 2.0f;
            gctFLOAT ymax = absY / zNear * gc->state.viewport.height / 2.0f;

            if(xmax > fBound || ymax > fBound)
            {
                gctFLOAT max = xmax > ymax ? xmax : ymax;

                limit = max * zNear / ((1 << 22) - 1);
            }

            if(w < 0.0f)
            {
                float minWlimt = 0.0f;
                minWlimt= computeSpecailWlimit(gc, instantDraw, (gctINT) i);

                if(minWlimt > limit)
                {
                    limit = minWlimt;
                }
            }
        }
         else if ((z > 0.0f && w < 0.0) && ((chipCtx-> patchId== gcvPATCH_F18NEW) || (chipCtx->patchId == gcvPATCH_AIRNAVY)))
        {
            gctFLOAT xmax = absX  * gc->state.viewport.width / 2.0f;
            gctFLOAT ymax = absY  * gc->state.viewport.height / 2.0f;

            gctFLOAT max = xmax > ymax ? xmax : ymax;
            limit = max  / ((1 << 24) - 1);
        }
        else
        {
            gcmTRACE(gcvLEVEL_ERROR,"wlimit can't handle this issue");
        }

        if(limit > wlimit)
        {
            wlimit = limit;
            ok = gcvTRUE;
        }
    }

    if (disableWlimit)
    {
        gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE);
        chipCtx->wLimitSettled = gcvTRUE;
    }
    else if (ok)
    {
        gcmONERROR(gco3D_SetWPlaneLimitF(chipCtx->engine, wlimit));
        gcmONERROR(gco3D_SetWClipEnable(chipCtx->engine, gcvTRUE));
        chipCtx->wLimitSettled = gcvTRUE;
    }
    else
    {
        gco3D_SetWClipEnable(chipCtx->engine, gcvFALSE);
    }

OnError:
    gcmFOOTER();
    return status;
}
#endif

#define gcmES30_COLLECT_STREAM_INFO(streamInfo, instantDraw, gc, chipCtx, bInstance) \
    streamInfo.attribMask = instantDraw->attribMask; \
    streamInfo.u.es30.attributes = instantDraw->attributes; \
    streamInfo.first = instantDraw->first; \
    streamInfo.count = instantDraw->count; \
    streamInfo.instanced = bInstance; \
    streamInfo.instanceCount = gc->vertexArray.instanceCount; \
    streamInfo.primMode = instantDraw->primMode; \
    streamInfo.vertexInstIndex = gcSHADER_GetVertexInstIdInputIndex(chipCtx->activePrograms[__GLSL_STAGE_VS]->masterPgInstance->binaries[__GLSL_STAGE_VS]); \
    streamInfo.primCount = instantDraw->primCount

#define gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw) \
    indexInfo.count = instantDraw->count; \
    indexInfo.indexType = instantDraw->indexType; \
    indexInfo.u.es30.indexBuffer = instantDraw->indexBuffer; \
    indexInfo.indexMemory = instantDraw->indexMemory

__GL_INLINE gceSTATUS
gcChipSetVertexArrayBindBegin(
    IN __GLcontext*         gc,
    IN __GLchipInstantDraw* instantDraw,
    IN gctBOOL              fixWLimit
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER();

#if gcdUSE_WCLIP_PATCH
    /* WClipping Patch. */
    if (fixWLimit && chipCtx->wLimitPatch
#if __GL_CHIP_PATCH_ENABLED
        && (chipCtx->patchInfo.patchFlags.clipW < 2)
#endif
        )
    {
        gcmONERROR(gcChipComputeWlimitArg(gc, instantDraw));

        if (chipCtx->computeWlimitByVertex &&
            !chipCtx->wLimitSettled &&
            instantDraw->count <= chipCtx->wLimitComputeLimit &&
            instantDraw->positionIndex != -1)
        {
            gcmONERROR(gcChipComputeWlimitByVertex(gc, instantDraw));
        }
    }
#endif

    /* Need to convert to triangle list? */
    if(instantDraw->primMode == gcvPRIMITIVE_LINE_LOOP)
    {
        /*for line loop the last line is automatic added in hw implementation*/
        instantDraw->primCount = instantDraw->primCount - 1;
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetVertexArrayBindEnd(
    IN __GLcontext*         gc,
    IN __GLchipInstantDraw* instantDraw,
    IN gctBOOL              fixWLimit
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER();

#if gcdUSE_WCLIP_PATCH
    /* WClipping Patch. */
    if (fixWLimit && chipCtx->wLimitPatch  && !chipCtx->wLimitSettled
#if __GL_CHIP_PATCH_ENABLED
        && (chipCtx->patchInfo.patchFlags.clipW < 2)
#endif
        )
    {
        gcmONERROR(gcChipFixWlimit(gc));
    }
#endif

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetVertexArrayBind(
    IN __GLcontext*         gc,
    IN __GLchipInstantDraw* instantDraw,
    IN gctBOOL              fixWLimit,
    IN gctBOOL              instanced
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsVERTEXARRAY_STREAM_INFO streamInfo;
    gcsVERTEXARRAY_INDEX_INFO  indexInfo;

    gcmHEADER_ARG("gc=0x%x instanced=%d", gc, instanced);

    gcmONERROR(gcChipSetVertexArrayBindBegin(gc, instantDraw, fixWLimit));
    /* Collect info for hal level.*/
    gcmES30_COLLECT_STREAM_INFO(streamInfo, instantDraw, gc, chipCtx, instanced);
    gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw);

    /* Bind the vertex array to the hardware. */
#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRms,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRmsDirty,
                                         &streamInfo,
                                         &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         &streamInfo,
                                         &indexInfo));
#endif

    gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                        &indexInfo));

    gcmONERROR(gcChipSetVertexArrayBindEnd(gc, instantDraw, fixWLimit));

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE GLvoid *
gcChipPatchClaimIndexMemory (
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    gctSIZE_T size
    )
{
    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x size=%u", gc, chipCtx, size);

    /* Do we need more space or the required size is too few?? */
    if ((chipCtx->tempIndexBufferSize < size)
        || (chipCtx->tempIndexBufferSize > (size * 5)) )
    {
        /* Free previous buffer */
        if (chipCtx->tempIndexBuffer != gcvNULL)
        {
            (*gc->imports.free)(0, chipCtx->tempIndexBuffer);
        }

        /* Reallocate buffer */
        chipCtx->tempIndexBuffer = (*gc->imports.malloc)(gc, size);
        chipCtx->tempIndexBufferSize = size;
    }

    gcmFOOTER_ARG("buffer=0x%x bufferSize=%u", chipCtx->tempIndexBuffer, chipCtx->tempIndexBufferSize);

    return chipCtx->tempIndexBuffer;
}

static gceSTATUS
gcChipPatchLineStripIndexed(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gctBOOL PrimitiveRestart
    )
{
    gctSIZE_T newIndexCount = 0;
    gctSIZE_T oldIndexCount = instantDraw->count;
    gctSIZE_T indexSize     = 0;
    gctSIZE_T requiredSize  = 0;
    GLvoid * indexMemory = instantDraw->indexMemory;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ oldIndexBuffer = instantDraw->indexBuffer;
    gctBOOL indexLocked = gcvFALSE;
    gctSIZE_T i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    requiredSize  = instantDraw->primCount * 2 * indexSize;
    /* Get index buffer */
    if (oldIndexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(oldIndexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    /* Claim buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);

    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (instantDraw->indexType)
    {
    case gcvINDEX_8:
        {
            gctUINT8_PTR src = (gctUINT8_PTR)indexMemory;
            gctUINT8_PTR ptr = (gctUINT8_PTR)tempIndices;

            for (i = 0; i < (oldIndexCount - 1); ++i)
            {
                if (!PrimitiveRestart || ((src[i] != 0xFF) && (src[i+1] != 0xFF)))
                {
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 2;
                }
            }
        }
        break;

    case gcvINDEX_16:
        {
            gctUINT16_PTR src = (gctUINT16_PTR)indexMemory;
            gctUINT16_PTR ptr = (gctUINT16_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 0; i < (oldIndexCount - 1); ++i)
            {
                if (!PrimitiveRestart || ((src[i] != 0xFFFF) && (src[i+1] != 0xFFFF)))
                {
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 2;
                }
            }
        }
        break;

    case gcvINDEX_32:
        {
            gctUINT32_PTR src = (gctUINT32_PTR)indexMemory;
            gctUINT32_PTR ptr = (gctUINT32_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 0; i < (oldIndexCount - 1); ++i)
            {
                if (!PrimitiveRestart || ((src[i] != 0xFFFFFFFF) && (src[i+1] != 0xFFFFFFFF)))
                {
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 2;
                }
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    instantDraw->primMode = gcvPRIMITIVE_LINE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->primCount = newIndexCount / 2;

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(oldIndexBuffer));
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchLineStrip(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    gctSIZE_T newPrimitiveCount;
    gctSIZE_T newIndexCount;
    gctSIZE_T first;
    gctSIZE_T vertexCount;
    gceINDEX_TYPE indexType;
    gctSIZE_T requiredSize;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Get values */
    newPrimitiveCount = instantDraw->primCount;
    newIndexCount = newPrimitiveCount * 2;
    first = instantDraw->first;
    vertexCount = (gctSIZE_T)(gc->vertexArray.end - gc->vertexArray.start);
    requiredSize = 0;

    /* Check if the count fits in 8-bit. */
    if (first + vertexCount + 1 < 256)
    {
        /* 8-bit indices. */
        indexType     = gcvINDEX_8;
        requiredSize  = newIndexCount;
    }

    /* Check if the count fits in 16-bit. */
    else if (first + vertexCount + 1 < 65536)
    {
        /* 16-bit indices. */
        indexType     = gcvINDEX_16;
        requiredSize  = newIndexCount * 2;
    }

    else
    {
        /* 32-bit indices. */
        indexType     = gcvINDEX_32;
        requiredSize  = newIndexCount * 4;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (indexType)
    {
    case gcvINDEX_8:
        {
            /* Cast pointer to index buffer. */
            gctUINT8_PTR ptr = (gctUINT8_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT8) (first + i);
                *ptr++ = (gctUINT8) (first + i + 1);
            }
        }
        break;

    case gcvINDEX_16:
        {
            /* Cast pointer to index buffer. */
            gctUINT16_PTR ptr = (gctUINT16_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT16) (first + i);
                *ptr++ = (gctUINT16) (first + i + 1);
            }
        }
        break;

    case gcvINDEX_32:
        {
            /* Cast pointer to index buffer. */
            gctUINT32_PTR ptr = (gctUINT32_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT32)(first + i);
                *ptr++ = (gctUINT32)(first + i + 1);
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    chipCtx->indexLoops = 1;
    instantDraw->first = 0;
    instantDraw->primMode = gcvPRIMITIVE_LINE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->indexType = indexType;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchLineLoopIndexed(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gctBOOL PrimitiveRestart
    )
{
    gctSIZE_T newIndexCount = 0;
    gctSIZE_T oldIndexCount = instantDraw->count;
    gctSIZE_T indexSize     = 0;
    gctSIZE_T requiredSize  = 0;
    GLvoid * indexMemory = instantDraw->indexMemory;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ oldIndexBuffer = instantDraw->indexBuffer;
    gctBOOL indexLocked = gcvFALSE;
    gctBOOL restart = gcvTRUE;
    gctSIZE_T i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    requiredSize  = instantDraw->primCount * 2 * indexSize;
    /* Get index buffer */
    if (oldIndexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(oldIndexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    /* Claim buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (instantDraw->indexType)
    {
    case gcvINDEX_8:
        {
            gctUINT8_PTR src = (gctUINT8_PTR)indexMemory;
            gctUINT8_PTR ptr = (gctUINT8_PTR)tempIndices;

            /* Define pivot */
            gctUINT8 pivot = 0;

            for (i = 0; i < oldIndexCount - 1; ++i)
            {
                if (PrimitiveRestart && (src[i] == 0xFF || src[i+1] == 0xFF))
                {
                    if (src[i] == 0xFF)
                    {
                        restart = gcvTRUE;
                    }
                    else if (src[i+1] == 0xFF && !restart)
                    {
                        *ptr++ = src[i];
                        *ptr++ = pivot;
                        newIndexCount += 2;
                    }
                }
                else
                {
                    if (restart)
                    {
                        pivot = src[i];
                        restart = gcvFALSE;
                    }
                    *ptr++ = src[i];
                    *ptr++ = src[i + 1];
                    newIndexCount += 2;
                }
            }

            if (!(PrimitiveRestart && (restart || src[oldIndexCount - 1] == 0xFF)))
            {
                /* Append the last index */
                *ptr++ = src[oldIndexCount - 1];
                *ptr++ = pivot;
                newIndexCount += 2;
            }
        }
        break;

    case gcvINDEX_16:
        {
            gctUINT16_PTR src = (gctUINT16_PTR)indexMemory;
            gctUINT16_PTR ptr = (gctUINT16_PTR)tempIndices;

            /* Define pivot */
            gctUINT16 pivot = 0;

            for (i = 0; i < oldIndexCount - 1; ++i)
            {
                if (PrimitiveRestart && (src[i] == 0xFFFF || src[i+1] == 0xFFFF))
                {
                    if (src[i] == 0xFFFF)
                    {
                        restart = gcvTRUE;
                    }
                    else if (src[i+1] == 0xFFFF && !restart)
                    {
                        *ptr++ = src[i];
                        *ptr++ = pivot;
                        newIndexCount += 2;
                    }
                }
                else
                {
                    if (restart)
                    {
                        pivot = src[i];
                        restart = gcvFALSE;
                    }
                    *ptr++ = src[i];
                    *ptr++ = src[i + 1];
                    newIndexCount += 2;
                }
            }

            if (!(PrimitiveRestart && (restart || src[oldIndexCount - 1] == 0xFFFF)))
            {
                /* Append the last index */
                *ptr++ = src[oldIndexCount - 1];
                *ptr++ = pivot;
                newIndexCount += 2;
            }
        }
        break;

    case gcvINDEX_32:
        {
            gctUINT32_PTR src = (gctUINT32_PTR)indexMemory;
            gctUINT32_PTR ptr = (gctUINT32_PTR)tempIndices;

            /* Define pivot */
            gctUINT32 pivot = 0;

            for (i = 0; i < oldIndexCount - 1; ++i)
            {
                if (PrimitiveRestart && (src[i] == 0xFFFFFFFF || src[i+1] == 0xFFFFFFFF))
                {
                    if (src[i] == 0xFFFFFFFF)
                    {
                        restart = gcvTRUE;
                    }
                    else if (src[i+1] == 0xFFFFFFFF && !restart)
                    {
                        *ptr++ = src[i];
                        *ptr++ = pivot;
                        newIndexCount += 2;
                    }
                }
                else
                {
                    if (restart)
                    {
                        pivot = src[i];
                        restart = gcvFALSE;
                    }
                    *ptr++ = src[i];
                    *ptr++ = src[i + 1];
                    newIndexCount += 2;
                }
            }

            if (!(PrimitiveRestart && (restart || src[oldIndexCount - 1] == 0xFFFFFFFF)))
            {
                /* Append the last index */
                *ptr++ = src[oldIndexCount - 1];
                *ptr++ = pivot;
                newIndexCount += 2;
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    instantDraw->primMode = gcvPRIMITIVE_LINE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->primCount = ((newIndexCount/2) > 1 ? newIndexCount/2 : 0);

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(oldIndexBuffer));
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchLineLoop(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    gctSIZE_T newPrimitiveCount;
    gctSIZE_T newIndexCount;
    gctSIZE_T first;
    gctSIZE_T vertexCount;
    gceINDEX_TYPE indexType;
    gctSIZE_T requiredSize;
    GLvoid * tempIndices = gcvNULL;
    gctSIZE_T i;
    gceSTATUS status = gcvSTATUS_OK;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Get values */
    newPrimitiveCount = instantDraw->primCount;
    newIndexCount = newPrimitiveCount * 2;
    first = instantDraw->first;
    vertexCount = (gctSIZE_T)(gc->vertexArray.end - gc->vertexArray.start);
    requiredSize = 0;

    /* Check if the count fits in 8-bit. */
    if (first + vertexCount + 1 < 256)
    {
        /* 8-bit indices. */
        indexType     = gcvINDEX_8;
        requiredSize  = newIndexCount;
    }

    /* Check if the count fits in 16-bit. */
    else if (first + vertexCount + 1 < 65536)
    {
        /* 16-bit indices. */
        indexType     = gcvINDEX_16;
        requiredSize  = newIndexCount * 2;
    }

    else
    {
        /* 32-bit indices. */
        indexType     = gcvINDEX_32;
        requiredSize  = newIndexCount * 4;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (indexType)
    {
    case gcvINDEX_8:
        {
            /* Cast pointer to index buffer. */
            gctUINT8_PTR ptr = (gctUINT8_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < vertexCount; ++i)
            {
                *ptr++ = (gctUINT8) (first + i);
                if (i == vertexCount - 1)
                {
                    *ptr++ = (gctUINT8) (first);
                }
                else
                {
                    *ptr++ = (gctUINT8) (first + i + 1);
                }
            }
        }
        break;

    case gcvINDEX_16:
        {
            /* Cast pointer to index buffer. */
            gctUINT16_PTR ptr = (gctUINT16_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < vertexCount; ++i)
            {
                *ptr++ = (gctUINT16) (first + i);
                if (i == vertexCount - 1)
                {
                    *ptr++ = (gctUINT16) (first);
                }
                else
                {
                    *ptr++ = (gctUINT16) (first + i + 1);
                }
            }
        }
        break;

    case gcvINDEX_32:
        {
            /* Cast pointer to index buffer. */
            gctUINT32_PTR ptr = (gctUINT32_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < vertexCount; ++i)
            {
                *ptr++ =  (gctUINT32)(first + i);
                if (i == vertexCount - 1)
                {
                    *ptr++ =  (gctUINT32)first;
                }
                else
                {
                    *ptr++ =  (gctUINT32)(first + i + 1);
                }
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    chipCtx->indexLoops = 1;
    instantDraw->first = 0;
    instantDraw->primMode = gcvPRIMITIVE_LINE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->indexType = indexType;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchTriangleStripIndexed_cached(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    __GLbufferObject *idxBufObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
    __GLchipVertexBufferInfo *bufInfo = NULL;
    gcoBUFOBJ * pShadowIdxObj = gcvNULL;
    GLboolean   newCreated  = GL_FALSE;
    gctSIZE_T   indexOffset = __GL_PTR2SIZE(instantDraw->indexMemory);
    gctSIZE_T   elementSize = 0;
    GLboolean   oddOffset;    /* Whether start from an odd offset from the buffer? */
    GLubyte *   pSrcAddr = gcvNULL;
    GLubyte *   pDstAddr = gcvNULL;
    gceSTATUS   status = gcvSTATUS_OK;
    gctSIZE_T   unAlignedStep = 0;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, elementSize);
    unAlignedStep = indexOffset % elementSize;
    /* There must be index buffer bound for this draw */
    GL_ASSERT(idxBufObj);
    bufInfo = (__GLchipVertexBufferInfo*)idxBufObj->privateData;

    /* Reset patched triangle list indices if dirtied. */
    if (bufInfo->patchDirty || (bufInfo->unAlignedStep != unAlignedStep))
    {
        if (bufInfo->listIndexEven)
        {
            gcmONERROR(gcoBUFOBJ_Destroy(bufInfo->listIndexEven));
            bufInfo->listIndexEven = gcvNULL;
        }

        if (bufInfo->listIndexOdd)
        {
            gcmONERROR(gcoBUFOBJ_Destroy(bufInfo->listIndexOdd));
            bufInfo->listIndexOdd = gcvNULL;
        }

        bufInfo->patchDirty = gcvFALSE;
    }

    /* Make sure the offset must be aligned to elementSize. */
    GL_ASSERT(indexOffset % elementSize == 0);

    oddOffset = (indexOffset / elementSize) % 2 ? GL_TRUE : GL_FALSE;
    pShadowIdxObj = oddOffset ? &bufInfo->listIndexOdd : &bufInfo->listIndexEven;

    if (*pShadowIdxObj == gcvNULL)
    {
        gctSIZE_T i;
        /* Calc triangle list indices for all the even/odd indices onces */
        gctSIZE_T triangles = bufInfo->size / elementSize - (oddOffset ? 3 : 2);

        /* Lock the master bufObj */
        gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&pSrcAddr));
        pSrcAddr += unAlignedStep;
        if (oddOffset)
        {
            pSrcAddr += elementSize;
        }

        gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_ELEMENT_ARRAY_BUFFER, pShadowIdxObj));
        newCreated = GL_TRUE;
        gcmONERROR(gcoBUFOBJ_Upload(*pShadowIdxObj, gcvNULL, 0, triangles * 3 * elementSize, gcvBUFOBJ_USAGE_STATIC_DRAW));
        gcmONERROR(gcoBUFOBJ_Lock(*pShadowIdxObj, gcvNULL, (gctPOINTER*)&pDstAddr));

        /* Dispatch on index type. */
        switch (instantDraw->indexType)
        {
        case gcvINDEX_8:
            {
                gctUINT8_PTR src = (gctUINT8_PTR)pSrcAddr;
                gctUINT8_PTR dst = (gctUINT8_PTR)pDstAddr;

                /* Fill index buffer */
                for (i = 0; i < triangles; ++i)
                {
                    *dst++ = src[(i % 2) == 0 ? i : i + 1];
                    *dst++ = src[(i % 2) == 0 ? i + 1 : i];
                    *dst++ = src[i+2];
                }
            }
            break;

        case gcvINDEX_16:
            {
                gctUINT16_PTR src = (gctUINT16_PTR)pSrcAddr;
                gctUINT16_PTR dst = (gctUINT16_PTR)pDstAddr;

                /* Fill index buffer */
                for (i = 0; i < triangles; ++i)
                {
                    *dst++ = src[(i % 2) == 0 ? i : i + 1];
                    *dst++ = src[(i % 2) == 0 ? i + 1 : i];
                    *dst++ = src[i+2];
                }
            }
            break;

        case gcvINDEX_32:
            {
                gctUINT32_PTR src = (gctUINT32_PTR)pSrcAddr;
                gctUINT32_PTR dst = (gctUINT32_PTR)pDstAddr;

                /* Fill index buffer */
                for (i = 0; i < triangles; ++i)
                {
                    *dst++ = src[(i % 2) == 0 ? i : i + 1];
                    *dst++ = src[(i % 2) == 0 ? i + 1 : i];
                    *dst++ = src[i+2];
                }
            }
            break;

        default:
            break;
        }

        gcmONERROR(gcoBUFOBJ_Unlock(*pShadowIdxObj));
        /* Flush CPU cache */
        gcmONERROR(gcoBUFOBJ_CPUCacheOperation(*pShadowIdxObj, gcvCACHE_CLEAN));
    }

    bufInfo->unAlignedStep = unAlignedStep;
    instantDraw->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
    instantDraw->indexMemory = __GL_SIZE2PTR(3 * (indexOffset - unAlignedStep - (oddOffset ? elementSize : 0)));
    instantDraw->indexBuffer = *pShadowIdxObj;
    instantDraw->primCount = instantDraw->count - 2;
    instantDraw->count = instantDraw->primCount * 3;

OnError:
    if (pSrcAddr)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(bufInfo->bufObj));
    }

    /* Destroy the newly created shadow idxObj if any error */
    if (gcmIS_ERROR(status) && newCreated && *pShadowIdxObj)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Destroy(*pShadowIdxObj));
        *pShadowIdxObj = gcvNULL;
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchTriangleStripIndexed(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gctBOOL PrimitiveRestart
    )
{
    gctSIZE_T newIndexCount = 0;
    gctSIZE_T oldIndexCount = instantDraw->count;
    gctSIZE_T indexSize     = 0;
    gctSIZE_T requiredSize  = 0;
    GLvoid * indexMemory = instantDraw->indexMemory;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ oldIndexBuffer = instantDraw->indexBuffer;
    gctBOOL indexLocked = gcvFALSE;
    gctSIZE_T i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    requiredSize  = instantDraw->primCount * 3 * indexSize;
    /* Get index buffer */
    if (oldIndexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(oldIndexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    /* Claim buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (instantDraw->indexType)
    {
    case gcvINDEX_8:
        {
            gctUINT8_PTR src = (gctUINT8_PTR)indexMemory;
            gctUINT8_PTR ptr = (gctUINT8_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 0; i < (oldIndexCount - 2); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[i] != 0xFF)
                        && (src[i+1] != 0xFF)
                        && (src[i+2] != 0xFF)))
                {
                    *ptr++ = src[(i % 2) == 0 ? i : i + 1];
                    *ptr++ = src[(i % 2) == 0 ? i + 1 : i];
                    *ptr++ = src[i+2];
                    newIndexCount += 3;
                }
            }
        }
        break;

    case gcvINDEX_16:
        {
            gctUINT16_PTR src = (gctUINT16_PTR)indexMemory;
            gctUINT16_PTR ptr = (gctUINT16_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 0; i < (oldIndexCount - 2); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[i] != 0xFFFF)
                        && (src[i+1] != 0xFFFF)
                        && (src[i+2] != 0xFFFF)))
                {
                    *ptr++ = src[(i % 2) == 0 ? i : i + 1];
                    *ptr++ = src[(i % 2) == 0 ? i + 1 : i];
                    *ptr++ = src[i+2];
                    newIndexCount += 3;
                }
            }
        }
        break;

    case gcvINDEX_32:
        {
            gctUINT32_PTR src = (gctUINT32_PTR)indexMemory;
            gctUINT32_PTR ptr = (gctUINT32_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 0; i < (oldIndexCount - 2); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[i] != 0xFFFFFFFF)
                        && (src[i+1] != 0xFFFFFFFF)
                        && (src[i+2] != 0xFFFFFFFF)))
                {
                    *ptr++ = src[(i % 2) == 0 ? i : i + 1];
                    *ptr++ = src[(i % 2) == 0 ? i + 1 : i];
                    *ptr++ = src[i+2];
                    newIndexCount += 3;
                }

            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    instantDraw->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->primCount = newIndexCount / 3;

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(oldIndexBuffer));
    }
    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
gcChipPatchTriangleStrip(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    gctSIZE_T newPrimitiveCount;
    gctSIZE_T newIndexCount;
    gctSIZE_T first;
    gctSIZE_T vertexCount;
    gceINDEX_TYPE indexType;
    gctSIZE_T requiredSize;
    GLvoid * tempIndices = gcvNULL;
    gctSIZE_T i;
    gceSTATUS status = gcvSTATUS_OK;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Get values */
    newPrimitiveCount = instantDraw->primCount;
    newIndexCount = newPrimitiveCount * 3;
    first = instantDraw->first;
    vertexCount = (gctSIZE_T)(gc->vertexArray.end - gc->vertexArray.start);
    requiredSize = 0;

    /* Check if the count fits in 8-bit. */
    if (first + vertexCount + 1 < 256)
    {
        /* 8-bit indices. */
        indexType     = gcvINDEX_8;
        requiredSize  = newIndexCount;
    }

    /* Check if the count fits in 16-bit. */
    else if (first + vertexCount + 1 < 65536)
    {
        /* 16-bit indices. */
        indexType     = gcvINDEX_16;
        requiredSize  = newIndexCount * 2;
    }

    else
    {
        /* 32-bit indices. */
        indexType     = gcvINDEX_32;
        requiredSize  = newIndexCount * 4;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (indexType)
    {
    case gcvINDEX_8:
        {
            /* Cast pointer to index buffer. */
            gctUINT8_PTR ptr = (gctUINT8_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT8) ((i % 2 == 0) ? (first + i) : (first + i + 1));
                *ptr++ = (gctUINT8) ((i % 2 == 0) ? (first + i + 1) : (first + i));
                *ptr++ = (gctUINT8) (first + i + 2);
            }
        }
        break;

    case gcvINDEX_16:
        {
            /* Cast pointer to index buffer. */
            gctUINT16_PTR ptr = (gctUINT16_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT16) ((i % 2 == 0) ? (first + i) : (first + i + 1));
                *ptr++ = (gctUINT16) ((i % 2 == 0) ? (first + i + 1) : (first + i));
                *ptr++ = (gctUINT16) (first + i + 2);
            }
        }
        break;

    case gcvINDEX_32:
        {
            /* Cast pointer to index buffer. */
            gctUINT32_PTR ptr = (gctUINT32_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = ((i % 2 == 0) ?  (gctUINT32)(first + i) :  (gctUINT32)(first + i + 1));
                *ptr++ = ((i % 2 == 0) ?  (gctUINT32)(first + i + 1) :  (gctUINT32)(first + i));
                *ptr++ =  (gctUINT32)(first + i + 2);
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    chipCtx->indexLoops = 1;
    instantDraw->first = 0;
    instantDraw->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->indexType = indexType;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchTriangleFanIndexed(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gctBOOL PrimitiveRestart
    )
{
    gctSIZE_T newIndexCount = 0;
    gctSIZE_T oldIndexCount = instantDraw->count;
    gctSIZE_T indexSize     = 0;
    gctSIZE_T requiredSize  = 0;
    GLvoid * indexMemory = instantDraw->indexMemory;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ oldIndexBuffer = instantDraw->indexBuffer;
    gctBOOL indexLocked = gcvFALSE;
    gctSIZE_T i;
    /* Set 0 as pivot */
    gctSIZE_T pivot = 0;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    requiredSize  = instantDraw->primCount * 3 * indexSize;
    /* Get index buffer */
    if (oldIndexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(oldIndexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (instantDraw->indexType)
    {
    case gcvINDEX_8:
        {
            gctUINT8_PTR src = (gctUINT8_PTR)indexMemory;
            gctUINT8_PTR ptr = (gctUINT8_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 1; i < (oldIndexCount - 1); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[pivot] != 0xFF)
                        && (src[(i)] != 0xFF)
                        && (src[(i+1)] != 0xFF)))
                {
                    *ptr++ = src[pivot];
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 3;
                }
                else
                {
                    pivot =  i;
                }
            }
        }
        break;

    case gcvINDEX_16:
        {
            gctUINT16_PTR src = (gctUINT16_PTR)indexMemory;
            gctUINT16_PTR ptr = (gctUINT16_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 1; i < (oldIndexCount - 1); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[pivot] != 0xFFFF)
                        && (src[(i)] != 0xFFFF)
                        && (src[(i+1)] != 0xFFFF)))
                {
                    *ptr++ = src[pivot];
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 3;
                }
                else
                {
                    pivot = i;
                }
            }
        }
        break;

    case gcvINDEX_32:
        {
            gctUINT32_PTR src = (gctUINT32_PTR)indexMemory;
            gctUINT32_PTR ptr = (gctUINT32_PTR)tempIndices;

            /* Fill index buffer */
            for (i = 1; i < (oldIndexCount - 1); ++i)
            {
                if ((!PrimitiveRestart)
                    || ((src[pivot] != 0xFFFFFFFF)
                        && (src[(i)] != 0xFFFFFFFF)
                        && (src[(i+1)] != 0xFFFFFFFF)))
                {
                    *ptr++ = src[pivot];
                    *ptr++ = src[i];
                    *ptr++ = src[i+1];
                    newIndexCount += 3;
                }
                else
                {
                    pivot = i;
                }
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    instantDraw->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->primCount = newIndexCount / 3;

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(oldIndexBuffer));
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchTriangleFan(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    gctSIZE_T newPrimitiveCount;
    gctSIZE_T newIndexCount;
    gctSIZE_T first;
    gctSIZE_T vertexCount;
    gceINDEX_TYPE indexType;
    gctSIZE_T requiredSize;
    GLvoid * tempIndices = gcvNULL;
    gctSIZE_T i;
    gceSTATUS status = gcvSTATUS_OK;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Get values */
    newPrimitiveCount = instantDraw->primCount;
    newIndexCount = newPrimitiveCount * 3;
    first = instantDraw->first;
    vertexCount = (gctSIZE_T)(gc->vertexArray.end - gc->vertexArray.start);

    /* Check if the count fits in 8-bit. */
    if (first + vertexCount + 1 < 256)
    {
        /* 8-bit indices. */
        indexType     = gcvINDEX_8;
        requiredSize  = newIndexCount;
    }

    /* Check if the count fits in 16-bit. */
    else if (first + vertexCount + 1 < 65536)
    {
        /* 16-bit indices. */
        indexType     = gcvINDEX_16;
        requiredSize  = newIndexCount * 2;
    }

    else
    {
        /* 32-bit indices. */
        indexType     = gcvINDEX_32;
        requiredSize  = newIndexCount * 4;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (indexType)
    {
    case gcvINDEX_8:
        {
            /* Cast pointer to index buffer. */
            gctUINT8_PTR ptr = (gctUINT8_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT8) (first);
                *ptr++ = (gctUINT8) (first + i + 1);
                *ptr++ = (gctUINT8) (first + i + 2);
            }
        }
        break;

    case gcvINDEX_16:
        {
            /* Cast pointer to index buffer. */
            gctUINT16_PTR ptr = (gctUINT16_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT16) (first);
                *ptr++ = (gctUINT16) (first + i + 1);
                *ptr++ = (gctUINT16) (first + i + 2);
            }
        }
        break;

    case gcvINDEX_32:
        {
            /* Cast pointer to index buffer. */
            gctUINT32_PTR ptr = (gctUINT32_PTR) tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < newPrimitiveCount; ++i)
            {
                *ptr++ = (gctUINT32)first;
                *ptr++ = (gctUINT32)(first + i + 1);
                *ptr++ = (gctUINT32)(first + i + 2);
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    chipCtx->indexLoops = 1;
    instantDraw->first = 0;
    instantDraw->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
    instantDraw->indexMemory = tempIndices;
    instantDraw->count = newIndexCount;
    instantDraw->indexType = indexType;
    instantDraw->indexBuffer = gcvNULL;

OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchIndexedPR(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gctSIZE_T CountPerPrimitive
    )
{
    gctSIZE_T newIndexCount = 0;
    gctSIZE_T oldIndexCount = instantDraw->count;
    gctSIZE_T indexSize     = 0;
    gctSIZE_T requiredSize  = 0;
    GLvoid * indexMemory = instantDraw->indexMemory;
    GLvoid * tempIndices = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ oldIndexBuffer = instantDraw->indexBuffer;
    gctBOOL indexLocked = gcvFALSE;
    gctSIZE_T i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    requiredSize  = oldIndexCount * indexSize;
    /* Get index buffer */
    if (oldIndexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(oldIndexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    /* Allocate a temporary buffer. */
    tempIndices = gcChipPatchClaimIndexMemory(gc, chipCtx, requiredSize);
    if (!tempIndices)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Dispatch on index type. */
    switch (instantDraw->indexType)
    {
    case gcvINDEX_8:
        {
            gctUINT8_PTR src = (gctUINT8_PTR)indexMemory;
            gctUINT8_PTR ptr = (gctUINT8_PTR)tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < oldIndexCount; ++i)
            {
                if (src[i] != 0xFF)
                {
                    *ptr++ = src[i];
                    newIndexCount++;
                }
                else
                {
                    gctSIZE_T extra = newIndexCount % CountPerPrimitive;
                    if (extra)
                    {
                        ptr -= extra;
                        newIndexCount -= extra;
                    }
                }
            }
        }
        break;

    case gcvINDEX_16:
        {
            gctUINT16_PTR src = (gctUINT16_PTR)indexMemory;
            gctUINT16_PTR ptr = (gctUINT16_PTR)tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < oldIndexCount; ++i)
            {
                if (src[i] != 0xFFFF)
                {
                    *ptr++ = src[i];
                    newIndexCount++;
                }
                else
                {
                    gctSIZE_T extra = newIndexCount % CountPerPrimitive;
                    if (extra)
                    {
                        ptr -= extra;
                        newIndexCount -= extra;
                    }
                }
            }
        }
        break;

    case gcvINDEX_32:
        {
            gctUINT32_PTR src = (gctUINT32_PTR)indexMemory;
            gctUINT32_PTR ptr = (gctUINT32_PTR)tempIndices;

            /* Fill index buffer with First through First + Count - 1. */
            for (i = 0; i < oldIndexCount; i++)
            {
                if (src[i] != 0xFFFFFFFF)
                {
                    *ptr++ = src[i];
                    newIndexCount++;
                }
                else
                {
                    gctSIZE_T extra = newIndexCount % CountPerPrimitive;
                    if (extra)
                    {
                        ptr -= extra;
                        newIndexCount -= extra;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    /* Set output */
    instantDraw->indexMemory = tempIndices;
    instantDraw->indexBuffer = gcvNULL;
    instantDraw->count = newIndexCount;
    instantDraw->primCount = newIndexCount / CountPerPrimitive;

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(oldIndexBuffer));
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchShiftIndex(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    __GLbufferObject *idxBufObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
    __GLchipVertexBufferInfo *bufInfo = NULL;
    GLubyte * pSrcBase = gcvNULL;
    GLubyte * pDstBase = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* There must be index buffer bound for this draw */
    GL_ASSERT(idxBufObj);
    bufInfo = (__GLchipVertexBufferInfo*)idxBufObj->privateData;

    /* Reset patched shift indices if dirtied. */
    if (bufInfo->patchDirty)
    {
        if (bufInfo->shiftObj)
        {
            gcmONERROR(gcoBUFOBJ_Destroy(bufInfo->shiftObj));
            bufInfo->shiftObj = gcvNULL;
        }

        bufInfo->patchDirty = gcvFALSE;
        bufInfo->shiftBias = 0;
    }

    if (instantDraw->first < 0)
    {
        gctSIZE_T indexOffset = __GL_PTR2SIZE(instantDraw->indexMemory);
        gctSIZE_T unAlignedStep = 0;
        gctSIZE_T indexSize     = 0;

        gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);

        unAlignedStep = indexOffset % indexSize;

        /* Create shadow shift bufObj if needed */
        if (!bufInfo->shiftObj)
        {
            gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_ELEMENT_ARRAY_BUFFER, &bufInfo->shiftObj));
            gcmONERROR(gcoBUFOBJ_Upload(bufInfo->shiftObj, gcvNULL, 0, bufInfo->size, gcvBUFOBJ_USAGE_STATIC_DRAW));
        }

        /* Change shadow bufObj if its content cannot match current use */
        if (instantDraw->first < bufInfo->shiftBias ||
            bufInfo->unAlignedStep != unAlignedStep ||
            bufInfo->indexType != instantDraw->indexType)
        {
            gctSIZE_T i;
            gctSIZE_T count = (bufInfo->size - unAlignedStep) / indexSize;

            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&pSrcBase));
            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->shiftObj, gcvNULL, (gctPOINTER*)&pDstBase));
            pSrcBase += unAlignedStep;
            pDstBase += unAlignedStep;

            switch (instantDraw->indexType)
            {
            case gcvINDEX_8:
                for (i = 0; i < count; ++i)
                {
                    pDstBase[i] = (GLubyte)(pSrcBase[i] + instantDraw->first);
                }
                break;
            case gcvINDEX_16:
                for (i = 0; i < count; ++i)
                {
                    ((GLushort*)pDstBase)[i] = (GLushort)(((GLushort*)pSrcBase)[i] + instantDraw->first);
                }
                break;
            case gcvINDEX_32:
                for (i = 0; i < count; ++i)
                {
                    ((GLuint*)pDstBase)[i] = (GLuint)(((GLuint*)pSrcBase)[i] + instantDraw->first);
                }
                break;

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            bufInfo->shiftBias = instantDraw->first;
            bufInfo->unAlignedStep = unAlignedStep;
            bufInfo->indexType = instantDraw->indexType;
        }

        instantDraw->first -= bufInfo->shiftBias;
        instantDraw->indexBuffer = bufInfo->shiftObj;
    }

OnError:
    if (pSrcBase)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(bufInfo->bufObj));
    }

    if (pDstBase)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(bufInfo->shiftObj));
    }

    gcmFOOTER();
    return status;
}


static gceSTATUS
gcChipPatch32BitIndices(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL primitiveRestart = gc->state.enables.primitiveRestart;
    GLvoid * indexMemory = instantDraw->indexMemory;
    gctBOOL indexLocked  = gcvFALSE;
    gctUINT32_PTR src = gcvNULL;
    gctUINT32 min = ~0U;
    gctUINT32 max = 0;
    gctSIZE_T i;

    gctUINT32 maxIndex;

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Query caps */
    maxIndex = (gctUINT32) gc->constants.maxElementIndex;

    /* Get index buffer */
    if (instantDraw->indexBuffer)
    {
        gctPOINTER indexBase = gcvNULL;
        gcmONERROR(gcoBUFOBJ_Lock(instantDraw->indexBuffer, gcvNULL, &indexBase));
        indexMemory = (gctUINT8_PTR)indexBase + gcmPTR2SIZE(indexMemory);
        indexLocked = gcvTRUE;
    }

    if (instantDraw->indexBuffer != NULL)
    {
        gcmONERROR(gcoBUFOBJ_IndexGetRange(instantDraw->indexBuffer,
                                           gcvINDEX_32,
                                           gcmPTR2INT32(instantDraw->indexMemory),
                                           (gctUINT32)instantDraw->count,
                                           &min,
                                           &max));
    }
    else
    {
        /* Get source indices */
        src = (gctUINT32_PTR)indexMemory;

        for (i = 0; i < instantDraw->count; ++i)
        {
            if (primitiveRestart && (src[i] == 0xFFFFFFFF))
            {
                continue;
            }

            if (src[i] < min) min = src[i];
            if (src[i] > max) max = src[i];
        }
    }

    /* Can the indices be */
    if ((max > maxIndex) || (min > maxIndex))
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

OnError:
    if (indexLocked)
    {
        /* Unlock index buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(instantDraw->indexBuffer));
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipPatchConvertVertAttrib(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipInstantDraw *instantDraw,
    gcsATTRIBUTE_PTR AttrPtr
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLbufferObject *bufObj = gcvNULL;
    __GLchipVertexBufferInfo *bufInfo = gcvNULL;
    gctUINT8_PTR baseSrc = gcvNULL;
    gctUINT8_PTR baseDst = gcvNULL;
    gcoBUFOBJ stream = gcvNULL;
    gctSIZE_T count = 0;
    gctSIZE_T first = 0;
    gctSIZE_T i;
    gctSIZE_T requiredSize;
    gcoBUFOBJ newStream = gcvNULL;
    GLuint minIndex = 0;
    GLuint maxIndex = 0;
    gctBOOL srcLocked = gcvFALSE;
    gctBOOL dstLocked = gcvFALSE;

    /* IMPORTANT: We can use the same method as triangle strip indexed patch uses to store
     * patched buffer objects aside. This will help us re-use the patched buffer and not
     * create it over and over again.
     */

    /* Header */
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x instantDraw=0x%x AttrPtr=0x%x", gc, chipCtx, instantDraw, AttrPtr);

    /* Check if this attribute is an array */
    if (AttrPtr->enable)
    {
        GLuint binding = gc->vertexArray.boundVAO->vertexArray.attribute[AttrPtr->arrayIdx].attribBinding;
        const GLint defaultAtrrib[4] =
        {
            0x00000000,
            0x00000000,
            0x00000000,
            0x00000001
        };

        /* Get stream is this is a server side buffer */
        bufObj = __glGetCurrentVertexArrayBufObj(gc, binding);
        if (bufObj && bufObj->size > 0)
        {
            bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);
        }

        /* This is a server side buffer */
        if (bufInfo != gcvNULL)
        {
            stream = bufInfo->bufObj;
            gcmONERROR(gcoBUFOBJ_Lock(stream, gcvNULL, (gctPOINTER*)&baseSrc));
            baseSrc += gcmPTR2SIZE(AttrPtr->pointer);
            srcLocked = GL_TRUE;
        }
        /* This is a client side array */
        else
        {
            baseSrc = (gctUINT8_PTR)AttrPtr->pointer;
        }

        if (gc->vertexArray.indexCount != 0)
        {
            if (AttrPtr->divisor == 0)
            {
                GLuint index = 0;
                GLubyte *pSrcIndices = gcvNULL;

                /* Get max and min Index */
                if (instantDraw->indexBuffer)
                {
                    gctUINT8_PTR indexAddress = gcvNULL;
                    /* This is a server side buffer */
                    gcmONERROR(gcoBUFOBJ_Lock(instantDraw->indexBuffer, gcvNULL, (gctPOINTER*)&indexAddress));
                    pSrcIndices = (GLubyte*)indexAddress + gcmPTR2SIZE(instantDraw->indexMemory);
                }
                else
                {
                    /* This is a client side array */
                    pSrcIndices = (GLubyte*)instantDraw->indexMemory;
                }

                minIndex = (GLuint)-1;
                /* Process all elements to get vertex count */
                for (i = 0; i < instantDraw->count; ++i)
                {
                    /* Get the element. */
                    switch (instantDraw->indexType)
                    {
                    case gcvINDEX_8:
                        index = (GLuint)(pSrcIndices[i]) + instantDraw->first;
                        break;
                    case gcvINDEX_16:
                        index = (GLuint)(((GLushort*)pSrcIndices)[i]) + instantDraw->first;
                        break;
                    case gcvINDEX_32:
                        index = ((GLuint*)pSrcIndices)[i] + instantDraw->first;
                        break;
                    default:
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                        break;
                    }

                    /* Keep track of min/max element. */
                    if (index < minIndex) minIndex = index;
                    if (index > maxIndex) maxIndex = index;
                }

                if (instantDraw->indexBuffer)
                {
                    gcmONERROR(gcoBUFOBJ_Unlock(instantDraw->indexBuffer));
                }

                count = (gctSIZE_T)(maxIndex - minIndex + 1);
                baseSrc += (AttrPtr->stride * minIndex);
            }
            else
            {
                count = (gctSIZE_T)gcoMATH_Ceiling((gctFLOAT)gc->vertexArray.instanceCount / (gctFLOAT)AttrPtr->divisor);
                minIndex = 0;
            }
        }
        else
        {
            if (AttrPtr->divisor == 0)
            {
                count = (gctSIZE_T)instantDraw->count;
                first = (gctSIZE_T)instantDraw->first;
                baseSrc += (AttrPtr->stride * instantDraw->first);
            }
            else
            {
                count = (gctSIZE_T)gcoMATH_Ceiling((gctFLOAT)gc->vertexArray.instanceCount / (gctFLOAT)AttrPtr->divisor);
            }
            minIndex = 0;
        }

        /* Claim buffer */
        requiredSize = (first + count + minIndex) * sizeof(GLfloat) * 4; /* Always expand to 4-component stream here */
        gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_ARRAY_BUFFER, &newStream));
        gcmONERROR(gcoBUFOBJ_Upload(newStream, gcvNULL, 0, requiredSize, gcvBUFOBJ_USAGE_STATIC_DRAW));
        gcmONERROR(gcoBUFOBJ_Lock(newStream, gcvNULL, (gctPOINTER*)&baseDst));
        dstLocked = GL_TRUE;

        switch (AttrPtr->format)
        {
        case gcvVERTEX_INT_2_10_10_10_REV:
            {
                /* Cast pointer to buffer. */
                gctFLOAT_PTR pDst = (gctFLOAT_PTR)baseDst + AttrPtr->size * (minIndex + first);

                GL_ASSERT(AttrPtr->size == 4);
                GL_ASSERT(AttrPtr->convertScheme == gcvATTRIB_SCHEME_2_10_10_10_REV_TO_FLOAT);

                for (i = 0; i < count; ++i)
                {
                    gctINT16 int_x, int_y, int_z, int_w;
                    gctFLOAT float_x, float_y, float_z, float_w;

                    /* Get packed data */
                    gctUINT32 value = *((gctUINT32_PTR)(baseSrc + (AttrPtr->stride * i)));

                    /* Convert to short */
                    int_x = (gctINT16) (value & 0x3FF);
                    int_y = (gctINT16) ((value >> 10) & 0x3FF);
                    int_z = (gctINT16) ((value >> 20) & 0x3FF);
                    int_w = (gctINT16) ((value >> 30) & 0x3);

                    /* Handle sign */
                    int_x = (int_x & 0x200) ? -1 * ((int_x ^ 0x3FF) + 1) : int_x;
                    int_y = (int_y & 0x200) ? -1 * ((int_y ^ 0x3FF) + 1) : int_y;
                    int_z = (int_z & 0x200) ? -1 * ((int_z ^ 0x3FF) + 1) : int_z;
                    int_w = (int_w & 0x2)   ? -1 * ((int_w ^ 0x3)   + 1) : int_w;

                    /* Handle normalization */
                    if (AttrPtr->normalized)
                    {
                        float_x = gcmMAX(((gctFLOAT) int_x) / 511.0f, -1);
                        float_y = gcmMAX(((gctFLOAT) int_y) / 511.0f, -1);
                        float_z = gcmMAX(((gctFLOAT) int_z) / 511.0f, -1);
                        float_w = gcmMAX(((gctFLOAT) int_w) / 1.0f, -1);
                    }
                    else
                    {
                        float_x = (gctFLOAT) int_x;
                        float_y = (gctFLOAT) int_y;
                        float_z = (gctFLOAT) int_z;
                        float_w = (gctFLOAT) int_w;
                    }

                    /* Set data */
                    *pDst++ = float_x;
                    *pDst++ = float_y;
                    *pDst++ = float_z;
                    *pDst++ = float_w;
                }

                AttrPtr->format = gcvVERTEX_FLOAT;
                AttrPtr->stride = AttrPtr->size * sizeof(GLfloat);
            }
            break;

        case gcvVERTEX_UNSIGNED_INT_2_10_10_10_REV:
            {
                /* Cast pointer to buffer. */
                gctFLOAT_PTR pDst = (gctFLOAT_PTR)baseDst + AttrPtr->size * (minIndex + first);

                GL_ASSERT(AttrPtr->size == 4);
                GL_ASSERT(AttrPtr->convertScheme == gcvATTRIB_SCHEME_2_10_10_10_REV_TO_FLOAT);

                for (i = 0; i < count; ++i)
                {
                    gctUINT16 int_x, int_y, int_z, int_w;
                    gctFLOAT float_x, float_y, float_z, float_w;

                    /* Get packed data */
                    gctUINT32 value = *((gctUINT32_PTR)(baseSrc + (AttrPtr->stride * i)));

                    /* Convert to short */
                    int_x = (gctUINT16) (value & 0x3FF);
                    int_y = (gctUINT16) ((value >> 10) & 0x3FF);
                    int_z = (gctUINT16) ((value >> 20) & 0x3FF);
                    int_w = (gctUINT16) ((value >> 30) & 0x3);

                    /* Handle normalization */
                    if (AttrPtr->normalized)
                    {
                        /*
                        Get float value that will be correct when normalized
                        Verify normalization rules (normalized is TRUE) for
                        UNSIGNED_INT_2_10_10_10_REV (f2=c/3, f10=c/1023).
                        */
                        float_x = gcmMAX(((gctFLOAT) int_x) / 1023.0f, -1);
                        float_y = gcmMAX(((gctFLOAT) int_y) / 1023.0f, -1);
                        float_z = gcmMAX(((gctFLOAT) int_z) / 1023.0f, -1);
                        float_w = gcmMAX(((gctFLOAT) int_w) / 3.0f, -1);
                    }
                    else
                    {
                        float_x = (gctFLOAT) int_x;
                        float_y = (gctFLOAT) int_y;
                        float_z = (gctFLOAT) int_z;
                        float_w = (gctFLOAT) int_w;
                    }

                    /* Set data */
                    *pDst++ = float_x;
                    *pDst++ = float_y;
                    *pDst++ = float_z;
                    *pDst++ = float_w;
                }

                AttrPtr->format = gcvVERTEX_FLOAT;
                AttrPtr->stride = AttrPtr->size * sizeof(GLfloat);
            }
            break;

        case gcvVERTEX_INT8:
            switch (AttrPtr->convertScheme)
            {
            case gcvATTRIB_SCHEME_BYTE_TO_IVEC4:
                {
                    gctINT_PTR pDst = (gctINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        /* Get packed data */
                        gctINT j;
                        gctINT8_PTR pSrc = (gctINT8_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;

            case gcvATTRIB_SCHEME_UBYTE_TO_UVEC4:
                {
                    gctUINT_PTR pDst = (gctUINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        /* Get packed data */
                        gctINT j;
                        gctUINT8_PTR pSrc = (gctUINT8_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctUINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctUINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;
            default:
                GL_ASSERT(0);
            }

            AttrPtr->format = gcvVERTEX_INT32;
            AttrPtr->size = 4;
            AttrPtr->stride = AttrPtr->size * sizeof(GLint);
            break;

        case gcvVERTEX_INT16:
            switch (AttrPtr->convertScheme)
            {
            case gcvATTRIB_SCHEME_SHORT_TO_IVEC4:
                {
                    gctINT_PTR pDst = (gctINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        /* Get packed data */
                        gctINT j;
                        gctINT16_PTR pSrc = (gctINT16_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;

            case gcvATTRIB_SCHEME_USHORT_TO_UVEC4:
                {
                    gctUINT_PTR pDst = (gctUINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        /* Get packed data */
                        gctINT j;
                        gctUINT16_PTR pSrc = (gctUINT16_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctUINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctUINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;
            default:
                GL_ASSERT(0);
            }

            AttrPtr->format = gcvVERTEX_INT32;
            AttrPtr->size = 4;
            AttrPtr->stride = AttrPtr->size * sizeof(GLint);
            break;

        case gcvVERTEX_INT32:
            switch (AttrPtr->convertScheme)
            {
            case gcvATTRIB_SCHEME_INT_TO_IVEC4:
                {
                    gctINT_PTR pDst = (gctINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        gctINT j;
                        /* Get packed data */
                        gctINT_PTR pSrc = (gctINT_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;

            case gcvATTRIB_SCHEME_UINT_TO_UVEC4:
                {
                    gctUINT_PTR pDst = (gctUINT_PTR)baseDst + 4 * (minIndex + first);

                    for (i = 0; i < count; ++i)
                    {
                        gctINT j;
                        /* Get packed data */
                        gctUINT_PTR pSrc = (gctUINT_PTR)(baseSrc + (AttrPtr->stride * i));

                        for (j = 0; j < AttrPtr->size; ++j)
                        {
                            *pDst++ = (gctUINT)(*pSrc++);
                        }
                        for (j = AttrPtr->size; j < 4; j++)
                        {
                            *pDst++ = (gctUINT)(defaultAtrrib[j]);
                        }
                    }
                }
                break;

            default:
                GL_ASSERT(0);
            }

            AttrPtr->format = gcvVERTEX_INT32;
            AttrPtr->size = 4;
            AttrPtr->stride = AttrPtr->size * sizeof(GLint);
            break;

        default:
            gcmASSERT(GL_FALSE);
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        /* Set output */
        AttrPtr->stream = newStream;
        AttrPtr->tempStream = newStream;
        AttrPtr->pointer = 0;
    }

OnError:
    if (srcLocked)
    {
        /* Unlock buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(stream));
    }

    if (dstLocked)
    {
        /* Unlock buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(newStream));
        /* Flush CPU cache */
        gcmONERROR(gcoBUFOBJ_CPUCacheOperation(newStream, gcvCACHE_CLEAN));
    }

    if (gcmIS_ERROR(status) && newStream)
    {
        /* Destroy buffer */
        gcmVERIFY_OK(gcoBUFOBJ_Destroy(newStream));
    }

    gcmFOOTER();
    return status;
}

#if defined(ANDROID) && __GL_CHIP_ENABLE_MEMORY_REDUCTION
static gceSTATUS
gcChipReduceImageTexture(
    __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLuint    unit;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (unit = 0; unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits; unit ++)
    {
        if (__glBitmaskTest(&gc->texture.currentEnableMask, unit))
        {
            GLuint enableDim = gc->state.texture.texUnits[unit].realEnableDim;
            __GLtextureObject *texObj = gc->texture.units[unit].boundTextures[enableDim];
            __GLchipTextureInfo *texInfo;
            khrEGL_IMAGE *image;

            if (texObj == gcvNULL || texObj->privateData == gcvNULL)
            {
                continue;
            }

            texInfo = (__GLchipTextureInfo*)texObj->privateData;
            image   = (khrEGL_IMAGE *)texInfo->eglImage.image;

            /* This can only play with external texture */
            if (image != gcvNULL &&
                texInfo->object != gcvNULL &&
                texInfo->eglImage.directSample == gcvFALSE &&
                enableDim == __GL_TEXTURE_EXTERNAL_INDEX &&
                image->type == KHR_IMAGE_ANDROID_NATIVE_BUFFER)
            {
                gcoSURF mipmap = gcvNULL;

                gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, 0, &mipmap));

                if (mipmap != gcvNULL)
                {
                    GLuint index;
                    __GLtexUnit2Sampler *texUnit2Sampler = &gc->shaderProgram.texUnit2Sampler[unit];
                    /* For external, and not directly mapped image source,
                    ** we delete the texture surface, next time we need use the texture surface,
                    ** (at draw time or glEGLimageTargetTexture2D),we recreate it
                    */
                    gcmONERROR(gcoTEXTURE_Destroy(texInfo->object));
                    texInfo->object = gcvNULL;

                    /* Mark states of the samplers bound to this unit dirty */
                    for (index = 0; index < texUnit2Sampler->numSamplers; ++index)
                    {
                        __glBitmaskSet(&gc->shaderProgram.samplerStateKeepDirty, texUnit2Sampler->samplers[index]);
                    }
                };
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}
#endif

__GL_INLINE gceSTATUS
gcChipLockOutDrawIndirectBuf(
    __GLcontext *gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;

    if (gc->vertexArray.indexCount == 0)
    {

        __GLdrawArraysIndirectCommand *drawArraysCmd = (__GLdrawArraysIndirectCommand *)
                                                       __glChipMapBufferRange(gc,
                                                                              indirectObj,
                                                                              __GL_DRAW_INDIRECT_BUFFER_INDEX,
                                                                              (GLintptr)gc->vertexArray.indirectOffset,
                                                                              sizeof(__GLdrawArraysIndirectCommand),
                                                                              GL_MAP_READ_BIT);

        gc->vertexArray.start = drawArraysCmd->first;
        gc->vertexArray.end   = drawArraysCmd->first + drawArraysCmd->count;
        gc->vertexArray.instanceCount = drawArraysCmd->instanceCount;
        gc->vertexArray.baseVertex = drawArraysCmd->first;

        __glChipUnMapBufferObject(gc, indirectObj, __GL_DRAW_INDIRECT_BUFFER_INDEX);
    }
    else
    {
        __GLdrawElementsIndirectCommand *drawElementsCmd = (__GLdrawElementsIndirectCommand *)
                                                            __glChipMapBufferRange(gc,
                                                                                   indirectObj,
                                                                                   __GL_DRAW_INDIRECT_BUFFER_INDEX,
                                                                                   (GLintptr)gc->vertexArray.indirectOffset,
                                                                                   sizeof(__GLdrawElementsIndirectCommand),
                                                                                   GL_MAP_READ_BIT);
        gc->vertexArray.indexCount = drawElementsCmd->count;
        gc->vertexArray.instanceCount = drawElementsCmd->instanceCount;
        gc->vertexArray.baseVertex = drawElementsCmd->baseVertex;
        switch (gc->vertexArray.indexType)
        {
        case GL_UNSIGNED_BYTE:
            gc->vertexArray.indices = __GL_SIZE2PTR(drawElementsCmd->firstIndex * sizeof(GLubyte));
            break;
        case GL_UNSIGNED_SHORT:
            gc->vertexArray.indices = __GL_SIZE2PTR(drawElementsCmd->firstIndex * sizeof(GLushort));
            break;
        case GL_UNSIGNED_INT:
            gc->vertexArray.indices = __GL_SIZE2PTR(drawElementsCmd->firstIndex * sizeof(GLuint));
            break;
        default:
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
        }

        __glChipUnMapBufferObject(gc, indirectObj, __GL_DRAW_INDIRECT_BUFFER_INDEX);

    }


OnError:
    return status;
}

static GLvoid
gcChipGetDrawPath(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    gctBOOL forceInstancedDraw
    )
{
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x forceInstancedDraw=0x%x",
                   gc, chipCtx, forceInstancedDraw);

    if ((chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_1) &&
        (forceInstancedDraw))
    {
        if (chipCtx->indexLoops)
        {
            gc->dp.drawPrimitive = __glChipDrawElements;
        }
        else
        {
            gc->dp.drawPrimitive = __glChipDrawArrays;
        }
        /* We should not hit this case */
        GL_ASSERT(GL_FALSE);
    }
    else if (chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0)
    {
        if (chipCtx->indexLoops)
        {
            gc->dp.drawPrimitive = __glChipDrawElementsInstanced;
        }
        else
        {
            gc->dp.drawPrimitive = __glChipDrawArraysInstanced;
        }
    }
    else
    {
        if (chipCtx->indexLoops)
        {
            gc->dp.drawPrimitive = __glChipDrawElements;
        }
        else
        {
            gc->dp.drawPrimitive = __glChipDrawArrays;
        }
    }

    gcmFOOTER_NO();
    return;
}

__GL_INLINE gceSTATUS
gcChipValidateDrawPath(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLbufferObject *idxBufObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
    __GLchipInstantDraw *defaultInstant = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Initialize all instant draw attributes */
    __GL_MEMZERO(chipCtx->instantDraw, __GL_MAX_DRAW_LOOPS * sizeof(__GLchipInstantDraw));

#if __GL_CHIP_PATCH_ENABLED
    /* Route the draw through the patch logic. Old code just did this for indexed draw.
     * This behavior is repeated here.
     */
    if (gcChipPatchDraw(gc,
                        gc->vertexArray.primMode,
                        (gctSIZE_T)gc->vertexArray.indexCount,
                        gc->vertexArray.indexType,
                        gc->vertexArray.indices))
    {
        /* Nothing to draw now. */
        gc->dp.drawPrimitive = __glChipDrawNothing;
    }
    else
#endif
    {
        do
        {
            __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;
            GLboolean xfbEnabled = ((xfbObj->active) && (!xfbObj->paused)) ? GL_TRUE : GL_FALSE;
            /* Detect if we need to force instanced draw */
            gctBOOL forceInstancedDraw;

            if (gc->vertexArray.drawIndirect)
            {
                if (chipCtx->anyAttibGeneric
                ||  !chipCtx->chipFeature.hwFeature.hasDrawIndirect
                ||  (gc->vertexArray.primMode == GL_PATCHES_EXT
                    && !chipCtx->chipFeature.hwFeature.hasPatchListFetchFix
                    && chipCtx->patchId != gcvPATCH_CAR_CHASE)
                )
                {
                    gcmONERROR(gcChipLockOutDrawIndirectBuf(gc));
                }

                if (!chipCtx->chipFeature.hwFeature.hasDrawIndirect
                ||  (gc->vertexArray.primMode == GL_PATCHES_EXT
                    && !chipCtx->chipFeature.hwFeature.hasPatchListFetchFix
                    && chipCtx->patchId != gcvPATCH_CAR_CHASE)
                )
                {
                    gc->vertexArray.drawIndirect = gcvFALSE;
                }
            }

            forceInstancedDraw = ((gc->vertexArray.instanceCount > 1) || (chipCtx->anyAttibInstanced));

            /* Set first */
            defaultInstant->first = gc->vertexArray.baseVertex;
            defaultInstant->attribMask = chipCtx->attribMask;
            defaultInstant->attributes = chipCtx->attributeArray;
            defaultInstant->positionIndex = chipCtx->positionIndex;
            defaultInstant->primitiveRestart = gc->state.enables.primitiveRestart;

            /* Is it an indexed draw? */
            if (gc->vertexArray.indexCount == 0)
            {
                /* No, this is not an indexed draw */
                chipCtx->indexLoops = 0;

                /* Vertex count */
                defaultInstant->count = (gctSIZE_T)(gc->vertexArray.end - gc->vertexArray.start);
            }
            else
            {
                /* Yes, this is an indexed draw */
                chipCtx->indexLoops = 1;

                /* Index count */
                defaultInstant->count = (gctSIZE_T)gc->vertexArray.indexCount;

                /* Index type */
                switch (gc->vertexArray.indexType)
                {
                case GL_UNSIGNED_BYTE:
                    defaultInstant->indexType = gcvINDEX_8;
                    break;
                case GL_UNSIGNED_SHORT:
                    defaultInstant->indexType = gcvINDEX_16;
                    break;
                case GL_UNSIGNED_INT:
                    defaultInstant->indexType = gcvINDEX_32;
                    break;
                default:
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                /* Get indices offset or pointer */
                defaultInstant->indexMemory = gc->vertexArray.drawIndirect ? NULL : (gctPOINTER)gc->vertexArray.indices;

                /* Get index buffer */
                if (idxBufObj)
                {
                    __GLchipVertexBufferInfo *idxBufInfo = (__GLchipVertexBufferInfo*)idxBufObj->privateData;
                    defaultInstant->indexBuffer = idxBufInfo->bufObj;

                    if (idxBufInfo->size == 0)
                    {
                         gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    }
                }
            }

            /* Find HAL primitive and primitive count */
            switch (gc->vertexArray.primMode)
            {
            case GL_TRIANGLE_STRIP:
                defaultInstant->primMode = gcvPRIMITIVE_TRIANGLE_STRIP;
                defaultInstant->primCount = defaultInstant->count - 2;
                break;

            case GL_TRIANGLE_FAN:
                defaultInstant->primMode = gcvPRIMITIVE_TRIANGLE_FAN;
                defaultInstant->primCount = defaultInstant->count - 2;
                break;

            case GL_TRIANGLES:
                defaultInstant->primMode = gcvPRIMITIVE_TRIANGLE_LIST;
                defaultInstant->primCount = defaultInstant->count / 3;
                /* HW FE cannot drop redudant vertex for non-indexed mode, driver do it */
                if (xfbEnabled && !chipCtx->indexLoops)
                {
                    defaultInstant->count = defaultInstant->primCount * 3;
                }
                break;

            case GL_POINTS:
                defaultInstant->primMode = gcvPRIMITIVE_POINT_LIST;
                defaultInstant->primCount = defaultInstant->count;
                break;

            case GL_LINES:
                defaultInstant->primMode = gcvPRIMITIVE_LINE_LIST;
                defaultInstant->primCount = defaultInstant->count / 2;
                /* HW FE cannot drop redudant vertex for non-indexed mode, driver do it */
                if (xfbEnabled && !chipCtx->indexLoops)
                {
                    defaultInstant->count = defaultInstant->primCount * 2;
                }
                break;

            case GL_LINE_LOOP:
                defaultInstant->primMode = gcvPRIMITIVE_LINE_LOOP;
                defaultInstant->primCount = (defaultInstant->count >= 2) ? defaultInstant->count : 0;
                break;

            case GL_LINE_STRIP:
                defaultInstant->primMode = gcvPRIMITIVE_LINE_STRIP;
                defaultInstant->primCount = defaultInstant->count - 1;
                break;

            case GL_PATCHES_EXT:
                defaultInstant->primMode = gcvPRIMITIVE_PATCH_LIST;
                defaultInstant->primCount = defaultInstant->count / gc->shaderProgram.patchVertices;
                break;

            case GL_LINES_ADJACENCY_EXT:
                defaultInstant->primMode = gcvPRIMITIVE_LINES_ADJACENCY;
                defaultInstant->primCount = defaultInstant->count / 4;
                break;

            case GL_LINE_STRIP_ADJACENCY_EXT:
                defaultInstant->primMode = gcvPRIMITIVE_LINE_STRIP_ADJACENCY;
                defaultInstant->primCount = defaultInstant->count - 3;
                break;

            case GL_TRIANGLES_ADJACENCY_EXT:
                defaultInstant->primMode = gcvPRIMITIVE_TRIANGLES_ADJACENCY;
                defaultInstant->primCount = defaultInstant->count / 6;
                break;

            case GL_TRIANGLE_STRIP_ADJACENCY_EXT:
                defaultInstant->primMode = gcvPRIMITIVE_TRIANGLE_STRIP_ADJACENCY;
                defaultInstant->primCount = (defaultInstant->count / 2)  - 2;
                break;

            default:
                gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            /* Skip the incomplete primitive draw */
            if (defaultInstant->primCount <= 0)
            {
                gc->dp.drawPrimitive = __glChipDrawNothing;
                break;
            }

            if (gc->profiler.enable)
            {
                __glChipProfilerSet(gc, GL3_PROFILER_PRIMITIVE_TYPE, (gctHANDLE)(gctUINTPTR_T)defaultInstant->primMode);
                __glChipProfilerSet(gc, GL3_PROFILER_PRIMITIVE_COUNT, (gctHANDLE)(gctUINTPTR_T)(defaultInstant->primCount * gc->vertexArray.instanceCount));
            }

            /* Is any of the attrib need SW converted? */
            if (chipCtx->anyAttibConverted)
            {
                gctUINT i;
                gcsATTRIBUTE_PTR attribPtr = defaultInstant->attributes;
                GLuint enableBits = defaultInstant->attribMask;

                /* Walk through all attributes. */
                for (i = 0;
                     (i < gc->constants.shaderCaps.maxUserVertAttributes) && (enableBits != 0);
                     ++i, enableBits >>= 1, ++attribPtr)
                {
                    if ((enableBits & 1) && (attribPtr->convertScheme != gcvATTRIB_SCHEME_KEEP))
                    {
                        gcmONERROR(gcChipPatchConvertVertAttrib(gc, chipCtx, defaultInstant, attribPtr));
                    }
                }
            }

            if (chipCtx->indexLoops && defaultInstant->first < 0 &&
                !chipCtx->chipFeature.hwFeature.hasDrawElementBaseVertex
                )
            {
                GL_ASSERT(defaultInstant->indexBuffer);
                gcmONERROR(gcChipPatchShiftIndex(gc, chipCtx, defaultInstant));
            }

            if (chipCtx->chipFeature.haltiLevel >= __GL_CHIP_HALTI_LEVEL_3)
            {
                /* Get draw command */
                if (gc->vertexArray.drawIndirect)
                {
                    if (gc->vertexArray.indexCount)
                    {
                        gc->dp.drawPrimitive = __glChipDrawElementsIndirect;
                    }
                    else
                    {
                        gc->dp.drawPrimitive = __glChipDrawArraysIndirect;
                    }
                }
                else if (gc->vertexArray.multidrawIndirect)
                {
                    if (gc->vertexArray.indexCount)
                    {
                        gc->dp.drawPrimitive = __glChipMultiDrawElementsIndirect;
                    }
                    else
                    {
                        gc->dp.drawPrimitive = __glChipMultiDrawArraysIndirect;
                    }
                }
                else
                {
                    if (chipCtx->indexLoops)
                    {
                        gc->dp.drawPrimitive = __glChipDrawElementsInstanced;
                    }
                    else
                    {
                        gc->dp.drawPrimitive = __glChipDrawArraysInstanced;
                    }
                }
                break;
            }
            /* Older hardware */
            else
            {
                /*  Old HW cannot support full 32 bit indices. */
                if ((defaultInstant->indexType == gcvINDEX_32) && chipCtx->indexLoops)
                {
                    gcmONERROR(gcChipPatch32BitIndices(gc, chipCtx, defaultInstant));
                }

                /* Remove primitive restart. Convert them to their corresponding
                 * independent primitives.
                 */
                if ((gc->state.enables.primitiveRestart)
                    && (!chipCtx->chipFeature.hwFeature.primitiveRestart)
                    && (chipCtx->indexLoops))
                {
                    switch (defaultInstant->primMode)
                    {
                    case gcvPRIMITIVE_TRIANGLE_LIST:
                        gcmONERROR(gcChipPatchIndexedPR(gc, chipCtx, defaultInstant, 3));
                        break;
                    case gcvPRIMITIVE_LINE_LIST:
                        gcmONERROR(gcChipPatchIndexedPR(gc, chipCtx, defaultInstant, 2));
                        break;
                    case gcvPRIMITIVE_POINT_LIST:
                        gcmONERROR(gcChipPatchIndexedPR(gc, chipCtx, defaultInstant, 1));
                        break;
                    case gcvPRIMITIVE_LINE_STRIP:
                        gcmONERROR(gcChipPatchLineStripIndexed(gc, chipCtx, defaultInstant, gcvTRUE));
                        break;
                    case gcvPRIMITIVE_LINE_LOOP:
                        gcmONERROR(gcChipPatchLineLoopIndexed(gc, chipCtx, defaultInstant, gcvTRUE));
                        break;
                    case gcvPRIMITIVE_TRIANGLE_STRIP:
                        gcmONERROR(gcChipPatchTriangleStripIndexed(gc, chipCtx, defaultInstant, gcvTRUE));
                        break;
                    case gcvPRIMITIVE_TRIANGLE_FAN:
                        gcmONERROR(gcChipPatchTriangleFanIndexed(gc, chipCtx, defaultInstant, gcvTRUE));
                        break;
                    default:
                        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                    }
                }

                /* There are no bugs for the following cases, so do it quickly */
                if ((defaultInstant->primMode == gcvPRIMITIVE_TRIANGLE_LIST)
                    || (defaultInstant->primMode == gcvPRIMITIVE_LINE_LIST)
                    || (defaultInstant->primMode == gcvPRIMITIVE_POINT_LIST))
                {
                    /* Get draw command */
                    gcChipGetDrawPath(gc, chipCtx, forceInstancedDraw);
                    break;
                }

                switch (defaultInstant->primMode)
                {
                case gcvPRIMITIVE_TRIANGLE_STRIP:
                    /* Convert triangle strip to triangle list if HW cannot support */
                    if (chipCtx->indexLoops)
                    {
                        if (forceInstancedDraw || chipCtx->chipFeature.hwFeature.patchTriangleStrip)
                        {
                            if (defaultInstant->indexBuffer)
                            {
                                /* If index are from buffer object, it can be cached. */
                                gcmONERROR(gcChipPatchTriangleStripIndexed_cached(gc, chipCtx, defaultInstant));
                            }
                            else
                            {
                                gcmONERROR(gcChipPatchTriangleStripIndexed(gc, chipCtx, defaultInstant, gcvFALSE));
                            }
                        }
                    }
                    else
                    {
                        if (forceInstancedDraw)
                        {
                            gcmONERROR(gcChipPatchTriangleStrip(gc, chipCtx, defaultInstant));
                        }
                    }
                    break;

                case gcvPRIMITIVE_TRIANGLE_FAN:
                    /* Convert triangle fan to triangle list if HW cannot support */
                    if (forceInstancedDraw)
                    {
                        if (chipCtx->indexLoops)
                        {
                            gcmONERROR(gcChipPatchTriangleFanIndexed(gc, chipCtx, defaultInstant, gcvFALSE));
                        }
                        else
                        {
                            gcmONERROR(gcChipPatchTriangleFan(gc, chipCtx, defaultInstant));
                        }
                    }
                    else if (chipCtx->indexLoops &&
                             chipCtx->chipFeature.hwFeature.patchTriangleStrip &&
                             (chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30))
                    {
                        gcmONERROR(gcChipPatchTriangleFanIndexed(gc, chipCtx, defaultInstant, gcvFALSE));
                    }
                    break;

                case gcvPRIMITIVE_LINE_LOOP:
                    /* Convert line loop to line list if HW cannot support */
                    if (forceInstancedDraw || !chipCtx->chipFeature.hwFeature.lineLoop)
                    {
                        /* We can change it to line strip here for performance. But then
                        ** we would need to choose glChipDrawElements as the draw path
                        **/
                        if (chipCtx->indexLoops)
                        {
                            gcmONERROR(gcChipPatchLineLoopIndexed(gc, chipCtx, defaultInstant, gcvFALSE));
                        }
                        else
                        {
                            gcmONERROR(gcChipPatchLineLoop(gc, chipCtx, defaultInstant));
                        }
                    }
                    break;

                case gcvPRIMITIVE_LINE_STRIP:
                    /* Convert line strip to line list if HW cannot support */
                    if (forceInstancedDraw)
                    {
                        /* Convert line strip to line list */
                        if (chipCtx->indexLoops)
                        {
                            gcmONERROR(gcChipPatchLineStripIndexed(gc, chipCtx, defaultInstant, gcvFALSE));
                        }
                        else
                        {
                            gcmONERROR(gcChipPatchLineStrip(gc, chipCtx, defaultInstant));
                        }
                    }
                    break;

                default:
                    /* Code should not reach here normally */
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                /* Get draw command */
                gcChipGetDrawPath(gc, chipCtx, forceInstancedDraw);
            }

        } while (GL_FALSE);
    }

#if __GL_CHIP_PATCH_ENABLED
    if (chipCtx->patchInfo.patchFlags.vertexPack &&
        defaultInstant->indexBuffer &&
        gc->vertexArray.primMode == GL_TRIANGLES &&
        gc->shaderProgram.currentProgram)
    {
        __GLchipSLProgram *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
        __GLchipVertexBufferInfo *idxBufInfo = (__GLchipVertexBufferInfo*)idxBufObj->privateData;

        /* Route the draw through the patch logic. */
        __GLchipPatchClipInfo *clipInfo = gcChipPatchVertexPacking(gc,
                                                                   idxBufInfo,
                                                                   defaultInstant->indexType,
                                                                   __GL_PTR2SIZE(defaultInstant->indexMemory),
                                                                   defaultInstant->count);

#if gcdDUMP
        gcChipPatchDumpVertexPackingResult(clipInfo);
#endif

        /* Route the stream to bbox calculation as soon as the stream is finished */
        if (chipCtx->patchInfo.patchFlags.swClip && vsProgram->mvpUniform &&
            clipInfo &&
            clipInfo->packStatus == __GL_CHIP_PATCH_PACK_STATUS_READY)
        {

            __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
            instantDraw->first = 0;
            instantDraw->indexType      = clipInfo->indexType;
            instantDraw->indexMemory    = gcvNULL;
            instantDraw->indexBuffer    = clipInfo->indexObj;
            instantDraw->attributes     = clipInfo->newAttribs;
            instantDraw->attribMask     = clipInfo->newAttribMask;
            if (gcChipPatchBBoxClip(gc, &clipInfo->bboxes[__GL_CHIP_PATCH_MAIN_BBOX], vsProgram->mvpUniform->data))
            {
                gc->dp.drawPrimitive = __glChipDrawNothing;
            }

            /* Initiate bounding box split process */
            gcChipPatchSplitBBox(gc, clipInfo);

            /* check whether the bbox split has finished or not */
            if (clipInfo->bboxStatus == __GL_CHIP_PATCH_BBOX_STATUS_READY)
            {
                GLuint i;

                chipCtx->indexLoops = 0;

                /* for all bounding boxes */
                for (i = 0; i < __GL_CHIP_PATCH_BBOXES; i++)
                {
                    __GLchipPatchClipBox *bbox = &clipInfo->bboxes[i];

                    /* Enumerate every box, if the box contains vertices/triangles and
                    ** 1. It is main box that wasn't clipped, or
                    ** 2. sub box that wasn't clipped
                    */
                    if (bbox->count > 0 &&
                        (i == __GL_CHIP_PATCH_MAIN_BBOX ||
                         !gcChipPatchBBoxClip(gc, bbox, vsProgram->mvpUniform->data)))
                    {
                        GLuint loop = chipCtx->indexLoops++;
                        __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[loop];

                        if (loop != __GL_DEFAULT_LOOP)
                        {
                            __GL_MEMCOPY(instantDraw, defaultInstant, sizeof(__GLchipInstantDraw));
                        }

                        instantDraw->first = 0;
                        instantDraw->count = bbox->count;
                        instantDraw->primCount = bbox->count / 3;
                        instantDraw->indexBuffer = bbox->indexObj;
                    }
                }
            }

#if gcdDUMP
            gcChipPatchDumpVertexPackingResult(clipInfo);
#endif
        }
    }
#endif

#if gcdFRAMEINFO_STATISTIC
    {
        gctSTRING env;
        gctINT skipFrame = 0;
        gctUINT frameCount;
        gcoOS_GetEnv(gcvNULL, "SKIP_Frame", &env);
        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_FRAME_NUM,
                            gcvFRAMEINFO_OP_GET,
                            &frameCount);

        if (env)
        {
            gcoOS_StrToInt(env, &skipFrame);
        }

        if (((gctINT)frameCount < skipFrame) || g_dbgSkipDraw)
        {
            __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;
            if (((xfbObj == gcvNULL) ||
                 (!xfbObj->active)   ||
                 (xfbObj->paused)) &&
                ((gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED] == gcvNULL) &&
                 (gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE] == gcvNULL)
                )
               )
            {
                gc->dp.drawPrimitive = __glChipDrawNothing;
            }
        }
    }
#endif


OnError:
    if (gcmIS_ERROR(status))
    {
        /* Don't draw anything */
        gc->dp.drawPrimitive = __glChipDrawNothing;
    }

    gcmFOOTER();
    return status;
}

GLvoid
gcChipPatchFreeTmpAttibMem(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gctUINT i;

    /* Header */
    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < gc->constants.shaderCaps.maxUserVertAttributes; ++i)
    {
        gcsATTRIBUTE_PTR attrPtr = &chipCtx->attributeArray[i];
        if (attrPtr->tempStream != gcvNULL)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Destroy(attrPtr->tempStream));
            attrPtr->tempStream = gcvNULL;
        }

        if (attrPtr->tempMemory != gcvNULL)
        {
            (*gc->imports.free)(0, (gctPOINTER)attrPtr->tempMemory);
            attrPtr->tempMemory = gcvNULL;
        }
    }

    gcmFOOTER_NO();
    return;
}

__GL_INLINE gceSTATUS
gcChipValidateStream(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLprogramObject *vsProgObj = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_VS];
    __GLchipSLProgram  *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    GLuint vsInputArrayMask = vsProgObj->bindingInfo.vsInputArrayMask;
    /* INT attributes was not supported until halti2 */
    gctBOOL hwIntAttrib = chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3 ? gcvFALSE : gcvTRUE;
    GLint arrayIdx = -1;
    GLint attribIdx = 0;
    GLuint bitMask;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if ((!gc->vertexArray.varrayDirty              &&
            (instantDraw->indexBuffer != gcvNULL)) &&
            chipCtx->patchId == gcvPATCH_REALRACING)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (chipCtx->anyAttibConverted)
    {
        gcChipPatchFreeTmpAttibMem(gc);
    }

    chipCtx->anyAttibInstanced  = gcvFALSE;
    chipCtx->anyAttibGeneric = GL_FALSE;
    chipCtx->anyAttibConverted = gcvFALSE;
    chipCtx->positionIndex    = -1;
    chipCtx->attribMask = 0;
    chipCtx->directPositionIndex = -1;

    /* vsInputArrayMask means generic vertex attribute array index. */
    while (vsInputArrayMask)
    {
        gcsATTRIBUTE_PTR attribPtr;
        __GLchipSLLinkage* attribLinkage = gcvNULL;

        bitMask = (__GL_ONE_32 << ++arrayIdx);
        if (!(vsInputArrayMask & bitMask))
        {
            continue;
        }
        vsInputArrayMask &= ~bitMask;

        /*
        **  For aliasing attribute, multi-attributes will bind to one generic vertex attribute array index.
        **  chipCtx->attributeArray save all the attributes, including aliasing attributes.
        */
        for(attribLinkage = vsProgram->attribLinkage[arrayIdx]; attribLinkage != gcvNULL; attribLinkage = attribLinkage->next)
        {
            attribPtr = &chipCtx->attributeArray[attribIdx];
            attribPtr->linkage = attribLinkage->attribLocation;
            attribPtr->arrayIdx = (gctINT) arrayIdx;
            attribPtr->tempMemory = gcvNULL;
            attribPtr->tempStream = gcvNULL;

            /* Check whether VS required attribute was enabled by apps */
            if (vertexArrayState->attribEnabled & (__GL_ONE_32 << arrayIdx))
            {
                __GLbufferObject *bufObj;
                gceVERTEX_FORMAT format = gcvVERTEX_BYTE;
                __GLvertexAttrib *attribute = &vertexArrayState->attribute[arrayIdx];
                __GLvertexAttribBinding *attribBinding = &vertexArrayState->attributeBinding[attribute->attribBinding];
                GLboolean normalize = attribute->normalized;
                gceATTRIB_SCHEME scheme = gcvATTRIB_SCHEME_KEEP;

                /* Get the internal format */
                switch (attribute->type)
                {
                case GL_FLOAT:
                    format = gcvVERTEX_FLOAT;
                    normalize = GL_FALSE;
                    break;
                case GL_HALF_FLOAT_OES:
                case GL_HALF_FLOAT:
                    format = gcvVERTEX_HALF;
                    normalize = GL_FALSE;
                    break;
                case GL_FIXED:
                    format = gcvVERTEX_FIXED;
                    normalize = GL_FALSE;
                    break;

                case GL_UNSIGNED_INT:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT32;
                        if (!hwIntAttrib && (attribute->size < 4))
                        {
                            /*SW full map the attrib value*/
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_UINT_TO_UVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_UNSIGNED_INT;
                    }
                    break;

                case GL_INT:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT32;
                        if (!hwIntAttrib && (attribute->size < 4))
                        {
                            /*SW full map the attrib value*/
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_INT_TO_IVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_INT;
                    }
                    break;

                case GL_UNSIGNED_SHORT:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT16;
                        if (!hwIntAttrib)
                        {
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_USHORT_TO_UVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_UNSIGNED_SHORT;
                    }
                    break;

                case GL_SHORT:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT16;
                        if (hwIntAttrib && chipCtx->chipFeature.hwFeature.extendIntSign)
                        {
                            /* HW reuses normalize bit to extend sign to 32bit */
                            normalize = GL_TRUE;
                        }
                        else
                        {
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_SHORT_TO_IVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_SHORT;
                    }
                    break;

                case GL_UNSIGNED_BYTE:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT8;
                        if (!hwIntAttrib)
                        {
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_UBYTE_TO_UVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_UNSIGNED_BYTE;
                    }
                    break;

                case GL_BYTE:
                    if (attribute->integer)
                    {
                        format = gcvVERTEX_INT8;
                        if (hwIntAttrib && chipCtx->chipFeature.hwFeature.extendIntSign)
                        {
                            /* HW reuses normalize bit to extend sign to 32bit */
                            normalize = GL_TRUE;
                        }
                        else
                        {
                            chipCtx->anyAttibConverted = GL_TRUE;
                            scheme = gcvATTRIB_SCHEME_BYTE_TO_IVEC4;
                        }
                    }
                    else
                    {
                        format = gcvVERTEX_BYTE;
                    }
                    break;

                case GL_UNSIGNED_INT_10_10_10_2_OES:
                    format = gcvVERTEX_UNSIGNED_INT_10_10_10_2;
                    break;
                case GL_INT_10_10_10_2_OES:
                    format = gcvVERTEX_INT_10_10_10_2;
                    break;
                case GL_UNSIGNED_INT_2_10_10_10_REV:
                    format = gcvVERTEX_UNSIGNED_INT_2_10_10_10_REV;
                    if (!chipCtx->chipFeature.hwFeature.attrib2101010Rev)
                    {
                        chipCtx->anyAttibConverted = gcvTRUE;
                        scheme = gcvATTRIB_SCHEME_2_10_10_10_REV_TO_FLOAT;
                    }
                    break;

                case GL_INT_2_10_10_10_REV:
                    format = gcvVERTEX_INT_2_10_10_10_REV;
                    if (!chipCtx->chipFeature.hwFeature.attrib2101010Rev)
                    {
                        chipCtx->anyAttibConverted = gcvTRUE;
                        scheme = gcvATTRIB_SCHEME_2_10_10_10_REV_TO_FLOAT;
                    }
                    break;

                default:
                    format = gcvVERTEX_BYTE;
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    break;
                }

                /* Get vertex array attribute information. */
                attribPtr->enable     = gcvTRUE;
                attribPtr->size       = attribute->size;
                attribPtr->format     = format;
                attribPtr->normalized = normalize;
                attribPtr->stride     = attribBinding->stride;
                attribPtr->divisor    = attribBinding->divisor;
                if (attribPtr->divisor > 0)
                {
                    chipCtx->anyAttibInstanced = GL_TRUE;
                }

#if gcdUSE_WCLIP_PATCH
                if (attribPtr->linkage != (GLuint)-1 && vsProgram->attribLocation[attribPtr->linkage].pInput)
                {
                    attribPtr->isPosition = vsProgram->attribLocation[attribPtr->linkage].pInput->isPosition;

                    if (attribPtr->isPosition)
                    {
                        chipCtx->positionIndex = arrayIdx;
                    }

                    if (vsProgram->attribLocation[attribPtr->linkage].pInput->isDirectPosition)
                    {
                        chipCtx->directPositionIndex = arrayIdx;
                    }
                }
                else
                {
                    attribPtr->isPosition = gcvFALSE;
                }
#endif

                bufObj = __glGetCurrentVertexArrayBufObj(gc, attribute->attribBinding);
                if (bufObj)
                {
                    if (bufObj->size > 0)
                    {
                        __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);
                        attribPtr->stream = bufInfo->bufObj;
                        attribPtr->pointer = (gctCONST_POINTER)(attribBinding->offset + attribute->relativeOffset);
                    }
                    else
                    {
                        attribPtr->stream = gcvNULL;
                        attribPtr->enable = gcvFALSE;
                        attribPtr->genericValue[0] = gc->state.current.attribute[arrayIdx].f.x;
                        attribPtr->genericValue[1] = gc->state.current.attribute[arrayIdx].f.y;
                        attribPtr->genericValue[2] = gc->state.current.attribute[arrayIdx].f.z;
                        attribPtr->genericValue[3] = gc->state.current.attribute[arrayIdx].f.w;
                    }
                }
                else
                {
                    attribPtr->pointer = attribute->pointer;
                    attribPtr->stream = gcvNULL;
                }
#if gcdSYNC
                if (attribPtr->stream)
                {
                    gcoBUFOBJ_GetFence(attribPtr->stream, gcvFENCE_TYPE_READ);
                }
#endif
                attribPtr->convertScheme = scheme;
            }
            else
            {
                attribPtr->stream = gcvNULL;
                attribPtr->enable = gcvFALSE;
                attribPtr->genericValue[0] = gc->state.current.attribute[arrayIdx].f.x;
                attribPtr->genericValue[1] = gc->state.current.attribute[arrayIdx].f.y;
                attribPtr->genericValue[2] = gc->state.current.attribute[arrayIdx].f.z;
                attribPtr->genericValue[3] = gc->state.current.attribute[arrayIdx].f.w;

                chipCtx->anyAttibGeneric = GL_TRUE;
            }

            chipCtx->attribMask |= (__GL_ONE_32 << attribIdx);
            attribIdx++;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipValidateRenderTargetState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    __GLchipDirty *chipDirty = &chipCtx->chipDirty;

    gceSTATUS status = gcvSTATUS_OK;

    GLuint i, j, drawRTNumber;

    gcoSURF rtSurf, depthSurf;

    gcsSURF_VIEW *dsView = chipCtx->drawDepthView.surf ? &chipCtx->drawDepthView : &chipCtx->drawStencilView;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (chipDirty->uBuffer.bufferDirty)
    {
        GLint newPSOutputMapping[gcdMAX_DRAW_BUFFERS];
        GLint oldPSOutputMapping[gcdMAX_DRAW_BUFFERS];
        GLint *psOutputMapping = chipCtx->psOutputMapping;
        __GLchipDirty *chipDirty = &chipCtx->chipDirty;
        __GLchipSLProgram *fsProgram = chipCtx->activePrograms[__GLSL_STAGE_FS];
        __GLchipSLProgramInstance* pgInstance = fsProgram ? fsProgram->curPgInstance : gcvNULL;
        __GLchipHalRtSlotInfo rtHalMapping[__GL_MAX_DRAW_BUFFERS];
        GLuint halRTIndex = 0;

        __GL_MEMZERO(rtHalMapping, sizeof(rtHalMapping));

        /* Init new ps output mapping */
        for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
        {
            newPSOutputMapping[i] = -1;
        }

        /* Back up old ps output mapping */
        __GL_MEMCOPY(oldPSOutputMapping, psOutputMapping, sizeof(oldPSOutputMapping));

        if (chipDirty->uBuffer.sBuffer.rtSurfDirty)
        {
            /*
            ** We don't want to touch RT setting by APP.
            */
            gcsSURF_VIEW *pRTView;
            gcsSURF_VIEW tempRTView[gcdMAX_DRAW_BUFFERS];
            GLuint     rtLayerIndex[gcdMAX_DRAW_BUFFERS] = {0};
            GLuint     rtArraySize = 0;

            /*
            ** By default, all RT layer index is ZERO.
            */
            __GL_MEMZERO(tempRTView, sizeof(gcsSURF_VIEW) * gcdMAX_DRAW_BUFFERS);

            if (pgInstance &&
                (pgInstance->recompilePatchInfo.recompilePatchDirectivePtr) &&
                (pgInstance->pgStateKeyMask.s.hasRtPatchFmt))
            {
                gctUINT outputLoc[gcdMAX_SURF_LAYERS];
                gctUINT layers;

                /* Copy original RT setting. */
                __GL_MEMCOPY(tempRTView, chipCtx->drawRtViews, sizeof(gcsSURF_VIEW) * __GL_MAX_DRAW_BUFFERS);

                for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
                {
                    if (chipCtx->drawRtViews[i].surf)
                    {
                        if (gcmIS_ERROR(gcQueryOutputConversionDirective(pgInstance->recompilePatchInfo.recompilePatchDirectivePtr,
                                                                         i,
                                                                         outputLoc,
                                                                         &layers)))
                        {
                            layers = 1;
                            outputLoc[0] = i;
                        }

                        GL_ASSERT(layers <= gcdMAX_SURF_LAYERS);
                        for (j = 0; j < layers; ++j)
                        {
                            GLuint rtIndex = outputLoc[j];

                            GL_ASSERT(rtIndex < chipCtx->maxDrawRTs);

                            /* Either loc is itself, or loc is not occupied. */
                            GL_ASSERT((rtIndex == i) || (tempRTView[rtIndex].surf == gcvNULL));

                            tempRTView[rtIndex] = chipCtx->drawRtViews[i];
                            rtLayerIndex[rtIndex]  = j;
                        }
                    }
                }

                rtArraySize = chipCtx->maxDrawRTs;
                pRTView = tempRTView;
            }
            else
            {
                rtArraySize = gc->constants.shaderCaps.maxDrawBuffers;
                pRTView = &chipCtx->drawRtViews[0];
            }

            /* re-generate ps output mapping to remove bubble; */
            for (i = 0; i < rtArraySize; ++i)
            {
                if (pRTView[i].surf)
                {
                    for (j = 0; j < gc->constants.shaderCaps.maxDrawBuffers; ++j)
                    {
                        if (pRTView[i].surf == chipCtx->drawRtViews[j].surf &&
                        pRTView[i].firstSlice == chipCtx->drawRtViews[j].firstSlice)
                        {
                            rtHalMapping[j].slots[rtHalMapping[j].numOfSlots] = halRTIndex;
                            rtHalMapping[j].numOfSlots++;
                        }
                    }
                    newPSOutputMapping[halRTIndex++] = i ;
                }
            }

            if (chipCtx->drawRTnum != halRTIndex)
            {
                if ((chipCtx->drawRTnum == 0) || (halRTIndex == 0))
                {
                    chipDirty->uBuffer.sBuffer.zeroRTDirty = 1;
                }
                chipDirty->uBuffer.sBuffer.rtNumberDirty = 1;
                chipCtx->drawRTnum = halRTIndex;
            }

            if (pgInstance && pgInstance->pLastFragData)
            {
                chipDirty->uBuffer.sBuffer.rtNumberDirty = 1;
                chipCtx->drawRTnum = 0;
            }
            /* Update ps output mapping to HAL */
            gcmONERROR(gco3D_SetPSOutputMapping(chipCtx->engine, newPSOutputMapping));

            /* Save ps output mapping */
            __GL_MEMCOPY(psOutputMapping, newPSOutputMapping, sizeof(newPSOutputMapping));

            if (__GL_MEMCMP(chipCtx->rtHalMapping, rtHalMapping, sizeof(rtHalMapping)))
            {
                __GL_MEMCOPY(chipCtx->rtHalMapping, rtHalMapping, sizeof(rtHalMapping));
                /* Dirty states which depend on ps outputmapping */
                chipDirty->uDefer.sDefer.blend = 1;
                chipDirty->uDefer.sDefer.colorMask = 1;
            }

            /* Set new RT */
            for (i = 0; i < chipCtx->drawRTnum; ++i)
            {
                GLint rtIndex = psOutputMapping[i];
                GL_ASSERT(pRTView[rtIndex].surf);
                gcmONERROR(gco3D_SetTarget(chipCtx->engine, i, &pRTView[rtIndex], rtLayerIndex[rtIndex]));
            }

            /* Clear unused RT */
            for (i = chipCtx->drawRTnum; i < chipCtx->maxDrawRTs; ++i)
            {
                if (oldPSOutputMapping[i] != -1)
                {
                    GL_ASSERT((oldPSOutputMapping[i] >= 0) && (oldPSOutputMapping[i] < (GLint)chipCtx->maxDrawRTs));
                    gcmONERROR(gco3D_SetTarget(chipCtx->engine, i, gcvNULL, 0));
                }
            }
        }

        /* Since depth and stencil always share the same surface, so we don't need to set stencil surface. */
        if (chipDirty->uBuffer.sBuffer.zSurfDirty ||
            chipDirty->uBuffer.sBuffer.sSurfDirty ||
            chipDirty->uBuffer.sBuffer.zOffsetDirty ||
            chipDirty->uBuffer.sBuffer.sOffsetDirty
           )
        {
            gcmONERROR(gco3D_SetDepth(chipCtx->engine, dsView));

            if (chipDirty->uBuffer.sBuffer.zSurfDirty)
            {
                /* HAL will automatically set depthMode to NONE if there is no depth.
                ** Driver must set depthMode back if switched from non-depth mode.
                */
                chipDirty->uDefer.sDefer.depthMode = 1;
                chipDirty->uDefer.sDefer.depthTest = 1;
                chipDirty->uDefer.sDefer.depthMask = 1;
            }

            if (chipDirty->uBuffer.sBuffer.sSurfDirty)
            {
                chipDirty->uDefer.sDefer.stencilMode = 1;
                chipDirty->uDefer.sDefer.stencilTest = 1;
            }
        }

        /*
        ** If draw RT number changes, we need update color output number.
        ** This function must call after set/unset target, as we need loop all target to set function*/
        if (chipDirty->uBuffer.sBuffer.rtNumberDirty || chipDirty->uBuffer.sBuffer.rtSurfDirty)
        {
            gcmONERROR(gco3D_SetColorOutCount(chipCtx->engine, chipCtx->drawRTnum));
        }

        /*
        ** If draw RT number changes or surface change, we need update color cachemode.
        ** This function must call after gco3D_SetColorOutCount, we need loop all target to set function
        */
        if (chipDirty->uBuffer.sBuffer.rtNumberDirty ||
            chipDirty->uBuffer.sBuffer.rtSurfDirty)
        {
            gcmONERROR(gco3D_SetColorCacheMode(chipCtx->engine));
        }

        /*
        ** If switch between zero RT and non-zero RT, we need touch depth-only state.
        ** If s/d surface change, we need check if there is no d/s surface,
        ** as HAL assume there should have depth/stencil surface for depthOnly.
        */
        if (chipDirty->uBuffer.sBuffer.zeroRTDirty ||
            chipDirty->uBuffer.sBuffer.sSurfDirty  ||
            chipDirty->uBuffer.sBuffer.zSurfDirty)
        {
            gcmONERROR(gco3D_SetDepthOnly(chipCtx->engine, ((chipCtx->drawRTnum == 0) && (dsView->surf != gcvNULL))));
        }

        if (chipDirty->uBuffer.sBuffer.rtSamplesDirty)
        {
            gcmONERROR(gco3D_SetSamples(chipCtx->engine, chipCtx->drawRTSamples));
            gcmONERROR(gco3D_SetMinSampleShadingValue(chipCtx->engine, gc->state.multisample.minSampleShadingValue));
        }

        if (chipDirty->uBuffer.sBuffer.layeredDirty)
        {
            gcmONERROR(gco3D_SetRenderLayered(chipCtx->engine,
                                              chipCtx->drawLayered ? gcvTRUE : gcvFALSE,
                                              chipCtx->drawMaxLayers));
        }
    }

    /* Get fence if need. */
    if(!(chipCtx->chipFeature.hwFeature.hasBlitEngine))
    {
        /* Get fence for color attachment texture surafce. */
        drawRTNumber = chipCtx->drawRTnum < __GL_MAX_DRAW_BUFFERS ? chipCtx->drawRTnum : __GL_MAX_DRAW_BUFFERS;
        for (i = 0; i < drawRTNumber; i++)
        {
            rtSurf = chipCtx->drawRtViews[i].surf;
            if(rtSurf && (rtSurf->hints & gcvSURF_CREATE_AS_TEXTURE))
            {
                gcmONERROR(gcoSURF_GetFence(rtSurf, gcvFENCE_TYPE_WRITE));
            }
        }

        /* Get fence for depth attachment texture surafce. */
        depthSurf = dsView->surf;
        if(depthSurf && (depthSurf->hints & gcvSURF_CREATE_AS_TEXTURE))
        {
            gcmONERROR(gcoSURF_GetFence(dsView->surf, gcvFENCE_TYPE_WRITE));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

/* Here we need patch for some hw limitations */
__GL_INLINE gceSTATUS
gcChipValidatePatchState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (!gc->state.enables.rasterizerDiscard && fbo && fbo->shadowRender)
    {
        GLbitfield writeMask = GL_COLOR_BUFFER_BIT;

        if (gc->state.enables.depthTest && gc->state.depth.testFunc != GL_NEVER)
        {
            writeMask |= GL_DEPTH_BUFFER_BIT;
        }

        /* Stencil update related to depth, not evaluated here */
        if (gc->state.enables.stencilTest)
        {
            writeMask |= GL_STENCIL_BUFFER_BIT;
        }

        gcmONERROR(gcChipFBOMarkShadowRendered(gc, fbo, writeMask));
    }
#if __GL_CHIP_PATCH_ENABLED
    if (fbo && fbo->name)
    {
        /* Current in framebuffer, flag bound depth texture if touched */
        __GLfboAttachPoint* attachPoint = &fbo->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];

        if (attachPoint->objType == GL_TEXTURE)
        {
            __GLtextureObject *texObj = (__GLtextureObject*)attachPoint->object;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
            texInfo->isFboRendered = 1;
        }
    }
#endif

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipValidateLastFragDataUniform(
    __GLcontext *gc,
    __GLchipSLUniform *uniform
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceTILING tiling = gcvINVALIDTILED;
    gctPOINTER logicalAddress[3] = {gcvNULL};
    gctUINT32 baseAddress[3] = {0};
    GLuint width = 0, height = 0;
    gctINT stride = 0;
    GLuint imageInfo = 0;
    gctUINT *data;
    GLuint arrayIndex;

    if (uniform->category != gceTK_IMAGE || uniform->subUsage != __GL_CHIP_UNIFORM_SUB_USAGE_RT_IMAGE)
    {
        gcmASSERT(0);
    }

    for (arrayIndex = 0; arrayIndex < uniform->arraySize; ++arrayIndex)
    {
        if (chipCtx->drawRtViews[arrayIndex].surf)
        {
            gcsSURF_VIEW *rtView = &chipCtx->drawRtViews[arrayIndex];
            width = (gctUINT)chipCtx->drawRTWidth;
            height = (gctUINT)chipCtx->drawRTHeight;
            gcmASSERT(rtView->surf);
            gcmONERROR(gcoSURF_GetFormat(rtView->surf, gcvNULL, &format));

            gcmONERROR(gcoSURF_Lock(rtView->surf, baseAddress, logicalAddress));
            gcmONERROR(gcoSURF_GetAlignedSize(rtView->surf, gcvNULL, gcvNULL, &stride));

            gcmONERROR(gcoSURF_DisableTileStatus(rtView, gcvTRUE));
            gcmONERROR(gcoSURF_GetTiling(rtView->surf, &tiling));
            gcmONERROR(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));

            switch (tiling)
            {
            case gcvLINEAR:
                /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, LINEAR); */
                imageInfo = 0;
                break;
            case gcvTILED:
                /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, TILED); */
                imageInfo = 1 << 10;
                break;
            case gcvSUPERTILED:
                /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, SUPER_TILED); */
                imageInfo = 2 << 10;
                break;
            case gcvYMAJOR_SUPERTILED:
                /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, SUPER_TILED_YMAJOR); */
                imageInfo = 3 << 10;
                break;
            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                break;
            }
            /*
            imageInfo |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)));
            */
            imageInfo |= (1 << 12);
            /* load image from render target*/
            switch (format)
            {
            case gcvSURF_A8R8G8B8:
                /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (GCREG_SH_IMAGE_SHIFT_2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))           |
                gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                */
                imageInfo |= 0xF << 6 | 2 | 0 << 14;
                /*
                imageInfo |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x3 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
                */
                imageInfo |= (2 << 16) | (1 << 20) | (0 << 24) | (3 << 28);

                imageInfo |= 1 << 4;
                break;
            case gcvSURF_X8R8G8B8:
                /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (GCREG_SH_IMAGE_SHIFT_2 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))           |
                gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 3);
                */
                imageInfo |= 0xF << 6 | 2 | 3 << 14;
                /*
                imageInfo |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
                */
                imageInfo |= (2 << 16) | (1 << 20) | (0 << 24) | (4 << 28);

                imageInfo |= 1 << 4;
                break;
            case gcvSURF_R5G6B5:
                /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, PACKED565) |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 2:0) - (0 ?
 2:0) + 1))))))) << (0 ?
 2:0))) | (((gctUINT32) (GCREG_SH_IMAGE_SHIFT_1 & ((gctUINT32) ((((1 ?
 2:0) - (0 ?
 2:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 2:0) - (0 ? 2:0) + 1))))))) << (0 ? 2:0)))           |
                gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 3);
                */
                imageInfo |= 0x9 << 6 | 1 | 3 << 14;
                /*
                imageInfo |= ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) (0x2 & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) (0x1 & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))    |
                ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) (0x4 & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));
                */
                imageInfo |= (2 << 16) | (1 << 20) | (0 << 24) | (4 << 28);

                imageInfo |= 1 << 4;
                break;
            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                break;
            }
            data = (gctUINT*)((GLubyte*)uniform->data + arrayIndex * sizeof(gctFLOAT) * 4);
            data[0] = (gctUINT)(baseAddress[0] + gcChipGetSurfOffset(rtView));
            data[1] = (gctUINT)stride;
            data[2] = width | (height << 16);
            data[3] = imageInfo;
        }
    }
    uniform->dirty = GL_TRUE;

OnError:
    return status;
}

__GL_INLINE gceSTATUS
gcChipValidateChipDirty(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (chipCtx->chipDirty.uDefer.deferDirty)
    {
        __GLchipSLProgram *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];

        if (chipCtx->chipDirty.uDefer.sDefer.depthMode)
        {
            gcmONERROR(gcChipSetDepthMode(gc));
        }

        /* depthRange depends on depthMode */
        if (chipCtx->chipDirty.uDefer.sDefer.depthRange)
        {
            gcmONERROR(gcChipSetDepthRange(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.depthTest)
        {
            gcmONERROR(gcChipSetDepthTest(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.depthMask)
        {
            gcmONERROR(gcChipSetDepthMask(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.stencilRef)
        {
            gcmONERROR(gcChipSetStencilRef(gc, chipCtx));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.stencilMode)
        {
            gcmONERROR(gcChipSetStencilMode(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.stencilTest)
        {
            gcmONERROR(gcChipSetStencilTest(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.viewportScissor)
        {
            gcmONERROR(gcChipSetViewportScissor(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.culling)
        {
            gcmONERROR(gcChipSetCulling(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.colorMask)
        {
            gcmONERROR(gcChipSetColorMask(gc));

        }

        if (chipCtx->chipDirty.uDefer.sDefer.blend)
        {
            gcmONERROR(gcChipSetAlphaBlend(gc));
        }

#if gcdALPHA_KILL_IN_SHADER
        if (chipCtx->chipDirty.uDefer.sDefer.blend || chipCtx->chipDirty.uDefer.sDefer.fsReload)
        {
            gcmONERROR(gcChipSetAlphaKill(gc));
        }
#endif

        if(chipCtx->chipDirty.uDefer.sDefer.polygonOffset)
        {
            gcmONERROR(gcChipSetPolygonOffset(gc));
        }

        if (chipCtx->chipDirty.uDefer.sDefer.vsReload  ||
            chipCtx->chipDirty.uDefer.sDefer.fsReload  ||
            chipCtx->chipDirty.uDefer.sDefer.tcsReload ||
            chipCtx->chipDirty.uDefer.sDefer.tesReload ||
            chipCtx->chipDirty.uDefer.sDefer.gsReload)
        {
            if (gc->shaderProgram.currentProgram)
            {
                __GLchipSLProgramInstance* pgInstance = vsProgram->curPgInstance;

                GL_ASSERT(chipCtx->activePrograms[__GLSL_STAGE_VS] == chipCtx->activePrograms[__GLSL_STAGE_FS]);
                GL_ASSERT((chipCtx->activePrograms[__GLSL_STAGE_TCS] == gcvNULL) ||
                          ((chipCtx->activePrograms[__GLSL_STAGE_TCS] == chipCtx->activePrograms[__GLSL_STAGE_TES]) &&
                           (chipCtx->activePrograms[__GLSL_STAGE_TCS] == chipCtx->activePrograms[__GLSL_STAGE_VS]))
                         );
                GL_ASSERT((chipCtx->activePrograms[__GLSL_STAGE_GS] == gcvNULL) ||
                          (chipCtx->activePrograms[__GLSL_STAGE_GS] == chipCtx->activePrograms[__GLSL_STAGE_VS])
                         );
                gcmONERROR(gco3D_LoadProgram(chipCtx->engine,
                                             pgInstance->programState.hints->stageBits,
                                             &pgInstance->programState));

                chipCtx->activeProgState = &pgInstance->programState;
            }
            else if (gc->shaderProgram.boundPPO)
            {
                __GLSLStage stage;
                gctINT   shaderCount = 0;
                gcSHADER shaderArray[__GLSL_STAGE_LAST] = {0};
                GLuint key;
                __GLchipUtilsObject *hashObj;
                __GLchipProgCmdStateKey cmdStateKey;
                gcsPROGRAM_STATE_PTR pCmdInstance = gcvNULL;

                for (stage = 0; stage < __GLSL_STAGE_LAST; ++stage)
                {
                    __GLprogramObject *progObj = gc->shaderProgram.activeProgObjs[stage];
                    __GLchipSLProgram *program = chipCtx->activePrograms[stage];
                    if (program)
                    {
                        shaderArray[shaderCount++] = program->curPgInstance->binaries[stage];

                        GL_ASSERT(progObj);
                        cmdStateKey.stages[stage].progId = progObj->programInfo.uniqueId;
                        cmdStateKey.stages[stage].linkSeq = program->codeSeq;
                        cmdStateKey.stages[stage].instanceId = program->curPgInstance->instanceId;
                    }
                    else
                    {
                        cmdStateKey.stages[stage].progId = 0;
                        cmdStateKey.stages[stage].linkSeq = 0;
                        cmdStateKey.stages[stage].instanceId = 0;
                    }
                }
                key = gcChipUtilsEvaluateCRC32(&cmdStateKey, sizeof(cmdStateKey));

                if (!chipCtx->cmdInstaceCache)
                {
                    chipCtx->cmdInstaceCache = gcChipUtilsHashCreate(gc,
                                                                     __GL_PROG_CMD_HASH_ENTRY_NUM,
                                                                     __GL_PROG_CMD_HASH_ENTRY_SIZE,
                                                                     gcChipProgFreeCmdInstance);

                    if (!chipCtx->cmdInstaceCache)
                    {
                        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
                    }
                }

                hashObj = gcChipUtilsHashFindObjectByKey(gc, chipCtx->cmdInstaceCache, key);
                /* If the cmd instance never linked before */
                if (hashObj == gcvNULL)
                {
                    pCmdInstance = (gcsPROGRAM_STATE_PTR)gc->imports.calloc(gc, 1, sizeof(gcsPROGRAM_STATE));

                    gcmONERROR(gcLinkProgramPipeline(shaderCount,
                                                     shaderArray,
                                                     pCmdInstance));

                    hashObj = gcChipUtilsHashAddObject(gc, chipCtx->cmdInstaceCache, (GLvoid*)pCmdInstance, key, GL_FALSE);
                    if (!hashObj)
                    {
                        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
                    }
                }
                else
                {
                    pCmdInstance = (gcsPROGRAM_STATE_PTR)hashObj->pUserData;
                }

                gcmONERROR(gco3D_LoadProgram(chipCtx->engine,
                                             pCmdInstance->hints->stageBits,
                                             pCmdInstance));

                chipCtx->activeProgState = pCmdInstance;
            }
            else
            {
                GL_ASSERT(0);
            }
        }

        /* Validate RT info into the lastFragData image uniform*/
        if (chipCtx->chipDirty.uDefer.sDefer.lastFragData)
        {
            __GLchipSLProgramInstance* pgInstance = chipCtx->activePrograms[__GLSL_STAGE_FS] ?
                chipCtx->activePrograms[__GLSL_STAGE_FS]->curPgInstance : gcvNULL;
            if (pgInstance && pgInstance->pLastFragData)
            {
                gcChipValidateLastFragDataUniform(gc, pgInstance->pLastFragData);
            }
        }
    }

    if (chipCtx->chipDirty.uPatch.patchDirty)
    {
        gcmONERROR(gcChipSetViewportScissor(gc));
    }

    /* Flush shader resource (including xfb res) after program is loaded to HAL */
    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipFlushGLSLResourcesCB));
    gcmONERROR(gcChipValidateXFB(gc, chipCtx));

OnError:
    gcmFOOTER();
    return status;
}

/**********************************************************************************************/
/*Implementation of EXPORTED FUNCTIONS                                                        */
/**********************************************************************************************/
gceSTATUS
gcChipInitializeDraw(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status;
    gctSIZE_T i;
    gcsSURF_VIEW nullView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    for (i = 0; i < __GL_MAX_VERTEX_ATTRIBUTES; ++i)
    {
        chipCtx->attributeArray[i].genericValue[0] = 0.0f;
        chipCtx->attributeArray[i].genericValue[1] = 0.0f;
        chipCtx->attributeArray[i].genericValue[2] = 0.0f;
        chipCtx->attributeArray[i].genericValue[3] = 1.0f;
        chipCtx->attributeArray[i].genericSize     = 4;
        chipCtx->attributeArray[i].enable = gcvTRUE;
    }

    /* Construct the vertex array. */
    status = gcoVERTEXARRAY_Construct(chipCtx->hal, &chipCtx->vertexArray);

    /* Initialize last primitive type. */
    chipCtx->lastPrimitiveType = (GLenum)-1;

    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; ++i)
    {
        chipCtx->drawRtViews[i] = nullView;
    }
    chipCtx->drawDepthView = nullView;
    chipCtx->drawStencilView = nullView;
    chipCtx->readRtView = nullView;
    chipCtx->readDepthView = nullView;
    chipCtx->readStencilView = nullView;

    for (i = 0; i < gcdMAX_DRAW_BUFFERS; ++i)
    {
        chipCtx->psOutputMapping[i] = -1;
    }

    /* Mark all chip layer buffer dirty. */
    chipCtx->chipDirty.uBuffer.bufferDirty = (GLuint)~0;

    gcmFOOTER();
    /* Return status. */
    return status;
}

gceSTATUS
gcChipDeinitializeDraw(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);
    status = gcoVERTEXARRAY_Destroy(chipCtx->vertexArray);
    chipCtx->vertexArray = gcvNULL;
    gcmFOOTER();
    return status;
}

__GL_INLINE gctBOOL
gcChipCheckTriangle2CCW(
__GLcontext *gc,
__GLchipInstantDraw* instantDraw
)
{
    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);

    gctSIZE_T i;
    gctINT j;
    gcsATTRIBUTE_PTR attrib;
    gctPOINTER indexPtr = gcvNULL;
    gctFLOAT *vertexPtr;
    gceSTATUS status = gcvSTATUS_OK;
    gctFLOAT vertexs[3][4] = {
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
    };
    gctFLOAT det = 0.0;
    gctBOOL ret = gcvFALSE;
    gcmHEADER_ARG("gc=0x%x instantDraw=%d", gc, instantDraw);

    attrib = &instantDraw->attributes[chipCtx->directPositionIndex];

    for (i = 0; i < instantDraw->count; i++)
    {
        gctSIZE_T vertex;
        gctFLOAT vector[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        if (chipCtx->indexLoops)
        {
            if (instantDraw->indexBuffer)
            {
                gcmONERROR(gcoBUFOBJ_FastLock(instantDraw->indexBuffer, gcvNULL, &indexPtr));
                indexPtr = (gctPOINTER)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(indexPtr) + gcmPTR_TO_UINT64(instantDraw->indexMemory)));
            }
            else
            {
                indexPtr = instantDraw->indexMemory;
            }

            switch (instantDraw->indexType)
            {
            case gcvINDEX_8:
                vertex = *(gctUINT8 *)((gctUINT8_PTR)indexPtr + i * sizeof(GLubyte));
                break;

            case gcvINDEX_16:
                vertex = *(gctUINT16 *)((gctUINT8_PTR)indexPtr + i * sizeof(GLushort));
                break;

            case gcvINDEX_32:
                vertex = *(gctUINT *)((gctUINT8_PTR)indexPtr + i * sizeof(GLuint));
                break;

            default:
                vertex = 0;
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                break;
            }
        }
        else
        {
            vertex = i;
        }

        if (attrib->stream)
        {
            gcmONERROR(gcoBUFOBJ_FastLock(attrib->stream, gcvNULL, (gctPOINTER *)&vertexPtr));
            vertexPtr = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(vertexPtr) + gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex) * attrib->stride));
        }
        else
        {
            vertexPtr = (gctFLOAT *)gcmUINT64_TO_PTR((gcmPTR_TO_UINT64(attrib->pointer) + (instantDraw->first + vertex) * attrib->stride));
        }


        /* Get Value */
        if ((((gcmPTR2SIZE(vertexPtr)) & 3) == 0))
        {
            for (j = 0; j < attrib->size; j++)
            {
                vector[j] = vertexPtr[j];
            }
        }
        else
        {
            for (j = 0; j < attrib->size; j++)
            {

                readDataForWLimit(vertexPtr + j, vector[j])
            }

        }

        /*MVP matrix is identiy */
        vertexs[i][0] = vector[0];

        vertexs[i][1] = vector[1];

        vertexs[i][2] = vector[2];

        vertexs[i][3] = vector[3];
    }
    det = vertexs[0][0] * (vertexs[1][1] * vertexs[2][3] - vertexs[1][3] * vertexs[2][1])
        - vertexs[0][1] * (vertexs[1][0] * vertexs[2][3] - vertexs[1][3] * vertexs[2][0])
        + vertexs[0][3] * (vertexs[1][0] * vertexs[2][1] - vertexs[1][1] * vertexs[2][0]);

    if (det > 0.0f)
    {
        ret = gcvTRUE;
    }
OnError:
    gcmFOOTER();
    return ret;
}

/*****************************************************************************
** split draw func
******************************************************************************/
gceSTATUS
gcChipSplitDrawXFB(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);

    gctSIZE_T i;
    gctSIZE_T vertices = 0;
    __GLchipInstantDraw tmpInstantDraw;

    gcmHEADER();

    switch (instantDraw->primMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
        vertices = 1;
        break;
    case gcvPRIMITIVE_LINE_LIST:
        vertices = 2;
        break;
    case gcvPRIMITIVE_TRIANGLE_LIST:
        vertices = 3;
        break;
    default:
        break;
    }

    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    tmpInstantDraw.primCount = 1;
    tmpInstantDraw.count = vertices;

    for (i = 0; i < instantDraw->count / vertices; ++i)
    {
        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvFALSE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));

        tmpInstantDraw.first += (gctINT)vertices;

        gcmONERROR(gco3D_Semaphore(chipCtx->engine, gcvWHERE_COMMAND, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDraw1(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    gctSIZE_T i;
    __GLchipInstantDraw tmpInstantDraw;

    gcmHEADER();

    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    tmpInstantDraw.primCount = 1;

    for (i = 0; i < instantDraw->count - 1 ; ++i)
    {
        tmpInstantDraw.count = 2;
        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvTRUE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));

        tmpInstantDraw.indexMemory = (gctUINT8_PTR)(1 + __GL_PTR2SIZE(tmpInstantDraw.indexMemory));
        gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL);
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDraw2(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    gctSIZE_T indexSize              = 0;
    gctSIZE_T i;
    __GLchipInstantDraw tmpInstantDraw;
    gctUINT mask, writeMask;
    gctUINT8 reference;

    gcmHEADER();

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    tmpInstantDraw.primCount = 1;
    tmpInstantDraw.count = 3;

    for (i = 0; i < instantDraw->count / 3; ++i)
    {
        gctBOOL frontFace;
        frontFace = gcChipCheckTriangle2CCW(gc, &tmpInstantDraw);
        if (gc->state.polygon.frontFace == GL_CW)
        {
            frontFace = !frontFace;
        }

        if (frontFace)
        {
            mask = gc->state.stencil.front.mask;
            writeMask = gc->state.stencil.front.writeMask;
            reference = (gctUINT8)__glClampi(gc->state.stencil.front.reference, 0, chipCtx->drawStencilMask);
        }
        else
        {
            mask = gc->state.stencil.back.mask;
            writeMask = gc->state.stencil.back.writeMask;
            reference = (gctUINT8)__glClampi(gc->state.stencil.back.reference, 0, chipCtx->drawStencilMask);
        }

        gcmONERROR(gco3D_SetStencilWriteMask(chipCtx->engine, (gctUINT8)(writeMask & 0x00FF)));
        gcmONERROR(gco3D_SetStencilMask(chipCtx->engine, (gctUINT8)mask));
        gcmONERROR(gco3D_SetStencilReference(chipCtx->engine, reference, gcvTRUE));

        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvTRUE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));

        tmpInstantDraw.indexMemory = (gctUINT8_PTR)(3 * indexSize + __GL_PTR2SIZE(tmpInstantDraw.indexMemory));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDraw3(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    gctSIZE_T i;
    __GLchipInstantDraw tmpInstantDraw;

    gcmHEADER();

    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    tmpInstantDraw.primCount = 1;
    tmpInstantDraw.count = 2;

    for (i = 0; i < instantDraw->count - 1 ; ++i)
    {
        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvFALSE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));

        tmpInstantDraw.first += 1;
        gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE_STALL);
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDraw4Instanced(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    __GLchipInstantDraw tmpInstantDraw;
    gctUINT i, splitParts = 3;
    gctFLOAT offset = gc->state.line.requestedWidth / (gctFLOAT)(gc->state.viewport.height);
    /* faked attributes position and color*/
    gctFLOAT attribs[] = { -0.8f, -1.01f + offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         -0.8f, -1.01f - offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.0f, -1.01f + offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.0f, -1.01f - offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    gcmHEADER();

    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));

    for (i = 0; i < splitParts; i++)
    {
        switch (i)
        {
        case 0:
            /* Draw the 1-9 lines*/
            tmpInstantDraw.primCount = 9;
            tmpInstantDraw.count = 18;
            break;
        case 1:
            /* Draw the 11th line*/
            tmpInstantDraw.first += 20;
            tmpInstantDraw.primCount = 1;
            tmpInstantDraw.count = 2;
            break;
        case 2:
            /* Draw the 10th lines and split it to two triangles*/
            tmpInstantDraw.attributes->pointer = attribs;
            tmpInstantDraw.primMode = gcvPRIMITIVE_TRIANGLE_STRIP;
            tmpInstantDraw.count = 4;
            tmpInstantDraw.primCount = 2;
            tmpInstantDraw.first = 0;
            break;
        default:
            break;
        }
        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvFALSE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDraw4(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                 = gcvSTATUS_OK;
    __GLcontext* gc                  = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    __GLchipInstantDraw tmpInstantDraw;
    gctUINT i, splitParts = 3;
    gctFLOAT offset = gc->state.line.requestedWidth / (gctFLOAT)(gc->state.viewport.height);
    /* faked attributes position and color*/
    gctFLOAT attribs[] = { -0.8f, -1.01f + offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         -0.8f, -1.01f - offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.0f, -1.01f + offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                         0.0f, -1.01f - offset, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    gcmHEADER();

    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));

    for (i = 0; i < splitParts; i++)
    {
        switch (i)
        {
        case 0:
            /* Draw the 1-9 lines*/
            tmpInstantDraw.primCount = 9;
            break;
        case 1:
            /* Draw the 11th line*/
            tmpInstantDraw.first = 20;
            tmpInstantDraw.primCount = 1;
            break;
        case 2:
            /* Draw the 10th lines and split it to two triangles*/
            tmpInstantDraw.attributes->pointer = attribs;
            tmpInstantDraw.primMode = gcvPRIMITIVE_TRIANGLE_STRIP;
            tmpInstantDraw.primCount = 2;
            tmpInstantDraw.first = 0;
            break;
        default:
            break;
        }
        /* Bind vertex array */
        gcmONERROR(gcChipSetVertexArrayBind(gc, &tmpInstantDraw, gcvTRUE, gcvTRUE));

        /* Draw */
        gcmONERROR(gco3D_DrawPrimitives(chipCtx->engine,
                                        tmpInstantDraw.primMode,
                                        tmpInstantDraw.first,
                                        tmpInstantDraw.primCount));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipCopySpilitIndex(
    IN __GLchipInstantDraw* instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo,
    IN OUT gctPOINTER * Buffer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER indexBase = gcvNULL;
    gctPOINTER tempIndices = gcvNULL;
    gctPOINTER indexMemory = gcvNULL;
    gctSIZE_T count, primCount, i, j, bytes;
    gctSIZE_T indexSize = 0;
    gctSIZE_T offset = 0;
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcmHEADER();

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    offset = (instantDraw->count - splitDrawInfo->u.info_index_fetch.splitCount) * indexSize;
    gcoHAL_GetPatchID(gcvNULL, &patchId);

    /* Lock the index buffer. */
#if gcdSYNC
    if (patchId == gcvPATCH_GTFES30)
    {
        gcoBUFOBJ_WaitFence(instantDraw->indexBuffer, gcvFENCE_TYPE_WRITE);
    }
#endif
    gcmONERROR(gcoBUFOBJ_FastLock(instantDraw->indexBuffer, gcvNULL, &indexBase));

    indexBase = (gctUINT8_PTR)indexBase + gcmPTR2INT32(instantDraw->indexMemory);
    bytes = splitDrawInfo->u.info_index_fetch.splitCount * indexSize;

    splitDrawInfo->u.info_index_fetch.splitPrimMode = instantDraw->primMode;

    switch (instantDraw->primMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
    case gcvPRIMITIVE_LINE_LIST:
    case gcvPRIMITIVE_TRIANGLE_LIST:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset;

            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            gcoOS_MemCopy(tempIndices, indexMemory, bytes);
        }
        break;
    case gcvPRIMITIVE_LINE_STRIP:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            /* line strip need copy the last data */
            gcoOS_MemCopy(tempIndices, indexMemory, bytes);
        }
        break;
    case gcvPRIMITIVE_LINE_LOOP:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += 2 * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));

            /* line loop need copy the last data and the first data */
            gcoOS_MemCopy(tempIndices, indexMemory, bytes - indexSize);
            gcoOS_MemCopy((gctUINT8_PTR)tempIndices + bytes - indexSize, indexBase, indexSize);
            splitDrawInfo->u.info_index_fetch.splitPrimMode = gcvPRIMITIVE_LINE_STRIP;
        }
        break;
    case gcvPRIMITIVE_TRIANGLE_STRIP:
        {
            primCount = bytes / indexSize;
            bytes = 3 * primCount * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));
            count = offset / indexSize;
            switch (instantDraw->indexType)
            {
            case gcvINDEX_8:
                {
                    gctUINT8_PTR src = (gctUINT8_PTR)indexBase;
                    gctUINT8_PTR dst = (gctUINT8_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            case gcvINDEX_16:
                {
                    gctUINT16_PTR src = (gctUINT16_PTR)indexBase;
                    gctUINT16_PTR dst = (gctUINT16_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            case gcvINDEX_32:
                {
                    gctUINT32_PTR src = (gctUINT32_PTR)indexBase;
                    gctUINT32_PTR dst = (gctUINT32_PTR)tempIndices;
                    for(i = 0, j = count - 2; i < primCount; i++, j++)
                    {
                        dst[i * 3]     = src[(j % 2) == 0? j : j + 1];
                        dst[i * 3 + 1] = src[(j % 2) == 0? j + 1 : j];
                        dst[i * 3 + 2] = src[j + 2];
                    }
                }
                break;
            default:
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            splitDrawInfo->u.info_index_fetch.splitPrimMode = gcvPRIMITIVE_TRIANGLE_LIST;
        }
        break;
    case gcvPRIMITIVE_TRIANGLE_FAN:
        {
            indexMemory = (gctUINT8_PTR)indexBase + offset - indexSize;
            bytes += 2 * indexSize;
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      bytes,
                                      &tempIndices));

            /* trianglefan need copy the first data */
            gcoOS_MemCopy(tempIndices, indexBase, indexSize);
            gcoOS_MemCopy((gctUINT8_PTR)tempIndices + indexSize, indexMemory, bytes - indexSize);
        }
        break;
    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    splitDrawInfo->u.info_index_fetch.splitCount = bytes / indexSize;

    /* Translate primitive count. */
    switch (splitDrawInfo->u.info_index_fetch.splitPrimMode)
    {
    case gcvPRIMITIVE_POINT_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount;
        break;

    case gcvPRIMITIVE_LINE_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount / 2;
        break;

    case gcvPRIMITIVE_LINE_LOOP:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount;
        break;

    case gcvPRIMITIVE_LINE_STRIP:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount - 1;
        break;

    case gcvPRIMITIVE_TRIANGLE_LIST:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount / 3;
        break;

    case gcvPRIMITIVE_TRIANGLE_STRIP:
    case gcvPRIMITIVE_TRIANGLE_FAN:
        splitDrawInfo->u.info_index_fetch.splitPrimCount = splitDrawInfo->u.info_index_fetch.splitCount - 2;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    *Buffer = tempIndices;

OnError:

    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDrawIndexFetch(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                     = gcvSTATUS_OK;
    __GLcontext* gc                      = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw     = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx             = CHIP_CTXINFO(gc);
    gcsSPLIT_DRAW_INFO_PTR splitDrawInfo = (gcsSPLIT_DRAW_INFO_PTR)(SplitDrawInfo);

    gcePATCH_ID patchId = gcvPATCH_INVALID;
    gctPOINTER splitIndexMemory = gcvNULL;
    gctBOOL bAllocate = gcvFALSE;
    __GLchipInstantDraw tmpInstantDraw;
    gcsVERTEXARRAY_STREAM_INFO streamInfo;
    gcsVERTEXARRAY_INDEX_INFO  indexInfo;

    gcmHEADER();

    gcmONERROR(gcChipSetVertexArrayBindBegin(gc, instantDraw, gcvTRUE));

    /* Stream data not change, only need bind once.*/
    /* Collect info for hal level.*/
    gcmES30_COLLECT_STREAM_INFO(streamInfo, instantDraw, gc, chipCtx, gcvTRUE);
    gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw);

#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRms,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRmsDirty,
                                         &streamInfo,
                                         &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         &streamInfo,
                                         &indexInfo));
#endif

    /************************************************************************************
    **              first draw
    ************************************************************************************/
    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    gcoHAL_GetPatchID(gcvNULL, &patchId);

#if gcdSYNC
    if (patchId == gcvPATCH_GTFES30)
    {
        gcoBUFOBJ_WaitFence(instantDraw->indexBuffer, gcvFENCE_TYPE_WRITE);
    }
#endif
    if (instantDraw->primitiveRestart || instantDraw->count <= splitDrawInfo->u.info_index_fetch.splitCount)
    {
        tmpInstantDraw.count = 0;
    }
    else
    {
        tmpInstantDraw.count = instantDraw->count - splitDrawInfo->u.info_index_fetch.splitCount;
        if (instantDraw->primMode == gcvPRIMITIVE_LINE_LOOP)
        {
            tmpInstantDraw.primMode = gcvPRIMITIVE_LINE_STRIP;
        }
    }

    if (tmpInstantDraw.count > 0)
    {
        /* Update index */
        indexInfo.count = tmpInstantDraw.count;
        gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                            &indexInfo));

        /* Draw */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvTRUE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));
    }


    /************************************************************************************
    **              second draw
    ************************************************************************************/
    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));

    if (instantDraw->primitiveRestart || instantDraw->count <= splitDrawInfo->u.info_index_fetch.splitCount)
    {
        /* Already lock when collect info.*/
        gcmONERROR(gcoBUFOBJ_FastLock(instantDraw->indexBuffer, gcvNULL, &splitIndexMemory));
        splitIndexMemory =(gctUINT8_PTR)splitIndexMemory + gcmPTR2INT32(instantDraw->indexMemory);
        tmpInstantDraw.count = instantDraw->count;
    }
    else
    {
        gcmONERROR(gcChipCopySpilitIndex(&tmpInstantDraw,
                                         splitDrawInfo,
                                         &splitIndexMemory));
        bAllocate = gcvTRUE;
        tmpInstantDraw.count = splitDrawInfo->u.info_index_fetch.splitCount;
        tmpInstantDraw.primMode = splitDrawInfo->u.info_index_fetch.splitPrimMode;
        tmpInstantDraw.primCount = splitDrawInfo->u.info_index_fetch.splitPrimCount;
    }
    /* set tmpInstantDraw.*/
    tmpInstantDraw.indexMemory = splitIndexMemory;
    tmpInstantDraw.indexBuffer = gcvNULL;

    /* Update index */
    indexInfo.count = tmpInstantDraw.count;
    indexInfo.indexMemory = tmpInstantDraw.indexMemory;
    indexInfo.u.es30.indexBuffer = tmpInstantDraw.indexBuffer;
    gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                        &indexInfo));

    /* Draw */
    gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                             tmpInstantDraw.primMode,
                                             gcvTRUE,
                                             tmpInstantDraw.first,
                                             0,
                                             tmpInstantDraw.primCount,
                                             tmpInstantDraw.count,
                                             gc->vertexArray.instanceCount));

    /* end split draw.*/
    gcmONERROR(gcChipSetVertexArrayBindEnd(gc, instantDraw, gcvTRUE));

OnError:
    if (bAllocate && splitIndexMemory != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, splitIndexMemory);
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDrawWideLine(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                     = gcvSTATUS_OK;
    __GLcontext* gc                      = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw     = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx             = CHIP_CTXINFO(gc);
    gctFLOAT coordPerPixel = gc->state.line.aliasedWidth / (gctFLOAT)(gc->state.viewport.width - gc->state.viewport.x);
    gctFLOAT triPos[] = {-1.5f, -0.4f + coordPerPixel, 0.0f, 1.0f,
                         -1.5f, -0.4f - coordPerPixel, 0.0f, 1.0f,
                         0.1f, 0.5f + coordPerPixel, 0.0f, 1.0f,
                         0.1f, 0.5f - coordPerPixel, 0.0f, 1.0f
    };

    __GLchipInstantDraw tmpInstantDraw;
    gcsVERTEXARRAY_STREAM_INFO streamInfo;
    gcsVERTEXARRAY_INDEX_INFO  indexInfo;
    gctFLOAT preLineWidth = (gctFLOAT)gc->state.line.aliasedWidth;

    gcmHEADER();

    /* Draw the two lines.*/
    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));
    tmpInstantDraw.count = 4;
    tmpInstantDraw.primCount = 2;

    gcmONERROR(gcChipSetVertexArrayBindBegin(gc, &tmpInstantDraw, gcvTRUE));

    /* Collect info for hal level.*/
    gcmES30_COLLECT_STREAM_INFO(streamInfo, (&tmpInstantDraw), gc, chipCtx, gcvTRUE);
    gcmES30_COLLECT_INDEX_INFO(indexInfo, (&tmpInstantDraw));

#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRms,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRmsDirty,
                                         &streamInfo,
                                         &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         &streamInfo,
                                         &indexInfo));
#endif

    gcmONERROR(gcChipSetVertexArrayBindEnd(gc, &tmpInstantDraw, gcvTRUE));

    /* Draw */
    gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                             tmpInstantDraw.primMode,
                                             gcvFALSE,
                                             tmpInstantDraw.first,
                                             0,
                                             tmpInstantDraw.primCount,
                                             tmpInstantDraw.count,
                                             gc->vertexArray.instanceCount));

    /* Draw Last line, and split it into two triangle.*/
    gcmONERROR(gco3D_SetAALineWidth(chipCtx->engine, (GLfloat)1));
    tmpInstantDraw.attributes->pointer = triPos;
    tmpInstantDraw.primMode = gcvPRIMITIVE_TRIANGLE_STRIP;
    tmpInstantDraw.count = 4;
    tmpInstantDraw.primCount = 2;
    tmpInstantDraw.first = 0;

    gcmES30_COLLECT_STREAM_INFO(streamInfo, (&tmpInstantDraw), gc, chipCtx, gcvTRUE);
    gcmES30_COLLECT_INDEX_INFO(indexInfo, (&tmpInstantDraw));

#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRms,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRmsDirty,
                                         &streamInfo,
                                         &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         &streamInfo,
                                         &indexInfo));
#endif

    /* Draw */
    gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                             tmpInstantDraw.primMode,
                                             gcvFALSE,
                                             tmpInstantDraw.first,
                                             0,
                                             tmpInstantDraw.primCount,
                                             tmpInstantDraw.count,
                                             gc->vertexArray.instanceCount));

    /* reset line width */
    gcmONERROR(gco3D_SetAALineWidth(chipCtx->engine, preLineWidth));

OnError:

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipCollectSplitDrawArraysInfo(
    IN __GLcontext*         gc,
    IN __GLchipInstantDraw* instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];

    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
    &&  vsProgram->xfbCount > 0
    &&  gc->vertexArray.instanceCount == 1
    /* If gcvFEATURE_FE_START_VERTEX_SUPPORT not support, VertexId after split draw is not correct. */
    &&  chipCtx->chipFeature.hwFeature.hasFEstartVertex
    && (!chipCtx->chipFeature.hwFeature.hasPEB2BPixelFix || !chipCtx->chipFeature.hwFeature.hasV2MSAACoherencyFix)
    && chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0
    && (
        instantDraw->primMode == gcvPRIMITIVE_POINT_LIST ||
        instantDraw->primMode == gcvPRIMITIVE_LINE_LIST ||
        instantDraw->primMode == gcvPRIMITIVE_TRIANGLE_LIST
        )
       )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_XFB;
        splitDrawInfo->splitDrawFunc = gcChipSplitDrawXFB;
        return gcvSTATUS_OK;
    }

    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
    &&  gc->vertexArray.instanceCount == 1
    &&  (!chipCtx->chipFeature.hwFeature.hasPEB2BPixelFix || !chipCtx->chipFeature.hwFeature.hasV2MSAACoherencyFix)
    &&  chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0
    &&  instantDraw->primMode == gcvPRIMITIVE_LINE_STRIP
    &&  instantDraw->count == 129
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_3;
        splitDrawInfo->splitDrawFunc = gcChipSplitDraw3;
        return gcvSTATUS_OK;
    }

    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
    &&  (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3 && !chipCtx->chipFeature.hwFeature.hasPaLineClipFix)
    &&  gc->vertexArray.instanceCount == 1
    &&  instantDraw->primMode == gcvPRIMITIVE_LINE_LIST
    &&  instantDraw->count == 22
    &&  gc->state.line.requestedWidth == 5
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_4;
        if (chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0)
        {
            splitDrawInfo->splitDrawFunc = gcChipSplitDraw4Instanced;
        }
        else
        {
            splitDrawInfo->splitDrawFunc = gcChipSplitDraw4;
        }
        return gcvSTATUS_OK;
    }

    /* wide line split.*/
    if (vsProgram->progFlags.wideLineFix
    &&  chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0)
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_WIDE_LINE;
        splitDrawInfo->splitDrawFunc = gcChipSplitDrawWideLine;
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_OK;
}

#define SPILIT_INDEX_OFFSET       48
#define SPILIT_INDEX_CHUNCK_BYTE  64

gceSTATUS
gcChipSplitIndexFetch(
    IN __GLchipInstantDraw* instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    gceSTATUS status = gcvSTATUS_TRUE;
    gctUINT32 address = 0, tempAddress;
    gctUINT32 indexSize = 0;
    gctUINT32 spilitIndexMod;
    gctSIZE_T cutCount = 0;

    gcmHEADER();

    gcmASSERT(instantDraw->indexBuffer != gcvNULL);

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    gcmONERROR(gcoBUFOBJ_Lock(instantDraw->indexBuffer, &address, gcvNULL));
    /* Add offset */
    address += gcmPTR2INT32(instantDraw->indexMemory);
    /* Unlock the bufobj buffer. */
    gcmONERROR(gcoBUFOBJ_Unlock(instantDraw->indexBuffer));

    if (instantDraw->primMode == gcvPRIMITIVE_TRIANGLE_LIST)
    {
        cutCount = instantDraw->count % 3;
    }
    else if (instantDraw->primMode == gcvPRIMITIVE_LINE_LIST)
    {
        cutCount = instantDraw->count % 2;
    }

    /* compute the last index address.*/
    tempAddress = address + (gctUINT32)(instantDraw->count-cutCount-1) * indexSize;
    spilitIndexMod = tempAddress % SPILIT_INDEX_CHUNCK_BYTE;

    if (spilitIndexMod >= SPILIT_INDEX_OFFSET)
    {
        gcmFOOTER();
        return gcvSTATUS_FALSE;
    }

    /* Get primMode and split count.*/
    switch (instantDraw->primMode)
    {
        case gcvPRIMITIVE_POINT_LIST:
        case gcvPRIMITIVE_LINE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_STRIP:
        case gcvPRIMITIVE_TRIANGLE_FAN:
            splitDrawInfo->u.info_index_fetch.splitCount = spilitIndexMod /indexSize +1;
            break;
        case gcvPRIMITIVE_LINE_LOOP:
            splitDrawInfo->u.info_index_fetch.splitCount = spilitIndexMod /indexSize +1;
            break;
        case gcvPRIMITIVE_TRIANGLE_LIST:
            splitDrawInfo->u.info_index_fetch.splitCount = ((spilitIndexMod /(indexSize*3))+1)*3 + cutCount;
            break;
        case gcvPRIMITIVE_LINE_LIST:
            splitDrawInfo->u.info_index_fetch.splitCount = ((spilitIndexMod /(indexSize*2))+1)*2 + cutCount;
            break;
        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gcmFOOTER();
    return gcvSTATUS_TRUE;

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitDrawTCS(
    IN gctPOINTER GC,
    IN gctPOINTER InstantDraw,
    IN gctPOINTER SplitDrawInfo
    )
{
    gceSTATUS status                     = gcvSTATUS_OK;
    __GLcontext* gc                      = (__GLcontext*)(GC);
    __GLchipInstantDraw* instantDraw     = (__GLchipInstantDraw*)(InstantDraw);
    __GLchipContext *chipCtx             = CHIP_CTXINFO(gc);
    gcsSPLIT_DRAW_INFO_PTR splitDrawInfo = (gcsSPLIT_DRAW_INFO_PTR)(SplitDrawInfo);
    gctSIZE_T indexSize                  = 0;

    gctUINT copyOffset = 0;
    gctUINT copyLen    = 0;
    gctUINT alignLen   = 64;
    gctUINT i          = 0;
    gctUINT indexBufferSize    = 0;
    gctUINT bytesPerPatch      = 0;
    gctUINT alignBytesPerPatch = 0;

    gcsVERTEXARRAY_STREAM_INFO streamInfo;
    gcsVERTEXARRAY_INDEX_INFO  indexInfo;
    __GLchipInstantDraw tmpInstantDraw;

    gcmHEADER();

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);

    indexBufferSize    = (gctUINT)(indexSize * instantDraw->count);
    bytesPerPatch      = splitDrawInfo->u.info_tcs.indexPerPatch * (gctUINT)indexSize;
    alignBytesPerPatch = bytesPerPatch - (gctUINT)indexSize;
    gcmONERROR(gcChipSetVertexArrayBindBegin(gc, instantDraw, gcvTRUE));

    /* Stream data not change, only need bind once.*/
    /* Collect info for hal level.*/
    gcmES30_COLLECT_STREAM_INFO(streamInfo, instantDraw, gc, chipCtx, gcvTRUE);
    gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw);
    __GL_MEMCOPY(&tmpInstantDraw, instantDraw, sizeof(__GLchipInstantDraw));

#if gcdUSE_WCLIP_PATCH
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRms,
                                         (!chipCtx->wLimitPatch || chipCtx->wLimitSettled) ? gcvNULL : &chipCtx->wLimitRmsDirty,
                                         &streamInfo,
                                         &indexInfo));
#else
    gcmONERROR(gcoVERTEXARRAY_StreamBind(chipCtx->vertexArray,
                                         &streamInfo,
                                         &indexInfo));
#endif

    /****************** draw command ********************************/
    do
    {
        gctBOOL doCopy = gcvFALSE;

        if (copyOffset >= indexBufferSize)
        {
            /* done.*/
            break;
        }

        /* compute copyLen.*/
        for (i = 1; (alignLen * i) < indexBufferSize - copyOffset; ++i)
        {
            /* only one index beyond 64 bytes.*/
            if (((alignLen * i) % bytesPerPatch) == alignBytesPerPatch)
            {
                doCopy = gcvTRUE;
                break;
            }
        }

        if (doCopy)
        {
            copyLen = alignLen * i - alignBytesPerPatch;
        }
        else
        {
            copyLen = indexBufferSize - copyOffset;
        }

        /* No split draw for this time.*/
        if (copyLen == 0)
        {
            continue;
        }

        /* Update index */
        indexInfo.count = copyLen / (gctUINT)indexSize;
        indexInfo.indexMemory = ((gctUINT8_PTR)splitDrawInfo->u.info_tcs.indexPtr + copyOffset);
        indexInfo.u.es30.indexBuffer = gcvNULL;
        gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                            &indexInfo));

        /* compute draw parameter.*/
        tmpInstantDraw.count = indexInfo.count;
        tmpInstantDraw.primCount = indexInfo.count / splitDrawInfo->u.info_tcs.indexPerPatch;

        /* Call the gcoHARDWARE object. */
        gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                 tmpInstantDraw.primMode,
                                                 gcvTRUE,
                                                 tmpInstantDraw.first,
                                                 0,
                                                 tmpInstantDraw.primCount,
                                                 tmpInstantDraw.count,
                                                 gc->vertexArray.instanceCount));

        /* Update offset.*/
        copyOffset += copyLen;
    }
    while(copyOffset < indexBufferSize);

    /* end split draw.*/
    gcmONERROR(gcChipSetVertexArrayBindEnd(gc, instantDraw, gcvTRUE));

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSplitTCS(
    IN __GLchipInstantDraw* instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo)
{
    gceSTATUS   status = gcvSTATUS_FALSE;
    gctUINT32   indexSize = 0;
    gctPOINTER  indexBase = gcvNULL;
    gctUINT32   alignBytes = 64;
    gctUINT32   bytesPerPatch = 0;
    gctUINT32   alignBytesPerPatch = 0;
    gctBOOL     indexLocked = gcvFALSE;
    gctUINT     indexBufferSize = 0;
    gctUINT     i = 0;
    gctUINT     baseOffset = 0;

    gcmHEADER();

    gcmGET_INDEX_SIZE(instantDraw->indexType, indexSize);
    /* Lock the index buffer. */
    if (instantDraw->indexBuffer)
    {
#if gcdSYNC
        gcoBUFOBJ_WaitFence(instantDraw->indexBuffer, gcvFENCE_TYPE_WRITE);
#endif
        gcmONERROR(gcoBUFOBJ_Lock(instantDraw->indexBuffer, gcvNULL, &indexBase));
        indexLocked = gcvTRUE;
    }

    /* Get index ptr.*/
    if (instantDraw->indexBuffer != gcvNULL)
    {
        gctUINT32 tempAddr = 0;
        splitDrawInfo->u.info_tcs.indexPtr = (gctUINT8_PTR)indexBase + gcmPTR2INT32(instantDraw->indexMemory);

        /* The alignment of physical address should be the same as physical address,
        ** So, use logical address to compute address offset when align to 64.*/
        tempAddr = gcmPTR2INT32(splitDrawInfo->u.info_tcs.indexPtr);
        baseOffset = gcmALIGN(tempAddr, alignBytes) - tempAddr;
    }
    else if (instantDraw->indexMemory != gcvNULL)
    {
        /* For index pointer, address always align to 64 after updating to dynamic cache.*/
        splitDrawInfo->u.info_tcs.indexPtr = instantDraw->indexMemory;
        baseOffset = 0;
    }
    else
    {
        /* no need split.*/
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    splitDrawInfo->u.info_tcs.indexPerPatch = (gctUINT)(instantDraw->count / instantDraw->primCount);
    indexBufferSize = indexSize * (gctUINT)instantDraw->count;
    bytesPerPatch = splitDrawInfo->u.info_tcs.indexPerPatch * indexSize;
    alignBytesPerPatch = ((splitDrawInfo->u.info_tcs.indexPerPatch - 1) * indexSize);

    if (alignBytesPerPatch == alignBytes ||
        (baseOffset == 0 && splitDrawInfo->u.info_tcs.indexPerPatch % 2 == 0))
    {
        /* alignBytesPerPatch == alignBytes is corner case, can not handle.*/
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* check need do split.*/
    for (i = 0; (alignBytes * i + baseOffset) < indexBufferSize; ++i)
    {
        /* only one index beyond 64 bytes.*/
        if (((alignBytes * i + baseOffset) % bytesPerPatch) == alignBytesPerPatch)
        {
            gcmFOOTER();
            return gcvSTATUS_OK;
        }
    }

OnError:
    if (indexLocked)
    {
        /* Unlock the index buffer. */
        if (instantDraw->indexBuffer)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Unlock(instantDraw->indexBuffer));
        }
    }

    /* Return the status. */
    gcmFOOTER();
    return gcvSTATUS_FALSE;
}

__GL_INLINE gceSTATUS
gcChipCollectSplitDrawElementInfo(
    IN __GLcontext*         gc,
    IN __GLchipInstantDraw* instantDraw,
    IN OUT gcsSPLIT_DRAW_INFO_PTR splitDrawInfo
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    /* Collect split draw info.*/
    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
    &&  gc->vertexArray.instanceCount == 1
    &&  instantDraw->primMode == gcvPRIMITIVE_LINE_STRIP
    &&  instantDraw->count == 0x81
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_1;
        splitDrawInfo->splitDrawFunc = gcChipSplitDraw1;
        return gcvSTATUS_OK;
    }

    if (!chipCtx->chipFeature.hwFeature.hasPEEnhancement2
    &&  (chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_GTFES30)
    &&  (gc->vertexArray.instanceCount == 1 && instantDraw->primMode == gcvPRIMITIVE_TRIANGLE_LIST &&
         gc->state.enables.stencilTest &&
         (
          gc->state.stencil.back.writeMask != gc->state.stencil.front.writeMask ||
          gc->state.stencil.back.mask != gc->state.stencil.front.mask ||
          gc->state.stencil.back.reference != gc->state.stencil.front.reference
          )
         && chipCtx->directPositionIndex != -1
         )
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_2;
        splitDrawInfo->splitDrawFunc = gcChipSplitDraw2;
        return gcvSTATUS_OK;
    }

    /* Index split draw.*/
    if (!chipCtx->chipFeature.hwFeature.hasIndexFetchFix
    &&  instantDraw->indexBuffer != gcvNULL
    &&  splitDrawInfo->u.info_index_fetch.instanceCount == 1
    &&  (instantDraw->primitiveRestart || gcvSTATUS_TRUE == gcChipSplitIndexFetch(instantDraw, splitDrawInfo))
    )
    {
        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_INDEX_FETCH;
        splitDrawInfo->splitDrawFunc = gcChipSplitDrawIndexFetch;
        return gcvSTATUS_OK;
    }

    /* Tcs split draw.*/
    if (instantDraw->primMode == gcvPRIMITIVE_PATCH_LIST
    &&  instantDraw->first == 0
    &&  !chipCtx->chipFeature.hwFeature.hasPatchListFetchFix
    &&  gcvSTATUS_OK == gcChipSplitTCS(instantDraw, splitDrawInfo)
    )
    {
        /* The two kind split draw should be exclusive.*/
        gcmASSERT(chipCtx->chipFeature.hwFeature.hasIndexFetchFix);

        splitDrawInfo->splitDrawType = gcvSPLIT_DRAW_TCS;
        splitDrawInfo->splitDrawFunc = gcChipSplitDrawTCS;
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_OK;
}

GLboolean
__glChipDrawArraysInstanced(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

 #if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            goto OnError;
        }
    }
#endif

    if ((instantDraw->primMode == gcvPRIMITIVE_LINE_LIST) && (gc->vertexArray.instanceCount > 1))
    {
        instantDraw->count = instantDraw->primCount * 2;
    }

    if (instantDraw->count > 0 && instantDraw->primCount > 0)
    {
        gcsSPLIT_DRAW_INFO splitDrawInfo;

        __GL_MEMZERO(&splitDrawInfo, sizeof(gcsSPLIT_DRAW_INFO));

        /* Collect split draw info.*/
        gcChipCollectSplitDrawArraysInfo(gc, instantDraw, &splitDrawInfo);

        if (splitDrawInfo.splitDrawType != gcvSPLIT_DRAW_UNKNOWN)
        {
            gcmONERROR((*splitDrawInfo.splitDrawFunc)(gc, instantDraw, &splitDrawInfo));
        }
        else
        {
            /* Bind vertex array */
            if (gc->vertexArray.varrayDirty         ||
                 instantDraw->indexBuffer == gcvNULL ||
                 chipCtx->patchId != gcvPATCH_REALRACING)
            {
                gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvTRUE));
            }

            /* Draw */
            gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                     instantDraw->primMode,
                                                     gcvFALSE,
                                                     instantDraw->first,
                                                     0,
                                                     instantDraw->primCount,
                                                     instantDraw->count,
                                                     gc->vertexArray.instanceCount));
        }
    }

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipDrawElementsInstanced(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLuint loop;
    gceSTATUS status = gcvSTATUS_OK;
    gcoBUFOBJ alignedBuffer = gcvNULL;

    gcmHEADER_ARG("gc=0x%x", gc);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            goto OnError;
        }
    }
#endif

    for (loop = 0; loop < chipCtx->indexLoops; ++loop)
    {
        __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[loop];

        /* If the index buffer is not dynamic and instance count > 1
        we need to align it to 16 bytes for iMX6 */
        if ((instantDraw->indexBuffer) &&
            (gc->vertexArray.instanceCount > 1) &&
            (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3))
        {
            /* Align the buffer */
            gcmONERROR(gcoBUFOBJ_AlignIndexBufferWhenNeeded(
                instantDraw->indexBuffer,
                __GL_PTR2UINT(instantDraw->indexMemory),
                &alignedBuffer));

            /* Replace the buffer */
            if (alignedBuffer != gcvNULL)
            {
                instantDraw->indexBuffer = alignedBuffer;
                instantDraw->indexMemory = 0;
            }
        }

#if gcdSYNC
        if (instantDraw->indexBuffer)
        {
            gcmONERROR(gcoBUFOBJ_GetFence(instantDraw->indexBuffer, gcvFENCE_TYPE_READ));
        }
#endif

        if (instantDraw->count > 0 && instantDraw->primCount > 0)
        {
            gcsSPLIT_DRAW_INFO splitDrawInfo;

            __GL_MEMZERO(&splitDrawInfo, sizeof(gcsSPLIT_DRAW_INFO));

            /* Collect split draw info.*/
            splitDrawInfo.u.info_index_fetch.instanceCount = gc->vertexArray.instanceCount;
            gcChipCollectSplitDrawElementInfo(gc, instantDraw, &splitDrawInfo);

            if (splitDrawInfo.splitDrawType != gcvSPLIT_DRAW_UNKNOWN)
            {
                gcmONERROR((*splitDrawInfo.splitDrawFunc)(gc, instantDraw, &splitDrawInfo));
            }
            else
            {
                /* Bind vertex array */
                if (gc->vertexArray.varrayDirty            ||
                    instantDraw->indexBuffer == gcvNULL    ||
                    chipCtx->patchId != gcvPATCH_REALRACING)
                {
                    gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, (__GL_DEFAULT_LOOP == loop), gcvTRUE));
                }
                else
                {
                    gcsVERTEXARRAY_INDEX_INFO indexInfo;

                    gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw);
                    gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                                        &indexInfo));
                }

                /* Draw */
                gcmONERROR(gco3D_DrawInstancedPrimitives(chipCtx->engine,
                                                         instantDraw->primMode,
                                                         gcvTRUE,
                                                         instantDraw->first,
                                                         0,
                                                         instantDraw->primCount,
                                                         instantDraw->count,
                                                         gc->vertexArray.instanceCount));
            }
        }

        /* Destroy temporary buffer */
        if (alignedBuffer != gcvNULL)
        {
            gcmONERROR(gcoBUFOBJ_Destroy(alignedBuffer));
            alignedBuffer = gcvNULL;
        }
    }

    /* Intentional fall through */
OnError:
    /* Destroy temporary buffer if we still couldn't destroyed it */
    if (alignedBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Destroy(alignedBuffer));
        alignedBuffer = gcvNULL;
    }

    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

}

GLboolean
__glChipDrawArrays(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            goto OnError;
        }
    }
#endif

    if (instantDraw->count > 0 && instantDraw->primCount > 0)
    {
        gcsSPLIT_DRAW_INFO splitDrawInfo;

        __GL_MEMZERO(&splitDrawInfo, sizeof(gcsSPLIT_DRAW_INFO));

        /* Collect split draw info.*/
        gcChipCollectSplitDrawArraysInfo(gc, instantDraw, &splitDrawInfo);

        if (splitDrawInfo.splitDrawType != gcvSPLIT_DRAW_UNKNOWN)
        {
            gcmONERROR((*splitDrawInfo.splitDrawFunc)(gc, instantDraw, &splitDrawInfo));
        }
        else
        {
            /* Bind the vertex array to the hardware. */
            if (gc->vertexArray.varrayDirty            ||
                    instantDraw->indexBuffer == gcvNULL ||
                    chipCtx->patchId != gcvPATCH_REALRACING)
            {
                gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvFALSE));
            }

            /* Draw */
            gcmONERROR(gco3D_DrawPrimitives(chipCtx->engine,
                                            instantDraw->primMode,
                                            instantDraw->first,
                                            instantDraw->primCount));
        }
    }

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

}

GLboolean
__glChipDrawElements(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLuint loop;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

#if gcdDEBUG_OPTION && gcdDEBUG_OPTION_NO_GL_DRAWS
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_DEBUG)
        {
            goto OnError;
        }
    }
#endif

    for (loop = 0; loop < chipCtx->indexLoops; ++loop)
    {
        __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[loop];

#if gcdSYNC
        if (instantDraw->indexBuffer)
        {
            gcmONERROR(gcoBUFOBJ_GetFence(instantDraw->indexBuffer, gcvFENCE_TYPE_READ));
        }
#endif

        if (instantDraw->count > 0 && instantDraw->primCount > 0)
        {
            /* Bind vertex array */
            if (gc->vertexArray.varrayDirty            ||
                 instantDraw->indexBuffer == gcvNULL    ||
                 chipCtx->patchId != gcvPATCH_REALRACING)
            {
                gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, (__GL_DEFAULT_LOOP == loop), gcvFALSE));
            }
            else
            {
                gcsVERTEXARRAY_INDEX_INFO indexInfo;

                gcmES30_COLLECT_INDEX_INFO(indexInfo, instantDraw);
                gcmONERROR(gcoVERTEXARRAY_IndexBind(chipCtx->vertexArray,
                                                    &indexInfo));
            }

            /* Draw */
            gcmONERROR(gco3D_DrawIndexedPrimitives(chipCtx->engine,
                                                   instantDraw->primMode,
                                                   0,
                                                   0,
                                                   instantDraw->primCount));
        }
    }


    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipDrawArraysIndirect(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    GLuint offset = __GL_PTR2UINT(gc->vertexArray.indirectOffset);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Bind the vertex array to the hardware. */
    gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvFALSE));

    /* Draw */
    gcmONERROR(gco3D_DrawIndirectPrimitives(chipCtx->engine,
                                            instantDraw->primMode,
                                            gcvFALSE,
                                            offset,
                                            bufInfo->bufObj));

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipDrawElementsIndirect(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    GLuint offset = __GL_PTR2UINT(gc->vertexArray.indirectOffset);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Bind the vertex array to the hardware. */
    gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvFALSE));

    /* Draw */
    gcmONERROR(gco3D_DrawIndirectPrimitives(chipCtx->engine,
                                            instantDraw->primMode,
                                            gcvTRUE,
                                            offset,
                                            bufInfo->bufObj));

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipMultiDrawArraysIndirect(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    GLuint offset = __GL_PTR2UINT(gc->vertexArray.indirectOffset);
    GLuint drawCount = gc->vertexArray.drawcount;
    GLuint stride = gc->vertexArray.stride;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Bind the vertex array to the hardware. */
    gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvFALSE));

    /* Draw */
    gcmONERROR(gco3D_MultiDrawIndirectPrimitives(chipCtx->engine,
                                            instantDraw->primMode,
                                            gcvFALSE,
                                            offset,
                                            drawCount,
                                            stride,
                                            bufInfo->bufObj));

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipMultiDrawElementsIndirect(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
    __GLchipInstantDraw *instantDraw = &chipCtx->instantDraw[__GL_DEFAULT_LOOP];
    GLuint offset = __GL_PTR2UINT(gc->vertexArray.indirectOffset);
    GLuint drawCount = gc->vertexArray.drawcount;
    GLuint stride = gc->vertexArray.stride;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Bind the vertex array to the hardware. */
    gcmONERROR(gcChipSetVertexArrayBind(gc, instantDraw, gcvTRUE, gcvFALSE));

    /* Draw */
    gcmONERROR(gco3D_MultiDrawIndirectPrimitives(chipCtx->engine,
                                            instantDraw->primMode,
                                            gcvTRUE,
                                            offset,
                                            drawCount,
                                            stride,
                                            bufInfo->bufObj));

    /* Intentional fall through */
OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}


GLboolean
__glChipDrawNothing(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    status = gco3D_DrawNullPrimitives(chipCtx->engine);

    gcmFOOTER();

    return (gcmIS_ERROR(status) ? GL_FALSE : GL_TRUE);
}

GLboolean
__glChipFlush(
    __GLcontext *gc,
    GLboolean bInternal
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Freon requires sync to external in Flush api. */
    gcmONERROR(gcChipFboSyncFromShadowFreon(gc, gc->frameBuffer.drawFramebufObj));

    /* Commit command buffer. */
    gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvFALSE));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipFinish(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Sychronization between CPU and GPU, then drain all commands */
    gcmONERROR(gcChipFboSyncFromShadow(gc, gc->frameBuffer.drawFramebufObj));

    gcmONERROR(gcoSURF_Flush(gcvNULL));

    /* Sync to native resources. */
    gc->imports.syncNative();

    /* Commit command buffer. */
    gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

OnError:
    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

__GL_INLINE gceSTATUS
gcChipEvaluateActiveProgramStage(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    __GLSLStage stage;
    __GLprogramObject *progObj;

    /* Clear active stage bits */
    chipCtx->activeStageBits = 0;
    __GL_MEMZERO(chipCtx->activePrograms, __GLSL_STAGE_LAST * sizeof(__GLchipSLProgram*));
    __GL_MEMZERO(gc->shaderProgram.activeProgObjs, __GLSL_STAGE_LAST * sizeof(__GLprogramObject*));
    chipCtx->prePAProgram = gcvNULL;

    switch (gc->shaderProgram.mode)
    {
    case __GLSL_MODE_GRAPHICS:
        for (stage = __GLSL_STAGE_VS; stage <= __GLSL_STAGE_FS; ++stage)
        {
            progObj = __glGetCurrentStageProgram(gc, stage);
            if (progObj)
            {
                gc->shaderProgram.activeProgObjs[stage] = progObj;
                chipCtx->activePrograms[stage] = (__GLchipSLProgram*)progObj->privateData;
                __glBitmaskOR2(&gc->shaderProgram.samplerTexelFetchDirty, &chipCtx->activePrograms[stage]->texelFetchSamplerMask);
                chipCtx->activeStageBits |= chipCtx->activePrograms[stage]->stageBits;
            }
        }

        progObj = __glGetLastNonFragProgram(gc);

        if (progObj)
        {
            chipCtx->prePAProgram = (__GLchipSLProgram*)progObj->privateData;
        }

        break;

    case __GLSL_MODE_COMPUTE:
        {
            progObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_CS);
            if (progObj)
            {
                gc->shaderProgram.activeProgObjs[__GLSL_STAGE_CS] = progObj;
                chipCtx->activePrograms[__GLSL_STAGE_CS] = (__GLchipSLProgram*)progObj->privateData;
                __glBitmaskOR2(&gc->shaderProgram.samplerTexelFetchDirty, &chipCtx->activePrograms[__GLSL_STAGE_CS]->texelFetchSamplerMask);
                chipCtx->activeStageBits |= chipCtx->activePrograms[__GLSL_STAGE_CS]->stageBits;
            }
        }
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    return gcvSTATUS_OK;
}


gceSTATUS
gcChipTraverseProgramStages(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    programStageActionCallBackFunc callback
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLint halStage = -1;
    GLuint stageBits = chipCtx->activeStageBits;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x call=0x%x", gc, chipCtx, callback);

    while (stageBits)
    {
        __GLprogramObject *progObj;
        __GLchipSLProgram *program;
        __GLSLStage stage;
        GLuint stageMask = 1 << ++halStage;

        if (!(stageBits & stageMask))
        {
            continue;
        }
        stage = __glChipHALShaderStageToGL[halStage];
        progObj = gc->shaderProgram.activeProgObjs[stage];
        program = chipCtx->activePrograms[stage];

        gcmONERROR((*callback)(gc, progObj, program, stage));

        /* Clear all stage bits from this program */
        stageBits &= ~(program->stageBits);
    }

OnError:
    gcmFOOTER();
    return status;

}

GLboolean
__glChipDrawBegin(
    __GLcontext* gc,
    GLenum mode
    )
{
    GLboolean ret = GL_FALSE;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x mode=0x%04x", gc, mode);

    do
    {
        __GLchipSLProgram *vsProgram, *fsProgram, *tcsProgram = gcvNULL, *tesProgram = gcvNULL, *gsProgram = gcvNULL;
        /* primitive mode in previous stage */
        GLenum primMode = gc->vertexArray.primMode;

#if gcdFRAMEINFO_STATISTIC
        {
            gctUINT32 frameCount, drawCount;
            static gctUINT32 tgtFrameCount = 0;
            static gctUINT32 tgtDrawCount = 0;
            gcoHAL_FrameInfoOps(chipCtx->hal,
                                    gcvFRAMEINFO_FRAME_NUM,
                                    gcvFRAMEINFO_OP_GET,
                                    &frameCount);

            gcoHAL_FrameInfoOps(chipCtx->hal,
                                    gcvFRAMEINFO_DRAW_NUM,
                                    gcvFRAMEINFO_OP_GET,
                                    &drawCount);

            if (((tgtFrameCount == 0) || (frameCount == tgtFrameCount)) &&
                ((tgtDrawCount == 0) || (drawCount == tgtDrawCount)))
            {
                drawCount = drawCount;
                g_dbgReleasePhony++;
            }

            gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DRAW_NUM,
                                gcvFRAMEINFO_OP_INC,
                                gcvNULL);

        }
#endif

        if (gc->profiler.enable  && gc->profiler.perDrawMode)
        {
            __glChipProfilerSet(gc, GL3_PROFILER_DRAW_BEGIN, 0);
        }

        /* Special patch for fishnoodle.*/
        if (chipCtx->patchId == gcvPATCH_FISHNOODLE &&
            gc->vertexArray.indices == gcvNULL &&
            gc->vertexArray.indexCount == 12 &&
            !(gc->vertexArray.varrayDirty & __GL_DIRTY_VARRAY_ENABLE_BIT) &&
            !chipCtx->patchInfo.bufBindDirty)
        {
            break;
        }

        /* Evaluate program objects */
        if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & (__GL_DIRTY_GLSL_PROGRAM_SWITCH |
                                                        __GL_DIRTY_GLSL_MODE_SWITCH))
        {
            gcChipEvaluateActiveProgramStage(gc, chipCtx);

            if (gc->shaderProgram.currentProgram)
            {
                if (!__glChipValidateProgram(gc, gc->shaderProgram.currentProgram, GL_TRUE))
                {
                    __GLES_PRINT("ES30:skip draw because of program validate failed");
                    break;
                }
            }
            else if (gc->shaderProgram.boundPPO)
            {
                __GLprogramPipelineObject *ppObj = gc->shaderProgram.boundPPO;

                if ((!ppObj->stageProgs[__GLSL_STAGE_VS] && !ppObj->stageProgs[__GLSL_STAGE_FS]) ||
                    !__glChipValidateProgramPipeline(gc, ppObj, GL_TRUE))
                {
                    __GLES_PRINT("ES30:skip draw because of program pipeline validate failed");
                    break;
                }
            }
            else
            {
                __GLES_PRINT("ES30:skip draw because of no program object nor program pipeline object was active");
                break;
            }
        }

        /* update max unit /sampler.*/
        if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & (__GL_DIRTY_GLSL_PROGRAM_SWITCH |
                                                        __GL_DIRTY_GLSL_MODE_SWITCH    |
                                                        __GL_DIRTY_GLSL_UNIFORM))
        {
            __GLSLStage stage;
            __GLprogramObject *progObj;

            for (stage = __GLSL_STAGE_VS; stage <= __GLSL_STAGE_FS; ++stage)
            {
                progObj = __glGetCurrentStageProgram(gc, stage);
                if (progObj)
                {
                    gc->shaderProgram.maxSampler = gcmMAX(gc->shaderProgram.maxSampler, progObj->maxSampler);
                    gc->shaderProgram.maxUnit = gcmMAX(gc->shaderProgram.maxUnit, progObj->maxUnit);
                }
            }
        }

        /* Get state programs first. */
        vsProgram = chipCtx->activePrograms[__GLSL_STAGE_VS];
        fsProgram = chipCtx->activePrograms[__GLSL_STAGE_FS];

        if (__glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled)
        {
            tcsProgram = chipCtx->activePrograms[__GLSL_STAGE_TCS];
            tesProgram = chipCtx->activePrograms[__GLSL_STAGE_TES];
        }

        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            gsProgram = chipCtx->activePrograms[__GLSL_STAGE_GS];
        }

        /*1, VS stage check */
        if (!vsProgram)
        {
            if (tcsProgram || tesProgram)
            {
                __GLES_PRINT("ES30: tessellation is active and vertex shader is missing.");
                __GL_ERROR(GL_INVALID_OPERATION);
            }
            else if (gsProgram)
            {
                __GLES_PRINT("ES30: geometry is active and vertex shader is missing.");
                __GL_ERROR(GL_INVALID_OPERATION);
            }
            else
            {
                __GLES_PRINT("ES30:skip draw because of NULL VS program");
            }
            break;
        }

        if (!vsProgram->masterPgInstance || !vsProgram->masterPgInstance->programState.stateBuffer ||
            !vsProgram->masterPgInstance->programState.stateBufferSize)
        {
            __GLES_PRINT("ES30:skip draw because of invalid VS program instance");
            break;
        }

        /*2, TS stage check */
        if (__glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled)
        {
            if ((tcsProgram == gcvNULL) != (tesProgram == gcvNULL))
            {
                __GL_ERROR(GL_INVALID_OPERATION);
                break;
            }
            else if (tcsProgram)
            {
                if ((GL_PATCHES_EXT != gc->vertexArray.primMode) ||
                    (gcvNULL == vsProgram))
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }
            }
            else if (GL_PATCHES_EXT == gc->vertexArray.primMode)
            {
                __GL_ERROR(GL_INVALID_OPERATION);
                break;
            }

            if (tcsProgram)
            {
                if ((gc->vertexArray.indexCount > 0 &&
                     gc->vertexArray.indexCount < gc->shaderProgram.patchVertices) ||
                    (gc->vertexArray.indexCount == 0 &&
                     (gc->vertexArray.end - gc->vertexArray.start) < gc->shaderProgram.patchVertices)
                    )
                {
                    __GLES_PRINT("ES30: skip draw because of insufficient vertex count for TS");
                    break;
                }

                GL_ASSERT(tesProgram);
                if (!tcsProgram->masterPgInstance || !tcsProgram->masterPgInstance->programState.stateBuffer ||
                    !tcsProgram->masterPgInstance->programState.stateBufferSize ||
                    !tesProgram->masterPgInstance || !tesProgram->masterPgInstance->programState.stateBuffer ||
                    !tesProgram->masterPgInstance->programState.stateBufferSize)
                {
                    __GLES_PRINT("ES30:skip draw because of invalid tcs/tes program instance");
                    break;
                }

                primMode = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_TES]->bindingInfo.tessPrimitiveMode;
            }
        }

        /*3, GS stage check */
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            __GLprogramObject *gsProgObj = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_GS];
            GLboolean legalPrimType = GL_FALSE;

            if (gsProgram)
            {
                GLenum gsInputType = gsProgObj->bindingInfo.gsInputType;

                if (vsProgram == gcvNULL)
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }

                switch (gsInputType)
                {
                case GL_POINTS:
                    if (primMode == GL_POINTS)
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;

                case GL_LINES:
                    if ((primMode == GL_LINES) ||
                        (primMode == GL_LINE_STRIP) ||
                        (primMode == GL_LINE_LOOP))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                case GL_LINES_ADJACENCY_EXT:
                    if ((primMode == GL_LINES_ADJACENCY_EXT) ||
                        (primMode == GL_LINE_STRIP_ADJACENCY_EXT))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                case GL_TRIANGLES:
                    if ((primMode == GL_TRIANGLES) ||
                        (primMode == GL_TRIANGLE_STRIP) ||
                        (primMode == GL_TRIANGLE_FAN))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                case GL_TRIANGLES_ADJACENCY_EXT:
                    if ((primMode == GL_TRIANGLES_ADJACENCY_EXT) ||
                        (primMode == GL_TRIANGLE_STRIP_ADJACENCY_EXT))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                default:
                    GL_ASSERT(0);
                    break;
                }

                if (!legalPrimType)
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }

                primMode = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_GS]->bindingInfo.gsOutputType;

                if (gsProgObj->bindingInfo.gsOutVertices == 0)
                {
                    __GLES_PRINT("ES30: skip draw because of zero vertices output from GS");
                    break;
                }

                if (!gsProgram->masterPgInstance || !gsProgram->masterPgInstance->programState.stateBuffer ||
                    !gsProgram->masterPgInstance->programState.stateBufferSize)
                {
                    __GLES_PRINT("ES30:skip draw because of invalid GS program instance");
                    break;
                }
            }
        }

        /*4, XFB stage check */
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled ||
            __glExtension[__GL_EXTID_EXT_tessellation_shader].bEnabled)
        {
            __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;

            if (xfbObj->active && !xfbObj->paused)
            {
                GLboolean legalPrimType = GL_FALSE;
                switch (xfbObj->primMode)
                {
                case GL_POINTS:
                    legalPrimType = (primMode == GL_POINTS);
                    break;
                case GL_LINES:
                    if ((primMode == GL_LINES) ||
                        (primMode == GL_LINE_LOOP)  ||
                        (primMode == GL_LINE_STRIP) ||
                        (primMode == GL_LINES_ADJACENCY_EXT) ||
                        (primMode == GL_LINE_STRIP_ADJACENCY_EXT))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                case GL_TRIANGLES:
                    if ((primMode == GL_TRIANGLES) ||
                        (primMode == GL_TRIANGLE_FAN) ||
                        (primMode == GL_TRIANGLE_STRIP) ||
                        (primMode == GL_TRIANGLES_ADJACENCY_EXT) ||
                        (primMode == GL_TRIANGLE_STRIP_ADJACENCY_EXT))
                    {
                        legalPrimType = GL_TRUE;
                    }
                    break;
                }

                if (!legalPrimType)
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }
            }
        }

        /*5, FS stage check */
        if (!gc->state.enables.rasterizerDiscard && !fsProgram)
        {
            __GLES_PRINT("ES30:skip draw because of NULL PS program");
            break;
        }

        if (fsProgram &&
            (!fsProgram->masterPgInstance ||
             !fsProgram->masterPgInstance->programState.stateBuffer ||
             !fsProgram->masterPgInstance->programState.stateBufferSize)
           )
        {
            __GLES_PRINT("ES30:skip draw because of invalid PS program instance");
            break;
        }

        /*6, Check valid for KHR_blend_equation_advanced */
        if (__glExtension[__GL_EXTID_KHR_blend_equation_advanced].bEnabled &&
            fsProgram &&
            gc->state.enables.colorBuffer.blend[0])
        {
            gceLAYOUT_QUALIFIER layoutBit = gcChipUtilConvertLayoutQualifier(chipCtx, gc->state.raster.blendEquationRGB[0], NULL);

            if (layoutBit != gcvLAYOUT_QUALIFIER_NONE)
            {
                GLuint i;
                GLenum *pDrawBuffers = gcvNULL;
                GLuint drawbufferCount = 0;
                gceLAYOUT_QUALIFIER qualifier = gcvLAYOUT_QUALIFIER_NONE;

                for (i = 0; i < fsProgram->outCount; i++)
                {
                    qualifier |= fsProgram->outputs[i].layout;
                }

                if (DRAW_FRAMEBUFFER_BINDING_NAME)
                {
                    pDrawBuffers = gc->frameBuffer.drawFramebufObj->drawBuffers;
                }
                else
                {
                    pDrawBuffers = gc->state.raster.drawBuffers;
                }

                for (i = 1; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
                {
                    if (pDrawBuffers[i] != GL_NONE)
                    {
                        drawbufferCount++;
                    }
                }
                /* Advanced blending equations are supported only when rendering
                ** to a single color buffer using fragment color zero
                */
                if (qualifier != gcvLAYOUT_QUALIFIER_NONE && (fsProgram->outCount > 1 || drawbufferCount > 0))
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }

                /* Advanced blending equations require PS output with matching qualifier */
                if (!(qualifier & layoutBit))
                {
                    __GL_ERROR(GL_INVALID_OPERATION);
                    break;
                }
            }
        }


        /*7, If gl_PointSize was not set when draw GL_POINTS, while raster will not be discarded */
        GL_ASSERT(chipCtx->prePAProgram);
        if ((primMode == GL_POINTS) &&
            (!chipCtx->prePAProgram->masterPgInstance->programState.hints->prePaShaderHasPointSize) &&
            (!gc->state.enables.rasterizerDiscard)
           )
        {
            __GLES_PRINT("ES30:draw with undefined point size.");
        }

        /* Trivial rejections based on openGL state. */
        if (gc->state.enables.polygon.cullFace && gc->state.polygon.cullFace == GL_FRONT_AND_BACK &&
            (primMode >= GL_TRIANGLES && primMode <= GL_TRIANGLE_FAN))
        {
            __GLES_PRINT("ES30: skip draw because of double cull");
            break;
        }

        /* If there is no index buffer when calling DrawElements, this draw should break and return immediately*/
        if ((gc->vertexArray.indexCount != 0)
            && (gc->vertexArray.indices == gcvNULL)
            && !__glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX))
        {
            __GLES_PRINT("ES30:skip draw because of invalid ibo");
            break;
        }

        if (chipCtx->needStencilOpt && gc->state.enables.stencilTest)
        {
            __GLchipStencilOpt *stencilOpt = gcChipPatchStencilOptGetInfo(gc, GL_FALSE);

            if (stencilOpt)
            {
                /* If SW can determine the stencil test must fail for all fragments, we can skip the draw. */
                if (gcChipPatchStencilOptTest(gc, stencilOpt))
                {
                    if (gc->state.stencil.front.fail      != GL_KEEP ||
                        gc->state.stencil.back.fail       != GL_KEEP ||
                        gc->state.stencil.front.depthFail != GL_KEEP ||
                        gc->state.stencil.back.depthFail  != GL_KEEP ||
                        gc->state.stencil.front.depthPass != GL_KEEP ||
                        gc->state.stencil.back.depthPass  != GL_KEEP)
                    {
                        gcChipPatchStencilOptReset(stencilOpt, stencilOpt->width, stencilOpt->height, stencilOpt->bpp);
                    }
                }
                else
                {
                    break;
                }
            }
        }

        if (gc->flags & __GL_CONTEXT_SKIP_DRAW_MASK)
        {
            __GLES_PRINT("ES30:skip draw with context flag=0x%x", gc->flags);
            break;
        }
        chipCtx->primitveType = primMode;

        ret = GL_TRUE;
    } while (0);

    if (ret == GL_FALSE && gc->profiler.enable &&  gc->profiler.perDrawMode)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_DRAW_END, 0);
    }
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLboolean
__glChipDrawValidateState(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* For one validation stage, recompile validation must be
    ** put at very begin since recompiler may change anything
    */
    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipValidateRecompileStateCB));

    /* Validate stream */
    gcmONERROR(gcChipValidateStream(gc, chipCtx));

    /* Validate render buffer state*/
    gcmONERROR(gcChipValidateRenderTargetState(gc, chipCtx));

    /* Validate normal context state*/
    gcmONERROR(gcChipValidateState(gc, chipCtx));

    gcmONERROR(gcChipValidatePatchState(gc, chipCtx));

    /* Validate defer or patch dirty which will overwrite normal context state */
    gcmONERROR(gcChipValidateChipDirty(gc, chipCtx));

    /* Validate draw path depending on normal and patch flows */
    gcmONERROR(gcChipValidateDrawPath(gc, chipCtx));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gc->dp.drawPrimitive = __glChipDrawNothing;
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipDrawEnd(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

#if gcdSYNC
    gcePATCH_ID patchId = gcvPATCH_INVALID;
#endif

#if gcdFRAMEINFO_STATISTIC
    gctUINT32 frameCount, drawCount;
#endif

    gcmHEADER_ARG("gc=0x%x", gc);

#if defined(ANDROID) && __GL_CHIP_ENABLE_MEMORY_REDUCTION
    gcmVERIFY_OK(gcChipReduceImageTexture(gc));
#endif

#if gcdSYNC
    patchId = chipCtx->patchId;

    if (patchId == gcvPATCH_MGOHEAVEN2  ||
        patchId == gcvPATCH_SILIBILI    ||
        patchId == gcvPATCH_ELEMENTSDEF ||
        patchId == gcvPATCH_GLOFTKRHM)
    {
        gcmONERROR(gcoHAL_SendFence(gcvNULL));
    }
#endif

    /* Clear the dirty after all dependences was handled */
    chipCtx->chipDirty.uBuffer.bufferDirty = 0;
    chipCtx->chipDirty.uDefer.deferDirty = 0;
    chipCtx->chipDirty.uPatch.patchDirty = 0;

    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipClearUniformDirtyCB));

    {
        gcoSURF surface = gcvNULL;
        gctUINT8 enable;
        /* Now, __SURF_DRAWABLE_UPDATE almost used for system drawable. The optimization
           just for swapming and clearming system drawable. If fbo need the same optimization
           method, need more code to support.*/
        enable = (gctUINT8) gc->state.raster.colorMask[0].redMask
               | ((gctUINT8) gc->state.raster.colorMask[0].greenMask << 1)
               | ((gctUINT8) gc->state.raster.colorMask[0].blueMask  << 2)
               | ((gctUINT8) gc->state.raster.colorMask[0].alphaMask << 3);

        if (chipCtx->drawRtViews[0].surf && enable)
        {
            gcoSURF_SetFlags(chipCtx->drawRtViews[0].surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
        }

        /*. Set the flag of depth.*/
        surface = chipCtx->drawDepthView.surf ? chipCtx->drawDepthView.surf : chipCtx->drawStencilView.surf;

        if (surface != gcvNULL)
        {
            gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE);
        }
    }

    if (gc->profiler.enable && gc->profiler.perDrawMode)
    {
        __glChipProfilerSet(gc, GL3_PROFILER_DRAW_END, 0);
    }

#if gcdFRAMEINFO_STATISTIC
    {
        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_FRAME_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &frameCount);

        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DRAW_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &drawCount);
        /* increased in begin */
        drawCount--;

        if (chipCtx->activePrograms[__GLSL_STAGE_FS] &&
            chipCtx->activePrograms[__GLSL_STAGE_FS]->curPgInstance->programState.hints->fsIsDual16)
        {
            gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DUAL16_NUM,
                                gcvFRAMEINFO_OP_INC,
                                gcvNULL);
        }

        gcmDUMP(gcvNULL, "#[info: fID=%d, dID=%d(draw), pID=%d, ppID=%d]",
                frameCount, drawCount,
                gc->shaderProgram.currentProgram ? gc->shaderProgram.currentProgram->objectInfo.id : 0,
                gc->shaderProgram.currentProgram ? 0 : gc->shaderProgram.boundPPO->name);
    }

    if (g_dbgPerDrawKickOff)
    {
        gcmONERROR(gcoSURF_Flush(gcvNULL));
        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
    }

    if (g_dbgDumpImagePerDraw & (__GL_PERDRAW_DUMP_DRAW_RT | __GL_PERDRAW_DUMP_DRAW_DS))
    {
        gcmONERROR(gcChipUtilsDumpRT(gc, (__GL_PERDRAW_DUMP_DRAW_RT | __GL_PERDRAW_DUMP_DRAW_DS)));
    }
#endif

#if (gcdDUMP && gcdDUMP_VERIFY_PER_DRAW)
    gcmONERROR(gcChipUtilsVerifyRT(gc));
    gcmONERROR(gcChipUtilsVerifyImages(gc));
#endif

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", gcmIS_SUCCESS(status));
    return gcmIS_SUCCESS(status);
}

#define GLES_BLEND_TO_HW_BLEND(function) \
        switch (function)\
        {\
        case GL_ZERO:                    /* glvBLENDZERO */\
            func = gcvBLEND_ZERO;\
            break;\
        case GL_ONE:                     /* glvBLENDONE */\
            func = gcvBLEND_ONE;\
            break;\
        case GL_SRC_COLOR:               /* glvBLENDSRCCOLOR */\
            func = gcvBLEND_SOURCE_COLOR;\
            break;\
        case GL_ONE_MINUS_SRC_COLOR:     /* glvBLENDSRCCOLORINV */\
            func = gcvBLEND_INV_SOURCE_COLOR;\
            break;\
        case GL_SRC_ALPHA:               /* glvBLENDSRCALPHA */\
            func = gcvBLEND_SOURCE_ALPHA;\
            break;\
        case GL_ONE_MINUS_SRC_ALPHA:     /* glvBLENDSRCALPHAINV */\
            func = gcvBLEND_INV_SOURCE_ALPHA;\
            break;\
        case GL_DST_COLOR:               /* glvBLENDDSTCOLOR */\
            func = gcvBLEND_TARGET_COLOR;\
            break;\
        case GL_ONE_MINUS_DST_COLOR:     /* glvBLENDDSTCOLORINV */\
            func = gcvBLEND_INV_TARGET_COLOR;\
            break;\
        case GL_DST_ALPHA:               /* glvBLENDDSTALPHA */\
            func = gcvBLEND_TARGET_ALPHA;\
            break;\
        case GL_ONE_MINUS_DST_ALPHA:     /* glvBLENDDSTALPHAINV */\
            func = gcvBLEND_INV_TARGET_ALPHA;\
            break;\
        case GL_SRC_ALPHA_SATURATE:      /* glvBLENDSRCALPHASATURATE */\
            func = gcvBLEND_SOURCE_ALPHA_SATURATE;\
            break;\
        case GL_CONSTANT_COLOR:          /* gcvBLENDCONSTCOLOR */\
            func = gcvBLEND_CONST_COLOR;\
            break;\
        case GL_ONE_MINUS_CONSTANT_COLOR:/* glvBLENDCONSTCOLORINV */\
            func = gcvBLEND_INV_CONST_COLOR;\
            break;\
        case GL_CONSTANT_ALPHA:          /* glvBLENDCONSTALPHA */\
            func = gcvBLEND_CONST_ALPHA;\
            break;\
        case GL_ONE_MINUS_CONSTANT_ALPHA: /* glvBLENDCONSTALPHAINV */\
            func = gcvBLEND_INV_CONST_ALPHA;\
            break;\
        default:\
            func = gcvBLEND_ZERO;\
        }

#define OES_BLEND_MODE_TO_HW_MODE(m) \
            switch (m)\
            {\
            case GL_FUNC_ADD:\
                mode = gcvBLEND_ADD;\
                break;\
            case GL_FUNC_SUBTRACT:\
                mode = gcvBLEND_SUBTRACT;\
                break;\
            case GL_FUNC_REVERSE_SUBTRACT:\
                mode = gcvBLEND_REVERSE_SUBTRACT;\
                break;\
            case GL_MIN:\
                mode = gcvBLEND_MIN;\
                break;\
            case GL_MAX:\
                mode = gcvBLEND_MAX;\
                break;\
            default:\
                mode = gcvBLEND_ADD;\
            }

GLboolean
__glChipDrawPattern(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    static gceCOMPARE dsTest2HAL[] =
    {
        gcvCOMPARE_NEVER,
        gcvCOMPARE_LESS,
        gcvCOMPARE_EQUAL,
        gcvCOMPARE_LESS_OR_EQUAL,
        gcvCOMPARE_GREATER,
        gcvCOMPARE_NOT_EQUAL,
        gcvCOMPARE_GREATER_OR_EQUAL,
        gcvCOMPARE_ALWAYS,
    };

    static gcsFAST_FLUSH fastFlush;
    gceSTATUS status;
    gctINT i;
    gctFLOAT r,g,b,a;
    gceBLEND_MODE mode;
    gceBLEND_FUNCTION func;
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLbufferObject *idxBufObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
    __GLchipVertexBufferInfo *idxBufInfo = (__GLchipVertexBufferInfo*)idxBufObj->privateData;
    __GLprogramObject *currentProgram = gc->shaderProgram.currentProgram;
    __GLchipSLProgram *program = (__GLchipSLProgram *)currentProgram->privateData;
    GLuint vsInputArrayMask = currentProgram->bindingInfo.vsInputArrayMask;
    __GLbufferObject *bufObj;
    __GLchipVertexBufferInfo *bufInfo;
    GLint arrayIdx = -1;
    GLint attribIdx = 0;
    GLuint bitMask;
    gcsFAST_FLUSH_PTR pFastFlush = &fastFlush;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Fill into uniforms */
    pFastFlush->programValid = program ? gcvTRUE : gcvFALSE;
    if(program == gcvNULL)
    {
        pFastFlush->userDefUniformCount = 0;
        vsInputArrayMask = 0;
    }
    else
    {
        pFastFlush->userDefUniformCount = program->userDefUniformCount;
    }

    for (i = 0; i < program->userDefUniformCount; ++i)
    {
        gctUINT32 columns, rows;
        __GLchipSLUniform *uniform = &program->uniforms[i];

        gcTYPE_GetTypeInfo(uniform->dataType, &columns, &rows, gcvNULL);

        pFastFlush->uniforms[i].dirty = uniform->dirty;
        pFastFlush->uniforms[i].columns = columns;
        pFastFlush->uniforms[i].rows = rows;
        pFastFlush->uniforms[i].matrixStride = columns * 4;
        pFastFlush->uniforms[i].arrayStride = pFastFlush->uniforms[i].matrixStride * rows;
        pFastFlush->uniforms[i].halUniform[0] = uniform->halUniform[0];
        pFastFlush->uniforms[i].halUniform[1] = uniform->halUniform[1];
        pFastFlush->uniforms[i].data = uniform->data;

        if (uniform->halUniform[0])
        {
            pFastFlush->uniforms[i].physicalAddress[0] = uniform->stateAddress[0];
        }

        if (uniform->halUniform[1])
        {
            pFastFlush->uniforms[i].physicalAddress[1] = uniform->stateAddress[1];
        }
    }

    /* Fill into stream */
    pFastFlush->vsInputArrayMask = currentProgram->bindingInfo.vsInputArrayMask;
    pFastFlush->vertexArrayEnable = vertexArrayState->attribEnabled;

    while (vsInputArrayMask)
    {
        __GLchipSLLinkage* attribLinkage = gcvNULL;
        bitMask = (__GL_ONE_32 << ++arrayIdx);
        if (!(vsInputArrayMask & bitMask))
        {
            continue;
        }
        vsInputArrayMask &= ~bitMask;

        for (attribLinkage = program->attribLinkage[arrayIdx]; attribLinkage != gcvNULL; attribLinkage = attribLinkage->next)
        {
            /* Check whether VS required attribute was enabled by apps */
            if (vertexArrayState->attribEnabled & (__GL_ONE_32 << arrayIdx))
            {
                __GLvertexAttrib *pAttrib = &vertexArrayState->attribute[arrayIdx];
                __GLvertexAttribBinding *pAttribBinding = &vertexArrayState->attributeBinding[pAttrib->attribBinding];

                /* For GLES_PATTERN_GFX0 or GLES_PATTERN_GFX1, there is no aliasing attribute. */
                gcmASSERT(!attribLinkage->next);

                bufObj = __glGetCurrentVertexArrayBufObj(gc, pAttrib->attribBinding);

                if (!(bufObj && bufObj->size > 0))
                {
                    return GL_FALSE;
                }

                bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);

                pFastFlush->boundObjInfo[attribIdx] = bufInfo->bufObj;
                pFastFlush->attribute[attribIdx].stride = pAttribBinding->stride;
                pFastFlush->attribute[attribIdx].divisor = pAttribBinding->divisor;
                pFastFlush->attribute[attribIdx].pointer = (gctPOINTER)pAttrib->pointer;
                pFastFlush->attribute[attribIdx].size = pAttrib->size;
                pFastFlush->attributeLinkage[attribIdx] = attribLinkage->attribLocation;
                pFastFlush->attribMask |= (__GL_ONE_32 << attribIdx);
                attribIdx++;
            }
        }
    }

    /* Fill into index */
    pFastFlush->bufObj = idxBufInfo->bufObj;
    pFastFlush->indices = (gctPOINTER)gc->vertexArray.indices;
    pFastFlush->indexFormat = gcvINDEX_16;

    /* Fill into alpha */
    r = gcmCLAMP(gc->state.raster.blendColor.r, 0.0f, 1.0f);
    g = gcmCLAMP(gc->state.raster.blendColor.g, 0.0f, 1.0f);
    b = gcmCLAMP(gc->state.raster.blendColor.b, 0.0f, 1.0f);
    a = gcmCLAMP(gc->state.raster.blendColor.a, 0.0f, 1.0f);

    r = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(r, 0.0f), 1.0f));
    g = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(g, 0.0f), 1.0f));
    b = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(b, 0.0f), 1.0f));
    a = gcoMATH_Float2NormalizedUInt8(gcmMIN(gcmMAX(a, 0.0f), 1.0f));

    pFastFlush->color = ((gctUINT32)a << 24) | ((gctUINT32)r << 16) | ((gctUINT32)g << 8) | (gctUINT32)b;

    pFastFlush->blend = gc->state.enables.colorBuffer.blend[0];

    OES_BLEND_MODE_TO_HW_MODE(gc->state.raster.blendEquationAlpha[0]);
    pFastFlush->modeAlpha = mode;

    OES_BLEND_MODE_TO_HW_MODE(gc->state.raster.blendEquationRGB[0]);
    pFastFlush->modeColor = mode;

    GLES_BLEND_TO_HW_BLEND(gc->state.raster.blendSrcRGB[0]);
    pFastFlush->srcFuncColor = func;
    GLES_BLEND_TO_HW_BLEND(gc->state.raster.blendSrcAlpha[0]);
    pFastFlush->srcFuncAlpha = func;
    GLES_BLEND_TO_HW_BLEND(gc->state.raster.blendDstAlpha[0]);
    pFastFlush->trgFuncAlpha = func;
    GLES_BLEND_TO_HW_BLEND(gc->state.raster.blendDstRGB[0]);
    pFastFlush->trgFuncColor = func;

    /* Fill into depth compare */
    if((gc->state.enables.depthTest && chipCtx->drawDepthView.surf) ||
        (gc->state.enables.stencilTest && chipCtx->drawStencilView.surf))
    {
        pFastFlush->depthMode = gcvDEPTH_Z;
    }
    else
    {
        pFastFlush->depthMode = gcvDEPTH_NONE;
    }
    pFastFlush->depthInfoCompare = dsTest2HAL[gc->state.depth.testFunc - GL_NEVER];
    pFastFlush->compare = chipCtx->drawDepthView.surf ? pFastFlush->depthInfoCompare : gcvCOMPARE_ALWAYS;

    /* Fill into draw info */
    if (gc->vertexArray.indexCount == 0)
    {
        pFastFlush->drawCount = (gc->vertexArray.end - gc->vertexArray.start);
    }
    else
    {
        pFastFlush->drawCount = gc->vertexArray.indexCount;
    }
    pFastFlush->indexCount = gc->vertexArray.indexCount;
    pFastFlush->instanceCount = gc->vertexArray.instanceCount;
    pFastFlush->hasHalti = chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_0;

    gcmONERROR(gco3D_DrawPattern(chipCtx->engine, pFastFlush));

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;

OnError:
    return GL_FALSE;
}

GLboolean
__glChipDispatchNothing(
    __GLcontext *gc
    )
{
    return GL_TRUE;
}

GLboolean
__glChipComputeBegin(
    __GLcontext *gc
    )
{
    GLboolean ret = GL_TRUE;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLprogramObject *progObj = gc->shaderProgram.currentProgram;
    __GLprogramPipelineObject *ppObj = gc->shaderProgram.boundPPO;

    gcmHEADER_ARG("gc=0x%x", gc);

#if gcdFRAMEINFO_STATISTIC
    {
        gctUINT32 frameCount, drawCount;
        static gctUINT32 tgtFrameCount = 0;
        static gctUINT32 tgtDrawCount = 0;
        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_FRAME_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &frameCount);

        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DRAW_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &drawCount);

        if (((tgtFrameCount == 0) || (frameCount == tgtFrameCount)) &&
            ((tgtDrawCount == 0) || (drawCount == tgtDrawCount)))
        {
            drawCount = drawCount;
            g_dbgReleasePhony++;
        }

        if (g_dbgSkipDraw)
        {
            gc->dp.dispatchCompute = __glChipDispatchNothing;
        }
        else
        {
            gc->dp.dispatchCompute = __glChipDispatchCompute;
        }

        gcoHAL_FrameInfoOps(gcvNULL,
                    gcvFRAMEINFO_COMPUTE_NUM,
                    gcvFRAMEINFO_OP_INC,
                    gcvNULL);


        gcoHAL_FrameInfoOps(gcvNULL,
                            gcvFRAMEINFO_DRAW_NUM,
                            gcvFRAMEINFO_OP_INC,
                            gcvNULL);
    }
#endif

    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & (__GL_DIRTY_GLSL_PROGRAM_SWITCH |
                                                    __GL_DIRTY_GLSL_MODE_SWITCH))
    {
        gcChipEvaluateActiveProgramStage(gc, chipCtx);

        if (progObj)
        {
            if ((progObj->bindingInfo.activeShaderID[__GLSL_STAGE_CS] == 0) ||
                (GL_FALSE == __glChipValidateProgram(gc, progObj, GL_TRUE)))
            {
                ret = GL_FALSE;
                __GL_ERROR(GL_INVALID_OPERATION);
            }
        }
        else if (ppObj)
        {
            if ((ppObj->stageProgs[__GLSL_STAGE_CS] == NULL) ||
                (GL_FALSE == __glChipValidateProgramPipeline(gc, ppObj, GL_TRUE)))
            {
                ret = GL_FALSE;
                __GL_ERROR(GL_INVALID_OPERATION);
            }
        }
        else
        {
            ret = GL_FALSE;
            __GL_ERROR(GL_INVALID_OPERATION);
        }
    }

    /* update maxUnit / maxSampler.*/
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & (__GL_DIRTY_GLSL_PROGRAM_SWITCH |
                                                    __GL_DIRTY_GLSL_MODE_SWITCH    |
                                                    __GL_DIRTY_GLSL_UNIFORM))
    {
        progObj = __glGetCurrentStageProgram(gc, __GLSL_STAGE_CS);
        if (progObj)
        {
            gc->shaderProgram.maxSampler = gc->constants.shaderCaps.maxTextureSamplers;
            gc->shaderProgram.maxUnit = gc->constants.shaderCaps.maxCombinedTextureImageUnits;
        }
    }

    gcmFOOTER_NO();
    return ret;
}

GLboolean
__glChipComputeValidateState(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);

    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipValidateRecompileStateCB));

    gcmONERROR(gcChipValidateTexture(gc, chipCtx));

    gcmONERROR(gcChipValidateImage(gc, chipCtx));

    gcmONERROR(gcChipValidateShader(gc, chipCtx));

    if (chipCtx->chipDirty.uDefer.sDefer.csReload)
    {
        __GLchipSLProgram *program = chipCtx->activePrograms[__GLSL_STAGE_CS];
        if (program)
        {
            __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
            gcmONERROR(gco3D_LoadProgram(chipCtx->engine,
                                         pgInstance->programState.hints->stageBits,
                                         &pgInstance->programState));
            chipCtx->activeProgState = &pgInstance->programState;
        }
        else
        {
            gcmONERROR(gco3D_LoadProgram(chipCtx->engine,
                                         gcvPROGRAM_STAGE_COMPUTE_BIT,
                                         gcvNULL));

            chipCtx->activeProgState = gcvNULL;
        }
    }
    /* Flush shader resource after program is loaded to under layer */
    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipFlushGLSLResourcesCB));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipComputeEnd(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipClearUniformDirtyCB));

#if gcdFRAMEINFO_STATISTIC
    {
        gctUINT32 frameCount, drawCount;

        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_FRAME_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &frameCount);

        gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DRAW_NUM,
                                gcvFRAMEINFO_OP_GET,
                                &drawCount);
        /* increased in begin */
        drawCount--;

        GL_ASSERT(gc->shaderProgram.activeProgObjs[__GLSL_STAGE_CS]);
        GL_ASSERT(chipCtx->activePrograms[__GLSL_STAGE_CS]);

        gcmDUMP(gcvNULL, "#[info: fID=%d, dID=%d(compute) pID=%d]",
                frameCount, drawCount,
                gc->shaderProgram.activeProgObjs[__GLSL_STAGE_CS]->objectInfo.id);

        if (chipCtx->activePrograms[__GLSL_STAGE_CS]->curPgInstance->programState.hints->fsIsDual16)
        {
            gcoHAL_FrameInfoOps(chipCtx->hal,
                                gcvFRAMEINFO_DUAL16_NUM,
                                gcvFRAMEINFO_OP_INC,
                                gcvNULL);
        }

        if (g_dbgPerDrawKickOff)
        {
            /* Flush the cache. */
            gcmONERROR(gcoSURF_Flush(gcvNULL));

            /* Commit command buffer. */
            gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
        }

        if (g_dbgDumpImagePerDraw & __GL_PERDRAW_DUMP_DRAW_RT)
        {
            __GLchipSLProgram *program = chipCtx->activePrograms[__GLSL_STAGE_CS];
            __GLchipSLProgramInstance *pInstance;
            static char *txTypeStr[] = { "2D",
                                         "3D",
                                         "CUBE",
                                         "2D_A",
                                         "EXT",
                                         "2DMS",
                                         "2DMS_A",
                                         "CUBE_A"};
            GL_ASSERT(program);
            pInstance = program->curPgInstance;

            if (pInstance->extraImageUniformCount + program->imageUniformCount)
            {
                GLuint unit = 0;

                /* Flush the cache. */
                gcmONERROR(gcoSURF_Flush(gcvNULL));

                /* Commit command buffer. */
                gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

                while (unit < gc->constants.shaderCaps.maxImageUnit)
                {
                    __GLchipImageUnit2Uniform *pImageUnit2Uniform, *pExtraImageUnit2Uniform;
                    __GLimageUnitState *imageUnit;

                    pImageUnit2Uniform = &program->imageUnit2Uniform[unit];
                    pExtraImageUnit2Uniform = &pInstance->extraImageUnit2Uniform[unit];
                    imageUnit = &gc->state.image.imageUnit[unit];

                    if (pImageUnit2Uniform->numUniform + pExtraImageUnit2Uniform->numUniform > 0)
                    {
                        gcsSURF_VIEW texView;
                        __GLtextureObject *texObj = imageUnit->texObj;
                        GLboolean layered = ((imageUnit->type != __GL_IMAGE_2D) && !imageUnit->singleLayered);
                        gctSTRING fileName;
                        /* Build file name.*/
                        gctUINT fileNameOffset = 0;

                        texView = gcChipGetTextureSurface(chipCtx, texObj, layered, imageUnit->level, imageUnit->actualLayer);

                        if ((imageUnit->type != __GL_IMAGE_2D) && !imageUnit->singleLayered)
                        {
                            texView.firstSlice = 0;
                            layered = GL_TRUE;
                        }

                        if (imageUnit->singleLayered)
                        {
                            texView.numSlices = 1;
                        }
                        /* Allocate memory for output file name string. */
                        gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, __GLES_MAX_FILENAME_LEN, (gctPOINTER *) &fileName));

                        gcmVERIFY_OK(gcoOS_PrintStrSafe(fileName,
                                                        __GLES_MAX_FILENAME_LEN,
                                                        &fileNameOffset,
                                                        "fID%04d_dID%04d(compute)_pID%04d_imageUnit%02d(tex[%s]ID%04d_level%02d_layer_%02d_layered=%d)",
                                                        frameCount,
                                                        drawCount,
                                                        gc->shaderProgram.activeProgObjs[__GLSL_STAGE_CS]->objectInfo.id,
                                                        unit,
                                                        txTypeStr[texObj->targetIndex],
                                                        texObj->name,
                                                        imageUnit->level,
                                                        imageUnit->actualLayer,
                                                        layered));

                        gcmONERROR(gcChipUtilsDumpSurface(gc, &texView, fileName, gcvFALSE, (g_dbgDumpImagePerDraw >> 16)));

                        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, fileName));
                    }
                    unit++;
                }
            }

        }
    }

#endif

#if (gcdDUMP && gcdDUMP_VERIFY_PER_DRAW)
    gcmONERROR(gcChipUtilsVerifyImages(gc));
#endif

OnError:
    return;
}


gceSTATUS
gcChipLockOutComputeIndirectBuf(
    __GLcontext *gc
    )
{
    gctUINT8_PTR baseAddr = gcvNULL;
    __GLcomputeIndirectCmd *dispatchCmd = gcvNULL;
    __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DISPATCH_INDIRECT_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    if (!bufInfo->bufObj)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

#if gcdSYNC
    gcmONERROR(gcoBUFOBJ_WaitFence(bufInfo->bufObj, gcvFENCE_TYPE_WRITE));
#endif
    gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&baseAddr));
    gcmONERROR(gcoBUFOBJ_CPUCacheOperation_Range(bufInfo->bufObj,
                                                 (gctSIZE_T)gc->compute.offset,
                                                 gcmSIZEOF(__GLcomputeIndirectCmd),
                                                 gcvCACHE_INVALIDATE));

    dispatchCmd = (__GLcomputeIndirectCmd*)(baseAddr + gc->compute.offset);

    gc->compute.num_groups_x = dispatchCmd->num_groups_x;
    gc->compute.num_groups_y = dispatchCmd->num_groups_y;
    gc->compute.num_groups_z = dispatchCmd->num_groups_z;

OnError:
    if (baseAddr)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(bufInfo->bufObj));
    }
    return status;
}


GLboolean
__glChipDispatchCompute(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    do {
        gcsTHREAD_WALKER_INFO info;
        __GLprogramObject *programObject = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_CS];
        __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

        gcoOS_ZeroMemory(&info, gcmSIZEOF(gcsTHREAD_WALKER_INFO));

        info.dimensions = 3;
        info.workGroupSizeX = programObject->bindingInfo.workGroupSize[0];
        info.workGroupSizeY = programObject->bindingInfo.workGroupSize[1];
        info.workGroupSizeZ = programObject->bindingInfo.workGroupSize[2];
        /* Skip the dispatch if any dimension size is 0 */
        if (info.workGroupSizeX == 0 || info.workGroupSizeY == 0 || info.workGroupSizeZ == 0)
        {
            break;
        }

        if (gc->compute.indirect &&
            !chipCtx->chipFeature.hwFeature.hasComputeIndirect)
        {
            gcmERR_BREAK(gcChipLockOutComputeIndirectBuf(gc));
            gc->compute.indirect = gcvFALSE;
        }

        if (gc->compute.indirect)
        {
            gctUINT32 baseAddress = 0;
            __GLbufferObject *indirectObj = gc->bufferObject.generalBindingPoint[__GL_DISPATCH_INDIRECT_BUFFER_INDEX].boundBufObj;
            bufInfo = (__GLchipVertexBufferInfo*)(indirectObj->privateData);
            gcmERR_BREAK(gcoBUFOBJ_Lock(bufInfo->bufObj, &baseAddress, gcvNULL));
            info.indirect = gcvTRUE;
            info.baseAddress = baseAddress + (gctUINT32)gc->compute.offset;
            info.groupNumberUniformIdx = program->curPgInstance->groupNumUniformIdx;
        }
        else
        {
            info.indirect = gcvFALSE;
            info.baseAddress = 0xdeadbeaf;
            info.workGroupCountX = gc->compute.num_groups_x;
            info.workGroupCountY = gc->compute.num_groups_y;
            info.workGroupCountZ = gc->compute.num_groups_z;

            /* Skip the dispatch if any dimension size is 0 */
            if (info.workGroupCountX == 0 || info.workGroupCountY == 0 || info.workGroupCountZ == 0)
            {
                break;
            }
        }
        info.barrierUsed = program->curPgInstance->programState.hints->threadGroupSync;
        gcmERR_BREAK(gco3D_InvokeThreadWalker(chipCtx->engine, &info));
    } while (GL_FALSE);

    if (bufInfo)
    {
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(bufInfo->bufObj));
        bufInfo = gcvNULL;
    }

    if (gcmIS_SUCCESS(status))
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
    else
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
}





