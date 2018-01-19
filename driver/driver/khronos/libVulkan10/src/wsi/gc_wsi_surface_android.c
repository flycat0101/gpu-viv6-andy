/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


/*
 * Implement VK_KHR_android_surface
 * Implement support functions for VK_KHR_swapchain, VK_KHR_surface.
 */
#include "gc_vk_precomp.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR

/* Only for jellybean 4.2 and later (PLATFORM_SDK_VERSION >= 17) */
#define LOG_NDEBUG 1
#include <ui/ANativeObjectBase.h>
#include <sync/sync.h>
#include <cutils/log.h>

#ifdef DRM_GRALLOC
#   include <gralloc_handle.h>
#else
#   include <gc_gralloc_priv.h>
#endif

typedef struct __vkAndroidSurfaceKHRRec     __vkAndroidSurfaceKHR;
typedef struct __vkAndroidSwapchainKHRRec   __vkAndroidSwapchainKHR;
typedef struct __vkAndroidImageBufferRec    __vkAndroidImageBuffer;

/*
 * A VkSurfaceKHR object abstracts a native platform surface or window object.
 */
struct __vkAndroidSurfaceKHRRec
{
    VkIcdSurfaceBase                base;

    /* Native objects. */
    ANativeWindow *                 window;
    VkFormat                        format;

    /* Native object features. */
    int                             concreteType;
    int                             producerUsage;
    int                             consumerUsage;
    VkBool32                        hwFramebuffer;
    VkBool32                        queuesToComposer;

    /* Surface capabilities. */
    uint32_t                        minImageCount;
    uint32_t                        maxImageCount;
    VkExtent2D                      currentExtent;
    uint32_t                        maxImageArrayLayers;
    VkSurfaceTransformFlagsKHR      supportedTransforms;
    VkSurfaceTransformFlagBitsKHR   currentTransform;
    VkCompositeAlphaFlagsKHR        supportedCompositeAlpha;
    VkImageUsageFlags               supportedUsageFlags;
};

/*
 * A VkSwapchainKHR object (a.k.a. swapchain) provides the ability to present
 * rendering results to a surface. A swapchain is an abstraction for an array
 * of presentable images that are associated with a surface.
 */
struct __vkAndroidSwapchainKHRRec
{
    __vkSwapchainKHR                base;

    VkDevice                        device;
    __vkAndroidSurfaceKHR *         surface;

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
    __vkAndroidImageBuffer *        imageBuffers;
    uint32_t                        imageCount;

    VkCommandPool                   cmdPool;
    VkCommandBuffer                 cmdBuf;
};

struct __vkAndroidImageBufferRec
{
    VkImage                         renderTarget;
    VkDeviceMemory                  renderTargetMemory;

    VkBuffer                        resolveTarget;
    VkDeviceMemory                  resolveTargetMemory;
    void *                          nativeBuffer;
    uint32_t                        bufferRowLength;
    uint32_t                        bufferImageHeight;

    /* Acquired by app. */
    VkBool32                        acquired;
};

static struct
{
    int nativeFormat;
    VkFormat format;
}
formatXlateTable[] =
{
    {HAL_PIXEL_FORMAT_RGBA_8888, VK_FORMAT_R8G8B8A8_UNORM},
    /* NOTICE: No RGBX format in vulkan. */
    {HAL_PIXEL_FORMAT_RGBX_8888, VK_FORMAT_R8G8B8A8_UNORM},
    {HAL_PIXEL_FORMAT_RGB_888, VK_FORMAT_R8G8B8_UNORM},
    {HAL_PIXEL_FORMAT_RGB_565, VK_FORMAT_R5G6B5_UNORM_PACK16},
    {HAL_PIXEL_FORMAT_BGRA_8888, VK_FORMAT_B8G8R8A8_UNORM},
    {/* HAL_PIXEL_FORMAT_RGBA_5551 */ 6, VK_FORMAT_R5G5B5A1_UNORM_PACK16},
    {/* HAL_PIXEL_FORMAT_RGBA_4444 */ 7, VK_FORMAT_R4G4B4A4_UNORM_PACK16},
    {/* HAL_PIXEL_FORMAT_A_8 */ 8, VK_FORMAT_R8_UNORM},
    {/* HAL_PIXEL_FORMAT_L_8 */ 9, VK_FORMAT_R8_UNORM},
    {HAL_PIXEL_FORMAT_RGBA_8888, VK_FORMAT_R8G8B8A8_SRGB},
    {HAL_PIXEL_FORMAT_BGRA_8888, VK_FORMAT_B8G8R8A8_SRGB},
};

static VkFormat __TranslateAndroidFormat(
    int format
    )
{
    size_t i;

    for (i = 0; i < __VK_COUNTOF(formatXlateTable); i++)
    {
        if (format == formatXlateTable[i].nativeFormat)
            return formatXlateTable[i].format;
    }

    return VK_FORMAT_UNDEFINED;
}

static int __TranslateVkFormat(
    VkFormat format
    )
{
    size_t i;

    for (i = 0; i < __VK_COUNTOF(formatXlateTable); i++)
    {
        if (format == formatXlateTable[i].format)
            return formatXlateTable[i].nativeFormat;
    }

    return 0;
}


static VkResult __CancelNativeWindowBuffer(
    ANativeWindow * window,
    ANativeWindowBuffer_t * buffer
    )
{
    ALOGV(" %s: win=%p buffer=%p", __func__, window, buffer);

    if (window->cancelBuffer)
    {
        window->cancelBuffer(window, buffer, -1);
    }
    else
    {
        window->queueBuffer(window, buffer, -1);
    }

    buffer->common.decRef(&buffer->common);
    return VK_SUCCESS;
}

static VkResult __QueueNativeWindowBuffer(
    ANativeWindow * window,
    ANativeWindowBuffer_t * buffer,
    int fenceFd
    )
{
    ALOGV(" %s: window=%p buffer=%p fenceFd=%d", __func__, window, buffer, fenceFd);
    window->queueBuffer(window, buffer, fenceFd);

    buffer->common.decRef(&buffer->common);
    return VK_SUCCESS;
}

static VkResult __DequeueNativeWindowBuffer(
    ANativeWindow * window,
    uint64_t timeout,
    ANativeWindowBuffer_t **pBuffer,
    int *pFenceFd
    )
{
    VkResult result = (timeout > 0) ? VK_TIMEOUT : VK_NOT_READY;
    int rel;

    /* Dequeue a buffer from native window. */
    rel = window->dequeueBuffer(window, pBuffer, pFenceFd);

    if (rel != 0)
    {
        while (timeout > 2000000)
        {
            rel = window->dequeueBuffer(window, pBuffer, pFenceFd);

#if ANDROID_SDK_VERSION >= 20
            if (rel != -ENOSYS)
#   else
            if (rel != -EBUSY)
#   endif
            {
                result = VK_ERROR_SURFACE_LOST_KHR;
                break;
            }

            /* wait 2ms every time. */
            usleep(2000000);
            timeout -= 2000000;
        }
    }

    if (rel == 0)
    {
        (*pBuffer)->common.incRef(&(*pBuffer)->common);
        result = VK_SUCCESS;
    }

    ALOGV(" %s: window=%p =>\n\tbuffer=%p fenceFd=%d", __func__, window, *pBuffer, *pFenceFd);
    return result;
}

static void __NativeWindowApiConnect(
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win
    )
{
    ALOGV(" %s: surf=%p win=%p", __func__, surf, win);
    native_window_api_connect(win, NATIVE_WINDOW_API_EGL);
}

static void __NativeWindowApiDisconnect(
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win
    )
{
    ALOGV(" %s: surf=%p win=%p", __func__, surf, win);
    native_window_api_disconnect(win, NATIVE_WINDOW_API_EGL);
}

static VkResult __GatherANativeWindowProperties(
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win
    )
{
    int rel;
    VkResult result;
    ANativeWindowBuffer_t *buffer;
    struct private_handle_t *handle;
    int minUndequeued = 0;
    int fenceFd;
    int flags = 0;

    /*
     * Set default usage for native window. This is an egl surface,
     * thus only GPU can render to it.
     * GRALLOC_USAGE_SW_READ_NEVER and
     * GRALLOC_USAGE_SW_WRITE_NEVER are not really needed,
     * but it's better to be explicit.
     */
    surf->producerUsage = GRALLOC_USAGE_HW_RENDER
                        | GRALLOC_USAGE_HW_TEXTURE
                        | GRALLOC_USAGE_SW_READ_NEVER
                        | GRALLOC_USAGE_SW_WRITE_NEVER;

    native_window_set_usage(win, surf->producerUsage);

    /* Queries. */
    rel = win->query(win, NATIVE_WINDOW_CONCRETE_TYPE, &surf->concreteType);

    surf->consumerUsage = 0;
#if ANDROID_SDK_VERSION >= 19
    rel = win->query(win, NATIVE_WINDOW_CONSUMER_USAGE_BITS, &surf->consumerUsage);
#endif

    if (rel != 0)
    {
        ALOGW("%s: invalid win=%p", __func__, win);
        return VK_ERROR_SURFACE_LOST_KHR;
    }

    /* Dequeue a buffer. */
    result = __DequeueNativeWindowBuffer(win, UINT64_MAX, &buffer, &fenceFd);

    if (result != VK_SUCCESS)
    {
        ALOGE("%s: Failed to dequeue buffer", __func__);
        return result;
    }

    /* Update buffer size. */
    surf->currentExtent.width  = buffer->width;
    surf->currentExtent.height = buffer->height;
    surf->format               = __TranslateAndroidFormat(buffer->format);

    ALOGV(" %s: surf=%p win=%p: %dx%d format=%d->%d usage=0x%08llX:0x%08X", __func__,
        surf, win, buffer->width, buffer->height, buffer->format, surf->format,
        (unsigned long long) buffer->usage, surf->consumerUsage);

    if (surf->consumerUsage == 0)
    {
        /* Treat combined usage as consumer usage. */
        surf->consumerUsage = buffer->usage;
    }

#ifndef DRM_GRALLOC
    flags = ((struct private_handle_t*)buffer->handle)->flags;
#endif

    if (surf->concreteType == NATIVE_WINDOW_FRAMEBUFFER)
    {
        surf->hwFramebuffer    = VK_TRUE;
        surf->queuesToComposer = VK_FALSE;

        /*
         * ui/FramebufferNativeWindow has a not initialized pointer for
         * 'cancelBuffer'. Force 'queueBuffer' to avoid crash.
         */
        win->cancelBuffer = NULL;
        ALOGV("%s: win type: NATIVE_WINDOW_FRAMEBUFFER", __func__);
    }
    else if ((flags & 0x0001 /* PRIV_FLAGS_FRAMEBUFFER */) ||
        (surf->consumerUsage & GRALLOC_USAGE_HW_FB) ||
        (surf->consumerUsage & (GRALLOC_USAGE_HW_FB << 1)))
    {
        surf->hwFramebuffer    = VK_TRUE;
        surf->queuesToComposer = VK_FALSE;
        ALOGV(" %s: win type: FramebufferSurface", __func__);
    }
    else
    {
        surf->hwFramebuffer    = VK_FALSE;
        surf->queuesToComposer = (surf->consumerUsage & GRALLOC_USAGE_HW_COMPOSER)
                               ? VK_TRUE : VK_FALSE;

        ALOGV(" %s: win type: Surface queuesToComposer=%d",
             __func__, surf->queuesToComposer);
    }

    if (surf->hwFramebuffer)
    {
        /* FIXME: Where to get framebuffer buffer count? */
        surf->minImageCount = 3;
        surf->maxImageCount = 3;
    }
    else
    {
        /* min-Undequeued buffer count. */
        win->query(win, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeued);
        surf->minImageCount = minUndequeued + 2;
        surf->maxImageCount = 16;
    }

out:
    /* Cancel the buffer. */
    if (fenceFd != -1)
    {
        sync_wait(fenceFd, -1);
        close(fenceFd);
    }

    __CancelNativeWindowBuffer(win, buffer);
    return result;
}

static void __SetNativeWindowBufferCount(
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win,
    uint32_t imageCount
    )
{
    ALOGV(" %s: surf=%p win=%p imageCount=%d", __func__, surf, win, imageCount);
    native_window_set_buffer_count(win, imageCount);
}

static void __SetNativeWindowBufferFormat(
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win,
    VkFormat format
    )
{
    int nativeFormat = __TranslateVkFormat(format);
    native_window_set_buffers_format(win, nativeFormat);
}

static void __SelectNativeWindowRenderMode(
    __vkAndroidSwapchainKHR *sc,
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win
    )
{
    sc->renderMode = __VK_WSI_INDIRECT_RENDERING;
    ALOGV(" %s: sc=%p win=%p renderMode=%d", __func__, sc, win, sc->renderMode);
}

static void __UpdateNativeWindowProducerUsage(
    __vkAndroidSwapchainKHR *sc,
    __vkAndroidSurfaceKHR *surf,
    ANativeWindow *win
    )
{
    /* Default hw render producer usage. */
    int usage = GRALLOC_USAGE_HW_RENDER
              | GRALLOC_USAGE_HW_TEXTURE
              | GRALLOC_USAGE_SW_READ_NEVER
              | GRALLOC_USAGE_SW_WRITE_NEVER;

    switch (sc->renderMode)
    {
    case __VK_WSI_DIRECT_RENDERING_FCFILL:
    case __VK_WSI_DIRECT_RENDERING_FC_NOCC:
        /* Allocate render target with tile status but disable compression. */
#ifdef DRM_GRALLOC
        usage |= GRALLOC_USAGE_TS_VIV;
#else
        usage |= GRALLOC_USAGE_RENDER_TARGET_NO_CC_VIV;
#endif
        break;

    case __VK_WSI_DIRECT_RENDERING:
        /* Allocate normal render target. */
#ifdef DRM_GRALLOC
        usage |= GRALLOC_USAGE_TS_VIV;
#else
        /* FIXME: MSAA? */
        usage |= GRALLOC_USAGE_RENDER_TARGET_VIV;
#endif
        break;

    case __VK_WSI_DIRECT_RENDERING_NOFC:
        /* Allocate render target without tile status. */
#ifdef DRM_GRALLOC
        usage |= GRALLOC_USAGE_TILED_VIV;
#else
        usage |= GRALLOC_USAGE_RENDER_TARGET_NO_TS_VIV;
#endif
        break;

    default:
        break;
    }

    if (surf->producerUsage != usage)
    {
#ifndef DRM_GRALLOC
        if ((surf->producerUsage & usage) == usage)
        {
            /*
             * Append GRALLOC_USAGE_DUMMY_VIV(defined in gralloc) bitfield
             * to the new usage to indicate buffer usage change.
             * This is because in android framework:
             * if ((oldUsage & newUsage) == newUsage), buffer will not re-allocate.
             */
            usage |= GRALLOC_USAGE_DUMMY_VIV;
        }
#endif

        /* Pass usage to android framework. */
        ALOGV(" %s: surf=%p win=%p producerUsage=0x%08X", __func__, surf, win, usage);
        native_window_set_usage(win, usage);

        /* Store buffer usage. */
#ifdef DRM_GRALLOC
        surf->producerUsage = usage;
#else
        surf->producerUsage = (usage & ~GRALLOC_USAGE_DUMMY_VIV);
#endif
    }
}

#ifdef DRM_GRALLOC

static VkResult __WrapNativeWindowBufferMemory(
    VkDevice device,
    ANativeWindowBuffer_t *nativeBuffer,
    const VkAllocationCallbacks *pAllocator,
    VkDeviceMemory *pMemory
    )
{
    int err, fd;
    VkMemoryAllocateInfo allocInfo;
    __VkMemoryImportInfo importInfo;
    native_handle_t * handle = (native_handle_t*)nativeBuffer->handle;

    err = gralloc_handle_validate(handle);
    if (err)
    {
        /* Invalid handle. */
        ALOGE("%s(%d): invalid buffer=%p", __func__, __LINE__, nativeBuffer);
        return VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR;
    }

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    fd = (handle && handle->numFds) ? (int)handle->data[0] : -1;
    if (fd < 0)
    {
        ALOGE("%s(%d): invalid fd=%d", __func__, __LINE__, fd);
        return VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR;
    }

    allocInfo = (VkMemoryAllocateInfo) {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext              = NULL,
        .allocationSize     = 0,
        .memoryTypeIndex    = 0,
    };

    importInfo = (__VkMemoryImportInfo) {
        .type              = __VK_MEMORY_IMPORT_TYPE_LINUX_DMA_BUF,
        .u.dmaBuf.dmaBufFd = fd,
    };

    return __vk_ImportMemory(device, &allocInfo, &importInfo, pAllocator, pMemory);
}

#else

/*
 * Wrap native window buffer to __vkDeviceMemory
 */
static VkResult __WrapNativeWindowBufferMemory(
    VkDevice device,
    ANativeWindowBuffer_t *nativeBuffer,
    const VkAllocationCallbacks *pAllocator,
    VkDeviceMemory *pMemory
    )
{
    VkMemoryAllocateInfo allocInfo;
    __VkMemoryImportInfo importInfo;
    gc_native_handle_t*  handle;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    handle = gc_native_handle_get(nativeBuffer->handle);

    allocInfo = (VkMemoryAllocateInfo) {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext              = NULL,
        .allocationSize     = handle->nodeSize,
        .memoryTypeIndex    = 0,
    };

    importInfo = (__VkMemoryImportInfo) {
        .type               = __VK_MEMORY_IMPORT_TYPE_VIDEO_NODE,
        .u.videoMemNode.nodeName           = handle->node,
        .u.videoMemNode.nodePool           = handle->nodePool,
    };

    return __vk_ImportMemory(device, &allocInfo, &importInfo, pAllocator, pMemory);
}
#endif

static inline void __GetNativeWindowBufferSize(
    ANativeWindowBuffer_t *nativeBuffer,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pAlignedWidth
    )
{
    if (pWidth)
        *pWidth = (uint32_t) nativeBuffer->width;

    if (pHeight)
        *pHeight = (uint32_t) nativeBuffer->height;

    if (pAlignedWidth)
        *pAlignedWidth = (uint32_t) nativeBuffer->stride;
}

static const VkFormat wsiSupportedPresentFormats[] = {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R5G6B5_UNORM_PACK16,
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_R8G8B8A8_SRGB,
};


/* __vkSurfaceOperation::DestroySurface. */
static void androidDestroySurface(
    VkInstance  instance,
    VkSurfaceKHR surface,
    const VkAllocationCallbacks* pAllocator
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    __vkAndroidSurfaceKHR *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSurfaceKHR *, surface);
    ANativeWindow *win = surf->window;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    ALOGV("%s: surf=%p win=%p", __func__, surf, win);
    __SetNativeWindowBufferCount(surf, win, 0);
    __NativeWindowApiDisconnect(surf, win);
    win->common.decRef(&win->common);

    __VK_FREE(surf);
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceSupport. */
static VkResult androidGetPhysicalDeviceSurfaceSupport(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    VkSurfaceKHR surface,
    VkBool32* pSupported
    )
{
    __vkPhysicalDevice *phyDev  = (__vkPhysicalDevice *)physicalDevice;
    __vkAndroidSurfaceKHR *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSurfaceKHR *, surface);

    *pSupported = VK_FALSE;

    if (queueFamilyIndex <= phyDev->queueFamilyCount)
    {
        *pSupported = phyDev->queuePresentSupported[queueFamilyIndex];
    }

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceCapabilities. */
static VkResult androidGetPhysicalDeviceSurfaceCapabilities(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VkSurfaceCapabilitiesKHR* pSurfaceCapabilities
    )
{
    __vkAndroidSurfaceKHR *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSurfaceKHR *, surface);

    pSurfaceCapabilities->minImageCount             = surf->minImageCount;
    pSurfaceCapabilities->maxImageCount             = surf->maxImageCount;
    pSurfaceCapabilities->currentExtent             = surf->currentExtent;
    /* Btter to make swapchain image buffer size same as window size. */
    pSurfaceCapabilities->minImageExtent            = surf->currentExtent;
    pSurfaceCapabilities->maxImageExtent            = surf->currentExtent;
    pSurfaceCapabilities->maxImageArrayLayers       = surf->maxImageArrayLayers;
    pSurfaceCapabilities->supportedTransforms       = surf->supportedTransforms;
    pSurfaceCapabilities->currentTransform          = surf->currentTransform;
    pSurfaceCapabilities->supportedCompositeAlpha   = surf->supportedCompositeAlpha;
    pSurfaceCapabilities->supportedUsageFlags       = surf->supportedUsageFlags;

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfaceFormats. */
static VkResult androidGetPhysicalDeviceSurfaceFormats(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    VkSurfaceFormatKHR* pSurfaceFormats
    )
{
    if (pSurfaceFormats)
    {
        uint32_t i;
        uint32_t count = sizeof(wsiSupportedPresentFormats) / sizeof(wsiSupportedPresentFormats[0]);

        if (*pSurfaceFormatCount > count)
            *pSurfaceFormatCount = count;

        for (i = 0; i < *pSurfaceFormatCount; i++)
        {
            pSurfaceFormats[i].format     = wsiSupportedPresentFormats[i];
            pSurfaceFormats[i].colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }

        if (*pSurfaceFormatCount < count)
            return VK_INCOMPLETE;
    }
    else
    {
        *pSurfaceFormatCount = __VK_COUNTOF(wsiSupportedPresentFormats);
    }

    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDeviceSurfacePresentModes. */
static VkResult androidGetPhysicalDeviceSurfacePresentModes(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    VkPresentModeKHR* pPresentModes
    )
{
    static VkPresentModeKHR presentModes[] =
    {
        VK_PRESENT_MODE_MAILBOX_KHR,
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

static VkResult __CreateSwapchainCommandBuffer(
    __vkAndroidSwapchainKHR *sc
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

static VkResult __CreateImageBuffer(
    __vkAndroidSwapchainKHR  *sc,
    __vkAndroidImageBuffer *imageBuffer
    )
{
    VkImageCreateInfo imgInfo;
    VkBufferCreateInfo bufInfo;
    VkMemoryAllocateInfo memAlloc;
    VkResult result = VK_SUCCESS;

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
    imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;

    imageBuffer->renderTarget = VK_NULL_HANDLE;
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
    __VK_MEMZERO(&bufInfo, sizeof(VkBufferCreateInfo));
    bufInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.size        = (__VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->renderTarget))->pImgLevels[0].size;

    imageBuffer->resolveTarget = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateBuffer(sc->device, &bufInfo, gcvNULL, &imageBuffer->resolveTarget));

    imageBuffer->nativeBuffer = NULL;

    return VK_SUCCESS;

OnError:
    if (imageBuffer->renderTarget)
    {
        if (imageBuffer->renderTargetMemory)
            __vk_FreeMemory(sc->device, imageBuffer->renderTargetMemory, gcvNULL);

        __vk_DestroyImage(sc->device, imageBuffer->renderTarget, gcvNULL);

        imageBuffer->renderTarget = VK_NULL_HANDLE;
        imageBuffer->renderTargetMemory = VK_NULL_HANDLE;
    }

    if (imageBuffer->resolveTarget)
    {
        __vk_DestroyBuffer(sc->device, imageBuffer->resolveTarget, gcvNULL);

        imageBuffer->resolveTarget = VK_NULL_HANDLE;
    }

    return result;
}

static void __DestroySwapchainImageBuffer(
    __vkAndroidSwapchainKHR *sc,
    __vkAndroidImageBuffer *imageBuffer
    )
{
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
        if (imageBuffer->resolveTargetMemory)
        {
            __vk_FreeMemory(sc->device, imageBuffer->resolveTargetMemory, gcvNULL);
            imageBuffer->resolveTargetMemory = VK_NULL_HANDLE;
        }

        __vk_DestroyBuffer(sc->device, imageBuffer->resolveTarget, gcvNULL);
        imageBuffer->resolveTarget = VK_NULL_HANDLE;
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
    __vkAndroidSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSwapchainKHR *, swapchain);
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);
    ALOGV("%s: swapchain=%p", __func__, (void *) (uintptr_t)swapchain);

    if (sc->cmdPool)
        __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

    if (sc->imageBuffers)
    {
        uint32_t i;

        for (i = 0; i < sc->imageCount; i++)
        {
            __DestroySwapchainImageBuffer(sc, &sc->imageBuffers[i]);
        }
        __VK_FREE(sc->imageBuffers);
    }

    __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
}

static VkResult __GetSwapchainImages(
    VkDevice  device,
    VkSwapchainKHR swapchain,
    uint32_t*  pSwapchainImageCount,
    VkImage*  pSwapchainImages
    )
{
    __vkAndroidSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSwapchainKHR *, swapchain);
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

    ALOGV("%s: swapchain=%p imageCount=%u", __func__, sc, *pSwapchainImageCount);
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
    __vkAndroidSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSwapchainKHR *, swapchain);
    __vkAndroidSurfaceKHR *surf = sc->surface;
    ANativeWindow *win = surf->window;
    ANativeWindowBuffer_t *nativeBuffer;
    uint32_t bufferWidth;
    uint32_t bufferHeight;
    uint32_t bufferRowLength;
    uint32_t i;
    int fenceFd;
    int32_t emptySlot = -1;
    VkResult result;

    if (sc->expired)
        return VK_ERROR_OUT_OF_DATE_KHR;

    result = __DequeueNativeWindowBuffer(win, timeout, &nativeBuffer, &fenceFd);

    if (result != VK_SUCCESS)
        return result;

    __GetNativeWindowBufferSize(nativeBuffer, &bufferWidth, &bufferHeight, &bufferRowLength);

    if (bufferWidth != sc->imageExtent.width || bufferHeight != sc->imageExtent.height)
    {
        /* Update window properties. */
        surf->currentExtent.width  = bufferWidth;
        surf->currentExtent.height = bufferHeight;

        if (fenceFd != -1)
        {
            sync_wait(fenceFd, -1);
            close(fenceFd);
        }

        __CancelNativeWindowBuffer(win, nativeBuffer);
        /* native window resized. */
        return VK_ERROR_OUT_OF_DATE_KHR;
    }

    if (fenceFd != -1)
    {
        sync_wait(fenceFd, -1);
        close(fenceFd);
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

    /* Find the buffer index, stop at an empty slot. */
    for (i = 0; i < sc->imageCount; i++)
    {
        __vkAndroidImageBuffer *imageBuffer = &sc->imageBuffers[i];
        if (!imageBuffer->nativeBuffer)
        {
            emptySlot = i;
            break;
        }
        else if (imageBuffer->nativeBuffer == nativeBuffer)
        {
            imageBuffer->acquired = VK_TRUE;

            *pImageIndex = i;
            return VK_SUCCESS;
        }
    }

    if (emptySlot >= 0)
    {
        /*
         * New ANativeWindowBuffer comes. Bind it to imageBuffer.
         */
        __vkAndroidImageBuffer *imageBuffer = &sc->imageBuffers[emptySlot];
        VkDeviceMemory memory = VK_NULL_HANDLE;

        /* Wrap native window buffer as VkDeviceMemory. */
        __VK_ONERROR(__WrapNativeWindowBufferMemory(sc->device, nativeBuffer, NULL, &memory));

        result = __vk_BindBufferMemory(sc->device, imageBuffer->resolveTarget, memory, 0);

        if (result != VK_SUCCESS)
        {
            __vk_FreeMemory(sc->device, memory, NULL);
            goto OnError;
        }

        imageBuffer->bufferRowLength   = bufferRowLength;
        imageBuffer->bufferImageHeight = bufferHeight;
        imageBuffer->nativeBuffer = nativeBuffer;
        imageBuffer->resolveTargetMemory = memory;

        imageBuffer->acquired = VK_TRUE;

        *pImageIndex = i;
        return VK_SUCCESS;
    }

OnError:
    /* Unknown buffer handle comes. */
    __CancelNativeWindowBuffer(win, nativeBuffer);
    ALOGE("%s(%d): unexpected buffer handle change", __func__, __LINE__);
    return VK_ERROR_OUT_OF_DATE_KHR;
}

static VkResult __GenPresentCommand(
    __vkDevContext *devCtx,
    __vkAndroidSwapchainKHR *sc,
    __vkAndroidImageBuffer *imageBuffer
    )
{
    VkResult result = VK_SUCCESS;
    __vkBlitRes srcRes, dstRes;
    __vkImage *pSrcImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->renderTarget);

    VkCommandBufferBeginInfo cbi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    __VK_MEMZERO(&srcRes, sizeof(srcRes));
    srcRes.isImage                 = VK_TRUE;
    srcRes.u.img.pImage            = pSrcImg;
    srcRes.u.img.subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    srcRes.u.img.extent            = pSrcImg->createInfo.extent;

    __VK_MEMZERO(&dstRes, sizeof(dstRes));
    dstRes.isImage         = VK_FALSE;
    dstRes.u.buf.pBuffer   = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkBuffer*, imageBuffer->resolveTarget);
    dstRes.u.buf.rowLength = imageBuffer->bufferRowLength;
    dstRes.u.buf.imgHeight = imageBuffer->bufferImageHeight;

    __VK_ONERROR(__vk_BeginCommandBuffer(sc->cmdBuf, &cbi));

    __VK_ONERROR(devCtx->chipFuncs->CopyImage(
        sc->cmdBuf,
        &srcRes,
        &dstRes,
        VK_FALSE
        ));

    __VK_ONERROR(__vk_EndCommandBuffer(sc->cmdBuf));

OnError:
    return result;
}

static VkResult __CommitPresentCommand(
    VkQueue queue,
    __vkAndroidSwapchainKHR *sc,
    int *fenceFd
    )
{
    VkResult result;
    VkSubmitInfo si;

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

    /* Create fence sync. */
#if gcdANDROID_NATIVE_FENCE_SYNC
    do
    {
        gceSTATUS status;
        gctSIGNAL signal = gcvNULL;
        gcsHAL_INTERFACE iface;

        /* Create sync point. */
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &signal);

        if (gcmIS_ERROR(status))
        {
            gcoHAL_Commit(gcvNULL, gcvTRUE);
            break;
        }

        /* Create native fence. */
        status = gcoOS_CreateNativeFence(gcvNULL, signal, fenceFd);

        if (gcmIS_ERROR(status))
        {
            *fenceFd = -1;
#if __VK_NEW_DEVICE_QUEUE
            __vk_QueueWaitIdle(queue);
#  else
            gcoHAL_Commit(gcvNULL, gcvTRUE);
#  endif
            break;
        }

        /* Submit the sync point. */
        iface.command            = gcvHAL_SIGNAL;
        iface.engine             = gcvENGINE_RENDER;
        iface.u.Signal.signal    = gcmPTR_TO_UINT64(signal);
        iface.u.Signal.auxSignal = 0;
        iface.u.Signal.process   = (gctINT)gcoOS_GetCurrentProcessID();
        iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;
#if __VK_NEW_DEVICE_QUEUE
        __vk_QueueAppendEvent((__vkDevQueue *)queue, &iface);
        __vk_QueueCommitEvents((__vkDevQueue *)queue, VK_FALSE);
#  else
        /* Send event. */
        gcoHAL_ScheduleEvent(gcvNULL, &iface);
        gcoHAL_Commit(gcvNULL, gcvFALSE);
#  endif
        /* Now destroy the sync point. */
        gcoOS_DestroySignal(gcvNULL, signal);
    }
    while (VK_FALSE);

#else
#if __VK_NEW_DEVICE_QUEUE
    __vk_QueueWaitIdle(queue);
#  else
    gcoHAL_Commit(gcvNULL, gcvTRUE);
#  endif
    *fenceFd = -1;
#endif

OnError:
    return result;
}

static VkResult __QueuePresentSwapchainImage(
    VkQueue queue,
    __vkAndroidSwapchainKHR *sc,
    __vkAndroidSurfaceKHR *surf,
    __vkAndroidImageBuffer *imageBuffer
    )
{
    VkResult result = VK_SUCCESS;
    int fenceFd = -1;

    __vkDevContext *devCtx = (__vkDevContext *)sc->device;
    ANativeWindowBuffer_t *nativeBuffer = (ANativeWindowBuffer_t *) imageBuffer->nativeBuffer;

    imageBuffer->acquired = VK_FALSE;

    /* Generate queue commands. */
    __VK_ONERROR(__GenPresentCommand(devCtx, sc, imageBuffer));
    __VK_ONERROR(__CommitPresentCommand(queue, sc, &fenceFd));

    result = __QueueNativeWindowBuffer(surf->window, nativeBuffer, fenceFd);
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
    __vkAndroidSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSwapchainKHR *, swapchain);
    __vkAndroidSurfaceKHR *surf = sc->surface;

    ALOGV("%s: swapchain=%p surf=%p win=%p index=%d", __func__, sc, surf, surf->window, imageIndex);
    return __QueuePresentSwapchainImage(queue, sc, surf, &sc->imageBuffers[imageIndex]);
}

/* __vkSurfaceOperation::CreateSwapchain. */
static VkResult androidCreateSwapchain(
    VkDevice  device,
    const VkSwapchainCreateInfoKHR*  pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSwapchainKHR* pSwapchain
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkAndroidSwapchainKHR *sc = NULL;
    uint32_t i;
    uint32_t imageCount = 0;
    int swapInterval;
    VkResult result = VK_SUCCESS;
    __vkAndroidSurfaceKHR *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSurfaceKHR *, pCreateInfo->surface);
    ANativeWindow *win = surf->window;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (pCreateInfo->oldSwapchain)
    {
        __vkAndroidSwapchainKHR *oldSc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkAndroidSwapchainKHR *, pCreateInfo->oldSwapchain);
        VkBool32 acquired = VK_FALSE;

        ALOGE_IF(oldSc->surface != surf, "oldSwapchain does not match pCreateInfo->surface");

        if (oldSc->imageBuffers)
        {
            for (i = 0; i < oldSc->imageCount; i++)
            {
                __vkAndroidImageBuffer *imageBuffer = &oldSc->imageBuffers[i];

                if (imageBuffer->acquired)
                {
                    acquired = VK_TRUE;
                    continue;
                }

                __DestroySwapchainImageBuffer(oldSc, imageBuffer);
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
    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, sizeof(__vkAndroidSwapchainKHR), (__vkObject**)&sc));

    sc->device           = device;
    sc->surface          = surf;

    sc->minImageCount    = pCreateInfo->minImageCount;
    sc->imageFormat      = pCreateInfo->imageFormat;
    sc->imageColorSpace  = pCreateInfo->imageColorSpace;
    sc->imageExtent      = pCreateInfo->imageExtent;
    sc->imageArrayLayers = pCreateInfo->imageArrayLayers;
    sc->imageUsage       = pCreateInfo->imageUsage;
    sc->imageSharingMode = pCreateInfo->imageSharingMode;
    sc->preTransform     = pCreateInfo->preTransform;
    sc->compositeAlpha   = pCreateInfo->compositeAlpha;
    sc->presentMode      = pCreateInfo->presentMode;
    sc->clipped          = pCreateInfo->clipped;

    sc->imageCount       = 0;

    /* Require extra 1 image for async mode. */
    imageCount           = (pCreateInfo->presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                            ? sc->minImageCount + 1 : sc->minImageCount;

    __VK_ONERROR(__CreateSwapchainCommandBuffer(sc));

    __SelectNativeWindowRenderMode(sc, surf, win);
    __UpdateNativeWindowProducerUsage(sc, surf, win);
    __SetNativeWindowBufferCount(surf, win, imageCount);
    __SetNativeWindowBufferFormat(surf, win, sc->imageFormat);

    /* 0 for async mode (MAILBOX), 1 for sync mode (FIFO). */
    swapInterval = sc->presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? 0 : 1;
    win->setSwapInterval(win, swapInterval);

    /* Create swap chain images */
    sc->imageBuffers = (__vkAndroidImageBuffer *)__VK_ALLOC(
        (imageCount * sizeof(__vkAndroidImageBuffer)), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (!sc->imageBuffers)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }

    __VK_MEMZERO(sc->imageBuffers, imageCount * sizeof(__vkAndroidImageBuffer));

    for (i = 0; i < imageCount; i++)
    {
        __VK_ONERROR(__CreateImageBuffer(sc, &sc->imageBuffers[i]));
        sc->imageCount++;
    }

    /* Assign function pointers. */
    sc->base.DestroySwapchain   = __DestroySwapchain;
    sc->base.GetSwapchainImages = __GetSwapchainImages;
    sc->base.AcquireNextImage   = __AcquireNextImage;
    sc->base.QueuePresentSingle = __QueuePresentSingle;

    ALOGV("%s: surf=%p win=%p=>\n\tswapchain=%p", __func__, surf, win, sc);
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

            for (i = 0; i < imageCount; i++)
            {
                __DestroySwapchainImageBuffer(sc, &sc->imageBuffers[i]);
            }
            __VK_FREE(sc->imageBuffers);
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
    }
    return result;
}

VkResult VKAPI_CALL __vk_CreateAndroidSurfaceKHR(
    VkInstance instance,
    const VkAndroidSurfaceCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkSurfaceKHR* pSurface
    )
{
    __vkInstance *inst = (__vkInstance *)instance;
    __vkAndroidSurfaceKHR *surf;
    VkResult result;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&inst->memCb);

    surf = (__vkAndroidSurfaceKHR *)__VK_ALLOC(sizeof(__vkAndroidSurfaceKHR), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    if (!surf)
    {
        *pSurface = VK_NULL_HANDLE;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    do
    {
        ANativeWindow * win = pCreateInfo->window;

        /* Reference native window immediately. */
        surf->window = win;
        win->common.incRef(&win->common);

        __NativeWindowApiConnect(surf, win);

        result = __GatherANativeWindowProperties(surf, win);
        if (result != VK_SUCCESS) break;

        __SetNativeWindowBufferCount(surf, win, surf->minImageCount);

        surf->maxImageArrayLayers       = 1;
        surf->supportedTransforms       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        surf->currentTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        surf->supportedCompositeAlpha   = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        surf->supportedUsageFlags       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        /* Assign platform tpye. */
        surf->base.platform             = __VK_ICD_WSI_PLATFORM_ANDROID;

        ALOGV("%s: win=%p=>\n\tsurf=%p", __func__, win, surf);
        *pSurface = (VkSurfaceKHR)(uintptr_t)surf;
        return VK_SUCCESS;
    }
    while (0);

    __VK_FREE(surf);
    *pSurface = VK_NULL_HANDLE;
    return result;
}

#if (ANDROID_SDK_VERSION >= 24)
VKAPI_ATTR VkResult VKAPI_CALL __vk_GetSwapchainGrallocUsageANDROID(
    VkDevice device,
    VkFormat format,
    VkImageUsageFlags imageUsage,
    int* grallocUsage
    )
{
    *grallocUsage = GRALLOC_USAGE_HW_RENDER
#ifdef DRM_GRALLOC
                  | GRALLOC_USAGE_TILED_VIV
#else
                  | GRALLOC_USAGE_RENDER_TARGET_NO_TS_VIV
#endif
                  | GRALLOC_USAGE_HW_TEXTURE;

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_AcquireImageANDROID(
    VkDevice device,
    VkImage image,
    int nativeFenceFd,
    VkSemaphore semaphore,
    VkFence fence
    )
{
    if (nativeFenceFd != -1)
    {
        sync_wait(nativeFenceFd, -1);
        close(nativeFenceFd);
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

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL __vk_QueueSignalReleaseImageANDROID(
    VkQueue queue,
    uint32_t waitSemaphoreCount,
    const VkSemaphore* pWaitSemaphores,
    VkImage image,
    int* pNativeFenceFd
    )
{
    int fenceFd = -1;

    if (waitSemaphoreCount)
    {
        __vk_InsertSemaphoreWaits(queue, pWaitSemaphores, waitSemaphoreCount);
    }

#if gcdANDROID_NATIVE_FENCE_SYNC
    do
    {
        gceSTATUS status;
        gctSIGNAL signal = gcvNULL;
        gcsHAL_INTERFACE iface;

        /* Create sync point. */
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &signal);

        if (gcmIS_ERROR(status))
        {
            gcoHAL_Commit(gcvNULL, gcvTRUE);
            break;
        }

        /* Create native fence. */
        status = gcoOS_CreateNativeFence(gcvNULL, signal, &fenceFd);

        if (gcmIS_ERROR(status))
        {
            fenceFd = -1;
#if __VK_NEW_DEVICE_QUEUE
            __vk_QueueWaitIdle(queue);
#else
            gcoHAL_Commit(gcvNULL, gcvTRUE);
#endif
            break;
        }

        /* Submit the sync point. */
        iface.command            = gcvHAL_SIGNAL;
        iface.engine             = gcvENGINE_RENDER;
        iface.u.Signal.signal    = gcmPTR_TO_UINT64(signal);
        iface.u.Signal.auxSignal = 0;
        iface.u.Signal.process   = (gctINT)gcoOS_GetCurrentProcessID();
        iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;
#if __VK_NEW_DEVICE_QUEUE
        __vk_QueueAppendEvent((__vkDevQueue *)queue, &iface);
        __vk_QueueCommitEvents((__vkDevQueue *)queue, VK_FALSE);
#else
        /* Send event. */
        gcoHAL_ScheduleEvent(gcvNULL, &iface);
        gcoHAL_Commit(gcvNULL, gcvFALSE);
#endif
        /* Now destroy the sync point. */
        gcoOS_DestroySignal(gcvNULL, signal);
    }
    while (VK_FALSE);

#else
#if __VK_NEW_DEVICE_QUEUE
    __vk_QueueWaitIdle(queue);
#else
    gcoHAL_Commit(gcvNULL, gcvTRUE);
#endif
#endif

    *pNativeFenceFd = fenceFd;

    return VK_SUCCESS;
}

#  endif

__vkSurfaceOperation __vkAndroidSurfaceOperation =
{
    androidDestroySurface,
    androidGetPhysicalDeviceSurfaceSupport,
    androidGetPhysicalDeviceSurfaceCapabilities,
    androidGetPhysicalDeviceSurfaceFormats,
    androidGetPhysicalDeviceSurfacePresentModes,
    androidCreateSwapchain,
};

#endif


