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


#ifndef __gc_vsc_utils_mm_h_
#define __gc_vsc_utils_mm_h_

BEGIN_EXTERN_C()

#define JUNK_BYTE    0xcd
#define ZERO_BYTE    0x00
#define FULL_BYTE    0xff

/* When pooling, we can not use external tools, to detect memory issues located in
   all non-MM modules. So this flag will let VSC be mm-checkable by external tools.
   With this flag enabled, pooling will auto be closed, and all alloc/realloc/free
   in each level of MM will be directly use versions of c-lib. */
#define ENABLE_EXTERNAL_MEM_TOOL_CHECK 0

/* External alloc and free routines which do true allocation and free operation */
typedef void* (*PFN_VSC_EXTERNAL_ALLOC)(gctSIZE_T reqSize);
typedef void* (*PFN_VSC_EXTERNAL_REALLOC)(void*pOrgAddress, gctSIZE_T newReqSize);
typedef void  (*PFN_VSC_EXTERNAL_FREE)(void* pAddress);

typedef struct _VSC_MEMORY_MANAGEMENT_PARAM
{
    PFN_VSC_EXTERNAL_ALLOC   pfnAlloc;
    PFN_VSC_EXTERNAL_REALLOC pfnReAlloc;
    PFN_VSC_EXTERNAL_FREE    pfnFree;
}VSC_MEMORY_MANAGEMENT_PARAM;

/* Common block header to record user allocation info. A block is an allocation unit that
   memory system maintains. Note that we must design this header as small as we can */
typedef struct _VSC_COMMON_BLOCK_HEADER
{
    /* User requested allocation size */
    gctUINT                   userReqSize;
}VSC_COMMON_BLOCK_HEADER;

/* sizeof(VSC_COMMON_BLOCK_HEADER) */
#define COMMON_BLOCK_HEADER_SIZE  sizeof(VSC_COMMON_BLOCK_HEADER)

typedef struct _VSC_PRIMARY_MEM_POOL VSC_PRIMARY_MEM_POOL;
typedef struct _VSC_BUDDY_MEM_SYS    VSC_BUDDY_MEM_SYS;
typedef struct _VSC_ARENA_MEM_SYS    VSC_ARENA_MEM_SYS;

typedef void* (*PFN_VSC_MM_ALLOC)(void* pMemSys, gctUINT reqSize);
typedef void* (*PFN_VSC_MM_REALLOC)(void* pMemSys, void* pOrgAddress, gctUINT newReqSize);
typedef void  (*PFN_VSC_MM_FREE)(void* pMemSys, void *pData);

/*
 * A Common MM wrapper for most used functions of all mem system types
 */

typedef enum _VSC_MM_TYPE
{
    VSC_MM_TYPE_PMP   = 0,
    VSC_MM_TYPE_BMS,
    VSC_MM_TYPE_AMS
}VSC_MM_TYPE;

/* A common structure for memory management for use of outside world. */
typedef struct _VSC_MM
{
    VSC_MM_TYPE               mmType;
    union MEMSYS
    {
        void*                 pMemSys;
        VSC_PRIMARY_MEM_POOL* pPMP;
        VSC_BUDDY_MEM_SYS*    pBMS;
        VSC_ARENA_MEM_SYS*    pAMS;
    }ms;
}VSC_MM;

void vscMM_Initialize(VSC_MM* pMM, void* pMemSys, VSC_MM_TYPE mmType);
void* vscMM_Alloc(VSC_MM* pMM, gctUINT reqSize);
/* Note new address may be different with original, so all POINTER based info should be NOTICED!!!!!!! */
void* vscMM_Realloc(VSC_MM* pMM, void* pOrgAddress, gctUINT newReqSize);
void vscMM_Free(VSC_MM* pMM, void *pData);
void vscMM_Finalize(VSC_MM* pMM);

/*
 * Primary memory pool definition
 */

typedef struct _VSC_PRIMARY_MEM_CHUNK
{
    struct
    {
        gctUINT32                  bWholeChunkAllocated : 1;
        gctUINT32                  reserved             : 31;
    } flags;

    /* Note remainder valid data always excludes this chunk structure */
    gctUINT8*                      pStartOfRemainderValidData;
    gctUINT                        RemainderValidSize;

    /* Chunk list element of this chunk corresponds */
    VSC_BI_LIST_NODE_EXT           biChunkNode;
}VSC_PRIMARY_MEM_CHUNK;

struct _VSC_PRIMARY_MEM_POOL
{
    struct
    {
        gctUINT32                  bPooling     : 1; /* Are we pooling? */
        gctUINT32                  bInitialized : 1; /* Is PMP initialized? */
        gctUINT32                  reserved     : 30;
    } flags;

    VSC_MEMORY_MANAGEMENT_PARAM    mmParam;

    /* PMP global id */
    gctUINT                        id;

    /* Size of all chunks must be GE it */
    gctUINT                        lowLimitOfChunkSize;

    /* Alignment size. Note this is size alignment, not address alignment */
    gctUINT                        alignInSize;

    /* Chunk chain maintained by PMP. Note we always allocate new memory from tail chunk */
    VSC_BI_LIST                    biChunkChain;

    /* Maintain all allocated addrs by vscPMP_Alloc/vscPMP_Realloc when not pooling. It can
       eliminate memory leak when user miss calling vscPMP_Free, but it will give negative
       performance impact due to chain search. So generally, we should always use pooling,
       but when debugging some memory issues, we may need turn off pooling. */
    VSC_BI_LIST                    nativeAddrChain;

    /* Wrapper of this PMP for common use */
    VSC_MM                         mmWrapper;
};

/* Initialize primary memory pool. If user does not specify MM-param, then C-lib routines will be used */
void  vscPMP_Intialize(VSC_PRIMARY_MEM_POOL* pPMP, VSC_MEMORY_MANAGEMENT_PARAM* pMMParam,
                       gctUINT lowLimitOfChunkSize, gctUINT alignInSize, gctBOOL bPooling);

/* Allocate memory from PMP */
void* vscPMP_Alloc(VSC_PRIMARY_MEM_POOL* pPMP, gctUINT reqSize);

/* Re-allocate memory from PMP */
void* vscPMP_Realloc(VSC_PRIMARY_MEM_POOL* pPMP, void* pOrgAddress, gctUINT newReqSize);

/* It is only for unpooling case */
void vscPMP_Free(VSC_PRIMARY_MEM_POOL* pPMP, void *pData);

/* DONT RASHLY CALL IT!!!!! It is originally designed to release whole chunk if the chunk requested by
   BMS is larger than lowLimitOfChunkSize. See vscBMS_Finalize. If other users try to use it, they must
   assure all contents in chunk won't be used anymore */
void  vscPMP_ForceFreeChunk(VSC_PRIMARY_MEM_POOL* pPMP, void *pChunkValidBase);

/* DONT RASHLY CALL IT!!!!! Huge chunk here is the chunk whose size is GE lowLimitOfChunkSize and whole
   chunk is allocated by user to use. If other users try to use it, they must assure these huge chunks
   won't be used anymore */
void  vscPMP_ForceFreeAllHugeChunks(VSC_PRIMARY_MEM_POOL* pPMP);

/* Finalize PMP, all chunks will be deleted at once */
void  vscPMP_Finalize(VSC_PRIMARY_MEM_POOL* pPMP);

/* Print statistics info for user's debug */
void  vscPMP_PrintStatistics(VSC_PRIMARY_MEM_POOL* pPMP);

gctUINT vscPMP_GetLowLimitOfChunkSize(VSC_PRIMARY_MEM_POOL* pPMP);

/* Check whether PMP has been initialized, if yes, we can do allocation/deallocation now */
gctBOOL vscPMP_IsInitialized(VSC_PRIMARY_MEM_POOL* pPMP);

/*
 * Buddy memory system definition
 */

#define LOG2_MIN_BUDDY_BLOCK_SIZE         5
#define LOG2_MAX_BUDDY_BLOCK_SIZE         24
#define LOG2_DEFAULT_BUDDY_BLOCK_SIZE     18
#define LOG2_UNSUPPORTED_BUDDY_BLOCK_SIZE 0xFF /* For those over-sized blocks */
#define MIN_BUDDY_BLOCK_SIZE              (1 << LOG2_MIN_BUDDY_BLOCK_SIZE)
#define MAX_BUDDY_BLOCK_SIZE              (1 << LOG2_MAX_BUDDY_BLOCK_SIZE)

typedef struct _VSC_BUDDY_MEM_BLOCK_NODE
{
    /* Every buddy memory block has following buddy block header, so user requested memory size must be added
       by extra BUDDY_BLOCK_HEADER_SIZE when entering buddy system */
    struct BUDDY_BLOCK_HEADER
    {
        VSC_COMMON_BLOCK_HEADER cmnBlkHeader;     /* Only valid when block is allocated, no matte what BMS (normal) or
                                                     PMP  (oversized) allocates it. */
        gctUINT                 bAllocated  : 1;  /* Set if block is allocated by BMS, not by PMP, so for over-sized
                                                     block, it is always 0 since it is only allocated by PMP, not BMS */
        gctUINT                 highHalf    : 31; /* Low (0) or high (1) half of block, one bit per level of tree. For
                                                     certain block node in tree, this info must be certain. This is
                                                     topology info. Not meaningful for over-sized block */
        gctUINT                 log2CurSize : 16; /* Current block size after split, it must be LE log2OrgSize, for
                                                     over-sized block, must be set to LOG2_UNSUPPORTED_BLOCK_SIZE */
        gctUINT                 log2OrgSize : 16; /* Original allocated block size, for over-sized block, must be set
                                                     to LOG2_UNSUPPORTED_BLOCK_SIZE */
    }blkHeader;

    /* Following is only used when free, its space will be as part of space of user requested memory when
       bAllocated is TRUE (i.e, when the block is removed from free-available list) */
    VSC_BI_LIST_NODE_EXT        biBlockNode;
}VSC_BUDDY_MEM_BLOCK_NODE;

/* sizeof(BUDDY_BLOCK_HEADER) */
#define BUDDY_BLOCK_HEADER_SIZE           sizeof(struct BUDDY_BLOCK_HEADER)

typedef struct _VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP
{
    void*                     pBase;
    VSC_UNI_LIST_NODE_EXT     uniHugeAllocNode;
}VSC_BUDDY_MEM_HUGE_ALLOC_IN_PMP;

struct _VSC_BUDDY_MEM_SYS
{
    struct
    {
        gctUINT32                  bInitialized : 1; /* Is PMP initialized? */
        gctUINT32                  reserved     : 31;
    } flags;

    /* Underlying memory pool */
    VSC_PRIMARY_MEM_POOL*     pPriMemPool;

    /* Buddy global id */
    gctUINT                   id;

    /* Free-available list that buddy sys maintains */
    VSC_BI_LIST               freeAvailList[LOG2_MAX_BUDDY_BLOCK_SIZE + 1];

    /* Indicate which blocks can be coalesced */
    gctUINT                   coalesceMask;

    /* List of huge allocation in PMP */
    VSC_UNI_LIST              hugeAllocList;

    /* Followings are used for statistics info of maintainable blocks */
    gctUINT                   bytesInUse;
    gctUINT                   maxBytesInUse;
    gctUINT                   bytesAvailable;

    /* Followings are used for statistics info of over-sized blocks */
    gctUINT                   bytesOverSized;
    gctINT                    overSizedAllocatedTimes;
    gctINT                    overSizedFreedTimes;

    /* Wrapper of this BMS for common use */
    VSC_MM                    mmWrapper;
};

/* Initialize buddy mem sys */
void  vscBMS_Initialize(VSC_BUDDY_MEM_SYS* pBMS, VSC_PRIMARY_MEM_POOL* pBaseMemPool);

/* Allocate memory on buddy sys */
void* vscBMS_Alloc(VSC_BUDDY_MEM_SYS* pBMS, gctUINT reqSize);

/* Re-allocate memory on buddy sys */
void* vscBMS_Realloc(VSC_BUDDY_MEM_SYS* pBMS, void* pOrgAddress, gctUINT newReqSize);

/* Free memory that allocated by vscBMS_Alloc */
void  vscBMS_Free(VSC_BUDDY_MEM_SYS* pBMS, void *pData);

/* Finalize buddy mem sys. All mem allocated in PMP that larger than lowLimitOfChunkSize will be deleted
   if bDeleteHugeUnderlyingMem is set to TRUE. But bDeleteHugeUnderlyingMem should be FALSE in general,
   unless memory footprint used by BMS is very huge */
void  vscBMS_Finalize(VSC_BUDDY_MEM_SYS* pBMS, gctBOOL bDeleteHugeUnderlyingMem);

/* Print statistics info for user's debug */
void  vscBMS_PrintStatistics(VSC_BUDDY_MEM_SYS* pBMS);

/* Check whether BMS has been initialized, if yes, we can do allocation/deallocation now */
gctBOOL vscBMS_IsInitialized(VSC_BUDDY_MEM_SYS* pBMS);

/*
 * Arena memory system definition
 */

typedef struct _VSC_ARENA_MEM_CHUNK
{
    /* Note remainder valid data always excludes this chunk structure, as well as alignment */
    gctUINT8*                      pStartOfRemainderValidData;
    gctUINT                        RemainderValidSize;

    /* Chunk list node of this chunk corresponds */
    VSC_UNI_LIST_NODE_EXT          chunkNode;
}VSC_ARENA_MEM_CHUNK;

struct _VSC_ARENA_MEM_SYS
{
    struct
    {
        gctUINT32                  bInitialized : 1; /* Is PMP initialized? */
        gctUINT32                  reserved     : 31;
    } flags;

    /* Underlying memory pool */
    VSC_BUDDY_MEM_SYS*        pBuddyMemSys;

    /* Alignment size. This is not only size alignment, but address alignment */
    gctUINT                   align;

    /* Auto-allocated alloc size of chunk, including chunk header */
    gctUINT                   baseChunkSize;

    /* Chunk chain maintained by AMS. Note we always allocate new memory from tail chunk */
    VSC_UNI_LIST              chunkChain;

    /* Current chunk which is used to allocate */
    VSC_ARENA_MEM_CHUNK*      pCurChunk;

    /* Wrapper of this AMS for common use */
    VSC_MM                    mmWrapper;
};

/* Initialize arena mem sys */
void  vscAMS_Initialize(VSC_ARENA_MEM_SYS* pAMS, VSC_BUDDY_MEM_SYS* pBaseMemPool,
                        gctUINT initArenaSize, gctUINT align);

/* Allocate memory on arena sys */
void* vscAMS_Alloc(VSC_ARENA_MEM_SYS* pAMS, gctUINT reqSize);

/* Re-allocate memory on arena sys */
void* vscAMS_Realloc(VSC_ARENA_MEM_SYS* pAMS, void* pOrgAddress, gctUINT newReqSize);

/* All chunks will not be deleted, just move (reset) current chunk to head, so all chunks can be used later */
void  vscAMS_Reset(VSC_ARENA_MEM_SYS* pAMS);

/* Finalize arena mem sys. */
void  vscAMS_Finalize(VSC_ARENA_MEM_SYS* pAMS);

/* Print statistics info for user's debug */
void  vscAMS_PrintStatistics(VSC_ARENA_MEM_SYS* pAMS);

/* Check whether AMS has been initialized, if yes, we can do allocation/deallocation now */
gctBOOL vscAMS_IsInitialized(VSC_ARENA_MEM_SYS* pAMS);

END_EXTERN_C()

#endif /* __gc_vsc_utils_mm_h_ */


