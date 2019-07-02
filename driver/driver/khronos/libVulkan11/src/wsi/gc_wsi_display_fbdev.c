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
 * Implement VK_KHR_display for 'fbdev' hardware.
 */
#include "gc_vk_precomp.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <pthread.h>

/* Default Display Enumeration. */
#define FB_MAX_BUFFER_COUNT     8

/*
 * In fbdev style driver, there's no good way to find out display and display-plane
 * mapping, and plane index can not be re-ordered.
 * Predefine the fixed mapping information here.
 *
 * fbPlane[0] for each display is the main-lay, ie, the graphic plane, others are
 * overlays.
 */
struct
{
    uint32_t planeCount;
    uint32_t fbdevIndex[__VK_WSI_MAX_DISPLAY_PLANES];
}
__vkFbdevDisplayMapping[] =
{
    /* Display 0 */ {1, {0,}},
    /* Display 1 */ {0, {0,}}
};


typedef struct __vkFbdevDisplayPlaneRec         __vkFbdevDisplayPlane;
typedef struct __vkFbdevSwapchainKHRRec         __vkFbdevSwapchainKHR;
typedef struct __vkFbdevImageBufferRec          __vkFbdevImageBuffer;

struct __vkFbdevDisplayPlaneRec
{
    __vkDisplayPlane                    base;

    char                                fbPath[32];
    int                                 fd;

    /* properties. */
    VkFormat                            supportedFormats[16];
    uint32_t                            supportedFormatCount;
    uint32_t                            minImageCount;
    uint32_t                            maxImageCount;

    /* map memory to userspace. */
    void *                              userPtr;
    uint32_t                            userLength;

    /* underlying driver information. */
    struct fb_fix_screeninfo            fixInfo;
    struct fb_var_screeninfo            varInfo;

    VkFormat                            format;
    uint32_t                            stride;
    uint32_t                            alignedWidth;
    uint32_t                            alignedHeight;
    uint32_t                            imageCount;
};

typedef struct __vkFbdevDisplayPlaneParameterRec    __vkFbdevDisplayPlaneParameter;

struct __vkFbdevDisplayPlaneParameterRec
{
    VkDisplayModeKHR                    displayMode;
    uint32_t                            planeStackIndex;
    VkSurfaceTransformFlagBitsKHR       transform;
    float                               globalAlpha;
    VkDisplayPlaneAlphaFlagBitsKHR      alphaMode;
    VkExtent2D                          imageExtent;
    uint32_t                            imageCount;
    VkFormat                            imageFormat;
    VkColorSpaceKHR                     imageColorSpace;
    /* VkExtent2D                          imageExtent; */
    uint32_t                            imageArrayLayers;
    VkImageUsageFlags                   imageUsage;
    VkSharingMode                       imageSharingMode;
    VkSurfaceTransformFlagBitsKHR       preTransform;
    VkCompositeAlphaFlagBitsKHR         compositeAlpha;
    VkPresentModeKHR                    presentMode;
    VkBool32                            clipped;

};

/*
 * A VkSwapchainKHR object (a.k.a. swapchain) provides the ability to present
 * rendering results to a surface. A swapchain is an abstraction for an array
 * of presentable images that are associated with a surface.
 */
struct __vkFbdevSwapchainKHRRec
{
    __vkSwapchainKHR                    base;

    VkDevice                            device;
    VkIcdSurfaceDisplay *               surface;
    __vkFbdevDisplayPlane *             plane;

    /* It's an old swapchain, replaced by new one. */
    VkBool32                            expired;

    uint32_t                            minImageCount;
    VkFormat                            imageFormat;
    VkColorSpaceKHR                     imageColorSpace;
    VkExtent2D                          imageExtent;
    uint32_t                            imageArrayLayers;
    VkImageUsageFlags                   imageUsage;
    VkSharingMode                       imageSharingMode;
    VkCompositeAlphaFlagBitsKHR         compositeAlpha;
    VkSurfaceTransformFlagBitsKHR       preTransform;
    VkPresentModeKHR                    presentMode;
    VkBool32                            clipped;

    __vkFbdevImageBuffer *              imageBuffers;
    uint32_t                            imageCount;
    uint32_t                            currentImageIndex;

    VkCommandPool                       cmdPool;
    VkCommandBuffer                     cmdBuf;
};

struct __vkFbdevImageBufferRec
{
    __vkFbdevSwapchainKHR *             swapchain;

    VkImage                             renderTarget;
    VkDeviceMemory                      renderTargetMemory;

    VkBuffer                            resolveTarget;
    VkDeviceMemory                      resolveTargetMemory;
    uint32_t                            bufferRowLength;
    uint32_t                            bufferImageHeight;

    /* Acquired by app. */
    VkBool32                            acquired;
};

static struct
{
    VkFormat                            format;
    VkBool32                            enableAlpha;

    uint32_t                            bitsPerPixel;
    struct fb_bitfield                  red;
    struct fb_bitfield                  green;
    struct fb_bitfield                  blue;
    struct fb_bitfield                  transp;
}
__fbdevFormatXlate[] =
{
    {VK_FORMAT_R8G8B8A8_UNORM, VK_TRUE, 32, { 0, 8, 0}, { 8, 8, 0}, {16, 8, 0}, {24, 8, 0}},
    /* Fake RGRX8888 as RGBA8888. */
    {VK_FORMAT_R8G8B8A8_UNORM, VK_FALSE, 32, { 0, 8, 0}, { 8, 8, 0}, {16, 8, 0}, { 0, 0, 0}},

    {VK_FORMAT_B8G8R8A8_UNORM, VK_TRUE, 32, {16, 8, 0}, { 8, 8, 0}, { 0, 8, 0}, {24, 8, 0}},
    /* Fake BGRX8888 as BGRA8888. */
    {VK_FORMAT_B8G8R8A8_UNORM, VK_FALSE, 32, {16, 8, 0}, { 8, 8, 0}, { 0, 8, 0}, { 0, 0, 0}},

    {VK_FORMAT_R5G6B5_UNORM_PACK16, VK_FALSE, 16, {11, 5, 0}, { 5, 6, 0}, { 0, 5, 0}, { 0, 0, 0}},

};

static VkResult __TranslateFormatToFbdevInfo(
    VkFormat                            format,
    VkBool32                            enableAlpha,
    struct fb_var_screeninfo *          varInfo
    )
{
    uint32_t i;

    for (i = 0; i < __VK_COUNTOF(__fbdevFormatXlate); i++)
    {
        if ((__fbdevFormatXlate[i].format == format) &&
            __fbdevFormatXlate[i].enableAlpha == enableAlpha)
        {
            varInfo->bits_per_pixel = __fbdevFormatXlate[i].bitsPerPixel;
            varInfo->red            = __fbdevFormatXlate[i].red;
            varInfo->green          = __fbdevFormatXlate[i].green;
            varInfo->blue           = __fbdevFormatXlate[i].blue;
            varInfo->transp         = __fbdevFormatXlate[i].transp;
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

static VkFormat __TranslateFbdevInfoToFormat(
    const struct fb_var_screeninfo *    varInfo,
    VkBool32 *                          pEnableAlpha)
{
    uint32_t i;

    for (i = 0; i < __VK_COUNTOF(__fbdevFormatXlate); i++)
    {
        if ((__fbdevFormatXlate[i].bitsPerPixel == varInfo->bits_per_pixel) &&
            (__fbdevFormatXlate[i].red.offset == varInfo->red.offset) &&
            (__fbdevFormatXlate[i].red.length == varInfo->red.length) &&
            (__fbdevFormatXlate[i].green.offset == varInfo->green.offset) &&
            (__fbdevFormatXlate[i].green.length == varInfo->green.length) &&
            (__fbdevFormatXlate[i].blue.offset == varInfo->blue.offset) &&
            (__fbdevFormatXlate[i].blue.length == varInfo->blue.length) &&
            (__fbdevFormatXlate[i].transp.offset == varInfo->transp.offset) &&
            (__fbdevFormatXlate[i].transp.length == varInfo->transp.length))
        {
            if (pEnableAlpha)
                *pEnableAlpha = __fbdevFormatXlate[i].enableAlpha;

            return __fbdevFormatXlate[i].format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

/*
 * NOTICE:
 * It is platform specific.
 * alignedHeight may be aligned to page size on some platforms.
 */
static inline uint32_t __CalcAlignedHeight(
    const struct fb_var_screeninfo *    varInfo)
{
    return varInfo->yres_virtual / (varInfo->yres_virtual / varInfo->yres);
}

/* Default: one display-plane for one display, can not switch planes. */
static __vkDisplayKHR *__CreateFbdevDisplay(
    __vkPhysicalDevice *                phyDev,
    __vkFbdevDisplayPlane *             graphicPlane)
{
    struct fb_var_screeninfo *info = &graphicPlane->varInfo;
    __vkDisplayKHR *display;
    __vkDisplayModeKHR *displayMode;
    __VK_SET_ALLOCATIONCB(&phyDev->pInst->memCb);

    display = (__vkDisplayKHR *)__VK_ALLOC(sizeof(__vkDisplayKHR), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    __VK_MEMZERO(display, sizeof(__vkDisplayKHR));

    /* Get display name from graphics plane. */
    strncpy(display->displayName, graphicPlane->fixInfo.id, sizeof(display->displayName));

    display->physicalDimensions.width  = (info->width  <= 0) ? ((info->xres * 25.4f)/160.0f + 0.5f) : info->width;
    display->physicalDimensions.height = (info->height <= 0) ? ((info->yres * 25.4f)/160.0f + 0.5f) : info->height;
    display->physicalResolution.width  = info->xres;
    display->physicalResolution.height = info->yres;
    display->supportedTransforms       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    display->planeReorderPossible      = VK_FALSE;
    display->persistentContent         = VK_FALSE;

    /* Default: only current mode. */
    displayMode = (__vkDisplayModeKHR *)__VK_ALLOC(sizeof(__vkDisplayKHR), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    __VK_MEMZERO(displayMode, sizeof(__vkDisplayModeKHR));
    displayMode->sType = __VK_OBJECT_TYPE_DISPLAY_MODE_KHR;

    uint64_t refreshQuotient = (uint64_t) (info->upper_margin + info->lower_margin + info->yres)
            * (info->left_margin  + info->right_margin + info->xres) * info->pixclock;

    int refreshRate = refreshQuotient > 0 ? (int)(1000000000000000LLU / refreshQuotient) : 0;

    if (refreshRate == 0) {
        refreshRate = 60*1000;
    }

    displayMode->display                         = display;
    displayMode->parameters.visibleRegion.width  = info->xres;
    displayMode->parameters.visibleRegion.height = info->yres;
    displayMode->parameters.refreshRate          = refreshRate;

    display->displayModeCount = 1;
    display->displayModes[0]  = displayMode;

    display->planeStack[0]  = (__vkDisplayPlane *) graphicPlane;
    display->planeStackSize = 1;

    return display;
}

static VkResult __ValidateFbdevDisplayPlane(
    __vkFbdevDisplayPlane *             plane)
{
    int ret;

    if (plane->userPtr != NULL)
    {
        munmap(plane->userPtr, plane->userLength);
        plane->userPtr = NULL;
        plane->userLength = 0;
    }

    ret = ioctl(plane->fd, FBIOGET_FSCREENINFO, &plane->fixInfo);
    if (ret < 0)
        return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

    ret = ioctl(plane->fd, FBIOGET_VSCREENINFO, &plane->varInfo);
    if (ret < 0)
        return VK_ERROR_INCOMPATIBLE_DISPLAY_KHR;

    plane->varInfo.reserved[0] = 0;
    plane->varInfo.reserved[1] = 0;
    plane->varInfo.reserved[2] = 0;
    plane->varInfo.xoffset = 0;
    plane->varInfo.yoffset = 0;
    plane->varInfo.activate = FB_ACTIVATE_NOW;

    plane->format = __TranslateFbdevInfoToFormat(&plane->varInfo, VK_NULL_HANDLE);
    plane->stride = plane->fixInfo.line_length;

    plane->alignedWidth  = plane->fixInfo.line_length / (plane->varInfo.bits_per_pixel / 8);
    plane->alignedHeight = __CalcAlignedHeight(&plane->varInfo);

    plane->imageCount    = plane->varInfo.yres_virtual / plane->varInfo.yres;

    /* Align to page size. */
    plane->userLength = __VK_ALIGN(plane->fixInfo.smem_len, ((uint32_t) sysconf(_SC_PAGESIZE)));
    plane->userPtr    = mmap(0, plane->userLength, PROT_READ | PROT_WRITE, MAP_SHARED, plane->fd, 0);

    if (plane->userPtr == MAP_FAILED)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    return VK_SUCCESS;
}

static __vkFbdevDisplayPlane *__CreateFbdevDisplayPlane(
    __vkPhysicalDevice *                phyDev,
    const char *                        fbPath,
    uint32_t                            planeIndex)
{
    int fd = -1;
    uint32_t i;
    int ret;
    uint32_t alignedHeight;
    VkResult result = VK_SUCCESS;
    __vkFbdevDisplayPlane *plane = NULL;
    struct fb_var_screeninfo varInfo;

    __VK_SET_ALLOCATIONCB(&phyDev->pInst->memCb);

    fd = open(fbPath, O_RDWR);
    if (fd < 0)
        __VK_ONERROR(VK_INCOMPLETE);

    ret = ioctl(fd, FBIOGET_VSCREENINFO, &varInfo);
    if (ret < 0)
        __VK_ONERROR(VK_INCOMPLETE);

    plane = (__vkFbdevDisplayPlane *)__VK_ALLOC(sizeof(__vkFbdevDisplayPlane), 8, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    __VK_MEMZERO(plane, sizeof(__vkFbdevDisplayPlane));

    plane->base.planeIndex = planeIndex;
    plane->varInfo = varInfo;

    strncpy(plane->fbPath, fbPath, sizeof(plane->fbPath) - 1);
    plane->fd = fd;

    /* check supported formats. */
    for (i = 0; i < __VK_COUNTOF(__fbdevFormatXlate); i++)
    {
        VkFormat format = __fbdevFormatXlate[i].format;
        VkBool32 enableAlpha = __fbdevFormatXlate[i].enableAlpha;

        __TranslateFormatToFbdevInfo(format, enableAlpha, &varInfo);

        /* Test format. */
        varInfo.activate = FB_ACTIVATE_TEST;
        ret = ioctl(fd, FBIOPUT_VSCREENINFO, &varInfo);
        if (ret < 0)
            continue;

        /* get vscreen info. */
        ret = ioctl(fd, FBIOGET_VSCREENINFO, &varInfo);
        if (ret < 0)
            continue;

        format = __TranslateFbdevInfoToFormat(&varInfo, VK_NULL_HANDLE);

        if (format == __fbdevFormatXlate[i].format)
        {
            plane->supportedFormats[plane->supportedFormatCount] = format;

            /* Avoid duplicated format. */
            if ((plane->supportedFormatCount == 0) ||
                (plane->supportedFormats[plane->supportedFormatCount-1] != format))
            {
                plane->supportedFormatCount++;
            }
        }
    }

    if (plane->supportedFormatCount == 0)
    {
        /* No supported format. */
        __VK_ONERROR(VK_ERROR_FORMAT_NOT_SUPPORTED);
    }

    plane->minImageCount = 1;
    alignedHeight = __CalcAlignedHeight(&varInfo);

    /* Check maxImageCount. */
    for (i = FB_MAX_BUFFER_COUNT; i >= 1; i--)
    {
        varInfo.yres_virtual = alignedHeight * i;

        varInfo.activate = FB_ACTIVATE_TEST;
        ret = ioctl(plane->fd, FBIOPUT_VSCREENINFO, &varInfo);
        if (ret < 0) continue;

        ret = ioctl(plane->fd, FBIOGET_VSCREENINFO, &varInfo);
        if (ret < 0) continue;

        if (varInfo.yres_virtual >= alignedHeight * i)
        {
            /* this is the maxImageCount. */
            plane->maxImageCount = i;
            break;
        }
    }

    /* Do not need to restore display format */
    __VK_ONERROR(__ValidateFbdevDisplayPlane(plane));
    return plane;

OnError:
    /* Do not need to restore display format */
    if (plane)
    {
        __VK_FREE(plane);
    }

    if (fd >= 0)
    {
        close(fd);
    }

    return VK_NULL_HANDLE;
}

static VkResult __SetFbdevDisplayPlaneFormat(
    __vkFbdevDisplayPlane *             plane,
    VkFormat                            format
    )
{
    int ret;
    VkResult result = VK_SUCCESS;
    struct fb_var_screeninfo varInfo = plane->varInfo;

    do
    {
        varInfo.activate = FB_ACTIVATE_NOW;

        /* Try update format */
        __TranslateFormatToFbdevInfo(format, VK_TRUE, &varInfo);

        ret = ioctl(plane->fd, FBIOPUT_VSCREENINFO, &varInfo);
        if (!ret)
            break;

        /* Try update format without alpha channel. */
        __TranslateFormatToFbdevInfo(format, VK_FALSE, &varInfo);

        ret = ioctl(plane->fd, FBIOPUT_VSCREENINFO, &varInfo);
        if (!ret)
            break;

        result = VK_ERROR_FORMAT_NOT_SUPPORTED;
    }
    while (VK_FALSE);

    if (result != VK_SUCCESS)
    {
        /* Restore var screen info. */
        ioctl(plane->fd, FBIOPUT_VSCREENINFO, &plane->varInfo);
    }

    __ValidateFbdevDisplayPlane(plane);
    return result;
}

static VkResult __SetFbdevDisplayPlaneImageCount(
    __vkFbdevDisplayPlane *             plane,
    uint32_t                            imageCount
    )
{
    int ret;
    VkResult result = VK_SUCCESS;
    struct fb_var_screeninfo varInfo = plane->varInfo;

    varInfo.yres_virtual = plane->alignedHeight * imageCount;
    varInfo.activate     = FB_ACTIVATE_NOW;

    /* Update image count. */
    ret = ioctl(plane->fd, FBIOPUT_VSCREENINFO, &varInfo);

    if (ret < 0)
    {
        /* Restore var screen info. */
        ioctl(plane->fd, FBIOPUT_VSCREENINFO, &plane->varInfo);

        result = VK_ERROR_OUT_OF_DATE_KHR;
    }

    __ValidateFbdevDisplayPlane(plane);

    if (plane->imageCount != imageCount)
    {
        /* Failed to set image count. */
        result = VK_ERROR_OUT_OF_DATE_KHR;
    }

    return result;
}

static VkResult __SetFbdevDisplayPlaneParameter(
    __vkFbdevDisplayPlane *             plane,
    __vkFbdevDisplayPlaneParameter *    parameter)
{
    VkResult result = VK_SUCCESS;

    if (parameter->imageFormat != plane->format)
    {
        __VK_ONERROR(__SetFbdevDisplayPlaneFormat(plane, parameter->imageFormat));
    }

    if (parameter->imageCount != plane->imageCount)
    {
        __VK_ONERROR(__SetFbdevDisplayPlaneImageCount(plane, parameter->imageCount));
    }

OnError:
    return result;
}

static void __PostFbdevDisplayPlaneImage(
    __vkFbdevDisplayPlane *             plane,
    uint32_t                            imageIndex
    )
{
    int ret;

    /* Do not need to swap when render to front buffer. */
    if (plane->imageCount == 1)
        return;

    plane->varInfo.yoffset  = plane->alignedHeight * imageIndex;
    plane->varInfo.activate = FB_ACTIVATE_VBL;

    ret = ioctl(plane->fd, FBIOPAN_DISPLAY, &plane->varInfo);
    /* ret = ioctl(plane->fd, FBIOPUT_VSCREENINFO, &plane->varInfo); */

    if (ret < 0)
    {
        gcmPRINT("%s(%d): PAN display failed.", __func__, __LINE__);
    }
}

static void __GetFbdevBufferSize(
    __vkFbdevDisplayPlane *             plane,
    uint32_t *                          pWidth,
    uint32_t *                          pHeight,
    uint32_t *                          pAlignedWidth,
    uint32_t *                          pAlignedHeight,
    uint32_t *                          pMemorySize
    )
{
    if (pWidth)
        *pWidth = plane->varInfo.xres;

    if (pHeight)
        *pHeight = plane->varInfo.yres;

    if (pAlignedWidth)
        *pAlignedWidth = plane->alignedWidth;

    if (pAlignedHeight)
        *pAlignedHeight = plane->alignedHeight;

    if (pMemorySize)
        *pMemorySize = plane->stride * plane->alignedHeight;
}

/* Display functions. */
static VkResult fbdevCreateDisplayMode(
    VkPhysicalDevice                    physicalDevice,
    VkDisplayKHR                        display,
    const VkDisplayModeCreateInfoKHR*   pCreateInfo,
    const VkAllocationCallbacks*        pAllocator,
    VkDisplayModeKHR*                   pMode)
{
    __vkDisplayKHR *dpy = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayKHR *, display);
    uint32_t numModes = 0;
    __vkDisplayModeKHR *mode = VK_NULL_HANDLE;
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_ALLOCATIONCB(&phyDev->pInst->memCb);

    for (numModes = 0; numModes < dpy->displayModeCount; numModes++)
    {
        if ((pCreateInfo->parameters.refreshRate == dpy->displayModes[numModes]->parameters.refreshRate) &&
            (pCreateInfo->parameters.visibleRegion.width == dpy->displayModes[numModes]->parameters.visibleRegion.width) &&
            (pCreateInfo->parameters.visibleRegion.height == dpy->displayModes[numModes]->parameters.visibleRegion.height))
        {
            break;
        }
    }

    if (numModes >= dpy->displayModeCount)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    mode = (__vkDisplayModeKHR *)__VK_ALLOC(sizeof(__vkDisplayModeKHR), 4, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

    mode->display = dpy;
    mode->bitsPerPixel = dpy->displayModes[0]->bitsPerPixel;
    mode->interlaced = dpy->displayModes[0]->interlaced;
    mode->sType = dpy->displayModes[0]->sType;
    mode->parameters.refreshRate = pCreateInfo->parameters.refreshRate;
    mode->parameters.visibleRegion.width = pCreateInfo->parameters.visibleRegion.width;
    mode->parameters.visibleRegion.height = pCreateInfo->parameters.visibleRegion.height;
    *pMode = (VkDisplayModeKHR)(uintptr_t)mode;

    return VK_SUCCESS;
}


static VkResult fbdevGetDisplayPlaneCapabilities(
    VkPhysicalDevice                    physicalDevice,
    VkDisplayModeKHR                    mode,
    uint32_t                            planeIndex,
    VkDisplayPlaneCapabilitiesKHR*      pCapabilities)
{
    __vkDisplayModeKHR *m = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkDisplayModeKHR *, mode);

    pCapabilities->supportedAlpha = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
    pCapabilities->minSrcPosition = (VkOffset2D) {0, 0};
    pCapabilities->maxSrcPosition = (VkOffset2D) {0, 0};
    pCapabilities->minSrcExtent   = m->parameters.visibleRegion;
    pCapabilities->maxSrcExtent   = m->parameters.visibleRegion;
    pCapabilities->minDstPosition = (VkOffset2D) {0, 0};
    pCapabilities->maxDstPosition = (VkOffset2D) {0, 0};
    pCapabilities->minDstExtent   = m->parameters.visibleRegion;
    pCapabilities->maxDstExtent   = m->parameters.visibleRegion;

    return VK_SUCCESS;
}

/* Surface. */
static VkResult fbdevGetPhysicalDeviceSurfaceSupport(
    VkPhysicalDevice                    physicalDevice,
    uint32_t                            queueFamilyIndex,
    VkSurfaceKHR                        surface,
    VkBool32*                           pSupported)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;

    *pSupported = VK_FALSE;

    if (queueFamilyIndex <= phyDev->queueFamilyCount)
    {
        *pSupported = phyDev->queuePresentSupported[queueFamilyIndex];
    }

    return VK_SUCCESS;
}

static VkResult fbdevGetPhysicalDeviceSurfaceCapabilities(
    VkPhysicalDevice                    physicalDevice,
    VkSurfaceKHR                        surface,
    VkSurfaceCapabilitiesKHR*           pSurfaceCapabilities)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkIcdSurfaceDisplay *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceDisplay *, surface);
    __vkFbdevDisplayPlane *plane = (__vkFbdevDisplayPlane *) phyDev->displayPlanes[surf->planeIndex];

    pSurfaceCapabilities->minImageCount             = plane->minImageCount;
    pSurfaceCapabilities->maxImageCount             = plane->maxImageCount;
    pSurfaceCapabilities->maxImageArrayLayers       = 1;
    pSurfaceCapabilities->currentExtent.width       = plane->varInfo.xres;
    pSurfaceCapabilities->currentExtent.height      = plane->varInfo.yres;
    pSurfaceCapabilities->minImageExtent.width      = plane->varInfo.xres;
    pSurfaceCapabilities->minImageExtent.height     = plane->varInfo.yres;
    pSurfaceCapabilities->maxImageExtent.width      = plane->varInfo.xres;
    pSurfaceCapabilities->maxImageExtent.height     = plane->varInfo.yres;
    pSurfaceCapabilities->supportedTransforms       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->currentTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    pSurfaceCapabilities->supportedCompositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    pSurfaceCapabilities->supportedUsageFlags       = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    return VK_SUCCESS;
}

static VkResult fbdevGetPhysicalDeviceSurfaceFormats(
    VkPhysicalDevice                    physicalDevice,
    VkSurfaceKHR                        surface,
    uint32_t*                           pSurfaceFormatCount,
    VkSurfaceFormatKHR*                 pSurfaceFormats)
{
    __vkPhysicalDevice *phyDev = (__vkPhysicalDevice *)physicalDevice;
    VkIcdSurfaceDisplay *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceDisplay *, surface);
    __vkFbdevDisplayPlane *plane = (__vkFbdevDisplayPlane *) phyDev->displayPlanes[surf->planeIndex];

    if (pSurfaceFormats)
    {
        uint32_t i;

        if (*pSurfaceFormatCount > plane->supportedFormatCount)
            *pSurfaceFormatCount = plane->supportedFormatCount;

        for (i = 0; i < *pSurfaceFormatCount; i++)
        {
            pSurfaceFormats[i].format = plane->supportedFormats[i];
            pSurfaceFormats[i].colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }

        if (*pSurfaceFormatCount < plane->supportedFormatCount)
            return VK_INCOMPLETE;
    }
    else
    {
        *pSurfaceFormatCount = plane->supportedFormatCount;
    }

    return VK_SUCCESS;
}

static VkResult fbdevGetPhysicalDeviceSurfacePresentModes(
    VkPhysicalDevice                    physicalDevice,
    VkSurfaceKHR                        surface,
    uint32_t*                           pPresentModeCount,
    VkPresentModeKHR*                   pPresentModes)
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
static VkResult fbdevGetDeviceGroupSurfacePresentModesKHR(
    VkDevice                            device,
    VkSurfaceKHR                        surface,
    VkDeviceGroupPresentModeFlagsKHR*   pModes
    )
{
    if (pModes)
    {
        *pModes = VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_BIT_KHR;
    }
    return VK_SUCCESS;
}

/* __vkSurfaceOperation::GetPhysicalDevicePresentRectanglesKHR */
static VkResult fbdevGetPhysicalDevicePresentRectanglesKHR(
    VkPhysicalDevice                   physicalDevice,
    VkSurfaceKHR                       surface,
    uint32_t*                          pRectCount,
    VkRect2D*                          pRects
    )
{
    return VK_SUCCESS;
}

/******************************************************************************/

static VkResult __CreateSwapchainCommandBuffer(
    __vkFbdevSwapchainKHR *             sc
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

static VkResult __WrapFbdevBufferMemory(
    VkDevice                            device,
    __vkFbdevDisplayPlane *             plane,
    uint32_t                            bufferIndex,
    const VkAllocationCallbacks *       pAllocator,
    VkDeviceMemory *                    pMemory
    )
{
    VkMemoryAllocateInfo allocInfo;
    __VkMemoryImportInfo importInfo;

    size_t bufferSize;
    uint32_t bufferOffset;

    bufferSize      = plane->fixInfo.line_length * plane->alignedHeight;
    bufferOffset    = bufferSize * bufferIndex;

    allocInfo  = (VkMemoryAllocateInfo) {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext              = NULL,
        .allocationSize     = bufferSize,
        .memoryTypeIndex    = 0
    };

    importInfo = (__VkMemoryImportInfo) {
        .type               = __VK_MEMORY_IMPORT_TYPE_USER_MEMORY,
        .u.usermemory.physical           = plane->fixInfo.smem_start + bufferOffset,
        .u.usermemory.virtAddress        = (void *) ((uintptr_t) plane->userPtr + bufferOffset),
    };

    return __vk_ImportMemory(device, &allocInfo, &importInfo, pAllocator, pMemory);
}

static VkResult __CreateImageBuffer(
    __vkFbdevSwapchainKHR  *            sc,
    uint32_t                            index,
    __vkFbdevImageBuffer *              imageBuffer
    )
{
    VkImageCreateInfo imgInfo;
    VkBufferCreateInfo bufInfo;
    VkMemoryAllocateInfo memAlloc;
    VkDeviceMemory memory;
    uint32_t bufferRowLength;
    uint32_t bufferImageHeight;
    uint32_t bufferSize;
    VkResult result = VK_SUCCESS;

    imageBuffer->swapchain = sc;

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
    __VK_ONERROR(__vk_AllocateMemory(sc->device, &memAlloc, gcvNULL, &memory));

    /* bind memory to image. */
    __VK_ONERROR(__vk_BindImageMemory(sc->device, imageBuffer->renderTarget, memory, 0));

    /* Create the swap chain resolve buffers */
    __GetFbdevBufferSize(sc->plane, NULL, NULL, &bufferRowLength, &bufferImageHeight, &bufferSize);

    __VK_MEMZERO(&bufInfo, sizeof(VkBufferCreateInfo));
    bufInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufInfo.size        = bufferSize;

    imageBuffer->resolveTarget = VK_NULL_HANDLE;
    __VK_ONERROR(__vk_CreateBuffer(sc->device, &bufInfo, gcvNULL, &imageBuffer->resolveTarget));
    __VK_ONERROR(__WrapFbdevBufferMemory(sc->device, sc->plane, index, gcvNULL, &memory));
    __VK_ONERROR(__vk_BindBufferMemory(sc->device, imageBuffer->resolveTarget, memory, 0));

    imageBuffer->bufferRowLength   = bufferRowLength;
    imageBuffer->bufferImageHeight = bufferImageHeight;

    return VK_SUCCESS;

OnError:
    if (imageBuffer->renderTarget)
    {
        if (imageBuffer->renderTargetMemory)
            __vk_FreeMemory(sc->device, imageBuffer->renderTargetMemory, gcvNULL);

        __vk_DestroyImage(sc->device, imageBuffer->renderTarget, gcvNULL);

        imageBuffer->renderTarget       = VK_NULL_HANDLE;
        imageBuffer->renderTargetMemory = VK_NULL_HANDLE;
    }

    if (imageBuffer->resolveTarget)
    {
        __vk_DestroyBuffer(sc->device, imageBuffer->resolveTarget, gcvNULL);

        imageBuffer->resolveTarget = VK_NULL_HANDLE;
    }

    return result;
}

static void __DestroyImageBuffer(
    __vkFbdevImageBuffer *              imageBuffer
    )
{
    __vkFbdevSwapchainKHR *sc = imageBuffer->swapchain;

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
    VkDevice                            device,
    VkSwapchainKHR                      swapchain,
    const VkAllocationCallbacks*        pAllocator
    )
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFbdevSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFbdevSwapchainKHR *, swapchain);
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (sc->cmdPool)
        __vk_DestroyCommandPool(sc->device, sc->cmdPool, gcvNULL);

    if (sc->imageBuffers)
    {
        uint32_t i;

        for (i = 0; i < sc->imageCount; i++)
        {
            __DestroyImageBuffer(&sc->imageBuffers[i]);
        }
        __VK_FREE(sc->imageBuffers);
    }

    __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
}

/* __vkSwapchainKHR::GetSwapchainImages. */
static VkResult __GetSwapchainImages(
    VkDevice                            device,
    VkSwapchainKHR                      swapchain,
    uint32_t*                           pSwapchainImageCount,
    VkImage*                            pSwapchainImages
    )
{
    __vkFbdevSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFbdevSwapchainKHR *, swapchain);

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
            return VK_INCOMPLETE;
    }
    else
    {
        *pSwapchainImageCount = sc->imageCount;
    }

    return VK_SUCCESS;
}

/* __vkSwapchainKHR::AcquireNextImage. */
static VkResult __AcquireNextImage(
    VkDevice                            device,
    VkSwapchainKHR                      swapchain,
    uint64_t                            timeout,
    VkSemaphore                         semaphore,
    VkFence                             fence,
    uint32_t*                           pImageIndex
    )
{
    __vkFbdevSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFbdevSwapchainKHR *, swapchain);

    if (sc->expired)
        return VK_ERROR_OUT_OF_DATE_KHR;

    ++sc->currentImageIndex;
    if (sc->currentImageIndex >= sc->imageCount)
        sc->currentImageIndex = 0;

    if (semaphore)
    {
        __vk_SetSemaphore(device, semaphore, VK_TRUE);
    }

    if (fence)
    {
        __vkFence *fce = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFence *, fence);
        gcoOS_Signal(gcvNULL, fce->signal, VK_TRUE);
    }

    sc->imageBuffers[sc->currentImageIndex].acquired = VK_TRUE;
    *pImageIndex = sc->currentImageIndex;
    return VK_SUCCESS;
}

static VkResult __GenPresentCommand(
    __vkDevContext *                    devCtx,
    __vkFbdevSwapchainKHR *             sc,
    __vkFbdevImageBuffer *              imageBuffer
    )
{
    VkResult result = VK_SUCCESS;
    __vkBlitRes srcRes, dstRes;
    __vkImage *pSrcImg = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkImage *, imageBuffer->renderTarget);

    VkCommandBufferBeginInfo cbi = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext            = NULL,
        .flags            = 0,
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
        VK_FALSE,
        VK_FILTER_NEAREST,
        VK_TRUE
        ));

    __VK_ONERROR(__vk_EndCommandBuffer(sc->cmdBuf));

OnError:
    return result;
}

static VkResult __CommitPresentCommand(
    VkQueue                             queue,
    __vkFbdevSwapchainKHR *             sc,
    uint32_t                            imageIndex
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

    __VK_ONERROR(__vk_QueueIdle((__vkDevQueue *)queue));
    /* Post fb. */
    __PostFbdevDisplayPlaneImage(sc->plane, imageIndex);

OnError:
    return result;
}

/* __vkSwapchainKHR::QueuePresentSingle. */
static VkResult __QueuePresentSingle(
    VkQueue                             queue,
    const VkDisplayPresentInfoKHR*      pDisplayPresentInfo,
    VkSwapchainKHR                      swapchain,
    uint32_t                            imageIndex
    )
{
    VkResult result = VK_SUCCESS;
    __vkFbdevSwapchainKHR *sc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFbdevSwapchainKHR *, swapchain);
    __vkDevContext *devCtx = (__vkDevContext *)sc->device;
    __vkFbdevImageBuffer *imageBuffer = &sc->imageBuffers[imageIndex];

    imageBuffer->acquired = VK_FALSE;

    /* Generate queue commands. */
    __VK_ONERROR(__GenPresentCommand(devCtx, sc, imageBuffer));
    __VK_ONERROR(__CommitPresentCommand(queue, sc, imageIndex));

OnError:
    return result;
}

static VkResult fbdevCreateSwapchain(
    VkDevice                            device,
    const VkSwapchainCreateInfoKHR*     pCreateInfo,
    const VkAllocationCallbacks*        pAllocator,
    VkSwapchainKHR*                     pSwapchain)
{
    __vkDevContext *devCtx = (__vkDevContext *)device;
    __vkFbdevSwapchainKHR *sc = NULL;
    uint32_t i;
    VkResult result = VK_SUCCESS;
    __vkPhysicalDevice *phyDev = devCtx->pPhyDevice;
    VkIcdSurfaceDisplay *surf = __VK_NON_DISPATCHABLE_HANDLE_CAST(VkIcdSurfaceDisplay *, pCreateInfo->surface);
    __vkFbdevDisplayPlane *plane = (__vkFbdevDisplayPlane *) phyDev->displayPlanes[surf->planeIndex];
    __vkFbdevDisplayPlaneParameter parameter;

    /* Set the allocator to the parent allocator or API defined allocator if valid */
    __VK_SET_API_ALLOCATIONCB(&devCtx->memCb);

    if (pCreateInfo->oldSwapchain)
    {
        __vkFbdevSwapchainKHR *oldSc = __VK_NON_DISPATCHABLE_HANDLE_CAST(__vkFbdevSwapchainKHR *, pCreateInfo->oldSwapchain);
        VkBool32 acquired = VK_FALSE;

        if (oldSc->imageBuffers)
        {
            for (i = 0; i < oldSc->imageCount; i++)
            {
                __vkFbdevImageBuffer *imageBuffer = &oldSc->imageBuffers[i];

                if (imageBuffer->acquired)
                {
                    acquired = VK_TRUE;
                    continue;
                }

                __DestroyImageBuffer(imageBuffer);
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
    __VK_ONERROR(__vk_CreateObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, sizeof(__vkFbdevSwapchainKHR), (__vkObject**)&sc));

    sc->device              = device;
    sc->surface             = surf;
    sc->plane               = plane;

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
    sc->currentImageIndex   = 0;

    __VK_ONERROR(__CreateSwapchainCommandBuffer(sc));

    parameter = (__vkFbdevDisplayPlaneParameter) {
        surf->displayMode,
        surf->planeStackIndex,
        surf->transform,
        surf->globalAlpha,
        surf->alphaMode,
        surf->imageExtent,
        pCreateInfo->minImageCount,
        pCreateInfo->imageFormat,
        pCreateInfo->imageColorSpace,
        pCreateInfo->imageArrayLayers,
        pCreateInfo->imageUsage,
        pCreateInfo->imageSharingMode,
        pCreateInfo->preTransform,
        pCreateInfo->compositeAlpha,
        pCreateInfo->presentMode,
        pCreateInfo->clipped
    };

    __VK_ONERROR(__SetFbdevDisplayPlaneParameter(plane, &parameter));

    /* Create swap chain images */
    sc->imageBuffers = (__vkFbdevImageBuffer *)__VK_ALLOC(
        (sc->minImageCount * sizeof(__vkFbdevImageBuffer)), 8, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);

    if (!sc->imageBuffers)
    {
        result = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto OnError;
    }

    __VK_MEMZERO(sc->imageBuffers, sc->minImageCount * sizeof(__vkFbdevImageBuffer));

    for (i = 0; i < sc->minImageCount; i++)
    {
        __VK_ONERROR(__CreateImageBuffer(sc, i, &sc->imageBuffers[i]));
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
                __DestroyImageBuffer(&sc->imageBuffers[i]);
            }
            __VK_FREE(sc->imageBuffers);
        }

        __vk_DestroyObject(devCtx, __VK_OBJECT_SWAPCHAIN_KHR, (__vkObject *)sc);
    }
    return result;
}


static VkResult __vkInitializePhysicalDeviceDisplaysDefault(
    __vkPhysicalDevice *                phyDev)
{
    uint32_t i, j;
    char fbPath[32];
    char * fbPathTemplate[] = {"/dev/fb%d", "/dev/graphics/fb%d"};

    /* Go through displays. */
    for (i = 0; i < __VK_COUNTOF(__vkFbdevDisplayMapping); i++)
    {
        __vkDisplayKHR *display = NULL;

        /* Go through display planes. */
        for (j = 0; j < __vkFbdevDisplayMapping[i].planeCount; j++)
        {
            uint32_t k;
            __vkFbdevDisplayPlane *plane = NULL;
            uint32_t planeIndex = __vkFbdevDisplayMapping[i].fbdevIndex[j];

            for (k = 0; k < __VK_COUNTOF(fbPathTemplate); k++)
            {
                snprintf(fbPath, sizeof(fbPath), fbPathTemplate[k], planeIndex);
                plane = __CreateFbdevDisplayPlane(phyDev, fbPath, phyDev->numberOfDisplayPlanes);

                if (plane)
                {
                    phyDev->displayPlanes[phyDev->numberOfDisplayPlanes++] = (__vkDisplayPlane *) plane;
                    break;
                }
            }

            if (!plane)
            {
                continue;
            }

            if (!display)
            {
                /* The first plane is the graphics plane, create display. */
                display = __CreateFbdevDisplay(phyDev, plane);
                phyDev->displays[phyDev->numberOfDisplays++] = (__vkDisplayKHR *)display;

                display->CreateDisplayMode                     = fbdevCreateDisplayMode;
                display->GetDisplayPlaneCapabilities           = fbdevGetDisplayPlaneCapabilities;
                display->GetPhysicalDeviceSurfaceSupport       = fbdevGetPhysicalDeviceSurfaceSupport;
                display->GetPhysicalDeviceSurfaceCapabilities  = fbdevGetPhysicalDeviceSurfaceCapabilities;
                display->GetPhysicalDeviceSurfaceFormats       = fbdevGetPhysicalDeviceSurfaceFormats;
                display->GetPhysicalDeviceSurfacePresentModes  = fbdevGetPhysicalDeviceSurfacePresentModes;
                display->GetDeviceGroupSurfacePresentModesKHR  = fbdevGetDeviceGroupSurfacePresentModesKHR;
                display->GetPhysicalDevicePresentRectanglesKHR = fbdevGetPhysicalDevicePresentRectanglesKHR;
                display->CreateSwapchain                       = fbdevCreateSwapchain;
            }
            else
            {
                display->planeStack[display->planeStackSize++] = (__vkDisplayPlane *)plane;
            }

            plane->base.currentDisplay = (VkDisplayKHR)(uintptr_t)display;
            plane->base.currentStackIndex = planeIndex;
            plane->base.supportedDisplays[plane->base.supportedDisplayCount++] = (__vkDisplayKHR *)display;

        }
    }

    return VK_SUCCESS;
}

VkResult __vkInitializePhysicalDeviceDisplays(
    __vkPhysicalDevice *                phyDevice)
    __attribute__((weak,alias("__vkInitializePhysicalDeviceDisplaysDefault")));


