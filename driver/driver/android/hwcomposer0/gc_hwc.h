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


#ifndef __gc_hwc_h_
#define __gc_hwc_h_


/*******************************************************************************
** Build options.
*/

/*
    ENABLE_SWAP_RECTANGLE

        Set to 1 for enabling swap rectangle optimization.
*/
#define ENABLE_SWAP_RECTANGLE       1

/*
    ENABLE_PLANE_ALPHA

        Set to 1 to support plane alpha in hwc.
        This needs android system patch for surfaceflinger.
*/
#define ENABLE_PLANE_ALPHA          1

/*
    ENABLE_DIM

        Set to 1 for enabling dim layer support in hwc.
        This needs android system patch for surfaceflinger.
*/
#define ENABLE_DIM                  1

/*
    ENABLE_CLEAR_HOLE

        Set it to 1 to enable clear hole support in hwc.
        This needs android system patch for surfaceflinger.

        NOTICE: This feature will block arbitrary rotation. Disable it if
        arbitrary rotation is needed.
*/
#define ENABLE_CLEAR_HOLE           0

/*
    CLEAR_FB_FOR_OVERLAY

        Enable it to clear overlay area to transparet in framebuffer memory.
        It is done with glClear for 3D composition (SurfaceFlinger.cpp),
        and with a 2D clear for hwcomposer (gc_hwc_set.cpp).
*/
#define CLEAR_FB_FOR_OVERLAY        1

/*
    FILTER_STRETCH_BY_DEFAULT

        Enable filter blit for stretching by default.
        Filter blit is slower than stretch but has better visual quality.
        Filter stretch configuration can be obtained from runtime property
        'hwc.stretch.filter', '1' for enabling while '0' for disabling.
        This build option will set its default value.
*/
#define FILTER_STRETCH_BY_DEFAULT   0

/*
    ENABLE_DITHER

        Enable it to use dither for output.
        Dithering is only when blit from higher precision to lower precision.
*/
#define ENABLE_DITHER               0


/*
 * Multi-source blit parameter.
 * YUV buffer address should always be 64-byte aligned per spec. But it may
 * also depend on the Soc.
 */
#define YUV_BUFFER_ALIGN            64

/*
 * Multi-source blit parameter.
 * Without 'Multi-source Blit Ex' feature, the buffer address should be 64-byte
 * aligned per spec. But it may also depend on the Soc.
 */
#define RGB_BUFFER_ALIGN            16


/******************************************************************************/

#include <stdlib.h>

#include <hardware/hwcomposer.h>
#include <log/log.h>

#if ANDROID_SDK_VERSION >= 16
#   include <ui/ANativeObjectBase.h>

#   undef LOGI
#   undef LOGD
#   undef LOGW
#   undef LOGE
#   define LOGI(...) ALOGI(__VA_ARGS__)
#   define LOGD(...) ALOGD(__VA_ARGS__)
#   define LOGW(...) ALOGW(__VA_ARGS__)
#   define LOGE(...) ALOGE(__VA_ARGS__)
#else
#   include <ui/android_native_buffer.h>
#endif

#include <gc_hal_base.h>
#include <gc_hal_raster.h>


#ifdef __cplusplus
extern "C" {
#endif

/* Extended composition types. */
enum
{
    /* NOTE: These enums are unknown to Android.
     * Android only checks against HWC_FRAMEBUFFER. */
    HWC_BLITTER = 100,
    HWC_DIM,
    HWC_CLEAR_HOLE,
    HWC_BYPASS
};


/* A framebuffer buffer. */
struct hwcBuffer;

/* Framebuffer window struct. */
struct hwcFramebuffer
{
    /* Framebuffer resolution. */
    gcsRECT                          res;

    /* Framebuffer stride. */
    gctUINT32                        stride;

    /* Framebuffer tiling. */
    gceTILING                        tiling;

    /* Framebuffer format. */
    gceSURF_FORMAT                   format;

    /* Bytes per pixel. */
    gctUINT32                        bytesPerPixel;

    /* Alignment mask for multi-source blit. */
    gctUINT32                        alignMask;

    /* Swap rectangles are valid? */
    gctBOOL                          valid;

    /* The first framebuffer buffer. */
    struct hwcBuffer *               head;

    /* Current frameubffer buffer. */
    struct hwcBuffer *               target;
};


/* Framebuffer buffers in linked list. */
struct hwcBuffer
{
     /* Buffer handle. */
     buffer_handle_t                 handle;

     /* Physical address of this buffer. */
     gctUINT32                       physical;

     /* Swap rectangle. */
     gcsRECT                         swapRect;

     /* Point to prev/next buffer. */
     struct hwcBuffer *              prev;
     struct hwcBuffer *              next;
};


/* Area struct. */
struct hwcArea
{
    /* Area potisition. */
    gcsRECT                          rect;

    /* Bit field, layers who own this Area. */
    gctUINT32                        owners;

    /* Point to next area. */
    struct hwcArea *                 next;
};


/* Area pool struct. */
struct hwcAreaPool
{
    /* Pre-allocated areas. */
    hwcArea *                        areas;

    /* Point to free area. */
    hwcArea *                        freeNodes;

    /* Next area pool. */
    hwcAreaPool *                    next;
};


/* Layer struct. */
struct hwcLayer
{
    /* Composition type of this layer. */
    gctUINT32                        compositionType;

    /***************************************************************************
    ** Color source from surface memory.
    */

    /* Source surface. */
    gcoSURF                          source;

    /* Source address. */
    gctUINT32                        addresses[3];
    gctUINT32                        addressNum;

    /* Source stride. */
    gctUINT32                        strides[3];
    gctUINT32                        strideNum;

    /* Source tiling. */
    gceTILING                        tiling;

    /* Source format. */
    gceSURF_FORMAT                   format;

    /* Source rotation. */
    gceSURF_ROTATION                 rotation;

    /* Source size. */
    gctUINT32                        width;
    gctUINT32                        height;

    /* Source orientation. */
    gceORIENTATION                   orientation;

    /* Mirror parameters. */
    gctBOOL                          hMirror;
    gctBOOL                          vMirror;

    /* Clipped source rectangle against full screen, in trasformed coord-sys. */
    gcsRECT                          srcRect;

    /* YUV format. */
    gctBOOL                          yuv;

    /* Bytes per pixel. */
    gctUINT32                        bytesPerPixel;

    /* Alignment masks for multi-source blit. */
    gctUINT32                        alignMaskX;
    gctUINT32                        alignMaskY;

    /* Original source rectangle before transformation. */
    gcsRECT                          orgRect;

    /***************************************************************************
    ** Color source from solid color.
    */

    /* Color value, this is for OVERLAY, CLEAR_HOLE and DIM. */
    gctUINT32                        color32;

    /***************************************************************************
    ** Alpha blending and premultiply parametes.
    */

    /* Opaque layer, means no alpha blending. */
    gctBOOL                          opaque;

    /* Alpha blending mode. */
    gceSURF_PIXEL_ALPHA_MODE         srcAlphaMode;
    gceSURF_PIXEL_ALPHA_MODE         dstAlphaMode;

    /* Global alpha blending mode. */
    gceSURF_GLOBAL_ALPHA_MODE        srcGlobalAlphaMode;
    gceSURF_GLOBAL_ALPHA_MODE        dstGlobalAlphaMode;

    /* Blend factor. */
    gceSURF_BLEND_FACTOR_MODE        srcFactorMode;
    gceSURF_BLEND_FACTOR_MODE        dstFactorMode;

    /* Perpixel alpha premultiply. */
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  srcPremultSrcAlpha;
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstPremultDstAlpha;

    /* Source global alpha premultiply. */
    gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultGlobalMode;

    /* Dest global alpha demultiply. */
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstDemultDstAlpha;

    /* Global alpha value. */
    gctUINT32                        srcGlobalAlpha;
    gctUINT32                        dstGlobalAlpha;

    /***************************************************************************
    ** Dest information.
    */

    /* Clipped dest rectangle against full screen. */
    gcsRECT                          dstRect;

    /* Original dest rectangle before clipping. */
    gcsRECT                          orgDest;

    /* Clip rectangles on dest. */
    gcsRECT_PTR                      clipRects;

    /* Number of clip rectangles. */
    gctUINT                          clipCount;

    /***************************************************************************
    ** Stretching.
    */

    /* Stretch bit flag. */
    gctBOOL                          stretch;

    /* Stretch blit: factors. */
    gctUINT32                        hFactor;
    gctUINT32                        vFactor;

    /* Stretch blit: steps. */
    gctINT32                         srcXStep;
    gctINT32                         srcYStep;
    gctINT32                         dstXStep;
    gctINT32                         dstYStep;

    /* Filter blit flag. */
    gctBOOL                          filter;

    /* Filter blit: kernel size. */
    gctUINT8                         hKernel;
    gctUINT8                         vKernel;
};


/* HWC context. */
struct hwcContext
{
    hwc_composer_device_t device;

#if ANDROID_SDK_VERSION >= 14
    /* ICS and later. */
    const hwc_procs_t *   callback;
#endif

    /***************************************************************************
    ** Features.
    */

    /* Software environment: filter stretch is enabled? */
    gctBOOL                          filterStretch;

    /* Feature: 2D separated core? */
    gctBOOL                          separated2D;

    /* Feature: 2D separated YUV stride? */
    gctBOOL                          yuvSeparateStride;

    /* Feature: 2D multi-source blit capability. */
    gctBOOL                          multiSourceBlt;

    /* Feature: 2D multi-source blit source mirror. */
    gctBOOL                          msSourceMirror;

    /* Feature: 2D multi-source blit source count. */
    gctUINT32                        msMaxSource;

    /* Feature: 2D multi-source blit byte alignment. */
    gctBOOL                          msByteAlignment;

    /* Feature: 2D multi-source blit prefer source rotation. */
    gctBOOL                          msSourceRotation;

    /* Feature: 2D multi-source blit can move to odd coordinates. */
    gctBOOL                          msOddCoord;

    /* Feature: 2D multi-source blit with source offset. */
    gctBOOL                          msSourceOffset;

    /* Feature: One pass filter/YUV blit. */
    gctBOOL                          opf;

    /* Feature: 2D tiled input. */
    gctBOOL                          tiledInput;

    /* Feature: 2D compression output. */
    gctBOOL                          compression;

    /***************************************************************************
    ** States.
    */

    /* Geometry change flag. */
    gctBOOL                          geometryChanged;

    /* Flag: has OpenGL ES composition. */
    gctBOOL                          hasGles;

    /* Flag: has 2D composition. */
    gctBOOL                          hasG2D;

    /* Flag: has dim layer. */
    gctBOOL                          hasDim;

    /* Flag: has clear hole layer. */
    gctBOOL                          hasClearHole;

    /* Flag: has overlay layer. */
    gctBOOL                          hasOverlay;

    /* Flag: all layers are overlay. */
    gctBOOL                          pureOverlay;

    /* Target framebuffer information. */
    hwcFramebuffer *                 framebuffer;

    /* Layers, max 32 layers. */
    gctUINT32                        layerCount;
    hwcLayer                         layers[32];

    /* Splited composition area queue. */
    hwcArea *                        compositionArea;

    /* Rectangles to be copyed from previous buffer. */
    hwcArea *                        swapArea;

    /* Pre-allocated area pool. */
    hwcAreaPool *                    areaPool;

    /***************************************************************************
    ** Debug dump optionis.
    */

    /*
     * Dump information of source layers.
     * Dump 2D compositioin detail.
     * Dump hwc composition time of each frame.
     *
     * Enable by property 'hwc.debug.dump_compose'.
     */

    gctBOOL                         dumpCompose;
    /*
     * Dump source input and target output into bitmap (/data/dump).
     * Make sure /data/dump has write permission.
     *
     * Enable by property 'hwc.debug.dump_bitmap'.
     */
    gctBOOL                         dumpBitmap;

    /*
     * Dump area spliting information.
     * Dump swap area information (swap rectangle optimization).
     *
     * Enable by property 'hwc.debug.dump_split_area'.
     */
    gctBOOL                         dumpSplitArea;

    /***************************************************************************
    ** GC Objects.
    */

    /* OS object. */
    gcoOS                            os;

    /* HAL object. */
    gcoHAL                           hal;

    /* Raster engine */
    gco2D                            engine;

    /***************************************************************************
    ** Vsync states.
    */
    void *                          vsync;
};


/*******************************************************************************
** Functions.
*/

gceSTATUS
hwcPrepare(
    IN hwcContext * Context,
    IN hwc_layer_list_t * List
    );


gceSTATUS
hwcSet(
    IN hwcContext * Context,
    IN android_native_buffer_t * BackBuffer,
    IN hwc_layer_list_t * List
    );


gceSTATUS
hwcComposeG2D(
    IN hwcContext * Context
    );

int
hwcVsyncSetup(
    IN hwcContext * Context
    );

int
hwcVsyncPeriod(
    IN hwcContext * Context
    );

int
hwcVsyncStop(
    IN hwcContext * Context
    );

int
hwcVsyncControl(
    IN hwcContext * Context,
    IN int Enable
    );

int
hwcOverlay(
    IN hwcContext * Context,
    IN hwc_layer_list_t * List
    );


#ifdef __cplusplus
}
#endif

#endif /* __gc_hwc_h_ */

