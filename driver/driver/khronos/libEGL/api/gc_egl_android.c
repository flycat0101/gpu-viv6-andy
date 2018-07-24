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


#include <gc_egl_precomp.h>

/*
 * To enable verbose messages, see cutils/log.h.
 * Must be included before log.h
 */
#define LOG_NDEBUG 1

#define ANDROID_DUMMY (31415926)

#include <cutils/log.h>

#if ANDROID_SDK_VERSION >= 16
#  include <ui/ANativeObjectBase.h>
#  undef LOGE
#  undef LOGW
#  undef LOGI
#  undef LOGD
#  undef LOGV
#  define LOGE(...) ALOGE(__VA_ARGS__)
#  define LOGW(...) ALOGW(__VA_ARGS__)
#  define LOGI(...) ALOGI(__VA_ARGS__)
#  define LOGD(...) ALOGD(__VA_ARGS__)
#  define LOGV(...) ALOGV(__VA_ARGS__)
#else
#  include <ui/android_native_buffer.h>
#  include <ui/egl/android_natives.h>
#endif

#if ANDROID_SDK_VERSION >= 17
#  include <sync/sync.h>
#endif

#include <hardware/gralloc.h>
#include <errno.h>

#if gcdENABLE_VG
#  include <gc_hal_engine_vg.h>
#endif

#ifdef DRM_GRALLOC
#  include <drm.h>
#  include <vivante_drm.h>
#  include <vivante_bo.h>
#  include <gralloc_vivante_bo.h>
#  include <gralloc_handle.h>
#  include "gc_hal_user.h"
#else
#  include <gc_gralloc_priv.h>
#endif

#include <graphics_ext.h>

typedef struct ANativeWindow *       PlatformWindowType;
typedef struct egl_native_pixmap_t * PlatformPixmapType;
typedef void *                       PlatformDisplayType;


#define ANDROID_DUMMY (31415926)

/******************************************************************************/
/* Display. */

static hw_module_t const* gralloc = gcvNULL;

static void *
_GetDefaultDisplay(
    void
    )
{
    /* Just a silly, non-zero value. */
    void * display = (void *) ANDROID_DUMMY;
    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    /* Nothing to do. */
    if (Display == (void *) ANDROID_DUMMY)
    {
        return;
    }
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    const unsigned int NUM_DISPLAYS = 1;

    /* See frameworks/native/opengl/libs/EGL/eglApi.cpp. */
    uintptr_t index = (uintptr_t) Display;

    if (index >= NUM_DISPLAYS)
    {
        return gcvFALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;
    int err;
    gctPOINTER pointer;

    LOGV("%s: dpy=%p display_id=%p", __func__, Display, Display->hdc);
    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &gralloc);

    if (err || !gralloc)
    {
        /* Failed to get gralloc module. */
        LOGE("%s: Failed open gralloc module", __func__);
        return EGL_FALSE;
    }

    Display->localInfo = (gctPOINTER) gralloc;
    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    LOGV("%s: dpy=%p", __func__, Display);
    return EGL_TRUE;
}

/*
 * These format are removed in system/graphics.h after Kitkat, but
 * are still defined in ui/PixelFormat.h.
 * Keep for compatibility.
 */
#if ANDROID_SDK_VERSION >= 19
#  define HAL_PIXEL_FORMAT_RGBA_5551  6
#  define HAL_PIXEL_FORMAT_RGBA_4444  7
#endif

static const struct
{
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;

    int luminanceSize;

    gceSURF_FORMAT halFormat;
}
_AndroidPixelFormats[] =
{
    {0, 0, 0, 0, 0, gcvSURF_UNKNOWN},  /* PIXEL_FORMAT_NONE. */
    {8, 8, 8, 8, 0, gcvSURF_A8B8G8R8}, /* HAL_PIXEL_FORMAT_RGBA_8888 */
    {8, 8, 8, 0, 0, gcvSURF_X8B8G8R8}, /* HAL_PIXEL_FORMAT_RGBX_8888 */
    {8, 8, 8, 0, 0, gcvSURF_UNKNOWN},  /* HAL_PIXEL_FORMAT_RGB_888, not supported */
    {5, 6, 5, 0, 0, gcvSURF_R5G6B5},   /* HAL_PIXEL_FORMAT_RGB_565 */
    {8, 8, 8, 8, 0, gcvSURF_A8R8G8B8}, /* HAL_PIXEL_FORMAT_BGRA_8888 */
    {5, 5, 5, 1, 0, gcvSURF_R5G5B5A1}, /* HAL_PIXEL_FORMAT_RGBA_5551 */
    {4, 4, 4, 4, 0, gcvSURF_R4G4B4A4}, /* HAL_PIXEL_FORMAT_RGBA_4444 */
    {0, 0, 0, 8, 0, gcvSURF_A8},       /* PIXEL_FORMAT_A8 */
    {0, 0, 0, 0, 8, gcvSURF_L8},       /* PIXEL_FORMAT_L8 */
    {0, 0, 0, 8, 8, gcvSURF_A8L8},     /* PIXEL_FORMAT_LA_88 */
};

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint id = 0;
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    gcmASSERT(Config != gcvNULL);

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    switch (patchId)
    {
    case gcvPATCH_TEMPLERUN:
        id = HAL_PIXEL_FORMAT_RGB_565;
        return id;

    default:
        break;
    }

    switch (Config->bufferSize)
    {
    case 16:
        id = (Config->greenSize == 6) ? HAL_PIXEL_FORMAT_RGB_565
           : (Config->greenSize == 5) ? HAL_PIXEL_FORMAT_RGBA_5551
           : HAL_PIXEL_FORMAT_RGBA_4444;
        break;

    case 32:
        id = (Config->alphaSize == 0) ? HAL_PIXEL_FORMAT_RGBX_8888
           : (Config->swizzleRB == EGL_TRUE) ? HAL_PIXEL_FORMAT_BGRA_8888
           : HAL_PIXEL_FORMAT_RGBA_8888;
        break;

    default:
        break;
    }

    LOGV("%s: dpy=%p configId=0x%x R%d-G%d-B%d-A%d-D%d-S%d visual=%d",
         __func__, Display, Config->configId,
         Config->redSize, Config->greenSize,
         Config->blueSize, Config->alphaSize,
         Config->depthSize, Config->stencilSize, id);

    return id;
}

/* Query of swap interval range. */
static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Max,
    OUT EGLint * Min
    )
{
    return EGL_FALSE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */


struct eglWindowInfo
{
    /*
     * Native window buffer count.
     * -1 if not set.
     *  0 when unknown.
     */
    int                 bufferCount;

    /* Native buffer producer(EGL) usage. */
    int                 producerUsage;

    /* Native buffer consumer usage. */
    int                 consumerUsage;

    /* Flag: Is framebuffer surface? */
    EGLBoolean          hwFramebuffer;

    /* Flag: Native concrete type of this window. */
    int                 concreteType;

    /* Flag: If this buffer is to queues to composer. */
    EGLBoolean          queuesToComposer;

    /* Window rendering api. */
    EGLint              api;

    /* First frame? */
    EGLBoolean          initialFrame;

    /* Size of last dequeued window buffer. */
    EGLint              bufferWidth;
    EGLint              bufferHeight;

    /* Current dequeued count. */
    volatile int32_t    dequeueCount;

    /* Framebuffer buffer handles. */
    EGLint              bufferCacheCount;

    struct
    {
        void *          handle;
        int             fd;
        void *          data;

        gcoSURF         surface;
        int             tsFd;
        EGLint          age;
    } bufferCache[32];
};

static int
_CancelBuffer(
    PlatformWindowType Window,
    android_native_buffer_t * Buffer
    )
{
    int rel;

    LOGV("%s: buffer=%p", __func__, Buffer);

#if ANDROID_SDK_VERSION >= 9
    if (Window->cancelBuffer)
    {
#if ANDROID_SDK_VERSION >= 17
        rel = Window->cancelBuffer(Window, Buffer, -1);
#   else
        rel = Window->cancelBuffer(Window, Buffer);
#   endif
    }
    else
    {
#if ANDROID_SDK_VERSION >= 17
        rel = Window->queueBuffer(Window, Buffer, -1);
#   else
        rel = Window->queueBuffer(Window, Buffer);
#   endif
    }

#else

    rel = Window->queueBuffer(Window, Buffer);
#endif

    return rel;
}

static int
_QueueBuffer(
    PlatformWindowType Window,
    android_native_buffer_t * Buffer,
    int FenceFd
    )
{
    int rel;

    LOGV("%s: buffer=%p fenceFd=%d", __func__, Buffer, FenceFd);

#if ANDROID_SDK_VERSION >= 17
    rel = Window->queueBuffer(Window, Buffer, FenceFd);
#   else
    (void) FenceFd;
    rel = Window->queueBuffer(Window, Buffer);
#   endif

    return rel;
}

static int
_DequeueBuffer(
    PlatformWindowType Window,
    android_native_buffer_t ** Buffer
    )
{
    int rel;
    EGLint tryCount;

#if ANDROID_SDK_VERSION >= 17
    int fenceFd;

    /* Dequeue a buffer from native window. */
    for (tryCount = 0; tryCount  < 500; tryCount++)
    {
        rel = Window->dequeueBuffer(Window, Buffer, &fenceFd);

#if ANDROID_SDK_VERSION >= 20
        if (rel != -ENOSYS)
#   else
        if (rel != -EBUSY)
#   endif
        {
            break;
        }

        gcoOS_Delay(gcvNULL, tryCount / 2);
    }

    if (rel != 0)
    {
        return rel;
    }

    LOGV("%s: buffer=%p fenceFd=%d", __func__, *Buffer, fenceFd);

    if (fenceFd != -1)
    {
#if (gcdANDROID_NATIVE_FENCE_SYNC >= 2)
        gceSTATUS status;

        /* GPU wait for 2000 ms. */
        status = gcoOS_WaitNativeFence(gcvNULL, fenceFd, 2000);

        if (status == gcvSTATUS_TIMEOUT)
        {
            /* Print a warning. */
            LOGW("%s: Warning: wait for fence fd=%d", __func__, fenceFd);

            /* Wait for ever. */
            status = gcoOS_WaitNativeFence(gcvNULL, fenceFd, gcvINFINITE);
        }
#   else
        /* Wait for 2000 ms. */
        rel = sync_wait(fenceFd, 2000);

        if (rel < 0 && errno == ETIME)
        {
            /* Print a warning. */
            LOGW("%s: Warning: wait for fence fd=%d", __func__, fenceFd);

            /* Wait for ever. */
            rel = sync_wait(fenceFd, -1);
        }
#   endif

        /* Close fence fd. */
        close(fenceFd);
    }

#else
    /* Dequeue a buffer from native window. */
    for (tryCount = 0; tryCount  < 500; tryCount++)
    {
        rel = Window->dequeueBuffer(Window, Buffer);

        if (rel != -EBUSY)
        {
            break;
        }

        gcoOS_Delay(gcvNULL, tryCount / 2);
    }

    if (rel != 0)
    {
        LOGE("%s: Failed to dequeue buffer: errno=%d", __func__, rel);
        return rel;
    }

    /* Lock buffer before rendering. */
    if (Window->lockBuffer(Window, *Buffer))
    {
        LOGE("%s: Failed to lock the window buffer", __func__);

        /* Cancel this buffer. */
        _CancelBuffer(Window, *Buffer);

        return -EINVAL;
    }
#endif

    return 0;
}

static int
_TryDequeueBuffer(
    PlatformWindowType Window,
    android_native_buffer_t ** Buffer
    )
{
    int rel;

#if ANDROID_SDK_VERSION >= 17
    int fenceFd;

    /* Dequeue a buffer from native window. */
    rel = Window->dequeueBuffer(Window, Buffer, &fenceFd);

    if (rel != 0)
    {
        /* Expected failure in some conditions. */
        LOGV("%s: Failed to dequeue buffer: errno=%d", __func__, rel);
        return rel;
    }

    LOGV("%s: buffer=%p fenceFd=%d", __func__, *Buffer, fenceFd);

    if (fenceFd != -1)
    {
        /* Wait for 2000 ms. */
        rel = sync_wait(fenceFd, 2000);

        if (rel < 0 && errno == ETIME)
        {
            /* Print a warning. */
            LOGW("%s: wait for fence fd=%d", __func__, fenceFd);

            /* Wait for ever. */
            rel = sync_wait(fenceFd, -1);
        }

        /* Close fence fd. */
        close(fenceFd);
    }

#else
    /* Dequeue a buffer from native window. */
    rel = Window->dequeueBuffer(Window, Buffer);

    if (rel != 0)
    {
        /* Expected failure in some conditions. */
        LOGV("%s: Failed to dequeue buffer: errno=%d", __func__, rel);
        return rel;
    }

    /* Lock buffer before rendering. */
    if (Window->lockBuffer(Window, *Buffer))
    {
        LOGE("%s: Failed to lock the window buffer", __func__);

        /* Cancel this buffer. */
        _CancelBuffer(Window, *Buffer);

        return -EINVAL;
    }
#endif

    return 0;
}

static gceSURF_FORMAT
_TranslateFormat(
    IN int Format
    )
{
    if (Format > 0 && Format < (int)gcmCOUNTOF(_AndroidPixelFormats))
    {
        return _AndroidPixelFormats[Format].halFormat;
    }
    else
    {
        switch (Format)
        {
        case HAL_PIXEL_FORMAT_YV12:
            return gcvSURF_YV12;
#if ANDROID_SDK_VERSION >= 18
        case HAL_PIXEL_FORMAT_YCbCr_420_888:
            return gcvSURF_NV12;
#endif
#if ANDROID_SDK_VERSION >= 24
        case HAL_PIXEL_FORMAT_YCbCr_422_888:
            return gcvSURF_YUY2;
#endif
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
            return gcvSURF_NV16;
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            return gcvSURF_NV21;
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            return gcvSURF_YUY2;
#if ANDROID_SDK_VERSION >= 26
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return gcvSURF_R10G10B10A2;
        case HAL_PIXEL_FORMAT_RGBA_FP16:
            return gcvSURF_A16B16G16R16F;
#endif
#if ANDROID_SDK_VERSION >= 25
        case HAL_PIXEL_FORMAT_RAW16:
            return gcvSURF_A16;
        case HAL_PIXEL_FORMAT_Y16:
            return gcvSURF_A16;
        case HAL_PIXEL_FORMAT_Y8:
            return gcvSURF_A8;
#endif
#if ANDROID_SDK_VERSION >= 17
        case HAL_PIXEL_FORMAT_BLOB:
            return gcvSURF_A8;
#endif
        /* graphics_ext. */
#ifdef FSL_YUV_EXT
        case HAL_PIXEL_FORMAT_YCbCr_422_P:
            return gcvSURF_UNKNOWN;
        case HAL_PIXEL_FORMAT_YCbCr_420_P:
            return gcvSURF_I420;
        case HAL_PIXEL_FORMAT_CbYCrY_422_I:
            return gcvSURF_UYVY;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
#if ANDROID_SDK_VERSION >= 27
        case HAL_PIXEL_FORMAT_NV12_TILED:
        case HAL_PIXEL_FORMAT_NV12_G1_TILED:
        case HAL_PIXEL_FORMAT_NV12_G2_TILED:
        case HAL_PIXEL_FORMAT_NV12_G2_TILED_COMPRESSED:
        case HAL_PIXEL_FORMAT_P010:
        case HAL_PIXEL_FORMAT_P010_TILED:
        case HAL_PIXEL_FORMAT_P010_TILED_COMPRESSED:
#endif
            return gcvSURF_NV12;
#endif
        default:
            LOGE("%s: unknown android format=%x", __func__, Format);
            return gcvSURF_UNKNOWN;
        }
    }
}

static EGLBoolean
_GetWindowProperties(
    IN PlatformWindowType Window,
    IN VEGLWindowInfo Info
    )
{
#if ANDROID_SDK_VERSION >= 11
    /*
     * Determine buffer count to be set for native window.
     * Only available for Honeycomb and later
     */
    PlatformWindowType win = Window;

    /* Query concrete type. */
    win->query(win, NATIVE_WINDOW_CONCRETE_TYPE, &Info->concreteType);

#if ANDROID_SDK_VERSION >= 19
    /* Query consumer usage bits. */
    win->query(win, NATIVE_WINDOW_CONSUMER_USAGE_BITS, &Info->consumerUsage);
#endif

    if (Info->concreteType == NATIVE_WINDOW_FRAMEBUFFER)
    {
        Info->hwFramebuffer    = EGL_TRUE;
        Info->queuesToComposer = EGL_FALSE;

        /*
         * ui/FramebufferNativeWindow has a not initialized pointer for
         * 'cancelBuffer'. Force 'queueBuffer' to avoid crash.
         */
        win->cancelBuffer = NULL;

        Info->bufferWidth   = 0;
        Info->bufferHeight  = 0;
        LOGV("%s: NATIVE_WINDOW_FRAMEBUFFER", __func__);
    }
    else
    {
        /*
         * On jellybean-4.2 and later, Framebuffer surface can be
         * NATIVE_WINDOW_SURFACE.
         */
        int rel;
        android_native_buffer_t *buffer;
        int flags, usage;

        /* Dequeue first buffer. */
        rel = _TryDequeueBuffer(win, &buffer);

        if (rel)
        {
            LOGE("%s: Failed to dequeue buffer", __func__);
            return EGL_FALSE;
        }

        /* Update latest buffer size. */
        Info->bufferWidth  = buffer->width;
        Info->bufferHeight = buffer->height;

        /* Get alloc flags and usage (combined consumer and producer usage). */
#ifdef DRM_GRALLOC
        flags = 0;
        usage = buffer->usage;
#else
        flags = ((struct private_handle_t*)buffer->handle)->flags;
        usage = gc_native_handle_get(buffer->handle)->allocUsage;
#endif

        if (Info->consumerUsage == 0)
        {
            /* Treat combined usage as consumer usage. */
            Info->consumerUsage = usage;
        }

        if ((flags & 0x0001 /* PRIV_FLAGS_FRAMEBUFFER */) ||
            (Info->consumerUsage & GRALLOC_USAGE_HW_FB) ||
            (Info->consumerUsage & (GRALLOC_USAGE_HW_FB << 1)))
        {
            Info->hwFramebuffer    = EGL_TRUE;
            Info->queuesToComposer = EGL_FALSE;
            LOGV("%s: FramebufferSurface", __func__);
        }
        else
        {
#if ANDROID_SDK_VERSION < 14
            /* Unknown. Will query later. */
            Info->queuesToComposer = (EGLBoolean) ~0U;
#   else
            /* Queues to composer if has HW_COMPOSER usage. */
            Info->queuesToComposer = (usage & GRALLOC_USAGE_HW_COMPOSER)
                                   ? EGL_TRUE : EGL_FALSE;
#   endif

            Info->hwFramebuffer    = EGL_FALSE;

            LOGV("%s: Surface, QueuesToComposer=%d",
                 __func__, Info->queuesToComposer);
        }

        /* Cancel the buffer. */
        _CancelBuffer(win, buffer);
    }

#else

    Info->queuesToComposer = EGL_FALSE;
    Info->hwFramebuffer    = EGL_TRUE;
#endif

    return EGL_TRUE;
}

#if ANDROID_SDK_VERSION >= 11
static struct
{
    gcePATCH_ID patchId;
    int bufferCount;
}
bufferCountList[] =
{
    {gcvPATCH_NENAMARK2,    6},
    {gcvPATCH_GLBM25,       6},
    {gcvPATCH_GLBM21,       6},
    {gcvPATCH_GLBM11,       6},
    {gcvPATCH_QUADRANT,     4},
};

#endif

static void
_SetBufferCount(
    IN VEGLDisplay Display,
    IN PlatformWindowType Window,
    IN VEGLWindowInfo Info
    )
{
    /*
     * Determine buffer count to be set for native window.
     * Only available for Honeycomb and later
     */
#if ANDROID_SDK_VERSION >= 11
    PlatformWindowType win = Window;

    if (Info->concreteType == NATIVE_WINDOW_FRAMEBUFFER)
    {
        /*
         * FramebufferNativeWindow, buffer count should be MAX_NUM_FRAME_BUFFERS
         * defined in frameworks/native/include/ui/FramebufferNativeWindow.h.
         */
        Info->bufferCount = 3;
        LOGI("FramebufferNativeWindow: bufferCount=%d", Info->bufferCount);
    }
    else if (Info->hwFramebuffer)
    {
#if ANDROID_SDK_VERSION >= 17
        /*
         * On JellyBean-4.2 and later, it could be surfaceflinger/
         * FramebufferSurface used as the hardware framebuffer surface.
         * It is required to set its bufferCount to numFramebuffers, and that
         * should be done at 'Consumer' (ie compositor) side. We do not need to
         * set at 'Producer' (ie, EGL here) side. Especially on android 5.0 and
         * later, the native window will try to use minimal buffers if buffer
         * count set in 'Producer' side. FBdev performance drop found on various
         * platforms in that condition.
         * Hence, do NOT set buffer here in EGL.
         */
        struct framebuffer_device_t* device;
        if (framebuffer_open(gralloc, &device) == 0)
        {
            Info->bufferCount = device->numFramebuffers;
            framebuffer_close(device);
        }
        else
        {
#ifdef NUM_FRAMEBUFFER_SURFACE_BUFFERS
            Info->bufferCount = NUM_FRAMEBUFFER_SURFACE_BUFFERS;
#  else
            Info->bufferCount = 3;
#  endif
        }

        LOGI("FramebufferSurface: bufferCount=%d", Info->bufferCount);
        /* native_window_set_buffer_count(win, Info->bufferCount); */
#endif
    }
    else
    {
        int rel;
        int bufferCount;
        int minUndequeued;
        android_native_buffer_t *buffer;
        android_native_buffer_t *buffer2;

        /* Set default value to 0. */
        Info->bufferCount = 0;


        /* Query minUndequeued buffer count. */
        win->query(win, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minUndequeued);

        /* Determine buffer count to set for the window. */
        bufferCount = minUndequeued + 2;

        /* Dequeue first buffer. */
        rel = _TryDequeueBuffer(win, &buffer);

        if (rel != 0)
        {
            /* This is error case. But let's blindly set buffer count. */
            LOGE("%s: Failed to dequeue buffer", __func__);

            /* Set native buffer count. */
            native_window_set_buffer_count(win, bufferCount);
            Info->bufferCount = bufferCount;
            return;
        }

        /* Update latest buffer size. */
        Info->bufferWidth  = buffer->width;
        Info->bufferHeight = buffer->height;

        /* Try dequeue second buffer. */
        rel = _TryDequeueBuffer(win, &buffer2);

        if (rel != 0)
        {
            gctUINT i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;

            gcoHAL_GetPatchID(gcvNULL, &patchId);

            for (i = 0; i < gcmCOUNTOF(bufferCountList); i++)
            {
                if (patchId == bufferCountList[i].patchId)
                {
                    /* Get per app buffer count. */
                    bufferCount = bufferCountList[i].bufferCount;
                    break;
                }
            }

            /* From libgui:
             * dequeueBuffer: can't dequeue multiple buffers without
             * setting the buffer count
             * So if EINVAL is returned, buffer count is not ever set. */
            _CancelBuffer(win, buffer);

            /* Set native buffer count. */
            LOGV("%s: Surface: set bufferCount to %d", __func__, bufferCount);
            native_window_set_buffer_count(win, bufferCount);

            /* Store buffer count set for the window. */
            Info->bufferCount = bufferCount;
        }
        else
        {
            LOGV("%s: Surface: bufferCount already set", __func__);

            /* Update latest buffer size. */
            Info->bufferWidth  = buffer2->width;
            Info->bufferHeight = buffer2->height;

            /* Cancel the buffers. */
            _CancelBuffer(win, buffer);
            _CancelBuffer(win, buffer2);

            /* Set buffer count to 0. */
            Info->bufferCount = 0;
        }
    }

    LOGV("%s: win=%p bufferCount=%d", __func__, win, Info->bufferCount);
#endif
}

static void
_UnsetBufferCount(
    IN VEGLDisplay Display,
    IN PlatformWindowType Window,
    IN VEGLWindowInfo Info
    )
{
#if ANDROID_SDK_VERSION >= 11
    if (Info->bufferCount > 0)
    {
        /* Unset buffer count if ever set. */
        native_window_set_buffer_count(Window, 0);
        Info->bufferCount = -1;
    }
#   endif
}

static void
_ApiConnect(
    IN PlatformWindowType Window
    )
{
#if ANDROID_SDK_VERSION >= 14
    PlatformWindowType win  = Window;
    VEGLThreadData thread = veglGetThreadData();

    if (thread && thread->api == EGL_OPENVG_API)
    {
        native_window_api_connect(win, NATIVE_WINDOW_API_EGL);
    }
#endif
}

static void
_ApiDisconnect(
    IN PlatformWindowType Window
    )
{
#if ANDROID_SDK_VERSION >= 14
    PlatformWindowType win  = Window;
    VEGLThreadData thread = veglGetThreadData();

    if (thread && (thread->api == EGL_OPENVG_API))
    {
        /*
         * Disconnect EGL API for OpenVG.
         * See _ApiConnect above.
         */
        native_window_api_disconnect(win, NATIVE_WINDOW_API_EGL);
    }
#endif
}

#ifdef DRM_GRALLOC

static gceSURF_FORMAT
_TranslateANativeBufferFormat(
    android_native_buffer_t * Buffer,
    EGLint RenderMode
    )
{
    if (Buffer->usage & GRALLOC_USAGE_TILED_VIV)
    {
        /* Use internal format for direct rendering. */
        switch (Buffer->format)
        {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return gcvSURF_A8R8G8B8;

        case HAL_PIXEL_FORMAT_RGBX_8888:
            return gcvSURF_X8R8G8B8;

        case HAL_PIXEL_FORMAT_BGRA_8888:
            return gcvSURF_A8R8G8B8;
            break;

        case HAL_PIXEL_FORMAT_RGB_565:
            return gcvSURF_R5G6B5;
            break;

        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            return gcvSURF_YUY2;

        default:
            ALOGE("%s: unsupported android format: %d",
                  __func__, Buffer->format);
            return gcvSURF_UNKNOWN;
        }
    }
    else
    {
        return _TranslateFormat(Buffer->format);
    }
}

/*
 * Get Vivante bo tiling and matched base type.
 * Base type could be:
 * * gcvSURF_BITMAP
 * * gcvSURF_TEXTURE
 * * gcvSURF_RENDER_TARGET
 * * gcvSURF_RENDER_TARGET_NO_TILE_STATUS
 *
 * EGL_TRUE on success.
 *   Out type gcvSURF_TYPE_UNKNOWN if can not find a matched type.
 * EGL_FALSE on error.
 */
static EGLBoolean
_GetVivanteDrmBufferBaseType(
    struct gralloc_vivante_bo_t * bo,
    struct drm_vivante_bo_tiling * TilingArgs,
    gceSURF_TYPE * BaseType
    )
{
    gceSURF_TYPE type;
    int err;

    /* GPU buffers. Get tiling information. */
    err = drm_vivante_bo_get_tiling(bo->bo, TilingArgs);

    if (err)
    {
        ALOGE("%s: failed to get bo tiling", __func__);
        return EGL_FALSE;
    }

    /* Determine surface type for GPU buffers from bo tiling. */
    switch (TilingArgs->tiling_mode)
    {
    case DRM_VIV_GEM_TILING_LINEAR:
        type = gcvSURF_BITMAP;
        break;

    case DRM_VIV_GEM_TILING_TILED:
        type = gcvSURF_TEXTURE;
        break;

    case DRM_VIV_GEM_TILING_SUPERTILED:
        type = (TilingArgs->ts_mode == DRM_VIV_GEM_TS_NONE) ?
               gcvSURF_RENDER_TARGET_NO_TILE_STATUS : gcvSURF_RENDER_TARGET;
        break;

    default:
        ALOGE("%s: unsupported tiling_mode: %d",
              __func__, TilingArgs->tiling_mode);
        return EGL_FALSE;
    }


    *BaseType = type;
    return EGL_TRUE;
}

/*
 * Get generic drm buffer tiling and matched base type.
 * Base type could be:
 * * gcvSURF_BITMAP
 * * gcvSURF_TEXTURE
 * * gcvSURF_RENDER_TARGET
 * * gcvSURF_RENDER_TARGET_NO_TILE_STATUS
 *
 * EGL_TRUE on success.
 *   Out type gcvSURF_TYPE_UNKNOWN if can not find a matched type.
 * EGL_FALSE on error.
 */
static inline EGLBoolean
_GetGenericDrmBufferBaseType(
    android_native_buffer_t * Buffer,
    gceSURF_TYPE * BaseType
    )
{
    /* TODO: Get base type. */
    if (Buffer->usage & GRALLOC_USAGE_TILED_VIV)
    {
        if (Buffer->usage & GRALLOC_USAGE_TS_VIV)
        {
            *BaseType = gcvSURF_RENDER_TARGET;
        }
        else
        {
            *BaseType = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
        }
    }
    else
    {
        *BaseType = gcvSURF_BITMAP;
    }

    return EGL_TRUE;
}

/*
 * Push surface tiling information to bo.
 * Return EGL_TRUE on success, EGL_FALSE on failure.
 */
static EGLBoolean
_PushVivanteDrmBufferStatus(
    gcoSURF Surface,
    struct gralloc_vivante_bo_t * bo
    )
{
    struct drm_vivante_bo_tiling tilingArgs;
    uint64_t timeStamp = 0;
    gceTILING tiling = gcvINVALIDTILED;

    gcoSURF_GetTiling(Surface, &tiling);

    switch (tiling)
    {
    case gcvLINEAR:
        tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_LINEAR;
        break;
    case gcvTILED:
        tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_TILED;
        break;
    case gcvSUPERTILED:
        tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_SUPERTILED;
        break;
    default:
        /* Unknown surface tiling. */
        return EGL_FALSE;
    }

    tilingArgs.ts_mode = Surface->tileStatusNode.pool == gcvPOOL_UNKNOWN ? DRM_VIV_GEM_TS_NONE
                       : Surface->tileStatusDisabled[0] ? DRM_VIV_GEM_TS_DISABLED
                       : Surface->compressed ? DRM_VIV_GEM_TS_COMPRESSED
                       : DRM_VIV_GEM_TS_NORMAL;

    tilingArgs.clear_value = ((uint64_t)Surface->fcValueUpper[0]) << 32
                           | Surface->fcValue[0];

    drm_vivante_bo_set_tiling(bo->bo, &tilingArgs);
    drm_vivante_bo_inc_timestamp(bo->bo, &timeStamp);

    return EGL_TRUE;
}

/*
 * Push surface tiling information to drm buffer.
 * Return EGL_TRUE on success, EGL_FALSE on failure.
 */
static EGLBoolean
_PushGenericDrmBufferStatus(
    gcoSURF Surface,
    android_native_buffer_t * Buffer
    )
{
    return EGL_TRUE;
}

/*
 * Pop surface tiling information from bo.
 * Timestamp: In last timeStamp, out updated timeStamp.
 * Returns EGL_TRUE on success, EGL_FALSE on failure.
 */
static EGLBoolean
_PopVivanteDrmBufferStatus(
    struct gralloc_vivante_bo_t * Bo,
    gcoSURF Surface,
    gctUINT64 * TimeStamp
    )
{
    int err;
    struct drm_vivante_bo_tiling tilingArgs;
    uint64_t timeStamp = 0;

    err = drm_vivante_bo_get_tiling(Bo->bo, &tilingArgs);

    if (err)
    {
        /* Error. */
        return EGL_FALSE;
    }

    err = drm_vivante_bo_get_timestamp(Bo->bo, &timeStamp);

    if (err)
    {
        return EGL_FALSE;
    }

    if (timeStamp == *TimeStamp)
    {
        /* surface is not updated. */
        return EGL_TRUE;
    }

    *TimeStamp = timeStamp;

    /* Update tile status information. */
    switch (tilingArgs.ts_mode)
    {
    case DRM_VIV_GEM_TS_COMPRESSED:
        /* Surface->compressed = gcvTRUE; */
    case DRM_VIV_GEM_TS_NORMAL:
        Surface->tileStatusDisabled[0] = gcvFALSE;
        Surface->fcValue[0]      = (gctUINT32)tilingArgs.clear_value;
        Surface->fcValueUpper[0] = (gctUINT32)(tilingArgs.clear_value >> 32);
        break;
    case DRM_VIV_GEM_TS_DISABLED:
        Surface->tileStatusDisabled[0] = gcvTRUE;
        break;
    default:
        break;
    }

    return EGL_TRUE;
}

/*
 * Pop surface tiling information from drm buffer.
 * Timestamp: In last timeStamp, out updated timeStamp.
 * Returns EGL_TRUE on success, EGL_FALSE on failure.
 */
static EGLBoolean
_PopGenericDrmBufferStatus(
    android_native_buffer_t * Buffer,
    gcoSURF Surface,
    gctUINT64 * TimeStamp
    )
{
    /* TODO: Query tiling, ts state, fast-clear value, etc. */
    *TimeStamp++;
    return EGL_TRUE;
}

/*
 * Calibrate buffer type for rendering mode.
 * EGL_TRUE on success.
 * EGL_FALSE if baseType can not support specified rendering mode.
 */
static inline EGLBoolean
_CalibrateSurfaceType(
    gceSURF_TYPE BaseType,
    EGLint RenderMode,
    gceSURF_TYPE * Type
    )
{
    gceSURF_TYPE type = gcvSURF_TYPE_UNKNOWN;

    /*
     * BaseType could be:
     * * gcvSURF_BITMAP
     * * gcvSURF_TEXTURE
     * * gcvSURF_RENDER_TARGET
     * * gcvSURF_RENDER_TARGET_NO_TILE_STATUS
     */
    /* Override type for window buffer according to renderMode. */
    switch (RenderMode)
    {
    case VEGL_INDIRECT_RENDERING:
#if gcdGPU_LINEAR_BUFFER_ENABLED
        if (BaseType == gcvSURF_BITMAP)
        {
            type = gcvSURF_BITMAP;
        }
#else
        if (BaseType == gcvSURF_TEXTURE)
        {
            type = gcvSURF_TEXTURE;
        }
#endif
        break;
    case VEGL_DIRECT_RENDERING_FCFILL:
        if (BaseType == gcvSURF_RENDER_TARGET)
        {
            type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
        }
        else if (BaseType == gcvSURF_RENDER_TARGET_NO_TILE_STATUS)
        {
            /* Need append tile status later. */
            type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
        }
        break;
    case VEGL_DIRECT_RENDERING_NOFC:
        if (BaseType == gcvSURF_RENDER_TARGET ||
            BaseType == gcvSURF_RENDER_TARGET_NO_TILE_STATUS)
        {
            type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
        }
        break;
    case VEGL_DIRECT_RENDERING_FC_NOCC:
        if (BaseType == gcvSURF_RENDER_TARGET)
        {
            type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
        }
        break;
    case VEGL_DIRECT_RENDERING:
        if (BaseType == gcvSURF_RENDER_TARGET)
        {
            type = gcvSURF_RENDER_TARGET;
        }
        break;
    case -1:
    default:
        /* Use base type directly. */
        type = BaseType;
        break;
    }

    if (Type)
    {
        *Type = type;
    }

    return (type != gcvSURF_TYPE_UNKNOWN);
}

static EGLBoolean
_CreateVivanteDrmBufferSurface(
    android_native_buffer_t * Buffer,
    struct gralloc_vivante_bo_t * Bo,
    EGLint RenderMode,
    gceSURF_FORMAT Format,
    gcoSURF * Surface
    )
{
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gceSURF_TYPE baseType;
    gceSURF_TYPE type;
    struct drm_vivante_bo_tiling tilingArgs;
    uint32_t node = 0, tsNode = 0;
    uint64_t param;
    gcePOOL pool;
    gctSIZE_T size;
    int err;

    if (!_GetVivanteDrmBufferBaseType(Bo, &tilingArgs, &baseType))
    {
        return EGL_FALSE;
    }

    if (!_CalibrateSurfaceType(baseType, RenderMode, &type))
    {
        return EGL_FALSE;
    }

    /* Get pool type. */
    err = drm_vivante_bo_query(Bo->bo, DRM_VIV_GEM_PARAM_POOL, &param);

    if (err)
    {
        return EGL_FALSE;
    }
    pool = (gcePOOL)param;

    err = drm_vivante_bo_query(Bo->bo, DRM_VIV_GEM_PARAM_SIZE, &param);

    if (err)
    {
        return EGL_FALSE;
    }
    size = (gctSIZE_T)param;

    /* Indicate display buffer. */
    type |= gcvSURF_CREATE_AS_DISPLAYBUFFER;

    /* Indicate can exported as dma-buf fd */
    type |= gcvSURF_DMABUF_EXPORTABLE;

    /* Append no-vidmem hint to type. */
    type |= gcvSURF_NO_VIDMEM;

    /* Create surface wraper. */
    gcmONERROR(gcoSURF_Construct(gcvNULL,
                                 Buffer->width, Buffer->height, 1,
                                 type, Format, (gcePOOL)pool,
                                 &surface));

    /* Set 128B cache mode. */
    surface->cacheMode = gcvCACHE_128;

    /* Reference and get node. */
    err = drm_vivante_bo_ref_node(Bo->bo, &node, &tsNode);

    if (err)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Assign parameters to surface. */
    surface->size = size;

    surface->node.u.normal.node = node;
    surface->node.pool          = pool;
    surface->node.size          = size; /* ??? */

    /* shared mutex, for what? */
    gcmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &surface->node.sharedMutex));

#if gcdENABLE_3D
    if (tsNode != 0)
    {
        surface->tileStatusNode.u.normal.node = tsNode;
        surface->tileStatusNode.pool          = pool; /* ??? */
        surface->tileStatusNode.size          = size >> 8; /* ??? */

        gcmONERROR(gcoOS_CreateMutex(gcvNULL,
                                     &surface->tileStatusNode.sharedMutex));
    }
#endif

    /* Initial lock. */
    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    /* Set y-inverted rendering. */
    gcmONERROR(gcoSURF_SetFlags(surface,
                                gcvSURF_FLAG_CONTENT_YINVERTED,
                                gcvTRUE));

    if (RenderMode >= 0)
    {
        gceTILING tiling = gcvINVALIDTILED;

        /* Tiling correction. */
        gcoSURF_GetTiling(surface, &tiling);

        switch (tiling)
        {
        case gcvLINEAR:
            tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_LINEAR;
            break;
        case gcvTILED:
            tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_TILED;
            break;
        case gcvSUPERTILED:
            tilingArgs.tiling_mode = DRM_VIV_GEM_TILING_SUPERTILED;
            break;
        default:
            /* Unknown surface tiling. */
            gcoSURF_Destroy(surface);
            return EGL_FALSE;
        }

        /* Set default tile status state for Producer. */
        tilingArgs.ts_mode = surface->tileStatusNode.pool == gcvPOOL_UNKNOWN ? DRM_VIV_GEM_TS_NONE
                           : surface->tileStatusDisabled[0] ? DRM_VIV_GEM_TS_DISABLED
                           : surface->compressed ? DRM_VIV_GEM_TS_COMPRESSED
                           : DRM_VIV_GEM_TS_NORMAL;

        drm_vivante_bo_set_tiling(Bo->bo, &tilingArgs);
    }
    else
    {
        /* Corrent tile status state. */
        surface->tileStatusDisabled[0] =
                (tilingArgs.ts_mode == DRM_VIV_GEM_TS_NONE) ||
                (tilingArgs.ts_mode == DRM_VIV_GEM_TS_DISABLED);
    }

    *Surface = surface;
    return EGL_TRUE;

OnError:
    if (surface)
    {
        gcoSURF_Destroy(surface);
    }

    return EGL_FALSE;
}

static EGLBoolean
_CreateGenericDrmBufferSurface(
    android_native_buffer_t * Buffer,
    EGLint RenderMode,
    gceSURF_FORMAT Format,
    gcoSURF * Surface
    )
{
    gceSTATUS status;
    native_handle_t * handle;
    gceSURF_TYPE baseType;
    gceSURF_TYPE type;
    gctUINT stride = (gctUINT)Buffer->stride;
    gcoSURF surface = gcvNULL;
    int fd;

    if (!_GetGenericDrmBufferBaseType(Buffer, &baseType))
    {
        return EGL_FALSE;
    }

    if (!_CalibrateSurfaceType(baseType, RenderMode, &type))
    {
        return EGL_FALSE;
    }

    /* Translate stride in bytes to in pixels. */
    switch (Format)
    {
    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_X8B8G8R8:
        stride *= 4;
        break;

    case gcvSURF_R5G6B5:
    case gcvSURF_R4G4B4A4:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        stride *= 2;
        break;

    case gcvSURF_NV12:
    case gcvSURF_NV21:
    case gcvSURF_NV16:
    case gcvSURF_NV61:
    case gcvSURF_YV12:
    case gcvSURF_I420:
        break;

    default:
        ALOGE("%s: unknown format=%x", __func__, Format);
        *Surface = gcvNULL;
        return EGL_FALSE;
    }

    handle = (native_handle_t *)Buffer->handle;
    fd = (handle && handle->numFds) ? (int)handle->data[0] : -1;

    if (fd < 0)
    {
        ALOGE("%s: invalid fd=%d", __func__, fd);
        return EGL_FALSE;
    }

    /* Indicate display buffer. */
    type |= gcvSURF_CREATE_AS_DISPLAYBUFFER;

    /* Indicate can exported as dma-buf fd */
    type |= gcvSURF_DMABUF_EXPORTABLE;

    /* TODO: Attach tile status from generic drm buffer. */
    status = gcoSURF_WrapUserMemory(gcvNULL,
                                    Buffer->width,
                                    Buffer->height,
                                    stride,
                                    1,
                                    type,
                                    Format,
                                    (gctUINT32)fd,
                                    gcvALLOC_FLAG_DMABUF,
                                    &surface);

    if (gcmIS_SUCCESS(status))
    {
        gcoSURF_SetFlags(surface,
                         gcvSURF_FLAG_CONTENT_YINVERTED,
                         gcvTRUE);

        /* Set 128B cache mode. */
        surface->cacheMode = gcvCACHE_128;
    }

    if ((RenderMode == VEGL_DIRECT_RENDERING_FCFILL) &&
        (baseType == gcvSURF_RENDER_TARGET_NO_TILE_STATUS))
    {
        status = gcoSURF_AppendTileStatus(surface);
    }

    *Surface = surface;
    return gcmIS_SUCCESS(status) ? EGL_TRUE : EGL_FALSE;
}

static EGLBoolean
_GetANativeBufferSurface(
    android_native_buffer_t * Buffer,
    EGLint RenderMode,
    gcoSURF * Surface
    )
{
    gcoSURF surface = gcvNULL;
    gceSURF_TYPE type;
    gceSURF_FORMAT format;
    EGLBoolean success;

    struct gralloc_vivante_bo_t * bo;
    int err;

    format = _TranslateANativeBufferFormat(Buffer, RenderMode);

    if (format == gcvSURF_UNKNOWN)
    {
        return EGL_FALSE;
    }

    err = gralloc_handle_validate(Buffer->handle);

    if (err)
    {
        /* Invalid handle. */
        ALOGE("%s: invalid buffer=%p", __func__, Buffer);
        return EGL_FALSE;
    }

    /* Try get gralloc_vivante_bo. */
    bo = gralloc_vivante_bo_from_handle(Buffer->handle);

    if (bo)
    {
        /* Create drm buffer by vivante drm interface. */
        success = _CreateVivanteDrmBufferSurface(
                        Buffer, bo, RenderMode, format, &surface);
    }
    else
    {
        /* Create drm buffer by generic drm interface. */
        success = _CreateGenericDrmBufferSurface(
                        Buffer, RenderMode, format, &surface);
    }

    *Surface = surface;
    return success;
}

/*
 * Negotiate render mode with buffer allocator.
 * IN:  prefered render mode.
 * OUT: matched render mode.
 */
static EGLBoolean
_NegotiateRenderMode(
    struct ANativeWindow * Win,
    EGLint PreferRenderMode,
    EGLint * MatchedRenderMode
    )
{
    int err;
    android_native_buffer_t * buffer;
    struct gralloc_vivante_bo_t *bo;
    EGLBoolean success = EGL_FALSE;
    gceSURF_TYPE baseType = gcvSURF_TYPE_UNKNOWN;
    gceSURF_TYPE type = gcvSURF_TYPE_UNKNOWN;
    EGLint renderMode = -1;
    EGLBoolean found = EGL_FALSE;
    gctUINT i;
    const EGLint preferModes[] =
    {
        VEGL_DIRECT_RENDERING,
        VEGL_DIRECT_RENDERING_FC_NOCC,
        VEGL_DIRECT_RENDERING_FCFILL,
        VEGL_INDIRECT_RENDERING,
    };

    err = _DequeueBuffer(Win, &buffer);

    if (err < 0)
    {
        /* Failed to dequeue buffer. */
        return EGL_FALSE;
    }

    buffer->common.incRef(&buffer->common);

    err = gralloc_handle_validate(buffer->handle);

    if (err)
    {
        /* Invalid handle. */
        ALOGE("%s: invalid buffer=%p", __func__, buffer);
        success = EGL_FALSE;
        goto out;
    }

    /* Try get gralloc_vivante_bo. */
    bo = gralloc_vivante_bo_from_handle(buffer->handle);

    if (bo)
    {
        struct drm_vivante_bo_tiling tilingArgs;

        /* Try to get matched surface type for buffer and renderMode. */
        success = _GetVivanteDrmBufferBaseType(bo, &tilingArgs, &baseType);

        (void)tilingArgs;
    }
    else
    {
        success = _GetGenericDrmBufferBaseType(buffer, &baseType);
    }

    if (!success)
    {
        goto out;
    }

    success = EGL_FALSE;

    /* Special for NOFC render mode, use this mode when explicitly reqeusted. */
    if (PreferRenderMode == VEGL_DIRECT_RENDERING_NOFC)
    {
        if (_CalibrateSurfaceType(baseType, VEGL_DIRECT_RENDERING_NOFC, &type))
        {
            renderMode = VEGL_DIRECT_RENDERING_NOFC;
            success = EGL_TRUE;
        }

        /* Do not try other modes. */
        goto out;
    }

    for (i = 0; i < gcmCOUNTOF(preferModes); i++)
    {
        if (preferModes[i] == PreferRenderMode)
        {
            found = EGL_TRUE;
        }

        if (found)
        {
            if (_CalibrateSurfaceType(baseType, preferModes[i], &type))
            {
                renderMode = preferModes[i];
                success = EGL_TRUE;
                break;
            }
        }
    }

out:
    buffer->common.decRef(&buffer->common);
    _CancelBuffer(Win, buffer);

    if (success)
    {
        *MatchedRenderMode = renderMode;
        return EGL_TRUE;
    }

    return EGL_FALSE;
}

#else

static EGLBoolean
_GetANativeBufferSurface(
    android_native_buffer_t * Buffer,
    EGLint RenderMode,
    gcoSURF * Surface
    )
{
    gcoSURF surface;

    if (!Buffer->handle)
    {
        return EGL_FALSE;
    }

    surface = (gcoSURF)gcmUINT64_TO_PTR(
                    gc_native_handle_get(Buffer->handle)->surface);

    if (!surface)
    {
        return EGL_FALSE;
    }

    gcoSURF_ReferenceSurface(surface);

    *Surface = surface;
    return EGL_TRUE;
}

static inline EGLBoolean
_NegotiateRenderMode(
    struct ANativeWindow * Win,
    EGLint PreferRenderMode,
    EGLint * OutRenderMode
    )
{
    *OutRenderMode = PreferRenderMode;
    return EGL_TRUE;
}
#endif

static inline void
_InvalidateBufferCache(
    VEGLSurface Surface
    )
{
    EGLint i;
    VEGLWindowInfo info = Surface->winInfo;

    for (i = 0; i < info->bufferCacheCount; i++)
    {
        if (info->bufferCache[i].handle &&
            info->bufferCache[i].surface)
        {
            /* Unref the surface. */
            gcoSURF_Destroy(info->bufferCache[i].surface);

            if (info->bufferCache[i].tsFd >= 0)
            {
                close(info->bufferCache[i].tsFd);
                info->bufferCache[i].tsFd = -1;
            }
        }
    }

    info->bufferCacheCount = 0;
}

static inline EGLBoolean
_UpdateBufferCache(
    VEGLSurface EglSurface,
    android_native_buffer_t * Buffer,
    gcoSURF * Surface
    )
{
    EGLint i;
    VEGLWindowInfo info = EglSurface->winInfo;
    native_handle_t *handle = (native_handle_t *)Buffer->handle;
    gcoSURF surface;
    int fd;
    int tsFd = -1;
    void * data;
    EGLBoolean invalidate = EGL_FALSE;

    fd = (int)handle->data[0];

#ifdef DRM_GRALLOC
    data = gralloc_vivante_bo_from_handle(Buffer->handle);
#else
    data = (void *)(intptr_t)gc_native_handle_get(Buffer->handle)->surface;
#endif

    for (i = 0; i < info->bufferCacheCount; i++)
    {
        if (info->bufferCache[i].handle == handle)
        {
            if (info->bufferCache[i].fd == fd &&
                info->bufferCache[i].data == data)
            {
                /* The buffer handle is already cached. */
                *Surface = info->bufferCache[i].surface;
                return EGL_TRUE;
            }

            LOGV("%s: buffer out of data: buffer=%p", __func__, handle);
            invalidate = EGL_TRUE;
            break;
        }
    }

    if ((info->bufferCount > 0 && i > info->bufferCount) ||
        info->bufferCacheCount == 32)
    {
        /* Seems buffers are out of date. */
        LOGV("%s: buffers out of data: buffer=%p", __func__, handle);
        invalidate = EGL_TRUE;
    }

    if (invalidate)
    {
        _InvalidateBufferCache(EglSurface);
        i = 0;
    }

    LOGV("%s: win=%p cache handle[%d]=%p", __func__, EglSurface->hwnd, i, handle);
    if (!_GetANativeBufferSurface(Buffer,
                                  EglSurface->renderMode,
                                  &surface))
    {
        return EGL_FALSE;
    }

#ifdef DRM_GRALLOC
    /* Get ready for meta date for framebuffer. */
    if (data && info->hwFramebuffer)
    {
        /* Export ts fd for metadata. */
        gctUINT32 tsNode = surface->tileStatusNode.u.normal.node;
        gcmVERIFY_OK(gcoHAL_ExportVideoMemory(tsNode, O_RDWR, &tsFd));
    }
#endif

    /* Not cached yet, cache it. */
    info->bufferCache[i].handle  = handle;
    info->bufferCache[i].fd      = fd;
    info->bufferCache[i].data    = data;
    info->bufferCache[i].surface = surface;
    info->bufferCache[i].tsFd    = tsFd;
    info->bufferCache[i].age     = 0;

    /* This assignment should be atomic to support concurrent. */
    info->bufferCacheCount = i + 1;

    *Surface = surface;
    return EGL_TRUE;
}

#if gcdENABLE_RENDER_INTO_WINDOW && gcdENABLE_3D && (ANDROID_SDK_VERSION >= 14)
/*
 * For apps in this list, EGL will use indirect rendering,
 * ie, disable no-resolve.
 */
static gcePATCH_ID indirectList[] =
{
    gcvPATCH_LEANBACK,
};

#endif

/*
 * Native format must be compatible with EGLConfig and renderTargetFormat.
 */
static EGLBoolean
_IsFormatCompatible(
    PlatformWindowType Win,
    IN VEGLConfig Config,
    IN gceSURF_FORMAT RenderTargetFormat
    )
{
    int format = 0;
    EGLint redSize, greenSize, blueSize, alphaSize;
    EGLint redSizeRT   = 0;
    EGLint greenSizeRT = 0;
    EGLint blueSizeRT  = 0;
    EGLint alphaSizeRT = 0;

    /* Query native window format. */
    Win->query(Win, NATIVE_WINDOW_FORMAT, &format);

    if (format <= 0 || format >= (int)gcmCOUNTOF(_AndroidPixelFormats))
    {
        return EGL_FALSE;
    }

    redSize   = _AndroidPixelFormats[format].redSize;
    greenSize = _AndroidPixelFormats[format].greenSize;
    blueSize  = _AndroidPixelFormats[format].blueSize;
    alphaSize = _AndroidPixelFormats[format].alphaSize;

    /* Check pixel format compatibility. */
    if ((Config->redSize   == 0 && redSize   != 0) ||
        (Config->redSize   > redSize) ||
        (Config->greenSize == 0 && greenSize != 0) ||
        (Config->greenSize > greenSize) ||
        (Config->blueSize  == 0 && blueSize  != 0) ||
        (Config->blueSize  > blueSize) ||
        (Config->alphaSize == 0 && alphaSize != 0) ||
        (Config->alphaSize > alphaSize))
    {
        /* Native window format is not compatible with EGL config. */
        return EGL_FALSE;
    }

    /* Check renderTarget pixel format compatibility. */
    switch (RenderTargetFormat)
    {
    case gcvSURF_A4R4G4B4:
        alphaSizeRT = 4;
    case gcvSURF_X4R4G4B4:
        redSizeRT = greenSizeRT = blueSizeRT = 4;
        break;
    case gcvSURF_A1R5G5B5:
        alphaSizeRT = 1;
    case gcvSURF_X1R5G5B5:
        redSizeRT = greenSizeRT = blueSizeRT = 5;
        break;
    case gcvSURF_R5G6B5:
        redSizeRT   = 5;
        greenSizeRT = 6;
        blueSizeRT  = 5;
        break;
    case gcvSURF_A8R8G8B8:
        alphaSizeRT = 8;
    case gcvSURF_X8R8G8B8:
        redSizeRT = greenSizeRT = blueSizeRT = 8;
        break;
    case gcvSURF_A8:
        alphaSizeRT = 8;
        break;
    default:
        return EGL_FALSE;
        break;
    }

    if ((redSizeRT   == 0 && redSize   != 0) ||
        (redSizeRT   > redSize) ||
        (greenSizeRT == 0 && greenSize != 0) ||
        (greenSizeRT > greenSize) ||
        (blueSizeRT  == 0 && blueSize  != 0) ||
        (blueSizeRT  > blueSize) ||
        (alphaSizeRT == 0 && alphaSize != 0) ||
        (alphaSizeRT > alphaSize))
    {
        /*
         * Native window format is not compatible with required render
         * target format.
         */
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLint
_QueryRenderMode(
    IN VEGLSurface Surface
    )
{
#if gcdENABLE_RENDER_INTO_WINDOW && gcdENABLE_3D && (ANDROID_SDK_VERSION >= 14)
    /*
     * Determine render into window level.
     */
    EGLint renderMode = 0;
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;
    VEGLConfig config    = &Surface->config;

    do
    {
        gceSTATUS status;

        gceHARDWARE_TYPE currentType;
        EGLBoolean separated2D = EGL_FALSE;
        EGLBoolean fcFill = EGL_FALSE;
        EGLBoolean compressNOAA = EGL_FALSE;
        EGLint mode3D = 0, mode2D = 0;

        gctUINT i;
        EGLBoolean indirect = EGL_FALSE;
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        /* Get patch id. */
        gcoHAL_GetPatchID(gcvNULL, &patchId);

        for (i = 0; i < gcmCOUNTOF(indirectList); i++)
        {
            if (patchId == indirectList[i])
            {
                indirect = EGL_TRUE;
                break;
            }
        }

        if (indirect)
        {
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }

        if (Surface->openVG)
        {
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }

        /*
         * XXX: currently there's no hardware feature check for imx8mscale
         * DEC400 framebuffer compression. Use android property to get this
         * information.
         */
        if (info->hwFramebuffer || patchId == gcvPATCH_ANDROID_COMPOSITOR)
        {
            gctSTRING fbTileSupport = gcvNULL;

            if (!gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "ro.boot.fbTileSupport", &fbTileSupport)) ||
                !fbTileSupport ||
                !gcmIS_SUCCESS(gcoOS_StrCmp(fbTileSupport, "enable")))
            {
                /* Indirect rendering for Framebuffer. */
                renderMode = VEGL_INDIRECT_RENDERING;
                break;
            }
        }

        if ((info->consumerUsage & GRALLOC_USAGE_SW_WRITE_MASK) ||
            (info->consumerUsage & GRALLOC_USAGE_SW_READ_MASK) ||
            (info->consumerUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER))
        {
            /* Disable no-resolve when the consumer requires linear. */
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }

#if ANDROID_SDK_VERSION >= 17
        if (info->consumerUsage & GRALLOC_USAGE_HW_CAMERA_MASK)
        {
            /* Disable no-resolve when the consumer requires linear. */
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }

        if (patchId == gcvPATCH_DEQP)
        {
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }
#   endif

        /* Disable no-resolve if software buffer preserve is disabled. */
        if (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
        {
#if gcdENABLE_BLIT_BUFFER_PRESERVE
            if (config->samples > 1)
            {
                /*
                 * Currently preserve (RS) can not support MSAA buffers.
                 * And, will NOT support that because copy from/to MSAA buffers
                 * may most likely cost more bandwidth than a resolve MSAA to
                 * non-MSAA in indirect rendering mode.
                 */
                renderMode = VEGL_INDIRECT_RENDERING;
                break;
            }
#   else
            /*
             * Direct rendering cannot support 'PRESERVED' swap behavior without
             * Blit-preserve (sw preserve) feature.
             */
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
#   endif
        }

        if (!_IsFormatCompatible(win, config, Surface->renderTargetFormat))
        {
            /* Pixel format not compatible. */
            renderMode = VEGL_INDIRECT_RENDERING;
            break;
        }

        /* Check FC fill feature. */
        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_TILE_FILLER
            );

        if (status == gcvSTATUS_TRUE)
        {
            fcFill = EGL_TRUE;
        }

        /* Query super tiled TX. */
        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_SUPERTILED_TEXTURE
            );

        if (status == gcvSTATUS_TRUE)
        {
            mode3D = VEGL_DIRECT_RENDERING_NOFC;
        }

        /* Query TX with FC. */
        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_TEXTURE_TILE_STATUS_READ
            );

        if (status == gcvSTATUS_TRUE)
        {
            mode3D = VEGL_DIRECT_RENDERING_FC_NOCC;
        }

        /* Query TX with CC. */
        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_TX_DECOMPRESSOR
            );

        if (status == gcvSTATUS_TRUE)
        {
            mode3D = VEGL_DIRECT_RENDERING;
        }

        /* Check 2D core. */
        status = gcoHAL_QuerySeparated2D(gcvNULL);

        if (status == gcvSTATUS_TRUE)
        {
            /* Separated 2D hardware. */
            separated2D = EGL_TRUE;

            gcoHAL_GetHardwareType(gcvNULL, &currentType);

            /* Set to 2D hardware. */
            gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);
        }

        /* Query 2D pipe. */
        status = gcoHAL_IsFeatureAvailable(
            gcvNULL,
            gcvFEATURE_PIPE_2D
            );

        if (status == gcvSTATUS_TRUE)
        {
            /* Query 2D tile input. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_2D_TILING
                );

            if (status == gcvSTATUS_TRUE)
            {
                mode2D = VEGL_DIRECT_RENDERING_NOFC;
            }

            /* Query 2D FC: full tile status support. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_2D_FC_SOURCE
                );

            if (status == gcvSTATUS_TRUE)
            {
                mode2D = VEGL_DIRECT_RENDERING;
            }

            if (mode2D == VEGL_DIRECT_RENDERING)
            {
                EGLBoolean multiSourceBlit = EGL_FALSE;
                EGLBoolean msSourceOffsets = EGL_FALSE;

                /* Query 2D multi-source blit. */
                status = gcoHAL_IsFeatureAvailable(
                    gcvNULL,
                    gcvFEATURE_2D_MULTI_SOURCE_BLT
                    );

                multiSourceBlit = (status == gcvSTATUS_TRUE);

                /* Query 2D multi-source blit v1.5. */
                status = gcoHAL_IsFeatureAvailable(
                    gcvNULL,
                    gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT
                    );

                if (status == gcvSTATUS_TRUE)
                {
                    msSourceOffsets = EGL_TRUE;
                }

                /* Query 2D multi-source blit v2. */
                status = gcoHAL_IsFeatureAvailable(
                    gcvNULL,
                    gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2
                    );

                if (status == gcvSTATUS_TRUE)
                {
                    msSourceOffsets = EGL_TRUE;
                }

                if (multiSourceBlit && !msSourceOffsets)
                {
                    /*
                     * 2D support tile status and multi-source blit.
                     * But multi-source blit does not support source offsets.
                     */
                    mode2D = VEGL_DIRECT_RENDERING_NOFC;
                }
            }

            /* Query 2D FC: Compression-NOAA. */
            status = gcoHAL_IsFeatureAvailable(
                gcvNULL,
                gcvFEATURE_2D_CC_NOAA_SOURCE
                );

            if (status == gcvSTATUS_TRUE)
            {
                compressNOAA = EGL_TRUE;
            }
        }
        else
        {
            /* No 2D core. 2D will not contribute constraits. */
            mode2D = -1;
        }

        if (separated2D)
        {
            /* Restore hardware type. */
            gcoHAL_SetHardwareType(gcvNULL, currentType);
        }

        /* Determine render into window mode. */
        if ((mode3D >= VEGL_DIRECT_RENDERING_NOFC) &&
            ((mode2D >= VEGL_DIRECT_RENDERING_NOFC) || (mode2D == -1)) &&
            fcFill)
        {
            renderMode = VEGL_DIRECT_RENDERING_FCFILL;
        }

#if gcdENABLE_RENDER_INTO_WINDOW_WITH_FC
        if ((mode3D >= VEGL_DIRECT_RENDERING_FC_NOCC) &&
            ((mode2D >= VEGL_DIRECT_RENDERING_FC_NOCC) || (mode2D == -1)))
        {
            /* No resolve with tile status. */
            renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
        }

        /*
         * If has 2D and 2D has color compression support, we can use compressed
         * output. 3D composition should be rare.
         * If there's no 2D, and 3D TX has decompress support, we can also use
         * compressed output.
         */
        if (((mode2D == VEGL_DIRECT_RENDERING) &&
                (compressNOAA || config->samples == 4)) ||
            ((mode2D == -1) && (mode3D == VEGL_DIRECT_RENDERING)))
        {
            /* No resolve with tile status, compressed. */
            renderMode = VEGL_DIRECT_RENDERING;
        }

        if (config->samples == 4)
        {
            renderMode = VEGL_INDIRECT_RENDERING;
        }
#endif

        /* Special render into window mode. */
        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            (mode2D >= VEGL_DIRECT_RENDERING_NOFC))
        {
            if ((patchId == gcvPATCH_NENAMARK2) ||
                (patchId == gcvPATCH_GLBM25))
            {
                /* Use render into window w/o tile status mode. */
                renderMode = VEGL_DIRECT_RENDERING_NOFC;
            }
        }

        if ((renderMode < VEGL_DIRECT_RENDERING) &&
            (config->samples  > 1))
        {
            /* Disable No Resolve for MSAA case. */
            renderMode = VEGL_INDIRECT_RENDERING;
        }

        if (patchId == gcvPATCH_SBROWSER)
        {
            if (Surface->swapBehavior == EGL_BUFFER_PRESERVED)
            {
                renderMode = 0;
            }
        }
    }
    while (0);

    LOGV("%s: renderMode=%d", __func__, renderMode);
    return renderMode;

#else

    /* Pemanently disabled no-resolve. */
    return VEGL_INDIRECT_RENDERING;
#endif
}

#ifdef DRM_GRALLOC
static void
_SetBufferUsage(
    IN VEGLSurface Surface,
    IN EGLint RenderMode
    )
{
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;

    /* Default hw render producer usage. */
    int usage = GRALLOC_USAGE_HW_RENDER
              | GRALLOC_USAGE_HW_TEXTURE
              | GRALLOC_USAGE_SW_READ_NEVER
              | GRALLOC_USAGE_SW_WRITE_NEVER;

    /*
     * Check direct rendering mode.
     * Append GRALLOC_USAGE_RENDER_TARGET(defined in gralloc) bitfield
     * to the new usage.
     * When the bitfield is there, vivante gralloc will allocate specified
     * render target surface.
     */
    switch (RenderMode)
    {
    case VEGL_DIRECT_RENDERING_FC_NOCC:
    case VEGL_DIRECT_RENDERING_FCFILL:
    case VEGL_DIRECT_RENDERING:
        usage |= GRALLOC_USAGE_TS_VIV;
    case VEGL_DIRECT_RENDERING_NOFC:
        /* Allocate render target with tile status but disable compression. */
        usage |= GRALLOC_USAGE_TILED_VIV;
        break;

    default:
        break;
    }

    if (Surface->protectedContent)
    {
        /* Protected content. */
        usage |= GRALLOC_USAGE_PROTECTED;
    }

    if (info->producerUsage != usage)
    {
#ifndef DRM_GRALLOC
        if ((info->producerUsage & usage) == usage)
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
        LOGV("%s: set producerUsage to 0x%08X", __func__, usage);
        native_window_set_usage(win, usage);

        /* Store buffer usage. */
#ifdef DRM_GRALLOC
        info->producerUsage = usage;
#else
        info->producerUsage = (usage & ~GRALLOC_USAGE_DUMMY_VIV);
#endif

        /* Buffer usage changed, invalidate cached buffers. */
        _InvalidateBufferCache(Surface);
    }
}

#else
static void
_SetBufferUsage(
    IN VEGLSurface Surface,
    IN EGLint RenderMode
    )
{
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;

    /* Default hw render producer usage. */
    int usage = GRALLOC_USAGE_HW_RENDER
              | GRALLOC_USAGE_HW_TEXTURE
              | GRALLOC_USAGE_SW_READ_NEVER
              | GRALLOC_USAGE_SW_WRITE_NEVER;

    /*
     * Check direct rendering mode.
     * Append GRALLOC_USAGE_RENDER_TARGET(defined in gralloc) bitfield
     * to the new usage.
     * When the bitfield is there, vivante gralloc will allocate specified
     * render target surface.
     */
    switch (RenderMode)
    {
    case VEGL_DIRECT_RENDERING_FCFILL:
    case VEGL_DIRECT_RENDERING_FC_NOCC:
        /* Allocate render target with tile status but disable compression. */
        usage |= GRALLOC_USAGE_RENDER_TARGET_NO_CC_VIV;
        break;

    case VEGL_DIRECT_RENDERING:
        /* Allocate normal render target. */
        usage |= (Surface->config.samples == 4)
               ? GRALLOC_USAGE_RENDER_TARGET_MSAA_VIV
               : GRALLOC_USAGE_RENDER_TARGET_VIV;
        break;

    case VEGL_DIRECT_RENDERING_NOFC:
        /* Allocate render target without tile status. */
        usage |= GRALLOC_USAGE_RENDER_TARGET_NO_TS_VIV;
        break;

    default:
        if (Surface->openVG)
        {
            /* Hardware openvg render. */
            usage |= GRALLOC_USAGE_HW_VG_RENDER_VIV;
        }
        break;
    }

#if ANDROID_SDK_VERSION >= 14
    if (Surface->protectedContent)
    {
        /* Protected content. */
        usage |= GRALLOC_USAGE_PROTECTED;
    }
#endif

    if (info->producerUsage != usage)
    {
        if ((info->producerUsage & usage) == usage)
        {
            /*
             * Append GRALLOC_USAGE_DUMMY_VIV(defined in gralloc) bitfield
             * to the new usage to indicate buffer usage change.
             * This is because in android framework:
             * if ((oldUsage & newUsage) == newUsage), buffer will not re-allocate.
             */
            usage |= GRALLOC_USAGE_DUMMY_VIV;
        }

        /* Pass usage to android framework. */
        LOGV("%s: set producerUsage to 0x%08X", __func__, usage);
        native_window_set_usage(win, usage);

        /* Store buffer usage. */
        info->producerUsage = (usage & ~GRALLOC_USAGE_DUMMY_VIV);

        /* Buffer usage changed, invalidate cached buffers. */
        _InvalidateBufferCache(Surface);
    }
}
#endif

static EGLBoolean
_ConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN void * Window
    )
{
    gceSTATUS status;
    VEGLWindowInfo info = gcvNULL;
    PlatformWindowType win = (PlatformWindowType) Window;
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != (PlatformWindowType) gcvNULL);
    gcmASSERT(Surface->winInfo == gcvNULL);

    /*
     * Set default usage for native window. This is an egl surface,
     * thus only GPU can render to it.
     * GRALLOC_USAGE_SW_READ_NEVER and
     * GRALLOC_USAGE_SW_WRITE_NEVER are not really needed,
     * but it's better to be explicit.
     */
    int usage = GRALLOC_USAGE_HW_RENDER
              | GRALLOC_USAGE_HW_TEXTURE
              | GRALLOC_USAGE_SW_READ_NEVER
              | GRALLOC_USAGE_SW_WRITE_NEVER;

    LOGV("%s: win=%p", __func__, win);
    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo == gcvNULL);

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglWindowInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglWindowInfo));
    info = pointer;

    /* Initialize basic window info. */
    info->bufferCount  = -1;
    info->initialFrame = EGL_TRUE;

    /* Reference andriod native window. */
    win->common.incRef(&win->common);

    /* Set default usage. */
    native_window_set_usage(win, usage);
    info->producerUsage = usage;

    /* Connect to EGL api. */
    _ApiConnect(win);

    /* Check window surface types. */
    _GetWindowProperties(win, info);

    /* Set buffer count. */
    _SetBufferCount(Display, win, info);

    Surface->winInfo = info;
    return EGL_TRUE;

OnError:
    if (info)
    {
        gcmOS_SAFE_FREE(gcvNULL, info);
        Surface->winInfo = gcvNULL;
    }

    LOGE("%s: failed", __func__);
    return EGL_FALSE;
}

static EGLBoolean
_DisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    LOGV("%s: win=%p", __func__, win);

    /* Clean up local buffer cache. */
    _InvalidateBufferCache(Surface);

    /* Unset the buffer count. */
    _UnsetBufferCount(Display, win, info);

    /* Disconnect EGL api. */
    _ApiDisconnect(win);

    /* Dereference andriod native window. */
    win->common.decRef(&win->common);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Free window info. */
    gcmOS_SAFE_FREE(gcvNULL, info);
    Surface->winInfo = gcvNULL;

    return EGL_TRUE;
}

static EGLBoolean
_BindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    )
{
    int result;
    EGLint renderMode;
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;
    android_native_buffer_t *buffer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    LOGV("%s: win=%p", __func__, win);

    /* Query prefered render mode. */
    renderMode = _QueryRenderMode(Surface);

    /* Set final producer usage. */
    _SetBufferUsage(Surface, renderMode);

    /* Validate if selected mode can be used. */
    if (!_NegotiateRenderMode(win, renderMode, RenderMode))
    {
        /* Error occured. */
        return EGL_FALSE;
    }

    if (*RenderMode != renderMode)
    {
        /* Render mode changed. */
        _SetBufferUsage(Surface, *RenderMode);
    }

    return EGL_TRUE;
}

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    LOGV("%s: win=%p", __func__, Surface->hwnd);
    return EGL_TRUE;
}

static EGLBoolean
_GetWindowSize(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
    gceSTATUS status;
    EGLint width;
    EGLint height;

    /* Get shortcut. */
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != (PlatformWindowType) gcvNULL);
    gcmASSERT(info);

    if (info->bufferWidth && info->bufferHeight)
    {
        width  = info->bufferWidth;
        height = info->bufferHeight;
    }
    else
    {
        int rel;

#if ANDROID_SDK_VERSION >= 14
        /* Simple check from server side. */
        rel = win->query(win, NATIVE_WINDOW_WIDTH, &width);

        if (rel != 0)
        {
            LOGE("%s: window has been abandoned", __func__);
            return EGL_FALSE;
        }

        rel  = win->query(win, NATIVE_WINDOW_DEFAULT_WIDTH,  &width);
        rel |= win->query(win, NATIVE_WINDOW_DEFAULT_HEIGHT, &height);
#else
        rel  = win->query(win, NATIVE_WINDOW_WIDTH,  &width);
        rel |= win->query(win, NATIVE_WINDOW_HEIGHT, &height);
#endif

        if (rel != 0)
        {
            LOGE("%s: window has been abandoned\n", __func__);
            return EGL_FALSE;
        }
    }

    *Width  = width;
    *Height = height;

    LOGV("%s: win=%p size=%dx%d", __func__, win, width, height);
    return EGL_TRUE;
}

static EGLBoolean
_GetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;
    android_native_buffer_t *buffer;
    gcoSURF surface;
    int rel;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Dequeue next buffer. */
    rel = _DequeueBuffer(win, &buffer);

    if (rel < 0)
    {
        BackBuffer->context  = gcvNULL;
        BackBuffer->surface  = gcvNULL;
        LOGE("%s: failed to dequeue buffer", __func__);
        return EGL_FALSE;
    }

    /* Update latest buffer size. */
    info->bufferWidth  = buffer->width;
    info->bufferHeight = buffer->height;

    /* Keep a reference on the buffer. */
    buffer->common.incRef(&buffer->common);

    if (!_UpdateBufferCache(Surface, buffer, &surface))
    {
        BackBuffer->context = gcvNULL;
        BackBuffer->surface = gcvNULL;

        /* Cancel this buffer. */
        _CancelBuffer(win, buffer);

        buffer->common.decRef(&buffer->common);

        LOGE("%s: invalid buffer handle", __func__);
        return EGL_FALSE;
    }

    /* Get back buffer. */
    BackBuffer->surface = surface;
    BackBuffer->context = buffer;
    BackBuffer->flip    = gcvTRUE;

    LOGV("%s: win=%p buffer=%p surface=%p", __func__,
         win, buffer, BackBuffer->surface);
    return EGL_TRUE;
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    VEGLWindowInfo winInfo = Surface->winInfo;
    android_native_buffer_t * buffer;
    EGLint i;

    buffer = (android_native_buffer_t *) BackBuffer->context;

    for (i = 0; i < winInfo->bufferCacheCount; i++)
    {
        if (winInfo->bufferCache[i].age > 0)
        {
            winInfo->bufferCache[i].age += 1;
        }

        if (winInfo->bufferCache[i].handle == buffer->handle)
        {
            winInfo->bufferCache[i].age = 1;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean
_QueryBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    OUT EGLint *BufferAge
    )
{
    VEGLWindowInfo winInfo = Surface->winInfo;
    android_native_buffer_t * buffer;
    EGLint i;

    buffer = (android_native_buffer_t *) BackBuffer->context;

    for (i = 0; i < winInfo->bufferCacheCount; i++)
    {
        if (winInfo->bufferCache[i].handle == buffer->handle)
        {
            *BufferAge = winInfo->bufferCache[i].age;
            return EGL_TRUE;
        }
    }

    return EGL_FALSE;
}

#ifdef DRM_GRALLOC
static void
_PushBufferStatus(
    android_native_buffer_t * Buffer,
    gcoSURF Surface
    )
{
    int err;
    struct gralloc_vivante_bo_t * bo;
    struct drm_vivante_bo_tiling tilingArgs;
    uint64_t timeStamp = 0;
    gceTILING tiling = gcvINVALIDTILED;

    bo = gralloc_vivante_bo_from_handle(Buffer->handle);

    if (bo)
    {
        /* GPU-buffer. */
        _PushVivanteDrmBufferStatus(Surface, bo);
    }
    else
    {
        _PushGenericDrmBufferStatus(Surface, Buffer);
    }
}
#endif

#ifdef DRM_GRALLOC
static EGLBoolean
_UpdateBufferMetadata(
    IN VEGLSurface EglSurface,
    IN gcoSURF Surface
    )
{
    EGLint i;
    VEGLWindowInfo info = EglSurface->winInfo;

    for (i = 0; i < info->bufferCacheCount; i++)
    {
        if (info->bufferCache[i].surface == Surface)
        {
            break;
        }
    }

    if (i < info->bufferCacheCount && info->bufferCache[i].tsFd >= 0)
    {
        gcoSURF_UpdateMetadata(Surface, info->bufferCache[i].tsFd);
    }

    return EGL_TRUE;
}
#endif

static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN struct eglRegion * Region,
    IN struct eglRegion * DamageHint
    )
{
    PlatformWindowType win = Surface->hwnd;
    android_native_buffer_t * buffer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    /* Extract objects. */
    buffer = (android_native_buffer_t *) BackBuffer->context;

    LOGV("%s: win=%p buffer=%p surface=%p", __func__,
         win, buffer, BackBuffer->surface);

#ifdef DRM_GRALLOC
    /* Update buffer ts info and timestamp. */
    _PushBufferStatus(buffer, BackBuffer->surface);

    /* Update surface metadata. */
    _UpdateBufferMetadata(Surface, BackBuffer->surface);
#else
    /* Surface has changed. */
    gcmVERIFY_OK(gcoSURF_UpdateTimeStamp(BackBuffer->surface));

    /* Push surface shared states to shared buffer. */
    gcmVERIFY_OK(gcoSURF_PushSharedInfo(BackBuffer->surface));
#endif

    /* Queue native buffer back into native window. Triggers composition. */
    _QueueBuffer(win, buffer, -1);

    /* Decrease reference of native buffer.*/
    buffer->common.decRef(&buffer->common);

    return EGL_TRUE;
}

#if gcdANDROID_NATIVE_FENCE_SYNC >= 2
static EGLBoolean
_PostWindowBackBufferFence(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN struct eglRegion * DamageHint
    )
{
    PlatformWindowType win = Surface->hwnd;
    android_native_buffer_t * buffer;
    gctSIGNAL signal;
    int fenceFd = -1;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);

    /* Extract objects. */
    buffer = (android_native_buffer_t *) BackBuffer->context;

    LOGV("%s: win=%p buffer=%p surface=%p", __func__,
         win, buffer, BackBuffer->surface);

#ifdef DRM_GRALLOC
    /* Update buffer ts info and timestamp. */
    _PushBufferStatus(buffer, BackBuffer->surface);

    /* Update surface metadata. */
    _UpdateBufferMetadata(Surface, BackBuffer->surface);
#else
    /* Surface has changed. */
    gcmVERIFY_OK(gcoSURF_UpdateTimeStamp(BackBuffer->surface));

    /* Push surface shared states to shared buffer. */
    gcmVERIFY_OK(gcoSURF_PushSharedInfo(BackBuffer->surface));
#endif

    do
    {
        gceSTATUS status;
#if gcdENABLE_VG
        gcsTASK_SIGNAL_PTR task;
#endif
#if gcdENABLE_3D && gcdENABLE_VG
        gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

        gcoHAL_GetHardwareType(gcvNULL, &hwType);

        if (hwType == gcvHARDWARE_VG)
#endif
        {
#if gcdENABLE_VG
            if (gcmIS_ERROR(gcoHAL_ReserveTask(gcvNULL,
#if gcdGC355_PROFILER
                                               gcvNULL,
                                               0,0,0,
#   endif
                                               gcvBLOCK_PIXEL,
                                               1,
                                               gcmSIZEOF(gcsTASK_SIGNAL),
                                               (gctPOINTER *) &task)))
            {
                gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
                break;
            }
#endif
        }

        /* Create sync point. */
        status = gcoOS_CreateSignal(gcvNULL, gcvTRUE, &signal);

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
            break;
        }

        /* Create native fence. */
        status = gcoOS_CreateNativeFence(gcvNULL, signal, &fenceFd);

        if (gcmIS_ERROR(status))
        {
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvTRUE));
            break;
        }

#if gcdENABLE_3D && gcdENABLE_VG
        if (hwType == gcvHARDWARE_VG)
#endif
        {
#if gcdENABLE_VG
            task->id     = gcvTASK_SIGNAL;
            task->signal = signal;
#endif
        }
#if gcdENABLE_3D && gcdENABLE_VG
        else
#endif
        {
#if gcdENABLE_3D
            gcsHAL_INTERFACE iface;

            /* Submit the sync point. */
            iface.command            = gcvHAL_SIGNAL;
            iface.engine             = gcvENGINE_RENDER;
            iface.u.Signal.signal    = gcmPTR_TO_UINT64(signal);
            iface.u.Signal.auxSignal = 0;
            iface.u.Signal.process   = gcmPTR_TO_UINT64(Display->process);
            iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

            /* Send event. */
            gcoHAL_ScheduleEvent(gcvNULL, &iface);
#endif
        }

        /* Commit commands. */
        gcoHAL_Commit(gcvNULL, gcvFALSE);

        /* Now destroy the sync point. */
        gcmVERIFY_OK(gcoOS_DestroySignal(gcvNULL, signal));
    }
    while (gcvFALSE);

    /* Queue native buffer back into native window. Triggers composition. */
    _QueueBuffer(win, buffer, fenceFd);

    /* Decrease reference of native buffer.*/
    buffer->common.decRef(&buffer->common);

    return EGL_TRUE;
}
#endif

static EGLBoolean
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    PlatformWindowType win = Surface->hwnd;
    android_native_buffer_t * buffer;

    /* Extract objects. */
    buffer = (android_native_buffer_t *) BackBuffer->context;

    LOGV("%s: win=%p buffer=%p surface=%p", __func__,
         win, buffer, BackBuffer->surface);

    /* Cancel native buffer. */
    _CancelBuffer(win, buffer);

    /* Decrease reference of native buffer.*/
    buffer->common.decRef(&buffer->common);

    return EGL_TRUE;
}

static EGLBoolean
_SynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
#if ANDROID_SDK_VERSION >= 11
    PlatformWindowType win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;
    EGLBoolean sync = EGL_FALSE;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    do
    {
        gcePATCH_ID patchId = gcvPATCH_INVALID;

        if (info->initialFrame)
        {
            info->initialFrame = EGL_FALSE;
            sync = EGL_TRUE;
            break;
        }

        /* Get patch id. */
        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_ANDROID_CTS_MEDIA)
        {
            sync = EGL_TRUE;
            break;
        }

        else if (patchId == gcvPATCH_ANDROID_CTS_UIRENDERING)
        {
            sync = EGL_TRUE;
            break;
        }

        /*
         * Force synchronous flip for android compositor.
         * Notice that it is only when ANDROID_native_fence_sync disabled.
         */
        else if (patchId == gcvPATCH_ANDROID_COMPOSITOR)
        {
            sync = EGL_TRUE;
            break;
        }

        else if (patchId == gcvPATCH_YOUTUBE_TV)
        {
            sync = EGL_TRUE;
            break;
        }


        /*
         * Found hang on ics and later in query 'QUEUES_TO_WINDOW_COMPOSER',
         * use alternative way to get this variable, which is cached already.
         * For versions before ics, get the variable here.
         */
#if ANDROID_SDK_VERSION < 14
        if (info->queuesToComposer == (EGLBoolean) ~0U)
        {
            int flag = 0;

            /* Query from native window. */
            win->query(win, NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER, &flag);

            info->queuesToComposer = (flag != 0) ? EGL_TRUE : EGL_FALSE;
        }
#   endif

        /* Synchronous for the compositor. */
        if (!info->queuesToComposer)
        {
            sync = EGL_TRUE;
            break;
        }
    }
    while (gcvFALSE);

    LOGV("synchronous post=%d", sync);
    return sync;
#else

    return EGL_FALSE;
#endif
}

static EGLBoolean
_HasWindowBuffer(
    IN VEGLSurface Surface,
    EGLClientBuffer Handle
    )
{
    EGLint i;
    VEGLWindowInfo info = Surface->winInfo;

    for (i = 0; i < info->bufferCacheCount; i++)
    {
        if (info->bufferCache[i].handle == Handle)
        {
            return EGL_TRUE;
        }
    }

    return EGL_FALSE;
}

#if ANDROID_SDK_VERSION < 26

/******************************************************************************/
/* Pixmap. */

struct eglPixmapInfo
{
    /* Native pixmap geometry info in Vivante HAL. */
    gctINT              width;
    gctINT              height;
    gceSURF_FORMAT      format;
    gctINT              stride;

    /* Pixmap wrapper. */
    gcoSURF             wrapper;

    /* Shadow surface, exists when the wrapper is not resovable. */
    gcoSURF             shadow;
};

static void
_DoSyncFromPixmap(
    PlatformPixmapType Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, gcvNULL, gcvNULL, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (stride == Info->stride)
    {
        /* Same stride. */
        gcoOS_MemCopy(memory[0], Pixmap->data, stride * Info->height);
    }
    else
    {
        /* Copy line by line. */
        gctINT y;
        gctUINT8_PTR source = (gctUINT8_PTR) Pixmap->data;
        gctUINT8_PTR dest   = (gctUINT8_PTR) memory[0];
        gctINT shadowStride = stride;

        /* Get min stride. */
        stride = gcmMIN(shadowStride, Info->stride);

        for (y = 0; y < Info->height; y++)
        {
            /* Copy a scanline. */
            gcoOS_MemCopy(dest, source, stride);

            /* Advance to next line. */
            source += Info->stride;
            dest   += shadowStride;
        }
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));

OnError:
    return;
}

static void
_DoSyncToPixmap(
    PlatformPixmapType Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, gcvNULL, gcvNULL, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (stride == Info->stride)
    {
        /* Same stride. */
        gcoOS_MemCopy(Pixmap->data, memory[0], stride * Info->height);
    }
    else
    {
        /* Copy line by line. */
        gctINT y;
        gctUINT8_PTR source = (gctUINT8_PTR) memory[0];
        gctUINT8_PTR dest   = (gctUINT8_PTR) Pixmap->data;
        gctINT shadowStride = stride;

        /* Get min stride. */
        stride = gcmMIN(shadowStride, Info->stride);

        for (y = 0; y < Info->height; y++)
        {
            /* Copy a scanline. */
            gcoOS_MemCopy(dest, source, stride);

            /* Advance to next line. */
            source += shadowStride;
            dest   += Info->stride;
        }
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));

OnError:
    return;
}
#endif

static EGLBoolean
_MatchPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN struct eglConfig * Config
    )
{
#if ANDROID_SDK_VERSION < 26
    gceSTATUS status;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;
    PlatformPixmapType pixmap = (PlatformPixmapType) Pixmap;

    if (pixmap->version != sizeof (egl_native_pixmap_t))
    {
        LOGE("This is not an Android pixmap");
        return EGL_FALSE;
    }

    /* Translate to HAL format. */
    pixmapFormat = _TranslateFormat(pixmap->format);

    if (pixmapFormat == gcvSURF_UNKNOWN)
    {
        /* Unknown format? */
        return EGL_FALSE;
    }

    /* Check if format is matched. */
    switch (pixmapFormat)
    {
    case gcvSURF_R5G6B5:
        if ((Config->redSize   != 5)
        ||  (Config->greenSize != 6)
        ||  (Config->blueSize  != 5))
        {
            match = EGL_FALSE;
        }
        break;

    case gcvSURF_X8R8G8B8:
        if ((Config->redSize   != 8)
        ||  (Config->greenSize != 8)
        ||  (Config->blueSize  != 8)
        ||  (Config->alphaSize != 0))
        {
            match = EGL_FALSE;
        }
        break;

    default:
        break;
    }

    return match;
#else
    return EGL_FALSE;
#endif
}

static EGLBoolean
_ConnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    )
{
#if ANDROID_SDK_VERSION < 26
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL needShadow = gcvFALSE;
    gcoSURF wrapper = gcvNULL;
    gceSURF_FORMAT pixmapFormat;
    gctINT pixmapStride;
    gcoSURF shadow = gcvNULL;
    gctPOINTER pointer;
    VEGLPixmapInfo info = gcvNULL;
    PlatformPixmapType pixmap = (PlatformPixmapType) Pixmap;

    if ((pixmap == (PlatformPixmapType) gcvNULL) ||
        (pixmap->version != sizeof (egl_native_pixmap_t)))
    {
        LOGE("This is not an Android pixmap");
        return EGL_FALSE;
    }

    /* Translate to HAL format. */
    pixmapFormat = _TranslateFormat(pixmap->format);

    /* Get native pixmap stride. */
    switch (pixmapFormat)
    {
    case gcvSURF_R4G4B4A4:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_R5G6B5:
        pixmapStride = pixmap->stride * 2;
        break;
    case gcvSURF_A8B8G8R8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_A8R8G8B8:
        pixmapStride = pixmap->stride * 4;
        break;
    case gcvSURF_A8:
        /* Need shadow. */
        pixmapStride = pixmap->stride;
        break;
        /* Resolve can not support the two format. */
    default:
        /* Unknown format. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    do
    {
        if (((gctUINTPTR_T) pixmap->data) & 0x3F)
        {
            needShadow = gcvTRUE;
            break;
        }

        if (pixmap->stride < 16)
        {
            /* Too small in width. */
            needShadow = gcvTRUE;
            break;
        }


        gctINT vstride;
        vstride = (pixmap->vstride != 0) ? pixmap->vstride : pixmap->height;

         /* Height needs to be 4 aligned or vstride is large enough. */
        if ((pixmap->height & 3) &&
            (vstride < ((pixmap->height + 3) & ~3)))
        {
            /*
             * Not enough memory in height.
             * Resolve may exceeds the buffer and overwrite other memory.
             */
            needShadow = gcvTRUE;
            break;
        }
    }
    while (gcvFALSE);

    do
    {
        /* Construct pixmap wrapper. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmap->width,
                              pixmap->height,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_USER,
                              &wrapper));

        /* Set pixels. */
        status = gcoSURF_SetBuffer(wrapper,
                                   gcvSURF_BITMAP,
                                   pixmapFormat,
                                   pixmapStride,
                                   pixmap->data,
                                   gcvINVALID_ADDRESS);

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Do the wrap. */
        status = gcoSURF_SetWindow(wrapper,
                                   0, 0,
                                   pixmap->width,
                                   pixmap->height);
    }
    while (gcvFALSE);

    if (gcmIS_ERROR(status) && (wrapper != gcvNULL))
    {
        /* Failed to wrap as surface object. */
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
        wrapper = gcvFALSE;

        /* Shadow required and format must be supported. */
        needShadow = gcvTRUE;
    }

    if (needShadow)
    {
        /* Construct the shadow surface. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmap->width,
                              pixmap->height,
                              1,
                              gcvSURF_BITMAP,
                              pixmapFormat,
                              gcvPOOL_DEFAULT,
                              &shadow));
    }

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglPixmapInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglPixmapInfo));
    info = pointer;

    /* Save flags. */
    info->width    = pixmap->width;
    info->height   = pixmap->height;
    info->format   = pixmapFormat;
    info->stride   = pixmapStride;
    info->wrapper  = wrapper;
    info->shadow   = shadow;

    LOGV("%s(%d): display=%p pixmap=%p wrapper=%p shadow=%p",
         __func__, __LINE__, Display, pixmap, wrapper, shadow);

    /* Output. */
    *Info    = info;
    *Surface = (shadow != gcvNULL) ? shadow : wrapper;

    return EGL_TRUE;

OnError:
    if (wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(wrapper));
    }

    if (shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(shadow));
    }

    if (info != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, info);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return EGL_FALSE;
#else
    return EGL_FALSE;
#endif
}

static EGLBoolean
_DisconnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
#if ANDROID_SDK_VERSION < 26
    /* Free pixmap wrapper. */
    LOGV("%s(%d): display=%p pixmap=%p", __func__, __LINE__, Display, Pixmap);

    if (Info->wrapper != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->wrapper));
        Info->wrapper = gcvNULL;
    }

    if (Info->shadow != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(Info->shadow));
        Info->shadow = gcvNULL;
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, Info);
    return EGL_TRUE;
#else
    return EGL_FALSE;
#endif
}

static EGLBoolean
_GetPixmapSize(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info,
    OUT EGLint * Width,
    OUT EGLint * Height
    )
{
#if ANDROID_SDK_VERSION < 26
    PlatformPixmapType pixmap = (PlatformPixmapType) Pixmap;
    if (pixmap->version != sizeof (egl_native_pixmap_t))
    {
        return EGL_FALSE;
    }

    *Width  = Info->width;
    *Height = Info->height;

    return EGL_TRUE;
#else
    return EGL_FALSE;
#endif
}

static EGLBoolean
_SyncFromPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
#if ANDROID_SDK_VERSION < 26
    PlatformPixmapType pixmap = (PlatformPixmapType) Pixmap;
    LOGV("%s(%d): pixmap=%p", __func__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncFromPixmap(pixmap, Info);
    }
    else
    {
        gcmVERIFY_OK(
            gcoSURF_SetWindow(Info->wrapper,
                              0, 0,
                              Info->width,
                              Info->height));
    }

    return EGL_TRUE;
#else
    return EGL_FALSE;
#endif
}

static EGLBoolean
_SyncToPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
#if ANDROID_SDK_VERSION < 26
    PlatformPixmapType pixmap = (PlatformPixmapType) Pixmap;
    LOGV("%s(%d): pixmap=%p", __func__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncToPixmap(pixmap, Info);
    }

    return EGL_TRUE;
#else
    return EGL_FALSE;
#endif
}


static struct eglPlatform androidPlatform =
{
    EGL_PLATFORM_ANDROID_VIV,

    _GetDefaultDisplay,
    _ReleaseDefaultDisplay,
    _IsValidDisplay,
    _InitLocalDisplayInfo,
    _DeinitLocalDisplayInfo,
    _GetNativeVisualId,
    _GetSwapInterval,
    _SetSwapInterval,
    _ConnectWindow,
    _DisconnectWindow,
    _BindWindow,
    _UnbindWindow,
    _GetWindowSize,
    _GetWindowBackBuffer,
    _PostWindowBackBuffer,
#if gcdANDROID_NATIVE_FENCE_SYNC >= 2
    _PostWindowBackBufferFence,
#else
    gcvNULL,
#endif
    _CancelWindowBackBuffer,
    _SynchronousPost,
    _HasWindowBuffer,
    _UpdateBufferAge,
    _QueryBufferAge,
    _MatchPixmap,
    _ConnectPixmap,
    _DisconnectPixmap,
    _GetPixmapSize,
    _SyncFromPixmap,
    _SyncToPixmap,
};

VEGLPlatform
veglGetAndroidPlatform(
    void * NativeDisplay
    )
{
    return &androidPlatform;
}

static gctBOOL
_UpdateANativeBuffer(
    khrEGL_IMAGE * Image
    )
{
#ifdef DRM_GRALLOC
    EGLBoolean success;
    android_native_buffer_t * buffer;
    struct gralloc_vivante_bo_t * bo;
    gcoSURF surface = Image->surface;
    gctUINT64 timeStamp = Image->u.ANativeBuffer.timeStamp;

    buffer = Image->u.ANativeBuffer.nativeBuffer;

    bo = gralloc_vivante_bo_from_handle(buffer->handle);

    if (bo)
    {
        success = _PopVivanteDrmBufferStatus(bo, surface, &timeStamp);
    }
    else
    {
        success = _PopGenericDrmBufferStatus(buffer, surface, &timeStamp);
    }

    if (success)
    {
        if (timeStamp == Image->u.ANativeBuffer.timeStamp)
        {
            /* Same timeStamp, not updated. */
            return gcvFALSE;
        }

        Image->u.ANativeBuffer.timeStamp = timeStamp;
    }

    return gcvTRUE;
#else
    gceSTATUS status;
    gctUINT64 timeStamp;

    gcmASSERT(Image->type == KHR_IMAGE_ANDROID_NATIVE_BUFFER);

    /* Pop shared info. */
    gcmONERROR(gcoSURF_PopSharedInfo(Image->surface));

    /* Query surface time stamp. */
    gcmVERIFY_OK(gcoSURF_QueryTimeStamp(Image->surface, &timeStamp));

    if (timeStamp > Image->u.ANativeBuffer.timeStamp)
    {
        /* Surface has updated. */
        Image->u.ANativeBuffer.timeStamp = timeStamp;
        return gcvTRUE;
    }

    return gcvFALSE;

OnError:
    return gcvFALSE;
#endif
}

EGLenum
_CreateImageFromANativeBuffer(
    VEGLThreadData Thread,
    EGLClientBuffer Buffer,
    VEGLImage Image
    )
{
    android_native_buffer_t * nativeBuffer;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gcoSURF surface;

    /* Create short cut of a native buffer. */
    nativeBuffer = (android_native_buffer_t*)Buffer;

    /* Validate native buffer object. */
    if (nativeBuffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC)
    {
        return EGL_BAD_PARAMETER;
    }

    /* Validate vative buffer version. */
    if (nativeBuffer->common.version != sizeof(android_native_buffer_t))
    {
        return EGL_BAD_PARAMETER;
    }

    /* Cast surface object. */
    if (!_GetANativeBufferSurface(Buffer, -1, &surface))
    {
        return EGL_BAD_PARAMETER;
    }

    /* Query surface format. */
    gcmVERIFY_OK(gcoSURF_GetFormat(surface, gcvNULL, &format));

    Image->image.magic = KHR_EGL_IMAGE_MAGIC_NUM;
    Image->image.type  = KHR_IMAGE_ANDROID_NATIVE_BUFFER;

    /* Stores image buffer */
    Image->image.surface = surface;
    Image->image.update  = _UpdateANativeBuffer;

    /* Store android native buffer. */
    Image->image.u.ANativeBuffer.nativeBuffer = (gctPOINTER) nativeBuffer;
    Image->image.u.ANativeBuffer.format       = format;
    Image->image.u.ANativeBuffer.timeStamp    = 0;

    /* Keep a reference of native buffer. */
    nativeBuffer->common.incRef(&nativeBuffer->common);

    return EGL_SUCCESS;
}

