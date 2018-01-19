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


#ifndef __gc_vsc_utils_bitvector_h_
#define __gc_vsc_utils_bitvector_h_

BEGIN_EXTERN_C()

/*
 * bit-vector
 */

typedef struct _VSC_BIT_VECTOR
{
    /* True bit count in this bit-vector */
    gctINT                    bitCount;

    /* How many UINTs are allocated to accommodate bit count of this bit-vector. Note that after
       resize, numOfUINT may be greater than (((bitCount) + 31) >> 5) */
    gctINT                    numOfUINT;

    /* Content of this bit-vector */
    gctUINT*                  pBits;

    /* What type of MM are this bit-vector built on? */
    VSC_MM*                   pMM;
}VSC_BIT_VECTOR;

#define BV_SIZE_2_UINT_COUNT(bvSize)                  (((bvSize) + 31) >> 5)
#define BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal)    ((bitOrdinal) >> 5)
#define BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal)     (31 - ((bitOrdinal) & 31)) /* MSB has the least ordinal */
#define BV_MAKE_BIT_ORDINAL(uintIndex, bitLocalLoc)   (((uintIndex) << 5) + 31 - (bitLocalLoc))

#define BV_MIN_NUM_OF_UINT(pBV)                       BV_SIZE_2_UINT_COUNT((pBV)->bitCount)

#define BV_SET(pBV, uintIndex, mask)                  VSC_UTILS_SET((pBV)->pBits[(uintIndex)], (mask))
#define BV_CLR(pBV, uintIndex, mask)                  VSC_UTILS_CLR((pBV)->pBits[(uintIndex)], (mask))
#define BV_TST(pBV, uintIndex, mask)                  VSC_UTILS_TST((pBV)->pBits[(uintIndex)], (mask))

#define BV_BIT_SET(pBV, uintIndex, localBitLoc)       BV_SET((pBV), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))
#define BV_BIT_CLR(pBV, uintIndex, localBitLoc)       BV_CLR((pBV), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))
#define BV_BIT_TST(pBV, uintIndex, localBitLoc)       BV_TST((pBV), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))

#define BV_MASK(pBV, uintIndex, mask)                 BV_TST((pBV), (uintIndex), (mask))

#define BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal)                          \
        (((bitOrdinal) >= 0) && ((bitOrdinal) < (pBV)->bitCount))
#define BV_IS_LEGAL_BIT_ORDINAL_RANGE(pBV, startBitOrdinal, szRange)      \
        (((startBitOrdinal) >= 0) &&                                      \
        ((startBitOrdinal) < ((startBitOrdinal) + (szRange))) &&          \
        (((startBitOrdinal) + (szRange)) <= (pBV)->bitCount))
#define BV_IS_VALID(pBV)                                                  \
        ((pBV)->bitCount > 0 &&                                           \
         (pBV)->numOfUINT > 0 &&                                          \
         (pBV)->pBits != gcvNULL &&                                       \
         (pBV)->pMM != gcvNULL)

/* Creation, resize and destroy */
VSC_BIT_VECTOR* vscBV_Create(VSC_MM* pMM, gctINT bvSize);
void vscBV_Initialize(VSC_BIT_VECTOR* pBV, VSC_MM* pMM, gctINT bvSize);
void vscBV_Resize(VSC_BIT_VECTOR *pBV, gctINT newBVSize, gctBOOL bKeep);
void vscBV_Destroy(VSC_BIT_VECTOR* pBV);
void vscBV_Finalize(VSC_BIT_VECTOR* pBV);

/* Set, clear and test */
void vscBV_SetAll(VSC_BIT_VECTOR* pBV);
void vscBV_ClearAll(VSC_BIT_VECTOR* pBV);
void vscBV_SetResidual(VSC_BIT_VECTOR* pBV);
void vscBV_ClearResidual(VSC_BIT_VECTOR* pBV);
gctBOOL vscBV_TestAndSetBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal);
gctBOOL vscBV_TestAndClearBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal);
void vscBV_SetInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);
void vscBV_ClearInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);
gctBOOL vscBV_TestInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);
gctBOOL vscBV_TestAndSetInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);
gctBOOL vscBV_TestAndClearInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);

static gcmINLINE void vscBV_SetBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal));

    uintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal);
    localBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal);
    BV_BIT_SET(pBV, uintIndex, localBitLoc);
}

static gcmINLINE void vscBV_ClearBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal));

    uintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal);
    localBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal);
    BV_BIT_CLR(pBV, uintIndex, localBitLoc);
}

static gcmINLINE gctBOOL vscBV_TestBit(VSC_BIT_VECTOR* pBV, gctINT bitOrdinal)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BV_IS_LEGAL_BIT_ORDINAL(pBV, bitOrdinal));

    uintIndex = BV_BIT_GLOBAL_LOC_2_UINT_INDEX(bitOrdinal);
    localBitLoc = BV_BIT_GLOBAL_LOC_2_LOCAL_LOC(bitOrdinal);
    return BV_BIT_TST(pBV, uintIndex, localBitLoc);
}

/* Single bit search */
gctINT vscBV_FindSetBitForward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal);
gctINT vscBV_FindSetBitBackward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal);
gctINT vscBV_FindSetBitInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);
gctINT vscBV_FindClearBitForward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal);
gctINT vscBV_FindClearBitBackward(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal);
gctINT vscBV_FindClearBitInRange(VSC_BIT_VECTOR* pBV, gctINT startBitOrdinal, gctINT szRange);

/* Multiple continuous bits search */
gctINT vscBV_FindContinuousSetBitsForward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal);
gctINT vscBV_FindContinuousSetBitsBackward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal);
gctINT vscBV_FindContinuousSetBitsInRange(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal, gctINT szRange);
gctINT vscBV_FindContinuousClearBitsForward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal);
gctINT vscBV_FindContinuousClearBitsBackward(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal);
gctINT vscBV_FindContinuousClearBitsInRange(VSC_BIT_VECTOR* pBV, gctINT bitCount, gctINT startBitOrdinal, gctINT szRange);

/* Condition op */
gctBOOL vscBV_Any(VSC_BIT_VECTOR* pBV);
gctBOOL vscBV_All(VSC_BIT_VECTOR* pBV);
gctBOOL vscBV_Equal(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2);
gctBOOL vscBV_GreatEqual(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2);
gctBOOL vscBV_LessThan(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2);
gctBOOL vscBV_Intersected(VSC_BIT_VECTOR* pBV1, VSC_BIT_VECTOR* pBV2);

/* Logic and arithmetics op */
void vscBV_Not(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc);
gctBOOL vscBV_Xor(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2);
gctBOOL vscBV_Or1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc);
void vscBV_Or2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2);
gctBOOL vscBV_And1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc);
void vscBV_And2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2);
gctBOOL vscBV_Minus1(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc);
void vscBV_Minus2(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc1, VSC_BIT_VECTOR* pSrc2);

/* Misc */
gctUINT vscBV_CountBits(VSC_BIT_VECTOR* pBV);
void vscBV_Copy(VSC_BIT_VECTOR* pDst, VSC_BIT_VECTOR* pSrc);
void PrintBitVector(VSC_BIT_VECTOR* pBV);
void PrintBitVectorBits(VSC_BIT_VECTOR* pBV);

/*
 * bit-matrix
 */

typedef struct _VSC_BIT_MATRIX
{
    /* True row bit count in this bit-matrix */
    gctINT                    rowBitCount;

    /* True colomn bit count in this bit-matrix */
    gctINT                    colBitCount;

    /* How many UINTs are allocated to accommodate bit count of this bit-matrix. Note that after
       resize, rowNumOfUINT may be greater than (((rowBitCount + 31) >> 5)), and colNumOfUINT
       may be greater than colBitCount */
    gctINT                    rowNumOfUINT;
    gctINT                    colNumOfUINT;

    /* Content of this bit-matrix */
    gctUINT*                  pBits;

    /* What type of MM are this bit-matrix built on? */
    VSC_MM*                   pMM;
}VSC_BIT_MATRIX;

#define BM_ROW_SIZE_2_UINT_COUNT(rowSize)                                (((rowSize) + 31) >> 5)
#define BM_COL_SIZE_2_UINT_COUNT(colSize)                                (colSize)
#define BM_BIT_GLOBAL_LOC_2_UINT_INDEX(pBM, bitOrdinalX, bitOrdinalY)    (((pBM)->rowNumOfUINT *(bitOrdinalY)) + ((bitOrdinalX) >> 5))
#define BM_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBM, bitOrdinalX, bitOrdinalY)     (31 - ((bitOrdinalX) & 31)) /* MSB has the least ordinal */
#define BM_MAKE_BIT_ORDINAL_X(pBM, uintIndex, bitLocalLoc)               ((((uintIndex) % (pBM)->rowNumOfUINT) << 5) + 31 - (bitLocalLoc))
#define BM_MAKE_BIT_ORDINAL_Y(pBM, uintIndex, bitLocalLoc)               ((uintIndex) / (pBM)->rowNumOfUINT)

#define BM_SET(pBM, uintIndex, mask)                  VSC_UTILS_SET((pBM)->pBits[(uintIndex)], (mask))
#define BM_CLR(pBM, uintIndex, mask)                  VSC_UTILS_CLR((pBM)->pBits[(uintIndex)], (mask))
#define BM_TST(pBM, uintIndex, mask)                  VSC_UTILS_TST((pBM)->pBits[(uintIndex)], (mask))

#define BM_BIT_SET(pBM, uintIndex, localBitLoc)       BM_SET((pBM), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))
#define BM_BIT_CLR(pBM, uintIndex, localBitLoc)       BM_CLR((pBM), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))
#define BM_BIT_TST(pBM, uintIndex, localBitLoc)       BM_TST((pBM), (uintIndex), VSC_UTILS_MAKE_BIT_MASK((localBitLoc)))

#define BM_IS_LEGAL_BIT_ORDINAL(pBM, bitOrdinalX, bitOrdinalY)                          \
        (((bitOrdinalX) >= 0) && ((bitOrdinalX) < (pBM)->rowBitCount) &&                \
         ((bitOrdinalY) >= 0) && ((bitOrdinalY) < (pBM)->colBitCount))

/* Creation, resize and destroy */
VSC_BIT_MATRIX* vscBM_Create(VSC_MM* pMM, gctINT rowSize, gctINT colSize);
void vscBM_Initialize(VSC_BIT_MATRIX* pBM, VSC_MM* pMM, gctINT rowSize, gctINT colSize);
void vscBM_Destroy(VSC_BIT_MATRIX* pBM);
void vscBM_Finalize(VSC_BIT_MATRIX* pBM);

/* Set, clear and test */
static gcmINLINE void vscBM_SetBit(VSC_BIT_MATRIX* pBM, gctINT bitOrdinalX, gctINT bitOrdinalY)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BM_IS_LEGAL_BIT_ORDINAL(pBM, bitOrdinalX, bitOrdinalY));

    uintIndex = BM_BIT_GLOBAL_LOC_2_UINT_INDEX(pBM, bitOrdinalX, bitOrdinalY);
    localBitLoc = BM_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBM, bitOrdinalX, bitOrdinalY);
    BM_BIT_SET(pBM, uintIndex, localBitLoc);
}

static gcmINLINE void vscBM_ClearBit(VSC_BIT_MATRIX* pBM, gctINT bitOrdinalX, gctINT bitOrdinalY)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BM_IS_LEGAL_BIT_ORDINAL(pBM, bitOrdinalX, bitOrdinalY));

    uintIndex = BM_BIT_GLOBAL_LOC_2_UINT_INDEX(pBM, bitOrdinalX, bitOrdinalY);
    localBitLoc = BM_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBM, bitOrdinalX, bitOrdinalY);
    BM_BIT_CLR(pBM, uintIndex, localBitLoc);
}

static gcmINLINE gctBOOL vscBM_TestBit(VSC_BIT_MATRIX* pBM, gctINT bitOrdinalX, gctINT bitOrdinalY)
{
    gctINT uintIndex, localBitLoc;

    gcmASSERT(BM_IS_LEGAL_BIT_ORDINAL(pBM, bitOrdinalX, bitOrdinalY));

    uintIndex = BM_BIT_GLOBAL_LOC_2_UINT_INDEX(pBM, bitOrdinalX, bitOrdinalY);
    localBitLoc = BM_BIT_GLOBAL_LOC_2_LOCAL_LOC(pBM, bitOrdinalX, bitOrdinalY);
    return BM_BIT_TST(pBM, uintIndex, localBitLoc);
}

/*
 * state-vector
 */

typedef struct _VSC_STATE_VECTOR
{
    /* We use BV array to wrap a SV */
    VSC_BIT_VECTOR*           pBVs;

    /* How many BVs need to use to represent stateCount states, it should be log2(vscAlignToPow2(stateCount)).
       So if stateCount is LE 2, then SV is equal to BV */
    gctINT                    bvCount;

    /* How many states this SV represents. NOTE that user's state value can not be GE stateCount. User can
       enumrate their states values (from 0 to (stateCount - 1)), but they are not aware of how stores in BVs */
    gctUINT                   stateCount;

    /* How many size of this state-vector */
    gctINT                    svSize;

    /* What type of MM are this multiple-bits-vector built on? */
    VSC_MM*                   pMM;
}VSC_STATE_VECTOR;

#define SV_IS_LEGAL_ORDINAL(pSV, ordinal)                          \
        (((ordinal) >= 0) && ((ordinal) < (pSV)->svSize))
#define SV_IS_LEGAL_ORDINAL_RANGE(pSV, startOrdinal, szRange)      \
        (((startOrdinal) >= 0) &&                                  \
        ((startOrdinal) < ((startOrdinal) + (szRange))) &&         \
        (((startOrdinal) + (szRange)) <= (pSV)->svSize))
#define SV_IS_LEGAL_STATE(pSV, state)                              \
        (state < (pSV)->stateCount)
#define SV_IS_VALID(pSV)                                           \
        ((pSV)->bvCount > 0 &&                                     \
         (pSV)->stateCount > 1 &&                                  \
         (pSV)->svSize > 0 &&                                      \
         (pSV)->pBVs != gcvNULL &&                                 \
         (pSV)->pMM != gcvNULL)

#define INVALID_STATE_LOC   (-1)

/* Creation, resize and destroy */
VSC_STATE_VECTOR* vscSV_Create(VSC_MM* pMM, gctINT svSize, gctUINT stateCount);
void vscSV_Initialize(VSC_STATE_VECTOR* pSV, VSC_MM* pMM, gctINT svSize, gctUINT stateCount);
void vscSV_Resize(VSC_STATE_VECTOR *pSV, gctINT newSvSize, gctBOOL bKeep);
void vscSV_Destroy(VSC_STATE_VECTOR* pSV);
void vscSV_Finalize(VSC_STATE_VECTOR* pSV);

/* Set, get and test state */
void vscSV_Set(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state);
gctUINT vscSV_Get(VSC_STATE_VECTOR* pSV, gctINT ordinal);
gctBOOL vscSV_Test(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state);
void vscSV_SetAll(VSC_STATE_VECTOR* pSV, gctUINT state);
void vscSV_SetResidual(VSC_STATE_VECTOR* pSV, gctUINT state);
gctBOOL vscSV_TestAndSet(VSC_STATE_VECTOR* pSV, gctINT ordinal, gctUINT state);
void vscSV_SetInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state);
gctBOOL vscSV_TestInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state);
gctBOOL vscSV_TestAndSetInRange(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctINT szRange, gctUINT state);

/* State search */
gctINT vscSV_FindStateForward(VSC_STATE_VECTOR* pSV, gctINT startOrdinal, gctUINT state);

/* Condition op */
gctBOOL vscSV_Any(VSC_STATE_VECTOR* pSV, gctUINT state);
gctBOOL vscSV_All(VSC_STATE_VECTOR* pSV, gctUINT state);
gctBOOL vscSV_Equal(VSC_STATE_VECTOR* pSV1, VSC_STATE_VECTOR* pSV2);

/* Misc */
gctUINT vscSV_CountStateCount(VSC_STATE_VECTOR* pSV, gctUINT state);
void vscSV_Copy(VSC_STATE_VECTOR* pDst, VSC_STATE_VECTOR* pSrc);

END_EXTERN_C()

#endif /* __gc_vsc_utils_bitvector_h_ */

