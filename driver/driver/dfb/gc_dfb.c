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
 * \file gc_dfb.c
 */

#include <directfb.h>

#include <dfb_types.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/screens.h>
#include <core/state.h>
#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/graphics_driver.h>

#include <gfx/convert.h>
#include <gfx/util.h>

#include <misc/conf.h>
#include <misc/util.h>

#if GAL_SURFACE_POOL
#include <core/surface_pool.h>
#endif /* GAL_SURFACE_POOL */

#include <direct/hash.h>

#include "gc_dfb.h"
#include "gc_dfb_state.h"
#include "gc_dfb_raster.h"
#include "gc_dfb_conf.h"

DFB_GRAPHICS_DRIVER( gal )

D_DEBUG_DOMAIN( Gal_Driver, "Gal/Driver", "Driver loading and registration" );

/*******************************************************************************
***** Version Signature *******************************************************/

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _DFB_VERSION = "\n\0$VERSION$"
                            gcmTXT2STR(gcvVERSION_MAJOR) "."
                            gcmTXT2STR(gcvVERSION_MINOR) "."
                            gcmTXT2STR(gcvVERSION_PATCH) ":"
                            gcmTXT2STR(gcvVERSION_BUILD) "$\n";

#define CONF_DEBUG              0

#define INDEX_COLOR_TABLE_LEN   256

#define POOL_SRC_WIDTH          256
#define POOL_SRC_HEIGHT         256

#define THRESHOLD_VALUE         1600

#define MAX_MEMORY_MAPPING_CACHE_SIZE        40
#define MAX_MEMORY_MAPPING_TOTAL_MEMORY_SIZE (2 * 1024 * 1024 * MAX_MEMORY_MAPPING_CACHE_SIZE)

static
GalBlendFunc gal_source_blend_funcs[] = {
    /* DSBF_ZERO -- (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ZERO,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_ONE -- (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCCOLOR -- (Rs, Gs, Bs, As) */
    {   0, 0, 0, 0  },
    /* DSBF_INVSRCCOLOR -- (1, 1, 1, 1) - (Rs, Gs, Bs, As) */
    {   0, 0, 0, 0  },
    /* DSBF_SRCALPHA -- (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_MULTIPLY
    },
    /* DSBF_INVSRCALPHA -- (1, 1, 1, 1) - (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED,
        gcvSURF_COLOR_MULTIPLY
    },
    /* DSBF_DESTALPHA -- (Ad, Ad, Ad, Ad) or (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVDESTALPHA (1, 1, 1, 1) - (Ad, Ad, Ad, Ad) or (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTCOLOR -- (Rd, Gd, Bd, Ad) */
    {   0, 0, 0, 0  },
    /* DSBF_INVDESTCOLOR -- (1, 1, 1, 1) - (Rd, Gd, Bd, Ad) */
    {   0, 0, 0, 0  },
    /* DSBF_SRCALPHASAT -- (f, f, f, 1), f = min(As, 1 - Ad) or (0, 0, 0, 1) */
    {   0, 0, 0, 0  }
};

static
GalBlendFunc gal_dest_blend_funcs[] = {
    /* DSBF_ZERO -- (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ZERO,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_ONE -- (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCCOLOR -- (Rs, Gs, Bs, As) */
    {   0, 0, 0, 0  },
    /* DSBF_INVSRCCOLOR -- (1, 1, 1, 1) - (Rs, Gs, Bs, As) */
    {   0, 0, 0, 0  },
    /* DSBF_SRCALPHA -- (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVSRCALPHA -- (1, 1, 1, 1) - (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTALPHA -- (Ad, Ad, Ad, Ad) or (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_MULTIPLY
    },
    /* DSBF_INVDESTALPHA -- (1, 1, 1, 1) - (Ad, Ad, Ad, Ad) or (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_INVERSED,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_MULTIPLY
    },
    /* DSBF_DESTCOLOR -- (Rd, Gd, Bd, Ad) */
    {   0, 0, 0, 0  },
    /* DSBF_INVDESTCOLOR -- (1, 1, 1, 1) - (Rd, Gd, Bd, Ad) */
    {   0, 0, 0, 0  },
};

static
GalBlendFunc gal_source_blend_funcs_full_dfb[] = {
    /* DSBF_ZERO -- (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ZERO,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_ONE -- (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCCOLOR -- (Rs, Gs, Bs, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVSRCCOLOR -- (1, 1, 1, 1) - (Rs, Gs, Bs, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_INVERSED_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCALPHA -- (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVSRCALPHA -- (1, 1, 1, 1) - (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTALPHA -- (Ad, Ad, Ad, Ad) or (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVDESTALPHA (1, 1, 1, 1) - (Ad, Ad, Ad, Ad) or (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTCOLOR -- (Rd, Gd, Bd, Ad) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVDESTCOLOR -- (1, 1, 1, 1) - (Rd, Gd, Bd, Ad) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCALPHASAT -- (f, f, f, 1), f = min(As, 1 - Ad) or (0, 0, 0, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_SRC_ALPHA_SATURATED,
        gcvSURF_COLOR_STRAIGHT
    }
};

static
GalBlendFunc gal_dest_blend_funcs_full_dfb[] = {
    /* DSBF_ZERO -- (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ZERO,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_ONE -- (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCCOLOR -- (Rs, Gs, Bs, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVSRCCOLOR -- (1, 1, 1, 1) - (Rs, Gs, Bs, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCALPHA -- (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVSRCALPHA -- (1, 1, 1, 1) - (As, As, As, As) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTALPHA -- (Ad, Ad, Ad, Ad) or (1, 1, 1, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_STRAIGHT_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVDESTALPHA -- (1, 1, 1, 1) - (Ad, Ad, Ad, Ad) or (0, 0, 0, 0) */
    {
        gcvSURF_PIXEL_ALPHA_INVERSED,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_INVERSED_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_DESTCOLOR -- (Rd, Gd, Bd, Ad) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_INVDESTCOLOR -- (1, 1, 1, 1) - (Rd, Gd, Bd, Ad) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_COLOR_INVERSED_NO_CROSS,
        gcvSURF_COLOR_STRAIGHT
    },
    /* DSBF_SRCALPHASAT -- (f, f, f, 1), f = min(As, 1 - Ad) or (0, 0, 0, 1) */
    {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_SRC_ALPHA_SATURATED_CROSS,
        gcvSURF_COLOR_STRAIGHT
    }
};

static void
gal_set_default_state( GalDriverData *vdrv );

static DFBResult
gal_setup_driver( GalDriverData *drv,
                  GalDeviceData *dev );

static void
gal_cleanup_driver( GalDriverData *drv );

static DFBResult
gal_setup_device( CoreGraphicsDevice *device,
                  GalDriverData      *drv,
                  GalDeviceData      *dev );

static void
gal_cleanup_device( GalDeviceData *dev );

/**
 * Make sure that the hardware has finished
 * all operations.
 *
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return  DFB_OK: succeed
 * \return  DFB_FAILURE: failed
 */
static DFBResult
galEngineSync( void *drv, void *dev )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );

    /* Return if no HW acceleration is called yet. */
    if (!vdrv->dirty) {
        return DFB_OK;
    }

    D_DEBUG_ENTER( Gal_Driver, "drv: %p, dev: %p.\n", drv, dev );

    do {
        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush pending primitives if any. */
        gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

        /* Flush the 2D pipeline. */
        gcmERR_BREAK(gco2D_Flush( vdrv->engine_2d ));

        /* Commit and wait for hardware idle. */
        gcmERR_BREAK(gco2D_Commit( vdrv->engine_2d, true ));
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Driver, "Failed to sync the engine.\n" );

        return DFB_FAILURE;
    }

    /* Clear the dirty flag. */
    vdrv->dirty = false;

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return DFB_OK;
}


/**
 * Emit any buffered commands.
 *
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return  none
 */
static void
galEmitCommands(void *drv, void *dev)
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;

    gceSTATUS      status = gcvSTATUS_OK;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );

    D_DEBUG_ENTER( Gal_Driver, "drv: %p, dev: %p.\n", drv, dev );

    /* Return if no HW acceleration is called yet. */
    if (!vdrv->dirty) {
        return;
    }

    do {
        /* Save the current hardware type */
        gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));

        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

        /* Flush pending primitives if any. */
        gcmERR_BREAK(_FlushPendingPrimitives( vdrv, vdev ));

        gcmERR_BREAK(gco2D_Commit( vdrv->engine_2d, false ));
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT(Gal_Driver, "Failed to emit the commands.\n" );
    }

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * This is called by DirectFB core to probe which
 * driver supports particular hardware in the system.
 *
 * \param [in]  device: Core graphics device.
 *
 * \return  0: not supported.
 * \return  1: supported.
 */
static int
driver_probe( CoreGraphicsDevice *device )
{
    D_ASSERT( device != NULL );

    D_DEBUG_ENTER( Gal_Driver, "device: %p.\n", device );


    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return 1;
}

/**
 * Get the driver information.

 * \param [in]  device: Core graphics device.
 * \param [out] info: The variable to receive
 *                      the driver information.
 *
 * \return none
 */
static void
driver_get_info( CoreGraphicsDevice *device,
                 GraphicsDriverInfo *info )
{
    D_ASSERT( device != NULL );
    D_ASSERT( info   != NULL );

    D_DEBUG_ENTER( Gal_Driver, "device: %p, info: %p.\n", device, info );

    /* Fill in driver info structure. */
    snprintf( info->name,
              DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
              "Vivante GC 2D Accelerator Driver" );

    snprintf( info->vendor,
              DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
              "Vivante" );

    snprintf( info->url,
              DFB_GRAPHICS_DRIVER_INFO_URL_LENGTH,
              "http://www.vivantecorp.com" );

    /* The driver's version. */
    info->version.major = 1;
    info->version.minor = 0;

    /*
     * Fill in the size of the driver specific data
     * and the device specific data.
     */
    info->driver_data_size = sizeof( GalDriverData );
    info->device_data_size = sizeof( GalDeviceData );

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * Initialize the driver.
 *
 * After successively acquire all required resources,
 * the driver should register screens and layers. Also
 * needs to return list of callback functions for
 * hardware accelerations via funcs.
 *
 * \param [in]  device: core graphics device
 * \param [out] funcs: the pointer to receive the callback
 *                      functions
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 * \param [in]  core: core dfb data
 *
 * \return  DFB_OK: succeed
 * \return  DFB_INIT: failed
 */
static DFBResult
driver_init_driver( CoreGraphicsDevice  *device,
                    GraphicsDeviceFuncs *funcs,
                    void                *drv,
                    void                *dev,
                    CoreDFB             *core )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;
    DFBResult      ret  = DFB_OK;

    D_ASSERT( device != NULL );
    D_ASSERT( funcs  != NULL );
    D_ASSERT( drv    != NULL );
    D_ASSERT( dev    != NULL );
    D_ASSERT( core   != NULL );

    D_DEBUG_ENTER( Gal_Driver, "device: %p, funcs: %p, drv: %p, dev: %p.\n", device, funcs, drv, dev );

    vdrv->master = dfb_core_is_master( core );

    /* Setup the driver. */
    ret = gal_setup_driver( vdrv, vdev );
    if (ret != DFB_OK) {
        D_DEBUG_AT( Gal_Driver, "Failed to setup driver.\n" );

        return DFB_INIT;
    }

    /*
     * Initialize the state variables.
     * ie. state members in vdrv struct.
     */
    gal_set_default_state( vdrv );

    /* Fill in the function list. */

    /* State management. */
    funcs->CheckState       = galCheckState;
    funcs->SetState         = galSetState;

    /* Syncronization. */
    funcs->EngineSync       = galEngineSync;
    funcs->EmitCommands     = galEmitCommands;

    /* No need to implement flush read cache function.
     * It's totally same as EngineSync in our driver.
     * Each time dfb calls flush, it has already called EngineSync.
     */
/*  funcs->FlushReadCache   = galFlushReadCache; */

    /* Draw functions. */
    funcs->FillRectangle    = galFillRectangle;
    funcs->FillTriangle     = galFillTriangle;
#if (DIRECTFB_MAJOR_VERSION >= 1) && (DIRECTFB_MINOR_VERSION >= 6)
    funcs->FillTrapezoid    = galFillTrapezoid;
    funcs->BatchFill        = galBatchFill;
    funcs->Blit2            = galBlit2;
#endif
    funcs->DrawRectangle    = galDrawRectangle;
    funcs->DrawLine         = galDrawLine;

    /* Blit functions. */
    funcs->Blit             = galBlit;
    funcs->StretchBlit      = galStretchBlit;

#if GAL_SURFACE_POOL
    /* Join the pool. */
    if (!vdrv->master) {
        dfb_surface_pool_join( core,
                               vdev->pool,
                               &galSurfacePoolFuncs );
    }
#endif

    vdrv->core = core;

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return DFB_OK;
}

/**
 * Close the driver.
 *
 * \param [in]  device: core graphics device
 * \param [out] drv: driver specific data
 *
 * \return  none
 */
static void
driver_close_driver( CoreGraphicsDevice *device,
                     void               *drv )
{
    GalDriverData *vdrv = drv;
    /*GalDeviceData *vdev;*/

    D_ASSERT( device != NULL );
    D_ASSERT( drv    != NULL );

    D_DEBUG_ENTER( Gal_Driver,
                   "device: %p, drv: %p.\n",
                   device, drv );

    /*vdev = vdrv->vdev;*/

#if GAL_SURFACE_POOL
#endif /* GAL_SURFACE_POOL */

    gal_cleanup_driver( vdrv );

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * Initialize the device.
 *
 * \param [in]  device: core graphics device
 * \param [in]  device_info: device information
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return
 * \return
 */
static DFBResult
driver_init_device( CoreGraphicsDevice *device,
                    GraphicsDeviceInfo *device_info,
                    void               *drv,
                    void               *dev )
{
    GalDeviceData *vdev = dev;
    GalDriverData *vdrv = drv;
    DFBResult      ret  = DFB_OK;

    D_ASSERT( device      != NULL );
    D_ASSERT( device_info != NULL );
    D_ASSERT( drv         != NULL );
    D_ASSERT( dev         != NULL );

    D_DEBUG_ENTER( Gal_Driver,
                   "device: %p, device_info: %p, drv: %p, dev: %p.\n",
                   device, device_info, drv, dev );

    /* Setup the device. */
    ret = gal_setup_device( device, vdrv, vdev );
    if (ret != DFB_OK) {
        D_DEBUG_AT( Gal_Driver, "Failed to setup device.\n" );

        return DFB_INIT;
    }

    switch (vdev->chipModel)
    {
    case gcv300:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC300" );
        break;
    case gcv400:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC400" );
        break;
    case gcv410:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC410" );
        break;
    case gcv450:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC450" );
        break;
    case gcv500:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC500" );
        break;
    case gcv530:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC530" );
        break;
    case gcv600:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC600" );
        break;
    case gcv700:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC700" );
        break;
    case gcv800:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC800" );
        break;
    case gcv1000:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "GC1000" );
        break;
    default:
        snprintf( device_info->name,
                  DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH,
                  "%s", "unknown chip model" );
        break;
    }

    snprintf( device_info->vendor,
              DFB_GRAPHICS_DEVICE_INFO_VENDOR_LENGTH,
              "%s", "Vivante" );

    /* Set hardware capabilities. */
    device_info->caps.flags    = CCF_CLIPPING | CCF_NOTRIEMU;
    device_info->caps.accel    = VIVANTE_GAL_DRAWING_FUNCTIONS |
                                 VIVANTE_GAL_BLITTING_FUNCTIONS;
    device_info->caps.drawing  = VIVANTE_GAL_DRAWING_FLAGS;
    device_info->caps.blitting = VIVANTE_GAL_BLITTING_FLAGS;

    device_info->limits.surface_byteoffset_alignment = GAL_BYTEOFFSET_ALIGNMENT;
    device_info->limits.surface_pixelpitch_alignment = GAL_PIXELPITCH_ALIGNMENT;

#if GAL_SURFACE_POOL
    /* Initialize the surface pool. */
    dfb_surface_pool_initialize( vdrv->core,
                                 &galSurfacePoolFuncs,
                                 &vdev->pool );
#endif /* GAL_SURFACE_POOL */

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return DFB_OK;
}

/**
 * Close the device.
 *
 * \param [in]  device: core graphics device
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return  none
 */
static void
driver_close_device( CoreGraphicsDevice *device,
                     void               *drv,
                     void               *dev )
{
    GalDeviceData *vdev = dev;

    D_DEBUG_ENTER( Gal_Driver,
                   "device: %p, drv: %p, dev: %p.\n",
                   device, drv, dev );


#if GAL_SURFACE_POOL
    if (vdev->pool) {
        /* Destroy the surface pool. */
        dfb_surface_pool_destroy( vdev->pool );
        vdev->pool = NULL;
    }
#endif /* GAL_SURFACE_POOL */

    gal_cleanup_device( vdev );

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * Initialize the state.
 * ie. the state members in vdev.
 */
static void
gal_set_default_state( GalDriverData *vdrv )
{
    GalDeviceData *vdev;

    D_ASSERT( vdrv != NULL );

    vdev = vdrv->vdev;

    /* Set global alpha. */
    vdrv->global_alpha_value = 0xFF;

    vdrv->draw_src_global_alpha_mode  = gcvSURF_GLOBAL_ALPHA_OFF;
    vdrv->draw_dst_global_alpha_mode  = gcvSURF_GLOBAL_ALPHA_OFF;

    vdrv->blit_src_global_alpha_mode  = gcvSURF_GLOBAL_ALPHA_OFF;
    vdrv->blit_dst_global_alpha_mode  = gcvSURF_GLOBAL_ALPHA_OFF;

    /* Set the blending functions. */
    vdrv->src_blend = DSBF_SRCALPHA;
    vdrv->dst_blend = DSBF_INVSRCALPHA;

    /* Set the transparency. */
    if (!vdev->hw_2d_pe20) {
        vdrv->draw_trans  = gcvSURF_OPAQUE;
        vdrv->blit_trans  = gcvSURF_OPAQUE;

        vdrv->trans_color = 0;

    }
    else {
        vdrv->src_ckey = vdrv->dst_ckey = (DFBColor){0,0,0,0};
        vdrv->src_ckey32 = vdrv->dst_ckey32 = 0;

        vdrv->src_trans_mode = gcv2D_OPAQUE;
        vdrv->dst_trans_mode = gcv2D_OPAQUE;
        vdrv->pat_trans_mode = gcv2D_OPAQUE;
    }

    /* Set default ROP. */
    vdrv->draw_fg_rop = 0xCC;
    vdrv->draw_bg_rop = 0xCC;

    vdrv->blit_fg_rop = 0xCC;
    vdrv->blit_bg_rop = 0xCC;

    vdrv->src_ckey_blit_rop = 0xCC;
    vdrv->dst_ckey_blit_rop = 0xCC;

    /* Set default rotation. */
    vdrv->src_rotation    = gcvSURF_0_DEGREE;
    vdrv->dst_rotation    = gcvSURF_0_DEGREE;

    vdrv->draw_hor_mirror = false;
    vdrv->draw_ver_mirror = false;

    vdrv->blit_hor_mirror = false;
    vdrv->blit_ver_mirror = false;

    vdrv->dst_blend_no_alpha        = false;
    vdrv->need_rop_blend_w = false;

    /* Color. */
    vdrv->color            = 0;
    vdrv->modulate_color   = 0;
    vdrv->color_is_changed = false;

    /* Set default aligned width. */
    vdrv->src_aligned_width = 0;
    vdrv->dst_aligned_width = 0;

    /* Blend. */
    vdrv->draw_blend = false;
    vdrv->blit_blend = false;

    /* Saved flags. */
    vdrv->draw_flags = DSDRAW_NOFX;
    vdrv->blit_flags = DSBLIT_NOFX;

    /* Need to re-program the HW. */
    vdrv->need_reprogram = true;

    /* Initialize multiply status. */
    vdrv->src_pmp_src_alpha = gcv2D_COLOR_MULTIPLY_DISABLE;
    vdrv->dst_pmp_dst_alpha = gcv2D_COLOR_MULTIPLY_DISABLE;
    vdrv->src_pmp_glb_mode  = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
    vdrv->dst_pmp_dst_alpha = gcv2D_COLOR_MULTIPLY_DISABLE;

    vdrv->saved_modulate_flag = 0;

    /* Src and dst multiply for alpha blending. */
    vdrv->need_src_alpha_multiply = false;
    vdrv->need_glb_alpha_multiply = false;

    /* Source is YUV color format. */
    vdrv->src_is_yuv_format = false;

    /* Need smooth scaling. */
    vdrv->smooth_scale = false;

    /* Saved src and dest physical addresses. */
    vdrv->last_src_phys_addr = ~0;
    vdrv->last_dst_phys_addr = ~0;

    /* Set the default valid flag. */
    vdrv->valid = HSVF_NOFX | HSVF_PALETTE;

    /* Initialize accel function. */
    vdrv->accel = DFXL_NONE;

    /* Use the pool surface as the brush. */
    vdrv->use_pool_source = false;

    /* Use source. */
    vdrv->use_source = false;

    return;
}

/**
 * Initialize the memory mapping cache.
 *
 * \param [in]  mapping_cache:    memory mapping cache
 * \param [in]  max_mapping_num:  max mapping number in the cache
 * \param [in]  max_mapping_size: max total memory size in the cache
 *
 * \return
 */
static DFBResult
gal_init_memory_mapping_cache( GALMemoryMappingCache *mapping_cache,
                               unsigned int           max_size,
                               unsigned int           max_total_memory )
{
    DFBResult ret = DFB_OK;

    D_DEBUG_ENTER( Gal_Driver, "mapping_cache: %p, max_size: %u, max_total_memory: %u.\n", mapping_cache, max_size, max_total_memory );

    do {
        if (!mapping_cache) {
            ret = DFB_INVARG;
            break;
        }

        /* Create the hash table. */
        ret = direct_hash_create(  max_size,
                                  &mapping_cache->mappings );
        if (ret != DFB_OK) {
            D_DEBUG_AT( Gal_Driver,
                        "Failed to create the hash table for memory mapping cache.\n" );

            break;
        }

        mapping_cache->max_size         = max_size;
        mapping_cache->max_total_memory = max_total_memory;

    } while (0);

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return ret;
}

/**
 * Destroy the memory mapping cache.
 *
 * \param [in]  mapping_cache: memory mapping cache
 *
 * \return
 */
static void
gal_destroy_memory_mapping_cache( GALMemoryMappingCache *mapping_cache )
{
    D_DEBUG_ENTER( Gal_Driver , "\n");

    do {
        if (!mapping_cache || !mapping_cache->mappings) {
            return;
        }

        /* Destroy the hash table. */
        D_DEBUG_AT( Gal_Driver,
                    "Destroy the hash table for memory mapping cache.\n" );
        direct_hash_destroy( mapping_cache->mappings );

        mapping_cache->max_size         = 0;
        mapping_cache->max_total_memory = 0;
    } while (0);

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * Setup the driver.
 *
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return
 */
static DFBResult
gal_setup_driver( GalDriverData *drv,
                  GalDeviceData *dev )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = dev;
    gceSTATUS      status = gcvSTATUS_OK;
    int            pending_array_size;
    DFBResult      ret    = DFB_OK;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );

    D_DEBUG_ENTER( Gal_Driver, "vdrv: %p, vdev: %p.\n", vdrv, vdev );

    /* Save the current hardware type */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

    do {
        /* Create OS object. */
        gcmERR_BREAK(gcoOS_Construct( NULL, &vdrv->os ));

        /* Create HAL object. */
        gcmERR_BREAK(gcoHAL_Construct( NULL, vdrv->os, &vdrv->hal ));

        /* Get the gco2D object pointer. */
        gcmERR_BREAK(gco2D_Construct( vdrv->hal, &vdrv->engine_2d ));
        gcmERR_BREAK(gco2D_Set2DEngine(vdrv->engine_2d));

        if (vdrv->master) {
            /* Determine whether PE 2.0 is present. */
            vdev->hw_2d_pe20 = gcoHAL_IsFeatureAvailable(vdrv->hal,
                                                         gcvFEATURE_2DPE20)
                               == gcvSTATUS_TRUE;

            /* Determine whether the 2D core for DFB is present. */
            vdev->hw_2d_full_dfb = gcoHAL_IsFeatureAvailable(vdrv->hal,
                                                             gcvFEATURE_FULL_DIRECTFB)
                                   == gcvSTATUS_TRUE;

           /* Determine whether 2D hardware support multi source blit. */
            vdev->hw_2d_multi_src = gcoHAL_IsFeatureAvailable(vdrv->hal,
                                                              gcvFEATURE_2D_MULTI_SOURCE_BLT)
                                   == gcvSTATUS_TRUE;

#if GAL_SURFACE_COMPRESSED
           /* Determine whether 2D hardware support compression. */
            vdev->hw_2d_compression = gcoHAL_IsFeatureAvailable(vdrv->hal, gcvFEATURE_2D_COMPRESSION)
                                      == gcvTRUE;

            vdrv->per_process_compression = gcoOS_DetectProcessByEncryptedName("\x9b\x99\xa0\x9b\x90\x94")
                                            == gcvTRUE;
#else
            vdev->hw_2d_compression = gcvFALSE;
            vdrv->per_process_compression = gcvFALSE;
#endif

            vdev->hw_yuv420_output = gcoHAL_IsFeatureAvailable(vdrv->hal, gcvFEATURE_2D_YUV420_OUTPUT_LINEAR) == gcvTRUE;

            vdev->max_pending_num  = MAX_PENDING_NUM;
            vdev->gard_pending_num = GARD_PENDING_NUM;
        }

        if (!vdev->hw_2d_pe20) {
            /* Allocate and save a temporary video memory space. */
            vdrv->pool_src_format        = DSPF_ARGB;
            vdrv->pool_src_native_format = gcvSURF_A8R8G8B8;

            vdrv->pool_src_rect.left   = 0;
            vdrv->pool_src_rect.top    = 0;
            vdrv->pool_src_rect.right  = 1;
            vdrv->pool_src_rect.bottom = 1;

            vdrv->pool_src_width  = POOL_SRC_WIDTH;
            vdrv->pool_src_height = POOL_SRC_HEIGHT;

            gcmERR_BREAK(gcoSURF_Construct( vdrv->hal,
                                            vdrv->pool_src_width,
                                            vdrv->pool_src_height,
                                            1,
                                            gcvSURF_BITMAP,
                                            vdrv->pool_src_native_format,
                                            gcvPOOL_DEFAULT,
                                            &vdrv->pool_surf ));

            gcmERR_BREAK(gcoSURF_Lock( vdrv->pool_surf,
                                       &vdrv->pool_src_phys_addr,
                                       &vdrv->pool_src_logic_addr ));

            gcmERR_BREAK(gcoSURF_GetAlignedSize( vdrv->pool_surf,
                                                 &vdrv->pool_src_aligned_width,
                                                 &vdrv->pool_src_aligned_height,
                                                 &vdrv->pool_src_pitch ));

            D_DEBUG_AT( Gal_Driver,
                        "vdrv->pool_src_width: %u\n"
                        "vdrv->pool_src_height: %u\n"
                        "vdrv->pool_src_native_format: %u\n"
                        "vdrv->pool_src_phys_addr: 0x%08X\n"
                        "vdrv->pool_src_logic_addr: %p\n"
                        "vdrv->pool_src_pitch: %u\n",
                        vdrv->pool_src_width,
                        vdrv->pool_src_height,
                        vdrv->pool_src_native_format,
                        vdrv->pool_src_phys_addr,
                        vdrv->pool_src_logic_addr,
                        vdrv->pool_src_pitch );

            /* Setup the tmp surface. */
            vdrv->tmp_dst_surf       = NULL;
            vdrv->tmp_dst_format     = gcvSURF_A8R8G8B8;

            vdrv->tmp_dst_logic_addr = NULL;
            vdrv->tmp_dst_phys_addr  = ~0;

            vdrv->tmp_dst_width      = 0;
            vdrv->tmp_dst_height     = 0;
        }

        /* Initialize tile. */
        vdrv->src_tiling = gcvLINEAR;
        vdrv->src_tile_status = gcv2D_TSC_DISABLE;
        vdrv->src_tile_status_addr = gcvINVALID_ADDRESS;
        vdrv->dst_tiling = gcvLINEAR;
        vdrv->dst_tile_status = gcv2D_TSC_DISABLE;
        vdrv->dst_tile_status_addr = gcvINVALID_ADDRESS;

        /* Initialize pending primitives. */
        vdrv->pending_num  = 0;
        vdrv->pending_type = GAL_PENDING_NONE;

        pending_array_size = vdev->max_pending_num + vdev->gard_pending_num;

        vdrv->pending_src_rects = D_CALLOC( 1, sizeof(gcsRECT) * pending_array_size );
        gcmERR_BREAK(vdrv->pending_src_rects ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);

        vdrv->pending_dst_rects = D_CALLOC( 1, sizeof(gcsRECT) * pending_array_size );
        gcmERR_BREAK(vdrv->pending_dst_rects ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);

        vdrv->pending_colors = D_CALLOC( 1, sizeof(unsigned int) * pending_array_size );
        gcmERR_BREAK(vdrv->pending_colors ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);

        /* Set alpha blending function list. */
        if (vdev->hw_2d_full_dfb) {
            vdrv->src_blend_funcs = gal_source_blend_funcs_full_dfb;
            vdrv->dst_blend_funcs = gal_dest_blend_funcs_full_dfb;
        } else {
            vdrv->src_blend_funcs = gal_source_blend_funcs;
            vdrv->dst_blend_funcs = gal_dest_blend_funcs;
        }

        /* Initialize the memory mapping cache. */
        vdrv->memory_mapping_cache.max_size         = 0;
        vdrv->memory_mapping_cache.max_total_memory = 0;
        vdrv->memory_mapping_cache.mappings         = 0;
        ret = gal_init_memory_mapping_cache( &vdrv->memory_mapping_cache,
                                              MAX_MEMORY_MAPPING_CACHE_SIZE,
                                              MAX_MEMORY_MAPPING_TOTAL_MEMORY_SIZE );
        if (ret != DFB_OK) {
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }

        /* Allocate rectangle converting array */
        vdrv->convert_rect = D_CALLOC(1, sizeof(gcsRECT) * 1024);
        gcmERR_BREAK(vdrv->convert_rect ? gcvSTATUS_OK : gcvSTATUS_OUT_OF_MEMORY);
        vdrv->cur_convert_rect_num = 1024;

        /* No HW acceleration is called yet. */
        vdrv->dirty = false;

        /* Save the device data pointer. */
        vdrv->vdev = vdev;

    } while (0);

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Driver, "Failed to setup the driver.\n" );

        /* Destroy the memory mapping cache. */
        gal_destroy_memory_mapping_cache( &vdrv->memory_mapping_cache );

        /* Destroy the tmp dest surface. */
        if (!vdev->hw_2d_pe20) {
            if (vdrv->pool_surf) {
                /* Unlock the pool surface. */
                if (vdrv->pool_src_logic_addr) {
                    gcmVERIFY_OK(gcoSURF_Unlock( vdrv->pool_surf, vdrv->pool_src_logic_addr ));
                    vdrv->pool_src_logic_addr = NULL;
                }

                /* Destroy the pool surface. */
                gcmVERIFY_OK(gcoSURF_Destroy( vdrv->pool_surf ));
                vdrv->pool_surf = NULL;
            }
        }

        /* Free pending primitives. */
        if (vdrv->pending_src_rects) {
            D_FREE( vdrv->pending_src_rects );
            vdrv->pending_src_rects = NULL;
        }

        if (vdrv->pending_dst_rects) {
            D_FREE( vdrv->pending_dst_rects );
            vdrv->pending_dst_rects = NULL;
        }

        if (vdrv->pending_colors) {
            D_FREE( vdrv->pending_colors );
            vdrv->pending_colors = NULL;
        }

        vdrv->pending_type = GAL_PENDING_NONE;
        vdrv->pending_num  = 0;

        /* Destroy engine object. */
        if (vdrv->engine_2d)
        {
            gcmVERIFY_OK(gco2D_UnSet2DEngine(vdrv->engine_2d));
            gcmVERIFY_OK(gco2D_Destroy(vdrv->engine_2d));
        }

        /* Destroy hal object. */
        if (vdrv->hal) {
            gcmVERIFY_OK(gco2D_Commit( vdrv->engine_2d, true ));
            gcmVERIFY_OK(gcoHAL_Destroy( vdrv->hal ));

            vdrv->hal = NULL;
        }

        /* Destroy os object. */
        if (vdrv->os) {
            gcmVERIFY_OK(gcoOS_Destroy( vdrv->os ));
            vdrv->os = NULL;
        }

        ret = DFB_INIT;
    }

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return ret;
}

static bool
gal_release_memory_mappings( DirectHash    *hash,
                             unsigned long  key,
                             void          *value,
                             void          *ctx )
{
    gceSTATUS status = gcvSTATUS_OK;
    bool      ret    = true;

    D_DEBUG_ENTER( Gal_Driver, "\n" );

    do {
        GALMemoryMapping *mapping = value;

        if (!value) {
            break;
        }

        D_DEBUG_AT( Gal_Driver,
                    "Unmap user memory:\n"
                    "\tlogic_addr: %p\n"
                    "\tsize: %d\n"
                    "\tgpu_addr: 0x%08X\n",
                    mapping->logic_addr,
                    mapping->size,
                    mapping->gpu_addr );

        gcmERR_BREAK(gcoHAL_UnlockVideoMemory( mapping->node,
                                               gcvSURF_BITMAP,
                                               gcvENGINE_RENDER));

        gcmERR_BREAK(gcoHAL_ReleaseVideoMemory( mapping->node ));

        D_FREE( mapping );
    } while (0);

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Driver,
                    "Failed to unmap user memory.\n" );

        ret = false;
    }

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return ret;
}

/**
 * Cleanup the driver.
 *
 * \param [in]  drv: driver specific data
 *
 * \return  none
 */
static void
gal_cleanup_driver( GalDriverData *drv )
{
    GalDriverData *vdrv   = drv;
    GalDeviceData *vdev   = vdrv->vdev;
    gceSTATUS      status = gcvSTATUS_OK;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    D_ASSERT( drv  != NULL );

    D_DEBUG_ENTER( Gal_Driver, "\n" );

    /* Save the current hardware type */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

    do {
        /* Destroy the resources. */
        if (vdrv->tmp_dst_surf) {
            if (vdrv->tmp_dst_logic_addr) {
                gcmVERIFY_OK(gcoSURF_Unlock( vdrv->tmp_dst_surf, vdrv->tmp_dst_logic_addr ));
                vdrv->tmp_dst_logic_addr = NULL;
            }

            gcmVERIFY_OK(gcoSURF_Destroy( vdrv->tmp_dst_surf ));
            vdrv->tmp_dst_surf = NULL;
        }

        if (!vdev->hw_2d_pe20) {

            /* Destroy the pool surface. */
            if (vdrv->pool_surf) {
                if (vdrv->pool_src_logic_addr) {
                    gcmVERIFY_OK(gcoSURF_Unlock( vdrv->pool_surf, vdrv->pool_src_logic_addr ));
                    vdrv->pool_src_logic_addr = NULL;
                }

                gcmVERIFY_OK(gcoSURF_Destroy( vdrv->pool_surf ));
                vdrv->pool_surf = NULL;
            }
        }

        /* Free pending primitives. */
        if (vdrv->pending_src_rects) {
            D_FREE( vdrv->pending_src_rects );
            vdrv->pending_src_rects = NULL;
        }

        if (vdrv->pending_dst_rects) {
            D_FREE( vdrv->pending_dst_rects );
            vdrv->pending_dst_rects = NULL;
        }

        if (vdrv->pending_colors) {
            D_FREE( vdrv->pending_colors );
            vdrv->pending_colors = NULL;
        }

        vdrv->pending_type = GAL_PENDING_NONE;
        vdrv->pending_num  = 0;

        /* Release the mappings. */
        direct_hash_iterate( vdrv->memory_mapping_cache.mappings,
                             gal_release_memory_mappings,
                             NULL );

        /* Destroy the memory mapping cache. */
        gal_destroy_memory_mapping_cache( &vdrv->memory_mapping_cache );

        if (vdrv->convert_rect != NULL) {
            D_FREE(vdrv->convert_rect);
            vdrv->convert_rect = NULL;
        }

        /* Destroy engine object. */
        if (vdrv->engine_2d)
        {
            gcmERR_BREAK(gco2D_UnSet2DEngine(vdrv->engine_2d));
            gcmERR_BREAK(gco2D_Destroy(vdrv->engine_2d));
        }

        /* Destroy hal object. */
        if (vdrv->hal) {
            gcmVERIFY_OK(gco2D_Commit( vdrv->engine_2d, true ));
            gcmVERIFY_OK(gcoHAL_Destroy( vdrv->hal ));

            vdrv->hal = NULL;
        }

        /* Destroy os object. */
        if (vdrv->os) {
            gcmERR_BREAK(gcoOS_Destroy( vdrv->os ));
            vdrv->os = NULL;
        }
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {
        D_DEBUG_AT( Gal_Driver, "Failed to cleanup the driver.\n" );
    }

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}

/**
 * Setup the device.
 *
 * \param [in]  drv: driver specific data
 * \param [in]  dev: device specific data
 *
 * \return
 */
static DFBResult
gal_setup_device( CoreGraphicsDevice *device,
                  GalDriverData      *drv,
                  GalDeviceData      *dev )
{
    GalDriverData *vdrv = drv;
    GalDeviceData *vdev = dev;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;

    gceSTATUS status = gcvSTATUS_OK;
    DFBResult ret    = DFB_OK;

    D_ASSERT( drv != NULL );
    D_ASSERT( dev != NULL );

    /* Save the current hardware type */
    gcmVERIFY_OK(gcoHAL_GetHardwareType(gcvNULL, &currentType));
    gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_2D));

    do {
        /* Get HW info. */
        gcmERR_BREAK(gcoHAL_QueryChipIdentity( vdrv->hal,
                                               &vdev->chipModel,
                                               &vdev->chipRevision,
                                               gcvNULL,
                                               gcvNULL));

        D_DEBUG_AT( Gal_Driver,
                    "[Hardware info] chipModel: 0x%08X, chipRevision: 0x%08X\n",
                    vdev->chipModel, vdev->chipRevision);

        gcmERR_BREAK(gcoOS_GetBaseAddress( vdrv->os, &vdev->baseAddress));

        D_DEBUG_AT( Gal_Driver,
                    "Memory base address: 0x%08X.\n", vdev->baseAddress );

        /* Initialize the index color table. */
        vdev->palette = NULL;
        vdev->color_table_len = 0;
        memset( vdev->color_table, 0, 256 );

        /* Initialize configurations. */
        if (gal_config_init( vdev, &vdev->config ) != DFB_OK) {
            D_ERROR( "Failed to get the configuration file.\n" );

            status = gcvSTATUS_GENERIC_IO;
            break;
        }

#if CONF_DEBUG
        if (vdev->config.filename[0] != '\0')
        {
            printf(" config->filename: %s\n", vdev->config.filename );
        } else {
            printf(" config->filename is empty.\n");
        }

        printf( "[FILLRECTANGLE] NONE: %u, XOR: %u, BLEND: %u\n", vdev->config.primitive_settings[GAL_CONF_FILLRECTANGLE].none, !!(vdev->config.primitive_settings[GAL_CONF_FILLRECTANGLE].flags & DSDRAW_XOR), !!(vdev->config.primitive_settings[GAL_CONF_FILLRECTANGLE].flags & DSDRAW_BLEND) );

        printf( "[DRAWRECTANGLE] NONE: %u, XOR: %u, BLEND: %u\n", vdev->config.primitive_settings[GAL_CONF_DRAWRECTANGLE].none, !!(vdev->config.primitive_settings[GAL_CONF_DRAWRECTANGLE].flags & DSDRAW_XOR), !!(vdev->config.primitive_settings[GAL_CONF_DRAWRECTANGLE].flags & DSDRAW_BLEND) );

        printf( "[DRAWLINE     ] NONE: %u, XOR: %u, BLEND: %u\n", vdev->config.primitive_settings[GAL_CONF_DRAWLINE].none, !!(vdev->config.primitive_settings[GAL_CONF_DRAWLINE].flags & DSDRAW_XOR), !!(vdev->config.primitive_settings[GAL_CONF_DRAWLINE].flags & DSDRAW_BLEND) );

        printf( "[FILLTRIANGLE ] NONE: %u, XOR: %u, BLEND: %u\n", vdev->config.primitive_settings[GAL_CONF_FILLTRIANGLE].none, !!(vdev->config.primitive_settings[GAL_CONF_FILLTRIANGLE].flags & DSDRAW_XOR), !!(vdev->config.primitive_settings[GAL_CONF_FILLTRIANGLE].flags & DSDRAW_BLEND) );

        printf( "[BLIT         ] NONE: %u, XOR: %u, ALPHACHANNEL: %u, COLORALPHA: %u, COLORIZE: %u, SRC_COLORKEY: %u, ROTATE180: %u\n", vdev->config.primitive_settings[GAL_CONF_BLIT].none, !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_XOR), !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_BLEND_ALPHACHANNEL), !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_BLEND_COLORALPHA), !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_COLORIZE), !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_SRC_COLORKEY), !!(vdev->config.primitive_settings[GAL_CONF_BLIT].flags & DSBLIT_ROTATE180) );

        printf( "[STRETCHBLIT  ] NONE: %u, XOR: %u, ALPHACHANNEL: %u, COLORALPHA: %u, COLORIZE: %u, SRC_COLORKEY: %u, ROTATE180: %u\n", vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].none, !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_XOR), !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_BLEND_ALPHACHANNEL), !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_BLEND_COLORALPHA), !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_COLORIZE), !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_SRC_COLORKEY), !!(vdev->config.primitive_settings[GAL_CONF_STRETCHBLIT].flags & DSBLIT_ROTATE180) );
#endif

        /* Set threshold for rendering small objects. */
        vdev->turn_on_threshold = false;
        vdev->disable_threshold = false;
        vdev->threshold_value   = THRESHOLD_VALUE;
    } while (0);

    if (currentType != gcvHARDWARE_2D)
    {
        /* Restore the current hardware type */
        gcmVERIFY_OK(gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    if (gcmIS_ERROR( status )) {

        D_DEBUG_AT( Gal_Driver, "Failed to setup GAL device." );

        ret = DFB_INIT;
    }

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return ret;
}

/**
 * Cleanup the device.
 *
 * \param [in]  dev: device specific data
 *
 * \return  none
 */
static void
gal_cleanup_device( GalDeviceData *dev )
{
    D_DEBUG_ENTER( Gal_Driver, "\n" );

    D_DEBUG_EXIT( Gal_Driver, "\n" );

    return;
}
