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


/*
 * Implement VK_KHR_wayland_surface
 * Implement support functions for VK_KHR_swapchain, VK_KHR_surface.
 */
#include "gc_vk_precomp.h"

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <poll.h>
#include <pthread.h>
#include <fcntl.h>

#include <wayland-client.h>
#include <wayland-viv-client-protocol.h>

typedef struct __vkWaylandSwapchainKHRRec   __vkWaylandSwapchainKHR;
typedef struct __vkWaylandImageBufferRec    __vkWaylandImageBuffer;
static VkExtent2D VIV_EXTENT = (VkExtent2D){-1, -1};

/*
 * A VkSwapchainKHR object (a.k.a. swapchain) provides the ability to present
 * rendering results to a surface. A swapchain is an abstraction for an array
 * of presentable images that are associated with a surface.
 */
struct __vkWaylandSwapchainKHRRec
{
    __vkSwapchainKHR                base;

    VkDevice                        device;
    VkIcdSurfaceWayland *           surface;

    struct wl_registry *            wl_registry;
    struct wl_event_queue *         wl_queue;

    struct wl_viv *                 wl_viv;
    struct wl_compositor *          wl_compositor;
    struct wl_region *              opaque_region;

    /* It's an old swapchain, replaced by new one. */
    VkBool32                        expired;

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

    __VkDirectRenderMode            renderMode;
    __vkWaylandImageBuffer *        imageBuffers;
    uint32_t                        imageCount;

    VkCommandPool                   cmdPool;
    VkCommandBuffer                 cmdBuf;
};

typedef enum __vkImageBufferStateRec
{
    __VK_IMAGE_STATE_FREE,
    __VK_IMAGE_STATE_DEQUEUED,
    __VK_IMAGE_STATE_QUEUED,
}
__vkImageBufferState;

struct __vkWaylandImageBufferRec
{
    __vkWaylandSwapchainKHR *       swapchain;

    VkImage                         renderTarget;
    VkDeviceMemory                  renderTargetMemory;

    VkImage                         resolveTarget;
    VkDeviceMemory                  resolveTargetMemory;
    int32_t                         resolveFd;

    struct wl_buffer *              wl_buf;
    struct wl_callback *            frame_callback;
    __vkImageBufferState            state;
};

static inline int
poll_event(struct wl_display *wl_dpy, short int events, int timeout)
{
    int ret;
    struct pollfd pfd[1];

    pfd[0].fd = wl_display_get_fd(wl_dpy);
    pfd[0].events = events;

    do
    {
        ret = poll(pfd, 1, timeout);
    }
    while (ret == -1 && errno == EINTR);

    return ret;
}

/*
 * Our optimized dispatch_queue, with timeout.
 * Use timeount to avoid deadlock in multi-threaded environment.
 * Returns the number of dispatched events on success or -1 on failure.
 */
static int
dispatch_queue(struct wl_display *wl_dpy,
                struct wl_event_queue *wl_queue, int timeout)
{
    int ret;

    if (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
        return wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

    for (;;)
    {
        ret = wl_display_flush(wl_dpy);

        if (ret != -1 || errno != EAGAIN)
            break;

        if (poll_event(wl_dpy, POLLOUT, -1) == -1)
        {
            wl_display_cancel_read(wl_dpy);
            return -1;
        }
    }

    /* Don't stop if flushing hits an EPIPE; continue so we can read any
     * protocol error that may have triggered it. */
    if (ret < 0 && errno != EPIPE)
    {
        wl_display_cancel_read(wl_dpy);
        return -1;
    }

    ret = poll_event(wl_dpy, POLLIN, timeout);

    /* cancel read when on error or timeout. */
    if (ret == -1 || ret == 0)
    {
        wl_display_cancel_read(wl_dpy);
        return ret;
    }

    if (wl_display_read_events(wl_dpy) == -1)
        return -1;

    return wl_display_dispatch_queue_pending(wl_dpy, wl_queue);
}

static void
sync_callback(void *data, struct wl_callback *callback, uint32_t serial)
{
    int *done = data;

    *done = 1;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener sync_listener = {
    sync_callback
};

/*
 * Our optimized roundtrip_queue. It's thread-safe.
 * Return -1 on error, 0 on success.
 */
static int
roundtrip_queue(struct wl_display *wl_dpy, struct wl_event_queue *wl_queue)
{
    struct wl_callback *callback;
    int done, ret = 0;

    done = 0;

    /*
     * This is to block read & dispatch events in other threads, so that the
     * callback is with correct queue and listener when 'done' event.
     */
    while (wl_display_prepare_read_queue(wl_dpy, wl_queue) == -1)
        wl_display_dispatch_queue_pending(wl_dpy, wl_queue);

    callback = wl_display_sync(wl_dpy);

    if (callback == NULL)
    {
        wl_display_cancel_read(wl_dpy);
        return -1;
    }

    wl_proxy_set_queue((struct wl_proxy *) callback, wl_queue);
    wl_callback_add_listener(callback, &sync_listener, &done);

    wl_display_cancel_read(wl_dpy);

    while (!done && ret >= 0)
        ret = dispatch_queue(wl_dpy, wl_queue, 5);

    if (ret == -1 && !done)
        wl_callback_destroy(callback);

    return ret;
}


/* __vkSurfaceOperation::DestroySurface. */
static void waylandDestroySurface(
    VkInstance  instance,
    VkSurfaceKHR surface,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceWayland *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWayland*, surface);

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    __VK_FREE(surf);
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceSupport. */
static VkResult waylandGetPhysicalDeviceSurfaceSupport(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported
    )
{
    __vkPhysicalDevice *phyDev  = (__vkPhysicalDevice *)physicalDevice;

    *pSupported = VK_FALSE;

    if (queueFamilyIndex <= phyDev->queueFamilyCount)
    {
        *pSupported = phyDev->queuePresentSupported[queueFamilyIndex];
    }

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceCapabilities. */
static VkResult waylandGetPhysicalDeviceSurfaceCapabilities(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities
    )
{
    VkIcdSurfaceWayland *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWayland*, surface);
    (void) surf;

    pSurfaceCapabilities->minImageCount           = 1;
    pSurfaceCapabilities->maxImageCount           = 8;
    pSurfaceCapabilities->currentExtent           = (VkExtent2D){1024, 768};
    pSurfaceCapabilities->minImageExtent          = (VkExtent2D){1, 1};
    pSurfaceCapabilities->maxImageExtent          = VIV_EXTENT;
    pSurfaceCapabilities->maxImageArrayLayers     = 1;
    pSurfaceCapabilities->supportedTransforms     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->currentTransform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR | VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    pSurfaceCapabilities->supportedUsageFlags     = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceFormats. */
static VkResult waylandGetPhysicalDeviceSurfaceFormats(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats
    )
{
    static const VkFormat supportedPresentFormats[] = {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R5G6B5_UNORM_PACK16,
    };

    if (pSurfaceFormats)
    {
        uint32_t i;
        uint32_t count = sizeof(supportedPresentFormats) / sizeof(supportedPresentFormats[0]);

        if (*pSurfaceFormatCount > count)
            *pSurfaceFormatCount = count;

        for (i = 0; i < *pSurfaceFormatCount; i++)
        {
            pSurfaceFormats[i].format     = supportedPresentFormats[i];
            pSurfaceFormats[i].colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }

        if (*pSurfaceFormatCount < count)
            return VK_INCOMPLETE;
    }
    else
    {
        *pSurfaceFormatCount = __VK_COUNTOF(supportedPresentFormats);
    }

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfacePresentModes. */
static VkResult waylandGetPhysicalDeviceSurfacePresentModes(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes
    )
{
    static VkPresentModeKHR presentModes[] =
    {
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

/* __vkSurfaceOperation::GetDeviceGroupSurfacePresentModesKHR */
static VkResult waylandGetDeviceGroupSurfacePresentModesKHR(
    VkDevice device,
    VkSurfaceKHR surface,
    VkDeviceGroupPresentModeFlagsKHR* pModes
    )
{
    if (pModes)
    {
        *pModes = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;
    }
    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDevicePresentRectanglesKHR */
static VkResult waylandGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pRectCount,
    VkRect2D* pRects
    )
{
    return VK_SUCCESS;
}


static VkResult __CreateSwapchainCommandBuffer(
    __vkWaylandSwapchainKHR *sc
    )
{
    VkResult result;
    VkCommandPoolCreateInfo cpci;
    VkCommandBufferAllocateInfo cbai;

    /* Create a command pool for each swap cahin thread and allocate a command buffer */
    __VK_MEMZERO(&cpci, sizeof(VkCommandPoolCreateInfo));
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    result = __vk_CreateCommandPool(sc->device, &cpci, gcvNULL, &sc->cmdPool);

    if (result != VK_SUCCESS)
        return result;

    __VK_MEMZERO(&cbai,sizeof(cbai));
    cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandPool        = sc->cmdPool;
    cbai.commandBufferCount = 1;
    result = __vk_AllocateCommandBuffers(sc->device, &cbai, &sc->cmdBuf);

    if (result != VK_SUCCESS)
    {
        __vk_DestroyCommandPool(sc->device, sc->cmdPool, NULL);
        sc->cmdPool = VK_NULL_HANDLE;
    }

    return result;
}

static void buffer_handle_release(void *data, struct wl_buffer *wl_buf)
{
    __vkWaylandImageBuffer *imageBuffer = data;

    if  (imageBuffer->state != __VK_IMAGE_STATE_QUEUED)
    {
        fprintf(stderr, "%s: ERROR: invalid state=%d\n",
                __func__, imageBuffer->state);
    }

    imageBuffer->state = __VK_IMAGE_STATE_FREE;
}

static struct wl_buffer_listener buffer_listener = {
    /**
     * release - compositor releases buffer
     *
     * Sent when this wl_buffer is no longer used by the compositor.
     * The client is now free to re-use or destroy this buffer and its
     * backing storage.
     *
     * If a client receives a release event before the frame callback
     * requested in the same wl_surface.commit that attaches this
     * wl_buffer to a surface, then the client is immediately free to
     * re-use the buffer and its backing storage, and does not need a
     * second buffer for the next surface content update. Typically
     * this is possible, when the compositor maintains a copy of the
     * wl_surface contents, e.g. as a GL texture. This is an important
     * optimization for GL(ES) compositors with wl_shm clients.
     */
    buffer_handle_release
};

static VkResult __CreateImageBuffer(
    __vkWaylandSwapchainKHR  *sc,
    __vkWaylandImageBuffer *imageBuffer
    )
{
    VkImageCreateInfo imgInfo;
    VkMemoryAllocateInfo memAlloc;
    VkResult result = VK_SUCCESS;

    imageBuffer->swapchain = sc;
    imageBuffer->renderTarget = VK_NULL_HANDLE;
    imageBuffer->renderTargetMemory = VK_NULL_HANDLE;
    imageBuffer->resolveTarget = VK_NULL_HANDLE;
    imageBuffer->resolveTargetMemory = VK_NULL_HANDLE;
    imageBuffer->resolveFd = -1;

    /* Create the swap chain render target images */
    __VK_MEMZERO(&imgInfo, sizeof(VkImageCreateInfo));
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.format        = sc->imageFormat;
    imgInfo.extent.width  = sc->imageExtent.width;
    imgInfo.extent.height = sc->imageExtent.height;
    imgInfo.extent.depth  = 1;
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = sc->imageArrayLayers;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage         = sc->imageUsage;
    imgInfo.flags         = 0;
    __VK_ONERROR(__vk_CreateImage(sc->device, &imgInfo, gcvNULL, &imageBuffer->renderTarget));

    /* allocate memory for image. */
    __VK_MEMZERO(&memAlloc, sizeof(memAlloc));
    memAlloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize  = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->renderTarget))->memReq.size;
    memAlloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory(sc->device, &memAlloc, gcvNULL, &imageBuffer->renderTargetMemory));

    /* bind memory to image. */
    __VK_ONERROR(__vk_BindImageMemory(sc->device, imageBuffer->renderTarget, imageBuffer->renderTargetMemory, 0));

    /* Create the swap chain resolve buffers */
    __VK_MEMZERO(&imgInfo, sizeof(VkImageCreateInfo));
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.format        = sc->imageFormat;
    imgInfo.extent.width  = sc->imageExtent.width;
    imgInfo.extent.height = sc->imageExtent.height;
    imgInfo.extent.depth  = 1;
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = sc->imageArrayLayers;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling        = VK_IMAGE_TILING_LINEAR;
    imgInfo.usage         = sc->imageUsage;
    imgInfo.flags         = 0;
    __VK_ONERROR(__vk_CreateImage(sc->device, &imgInfo, gcvNULL, &imageBuffer->resolveTarget));

    /* allocate memory for image. */
    __VK_MEMZERO(&memAlloc, sizeof(memAlloc));
    memAlloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize  = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->resolveTarget))->memReq.size;
    memAlloc.memoryTypeIndex = 0;
    __VK_ONERROR(__vk_AllocateMemory(sc->device, &memAlloc, gcvNULL, &imageBuffer->resolveTargetMemory));

    /* bind memory to image. */
    __VK_ONERROR(__vk_BindImageMemory(sc->device, imageBuffer->resolveTarget, imageBuffer->resolveTargetMemory, 0));

    {
        uint32_t alignedWidth;
        int32_t stride;
        gceSURF_FORMAT format;
        gcsSURF_NODE *surfNode;
        uint32_t node = 0;
        gcsHAL_INTERFACE iface;
        __vkImage *image;
        gceSTATUS status;

        image = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->resolveTarget);
        surfNode = &image->memory->node;

        alignedWidth  = (sc->imageExtent.width + 15) & ~15;

        switch (sc->imageFormat)
        {
        case VK_FORMAT_B8G8R8A8_UNORM:
            format = gcvSURF_A8R8G8B8;
            stride = alignedWidth * 4;
            break;

        case VK_FORMAT_B8G8R8A8_SRGB:
            format = gcvSURF_A8R8G8B8;
            stride = alignedWidth * 4;
            break;

        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            format = gcvSURF_R5G6B5;
            stride = alignedWidth * 2;
            break;

        default:
            result = VK_ERROR_FORMAT_NOT_SUPPORTED;
            goto OnError;
        }

        /* Name video memory in global space */
        __VK_MEMZERO(&iface, sizeof(iface));
        iface.command = gcvHAL_NAME_VIDEO_MEMORY;
        iface.u.NameVideoMemory.handle = surfNode->u.normal.node;

        status = gcoOS_DeviceControl(
            gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(gcsHAL_INTERFACE),
            &iface, gcmSIZEOF(gcsHAL_INTERFACE)
            );

        if (gcmIS_SUCCESS(status))
        {
            status = iface.status;
        }
        if (gcmIS_ERROR(status))
        {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto OnError;
        }
        node = iface.u.NameVideoMemory.name;

        /* Export video memory as dmabuf fd */
        __VK_MEMZERO(&iface, sizeof(iface));
        iface.command = gcvHAL_EXPORT_VIDEO_MEMORY;
        iface.u.ExportVideoMemory.node = surfNode->u.normal.node;
        iface.u.ExportVideoMemory.flags = O_RDWR;
        status = gcoOS_DeviceControl(
            gcvNULL,
            IOCTL_GCHAL_INTERFACE,
            &iface, gcmSIZEOF(gcsHAL_INTERFACE),
            &iface, gcmSIZEOF(gcsHAL_INTERFACE)
            );

        if (gcmIS_SUCCESS(status))
        {
            status = iface.status;
        }
        if (gcmIS_ERROR(status))
        {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto OnError;
        }
        imageBuffer->resolveFd = iface.u.ExportVideoMemory.fd;

        imageBuffer->wl_buf = wl_viv_create_buffer(
            sc->wl_viv,
            sc->imageExtent.width,
            sc->imageExtent.height,
            stride,
            format,
            gcvSURF_BITMAP,
            node,
            surfNode->pool,
            surfNode->size,
            0,
            0,
            0,
            imageBuffer->resolveFd);

        wl_buffer_add_listener(imageBuffer->wl_buf, &buffer_listener, imageBuffer);

        imageBuffer->frame_callback = NULL;
        imageBuffer->state = __VK_IMAGE_STATE_FREE;
    }

    return VK_SUCCESS;

OnError:
    if (imageBuffer->renderTarget)
    {
        if (imageBuffer->renderTargetMemory)
        {
            __vk_FreeMemory(sc->device, imageBuffer->renderTargetMemory, gcvNULL);
            imageBuffer->renderTargetMemory = VK_NULL_HANDLE;
        }

        __vk_DestroyImage(sc->device, imageBuffer->renderTarget, gcvNULL);
        imageBuffer->renderTarget = VK_NULL_HANDLE;
    }

    if (imageBuffer->resolveTarget)
    {
        if (imageBuffer->resolveFd >= 0)
        {
            close(imageBuffer->resolveFd);
            imageBuffer->resolveFd = -1;
        }

        if (imageBuffer->resolveTargetMemory)
        {
            __vk_FreeMemory(sc->device, imageBuffer->resolveTargetMemory, gcvNULL);
            imageBuffer->resolveTargetMemory = VK_NULL_HANDLE;
        }

        __vk_DestroyImage(sc->device, imageBuffer->resolveTarget, gcvNULL);
        imageBuffer->resolveTarget = VK_NULL_HANDLE;
    }

    return result;
}

static void __DestroyImageBuffer(
    __vkWaylandSwapchainKHR * sc,
    __vkWaylandImageBuffer *imageBuffer
    )
{
    if (imageBuffer->wl_buf)
    {
        int ret = 0;
        VkIcdSurfaceWayland *surf = sc->surface;

        while (imageBuffer->frame_callback && ret != -1)
            ret = dispatch_queue(surf->display, sc->wl_queue, 5);
    }

    if (imageBuffer->renderTarget)
    {
        if (imageBuffer->renderTargetMemory)
        {
            __vk_FreeMemory(sc->device, imageBuffer->renderTargetMemory, gcvNULL);
            imageBuffer->renderTargetMemory = VK_NULL_HANDLE;
        }

        __vk_DestroyImage(sc->device, imageBuffer->renderTarget, gcvNULL);
        imageBuffer->renderTarget = VK_NULL_HANDLE;
    }

    if (imageBuffer->resolveTarget)
    {
        if (imageBuffer->resolveFd >= 0)
        {
            close(imageBuffer->resolveFd);
            imageBuffer->resolveFd = -1;
        }

        if (imageBuffer->resolveTargetMemory)
        {
            __vk_FreeMemory(sc->device, imageBuffer->resolveTargetMemory, gcvNULL);
            imageBuffer->resolveTargetMemory = VK_NULL_HANDLE;
        }

        __vk_DestroyImage(sc->device, imageBuffer->resolveTarget, gcvNULL);
        imageBuffer->resolveTarget = VK_NULL_HANDLE;
    }

    if (imageBuffer->wl_buf)
    {
        wl_buffer_destroy(imageBuffer->wl_buf);
        imageBuffer->wl_buf = NULL;
    }
}

/* __vkSwapchainKHR::DestroySwapchain. */
static void __DestroySwapchain(
    VkDevice  device,
    VkSwapchainKHR swapchain,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkWaylandSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWaylandSwapchainKHR *, swapchain);
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (sc->cmdPool)
        __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

    if (sc->imageBuffers)
    {
        uint32_t i;

        for (i = 0; i < sc->imageCount; i++)
        {
            __DestroyImageBuffer(sc, &sc->imageBuffers[i]);
        }
        __VK_FREE(sc->imageBuffers);
    }

    if (sc->surface)
    {
        VkIcdSurfaceWayland *surf = sc->surface;
        roundtrip_queue(surf->display, sc->wl_queue);

        wl_registry_destroy(sc->wl_registry);
        wl_viv_destroy(sc->wl_viv);

        if (sc->opaque_region)
            wl_region_destroy(sc->opaque_region);

        wl_event_queue_destroy(sc->wl_queue);
    }

    __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
}

/* __vkSwapchainKHR::GetSwapchainImages. */
static VkResult __GetSwapchainImages(
    VkDevice  device,
    VkSwapchainKHR swapchain,
    uint32_t*  pSwapchainImageCount,
    VkImage*  pSwapchainImages
    )
{
    __vkWaylandSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWaylandSwapchainKHR *, swapchain);
    VkResult result = VK_SUCCESS;

    if (pSwapchainImages)
    {
        uint32_t i;

        if (*pSwapchainImageCount > sc->imageCount)
            *pSwapchainImageCount = sc->imageCount;

        for (i = 0; i < *pSwapchainImageCount; i++)
        {
            pSwapchainImages[i] = sc->imageBuffers[i].renderTarget;
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
    VkSwapchainKHR swapchain,
    uint64_t  timeout,
    VkSemaphore  semaphore,
    VkFence  fence,
    uint32_t*  pImageIndex
    )
{
    __vkWaylandSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWaylandSwapchainKHR *, swapchain);
    VkIcdSurfaceWayland *surf = sc->surface;
    __vkWaylandImageBuffer *imageBuffer = VK_NULL_HANDLE;
    uint32_t imageIndex = 0;

    if (sc->expired)
        return VK_ERROR_OUT_OF_DATE_KHR;

    /* Try to read and dispatch some events. */
    dispatch_queue(surf->display, sc->wl_queue, 0);

    /* Dispatch the default queue. */
    if (wl_display_prepare_read(surf->display) == -1)
        wl_display_dispatch_pending(surf->display);
    else
        wl_display_cancel_read(surf->display);

    if (sc->imageCount > 1)
    {
        for (;;)
        {
            int i;

            for (i = 0; i < sc->imageCount; i++)
            {
                if (sc->imageBuffers[i].state == __VK_IMAGE_STATE_FREE)
                {
                    imageIndex  = i;
                    imageBuffer = &sc->imageBuffers[i];
                    break;
                }
            }

            if (imageBuffer)
            {
                break;
            }

            /* There should be some events to be handled.  */
            dispatch_queue(surf->display, sc->wl_queue, 1);

            /* Dispatch the default queue. */
            if (wl_display_prepare_read(surf->display) == -1)
                wl_display_dispatch_pending(surf->display);
            else
                wl_display_cancel_read(surf->display);
        }
    }
    else
    {
        imageBuffer = &sc->imageBuffers[0];
    }

    if (semaphore)
    {
        /* Set to signaled by CPU. */
        __vk_SetSemaphore(device, semaphore, VK_TRUE);
    }

    if (fence)
    {
        /* Set to signaled by CPU. */
        __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
        gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);
    }

    imageBuffer->state = __VK_IMAGE_STATE_DEQUEUED;
    *pImageIndex       = imageIndex;

    return VK_SUCCESS;
}

static VkResult __GenPresentCommand(
    __vkDevContext *devCtx,
    __vkWaylandSwapchainKHR *sc,
    __vkWaylandImageBuffer *imageBuffer
    )
{
    VkResult result = VK_SUCCESS;
    __vkBlitRes srcRes, dstRes;
    __vkImage *srcImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->renderTarget);
    __vkImage *dstImage = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->resolveTarget);

    VkCommandBufferBeginInfo cbi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    __VK_MEMZERO(&srcRes, sizeof(srcRes));
    srcRes.isImage                 = VK_TRUE;
    srcRes.u.img.pImage            = srcImage;
    srcRes.u.img.subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcRes.u.img.extent            = srcImage->createInfo.extent;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage                 = VK_TRUE;
    dstRes.u.img.pImage            = dstImage;
    dstRes.u.img.subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    dstRes.u.img.extent            = dstImage->createInfo.extent;

    __VK_ONERROR(__vk_BeginCommandBuffer(sc->cmdBuf, &cbi));

    __VK_ONERROR(devCtx->chipFuncs->CopyImage(
        sc->cmdBuf,
        &srcRes,
        &dstRes,
        VK_FALSE,
        VK_FILTER_NEAREST,
        VK_TRUE
        ));

    __VK_ONERROR(__vk_EndCommandBuffer(sc->cmdBuf));

OnError:
    return result;
}

static VkResult __CommitPresentCommand(
    VkQueue queue,
    __vkWaylandSwapchainKHR *sc
    )
{
    VkResult result;
    __vkCommandBuffer *cmd;
    __vkStateBuffer *stateBuffer;
    uint32_t stateBufCount;
    __vk_CommitInfo *pCommits[__VK_MAX_COMMITS_POOL_COUNT] = {gcvNULL};
    __vk_CommitInfo commitInfo;

    cmd = (__vkCommandBuffer *)sc->cmdBuf;
    stateBuffer = cmd->stateBufferList;
    stateBufCount = cmd->lastStateBufferIndex;

    if (cmd->stateBufferTail->bufOffset != 0)
        stateBufCount++;

    commitInfo.stateStart = stateBuffer->bufStart;
    commitInfo.stateSize  = stateBuffer->bufOffset;
    commitInfo.statePipe  = stateBuffer->bufPipe;

    pCommits[0] = &commitInfo;

    __VK_ONERROR(__vk_CommitStateBuffers(queue, pCommits, 0, 1));

    __vk_QueueWaitIdle(queue);

OnError:
    return result;
}

static void
frame_callback_handle_done(void *data, struct wl_callback *callback, uint32_t time)
{
    __vkWaylandImageBuffer *imageBuffer= data;

    imageBuffer->frame_callback = NULL;
    wl_callback_destroy(callback);
}

static const struct wl_callback_listener frame_callback_listener = {
    /**
     * done - done event
     * @callback_data: request-specific data for the wl_callback
     *
     * Notify the client when the related request is done.
     */
    frame_callback_handle_done
};

static VkResult __QueuePresentSwapchainImage(
    VkQueue queue,
    __vkWaylandSwapchainKHR *sc,
    VkIcdSurfaceWayland *surf,
    __vkWaylandImageBuffer *imageBuffer
    )
{
    VkResult result = VK_SUCCESS;
    __vkDevContext *devCtx = (__vkDevContext *)sc->device;

    /* Generate queue commands. */
    __VK_ONERROR(__GenPresentCommand(devCtx, sc, imageBuffer));
    __VK_ONERROR(__CommitPresentCommand(queue, sc));

    /*
     * This is to block read & dispatch events in other threads, so that the
     * callback is with correct queue and listener when 'done' event.
     */
    while (wl_display_prepare_read_queue(surf->display, sc->wl_queue) == -1)
        wl_display_dispatch_queue_pending(surf->display, sc->wl_queue);

    if (sc->opaque_region)
        wl_surface_set_opaque_region(surf->surface, sc->opaque_region);

    imageBuffer->frame_callback = wl_surface_frame(surf->surface);
    wl_proxy_set_queue((struct wl_proxy *)imageBuffer->frame_callback, sc->wl_queue);
    wl_callback_add_listener(imageBuffer->frame_callback, &frame_callback_listener, imageBuffer);

    wl_display_cancel_read(surf->display);

    imageBuffer->state = __VK_IMAGE_STATE_QUEUED;

    wl_surface_attach(surf->surface, imageBuffer->wl_buf, 0, 0);
    wl_surface_damage(surf->surface, 0, 0, sc->imageExtent.width, sc->imageExtent.height);
    wl_surface_commit(surf->surface);

    /* flush events. */
    wl_display_flush(surf->display);

    return result;

OnError:
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
    __vkWaylandSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWaylandSwapchainKHR *, swapchain);
    VkIcdSurfaceWayland *surf = sc->surface;

    return __QueuePresentSwapchainImage(queue, sc, surf, &sc->imageBuffers[imageIndex]);
}

static void
registry_handle_global(void *data, struct wl_registry *registry, uint32_t name,
                       const char *interface, uint32_t version)
{
    __vkWaylandSwapchainKHR *sc = data;

    if (strcmp(interface, "wl_viv") == 0)
    {
        sc->wl_viv = wl_registry_bind(registry, name, &wl_viv_interface, 1);
        wl_proxy_set_queue((struct wl_proxy *)sc->wl_viv, sc->wl_queue);
    }
    else if (strcmp(interface, "wl_compositor") == 0)
    {
        sc->wl_compositor = wl_registry_bind(registry, name,
                &wl_compositor_interface, 1);
        wl_proxy_set_queue((struct wl_proxy *)sc->wl_compositor, sc->wl_queue);
    }
}

static const struct wl_registry_listener registry_listener = {
    /**
     * global - announce global object
     * @name: (none)
     * @interface: (none)
     * @version: (none)
     *
     * Notify the client of global objects.
     *
     * The event notifies the client that a global object with the
     * given name is now available, and it implements the given version
     * of the given interface.
     */
    registry_handle_global
};


/* __vkSurfaceOperation::CreateSwapchain. */
static VkResult waylandCreateSwapchain(
    VkDevice  device,
    const VkSwapchainCreateInfoKHR*  pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchain
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkWaylandSwapchainKHR *sc = NULL;
    uint32_t i;
    VkResult result = VK_SUCCESS;
    VkIcdSurfaceWayland *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceWayland *, pCreateInfo->surface);

    switch (pCreateInfo->imageFormat)
    {
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        break;

    default:
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (pCreateInfo->oldSwapchain)
    {
        __vkWaylandSwapchainKHR *oldSc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkWaylandSwapchainKHR *, pCreateInfo->oldSwapchain);
        VkBool32 acquired = VK_FALSE;

        if (oldSc->imageBuffers)
        {
            for (i = 0; i < oldSc->imageCount; i++)
            {
                __vkWaylandImageBuffer *imageBuffer = &oldSc->imageBuffers[i];

                if (imageBuffer->state == __VK_IMAGE_STATE_DEQUEUED)
                {
                    acquired = VK_TRUE;
                    continue;
                }

                __DestroyImageBuffer(oldSc, imageBuffer);
            }

            if (!acquired)
            {
                /* Can free memory. */
                __VK_FREE(oldSc->imageBuffers);
                oldSc->imageBuffers = gcvNULL;
            }
        }

        if (!acquired && oldSc->cmdPool)
        {
            __vk_DestroyCommandPool(oldSc->device, oldSc->cmdPool, gcvNULL);
            oldSc->cmdPool = VK_NULL_HANDLE;
        }

        oldSc->expired = VK_TRUE;
    }

    *pSwapchain  = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, sizeof(__vkWaylandSwapchainKHR), (__vkObject**)&sc));

    sc->device              = device;
    sc->surface             = surf;
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

    VIV_EXTENT = pCreateInfo->imageExtent;

    /* reigister wayland globals. */
    sc->wl_queue    = wl_display_create_queue(surf->display);
    sc->wl_registry = wl_display_get_registry(surf->display);

    wl_proxy_set_queue((struct wl_proxy *)sc->wl_registry, sc->wl_queue);
    wl_registry_add_listener(sc->wl_registry, &registry_listener, sc);

    roundtrip_queue(surf->display, sc->wl_queue);

    if (sc->compositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
    {
        sc->opaque_region = wl_compositor_create_region(sc->wl_compositor);
        wl_proxy_set_queue((struct wl_proxy *)sc->opaque_region, sc->wl_queue);

        /* Set opaque region to full surface. */
        wl_region_add(sc->opaque_region, 0, 0, sc->imageExtent.width, sc->imageExtent.height);
    }

    sc->imageCount = 0;

    __VK_ONERROR(__CreateSwapchainCommandBuffer(sc));

    /* Create swap chain images */
    sc->imageBuffers = (__vkWaylandImageBuffer *)__VK_ALLOC(
        (sc->minImageCount * sizeof(__vkWaylandImageBuffer)), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (!sc->imageBuffers)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }

    __VK_MEMZERO(sc->imageBuffers, sc->minImageCount * sizeof(__vkWaylandImageBuffer));

    for (i = 0; i < sc->minImageCount; i++)
    {
        __VK_ONERROR(__CreateImageBuffer(sc, &sc->imageBuffers[i]));
        sc->imageCount++;
    }

    /* Assign function pointers. */
    sc->base.DestroySwapchain   = __DestroySwapchain;
    sc->base.GetSwapchainImages = __GetSwapchainImages;
    sc->base.AcquireNextImage   = __AcquireNextImage;
    sc->base.QueuePresentSingle = __QueuePresentSingle;

    *pSwapchain = (VkSwapchainKHR)(uintptr_t)sc;
    return VK_SUCCESS;

OnError:
    if (sc != NULL)
    {
        if (sc->cmdPool)
            __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

        if (sc->imageBuffers)
        {
            uint32_t i;

            for (i = 0; i < sc->imageCount; i++)
            {
                __DestroyImageBuffer(sc, &sc->imageBuffers[i]);
            }
            __VK_FREE(sc->imageBuffers);
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
    }
    return result;
}

VkResult VKAPI_CALL __vk_CreateWaylandSurfaceKHR(
    VkInstance instance,
    const VkWaylandSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    VkIcdSurfaceWayland *surf;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    surf = (VkIcdSurfaceWayland *)__VK_ALLOC(sizeof(VkIcdSurfaceWayland), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    if (!surf)
    {
        *pSurface = VK_NULL_HANDLE;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    /* Native objects. */
    surf->base.platform = VK_ICD_WSI_PLATFORM_WAYLAND;
    surf->display       = pCreateInfo->display;
    surf->surface       = pCreateInfo->surface;

    *pSurface = (VkSurfaceKHR)(uintptr_t)surf;
    return VK_SUCCESS;
}

VkBool32 VKAPI_CALL __vk_GetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    struct wl_display* display
    )
{
    return VK_TRUE;
}

__vkSurfaceOperation __vkWaylandSurfaceOperation =
{
    waylandDestroySurface,
    waylandGetPhysicalDeviceSurfaceSupport,
    waylandGetPhysicalDeviceSurfaceCapabilities,
    waylandGetPhysicalDeviceSurfaceFormats,
    waylandGetPhysicalDeviceSurfacePresentModes,
    waylandGetDeviceGroupSurfacePresentModesKHR,
    waylandGetPhysicalDevicePresentRectanglesKHR,
    waylandCreateSwapchain,
};

#endif

