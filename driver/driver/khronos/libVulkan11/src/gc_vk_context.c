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


#include "gc_vk_precomp.h"

#if (defined(DBG) && DBG) || defined(DEBUG) || defined(_DEBUG)
int32_t __vkDebugLevel = __VK_DBG_LEVEL_ERROR;
#endif

extern const VkExtensionProperties g_DeviceExtensions[];
extern const uint32_t g_DeviceExtensionsCount;
extern __vkExtension g_EnabledExtensions[];

/* NOTICE: This string array order MUST match item order in dispatch table. */
#define __vkProcInfo_(func) #func,
const char *__vkProcInfoTable[] =
{
    __VK_API_ENTRIES(__vkProcInfo_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkProcInfo_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkProcInfo_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkProcInfo_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkProcInfo_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkProcInfo_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkProcInfo_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkProcInfo_)
#endif
};

PFN_vkVoidFunction __vk_GetApiProcAddr(
    const char *pName
    )
{
    const char *apiName;
    size_t i;
    PFN_vkVoidFunction *table = (PFN_vkVoidFunction *)&__vkApiDispatchTable;

    /* Skip invalid names first */
    if (!pName || pName[0] != 'v' || pName[1] != 'k' || pName[2] == '\0')
    {
        return VK_NULL_HANDLE;
    }

    if (strcmp(pName, "vkBindBufferMemory2KHR") == 0) pName = "vkBindBufferMemory2";
    if (strcmp(pName, "vkBindImageMemory2KHR") == 0) pName = "vkBindImageMemory2";
    if (strcmp(pName, "vkCreateDescriptorUpdateTemplateKHR") == 0) pName = "vkCreateDescriptorUpdateTemplate";
    if (strcmp(pName, "vkGetBufferMemoryRequirements2KHR") == 0) pName = "vkGetBufferMemoryRequirements2";
    if (strcmp(pName, "vkTrimCommandPoolKHR") == 0) pName = "vkTrimCommandPool";
    if (strcmp(pName, "vkCmdSetDeviceMaskKHR") == 0) pName = "vkCmdSetDeviceMask";
    if (strcmp(pName, "vkCmdDispatchBaseKHR") == 0) pName = "vkCmdDispatchBase";
    if (strcmp(pName, "vkDestroyDescriptorUpdateTemplateKHR") == 0) pName = "vkDestroyDescriptorUpdateTemplate";
    if (strcmp(pName, "vkUpdateDescriptorSetWithTemplateKHR") == 0) pName = "vkUpdateDescriptorSetWithTemplate";
    if (strcmp(pName, "vkGetImageMemoryRequirements2KHR") == 0) pName = "vkGetImageMemoryRequirements2";
    if (strcmp(pName, "vkGetDeviceGroupPeerMemoryFeaturesKHR") == 0) pName = "vkGetDeviceGroupPeerMemoryFeatures";
    if (strcmp(pName, "vkCreateSamplerYcbcrConversionKHR") == 0) pName = "vkCreateSamplerYcbcrConversion";
    if (strcmp(pName, "vkDestroySamplerYcbcrConversionKHR") == 0) pName = "vkDestroySamplerYcbcrConversion";
    if (strcmp(pName, "vkGetDescriptorSetLayoutSupportKHR") == 0) pName = "vkGetDescriptorSetLayoutSupport";
    if (strcmp(pName, "vkGetImageSparseMemoryRequirements2KHR") == 0) pName = "vkGetImageSparseMemoryRequirements2";

    /* Skip the first two characters "vk" of procName */
    apiName = pName + 2;

    /* Find API function's offset in __vkProcInfoTable[] table */
    for (i = 0; i < __VK_TABLE_SIZE(__vkProcInfoTable); ++i)
    {
        if (strcmp(__vkProcInfoTable[i], apiName) == 0)
        {
            return table[i];
        }
    }

    return VK_NULL_HANDLE;
}

static void __vki_DetachDevice(
    __vkDevContext *devCtx
    )
{
    uint32_t i;

    for (i = 0; i < devCtx->chipInfo->gpuCoreCount; i++)
    {
        if (devCtx->context[i] != 0)
        {
            gcsHAL_INTERFACE iface;
            iface.command = gcvHAL_DETACH;
            iface.u.Detach.context = devCtx->context[i];
            __VK_VERIFY_OK(__vk_DeviceControl(&iface, i));
            devCtx->context[i] = 0;
        }
    }
    return;

}

static VkResult __vki_AttachDevice(
    __vkDevContext *devCtx
    )
{
    VkResult result = VK_SUCCESS;
    gcsHAL_INTERFACE iface;
    uint32_t coreIdx;

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        for (coreIdx = 0; coreIdx < devCtx->chipInfo->gpuCoreCount; coreIdx++)
        {
            iface.command = gcvHAL_ATTACH;
#if gcdDUMP
            iface.u.Attach.map = gcvTRUE;
#else
            iface.u.Attach.map = gcvFALSE;
#endif
            __VK_ONERROR(__vk_DeviceControl(&iface, coreIdx));
            devCtx->context[coreIdx] = iface.u.Attach.context;
            __VK_ASSERT(devCtx->context[coreIdx] != 0);
#if gcdDUMP
            if (coreIdx == 0)
            {
                uint32_t j;
                devCtx->currentContext = 0;
                devCtx->contextBytes = iface.u.Attach.bytes;

                for (j = 0; j < gcdCONTEXT_BUFFER_COUNT; j++)
                {
                    devCtx->contextLogical[j] = gcmUINT64_TO_PTR(iface.u.Attach.logicals[j]);
                }
            }
#endif
        }
    }
    else
    {
        iface.command = gcvHAL_ATTACH;
#if gcdDUMP
        iface.u.Attach.map = gcvTRUE;
#else
        iface.u.Attach.map = gcvFALSE;
#endif
        coreIdx = devCtx->option->affinityCoreID;
        __VK_ONERROR(__vk_DeviceControl(&iface, coreIdx));
        devCtx->context[coreIdx] = iface.u.Attach.context;
        __VK_ASSERT(devCtx->context[coreIdx] != 0);

#if gcdDUMP
        {
            uint32_t i;
            devCtx->currentContext = 0;
            devCtx->contextBytes = iface.u.Attach.bytes;
            for (i = 0; i < gcdCONTEXT_BUFFER_COUNT; i++)
            {
                devCtx->contextLogical[i] = gcmUINT64_TO_PTR(iface.u.Attach.logicals[i]);
            }
        }
#endif
    }

    return VK_SUCCESS;

OnError:
    __vki_DetachDevice(devCtx);
    return result;

}

void __vk_AllocateVidMemoryCB(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctSIZE_T size,
    gctUINT32 align,
    gctPOINTER *opaqueNode,
    gctPOINTER *memory,
    gctUINT32 *physical,
    gctPOINTER initialData,
    gctBOOL zeroMemory
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)context;
    VkResult result = VK_SUCCESS;
    void *hostPtr = VK_NULL_HANDLE;
    VkDeviceMemory vkDevMem = VK_NULL_HANDLE;
    __vkDeviceMemory *devMem;
    VkDeviceSize vkDevMemSize = __VK_ALIGN(size, align);
    VkMemoryAllocateInfo allocInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        gcvNULL,
        vkDevMemSize,
        0
    };
    __VK_ASSERT(opaqueNode);
    __VK_ASSERT(physical);
    __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &allocInfo, VK_NULL_HANDLE, &vkDevMem));
    __VK_ONERROR(__vk_MapMemory((VkDevice)devCtx, vkDevMem, 0, vkDevMemSize, 0, &hostPtr));

    gcmDUMP(gcvNULL, "#[info: video memory allocate for VSC %s]", tag);

    if (initialData)
    {
        __VK_MEMCOPY(hostPtr, initialData, size);
    }
    else if (zeroMemory)
    {
        __VK_MEMZERO(hostPtr, size);
    }

    devMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *,vkDevMem);
    gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, devMem->devAddr, hostPtr, 0, size);

    *physical = devMem->devAddr;
    *opaqueNode = (gctPOINTER)devMem;

    if (memory)
    {
        *memory = hostPtr;
    }
OnError:
    if ((VK_SUCCESS != result) && vkDevMem)
    {
        __vk_FreeMemory((VkDevice)devCtx, vkDevMem, VK_NULL_HANDLE);
    }
    return;
}

void __vk_FreeVidMemoryCB(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctPOINTER opaqueNode
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)context;
    VkDeviceMemory vkDevMem = (VkDeviceMemory)(uintptr_t)opaqueNode;
    __vk_UnmapMemory((VkDevice)devCtx, vkDevMem);
    __vk_FreeMemory((VkDevice)devCtx, vkDevMem, VK_NULL_HANDLE);
    return;
}


VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkDevContext *devCtx = gcvNULL;
    VkMemoryAllocateInfo mem_alloc;
    VkBufferCreateInfo buf_info;
    VkDeviceMemory fenceMemory;
    VkResult result = VK_SUCCESS;
    uint32_t i, iExt;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&phyDev->pInst->memCb);

    for (i = 0; i < g_DeviceExtensionsCount; i++)
    {
        g_EnabledExtensions[i].bEnabled = VK_FALSE;
    }

    for (iExt = 0; iExt < pCreateInfo->enabledExtensionCount; iExt++)
    {
        VkBool32 found = VK_FALSE;

        for (i = 0; i < g_DeviceExtensionsCount; i++)
        {
            if (gcoOS_StrCmp(pCreateInfo->ppEnabledExtensionNames[iExt], g_DeviceExtensions[i].extensionName) == gcvSTATUS_OK)
            {
                g_EnabledExtensions[i].bEnabled = VK_TRUE;
                found = VK_TRUE;
                break;
            }
        }

        if (!found)
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    /* Allocate memory for __vkDevContext.
    */
    devCtx = (__vkDevContext*)__VK_ALLOC(sizeof(__vkDevContext), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!devCtx)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    __VK_MEMZERO(devCtx, sizeof(__vkDevContext));

    if (pCreateInfo->pEnabledFeatures)
    {
        for (i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++)
        {
            if (*((VkBool32*)(pCreateInfo->pEnabledFeatures) + i))
            {
                if (*((VkBool32*)&phyDev->phyDevFeatures + i))
                {
                    continue;
                }
                else
                {
                    return VK_ERROR_FEATURE_NOT_PRESENT;
                }
            }
        }
        devCtx->enabledFeatures = *pCreateInfo->pEnabledFeatures;
    }
    else
    {
        __VK_MEMZERO(&devCtx->enabledFeatures, sizeof(devCtx->enabledFeatures));
    }

    set_loader_magic_value(devCtx);

    devCtx->sType = __VK_OBJECT_TYPE_DEV_CONTEXT;
    devCtx->pPhyDevice = phyDev;
    devCtx->memCb = __VK_ALLOCATIONCB;
    devCtx->threadId = (uint32_t)gcmPTR2SIZE(gcoOS_GetCurrentThreadID());
    devCtx->option = &phyDev->pInst->drvOption;
    devCtx->chipInfo = &phyDev->pInst->chipInfo;
    devCtx->database = phyDev->phyDevConfig.database;

    if (devCtx->database->ROBUSTNESS && gcvPATCH_SASCHAWILLEMS == devCtx->pPhyDevice->pInst->patchID)
    {
        devCtx->enabledFeatures.robustBufferAccess = VK_TRUE;
    }

#if __VK_RESOURCE_INFO
    __VK_ONERROR(gcoOS_AtomConstruct(gcvNULL, &devCtx->atom_id));
#endif

    __VK_ONERROR(__vki_AttachDevice(devCtx));

    /* Initilize VSC context */
    devCtx->vscSysCtx.pCoreSysCtx = &phyDev->vscCoreSysCtx;
    devCtx->vscSysCtx.drvCBs.pfnAllocVidMemCb = (PFN_ALLOC_VIDMEM_CB)__vk_AllocateVidMemoryCB;
    devCtx->vscSysCtx.drvCBs.pfnFreeVidMemCb = (PFN_FREE_VIDMEM_CB)__vk_FreeVidMemoryCB;
    devCtx->vscSysCtx.hDrv = devCtx;

    __VK_ONERROR(__vkInitilizeChipModule(phyDev, devCtx));

    __VK_ONERROR(__vk_CreateDeviceQueues(devCtx,
                                          pCreateInfo->queueCreateInfoCount,
                                          pCreateInfo->pQueueCreateInfos));
    /***********************************************************************
    ** Construct the default gcHAL objects
    */

    /* Create a VkBuffer for fence memory */
    __VK_MEMZERO(&buf_info, sizeof(VkBufferCreateInfo));
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.size = __VK_MAX_FENCE_COUNT * sizeof(uint64_t);

    devCtx->fenceBuffer = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateBuffer((VkDevice)devCtx, &buf_info, gcvNULL, &devCtx->fenceBuffer));

    /* Allocate device memory for fences */
    __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.allocationSize = __VK_MAX_FENCE_COUNT * sizeof(__vkHwFenceData);
    mem_alloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory((VkDevice)devCtx, &mem_alloc, gcvNULL, &fenceMemory));

    /* bind memory */
    __VK_ONERROR(__vk_BindBufferMemory((VkDevice)devCtx, devCtx->fenceBuffer, fenceMemory, 0));

    devCtx->lastFenceIndex = (uint32_t)-1;
    devCtx->fenceCount = 0;

    /* Lock the phyDev->mutex */
    __VK_ONERROR(gcoOS_AcquireMutex(gcvNULL, phyDev->mutex, gcvINFINITE));

    /* Insert the devCtx to phyDevice->devContextList */
    devCtx->pNext = phyDev->devContextList;
    phyDev->devContextList = devCtx;

    /* Release the phyDev->mutex */
    __VK_ONERROR(gcoOS_ReleaseMutex(gcvNULL, phyDev->mutex));

    /* Return the device context */
    *pDevice = (VkDevice)devCtx;

    return result;

OnError:
    if (devCtx)
    {
        if (devCtx->chipPriv)
        {
            (*devCtx->chipFuncs->FinializeChipModule)((VkDevice)devCtx);
        }

        if (devCtx->fenceBuffer)
        {
            __vkBuffer *buf;

            buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devCtx->fenceBuffer);

            if (buf->memory)
            {
                __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);
            }

            __vk_DestroyBuffer((VkDevice)devCtx, devCtx->fenceBuffer, gcvNULL);
        }

        __vk_DestroyDeviceQueues(devCtx);

        __vki_DetachDevice(devCtx);

        __VK_FREE(devCtx);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyDevice(
    VkDevice device,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (devCtx)
    {
        __vkPhysicalDevice *phyDev = devCtx->pPhyDevice;
        __vkDevContext *tmpctx, *prectx;

        /* Set the allocator to the parent allocator or API defined allocator if valid */
        __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

        /* Lock the phyDev->mutex */
        gcoOS_AcquireMutex(gcvNULL, phyDev->mutex, gcvINFINITE);

        /* Remove devCtx from the phyDev->devContextList linked list */
        prectx = tmpctx = phyDev->devContextList;
        while (tmpctx != devCtx && tmpctx->pNext)
        {
            prectx = tmpctx;
            tmpctx = tmpctx->pNext;
        }
        if (tmpctx == devCtx)
        {
            if (tmpctx == phyDev->devContextList)
            {
                phyDev->devContextList = tmpctx->pNext;
            }
            else
            {
                prectx->pNext = tmpctx->pNext;
            }
        }

        /* Release the phyDev->mutex */
        gcoOS_ReleaseMutex(gcvNULL, phyDev->mutex);

#if __VK_RESOURCE_SAVE_TGA
        if (devCtx->auxCmdPool)
        {
            if (devCtx->auxCmdBuf)
            {
                __vk_FreeCommandBuffers(device, devCtx->auxCmdPool, 1, &devCtx->auxCmdBuf);
            }

            __vk_DestroyCommandPool(device, devCtx->auxCmdPool, gcvNULL);
        }
#endif

        (*devCtx->chipFuncs->FinializeChipModule)(device);

#if __VK_RESOURCE_INFO
        gcoOS_AtomDestroy(gcvNULL, devCtx->atom_id);
#endif

        /* Destroy the devCtx if it is valid (in the phyDev->devContextList) */
        if (tmpctx == devCtx)
        {
            if (devCtx->fenceBuffer)
            {
                __vkBuffer *buf;

                buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devCtx->fenceBuffer);

                if (buf->memory)
                    __vk_FreeMemory((VkDevice)(uintptr_t)devCtx, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);

                __vk_DestroyBuffer((VkDevice)(uintptr_t)devCtx, devCtx->fenceBuffer, gcvNULL);
            }

            __vk_DestroyDeviceQueues(devCtx);

            __vki_DetachDevice(devCtx);

            __VK_FREE(devCtx);
        }
    }
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_GetDeviceProcAddr(
    VkDevice device,
    const char* pName
    )
{
    uint32_t i;
    /* Skip invalid names first */
    if (!pName || pName[0] != 'v' || pName[1] != 'k' || pName[2] == '\0')
    {
        return VK_NULL_HANDLE;
    }

    if (strcmp(pName, "vkDestroyInstance") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkEnumeratePhysicalDevices") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkEnumeratePhysicalDeviceGroups") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkEnumeratePhysicalDeviceGroupsKHR") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceFeatures") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceFormatProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceImageFormatProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceQueueFamilyProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceMemoryProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkCreateDevice") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkEnumerateDeviceLayerProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceSparseImageFormatProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceFeatures2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceFormatProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceImageFormatProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceQueueFamilyProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceMemoryProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceSparseImageFormatProperties2") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceExternalBufferProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceExternalFenceProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkGetPhysicalDeviceExternalSemaphoreProperties") == 0) return VK_NULL_HANDLE;
    if (strcmp(pName, "vkCreateSamplerYcbcrConversionKHR") == 0) return VK_NULL_HANDLE;

    for (i = 0; i < g_DeviceExtensionsCount; i++)
    {
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
             if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkCreateSwapchainKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkDestroySwapchainKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetSwapchainImagesKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkAcquireNextImageKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkQueuePresentKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetDeviceGroupPresentCapabilitiesKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetDeviceGroupSurfacePresentModesKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetPhysicalDevicePresentRectanglesKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkAcquireNextImage2KHR") == 0)return VK_NULL_HANDLE;
             }
             continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkTrimCommandPoolKHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkGetImageSparseMemoryRequirements2KHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetBufferMemoryRequirements2KHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkGetImageMemoryRequirements2KHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkBindBufferMemory2KHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkBindImageMemory2KHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkCreateDescriptorUpdateTemplateKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkDestroyDescriptorUpdateTemplateKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkUpdateDescriptorSetWithTemplateKHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_DEVICE_GROUP_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkGetDeviceGroupPeerMemoryFeaturesKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkCmdSetDeviceMaskKHR") == 0)return VK_NULL_HANDLE;
                 if (strcmp(pName, "vkCmdDispatchBaseKHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
        if (strcmp(g_DeviceExtensions[i].extensionName, VK_KHR_MAINTENANCE3_EXTENSION_NAME) == 0)
        {
            if (!g_EnabledExtensions[i].bEnabled)
             {
                 if (strcmp(pName, "vkGetDescriptorSetLayoutSupportKHR") == 0)return VK_NULL_HANDLE;
             }
            continue;
        }
    }

    return __vk_GetApiProcAddr(pName);
}

VKAPI_ATTR void VKAPI_CALL __vk_GetDeviceQueue(
    VkDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    VkQueue* pQueue
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;

    __VK_ASSERT(queueFamilyIndex <= devCtx->pPhyDevice->queueFamilyCount);
    __VK_ASSERT(queueIndex <= devCtx->queueCount[queueFamilyIndex]);
    *pQueue = (VkQueue)&devCtx->devQueues[queueFamilyIndex][queueIndex];
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_DeviceWaitIdle(VkDevice device)
{
     __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    for (i = 0; i < devCtx->devQueueCreateCount; i++)
    {
        uint32_t j;
        for (j = 0; j < devCtx->queueCount[i]; j++)
        {
            __vkDevQueue *devQueue = &devCtx->devQueues[i][j];
            __VK_ONERROR(__vk_QueueWaitIdle((VkQueue)devQueue));
        }
    }

OnError:

    return result;
}


