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


#include "gc_vsc.h"

void vscBM_Initialize(VSC_BIT_MATRIX* pBM, VSC_MM* pMM, gctINT rowSize, gctINT colSize)
{
    gctINT   totalNumOfUINT;

    if (pMM == gcvNULL && (rowSize <= 0 || colSize <= 0))
    {
        memset(pBM, 0, sizeof(VSC_BIT_MATRIX));
        return;
    }

    /* Force to safe size */
    if (rowSize <= 0)
    {
        rowSize = 1;
    }

    if (colSize <= 0)
    {
        colSize = 1;
    }

    pBM->pBits = gcvNULL;
    pBM->rowBitCount = rowSize;
    pBM->colBitCount = colSize;
    pBM->pMM = pMM;
    pBM->rowNumOfUINT = BM_ROW_SIZE_2_UINT_COUNT(rowSize);
    pBM->colNumOfUINT = BM_COL_SIZE_2_UINT_COUNT(colSize);

    totalNumOfUINT = pBM->rowNumOfUINT * pBM->colNumOfUINT;
    if (totalNumOfUINT)
    {
        pBM->pBits = (gctUINT*)vscMM_Alloc(pMM, totalNumOfUINT * sizeof(gctUINT));

        if (pBM->pBits == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
            return;
        }
        else
        {
            memset(pBM->pBits, CLR_VALUE, totalNumOfUINT * sizeof(gctUINT));
        }
    }
}

VSC_BIT_MATRIX* vscBM_Create(VSC_MM* pMM, gctINT rowSize, gctINT colSize)
{
    VSC_BIT_MATRIX*   pBM = gcvNULL;

    pBM = (VSC_BIT_MATRIX*)vscMM_Alloc(pMM, sizeof(VSC_BIT_MATRIX));

    vscBM_Initialize(pBM, pMM, rowSize, colSize);

    return pBM;
}

void vscBM_Finalize(VSC_BIT_MATRIX* pBM)
{
    if (pBM->pMM)
    {
        vscMM_Free(pBM->pMM, pBM->pBits);
    }

    pBM->pBits = gcvNULL;
    pBM->rowBitCount = 0;
    pBM->colBitCount = 0;
    pBM->rowNumOfUINT = 0;
    pBM->colNumOfUINT = 0;
}

void vscBM_Destroy(VSC_BIT_MATRIX* pBM)
{
    if (pBM)
    {
        vscBM_Finalize(pBM);
        vscMM_Free(pBM->pMM, pBM);
        pBM = gcvNULL;
    }
}

