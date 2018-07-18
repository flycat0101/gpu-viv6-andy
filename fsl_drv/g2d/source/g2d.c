/*
 *  Copyright (C) 2013-2014 Freescale Semiconductor, Inc.
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */

/*
 *	g2d.c
 *	Gpu 2d file implements all related g2d api exposed to application
 *	History :
 *	Date(y.m.d)        Author            Version        Description
 *	2012-10-22         Li Xianzhong      0.1            Created
 *	2013-02-22         Li Xianzhong      0.2            g2d_copy API is added
 *	2013-03-21         Li Xianzhong      0.4            g2d clear/rotation/flip APIs are supported
 *	2013-04-09         Li Xianzhong      0.5            g2d alpha blending feature is enhanced
 *	2013-05-17         Li Xianzhong      0.6            support vg core in g2d library
 *	2013-12-23         Li Xianzhong      0.7            support blend dim feature
 *	2014-03-20         Li Xianzhong      0.8            support pre-multipied & de-mutlipy out alpha
 *  2015-04-10         Meng Mingming     0.9            support multiple source blit
 *  2015-11-03         Meng Mingming     1.0            support query 2D hardware type and feature
 *  2016-05-24         Meng Mingming     1.1            support get g2d_buf from dma fd
*/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_driver.h>
#include <gc_hal_raster.h>
#include <gc_hal_engine_vg.h>
#include "g2dExt.h"

#if defined(ANDROID)

#include <cutils/log.h>

#if defined(LOGE)
#define g2d_printf LOGE
#elif defined(ALOGE)
#define g2d_printf ALOGE
#endif

#else

#define g2d_printf printf

#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#ifndef gcvVERSION_DATE
#define gcvVERSION_DATE      __DATE__
#endif

#ifndef gcvVERSION_TIME
#define gcvVERSION_TIME      __TIME__
#endif

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _G2D_VERSION = "\n\0$VERSION$"
                        gcmTXT2STR(gcvVERSION_MAJOR) "."
                        gcmTXT2STR(gcvVERSION_MINOR) "."
                        gcmTXT2STR(gcvVERSION_PATCH) ":"
                        gcmTXT2STR(gcvVERSION_BUILD)
#ifdef GIT_STRING
                        ":"gcmTXT2STR(GIT_STRING)
#endif
                        "$\n";

/* Filter blit block size: 5 or 9 */
#define gcdFILTER_BLOCK_SIZE    9

struct gcoContext
{
    /* Feature: 2D separated core? */
    gctBOOL                          separated2D;

    /*
       ** GC Objects.
    */

    /* OS object. */
    gcoOS                            os;

    /* HAL object. */
    gcoHAL                           hal;

    /* 2D Raster engine */
    gco2D                            engine2D;

    /* Alpha blending */
    gctBOOL                          blending;

    /* Global alpha enablement */
    gctBOOL                          global_alpha_enable;

    /* Dither */
    gctBOOL                          dither;

    /* Dim effect */
    gctBOOL                          blend_dim;

    /* blur effect */
    gctBOOL                          blur;

    /* Thread ID */
    gctHANDLE                        threadID;

    /* Current hardware type */
    enum g2d_hardware_type           current_type;

    /* 2D base address */
    gctUINT32                        baseAddress2D;

    /* Feature: 2D mirror externsion. */
    gctBOOL                          mirrorExt;

#if g2dENABLE_VG
    /* VG Raster engine */
    gcoVG                            engineVG;

    /* Feature: VG separated core? */
    gctBOOL                          separatedVG;

    /* VG base address */
    gctUINT32                        baseAddressVG;
#endif

    /* Clipping 2D flag */
    gctBOOL                          clipping2D;

    /* Clipping rectangle */
    gcsRECT                          clipRect2D;
};

struct g2d_buf_context
{
    void *handle;
    int  cacheable;
    int  physical;
};

int g2d_open(void **handle)
{
    gctINT i;
    gcsHAL_INTERFACE iface;
    gceSTATUS  status    = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_INVALID;
    gctBOOL has2DPipe = gcvFALSE, hasVGPipe = gcvFALSE;

    if(!handle)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    *handle = (void *)0x0;

    context = (struct gcoContext *)malloc(sizeof(struct gcoContext));
    memset(context, 0, sizeof(struct gcoContext));

    /* Initialize GC stuff. */
    /* Construct os object. */
    gcmONERROR(
        gcoOS_Construct(gcvNULL, &context->os));

    /* Construct hal object. */
    gcmONERROR(
        gcoHAL_Construct(gcvNULL, context->os, &context->hal));

#if g2dENABLE_VG
    /* Query the kernel version number. */
    iface.command = gcvHAL_VERSION;
    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    /* Test if versions match. */
    if ((iface.u.Version.major != gcvVERSION_MAJOR)
         ||(iface.u.Version.minor != gcvVERSION_MINOR)
         ||(iface.u.Version.patch != gcvVERSION_PATCH)
         ||(iface.u.Version.build != gcvVERSION_BUILD)
    )
    {
         g2d_printf("g2d user version %d.%d.%d build %u %s %s\n",
                   gcvVERSION_MAJOR, gcvVERSION_MINOR, gcvVERSION_PATCH,
                   gcvVERSION_BUILD, gcvVERSION_DATE, gcvVERSION_TIME);
         g2d_printf("gpu kernel version %d.%d.%d build %u\n",
                   iface.u.Version.major, iface.u.Version.minor,
                   iface.u.Version.patch, iface.u.Version.build);

         gcmONERROR(gcvSTATUS_VERSION_MISMATCH);
    }
#endif

    /* Query chip info */
    iface.command = gcvHAL_CHIP_INFO;

    gcmONERROR(gcoOS_DeviceControl(gcvNULL,
                                   IOCTL_GCHAL_INTERFACE,
                                   &iface, gcmSIZEOF(iface),
                                   &iface, gcmSIZEOF(iface)));

    for (i = 0; i < iface.u.ChipInfo.count; i++)
    {
        switch (iface.u.ChipInfo.types[i])
        {
        case gcvHARDWARE_3D2D:
            has2DPipe = gcvTRUE;
            break;
        case gcvHARDWARE_2D:
            has2DPipe = gcvTRUE;
            context->separated2D = gcvTRUE;
            break;
#if g2dENABLE_VG
        case gcvHARDWARE_VG:
            hasVGPipe = gcvTRUE;
            context->separatedVG = gcvTRUE;
            break;
#endif
        default:
            break;
        }
    }

    /* Check 2D pipe existance. */
    if (has2DPipe == gcvFALSE && hasVGPipe == gcvFALSE)
    {
        g2d_printf("%s: 2D/VG PIPE not found!\n", __FUNCTION__);
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    context->baseAddress2D = 0;

    /* Switch to 2D core if has separated 2D core. */
    if (has2DPipe)
    {
        if (context->separated2D)
        {
            /* Query current hardware type. */
            gcmONERROR(
                gcoHAL_GetHardwareType(context->hal, &currentType));

            gcmONERROR(
                gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D));
        }

        gcmONERROR(
            gcoOS_GetBaseAddress(gcvNULL, &context->baseAddress2D));

        /* Check mirror externsion. */
        context->mirrorExt =
            gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_2D_MIRROR_EXTENSION);

        context->clipping2D = gcvFALSE;
    }

    /* Get gco2D object pointer. */
    gcmONERROR(
        gcoHAL_Get2DEngine(context->hal, &context->engine2D));

    if (has2DPipe && context->separated2D)
    {
        gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

#if g2dENABLE_VG
    context->baseAddressVG = 0;

    /* Switch to VG core if has separated VG core. */
    if (hasVGPipe && context->separatedVG)
    {
       /* Query current hardware type. */
       gcmONERROR(
            gcoHAL_GetHardwareType(context->hal, &currentType));

        gcmONERROR(
            gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_VG));

        gcmONERROR(
            gcoOS_GetBaseAddress(gcvNULL, &context->baseAddressVG));

        /* Get gcoVG object pointer. */
        gcmONERROR(
            gcoHAL_GetVGEngine(context->hal, &context->engineVG));

        gcmVERIFY_OK(
           gcoHAL_SetHardwareType(gcvNULL, currentType));
    }
#endif

    context->threadID = gcoOS_GetCurrentThreadID();

    if(has2DPipe)
    {
        context->current_type = G2D_HARDWARE_2D;
    }
#if g2dENABLE_VG
    else if(hasVGPipe)
    {
        context->current_type = G2D_HARDWARE_VG;
    }
#endif

    *handle = (void *)context;

    return 0;

OnError:
    if (context != gcvNULL)
    {
        if (currentType != gcvHARDWARE_INVALID)
        {
            gcmVERIFY_OK(
                gcoHAL_SetHardwareType(gcvNULL, currentType));
        }

        if (context->hal != gcvNULL)
        {
            gcmVERIFY_OK(
                gcoHAL_Destroy(context->hal));

            gcmVERIFY_OK(
                gcoOS_Destroy(context->os));
        }

        free(context);
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}

int g2d_make_current(void *handle, enum g2d_hardware_type type)
{
    struct gcoContext * context = gcvNULL;

    context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    if(context->current_type == type) return 0;

    switch(type)
    {
    case G2D_HARDWARE_2D:
       if(context->engine2D)
       {
           context->current_type = type;
       }
       break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
       if(context->engineVG)
       {
           context->current_type = type;
       }
       break;
#endif
    default:
       break;
    }

    if(context->current_type != type)
    {
       g2d_printf("%s: fail to set current hardware type to %d\n", __FUNCTION__, type);
       return -1;
    }

    return 0;
}

static int g2d_clear_2d(void *handle, struct g2d_surface *area)
{
    gcsRECT  clrRect;
    gctUINT32 dstBits,clrColor = 0;
    gceSTATUS  status = gcvSTATUS_OK;
    gctINT clrWidth, clrHeight, clrStride;
    struct gcoContext * context = gcvNULL;
    gceSURF_FORMAT dstFormat = gcvSURF_UNKNOWN;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;

    clrWidth = area->right - area->left;
    clrHeight = area->bottom - area->top;

    if(clrWidth <=0 || clrHeight <= 0)
    {
        g2d_printf("%s: Invalid clear rect, left %d, top %d, right %d, bottom %d!\n", __FUNCTION__, area->left, area->top, area->right, area->bottom);
        return -1;
    }

    clrStride = area->stride;
    if(clrStride <= 0) clrStride = clrWidth;

    switch(area->format)
    {
      case G2D_RGB565:
        dstBits = 16;
        dstFormat = gcvSURF_R5G6B5;
        break;
      case G2D_RGBA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8B8G8R8;
        break;
      case G2D_RGBX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8B8G8R8;
        break;
      case G2D_BGRA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8R8G8B8;
        break;
      case G2D_BGRX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8R8G8B8;
        break;
      case G2D_BGR565:
        dstBits = 16;
        dstFormat = gcvSURF_B5G6R5;
        break;
      case G2D_ARGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8A8;
        break;
      case G2D_ABGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8A8;
        break;
      case G2D_XRGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8X8;
        break;
      case G2D_XBGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8X8;
        break;
      default:
        g2d_printf("%s: surface format %d is not supported !\n", __FUNCTION__, area->format);
        return -1;
        break;
    }

    clrStride = clrStride * dstBits / 8;

    //convert to BGRA8888
    clrColor = ((area->clrcolor >> 16) & 0xff)
               | ((area->clrcolor & 0xff) << 16)
               | (area->clrcolor & 0xff00ff00);

    /* Set clear rect. */
    clrRect.left = area->left;
    clrRect.top = area->top;
    clrRect.right = area->right;
    clrRect.bottom = area->bottom;

    /* Set current hardware type to 2D. */
    if (context->separated2D)
    {
       /* Query current hardware type. */
       gcoHAL_GetHardwareType(context->hal, &currentType);

       gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
    }

    /* Disable alpha blending. */
    gcmONERROR(
        gco2D_DisableAlphaBlend(context->engine2D));

    /* No premultiply. */
    gcmONERROR(
        gco2D_SetPixelMultiplyModeAdvanced(context->engine2D,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                           gcv2D_COLOR_MULTIPLY_DISABLE));

    /***************************************************************************
     ** Setup Target.
     */
    gcmONERROR(
          gco2D_SetTarget(context->engine2D,
                     area->planes[0] - context->baseAddress2D,
                     clrStride,
                     gcvSURF_0_DEGREE,
                     0));

    /* Set clip rectangle. */
    gcmONERROR(gco2D_SetClipping(context->engine2D, &clrRect));

    /* Perform a Clear. */
    gcmONERROR(
        gco2D_Clear(context->engine2D,
                    1U,
                    &clrRect,
                    clrColor,
                    0xCC,
                    0xCC,
                    dstFormat));

    if (context->separated2D)
    {
        gcmVERIFY_OK(
            gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    return 0;

OnError:
    if (context->separated2D)
    {
        gcmVERIFY_OK(
            gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}

#if g2dENABLE_VG
static gceSTATUS g2d_construct_vg_surface(void *handle, gcoSURF *vg_surf, struct g2d_surface *g2d_surf)
{
    gctUINT32 pixBits;
    gctUINT32 yuvBuffers[3];
    gcoSURF surface = gcvNULL;
    gctBOOL videoMode = gcvFALSE;
    gceSTATUS  status = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gceSURF_FORMAT pixFormat = gcvSURF_UNKNOWN;

    context = (struct gcoContext *)handle;

    if(vg_surf == gcvNULL || g2d_surf == NULL)
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    memset(&yuvBuffers[0], 0, sizeof(yuvBuffers));

    switch(g2d_surf->format)
    {
      case G2D_RGB565:
        pixBits = 16;
        pixFormat = gcvSURF_R5G6B5;
        break;
      case G2D_RGBA8888:
        pixBits = 32;
        pixFormat = gcvSURF_A8B8G8R8;
        break;
      case G2D_RGBX8888:
        pixBits = 32;
        pixFormat = gcvSURF_X8B8G8R8;
        break;
      case G2D_BGRA8888:
        pixBits = 32;
        pixFormat = gcvSURF_A8R8G8B8;
        break;
      case G2D_BGRX8888:
        pixBits = 32;
        pixFormat = gcvSURF_X8R8G8B8;
        break;
      case G2D_BGR565:
        pixBits = 16;
        pixFormat = gcvSURF_B5G6R5;
        break;
      case G2D_ARGB8888:
        pixBits = 32;
        pixFormat = gcvSURF_B8G8R8A8;
        break;
      case G2D_ABGR8888:
        pixBits = 32;
        pixFormat = gcvSURF_R8G8B8A8;
        break;
      case G2D_XRGB8888:
        pixBits = 32;
        pixFormat = gcvSURF_B8G8R8X8;
        break;
      case G2D_XBGR8888:
        pixBits = 32;
        pixFormat = gcvSURF_R8G8B8X8;
        break;
      case G2D_NV12:
        pixBits = 8;
        pixFormat = gcvSURF_NV12;
        yuvBuffers[0] = g2d_surf->planes[0] - context->baseAddressVG;
        yuvBuffers[1] = g2d_surf->planes[1] - context->baseAddressVG;
        videoMode = gcvTRUE;
        break;
      case G2D_YUYV:
        pixBits = 16;
        pixFormat = gcvSURF_YUY2;
        yuvBuffers[0] = g2d_surf->planes[0] - context->baseAddressVG;
        videoMode = gcvTRUE;
        break;
      case G2D_NV16:
        pixBits = 8;
        pixFormat = gcvSURF_NV16;
        yuvBuffers[0] = g2d_surf->planes[0] - context->baseAddressVG;
        yuvBuffers[1] = g2d_surf->planes[1] - context->baseAddressVG;
        videoMode = gcvTRUE;
        break;
      default:
        g2d_printf("%s: surface format %d is not support by hardware vg !\n", __FUNCTION__, g2d_surf->format);
        return gcvSTATUS_INVALID_ARGUMENT;
        break;
    }

    gcmONERROR(
        gcoSURF_ConstructWrapper(gcvNULL,
		&surface
		));

    if(surface == gcvNULL)
    {
         return gcvSTATUS_OUT_OF_RESOURCES;
    }

    gcmONERROR(
        gcoSURF_SetOrientation(
		surface,
		gcvORIENTATION_BOTTOM_TOP
		));


    /* It's ok to always use videoMode, but we don't do that. */
    //yuvBuffers[0] = g2d_surf->planes[0] - context->baseAddressVG;
    //videoMode = gcvTRUE;
    if(videoMode == gcvTRUE)
    {
        gcmONERROR(
            gcoSURF_SetVideoBuffer(
                    surface,
                    gcvSURF_BITMAP,
                    pixFormat,
                    g2d_surf->width,
                    g2d_surf->height,
                    g2d_surf->stride * pixBits / 8,
                    gcvNULL,//no logical address input
                    &yuvBuffers[0]
		    ));
    }
    else
    {
        gcmONERROR(
            gcoSURF_SetBuffer(
                    surface,
                    gcvSURF_BITMAP,
                    pixFormat,
                    g2d_surf->stride * pixBits / 8,
                    (gctPOINTER)0x12345678,//avoid mapping crash in surfaceflinger
                    g2d_surf->planes[0] - context->baseAddressVG
		    ));

        /* Set the window. */
        gcmONERROR(
            gcoSURF_SetWindow(
                    surface,
                    0, 0,
                    g2d_surf->width,
                    g2d_surf->height
                    ));
        gcmONERROR(
            gcoSURF_Lock(surface, gcvNULL, gcvNULL));

    }

    gcmONERROR(
        gcoSURF_SetColorType(
		surface,
		gcvSURF_COLOR_UNKNOWN
		));

    *vg_surf = surface;

    return status;

OnError:

    if(surface)
    {
        gcoSURF_Unlock(surface, gcvNULL);
         gcoSURF_Destroy(surface);
    }

    return status;
}

static int g2d_clear_vg(void *handle, struct g2d_surface *area)
{
    gcoSURF vg_surface;
    gctINT clrWidth, clrHeight;
    gceSTATUS  status = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;

    clrWidth = area->right - area->left;
    clrHeight = area->bottom - area->top;

    if(clrWidth <=0 || clrHeight <= 0)
    {
        g2d_printf("%s: Invalid clear rect, left %d, top %d, right %d, bottom %d!\n", __FUNCTION__, area->left, area->top, area->right, area->bottom);
        return -1;
    }

    /* Set current hardware type to VG. */
    if (context->separatedVG)
    {
       /* Query current hardware type. */
       gcoHAL_GetHardwareType(context->hal, &currentType);

       gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_VG);
    }

    gcmONERROR(
        g2d_construct_vg_surface(handle, &vg_surface,
                area
                ));

    gcmONERROR(
        gcoVG_SetTarget(
		context->engineVG, vg_surface, gcvORIENTATION_BOTTOM_TOP
		));

    gcmONERROR(
        gcoVG_EnableMask(
		context->engineVG, gcvFALSE
		));

    gcmONERROR(
        gcoVG_SetImageMode(
		context->engineVG, gcvVG_IMAGE_NONE
		));

    gcmONERROR(
	gcoVG_SetBlendMode(
		context->engineVG, gcvVG_BLEND_SRC
		));

    gcmONERROR(
        gcoVG_EnableScissor(
		context->engineVG, gcvFALSE
		));

    gcmONERROR(
	gcoVG_EnableColorTransform(
		context->engineVG, gcvFALSE
		));

    gcmONERROR(
	gcoVG_SetSolidPaint(
		context->engineVG,
		area->clrcolor & 0xff,
		(area->clrcolor >> 8) & 0xff,
		(area->clrcolor >> 16) & 0xff,
		(area->clrcolor >> 24) & 0xff
		));

    gcmONERROR(
	gcoVG_Clear(
		context->engineVG,
		area->left,
		area->top,
		clrWidth, clrHeight
		));

    gcmONERROR(
        gcoSURF_Destroy((gcoSURF) vg_surface));

    if (context->separatedVG)
    {
        gcmVERIFY_OK(
            gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    return 0;

OnError:
    if (context->separatedVG)
    {
        gcmVERIFY_OK(
            gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}
#endif

int g2d_clear(void *handle, struct g2d_surface *area)
{
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!area)
    {
        g2d_printf("%s: Invalid area parameter!\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

#if g2dENABLE_VG
    if(context->current_type == G2D_HARDWARE_VG)
    {
        if((area->planes[0] - context->baseAddress2D) & 0x80000000)
        {
            //roll back to 2D when the address is mapped by 3D MMU
            g2d_make_current(handle, G2D_HARDWARE_2D);
        }
    }
#endif

    switch(context->current_type)
    {
    case G2D_HARDWARE_2D:
    default:
        return g2d_clear_2d(handle, area);
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
        return g2d_clear_vg(handle, area);
        break;
#endif
    }

    return 0;
}

int g2d_query_hardware(void *handle, enum g2d_hardware_type type, int *available)
{

    struct gcoContext * context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!available) return -1;

    switch(type)
    {
    case G2D_HARDWARE_2D:
        *available = (context->engine2D != gcvNULL) ? 1 : 0;
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
        *available = (context->engineVG != gcvNULL) ? 1 : 0;
        break;
#endif
    default:
        break;
    }

    return 0;
}

int g2d_query_feature(void *handle, enum g2d_feature feature, int *available)
{
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!available) return -1;

    switch(feature)
    {
    case G2D_SCALING:
    case G2D_SRC_YUV:
        *available = 1;
        break;
    case G2D_ROTATION:
    case G2D_DST_YUV:
        *available = (context->current_type == G2D_HARDWARE_2D) ? 1 : 0;
        break;
    case G2D_MULTI_SOURCE_BLT:
        *available = ((context->current_type == G2D_HARDWARE_2D) ? 1 : 0) & ((int)(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_2D_MULTI_SOURCE_BLT_EX) == gcvTRUE));
        break;
    default:
        break;
    }

    return 0;
}

int g2d_query_cap(void *handle, enum g2d_cap_mode cap, int *enable)
{
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!enable) return 0;

    switch(cap)
    {
    case G2D_BLEND:
        *enable = (int)(context->blending == gcvTRUE);
        break;
    case G2D_GLOBAL_ALPHA:
        *enable = (int)(context->global_alpha_enable == gcvTRUE);
        break;
    case G2D_DITHER:
        *enable = (int)(context->dither == gcvTRUE);
        break;
    case G2D_BLEND_DIM:
        *enable = (int)(context->blend_dim == gcvTRUE);
        break;
    default:
        break;
    }

    return 0;

}
int g2d_enable(void *handle, enum g2d_cap_mode cap)
{
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    switch(cap)
    {
    case G2D_BLEND:
        context->blending = gcvTRUE;
        break;
    case G2D_GLOBAL_ALPHA:
        context->global_alpha_enable = gcvTRUE;
        break;
    case G2D_DITHER:
        context->dither = gcvTRUE;
        if(context->current_type == G2D_HARDWARE_2D)
        {
            gco2D_EnableDither(context->engine2D, gcvTRUE);
        }
#if g2dENABLE_VG
        else if(context->current_type == G2D_HARDWARE_VG)
        {
            gcoVG_EnableDither(context->engineVG, gcvTRUE);
        }
#endif
        break;
    case G2D_BLEND_DIM:
        context->blend_dim = gcvTRUE;
        break;
    case  G2D_BLUR:
        context->blur = gcvTRUE;
        break;
    default:
        break;
    }

    return 0;
}
int g2d_disable(void *handle, enum g2d_cap_mode cap)
{
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    switch(cap)
    {
    case G2D_BLEND:
        context->blending = gcvFALSE;
        break;
    case G2D_GLOBAL_ALPHA:
        context->global_alpha_enable = gcvFALSE;
        break;
    case G2D_DITHER:
        context->dither = gcvFALSE;
        if(context->current_type == G2D_HARDWARE_2D)
        {
            gco2D_EnableDither(context->engine2D, gcvFALSE);
        }
#if g2dENABLE_VG
        else if(context->current_type == G2D_HARDWARE_VG)
        {
            gcoVG_EnableDither(context->engineVG, gcvFALSE);
        }
#endif
        break;
    case G2D_BLEND_DIM:
        context->blend_dim = gcvFALSE;
        break;
    case G2D_BLUR:
        context->blur = gcvFALSE;
        break;
    default:
        break;
    }

    return 0;
}

static gctBOOL
_HasAlpha(
    IN gceSURF_FORMAT Format
    )
{
    return (Format == gcvSURF_R5G6B5) ? gcvFALSE
         : (
               (Format == gcvSURF_A8R8G8B8)
            || (Format == gcvSURF_A8B8G8R8)
            || (Format == gcvSURF_B8G8R8A8)
            || (Format == gcvSURF_R8G8B8A8)
           );
}

int g2d_set_clipping(void *handle, int left, int top, int right, int bottom)
{
    struct gcoContext * context = gcvNULL;

    context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    context->clipRect2D.left = left;
    context->clipRect2D.top = top;
    context->clipRect2D.right = right;
    context->clipRect2D.bottom = bottom;

    context->clipping2D = gcvTRUE;

    return 0;
}

static int g2d_blit_2d(void *handle, struct g2d_surfaceEx *srcEx, struct g2d_surfaceEx *dstEx)
{
    gctUINT32 physAddress;
    gctBOOL hMirror, vMirror;
    gctUINT32 srcBits, dstBits;
    gctBOOL filterblit = gcvFALSE;
    gctBOOL stretchblit = gcvFALSE;
    gceTILING srcTile, dstTile;
    gceSURF_ROTATION srcRot, dstRot;
    gctUINT32 Stride,uStride,vStride;
    gcsRECT srcRect, dstRect, subRect;
    gceSTATUS  status    = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gceSURF_FORMAT srcFormat = gcvSURF_UNKNOWN;
    gceSURF_FORMAT dstFormat = gcvSURF_UNKNOWN;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;

    struct g2d_surface *src = (struct g2d_surface *)srcEx;
    struct g2d_surface *dst = (struct g2d_surface *)dstEx;

    srcBits = 8;
    uStride = vStride = 0;

    switch(src->format)
    {
      case G2D_RGB565:
        srcBits = 16;
        srcFormat = gcvSURF_R5G6B5;
        break;
      case G2D_RGBA8888:
        srcBits = 32;
        srcFormat = gcvSURF_A8B8G8R8;
        break;
      case G2D_RGBX8888:
        srcBits = 32;
        srcFormat = gcvSURF_X8B8G8R8;
        break;
      case G2D_BGRA8888:
        srcBits = 32;
        srcFormat = gcvSURF_A8R8G8B8;
        break;
      case G2D_BGRX8888:
        srcBits = 32;
        srcFormat = gcvSURF_X8R8G8B8;
        break;
      case G2D_BGR565:
        srcBits = 16;
        srcFormat = gcvSURF_B5G6R5;
        break;
      case G2D_ARGB8888:
        srcBits = 32;
        srcFormat = gcvSURF_B8G8R8A8;
        break;
      case G2D_ABGR8888:
        srcBits = 32;
        srcFormat = gcvSURF_R8G8B8A8;
        break;
      case G2D_XRGB8888:
        srcBits = 32;
        srcFormat = gcvSURF_B8G8R8X8;
        break;
      case G2D_XBGR8888:
        srcBits = 32;
        srcFormat = gcvSURF_R8G8B8X8;
        break;
      case G2D_NV12:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_NV12;
        uStride = src->stride;
        break;
      case G2D_NV21:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_NV21;
        uStride = src->stride;
        break;
      case G2D_I420:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_I420;
        uStride = vStride = src->stride / 2;
        break;
      case G2D_YV12:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_YV12;
        uStride = vStride = src->stride / 2;
        break;
      case G2D_YUYV:
        srcBits = 16;
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_YUY2;
        break;
      case G2D_YVYU:
        srcBits = 16;
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_YVYU;
        break;
      case G2D_UYVY:
        srcBits = 16;
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_UYVY;
        break;
      case G2D_VYUY:
        srcBits = 16;
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_VYUY;
        break;
      case G2D_NV16:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_NV16;
        uStride = src->stride;
        break;
      case G2D_NV61:
        filterblit = gcvTRUE;
        srcFormat = gcvSURF_NV61;
        uStride = src->stride;
        break;
      default:
        g2d_printf("%s: Invalid src format %d!\n", __FUNCTION__, src->format);
        return -1;
        break;
    }

    if(context->blend_dim && (srcBits != 32 || filterblit == gcvTRUE))
    {
        g2d_printf("%s: invalid color format for dim blit !\n", __FUNCTION__);
        return -1;
    }

    dstBits = 8;

    switch(dst->format)
    {
      case G2D_RGB565:
        dstBits = 16;
        dstFormat = gcvSURF_R5G6B5;
        break;
      case G2D_RGBA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8B8G8R8;
        break;
      case G2D_RGBX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8B8G8R8;
        break;
      case G2D_BGRA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8R8G8B8;
        break;
      case G2D_BGRX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8R8G8B8;
        break;
      case G2D_BGR565:
        dstBits = 16;
        dstFormat = gcvSURF_B5G6R5;
        break;
      case G2D_ARGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8A8;
        break;
      case G2D_ABGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8A8;
        break;
      case G2D_XRGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8X8;
        break;
      case G2D_XBGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8X8;
        break;
      case G2D_YUYV:
        dstBits = 16;
        filterblit = gcvTRUE;
        dstFormat = gcvSURF_YUY2;
        break;
      default:
        g2d_printf("%s: Invalid dst format %d!\n", __FUNCTION__, dst->format);
        return -1;
        break;
    }

    srcTile = gcvLINEAR;
    dstTile = gcvLINEAR;

    switch(srcEx->tiling)
    {
    case G2D_LINEAR:
    default:
        srcTile = gcvLINEAR;
        break;
    case G2D_TILED:
        srcTile = gcvTILED;
        break;
    case G2D_SUPERTILED:
        srcTile = gcvSUPERTILED;
        break;
    }

    switch(dstEx->tiling)
    {
    case G2D_LINEAR:
    default:
        dstTile = gcvLINEAR;
        break;
    case G2D_TILED:
        dstTile = gcvTILED;
        break;
    case G2D_SUPERTILED:
        dstTile = gcvSUPERTILED;
        break;
    }

    hMirror  = gcvFALSE;
    vMirror  = gcvFALSE;

    srcRot = gcvSURF_0_DEGREE;
    dstRot = gcvSURF_0_DEGREE;

    switch(src->rot)
    {
      case G2D_ROTATION_0:
      default:
        srcRect.left = src->left;
        srcRect.top = src->top;
        srcRect.right = src->right;
        srcRect.bottom = src->bottom;

        break;
      case G2D_ROTATION_90:
        srcRect.left = src->top;
        srcRect.top = src->width - src->right;
        srcRect.right = src->bottom;
        srcRect.bottom = src->width - src->left;

        srcRot = gcvSURF_90_DEGREE;
        break;
      case G2D_ROTATION_180:
        srcRect.left = src->width - src->right;
        srcRect.top = src->height - src->bottom;
        srcRect.right = src->width - src->left;
        srcRect.bottom = src->height - src->top;

        srcRot = gcvSURF_180_DEGREE;
        break;
      case G2D_ROTATION_270:
        srcRect.left = src->height - src->bottom;
        srcRect.top = src->left;
        srcRect.right = src->height - src->top;
        srcRect.bottom = src->right;

        srcRot = gcvSURF_270_DEGREE;
        break;
      case G2D_FLIP_H:
        srcRect.left = src->left;
        srcRect.top = src->top;
        srcRect.right = src->right;
        srcRect.bottom = src->bottom;

        hMirror = gcvTRUE;
        break;
      case G2D_FLIP_V:
        srcRect.left = src->left;
        srcRect.top = src->top;
        srcRect.right = src->right;
        srcRect.bottom = src->bottom;

        vMirror = gcvTRUE;
        break;
    }

    switch(dst->rot)
    {
      case G2D_ROTATION_0:
      default:
        dstRect.left = dst->left;
        dstRect.top = dst->top;
        dstRect.right = dst->right;
        dstRect.bottom = dst->bottom;

        if(context->clipping2D)
        {
            subRect = context->clipRect2D;
        }
        break;
      case G2D_ROTATION_90:
        dstRect.left = dst->top;
        dstRect.top = dst->width - dst->right;
        dstRect.right = dst->bottom;
        dstRect.bottom = dst->width - dst->left;

        if(context->clipping2D)
        {
            subRect.left = context->clipRect2D.top;
            subRect.top  = dst->width - context->clipRect2D.right;
            subRect.right = context->clipRect2D.bottom;
            subRect.bottom = dst->width - context->clipRect2D.left;
        }

        dstRot = gcvSURF_90_DEGREE;
        break;
      case G2D_ROTATION_180:
        dstRect.left = dst->width - dst->right;
        dstRect.top = dst->height - dst->bottom;
        dstRect.right = dst->width - dst->left;
        dstRect.bottom = dst->height - dst->top;

        if(context->clipping2D)
        {
            subRect.left = dst->width - context->clipRect2D.right;
            subRect.top  = dst->height - context->clipRect2D.bottom;
            subRect.right = dst->width - context->clipRect2D.left;
            subRect.bottom = dst->height - context->clipRect2D.top;
        }

        dstRot = gcvSURF_180_DEGREE;
        break;
      case G2D_ROTATION_270:
        dstRect.left = dst->height - dst->bottom;
        dstRect.top = dst->left;
        dstRect.right = dst->height - dst->top;
        dstRect.bottom = dst->right;

        if(context->clipping2D)
        {
            subRect.left = dst->height - context->clipRect2D.bottom;
            subRect.top  = context->clipRect2D.left;
            subRect.right = dst->height - context->clipRect2D.top;
            subRect.bottom = context->clipRect2D.right;
        }

        dstRot = gcvSURF_270_DEGREE;
        break;
      case G2D_FLIP_H:
        dstRect.left = dst->left;
        dstRect.top = dst->top;
        dstRect.right = dst->right;
        dstRect.bottom = dst->bottom;

        if(context->clipping2D)
        {
            subRect = context->clipRect2D;
        }

        hMirror = gcvTRUE;
        break;
      case G2D_FLIP_V:
        dstRect.left = dst->left;
        dstRect.top = dst->top;
        dstRect.right = dst->right;
        dstRect.bottom = dst->bottom;

        if(context->clipping2D)
        {
            subRect = context->clipRect2D;
        }

        vMirror = gcvTRUE;
        break;
     }

    if((srcRect.right-srcRect.left != dstRect.right-dstRect.left)
       || (srcRect.bottom-srcRect.top != dstRect.bottom-dstRect.top))
    {
        if(filterblit == gcvFALSE)
        {
            //the workaround for pioneer test case, blit one pixel line
            if((srcRect.right-srcRect.left == 1 || srcRect.bottom-srcRect.top == 1) && context->clipping2D == gcvFALSE)
            {
                stretchblit = gcvTRUE;
            }
            else
            {
                filterblit = gcvTRUE;
            }
        }
    }

    if(context->blend_dim)
    {
        filterblit = gcvFALSE;
    }

    if(context->blur)
    {
        filterblit = gcvTRUE;
    }

    /* Set current hardware type to 2D. */
    if (context->separated2D)
    {
       /* Query current hardware type. */
       gcoHAL_GetHardwareType(context->hal, &currentType);

       gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
    }

    /* Setup source index. */
    gcmONERROR(
        gco2D_SetCurrentSourceIndex(context->engine2D, 0U));

    if(context->blending || context->blend_dim)
    {
       /***************************************************************************
       ** Alpha blending and premultiply parametes.
       */

       /* Alpha blending mode. */
       gceSURF_PIXEL_ALPHA_MODE         srcAlphaMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;
       gceSURF_PIXEL_ALPHA_MODE         dstAlphaMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;

       /* Perpixel alpha premultiply. */
       gce2D_PIXEL_COLOR_MULTIPLY_MODE  srcPremultSrcAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;
       gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstPremultDstAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;

       /* Global alpha blending mode. */
       gceSURF_GLOBAL_ALPHA_MODE        srcGlobalAlphaMode = gcvSURF_GLOBAL_ALPHA_OFF;
       gceSURF_GLOBAL_ALPHA_MODE        dstGlobalAlphaMode = gcvSURF_GLOBAL_ALPHA_OFF;

       /* Global alpha premultiply. */
       gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;
       //gce2D_GLOBAL_COLOR_MULTIPLY_MODE dstPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;

       /* Dest global alpha demultiply. */
       gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstDemultDstAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;

       /* Blend factor. */
       gceSURF_BLEND_FACTOR_MODE        srcFactorMode = gcvSURF_BLEND_ONE;
       gceSURF_BLEND_FACTOR_MODE        dstFactorMode = gcvSURF_BLEND_ONE;

       /* Global alpha value. */
       gctUINT32                        srcGlobalAlpha = 0xFF000000;
       gctUINT32                        dstGlobalAlpha = 0xFF000000;

       /* Get perpixelAlpha enabling state. */
       gctBOOL perpixelAlpha = _HasAlpha(srcFormat);

       switch(src->blendfunc & 0xf)
       {
          case G2D_ZERO://Cs = Cs * 0
             srcFactorMode        = gcvSURF_BLEND_ZERO;
             break;
          case G2D_ONE: //Cs = Cs * 1
             srcFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }
             break;
          case G2D_SRC_ALPHA://Cs = Cs * As * 1
             srcFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha)
             {
                 srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                 if(src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA)
                 {
                    g2d_printf("%s: G2D_PRE_MULTIPLIED_ALPHA should not be set with G2D_SRC_ALPHA, ignored\n", __FUNCTION__);
                 }
             }
             break;
          case G2D_ONE_MINUS_SRC_ALPHA://Cs = Cs * (1 - As) * 1
             srcFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha)
             {
                 srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 srcAlphaMode         = gcvSURF_PIXEL_ALPHA_INVERSED;
             }
             break;
          case G2D_DST_ALPHA://Cs = Cs * Ad
             srcFactorMode        = gcvSURF_BLEND_STRAIGHT;
             if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }

             break;
          case G2D_ONE_MINUS_DST_ALPHA://Cs = Cs * (1 - Ad)
             srcFactorMode        = gcvSURF_BLEND_INVERSED;
             if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }
             break;
          default:
             srcFactorMode        = gcvSURF_BLEND_ONE;
             break;
       }

       if(context->global_alpha_enable && src->global_alpha < 0xFF)
       {
           srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA;
           srcGlobalAlpha       = src->global_alpha << 24;

           if(perpixelAlpha)
           {
               srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_SCALE;
           }
           else
           {
               srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_ON;
           }
       }

       perpixelAlpha = _HasAlpha(dstFormat);

       switch(dst->blendfunc & 0xf)
       {
          case G2D_ZERO://Cd = Cd * 0
             dstFactorMode        = gcvSURF_BLEND_ZERO;
             break;
          case G2D_ONE://Cd = Cd * 1
             dstFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }
             break;
          case G2D_SRC_ALPHA://Cd = Cd * As
             dstFactorMode        = gcvSURF_BLEND_STRAIGHT;
             if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }
             break;
          case G2D_ONE_MINUS_SRC_ALPHA://Cd = Cd * (1 - As)
             dstFactorMode        = gcvSURF_BLEND_INVERSED;
             if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
             {
                 dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                 dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
             }
             break;
          case G2D_DST_ALPHA://Cd = Cd * Ad * 1
             dstFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha)
             {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                if(dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA)
                {
                    g2d_printf("%s: G2D_PRE_MULTIPLIED_ALPHA should not be set with G2D_DST_ALPHA, ignored\n", __FUNCTION__);
                }
             }
             break;
          case G2D_ONE_MINUS_DST_ALPHA://Cd = Cd * (1 - Ad) * 1
             dstFactorMode        = gcvSURF_BLEND_ONE;
             if(perpixelAlpha)
             {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_INVERSED;
             }
             break;
          default:
             dstFactorMode        = gcvSURF_BLEND_ONE;
             break;
       }

       if(dst->blendfunc & G2D_DEMULTIPLY_OUT_ALPHA)
       {
           dstDemultDstAlpha = gcv2D_COLOR_MULTIPLY_ENABLE;
       }

#if 0 //dst global alpha does not work on gc320
       if(context->global_alpha_enable && dst->global_alpha < 0xFF)
       {
           dstPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA;
           dstGlobalAlpha       = dst->global_alpha << 24;

           if(perpixelAlpha)
           {
               dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_SCALE;
           }
           else
           {
               dstGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_ON;
           }
       }
#endif
       gcmONERROR(
            gco2D_EnableAlphaBlendAdvanced(context->engine2D,
                                           srcAlphaMode,
                                           dstAlphaMode,
                                           srcGlobalAlphaMode,
                                           dstGlobalAlphaMode,
                                           srcFactorMode,
                                           dstFactorMode));

       gcmONERROR(
            gco2D_SetPixelMultiplyModeAdvanced(context->engine2D,
                                               srcPremultSrcAlpha,
                                               dstPremultDstAlpha,
                                               srcPremultGlobalMode,
                                               dstDemultDstAlpha));

       gcmONERROR(
            gco2D_SetSourceGlobalColorAdvanced(context->engine2D,
                                               srcGlobalAlpha));

       gcmONERROR(
            gco2D_SetTargetGlobalColorAdvanced(context->engine2D,
                                               dstGlobalAlpha));
    }
    else
    {
       /* Disable alhpa blending. */
       gcmONERROR(
            gco2D_DisableAlphaBlend(context->engine2D));


       /* Disable premultiply. */
       gcmONERROR(
            gco2D_SetPixelMultiplyModeAdvanced(context->engine2D,
                     gcv2D_COLOR_MULTIPLY_DISABLE,
                     gcv2D_COLOR_MULTIPLY_DISABLE,
                     gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                     gcv2D_COLOR_MULTIPLY_DISABLE));
    }

    /* Setup mirror. */
    gcmONERROR(
       gco2D_SetBitBlitMirror(context->engine2D,
                              hMirror,
                              vMirror));

    Stride = dst->stride * dstBits / 8;

    /** Setup Target */

    physAddress = dst->planes[0] - context->baseAddress2D;

    gcmONERROR(
            gco2D_SetGenericTarget(context->engine2D,
                                  (gctUINT32_PTR)&physAddress,
                                  1U,
                                  &Stride,
                                  1U,
                                  dstTile,
                                  dstFormat,
                                  dstRot,
                                  dst->width,
                                  dst->height));

    if(filterblit == gcvTRUE)
    {
        gctUINT32 src_planes[3];
        gctUINT32 src_strides[3];
        gctUINT32 dst_planes0;

        if(context->blur == gcvFALSE)
        {
            gctFLOAT hFactor = (gctFLOAT)(srcRect.right  - srcRect.left)
                                        / (gctFLOAT) (dstRect.right  - dstRect.left);

            gctFLOAT vFactor = (gctFLOAT)(srcRect.bottom - srcRect.top)
                                        / (gctFLOAT) (dstRect.bottom - dstRect.top);

            gctINT hKernel = (hFactor == 1.0f / 1) ? 1U
                            : (hFactor >= 1.0f / 2) ? 3U : 9U;

            gctINT vKernel = (vFactor == 1.0f / 1) ? 1U
                            : (vFactor >= 1.0f / 2) ? 3U : 9U;

            /* Set kernel size. */
            gcmONERROR(gco2D_SetKernelSize(context->engine2D, hKernel, vKernel));

            gcmONERROR(
                    gco2D_SetFilterType(context->engine2D,
                                        gcvFILTER_SYNC));
        }
        else
        {
            /* Blur blit. */
            gcmONERROR(
                    gco2D_SetKernelSize(context->engine2D, gcdFILTER_BLOCK_SIZE, gcdFILTER_BLOCK_SIZE));

            gcmONERROR(
                    gco2D_SetFilterType(context->engine2D, gcvFILTER_BLUR));
        }

        /* Mirror in dstRect instead of dstSubRect. */
#if gco2D_SetStateU32_Fix
        {
            gctUINT32 Value = 1U;
            gcmVERIFY_OK(
                    gco2D_SetStateU32(context->engine2D,
                                  gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE, &Value));
        }
#else
        gcmVERIFY_OK(
                gco2D_SetStateU32(context->engine2D,
                              gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE, 1U));
#endif

        if (!context->mirrorExt && (hMirror || vMirror))
        {
            gcsRECT tmpRect = srcRect;

            if (hMirror && vMirror)
            {
                /* Use 180 degree rotation to simulate. */
                srcRot = gcvSURF_180_DEGREE;
                srcRect.left   = src->width  - tmpRect.right;
                srcRect.top    = src->height - tmpRect.bottom;
                srcRect.right  = src->width  - tmpRect.left;
                srcRect.bottom = src->height - tmpRect.top;
            }
            else if (hMirror)
            {
                /* Use FLIP_X to simulate. */
                srcRot = gcvSURF_FLIP_X;
                srcRect.left   = src->width  - tmpRect.right;
                srcRect.right  = src->width  - tmpRect.left;
            }
            else /* if (vMirror) */
            {
                /* Use FLIP_Y to simulate. */
                srcRot = gcvSURF_FLIP_Y;
                srcRect.top    = src->height - tmpRect.bottom;
                srcRect.bottom = src->height - tmpRect.top;
            }

            /* Disable source mirror. */
           hMirror = vMirror = 0;

           /* Setup mirror. */
           gcmONERROR(
               gco2D_SetBitBlitMirror(context->engine2D,
                                      hMirror,
                                      vMirror));
        }

        if(context->clipping2D)
        {
            subRect.left -= dstRect.left;
            subRect.top -= dstRect.top;
            subRect.right -= dstRect.left;
            subRect.bottom -= dstRect.top;
        }
        if(context->blur && !(context->clipping2D))
        {
            subRect.left  = 0;
            subRect.top   = 0;
            subRect.right = dstRect.right - dstRect.left;
            subRect.bottom = dstRect.bottom - dstRect.top;
        }

        src_planes[0] = src->planes[0] - context->baseAddress2D;
        src_planes[1] = src->planes[1] - context->baseAddress2D;
        src_planes[2] = src->planes[2] - context->baseAddress2D;

        src_strides[0] = src->stride * srcBits / 8;
        src_strides[1] = uStride * srcBits / 8;
        src_strides[2] = vStride * srcBits / 8;

        dst_planes0 = dst->planes[0] - context->baseAddress2D;

        /* Trigger filter blit. */
        gcmONERROR(
                gco2D_FilterBlitEx2(context->engine2D,
                                   &src_planes[0],
                                   3U,
                                   &src_strides[0],
                                   3U,
                                   srcTile,
                                   srcFormat,
                                   srcRot,
                                   src->width,
                                   src->height,
                                   &srcRect,
                                   &dst_planes0,
                                   1U,
                                   &Stride,
                                   1U,
                                   dstTile,
                                   dstFormat,
                                   dstRot,
                                   dst->width,
                                   dst->height,
                                   &dstRect,
                                   context->clipping2D ? &subRect : gcvNULL));


        if(context->blur)
        {
            gcmONERROR(
                gco2D_FilterBlitEx2(context->engine2D,
                                    &dst_planes0,
                                    1U,
                                    &Stride,
                                    1U,
                                    dstTile,
                                    dstFormat,
                                    gcvSURF_0_DEGREE,
                                    dst->width,
                                    dst->height,
                                    &dstRect,
                                    &dst_planes0,
                                    1U,
                                    &Stride,
                                    1U,
                                    dstTile,
                                    dstFormat,
                                    gcvSURF_0_DEGREE,
                                    dst->width,
                                    dst->height,
                                    &dstRect,
                                    &subRect));

        }

    }
    else if(stretchblit)
    {
        gctUINT32 hFactor,vFactor;
        gctUINT32 srcAddress = src->planes[0] - context->baseAddress2D;
        gctUINT32 srcStride = src->stride * srcBits / 8;

        gcmVERIFY_OK(
            gco2D_CalcStretchFactor(context->engine2D,
                                    src->right - src->left,
                                    dst->right - dst->left,
                                    &hFactor));

        gcmVERIFY_OK(
            gco2D_CalcStretchFactor(context->engine2D,
                                    src->bottom - src->top,
                                    dst->bottom - dst->top,
                                    &vFactor));

        /* Set source address. */
        gcmONERROR(
            gco2D_SetGenericSource(context->engine2D,
                                   &srcAddress,
                                   1,
                                   &srcStride,
                                   1,
                                   srcTile,
                                   srcFormat,
                                   srcRot,
                                   src->width,
                                   src->height));
        /* Set source rect. */
        gcmONERROR(gco2D_SetSource(context->engine2D, &srcRect));

        /* Update stretch factors. */
        gcmONERROR(
            gco2D_SetStretchFactors(context->engine2D,
                                    hFactor,
                                    vFactor));
        /* Set clip rectangle. */
        gcmONERROR(gco2D_SetClipping(context->engine2D, &dstRect));

        /* StretchBlit. */
        gcmONERROR(
            gco2D_StretchBlit(context->engine2D,
                                  1U,
                                  &dstRect,
                                  0xCC,
                                  0xCC,
                                  dstFormat));
    }
    else if(context->blend_dim)
    {
        gcmONERROR(
            gco2D_LoadSolidBrush(context->engine2D,
                                 srcFormat,
                                 gcvTRUE,
                                 src->clrcolor,
                                 -1));


        gcmONERROR(gco2D_SetClipping(context->engine2D, context->clipping2D ? &subRect : & dstRect));

        /* Do bit blit. */
        gcmONERROR(
            gco2D_Blit(context->engine2D,
                       1U,
                       &dstRect,
                       0xF0,
                       0xF0,//patcopy, see http://technet.microsoft.com/en-us/library/dd145130
                       dstFormat));
    }
    else
    {
        /* Set source address. */
        Stride = src->stride * srcBits / 8;

        physAddress = src->planes[0] - context->baseAddress2D;

        gcmONERROR(
            gco2D_SetGenericSource(context->engine2D,
                                   (gctUINT32_PTR)&physAddress,
                                   1U,
                                   &Stride,
                                   1U,
                                   srcTile,
                                   srcFormat,
                                   srcRot,
                                   src->width,
                                   src->height));


        gcmONERROR(gco2D_SetSource(context->engine2D, &srcRect));

        gcmONERROR(gco2D_SetClipping(context->engine2D, context->clipping2D ? &subRect : & dstRect));

        /* Do bit blit. */
        gcmONERROR(
                 gco2D_Blit(context->engine2D,
				    1U,
				    &dstRect,
				    0xCC,
				    0xCC,
				    dstFormat));
    }

    if (context->separated2D)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }
    context->clipping2D = gcvFALSE;

    return 0;

OnError:

    if (context->separated2D)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}

#if g2dENABLE_VG
static int g2d_blit_vg(void *handle, struct g2d_surface *src, struct g2d_surface *dst)
{
    gcsVG_RECT srcRect, dstRect;
    gceSTATUS  status    = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gctINT  srcWidth,srcHeight,dstWidth,dstHeight;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;
    gcoSURF src_surface = gcvNULL,dst_surface = gcvNULL;
    gceIMAGE_FILTER vg_blit_filter = gcvFILTER_POINT;
    gceVG_BLEND vg_blend_mode = gcvVG_BLEND_SRC;

    context = (struct gcoContext *)handle;

    if(src->rot != G2D_ROTATION_0 || dst->rot != G2D_ROTATION_0)
    {
        g2d_printf("%s: rotation is not supported by hardware vg, should enable G2D_HARDWARE_2D type \n", __FUNCTION__);
        return -1;
    }

    srcWidth = src->right - src->left;
    srcHeight = src->bottom - src->top;
    dstWidth = dst->right - dst->left;
    dstHeight = dst->bottom - dst->top;

    if(srcWidth != dstWidth || srcHeight != dstHeight)
    {
         vg_blit_filter = gcvFILTER_BI_LINEAR;
    }

    if(context->blending == gcvTRUE)
    {
       if((src->blendfunc & 0xf) == G2D_ONE && (dst->blendfunc & 0xf) == G2D_ZERO)
          vg_blend_mode = gcvVG_BLEND_SRC;
       else if((src->blendfunc & 0xf) == G2D_ONE && (dst->blendfunc & 0xf) == G2D_ONE_MINUS_SRC_ALPHA)
          vg_blend_mode = gcvVG_BLEND_SRC_OVER;
       else if((src->blendfunc & 0xf) == G2D_ONE_MINUS_DST_ALPHA && (dst->blendfunc & 0xf) == G2D_ONE)
          vg_blend_mode = gcvVG_BLEND_DST_OVER;
       else if((src->blendfunc & 0xf) == G2D_DST_ALPHA && (dst->blendfunc & 0xf) == G2D_ZERO)
          vg_blend_mode = gcvVG_BLEND_SRC_IN;
       else if((src->blendfunc & 0xf) == G2D_ZERO && (dst->blendfunc & 0xf) == G2D_SRC_ALPHA)
          vg_blend_mode = gcvVG_BLEND_DST_IN;
       else
       {
          g2d_printf("%s: blend mode(0x%x, 0x%x) is not supported by hardware vg, should enable hardware 2d.\n", __FUNCTION__, src->blendfunc, dst->blendfunc);
          return -1;
       }
    }

    /* Set current hardware type to VG. */
    if (context->separatedVG)
    {
       /* Query current hardware type. */
       gcoHAL_GetHardwareType(context->hal, &currentType);

       gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_VG);
    }

    gcmONERROR(
        g2d_construct_vg_surface(handle, &src_surface,
                src));

    gcmONERROR(
        g2d_construct_vg_surface(handle, &dst_surface,
                dst));

    gcmONERROR(
        gcoVG_SetTarget(
                context->engineVG, dst_surface, gcvORIENTATION_BOTTOM_TOP));

    gcmONERROR(
        gcoVG_EnableMask(
                context->engineVG, gcvFALSE));

    gcmONERROR(
        gcoVG_SetImageMode(
                context->engineVG, gcvVG_IMAGE_NORMAL));

    gcmONERROR(
        gcoVG_SetBlendMode(
                context->engineVG, vg_blend_mode));

    gcmONERROR(
        gcoVG_EnableScissor(
                context->engineVG, gcvFALSE));

    if(vg_blit_filter == gcvFILTER_POINT)
    {
        gcmONERROR(
            gcoVG_EnableColorTransform(
                    context->engineVG, gcvFALSE));
    }
    else
    {
        gctFLOAT ColorTransform[8];

        ColorTransform[0] = 0.995f;
        ColorTransform[1] = 0.995f;
        ColorTransform[2] = 0.995f;
        ColorTransform[3] = 1.0f;
        ColorTransform[4] = 0.001f;
        ColorTransform[5] = 0.001f;
        ColorTransform[6] = 0.001f;
        ColorTransform[7] = 0.0f;

        gcmONERROR(
            gcoVG_EnableColorTransform(
                    context->engineVG, gcvTRUE));

        gcmONERROR(
            gcoVG_SetColorTransform(
                    context->engineVG,ColorTransform));
    }

    srcRect.x = src->left;
    srcRect.y = src->top;
    srcRect.width = srcWidth;
    srcRect.height = srcHeight;

    dstRect.x = dst->left;
    dstRect.y = dst->top;
    dstRect.width = dstWidth;
    dstRect.height = dstHeight;

    gcmONERROR(
        gcoVG_DrawImageFilter(
                context->engineVG,
                src_surface,
                &srcRect,
                &dstRect,
                gcvFALSE,
                gcvFALSE,
                vg_blit_filter));

    gcoSURF_Unlock((gcoSURF) src_surface, gcvNULL);
    gcmONERROR(
        gcoSURF_Destroy((gcoSURF) src_surface));

    gcoSURF_Unlock((gcoSURF) dst_surface, gcvNULL);
    gcmONERROR(
        gcoSURF_Destroy((gcoSURF) dst_surface));

    if (context->separatedVG)
    {
        gcmVERIFY_OK(
            gcoHAL_SetHardwareType(gcvNULL, currentType));
    }

    return 0;

OnError:

    if (context->separatedVG)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}
#endif

int g2d_blit(void *handle, struct g2d_surface *src, struct g2d_surface *dst)
{
    struct g2d_surfaceEx srcEx, dstEx;
    srcEx.base = *src;
    dstEx.base = *dst;

    srcEx.tiling = G2D_LINEAR;
    dstEx.tiling = G2D_LINEAR;

    return g2d_blitEx(handle, &srcEx, &dstEx);
}

int g2d_blitEx(void *handle, struct g2d_surfaceEx *srcEx, struct g2d_surfaceEx *dstEx)
{
    gctINT  srcWidth,srcHeight,dstWidth,dstHeight;
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!srcEx || !dstEx)
    {
        g2d_printf("%s: Invalid src and dst parameters!\n", __FUNCTION__);
        return -1;
    }

    struct g2d_surface *src = (struct g2d_surface *)srcEx;
    struct g2d_surface *dst = (struct g2d_surface *)dstEx;

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    if(!context->blend_dim)
    {
        srcWidth = src->right - src->left;
        srcHeight = src->bottom - src->top;

        if(srcWidth <=0 || srcHeight <= 0 || srcWidth > src->width || srcHeight > src->height || src->width > src->stride)
        {
            g2d_printf("%s: Invalid src rect, left %d, top %d, right %d, bottom %d, width %d, height %d, stride %d!\n",
                       __FUNCTION__, src->left, src->top, src->right, src->bottom, src->width, src->height, src->stride);
            return -1;
        }

        if(!src->planes[0])
        {
            g2d_printf("%s: Invalid src planes[0] pointer=0x%x !\n", __FUNCTION__, src->planes[0]);
            return -1;
        }
    }
    else if(context->current_type != G2D_HARDWARE_2D)
    {
        g2d_printf("%s: blend dim is only supported by gpu 2d !\n", __FUNCTION__);
        return -1;
    }

    dstWidth = dst->right - dst->left;
    dstHeight = dst->bottom - dst->top;

    if(dstWidth <=0 || dstHeight <= 0 || dstWidth > dst->width || dstHeight > dst->height || dst->width > dst->stride)
    {
        g2d_printf("%s: Invalid dst rect, left %d, top %d, right %d, bottom %d, width %d, height %d, stride %d!\n",
                   __FUNCTION__, dst->left, dst->top, dst->right, dst->bottom, dst->width, dst->height, dst->stride);
        return -1;
    }

    if(!dst->planes[0])
    {
        g2d_printf("%s: Invalid dst planes[0] pointer=0x%x !\n", __FUNCTION__, dst->planes[0]);
        return -1;
    }

#if g2dENABLE_VG
    if(context->current_type == G2D_HARDWARE_VG)
    {
        if(((src->planes[0] - context->baseAddress2D) & 0x80000000)
          || ((dst->planes[0] - context->baseAddress2D) & 0x80000000))
        {
            //roll back to 2D when the address is mapped by 3D MMU
            g2d_make_current(handle, G2D_HARDWARE_2D);
        }
    }
#endif

    switch(context->current_type)
    {
    case G2D_HARDWARE_2D:
    default:
        return g2d_blit_2d(handle, srcEx, dstEx);
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
        return g2d_blit_vg(handle, src, dst);
        break;
#endif
    }

    return 0;
}

/*
* g2d_multi_blit
*/
static int g2d_multi_blit_2d(void *handle, struct g2d_surface_pair *sp[], int layers)
{
    gctINT i;
    gctUINT32 SourceMask[9] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF};
    gctUINT32 physAddress;
    gctUINT32 srcBits, dstBits;
    gceSURF_ROTATION srcRot, dstRot;
    gctUINT32 Stride,uStride = 0,vStride = 0;
    gctUINT32 srcStride[3];
    gctUINT32 srcStrideNum = 1;
    gctUINT32 srcPlanes[3];
    gcsRECT srcRect, subRect;
    gceSTATUS  status    = gcvSTATUS_OK;
    struct gcoContext * context = gcvNULL;
    gceSURF_FORMAT srcFormat = gcvSURF_UNKNOWN;
    gceSURF_FORMAT dstFormat = gcvSURF_UNKNOWN;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;
    //printf("context->current_type : %d\n", context->current_type);

    struct g2d_surface *src = gcvNULL;
    struct g2d_surface *dst = gcvNULL;

    /*-----------------------------*/
    /* init data for dst. */
    dst = &(sp[0]->d);

    switch(dst->format)
    {
    case G2D_RGB565:
        dstBits = 16;
        dstFormat = gcvSURF_R5G6B5;
        break;
    case G2D_RGBA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8B8G8R8;
        break;
    case G2D_RGBX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8B8G8R8;
        break;
    case G2D_BGRA8888:
        dstBits = 32;
        dstFormat = gcvSURF_A8R8G8B8;
        break;
    case G2D_BGRX8888:
        dstBits = 32;
        dstFormat = gcvSURF_X8R8G8B8;
        break;
    case G2D_BGR565:
        dstBits = 16;
        dstFormat = gcvSURF_B5G6R5;
        break;
    case G2D_ARGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8A8;
        break;
    case G2D_ABGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8A8;
        break;
    case G2D_XRGB8888:
        dstBits = 32;
        dstFormat = gcvSURF_B8G8R8X8;
        break;
    case G2D_XBGR8888:
        dstBits = 32;
        dstFormat = gcvSURF_R8G8B8X8;
        break;
    case G2D_YUYV:
        dstBits = 16;
        dstFormat = gcvSURF_YUY2;
        break;
    default:
        g2d_printf("%s: Invalid dst format %d!\n", __FUNCTION__, dst->format);
        return -1;
        break;
    }

    switch(dst->rot)
    {
    case G2D_ROTATION_0:
    default:
        /* for multi source blit, only set dstRot one time. */
        /* so set dstRot a default data */
        dstRot = gcvSURF_0_DEGREE;
        break;
    }

    /* for gcc warning "-Wunused-but-set-variable" */
    /*gcsRECT dstRect8[8];
    for(i = 0; i < layers; i++)
    {
        dstRect8[i].left   = sp[i]->d.left;
        dstRect8[i].top    = sp[i]->d.top;
        dstRect8[i].right  = sp[i]->d.right;
        dstRect8[i].bottom = sp[i]->d.bottom;
    }*/

    /* set alpha blending for dst. */

    /* Alpha blending mode. */
    gceSURF_PIXEL_ALPHA_MODE         dstAlphaMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;

    /* Perpixel alpha premultiply. */
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstPremultDstAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;

    /* Global alpha blending mode. */
    gceSURF_GLOBAL_ALPHA_MODE        dstGlobalAlphaMode = gcvSURF_GLOBAL_ALPHA_OFF;

    /* Global alpha premultiply. */
    //gce2D_GLOBAL_COLOR_MULTIPLY_MODE dstPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;

    /* Dest global alpha demultiply. */
    gce2D_PIXEL_COLOR_MULTIPLY_MODE  dstDemultDstAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;

    /* Blend factor. */
    gceSURF_BLEND_FACTOR_MODE        dstFactorMode = gcvSURF_BLEND_ONE;

    /* Global alpha value. */
    gctUINT32                        dstGlobalAlpha = 0xFF000000;

    /* Get perpixelAlpha enabling state. */
    gctBOOL perpixelAlpha = _HasAlpha(dstFormat);

    if(context->blending)
    {
        switch(dst->blendfunc & 0xf)
        {
        case G2D_ZERO://Cd = Cd * 0
            dstFactorMode        = gcvSURF_BLEND_ZERO;
            break;
        case G2D_ONE://Cd = Cd * 1
            dstFactorMode        = gcvSURF_BLEND_ONE;
            if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
            {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
            }
            break;
        case G2D_SRC_ALPHA://Cd = Cd * As
            dstFactorMode        = gcvSURF_BLEND_STRAIGHT;
            if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
            {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
            }
            break;
        case G2D_ONE_MINUS_SRC_ALPHA://Cd = Cd * (1 - As)
            dstFactorMode        = gcvSURF_BLEND_INVERSED;
            if(perpixelAlpha && (dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
            {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
            }
            break;
        case G2D_DST_ALPHA://Cd = Cd * Ad * 1
            dstFactorMode        = gcvSURF_BLEND_ONE;
            if(perpixelAlpha)
            {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                if(dst->blendfunc & G2D_PRE_MULTIPLIED_ALPHA)
                {
                    g2d_printf("%s: G2D_PRE_MULTIPLIED_ALPHA should not be set with G2D_DST_ALPHA, ignored\n",
                        __FUNCTION__);
                }
            }
            break;
        case G2D_ONE_MINUS_DST_ALPHA://Cd = Cd * (1 - Ad) * 1
            dstFactorMode        = gcvSURF_BLEND_ONE;
            if(perpixelAlpha)
            {
                dstPremultDstAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                dstAlphaMode         = gcvSURF_PIXEL_ALPHA_INVERSED;
            }
            break;
        default:
            dstFactorMode        = gcvSURF_BLEND_ONE;
            break;
        }
    
        if(dst->blendfunc & G2D_DEMULTIPLY_OUT_ALPHA)
        {
            dstDemultDstAlpha = gcv2D_COLOR_MULTIPLY_ENABLE;
        }
    }

    
    /**/
    /* Set current hardware type to 2D. */
    if (context->separated2D)
    {
        /* Query current hardware type. */
        gcmONERROR(
            gcoHAL_GetHardwareType(context->hal, &currentType));
    
        gcmONERROR(
            gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D));
    }


    /*------------------------------------*/
    /* 1. Setup Target */
    /* 2. Setup Source */
    /* 3. Call multisource blit */
    /*-----------------------------------*/

    /* Setup Target */
    Stride = dst->stride * dstBits / 8;

    physAddress = dst->planes[0] - context->baseAddress2D;

    gcmONERROR(
        gco2D_SetGenericTarget(context->engine2D,
                                (gctUINT32_PTR)&physAddress, 1U,
                                &Stride, 1U,
                                gcvLINEAR,
                                dstFormat,
                                dstRot,
                                dst->width,
                                dst->height));

    /* Setup source */
    for(i = 0; i < layers; i++)
    {
        /* Setup source Index. */
        gcmONERROR(
            gco2D_SetCurrentSourceIndex(context->engine2D, i));

        /* Setup source physAddress */
        src = &(sp[i]->s);

        switch(src->format)
        {
        case G2D_RGB565:
            srcBits = 16;
            srcFormat = gcvSURF_R5G6B5;
            break;
        case G2D_RGBA8888:
            srcBits = 32;
            srcFormat = gcvSURF_A8B8G8R8;
            break;
        case G2D_RGBX8888:
            srcBits = 32;
            srcFormat = gcvSURF_X8B8G8R8;
            break;
        case G2D_BGRA8888:
            srcBits = 32;
            srcFormat = gcvSURF_A8R8G8B8;
            break;
        case G2D_BGRX8888:
            srcBits = 32;
            srcFormat = gcvSURF_X8R8G8B8;
            break;
        case G2D_BGR565:
            srcBits = 16;
            srcFormat = gcvSURF_B5G6R5;
            break;
        case G2D_ARGB8888:
            srcBits = 32;
            srcFormat = gcvSURF_B8G8R8A8;
            break;
        case G2D_ABGR8888:
            srcBits = 32;
            srcFormat = gcvSURF_R8G8B8A8;
            break;
        case G2D_XRGB8888:
            srcBits = 32;
            srcFormat = gcvSURF_B8G8R8X8;
            break;
        case G2D_XBGR8888:
            srcBits = 32;
            srcFormat = gcvSURF_R8G8B8X8;
            break;
        case G2D_YUYV:
            srcBits = 16;
            srcFormat = gcvSURF_YUY2;
            break;
        case G2D_YVYU:
            srcBits = 16;
            srcFormat = gcvSURF_YVYU;
            break;
        case G2D_UYVY:
            srcBits = 16;
            srcFormat = gcvSURF_UYVY;
            break;
        case G2D_VYUY:
            srcBits = 16;
            srcFormat = gcvSURF_VYUY;
            break;
        case G2D_NV16:
            srcBits = 8;
            srcFormat = gcvSURF_NV16;
            uStride = src->stride;
            srcStrideNum = 2;
            break;
        case G2D_I420:
            srcBits = 8;
            srcFormat = gcvSURF_I420;
            uStride = vStride = src->stride / 2;
            srcStrideNum = 3;
            break;
        default:
            g2d_printf("%s: Invalid src format %d!\n", __FUNCTION__, src->format);
            return -1;
            break;
        }

        switch(src->rot)
        {
        case G2D_ROTATION_0:
        default:
            srcRect.left = src->left;
            srcRect.top = src->top;
            srcRect.right = src->right;
            srcRect.bottom = src->bottom;
        
            srcRot = gcvSURF_0_DEGREE;
            break;
        case G2D_ROTATION_90:
            srcRect.left = src->top;
            srcRect.top = src->width - src->right;
            srcRect.right = src->bottom;
            srcRect.bottom = src->width - src->left;

            srcRot = gcvSURF_90_DEGREE;
            break;
        case G2D_ROTATION_180:
            srcRect.left = src->width - src->right;
            srcRect.top = src->height - src->bottom;
            srcRect.right = src->width - src->left;
            srcRect.bottom = src->height - src->top;

            srcRot = gcvSURF_180_DEGREE;
            break;
        case G2D_ROTATION_270:
            srcRect.left = src->height - src->bottom;
            srcRect.top = src->left;
            srcRect.right = src->height - src->top;
            srcRect.bottom = src->right;

            srcRot = gcvSURF_270_DEGREE;
            break;
        case G2D_FLIP_H:
            srcRect.left = src->width - src->right;
            srcRect.top = src->top;
            srcRect.right = src->width - src->left;
            srcRect.bottom = src->bottom;

            srcRot = gcvSURF_FLIP_X;
            break;
        case G2D_FLIP_V:
            srcRect.left = src->left;
            srcRect.top = src->height - src->bottom;
            srcRect.right = src->right;
            srcRect.bottom = src->height - src->top;

            srcRot = gcvSURF_FLIP_Y;
            break;
        }

        /* set alpha blending for each source. */
        if(context->blending)
        {
            /* Alpha blending mode. */
            gceSURF_PIXEL_ALPHA_MODE         srcAlphaMode = gcvSURF_PIXEL_ALPHA_STRAIGHT;

            /* Perpixel alpha premultiply. */
            gce2D_PIXEL_COLOR_MULTIPLY_MODE  srcPremultSrcAlpha = gcv2D_COLOR_MULTIPLY_DISABLE;

            /* Global alpha blending mode. */
            gceSURF_GLOBAL_ALPHA_MODE        srcGlobalAlphaMode = gcvSURF_GLOBAL_ALPHA_OFF;

            /* Global alpha premultiply. */
            gce2D_GLOBAL_COLOR_MULTIPLY_MODE srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE;

            /* Blend factor. */
            gceSURF_BLEND_FACTOR_MODE        srcFactorMode = gcvSURF_BLEND_ONE;

            /* Global alpha value. */
            gctUINT32                        srcGlobalAlpha = 0xFF000000;

            /* Get perpixelAlpha enabling state. */
            gctBOOL perpixelAlpha = _HasAlpha(srcFormat);

            switch(src->blendfunc & 0xf)
            {
            case G2D_ZERO://Cs = Cs * 0
                srcFactorMode        = gcvSURF_BLEND_ZERO;
                break;
            case G2D_ONE: //Cs = Cs * 1
                srcFactorMode        = gcvSURF_BLEND_ONE;
                if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
                {
                    srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                    srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                }
                break;
            case G2D_SRC_ALPHA://Cs = Cs * As * 1
                srcFactorMode        = gcvSURF_BLEND_ONE;
                if(perpixelAlpha)
                {
                    srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                    srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                    if(src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA)
                    {
                        g2d_printf("%s: G2D_PRE_MULTIPLIED_ALPHA should not be set with G2D_SRC_ALPHA, ignored\n",
                            __FUNCTION__);
                    }
                }
                break;
            case G2D_ONE_MINUS_SRC_ALPHA://Cs = Cs * (1 - As) * 1
                srcFactorMode        = gcvSURF_BLEND_ONE;
                if(perpixelAlpha)
                {
                    srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                    srcAlphaMode         = gcvSURF_PIXEL_ALPHA_INVERSED;
                }
                break;
            case G2D_DST_ALPHA://Cs = Cs * Ad
                srcFactorMode        = gcvSURF_BLEND_STRAIGHT;
                if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
                {
                    srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                    srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                }
                break;
            case G2D_ONE_MINUS_DST_ALPHA://Cs = Cs * (1 - Ad)
                srcFactorMode        = gcvSURF_BLEND_INVERSED;
                if(perpixelAlpha && (src->blendfunc & G2D_PRE_MULTIPLIED_ALPHA))
                {
                    srcPremultSrcAlpha   = gcv2D_COLOR_MULTIPLY_ENABLE;
                    srcAlphaMode         = gcvSURF_PIXEL_ALPHA_STRAIGHT;
                }
                break;
            default:
                srcFactorMode        = gcvSURF_BLEND_ONE;
                break;
            }
            
            if(context->global_alpha_enable && src->global_alpha < 0xFF)
            {
                srcPremultGlobalMode = gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA;
                srcGlobalAlpha       = src->global_alpha << 24;

                if(perpixelAlpha)
                {
                    srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_SCALE;
                }
                else
                {
                    srcGlobalAlphaMode   = gcvSURF_GLOBAL_ALPHA_ON;
                }
            }

            gcmONERROR(
                gco2D_EnableAlphaBlendAdvanced(context->engine2D,
                                                srcAlphaMode,
                                                dstAlphaMode,
                                                srcGlobalAlphaMode,
                                                dstGlobalAlphaMode,
                                                srcFactorMode,
                                                dstFactorMode));

            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(context->engine2D,
                                                    srcPremultSrcAlpha,
                                                    dstPremultDstAlpha,
                                                    srcPremultGlobalMode,
                                                    dstDemultDstAlpha));
            gcmONERROR(
                gco2D_SetSourceGlobalColorAdvanced(context->engine2D,
                                                    srcGlobalAlpha));
        
            gcmONERROR(
                gco2D_SetTargetGlobalColorAdvanced(context->engine2D,
                                                    dstGlobalAlpha));
        }
        else
        {
            /* Disable alhpa blending. */
            gcmONERROR(
                gco2D_DisableAlphaBlend(context->engine2D));
        
            /* Disable premultiply. */
            gcmONERROR(
                gco2D_SetPixelMultiplyModeAdvanced(context->engine2D,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE,
                                                    gcv2D_COLOR_MULTIPLY_DISABLE));
        }

        
        /**/
        /*----*/
        //Stride = src->stride * srcBits / 8;
        //physAddress = src->planes[0] - context->baseAddress2D;
        switch(srcStrideNum)
        {
        case 3:
            srcPlanes[2] = src->planes[2] - context->baseAddress2D;
            srcStride[2] = vStride * srcBits / 8;
        case 2:
            srcPlanes[1] = src->planes[1] - context->baseAddress2D;
            srcStride[1] = uStride * srcBits / 8;
        case 1:
            srcPlanes[0] = src->planes[0] - context->baseAddress2D;
            srcStride[0] = src->stride * srcBits / 8;
            break;
        default:
            g2d_printf("%s, wrong srcStrideNum\n", __FUNCTION__);
            break;
        }


        gcmONERROR(
            gco2D_SetGenericSource(context->engine2D,
                                    (gctUINT32_PTR)&srcPlanes[0], srcStrideNum,
                                    &srcStride[0], srcStrideNum,
                                    gcvLINEAR,
                                    srcFormat,
                                    srcRot,
                                    src->width,
                                    src->height));
        
        /* Setup source Rect */
        gcmONERROR(gco2D_SetSource(context->engine2D, &srcRect));

        //gcmONERROR(gco2D_SetTargetRect(context->engine2D, &srcRect));

        /* Setup Clipping Rect */
        subRect.left = 0;
        subRect.top  = 0;
        subRect.right = dst->width;
        subRect.bottom = dst->height;
        gcmONERROR(gco2D_SetClipping(context->engine2D, &subRect));

        gcmONERROR(gco2D_SetROP(context->engine2D, 0xCC, 0xCC));
    }

    /* Do multissource blit. */
    gcmONERROR(gco2D_MultiSourceBlit(context->engine2D, SourceMask[layers], &subRect, 1U));

    //gcmONERROR(gco2D_MultiSourceBlit(context->engine2D, SourceMask[layers], gcvNULL, 0));

    /*---------------------------------------*/

    if (context->separated2D)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }
    context->clipping2D = gcvFALSE;

    return 0;

OnError:
    if (context->separated2D)
    {
        gcoHAL_SetHardwareType(context->hal, currentType);
    }

    g2d_printf("%s: fail with status %d\n", __FUNCTION__, status);
    return -1;
}

int g2d_multi_blit(void *handle, struct g2d_surface_pair *sp[], int layers)
{
    int i = 0;
    gctINT  srcWidth,srcHeight,dstWidth,dstHeight;
    struct g2d_surface *src = gcvNULL;
    struct g2d_surface *dst = gcvNULL;
    struct gcoContext * context = (struct gcoContext *)handle;

    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(!sp || layers < 1 || layers > 8)
    {
        g2d_printf("%s: Invalid src and dst parameters!\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    if(!context->blend_dim)
    {
        for(i = 0; i < layers; i++)
        {
            src = &(sp[i]->s);
            srcWidth = src->right - src->left;
            srcHeight = src->bottom - src->top;

            if(srcWidth <=0 || srcHeight <= 0 || srcWidth > src->width || srcHeight > src->height || src->width > src->stride)
            {
                g2d_printf("%s: Invalid src rect, left %d, top %d, right %d, bottom %d, width %d, height %d, stride %d!\n",
                               __FUNCTION__, src->left, src->top, src->right, src->bottom, src->width, src->height, src->stride);
                return -1;
            }
        
            if(!src->planes[0])
            {
                g2d_printf("%s: Invalid src planes[0] pointer=0x%x !\n", __FUNCTION__, src->planes[0]);
                return -1;
            }
        }
    }
    else if(context->current_type != G2D_HARDWARE_2D)
    {
        g2d_printf("%s: blend dim is only supported by gpu 2d !\n", __FUNCTION__);
        return -1;
    }

    for(i = 0; i < layers; i++)
    {
        dst = &(sp[i]->d);
        dstWidth = dst->right - dst->left;
        dstHeight = dst->bottom - dst->top;
    
        if(dstWidth <=0 || dstHeight <= 0 || dstWidth > dst->width || dstHeight > dst->height || dst->width > dst->stride)
        {
            g2d_printf("%s: Invalid dst rect, left %d, top %d, right %d, bottom %d, width %d, height %d, stride %d!\n",
                        __FUNCTION__, dst->left, dst->top, dst->right, dst->bottom, dst->width, dst->height, dst->stride);
            return -1;
        }
        
        if(!dst->planes[0])
        {
            g2d_printf("%s: Invalid dst planes[0] pointer=0x%x !\n", __FUNCTION__, dst->planes[0]);
            return -1;
        }
    }

#if g2dENABLE_VG
    if(context->current_type == G2D_HARDWARE_VG)
    {
        /* TODO roll back to 2D when the address is mapped by 3D MMU*/
        src = &(sp[0]->s);
        dst = &(sp[0]->d);
        
        if(((src->planes[0] - context->baseAddress2D) & 0x80000000)
                || ((dst->planes[0] - context->baseAddress2D) & 0x80000000))
        {
            //roll back to 2D when the address is mapped by 3D MMU
            g2d_make_current(handle, G2D_HARDWARE_2D);
        }
    }
#endif

    switch(context->current_type)
    {
    case G2D_HARDWARE_2D:
    default:
        return g2d_multi_blit_2d(handle, sp, layers);
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
        //return g2d_multi_blit_vg(handle, sp, layers);
        break;
#endif
    }

    return 0;
}

/**/
#define GC320_READ_STRIDE   512 //gc320 use tile mode instead of linear mode to read data
#define HW_COPY_THRESHOLD (32*32) //the experiential value from GB copybit sw/hw switch
int g2d_copy(void *handle, struct g2d_buf *d, struct g2d_buf* s, int size)
{
    int blit_size;
    int ret,width,height;
    struct g2d_surface src,dst;
    int dither=0, blending=0;
  //  struct g2d_buf_context *bufctx = NULL;

#ifdef GC320_READ_STRIDE
    if(size >= (int)(GC320_READ_STRIDE * PAGE_SIZE * 2))
    {
        width = GC320_READ_STRIDE;
    }
    else
    {
        width = GC320_READ_STRIDE >> 3;
    }

#else
    if(size >= (HW_COPY_THRESHOLD << 2)* PAGE_SIZE)
    {
        width = HW_COPY_THRESHOLD << 2;
    }
    else if(size >= HW_COPY_THRESHOLD * PAGE_SIZE)
    {
        width = HW_COPY_THRESHOLD;
    }
    else
    {
        width = HW_COPY_THRESHOLD >> 2;//RGBA pixel size is 4 bytes
    }
#endif

    height = size / (width * 4);
    if(height >= 16384) height = 16384;//gpu 2d limit for height

    blit_size = height * width * 4;

    if(!blit_size)
    {
         memcpy((void*)d->buf_vaddr, (void*)s->buf_vaddr, size);
         return 0;
    }

    src.planes[0] = s->buf_paddr;
    src.planes[1] = 0;
    src.planes[2] = 0;

    src.left = 0;
    src.top = 0;
    src.right = width;
    src.bottom = height;
    src.stride = width;
    src.width  = width;
    src.height = height;
    src.format = G2D_RGBA8888;
    src.rot    = G2D_ROTATION_0;

    dst.planes[0] = d->buf_paddr;
    dst.planes[1] = 0;
    dst.planes[2] = 0;

    dst.left = 0;
    dst.top = 0;
    dst.right = width;
    dst.bottom = height;
    dst.stride = width;
    dst.width  = width;
    dst.height = height;
    dst.format = G2D_RGBA8888;
    dst.rot    = G2D_ROTATION_0;

    g2d_query_cap(handle, G2D_DITHER, &dither);
    if(dither) g2d_disable(handle, G2D_DITHER);

    g2d_query_cap(handle, G2D_BLEND, &blending);
    if(blending) g2d_disable(handle, G2D_BLEND);

    ret = g2d_blit(handle, &src, &dst);

    if(dither) g2d_enable(handle, G2D_DITHER);
    if(blending) g2d_enable(handle, G2D_BLEND);

    if(ret) return ret;

    if(blit_size == size) return 0;

    if(!s->buf_vaddr || !d->buf_vaddr) return -1;

    if((size - blit_size) >= width * 4)
    {
        struct g2d_buf subd,subs;

        subd.buf_handle = d->buf_handle;
        subs.buf_size = d->buf_size - blit_size;
        subd.buf_paddr = d->buf_paddr + blit_size;
        subd.buf_vaddr = (void*)(gcmPTR2INT(d->buf_vaddr) + blit_size);

        subs.buf_handle = s->buf_handle;
        subs.buf_size = s->buf_size - blit_size;;
        subs.buf_paddr = s->buf_paddr + blit_size;
        subs.buf_vaddr = (void*)(gcmPTR2INT(s->buf_vaddr) + blit_size);

        return g2d_copy(handle, &subd, &subs, size - blit_size);
    }

    memcpy((void*)(gcmPTR2INT(d->buf_vaddr) + blit_size), (void*)(gcmPTR2INT(s->buf_vaddr) + blit_size), size - blit_size);

#if 0 //disable below code since cache flush may require page-alignment, the better method is to invalidate destination buffer before copy
    //flush cache for the cacheable memory
    bufctx = (struct g2d_buf_context *)d->buf_handle;
    if(bufctx->cacheable)
    {
        struct g2d_buf temp;

        temp.buf_handle = d->buf_handle;
        temp.buf_vaddr  = (void*)(((int)d->buf_vaddr) + blit_size);
        temp.buf_paddr  = d->buf_paddr + blit_size;
        temp.buf_size   = d->buf_size - blit_size;

        g2d_cache_op(&temp, G2D_CACHE_FLUSH);
    }
#endif
    return 0;
}

int g2d_flush(void *handle)
{
    struct gcoContext * context = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    /* Query current hardware type. */
    gcoHAL_GetHardwareType(context->hal, &currentType);

    switch(context->current_type)
    {
    case G2D_HARDWARE_2D:
    default:
        /* Set current hardware type to 2D. */
        if (context->separated2D)
        {
           gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
        }
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
         /* Set current hardware type to VG. */
        if (context->separatedVG)
        {
           gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_VG);
        }

        //gcoHAL_Flush is only used to flush VG pipeline
        gcmVERIFY_OK( gcoHAL_Flush(context->hal));

        break;
#endif
    }

    gcmVERIFY_OK(
          gcoHAL_Commit(context->hal, gcvFALSE));

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

int g2d_finish(void *handle)
{
    struct gcoContext * context = gcvNULL;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    /* Query current hardware type. */
    gcoHAL_GetHardwareType(context->hal, &currentType);

    switch(context->current_type)
    {
    case G2D_HARDWARE_2D:
    default:
        /* Set current hardware type to 2D. */
        if (context->separated2D)
        {
           gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_2D);
        }
        break;
#if g2dENABLE_VG
    case G2D_HARDWARE_VG:
         /* Set current hardware type to VG. */
        if (context->separatedVG)
        {
           gcoHAL_SetHardwareType(context->hal, gcvHARDWARE_VG);
        }

        //gcoHAL_Flush is only used to flush VG pipeline
        gcmVERIFY_OK( gcoHAL_Flush(context->hal));

        break;
#endif
    }

    gcmVERIFY_OK(
          gcoHAL_Commit(context->hal, gcvTRUE));

    gcmVERIFY_OK(
          gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;
}

int g2d_close(void *handle)
{
    struct gcoContext * context = gcvNULL;

    context = (struct gcoContext *)handle;
    if(!context)
    {
        g2d_printf("%s: Invalid handle !\n", __FUNCTION__);
        return -1;
    }

    if(context->threadID != gcoOS_GetCurrentThreadID())
    {
        g2d_printf("%s: invalid g2d thread context !\n", __FUNCTION__);
        return -1;
    }

    /* Destroy hal object. */
    gcmVERIFY_OK(
        gcoHAL_Destroy(context->hal));

    /* Destroy os object. */
    gcmVERIFY_OK(
        gcoOS_Destroy(context->os));

    /* TODO: Free allocated memory. */

    /* Clean context. */
    free(context);

    return 0;
}

int g2d_cache_op(struct g2d_buf *buf, enum g2d_cache_mode op)
{
    gctSIZE_T Bytes = 0;
    gctPOINTER Logical = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gcuVIDMEM_NODE_PTR Node = gcvNULL;
    struct g2d_buf_context *bufctx = NULL;

    if(!buf || !buf->buf_handle)
    {
        g2d_printf("%s: invalid buffer !\n", __FUNCTION__);
        return -1;
    }

    bufctx = (struct g2d_buf_context *)buf->buf_handle;

    Bytes = (gctSIZE_T)buf->buf_size;
    Logical = (gctPOINTER)buf->buf_vaddr;
    Node = (gcuVIDMEM_NODE_PTR)bufctx->handle;

    if(!Bytes || !Logical || !Node)
    {
        g2d_printf("%s: invalid buffer data!\n", __FUNCTION__);
        return -1;
    }

    switch(op)
    {
    case G2D_CACHE_CLEAN:
       gcmONERROR(gcoOS_CacheClean(gcvNULL, gcmPTR_TO_UINT64(Node), Logical, Bytes));
       break;
    case G2D_CACHE_FLUSH:
       gcmONERROR(gcoOS_CacheFlush(gcvNULL, gcmPTR_TO_UINT64(Node), Logical, Bytes));
       break;
    case G2D_CACHE_INVALIDATE:
       gcmONERROR(gcoOS_CacheInvalidate(gcvNULL, gcmPTR_TO_UINT64(Node), Logical, Bytes));
       break;
    default:
       break;
    }

    /* Success. */
    return 0;

OnError:

    g2d_printf("%s: fail with status %d", __FUNCTION__, status);

    /* Fail. */
    return -1;
}

struct g2d_buf * g2d_alloc(int size, int cacheable)
{
    gctSIZE_T Bytes = size;
    gctUINT32 physAddr = 0;
    struct g2d_buf *buf = NULL;
    gctPOINTER virtAddr = gcvNULL;
    gctPOINTER bufHandle= gcvNULL;
    gceSTATUS Status = gcvSTATUS_OK;
    struct g2d_buf_context *bufctx = NULL;

    Status = gcoOS_AllocateVideoMemory(gcvNULL, gcvTRUE, cacheable ? gcvTRUE : gcvFALSE, &Bytes, &physAddr, &virtAddr, &bufHandle);
    if(Status != gcvSTATUS_OK)
    {
        g2d_printf("%s: alloc memory fail with size %d!\n", __FUNCTION__, size);
        return  NULL;
    }

    buf = (struct g2d_buf *)malloc(sizeof(struct g2d_buf));
    if(!buf)
    {
        g2d_printf("%s: malloc g2d_buf fail !\n", __FUNCTION__);
        return NULL;
    }

    bufctx = (struct g2d_buf_context *)malloc(sizeof(struct g2d_buf_context));
    if(!bufctx)
    {
        g2d_printf("%s: malloc g2d_buf_context fail !\n", __FUNCTION__);
        free(buf); buf = NULL;
        return NULL;
    }

    bufctx->handle = (void*)bufHandle;
    bufctx->cacheable = cacheable;
    bufctx->physical = (int)physAddr;

    buf->buf_handle = (void*)bufctx;
    buf->buf_vaddr = (void *)virtAddr;
    buf->buf_paddr = (int)physAddr;
    buf->buf_size = (int)Bytes;

    return buf;
}

int g2d_free(struct g2d_buf *buf)
{
    gceSTATUS Status = gcvSTATUS_OK;
    struct g2d_buf_context *bufctx = NULL;

    if(!buf || !buf->buf_handle)
    {
        g2d_printf("%s: invalid g2d buf handle !\n", __FUNCTION__);
        return -1;
    }

    bufctx = (struct g2d_buf_context *)buf->buf_handle;

    Status = gcoOS_FreeVideoMemory(gcvNULL, (gctPOINTER)bufctx->handle);
    if(Status == gcvSTATUS_OK)
    {
        free(buf);
        free(bufctx);
        return 0;
    }

    return -1;
}

int g2d_buf_export_fd(struct g2d_buf *buf)
{
    return G2D_STATUS_NOT_SUPPORTED;
}

#if defined(LINUX)
#if !defined(ANDROID)
/* ------------------------
 * g2d_ion
 * ------------------------
 */
#include <linux/ion.h>
#include <linux/version.h>
#include <linux/dma-buf.h>

const char *dev_ion = "/dev/ion";

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 34)
static int
g2d_ion_phys_dma(int fd, int dmafd, gctUINT32 *paddr, size_t *size)
{
    int ret;
    struct ion_phys_dma_data data = {
        .phys   = 0,
        .size   = 0,
        .dmafd  = dmafd,
    };

    struct ion_custom_data custom = {
        .cmd = ION_IOC_PHYS_DMA,
        .arg = (uintptr_t)&data,
    };

    ret = ioctl(fd, ION_IOC_CUSTOM, &custom);
    if(ret < 0)
        return ret;

    *paddr = data.phys;
    *size  = data.size;

    return 0;
}

struct g2d_buf *g2d_buf_from_virt_addr(void *vaddr, int size)
{
    int ret;
    int fd;

    if(!vaddr)
        return NULL;

    fd = open(dev_ion, O_RDWR);
    if(fd < 0)
    {
        g2d_printf("open %s failed!\n", dev_ion);
        return NULL;
    }

    struct ion_phys_virt_data data = {
        .virt   = (unsigned long)vaddr,
        .phys   = 0,
        .size   = size,
    };

    struct ion_custom_data custom = {
        .cmd = ION_IOC_PHYS_VIRT,
        .arg = (uintptr_t)&data,
    };

    ret = ioctl(fd, ION_IOC_CUSTOM, &custom);
    close(fd);
    if(ret < 0)
        return NULL;

    struct g2d_buf *buf = (struct g2d_buf *)malloc(sizeof(struct g2d_buf));
    if(!buf)
    {
        g2d_printf("%s: malloc g2d_buf fail !\n", __FUNCTION__);
        return NULL;
    }

    buf->buf_paddr = data.phys;
    buf->buf_vaddr = vaddr;
    buf->buf_size = data.size;

    return buf;
}
#else
static int
g2d_ion_phys_dma(int fd, int dmafd, gctUINT32 *paddr, size_t *size)
{
    int ret = 0;
    struct dma_buf_phys query;

    ret = ioctl(dmafd, DMA_BUF_IOCTL_PHYS, &query);
    *paddr = query.phys;

    return ret;
}

struct g2d_buf *g2d_buf_from_virt_addr(void *vaddr, int size)
{
    g2d_printf("%s: g2d_buf_alloc_from_virt_addr not supported !\n", __FUNCTION__);
    return NULL;
}
#endif

struct g2d_buf * g2d_buf_from_fd(int fd)
{
    gctUINT32 physAddr = 0;
    size_t size = 0;
    struct g2d_buf *buf = NULL;

    int ion_fd, ret;
    ion_fd = open(dev_ion, O_RDWR);
    if(ion_fd < 0)
    {
        g2d_printf("open %s failed!\n", dev_ion);
        return NULL;
    }

    ret = g2d_ion_phys_dma(ion_fd, fd, &physAddr, &size);
    close(ion_fd);
    if(ret < 0)
        return NULL;

    /* Construct g2d_buf */
    buf = (struct g2d_buf *)malloc(sizeof(struct g2d_buf));
    if(!buf)
    {
        g2d_printf("%s: Invalid g2d_buf !\n", __FUNCTION__);
        return NULL;
    }

    buf->buf_paddr = (int)physAddr;
    buf->buf_size  = size;

    return buf;
}

#endif
#endif
