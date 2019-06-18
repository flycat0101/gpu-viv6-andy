/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems. All Rights Reserved.
 * Copyright 2016, Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/slogcodes.h>
#include <pthread.h>

#include <screen/blit.h>
#include <screen/screen.h>
#include <KHR/khronos_utils.h>

#include "g2dExt.h"

#include <gc_hal.h>
#include <gc_hal_raster.h>
#include <gc_hal_engine.h>

#include <time.h>

#if 0
/* QNX MOD xsun: vivante don't provide this file for imx6x but there is one from mmp2 (now mmp3)
   Tarang confirmed that it is not required for now */
#include <gc_hal_egl.h>
#else
#include <EGL/egl.h>
#include "gc_hal_types.h"
#endif

#include <WF/wfd.h>
#include <WF/wfdext.h>

#include <assert.h>

#include <WF/private/imx8_common_wfd_g2d.h>

/* XXX Move to sys/slogcodes.h */
#ifndef _SLOGC_GRAPHICS_WINMGR
#define _SLOGC_GRAPHICS_WINMGR  __C(_SLOGC_GRAPHICS, 300)
#endif

#define error(format...) slogf(_SLOGC_GRAPHICS_WINMGR, _SLOG_ERROR, "gpu/vivante: " format)
#define info(format...) slogf(_SLOGC_GRAPHICS_WINMGR, _SLOG_INFO, "gpu/vivante: " format)

#define VIVANTE_MODULE_MUTINIT \
    pthread_mutex_init(&blt_ctx->mutex, NULL)

#define VIVANTE_MODULE_LOCK \
    pthread_mutex_lock(&blt_ctx->mutex)

#define VIVANTE_MODULE_UNLOCK \
    pthread_mutex_unlock(&blt_ctx->mutex)

#define VIVANTE_MODULE_MUTDEINIT \
    pthread_mutex_destroy(&blt_ctx->mutex)

#define VIVANTE_LOGERR_CALL_GC(_func, _params) \
    gc_rc = _func _params; \
    if (gc_rc != gcvSTATUS_OK) { \
        error("%s[%d]: %s failed (%d %s)", __FUNCTION__, __LINE__, #_func, gc_rc, gc_rc_2_a(gc_rc)); \
    }

#define G2D_LOGERR_CALL_GC(_func, _params) \
    gc_rc = _func _params; \
    if (gc_rc != 0) { \
        error("%s[%d]: %s failed (%d)", __FUNCTION__, __LINE__, #_func, gc_rc); \
    }

#define	IS_SCALED( args )    ( (args).sw != (args).dw || (args).sh != (args).dh)

typedef struct {
        struct g2d_surfaceEx G2DSurfaceEx;
        gctPOINTER      MappingInfo;
        win_image_t    *NativeImage;
} vivante_handle_t;

/* This is the maximum for SetCurrentSourceIndex(), and a multisource blit */
/* TODO This value shall be taken from G2D */
#define MAX_PER_BATCH   8

/* save up to this number of "homogeneous" blits */
#define MAX_BLIT_OPS    32
typedef struct blit_op {
    vivante_handle_t        *src;
    vivante_handle_t        *dst;
    win_blit_t              args;
} blit_op_t;

struct win_blit_ctx {
	void		*G2Dhandle;		/* The structure of the G2D context is defined in the g2d.c.*/
    pthread_mutex_t mutex;

    /* Create a thread so we can use its delicious TLS */
    pthread_t		tls_thread;
    bool		tls_constructed;
    int			tls_errno;
    pthread_barrier_t	tls_barrier;

    int         multisource;
    int         print_stats;    /* 0 -- no stats, 1 -- totals, 2... -- debugging messages */
    unsigned    n_blits;
    struct g2d_surface_pair  blits[MAX_BLIT_OPS];
};

#if defined(MULTIBLIT)
static void do_blits( struct win_blit_ctx * );
#endif

#if defined (gcdMMUV2_ENABLE)
#error "gcdMMUV2 is not supported!"
#endif

/*
static int
gc_rc_2_errno(int gc_rc)
{
    switch (gc_rc) {
    case gcvSTATUS_OK:
        return EOK;
    case gcvSTATUS_INVALID_ARGUMENT:
    case gcvSTATUS_INVALID_OBJECT:
    case gcvSTATUS_INVALID_DATA:
        return EINVAL;
    case gcvSTATUS_OUT_OF_MEMORY:
        return ENOMEM;
    case gcvSTATUS_MEMORY_LOCKED:
    case gcvSTATUS_MEMORY_UNLOCKED:
        return EACCES;
    case gcvSTATUS_GENERIC_IO:
        return EIO;
    case gcvSTATUS_INVALID_ADDRESS:
    case gcvSTATUS_NOT_ALIGNED:
            return EFAULT;
    case gcvSTATUS_NOT_SUPPORTED:
        return ENOTSUP;
    case gcvSTATUS_OUT_OF_RESOURCES:
        return EAGAIN;
    case gcvSTATUS_INVALID_REQUEST:
        return ECANCELED;
    default:
        return !EOK;
    }
}


#define gc_rc_2_a_line(x) do { if (gc_rc == (x)) return #x; } while (0)
static const char *
gc_rc_2_a(int gc_rc)
{
	gc_rc_2_a_line(gcvSTATUS_OK);
	gc_rc_2_a_line(gcvSTATUS_INVALID_ARGUMENT);
	gc_rc_2_a_line(gcvSTATUS_INVALID_OBJECT);
	gc_rc_2_a_line(gcvSTATUS_INVALID_DATA);
	gc_rc_2_a_line(gcvSTATUS_OUT_OF_MEMORY);
	gc_rc_2_a_line(gcvSTATUS_MEMORY_LOCKED);
	gc_rc_2_a_line(gcvSTATUS_MEMORY_UNLOCKED);
	gc_rc_2_a_line(gcvSTATUS_GENERIC_IO);
	gc_rc_2_a_line(gcvSTATUS_INVALID_ADDRESS);
	gc_rc_2_a_line(gcvSTATUS_NOT_ALIGNED);
	gc_rc_2_a_line(gcvSTATUS_NOT_SUPPORTED);
	gc_rc_2_a_line(gcvSTATUS_OUT_OF_RESOURCES);
	gc_rc_2_a_line(gcvSTATUS_INVALID_REQUEST);
	return "unknown";
}
#undef gc_rc_2_a_line
*/
#define CHECK_PTHREAD_ERR(x) do { \
	int rv = (x); \
	if (rv) { \
		error("%s:%d: %s: %s", __FUNCTION__, __LINE__, #x, strerror(rv)); \
	} \
} while(0)

#define CHECK_BARRIER_ERR(x) do { \
	int rv = (x); \
	if (rv && rv != PTHREAD_BARRIER_SERIAL_THREAD) { \
		error("%s:%d: %s: %s", __FUNCTION__, __LINE__, #x, strerror(rv)); \
	} \
} while(0)

#define FAIL_PTHREAD_ERR(x) do { \
	int rv = (x); \
	if (rv) { \
		error("%s:%d: %s: %s", __FUNCTION__, __LINE__, #x, strerror(rv)); \
		goto fail; \
	} \
} while(0)

static int
vivante_tls_thread_fn_init(struct win_blit_ctx *blt_ctx)
{
    gceSTATUS rc;
    int g2d_value;

    // "constructed" means "we executed the constructor function," not
    // "the blt_ctx is in a usable state."
    if (blt_ctx->tls_constructed) {
    	error("vivante_tls_thread_fn_init already called!");
        return ENOMEM;
    } else {
	blt_ctx->tls_constructed = true;
    }

	ThreadCtl( _NTO_TCTL_IO, 0 );

    if(g2d_open(&blt_ctx->G2Dhandle) == -1 || blt_ctx->G2Dhandle == NULL) {
    	error("Fail to open g2d device!\n");
    	return EIO;
    }

    blt_ctx->multisource = 1;

    char val[30];
    const char *key = "blit-multisource-disable";

    rc = __khrGetDisplayConfigValue( 1, key, val, sizeof(val) );
    if(rc == EOK) {
        val[sizeof(val)-1] = 0;
        if (strlen(val) <= sizeof val - 2) {
            blt_ctx->multisource = atoi( val ) ? 0 : 1;
        } else {
            error( "'%s' value too long; ignored", key );
        }
    }

    if( blt_ctx->multisource ) {
        /* We want multiblit, confirm if this possible on this hardware */
    	rc = g2d_query_feature(blt_ctx->G2Dhandle, G2D_MULTI_SOURCE_BLT, &g2d_value);
    	if (rc < 0){
    		error("g2d_query_feature failed.\n");
    		return EINVAL;
    	}
    	blt_ctx->multisource = g2d_value;


        /* NOTES Is the chip identity really necessary? */
 /*       rc = gcoHAL_QueryChipIdentity( blt_ctx->hal,
            &blt_ctx->chipModel, &blt_ctx->chipRevision,
            &blt_ctx->chipFeatures, &blt_ctx->chipMinorFeatures );

        if (rc < 0) {
            error("%s: gcoHAL_QueryChipIdentity() failed, [status:%d]", __FUNCTION__, rc );
            return gc_rc_2_errno(rc);
        }
 */
    }

    key = "blit-stats";
    rc = __khrGetDisplayConfigValue( 1, key, val, sizeof(val));
    if(rc == EOK) {
        val[sizeof(val)-1] = 0;
        if (strlen(val) <= sizeof val - 2) {
            blt_ctx->print_stats = atoi(val);
        } else {
            error( "'%s' value too long; ignored", key );
        }
    }

    if( blt_ctx->print_stats ) {
    	rc = g2d_query_feature(blt_ctx->G2Dhandle, G2D_MULTI_SOURCE_BLT, &g2d_value);
    	info("gcvFEATURE_2D_MULTI_SOURCE_BLT=%s\n", g2d_value ? "true" : "false");
    }

    return EOK;
}

static
void
vivante_tls_thread_fn_fini(struct win_blit_ctx *blt_ctx)
{
    gceSTATUS gc_rc;

    // We are in the destructor, don't call us again.
    if (!blt_ctx->tls_constructed) {
    	error("vivante_tls_thread_fn_fini already called or vivante_tls_thread_fn_init not called!");
	return;
    } else {
	blt_ctx->tls_constructed = false;
    }
    if(blt_ctx->G2Dhandle) {
    	G2D_LOGERR_CALL_GC(g2d_close, (blt_ctx->G2Dhandle));
        blt_ctx->G2Dhandle = NULL;
    }
}

static void*
vivante_tls_thread_fn(void *arg)
    {
	struct win_blit_ctx *blt_ctx = arg;

	CHECK_PTHREAD_ERR(pthread_setname_np(pthread_self(), "vivante 2D TLS thread"));

	// Run the original init code
	blt_ctx->tls_errno = vivante_tls_thread_fn_init(blt_ctx);

	// slog errors here, but do nothing about them in this thread
	if (blt_ctx->tls_errno != EOK) {
		error("TLS thread failed: %s", strerror(blt_ctx->tls_errno));
    }

	// Init finished, wake up init function in other thread
    CHECK_BARRIER_ERR(pthread_barrier_wait(&blt_ctx->tls_barrier));

	// Wait for some other thread to call the fini function
    CHECK_BARRIER_ERR(pthread_barrier_wait(&blt_ctx->tls_barrier));

	// Run the original fini code
	vivante_tls_thread_fn_fini(blt_ctx);

	return NULL;
}

static int
vivante_tls_thread_create(struct win_blit_ctx *blt_ctx)
{
	FAIL_PTHREAD_ERR(pthread_barrier_init(&blt_ctx->tls_barrier, NULL, 2));

	FAIL_PTHREAD_ERR(pthread_create(&blt_ctx->tls_thread, NULL, vivante_tls_thread_fn, blt_ctx));

	CHECK_BARRIER_ERR(pthread_barrier_wait(&blt_ctx->tls_barrier));

	if (blt_ctx->tls_constructed) {
		return blt_ctx->tls_errno;
	}

	blt_ctx->tls_errno = 0;
	return 0;

fail:
	error("create TLS thread failed");
	return -1;
}

static int
vivante_tls_thread_destroy(struct win_blit_ctx *blt_ctx)
{
	if (blt_ctx->tls_constructed) {
		CHECK_BARRIER_ERR(pthread_barrier_wait(&blt_ctx->tls_barrier));
		CHECK_PTHREAD_ERR(pthread_join(blt_ctx->tls_thread, NULL));
		if (blt_ctx->tls_constructed) {
			error("TLS thread failed to run destructor?");
		}
	}

	CHECK_PTHREAD_ERR(pthread_barrier_destroy(&blt_ctx->tls_barrier));
	return 0;
}

/**
 * Create bliter context.
 * @param ctx [OUT] Pointer where the pointer to the context will be stored.
 * @return Error value.
 */
int vivante_ctx_init(win_blit_ctx_t *ctx)
{
    struct win_blit_ctx *blt_ctx;

    blt_ctx = calloc(1, sizeof(*blt_ctx));
    if (!blt_ctx) {
    	error("%s: memory allocation failure", __FUNCTION__);
        return ENOMEM;
    }

    FAIL_PTHREAD_ERR(pthread_mutex_init(&blt_ctx->mutex, NULL));

    if (vivante_tls_thread_create(blt_ctx)) {
        error("%s: thread creation failure", __FUNCTION__);
	vivante_tls_thread_destroy(blt_ctx);
        return ENOMEM;
    }

    *ctx = blt_ctx;

    // Return value from original vivante_ctx_init
    return blt_ctx->tls_errno;

fail:
    free(blt_ctx);
    return -1;
}
/**
 * Destroys the blitter context.
 *
 * @param ctx [IN] Blitter context to destroy.
 */
static void
vivante_ctx_fini(win_blit_ctx_t ctx)
{
    struct win_blit_ctx *blt_ctx   = (struct win_blit_ctx *)ctx;

    // XXX: what purpose does this serve?
    pthread_mutex_lock(&blt_ctx->mutex);
    pthread_mutex_unlock(&blt_ctx->mutex);

    vivante_tls_thread_destroy(blt_ctx);

    CHECK_PTHREAD_ERR(pthread_mutex_destroy(&blt_ctx->mutex));

    memset(ctx, 0, sizeof(struct win_blit_ctx));
    free(ctx);
}

#define G2D_FORMAT_NOT_SUPPORTED(format)		\
		do {										\
			error("%s: format %s not supported", __FUNCTION__, #format);	\
			return -1; 							\
		}while(0)

/**
 * Updates the info in the g2d surface according to the NativeImage.
 * @param handle[IN/OUT] Screen handle.
 * @retval 0        Success, image format supported by g2d.
 * @retval -1       Required screen image format not supported by g2d.
 */
static int
update_g2d_surface(vivante_handle_t *handle) {

        switch (GET_TILING_MODE(handle->NativeImage->format)) {
        case WFD_FORMAT_IMX8X_TILING_MODE_LINEAR:
            handle->G2DSurfaceEx.tiling = G2D_LINEAR;
            break;
        case WFD_FORMAT_IMX8X_TILING_MODE_VIVANTE_TILED:
            handle->G2DSurfaceEx.tiling = G2D_TILED;
            break;
        case WFD_FORMAT_IMX8X_TILING_MODE_VIVANTE_SUPER_TILED:
            handle->G2DSurfaceEx.tiling = G2D_SUPERTILED;
            break;
        case WFD_FORMAT_IMX8X_TILING_MODE_AMPHION_TILED:
            handle->G2DSurfaceEx.tiling = G2D_AMPHION_TILED;
            break;
        case WFD_FORMAT_IMX8X_TILING_MODE_AMPHION_INTERLACED:
            handle->G2DSurfaceEx.tiling = G2D_AMPHION_INTERLACED;
            break;
        default:
            error("%s: unknown tiling format - 0x%X!", __FUNCTION__, GET_TILING_MODE(handle->NativeImage->format));
            return -1;
            break;
        }

	switch (WIN_FORMAT(handle->NativeImage->format)) {
	case SCREEN_FORMAT_BYTE:
		// gcvSURF_L8 - NOTES not BYTE format supported by g2d??
		G2D_FORMAT_NOT_SUPPORTED(BYTE);
		break;
	case SCREEN_FORMAT_RGBA4444:
		// gcvSURF_A4R4G4B4 - NOTES RGBA4444 format not supported by g2d??
		G2D_FORMAT_NOT_SUPPORTED(RGBA4444);
		break;
	case SCREEN_FORMAT_RGBX4444:
		// gcvSURF_X4R4G4B4 - NOTES RGBX4444 format not supported by g2d??
		G2D_FORMAT_NOT_SUPPORTED(RGBX4444);
		break;
	case SCREEN_FORMAT_RGBA5551:
		// gcvSURF_A1R5G5B5 - NOTES RGBA5551 format not supported by g2d??
		G2D_FORMAT_NOT_SUPPORTED(RGBA5551);
		break;
	case SCREEN_FORMAT_RGBX5551:
		// gcvSURF_X1R5G5B5 - NOTES RGBA5551 format not supported by g2d??
		G2D_FORMAT_NOT_SUPPORTED(RGBA5551);
		break;
	case SCREEN_FORMAT_RGB565:
		handle->G2DSurfaceEx.base.format = G2D_RGB565;
		break;
	case SCREEN_FORMAT_RGB888:
		//gcvSURF_R8G8B8 - NOTES RGB888 format not supported by g2d??
		handle->G2DSurfaceEx.base.format = G2D_RGB888;
		break;
	case SCREEN_FORMAT_RGBA8888:
		handle->G2DSurfaceEx.base.format = G2D_BGRA8888;
		break;
	case SCREEN_FORMAT_RGBX8888:
		handle->G2DSurfaceEx.base.format = G2D_BGRX8888;
		break;
	case SCREEN_FORMAT_UYVY:
		handle->G2DSurfaceEx.base.format = G2D_UYVY;
		break;
	case SCREEN_FORMAT_YUY2:
		handle->G2DSurfaceEx.base.format = G2D_YUYV;
		break;
	case SCREEN_FORMAT_YV12:
		handle->G2DSurfaceEx.base.format = G2D_YV12;
		break;
	case SCREEN_FORMAT_YUV420:
		handle->G2DSurfaceEx.base.format = G2D_I420;
		break;
	case SCREEN_FORMAT_YVYU:
		handle->G2DSurfaceEx.base.format = G2D_YVYU;
		break;
	case SCREEN_FORMAT_NV12:
		handle->G2DSurfaceEx.base.format = G2D_NV12;
		break;
	default:
		error("%s: format %d not supported", __FUNCTION__, handle->NativeImage->format);
		return -1;
		break;
	}

	/* Update stride of the g2d surface structure */
	switch(handle->G2DSurfaceEx.base.format)
	{
	case G2D_RGB565:
	case G2D_BGR565:
	case G2D_YUYV:
	case G2D_UYVY:
	case G2D_YVYU:
		handle->G2DSurfaceEx.base.stride = handle->NativeImage->strides[0]/2;
		break;
	case G2D_RGB888:
		handle->G2DSurfaceEx.base.stride = handle->NativeImage->strides[0]/3;
		break;
	case G2D_RGBA8888:
	case G2D_RGBX8888:
	case G2D_BGRA8888:
	case G2D_BGRX8888:
	case G2D_ARGB8888:
	case G2D_ABGR8888:
	case G2D_XRGB8888:
	case G2D_XBGR8888:
		handle->G2DSurfaceEx.base.stride = handle->NativeImage->strides[0]/4;
		break;
        case G2D_NV12:
        case G2D_NV16:
        case G2D_YV12:
        case G2D_I420:
        handle->G2DSurfaceEx.base.stride = handle->NativeImage->strides[0];
		break;
	default:
		break;
	}

	/* Update planes part of the g2d surface */
        handle->G2DSurfaceEx.base.planes[0] = handle->NativeImage->paddr;

	switch (handle->G2DSurfaceEx.base.format) {
	case G2D_RGB565:
	case G2D_RGB888:
	case G2D_RGBA8888:
	case G2D_RGBX8888:
	case G2D_BGRA8888:
	case G2D_BGRX8888:
	case G2D_BGR565:
	case G2D_YUYV:
	case G2D_UYVY:
	case G2D_YVYU:
		// no change
		break;
	case G2D_NV12:
	case G2D_NV16:
                handle->G2DSurfaceEx.base.planes[1] = handle->G2DSurfaceEx.base.planes[0] + handle->NativeImage->planar_offsets[1];
		break;
	case G2D_YV12:
	case G2D_I420:
		handle->G2DSurfaceEx.base.planes[1] = handle->G2DSurfaceEx.base.planes[0] + handle->NativeImage->planar_offsets[1];
		handle->G2DSurfaceEx.base.planes[2] = handle->G2DSurfaceEx.base.planes[0] + handle->NativeImage->planar_offsets[2];
		break;
	default:
		break;
	}

	return EOK;
}


void printbuff(const char *message,  win_image_t *image) {
  uint32_t *vaddr = image->vaddr;
  
  info("%s paddr 0x%08lx buffer: %08x %08x %08x %08x %08x %08x %08x %08x", message, image->paddr,
	vaddr[0], vaddr[1], vaddr[2], vaddr[3], vaddr[4], vaddr[5], vaddr[6], vaddr[7]);
}

/**
 * Fills a rectangular area in the target window with a requested color.
 *
 * @param ctx Current blitter context.
 * @param dst Destination window handle to fill.
 * @param rect Rectangular area to fill.
 * @param color Color used to fill the target window, format is A8R8G8B8.
 * @param alpha Not used.
 * @retval EOK Sucess
 * @retval EINVAL Failure.
 */
static int
vivante_fill(win_blit_ctx_t ctx, win_handle_t dst,
    const win_rect_t *rect, uint32_t color, uint8_t alpha)
{
    vivante_handle_t *d = dst;
    int		         gc_rc;

    struct win_blit_ctx *blt_ctx   = (struct win_blit_ctx *)ctx;

    /* Swap red and blue channel.
       The clrColor is in G2D_RGBA8888 format, which has
       red offset 0 and blue offset 16. */
    uint32_t clrColor = ((color >> 16) & 0xFF) | ((color & 0xFF) << 16) | (color & 0xFF00FF00);

    pthread_mutex_lock(&blt_ctx->mutex);
#if defined(MULTIBLIT)
    if( ctx->n_blits ) {
        do_blits( ctx );
    }
#endif
    d->G2DSurfaceEx.base.width = d->NativeImage->width;
    d->G2DSurfaceEx.base.height = d->NativeImage->height;
    d->G2DSurfaceEx.base.left = rect->x1;
    d->G2DSurfaceEx.base.top = rect->y1;
    d->G2DSurfaceEx.base.right = rect->x2;
    d->G2DSurfaceEx.base.bottom = rect->y2;
    d->G2DSurfaceEx.base.clrcolor = clrColor;
    d->G2DSurfaceEx.base.rot = G2D_ROTATION_0;


    G2D_LOGERR_CALL_GC(update_g2d_surface,(d));

    G2D_LOGERR_CALL_GC(g2d_clear, (blt_ctx->G2Dhandle, &d->G2DSurfaceEx.base));

    pthread_mutex_unlock(&blt_ctx->mutex);

    if (gc_rc == 0) {
    	return EOK;
    } else {
    	return EINVAL;
    }

	return EOK;
}

/**
 * Performs one blit operation
 *
 * @param ctx Current blitter context.
 * @param src Handle to the source window.
 * @param dst Handle to the destination window.
 * @param args Blit operation arguments.
 * @retval EOK Sucess
 * @retval EINVAL Failure.
 */
static int
blit(win_blit_ctx_t ctx, win_handle_t src, win_handle_t dst, const win_blit_t *args )
{
    vivante_handle_t         *s = src;
    vivante_handle_t         *d = dst;
    int                      rotation = args->rotation;
    int                      gc_rc;

    if( ctx->print_stats > 1 ) {
        info("%p: %s()\n", ctx, __func__ );
    }

    /* GPU will clip the destination but not the source. */
    if (args->sx < 0 || args->sy < 0 ||
        args->sw > s->NativeImage->width ||
        args->sh > s->NativeImage->height) {
        error("%s[%d]: Invalid source rect: (x=%d, y=%d, w=%d, h=%d); Surface dim: (w=%d, h=%d)",
            __FUNCTION__, __LINE__, args->sx, args->sy, args->sw, args->sh, s->NativeImage->width, s->NativeImage->height);
        return EINVAL;
    }

    /* Set G2D transparency */
	s->G2DSurfaceEx.base.blendfunc = G2D_ONE;
	d->G2DSurfaceEx.base.blendfunc = G2D_ONE_MINUS_SRC_ALPHA;

	if(!args->premult_alpha) {
		s->G2DSurfaceEx.base.blendfunc |= G2D_PRE_MULTIPLIED_ALPHA;
	}

	if (args->transp == SCREEN_TRANSPARENCY_TEST) {
		G2D_LOGERR_CALL_GC(g2d_enable, (ctx->G2Dhandle, G2D_BLEND));
		G2D_LOGERR_CALL_GC(g2d_disable, (ctx->G2Dhandle, G2D_GLOBAL_ALPHA));
	} else if (args->transp == SCREEN_TRANSPARENCY_SOURCE_OVER) {
		G2D_LOGERR_CALL_GC(g2d_enable,(ctx->G2Dhandle, G2D_BLEND));
	} else if(args->global_alpha < 255) {
		s->G2DSurfaceEx.base.global_alpha = args->global_alpha;
		G2D_LOGERR_CALL_GC(g2d_enable,(ctx->G2Dhandle, G2D_BLEND));
		G2D_LOGERR_CALL_GC(g2d_enable, (ctx->G2Dhandle, G2D_GLOBAL_ALPHA));
	} else {
		G2D_LOGERR_CALL_GC(g2d_disable, (ctx->G2Dhandle, G2D_BLEND));
	}

	/* Set size and rotation parameters */
	s->G2DSurfaceEx.base.width = s->NativeImage->width;
	s->G2DSurfaceEx.base.height = s->NativeImage->height;

	s->G2DSurfaceEx.base.left = args->sx;
	s->G2DSurfaceEx.base.top = args->sy;
	s->G2DSurfaceEx.base.right = args->sx + args->sw;
	s->G2DSurfaceEx.base.bottom = args->sy + args->sh;
	s->G2DSurfaceEx.base.rot = G2D_ROTATION_0;
	if (args->mirror && args->flip) {
		/* Mirror and flip together means rotation by 180degree */
		rotation += 180;
		if (rotation > 360) {
			rotation -= 360;
		}
	} else if (args->mirror) {
		s->G2DSurfaceEx.base.rot = G2D_FLIP_H;
	} else if (args->flip) {
		s->G2DSurfaceEx.base.rot = G2D_FLIP_V;
	}
	G2D_LOGERR_CALL_GC(update_g2d_surface,(s));

	d->G2DSurfaceEx.base.width = d->NativeImage->width;
	d->G2DSurfaceEx.base.height = d->NativeImage->height;
	d->G2DSurfaceEx.base.left = args->dx;
	d->G2DSurfaceEx.base.top = args->dy;
	d->G2DSurfaceEx.base.right = args->dx + args->dw;
	d->G2DSurfaceEx.base.bottom = args->dy + args->dh;
    switch (rotation) {
	case 90:
		d->G2DSurfaceEx.base.rot = G2D_ROTATION_90;
		break;
	case 180:
		d->G2DSurfaceEx.base.rot = G2D_ROTATION_180;
		break;
	case 270:
		d->G2DSurfaceEx.base.rot = G2D_ROTATION_270;
		break;
	default:
		d->G2DSurfaceEx.base.rot = G2D_ROTATION_0;
		break;
    }
    G2D_LOGERR_CALL_GC(update_g2d_surface,(d));

    G2D_LOGERR_CALL_GC(g2d_blitEx,(ctx->G2Dhandle, &s->G2DSurfaceEx, &d->G2DSurfaceEx));
    G2D_LOGERR_CALL_GC(g2d_disable,(ctx->G2Dhandle, G2D_GLOBAL_ALPHA));
    G2D_LOGERR_CALL_GC(g2d_disable,(ctx->G2Dhandle, G2D_BLEND));

    if (gc_rc == 0) {
    	return EOK;
    } else {
    	return EINVAL;
    }
}

/**
 * Perform all the stored blit operations.

 * @param ctx Current blitter context.
 */
static void
vivante_flush(win_blit_ctx_t ctx)
{
#if defined(MULTIBLIT)
    struct win_blit_ctx *blt_ctx   = (struct win_blit_ctx *)ctx;

    pthread_mutex_lock(&blt_ctx->mutex);

    if( blt_ctx->n_blits ) {
        do_blits( blt_ctx );
    }

    pthread_mutex_unlock(&blt_ctx->mutex);
#endif
}

/**
 * Finish all the stored commands.
 *
 * @param ctx
 */

static void
vivante_finish(win_blit_ctx_t ctx)
{
    gceSTATUS gc_rc;
    struct win_blit_ctx *blt_ctx   = (struct win_blit_ctx *)ctx;

    pthread_mutex_lock(&blt_ctx->mutex);
#if defined(MULTIBLIT)
    if( blt_ctx->n_blits ) {
        do_blits( blt_ctx );
    }
#endif

    G2D_LOGERR_CALL_GC(g2d_finish, (blt_ctx->G2Dhandle));

    pthread_mutex_unlock(&blt_ctx->mutex);

}


/**
 * Allocates and initializes a handle for a window. For gpu implementation, the data buffer of the image is mapped and registered
 * in the gpu driver.
 *
 * @param ctx Current blitter context.
 * @param img Pointer to the win_image_t structure, which contains content of the window.
 *
 * @return Pointer to the new window handle.
 * @retval NULL In case of an error.
 */
static win_handle_t
vivante_alloc(win_blit_ctx_t ctx, win_image_t *img)
{
    vivante_handle_t    *handle;

    /* TODO - FIXME - remove the unneeded cast to struct win_blit_ctx * -
     * here and other instances.
     */
    struct win_blit_ctx *blt_ctx   = (struct win_blit_ctx *)ctx;

    if (!(img->flags & WIN_IMAGE_FLAG_PHYS_CONTIG)) {
        error("%s: could not ascertain if the buffer is contiguous. "
                "g2d blitter does not handle non-contiguous buffers!", __FUNCTION__);
        return NULL;
    }

    pthread_mutex_lock(&blt_ctx->mutex);

    if (blt_ctx->tls_errno != EOK) {
        pthread_mutex_unlock(&blt_ctx->mutex);
        error("%s:%d: thread creation failed, errno %s", __FUNCTION__, __LINE__, strerror(blt_ctx->tls_errno));
        return NULL;
    }

    handle = malloc(sizeof(*handle));
    if (!handle) {
        pthread_mutex_unlock(&blt_ctx->mutex);
        error("%s: could not allocate memory for handle", __FUNCTION__);
        return NULL;
    }

    handle->NativeImage = img;
    handle->MappingInfo = NULL;

    pthread_mutex_unlock(&blt_ctx->mutex);

    return handle;
}

/**
 * Destroy the window handle
 *
 * @param ctx Current blitter context.
 * @param hnd Window handle to be destroyed.
 */
static void
vivante_free(win_blit_ctx_t ctx, win_handle_t hnd)
{
    vivante_handle_t    *handle = hnd;

    struct win_blit_ctx *blt_ctx  = (struct win_blit_ctx *)ctx;

    pthread_mutex_lock(&blt_ctx->mutex);
    free(handle);

    pthread_mutex_unlock(&blt_ctx->mutex);
}

static int
vivante_prefx(win_blit_ctx_t ctx, EGLSurface surf, const win_image_t *img)
{
    return 0;
}

/*********************************************************** VIVANTE_BLIT() ***/
/**
 *
 * @param ctx Current blitter context.
 * @param src
 * @param dst
 * @param args
 * @return
 */
static int
vivante_blit( win_blit_ctx_t ctx, win_handle_t src, win_handle_t dst, const win_blit_t *args )
{
    struct win_blit_ctx *blt_ctx = (struct win_blit_ctx *)ctx;

    pthread_mutex_lock( &blt_ctx->mutex );

    if( blt_ctx->print_stats > 1 ) {
        info("%p: blt%d"
                ",srchdl=%p,paddr=0x%lx, srcfmt=%d,srcx=%d,srcy=%d,srcw=%d,srch=%d",
                blt_ctx, blt_ctx->n_blits+1,
                src, ((vivante_handle_t*)src)->NativeImage->paddr, ((vivante_handle_t*)src)->NativeImage->format, args->sx, args->sy, args->sw, args->sh
            );
		info("%p: blt%d"
                ",dsthdl=%p,paddr=0x%lx, dstfmt=%d,dstx=%d,dsty=%d,dstw=%d,dsth=%d",
                blt_ctx, blt_ctx->n_blits+1,		
                dst, ((vivante_handle_t*)dst)->NativeImage->paddr, ((vivante_handle_t*)dst)->NativeImage->format, args->dx, args->dy, args->dw, args->dh
            );
		info("%p: blt%d"
                ",trans=%d,premult_alpha=%d,global_alpha=%d"
                ",quality=%d,rotation=%d,flip=%d,mirror=%d%s",
                blt_ctx, blt_ctx->n_blits+1,
                args->transp, args->premult_alpha, args->global_alpha,
                args->quality, args->rotation, args->flip, args->mirror,
		IS_SCALED(*args) ? "  # scaled" : ""
            );
	    printbuff("src pre ", ((vivante_handle_t*)src)->NativeImage);
	    printbuff("dst pre ", ((vivante_handle_t*)dst)->NativeImage);
    }

    /* NOTES - just use the multiblit detect capability of the G2D to allow multiblit */

    // FIXME: multisource blit seems to be broken, work around it for now
    if( 1 || !blt_ctx->multisource ) {
        /* no multi-source caps -- use as is */
        int rc = blit( ctx, src, dst, args );

    if( blt_ctx->print_stats > 1 ) {
		printbuff("src post",((vivante_handle_t*)src)->NativeImage);
		printbuff("dst post",((vivante_handle_t*)dst)->NativeImage);	
    }

	pthread_mutex_unlock( &blt_ctx->mutex );
        return rc;
    }
    /* NOTES - currently only single-blit is allowed */

#if defined(MULTIBLIT)
    /* Add the new blit into the array */
    struct g2d_surface_pair *op = &blt_ctx->blits[blt_ctx->n_blits];
    op->s = src;
    op->d = dst;

    blt_ctx->n_blits++;

    if( MAX_BLIT_OPS == blt_ctx->n_blits ) {
        do_blits( blt_ctx );
    }
#endif

    pthread_mutex_unlock( &blt_ctx->mutex );

    return 0;
}
#if defined(MULTIBLIT)
/************************************************************** GET_BATCH() ***/
/* Returns the starting element in the blits array to process, or a NULL
 * 'n_' returns number of elements in the batch.
 */
static blit_op_t *
get_batch( struct win_blit_ctx *blt_ctx, blit_op_t *first_op, unsigned *n_ )
{
    blit_op_t *end = &blt_ctx->blits[blt_ctx->n_blits];

    if( first_op >= end ) {
        *n_ = 0;
        return NULL;
    }

    /* Multi-source blit constraints:
     *   -- gc320 -- can't do "non-zero based origin source and dest".
     *   -- IS_YUV_FORMAT() -- Supports only one YUV source at a time.
     *   -- IS_SCALED() -- Multi-source Blit does not support stretch, filter or scaling.
     *     !!! This is important because we could implement 'fills' as scaled blits.
     *   -- IS_YUV_DST_CONSTRAINED() No support for source and destination rotation, mirror X, mirror XY,
     *      flip X, flip XY, if output is YUY2 or UYVY format.
     *   -- The first destination rectangle should cover whole render area.
     *      If not, then user needs to enable alpha blending to make sure
     *      the GPU can fetch the destination pixels in uncovered area.
     *   -- Multiple destination support is achieved through multiple iterations
     *      of multi-source blit.
     *   -- 8 destinations support the same rotation
     */

    *n_ = 1;

    if( IS_SCALED( first_op->args ) ) {
        /* Scaled blits go on their own */
        return first_op;
    }

    if( IS_YUV_DST_CONSTRAINED( first_op->dst->Format, first_op->args ) ) {
        /* These go on their own too */
        return first_op;
    }

    int yuv = IS_YUV_FORMAT( first_op->src->Format );

#if 1
    /* TODO: This code could be disabled once we confirm that gc320 is actually
     * capable of multiblitting YUV formats */
    if( yuv && blt_ctx->chipModel == gcv320 ) {
        /* gc320 can't do a general case for yuv */
        return first_op;
    }
#endif

    win_blit_t *args = &first_op->args;
    blit_op_t *op = first_op + 1;

    /* Loop until we have a "fitting" blit -- in relation to the first one */
    while( op < end ) {

        if( IS_YUV_FORMAT( op->src->Format ) ) {
            if( yuv ) {
                /* only one YUV src is allowed for multi-source blits */
                break;
            }
            yuv = 1;
        }

        if( first_op->dst != op->dst ) {
            /* different destination -- user level blits, not the compositer */
            break;
        }

        if( IS_SCALED( op->args ) ) {
            /* no scaled blits */
            break;
        }

        if( args->global_alpha != op->args.global_alpha ) {
            /* different global alpha */
            break;
        }

        if( IS_YUV_DST_CONSTRAINED( op->dst->Format, op->args ) ) {
            /* this will go next, and on its own */
            break;
        }

        if( op->args.flip != args->flip ||
            op->args.mirror != args->mirror ||
            op->args.rotation != args->rotation
        ) {
            /* All of these must match */
            break;
        }

        (*n_)++;
        op++;
    }

    return first_op;
}


#if 1
/* These are handydandy Vivante helpers */
#define POOL_SIZE 1024

/* Area struct. */
typedef struct _T2DArea
{
    /* Area potisition. */
    gcsRECT                 rect;

    /* Bit field, layers who own this Area. */
    gctUINT32               owners;

    /* Point to next area. */
    struct _T2DArea *       next;
} T2DArea;

/* Area pool struct. */
typedef struct _T2DAreaPool
{
    /* Pre-allocated areas. */
    T2DArea *               areas;

    /* Point to free area. */
    T2DArea *               freeNodes;

    /* Next area pool. */
    struct _T2DAreaPool *   next;
} T2DAreaPool;

/* HWC context. */
typedef struct _T2DAreaContext
{
    /* Pre-allocated area pool. */
    T2DAreaPool *           areaPool;
} T2DAreaContext;

static void FreeContext( IN T2DAreaContext *Context )
{
    T2DAreaPool *pPool, *pNext;

    pPool = Context->areaPool;

    while (pPool != gcvNULL)
    {
        pNext = pPool->next;

        if (pPool->areas != gcvNULL)
        {
            free(pPool->areas);
        }

        free(pPool);

        pPool = pNext;
    }
}

static T2DArea *AllocateArea(
    IN T2DAreaContext *Context,
    IN T2DArea * Slibing,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    T2DArea * area;
    T2DAreaPool * pool  = Context->areaPool;

    if (pool == NULL)
    {
        /* First pool. */
        pool = (T2DAreaPool *) malloc(sizeof (T2DAreaPool));
        Context->areaPool = pool;

        /* Clear fields. */
        pool->areas     = NULL;
        pool->freeNodes = NULL;
        pool->next      = NULL;
    }

    for (;;)
    {
        if (pool->areas == NULL)
        {
            /* No areas allocated, allocate now. */
            pool->areas = (T2DArea *) malloc(sizeof (T2DArea) * POOL_SIZE);

            /* Get area. */
            area = pool->areas;

            /* Update freeNodes. */
            pool->freeNodes = area + 1;

            break;
        }

        else if (pool->freeNodes - pool->areas >= POOL_SIZE)
        {
            /* This pool is full. */
            if (pool->next == NULL)
            {
                /* No more pools, allocate one. */
                pool->next = (T2DAreaPool *) malloc(sizeof (T2DAreaPool));

                /* Point to the new pool. */
                pool = pool->next;

                /* Clear fields. */
                pool->areas     = NULL;
                pool->freeNodes = NULL;
                pool->next      = NULL;
            }

            else
            {
                /* Advance to next pool. */
                pool = pool->next;
            }
        }

        else
        {
            /* Get area and update freeNodes. */
            area = pool->freeNodes++;

            break;
        }
    }

    /* Update area fields. */
    area->rect   = *Rect;
    area->owners = Owner;

    if (Slibing == NULL)
    {
        area->next = NULL;
    }

    else
    {
        area->next = Slibing->next;
        Slibing->next = area;
    }

    return area;
}

/************************************************************** SplitArea() ***/
static int SplitArea(
    IN T2DAreaContext * Context,
    IN T2DArea * Area,
    IN gcsRECT * Rect,
    IN gctUINT32 Owner
    )
{
    gcsRECT r0[4];
    gcsRECT r1[4];
    gctUINT32 c0 = 0;
    gctUINT32 c1 = 0;
    gctUINT32 i;

    gcsRECT * rect;

    for (;;)
    {
        rect = &Area->rect;

        if ((Rect->left   < rect->right)
        &&  (Rect->top    < rect->bottom)
        &&  (Rect->right  > rect->left)
        &&  (Rect->bottom > rect->top)
        )
        {
            /* Overlapped. */
            break;
        }

        if (Area->next == NULL)
        {
            /* This rectangle is not overlapped with any area. */
            AllocateArea(Context, Area, Rect, Owner);
            return 1;
        }

        Area = Area->next;
    }

    /* OK, the rectangle is overlapped with 'rect' area. */
    if ((Rect->left <= rect->left)
    &&  (Rect->right >= rect->right)
    )
    {
        /* |-><-| */
        /* +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         * | X | X | X |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->left <= rect->left)
    {
        /* |-> */
        /* +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         * | X | X |   |
         * +---+---+---+
         */

        if (Rect->left < rect->left)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->left;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else if (Rect->right >= rect->right)
    {
        /*    <-| */
        /* +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         * |   | X | X |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        if (Rect->right > rect->right)
        {
            r1[c1].left   = rect->right;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    else
    {
        /* | */
        /* +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         * |   | X |   |
         * +---+---+---+
         */

        /* if (rect->left < Rect->left) */
        {
            r0[c0].left   = rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->left;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->top < rect->top)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = Rect->top;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = rect->top;

            c1++;
        }

        else if (rect->top < Rect->top)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = rect->top;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = Rect->top;

            c0++;
        }

        /* if (rect->right > Rect->right) */
        {
            r0[c0].left   = Rect->right;
            r0[c0].top    = rect->top;
            r0[c0].right  = rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }

        if (Rect->bottom > rect->bottom)
        {
            r1[c1].left   = Rect->left;
            r1[c1].top    = rect->bottom;
            r1[c1].right  = Rect->right;
            r1[c1].bottom = Rect->bottom;

            c1++;
        }

        else if (rect->bottom > Rect->bottom)
        {
            r0[c0].left   = Rect->left;
            r0[c0].top    = Rect->bottom;
            r0[c0].right  = Rect->right;
            r0[c0].bottom = rect->bottom;

            c0++;
        }
    }

    if (c1 > 0)
    {
        /* Process rects outside area. */
        if (Area->next == NULL)
        {
            /* Save rects outside area. */
            for (i = 0; i < c1; i++)
            {
                AllocateArea(Context, Area, &r1[i], Owner);
            }
        }

        else
        {
            /* Rects outside area. */
            for (i = 0; i < c1; i++)
            {
                SplitArea(Context, Area, &r1[i], Owner);
            }
        }
    }

    if (c0 > 0)
    {
        /* Save rects inside area but not overlapped. */
        for (i = 0; i < c0; i++)
        {
            AllocateArea(Context, Area, &r0[i], Area->owners);
        }

        /* Update overlapped area. */
        if (rect->left   < Rect->left)   { rect->left   = Rect->left;   }
        if (rect->top    < Rect->top)    { rect->top    = Rect->top;    }
        if (rect->right  > Rect->right)  { rect->right  = Rect->right;  }
        if (rect->bottom > Rect->bottom) { rect->bottom = Rect->bottom; }
    }

    /* The area is owned by the new owner as well. */
    Area->owners |= Owner;

    /* Return number of layers involved */
    return __builtin_popcount( Area->owners );
}

#endif

/******************************************************** BYTES_PER_PIXEL() ***/
static unsigned
bytes_per_pixel( gceSURF_FORMAT format )
{
    switch( format ) {
    case gcvSURF_L8:
        return 1;

    case gcvSURF_A4R4G4B4:
    case gcvSURF_X4R4G4B4:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_X1R5G5B5:
    case gcvSURF_R5G6B5:
        return 2;

    case gcvSURF_R8G8B8:
        return 3;

    case gcvSURF_A8R8G8B8:
    case gcvSURF_X8R8G8B8:
        return 4;

    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_YUY2:
        return 2;

    case gcvSURF_YV12:
    case gcvSURF_I420:
    case gcvSURF_NV12:
        /* TODO: The number is actually 1.5 bytes per pixel
         * but since this is for a rough estimate of required memory bandwidth
         */
        return 2;

    default:
        return 4;
    }
}

static void
calc_stats( struct win_blit_ctx *blt_ctx, blit_op_t *ops, unsigned n, T2DAreaContext *ctx )
{
    unsigned long org_bytes = 0, act_bytes = 0;
    static unsigned long long org_MB = 0, act_MB = 0;
    static unsigned long org_acc = 0, act_acc = 0;
    static time_t next_update = 0;
    unsigned i, cnt = 1;
    blit_op_t *op = ops;

    if( blt_ctx->print_stats > 1 ) {
        info("\n%p:%s()\n", blt_ctx,__func__ );
    }

    for( i = 0; i < n; i++, op++ ) {

        org_bytes += bytes_per_pixel( op->src->Format ) * op->args.sw * op->args.sh
                  + bytes_per_pixel( op->dst->Format ) * op->args.dw * op->args.dh;

        if( blt_ctx->print_stats > 1 ) {
            info("%p:blt%d: (%d, %d) --> (%d, %d)\n", blt_ctx, i+1,
                op->args.dx, op->args.dy,
                op->args.dx + op->args.dw,
                op->args.dy + op->args.dh );
        }
    }

    T2DAreaPool *pAreaPool = ctx->areaPool;

    /* Dump results */
    while (pAreaPool != gcvNULL)
    {
        T2DArea *pArea = pAreaPool->areas;

        while (pArea != gcvNULL)
        {
            int w = pArea->rect.right - pArea->rect.left;
            int h = pArea->rect.bottom - pArea->rect.top;

            if( blt_ctx->print_stats > 1 ) {
                info("%p:..mlt%d: (%d, %d) --> (%d, %d), owners = 0x%x (n=%d)\n",
                    blt_ctx, cnt++,
                    pArea->rect.left, pArea->rect.top, pArea->rect.right,
                    pArea->rect.bottom, pArea->owners,
                    __builtin_popcount( pArea->owners )
                );
            }

            for( i = 0; i < n; i++ ) {
                if( (1 << i) & pArea->owners ) {
                    act_bytes += bytes_per_pixel( ops[i].src->Format ) * w * h;
                }
            }

            act_bytes += bytes_per_pixel( ops->dst->Format ) * w * h;

            pArea = pArea->next;
        }
        pAreaPool = pAreaPool->next;
    }

    if( blt_ctx->print_stats > 1 ) {
        info("%p: n_blits=%d, org_bytes=%lu, act_bytes=%lu, ratio=%f\n",
            blt_ctx, n, org_bytes, act_bytes, (double)act_bytes/(double)org_bytes );
    }

    #define MB (1024*1024)

    if( (org_acc += org_bytes) > MB ) {
        org_MB += org_acc/MB;
        org_acc %= MB;
    }

    if( (act_acc += act_bytes) > MB ) {
        act_MB += act_acc/MB;
        act_acc %= MB;
    }

    if( time(NULL) > next_update ) {
        error("%p:[O:%lluMB,A:%lluMB,R:%f]\n",
            blt_ctx, org_MB, act_MB, (double)act_MB/(double)org_MB );
        next_update = time(NULL) + 5;
    }

    return;
}

/********************************************************* GET_SURF_SPECS() ***/
static int
get_surf_specs( vivante_handle_t *surf,
    gctUINT32_PTR addrs, gctUINT32_PTR n_addrs,
    gctUINT32_PTR strides, gctUINT32_PTR n_strides )
{
    gceSURF_FORMAT format = surf->Format;

    if( format == gcvSURF_NV12 ) {
        *n_addrs = *n_strides = 2;

        addrs[0] = surf->Address + surf->NativeImage->planar_offsets[0];
        strides[0] = surf->NativeImage->strides[0];

        addrs[1] = surf->Address + surf->NativeImage->planar_offsets[1];
        strides[1] = surf->NativeImage->strides[0];

    } else if( format == gcvSURF_I420 || format == gcvSURF_YV12 ) {

        *n_addrs = *n_strides = 3;

        addrs[0] = surf->Address + surf->NativeImage->planar_offsets[0];
        strides[0] = surf->NativeImage->strides[0];

        /* TODO: It is not clear if the plane order needs to be flipped depending on the format */
        addrs[1] = surf->Address + surf->NativeImage->planar_offsets[1]; /* SrcUAddress */
        strides[1] = surf->NativeImage->strides[0]/2;

        addrs[2] = surf->Address + surf->NativeImage->planar_offsets[2]; /* SrcVAddress */
        strides[2] = surf->NativeImage->strides[0]/2;

    } else {
        *n_addrs = *n_strides = 1;
        addrs[0] = surf->Address;
        strides[0] = surf->NativeImage->strides[0];
    }
    return 0;
}

/************************************************************ SURF_ADJUST() ***/
/* Recalculate address for the specifed offsets */
static void
surf_adjust( vivante_handle_t *surf,
    gctUINT32_PTR addrs, gctUINT32 n_addrs,
    gctUINT32_PTR strides, gctUINT32 n_strides,
    int offset_x, int offset_y )
{
    gceSURF_FORMAT format = surf->Format;

    addrs[0] += offset_y * strides[0];

    if( format == gcvSURF_NV12 ) {
        addrs[0] += offset_x;

        addrs[1] += (offset_y/2) * strides[1];
        addrs[1] += offset_x - (offset_x % 2);

    } else if( format == gcvSURF_I420 || format == gcvSURF_YV12 ) {
        addrs[0] += offset_x;

        addrs[1] += (offset_y/2) * strides[1];
        addrs[1] += offset_x/2;

        addrs[2] += (offset_y/2) * strides[2];
        addrs[2] += offset_x/2;

    } else {
        addrs[0] += offset_x * bytes_per_pixel( format );

    }
}

/********************************************************** DO_MULTI_BLIT() ***/
static void
do_multi_blit( T2DAreaContext *ctx, struct win_blit_ctx *blt_ctx, blit_op_t *ops, const unsigned n )
{
    T2DAreaPool *pAreaPool = ctx->areaPool;
    gceSTATUS   gc_rc;
    unsigned    i;
    blit_op_t   *op;
    gcsRECT     drect = { 0, 0, 0, 0 };
    gcsRECT     srect;

    gctUINT8    bg_rop;
    int         quality;

    gctUINT32   addrs[3], n_addrs;
    gctUINT32   strides[3], n_strides;

    if( blt_ctx->print_stats ) {
        /* We calc stats at the lowest logging level. */
        calc_stats( blt_ctx, ops, n, ctx );
    }

    VIVANTE_MODULE_SET_CONTEXT(blt_ctx);

    for( i = 0, op = ops; i < n; i++, op++ ) {

        /* Calculate bounding (clipping) box of the destination */
        if( blt_ctx->chipModel > gcv320 ) {
            if( drect.right < op->args.dx + op->args.dw ) {
                drect.right = op->args.dx + op->args.dw;
            }
            if( drect.bottom < op->args.dy + op->args.dh ) {
                drect.bottom = op->args.dy + op->args.dh;
            }
        } else {
            /* NOTE: Assuming gc320 and below can only do "zero based origin"
             * The clipping rect in this case is going to be as big as the largest blit
             */
            if( drect.right < op->args.dw ) {
                drect.right = op->args.dw;
            }
            if( drect.bottom < op->args.dh ) {
                drect.bottom = op->args.dh;
            }
        }

        VIVANTE_LOGERR_CALL_GC( gco2D_SetCurrentSourceIndex, (blt_ctx->engine, i) );

        /* skip setup of the source if this is gc320 or below -- it doesn't support origins other than 0,0 */
        if( blt_ctx->chipModel > gcv320 ) {

            if( -1 == get_surf_specs( op->src, addrs, &n_addrs, strides, &n_strides ) ) {
                error( "get_specs() failed on src#%d.\n", i );
                return;
            }

            VIVANTE_LOGERR_CALL_GC( gco2D_SetGenericSource, (blt_ctx->engine,
                addrs, n_addrs,
                strides, n_strides,
                0, op->src->Format,
                ops->args.rotation,
                op->src->NativeImage->width, op->src->NativeImage->height ) );
        }

#if 0
        /* NOTE: According to Vivante this is not required for QNX yet */
        VIVANTE_LOGERR_CALL_GC(gco2D_SetSourceTileStatus(
            egn2D,
            t2d->surf[i]->tileStatusConfig,
            t2d->surf[i]->format,
            t2d->surf[i]->tileStatusClear,
            t2d->surf[i]->tileStatusAddress
            ));
#endif

        set_transparency( blt_ctx, &op->args, &bg_rop, &quality );

        VIVANTE_LOGERR_CALL_GC( gco2D_SetROP, (blt_ctx->engine, ROP_SRC, bg_rop) );

        VIVANTE_LOGERR_CALL_GC( gco2D_SetBitBlitMirror, (blt_ctx->engine, ops->args.mirror, ops->args.flip ) );
    }

    if( blt_ctx->chipModel > gcv320 ) {

        if( -1 == get_surf_specs( ops->dst, addrs, &n_addrs, strides, &n_strides ) ) {
            error( "get_specs() failed on dst." );
            return;
        }

        VIVANTE_LOGERR_CALL_GC( gco2D_SetGenericTarget, (blt_ctx->engine,
                addrs, n_addrs,
                strides, n_strides,
                0, ops->dst->Format,
                ops->args.rotation,
                ops->dst->NativeImage->width, ops->dst->NativeImage->height ) );
    }

    VIVANTE_LOGERR_CALL_GC( gco2D_SetClipping, (blt_ctx->engine, &drect) );

    if( blt_ctx->print_stats > 1 ) {
        info("%p: clipRect: (%d,%d) --> (%d,%d)\n", blt_ctx, drect.left, drect.top, drect.right, drect.bottom );
    }

#if 0
    /* NOTE: According to Vivante this is not required for QNX yet */
    VIVANTE_LOGERR_CALL_GC( gco2D_SetTargetTileStatus(
        egn2D,
        result->tileStatusConfig,
        result->format,
        gcvSURF_0_DEGREE,
        result->tileStatusAddress
        ));
#endif

    /* Loop all the areas. */
    pAreaPool = ctx->areaPool;

    while( pAreaPool != gcvNULL ) {
        T2DArea *pArea = pAreaPool->areas;
        unsigned i;
        while( pArea != gcvNULL ) {

            int pW = pArea->rect.right - pArea->rect.left;
            int pH = pArea->rect.bottom - pArea->rect.top;

            for (i = 0; i < n; i++) {

                if( (1 << i) & pArea->owners ) {
                    blit_op_t *op = ops+i;

                    VIVANTE_LOGERR_CALL_GC(gco2D_SetCurrentSourceIndex, (blt_ctx->engine, i) );

                    int offset_x = op->args.sx - op->args.dx;
                    int offset_y = op->args.sy - op->args.dy;

                    /* Translate the source rect */
                    srect.left   = pArea->rect.left    + offset_x;
                    srect.top    = pArea->rect.top     + offset_y;
                    srect.right  = pArea->rect.right   + offset_x;
                    srect.bottom = pArea->rect.bottom  + offset_y;

                    if( blt_ctx->chipModel <= gcv320 ) {

                        /* Since the source and destination must match we are bending over */
                        /* TODO: Vivante needs to fix this -- then we remove this "special" code */

                        if( -1 == get_surf_specs( op->src, addrs, &n_addrs, strides, &n_strides ) ) {
                            error( "get_specs() failed on src#%d.", i );
                            return;
                        }

                        surf_adjust( op->src, addrs, n_addrs, strides, n_strides, srect.left, srect.top );

                        srect.left = srect.top = 0;
                        srect.right = pW;
                        srect.bottom = pH;

                        VIVANTE_LOGERR_CALL_GC( gco2D_SetGenericSource, (blt_ctx->engine,
                            addrs, n_addrs,
                            strides, n_strides,
                            0, op->src->Format,
                            ops->args.rotation,
                            op->src->NativeImage->width, op->src->NativeImage->height ) );

                    }

                    if( blt_ctx->print_stats > 1 ) {
                        info("%p: ..source_idx=%d, src_rect=(%d,%d)->(%d,%d)\n",
                            blt_ctx, i+1, srect.left, srect.top, srect.right, srect.bottom );
                    }

                    VIVANTE_LOGERR_CALL_GC(gco2D_SetSource, (blt_ctx->engine, &srect) );
                }
            }

            if( blt_ctx->chipModel <= gcv320 ) {

                if( -1 == get_surf_specs( ops->dst, addrs, &n_addrs, strides, &n_strides ) ) {
                    error( "get_specs() failed on dst." );
                    return;
                }

                surf_adjust( ops->dst, addrs, n_addrs, strides, n_strides, pArea->rect.left, pArea->rect.top );

                VIVANTE_LOGERR_CALL_GC( gco2D_SetGenericTarget, (blt_ctx->engine,
                    addrs, n_addrs,
                    strides, n_strides,
                    0, ops->dst->Format,
                    ops->args.rotation,
                    ops->dst->NativeImage->width, ops->dst->NativeImage->height ) );

                if( blt_ctx->print_stats > 1 ) {
                    info("%p: multisrcBlit: owners=%#x, rect=(%d,%d)->(%d,%d)\n",
                                blt_ctx, pArea->owners,
                                srect.left, srect.top, srect.right, srect.bottom );
                }

                VIVANTE_LOGERR_CALL_GC( gco2D_MultiSourceBlit, (blt_ctx->engine, pArea->owners, &srect, 1) );

            } else {

                if( blt_ctx->print_stats > 1 ) {
                    info("%p: multisrcBlit: owners=%#x, rect=(%d,%d)->(%d,%d)\n",
                                blt_ctx, pArea->owners,
                                pArea->rect.left, pArea->rect.top, pArea->rect.right, pArea->rect.bottom );
                }

                VIVANTE_LOGERR_CALL_GC( gco2D_MultiSourceBlit, (blt_ctx->engine, pArea->owners, &pArea->rect, 1) );

            }

            pArea = pArea->next;

        }

        pAreaPool = pAreaPool->next;

    }

    VIVANTE_LOGERR_CALL_GC( gcoHAL_Commit, (blt_ctx->hal, gcvTRUE));
}

/********************************************************** PROCESS_BATCH() ***/
/* We have at least one blit_op in the batch */
static void
process_batch( struct win_blit_ctx *blt_ctx, blit_op_t *ops, unsigned n )
{
    unsigned i = 0;

    if( blt_ctx->print_stats > 1 ) {
        info("%p: process_batch() n = %d\n", blt_ctx, n );
    }

    while( i < n ) {

        /* TODO: Do we need to put dst as the first rect when we need to loop more than once?
         * After a few quick tests -- this doesn't seem to be required.
         */

        if( n == 1 ) {
            if( blt_ctx->print_stats > 1 ) {
                info("%p: %s(): fallback to blit()\n", blt_ctx, __func__);
            }
            blit( blt_ctx, ops->src, ops->dst, &ops->args );
            return;
        }

        T2DAreaContext ctx = { NULL };
        T2DArea *pArea;

        /* Split the dest rectangles into areas. */
        gcsRECT rect = {ops->args.dx, ops->args.dy,
                        ops->args.dx + ops->args.dw,
                        ops->args.dy + ops->args.dh };

        pArea = AllocateArea( &ctx, gcvNULL, &rect, 1 );
        if( NULL == pArea ) {
            error( "%p: AllocateArea() out-of-memory", blt_ctx );
            return;
        }

        blit_op_t *op = ops + 1;

        for( i = 1; i < MAX_PER_BATCH && i < n; i++, op++ ) {

            rect.left   = op->args.dx;
            rect.top    = op->args.dy;
            rect.right  = op->args.dx + op->args.dw;
            rect.bottom = op->args.dy + op->args.dh;

            if( SplitArea( &ctx, pArea, &rect, 1 << i ) >= MAX_PER_BATCH ) {
                break;
            }
        }

        do_multi_blit( &ctx, blt_ctx, ops, i );

        FreeContext( &ctx );

        ops += i;
        n -= i;
        i = 0;
    }

    return;
}

/*************************************************************** DO_BLITS() ***/
static void
do_blits( struct win_blit_ctx *ctx )
{
    blit_op_t *ops = ctx->blits;
    unsigned n = 0;

    /* validate the incoming blits, split them into "workable" batches... */
    while( (ops = get_batch( ctx, ops + n, &n )) ) {

        /* process each batch */
        process_batch( ctx, ops, n );
    }

    ctx->n_blits = 0;

}
#endif
/*********************************************** WIN_BLIT_MODULE_GETFUNCS() ***/
void
win_blit_module_getfuncs(win_blit_module_t *module)
{
    module->funcs.ctx_init = vivante_ctx_init;
    module->funcs.ctx_fini = vivante_ctx_fini;
    module->funcs.alloc = vivante_alloc;
    module->funcs.free = vivante_free;
    module->funcs.fill = vivante_fill;
    module->funcs.blit = vivante_blit;
    module->funcs.flush = vivante_flush;
    module->funcs.finish = vivante_finish;
    module->funcs.prefx = vivante_prefx;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL$ $Rev$")
#endif
