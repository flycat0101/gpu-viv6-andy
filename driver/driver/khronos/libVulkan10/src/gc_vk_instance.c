/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vk_precomp.h"

extern gctPOINTER  __vkRootMutex;
extern __vkInstance *__vkInstanceRoot;

VKAPI_ATTR void* VKAPI_CALL __vk_AllocSystemMemory(
    void *pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope
    );

VKAPI_ATTR void VKAPI_CALL __vk_FreeSystemMemory(
    void *pUserData,
    void *pMem
    );

#if DEBUG_VULKAN_ALLOCATOR
VKAPI_ATTR void* VKAPI_CALL __vk_AllocSystemMemoryWrapper(
    void *pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope
    );

VKAPI_ATTR void VKAPI_CALL __vk_FreeSystemMemoryWrapper(
    void *pUserData,
    void *pMem
    );

uint32_t g_vkAllocations = 0;

VkAllocationCallbacks __vkAllocatorWrapper =
{
    gcvNULL,
    __vk_AllocSystemMemoryWrapper,
    gcvNULL,
    __vk_FreeSystemMemoryWrapper,
    gcvNULL,
    gcvNULL
};

VKAPI_ATTR void* VKAPI_CALL __vk_AllocSystemMemoryWrapper(
    void *pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope
    )
{
    VkAllocationCallbacks *pAllocator = (VkAllocationCallbacks *)pUserData;
    void *      pMem = gcvNULL;

    pMem = pAllocator->pfnAllocation(pAllocator->pUserData, size, alignment, scope);

    if (pMem)
    {
        g_vkAllocations++;
        __VK_DEBUG_PRINT(0, "Allocate SUCCESS:: # of Allocations = %p, pUserData = %p,"
            "pfnAllocate = %p, pfnFree = %p, pMem = %p, size = %p\n",
            g_vkAllocations, pAllocator->pUserData, pAllocator->pfnAllocation, pAllocator->pfnFree, pMem, size);
    }
    else
    {
        __VK_DEBUG_PRINT(0, "Allocate FAILED :: # of Allocations = %p, pUserData = %p,"
            "pfnAllocate = %p, pfnFree = %p, pMem = %p, size = %p\n",
            g_vkAllocations, pAllocator->pUserData, pAllocator->pfnAllocation, pAllocator->pfnFree, pMem, size);

    }

    return pMem;
}

VKAPI_ATTR void VKAPI_CALL __vk_FreeSystemMemoryWrapper(
    void *pUserData,
    void *pMem
    )
{
    VkAllocationCallbacks *pAllocator = (VkAllocationCallbacks *)pUserData;

    if (pMem)
    {
        g_vkAllocations--;
        __VK_DEBUG_PRINT(0, " Free            :: # of Allocations = %p, pUserData = %p, pfnAllocate = %p, pfnFree = %p, pMem = %p\n",
            g_vkAllocations, pAllocator->pUserData, pAllocator->pfnAllocation, pAllocator->pfnFree, pMem);
    }

    pAllocator->pfnFree(pAllocator->pUserData, pMem);
}

#endif

static const VkExtensionProperties g_InstanceExtensions[] =
{
    {VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
    {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_SPEC_VERSION},
    {VK_KHR_DISPLAY_EXTENSION_NAME, VK_KHR_DISPLAY_SPEC_VERSION},
#ifdef VK_USE_PLATFORM_WIN32_KHR
    {VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_SPEC_VERSION},
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    {VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_SPEC_VERSION},
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    {VK_KHR_MIR_SURFACE_EXTENSION_NAME, VK_KHR_MIR_SURFACE_SPEC_VERSION},
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    {VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_SPEC_VERSION},
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    {VK_KHR_XCB_SURFACE_EXTENSION_NAME, VK_KHR_XCB_SURFACE_SPEC_VERSION},
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    {VK_KHR_XLIB_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_SPEC_VERSION},
#endif
};

static VkAllocationCallbacks __vkAllocator =
{
    gcvNULL,
    __vk_AllocSystemMemory,
    gcvNULL,
    __vk_FreeSystemMemory,
    gcvNULL,
    gcvNULL
};

VKAPI_ATTR void* VKAPI_CALL __vk_AllocSystemMemory(
    void *pUserData,
    size_t size,
    size_t alignment,
    VkSystemAllocationScope scope
    )
{
    void *      pMem = gcvNULL;
    gceSTATUS   status;

    status = gcoOS_Allocate(gcvNULL, size, &pMem);

    if (gcmIS_ERROR(status))
    {
        pMem = gcvNULL;
    }

    return pMem;
}

VKAPI_ATTR void VKAPI_CALL __vk_FreeSystemMemory(
    void *pUserData,
    void *pMem
    )
{
    gcoOS_Free(gcvNULL, pMem);
}

static VkResult __vki_InitializeChipInfo(
    __vkInstance *inst
    )
{
    gcsHAL_INTERFACE iface;
    uint32_t chipCount, i;
    __vkChipInfo *chipInfo = &inst->chipInfo;
    VkResult result = VK_SUCCESS;

    iface.command = gcvHAL_CHIP_INFO;
    __VK_ONERROR(__vk_DeviceControl(&iface, 0));
    chipCount = iface.u.ChipInfo.count;

    for (i = 0; i < chipCount; i++)
    {
        switch (iface.u.ChipInfo.types[i])
        {
        case gcvHARDWARE_3D:
        case gcvHARDWARE_3D2D:
            chipInfo->gpuCoreID[chipInfo->gpuCoreCount++] = iface.u.ChipInfo.ids[i];
            __VK_ASSERT(chipInfo->gpuCoreCount <= __VK_MAX_GPU_CORE_COUNT);
            break;
        default:
            break;
        }
    }

#if __VK_NEW_DEVICE_QUEUE
    iface.command = gcvHAL_GET_BASE_ADDRESS;
    __VK_ONERROR(__vk_DeviceControl(&iface, 0));
    chipInfo->flatMappingStart = iface.u.GetBaseAddress.flatMappingStart;
    chipInfo->flatMappingEnd   = iface.u.GetBaseAddress.flatMappingEnd;
#endif

    return VK_SUCCESS;

OnError:
    return result;
}

static void __vki_InitializeDrvOption(
    __vkInstance *inst
    )
{
    gctSTRING affinity = gcvNULL;

    __vkDrvCtrlOption *option = &inst->drvOption;

    if (inst->chipInfo.gpuCoreCount <= 1)
    {
        option->affinityMode = __VK_MGPU_AFFINITY_NATIVE;
        option->affinityCoreID = 0;
        return;
    }

    option->affinityMode = __VK_MGPU_AFFINITY_COMBINE;
    option->affinityCoreID = 0;

    gcoOS_GetEnv(gcvNULL, "VIV_MGPU_AFFINITY", &affinity);

    if (affinity)
    {
        gctSIZE_T length;
        gcoOS_StrLen(affinity, &length);

        if (length >= 1)
        {
            if (affinity[0] == '0')
            {
                option->affinityMode = __VK_MGPU_AFFINITY_COMBINE;
            }
            else if ((affinity[0] == '1') && (affinity[1] == ':'))
            {
                if ((affinity[2] != '0') || (affinity[2] != '1'))
                {
                    option->affinityMode = __VK_MGPU_AFFINITY_INDEPENDENT;
                    option->affinityCoreID = affinity[2] - '0';
                }
            }
        }
    }

    /* Only inpendent mode could have non-zero affinityCoreID */
    __VK_ASSERT((option->affinityMode == __VK_MGPU_AFFINITY_INDEPENDENT) ||
                (option->affinityCoreID == 0));
    return;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance
    )
{
    uint32_t iExt;
    __vkInstance *inst = VK_NULL_HANDLE;
    VkResult result = VK_SUCCESS;
    const VkApplicationInfo *pAppInfo = pCreateInfo->pApplicationInfo;

    /* Set the allocator to the local allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&__vkAllocator);

#if (defined(DEBUG) || defined(_DEBUG))
    {
        gctSTRING sWaitDebugger;
        gcoOS_GetEnv(gcvNULL, "VIV_WAIT_DEBUGGER", &sWaitDebugger);
        if (sWaitDebugger && (sWaitDebugger[0] == '1'))
        {
            static uint32_t uiWaitDebugger = 1;
            while(uiWaitDebugger)
            {
                uiWaitDebugger = 1;
            }
        }
    }
#endif

    if (pAppInfo && pAppInfo->apiVersion != 0)
    {
        if (VK_VERSION_MAJOR(pAppInfo->apiVersion) != 1 ||
            VK_VERSION_MINOR(pAppInfo->apiVersion) != 0)
        {
            __VK_ONERROR(VK_ERROR_INCOMPATIBLE_DRIVER);
        }
    }

    for (iExt = 0; iExt < pCreateInfo->enabledExtensionCount; iExt++)
    {
        uint32_t i;
        VkBool32 found = VK_FALSE;

        for (i = 0; i < __VK_COUNTOF(g_InstanceExtensions); i++)
        {
            if (gcoOS_StrCmp(pCreateInfo->ppEnabledExtensionNames[iExt],
                g_InstanceExtensions[i].extensionName) == gcvSTATUS_OK)
            {
                found = VK_TRUE;
                break;
            }
        }

        if (!found)
        {
            __VK_ONERROR(VK_ERROR_EXTENSION_NOT_PRESENT);
        }
    }

    /* Allocate memory for a VK instance */
    inst = (__vkInstance*)__VK_ALLOC(sizeof(__vkInstance), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    __VK_ONERROR(inst ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __VK_MEMZERO(inst, sizeof(__vkInstance));
    set_loader_magic_value(inst);

    inst->memCb = __VK_ALLOCATIONCB;

    inst->sType = __VK_OBJECT_TYPE_INSTANCE;

    if (pAppInfo)
    {
        inst->apiVersion = pAppInfo->apiVersion;
        inst->applicationVersion = pAppInfo->applicationVersion;
        inst->engineVersion = pAppInfo->engineVersion;
        if (pAppInfo->pApplicationName)
        {
            gcoOS_StrCopySafe((gctSTRING)inst->applicationName, __VK_MAX_NAME_LENGTH, pAppInfo->pApplicationName);
        }
        if (pAppInfo->pEngineName)
        {
            gcoOS_StrCopySafe((gctSTRING)inst->engineName, __VK_MAX_NAME_LENGTH, pAppInfo->pEngineName);
        }
    }

    __VK_ONERROR(__vki_InitializeChipInfo(inst));

    __vki_InitializeDrvOption(inst);

    /* Lock the __vkRootMutex */
    gcoOS_AcquireMutex(gcvNULL, __vkRootMutex, gcvINFINITE);

    /* Insert inst to the beginning of __vkInstanceRoot linked list */
    inst->pNext = __vkInstanceRoot;
    __vkInstanceRoot = inst;

    /* Release the __vkRootMutex */
    gcoOS_ReleaseMutex(gcvNULL, __vkRootMutex);

    /* Return pointer to inst */
    *pInstance = (VkInstance)inst;

    return VK_SUCCESS;

OnError:
    if (inst)
    {
        __VK_FREE(inst);
    }
    return result;
}

static void __vki_FinalizeGlobals(void)
{
    /* Lock the __vkRootMutex */
    gcoOS_AcquireMutex(gcvNULL, __vkRootMutex, gcvINFINITE);

    /* Final clean up if the last instance is destroyed */
    if (__vkInstanceRoot == gcvNULL)
    {
        /* Reset __vkApiDispatchTable to Nop functions */
        __vkApiDispatchTable = __vkNopDispatchTable;

        /* Release the __vkRootMutex */
        gcoOS_ReleaseMutex(gcvNULL, __vkRootMutex);
        gcoOS_DeleteMutex(gcvNULL, __vkRootMutex);
        __vkRootMutex = gcvNULL;
    }
    else
    {
        /* Release the __vkRootMutex */
        gcoOS_ReleaseMutex(gcvNULL, __vkRootMutex);
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyInstance(
    VkInstance instance,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    __vkInstance *tmpinst, *preinst;
    __vkPhysicalDevice *phyDev;
    int32_t i;

    /* Set the allocator to the local allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    /* Lock the __vkRootMutex */
    gcoOS_AcquireMutex(gcvNULL, __vkRootMutex, gcvINFINITE);

    /* Remove instance from the __vkInstanceRoot linked list */
    preinst = tmpinst = __vkInstanceRoot;
    while (tmpinst != inst && tmpinst->pNext)
    {
        preinst = tmpinst;
        tmpinst = tmpinst->pNext;
    }
    if (tmpinst == inst)
    {
        if (inst == __vkInstanceRoot)
        {
            __vkInstanceRoot = tmpinst->pNext;
        }
        else
        {
            preinst->pNext = tmpinst->pNext;
        }
    }

    /* Release the __vkRootMutex */
    gcoOS_ReleaseMutex(gcvNULL, __vkRootMutex);

    /* Free the instance if it is valid (in the __vkInstanceRoot linked list) */
    if (tmpinst == inst)
    {
        /* De-initialize inst->physicalDevice[i] */
        for (i = 0; i < __VK_MAX_PDEV_COUNT; i++)
        {
            phyDev = &inst->physicalDevice[i];

            if (phyDev->mutex != gcvNULL)
            {
                gcFinalizeCompiler();
                gcoOS_DeleteMutex(gcvNULL, phyDev->mutex);
            }
            phyDev->mutex = gcvNULL;

            vscDestroyPrivateData(&phyDev->vscCoreSysCtx, phyDev->vscCoreSysCtx.hPrivData);
        }

        __VK_FREE(inst);
    }

    __vki_FinalizeGlobals();
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateInstanceLayerProperties(
    uint32_t* pCount,
    VkLayerProperties* pProperties
    )
{
    if (pProperties)
    {
        if (*pCount > 0)
            __VK_ASSERT(0);
    }
    else
    {
        *pCount = 0;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateInstanceExtensionProperties
    (const char* pLayerName,
    uint32_t* pCount,
    VkExtensionProperties* pProperties
    )
{
    if (!pProperties)
    {
        *pCount = __VK_COUNTOF(g_InstanceExtensions);
    }
    else
    {
        uint32_t i;

        for (i = 0; i < __VK_COUNTOF(g_InstanceExtensions); i++)
        {
            __VK_MEMCOPY(&pProperties[i], &g_InstanceExtensions[i], sizeof(VkExtensionProperties));
        }
    }

    return VK_SUCCESS;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_GetInstanceProcAddr(
    VkInstance instance,
    const char* pName
    )
{
    return __vk_GetApiProcAddr(pName);
}

VkResult __vk_DeviceControl(
    gcsHAL_INTERFACE * iface,
    uint32_t coreIdex
    )
{
    gceSTATUS status;

    iface->hardwareType = gcvHARDWARE_3D;
    iface->coreIndex = coreIdex;
    iface->ignoreTLS = gcvTRUE;

    /* Call kernel service. */
    status = gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        iface, gcmSIZEOF(gcsHAL_INTERFACE),
        iface, gcmSIZEOF(gcsHAL_INTERFACE)
        );

    if (gcmIS_SUCCESS(status))
    {
        status = iface->status;
    }

    if (status == gcvSTATUS_OUT_OF_MEMORY)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    else if (gcmIS_ERROR(status))
    {
        return VK_ERROR_DEVICE_LOST;
    }
    else
    {
        return VK_SUCCESS;
    }
}



