/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vk_context_h__
#define __gc_vk_context_h__

typedef struct __vkInstanceRec          __vkInstance;
typedef struct __vkPhysicalDeviceRec    __vkPhysicalDevice;
typedef struct __vkDevContextRec        __vkDevContext;
typedef struct __vkDevQueueRec          __vkDevQueue;
typedef struct __vkBlitResRec           __vkBlitRes;
typedef struct __vkCmdResNodeRec        __vkCmdResNode;
typedef struct __vkUtilsHashRec         __vkUtilsHash;

#include "gc_vk_object.h"
#include "gc_vk_chip.h"
#include "gc_vk_wsi.h"

#define __VK_MAX_PDEV_COUNT             2
#define __VK_MAX_GPU_CORE_COUNT         4
#define __VK_MAX_FENCE_COUNT            16384
#define __VK_MAX_QUERY_COUNT            (4096 / sizeof(uint64_t))
#define __VK_MAX_NAME_LENGTH            256

enum
{
    __VK_PDEV_QUEUEFAMILY_GENERAL      = 0,
    __VK_PDEV_QUEUEFAMILY_BLIT,
    __VK_PDEV_QUEUEFAMILY_MAX,
};

#define __VK_LOG_API(...)  if (__vkEnableApiLog) __VK_PRINT(__VA_ARGS__)

typedef struct __vkDrvCtrlOptionRec
{
    uint32_t affinityMode;
    uint32_t affinityCoreID;
}__vkDrvCtrlOption;

typedef struct __vkChipInfoRec
{
    uint32_t gpuCoreCount;
    uint32_t gpuCoreID[__VK_MAX_GPU_CORE_COUNT];
    uint32_t flatMappingRangeCount;
    gcsFLAT_MAPPING_RANGE flatMappingRanges[gcdMAX_FLAT_MAPPING_COUNT];
}__vkChipInfo;

/* 16 byte for hw fence data because of hardware limitation. */
typedef struct __vkHwFenceDataRec
{
    uint64_t value;
    uint64_t padding;
}
__vkHwFenceData;

struct __vkDevContextRec
{
    /*
     * require 'hwvulkan_dispatch_t' on android-n,
     * which is same as 'VK_LOADER_DATA' currently.
     */
    VK_LOADER_DATA loaderInfo;

    __vkObjectType sType;

    uint32_t threadId;

    VkAllocationCallbacks memCb;

    __vkPhysicalDevice *pPhyDevice;

    /* Logical CMD queues associated with the devContext. */
    uint32_t devQueueCreateCount;

    __vkDevQueue *devQueues[__VK_PDEV_QUEUEFAMILY_MAX];
    uint32_t queueCount[__VK_PDEV_QUEUEFAMILY_MAX];

    uint32_t fenceInUse[__VK_MAX_FENCE_COUNT / 32];
    uint32_t lastFenceIndex;
    uint32_t fenceCount;
    VkBuffer fenceBuffer;

    /* All VK objects created through vkCreate*() API with this devContext. */
    __vkObjectList vkObject[__VK_DEV_OBJECT_COUNT];

#if !__VK_NEW_DEVICE_QUEUE
    /* GCHAL Interfaces */
    gcoHAL hal;
    gcoHARDWARE hardware;       /* Should only be used for feature / cap checks */
#endif
    /* Current error status */
    VkResult currentResult;

    uint32_t context[__VK_MAX_GPU_CORE_COUNT];

#if gcdDUMP
    uint32_t contextPhysical[gcdCONTEXT_BUFFER_COUNT];
    void*    contextLogical[gcdCONTEXT_BUFFER_COUNT];
    uint32_t contextBytes;
    uint32_t currentContext;
#endif


#if __VK_RESOURCE_INFO
    gcsATOM_PTR atom_id;

#if __VK_RESOURCE_SAVE_TGA
    VkQueue         auxQueue;
    VkCommandPool   auxCmdPool;
    VkCommandBuffer auxCmdBuf;
#endif
#endif
    /* shortcuts */
    const __vkDrvCtrlOption *option;
    const __vkChipInfo *chipInfo;
    const gcsFEATURE_DATABASE *database;
    VSC_SYS_CONTEXT vscSysCtx;

    __vkChipFuncTable *chipFuncs;
    void *chipPriv;
    struct __vkDevContextRec *pNext;
};

typedef struct __vkDebugCallbackEXTRec
{
    __vkObjectType sType;

    /* Debug report callback specific fields */
    VkDebugReportCallbackCreateInfoEXT createInfo;
} __vkDebugCallbackEXT;


typedef struct __vkPhysicalDeviceConfigRec
{
    uint32_t chipModel;
    uint32_t chipRevision;
    uint32_t productID;
    uint32_t ecoID;
    uint32_t customerID;
    uint32_t chipFlags;
    const gcsFEATURE_DATABASE *database;
}__vkPhysicalDeviceConfig;

struct __vkPhysicalDeviceRec
{
    /*
     * require 'hwvulkan_dispatch_t' on android-n,
     * which is same as 'VK_LOADER_DATA' currently.
     */
    VK_LOADER_DATA loaderInfo;
    __vkObjectType sType;

    __vkDevContext *devContextList;
    gctPOINTER  mutex;

    __vkInstance *pInst;
#if !__VK_NEW_DEVICE_QUEUE
    gcoHARDWARE hardware;
#endif
    VkPhysicalDeviceFeatures phyDevFeatures;
    VkPhysicalDeviceProperties phyDevProp;
    VkPhysicalDeviceMemoryProperties phyDevMemProp;

    uint32_t queueFamilyCount;
    VkQueueFamilyProperties queueProp[__VK_PDEV_QUEUEFAMILY_MAX];
    VkBool32 queuePresentSupported[__VK_PDEV_QUEUEFAMILY_MAX];

    uint32_t numberOfDisplays;
    __vkDisplayKHR *displays[__VK_WSI_MAX_PHYSICAL_DISPLAYS];

    uint32_t numberOfDisplayPlanes;
    __vkDisplayPlane *displayPlanes[__VK_WSI_MAX_DISPLAY_PLANES];

    gcsGLSLCaps shaderCaps;
    __vkPhysicalDeviceConfig phyDevConfig;
    VSC_CORE_SYS_CONTEXT vscCoreSysCtx;
};

enum
{
    __VK_MGPU_AFFINITY_INVALID     = 0,
    __VK_MGPU_AFFINITY_COMBINE     = 1,
    __VK_MGPU_AFFINITY_INDEPENDENT = 2,
    __VK_MGPU_AFFINITY_NATIVE      = 3,
};

struct __vkInstanceRec
{
    /*
     * require 'hwvulkan_dispatch_t' on android-n,
     * which is same as 'VK_LOADER_DATA' currently.
     */
    VK_LOADER_DATA loaderInfo;
    __vkObjectType sType;

    uint32_t physicalDeviceCount;
    __vkPhysicalDevice physicalDevice[__VK_MAX_PDEV_COUNT];

    const char         applicationName[__VK_MAX_NAME_LENGTH];
    uint32_t           applicationVersion;
    const char         engineName[__VK_MAX_NAME_LENGTH];
    uint32_t           engineVersion;
    uint32_t           apiVersion;

    VkAllocationCallbacks memCb;

    __vkDrvCtrlOption  drvOption;
    __vkChipInfo       chipInfo;

    struct __vkInstanceRec *pNext;
};

extern uint32_t  __vkEnableApiLog;

extern VkResult __vk_CreateObject(__vkDevContext *devCtx, __vkObjectIndex index, uint32_t size, __vkObject **pObject);
extern VkResult __vk_DestroyObject(__vkDevContext *devCtx, __vkObjectIndex index, __vkObject *obj);
extern VkResult __vk_DestroyObjectList(__vkDevContext *devCtx, __vkObjectIndex index);
extern PFN_vkVoidFunction __vk_GetApiProcAddr(const char *procName);
extern VkResult __vkInitilizeChipModule(__vkPhysicalDevice *phyDevice, __vkDevContext *devCtx);
extern VkResult __vk_SetSemaphore(VkDevice device, VkSemaphore semaphore, VkBool32 signaled);


#endif /* __gc_vk_context_h__ */


