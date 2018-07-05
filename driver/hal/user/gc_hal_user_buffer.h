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


#ifndef __gc_hal_user_buffer_h_
#define __gc_hal_user_buffer_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
************************ Command Buffer and Event Objects **********************
\******************************************************************************/

#define gcdRENDER_FENCE_LENGTH                      (6 * gcmSIZEOF(gctUINT32))
#define gcdBLT_FENCE_LENGTH                         (10 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_FLUSHCACHE_LENGTH               (2 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_PAUSE_OQ_LENGTH                 (2 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_PAUSE_XFBWRITTEN_QUERY_LENGTH   (4 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_PAUSE_PRIMGEN_QUERY_LENGTH      (4 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_PAUSE_XFB_LENGTH                (2 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_HW_FENCE_32BIT                  (4 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_HW_FENCE_64BIT                  (6 * gcmSIZEOF(gctUINT32))
#define gcdRESERVED_PAUSE_PROBE_LENGTH              (TOTAL_PROBE_NUMBER * 2 * gcmSIZEOF(gctUINT32))

#define gcdRESUME_OQ_LENGTH                         (2 * gcmSIZEOF(gctUINT32))
#define gcdRESUME_XFBWRITTEN_QUERY_LENGTH           (4 * gcmSIZEOF(gctUINT32))
#define gcdRESUME_PRIMGEN_QUERY_LENGTH              (4 * gcmSIZEOF(gctUINT32))
#define gcdRESUME_XFB_LENGH                         (2 * gcmSIZEOF(gctUINT32))
#define gcdRESUME_PROBE_LENGH                       (TOTAL_PROBE_NUMBER * 2 * gcmSIZEOF(gctUINT32))


/* State delta record. */
typedef struct _gcsSTATE_DELTA_RECORD * gcsSTATE_DELTA_RECORD_PTR;
typedef struct _gcsSTATE_DELTA_RECORD
{
    /* State address. */
    gctUINT                     address;

    /* State mask. */
    gctUINT32                   mask;

    /* State data. */
    gctUINT32                   data;
}
gcsSTATE_DELTA_RECORD;

/* State delta. */
typedef struct _gcsSTATE_DELTA
{
    /* For debugging: the number of delta in the order of creation. */
    gctUINT                     num;

    /* Main state delta ID. Every time state delta structure gets reinitialized,
       main ID is incremented. If main state ID overflows, all map entry IDs get
       reinitialized to make sure there is no potential erroneous match after
       the overflow.*/
    gctUINT                     id;

    /* Vertex element count for the delta buffer. */
    gctUINT                     elementCount;

    /* Number of states currently stored in the record array. */
    gctUINT                     recordCount;

    /* Record array; holds all modified states in gcsSTATE_DELTA_RECORD. */
    gctUINT64                   recordArray;
    gctUINT                     recordSize;

    /* Map entry ID is used for map entry validation. If map entry ID does not
       match the main state delta ID, the entry and the corresponding state are
       considered not in use. */
    gctUINT64                   mapEntryID;
    gctUINT                     mapEntryIDSize;

    /* If the map entry ID matches the main state delta ID, index points to
       the state record in the record array. */
    gctUINT64                   mapEntryIndex;
}
gcsSTATE_DELTA;

#define gcdPATCH_LIST_SIZE      1024

/* Command buffer patch record. */
typedef struct _gcsPATCH
{
    /* Handle of a video memory node. */
    gctUINT32                   handle;

    /* Flag */
    gctUINT32                   flag;
}
gcsPATCH;

/* List of patches for the command buffer. */
typedef struct _gcsPATCH_LIST
{
    /* Array of patch records. */
    struct _gcsPATCH            patch[gcdPATCH_LIST_SIZE];

    /* Number of patches in the array. */
    gctUINT                     count;

    /* Next item in the list. */
    struct _gcsPATCH_LIST       *next;
}
gcsPATCH_LIST;

#define FENCE_NODE_LIST_INIT_COUNT         100

typedef struct _gcsFENCE_APPEND_NODE
{
    gcsSURF_NODE_PTR    node;
    gceFENCE_TYPE       type;

}gcsFENCE_APPEND_NODE;

typedef gcsFENCE_APPEND_NODE   *   gcsFENCE_APPEND_NODE_PTR;

typedef struct _gcsFENCE_LIST    *   gcsFENCE_LIST_PTR;

typedef struct _gcsFENCE_LIST
{
    /* Resource that need get fence, but command used this resource not generated */
    gcsFENCE_APPEND_NODE_PTR        pendingList;
    gctUINT                         pendingCount;
    gctUINT                         pendingAllocCount;

    /* Resoure that already generated command in this command buffer but not get fence */
    gcsFENCE_APPEND_NODE_PTR        onIssueList;
    gctUINT                         onIssueCount;
    gctUINT                         onIssueAllocCount;
}
gcsFENCE_LIST;

/* Command buffer object. */
struct _gcoCMDBUF
{
    /* The object. */
    gcsOBJECT                   object;

    /* Commit count. */
    gctUINT64                   commitCount;

    /* Command buffer entry and exit pipes. */
    gcePIPE_SELECT              entryPipe;
    gcePIPE_SELECT              exitPipe;

    /* Feature usage flags. */
    gctBOOL                     using2D;
    gctBOOL                     using3D;

    /* Size of reserved tail for each commit. */
    gctUINT32                   reservedTail;

    /* Physical address of command buffer. Just a name. */
    gctUINT32                   physical;

    /* Logical address of command buffer. */
    gctUINT64                   logical;

    /* Number of bytes in command buffer. */
    gctUINT32                   bytes;

    /* Start offset into the command buffer. */
    gctUINT32                   startOffset;

    /* Current offset into the command buffer. */
    gctUINT32                   offset;

    /* Number of free bytes in command buffer. */
    gctUINT32                   free;

    /* Location of the last reserved area. */
    gctUINT64                   lastReserve;
    gctUINT32                   lastOffset;

#if gcdSECURE_USER
    /* Hint array for the current command buffer. */
    gctUINT                     hintArraySize;
    gctUINT64                   hintArray;
    gctUINT64                   hintArrayTail;
#endif

    /* Last load state command location and hardware address. */
    gctUINT64                   lastLoadStatePtr;
    gctUINT32                   lastLoadStateAddress;
    gctUINT32                   lastLoadStateCount;

    /* List of patches. */
    gctUINT64                   patchHead;

    /* Link to next gcoCMDBUF object in one commit. */
    gctUINT64                   nextCMDBUF;

    /*
    * Put pointer type member after this line.
    */

    /* Completion signal. */
    gctSIGNAL                   signal;

    /* Link to the siblings. */
    gcoCMDBUF                   prev;
    gcoCMDBUF                   next;

    /* Mirror command buffer(s). */
    gcoCMDBUF                   *mirrors;
    gctUINT32                   mirrorCount;
};

typedef struct _gcsChunkHead * gcsChunkHead_PTR;
struct _gcsChunkHead
{
    gcsChunkHead_PTR next;
};

typedef struct _gcsQUEUE
{
    /* Pointer to next gcsQUEUE structure in gcsQUEUE. */
    gctUINT64                   next;

    /* Event information. */
    gcsHAL_INTERFACE            iface;
}
gcsQUEUE;

/* Event queue. */
struct _gcoQUEUE
{
    /* The object. */
    gcsOBJECT                   object;

    /* Pointer to current event queue. */
    gcsQUEUE_PTR                head;
    gcsQUEUE_PTR                tail;

    /* chunks of the records. */
    gctPOINTER                  chunks;

    /* List of free records. */
    gcsQUEUE_PTR                freeList;

    #define gcdIN_QUEUE_RECORD_LIMIT 16
    /* Number of records currently in queue */
    gctUINT32                   recordCount;

    /* Max size of pending unlock node in vidmem pool not committed */
    gctUINT                     maxUnlockBytes;

    gceENGINE                   engine;
};

struct _gcsTEMPCMDBUF
{
    gctUINT32 currentByteSize;
    gctPOINTER buffer;
    gctBOOL  inUse;
};

typedef struct _gcoWorkerInfo
{
    gctBOOL                     commit;
    gctSIGNAL                   signal;

    gctPOINTER                  mutex;
    gcoBUFFER                   buffer;

    gceHARDWARE_TYPE            hardwareType;
    gctUINT32                   currentCoreIndex;

    gctBOOL                     emptyBuffer;
    gcoCMDBUF                   commandBuffer;

    gcsSTATE_DELTA_PTR          stateDelta;
    gctUINT32                   context;
    gctUINT32_PTR               contexts;
    gcoQUEUE                    queue;
}
gcoWorkerInfo;

gceSTATUS
gcoSuspendWorker(
    gcoBUFFER Buffer
    );

gceSTATUS
gcoResumeWorker(
    gcoBUFFER Buffer
    );

gcoWorkerInfo*
gcoGetWorker(
    gcoQUEUE Queue,
    gcoBUFFER Buffer,
    gctBOOL EmptyBuffer
    );

gctBOOL
gcoSubmitWorker(
    gcoBUFFER Buffer,
    gcoWorkerInfo* Worker,
    gctBOOL Stall
    );

typedef struct {
    gctUINT inputBase;
    gctUINT count;
    gctUINT outputBase;
}
gcsSTATEMIRROR;

extern const gcsSTATEMIRROR mirroredStates[];
extern gctUINT mirroredStatesCount;

/* Update the state delta. */
static gcmINLINE void UpdateStateDelta(
    IN gcsSTATE_DELTA_PTR StateDelta,
    IN gctUINT32 Address,
    IN gctUINT32 Mask,
    IN gctUINT32 Data
    )
{
    gcsSTATE_DELTA_RECORD_PTR recordArray;
    gcsSTATE_DELTA_RECORD_PTR recordEntry;
    gctUINT32_PTR mapEntryID;
    gctUINT32_PTR mapEntryIndex;
    gctUINT deltaID;
    gctUINT32 i;

    /* Get the current record array. */
    recordArray = (gcsSTATE_DELTA_RECORD_PTR)(gcmUINT64_TO_PTR(StateDelta->recordArray));

    /* Get shortcuts to the fields. */
    deltaID       = StateDelta->id;
    mapEntryID    = (gctUINT32_PTR)(gcmUINT64_TO_PTR(StateDelta->mapEntryID));
    mapEntryIndex = (gctUINT32_PTR)(gcmUINT64_TO_PTR(StateDelta->mapEntryIndex));

    gcmASSERT(Address < (StateDelta->mapEntryIDSize / gcmSIZEOF(gctUINT)));

    for (i = 0; i < mirroredStatesCount; i++)
    {
        if ((Address >= mirroredStates[i].inputBase) &&
            (Address < (mirroredStates[i].inputBase + mirroredStates[i].count)))
        {
            Address = mirroredStates[i].outputBase + (Address - mirroredStates[i].inputBase);
            break;
        }
    }

    /* Has the entry been initialized? */
    if (mapEntryID[Address] != deltaID)
    {
        /* No, initialize the map entry. */
        mapEntryID    [Address] = deltaID;
        mapEntryIndex [Address] = StateDelta->recordCount;

        /* Get the current record. */
        recordEntry = &recordArray[mapEntryIndex[Address]];

        /* Add the state to the list. */
        recordEntry->address = Address;
        recordEntry->mask    = Mask;
        recordEntry->data    = Data;

        /* Update the number of valid records. */
        StateDelta->recordCount += 1;
    }

    /* Regular (not masked) states. */
    else if (Mask == 0)
    {
        /* Get the current record. */
        recordEntry = &recordArray[mapEntryIndex[Address]];

        /* Update the state record. */
        recordEntry->mask = 0;
        recordEntry->data = Data;
    }

    /* Masked states. */
    else
    {
        /* Get the current record. */
        recordEntry = &recordArray[mapEntryIndex[Address]];

        /* Update the state record. */
        recordEntry->mask |=  Mask;
        recordEntry->data &= ~Mask;
        recordEntry->data |= (Data & Mask);
    }
}

#if (gcdENABLE_3D || gcdENABLE_2D)
gceSTATUS ResetStateDelta(
    IN gcsSTATE_DELTA_PTR StateDelta
    );

gceSTATUS MergeStateDelta(
    IN gcsSTATE_DELTA_PTR DestStateDelta,
    IN gcsSTATE_DELTA_PTR SrcStateDelta
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_user_buffer_h_ */
