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

/* Mutex to protect __vkInstanceRoot linked list. */
gctPOINTER  __vkRootMutex = gcvNULL;
__vkInstance *__vkInstanceRoot = gcvNULL;

uint32_t  __vkEnableValidation = VK_FALSE;
uint32_t  __vkEnableApiLog = VK_TRUE;
static gcoOS __vkRootHalOS = gcvNULL;


static VkResult __vk_InitializeGlobals(void)
{
    VkResult result = VK_SUCCESS;
    if (__vkRootMutex == gcvNULL)
    {
        gctSTRING tracemode = gcvNULL;
        gcsHAL_INTERFACE iface;
        /* Initialize __vkRootMutex lock */
        if (gcmIS_ERROR(gcoOS_CreateMutex(gcvNULL, &__vkRootMutex)))
        {
            __VK_ONERROR(VK_ERROR_INITIALIZATION_FAILED);
        }
        if (gcmIS_ERROR(gcoOS_Construct(gcvNULL, &__vkRootHalOS)))
        {
            __VK_ONERROR(VK_ERROR_INITIALIZATION_FAILED);
        }
        /* Test if versions match. */
        iface.command = gcvHAL_VERSION;
        __VK_ONERROR(__vk_DeviceControl(&iface, 0));

        if ((iface.u.Version.major != gcvVERSION_MAJOR)
        ||  (iface.u.Version.minor != gcvVERSION_MINOR)
        ||  (iface.u.Version.patch != gcvVERSION_PATCH)
        ||  (iface.u.Version.build != gcvVERSION_BUILD))
        {
            __VK_PRINT("vk user version %d.%d.%d build %u",
                     gcvVERSION_MAJOR, gcvVERSION_MINOR,
                     gcvVERSION_PATCH, gcvVERSION_BUILD);
            __VK_PRINT("vk kernel version %d.%d.%d build %u",
                     iface.u.Version.major, iface.u.Version.minor,
                     iface.u.Version.patch, iface.u.Version.build);

            __VK_ONERROR(VK_ERROR_INCOMPATIBLE_DRIVER);
        }

        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_TRACE", &tracemode)) && tracemode)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "0")))
            {
                __vkEnableApiLog = VK_FALSE;
                __vkEnableValidation = VK_FALSE;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "1")))
            {
                __vkEnableApiLog = VK_TRUE;
                __vkEnableValidation = VK_TRUE;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "2")))
            {
                __vkEnableApiLog = VK_FALSE;
                __vkEnableValidation = VK_TRUE;
            }
            else
            {
                __VK_PRINT("VK: Unsupported trace mode");
            }
        }

        if (__vkEnableValidation)
        {
            __vkApiDispatchTable = __vkValidEntryFuncTable;
        }
        else
        {
            __vkApiDispatchTable = __vkDrvEntryFuncTable;
        }
        gcSetGLSLCompiler(gcCompileShader);
    }

    return VK_SUCCESS;

OnError:
    if (__vkRootMutex)
    {
        gcoOS_DeleteMutex(gcvNULL, __vkRootMutex);
    }
    return result;
}

/*
** Vulkan API 1.0
*/

VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    VkResult result = __vk_InitializeGlobals();
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return __vkApiDispatchTable.CreateInstance(pCreateInfo, pAllocator, pInstance);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyInstance(instance, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
    return __vkApiDispatchTable.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    __vkApiDispatchTable.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
    __vkApiDispatchTable.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice,
 VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage,
 VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
    return __vkApiDispatchTable.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    __vkApiDispatchTable.GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
    __vkApiDispatchTable.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    __vkApiDispatchTable.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    return __vkApiDispatchTable.GetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName)
{
    return __vkApiDispatchTable.GetDeviceProcAddr(device, pName);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    return __vkApiDispatchTable.CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyDevice(device, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    VkResult result = __vk_InitializeGlobals();
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return __vkApiDispatchTable.EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    return __vkApiDispatchTable.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties)
{
    VkResult result = __vk_InitializeGlobals();
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return __vkApiDispatchTable.EnumerateInstanceLayerProperties(pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties)
{
    return __vkApiDispatchTable.EnumerateDeviceLayerProperties(physicalDevice, pCount, pProperties);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    __vkApiDispatchTable.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    return __vkApiDispatchTable.QueueSubmit(queue, submitCount, pSubmits, fence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(VkQueue queue)
{
    return __vkApiDispatchTable.QueueWaitIdle(queue);
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(VkDevice device)
{
    return __vkApiDispatchTable.DeviceWaitIdle(device);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    return __vkApiDispatchTable.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

VKAPI_ATTR void VKAPI_CALL vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.FreeMemory(device, memory, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return __vkApiDispatchTable.MapMemory(device, mem, offset, size, flags, ppData);
}

VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(VkDevice device, VkDeviceMemory mem)
{
    __vkApiDispatchTable.UnmapMemory(device, mem);
}

VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    return __vkApiDispatchTable.FlushMappedMemoryRanges(device, memRangeCount, pMemRanges);
}

VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    return __vkApiDispatchTable.InvalidateMappedMemoryRanges(device, memRangeCount, pMemRanges);
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
    __vkApiDispatchTable.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    return __vkApiDispatchTable.BindBufferMemory(device, buffer, mem, memOffset);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    return __vkApiDispatchTable.BindImageMemory(device, image, mem, memOffset);
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
    __vkApiDispatchTable.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
    __vkApiDispatchTable.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pNumRequirements, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
    __vkApiDispatchTable.GetImageSparseMemoryRequirements(device, image, pNumRequirements, pSparseMemoryRequirements);
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice,
 VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
 VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
    __vkApiDispatchTable.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    return __vkApiDispatchTable.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    return __vkApiDispatchTable.CreateFence(device, pCreateInfo, pAllocator, pFence);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyFence(device, fence, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
    return __vkApiDispatchTable.ResetFences(device, fenceCount, pFences);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(VkDevice device, VkFence fence)
{
    return __vkApiDispatchTable.GetFenceStatus(device, fence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    return __vkApiDispatchTable.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    return __vkApiDispatchTable.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroySemaphore(device, semaphore, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
    return __vkApiDispatchTable.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyEvent(device, event, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(VkDevice device, VkEvent event)
{
    return __vkApiDispatchTable.GetEventStatus(device, event);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(VkDevice device, VkEvent event)
{
    return __vkApiDispatchTable.SetEvent(device, event);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(VkDevice device, VkEvent event)
{
    return __vkApiDispatchTable.ResetEvent(device, event);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
    return __vkApiDispatchTable.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyQueryPool(device, queryPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    return __vkApiDispatchTable.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    return __vkApiDispatchTable.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyBuffer(device, buffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
    return __vkApiDispatchTable.CreateBufferView(device, pCreateInfo, pAllocator, pView);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyBufferView(device, bufferView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
    return __vkApiDispatchTable.CreateImage(device, pCreateInfo, pAllocator, pImage);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyImage(device, image, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
    __vkApiDispatchTable.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
    return __vkApiDispatchTable.CreateImageView(device, pCreateInfo, pAllocator, pView);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyImageView(device, imageView, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    return __vkApiDispatchTable.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyShaderModule(device, shaderModule, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
    return __vkApiDispatchTable.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
    return __vkApiDispatchTable.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(VkDevice device, VkPipelineCache destCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    return __vkApiDispatchTable.MergePipelineCaches(device, destCache, srcCacheCount, pSrcCaches);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    return __vkApiDispatchTable.CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    return __vkApiDispatchTable.CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyPipeline(device, pipeline, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    return __vkApiDispatchTable.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    return __vkApiDispatchTable.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroySampler(device, sampler, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    return __vkApiDispatchTable.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    return __vkApiDispatchTable.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    return __vkApiDispatchTable.ResetDescriptorPool(device, descriptorPool, flags);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
    return __vkApiDispatchTable.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet* pDescriptorSets)
{
    return __vkApiDispatchTable.FreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(VkDevice device, uint32_t writeCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t copyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    __vkApiDispatchTable.UpdateDescriptorSets(device, writeCount, pDescriptorWrites, copyCount, pDescriptorCopies);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    return __vkApiDispatchTable.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyFramebuffer(device, framebuffer, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    return __vkApiDispatchTable.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyRenderPass(device, renderPass, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
    __vkApiDispatchTable.GetRenderAreaGranularity(device, renderPass, pGranularity);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    return __vkApiDispatchTable.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
     __vkApiDispatchTable.DestroyCommandPool(device, commandPool, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    return __vkApiDispatchTable.ResetCommandPool(device, commandPool, flags);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    return __vkApiDispatchTable.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    __vkApiDispatchTable.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    return __vkApiDispatchTable.BeginCommandBuffer(commandBuffer, pBeginInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
    return __vkApiDispatchTable.EndCommandBuffer(commandBuffer);
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    return __vkApiDispatchTable.ResetCommandBuffer(commandBuffer, flags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    __vkApiDispatchTable.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewPort, uint32_t viewportCount, const VkViewport* pViewports)
{
    __vkApiDispatchTable.CmdSetViewport(commandBuffer, firstViewPort, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    __vkApiDispatchTable.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    __vkApiDispatchTable.CmdSetLineWidth(commandBuffer, lineWidth);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    __vkApiDispatchTable.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    __vkApiDispatchTable.CmdSetBlendConstants(commandBuffer, blendConstants);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    __vkApiDispatchTable.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    __vkApiDispatchTable.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    __vkApiDispatchTable.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    __vkApiDispatchTable.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer,
 VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet,
 uint32_t setCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount,
 const uint32_t* pDynamicOffsets)
{
    __vkApiDispatchTable.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    __vkApiDispatchTable.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t startBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    __vkApiDispatchTable.CmdBindVertexBuffers(commandBuffer, startBinding, bindingCount, pBuffers, pOffsets);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    __vkApiDispatchTable.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    __vkApiDispatchTable.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __vkApiDispatchTable.CmdDrawIndirect(commandBuffer, buffer, offset, count, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __vkApiDispatchTable.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    __vkApiDispatchTable.CmdDispatch(commandBuffer, x, y, z);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    __vkApiDispatchTable.CmdDispatchIndirect(commandBuffer, buffer, offset);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    __vkApiDispatchTable.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    __vkApiDispatchTable.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    __vkApiDispatchTable.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __vkApiDispatchTable.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __vkApiDispatchTable.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData)
{
    __vkApiDispatchTable.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize fillSize, uint32_t data)
{
    __vkApiDispatchTable.CmdFillBuffer(commandBuffer, destBuffer, destOffset, fillSize, data);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __vkApiDispatchTable.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __vkApiDispatchTable.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    __vkApiDispatchTable.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    __vkApiDispatchTable.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __vkApiDispatchTable.CmdSetEvent(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __vkApiDispatchTable.CmdResetEvent(commandBuffer, event, stageMask);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(VkCommandBuffer commandBuffer,
 uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask,
 VkPipelineStageFlags destStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
 uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __vkApiDispatchTable.CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, destStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(VkCommandBuffer commandBuffer,
 VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask,
 VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers,
 uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers,
 uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __vkApiDispatchTable.CmdPipelineBarrier(commandBuffer, srcStageMask, destStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags)
{
    __vkApiDispatchTable.CmdBeginQuery(commandBuffer, queryPool, slot, flags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot)
{
    __vkApiDispatchTable.CmdEndQuery(commandBuffer, queryPool, slot);
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    __vkApiDispatchTable.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t entry)
{
    __vkApiDispatchTable.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, entry);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize destStride, VkQueryResultFlags flags)
{
    __vkApiDispatchTable.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, destBuffer, destOffset, destStride, flags);
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t start, uint32_t length, const void* values)
{
    __vkApiDispatchTable.CmdPushConstants(commandBuffer, layout, stageFlags, start, length, values);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    __vkApiDispatchTable.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    __vkApiDispatchTable.CmdNextSubpass(commandBuffer, contents);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    __vkApiDispatchTable.CmdEndRenderPass(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount, const VkCommandBuffer* pCmdBuffers)
{
    __vkApiDispatchTable.CmdExecuteCommands(commandBuffer, commandBuffersCount, pCmdBuffers);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(VkInstance  instance, VkSurfaceKHR  surface, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroySurfaceKHR(instance, surface, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
{
    return __vkApiDispatchTable.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
{
    return __vkApiDispatchTable.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
{
    return __vkApiDispatchTable.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
{
    return __vkApiDispatchTable.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(VkDevice  device, const VkSwapchainCreateInfoKHR*  pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR*  pSwapchain)
{
    return __vkApiDispatchTable.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(VkDevice  device, VkSwapchainKHR  swapchain, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(VkDevice  device, VkSwapchainKHR  swapchain, uint32_t*  pSwapchainImageCount, VkImage*  pSwapchainImages)
{
    return __vkApiDispatchTable.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(VkDevice  device, VkSwapchainKHR  swapchain, uint64_t  timeout, VkSemaphore  semaphore, VkFence  fence, uint32_t*  pImageIndex)
{
    return __vkApiDispatchTable.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue  queue, const VkPresentInfoKHR*  pPresentInfo)
{
    return __vkApiDispatchTable.QueuePresentKHR(queue, pPresentInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties)
{
    return __vkApiDispatchTable.GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties)
{
    return __vkApiDispatchTable.GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays)
{
    return __vkApiDispatchTable.GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties)
{
    return __vkApiDispatchTable.GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode)
{
    return __vkApiDispatchTable.CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities)
{
    return __vkApiDispatchTable.GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains)
{
    return __vkApiDispatchTable.CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    return __vkApiDispatchTable.CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    __vkApiDispatchTable.DestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    __vkApiDispatchTable.DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(VkInstance instance, Display* dpy, Window window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateXlibSurfaceKHR(instance, dpy, window, pAllocator, pSurface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID)
{
    return __vkApiDispatchTable.GetPhysicalDeviceXlibPresentationSupportKHR(physicalDevice, queueFamilyIndex, dpy, visualID);
}
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(VkInstance instance, xcb_connection_t* connection, xcb_window_t window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateXcbSurfaceKHR(instance, connection, window, pAllocator, pSurface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id)
{
    return __vkApiDispatchTable.GetPhysicalDeviceXcbPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection, visual_id);
}
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display)
{
    return __vkApiDispatchTable.GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);
}
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMirSurfaceKHR(VkInstance instance, MirConnection* connection, MirSurface* mirSurface, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateMirSurfaceKHR(instance, connection, mirSurface, pAllocator, pSurface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, MirConnection* connection)
{
    return __vkApiDispatchTable.GetPhysicalDeviceMirPresentationSupportKHR(physicalDevice, queueFamilyIndex, connection);
}
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

#if (ANDROID_SDK_VERSION >= 24)
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(VkDevice device, VkFormat format, VkImageUsageFlags imageUsage, int* grallocUsage)
{
    return __vkApiDispatchTable.GetSwapchainGrallocUsageANDROID(device, format, imageUsage, grallocUsage);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(VkDevice device, VkImage image, int nativeFenceFd, VkSemaphore semaphore, VkFence fence)
{
    return __vkApiDispatchTable.AcquireImageANDROID(device, image, nativeFenceFd, semaphore, fence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(VkQueue queue, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, VkImage image, int* pNativeFenceFd)
{
    return __vkApiDispatchTable.QueueSignalReleaseImageANDROID(queue, waitSemaphoreCount, pWaitSemaphores, image, pNativeFenceFd);
}
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return __vkApiDispatchTable.CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
{
    return __vkApiDispatchTable.GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
}
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    VkResult result = __vk_InitializeGlobals();
    if (result != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return __vkApiDispatchTable.icdGetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pVersion)
{
    VkResult result = __vk_InitializeGlobals();
    if (result != VK_SUCCESS)
    {
        return result;
    }

    return __vkApiDispatchTable.icdNegotiateLoaderICDInterfaceVersion(pVersion);
}



