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


#ifndef __gc_vk_dispatch_h__
#define __gc_vk_dispatch_h__

#if defined(_WIN32)
#undef CreateSemaphore
#undef CreateEvent
#endif

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 24)
#  include <vulkan/vk_android_native_buffer.h>
#endif

#define __VK_API_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateInstance) \
    vkApiMacro(DestroyInstance) \
    vkApiMacro(EnumeratePhysicalDevices) \
    vkApiMacro(GetPhysicalDeviceFeatures) \
    vkApiMacro(GetPhysicalDeviceFormatProperties) \
    vkApiMacro(GetPhysicalDeviceImageFormatProperties) \
    vkApiMacro(GetPhysicalDeviceProperties) \
    vkApiMacro(GetPhysicalDeviceQueueFamilyProperties) \
    vkApiMacro(GetPhysicalDeviceMemoryProperties) \
    vkApiMacro(GetInstanceProcAddr) \
    vkApiMacro(GetDeviceProcAddr) \
    vkApiMacro(CreateDevice) \
    vkApiMacro(DestroyDevice) \
    vkApiMacro(EnumerateInstanceExtensionProperties) \
    vkApiMacro(EnumerateDeviceExtensionProperties) \
    vkApiMacro(EnumerateInstanceLayerProperties) \
    vkApiMacro(EnumerateDeviceLayerProperties) \
    vkApiMacro(GetDeviceQueue) \
    vkApiMacro(QueueSubmit) \
    vkApiMacro(QueueWaitIdle) \
    vkApiMacro(DeviceWaitIdle) \
    vkApiMacro(AllocateMemory) \
    vkApiMacro(FreeMemory) \
    vkApiMacro(MapMemory) \
    vkApiMacro(UnmapMemory) \
    vkApiMacro(FlushMappedMemoryRanges) \
    vkApiMacro(InvalidateMappedMemoryRanges) \
    vkApiMacro(GetDeviceMemoryCommitment) \
    vkApiMacro(BindBufferMemory) \
    vkApiMacro(BindImageMemory) \
    vkApiMacro(GetBufferMemoryRequirements) \
    vkApiMacro(GetImageMemoryRequirements) \
    vkApiMacro(GetImageSparseMemoryRequirements) \
    vkApiMacro(GetPhysicalDeviceSparseImageFormatProperties) \
    vkApiMacro(QueueBindSparse) \
    vkApiMacro(CreateFence) \
    vkApiMacro(DestroyFence) \
    vkApiMacro(ResetFences) \
    vkApiMacro(GetFenceStatus) \
    vkApiMacro(WaitForFences) \
    vkApiMacro(CreateSemaphore) \
    vkApiMacro(DestroySemaphore) \
    vkApiMacro(CreateEvent) \
    vkApiMacro(DestroyEvent) \
    vkApiMacro(GetEventStatus) \
    vkApiMacro(SetEvent) \
    vkApiMacro(ResetEvent) \
    vkApiMacro(CreateQueryPool) \
    vkApiMacro(DestroyQueryPool) \
    vkApiMacro(GetQueryPoolResults) \
    vkApiMacro(CreateBuffer) \
    vkApiMacro(DestroyBuffer) \
    vkApiMacro(CreateBufferView) \
    vkApiMacro(DestroyBufferView) \
    vkApiMacro(CreateImage) \
    vkApiMacro(DestroyImage) \
    vkApiMacro(GetImageSubresourceLayout) \
    vkApiMacro(CreateImageView) \
    vkApiMacro(DestroyImageView) \
    vkApiMacro(CreateShaderModule) \
    vkApiMacro(DestroyShaderModule) \
    vkApiMacro(CreatePipelineCache) \
    vkApiMacro(DestroyPipelineCache) \
    vkApiMacro(GetPipelineCacheData) \
    vkApiMacro(MergePipelineCaches) \
    vkApiMacro(CreateGraphicsPipelines) \
    vkApiMacro(CreateComputePipelines) \
    vkApiMacro(DestroyPipeline) \
    vkApiMacro(CreatePipelineLayout) \
    vkApiMacro(DestroyPipelineLayout) \
    vkApiMacro(CreateSampler) \
    vkApiMacro(DestroySampler) \
    vkApiMacro(CreateDescriptorSetLayout) \
    vkApiMacro(DestroyDescriptorSetLayout) \
    vkApiMacro(CreateDescriptorPool) \
    vkApiMacro(DestroyDescriptorPool) \
    vkApiMacro(ResetDescriptorPool) \
    vkApiMacro(AllocateDescriptorSets) \
    vkApiMacro(FreeDescriptorSets) \
    vkApiMacro(UpdateDescriptorSets) \
    vkApiMacro(CreateFramebuffer) \
    vkApiMacro(DestroyFramebuffer) \
    vkApiMacro(CreateRenderPass) \
    vkApiMacro(DestroyRenderPass) \
    vkApiMacro(GetRenderAreaGranularity) \
    vkApiMacro(CreateCommandPool) \
    vkApiMacro(DestroyCommandPool) \
    vkApiMacro(ResetCommandPool) \
    vkApiMacro(AllocateCommandBuffers) \
    vkApiMacro(FreeCommandBuffers) \
    vkApiMacro(BeginCommandBuffer) \
    vkApiMacro(EndCommandBuffer) \
    vkApiMacro(ResetCommandBuffer) \
    vkApiMacro(CmdBindPipeline) \
    vkApiMacro(CmdSetViewport) \
    vkApiMacro(CmdSetScissor) \
    vkApiMacro(CmdSetLineWidth) \
    vkApiMacro(CmdSetDepthBias) \
    vkApiMacro(CmdSetBlendConstants) \
    vkApiMacro(CmdSetDepthBounds) \
    vkApiMacro(CmdSetStencilCompareMask) \
    vkApiMacro(CmdSetStencilWriteMask) \
    vkApiMacro(CmdSetStencilReference) \
    vkApiMacro(CmdBindDescriptorSets) \
    vkApiMacro(CmdBindIndexBuffer) \
    vkApiMacro(CmdBindVertexBuffers) \
    vkApiMacro(CmdDraw) \
    vkApiMacro(CmdDrawIndexed) \
    vkApiMacro(CmdDrawIndirect) \
    vkApiMacro(CmdDrawIndexedIndirect) \
    vkApiMacro(CmdDispatch) \
    vkApiMacro(CmdDispatchIndirect) \
    vkApiMacro(CmdCopyBuffer) \
    vkApiMacro(CmdCopyImage) \
    vkApiMacro(CmdBlitImage) \
    vkApiMacro(CmdCopyBufferToImage) \
    vkApiMacro(CmdCopyImageToBuffer) \
    vkApiMacro(CmdUpdateBuffer) \
    vkApiMacro(CmdFillBuffer) \
    vkApiMacro(CmdClearColorImage) \
    vkApiMacro(CmdClearDepthStencilImage) \
    vkApiMacro(CmdClearAttachments) \
    vkApiMacro(CmdResolveImage) \
    vkApiMacro(CmdSetEvent) \
    vkApiMacro(CmdResetEvent) \
    vkApiMacro(CmdWaitEvents) \
    vkApiMacro(CmdPipelineBarrier) \
    vkApiMacro(CmdBeginQuery) \
    vkApiMacro(CmdEndQuery) \
    vkApiMacro(CmdResetQueryPool) \
    vkApiMacro(CmdWriteTimestamp) \
    vkApiMacro(CmdCopyQueryPoolResults) \
    vkApiMacro(CmdPushConstants) \
    vkApiMacro(CmdBeginRenderPass) \
    vkApiMacro(CmdNextSubpass) \
    vkApiMacro(CmdEndRenderPass) \
    vkApiMacro(CmdExecuteCommands) \
    vkApiMacro(EnumerateInstanceVersion) \
    vkApiMacro(BindBufferMemory2) \
    vkApiMacro(BindImageMemory2) \
    vkApiMacro(GetDeviceGroupPeerMemoryFeatures) \
    vkApiMacro(CmdSetDeviceMask) \
    vkApiMacro(CmdDispatchBase) \
    vkApiMacro(EnumeratePhysicalDeviceGroups) \
    vkApiMacro(GetImageMemoryRequirements2) \
    vkApiMacro(GetBufferMemoryRequirements2) \
    vkApiMacro(GetImageSparseMemoryRequirements2) \
    vkApiMacro(GetPhysicalDeviceFeatures2) \
    vkApiMacro(GetPhysicalDeviceProperties2) \
    vkApiMacro(GetPhysicalDeviceFormatProperties2) \
    vkApiMacro(GetPhysicalDeviceImageFormatProperties2) \
    vkApiMacro(GetPhysicalDeviceQueueFamilyProperties2) \
    vkApiMacro(GetPhysicalDeviceMemoryProperties2) \
    vkApiMacro(GetPhysicalDeviceSparseImageFormatProperties2) \
    vkApiMacro(TrimCommandPool) \
    vkApiMacro(GetDeviceQueue2) \
    vkApiMacro(CreateSamplerYcbcrConversion) \
    vkApiMacro(DestroySamplerYcbcrConversion) \
    vkApiMacro(CreateDescriptorUpdateTemplate) \
    vkApiMacro(DestroyDescriptorUpdateTemplate) \
    vkApiMacro(UpdateDescriptorSetWithTemplate) \
    vkApiMacro(GetPhysicalDeviceExternalBufferProperties) \
    vkApiMacro(GetPhysicalDeviceExternalFenceProperties) \
    vkApiMacro(GetPhysicalDeviceExternalSemaphoreProperties) \
    vkApiMacro(GetDescriptorSetLayoutSupport) \
    vkApiMacro(DestroySurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceSupportKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceCapabilitiesKHR) \
    vkApiMacro(GetPhysicalDeviceSurfaceFormatsKHR) \
    vkApiMacro(GetPhysicalDeviceSurfacePresentModesKHR) \
    vkApiMacro(CreateSwapchainKHR) \
    vkApiMacro(DestroySwapchainKHR) \
    vkApiMacro(GetSwapchainImagesKHR) \
    vkApiMacro(AcquireNextImageKHR) \
    vkApiMacro(QueuePresentKHR) \
    vkApiMacro(GetDeviceGroupPresentCapabilitiesKHR) \
    vkApiMacro(GetDeviceGroupSurfacePresentModesKHR) \
    vkApiMacro(GetPhysicalDevicePresentRectanglesKHR) \
    vkApiMacro(AcquireNextImage2KHR) \
    vkApiMacro(GetPhysicalDeviceDisplayPropertiesKHR) \
    vkApiMacro(GetPhysicalDeviceDisplayPlanePropertiesKHR) \
    vkApiMacro(GetDisplayPlaneSupportedDisplaysKHR) \
    vkApiMacro(GetDisplayModePropertiesKHR) \
    vkApiMacro(CreateDisplayModeKHR) \
    vkApiMacro(GetDisplayPlaneCapabilitiesKHR) \
    vkApiMacro(CreateDisplayPlaneSurfaceKHR) \
    vkApiMacro(CreateSharedSwapchainsKHR) \
    vkApiMacro(CreateDebugReportCallbackEXT) \
    vkApiMacro(DestroyDebugReportCallbackEXT) \
    vkApiMacro(DebugReportMessageEXT)

#define __VK_WSI_XLIB_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateXlibSurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceXlibPresentationSupportKHR)

#define __VK_WSI_XCB_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateXcbSurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceXcbPresentationSupportKHR)

#define __VK_WSI_WAYLAND_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateWaylandSurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceWaylandPresentationSupportKHR)

#define __VK_WSI_MIR_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateMirSurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceMirPresentationSupportKHR)

#define __VK_WSI_ANDROID_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateAndroidSurfaceKHR)

/* VK_ANDROID_native_buffer extension. */
#define __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(vkApiMacro) \
    vkApiMacro(GetSwapchainGrallocUsageANDROID) \
    vkApiMacro(AcquireImageANDROID) \
    vkApiMacro(QueueSignalReleaseImageANDROID)

#define __VK_WSI_WIN32_ENTRIES(vkApiMacro) \
    vkApiMacro(CreateWin32SurfaceKHR) \
    vkApiMacro(GetPhysicalDeviceWin32PresentationSupportKHR)

#define __VK_ICD_API_ENTRIES(vkApiMacro) \
    vkApiMacro(GetInstanceProcAddr) \
    vkApiMacro(NegotiateLoaderICDInterfaceVersion)


/* Define Vulkan 1.0 API Dispatch Table */

#define __vkDisp_(entry)        PFN_vk##entry entry;
#define __vkICDDisp_(entry)     PFN_vk##entry icd##entry;

typedef struct __vkDispatchTableRec
{
    __VK_API_ENTRIES(__vkDisp_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkDisp_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkDisp_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkDisp_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkDisp_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkDisp_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkDisp_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkDisp_)
#endif
    __VK_ICD_API_ENTRIES(__vkICDDisp_)
} __vkDispatchTable;


extern __vkDispatchTable __vkDrvEntryFuncTable;
extern __vkDispatchTable __vkValidEntryFuncTable;
extern __vkDispatchTable __vkNopDispatchTable;
extern __vkDispatchTable __vkApiDispatchTable;


/* Vulkan 1.0 API driver entry functions */

extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageUsageFlags flags, VkImageFormatProperties* pImageFormatProperties);


extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
extern VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_GetInstanceProcAddr(VkInstance instance, const char* pName);
extern VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_GetDeviceProcAddr(VkDevice device, const char* pName);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueWaitIdle(VkQueue queue);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_DeviceWaitIdle(VkDevice device);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
extern VKAPI_ATTR void     VKAPI_CALL __vk_FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_MapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
extern VKAPI_ATTR void     VKAPI_CALL __vk_UnmapMemory(VkDevice device, VkDeviceMemory mem);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_FlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_InvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memOffset);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memOffset);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pNumRequirements, VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties);


extern VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetFenceStatus(VkDevice device, VkFence fence);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetEventStatus(VkDevice device, VkEvent event);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_SetEvent(VkDevice device, VkEvent event);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetEvent(VkDevice device, VkEvent event);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_MergePipelineCaches(VkDevice device, VkPipelineCache destCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);


extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet* pDescriptorSets);
extern VKAPI_ATTR void     VKAPI_CALL __vk_UpdateDescriptorSets(VkDevice device, uint32_t writeCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t copyCount, const VkCopyDescriptorSet* pDescriptorCopies);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
extern VKAPI_ATTR void     VKAPI_CALL __vk_FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EndCommandBuffer(VkCommandBuffer commandBuffer);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);


extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t startBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer destBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize dataSize, const void* pData);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize fillSize, uint32_t data);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);


extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);


extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t entry);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize destStride, VkQueryResultFlags flags);


extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t start, uint32_t length, const void* values);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdEndRenderPass(VkCommandBuffer commandBuffer);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCmdBuffers);

/* Vulkan 1.1 API driver entry functions */

extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumerateInstanceVersion(uint32_t* pApiVersion);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
extern VKAPI_ATTR void     VKAPI_CALL __vk_CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR void     VKAPI_CALL __vk_UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
extern VKAPI_ATTR void     VKAPI_CALL __vk_GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport);

/* VK_KHR_surface */
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroySurfaceKHR(VkInstance  instance, VkSurfaceKHR  surface, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
/* VK_KHR_swapchain */
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSwapchainKHR(VkDevice  device, const VkSwapchainCreateInfoKHR*  pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR*  pSwapchain);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroySwapchainKHR(VkDevice  device, VkSwapchainKHR  swapchain, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetSwapchainImagesKHR(VkDevice  device, VkSwapchainKHR  swapchain, uint32_t*  pSwapchainImageCount, VkImage*  pSwapchainImages);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AcquireNextImageKHR(VkDevice  device, VkSwapchainKHR  swapchain, uint64_t  timeout, VkSemaphore  semaphore, VkFence  fence, uint32_t*  pImageIndex);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_QueuePresentKHR(VkQueue  queue, const VkPresentInfoKHR*  pPresentInfo);
/* VK_KHR_device_group */
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
/* VK_KHR_display */
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
/* VK_KHR_display_swapchain */
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains);
/* VK_EXT_debug_report */
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
extern VKAPI_ATTR void     VKAPI_CALL __vk_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);



#ifdef VK_USE_PLATFORM_XLIB_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateXlibSurfaceKHR(VkInstance instance, Display* dpy, Window window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
extern VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateXcbSurfaceKHR(VkInstance instance, xcb_connection_t* connection, xcb_window_t window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
extern VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
extern VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display);
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateMirSurfaceKHR(VkInstance instance, MirConnection* connection, MirSurface* mirSurface, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
extern VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, MirConnection* connection);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
#if (ANDROID_SDK_VERSION >= 24)
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_GetSwapchainGrallocUsageANDROID(VkDevice device, VkFormat format, VkImageUsageFlags imageUsage, int* grallocUsage);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_AcquireImageANDROID(VkDevice device, VkImage image, int nativeFenceFd, VkSemaphore semaphore, VkFence fence);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueSignalReleaseImageANDROID(VkQueue queue, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, VkImage image, int* pNativeFenceFd);
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
extern VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
#endif

extern VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName);
extern VKAPI_ATTR VkResult VKAPI_CALL __vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pVersion);

#endif /* __gc_vk_dispatch_h__ */


