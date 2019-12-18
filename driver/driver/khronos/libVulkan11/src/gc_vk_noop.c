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

#define __VK_NOP_MESSAGE(...)  __VK_PRINT("No Vulkan Instance Initialized!")

/* Instance APIs should always work */
#define __nop_CreateInstance vkCreateInstance
#define __nop_DestroyInstance vkDestroyInstance
#define __nop_EnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties
#define __nop_EnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties
#define __nop_EnumerateInstanceVersion vkEnumerateInstanceVersion

VKAPI_ATTR VkResult VKAPI_CALL __nop_EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageUsageFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __nop_GetInstanceProcAddr(VkInstance instance, const char* pName)
{
    return __vk_GetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __nop_GetDeviceProcAddr(VkDevice device, const char* pName)
{
    return VK_NULL_HANDLE;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_QueueWaitIdle(VkQueue queue)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_DeviceWaitIdle(VkDevice device)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_MapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_UnmapMemory(VkDevice device, VkDeviceMemory mem)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_FlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_InvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pNumRequirements, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetFenceStatus(VkDevice device, VkFence fence)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetEventStatus(VkDevice device, VkEvent event)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL VKAPI_CALL __nop_SetEvent(VkDevice device, VkEvent event)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ResetEvent(VkDevice device, VkEvent event)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_MergePipelineCaches(VkDevice device, VkPipelineCache destCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet* pDescriptorSets)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_UpdateDescriptorSets(VkDevice device, uint32_t writeCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t copyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewPort, uint32_t viewportCount, const VkViewport* pViewports)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    __VK_NOP_MESSAGE();
}

 VKAPI_ATTR void VKAPI_CALL __nop_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    __VK_NOP_MESSAGE();
}

 VKAPI_ATTR void VKAPI_CALL __nop_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t startBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDraw(VkCommandBuffer commandBuffer, uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer destBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize dataSize, const void* pData)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize fillSize, uint32_t data)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL VKAPI_CALL __nop_CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t entry)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize destStride, VkQueryResultFlags flags)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t start, uint32_t length, const void* values)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount, const VkCommandBuffer* pCmdBuffers)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
}
VKAPI_ATTR void VKAPI_CALL __nop_CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroySurfaceKHR(VkInstance  instance, VkSurfaceKHR  surface, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateSwapchainKHR(VkDevice  device, const VkSwapchainCreateInfoKHR*  pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR*  pSwapchain)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroySwapchainKHR(VkDevice  device, VkSwapchainKHR  swapchain, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetSwapchainImagesKHR(VkDevice  device, VkSwapchainKHR  swapchain, uint32_t*  pSwapchainImageCount, VkImage*  pSwapchainImages)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_QueuePresentKHR(VkQueue  queue, const VkPresentInfoKHR*  pPresentInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL __nop_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    __VK_NOP_MESSAGE();
}

VKAPI_ATTR void VKAPI_CALL __nop_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    __VK_NOP_MESSAGE();
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateXlibSurfaceKHR(VkInstance instance, Display* dpy, Window window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __nop_GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID)
{
    __VK_NOP_MESSAGE();
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateXcbSurfaceKHR(VkInstance instance, xcb_connection_t* connection, xcb_window_t window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __nop_GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id)
{
    __VK_NOP_MESSAGE();
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __nop_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display)
{
    __VK_NOP_MESSAGE();
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateMirSurfaceKHR(VkInstance instance, MirConnection* connection, MirSurface* mirSurface, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __nop_GetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, MirConnection* connection)
{
    __VK_NOP_MESSAGE();
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

#if (ANDROID_SDK_VERSION >= 24)
VKAPI_ATTR VkResult VKAPI_CALL __nop_GetSwapchainGrallocUsageANDROID(VkDevice device, VkFormat format, VkImageUsageFlags imageUsage, int* grallocUsage)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_AcquireImageANDROID(VkDevice device, VkImage image, int nativeFenceFd, VkSemaphore semaphore, VkFence fence)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_QueueSignalReleaseImageANDROID(VkQueue queue, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, VkImage image, int* pNativeFenceFd)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}
#  endif
#if (ANDROID_SDK_VERSION >= 26)
VKAPI_ATTR VkResult VKAPI_CALL __nop_GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}
#  endif
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL __nop_CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __nop_GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
{
    __VK_NOP_MESSAGE();
    return VK_TRUE;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_ImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __nop_icdGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    __VK_NOP_MESSAGE();
    return VK_NULL_HANDLE;
}

VKAPI_ATTR VkResult VKAPI_CALL __nop_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pVersion)
{
    __VK_NOP_MESSAGE();
    return VK_SUCCESS;
}


#define __vkNop_(func)      __nop_##func,
#define __vkICDNop_(func)   __nop_icd##func,

__vkDispatchTable __vkNopDispatchTable = {
    __VK_API_ENTRIES(__vkNop_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkNop_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkNop_)
#  endif
#if (ANDROID_SDK_VERSION >= 26)
    __VK_EXT_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_ENTRIES(__vkNop_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkNop_)
#endif
    __VK_ICD_API_ENTRIES(__vkICDNop_)
};

/* Global Vulkan API dispatch table */
__vkDispatchTable __vkApiDispatchTable = {
    __VK_API_ENTRIES(__vkNop_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkNop_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkNop_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkNop_)
#  endif
#if (ANDROID_SDK_VERSION >= 26)
    __VK_EXT_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_ENTRIES(__vkNop_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkNop_)
#endif
    __VK_ICD_API_ENTRIES(__vkICDNop_)
};



