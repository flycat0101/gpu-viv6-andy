#include <gc_hal.h>
#include <gc_hal_user.h>

#include <gralloc_handle.h>
#include <graphics_ext.h>

#include <log/log.h>

#include <drm.h>
#include <vivante_drm.h>
#include <vivante_bo.h>
#include <gralloc_vivante_bo.h>
#include <gralloc_handle.h>

#ifndef ARRAY_SIZE
#  define ARRAY_SIZE(a)    (sizeof(a)/sizeof((a)[0]))
#endif

static const struct {
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;

    int luminanceSize;

    gceSURF_FORMAT halFormat;
}
pixel_formats[] =
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

static gceSURF_FORMAT
translate_format(buffer_handle_t handle)
{
    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TILED_VIV) {
        /* Use internal format for direct rendering. */
        switch (gralloc_handle_format(handle)) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_PE_A8B8G8R8) ? gcvSURF_A8B8G8R8 : gcvSURF_A8R8G8B8;

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
                  __func__, gralloc_handle_format(handle));
            return gcvSURF_UNKNOWN;
        }
    } else if (gralloc_handle_format(handle) > 0 &&
            gralloc_handle_format(handle) < (int)ARRAY_SIZE(pixel_formats)) {
        return pixel_formats[gralloc_handle_format(handle)].halFormat;
    } else {
        switch (gralloc_handle_format(handle)) {
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
#endif
            return gcvSURF_NV12;
#endif
        default:
            ALOGE("%s: unknown android format=%x",
                __func__, gralloc_handle_format(handle));
            return gcvSURF_UNKNOWN;
        }
    }
}

static int get_vivante_drm_buffer_type(struct gralloc_vivante_bo_t * bo,
                struct drm_vivante_bo_tiling * tilingArgs,
                gceSURF_TYPE *pType)
{
    int err;
    gceSURF_TYPE type;

    /* GPU buffers. Get tiling information. */
    err = drm_vivante_bo_get_tiling(bo->bo, tilingArgs);

    if (err) {
        ALOGE("%s: failed to get bo tiling", __func__);
        return err;
    }

    /* Determine surface type for GPU buffers from bo tiling. */
    switch (tilingArgs->tiling_mode) {
    case DRM_VIV_GEM_TILING_LINEAR:
        type = gcvSURF_BITMAP;
        break;

    case DRM_VIV_GEM_TILING_TILED:
        type = gcvSURF_TEXTURE;
        break;

    case DRM_VIV_GEM_TILING_SUPERTILED:
        type = (tilingArgs->ts_mode == DRM_VIV_GEM_TS_NONE) ?
               gcvSURF_RENDER_TARGET_NO_TILE_STATUS : gcvSURF_RENDER_TARGET;
        break;

    default:
        ALOGE("%s: unsupported tiling_mode: %d",
              __func__, tilingArgs->tiling_mode);
        return -EINVAL;
    }


    *pType = type;
    return 0;
}

static inline int get_generic_drm_buffer_type(buffer_handle_t handle,
                        gceSURF_TYPE *pType)
{
    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TILED_VIV) {
        if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TS_VIV)
            *pType = gcvSURF_RENDER_TARGET;
        else
            *pType = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
    } else
        *pType = gcvSURF_BITMAP;

    return 0;
}

static int create_vivante_drm_buffer_surface(buffer_handle_t handle,
                struct gralloc_vivante_bo_t * bo, gceSURF_FORMAT format,
                gcoSURF * pSurface
    )
{
    gceSTATUS status;
    gcoSURF surface = gcvNULL;
    gceSURF_TYPE type;
    struct drm_vivante_bo_tiling tilingArgs;
    uint32_t node = 0, tsNode = 0;
    uint64_t param;
    gcePOOL pool;
    gctSIZE_T size;
    int err;

    err = get_vivante_drm_buffer_type(bo, &tilingArgs, &type);

    if (err)
        return err;

    /* Get pool type. */
    err = drm_vivante_bo_query(bo->bo, DRM_VIV_GEM_PARAM_POOL, &param);

    if (err)
        return err;

    pool = (gcePOOL)param;

    err = drm_vivante_bo_query(bo->bo, DRM_VIV_GEM_PARAM_SIZE, &param);

    if (err)
        return err;

    size = (gctSIZE_T)param;

    /* Indicate display buffer. */
    type |= gcvSURF_CREATE_AS_DISPLAYBUFFER;

    /* Append no-vidmem hint to type. */
    type |= gcvSURF_NO_VIDMEM;

    /* Create surface wraper. */
    gcmONERROR(gcoSURF_Construct(gcvNULL,
                                 gralloc_handle_width(handle),
                                 gralloc_handle_height(handle),
                                 1, type, format, (gcePOOL)pool,
                                 &surface));

    /* Reference and get node. */
    err = drm_vivante_bo_ref_node(bo->bo, &node, &tsNode);

    if (err)
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);

    /* Assign parameters to surface. */
    surface->size = size;

    surface->node.u.normal.node = node;
    surface->node.pool          = pool;
    surface->node.size          = size; /* ??? */

#if gcdENABLE_3D
    if (tsNode != 0)
    {
        surface->tileStatusNode.u.normal.node = tsNode;
        surface->tileStatusNode.pool          = pool; /* ??? */
        surface->tileStatusNode.size          = size >> 8; /* ??? */
    }
#endif

    /* Initial lock. */
    gcmONERROR(gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    /* Set y-inverted rendering. */
    gcmONERROR(gcoSURF_SetFlags(surface,
                                gcvSURF_FLAG_CONTENT_YINVERTED,
                                gcvTRUE));

    /* Corrent tile status state. */
    surface->tileStatusDisabled[0] =
            (tilingArgs.ts_mode == DRM_VIV_GEM_TS_NONE) ||
            (tilingArgs.ts_mode == DRM_VIV_GEM_TS_DISABLED);

    surface->fcValue[0]      = (gctUINT32)tilingArgs.clear_value;
    surface->fcValueUpper[0] = (gctUINT32)(tilingArgs.clear_value >> 32);

    *pSurface = surface;
    return 0;

OnError:
    if (surface)
        gcoSURF_Destroy(surface);

    return -EINVAL;
}

static int create_generic_drm_buffer_surface(buffer_handle_t handle,
                gceSURF_FORMAT format, gcoSURF * pSurface)
{
    gceSTATUS status;
    gceSURF_TYPE type;
    gctUINT stride = gralloc_handle_stride(handle);
    gcoSURF surface = gcvNULL;
    int fd;

    get_generic_drm_buffer_type(handle, &type);

    /* Translate stride in bytes to in pixels. */
    switch (format)
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
        ALOGE("%s: unknown format=%x", __func__, format);
        *pSurface = gcvNULL;
        return -EINVAL;
    }

    fd = gralloc_handle_fd(handle);

    if (fd < 0)
    {
        ALOGE("%s: invalid fd=%d", __func__, fd);
        return -EINVAL;
    }

    /* Indicate display buffer. */
    type |= gcvSURF_CREATE_AS_DISPLAYBUFFER;


    /* TODO: Attach tile status from generic drm buffer. */
    status = gcoSURF_WrapUserMemory(gcvNULL,
                                    gralloc_handle_width(handle),
                                    gralloc_handle_height(handle),
                                    stride,
                                    1,
                                    type,
                                    format,
                                    (gctUINT32)fd,
                                    gcvALLOC_FLAG_DMABUF,
                                    &surface);

    if (gcmIS_SUCCESS(status))
    {
        gcoSURF_SetFlags(surface,
                         gcvSURF_FLAG_CONTENT_YINVERTED,
                         gcvTRUE);
    }

    *pSurface = surface;
    return gcmIS_SUCCESS(status) ? 0 : -EINVAL;
}

int gralloc_buffer_create_surface(buffer_handle_t handle,
            gcoSURF *pSurface)
{
    gcoSURF surface;
    gceSURF_FORMAT format;
    struct gralloc_vivante_bo_t * bo;
    int err;

    format = translate_format(handle);

    if (format == gcvSURF_UNKNOWN) {
        ALOGE("%s: unknown buffer format", __func__);
        return -EINVAL;
    }

    err = gralloc_handle_validate(handle);
    if (err) {
        ALOGE("%s: invalid buffer=%p", __func__, handle);
        return err;
    }

    /* Try get gralloc_vivante_bo. */
    bo = gralloc_vivante_bo_from_handle(handle);

    if (bo)
        /* Create drm buffer by vivante drm interface. */
        err = create_vivante_drm_buffer_surface(handle, bo, format, &surface);
    else
        /* Create drm buffer by generic drm interface. */
        err = create_generic_drm_buffer_surface(handle, format, &surface);

    if (!err)
        *pSurface = surface;

    return err;
}

static int pop_vivante_drm_buffer_status(
                struct gralloc_vivante_bo_t * bo, gcoSURF surface,
                uint64_t *pTimeStamp)
{
    int err;
    struct drm_vivante_bo_tiling tilingArgs;
    uint64_t timeStamp = 0;

    err = drm_vivante_bo_get_tiling(bo->bo, &tilingArgs);

    if (err)
        return err;

    err = drm_vivante_bo_get_timestamp(bo->bo, &timeStamp);

    if (err)
        return err;

    if (timeStamp == *pTimeStamp)
        /* surface is not updated. */
        return 1;

    *pTimeStamp = timeStamp;

    /* Update tile status information. */
    switch (tilingArgs.ts_mode)
    {
    case DRM_VIV_GEM_TS_COMPRESSED:
        /* Surface->compressed = gcvTRUE; */
    case DRM_VIV_GEM_TS_NORMAL:
        surface->tileStatusDisabled[0] = gcvFALSE;
        surface->fcValue[0]      = (gctUINT32)tilingArgs.clear_value;
        surface->fcValueUpper[0] = (gctUINT32)(tilingArgs.clear_value >> 32);
        break;
    case DRM_VIV_GEM_TS_DISABLED:
        surface->tileStatusDisabled[0] = gcvTRUE;
        break;
    default:
        break;
    }

    return 0;
}

static int pop_generic_drm_buffer_status(buffer_handle_t handle,
                gcoSURF surface, uint64_t *pTimeStamp)
{
    (void)handle;
    (void)surface;

    if (pTimeStamp)
        (*pTimeStamp)++;

    return 0;
}

int gralloc_buffer_sync_surface(buffer_handle_t handle,
            gcoSURF surface, uint64_t *pTimeStamp)
{
    int err;
    struct gralloc_vivante_bo_t * bo;
    uint64_t timeStamp = 0;

    if (pTimeStamp)
        timeStamp = *pTimeStamp;

    bo = gralloc_vivante_bo_from_handle(handle);

    if (bo)
        err = pop_vivante_drm_buffer_status(bo, surface, &timeStamp);
    else
        err = pop_generic_drm_buffer_status(handle, surface, &timeStamp);

    if (pTimeStamp)
        *pTimeStamp = timeStamp;

    return err;
}

/*
 * Push surface tiling information to bo.
 * Return EGL_TRUE on success, EGL_FALSE on failure.
 */
static int push_vivante_drm_buffer_status(gcoSURF surface,
                struct gralloc_vivante_bo_t *bo)
{
    struct drm_vivante_bo_tiling tilingArgs;
    uint64_t timeStamp = 0;
    gceTILING tiling = gcvINVALIDTILED;

    gcoSURF_GetTiling(surface, &tiling);

    switch (tiling) {
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
        return -EINVAL;
    }

    tilingArgs.ts_mode = surface->tileStatusNode.pool == gcvPOOL_UNKNOWN ? DRM_VIV_GEM_TS_NONE
                       : surface->tileStatusDisabled[0] ? DRM_VIV_GEM_TS_DISABLED
                       : surface->compressed ? DRM_VIV_GEM_TS_COMPRESSED
                       : DRM_VIV_GEM_TS_NORMAL;

    tilingArgs.clear_value = ((uint64_t)surface->fcValueUpper[0]) << 32
                           | surface->fcValue[0];

    drm_vivante_bo_set_tiling(bo->bo, &tilingArgs);
    drm_vivante_bo_inc_timestamp(bo->bo, &timeStamp);
    (void)timeStamp;

    return 0;
}

/*
 * Push surface tiling information to drm buffer.
 * Return EGL_TRUE on success, EGL_FALSE on failure.
 */
static int push_generic_drm_buffer_status(gcoSURF surface,
                buffer_handle_t handle)
{
    /*XXX:
     * For generic dma buffer, imx8mscale temporilary put ts at the end of
     * master buffer.
     */
    if (gralloc_handle_usage(handle) & GRALLOC_USAGE_TS_VIV) {
        gctINT32 stride;
        gctUINT32 height;
        gctUINT32 size;

        gctPOINTER memory[3];
        gctPOINTER ts;

        gcoSURF_GetAlignedSize(surface, gcvNULL, &height, &stride);
        size = stride * height;

        gcoSURF_Lock(surface, gcvNULL, memory);

        ts = (gctUINT8_PTR)memory[0] + size;
        memcpy(ts, surface->tileStatusNode.logical, surface->tileStatusNode.size);

        gcoSURF_Unlock(surface, memory[0]);
    }

    return 0;
}

int gralloc_buffer_sync_from_surface(buffer_handle_t handle, gcoSURF surface)
{
    int err;
    struct gralloc_vivante_bo_t * bo;

    bo = gralloc_vivante_bo_from_handle(handle);

    if (bo)
        /* GPU-buffer. */
        err = push_vivante_drm_buffer_status(surface, bo);
    else
        err = push_generic_drm_buffer_status(surface, handle);

    return err;
}
