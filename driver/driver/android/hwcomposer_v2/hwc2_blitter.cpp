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


#include "hwc2_common.h"
#include "hwc2_util.h"

#include <inttypes.h>
#include <math.h>
#include <pthread.h>

#include <gc_gralloc_priv.h>
#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_raster.h>
#include <gc_hal_driver.h>

#include <sync/sync.h>
#include <cutils/properties.h>

/* tiny layer, if width < TINY_WIDTH or height < TINY_HEIGHT */
#define TINY_WIDTH          32
#define TINY_HEIGHT         32

/* tiny layer, if rect size < TINY_RECT */
#define TINY_RECT_WIDTH     64
#define TINY_RECT_HEIGHT    64

/* Buffer alignment required for multi-source blit v1. */
#define YUV_BUFFER_ALIGN    64
#define RGB_BUFFER_ALIGN    64


static int query_hardware_types(blitter_device_t *blt)
{
    gceSTATUS status;

    gcsHAL_INTERFACE iface = {
        .ignoreTLS = gcvFALSE,
        .command   = gcvHAL_CHIP_INFO,
    };

    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    for (int i = 0; i < iface.u.ChipInfo.count; i++) {
        switch (iface.u.ChipInfo.types[i]) {
            case gcvHARDWARE_3D2D:
                blt->blitterHwType = blt->hwType3D = gcvHARDWARE_3D2D;
                break;
            case gcvHARDWARE_2D:
                blt->blitterHwType = gcvHARDWARE_2D;
                break;
            case gcvHARDWARE_3D:
                blt->hwType3D = gcvHARDWARE_3D;
                break;
            default:
                break;
        }
    }

    if (unlikely(blt->blitterHwType == gcvHARDWARE_INVALID)) {
        ALOGE("No blitter hardware!");
        return -EINVAL;
    }

    __hwc2_trace(2, "blitter-hardware=%d 3D-hardware=%d",
        blt->blitterHwType, blt->hwType3D);
    return 0;

OnError:
    /* Can not recovery. */
    LOG_ALWAYS_FATAL("Query Chip info failed");
    return -EINVAL;
}

static inline void enter_blitter_hardware(__hwc2_device_t *dev)
{
    blitter_device_t *blt = &dev->blt;

    gcoHAL_GetHardwareType(gcvNULL, &blt->currentHwType);

    if (blt->currentHwType != blt->blitterHwType)
        gcoHAL_SetHardwareType(gcvNULL, blt->blitterHwType);
}

static inline void leave_blitter_hardware(__hwc2_device_t *dev)
{
    blitter_device_t *blt = &dev->blt;

    if (blt->currentHwType != blt->blitterHwType)
        gcoHAL_SetHardwareType(gcvNULL, blt->currentHwType);
}

/*
 * has_hw_feature - A shortcut to HAL function.
 * Convert gceSTATUS to boolean.
 * Returns boolean.
 */
static inline int has_hw_feature(gceFEATURE feature)
{
    return (gcoHAL_IsFeatureAvailable(gcvNULL, feature) == gcvSTATUS_TRUE);
}

static int init_blitter_feature(blitter_device_t *blt)
{
    int unifiedDestRect = 0;
    gctUINT superTileMode = 0;

    if (blt->hwType3D != gcvHARDWARE_INVALID) {
        gcoHAL_SetHardwareType(gcvNULL, blt->hwType3D);
        gcmVERIFY_OK(gcoHAL_QuerySuperTileMode(&superTileMode));
        gcoHAL_SetHardwareType(gcvNULL, blt->blitterHwType);
    }

    if (gcmIS_ERROR(gcoHAL_Get2DEngine(gcvNULL, &blt->engine))) {
        ALOGE("%s(%d): No 2D engine", __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (unlikely(!has_hw_feature(gcvFEATURE_2DPE20))) {
        ALOGE("%s(%d): Requires PE20, but not supported ",
            __FUNCTION__, __LINE__);
        return -EINVAL;
    }

    if (has_hw_feature(gcvFEATURE_2D_ONE_PASS_FILTER))
        blt->onePassFilter = 1;

    if (has_hw_feature(gcvFEATURE_2D_TILING))
        blt->tiledInput = 1;

    if (has_hw_feature(gcvFEATURE_2D_COMPRESSION)) {
        blt->compression2D = 1;
        /* Use source rotation for multi-source blit. */
        blt->multiBltSourceRotation = 1;
    }

    if (has_hw_feature(gcvFEATURE_2D_MIRROR_EXTENSION))
        blt->mirrorExt = 1;

    if (has_hw_feature(gcvFEATURE_2D_YUV_SEPARATE_STRIDE))
        blt->yuvSeparateStride = 1;

    if (has_hw_feature(gcvFEATURE_2D_MULTI_SOURCE_BLT)) {
        blt->multiBlt          = 1;
        blt->multiBltMaxSource = 4;
    }

    if (has_hw_feature(gcvFEATURE_2D_MULTI_SOURCE_BLT_EX)) {
        blt->multiBltEx            = 1;
        blt->multiBltMaxSource     = 8;
        blt->multiBltSourceMirror  = 1;
        blt->multiBltByteAlignment = 1;
        blt->multiBltMaxYuvSource  = 1;
    }

    if (has_hw_feature(gcvFEATURE_2D_MULTI_SOURCE_BLT_EX2)) {
        blt->multiBlt             = 1;
        blt->multiBlt2            = 1;
        blt->multiBltMaxYuvSource = 8;
        blt->multiBltSourceOffset = 1;
    }

    if (has_hw_feature(gcvFEATURE_2D_ALL_QUAD))
        blt->multiBltSourceRotation = 1;
    else
        blt->multiBltOddCoord = 1;

    if (has_hw_feature(gcvFEATURE_2D_MULTI_SRC_BLT_TO_UNIFIED_DST_RECT)) {
        blt->multiBlt             = 1;
        blt->multiBlt1_5          = 1;
        blt->multiBltMaxYuvSource = 8;
        blt->multiBltSourceOffset = 1;

        unifiedDestRect   = 1;
    }

    /* Set 2D parameters. */
    gco2D_SetStateU32(blt->engine, gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE, 1);
    gco2D_SetStateU32(blt->engine, gcv2D_STATE_XRGB_ENABLE, 1);

    if (unifiedDestRect && !blt->multiBlt2)
        gco2D_SetStateU32(blt->engine, gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT, 1);

    gco2D_SetStateU32(blt->engine, gcv2D_STATE_SUPER_TILE_VERSION, (superTileMode + 1));

    /* Use filter scaling when one-pass-filter-blit exists. */
    blt->filterScale = blt->onePassFilter;

    ALOGI("hwcomposer2 blitter features:\n"
          "One Pass Filter:    %d\n"
          "Tile Input:         %d\n"
          "Output Compression: %d\n"
          "Mirror Ext:         %d\n"
          "YUV Strides:        %d\n"
          "Multi-source Blit:  %d\n",
          blt->onePassFilter,
          blt->tiledInput,
          blt->compression2D,
          blt->mirrorExt,
          blt->yuvSeparateStride,
          blt->multiBlt);

    if (blt->multiBlt) {
        ALOGI("  Ex:               %d\n"
              "  V1.5:             %d\n"
              "  V2:               %d\n"
              "  Source Mirror:    %d\n"
              "  Source Rotation:  %d\n"
              "  Byte Alignment:   %d\n"
              "  Source Offsets:   %d\n"
              "  Max Source:       %d\n"
              "  Max YUV Source:   %d\n",
              blt->multiBltEx,
              blt->multiBlt1_5,
              blt->multiBlt2,
              blt->multiBltSourceMirror,
              blt->multiBltSourceRotation,
              blt->multiBltByteAlignment,
              blt->multiBltSourceOffset,
              blt->multiBltMaxSource,
              blt->multiBltMaxYuvSource);
    }

    return 0;
}

static inline void check_property_option(blitter_device_t *blt)
{
    char property[PROPERTY_VALUE_MAX];

    if (property_get("hwc2.filter_scale", property, NULL) > 0)
        blt->filterScale = (property[0] != '0');
}

/* TODO: do in another thread. */
int blitter_init(__hwc2_device_t *dev)
{
    int err;
    blitter_device_t *blt = &dev->blt;
    __hwc2_trace(2, "blt=%p", blt);

    memset(blt, 0, sizeof(blitter_device_t));

    /* Get required hardware type. */
    err = query_hardware_types(blt);

    if (unlikely(err)) {
        ALOGE("%s(%d): Failed", __FUNCTION__, __LINE__);
        return err;
    }

    /* Switch to blitter hardware. */
    enter_blitter_hardware(dev);

    /* Init 2D features. */
    err = init_blitter_feature(blt);

    /* Restore hardware type. */
    leave_blitter_hardware(dev);

    if (unlikely(err)) {
        ALOGE("%s(%d): Failed", __FUNCTION__, __LINE__);
        return err;
    }

    /* Check options. */
    check_property_option(blt);

    return 0;
}

/* TODO: do in another thread. */
void blitter_close(__hwc2_device_t *dev)
{
    blitter_device_t *blt = &dev->blt;
    __hwc2_trace(2, "blt=%p", blt);

    enter_blitter_hardware(dev);

    gco2D_FreeFilterBuffer(blt->engine);
    gcoHAL_Commit(gcvNULL, gcvTRUE);

    leave_blitter_hardware(dev);
}

int32_t blitter_init_display(struct __hwc2_display *dpy)
{
    dpy->blt.numBltLayers = 0;
    __hwc2_list_init(&dpy->blt.copyArea);

    __hwc2_list_init(&dpy->blt.wormHole);
    __hwc2_list_init(&dpy->blt.compArea);

    __hwc2_list_init(&dpy->blt.wormHoleDamaged);
    __hwc2_list_init(&dpy->blt.compAreaDamaged);

    __hwc2_list_init(&dpy->blt.freeArea);

    __hwc2_list_init(&dpy->blt.outputList);

    return 0;
}

void blitter_close_display(struct __hwc2_display *dpy)
{
    __hwc2_list_t *pos, *n;
    __HWC2_DEFINE_LIST(list);

    __hwc2_list_splice_init(&dpy->blt.copyArea, &list);
    __hwc2_list_splice_init(&dpy->blt.wormHole, &list);
    __hwc2_list_splice_init(&dpy->blt.compArea, &list);
    __hwc2_list_splice_init(&dpy->blt.wormHoleDamaged, &list);
    __hwc2_list_splice_init(&dpy->blt.compAreaDamaged, &list);
    __hwc2_list_splice_init(&dpy->blt.freeArea, &list);

    __hwc2_list_for_each_safe(pos, n, &list)
        free(__hwc2_list_entry(pos, struct area, link));
}

int blitter_init_layer(struct __hwc2_display *dpy,
            struct __hwc2_layer *layer)
{
    __hwc2_list_init(&layer->blt.visibleDamaged);
    return 0;
}

void blitter_fini_layer(struct __hwc2_display *dpy,
            struct __hwc2_layer *layer)
{
    __hwc2_list_splice_init(&layer->blt.visibleDamaged, &dpy->blt.freeArea);
}

/******************************************************************************/

/*
 * can_use_blitter - check blitter capability for single layer.
 * Returns boolean.
 */
static int32_t can_use_blitter(__hwc2_device_t *device,
        __hwc2_display_t *dpy, __hwc2_layer_t *layer)
{
    blitter_device_t *blt = &device->blt;
    gc_native_handle_t *handle;
    gcoSURF surface;

    switch (layer->composition) {
        case HWC2_COMPOSITION_DEVICE:
        case HWC2_COMPOSITION_CURSOR:
            /* Break to have more checks below. */
            break;
        case HWC2_COMPOSITION_SOLID_COLOR:
            return 1;
        default:
            __hwc2_trace(2, "layer=%p(%u) CLIENT", layer, layer->id);
            return 0;
    }

    if (unlikely(!layer->buffer)) {
        __hwc2_trace(2, "layer=%p(%u) no buffer", layer, layer->id);
        return 0;
    }

    if (unlikely(private_handle_t::validate(layer->buffer))) {
        __hwc2_trace(2, "layer=%p(%u) invalid handle", layer, layer->id);
        return 0;
    }

    handle = gc_native_handle_get(layer->buffer);

    if (unlikely(!handle->surface)) {
        __hwc2_trace(2, "layer=%p(%u) no surface", layer, layer->id);
        return 0;
    }

    surface = (gcoSURF)handle->surface;

    if ((handle->halFormat == gcvSURF_YV12 ||
            handle->halFormat == gcvSURF_I420) && !blt->yuvSeparateStride) {

        gctINT stride = 0;
        gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, &stride);

        if (((uint32_t)stride) & 0x1F) {
            __hwc2_trace(2, "layer=%p(%u) not aligned", layer, layer->id);
            return 0;
        }
    }

    if (!blt->tiledInput) {
        gceTILING tiling;
        gcoSURF_GetTiling(surface, &tiling);

        if (tiling != gcvLINEAR) {
            __hwc2_trace(2, "layer=%p(%u) tiled", layer, layer->id);
            return 0;
        }
    }

    return 1;
}

int32_t /*hwc2_error_t*/ __hwc2_validate_blitter(
        __hwc2_device_t *device, __hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;
    uint32_t num = 0;

    __hwc2_trace(0, "dpy=%p", dpy);

    /* Reset states. */
    dpy->blt.revalidated = 0;

    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer;
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        if (num >= 64)
            layer->composerSel = COMP_USE_NULL;
        else if (can_use_blitter(device, dpy, layer)) {
            layer->composerSel = COMP_USE_BLIT;
            num++;
        } else
            layer->composerSel = COMP_USE_NULL;
    }

    __hwc2_trace(1, "can_use_blitter num=%u", num);
    return HWC2_ERROR_NONE;
}

/******************************************************************************/


/*
 * translate_frect - Translate android float rect to HAL.
 * 2D HAL requies the same coordinate system for source and dest.
 */
static inline void translate_frect(uint32_t width, uint32_t height,
                        hwc_frect_t rect, int32_t transform,
                        hwc_frect_t *out)
{
    switch (transform) {
        case HWC_TRANSFORM_FLIP_H:
        case HWC_TRANSFORM_FLIP_V:
        case 0:
        default:
            *out = rect;
            break;
        case HWC_TRANSFORM_ROT_270:
        case HWC_TRANSFORM_FLIP_V_ROT_90:
            out->left   = rect.top;
            out->top    = width  - rect.right;
            out->right  = rect.bottom;
            out->bottom = width  - rect.left;
            break;
        case HWC_TRANSFORM_ROT_180:
            out->left   = width  - rect.right;
            out->top    = height - rect.bottom;
            out->right  = width  - rect.left;
            out->bottom = height - rect.top;
            break;
        case HWC_TRANSFORM_ROT_90:
        case HWC_TRANSFORM_FLIP_H_ROT_90:
            out->left   = height - rect.bottom;
            out->top    = rect.left,
            out->right  = height - rect.top;
            out->bottom = rect.right;
            break;
    }
}

/*
 * translate_transform - Translate android transformation to HAL.
 */
static inline void translate_transform(int32_t transform,
                        int *hMirror, int *vMirror,
                        gceSURF_ROTATION *rotation)
{
    switch (transform) {
        case 0:
        default:
            *hMirror  = 0;
            *vMirror  = 0;
            *rotation = gcvSURF_0_DEGREE;
            break;
        case HWC_TRANSFORM_FLIP_H:
            *hMirror  = 1;
            *vMirror  = 0;
            *rotation = gcvSURF_0_DEGREE;
            break;
        case HWC_TRANSFORM_FLIP_V:
            *hMirror  = 0;
            *vMirror  = 1;
            *rotation = gcvSURF_0_DEGREE;
            break;
        case HWC_TRANSFORM_ROT_90:
            *hMirror  = 0;
            *vMirror  = 0;
            *rotation = gcvSURF_270_DEGREE;
            break;
        case HWC_TRANSFORM_ROT_180:
            *hMirror  = 0;
            *vMirror  = 0;
            *rotation = gcvSURF_180_DEGREE;
            break;
        case HWC_TRANSFORM_ROT_270:
            *hMirror  = 0;
            *vMirror  = 0;
            *rotation = gcvSURF_90_DEGREE;
            break;
        case HWC_TRANSFORM_FLIP_H_ROT_90:
            *hMirror  = 0;
            *vMirror  = 1;
            *rotation = gcvSURF_270_DEGREE;
            break;
        case HWC_TRANSFORM_FLIP_V_ROT_90:
            *hMirror  = 0;
            *vMirror  = 1;
            *rotation = gcvSURF_90_DEGREE;
            break;
    }
}

/*
 * has_alpha_channel - Check HAL format, if need per-pixel blending.
 */
static inline int has_alpha_channel(int format)
{
    switch (format) {
        case gcvSURF_R5G6B5:
        default:
            return 0;
        case gcvSURF_A8R8G8B8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_A1R5G5B5:
        case gcvSURF_A1B5G5R5:
        case gcvSURF_B8G8R8A8:
        case gcvSURF_R8G8B8A8:
            return 1;
    }
}

/* Define the table entries. */
#define FORMULA_DISABLED                    0
#define FORMULA_PREMULT                     1
#define FORMULA_PREMULT_PERPIXEL            2
#define FORMULA_PREMULT_PLANE               3
#define FORMULA_COVERAGE                    4

struct blend_formula
{
    gceSURF_PIXEL_ALPHA_MODE         srcAlphaMode;
    gceSURF_PIXEL_ALPHA_MODE         dstAlphaMode;
    gceSURF_GLOBAL_ALPHA_MODE        srcGlobalAlphaMode;
    gceSURF_GLOBAL_ALPHA_MODE        dstGlobalAlphaMode;
    gceSURF_BLEND_FACTOR_MODE        srcFactorMode;
    gceSURF_BLEND_FACTOR_MODE        dstFactorMode;
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  srcPremultSrcAlpha;
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstPremultDstAlpha;
    gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultGlobalMode;
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstDemultDstAlpha;
};

struct blend_formula blend_formula[] =
{
    [FORMULA_DISABLED] = {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ZERO,
        gcvSURF_BLEND_ZERO,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE
    },

    /*
     * Cs' = Cs * Ags
     * As' = As * Ags
     *
     * C  = Cs' + Cd * (1 - As') = Cs * Ags + Cd * (1 - As * Ags)
     * A  = As' + Ad * (1 - As') = As * Ags + Ad * (1 - As * Ags)
     */
    [FORMULA_PREMULT] = {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_SCALE,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_BLEND_INVERSED,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
        gcv2D_COLOR_MULTIPLY_DISABLE
    },

    /*
     * Cs' = Cs
     * As' = As
     *
     * C  = Cs' + Cd * (1 - As') = Cs + Cd * (1 - As)
     * A  = As' + Ad * (1 - As') = As + Ad * (1 - As)
     */
    [FORMULA_PREMULT_PERPIXEL] = {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_BLEND_INVERSED,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE
    },

    /*
     * Cs' = Cs * Ags
     * As' = 1  * Ags
     *
     * C  = Cs' + Cd * (1 - As') = Cs * Ags + Cd * (1 - Ags)
     * A  = As' + Ad * (1 - As') = 1  * Ags + Ad * (1 - Ags)
     */
    [FORMULA_PREMULT_PLANE] = {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_ON,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_BLEND_INVERSED,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
        gcv2D_COLOR_MULTIPLY_DISABLE
    },

    /* TODO: This is not fully correct. */
    [FORMULA_COVERAGE] = {
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_PIXEL_ALPHA_STRAIGHT,
        gcvSURF_GLOBAL_ALPHA_ON,
        gcvSURF_GLOBAL_ALPHA_OFF,
        gcvSURF_BLEND_ONE,
        gcvSURF_BLEND_INVERSED,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_COLOR_MULTIPLY_DISABLE,
        gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR,
        gcv2D_COLOR_MULTIPLY_DISABLE
    },
};

/*
 * refresh_layer_blend - Refresh blend formula for layer.
 * For Premultiplied blending, we use three concrete formula:
 * 1. Only per-pixel blending.
 * 2. Only plane alpha blending.
 * 3. Normal: both per-pixel and plane alpha
 */
static inline void refresh_layer_blend(__hwc2_display_t *dpy,
                            __hwc2_layer_t *layer)
{
    uint32_t formula;

    if (layer->blendMode == HWC2_BLEND_MODE_NONE) {
        formula = FORMULA_DISABLED;
    } else if (unlikely(layer->blendMode == HWC2_BLEND_MODE_COVERAGE)) {
        formula = FORMULA_COVERAGE;
    } else if (layer->blendMode == HWC2_BLEND_MODE_PREMULTIPLIED) {
        if (!layer->buffer || private_handle_t::validate(layer->buffer))
            formula = FORMULA_PREMULT_PLANE;
        else {
            gc_native_handle_t *hnd = gc_native_handle_get(layer->buffer);
            int perpixelAlpha = has_alpha_channel(hnd->halFormat);

            if (perpixelAlpha && (layer->planeAlpha < 1.0))
                formula = FORMULA_PREMULT;
            else if (perpixelAlpha)
                formula = FORMULA_PREMULT_PERPIXEL;
            else
                formula = FORMULA_PREMULT_PLANE;
        }
    } else {
        formula = FORMULA_DISABLED;
    }

    /* update blend formula. */
    layer->blt.blendFormula = formula;
    layer->blt.opaque       = 0;

    /* Covert [0.0, 1.0] to [0, 255]. */
    if (layer->planeAlpha >= 1.0)
        layer->blt.globalAlpha = 255;
    else
        layer->blt.globalAlpha = (uint32_t)(layer->planeAlpha * 256);

    __hwc2_trace(2, "layer=%p(%u) formula=%d planeAlpha=%.4f(%u)",
        layer, layer->id, formula, layer->planeAlpha, layer->blt.globalAlpha);
}

static inline void refresh_blend(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, uint32_t layoutChanged)
{
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];

        /*
         * blendChanged flag will be reset per frame, regardless whether
         * blitter is used. When new blitter layer comes, we need refresh
         * its blend parameters.
         */
        if (layer->blendChanged || layoutChanged)
            refresh_layer_blend(dpy, layer);
    }
}

/*
 * alloc_area - Allocate a new area.
 * with rect and layerMask information.
 */
static struct area *alloc_area(__hwc2_display_t *dpy,
                        const hwc_rect_t *rect, uint64_t layerMask)
{
    struct area *area;
    __hwc2_list_t *list = &dpy->blt.freeArea;

    if (unlikely(__hwc2_list_empty(list))) {
        area = (struct area *)malloc(sizeof(struct area));
        LOG_ALWAYS_FATAL_IF(!area, "Out of memory");
    } else {
        area = __hwc2_list_entry(list->next, struct area, link);
        __hwc2_list_del(&area->link);
    }

    area->rect      = *rect;
    area->layerMask = layerMask;
    return area;
}

/*
 * reclaim_area - Recycle an area struct.
 * Remove from previous list, then move to freeArea list.
 */
static inline void reclaim_area(__hwc2_display_t *dpy,
                        struct area *area)
{
    __hwc2_list_del(&area->link);
    __hwc2_list_add(&area->link, &dpy->blt.freeArea);
}

/*
 * reclaim_area_list - Recycle the whole area list.
 */
static inline void reclaim_area_list(__hwc2_display_t *dpy,
                            __hwc2_list *list)
{
    __hwc2_list_splice_init(list, &dpy->blt.freeArea);
}

/*
 * add_alloc_area - Allocate a new area and add to list.
 */
static inline struct area *add_alloc_area(__hwc2_display_t *dpy,
        __hwc2_list_t *list, const hwc_rect_t *rect, uint64_t layerMask)
{
    struct area *area = alloc_area(dpy, rect, layerMask);
    __hwc2_list_add_tail(&area->link, list);

    return area;
}

static inline void copy_area_list(__hwc2_display_t *dpy,
                        __hwc2_list *list, __hwc2_list *src)
{
    __hwc2_list_t *pos;

    __hwc2_list_for_each(pos, src) {
        struct area* a = __hwc2_list_entry(pos, struct area, link);
        add_alloc_area(dpy, list, &a->rect, a->layerMask);
    }
}

/*
 * split_area_recursive_x - Recursive split area, x major
 */
static void split_area_recursive_x(__hwc2_display_t *dpy,
                __hwc2_list_t *list, hwc_rect_t rect, uint64_t layerMask)
{
    struct area *area;
    __hwc2_list_t *pos;
    hwc_rect_t r0[4];
    hwc_rect_t r1[4];
    uint32_t numIn = 0;
    uint32_t numOut = 0;
    int found = 0;
    hwc_rect_t *r = NULL;

    __hwc2_list_for_each(pos, list) {
        area = __hwc2_list_entry(pos, struct area, link);
        r = &area->rect;

        if ((rect.left < r->right)  && (rect.right  > r->left)  &&
            (rect.top  < r->bottom) && (rect.bottom > r->top)) {
            found = 1;
            break;
        }
    }

    if (unlikely(!found)) {
        return;
    }

    if (r->top < rect.top) {
        /*
         * i----------i  <-- r
         * |    in    |
         * i--+----+--i
         * |  |rect|  |
         * |  |    |  |
         * |  |    |  |
         * +----------+
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = r->top;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = rect.top;
        numIn++;
        /* overlapped top. */
        r->top = rect.top;
    } else if (rect.top < r->top) {
        /*
         *    o----o
         *    |out |
         * +--o -- o--+  <-- r
         * |  |rect|  |
         * |  |    |  |
         * |  |    |  |
         * +----------+
         */
        r1[numOut].left   = rect.left;
        r1[numOut].top    = rect.top;
        r1[numOut].right  = rect.right;
        r1[numOut].bottom = r->top;
        numOut++;
    }

    if (rect.bottom < r->bottom) {
        /*
         * +----------+  <-- r
         * |  |    |  |
         * |  |    |  |
         * |  |rect|  |
         * i--+----+--i
         * |    in    |
         * i----------i
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = rect.bottom;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped bottom. */
        r->bottom = rect.bottom;
    } else if (r->bottom < rect.bottom) {
        /*
         * +----------+  <-- r
         * |  |    |  |
         * |  |    |  |
         * |  |rect|  |
         * +--o -- o--+
         *    |out |
         *    o----o
         */
        r1[numOut].left   = rect.left;
        r1[numOut].top    = r->bottom;
        r1[numOut].right  = rect.right;
        r1[numOut].bottom = rect.bottom;
        numOut++;
    }

    /* Use overlapped r->top and r->bottom below. */
    if (r->left < rect.left) {
        /* +-------------+  <-- r
         * i---+----- - -|
         * |in | rect    |
         * i---+----- - -|
         * +-------------+
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = r->top;
        r0[numIn].right  = rect.left;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped left. */
        r->left = rect.left;
    } else if (rect.left < r->left) {
        /*     +-----------+  <-- r
         * o---o------- - -|
         * |out    rect    |
         * o---o------- - -|
         *     +-----------+
         */
        r1[numOut].left   = rect.left;
        r1[numOut].top    = r->top;
        r1[numOut].right  = r->left;
        r1[numOut].bottom = r->bottom;
        numOut++;
    }

    if (rect.right < r->right) {
        /* +-------------+  <-- r
         * |- - -----+---i
         * |    rect |in |
         * |- - -----+---i
         * +-------------+
         */
        r0[numIn].left   = rect.right;
        r0[numIn].top    = r->top;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped right. */
        r->right = rect.right;
    } else if (r->right < rect.right) {
        /* +-----------+  <-- r
         * | - - ------o---o
         * |     rect   out|
         * | - - ------o---o
         * +-----------+
         */
        r1[numOut].left   = r->right;
        r1[numOut].top    = r->top;
        r1[numOut].right  = rect.right;
        r1[numOut].bottom = r->bottom;
        numOut++;
    }

    /* Process rects outside of area. */
    for (uint32_t i = 0; i < numOut; i++) {
        split_area_recursive_x(dpy, list, r1[i], layerMask);
    }

    if (numIn > 0) {
        /* Save rects inside area but not overlapped. */
        for (uint32_t i = 0; i < numIn; i++) {
            struct area *a;
            a = alloc_area(dpy, &r0[i], area->layerMask);
            __hwc2_list_add_tail(&a->link, list);
        }
    }

    /* The area is owned by the new owner as well. */
    area->layerMask |= layerMask;
}

/*
 * split_area_recursive_y - Recursive split area, y major
 * It's similar to split_area_recursive_y, but prefer vertical split, benifit
 * for rotated composition.
 */
static void split_area_recursive_y(__hwc2_display_t *dpy,
                __hwc2_list_t *list, hwc_rect_t rect, uint64_t layerMask)
{
    struct area *area;
    __hwc2_list_t *pos;
    hwc_rect_t r0[4];
    hwc_rect_t r1[4];
    uint32_t numIn = 0;
    uint32_t numOut = 0;
    int found = 0;
    hwc_rect_t *r = NULL;

    __hwc2_list_for_each(pos, list) {
        area = __hwc2_list_entry(pos, struct area, link);
        r = &area->rect;

        if ((rect.left < r->right)  && (rect.right  > r->left)  &&
                (rect.top  < r->bottom) && (rect.bottom > r->top)) {
            found = 1;
            break;
        }
    }

    if (unlikely(!found)) {
        return;
    }

    if (r->left < rect.left) {
        /* i---i---------+  <-- r
         * |   +----- - -|
         * |in | rect    |
         * |   +----- - -|
         * i---i---------+
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = r->top;
        r0[numIn].right  = rect.left;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped left. */
        r->left = rect.left;
    } else if (rect.left < r->left) {
        /*     +-----------+  <-- r
         * o---o------- - -|
         * |out    rect    |
         * o---o------- - -|
         *     +-----------+
         */
        r1[numOut].left   = rect.left;
        r1[numOut].top    = rect.top;
        r1[numOut].right  = r->left;
        r1[numOut].bottom = rect.bottom;
        numOut++;
    }

    if (rect.right < r->right) {
        /* +---------i---i  <-- r
         * |- - -----+   |
         * |    rect |in |
         * |- - -----+   |
         * +---------i---i
         */
        r0[numIn].left   = rect.right;
        r0[numIn].top    = r->top;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped right. */
        r->right = rect.right;
    } else if (r->right < rect.right) {
        /* +-----------+  <-- r
         * | - - ------o---o
         * |     rect   out|
         * | - - ------o---o
         * +-----------+
         */
        r1[numOut].left   = r->right;
        r1[numOut].top    = rect.top;
        r1[numOut].right  = rect.right;
        r1[numOut].bottom = rect.bottom;
        numOut++;
    }

    /* Use overlapped r->left and r->right below. */
    if (r->top < rect.top) {
        /*
         * +--i----i--+  <-- r
         * |  i in i  |
         * |  +----+  |
         * |  |rect|  |
         * |  |    |  |
         * |  |    |  |
         * +----------+
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = r->top;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = rect.top;
        numIn++;
        /* overlapped top. */
        r->top = rect.top;
    } else if (rect.top < r->top) {
        /*
         *    o----o
         *    |out |
         * +--o -- o--+  <-- r
         * |  |rect|  |
         * |  |    |  |
         * |  |    |  |
         * +----------+
         */
        r1[numOut].left   = r->left;
        r1[numOut].top    = rect.top;
        r1[numOut].right  = r->right;
        r1[numOut].bottom = r->top;
        numOut++;
    }

    if (rect.bottom < r->bottom) {
        /*
         * +----------+  <-- r
         * |  |    |  |
         * |  |    |  |
         * |  |rect|  |
         * |  +----+  |
         * |  i in i  |
         * +--i----i--+
         */
        r0[numIn].left   = r->left;
        r0[numIn].top    = rect.bottom;
        r0[numIn].right  = r->right;
        r0[numIn].bottom = r->bottom;
        numIn++;
        /* overlapped bottom. */
        r->bottom = rect.bottom;
    } else if (r->bottom < rect.bottom) {
        /*
         * +----------+  <-- r
         * |  |    |  |
         * |  |    |  |
         * |  |rect|  |
         * +--o -- o--+
         *    |out |
         *    o----o
         */
        r1[numOut].left   = r->left;
        r1[numOut].top    = r->bottom;
        r1[numOut].right  = r->right;
        r1[numOut].bottom = rect.bottom;
        numOut++;
    }

    /* Process rects outside of area. */
    for (uint32_t i = 0; i < numOut; i++) {
        split_area_recursive_y(dpy, list, r1[i], layerMask);
    }

    if (numIn > 0) {
        /* Save rects inside area but not overlapped. */
        for (uint32_t i = 0; i < numIn; i++) {
            struct area *a;
            a = alloc_area(dpy, &r0[i], area->layerMask);
            __hwc2_list_add_tail(&a->link, list);
        }
    }

    /* The area is owned by the new owner as well. */
    area->layerMask |= layerMask;
}

static inline uint32_t is_empty_rect(hwc_rect_t r)
{
    return r.left >= r.right ||
        r.top >= r.bottom;
}

static void split_area(__hwc2_display_t *dpy, int ymajor,
                __hwc2_list_t *list, hwc_rect_t rect, uint64_t layerMask)
{
    /* skip empty rect. */
    if (unlikely(is_empty_rect(rect)))
        return;

    if (unlikely(ymajor)) {
        split_area_recursive_y(dpy, list, rect, layerMask);
    } else {
        split_area_recursive_x(dpy, list, rect, layerMask);
    }
}

static int inline merge_area_once(__hwc2_display_t *dpy, __hwc2_list_t *list)
{
    int merged = 0;
    __hwc2_list_t *pos0, *pos1, *n;
    struct area *a0, *a1;

    __hwc2_list_for_each(pos0, list) {
        a0 = __hwc2_list_entry(pos0, struct area, link);

        for (pos1 = pos0->next, n = pos1->next; pos1 != list;
                pos1 = n, n = n->next) {
            a1 = __hwc2_list_entry(pos1, struct area, link);

            if (likely(a0->layerMask != a1->layerMask))
                continue;

            if ((a0->rect.top    == a1->rect.top)  &&
                (a0->rect.bottom == a1->rect.bottom)) {
                if (a0->rect.left == a1->rect.right) {
                    a0->rect.left = a1->rect.left;
                    reclaim_area(dpy, a1);
                    merged++;
                } else if (a0->rect.right == a1->rect.left) {
                    a0->rect.right = a1->rect.right;
                    reclaim_area(dpy, a1);
                    merged++;
                }
            } else if ((a0->rect.left == a1->rect.left) &&
                        (a0->rect.right == a1->rect.right)) {
                if (a0->rect.top == a1->rect.bottom) {
                    a0->rect.top = a1->rect.top;
                    reclaim_area(dpy, a1);
                    merged++;
                } else if (a0->rect.bottom == a1->rect.top) {
                    a0->rect.bottom = a1->rect.bottom;
                    reclaim_area(dpy, a1);
                    merged++;
                }
            }
        }
    }

    return merged;
}

/*
 * merge_area - merge adjacent area(s) which have the same layerMask.
 */
static inline void merge_area(__hwc2_display_t *dpy, __hwc2_list_t *list)
{
    int merged;

    do {
        merged = merge_area_once(dpy, list);
    } while (merged != 0);
}

#if defined(__HWC2_TRACE)
static const char * bits_to_string(uint64_t mask)
{
    static char str[512];
    static size_t pos;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char *s;
    size_t len = 0;

    pthread_mutex_lock(&mutex);
    if (pos + 256 > sizeof(str))
        pos = 0; /* rewind. */

    s = str + pos;

    for (uint32_t i = 0; i < 64; i++) {
        if (mask & (1ull << i)) {
            len += sprintf(s + len, "%u,", i);
        }
    }

    if (len)
        s[len-1] = '\0';
    else
        s[0] = '\0';
    pos += len + 1;
    pthread_mutex_unlock(&mutex);

    return s;
}

static void trace_area_list(__hwc2_list_t *list)
{
    __hwc2_list_t *pos;
    if (likely(!__hwc2_traceEnabled))
        return;

    if (__hwc2_list_empty(list)) {
        __hwc2_trace_string("  (empty)");
        return;
    }

    __hwc2_list_for_each(pos, list) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);

        __hwc2_trace_string("  [%d,%d,%d,%d] mask=0x%" PRIx64": %s",
                area->rect.left, area->rect.top,
                area->rect.right, area->rect.bottom,
                area->layerMask, bits_to_string(area->layerMask));
    }
}

static void trace_area_list_no_mask(__hwc2_list_t *list)
{
    __hwc2_list_t *pos;
    if (likely(!__hwc2_traceEnabled))
        return;

    if (__hwc2_list_empty(list)) {
        __hwc2_trace_string("  (empty)");
        return;
    }

    __hwc2_list_for_each(pos, list) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);

        __hwc2_trace_string("  [%d,%d,%d,%d]",
                area->rect.left, area->rect.top,
                area->rect.right, area->rect.bottom);
    }
}

#else
#  define trace_area_list(...)          do {} while (0)
#  define trace_area_list_no_mask(...)  do {} while (0)
#endif

/*
 * refresh_layout - refresh bltter_display::bltLayers array.
 *
 * Geometry changed when the array content changed or array size changed.
 * Returns number of layers to use blitter composition.
 *
 * returns 1 if layout changed, 0 for no.
 */
static inline uint32_t refresh_layout(__hwc2_device_t *dev,
                            __hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;
    uint32_t num = 0;
    uint32_t layoutChanged = 0;

    /* Refresh blitter layer array. */
    __hwc2_list_for_each(pos, &dpy->layers) {
        __hwc2_layer_t *layer;
        layer = __hwc2_list_entry(pos, __hwc2_layer_t, link);

        if (layer->composerSel == COMP_USE_BLIT &&
                layer->composition != HWC2_COMPOSITION_CLIENT) {
            if (dpy->blt.bltLayers[num] != layer) {
                dpy->blt.bltLayers[num] = layer;
                /* layer array contents changed. */
                layoutChanged = 1;
            }
            num++;
        }
    }

    if (num != dpy->blt.numBltLayers) {
        dpy->blt.numBltLayers = num;
        /* layer array size changed. */
        layoutChanged = 1;
    }

    __hwc2_trace(2, "layoutChanged=%d numBltLayers=%d", layoutChanged, num);

    return layoutChanged;
}

/*
 * refresh_layer_geometry - refresh layer coordinates
 * Set flags and calculate factors.
 */
static inline void refresh_layer_geometry(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                        uint32_t id)
{
    gc_native_handle_t *handle;
    uint32_t width;
    uint32_t height;

    hwc_frect_t *s  = &layer->sourceCrop;
    hwc_frect_t *ss = &layer->blt.sourceCropD;
    hwc_rect_t  *d  = &layer->displayFrame;

    /* Reset flags. */
    layer->blt.tough   = 0;
    layer->blt.topTiny = 0;
    layer->blt.scaled  = 0;
    layer->blt.filter  = 0;

    __hwc2_trace(0, "layer=%p(%u) buffer=%p", layer, layer->id, layer->buffer);

    if (!layer->buffer) {
        __hwc2_trace(1, "displayFrame=[%d,%d,%d,%d] visibleRegion=",
                d->left, d->top, d->right, d->bottom);
        __hwc2_trace_region(layer->visibleRegion);
        return;
    }

    /* Translate source rectangle rectangle to dest coordinate system. */
    switch (layer->transform) {
        case 0:
        case HWC_TRANSFORM_FLIP_H:
        case HWC_TRANSFORM_FLIP_V:
            *ss = *s;
            break;
        default:
            handle = gc_native_handle_get(layer->buffer);

            width  = (uint32_t)handle->width;
            height = (uint32_t)handle->height;

            translate_frect(width, height,
                    layer->sourceCrop, layer->transform, ss);
            break;
    }

    /* Calculate scale factors. */
    layer->blt.hScale = (d->right  - d->left) / (ss->right  - ss->left);
    layer->blt.vScale = (d->bottom - d->top ) / (ss->bottom - ss->top );

    /* Translate to mirror and rotation. */
    translate_transform(layer->transform,
        &layer->blt.hMirror, &layer->blt.vMirror,
        &layer->blt.rotation);

    if ((layer->blt.hScale != 1.0f) || (layer->blt.vScale != 1.0f)) {
        layer->blt.scaled = 1;

        if (dev->blt.filterScale)
            layer->blt.filter = 1;
        else
            /* Do not blit by area for stretch blit. */
            layer->blt.tough = 1;
    }

    __hwc2_trace_string("sourceCrop=[%.1f,%.1f,%.1f,%.1f] ([%.1f,%.1f,%.1f,%.1f])",
            s->left, s->top, s->right, s->bottom,
            ss->left, ss->top, ss->right, ss->bottom);

    __hwc2_trace_string("displayFrame=[%d,%d,%d,%d] visibleRegion=",
            d->left, d->top, d->right, d->bottom);

    __hwc2_trace_region(layer->visibleRegion);

    __hwc2_trace(1, "scale=%.4f,%.4f mirror=%d,%d rotation=%d",
            layer->blt.hScale, layer->blt.vScale,
            layer->blt.hMirror, layer->blt.vMirror, layer->blt.rotation);
}

/*
 * refresh_tough_layer_blend_hole - refresh tough-blend-hole.
 * Try to optimize out.
 */
static inline void refresh_tough_layer_blend_hole(__hwc2_display_t *dpy,
                        __hwc2_layer_t *layer, uint32_t id)
{
    __hwc2_list_t *pos, *n;
    uint64_t mask = 1ull << id;
    __HWC2_DEFINE_LIST(list);
    int optimizeOut = 1;

    __hwc2_list_for_each_safe(pos, n, &dpy->blt.compArea) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);

        if (!(area->layerMask & mask)) {
            /* Not includes the tough layer. */
            continue;
        }

        if (area->layerMask & (mask - 1)) {
            /* Not at bottom. */
            optimizeOut = 0;
            continue;
        }

        /* find a tough-blend-hole. */
        if (area->layerMask == mask) {
            __hwc2_list_move_tail(&area->link, &list);
        } else {
            struct area *a;
            a = alloc_area(dpy, &area->rect, area->layerMask);
            __hwc2_list_add_tail(&a->link, &list);
        }
    }

    if (optimizeOut) {
        /* Disable blending for such tough-blend-hole. */
        layer->blt.opaque = 1;
        /* Drop the arars. */
        __hwc2_list_splice_tail(&list, &dpy->blt.freeArea);
    } else {
        /* Add to worm hole list. */
        __hwc2_list_splice_tail(&list, &dpy->blt.wormHole);
    }
}

/*
 * refresh_worm_hole - Refresh worm-holes.
 * Worm-holes includes:
 * 1. empty-hole
 * 2. tough-blend-hole which can not be optimized out.
 */
static inline void refresh_worm_hole(__hwc2_display_t *dpy)
{
    __hwc2_list_t *pos, *n;

    reclaim_area_list(dpy, &dpy->blt.wormHole);

    /* check empty-hole. */
    __hwc2_list_for_each_safe(pos, n, &dpy->blt.compArea) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);

        if (!area->layerMask) {
            /* empty-hole, move to worm hole list. */
            __hwc2_list_move_tail(&area->link, &dpy->blt.wormHole);
        }
    }

    if (!dpy->blt.toughMask)
        return;

    /* check optimization for tough blend layer. */
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];

        if (layer->blt.tough && !layer->blt.topTiny &&
            layer->blt.blendFormula != FORMULA_DISABLED) {
            /* A blend tough layer, try to check blend hole. */
            refresh_tough_layer_blend_hole(dpy, layer, i);
        }
    }
}

/*
 * refresh_geometry - Validate display geometry changes.
 */
static inline void refresh_geometry(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, uint32_t layoutChanged)
{
    __hwc2_layer_t *layer;
    uint32_t numTopTiny = 0;
    int geometryChanged = layoutChanged;

    __hwc2_trace(0, "");

    /* Refresh coordinates. */
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        layer = dpy->blt.bltLayers[i];

        /*
         * geometryChanged flag will be reset per frame, regardless whether
         * blitter is used. When new blitter layer comes, we need refresh
         * its geometry parameters.
         */
        if (layer->geometryChanged || layoutChanged) {
            refresh_layer_geometry(dev, dpy, layer, i);
            geometryChanged = 1;
        }
    }

    if (!geometryChanged) {
        __hwc2_trace(1, "Geometry not changed");
        return;
    }

    /* Reset tough layer mask. */
    dpy->blt.toughMask = 0;

    /* check tough layers. */
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        layer = dpy->blt.bltLayers[i];
        if (layer->blt.tough)
            dpy->blt.toughMask |= (1ull << i);
    }

    /*
     * check another tough layer type: tiny layer on top.
     * when @i underflow, its value will be very large, which is GT 64.
     */
    for (uint32_t i = dpy->blt.numBltLayers - 1; i <= 64; i--) {
        layer = dpy->blt.bltLayers[i];
        hwc_rect_t *d = &layer->displayFrame;

        int dw = d->right  - d->left;
        int dh = d->bottom - d->top;

        if (dw < TINY_WIDTH || dh < TINY_HEIGHT ||
                (dw < TINY_RECT_WIDTH && dh < TINY_RECT_HEIGHT)) {
            /* Top-tiny, tough layer. */
            layer->blt.topTiny = 1;
            layer->blt.tough = 1;

            dpy->blt.toughMask |= (1ull << i);
            numTopTiny++;
        } else
            break;
    }

    /* Reclaim geometry area lists. */
    reclaim_area_list(dpy, &dpy->blt.compArea);
    reclaim_area_list(dpy, &dpy->blt.wormHole);

    hwc_rect_t screen = {0, 0, (int)dpy->width, (int)dpy->height};
    add_alloc_area(dpy, &dpy->blt.compArea, &screen, 0ull);

    int ymajor = 0;
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        layer = dpy->blt.bltLayers[i];

        if (layer->transform & HWC_TRANSFORM_ROT_90) {
            ymajor = 1;
            break;
        }
    }

    /* split area, skip top tiny layers. */
    for (uint32_t i = 0; i < dpy->blt.numBltLayers - numTopTiny; i++) {
        layer = dpy->blt.bltLayers[i];

        for (size_t j = 0; j < layer->visibleRegion.numRects; j++) {
            hwc_rect_t rect = layer->visibleRegion.rects[j];
            split_area(dpy, ymajor, &dpy->blt.compArea, rect, 1ull << i);
        }
    }

    /* Try to merge adjacent areas. */
    merge_area(dpy, &dpy->blt.compArea);

    /* Detect worm-holes. */
    refresh_worm_hole(dpy);

    __hwc2_trace_string("composition area:");
    trace_area_list(&dpy->blt.compArea);

    __hwc2_trace_string("worm-hole area:");
    trace_area_list_no_mask(&dpy->blt.wormHole);

    __hwc2_trace(1, "done");
}



static inline void update_ref_buffer(__hwc2_display_t *dpy,
                        buffer_handle_t buffer)
{
    struct output *output = dpy->blt.output;

    if (output && buffer)
        dpy->blt.bufferRef = output->buffer;
    else
        dpy->blt.bufferRef = NULL;
}

static inline void reset_output_list(__hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;
    struct output *output;

    __hwc2_list_for_each(pos, &dpy->blt.outputList) {
        output = __hwc2_list_entry(pos, struct output, link);

        output->buffer = NULL;
        /* Set invalid age. */
        output->age = 0;

        reclaim_area_list(dpy, &output->damage);
    }
}

/*
 * find_output - find a proper output struct, with 'buffer'.
 */
static inline struct output *find_output(__hwc2_display_t *dpy,
                                    buffer_handle_t buffer)
{
    __hwc2_list_t *pos;
    struct output *output;

    /* Find output with the same buffer. */
    __hwc2_list_for_each(pos, &dpy->blt.outputList) {
        output = __hwc2_list_entry(pos, struct output, link);
        if (output->buffer == buffer) {
            __hwc2_trace(2, "output=%p age=%u", output, output->age);
            return output;
        }
    }

    /* Find a empty slot. */
    __hwc2_list_for_each(pos, &dpy->blt.outputList) {
        output = __hwc2_list_entry(pos, struct output, link);
        if (!output->buffer) {
            output->buffer = buffer;
            output->age    = 0;
            __hwc2_trace(2, "output=%p", output);
            return output;
        }
    }

    /* Allocate a new output struct. */
    output = (struct output *)malloc(sizeof(struct output));
    LOG_ALWAYS_FATAL_IF(!output, "Out of memory");

    output->buffer = buffer;
    output->age    = 0;
    __hwc2_list_init(&output->damage);
    __hwc2_list_add(&output->link, &dpy->blt.outputList);

    __hwc2_trace(2, "alloc output=%p", output);
    return output;
}

static inline void merge_visible_region_damage(__hwc2_display_t *dpy,
                        __hwc2_list_t *damage, hwc_region_t *region)
{
    /* For visible region, numRects=0 means no rect. */
    for (uint32_t i = 0; i < region->numRects; i++)
        split_area(dpy, 0, damage, region->rects[i], 1ull);
}

static inline void make_bounding_box(hwc_rect_t *box, hwc_rect_t rect)
{
    if (rect.left < box->left) box->left = rect.left;
    if (rect.top  < box->top ) box->top  = rect.top;
    if (rect.right  > box->right ) box->right  = rect.right;
    if (rect.bottom > box->bottom) box->bottom = rect.bottom;
}

static inline void merge_visible_bounding_damage(__hwc2_display_t *dpy,
                        __hwc2_list_t *damage, hwc_region_t *region0,
                        hwc_region_t *region1)
{
    hwc_rect_t box = {(int)dpy->width, (int)dpy->height, 0, 0};

    /* For visible region, numRects=0 means no rect. */
    for (uint32_t i = 0; i < region0->numRects; i++)
        make_bounding_box(&box, region0->rects[i]);

    for (uint32_t i = 0; i < region1->numRects; i++)
        make_bounding_box(&box, region1->rects[i]);

    split_area(dpy, 0, damage, box, 1ull);
}

static inline void merge_layer_damage_region(__hwc2_display_t *dpy,
                        __hwc2_list_t *damage, __hwc2_layer_t *layer)
{
    if (layer->geometryChanged || layer->blendChanged || layer->colorChanged) {
        if (likely(!layer->blt.topTiny)) {
            /*
             * Geometry or content changed, treat all visibleRegion as damage
             * region. And may include previous visibleRegion as well.
             */
            if (layer->visibleRegionChanged)
                merge_visible_region_damage(dpy, damage, &layer->visibleRegionPrev);

            merge_visible_region_damage(dpy, damage, &layer->visibleRegion);
        } else
            merge_visible_bounding_damage(dpy, damage,
                    &layer->visibleRegionPrev, &layer->visibleRegion);
    } else {
        /* Use surface damage as damage area. */
        if (!__hwc2_region_is_empty(layer->surfaceDamage))
            /* TODO: use surface damage. */
            merge_visible_region_damage(dpy, damage, &layer->visibleRegion);
    }
}

static inline void refresh_damage_region(__hwc2_display_t *dpy,
                        struct output *output, uint32_t layoutChanged)
{
    __hwc2_list_t *damage = &output->damage;
    __hwc2_list_t *pos, *n;

    reclaim_area_list(dpy, damage);
    hwc_rect_t screen = {0, 0, (int)dpy->width, (int)dpy->height};

    if (layoutChanged)
        add_alloc_area(dpy, damage, &screen, 1ull);
    else {
        /*
         * Output buffer should not be null when layout not changed.
         */
        LOG_ALWAYS_FATAL_IF(!dpy->blt.bufferRef, "No previous buffer");
        add_alloc_area(dpy, damage, &screen, 0ull);

        for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
            __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
            merge_layer_damage_region(dpy, damage, layer);
        }

        __hwc2_list_for_each_safe(pos, n, damage) {
            struct area *a = __hwc2_list_entry(pos, struct area, link);
            if (!a->layerMask)
                reclaim_area(dpy, a);
        }

        merge_area(dpy, damage);
    }

    __hwc2_trace_string("damage area:");
    trace_area_list_no_mask(damage);
}

/* append damage region to copyArea. */
static inline void merge_copy_region_one(__hwc2_display_t *dpy,
                        __hwc2_list_t *damage, uint64_t mask)
{
    __hwc2_list_t *pos;

    __hwc2_list_for_each(pos, damage) {
        struct area *a = __hwc2_list_entry(pos, struct area, link);
        split_area(dpy, 0, &dpy->blt.copyArea, a->rect, mask);
    }
}

static void refresh_copy_region(__hwc2_display_t *dpy,
                        struct output *output, uint32_t layoutChanged)
{
    /* Reset copy area. */
    reclaim_area_list(dpy, &dpy->blt.copyArea);

    /*
     * Re-generate copy area list.
     * If layoutChanged, need composite full screen, there's no area to copy
     * from previous buffer.
     * If no output buffer, no need to generate copy area either.
     */
    if (output && !layoutChanged) {
        __hwc2_list_t *pos, *n;
        hwc_rect_t screen = {0, 0, (int)dpy->width, (int)dpy->height};

        if (!output->age)
            add_alloc_area(dpy, &dpy->blt.copyArea, &screen, 1ull);
        else {
            /* Add fullscreen back ground. */
            add_alloc_area(dpy, &dpy->blt.copyArea, &screen, 0ull);

            /* Add area to copy: mask = 1ull. */
            __hwc2_list_for_each(pos, &dpy->blt.outputList) {
                struct output *other;
                other = __hwc2_list_entry(pos, struct output, link);

                if (other->age && other->age < output->age)
                    merge_copy_region_one(dpy, &other->damage, 1ull);
            }
        }

        /* Reduce area to copy: mask = 2ull. */
        merge_copy_region_one(dpy, &output->damage, 2ull);

        /* Only keep mask = 1ull. */
        __hwc2_list_for_each_safe(pos, n, &dpy->blt.copyArea) {
            struct area *a = __hwc2_list_entry(pos, struct area, link);
            if (a->layerMask != 1ull)
                reclaim_area(dpy, a);
        }

        merge_area(dpy, &dpy->blt.copyArea);
    }

    __hwc2_trace_string("copy area:");
    trace_area_list_no_mask(&dpy->blt.copyArea);
}

/*
 * clamp_rect - Clamp rect with clip.
 */
static inline void clamp_rect(hwc_rect_t *rect, hwc_rect_t clip)
{
    if (rect->left < clip.left)
        rect->left = clip.left;

    if (rect->top < clip.top)
        rect->top = clip.top;

    if (rect->right > clip.right)
        rect->right = clip.right;

    if (rect->bottom > clip.bottom)
        rect->bottom = clip.bottom;
}

static inline void clamp_area_with_damage(__hwc2_display_t *dpy,
                        struct area* area, __hwc2_list_t *damage)
{
    __hwc2_list_t *pos;
    hwc_rect_t rect = area->rect;
    uint32_t num = 0;

    __hwc2_list_for_each(pos, damage) {
        struct area *dmg = __hwc2_list_entry(pos, struct area, link);
        hwc_rect_t r = rect;

        clamp_rect(&r, dmg->rect);

        if (is_empty_rect(r))
            continue;

        if (!num)
            /* modify on original one. */
            area->rect = r;
        else
            area = add_alloc_area(dpy, &area->link, &r, area->layerMask);

        num++;
    }

    if (!num)
        reclaim_area(dpy, area);
}

static void clamp_region_with_damage(__hwc2_display_t *dpy,
                    __hwc2_list_t *region, __hwc2_list_t *damage)
{
    __hwc2_list_t *pos, *n;

    __hwc2_list_for_each_safe(pos, n, region) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);
        clamp_area_with_damage(dpy, area, damage);
    }
}

static void refresh_damaged_geometry(__hwc2_display_t *dpy,
                    struct output *output)
{
    uint32_t i;
    uint32_t mask;
    uint32_t toughMask = dpy->blt.toughMask;
    __hwc2_list_t *list;
    __hwc2_list_t *damage = &output->damage;

    /* clamp worm-hole with damage region. */
    list = &dpy->blt.wormHoleDamaged;

    reclaim_area_list(dpy, list);
    copy_area_list(dpy, list, &dpy->blt.wormHole);
    clamp_region_with_damage(dpy, list, damage);

    __hwc2_trace_string("damaged worm-hole area:");
    trace_area_list_no_mask(list);

    /* clamp composition-area with damage region. */
    list = &dpy->blt.compAreaDamaged;

    reclaim_area_list(dpy, list);
    copy_area_list(dpy, list, &dpy->blt.compArea);
    clamp_region_with_damage(dpy, list, damage);

    __hwc2_trace_string("damaged composition area:");
    trace_area_list(list);

    /* clamp layer visibleRegion with damage region, for tough layer. */
    for (i = 0, mask = 1; mask <= toughMask; i++, mask <<= 1) {
        if (!(mask & toughMask))
            continue;

        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
        hwc_region_t *region = &layer->visibleRegion;

        list = &layer->blt.visibleDamaged;

        reclaim_area_list(dpy, list);

        for (uint32_t j = 0; j < region->numRects; j++)
            add_alloc_area(dpy, list, &region->rects[j], 0ull);

        clamp_region_with_damage(dpy, list, damage);

        __hwc2_trace_string("layer=%p(%u) damaged visible region:", layer, layer->id);
        trace_area_list_no_mask(list);
    }
}

static void inline refresh_output_ages(__hwc2_display_t *dpy,
                        struct output *output)
{
    __hwc2_list_t *pos;
    uint32_t age = output->age;

    __hwc2_list_for_each(pos, &dpy->blt.outputList) {
        struct output *buf = __hwc2_list_entry(pos, struct output, link);

        /*
         * Set invalid age for buffer's age is older than current age.
         * That means the buffer is not used last time. For example
         * The asterisk(*) means current back buffer.
         *               buffer-A    buffer-B    buffer-C
         * frame 0 on A:    1 *         0          0
         * frame 1 on B:    2           1 *        0
         * frame 2 on C:    3           2          1 *
         * frame 3 on B!!:  0 (4)       1 *        2
         * frame 4 on A!!:  1 *         2          3
         *
         * Set buffer-A age to '4' is invalid. When frame 3 on B, changes in
         * frame 1 on B is lost. To solve the issue:
         * 1. Here we set buffer-A age to '0' in frame 3.
         * 2. Later frame 4 on A (age is 0) must copy all from buffer-B in
         * function refresh_copy_region() above.
         */
        if (age && buf->age > age)
            buf->age = 0;
        if (buf->age)
            buf->age++;
    }

    output->age = 1;
}

static void refresh_damage(__hwc2_device_t *dev, __hwc2_display_t *dpy,
                    buffer_handle_t buffer, uint32_t layoutChanged)
{
    struct output *output = NULL;

    __hwc2_trace(0, "prev-output=%p numBltLayers=%u buffer=%p",
            dpy->blt.output, dpy->blt.numBltLayers, buffer);

    /* Update previous buffer, the reference. */
    update_ref_buffer(dpy, buffer);

    if (buffer) {
        /* Get output with same buffer, won't fail. */
        output = find_output(dpy, buffer);

        refresh_damage_region(dpy, output, layoutChanged);
        refresh_copy_region(dpy, output, layoutChanged);
        refresh_damaged_geometry(dpy, output);

        refresh_output_ages(dpy, output);
    } else
        reset_output_list(dpy);

    dpy->blt.output = output;
    __hwc2_trace(1, "done");
}


/*
 * revalidate_display - Re-validate display when presentDisplay.
 *
 * Blitter allows the caller to change composition method after validateDisplay.
 * Re-validation is required when presentDisplay.
 *
 * 'Re-validation' will remove layers which disables 'composerSel', and gather
 * more information for layers with 'composerSel'.
 */
static inline void revalidate_display(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, buffer_handle_t buffer)
{
    uint32_t layoutChanged = 0;

    __hwc2_trace(0, "revalidated=%d", dpy->blt.revalidated);

    if (unlikely(!dpy->blt.revalidated)) {
        layoutChanged = refresh_layout(dev, dpy);

        /* refresh alpha blend. */
        refresh_blend(dev, dpy, layoutChanged);

        /* refresh coordinates, etc. */
        refresh_geometry(dev, dpy, layoutChanged);

        /* Updated display flags. */
        dpy->blt.revalidated   = 1;
    }

    /* refresh damage region and copy region. */
    refresh_damage(dev, dpy, buffer, layoutChanged);

    __hwc2_trace(1, "done");
}

static inline void calculate_alignment_mask(__hwc2_device_t *dev,
                        struct surface *sur, uint32_t isTarget,
                        uint32_t *maskX, uint32_t *maskY)
{
    if (dev->blt.multiBltSourceOffset) {
        /* Not required. */
        *maskX = 0;
        *maskY = 0;
        return;
    }

    if (sur->tiling == gcvLINEAR) {
        /* Linear buffer, checks memory & format requirements. */
        uint32_t mx;
        uint32_t fx, fy;

        /* format alignment limitation. */
        fx = sur->formatInfo->blockWidth  - 1;
        fy = sur->formatInfo->blockHeight - 1;

        /* Memory alignment limitation. */
        if (dev->blt.multiBltByteAlignment && !isTarget &&
            sur->formatInfo->fmtClass == gcvFORMAT_CLASS_RGBA)
            mx = 0;
        else if (sur->formatInfo->fmtClass == gcvFORMAT_CLASS_YUV)
            mx = YUV_BUFFER_ALIGN / sur->bytesPerPixel - 1;
        else
            mx = RGB_BUFFER_ALIGN / sur->bytesPerPixel - 1;

        *maskX = mx > fx ? mx : fx;
        *maskY = fy;
    } else {
        /* Tiled buffer, checks memory & tiling requirements. */
        uint32_t mx;
        uint32_t tx, ty;
        uint32_t size;

        /* Tiling alignment limitation. */
        switch (sur->tiling) {
            case gcvTILED:
                tx = 3;
                ty = 3;
                break;
            case gcvMULTI_TILED:
                tx = 3;
                ty = 7;
                break;
            case gcvSUPERTILED:
                tx = 63;
                ty = 63;
                break;
            case gcvMULTI_SUPERTILED:
                tx = 63;
                ty = 127;
                break;
            default:
                ALOGE("Unknown tiling: 0x%X", sur->tiling);
                tx = 7;
                ty = 7;
                break;
        }

        /* 'size' amount of bytes are put contiguously in memory. */
        size = (tx + 1) * (ty + 1) * sur->bytesPerPixel;

        /* Memory alignment limitation. */
        if (sur->formatInfo->fmtClass == gcvFORMAT_CLASS_YUV)
            mx = (YUV_BUFFER_ALIGN > size) ?
                    YUV_BUFFER_ALIGN / size - 1 : 0;
        else
            mx = (RGB_BUFFER_ALIGN > size) ?
                    RGB_BUFFER_ALIGN / size - 1 : 0;

        *maskX = mx > tx ? tx : tx;
        *maskY = ty;
    }
}

/*
 * lock_surface - Lock HAL surface and cache required information.
 * Requires corresponding unlock_surface later.
 */
static int lock_surface(__hwc2_device_t *dev, gcoSURF surface,
                    struct surface *sur, uint32_t isTarget)
{
    gceTILING tiling;
    gctINT stride;
    gctUINT width;
    gctUINT height;
    gceSURF_FORMAT format;
    gcsSURF_FORMAT_INFO_PTR info[2];
    gce2D_TILE_STATUS_CONFIG tsConfig = gcv2D_TSC_DISABLE;

    if (!surface) {
        sur->surface = NULL;
        return 0;
    }

    gcoSURF_GetTiling(surface, &tiling);
    gcoSURF_GetFormat(surface, NULL, &format);
    gcoSURF_GetSize(surface, &width, &height, gcvNULL);
    gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, &stride);

    if (gcmIS_ERROR(gcoSURF_QueryFormat(format, info)))
        return -EINVAL;

    if (gcmIS_ERROR(gcoSURF_Lock(surface, sur->address, gcvNULL))) {
        return -EINVAL;
    }

    sur->width  = width;
    sur->height = height;
    sur->format = format;
    sur->tiling = tiling;
    sur->tsConfig   = tsConfig;
    sur->formatInfo = info[0];

    switch (format) {
        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_YV12:
        case gcvSURF_I420:
            sur->bytesPerPixel = 1;
            break;
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            sur->bytesPerPixel = 1;
            break;
        default:
            sur->bytesPerPixel = info[0]->bitsPerPixel / 8;
            break;
    }

    switch (format) {
        case gcvSURF_NV12:
        case gcvSURF_NV21:
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            sur->stride[0] = sur->stride[1] = stride;
            sur->numPlanes = 2;
            break;
        case gcvSURF_YV12:
        case gcvSURF_I420:
            sur->stride[0] = stride;
            sur->stride[1] = sur->stride[2] = ((stride / 2) + 0xf) & ~0xf;
            sur->numPlanes = 3;
            break;
        default:
            sur->stride[0] = stride;
            sur->numPlanes = 1;
            break;
    }

    if (tiling & gcvTILING_SPLIT_BUFFER) {
        gctUINT32 offset = 0;
        gcoSURF_GetBottomBufferOffset(surface, &offset);

        sur->stride[1]  = sur->stride[0];
        sur->address[1] = sur->address[0] + offset;
        sur->numPlanes  = 2;
    }

    calculate_alignment_mask(dev, sur, isTarget,
            &sur->alignMaskX, &sur->alignMaskY);

    sur->surface = surface;

    return 0;
}

/*
 * unlock_surface - release locked HAL surface and reset information.
 */
static void unlock_surface(__hwc2_device_t *dev, struct surface *sur)
{
    if (!sur->surface)
        return;

    gcmVERIFY_OK(gcoSURF_Unlock(sur->surface, gcvNULL));

    sur->width  = 0;
    sur->height = 0;
    sur->format = gcvSURF_UNKNOWN;
    sur->tiling = gcvINVALIDTILED;

    sur->surface = NULL;
}

/*
 * acquire_output_buffer - Acquire output buffer for write.
 */
static inline void acquire_output_buffer(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, buffer_handle_t buffer,
                        int32_t releaseFence)
{
    gc_native_handle_t *handle = gc_native_handle_get(buffer);
    gcoSURF surface = (gcoSURF)handle->surface;

    if (releaseFence != -1) {
        sync_wait(releaseFence, -1);
        close(releaseFence);
    }

    dpy->blt.surface = surface;
    dpy->blt.format  = (gceSURF_FORMAT)handle->halFormat;

    lock_surface(dev, surface, &dpy->blt.sur, 1);
}

/*
 * release_output_buffer - Release output buffer.
 */
static inline void release_output_buffer(__hwc2_device *dev, __hwc2_display_t *dpy)
{
    if (dpy->blt.surface) {
        unlock_surface(dev, &dpy->blt.sur);
        dpy->blt.surface = NULL;

        gcoHAL_Commit(gcvNULL, gcvFALSE);
    }
}

/*
 * acquire_layer_source - Acquire layer buffer for read.
 */
static inline void acquire_layer_source(__hwc2_device_t *dev,
                        __hwc2_layer_t *layer)
{
    if (layer->composition == HWC2_COMPOSITION_DEVICE ||
            layer->composition == HWC2_COMPOSITION_CURSOR) {

        if (unlikely(!layer->buffer || private_handle_t::validate(layer->buffer))) {
            layer->blt.surface = NULL;
            layer->blt.color32 = 0;

            __hwc2_trace(2, "layer=%p(%d): buggy", layer, layer->id);
        } else {
            gc_native_handle_t *handle = gc_native_handle_get(layer->buffer);
            gcoSURF surface = (gcoSURF)(uintptr_t)handle->surface;

            layer->blt.surface = surface;
            layer->blt.color32 = 0;

            if (handle->halFormat >= gcvSURF_YUY2 &&
                    handle->halFormat <= gcvSURF_NV61) {
                /*
                 * Need filter-blit for yuv format, when
                 * 1. no one pass filter, or
                 * 2. layer is scaled.
                 */
                if (!dev->blt.onePassFilter || layer->blt.scaled)
                    layer->blt.filter = 1;

                layer->blt.yuvClass = 1;
            }

            lock_surface(dev, surface, &layer->blt.sur, 0);

            __hwc2_trace(2, "layer=%p(%d): yuv=%d mask=%x,%x",
                layer, layer->id, layer->blt.yuvClass,
                layer->blt.sur.alignMaskX, layer->blt.sur.alignMaskY);
        }
    } else {
        layer->blt.surface = NULL;
        layer->blt.color32 = (layer->solidColor.a << 24) |
                (layer->solidColor.r << 16) | (layer->solidColor.g << 8) |
                layer->solidColor.b;
    }

    int fd = layer->acquireFence;
    if (fd != -1) {
        sync_wait(fd, -1);
        close(fd);
        layer->acquireFence = -1;
    }
}

/*
 * acquire_sources - Acquire all layer sources for read.
 */
static inline void acquire_sources(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy)
{
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
        acquire_layer_source(dev, layer);
    }
}

/*
 * release_sources - Release all layer sources.
 */
static inline void release_sources(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy, int releaseFence)
{
    int needCommit = 0;

    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];

        if (layer->blt.surface) {
            unlock_surface(dev, &layer->blt.sur);
            layer->blt.surface = NULL;
            needCommit = 1;
        }

        /* TODO: Set releaseFence to layer. */
    }

    if (needCommit) {
        gcoHAL_Commit(gcvNULL, gcvFALSE);
    }
}

static inline void release_fences_quirk(__hwc2_device_t *dev,
                        __hwc2_display_t *dpy)
{
    for (uint32_t i = 0; i < dpy->blt.numBltLayers; i++) {
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];

        int fd = layer->acquireFence;
        if (fd != -1) {
            close(fd);
            layer->acquireFence = -1;
        }
    }
}

/*
 * obtain_output_surface - Obtain cached output buffer information.
 * 'lock_surface' must be called before this function.
 */
static inline
struct surface *obtain_output_surface(__hwc2_device_t *dev,
                            __hwc2_display_t *dpy)
{
    return &dpy->blt.sur;
}

/*
 * obtain_layer_surface - Obtain cached layer buffer information.
 * 'lock_surface' must be called before this function.
 */
static inline
struct surface *obtain_layer_surface(__hwc2_device_t *dev,
                            __hwc2_layer_t *layer)
{
    return &layer->blt.sur;
}

/*
 * calculate_source_clip - Calculate source clip rectangle in dest coordiante
 *                          system corresponding to dest clip.
 */
static inline void calculate_source_clip(__hwc2_layer_t *layer,
                        const gcsRECT *dstClip, gcsRECT *srcClip)
{
    hwc_frect_t *src = &layer->blt.sourceCropD;
    hwc_rect_t *dest = &layer->displayFrame;

    int dl = dest->left   - dstClip->left;
    int dt = dest->top    - dstClip->top;
    int dr = dest->right  - dstClip->right;
    int db = dest->bottom - dstClip->bottom;

    if (layer->blt.hMirror) {
        int tl = dl;
        dl = -dr;
        dr = -tl;
    }

    if (layer->blt.vMirror) {
        int tt = dt;
        dt = -db;
        db = -tt;
    }

    if (layer->blt.scaled) {
        srcClip->left   = ceilf (src->left   - dl * layer->blt.hScale);
        srcClip->top    = ceilf (src->top    - dt * layer->blt.vScale);
        srcClip->right  = floorf(src->right  - dr * layer->blt.hScale);
        srcClip->bottom = floorf(src->bottom - db * layer->blt.vScale);

    } else {
        srcClip->left   = src->left   - dl;
        srcClip->top    = src->top    - dt;
        srcClip->right  = src->right  - dr;
        srcClip->bottom = src->bottom - db;
    }
}

static inline void commit_blit(__hwc2_device_t *device, int *fence)
{
    if (fence) {
        /* TODO: use native fence sync. */
        gcoHAL_Commit(gcvNULL, gcvTRUE);
        *fence = -1;
    } else {
        gcoHAL_Commit(gcvNULL, gcvTRUE);
    }
}

/*
 * program_output_buffer - Program target buffer.
 *
 * Blitter will only use the output buffer as target. Hence we only need to
 * program it once --- exception is that, multi-blt-v1 need fake target surface,
 * in which case we'll recover the target buffer inside that function.
 */
static void program_output_buffer(__hwc2_device_t *device,
                    __hwc2_display_t *dpy)
{
    blitter_device_t *blt = &device->blt;
    struct surface *sur;

    sur = obtain_output_surface(device, dpy);

    __hwc2_trace(2, "surface=%p size=%ux%u format=%d stride=%u"
        " tiling=0x%x address=0x%08x",
        sur->surface, sur->width, sur->height, sur->format,
        sur->stride[0], sur->tiling, sur->address[0]);

    gcmVERIFY_OK(gcoSURF_Set2DTarget(dpy->blt.surface, gcvSURF_0_DEGREE));

    dpy->blt.fullScreen = {
        0, 0,
        (gctINT)sur->width,
        (gctINT)sur->height
    };

    gcmVERIFY_OK(gco2D_SetClipping(blt->engine, &dpy->blt.fullScreen));
}

/*
 * program_copy_region - Copy not damaged region from previous buffer.
 */
static void program_copy_region(__hwc2_device_t *device,
                    __hwc2_display_t *dpy)
{
    __hwc2_list_t *pos;
    gceSTATUS status;

    if (__hwc2_list_empty(&dpy->blt.copyArea)) {
        __hwc2_trace(2, "no copy region");
        return;
    }

    __hwc2_trace(0, "");
    blitter_device_t *blt = &device->blt;

    gc_native_handle_t *handle = gc_native_handle_get(dpy->blt.bufferRef);
    gcoSURF prev = (gcoSURF)handle->surface;

    /* Disable blending and premultiply. */
    gcmONERROR(gco2D_DisableAlphaBlend(blt->engine));
    gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(blt->engine,
            gcv2D_COLOR_MULTIPLY_DISABLE, gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE, gcv2D_COLOR_MULTIPLY_DISABLE));

    gcmONERROR(gcoSURF_Set2DSource(prev, gcvSURF_0_DEGREE));
    gcmONERROR(gco2D_SetBitBlitMirror(blt->engine, gcvFALSE, gcvFALSE));

    __hwc2_list_for_each(pos, &dpy->blt.copyArea) {
        struct area *a = __hwc2_list_entry(pos, struct area, link);
        gcsRECT rect = {
            a->rect.left,  a->rect.top,
            a->rect.right, a->rect.bottom,
        };

        __hwc2_trace_string("[%d,%d,%d,%d]",
            rect.left, rect.top, rect.right, rect.bottom);

        gcmONERROR(gco2D_SetSource(blt->engine, &rect));
        gcmONERROR(gco2D_Blit(blt->engine, 1, &rect, 0xCC, 0xCC, dpy->blt.format));
    }
    __hwc2_trace(1, "");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * program_clear_worm_hole - Clear worm holes to transparent black.
 */
static void program_clear_worm_hole(__hwc2_device_t *device,
                    __hwc2_display_t *dpy)
{
    blitter_device_t *blt = &device->blt;
    __hwc2_list_t *pos;
    gcsRECT rects[16];
    gctUINT num = 0;
    gceSTATUS status;

    if (__hwc2_list_empty(&dpy->blt.wormHoleDamaged)) {
        __hwc2_trace(2, "no worm-hole");
        return;
    }

    __hwc2_trace(0, "");

    gcmONERROR(gco2D_DisableAlphaBlend(blt->engine));

    gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(blt->engine,
            gcv2D_COLOR_MULTIPLY_DISABLE, gcv2D_COLOR_MULTIPLY_DISABLE,
            gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE, gcv2D_COLOR_MULTIPLY_DISABLE));

    __hwc2_list_for_each(pos, &dpy->blt.wormHoleDamaged) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);

        rects[num++] = {
            area->rect.left,  area->rect.top,
            area->rect.right, area->rect.bottom
        };

        __hwc2_trace_string("[%d,%d,%d,%d]",
                area->rect.left,  area->rect.top,
                area->rect.right, area->rect.bottom);

        if (num >= ARRAY_SIZE(rects) ||
                __hwc2_list_is_last(pos, &dpy->blt.wormHoleDamaged)) {
            gcmONERROR(gco2D_Clear(blt->engine, num, rects,
                    0x00000000, 0xCC, 0xCC, dpy->blt.format));
            num = 0;
        }
    }

    __hwc2_trace(1, "");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * 3 conditions to disable alpha blend:
 * 1. Layer does not enable alpha blend (blitter_layer::blendForumula)
 * 2. tough-blend-hole optimized out, per layer, (blitter_layer::opaque)
 * 3. blend-hole optimization, per area (parameter opaque)
 */
static void program_layer_blend(__hwc2_device_t *device,
                    __hwc2_layer_t *layer, int opaque)
{
    blitter_device_t *blt = &device->blt;
    struct blend_formula *formula;
    gctUINT32 srcGlobal;
    gceSTATUS status;

    __hwc2_trace(2, "layer=%p(%u) formula=%d globalAlpha=%u opaque=%d",
            layer, layer->id, layer->blt.blendFormula,
            layer->blt.globalAlpha, (layer->blt.opaque | opaque));

    formula = &blend_formula[layer->blt.blendFormula];

    if (layer->blt.blendFormula == FORMULA_DISABLED ||
            layer->blt.opaque || opaque) {
        gcmONERROR(gco2D_DisableAlphaBlend(blt->engine));
    } else {
        gcmONERROR(gco2D_EnableAlphaBlendAdvanced(blt->engine,
                formula->srcAlphaMode, formula->dstAlphaMode,
                formula->srcGlobalAlphaMode, formula->dstGlobalAlphaMode,
                formula->srcFactorMode, formula->dstFactorMode));
    }

    gcmONERROR(gco2D_SetPixelMultiplyModeAdvanced(blt->engine,
            formula->srcPremultSrcAlpha, formula->dstPremultDstAlpha,
            formula->srcPremultGlobalMode, formula->dstDemultDstAlpha));

    srcGlobal = layer->blt.globalAlpha;

    srcGlobal |= srcGlobal << 8;
    srcGlobal |= srcGlobal << 16;

    gcmONERROR(gco2D_SetSourceGlobalColorAdvanced(blt->engine, srcGlobal));
    gcmONERROR(gco2D_SetTargetGlobalColorAdvanced(blt->engine, 0xFFFFFFFF));
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
}

/*
 * program_bit_blit - Normal bit blit.
 */
static void program_bit_blit(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rects, uint32_t numRects)
{
    blitter_device_t *blt = &device->blt;
    gceSTATUS status;
    gcoSURF surface;
    gceSURF_FORMAT format = dpy->blt.format;

    surface = layer->blt.surface;

    __hwc2_trace(0, "layer=%p(%u) numRects=%u buffer=%p surface=%p",
            layer, layer->id, numRects, layer->buffer, surface);

    gctBOOL hMirror = layer->blt.hMirror;
    gctBOOL vMirror = layer->blt.vMirror;
    gceSURF_ROTATION rotation = gceSURF_ROTATION(layer->blt.rotation);

    gcmONERROR(gcoSURF_Set2DSource(surface, rotation));
    gcmONERROR(gco2D_SetBitBlitMirror(blt->engine, hMirror, vMirror));

    /* Go through the rectangles. */
    for (uint32_t i = 0; i < numRects; i++) {
        gcsRECT srcRect;
        gcsRECT dstRect = {
            rects[i].left,  rects[i].top,
            rects[i].right, rects[i].bottom
        };

        calculate_source_clip(layer, &dstRect, &srcRect);

        __hwc2_trace_string("[%d,%d,%d,%d] => [%d,%d,%d,%d]",
                srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);

        gcmONERROR(gco2D_SetSource(blt->engine, &srcRect));
        gcmONERROR(gco2D_Blit(blt->engine, 1, &dstRect, 0xCC, 0xCC, format));
    }

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * program_stretch_blit - stretch blit.
 */
static void program_stretch_blit(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rects, uint32_t numRects)
{
    blitter_device_t *blt = &device->blt;
    gceSTATUS status;
    gcoSURF surface;
    gceSURF_FORMAT format = dpy->blt.format;
    hwc_region_t *region = &layer->visibleRegion;

    surface = layer->blt.surface;

    __hwc2_trace(0, "layer=%p(%u) numRects=%u buffer=%p surface=%p",
            layer, layer->id, numRects, layer->buffer, surface);

    gctBOOL hMirror = layer->blt.hMirror;
    gctBOOL vMirror = layer->blt.vMirror;
    gceSURF_ROTATION rotation = gceSURF_ROTATION(layer->blt.rotation);

    gcmONERROR(gcoSURF_Set2DSource(surface, rotation));
    gcmONERROR(gco2D_SetBitBlitMirror(blt->engine, hMirror, vMirror));

    /* Go through the rectangles. */
    for (uint32_t i = 0; i < numRects; i++) {
        gcsRECT srcRect;
        gcsRECT dstRect;
        uint32_t found = 0;

        gcsRECT clipRect = {
            rects[i].left,
            rects[i].top,
            rects[i].right,
            rects[i].bottom
        };

        /*
         * Find rect of visibleRegion.
         * This is to blit with same dstRect whenever 'rects'/damage changes, to
         * avoid precision issue.
         */
        for (uint32_t j = 0; j < region->numRects; j++) {
            const hwc_rect_t *r = &region->rects[j];
            if (r->left <= clipRect.left && r->top  <= clipRect.top &&
                r->right >= clipRect.right && r->bottom >= clipRect.bottom) {
                dstRect = {
                    r->left,  r->top,
                    r->right, r->bottom
                };
                found = 1;
                break;
            }
        }

        LOG_ALWAYS_FATAL_IF(!found, "Invalid clip");

        calculate_source_clip(layer, &dstRect, &srcRect);

        __hwc2_trace_string("[%d,%d,%d,%d] => [%d,%d,%d,%d] clip[%d,%d,%d,%d]",
                srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                dstRect.left, dstRect.top, dstRect.right, dstRect.bottom,
                clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);

        if (unlikely(srcRect.right <= srcRect.left)) {
            gctUINT width;
            gcoSURF_GetSize(surface, &width, NULL, NULL);

            if (srcRect.left < (gctINT)width)
                srcRect.right = srcRect.left + 1;
            else
                continue;
        }

        if (unlikely(srcRect.bottom <= srcRect.top)) {
            gctUINT height;
            gcoSURF_GetSize(surface, NULL, &height, NULL);

            if (srcRect.top < (gctINT)height)
                srcRect.bottom = srcRect.top + 1;
            else
                continue;
        }

        gcmONERROR(gco2D_SetSource(blt->engine, &srcRect));
        gco2D_SetStretchRectFactors(blt->engine, &srcRect, &dstRect);
        gcmONERROR(gco2D_SetClipping(blt->engine, &clipRect));

        gcmONERROR(gco2D_StretchBlit(blt->engine, 1, &dstRect, 0xCC, 0xCC, format));
    }

    /* Reset clipping. */
    gcmONERROR(gco2D_SetClipping(blt->engine, &dpy->blt.fullScreen));

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * program_filter_blit - Launch filter filter
 * For scaling, or YUV sources.
 */
static void program_filter_blit(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rects, uint32_t numRects)
{
    blitter_device_t *blt = &device->blt;
    gceSTATUS status;
    struct surface *src;
    struct surface *dst;

    __hwc2_trace(0, "layer=%p(%u) numRects=%u buffer=%p",
            layer, layer->id, numRects, layer->buffer);

    src = obtain_layer_surface(device, layer);
    dst = obtain_output_surface(device, dpy);

    hwc_frect_t *s = &layer->blt.sourceCropD;
    hwc_rect_t *d = &layer->displayFrame;

    gcsRECT srcRect = {
        (gctINT)ceilf (s->left),
        (gctINT)ceilf (s->top),
        (gctINT)floorf(s->right),
        (gctINT)floorf(s->bottom)
    };

    gcsRECT dstRect = {
        d->left,  d->top,
        d->right, d->bottom
    };

    for (uint32_t i = 0; i < numRects; i++) {
        gcsRECT subRect = {
            rects[i].left   - dstRect.left,
            rects[i].top    - dstRect.top,
            rects[i].right  - dstRect.left,
            rects[i].bottom - dstRect.top,
        };

        gctUINT hKernel;
        gctUINT vKernel;

        /* TODO: translate to dest rotation. */
        gctBOOL hMirror = layer->blt.hMirror;
        gctBOOL vMirror = layer->blt.vMirror;

        if (likely(layer->blt.scaled)) {
            hKernel = (layer->blt.hScale <= 2) ? 3 : 5;
            vKernel = (layer->blt.vScale <= 2) ? 3 : 5;
        } else {
            hKernel = 1;
            vKernel = 1;
        }

        __hwc2_trace_string("[%d,%d,%d,%d] => [%d,%d,%d,%d] clip[%d,%d,%d,%d]",
            srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
            dstRect.left, dstRect.top, dstRect.right, dstRect.bottom,
            rects[i].left, rects[i].top, rects[i].right, rects[i].bottom);

        gcmONERROR(gco2D_SetBitBlitMirror(blt->engine, hMirror, vMirror));

        /* Use SINC filter, sinc(x) = sin(x)/x. */
        gcmONERROR(gco2D_SetFilterType(blt->engine, gcvFILTER_SYNC));
        gcmONERROR(gco2D_SetKernelSize(blt->engine, hKernel, vKernel));

        gcmONERROR(gco2D_FilterBlitEx2(blt->engine,
                        src->address, src->numPlanes,
                        src->stride, src->numPlanes,
                        src->tiling, src->format,
                        layer->blt.rotation,
                        src->width, src->height,
                        &srcRect,
                        dst->address, dst->numPlanes,
                        dst->stride, dst->numPlanes,
                        dst->tiling, dst->format,
                        gcvSURF_0_DEGREE,
                        dst->width, dst->height,
                        &dstRect, &subRect));
    }

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * program_blit_solid_color - blit with a single color.
 *
 * Use clear when blending disabled, or pattern otherwise.
 */
static void program_blit_solid_color(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rects, uint32_t numRects)
{
    blitter_device_t *blt = &device->blt;
    gceSTATUS status;
    gceSURF_FORMAT format = dpy->blt.format;

    __hwc2_trace(0, "layer=%p(%u) rects=%p numRects=%u",
            layer, layer->id, rects, numRects);

    for (uint32_t i = 0; i < numRects; i++) {
        gcsRECT rect = {
            rects[i].left,  rects[i].top,
            rects[i].right, rects[i].bottom
        };

        __hwc2_trace_string("[%d,%d,%d,%d]",
            rect.left, rect.top, rect.right, rect.bottom);

        if (layer->blt.blendFormula == FORMULA_DISABLED) {
            /* Use clear. */
            gcmONERROR(gco2D_Clear(blt->engine,
                            1, &rect, layer->blt.color32, 0xCC, 0xCC, format));
        } else {
            /* Use blit with pattern. */
            gcmONERROR(gco2D_LoadSolidBrush(blt->engine,
                            format, gcvTRUE, layer->blt.color32, ~0ull));

            gcmONERROR(gco2D_Blit(blt->engine, 1, &rect, 0xF0, 0xF0, format));
        }
    }

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
    __hwc2_trace(1, "error");
}

/*
 * blit_layer_rects - Blit multiple rects of given layer.
 */
static inline void blit_layer_rects(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rects, uint32_t num)
{
    if (layer->blt.surface) {
        if (layer->blt.filter)
            program_filter_blit(device, dpy, layer, rects, num);
        else if (layer->blt.scaled)
            program_stretch_blit(device, dpy, layer, rects, num);
        else
            program_bit_blit(device, dpy, layer, rects, num);
    } else
        program_blit_solid_color(device, dpy, layer, rects, num);
}

/*
 * blit_layer - Blit all rects of visibleRegion of the layer.
 *
 * Blit by layer, only used for layers can not be splitted to area(s).
 */
static void blit_layer(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer)
{
    __hwc2_list_t *pos;
    hwc_rect_t rects[16];
    uint32_t num = 0;

    __hwc2_trace(0, "layer=%p(%u) composition=%s(%d)", layer, layer->id,
            composition_name(layer->composition), layer->composition);

    program_layer_blend(device, layer, 0);

    __hwc2_list_for_each(pos, &layer->blt.visibleDamaged) {
        struct area *area = __hwc2_list_entry(pos, struct area, link);
        rects[num++] = area->rect;

        if (num >= ARRAY_SIZE(rects) ||
                __hwc2_list_is_last(pos, &layer->blt.visibleDamaged)) {
            blit_layer_rects(device, dpy, layer, rects, num);
            num = 0;
        }
    }

    __hwc2_trace(1, "done");
}

struct multi_blit_param
{
    int opaque;
    uint32_t numTotal;
    uint32_t numBuffer;
    uint32_t numYuvSource;

    /* Rects. */
    hwc_rect_t marginRect;
    hwc_rect_t maskRect;

    /* Need same mirror if no multiBltSourceMirror. */
    int hMirror;
    int vMirror;
    /* Need same rotation if no multiBltSourceRotation. */
    gceSURF_ROTATION rotation;
};

/*
 * init_multi_blit_batch - Initial multi-blit parameter struct.
 * @opaque: Set to 1 if the bottom layer is blend-hole can be optimized out.
 */
static void init_multi_blit_batch(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rect, int opaque,
                    struct multi_blit_param *param)
{
    param->opaque = opaque;
    param->numTotal = 0;
    param->numBuffer = 0;
    param->numYuvSource = 0;

    param->marginRect = {-1, -1, -1, -1};
    param->maskRect = {0, 0, 0, 0};

    if (!device->blt.multiBltSourceOffset) {
        struct surface *sur    = obtain_output_surface(device, dpy);

        param->maskRect.left   = sur->alignMaskX;
        param->marginRect.left = sur->alignMaskX & rect->left;
    }

    param->vMirror  = 0;
    param->hMirror  = 0;
    param->rotation = gcvSURF_0_DEGREE;
}

/*
 * append_multi_blit_layer - Try to append a layer for multi-blit.
 * Returns 0 if success. 1 for failure.
 */
static int32_t append_multi_blit_layer(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, __hwc2_layer_t *layer,
                    const hwc_rect_t *rect, struct multi_blit_param *param)
{
    blitter_device_t *blt = &device->blt;
    struct surface *sur;
    int ml = 0, mt = 0, mr = 0, mb = 0; /* Margin Masks. */
    int el = -1, et = -1, er = -1, eb = -1; /* Margins. */
    int dl, dt, dr, db; /* Delta. */
    hwc_rect_t *marginRect = &param->marginRect;
    hwc_rect_t *maskRect   = &param->maskRect;
    hwc_rect_t sourceCrop;  /* source crop rect in integer. */

    if (param->numTotal == device->blt.multiBltMaxSource) {
        __hwc2_trace(2, "layer=%p(%u): too many layers: %u",
                layer, layer->id, param->numTotal);
        return 1;
    }

    /* Can always handle solid color. */
    if (!layer->blt.surface)
        goto success;

    if (layer->blt.yuvClass) {
        if (param->numYuvSource == device->blt.multiBltMaxYuvSource) {
            __hwc2_trace(2, "layer=%p(%u): too many yuv layers: %u",
                    layer, layer->id, param->numYuvSource);
            return 1;
        }
    }

    if (unlikely(layer->blt.scaled)) {
        /* Do not handle scaling. */
        __hwc2_trace(2, "layer=%p(%u): scaled", layer, layer->id);
        return 1;
    }

    if (!blt->multiBltSourceMirror) {
        if (!param->numBuffer) {
            param->hMirror  = layer->blt.hMirror;
            param->vMirror  = layer->blt.vMirror;
        } else if ((param->hMirror != layer->blt.hMirror) ||
                (param->vMirror != layer->blt.vMirror)) {
            __hwc2_trace(2, "layer=%p(%u): different mirror: %d,%d vs %d,%d",
                layer, layer->id,
                param->hMirror, layer->blt.hMirror,
                param->vMirror, layer->blt.vMirror);
            return 1;
        }
    }

    if (!blt->multiBltSourceRotation) {
        if (!param->numBuffer)
            param->rotation = layer->blt.rotation;
        else if (param->rotation != layer->blt.rotation) {
            __hwc2_trace(2, "layer=%p(%u): different rotation: %d vs %d",
                layer, layer->id,
                param->rotation, layer->blt.rotation);
            return 1;
        }
    }

    if (!blt->multiBltSourceMirror && !blt->multiBltSourceRotation) {
        if ((param->hMirror || param->vMirror) &&
                param->rotation != gcvSURF_0_DEGREE) {
            __hwc2_trace(2, "layer=%p(%u): mirror and rotation both exist",
                    layer, layer->id);
            return 1;
        }
    }

    /* No more check if source offset feature exists. */
    if (blt->multiBltSourceOffset)
        goto success;

    sur = obtain_layer_surface(device, layer);

    /* No alignment requirement, neither even coordinate limitation. */
    if (!sur->alignMaskX && !sur->alignMaskY && blt->multiBltOddCoord)
        goto success;

    sourceCrop = {
        (int)layer->sourceCrop.left,
        (int)layer->sourceCrop.top,
        (int)layer->sourceCrop.right,
        (int)layer->sourceCrop.bottom,
    };

    /* Delta. */
    dl = layer->displayFrame.left   - rect->left;
    dt = layer->displayFrame.top    - rect->top;
    dr = layer->displayFrame.right  - rect->right;
    db = layer->displayFrame.bottom - rect->bottom;

    if (layer->blt.hMirror) {
        int tl = dl;
        dl = -dr;
        dr = -tl;
    }

    if (layer->blt.vMirror) {
        int tt = dt;
        dt = -db;
        db = -tt;
    }

    if (!sur->alignMaskX && !sur->alignMaskY) {
        int dx, dy;

        el = marginRect->left   > 0 ? marginRect->left   : 0;
        et = marginRect->top    > 0 ? marginRect->top    : 0;
        er = marginRect->right  > 0 ? marginRect->right  : 0;
        eb = marginRect->bottom > 0 ? marginRect->bottom : 0;

        switch (layer->blt.rotation) {
            case gcvSURF_0_DEGREE:
            default:
                dx = sourceCrop.left - dl - el;
                dy = sourceCrop.top  - dt - et;
                break;
            case gcvSURF_90_DEGREE:
                dx = sourceCrop.left + db - eb;
                dy = sourceCrop.top  - dl - el;
                break;
            case gcvSURF_180_DEGREE:
                dx = sourceCrop.left + dr - er;
                dy = sourceCrop.top  + db - eb;
                break;
            case gcvSURF_270_DEGREE:
                dx = sourceCrop.left - dt - et;
                dy = sourceCrop.top  + dr - er;
                break;
        }

        /* Even coordinate limitation, can not accept odd ones. */
        if (((dx < 0) && (dx & 1)) || ((dy < 0) && (dy & 1))) {
            __hwc2_trace(2, "layer=%p(%u): odd start address", layer, layer->id);
            return 1;
        } else
            goto success;
    }

    /* Check start address alignments.
     * We need to move buffer address to an aligned one, so we
     * need compute corresponding coordinate offsets.
     * The 'margin's are actually left coodinate offsets in
     * 'original coord sys'.
     *
     * Left coordinate becomes at left,bottom,right,top for 0,90,
     * 180,270 degree rotated, respectively.
     * So the first step is to compute needed coordinate (need left
     * for 0 deg, bottom for 90 deg, etc) to 'original coord sys',
     * and then move the left coordinate in 'original coord sys' to
     * aligned one.
     *
     * Top coordinate becomes at top,left,bottom,right for 0,90,
     * 180,270 degree rotated, respectively.
     * So the first step is to compute needed coordinate (need top
     * for 0 deg, left for 90 deg, etc) to 'original coord sys',
     * and then move the top coordinate in 'original coord sys' to
     * aligned one.
     *
     * [+]-------------------------+  [+]------------------------+
     *  |        ^                 |  |        ^                 |
     *  |        L (270 deg)       |  |        T (0 deg)         |
     *  |        v                 |  |        v                 |
     *  |     +--------------+     |  |     +--------------+     |
     *  |     | blit rect    |     |  |     | blit rect    |     |
     *  |<-L->|              |<-L->|  |<-T->|              |<-T->|
     *  |(0   |              |(180 |  |(90  |              |(270 |
     *  | deg)|              | deg)|  | deg)|              | deg)|
     *  |     +--------------+     |  |     +--------------+     |
     *  |        ^                 |  |        ^                 |
     *  |        L (90 deg)        |  |        T (180 deg)       |
     *  |        v                 |  |        v                 |
     *  +--------------------------+  +--------------------------+
     */
    switch (layer->blt.rotation) {
        case gcvSURF_0_DEGREE:
        default:
            ml = sur->alignMaskX;
            mt = sur->alignMaskY;
            el = (sourceCrop.left - dl) & ml;
            et = (sourceCrop.top  - dt) & mt;
            break;
        case gcvSURF_90_DEGREE:
            mb = sur->alignMaskX;
            ml = sur->alignMaskY;
            eb = (sourceCrop.left + db) & mb;
            el = (sourceCrop.top  - dl) & ml;
            break;
        case gcvSURF_180_DEGREE:
            mr = sur->alignMaskX;
            mb = sur->alignMaskY;
            er = (sourceCrop.left + dr) & mr;
            eb = (sourceCrop.top  + db) & mb;
            break;
        case gcvSURF_270_DEGREE:
            mt = sur->alignMaskX;
            mr = sur->alignMaskY;
            et = (sourceCrop.left - dt) & mt;
            er = (sourceCrop.top  + dr) & mr;
            break;
    }

    if (el >= 0 && marginRect->left >= 0) {
        if (ml > maskRect->left) {
            if ((el & maskRect->left) != marginRect->left) {
                __hwc2_trace(2, "layer=%p(%u): stronger left not match", layer, layer->id);
                return 1;
            }
        } else {
            if ((marginRect->left & ml) != el) {
                __hwc2_trace(2, "layer=%p(%u): weaker left not match", layer, layer->id);
                return 1;
            }
        }
    } else if (er >= 0 && marginRect->top >= 0) {
        if (mr > maskRect->right) {
            if ((er & maskRect->right) != marginRect->right) {
                __hwc2_trace(2, "layer=%p(%u): stronger right not match", layer, layer->id);
                return 1;
            }
        } else {
            if ((marginRect->right & mr) != er) {
                __hwc2_trace(2, "layer=%p(%u): weaker right not match", layer, layer->id);
                return 1;
            }
        }
    }

    if (et >= 0 && marginRect->top >= 0) {
        if (mt > maskRect->top) {
            if ((et & maskRect->top) != marginRect->top) {
                __hwc2_trace(2, "layer=%p(%u): stronger top not match", layer, layer->id);
                return 1;
            }
        } else {
            if ((marginRect->top & mt) != et) {
                __hwc2_trace(2, "layer=%p(%u): weaker top not match", layer, layer->id);
                return 1;
            }
        }
    } else if (eb >= 0 && marginRect->bottom >= 0) {
        if (mb > maskRect->bottom) {
            if ((eb & maskRect->bottom) != marginRect->bottom) {
                __hwc2_trace(2, "layer=%p(%u): stronger bottom not match", layer, layer->id);
                return 1;
            }
        } else {
            if ((marginRect->bottom & mb) != eb) {
                __hwc2_trace(2, "layer=%p(%u): weaker bottom not match", layer, layer->id);
                return 1;
            }
        }
    }

    if (el >= 0 && ml > maskRect->left) {
        marginRect->left = el;
        maskRect->left   = ml;
    } else if (er >= 0 && mr > maskRect->right) {
        marginRect->right = er;
        maskRect->right   = mr;
    }

    if (et >= 0 && mt > maskRect->top) {
        marginRect->top = et;
        maskRect->top   = mt;
    } else if (eb >= 0 && mb > maskRect->bottom) {
        marginRect->bottom = eb;
        maskRect->bottom   = mb;
    }

success:
    __hwc2_trace(2, "layer=%p(%u) >> "
            "mask=[0x%x,0x%x,0x%x,0x%x] margin=[%d,%d,%d,%d]",
            layer, layer->id,
            maskRect->left, maskRect->top,
            maskRect->right, maskRect->bottom,
            marginRect->left, marginRect->top,
            marginRect->right, marginRect->bottom);

    if (layer->blt.yuvClass)
        param->numYuvSource++;

    if (layer->blt.surface)
        param->numBuffer++;

    param->numTotal++;
    return 0;
}

/* Multi-blit v1 helper functions. */
/*
 * shift_coord_address - Re-calculate addresses because of coordinate shift.
 */
static void shift_coord_address(struct surface *sur, int dx, int dy,
                    uint32_t outAddress[3])
{
    /* Covert to signed, because dx,dy may be negative. */
    int64_t address[3] = {
        int64_t(sur->address[0]),
        int64_t(sur->address[1]),
        int64_t(sur->address[2]),
    };
    int32_t stride[3] = {
        int32_t(sur->stride[0]),
        int32_t(sur->stride[1]),
        int32_t(sur->stride[2]),
    };
    int32_t bytesPerPixel = int32_t(sur->bytesPerPixel);

    switch (sur->tiling) {
        case gcvMULTI_SUPERTILED:
            dy /= 2;
            outAddress[1] = address[1] + stride[1] * dy + bytesPerPixel * (dx * 64);
        case gcvSUPERTILED:
            outAddress[0] = address[0] + stride[0] * dy + bytesPerPixel * (dx * 64);
            return;
        case gcvMULTI_TILED:
            dy /= 2;
            outAddress[1] = address[1] + stride[1] * dy + bytesPerPixel * (dx * 4);
        case gcvTILED:
            outAddress[0] = address[0] + stride[0] * dy + bytesPerPixel * (dx * 4);
            return;
        default:
            outAddress[0] = address[0] + stride[0] * dy + bytesPerPixel * dx;
            /* More check for yuv formats below. */
            break;
    }

    switch (sur->format) {
        case gcvSURF_NV16:
        case gcvSURF_NV61:
            outAddress[1] = address[1] + stride[1] * dy + 1 * dx;
            break;
        case gcvSURF_NV12:
        case gcvSURF_NV21:
            outAddress[1] = address[1] + stride[1] * dy / 2 + 1 * dx;
            break;
        case gcvSURF_YV12:
        case gcvSURF_I420:
            outAddress[1] = address[1] + stride[1] * dy / 2 + 1 * dx / 2;
            outAddress[2] = address[2] + stride[2] * dy / 2 + 1 * dx / 2;
            break;
        default:
            break;
    }
}

/*
 * program_fake_source - program fake source buffer.
 * Fake source means shift'ed source buffer.
 */
static void inline program_fake_source(__hwc2_device_t *dev,
                        struct surface* sur, int dx, int dy,
                        gceSURF_ROTATION rotation,
                        uint32_t width, uint32_t height)
{
    gceSTATUS status;
    struct blitter_device *blt = &dev->blt;

    if (!dx && !dy && !width && !height) {
        __hwc2_trace(2, "format=%d tiling=0x%x stride=%u address=0x%08X",
            sur->format, sur->tiling, sur->stride[0], sur->address[0]);

        gcmONERROR(gcoSURF_Set2DSource(sur->surface, rotation));
    } else {
        if (!width)
            width = sur->width;

        if (!height)
            height = sur->height;

        if (dx || dy) {
            uint32_t address[3];
            shift_coord_address(sur, dx, dy, address);

            __hwc2_trace(2, "format=%d tiling=0x%x stride=%u shift=%d,%d "
                    "address=0x%08X -> 0x%08X",
                    sur->format, sur->tiling, sur->stride[0], dx, dy,
                    sur->address[0], address[0]);

            gcmONERROR(gco2D_SetGenericSource(blt->engine,
                    address, sur->numPlanes, sur->stride, sur->numPlanes,
                    sur->tiling, sur->format, rotation, width, height));

            gcmONERROR(gco2D_SetSourceTileStatus(blt->engine,
                    gcv2D_TSC_DISABLE, gcvSURF_UNKNOWN, 0, ~0U));
        } else {
            __hwc2_trace(2, "format=%d tiling=0x%x stride=%u address=0x%08X",
                    sur->format, sur->tiling, sur->stride[0], sur->address[0]);

            gcmONERROR(gco2D_SetGenericSource(blt->engine,
                    sur->address, sur->numPlanes, sur->stride, sur->numPlanes,
                    sur->tiling, sur->format, rotation, width, height));

            gcmONERROR(gco2D_SetSourceTileStatus(blt->engine,
                    gcv2D_TSC_DISABLE, gcvSURF_UNKNOWN, 0, ~0U));
        }
    }

    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
}

/*
 * program_fake_target - program fake target buffer.
 * Like fake source, fake target means shift'ed target buffer.
 */
static void inline program_fake_target(__hwc2_device_t *dev,
                        struct surface* sur, int dx, int dy,
                        gceSURF_ROTATION rotation,
                        uint32_t width, uint32_t height)
{
    gceSTATUS status;
    struct blitter_device *blt = &dev->blt;

    if (!dx && !dx && !width && !height) {
        __hwc2_trace(2, "format=%d tiling=0x%x stride=%u address=0x%08X",
            sur->format, sur->tiling, sur->stride[0], sur->address[0]);

        gcmONERROR(gcoSURF_Set2DTarget(sur->surface, rotation));
    } else {
        if (!width)
            width = sur->width;

        if (!height)
            height = sur->height;

        if (dx || dy) {
            uint32_t address[3];
            shift_coord_address(sur, dx, dy, address);

            __hwc2_trace(2, "format=%d tiling=0x%x stride=%u shift=%d,%d "
                    "address=0x%08X -> 0x%08X",
                    sur->format, sur->tiling, sur->stride[0], dx, dy,
                    sur->address[0], address[0]);

            gcmONERROR(gco2D_SetGenericTarget(blt->engine,
                    address, sur->numPlanes, sur->stride, sur->numPlanes,
                    sur->tiling, sur->format, rotation, width, height));

            gcmONERROR(gco2D_SetTargetTileStatus(blt->engine,
                    gcv2D_TSC_DISABLE, gcvSURF_UNKNOWN, 0, ~0U));
        } else {
            __hwc2_trace(2, "format=%d tiling=0x%x stride=%u address=0x%08X",
                    sur->format, sur->tiling, sur->stride[0], sur->address[0]);

            gcmONERROR(gco2D_SetGenericTarget(blt->engine,
                    sur->address, sur->numPlanes, sur->stride, sur->numPlanes,
                    sur->tiling, sur->format, rotation, width, height));

            gcmONERROR(gco2D_SetTargetTileStatus(blt->engine,
                    gcv2D_TSC_DISABLE, gcvSURF_UNKNOWN, 0, ~0U));
        }
    }

    return;

OnError:
    ALOGE("%s(%d): Failed, status=%d", __FUNCTION__, __LINE__, status);
}

/*
 * program_multi_blit_v1 - program multi blit batch, v1.
 *
 * v1 can ONLY support the same source rectangles and target rectangle. We shift
 * source buffers and target buffer to get the same fake rectangle.
 * The shift'ed source is called fake source. The shift'ed target is called fake
 * target.
 */
static void program_multi_blit_v1(__hwc2_device_t *device, __hwc2_display_t *dpy,
                    const hwc_rect_t *rect, uint64_t batchMask,
                    struct multi_blit_param *param)
{
    gceSTATUS status;

    struct blitter_device *blt = &device->blt;
    struct surface *target;

    uint32_t i;
    uint64_t mask;
    uint32_t sourceMask = 0;
    uint32_t sourceNum  = 0;
    uint32_t solidColorMask = 0; /* Mask of solid color layers. */

    /* Fake a rectangle, which is the same rectangle on source and dest.
     * This rectangle is actually clipRect against a new 'initial point' */
    gcsRECT fakeRect;
    int rectWidth  = rect->right  - rect->left;
    int rectHeight = rect->bottom - rect->top;

    /* Fake Surface size, which is the same size on source and dest. */
    uint32_t fakeWidth;
    uint32_t fakeHeight;

    gceSURF_ROTATION dstRotation = gcvSURF_0_DEGREE;

    hwc_rect_t *marginRect = &param->marginRect;

    if (marginRect->left < 0)
        marginRect->left = 0;
    if (marginRect->right < 0)
        marginRect->right  = 0;
    if (marginRect->top < 0)
        marginRect->top = 0;
    if (marginRect->bottom < 0)
        marginRect->bottom = 0;

    __hwc2_trace(0, "margin=[%d,%d,%d,%d] batchMask=0x%" PRIx64,
            marginRect->left, marginRect->top,
            marginRect->right, marginRect->bottom,
            batchMask);

    if (!blt->multiBltSourceMirror) {
        dstRotation = param->vMirror ? gcvSURF_FLIP_Y
                    : param->hMirror ? gcvSURF_FLIP_X
                    : gcvSURF_0_DEGREE;
    }

    if (!blt->multiBltSourceRotation) {
        if (param->rotation == gcvSURF_90_DEGREE)
            dstRotation = gcvSURF_270_DEGREE;
        else if (param->rotation == gcvSURF_270_DEGREE)
            dstRotation = gcvSURF_90_DEGREE;
        else if (param->rotation == gcvSURF_180_DEGREE)
            dstRotation = gcvSURF_180_DEGREE;
    }

    /***************************************************************************
    ** Determine fake rectangle and surface size.
    */

    /*  Fake rectangle is the clipRect against a new 'initial point'
     *
     *    orig 'initial point'
     *   [+]--------------------------------------...
     *    | \
     *    |   \
     *    |     \  new 'initial point'
     *    |     [+]-------------------------+
     *    |      |        ^                 |
     *    |      |        T (270 deg)       |
     *    |      |        v                 |
     *    |      |     +--------------+     |
     *    |      |     | fake rect    |     |
     *    |      |<-L->|              |<-R->|
     *    |      |(0   |              |(180 |
     *    |      | deg)|              | deg)|
     *    |      |     +--------------+     |
     *    |      |        ^                 |
     *    |      |        B (90 deg)        |
     *    |      |        v                 |
     *    |      +--------------------------+
     *    .
     *    .
     *    .
     *
     * L, T, R, B: are margin-s to the surface boundary.
     */

    /*
     * The marginRect is calculated for source rotation,
     * move it to dst rotation.
     */
    switch (dstRotation) {
        case gcvSURF_FLIP_X:
            fakeRect.left   = marginRect->right;
            fakeRect.top    = marginRect->top;
            fakeRect.right  = marginRect->right + rectWidth;
            fakeRect.bottom = marginRect->top   + rectHeight;
            break;
        case gcvSURF_FLIP_Y:
            fakeRect.left   = marginRect->left;
            fakeRect.top    = marginRect->bottom;
            fakeRect.right  = marginRect->left   + rectWidth;
            fakeRect.bottom = marginRect->bottom + rectHeight;
            break;
        case gcvSURF_90_DEGREE:
            fakeRect.left   = marginRect->top;
            fakeRect.top    = marginRect->right;
            fakeRect.right  = marginRect->top    + rectHeight;
            fakeRect.bottom = marginRect->right  + rectWidth;
            break;
        case gcvSURF_270_DEGREE:
            fakeRect.left   = marginRect->bottom;
            fakeRect.top    = marginRect->left;
            fakeRect.right  = marginRect->bottom + rectHeight;
            fakeRect.bottom = marginRect->left   + rectWidth;
            break;
        case gcvSURF_180_DEGREE:
            fakeRect.left   = marginRect->right;
            fakeRect.top    = marginRect->bottom;
            fakeRect.right  = marginRect->right  + rectWidth;
            fakeRect.bottom = marginRect->bottom + rectHeight;
            break;
        default:
            fakeRect.left   = marginRect->left;
            fakeRect.top    = marginRect->top;
            fakeRect.right  = marginRect->left + rectWidth;
            fakeRect.bottom = marginRect->top  + rectHeight;
            break;
    }

    /* Calculate surface size. */
    fakeWidth  = rectWidth  + marginRect->left + marginRect->right;
    fakeHeight = rectHeight + marginRect->top  + marginRect->bottom;

    target = obtain_output_surface(device, dpy);

    /* Compute offset to dest 'initial point'. */
    int dx = rect->left - marginRect->left;
    int dy = rect->top  - marginRect->top;

    __hwc2_trace_string("fakeRect=[%d,%d,%d,%d] fakeSize=%dx%d rot=%d",
            fakeRect.left,  fakeRect.top,
            fakeRect.right, fakeRect.bottom,
            fakeWidth, fakeHeight, dstRotation);

    /* Disable dest mirror. */
    gcmONERROR(gco2D_SetBitBlitMirror(blt->engine, gcvFALSE, gcvFALSE));

    /* Setup target. */
    program_fake_target(device, target, dx, dy, dstRotation, fakeWidth, fakeHeight);

    for (i = 0, mask = 1; mask <= batchMask; i++, mask <<= 1) {
        if (!(batchMask &mask))
            continue;

        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
        __hwc2_trace(0, "layer=%p(%u)", layer, layer->id);

        /* Setup source index. */
        gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, sourceNum));

        /* Setup source. */
        if (layer->blt.surface != gcvNULL) {
            struct surface *sur;

            int width  = fakeWidth;
            int height = fakeHeight;
            gceSURF_ROTATION rotation;
            int dl, dt, dr, db;

            hwc_rect_t sourceCrop = {
                (int)layer->sourceCrop.left,
                (int)layer->sourceCrop.top,
                (int)layer->sourceCrop.right,
                (int)layer->sourceCrop.bottom
            };

            sur = obtain_layer_surface(device, layer);

            dl = layer->displayFrame.left   - rect->left;
            dt = layer->displayFrame.top    - rect->top;
            dr = layer->displayFrame.right  - rect->right;
            db = layer->displayFrame.bottom - rect->bottom;

            if (layer->blt.hMirror) {
                int tl = dl;
                dl = -dr;
                dr = -tl;
            }

            if (layer->blt.vMirror) {
                int tt = dt;
                dt = -db;
                db = -tt;
            }

            /* Compute offset to source initial point. */
            switch (layer->blt.rotation) {
                case gcvSURF_0_DEGREE:
                default:
                    dx = sourceCrop.left - dl - marginRect->left;
                    dy = sourceCrop.top  - dt - marginRect->top;
                    break;
                case gcvSURF_90_DEGREE:
                    dx = sourceCrop.left + db - marginRect->bottom;
                    dy = sourceCrop.top  - dl - marginRect->left;
                    break;
                case gcvSURF_180_DEGREE:
                    dx = sourceCrop.left + dr - marginRect->right;
                    dy = sourceCrop.top  + db - marginRect->bottom;
                    break;
                case gcvSURF_270_DEGREE:
                    dx = sourceCrop.left - dt - marginRect->top;
                    dy = sourceCrop.top  + dr - marginRect->right;
                    break;
            }

            if (layer->blt.rotation == gcvSURF_90_DEGREE ||
                layer->blt.rotation == gcvSURF_180_DEGREE) {
                /* Swap width and height. */
                width  = fakeHeight;
                height = fakeWidth;
            }

            if (!blt->multiBltSourceRotation)
                rotation = gcvSURF_0_DEGREE;
            else
                rotation = layer->blt.rotation;

            __hwc2_trace_string("fakeSize=%dx%d rot=%d mirror=%d,%d",
                    width, height, rotation,
                    layer->blt.hMirror, layer->blt.vMirror);

            if (blt->multiBltSourceMirror) {
                /* Set mirror for current source. */
                gcmONERROR(gco2D_SetBitBlitMirror(blt->engine,
                            layer->blt.hMirror, layer->blt.vMirror));
            }

            /* Setup source. */
            program_fake_source(device, sur, dx, dy, rotation, width, height);

            gcmONERROR(gco2D_SetSource(blt->engine, &fakeRect));
            gcmONERROR(gco2D_SetROP(blt->engine, 0xCC, 0xCC));

            /* Try to set source address of previous no-source layers to
               current layer. */
            for (uint32_t j = 0; solidColorMask != 0; j++) {
                if ((solidColorMask & (1 << j)) == 0)
                    continue;

                gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, j));
                program_fake_source(device, sur, dx, dy, rotation, width, height);
                gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, sourceNum));

                solidColorMask &= ~(1 << j);
            }
        } else {
            __hwc2_trace_string("color32=0x%08x", layer->blt.color32);

            /* Color source is still needed if uses patthen only. We set dummy
             * color source to framebuffer target. */
            program_fake_source(device, target, 0, 0, gcvSURF_0_DEGREE,
                    fakeWidth, fakeHeight);

            gcmONERROR(gco2D_SetSource(blt->engine, &fakeRect));
            gcmONERROR(gco2D_SetROP(blt->engine, 0xF0, 0xCC));

            gcmONERROR(gco2D_LoadSolidBrush(blt->engine,
                    gcvSURF_UNKNOWN, gcvTRUE, layer->blt.color32, ~0ULL));

            /* Record solid color layer. */
            solidColorMask |= (1 << sourceNum);
        }

        int opaque = (sourceNum == 0) ? param->opaque : 0;
        program_layer_blend(device, layer, opaque);

        if (blt->multiBlt2) {
            /* Set target rectangle for each source. */
            gcmONERROR(gco2D_SetTargetRect(blt->engine, &fakeRect));
        }

        /* Append mask to sourceMask. */
        sourceMask = ((sourceMask << 1) | 1);
        sourceNum++;

        __hwc2_trace(1, "sourceMask=0x%X sourceNum=%u", sourceMask, sourceNum);
    }

    gcmONERROR(gco2D_SetClipping(blt->engine, &fakeRect));
    gcmONERROR(gco2D_MultiSourceBlit(blt->engine, sourceMask, &fakeRect, 1));

    /* Reset states. */
    gcmONERROR(gco2D_SetClipping(blt->engine, &dpy->blt.fullScreen));
    gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, 0));
    gcmVERIFY_OK(gcoSURF_Set2DTarget(dpy->blt.surface, gcvSURF_0_DEGREE));

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("Failed in %s: status=%d", __FUNCTION__, status);
    gcmVERIFY_OK(gco2D_SetClipping(blt->engine, &dpy->blt.fullScreen));
    gcmVERIFY_OK(gco2D_SetCurrentSourceIndex(blt->engine, 0));
    gcmVERIFY_OK(gcoSURF_Set2DTarget(dpy->blt.surface, gcvSURF_0_DEGREE));
    __hwc2_trace(1, "error");
}

/*
 * program_multi_blit_v2 - program multi blit batch, v2.
 *
 * v2 has 2 types:
 * 1. different source rectangles, and dest rectangles. Use clip rectangle to
 *    set required rect.
 * 2. different source rectangles, but shares the same dest rectangle. We
 *    calculate corresponding source rects, respectively.
 */
static void program_multi_blit_v2(__hwc2_device_t *device, __hwc2_display_t *dpy,
                    const hwc_rect_t *rect, uint64_t batchMask,
                    struct multi_blit_param *param)
{
    gceSTATUS status;

    struct blitter_device *blt = &device->blt;

    uint32_t i;
    uint64_t mask;
    uint32_t solidColorMask  = 0;
    uint32_t sourceMask = 0;
    uint32_t sourceNum  = 0;

    gcsRECT dstRect = {
        rect->left,  rect->top,
        rect->right, rect->bottom
    };

    __hwc2_trace(0, "batchMask=0x%" PRIx64, batchMask);

    for (i = 0, mask = 1; mask <= batchMask; i++, mask <<= 1) {
        if (!(batchMask &mask))
            continue;

        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
        __hwc2_trace(0, "layer=%p(%u)", layer, layer->id);

        /* Setup source index. */
        gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, sourceNum));

        if (layer->blt.surface) {
            if (blt->multiBlt2) {
                gcsRECT sourceCrop = {
                    (gctINT)ceilf (layer->blt.sourceCropD.left),
                    (gctINT)ceilf (layer->blt.sourceCropD.top),
                    (gctINT)floorf(layer->blt.sourceCropD.right),
                    (gctINT)floorf(layer->blt.sourceCropD.bottom)
                };
                gcsRECT displayFrame = {
                    layer->displayFrame.left,
                    layer->displayFrame.top,
                    layer->displayFrame.right,
                    layer->displayFrame.bottom,
                };

                gcmONERROR(gco2D_SetSource(blt->engine, &sourceCrop));
                gcmONERROR(gco2D_SetTargetRect(blt->engine, &displayFrame));
                gcmONERROR(gco2D_SetClipping(blt->engine, &dstRect));

                __hwc2_trace_string("rot=%d mirror=%d,%d",
                         layer->blt.rotation,
                         layer->blt.hMirror, layer->blt.vMirror);
            } else {
                gcsRECT srcRect;

                calculate_source_clip(layer, &dstRect, &srcRect);

                /* Set source rect. */
                gcmONERROR(gco2D_SetSource(blt->engine, &srcRect));

                __hwc2_trace_string("srcRect[%d,%d,%d,%d] rot=%d mirror=%d,%d",
                        srcRect.left, srcRect.top,
                        srcRect.right, srcRect.bottom,
                        layer->blt.rotation,
                        layer->blt.hMirror, layer->blt.vMirror);
            }

            gcmONERROR(gco2D_SetBitBlitMirror(blt->engine,
                    layer->blt.hMirror, layer->blt.vMirror));

            gcmONERROR(gcoSURF_Set2DSource(layer->blt.surface,
                    layer->blt.rotation));

            gcmONERROR(gco2D_SetROP(blt->engine, 0xCC, 0xCC));

            for (uint32_t j = 0; solidColorMask != 0U; j++) {
                if ((solidColorMask & (1 << j)) == 0)
                    continue;

                gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, j));
                gcmONERROR(gcoSURF_Set2DSource(
                        layer->blt.surface, layer->blt.rotation));

                gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, sourceNum));

                solidColorMask &= ~(1 << j);
            }
        } else {
            gcmONERROR(gcoSURF_Set2DSource(dpy->blt.surface, gcvSURF_0_DEGREE));

            gcmONERROR(gco2D_SetSource(blt->engine, &dstRect));
            gcmONERROR(gco2D_SetTargetRect(blt->engine, &dstRect));
            gcmONERROR(gco2D_SetClipping(blt->engine, &dstRect));

            gcmONERROR(gco2D_LoadSolidBrush(blt->engine,
                    gcvSURF_UNKNOWN, gcvTRUE, layer->blt.color32, ~0ULL));

            gcmONERROR(gco2D_SetROP(blt->engine, 0xF0, 0xCC));

            solidColorMask |= (1 << sourceNum);

            __hwc2_trace_string("color32=0x%08X", layer->blt.color32);
        }

        int opaque = (sourceNum == 0) ? param->opaque : 0;
        program_layer_blend(device, layer, opaque);

        /* Append mask to sourceMask. */
        sourceMask = ((sourceMask << 1U) | 1U);
        sourceNum++;

        __hwc2_trace(1, "sourceMask=0x%X sourceNum=%u", sourceMask, sourceNum);
    }

    if (blt->multiBlt2) {
        /* Target rectangles are separated given. */
        gcmONERROR(gco2D_MultiSourceBlit(
                blt->engine, sourceMask, gcvNULL, 0U));
    } else {
        /* Set clip rectangle. */
        gcmONERROR(gco2D_SetClipping(blt->engine, &dstRect));
        gcmONERROR(gco2D_MultiSourceBlit(blt->engine, sourceMask, &dstRect, 1));
    }

    /* Reset states. */
    gcmONERROR(gco2D_SetClipping(blt->engine, &dpy->blt.fullScreen));
    gcmONERROR(gco2D_SetCurrentSourceIndex(blt->engine, 0));

    __hwc2_trace(1, "done");
    return;

OnError:
    ALOGE("Failed in %s: status=%d", __FUNCTION__, status);
    __hwc2_trace(1, "error");
}

static inline void program_multi_blit(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, const hwc_rect_t *rect,
                        uint64_t batchMask, struct multi_blit_param *param)
{
    if (device->blt.multiBltSourceOffset)
        return program_multi_blit_v2(device, dpy, rect, batchMask, param);
    else
        return program_multi_blit_v1(device, dpy, rect, batchMask, param);
}

/*
 * blit_area_multi - Blit layers in the same area, with multi-blt enabled.
 *
 * Blit by area is used for multi-source blit, and optimize out blend-holes.
 * @batchMask:  Only blit layers specified in this batch.
 */
static void blit_area_multi(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, struct area *area,
                    uint64_t batchMask)
{
    uint32_t i = 0;
    uint64_t mask = 1;
    int32_t err;
    struct multi_blit_param param;

    __hwc2_trace(0, "area=[%d,%d,%d,%d] mask=0x%" PRIx64
        " batch=0x%" PRIx64 ": %s",
        area->rect.left, area->rect.top, area->rect.right, area->rect.bottom,
        area->layerMask, batchMask, bits_to_string(batchMask));

    while (mask <= batchMask) {
        if (!(mask & batchMask)) {
            i++;
            mask <<= 1;
            continue;
        }

        /* Blend-hole optimization, disable blend for bottom layer. */
        int opaque = ((mask - 1) & area->layerMask) == 0;
        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];

        init_multi_blit_batch(device, dpy, layer, &area->rect, opaque, &param);
        err = append_multi_blit_layer(device, dpy, layer, &area->rect, &param);

        if (!err) {
            uint32_t num = 1;
            uint64_t batch = mask;

            for (i++, mask <<= 1; mask <= batchMask; i++, mask <<= 1) {
                if (!(mask & batchMask)) continue;

                __hwc2_layer_t *ly = dpy->blt.bltLayers[i];
                err = append_multi_blit_layer(device, dpy, ly, &area->rect, &param);

                if (!err) {
                    batch |= mask;
                    num++;
                } else
                    break;
            }

            if (num > 1)
                program_multi_blit(device, dpy, &area->rect, batch, &param);
            else {
                /* Do single blit. */
                program_layer_blend(device, layer, opaque);
                blit_layer_rects(device, dpy, layer, &area->rect, 1);
            }

            /*
             * Try next batch.
             * Maybe current layer can do multi-blit with later ones.
             */
            continue;
        }

        /* Do single blit. */
        program_layer_blend(device, layer, opaque);
        blit_layer_rects(device, dpy, layer, &area->rect, 1);

        /* Advance to next layer. */
        i++;
        mask <<= 1;
    }

    __hwc2_trace(1, "done");
}

 /*
 * blit_area_single - Blit layers in the same area, only one source.
 *
 * Blit by area to optimize out blend-holes.
 * @batchMask:  Only blit layers specified in this batch.
 */
static void blit_area_single(__hwc2_device_t *device,
                    __hwc2_display_t *dpy, struct area *area,
                    uint64_t batchMask)
{
    uint32_t i;
    uint64_t mask;

    for (i = 0, mask = 1; mask <= batchMask; i++, mask <<= 1) {
        if (!(mask & batchMask))
            continue;

        __hwc2_layer_t *layer = dpy->blt.bltLayers[i];
        int opaque = ((mask - 1) & area->layerMask) == 0;

        /* Do single blit. */
        program_layer_blend(device, layer, opaque);
        blit_layer_rects(device, dpy, layer, &area->rect, 1);
    }
}

/*
 * blit_area - Blit by area.
 * Blit by area is used for multi-source blit, and optimize out blend-holes.
 * @batchMask:  Only blit layers specified in this batch.
 */
static inline void blit_area(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, struct area *area,
                        uint64_t batchMask)
{
    if (device->blt.multiBlt)
        blit_area_multi(device, dpy, area, batchMask);
    else
        blit_area_single(device, dpy, area, batchMask);
}

/*
 * find_recent_batch - Find recent layer batch since the most recent tough.
 * It returns mask for most tough layer +1 to @current (exclusive).
 */
static inline uint64_t find_recent_batch(__hwc2_display_t *dpy,
                            uint64_t layerMask, uint32_t current)
{
    /* Determine the recent tough layer. */
    uint64_t batch = 0;

    for (uint32_t recent = current - 1; recent <= 64; recent--) {
        uint64_t mask = 1ull << recent;

        if (layerMask & mask) {
            if (dpy->blt.toughMask & mask)
                return batch;

            batch |= mask;
        }
    }

    return batch;
}

/*
 * blit_sources - blit source layers.
 */
static void blit_sources(__hwc2_device_t *device, __hwc2_display_t *dpy)
{
    __hwc2_layer_t *layer;
    __hwc2_list_t *pos;

    uint32_t id;

    __hwc2_trace(0, "");

    for (id = 0; id < dpy->blt.numBltLayers; id++) {
        layer = dpy->blt.bltLayers[id];

        /* Find a tough layer. */
        if (!layer->blt.tough)
            continue;

        /* It's skyline. */
        if (layer->blt.topTiny)
            break;

        __hwc2_trace_string("tough layer=%p(%u) id=%u", layer, layer->id, id);

        __hwc2_list_for_each(pos, &dpy->blt.compAreaDamaged) {
            struct area *area = __hwc2_list_entry(pos, struct area, link);
            uint64_t batch;

            /* Reject area does not include this tough layer. */
            if (!(area->layerMask & (1ull << id)))
                continue;

            batch = find_recent_batch(dpy, area->layerMask, id);

            if (batch)
                blit_area(device, dpy, area, batch);
        }

        /* Blit the tough layer. */
        blit_layer(device, dpy, layer);
    }

    if (layer->blt.topTiny || id == dpy->blt.numBltLayers) {
        /* skyline at top-tiny layer or checked all layers. */
        __hwc2_trace_string("skyline id=%u", id);

        __hwc2_list_for_each(pos, &dpy->blt.compAreaDamaged) {
            struct area *area = __hwc2_list_entry(pos, struct area, link);
            uint64_t batch;

            batch = find_recent_batch(dpy, area->layerMask, id);

            if (batch)
                blit_area(device, dpy, area, batch);
        }
    }

    /* Blit top tiny layers. */
    while (id < dpy->blt.numBltLayers) {
        __hwc2_trace_string("top-tiny layer=%p(%u) id=%u", layer, layer->id, id);
        layer = dpy->blt.bltLayers[id++];
        blit_layer(device, dpy, layer);
    }

    __hwc2_trace(1, "done");
}

/* TODO: do in another thread */
static inline int32_t blit_display(__hwc2_device_t *device,
                        __hwc2_display_t *dpy, buffer_handle_t buffer)
{
    struct output *output = dpy->blt.output;

    /* Setup output buffer state. */
    program_output_buffer(device, dpy);

    /* Optimization, copy not damaged area from previous buffer. */
    program_copy_region(device, dpy);

    if (!output || __hwc2_list_empty(&output->damage)) {
        /* no damage, no more composition. */
        __hwc2_trace(2, "no damaged");
        return 0;
    }

    /* Clear wormholes. */
    program_clear_worm_hole(device, dpy);

    /* Blit source layers. */
    blit_sources(device, dpy);

    return 0;
}

int32_t __hwc2_queue_blit(__hwc2_device_t *device,
            __hwc2_display_t *dpy, buffer_handle_t buffer,
            int32_t releaseFence, int32_t *outAcquireFence)
{
    int32_t err = HWC2_ERROR_NONE;
    int32_t fence = -1;

    __hwc2_trace(0, "dpy=%p buffer=%p releaesFence=%d",
        dpy, buffer, releaseFence);

    revalidate_display(device, dpy, buffer);

    if (!buffer) {
        /* No output buffer. */
        *outAcquireFence = -1;

        /* Tricky case: with sources but no target. */
        release_fences_quirk(device, dpy);

        __hwc2_trace(1, "no output");
        /* Return 1 for skip. */
        return 1;
    }

    enter_blitter_hardware(device);

    acquire_output_buffer(device, dpy, buffer, releaseFence);
    acquire_sources(device, dpy);

    /* Do blit the display. */
    err = blit_display(device, dpy, buffer);
    commit_blit(device, &fence);

    /* Release source and output. */
    release_sources(device, dpy, fence);
    release_output_buffer(device, dpy);

    leave_blitter_hardware(device);

    *outAcquireFence = fence;

    __hwc2_trace(1, "return=%d", err);
    return err;
}

