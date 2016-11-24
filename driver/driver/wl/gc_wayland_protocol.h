/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __WAYLANDEGLVIV_H__
#define __WAYLANDEGLVIV_H__

/* Wayland platform. */
#include <wayland-egl.h>

#include <gc_hal.h>

typedef struct _gcsWL_VIV_BUFFER
{
   struct wl_resource *wl_buffer;
   gcoSURF surface;
   gctINT32 width, height;
}
gcsWL_VIV_BUFFER;

#define WL_COMPOSITOR_SIGNATURE (0x31415926)
#define WL_CLIENT_SIGNATURE             (0x27182818)
#define WL_LOCAL_DISPLAY_SIGNATURE      (0x27182991)

typedef struct _gcsWL_EGL_DISPLAY
{
   struct wl_display* wl_display;
   struct wl_viv* wl_viv;
   struct wl_registry *registry;
   struct wl_event_queue    *wl_queue;
   struct wl_event_queue    *wl_swap_queue;
   gctINT swapInterval;
   gctINT file;
} gcsWL_EGL_DISPLAY;

typedef struct _gcsWL_LOCAL_DISPLAY {
    gctUINT wl_signature;
    gctPOINTER localInfo;
} gcsWL_LOCAL_DISPLAY;

typedef struct _gcsWL_EGL_BUFFER_INFO
{
   gctINT32 width;
   gctINT32 height;
   gctINT32 stride;
   gceSURF_FORMAT format;
   gceSURF_TYPE   type;
   gcuVIDMEM_NODE_PTR node;
   gcePOOL pool;
   gctSIZE_T bytes;
   gcoSURF surface;
   gctINT32 invalidate;
   gctBOOL locked;
} gcsWL_EGL_BUFFER_INFO;

typedef struct _gcsWL_EGL_BUFFER
{
   gctUINT wl_signature;
   gcsWL_EGL_BUFFER_INFO info;
   struct wl_buffer* wl_buffer;
   struct wl_callback* frame_callback;
   struct wl_list link;
} gcsWL_EGL_BUFFER;


#define container_of(ptr, type, member) ({              \
    const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

gcsWL_EGL_DISPLAY*
gcoWL_GetDisplay(struct wl_display*);

void
gcoWL_CreateGhostBuffer(gcsWL_EGL_DISPLAY*, gcsWL_EGL_BUFFER *);

void
gcoWL_ReleaseDisplay(gcsWL_EGL_DISPLAY*);

#endif /* __WAYLANDEGLVIV_H__ */
