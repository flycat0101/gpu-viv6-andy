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

/*
  Arena memory system uses BMS (buddy memory system) to build a memory arena where multiple
  purpose usages (or same purpose usage with multiple times) can exist async. In most cases,
  it will be used as dummy or scratch memory.
*/

#define AMS_GET_CHUNK_PTR(pChunkListNode)    ((VSC_ARENA_MEM_CHUNK*)vscULNDEXT_GetContainedUserData((pChunkListNode)))

static void _CalcBaseChunkSize(VSC_ARENA_MEM_SYS* pAMS, gctUINT reqSize)
{
    gctUINT alignedSize;

    alignedSize = reqSize + sizeof(VSC_ARENA_MEM_CHUNK) + pAMS->align;
    pAMS->baseChunkSize = vscAlignToPow2(alignedSize, LOG2_MAX_BUDDY_BLOCK_SIZE);
}

static void _InitializeChunk(VSC_ARENA_MEM_SYS* pAMS, VSC_ARENA_MEM_CHUNK* pChunk)
{
    gctINT    alignedChunkHeaderSize;
    gctUINT8* pData = (gctUINT8*)pChunk + sizeof(VSC_ARENA_MEM_CHUNK);

    alignedChunkHeaderSize= (gctINT)(((gctUINT8*)VSC_UTILS_ALIGN((gctUINTPTR_T)pData, pAMS->align)) - ((gctUINT8*)pChunk));

    gcmASSERT(alignedChunkHeaderSize >= 0);
    pChunk->RemainderValidSize = pAMS->baseChunkSize - (gctUINT)alignedChunkHeaderSize;
    pChunk->pStartOfRemainderValidData = (gctUINT8*)pChunk + alignedChunkHeaderSize;
}

static void _CreateNewChunk(VSC_ARENA_MEM_SYS* pAMS)
{
    VSC_ARENA_MEM_CHUNK* pNewChunk;

    pNewChunk = (VSC_ARENA_MEM_CHUNK*)vscBMS_Alloc(pAMS->pBuddyMemSys, pAMS->baseChunkSize);

    /* Fill in contents */
    _InitializeChunk(pAMS, pNewChunk);
    vscULNDEXT_Initialize(&pNewChunk->chunkNode, pNewChunk);

    /* Insert it after current chunk in the chunk chain */
    if (pAMS->pCurChunk)
    {
        vscUNILST_InsertAfter(&pAMS->chunkChain, CAST_ULEN_2_ULN(&pAMS->pCurChunk->chunkNode),
                              CAST_ULEN_2_ULN(&pNewChunk->chunkNode));
    }
    else
    {
        vscUNILST_Append(&pAMS->chunkChain, CAST_ULEN_2_ULN(&pNewChunk->chunkNode));
    }

    pAMS->pCurChunk = pNewChunk;
}

void vscAMS_Initialize(VSC_ARENA_MEM_SYS* pAMS, VSC_BUDDY_MEM_SYS* pBaseMemPool,
                       gctUINT initArenaSize, gctUINT align)
{
    pAMS->pBuddyMemSys = pBaseMemPool;
    pAMS->align = align;
    vscUNILST_Initialize(&pAMS->chunkChain, gcvFALSE);
    _CalcBaseChunkSize(pAMS, initArenaSize);
    _CreateNewChunk(pAMS);

    vscMM_Initialize(&pAMS->mmWrapper, pAMS, VSC_MM_TYPE_AMS);

    pAMS->flags.bInitialized = gcvTRUE;
}

void* vscAMS_Alloc(VSC_ARENA_MEM_SYS* pAMS, gctUINT reqSize)
{
#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    return vscPMP_Alloc(pAMS->pBuddyMemSys->pPriMemPool, reqSize);
#else
    VSC_UNI_LIST_NODE_EXT*   pNextNode;
    VSC_ARENA_MEM_CHUNK*     pNextChunk;
    VSC_COMMON_BLOCK_HEADER* pCmnBlkHeader;
    gctUINT                  reqSizeWithHeader;
    void*                    retPtr;

    reqSizeWithHeader = reqSize + COMMON_BLOCK_HEADER_SIZE;

    if (pAMS->pCurChunk->RemainderValidSize < reqSizeWithHeader)
    {
        pNextNode = vscULNDEXT_GetNextNode(&pAMS->pCurChunk->chunkNode);
        if (pNextNode)
        {
            pNextChunk = AMS_GET_CHUNK_PTR(pNextNode);

            _InitializeChunk(pAMS, pNextChunk);
            pAMS->pCurChunk = pNextChunk;
        }
        else
        {
            _CreateNewChunk(pAMS);
        }
    }

    /* If reqSizeWithHeader is GE baseChunkSize, do we need ask user to increase baseChunkSize or
       arena-internally increase baseChunkSize ??? */
    gcmASSERT(pAMS->pCurChunk->RemainderValidSize >= reqSizeWithHeader);

    /* Now allocate on chunk */
    pAMS->pCurChunk->RemainderValidSize -= reqSizeWithHeader;
    retPtr = pAMS->pCurChunk->pStartOfRemainderValidData;
    pAMS->pCurChunk->pStartOfRemainderValidData += reqSizeWithHeader;

    pCmnBlkHeader = (VSC_COMMON_BLOCK_HEADER*)retPtr;
    pCmnBlkHeader->userReqSize = reqSize;

    return (void*)((gctUINT8*)retPtr + COMMON_BLOCK_HEADER_SIZE);
#endif
}

void* vscAMS_Realloc(VSC_ARENA_MEM_SYS* pAMS, void* pOrgAddress, gctUINT newReqSize)
{
#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    return vscPMP_Realloc(pAMS->pBuddyMemSys->pPriMemPool, pOrgAddress, newReqSize);
#else
    VSC_COMMON_BLOCK_HEADER* pOrgCmnBlkHeader;
    void*                    pNewAddress = gcvNULL;

    if (pOrgAddress == gcvNULL)
    {
        /* If pOrgAddress is NULL, realloc behaves the same way as malloc
         * and allocates a new block of newReqSize bytes */
        return vscAMS_Alloc(pAMS, newReqSize);
    }

    pOrgCmnBlkHeader = (VSC_COMMON_BLOCK_HEADER*)((gctUINT8*)pOrgAddress - COMMON_BLOCK_HEADER_SIZE);

    /* Just return original if no resize */
    if (newReqSize <= pOrgCmnBlkHeader->userReqSize)
    {
        return pOrgAddress;
    }

    /* Just call alloc to alloc a new one, and copy original data to new place */
    pNewAddress = vscAMS_Alloc(pAMS, newReqSize);
    memcpy(pNewAddress, pOrgAddress, pOrgCmnBlkHeader->userReqSize);

    return pNewAddress;
#endif
}

void vscAMS_Reset(VSC_ARENA_MEM_SYS* pAMS)
{
    VSC_ARENA_MEM_CHUNK* pHeadChunk;

    if (!vscUNILST_IsEmpty(&pAMS->chunkChain))
    {
        pHeadChunk = AMS_GET_CHUNK_PTR(CAST_ULN_2_ULEN(vscUNILST_GetHead(&pAMS->chunkChain)));

        _InitializeChunk(pAMS, pHeadChunk);

        /* Reset arena */
        pAMS->pCurChunk = pHeadChunk;
    }
}

void vscAMS_Finalize(VSC_ARENA_MEM_SYS* pAMS)
{
    VSC_ARENA_MEM_CHUNK*    pChunk;
    VSC_UNI_LIST_NODE_EXT*  pThisChunkNode;

    /* No need to go on if it was not initialized before */
    if (!pAMS->flags.bInitialized)
    {
        return;
    }

    for (pThisChunkNode = CAST_ULN_2_ULEN(vscUNILST_GetHead(&pAMS->chunkChain));
         pThisChunkNode != gcvNULL;
         pThisChunkNode = CAST_ULN_2_ULEN(vscUNILST_GetHead(&pAMS->chunkChain)))
    {
        pChunk = AMS_GET_CHUNK_PTR(pThisChunkNode);

        /* Remove it from chain */
        vscUNILST_Remove(&pAMS->chunkChain, CAST_ULEN_2_ULN(&pChunk->chunkNode));
        vscULNDEXT_Finalize(&pChunk->chunkNode);

        /* Delete it now */
        vscBMS_Free(pAMS->pBuddyMemSys, pChunk);
    }

    vscUNILST_Finalize(&pAMS->chunkChain);
    pAMS->baseChunkSize = 0;
    pAMS->pCurChunk = gcvNULL;

    vscMM_Finalize(&pAMS->mmWrapper);

    pAMS->flags.bInitialized = gcvFALSE;
}

void  vscAMS_PrintStatistics(VSC_ARENA_MEM_SYS* pAMS)
{

}

gctBOOL vscAMS_IsInitialized(VSC_ARENA_MEM_SYS* pAMS)
{
    return pAMS->flags.bInitialized;
}


