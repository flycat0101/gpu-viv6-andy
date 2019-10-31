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
#define __VK_MAX_PDGRO_COUNT            1
#define __VK_MAX_GPU_CORE_COUNT         4
#define __VK_MAX_FENCE_COUNT            32768
#define __VK_MAX_QUERY_BUF_SIZE         4096
#define __VK_MAX_NAME_LENGTH            256

enum
{
    __VK_PDEV_QUEUEFAMILY_GENERAL      = 0,
    __VK_PDEV_QUEUEFAMILY_BLIT,
    __VK_PDEV_QUEUEFAMILY_MAX,
};

#define __VK_LOG_API(...)  if (__vkEnableApiLog) __VK_PRINT(__VA_ARGS__)

typedef enum
{
    __VK_EXTID_KHR_16BIT_STORAGE,
    __VK_EXTID_KHR_BIND_MEMORY_2,
    __VK_EXTID_KHR_DESCRIPTOR_UPDATE_TEMPLATE,
    __VK_EXTID_KHR_DEVICE_GROUP,
    __VK_EXTID_KHR_EXTERNAL_MEMORY,
    __VK_EXTID_KHR_GET_MEMORY_REQUIREMENTS_2,
    __VK_EXTID_KHR_MAINTENANCE1,
    __VK_EXTID_KHR_MAINTENANCE2,
    __VK_EXTID_KHR_MAINTENANCE3,
    __VK_EXTID_KHR_VARIABLE_POINTERS,
#if defined(VK_USE_PLATFORM_ANDROID_KHR) && (ANDROID_SDK_VERSION >= 24)
    __VK_EXTID_ANDROID_NATIVE_BUFFER,
#else
    __VK_EXTID_KHR_SWAPCHAIN,
#endif
    __VK_EXTID_EXT_LAST,
}
__vkExtID;

typedef struct __vkExtensionRec
{
    const char *  name;           /* the extension name */
    VkBool32      bEnabled;       /* is this extension enabled ? */
} __vkExtension;

typedef struct __vkExtProcAliasRec
{
    __vkExtID       index;          /* the extension index to which the function belongs to */
    const char *    procName;       /* extension function name */
} __vkExtProcAlias;

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

    gctPOINTER fenceMutex;
    uint32_t fenceInUse[__VK_MAX_FENCE_COUNT / 32];
    uint32_t lastFenceIndex;
    uint32_t fenceCount;
    VkBuffer fenceBuffer;

    /*record the signal related with Native fence*/
    void *fdSignal[8];
    uint32_t fdCount;

    /* All VK objects created through vkCreate*() API with this devContext. */
    __vkObjectList vkObject[__VK_DEV_OBJECT_COUNT];

    /* Current error status */
    VkResult currentResult;

    uint32_t context[__VK_MAX_GPU_CORE_COUNT];

#if gcdDUMP
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
    VkPhysicalDeviceFeatures enabledFeatures;

    __vkChipFuncTable *chipFuncs;
    void *chipPriv;
    struct __vkDevContextRec *pNext;
    VkBool32  msaa_64bpp;

    /* Store extension enable state */
    __vkExtension *pEnabledExtensions;
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
    gcsHAL_QUERY_CHIP_OPTIONS options;
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

    VkPhysicalDeviceFeatures phyDevFeatures;
    VkPhysicalDeviceProperties phyDevProp;
    VkPhysicalDeviceMemoryProperties phyDevMemProp;

    VkPhysicalDeviceIDProperties phyDevIDProp;
    VkPhysicalDeviceMaintenance3Properties phyDevMain3Prop;
    VkPhysicalDeviceMultiviewProperties phyDevMultiViewProp;
    VkPhysicalDevicePointClippingProperties phyDevPointClipProp;
    VkPhysicalDeviceProtectedMemoryProperties phyDevproMemProp;
    VkPhysicalDeviceSubgroupProperties phyDevSubProp;

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

    uint32_t physicalDeviceGroupCount;
    VkPhysicalDeviceGroupProperties phyDevGroProperties[__VK_MAX_PDGRO_COUNT];

    const char         applicationName[__VK_MAX_NAME_LENGTH];
    uint32_t           applicationVersion;
    const char         engineName[__VK_MAX_NAME_LENGTH];
    uint32_t           engineVersion;
    uint32_t           apiVersion;

    VkAllocationCallbacks memCb;

    __vkDrvCtrlOption  drvOption;
    __vkChipInfo       chipInfo;
    gcePATCH_ID        patchID;

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


