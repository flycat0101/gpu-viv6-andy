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


#include "gc_vsc.h"

#define VSC_DEBUG_MEM 0

void vscBV_Initialize(VSC_BIT_VECTOR* pBV, VSC_MM* pMM, gctINT bvSize)
{
    if (pMM == gcvNULL && bvSize <= 0)
    {
        memset(pBV, 0, sizeof(VSC_BIT_VECTOR));
        return;
    }

    /* Force to safe size */
    if (bvSize <= 0)
    {
        bvSize = 1;
    }

    pBV->pBits = gcvNULL;
    pBV->bitCount = bvSize;
    pBV->pMM = pMM;
    pBV->numOfUINT = BV_SIZE_2_UINT_COUNT(bvSize);
    if (pBV->numOfUINT)
    {
        pBV->pBits = (gctUINT*)vscMM_Alloc(pMM, pBV->numOfUINT * sizeof(gctUINT));

        if (pBV->pBits == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
            return;
        }
        else
        {
            memset(pBV->pBits, CLR_VALUE, pBV->numOfUINT * sizeof(gctUINT));
#if VSC_DEBUG_MEM
            if (pMM->mmType != VSC_MM_TYPE_BMS)
            {
                gcoOS_Print("vscBV_Initialize(pBV->pBits:0x%X, pMM:0x%X, bvSize:%d)\n", pBV->pBits, pMM, bvSize);
            }
#endif
        }
    }
}

VSC_BIT_VECTOR* vscBV_Create(VSC_MM* pMM, gctINT bvSize)
{
    VSC_BIT_VECTOR*   pBV = gcvNULL;

    pBV = (VSC_BIT_VECTOR*)vscMM_Alloc(pMM, sizeof(VSC_BIT_VECTOR));

    vscBV_Initialize(pBV, pMM, bvSize);

    return pBV;
}

static gcmINLINE void _SetResidualVB(VSC_BIT_VECTOR *pBV)
{
    gctINT          minNumOfUINT, lastBitLocalLoc, i;

    lastBitLocalLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1);
    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV);

    BV_SET(pBV, minNumOfUINT - 1, ~VSC_UTILS_MAKE_HIGH_MASK(lastBitLocalLoc));
    for (i = minNumOfUINT; i < pBV->numOfUINT; i ++)
    {
        pBV->pBits[i] = SET_VALUE;
    }
}

static gcmINLINE void _ClearResidualVB(VSC_BIT_VECTOR *pBV)
{
    gctINT          minNumOfUINT, lastBitLocalLoc, i;

    lastBitLocalLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1);
    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV);

    BV_CLR(pBV, minNumOfUINT - 1, ~VSC_UTILS_MAKE_HIGH_MASK(lastBitLocalLoc));
    for (i = minNumOfUINT; i < pBV->numOfUINT; i ++)
    {
        pBV->pBits[i] = CLR_VALUE;
    }
}

void vscBV_Resize(VSC_BIT_VECTOR *pBV, gctINT newBVSize, gctBOOL bKeep)
{
    VSC_BIT_VECTOR  orgBV;
    gctINT          newNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pBV));

    /* Force to safe size */
    if (newBVSize <= 0)
    {
        newBVSize = 1;
    }

    orgBV = *pBV;

    /* If original UINTs can not hold new BV, allocate a new one */
    newNumOfUINT = BV_SIZE_2_UINT_COUNT(newBVSize);
    if (newNumOfUINT > orgBV.numOfUINT)
    {
        pBV->pBits = (gctUINT*)vscMM_Alloc(pBV->pMM, newNumOfUINT*sizeof(gctUINT));
        pBV->numOfUINT = newNumOfUINT;
    }

    /* Resize true bit count to new one */
    pBV->bitCount = newBVSize;

    /* Do we want to keep original bit data? */
    if (bKeep)
    {
        /* If size of new BV is larger than original one, then we are going to grow the BV, so all unused bits
           in original BV must be cleared */
        if (pBV->bitCount > orgBV.bitCount)
        {
            _ClearResidualVB(&orgBV);
        }

        /* A new BV bits has been allocated, so we need copy original BV to new BV */
        if (pBV->pBits != orgBV.pBits)
        {
            memset(pBV->pBits, CLR_VALUE, pBV->numOfUINT*sizeof(gctUINT));

            for (i = 0; i < orgBV.numOfUINT; i ++)
            {
                pBV->pBits[i] = orgBV.pBits[i];
            }
        }
    }
    else
    {
        memset(pBV->pBits, CLR_VALUE, pBV->numOfUINT*sizeof(gctUINT));
    }

    if (pBV->pBits != orgBV.pBits)
    {
        vscMM_Free(pBV->pMM, orgBV.pBits);
    }
}

void vscBV_Finalize(VSC_BIT_VECTOR* pBV)
{
#if VSC_DEBUG_MEM
    if (pBV->pMM == gcvNULL || pBV->pMM->mmType != VSC_MM_TYPE_BMS)
    {
        gcoOS_Print("vscBV_Finalize(pBV->pBits:0x%X, pMM:0x%X, bvSize:%d)\n", pBV->pBits, pBV->pMM, pBV->bitCount);
    }
#endif
    if (pBV->pMM)
    {
        vscMM_Free(pBV->pMM, pBV->pBits);
    }

    pBV->pBits = gcvNULL;
    pBV->bitCount = 0;
    pBV->numOfUINT = 0;
}

void vscBV_Reset(VSC_BIT_VECTOR* pBV)
{
    memset(pBV->pBits, CLR_VALUE, pBV->numOfUINT*sizeof(gctUINT));
}

void vscBV_Destroy(VSC_BIT_VECTOR* pBV)
{
    if (pBV)
    {
        vscBV_Finalize(pBV);
        vscMM_Free(pBV->pMM, pBV);
        pBV = gcvNULL;
    }
}

void PrintBitVector(VSC_BIT_VECTOR* pBV)
{

}

void PrintBitVectorBits(VSC_BIT_VECTOR* pBV)
{

}

/***************************************************************************************************

NOTE ALL FOLLOWING CODE BELOW MUST BE AS FAST AS WE CAN BECAUSE BV IS CRITICAL FOR ANALYSIS AND OPT

***************************************************************************************************/

void vscBV_SetAll(VSC_BIT_VECTOR* pBV)
{
    gcmASSERT(BV_IS_VALID(pBV));

    memset(pBV->pBits, SET_VALUE, pBV->numOfUINT*sizeof(gctUINT));
}

void vscBV_ClearAll(VSC_BIT_VECTOR* pBV)
{
    gcmASSERT(BV_IS_VALID(pBV));

    memset(pBV->pBits, CLR_VALUE, pBV->numOfUINT*sizeof(gctUINT));
}

void vscBV_SetResidual(VSC_BIT_VECTOR* pBV)
{
    gcmASSERT(BV_IS_VALID(pBV));

    _SetResidualVB(pBV);
}

void vscBV_ClearResidual(VSC_BIT_VECTOR* pBV)
{
    gcmASSERT(BV_IS_VALID(pBV));

    _ClearResidualVB(pBV);
}

gctBOOL vscBV_TestAndSetBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal));

    uintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal);
    localBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal);

    /* Check whether the bit was already set, if yes, just return */
    if (BV_BIT_TST(pBV, uintIndex, localBitLoc))
    {
        return gcvTRUE;
    }

    /* If no, set it now */
    BV_BIT_SET(pBV, uintIndex, localBitLoc);
    return gcvFALSE;
}

gctBOOL vscBV_TestAndClearBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal)
{
    gctINT  uintIndex, localBitLoc;
    gctBOOL bSetBefore = gcvFALSE;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal));

    uintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal);
    localBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal);

    /* Check whether the bit was already set, if yes, clear it */
    if (BV_BIT_TST(pBV, uintIndex, localBitLoc))
    {
        BV_BIT_CLR(pBV, uintIndex, localBitLoc);
        bSetBefore = gcvTRUE;
    }

    return bSetBefore;
}

void vscBV_SetInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i;
    gctUINT startMask, endMask;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange));

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick set for range size 1 */
    if (szRange == 1)
    {
        BV_BIT_SET(pBV, startUintIndex, startLocalBitLoc);
        return;
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    /* Mask in startUintIndex and endUintIndex for start and end ordinal respectively */
    startMask = VSC_UTILS_MAKE_BIT_MASK(startLocalBitLoc);
    startMask = startMask | (startMask - 1);
    endMask = VSC_UTILS_MAKE_HIGH_MASK(endLocalBitLoc);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        BV_SET(pBV, startUintIndex, (startMask & endMask));
    }
    else
    {
        /* Start and end ordinary are located in different UINT */

        BV_SET(pBV, startUintIndex, startMask);

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            pBV->pBits[i] = SET_VALUE;
        }

        BV_SET(pBV, endUintIndex, endMask);
    }
}

void vscBV_ClearInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i;
    gctUINT startMask, endMask;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange));

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick clear for range size 1 */
    if (szRange == 1)
    {
        BV_BIT_CLR(pBV, startUintIndex, startLocalBitLoc);
        return;
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    /* Mask in startUintIndex and endUintIndex for start and end ordinal respectively */
    startMask = VSC_UTILS_MAKE_BIT_MASK(startLocalBitLoc);
    startMask = startMask | (startMask - 1);
    endMask = VSC_UTILS_MAKE_HIGH_MASK(endLocalBitLoc);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        BV_CLR(pBV, startUintIndex, (startMask & endMask));
    }
    else
    {
        /* Start and end ordinary are located in different UINTs */

        BV_CLR(pBV, startUintIndex, startMask);

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            pBV->pBits[i] = CLR_VALUE;
        }

        BV_CLR(pBV, endUintIndex, endMask);
    }
}

gctBOOL vscBV_TestInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i;
    gctUINT startMask, endMask;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange));

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick check for range size 1 */
    if (szRange == 1)
    {
        return BV_BIT_TST(pBV, startUintIndex, startLocalBitLoc);
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    /* Mask in startUintIndex and endUintIndex for start and end ordinal respectively */
    startMask = VSC_UTILS_MAKE_BIT_MASK(startLocalBitLoc);
    startMask = startMask | (startMask - 1);
    endMask = VSC_UTILS_MAKE_HIGH_MASK(endLocalBitLoc);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        if (BV_TST(pBV, startUintIndex, (startMask & endMask)))
        {
            return gcvTRUE;
        }
    }
    else
    {
        /* Start and end ordinary are located in different UINT */

        if (BV_TST(pBV, startUintIndex, startMask))
        {
            return gcvTRUE;
        }

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            if (pBV->pBits[i])
            {
                return gcvTRUE;
            }
        }

        if (BV_TST(pBV, endUintIndex, endMask))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

gctBOOL vscBV_TestAndSetInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i;
    gctUINT startMask, endMask;
    gctBOOL bSetBefore = gcvFALSE;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange));

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick set for range size 1 */
    if (szRange == 1)
    {
        bSetBefore = BV_BIT_TST(pBV, startUintIndex, startLocalBitLoc);
        BV_BIT_SET(pBV, startUintIndex, startLocalBitLoc);
        return bSetBefore;
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    /* Mask in startUintIndex and endUintIndex for start and end ordinal respectively */
    startMask = VSC_UTILS_MAKE_BIT_MASK(startLocalBitLoc);
    startMask = startMask | (startMask - 1);
    endMask = VSC_UTILS_MAKE_HIGH_MASK(endLocalBitLoc);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        bSetBefore = BV_TST(pBV, startUintIndex, (startMask & endMask));
        BV_SET(pBV, startUintIndex, (startMask & endMask));
    }
    else
    {
        /* Start and end ordinary are located in different UINT */

        bSetBefore |= BV_TST(pBV, startUintIndex, startMask);
        BV_SET(pBV, startUintIndex, startMask);

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            bSetBefore |= (pBV->pBits[i] != CLR_VALUE);
            pBV->pBits[i] = SET_VALUE;
        }

        bSetBefore |= BV_TST(pBV, endUintIndex, endMask);
        BV_SET(pBV, endUintIndex, endMask);
    }

    return bSetBefore;
}

gctBOOL vscBV_TestAndClearInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i;
    gctUINT startMask, endMask;
    gctBOOL bSetBefore = gcvFALSE;

    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange));

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick clear for range size 1 */
    if (szRange == 1)
    {
        bSetBefore = BV_BIT_TST(pBV, startUintIndex, startLocalBitLoc);;
        BV_BIT_CLR(pBV, startUintIndex, startLocalBitLoc);
        return bSetBefore;
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    /* Mask in startUintIndex and endUintIndex for start and end ordinal respectively */
    startMask = VSC_UTILS_MAKE_BIT_MASK(startLocalBitLoc);
    startMask = startMask | (startMask - 1);
    endMask = VSC_UTILS_MAKE_HIGH_MASK(endLocalBitLoc);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        bSetBefore = BV_TST(pBV, startUintIndex, (startMask & endMask));
        BV_CLR(pBV, startUintIndex, (startMask & endMask));
    }
    else
    {
        /* Start and end ordinary are located in different UINTs */

        bSetBefore |= BV_TST(pBV, startUintIndex, startMask);
        BV_CLR(pBV, startUintIndex, startMask);

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            bSetBefore |= (pBV->pBits[i] != CLR_VALUE);
            pBV->pBits[i] = CLR_VALUE;
        }

        bSetBefore |= BV_TST(pBV, endUintIndex, endMask);
        BV_CLR(pBV, endUintIndex, endMask);
    }

    return bSetBefore;
}

static gcmINLINE gctBOOL _FindSigSetOrClearBitInLocalRange(gctBOOL bFindSet,
                                                           gctUINT bitValueInThisUint,
                                                           gctINT  startLocalBitLoc, /* Include this loc */
                                                           gctINT  endLocalBitLoc, /* Include this loc */
                                                           gctBOOL bForward,
                                                           gctINT* pRetBitLoc)
{
    gctBOOL bFound = gcvFALSE;
    gctUINT bitRangeMask;

    gcmASSERT(startLocalBitLoc >= 0 && startLocalBitLoc <= 31);
    gcmASSERT(endLocalBitLoc >= 0 && endLocalBitLoc <= 31);

    *pRetBitLoc = INVALID_BIT_LOC;

    bitRangeMask = VSC_UTILS_MAKE_BIT_RANGE_MASK(startLocalBitLoc, endLocalBitLoc);

    if (bFindSet)
    {
        /* Default is 0 */
        bitValueInThisUint = VSC_UTILS_MASK(bitValueInThisUint, bitRangeMask);

        if (bitValueInThisUint != CLR_VALUE)
        {
            if (bForward)
            {
                *pRetBitLoc = vscFindMostSigBit(bitValueInThisUint);
            }
            else
            {
                *pRetBitLoc = vscFindLeastSigBit(bitValueInThisUint);
            }

            bFound = gcvTRUE;
        }
    }
    else
    {
        /* Default is 1 */
        VSC_UTILS_SET(bitValueInThisUint, ~bitRangeMask);

        if (bitValueInThisUint != SET_VALUE)
        {
            if (bForward)
            {
                *pRetBitLoc = vscFindMostSigBit((~bitValueInThisUint));
            }
            else
            {
                *pRetBitLoc = vscFindLeastSigBit((~bitValueInThisUint));
            }

            bFound = gcvTRUE;
        }
    }

    return bFound;
}

static gcmINLINE gctINT _FindBitForward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctBOOL bFindSet)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  uintIndex, lastUintIndex, retBitLoc = INVALID_BIT_LOC;
    gctUINT uint;

    if (startBitOrdinal >= pBV->bitCount)
    {
        return INVALID_BIT_LOC;
    }

    if (startBitOrdinal < 0)
    {
        startBitOrdinal = 0;
    }

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* Find in uints except last uint */
    lastUintIndex = BV_MIN_NUM_OF_UINT(pBV) - 1;
    uintIndex = startUintIndex;
    uint = pBV->pBits[uintIndex];
    while (uintIndex < lastUintIndex)
    {
        if (_FindSigSetOrClearBitInLocalRange(bFindSet, uint, 0, startLocalBitLoc,
                                              gcvTRUE, &retBitLoc))
        {
            gcmASSERT(retBitLoc != INVALID_BIT_LOC);
            return BV_MAKE_BIT_ORDINAL(uintIndex, retBitLoc);
        }

        uintIndex ++;
        uint = pBV->pBits[uintIndex];
        startLocalBitLoc = 31;
    }

    /* Now check in last uint */
    if (_FindSigSetOrClearBitInLocalRange(bFindSet, uint,
                                          BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1),
                                          startLocalBitLoc, gcvTRUE, &retBitLoc))
    {
        gcmASSERT(retBitLoc != INVALID_BIT_LOC);
        return BV_MAKE_BIT_ORDINAL(uintIndex, retBitLoc);
    }

    return INVALID_BIT_LOC;
}

static gcmINLINE gctINT _FindBitBackward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctBOOL bFindSet)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  uintIndex, retBitLoc = INVALID_BIT_LOC;
    gctUINT uint;

    if (startBitOrdinal >= pBV->bitCount)
    {
        startBitOrdinal = pBV->bitCount - 1;
    }

    if (startBitOrdinal < 0)
    {
        return INVALID_BIT_LOC;
    }

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* Backward find from current uint to first uint */
    uintIndex = startUintIndex;
    uint = pBV->pBits[uintIndex];
    while (uintIndex >= 0)
    {
        if (_FindSigSetOrClearBitInLocalRange(bFindSet, uint, startLocalBitLoc,
                                              31, gcvFALSE, &retBitLoc))
        {
            gcmASSERT(retBitLoc != INVALID_BIT_LOC);
            return BV_MAKE_BIT_ORDINAL(uintIndex, retBitLoc);
        }

        uintIndex --;
        if (uintIndex >= 0)
        {
            uint = pBV->pBits[uintIndex];
        }
        startLocalBitLoc = 0;
    }

    return INVALID_BIT_LOC;
}

static gcmINLINE gctINT _FindBitInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange, gctBOOL bFindSet)
{
    gctINT  startUintIndex, startLocalBitLoc;
    gctINT  endUintIndex, endLocalBitLoc, i, retBitLoc = INVALID_BIT_LOC;
    gctUINT uint;

    gcmASSERT(szRange > 0);

    if (startBitOrdinal < 0)
    {
        startBitOrdinal = 0;
    }

    if (startBitOrdinal >= pBV->bitCount)
    {
        return INVALID_BIT_LOC;
    }

    szRange = vscMIN(szRange, pBV->bitCount - startBitOrdinal);

    startUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal);
    startLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal);

    /* A quick find for range size 1 */
    if (szRange == 1)
    {
        if ((bFindSet && BV_BIT_TST(pBV, startUintIndex, startLocalBitLoc)) ||
            (!bFindSet && !BV_BIT_TST(pBV, startUintIndex, startLocalBitLoc)))
        {
            return startBitOrdinal;
        }
        else
        {
            return INVALID_BIT_LOC;
        }
    }

    endUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(startBitOrdinal + szRange - 1);
    endLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(startBitOrdinal + szRange - 1);

    if (startUintIndex == endUintIndex)
    {
        /* Start and end ordinary are located in same UINT */

        if (_FindSigSetOrClearBitInLocalRange(bFindSet, pBV->pBits[startUintIndex], endLocalBitLoc,
                                              startLocalBitLoc, gcvTRUE, &retBitLoc))
        {
            gcmASSERT(retBitLoc != INVALID_BIT_LOC);
            return BV_MAKE_BIT_ORDINAL(startUintIndex, retBitLoc);
        }
    }
    else
    {
        /* Start and end ordinary are located in different UINT */

        if (_FindSigSetOrClearBitInLocalRange(bFindSet, pBV->pBits[startUintIndex], 0,
                                              startLocalBitLoc, gcvTRUE, &retBitLoc))
        {
            gcmASSERT(retBitLoc != INVALID_BIT_LOC);
            return BV_MAKE_BIT_ORDINAL(startUintIndex, retBitLoc);
        }

        for (i = startUintIndex + 1; i < endUintIndex; i ++)
        {
            uint = pBV->pBits[i];
            if (_FindSigSetOrClearBitInLocalRange(bFindSet, uint, 0, 31, gcvTRUE, &retBitLoc))
            {
                gcmASSERT(retBitLoc != INVALID_BIT_LOC);
                return BV_MAKE_BIT_ORDINAL(i, retBitLoc);
            }
        }

        if (_FindSigSetOrClearBitInLocalRange(bFindSet, pBV->pBits[endUintIndex],
                                              endLocalBitLoc, 31, gcvTRUE, &retBitLoc))
        {
            gcmASSERT(retBitLoc != INVALID_BIT_LOC);
            return BV_MAKE_BIT_ORDINAL(endUintIndex, retBitLoc);
        }
    }

    return INVALID_BIT_LOC;
}

gctINT vscBV_FindSetBitForward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitForward(pBV, startBitOrdinal, gcvTRUE);
}

gctINT vscBV_FindSetBitBackward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitBackward(pBV, startBitOrdinal, gcvTRUE);
}

gctINT vscBV_FindSetBitInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitInRange(pBV, startBitOrdinal, szRange, gcvTRUE);
}

gctINT vscBV_FindClearBitForward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitForward(pBV, startBitOrdinal, gcvFALSE);
}

gctINT vscBV_FindClearBitBackward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitBackward(pBV, startBitOrdinal, gcvFALSE);
}

gctINT vscBV_FindClearBitInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindBitInRange(pBV, startBitOrdinal, szRange, gcvFALSE);
}

typedef gctINT (*PFN_SEARCH_BIT)(VSC_BIT_VECTOR*, gctINT);
typedef gctINT (*PFN_SEARCH_BIT_RANGE)(VSC_BIT_VECTOR*, gctINT, gctINT);

static gcmINLINE gctINT _FindContinuousBits(VSC_BIT_VECTOR* pBV,
                                            gctINT bitCount,
                                            gctINT startBitOrdinal,
                                            gctBOOL bForward,
                                            PFN_SEARCH_BIT pfnBitSearchCallBack)
{
    gctINT   startBitOrdinalForThisRound = startBitOrdinal, i;
    gctINT   thisSearchBitOrdinal;

    gcmASSERT(bitCount > 0);

    if (bForward)
    {
        if (startBitOrdinal >= pBV->bitCount)
        {
            return INVALID_BIT_LOC;
        }

        if (startBitOrdinal < 0)
        {
            startBitOrdinal = 0;
        }
    }
    else
    {
        if (startBitOrdinal >= pBV->bitCount)
        {
            startBitOrdinal = pBV->bitCount - 1;
        }

        if (startBitOrdinal < 0)
        {
            return INVALID_BIT_LOC;
        }
    }

    while (gcvTRUE)
    {
        /* Do we need go on checking? */
        if (bForward)
        {
            if ((startBitOrdinalForThisRound + bitCount - 1) >= pBV->bitCount)
            {
                return INVALID_BIT_LOC;
            }
        }
        else
        {
            if ((startBitOrdinalForThisRound - bitCount + 1) < 0)
            {
                return INVALID_BIT_LOC;
            }
        }

        /* Check bits for this round */
        for (i = startBitOrdinalForThisRound; i < startBitOrdinalForThisRound + bitCount; i ++)
        {
            thisSearchBitOrdinal = pfnBitSearchCallBack(pBV, i);
            if (thisSearchBitOrdinal != i)
            {
                break;
            }
        }

        if (i == startBitOrdinalForThisRound + bitCount)
        {
            /* Yes, we find these continuous bits */
            return startBitOrdinalForThisRound;
        }
        else
        {
            /* Let try next round */

            if (bForward)
            {
                startBitOrdinalForThisRound = i + 1;
            }
            else
            {
                startBitOrdinalForThisRound = i - 1;
            }
        }
    }

    return INVALID_BIT_LOC;
}

static gcmINLINE gctINT _FindContinuousBitsInRange(VSC_BIT_VECTOR* pBV,
                                                   gctINT bitCount,
                                                   gctINT startBitOrdinal,
                                                   gctINT szRange,
                                                   PFN_SEARCH_BIT_RANGE pfnBitSearchCallBack)
{
    gctINT   startBitOrdinalForThisRound = startBitOrdinal, i;
    gctINT   endBitOrdinal, thisSearchBitOrdinal;

    gcmASSERT(bitCount > 0);

    if (startBitOrdinal < 0)
    {
        startBitOrdinal = 0;
    }

    if (startBitOrdinal >= pBV->bitCount)
    {
        return INVALID_BIT_LOC;
    }

    szRange = vscMIN(szRange, pBV->bitCount - startBitOrdinal);
    endBitOrdinal = startBitOrdinal + szRange - 1;

    while (gcvTRUE)
    {
        /* Do we need go on checking? */
        if ((startBitOrdinalForThisRound + bitCount - 1) > endBitOrdinal)
        {
            return INVALID_BIT_LOC;
        }

        /* Check bits for this round */
        for (i = startBitOrdinalForThisRound; i < startBitOrdinalForThisRound + bitCount; i ++)
        {
            thisSearchBitOrdinal = pfnBitSearchCallBack(pBV, i, endBitOrdinal - startBitOrdinalForThisRound);
            if (thisSearchBitOrdinal != i)
            {
                break;
            }
        }

        if (i == startBitOrdinalForThisRound + bitCount)
        {
            /* Yes, we find these continuous bits */
            return startBitOrdinalForThisRound;
        }
        else
        {
            /* Let try next round */
            startBitOrdinalForThisRound = i + 1;
        }
    }

    return INVALID_BIT_LOC;
}

gctINT vscBV_FindContinuousSetBitsForward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindContinuousBits(pBV, bitCount, startBitOrdinal, gcvTRUE, vscBV_FindSetBitForward);
}

gctINT vscBV_FindContinuousSetBitsBackward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindContinuousBits(pBV, bitCount, startBitOrdinal, gcvFALSE, vscBV_FindSetBitBackward);
}

gctINT vscBV_FindContinuousSetBitsInRange(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal, gctINT szRange)
{
    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(szRange > 0);

    return _FindContinuousBitsInRange(pBV, bitCount, startBitOrdinal, szRange, vscBV_FindSetBitInRange);
}

gctINT vscBV_FindContinuousClearBitsForward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindContinuousBits(pBV, bitCount, startBitOrdinal, gcvTRUE, vscBV_FindClearBitForward);
}

gctINT vscBV_FindContinuousClearBitsBackward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal)
{
    gcmASSERT(BV_IS_VALID(pBV));

    return _FindContinuousBits(pBV, bitCount, startBitOrdinal, gcvFALSE, vscBV_FindClearBitBackward);
}

gctINT vscBV_FindContinuousClearBitsInRange(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal, gctINT szRange)
{
    gcmASSERT(BV_IS_VALID(pBV));
    gcmASSERT(szRange > 0);

    return _FindContinuousBitsInRange(pBV, bitCount, startBitOrdinal, szRange, vscBV_FindClearBitInRange);
}

gctBOOL vscBV_Any(VSC_BIT_VECTOR* pBV)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pBV));

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1);
    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if (pBV->pBits[i] != CLR_VALUE)
        {
            return gcvTRUE;
        }
    }

    /* Now check in last uint */
    if (BV_MASK(pBV, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)) != CLR_VALUE)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL vscBV_All(VSC_BIT_VECTOR* pBV)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pBV));

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1);
    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if (pBV->pBits[i] != SET_VALUE)
        {
            return gcvFALSE;
        }
    }

    /* Now check in last uint */
    if (BV_MASK(pBV, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)) !=
                                    VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL vscBV_Equal(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pBV1));
    gcmASSERT(BV_IS_VALID(pBV2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV1);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pBV2));
    gcmASSERT(pBV1->bitCount == pBV2->bitCount);

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV1->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV1->bitCount - 1);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if (pBV1->pBits[i] != pBV2->pBits[i])
        {
            return gcvFALSE;
        }
    }

    /* Now check in last uint */
    if (BV_MASK(pBV1, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)) !=
        BV_MASK(pBV2, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL vscBV_GreatEqual(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;
    gctUINT uint1, uint2;

    gcmASSERT(BV_IS_VALID(pBV1));
    gcmASSERT(BV_IS_VALID(pBV2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV1);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pBV2));
    gcmASSERT(pBV1->bitCount == pBV2->bitCount);

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV1->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV1->bitCount - 1);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if ((pBV1->pBits[i] & pBV2->pBits[i]) != pBV2->pBits[i])
        {
            return gcvFALSE;
        }
    }

    /* Now check in last uint */
    uint1 = BV_MASK(pBV1, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    uint2 = BV_MASK(pBV2, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    if ((uint1 & uint2) != uint2)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL vscBV_LessThan(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;
    gctUINT uint1, uint2;

    gcmASSERT(BV_IS_VALID(pBV1));
    gcmASSERT(BV_IS_VALID(pBV2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV1);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pBV2));
    gcmASSERT(pBV1->bitCount == pBV2->bitCount);

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV1->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV1->bitCount - 1);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if ((pBV1->pBits[i] & pBV2->pBits[i]) != pBV1->pBits[i] ||
            pBV1->pBits[i] == pBV2->pBits[i])
        {
            return gcvFALSE;
        }
    }

    /* Now check in last uint */
    uint1 = BV_MASK(pBV1, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    uint2 = BV_MASK(pBV2, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    if ((uint1 & uint2) != uint1 || uint1 == uint2)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL vscBV_Intersected(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT, i;
    gctUINT uint1, uint2;

    gcmASSERT(BV_IS_VALID(pBV1));
    gcmASSERT(BV_IS_VALID(pBV2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV1);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pBV2));
    gcmASSERT(pBV1->bitCount == pBV2->bitCount);

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV1->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV1->bitCount - 1);

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        if (pBV1->pBits[i] & pBV2->pBits[i])
        {
            return gcvTRUE;
        }
    }

    /* Now check in last uint */
    uint1 = BV_MASK(pBV1, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    uint2 = BV_MASK(pBV2, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
    if (uint1 & uint2)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctUINT vscBV_CountBits(VSC_BIT_VECTOR* pBV)
{
    gctUINT  lastUintIndex, lastLocalBitLoc;
    gctUINT  minNumOfUINT, i, count;

    gcmASSERT(BV_IS_VALID(pBV));

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pBV->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBV->bitCount - 1);
    minNumOfUINT = BV_MIN_NUM_OF_UINT(pBV);

    count = 0;

    /* Check in uints except last uint */
    for (i = 0; i < minNumOfUINT-1; i ++)
    {
        count += vscFindPopulation(pBV->pBits[i]);
    }

    /* Now check in last uint */
    count += vscFindPopulation(BV_MASK(pBV, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)));

    return count;
}

void vscBV_Copy(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc)
{
    gctINT  lastUintIndex, lastLocalBitLoc;
    gctINT  minNumOfUINT;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pSrc);
    gcmASSERT(minNumOfUINT <= BV_MIN_NUM_OF_UINT(pDst));
    gcmASSERT(pSrc->bitCount <= pDst->bitCount);

    lastUintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(pSrc->bitCount - 1);
    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pSrc->bitCount - 1);

    /* Uints except last uint */
    memcpy(pDst->pBits, pSrc->pBits, sizeof(gctUINT) * (minNumOfUINT -1));

    /* Last uint */
    BV_CLR(pDst, lastUintIndex, BV_MASK(pDst, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)));
    BV_SET(pDst, lastUintIndex, BV_MASK(pSrc, lastUintIndex, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc)));
}

void vscBV_Not(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc)
{
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pSrc);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pDst));
    gcmASSERT(pSrc->bitCount == pDst->bitCount);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        pDst->pBits[i] = ~pSrc->pBits[i];
    }
}

gctBOOL vscBV_Xor(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2)
{
    gctINT  minNumOfUINT, i, lastLocalBitLoc;
    gctBOOL bChanged = gcvFALSE;
    gctUINT tmp;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc1));
    gcmASSERT(BV_IS_VALID(pSrc2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pDst);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc1));
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc2));
    gcmASSERT(pSrc1->bitCount == pDst->bitCount);
    gcmASSERT(pSrc2->bitCount == pDst->bitCount);

    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pDst->bitCount - 1);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        tmp = pSrc1->pBits[i] ^ pSrc2->pBits[i];

        /* For last special uint */
        if (i == minNumOfUINT - 1)
        {
            tmp = VSC_UTILS_MASK(tmp, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
        }

        bChanged |= (tmp != 0);
        pDst->pBits[i] = tmp;
    }

    return bChanged;
}

gctBOOL vscBV_Or1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc)
{
    gctINT  minNumOfUINT, i, lastLocalBitLoc;
    gctBOOL bChanged = gcvFALSE;
    gctUINT oldUint;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pSrc);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pDst));
    gcmASSERT(pSrc->bitCount == pDst->bitCount);

    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pDst->bitCount - 1);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        oldUint = pDst->pBits[i];
        pDst->pBits[i] |= pSrc->pBits[i];

        /* For last special uint */
        if (i == minNumOfUINT - 1)
        {
            oldUint = VSC_UTILS_MASK(oldUint, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
            pDst->pBits[i] = VSC_UTILS_MASK(pDst->pBits[i], VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
        }

        bChanged |= (pDst->pBits[i] != oldUint);
    }

    return bChanged;
}

void vscBV_Or2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2)
{
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc1));
    gcmASSERT(BV_IS_VALID(pSrc2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pDst);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc1));
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc2));
    gcmASSERT(pSrc1->bitCount == pDst->bitCount);
    gcmASSERT(pSrc2->bitCount == pDst->bitCount);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        pDst->pBits[i] = pSrc1->pBits[i] | pSrc2->pBits[i];
    }
}

gctBOOL vscBV_And1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc)
{
    gctINT  minNumOfUINT, i, lastLocalBitLoc;
    gctBOOL bChanged = gcvFALSE;
    gctUINT oldUint;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pSrc);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pDst));
    gcmASSERT(pSrc->bitCount == pDst->bitCount);

    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pDst->bitCount - 1);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        oldUint = pDst->pBits[i];
        pDst->pBits[i] &= pSrc->pBits[i];

        /* For last special uint */
        if (i == minNumOfUINT - 1)
        {
            oldUint = VSC_UTILS_MASK(oldUint, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
            pDst->pBits[i] = VSC_UTILS_MASK(pDst->pBits[i], VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
        }

        bChanged |= (pDst->pBits[i] != oldUint);
    }

    return bChanged;
}

void vscBV_And2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2)
{
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc1));
    gcmASSERT(BV_IS_VALID(pSrc2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pDst);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc1));
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc2));
    gcmASSERT(pSrc1->bitCount == pDst->bitCount);
    gcmASSERT(pSrc2->bitCount == pDst->bitCount);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        pDst->pBits[i] = pSrc1->pBits[i] & pSrc2->pBits[i];
    }
}

gctBOOL vscBV_Minus1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc)
{
    gctINT  minNumOfUINT, i, lastLocalBitLoc;
    gctBOOL bChanged = gcvFALSE;
    gctUINT oldUint;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pSrc);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pDst));
    gcmASSERT(pSrc->bitCount == pDst->bitCount);

    lastLocalBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(pDst->bitCount - 1);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        oldUint = pDst->pBits[i];
        pDst->pBits[i] &= (~(pSrc->pBits[i]));

        /* For last special uint */
        if (i == minNumOfUINT - 1)
        {
            oldUint = VSC_UTILS_MASK(oldUint, VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
            pDst->pBits[i] = VSC_UTILS_MASK(pDst->pBits[i], VSC_UTILS_MAKE_HIGH_MASK(lastLocalBitLoc));
        }

        bChanged |= (pDst->pBits[i] != oldUint);
    }

    return bChanged;
}

void vscBV_Minus2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2)
{
    gctINT  minNumOfUINT, i;

    gcmASSERT(BV_IS_VALID(pDst));
    gcmASSERT(BV_IS_VALID(pSrc1));
    gcmASSERT(BV_IS_VALID(pSrc2));

    minNumOfUINT = BV_MIN_NUM_OF_UINT(pDst);
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc1));
    gcmASSERT(minNumOfUINT == BV_MIN_NUM_OF_UINT(pSrc2));
    gcmASSERT(pSrc1->bitCount == pDst->bitCount);
    gcmASSERT(pSrc2->bitCount == pDst->bitCount);

    for (i = 0; i < minNumOfUINT; i ++)
    {
        pDst->pBits[i] = pSrc1->pBits[i] & ~(pSrc2->pBits[i]);
    }
}

