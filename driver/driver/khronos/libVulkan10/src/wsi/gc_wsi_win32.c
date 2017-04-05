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

#ifdef VK_USE_PLATFORM_WIN32_KHR

typedef struct __vkWin32SwapchainKHRRec     __vkWin32SwapchainKHR;
typedef struct __vkWin32ImageBufferRec      __vkWin32ImageBuffer;


typedef enum __vkSwapChainStateRec
{
    __VK_SWAP_CHAIN_STATE_VALID,
    __VK_SWAP_CHAIN_STATE_OUT_OF_DATE,
    __VK_SWAP_CHAIN_STATE_OLD
} __vkSwapChainState;

typedef struct __vkPresentQueueEntryRec
{
    uint32_t                        imageIndex;
    VkBool32                        persistent;
    VkRect2D                        srcRect;
    VkRect2D                        dstRect;
    VkRect2D *                      pSrcRect;
    VkRect2D *                      pDstRect;
} __vkPresentQueueEntry;

typedef struct __vkPresentQueueRec
{
    __vkPresentQueueEntry *         entry;
    uint32_t                        numberofEntries;

    /* Signaled when numberofEntries reduced. */
    gctSIGNAL                       queueUpdate;

    gctSIGNAL                       threadStart;
    volatile VkBool32               threadStop;
} __vkPresentQueue;


struct __vkWin32SwapchainKHRRec
{
    __vkSwapchainKHR                base;

    /* Swapchain specific fields */
    VkDevice                        device;
    VkIcdSurfaceWin32 *             surface;

    uint32_t                        minImageCount;
    VkFormat                        imageFormat;
    VkColorSpaceKHR                 imageColorSpace;
    VkExtent2D                      imageExtent;
    uint32_t                        imageArrayLayers;
    VkImageUsageFlags               imageUsage;
    VkSharingMode                   imageSharingMode;
    VkCompositeAlphaFlagBitsKHR     compositeAlpha;
    VkSurfaceTransformFlagBitsKHR   preTransform;
    VkPresentModeKHR                presentMode;
    VkBool32                        clipped;

    __vkWin32ImageBuffer *          imageBuffers;
    uint32_t                        imageCount;

    __vkSwapChainState              state;

    /* Signaled when any imageBuffer presented. */
    gctSIGNAL                       signal;

    /* Each swap chain has its own present thread */
    gctPOINTER                      presentThread;
    __vkPresentQueue                presentQueue;
    gctPOINTER                      presentMutex;

    VkCommandPool                   cmdPool;
    VkCommandBuffer                 cmdBuf;
};

typedef enum __vkWin32ImageStateRec
{
    __VK_SC_IMAGE_STATE_FREE,
    __VK_SC_IMAGE_STATE_APP,
    __VK_SC_IMAGE_STATE_HW,
}
__vkWin32ImageState;

struct __vkWin32ImageBufferRec
{
    __vkWin32SwapchainKHR *         swapchain;

    VkImage                         image;

    VkBuffer                        buffer;

    VkFence                         fence;
    VkSemaphore                     semaphore;
    gctSIGNAL                       signal;

    __vkWin32ImageState             state;
};

typedef struct __BITFIELDINFO
{
    BITMAPINFO                      bmi;
    RGBQUAD                         bmiColors[2];
}
BITFIELDINFO;

static VkResult __GetWin32SurfaceExtent(
    VkIcdSurfaceWin32 * surf,
    VkExtent2D *pExtent
    )
{
    RECT rect;

    if (!GetClientRect(surf->hwnd, &rect))
    {
        return VK_ERROR_SURFACE_LOST_KHR;
    }

    pExtent->width  = rect.right  - rect.left;
    pExtent->height = rect.bottom - rect.top;

    return VK_SUCCESS;
}

static VkResult __ValidateWin32SwapChain(
    VkIcdSurfaceWin32 * surf,
    VkExtent2D *pExtent
    )
{
    VkResult result;
    VkExtent2D extent;

    result = __GetWin32SurfaceExtent(surf, &extent);

    if (result != VK_SUCCESS)
    {
        return result;
    }

    if (pExtent)
    {
        if ((pExtent->height != extent.height) || (pExtent->width != extent.width))
            result = VK_ERROR_OUT_OF_DATE_KHR;
    }

    return result;
}

VkResult __vki_DisplayWin32SwapChainImage(
    VkIcdSurfaceWin32 * surf,
    __vkWin32SwapchainKHR * sc,
    uint32_t imageIndex,
    VkRect2D *srcRect,
    VkRect2D *dstRect
    )
{
    __vkImage *img = (__vkImage *)sc->imageBuffers[imageIndex].image;
    __vkBuffer *buf = (__vkBuffer *)sc->imageBuffers[imageIndex].buffer;
    const __vkFormatInfo *formatInfo = VK_NULL_HANDLE;
    gctPOINTER memory = gcvNULL;
    BITFIELDINFO bfi;
    PBITMAPINFOHEADER info = &bfi.bmi.bmiHeader;
    uint32_t *mask = (uint32_t *)(info + 1);
    HDC hdc;
    int32_t srcX, srcY, srcWidth, srcHeight;
    int32_t dstX, dstY, dstWidth, dstHeight;
    uint32_t count;
    RECT rect;
    VkExtent2D extent;
    VkResult result = VK_INCOMPLETE;

    /* Gather source information */
    formatInfo = &img->formatInfo;

    if (GetClientRect(surf->hwnd, &rect))
    {
        extent.width  = rect.right  - rect.left;
        extent.height = rect.bottom - rect.top;
    }
    else
    {
        return VK_ERROR_SURFACE_LOST_KHR;
    }

    /* Lock the resolve buffer memory */
    __VK_ONERROR(__vk_MapMemory(sc->device, (VkDeviceMemory)buf->memory, 0, buf->memReq.size, 0, &memory));

    /* Fill in the bitmap information */
    info->biSize          = sizeof(bfi.bmi.bmiHeader);
    info->biWidth         = img->pImgLevels[0].alignedW;
    info->biHeight        = -(int32_t)img->pImgLevels[0].alignedH;
    info->biPlanes        = 1;
    info->biBitCount      = (WORD)formatInfo->bitsPerBlock;
    info->biCompression   = BI_BITFIELDS;
    info->biSizeImage     = ((extent.width * formatInfo->bitsPerBlock / 8 + 3) & ~3) * extent.height;
    info->biXPelsPerMeter = 0;
    info->biYPelsPerMeter = 0;
    info->biClrUsed       = 0;
    info->biClrImportant  = 0;

    switch (formatInfo->residentImgFormat)
    {
    case VK_FORMAT_B8G8R8A8_UNORM:
        mask[0] = 0x00FF0000;
        mask[1] = 0x0000FF00;
        mask[2] = 0x000000FF;
        break;

    case __VK_FORMAT_A4R4G4B4_UNFORM_PACK16:
        mask[0] = 0x00000F00;
        mask[1] = 0x000000F0;
        mask[2] = 0x0000000F;
        break;

    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        mask[0] = 0x0000F800;
        mask[1] = 0x000007E0;
        mask[2] = 0x0000001F;
        break;

    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        mask[0] = 0x00007C00;
        mask[1] = 0x000003E0;
        mask[2] = 0x0000001F;
        break;

    default:
        return result;
    }

    srcX      = dstX      = rect.left;
    srcY      = dstY      = rect.top;
    srcWidth  = dstWidth  = extent.width;
    srcHeight = dstHeight = extent.height;

    if (srcRect)
    {
        srcX      = srcRect->offset.x;
        srcY      = srcRect->offset.y;
        srcWidth  = srcRect->extent.width;
        srcHeight = srcRect->extent.height;
    }

    if (dstRect)
    {
        dstX      = srcRect->offset.x;
        dstY      = srcRect->offset.y;
        dstWidth  = srcRect->extent.width;
        dstHeight = srcRect->extent.height;
    }

    hdc = GetDC(surf->hwnd);

    if (hdc)
    {
        /* Draw bitmap bits to window. */
        count = StretchDIBits(
            hdc,
            dstX, dstY, dstWidth, dstHeight,
            srcX, srcY, srcWidth, srcHeight,
            memory,
            (BITMAPINFO *) info,
            DIB_RGB_COLORS,
            SRCCOPY);

        ReleaseDC(surf->hwnd, hdc);

        result = VK_SUCCESS;
    }

    /* Unlock the resolve buffer memory */
    __vk_UnmapMemory(sc->device, (VkDeviceMemory)buf->memory);

OnError:
    return result;
}


VKAPI_ATTR VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex
    )
{
    /* Always return true */
    return VK_TRUE;
}

/* __vkSurfaceOperation::DestroySurface. */
static void win32DestroySurface(
    VkInstance  instance,
    VkSurfaceKHR  surface,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceWin32 *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWin32 *, surface);

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    __VK_FREE(surf);
}

static const VkFormat wsiSupportedPresentFormats[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_B5G6R5_UNORM_PACK16,
};

static VkResult __vki_PresentSwapChainImage(
    VkQueue queue,
    VkSwapchainKHR swapchain,
    uint32_t imageIndex,
    VkRect2D *srcRect,
    VkRect2D *dstRect
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    __vkDevContext *devCtx = (__vkDevContext *)sc->device;
    __vkImage *pSrcImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, sc->imageBuffers[imageIndex].image);
    VkCommandBufferBeginInfo cbi;
    VkSubmitInfo si;
    __vkBlitRes srcRes, dstRes;
    uint32_t i, bytePerPixel, sliceBytes;

    VkResult result = VK_SUCCESS;

    bytePerPixel = g_vkFormatInfoTable[pSrcImg->createInfo.format].bitsPerBlock / 8;

    __VK_MEMZERO(&srcRes, sizeof(srcRes));
    srcRes.isImage = VK_TRUE;
    srcRes.u.img.pImage = pSrcImg;
    srcRes.u.img.subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcRes.u.img.extent = pSrcImg->createInfo.extent;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage = VK_FALSE;
    dstRes.u.buf.pBuffer = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, sc->imageBuffers[imageIndex].buffer);
    dstRes.u.buf.rowLength = pSrcImg->pImgLevels[0].alignedW;
    dstRes.u.buf.imgHeight = pSrcImg->pImgLevels[0].alignedH;
    if (dstRect)
    {
        dstRes.u.buf.offset = (dstRect->offset.x + dstRect->offset.y * dstRes.u.buf.rowLength) * bytePerPixel;
    }
    sliceBytes = dstRes.u.buf.rowLength * dstRes.u.buf.imgHeight * bytePerPixel;

    __VK_MEMZERO(&cbi, sizeof(VkCommandBufferBeginInfo));
    cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    __VK_ONERROR(__vk_BeginCommandBuffer(sc->cmdBuf, &cbi));

    for (i = 0; i < sc->imageArrayLayers; i++)
    {
        __VK_ONERROR(devCtx->chipFuncs->CopyImage(
            sc->cmdBuf,
            &srcRes,
            &dstRes,
            VK_FALSE
            ));

        srcRes.u.img.subRes.arrayLayer++;
        dstRes.u.buf.offset += sliceBytes;
    }

    __VK_ONERROR(__vk_EndCommandBuffer(sc->cmdBuf));

#if __VK_NEW_DEVICE_QUEUE
    {
        gcsHAL_INTERFACE iface;

        iface.command             = gcvHAL_SIGNAL;
        iface.u.Signal.signal     = gcmPTR_TO_UINT64(sc->imageBuffers[imageIndex].signal);
        iface.u.Signal.auxSignal  = 0;
        iface.u.Signal.process    = gcmPTR2INT32(gcoOS_GetCurrentProcessID());
        iface.u.Signal.fromWhere  = gcvKERNEL_PIXEL;
        __VK_ONERROR(__vk_QueueAppendEvent((__vkDevQueue *)queue, &iface));

        iface.command             = gcvHAL_SIGNAL;
        iface.u.Signal.signal     = gcmPTR_TO_UINT64(sc->presentQueue.threadStart);
        iface.u.Signal.auxSignal  = 0;
        iface.u.Signal.process    = gcmPTR2INT32(gcoOS_GetCurrentProcessID());
        iface.u.Signal.fromWhere  = gcvKERNEL_PIXEL;
        __VK_ONERROR(__vk_QueueAppendEvent((__vkDevQueue *)queue, &iface));
    }
#else
    /* Signal buffer swap complete */
    __VK_ONERROR(gcoHAL_ScheduleSignal(
        sc->imageBuffers[imageIndex].signal,
        gcvNULL,
        gcmPTR2INT32(gcoOS_GetCurrentProcessID()),
        gcvKERNEL_PIXEL
        ));

    /* Signal to wake up thread */
    __VK_ONERROR(gcoHAL_ScheduleSignal(
        sc->presentQueue.threadStart,
        gcvNULL,
        gcmPTR2INT32(gcoOS_GetCurrentProcessID()),
        gcvKERNEL_PIXEL
        ));
#endif

    __VK_MEMZERO(&si, sizeof(VkSubmitInfo));
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &sc->cmdBuf;

    __VK_ONERROR(__vk_QueueSubmit(
        queue,
        1,
        &si,
        VK_NULL_HANDLE
        ));

     gcmDUMP(gcvNULL,
            "@[swap 0x%08X %dx%d +%u]",
            dstRes.u.buf.pBuffer->memory->devAddr,
            pSrcImg->pImgLevels[0].alignedW,
            pSrcImg->pImgLevels[0].alignedH,
            pSrcImg->pImgLevels[0].stride);

OnError:
    return result;
}

VkResult __vki_scPresentThread(
    VkSwapchainKHR swapchain
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    VkResult result = VK_SUCCESS;
    uint32_t i;

    while (!sc->presentQueue.threadStop)
    {
        i = 0;
        gcoOS_AcquireMutex(gcvNULL, sc->presentMutex, gcvINFINITE);

        while (i < sc->presentQueue.numberofEntries)
        {
            uint32_t ii = sc->presentQueue.entry[i].imageIndex;
            __vkPresentQueueEntry *entry = &sc->presentQueue.entry[i++];

            if (sc->imageBuffers[ii].state != __VK_SC_IMAGE_STATE_HW)
                continue;

            if (gcmIS_SUCCESS(gcoOS_WaitSignal(gcvNULL, sc->imageBuffers[ii].signal, 0)))
            {
                __vki_DisplayWin32SwapChainImage(
                    sc->surface,
                    sc,
                    ii,
                    entry->pSrcRect,
                    entry->pDstRect
                    );

                /*
                 * Set to free first, and will be updated to 'APP' if
                 * already reserved through fence or semaphore.
                 */
                sc->imageBuffers[ii].state = __VK_SC_IMAGE_STATE_FREE;

                /* Signal fence if one is used */
                if (sc->imageBuffers[ii].fence)
                {
                    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, sc->imageBuffers[ii].fence);

                    gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);

                    sc->imageBuffers[ii].fence = VK_NULL_HANDLE;
                    sc->imageBuffers[ii].state = __VK_SC_IMAGE_STATE_APP;
                }

                /* Signal semaphore if one is used */
                if (sc->imageBuffers[ii].semaphore)
                {
                    __vk_SetSemaphore(sc->device, sc->imageBuffers[ii].semaphore, VK_TRUE);

                    sc->imageBuffers[ii].semaphore = VK_NULL_HANDLE;
                    sc->imageBuffers[ii].state     = __VK_SC_IMAGE_STATE_APP;
                }

                /* Should never happen */
                __VK_ASSERT(sc->presentQueue.numberofEntries);

                if (--sc->presentQueue.numberofEntries > 0)
                {
                    uint32_t j;

                    for (j = 0; j < sc->presentQueue.numberofEntries; j++)
                    {
                        sc->presentQueue.entry[j] = sc->presentQueue.entry[j+1];
                    }
                }

                i = 0;
                gcoOS_Signal(gcvNULL, sc->presentQueue.queueUpdate, VK_TRUE);
            }
        }

        gcoOS_ReleaseMutex(gcvNULL, sc->presentMutex);

        gcoOS_Signal(gcvNULL, sc->signal, VK_TRUE);

        if (gcmIS_ERROR(gcoOS_WaitSignal(gcvNULL, sc->presentQueue.threadStart, gcvINFINITE)))
            break;
    }

    return result;
}

static VkResult __CreateSwapchainImageBuffer(
    VkSwapchainKHR  swapchain,
    __vkWin32ImageBuffer *imageBuffer
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    VkImageCreateInfo img_info;
    VkBufferCreateInfo buf_info;
    VkMemoryAllocateInfo mem_alloc;
    VkDeviceMemory memory;
    VkResult result = VK_SUCCESS;

    imageBuffer->swapchain = sc;

    /* Create the swap chain render target images */
    __VK_MEMZERO(&img_info, sizeof(VkImageCreateInfo));
    img_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    img_info.imageType     = VK_IMAGE_TYPE_2D;
    img_info.format        = sc->imageFormat;
    img_info.extent.width  = sc->imageExtent.width;
    img_info.extent.height = sc->imageExtent.height;
    img_info.extent.depth  = 1;
    img_info.mipLevels     = 1;
    img_info.arrayLayers   = sc->imageArrayLayers;
    img_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    img_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    img_info.usage         = sc->imageUsage;
    img_info.flags         = 0;

    imageBuffer->image = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateImage(sc->device, &img_info, gcvNULL, &imageBuffer->image));

    /* allocate memory */
    __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.allocationSize = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->image))->memReq.size;
    mem_alloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory(sc->device, &mem_alloc, gcvNULL, &memory));

    /* bind memory */
    __VK_ONERROR(__vk_BindImageMemory(sc->device, imageBuffer->image, memory, 0));

    /* Create the swap chain resolve buffers */
    __VK_MEMZERO(&buf_info, sizeof(VkBufferCreateInfo));
    buf_info.sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.usage          = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buf_info.sharingMode    = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.size           = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->image))->pImgLevels[0].size;

    imageBuffer->buffer = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateBuffer(sc->device, &buf_info, gcvNULL, &imageBuffer->buffer));

    /* allocate memory */
    __VK_MEMZERO(&mem_alloc, sizeof(mem_alloc));
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.allocationSize = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, imageBuffer->buffer))->memReq.size;
    mem_alloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory(sc->device, &mem_alloc, gcvNULL, &memory));

    /* bind memory */
    __VK_ONERROR(__vk_BindBufferMemory(sc->device, imageBuffer->buffer, memory, 0));

    /* Signal from present queue thread */
    __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_FALSE, &imageBuffer->signal));

    imageBuffer->state = __VK_SC_IMAGE_STATE_FREE;

    return result;

OnError:

    if (imageBuffer->image)
    {
        __vkImage *img;

        img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->image);

        if (img->memory)
            __vk_FreeMemory(sc->device, (VkDeviceMemory)(uintptr_t)img->memory, gcvNULL);

        __vk_DestroyImage(sc->device, imageBuffer->image, gcvNULL);
    }

    if (imageBuffer->buffer)
    {
        __vkBuffer *buf;

        buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, imageBuffer->buffer);

        if (buf->memory)
            __vk_FreeMemory(sc->device, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);

        __vk_DestroyBuffer(sc->device, imageBuffer->buffer, gcvNULL);
    }

    return result;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceSupport. */
static VkResult win32GetPhysicalDeviceSurfaceSupport(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    *pSupported = VK_FALSE;

    if (queueFamilyIndex <= phyDev->queueFamilyCount)
    {
        *pSupported = phyDev->queuePresentSupported[queueFamilyIndex];
    }

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceCapabilities. */
static VkResult win32GetPhysicalDeviceSurfaceCapabilities(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities
    )
{
    VkExtent2D extent;
    VkIcdSurfaceWin32 *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWin32 *, surface);
    VkResult result = VK_SUCCESS;

    __VK_ONERROR(__GetWin32SurfaceExtent(surf, &extent));

    pSurfaceCapabilities->minImageCount           = 2;
    pSurfaceCapabilities->maxImageCount           = 0;
    pSurfaceCapabilities->currentExtent.width     = extent.width;
    pSurfaceCapabilities->currentExtent.height    = extent.height;
    pSurfaceCapabilities->minImageExtent.width    = extent.width;
    pSurfaceCapabilities->minImageExtent.height   = extent.height;
    pSurfaceCapabilities->maxImageExtent.width    = extent.width;
    pSurfaceCapabilities->maxImageExtent.height   = extent.height;
    pSurfaceCapabilities->maxImageArrayLayers     = 1;
    pSurfaceCapabilities->supportedTransforms     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->currentTransform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    pSurfaceCapabilities->supportedUsageFlags     = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

OnError:

    return result;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceFormats. */
static VkResult win32GetPhysicalDeviceSurfaceFormats(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats
    )
{
    VkResult result = VK_SUCCESS;

    if (pSurfaceFormats)
    {
        uint32_t i;

        if (*pSurfaceFormatCount > __VK_COUNTOF(wsiSupportedPresentFormats))
            *pSurfaceFormatCount = __VK_COUNTOF(wsiSupportedPresentFormats);

        for (i = 0; i < *pSurfaceFormatCount; i++)
        {
            pSurfaceFormats[i].format = wsiSupportedPresentFormats[i];
            pSurfaceFormats[i].colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }

        if (*pSurfaceFormatCount < __VK_COUNTOF(wsiSupportedPresentFormats))
            return VK_INCOMPLETE;
    }
    else
    {
        *pSurfaceFormatCount = __VK_COUNTOF(wsiSupportedPresentFormats);
    }

    return result;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfacePresentModes. */
static VkResult win32GetPhysicalDeviceSurfacePresentModes(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes
    )
{
    static VkPresentModeKHR presentModes[] =
    {
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
    };

    if (pPresentModes)
    {
        uint32_t i;

        if (*pPresentModeCount > __VK_COUNTOF(presentModes))
            *pPresentModeCount = __VK_COUNTOF(presentModes);

        for (i = 0; i < *pPresentModeCount; i++)
        {
            pPresentModes[i] = presentModes[i];
        }

        if (*pPresentModeCount < __VK_COUNTOF(presentModes))
            return VK_INCOMPLETE;
    }
    else
    {
        *pPresentModeCount = __VK_COUNTOF(presentModes);
    }

    return VK_SUCCESS;
}

static VkResult __DestroySwapChainImageBuffer(
    __vkWin32ImageBuffer *imageBuffer
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, imageBuffer->swapchain);
    VkResult result = VK_SUCCESS;

    if (imageBuffer->image)
    {
        __vkImage *img;

        img = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->image);

        if (img->memory)
            __vk_FreeMemory(sc->device, (VkDeviceMemory)(uintptr_t)img->memory, gcvNULL);

        __vk_DestroyImage(sc->device, imageBuffer->image, gcvNULL);
    }

    if (imageBuffer->buffer)
    {
        __vkBuffer *buf;

        buf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer *, imageBuffer->buffer);

        if (buf->memory)
            __vk_FreeMemory(sc->device, (VkDeviceMemory)(uintptr_t)buf->memory, gcvNULL);

        __vk_DestroyBuffer(sc->device, imageBuffer->buffer, gcvNULL);
    }

    if (imageBuffer->signal)
        gcoOS_DestroySignal(gcvNULL, imageBuffer->signal);

    return result;
}

/* __vkSwapchainKHR::DestroySwapchain. */
static void __DestroySwapchain(
    VkDevice  device,
    VkSwapchainKHR  swapchain,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    uint32_t i;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (sc->presentThread)
    {
        sc->presentQueue.threadStop = VK_TRUE;

        gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);
        gcoOS_CloseThread(gcvNULL, sc->presentThread);
    }

    if (sc->imageBuffers)
    {
        for (i = 0; i < sc->imageCount; i++)
        {
            __DestroySwapChainImageBuffer(&sc->imageBuffers[i]);
        }
        __VK_FREE(sc->imageBuffers);
    }

    if (sc->cmdPool)
        __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

    if (sc->presentQueue.entry)
        __VK_FREE(sc->presentQueue.entry);

    gcoOS_DestroySignal(gcvNULL, sc->signal);

    gcoOS_DestroySignal(gcvNULL, sc->presentQueue.threadStart);

    if (sc->presentQueue.queueUpdate)
        gcoOS_DestroySignal(gcvNULL, sc->presentQueue.queueUpdate);

    /* Don't destroy the object when called to destro an "old" swapchain from __vk_CreateSwapchainKHR() */
    if (sc->state == __VK_SWAP_CHAIN_STATE_OLD)
    {
        __VK_MEMZERO(sc, sizeof(__vkWin32SwapchainKHR));
        sc->state = __VK_SWAP_CHAIN_STATE_OUT_OF_DATE;
    }
    else
    {
        __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
    }
}

/* __vkSwapchainKHR::GetSwapchainImages. */
static VkResult __GetSwapchainImages(
    VkDevice  device,
    VkSwapchainKHR  swapchain,
    uint32_t*  pSwapchainImageCount,
    VkImage*  pSwapchainImages
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    VkResult result = VK_SUCCESS;

    if (pSwapchainImages)
    {
        uint32_t i;

        if (*pSwapchainImageCount > sc->imageCount)
            *pSwapchainImageCount = sc->imageCount;

        for (i = 0; i < *pSwapchainImageCount; i++)
        {
            pSwapchainImages[i] = sc->imageBuffers[i].image;
        }

        if (*pSwapchainImageCount < sc->imageCount)
            result = VK_INCOMPLETE;
    }
    else
    {
        *pSwapchainImageCount = sc->imageCount;
    }

    return result;
}

/* __vkSwapchainKHR::AcquireNextImage. */
static VkResult __AcquireNextImage(
    VkDevice  device,
    VkSwapchainKHR  swapchain,
    uint64_t  timeout,
    VkSemaphore  semaphore,
    VkFence  fence,
    uint32_t*  pImageIndex
    )
{
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);
    __vkSemaphore *sph = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkSemaphore *, semaphore);
    __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
    gceSTATUS status;
    uint32_t waitTimeout;
    VkResult result = VK_SUCCESS;

    if (sc->state != __VK_SWAP_CHAIN_STATE_VALID)
        result = VK_ERROR_OUT_OF_DATE_KHR;
    else
        result = __ValidateWin32SwapChain(sc->surface, &sc->imageExtent);

    if (!__VK_IS_SUCCESS(result))
    {
        if (sc->state == __VK_SWAP_CHAIN_STATE_VALID)
            sc->state = __VK_SWAP_CHAIN_STATE_OUT_OF_DATE;

        return result;
    }

    /* Start worker thread */
    gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);

    /* The granularity of timeout is nanoseconds, gcoOS_WaitSignal() is millisecods and gcoOS_GetTime is milliseconds */
    if (timeout == 0)
        waitTimeout = 0;
    else if (timeout == UINT64_MAX)
        waitTimeout = gcvINFINITE;
    else
        waitTimeout = gcmMAX(1, (uint32_t)(timeout / 1000000));

    do
    {
        uint32_t i;
        gcoOS_AcquireMutex(gcvNULL, sc->presentMutex, gcvINFINITE);

        for (i = 0; i < sc->imageCount; i++)
        {
            if (sc->imageBuffers[i].state == __VK_SC_IMAGE_STATE_FREE)
            {
                /* Found avaiable image without specified timeout. */
                *pImageIndex = i;

                /* Set state to show app has ownwership */
                sc->imageBuffers[i].state = __VK_SC_IMAGE_STATE_APP;

                /* Set the sync objects to signaled state */
                if (fence)
                {
                    gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);
                    sc->imageBuffers[*pImageIndex].fence = VK_NULL_HANDLE;
                }

                if (semaphore)
                {
                    __vk_SetSemaphore(device, semaphore, VK_TRUE);
                    sc->imageBuffers[*pImageIndex].semaphore = VK_NULL_HANDLE;
                }

                gcoOS_ReleaseMutex(gcvNULL, sc->presentMutex);
                return VK_SUCCESS;
            }
        }

        gcoOS_ReleaseMutex(gcvNULL, sc->presentMutex);

        /* Start worker thread */
        gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);

        /* Wait on present event */
        status = gcoOS_WaitSignal (gcvNULL, sc->signal, waitTimeout);
    }
    while (status != gcvSTATUS_TIMEOUT);

    if (waitTimeout == 0)
    {
        /* VK_NOT_READY is returned if timeout is zero and no image was available. */
        result = VK_NOT_READY;
    }
    else
    {
        /*
         * VK_TIMEOUT is returned if timeout is greater than zero and less and UINT64_MAX,
         * and no image becaome available within the time allowed.
         */
        result = VK_TIMEOUT;
    }

    /* If we are using sync objects assign objects to swap chain image */
    if (semaphore || fence)
    {
        uint32_t i;
        uint32_t ii = 0;

        gcoOS_AcquireMutex(gcvNULL, sc->presentMutex, gcvINFINITE);

        for (i = 0; i < sc->imageCount; i++)
        {
            /* Loop through the current present queue entries in order - FIFO */
            ii = sc->presentQueue.entry[i].imageIndex;

            /* Use this entry if free or no fence or semaphore currently assocaiated with this image */
            if ((sc->imageBuffers[ii].state == __VK_SC_IMAGE_STATE_FREE) ||
                ((sc->imageBuffers[ii].state != __VK_SC_IMAGE_STATE_APP) &&
                 (!sc->imageBuffers[ii].fence && !sc->imageBuffers[ii].semaphore)))
            {
                break;
            }
        }

        if (i < sc->imageCount)
        {
            *pImageIndex = ii;

            /* Double check to make sure this image is not available */
            if (sc->imageBuffers[*pImageIndex].state == __VK_SC_IMAGE_STATE_FREE)
            {
                /* Set the sync objects to signaled state */
                if (fce)
                {
                    gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);
                    sc->imageBuffers[*pImageIndex].fence = VK_NULL_HANDLE;
                }

                if (sph)
                {
                    __vk_SetSemaphore(device, semaphore, VK_TRUE);
                    sc->imageBuffers[*pImageIndex].semaphore = VK_NULL_HANDLE;
                }
            }
            else
            {
                sc->imageBuffers[*pImageIndex].fence = fence;
                sc->imageBuffers[*pImageIndex].semaphore = semaphore;
            }

            result = VK_SUCCESS;
        }

        gcoOS_ReleaseMutex(gcvNULL, sc->presentMutex);

        /* Start worker thread */
        gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);
    }

    return result;
}

/* __vkSwapchainKHR::QueuePresentSingle. */
static VkResult __QueuePresentSingle(
    VkQueue  queue,
    const VkDisplayPresentInfoKHR* pDisplayPresentInfo,
    VkSwapchainKHR swapchain,
    uint32_t imageIndex
    )
{
    VkResult result = VK_SUCCESS;
    __vkWin32SwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, swapchain);

    if (sc->state != __VK_SWAP_CHAIN_STATE_VALID)
        result = VK_ERROR_OUT_OF_DATE_KHR;
    else
        result = __ValidateWin32SwapChain(sc->surface, &sc->imageExtent);

    if (!__VK_IS_SUCCESS(result))
    {
        if (sc->state == __VK_SWAP_CHAIN_STATE_VALID)
            sc->state = __VK_SWAP_CHAIN_STATE_OUT_OF_DATE;

        return result;
    }

    /* Add new entry to this swap chain's present queue and wait if none are available */
    do
    {
        if (sc->presentQueue.numberofEntries >= sc->imageCount)
        {
            /* Start worker thread */
            gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);

            gcoOS_WaitSignal (gcvNULL, sc->presentQueue.queueUpdate, gcvINFINITE);
        }
        else
        {
            uint32_t slot;
            gcoOS_AcquireMutex(gcvNULL, sc->presentMutex, gcvINFINITE);

            slot = sc->presentQueue.numberofEntries++;
            sc->presentQueue.entry[slot].imageIndex = imageIndex;

            if (pDisplayPresentInfo)
            {
                sc->presentQueue.entry[slot].srcRect    = pDisplayPresentInfo->srcRect;
                sc->presentQueue.entry[slot].dstRect    = pDisplayPresentInfo->dstRect;
                sc->presentQueue.entry[slot].pSrcRect   = &sc->presentQueue.entry[slot].srcRect;
                sc->presentQueue.entry[slot].pDstRect   = &sc->presentQueue.entry[slot].dstRect;
                sc->presentQueue.entry[slot].persistent = pDisplayPresentInfo->persistent;
            }
            else
            {
                sc->presentQueue.entry[slot].pSrcRect   = VK_NULL_HANDLE;
                sc->presentQueue.entry[slot].pDstRect   = VK_NULL_HANDLE;
                sc->presentQueue.entry[slot].persistent = VK_FALSE;
            }

            __VK_ONERROR(__vki_PresentSwapChainImage(
                queue,
                swapchain,
                imageIndex,
                sc->presentQueue.entry[slot].pSrcRect,
                sc->presentQueue.entry[slot].pDstRect
                ));

            sc->imageBuffers[imageIndex].state = __VK_SC_IMAGE_STATE_HW;

            /* Start worker thread */
            gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);

            gcoOS_ReleaseMutex(gcvNULL, sc->presentMutex);

            break;
        }
    }
    while (sc->presentQueue.numberofEntries >= sc->imageCount);

    /* Kick off present thread */
    gcoOS_Signal(gcvNULL, sc->presentQueue.threadStart, VK_TRUE);

    if (sc->presentThread == gcvNULL)
    {
        gcoOS_CreateThread(gcvNULL, (gcTHREAD_ROUTINE)__vki_scPresentThread, sc, &sc->presentThread);
    }

OnError:
    return result;
}

/* __vkSurfaceOperation::CreateSwapchain. */
static VkResult win32CreateSwapchain(
    VkDevice  device,
    const VkSwapchainCreateInfoKHR*  pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR*  pSwapchain
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkWin32SwapchainKHR *sc = VK_NULL_HANDLE;
    VkIcdSurfaceWin32 *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWin32 *, pCreateInfo->surface);
    VkCommandPoolCreateInfo cpci;
    VkCommandBufferAllocateInfo cbai;
    uint32_t i;
    VkResult result = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (!__VK_IS_SUCCESS(__ValidateWin32SwapChain(surf, (VkExtent2D *)&pCreateInfo->imageExtent)))
    {
        return VK_ERROR_SURFACE_LOST_KHR;
    }

    *pSwapchain = VK_NULL_HANDLE;

    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, sizeof(__vkWin32SwapchainKHR), (__vkObject**)&sc));

    sc->device              = device;
    sc->surface             = surf;
    sc->state               = __VK_SWAP_CHAIN_STATE_VALID;

    sc->minImageCount       = pCreateInfo->minImageCount;
    sc->imageFormat         = pCreateInfo->imageFormat;
    sc->imageColorSpace     = pCreateInfo->imageColorSpace;
    sc->imageExtent         = pCreateInfo->imageExtent;
    sc->imageArrayLayers    = pCreateInfo->imageArrayLayers;
    sc->imageUsage          = pCreateInfo->imageUsage;
    sc->imageSharingMode    = pCreateInfo->imageSharingMode;
    sc->preTransform        = pCreateInfo->preTransform;
    sc->compositeAlpha      = pCreateInfo->compositeAlpha;
    sc->presentMode         = pCreateInfo->presentMode;
    sc->clipped             = pCreateInfo->clipped;

    sc->imageCount          = 0;

    /* Create swap chain images */
    sc->imageBuffers = (__vkWin32ImageBuffer *)__VK_ALLOC(
        (pCreateInfo->minImageCount * sizeof(__vkWin32ImageBuffer)),
        8,
        VK_SYSTEM_ALLOCATION_SCOPE_COMMAND
        );

    if (!sc->imageBuffers)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }
    __VK_MEMZERO(sc->imageBuffers, pCreateInfo->minImageCount * sizeof(__vkWin32ImageBuffer));

    for (i = 0; i < sc->minImageCount; i++)
    {
        __VK_ONERROR(__CreateSwapchainImageBuffer((VkSwapchainKHR)(uintptr_t)sc, &sc->imageBuffers[i]));
        sc->imageCount++;
    }

    /* Create a command pool for each swap chain thread and allocate a command buffer */
    __VK_MEMZERO(&cpci, sizeof(VkCommandPoolCreateInfo));
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    __VK_ONERROR(__vk_CreateCommandPool(sc->device, &cpci, gcvNULL, &sc->cmdPool));

    __VK_MEMZERO(&cbai, sizeof(cbai));
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandPool = sc->cmdPool;
    cbai.commandBufferCount = 1;
    __VK_ONERROR(__vk_AllocateCommandBuffers(sc->device, &cbai, &sc->cmdBuf));

    /* Allocate memory for swap chains present queue entries */
    sc->presentQueue.entry = (__vkPresentQueueEntry *)__VK_ALLOC(
        (sc->imageCount * sizeof(__vkPresentQueueEntry)),
        4,
        VK_SYSTEM_ALLOCATION_SCOPE_COMMAND
        );

    if (!sc->presentQueue.entry)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }
    __VK_MEMZERO(sc->presentQueue.entry, sc->imageCount * sizeof(__vkPresentQueueEntry));

    /* Signal from present queue thread */
    __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_FALSE, &sc->signal));

    /* Present queue thread start and stop signals */
    __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_FALSE, &sc->presentQueue.threadStart));
    __VK_ONERROR(gcoOS_CreateSignal(gcvNULL, VK_FALSE, &sc->presentQueue.queueUpdate));

    sc->presentQueue.threadStop = VK_FALSE;

    __VK_ONERROR(gcoOS_CreateMutex(gcvNULL, &sc->presentMutex));

    sc->base.DestroySwapchain   = __DestroySwapchain;
    sc->base.GetSwapchainImages = __GetSwapchainImages;
    sc->base.AcquireNextImage   = __AcquireNextImage;
    sc->base.QueuePresentSingle = __QueuePresentSingle;

    *pSwapchain = (VkSwapchainKHR)(uintptr_t)sc;

OnError:

    if (pCreateInfo->oldSwapchain)
    {
        __vkWin32SwapchainKHR *old_sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWin32SwapchainKHR *, pCreateInfo->oldSwapchain);

        old_sc->state = __VK_SWAP_CHAIN_STATE_OLD;
        __DestroySwapchain(device, pCreateInfo->oldSwapchain, pAllocator);
    }

    if (result != VK_SUCCESS)
    {
        if (sc)
        {
            if (sc->imageBuffers)
            {
                for (i = 0; i < sc->imageCount; i++)
                {
                    __DestroySwapChainImageBuffer(&sc->imageBuffers[i]);
                }
                __VK_FREE(sc->imageBuffers);
            }

            if (sc->cmdPool)
                __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

            if (sc->presentQueue.entry)
                __VK_FREE(sc->presentQueue.entry);

            if (sc->signal)
                gcoOS_DestroySignal(gcvNULL, sc->signal);

            if (sc->presentQueue.threadStart)
                gcoOS_DestroySignal(gcvNULL, sc->presentQueue.threadStart);

            if (sc->presentQueue.queueUpdate)
                gcoOS_DestroySignal(gcvNULL, sc->presentQueue.queueUpdate);

            __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
        }
    }

    return result;
}


VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateWin32SurfaceKHR(
    VkInstance instance,
    const VkWin32SurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceWin32 *surf;
    VkResult result = VK_SUCCESS;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    surf = (VkIcdSurfaceWin32 *)__VK_ALLOC(sizeof(VkIcdSurfaceWin32), 4, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!surf)
    {
        *pSurface = VK_NULL_HANDLE;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    surf->base.platform = VK_ICD_WSI_PLATFORM_WIN32;
    surf->hinstance     = pCreateInfo->hinstance;
    surf->hwnd          = pCreateInfo->hwnd;

    *pSurface = (VkSurfaceKHR)(uintptr_t)surf;
    ShowWindow(surf->hwnd, SW_SHOWNORMAL);

    return result;
}

/*****************************************************************************/
/* VK_KHR_display for win32. */

typedef struct __vkWin32DisplayKHRRec       __vkWin32DisplayKHR;
typedef struct __vkWin32DisplayPlaneRec     __vkWin32DisplayPlane;
typedef struct __vkWin32DisplayModeKHRRec   __vkWin32DisplayModeKHR;

struct __vkWin32DisplayKHRRec
{
    /* Display specific fields */
    DISPLAY_DEVICE device;
    DEVMODE displayMode;
    VkExtent2D dimensions;
    VkExtent2D resoultion;
    uint32_t bpp;
};

struct __vkWin32DisplayPlaneRec
{
    uint32_t planeIndex;
};

struct __vkWin32DisplayModeKHRRec
{
    __vkObjectType sType;

    /* DisplayMode specific fields */
};


VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayPropertiesKHR* pProperties
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    __vkWin32DisplayKHR *disp;
    VkResult result = VK_SUCCESS;
    uint32_t i = 0;
    __VK_SET_ALLOCATIONCB(&phyDev->pInst->memCb);

    if (pProperties)
    {
        if (*pPropertyCount > phyDev->numberOfDisplays)
            *pPropertyCount = phyDev->numberOfDisplays;

        for (i = 0; i < *pPropertyCount; i++)
        {
            disp = (__vkWin32DisplayKHR *) phyDev->displays[i];
            pProperties[i].display = (VkDisplayKHR)(uintptr_t)disp;
            pProperties[i].displayName = disp->device.DeviceName;
            pProperties[i].physicalDimensions = disp->dimensions;
            pProperties[i].physicalResolution = disp->resoultion;
            pProperties[i].supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            pProperties[i].persistentContent = VK_FALSE;
            pProperties[i].planeReorderPossible = VK_FALSE;
        }

        if (*pPropertyCount < phyDev->numberOfDisplays)
            result = VK_INCOMPLETE;
    }
    else
    {
        VkBool32 validAdapter = VK_TRUE;

        do
        {
            DISPLAY_DEVICE adapter;
            VkBool32 validDisplay = VK_TRUE;

            adapter.cb = sizeof(DISPLAY_DEVICE);
            validAdapter = EnumDisplayDevices(
                gcvNULL,
                i,
                &adapter,
                EDD_GET_DEVICE_INTERFACE_NAME
                );

            /* Only concerned with the primary adapter for now */
            if (!(adapter.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
            {
                continue;
            }

            do
            {
                disp = (__vkWin32DisplayKHR *)__VK_ALLOC(sizeof(__vkWin32DisplayKHR), 4, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
                phyDev->displays[i] = (__vkDisplayKHR *) disp;
                __VK_MEMZERO(disp, sizeof(__vkWin32DisplayKHR));

                disp->device.cb = sizeof(DISPLAY_DEVICE);
                validAdapter = EnumDisplayDevices(
                    adapter.DeviceName,
                    i,
                    &disp->device,
                    EDD_GET_DEVICE_INTERFACE_NAME
                    );

                if (disp->device.StateFlags & DISPLAY_DEVICE_ACTIVE &&
                    disp->device.StateFlags & DISPLAY_DEVICE_ATTACHED)
                {
                    HDC hdc;
                    hdc = CreateDC(disp->device.DeviceName, NULL, NULL, NULL);
                    disp->dimensions.width  = GetDeviceCaps(hdc, HORZSIZE);
                    disp->dimensions.height = GetDeviceCaps(hdc, VERTSIZE);
                    disp->resoultion.width  = GetDeviceCaps(hdc, HORZRES);
                    disp->resoultion.height = GetDeviceCaps(hdc, VERTRES);
                    disp->bpp               = GetDeviceCaps(hdc, BITSPIXEL);
                    i++;
                }

                if (i >= __VK_WSI_MAX_PHYSICAL_DISPLAYS)
                {
                    i--;
                    validDisplay = validAdapter = VK_FALSE;
                }
            } while (validDisplay);

        } while (validAdapter);

        *pPropertyCount = i;
        phyDev->numberOfDisplays = *pPropertyCount;
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t* pPropertyCount,
    VkDisplayPlanePropertiesKHR* pProperties
    )
{
    if (pProperties)
    {
        uint32_t i;

        if (*pPropertyCount > 1)
            *pPropertyCount = 1;

        for (i = 0; i < *pPropertyCount; i++)
        {
            pProperties[i].currentStackIndex = 0;
        }

        if (*pPropertyCount < 1)
            return VK_INCOMPLETE;
    }
    else
    {
        *pPropertyCount = 1;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t planeIndex,
    uint32_t* pDisplayCount,
    VkDisplayKHR* pDisplays
    )
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    if (pDisplays)
    {
        uint32_t i;

        if (*pDisplayCount > 1)
            *pDisplayCount = 1;

        for (i = 0; i < *pDisplayCount; i++)
        {
            pDisplays[i] = (VkDisplayKHR)(uintptr_t)phyDev->displays[i];
        }

        if (*pDisplayCount < 1)
            return VK_INCOMPLETE;
    }
    else
    {
        *pDisplayCount = 1;
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayModePropertiesKHR(
    VkPhysicalDevice physicalDevice,
    VkDisplayKHR display,
    uint32_t* pPropertyCount,
    VkDisplayModePropertiesKHR* pProperties
    )
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayModeKHR(
    VkPhysicalDevice physicalDevice,
    VkDisplayKHR display,
    const VkDisplayModeCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDisplayModeKHR* pMode
    )
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_GetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice physicalDevice,
    VkDisplayModeKHR mode,
    uint32_t planeIndex,
    VkDisplayPlaneCapabilitiesKHR* pCapabilities
    )
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_CreateDisplayPlaneSurfaceKHR(
    VkInstance instance,
    const VkDisplaySurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    return VK_SUCCESS;
}


__vkSurfaceOperation __vkWin32SurfaceOperation =
{
    win32DestroySurface,
    win32GetPhysicalDeviceSurfaceSupport,
    win32GetPhysicalDeviceSurfaceCapabilities,
    win32GetPhysicalDeviceSurfaceFormats,
    win32GetPhysicalDeviceSurfacePresentModes,
    win32CreateSwapchain,
};


__vkSurfaceOperation __vkDisplaySurfaceOperation =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

#endif


