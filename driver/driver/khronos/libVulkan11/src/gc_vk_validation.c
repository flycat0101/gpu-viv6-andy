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


#include "gc_vk_precomp.h"

#define __VK_PTRVALUE(ptr)  ((ptr) ? *(ptr) : 0)

/* Vivante Implementation Defined Validation Errors */
#define __VK_ERROR_DEVICE_ALREADY_CREATED               ((VkResult)(VK_RESULT_RANGE_SIZE + 0))
#define __VK_ERROR_INVALID_POINTER                      ((VkResult)(VK_RESULT_RANGE_SIZE + 1))
#define __VK_ERROR_INVALID_VALUE                        ((VkResult)(VK_RESULT_RANGE_SIZE + 2))
#define __VK_ERROR_INVALID_HANDLE                       ((VkResult)(VK_RESULT_RANGE_SIZE + 3))
#define __VK_ERROR_INVALID_FORMAT                       ((VkResult)(VK_RESULT_RANGE_SIZE + 4))
#define __VK_ERROR_INVALID_IMAGE                        ((VkResult)(VK_RESULT_RANGE_SIZE + 5))
#define __VK_ERROR_INVALID_DESCRIPTOR_SET_DATA          ((VkResult)(VK_RESULT_RANGE_SIZE + 6))
#define __VK_ERROR_INVALID_QUEUE_TYPE                   ((VkResult)(VK_RESULT_RANGE_SIZE + 7))
#define __VK_ERROR_INVALID_LAYER                        ((VkResult)(VK_RESULT_RANGE_SIZE + 8))
#define __VK_ERROR_BAD_SHADER_CODE                      ((VkResult)(VK_RESULT_RANGE_SIZE + 9))
#define __VK_ERROR_BAD_PIPELINE_DATA                    ((VkResult)(VK_RESULT_RANGE_SIZE + 10))
#define __VK_ERROR_NOT_MAPPABLE                         ((VkResult)(VK_RESULT_RANGE_SIZE + 11))
#define __VK_ERROR_INCOMPLETE_COMMAND_BUFFER            ((VkResult)(VK_RESULT_RANGE_SIZE + 12))
#define __VK_ERROR_BUILDING_COMMAND_BUFFER              ((VkResult)(VK_RESULT_RANGE_SIZE + 13))
#define __VK_ERROR_DEVICE_MISMATCH                      ((VkResult)(VK_RESULT_RANGE_SIZE + 14))
#define __VK_ERROR_NOT_ALIGNED                          ((VkResult)(VK_RESULT_RANGE_SIZE + 15))


extern __vkInstance *__vkInstanceRoot;
extern gctPOINTER  __vkRootMutex;

static const char* __vkiGetResultString(VkResult result)
{
    switch ((int32_t)result)
    {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case __VK_ERROR_DEVICE_ALREADY_CREATED:
            return "__VK_ERROR_DEVICE_ALREADY_CREATED";
        case __VK_ERROR_INVALID_POINTER:
            return "__VK_ERROR_INVALID_POINTER";
        case __VK_ERROR_INVALID_VALUE:
            return "__VK_ERROR_INVALID_VALUE";
        case __VK_ERROR_INVALID_HANDLE:
            return "__VK_ERROR_INVALID_HANDLE";
        case __VK_ERROR_INVALID_FORMAT:
            return "__VK_ERROR_INVALID_FORMAT";
        case __VK_ERROR_INVALID_IMAGE:
            return "__VK_ERROR_INVALID_IMAGE";
        case __VK_ERROR_INVALID_DESCRIPTOR_SET_DATA:
            return "__VK_ERROR_INVALID_DESCRIPTOR_SET_DATA";
        case __VK_ERROR_INVALID_QUEUE_TYPE:
            return "__VK_ERROR_INVALID_QUEUE_TYPE";
        case __VK_ERROR_INVALID_LAYER:
            return "__VK_ERROR_INVALID_LAYER";
        case __VK_ERROR_BAD_SHADER_CODE:
            return "__VK_ERROR_BAD_SHADER_CODE";
        case __VK_ERROR_BAD_PIPELINE_DATA:
            return "__VK_ERROR_BAD_PIPELINE_DATA";
        case __VK_ERROR_NOT_MAPPABLE:
            return "__VK_ERROR_NOT_MAPPABLE";
        case __VK_ERROR_INCOMPLETE_COMMAND_BUFFER:
            return "__VK_ERROR_INCOMPLETE_COMMAND_BUFFER";
        case __VK_ERROR_BUILDING_COMMAND_BUFFER:
            return "__VK_ERROR_BUILDING_COMMAND_BUFFER";
        case __VK_ERROR_DEVICE_MISMATCH:
            return "__VK_ERROR_DEVICE_MISMATCH";
        case __VK_ERROR_NOT_ALIGNED:
            return "__VK_ERROR_NOT_ALIGNED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        default:
            return "__VK_ERROR_UNKNOWN_RESULT";
    }
}

__VK_INLINE void __vkLogShaderStrings(size_t codeSize, unsigned char *pCode)
{
    size_t i;
    uint32_t offs;
    char strbuf[80];

    __VK_LOG_API("#### SPIRV Size: %d ####\n", codeSize);
    for (i = 0, offs = 0; i < codeSize; i++)
    {
        gcoOS_PrintStrSafe(strbuf, 80, &offs, "%02X ", pCode[i]);
        if (offs >= 75 || i == (codeSize - 1))
        {
            strbuf[offs] = '\0';
            __VK_LOG_API("%s\n", strbuf);
            offs = 0;
        }
    }
    __VK_LOG_API("####\n");
}

/*
** Vulkan API Validation Layer that is enabled for App development and can be skipped at runtime.
*/
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    VkResult result = VK_SUCCESS;
    __vkInstance *inst;

    __VK_LOG_API("(tid=%p): vkCreateInstance(%p, %p)", gcoOS_GetCurrentThreadID(), pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!pCreateInfo || !pInstance)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    if (pCreateInfo->sType != VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    /* Lock the __vkRootMutex */
    gcoOS_AcquireMutex(gcvNULL, __vkRootMutex, gcvINFINITE);

    inst = __vkInstanceRoot;
    while (inst)
    {
        __VK_DEBUG_PRINT(0, "%s Possible Leak :: %p,", "VK_INSTANCE", inst);
        inst = inst->pNext;
    }
    /* Release the __vkRootMutex */
    gcoOS_ReleaseMutex(gcvNULL, __vkRootMutex);

    result = __vk_CreateInstance(pCreateInfo, pAllocator, pInstance);

vk_Exit:
    __VK_LOG_API(" ==> %s (instance=%p)\n", __vkiGetResultString(result), __VK_PTRVALUE(pInstance));
    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkDestroyInstance(%p, %p)", gcoOS_GetCurrentThreadID(), instance, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    for (i = 0; i < __VK_MAX_PDEV_COUNT; i++)
    {
        __vkPhysicalDevice *phyDev = &inst->physicalDevice[i];

        if (phyDev->mutex != gcvNULL)
        {
            __vkDevContext *tmpctx;
            gcoOS_AcquireMutex(gcvNULL, phyDev->mutex, gcvINFINITE);
            tmpctx = phyDev->devContextList;
            while (tmpctx)
            {
                __VK_DEBUG_PRINT(0, "%s Leak :: %p,", "VK_CONTEXT", tmpctx);
                tmpctx = tmpctx->pNext;
            }
            gcoOS_ReleaseMutex(gcvNULL, phyDev->mutex);
        }
    }

    __vk_DestroyInstance(instance, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumeratePhysicalDevices(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pPhysicalDeviceCount, pPhysicalDevices);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!pPhysicalDeviceCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    if (pPhysicalDevices)
    {
        if (*pPhysicalDeviceCount > inst->physicalDeviceCount)
        {
            result = __VK_ERROR_INVALID_VALUE;
            goto vk_Exit;
        }
    }

    result = __vk_EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);

vk_Exit:
    __VK_LOG_API(" ==> %s (physicalDeviceCount=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPhysicalDeviceCount));
    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceFeatures(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pFeatures);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFeatures)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceFeatures(physicalDevice, pFeatures);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceFormatProperties(%p, %u, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, format, pFormatProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (format > VK_FORMAT_END_RANGE &&
        !(format >= VK_FORMAT_YCBCR_START &&
        format <= VK_FROMAT_YCBCR_END))
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (!pFormatProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageUsageFlags flags, VkImageFormatProperties* pImageFormatProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceImageFormatProperties(%p, %u, %u, %u, %u, %u, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (format > VK_FORMAT_END_RANGE &&
        !(format >= VK_FORMAT_YCBCR_START &&
        format <= VK_FROMAT_YCBCR_END))
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (type > VK_IMAGE_TYPE_END_RANGE)
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (tiling > VK_IMAGE_TILING_END_RANGE)
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (!pImageFormatProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceProperties(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceProperties(physicalDevice, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceQueueFamilyProperties(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pQueueFamilyPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceQueueFamilyProperties(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pMemoryProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMemoryProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __valid_GetInstanceProcAddr(VkInstance instance, const char* pName)
{
    PFN_vkVoidFunction pFunc = gcvNULL;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetInstanceProcAddr(%p, %s)", gcoOS_GetCurrentThreadID(), instance, pName);

    /* Do not check instance, NULL instance is valid. */
    if (!pName)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    pFunc = __vk_GetInstanceProcAddr(instance, pName);

vk_Exit:
    if (__VK_IS_SUCCESS(result))
    {
        __VK_LOG_API(" ==> %p\n", pFunc);
    }
    else
    {
        __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    }

    return pFunc;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __valid_GetDeviceProcAddr(VkDevice device, const char* pName)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    PFN_vkVoidFunction pFunc = gcvNULL;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDeviceProcAddr(%p, %s)", gcoOS_GetCurrentThreadID(), device, pName);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pName)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    pFunc = __vk_GetDeviceProcAddr(device, pName);

vk_Exit:
    __VK_LOG_API(" ==> %p\n", pFunc);
    devCtx->currentResult = result;

    return pFunc;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDevice(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pDevice)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

    /* Initialize VK object lists for object memory tracking and statistics */
    if (result == VK_SUCCESS)
    {
        __vk_InitObjectLists(*pDevice);
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (device=%p)\n", __vkiGetResultString(result), __VK_PTRVALUE(pDevice));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyDevice(%p)", gcoOS_GetCurrentThreadID(), device, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_FiniObjectLists(devCtx);

    __vk_DestroyDevice(device, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumerateInstanceExtensionProperties(%s, %p, %p)", gcoOS_GetCurrentThreadID(), pLayerName, pCount, pProperties);

    if (!pCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (count=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCount));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pCount, VkExtensionProperties* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumerateDeviceExtensionProperties(%p, %s, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pLayerName, pCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (count=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCount));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties)
{
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumerateInstanceLayerProperties(%p, %p)", gcoOS_GetCurrentThreadID(), pCount, pProperties);

    if (!pCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_EnumerateInstanceLayerProperties(pCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (count=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCount));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount, VkLayerProperties* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumerateDeviceLayerProperties(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_EnumerateDeviceLayerProperties(physicalDevice, pCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (count=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCount));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPhysicalDevice *phyDev = devCtx->pPhyDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDeviceQueue(%p, %u, %u)", gcoOS_GetCurrentThreadID(), device, queueFamilyIndex, queueIndex);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (queueFamilyIndex > 0 || queueIndex >= phyDev->queueFamilyCount)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pQueue)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);

vk_Exit:
    __VK_LOG_API(" ==> %s (queue=%p)\n", __vkiGetResultString(result), __VK_PTRVALUE(pQueue));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence)
{
    __vkDevQueue *pqe = (__vkDevQueue *)queue;
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;
    VkCommandBuffer *cmdlist;
    __vkCommandBuffer *cmd;
    uint32_t i, j, cmdbufcount;

    __VK_LOG_API("(tid=%p): vkQueueSubmit(%p, %u, %p, 0x%llx)", gcoOS_GetCurrentThreadID(), queue, submitCount, pSubmits, fence);

    /* API validation logic that can be skipped at runtime */
    if (!pqe || pqe->sType != __VK_OBJECT_TYPE_CMD_QUEUE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (submitCount && !pSubmits)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < submitCount; i++)
    {
        cmdlist = (VkCommandBuffer *)pSubmits[i].pCommandBuffers;
        cmdbufcount = pSubmits[i].commandBufferCount;
        if (cmdbufcount && !cmdlist)
        {
            result = __VK_ERROR_INVALID_POINTER;
            goto vk_Exit;
        }
        for (j = 0; j < cmdbufcount; j++)
        {
            cmd = (__vkCommandBuffer *)cmdlist[j];
            if (!cmd || cmd->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
            {
                result = __VK_ERROR_INVALID_HANDLE;
                goto vk_Exit;
            }
        }
    }
    if (fce && (fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE)))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    result = __vk_QueueSubmit(queue, submitCount, pSubmits, fence);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    pqe->pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_QueueWaitIdle(VkQueue queue)
{
    __vkDevQueue *pqe = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkQueueWaitIdle(%p)", gcoOS_GetCurrentThreadID(), queue);

    /* API validation logic that can be skipped at runtime */
    if (!pqe || pqe->sType != __VK_OBJECT_TYPE_CMD_QUEUE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_QueueWaitIdle(queue);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    pqe->pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_DeviceWaitIdle(VkDevice device)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDeviceWaitIdle(%p)", gcoOS_GetCurrentThreadID(), device);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_DeviceWaitIdle(device);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkAllocateMemory(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pAllocateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pAllocateInfo || pAllocateInfo->sType != VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pAllocateInfo->memoryTypeIndex >= devCtx->pPhyDevice->phyDevMemProp.memoryTypeCount)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMemory)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_DEVICE_MEMORY, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pMemory));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (memory=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pMemory));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, memory);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkFreeMemory(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, memory, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pMem)
    {
        if (pMem->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
        if (pMem->devCtx != devCtx)
        {
            result = __VK_ERROR_DEVICE_MISMATCH;
            goto vk_Exit;
        }
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_DEVICE_MEMORY, (__vkObject*)pMem);

    __vk_FreeMemory(device, memory, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_MapMemory(VkDevice device, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, mem);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkMapMemory(%p, 0x%llx, %llu, %llu, %u, %p)", gcoOS_GetCurrentThreadID(), device, mem, offset, size, flags, ppData);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMem || pMem->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pMem->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if (pMem->mapped)
    {
        result = VK_ERROR_MEMORY_MAP_FAILED;
        goto vk_Exit;
    }
    if (size != VK_WHOLE_SIZE && offset + size > pMem->allocInfo.allocationSize)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (flags != 0)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!ppData)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_MapMemory(device, mem, offset, size, flags, ppData);

vk_Exit:
    __VK_LOG_API(" ==> %s (memAddr=%p)\n", __vkiGetResultString(result), __VK_PTRVALUE(ppData));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_UnmapMemory(VkDevice device, VkDeviceMemory mem)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, mem);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkUnmapMemory(%p, 0x%llx)", gcoOS_GetCurrentThreadID(), device, mem);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pMem->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if (!pMem || pMem->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMem->mapped)
    {
        result = VK_ERROR_MEMORY_MAP_FAILED;
        goto vk_Exit;
    }

    __vk_UnmapMemory(device, mem);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_FlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkFlushMappedMemoryRanges(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, memRangeCount, pMemRanges);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (memRangeCount == 0)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pMemRanges || pMemRanges->sType != VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < memRangeCount; ++i)
    {
        const VkMappedMemoryRange *pMemRange = &pMemRanges[i];
        __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory*, pMemRange->memory);

        if (pMemRange->sType != VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
        if (pMem->devCtx != devCtx)
        {
            result = __VK_ERROR_DEVICE_MISMATCH;
            goto vk_Exit;
        }
        if (!pMem->mapped)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    result = __vk_FlushMappedMemoryRanges(device, memRangeCount, pMemRanges);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_InvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkInvalidateMappedMemoryRanges(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, memRangeCount, pMemRanges);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (memRangeCount == 0)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pMemRanges)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < memRangeCount; ++i)
    {
        const VkMappedMemoryRange *pMemRange = &pMemRanges[i];
        __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory*, pMemRange->memory);

        if (pMemRange->sType != VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
        if (pMem->devCtx != devCtx)
        {
            result = __VK_ERROR_DEVICE_MISMATCH;
            goto vk_Exit;
        }
        if (!pMem->mapped)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
        if (pMemRange->size != VK_WHOLE_SIZE && pMemRange->offset + pMemRange->size > pMem->allocInfo.allocationSize)
        {
            result = __VK_ERROR_INVALID_VALUE;
            goto vk_Exit;
        }
    }

    result = __vk_InvalidateMappedMemoryRanges(device, memRangeCount, pMemRanges);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, memory);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDeviceMemoryCommitment(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, memory, pCommittedMemoryInBytes);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMem || pMem->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pMem->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if ((devCtx->pPhyDevice->phyDevMemProp.memoryTypes[pMem->allocInfo.memoryTypeIndex].propertyFlags &
         VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
        ) == 0
       )
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCommittedMemoryInBytes)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);

vk_Exit:
    __VK_LOG_API(" ==> %s (committedMemoryInBytes=%llu)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCommittedMemoryInBytes));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    __vkDeviceMemory *dvm = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, mem);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkBindBufferMemory(%p, 0x%llx, 0x%llx, %llu)", gcoOS_GetCurrentThreadID(), device, buffer, mem, memOffset);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!buf || buf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (buf->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if (buf->memory)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dvm || dvm->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (dvm->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if (memOffset % buf->memReq.alignment)
    {
        result = __VK_ERROR_NOT_ALIGNED;
        goto vk_Exit;
    }
    if (buf->createInfo.usage & (VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) &&
        memOffset % devCtx->pPhyDevice->phyDevProp.limits.minTexelBufferOffsetAlignment)
    {
        result = __VK_ERROR_NOT_ALIGNED;
        goto vk_Exit;
    }
    if (buf->createInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT &&
        memOffset % devCtx->pPhyDevice->phyDevProp.limits.minUniformBufferOffsetAlignment)
    {
        result = __VK_ERROR_NOT_ALIGNED;
        goto vk_Exit;
    }
    if (buf->createInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT &&
        memOffset % devCtx->pPhyDevice->phyDevProp.limits.minStorageBufferOffsetAlignment)
    {
        result = __VK_ERROR_NOT_ALIGNED;
        goto vk_Exit;
    }
    if (memOffset > dvm->allocInfo.allocationSize ||
        memOffset + buf->createInfo.size > dvm->allocInfo.allocationSize)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if ((buf->memReq.memoryTypeBits & (1 << dvm->allocInfo.memoryTypeIndex)) == 0)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    result = __vk_BindBufferMemory(device, buffer, mem, memOffset);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memOffset)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *pImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    __vkDeviceMemory *pMem = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory *, mem);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkBindImageMemory(%p, 0x%llx, 0x%llx, %llu)", gcoOS_GetCurrentThreadID(), device, image, mem, memOffset);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pImg || pImg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMem || pMem->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DEVICE_MEMORY))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_BindImageMemory(device, image, mem, memOffset);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetBufferMemoryRequirements(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, buffer, pMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!buf || buf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (buf->devCtx != devCtx)
    {
        result = __VK_ERROR_DEVICE_MISMATCH;
        goto vk_Exit;
    }
    if (!pMemoryRequirements)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetImageMemoryRequirements(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, image, pMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMemoryRequirements)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetImageMemoryRequirements(device, image, pMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pNumRequirements, VkSparseImageMemoryRequirements* pSparseMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetImageSparseMemoryRequirements(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), device, image, pNumRequirements, pSparseMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pNumRequirements || !pSparseMemoryRequirements)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetImageSparseMemoryRequirements(device, image, pNumRequirements, pSparseMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s (numRequirements=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pNumRequirements));
    devCtx->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSparseImageFormatProperties(%p, %u, %u, %u, %u, %u, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (format > VK_FORMAT_END_RANGE &&
        !(format >= VK_FORMAT_YCBCR_START &&
        format <= VK_FROMAT_YCBCR_END))
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (type > VK_IMAGE_TYPE_END_RANGE)
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (tiling > VK_IMAGE_TILING_END_RANGE)
    {
        result = __VK_ERROR_INVALID_FORMAT;
        goto vk_Exit;
    }
    if (!pPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (propertyCount=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPropertyCount));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence)
{
    __vkDevQueue *pqe = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkQueueBindSparse(%p, %u, %p, 0x%llx)", gcoOS_GetCurrentThreadID(), queue, bindInfoCount, pBindInfo, fence);

    /* API validation logic that can be skipped at runtime */
    if (!pqe || pqe->sType != __VK_OBJECT_TYPE_CMD_QUEUE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pBindInfo)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    pqe->pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateFence(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_FENCE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFence)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateFence(device, pCreateInfo, pAllocator, pFence);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_FENCE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pFence));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (fence=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pFence));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyFence(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, fence, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!fce || fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_FENCE, (__vkObject*)fce);

    __vk_DestroyFence(device, fence, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFence *fce;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkResetFences(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, fenceCount, pFences);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFences)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < fenceCount; i++)
    {
        fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pFences[i]);
        if (!fce || fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    result = __vk_ResetFences(device, fenceCount, pFences);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetFenceStatus(VkDevice device, VkFence fence)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetFenceStatus(%p, 0x%llx)", gcoOS_GetCurrentThreadID(), device, fence);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!fce || fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetFenceStatus(device, fence);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFence *fce;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkWaitForFences(%p, %u, %p, %d, %llu)", gcoOS_GetCurrentThreadID(), device, fenceCount, pFences, waitAll, timeout);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFences)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < fenceCount; i++)
    {
        fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pFences[i]);
        if (!fce || fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    result = __vk_WaitForFences(device, fenceCount, pFences, waitAll, timeout);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateSemaphore(%p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSemaphore)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_SEMAPHORE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pSemaphore));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (semaphore=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSemaphore));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSemaphore *smp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySemaphore(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, semaphore, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!smp || smp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SEMAPHORE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_SEMAPHORE, (__vkObject*)smp);

    __vk_DestroySemaphore(device, semaphore, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateEvent(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_EVENT_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pEvent)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateEvent(device, pCreateInfo, pAllocator, pEvent);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_EVENT, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pEvent));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (event=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pEvent));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyEvent(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, event, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_EVENT, (__vkObject*)evt);

    __vk_DestroyEvent(device, event, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetEventStatus(VkDevice device, VkEvent event)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetEventStatus(%p, 0x%llx)", gcoOS_GetCurrentThreadID(), device, event);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetEventStatus(device, event);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_SetEvent(VkDevice device, VkEvent event)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkSetEvent(%p, 0x%llx)", gcoOS_GetCurrentThreadID(), device, event);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_SetEvent(device, event);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ResetEvent(VkDevice device, VkEvent event)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkResetEvent(%p, 0x%llx)", gcoOS_GetCurrentThreadID(), device, event);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ResetEvent(device, event);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL __valid_ImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkImportSemaphoreFdKHR(%p, %p)", gcoOS_GetCurrentThreadID(), device, pImportSemaphoreFdInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ImportSemaphoreFdKHR(device, pImportSemaphoreFdInfo);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR* pGetFdInfo, int* pFd)
{
     __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetSemaphoreFdKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pGetFdInfo, pFd);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetSemaphoreFdKHR(device, pGetFdInfo, pFd);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR* pImportFenceFdInfo)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkImportFenceFdKHR(%p, %p)", gcoOS_GetCurrentThreadID(), device, pImportFenceFdInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ImportFenceFdKHR(device, pImportFenceFdInfo);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR* pGetFdInfo, int* pFd)
{
     __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetFenceFdKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pGetFdInfo, pFd);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetFenceFdKHR(device, pGetFdInfo, pFd);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateQueryPool(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pQueryPool)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_QUERY_POOL, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pQueryPool));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (queryPool=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pQueryPool));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyQueryPool(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, queryPool, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_QUERY_POOL, (__vkObject*)qyp);

    __vk_DestroyQueryPool(device, queryPool, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetQueryPoolResults(%p, 0x%llx, %u, %u, %p, %p, %llu, %u)", gcoOS_GetCurrentThreadID(), device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pData)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateBuffer(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    /* We do NOT support sparse resource */
    if (pCreateInfo->flags != 0 || pCreateInfo->usage == 0)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (pCreateInfo->usage & ~(VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                               VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
                               VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
                              )
       )
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pCreateInfo->sharingMode > VK_SHARING_MODE_END_RANGE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT &&
        (pCreateInfo->queueFamilyIndexCount == 0 || !pCreateInfo->pQueueFamilyIndices)
       )
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pBuffer)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_BUFFER, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pBuffer));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (buffer=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pBuffer));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyBuffer(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, buffer, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (buf)
    {
        if (buf->devCtx != devCtx)
        {
            result = __VK_ERROR_DEVICE_MISMATCH;
            goto vk_Exit;
        }
        if (buf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_BUFFER, (__vkObject*)buf);

    __vk_DestroyBuffer(device, buffer, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *buf = gcvNULL;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateBufferView(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO ||
        pCreateInfo->pNext || pCreateInfo->flags != 0)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, pCreateInfo->buffer);
    if (!buf || buf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!(buf->createInfo.usage & 0x1ff))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pCreateInfo->format > VK_FORMAT_END_RANGE)
    {
        result = VK_ERROR_FORMAT_NOT_SUPPORTED;
        goto vk_Exit;
    }
    if (pCreateInfo->offset % devCtx->pPhyDevice->phyDevProp.limits.minTexelBufferOffsetAlignment)
    {
        result = __VK_ERROR_NOT_ALIGNED;
        goto vk_Exit;
    }
    if (pCreateInfo->range != VK_WHOLE_SIZE)
    {
        if (pCreateInfo->offset + pCreateInfo->range > buf->createInfo.size)
        {
            result = __VK_ERROR_INVALID_VALUE;
            goto vk_Exit;
        }
    }
    if (!pView)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateBufferView(device, pCreateInfo, pAllocator, pView);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_BUFFER_VIEW, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pView));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (view=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pView));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBufferView *bfv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBufferView *, bufferView);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyBufferView(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, bufferView, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfv || bfv->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER_VIEW))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_BUFFER_VIEW, (__vkObject*)bfv);

    __vk_DestroyBufferView(device, bufferView, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateImage(%p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pImage)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateImage(device, pCreateInfo, pAllocator, pImage);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_IMAGE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pImage));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (image=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pImage));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyImage(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, image, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_IMAGE, (__vkObject*)img);

    __vk_DestroyImage(device, image, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetImageSubresourceLayout(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), device, image, pSubresource, pLayout);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSubresource || !pLayout)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetImageSubresourceLayout(device, image, pSubresource, pLayout);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateImageView(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pView)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateImageView(device, pCreateInfo, pAllocator, pView);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_IMAGE_VIEW, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pView));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (view=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pView));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImageView *imv = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImageView *, imageView);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyImageView(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, imageView, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!imv || imv->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE_VIEW))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_IMAGE_VIEW, (__vkObject*)imv);

    __vk_DestroyImageView(device, imageView, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateShaderModule(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    __vkLogShaderStrings(pCreateInfo->codeSize, (unsigned char *)pCreateInfo->pCode);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pShaderModule)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_SHADER_MODULE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pShaderModule));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (shaderModule=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pShaderModule));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkShaderModule *shm = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkShaderModule *, shaderModule);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyShaderModule(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, shaderModule, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!shm || shm->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SHADER_MODULE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_SHADER_MODULE, (__vkObject*)shm);

    __vk_DestroyShaderModule(device, shaderModule, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreatePipelineCache(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPipelineCache)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_PIPELINE_CACHE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pPipelineCache));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (pipelineCache=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPipelineCache));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineCache *ppc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache *, pipelineCache);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyPipelineCache(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, pipelineCache, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppc || ppc->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_CACHE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_PIPELINE_CACHE, (__vkObject*)ppc);

    __vk_DestroyPipelineCache(device, pipelineCache, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineCache *ppc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache *, pipelineCache);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPipelineCacheData(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), device, pipelineCache, pDataSize, pData);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppc || ppc->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_CACHE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetPipelineCacheData(device, pipelineCache, pDataSize, pData);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_MergePipelineCaches(VkDevice device, VkPipelineCache destCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineCache *ppc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache *, destCache);
    __vkPipelineCache *spc;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkMergePipelineCaches(%p, 0x%llx, %u, %p)", gcoOS_GetCurrentThreadID(), device, destCache, srcCacheCount, pSrcCaches);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppc || ppc->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_CACHE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (srcCacheCount > 0 && !pSrcCaches)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < srcCacheCount; i++)
    {
        spc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineCache *, pSrcCaches[i]);
        if (!spc || spc->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_CACHE))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    result = __vk_MergePipelineCaches(device, destCache, srcCacheCount, pSrcCaches);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateGraphicsPipelines(%p, 0x%llx, %u, %p %p)", gcoOS_GetCurrentThreadID(), device, pipelineCache, createInfoCount, pCreateInfos, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfos || pCreateInfos->sType != VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPipelines)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_PIPELINE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pPipelines));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (pipeline=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPipelines));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateComputePipelines(%p, 0x%llx, %u, %p, %p)", gcoOS_GetCurrentThreadID(), device, pipelineCache, createInfoCount, pCreateInfos, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfos || pCreateInfos->sType != VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPipelines)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_PIPELINE, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pPipelines));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (pipeline=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPipelines));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipeline *ppl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyPipeline(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, pipeline, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppl || ppl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_PIPELINE, (__vkObject*)ppl);

    __vk_DestroyPipeline(device, pipeline, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreatePipelineLayout(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPipelineLayout)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_PIPELINE_LAYOUT, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pPipelineLayout));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (pipelineLayout=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pPipelineLayout));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkPipelineLayout *ppl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout *, pipelineLayout);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyPipelineLayout(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, pipelineLayout, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppl || ppl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_LAYOUT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_PIPELINE_LAYOUT, (__vkObject*)ppl);

    __vk_DestroyPipelineLayout(device, pipelineLayout, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateSampler(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSampler)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateSampler(device, pCreateInfo, pAllocator, pSampler);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_SAMPLER, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pSampler));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (sampler=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSampler));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSampler *spl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSampler *, sampler);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySampler(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, sampler, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!spl || spl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SAMPLER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_SAMPLER, (__vkObject*)spl);

    __vk_DestroySampler(device, sampler, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDescriptorSetLayout(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSetLayout)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pSetLayout));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (layout=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSetLayout));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorSetLayout *dsl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetLayout *, descriptorSetLayout);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySampler(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, descriptorSetLayout, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dsl || dsl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTORSET_LAYOUT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTORSET_LAYOUT, (__vkObject*)dsl);

    __vk_DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDescriptorPool(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pDescriptorPool)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_DESCRIPTOR_POOL, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pDescriptorPool));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (descPool=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pDescriptorPool));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, descriptorPool);
    VkResult result = VK_SUCCESS;
    uint32_t i = 0;

    __VK_LOG_API("(tid=%p): vkDestroyDescriptorPool(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, descriptorPool, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dsp || dsp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTOR_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (dsp)
    {
        __vkDescriptorSet *descSet;
        for (i = 0; i < dsp->maxSets; i++)
        {
            __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, &dsp->pDescSets[i]);
            descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
            if (descSet)
            {
                if (descSet->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTORSET))
                {
                    result = __VK_ERROR_INVALID_HANDLE;
                    goto vk_Exit;
                }

                __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject*)descSet);
            }
        }
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTOR_POOL, (__vkObject*)dsp);

    __vk_DestroyDescriptorPool(device, descriptorPool, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, descriptorPool);
    VkResult result = VK_SUCCESS;
    uint32_t i = 0;

    __VK_LOG_API("(tid=%p): vkResetDescriptorPool(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), device, descriptorPool, flags);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dsp || dsp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTOR_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    /* Loop through the command pool's command buffers */
    if (dsp)
    {
        __vkDescriptorSet *descSet;
        for (i = 0; i < dsp->maxSets; i++)
        {
            __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, &dsp->pDescSets[i]);
            descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
            if (descSet)
            {
                if (descSet->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTORSET))
                {
                    result = __VK_ERROR_INVALID_HANDLE;
                    goto vk_Exit;
                }

                __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject*)descSet);
            }
        }
    }

    result = __vk_ResetDescriptorPool(device, descriptorPool, flags);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    __vkDescriptorSet *des;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkAllocateDescriptorSets(%p, %p)", gcoOS_GetCurrentThreadID(), device, pAllocateInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pAllocateInfo || pAllocateInfo->sType != VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);

    if (result == VK_SUCCESS)
    {
        for (i = 0; i < pAllocateInfo->descriptorSetCount; i++)
        {
            __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pDescriptorSets[i]);
            des = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
            __vk_InsertObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject*)des);
        }
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (descSet=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pDescriptorSets));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet* pDescriptorSets)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkDescriptorPool *dsp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorPool *, descriptorPool);
    VkResult result = VK_SUCCESS;
    __vkDescriptorSet *des;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkFreeDescriptorSets(%p, 0x%llx, %u, %p)", gcoOS_GetCurrentThreadID(), device, descriptorPool, count, pDescriptorSets);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dsp || dsp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTOR_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pDescriptorSets)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < count; i++)
    {
        __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pDescriptorSets[i]);
        des = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
        if (!des || des->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTORSET))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }

        __vk_RemoveObject(devCtx, __VK_OBJECT_DESCRIPTORSET, (__vkObject*)des);
    }

    result = __vk_FreeDescriptorSets(device, descriptorPool, count, pDescriptorSets);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_UpdateDescriptorSets(VkDevice device, uint32_t writeCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t copyCount, const VkCopyDescriptorSet* pDescriptorCopies)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkUpdateDescriptorSets(%p, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), device, writeCount, pDescriptorWrites, copyCount, pDescriptorCopies);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (writeCount && !pDescriptorWrites)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (copyCount && !pDescriptorCopies)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < writeCount; i++)
    {
        if (pDescriptorWrites[i].sType != VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }
    for (i = 0; i < copyCount; i++)
    {
        if (pDescriptorCopies[i].sType != VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET)
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    __vk_UpdateDescriptorSets(device, writeCount, pDescriptorWrites, copyCount, pDescriptorCopies);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateFramebuffer(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFramebuffer)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_FRAMEBUFFER, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pFramebuffer));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (framebuffer=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pFramebuffer));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFramebuffer *fbb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFramebuffer *, framebuffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyFramebuffer(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, framebuffer, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!fbb || fbb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FRAMEBUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_FRAMEBUFFER, (__vkObject*)fbb);

    __vk_DestroyFramebuffer(device, framebuffer, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateRenderPass(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pRenderPass)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_RENDER_PASS, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pRenderPass));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (renderPass=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pRenderPass));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkRenderPass *rdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, renderPass);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyRenderPass(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, renderPass, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!rdp || rdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_RENDER_PASS))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_RENDER_PASS, (__vkObject*)rdp);

    __vk_DestroyRenderPass(device, renderPass, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkRenderPass *rdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, renderPass);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetRenderAreaGranularity(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, renderPass, pGranularity);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!rdp || rdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_RENDER_PASS))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pGranularity)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetRenderAreaGranularity(device, renderPass, pGranularity);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateCommandPool(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCommandPool)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_COMMAND_POOL, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pCommandPool));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (commandPool=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCommandPool));
    devCtx->currentResult = result;

    return result;
}

static VkResult __vk_RemoveCommandBuffersObject(
    VkDevice device,
    VkCommandPool commandPool
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    VkResult result = VK_SUCCESS;
    VkCommandBuffer pBuf = cdp->vkCmdBufferList;

    /* Remove any remaining command buffers that the app forgot */
    while (pBuf)
    {
        __vkCommandBuffer *cmd = (__vkCommandBuffer *)pBuf;

        if (cmd && commandPool == cmd->commandPool)
        {
            pBuf = (VkCommandBuffer)cmd->next;
            __vk_RemoveObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject*)cmd);
        }
        else
        {
            result = __VK_ERROR_INVALID_HANDLE;
            break;
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyCommandPool(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, commandPool, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!cdp || cdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    /* before free cdp, driver will try to free cmd obj in cdp which
    ** app forget to do. In validation layer, the global object managment
    ** need keep the same logic
    */
    __vk_RemoveCommandBuffersObject(device, commandPool);

    __vk_RemoveObject(devCtx, __VK_OBJECT_COMMAND_POOL, (__vkObject*)cdp);

    __vk_DestroyCommandPool(device, commandPool, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkResetCommandPool(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), device, commandPool, flags);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!cdp || cdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ResetCommandPool(device, commandPool, flags);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;
    __vkCommandPool *cdp;
    __vkCommandBuffer *cmb;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkAllocateCommandBuffers(%p, %p)", gcoOS_GetCurrentThreadID(), device, pAllocateInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pAllocateInfo || pAllocateInfo->sType != VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, pAllocateInfo->commandPool);
    if (cdp && cdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_POOL))
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pCommandBuffers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);

    if (result == VK_SUCCESS)
    {
        for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
        {
            cmb = (__vkCommandBuffer *)pCommandBuffers[i];
            __vk_InsertObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject*)cmb);
        }
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (commandBuffer=%p)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCommandBuffers));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    VkResult result = VK_SUCCESS;
    __vkCommandBuffer *cmb;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkFreeCommandBuffers(%p, 0x%llx, %u, %p)", gcoOS_GetCurrentThreadID(), device, commandPool, commandBufferCount, pCommandBuffers);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!cdp || cdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    for (i = 0; i < commandBufferCount; i++)
    {
        cmb = (__vkCommandBuffer *)pCommandBuffers[i];
        if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }

        __vk_RemoveObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject*)cmb);
    }

    __vk_FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkBeginCommandBuffer(%p, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, pBeginInfo);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pBeginInfo || pBeginInfo->sType != VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_BeginCommandBuffer(commandBuffer, pBeginInfo);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEndCommandBuffer(%p)", gcoOS_GetCurrentThreadID(), commandBuffer);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_EndCommandBuffer(commandBuffer);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkResetCommandBuffer(%p, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, flags);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ResetCommandBuffer(commandBuffer, flags);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *ppl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdBindPipeline(%p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, pipelineBindPoint, pipeline);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pipelineBindPoint > VK_PIPELINE_BIND_POINT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!ppl || ppl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewPort, uint32_t viewportCount, const VkViewport* pViewports)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetViewport(%p, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, firstViewPort, viewportCount, pViewports);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetViewport(commandBuffer, firstViewPort, viewportCount, pViewports);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetScissor(%p, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, firstScissor, scissorCount, pScissors);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetLineWidth(%p, %f)", gcoOS_GetCurrentThreadID(), commandBuffer, lineWidth);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetLineWidth(commandBuffer, lineWidth);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetDepthBias(%p, %f, %f, %f)", gcoOS_GetCurrentThreadID(), commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetBlendConstants(%p, %f %f %f %f)", gcoOS_GetCurrentThreadID(), commandBuffer,
                                    blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetBlendConstants(commandBuffer, blendConstants);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetDepthBounds(%p, %f %f)", gcoOS_GetCurrentThreadID(), commandBuffer, minDepthBounds, maxDepthBounds);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetStencilCompareMask(%p, %u %u)", gcoOS_GetCurrentThreadID(), commandBuffer, faceMask, compareMask);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

 VKAPI_ATTR void VKAPI_CALL __valid_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetStencilWriteMask(%p, %u %u)", gcoOS_GetCurrentThreadID(), commandBuffer, faceMask, writeMask);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

 VKAPI_ATTR void VKAPI_CALL __valid_CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetStencilReference(%p, %u %u)", gcoOS_GetCurrentThreadID(), commandBuffer, faceMask, reference);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetStencilReference(commandBuffer, faceMask, reference);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkPipelineLayout *ppl = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout *, layout);
    __vkDescriptorSet *des;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkCmdBindDescriptorSets(%p, %u, 0x%llx, %u, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pipelineBindPoint > VK_PIPELINE_BIND_POINT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!ppl || ppl->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_LAYOUT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    for (i = 0; i < setCount; i++)
    {
        __vkDescriptorSetEntry *pDescSets = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSetEntry *, pDescriptorSets[i]);
        des = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescSets->descSet);
        if (!des || des->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_DESCRIPTORSET))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }
    if (dynamicOffsetCount > 0 && !pDynamicOffsets)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, setCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdBindIndexBuffer(%p, 0x%llx, %llu, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, buffer, offset, indexType);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (indexType > VK_INDEX_TYPE_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }

    __vk_CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t startBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkCmdBindVertexBuffers(%p, %u, %u, %p, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, startBinding, bindingCount, pBuffers, pOffsets);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pBuffers || !pOffsets)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < bindingCount; i++)
    {
        bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, pBuffers[i]);
        if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    __vk_CmdBindVertexBuffers(commandBuffer, startBinding, bindingCount, pBuffers, pOffsets);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDraw(%p, %u, %u, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDrawIndexed(%p, %u, %u, %u, %d, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDrawIndirect(%p, 0x%llx, %llu, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, buffer, offset, count, stride);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDrawIndirect(commandBuffer, buffer, offset, count, stride);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDrawIndexedIndirect(%p, 0x%llx, %llu, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, buffer, offset, count, stride);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDrawIndexedIndirect(commandBuffer, buffer, offset, count, stride);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDispatch(%p, %u, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, x, y, z);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDispatch(commandBuffer, x, y, z);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, buffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDispatchIndirect(%p, 0x%llx, %llu)", gcoOS_GetCurrentThreadID(), commandBuffer, buffer, offset);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDispatchIndirect(commandBuffer, buffer, offset);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *sbf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, srcBuffer);
    __vkBuffer *dbf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdCopyBuffer(%p, 0x%llx, 0x%llx, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcBuffer, destBuffer, regionCount, pRegions);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!sbf || sbf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dbf || dbf->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, regionCount, pRegions);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkImage *smg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, srcImage);
    __vkImage *dmg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, destImage);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdCopyImage(%p, 0x%llx, %u, 0x%llx, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!smg || smg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dmg || dmg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (srcImageLayout > VK_IMAGE_LAYOUT_END_RANGE || destImageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (regionCount > 0 && !pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdCopyImage(commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkImage *smg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, srcImage);
    __vkImage *dmg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, destImage);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdBlitImage(%p, 0x%llx, %u, 0x%llx, %u, %u, %p, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions, filter);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!smg || smg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dmg || dmg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (srcImageLayout > VK_IMAGE_LAYOUT_END_RANGE || destImageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (regionCount > 0 && !pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (filter > VK_FILTER_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }

    __vk_CmdBlitImage(commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions, filter);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, srcBuffer);
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, destImage);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdCopyBufferToImage(%p, 0x%llx, 0x%llx, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcBuffer, destImage, destImageLayout, regionCount, pRegions);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (destImageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (regionCount > 0 && !pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdCopyBufferToImage(commandBuffer, srcBuffer, destImage, destImageLayout, regionCount, pRegions);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer destBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, srcImage);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdCopyImageToBuffer(%p, 0x%llx, %u, 0x%llx, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcImage, srcImageLayout, destBuffer, regionCount, pRegions);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (srcImageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (regionCount > 0 && !pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, destBuffer, regionCount, pRegions);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize dataSize, const void* pData)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdUpdateBuffer(%p, 0x%llx, %llu, %llu, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, destBuffer, destOffset, dataSize, pData);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (dataSize > 0 && !pData)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdUpdateBuffer(commandBuffer, destBuffer, destOffset, dataSize, pData);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize fillSize, uint32_t data)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdFillBuffer(%p, 0x%llx, %llu, %llu, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, destBuffer, destOffset, fillSize, data);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdFillBuffer(commandBuffer, destBuffer, destOffset, fillSize, data);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdClearColorImage(%p, 0x%llx, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (imageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pColor || !pRanges)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkImage *img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdClearDepthStencilImage(%p, 0x%llx, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!img || img->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (imageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (rangeCount > 0 && !pRanges)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdClearAttachments(%p, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, attachmentCount, pAttachments, rectCount, pRects);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pAttachments)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pRects)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkImage *smg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, srcImage);
    __vkImage *dmg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, destImage);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdResolveImage(%p, 0x%llx, %u, 0x%llx, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!smg || smg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dmg || dmg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (srcImageLayout > VK_IMAGE_LAYOUT_END_RANGE || destImageLayout > VK_IMAGE_LAYOUT_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (regionCount > 0 && !pRegions)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdResolveImage(commandBuffer, srcImage, srcImageLayout, destImage, destImageLayout, regionCount, pRegions);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetEvent(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, event, stageMask);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetEvent(commandBuffer, event, stageMask);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdResetEvent(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, event, stageMask);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdResetEvent(commandBuffer, event, stageMask);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkEvent *evt;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkCmdWaitEvents(%p, %u, %p, %u, %u, %u, %p, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, eventCount, pEvents, srcStageMask, destStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);



    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    for (i = 0; i < eventCount; i++)
    {
        evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, pEvents[i]);
        if (!evt || evt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_EVENT))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }
    if (memoryBarrierCount > 0 && !pMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (bufferMemoryBarrierCount > 0 && !pBufferMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (imageMemoryBarrierCount > 0 && !pImageMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, destStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags destStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdPipelineBarrier(%p, %u, %u, %u, %u, %p, %u, %p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, srcStageMask, destStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);



    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (memoryBarrierCount > 0 && !pMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (bufferMemoryBarrierCount > 0 && !pBufferMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (imageMemoryBarrierCount > 0 && !pImageMemoryBarriers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdPipelineBarrier(commandBuffer, srcStageMask, destStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdBeginQuery(%p, 0x%llx, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, queryPool, slot, flags);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdBeginQuery(commandBuffer, queryPool, slot, flags);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t slot)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdEndQuery(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, queryPool, slot);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdEndQuery(commandBuffer, queryPool, slot);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdResetQueryPool(%p, 0x%llx, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, queryPool, firstQuery, queryCount);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t entry)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdWriteTimestamp(%p, %u, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, pipelineStage, queryPool, entry);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (pipelineStage > VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }

    __vk_CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, entry);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize destStride, VkQueryResultFlags flags)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    __vkBuffer *bfr = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdCopyQueryPoolResults(%p, 0x%llx, %u, %u, 0x%llx, %llu, %llu, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, queryPool, firstQuery, queryCount, destBuffer, destOffset, destStride, flags);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!qyp || qyp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_QUERY_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!bfr || bfr->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, destBuffer, destOffset, destStride, flags);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t start, uint32_t length, const void* values)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    __vkPipelineLayout *plt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout *, layout);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdPushConstants(%p, 0x%llx, %u, %u, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, layout, stageFlags, start, length, values);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!plt || plt->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_PIPELINE_LAYOUT))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (length > 0 && !values)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdPushConstants(commandBuffer, layout, stageFlags, start, length, values);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdBeginRenderPass(%p, %p, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, pRenderPassBegin, contents);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pRenderPassBegin || pRenderPassBegin->sType != VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdNextSubpass(%p, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, contents);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (contents > VK_SUBPASS_CONTENTS_END_RANGE)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }

    __vk_CmdNextSubpass(commandBuffer, contents);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdEndRenderPass(VkCommandBuffer commandBuffer)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdEndRenderPass(%p)", gcoOS_GetCurrentThreadID(), commandBuffer);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdEndRenderPass(commandBuffer);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

    cmb->result = result;
    cmb->obj.pDevContext->currentResult = result;
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBuffersCount, const VkCommandBuffer* pCmdBuffers)
{
    VkResult result = VK_SUCCESS;
    __vkCommandBuffer *cmd;
    uint32_t i;

    __VK_LOG_API("(tid=%p): vkCmdExecuteCommands(%p, %u, %p)", gcoOS_GetCurrentThreadID(), commandBuffer, commandBuffersCount, pCmdBuffers);

    /* API validation logic that can be skipped at runtime */
    if (!pCmdBuffers)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    for (i = 0; i < commandBuffersCount; i++)
    {
        cmd = (__vkCommandBuffer *)pCmdBuffers[i];
        if (!cmd || cmd->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
        {
            result = __VK_ERROR_INVALID_HANDLE;
            goto vk_Exit;
        }
    }

    __vk_CmdExecuteCommands(commandBuffer, commandBuffersCount, pCmdBuffers);

vk_Exit:
    __VK_LOG_API(" --> %s\n", __vkiGetResultString(result));

//    cmd->result = result;
//    cmd->obj.pDevContext->currentResult = result;
}

/* Vulkan 1.1 APIs */

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumerateInstanceVersion(uint32_t* pApiVersion)
{
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumerateInstanceVersion(%p)", gcoOS_GetCurrentThreadID(), pApiVersion);

    /* API validation logic that can be skipped at runtime */
    if (!pApiVersion)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_EnumerateInstanceVersion(pApiVersion);

vk_Exit:
    __VK_LOG_API(" ==> %s (ApiVersion=%d)\n", __vkiGetResultString(result), __VK_PTRVALUE(pApiVersion));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkBindBufferMemory2(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, bindInfoCount, pBindInfos);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pBindInfos || pBindInfos->sType != VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_BindBufferMemory2(device, bindInfoCount, pBindInfos);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkBindImageMemory2(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, bindInfoCount, pBindInfos);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pBindInfos || pBindInfos->sType != VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_BindImageMemory2(device, bindInfoCount, pBindInfos);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDeviceGroupPeerMemoryFeatures(%p, %u, %u, %u, %p)", gcoOS_GetCurrentThreadID(), device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPeerMemoryFeatures)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdSetDeviceMask(%p, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, deviceMask);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdSetDeviceMask(commandBuffer, deviceMask);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    __vkCommandBuffer *cmb = (__vkCommandBuffer *)commandBuffer;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCmdDispatchBase(%p, %u, %u, %u, %u, %u, %u)", gcoOS_GetCurrentThreadID(), commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);

    /* API validation logic that can be skipped at runtime */
    if (!cmb || cmb->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_BUFFER))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkEnumeratePhysicalDeviceGroups(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!pPhysicalDeviceGroupCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    if (pPhysicalDeviceGroupProperties)
    {
        if (*pPhysicalDeviceGroupCount > inst->physicalDeviceGroupCount)
        {
            result = __VK_ERROR_INVALID_VALUE;
            goto vk_Exit;
        }
    }

    result = __vk_EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetImageMemoryRequirements2(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pInfo, pMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pInfo || pInfo->sType != VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pMemoryRequirements || pMemoryRequirements->sType != VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetBufferMemoryRequirements2(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pInfo, pMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pInfo || pInfo->sType != VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pMemoryRequirements || pMemoryRequirements->sType != VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetImageSparseMemoryRequirements2(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pInfo || pInfo->sType != VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pSparseMemoryRequirements || pSparseMemoryRequirements->sType != VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceFeatures2(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pFeatures);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFeatures || pFeatures->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceProperties2(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pProperties || pProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceProperties2(physicalDevice, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceFormatProperties2(%p, %u, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, format, pFormatProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFormatProperties || pFormatProperties->sType != VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceImageFormatProperties2(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pImageFormatInfo, pImageFormatProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pImageFormatInfo || pImageFormatInfo->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pImageFormatProperties || pImageFormatProperties->sType != VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceQueueFamilyProperties2(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pQueueFamilyProperties || pQueueFamilyProperties->sType != VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceMemoryProperties2(%p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pMemoryProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pMemoryProperties || pMemoryProperties->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSparseImageFormatProperties2(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pFormatInfo, pPropertyCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pFormatInfo || pFormatInfo->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pProperties || pProperties->sType != VK_STRUCTURE_TYPE_SPARSE_IMAGE_FORMAT_PROPERTIES_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkTrimCommandPool(%p, 0x%llx, %u)", gcoOS_GetCurrentThreadID(), device, commandPool, flags);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!cdp || cdp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_COMMAND_POOL))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_TrimCommandPool(device, commandPool, flags);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDeviceQueue2(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pQueueInfo, pQueue);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pQueueInfo || pQueueInfo->sType != VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pQueue)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDeviceQueue2(device, pQueueInfo, pQueue);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateSamplerYcbcrConversion(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator, pYcbcrConversion);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pYcbcrConversion)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySamplerYcbcrConversion(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, ycbcrConversion, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDescriptorUpdateTemplate(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pDescriptorUpdateTemplate)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyDescriptorUpdateTemplate(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, descriptorUpdateTemplate, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkUpdateDescriptorSetWithTemplate(%p, 0x%llx, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, descriptorSet, descriptorUpdateTemplate, pData);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pData)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceExternalBufferProperties(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pExternalBufferInfo, pExternalBufferProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pExternalBufferInfo || pExternalBufferInfo->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pExternalBufferProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceExternalFenceProperties(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pExternalFenceInfo, pExternalFenceProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pExternalFenceInfo || pExternalFenceInfo->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FENCE_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pExternalFenceProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceExternalSemaphoreProperties(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pExternalSemaphoreInfo || pExternalSemaphoreInfo->sType != VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_INFO)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pExternalSemaphoreProperties)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDescriptorSetLayoutSupport(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pSupport);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_SUPPORT)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }
    if (!pSupport)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

/* Vulkan Extension APIs */

VKAPI_ATTR void VKAPI_CALL __valid_DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySurfaceKHR(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), instance, surface, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!surface)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

     __vk_DestroySurfaceKHR(instance, surface, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSurfaceSupportKHR(%p, %u, 0x%llx)", gcoOS_GetCurrentThreadID(), physicalDevice, queueFamilyIndex, surface);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!surface)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (queueFamilyIndex >= phyDev->queueFamilyCount)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }
    if (!pSupported)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);

vk_Exit:
    __VK_LOG_API(" ==> %s (supported=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSupported));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSurfaceCapabilitiesKHR(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, surface, pSurfaceCapabilities);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!surface)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurfaceCapabilities)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSurfaceFormatsKHR(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!surface)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurfaceFormatCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceSurfacePresentModesKHR(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, surface, pPresentModeCount, pPresentModes);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!surface)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPresentModeCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateSwapchainKHR(VkDevice  device, const VkSwapchainCreateInfoKHR*  pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR*  pSwapchain)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateSwapchainKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSwapchain)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);

    if (result == VK_SUCCESS)
    {
        __vk_InsertObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkObject*, *pSwapchain));
    }

vk_Exit:
    __VK_LOG_API(" ==> %s (swapChain=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSwapchain));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroySwapchainKHR(VkDevice  device, VkSwapchainKHR  swapchain, const VkAllocationCallbacks* pAllocator)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSwapchainKHR *swp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroySwapchainKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, swapchain, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!swp || swp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SWAPCHAIN_KHR))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_RemoveObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject*)swp);

    __vk_DestroySwapchainKHR(device, swapchain, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSwapchainKHR *swp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetSwapchainImagesKHR(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), device, swapchain, pSwapchainImageCount, pSwapchainImages);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!swp || swp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SWAPCHAIN_KHR))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSwapchainImageCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);

vk_Exit:
    __VK_LOG_API(" ==> %s (swapchainImageCount=%llu)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSwapchainImageCount));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSwapchainKHR *swp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSwapchainKHR *, swapchain);
    __vkSemaphore *smp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkAcquireNextImageKHR(%p, 0x%llx, %llu, %u, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, swapchain, timeout, semaphore, fence, pImageIndex);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!swp || swp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SWAPCHAIN_KHR))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (smp && (smp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SEMAPHORE)))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (fce && (fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE)))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);

vk_Exit:
    __VK_LOG_API(" ==> %s (imageIndex=%u)\n", __vkiGetResultString(result), __VK_PTRVALUE(pImageIndex));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_QueuePresentKHR(VkQueue  queue, const VkPresentInfoKHR*  pPresentInfo)
{
    __vkDevQueue *pqe = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkQueuePresentKHR(%p, %p)", gcoOS_GetCurrentThreadID(), queue, pPresentInfo);

    /* API validation logic that can be skipped at runtime */
    if (!pqe || pqe->sType != __VK_OBJECT_TYPE_CMD_QUEUE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPresentInfo || pPresentInfo->sType != VK_STRUCTURE_TYPE_PRESENT_INFO_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_QueuePresentKHR(queue, pPresentInfo);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    pqe->pDevContext->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities)
{
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = (__vkDevContext *)device;

    __VK_LOG_API("(tid=%p): vkGetDeviceGroupPresentCapabilitiesKHR(%p, %p)", gcoOS_GetCurrentThreadID(), device, pDeviceGroupPresentCapabilities);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!pDeviceGroupPresentCapabilities)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes)
{
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = (__vkDevContext *)device;

    __VK_LOG_API("(tid=%p): vkGetDeviceGroupSurfacePresentModesKHR(%p, 0x%llx, %p)", gcoOS_GetCurrentThreadID(), device, surface, pModes);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!surface || !pModes)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects)
{
    VkResult result = VK_SUCCESS;
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDevicePresentRectanglesKHR(%p, 0x%llx, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, surface, pRectCount, pRects);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!surface || !pRectCount)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (*pRectCount != 0 && !pRects)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex)
{
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = (__vkDevContext *)device;

    __VK_LOG_API("(tid=%p): vkAcquireNextImage2KHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pAcquireInfo, pImageIndex);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    if (!pAcquireInfo || !pImageIndex)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceDisplayPropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPropertiesKHR* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceDisplayPropertiesKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pPropertyCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, pPropertyCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetPhysicalDeviceDisplayPlanePropertiesKHR(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkDisplayPlanePropertiesKHR* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceDisplayPlanePropertiesKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, pPropertyCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice, pPropertyCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetDisplayPlaneSupportedDisplaysKHR(VkPhysicalDevice physicalDevice, uint32_t planeIndex, uint32_t* pDisplayCount, VkDisplayKHR* pDisplays)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDisplayPlaneSupportedDisplaysKHR(%p, %u, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, planeIndex, pDisplayCount, pDisplays);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pDisplayCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetDisplayPlaneSupportedDisplaysKHR(physicalDevice, planeIndex, pDisplayCount, pDisplays);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetDisplayModePropertiesKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, uint32_t* pPropertyCount, VkDisplayModePropertiesKHR* pProperties)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkDisplayKHR *dpy = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayKHR *, display);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDisplayModePropertiesKHR(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, display, pPropertyCount, pProperties);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dpy)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pPropertyCount)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_GetDisplayModePropertiesKHR(physicalDevice, display, pPropertyCount, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDisplayModeKHR(VkPhysicalDevice physicalDevice, VkDisplayKHR display, const VkDisplayModeCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDisplayModeKHR* pMode)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkDisplayKHR *dpy = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayKHR *, display);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDisplayModeKHR(%p, %p, %p, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, display, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dpy)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_CreateDisplayModeKHR(physicalDevice, display, pCreateInfo, pAllocator, pMode);

vk_Exit:
    __VK_LOG_API(" ==> %s (displayMode=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pMode));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetDisplayPlaneCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR* pCapabilities)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkDisplayModeKHR *dpm = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayModeKHR *, mode);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetDisplayPlaneCapabilitiesKHR(%p, %p, %u, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, mode, planeIndex, pCapabilities);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dpm || dpm->sType != __VK_OBJECT_TYPE_DISPLAY_MODE_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCapabilities)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    __vk_GetDisplayPlaneCapabilitiesKHR(physicalDevice, mode, planeIndex, pCapabilities);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDisplayPlaneSurfaceKHR(VkInstance instance, const VkDisplaySurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDisplayPlaneSurfaceKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurface)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

     result = __vk_CreateDisplayPlaneSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

vk_Exit:
    __VK_LOG_API(" ==> %s (surface=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSurface));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateSharedSwapchainsKHR(VkDevice device, uint32_t swapchainCount, const VkSwapchainCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchains)
{
//  __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateSharedSwapchainsKHR(%p, %u, %p, %p)", gcoOS_GetCurrentThreadID(), device, swapchainCount, pCreateInfos, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!pCreateInfos || pCreateInfos->sType != VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

     result = __vk_CreateSharedSwapchainsKHR(device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);

vk_Exit:
    __VK_LOG_API(" ==> %s (swapChain=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSwapchains));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateDebugReportCallbackEXT(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pCallback)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

     result = __vk_CreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);

vk_Exit:
    __VK_LOG_API(" ==> %s (Callback=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pCallback));

    return result;
}

VKAPI_ATTR void VKAPI_CALL __valid_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
{
    __vkInstance *inst = (__vkInstance *)instance;
    __vkDebugCallbackEXT *dcb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDebugCallbackEXT *, callback);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDestroyDebugReportCallbackEXT(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, callback, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!dcb || dcb->sType != __VK_OBJECT_TYPE_DEBUG_CB_EXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_DestroyDebugReportCallbackEXT(instance, callback, pAllocator);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

VKAPI_ATTR void VKAPI_CALL __valid_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkDebugReportMessageEXT(%p, %u, %u, 0x%llx, %u, %d,\n%s, %s)", gcoOS_GetCurrentThreadID(), instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    __vk_DebugReportMessageEXT(instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
}

#ifdef VK_USE_PLATFORM_XLIB_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateXlibSurfaceKHR(VkInstance instance, Display* dpy, Window window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __valid_GetPhysicalDeviceXlibPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display* dpy, VisualID visualID)
{
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateXcbSurfaceKHR(VkInstance instance, xcb_connection_t* connection, xcb_window_t window, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __valid_GetPhysicalDeviceXcbPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, xcb_connection_t* connection, xcb_visualid_t visual_id)
{
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateWaylandSurfaceKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurface)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

     result = __vk_CreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

vk_Exit:
    __VK_LOG_API(" ==> %s (surface=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSurface));

    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __valid_GetPhysicalDeviceWaylandPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, struct wl_display* display)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkBool32 result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceWaylandPresentationSupportKHR(%p, %u, %p)", gcoOS_GetCurrentThreadID(), physicalDevice, queueFamilyIndex, display);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (queueFamilyIndex >= phyDev->queueFamilyCount)
    {
        result = __VK_ERROR_INVALID_VALUE;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, display);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}
#endif

#ifdef VK_USE_PLATFORM_MIR_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateMirSurfaceKHR(VkInstance instance, MirConnection* connection, MirSurface* mirSurface, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __valid_GetPhysicalDeviceMirPresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, MirConnection* connection)
{
    return VK_TRUE;
}
#endif

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateAndroidSurfaceKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurface)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

     result = __vk_CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

vk_Exit:
    __VK_LOG_API(" ==> %s (surface=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSurface));

    return result;
}

#if (ANDROID_SDK_VERSION >= 24)
VKAPI_ATTR VkResult VKAPI_CALL __valid_GetSwapchainGrallocUsageANDROID(VkDevice device, VkFormat format, VkImageUsageFlags imageUsage, int* grallocUsage)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetSwapchainGrallocUsageANDROID(%p, %u, %u)", gcoOS_GetCurrentThreadID(), device, format, imageUsage);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetSwapchainGrallocUsageANDROID(device, format, imageUsage, grallocUsage);

vk_Exit:
    __VK_LOG_API(" ==> %s (grallocUsage=%d)\n", __vkiGetResultString(result), __VK_PTRVALUE(grallocUsage));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_AcquireImageANDROID(VkDevice device, VkImage image, int nativeFenceFd, VkSemaphore semaphore, VkFence fence)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkImage *pImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, image);
    __vkSemaphore *smp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkAcquireImageANDROID(%p, 0x%llx, %d, %u, 0x%llx)", gcoOS_GetCurrentThreadID(), device, image, nativeFenceFd, semaphore, fence);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pImg || pImg->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_IMAGE))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (smp && (smp->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_SEMAPHORE)))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (fce && (fce->obj.sType != __VK_OBJECT_INDEX_TO_TYPE(__VK_OBJECT_FENCE)))
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_AcquireImageANDROID(device, image, nativeFenceFd, semaphore, fence);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_QueueSignalReleaseImageANDROID(VkQueue queue, uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores, VkImage image, int* pNativeFenceFd)
{
    __vkDevQueue *pqe = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkQueueSignalReleaseImageANDROID(%p, %u, %p, 0x%llx)", gcoOS_GetCurrentThreadID(), queue, waitSemaphoreCount, pWaitSemaphores, image);

    if (!pqe || pqe->sType != __VK_OBJECT_TYPE_CMD_QUEUE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_QueueSignalReleaseImageANDROID(queue, waitSemaphoreCount, pWaitSemaphores, image, pNativeFenceFd);

vk_Exit:
    __VK_LOG_API(" ==> %s (nativeFenceFd=%d)\n", __vkiGetResultString(result), __VK_PTRVALUE(pNativeFenceFd));
    pqe->pDevContext->currentResult = result;

    return result;
}
#endif
#if (ANDROID_SDK_VERSION >= 26)

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetAndroidHardwareBufferPropertiesANDROID(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, buffer, pProperties);

    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);

vk_Exit:
    __VK_LOG_API(" ==> %s (property=%p)\n", __vkiGetResultString(result), pProperties);
    devCtx->currentResult = result;

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetMemoryAndroidHardwareBufferANDROID(%p, %p, %p)", gcoOS_GetCurrentThreadID(), device, pInfo, pBuffer);

    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);

vk_Exit:
    __VK_LOG_API(" ==> %s (buffer=%p)\n", __vkiGetResultString(result), *pBuffer);
    devCtx->currentResult = result;

    return result;
}
#  endif
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
VKAPI_ATTR VkResult VKAPI_CALL __valid_CreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface)
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkCreateWin32SurfaceKHR(%p, %p, %p)", gcoOS_GetCurrentThreadID(), instance, pCreateInfo, pAllocator);

    /* API validation logic that can be skipped at runtime */
    if (!inst || inst->sType != __VK_OBJECT_TYPE_INSTANCE)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }
    if (!pSurface)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

     result = __vk_CreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);

vk_Exit:
    __VK_LOG_API(" ==> %s (surface=0x%llx)\n", __vkiGetResultString(result), __VK_PTRVALUE(pSurface));

    return result;
}

VKAPI_ATTR VkBool32 VKAPI_CALL __valid_GetPhysicalDeviceWin32PresentationSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkBool32 result = VK_TRUE;

    __VK_LOG_API("(tid=%p): vkGetPhysicalDeviceWin32PresentationSupportKHR(%p, %u)", gcoOS_GetCurrentThreadID(), physicalDevice, queueFamilyIndex);

    /* API validation logic that can be skipped at runtime */
    if (!phyDev || phyDev->sType != __VK_OBJECT_TYPE_PHYSICAL_DEVICE)
    {
        result = VK_FALSE;
        goto vk_Exit;
    }
    if (queueFamilyIndex >= phyDev->queueFamilyCount)
    {
        result = VK_FALSE;
        goto vk_Exit;
    }

    result = __vk_GetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetSemaphoreWin32HandleKHR(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, pGetWin32HandleInfo, pHandle);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetSemaphoreWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo)
{
   __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkImportSemaphoreWin32HandleKHR(%p, %u)", gcoOS_GetCurrentThreadID(), device, pImportSemaphoreWin32HandleInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ImportSemaphoreWin32HandleKHR(device, pImportSemaphoreWin32HandleInfo);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_GetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo, HANDLE* pHandle)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkGetFenceWin32HandleKHR(%p, %u, %p)", gcoOS_GetCurrentThreadID(), device, pGetWin32HandleInfo, pHandle);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_GetFenceWin32HandleKHR(device, pGetWin32HandleInfo, pHandle);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_ImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo)
{
   __vkDevContext *devCtx = (__vkDevContext *)device;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vkImportFenceWin32HandleKHR(%p, %u)", gcoOS_GetCurrentThreadID(), device, pImportFenceWin32HandleInfo);

    /* API validation logic that can be skipped at runtime */
    if (!devCtx || devCtx->sType != __VK_OBJECT_TYPE_DEV_CONTEXT)
    {
        result = __VK_ERROR_INVALID_HANDLE;
        goto vk_Exit;
    }

    result = __vk_ImportFenceWin32HandleKHR(device, pImportFenceWin32HandleInfo);

vk_Exit:
    __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));

    return result;
}
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL __valid_icdGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    PFN_vkVoidFunction pFunc = gcvNULL;
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vk_icdGetInstanceProcAddr(%p, %s)", gcoOS_GetCurrentThreadID(), instance, pName);

    /* Do not check instance, NULL instance is valid. */
    if (!pName)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    pFunc = __vk_icdGetInstanceProcAddr(instance, pName);

vk_Exit:
    if (__VK_IS_SUCCESS(result))
    {
        __VK_LOG_API(" ==> %p\n", pFunc);
    }
    else
    {
        __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
    }

    return pFunc;
}

VKAPI_ATTR VkResult VKAPI_CALL __valid_icdNegotiateLoaderICDInterfaceVersion(uint32_t *pVersion)
{
    VkResult result = VK_SUCCESS;

    __VK_LOG_API("(tid=%p): vk_icdNegotiateLoaderICDInterfaceVersion(%p)", gcoOS_GetCurrentThreadID(), pVersion);

    if (!pVersion)
    {
        result = __VK_ERROR_INVALID_POINTER;
        goto vk_Exit;
    }

    result = __vk_icdNegotiateLoaderICDInterfaceVersion(pVersion);

vk_Exit:
    if (__VK_IS_SUCCESS(result))
    {
        __VK_LOG_API(" ==> %s (version=%d)\n", __vkiGetResultString(result), __VK_PTRVALUE(pVersion));
    }
    else
    {
        __VK_LOG_API(" ==> %s\n", __vkiGetResultString(result));
        result = VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    return result;
}



#define __vkValid_(func)    __valid_##func,
#define __vkICDValid_(func) __valid_icd##func,

__vkDispatchTable __vkValidEntryFuncTable = {
    __VK_API_ENTRIES(__vkValid_)
#ifdef VK_USE_PLATFORM_XLIB_KHR
    __VK_WSI_XLIB_ENTRIES(__vkValid_)
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    __VK_WSI_XCB_ENTRIES(__vkValid_)
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    __VK_WSI_WAYLAND_ENTRIES(__vkValid_)
#endif
#ifdef VK_USE_PLATFORM_MIR_KHR
    __VK_WSI_MIR_ENTRIES(__vkValid_)
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    __VK_WSI_ANDROID_ENTRIES(__vkValid_)
#if (ANDROID_SDK_VERSION >= 24)
    __VK_WSI_ANDROID_NATIVE_BUFFER_ENTRIES(__vkValid_)
#  endif
#if (ANDROID_SDK_VERSION >= 26)
    __VK_EXT_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_ENTRIES(__vkValid_)
#  endif
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
    __VK_WSI_WIN32_ENTRIES(__vkValid_)
#endif
    __VK_ICD_API_ENTRIES(__vkICDValid_)
};



