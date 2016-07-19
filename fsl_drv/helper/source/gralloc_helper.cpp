/*
 *  Copyright (C) 2014 Freescale Semiconductor, Inc.
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

/*
 *	gralloc_helper.c
 *	This helper file implements the required graphic buffer helper function for Android gralloc
 *	History :
 *	Date(y.m.d)        Author            Version        Description
 *	2014-06-23         Li Xianzhong      0.1            Created
 *	2014-09-23         Li Xianzhong      0.2            Refined
*/
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_user.h>
#include <gralloc_priv.h>
#include <gc_gralloc_helper.h>
#include <binder/IPCThreadState.h>

#include "gpuhelper.h"

private_handle_t* graphic_handle_create(int fd, int size, int flags)
{
    return new private_handle_t(dup(fd), size, flags);
}

void graphic_handle_destroy(private_handle_t* hnd)
{
    close(hnd->fd);
    delete hnd;
}

int graphic_handle_validate(buffer_handle_t handle)
{
    return private_handle_t::validate(handle);
}

// graphic buffer alloc/free relevant.
int graphic_buffer_alloc(int width, int height, int format, int usage,
            int stride, size_t size, private_handle_t** handle)
{
    gctUINT32 physAddr = 0;
    gctSIZE_T Bytes = size;
    gctPOINTER bufNode= gcvNULL;
    gctPOINTER virtAddr = gcvNULL;
    private_handle_t* hnd = gcvNULL;
    gctBOOL cacheable = gcvFALSE;
    gceSTATUS status;
    int fd =0;

    if(!handle) return -EINVAL;

    /*
    * reserve stdin/stdout/stderr
    */
    while( (fd = open("/dev/null", O_RDONLY, 0)) < 3 && fd >= 0 )
    {
        close(fd);
    }

    hnd = new private_handle_t(fd, size, 0);

    if (usage & (GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE))
    {
        cacheable = gcvFALSE;
    }
    else if (usage & GRALLOC_USAGE_SW_WRITE_MASK)
    {
        cacheable = gcvTRUE;
    }

    status = gcoOS_AllocateVideoMemory(gcvNULL, gcvTRUE, cacheable, &Bytes, &physAddr, &virtAddr, &bufNode);
    if(gcvSTATUS_OUT_OF_MEMORY == status && gcvTRUE == cacheable)
    {
        cacheable = gcvFALSE;
        gcmONERROR(
            gcoOS_AllocateVideoMemory(gcvNULL, gcvTRUE, cacheable, &Bytes, &physAddr, &virtAddr, &bufNode));
    }
    else if(gcmIS_ERROR(status))
    {
        gcmONERROR(status);
    }

    hnd->width = width;
    hnd->height = height;
    hnd->format = format;

    hnd->stride = stride;
    hnd->usage  = usage;
    hnd->vidmem = gcmPTR2INT(bufNode);

    hnd->phys = physAddr;
    hnd->base = gcmPTR_TO_UINT64(virtAddr);

    memset((void *)hnd->base, 0, Bytes);

    gc_gralloc_wrap(hnd, width, height, format, stride, hnd->phys, (void *)hnd->base);

    /* Naming video memory node. */
    gcmVERIFY_OK(gcoHAL_NameVideoMemory(gcmPTR2INT(bufNode), (gctUINT32*)&hnd->node));

    *handle = hnd;
    return 0;

OnError:
    /* Error roll back. */
    if (hnd != NULL)
    {
        delete hnd;
    }

    /* Close opened fd. */
    if(fd >= 3)
    {
        close(fd);
    }

    ALOGE("%s, Failed to allocate buffer with size %d, status=%d", __FUNCTION__, size, status);

    return -EINVAL;
}

int graphic_buffer_free(private_handle_t* hnd)
{
    gceSTATUS status;

    /* Parameter check. */
    if (private_handle_t::validate(hnd))
    {
        return -EINVAL;
    }

    gc_gralloc_unwrap(hnd);

    status = gcoOS_FreeVideoMemory(gcvNULL, gcmINT2PTR(hnd->vidmem));

    if (gcmIS_SUCCESS(status))
    {
        close(hnd->fd);
        delete hnd;

        return 0;
    }

    ALOGE("%s, Failed to free buffer handle=%p, status = %d", __FUNCTION__, (void*)hnd, status);

    return -EINVAL;
}

int graphic_buffer_register(private_handle_t* hnd)
{
    gctUINT32 node=0;
    gctUINT32 physAddr = 0;
    gctPOINTER virtAddr = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gceHARDWARE_TYPE currentType;

    /* Parameter check. */
    if (private_handle_t::validate(hnd))
    {
        return -EINVAL;
    }

    /* Query current hardware type. */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

    /* Import video memory node. */
    gcmVERIFY_OK(
        gcoHAL_ImportVideoMemory(hnd->node, &node));

    gcmVERIFY_OK(
        gcoHAL_SetHardwareType(gcvNULL, currentType));

    gcmONERROR(
        gcoOS_LockVideoMemory(gcvNULL, gcmINT2PTR(node), gcvTRUE, gcvFALSE, &physAddr, &virtAddr));

    hnd->vidmem = (int)node;
    hnd->phys   = (int)physAddr;
    hnd->base   = gcmPTR_TO_UINT64(virtAddr);

    return gc_gralloc_register_wrap(hnd, (unsigned long)physAddr, (void*)virtAddr);

OnError:

    ALOGE("%s, Failed to register buffer handle=%p, status=%d", __FUNCTION__, (void *) hnd, status);

    return -EINVAL;
}

int graphic_buffer_unregister(private_handle_t* hnd)
{
    gceSTATUS status;

    /* Parameter check. */
    if (private_handle_t::validate(hnd))
    {
        return -EINVAL;
    }

    gc_gralloc_unwrap(hnd);

    status = gcoOS_FreeVideoMemory(gcvNULL, gcmINT2PTR(hnd->vidmem));

    if (gcmIS_SUCCESS(status))
    {
        return 0;
    }

    ALOGE("%s, Failed to unregister buffer handle=%p, status = %d", __FUNCTION__, (void*)hnd, status);

    return -EINVAL;
}

int graphic_buffer_lock(private_handle_t* hnd)
{
    return 0;
}

int graphic_buffer_unlock(private_handle_t* hnd)
{
    /* Parameter check. */
    if (private_handle_t::validate(hnd))
    {
        return -EINVAL;
    }

    if (!(hnd->usage & (GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE)) && (hnd->usage & GRALLOC_USAGE_SW_WRITE_MASK))
    {
        gcmVERIFY_OK(
            gcoOS_CacheFlush(gcvNULL, gcmPTR_TO_UINT64(hnd->node), (gctPOINTER)hnd->base, hnd->size));
    }

    // buffer content is now updated.
    gc_gralloc_notify_change(hnd);

    return 0;
}

int graphic_buffer_wrap(private_handle_t* hnd,
         int width, int height, int format, int stride,
         unsigned long phys, void* vaddr)
{
    return gc_gralloc_wrap(hnd, width, height, format, stride,
                           phys, vaddr);
}

int graphic_buffer_register_wrap(private_handle_t* hnd,
         unsigned long phys, void* vaddr)
{
    return gc_gralloc_register_wrap(hnd, phys, vaddr);
}

int graphic_buffer_unwrap(private_handle_t* hnd)
{
    return gc_gralloc_unwrap(hnd);
}
