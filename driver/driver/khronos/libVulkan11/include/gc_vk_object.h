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


#ifndef __gc_vk_object_h__
#define __gc_vk_object_h__

typedef uint32_t __vkObjectType;

typedef enum
{
    __VK_OBJECT_FENCE = 0,
    __VK_OBJECT_SEMAPHORE,
    __VK_OBJECT_EVENT,
    __VK_OBJECT_QUERY_POOL,
    __VK_OBJECT_BUFFER,
    __VK_OBJECT_BUFFER_VIEW,
    __VK_OBJECT_IMAGE,
    __VK_OBJECT_IMAGE_VIEW,
    __VK_OBJECT_SHADER_MODULE,
    __VK_OBJECT_PIPELINE_CACHE,
    __VK_OBJECT_PIPELINE,
    __VK_OBJECT_PIPELINE_LAYOUT,
    __VK_OBJECT_SAMPLER,
    __VK_OBJECT_DESCRIPTORSET,
    __VK_OBJECT_DESCRIPTORSET_LAYOUT,
    __VK_OBJECT_DESCRIPTOR_POOL,
    __VK_OBJECT_DESCRIPTOR_UPDATE_TEMPLATE,
    __VK_OBJECT_FRAMEBUFFER,
    __VK_OBJECT_RENDER_PASS,
    __VK_OBJECT_COMMAND_POOL,
    __VK_OBJECT_COMMAND_BUFFER,
    __VK_OBJECT_DEVICE_MEMORY,
    __VK_OBJECT_SWAPCHAIN_KHR,
    __VK_OBJECT_SURFACE_KHR,
    __VK_OBJECT_YCBCR_CONVERSION,
    __VK_DEV_OBJECT_COUNT

} __vkObjectIndex;


#define  __VK_OBJECT_TYPE_BASE              0xabcd
#define  __VK_OBJECT_INDEX_TO_TYPE(index)   (__VK_OBJECT_TYPE_BASE + index)

#define  __VK_OBJECT_TYPE_INSTANCE          (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 1)
#define  __VK_OBJECT_TYPE_PHYSICAL_DEVICE   (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 2)
#define  __VK_OBJECT_TYPE_DEV_CONTEXT       (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 3)
#define  __VK_OBJECT_TYPE_CMD_QUEUE         (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 4)
#define  __VK_OBJECT_TYPE_DISPLAY_KHR       (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 5)
#define  __VK_OBJECT_TYPE_DISPLAY_MODE_KHR  (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 6)
#define  __VK_OBJECT_TYPE_DEBUG_CB_EXT      (__VK_OBJECT_TYPE_BASE + __VK_DEV_OBJECT_COUNT + 7)


typedef struct __vkObjectRec
{
    /*
     * require 'hwvulkan_dispatch_t' on android-n for dispatchable handles,
     * 'hwvulkan_dispatch_t' is same as 'VK_LOADER_DATA' currently.
     */
    VK_LOADER_DATA loaderInfo;
    __vkObjectType sType;
    __vkDevContext *pDevContext;
    int32_t id;
    struct __vkObjectRec *pNext;
} __vkObject;

typedef VkResult (*PFN_ObjectStatFunc)(__vkDevContext *devCtx, __vkObject *obj, int32_t flag);

typedef struct __vkObjectListRec
{
    __vkObject *objList;
    gctPOINTER  objMutex;
    PFN_ObjectStatFunc objectStatFunc;
    VkBool32 mutexClosed;
} __vkObjectList;

typedef struct __vkFenceRec
{
    __vkObject obj; /* Must be the first field */

    /* Fence specific fields */
    gctSIGNAL signal;

    /*support fence_win32 extention */
    void *winHandle;

    VkExternalFenceHandleTypeFlagBits handleType;

    /*support fence_fd extension*/
    int fenceFd;

    VkBool32 exported;
    VkBool32 imported;

    gctSIGNAL permanent;

} __vkFence;

typedef struct __vkSemaphoreRec
{
    __vkObject obj; /* Must be the first field */

    /* Semaphore specific fields */
    uint32_t fenceIndex;

    /*support semaphore_win32 extention */
    void *winHandle;

    VkExternalSemaphoreHandleTypeFlagBits handleType;

    /*support semaphore_fd extension*/
    int fenceFd;

    int32_t signalIndex;

} __vkSemaphore;

typedef struct __vkEventRec
{
    __vkObject obj; /* Must be the first field */

    /* Event specific fields */
    uint32_t fenceIndex;

} __vkEvent;

VkResult __vk_InitObjectLists(VkDevice device);
VkResult __vk_FiniObjectLists(__vkDevContext *devCtx);
VkResult __vk_InsertObject(__vkDevContext *devCtx, __vkObjectIndex index, __vkObject *obj);
VkResult __vk_RemoveObject(__vkDevContext *devCtx, __vkObjectIndex index, __vkObject *obj);
VkBool32 __vk_SearchObject(__vkDevContext *devCtx, __vkObjectIndex index,__vkObject *obj);


#endif /* __gc_vk_object_h__ */


