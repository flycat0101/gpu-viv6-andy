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


uint8_t* __vk_AllocateStateBuffer(
    VkCommandPool commandPool
    )
{
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    uint8_t  *retBuf = gcvNULL;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    retBuf = (uint8_t*)__VK_ALLOC(cdp->sizeOfEachStateBuffer, 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (retBuf)
        cdp->numOfStateBuffers++;

    return retBuf;
}

void __vk_FreeStateBuffer(
    VkCommandPool commandPool,
    void* buf
    )
{
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);

    if (buf)
    {
        __VK_SET_ALLOCATIONCB(&cdp->memCb);

        __VK_FREE(buf);

        cdp->numOfStateBuffers--;
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateCommandPool(
    VkDevice device,
    const VkCommandPoolCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkCommandPool* pCommandPool
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp;
    VkResult result = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    *pCommandPool  = VK_NULL_HANDLE;

    result = __vk_CreateObject(devCtx, __VK_OBJECT_COMMAND_POOL, sizeof(__vkCommandPool), (__vkObject**)&cdp);
    if (result == VK_SUCCESS)
    {
        cdp->threadId = (uint32_t)gcmPTR2INT(gcoOS_GetCurrentThreadID());
        cdp->queueFamilyIndex = pCreateInfo->queueFamilyIndex;
        cdp->memCb = __VK_ALLOCATIONCB;
        cdp->flags = pCreateInfo->flags;
        cdp->numOfCmdBuffers = 0;
        cdp->vkCmdBufferList = gcvNULL;
        cdp->numOfStateBuffers = 0;
        cdp->sizeOfEachStateBuffer = __VK_STATEBUFFER_SIZE;

        /* Return the object pointer as a 64-bit handle */
        *pCommandPool = (VkCommandPool) (uintptr_t) cdp;
    }
    else
    {
        *pCommandPool = VK_NULL_HANDLE;
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyCommandPool(
    VkDevice device,
    VkCommandPool commandPool,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);

    /* Free any remaining command buffers that the app forgot */
    while (cdp->vkCmdBufferList)
    {
        __vk_FreeCommandBuffers(device, commandPool, 1, &cdp->vkCmdBufferList);
    }

    __vk_DestroyObject(devCtx, __VK_OBJECT_COMMAND_POOL, (__vkObject *)cdp);
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetCommandPool(
    VkDevice device,
    VkCommandPool commandPool,
    VkCommandPoolResetFlags flags
    )
{
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)cdp->vkCmdBufferList;

    /* Loop through the command pool's command buffers */
    while (cmd)
    {
        __vk_ResetCommandBuffer((VkCommandBuffer)cmd, flags);
        cmd = cmd->next;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_AllocateCommandBuffers(
    VkDevice device,
    const VkCommandBufferAllocateInfo* pAllocateInfo,
    VkCommandBuffer* pCommandBuffers
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, pAllocateInfo->commandPool);
    __vkCommandBuffer *cmd;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    __VK_MEMZERO(pCommandBuffers, (pAllocateInfo->commandBufferCount * sizeof(VkCommandBuffer)));

    for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
    {
        result = __vk_CreateObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, sizeof(__vkCommandBuffer), (__vkObject**)&cmd);
        if (result != VK_SUCCESS)
        {
            break;
        }

        /* Initialize __vkCommandBuffer specific data fields here */
        cmd->commandPool = pAllocateInfo->commandPool;
        cmd->devCtx = devCtx;
        cmd->bufWriteRequestCount = 0;
        cmd->level = pAllocateInfo->level;
        cmd->state = __VK_CMDBUF_STATE_FREE;
        cmd->result = VK_SUCCESS;

        cmd->stateBufferList = cmd->stateBufferTail = (__vkStateBuffer *)__VK_ALLOC(
            sizeof(__vkStateBuffer), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
        if (!cmd->stateBufferList)
        {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;

            __vk_DestroyObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject *)cmd);
            break;
        }
        cmd->stateBufferList->next = gcvNULL;
        cmd->stateBufferList->bufPipe = (uint32_t)gcvPIPE_INVALID;
        cmd->stateBufferList->bufStart = gcvNULL;
        cmd->stateBufferList->bufSize = 0;
        cmd->stateBufferList->bufOffset = 0;
        cmd->stateBufferList->secBufCount = 0;
        cmd->lastStateBufferIndex = 0;
        cmd->sequenceID = 0;
        cmd->gpuRenderingMode = gcvMULTI_GPU_RENDERING_MODE_INVALID;

        cmd->curScrachBufIndex = 0;
        cmd->bindInfo.pipeline.activePipeline = VK_PIPELINE_BIND_POINT_MAX_ENUM;

        if (!__VK_IS_SUCCESS((*devCtx->chipFuncs->AllocateCommandBuffer)(device, (VkCommandBuffer)(uintptr_t)cmd)))
        {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;

            __VK_FREE(cmd->stateBufferList);
            __vk_DestroyObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject *)cmd);
            break;
        }
        /* Initialize secondary command buffer list */
        cmd->executeList = cmd->executeTail = gcvNULL;

        cmd->scratchHead = gcvNULL;

        cmd->next = gcvNULL;

        cdp->numOfCmdBuffers++;

        /* Return the object pointer as a 64-bit handle */
        pCommandBuffers[i] = (VkCommandBuffer)cmd;

        if (cdp->vkCmdBufferList == NULL)
        {
            cdp->vkCmdBufferList = pCommandBuffers[i];
        }
        else
        {
            __vkCommandBuffer *tmp = (__vkCommandBuffer *)cdp->vkCmdBufferList;

            while (tmp->next)
            {
                tmp = tmp->next;
            }
            tmp->next = cmd;
        }
    }

    if (result != VK_SUCCESS)
    {
        for (i = 0; i < pAllocateInfo->commandBufferCount; i++)
        {
            if (pCommandBuffers[i])
            {
                cmd = (__vkCommandBuffer *)pCommandBuffers[i];

                if (cmd->stateBufferList)
                {
                    __VK_FREE(cmd->stateBufferList);
                }

                (*devCtx->chipFuncs->FreeCommandBuffer)(device, (VkCommandBuffer)(uintptr_t)cmd);

                if (pCommandBuffers[i] == cdp->vkCmdBufferList)
                    cdp->vkCmdBufferList = VK_NULL_HANDLE;

                __vk_DestroyObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject *)cmd);

                cdp->numOfCmdBuffers--;

                pCommandBuffers[i] = gcvNULL;
            }
            else
            {
                break;
            }
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_FreeCommandBuffers(
    VkDevice device,
    VkCommandPool commandPool,
    uint32_t commandBufferCount,
    const VkCommandBuffer* pCommandBuffers
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, commandPool);
    __vkCommandBuffer *cmd;
    uint32_t i;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    for (i = 0; i < commandBufferCount; i++)
    {
        cmd = (__vkCommandBuffer *)pCommandBuffers[i];

        if (commandPool == cmd->commandPool)
        {
            __vkScratchMem *pScratch = cmd->scratchHead;

            if (pCommandBuffers[i] == cdp->vkCmdBufferList)
            {
                cdp->vkCmdBufferList = (VkCommandBuffer)cmd->next;
            }
            else
            {
                __vkCommandBuffer *tmp = (__vkCommandBuffer *)cdp->vkCmdBufferList;

                while (cmd != tmp->next)
                {
                    tmp = tmp->next;

                    if (tmp == NULL)
                        __VK_ASSERT(0);
                }
                tmp->next = cmd->next;
            }

            /* Free all the scratch memory used in this command buffer */
            while (pScratch)
            {
                __vkScratchMem *pCurrent = pScratch;
                pScratch = pScratch->next;

                __vk_FreeMemory(device, (VkDeviceMemory)(uintptr_t)pCurrent->memory, VK_NULL_HANDLE);

                __VK_FREE(pCurrent);
            }
            cmd->scratchHead = gcvNULL;

#if __VK_RESOURCE_INFO
            __vk_utils_freeCmdRes(cmd);
#endif
            /* Free any secondary execute info */
            while (cmd->executeList)
            {
                __vkCmdExecuteCommandsInfo *temp = cmd->executeList;

                cmd->executeList = cmd->executeList->next;
                __VK_FREE(temp);
            }
            cmd->executeTail = cmd->executeList;

            /* Free the state buffers */
            while (cmd->stateBufferList)
            {
                __vkStateBuffer *temp = cmd->stateBufferList;

                cmd->stateBufferList = cmd->stateBufferList->next;
                __vk_FreeStateBuffer(cmd->commandPool, temp->bufStart);
                __VK_FREE(temp);
            }
            cmd->stateBufferTail = cmd->stateBufferList;

            (*devCtx->chipFuncs->FreeCommandBuffer)(device, (VkCommandBuffer)(uintptr_t)cmd);

            __VK_VERIFY_OK(__vk_DestroyObject(devCtx, __VK_OBJECT_COMMAND_BUFFER, (__vkObject *)cmd));

            cdp->numOfCmdBuffers--;
        }
        else
        {
            __VK_ASSERT(0);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetCommandBuffer(
    VkCommandBuffer commandBuffer,
    VkCommandBufferResetFlags flags
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkStateBuffer *state = cmd->stateBufferList;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmd->commandPool);
    __vkScratchMem *pScratch = cmd->scratchHead;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    /* Have to reset the command pool if pool was created without VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT */
    if (!(cdp->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT))
        return VK_INCOMPLETE;

    cmd->state = __VK_CMDBUF_STATE_FREE;

    /* Reset all state buffers (Should we free all but one always???) */
    while (state)
    {
        __vkStateBuffer *temp = state;

        /* If VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT is set free all but the first state buffer */
        if (flags & VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT)
        {
            if (state != cmd->stateBufferList)
            {
                 state = state->next;
                __vk_FreeStateBuffer(cmd->commandPool, temp->bufStart);
                __VK_FREE(temp);
            }
            else
            {
                state->bufOffset = 0;
                state->bufPipe = (uint32_t)gcvPIPE_INVALID;
                state->secBufCount = 0;
                state = state->next;
                cmd->stateBufferList->next = gcvNULL;
            }
        }
        else
        {
            state->bufOffset = 0;
            state->bufPipe = (uint32_t)gcvPIPE_INVALID;
            state->secBufCount = 0;
            state = state->next;
            temp->next = gcvNULL;
        }
    }
    cmd->stateBufferTail = cmd->stateBufferList;
    cmd->lastStateBufferIndex = 0;
    cmd->curScrachBufIndex = 0;
    cmd->bindInfo.pipeline.graphics = 0;
    cmd->bindInfo.pipeline.compute = 0;
    cmd->bindInfo.pipeline.activePipeline = VK_PIPELINE_BIND_POINT_MAX_ENUM;
    cmd->sequenceID = 0;
    cmd->gpuRenderingMode = gcvMULTI_GPU_RENDERING_MODE_INVALID;

    /* Free all the scratch memory used in this command buffer */
    while (pScratch)
    {
        __vkScratchMem *pCurrent = pScratch;
        pScratch = pScratch->next;

        __vk_FreeMemory((VkDevice)cmd->devCtx, (VkDeviceMemory)(uintptr_t)pCurrent->memory, VK_NULL_HANDLE);

        __VK_FREE(pCurrent);
    }
    cmd->scratchHead = gcvNULL;

    /* Free any secondary execute info */
    while (cmd->executeList)
    {
        __vkCmdExecuteCommandsInfo *temp = cmd->executeList;

        cmd->executeList = cmd->executeList->next;
        __VK_FREE(temp);
    }
    cmd->executeTail = cmd->executeList;

#if __VK_RESOURCE_INFO
    __vk_utils_freeCmdRes(cmd);
#endif
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_BeginCommandBuffer(
    VkCommandBuffer commandBuffer,
    const VkCommandBufferBeginInfo* pBeginInfo
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmd->commandPool);
    __vkDevContext *devCtx = cmd->devCtx;

#if !__VK_NEW_DEVICE_QUEUE
    VkResult result = VK_SUCCESS;

    cmd->threadId = (uint32_t)gcmPTR2INT(gcoOS_GetCurrentThreadID());;

    /* There can be different threads within a device context so set the current HW type */
    if (cmd->threadId != devCtx->threadId)
    {
        __VK_ONERROR(gcoHAL_Construct(gcvNULL, gcvNULL, &cmd->hal));
        __VK_ONERROR(gcoHAL_SetHardwareType(cmd->hal, gcvHARDWARE_3D));
    }
#endif

    /* Reset the command buffer if command pool was created with __VK_CMDBUF_STATE_FREE */
    if (((cmd->state == __VK_CMDBUF_STATE_EXECUTABLE) || (cmd->state == __VK_CMDBUF_STATE_RESET_REQUIRED)) &&
        (cdp->flags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT))
    {
        __vk_ResetCommandBuffer(commandBuffer, 0);
    }

    cmd->state = __VK_CMDBUF_STATE_RECORDING;

    cmd->usage = pBeginInfo->flags;
    cmd->stateBufferTail->bufPipe = (uint32_t)gcvPIPE_INVALID;

    if (cmd->level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        __vkRenderPass *rdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, pBeginInfo->pInheritanceInfo->renderPass);
        cmd->bindInfo.renderPass.rdp = rdp;
        if (rdp)
        {
            cmd->bindInfo.renderPass.subPass = &rdp->subPassInfo[pBeginInfo->pInheritanceInfo->subpass];
        }

        if ((pBeginInfo->pInheritanceInfo->framebuffer == VK_NULL_HANDLE) && (rdp != VK_NULL_HANDLE))
            cmd->bindInfo.renderPass.fb = rdp->fbDefault;
        else
            cmd->bindInfo.renderPass.fb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFramebuffer *, pBeginInfo->pInheritanceInfo->framebuffer);

        cmd->bindInfo.renderPass.subPassContent = VK_SUBPASS_CONTENTS_INLINE;
        cmd->bindInfo.renderPass.dirty = VK_TRUE;
    }

    return (*devCtx->chipFuncs->BeginCommandBuffer)(commandBuffer);

#if !__VK_NEW_DEVICE_QUEUE
OnError:

    return result;
#endif
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_EndCommandBuffer(
    VkCommandBuffer commandBuffer
    )
{
    VkResult ret = VK_SUCCESS;
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkDevContext *devCtx = cmd->devCtx;

    ret = (*devCtx->chipFuncs->EndCommandBuffer)(commandBuffer);
    cmd->state = __VK_CMDBUF_STATE_EXECUTABLE;

    return ret;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdExecuteCommands(
    VkCommandBuffer commandBuffer,
    uint32_t commandBuffersCount,
    const VkCommandBuffer* pCmdBuffers
    )
{
    __vkCommandBuffer *primary = (__vkCommandBuffer *)commandBuffer;
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, primary->commandPool);
    uint32_t i;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    primary = (__vkCommandBuffer *)commandBuffer;

    for (i = 0; i < commandBuffersCount; i++)
    {
        __vkCommandBuffer *secondary;
        __vkCmdExecuteCommandsInfo *info = gcvNULL;

        secondary = (__vkCommandBuffer *)pCmdBuffers[i];

        /* Previous primary must be reset before next submission */
        if (secondary->secondaryInfo.prevPrimarySubmission != commandBuffer)
        {
            __vkCommandBuffer *previous = (__vkCommandBuffer *)secondary->secondaryInfo.prevPrimarySubmission;
            if (previous)
                previous->state = __VK_CMDBUF_STATE_RESET_REQUIRED;
        }

        /* Set secondary state */
        secondary->state = __VK_CMDBUF_STATE_EXECUTION_PENDING;
        secondary->secondaryInfo.prevPrimarySubmission = commandBuffer;

        info = (__vkCmdExecuteCommandsInfo *)__VK_ALLOC(
            sizeof(__vkCmdExecuteCommandsInfo), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

        if (!info)
        {
            primary->result = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }
        else
        {
            info->primary = commandBuffer;
            info->secondary = pCmdBuffers[i];
            info->stateBufferListIndex = (uint32_t)(primary->stateBufferTail - primary->stateBufferList);
            info->stateBufferOffset = primary->stateBufferTail->bufOffset;
            info->stateBufferPipe = primary->stateBufferTail->bufPipe;
            info->next = gcvNULL;
            primary->stateBufferTail->secBufCount++;
            if (!primary->executeTail)
            {
                primary->executeList = primary->executeTail = info;
            }
            else
            {
                 primary->executeTail->next = info;
                 primary->executeTail = info;
            }
        }

        /* Set the primary current pipe to the secondary's to force pipe select if needed */
        primary->stateBufferTail->bufPipe = secondary->stateBufferTail->bufPipe;
#if __VK_RESOURCE_INFO
        __vk_utils_mergeCmdRes(primary, secondary);
#endif
    }

    /* re-reprogram render pass states after 2nd command buffer execution. */
    if (commandBuffersCount)
    {
        primary->bindInfo.renderPass.dirty = VK_TRUE;
    }
}

#if __VK_NEW_DEVICE_QUEUE
VkResult __vk_InsertSemaphoreWaits(
    VkQueue queue,
    const VkSemaphore* pSemaphores,
    uint32_t semaphoreCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devQueue->pDevContext->fenceBuffer);
    __vkDeviceMemory *memory = fenceBuffer->memory;
    uint32_t fenceAddress;
    uint32_t stateSize;
    uint32_t *states;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    fenceAddress = memory->devAddr;

    stateSize = 2 * semaphoreCount * sizeof(uint32_t);

    states = (uint32_t *)__vk_QueueGetSpace(devQueue, stateSize);

    __VK_ONERROR(states ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    for (i = 0; i < semaphoreCount; i++)
    {
        __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pSemaphores[i]);

        *states++ = __VK_CMD_HW_FENCE_WAIT(10);
        *states++ = fenceAddress + sph->fenceIndex * sizeof(__vkHwFenceData);
    }

    __vk_QueueReleaseSpace(devQueue, stateSize);

OnError:

    return result;
}

VkResult __vk_InsertSemaphoreSignals(
    VkQueue queue,
    const VkSemaphore* pSemaphores,
    uint32_t semaphoreCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devQueue->pDevContext->fenceBuffer);
    __vkDeviceMemory *memory = fenceBuffer->memory;
    uint32_t fenceAddress;
    uint32_t *states;
    uint32_t stateSize;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    fenceAddress = memory->devAddr;

    stateSize = __VK_3D_FLUSH_PIPE_DMA_SIZE + (6 * semaphoreCount * sizeof(uint32_t));

    states = (uint32_t *)__vk_QueueGetSpace(devQueue, stateSize);

    __VK_ONERROR(states ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    __vkCmdLoadFlush3DHWStates(&states);

    for (i = 0; i < semaphoreCount; i++)
    {
        __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pSemaphores[i]);

        __vkCmdLoadSingleHWState(&states,
            0x0E1A, VK_FALSE, (fenceAddress + sph->fenceIndex * sizeof(__vkHwFenceData)));
        __vkCmdLoadSingleHWState(&states, 0x0E26, VK_FALSE, 0);
        __vkCmdLoadSingleHWState(&states, 0x0E1B, VK_FALSE, 1);
    }

    __vk_QueueReleaseSpace(devQueue, stateSize);

OnError:

    return result;
}

VkResult __vk_CommitSubmitFence(VkQueue queue, VkFence fence)
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
   __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    VkResult result = VK_SUCCESS;

    /* Event to signal when any device submit operation completes */
    if (fce)
    {
        gcsHAL_INTERFACE iface;
        iface.command             = gcvHAL_SIGNAL;
        iface.u.Signal.signal     = gcmPTR_TO_UINT64(fce->signal);
        iface.u.Signal.auxSignal  = 0;
        iface.u.Signal.process    = devQueue->pDevContext->threadId;
        iface.u.Signal.fromWhere  = devQueue->pDevContext->database->REG_BltEngine ?
                                    gcvKERNEL_BLT : gcvKERNEL_PIXEL;

        __VK_ONERROR(__vk_QueueAppendEvent(devQueue, &iface));

        __VK_ONERROR(__vk_QueueCommit(devQueue));
    }

OnError:

    return result;
}

VkResult __vk_CommitStateBuffers(
    VkQueue queue,
    __vk_CommitInfo* pCommits,
    uint32_t commitCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    for (i = 0; i < commitCount; i++)
    {
        uint32_t *states;
        uint32_t stateSize = pCommits[i].stateSize;

        states = (uint32_t *)__vk_QueueGetSpace(devQueue, stateSize);
        __VK_ONERROR(states ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);
        __VK_MEMCOPY(states, pCommits[i].stateStart, stateSize);

        __vk_QueueReleaseSpace(devQueue, stateSize);
    }

    __VK_ONERROR(__vk_QueueCommit(devQueue));

#if __VK_RESOURCE_SAVE_TGA || gcdDUMP
    __VK_ONERROR(__vk_QueueIdle(devQueue));
#endif

OnError:

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueWaitIdle(
    VkQueue queue
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;

    return __vk_QueueIdle(devQueue);
}

#else

VkResult __vk_InsertSemaphoreWaits(
    VkQueue queue,
    const VkSemaphore* pSemaphores,
    uint32_t semaphoreCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devQueue->pDevContext->fenceBuffer);
    __vkDeviceMemory *memory = fenceBuffer->memory;
    gcoCMDBUF reserve;
    gcoHARDWARE hardware = gcvNULL;
    uint32_t fenceAddress;
    uint32_t stateSize;
    uint32_t *states;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    /* Use the current thread's gcoHARDWARE object */
    __VK_ONERROR(gcoHAL_GetHardware(devQueue->pDevContext->hal, &hardware));

    fenceAddress = memory->devAddr;

    stateSize = 2 * semaphoreCount * sizeof(uint32_t);

    __VK_ONERROR(gcoBUFFER_Reserve(
        hardware->engine[gcvENGINE_RENDER].buffer,
        stateSize,
        gcvTRUE,
        gcvCOMMAND_3D,
        &reserve
        ));

    states = (uint32_t *)gcmUINT64_TO_PTR(reserve->lastReserve);

    for (i = 0; i < semaphoreCount; i++)
    {
        __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pSemaphores[i]);

        *states++ = __VK_CMD_HW_FENCE_WAIT(10);
        *states++ = fenceAddress + sph->fenceIndex * sizeof(__vkHwFenceData);
    }

OnError:

    return result;
}

VkResult __vk_InsertSemaphoreSignals(
    VkQueue queue,
    const VkSemaphore* pSemaphores,
    uint32_t semaphoreCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devQueue->pDevContext->fenceBuffer);
    __vkDeviceMemory *memory = fenceBuffer->memory;
    gcoCMDBUF reserve;
    gcoHARDWARE hardware = gcvNULL;
    uint32_t fenceAddress;
    uint32_t *states;
    uint32_t stateSize;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    /* Use the current thread's gcoHARDWARE object */
    __VK_ONERROR(gcoHAL_GetHardware(devQueue->pDevContext->hal, &hardware));

    fenceAddress = memory->devAddr;

    stateSize = __VK_3D_FLUSH_PIPE_DMA_SIZE + (6 * semaphoreCount * sizeof(uint32_t));

    __VK_ONERROR(gcoBUFFER_Reserve(
        hardware->engine[gcvENGINE_RENDER].buffer,
        stateSize,
        gcvTRUE,
        gcvCOMMAND_3D,
        &reserve
        ));

    states = (uint32_t *)gcmUINT64_TO_PTR(reserve->lastReserve);

    __vkCmdLoadFlush3DHWStates(&states);

    for (i = 0; i < semaphoreCount; i++)
    {
        __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pSemaphores[i]);

        __vkCmdLoadSingleHWState(&states,
            0x0E1A, VK_FALSE, (fenceAddress + sph->fenceIndex * sizeof(__vkHwFenceData)));
        __vkCmdLoadSingleHWState(&states, 0x0E26, VK_FALSE, 0);
        __vkCmdLoadSingleHWState(&states, 0x0E1B, VK_FALSE, 1);
    }

OnError:

    return result;
}

VkResult __vk_CommitSubmitFence(VkQueue queue, VkFence fence)
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
   __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    gcoHARDWARE hardware = gcvNULL;
    VkResult result = VK_SUCCESS;

    /* Use the current thread's gcoHARDWARE object */
    __VK_ONERROR(gcoHAL_GetHardware(devQueue->pDevContext->hal, &hardware));

    /* Event to signal when any device submit operation completes */
    if (fce)
    {
        __VK_VERIFY_OK(gcoHAL_ScheduleSignal(
            fce->signal,
            gcvNULL,
            gcmPTR2INT32(gcoOS_GetCurrentProcessID()),
            gcvKERNEL_BLT
            ));

        __VK_ONERROR(gcoHAL_Commit(gcvNULL, VK_FALSE));
    }

OnError:

    return result;
}

VkResult __vk_CommitStateBuffers(
    VkQueue queue,
    __vk_CommitInfo* pCommits,
    uint32_t commitCount
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    VkResult result = VK_SUCCESS;
    gcoCMDBUF reserve;
    gcoHARDWARE hardware = gcvNULL;
    uint32_t i;
    VkBool32 stallHw = VK_FALSE;

    /* Use the current thread's gcoHARDWARE object */
    __VK_ONERROR(gcoHAL_GetHardware(devQueue->pDevContext->hal, &hardware));

    for (i = 0; i < commitCount; i++)
    {
        __VK_ONERROR(gcoBUFFER_Reserve(
            hardware->engine[gcvENGINE_RENDER].buffer,
            pCommits[i].stateSize,
            gcvTRUE,
            gcvCOMMAND_3D,
            &reserve
            ));

        __VK_MEMCOPY(gcmUINT64_TO_PTR(reserve->lastReserve), pCommits[i].stateStart, pCommits[i].stateSize);
        hardware->currentPipe = (gcePIPE_SELECT)pCommits[i].statePipe;
    }

#if __VK_RESOURCE_SAVE_TGA || gcdDUMP
    stallHw = VK_TRUE;
#endif

    __VK_ONERROR(gcoHAL_Commit(gcvNULL, stallHw));

OnError:

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueWaitIdle(
    VkQueue queue
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    gcoHARDWARE hardware = gcvNULL;
    uint32_t flushCmds[__VK_3D_FLUSH_PIPE_DMA_SIZE / sizeof(uint32_t)];
    uint32_t *states = flushCmds;
    uint32_t stateSize = 0;
    uint32_t statePipe;
    gcoCMDBUF reserve;
    VkResult result = VK_SUCCESS;

    /* Use the current thread's gcoHARDWARE object */
    __VK_ONERROR(gcoHAL_GetHardware(devQueue->pDevContext->hal, &hardware));

    statePipe = hardware->currentPipe;

    /* Add the appropriate flush to the command buffer. */
    if (statePipe == gcvPIPE_2D)
    {
        stateSize = __VK_2D_FLUSH_PIPE_DMA_SIZE / sizeof(uint32_t);

        __vkCmdLoadFlush2DHWStates(&states);
    }
    else if (statePipe == gcvPIPE_3D)
    {
        stateSize = __VK_3D_FLUSH_PIPE_DMA_SIZE / sizeof(uint32_t);

        __vkCmdLoadFlush3DHWStates(&states);
    }

    if (stateSize)
    {
        __VK_ONERROR(gcoBUFFER_Reserve(
            hardware->engine[gcvENGINE_RENDER].buffer,
            stateSize,
            gcvTRUE,
            gcvCOMMAND_3D,
            &reserve
            ));

        __VK_MEMCOPY(gcmUINT64_TO_PTR(reserve->lastReserve), flushCmds, stateSize);
    }

    /* Signal for queue idle */
    __VK_ONERROR(gcoHAL_ScheduleSignal(
        devQueue->signal,
        gcvNULL,
        gcmPTR2INT32(gcoOS_GetCurrentProcessID()),
        gcvKERNEL_PIXEL
        ));

    __VK_ONERROR(gcoHAL_Commit(gcvNULL, VK_FALSE));

    __VK_ONERROR(gcoOS_WaitSignal(gcvNULL, devQueue->signal,gcvINFINITE));

OnError:

    return result;
}

#endif

VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueSubmit(
    VkQueue queue,
    uint32_t submitCount,
    const VkSubmitInfo* pSubmits,
    VkFence fence
    )
{
    __vkDevQueue *devQueue = (__vkDevQueue *)queue;
    __vk_CommitInfo *pCommits = gcvNULL;
    uint32_t isub, icmd, istate, iexe;
    VkResult result = VK_SUCCESS;

    __VK_SET_ALLOCATIONCB(&devQueue->pDevContext->memCb);

    /* TODO: Add semaphore support */

    pCommits = (__vk_CommitInfo *)__VK_ALLOC(
        (__VK_MAX_COMMITS * sizeof(__vk_CommitInfo)), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (!pCommits)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    __VK_MEMZERO(pCommits, (__VK_MAX_COMMITS * sizeof(__vk_CommitInfo)));

    for (isub = 0; isub < submitCount; isub++)
    {
        if (pSubmits[isub].waitSemaphoreCount)
        {
            __vk_InsertSemaphoreWaits(
                queue, pSubmits[isub].pWaitSemaphores, pSubmits[isub].waitSemaphoreCount);
        }

        for (icmd = 0; icmd < pSubmits[isub].commandBufferCount; icmd++)
        {
            __vkCommandBuffer *cmd = (__vkCommandBuffer *)pSubmits[isub].pCommandBuffers[icmd];
            __vkCmdExecuteCommandsInfo *exeInfo = cmd->executeList;
            __vkStateBuffer *stateBuffer = cmd->stateBufferList;
            uint32_t stateBufCount = cmd->lastStateBufferIndex;
            uint32_t icommits = 0;

            if (cmd->state != __VK_CMDBUF_STATE_EXECUTABLE)
                continue;

            cmd->state = __VK_CMDBUF_STATE_EXECUTION_PENDING;

            /* If the last state buffer is not empty increment the state buffer count */
            if ((cmd->stateBufferTail->bufOffset != 0) || (cmd->stateBufferTail->secBufCount != 0))
                stateBufCount++;

            for (istate = 0; istate < stateBufCount; istate++)
            {
                uint8_t *stateStart = stateBuffer->bufStart;
                uint32_t stateSize  = stateBuffer->bufOffset;
                uint32_t statePipe  = stateBuffer->bufPipe;

                /* First commit */
                if (stateBuffer->secBufCount)
                {
                    stateStart = stateBuffer->bufStart;
                    stateSize  = exeInfo->stateBufferOffset;
                    statePipe  = exeInfo->stateBufferPipe;
                }

                for (iexe = 0; iexe < stateBuffer->secBufCount; iexe++)
                {
                    __vkCommandBuffer *sec = (__vkCommandBuffer *)exeInfo->secondary;
                    __vkStateBuffer *secStateBuffer =sec->stateBufferList;
                    uint32_t secStateBufCount = sec->lastStateBufferIndex;
                    uint32_t isecState;

                    /* Add commit for current state buffer chunk */
                    if (stateSize)
                    {
                        pCommits[icommits].stateStart = stateStart;
                        pCommits[icommits].stateSize  = stateSize;
                        pCommits[icommits].statePipe  = statePipe;

                        icommits++;
                        __VK_ASSERT(icommits < __VK_MAX_COMMITS);
                    }

                    /* Update the remaining state buffer info */
                    stateStart = stateBuffer->bufStart + exeInfo->stateBufferOffset;
                    if (iexe == (stateBuffer->secBufCount - 1))
                    {
                        stateSize = stateBuffer->bufOffset - exeInfo->stateBufferOffset;
                        statePipe = stateBuffer->bufPipe;
                    }
                    else
                    {
                        stateSize = exeInfo->next->stateBufferOffset - exeInfo->stateBufferOffset;
                        statePipe = exeInfo->next->stateBufferPipe;
                    }

                    /* If the first state buffer is empty we do nothing and loop to the next command buffer */
                    if (sec->stateBufferList->bufOffset)
                        secStateBufCount++;

                    for (isecState = 0; isecState < secStateBufCount; isecState++)
                    {
                        pCommits[icommits].stateStart = secStateBuffer->bufStart;
                        pCommits[icommits].stateSize  = secStateBuffer->bufOffset;
                        pCommits[icommits].statePipe  = secStateBuffer->bufPipe;

                        icommits++;
                        __VK_ASSERT(icommits < __VK_MAX_COMMITS);

                        secStateBuffer = secStateBuffer->next;
                    }

                    exeInfo = exeInfo->next;
                }

                /* Add commit for current state buffer final chunk */
                if (stateSize)
                {
                    pCommits[icommits].stateStart = stateStart;
                    pCommits[icommits].stateSize  = stateSize;
                    pCommits[icommits].statePipe  = statePipe;

                    icommits++;
                }

                stateBuffer = stateBuffer->next;
            }

#if __VK_RESOURCE_INFO
            __vk_utils_dumpCmdInputRes(cmd);
#endif

            /* Commit all the remaining commits for this command buffer */
            if (icommits)
            {
                result = __vk_CommitStateBuffers(queue, pCommits, icommits);
                if (result != VK_SUCCESS)
                {
                    goto vk_OnError;
                }
                icommits = 0;
            }
#if __VK_RESOURCE_INFO
            __vk_utils_dumpCmdOutputRes(cmd);
#endif
            cmd->state = __VK_CMDBUF_STATE_EXECUTABLE;
        }

        if (pSubmits[isub].signalSemaphoreCount)
        {
            __vk_InsertSemaphoreSignals(
                queue, pSubmits[isub].pSignalSemaphores, pSubmits[isub].signalSemaphoreCount);
        }
    }

    result = __vk_CommitSubmitFence(queue, fence);

vk_OnError:
    __VK_FREE(pCommits);
    return result;
}


VKAPI_ATTR void VKAPI_CALL __vk_CmdBeginRenderPass(
    VkCommandBuffer commandBuffer,
    const VkRenderPassBeginInfo* pRenderPassBegin,
    VkSubpassContents contents
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkRenderPass *rdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkRenderPass *, pRenderPassBegin->renderPass);
    __vkFramebuffer *fb = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFramebuffer *, pRenderPassBegin->framebuffer);
    uint32_t i;

    __VK_ASSERT(rdp->attachmentCount == fb->attachmentCount);

    cmd->state = __VK_CMDBUF_STATE_BEGIN_RENDERPASS;        // Needed in __vk_CmdClearAttachments() only

    cmd->bindInfo.renderPass.rdp = (__vkRenderPass *)rdp;
    cmd->bindInfo.renderPass.subPass = &rdp->subPassInfo[0];
    cmd->bindInfo.renderPass.subPassContent = contents;
    cmd->bindInfo.renderPass.fb = fb;
    cmd->bindInfo.renderPass.dirty = VK_TRUE;

    rdp->fbDefault = fb;

    if (rdp->dependencyCount > 0)
    {
        for (i = 0; i < rdp->dependencyCount; i++)
        {
            if ((rdp->pDependencies[i].dstSubpass == 0) && (rdp->pDependencies[i].srcSubpass == VK_SUBPASS_EXTERNAL))
            {
                const VkMemoryBarrier memBarrier =
                {
                    VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                    0,
                    rdp->pDependencies[i].srcAccessMask,
                    rdp->pDependencies[i].dstAccessMask
                };

                __vk_CmdPipelineBarrier(
                    commandBuffer,
                    rdp->pDependencies[i].srcStageMask,
                    rdp->pDependencies[i].dstStageMask,
                    rdp->pDependencies[i].dependencyFlags,
                    1,
                    &memBarrier,
                    0,
                    VK_NULL_HANDLE,
                    0,
                    VK_NULL_HANDLE
                    );
            }
        }
    }

    for (i = 0; i < pRenderPassBegin->clearValueCount; i++)
    {
        if ((rdp->attachments[i].usedInRenderPass) &&
            (rdp->attachments[i].loadClear || rdp->attachments[i].stencil_loadClear))
        {
            VkClearAttachment clearAttachment;
            VkClearRect clearRect;

            clearAttachment.aspectMask = fb->imageViews[i]->createInfo.subresourceRange.aspectMask;
            /* Determine correct aspect mask for depth / stencil clear */
            if (rdp->attachments[i].stencil_loadClear)
            {
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

                if (rdp->attachments[i].loadClear)
                    clearAttachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            else
            {
                clearAttachment.aspectMask &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
            }

            clearAttachment.clearValue = pRenderPassBegin->pClearValues[i];
            if (clearAttachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
                clearAttachment.colorAttachment = i;
            else
                clearAttachment.colorAttachment = 0;

            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = fb->layers;
            clearRect.rect = pRenderPassBegin->renderArea;

            __vk_CmdClearAttachments(commandBuffer, 1, &clearAttachment, 1, &clearRect);

        }
    }

    cmd->state = __VK_CMDBUF_STATE_RECORDING;
}

void __vki_CmdResolveSubPass(
    VkCommandBuffer commandBuffer
    )
{
    uint32_t i, j;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkRenderPass *rdp = cmdBuf->bindInfo.renderPass.rdp;
    __vkFramebuffer *fb = cmdBuf->bindInfo.renderPass.fb;

    for (i = 0; i < rdp->subPassInfoCount; i++)
    {
        __vkRenderSubPassInfo *subPassInfo = &rdp->subPassInfo[i];
        for (j = 0; j < subPassInfo->colorCount; j++)
        {
            uint32_t resolve_ref = subPassInfo->resolve_attachment_index[j];
            uint32_t color_ref = subPassInfo->color_attachment_index[j];
            VkImageCopy regions;

            if (resolve_ref != VK_ATTACHMENT_UNUSED)
            {
                __vkImageView *colorImageViews = fb->imageViews[color_ref];
                __vkImageView *resolveImageViews = fb->imageViews[resolve_ref];
                __vkImage *colorImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, colorImageViews->createInfo.image);
                __vkImage *resolveImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, resolveImageViews->createInfo.image);
                VkImage srcImg = colorImageViews->createInfo.image;
                VkImage dstImg = resolveImageViews->createInfo.image;

                const VkMemoryBarrier memBarrier =
                {
                    VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                    0,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                };
                __VK_MEMZERO(&regions, sizeof(VkImageCopy));

                regions.dstOffset.x = regions.dstOffset.y = regions.dstOffset.z = 0;
                regions.srcOffset.x = regions.srcOffset.y = regions.srcOffset.z = 0;

                regions.srcSubresource.aspectMask = colorImageViews->createInfo.subresourceRange.aspectMask;
                regions.srcSubresource.mipLevel = colorImageViews->createInfo.subresourceRange.baseMipLevel;
                regions.srcSubresource.baseArrayLayer = colorImageViews->createInfo.subresourceRange.baseArrayLayer;
                regions.srcSubresource.layerCount = fb->layers;

                regions.dstSubresource.aspectMask = resolveImageViews->createInfo.subresourceRange.aspectMask;
                regions.dstSubresource.mipLevel = resolveImageViews->createInfo.subresourceRange.baseMipLevel;
                regions.dstSubresource.baseArrayLayer = resolveImageViews->createInfo.subresourceRange.baseArrayLayer;
                regions.dstSubresource.layerCount = fb->layers;

                regions.extent.width  = fb->width;
                regions.extent.height = fb->height;
                regions.extent.depth  = 1;

                __vk_CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                    1, &memBarrier, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);

                __vk_CmdCopyImage(commandBuffer, srcImg, colorImg->createInfo.initialLayout, dstImg, resolveImg->createInfo.initialLayout, 1, &regions);
            }
        }
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdNextSubpass(
    VkCommandBuffer commandBuffer,
    VkSubpassContents contents
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkRenderPass *rdp = cmd->bindInfo.renderPass.rdp;

    cmd->bindInfo.renderPass.subPass++;
    cmd->bindInfo.renderPass.subPassContent = contents;
    cmd->bindInfo.renderPass.dirty = VK_TRUE;

    if (rdp->dependencyCount > 0)
    {
        uint32_t isp,idp;

        for (isp = 0; isp < rdp->subPassInfoCount; isp++)
        {
            if (cmd->bindInfo.renderPass.subPass == &rdp->subPassInfo[isp])
                break;
        }

        for (idp = 0; idp < rdp->dependencyCount; idp++)
        {
            if ((rdp->pDependencies[idp].dstSubpass == isp) &&
                ((rdp->pDependencies[idp].srcSubpass == VK_SUBPASS_EXTERNAL) ||
                 (rdp->pDependencies[idp].srcSubpass < isp)))
            {
                const VkMemoryBarrier memBarrier =
                {
                    VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                    0,
                    rdp->pDependencies[idp].srcAccessMask,
                    rdp->pDependencies[idp].dstAccessMask
                };

                __vk_CmdPipelineBarrier(
                    commandBuffer,
                    rdp->pDependencies[idp].srcStageMask,
                    rdp->pDependencies[idp].dstStageMask,
                    rdp->pDependencies[idp].dependencyFlags,
                    1,
                    &memBarrier,
                    0,
                    VK_NULL_HANDLE,
                    0,
                    VK_NULL_HANDLE
                    );
            }
        }
    }

    __vki_CmdResolveSubPass(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdEndRenderPass(
    VkCommandBuffer commandBuffer
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;

    __vki_CmdResolveSubPass(commandBuffer);

    cmd->state = __VK_CMDBUF_STATE_RECORDING;

    cmd->bindInfo.renderPass.rdp = VK_NULL_HANDLE;
    cmd->bindInfo.renderPass.fb = VK_NULL_HANDLE;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdClearAttachments(
    VkCommandBuffer commandBuffer,
    uint32_t attachmentCount,
    const VkClearAttachment* pAttachments,
    uint32_t rectCount,
    const VkClearRect* pRects
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    __vkFramebuffer *fb = cmd->bindInfo.renderPass.fb;
    VkResult result = VK_SUCCESS;
    uint32_t ia, ir;

    const VkMemoryBarrier memBarrier =
    {
        VK_STRUCTURE_TYPE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    for (ia = 0; ia < attachmentCount; ia++)
    {
        uint32_t imgViewIndex = ia;

        if (pAttachments[ia].aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
            if (cmd->state == __VK_CMDBUF_STATE_BEGIN_RENDERPASS)
                imgViewIndex = pAttachments[ia].colorAttachment;
            else
                imgViewIndex = cmd->bindInfo.renderPass.subPass->color_attachment_index[pAttachments[ia].colorAttachment];
        else
            imgViewIndex = cmd->bindInfo.renderPass.subPass->dsAttachIndex;

        for (ir = 0; ir < rectCount; ir++)
        {
            VkImageSubresource subResource;
            uint32_t il;

            subResource.aspectMask = pAttachments[ia].aspectMask;
            subResource.mipLevel = fb->imageViews[imgViewIndex]->createInfo.subresourceRange.baseMipLevel;
            for (il = pRects[ir].baseArrayLayer; il < pRects[ir].layerCount; il++)
            {
                subResource.arrayLayer = il;
                __VK_ONERROR(cmd->devCtx->chipFuncs->ClearImage(
                    commandBuffer,
                    fb->imageViews[imgViewIndex]->createInfo.image,
                    &subResource,
                    (VkClearValue *)&pAttachments[ia].clearValue,
                    (VkRect2D *)&pRects[ir].rect
                    ));
            }
        }
    }

    /* For clear in renderPass, if TS enable, postphone setTS in drawValidate. */
    cmd->bindInfo.renderPass.dirty = VK_TRUE;

    __vk_CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
        1, &memBarrier, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE);


OnError:
    if (!__VK_IS_SUCCESS(result))
    {
        if (result != VK_ERROR_FORMAT_NOT_SUPPORTED)
            __VK_ASSERT(0);
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdResolveImage(
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageResolve* pRegions
    )
{
    uint32_t ir, il;
    __vkImage *pSrcImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, srcImage);
    __vkImage *pDstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dstImage);
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    VkResult result = VK_SUCCESS;

    for (ir = 0; ir < regionCount; ++ir)
    {
        __vkBlitRes srcRes, dstRes;
        uint32_t srcLayers;
        __VK_DEBUG_ONLY(uint32_t dstLayers);

        srcRes.isImage = VK_TRUE;
        srcRes.u.img.pImage = pSrcImage;
        srcRes.u.img.subRes.aspectMask = pRegions[ir].srcSubresource.aspectMask;
        srcRes.u.img.subRes.mipLevel   = pRegions[ir].srcSubresource.mipLevel;
        srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcSubresource.baseArrayLayer;
        srcRes.u.img.offset            = pRegions[ir].srcOffset;
        srcRes.u.img.extent            = pRegions[ir].extent;

        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = pDstImage;
        dstRes.u.img.subRes.aspectMask = pRegions[ir].dstSubresource.aspectMask;
        dstRes.u.img.subRes.mipLevel   = pRegions[ir].dstSubresource.mipLevel;
        dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstSubresource.baseArrayLayer;
        dstRes.u.img.offset            = pRegions[ir].dstOffset;
        dstRes.u.img.extent            = pRegions[ir].extent;

        if (pSrcImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcOffset.z;
            srcLayers = pRegions[ir].extent.depth;
        }
        else
        {
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcSubresource.baseArrayLayer;
            srcLayers = pRegions[ir].srcSubresource.layerCount;
        }

        if (pDstImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstOffset.z;
            __VK_DEBUG_ONLY(dstLayers = pRegions[ir].extent.depth);
        }
        else
        {
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstSubresource.baseArrayLayer;
            __VK_DEBUG_ONLY(dstLayers = pRegions[ir].dstSubresource.layerCount);
        }

        __VK_ASSERT(srcLayers == dstLayers);
        for (il = 0; il < srcLayers; il++)
        {
            __VK_ONERROR(devCtx->chipFuncs->CopyImage(
                commandBuffer,
                &srcRes,
                &dstRes,
                VK_FALSE
                ));

#if __VK_RESOURCE_INFO
            __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif

            srcRes.u.img.subRes.arrayLayer++;
            dstRes.u.img.subRes.arrayLayer++;
        }
    }

    return;

OnError:

    __VK_ASSERT(0);
}


VKAPI_ATTR void VKAPI_CALL __vk_CmdBindPipeline(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipeline pipeline
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkPipeline *pip = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipeline *, pipeline);
    __vkPipeline *oldpip = VK_NULL_HANDLE;
    switch (pipelineBindPoint)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        oldpip = cmdBuf->bindInfo.pipeline.graphics;
        cmdBuf->bindInfo.pipeline.graphics = pip;
        cmdBuf->bindInfo.pipeline.dirty |= __VK_CMDBUF_BINDNIGINFO_PIPELINE_GRAPHICS_DIRTY;
        break;
    case VK_PIPELINE_BIND_POINT_COMPUTE:
        oldpip = cmdBuf->bindInfo.pipeline.compute;
        cmdBuf->bindInfo.pipeline.compute = pip;
        cmdBuf->bindInfo.pipeline.dirty |= __VK_CMDBUF_BINDINGINFO_PIPELINE_COMPUTE_DIRTY;
        break;
    default:
        __VK_ASSERT(0);
        break;
    }

    (*cmdBuf->devCtx->chipFuncs->BindPipeline)(commandBuffer, (VkPipeline)(uintptr_t)oldpip, (VkPipeline)(uintptr_t)pip);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetViewport(
    VkCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const VkViewport* pViewports
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __VK_MEMCOPY(&cmdBuf->bindInfo.dynamicStates.viewport.viewports[firstViewport], pViewports, viewportCount * sizeof(VkViewport));
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_VIEWPORT_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetScissor(
    VkCommandBuffer commandBuffer,
    uint32_t firstScissor,
    uint32_t scissorCount,
    const VkRect2D* pScissors
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __VK_MEMCOPY(&cmdBuf->bindInfo.dynamicStates.scissor.scissors[firstScissor], pScissors, scissorCount * sizeof(VkRect2D));
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_SCISSOR_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetLineWidth(
    VkCommandBuffer commandBuffer,
    float lineWidth
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    cmdBuf->bindInfo.dynamicStates.lineWidth.width = lineWidth;
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_LINE_WIDTH_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetDepthBias(
    VkCommandBuffer commandBuffer,
    float depthBiasConstantFactor,
    float depthBiasClamp,
    float depthBiasSlopeFactor
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    cmdBuf->bindInfo.dynamicStates.depthBias.depthBiasConstantFactor = depthBiasConstantFactor;
    cmdBuf->bindInfo.dynamicStates.depthBias.depthBiasClamp = depthBiasClamp;
    cmdBuf->bindInfo.dynamicStates.depthBias.depthBiasSlopeFactor = depthBiasSlopeFactor;
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_DEPTH_BIAS_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetBlendConstants(
    VkCommandBuffer commandBuffer,
    const float blendConstants[4]
)
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    __VK_MEMCOPY(&cmdBuf->bindInfo.dynamicStates.blend.blendConstants[0], blendConstants, 4 * sizeof(float));
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_BLEND_CONSTANTS_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetDepthBounds(
    VkCommandBuffer commandBuffer,
    float minDepthBounds,
    float maxDepthBounds
    )
{
    /* NOT SUPPORTED */
    return;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetStencilCompareMask(
    VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t compareMask
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.compareMask[__VK_FACE_FRONT] = compareMask;
    }

    if (faceMask & VK_STENCIL_FACE_BACK_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.compareMask[__VK_FACE_BACK] = compareMask;
    }
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetStencilWriteMask(
    VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t writeMask
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.writeMask[__VK_FACE_FRONT] = writeMask;
    }

    if (faceMask & VK_STENCIL_FACE_BACK_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.writeMask[__VK_FACE_BACK] = writeMask;
    }
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_STENCIL_WRITE_MASK_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetStencilReference(
    VkCommandBuffer commandBuffer,
    VkStencilFaceFlags faceMask,
    uint32_t reference
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.reference[__VK_FACE_FRONT] = reference;
    }

    if (faceMask & VK_STENCIL_FACE_BACK_BIT)
    {
        cmdBuf->bindInfo.dynamicStates.stencil.reference[__VK_FACE_BACK] = reference;
    }
    cmdBuf->bindInfo.dynamicStates.dirtyMask |= __VK_DYNAMIC_STATE_STENCIL_REFERENCE_BIT;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdBindDescriptorSets(
    VkCommandBuffer commandBuffer,
    VkPipelineBindPoint pipelineBindPoint,
    VkPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const VkDescriptorSet* pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets
    )
{
    uint32_t i;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    __vkCmdBindDescSetInfo *bindDescInfo = VK_NULL_HANDLE;
    __vkPipelineLayout * plt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkPipelineLayout *, layout);
    uint32_t offsetInDynamicOffsets = 0;

    switch (pipelineBindPoint)
    {
    case VK_PIPELINE_BIND_POINT_GRAPHICS:
        bindDescInfo = &cmdBuf->bindInfo.bindDescSet.graphics;
        break;
    case VK_PIPELINE_BIND_POINT_COMPUTE:
        bindDescInfo = &cmdBuf->bindInfo.bindDescSet.compute;
        break;
    default:
        __VK_ASSERT(0);
        break;
    }

    for (i = 0; i < setCount; i++)
    {
        uint32_t setIdx = i + firstSet;
        uint32_t dynamicDescriptorCount = plt->descSetLayout[setIdx]->dynamicDescriptorCount;
        __vkDescriptorSet *descSet = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDescriptorSet *, pDescriptorSets[i]);

        __VK_ASSERT(setIdx < __VK_MAX_DESCRIPTOR_SETS);
        bindDescInfo->descSets[setIdx] = descSet;

        __VK_ASSERT(dynamicDescriptorCount == descSet->descSetLayout->dynamicDescriptorCount);

        if (dynamicDescriptorCount)
        {
            __VK_ASSERT(bindDescInfo->dynamicOffsets[setIdx]);

            __VK_MEMCOPY(
                bindDescInfo->dynamicOffsets[setIdx],
                pDynamicOffsets + offsetInDynamicOffsets,
                sizeof(uint32_t) * dynamicDescriptorCount);

            offsetInDynamicOffsets += dynamicDescriptorCount;
        }

        bindDescInfo->dirtyMask |= 1 << setIdx;
    }

    __VK_ASSERT(offsetInDynamicOffsets <= dynamicOffsetCount);

    if (setCount)
    {
        (*cmdBuf->devCtx->chipFuncs->BindDescritptors)(commandBuffer, pipelineBindPoint, firstSet, setCount);
    }
    return;

}

VKAPI_ATTR void VKAPI_CALL __vk_CmdBindIndexBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    VkIndexType indexType
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    cmdBuf->bindInfo.indexBuffer.firstIndex = 0;                    /* Updated at draw time */
    cmdBuf->bindInfo.indexBuffer.buffer     = buffer;
    cmdBuf->bindInfo.indexBuffer.offset     = offset;
    cmdBuf->bindInfo.indexBuffer.indexType  = indexType;
    cmdBuf->bindInfo.indexBuffer.dirty      = VK_TRUE;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdBindVertexBuffers(
    VkCommandBuffer commandBuffer,
    uint32_t startBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    uint32_t i;

    cmdBuf->bindInfo.vertexBuffers.firstBinding  = startBinding;    /* Currently not used */
    cmdBuf->bindInfo.vertexBuffers.bindingCount  = bindingCount;    /* Currently not used */
    cmdBuf->bindInfo.vertexBuffers.firstInstance = 0;               /* Updated at draw time */

    for (i = 0; i < bindingCount; i++)
    {
        cmdBuf->bindInfo.vertexBuffers.buffers[startBinding + i] = pBuffers[i];
        cmdBuf->bindInfo.vertexBuffers.offsets[startBinding + i] = pOffsets[i];
        cmdBuf->bindInfo.vertexBuffers.dirtyBits                |= 1 << (startBinding + i);
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDraw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
#if __VK_RESOURCE_INFO
    __vk_utils_insertDrawCmdRes(commandBuffer, VK_NULL_HANDLE);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->Draw)(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDrawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
#if __VK_RESOURCE_INFO
    __vk_utils_insertDrawCmdRes(commandBuffer, VK_NULL_HANDLE);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->DrawIndexed)(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDrawIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
    __VK_ASSERT(count == 0 || count == 1);
#if __VK_RESOURCE_INFO
    __vk_utils_insertDrawCmdRes(commandBuffer, buffer);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->DrawIndirect)(commandBuffer, buffer, offset, count, stride));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDrawIndexedIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t count,
    uint32_t stride
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
    __VK_ASSERT(count == 0 || count == 1);
#if __VK_RESOURCE_INFO
    __vk_utils_insertDrawCmdRes(commandBuffer, buffer);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->DrawIndexedIndirect)(commandBuffer, buffer, offset, count, stride));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDispatch(
    VkCommandBuffer commandBuffer,
    uint32_t x,
    uint32_t y,
    uint32_t z
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
#if __VK_RESOURCE_INFO
    __vk_utils_insertComputeCmdRes(commandBuffer, VK_NULL_HANDLE);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->Dispatch)(commandBuffer, x, y, z));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdDispatchIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer *)commandBuffer)->devCtx;
#if __VK_RESOURCE_INFO
    __vk_utils_insertComputeCmdRes(commandBuffer, buffer);
#endif
    __VK_VERIFY_OK((*devCtx->chipFuncs->DispatchIndirect)(commandBuffer, buffer, offset));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
    uint32_t ir;
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkBuffer *pSrcBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, srcBuffer);
    __vkBuffer *pDstBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, dstBuffer);
    __vkBlitRes srcRes, dstRes;
    VkResult result;

    __VK_MEMZERO(&srcRes, sizeof(srcRes));
    srcRes.isImage = VK_FALSE;
    srcRes.u.buf.pBuffer = pSrcBuf;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_FALSE;
    dstRes.u.buf.pBuffer = pDstBuf;

    /* Since this only copies raw image data we will simply use the CopyBuffer() */
    for (ir = 0; ir < regionCount; ir++)
    {
        srcRes.u.buf.offset = pRegions[ir].srcOffset;
        dstRes.u.buf.offset = pRegions[ir].dstOffset;

        __VK_ONERROR(devCtx->chipFuncs->CopyBuffer(
            commandBuffer,
            &srcRes,
            &dstRes,
            pRegions[ir].size
            ));

#if __VK_RESOURCE_INFO
        __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif
    }

    return;

OnError:

    __VK_ASSERT(0);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdCopyImage(
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageCopy* pRegions
    )
{
    uint32_t ir, il;
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer*)commandBuffer;
    __vkDevContext *devCtx = cmdBuf->devCtx;
    __vkImage *pSrcImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, srcImage);
    __vkImage *pDstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dstImage);
    uint32_t srcFormat = (uint32_t)pSrcImage->createInfo.format;
    uint32_t dstFormat = (uint32_t)pDstImage->createInfo.format;
    VkBool32 srcFaked = pSrcImage->formatInfo.residentImgFormat != srcFormat;
    VkBool32 dstFaked = pDstImage->formatInfo.residentImgFormat != dstFormat;
    VkResult result = VK_SUCCESS;

    for (ir = 0; ir < regionCount; ir++)
    {
        uint32_t srcLayers;
        __vkBlitRes srcRes, dstRes;
        __VK_DEBUG_ONLY(uint32_t dstLayers);

        srcRes.isImage = VK_TRUE;
        srcRes.u.img.pImage = pSrcImage;
        srcRes.u.img.subRes.aspectMask = pRegions[ir].srcSubresource.aspectMask;
        srcRes.u.img.subRes.mipLevel = pRegions[ir].srcSubresource.mipLevel;
        srcRes.u.img.offset = pRegions[ir].srcOffset;
        srcRes.u.img.extent = pRegions[ir].extent;

        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = pDstImage;
        dstRes.u.img.subRes.aspectMask = pRegions[ir].dstSubresource.aspectMask;
        dstRes.u.img.subRes.mipLevel = pRegions[ir].dstSubresource.mipLevel;
        dstRes.u.img.offset = pRegions[ir].dstOffset;
        dstRes.u.img.extent = pRegions[ir].extent;

        if (pSrcImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcOffset.z;
            srcLayers = pRegions[ir].extent.depth;
        }
        else
        {
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcSubresource.baseArrayLayer;
            srcLayers = pRegions[ir].srcSubresource.layerCount;
        }

        if (pDstImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstOffset.z;
            __VK_DEBUG_ONLY(dstLayers = pRegions[ir].extent.depth);
        }
        else
        {
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstSubresource.baseArrayLayer;
            __VK_DEBUG_ONLY(dstLayers = pRegions[ir].dstSubresource.layerCount);
        }

        __VK_ASSERT(srcLayers == dstLayers);

        for (il = 0; il < srcLayers; ++il)
        {
            __vkBlitRes *pSrcRes = &srcRes;
            __vkBlitRes *pDstRes = &dstRes;
            VkImage srcTmpImg = VK_NULL_HANDLE;
            VkImage dstTmpImg = VK_NULL_HANDLE;

            do
            {
                /* Skip the case if the 2 formats are same and both was faked to same resident formats */
                if (srcFormat != dstFormat ||
                    pSrcImage->formatInfo.residentImgFormat != pDstImage->formatInfo.residentImgFormat)
                {
                    __vkScratchMem *pScratchMem = gcvNULL;
                    VkImageCreateInfo imgCreateInfo;

                    if (srcFaked)
                    {
                        static __vkBlitRes srcTmpRes;
                        __vkImage *pSrcTmpImg = gcvNULL;

                        __VK_MEMCOPY(&imgCreateInfo, &pSrcImage->createInfo, sizeof(VkImageCreateInfo));
                        imgCreateInfo.extent        = srcRes.u.img.extent;
                        imgCreateInfo.mipLevels     = 1;
                        imgCreateInfo.arrayLayers   = 1;
                        imgCreateInfo.tiling        = VK_IMAGE_TILING_LINEAR;
                        imgCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                        __vk_CreateImage((VkDevice)devCtx, &imgCreateInfo, VK_NULL_HANDLE, &srcTmpImg);

                        pSrcTmpImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, srcTmpImg);
                        pScratchMem = __vkGetScratchMem(cmdBuf, pSrcTmpImg->memReq.size);
                        __vk_BindImageMemory((VkDevice)devCtx, srcTmpImg, (VkDeviceMemory)(uintptr_t)pScratchMem->memory, 0);

                        __VK_MEMCOPY(&srcTmpRes, &srcRes, sizeof(__vkBlitRes));
                        srcTmpRes.u.img.pImage = pSrcTmpImg;
                        srcTmpRes.u.img.subRes.mipLevel = 0;
                        srcTmpRes.u.img.subRes.arrayLayer = 0;
                        srcTmpRes.u.img.offset.x = 0;
                        srcTmpRes.u.img.offset.y = 0;
                        srcTmpRes.u.img.offset.z = 0;

                        __VK_ERR_BREAK(devCtx->chipFuncs->CopyImage(
                            commandBuffer,
                            &srcRes,
                            &srcTmpRes,
                            VK_FALSE
                            ));
#if __VK_RESOURCE_INFO
                        __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &srcTmpRes);
#endif
                        pSrcRes = &srcTmpRes;
                    }

                    if (dstFaked)
                    {
                        static __vkBlitRes dstTmpRes;
                        __vkImage *pDstTmpImg = gcvNULL;

                        __VK_MEMCOPY(&imgCreateInfo, &pDstImage->createInfo, sizeof(VkImageCreateInfo));
                        imgCreateInfo.extent        = dstRes.u.img.extent;
                        imgCreateInfo.mipLevels     = 1;
                        imgCreateInfo.arrayLayers   = 1;
                        imgCreateInfo.tiling        = VK_IMAGE_TILING_LINEAR;
                        imgCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                        __vk_CreateImage((VkDevice)devCtx, &imgCreateInfo, VK_NULL_HANDLE, &dstTmpImg);

                        pDstTmpImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dstTmpImg);
                        pScratchMem = __vkGetScratchMem(cmdBuf, pDstTmpImg->memReq.size);
                        __vk_BindImageMemory((VkDevice)devCtx, dstTmpImg, (VkDeviceMemory)(uintptr_t)pScratchMem->memory, 0);

                        __VK_MEMCOPY(&dstTmpRes, &dstRes, sizeof(__vkBlitRes));
                        dstTmpRes.u.img.pImage = pDstTmpImg;
                        dstTmpRes.u.img.subRes.mipLevel = 0;
                        dstTmpRes.u.img.subRes.arrayLayer = 0;
                        dstTmpRes.u.img.offset.x = 0;
                        dstTmpRes.u.img.offset.y = 0;
                        dstTmpRes.u.img.offset.z = 0;

                        pDstRes = &dstTmpRes;
                    }
                }

                if (!pSrcImage->formatInfo.compressed && !pDstImage->formatInfo.compressed)
                {
                    __VK_ERR_BREAK(devCtx->chipFuncs->CopyImage(
                        commandBuffer,
                        pSrcRes,
                        pDstRes,
                        VK_TRUE
                        ));
#if __VK_RESOURCE_INFO
                    __vk_utils_insertCopyCmdRes(commandBuffer, pSrcRes, pDstRes);
#endif
                }

                if (pDstRes != &dstRes)
                {
                    __VK_ERR_BREAK(devCtx->chipFuncs->CopyImage(
                        commandBuffer,
                        pDstRes,
                        &dstRes,
                        VK_FALSE
                        ));
#if __VK_RESOURCE_INFO
                    __vk_utils_insertCopyCmdRes(commandBuffer, pDstRes, &dstRes);
#endif
                }
            } while (VK_FALSE);

            if (srcTmpImg != VK_NULL_HANDLE)
            {
                __vk_DestroyImage((VkDevice)devCtx, srcTmpImg, VK_NULL_HANDLE);
            }

            if (dstTmpImg != VK_NULL_HANDLE)
            {
                __vk_DestroyImage((VkDevice)devCtx, dstTmpImg, VK_NULL_HANDLE);
            }

            __VK_ONERROR(result);

#if __VK_RESOURCE_INFO
            __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif

            srcRes.u.img.subRes.arrayLayer++;
            dstRes.u.img.subRes.arrayLayer++;
        }
    }

    return;

OnError:

    __VK_ASSERT(0);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdBlitImage(
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageBlit* pRegions,
    VkFilter filter
    )
{
    uint32_t ir;
    __vkImage *pSrcImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, srcImage);
    __vkImage *pDstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, dstImage);
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    VkResult result = VK_SUCCESS;

    for (ir = 0; ir < regionCount; ++ir)
    {
        uint32_t il;
        VkBool32 scale = VK_FALSE;
        VkBool3D reverse = {VK_FALSE, VK_FALSE, VK_FALSE};
        __vkBlitRes srcRes, dstRes;

        srcRes.isImage = VK_TRUE;
        srcRes.u.img.pImage = pSrcImage;
        srcRes.u.img.subRes.aspectMask = pRegions[ir].srcSubresource.aspectMask;
        srcRes.u.img.subRes.mipLevel   = pRegions[ir].srcSubresource.mipLevel;
        srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcSubresource.baseArrayLayer;
        if (pRegions[ir].srcOffsets[1].x > pRegions[ir].srcOffsets[0].x)
        {
            srcRes.u.img.offset.x      = pRegions[ir].srcOffsets[0].x;
            srcRes.u.img.extent.width  = pRegions[ir].srcOffsets[1].x - pRegions[ir].srcOffsets[0].x;
        }
        else
        {
            srcRes.u.img.offset.x      = pRegions[ir].srcOffsets[1].x;
            srcRes.u.img.extent.width  = pRegions[ir].srcOffsets[0].x - pRegions[ir].srcOffsets[1].x;
            reverse.x = !reverse.x;
        }
        if (pRegions[ir].srcOffsets[1].y > pRegions[ir].srcOffsets[0].y)
        {
            srcRes.u.img.offset.y      = pRegions[ir].srcOffsets[0].y;
            srcRes.u.img.extent.height = pRegions[ir].srcOffsets[1].y - pRegions[ir].srcOffsets[0].y;
        }
        else
        {
            srcRes.u.img.offset.y      = pRegions[ir].srcOffsets[1].y;
            srcRes.u.img.extent.height = pRegions[ir].srcOffsets[0].y - pRegions[ir].srcOffsets[1].y;
            reverse.y = !reverse.y;
        }
        if (pRegions[ir].srcOffsets[1].z > pRegions[ir].srcOffsets[0].z)
        {
            srcRes.u.img.offset.z      = pRegions[ir].srcOffsets[0].z;
            srcRes.u.img.extent.depth  = pRegions[ir].srcOffsets[1].z - pRegions[ir].srcOffsets[0].z;
        }
        else
        {
            srcRes.u.img.offset.z      = pRegions[ir].srcOffsets[1].z;
            srcRes.u.img.extent.depth  = pRegions[ir].srcOffsets[0].z - pRegions[ir].srcOffsets[1].z;
            reverse.z = !reverse.z;
        }


        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = pDstImage;
        dstRes.u.img.subRes.aspectMask = pRegions[ir].dstSubresource.aspectMask;
        dstRes.u.img.subRes.mipLevel   = pRegions[ir].dstSubresource.mipLevel;
        dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstSubresource.baseArrayLayer;
        if (pRegions[ir].dstOffsets[1].x > pRegions[ir].dstOffsets[0].x)
        {
            dstRes.u.img.offset.x      = pRegions[ir].dstOffsets[0].x;
            dstRes.u.img.extent.width  = pRegions[ir].dstOffsets[1].x - pRegions[ir].dstOffsets[0].x;
        }
        else
        {
            dstRes.u.img.offset.x      = pRegions[ir].dstOffsets[1].x;
            dstRes.u.img.extent.width  = pRegions[ir].dstOffsets[0].x - pRegions[ir].dstOffsets[1].x;
            reverse.x = !reverse.x;
        }
        if (pRegions[ir].dstOffsets[1].y > pRegions[ir].dstOffsets[0].y)
        {
            dstRes.u.img.offset.y      = pRegions[ir].dstOffsets[0].y;
            dstRes.u.img.extent.height = pRegions[ir].dstOffsets[1].y - pRegions[ir].dstOffsets[0].y;
        }
        else
        {
            dstRes.u.img.offset.y      = pRegions[ir].dstOffsets[1].y;
            dstRes.u.img.extent.height = pRegions[ir].dstOffsets[0].y - pRegions[ir].dstOffsets[1].y;
            reverse.y = !reverse.y;
        }
        if (pRegions[ir].dstOffsets[1].z > pRegions[ir].dstOffsets[0].z)
        {
            dstRes.u.img.offset.z      = pRegions[ir].dstOffsets[0].z;
            dstRes.u.img.extent.depth  = pRegions[ir].dstOffsets[1].z - pRegions[ir].dstOffsets[0].z;
        }
        else
        {
            dstRes.u.img.offset.z      = pRegions[ir].dstOffsets[1].z;
            dstRes.u.img.extent.depth  = pRegions[ir].dstOffsets[0].z - pRegions[ir].dstOffsets[1].z;
            reverse.z = !reverse.z;
        }

        if (gcoOS_MemCmp(&srcRes.u.img.extent, &dstRes.u.img.extent, sizeof(VkExtent3D)) != 0)
        {
            scale = VK_TRUE;
        }

        if (!scale && !reverse.x && !reverse.y && !reverse.z)   /* TODO: Include unsupported format check */
        {
            uint32_t srcLayers;
            __VK_DEBUG_ONLY(uint32_t dstLayers);

            if (pSrcImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
            {
                srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcOffsets[0].z;
                srcLayers = srcRes.u.img.extent.depth;
            }
            else
            {
                srcRes.u.img.subRes.arrayLayer = pRegions[ir].srcSubresource.baseArrayLayer;
                srcLayers = pRegions[ir].srcSubresource.layerCount;
            }

            if (pDstImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
            {
                dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstOffsets[0].z;
                __VK_DEBUG_ONLY(dstLayers = dstRes.u.img.extent.depth);
            }
            else
            {
                dstRes.u.img.subRes.arrayLayer = pRegions[ir].dstSubresource.baseArrayLayer;
                __VK_DEBUG_ONLY(dstLayers = pRegions[ir].dstSubresource.layerCount);
            }

            __VK_ASSERT(srcLayers == dstLayers);

            for (il = 0; il < srcLayers; il++)
            {
                __VK_ONERROR(devCtx->chipFuncs->CopyImage(
                    commandBuffer,
                    &srcRes,
                    &dstRes,
                    VK_FALSE
                    ));

#if __VK_RESOURCE_INFO
                __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif

                srcRes.u.img.subRes.arrayLayer++;
                dstRes.u.img.subRes.arrayLayer++;
            }
        }
        else
        {
            uint32_t layers = 1;

            /* Do per-slice compute blit if neither src nor dst are of 3D type. */
            if (pSrcImage->createInfo.imageType != VK_IMAGE_TYPE_3D &&
                pDstImage->createInfo.imageType != VK_IMAGE_TYPE_3D)
            {
                layers = pRegions[ir].srcSubresource.layerCount;
            }

            for (il = 0; il < layers; il++)
            {
                __VK_ONERROR(devCtx->chipFuncs->ComputeBlit(
                    commandBuffer,
                    &srcRes,
                    &dstRes,
                    &reverse,
                    filter
                    ));

#if __VK_RESOURCE_INFO
                __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif
                srcRes.u.img.subRes.arrayLayer++;
                dstRes.u.img.subRes.arrayLayer++;
            }
        }
    }

    return;

OnError:

    __VK_ASSERT(0);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdCopyBufferToImage
    (VkCommandBuffer commandBuffer,
    VkBuffer srcBuffer,
    VkImage dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkBufferImageCopy* pRegions
    )
{
    uint32_t ir, il;
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkBuffer *pSrcBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, srcBuffer);
    __vkImage *pDstImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage  *, dstImage);
    VkResult result = VK_SUCCESS;

    for (ir = 0; ir < regionCount; ir++)
    {
        const __vkFormatInfo *fmtInfo;
        __vkBlitRes srcRes, dstRes;
        uint32_t srcWidth, srcHeight, srcSliceBytes;
        uint32_t dstLayers;

        srcRes.isImage = VK_FALSE;
        srcRes.u.buf.pBuffer = pSrcBuf;
        srcRes.u.buf.offset = pRegions[ir].bufferOffset;
        srcRes.u.buf.rowLength = pRegions[ir].bufferRowLength;
        srcRes.u.buf.imgHeight = pRegions[ir].bufferImageHeight;

        dstRes.isImage = VK_TRUE;
        dstRes.u.img.pImage = pDstImg;
        dstRes.u.img.subRes.aspectMask = pRegions[ir].imageSubresource.aspectMask;
        dstRes.u.img.subRes.mipLevel = pRegions[ir].imageSubresource.mipLevel;
        dstRes.u.img.offset = pRegions[ir].imageOffset;
        dstRes.u.img.extent = pRegions[ir].imageExtent;

        if (pDstImg->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            /* According to vulkan spec, for 3D image, baseArrayLayer = 0, arrayLayers =1.
            ** But for HW, 3D Image arrangement is the same as 2D Array. So we also use baseArrayLayer and arrayLayers.
            */
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].imageOffset.z;
            dstLayers = pRegions[ir].imageExtent.depth;
        }
        else
        {
            dstRes.u.img.subRes.arrayLayer = pRegions[ir].imageSubresource.baseArrayLayer;
            dstLayers = pRegions[ir].imageSubresource.layerCount;
        }

        fmtInfo = &g_vkFormatInfoTable[pDstImg->createInfo.format];
        srcWidth  = (pRegions[ir].bufferRowLength != 0)   ? pRegions[ir].bufferRowLength   : pRegions[ir].imageExtent.width;
        srcHeight = (pRegions[ir].bufferImageHeight != 0) ? pRegions[ir].bufferImageHeight : pRegions[ir].imageExtent.height;
        srcSliceBytes = ((srcWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8) * (srcHeight / fmtInfo->blockSize.height);

        for (il = 0; il < dstLayers; il++)
        {
            __VK_ONERROR(devCtx->chipFuncs->CopyImage(
                commandBuffer,
                &srcRes,
                &dstRes,
                VK_FALSE
                ));
#if __VK_RESOURCE_INFO
            __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif
            srcRes.u.buf.offset += srcSliceBytes;
            dstRes.u.img.subRes.arrayLayer++;
        }
    }

    return;

OnError:

    __VK_ASSERT(0);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdCopyImageToBuffer(
    VkCommandBuffer commandBuffer,
    VkImage srcImage,
    VkImageLayout srcImageLayout,
    VkBuffer dstBuffer,
    uint32_t regionCount,
    const VkBufferImageCopy* pRegions
    )
{
    uint32_t ir, il;
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkImage *pSrcImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage   *, srcImage);
    __vkBuffer *pDstBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, dstBuffer);
    VkResult result = VK_SUCCESS;

    for (ir = 0; ir < regionCount; ir++)
    {
        const __vkFormatInfo *fmtInfo;
        __vkBlitRes srcRes, dstRes;
        uint32_t dstWidth, dstHeight, dstSliceBytes;
        uint32_t srcLayers;

        srcRes.isImage = VK_TRUE;
        srcRes.u.img.pImage = pSrcImg;
        srcRes.u.img.subRes.aspectMask = pRegions[ir].imageSubresource.aspectMask;
        srcRes.u.img.subRes.mipLevel = pRegions[ir].imageSubresource.mipLevel;
        srcRes.u.img.offset = pRegions[ir].imageOffset;
        srcRes.u.img.extent = pRegions[ir].imageExtent;

        dstRes.isImage = VK_FALSE;
        dstRes.u.buf.pBuffer = pDstBuf;
        dstRes.u.buf.offset = pRegions[ir].bufferOffset;
        dstRes.u.buf.rowLength = pRegions[ir].bufferRowLength;
        dstRes.u.buf.imgHeight = pRegions[ir].bufferImageHeight;

        if (pSrcImg->createInfo.imageType == VK_IMAGE_TYPE_3D)
        {
            /* According to vulkan spec, for 3D image, baseArrayLayer = 0, arrayLayers =1.
            ** But for HW, 3D Image arrangement is the same as 2D Array. So we also use baseArrayLayer and arrayLayers.
            */
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].imageOffset.z;
            srcLayers = pRegions[ir].imageExtent.depth;
        }
        else
        {
            srcRes.u.img.subRes.arrayLayer = pRegions[ir].imageSubresource.baseArrayLayer;
            srcLayers = pRegions[ir].imageSubresource.layerCount;
        }

        fmtInfo = &g_vkFormatInfoTable[pSrcImg->createInfo.format];
        dstWidth  = (pRegions[ir].bufferRowLength != 0)   ? pRegions[ir].bufferRowLength   : pRegions[ir].imageExtent.width;
        dstHeight = (pRegions[ir].bufferImageHeight != 0) ? pRegions[ir].bufferImageHeight : pRegions[ir].imageExtent.height;
        dstSliceBytes = ((dstWidth / fmtInfo->blockSize.width) * fmtInfo->bitsPerBlock / 8) * (dstHeight / fmtInfo->blockSize.height);

        for (il = 0; il < srcLayers; il++)
        {
            __VK_ONERROR(devCtx->chipFuncs->CopyImage(
                commandBuffer,
                &srcRes,
                &dstRes,
                VK_FALSE
                ));
#if __VK_RESOURCE_INFO
            __vk_utils_insertCopyCmdRes(commandBuffer, &srcRes, &dstRes);
#endif
            srcRes.u.img.subRes.arrayLayer++;
            dstRes.u.buf.offset += dstSliceBytes;
        }
    }

OnError:
    if (!__VK_IS_SUCCESS(result))
    {
        if (result != VK_ERROR_FORMAT_NOT_SUPPORTED)
            __VK_ASSERT(0);
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdUpdateBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize dataSize,
    const void* pData
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;

#if __VK_RESOURCE_INFO
    __vkBlitRes dstRes;
    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_FALSE;
    dstRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, dstBuffer);
    dstRes.u.buf.offset = dstOffset;
    __vk_utils_insertCopyCmdRes(commandBuffer, gcvNULL, &dstRes);
#endif

    __VK_VERIFY_OK((*devCtx->chipFuncs->UpdateBuffer)(commandBuffer, dstBuffer, dstOffset, dataSize, pData));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdFillBuffer(
    VkCommandBuffer commandBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize fillSize,
    uint32_t data
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;

#if __VK_RESOURCE_INFO
    __vkBlitRes dstRes;
    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_FALSE;
    dstRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, dstBuffer);
    dstRes.u.buf.offset = dstOffset;
    __vk_utils_insertCopyCmdRes(commandBuffer, gcvNULL, &dstRes);
#endif

    __VK_VERIFY_OK((*devCtx->chipFuncs->FillBuffer)(commandBuffer, dstBuffer, dstOffset, fillSize, data));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdClearColorImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearColorValue* pColor,
    uint32_t rangeCount,
    const VkImageSubresourceRange* pRanges
    )
{
    uint32_t level;
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkImage *pDstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, image);
    __vkBlitRes dstRes;
    VkClearValue clearValue;
    VkRect2D clearRect;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_TRUE;
    dstRes.u.img.pImage = pDstImage;

    for (i = 0; i < rangeCount; i++)
    {
        dstRes.u.img.subRes.aspectMask = pRanges[i].aspectMask;
        clearValue.color = *pColor;
        clearRect.offset.x = 0;
        clearRect.offset.y = 0;

        for (level = pRanges[i].baseMipLevel; level < pRanges[i].baseMipLevel + pRanges[i].levelCount; ++level)
        {
            uint32_t il, layers;
            __vkImageLevel *pLevel = &pDstImage->pImgLevels[level];

            dstRes.u.img.subRes.mipLevel = level;
            clearRect.extent.width  = pLevel->requestW;
            clearRect.extent.height = pLevel->requestH;

            if (pDstImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
            {
                dstRes.u.img.subRes.arrayLayer = 0;
                layers = pDstImage->createInfo.extent.depth;
            }
            else
            {
                dstRes.u.img.subRes.arrayLayer = pRanges->baseArrayLayer;
                layers = pRanges->layerCount;
            }

            for (il = 0; il < layers; ++il)
            {
                __VK_ONERROR((*devCtx->chipFuncs->ClearImage)(commandBuffer, image, &dstRes.u.img.subRes, &clearValue, &clearRect));

#if __VK_RESOURCE_INFO
                __vk_utils_insertCopyCmdRes(commandBuffer, gcvNULL, &dstRes);
#endif

                dstRes.u.img.subRes.arrayLayer++;
            }
        }
    }

OnError:
   return;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdClearDepthStencilImage(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue* pDepthStencil,
    uint32_t rangeCount,
    const VkImageSubresourceRange* pRanges
    )
{
    uint32_t level;
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkImage *pDstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage*, image);
    __vkBlitRes dstRes;
    VkClearValue clearValue;
    VkRect2D clearRect;
    VkResult result = VK_SUCCESS;
    uint32_t i;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_TRUE;
    dstRes.u.img.pImage = pDstImage;

    for (i = 0; i < rangeCount; i++)
    {
        dstRes.u.img.subRes.aspectMask = pRanges[i].aspectMask;
        clearValue.depthStencil = *pDepthStencil;
        clearRect.offset.x = 0;
        clearRect.offset.y = 0;
        for (level = pRanges[i].baseMipLevel; level < pRanges[i].baseMipLevel + pRanges[i].levelCount; ++level)
        {
            uint32_t il, layers;
            __vkImageLevel *pLevel = &pDstImage->pImgLevels[level];

            dstRes.u.img.subRes.mipLevel = level;
            clearRect.extent.width = pLevel->requestW;
            clearRect.extent.height = pLevel->requestH;

            if (pDstImage->createInfo.imageType == VK_IMAGE_TYPE_3D)
            {
                dstRes.u.img.subRes.arrayLayer = 0;
                layers = pDstImage->createInfo.extent.depth;
            }
            else
            {
                dstRes.u.img.subRes.arrayLayer = pRanges->baseArrayLayer;
                layers = pRanges->layerCount;
            }

            for (il = 0; il < layers; ++il)
            {
                __VK_ONERROR((*devCtx->chipFuncs->ClearImage)(commandBuffer, image, &dstRes.u.img.subRes, &clearValue, &clearRect));

#if __VK_RESOURCE_INFO
                __vk_utils_insertCopyCmdRes(commandBuffer, gcvNULL, &dstRes);
#endif

                dstRes.u.img.subRes.arrayLayer++;
            }
        }
    }

OnError:
    return;
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdSetEvent(
    VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags stageMask
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    (*cmdBuf->devCtx->chipFuncs->SetEvent)(commandBuffer, event, stageMask, VK_TRUE);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdResetEvent(
    VkCommandBuffer commandBuffer,
    VkEvent event,
    VkPipelineStageFlags stageMask
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    (*cmdBuf->devCtx->chipFuncs->SetEvent)(commandBuffer, event, stageMask, VK_FALSE);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdWaitEvents(
    VkCommandBuffer commandBuffer,
    uint32_t eventCount,
    const VkEvent* pEvents,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags destStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    (*cmdBuf->devCtx->chipFuncs->WaitEvents)(commandBuffer, eventCount, pEvents, srcStageMask, destStageMask,
        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdPipelineBarrier
    (VkCommandBuffer commandBuffer,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags destStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* pImageMemoryBarriers
    )
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;

    (*cmdBuf->devCtx->chipFuncs->PipelineBarrier)(commandBuffer, srcStageMask, destStageMask, dependencyFlags,
        memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers,
        imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdBeginQuery(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query,
    VkQueryControlFlags flags
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);

    if (qyp->pQueries[query].state != __VK_QUERY_RESET)
    {
        __VK_ASSERT(0);
        return;
    }

    qyp->pQueries[query].state = __VK_QUERY_BEGIN;
    qyp->pQueries[query].flags = flags;

    __VK_VERIFY_OK(devCtx->chipFuncs->ProcessQueryRequest(commandBuffer, queryPool, query));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdEndQuery(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t query
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);

    if (qyp->pQueries[query].state == __VK_QUERY_ISSUED)
    {
        qyp->pQueries[query].state = __VK_QUERY_END;
        __VK_VERIFY_OK(devCtx->chipFuncs->ProcessQueryRequest(commandBuffer, queryPool, query));
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdResetQueryPool(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount
    )
{
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    uint32_t iq;

    /* We use events to determine if a query's status is available or not */
    for (iq = firstQuery; iq < (firstQuery + queryCount); iq++)
    {
        qyp->pQueries[iq].state = __VK_QUERY_RESET;
        __VK_VERIFY_OK(devCtx->chipFuncs->ProcessQueryRequest(commandBuffer, queryPool, iq));
        __vk_CmdResetEvent(commandBuffer, qyp->pQueries[iq].event, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    }

    __VK_VERIFY_OK(devCtx->chipFuncs->FillBuffer(
        commandBuffer,
        qyp->queryBuffer,
        (firstQuery * sizeof(uint64_t)),
        (queryCount * sizeof(uint64_t)),
        0
        ));
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdWriteTimestamp(
    VkCommandBuffer commandBuffer,
    VkPipelineStageFlagBits pipelineStage,
    VkQueryPool queryPool,
    uint32_t entry
    )
{
    /* TODO */
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdCopyQueryPoolResults(
    VkCommandBuffer commandBuffer,
    VkQueryPool queryPool,
    uint32_t firstQuery,
    uint32_t queryCount,
    VkBuffer destBuffer,
    VkDeviceSize destOffset,
    VkDeviceSize destStride,
    VkQueryResultFlags flags
    )
{
    __vkQueryPool *qyp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkQueryPool *, queryPool);
    __vkDevContext *devCtx = ((__vkCommandBuffer*)commandBuffer)->devCtx;
    __vkBuffer *queryBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, qyp->queryBuffer);
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devCtx->fenceBuffer);
    __vkBuffer *dstBuf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, destBuffer);
    __vkBlitRes srcRes,dstRes;
    uint32_t copySize;
    uint32_t iq;
    srcRes.isImage = dstRes.isImage = VK_FALSE;
    dstRes.u.buf.pBuffer = dstBuf;

    for (iq = firstQuery; iq < (firstQuery + queryCount); iq++)
    {
        __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, qyp->pQueries[iq].event);

        if (flags & VK_QUERY_RESULT_WAIT_BIT)
        {
            VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

            __vk_CmdWaitEvents(
                commandBuffer,
                1,
                &qyp->pQueries[iq].event,
                srcFlags,
                dstFlags,
                0,
                VK_NULL_HANDLE,
                0,
                VK_NULL_HANDLE,
                0,
                VK_NULL_HANDLE
                );
        }
        srcRes.u.buf.pBuffer = queryBuffer;
        srcRes.u.buf.offset = iq * sizeof(uint64_t);
        dstRes.u.buf.offset = destOffset + (iq*destStride);
        if (flags & VK_QUERY_RESULT_64_BIT)
        {
            copySize = sizeof(uint64_t);

        }
        else
        {
            copySize = sizeof(uint32_t);
        }

        /* Copy the query result */
        __VK_VERIFY_OK(devCtx->chipFuncs->CopyBuffer(
            commandBuffer,
            &srcRes,
            &dstRes,
            copySize
            ));

        if ((flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) && (destStride >= (2 * copySize)))
        {
            /* Copy the query availability status (event fence value) */
             srcRes.u.buf.pBuffer = fenceBuffer;
             srcRes.u.buf.offset = evt->fenceIndex * sizeof(__vkHwFenceData);
             dstRes.u.buf.offset += copySize ;
            __VK_VERIFY_OK(devCtx->chipFuncs->CopyBuffer(
                commandBuffer,
                &srcRes,
                &dstRes,
                copySize
                ));
        }
    }
}

VKAPI_ATTR void VKAPI_CALL __vk_CmdPushConstants(
    VkCommandBuffer commandBuffer,
    VkPipelineLayout layout,
    VkShaderStageFlags stageFlags,
    uint32_t start,
    uint32_t length,
    const void* values)
{
    __vkCommandBuffer *cmdBuf = (__vkCommandBuffer *)commandBuffer;
    uint32_t *startUpdate = (uint32_t *)((uint8_t *)cmdBuf->bindInfo.pushConstants.values + start);

    __VK_ASSERT(((start + length) >> 2) <= __VK_MAX_PUSHCONST_SIZE_IN_UINT);

    __VK_MEMCOPY(startUpdate, values, length);

    if (length == (__VK_MAX_PUSHCONST_SIZE_IN_UINT << 2))
    {
        cmdBuf->bindInfo.pushConstants.dirtyMask = (uint32_t)~0;
    }
    else
    {
        cmdBuf->bindInfo.pushConstants.dirtyMask |= (~(~0 << (length >> 2))) << (start >> 2);
    }
}

void __vk_CmdPipeSelect(
    VkCommandBuffer commandBuffer,
    uint32_t pipe
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;

    /* Insert the pipe select. */
    if ((pipe == gcvPIPE_2D) || (pipe == gcvPIPE_3D))
    {
        uint32_t *states = (uint32_t *)(cmd->stateBufferTail->bufStart + cmd->stateBufferTail->bufOffset);

        /* Select Pipe */
        __vkCmdLoadSingleHWState(&states, 0x0E00, VK_FALSE, pipe);

        /* Increase the buffer offset */
        cmd->stateBufferTail->bufOffset += __VK_PIPE_SELECT_DMA_SIZE;

        cmd->stateBufferTail->bufPipe = pipe;
    }
}

void __vk_CmdPipeFlush(
    VkCommandBuffer commandBuffer,
    VkBool32 bypassDmaFunctions
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;
    uint32_t pipe = cmd->stateBufferTail->bufPipe;

    /* Add the appropriate flush to the command buffer. */
    if (pipe == gcvPIPE_2D)
    {
        uint32_t *states = NULL;
        uint32_t requestSize = __VK_2D_FLUSH_PIPE_DMA_SIZE / sizeof(uint32_t);

        if (bypassDmaFunctions)
        {
            states = (uint32_t *)(cmd->stateBufferTail->bufStart + cmd->stateBufferTail->bufOffset);
        }
        else
        {
            __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);
        }

        __vkCmdLoadFlush2DHWStates(&states);

        if (bypassDmaFunctions)
        {
            cmd->stateBufferTail->bufOffset += __VK_2D_FLUSH_PIPE_DMA_SIZE;
        }
        else
        {
            __vk_CmdReleaseBuffer(commandBuffer, requestSize);
        }
    }
    else if (pipe == gcvPIPE_3D)
    {
        uint32_t *states = NULL;
        uint32_t requestSize = __VK_3D_FLUSH_PIPE_DMA_SIZE / sizeof(uint32_t);

        if (bypassDmaFunctions)
        {
            states = (uint32_t *)(cmd->stateBufferTail->bufStart + cmd->stateBufferTail->bufOffset);
        }
        else
        {
            __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);
        }

        __vkCmdLoadFlush3DHWStates(&states);

        if (bypassDmaFunctions)
        {
            cmd->stateBufferTail->bufOffset += __VK_3D_FLUSH_PIPE_DMA_SIZE;
        }
        else
        {
            __vk_CmdReleaseBuffer(commandBuffer, requestSize);
        }
    }
}

void __vk_CmdAquireBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t count,
    uint32_t **cmdBuffer
    )
{
    __vkCommandBuffer *cmd = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandBuffer *, commandBuffer);
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmd->commandPool);
    uint32_t pipe = gcvPIPE_3D;
    VkBool32 bPipeSelectNeeded = VK_FALSE;
    VkBool32 bPipeFlushNeeded = VK_FALSE;
    uint32_t pipeSelectFlushDMASize = 0;
    uint32_t bytesAvailable;

    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    *cmdBuffer = gcvNULL;

    /* Can't request buffer space if a request is pending */
    if (cmd->bufWriteRequestCount)
    {
        __VK_ASSERT(VK_FALSE);
        return;
    }

    /* Zero size request is invalid */
    if (!count)
    {
        __VK_ASSERT(VK_FALSE);
        return;
    }

    if (pipe != cmd->stateBufferTail->bufPipe)
    {
        if (cmd->stateBufferTail->bufPipe == gcvPIPE_2D)
        {
            bPipeSelectNeeded = VK_TRUE;
            bPipeFlushNeeded  = VK_TRUE;
            pipeSelectFlushDMASize = __VK_PIPE_SELECT_DMA_SIZE + __VK_2D_FLUSH_PIPE_DMA_SIZE;
        }
        else if (cmd->stateBufferTail->bufPipe == gcvPIPE_3D)
        {
            bPipeSelectNeeded = VK_TRUE;
            bPipeFlushNeeded  = VK_TRUE;
            pipeSelectFlushDMASize = __VK_PIPE_SELECT_DMA_SIZE + __VK_3D_FLUSH_PIPE_DMA_SIZE;
        }
        else
        {
            bPipeSelectNeeded = VK_TRUE;
            pipeSelectFlushDMASize = __VK_PIPE_SELECT_DMA_SIZE;
        }
    }

    /* Allocate first state buffers memroy when first requested */
    if (cmd->stateBufferTail->bufSize == 0)
    {
        cmd->stateBufferTail->bufStart = __vk_AllocateStateBuffer(cmd->commandPool);
        if (!cmd->stateBufferTail->bufStart)
        {
            __VK_ASSERT(VK_FALSE);
            return;
        }
        cmd->stateBufferTail->bufSize = cdp->sizeOfEachStateBuffer;
        cmd->stateBufferTail->bufOffset = 0;
    }

    bytesAvailable = cmd->stateBufferTail->bufSize - cmd->stateBufferTail->bufOffset - pipeSelectFlushDMASize;
    if ((count * sizeof(uint32_t)) > bytesAvailable)
    {
        __vkStateBuffer *temp = cmd->stateBufferTail->next;

        /* Allocate new state buffer if needed */
        if (!temp)
        {
            temp = (__vkStateBuffer *)__VK_ALLOC(sizeof(__vkStateBuffer), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
            if (!temp)
            {
                __VK_ASSERT(VK_FALSE);
                return;
            }

            temp->bufPipe = cmd->stateBufferTail->bufPipe;
            cmd->stateBufferTail->next = temp;
            cmd->stateBufferTail = temp;

            cmd->stateBufferTail->bufStart = __vk_AllocateStateBuffer(cmd->commandPool);
            if (!cmd->stateBufferTail->bufStart)
            {
                __VK_ASSERT(VK_FALSE);
                return;
            }
            cmd->stateBufferTail->bufSize = cdp->sizeOfEachStateBuffer;
            cmd->stateBufferTail->bufOffset = 0;
            cmd->stateBufferTail->secBufCount = 0;
            cmd->stateBufferTail->next = gcvNULL;
        }
        else
        {
            /* If the comand buffer was reset the tail is reset to the head of the list so just move to the next */
            cmd->stateBufferTail = temp;
        }
        cmd->lastStateBufferIndex++;
    }

    if (bPipeFlushNeeded)
    {
        __vk_CmdPipeFlush(commandBuffer, VK_TRUE);
    }

    if (bPipeSelectNeeded)
    {
        __vk_CmdPipeSelect(commandBuffer, pipe);
    }

    *cmdBuffer = (uint32_t *)(cmd->stateBufferTail->bufStart + cmd->stateBufferTail->bufOffset);

    cmd->bufWriteRequestCount = count;
}

void __vk_CmdReleaseBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t count
    )
{
    __vkCommandBuffer *cmd = (__vkCommandBuffer *)commandBuffer;

    /* Invalid call if no current request pending */
    __VK_ASSERT(cmd->bufWriteRequestCount);

    /* Count should be less than or equal to current request */
    __VK_ASSERT(count <= cmd->bufWriteRequestCount);

    cmd->stateBufferTail->bufOffset += count * sizeof(uint32_t);
    cmd->bufWriteRequestCount = 0;
}

void __vk_CmdAquireBufferAndLoadHWStates(
    VkCommandBuffer commandBuffer,
    uint32_t address,
    VkBool32 fixedPoint,
    uint32_t count,
    void *data
    )
{
    uint32_t requestSize;
    uint32_t *states;

    /* Align States request to a multiple of 2 dwords for command alignment. */
    requestSize = ((count + 2) / 2) * 2;

    __vk_CmdAquireBuffer(commandBuffer, requestSize, &states);

    __vkCmdLoadBatchHWStates(&states, address, fixedPoint, count, data);

    __vk_CmdReleaseBuffer(commandBuffer, requestSize);
}

__vkScratchMem* __vkGetScratchMem(
    __vkCommandBuffer *cmdBuf,
    VkDeviceSize size
    )
{
    __vkCommandPool *cdp = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkCommandPool *, cmdBuf->commandPool);
    __vkScratchMem *scratch = gcvNULL;
    VkDeviceMemory vkDevMem = VK_NULL_HANDLE;
    VkMemoryAllocateInfo allocInfo =
    {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        gcvNULL,
        size,
        0
    };
    VkResult result = VK_SUCCESS;
    __VK_SET_ALLOCATIONCB(&cdp->memCb);

    __VK_ONERROR(__vk_AllocateMemory((VkDevice)cmdBuf->devCtx, &allocInfo, VK_NULL_HANDLE, &vkDevMem));

    /* Allocate scratch gpu memory and copy host data to it first */
    scratch = (__vkScratchMem *)__VK_ALLOC(sizeof(__vkScratchMem), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    __VK_ONERROR(scratch ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY);

    scratch->next = cmdBuf->scratchHead;
    cmdBuf->scratchHead = scratch;

    scratch->memory = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDeviceMemory*, vkDevMem);

    return scratch;

OnError:
    if (vkDevMem)
    {
        __vk_FreeMemory((VkDevice)cmdBuf->devCtx, vkDevMem, VK_NULL_HANDLE);
    }
    return VK_NULL_HANDLE;
}


