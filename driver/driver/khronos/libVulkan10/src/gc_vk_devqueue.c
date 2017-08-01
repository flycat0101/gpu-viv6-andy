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


#include "gc_vk_precomp.h"

#if __VK_NEW_DEVICE_QUEUE

#define __VK_QUEUE_EVENT_TRUNK_SIZE             15
#define __VK_QUEUE_MINIMAL_CMDBUF_SIZE          (4 << 10)
#define __VK_QUEUE_MAX_PENDING_EVENT_COUNT      16


typedef struct _chunkHead
{
    struct _chunkHead *next;
}_chunkHead, *_chunkHead_PTR;

static void __vki_QueueCopyCMDBUF(
    gcoCMDBUF Dest,
    gcoCMDBUF Src
    )
{
    gctUINT commandBufferSize;

    commandBufferSize = Src->offset + Src->reservedTail - Src->startOffset;
    Dest->startOffset = Src->startOffset;
    Dest->offset    = Src->offset;

    gcoOS_MemCopy(
        gcmUINT64_TO_PTR(Dest->logical + Dest->startOffset),
        gcmUINT64_TO_PTR(Src->logical + Src->startOffset),
        commandBufferSize
        );

    return;
}


static VkResult __vki_QueueDestroyCMDBUF(
    __vkDevQueue *devQueue,
    gcoCMDBUF commandBuffer
    )
{
    gcsHAL_INTERFACE iface;
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;
    VkResult result = VK_SUCCESS;

    if (commandBuffer != gcvNULL)
    {
        if (gcmUINT64_TO_PTR(commandBuffer->logical) != gcvNULL)
        {
            switch (info->source)
            {
            case gcvCMDBUF_VIRTUAL:
                iface.command = gcvHAL_FREE_VIRTUAL_COMMAND_BUFFER;
                iface.u.FreeVirtualCommandBuffer.bytes    = commandBuffer->bytes;
                iface.u.FreeVirtualCommandBuffer.physical = commandBuffer->physical;
                iface.u.FreeVirtualCommandBuffer.logical  = commandBuffer->logical;
                __VK_ONERROR(__vk_DeviceControl(&iface, 0));
                break;

            case gcvCMDBUF_CONTIGUOUS:
                iface.command = gcvHAL_FREE_CONTIGUOUS_MEMORY;
                iface.u.FreeContiguousMemory.bytes    = commandBuffer->bytes;
                iface.u.FreeContiguousMemory.physical = commandBuffer->physical;
                iface.u.FreeContiguousMemory.logical  = commandBuffer->logical;
                __VK_ONERROR(__vk_DeviceControl(&iface, 0));
                break;

            case gcvCMDBUF_RESERVED:
                iface.engine = gcvENGINE_RENDER;
                iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
                iface.u.UnlockVideoMemory.node = commandBuffer->physical;
                iface.u.UnlockVideoMemory.type = gcvSURF_BITMAP;
                __VK_ONERROR(__vk_DeviceControl(&iface, 0));

                /* Release the allocated video memory synchronously. */
                iface.command = gcvHAL_RELEASE_VIDEO_MEMORY;
                iface.u.ReleaseVideoMemory.node = commandBuffer->physical;
                __VK_ONERROR(__vk_DeviceControl(&iface, 0));
                break;

                default:
                    break;
                }
            /* Reset the buffer pointer. */
            commandBuffer->logical = 0;

            if (commandBuffer->mirrorCount)
            {
                uint32_t i;
                for (i = 0; i < commandBuffer->mirrorCount; i++)
                {
                    __vki_QueueDestroyCMDBUF(devQueue, commandBuffer->mirrors[i]);
                }
            }
        }
        /* Destroy signal. */
        if (commandBuffer->signal != gcvNULL)
        {
            if (gcmIS_ERROR(gcoOS_DestroySignal(gcvNULL, commandBuffer->signal)))
            {
                __VK_ONERROR(VK_ERROR_DEVICE_LOST);
            }
            commandBuffer->signal = gcvNULL;
        }

        /* Free the gcoCMDBUF object. */
        gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, commandBuffer);
    }

    /* Success. */
    return VK_SUCCESS;

OnError:
    /* Return the status. */
    return result;
}

static gcoCMDBUF __vki_QueueCreateCMDBUF(
    __vkDevQueue *devQueue,
    uint32_t bytes,
    VkBool32 mirror
    )
{
    __vkDevContext *devCtx = devQueue->pDevContext;
    gcoCMDBUF commandBuffer = gcvNULL;
    gctSIZE_T objectSize    = 0;
    gctPOINTER pointer      = gcvNULL;
    gctUINT32 tmpSize       = 0;
    gctUINT32 node          = 0;
    gceSTATUS status;
    _CMDBUFinfo *info       = &devQueue->commandBufferInfo;
    VkResult result;
    __VK_SET_ALLOCATIONCB(&devCtx->memCb);
    /* Set the size of the object. */
    objectSize = gcmSIZEOF(struct _gcoCMDBUF);

    /* Allocate the gcoCMDBUF object. */
    /* Currently in most OS we are able to access the user-side data from
       the kernel by simple memory mapping, therefore here we allocate the
       object from the cached user memory. */
    gcmONERROR(gcoOS_AllocateSharedMemory(gcvNULL, objectSize, &pointer));
    commandBuffer = pointer;

    /* Reset the command buffer object. */
    gcoOS_ZeroMemory(commandBuffer, objectSize);

    commandBuffer->entryPipe = gcvPIPE_3D;
    commandBuffer->exitPipe  = gcvPIPE_3D;
    commandBuffer->using3D = gcvTRUE;

    /* Initialize the gcoCMDBUF object. */
    commandBuffer->object.type = gcvOBJ_COMMANDBUFFER;

    if (!mirror)
    {
        /* Create the signal. */
        gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvFALSE, &commandBuffer->signal));

        /* Mark the buffer as available. */
        gcmONERROR(gcoOS_Signal(gcvNULL, commandBuffer->signal, gcvTRUE));
    }

    /* Try to allocate the command buffer space. */
    while (gcvTRUE)
    {
        gcsHAL_INTERFACE iface;

        /* Set the desired size. */
        commandBuffer->bytes = bytes;

        /* Allocate the buffer for the command. */
        switch (info->source)
        {
        case gcvCMDBUF_VIRTUAL:
            iface.command = gcvHAL_ALLOCATE_VIRTUAL_COMMAND_BUFFER;
            iface.u.AllocateVirtualCommandBuffer.bytes = commandBuffer->bytes;

            /* Call kernel service. */
            result = __vk_DeviceControl(&iface, 0);

            if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
                goto retry;

            __VK_ONERROR(result);

            commandBuffer->bytes = (gctUINT32)iface.u.AllocateVirtualCommandBuffer.bytes;
            commandBuffer->physical = iface.u.AllocateVirtualCommandBuffer.physical;
            commandBuffer->logical = iface.u.AllocateVirtualCommandBuffer.logical;
            break;

        case gcvCMDBUF_CONTIGUOUS:
            /* Initialize the gcsHAL_INTERFACE structure. */
            iface.command = gcvHAL_ALLOCATE_CONTIGUOUS_MEMORY;
            iface.u.AllocateContiguousMemory.bytes = commandBuffer->bytes;

            /* Call kernel driver. */
            /* Call kernel service. */
            result = __vk_DeviceControl(&iface, 0);
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
                goto retry;

            __VK_ONERROR(result);


            commandBuffer->bytes = (gctUINT32)iface.u.AllocateContiguousMemory.bytes;
            commandBuffer->physical = iface.u.AllocateContiguousMemory.physical;
            commandBuffer->logical = iface.u.AllocateContiguousMemory.logical;
            break;

        case gcvCMDBUF_RESERVED:
            iface.command   = gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;

            iface.u.AllocateLinearVideoMemory.bytes = bytes;
            iface.u.AllocateLinearVideoMemory.alignment = info->alignmentSize;
            iface.u.AllocateLinearVideoMemory.type = gcvSURF_BITMAP;
            iface.u.AllocateLinearVideoMemory.pool = gcvPOOL_SYSTEM;
            iface.u.AllocateLinearVideoMemory.flag      = 0;

            /* Call kernel service. */
            result = __vk_DeviceControl(&iface, 0);

            if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
                goto retry;

            __VK_ONERROR(result);

            node  = iface.u.AllocateLinearVideoMemory.node;
            tmpSize = iface.u.AllocateLinearVideoMemory.bytes;

            iface.engine = gcvENGINE_RENDER;
            iface.command = gcvHAL_LOCK_VIDEO_MEMORY;
            iface.u.LockVideoMemory.node = node;
            iface.u.LockVideoMemory.cacheable = gcvFALSE;

            /* Call the kernel. */
            __VK_ONERROR(__vk_DeviceControl(&iface, 0));
            commandBuffer->physical = node;
            commandBuffer->bytes = tmpSize;
            commandBuffer->logical = iface.u.LockVideoMemory.memory;

            break;

        default:
            break;
        }

        /* Initialize command buffer. */
        commandBuffer->free = commandBuffer->bytes;
        /* The command buffer is successfully allocated. */
        break;

retry:
        if (bytes <= __VK_QUEUE_MINIMAL_CMDBUF_SIZE)
            goto OnError;

        /* Try lower size. */
        bytes >>= 1;
    }

    commandBuffer->reservedTail = info->reservedTailSize;

    if (info->mirrorCount && !mirror)
    {
        uint32_t i;
        commandBuffer->mirrorCount = info->mirrorCount;

        /* Allocate mirror of command buffer. */
        commandBuffer->mirrors = __VK_ALLOC(gcmSIZEOF(gcoCMDBUF *) * commandBuffer->mirrorCount,
            8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

        __VK_ONERROR(commandBuffer->mirrors ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(commandBuffer->mirrors, gcmSIZEOF(gcoCMDBUF *) * commandBuffer->mirrorCount);

        for (i = 0; i < commandBuffer->mirrorCount; i++)
        {
            commandBuffer->mirrors[i] = __vki_QueueCreateCMDBUF(devQueue, commandBuffer->bytes, VK_TRUE);
            if (!commandBuffer->mirrors[i])
            {
                __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
            }

            if (commandBuffer->bytes != commandBuffer->mirrors[i]->bytes)
            {
                __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
            }
        }
    }

    /* Return pointer to the gcoCMDBUF object. */
    return commandBuffer;

OnError:
    /* Roll back. */
    __VK_VERIFY_OK(__vki_QueueDestroyCMDBUF(devQueue, commandBuffer));
    return VK_NULL_HANDLE;
}


static VkResult __vki_QueueGetCMDBUF(
    __vkDevQueue *devQueue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoCMDBUF commandBuffer = gcvNULL;
    VkResult result = VK_SUCCESS;
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;

    /* Determine the next command buffer. */
    if (devQueue->commandBufferTail == gcvNULL)
    {
        /* Get the head of the list. */
        devQueue->commandBufferTail =
        commandBuffer               = devQueue->commandBufferList;
    }
    else
    {
        /* Get the next command buffer. */
        devQueue->commandBufferTail =
        commandBuffer               = devQueue->commandBufferTail->next;
    }

    if (commandBuffer != gcvNULL)
    {
        /* Test if command buffer is available. */
        status = gcoOS_WaitSignal(gcvNULL, commandBuffer->signal, 0);
    }

    /* Not available? */
    if ((status == gcvSTATUS_TIMEOUT) || (commandBuffer == gcvNULL))
    {
        /* Construct new command buffer. */
        if ((info->maxCount == 0)
        ||  (info->curCount < info->maxCount))
        {
            gcoCMDBUF temp = gcvNULL;
            temp = __vki_QueueCreateCMDBUF(devQueue, info->size, VK_FALSE);
            if (!temp)
            {
                __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
            }
            /* Insert into the list. */
            if (commandBuffer == gcvNULL)
            {
                devQueue->commandBufferList =
                temp->next = temp->prev = temp;
            }
            else
            {
                temp->prev = commandBuffer->prev;
                temp->next = commandBuffer;
                commandBuffer->prev->next = temp;
                commandBuffer->prev = temp;
            }

            devQueue->commandBufferTail = temp;

            info->curCount += 1;
            commandBuffer = temp;
        }

        /* Wait for buffer to become available. */
        gcmONERROR(gcoOS_WaitSignal(gcvNULL, commandBuffer->signal, gcvINFINITE));
    }
    else
    {
        gcmONERROR(status);
    }

    /* Reset command buffer. */
    commandBuffer->startOffset = 0;
    commandBuffer->offset      = devQueue->commandBufferInfo.reservedHeadSize;
    commandBuffer->free        = commandBuffer->bytes - devQueue->commandBufferInfo.totalReservedSize;

    return VK_SUCCESS;

OnError:
    if (gcmIS_ERROR(status))
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    return result;
}

static void __vki_QueueFinalize(
    __vkDevContext *devCtx,
    __vkDevQueue *devQueue
    )
{
    /*** Destroy all command buffers. ***/
    while (devQueue->commandBufferList != gcvNULL)
    {
        /* Get the head of the list. */
        gcoCMDBUF commandBuffer = devQueue->commandBufferList;

        /* Remove the head buffer from the list. */
        if (commandBuffer->next == commandBuffer)
        {
            devQueue->commandBufferList = gcvNULL;
            devQueue->commandBufferTail = gcvNULL;
        }
        else
        {
            commandBuffer->prev->next =
            devQueue->commandBufferList = commandBuffer->next;
            commandBuffer->next->prev = commandBuffer->prev;
        }

        /* Destroy command buffer. */
        __VK_VERIFY_OK(__vki_QueueDestroyCMDBUF(devQueue, commandBuffer));
    }

    /*** Event management ***/
    __VK_VERIFY_OK(__vk_QueueCommitEvents(devQueue, VK_TRUE));
    while (devQueue->eventChunks != gcvNULL)
    {
        _chunkHead_PTR p;
        /* Unlink the first chunk. */
        p = (_chunkHead_PTR)devQueue->eventChunks;
        devQueue->eventChunks = p->next;
        /* Free the memory. */
        gcmOS_SAFE_FREE_SHARED_MEMORY(gcvNULL, p);
    }
}

static VkResult __vki_QueueInitialize(
    __vkDevContext *devCtx,
    __vkDevQueue *devQueue
    )
{
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;
    VkResult result = VK_SUCCESS;
    uint32_t flushCacheSize = 0;

    /*** Event management ***/
    devQueue->eventHead = devQueue->eventTail = VK_NULL_HANDLE;
    devQueue->eventCount = 0;
    devQueue->eventChunks = VK_NULL_HANDLE;
    devQueue->eventFreeList = gcvNULL;

    /*** command buffer info ***/
    info->curCount = 0;
    info->maxCount = gcdMAX_CMD_BUFFERS;
    info->size = gcdCMD_BUFFER_SIZE;
    /* 64bit alignment */
    info->alignmentSize = 8;
    /* for SelectPipe() */
    info->reservedHeadSize = 32;
    /* set default */
    info->mirrorCount = 0;

    if (devQueue->queueFamilyIndex == __VK_PDEV_QUEUEFAMILY_GENERAL)
    {
        if ((devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        && (devCtx->database->ChipEnableLink))
        {
            /* For ChipEnable() */
            info->reservedTailSize = 8 * devCtx->chipInfo->gpuCoreCount;
            /* For Link() */
            info->reservedTailSize += 8 * devCtx->chipInfo->gpuCoreCount;
        }
        else
        {
            /* mirror command for combined MGPU */
            info->mirrorCount = (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
                              ? (devCtx->chipInfo->gpuCoreCount - 1) : 0;
            if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
            {
                /* For ChipEnable() */
                info->reservedTailSize = 8;
            }
            /* for Link() */
            info->reservedTailSize += 8;
        }

        /* starting from HALTI5 hardware, we uses fence to track commit stamp in KMD */
        if (devCtx->database->REG_Halti5)
        {
            info->reservedTailSize += gcdRENDER_FENCE_LENGTH;
        }
    }
    else if (devQueue->queueFamilyIndex == __VK_PDEV_QUEUEFAMILY_BLIT)
    {
        /* async blit queue always use fence to track commit stamp in KMD */
        info->reservedTailSize = gcdBLT_FENCE_LENGTH;
    }
    else
    {
        __VK_ASSERT(0);
    }

    /*
    ** Flush all cache as we don't use event to asynchronizedly unlock/free memory
    */
    __VK_ONERROR((*devCtx->chipFuncs->flushCache)((VkDevice)devCtx, VK_NULL_HANDLE, &flushCacheSize));
    info->reservedUserSize = flushCacheSize * sizeof(uint32_t);

    if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
    {
        uint32_t multiGPUSyncSize = 0;
        __VK_ONERROR((*devCtx->chipFuncs->setMultiGpuSync)((VkDevice)devCtx, VK_NULL_HANDLE, &multiGPUSyncSize));
        info->reservedUserSize += multiGPUSyncSize * sizeof(uint32_t);
    }

    info->totalReservedSize = info->alignmentSize
                            + info->reservedHeadSize
                            + info->reservedTailSize
                            + info->reservedUserSize;

#if gcdALLOC_CMD_FROM_RESERVE
    info->source = gcvCMDBUF_RESERVED;
#elif gcdSECURITY || gcdDISABLE_GPU_VIRTUAL_ADDRESS || !USE_KERNEL_VIRTUAL_BUFFERS
    info->source = gcvCMDBUF_CONTIGUOUS;
#else
    info->source = gcvCMDBUF_VIRTUAL;
#endif

OnError:
    return result;
}

VkResult __vk_QueueAppendEvent(
    __vkDevQueue *devQueue,
    gcsHAL_INTERFACE *Interface
    )
{
    VkResult result = VK_SUCCESS;
    gcsQUEUE_PTR record = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    _chunkHead_PTR p;
    uint32_t i;

    /* Check if we have records on the free list. */
    if (devQueue->eventFreeList == gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_AllocateSharedMemory(gcvNULL,
                gcmSIZEOF(*p) + gcmSIZEOF(*record) * __VK_QUEUE_EVENT_TRUNK_SIZE,
                &pointer)))
        {
            __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
        }

        p = (_chunkHead_PTR)pointer;

        /* Put it on the chunk list. */
        p->next  = (_chunkHead_PTR)devQueue->eventChunks;
        devQueue->eventChunks = p;

        /* Put the records on free list. */
        for (i = 0, record = (gcsQUEUE_PTR)(p + 1); i < __VK_QUEUE_EVENT_TRUNK_SIZE; i++, record++)
        {
            record->next = gcmPTR_TO_UINT64(devQueue->eventFreeList);
            devQueue->eventFreeList = record;
        }
    }

    /* Allocate from the free list. */
    record = devQueue->eventFreeList;
    devQueue->eventFreeList = (gcsQUEUE_PTR)gcmUINT64_TO_PTR(record->next);

    /* Initialize record. */
    record->next = 0;
    __VK_MEMCOPY(&record->iface, Interface, gcmSIZEOF(record->iface));

    if (devQueue->eventHead == gcvNULL)
    {
        /* Initialize queue. */
        devQueue->eventHead = record;
    }
    else
    {
        /* Append record to end of queue. */
        devQueue->eventTail->next = gcmPTR_TO_UINT64(record);
    }

    /* Mark end of queue. */
    devQueue->eventTail = record;

    /* update count */
    devQueue->eventCount++;

    if (devQueue->eventCount > __VK_QUEUE_MAX_PENDING_EVENT_COUNT)
    {
        __VK_ONERROR(__vk_QueueCommit(devQueue));
    }

    /* Success. */
    return VK_SUCCESS;

OnError:
    return result;
}

static void __vki_QueueFreeEvents(
    __vkDevQueue *devQueue
    )
{
    /* Free any records in the queue. */
    while (devQueue->eventHead != gcvNULL)
    {
        gcsQUEUE_PTR record;

        /* Unlink the first record from the queue. */
        record = devQueue->eventHead;
        devQueue->eventHead = (gcsQUEUE_PTR)gcmUINT64_TO_PTR(record->next);
        /* Put record on free list. */
        record->next = gcmPTR_TO_UINT64(devQueue->eventFreeList);
        devQueue->eventFreeList = record;
    }
    /* Update count */
    devQueue->eventCount = 0;
}

VkResult __vk_QueueCommit(
    __vkDevQueue *devQueue
    )
{
    __vkDevContext *devCtx = devQueue->pDevContext;
    gcoCMDBUF commandBuffer;
    gctBOOL emptyBuffer;
    VkResult result = VK_SUCCESS;
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;

    /* Grab the head command buffer. */
    commandBuffer = devQueue->commandBufferTail;

    if (commandBuffer == gcvNULL)
    {
        /* No current command buffer, of course empty buffer. */
        emptyBuffer = gcvTRUE;
    }
    else
    {
        /* Advance commit count. */
        commandBuffer->commitCount++;
        /* Determine whether the buffer is empty. */
        emptyBuffer = (commandBuffer->offset - commandBuffer->startOffset <= info->reservedHeadSize);
    }

    /* Send for execution. */
    if (emptyBuffer)
    {
        __VK_ONERROR(__vk_QueueCommitEvents(devQueue, VK_FALSE));
    }
    else
    {
        gcsHAL_INTERFACE iface;
        gctUINT32 newOffset;
        gctSIZE_T spaceLeft;
        gcoCMDBUF *commandBufferMirrors = commandBuffer->mirrors;
        uint32_t coreIdx;

        static const gceENGINE s_xlateQueueFamily[] =
        {
            /* __VK_PDEV_QUEUEFAMILY_GENERAL */
            gcvENGINE_RENDER,
            /* __VK_PDEV_QUEUEFAMILY_BLIT */
            gcvENGINE_BLT,
        };
        uint32_t  *flushCacheCommand;
        uint32_t alignedBytes, flushCacheSize;

        __VK_MEMZERO(&iface, sizeof(iface));
        alignedBytes = gcmALIGN(commandBuffer->offset, info->alignmentSize) - commandBuffer->offset;

        flushCacheCommand = (gctUINT32_PTR)((gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical)
                          + commandBuffer->offset + alignedBytes);
        __VK_ONERROR((*devCtx->chipFuncs->flushCache)((VkDevice)devCtx, &flushCacheCommand, &flushCacheSize));

        commandBuffer->offset += alignedBytes + flushCacheSize* sizeof(uint32_t);

        /* Insert mult-gpu sync for combined mode */
        if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        {
            uint32_t  *multiGpuCommand;
            uint32_t alignedBytes, multiGPUSyncSize;

            alignedBytes = gcmALIGN(commandBuffer->offset, info->alignmentSize) - commandBuffer->offset;

            multiGpuCommand = (gctUINT32_PTR)((gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical)
                            + commandBuffer->offset + alignedBytes);
            __VK_ONERROR((*devCtx->chipFuncs->setMultiGpuSync)((VkDevice)devCtx, &multiGpuCommand, &multiGPUSyncSize));
            commandBuffer->offset += alignedBytes + multiGPUSyncSize * sizeof(uint32_t);
        }

        /* Make sure the tail got aligned properly. */
        commandBuffer->offset = gcmALIGN(commandBuffer->offset, info->alignmentSize);

        /* The current pipe becomes the exit pipe for the current command buffer. */
        commandBuffer->exitPipe = gcvPIPE_3D;

        iface.u.Commit.commandBuffer = gcmPTR_TO_UINT64(iface.u.Commit.commandBuffers);
        iface.u.Commit.context = gcmPTR_TO_UINT64(iface.u.Commit.contexts);
        iface.u.Commit.delta = gcmPTR_TO_UINT64(iface.u.Commit.deltas);

        if (devCtx->option->affinityMode == __VK_MGPU_AFFINITY_COMBINE)
        {
            for (coreIdx = 0; coreIdx < devCtx->chipInfo->gpuCoreCount; coreIdx++)
            {
                iface.u.Commit.contexts[coreIdx] = devCtx->context[coreIdx];
                iface.command = gcvHAL_COMMIT;
                iface.engine = s_xlateQueueFamily[devQueue->queueFamilyIndex];
                iface.u.Commit.commandBuffers[coreIdx] = gcmPTR_TO_UINT64(commandBuffer);
                iface.u.Commit.deltas[coreIdx] = 0;
                iface.u.Commit.queue = gcmPTR_TO_UINT64(devQueue->eventHead);
                iface.u.Commit.shared = gcvTRUE;
                iface.u.Commit.index = coreIdx;

                if (commandBufferMirrors && (coreIdx > 0))
                {
                    __vki_QueueCopyCMDBUF(commandBufferMirrors[coreIdx - 1], commandBuffer);
                    iface.u.Commit.commandBuffers[coreIdx] = gcmPTR_TO_UINT64(commandBufferMirrors[coreIdx - 1]);
                    iface.u.Commit.index = 0;
                }
            }
            iface.u.Commit.count = devCtx->chipInfo->gpuCoreCount;
            __VK_ONERROR(__vk_DeviceControl(&iface, 0));

            /* UPdate commit stamp. */
            devQueue->commitStamp = iface.u.Commit.commitStamp;
        }
        else
        {
            coreIdx = devCtx->option->affinityCoreID;
            iface.u.Commit.contexts[0] = devCtx->context[coreIdx];
            iface.command = gcvHAL_COMMIT;
            iface.engine = s_xlateQueueFamily[devQueue->queueFamilyIndex];
            iface.u.Commit.commandBuffers[0] = gcmPTR_TO_UINT64(commandBuffer);
            iface.u.Commit.deltas[0] = 0;
            iface.u.Commit.queue = gcmPTR_TO_UINT64(devQueue->eventHead);
            iface.u.Commit.shared = gcvFALSE;
            iface.u.Commit.index = 0;
            iface.u.Commit.count = 1;

            /* Call kernel service. */
            __VK_ONERROR(__vk_DeviceControl(&iface, coreIdx));
            /* UPdate commit stamp. */
            devQueue->commitStamp = iface.u.Commit.commitStamp;
        }
#if gcdDUMP
        {
            uint8_t *dumpLogical;
            uint32_t dumpBytes;

            if (iface.u.Commit.contextSwitched)
            {
                /* Dump current context buffer. */
                gcmDUMP_BUFFER(gcvNULL,
                               "context",
                                devCtx->context[devCtx->option->affinityCoreID],
                                devCtx->contextLogical[devCtx->currentContext],
                                0,
                                devCtx->contextBytes);

                /* Advance to next context buffer. */
                devCtx->currentContext = (devCtx->currentContext + 1) % gcdCONTEXT_BUFFER_COUNT;
            }

            dumpLogical = (gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical)
                        + commandBuffer->startOffset
                        + info->reservedHeadSize;

            dumpBytes = commandBuffer->offset
                      - commandBuffer->startOffset
                      - info->reservedHeadSize;

            /* Dump commited command buffer. */
            gcmDUMP_BUFFER(gcvNULL,
                           "command",
                           devCtx->context[devCtx->option->affinityCoreID],
                           dumpLogical,
                           0,
                           dumpBytes);
        }
#endif

        /* Advance the offset for next commit. */
        newOffset = commandBuffer->offset + info->reservedTailSize;

        /* Compute the size of the space left in the buffer. */
        spaceLeft = commandBuffer->bytes - newOffset;

        if (spaceLeft > info->totalReservedSize)
        {
            /* Adjust buffer offset and size. */
            commandBuffer->startOffset = newOffset;
            commandBuffer->offset      = commandBuffer->startOffset + info->reservedHeadSize;
            commandBuffer->free        = commandBuffer->bytes - commandBuffer->offset
                                       - info->alignmentSize - info->reservedTailSize - info->reservedUserSize;
        }
        else
        {
            /* Buffer is full. */
            commandBuffer->startOffset = commandBuffer->bytes;
            commandBuffer->offset      = commandBuffer->bytes;
            commandBuffer->free        = 0;
        }
    }

    /* Empty buffer must be the tail. */
    __vki_QueueFreeEvents(devQueue);

    return VK_SUCCESS;

OnError:

    return result;
}



void * __vk_QueueGetSpace(
    __vkDevQueue *devQueue,
    uint32_t bytes
    )
{
    gcoCMDBUF commandBuffer;
    gctUINT32 alignBytes, alignedBytes;
    gctUINT32 offset;
    gcsHAL_INTERFACE iface;
    VkResult result;
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;

    __VK_ASSERT(devQueue->inProcessBytes == 0);

    /* Get the current command buffer. */
    commandBuffer = devQueue->commandBufferTail;

    if (commandBuffer == gcvNULL)
    {
        /* Grab a new command buffer. */
        __VK_ONERROR(__vki_QueueGetCMDBUF(devQueue));

        /* Get the new buffer. */
        commandBuffer = devQueue->commandBufferTail;
    }

    /* Compute the number of aligned bytes. */
    alignBytes = gcmALIGN(commandBuffer->offset, info->alignmentSize)- commandBuffer->offset;
    /* Compute the number of required bytes. */
    alignedBytes = bytes + alignBytes;
    devQueue->inProcessBytes = alignedBytes;

    if (alignedBytes > commandBuffer->free)
    {
        /* Sent event to signal when command buffer completes. */
        iface.command            = gcvHAL_SIGNAL;
        iface.u.Signal.signal    = gcmPTR_TO_UINT64(commandBuffer->signal);
        iface.u.Signal.auxSignal = 0;
        iface.u.Signal.process   = devQueue->pDevContext->threadId;
        iface.u.Signal.fromWhere = gcvKERNEL_COMMAND;

        __VK_ONERROR(__vk_QueueAppendEvent(devQueue, &iface));
        __VK_ONERROR(__vk_QueueCommit(devQueue));
        /* Grab a new command buffer. */
        __VK_ONERROR(__vki_QueueGetCMDBUF(devQueue));
        /* Get new buffer. */
        commandBuffer = devQueue->commandBufferTail;

        if (alignedBytes > commandBuffer->free)
        {
            /* This just won't fit! */
            __VK_PRINT("require space is too big to fit in one new CMDBUF\n");
            __VK_ONERROR(VK_ERROR_OUT_OF_HOST_MEMORY);
        }
    }
    /* Determine the data offset. */
    offset = commandBuffer->offset + alignBytes;

    /* Update the last reserved location. */
    commandBuffer->lastReserve = gcmPTR_TO_UINT64((gctUINT8_PTR) gcmUINT64_TO_PTR(commandBuffer->logical) + offset);
    commandBuffer->lastOffset  = offset;

    return(gcmUINT64_TO_PTR(commandBuffer->lastReserve));

OnError:
    return VK_NULL_HANDLE;
}

void __vk_QueueReleaseSpace(
    __vkDevQueue *devQueue,
    uint32_t bytes
    )
{
    gcoCMDBUF commandBuffer;
    gctUINT32 alignBytes, alignedBytes;
    _CMDBUFinfo *info = &devQueue->commandBufferInfo;

    /* Get the current command buffer. */
    commandBuffer = devQueue->commandBufferTail;
    __VK_ASSERT(commandBuffer);
    /* Compute the number of aligned bytes. */
    alignBytes = gcmALIGN(commandBuffer->offset, info->alignmentSize)- commandBuffer->offset;
    /* Compute the number of required bytes. */
    alignedBytes = bytes + alignBytes;

    __VK_ASSERT(devQueue->inProcessBytes >= alignedBytes);

    /* Adjust command buffer size. */
    commandBuffer->offset += alignedBytes;
    commandBuffer->free   -= alignedBytes;
    devQueue->inProcessBytes = 0;
    return;
}

VkResult __vk_QueueIdle(
    __vkDevQueue *devQueue
    )
{
    VkResult result = VK_SUCCESS;
        gcsHAL_INTERFACE iface;

    /* Dump the stall. */
    gcmDUMP(gcvNULL, "@[stall]");

    /* Create a signal event. */
    iface.command            = gcvHAL_SIGNAL;
    iface.u.Signal.signal    = gcmPTR_TO_UINT64(devQueue->signal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process   = devQueue->pDevContext->threadId;
    iface.u.Signal.fromWhere = devQueue->pDevContext->database->REG_BltEngine ? gcvKERNEL_BLT : gcvKERNEL_PIXEL;

    __VK_ONERROR(__vk_QueueAppendEvent(devQueue, &iface));

    __VK_ONERROR(__vk_QueueCommitEvents(devQueue, VK_FALSE));

    if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, devQueue->signal, gcvINFINITE)))
    {
        __VK_ONERROR(VK_ERROR_DEVICE_LOST);
    }

    return VK_SUCCESS;
OnError:
    return result;
}

VkResult __vk_QueueCommitEvents(
    __vkDevQueue *devQueue,
    VkBool32 idle
    )
{
    VkResult result = VK_SUCCESS;

    if (devQueue->eventHead != gcvNULL)
    {
        gcsHAL_INTERFACE iface;
        static const gceENGINE s_xlateQueueFamily[] =
        {
            /* __VK_PDEV_QUEUEFAMILY_GENERAL */
            gcvENGINE_RENDER,
            /* __VK_PDEV_QUEUEFAMILY_BLIT */
            gcvENGINE_BLT,
        };
        iface.command       = gcvHAL_EVENT_COMMIT;
        iface.u.Event.queue = gcmPTR_TO_UINT64(devQueue->eventHead);
        iface.engine = s_xlateQueueFamily[devQueue->queueFamilyIndex];
        /* For combine or native mode, affinityCoreID is always zero */
        __VK_ONERROR(__vk_DeviceControl(&iface, devQueue->pDevContext->option->affinityCoreID));
        /* Free any records in the queue. */
        __vki_QueueFreeEvents(devQueue);

        /* Wait for the execution to complete. */
        if (idle)
        {
            __VK_ONERROR(__vk_QueueIdle(devQueue));
        }
    }

    return VK_SUCCESS;

OnError:

    return result;
}

#endif


VkResult __vk_CreateDeviceQueues(
    __vkDevContext *devCtx,
    uint32_t queueCreateInfoCount,
    const VkDeviceQueueCreateInfo* pQueueCreateInfos
    )
{
    __VK_DEBUG_ONLY(__vkPhysicalDevice *phyDev = devCtx->pPhyDevice;)
    VkResult result = VK_SUCCESS;
    uint32_t i, j;

    __VK_SET_ALLOCATIONCB(&devCtx->memCb);
    /* Initialize devQueue structures */
    devCtx->devQueueCreateCount = queueCreateInfoCount;

    for (i = 0; i < devCtx->devQueueCreateCount; i++)
    {
        uint32_t queueFamilyIndex = pQueueCreateInfos[i].queueFamilyIndex;
        uint32_t queueCount = pQueueCreateInfos[i].queueCount;

        __VK_ASSERT(queueFamilyIndex <= phyDev->queueFamilyCount);
        __VK_ASSERT(queueCount <= phyDev->queueProp[queueFamilyIndex].queueCount);

        devCtx->devQueues[queueFamilyIndex] = (__vkDevQueue *)__VK_ALLOC(
            sizeof(__vkDevQueue) * queueCount, 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

        devCtx->queueCount[i] = queueCount;
        __VK_ONERROR(devCtx->devQueues[queueFamilyIndex] ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMZERO(devCtx->devQueues[queueFamilyIndex], (sizeof(__vkDevQueue) * queueCount));

        for (j = 0; j < queueCount; j++)
        {
            __vkDevQueue *devQueue = &devCtx->devQueues[queueFamilyIndex][j];
            set_loader_magic_value(devQueue);
            devQueue->sType = __VK_OBJECT_TYPE_CMD_QUEUE;
            devQueue->pDevContext = devCtx;
            devQueue->flags = pQueueCreateInfos[i].flags;
            devQueue->queueFamilyIndex = queueFamilyIndex;
            devQueue->queuePriority = (float)pQueueCreateInfos[i].pQueuePriorities[j];
#if __VK_NEW_DEVICE_QUEUE
            __VK_ONERROR(__vki_QueueInitialize(devCtx, devQueue));
#endif
            /* Signal for queue idle */
            __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_FALSE, &devQueue->signal));
        }
    }
    return VK_SUCCESS;

OnError:
    __vk_DestroyDeviceQueues(devCtx);
    return result;
}


void __vk_DestroyDeviceQueues(
    __vkDevContext *devCtx
    )
{
    __vkPhysicalDevice *phyDev = devCtx->pPhyDevice;
    uint32_t i, j;

    __VK_SET_ALLOCATIONCB(&devCtx->memCb);

    for (i = 0; i < phyDev->queueFamilyCount; i++)
    {
        if (devCtx->devQueues[i])
        {
            for (j = 0; j < devCtx->queueCount[i]; j++)
            {
                __vkDevQueue *devQueue = &devCtx->devQueues[i][j];

                if (devQueue)
                {
#if __VK_NEW_DEVICE_QUEUE
                    __vki_QueueFinalize(devCtx, devQueue);
#endif
                    if (devQueue->signal)
                    {
                        gcoOS_DestroySignal(gcvNULL, devQueue->signal);
                    }
                }
            }

            __VK_FREE(devCtx->devQueues[i]);
            devCtx->devQueues[i] = VK_NULL_HANDLE;
            devCtx->queueCount[i] = 0;
        }
    }

    return;
}


