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


#include "gc_vsc.h"

VSC_ErrCode vscSV_Initialize(VSC_STATE_VECTOR* pSV, VSC_MM* pMM, gctINT svSize, gctUINT stateCount)
{
    gctINT bvIdx;

    if (pMM == gcvNULL && svSize <= 0)
    {
        memset(pSV, 0, sizeof(VSC_STATE_VECTOR));
        return VSC_ERR_NONE;
    }

    /* Force to safe sv size */
    if (svSize <= 0)
    {
        svSize = 1;
    }

    /* Force to safe state size */
    if (stateCount < 1)
    {
        stateCount = 2;
    }

    pSV->pBVs = gcvNULL;
    pSV->stateCount = stateCount;
    pSV->svSize = svSize;
    pSV->pMM = pMM;
    pSV->bvCount = vscFindLeastSigBit(vscAlignToPow2(stateCount, MAX_EFFECT_POW2_EXPONENT));
    if (pSV->bvCount)
    {
        pSV->pBVs = (VSC_BIT_VECTOR*)vscMM_Alloc(pMM, pSV->bvCount * sizeof(VSC_BIT_VECTOR));

        if (pSV->pBVs == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
            return VSC_ERR_OUT_OF_MEMORY;
        }
        else
        {
            memset(pSV->pBVs, 0, pSV->bvCount * sizeof(VSC_BIT_VECTOR));
        }

        for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
        {
            vscBV_Initialize(&pSV->pBVs[bvIdx], pMM, svSize);
        }
    }
    return VSC_ERR_NONE;
}

VSC_STATE_VECTOR* vscSV_Create(VSC_MM* pMM, gctINT svSize, gctUINT stateCount)
{
    VSC_STATE_VECTOR*   pSV = gcvNULL;

    pSV = (VSC_STATE_VECTOR*)vscMM_Alloc(pMM, sizeof(VSC_STATE_VECTOR));
    if (!pSV)
        return gcvNULL;

    if (vscSV_Initialize(pSV, pMM, svSize, stateCount) != VSC_ERR_NONE)
        return gcvNULL;

    return pSV;
}

VSC_ErrCode vscSV_Resize(VSC_STATE_VECTOR *pSV, gctINT newSvSize, gctBOOL bKeep)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctINT bvIdx;

    gcmASSERT(SV_IS_VALID(pSV));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        errCode = vscBV_Resize(&pSV->pBVs[bvIdx], newSvSize, bKeep);
        if(errCode != VSC_ERR_NONE)
            return errCode;
    }
    return errCode;
}

void vscSV_Finalize(VSC_STATE_VECTOR* pSV)
{
    gctINT bvIdx;

    if (pSV->pMM)
    {
        for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
        {
            vscBV_Finalize(&pSV->pBVs[bvIdx]);
        }
    }

    vscMM_Free(pSV->pMM, pSV->pBVs);

    pSV->pBVs = gcvNULL;
    pSV->bvCount = 0;
    pSV->stateCount = 0;
}

void vscSV_Reset(VSC_STATE_VECTOR* pSV)
{
    gctINT bvIdx;

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        vscBV_Reset(&pSV->pBVs[bvIdx]);
    }
}

void vscSV_Destroy(VSC_STATE_VECTOR* pSV)
{
    if (pSV)
    {
        vscSV_Finalize(pSV);
        vscMM_Free(pSV->pMM, pSV);
        pSV = gcvNULL;
    }
}

void vscSV_Set(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state)
{
    gctINT bvIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_ORDINAL(pSV, ordinal));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        if (VSC_UTILS_TST_BIT(state, bvIdx))
        {
            vscBV_SetBit(&pSV->pBVs[bvIdx], ordinal);
        }
        else
        {
            vscBV_ClearBit(&pSV->pBVs[bvIdx], ordinal);
        }
    }
}

gctUINT vscSV_Get(VSC_STATE_VECTOR* pSV, gctINT ordinal)
{
    gctINT  bvIdx;
    gctUINT stateInSV = 0;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_ORDINAL(pSV, ordinal));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        if (vscBV_TestBit(&pSV->pBVs[bvIdx], ordinal))
        {
            VSC_UTILS_SET_BIT(stateInSV, bvIdx);
        }
        else
        {
            VSC_UTILS_CLR_BIT(stateInSV, bvIdx);
        }
    }

    return stateInSV;
}

gctBOOL vscSV_Test(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state)
{
    gctINT  bvIdx;
    gctBOOL bTestRes = gcvTRUE;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_ORDINAL(pSV, ordinal));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        bTestRes &= ((vscBV_TestBit(&pSV->pBVs[bvIdx], ordinal) ==
                      (VSC_UTILS_TST_BIT(state, bvIdx) != 0)) ? gcvTRUE : gcvFALSE);
    }

    return bTestRes;
}

void vscSV_SetAll(VSC_STATE_VECTOR* pSV, gctUINT state)
{
    gctINT bvIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        if (VSC_UTILS_TST_BIT(state, bvIdx))
        {
            vscBV_SetAll(&pSV->pBVs[bvIdx]);
        }
        else
        {
            vscBV_ClearAll(&pSV->pBVs[bvIdx]);
        }
    }
}

void vscSV_SetResidual(VSC_STATE_VECTOR* pSV, gctUINT state)
{
    gctINT bvIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        if (VSC_UTILS_TST_BIT(state, bvIdx))
        {
            vscBV_SetResidual(&pSV->pBVs[bvIdx]);
        }
        else
        {
            vscBV_ClearResidual(&pSV->pBVs[bvIdx]);
        }
    }
}

gctBOOL vscSV_TestAndSet(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state)
{
    gctBOOL bTestRes;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_ORDINAL(pSV, ordinal));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    bTestRes = vscSV_Test(pSV, ordinal, state);

    if (!bTestRes)
    {
        vscSV_Set(pSV, ordinal, state);
    }

    return bTestRes;
}

void vscSV_SetInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state)
{
    gctINT bvIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));
    gcmASSERT(SV_IS_LEGAL_ORDINAL_RANGE(pSV, startOrdinal, szRange));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        if (VSC_UTILS_TST_BIT(state, bvIdx))
        {
            vscBV_SetInRange(&pSV->pBVs[bvIdx], startOrdinal, szRange);
        }
        else
        {
            vscBV_ClearInRange(&pSV->pBVs[bvIdx], startOrdinal, szRange);
        }
    }
}

gctBOOL vscSV_TestInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state)
{
    gctINT  bvIdx;
    gctBOOL bTestRes = gcvTRUE;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));
    gcmASSERT(SV_IS_LEGAL_ORDINAL_RANGE(pSV, startOrdinal, szRange));

    for (bvIdx = 0; bvIdx < pSV->bvCount; bvIdx ++)
    {
        bTestRes &= ((vscBV_TestInRange(&pSV->pBVs[bvIdx], startOrdinal, szRange) ==
                      (VSC_UTILS_TST_BIT(state, bvIdx) != 0)) ? gcvTRUE : gcvFALSE);
    }

    return bTestRes;
}

gctBOOL vscSV_TestAndSetInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state)
{
    gctBOOL bTestRes;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));
    gcmASSERT(SV_IS_LEGAL_ORDINAL_RANGE(pSV, startOrdinal, szRange));

    bTestRes = vscSV_TestInRange(pSV, startOrdinal, szRange, state);

    if (!bTestRes)
    {
        vscSV_SetInRange(pSV, startOrdinal, szRange, state);
    }

    return bTestRes;
}

gctINT vscSV_FindStateForward(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctUINT state)
{
    gctINT  stateIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (stateIdx = startOrdinal; stateIdx < pSV->svSize; stateIdx ++)
    {
        if (vscSV_Get(pSV, stateIdx) == state)
        {
            return stateIdx;
        }
    }

    return INVALID_STATE_LOC;
}

gctBOOL vscSV_Any(VSC_STATE_VECTOR* pSV, gctUINT state)
{
    gctINT  stateIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (stateIdx = 0; stateIdx < pSV->svSize; stateIdx ++)
    {
        if (vscSV_Get(pSV, stateIdx) == state)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

gctBOOL vscSV_All(VSC_STATE_VECTOR* pSV, gctUINT state)
{
    gctINT  stateIdx;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (stateIdx = 0; stateIdx < pSV->svSize; stateIdx ++)
    {
        if (vscSV_Get(pSV, stateIdx) != state)
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

gctBOOL vscSV_Equal(VSC_STATE_VECTOR* pSV1, VSC_STATE_VECTOR* pSV2)
{
    gctINT  bvIdx;
    gctBOOL bTestRes = gcvTRUE;

    gcmASSERT(SV_IS_VALID(pSV1));
    gcmASSERT(SV_IS_VALID(pSV2));
    gcmASSERT(pSV2->bvCount == pSV1->bvCount);
    gcmASSERT(pSV2->svSize == pSV1->svSize);
    gcmASSERT(pSV2->stateCount == pSV1->stateCount);

    for (bvIdx = 0; bvIdx < pSV1->bvCount; bvIdx ++)
    {
        bTestRes &= vscBV_Equal(&pSV1->pBVs[bvIdx], &pSV2->pBVs[bvIdx]);
    }

    return bTestRes;
}

gctUINT vscSV_CountStateCount(VSC_STATE_VECTOR* pSV, gctUINT state)
{
    gctINT  stateIdx, stateCount = 0;

    gcmASSERT(SV_IS_VALID(pSV));
    gcmASSERT(SV_IS_LEGAL_STATE(pSV, state));

    for (stateIdx = 0; stateIdx < pSV->svSize; stateIdx ++)
    {
        if (vscSV_Get(pSV, stateIdx) == state)
        {
            stateCount ++;
        }
    }

    return stateCount;
}

void vscSV_Copy(VSC_STATE_VECTOR* pDst, VSC_STATE_VECTOR* pSrc)
{
    gctINT  bvIdx;

    gcmASSERT(SV_IS_VALID(pDst));
    gcmASSERT(SV_IS_VALID(pSrc));
    gcmASSERT(pDst->stateCount == pSrc->stateCount);
    gcmASSERT(pDst->bvCount == pSrc->bvCount);
    gcmASSERT(pDst->svSize >= pSrc->svSize);

    for (bvIdx = 0; bvIdx < pSrc->bvCount; bvIdx ++)
    {
        vscBV_Copy(&pDst->pBVs[bvIdx], &pSrc->pBVs[bvIdx]);
    }
}


