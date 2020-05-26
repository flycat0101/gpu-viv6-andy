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

VkResult __vk_SetHwFence(
    VkDevice device,
    uint32_t fenceIndex,
    VkBool32 signaled
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devCtx->fenceBuffer);
    __vkHwFenceData *fenceMemory;
    VkResult result = VK_SUCCESS;

    /* Lock surface for memory and mark as unsignalled */
    __VK_ONERROR(__vk_MapMemory(
        device,
        (VkDeviceMemory)(uintptr_t)fenceBuffer->memory,
        0,
        fenceBuffer->memory->size,
        0,
        (void **)&fenceMemory
        ));

    fenceMemory[fenceIndex].value = signaled ? 1 : 0;

    /* Unlock the memory */
    __vk_UnmapMemory(
        device,
        (VkDeviceMemory)(uintptr_t)fenceBuffer->memory
        );

OnError:

    return result;
}

VkBool32 __vk_GetHwFence(
    VkDevice device,
    uint32_t fenceIndex
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkBuffer *fenceBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, devCtx->fenceBuffer);
    __vkHwFenceData *fenceMemory;
    VkBool32 result;

    /* Lock surface for memory and mark as unsignalled */
    __VK_ONERROR(__vk_MapMemory(
        device,
        (VkDeviceMemory)(uintptr_t)fenceBuffer->memory,
        0,
        fenceBuffer->memory->size,
        0,
        (void **)&fenceMemory
        ));

    result = fenceMemory[fenceIndex].value ? VK_TRUE : VK_FALSE;

    /* Unlock the memory */
    __vk_UnmapMemory(
        device,
        (VkDeviceMemory)(uintptr_t)fenceBuffer->memory
        );

OnError:

    return result;
}

VkResult __vk_AllocateHwFence(
    VkDevice device,
    uint32_t* fenceIndex
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t i, index, shift;
    VkResult result = VK_SUCCESS;

    gcoOS_AcquireMutex(gcvNULL, devCtx->fenceMutex, gcvINFINITE);
    for (i = 0; i < __VK_MAX_FENCE_COUNT; i++)
    {
        if (++devCtx->lastFenceIndex >= __VK_MAX_FENCE_COUNT)
            devCtx->lastFenceIndex = 0;

        index = devCtx->lastFenceIndex / 32;
        shift = devCtx->lastFenceIndex % 32;

        if (!(devCtx->fenceInUse[index] & (1 << shift)))
        {
            /* Set as unsignalled */
           *fenceIndex = devCtx->lastFenceIndex;
            __VK_ONERROR(__vk_SetHwFence(device, *fenceIndex, VK_FALSE));
            devCtx->fenceInUse[index] |= (1 << shift);
            devCtx->fenceCount++;
            break;
        }
    }

OnError:
    gcoOS_ReleaseMutex(gcvNULL, devCtx->fenceMutex);

    if (i >= __VK_MAX_FENCE_COUNT)
        result = VK_ERROR_OUT_OF_DEVICE_MEMORY;

    return result;
}

void __vk_FreeHwFence(
    VkDevice device,
    uint32_t fenceIndex
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    uint32_t index, shift;

    index = fenceIndex / 32;
    shift = fenceIndex % 32;

    gcoOS_AcquireMutex(gcvNULL, devCtx->fenceMutex, gcvINFINITE);
    devCtx->fenceInUse[index] &= ~(1 << shift);
    devCtx->fenceCount--;
    gcoOS_ReleaseMutex(gcvNULL, devCtx->fenceMutex);
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateFence(
    VkDevice device,
    const VkFenceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkFence* pFence
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFence *fce;
    VkResult result = VK_SUCCESS;

    *pFence = VK_NULL_HANDLE;

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_FENCE, sizeof(__vkFence), (__vkObject**)&fce));

    /* Initialize __vkFence specific data fields here */
    __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_TRUE, &fce->signal));

    if (pCreateInfo->flags & VK_FENCE_CREATE_SIGNALED_BIT)
    {
        __VK_ONERROR(gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE));
    }

    if (pCreateInfo->pNext != VK_NULL_HANDLE)
    {
         VkExportFenceCreateInfo *exportInfo = (VkExportFenceCreateInfo *)pCreateInfo->pNext;
         __VK_ASSERT(exportInfo->sType == VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO);

         fce->handleType = (VkExternalFenceHandleTypeFlagBits)exportInfo->handleTypes;

        if (fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT ||
            fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT)
        {
            /* Fence's winHandle is Fence's created signal*/
            fce->winHandle = fce->signal;
        }
#if defined(LINUX) || defined(ANDROID)
        if (fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT ||
            fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT)
        {
            /*create the fence's fenceFd*/
            __VK_ONERROR(gcoOS_CreateNativeFence(gcvNULL, fce->signal, &fce->fenceFd));
        }
#endif
    }

    /* Return the object pointer as a 64-bit handle */
    *pFence = (VkFence)(uintptr_t)fce;

    return VK_SUCCESS;

OnError:
    if (fce)
    {
        if (fce->signal)
            gcoOS_DestroySignal(gcvNULL, fce->signal);

        __vk_DestroyObject(devCtx, __VK_OBJECT_FENCE, (__vkObject *)fce);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyFence(
    VkDevice device,
    VkFence fence,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (fence)
    {
        __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);

        if ((fce->signal && !fce->winHandle) ||
            (fce->signal && !fce->exported && !fce->imported))
        {
            gcoOS_DestroySignal(gcvNULL, fce->signal);
        }

        if (fce->permanent)
        {
            gcoOS_DestroySignal(gcvNULL, fce->permanent);
        }

#if defined(LINUX) || defined(ANDROID)
        if (fce->fenceFd && fce->handleType != VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT)
        {
            close(fce->fenceFd);
        }
#endif

        __vk_DestroyObject(devCtx, __VK_OBJECT_FENCE, (__vkObject *)fce);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetFences(
    VkDevice device,
    uint32_t fenceCount,
    const VkFence* pFences
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t i;

    for (i = 0; i < fenceCount; i++)
    {
        __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pFences[i]);

        if (fce->permanent)
        {
#if defined(LINUX) || defined(ANDROID)
            if (fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT)
            {
                if (fce->fenceFd)
                {
                    close(fce->fenceFd);
                }

                gcoOS_DestroySignal(gcvNULL, fce->signal);
            }
#endif
            fce->signal = fce->permanent;
            fce->permanent = VK_NULL_HANDLE;
        }
        else
        {
            __VK_ONERROR(gcoOS_Signal(gcvNULL, fce->signal, VK_FALSE));
        }
    }

OnError:
    return result;
}

VkResult __vkiGetFenceStatus(
    __vkFence *fce,
    uint32_t timeout
    )
{
    return (VkResult)gcoOS_WaitSignal(gcvNULL, fce->signal, timeout);
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetFenceStatus(
    VkDevice device,
    VkFence fence
    )
{
    VkResult result = VK_SUCCESS;
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);

    result = __vkiGetFenceStatus(fce, 0);

    if (result == (VkResult)gcvSTATUS_TIMEOUT)
    {
        result = VK_NOT_READY;
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_WaitForFences(
    VkDevice device,
    uint32_t fenceCount,
    const VkFence* pFences,
    VkBool32 waitAll,
    uint64_t timeout
    )
{
    VkResult result = VK_SUCCESS;
    uint32_t waitTimeout;
    uint32_t iFence;
    VkBool32 loop = VK_TRUE;
    gctUINT64 startTime, currentTime, deltaTime = 0;

    /* The granularity of timeout is nanoseconds, gcoOS_WaitSignal() is milliseconds and gcoOS_GetTime is microseconds */
    if (timeout == 0)
        waitTimeout = 0;
    else
        waitTimeout = gcmMAX(1, (uint32_t)(timeout / 1000000 / fenceCount));

    gcoOS_GetTime(&startTime);
    do
    {
        __vkFence *fce = VK_NULL_HANDLE;

        /* Check status first with timeout of zero */
        for (iFence = 0; iFence < fenceCount; iFence++)
        {
            fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pFences[iFence]);

            /* Get status ALWAYS using __vkiGetFenceStatus() */
            result = __vkiGetFenceStatus(fce, 0);

            /* If timeout is zero return timeout if any are not signaled */
            if ((result == (VkResult)gcvSTATUS_TIMEOUT) && timeout == 0)
            {
                loop = VK_FALSE;
                break;
            }

            /* If not waitAll then return if any are signaled */
            if (!waitAll && (result == VK_SUCCESS))
            {
                loop = VK_FALSE;
                break;
            }
            /* If waitAll break out of inner status loop if any not signaled */
            else if (waitAll && (result != VK_SUCCESS))
            {
                break;
            }
        }

        /* If all signaled than return success */
        if (result == VK_SUCCESS)
        {
            break;
        }
        else
        {
            /* If total time exceeds timeout then return timeout */
            if (deltaTime > timeout)
                break;
        }

        /* Wait on last fence that was not signaled */
        result = (VkResult)gcoOS_WaitSignal(gcvNULL, fce->signal, waitTimeout);

        gcoOS_GetTime(&currentTime);
        deltaTime = (currentTime - startTime) * 1000;   // Convert to nanoseconds
    } while (loop);

    if (result == (VkResult)gcvSTATUS_TIMEOUT)
        result = VK_TIMEOUT;

    return result;
}

VkResult __vk_SetSemaphore(
    VkDevice device,
    VkSemaphore semaphore,
    VkBool32 signaled
    )
{
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);
    VkResult result = VK_SUCCESS;

    result = __vk_SetHwFence(device, sph->fenceIndex, signaled);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateSemaphore(
    VkDevice device,
    const VkSemaphoreCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSemaphore* pSemaphore
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkSemaphore *sph = gcvNULL;
    VkResult result = VK_SUCCESS;

    result = __vk_CreateObject(devCtx, __VK_OBJECT_SEMAPHORE,
        sizeof(__vkSemaphore), (__vkObject**)&sph);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    result = __vk_AllocateHwFence(device, &sph->fenceIndex);
    sph->fenceFd = -1;
    sph->sphSignal = VK_NULL_HANDLE;

    if (result == VK_SUCCESS)
    {
        *pSemaphore = (VkSemaphore)(uintptr_t)sph;
    }
    else
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_SEMAPHORE, (__vkObject*)sph);
    }

    if (pCreateInfo->pNext != VK_NULL_HANDLE)
    {
         VkExportSemaphoreCreateInfo *exportInfo = (VkExportSemaphoreCreateInfo *)pCreateInfo->pNext;
         __VK_ASSERT(exportInfo->sType == VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO);

         sph->handleType = (VkExternalSemaphoreHandleTypeFlagBits)exportInfo->handleTypes;

        if (sph->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT ||
            sph->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT)
        {
            /* create semaphore's windows handle with signal*/
            __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_TRUE, &sph->winHandle));
        }
#if defined(LINUX) || defined(ANDROID)
        if (sph->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT ||
            sph->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT)
        {
            __vkSemaphoreNode *sphNode;

            __VK_SET_ALLOCATIONCB(&devCtx->memCb);

            /*use the fenceFd to represent semaphore's fd*/
            __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_TRUE, &sph->sphSignal));
            __VK_ONERROR(gcoOS_CreateNativeFence(gcvNULL, sph->sphSignal, &sph->fenceFd));

            sphNode = (__vkSemaphoreNode *)__VK_ALLOC(sizeof(__vkSemaphoreNode), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
            if (!sphNode)
            {
                return VK_ERROR_OUT_OF_HOST_MEMORY;
            }
            __VK_MEMZERO(sphNode, sizeof(__vkSemaphoreNode));

            sphNode->sphPointer = sph;
            sphNode->sphNext = VK_NULL_HANDLE;

            if (devCtx->currentSph == VK_NULL_HANDLE)
            {
                __VK_ASSERT(devCtx->sphList == VK_NULL_HANDLE);
                devCtx->currentSph = sphNode;
                devCtx->sphList    = sphNode;
            }
            else
            {
                devCtx->currentSph->sphNext = sphNode;
                devCtx->currentSph = sphNode;
            }
        }
#endif
    }

    return result;
 OnError:
    return VK_INCOMPLETE;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroySemaphore(
    VkDevice device,
    VkSemaphore semaphore,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (semaphore)
    {
        __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);

        __vk_FreeHwFence(device, sph->fenceIndex);

        if (!sph->sphSignal)
        {
#if defined(LINUX) || defined(ANDROID)
            if (sph->fenceFd >= 0)
            {
                close(sph->fenceFd);
            }
#endif
            __vk_DestroyObject(devCtx, __VK_OBJECT_SEMAPHORE, (__vkObject *)sph);
        }
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateEvent(
    VkDevice device,
    const VkEventCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkEvent* pEvent
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkEvent *evt;
    VkResult result;

    result = __vk_CreateObject(devCtx, __VK_OBJECT_EVENT, sizeof(__vkEvent), (__vkObject**)&evt);
    if (result != VK_SUCCESS)
    {
        return result;
    }

    result = __vk_AllocateHwFence(device, &evt->fenceIndex);

    if (result == VK_SUCCESS)
    {
        *pEvent = (VkEvent)(uintptr_t)evt;
    }
    else
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_EVENT, (__vkObject*)evt);
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL __vk_DestroyEvent(
    VkDevice device,
    VkEvent event,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    if (event)
    {
        __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
        __vk_FreeHwFence(device, evt->fenceIndex);
        __vk_DestroyObject(devCtx, __VK_OBJECT_EVENT, (__vkObject *)evt);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetEventStatus(
    VkDevice device,
    VkEvent event
    )
{
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result;

    if(__vk_GetHwFence(device, evt->fenceIndex))
    {
        result = VK_EVENT_SET;
    }
    else
    {
        result = VK_EVENT_RESET;
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_SetEvent(
    VkDevice device,
    VkEvent event
    )
{
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result;

    result = __vk_SetHwFence(device, evt->fenceIndex, VK_TRUE);

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_ResetEvent(
    VkDevice device,
    VkEvent event
    )
{
    __vkEvent *evt = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkEvent *, event);
    VkResult result;

    result = __vk_SetHwFence(device, evt->fenceIndex, VK_FALSE);

    return result;
}

VkResult VKAPI_CALL __vk_GetSemaphoreFdKHR(
    VkDevice device,
    const VkSemaphoreGetFdInfoKHR* pGetFdInfo,
    int* pFd
    )
{
    VkResult result = VK_SUCCESS;

#if defined(LINUX) || defined(ANDROID)
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pGetFdInfo->semaphore);

    if (sph->fenceFd >= 0)
    {
        int tempFd;
        tempFd = dup(sph->fenceFd);

        if (tempFd < 0)
        {
            result = VK_INCOMPLETE;
        }

        *pFd = tempFd;

        return result;
    }
    else
    {
        result = VK_INCOMPLETE;
    }
#endif

   return result;

}

VkResult VKAPI_CALL __vk_ImportSemaphoreFdKHR(
    VkDevice device,
    const VkImportSemaphoreFdInfoKHR* pImportSemaphoreFdInfo
    )
{
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pImportSemaphoreFdInfo->semaphore);
    VkResult result = VK_SUCCESS;

    sph->fenceFd = pImportSemaphoreFdInfo->fd;
    sph->handleType = pImportSemaphoreFdInfo->handleType;
    sph->payloadImported = VK_TRUE;

    return result;
}

VkResult VKAPI_CALL __vk_GetFenceFdKHR(
    VkDevice device,
    const VkFenceGetFdInfoKHR* pGetFdInfo,
    int* pFd
    )
{
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pGetFdInfo->fence);
    VkResult result = VK_SUCCESS;

    __VK_ASSERT(fce->handleType == pGetFdInfo->handleType);
    __VK_ASSERT(fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT);

    if (fce->fenceFd != VK_NULL_HANDLE)
    {
        *pFd = fce->fenceFd;
        fce->exported = VK_TRUE;
        return result;
    }
    else
    {
        result = VK_INCOMPLETE;
    }

   return result;

}

VkResult VKAPI_CALL __vk_ImportFenceFdKHR(
    VkDevice device,
    const VkImportFenceFdInfoKHR* pImportFenceFdInfo
    )
{
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pImportFenceFdInfo->fence);
    VkResult result = VK_SUCCESS;

    fce->handleType = pImportFenceFdInfo->handleType;

#if defined(LINUX) || defined(ANDROID)
    if (fce->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT)
    {
        fce->permanent = fce->signal;

        if(fce->fenceFd)
        {
            close(fce->fenceFd);
        }
        __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_TRUE, &fce->signal));
        __VK_ONERROR(gcoOS_CreateNativeFence(gcvNULL, fce->signal, &fce->fenceFd));

        if (!gcoOS_WaitNativeFence(gcvNULL, pImportFenceFdInfo->fd, 0))
        {
            gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);
        }

        close(pImportFenceFdInfo->fd);
    }
#endif

    fce->imported = VK_TRUE;


    return result;

#if defined(LINUX) || defined(ANDROID)
OnError:
    return VK_INCOMPLETE;
#endif
}

#if defined(_WIN32)
VkResult VKAPI_CALL __vk_GetSemaphoreWin32HandleKHR(
    VkDevice device,
    const VkSemaphoreGetWin32HandleInfoKHR* pGetWin32HandleInfo,
    HANDLE* pHandle
    )
{
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pGetWin32HandleInfo->semaphore);
    VkResult result = VK_SUCCESS;

    if (sph->winHandle != VK_NULL_HANDLE)
    {
        *pHandle = sph->winHandle;
        return result;
    }
    else
    {
        result = VK_INCOMPLETE;
    }

    return result;
}

VkResult VKAPI_CALL __vk_ImportSemaphoreWin32HandleKHR(
    VkDevice device,
    const VkImportSemaphoreWin32HandleInfoKHR* pImportSemaphoreWin32HandleInfo
    )
{
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, pImportSemaphoreWin32HandleInfo->semaphore);
    VkResult result = VK_SUCCESS;

    sph->winHandle = pImportSemaphoreWin32HandleInfo->handle;
    sph->handleType = pImportSemaphoreWin32HandleInfo->handleType;

    return result;
}

VkResult VKAPI_CALL __vk_GetFenceWin32HandleKHR(
    VkDevice device,
    const VkFenceGetWin32HandleInfoKHR* pGetWin32HandleInfo,
    HANDLE* pHandle
    )
{
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pGetWin32HandleInfo->fence);
    VkResult result = VK_SUCCESS;

    __VK_ASSERT(fce->handleType == pGetWin32HandleInfo->handleType);

    if (fce->winHandle != VK_NULL_HANDLE)
    {
        *pHandle = fce->winHandle;
        fce->exported = VK_TRUE;
        return result;
    }
    else
    {
        result = VK_INCOMPLETE;
    }

    return result;
}

VkResult VKAPI_CALL __vk_ImportFenceWin32HandleKHR(
    VkDevice device,
    const VkImportFenceWin32HandleInfoKHR* pImportFenceWin32HandleInfo
    )
{
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, pImportFenceWin32HandleInfo->fence);
    VkResult result = VK_SUCCESS;

    fce->winHandle = pImportFenceWin32HandleInfo->handle;
    fce->handleType = pImportFenceWin32HandleInfo->handleType;

    if (pImportFenceWin32HandleInfo->flags == VK_FENCE_IMPORT_TEMPORARY_BIT)
    {
        fce->permanent = fce->signal;
    }
    else
    {
        if (!fce->imported && !fce->exported)
        {
            gcoOS_DestroySignal(gcvNULL, fce->signal);
        }
        fce->permanent = VK_NULL_HANDLE;
    }

    fce->signal = fce->winHandle;
    fce->imported = VK_TRUE;

    return result;
}
#endif



