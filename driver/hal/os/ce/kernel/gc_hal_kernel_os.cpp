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


#include <windows.h>
#include <ceddk.h>
#include "gc_hal_kernel.h"
#include "gc_hal_kernel_ce.h"

#define _GC_OBJ_ZONE    gcvZONE_OS

#define gcd_GCPAGEHEAP_MAX_CACHE_NODE   5

#ifdef MEMORY_STAT
static gctSIZE_T    SpecialHeapMemoryCount  = 0;
static gctSIZE_T    HeapPageCount           = 0;
static gctSIZE_T    NonPagedMemoryCount     = 0;
static gctSIZE_T    PagedMemoryCount        = 0;
#endif // MEMORY_STAT

/******************************************************************************\
**********************************  Local   ************************************
\******************************************************************************/
typedef struct _gcsOSTIMER * gcsOSTIMER_PTR;
typedef struct _gcsOSTIMER
{
    UINT                    timerID;
    gctTIMERFUNCTION        function;
    gctPOINTER              data;
} gcsOSTIMER;


/*******************************************************************************
***** Version Signature *******************************************************/

#define _gcmNUM2STR(t)  #t
#define gcmNUM2STR(t)   _gcmNUM2STR(t)
const char * PLATFORM = "\n\0$PLATFORM$Windows CE "
                        gcmNUM2STR(UNDER_CE / 100) "."
                        gcmNUM2STR(UNDER_CE % 100) "$\n";

#if UNDER_CE >= 600
static gctBOOL IsAddressInUserSpace(IN gctPOINTER Pointer)
{
    return ((gctUINT)Pointer < 0x80000000);
}
#endif

// Heap code for pages.
class GCPAGEHEAP
{
protected:
    // Number of pages per heap.
    static const int c_Pages = 1024; // 4MB

    // Number of nodes.
    static int m_node;

    // Logical address of allocated page heap.
    gctPOINTER  m_Logical;

    // Physical startig and edning address of allocated page heap.
    gctUINT m_Physical, m_PhysicalEnd;

    // Bit array for pages.
    gctUINT32   m_Usage[c_Pages / 32];

    // Pointer to next page heap.
    GCPAGEHEAP* m_Next;

    // Pointer to GCHAL.
    GCHAL* m_Hal;

public:
    // Constructor.
    GCPAGEHEAP(
        GCHAL* hal
        )
        : m_Hal(hal)
    {
        // No next heap.
        m_Next = gcvNULL;

        // Allocate the pages.
        PHYSICAL_ADDRESS physical;
        if (!m_Hal->AllocateMemory(c_Pages * PAGE_SIZE,
                                   m_Logical,
                                   physical))
        {
            // Throw error.
            throw false;
        }

        // Copy physical address.
        m_Physical = physical.LowPart;

        // Compute the ending physical address of the allocated page heap.
        m_PhysicalEnd = m_Physical + c_Pages * PAGE_SIZE;

        // Mark all the pages as free.
        memset(m_Usage, 0, sizeof(m_Usage));

        ++m_node;
    }

    // Destructor.
    ~GCPAGEHEAP(
        void
        )
    {
        // Free next heap if avaiable.
        if (m_Next != gcvNULL)
        {
            // Free linked page heap.
            delete m_Next;

#ifdef MEMORY_STAT
            SpecialHeapMemoryCount -= sizeof(GCPAGEHEAP);
#endif // MEMORY_STAT
        }

        // Free the pages.
        gcmkVERIFY(VirtualFree(m_Logical, c_Pages * PAGE_SIZE, MEM_DECOMMIT));
        gcmkVERIFY(VirtualFree(m_Logical, 0, MEM_RELEASE));

        --m_node;
    }

    // Allocate a number of pages.
    bool AllocatePages(
        gctINT PageCount,
        PHYSICAL_ADDRESS& Physical,
        gctPOINTER& Logical
        )
    {
        if (PageCount > GCPAGEHEAP::c_Pages)
        {
            // Out of memory.
            return false;
        }

        // Compute requested mask for 32 or less pages.
        gctUINT32 requestedMask = (PageCount <= 32)
                                   ? (0xFFFFFFFF >> (32 - PageCount))
                                   : 0;

        GCPAGEHEAP* heap = gcvNULL;
        GCPAGEHEAP* last = this;

        // 32 or less pages?
        if (requestedMask != 0)
        {
            // Walk all page heaps.
            for (heap = last = this; heap != gcvNULL; heap = heap->m_Next)
            {
                // Walk allpages, 32 at a time.
                for (gctINT index = 0; index < c_Pages / 32; ++index)
                {
                    // Set initial mask.
                    LARGE_INTEGER mask;
                    mask.LowPart  = requestedMask;
                    mask.HighPart = 0;

                    // Walk all 32 pages inside this region.
                    for (gctINT page = 0; page < 32; page++)
                    {
                        // Are the lower pages free?
                        if (!(heap->m_Usage[index] & mask.LowPart) &&
                            // Verify for any upper pages.
                            ((mask.HighPart == 0) ||
                             ((index + 1 < c_Pages / 32) &&
                              !(heap->m_Usage[index + 1] & mask.HighPart))))
                        {
                            // Mark lower pages as used.
                            heap->m_Usage[index] |= mask.LowPart;

                            // Mark upper pages as used.
                            if (mask.HighPart != 0)
                            {
                                heap->m_Usage[index + 1] |= mask.HighPart;
                            }

                            // Compute byte offset of first page.
                            gctUINT32 offset = (index * 32 + page) * PAGE_SIZE;

                            // Return physical and logical addresses.
                            Physical.QuadPart = heap->m_Physical + offset;
                            Logical           = (gctUINT8*) heap->m_Logical + offset;

#ifdef MEMORY_STAT
                            HeapPageCount += PageCount;
#endif // MEMORY_STAT

                            // Success.
                            return true;
                        }

                        // Shift the mask;
                        mask.QuadPart <<= 1;
                    }
                }

                // Save last heap used.
                last = heap;
            }
        }

        try
        {
            // Allocate a new page heap.
            heap = new GCPAGEHEAP(m_Hal);

            if (heap == gcvNULL)
            {
                // Out of memory.
                return false;
            }

            heap->m_Next = last->m_Next;
            last->m_Next = heap;

#ifdef MEMORY_STAT
            SpecialHeapMemoryCount += sizeof(GCPAGEHEAP);
#endif // MEMORY_STAT
        }
        catch(bool)
        {
            // Out of memory.
            return false;
        }

#ifdef MEMORY_STAT
        HeapPageCount += PageCount;
#endif // MEMORY_STAT

        // Start at beginning of page heap.
        gctINT index = 0;

        // Loop while there are 32 or more pages.
        while (PageCount >= 32)
        {
            // Mark all 32 pages in this array entry as used.
            heap->m_Usage[index++] = 0xFFFFFFFF;

            // Reduce count by 32 pages.
            PageCount -= 32;
        }

        // Process final pages.
        if (PageCount > 0)
        {
            // Mark pages as used.
            heap->m_Usage[index] = (0xFFFFFFFF >> (32 - PageCount));
        }

        // Return physical and logical addresses.
        Physical.QuadPart = heap->m_Physical;
        Logical           = heap->m_Logical;

        // Success.
        return true;
    }

    bool FreePages(
        gctINT PageCount,
        PHYSICAL_ADDRESS Physical
        )
    {
        GCPAGEHEAP* heap;

#ifdef MEMORY_STAT
        HeapPageCount -= PageCount;
#endif // MEMORY_STAT

        // Walk all page heaps.
        for (heap = this; heap != gcvNULL; heap = heap->m_Next)
        {
            // Does the request belong to this heap?
            if ((Physical.LowPart >= heap->m_Physical) &&
                ((Physical.LowPart + PageCount * PAGE_SIZE) <= heap->m_PhysicalEnd))
            {
                // Walk all pages.
                UINT page  = (Physical.LowPart - heap->m_Physical) / PAGE_SIZE;
                UINT index = page / 32;
                UINT mask  = 1 << (page & 31);

                while (PageCount-- > 0)
                {
                    // Mark page as free.
                    heap->m_Usage[index] &= ~mask;

                    // Next page.
                    mask <<= 1;
                    if (mask == 0)
                    {
                        mask = 1;
                        ++index;
                    }
                }

                break;
            }
        }

        if (heap == gcvNULL)
        {
            gcmkFATAL("FreePages: free unknown physical memory 0x%08x\n", Physical.LowPart);

            return FALSE;
        }

        if (m_node > gcd_GCPAGEHEAP_MAX_CACHE_NODE)
        {
            GCPAGEHEAP* pre = gcvNULL;

            for (heap = this; heap != gcvNULL; pre = heap, heap = heap->m_Next)
            {
                UINT index;
                gctBOOL del = gcvTRUE;

                for (index = 0; index < gcmCOUNTOF(heap->m_Usage); index++)
                {
                    if (heap->m_Usage[index] != 0)
                    {
                        del = gcvFALSE;
                        break;
                    }
                }

                if (del && (heap != this))
                {
                    pre->m_Next = heap->m_Next;
                    heap->m_Next = gcvNULL;
                    delete heap;

                    if (m_node == gcd_GCPAGEHEAP_MAX_CACHE_NODE)
                    {
                        break;
                    }
                }
            }
        }

        return TRUE;
    }
};

int GCPAGEHEAP::m_node = 0;

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/

struct _gckOS
{
    // Object.
    gcsOBJECT                   object;

    // Locking mutex.
    gctPOINTER                  mutex;

    // Locking mutex for interrupt.
    gctPOINTER                  interruptMutex;

    // Heap.
    gckHEAP                     heap;

    // Pointer to GCHAL class.
    GCHAL *                     gchal;

    // Page heap.
    GCPAGEHEAP *                pageHeap;

#ifdef MEMORY_STAT
    gcsOBJ_LIST                 objList;
#endif // MEMORY_STAT

    TIMECAPS                    timerCaps;

    PHYSICAL_ADDRESS            paddingPagePhys;
    gctPOINTER                  paddingPageLogc;
};

#if UNDER_CE >= 600
#define gcmOS_LOCK(Os)      gcmkVERIFY_OK(gckOS_AcquireMutex((Os), (Os)->mutex, gcvINFINITE))
#define gcmOS_UNLOCK(Os)    gcmkVERIFY_OK(gckOS_ReleaseMutex((Os), (Os)->mutex))
#else
#define gcmOS_LOCK(Os)
#define gcmOS_UNLOCK(Os)
#endif


#ifdef MEMORY_STAT
extern "C" void
OnProcessExit(
    IN gckOS Os
    )
{
    gcsOBJ_NODE * node;

    gcmOS_LOCK(Os);

    gcmTRACE(0,
            "\n************************************\n"
            "SpecialHeapMemoryCount: 0x%x\n"
            "HeapPageCount: 0x%x\n"
            "NonPagedMemoryCount: 0x%x\n"
            "PagedMemoryCount: 0x%x\n"
            "\n",
            SpecialHeapMemoryCount, HeapPageCount, NonPagedMemoryCount, PagedMemoryCount);

    FOR_EACH_OBJ_NODE(&Os->objList, node)
    {
        gcmTRACE(0,
                "Obj: %d\n", node->id);
    }

    gcmTRACE(0,
            "************************************\n");

    gcmOS_UNLOCK(Os);
}
#endif // MEMORY_STAT

/*******************************************************************************
**
**  gckOS_Construct
**
**  Construct a new gckOS object.
**
**  INPUT:
**
**      gctPOINTER Context
**          Pointer to the mapped register space.
**
**  OUTPUT:
**
**      gckOS * Os
**          Pointer to a variable that will hold the pointer to the gckOS object.
*/
gceSTATUS gckOS_Construct(
    IN gctPOINTER Context,
    OUT gckOS * Os
    )
{
    gckOS os;

    gcmkHEADER_ARG("Context=0x%X", Context);

    // Verify the arguments.
    gcmkVERIFY_ARGUMENT(Os != gcvNULL);

    // Allocate the gckOS object.
    os = (gckOS) LocalAlloc(LMEM_FIXED, sizeof(struct _gckOS));

    if (os == gcvNULL)
    {
        // Out of memory.
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    gckOS_ZeroMemory(os, sizeof(struct _gckOS));

    // Initialize the gckOS object.
    os->object.type = gcvOBJ_OS;

    // Create the mutex.
    gcmkVERIFY_OK(gckOS_CreateMutex(os, &os->mutex));

    // Create the interrupt mutex.
    gcmkVERIFY_OK(gckOS_CreateMutex(os, &os->interruptMutex));

    // Set device extension.
    os->gchal = (GCHAL *) Context;

    // Allocate page heap.
    os->pageHeap = new GCPAGEHEAP(os->gchal);

    os->paddingPageLogc = gcvNULL;
    os->paddingPagePhys.LowPart = ~0U;
    gcmkVERIFY(os->pageHeap->AllocatePages(
        1,
        os->paddingPagePhys,
        os->paddingPageLogc));

#ifdef MEMORY_STAT
    SpecialHeapMemoryCount += sizeof(GCPAGEHEAP);
#endif // MEMORY_STAT

    // IMPORTANT! No heap yet.
    os->heap = gcvNULL;

    // Create the gckHEAP object.
    gcmkVERIFY_OK(gckHEAP_Construct(os, gcdHEAP_SIZE, &os->heap));

    if (timeGetDevCaps(&os->timerCaps, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        os->timerCaps.wPeriodMin = 1;
        os->timerCaps.wPeriodMax = ~0U;
    }

#ifdef MEMORY_STAT
    gcsOBJ_LIST_Initialize(&os->objList);
#endif // MEMORY_STAT

    // Return pointer to the gckOS object.
    *Os = os;

    // Success.
    gcmkFOOTER_ARG("*Os=0x%X", *Os);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Destroy
**
**  Destroy an gckOS object.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object that needs to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_Destroy(
    IN gckOS Os
    )
{
    gckHEAP heap;

    gcmkHEADER_ARG("Os=0x%X", Os);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    if (Os->heap != gcvNULL)
    {
        // Mark gckHEAP as gone.
        heap = Os->heap;
        Os->heap = gcvNULL;

        // Destroy the gckHEAP object.
        gcmkVERIFY_OK(gckHEAP_Destroy(heap));
    }

    // Delete page heap.
#ifdef MEMORY_STAT
    SpecialHeapMemoryCount -= sizeof(GCPAGEHEAP);
#endif // MEMORY_STAT

    if (Os->paddingPageLogc)
    {
        Os->pageHeap->FreePages(1, Os->paddingPagePhys);
        Os->paddingPageLogc = gcvNULL;
        Os->paddingPagePhys.LowPart = ~0u;
    }

    delete Os->pageHeap;

    // Free the interrupt mutex.
    gcmkVERIFY_OK(gckOS_DeleteMutex(Os, Os->interruptMutex));

    // Free the mutex.
    gcmkVERIFY_OK(gckOS_DeleteMutex(Os, Os->mutex));

    // Mark the gckOS object as unknown.
    Os->object.type = gcvOBJ_UNKNOWN;

    // Free the gckOS object.
    LocalFree(Os);

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Allocate
**
**  Allocate memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the allocated memory location.
*/
gceSTATUS gckOS_Allocate(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gctPOINTER memory;
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Bytes=%lu", Os, Bytes);

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_OS,
               "[ENTER] gckOS_Allocate");

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    // Do we have a heap?
    if (Os->heap != gcvNULL)
    {
        gcmOS_LOCK(Os);

        // Allocate from the heap.
        status = gckHEAP_Allocate(Os->heap, Bytes, Memory);

        gcmOS_UNLOCK(Os);
    }

    else
    {
        // Allocate memory from OS pool.
        memory = LocalAlloc(LMEM_FIXED, Bytes);

        if (memory == gcvNULL)
        {
            // Out of memory.
            status = gcvSTATUS_OUT_OF_MEMORY;
        }

        else
        {
            // Return pointer to the memory allocation.
            *Memory = memory;

            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                       "gckOS_Allocate: allocated %u bytes at %p",
                       Bytes, memory);

            // Success.
            status = gcvSTATUS_OK;
        }
    }

    if(status != gcvSTATUS_OK)
    {
        gckOS_DebugTrace(gcvLEVEL_ERROR, "ERROR: %s(%d) Allocate memory failed.", __FUNCTION__, __LINE__);
    }

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_OS,
               "[LEAVE] gckOS_Allocate(%u)",
               status);

    // Return the status.
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_Free
**
**  Free allocated memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Memory
**          Pointer to memory allocation to free.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_Free(
    IN gckOS Os,
    IN gctPOINTER Memory
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Memory=0x%X", Os, Memory);

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_OS,
               "[ENTER] gckOS_Free");

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);

    // Do we have a heap?
    if (Os->heap != gcvNULL)
    {
        gcmOS_LOCK(Os);

        // Free from the heap.
        status = gckHEAP_Free(Os->heap, Memory);

        gcmOS_UNLOCK(Os);
    }
    else
    {
        // Free the memory from the OS pool.
        status = (LocalFree(Memory) == gcvNULL)
                     ? gcvSTATUS_OK
                     : gcvSTATUS_INVALID_ARGUMENT;

        if (gcmIS_SUCCESS(status))
        {
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                       "gckOS_Free: Freed %p",
                       Memory);
        }
    }

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_OS,
               "[LEAVE] gckOS_Free(%u)",
               status);

    // Return the status.
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_MapMemory
**
**  Map physical memory into the current process.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to map.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the logical address of the
**          mapped memory.
*/
gceSTATUS gckOS_MapMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    // Pointer to our physical structure.
    gcsPHYSICAL * physical = (gcsPHYSICAL *) Physical;

    gcmkVERIFY_ARGUMENT(Bytes <= physical->bytes);

    // Map address in user.
    status = Os->gchal->MapInUser(physical, Logical);

    gcmkFOOTER_ARG("*Logical=0x%X", gcmOPT_VALUE(Logical));

    return status;
}

/*******************************************************************************
**
**  gckOS_UnmapMemory
**
**  Unmap physical memory out of the current process.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**      gctPOINTER Memory
**          Pointer to a previously mapped memory region.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_UnmapMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu Logical=0x%X",
                   Os, Physical, Bytes, Logical);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    // Pointer to our physical structure.
    gcsPHYSICAL * physical = (gcsPHYSICAL *) Physical;

    gcmkVERIFY_ARGUMENT(Bytes <= physical->bytes);

    // Unmap address from user.
    status = Os->gchal->UnmapFromUser(physical, Logical);

    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_UnmapUserLogical
**
**  Unmap user logical memory out of physical memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Start of physical address memory.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**      gctPOINTER Memory
**          Pointer to a previously mapped memory region.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapUserLogical(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocateNonPagedMemory
**
**  Allocate a number of pages from non-paged memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE if the pages need to be mapped into user space.
**
**      gctUINT32 Flag
**          Allocation attribute.
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that holds the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that hold the number of bytes allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that will hold the physical address of the
**          allocation.
**
**      gctPOINTER * Logical
**          Pointer to a variable that will hold the logical address of the
**          allocation.
*/
gceSTATUS gckOS_AllocateNonPagedMemory(
    IN gckOS Os,
    IN gctBOOL InUserSpace,
    IN gctUINT32 Flag,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL lock = gcvFALSE;
    gcsPHYSICAL* physical = gcvNULL;

    gcmkHEADER_ARG("Os=0x%X InUserSpace=%d *Bytes=%lu",
                   Os, InUserSpace, gcmOPT_VALUE(Bytes));

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Logical || !InUserSpace);
    gcmkVERIFY_ARGUMENT(*Bytes > 0);

    // Create a new gcsPHYSICAL structure.
    physical = new gcsPHYSICAL;
    if (physical == gcvNULL)
    {
        // Out of memory.
        gckOS_DebugTrace(gcvLEVEL_ERROR, "ERROR: %s(%d) Could not allocate gcsPHYSICAL structure.", __FUNCTION__, __LINE__);
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

#ifdef MEMORY_STAT
    SpecialHeapMemoryCount += sizeof(gcsPHYSICAL);
#endif // MEMORY_STAT

    // Align bytes to page size.
    physical->bytes = gcmALIGN(*Bytes, PAGE_SIZE);

    gcmOS_LOCK(Os);
    lock = gcvTRUE;

    // Allocate memory from page heap.
    if (Os->pageHeap)
    {
        Os->pageHeap->AllocatePages(
            physical->bytes / PAGE_SIZE,
            physical->physical,
            physical->logical);

        if (physical->logical)
        {
            physical->type = gcvPHYSICAL_TYPE_PAGEHEAP;
        }
    }

    if (!physical->logical && Os->gchal)
    {
        // Allocate physical memory.
        if (!Os->gchal->AllocateMemory(physical->bytes, physical->logical, physical->physical))
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        physical->type = gcvPHYSICAL_TYPE_PHYSICAL;
    }

    // Initialize gcsPHYSICAL structure.
    physical->reference = 1;
    physical->attr |= gcvPHYSICAL_ATTR_CONTIGUOUS;

    // Return arguments to caller.
    if (Logical)
    {
        if (InUserSpace)
        {
            gcmkONERROR(gckOS_LockPages(Os, physical, physical->bytes, gcvFALSE, Logical, gcvNULL));
        }
        else
        {
            *Logical = physical->logical;
        }
    }

    gcmkASSERT(physical->bytes);

    *Bytes    = physical->bytes;
    *Physical = (gctPHYS_ADDR) physical;

#ifdef MEMORY_STAT
    gcsOBJ_LIST_AddNode(&Os->objList, &physical->node);

    NonPagedMemoryCount += *Bytes;
#endif

OnError:

    if (status != gcvSTATUS_OK)
    {
#if gcdDEBUG
        gckOS_DebugTrace(gcvLEVEL_ERROR, "ERROR: %s(%d) Failed to allocate %d bytes.", __FUNCTION__, __LINE__, *Bytes);
#endif

        if (physical)
        {
            if (physical->logical)
            {
                if (physical->type == gcvPHYSICAL_TYPE_PAGEHEAP)
                {
                    gcmkVERIFY(Os->pageHeap->FreePages(physical->bytes / PAGE_SIZE,
                        physical->physical));
                }
                else
                {
                    gcmkVERIFY(Os->gchal->FreeMemory(
                        physical->bytes,
                        physical->logical,
                        physical->physical));
                }
            }

            delete physical;
#ifdef MEMORY_STAT
            SpecialHeapMemoryCount -= sizeof(gcsPHYSICAL);
#endif // MEMORY_STAT
        }
    }
    if (lock)
    {
        gcmOS_UNLOCK(Os);
    }

    // Return the status..
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeNonPagedMemory
**
**  Free previously allocated and mapped pages from non-paged memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes allocated.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocated memory.
**
**      gctPOINTER Logical
**          Logical address of the allocated memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_FreeNonPagedMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical
    )
{
    gcmkHEADER_ARG("Os=0x%X Bytes=%lu Physical=0x%X Logical=0x%X",
                   Os, Bytes, Physical, Logical);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);

    // Get pointer to gcsPHYSICAL structure,
    gcsPHYSICAL* physical = (gcsPHYSICAL*) Physical;

    gcmkVERIFY_ARGUMENT(physical->reference != 0);
    gcmkVERIFY_ARGUMENT(physical->bytes != 0);
    gcmkVERIFY_ARGUMENT((physical->type == gcvPHYSICAL_TYPE_PAGEHEAP)
        || (physical->type == gcvPHYSICAL_TYPE_PHYSICAL));

    // Unmap the page from memory.
    if (physical->reference > 1)
    {
        gcmkVERIFY_OK(gckOS_UnlockPages(Os, physical, Bytes, Logical));
    }

    if (physical->reference != 1)
    {
        gckOS_DebugTrace(gcvLEVEL_WARNING,
            "Freed locked NonPagedMemory: logical = 0x%08x(%d) physical = 0x%08x.\n",
            Logical, Bytes, Physical);
    }

    gcmOS_LOCK(Os);

    // Free the memory.
    if (physical->type == gcvPHYSICAL_TYPE_PAGEHEAP)
    {
        gcmkVERIFY(Os->pageHeap->FreePages(physical->bytes / PAGE_SIZE,
            physical->physical));
    }
    else
    {
        gcmkVERIFY(Os->gchal->FreeMemory(
            physical->bytes,
            physical->logical,
            physical->physical));
    }

    // Delete the gcsPHYSICAL structure.
    delete physical;

#ifdef MEMORY_STAT
    gcsOBJ_NODE_Detach(&physical->node);

    NonPagedMemoryCount -= Bytes;

    SpecialHeapMemoryCount -= sizeof(gcsPHYSICAL);
#endif

    gcmOS_UNLOCK(Os);

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocatePagedMemory
**
**  Allocate memory from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
*/
gceSTATUS
gckOS_AllocatePagedMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPHYS_ADDR * Physical
    )
{
    return gckOS_AllocatePagedMemoryEx(Os, gcvALLOC_FLAG_NONE, Bytes, gcvNULL, Physical);
}

/*******************************************************************************
**
**  gckOS_AllocatePagedMemoryEx
**
**  Allocate memory from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Flag
**          Allocation attribute.
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctUINT32 * Gid
**          Save the global ID for the piece of allocated memory.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
*/
gceSTATUS gckOS_AllocatePagedMemoryEx(
    IN gckOS Os,
    IN gctUINT32 Flag,
    IN gctSIZE_T Bytes,
    OUT gctUINT32 * Gid,
    OUT gctPHYS_ADDR * Physical
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsPHYSICAL* physical = gcvNULL;
    gctBOOL contiguous = Flag & gcvALLOC_FLAG_CONTIGUOUS;

    gcmkHEADER_ARG("Os=0x%X Flag=%x Bytes=%lu", Os, Flag, Bytes);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);

    // Align requested bytes to page boundary.
    Bytes = gcmALIGN(Bytes, PAGE_SIZE);
#if UNDER_CE <= 500
    Bytes = max(Bytes, 2 << 20);
#endif

    if (contiguous)
    {
        gcmkONERROR(gckOS_AllocateContiguous(
            Os,
            gcvFALSE,
            &Bytes,
            Physical,
            gcvNULL
            ));
    }
    else
    {
        // Allocate a new gcsPHYSICAL structure.
        physical = new gcsPHYSICAL;
        if (physical == gcvNULL)
        {
            // Out of memory.
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        // Reserve the requested number of pages.
        physical->logical = VirtualAlloc(gcvNULL,
                                         Bytes,
                                         MEM_RESERVE,
                                         PAGE_NOACCESS);
        if (physical->logical == gcvNULL)
        {
            // Out of memory.
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        // Commit the requested number of pages.
        if (gcvNULL == VirtualAlloc(physical->logical,
                                 Bytes,
                                 MEM_COMMIT,
                                 PAGE_READWRITE | PAGE_NOCACHE))
        {
            // Out of memory.
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        if (!LockPages(
            physical->logical,
            Bytes,
            NULL,
            LOCKFLAG_READ | LOCKFLAG_WRITE))
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }


        physical->bytes = Bytes;
        physical->type  = gcvPHYSICAL_TYPE_VIRTUAL;
        physical->reference = 1;

        // Return pointer to gcsPHYSICAL.
        *Physical = (gctPHYS_ADDR) physical;

        /* Gid is not in use at the moment. */
        if (Gid != gcvNULL)
        {
            *Gid = 0;
        }

#ifdef MEMORY_STAT
        SpecialHeapMemoryCount += sizeof(gcsPHYSICAL);
        PagedMemoryCount       += Bytes;
#endif // MEMORY_STAT
    }

    gcmkASSERT(((gcsPHYSICAL*)*Physical)->bytes);

OnError:

    if (status != gcvSTATUS_OK)
    {
#if gcdDEBUG
        gckOS_DebugTrace(gcvLEVEL_ERROR, "ERROR: %s(%d) failed to allocate memory: %d(%d)",
            __FUNCTION__, __LINE__, Bytes, contiguous);
#endif

        if (physical)
        {
            if (physical->logical)
            {
                // Free the reserved number of pages.
                gcmkVERIFY(VirtualFree(physical->logical,
                                        Bytes,
                                        MEM_DECOMMIT));

                gcmkVERIFY(VirtualFree(physical->logical,
                                        0,
                                        MEM_RELEASE));
            }

            // Delete gcsPHYSICAL structure.
            delete physical;
        }
    }

    // Return the status.
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreePagedMemory
**
**  Free memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_FreePagedMemory(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes
    )
{
    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    // Cast pointer to gcsPHYSICAL structure.
    gcsPHYSICAL* physical = (gcsPHYSICAL*) Physical;

    gcmkVERIFY_ARGUMENT(Bytes <= physical->bytes);
    gcmkVERIFY_ARGUMENT(physical->reference != 0);
    gcmkASSERT(physical->bytes);

    if (physical->reference != 1)
    {
        gckOS_DebugTrace(gcvLEVEL_WARNING,
            "Freed locked PagedMemory: logical = 0x%08x(%d) physical = 0x%08x.\n",
            physical->logical, Bytes, physical);
    }

    if (physical->type == gcvPHYSICAL_TYPE_VIRTUAL)
    {
        gcmkVERIFY(UnlockPages(physical->logical, physical->bytes));

        // Decommit the number of requested pages.
        gcmkVERIFY(VirtualFree(physical->logical,
                                physical->bytes,
                                MEM_DECOMMIT));

        // Release the number of reserved pages, with a 2MB minimum.
        gcmkVERIFY(VirtualFree(physical->logical,
                                0,
                                MEM_RELEASE));

        // Delete the gcsPHYSICAL structure.
        delete physical;

#ifdef MEMORY_STAT
        SpecialHeapMemoryCount -= sizeof(gcsPHYSICAL);
        PagedMemoryCount       -= Bytes;
#endif // MEMORY_STAT
    }
    else if (physical->type == gcvPHYSICAL_TYPE_WRAPPED_MEMORY)
    {
        // Delete the gcsPHYSICAL structure.
        delete physical;
    }
    else
    {
        gcmkVERIFY_OK(gckOS_FreeContiguous(
            Os,
            physical,
            physical->logical,
            physical->bytes));
    }

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AllocateContiguous
**
**  Allocate memory from the contiguous pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL InUserSpace
**          gcvTRUE if the pages need to be mapped into user space.
**
**      gctSIZE_T * Bytes
**          Pointer to the number of bytes to allocate.
**
**  OUTPUT:
**
**      gctSIZE_T * Bytes
**          Pointer to a variable that receives the number of bytes allocated.
**
**      gctPHYS_ADDR * Physical
**          Pointer to a variable that receives the physical address of the
**          memory allocation.
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the logical address of the
**          memory allocation.
*/
gceSTATUS
gckOS_AllocateContiguous(
    IN gckOS Os,
    IN gctBOOL InUserSpace,
    IN OUT gctSIZE_T * Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctPOINTER * Logical
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X InUserSpace=%d *Bytes=%lu",
                   Os, InUserSpace, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmkVERIFY_ARGUMENT(*Bytes > 0);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);

    /* Same as non-paged memory for now. */
    gcmkONERROR(gckOS_AllocateNonPagedMemory(Os,
                                             InUserSpace,
                                             gcvALLOC_FLAG_CONTIGUOUS,
                                             Bytes,
                                             Physical,
                                             Logical));

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=%lu *Physical=0x%X *Logical=0x%X",
                   *Bytes, *Physical, gcmOPT_POINTER(Logical));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeContiguous
**
**  Free memory allocated from the contiguous pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctPOINTER Logical
**          Logicval address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_FreeContiguous(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Logical=0x%X Bytes=%lu",
                   Os, Physical, Logical, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    /* Same of non-paged memory for now. */
    gcmkONERROR(gckOS_FreeNonPagedMemory(Os, Bytes, Physical, Logical));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_CreateMutex
**
**  Create a new mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Mutex
**          Pointer to a variable that will hold a pointer to the mutex.
*/
gceSTATUS gckOS_CreateMutex(
    IN gckOS Os,
    OUT gctPOINTER * Mutex
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    // Create the mutex.
    HANDLE mutex = CreateMutex(gcvNULL, gcvFALSE, gcvNULL);

    if (mutex == gcvNULL)
    {
        // Out of resources.
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    // Return mutex to caller.
    *Mutex = (gctPOINTER) mutex;

    // Success.
    gcmkFOOTER_ARG("*Mutex=0x%X", gcmOPT_VALUE(Mutex));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_DeleteMutex
**
**  Delete a mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mute to be deleted.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_DeleteMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex
    )
{
    gcmkHEADER_ARG("Os=0x%X Mutex=0x%X", Os, Mutex);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    // Close the mutex handle.
    gcmkVERIFY(CloseHandle((HANDLE) Mutex));

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AcquireMutex
**
**  Acquire a mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be acquired.
**
**      gctUINT32 Timeout
**          Timeout value specified in milliseconds.
**          Specify the value of gcvINFINITE to keep the thread suspended
**          until the mutex has been acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AcquireMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex,
    IN gctUINT32 Timeout
    )
{
    gcmkHEADER_ARG("Os=0x%X Mutex=0x%0x Timeout=%u", Os, Mutex, Timeout);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    // Compute timeout.
    DWORD timeout = (Timeout == gcvINFINITE)
        ? INFINITE
        : Timeout;

    // Wait for the mutex.
    DWORD status = WaitForSingleObject((HANDLE) Mutex, timeout);
    gcmkASSERT(status != WAIT_FAILED);

    if (status == WAIT_TIMEOUT)
    {
        // Timeout.
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_TIMEOUT);
        return gcvSTATUS_TIMEOUT;
    }

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ReleaseMutex
**
**  Release an acquired mutex.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Mutex
**          Pointer to the mutex to be released.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_ReleaseMutex(
    IN gckOS Os,
    IN gctPOINTER Mutex
    )
{
    gcmkHEADER_ARG("Os=0x%X Mutex=0x%0x", Os, Mutex);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Mutex != gcvNULL);

    // Release the mutex.
    gcmkVERIFY(ReleaseMutex((HANDLE) Mutex));

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_MapPhysical
**
**  Map a physical address into kernel space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Physical
**          Physical address of the memory to map.
**
**      gctSIZE_T Bytes
**          Number of bytes to map.
**
**  OUTPUT:
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the base address of the mapped
**          memory.
*/
gceSTATUS gckOS_MapPhysical(
    IN gckOS Os,
    IN gctUINT32 Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical
    )
{
    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Bytes);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    // Align to pages.
    gctUINT offset   = Physical & (PAGE_SIZE - 1);
    DWORD physical = Physical - offset;
    DWORD bytes    = gcmALIGN(Bytes + offset, PAGE_SIZE);

    // Create static mapping.
    gctPOINTER logical = CreateStaticMapping(physical >> 8, bytes);

    if (logical == gcvNULL)
    {
        // Out of resources.
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    // Return mapped address to user.
    *Logical = (gctUINT8 *) logical + offset;

    // Success.
    gcmkFOOTER_ARG("*Logical=0x%X", gcmOPT_VALUE(Logical));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_UnmapPhysical
**
**  Unmap a previously mapped memory region from kernel memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Pointer to the base address of the memory to unmap.
**
**      gctSIZE_T Bytes
**          Number of bytes to unmap.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_UnmapPhysical(
    IN gckOS Os,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    gcmkHEADER_ARG("Os=0x%X Logical=0x%X Bytes=%lu", Os, Logical, Bytes);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    gcmkVERIFY(DeleteStaticMapping(Logical, Bytes));

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ReadRegister
**
**  Read data from a register.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Address
**          Address of register.
**
**  OUTPUT:
**
**      gctUINT32 * Data
**          Pointer to a variable that receives the data read from the register.
*/
gceSTATUS
gckOS_ReadRegister(
    IN gckOS Os,
    IN gctUINT32 Address,
    OUT gctUINT32 * Data
    )
{
    return gckOS_ReadRegisterEx(Os, gcvCORE_MAJOR, Address, Data);
}

gceSTATUS
gckOS_ReadRegisterEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    OUT gctUINT32 * Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%X", Os, Core, Address);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Data != gcvNULL);

    // Read data from the register.
#ifdef EMULATOR
    U32 data;
    Dma->ReadRegister(Address, data);
    *Data = data;
#else
    *Data =
        READ_REGISTER_ULONG((PULONG) Os->gchal->GetRegisterAddress(Core, Address));
#endif

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
               "gckOS_ReadRegisterEx: Register 0x%04X returned 0x%08X",
               Address >> 2, *Data);

    // Success.
    gcmkFOOTER_ARG("*Data=0x%08x", gcmOPT_VALUE(Data));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_WriteRegister
**
**  Write data to a register.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Address
**          Address of register.
**
**      gctUINT32 Data
**          Data for register.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_WriteRegister(
    IN gckOS Os,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    return gckOS_WriteRegisterEx(Os, gcvCORE_MAJOR, Address, Data);
}

gceSTATUS
gckOS_WriteRegisterEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    IN gctUINT32 Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%X Data=0x%08x", Os, Core, Address, Data);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
               "gckOS_WriteRegisterEx: Writing 0x%08X to register 0x%04X",
               Data, Address >> 2);

#ifdef EMULATOR
    // Write data to the register.
    Dma->WriteRegister(Address, Data);

    // Monitor AQ_CMD_BUFFER_CTRL register. */
    if (Address == AQ_CMD_BUFFER_CTRL_Address)
    {
        if (gcmVERIFYFIELDVALUE(Data, AQ_CMD_BUFFER_CTRL, ENABLE, ENABLE))
        {
            // Start the threads when command queue gets enabled.
            Os->gchal->StartThreads();
        }
        else
        {
            // Stop the threads when command queue gets disabled.
            Os->gchal->StopThreads();
        }
    }
#else
    // Write data to the register.
    WRITE_REGISTER_ULONG((PULONG) Os->gchal->GetRegisterAddress(Core, Address), Data);
#endif

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetPageSize
**
**  Get the system's page size.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**  OUTPUT:
**
**      gctSIZE_T * PageSize
**          Pointer to a variable that will receive the system's page size.
*/
gceSTATUS gckOS_GetPageSize(
    IN gckOS Os,
    OUT gctSIZE_T * PageSize
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(PageSize != gcvNULL);

    // Return the page size.
    *PageSize = PAGE_SIZE;

    // Success.
    gcmkFOOTER_ARG("*PageSize", gcmOPT_VALUE(PageSize));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicExchange
**
**  Atomically exchange a pair of 32-bit values.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**    IN OUT gctINT32_PTR Target
**        Pointer to the 32-bit value to exchange.
**
**      IN gctINT32 NewValue
**          Specifies a new value for the 32-bit value pointed to by Target.
**
**    OUT gctINT32_PTR OldValue
**        The old value of the 32-bit value pointed to by Target.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomicExchange(
    IN gckOS Os,
    IN OUT gctUINT32_PTR Target,
    IN gctUINT32 NewValue,
    OUT gctUINT32_PTR OldValue
    )
{
    gcmkHEADER_ARG("Os=0x%X Target=0x%X NewValue=%u", Os, Target, NewValue);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    /* Exchange the pair of 32-bit values. */
    *OldValue = (gctUINT32)InterlockedExchange((LPLONG)Target, (LONG)NewValue);

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=%u", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicExchangePtr
**
**  Atomically exchange a pair of pointers.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**        gctPOINTER * Target
**            Pointer to the pointer value to exchange.
**
**      gctPOINTER NewValue
**          Specifies a new value for the pointer value pointed to by Target.
**
**  OUTPUT:
**
**        gctPOINTER * OldValue
**            The old value of the pointer value pointed to by Target.
**
*/
gceSTATUS
gckOS_AtomicExchangePtr(
    IN gckOS Os,
    IN OUT gctPOINTER * Target,
    IN gctPOINTER NewValue,
    OUT gctPOINTER * OldValue
    )
{
    gcmkHEADER_ARG("Os=0x%X Target=0x%X NewValue=0x%X", Os, Target, NewValue);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    /* Exchange the pair of 32-bit values. */
    *OldValue = InterlockedExchangePointer(Target, NewValue);

    /* Success. */
    gcmkFOOTER_ARG("*OldValue=0x%X", gcmOPT_VALUE(OldValue));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Delay
**
**  Delay execution of the current thread for a number of milliseconds.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 Delay
**          Delay to sleep, specified in milliseconds.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_Delay(
    IN gckOS Os,
    IN gctUINT32 Delay
    )
{
    gcmkHEADER_ARG("Os=0x%X Delay=%u", Os, Delay);

    // Sleep.
    Sleep(Delay);

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetPhysicalAddress
**
**  Get the physical system address of a corresponding virtual address.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Logical address.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Pointer to a variable that receives the 32-bit physical address.
*/
gceSTATUS gckOS_GetPhysicalAddress(
    IN gckOS Os,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address
    )
{
    gcmkHEADER_ARG("Os=0x%X Logical=0x%X", Os, Logical);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    // Query the physical page.
    DWORD page;
    if (!LockPages(Logical, 1, &page, LOCKFLAG_QUERY_ONLY))
    {
        // Out of resources.
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    // Return the physical address.
    *Address = (page << UserKInfo[KINX_PFN_SHIFT]) |
               ((gctUINT32) Logical & (PAGE_SIZE - 1));

    // Success.
    gcmkFOOTER_ARG("*Address=0x%08x", gcmOPT_VALUE(Address));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_UserLogicalToPhysical
**
**  Get the physical system address of a corresponding user virtual address.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Logical
**          Logical address.
**
**  OUTPUT:
**
**      gctUINT32 * Address
**          Pointer to a variable that receives the 32-bit physical address.
*/
gceSTATUS gckOS_UserLogicalToPhysical(
    IN gckOS Os,
    IN gctPOINTER Logical,
    OUT gctPHYS_ADDR_T * Address
    )
{
    return gckOS_GetPhysicalAddress(Os, Logical, Address);
}

/*******************************************************************************
**
**  gckOS_LockPages
**
**  Lock memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**      gctBOOL Cacheable
**          Cache mode of mapping.
**
**  OUTPUT:
**
**      gctPOINTER * Logical
**          Pointer to a variable that receives the address of the mapped
**          memory.
**
**      gctSIZE_T * PageCount
**          Pointer to a variable that receives the number of pages required for
**          the page table.
*/
gceSTATUS gckOS_LockPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctBOOL Cacheable,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%lu", Os, Physical, Logical);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    gcsPHYSICAL * physical = (gcsPHYSICAL *) Physical;

    gcmkONERROR(gckOS_MapMemory(
            Os, physical, Bytes, Logical));

    if (PageCount)
    {
        if (physical->logical)
        {
            *PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(physical->logical, Bytes);
        }
        else if (physical->physical.LowPart != ~0U)
        {
            *PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(physical->physical.LowPart, Bytes);
        }
        else
        {
            *PageCount = 0;
        }

        *PageCount += physical->extraPage;
    }

OnError:

    // Success.
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_UnlockPages
**
**  Unlock memory allocated from the paged pool.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T Bytes
**          Number of bytes of the allocation.
**
**      gctPOINTER Logical
**          Address of the mapped memory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS gckOS_UnlockPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Physical=0x%X Bytes=%u Logical=0x%X",
                   Os, Physical, Bytes, Logical);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Logical != gcvNULL);

    status = gckOS_UnmapMemory(
            Os, (gcsPHYSICAL *) Physical, Bytes, Logical);

    // Success.
    gcmkFOOTER_NO();
    return status;
}

/*******************************************************************************
**
**  gckOS_MapPages
**
**  Map paged memory into a page table.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPHYS_ADDR Physical
**          Physical address of the allocation.
**
**      gctSIZE_T PageCount
**          Number of pages required for the physical address.
**
**      gctPOINTER PageTable
**          Pointer to the page table to fill in.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_MapPages(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T PageCount,
    IN gctPOINTER PageTable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_MapPagesEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T PageCount,
    IN gctUINT32 Address,
    IN gctPOINTER PageTable,
    IN gctBOOL Writable,
    IN gceSURF_TYPE Type
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32_PTR pageTable = (gctUINT32_PTR) PageTable;
    gctSIZE_T i, n;

    gcmkHEADER_ARG("Os=0x%X Core=%d Physical=0x%X PageCount=%u PageTable=0x%X",
                   Os, Core, Physical, PageCount, PageTable);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(PageCount > 0);
    gcmkVERIFY_ARGUMENT(PageTable != gcvNULL);

    // Get pointer to gcsPHYSICAL structure.
    gcsPHYSICAL * physical = (gcsPHYSICAL *) Physical;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
               "gckOS_MapPagesEx: %p: %u bytes @ %p into %u pages @ %p",
               physical,
               physical->bytes,
               physical->logical,
               PageCount,
               PageTable);

    gcmOS_LOCK(Os);

    // Query the physical pages.
    if (physical->logical != gcvNULL)
    {
        n = ADDRESS_AND_SIZE_TO_SPAN_PAGES(physical->logical, physical->bytes);

        if (n + physical->extraPage != PageCount)
        {
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (!LockPages(physical->logical,
                       physical->bytes,
                       (PDWORD) PageTable,
                       LOCKFLAG_QUERY_ONLY))
        {
            // Out of resources.
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        // Shift page number to address.
        for (i = 0; i < n; i++)
        {
            pageTable[i] <<= UserKInfo[KINX_PFN_SHIFT];
        }
    }
    else
    {
        n = ADDRESS_AND_SIZE_TO_SPAN_PAGES( \
                physical->physical.LowPart, physical->bytes);

        if (n + physical->extraPage != PageCount)
        {
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        pageTable[0] = gcmALIGN_BASE(physical->physical.LowPart, PAGE_SIZE);

        for (i = 1; i < n; i++)
        {
            pageTable[i] = pageTable[i - 1] + PAGE_SIZE;
        }
    }

    for (i = 0; i < physical->extraPage; i++)
    {
        pageTable[n + i] = Os->paddingPagePhys.LowPart;
    }

    // Walk through all pages in the page table.
    for (i = 0; i < PageCount; i++)
    {
#if gcdENABLE_VG
        if (Core == gcvCORE_VG)
        {
            gcmkONERROR(
                    gckVGMMU_SetPage(Os->gchal->GetKernel(Core)->vg->mmu,
                           pageTable[i],
                           pageTable + i));
        }
        else
#endif
        {
            gcmkONERROR(
                    gckMMU_SetPage(Os->gchal->GetKernel(Core)->mmu,
                           pageTable[i],
                           Writable,
                           pageTable + i));
        }
    }

OnError:
    gcmOS_UNLOCK(Os);

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_UnmapPages(
    IN gckOS Os,
    IN gctSIZE_T PageCount,
    IN gctUINT32 Address
    )
{
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_MemoryBarrier
**
**  Make sure the CPU has executed everything up to this point and the data got
**  written to the specified pointer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Address
**          Address of memory that needs to be barriered.
**
**  OUTPUT:
**
**      Nothing.
*/

#ifdef DSB_AVAILABLE
EXTERN_C DSB();
#endif

gceSTATUS gckOS_MemoryBarrier(
    IN gckOS Os,
    IN gctPOINTER Address
    )
{
#ifdef DSB_AVAILABLE
    DSB();
#else
    CacheRangeFlush(Address, 4, CACHE_SYNC_ALL);
#endif

    // Success.
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gckOS_GetTime
**
**  Get the number of microseconds since the system started.
**
**  INPUT:
**
**  OUTPUT:
**
**      gctUINT64_PTR Time
**          Pointer to a variable to get time.
**
*/
gceSTATUS
gckOS_GetTime(
    OUT gctUINT64_PTR Time
    )
{
    gcmkHEADER_ARG("Time=0x%x", Time);

    gcmkVERIFY_ARGUMENT(Time != gcvNULL);

    *Time = timeGetTime();

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gckOS_MapUserPointer
**
**  Map a pointer frmo the user process into the kernel address space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER * Pointer
**          Pointer in user process space that needs to mapped.
**
**      gctSIZE_T Size
**          Number of bytes that need to be mapped.
**
**  OUTPUT:
**
**      gctPOINTER * Pointer
**          Updated pointer mapped in kernel address space.
*/
gceSTATUS
gckOS_MapUserPointer(
    IN gckOS Os,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size,
    OUT gctPOINTER * KernelPointer
    )
{
    gcmkHEADER_ARG("Os=0x%X Pointer=0x%X Size=%lu", Os, Pointer, Size);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Size > 0);
    gcmkVERIFY_ARGUMENT(KernelPointer != gcvNULL);

    /* Don't try to map NULL pointers. */
    if (Pointer == gcvNULL)
    {
        *KernelPointer = gcvNULL;
        gcmkFOOTER_ARG("*KernelPointer=0x%X", gcmOPT_VALUE(KernelPointer));
        return gcvSTATUS_OK;
    }

    // Map caller pointer to kernel process.
#if UNDER_CE >= 600
    if (IsAddressInUserSpace(Pointer))
    {
        HRESULT result = CeOpenCallerBuffer(KernelPointer,
                                            Pointer,
                                            Size,
                                            ARG_IO_PTR,
                                            FALSE);

        if (result != S_OK)
        {
            gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_RESOURCES);
            return gcvSTATUS_OUT_OF_RESOURCES;
        }
    }
    else
    {
        *KernelPointer = Pointer;
    }
#else
    *KernelPointer = MapCallerPtr(Pointer, Size);
#endif

    // Success.
    gcmkFOOTER_ARG("*KernelPointer=0x%X", gcmOPT_VALUE(KernelPointer));
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_UnmapUserPointer(
    IN gckOS Os,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size,
    IN gctPOINTER KernelPointer
    )
{
    gcmkHEADER_ARG("Os=0x%X Pointer=0x%X Size=%lu KernelPointer=0x%X",
                   Os, Pointer, Size, KernelPointer);

    // Verify the arguments.
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Size > 0);
    gcmkVERIFY_ARGUMENT(KernelPointer != gcvNULL);

    /* Don't try to unmap NULL pointers. */
    if (Pointer == gcvNULL)
    {
        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

#if UNDER_CE >= 600
    if (IsAddressInUserSpace(Pointer))
    {
        // Unmap the pointer.
        HRESULT result = CeCloseCallerBuffer(KernelPointer,
                                             Pointer,
                                             Size,
                                             ARG_IO_PTR);

        if (result != S_OK)
        {
            gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_RESOURCES);
            return gcvSTATUS_OUT_OF_RESOURCES;
        }
    }
#endif

    // Success.
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_QueryNeedCopy
**
**  Query whether the memory can be accessed or mapped directly or it has to be
**  copied.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID of the current process.
**
**  OUTPUT:
**
**      gctBOOL_PTR NeedCopy
**          Pointer to a boolean receiving gcvTRUE if the memory needs a copy or
**          gcvFALSE if the memory can be accessed or mapped dircetly.
*/
gceSTATUS
gckOS_QueryNeedCopy(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    OUT gctBOOL_PTR NeedCopy
    )
{
    gcmkHEADER_ARG("Os=0x%x ProcessID=%d", Os, ProcessID);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(NeedCopy != gcvNULL);

    /* No need to copy data. */
    *NeedCopy = gcvFALSE;

    /* Success. */
    gcmkFOOTER_ARG("*NeedCopy=%d", *NeedCopy);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CopyFromUserData
**
**  Copy data from user to kernel memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER KernelPointer
**          Pointer to kernel memory.
**
**      gctPOINTER Pointer
**          Pointer to user memory.
**
**      gctSIZE_T Size
**          Number of bytes to copy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_CopyFromUserData(
    IN gckOS Os,
    IN gctPOINTER KernelPointer,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size
    )
{
    gcmkHEADER_ARG("Os=0x%x KernelPointer=0x%x Pointer=0x%x Size=%lu",
                   Os, KernelPointer, Pointer, Size);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CopyToUserData
**
**  Copy data from kernel to user memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER KernelPointer
**          Pointer to kernel memory.
**
**      gctPOINTER Pointer
**          Pointer to user memory.
**
**      gctSIZE_T Size
**          Number of bytes to copy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_CopyToUserData(
    IN gckOS Os,
    IN gctPOINTER KernelPointer,
    IN gctPOINTER Pointer,
    IN gctSIZE_T Size
    )
{
    gcmkHEADER_ARG("Os=0x%x KernelPointer=0x%x Pointer=0x%x Size=%lu",
                   Os, KernelPointer, Pointer, Size);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_WriteMemory
**
**  Write data to a memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctPOINTER Address
**          Address of the memory to write to.
**
**      gctUINT32 Data
**          Data for register.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_WriteMemory(
    IN gckOS Os,
    IN gctPOINTER Address,
    IN gctUINT32 Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Address=0x%X Data=%u", Os, Address, Data);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    /* Write memory. */
    *(volatile gctUINT32 *) Address = Data;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_ReadMappedPointer(
    IN gckOS Os,
    IN gctPOINTER Address,
    IN gctUINT32_PTR Data
    )
{
    gcmkHEADER_ARG("Os=0x%X Address=0x%X Data=%u", Os, Address, Data);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    /* Read memory . */
    *Data = *(gctUINT32_PTR)Address;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CreateSignal
**
**  Create a new signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctBOOL ManualReset
**          If set to gcvTRUE, gckOS_Signal with gcvFALSE must be called in
**          order to set the signal to nonsignaled state.
**          If set to gcvFALSE, the signal will automatically be set to
**          nonsignaled state by gckOS_WaitSignal function.
**
**  OUTPUT:
**
**      gctSIGNAL * Signal
**          Pointer to a variable receiving the created gctSIGNAL.
*/
gceSTATUS
gckOS_CreateSignal(
    IN gckOS Os,
    IN gctBOOL ManualReset,
    OUT gctSIGNAL * Signal
    )
{
    HANDLE handle;

    gcmkHEADER_ARG("Os=0x%X ManualReset=%d", Os, ManualReset);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Create an event. */
    handle = CreateEvent(gcvNULL, ManualReset, gcvFALSE, gcvNULL);

    if (handle == gcvNULL)
    {
        /* Out of resources. */
        gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    /* Return handle of event. */
    *Signal = (gctSIGNAL) handle;

    /* Success. */
    gcmkFOOTER_ARG("*Signal=0x%X", gcmOPT_VALUE(Signal));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_DestroySignal
**
**  Destroy a signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroySignal(
    IN gckOS Os,
    IN gctSIGNAL Signal
    )
{
    gcmkHEADER_ARG("Os=0x%X Signal=0x%X", Os, Signal);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Close the event. */
    gcmkVERIFY(CloseHandle((HANDLE) Signal));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_Signal
**
**  Set a state of the specified signal.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctBOOL State
**          If gcvTRUE, the signal will be set to signaled state.
**          If gcvFALSE, the signal will be set to nonsignaled state.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_Signal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctBOOL State
    )
{
    gcmkHEADER_ARG("Os=0x%X Signal=0x%X State=%d", Os, Signal, State);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Set the new state of the event. */
    if (State)
    {
        /* Set the event to a signaled state. */
        gcmkVERIFY(SetEvent((HANDLE) Signal));
    }
    else
    {
        /* Set the event to a signaled state. */
        gcmkVERIFY(ResetEvent((HANDLE) Signal));
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_WaitSignal
**
**  Wait for a signal to become signaled.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to the gctSIGNAL.
**
**      gctUINT32 Wait
**          Number of milliseconds to wait.
**          Pass the value of gcvINFINITE for an infinite wait.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_WaitSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctBOOL Interruptable,
    IN gctUINT32 Wait
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    DWORD milliseconds;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X Wait=0x%08X", Os, Signal, Wait);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Convert wait to milliseconds. */
    milliseconds = (Wait == gcvINFINITE)
        ? INFINITE
        : Wait;

    /* Wait for event to become signaled. */
    switch (WaitForSingleObject((HANDLE) Signal, milliseconds))
    {
    case WAIT_OBJECT_0:
        /* Success. */
        break;

    case WAIT_TIMEOUT:
        /* Time out. */
        status = gcvSTATUS_TIMEOUT;
        break;

    default:
        /* Some other error. */
        status = gcvSTATUS_INVALID_DATA;
        break;
    }

    /* Success. */
    gcmkFOOTER_ARG("Signal=0x%X status=%d", Signal, status);
    return status;
}

/*******************************************************************************
**
**  gckOS_MapSignal
**
**  Map a signal in to the current process space.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to an gckOS object.
**
**      gctSIGNAL Signal
**          Pointer to tha gctSIGNAL to map.
**
**      gctHANDLE Process
**          Handle of process owning the signal.
**
**  OUTPUT:
**
**      gctSIGNAL * MappedSignal
**          Pointer to a variable receiving the mapped gctSIGNAL.
*/
gceSTATUS
gckOS_MapSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN HANDLE Process,
    OUT gctSIGNAL * MappedSignal
    )
{
    HANDLE handle;
    HANDLE currentProcess =  GetCurrentProcess();

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X Process=0x%X", Os, Signal, Process);

    if (Process == currentProcess)
    {
        handle = gcvNULL;
    }

    else if (!DuplicateHandle(Process,
                              Signal,
                              currentProcess,
                              &handle,
                              0,
                              gcvFALSE,
                              DUPLICATE_SAME_ACCESS))
    {
        /* Error. */
        gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    /* Return handle. */
    *(HANDLE *) MappedSignal = handle;

    /* Success. */
    gcmkFOOTER_ARG("*MappedSignal=0x%X", gcmOPT_VALUE(MappedSignal));
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**    gckOS_UnmapSignal
**
**    Unmap a signal .
**
**    INPUT:
**
**        gckOS Os
**            Pointer to an gckOS object.
**
**        gctSIGNAL Signal
**            Pointer to that gctSIGNAL mapped.
*/
gceSTATUS
gckOS_UnmapSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    HANDLE signal = (HANDLE)Signal;

    gcmkHEADER_ARG("Os=0x%X Signal=0x%X", Os, Signal);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    if (signal != gcvNULL)
    {
        /* Destroy the mapped signal. */
        status =
            gckOS_DestroySignal(Os, signal);
    }

    gcmkFOOTER();
    return status;
}

struct USER_MAPPING_INFO
{
    gctPOINTER    logical;
    gctUINT32_PTR mmuTable;
    gctSIZE_T     pageCount;
    gctUINT32     address;
    DWORD         physTable[1];
};

/*******************************************************************************
**
**  gckOS_MapUserMemory
**
**  Lock down a user buffer and return an DMA'able address to be used by the
**  hardware to access it.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory to lock down.
**
**      gctSIZE_T Size
**          Size in bytes of the memory to lock down.
**
**  OUTPUT:
**
**      gctPOINTER * Info
**          Pointer to variable receiving the information record required by
**          gckOS_UnmapUserMemory.
**
**
**      gctUINT32_PTR Address
**          Pointer to a variable that will receive the address DMA'able by the
**          hardware.
*/
gceSTATUS
gckOS_MapUserMemory(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPOINTER Memory,
    IN gctUINT32 Physical,
    IN gctSIZE_T Size,
    OUT gctPOINTER * Info,
    OUT gctUINT32_PTR Address
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_UnmapUserMemory
**
**  Unlock a user buffer and that was previously locked down by
**  gckOS_MapUserMemory.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory to unlock.
**
**      gctSIZE_T Size
**          Size in bytes of the memory to unlock.
**
**      gctPOINTER Info
**          Information record returned by gckOS_MapUserMemory.
**
**      gctUINT32_PTR Address
**          The address returned by gckOS_MapUserMemory.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_UnmapUserMemory(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctPOINTER Memory,
    IN gctSIZE_T Size,
    IN gctPOINTER Info,
    IN gctUINT32 Address
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
**
**  gckOS_GetBaseAddress
**
**  Get the base address for the physical memory.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**  OUTPUT:
**
**      gctUINT32_PTR BaseAddress
**          Pointer to a variable that will receive the base address.
*/
gceSTATUS
gckOS_GetBaseAddress(
    IN gckOS Os,
    OUT gctUINT32_PTR BaseAddress
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(BaseAddress != gcvNULL);

    /* Return base address. */
    *BaseAddress = Os->gchal->GetBaseAddress();

    /* Success. */
    gcmkFOOTER_ARG("*BaseAddress=0x%08x", gcmOPT_VALUE(BaseAddress));
    return gcvSTATUS_OK;
}

/******************************************************************************\
*********************************** S T U B S **********************************
\******************************************************************************/

gceSTATUS
gckOS_CreateUserSignal(
    IN gckOS Os,
    IN gctBOOL ManualReset,
    OUT gctINT_PTR SignalID
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_DestroyUserSignal(
    IN gckOS Os,
    IN gctINT SignalID
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_WaitUserSignal(
    IN gckOS Os,
    IN gctINT SignalID,
    IN gctUINT32 Wait
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_SignalUserSignal(
    IN gckOS Os,
    IN gctINT SignalID,
    IN gctBOOL State
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_UserSignal(
    IN gckOS Os,
    IN gctSIGNAL Signal,
    IN gctHANDLE Process
    )
{
    gceSTATUS status;
    gctSIGNAL mappedSignal;
    gctBOOL   mapped = gcvFALSE;

    /* Map the signal. */
    gcmkONERROR(
        gckOS_MapSignal(Os, Signal, Process, &mappedSignal));
    mapped = gcvTRUE;

    /* Set the signal to signalled state. */
    gcmkONERROR(
        gckOS_Signal(Os, (mappedSignal == gcvNULL) ? Signal : mappedSignal, gcvTRUE));

    /* Unmap the signal */
    mapped = gcvFALSE;
    gcmkONERROR(gckOS_UnmapSignal(Os, mappedSignal));

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    if (mapped)
    {
        gcmkVERIFY_OK(gckOS_UnmapSignal(Os, mappedSignal));
    }

    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_SuspendInterrupt(
    IN gckOS Os
    )
{
    return gckOS_SuspendInterruptEx(Os, gcvCORE_MAJOR);
}

gceSTATUS
gckOS_SuspendInterruptEx(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    status = gckOS_AcquireMutex(Os, Os->interruptMutex, gcvINFINITE);

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_ResumeInterrupt(
    IN gckOS Os
    )
{
    return gckOS_ResumeInterruptEx(Os, gcvCORE_MAJOR);
}

gceSTATUS
gckOS_ResumeInterruptEx(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    status = gckOS_ReleaseMutex(Os, Os->interruptMutex);

    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_GetTicks
**
**  Get the number of milliseconds since the system started.
**
**  INPUT:
**
**  OUTPUT:
**
*/
gceSTATUS
gckOS_GetTicks(
    OUT gctUINT32_PTR Time
    )
{
    gcmkHEADER_ARG("Time=0x%x", Time);

    gcmkVERIFY_ARGUMENT(Time != gcvNULL);

    *Time = GetTickCount();

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_TicksAfter
**
**  Compare time values got from gckOS_GetTicks.
**
**  INPUT:
**      gctUINT32 Time1
**          First time value to be compared.
**
**      gctUINT32 Time2
**          Second time value to be compared.
**
**  OUTPUT:
**
**      gctBOOL_PTR IsAfter
**          Pointer to a variable to result.
**
*/
gceSTATUS
gckOS_TicksAfter(
    IN gctUINT32 Time1,
    IN gctUINT32 Time2,
    OUT gctBOOL_PTR IsAfter
    )
{
     gcmkHEADER_ARG("Time1=0x%x Time2=0x%x IsAfter=0x%x", Time1, Time2, IsAfter);

     gcmkVERIFY_ARGUMENT(IsAfter != gcvNULL);

    *IsAfter = (Time2 < Time1);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_CreateTimer
**
**  Create a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctTIMERFUNCTION Function.
**          Pointer to a call back function which will be called when timer is
**          expired.
**
**      gctPOINTER Data.
**          Private data which will be passed to call back function.
**
**  OUTPUT:
**
**      gctPOINTER * Timer
**          Pointer to a variable receiving the created timer.
*/
gceSTATUS
gckOS_CreateTimer(
    IN gckOS Os,
    IN gctTIMERFUNCTION Function,
    IN gctPOINTER Data,
    OUT gctPOINTER * Timer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsOSTIMER_PTR timer;
    gcmkHEADER_ARG("Os=0x%X Function=0x%X Data=0x%X", Os, Function, Data);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    gcmkONERROR(gckOS_Allocate(Os, sizeof(gcsOSTIMER), (gctPOINTER*)&timer));

    timer->timerID  = 0;
    timer->function = Function;
    timer->data     = Data;

    *Timer = timer;

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_DestroyTimer
**
**  Destory a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be destoryed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroyTimer(
    IN gckOS Os,
    IN gctPOINTER Timer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsOSTIMER_PTR timer;
    gcmkHEADER_ARG("Os=0x%X Timer=0x%X", Os, Timer);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    timer = (gcsOSTIMER_PTR)Timer;

    if (timer->timerID)
    {
        gcmkONERROR(gckOS_StopTimer(Os, Timer));
    }

    gcmkONERROR(gcmkOS_SAFE_FREE(Os, Timer));

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_StartTimer
**
**  Schedule a software timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be scheduled.
**
**      gctUINT32 Delay
**          Delay in milliseconds.
**
**  OUTPUT:
**
**      Nothing.
*/

static void LocalTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    gcsOSTIMER_PTR timer = (gcsOSTIMER_PTR)dwUser;

    if ((timer->timerID == uTimerID) && timer->function)
    {
        timer->timerID = 0;
        timer->function(timer->data);
    }
}

gceSTATUS
gckOS_StartTimer(
    IN gckOS Os,
    IN gctPOINTER Timer,
    IN gctUINT32 Delay
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsOSTIMER_PTR timer = (gcsOSTIMER_PTR)Timer;

    gcmkHEADER_ARG("Os=0x%X Timer=0x%X Delay=%d", Os, Timer, Delay);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    if (timer->timerID)
    {
        gcmkONERROR(gckOS_StopTimer(Os, Timer));
    }

    timer->timerID = timeSetEvent(
      Delay,
      Os->timerCaps.wPeriodMin,
      LocalTimerCallback,
      (DWORD)Timer,
      TIME_ONESHOT
      );

    if (!timer->timerID)
    {
        status = gcvSTATUS_INVALID_DATA;
    }

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_StopTimer
**
**  Cancel a unscheduled timer.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Timer
**          Pointer to the timer to be cancel.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_StopTimer(
    IN gckOS Os,
    IN gctPOINTER Timer
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsOSTIMER_PTR timer = (gcsOSTIMER_PTR)Timer;

    gcmkHEADER_ARG("Os=0x%X Timer=0x%X", Os, Timer);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Timer != gcvNULL);

    if (timer->timerID)
    {
        MMRESULT mr = timeKillEvent(timer->timerID);
        if (mr != TIMERR_NOERROR)
        {
            gcmkONERROR(gcvSTATUS_NOT_FOUND);
        }

        timer->timerID = 0;
    }

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_MemCopy(
        IN gctPOINTER Destination,
        IN gctCONST_POINTER Source,
        IN gctSIZE_T Bytes
        )
{
    gcmkHEADER_ARG("Destination=0x%X Source=0x%X Bytes=%lu",
                   Destination, Source, Bytes);

    gcmkVERIFY_ARGUMENT(Destination != NULL);
    gcmkVERIFY_ARGUMENT(Source != NULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    memcpy(Destination, Source, Bytes);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}


gceSTATUS
gckOS_ZeroMemory(
    IN gctPOINTER Memory,
    IN gctSIZE_T Bytes
    )
{
    gcmkHEADER_ARG("Memory=0x%x Bytes=%lu", Memory, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes > 0);

    /* Zero the memory. */
    memset(Memory, 0, Bytes);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}


/*******************************************************************************
**
**  gckOS_AllocateMemory
**
**  Allocate memory wrapper.
**
**  INPUT:
**
**      gctSIZE_T Bytes
**          Number of bytes to allocate.
**
**  OUTPUT:
**
**      gctPOINTER * Memory
**          Pointer to a variable that will hold the allocated memory location.
*/
gceSTATUS
gckOS_AllocateMemory(
    IN gckOS Os,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    )
{
    gctPOINTER memory;
    gceSTATUS status;

    gcmkHEADER_ARG("Bytes=%lu", Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Bytes > 0);
    gcmkVERIFY_ARGUMENT(Memory != NULL);

    /* Allocate memory from OS pool. */
    memory = LocalAlloc(LMEM_FIXED, Bytes);

    if (memory == NULL)
    {
        /* Out of memory. */
        gckOS_DebugTrace(gcvLEVEL_ERROR, "ERROR: %s(%d) Allocate memory failed.", __FUNCTION__, __LINE__);
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Return pointer to the memory allocation. */
    *Memory = memory;

    /* Success. */
    gcmkFOOTER_ARG("*Memory=%p", *Memory);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_FreeMemory
**
**  Free allocated memory wrapper.
**
**  INPUT:
**
**      gctPOINTER Memory
**          Pointer to memory allocation to free.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_FreeMemory(
    IN gckOS Os,
    IN gctPOINTER Memory
    )
{
    gcmkHEADER_ARG("Memory=%p", Memory);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Memory != NULL);

    /* Free the memory from the OS pool. */
    LocalFree(Memory);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**  gckOS_CacheClean
**
**  Clean the cache for the specified addresses.  The GPU is going to need the
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctSIZE_T Offset
**          Offset to this memory block.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheClean(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctSIZE_T Offset,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    CacheRangeFlush(Logical, Bytes, CACHE_SYNC_WRITEBACK | CACHE_SYNC_L2_WRITEBACK);

    return gcvSTATUS_OK;
}

/*******************************************************************************
**  gckOS_CacheInvalidate
**
**  Invalidate the cache for the specified addresses. The GPU is going to need
**  data.  If the system is allocating memory as non-cachable, this function can
**  be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctSIZE_T Offset
**          Offset to this memory block.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheInvalidate(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctSIZE_T Offset,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    CacheRangeFlush(Logical, Bytes, CACHE_SYNC_DISCARD | CACHE_SYNC_L2_DISCARD);

    return gcvSTATUS_OK;
}

/*******************************************************************************
**  gckOS_CacheFlush
**
**  Clean the cache for the specified addresses and invalidate the lines as
**  well.  The GPU is going to need and modify the data.  If the system is
**  allocating memory as non-cachable, this function can be ignored.
**
**  ARGUMENTS:
**
**      gckOS Os
**          Pointer to gckOS object.
**
**      gctUINT32 ProcessID
**          Process ID Logical belongs.
**
**      gctPHYS_ADDR Handle
**          Physical address handle.  If gcvNULL it is video memory.
**
**      gctSIZE_T Offset
**          Offset to this memory block.
**
**      gctPOINTER Logical
**          Logical address to flush.
**
**      gctSIZE_T Bytes
**          Size of the address range in bytes to flush.
*/
gceSTATUS
gckOS_CacheFlush(
    IN gckOS Os,
    IN gctUINT32 ProcessID,
    IN gctPHYS_ADDR Handle,
    IN gctSIZE_T Offset,
    IN gctPOINTER Logical,
    IN gctSIZE_T Bytes
    )
{
    CacheRangeFlush(Logical, Bytes, CACHE_SYNC_DISCARD | CACHE_SYNC_L2_WRITEBACK | CACHE_SYNC_L2_DISCARD);

    return gcvSTATUS_OK;
}

/*******************************************************************************
** Broadcast interface.
*/

gceSTATUS
gckOS_Broadcast(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gceBROADCAST Reason
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Hardware=0x%X Reason=%d", Os, Hardware, Reason);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    switch (Reason)
    {
    case gcvBROADCAST_FIRST_PROCESS:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "First process has attached");
        break;

    case gcvBROADCAST_LAST_PROCESS:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "Last process has detached");

        /* Put GPU OFF. */
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware,
                                                gcvPOWER_OFF_BROADCAST));
        break;

    case gcvBROADCAST_GPU_IDLE:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "GPU idle.");

        /* Put GPU IDLE. */
#if gcdPOWER_SUSPEND_WHEN_IDLE
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware,
                                                gcvPOWER_SUSPEND_BROADCAST));
#else
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware,
                                                gcvPOWER_IDLE_BROADCAST));
#endif

        /* Add idle process DB. */
        gcmkONERROR(gckKERNEL_AddProcessDB(Hardware->kernel,
                                           1,
                                           gcvDB_IDLE,
                                           gcvNULL, gcvNULL, 0));
        break;

    case gcvBROADCAST_GPU_COMMIT:
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS, "COMMIT has arrived.");

        /* Add busy process DB. */
        gcmkONERROR(gckKERNEL_AddProcessDB(Hardware->kernel,
                                           0,
                                           gcvDB_IDLE,
                                           gcvNULL, gcvNULL, 0));

        /* Put GPU ON. */
        gcmkONERROR(
            gckHARDWARE_SetPowerManagementState(Hardware, gcvPOWER_ON_AUTO));
        break;

    case gcvBROADCAST_GPU_STUCK:
        gcmkTRACE_N(gcvLEVEL_ERROR, 0, "gcvBROADCAST_GPU_STUCK\n");
        gcmkONERROR(gckKERNEL_Recovery(Hardware->kernel));
        break;

    case gcvBROADCAST_AXI_BUS_ERROR:
        gcmkTRACE_N(gcvLEVEL_ERROR, 0, "gcvBROADCAST_AXI_BUS_ERROR\n");
        gcmkONERROR(gckKERNEL_Recovery(Hardware->kernel));
        break;

    case gcvBROADCAST_OUT_OF_MEMORY:
        status = gcvSTATUS_NOT_SUPPORTED;
        goto OnError;
        break;
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_BroadcastHurry
**
**  The GPU is running too slow.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gckHARDWARE Hardware
**          Pointer to the gckHARDWARE object.
**
**      gctUINT Urgency
**          The higher the number, the higher the urgency to speed up the GPU.
**          The maximum value is defined by the gcdDYNAMIC_EVENT_THRESHOLD.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_BroadcastHurry(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gctUINT Urgency
    )
{
    gcmkHEADER_ARG("Os=0x%x Hardware=0x%x Urgency=%u", Os, Hardware, Urgency);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_BroadcastCalibrateSpeed
**
**  Calibrate the speed of the GPU.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gckHARDWARE Hardware
**          Pointer to the gckHARDWARE object.
**
**      gctUINT Idle, Time
**          Idle/Time will give the percentage the GPU is idle, so you can use
**          this to calibrate the working point of the GPU.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_BroadcastCalibrateSpeed(
    IN gckOS Os,
    IN gckHARDWARE Hardware,
    IN gctUINT Idle,
    IN gctUINT Time
    )
{
    gcmkHEADER_ARG("Os=0x%x Hardware=0x%x Idle=%u Time=%u",
                   Os, Hardware, Idle, Time);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
********************************** Semaphores **********************************
*******************************************************************************/

/*******************************************************************************
**
**  gckOS_CreateSemaphore
**
**  Create a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Semaphore
**          Pointer to the variable that will receive the created semaphore.
*/
gceSTATUS
gckOS_CreateSemaphore(
    IN gckOS Os,
    OUT gctPOINTER * Semaphore
    )
{
    gceSTATUS status;
    HANDLE sem;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Ceate the semaphore. */
    sem = CreateSemaphore(NULL, 1, 10, NULL);
    if (sem == NULL)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Return to caller. */
    *Semaphore = (gctPOINTER) sem;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_AcquireSemaphore
**
**  Acquire a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AcquireSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Acquire the semaphore. */
    switch (WaitForSingleObject((HANDLE) Semaphore, INFINITE))
    {
    case WAIT_OBJECT_0:
        break;

    case WAIT_TIMEOUT:
        gcmkONERROR(gcvSTATUS_TIMEOUT);

    default:
        gcmkONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_TryAcquireSemaphore
**
**  Try to acquire a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be acquired.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_TryAcquireSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Acquire the semaphore. */
    switch (WaitForSingleObject((HANDLE) Semaphore, 0))
    {
    case WAIT_OBJECT_0:
        break;

    case WAIT_TIMEOUT:
        gcmkONERROR(gcvSTATUS_TIMEOUT);

    default:
        gcmkONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_ReleaseSemaphore
**
**  Release a previously acquired semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be released.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_ReleaseSemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%x Semaphore=0x%x", Os, Semaphore);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Release the semaphore. */
    if (!ReleaseSemaphore((HANDLE) Semaphore, 1, NULL))
    {
        gcmkONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_DestroySemaphore
**
**  Destroy a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**      gctPOINTER Semaphore
**          Pointer to the semaphore thet needs to be destroyed.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_DestroySemaphore(
    IN gckOS Os,
    IN gctPOINTER Semaphore
    )
{
    gcmkHEADER_ARG("Os=0x%x Semaphore=0x%x", Os, Semaphore);

     /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    /* Free the sempahore structure. */
    CloseHandle((HANDLE) Semaphore);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

#if gcdENABLE_VG
/********************************** Semaphores **********************************
*******************************************************************************/

/*******************************************************************************
**
**  gckOS_CreateSemaphore
**
**  Create a semaphore.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to the gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Semaphore
**          Pointer to the variable that will receive the created semaphore.
*/
gceSTATUS
gckOS_CreateSemaphoreVG(
    IN gckOS Os,
        OUT gctSEMAPHORE * Semaphore
    )
{
    gceSTATUS status;
    HANDLE semaphore;

    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    do
    {
    /* Create a semaphore object. */
        semaphore = CreateSemaphore(
                gcvNULL,                /* Default atributes. */
                0,                              /* Initial count. */
                gcvMAX_POS_INT, /* Maximum count. */
                gcvNULL                 /* Default name. */
                );

        /* Failed? */
        if (semaphore == gcvNULL)
        {
#if gcvDEBUG
            gctUINT error = GetLastError();
#endif
            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        /* Set the handle. */
        * Semaphore = (gctSEMAPHORE) semaphore;

/* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmkFOOTER();
    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_IncrementSemaphore(
        IN gckOS Os,
        IN gctSEMAPHORE Semaphore
        )
{
    gceSTATUS status;

#if gcdENABLE_INFINITE_SPEED_HW
    status = gcvSTATUS_OK;
#else
    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    do
    {
        /* Increment the semaphore's count. */
        if (!ReleaseSemaphore(Semaphore, 1, gcvNULL))
        {
#if gcvDEBUG
            gctUINT error = GetLastError();
#endif
            status = gcvSTATUS_INVALID_OBJECT;
                break;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmkFOOTER();
#endif

    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_DecrementSemaphore(
        IN gckOS Os,
        IN gctSEMAPHORE Semaphore
        )
{
    gceSTATUS status;

#if gcdENABLE_INFINITE_SPEED_HW
    status = gcvSTATUS_OK;
#else
    gctUINT result;

    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Semaphore != gcvNULL);

    do
    {
        /* Decrement the semaphore's count. If the count is zero, wait
           until it gets incremented. */
        result = WaitForSingleObject(Semaphore, INFINITE);

        /* Error? */
        if (result != WAIT_OBJECT_0)
        {
#if gcvDEBUG
            gctUINT error = GetLastError();
#endif

            status = gcvSTATUS_INVALID_OBJECT;
            break;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);
#endif

    gcmkFOOTER();
    /* Return the status. */
    return status;
}


/******************************************************************************\
******************************** Thread Object *********************************
\******************************************************************************/

gceSTATUS
gckOS_StartThread(
        IN gckOS Os,
        IN gctTHREADFUNC ThreadFunction,
        IN gctPOINTER ThreadParameter,
        OUT gctTHREAD * Thread
        )
{
    gceSTATUS status;
    HANDLE thread;

    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(ThreadFunction != gcvNULL);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    do
    {
        /* Start the thread. */
        thread = CreateThread(
                gcvNULL, 0,
                (LPTHREAD_START_ROUTINE) ThreadFunction,
                ThreadParameter,
                0, gcvNULL
                );

        /* Failed? */
        if (thread == gcvNULL)
        {
#if gcvDEBUG
            gctUINT error = GetLastError();
#endif

            status = gcvSTATUS_GENERIC_IO;
            break;
        }

        /* Set the thread handle. */
        * Thread = (gctTHREAD) thread;

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmkFOOTER();
    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_StopThread(
        IN gckOS Os,
        IN gctTHREAD Thread
        )
{
    gceSTATUS status;
    gctUINT result;

    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    do
    {
        /* Thread should have already been enabled to terminate, wait
           for it to stop naturally. */
        result = WaitForSingleObject(Thread, INFINITE);

        /* Error? */
        if (result != WAIT_OBJECT_0)
        {
                status = gcvSTATUS_GENERIC_IO;
                break;
        }

        /* Close interrupt thread. */
        if (!CloseHandle(Thread))
        {
#if gcvDEBUG
            gctUINT error = GetLastError();
#endif

            status = gcvSTATUS_INVALID_OBJECT;
            break;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    gcmkFOOTER();
    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_VerifyThread(
        IN gckOS Os,
        IN gctTHREAD Thread
        )
{
    gceSTATUS status;
    gctUINT result;

    gcmkHEADER_ARG("Os=0x%x", Os);
    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Thread != gcvNULL);

    do
    {
        /* Verify the state of the thread. */
        result = WaitForSingleObject(Thread, 0);

        /* If the wait timed out, it meand the thread is still running. */
        if (result == STATUS_TIMEOUT)
        {
            status = gcvSTATUS_OK;
            break;
        }

        /* The thread does not appear to be active. */
        status = gcvSTATUS_TERMINATE;
    }
    while (gcvFALSE);

    gcmkFOOTER();
    /* Return the status. */
    return status;
}

gceSTATUS
gckOS_GetKernelLogical(
    IN gckOS Os,
    IN gctUINT32 Address,
    OUT gctPOINTER * KernelPointer)
{
    return gckOS_GetKernelLogicalEx(Os, gcvCORE_MAJOR, Address, KernelPointer);
}

gceSTATUS
gckOS_GetKernelLogicalEx(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT32 Address,
    OUT gctPOINTER * KernelPointer
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%X Core=%d Address=0x%08x", Os, Core, Address);

    do
    {
        gcePOOL pool;
        gctUINT32 offset;
        gctPOINTER logical = gcvNULL;

#if gcdENABLE_VG
       if (Core == gcvCORE_VG)
       {
           gcmkERR_BREAK(gckVGHARDWARE_SplitMemory(
                Os->gchal->GetKernel(Core)->vg->hardware, Address, &pool, &offset
                ));
       }
       else
#endif
       {
        /* Split the memory address into a pool type and offset. */
            gcmkERR_BREAK(gckHARDWARE_SplitMemory(
                Os->gchal->GetKernel(Core)->hardware, Address, &pool, &offset
                ));
       }

        /* Dispatch on pool. */
        switch (pool)
        {
        case gcvPOOL_LOCAL_INTERNAL:
            /* Internal memory. */
            logical =  Os->gchal->GetInternalLogical();
            break;

        case gcvPOOL_LOCAL_EXTERNAL:
            /* External memory. */
            logical =  Os->gchal->GetExternalLogical();
            break;

        case gcvPOOL_SYSTEM:
            /* System memory. */
            logical =  Os->gchal->GetContiguousLogical();
            break;

        default:
            /* Invalid memory pool. */
            return gcvSTATUS_INVALID_ARGUMENT;
        }

        if (logical == gcvNULL)
        {
            gcmkERR_BREAK(gcvSTATUS_OUT_OF_MEMORY);
        }

        /* Build logical address of specified address. */
        * KernelPointer = ((gctUINT8_PTR) logical) + offset;

        /* Success. */
        gcmkFOOTER_ARG("*KernelPointer=0x%X", *KernelPointer);
        return gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Return status. */
    gcmkFOOTER();
    return status;
}
#endif

/*******************************************************************************
**
**  gckOS_AtomConstruct
**
**  Create an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**  OUTPUT:
**
**      gctPOINTER * Atom
**          Pointer to a variable receiving the constructed atom.
*/
gceSTATUS
gckOS_AtomConstruct(
    IN gckOS Os,
    OUT gctPOINTER * Atom
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Allocate the atom. */
    gcmkONERROR(gckOS_Allocate(Os, gcmSIZEOF(LONG), Atom));

    /* Initialize the atom. */
    InterlockedExchange((LONG *) *Atom, 0);

    /* Success. */
    gcmkFOOTER_ARG("*Atom=0x%x", *Atom);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_AtomDestroy
**
**  Destroy an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom to destroy.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomDestroy(
    IN gckOS Os,
    OUT gctPOINTER Atom
    )
{
    gceSTATUS status;

    gcmkHEADER_ARG("Os=0x%x Atom=0x%0x", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Free the atom. */
    gcmkONERROR(gckOS_Free(Os, Atom));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
**
**  gckOS_AtomIncrement
**
**  Atomically increment the 32-bit integer value inside an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**  OUTPUT:
**
**      gctINT32_PTR Value
**          Pointer to a variable the receives the original value of the atom.
*/
gceSTATUS
gckOS_AtomIncrement(
    IN gckOS Os,
    IN gctPOINTER Atom,
    OUT gctINT32_PTR Value
    )
{
    gcmkHEADER_ARG("Os=0x%x Atom=0x%0x", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Increment the atom. */
    *Value = InterlockedIncrement((LONG *) Atom) - 1;

    /* Success. */
    gcmkFOOTER_ARG("*Value=%d", *Value);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomDecrement
**
**  Atomically decrement the 32-bit integer value inside an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**  OUTPUT:
**
**      gctINT32_PTR Value
**          Pointer to a variable the receives the original value of the atom.
*/
gceSTATUS
gckOS_AtomDecrement(
    IN gckOS Os,
    IN gctPOINTER Atom,
    OUT gctINT32_PTR Value
    )
{
    gcmkHEADER_ARG("Os=0x%x Atom=0x%0x", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Decrement the atom. */
    *Value = InterlockedDecrement((LONG *) Atom) + 1;

    /* Success. */
    gcmkFOOTER_ARG("*Value=%d", *Value);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicSetMask
**
**  Atomically set mask to Atom
**
**  INPUT:
**      IN OUT gctPOINTER Atom
**          Pointer to the atom to set.
**
**      IN gctUINT32 Mask
**          Mask to set.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomSetMask(
    IN gctPOINTER Atom,
    IN gctUINT32 Mask
    )
{
    gctUINT32 oval, nval;

    gcmkHEADER_ARG("Atom=0x%0x");
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    do
    {
        oval = *(LONG volatile *) Atom;
        nval = oval | Mask;
    } while (InterlockedCompareExchange(
                                (volatile LONG *) Atom,
                                nval,
                                oval) != oval);
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomicClearMask
**
**  Atomically clear mask from Atom
**
**  INPUT:
**      IN OUT gctPOINTER Atom
**          Pointer to the atom to clear.
**
**      IN gctUINT32 Mask
**          Mask to clear.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomClearMask(
    IN gctPOINTER Atom,
    IN gctUINT32 Mask
    )
{
    gctUINT32 oval, nval;

    gcmkHEADER_ARG("Atom=0x%0x");
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    do
    {
        oval = *(LONG volatile *) Atom;
        nval = oval & ~Mask;
    } while (InterlockedCompareExchange(
                                (volatile LONG *) Atom,
                                nval,
                                oval) != oval);
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomGet
**
**  Get the 32-bit value protected by an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**  OUTPUT:
**
**      gctINT32_PTR Value
**          Pointer to a variable the receives the value of the atom.
*/
gceSTATUS
gckOS_AtomGet(
    IN gckOS Os,
    IN gctPOINTER Atom,
    OUT gctINT32_PTR Value
    )
{
    gcmkHEADER_ARG("Os=0x%x Atom=0x%0x", Os, Atom);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Increment the atom. */
    *Value = *(LONG volatile *) Atom;

    /* Success. */
    gcmkFOOTER_ARG("*Value=%d", *Value);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_AtomSet
**
**  Set the 32-bit value protected by an atom.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gctPOINTER Atom
**          Pointer to the atom.
**
**      gctINT32 Value
**          The value of the atom.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_AtomSet(
    IN gckOS Os,
    IN gctPOINTER Atom,
    IN gctINT32 Value
    )
{
    gcmkHEADER_ARG("Os=0x%x Atom=0x%0x Value=%d", Os, Atom, Value);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Atom != gcvNULL);

    /* Increment the atom. */
    *(LONG volatile *) Atom = Value;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetProcessID
**
**  Get current process ID.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      gctUINT32_PTR ProcessID
**          Pointer to the variable that receives the process ID.
*/
gceSTATUS
gckOS_GetProcessID(
    OUT gctUINT32_PTR ProcessID
    )
{
    gctUINT32 handle;
    gcmkHEADER();

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(ProcessID != gcvNULL);

#if UNDER_CE >= 600
    handle = GCHAL::GetObject()->GetCurrentPID();
    if (handle == 0)
    {
        gckOS_GetCurrentProcessID(&handle);
    }
#else
    gckOS_GetCurrentProcessID(&handle);
#endif

    gcmkASSERT(handle != 0);

    *ProcessID = (gctUINT32)handle;

    /* Success. */
    gcmkFOOTER_ARG("*ProcessID=%u", *ProcessID);
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_GetCurrentProcessID(
    OUT gctUINT32_PTR ProcessID
    )
{
    gctHANDLE handle;
    gcmkHEADER();

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(ProcessID != gcvNULL);

    /* Get process ID. */
#if UNDER_CE >= 600
    handle = (gctHANDLE)GetCallerVMProcessId();
#else
    /* Get current process handle. */
    handle = GetOwnerProcess();
#endif

    *ProcessID = (gctUINT32)handle;

    /* Success. */
    gcmkFOOTER_ARG("*ProcessID=%u", *ProcessID);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_GetThreadID
**
**  Get current thread ID.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      gctUINT32_PTR ThreadID
**          Pointer to the variable that receives the thread ID.
*/
gceSTATUS
gckOS_GetThreadID(
    OUT gctUINT32_PTR ThreadID
    )
{
    gcmkHEADER();

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(ThreadID != gcvNULL);

    /* Get thread ID. */
    *ThreadID = GetCurrentThreadId();

    /* Success. */
    gcmkFOOTER_ARG("*ThreadID=%u", *ThreadID);
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_SetGPUPower
**
**  Set the power of the GPU on or off.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gceCORE Core
**          GPU whose power is set.
**
**      gctBOOL Clock
**          gcvTRUE to turn on the clock, or gcvFALSE to turn off the clock.
**
**      gctBOOL Power
**          gcvTRUE to turn on the power, or gcvFALSE to turn off the power.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_SetGPUPower(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctBOOL Clock,
    IN gctBOOL Power
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d Clock=%d Power=%d", Os, Core, Clock, Power);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
**
**  gckOS_ResetGPU
**
**  Reset the GPU.
**
**  INPUT:
**
**      gckOS Os
**          Pointer to a gckOS object.
**
**      gckCORE Core
**          GPU whose power is set.
**
**  OUTPUT:
**
**      Nothing.
*/
gceSTATUS
gckOS_ResetGPU(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    gcmkHEADER_ARG("Os=0x%X Core=%d", Os, Core);

    gcmkFOOTER_NO();
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_DumpCallStack(
    IN gckOS Os
    )
{
    gcmkHEADER_ARG("Os=0x%X", Os);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}
/*----------------------------------------------------------------------------*/
/*----- Profile --------------------------------------------------------------*/

gceSTATUS
gckOS_GetProfileTick(
    OUT gctUINT64_PTR Tick
    )
{
    LARGE_INTEGER q;

    QueryPerformanceCounter(&q);
    *Tick = q.QuadPart;

    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_QueryProfileTickRate(
    OUT gctUINT64_PTR TickRate
    )
{
    LARGE_INTEGER q;

    QueryPerformanceFrequency(&q);
    *TickRate = q.QuadPart;

    return gcvSTATUS_OK;
}

gctUINT32
gckOS_ProfileToMS(
    IN gctUINT64 Ticks
    )
{
    static LARGE_INTEGER f;

    if (f.QuadPart == 0)
    {
        QueryPerformanceFrequency(&f);
    }

    return (gctUINT32) (1000 * Ticks / f.QuadPart);
}

gceSTATUS
gckOS_GetProcessNameByPid(
    IN gctINT Pid,
    IN gctSIZE_T Length,
    OUT gctUINT8_PTR String
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_PrepareGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_FinishGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_QueryGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core,
    OUT gctUINT32 * Frequency,
    OUT gctUINT8 * Scale
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_SetGPUFrequency(
    IN gckOS Os,
    IN gceCORE Core,
    IN gctUINT8 Scale
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_CreateKernelVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    gcsPHYSICAL * physical = (gcsPHYSICAL *)Physical;

    *PageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(physical->logical, Bytes);
    *Logical   = physical->logical;

    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_DestroyKernelVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_CreateUserVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Logical,
    OUT gctSIZE_T * PageCount
    )
{
    return gckOS_LockPages(Os, Physical, Bytes, gcvFALSE, Logical, PageCount);
}

gceSTATUS
gckOS_DestroyUserVirtualMapping(
    IN gckOS Os,
    IN gctPHYS_ADDR Physical,
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical
    )
{
    return gckOS_UnlockPages(Os, Physical, Bytes, Logical);
}

void
gckOS_DumpParam(
    void
    )
{
    return;
}

gceSTATUS
gckOS_CPUPhysicalToGPUPhysical(
    IN gckOS Os,
    IN gctPHYS_ADDR_T CPUPhysical,
    IN gctPHYS_ADDR_T * GPUPhysical
    )
{
    gcmkHEADER_ARG("CPUPhysical=0x%X", CPUPhysical);

    *GPUPhysical = CPUPhysical;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_GPUPhysicalToCPUPhysical(
    IN gckOS Os,
    IN gctUINT32 GPUPhysical,
    IN gctPHYS_ADDR_T * CPUPhysical
    )
{
    gcmkHEADER_ARG("GPUPhysical=0x%X", GPUPhysical);

    *CPUPhysical = GPUPhysical;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_GetFd(
    IN gctSTRING Name,
    IN gcsFDPRIVATE_PTR Private,
    OUT gctINT * Fd
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_QueryOption(
    IN gckOS Os,
    IN gctCONST_STRING Option,
    OUT gctUINT32 * Value
    )
{
    if (!strcmp(Option, "physBase"))
    {
        *Value = Os->gchal->GetBaseAddress();
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "physSize"))
    {
        *Value = Os->gchal->GetPhysSize();
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "contiguousSize"))
    {
        *Value = Os->gchal->GetContiguousSize();
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "contiguousBase"))
    {
        *Value = Os->gchal->GetContiguousBase();
        return gcvSTATUS_OK;
    }
    else if (!strcmp(Option, "mmu"))
    {
        *Value = 1;
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_WrapMemory(
    IN gckOS Os,
    IN gcsUSER_MEMORY_DESC_PTR Desc,
    OUT gctSIZE_T *Bytes,
    OUT gctPHYS_ADDR * Physical,
    OUT gctBOOL *Contiguous
    )
{
    gceSTATUS status;
    gctPOINTER logical;
    gcsPHYSICAL* physical = gcvNULL;
    gctUINT32 address;
    gctBOOL contiguous;
    gctPOINTER pointer = gcvNULL;
    gctBOOL mapped = gcvFALSE;
    gctSIZE_T pageCount, i;
    DWORD *physTable = gcvNULL;
    gctUINT32 processID;

    gcmkHEADER_ARG("Os=0x%X Desc=0x%X logical=0x%X, physical=0x%x, size=%d", \
        Os, Desc, (gctUINT32)Desc->logical, Desc->physical, Desc->size);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Desc != gcvNULL);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmkVERIFY_ARGUMENT(Physical != gcvNULL);
    gcmkVERIFY_ARGUMENT(Contiguous != gcvNULL);

    /* Verify the user memory info. */
    if ((Desc->physical == gcvINVALID_ADDRESS) && (Desc->logical == 0))
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Desc->size == 0)
    {
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    logical = gcmINT2PTR(Desc->logical);

    /* Check if the memory is physically contiguous. */
    if (Desc->physical != gcvINVALID_ADDRESS)
    {
        address = Desc->physical;

        contiguous = gcvTRUE;
    }
    else
    {
        address = gcmPTR2INT32(logical);

        contiguous = gcvTRUE;

        gcmkONERROR(gckOS_MapUserPointer(
            Os,
            logical,
            Desc->size,
            &pointer));

        mapped = gcvTRUE;

        /* Get the number of required pages. */
        pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(logical, Desc->size);

        /* Allocate the array of page addresses. */
        gcmkONERROR(gckOS_Allocate(
            Os,
            pageCount * sizeof(DWORD),
            (gctPOINTER *)&physTable));

        /* Lock the pages. */
        if (!LockPages(pointer, Desc->size, physTable, LOCKFLAG_QUERY_ONLY))
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        /* Loop the table. */
        for (i = 1; i < pageCount; i++)
        {
            if ((physTable[i - 1] << UserKInfo[KINX_PFN_SHIFT]) + PAGE_SIZE
                != (physTable[i] << UserKInfo[KINX_PFN_SHIFT]))
            {
                contiguous = gcvFALSE;
                break;
            }
        }

        gcmkOS_SAFE_FREE(Os, physTable);

        gcmkVERIFY_OK(gckOS_UnmapUserPointer(
            Os,
            logical,
            Desc->size,
            pointer));

        mapped = gcvFALSE;
    }

    /* Flush the data cache. */
    if (logical != gcvNULL)
    {
        gcmkONERROR(gckOS_GetProcessID(&processID));

        gcmkONERROR(gckOS_CacheFlush(Os,
                                     processID,
                                     gcvNULL,
                                     0,
                                     logical,
                                     Desc->size));
    }

    /* Allocate a new gcsPHYSICAL structure. */
    physical = new gcsPHYSICAL;
    if (physical == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    physical->type              = gcvPHYSICAL_TYPE_WRAPPED_MEMORY;
    physical->physical.LowPart  = Desc->physical;
    physical->physical.HighPart = 0;
    physical->logical           = logical;
    physical->bytes             = Desc->size;
    physical->reference         = 1;
#if gcdENABLE_2D
    physical->extraPage         = 2;
#else
    physical->extraPage         = (
            ((address + gcmALIGN(Desc->size + 64, 64) + PAGE_SIZE - 1) >> PAGE_SHIFT) >
            ((address + Desc->size + PAGE_SIZE - 1) >> PAGE_SHIFT)
            ) ? 1 : 0;
#endif

    /* The return values. */
    *Bytes      = Desc->size;
    *Physical   = (gctPHYS_ADDR) physical;
    *Contiguous = contiguous;

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=%d, *Physical=0x%08x, *Contiguous=%d", \
        gcmOPT_VALUE(Bytes), gcmOPT_VALUE(Physical), gcmOPT_VALUE(Contiguous));
    return gcvSTATUS_OK;

OnError:
    if (physTable != gcvNULL)
    {
        gcmkOS_SAFE_FREE(Os, physTable);
    }

    if (mapped)
    {
        gcmkVERIFY_OK(gckOS_UnmapUserPointer(
            Os,
            logical,
            Desc->size,
            pointer));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckOS_PhysicalToPhysicalAddress(
    IN gckOS Os,
    IN gctPOINTER Physical,
    IN gctUINT32 Offset,
    OUT gctPHYS_ADDR_T * PhysicalAddress
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gckOS_GetPolicyID(
    IN gckOS Os,
    IN gceSURF_TYPE Type,
    OUT gctUINT32_PTR PolicyID,
    OUT gctUINT32_PTR AXIConfig
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

