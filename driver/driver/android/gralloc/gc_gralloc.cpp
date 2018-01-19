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


/* Please do NOT edit this file. */

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <binder/IPCThreadState.h>

#include "gc_gralloc.h"
#include "gc_gralloc_priv.h"
#include "gc_gralloc_format_xlate.h"

#include <gc_hal.h>
#include <gc_hal_base.h>

#include <gc_hal_user.h>

#if ANDROID_SDK_VERSION >= 16
#   undef  LOGV
#   undef  LOGI
#   undef  LOGE
#   define LOGV(...) ALOGV(__VA_ARGS__)
#   define LOGI(...) ALOGI(__VA_ARGS__)
#   define LOGE(...) ALOGE(__VA_ARGS__)
#endif


/******************************************************************************/

static gceHARDWARE_TYPE defaultHwType;

static int has2DCore;
static int has3DCore;
static int hasVGCore;
static int has3D2DCore;

static pthread_once_t onceControl = PTHREAD_ONCE_INIT;

/* Find out online hardwares and default hardware type. */
static void
check_hardware_types(
    void
    )
{
    gcsHAL_INTERFACE iface;
    iface.ignoreTLS = gcvFALSE;
    iface.command = gcvHAL_CHIP_INFO;

    defaultHwType = gcvHARDWARE_INVALID;

    gcmVERIFY_OK(
        gcoOS_DeviceControl(gcvNULL,
                            IOCTL_GCHAL_INTERFACE,
                            &iface, gcmSIZEOF(iface),
                            &iface, gcmSIZEOF(iface)));

    /* Determine hardware features. */
    for (gctINT i = 0; i < iface.u.ChipInfo.count; i++)
    {
        switch (iface.u.ChipInfo.types[i])
        {
        case gcvHARDWARE_3D2D:
            defaultHwType = gcvHARDWARE_3D2D;
            has3D2DCore   = 1;
            break;

        case gcvHARDWARE_3D:
            /* 3D by default when exist. */
            defaultHwType = gcvHARDWARE_3D;
            has3DCore     = 1;
            break;

        case gcvHARDWARE_VG:
            hasVGCore     = 1;
            if (defaultHwType == gcvHARDWARE_INVALID)
            {
                /* VG type if VG only. */
                defaultHwType = gcvHARDWARE_VG;
            }
            break;

        case gcvHARDWARE_2D:
            has2DCore     = 1;
            if ((defaultHwType == gcvHARDWARE_INVALID) ||
                (defaultHwType == gcvHARDWARE_VG))
            {
                /* 2D type if 2D only or 2D,VG. */
                defaultHwType = gcvHARDWARE_2D;
            }
            break;

        default:
            break;
        }
    }

    if (defaultHwType == gcvHARDWARE_INVALID)
    {
        LOGE("Failed to get hardware types");
    }
}

static int
usage_has_type(
    int Usage,
    int Type
    )
{
    return ((Usage & GRALLOC_USAGE_PRIVATE_MASK_VIV) == Type);
}

static void
switch_hardware_type(
    int Usage
    )
{
    pthread_once(&onceControl, check_hardware_types);

    if (usage_has_type(Usage, GRALLOC_USAGE_HW_VG_RENDER_VIV))
    {
        /* Set to vg type if requested. */
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_VG);
    }
    else
    {
        /* Otherwise follow default hw type. */
        gcoHAL_SetHardwareType(gcvNULL, defaultHwType);
    }
}

/******************************************************************************/

static int
gc_gralloc_map(
    gralloc_module_t const * Module,
    buffer_handle_t Handle,
    void ** Vaddr
    )
{
    gceSTATUS status;
    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    gctUINT32 address[3];
    gctPOINTER memory[3];

    /* Retrieve surface. */
    gcoSURF surface = (gcoSURF) handle->surface;

    /* Lock surface for memory. */
    gcmONERROR(gcoSURF_Lock(surface, address, memory));

    hnd->base = uintptr_t(memory[0]);

    LOGV(" > Map buffer=%p: "
         "surface=%p "
         "physical=%p "
         "logical=%p",
         (void *) hnd,
         surface,
         (void *) intptr_t(address[0]), memory[0]);

    /* Obtain virtual address. */
    *Vaddr = (void *) hnd->base;

    return 0;

OnError:
    LOGE("Failed to map hnd=%p, status=%d", (void *) hnd, status);

    return -EINVAL;
}

static int
gc_gralloc_unmap(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    )
{
    gceSTATUS status;
    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    /* Retrieve surface. */
    gcoSURF surface = (gcoSURF) handle->surface;

    /* Unlock surface. */
    gcmONERROR(gcoSURF_Unlock(surface, gcvNULL));

    hnd->base = 0;

    LOGV(" > Unmap buffer=%p", (void *) hnd);

    return 0;

OnError:
    LOGE("Failed to unmap buffer=%p, status=%d", (void *) hnd, status);

    return -EINVAL;
}

/******************************************************************************/


static int
gc_gralloc_alloc_buffer(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride
    )
{
    int err = 0;
    int fd = -1;

    gceSTATUS status = gcvSTATUS_OK;
    gceHARDWARE_TYPE allocHwType = gcvHARDWARE_3D;
    gctUINT stride   = 0;

    gceSURF_FORMAT halFormat = gcvSURF_UNKNOWN;
    gceSURF_TYPE type;
    gcePOOL pool;
    gceORIENTATION orientation;
    gctUINT samples = 0;

    gcoSURF surface = gcvNULL;

    gctSIGNAL signal = gcvNULL;
    gctINT clientPID = 0;
    gctUINT32 node, tsNode;
    gctUINT32 nodes[3];

    gctSHBUF shBuf = gcvNULL;

    android::IPCThreadState* ipc = android::IPCThreadState::self();
    clientPID = ipc->getCallingPid();

    /* Buffer handle. */
    private_handle_t * hnd = NULL;
    gc_native_handle_t * handle;
    void * vaddr;

    /* Get hardware type for buffer allocation. */
    gcoHAL_GetHardwareType(gcvNULL, &allocHwType);

    /* Cast module. */
    gralloc_module_t * module =
        reinterpret_cast<gralloc_module_t *>(Dev->common.module);

    /* Convert to hal pixel format. */
    halFormat = gc_gralloc_translate_format(Format);

    if (halFormat == gcvSURF_UNKNOWN)
    {
        /* Not supported format. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Determine buffer type and orientation. */
    if (Usage & (GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_SW_READ_MASK))
    {
        /* Linear buffer for SW access. */
        if (((Usage & GRALLOC_USAGE_SW_WRITE_OFTEN) == GRALLOC_USAGE_SW_WRITE_OFTEN) ||
            ((Usage & GRALLOC_USAGE_SW_READ_OFTEN) == GRALLOC_USAGE_SW_READ_OFTEN))
        {
            /* Frequently SW access, enable cacheable. */
            type = gcvSURF_CACHEABLE_BITMAP;
        }
        else
        {
            /* Rare SW access, disable cacheable. */
            type = gcvSURF_BITMAP;
        }

        pool = gcvPOOL_DEFAULT;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        LOGV(" > Allocate SW BITMAP (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_RENDER_TARGET_VIV))
    {
        /* 3D No resolve. */
        type = gcvSURF_RENDER_TARGET;
        orientation = gcvORIENTATION_BOTTOM_TOP;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate RT (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_RENDER_TARGET_NO_TS_VIV))
    {
        /* 3D No resolve, w/o tile status. */
        type = (gceSURF_TYPE) (gcvSURF_RENDER_TARGET | gcvSURF_NO_TILE_STATUS);
        orientation = gcvORIENTATION_BOTTOM_TOP;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate RT_NO_TS (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_RENDER_TARGET_NO_CC_VIV))
    {
        /* 3D No resolve, No color compression. */
        type = (gceSURF_TYPE) (gcvSURF_RENDER_TARGET | gcvSURF_NO_COMPRESSION);
        orientation = gcvORIENTATION_BOTTOM_TOP;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate RT_NO_CC (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_RENDER_TARGET_MSAA_VIV))
    {
        /* 3D No resolve, MSAA, tile status, compression. */
        type = gcvSURF_RENDER_TARGET;
        orientation = gcvORIENTATION_BOTTOM_TOP;
        pool = gcvPOOL_DEFAULT;
        samples = 4;
        LOGV(" > Allocate RT MSAA (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_TEXTURE_VIV))
    {
        /* 3D Tiled output. */
        type = gcvSURF_TEXTURE;
        orientation = gcvORIENTATION_BOTTOM_TOP;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate TX (type=%x, halFormat=%d)", type, halFormat);
    }
    else if (usage_has_type(Usage, GRALLOC_USAGE_HW_VG_RENDER_VIV))
    {
        /* VG linear output. */
        type = gcvSURF_BITMAP;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate VG BITMAP (type=%x, halFormat=%d)", type, halFormat);
    }
#if ANDROID_SDK_VERSION >= 14
    else if (Usage & (GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_COMPOSER))
#else
    else if (Usage & GRALLOC_USAGE_HW_RENDER)
#endif
    {
#if gcdGPU_LINEAR_BUFFER_ENABLED
        /* 3D Linear output. */
        type = gcvSURF_BITMAP;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate 3D BITMAP (type=%x, halFormat=%d)", type, halFormat);
#else
        /* 3D tile output. */
        type = gcvSURF_TEXTURE;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate TX (type=%x, halFormat=%d)", type, halFormat);
#endif
    }
    else if (Usage & GRALLOC_USAGE_HW_TEXTURE)
    {
        /* No hw-render texture, use bitmap. */
        type = gcvSURF_BITMAP;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Allocate BITMAP (type=%x, halFormat=%d)", type, halFormat);
    }
    else
    {
        /* Unknown usage. */
        type = gcvSURF_BITMAP;
        orientation = gcvORIENTATION_TOP_BOTTOM;
        pool = gcvPOOL_DEFAULT;
        LOGV(" > Unknown usage: Allocate BITMAP (type=%x, halFormat=%d)", type, halFormat);
    }

#if gcdENABLE_3D
    /* May alter hal format for no-resolve and tiled output. */
    if ((type & 0xFF) == gcvSURF_RENDER_TARGET)
    {
        switch (halFormat)
        {
        case gcvSURF_R4G4B4A4:
            halFormat = gcvSURF_A4R4G4B4;
            break;

        case gcvSURF_R5G5B5A1:
            halFormat = gcvSURF_A1R5G5B5;
            break;

        case gcvSURF_R5G6B5:
            break;

        case gcvSURF_A8B8G8R8:
        case gcvSURF_A8R8G8B8:
            halFormat = gcvSURF_A8R8G8B8;
            break;

        case gcvSURF_X8B8G8R8:
            halFormat = gcvSURF_X8R8G8B8;
            break;

        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:

        case gcvSURF_YV12:
        case gcvSURF_I420:

        case gcvSURF_YUY2:
        case gcvSURF_UYVY:
            halFormat = gcvSURF_YUY2;
            break;

        default:
            halFormat = gcvSURF_A8R8G8B8;
            break;
        }
    }
    else if (type == gcvSURF_TEXTURE)
    {
        /* Tiled output. */
        gcmONERROR(
            gcoTEXTURE_GetClosestFormat(gcvNULL, halFormat, &halFormat));
    }
#endif

#if ANDROID_SDK_VERSION >= 14
    if (Usage & GRALLOC_USAGE_PROTECTED)
    {
        /* Append protected flag. */
        type = (gceSURF_TYPE) (type | gcvSURF_PROTECTED_CONTENT);
    }
#endif

    /* Construct surface. */
    status = gcoSURF_Construct(gcvNULL,
                               Width, Height,
                               1,
                                type,
                                halFormat,
                                pool,
                                &surface);

    if (gcmIS_ERROR(status))
    {
        LOGE("Failed to construct surface");
        goto OnError;
    }

    if (usage_has_type(Usage, GRALLOC_USAGE_HW_VG_RENDER_VIV))
    {
        /*
        * Quick fix for VG:
        * In surfaceflinger side: transfer first lock to 2D/3D lock.
        */
        gcmONERROR(gcoSURF_Unlock(surface, gcvNULL));
        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

        gcoHAL_SetHardwareType(gcvNULL, defaultHwType);

        /* First lock for 2D/3D. */
        gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

        switch_hardware_type(Usage);
    }

#if gcdENABLE_3D || gcdENABLE_VG
    if (samples > 1)
    {
        /* Set samples. */
        gcmONERROR(gcoSURF_SetSamples(surface, 4));
    }
#endif

    /* Set Y inverted content flag. */
    gcmONERROR(
        gcoSURF_SetFlags(surface,
                         gcvSURF_FLAG_CONTENT_YINVERTED,
                         gcvTRUE));

    (void) orientation;

    /* Android expects stride in pixels. */
    gcmONERROR(
        gcoSURF_GetAlignedSize(surface, &stride, gcvNULL, gcvNULL));

#if gcdENABLE_3D
    /* Set tile status disabled by default for compositor. */
    surface->tileStatusDisabled[0] = gcvTRUE;
#endif

#if gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
    if (!(Usage & GRALLOC_USAGE_HW_RENDER))
    {
        /* For CPU apps, we must synchronize lock requests from CPU with the composition.
         * Composition could happen in the following ways. i)2D, ii)3D, iii)CE, iv)copybit 2D.
         * (Note that, we are not considering copybit composition of 3D apps, which also uses
         * linear surfaces. Can be added later if needed.)

         * In all cases, the following mechanism must be used for proper synchronization
         * between CPU and GPU :

         * - App on gralloc::lock
         *      wait_signal(hwDoneSignal);

         * - Compositor on composition
         *      set_unsignalled(hwDoneSignal);
         *      issue composition;
         *      schedule_event(hwDoneSignal, clientPID);

         *  This is a manually reset signal, which is unsignalled by the compositor when
         *  buffer is in use, prohibiting app from obtaining write lock.
         */
        /* Manually reset signal, for CPU/GPU sync. */
        gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &signal));

        /* Initially signalled. */
        gcmONERROR(gcoOS_Signal(gcvNULL, signal, gcvTRUE));

        LOGV(" > Create signal=%p", signal);
    }
#endif

    /* Alloc shared buffer for surface. */
    gcmONERROR(gcoSURF_AllocShBuffer(surface, &shBuf));
    LOGV(" > Create shBuf=%p", shBuf);

    nodes[0] = surface->node.u.normal.node;
#if gcdENABLE_3D
    nodes[1] = surface->tileStatusNode.u.normal.node;
#endif
    nodes[2] = 0;

    gcmONERROR(gcoHAL_GetGraphicBufferFd(nodes, shBuf, signal, &fd));

    /* Allocate buffer handle. */
    hnd = new private_handle_t(fd, stride * Height, 0);
    handle = gc_native_handle_get(hnd);

    handle->width        = (int) Width;
    handle->height       = (int) Height;
    handle->format       = (int) Format;

    /* Save linear surface to buffer handle. */
    handle->surface      = intptr_t(surface);
    handle->halFormat    = (int) halFormat;
    handle->type         = (int) type;
    handle->samples      = (int) samples;
    /* Record surface info. */
    hnd->size            = (int) surface->size;

    /* Naming video memory node. */
    node = surface->node.u.normal.node;
    gcmVERIFY_OK(gcoHAL_NameVideoMemory(node, &node));

    handle->node         = (int) node;
    handle->nodePool     = (int) surface->node.pool;
    handle->nodeSize     = (int) surface->node.size;

#if gcdENABLE_3D
    /* Naming tile status video memory node. */
    tsNode = surface->tileStatusNode.u.normal.node;

    if (tsNode != 0)
    {
        gcmVERIFY_OK(gcoHAL_NameVideoMemory(tsNode, &tsNode));
    }

    handle->tsNode       = (int) tsNode;
    handle->tsNodePool   = (int) surface->tileStatusNode.pool;
    handle->tsNodeSize   = (int) surface->tileStatusNode.size;
#endif

    handle->hwDoneSignal = (int) intptr_t(signal);

    /* Record usage to recall later in hwc. */
    handle->lockUsage    = 0;
    handle->allocUsage   = Usage;
    handle->clientPID    = clientPID;
    handle->shBuf        = (int) intptr_t(shBuf);

    /* Map video memory to userspace. */
    err = gc_gralloc_map(module, hnd, &vaddr);

    LOGV("Allocate buffer=%p  for pid=%d: "
         "size=%dx%d "
         "format=%d "
         "usage=0x%08X "
         "info=%x:%x:%x-%x:%x:%x",
         (void *) hnd,
         handle->clientPID,
         handle->width,
         handle->height,
         handle->format,
         handle->allocUsage,
         handle->node,
         handle->nodePool,
         handle->nodeSize,
         handle->tsNode,
         handle->tsNodePool,
         handle->tsNodeSize);

    if (err == 0)
    {
        *Handle = hnd;

#if ANDROID_SDK_VERSION >= 18
        if (Format == HAL_PIXEL_FORMAT_YCbCr_420_888)
        {
            *Stride = 0;
        }
        else
#endif
        {
            *Stride = stride;
        }

#if gcdENABLE_3D
        if (vaddr && !(Usage & GRALLOC_USAGE_HW_RENDER))
#else
        if (vaddr)
#endif
        {
            gcoOS_ZeroMemory(vaddr, hnd->size);
        }
    }
    else
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /*
     * Lock for possible composition hardware types for performance.
     * Do not need to restore hardware type because the caller will.
     */
    if (has2DCore && allocHwType != gcvHARDWARE_2D)
    {
        /*
         * Lock for 2D core in surfaceflinger side. 2D hwcomposer could be
         * use for composition.
         */
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);
        gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));
    }

    if (has3DCore && allocHwType != gcvHARDWARE_3D)
    {
        /*
         * Lock for 3D core in surfaceflinger side. 3D composition might be
         * be used.
         */
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);
        gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));
    }

    if (has3D2DCore && allocHwType != gcvHARDWARE_3D2D)
    {
        /*
         * Lock for 3D/2D core in surfaceflinger side. Both 3D/2D composition
         * could be used.
         */
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D2D);
        gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));
    }

    return 0;

OnError:
    /* Close opened fd. */
    if (fd > 0)
    {
        close(fd);
    }

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, allocHwType);

    /* Destroy linear surface. */
    if (surface != gcvNULL)
    {
        gcoSURF_Destroy(surface);
    }

    /* Destroy signal. */
    if (signal)
    {
        gcoOS_DestroySignal(gcvNULL, signal);
    }

    /* Error roll back. */
    if (hnd != NULL)
    {
        delete hnd;
    }

    LOGE("Failed to allocate, status=%d", status);

    return -EINVAL;
}

int
gc_gralloc_alloc(
    alloc_device_t * Dev,
    int Width,
    int Height,
    int Format,
    int Usage,
    buffer_handle_t * Handle,
    int * Stride)
{
    int err;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

    if (!Handle || !Stride)
    {
        return -EINVAL;
    }

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(Usage);

    /* Alloc buffer. */
    err = gc_gralloc_alloc_buffer(
        Dev, Width, Height, Format, Usage, Handle, Stride);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return err;
}

int
gc_gralloc_free(
    alloc_device_t * Dev,
    buffer_handle_t Handle
    )
{
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;
    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    /* Cast private buffer handle. */
    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(handle->allocUsage);

    /* Free buffer. */
    gralloc_module_t * module =
        reinterpret_cast<gralloc_module_t *>(Dev->common.module);

    close(hnd->fd);

    if (hnd->base)
    {
        /* this buffer was mapped, unmap it now. */
        gc_gralloc_unmap(module, hnd);
    }

    /*
    * Memory is merely scheduled to be freed.
    * It will be freed after a h/w event.
    */
    if (handle->surface != 0)
    {
        gceHARDWARE_TYPE allocHwType = gcvHARDWARE_3D;
        gcoSURF surface = (gcoSURF) intptr_t(handle->surface);

        /* Get hardware type for buffer allocation. */
        gcoHAL_GetHardwareType(gcvNULL, &allocHwType);

        /*
         * Unlock for possible composition hardware types.
         */
        if (has2DCore && allocHwType != gcvHARDWARE_2D)
        {
            /* Unlock for 2D core in surfaceflinger side. */
            gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);
            gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        if (has3DCore && allocHwType != gcvHARDWARE_3D)
        {
            /* Unlock for 3D core in surfaceflinger side. */
            gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D);
            gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        if (has3D2DCore && allocHwType != gcvHARDWARE_3D2D)
        {
            /* Unlock for 3D/2D core in surfaceflinger side. */
            gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D2D);
            gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        if (usage_has_type(handle->allocUsage, GRALLOC_USAGE_HW_VG_RENDER_VIV))
        {
            /*
            * Quick fix for VG:
            * In surfaceflinger side: transfer last lock to 2D/3D.
            */
            gcoHAL_SetHardwareType(gcvNULL, defaultHwType);

            /* Last lock for 2D/3D. */
            gcmVERIFY_OK(gcoSURF_Unlock(surface, gcvNULL));
            gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, allocHwType);

        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    /* Destroy signal if exists. */
    if (handle->hwDoneSignal != 0)
    {
        gcmVERIFY_OK(
            gcoOS_DestroySignal(gcvNULL, (gctSIGNAL) intptr_t(handle->hwDoneSignal)));

        handle->hwDoneSignal = 0;
    }

    handle->clientPID = 0;

    LOGV("Free buffer=%p", hnd);

    /* Commit command buffer. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    delete hnd;

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return 0;
}

/******************************************************************************/

int
gc_gralloc_lock(
    gralloc_module_t const* Module,
    buffer_handle_t Handle,
    int Usage,
    int Left,
    int Top,
    int Width,
    int Height,
    void ** Vaddr
    )
{
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    if (handle->surface == 0)
    {
        /* Invalid handle. */
        *Vaddr = NULL;
        return -EINVAL;
    }

    if ((Usage & handle->allocUsage) != Usage)
    {
        LOGE("lock: Invalid access to buffer=%p: "
             "lockUsage=0x%08x, allocUsage=0x%08x",
             hnd, Usage, handle->allocUsage);
    }

#if ANDROID_SDK_VERSION >= 18
    if (handle->format == HAL_PIXEL_FORMAT_YCbCr_420_888)
    {
        /* Not for HAL_PIXEL_FORMAT_YCbCr_420_888 */
        return -EINVAL;
    }
#endif

    *Vaddr = (void *) hnd->base;
    handle->lockUsage = (int) Usage;

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(handle->allocUsage);

    if (handle->hwDoneSignal != 0)
    {
        /* Wait for compositor (2D, 3D, CE) to source from the CPU buffer. */
        gcmVERIFY_OK(
            gcoOS_WaitSignal(gcvNULL,
                             (gctSIGNAL) intptr_t(handle->hwDoneSignal),
                             gcvINFINITE));
    }

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return 0;
}

#if ANDROID_SDK_VERSION >= 18
int
gc_gralloc_lock_ycbcr(
    struct gralloc_module_t const* Module,
    buffer_handle_t Handle,
    int Usage,
    int Left,
    int Top,
    int Width,
    int Height,
    struct android_ycbcr *YCbCr
    )
{
    void * vaddr[3];
    gctINT stride;
    gcoSURF surface;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    if (handle->surface == 0)
    {
        /* Invalid handle. */
        YCbCr->y = YCbCr->cb = YCbCr->cr = NULL;
        return -EINVAL;
    }

    if ((Usage & handle->allocUsage) != Usage)
    {
        LOGE("lock_ycbcr: Invalid access to buffer=%p: "
             "lockUsage=0x%08x, allocUsage=0x%08x",
             hnd, Usage, handle->allocUsage);
    }

    handle->lockUsage = (int) Usage;

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(handle->allocUsage);

    /* Obtain HAL surface. */
    surface = (gcoSURF) handle->surface;

    /* Lock to get plane memory. */
    gcmVERIFY_OK(
        gcoSURF_Lock(surface, gcvNULL, vaddr));

    /* Should be already locked, so the unlock is safe. */
    gcmVERIFY_OK(
        gcoSURF_Unlock(surface, gcvNULL));

    gcmVERIFY_OK(
        gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, &stride));

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    /* Fill in addresses. */
    switch (handle->halFormat)
    {
    case gcvSURF_YUY2:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cb = (void *)((uintptr_t) vaddr[0] + 1);
        YCbCr->cr = (void *)((uintptr_t) vaddr[0] + 3);
        break;
    case gcvSURF_UYVY:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 1);
        YCbCr->cb = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cr = (void *)((uintptr_t) vaddr[0] + 2);
        break;
    case gcvSURF_YVYU:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cb = (void *)((uintptr_t) vaddr[0] + 3);
        YCbCr->cr = (void *)((uintptr_t) vaddr[0] + 1);
        break;
    case gcvSURF_VYUY:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 2);
        YCbCr->cb = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cr = (void *)((uintptr_t) vaddr[0] + 1);
        break;
    case gcvSURF_NV16:
    case gcvSURF_NV12:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cb = (void *)((uintptr_t) vaddr[1] + 0);
        YCbCr->cr = (void *)((uintptr_t) vaddr[1] + 1);
        break;
    case gcvSURF_NV61:
    case gcvSURF_NV21:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cb = (void *)((uintptr_t) vaddr[1] + 1);
        YCbCr->cr = (void *)((uintptr_t) vaddr[1] + 0);
        break;
    case gcvSURF_YV12:
    case gcvSURF_I420:
        YCbCr->y  = (void *)((uintptr_t) vaddr[0] + 0);
        YCbCr->cb = (void *)((uintptr_t) vaddr[1] + 0);
        YCbCr->cr = (void *)((uintptr_t) vaddr[2] + 0);
        break;
    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_R5G6B5:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_R4G4B4A4:
        YCbCr->y  = vaddr[0];
        YCbCr->cb = NULL;
        YCbCr->cr = NULL;
        break;
    default:
        /* Not support for other formats currently. */
        return -EINVAL;
    }

    /* Fill in strides. */
    switch (handle->halFormat)
    {
    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        /* Interleaved 422 formats. */
        YCbCr->ystride = stride;
        YCbCr->cstride = stride;
        YCbCr->chroma_step = 4;
        break;
    case gcvSURF_NV16:
    case gcvSURF_NV61:
    case gcvSURF_NV12:
    case gcvSURF_NV21:
        /* Semi-planar 422/420 formats. */
        YCbCr->ystride = stride;
        YCbCr->cstride = stride;
        YCbCr->chroma_step = 2;
        break;
    case gcvSURF_YV12:
    case gcvSURF_I420:
        /* Planar 420 formats. */
        YCbCr->ystride = stride;
        YCbCr->cstride = ((stride / 2) + 0xf) & ~0xf;
        YCbCr->chroma_step = 1;
        break;
    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_R5G6B5:
    case gcvSURF_R5G5B5A1:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_R4G4B4A4:
        YCbCr->ystride = stride;
        YCbCr->cstride = 0;
        YCbCr->chroma_step = 0;
        break;
    default:
        /* Not support for other formats currently. */
        return -EINVAL;
    }
    return 0;
}
#endif

int
gc_gralloc_unlock(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    )
{
    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    if (handle->surface == 0)
    {
        /* Invalid handle. */
        return -EINVAL;
    }

    /* Update time stamp. */
    gcmVERIFY_OK(gcoSURF_UpdateTimeStamp((gcoSURF) handle->surface));

    /* Push surface shared states. */
    gcmVERIFY_OK(gcoSURF_PushSharedInfo((gcoSURF) handle->surface));

    if (handle->lockUsage & GRALLOC_USAGE_SW_WRITE_MASK)
    {
        gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

        gcoHAL_GetHardwareType(gcvNULL, &hwType);

        /* Switch to proper hardware type. */
        switch_hardware_type(handle->allocUsage);

        /* Ignoring the error. */
        gcmVERIFY_OK(
            gcoSURF_CPUCacheOperation((gcoSURF) handle->surface,
                                      gcvCACHE_CLEAN));

        /* Restore hardware type. */
        gcoHAL_SetHardwareType(gcvNULL, hwType);
    }

    handle->lockUsage = 0;

    return 0;
}

/******************************************************************************/

int
gc_gralloc_register_buffer(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    )
{
    int err = 0;
    void * vaddr = NULL;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    /*
    if (hnd->pid == getpid())
    {
        return 0;
    }
    */

    gcoSURF surface        = gcvNULL;
    gctSIGNAL signal       = gcvNULL;
    gctUINT32 node;
#if gcdENABLE_3D
    gctUINT32 tsNode = 0;
#endif
    gceSTATUS status;

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(handle->allocUsage);

    /* Register linear surface if exists. */
    if (handle->surface != 0)
    {
        gceSURF_TYPE type = (gceSURF_TYPE) handle->type;

        /*
         * This video node was created in another process.
         * Import it into this process..
         * gcvSURF_NO_VIDMEM is used to signify that no video memory node
         * should be created for this surface, as it has alreaady been
         * created in the server.
         */
        type = (gceSURF_TYPE) (type | gcvSURF_NO_VIDMEM);

        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              (gctUINT) handle->width,
                              (gctUINT) handle->height,
                              1,
                              type,
                              (gceSURF_FORMAT) handle->halFormat,
                              (gcePOOL) handle->nodePool,
                              &surface));

#if gcdENABLE_3D || gcdEANBLE_VG
        if (handle->samples > 0)
        {
            /* Set multi-samples. */
            gcmONERROR(gcoSURF_SetSamples(surface, handle->samples));
        }
#endif

        /* Restore surface info. */
        surface->size               = (gctSIZE_T) hnd->size;

        /* Import video memory node. */
        gcmVERIFY_OK(gcoHAL_ImportVideoMemory(handle->node, &node));

        surface->node.u.normal.node = node;
        surface->node.pool          = (gcePOOL)   handle->nodePool;
        surface->node.size          = (gctSIZE_T) handle->nodeSize;

        gcmONERROR(gcoOS_CreateMutex(gcvNULL, &surface->node.sharedMutex));

#if gcdENABLE_3D
        /* Import tile status video memory node. */
        if (handle->tsNode != 0)
        {
            gcmVERIFY_OK(gcoHAL_ImportVideoMemory(handle->tsNode, &tsNode));
        }

        surface->tileStatusNode.u.normal.node = tsNode;
        surface->tileStatusNode.pool          = (gcePOOL  ) handle->tsNodePool;
        surface->tileStatusNode.size          = (gctSIZE_T) handle->tsNodeSize;

        gcmONERROR(gcoOS_CreateMutex(gcvNULL, &surface->tileStatusNode.sharedMutex));
#endif

        /* Lock once as it's done in gcoSURF_Construct with vidmem. */
        gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

        /* Set Y inverted content flag. */
        gcmONERROR(gcoSURF_SetFlags(surface, gcvSURF_FLAG_CONTENT_YINVERTED, gcvTRUE));

        /* Restore surface. */
        handle->surface = intptr_t(surface);

        /* Map handle. */
        err = gc_gralloc_map(Module, hnd, &vaddr);

        if (err)
        {
            goto OnError;
        }
    }

    /* Map signal if exists. */
    if (handle->hwDoneSignal != 0)
    {
        /* Map signal to be used in CPU/GPU synchronization. */
        gcmONERROR(gcoOS_MapSignal((gctSIGNAL) intptr_t(handle->hwDoneSignal), &signal));

        handle->hwDoneSignal = (int) intptr_t(signal);

        LOGV(" > Map signal=%x for buffer=%p", handle->hwDoneSignal, hnd);
    }

    if (handle->shBuf != 0)
    {
        /* Bind shared buffer to surface. */
        gcmVERIFY_OK(gcoSURF_BindShBuffer(surface, (gctSHBUF) intptr_t(handle->shBuf)));
    }

    LOGV("Register buffer=%p from pid=%d: "
         "size=%dx%d format=%d usage=0x%08X "
         "info=%x:%x:%x-%x:%x:%x",
         hnd,
         handle->clientPID,
         handle->width,
         handle->height,
         handle->format,
         handle->allocUsage,
         handle->node,
         handle->nodePool,
         handle->nodeSize,
         handle->tsNode,
         handle->tsNodePool,
         handle->tsNodeSize);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return err;

OnError:
    /* Error roll back. */
    if (vaddr != NULL)
    {
        gc_gralloc_unmap(Module, hnd);
    }

    if (surface != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Destroy(surface));
    }

    if (signal)
    {
        gcmVERIFY_OK(
            gcoOS_DestroySignal(gcvNULL, (gctSIGNAL) intptr_t(handle->hwDoneSignal)));
    }

    /* Reset values. */
    hnd->base = 0;
    handle->surface = 0;
    handle->hwDoneSignal = 0;

    LOGE("Failed to Register buffer=%p: "
         "pid=%d size=%dx%d format=%d usage=0x%08X "
         "info=%x:%x:%x-%x:%x:%x",
         hnd,
         handle->clientPID,
         handle->width,
         handle->height,
         handle->format,
         handle->allocUsage,
         handle->node,
         handle->nodePool,
         handle->nodeSize,
         handle->tsNode,
         handle->tsNodePool,
         handle->tsNodeSize);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return -EINVAL;
}

int
gc_gralloc_unregister_buffer(
    gralloc_module_t const * Module,
    buffer_handle_t Handle
    )
{
    gceSTATUS status;
    gceHARDWARE_TYPE hwType = gcvHARDWARE_3D;

    if (private_handle_t::validate(Handle) < 0)
        return -EINVAL;

    private_handle_t * hnd = (private_handle_t *) Handle;
    gc_native_handle_t * handle = gc_native_handle_get(hnd);

    /*
    if (hnd->pid == getpid())
    {
        return 0;
    }
    */

    /* Get hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &hwType);

    /* Switch to proper hardware type. */
    switch_hardware_type(handle->allocUsage);

    if (hnd->base != 0)
    {
        gc_gralloc_unmap(Module, hnd);
    }

    /* Destroy signal if exist. */
    if (handle->hwDoneSignal)
    {
        LOGV(" > Unmap signal=%x for buffer=%p", handle->hwDoneSignal, hnd);

        gcmONERROR(gcoOS_UnmapSignal((gctSIGNAL) intptr_t(handle->hwDoneSignal)));
        handle->hwDoneSignal = 0;
    }

    /* Unregister linear surface if exists. */
    if (handle->surface != 0)
    {
        /* Non-3D rendering...
         * Append the gcvSURF_NO_VIDMEM flag to indicate we are only destroying
         * the wrapper here, actual vid mem node is destroyed in the process
         * that created it. */
        gcoSURF surface = (gcoSURF) handle->surface;

        gcmONERROR(gcoSURF_Destroy(surface));

        handle->surface = 0;

        LOGV("Unregister buffe=%p", hnd);
    }

    handle->clientPID = 0;

    /* Commit the command buffer. */
    gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return 0;

OnError:
    LOGE("Failed to unregister buffer=%p, status=%d", (void *) hnd, status);

    /* Restore hardware type. */
    gcoHAL_SetHardwareType(gcvNULL, hwType);

    return -EINVAL;
}

