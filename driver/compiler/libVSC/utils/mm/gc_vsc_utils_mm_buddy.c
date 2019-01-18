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

/*
   Buddy memory system can reduce memory fragments due to frequently allocation and
   free of small blocks. Also buddy memory system can provide memory reuse strategy.
   Note this BMS is built on PMP (primary memory pool system).
*/

#define BMS_GET_BUDDY_BLOCK_PTR(pAvailListNode)    ((pAvailListNode) ? \
                                                    (VSC_BUDDY_MEM_BLOCK_NODE*)vscBLNDEXT_GetContainedUserData((pAvailListNode)) : \
                                                    (VSC_BUDDY_MEM_BLOCK_NODE*)gcvNULL)

/* Global buddy mem sys counter */
static gctINT bmsCounter = 0;

static VSC_BUDDY_MEM_BLOCK_NODE* _AllocInUnderlyingMem(VSC_BUDDY_MEM_SYS* pBMS, gctUINT reqBlockSize)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pRet;

    /* Allocate block */
    pRet = (VSC_BUDDY_MEM_BLOCK_NODE*)vscPMP_Alloc(pBMS->pPriMemPool, reqBlockSize);

    /* If this allocation is a huge allocation, track it */
    if (reqBlockSize > vscPMP_GetLowLimitOfChunkSize(pBMS->pPriMemPool))
    {
        VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP* pHugeAllocNode;

        pHugeAllocNode = (VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP*)vscPMP_Alloc(pBMS->pPriMemPool,
                                                          sizeof(VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP));

        pHugeAllocNode->pBase = pRet;
        vscULNDEXT_Initialize(&pHugeAllocNode->uniHugeAllocNode, pHugeAllocNode);
        vscUNILST_Append(&pBMS->hugeAllocList, CAST_ULEN_2_ULN(&pHugeAllocNode->uniHugeAllocNode));
    }

    return pRet;
}

static VSC_BUDDY_MEM_BLOCK_NODE* _ReallocInUnderlyingMem(VSC_BUDDY_MEM_SYS* pBMS, VSC_BUDDY_MEM_BLOCK_NODE* pOrgBlock,
                                                         gctUINT orgReqBlockSize, gctUINT newReqBlockSize)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pNewRet;

    /* Re-allocate block */
    pNewRet = (VSC_BUDDY_MEM_BLOCK_NODE*)vscPMP_Realloc(pBMS->pPriMemPool, pOrgBlock, newReqBlockSize);

    /* If this allocation is a huge allocation, track it */
    if (newReqBlockSize > vscPMP_GetLowLimitOfChunkSize(pBMS->pPriMemPool) &&
        !((pNewRet == pOrgBlock) && (orgReqBlockSize > vscPMP_GetLowLimitOfChunkSize(pBMS->pPriMemPool))))
    {
        VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP* pHugeAllocNode;

        pHugeAllocNode = (VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP*)vscPMP_Alloc(pBMS->pPriMemPool,
                                                          sizeof(VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP));

        pHugeAllocNode->pBase = pNewRet;
        vscULNDEXT_Initialize(&pHugeAllocNode->uniHugeAllocNode, pHugeAllocNode);
        vscUNILST_Append(&pBMS->hugeAllocList, CAST_ULEN_2_ULN(&pHugeAllocNode->uniHugeAllocNode));
    }

    return pNewRet;
}

static void _DeleteHugeUnderlyingMem(VSC_BUDDY_MEM_SYS* pBMS)
{
    VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP*  pHugeAlloc;
    VSC_UNI_LIST_NODE_EXT*            pThisHugeAllocNode;

    for (pThisHugeAllocNode = CAST_ULN_2_ULEN(vscUNILST_GetHead(&pBMS->hugeAllocList));
         pThisHugeAllocNode != gcvNULL;
         pThisHugeAllocNode = CAST_ULN_2_ULEN(vscUNILST_GetHead(&pBMS->hugeAllocList)))
    {
        pHugeAlloc = (VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP*)vscULNDEXT_GetContainedUserData((pThisHugeAllocNode));
        vscUNILST_Remove(&pBMS->hugeAllocList, CAST_ULEN_2_ULN(&pHugeAlloc->uniHugeAllocNode));
        vscULNDEXT_Finalize(&pHugeAlloc->uniHugeAllocNode);
        vscPMP_ForceFreeChunk(pBMS->pPriMemPool, pHugeAlloc->pBase);
    }

    vscUNILST_Finalize(&pBMS->hugeAllocList);
}

static void _RemoveBlockFromFreeAvailList(VSC_BUDDY_MEM_SYS* pBMS, gctINT log2Size, VSC_BUDDY_MEM_BLOCK_NODE* pBlockToRemove)
{
    vscBILST_Remove(&pBMS->freeAvailList[log2Size], CAST_BLEN_2_BLN(&pBlockToRemove->biBlockNode));
    vscBLNDEXT_Finalize(&pBlockToRemove->biBlockNode);
}

static void _CoalesceFreeBuddyBlocks(VSC_BUDDY_MEM_SYS* pBMS)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pCandidateBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pBuddyBlockOfCandidate;
    VSC_BUDDY_MEM_BLOCK_NODE* pCoalescedBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pNextBlock;
    gctUINT                   blkSize, highHalfBlkSize, i;

    /* No blocks in any level are free'ed before, just bail out */
    if (pBMS->coalesceMask == 0)
    {
        return;
    }

    /* From low level to high level of tree, check whether we can do block-coalescing */
    blkSize = MIN_BUDDY_BLOCK_SIZE;
    for (i = LOG2_MIN_BUDDY_BLOCK_SIZE; i < LOG2_MAX_BUDDY_BLOCK_SIZE; i++)
    {
        /* Can this level be coalesc'able? If yes, try it! */
        if ((1 << i) & pBMS->coalesceMask)
        {
            /* Go through all available free'ed blocks in this level */
            pCandidateBlock = BMS_GET_BUDDY_BLOCK_PTR(CAST_BLN_2_BLEN(vscBILST_GetHead(&pBMS->freeAvailList[i])));
            while (pCandidateBlock)
            {
                /* Only sub block LT original block size can be folded */
                if (i < pCandidateBlock->blkHeader.log2OrgSize)
                {
                    /* Get buddy block of candidate block based on high half or low half */
                    highHalfBlkSize = pCandidateBlock->blkHeader.highHalf & blkSize;
                    pBuddyBlockOfCandidate = (VSC_BUDDY_MEM_BLOCK_NODE*)((gctUINT8*)pCandidateBlock + blkSize - (highHalfBlkSize << 1));

                    /* If buddy is also a free one, fine, let's coalesce them */
                    if (pBuddyBlockOfCandidate->blkHeader.bAllocated == 0 &&
                        pBuddyBlockOfCandidate->blkHeader.log2CurSize == i)
                    {
                        gcmASSERT(pCandidateBlock->blkHeader.log2OrgSize == pBuddyBlockOfCandidate->blkHeader.log2OrgSize);

                        /* Get low-half of entity of these two buddies */
                        pCoalescedBlock = (VSC_BUDDY_MEM_BLOCK_NODE*)((gctUINT8*) pCandidateBlock - highHalfBlkSize);

                        /* Remove candidate self from free-available list */
                        pNextBlock = BMS_GET_BUDDY_BLOCK_PTR(vscBLNDEXT_GetNextNode(&pCandidateBlock->biBlockNode));
                        _RemoveBlockFromFreeAvailList(pBMS, i, pCandidateBlock);
                        pCandidateBlock = pNextBlock;

                        /* Remove buddy from free-available list */
                        pNextBlock = BMS_GET_BUDDY_BLOCK_PTR(vscBLNDEXT_GetNextNode(&pBuddyBlockOfCandidate->biBlockNode));
                        _RemoveBlockFromFreeAvailList(pBMS, i, pBuddyBlockOfCandidate);

                        /* Special case that two buddies are adjacently together */
                        if (pCandidateBlock == pBuddyBlockOfCandidate)
                        {
                            pCandidateBlock = pNextBlock;
                        }

                        /* Insert coalesced block to the head of free-available list with 1 level up */
                        pCoalescedBlock->blkHeader.log2CurSize = i + 1;
                        pCoalescedBlock->blkHeader.bAllocated = 0;
                        vscBLNDEXT_Initialize(&pCoalescedBlock->biBlockNode, pCoalescedBlock);
                        vscBILST_Prepend(&pBMS->freeAvailList[i + 1], CAST_BLEN_2_BLN(&pCoalescedBlock->biBlockNode));

                        /* New coalesced block can also be as candidate to do up-level coalescing */
                        pBMS->coalesceMask |= 1 << (i + 1);
                    }
                    else
                    {
                        pCandidateBlock = BMS_GET_BUDDY_BLOCK_PTR(vscBLNDEXT_GetNextNode(&pCandidateBlock->biBlockNode));
                    }
                }
                else
                {
                    pCandidateBlock = BMS_GET_BUDDY_BLOCK_PTR(vscBLNDEXT_GetNextNode(&pCandidateBlock->biBlockNode));
                }
            }
        }

        /* Go to up level of tree */
        blkSize <<= 1;
    }

    /* Yes, we have coalesced all candidates, so we reset coalesce-mask to uncoalesc'able status for all levels */
    pBMS->coalesceMask = 0;
}

/* Coalesce start block with free buddy blocks between level range [pStartBlock->blkHeader.log2CurSize, upReqLog2Size) */
static VSC_BUDDY_MEM_BLOCK_NODE* _CoalesceBlocks(VSC_BUDDY_MEM_SYS* pBMS, VSC_BUDDY_MEM_BLOCK_NODE* pStartBlock, gctINT upReqLog2Size)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pCandidateBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pHighBuddyOfCandidate;
    gctUINT                   blkSize, highHalfBlkSize, i;
    gctBOOL                   bCoalescable = gcvTRUE;

    gcmASSERT(pStartBlock->blkHeader.log2CurSize < (gctUINT)upReqLog2Size);

    /* Firstly check whether we can coalesce these levels? */
    pCandidateBlock = pStartBlock;
    blkSize = (1 << pStartBlock->blkHeader.log2CurSize);
    for (i = (gctUINT)pStartBlock->blkHeader.log2CurSize; i < (gctUINT)upReqLog2Size; i ++)
    {
        /* Only sub block LT original block size can be folded */
        if (i >= pCandidateBlock->blkHeader.log2OrgSize)
        {
            bCoalescable = gcvFALSE;
            break;
        }

        /* Only low-part can be as candidate */
        highHalfBlkSize = pCandidateBlock->blkHeader.highHalf & blkSize;
        if (highHalfBlkSize)
        {
            bCoalescable = gcvFALSE;
            break;
        }

        /* Only when high buddy is a free one, then we can go on */
        pHighBuddyOfCandidate = (VSC_BUDDY_MEM_BLOCK_NODE*)((gctUINT8*)pCandidateBlock + blkSize);
        if (!(pHighBuddyOfCandidate->blkHeader.bAllocated == 0 &&
            pHighBuddyOfCandidate->blkHeader.log2CurSize == i))
        {
            bCoalescable = gcvFALSE;
            break;
        }

        /* Go to up level of tree */
        blkSize <<= 1;
    }

    /* If we can, just coalesce them */
    if (bCoalescable)
    {
        pCandidateBlock = pStartBlock;
        blkSize = (1 << pStartBlock->blkHeader.log2CurSize);
        for (i = (gctUINT)pStartBlock->blkHeader.log2CurSize; i < (gctUINT)upReqLog2Size; i ++)
        {
            /* Get high-part buddy of candidate */
            pHighBuddyOfCandidate = (VSC_BUDDY_MEM_BLOCK_NODE*)((gctUINT8*)pCandidateBlock + blkSize);
            gcmASSERT(pCandidateBlock->blkHeader.log2OrgSize == pHighBuddyOfCandidate->blkHeader.log2OrgSize);

            /* Remove candidate self from free-available list if candidate is a free one */
            if (!pCandidateBlock->blkHeader.bAllocated)
            {
                _RemoveBlockFromFreeAvailList(pBMS, i, pCandidateBlock);
            }

            /* Remove buddy from free-available list */
            _RemoveBlockFromFreeAvailList(pBMS, i, pHighBuddyOfCandidate);

            /* Mark it has been coalesced */
            pCandidateBlock->blkHeader.log2CurSize = i + 1;

            /* If coalesced one is freed, just insert it to free-available list */
            if (!pCandidateBlock->blkHeader.bAllocated)
            {
                /* Insert coalesced block to the head of free-available list with 1 level up */
                vscBLNDEXT_Initialize(&pCandidateBlock->biBlockNode, pCandidateBlock);
                vscBILST_Prepend(&pBMS->freeAvailList[i + 1], CAST_BLEN_2_BLN(&pCandidateBlock->biBlockNode));

                /* New coalesced block can also be as candidate to do up-level coalescing */
                pBMS->coalesceMask |= 1 << (i + 1);
            }

            /* Go to up level of tree */
            blkSize <<= 1;
        }

        return pCandidateBlock;
    }

    return gcvNULL;
}

static VSC_BUDDY_MEM_BLOCK_NODE* _FindSmallestBlockByReqLog2Size(VSC_BUDDY_MEM_SYS* pBMS, gctINT reqLog2Size)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pResBlock;
    gctINT                    log2Size;

    pResBlock = gcvNULL;
    log2Size = reqLog2Size;
    while (log2Size <= LOG2_MAX_BUDDY_BLOCK_SIZE)
    {
        if (!vscBILST_IsEmpty(&pBMS->freeAvailList[log2Size]))
        {
            /* OK, we find the corresponding free-available list. Let's get head as the result */
            pResBlock = BMS_GET_BUDDY_BLOCK_PTR(CAST_BLN_2_BLEN(vscBILST_GetHead(&pBMS->freeAvailList[log2Size])));

            /* Remove result (head) from free-available list */
            _RemoveBlockFromFreeAvailList(pBMS, log2Size, pResBlock);

            gcmASSERT(pResBlock->blkHeader.log2CurSize == (gctUINT)log2Size);

            break;
        }

        /* Go to up level of tree */
        log2Size ++;
    }

    return pResBlock;
}

static VSC_BUDDY_MEM_BLOCK_NODE* _AllocInternal(VSC_BUDDY_MEM_SYS* pBMS, gctINT reqLog2Size)
{
    VSC_BUDDY_MEM_BLOCK_NODE* pResBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pLowHalfBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pHighHalfBlock;
    gctUINT                   blkSize;
    gctINT                    log2Size;

    /* 1. Try to find the smallest available block GE requested log2-size in free-available list */
    pResBlock = _FindSmallestBlockByReqLog2Size(pBMS, reqLog2Size);

    /* 2. If we can not find in 1st step, then we hope coalescing free blocks can be helpful, so try again */
    if (pResBlock == gcvNULL)
    {
        _CoalesceFreeBuddyBlocks(pBMS);
        pResBlock = _FindSmallestBlockByReqLog2Size(pBMS, reqLog2Size);
    }

    /* 3. If we still can not find it. Let's allocate a new block for this */
    if (pResBlock == gcvNULL)
    {
        /* We always allocate block GE log(2, 18), so it can hold many small blocks */
        log2Size = (reqLog2Size <= LOG2_DEFAULT_BUDDY_BLOCK_SIZE) ? LOG2_DEFAULT_BUDDY_BLOCK_SIZE : reqLog2Size;
        blkSize = (1 << log2Size);

        /* Now call underlying PMP to allocate the block */
        pResBlock = _AllocInUnderlyingMem(pBMS, blkSize);

        pResBlock->blkHeader.bAllocated = 0; /* Only PMP allocate it, BMS has not finished allocating it */
        pResBlock->blkHeader.log2CurSize = log2Size;
        pResBlock->blkHeader.log2OrgSize = log2Size;
        pResBlock->blkHeader.highHalf = 0;  /* Suppose all halves are low part */

        pBMS->bytesAvailable += blkSize;
    }

    gcmASSERT(pResBlock);
    log2Size = pResBlock->blkHeader.log2CurSize;

    /* 4. Split off part of excess above size into low level small blocks, and add them to free-available list */
    while (log2Size > reqLog2Size)
    {
        /* Go to low level of tree */
        log2Size--;
        blkSize = (1 << log2Size);

        /* Low-half (left-half) will be always added into free-available list  Note from step 1 and step 2, we
           can draw conclusion that levels among [reqLog2Size, log2Size) must have no free-available blocks. */
        pLowHalfBlock = pResBlock;
        pLowHalfBlock->blkHeader.log2CurSize = log2Size;
        vscBLNDEXT_Initialize(&pLowHalfBlock->biBlockNode, pLowHalfBlock);
        gcmASSERT(vscBILST_IsEmpty(&pBMS->freeAvailList[log2Size]));
        vscBILST_Prepend(&pBMS->freeAvailList[log2Size], CAST_BLEN_2_BLN(&pLowHalfBlock->biBlockNode));

        /* High-half (right-half) is continuous with low-half in memory. For high-half, we must mark this half
           locates in high (right) side of this log2Size level. */
        pHighHalfBlock = (VSC_BUDDY_MEM_BLOCK_NODE*)((gctUINT8*)pLowHalfBlock + blkSize);
        pHighHalfBlock->blkHeader = pLowHalfBlock->blkHeader;
        pHighHalfBlock->blkHeader.highHalf |= blkSize;

        /* High-half is always as potential result block */
        pResBlock = pHighHalfBlock;
    }

    /* Mark the block has been allocated by BMS */
    pResBlock->blkHeader.bAllocated = 1;

    return pResBlock;
}

static VSC_BUDDY_MEM_BLOCK_NODE* _TryToReallocOnSite(VSC_BUDDY_MEM_SYS* pBMS, VSC_BUDDY_MEM_BLOCK_NODE* pOrgBlock,
                                                     gctINT orgReqLog2Size, gctINT newReqLog2Size)
{
    gcmASSERT(pOrgBlock->blkHeader.log2CurSize == (gctUINT)orgReqLog2Size);

    /* 1. Check block has margin to fill extra size */
    if (newReqLog2Size == orgReqLog2Size)
    {
        return pOrgBlock;
    }

    /* 2. Try to coalesce high-part buddies to get bigger block */
    return _CoalesceBlocks(pBMS, pOrgBlock, newReqLog2Size);
}

static gctINT _FindSmallestBlockSizeInLog2(gctUINT reqBlkSize)
{
    gctINT                    log2Size;
    gctUINT                   blkSize;

    log2Size = LOG2_MIN_BUDDY_BLOCK_SIZE;
    blkSize = MIN_BUDDY_BLOCK_SIZE;
    while (blkSize < reqBlkSize)
    {
        /* Go to up level of tree */
        blkSize <<= 1;
        log2Size ++;
    }

    return log2Size;
}

void vscBMS_Initialize(VSC_BUDDY_MEM_SYS* pBMS, VSC_PRIMARY_MEM_POOL* pBaseMemPool)
{
    gctINT i;

    gcmASSERT(pBaseMemPool);

    pBMS->pPriMemPool = pBaseMemPool;

    /* Do we need make it thread-safe???? */
    pBMS->id = bmsCounter ++;

    for (i = 0; i <= LOG2_MAX_BUDDY_BLOCK_SIZE; i++)
    {
        vscBILST_Initialize(&pBMS->freeAvailList[i], gcvFALSE);
    }

    pBMS->coalesceMask = 0;

    vscUNILST_Initialize(&pBMS->hugeAllocList, gcvFALSE);

    pBMS->bytesInUse = 0;
    pBMS->maxBytesInUse = 0;
    pBMS->bytesAvailable = 0;

    pBMS->bytesOverSized = 0;
    pBMS->overSizedAllocatedTimes = 0;
    pBMS->overSizedFreedTimes = 0;

    vscMM_Initialize(&pBMS->mmWrapper, pBMS, VSC_MM_TYPE_BMS);

    pBMS->flags.bInitialized = gcvTRUE;
}

void* vscBMS_Alloc(VSC_BUDDY_MEM_SYS* pBMS, gctUINT reqSize)
{
#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    return vscPMP_Alloc(pBMS->pPriMemPool, reqSize);
#else
    VSC_BUDDY_MEM_BLOCK_NODE* pResBlock;
    gctUINT                   reqSizeWithHeader;
    gctINT                    log2Size;

    reqSizeWithHeader = min_bm_node_size(reqSize);

    /* For the case buddy system can maintain */
    if (reqSizeWithHeader <= MAX_BUDDY_BLOCK_SIZE)
    {
        /* Find the smallest 2^n block size that can just hold requested size */
        log2Size = _FindSmallestBlockSizeInLog2(reqSizeWithHeader);

        /* Do true allocation with buddy algorithm */
        pResBlock = _AllocInternal(pBMS, log2Size);

        pBMS->bytesInUse += (1 << log2Size);
        pBMS->bytesAvailable -= (1 << log2Size);
        if (pBMS->bytesInUse > pBMS->maxBytesInUse)
        {
            pBMS->maxBytesInUse = pBMS->bytesInUse;
        }
    }
    /* For the case buddy system can not maintain because the block is over-sized */
    else
    {
        /* Directly allocate by PMP */
        pResBlock = _AllocInUnderlyingMem(pBMS, reqSizeWithHeader);

        pResBlock->blkHeader.bAllocated = 0;
        pResBlock->blkHeader.highHalf = 0;
        pResBlock->blkHeader.log2CurSize = LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE;
        pResBlock->blkHeader.log2OrgSize = LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE;

        pBMS->bytesOverSized += reqSizeWithHeader;
        pBMS->overSizedAllocatedTimes ++;
    }

    pResBlock->blkHeader.cmnBlkHeader.userReqSize = reqSize;

    /* Return true memory content by shifting out of block header */
    return (void*)(&pResBlock->biBlockNode);
#endif
}

void* vscBMS_Realloc(VSC_BUDDY_MEM_SYS* pBMS, void* pOrgAddress, gctUINT newReqSize)
{
#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    return vscPMP_Realloc(pBMS->pPriMemPool, pOrgAddress, newReqSize);
#else
    VSC_BUDDY_MEM_BLOCK_NODE* pOrgBlock;
    VSC_BUDDY_MEM_BLOCK_NODE* pNewBlock;
    void*                     pNewAddress = gcvNULL;
    gctUINT                   orgReqSizeWithHeader, newReqSizeWithHeader;
    gctINT                    orgLog2Size, newLog2Size;

    if (pOrgAddress == gcvNULL)
    {
        /* If pOrgAddress is NULL, realloc behaves the same way as malloc
         * and allocates a new block of newReqSize bytes */
        return vscBMS_Alloc(pBMS, newReqSize);
    }

    /* Retrieve block corresponding to requested memory content */
    pOrgBlock = userDataToBmNode(pOrgAddress);

    /* Just return original if no resize */
    if (newReqSize <= pOrgBlock->blkHeader.cmnBlkHeader.userReqSize)
    {
        return pOrgAddress;
    }

    orgReqSizeWithHeader = min_bm_node_size(pOrgBlock->blkHeader.cmnBlkHeader.userReqSize);
    newReqSizeWithHeader = min_bm_node_size(newReqSize);

    if (newReqSizeWithHeader > MAX_BUDDY_BLOCK_SIZE)
    {
        if (orgReqSizeWithHeader <= MAX_BUDDY_BLOCK_SIZE)
        {
            /* Original was allocated by BMS, but new request will be allocated by PMP, so just
               release original allocation by BMS and allocate new one by PMP */

            pNewAddress = vscBMS_Alloc(pBMS, newReqSize);
            memcpy(pNewAddress, pOrgAddress, pOrgBlock->blkHeader.cmnBlkHeader.userReqSize);
            vscBMS_Free(pBMS, pOrgAddress);
            return pNewAddress;
        }
        else
        {
            /* Both original and new are all oversized, so call PMP's realloc */

            pNewBlock = _ReallocInUnderlyingMem(pBMS, pOrgBlock, orgReqSizeWithHeader, newReqSizeWithHeader);
            if (pNewBlock != pOrgBlock)
            {
                pNewBlock->blkHeader.bAllocated = 0;
                pNewBlock->blkHeader.highHalf = 0;
                pNewBlock->blkHeader.log2CurSize = LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE;
                pNewBlock->blkHeader.log2OrgSize = LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE;

                pBMS->overSizedAllocatedTimes ++;
                pBMS->bytesOverSized += newReqSizeWithHeader;
            }
            else
            {
                pBMS->bytesOverSized += (newReqSizeWithHeader - orgReqSizeWithHeader);
            }

            pNewAddress = (void*)(&pNewBlock->biBlockNode);
            pNewBlock->blkHeader.cmnBlkHeader.userReqSize = newReqSize;
            return pNewAddress;
        }
    }
    else
    {
        /* Both original and new can be allocated by BMS, so try to reuse allocation as possible as we can */

        orgLog2Size = _FindSmallestBlockSizeInLog2(orgReqSizeWithHeader);
        newLog2Size = _FindSmallestBlockSizeInLog2(newReqSizeWithHeader);
        pNewBlock = _TryToReallocOnSite(pBMS, pOrgBlock, orgLog2Size, newLog2Size);

        if (pNewBlock)
        {
            gcmASSERT(pNewBlock == pOrgBlock);

            pBMS->bytesInUse += ((1 << newLog2Size) - (1 << orgLog2Size));
            pBMS->bytesAvailable -= ((1 << newLog2Size) - (1 << orgLog2Size));
            if (pBMS->bytesInUse > pBMS->maxBytesInUse)
            {
                pBMS->maxBytesInUse = pBMS->bytesInUse;
            }

            pNewAddress = (void*)(&pNewBlock->biBlockNode);
            pNewBlock->blkHeader.cmnBlkHeader.userReqSize = newReqSize;
            return pNewAddress;
        }
        else
        {
            pNewAddress = vscBMS_Alloc(pBMS, newReqSize);
            memcpy(pNewAddress, pOrgAddress, pOrgBlock->blkHeader.cmnBlkHeader.userReqSize);
            vscBMS_Free(pBMS, pOrgAddress);
            return pNewAddress;
        }
    }
#endif
}

void vscBMS_Free(VSC_BUDDY_MEM_SYS* pBMS, void *pData)
{
#if ENABLE_EXTERNAL_MEM_TOOL_CHECK
    return vscPMP_Free(pBMS->pPriMemPool, pData);
#else
    VSC_BUDDY_MEM_BLOCK_NODE* pBlockToDelete;
    int                       log2Size;

    if (pData != gcvNULL)
    {
        /* Retrieve block corresponding to requested memory content */
        pBlockToDelete = userDataToBmNode(pData);

        log2Size = pBlockToDelete->blkHeader.log2CurSize;

        /* For the case buddy system can maintain */
        if (log2Size != LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE)
        {
            gcmASSERT(log2Size >= LOG2_MIN_BUDDY_BLOCK_SIZE && log2Size <= LOG2_MAX_BUDDY_BLOCK_SIZE);
            gcmASSERT(pBlockToDelete->blkHeader.bAllocated != 0);

            pBMS->bytesInUse -= (1 << log2Size);
            pBMS->bytesAvailable += (1 << log2Size);

            /* Insert this block to the head of free-available list */
            vscBLNDEXT_Initialize(&pBlockToDelete->biBlockNode, pBlockToDelete);
            vscBILST_Prepend(&pBMS->freeAvailList[log2Size], CAST_BLEN_2_BLN(&pBlockToDelete->biBlockNode));

            /* Mart it as free'ed by BMS. Note PMP has not free'ed it */
            pBlockToDelete->blkHeader.bAllocated = 0;

            /* Mask this block can be coalesc'able with its buddy on level log2Size later */
            pBMS->coalesceMask |= (1 << log2Size);
        }
        /* For the case buddy system can not maintain because the block is over-sized */
        else
        {
            gcmASSERT(pBlockToDelete->blkHeader.log2OrgSize == LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE);

            /* Although over-sized block is not maintained by buddy sys, it also is not going to be
               deleted by underlying primary memory pool (PMP) by default. PMP will delete it when
               all chunks are deleted or explicitly deleted by doing finalizing by explicitly calling
               vscBMS_Finalize. So just record we have hit a over-sized memory release. */
            pBMS->overSizedFreedTimes ++;
        }
    }
#endif
}

void vscBMS_Finalize(VSC_BUDDY_MEM_SYS* pBMS, gctBOOL bDeleteHugeUnderlyingMem)
{
    gctINT i;

    /* No need to go on if it was not initialized before */
    if (!pBMS->flags.bInitialized)
    {
        return;
    }

    for (i = 0; i <= LOG2_MAX_BUDDY_BLOCK_SIZE; i++)
    {
        vscBILST_Finalize(&pBMS->freeAvailList[i]);
    }

    /* Delete huge PMP mem */
    if (bDeleteHugeUnderlyingMem)
    {
        _DeleteHugeUnderlyingMem(pBMS);
    }

    vscMM_Finalize(&pBMS->mmWrapper);

    pBMS->flags.bInitialized = gcvFALSE;
}

void vscBMS_PrintStatistics(VSC_BUDDY_MEM_SYS* pBMS)
{
}

gctBOOL vscBMS_IsInitialized(VSC_BUDDY_MEM_SYS* pBMS)
{
    return pBMS->flags.bInitialized;
}


