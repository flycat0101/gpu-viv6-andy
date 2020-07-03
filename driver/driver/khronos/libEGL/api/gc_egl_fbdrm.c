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


/* Enable sigaction declarations. */
#if !defined _XOPEN_SOURCE
#   define _XOPEN_SOURCE 501
#endif

#include "gc_egl_platform.h"
#include "gc_egl_fb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>

#include <poll.h>
#include <errno.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#ifndef O_CLOEXEC
# define O_CLOEXEC 02000000
#endif

#include "gc_hal.h"
#include "gc_hal_user.h"

#if defined(MXC_FBDEV) && MXC_FBDEV
/* FSL: external resolve. */
#include "mxcfb.h"
#include "ipu.h"
#endif

#define gcdUSE_PIXMAP_SURFACE 1


#define _GC_OBJ_ZONE    gcvZONE_OS

#define GC_FB_MAX_SWAP_INTERVAL     10
#define GC_FB_MIN_SWAP_INTERVAL     0

struct plane
{
    drmModePlane *plane;
    drmModeObjectProperties *props;
    drmModePropertyRes **props_info;
};

struct crtc
{
    drmModeCrtc *crtc;
    drmModeObjectProperties *props;
    drmModePropertyRes **props_info;
};

struct connector
{
    drmModeConnector *connector;
    drmModeObjectProperties *props;
    drmModePropertyRes **props_info;
};

#define get_resource(type, Type, id) \
    do { \
        drm->type.type = drmModeGet##Type(drm->fd, id); \
        if (!drm->type.type) \
        { \
            printf("could not get %s %i: %s\n", #type, id, strerror(errno)); \
            return -1; \
        } \
    } while (0)


#define get_properties(type, TYPE, id) \
    do { \
        uint32_t i; \
        drm->type.props = drmModeObjectGetProperties(drm->fd, id, DRM_MODE_OBJECT_##TYPE); \
        if (!drm->type.props) \
        { \
            printf("could not get %s %u properties: %s\n", #type, id, strerror(errno)); \
            return -1; \
        } \
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, \
            (sizeof (drm->type.props_info) * drm->type.props->count_props), \
            (gctPOINTER *)&drm->type.props_info))) \
        { \
            printf("There is out of memory.\n"); \
            return -1; \
        } \
        gcoOS_ZeroMemory(&drm->type.props_info, sizeof (drm->type.props_info) * drm->type.props->count_props); \
        for (i = 0; i < drm->type.props->count_props; i++) \
        { \
            drm->type.props_info[i] = drmModeGetProperty(drm->fd, drm->type.props->props[i]); \
        } \
    } while (0)


struct _DRMDisplay
{
    int fd;
    drmModeModeInfo *mode;
    uint32_t crtc_id;
    uint32_t plane_id;
    uint32_t connector_id;
    uint32_t height;
    uint32_t width;
    uint32_t crtc_index;
    uint32_t pending;

    struct plane plane;
    struct crtc crtc;
    struct connector connector;
    gctBOOL atomic_modeset;
};

struct drm_fb
{
    uint32_t fb_id;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
    uint32_t bpp;
    uint32_t handle;
    int32_t fd;
    uint8_t *map;
};


/* Structure that defines a display. */
struct _FBDisplay
{
    uint32_t                signature;
    gctINT                  index;
    gctINT                  file;
    gctINT                  stride;
    gctINT                  width;
    gctINT                  height;
    gctINT                  alignedHeight;
    gctINT                  bpp;
    gctINT                  size;
    gctPOINTER              memory;
    struct fb_var_screeninfo    varInfo;
    gctINT                  backBufferY;
    gctINT                  multiBuffer;
    pthread_cond_t          cond;
    pthread_mutex_t         condMutex;
    gctUINT                 alphaLength;
    gctUINT                 alphaOffset;
    gctUINT                 redLength;
    gctUINT                 redOffset;
    gctUINT                 greenLength;
    gctUINT                 greenOffset;
    gctUINT                 blueLength;
    gctUINT                 blueOffset;
    gceSURF_FORMAT          format;
    gceTILING               tiling;
    gctINT                  refCount;
    gctBOOL                 panVsync;

    struct _FBDisplay *     next;

    /* FSL: external resolve and special PAN timing. */
    gctBOOL                 fbPrefetch;

    gctUINT32               bufferStatus;

    struct _DRMDisplay      drm;
    struct drm_fb *         drmfbs;
};

/* Structure that defines a window. */
struct _FBWindow
{
    struct _FBDisplay* display;
    gctUINT            offset;
    gctINT             x, y;
    gctINT             width;
    gctINT             height;
    gctINT             swapInterval;
    /* Color format. */
    gceSURF_FORMAT     format;
};

/* Structure that defines a pixmap. */
struct _FBPixmap
{
    /* Pointer to memory bits. */
    gctPOINTER       original;
    gctPOINTER       bits;

    /* Bits per pixel. */
    gctINT         bpp;

    /* Size. */
    gctINT         width, height;
    gctINT         alignedWidth, alignedHeight;
    gctINT         stride;

#if gcdUSE_PIXMAP_SURFACE
    gcoSURF         surface;
    gctUINT32       gpu;
#endif
};

static int
add_plane_property(
    struct _DRMDisplay * drm,
    drmModeAtomicReq *req,
    uint32_t obj_id,
    const char *name,
    uint64_t value
    )
{
    struct plane obj = drm->plane;
    unsigned int i;
    int prop_id = -1;

    for (i = 0 ; i < obj.props->count_props ; i++)
    {
        if (strcmp(obj.props_info[i]->name, name) == 0)
        {
            prop_id = obj.props_info[i]->prop_id;
            break;
        }
    }

    if (prop_id < 0)
    {
        printf("no plane property: %s\n", name);
        return -EINVAL;
    }

    return drmModeAtomicAddProperty(req, obj_id, prop_id, value);
}

static int
drm_atomic_commit(
    struct _DRMDisplay * drm,
    uint32_t fb_id
    )
{
    drmModeAtomicReq *req;
    uint32_t plane_id = drm->plane.plane->plane_id;
    int ret;

    req = drmModeAtomicAlloc();

    add_plane_property(drm,req, plane_id, "FB_ID",   fb_id);
    add_plane_property(drm,req, plane_id, "CRTC_ID", drm->crtc_id);
    add_plane_property(drm,req, plane_id, "SRC_X",   0);
    add_plane_property(drm,req, plane_id, "SRC_Y",   0);
    add_plane_property(drm,req, plane_id, "SRC_W",   drm->mode->hdisplay << 16);
    add_plane_property(drm,req, plane_id, "SRC_H",   drm->mode->vdisplay << 16);
    add_plane_property(drm,req, plane_id, "CRTC_X",  0);
    add_plane_property(drm,req, plane_id, "CRTC_Y",  0);
    add_plane_property(drm,req, plane_id, "CRTC_W",  drm->mode->hdisplay);
    add_plane_property(drm,req, plane_id, "CRTC_H",  drm->mode->vdisplay);

    ret = drmModeAtomicCommit(drm->fd, req, DRM_MODE_PAGE_FLIP_EVENT, drm);
    drmModeAtomicFree(req);
    return ret;
}

static void
flip_handler(
    int fd,
    unsigned frame,
    unsigned sec,
    unsigned usec,
    void *data
    )
{
    struct _DRMDisplay* drm = data;
    drm->pending--;
}

static void
wait_flip(
    struct _DRMDisplay* drm
    )
{
    const int timeout = -1;
    struct pollfd fds;
    drmEventContext ev;

    fds.fd = drm->fd;
    fds.revents = 0;
    fds.events = POLLIN;

    ev.version = 2;
    ev.page_flip_handler = flip_handler;

    if (poll(&fds, 1, timeout) < 0)
    {
        return;
    }

    if (fds.revents & (POLLHUP | POLLERR))
    {
        return;
    }

    if (fds.revents & POLLIN)
    {
        drmHandleEvent(drm->fd, &ev);
    }
}


static int
pageflip(
    struct _DRMDisplay * drm,
    uint32_t fb_id
    )
{
    int ret;

    if (drm->pending > 0)
    {
        wait_flip(drm);
    }

    ret = drm_atomic_commit(drm, fb_id);
    if (!ret)
    {
        drm->pending++;
    }

    return ret;
}

static struct _FBDisplay *displayStack = gcvNULL;
static pthread_mutex_t displayMutex;

static pthread_once_t onceControl = PTHREAD_ONCE_INIT;

static void
onceInit(
    void
    )
{
    pthread_mutexattr_t mta;

    /* Init mutex attribute. */
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

    /* Set display mutex as recursive. */
    pthread_mutex_init(&displayMutex, &mta);
    /* Destroy mta.*/
    pthread_mutexattr_destroy(&mta);
}

/*******************************************************************************
** Display. ********************************************************************
*/

static int
drm_initdisplay(
    struct _DRMDisplay * drm,
    int connectorIndex
    )
{
    drmModeRes *resources;
    drmModeConnector *connector = NULL;
    drmModeEncoder *encoder = NULL;
    int i, j, area, index;
    drmModePlaneRes *plane_resources;
    int ret;
    int plane_id = -1;
    gctBOOL found_primary = gcvFALSE;

    drm->fd = drmOpen("imx-drm", NULL);
    if (drm->fd < 0)
    {
        fprintf(stderr, "could not open drm device\n");
        return -1;
    }

    resources = drmModeGetResources(drm->fd);
    if (!resources)
    {
        fprintf(stderr, "drmModeGetResources failed: %s\n", strerror(errno));
        return -1;
    }

    for (i = 0, index=0; i < resources->count_connectors; ++i)
    {
        connector = drmModeGetConnector(drm->fd, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED)
        {
            if (connectorIndex == index)
            {
                break;
            }
            index++;
        }
        drmModeFreeConnector(connector);
        connector = NULL;
    }

    if (!connector)
    {
        fprintf(stderr, "no connected connector!\n");
        return -1;
    }

    for (i = 0, area = 0; i < connector->count_modes; ++i)
    {
        drmModeModeInfo *current_mode = &connector->modes[i];
        int current_area = current_mode->hdisplay * current_mode->vdisplay;

        if (current_area > area)
        {
            drm->mode = current_mode;
            drm->height = current_mode->vdisplay;
            drm->width = current_mode->hdisplay;
            area = current_area;
        }
    }
    if (!drm->mode)
    {
        fprintf(stderr, "could not find mode!\n");
        return -1;
    }

    for (i = 0; i < resources->count_encoders; ++i)
    {
        encoder = drmModeGetEncoder(drm->fd, resources->encoders[i]);
        if (encoder->encoder_id == connector->encoder_id)
        {
            break;
        }
        drmModeFreeEncoder(encoder);
        encoder = NULL;
    }

    if (!encoder)
    {
        fprintf(stderr, "could not find encoder!\n");
        return -1;
    }
    drm->crtc_id = encoder->crtc_id;
    drm->connector_id = connector->connector_id;

    for (i = 0; i < resources->count_crtcs; ++i)
    {
        if (resources->crtcs[i] == drm->crtc_id)
        {
            drm->crtc_index = i;
            break;
        }
    }
    drmModeFreeResources(resources);

    drm->atomic_modeset = gcvTRUE;
    ret = drmSetClientCap(drm->fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret)
    {
        drm->atomic_modeset = gcvFALSE;
        return -1;
    }

    drmSetClientCap(drm->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    plane_resources =  drmModeGetPlaneResources(drm->fd);
    if (!plane_resources)
    {
        printf("drmModeGetPlaneResources failed\n");
        return -1;
    }

    for (i = 0; i < plane_resources->count_planes && !found_primary; i++)
    {
        uint32_t id = plane_resources->planes[i];
        drmModePlanePtr plane = drmModeGetPlane(drm->fd, id);

        if (!plane)
        {
            printf("drmModeGetPlane(%u) failed: %s\n", id, strerror(errno));
            continue;
        }

        if (plane->possible_crtcs & (1 << drm->crtc_index))
        {
            drmModeObjectProperties* props = drmModeObjectGetProperties(drm->fd, id, DRM_MODE_OBJECT_PLANE);
            plane_id = id;

            for (j = 0; j < props->count_props; j++)
            {
                drmModePropertyPtr p = drmModeGetProperty(drm->fd, props->props[j]);

                if ((strcmp(p->name, "type") == 0) &&
                    (props->prop_values[j] == DRM_PLANE_TYPE_PRIMARY))
                {
                    found_primary = gcvTRUE;
                }
                drmModeFreeProperty(p);
            }
            drmModeFreeObjectProperties(props);
        }
        drmModeFreePlane(plane);
    }
    drmModeFreePlaneResources(plane_resources);
    drm->plane_id = plane_id;

    if (drm->atomic_modeset)
    {
        get_resource(plane, Plane, plane_id);
        get_resource(crtc, Crtc, drm->crtc_id);
        get_resource(connector, Connector, drm->connector_id);

        get_properties(plane, PLANE, plane_id);
        get_properties(crtc, CRTC, drm->crtc_id);
        get_properties(connector, CONNECTOR, drm->connector_id);
    }

    drm->pending = 0;
    return 0;
}

static void
drm_destroy_fb(
    int drmfd,
    struct drm_fb * drmfb
    )
{
    if (drmfb)
    {
        if (drmfb->map)
        {
            munmap(drmfb->map, drmfb->size);
            drmfb->map = gcvNULL;
        }

        if (drmfb->fd >= 0)
        {
            close(drmfb->fd);
            drmfb->fd = -1;
        }

        if (drmfb->fb_id)
        {
            drmModeRmFB(drmfd, drmfb->fb_id);
            drmfb->fb_id = 0;
        }

        if (drmfb->handle)
        {
            struct drm_mode_destroy_dumb dreq = {
                .handle = drmfb->handle
            };
            drmIoctl(drmfd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        }
    }
}

static int
drm_create_fb(
    int drmfd,
    gctBOOL_PTR fbPrefetch,
    int width,
    int height,
    uint32_t bpp,
    struct drm_fb * drmfb
    )
{
    int ret = -1;
    uint32_t format = (bpp == 16) ? DRM_FORMAT_RGB565 : DRM_FORMAT_XRGB8888;
    gceSTATUS status = gcvSTATUS_OK;

#if defined(DRM_FORMAT_MOD_VIVANTE_SUPER_TILED)
    if (*fbPrefetch)
    {
        uint32_t handles[4] = {0};
        uint32_t strides[4] = {0};
        uint32_t offsets[4] = {0};
        uint64_t modifiers[4] = {0};

        /* For DRM_FORMAT_MOD_VIVANTE_SUPER_TILED need to have 64 pixels aligned*/
        struct drm_mode_create_dumb creq = {
            .width  = gcmALIGN(width,  64),
            .height = gcmALIGN(height, 64),
            .bpp    = bpp
        };

        ret = drmIoctl(drmfd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
        if (ret < 0)
        {
            fprintf(stderr, "cannot create dumb buffer (%d): %m\n", errno);
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
        drmfb->handle = creq.handle;
        drmfb->width = creq.width;
        drmfb->height = creq.height;
        drmfb->stride = creq.pitch;
        drmfb->size = creq.size;
        drmfb->bpp = creq.bpp;

        handles[0] = creq.handle;
        strides[0] = creq.pitch;
        modifiers[0] = DRM_FORMAT_MOD_VIVANTE_SUPER_TILED;

        ret = drmModeAddFB2WithModifiers(drmfd, creq.width, creq.height,
                                         format, handles, strides, offsets,
                                         modifiers, &drmfb->fb_id, DRM_MODE_FB_MODIFIERS);
        if (ret < 0)
        {
            struct drm_mode_destroy_dumb dreq = {
                .handle = drmfb->handle
            };

            fprintf(stderr, "FB Modifier not supported. Falling back to Linear FB\n");
            drmIoctl(drmfd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
            drmfb->handle = 0;
        }
    }
#endif

    if (ret < 0)
    {
        uint32_t handles[4] = {0};
        uint32_t strides[4] = {0};
        uint32_t offsets[4] = {0};

        struct drm_mode_create_dumb creq = {
            .width  = width,
            .height = height,
            .bpp    = bpp
        };

        ret = drmIoctl(drmfd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
        if (ret < 0)
        {
            fprintf(stderr, "cannot create dumb buffer (%d): %m\n", errno);
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
        drmfb->handle = creq.handle;
        drmfb->width = creq.width;
        drmfb->height = creq.height;
        drmfb->stride = creq.pitch;
        drmfb->size = creq.size;
        drmfb->bpp = creq.bpp;

        handles[0] = creq.handle;
        strides[0] = creq.pitch;

        ret = drmModeAddFB2(drmfd, creq.width, creq.height, format,
                            handles, strides, offsets, &drmfb->fb_id, 0);
        if (ret)
        {
            ret = -errno;
            fprintf(stderr, "cannot create framebuffer (%d): %m\n", errno);
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
        *fbPrefetch = gcvFALSE;
    }

    if (drmPrimeHandleToFD(drmfd, drmfb->handle, O_RDWR, &drmfb->fd))
    {
        fprintf(stderr, "cannot export dumb buffer as dma-buf fd (%d): %m\n", errno);
        ret = -errno;
        gcmONERROR(gcvSTATUS_GENERIC_IO);
    }
    else
    {
        struct drm_mode_map_dumb mreq = {
            .handle = drmfb->handle
        };

        /* prepare buffer for memory mapping */
        ret = drmIoctl(drmfd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
        if (ret)
        {
            ret = -errno;
            fprintf(stderr, "cannot map dumb buffer (%d): %m\n", errno);
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }

        /* perform actual memory mapping */
        drmfb->map = mmap(0, drmfb->size, PROT_READ | PROT_WRITE, MAP_SHARED, drmfd, mreq.offset);
        if (drmfb->map == MAP_FAILED)
        {
            ret = -errno;
            fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n", errno);
            gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    }

OnError:
    if (gcmIS_ERROR(status))
    {
        drm_destroy_fb(drmfd, drmfb);
    }

    return ret;
}

static void
_destroy_display(
    struct _FBDisplay* display
    )
{
    if (display)
    {
        int i = 0;

        for (i = 0; i< display->multiBuffer; ++i)
        {
            drm_destroy_fb(display->drm.fd, &display->drmfbs[i]);
        }

        if (display->drm.fd >= 0)
        {
            drmClose(display->drm.fd);
            display->drm.fd = -1;
        }

        if (display->file >= 0)
        {
            close(display->file);
            display->file = -1;
        }

        pthread_mutex_destroy(&(display->condMutex));
        pthread_cond_destroy(&(display->cond));

        if (display->drmfbs)
        {
            gcmOS_SAFE_FREE(gcvNULL, display->drmfbs);
        }

        gcmOS_SAFE_FREE(gcvNULL, display);
        display = gcvNULL;
    }
}

gceSTATUS
drmfb_GetDisplayByIndex(
    IN gctINT DisplayIndex,
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    struct _FBDisplay* display = NULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    /* Init global variables. */
    pthread_once(&onceControl, onceInit);

    /* Lock display stack. */
    pthread_mutex_lock(&displayMutex);

    {
        char *p;
        int i;
        char fbDevName[256];
        struct drm_fb *drmfb = NULL;
        int connectorIndex = 0;
        gctBOOL defaultMultiBuffer = gcvTRUE;

        if (DisplayIndex < 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        for (display = displayStack; display != NULL; display = display->next)
        {
            if (display->index == DisplayIndex)
            {
                /* Find display.*/
                display->refCount++;
                goto OnExit;
            }
        }

        gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof (struct _FBDisplay), (gctPOINTER *)&display));

        display->signature = gcmCC('F', 'B', 'D', 'V');
        display->index    = DisplayIndex;
        display->memory   = gcvNULL;
        display->file     = -1;
        display->tiling   = gcvLINEAR;
        display->bufferStatus = 0;
        display->multiBuffer = 3;
        display->drm.fd = -1;

#if defined(MXC_FBDEV) && MXC_FBDEV
        display->fbPrefetch = gcvTRUE;
        p = getenv("GPU_VIV_EXT_RESOLVE");
        if ((p != gcvNULL) && (p[0] == '0'))
        {
           /* Disable resolve requested. */
            display->fbPrefetch = gcvFALSE;
        }
#endif

        p = getenv("FB_MULTI_BUFFER");
        if (p != NULL)
        {
            display->multiBuffer = atoi(p);
            if (display->multiBuffer < 1)
            {
                display->multiBuffer = 1;
            }
            /* FSL: limit max buffer count. */
            else if (display->multiBuffer > 8)
            {
                display->multiBuffer = 8;
            }
            defaultMultiBuffer = gcvFALSE;
        }

        if (display->file < 0)
        {
            unsigned char i = 0;
            char const * const fbDevicePath[] =
            {
                "/dev/fb%d",
                "/dev/graphics/fb%d",
                gcvNULL
            };

            /* Create a handle to the device. */
            while ((display->file == -1) && fbDevicePath[i])
            {
                sprintf(fbDevName, fbDevicePath[i], DisplayIndex);
                display->file = open(fbDevName, O_RDWR);
                i++;
            }
        }

        if (display->file < 0)
        {
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        /* Get variable framebuffer information. */
        if (ioctl(display->file, FBIOGET_VSCREENINFO, &display->varInfo) < 0)
        {
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }

        sprintf(fbDevName,"FB_FRAMEBUFFER_%d", DisplayIndex);
        p = getenv(fbDevName);

        if (p != NULL)
        {
            connectorIndex = atoi(p);
            if (connectorIndex < 0)
            {
                connectorIndex = 0;
            }
        }

        if (drm_initdisplay(&display->drm, connectorIndex) < 0)
        {
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }

        gcmONERROR(gcoOS_Allocate(gcvNULL, display->multiBuffer * sizeof(struct drm_fb), (gctPOINTER *)&display->drmfbs));

        for (i = 0; i < display->multiBuffer; ++i)
        {
            display->drmfbs[i].fd = -1;

            if (drm_create_fb(display->drm.fd, &display->fbPrefetch, display->drm.width,
                              display->drm.height, display->varInfo.bits_per_pixel,
                              &display->drmfbs[i]) < 0)
            {
                gcmONERROR(gcvSTATUS_GENERIC_IO);
            }
            drmfb = &display->drmfbs[i];

            /* Enable FB_MULTI_BUFFER default only when fbPrefetch enabled,
            ** otherwise fallback single buffer for benchmarks
            */
            if (defaultMultiBuffer && !display->fbPrefetch)
            {
                display->multiBuffer = 1;
            }
        }

        if (display->multiBuffer == 1)
        {
            if (drmModeSetPlane(display->drm.fd, display->drm.plane_id, display->drm.crtc_id, drmfb->fb_id,
                                0, 0, 0, display->drm.width, display->drm.height,
                                0, 0, display->drm.width << 16, display->drm.height << 16))
            {
                fprintf(stderr, "failed to enable plane: %s\n", strerror(errno));
                gcmONERROR(gcvSTATUS_GENERIC_IO);
            }
            drmDropMaster(display->drm.fd);
        }

        /* Default aligned height, equals to y resolution. */
        display->alignedHeight = drmfb->height;

        /* Get current virtual screen size. */
        if (ioctl(display->file, FBIOGET_VSCREENINFO, &(display->varInfo)) < 0)
        {
            gcmONERROR(gcvSTATUS_GENERIC_IO);
        }
        display->stride = drmfb->stride;
        display->size   = drmfb->size;
        display->width  = drmfb->width;
        display->height = drmfb->height;
        display->bpp    = drmfb->bpp;
        display->memory = drmfb->map;

        /*display->format = gcvSURF_X8R8G8B8*/
        display->varInfo.yres_virtual *= display->multiBuffer;
        if (display->multiBuffer > 1)
        {
            /* Calculate actual buffer count. */
            display->multiBuffer = display->varInfo.yres_virtual
                                 / display->alignedHeight;
        }

        /* Compute aligned height of one backBuffer. */
        display->alignedHeight = display->varInfo.yres_virtual
                               / display->multiBuffer;

        /* Move to off-screen memory. */
        if (display->varInfo.yoffset % display->alignedHeight == 0)
        {
            display->backBufferY = display->varInfo.yoffset
                                 + display->alignedHeight;
        }
        else
        {
            display->varInfo.yoffset = 0;
            display->backBufferY = display->alignedHeight;
        }

        if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
        {
            /* Warp around. */
            display->backBufferY = 0;
        }

        /* Get the color format. */
        switch (display->varInfo.green.length)
        {
        case 4:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X4B4G4R4 : gcvSURF_A4B4G4R4;
            }
            break;

        case 5:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X1B5G5R5 : gcvSURF_A1B5G5R5;
            }
            break;

        case 6:
            display->format = gcvSURF_R5G6B5;
            break;

        case 8:
            if (display->varInfo.blue.offset == 0)
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
            }
            else
            {
                display->format = (display->varInfo.transp.length == 0) ? gcvSURF_X8B8G8R8 : gcvSURF_A8B8G8R8;
            }
            break;

        default:
            display->format = gcvSURF_UNKNOWN;
            gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
            break;
        }

        display->tiling = gcvLINEAR;

        /* Get the color info. */
        display->alphaLength = display->varInfo.transp.length;
        display->alphaOffset = display->varInfo.transp.offset;

        display->redLength   = display->varInfo.red.length;
        display->redOffset   = display->varInfo.red.offset;

        display->greenLength = display->varInfo.green.length;
        display->greenOffset = display->varInfo.green.offset;

        display->blueLength  = display->varInfo.blue.length;
        display->blueOffset  = display->varInfo.blue.offset;

        display->refCount = 0;

        /* Find a way to detect if pan display is with vsync. */
#if defined(MXC_FBDEV) && MXC_FBDEV
        display->panVsync = gcvTRUE;
#else
        display->panVsync = gcvFALSE;
#endif

        pthread_cond_init(&(display->cond), gcvNULL);
        pthread_mutex_init(&(display->condMutex), gcvNULL);

        /* Add display into stack. */
        display->next = displayStack;
        displayStack = display;
        display->refCount++;
    }

OnExit:
OnError:
    pthread_mutex_unlock(&displayMutex);

    if (gcmIS_ERROR(status) && display)
    {
        _destroy_display(display);
        display = gcvNULL;
    }

    *Display = (PlatformDisplayType)display;

    gcmFOOTER();
    return status;
}

gceSTATUS
drmfb_GetDisplay(
    OUT PlatformDisplayType * Display,
    IN gctPOINTER Context
    )
{
    return drmfb_GetDisplayByIndex(0, Display, Context);
}

gceSTATUS
drmfb_GetDisplayInfo(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctSIZE_T * Physical,
    OUT gctINT * Stride,
    OUT gctINT * BitsPerPixel
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    gcmHEADER_ARG("Display=0x%x", Display);
    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Width != gcvNULL)
    {
        *Width = display->width;
    }

    if (Height != gcvNULL)
    {
        *Height = display->height;
    }

    if (Physical != gcvNULL)
    {
        *Physical = 0;
    }

    if (Stride != gcvNULL)
    {
        *Stride = display->stride;
    }

    if (BitsPerPixel != gcvNULL)
    {
        *BitsPerPixel = display->bpp;
    }

    gcmFOOTER_NO();
    return status;
}

typedef struct _fbdevDISPLAY_INFO
{
    /* The size of the display in pixels. */
    int                         width;
    int                         height;

    /* The stride of the dispay. -1 is returned if the stride is not known
    ** for the specified display.*/
    int                         stride;

    /* The color depth of the display in bits per pixel. */
    int                         bitsPerPixel;

    /* Can be wraped as surface. */
    int                         wrapFB;

    /* FB_MULTI_BUFFER support */
    int                         multiBuffer;
    int                         backBufferY;

    /* Tiled buffer / tile status support. */
    int                         tiledBuffer;
    int                         tileStatus;
    int                         compression;

    /* The color info of the display. */
    unsigned int                alphaLength;
    unsigned int                alphaOffset;
    unsigned int                redLength;
    unsigned int                redOffset;
    unsigned int                greenLength;
    unsigned int                greenOffset;
    unsigned int                blueLength;
    unsigned int                blueOffset;

    /* Display flip support. */
    int                         flip;
}
fbdevDISPLAY_INFO;

gceSTATUS
drmfb_GetDisplayInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT fbdevDISPLAY_INFO * DisplayInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display = (struct _FBDisplay*)Display;

    gcmHEADER_ARG("Display=0x%x Window=0x%x DisplayInfoSize=%u", Display, Window, DisplayInfoSize);

    /* Valid display? and structure size? */
    if ((display == gcvNULL) || (DisplayInfoSize != sizeof(fbdevDISPLAY_INFO)))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (display->memory == gcvNULL)
    {
        /* No offset. */
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    /* Return the size of the display. */
    DisplayInfo->width  = display->width;
    DisplayInfo->height = display->height;

    /* Return the stride of the display. */
    DisplayInfo->stride = display->stride;

    /* Return the color depth of the display. */
    DisplayInfo->bitsPerPixel = display->bpp;

    /* Return the color info. */
    DisplayInfo->alphaLength = display->alphaLength;
    DisplayInfo->alphaOffset = display->alphaOffset;
    DisplayInfo->redLength   = display->redLength;
    DisplayInfo->redOffset   = display->redOffset;
    DisplayInfo->greenLength = display->greenLength;
    DisplayInfo->greenOffset = display->greenOffset;
    DisplayInfo->blueLength  = display->blueLength;
    DisplayInfo->blueOffset  = display->blueOffset;

    /* Determine flip support. */
    DisplayInfo->flip = (display->multiBuffer > 1);

    /* 355_FB_MULTI_BUFFER */
    DisplayInfo->multiBuffer = Display->multiBuffer;
    DisplayInfo->backBufferY = Display->backBufferY;

    DisplayInfo->wrapFB = gcvTRUE;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
drmfb_GetDisplayVirtual(
    IN PlatformDisplayType Display,
    OUT gctINT * Width,
    OUT gctINT * Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x", Display);

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        /* Bad display pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer < 1)
    {
        /* Turned off. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER_NO();
        return status;
    }

    /* Save size of virtual buffer. */
    *Width  = display->varInfo.xres_virtual;
    *Height = display->varInfo.yres_virtual;

    /* Success. */
    gcmFOOTER_ARG("*Width=%d *Height=%d", *Width, *Height);
    return status;
}

gceSTATUS
drmfb_GetDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    struct _FBWindow* window;
    gctINT curIndex, panIndex;

    gcmHEADER_ARG("Display=%p Window=%p", Display, Window);

    display = (struct _FBDisplay*)Display;
    window = (struct _FBWindow*)Window;
    if (!display || !window)
    {
        /* Invalid display or window pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer <= 1)
    {
        /* Turned off. */
        status = gcvSTATUS_NOT_SUPPORTED;
        gcmFOOTER_NO();
        return status;
    }

    pthread_mutex_lock(&(display->condMutex));

    /* Return current back buffer. */
    *X = 0;
    *Y = display->backBufferY;

    /* FSL: need wait next two PANs to ensure the buffer is not in use. */
    curIndex = display->backBufferY / display->alignedHeight;
    panIndex = (curIndex + 2) % display->multiBuffer;

    /* if swap interval is zero do not wait for the back buffer to change */
    if (window->swapInterval != 0)
    {
        /* For multiBuffer <3, the work flow will be in the same thread */
        /* There is no need to check the status. */
        if (display->multiBuffer >= 3)
        {
            while ((display->bufferStatus & (1 << panIndex)) != 0)
            {
                pthread_cond_wait(&(display->cond), &(display->condMutex));
            }

            display->bufferStatus |= 1 << curIndex;
        }
    }

    /* Move to next back buffer. */
    display->backBufferY += display->alignedHeight;

    if (display->backBufferY >= (int)(display->varInfo.yres_virtual))
    {
        /* Wrap around. */
        display->backBufferY = 0;
    }

    pthread_mutex_unlock(&(display->condMutex));

    /* Success. */
    gcmFOOTER_ARG("*Offset=%u *X=%d *Y=%d", *Offset, *X, *Y);
    return status;
}

gceSTATUS
drmfb_SetDisplayVirtual(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    struct _FBWindow* window;

    gcmHEADER_ARG("Display=%p Window=%p Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    window = (struct _FBWindow*)Window;
    if (window == NULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    /*
     * This section may be called in a different thread, while main thread is
     * trying to free the display struct.
     * We must protect the display struct when the thread is running inside.
     */
    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }
    if (display == NULL)
    {
        pthread_mutex_unlock(&displayMutex);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer > 1)
    {
        /* clamp swap interval to be safe */
        gctINT swapInterval = window->swapInterval;

        /* if swap interval is 0 skip this step */
        if (swapInterval != 0 || !display->panVsync)
        {
            pthread_mutex_lock(&display->condMutex);

            /* Set display offset. */
            display->varInfo.xoffset  = X;
            display->varInfo.yoffset  = Y;
            display->varInfo.activate = FB_ACTIVATE_VBL;

            /* FSL: ywrap required. */
            if (display->varInfo.yoffset % display->varInfo.yres != 0)
            {
                display->varInfo.vmode |= FB_VMODE_YWRAP;
            }
            else
            {
                display->varInfo.vmode &= ~FB_VMODE_YWRAP;
            }

            {
                gctINT index = display->varInfo.yoffset / display->alignedHeight;

                if (display->drm.atomic_modeset)
                {
                    if (pageflip(&display->drm,display->drmfbs[index].fb_id))
                    {
                        fprintf(stderr, "failed to flip page: %s\n", strerror(errno));
                        return -1;
                    }
                }
                else if (drmModeSetPlane(display->drm.fd, display->drm.plane_id,
                                         display->drm.crtc_id, display->drmfbs[index].fb_id,
                                         0, 0, 0, display->drm.width, display->drm.height,
                                         0, 0, display->drm.width << 16, display->drm.height << 16))
                {
                    fprintf(stderr, "failed to enable plane: %s\n", strerror(errno));
                    return -1;
                }

                /* FSL: buffer PAN'ed. */
                display->bufferStatus &= ~(1<<index);
            }

            pthread_cond_broadcast(&display->cond);
            pthread_mutex_unlock(&display->condMutex);
        }
    }

    pthread_mutex_unlock(&displayMutex);

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_SetDisplayVirtualEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    return drmfb_SetDisplayVirtual(Display, Window, Offset, X, Y);
}

gceSTATUS
drmfb_CancelDisplayBackbuffer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Offset=%u X=%d Y=%d", Display, Window, Offset, X, Y);

    /*
     * This section may be called in a different thread, while main thread is
     * trying to free the display struct.
     * We must protect the display struct when the thread is running inside.
     */
    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    if (display == NULL)
    {
        pthread_mutex_unlock(&displayMutex);
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (display->multiBuffer > 1)
    {
        gctINT next;
        gctINT index;
        pthread_mutex_lock(&display->condMutex);

        next = (Y + display->alignedHeight);
        if (next >= (int) display->varInfo.yres_virtual)
        {
            next = 0;
        }

        if (next != display->backBufferY)
        {
            gcmPRINT("%s: Canceling non-last buffer", __func__);
        }

        /* Roll back the buffer. */
        display->backBufferY = Y;

        index = Y / display->alignedHeight;
        display->bufferStatus &= ~(1<<index);

        pthread_cond_broadcast(&display->cond);
        pthread_mutex_unlock(&display->condMutex);
    }

    pthread_mutex_unlock(&displayMutex);

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_SetSwapInterval(
    IN PlatformWindowType Window,
    IN gctINT Interval
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBWindow * window;

    gcmHEADER_ARG("Window=%p Interval=%d", Window, Interval);

    window = (struct _FBWindow*)Window;
    if (window == gcvNULL)
    {
        /* Invalid window pointer. */
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    window->swapInterval = gcmCLAMP(Interval, GC_FB_MIN_SWAP_INTERVAL, GC_FB_MAX_SWAP_INTERVAL);

    /* Success. */
    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_GetSwapInterval(
    IN PlatformDisplayType Display,
    IN gctINT_PTR Min,
    IN gctINT_PTR Max
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;

    gcmHEADER_ARG("Display=0x%x Min=0x%x Max=0x%x", Display, Min, Max);

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
       /* Invalid display pointer. */
       status = gcvSTATUS_INVALID_ARGUMENT;
       gcmFOOTER();
       return status;
    }

    if (Min != gcvNULL)
    {
        *Min = GC_FB_MIN_SWAP_INTERVAL;
    }

    if (Max != gcvNULL)
    {
        *Max = GC_FB_MAX_SWAP_INTERVAL;
    }

    /* Success. */
    gcmFOOTER_NO();
    return status;

}

gceSTATUS
drmfb_DestroyDisplay(
    IN PlatformDisplayType Display
    )
{
    struct _FBDisplay* display = gcvNULL;

    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    if (display && (--display->refCount) == 0)
    {
        /* Unlink form display stack. */
        if (display == displayStack)
        {
            /* First one. */
            displayStack = display->next;
        }
        else
        {
            struct _FBDisplay* prev = displayStack;

            while (prev->next != display)
            {
                prev = prev->next;
            }

            prev->next = display->next;
        }
    }
    else
    {
        display = gcvNULL;
    }

    pthread_mutex_unlock(&displayMutex);

    if (display)
    {
        _destroy_display(display);
    }

    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_DisplayBufferRegions(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT NumRects,
    IN gctINT_PTR Rects
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

/*******************************************************************************
** Windows. ********************************************************************
*/

gceSTATUS
drmfb_CreateWindow(
    IN PlatformDisplayType Display,
    IN gctINT X,
    IN gctINT Y,
    IN gctINT Width,
    IN gctINT Height,
    OUT PlatformWindowType * Window
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    struct _FBDisplay * display;
    struct _FBWindow *  window;
    gctINT              ignoreDisplaySize = 0;
    gctCHAR *           p;

    gcmHEADER_ARG("Display=%p X=%d Y=%d Width=%d Height=%d",
                  Display, X, Y, Width, Height);

    display = (struct _FBDisplay*) Display;
    if (display == NULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    p = getenv("FB_IGNORE_DISPLAY_SIZE");
    if (p != gcvNULL)
    {
        ignoreDisplaySize = atoi(p);
    }

    if (Width == 0)
    {
        Width = display->width;
    }

    if (Height == 0)
    {
        Height = display->height;
    }

    if (X == -1)
    {
        X = ((display->width - Width) / 2) & ~15;
    }

    if (Y == -1)
    {
        Y = ((display->height - Height) / 2) & ~7;
    }

    if (X < 0) X = 0;
    if (Y < 0) Y = 0;
    if (!ignoreDisplaySize)
    {
        if (X + Width  > display->width)  Width  = display->width  - X;
        if (Y + Height > display->height) Height = display->height - Y;
    }

#if defined(MXC_FBDEV) && MXC_FBDEV
    /* FSL: external resolve. */
    do
    {
        /*int err;*/
        int extResolve = 1;
        /*struct fb_var_screeninfo varInfo;*/

        if (!display->fbPrefetch)
        {
            /* No prefetch in hardware. */
            break;
        }

        if ((X != 0) || (Y != 0) ||
            (Width != display->width) || (Height != display->height))
        {
            /* Not full screen, can not enable. */
            extResolve = 0;
        }

        /* Check window size alignment. */
        if ((display->varInfo.xres_virtual & 0x3F) ||
            (display->varInfo.yres_virtual & 0x3F))
        {
            extResolve = 0;
        }
        display->tiling  = extResolve ? gcvSUPERTILED : gcvLINEAR;
    }
    while (0);
#endif

    do
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof (struct _FBWindow), (gctPOINTER *)&window)))
        {
            status = gcvSTATUS_OUT_OF_RESOURCES;
            break;
        }
        window->offset = Y * display->stride + X * ((display->bpp + 7) / 8);

        window->display = display;
        window->format = display->format;
        window->x = X;
        window->y = Y;

        window->width  = Width;
        window->height = Height;
        window->swapInterval = 1;
        *Window = (PlatformWindowType) window;
    }
    while (0);

    gcmFOOTER_ARG("*Window=0x%x", *Window);
    return status;
}

gceSTATUS
drmfb_GetWindowInfo(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBWindow* window;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);
    window = (struct _FBWindow*)Window;
    if (window == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (X != gcvNULL)
    {
        *X = window->x;
    }

    if (Y != gcvNULL)
    {
        *Y = window->y;
    }

    if (Width != gcvNULL)
    {
        *Width = window->width;
    }

    if (Height != gcvNULL)
    {
        *Height = window->height;
    }

    if (BitsPerPixel != gcvNULL)
    {
        *BitsPerPixel = window->display->bpp;
    }

    if (Offset != gcvNULL)
    {
        *Offset = window->offset;
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_DestroyWindow(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    if (Window != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, Window);
    }
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_DrawImage(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    unsigned char * ptr;
    int y;
    unsigned int bytes     = (Right - Left) * BitsPerPixel / 8;
    unsigned char * source = (unsigned char *) Bits;
    struct _FBDisplay* display;
    struct _FBWindow* window;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Display=0x%x Window=0x%x Left=%d Top=%d Right=%d Bottom=%d Width=%d Height=%d BitsPerPixel=%d Bits=0x%x",
                  Display, Window, Left, Top, Right, Bottom, Width, Height, BitsPerPixel, Bits);

    window = (struct _FBWindow*)Window;

    if ((window == gcvNULL) || (Bits == gcvNULL))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }
    else
    {
        display = window->display;
    }

    ptr = (unsigned char *) display->memory + (Left * display->bpp / 8);

    if (BitsPerPixel != display->bpp)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Height < 0)
    {
        for (y = Bottom - 1; y >= Top; --y)
        {
            unsigned char * line = ptr + y * display->stride;
            memcpy(line, source, bytes);

            source += Width * BitsPerPixel / 8;
        }
    }
    else
    {
        for (y = Top; y < Bottom; ++y)
        {
            unsigned char * line = ptr + y * display->stride;
            memcpy(line, source, bytes);

            source += Width * BitsPerPixel / 8;
        }
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_GetImage(
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    OUT gctINT * BitsPerPixel,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}



/*******************************************************************************
** Pixmaps. ********************************************************************
*/

gceSTATUS
drmfb_CreatePixmap(
    IN PlatformDisplayType Display,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    OUT PlatformPixmapType * Pixmap
    )
{
    gceSTATUS status = gcvSTATUS_OK;
#if !gcdUSE_PIXMAP_SURFACE
    gctINT alignedWidth, alignedHeight;
#endif
    struct _FBPixmap* pixmap;
#if gcdUSE_PIXMAP_SURFACE
    gceSURF_FORMAT format;
    gctPOINTER memAddr[3] = {gcvNULL};
#endif
    gcmHEADER_ARG("Display=0x%x Width=%d Height=%d BitsPerPixel=%d", Display, Width, Height, BitsPerPixel);

    if ((Width <= 0) || (Height <= 0) || (BitsPerPixel <= 0))
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    do
    {
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof (struct _FBPixmap), (gctPOINTER *)&pixmap)))
        {
            break;
        }
        gcoOS_ZeroMemory(pixmap, sizeof (struct _FBPixmap));
#if gcdUSE_PIXMAP_SURFACE
        if (BitsPerPixel <= 16)
        {
            format = gcvSURF_R5G6B5;
        }
        else if (BitsPerPixel == 24)
        {
            format = gcvSURF_X8R8G8B8;
        }
        else
        {
            format = gcvSURF_A8R8G8B8;
        }

        gcmERR_BREAK(gcoSURF_Construct(
            gcvNULL,
            Width, Height, 1,
            gcvSURF_BITMAP,
            format,
            gcvPOOL_DEFAULT,
            &pixmap->surface
        ));

        gcmERR_BREAK(gcoSURF_Lock(
            pixmap->surface,
                    &pixmap->gpu,
            memAddr
        ));
        pixmap->bits = memAddr[0];

        gcmERR_BREAK(gcoSURF_GetSize(
            pixmap->surface,
            (gctUINT *) &pixmap->width,
            (gctUINT *) &pixmap->height,
            gcvNULL
        ));

        gcmERR_BREAK(gcoSURF_GetAlignedSize(
            pixmap->surface,
            gcvNULL,
            gcvNULL,
            &pixmap->stride
        ));

        pixmap->bpp = (BitsPerPixel <= 16) ? 16 : 32;
#else
        alignedWidth   = (Width + 0x0F) & (~0x0F);
        alignedHeight  = (Height + 0x3) & (~0x03);
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, alignedWidth * alignedHeight * (BitsPerPixel + 7) / 8 + 64, (gctPOINTER *)&pixmap->original)))
        {
            gcmOS_SAFE_FREE(gcvNULL, pixmap);
            pixmap = gcvNULL;

            break;
        }
        pixmap->bits = (gctPOINTER)(((gctUINTPTR_T)(gctCHAR*)pixmap->original + 0x3F) & (~0x3F));

        pixmap->width = Width;
        pixmap->height = Height;
        pixmap->alignedWidth    = alignedWidth;
        pixmap->alignedHeight   = alignedHeight;
        pixmap->bpp     = (BitsPerPixel + 0x07) & (~0x07);
        pixmap->stride  = Width * (pixmap->bpp) / 8;
#endif

        *Pixmap = (PlatformPixmapType)pixmap;
        gcmFOOTER_ARG("*Pixmap=0x%x", *Pixmap);
        return status;
    }
    while (0);

#if gcdUSE_PIXMAP_SURFACE
    if (pixmap!= gcvNULL)
    {
        if (pixmap->bits != gcvNULL)
        {
            gcoSURF_Unlock(pixmap->surface, pixmap->bits);
        }
        if (pixmap->surface != gcvNULL)
        {
            gcoSURF_Destroy(pixmap->surface);
        }
        gcmOS_SAFE_FREE(gcvNULL, pixmap);
    }
#endif

    status = gcvSTATUS_OUT_OF_RESOURCES;
    gcmFOOTER();
    return status;
}

gceSTATUS
drmfb_GetPixmapInfo(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBPixmap* pixmap;
    gcmHEADER_ARG("Display=0x%x Pixmap=0x%x", Display, Pixmap);
    pixmap = (struct _FBPixmap*)Pixmap;
    if (pixmap == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmFOOTER();
        return status;
    }

    if (Width != NULL)
    {
        *Width = pixmap->width;
    }

    if (Height != NULL)
    {
        *Height = pixmap->height;
    }

    if (BitsPerPixel != NULL)
    {
        *BitsPerPixel = pixmap->bpp;
    }

    if (Stride != NULL)
    {
        *Stride = pixmap->stride;
    }

    if (Bits != NULL)
    {
        *Bits = (pixmap->bits ? pixmap->bits : pixmap->original);
    }

    gcmFOOTER_NO();
    return status;
}

gceSTATUS
drmfb_DestroyPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap
    )
{
    struct _FBPixmap* pixmap;
    pixmap = (struct _FBPixmap*)Pixmap;
    if (pixmap != gcvNULL)
    {
#if gcdUSE_PIXMAP_SURFACE
        if (pixmap->bits != gcvNULL)
        {
            gcoSURF_Unlock(pixmap->surface, pixmap->bits);
        }
        if (pixmap->surface != gcvNULL)
        {
            gcoSURF_Destroy(pixmap->surface);
        }
#else
        if (pixmap->original != NULL)
        {
            gcmOS_SAFE_FREE(gcvNULL, pixmap->original);
        }
#endif
        gcmOS_SAFE_FREE(gcvNULL, pixmap);
        pixmap = gcvNULL;
    }
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_DrawPixmap(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_SetWindowTitle(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctCONST_STRING Title
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_CapturePointer(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_CreateClientBuffer(
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT Format,
    IN gctINT Type,
    OUT gctPOINTER * ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_GetClientBufferInfo(
    IN gctPOINTER ClientBuffer,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_DestroyClientBuffer(
    IN gctPOINTER ClientBuffer
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_InitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_DeinitLocalDisplayInfo(
    IN PlatformDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_GetDisplayBackbufferEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    return drmfb_GetDisplayBackbuffer(Display, Window, context, surface, Offset, X, Y);
}

gceSTATUS
drmfb_IsValidDisplay(
    IN PlatformDisplayType Display
    )
{
    struct _FBDisplay* display;

    pthread_mutex_lock(&displayMutex);

    for (display = displayStack; display != NULL; display = display->next)
    {
        if (display == (struct _FBDisplay*) Display)
        {
            /* Found display. */
            break;
        }
    }

    pthread_mutex_unlock(&displayMutex);

    if (display != NULL)
    {
        return gcvSTATUS_OK;
    }

    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
drmfb_GetNativeVisualId(
    IN PlatformDisplayType Display,
    OUT gctINT* nativeVisualId
    )
{
    *nativeVisualId = 0;
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_GetWindowInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    OUT gctINT * X,
    OUT gctINT * Y,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctUINT * Offset,
    OUT gceSURF_FORMAT * Format,
    OUT gceSURF_TYPE * Type
    )
{
    struct _FBDisplay* display;

    display = (struct _FBDisplay*)Display;

    /* Valid display? and structure size? */
    if (display == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (gcmIS_ERROR(drmfb_GetWindowInfo(
                          Display,
                          Window,
                          X,
                          Y,
                          (gctINT_PTR) Width,
                          (gctINT_PTR) Height,
                          (gctINT_PTR) BitsPerPixel,
                          gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Get the color format. */
    switch (display->greenLength)
    {
    case 4:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X4R4G4B4 : gcvSURF_A4R4G4B4;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X4B4G4R4 : gcvSURF_A4B4G4R4;
        }
        break;

    case 5:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X1R5G5B5 : gcvSURF_A1R5G5B5;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X1B5G5R5 : gcvSURF_A1B5G5R5;
        }
        break;

    case 6:
        *Format = gcvSURF_R5G6B5;
        break;

    case 8:
        if (display->blueOffset == 0)
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X8R8G8B8 : gcvSURF_A8R8G8B8;
        }
        else
        {
            *Format = (display->alphaLength == 0) ? gcvSURF_X8B8G8R8 : gcvSURF_A8B8G8R8;
        }
        break;

    default:
        /* Unsupported colot depth. */
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (Type != gcvNULL)
    {
        /* FSL: tiling. */
        switch (display->tiling)
        {
        case gcvLINEAR:
            *Type = gcvSURF_BITMAP;
            break;
        default:
            *Type = gcvSURF_RENDER_TARGET_NO_TILE_STATUS;
            break;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_DrawImageEx(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gctINT Left,
    IN gctINT Top,
    IN gctINT Right,
    IN gctINT Bottom,
    IN gctINT Width,
    IN gctINT Height,
    IN gctINT BitsPerPixel,
    IN gctPOINTER Bits,
    IN gceSURF_FORMAT Format
    )
{
    return drmfb_DrawImage(Display,
                           Window,
                           Left,
                           Top,
                           Right,
                           Bottom,
                           Width,
                           Height,
                           BitsPerPixel,
                           Bits);
}

gceSTATUS
drmfb_SetWindowFormat(
    IN PlatformDisplayType Display,
    IN PlatformWindowType Window,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format
    )
{
#if defined(MXC_FBDEV) && MXC_FBDEV
    /* FSL: external resolve. */
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display;
    struct _FBWindow* window;
    int nonstd = 0;
    gceTILING tiling = gcvINVALIDTILED;
    gcoSURF tmpSurface = gcvNULL;
    gctUINT width;
    gctUINT height;

    display = (struct _FBDisplay*)Display;
    if (display == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (!display->fbPrefetch)
    {
        return gcvSTATUS_NOT_SUPPORTED;
    }

    window = (struct _FBWindow*)Window;
    if (window == gcvNULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if (Format != window->format)
    {
        /* Can not change format. */
        return gcvSTATUS_NOT_SUPPORTED;
    }

    switch ((int)Type)
    {
    case gcvSURF_RENDER_TARGET:
        return gcvSTATUS_NOT_SUPPORTED;

    case gcvSURF_RENDER_TARGET_NO_TILE_STATUS:
        if ((window->width  != display->width) ||
            (window->height != display->height))
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

        width  = (gctUINT) window->width;
        height = (gctUINT) window->height;

        status = gcoSURF_Construct(gcvNULL,
                                   width, height, 1,
                                   gcvSURF_RENDER_TARGET | gcvSURF_NO_VIDMEM,
                                   window->format,
                                   gcvPOOL_DEFAULT,
                                   &tmpSurface);

        if (gcmIS_SUCCESS(status))
        {
            gcoSURF_GetTiling(tmpSurface, &tiling);
            gcoSURF_Destroy(tmpSurface);
        }
        break;

    case gcvSURF_BITMAP:
        tiling = gcvLINEAR;
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    /* Translate tiling to nonstd. */
    switch (tiling)
    {
    case gcvTILED:
        if (display->bpp == 16)
        {
            nonstd = IPU_PIX_FMT_GPU16_ST;
        }
        else if (display->bpp == 32)
        {
            nonstd = IPU_PIX_FMT_GPU32_ST;
        }
        else
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        break;

    case gcvSUPERTILED:
        if (display->bpp == 16)
        {
            nonstd = IPU_PIX_FMT_GPU16_SRT;
        }
        else if (display->bpp == 32)
        {
            nonstd = IPU_PIX_FMT_GPU32_SRT;
        }
        else
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
        break;

    case gcvLINEAR:
        nonstd = 0;
        break;

    default:
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if ((display->varInfo.nonstd == nonstd) &&
        (display->tiling == tiling))
    {
        return gcvSTATUS_OK;
    }

    /* Prefetch mode/display tiling changed. */
    display->varInfo.nonstd = nonstd;
    display->tiling = tiling;

    return gcvSTATUS_OK;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

gceSTATUS
drmfb_GetPixmapInfoEx(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    OUT gctINT * Width,
    OUT gctINT * Height,
    OUT gctINT * BitsPerPixel,
    OUT gctINT * Stride,
    OUT gctPOINTER * Bits,
    OUT gceSURF_FORMAT * Format
    )
{
    if (gcmIS_ERROR(drmfb_GetPixmapInfo(Display,
                                        Pixmap,
                                        Width,
                                        Height,
                                        BitsPerPixel,
                                        gcvNULL,
                                        gcvNULL)))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Return format for pixmap depth. */
    switch (*BitsPerPixel)
    {
    case 16:
        *Format = gcvSURF_R5G6B5;
        break;

    case 32:
        *Format = gcvSURF_A8R8G8B8;
        break;

    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    /* Success. */
    return gcvSTATUS_OK;
}

gceSTATUS
drmfb_CopyPixmapBits(
    IN PlatformDisplayType Display,
    IN PlatformPixmapType Pixmap,
    IN gctUINT DstWidth,
    IN gctUINT DstHeight,
    IN gctINT DstStride,
    IN gceSURF_FORMAT DstFormat,
    OUT gctPOINTER DstBits
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gctBOOL
drmfb_SynchronousFlip(
    IN PlatformDisplayType Display
    )
{
    /* FSL: synchronous flip for 2 buffers. */
    struct _FBDisplay* display;

    display = (struct _FBDisplay*)Display;

    if (display->multiBuffer == 2)
    {
        /*
         * FSL: Synchronous flip when 2 buffers to avoid display tearing.
         * Tests show tearing is still there...
         */
        return gcvTRUE;
    }

    return gcvFALSE;
}
gceSTATUS
drmfb_CreateContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}
gceSTATUS
drmfb_DestroyContext(
    IN gctPOINTER Display,
    IN gctPOINTER Context)
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_MakeCurrent(
    IN gctPOINTER Display,
    IN PlatformWindowType DrawDrawable,
    IN PlatformWindowType ReadDrawable,
    IN gctPOINTER Context,
    IN gcoSURF ResolveTarget
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_CreateDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_DestroyDrawable(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_RSForSwap(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctPOINTER resolve
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_SwapBuffers(
    IN gctPOINTER Display,
    IN PlatformWindowType Drawable,
    IN gcoSURF RenderTarget,
    IN gcoSURF ResolveTarget,
    IN gctPOINTER ResolveBits,
    OUT gctUINT *Width,
    OUT gctUINT *Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
drmfb_ResizeWindow(
    IN gctPOINTER localDisplay,
    IN PlatformWindowType Drawable,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

#include <gc_egl_precomp.h>


/*
 * Number of temorary linear 'resolve' surfaces.
 * Need alloate those surfaces when can directly resolve into window back
 * buffers.
 */
#define NUM_TEMPORARY_RESOLVE_SURFACES      1

/*
 * Make sure GPU rendering and window back buffer displaying (may be by CPU)
 * are synchronized.
 * The idea is to wait until buffer is displayed before next time return back
 * to GPU rendering.
 */
#define SYNC_TEMPORARY_RESOLVE_SURFACES     0


/******************************************************************************/
/* Display. */

static void *
_GetDefaultDisplay(
    void
    )
{
    PlatformDisplayType display = gcvNULL;
    drmfb_GetDisplay(&display, gcvNULL);

    return display;
}

static void
_ReleaseDefaultDisplay(
    IN void * Display
    )
{
    drmfb_DestroyDisplay((PlatformDisplayType) Display);
}

static EGLBoolean
_IsValidDisplay(
    IN void * Display
    )
{
    if (gcmIS_ERROR(drmfb_IsValidDisplay((PlatformDisplayType) Display)))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_InitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = drmfb_InitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
                                        &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_DeinitLocalDisplayInfo(
    IN VEGLDisplay Display
    )
{
    gceSTATUS status;

    status = drmfb_DeinitLocalDisplayInfo((PlatformDisplayType) Display->hdc,
                                          &Display->localInfo);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLint
_GetNativeVisualId(
    IN VEGLDisplay Display,
    IN struct eglConfig * Config
    )
{
    EGLint visualId = 0;

    drmfb_GetNativeVisualId((PlatformDisplayType) Display->hdc, &visualId);
    return visualId;
}

/* Query of swap interval range. */
static EGLBoolean
_GetSwapInterval(
    IN VEGLDisplay Display,
    OUT EGLint * Min,
    OUT EGLint * Max
    )
{
    gceSTATUS status;

    /* Get swap interval from HAL OS layer. */
    status = drmfb_GetSwapInterval((PlatformDisplayType) Display->hdc,
                                   Min, Max);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SetSwapInterval(
    IN VEGLSurface Surface,
    IN EGLint Interval
    )
{
    gceSTATUS status;

    status = drmfb_SetSwapInterval((PlatformWindowType)Surface->hwnd, Interval);

    if (status == gcvSTATUS_NOT_SUPPORTED)
    {
        /*
         * return true to maintain legacy behavior. If the feature is not there
         * we were ignoring it. And now we are ignoring it too.
         */
        return EGL_TRUE;
    }
    else if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

/******************************************************************************/
/* Window. */

typedef struct eglNativeBuffer * VEGLNativeBuffer;

struct eglNativeBuffer
{
    gctPOINTER          context;
    gcsPOINT            origin;
    gcoSURF             surface;

    EGLint              age;

    /* Buffer lock. */
    gctSIGNAL           lock;

    VEGLNativeBuffer    prev;
    VEGLNativeBuffer    next;
};

struct eglWindowInfo
{
    /*
     * Can directly access window memory?
     * True  for FBDEV, DFB, QNX, DDraw, etc.
     * False for GDI, X11, DRI, etc.
     *
     * As descripted in comments in 'bindWindow' in header file, 4 conditions
     * for window back buffer:
     * If 'fbDirect' window memory: ('fbDirect' = 'True')
     *   1. Direct window back buffer surface ('wrapFB' = 'False')
     *   2. Wrapped surface ('wrapFB' = 'True')
     *   3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
     * Else:
     *   4. Temporary surface. ('fbDirect' = 'False')
     */
    EGLBoolean          fbDirect;

    /*
     * Wrap window back buffer as HAL surface object?
     * Invalid if 'fbDirect' is 'False'.
     */
    EGLBoolean          wrapFB;

    /* Window back buffer list, wrappers or temporary surface objects. */
    VEGLNativeBuffer    bufferList;
    gctPOINTER          bufferListMutex;

    /* Information obtained by drmfb_GetDisplayInfoEx. */
    gctINT              stride;
    gctUINT             width;
    gctUINT             height;
    gceSURF_FORMAT      format;
    gceSURF_TYPE        type;
    gctINT              bitsPerPixel;
    gctUINT             xresVirtual;
    gctUINT             yresVirtual;
    gctUINT             multiBuffer;
};



/*
 * Create wrappers or temporary surface object(s) for native window (conditions
 * 2, 3 and 4 mentioned above).
 *
 * o 2. Wrapped surface ('wrapFB' = 'True')
 * o 3. Temporary surface ('wrapFB' = 'False') (Not supported for now.)
 * o 4. Temporary surface. ('fbDirect' = 'False')
 */
static gceSTATUS
_CreateWindowBuffers(
    VEGLDisplay Display,
    void * Window,
    VEGLWindowInfo Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    struct _FBDisplay* display = Display->hdc;
    VEGLNativeBuffer buffer;

    if (Info->fbDirect)
    {
        if (display->drmfbs)
        {
            gctUINT i;
            gctPOINTER pointer;
            gctUINT alignedHeight;
            gctSTRING disableClear = gcvNULL;

            gcoOS_GetEnv(gcvNULL, "GPU_VIV_DISABLE_CLEAR_FB", &disableClear);

            /* Lock. */
            gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

            alignedHeight = Info->yresVirtual / Info->multiBuffer;

            for (i = 0; i < Info->multiBuffer; ++i)
            {
                /* Allocate native buffer object. */
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          sizeof(struct eglNativeBuffer),
                                          &pointer));

                gcoOS_ZeroMemory(pointer, sizeof(struct eglNativeBuffer));
                buffer = pointer;

                /* Add into buffer list. */
                if (Info->bufferList != gcvNULL)
                {
                    VEGLNativeBuffer prev = Info->bufferList->prev;

                    buffer->prev = prev;
                    buffer->next = Info->bufferList;

                    prev->next = buffer;
                    Info->bufferList->prev = buffer;
                }
                else
                {
                    buffer->prev = buffer->next = buffer;
                    Info->bufferList = buffer;
                }

                gcmONERROR(gcoSURF_WrapUserMemory(
                    gcvNULL,
                    Info->width,
                    Info->height,
                    Info->stride,
                    1,
                    Info->type,
                    Info->format,
                    display->drmfbs[i].fd,
                    gcvALLOC_FLAG_DMABUF,
                    &buffer->surface
                ));

                if (disableClear != gcvNULL)
                {
                    gctPOINTER logical[3] = {gcvNULL};

                    do {
                        gcmERR_BREAK(gcoSURF_Lock(buffer->surface, gcvNULL, &logical[0]));

                        /* For a new surface, clear it to get rid of noises. */
                        gcoOS_ZeroMemory(logical[0], alignedHeight * Info->stride);
                    } while (gcvFALSE);

                    if (logical[0])
                    {
                        gcmONERROR(gcoSURF_Unlock(buffer->surface, logical[0]));
                    }
                }

#if gcdENABLE_3D
                if ((gceSURF_TYPE)((gctUINT32)Info->type & 0xFF) == gcvSURF_RENDER_TARGET)
                {
                    /* Render target surface orientation is different. */
                    gcmVERIFY_OK(
                        gcoSURF_SetFlags(buffer->surface,
                                         gcvSURF_FLAG_CONTENT_YINVERTED,
                                         gcvTRUE));

                    if (!(Info->type & gcvSURF_NO_TILE_STATUS))
                    {
                        /* Append tile status to this user-pool rt. */
                        gcmVERIFY_OK(gcoSURF_AppendTileStatus(buffer->surface));
                    }
                }
#endif

                buffer->context  = gcvNULL;
                buffer->origin.x = 0;
                buffer->origin.y = alignedHeight * i;

                gcmTRACE(gcvLEVEL_INFO,
                         "%s(%d): buffer[%d]: yoffset=%-4d",
                         __FUNCTION__, __LINE__,
                         i, alignedHeight * i);
            }

            gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
        }
        else
        {
            gcmPRINT("%s(%d): Invalid integration!", __FUNCTION__, __LINE__);
            return gcvSTATUS_OK;
        }
    }
    else
    {
        /* Create temporary surface objects */
        gctINT i;
        gctPOINTER pointer;

        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        for (i = 0; i < NUM_TEMPORARY_RESOLVE_SURFACES; i++)
        {
            /* Allocate native buffer object. */
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      sizeof (struct eglNativeBuffer),
                                      &pointer));

            gcoOS_ZeroMemory(pointer, sizeof (struct eglNativeBuffer));
            buffer = pointer;

            /* Add into buffer list. */
            if (Info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer prev = Info->bufferList->prev;

                buffer->prev = prev;
                buffer->next = Info->bufferList;

                prev->next = buffer;
                Info->bufferList->prev = buffer;
            }
            else
            {
                buffer->prev = buffer->next = buffer;
                Info->bufferList = buffer;
            }

            /* Construct temporary resolve surfaces. */
            gcmONERROR(gcoSURF_Construct(gcvNULL,
                                         Info->width,
                                         Info->height, 1,
                                         gcvSURF_BITMAP,
                                         Info->format,
                                         gcvPOOL_DEFAULT,
                                         &buffer->surface));

#if SYNC_TEMPORARY_RESOLVE_SURFACES
            /* Create the buffer lock. */
            gcmONERROR(gcoOS_CreateSignal(gcvNULL, gcvTRUE, &buffer->lock));

            /* Set initial 'unlocked' state. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
#endif

            gcmTRACE(gcvLEVEL_INFO,
                     "%s(%d): buffer[%d]: surface=%p",
                     __FUNCTION__, __LINE__,
                     i, buffer->surface);
        }

        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    return EGL_TRUE;

OnError:
    /* Error roll back. */
    if ((buffer = Info->bufferList) != gcvNULL)
    {
        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;
    }

    /* The buffer list mutex must be acquired. */
    gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    return status;
}

static void
_FreeWindowBuffers(
    VEGLSurface Surface,
    void * Window,
    VEGLWindowInfo Info
    )
{
    if (Info->bufferList)
    {
        VEGLNativeBuffer buffer;

        /* Make sure all workers have been processed. */
        if (Surface->workerDoneSignal != gcvNULL)
        {
            gcmVERIFY_OK(gcoOS_WaitSignal(gcvNULL,
                                          Surface->workerDoneSignal,
                                          gcvINFINITE));
        }

        /* Lock buffers. */
        gcoOS_AcquireMutex(gcvNULL, Info->bufferListMutex, gcvINFINITE);

        /* Go through all buffers. */
        buffer = Info->bufferList;

        do
        {
            VEGLNativeBuffer next = buffer->next;

            /* Destroy the surface. */
            gcoSURF_Destroy(buffer->surface);
            buffer->surface = gcvNULL;

            if (buffer->lock != gcvNULL)
            {
                /* Destroy the signal. */
                gcoOS_DestroySignal(gcvNULL, buffer->lock);
                buffer->lock = gcvNULL;
            }

            gcmOS_SAFE_FREE(gcvNULL, buffer);

            /* Go to next. */
            buffer = next;
        }
        while (buffer != Info->bufferList);

        /* All buffers free'ed. */
        Info->bufferList = gcvNULL;

        /* Unlock. */
        gcoOS_ReleaseMutex(gcvNULL, Info->bufferListMutex);
    }

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));
}

static
EGLBoolean
_QueryWindowInfo(
    IN VEGLDisplay Display,
    IN void * Window,
    IN VEGLWindowInfo Info
    )
{
    gceSTATUS status;
    fbdevDISPLAY_INFO dInfo;
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE type;
    gctINT bitsPerPixel;

    /* Get Window info. */
    status = drmfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) Window,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   &bitsPerPixel,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Initialize window geometry info. */
    Info->width        = width;
    Info->height       = height;
    Info->format       = format;
    Info->type         = type;
    Info->bitsPerPixel = bitsPerPixel;

    /* Get display information. */
    gcoOS_ZeroMemory(&dInfo, sizeof (fbdevDISPLAY_INFO));

    status = drmfb_GetDisplayInfoEx((PlatformDisplayType) Display->hdc,
                                    (PlatformWindowType) Window,
                                    Display->localInfo,
                                    sizeof (fbdevDISPLAY_INFO),
                                    &dInfo);

    if (gcmIS_ERROR(status))
    {
        Info->fbDirect     = EGL_FALSE;
        Info->stride       = 0;
        Info->wrapFB       = gcvFALSE;
        Info->multiBuffer  = 1;
    }
    else
    {
        Info->fbDirect     = EGL_TRUE;
        Info->stride       = dInfo.stride;
        Info->wrapFB       = dInfo.wrapFB;
        Info->multiBuffer  = dInfo.multiBuffer > 1 ? dInfo.multiBuffer : 1;
    }

    /* Get virtual size. */
    status = drmfb_GetDisplayVirtual((PlatformDisplayType) Display->hdc,
                                     (gctINT_PTR) &Info->xresVirtual,
                                     (gctINT_PTR) &Info->yresVirtual);

    if (gcmIS_ERROR(status))
    {
        Info->xresVirtual = Info->width;
        Info->yresVirtual = Info->height;

        if (Info->multiBuffer > 1)
        {
            Info->multiBuffer = 1;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean
_ConnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN void * Window
    )
{
    gceSTATUS status;
    VEGLWindowInfo info = gcvNULL;
    void * win = Window;
    gctPOINTER pointer;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(win != gcvNULL);
    gcmASSERT(Surface->winInfo == gcvNULL);

    /* Allocate memory. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof (struct eglWindowInfo),
                              &pointer));

    gcoOS_ZeroMemory(pointer, sizeof (struct eglWindowInfo));
    info = pointer;

    /* Query window information. */
    if (_QueryWindowInfo(Display, Window, info) == EGL_FALSE)
    {
        /* Bad native window. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Create buffer mutex. */
    gcmONERROR(gcoOS_CreateMutex(gcvNULL, &info->bufferListMutex));

    /* Create window drawable? */
    drmfb_CreateDrawable(Display->localInfo, (PlatformWindowType) win);

    /* Create window buffers to represent window native bufers. */
    gcmONERROR(_CreateWindowBuffers(Display, Window, info));

    /* Save window info structure. */
    Surface->winInfo = info;
    return EGL_TRUE;

OnError:
    if (info)
    {
        if (info->bufferListMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
            info->bufferListMutex = gcvNULL;
        }

        gcmOS_SAFE_FREE(gcvNULL, info);
        Surface->winInfo = gcvNULL;
    }

    return EGL_FALSE;
}

static EGLBoolean
_DisconnectWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    /* Get shortcut. */
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    /* Free native window buffers. */
    _FreeWindowBuffers(Surface, win, info);

    /* Delete the mutex. */
    gcoOS_DeleteMutex(gcvNULL, info->bufferListMutex);
    info->bufferListMutex = gcvNULL;

    drmfb_DestroyDrawable(Display->localInfo, (PlatformWindowType) win);

    /* Commit accumulated commands. */
    gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

    gcmOS_SAFE_FREE(gcvNULL, info);
    return EGL_TRUE;
}

#if gcdENABLE_RENDER_INTO_WINDOW && gcdENABLE_3D
/*
 * For apps in this list, EGL will use indirect rendering,
 * ie, disable no-resolve.
 */
static gcePATCH_ID indirectList[] =
{
    -1,
};

#endif


static EGLBoolean
_BindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    OUT EGLint * RenderMode
    )
{
    gceSTATUS status;

    /* Get shortcut. */
    void * win  = Surface->hwnd;
    VEGLWindowInfo info   = Surface->winInfo;
    /* Indirect rendering by default. */
    EGLint renderMode     = VEGL_INDIRECT_RENDERING;
    EGLBoolean winChanged = EGL_FALSE;
    gctINT width          = 0;
    gctINT height         = 0;
    gceSURF_FORMAT format = gcvSURF_UNKNOWN;
    gceSURF_TYPE type     = gcvSURF_UNKNOWN;

    status = drmfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    /* Check window resize. */
    if ((info->width  != (gctUINT)width) ||
        (info->height != (gctUINT)height) ||
        (info->format != format))
    {
        /* Native window internally changed. */
        winChanged = EGL_TRUE;
    }

    if (Surface->openVG)
    {
        /* Check direct rendering support for 2D VG. */
        do
        {
            EGLBoolean formatSupported = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            switch (format)
            {
            case gcvSURF_A8R8G8B8:
            case gcvSURF_A8B8G8R8:
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8R8G8B8:
            case gcvSURF_X8B8G8R8:
                if (Surface->config.alphaSize == 0)
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_R5G6B5:
                if ((Surface->config.redSize == 5) &&
                    (Surface->config.greenSize == 6) &&
                    (Surface->config.blueSize == 5) &&
                    (Surface->config.alphaSize == 0))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_A4R4G4B4:
            case gcvSURF_A4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4) &&
                    (Surface->config.alphaSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            case gcvSURF_X4R4G4B4:
            case gcvSURF_X4B4G4R4:
                if ((Surface->config.redSize == 4) &&
                    (Surface->config.greenSize == 4) &&
                    (Surface->config.blueSize == 4))
                {
                    formatSupported = EGL_TRUE;
                }
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            /* Should use direct rendering. */
            renderMode = VEGL_DIRECT_RENDERING;
        }
        while (gcvFALSE);

        if ((type != gcvSURF_BITMAP) ||
            (info->type != gcvSURF_BITMAP))
        {
            /* Request linear buffer for hardware OpenVG. */
            status = drmfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            /* Recreate window buffers. */
            _FreeWindowBuffers(Surface, win, info);
            gcmONERROR(_CreateWindowBuffers(Display, win, info));
        }
    }
    else
    {
#if gcdENABLE_3D
#if gcdENABLE_RENDER_INTO_WINDOW
        /* 3D pipe. */
        do
        {
            /* Check if direct rendering is available. */
            EGLBoolean fcFill = EGL_FALSE;
            EGLBoolean formatSupported;
            gceSURF_FORMAT reqFormat = format;

            EGLint i;
            gcePATCH_ID patchId = gcvPATCH_INVALID;
            EGLBoolean indirect = EGL_FALSE;

            if (!info->fbDirect)
            {
                /* No direct access to back buffer. */
                break;
            }

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
                /* Forced indirect rendering. */
                break;
            }

            if (Surface->config.samples > 1)
            {
                /* Can not support MSAA, stop. */
                break;
            }

            /* Require fc-fill feature in hardware. */
            status = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TILE_FILLER);

            if (status == gcvSTATUS_TRUE)
            {
                /* Has fc-fill feature. */
                fcFill = EGL_TRUE;
            }

            /* Check window format. */
            switch (format)
            {
            case gcvSURF_A8B8G8R8:
                reqFormat = gcoHAL_IsFeatureAvailable(gcvNULL,gcvFEATURE_PE_A8B8G8R8) ? gcvSURF_A8B8G8R8 : gcvSURF_A8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_X8B8G8R8:
                reqFormat = gcvSURF_X8R8G8B8;
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_A8R8G8B8:
            case gcvSURF_X8R8G8B8:
            case gcvSURF_A4R4G4B4:
            case gcvSURF_X4R4G4B4:
                formatSupported = EGL_TRUE;
                break;
            case gcvSURF_R5G6B5:
                formatSupported = (Surface->config.alphaSize == 0) ? EGL_TRUE : EGL_FALSE;
                break;
            default:
                formatSupported = EGL_FALSE;
                break;
            }

            if (!formatSupported)
            {
                /* Format not supported, stop. */
                break;
            }

            if (!info->wrapFB)
            {
                /* Try many direct rendering levels here. */
                /* 1. The best, direct rendering with compression. */
                if ((type != gcvSURF_RENDER_TARGET) ||
                    (info->type != gcvSURF_RENDER_TARGET)  ||
                    (info->format != reqFormat))
                {
                    status = drmfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering with compression. */
                        renderMode = VEGL_DIRECT_RENDERING;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already rendering with compression. */
                    renderMode = VEGL_DIRECT_RENDERING;
                }

                /* 2. Second, with tile status, no compression. */
                if ((type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                    (info->format != reqFormat))
                {

                    status = drmfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                                   (PlatformWindowType) win,
                                                   gcvSURF_RENDER_TARGET_NO_COMPRESSION,
                                                   reqFormat);

                    if (gcmIS_SUCCESS(status))
                    {
                        /* Should use direct rendering without compression. */
                        renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;

                        /* Window is changed. */
                        winChanged = EGL_TRUE;
                        break;
                    }

                    /* Not an error. */
                    status = gcvSTATUS_OK;
                }
                else
                {
                    /* Already direct rendering without compression. */
                    renderMode = VEGL_DIRECT_RENDERING_FC_NOCC;
                }
            }

            if (!fcFill)
            {
                /* Do not need check the next mode. */
                break;
            }

            /*
             * Special for FC-FILL mode: tile status is required.
             * Final internal render type should be RENDER_TARGET_NO_COMPRESSION.
             */
            if ((type != gcvSURF_RENDER_TARGET_NO_TILE_STATUS) ||
                (info->type != gcvSURF_RENDER_TARGET_NO_COMPRESSION) ||
                (info->format != reqFormat))
            {
                /* Try FC fill. */
                status = drmfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                               (PlatformWindowType) win,
                                               gcvSURF_RENDER_TARGET_NO_TILE_STATUS,
                                               reqFormat);

                if (gcmIS_SUCCESS(status))
                {
                    /* Should use direct rendering with fc-fill. */
                    renderMode = VEGL_DIRECT_RENDERING_FCFILL;

                    /* Window is changed. */
                    winChanged = EGL_TRUE;
                    break;
                }

                /* Not an error. */
                status = gcvSTATUS_OK;
            }
            else
            {
                /* Already direct rendering with fc-fill. */
                renderMode = VEGL_DIRECT_RENDERING_FCFILL;
            }
        }
        while (gcvFALSE);
#   endif

        if ((renderMode == VEGL_INDIRECT_RENDERING) &&
            ((type != gcvSURF_BITMAP) || (info->type != gcvSURF_BITMAP)))
        {
            /* Only linear supported in this case. */
            status = drmfb_SetWindowFormat((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           gcvSURF_BITMAP,
                                           format);

            if (gcmIS_ERROR(status))
            {
                /* Can not support non-bitmap. */
                return EGL_FALSE;
            }

            /* Window type is changed. */
            winChanged = EGL_TRUE;
        }

        if (winChanged)
        {
            /* Query window info again in case other parameters chagned. */
            _QueryWindowInfo(Display, win, info);

            if ((renderMode == VEGL_DIRECT_RENDERING_FCFILL) &&
                (info->type == gcvSURF_RENDER_TARGET_NO_TILE_STATUS))
            {
                /* Special for FC-FILL mode: tile status is required. */
                info->type = gcvSURF_RENDER_TARGET_NO_COMPRESSION;
            }

            /* Recreate window buffers. */
            _FreeWindowBuffers(Surface, win, info);
            gcmONERROR(_CreateWindowBuffers(Display, win, info));
        }


#endif
    }

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): winChanged=%d format=%d type=%x EGLConfig=%d%d%d%d "
             " renderMode=%d",
             __FUNCTION__, __LINE__,
             winChanged,
             info->format,
             info->type,
             Surface->config.redSize,
             Surface->config.greenSize,
             Surface->config.blueSize,
             Surface->config.alphaSize,
             renderMode);

    *RenderMode = renderMode;
    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
_UnbindWindow(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
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
    gctINT width;
    gctINT height;
    gceSURF_FORMAT format;
    gceSURF_TYPE   type;

    /* Get shortcut. */
    void * win = Surface->hwnd;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(Surface->winInfo);

    status = drmfb_GetWindowInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformWindowType) win,
                                   gcvNULL, gcvNULL,
                                   &width, &height,
                                   gcvNULL,
                                   gcvNULL,
                                   &format,
                                   &type);

    (void) format;
    (void) type;

    if (gcmIS_ERROR(status))
    {
        /* Bad native window. */
        return EGL_FALSE;
    }

    *Width  = width;
    *Height = height;

    return EGL_TRUE;
}

static EGLBoolean
_GetWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    gceSTATUS status;
    void * win = Surface->hwnd;
    VEGLWindowInfo info  = Surface->winInfo;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    if (info->fbDirect)
    {
        gctUINT offset;

        BackBuffer->surface  = gcvNULL;
        BackBuffer->context  = gcvNULL;
        BackBuffer->origin.x = 0;
        BackBuffer->origin.y = 0;
        BackBuffer->flip     = gcvTRUE;

        /* Formerly veglGetDisplayBackBuffer. */
        status = drmfb_GetDisplayBackbufferEx((PlatformDisplayType) Display->hdc,
                                              (PlatformWindowType) win,
                                               Display->localInfo,
                                               &BackBuffer->context,
                                               &BackBuffer->surface,
                                               &offset,
                                               &BackBuffer->origin.x,
                                               &BackBuffer->origin.y);

        if (gcmIS_ERROR(status))
        {
            /*
             * Fomerly, returns flip=false, then it will use first wrapper.
             */
            VEGLNativeBuffer buffer = info->bufferList;

            if (!buffer)
            {
                /* No wrappers? Bad native window. */
                return EGL_FALSE;
            }

            /* Copy out back buffer. */
            BackBuffer->context = buffer->context;
            BackBuffer->origin  = buffer->origin;
            BackBuffer->surface = buffer->surface;

            BackBuffer->flip    = gcvFALSE;

            /* Increase reference count. */
            /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

            return EGL_TRUE;
        }

        if (BackBuffer->surface)
        {
            /* Returned the surface directly. */
            return EGL_TRUE;
        }
        else
        {
            VEGLNativeBuffer buffer = gcvNULL;

            /* WrapFB or temporary surface, go through bufferList to find */
            gcmASSERT(info->wrapFB);

            if (info->bufferList != gcvNULL)
            {
                VEGLNativeBuffer buf = info->bufferList;

                gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

                do
                {
                    if ((buf->context  == BackBuffer->context)  &&
                        (buf->origin.x == BackBuffer->origin.x) &&
                        (buf->origin.y == BackBuffer->origin.y))
                    {
                        /* Found. */
                        buffer = buf;
                        break;
                    }

                    buf = buf->next;
                }
                while (buffer != info->bufferList);

                gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);
            }

            if (buffer != gcvNULL)
            {
                /* Return the found surface. */
                BackBuffer->surface  = buffer->surface;
                BackBuffer->context  = buffer->context;
                BackBuffer->origin.x = buffer->origin.x;
                BackBuffer->origin.y = buffer->origin.y;

                /* Increase reference count. */
                /* gcoSURF_ReferenceSurface(BackBuffer->surface); */
                return EGL_TRUE;
            }
            else
            {
                /* Bad native window. */
                return EGL_FALSE;
            }
        }
    }
    else
    {
        /* Return the temorary surface object. */
        VEGLNativeBuffer buffer;

        gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

        buffer = info->bufferList;

        BackBuffer->surface  = buffer->surface;
        BackBuffer->context  = buffer;
        BackBuffer->origin.x = 0;
        BackBuffer->origin.y = 0;
        BackBuffer->flip     = gcvTRUE;

        info->bufferList = buffer->next;

        gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

        if (buffer->lock != gcvNULL)
        {
            /* Wait for buffer lock. */
            for (;;)
            {
                status = gcoOS_WaitSignal(gcvNULL, buffer->lock, 5000);

                if (status == gcvSTATUS_TIMEOUT)
                {
                    gcmPRINT("Wait for buffer lock timeout");
                    continue;
                }

                break;
            }

            /*
             * Set the buffer to 'locked' state.
             * It will be 'unlocked' when buffer posted to display.
             * This can make sure next time GetWindowBackBuffer, the buffer
             * is 'posted' before returns for GPU rendering.
             */
            gcoOS_Signal(gcvNULL, buffer->lock, gcvFALSE);
        }

        /* Increase reference count. */
        /* gcoSURF_ReferenceSurface(BackBuffer->surface); */

        return EGL_TRUE;
    }
}

static EGLBoolean
_PostWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer,
    IN struct eglRegion * Region,
    IN struct eglRegion * DamageHint
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    (void) surface;

    if (info->fbDirect)
    {
        surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

        status = drmfb_SetDisplayVirtualEx((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           BackBuffer->context,
                                           surface,
                                           0,
                                           BackBuffer->origin.x,
                                           BackBuffer->origin.y);

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }
    }
    else
    {
        VEGLNativeBuffer buffer;
        gctINT alignedWidth, alignedHeight;
        gceORIENTATION orientation;
        gceSURF_FORMAT format = gcvSURF_UNKNOWN;
        gcsSURF_FORMAT_INFO_PTR formatInfo;
        gctPOINTER memory[3] = {gcvNULL};
        gctINT i;

        /* Cast type. */
        buffer = (VEGLNativeBuffer) BackBuffer->context;

        /* Get aligned size. */
        gcmVERIFY_OK(gcoSURF_GetAlignedSize(BackBuffer->surface,
                                            (gctUINT_PTR) &alignedWidth,
                                            (gctUINT_PTR) &alignedHeight,
                                            gcvNULL));

        gcmVERIFY_OK(gcoSURF_QueryOrientation(BackBuffer->surface, &orientation));

        if (orientation == gcvORIENTATION_BOTTOM_TOP)
        {
            alignedHeight = -alignedHeight;
        }

        /* Gather source information. */
        gcmVERIFY_OK(gcoSURF_GetFormat(BackBuffer->surface,
                                       gcvNULL,
                                       &format));

        /* Query format. */
        if (gcoSURF_QueryFormat(format, &formatInfo))
        {
            return EGL_FALSE;
        }

        /* Lock surface for memory. */
        if (gcoSURF_Lock(BackBuffer->surface, gcvNULL, memory))
        {
            return EGL_FALSE;
        }

        for (i = 0; i < Region->numRects; i++)
        {
            EGLint left   = Region->rects[i * 4 + 0];
            EGLint top    = Region->rects[i * 4 + 1];
            EGLint width  = Region->rects[i * 4 + 2];
            EGLint height = Region->rects[i * 4 + 3];

            /* Draw image. */
            status = drmfb_DrawImageEx((PlatformDisplayType) Display->hdc,
                                       (PlatformWindowType) win,
                                       left, top, left + width, top + height,
                                       alignedWidth, alignedHeight,
                                       formatInfo->bitsPerPixel,
                                       memory[0],
                                       format);

            if (gcmIS_ERROR(status))
            {
                break;
            }
        }

        /* Unlock the surface. */
        gcmVERIFY_OK(gcoSURF_Unlock(BackBuffer->surface, memory[0]));

        if (buffer->lock != gcvNULL)
        {
            /* The buffer is now posted. */
            gcmVERIFY_OK(gcoOS_Signal(gcvNULL, buffer->lock, gcvTRUE));
        }

        if (gcmIS_ERROR(status))
        {
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean
_CancelWindowBackBuffer(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    void * win = Surface->hwnd;
    VEGLWindowInfo info = Surface->winInfo;
    gcoSURF surface;
    gceSTATUS status = gcvSTATUS_OK;

    gcmASSERT(Surface->type & EGL_WINDOW_BIT);
    gcmASSERT(info);

    surface = info->wrapFB ? gcvNULL : BackBuffer->surface;

    status = drmfb_CancelDisplayBackbuffer((PlatformDisplayType) Display->hdc,
                                           (PlatformWindowType) win,
                                           BackBuffer->context,
                                           surface,
                                           0,
                                           BackBuffer->origin.x,
                                           BackBuffer->origin.y);

    if (gcmIS_ERROR(status))
    {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static EGLBoolean
_SynchronousPost(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface
    )
{
    return drmfb_SynchronousFlip((PlatformDisplayType)Display->hdc);
}

static EGLBoolean
_UpdateBufferAge(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN struct eglBackBuffer * BackBuffer
    )
{
    VEGLWindowInfo info  = Surface->winInfo;
    VEGLNativeBuffer buffer = info->bufferList;

    gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

    do
    {
        if ((buffer->context  == BackBuffer->context)  &&
            (buffer->origin.x == BackBuffer->origin.x) &&
            (buffer->origin.y == BackBuffer->origin.y))
        {
            /* Current buffer. */
            buffer->age = 1;
        }
        else if (buffer->age)
        {
            buffer->age++;
        }

        buffer = buffer->next;
    }
    while (buffer != info->bufferList);

    gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

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
    VEGLWindowInfo info  = Surface->winInfo;

    if (BackBuffer)
    {
        VEGLNativeBuffer buffer = info->bufferList;

        gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

        do
        {
            if ((buffer->context  == BackBuffer->context)  &&
                (buffer->origin.x == BackBuffer->origin.x) &&
                (buffer->origin.y == BackBuffer->origin.y))
            {
                /* Found buffer, return its age. */
                *BufferAge = buffer->age;
                break;
            }

            buffer = buffer->next;
        }
        while (buffer != info->bufferList);

        gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

        return EGL_TRUE;
    }
    else if (!Surface->newSwapModel)
    {
        EGLint age = info->multiBuffer;
        VEGLNativeBuffer buffer = info->bufferList;

        gcoOS_AcquireMutex(gcvNULL, info->bufferListMutex, gcvINFINITE);

        /*
         * In fbdev, back buffers are returned in order. It's safe to return
         * buffer-count as buffer age --- except that there're 0 aged buffers.
         */
        do
        {
            if (buffer->age == 0)
            {
                age = 0;
                break;
            }

            buffer = buffer->next;
        }
        while (buffer != info->bufferList);

        gcoOS_ReleaseMutex(gcvNULL, info->bufferListMutex);

        *BufferAge = age;
        return EGL_TRUE;
    }

    return EGL_FALSE;
}

/******************************************************************************/
/* Pixmap. */

struct eglPixmapInfo
{
    /* Native pixmap geometry info in Vivante HAL. */
    gctINT              width;
    gctINT              height;
    gceSURF_FORMAT      format;
    gctINT              stride;
    gctINT              bitsPerPixel;

    /* Pixmap memory. */
    gctUINT8_PTR        data;

    /* Reference native display. */
    void *              hdc;

    /* Pixmap wrapper. */
    gcoSURF             wrapper;

    /* Shadow surface, exists when the wrapper is not resovable. */
    gcoSURF             shadow;
};

static void
_DoSyncFromPixmap(
    void * Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(memory[0], Info->data, stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) Info->data;
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
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(drmfb_CopyPixmapBits((PlatformDisplayType) Info->hdc,
                                        (PlatformPixmapType) Pixmap,
                                        width, height,
                                        stride,
                                        Info->format,
                                        memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    /* Unlock. */
    if (memory[0] != gcvNULL)
    {
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

static void
_DoSyncToPixmap(
    void * Pixmap,
    VEGLPixmapInfo Info
    )
{
    gceSTATUS status;
    gctPOINTER memory[3] = {gcvNULL};
    gctINT stride;
    gctUINT width, height;

    /* Get shortcut. */
    gcoSURF shadow = Info->shadow;

    /* Query shadow surface stride. */
    gcmONERROR(gcoSURF_GetAlignedSize(shadow, &width, &height, &stride));

    /* Lock for pixels. */
    gcmONERROR(gcoSURF_Lock(shadow, gcvNULL, memory));

    if (Info->data != gcvNULL)
    {
        if (stride == Info->stride)
        {
            /* Same stride. */
            gcoOS_MemCopy(Info->data, memory[0], stride * Info->height);
        }
        else
        {
            /* Copy line by line. */
            gctINT y;
            gctUINT8_PTR source = (gctUINT8_PTR) memory[0];
            gctUINT8_PTR dest   = (gctUINT8_PTR) Info->data;
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
    }
    else
    {
        /* Call underlying OS layer function to copy pixels. */
        gcmONERROR(drmfb_DrawPixmap((PlatformDisplayType) Info->hdc,
                                    (PlatformPixmapType) Pixmap,
                                    0, 0,
                                    Info->width,
                                    Info->height,
                                    width,
                                    height,
                                    Info->bitsPerPixel,
                                    memory[0]));
    }

    /* Unlock. */
    gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    return;

OnError:
    if (memory[0] != gcvNULL)
    {
        /* Unlock. */
        gcmVERIFY_OK(gcoSURF_Unlock(shadow, gcvNULL));
    }
}

static EGLBoolean
_MatchPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN struct eglConfig * Config
    )
{
    gceSTATUS status;
    gctINT width, height, bitsPerPixel;
    gceSURF_FORMAT pixmapFormat;
    EGLBoolean match = EGL_TRUE;

    status = drmfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                   (PlatformPixmapType) Pixmap,
                                   &width,
                                   &height,
                                   &bitsPerPixel,
                                   gcvNULL, gcvNULL, &pixmapFormat);

    if (gcmIS_ERROR(status))
    {
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
}

static EGLBoolean
_ConnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    OUT VEGLPixmapInfo * Info,
    OUT gcoSURF * Surface
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL needShadow = gcvFALSE;
    gctINT pixmapWidth;
    gctINT pixmapHeight;
    gctINT pixmapStride = 0;
    gceSURF_FORMAT pixmapFormat;
    gctINT pixmapBpp;
    gctPOINTER pixmapBits = gcvNULL;
    gctPHYS_ADDR_T pixmapPhysical = gcvINVALID_PHYSICAL_ADDRESS;
    gcoSURF wrapper = gcvNULL;
    gcoSURF shadow = gcvNULL;
    gctPOINTER pointer;
    VEGLPixmapInfo info = gcvNULL;

    /* Query pixmap geometry info. */
    gcmONERROR(drmfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                                     (PlatformPixmapType) Pixmap,
                                     &pixmapWidth,
                                     &pixmapHeight,
                                     &pixmapBpp,
                                     gcvNULL,
                                     gcvNULL,
                                     &pixmapFormat));

    /* Query pixmap bits. */
    status = drmfb_GetPixmapInfo((PlatformDisplayType) Display->hdc,
                                 (PlatformPixmapType) Pixmap,
                                 gcvNULL,
                                 gcvNULL,
                                 gcvNULL,
                                 &pixmapStride,
                                 &pixmapBits);

    do
    {
        if (gcmIS_ERROR(status) || !pixmapBits)
        {
            /* Can not wrap as surface object. */
            needShadow = gcvTRUE;
            break;
        }

        if (((gctUINTPTR_T) pixmapBits) & 0x3F)
        {
            needShadow = gcvTRUE;
            break;
        }

        if ((pixmapStride * 8 / pixmapBpp) < 16)
        {
            /* Too small in width. */
            needShadow = gcvTRUE;
            break;
        }


        /* Height needs to be 4 aligned or vstride is large enough. */
        if (pixmapHeight & 3)
        {
            /*
             * Not enough memory in height.
             * Resolve may exceeds the buffer and overwrite other memory.
             */
            needShadow = gcvTRUE;
            break;
        }

        /* Check pixmap format. */
        switch (pixmapFormat)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_A8B8G8R8:
        case gcvSURF_X8R8G8B8:
        case gcvSURF_X8B8G8R8:
        case gcvSURF_R5G6B5:
        case gcvSURF_A4R4G4B4:
        case gcvSURF_A4B4G4R4:
        case gcvSURF_X4R4G4B4:
        case gcvSURF_X4B4G4R4:
            break;

        default:
            /* Resolve can not support such format. */
            return EGL_FALSE;
        }
    }
    while (gcvFALSE);

    do
    {
        if (needShadow)
        {
            /* No pixmap wrapper. */
            status = gcvSTATUS_OK;
            break;
        }

        /* Construct pixmap wrapper. */
        gcmONERROR(
            gcoSURF_Construct(gcvNULL,
                              pixmapWidth,
                              pixmapHeight,
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
                                   pixmapBits,
                                   pixmapPhysical);

        if (gcmIS_ERROR(status))
        {
            /* Failed to wrap. */
            break;
        }

        /* Do the wrap. */
        status = gcoSURF_SetWindow(wrapper,
                                   0, 0,
                                   pixmapWidth,
                                   pixmapHeight);
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
                              pixmapWidth,
                              pixmapHeight,
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

    /* Save pixmap info. */
    info->width        = pixmapWidth;
    info->height       = pixmapHeight;
    info->format       = pixmapFormat;
    info->stride       = pixmapStride;
    info->bitsPerPixel = pixmapBpp;
    info->data         = pixmapBits;
    info->hdc          = (PlatformDisplayType) Display->hdc;
    info->wrapper      = wrapper;
    info->shadow       = shadow;

    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p wrapper=%p shadow=%p",
             __FUNCTION__, __LINE__, Display, Pixmap, wrapper, shadow);

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
}

static EGLBoolean
_DisconnectPixmap(
    IN VEGLDisplay Display,
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    /* Free pixmap wrapper. */
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): display=%p pixmap=%p",
             __FUNCTION__, __LINE__, Display, Pixmap);

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
    gceSTATUS status;
    gctINT bitsPerPixel;
    gceSURF_FORMAT format;
    gctINT width, height;

    /* Query pixmap info again. */
    gcmONERROR(
        drmfb_GetPixmapInfoEx((PlatformDisplayType) Display->hdc,
                              (PlatformPixmapType) Pixmap,
                              &width,
                              &height,
                              &bitsPerPixel,
                              gcvNULL,
                              gcvNULL,
                              &format));

    (void) bitsPerPixel;
    (void) format;

    gcmASSERT(width  == Info->width);
    gcmASSERT(height == Info->height);

    *Width  = width;
    *Height = height;

    return EGL_TRUE;

OnError:
    return EGL_FALSE;
}

static EGLBoolean
_SyncFromPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncFromPixmap(Pixmap, Info);
    }
    else
    {
        gcmVERIFY_OK(gcoSURF_CPUCacheOperation(Info->wrapper, gcvCACHE_FLUSH));
    }

    return EGL_TRUE;
}

static EGLBoolean
_SyncToPixmap(
    IN void * Pixmap,
    IN VEGLPixmapInfo Info
    )
{
    gcmTRACE(gcvLEVEL_INFO,
             "%s(%d): pixmap=%p",
             __FUNCTION__, __LINE__, Pixmap);

    if (Info->shadow != gcvNULL)
    {
        /* Copy if not wrapped. */
        _DoSyncToPixmap(Pixmap, Info);
    }

    return EGL_TRUE;
}


static struct eglPlatform fbdrmPlatform =
{
    EGL_PLATFORM_FB_VIV,
    0,
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
    gcvNULL,
    _CancelWindowBackBuffer,
    _SynchronousPost,
    gcvNULL,
    _UpdateBufferAge,
    _QueryBufferAge,
    _MatchPixmap,
    _ConnectPixmap,
    _DisconnectPixmap,
    _GetPixmapSize,
    _SyncFromPixmap,
    _SyncToPixmap,
};

gctBOOL
IsDRMModesetAvailable(
    )
{
    int i;
    int fd = -1;
    drmModeRes *resources;
    gctBOOL support = gcvFALSE;
    static const char *modules[] =
    {
        "imx-drm",
        /*add more*/
    };

    for (i = 0; i < (int)gcmCOUNTOF(modules); ++i)
    {
        fd = drmOpen(modules[i], NULL);
        if (fd >= 0)
        {
            break;
        }
    }

    if (fd < 0)
    {
        goto OnError;
    }

    /*Check for any connectors available. DRM will create dummy FBDEV if no connectors available*/
    resources = drmModeGetResources(fd);
    if (resources)
    {
        int i;
        /* find a connected connector: */
        for (i = 0; i < resources->count_connectors; ++i)
        {
            drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
            if (connector->connection == DRM_MODE_CONNECTED)
            {
                support = gcvTRUE;
                break;
            }
            drmModeFreeConnector(connector);
        }
    }

OnError:
    if (fd >= 0)
    {
        drmClose(fd);
    }
    return support;
}

VEGLPlatform
drmfb_GetFbPlatform(
    void * NativeDisplay
    )
{
    return &fbdrmPlatform;
}

static struct eglFbPlatform fbdrmBackend =
{
    EGL_PLATFORM_FB_VIV,
    drmfb_GetDisplay,
    drmfb_GetDisplayByIndex,
    drmfb_GetDisplayInfo,
    drmfb_DestroyDisplay,
    drmfb_CreateWindow,
    drmfb_GetWindowInfo,
    drmfb_DestroyWindow,
    drmfb_CreatePixmap,
    drmfb_GetPixmapInfo,
    drmfb_DestroyPixmap,
    drmfb_GetFbPlatform
};

struct eglFbPlatform* getFbDrmBackend()
{
    /*In case need for legacy FB*/
    char *p = getenv("FB_DRM_MODESET");

    if (p && strcmp(p, "0") && IsDRMModesetAvailable())
    {
        return &fbdrmBackend;
    }
    return gcvNULL;
}

/******************************************************************************/

