/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef blitter_h_
#define blitter_h_

/*
 * NOTICE: SOC vendor:
 * Vivante blitter code. Report bugs to Vivante, instead of change it.
 */
#include <gc_hal.h>
#include <gc_hal_base.h>

struct __hwc2_device;
struct __hwc2_display;
struct __hwc2_layer;

/* Blitter structures. */
typedef struct blitter_device    blitter_device_t;
typedef struct blitter_display   blitter_display_t;
typedef struct blitter_layer     blitter_layer_t;

struct blitter_device
{
    /* Hardware types. */
    gceHARDWARE_TYPE currentHwType;
    gceHARDWARE_TYPE blitterHwType;;
    gceHARDWARE_TYPE hwType3D;

    /* The blitter engine. */
    gco2D engine;

    /* Features. */
    int onePassFilter;
    int tiledInput;
    int compression2D;

    int mirrorExt;
    int yuvSeparateStride;

    int multiBlt;
    int multiBltEx;
    int multiBlt1_5;
    int multiBlt2;

    int multiBltSourceMirror;
    int multiBltByteAlignment;
    int multiBltSourceRotation;
    int multiBltOddCoord;
    int multiBltSourceOffset;
    uint32_t multiBltMaxSource;
    uint32_t multiBltMaxYuvSource;

    /* Options. */
    /* Use filter-blit for scaling. */
    int filterScale;


    /* Debug options. */
};

struct area
{
    hwc_rect_t rect;

    /* Mask to reference blitter_display::bltLayers. */
    uint64_t layerMask;

    /* Link area in list. */
    __hwc2_list_t link;
};

struct surface
{
    gcoSURF surface;

    uint32_t width;
    uint32_t height;
    gceSURF_FORMAT format;
    gcsSURF_FORMAT_INFO_PTR formatInfo;
    uint32_t bytesPerPixel;
    gceTILING tiling;
    uint32_t address[3];
    uint32_t stride[3];
    uint32_t numPlanes;
    gce2D_TILE_STATUS_CONFIG tsConfig;

    uint32_t alignMaskX;    /* For multi-blit. */
    uint32_t alignMaskY;
};

struct output
{
    buffer_handle_t buffer;

    /*
     * Buffer age.
     * The 'age' is the number of frames elapsed since the contents were most
     * recently defined. The back buffer can either invalid (has an age of 0)
     * or it may contain the contents from n frames prior to the current frame.
     *
     * The current back buffer's age is set to 1. Any other color buffers' ages
     * are incremented by 1 if their age was previously greater than 0.
     */
    uint32_t age;

    /* A list of struct area. */
    __hwc2_list_t damage;

    /* Link outputs together. */
    __hwc2_list_t link;
};

/*
 * Tough layers forced blitting by layer, but not area.
 * 1. Scaled, and stretch blit is used, can not split such layer
 *    because of hardware precision issue in stretch blit.
 * 2. Top and tiny layers, it does not worth splittng others.
 *
 * In some conditions, tough layer does not need take part in split-area:
 * 1. at top
 * 2. but not bottom
 * 3. alpha blending enabled
 * Without split-area, such tough-blend-holes become empty-holes.
 *
 * Area usage concepts:
 *
 *
 *                          top, small, tough, blend or not  -->  +------------+
 *                                                                |   layer 4  |
 *                                  +-----------------------------+-+----------+
 *                                  |       tough layer 3 (blend)   |          .
 *            +---------------------+--------------+----------------+          .
 *            |           layer 2(blend)           |                .          .
 *            +---------------------+--------------+                .          .
 *            |        layer 1      |              .                .          .
 * +----------+---------------------+--------------+----------------+----------+
 * |empty-hole|                     |  blend-hole  |tough-blend-hole|empty-hole|
 * +----------+          ^          +--------------+----------------+----------+
 *                       |
 *               multi-source-blit
 *
 * 1. empty-hole
 *    area does not covered by any layer.
 *    Need clear.
 *
 * 2. blend-hole
 *    area covered by some layer(s), but the bottom layer is with blending.
 *    Optimized clear by blit area.
 *
 * 3. tough-blend-hole
 *    Same as blend-hole, but the bottom layer is a tough layer and with
 *    blending, can not optimize, with exceptions below.
 *
 * 4. multi-source-blit area
 *    In example above, that area can use multi-source blit for layer 1 and
 *    layer 2.
 *
 * 5. Will not split-area for 'top small tough layer', hence its worm-hole will
 *    be empty-hole.
 *
 *
 * tough-blend-hole exception:
 *                                                      +------------+
 *                                                      |   layer 4  |
 *                                                      +------------+---------+
 *                                                      | tough layer 3 (blend)|
 *                         +----------------------------+------------+---------+
 *                         |   tough layer 2 (blend)    |            .         .
 *            +------------+------+---------------------+            .         .
 *            |      layer 1      |                     .            .         .
 * +----------+-------------------+---------------------xxxxxxxxxxxxxxxxxxxxxxx+
 * |empty-hole|                   |  tough-blend-hole 1 x tough-blend-hole(s) 2|
 * +----------+                   +---------------------xxxxxxxxxxxxxxxxxxxxxxx+
 *
 * Area of tough-blend-hole(s) 2 is equivalent to are of layer 3, can disable
 * blending of the tough layer for optimization.
 */
struct blitter_display
{
    /*
     * Boolean, Reset after validate display.
     * Set after once blit_display.
     * This is because after validate-display, 'useBlt' may be changed, still.
     */
    uint32_t revalidated;

    __hwc2_list_t outputList;   /* Previous output buffers. */
    struct output *output;      /* Current output. */
    buffer_handle_t bufferRef;  /* Previous reference buffer. */

    /* Blitter layers array, 64 max. */
    uint32_t numBltLayers;
    struct __hwc2_layer *bltLayers[64];

    /* Tough layer mask. */
    uint64_t toughMask;

    /* Area lists. */
    __hwc2_list_t copyArea;     /* Need to copy from previous. */

    __hwc2_list_t wormHole;     /* Empty-hole and tough-blend-hole. */
    __hwc2_list_t compArea;     /* Composition area. */

    /* wormHole and compArea overlapped with damage. */
    __hwc2_list_t wormHoleDamaged;
    __hwc2_list_t compAreaDamaged;

    __hwc2_list_t freeArea;     /* Area repo. */

    /* Surface information >>>. */
    gcoSURF surface;
    struct surface sur;

    gcoSURF surfaceRef;
    /* <<< End surface information. */

    gcsRECT fullScreen;

    gceSURF_FORMAT format;
};

struct blitter_layer
{
    int tough;  /* boolean. */
    int topTiny;/* boolean, a top-tiny layer, will not split. */
    int scaled; /* boolean, if scaling. */
    int filter; /* boolean, use filter-blit. */

    /* Alpha blending states >>> */
    uint32_t blendFormula;  /* Layer blend formula. */
    float globalAlpha;      /* Layer plane alpha. */
    int opaque;             /* Force no blend for tough-blend-hole opt. */
    /* <<< End alpha blending states. */

    /* Geometry states below >>> */
    /* Source crop in dest coordinate system. */
    hwc_frect_t sourceCropD;
    /* Dest rect in source coordinate system. */
    hwc_rect_t displayFrameS;

    /* scale factors, dest/srouce. */
    float hScale;
    float vScale;

    /* transform stats. */
    int hMirror;
    int vMirror;
    gceSURF_ROTATION rotation;
    /* <<< End geometry states. */

    /* visible region overlapped with damage. */
    __hwc2_list_t visibleDamaged;

    /* Source information below >>> */
    gcoSURF surface;
    struct surface sur;

    uint32_t color32;

    int yuvClass;   /* boolean, is yuv class format. */
    /* <<< End source information. */

    __hwc2_list_t link;
};

/*
 * Blitter functions for hwc2_common.
 * Do not need to use these functions in module entry.
 */
int blitter_init(struct __hwc2_device *dev);
void blitter_close(struct __hwc2_device *dev);

int blitter_init_display(struct __hwc2_display *dpy);
void blitter_close_display(struct __hwc2_display *dpy);

int blitter_init_layer(struct __hwc2_display *dpy,
            struct __hwc2_layer *layer);
void blitter_fini_layer(struct __hwc2_display *dpy,
            struct __hwc2_layer *layer);

#endif
