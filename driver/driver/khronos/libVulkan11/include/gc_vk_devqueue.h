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


#ifndef __gc_vk_devqueue_h__
#define __gc_vk_devqueue_h__


typedef struct _CMDBUFinfo
{
    /* all sizes are in byte */
    uint32_t alignmentSize;
    uint32_t reservedHeadSize;
    uint32_t reservedTailSize;
    uint32_t reservedUserSize;
    uint32_t totalReservedSize;
    uint32_t mirrorCount;
    uint32_t curCount;
    uint32_t maxCount;
    uint32_t size;
} _CMDBUFinfo;

struct __vkDevQueueRec
{
    /*
     * require 'hwvulkan_dispatch_t' on android-n,
     * which is same as 'VK_LOADER_DATA' currently.
     */
    VK_LOADER_DATA loaderInfo;
    __vkObjectType sType;

    __vkDevContext *pDevContext;
    VkDeviceQueueCreateFlags flags;
    uint32_t queueFamilyIndex;
    float queuePriority;

    /*** Event ***/
    /* Pointer to current event queue. */
    gcsQUEUE_PTR                eventHead;
    gcsQUEUE_PTR                eventTail;
    gctPOINTER                  eventChunks;
    gcsQUEUE_PTR                eventFreeList;
    uint32_t                    eventCount;

    /*** CMDBUF ***/
    /* List of command buffers. */
    _CMDBUFinfo                 commandBufferInfo;
    gcoCMDBUF                   commandBufferList;
    gcoCMDBUF                   commandBufferTail;
    uint64_t                    commitStamp;
    uint32_t                    inProcessBytes;
    gctSIGNAL signal;
    VkBool32                    commitMutex;
    /*mark the last QueueSubmit whether have command buffer commit*/
    VkBool32                    commitFlag;
};

VkResult __vk_CreateDeviceQueues(
    __vkDevContext *devCtx,
    uint32_t queueCreateInfoCount,
    const VkDeviceQueueCreateInfo* pQueueCreateInfos
    );

void __vk_DestroyDeviceQueues(
    __vkDevContext *devCtx
    );


VkResult __vk_QueueCommitEvents(
    __vkDevQueue *devQueue,
    VkBool32 idle
    );

VkResult __vk_QueueIdle(
    __vkDevQueue *devQueue
    );

void * __vk_QueueGetSpace(
    __vkDevQueue *devQueue,
    uint32_t bytes
    );

void __vk_QueueReleaseSpace(
    __vkDevQueue *devQueue,
    uint32_t bytes
    );

VkResult __vk_QueueCommit(
    __vkDevQueue *devQueue
    );

VkResult __vk_QueueAppendEvent(
    __vkDevQueue *devQueue,
    gcsHAL_INTERFACE *Interface
    );


#endif


