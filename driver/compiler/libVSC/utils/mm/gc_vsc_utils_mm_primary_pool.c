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
  Primary memory pool provides size-controllable chunk to diminish external (system
  call) malloc/free calls by small allocations are fully acted on these chunks. Also
  this pool can diminish fragments of underlying memory system due to underlying mem
  sys can only see our big chunks.
*/

#define ENABLE_PMP_DEBUG_DUMP                0

#define PMP_GET_CHUNK_PTR(pChunkListNode)    ((pChunkListNode) ? \
                                              (VSC_PRIMARY_MEM_CHUNK*)vscBLNDEXT_GetContainedUserData((pChunkListNode)) : \
                                              (VSC_PRIMARY_MEM_CHUNK*)gcvNULL)

/* Global PMP counter */
static gctINT pmpCounter = 0;

static void _CreateNewChunk(VSC_PRIMARY_MEM_POOL* pPMP, gctUINT reqSize)
{
    VSC_PRIMARY_MEM_CHUNK* pNewChunk;
    gctINT                 chunkHeaderSize;
    gctUINT8*              pMem;
    gctUINT                chunkSize = reqSize;

    /* Calc chunk size */
    chunkHeaderSize = VSC_UTILS_ALIGN(sizeof(VSC_PRIMARY_MEM_CHUNK), pPMP->alignInSize);
    if (chunkSize < pPMP->lowLimitOfChunkSize)
    {
        chunkSize = pPMP->lowLimitOfChunkSize;
    }

    /* New a chunk */
    pMem = (gctUINT8*)pPMP->mmParam.pfnAlloc(chunkSize + chunkHeaderSize);
    if (pMem != gcvNULL)
    {
        /* Cast to chunk header */
        pNewChunk = (VSC_PRIMARY_MEM_CHUNK*)pMem;

        /* Fill in contents */
        pNewChunk->RemainderValidSize = chunkSize;
        pNewChunk->pStartOfRemainderValidData = pMem + chunkHeaderSize;
        pNewChunk->flags.bWholeChunkAllocated = gcvFALSE;
        vscBLNDEXT_Initialize(&pNewChunk->biChunkNode, pNewChunk);

        /* Append it at the tail of chunk chain */
        vscBILST_Append(&pPMP->biChunkChain, CAST_BLEN_2_BLN(&pNewChunk->biChunkNode));
    }
    else
    {
        /* ERROR */
        gcmASSERT(gcvFALSE);
    }
}

static void _DeleteChunk(VSC_PRIMARY_MEM_POOL* pPMP, VSC_PRIMARY_MEM_CHUNK* pChunkToDelete)
{
    /* Remove it from chain */
    vscBILST_Remove(&pPMP->biChunkChain, CAST_BLEN_2_BLN(&pChunkToDelete->biChunkNode));
    vscBLNDEXT_Finalize(&pChunkToDelete->biChunkNode);

    /* Delete it now */
    pPMP->mmParam.pfnFree(pChunkToDelete);
}

void vscPMP_Intialize(VSC_PRIMARY_MEM_POOL* pPMP, VSC_MEMORY_MANAGEMENT_PARAM* pMMParam,
                      gctUINT lowLimitOfChunkSize, gctUINT alignInSize, gctBOOL bPooling)
{
#if !ENABLE_EXTERNAL_MEM_TOOL_CHECK
    if (pMMParam)
    {
        pPMP->mmParam.pfnAlloc = pMMParam->pfnAlloc;
        pPMP->mmParam.pfnReAlloc = pMMParam->pfnReAlloc;
        pPMP->mmParam.pfnFree = pMMParam->pfnFree;
    }
    else
#endif
    {
        /* Default parameters with c-lib */
        pPMP->mmParam.pfnAlloc = (PFN_VSC_EXTERNAL_ALLOC)malloc;
        pPMP->mmParam.pfnReAlloc = (PFN_VSC_EXTERNAL_REALLOC)realloc;
        pPMP->mmParam.pfnFree = (PFN_VSC_EXTERNAL_FREE)free;
    }

    /* Do we need make it thread-safe???? */
    pPMP->id = pmpCounter ++;

    pPMP->lowLimitOfChunkSize = lowLimitOfChunkSize;
    pPMP->alignInSize = alignInSize;

    pPMP->flags.bPooling = bPooling;

#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    pPMP->flags.bPooling = gcvFALSE;
#endif

    vscBILST_Initialize(&pPMP->biChunkChain, gcvFALSE);

    /* Try to create default chunk in this pool */
    if (pPMP->flags.bPooling)
    {
        _CreateNewChunk(pPMP, 0);
    }
    else
    {
        vscBILST_Initialize(&pPMP->nativeAddrChain, gcvFALSE);
    }

    vscMM_Initialize(&pPMP->mmWrapper, pPMP, VSC_MM_TYPE_PMP);

    pPMP->flags.bInitialized = gcvTRUE;

#if ENABLE_PMP_DEBUG_DUMP
    vscERR_ReportWarning(gcvNULL, 0, 0, "PMP %d is initialized", pPMP->id);
#endif
}

void* vscPMP_Alloc(VSC_PRIMARY_MEM_POOL* pPMP, gctUINT reqSize)
{
    VSC_PRIMARY_MEM_CHUNK*   pAllocChunk = gcvNULL;
    VSC_BI_LIST_NODE_EXT*    pThisChunkNode;
    VSC_BI_LIST_NODE_EXT*    pThisNativeAddrNode;
    VSC_COMMON_BLOCK_HEADER* pCmnBlkHeader;
    gctINT                   chunkHeaderSize;
    gctUINT                  alignedSize;
    void*                    retPtr;

    /* If we are not at pooling status, just call underlying alloc */
    if (!pPMP->flags.bPooling)
    {
        retPtr = pPMP->mmParam.pfnAlloc(reqSize);
        gcmASSERT(retPtr);

        pThisNativeAddrNode = (VSC_BI_LIST_NODE_EXT*)pPMP->mmParam.pfnAlloc(sizeof(VSC_BI_LIST_NODE_EXT));
        if (pThisNativeAddrNode)
        {
            vscBLNDEXT_Initialize(pThisNativeAddrNode, retPtr);

            /* Append it at the tail of chunk chain */
            vscBILST_Append(&pPMP->nativeAddrChain, CAST_BLEN_2_BLN(pThisNativeAddrNode));
        }
        else
        {
            /* ERROR */
            gcmASSERT(gcvFALSE);
        }

        return retPtr;
    }

    alignedSize = VSC_UTILS_ALIGN((reqSize + COMMON_BLOCK_HEADER_SIZE), pPMP->alignInSize);

    /* Check whether tail chunk can meet requirement */
    if (PMP_GET_CHUNK_PTR(CAST_BLN_2_BLEN(vscBILST_GetTail(&pPMP->biChunkChain)))->RemainderValidSize < alignedSize)
    {
        /* Search in chunk chain to see which chunk can meet requirement, if yes, pull it to the tail of the chain */
        for (pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain));
             pThisChunkNode != gcvNULL;
             pThisChunkNode = vscBLNDEXT_GetNextNode(pThisChunkNode))
        {
            pAllocChunk = PMP_GET_CHUNK_PTR(pThisChunkNode);
            if (pAllocChunk->RemainderValidSize >= alignedSize)
            {
                /* Remove it from chain */
                vscBILST_Remove(&pPMP->biChunkChain, CAST_BLEN_2_BLN(pThisChunkNode));

                /* Re-append it at the tail of chunk chain */
                vscBILST_Append(&pPMP->biChunkChain, CAST_BLEN_2_BLN(pThisChunkNode));

                break;
            }
        }

        /* If no found, let's create a new chunk */
        if (!pThisChunkNode)
        {
            _CreateNewChunk(pPMP, alignedSize);
        }
    }

    pAllocChunk = PMP_GET_CHUNK_PTR(CAST_BLN_2_BLEN(vscBILST_GetTail(&pPMP->biChunkChain)));

    /* Check whether whole chunk is allocated */
    chunkHeaderSize = VSC_UTILS_ALIGN(sizeof(VSC_PRIMARY_MEM_CHUNK), pPMP->alignInSize);
    if ((gctUINT8*)pAllocChunk + chunkHeaderSize == pAllocChunk->pStartOfRemainderValidData &&
        alignedSize == pAllocChunk->RemainderValidSize)
    {
        pAllocChunk->flags.bWholeChunkAllocated = gcvTRUE;
    }

    /* Now allocate on chunk */
    pAllocChunk->RemainderValidSize -= alignedSize;
    retPtr = pAllocChunk->pStartOfRemainderValidData;
    pAllocChunk->pStartOfRemainderValidData += alignedSize;

    pCmnBlkHeader = (VSC_COMMON_BLOCK_HEADER*)retPtr;
    pCmnBlkHeader->userReqSize = reqSize;

    return (void*)((gctUINT8*)retPtr + COMMON_BLOCK_HEADER_SIZE);
}

void* vscPMP_Realloc(VSC_PRIMARY_MEM_POOL* pPMP, void* pOrgAddress, gctUINT newReqSize)
{
    VSC_PRIMARY_MEM_CHUNK*   pAllocChunk = gcvNULL;
    VSC_BI_LIST_NODE_EXT*    pThisChunkNode;
    VSC_BI_LIST_NODE_EXT*    pThisNativeAddrNode;
    void*                    pThisNativeAddr;
    VSC_COMMON_BLOCK_HEADER* pOrgCmnBlkHeader;
    void*                    pNewAddress = gcvNULL;
    gctINT                   chunkHeaderSize;
    gctUINT                  orgAlignedSize, newAlignedSize, deltaSize;

    if (pOrgAddress == gcvNULL)
    {
        /* If pOrgAddress is NULL, realloc behaves the same way as malloc
         * and allocates a new block of newReqSize bytes */
        return vscPMP_Alloc(pPMP, newReqSize);
    }

    /* If we are not at pooling status, just call underlying alloc */
    if (!pPMP->flags.bPooling)
    {
        for (pThisNativeAddrNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->nativeAddrChain));
             pThisNativeAddrNode != gcvNULL;
             pThisNativeAddrNode = vscBLNDEXT_GetNextNode(pThisNativeAddrNode))
        {
            pThisNativeAddr = vscBLNDEXT_GetContainedUserData(pThisNativeAddrNode);
            gcmASSERT(pThisNativeAddr != gcvNULL);

            if (pThisNativeAddr == pOrgAddress)
            {
                break;
            }
        }

        /* If not, it must be ERROR */
        if (pThisNativeAddrNode == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
        }

        pNewAddress = pPMP->mmParam.pfnReAlloc(pOrgAddress, newReqSize);
        gcmASSERT(pNewAddress);

        /* Save new address */
        vscBSNODE_Initialize(&pThisNativeAddrNode->baseNode, pNewAddress);

        return pNewAddress;
    }

    pOrgCmnBlkHeader = (VSC_COMMON_BLOCK_HEADER*)((gctUINT8*)pOrgAddress - COMMON_BLOCK_HEADER_SIZE);

    /* Just return original if no resize */
    if (newReqSize <= pOrgCmnBlkHeader->userReqSize)
    {
        return pOrgAddress;
    }

    orgAlignedSize = VSC_UTILS_ALIGN((pOrgCmnBlkHeader->userReqSize + COMMON_BLOCK_HEADER_SIZE),
                                     pPMP->alignInSize);
    newAlignedSize = VSC_UTILS_ALIGN((newReqSize + COMMON_BLOCK_HEADER_SIZE), pPMP->alignInSize);
    deltaSize = newAlignedSize - orgAlignedSize;

    /* Firstly check if original allocation is from the end of one of chunks, if yes, check
       whether remainder of that chunk can hold new requested size, if still yes, it is happy
       to allocate from original place and increase block size */
    for (pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain));
         pThisChunkNode != gcvNULL;
         pThisChunkNode = vscBLNDEXT_GetNextNode(pThisChunkNode))
    {
        pAllocChunk = PMP_GET_CHUNK_PTR(pThisChunkNode);
        if ((void*)(pAllocChunk->pStartOfRemainderValidData - orgAlignedSize) ==
            (void*)pOrgCmnBlkHeader)
        {
            if (pAllocChunk->RemainderValidSize >= deltaSize)
            {
                pNewAddress = pOrgAddress;

                pAllocChunk->RemainderValidSize -= deltaSize;
                pAllocChunk->pStartOfRemainderValidData += deltaSize;
                pOrgCmnBlkHeader->userReqSize = newReqSize;

                chunkHeaderSize = VSC_UTILS_ALIGN(sizeof(VSC_PRIMARY_MEM_CHUNK), pPMP->alignInSize);
                if ((gctUINT8*)pAllocChunk + chunkHeaderSize == (gctUINT8*)pOrgCmnBlkHeader &&
                    pAllocChunk->RemainderValidSize == 0)
                {
                    pAllocChunk->flags.bWholeChunkAllocated = gcvTRUE;
                }

                return pNewAddress;
            }
        }
    }

    /* Then just call alloc to alloc a new one, and copy original data to new place */
    pNewAddress = vscPMP_Alloc(pPMP, newReqSize);
    memcpy(pNewAddress, pOrgAddress, pOrgCmnBlkHeader->userReqSize);

    return pNewAddress;
}

void vscPMP_Free(VSC_PRIMARY_MEM_POOL* pPMP, void *pData)
{
    VSC_BI_LIST_NODE_EXT*  pThisNativeAddrNode;
    void*                  pThisNativeAddr;

    if (pPMP->flags.bPooling)
    {
        return;
    }

    /* Check whether it is in native addr chain */
    for (pThisNativeAddrNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->nativeAddrChain));
         pThisNativeAddrNode != gcvNULL;
         pThisNativeAddrNode = vscBLNDEXT_GetNextNode(pThisNativeAddrNode))
    {
        pThisNativeAddr = vscBLNDEXT_GetContainedUserData(pThisNativeAddrNode);
        gcmASSERT(pThisNativeAddr != gcvNULL);

        if (pThisNativeAddr == pData)
        {
            break;
        }
    }

    /* If not, it must be ERROR */
    if (pThisNativeAddrNode == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
    }

    /* Free it now */
    pPMP->mmParam.pfnFree(pData);

    vscBILST_Remove(&pPMP->nativeAddrChain, CAST_BLEN_2_BLN(pThisNativeAddrNode));
    vscBLNDEXT_Finalize(pThisNativeAddrNode);

    /* Free node self */
    pPMP->mmParam.pfnFree(pThisNativeAddrNode);
}

void vscPMP_ForceFreeChunk(VSC_PRIMARY_MEM_POOL* pPMP, void *pChunkValidBase)
{
    VSC_PRIMARY_MEM_CHUNK* pChunkToDelete;
    VSC_PRIMARY_MEM_CHUNK* pChunk;
    VSC_BI_LIST_NODE_EXT*  pThisChunkNode;
    gctINT                 chunkHeaderSize;

    if (!pPMP->flags.bPooling)
    {
        return;
    }

    chunkHeaderSize = VSC_UTILS_ALIGN(sizeof(VSC_PRIMARY_MEM_CHUNK), pPMP->alignInSize);

    pChunkToDelete = (VSC_PRIMARY_MEM_CHUNK*)((gctUINT8*)pChunkValidBase - chunkHeaderSize);

    for (pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain));
         pThisChunkNode != gcvNULL;
         pThisChunkNode = vscBLNDEXT_GetNextNode(pThisChunkNode))
    {
        pChunk = PMP_GET_CHUNK_PTR(pThisChunkNode);

        /* Found */
        if (pChunk == pChunkToDelete)
        {
            _DeleteChunk(pPMP, pChunk);
            break;
        }
    }

    /* Create a default chunk if no chunk is in the chain */
    if (vscBILST_IsEmpty(&pPMP->biChunkChain))
    {
        _CreateNewChunk(pPMP, 0);
    }
}

void vscPMP_ForceFreeAllHugeChunks(VSC_PRIMARY_MEM_POOL* pPMP)
{
    VSC_PRIMARY_MEM_CHUNK* pChunk;
    VSC_BI_LIST_NODE_EXT*  pThisChunkNode;
    VSC_BI_LIST_NODE_EXT*  pNextChunkNode;
    gctINT                 chunkHeaderSize;

    if (!pPMP->flags.bPooling)
    {
        return;
    }

    chunkHeaderSize = VSC_UTILS_ALIGN(sizeof(VSC_PRIMARY_MEM_CHUNK), pPMP->alignInSize);

    for (pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain));
         pThisChunkNode != gcvNULL;
         pThisChunkNode = pNextChunkNode)
    {
        gctUINT thisChunkOriDataSize;

        pChunk = PMP_GET_CHUNK_PTR(pThisChunkNode);

        pNextChunkNode = vscBLNDEXT_GetNextNode(pThisChunkNode);

        thisChunkOriDataSize = (gctUINT)(pChunk->pStartOfRemainderValidData -
                                         (gctUINT8*)pChunk - chunkHeaderSize);

        if (thisChunkOriDataSize >= pPMP->lowLimitOfChunkSize &&
            pChunk->flags.bWholeChunkAllocated)
        {
            _DeleteChunk(pPMP, pChunk);
        }
    }

    /* Create a default chunk if no chunk is in the chain */
    if (vscBILST_IsEmpty(&pPMP->biChunkChain))
    {
        _CreateNewChunk(pPMP, 0);
    }
}

void vscPMP_Finalize(VSC_PRIMARY_MEM_POOL* pPMP)
{
    VSC_PRIMARY_MEM_CHUNK* pChunk;
    VSC_BI_LIST_NODE_EXT*  pThisChunkNode;
    VSC_BI_LIST_NODE_EXT*  pThisNativeAddrNode;
    void*                  pThisNativeAddr;

    /* No need to go on if it was not initialized before */
    if (!pPMP->flags.bInitialized)
    {
        return;
    }

    if (!pPMP->flags.bPooling)
    {
        for (pThisNativeAddrNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->nativeAddrChain));
             pThisNativeAddrNode != gcvNULL;
             pThisNativeAddrNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->nativeAddrChain)))
        {
            pThisNativeAddr = vscBLNDEXT_GetContainedUserData(pThisNativeAddrNode);
            gcmASSERT(pThisNativeAddr != gcvNULL);

            /* Free address allocated by external allocator */
            pPMP->mmParam.pfnFree(pThisNativeAddr);

            vscBILST_Remove(&pPMP->nativeAddrChain, CAST_BLEN_2_BLN(pThisNativeAddrNode));
            vscBLNDEXT_Finalize(pThisNativeAddrNode);

            /* Free node self */
            pPMP->mmParam.pfnFree(pThisNativeAddrNode);
        }

        vscBILST_Finalize(&pPMP->nativeAddrChain);

        return;
    }

    for (pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain));
         pThisChunkNode != gcvNULL;
         pThisChunkNode = CAST_BLN_2_BLEN(vscBILST_GetHead(&pPMP->biChunkChain)))
    {
        pChunk = PMP_GET_CHUNK_PTR(pThisChunkNode);

        _DeleteChunk(pPMP, pChunk);
    }

    vscBILST_Finalize(&pPMP->biChunkChain);

    vscMM_Finalize(&pPMP->mmWrapper);

    pPMP->flags.bInitialized = gcvFALSE;

#if ENABLE_PMP_DEBUG_DUMP
    vscERR_ReportWarning(gcvNULL, 0, 0, "PMP %d is finalized", pPMP->id);
#endif
}

gctUINT vscPMP_GetLowLimitOfChunkSize(VSC_PRIMARY_MEM_POOL* pPMP)
{
    return pPMP->lowLimitOfChunkSize;
}

void vscPMP_PrintStatistics(VSC_PRIMARY_MEM_POOL* pPMP)
{

}

gctBOOL vscPMP_IsInitialized(VSC_PRIMARY_MEM_POOL* pPMP)
{
    return pPMP->flags.bInitialized;
}


