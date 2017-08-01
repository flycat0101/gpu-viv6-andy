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


/**
 * \file gc_dfb_pool.c
 */

#include <asm/types.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/gfxcard.h>
#include <core/surface_pool.h>
#include <core/core.h>
#include <fusion/fusion.h>

#include <gfx/convert.h>

#include "gc_dfb.h"
#include "gc_dfb_utils.h"

D_DEBUG_DOMAIN( Gal_Pool, "Gal/Pool", "Surface pool management" );

typedef struct {
    int magic;
} GalPoolData;

typedef struct {
    int magic;
    FusionCall  call;

    CoreDFB *core;
} GalPoolLocalData;
static FusionCallHandlerResult
gal_surface_pool_call_handler( int          caller,
                               int          call_arg,
                               void         *call_ptr,
                               void         *ctx,
                               unsigned int serial,
                               int          *ret_val )
{
     gcmVERIFY_OK(gcoSURF_Destroy( call_ptr ));

     return FCHR_RETURN;
}

static int
galPoolDataSize()
{
    return sizeof( GalPoolData );
}

static int
galPoolLocalDataSize()
{
    return sizeof( GalPoolLocalData );
}

static int
galAllocationDataSize()
{
    return sizeof( GalAllocationData );
}

static DFBResult
galInitPool( CoreDFB                    *core,
             CoreSurfacePool            *pool,
             void                       *pool_data,
             void                       *pool_local,
             void                       *system_data,
             CoreSurfacePoolDescription *ret_desc )
{
    GalPoolData      *data  = pool_data;
    GalPoolLocalData *local = pool_local;

    D_ASSERT( core       != NULL );
    D_ASSERT( pool       != NULL );
    D_ASSERT( pool_data  != NULL );
    D_ASSERT( pool_local != NULL );
    D_ASSERT( ret_desc   != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );

    D_DEBUG_ENTER( Gal_Pool,
                   "core: %p, pool: %p, pool_data: %p, pool_local: %p,system_data: %p, ret_desc: %p\n",
                   core, pool, pool_data, pool_local, system_data, ret_desc );

#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 4)
    ret_desc->caps     = CSPCAPS_NONE;
    ret_desc->types    = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_EXTERNAL | CSTF_PREALLOCATED;
    ret_desc->priority = CSPP_PREFERED;

    ret_desc->access[CSAID_CPU]    = CSAF_READ | CSAF_WRITE;
    ret_desc->access[CSAID_GPU]    = CSAF_READ | CSAF_WRITE;
    ret_desc->access[CSAID_LAYER1] = CSAF_READ | CSAF_WRITE;
#else
    ret_desc->caps     = CSPCAPS_NONE;
    ret_desc->access   = CSAF_CPU_READ | CSAF_CPU_WRITE | CSAF_GPU_READ | CSAF_GPU_WRITE;
    ret_desc->types    = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_EXTERNAL | CSTF_PREALLOCATED;
    ret_desc->priority = CSPP_PREFERED;
#endif

    snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "GAL Surface Pool" );

    local->core = core;

    D_MAGIC_SET( data, GalPoolData );
    D_MAGIC_SET( local, GalPoolLocalData );

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    /* Fusion_call_init, Init the gal_surface_pool_call_handler. */
    fusion_call_init( &local->call, gal_surface_pool_call_handler, local, dfb_core_world(core) );
    return DFB_OK;
}

static DFBResult
galJoinPool( CoreDFB         *core,
             CoreSurfacePool *pool,
             void            *pool_data,
             void            *pool_local,
             void            *system_data )
{
    GalPoolLocalData *local = pool_local;

    D_ASSERT( core       != NULL );
    D_ASSERT( pool       != NULL );
    D_ASSERT( pool_data  != NULL );
    D_ASSERT( pool_local != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );

    D_DEBUG_ENTER( Gal_Pool,
                   "core: %p, pool: %p, pool_data: %p, pool_local: %p,system_data: %p\n",
                   core, pool, pool_data, pool_local, system_data );

    D_MAGIC_SET( local, GalPoolLocalData );

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    /* GalJoinPool return, Init the gal_surface_pool_call_handler. */
    return fusion_call_init( &local->call, gal_surface_pool_call_handler, local, dfb_core_world(core) );
}

static DFBResult
galDestroyPool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
    GalPoolData      *data  = pool_data;
    GalPoolLocalData *local = pool_local;

    D_ASSERT( pool       != NULL );
    D_ASSERT( pool_data  != NULL );
    D_ASSERT( pool_local != NULL );

    D_MAGIC_ASSERT( pool,  CoreSurfacePool );
    D_MAGIC_ASSERT( data,  GalPoolData );
    D_MAGIC_ASSERT( local, GalPoolLocalData );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p\n",
                   pool, pool_data, pool_local );

    /* Destory the gal_surface_pool_call_handler. */
    fusion_call_destroy( &local->call );
    fusion_id( dfb_core_world(NULL) );

    D_MAGIC_CLEAR( data );
    D_MAGIC_CLEAR( local );

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galLeavePool( CoreSurfacePool *pool,
              void            *pool_data,
              void            *pool_local )
{
    GalPoolLocalData *local = pool_local;

    D_ASSERT( pool       != NULL );
    D_ASSERT( pool_data  != NULL );
    D_ASSERT( pool_local != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( local, GalPoolLocalData );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p\n",
                   pool, pool_data, pool_local );

    fusion_call_destroy( &local->call );
    fusion_id( dfb_core_world(NULL) );
    D_MAGIC_CLEAR( local );

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galTestConfig( CoreSurfacePool         *pool,
               void                    *pool_data,
               void                    *pool_local,
               CoreSurfaceBuffer       *buffer,
               const CoreSurfaceConfig *config )
{
    CoreSurface *surface;

    D_ASSERT( pool       != NULL );
    D_ASSERT( pool_data  != NULL );
    D_ASSERT( pool_local != NULL );
    D_ASSERT( buffer     != NULL );
    D_ASSERT( config     != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p, buffer: %p, config: %p\n",
                   pool, pool_data, pool_local, buffer, config );

    surface = buffer->surface;
    D_MAGIC_ASSERT( surface, CoreSurface );

    D_DEBUG_AT( Gal_Pool,
                "surface->type: 0x%08X, surface->resource_id: %lu\n",
                surface->type, surface->resource_id );

    if ((surface->type & CSTF_LAYER) && surface->resource_id == DLID_PRIMARY)
    {
        D_DEBUG_EXIT( Gal_Pool, "Primary layer is not supported by GAL pool.\n" );

        return DFB_UNSUPPORTED;
    }

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galAllocateBuffer( CoreSurfacePool       *pool,
                   void                  *pool_data,
                   void                  *pool_local,
                   CoreSurfaceBuffer     *buffer,
                   CoreSurfaceAllocation *allocation,
                   void                  *alloc_data )
{
    CoreSurface       *surface;
    GalAllocationData *alloc  = alloc_data;
    GalDriverData     *vdrv   = dfb_gfxcard_get_driver_data();
    gceSTATUS          status = gcvSTATUS_OK;
    gcoSURF            surf   = NULL;

    GalPoolLocalData  *local = pool_local;
    gceSURF_FORMAT     format;
    bool               ret;
    unsigned int       aligned_width;
    unsigned int       aligned_height;
    int                aligned_stride;

    int                index = 0;

    D_ASSERT( pool        != NULL );
    D_ASSERT( pool_data   != NULL );
    D_ASSERT( pool_local  != NULL );
    D_ASSERT( allocation  != NULL );
    D_ASSERT( alloc_data  != NULL );
    D_ASSERT( alloc->surf == NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p, buffer: %p, allocation: %p, alloc_data: %p\n",
                   pool, pool_data, pool_local, buffer, allocation, alloc_data );

    do {
        surface = buffer->surface;
        alloc->prealloc_addr = gcvNULL;
        alloc->prealloc_phys = 0;

        D_MAGIC_ASSERT( surface, CoreSurface );

        if ((surface->type & CSTF_LAYER) && surface->resource_id == DLID_PRIMARY ) {
            D_DEBUG_AT( Gal_Pool, "Primary surface is not supported by GAL pool.\n" );

            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        ret = gal_get_native_format( surface->config.format, &format );
        if (!ret) {
            D_DEBUG_AT( Gal_Pool, "Unsupported color format.\n" );

            status = gcvSTATUS_NOT_SUPPORTED;
            break;
        }

        D_DEBUG_AT( Gal_Pool,
                    "surface->config.size.w: %d\n"
                    "surface->config.size.h: %d\n",
                    surface->config.size.w,
                    surface->config.size.h );

         if (surface->config.flags & CSCONF_PREALLOCATED)
         {
            for (index=0; index<MAX_SURFACE_BUFFERS; index++) {
                  if (surface->buffers[index] == buffer)
                       break;
             }

             if (index == MAX_SURFACE_BUFFERS)
             {
                 D_DEBUG_AT(Gal_Pool,
                            "Not found!\n");
             }

             else if (!surface->config.preallocated[index].addr ||
                  surface->config.preallocated[index].pitch < DFB_BYTES_PER_LINE(surface->config.format,
                                                                                 surface->config.size.w))
             {
                 D_DEBUG_AT(Gal_Pool,
                            "Address is invalid!\n");
             }

             else
             {
                 if ((uintptr_t)surface->config.preallocated[index].addr & 63)
                 {
                     D_WARN( "Prealloc address is not aligned!\n" );
                 }
                 else
                 {
                     alloc->prealloc_addr = surface->config.preallocated[index].addr;
                     D_DEBUG_AT(Gal_Pool,
                                "prealloc addr: %p\n", alloc->prealloc_addr );
                 }
             }
         }

        if (vdrv->vdev->hw_yuv420_output && format == gcvSURF_NV16)
        {
            gcmERR_BREAK(gcoSURF_Construct( vdrv->hal,
                                            gcmALIGN(surface->config.size.w, 64),
                                            surface->config.size.h,
                                            1,
                                            gcvSURF_BITMAP,
                                            gcvSURF_NV16,
                                            gcvPOOL_DEFAULT,
                                            &surf ));

            aligned_width = surface->config.size.w;
            aligned_height = surface->config.size.h;
            aligned_stride = surface->config.size.w;

            alloc->size = aligned_height * gcmALIGN(surface->config.size.w, 64);
        }
        else
        {
            /* Create a gcoSURF surface. */
            gcmERR_BREAK(gcoSURF_Construct( vdrv->hal,
                                            surface->config.size.w,
                                            surface->config.size.h,
                                            1,
                                            gcvSURF_BITMAP,
                                            format,
                                            alloc->prealloc_addr!=gcvNULL ? gcvPOOL_USER :  gcvPOOL_DEFAULT,
                                            &surf ));

            gcmERR_BREAK(gcoSURF_GetAlignedSize( surf,
                                                 &aligned_width,
                                                 &aligned_height,
                                                 &aligned_stride ));

            alloc->size = aligned_height * aligned_stride;
        }

        D_DEBUG_AT( Gal_Pool,
                    "aligned_width: %u, aligned_height: %u, aligned_stride: %d\n",
                    aligned_width, aligned_height, aligned_stride );

        alloc->surf   = surf;
        alloc->pitch  = aligned_stride;
        alloc->offset = 0;

#if GAL_SURFACE_COMPRESSED
        if (vdrv->vdev->hw_2d_compression &&
            (format == gcvSURF_A8R8G8B8 ||
            format == gcvSURF_X8R8G8B8))
        {
            alloc->tssize = alloc->size / 64;
            gcmERR_BREAK(gcoHAL_AllocateVideoMemory(
                256,
                gcvSURF_BITMAP,
                0,
                gcvPOOL_DEFAULT,
                &alloc->tssize,
                &alloc->tsnode ));

            gcmERR_BREAK(gcoHAL_LockVideoMemory(
                alloc->tsnode,
                gcvFALSE,
                gcvENGINE_RENDER,
                &alloc->tsphysical,
                &alloc->tslogical));

            if (format == gcvSURF_A8R8G8B8)
            {
                memset(alloc->tslogical, 0x88, alloc->tssize);
            }
            else
            {
                memset(alloc->tslogical, 0x66, alloc->tssize);
            }
        }
        else
        {
            alloc->tsphysical = gcvINVALID_ADDRESS;
            D_WARN( "2D Compression only supports ARGB8 and XRGB8\n" );
        }
#endif

        /* Add alloc->call and alloc->fid for fusion handler. */
        alloc->call = local->call;
        alloc->fid  = fusion_id( dfb_core_world(NULL) );
        D_DEBUG_AT( Gal_Pool,
                    "pitch: %d, size: %d\n",
                    alloc->pitch, alloc->size );

        allocation->size   = alloc->size;
        allocation->offset = alloc->offset;

        D_MAGIC_SET( alloc, GalAllocationData );
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Pool, "Failed to alloc buffer.\n" );

        if (surf) {
            gcoSURF_Destroy( surf );
        }

        return DFB_FAILURE;
    }

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galDeallocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
    int ret=0;
    GalAllocationData *alloc = alloc_data;
#if GAL_SURFACE_COMPRESSED
    GalDriverData     *vdrv   = dfb_gfxcard_get_driver_data();
#endif

    D_MAGIC_ASSERT( pool,  CoreSurfacePool );
    D_MAGIC_ASSERT( alloc, GalAllocationData );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p, buffer: %p, allocation: %p, alloc_data: %p\n",
                   pool, pool_data, pool_local, buffer, allocation, alloc_data );

    /* Destroy the gcoSURF surface. */
    /* Comment gcoSURF_Destroy interface form the DFB master. */
    /*
       gcmVERIFY_OK(gcoSURF_Destroy( alloc->surf ));
    */

    /* fusion_call_execute the DFB slave form the master through fusion_call_execute*/
    if (alloc->surf != NULL) {
        ret = fusion_call_execute( &alloc->call, FCEF_ONEWAY, 0, alloc->surf, NULL );

        if (ret)
            D_WARN( "SurfPool/Gal: Could not call buffer Slave owner to free it there!\n" );
    }

    alloc->surf = NULL;
    alloc->prealloc_addr = gcvNULL;
    alloc->prealloc_phys = 0;

#if GAL_SURFACE_COMPRESSED
    if (vdrv->vdev->hw_2d_compression &&
        alloc->tsphysical != gcvINVALID_ADDRESS)
    {
        gcoHAL_UnlockVideoMemory(alloc->tsnode, gcvSURF_BITMAP, gcvENGINE_RENDER);
        gcoHAL_ReleaseVideoMemory(alloc->tsnode);
        alloc->tsphysical = gcvINVALID_ADDRESS;
    }
#endif

    D_MAGIC_CLEAR( alloc );

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galLock( CoreSurfacePool       *pool,
         void                  *pool_data,
         void                  *pool_local,
         CoreSurfaceAllocation *allocation,
         void                  *alloc_data,
         CoreSurfaceBufferLock *lock )
{
    GalAllocationData *alloc  = alloc_data;
    gceSTATUS          status = gcvSTATUS_OK;

    void              *addr[3];
    unsigned int       phys[3];


    D_ASSERT( pool        != NULL );
    D_ASSERT( pool_data   != NULL );
    D_ASSERT( pool_local  != NULL );
    D_ASSERT( allocation  != NULL );
    D_ASSERT( alloc_data  != NULL );
    D_ASSERT( lock        != NULL );
    D_ASSERT( alloc->surf != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, GalAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p, allocation: %p, alloc_data: %p, lock: %p\n",
                   pool, pool_data, pool_local, allocation, alloc_data, lock );

    do {
        if ( alloc->prealloc_addr == gcvNULL || alloc->prealloc_phys )
        {
            /* Lock the surface. */
            gcmERR_BREAK(gcoSURF_Lock( alloc->surf, phys, addr ));

            lock->addr   = addr[0];
            lock->phys   = phys[0];

            D_DEBUG_AT( Gal_Pool,
                        "lock->offset: %lu, lock->pitch: %d, lock->addr: %p, lock->phys: 0x%08lx\n",
                        lock->offset, lock->pitch, lock->addr, lock->phys );
        }
        else
        {
            lock->addr   = alloc->prealloc_addr;
            D_DEBUG_AT( Gal_Pool, "Prelock addr: %p\n", lock->addr);
        }

        lock->pitch  = alloc->pitch;
        lock->offset = alloc->offset;

    } while (0);

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Pool, "Failed to lock the surface.\n" );
        return DFB_FAILURE;
    }

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

static DFBResult
galUnlock( CoreSurfacePool       *pool,
           void                  *pool_data,
           void                  *pool_local,
           CoreSurfaceAllocation *allocation,
           void                  *alloc_data,
           CoreSurfaceBufferLock *lock )
{
    GalAllocationData *alloc  = alloc_data;
    gceSTATUS          status = gcvSTATUS_OK;

    D_ASSERT( alloc       != NULL );
    D_ASSERT( pool        != NULL );
    D_ASSERT( pool_data   != NULL );
    D_ASSERT( pool_local  != NULL );
    D_ASSERT( allocation  != NULL );
    D_ASSERT( alloc_data  != NULL );
    D_ASSERT( lock        != NULL );
    D_ASSERT( alloc->surf != NULL );

    D_MAGIC_ASSERT( pool, CoreSurfacePool );
    D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
    D_MAGIC_ASSERT( alloc, GalAllocationData );
    D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

    D_DEBUG_ENTER( Gal_Pool,
                   "pool: %p, pool_data: %p, pool_local: %p, allocation: %p, alloc_data: %p, lock: %p\n",
                   pool, pool_data, pool_local, allocation, alloc_data, lock );

    do {
       if ( alloc->prealloc_addr == gcvNULL || alloc->prealloc_phys )
       {
            gcmERR_BREAK(gcoSURF_Unlock( alloc->surf, lock->addr ));
       }
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Pool, "Failed to unlock the surface.\n" );
        return DFB_FAILURE;
    }

    D_DEBUG_EXIT( Gal_Pool, "\n" );

    return DFB_OK;
}

const SurfacePoolFuncs galSurfacePoolFuncs = {
    PoolDataSize:       galPoolDataSize,
    PoolLocalDataSize:  galPoolLocalDataSize,
    AllocationDataSize: galAllocationDataSize,

    InitPool:           galInitPool,
    JoinPool:           galJoinPool,
    DestroyPool:        galDestroyPool,
    LeavePool:          galLeavePool,

    TestConfig:         galTestConfig,
    AllocateBuffer:     galAllocateBuffer,
    DeallocateBuffer:   galDeallocateBuffer,

    Lock:               galLock,
    Unlock:             galUnlock
};
