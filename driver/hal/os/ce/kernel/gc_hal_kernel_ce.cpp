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


#include <windows.h>
#include <ceddk.h>
#include <devload.h>
#include <nkintr.h>
#include "gc_hal_kernel.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel_ce.h"

// Reference counter.
GCHAL * GCHAL::m_Object = gcvNULL;
gctINT GCHAL::m_ReferenceCount = 0;

gctBOOL GCHAL::Lock()
{
    if (m_Mutex == gcvNULL)
    {
        return gcvFALSE;
    }

    if (WaitForSingleObject(m_Mutex, INFINITE) == WAIT_OBJECT_0)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL GCHAL::Unlock()
{
    if (m_Mutex == gcvNULL)
    {
        return gcvFALSE;
    }

    return ReleaseMutex(m_Mutex);
}

#if UNDER_CE >= 600
static DWORD g_ProcessID = TLS_OUT_OF_INDEXES;
static gctUINT32 g_KernelPID = 0;

gctUINT32 GCHAL::GetCurrentPID()
{
    return (gctUINT32)TlsGetValue(g_ProcessID);
}

gctPOINTER GCHAL::GetProcessContiguousLogical(void)
{
    gctPOINTER logical = gcvNULL;

    gcmkVERIFY_OK(gckOS_MapMemory(
                m_Os,
                (gctPHYS_ADDR)&m_Contiguous,
                m_Contiguous.bytes,
                &logical));

    return logical;
}
#endif

gctBOOL WINAPI
DllMain(
    HANDLE Instance,
    DWORD Reason,
    LPVOID Reserved
    )
{
    return gcvTRUE;
}

// Size of register space.
const gctSIZE_T c_RegisterSize = 256 << 10;

bool
GCHAL::AllocateMemory(
    IN gctSIZE_T Bytes,
    OUT gctPOINTER& Logical,
    OUT PHYSICAL_ADDRESS& Physical
    )
{
#if UNDER_CE >= 600
    // Allocate contiguous physical memory.
    Physical.QuadPart = 0;
    gctPOINTER contiguous = AllocPhysMem(Bytes,
                                         PAGE_READWRITE | PAGE_NOCACHE,
                                         0,
                                         0,
                                         &Physical.LowPart);
    if (contiguous == gcvNULL)
    {
        // Out of memory.
        goto OnError;
    }

    if ((Physical.LowPart < m_BaseAddress) || (Physical.HighPart != 0))
    {
        goto OnError;
    }

    Physical.LowPart -= m_BaseAddress;
    Logical = contiguous;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                  "GCHAL: Allocated %u bytes of memory @ %08X(%p)",
                  Bytes,
                  Physical.LowPart,
                  Logical);

    // Success.
    return true;

OnError:
    if (contiguous != gcvNULL)
    {
        // Free the physical memory.
        FreePhysMem(contiguous);
    }

    return false;
#else
    // Allocate contiguous physical memory.
    gctPOINTER contiguous = AllocPhysMem(Bytes,
                                         PAGE_READWRITE | PAGE_NOCACHE,
                                         0,
                                         0,
                                         &Physical.LowPart);

    if (contiguous == gcvNULL)
    {
        // Out of memory.
        return false;
    }

    // Reset upper 32-bits of physical address.
    Physical.HighPart = 0;

    // Reserve the shared memory.
    Logical = VirtualAlloc(gcvNULL, Bytes, MEM_RESERVE, PAGE_NOACCESS);

    if (Logical == gcvNULL)
    {
        // Free the physical memory.
        FreePhysMem(contiguous);

        // Out of memory.
        return false;
    }

    // Copy the contiguous pages into shared memory.
    if (!VirtualCopy(Logical,
                     contiguous,
                     Bytes,
                     PAGE_READWRITE | PAGE_NOCACHE))
    {
        // Free the shared memory.
        VirtualFree(Logical, 0, MEM_RELEASE);

        // Free the contiguous memory.
        FreePhysMem(contiguous);

        return false;
    }

    // Don't need the original memory anymore.
    FreePhysMem(contiguous);

    gcmkASSERT(Physical.LowPart >= m_BaseAddress);
    Physical.LowPart -= m_BaseAddress;
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                  "GCHAL: Allocated %u bytes of memory @ %08X(%p)",
                  Bytes,
                  Physical.LowPart,
                  Logical);

    // Success.
    return true;
#endif
}

bool GCHAL::FreeMemory(
    IN gctSIZE_T Bytes,
    IN gctPOINTER Logical,
    IN PHYSICAL_ADDRESS Physical
    )
{
#if UNDER_CE >= 600
    if (Logical)
    {
        if (FreePhysMem(Logical))
        {
            return TRUE;
        }
    }

    return FALSE;
#else
    // Free the virtual memory.
    return VirtualFree(Logical, Bytes, MEM_DECOMMIT | MEM_RELEASE) != FALSE;
#endif
}

// GCHAL constructor.
GCHAL::GCHAL(
    LPCTSTR RegistryPath
    )
{
    int i;
    gceSTATUS status;
    gceHARDWARE_TYPE type;
    gckDB sharedDB = gcvNULL;

    // Zero out the heaps.
    m_Internal.bytes   = 0;
    m_InternalHeap     = gcvNULL;
    m_External.bytes   = 0;
    m_ExternalHeap     = gcvNULL;
    m_Contiguous.bytes = 0;
    m_ContiguousHeap   = gcvNULL;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        m_Kernels[i]         = gcvNULL;
        m_InterruptIDs[i]    = INVALID_IRQ_NO;
        m_MemBases[i]        = 0;
    }

    // Set default values.
    m_Contiguous.bytes = 16 << 20;
    m_BaseAddress = 0;
    m_PhysSize = 0;

    m_PowerManagement  = gcvTRUE;
    m_GpuProfiler      = gcvFALSE;

    m_Device           = gcvNULL;

    // Read parameters from the registry.
    if (!ReadRegistry(RegistryPath))
    {
        throw false;
    }

    /* when enable gpu profiler, we need to turn off gpu powerMangement */
    if(m_GpuProfiler)
    {
        m_PowerManagement = gcvFALSE;
    }

#if UNDER_CE >= 600
    // No processes attached.
    m_Processes = gcvNULL;
#endif

#ifdef EMULATOR
    // No mapped address.
    m_LastMapping.mapped = gcvNULL;

#if !defined(DYNAMIC_MEMORY_CONTROLLER)
    // Reset C-Model.
    _Memory = new AQMEMORY;
    Reset();

    // Set context for AQMEMORY.
    LocalMemory.m_Hal = this;
#else
    // Create C-Model.
    Reset();

    // Set the emulator instance.
    MemoryCtrl->m_Hal = this;
#endif

    // Create the events.
    m_WaitEvent = CreateEvent(gcvNULL, gcvFALSE, gcvFALSE, gcvNULL);

    // No threads defined.
    m_CModelThread = gcvNULL;
#else
    // Start mapping at the base address of the chip.
    PHYSICAL_ADDRESS physical;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_MemBases[i] != 0)
        {
            physical.QuadPart = m_MemBases[i];

            // Map the registers.
#if gcdDEBUG
            m_RegisterBases[i] = (gctUINT8*) MmMapIoSpace(physical, 2<<20, gcvFALSE);
#else
            m_RegisterBases[i] = (gctUINT8*) MmMapIoSpace(physical, c_RegisterSize, gcvFALSE);
#endif

            gcmkASSERT(m_RegisterBases[i] != gcvNULL);
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                       "GCHAL: Registers allocated at %p", m_RegisterBases[i]);

            // Update physical address.
            physical.QuadPart += c_RegisterSize;
        }
        else
        {
            m_RegisterBases[i] = gcvNULL;
        }
#endif
    }

    // Create interrupt event.
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_InterruptIDs[i] != INVALID_IRQ_NO)
        {
            m_InterruptEvents[i]  = CreateEvent(gcvNULL, gcvFALSE, gcvFALSE, gcvNULL);
        }

        m_InterruptThreads[i] = gcvNULL;
    }

    m_Mutex = CreateMutex(gcvNULL, gcvFALSE, gcvNULL);
    if (m_Mutex == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
    }

#ifndef EMULATOR
    // Enable the interrupts.
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_InterruptIDs[i] != INVALID_IRQ_NO)
        {
            InterruptInitialize(m_InterruptIDs[i], m_InterruptEvents[i], gcvNULL, 0);
        }
    }

    // Start the interrupt threads.
    StartThreads();

    // Clear any lingering interrupts.
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_InterruptIDs[i] != INVALID_IRQ_NO)
        {
            InterruptDone(m_InterruptIDs[i]);
        }
    }
#endif


    // Construct the gckOS object.
    gcmkONERROR(gckOS_Construct(this, &m_Os));

    /* Construct the gckDEVICE object for os independent core management. */
    gcmkONERROR(gckDEVICE_Construct(m_Os, &m_Device));

    /* Setup contiguous vidmem */
    if (m_Contiguous.physical.LowPart != ~0U)
    {
        /* Construct the contiguous memory heap. */
        gcmkONERROR(gckVIDMEM_Construct(
            m_Os,
            m_Contiguous.physical.LowPart,
            m_Contiguous.bytes,
            64,
            0,
            &m_ContiguousHeap));

        m_Contiguous.type      = gcvPHYSICAL_TYPE_RESERVED_PHYSICAL;
        m_Contiguous.attr     |= gcvPHYSICAL_ATTR_CONTIGUOUS;
        m_Contiguous.reference = 1;
    }
    else
    {
        // Try to allocate contiguous memory, starting at the specified value,
        // decreasing 4MB each time until we proceed.
        for (; m_Contiguous.bytes > 0; m_Contiguous.bytes -= 4 << 20)
        {
            // Allocate contiguous memory. */
            if (AllocateMemory(m_Contiguous.bytes,
                               m_Contiguous.logical,
                               m_Contiguous.physical))
            {
                /* Construct the contiguous memory heap. */
                gceSTATUS status = gckVIDMEM_Construct(m_Os,
                                                     m_Contiguous.physical.LowPart |
                                                     m_SystemMemoryBaseAddress,
                                                     m_Contiguous.bytes,
                                                     64,
                                                     0,
                                                     &m_ContiguousHeap);

                if (gcmIS_SUCCESS(status))
                {

                    m_Contiguous.type      = gcvPHYSICAL_TYPE_PHYSICAL;
                    m_Contiguous.attr     |= gcvPHYSICAL_ATTR_CONTIGUOUS;
                    m_Contiguous.reference = 1;

                    /* Success, abort loop. */
                    break;
                }

                /* Free allocated contiguous memory. */
                FreeMemory(m_Contiguous.bytes, m_Contiguous.logical, m_Contiguous.physical);
            }
        }

        if (m_Contiguous.bytes == 0)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        else
        {
            gcmkONERROR(status);
        }
    }


    if (m_InterruptIDs[gcvCORE_MAJOR] != INVALID_IRQ_NO)
    {
        gckDEVICE_AddCore(m_Device, gcvCORE_MAJOR, gcvCHIP_ID_DEFAULT, this, &m_Kernels[gcvCORE_MAJOR]);

        // Set the power management
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(
            m_Kernels[gcvCORE_MAJOR]->hardware, m_PowerManagement
            ));

        gcmkONERROR(gckHARDWARE_SetGpuProfiler(
            m_Kernels[gcvCORE_MAJOR]->hardware, m_GpuProfiler
            ));
    }
    else
    {
        m_Kernels[gcvCORE_MAJOR] = gcvNULL;
    }

    if (m_InterruptIDs[gcvCORE_2D] != INVALID_IRQ_NO)
    {
        gckDEVICE_AddCore(m_Device, gcvCORE_2D, gcvCHIP_ID_DEFAULT, this, &m_Kernels[gcvCORE_2D]);

        /* Verify the hardware type */
        gcmkONERROR(gckHARDWARE_GetType(m_Kernels[gcvCORE_2D]->hardware, &type));

        if (type != gcvHARDWARE_2D)
        {
            gcmkTRACE_ZONE(
                gcvLEVEL_ERROR, gcvZONE_DRIVER,
                "%s(%d): Unexpected hardware type: %d\n",
                __FUNCTION__, __LINE__,
                type
                );

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        // Set the power management
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(
            m_Kernels[gcvCORE_2D]->hardware, m_PowerManagement
            ));
    }
    else
    {
        m_Kernels[gcvCORE_2D] = gcvNULL;
    }

    if (m_InterruptIDs[gcvCORE_VG] != INVALID_IRQ_NO)
    {
#if gcdENABLE_VG
        gckDEVICE_AddCore(m_Device, gcvCORE_VG, gcvCHIP_ID_DEFAULT, this, &m_Kernels[gcvCORE_VG]);

        // Set the power management
        gcmkONERROR(gckVGHARDWARE_EnablePowerManagement(
            m_Kernels[gcvCORE_VG]->vg->hardware,
            m_PowerManagement
            ));
#else
        m_Kernels[gcvCORE_VG] = gcvNULL;
#endif
    }
    else
    {
        m_Kernels[gcvCORE_VG] = gcvNULL;
    }

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_Kernels[i] != gcvNULL) break;
    }

    if (i == gcdMAX_GPU_COUNT) gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    if ((m_PhysSize != 0)
       && (m_Kernels[gcvCORE_MAJOR] != gcvNULL)
       && (m_Kernels[gcvCORE_MAJOR]->hardware->mmuVersion != 0))
    {
        /* Reset the base address */
        m_BaseAddress = 0;
    }

    gctUINT32 internalBaseAddress, internalAlignment;
    gctUINT32 externalBaseAddress, externalAlignment;
    gctUINT32 horizontalTileSize, verticalTileSize;

#if gcdENABLE_VG
    if (i == gcvCORE_VG)
    {
        // Query the ceiling of the system memory.
        gcmkONERROR(gckVGHARDWARE_QuerySystemMemory(m_Kernels[i]->vg->hardware,
                                                    &m_SystemMemorySize,
                                                    &m_SystemMemoryBaseAddress));

        // Query the amount of video memory.
        gcmkONERROR(gckVGHARDWARE_QueryMemory(m_Kernels[i]->vg->hardware,
                                              &m_Internal.bytes,
                                              &internalBaseAddress,
                                              &internalAlignment,
                                              &m_External.bytes,
                                              &externalBaseAddress,
                                              &externalAlignment,
                                              &horizontalTileSize,
                                              &verticalTileSize));
    }
    else
#endif
    {
        // Query the ceiling of the system memory.
        gcmkONERROR(gckHARDWARE_QuerySystemMemory(m_Kernels[i]->hardware,
                                                    &m_SystemMemorySize,
                                                    &m_SystemMemoryBaseAddress));

        // Query the amount of video memory.
        gcmkONERROR(gckHARDWARE_QueryMemory(m_Kernels[i]->hardware,
                                              &m_Internal.bytes,
                                              &internalBaseAddress,
                                              &internalAlignment,
                                              &m_External.bytes,
                                              &externalBaseAddress,
                                              &externalAlignment,
                                              &horizontalTileSize,
                                              &verticalTileSize));
    }

    if (m_Internal.bytes > 0)
    {
        // Create the internal memory heap.
        gceSTATUS status = gckVIDMEM_Construct(m_Os,
                                               internalBaseAddress,
                                               m_Internal.bytes,
                                               internalAlignment,
                                               0,
                                               &m_InternalHeap);

        if (gcmIS_ERROR(status))
        {
            // Error, remove internal heap.
            m_Internal.bytes = 0;
        }
        else
        {
#ifdef EMULATOR
            // Allocate internal memory.
            VERIFY(AllocateMemory(m_Internal.bytes,
                                  m_Internal.logical,
                                  m_Internal.physical));
#else
            // Map internal memory.
            m_Internal.physical  = physical;
            m_Internal.logical   = MmMapIoSpace(physical,
                                                m_Internal.bytes,
                                                gcvFALSE);
            physical.QuadPart += m_Internal.bytes;
#endif
            m_Internal.reference = 1;
            gcmkASSERT(m_Internal.logical != gcvNULL);
        }
    }

    if (m_External.bytes > 0)
    {
        // Create the external memory heap.
        gceSTATUS status = gckVIDMEM_Construct(m_Os,
                                             externalBaseAddress,
                                             m_External.bytes,
                                             externalAlignment,
                                             0,
                                             &m_ExternalHeap);

        if (gcmIS_ERROR(status))
        {
            // Error, remove external heap.
            m_External.bytes = 0;
        }
        else
        {
#ifdef EMULATOR
            // Allocate external memory.
            VERIFY(AllocateMemory(m_External.bytes,
                                  m_External.logical,
                                  m_External.physical));
#else
            // Map external memory.
            m_External.physical  = physical;
            m_External.logical   = MmMapIoSpace(physical,
                                                m_External.bytes,
                                                gcvFALSE);
#endif
            m_External.reference = 1;
            gcmkASSERT(m_External.logical != gcvNULL);
        }
    }

    if (m_InterruptIDs[gcvCORE_MAJOR] != INVALID_IRQ_NO)
    {
        RETAILMSG(gcvTRUE,
                 (TEXT("Major GPU: SysIntr=%d MemBases=0x%x MMU Version=%d\r\n"),
                  m_InterruptIDs[gcvCORE_MAJOR],
                  m_MemBases[gcvCORE_MAJOR],
                  m_Kernels[gcvCORE_MAJOR]->hardware->mmuVersion));
    }

    if (m_InterruptIDs[gcvCORE_2D] != INVALID_IRQ_NO)
    {
        RETAILMSG(gcvTRUE,
                 (TEXT("2D GPU: SysIntr=%d MemBases=0x%x MMU Version=%d\r\n"),
                  m_InterruptIDs[gcvCORE_2D],
                  m_MemBases[gcvCORE_2D],
                  m_Kernels[gcvCORE_2D]->hardware->mmuVersion));
    }

    if (m_InterruptIDs[gcvCORE_VG] != INVALID_IRQ_NO)
    {
        RETAILMSG(gcvTRUE,
                 (TEXT("VG GPU: SysIntr=%d MemBases=0x%x\r\n"),
                  m_InterruptIDs[gcvCORE_VG],
                  m_MemBases[gcvCORE_VG]));
    }

    RETAILMSG(gcvTRUE,
        (TEXT("Video memory: BaseAddress=0x%x PhysBase=0x%x size=0x%x physSize=0x%x\r\n"),
        m_BaseAddress,
        m_Contiguous.physical.LowPart,
        m_Contiguous.bytes,
        m_PhysSize));

    return;

OnError:
    throw false;
}

// GCHAL destructor.
GCHAL::~GCHAL(
    void
    )
{
    int i;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_Kernels[i] != gcvNULL)
        {
            if (i != gcvCORE_VG)
            {
                /* Switch to ON power state. */
                gcmkVERIFY_OK(
                    gckHARDWARE_SetPowerState(m_Kernels[i]->hardware,
                                              gcvPOWER_ON));
            }

#ifndef EMULATOR
            // Disable the interrupt.
            InterruptDisable(m_InterruptIDs[i]);
#endif

            // Stop the threads.
            StopThreads((gceCORE)i);

            // Delete the events.
            CloseHandle(m_InterruptEvents[i]);
        }
    }

    if (m_Internal.bytes > 0)
    {
        // Destroy the internal heap.
        gcmkVERIFY_OK(gckVIDMEM_Destroy(m_InternalHeap));

#ifdef EMULATOR
        // Free the internal memory.
        gcmkVERIFY(FreeMemory(m_Internal.bytes,
                             m_Internal.logical,
                             m_Internal.physical));
#else
        // Unmap the internal memory.
        MmUnmapIoSpace(m_Internal.logical, m_Internal.bytes);
#endif
    }

    if (m_External.bytes > 0)
    {
        // Destroy the external heap.
        gcmkVERIFY_OK(gckVIDMEM_Destroy(m_ExternalHeap));

#ifdef EMULATOR
        // Free the external memory.
        gcmkVERIFY(FreeMemory(m_External.bytes,
                             m_External.logical,
                             m_External.physical));
#else
        // Unmap the external memory.
        MmUnmapIoSpace(m_External.logical, m_External.bytes);
#endif
    }

    if (m_Contiguous.bytes > 0)
    {
        // Destroy the contiguous heap.
        gcmkVERIFY_OK(gckVIDMEM_Destroy(m_ContiguousHeap));

        if (m_Contiguous.type == gcvPHYSICAL_TYPE_RESERVED_PHYSICAL)
        {
            if (m_Contiguous.attr & gcvPHYSICAL_ATTR_IOMAPPED)
            {
                MmUnmapIoSpace(m_Contiguous.logical, m_Contiguous.bytes);
            }
        }
        else
        {
            // Free the contiguous memory.
            gcmkVERIFY(FreeMemory(m_Contiguous.bytes,
                                 m_Contiguous.logical,
                                 m_Contiguous.physical));
        }
    }

#ifdef EMULATOR
    CloseHandle(m_WaitEvent);

    // Delete C-Model.
    Quit();

#if !defined(DYNAMIC_MEMORY_CONTROLLER)
    delete _Memory;
#endif

    // Unmap any mapped memory.
    if (m_LastMapping.mapped != gcvNULL)
    {
        gcmkVERIFY(DeleteStaticMapping(m_LastMapping.mapped, PAGE_SIZE));
    }
#else
    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_RegisterBases[i] != gcvNULL)
        {
            // Unmap the register space.
            MmUnmapIoSpace(m_RegisterBases[i], c_RegisterSize);
        }
    }
#endif

    if (m_Mutex)
    {
        gcmkVERIFY(CloseHandle(m_Mutex));
    }

    gcmkVERIFY_OK(gckDEVICE_Destroy(m_Os, m_Device));

    // Destruct the gckOS object.
    gckOS_Destroy(m_Os);
}

extern "C" DWORD
GPU_Init(
    LPCTSTR registryPath
    )
{
    DWORD ret;

    RETAILMSG(gcvTRUE,
              (TEXT("GALCORE %d.%d.%d(%d)\r\n"),
              gcvVERSION_MAJOR,
              gcvVERSION_MINOR,
              gcvVERSION_PATCH,
              gcvVERSION_BUILD));

#if UNDER_CE >= 600
    g_ProcessID = TlsAlloc();
    gckOS_GetCurrentProcessID(&g_KernelPID);
    gcmkVERIFY(TlsSetValue(g_ProcessID, (LPVOID)g_KernelPID));
#endif

    ret = GCHAL::AddReference(registryPath);

    if (ret == 0)
    {
        RETAILMSG(gcvTRUE,
            (TEXT("GALCORE failed in initialization!\r\n")));
    }

    return ret;
}

extern "C" gctBOOL
GPU_Deinit(
    DWORD channelInterruptLoop
    )
{
#if UNDER_CE >= 600
    TlsFree(g_ProcessID);
    g_ProcessID = TLS_OUT_OF_INDEXES;
    g_KernelPID = 0;
#endif

    GCHAL::Release();
    return gcvTRUE;
}


extern "C" DWORD
GPU_Open(
    DWORD Reference,
    DWORD access,
    DWORD shareMode
    )
{
    int i;
    GCHAL *gchal = GCHAL::GetObject();

    if (gchal == gcvNULL)
    {
        return (DWORD)INVALID_HANDLE_VALUE;
    }

    if ((Reference == IOCTL_GCHAL_KERNEL_INTERFACE)
        && (access == IOCTL_GCHAL_KERNEL_INTERFACE)
        && (shareMode == IOCTL_GCHAL_KERNEL_INTERFACE))
    {

        if (gchal->GetKernel(gcvCORE_MAJOR) == gcvNULL
                 && gchal->GetKernel(gcvCORE_2D) == gcvNULL
                 && gchal->GetKernel(gcvCORE_VG) == gcvNULL)
        {
            return (DWORD)INVALID_HANDLE_VALUE;
        }
    }
#if UNDER_CE >= 600
    gctUINT32 processId;
    gckOS_GetCurrentProcessID(&processId);
    gcmkVERIFY(TlsSetValue(g_ProcessID, (LPVOID)processId));
#endif

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (GCHAL::GetObject()->GetKernel((gceCORE)i) != gcvNULL)
        {
            if (gcmIS_ERROR(gckKERNEL_AttachProcess(GCHAL::GetObject()->GetKernel((gceCORE)i),
                                                    gcvTRUE)))
            {
                gcmkTRACE(gcvLEVEL_ERROR, "Process attach failed\n");
            }
        }
    }

    return (DWORD)INVALID_HANDLE_VALUE;
}

#ifdef MEMORY_STAT
extern "C" void
OnProcessExit(
    IN gckOS Os
    );
#endif // MEMORY_STAT

extern "C" gctBOOL
GPU_Close(
    DWORD Reference
    )
{
    int i;

    for (i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (GCHAL::GetObject()->GetKernel((gceCORE)i) != gcvNULL)
        {
            if (gcmIS_ERROR(gckKERNEL_AttachProcess(GCHAL::GetObject()->GetKernel((gceCORE)i),
                                                    gcvFALSE)))
            {
                gcmkTRACE(gcvLEVEL_ERROR, "Process detatch failed\n");
            }
        }
    }

#if UNDER_CE >= 600
    if (Reference != IOCTL_GCHAL_KERNEL_INTERFACE)
    {
        GCHAL::GetObject()->UnmapAllUserMemory(GetCallerVMProcessId());
    }
#endif

#ifdef MEMORY_STAT
    OnProcessExit(GCHAL::GetObject()->GetOs());
#endif // MEMORY_STAT

#if UNDER_CE >= 600
    gcmkVERIFY(TlsSetValue(g_ProcessID, 0));
#endif

    return gcvTRUE;
}

extern "C" DWORD
GPU_Read(
    DWORD Reference,
    LPVOID buffer,
    DWORD numRead
    )
{
    return -1;
}

extern "C" DWORD
GPU_Write(
    DWORD Reference,
    LPCVOID buffer,
    DWORD numBytes
    )
{
    return -1;
}

extern "C" gctBOOL
GPU_IOControl(
    DWORD Reference,
    DWORD IoControlCode,
    PBYTE InputBuffer,
    DWORD InputBufferSize,
    PBYTE OutputBuffer,
    DWORD OutputBufferSize,
    PDWORD ReturnedBytes
    )
{
    gceSTATUS status;

    /* Make sure it is an gcsHAL_INTERFACE call. */
    if ( ((IoControlCode != IOCTL_GCHAL_INTERFACE) &&
         (IoControlCode != IOCTL_GCHAL_KERNEL_INTERFACE)) ||
         (InputBufferSize != sizeof(gcsHAL_INTERFACE)) ||
         (OutputBufferSize != sizeof(gcsHAL_INTERFACE)) )
    {
        /* Error. */
        return gcvFALSE;
    }

#if UNDER_CE >= 600
    gctUINT32 processId;

    gckOS_GetCurrentProcessID(&processId);

    if (IoControlCode == IOCTL_GCHAL_KERNEL_INTERFACE)
    {
        gcmkVERIFY(TlsSetValue(g_ProcessID, (LPVOID)g_KernelPID));
    }
    else
    {
        gcmkVERIFY(TlsSetValue(g_ProcessID, (LPVOID)processId));
    }
#endif

    gcsHAL_INTERFACE * iface = (gcsHAL_INTERFACE *) InputBuffer;

    status = gckDEVICE_Dispatch(GCHAL::GetObject()->GetDevice(), iface);

#if UNDER_CE >= 600
    if ((IoControlCode == IOCTL_GCHAL_KERNEL_INTERFACE)
        && (processId != g_KernelPID))
    {
        gcmkVERIFY(TlsSetValue(g_ProcessID, (LPVOID)processId));
    }
#endif

    /* Success. */
    return gcvTRUE;
}

extern "C" void
GPU_PowerDown(
    DWORD hContext
    )
{
    for (int i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (GCHAL::GetObject()->GetKernel((gceCORE)i) != gcvNULL)
        {
#if gcdENABLE_VG
            if (i != gcvCORE_VG)
#endif
            {
                if (gcmIS_ERROR(gckHARDWARE_SetPowerState(
                        GCHAL::GetObject()->GetKernel((gceCORE)i)->hardware,
                        gcvPOWER_OFF)))
                {
                    gcmkTRACE(gcvLEVEL_ERROR, "Power off failed\n");
                }
            }
        }
    }
}

extern "C" void
GPU_PowerUp(
    DWORD hContext
    )
{
    for (int i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (GCHAL::GetObject()->GetKernel((gceCORE)i) != gcvNULL)
        {
#if gcdENABLE_VG
            if (i != gcvCORE_VG)
#endif
            {
                if (gcmIS_ERROR(gckHARDWARE_SetPowerState(
                        GCHAL::GetObject()->GetKernel((gceCORE)i)->hardware,
                        gcvPOWER_ON)))
                {
                    gcmkTRACE(gcvLEVEL_ERROR, "Power on failed\n");
                }
            }
        }
    }
}

extern "C" DWORD
GPU_Seek(
    DWORD hOpenContext,
    long Amount,
    WORD Type
    )
{
    return -1;
}

void
GCHAL::StartThreads(
    void
    )
{
#ifdef EMULATOR
    if (m_CModelThread == gcvNULL)
    {
        // Create C-Model thread.
        m_CModelThread = CreateThread(gcvNULL, 0, CModelLoop, this, 0, gcvNULL);
    }
#endif

    for (int i = 0; i < gcdMAX_GPU_COUNT; i++)
    {
        if (m_InterruptIDs[i] != INVALID_IRQ_NO
            && m_InterruptThreads[i] == gcvNULL)
        {
            switch (i)
            {
            case gcvCORE_MAJOR:
                // Create interrupt thread.
                m_InterruptThreads[i] = CreateThread(gcvNULL, 0, InterruptLoop, this, 0, gcvNULL);
                break;

            case gcvCORE_2D:
                // Create interrupt thread.
                m_InterruptThreads[i] = CreateThread(gcvNULL, 0, InterruptLoop2D, this, 0, gcvNULL);
                break;

            case gcvCORE_VG:
                // Create interrupt thread for VG.
                m_InterruptThreads[i] = CreateThread(gcvNULL, 0, InterruptLoopVG, this, 0, gcvNULL);
                break;

            default:
                break;
            }

            if (m_InterruptThreads[i] != gcvNULL)
            {
                // Boost the thread priority.
                CeSetThreadPriority(m_InterruptThreads[i], 152);
            }
        }
    }
}

void
GCHAL::StopThreads(
    gceCORE Core
    )
{
#ifdef EMULATOR
    if (m_CModelThread != gcvNULL)
    {
        // Terminate C-Model thread.
        TerminateThread(m_CModelThread, 0);
        m_CModelThread = gcvNULL;
    }
#endif

    if (m_InterruptThreads[Core] != gcvNULL)
    {
        // Terminate niterrupt thread.
        TerminateThread(m_InterruptThreads[Core], 0);
        m_InterruptThreads[Core] = gcvNULL;
    }
}

DWORD
GCHAL::InterruptLoop(
    LPVOID Context
    )
{
    gceSTATUS status;
    GCHAL* gchal = (GCHAL*) Context;

    // Wait for interrupt event from C-Model.
    while (WaitForSingleObject(gchal->m_InterruptEvents[gcvCORE_MAJOR],
                               INFINITE) == WAIT_OBJECT_0)
    {
        gcmkVERIFY_OK(gckOS_SuspendInterruptEx(gchal->m_Os, gcvCORE_MAJOR));

        status = gckHARDWARE_Interrupt(gchal->m_Kernels[gcvCORE_MAJOR]->hardware);

        gcmkVERIFY_OK(gckOS_ResumeInterruptEx(gchal->m_Os, gcvCORE_MAJOR));

        // Notify HAL of interrupt.
        if (gcmIS_SUCCESS(status))
        {
            gckKERNEL_Notify(gchal->m_Kernels[gcvCORE_MAJOR], gcvNOTIFY_INTERRUPT);
        }

#ifndef EMULATOR
        // Interrupt handled.
        InterruptDone(gchal->m_InterruptIDs[gcvCORE_MAJOR]);
#endif
    }

    // Success.
    return STATUS_SUCCESS;
}

DWORD
GCHAL::InterruptLoop2D(
    LPVOID Context
    )
{
    gceSTATUS status;
    GCHAL* gchal = (GCHAL*) Context;

    // Wait for interrupt event from C-Model.
    while (WaitForSingleObject(gchal->m_InterruptEvents[gcvCORE_2D],
                               INFINITE) == WAIT_OBJECT_0)
    {
        gcmkVERIFY_OK(gckOS_SuspendInterruptEx(gchal->m_Os, gcvCORE_2D));

        status = gckHARDWARE_Interrupt(gchal->m_Kernels[gcvCORE_2D]->hardware);

        gcmkVERIFY_OK(gckOS_ResumeInterruptEx(gchal->m_Os, gcvCORE_2D));

        // Notify HAL of interrupt.
        if (gcmIS_SUCCESS(status))
        {
            gckKERNEL_Notify(gchal->m_Kernels[gcvCORE_2D], gcvNOTIFY_INTERRUPT);
        }

#ifndef EMULATOR
        // Interrupt handled.
        InterruptDone(gchal->m_InterruptIDs[gcvCORE_2D]);
#endif
    }

    // Success.
    return STATUS_SUCCESS;
}


DWORD
GCHAL::InterruptLoopVG(
    LPVOID Context
    )
{
    GCHAL* gchal = (GCHAL*) Context;

    // Wait for interrupt event from C-Model.
    while (WaitForSingleObject(gchal->m_InterruptEvents[gcvCORE_VG],
                               INFINITE) == WAIT_OBJECT_0)
    {
        gcmkVERIFY_OK(gckOS_SuspendInterruptEx(gchal->m_Os, gcvCORE_VG));

#if gcdENABLE_VG
        gcmkVERIFY_OK(gckVGINTERRUPT_Enque(gchal->m_Kernels[gcvCORE_VG]->vg->interrupt));
#endif

        gcmkVERIFY_OK(gckOS_ResumeInterruptEx(gchal->m_Os, gcvCORE_VG));

#ifndef EMULATOR
        // Interrupt handled.
        InterruptDone(gchal->m_InterruptIDs[gcvCORE_VG]);
#endif
    }

    // Success.
    return STATUS_SUCCESS;
}

gceSTATUS
GCHAL::MapInUser(
    IN gcsPHYSICAL * Physical,
    OUT gctPOINTER * Logical
    )
{
    if (!Logical)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

#if UNDER_CE >= 600
    PROCESS * p = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    DWORD processId = GetCallerVMProcessId();

    gcmkVERIFY(Lock());

    for (p = m_Processes; p != gcvNULL; p = p->next)
    {
        if ( (p->process == processId) &&
             (p->Physical == Physical))
        {
            ++ p->reference;
            *Logical = p->logical;

            gcmkVERIFY(Unlock());

            return gcvSTATUS_OK;
        }
    }

    gcmkVERIFY(Unlock());

    p = new PROCESS;
    if (!p)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    p->next      = gcvNULL;
    p->process   = processId;
    p->Physical  = Physical;
    p->reference = 1;
    p->mapped    = gcvTRUE;

    if ((Physical->type == gcvPHYSICAL_TYPE_WRAPPED_MEMORY) &&
        (Physical->logical != gcvNULL))
    {
        p->logical = Physical->logical;
        p->mapped  = gcvFALSE;
    }
    else
    {
        p->logical = VirtualAllocEx(
                                    HANDLE(processId),
                                    NULL,
                                    Physical->bytes,
                                    MEM_RESERVE,
                                    PAGE_READWRITE | PAGE_NOCACHE
                                    );
        if (p->logical == gcvNULL)
        {
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        if (Physical->logical)
        {
            if (!VirtualCopyEx(
                                HANDLE(processId),
                                p->logical,
                                GetCurrentProcess(),
                                Physical->logical,
                                Physical->bytes,
                                PAGE_READWRITE | PAGE_NOCACHE
                                ))
            {
                gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
        }
        else if (Physical->physical.LowPart != ~0U)
        {
            if (!VirtualCopyEx(
                                HANDLE(processId),
                                p->logical,
                                GetCurrentProcess(),
                                LPVOID(Physical->physical.LowPart >> 8),
                                Physical->bytes,
                                PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL
                                ))
            {
                gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
        }
        else
        {
            gcmkONERROR(gcvSTATUS_INVALID_ADDRESS);
        }

#ifdef ARMV7
        // Change memory to be non-cacheable & bufferable.
        VirtualSetAttributesEx(HANDLE(processId), p->logical, Physical->bytes, 0x040, 0x1CC, NULL);
#endif
    }

    // Insert to m_Processes.
    gcmkVERIFY(Lock());

    p->next = m_Processes;
    m_Processes = p;
    ++Physical->reference;

    gcmkVERIFY(Unlock());

    *Logical = p->logical;

OnError:

    if (status != gcvSTATUS_OK)
    {
        if (p != gcvNULL)
        {
            if (p->mapped && (p->logical != gcvNULL))
            {
                gcmkVERIFY(VirtualFreeEx(
                    HANDLE(p->process),
                    p->logical,
                    Physical->bytes,
                    MEM_DECOMMIT));

                gcmkVERIFY(VirtualFreeEx(
                    HANDLE(p->process),
                    p->logical,
                    0,
                    MEM_RELEASE));
            }

            delete p;
        }
    }

    return status;
#else

    gcmkVERIFY(Lock());

    if ((Physical->reference == 1)
        && (Physical->physical.QuadPart != ~0)
        && (Physical->logical == gcvNULL)
        && !(Physical->attr & gcvPHYSICAL_ATTR_IOMAPPED))
    {
        // Map physical address.
        Physical->logical = MmMapIoSpace(Physical->physical, Physical->bytes, FALSE);

        if (Physical->logical == gcvNULL)
        {
            gcmkVERIFY(Unlock());
            return gcvSTATUS_OUT_OF_RESOURCES;
        }

        Physical->attr |= gcvPHYSICAL_ATTR_IOMAPPED;
    }

    if (!Physical->logical)
    {
        gcmkVERIFY(Unlock());
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    ++Physical->reference;

    *Logical = Physical->logical;

    gcmkVERIFY(Unlock());

    return gcvSTATUS_OK;
#endif
}

gceSTATUS
GCHAL::UnmapFromUser(
    IN gcsPHYSICAL * Physical,
    IN gctPOINTER Logical
    )
{
#if UNDER_CE >= 600
    gcmkVERIFY(Lock());
    for (PROCESS *p = m_Processes, *last = gcvNULL; p != NULL; p = p->next)
    {
        if ((Logical == p->logical) && (p->Physical == Physical))
        {
            --p->reference;

            if (p->reference == 0)
            {
                if (p->mapped)
                {
                    gcmkVERIFY(VirtualFreeEx(HANDLE(p->process),
                                            Logical,
                                            Physical->bytes,
                                            MEM_DECOMMIT));

                    gcmkVERIFY(VirtualFreeEx(HANDLE(p->process),
                                            Logical,
                                            0,
                                            MEM_RELEASE));
                }

                if (last == gcvNULL)
                {
                    m_Processes = p->next;
                }
                else
                {
                    last->next = p->next;
                }

                --Physical->reference;

                delete p;
            }

            gcmkVERIFY(Unlock());
            return gcvSTATUS_OK;
        }

        last = p;
    }

    gcmkVERIFY(Unlock());
    return gcvSTATUS_NOT_FOUND;
#else

    if (Logical != Physical->logical)
    {
        return gcvSTATUS_INVALID_ADDRESS;
    }

    gcmkVERIFY(Lock());

    // Decrement reference.
    --Physical->reference;

    if ((Physical->reference == 1)
        && (Physical->logical != gcvNULL)
        && (Physical->attr & gcvPHYSICAL_ATTR_IOMAPPED))
    {
        // Unmap physical address.
        MmUnmapIoSpace(Physical->logical, Physical->bytes);
        Physical->logical = gcvNULL;
        Physical->attr &= ~gcvPHYSICAL_ATTR_IOMAPPED;
    }

    gcmkVERIFY(Unlock());
    return gcvSTATUS_OK;
#endif
}

gceSTATUS
GCHAL::UnmapAllUserMemory(
    IN DWORD ProcessId
    )
{
#if UNDER_CE >= 600
    PROCESS *p, *prev = gcvNULL, *next = gcvNULL;

    gcmkVERIFY(Lock());

    p = m_Processes;

    while (p != gcvNULL)
    {
        next = p->next;

        if (ProcessId == p->process)
        {
            if (p->mapped)
            {
                gckOS_DebugTrace(gcvLEVEL_WARNING, "Unmap 0x%08x(%d) for process 0x%08x.\n",
                    p->logical, p->Physical->bytes, ProcessId);

                gcmkVERIFY(VirtualFreeEx(
                                        HANDLE(p->process),
                                        p->logical,
                                        p->Physical->bytes,
                                        MEM_DECOMMIT));

                gcmkVERIFY(VirtualFreeEx(
                                        HANDLE(p->process),
                                        p->logical,
                                        0,
                                        MEM_RELEASE));
            }

            if (m_Processes == p)
            {
                m_Processes = next;
            }
            else
            {
                gcmkASSERT(prev != gcvNULL);

                prev->next = next;
            }

            delete p;
        }
        else
        {
            prev = p;
        }

        p = next;
    }

    gcmkVERIFY(Unlock());
#endif

    return gcvSTATUS_OK;
}

bool
GCHAL::ReadRegistry(
    LPCTSTR RegistryPath
    )
{
    // Open the device key.
    HKEY key = OpenDeviceKey(RegistryPath);

    // Query the SYSINTR value.
    DWORD valueType, valueSize = sizeof(m_InterruptIDs[gcvCORE_MAJOR]);
    DWORD irq;
    DWORD pm;
    DWORD pf;

    if (ERROR_SUCCESS != RegQueryValueEx(key,
                                         DEVLOAD_SYSINTR_VALNAME,
                                         gcvNULL,
                                         &valueType,
                                         (LPBYTE) &m_InterruptIDs[gcvCORE_MAJOR],
                                         &valueSize))
    {
        if (ERROR_SUCCESS == RegQueryValueEx(key,
                                             DEVLOAD_IRQ_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &irq,
                                             &valueSize))
        {
            if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                                 &irq,
                                 valueSize,
                                 &m_InterruptIDs[gcvCORE_MAJOR],
                                 valueSize,
                                 NULL))
            {
                RETAILMSG(gcvTRUE,
                          (TEXT("%s: ERROR: failed to request sysintr. (%s=%d)\r\n"),
                          TEXT("GCHAL::ReadRegistry"),
                          DEVLOAD_IRQ_VALNAME, irq));

                return false;
            }
        }
    }

    if (ERROR_SUCCESS == RegQueryValueEx(key,
                                         TEXT("Irq2D"),
                                         gcvNULL,
                                         &valueType,
                                         (LPBYTE) &irq,
                                         &valueSize))
    {
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                             &irq,
                             valueSize,
                             &m_InterruptIDs[gcvCORE_2D],
                             valueSize,
                             NULL))
        {
            RETAILMSG(gcvTRUE,
                      (TEXT("%s: ERROR: failed to request sysintr. (%s=%d)\r\n"),
                      TEXT("GCHAL::ReadRegistry"),
                      TEXT("Irq2D"), irq));

            return false;
        }
    }

    if (ERROR_SUCCESS == RegQueryValueEx(key,
                                         TEXT("IrqVG"),
                                         gcvNULL,
                                         &valueType,
                                         (LPBYTE) &irq,
                                         &valueSize))
    {
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
                             &irq,
                             valueSize,
                             &m_InterruptIDs[gcvCORE_VG],
                             valueSize,
                             NULL))
        {
            RETAILMSG(gcvTRUE,
                      (TEXT("%s: ERROR: failed to request sysintr. (%s=%d)\r\n"),
                      TEXT("GCHAL::ReadRegistry"),
                      TEXT("IrqVG"), irq));

            return false;
        }
    }

    if (m_InterruptIDs[gcvCORE_MAJOR] == INVALID_IRQ_NO
        && m_InterruptIDs[gcvCORE_2D] == INVALID_IRQ_NO
        && m_InterruptIDs[gcvCORE_VG] == INVALID_IRQ_NO)
    {
        RETAILMSG(gcvTRUE,
                  (TEXT("%s: failed to read any irq from registry.\r\n"),
                  TEXT("GCHAL::ReadRegistry")));

        return false;
    }

    // Query the MEMBASE values.
    if (m_InterruptIDs[gcvCORE_MAJOR] != INVALID_IRQ_NO)
    {
        valueSize = sizeof(m_MemBases[gcvCORE_MAJOR]);

        if (ERROR_SUCCESS != RegQueryValueEx(key,
                                             DEVLOAD_MEMBASE_VALNAME,
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &m_MemBases[gcvCORE_MAJOR],
                                             &valueSize))
        {
            RETAILMSG(gcvTRUE,
                      (TEXT("%s: ERROR: failed to read %s from registry.\r\n"),
                      TEXT("GCHAL::ReadRegistry"),
                      DEVLOAD_MEMBASE_VALNAME));

            return false;
        }
    }

    if (m_InterruptIDs[gcvCORE_2D] != INVALID_IRQ_NO)
    {
        valueSize = sizeof(m_MemBases[gcvCORE_2D]);

        if (ERROR_SUCCESS != RegQueryValueEx(key,
                                             TEXT("MemBase2D"),
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &m_MemBases[gcvCORE_2D],
                                             &valueSize))
        {
            RETAILMSG(gcvTRUE,
                      (TEXT("%s: ERROR: failed to read %s from registry.\r\n"),
                      TEXT("GCHAL::ReadRegistry"),
                      TEXT("MemBase2D")));

            return false;
        }
    }

    if (m_InterruptIDs[gcvCORE_VG] != INVALID_IRQ_NO)
    {
        valueSize = sizeof(m_MemBases[gcvCORE_VG]);

        if (ERROR_SUCCESS != RegQueryValueEx(key,
                                             TEXT("MemBaseVG"),
                                             gcvNULL,
                                             &valueType,
                                             (LPBYTE) &m_MemBases[gcvCORE_VG],
                                             &valueSize))
        {
            RETAILMSG(gcvTRUE,
                      (TEXT("%s: ERROR: failed to read %s from registry.\r\n"),
                      TEXT("GCHAL::ReadRegistry"),
                      TEXT("MemBaseVG")));

            return false;
        }
    }

    // Query the ContiguousSize value.
    valueSize = sizeof(m_Contiguous.bytes);
    if (ERROR_SUCCESS == RegQueryValueEx(key,
                                         TEXT("ContiguousSize"),
                                         gcvNULL,
                                         &valueType,
                                         (LPBYTE) &m_Contiguous.bytes,
                                         &valueSize))
    {
        if (m_Contiguous.bytes)
        {
            // Query the ContiguousBase value.
            valueSize = sizeof(m_Contiguous.physical.LowPart);
            RegQueryValueEx(key,
                            TEXT("ContiguousBase"),
                            gcvNULL,
                            &valueType,
                            (LPBYTE) &m_Contiguous.physical.LowPart,
                            &valueSize);
        }
    }

    // Query the BaseAddress value.
    valueSize = sizeof(m_BaseAddress);
    RegQueryValueEx(key,
        TEXT("BaseAddress"),
        gcvNULL,
        &valueType,
        (LPBYTE) &m_BaseAddress,
        &valueSize);

    // Query the BaseAddress value.
    valueSize = sizeof(m_PhysSize);
    RegQueryValueEx(key,
        TEXT("physSize"),
        gcvNULL,
        &valueType,
        (LPBYTE) &m_PhysSize,
        &valueSize);

    // Query the power management setting.
    valueSize = sizeof(pm);
    if (ERROR_SUCCESS == RegQueryValueEx(key,
                                        TEXT("PowerManagement"),
                                        gcvNULL,
                                        &valueType,
                                        (LPBYTE) &pm,
                                        &valueSize))
    {
        m_PowerManagement = (pm != 0);
    }

    // Query the gpu profiler setting.
    valueSize = sizeof(pf);
    if (ERROR_SUCCESS == RegQueryValueEx(key,
                                        TEXT("GpuProfiler"),
                                        gcvNULL,
                                        &valueType,
                                        (LPBYTE) &pf,
                                        &valueSize))
    {
        m_GpuProfiler = (pf != 0);
    }

    // Close the device key.
    RegCloseKey(key);

    // Return error status.
    return true;
}
