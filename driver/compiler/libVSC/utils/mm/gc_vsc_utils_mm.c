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

/*
  Common memory management interfaces for user.
*/

void vscMM_Initialize(VSC_MM* pMM, void* pMemSys, VSC_MM_TYPE mmType)
{
    pMM->mmType = mmType;
    pMM->ms.pMemSys = pMemSys;
}

void* vscMM_Alloc(VSC_MM* pMM, gctUINT reqSize)
{
    switch (pMM->mmType)
    {
    case VSC_MM_TYPE_PMP:
        return vscPMP_Alloc(pMM->ms.pPMP, reqSize);
        break;
    case VSC_MM_TYPE_BMS:
        return vscBMS_Alloc(pMM->ms.pBMS, reqSize);
        break;
    case VSC_MM_TYPE_AMS:
        return vscAMS_Alloc(pMM->ms.pAMS, reqSize);
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return gcvNULL;
}

void* vscMM_Realloc(VSC_MM* pMM, void* pOrgAddress, gctUINT newReqSize)
{
    switch (pMM->mmType)
    {
    case VSC_MM_TYPE_PMP:
        return vscPMP_Realloc(pMM->ms.pPMP, pOrgAddress, newReqSize);
        break;
    case VSC_MM_TYPE_BMS:
        return vscBMS_Realloc(pMM->ms.pBMS, pOrgAddress, newReqSize);
        break;
    case VSC_MM_TYPE_AMS:
        return vscAMS_Realloc(pMM->ms.pAMS, pOrgAddress, newReqSize);
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    return gcvNULL;
}

void vscMM_Free(VSC_MM* pMM, void *pData)
{
    switch (pMM->mmType)
    {
    case VSC_MM_TYPE_PMP:
        vscPMP_Free(pMM->ms.pPMP, pData);
        break;
    case VSC_MM_TYPE_BMS:
        vscBMS_Free(pMM->ms.pBMS, pData);
        break;
    case VSC_MM_TYPE_AMS:
        /* No actual free at all for AMS */
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
}

void vscMM_Finalize(VSC_MM* pMM)
{
    /* Nothing to do */
}

