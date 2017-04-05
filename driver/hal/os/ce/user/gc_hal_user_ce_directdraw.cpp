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


#if gcdENABLE_3D || gcdENABLE_VG
#include "gc_hal_user_ce.c"

#ifdef USE_CE_DIRECTDRAW

#include <ddraw.h>

typedef struct __DDBuffer
{
    gcoSURF                     surface;
    gctPOINTER                  lpSurface;
    gctINT                      stride;
    gctINT                      width;
    gctINT                      height;
    gceSURF_FORMAT              format;
    gcsPOINT                    origin;
} DDBuffer, *DDBuffer_PTR;

typedef struct __halLocalDDSurf
{
    struct __halLocalDDSurf     *next;
    gctPOINTER                  ddSurface;
    gctUINT32                   current;
    gctPOINTER                  semaphore;
    gctUINT32                   numberOfBuffer;
    DDBuffer                    buffers[1];
} halLocalDDSurf, *halLocalDDSurf_PTR;

typedef struct __halLocalDisplay
{
    IDirectDraw *pDD;
    IDirectDrawSurface *pDDS;
    gctPOINTER  logical;
    gctUINT     physical;
    gctINT      stride;
    gctINT      width;
    gctINT      height;
    RECT        rect;
    halLocalDDSurf_PTR localBackSurf;
} halLocalDisplay;

static gctBOOL
GetDDSFormat(LPDDPIXELFORMAT DdpFormat, gceSURF_FORMAT *Format, gctUINT32 *Bpp)
{
    gceSURF_FORMAT format;

    if (!(DdpFormat->dwFlags & DDPF_RGB))
    {
        return gcvFALSE;
    }

    switch (DdpFormat->dwRGBBitCount)
    {
    case 16:
        format = gcvSURF_R5G6B5;
        break;
    case 32:
        format = gcvSURF_X8R8G8B8;
        break;
    default:
        return gcvFALSE;
    }

    if (Format)
    {
        *Format = format;
    }

    if (Bpp)
    {
        *Bpp = DdpFormat->dwRGBBitCount;
    }

    return gcvTRUE;
}

typedef struct __EnumBackBuffer {
    gctUINT      num;
    DDBuffer_PTR buffers;
} EnumBackBuffer;

static HRESULT WINAPI
EnumGetBackBufferCallback(LPDIRECTDRAWSURFACE lpDDSurface,
                     LPDDSURFACEDESC lpDDSurfaceDesc,
                     LPVOID lpContext)
{
    EnumBackBuffer *pEbb = (EnumBackBuffer*)lpContext;

    while (pEbb)
    {
        DDSURFACEDESC dds;
        DDBuffer_PTR ddb = pEbb->buffers + pEbb->num;

        gcoOS_ZeroMemory(&dds, sizeof(dds));
        dds.dwSize = sizeof(dds);

        if (FAILED(lpDDSurface->Lock(NULL,
                &dds,
                DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,
                NULL)))
        {
            break;
        }

        lpDDSurface->Unlock(NULL);

        if (!(dds.dwFlags & DDSD_LPSURFACE))
        {
            break;
        }

        if (!(dds.dwFlags & DDSD_BACKBUFFERCOUNT))
        {
            break;
        }

        ddb->width = dds.dwWidth;
        ddb->height= dds.dwHeight;
        ddb->stride= dds.lPitch;
        ddb->lpSurface = dds.lpSurface;

        if (!GetDDSFormat(&dds.ddpfPixelFormat, &ddb->format, gcvNULL))
        {
            break;
        }

        gceSTATUS status;

        gcmERR_BREAK(gcoSURF_ConstructWrapper(
            gcvNULL,
            &ddb->surface
            ));

        /* Set the underlying frame buffer surface. */
        gcmERR_BREAK(gcoSURF_SetBuffer(
            ddb->surface,
            gcvSURF_BITMAP,
            ddb->format,
            ddb->stride,
            ddb->lpSurface,
            ~0U
            ));

        /* Set the window. */
        gcmERR_BREAK(gcoSURF_SetWindow(
            ddb->surface,
            0,
            0,
            ddb->width,
            ddb->height
            ));

        ++pEbb->num;

        return DDENUMRET_OK;
    }

    return DDENUMRET_CANCEL;
}

static gctBOOL
DestroyBackSufaces(halLocalDisplay *halLocalDisplay, halLocalDDSurf_PTR lSurf)
{
    if (lSurf)
    {
        if (halLocalDisplay->localBackSurf == lSurf)
        {
            halLocalDisplay->localBackSurf = gcvNULL;
        }
        else
        {
            halLocalDDSurf *pre;
            for (pre = halLocalDisplay->localBackSurf; pre != gcvNULL; pre = pre->next)
            {
                if (pre->next == lSurf)
                {
                    pre->next = lSurf->next;
                    break;
                }
            }
        }

        gctUINT i;

        for (i = 0; i < lSurf->numberOfBuffer; ++i)
        {
            if (lSurf->buffers[i].surface)
            {
                gcoSURF_Destroy(lSurf->buffers[i].surface);
                lSurf->buffers[i].surface = gcvNULL;
            }
        }

        if (lSurf->semaphore)
        {
            CloseHandle(lSurf->semaphore);
        }

        gcoOS_Free(gcvNULL, lSurf);

        return gcvTRUE;
    }
    else
    {
        for (lSurf = halLocalDisplay->localBackSurf; lSurf != gcvNULL;
            lSurf = halLocalDisplay->localBackSurf)
        {
            gctUINT i;

            halLocalDisplay->localBackSurf = lSurf->next;

            for (i = 0; i < lSurf->numberOfBuffer; ++i)
            {
                if (lSurf->buffers[i].surface)
                {
                    gcoSURF_Destroy(lSurf->buffers[i].surface);
                    lSurf->buffers[i].surface = gcvNULL;
                }
            }

            if (lSurf->semaphore)
            {
                CloseHandle(lSurf->semaphore);
            }

            gcoOS_Free(gcvNULL, lSurf);
        }
    }

    return gcvFALSE;
}

static gctBOOL
CreateBackSufaces(halLocalDisplay *halLocalDisplay, IDirectDrawSurface* pDDSurface, int BackBufferCount)
{
    halLocalDDSurf *lsurf = gcvNULL;

    do {
        gctSIZE_T size = sizeof(halLocalDDSurf) + sizeof(DDBuffer) * BackBufferCount;

        if (gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&lsurf) != gcvSTATUS_OK)
        {
            break;
        }

        gcoOS_ZeroMemory(lsurf, size);

        lsurf->numberOfBuffer = BackBufferCount + 1;
        lsurf->ddSurface = (gctPOINTER)pDDSurface;
        lsurf->current = 0;

        EnumBackBuffer ebb;

        ebb.num = 0;
        ebb.buffers = lsurf->buffers;

        if (FAILED(EnumGetBackBufferCallback(pDDSurface, NULL, (LPVOID)&ebb)))
        {
            break;
        }

        if (FAILED(pDDSurface->EnumAttachedSurfaces((LPVOID)&ebb, EnumGetBackBufferCallback)))
        {
            break;
        }

        gcmASSERT(ebb.num == BackBufferCount);

        lsurf->semaphore = CreateSemaphore(NULL, BackBufferCount, BackBufferCount, NULL);
        if (lsurf->semaphore == NULL)
        {
            break;
        }

        // Insert into local list of display.
        lsurf->next = halLocalDisplay->localBackSurf;
        halLocalDisplay->localBackSurf = lsurf;

        return gcvTRUE;

    } while (gcvFALSE);

    if (lsurf)
    {
        DestroyBackSufaces(halLocalDisplay, lsurf);
    }

    return gcvFALSE;
}

static gctBOOL
IsSupportedDDSurface(void *pDDSurface, gctUINT32 *Caps)
{
    if (IsWindow((HWND)pDDSurface))
    {
        return gcvFALSE;
    }

    if (IsBadReadPtr(pDDSurface, sizeof(pDDSurface)))
    {
        return gcvFALSE;
    }

    DDSCAPS dwsCap;
    IDirectDrawSurface *pDDS = (IDirectDrawSurface*)pDDSurface;

    if(FAILED(pDDS->GetCaps(&dwsCap)))
    {
        return gcvFALSE;
    }
    else
    {
        if (Caps)
        {
            *Caps = dwsCap.dwCaps;
        }
        return gcvTRUE;
    }
}

gceSTATUS
gcoOS_InitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    halLocalDisplay *ldisplay = (halLocalDisplay*)(*localDisplay);

    do {
        if (ldisplay == gcvNULL)
        {
            if (gcmIS_ERROR(gcoOS_Allocate(
                    gcvNULL, gcmSIZEOF(halLocalDisplay), (gctPOINTER*)&ldisplay)))
            {
                break;
            }

            gcoOS_ZeroMemory(ldisplay, gcmSIZEOF(halLocalDisplay));

#if WRAP_FULLSCREEN_WINDOW
            if (FAILED(DirectDrawCreate(NULL, &ldisplay->pDD, NULL)))
            {
                break;
            }

            SystemParametersInfo(SPI_GETWORKAREA, 0, &ldisplay->rect, FALSE);
#endif
            *localDisplay = ldisplay;
        }

        return status;

    } while (gcvFALSE);

    if (ldisplay)
    {
        if (ldisplay->pDD)
        {
            ldisplay->pDD->Release();
        }

        gcoOS_Free(gcvNULL, ldisplay);
    }

    *localDisplay = gcvNULL;
    status = gcvSTATUS_INVALID_ARGUMENT;
    return status;
}

gceSTATUS
gcoOS_DeinitLocalDisplayInfo(
    IN HALNativeDisplayType Display,
    IN OUT gctPOINTER * localDisplay
    )
{
    halLocalDisplay *ldisplay = (halLocalDisplay*)(*localDisplay);

    if (ldisplay)
    {
        if (ldisplay->localBackSurf)
        {
            DestroyBackSufaces(ldisplay, gcvNULL);
        }
#if WRAP_FULLSCREEN_WINDOW
        if (ldisplay->pDDS)
        {
            if (ldisplay->logical)
            {
                ldisplay->pDDS->Unlock(gcvNULL);
            }

            ldisplay->pDDS->Release();
        }

        if (ldisplay->pDD)
        {
            ldisplay->pDD->Release();
        }
#endif
        gcoOS_Free(gcvNULL, ldisplay);
    }

    *localDisplay = gcvNULL;
    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_GetDisplayInfoEx2(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    IN gctUINT DisplayInfoSize,
    OUT halDISPLAY_INFO * DisplayInfo
    )
{
    halLocalDisplay *ldisplay = (halLocalDisplay*)localDisplay;

    do {
        if (!DisplayInfo || !ldisplay)
        {
            break;
        }

        gcoOS_ZeroMemory(DisplayInfo, sizeof(halDISPLAY_INFO));

        if (IsWindow(Window))
        {
#if WRAP_FULLSCREEN_WINDOW
            RECT rect;

            /* Check whether fullscreen. */
            if (!GetClientRect(Window, &rect))
            {
                break;
            }

            if (!ClientToScreen(Window, (LPPOINT)&rect)
                || !ClientToScreen(Window, ((LPPOINT)&rect) + 1))
            {
                break;
            }

            if ((rect.left > ldisplay->rect.left)
                || (rect.top > ldisplay->rect.top)
                || (rect.right < ldisplay->rect.right)
                || (rect.bottom < ldisplay->rect.bottom))
            {
                break;
            }

            if (FAILED(ldisplay->pDD->SetCooperativeLevel(Window, DDSCL_FULLSCREEN)))
            {
                break;
            }

            if (ldisplay->pDDS == gcvNULL)
            {
                /* create primary surface. */

                DDSURFACEDESC dds;

                gcoOS_ZeroMemory(&dds, sizeof(dds));

                dds.dwSize = sizeof(dds);

                dds.dwFlags = DDSD_CAPS;

                dds.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

                if (FAILED(ldisplay->pDD->CreateSurface(&dds,&ldisplay->pDDS, NULL)))
                {
                    break;
                }

                if (FAILED(ldisplay->pDDS->Lock(NULL,
                        &dds,
                        DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,
                        NULL)))
                {
                    break;
                }

                if (!(dds.dwFlags & DDSD_LPSURFACE))
                {
                    ldisplay->pDDS->Unlock(NULL);
                    break;
                }

                SetLastError(0);
                SetWindowLong(Window, GWL_USERDATA, WRAP_FULLSCREEN_WINDOW_MAGIC);
                if (GetLastError() != 0)
                {
                    ldisplay->pDDS->Unlock(NULL);
                    break;
                }

                ldisplay->logical = dds.lpSurface;
                ldisplay->physical = ~0U;
                ldisplay->stride = dds.lPitch;
                ldisplay->width = dds.dwWidth;
                ldisplay->height = dds.dwHeight;
            }

            DisplayInfo->logical = ldisplay->logical;
            DisplayInfo->physical = ldisplay->physical;
            DisplayInfo->stride = ldisplay->stride ;
            DisplayInfo->flip  = gcvFALSE;
            DisplayInfo->width = ldisplay->width;
            DisplayInfo->height = ldisplay->height;
            DisplayInfo->wrapFB = gcvTRUE;
            DisplayInfo->multiBuffer = 1;

            return gcvSTATUS_OK;
#else
            return gcvSTATUS_INVALID_ARGUMENT;
#endif
        }
        else if (IsSupportedDDSurface(Window, gcvNULL))
        {
            IDirectDrawSurface *pDDS = (IDirectDrawSurface*)Window;
            DDSURFACEDESC dds;

            gcoOS_ZeroMemory(&dds, sizeof(dds));

            dds.dwSize = sizeof(dds);

            if (FAILED(pDDS->Lock(NULL,

                    &dds,

                    DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,

                    NULL)))
            {
                break;
            }

            pDDS->Unlock(NULL);

            if (!(dds.dwFlags & DDSD_LPSURFACE))
            {
                break;
            }

            DisplayInfo->logical = dds.lpSurface;
            DisplayInfo->physical = ~0U;
            DisplayInfo->stride = dds.lPitch;
            DisplayInfo->width = dds.dwWidth;
            DisplayInfo->height = dds.dwHeight;
            DisplayInfo->wrapFB = gcvTRUE;

            if ((dds.dwFlags & DDSD_BACKBUFFERCOUNT) && (dds.dwBackBufferCount > 0))
            {
                if (!CreateBackSufaces(ldisplay, pDDS, dds.dwBackBufferCount))
                {
                    break;
                }

                DisplayInfo->flip = gcvTRUE;
                DisplayInfo->multiBuffer = dds.dwBackBufferCount;
            }
            else
            {
                DisplayInfo->flip = gcvFALSE;
                DisplayInfo->multiBuffer = 1;
            }
            return gcvSTATUS_OK;
        }
    } while (gcvFALSE);
    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
gcoOS_GetWindowInfoEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("Display=0x%x Window=0x%x", Display, Window);
    if (IsWindow(Window))
    {
        RECT rect;
        INT bitsPerPixel;
        gceSURF_FORMAT format;

        /* Get device context bit depth. */
        bitsPerPixel = GetDeviceCaps(Display, BITSPIXEL);

        /* Return format for window depth. */
        switch (bitsPerPixel)
        {
        case 16:
            /* 16-bits per pixel. */
            format = gcvSURF_R5G6B5;
            break;

        case 32:
            /* 32-bits per pixel. */
            format = gcvSURF_A8R8G8B8;
            break;

        default:
            /* Unsupported colot depth. */
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        ShowWindow( Window, SW_SHOWNORMAL );

        /* Query window client rectangle. */
        if (!GetClientRect(Window, &rect))
        {
            /* Error. */
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        /* Set the output window parameters. */
        if (X != gcvNULL)
        {
            *X = rect.left;
        }

        if (Y != gcvNULL)
        {
            *Y = rect.top;
        }

        if (Width != gcvNULL)
        {
            *Width = rect.right  - rect.left;
        }

        if (Height != gcvNULL)
        {
            *Height = rect.bottom - rect.top;
        }

        if (BitsPerPixel != gcvNULL)
        {
            *BitsPerPixel = bitsPerPixel;
        }

        if (Format != gcvNULL)
        {
            *Format = format;
        }

        if (Type != gcvNULL)
        {
            *Type = gcvSURF_BITMAP;
        }

        /* Success. */
        gcmFOOTER_NO();
        return status;
    }
    else if (IsSupportedDDSurface(Window, gcvNULL))
    {
        IDirectDrawSurface *pDDS = (IDirectDrawSurface*)Window;
        DDSURFACEDESC        ddsd;

        gcoOS_ZeroMemory(&ddsd, sizeof(ddsd));

        ddsd.dwSize = sizeof(ddsd);

        if (FAILED(pDDS->GetSurfaceDesc(&ddsd)))
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        if (!GetDDSFormat(&ddsd.ddpfPixelFormat, Format, (gctUINT*)BitsPerPixel))
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        if (X != gcvNULL)
        {
            *X = 0;
        }

        if (Y != gcvNULL)
        {
            *Y = 0;
        }

        if (Width != gcvNULL)
        {
            *Width = ddsd.dwWidth;
        }

        if (Height != gcvNULL)
        {
            *Height = ddsd.dwHeight;
        }

        if (Type != gcvNULL)
        {
            *Type = gcvSURF_BITMAP;
        }

        gcmFOOTER_NO();
        return status;
    }

    status = gcvSTATUS_INVALID_ARGUMENT;
    gcmFOOTER();
    return status;
}

gceSTATUS
gcoOS_GetDisplayBackbufferEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER  localDisplay,
    OUT gctPOINTER  *  context,
    OUT gcoSURF     *  surface,
    OUT gctUINT * Offset,
    OUT gctINT * X,
    OUT gctINT * Y
    )
{
    gctUINT32 caps;

    if (context)
    {
        *context = gcvNULL;
    }

    if (surface)
    {
        *surface = gcvNULL;
    }

    if (Offset)
    {
        *Offset = 0;
    }

    if (X)
    {
        *X = 0;
    }

    if (Y)
    {
        *Y = 0;
    }

    do {
        if (!IsSupportedDDSurface(Window, &caps))
        {
            break;
        }

        if ((caps & DDSCAPS_FLIP) != DDSCAPS_FLIP)
        {
            break;
        }

        halLocalDisplay *ldisplay = (halLocalDisplay*)localDisplay;
        halLocalDDSurf *lSurf = ldisplay->localBackSurf;

        while (lSurf)
        {
            if (lSurf->ddSurface == Window)
            {
                break;
            }

            lSurf = lSurf->next;
        }

        if (!lSurf)
        {
            break;
        }

        if (!lSurf->semaphore)
        {
            break;
        }

        WaitForSingleObject(lSurf->semaphore, INFINITE);

        lSurf->current = (lSurf->current+ 1) % lSurf->numberOfBuffer;

        *context = lSurf;
        *surface = lSurf->buffers[lSurf->current].surface;

        return gcvSTATUS_OK;

    } while (gcvFALSE);

    return gcvSTATUS_INVALID_ARGUMENT;
}

gceSTATUS
gcoOS_SetDisplayVirtualEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gctUINT32 caps;
    halLocalDDSurf *lSurf = (halLocalDDSurf*)Context;
    DDSCAPS DDSCaps;


    if (!lSurf || !IsSupportedDDSurface(Window, &caps))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ((lSurf->ddSurface != Window)|| !(caps & DDSCAPS_FLIP))
    {
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    ((IDirectDrawSurface*)Window)->GetCaps(&DDSCaps);

    if(DDSCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        LPDIRECTDRAW pDD;
        HRESULT ret = DD_OK;

        ((IDirectDrawSurface*)Window)->GetDDInterface(&pDD);

        ret = pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);

        if(ret != DD_OK)
            printf("WARNING: WaitForVerticalBlank didn't work correctly, warning = 0x%x\n", ret);
    }

    ((IDirectDrawSurface*)Window)->Flip(NULL, DDFLIP_WAITNOTBUSY);

    ReleaseSemaphore(lSurf->semaphore, 1, NULL);

    return gcvSTATUS_OK;
}

gceSTATUS
gcoOS_CancelDisplayBackbuffer(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gctPOINTER Context,
    IN gcoSURF Surface,
    IN gctUINT Offset,
    IN gctINT X,
    IN gctINT Y
    )
{
    gcmPRINT("%s: TODO", __FUNCTION__);
    return gcvSTATUS_NOT_SUPPORTED;
}

gceSTATUS
gcoOS_DrawImageEx(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
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
    gceSTATUS status = gcvSTATUS_INVALID_ARGUMENT;
    HDC context = gcvNULL;
    IDirectDrawSurface *pDDS = gcvNULL;
    do
    {
        BITFIELDINFO bfi;
        PBITMAPINFOHEADER info = &bfi.bmi.bmiHeader;
        gctUINT32 *mask = (gctUINT32*)(info + 1);

        /* Get the device context of the window. */
        if (IsWindow(Window))
        {
            context = GetDC(Window);
        }
        else if (IsSupportedDDSurface(Window, gcvNULL))
        {
            pDDS = (IDirectDrawSurface*)Window;
            if (FAILED(pDDS->GetDC(&context)))
                break;
        }

        if (context == gcvNULL)
        {
            /* Error. */
            break;
        }

        switch (Format)
        {
        case gcvSURF_A8R8G8B8:
        case gcvSURF_X8R8G8B8:
            mask[0] = 0x00FF0000;
            mask[1] = 0x0000FF00;
            mask[2] = 0x000000FF;
            break;

        case gcvSURF_R8G8B8A8:
        case gcvSURF_R8G8B8X8:
            mask[0] = 0xFF000000;
            mask[1] = 0x00FF0000;
            mask[2] = 0x0000FF00;
            break;

        case gcvSURF_X8B8G8R8:
        case gcvSURF_A8B8G8R8:
            mask[0] = 0x000000FF;
            mask[1] = 0x0000FF00;
            mask[2] = 0x00FF0000;
            break;

        case gcvSURF_B8G8R8A8:
        case gcvSURF_B8G8R8X8:
            mask[0] = 0x0000FF00;
            mask[1] = 0x00FF0000;
            mask[2] = 0xFF000000;
            break;

        case gcvSURF_X4R4G4B4:
        case gcvSURF_A4R4G4B4:
            mask[0] = 0x00000F00;
            mask[1] = 0x000000F0;
            mask[2] = 0x0000000F;
            break;

        case gcvSURF_R4G4B4X4:
        case gcvSURF_R4G4B4A4:
            mask[0] = 0x0000F000;
            mask[1] = 0x00000F00;
            mask[2] = 0x000000F0;
            break;

        case gcvSURF_X4B4G4R4:
        case gcvSURF_A4B4G4R4:
            mask[0] = 0x0000000F;
            mask[1] = 0x000000F0;
            mask[2] = 0x00000F00;
            break;

        case gcvSURF_B4G4R4X4:
        case gcvSURF_B4G4R4A4:
            mask[0] = 0x000000F0;
            mask[1] = 0x00000F00;
            mask[2] = 0x0000F000;
            break;

        case gcvSURF_R5G6B5:
            mask[0] = 0x0000F800;
            mask[1] = 0x000007E0;
            mask[2] = 0x0000001F;
            break;

        case gcvSURF_B5G6R5:
            mask[0] = 0x0000001F;
            mask[1] = 0x000007E0;
            mask[2] = 0x0000F800;
            break;

        case gcvSURF_A1R5G5B5:
        case gcvSURF_X1R5G5B5:
            mask[0] = 0x00007C00;
            mask[1] = 0x000003E0;
            mask[2] = 0x0000001F;
            break;

        case gcvSURF_R5G5B5X1:
        case gcvSURF_R5G5B5A1:
            mask[0] = 0x0000F800;
            mask[1] = 0x000007C0;
            mask[2] = 0x0000003E;
            break;

        case gcvSURF_B5G5R5X1:
        case gcvSURF_B5G5R5A1:
            mask[0] = 0x0000003E;
            mask[1] = 0x000007C0;
            mask[2] = 0x0000F800;
            break;

        case gcvSURF_X1B5G5R5:
        case gcvSURF_A1B5G5R5:
            mask[0] = 0x0000001F;
            mask[1] = 0x000003E0;
            mask[2] = 0x00007C00;
            break;

        default:
            ReleaseDC(Window, context);
            return status;
        }

        /* Fill in the bitmap information. */
        info->biSize          = sizeof(bfi.bmi.bmiHeader);
        info->biWidth         = Width;
        info->biHeight        = - Height;
        info->biPlanes        = 1;
        info->biBitCount      = (gctUINT16) BitsPerPixel;
        info->biCompression   = BI_BITFIELDS;
        info->biSizeImage     = ((Width * BitsPerPixel / 8 + 3) & ~3) * gcmABS(Height);
        info->biXPelsPerMeter = 0;
        info->biYPelsPerMeter = 0;
        info->biClrUsed       = 0;
        info->biClrImportant  = 0;

        /* Draw bitmap bits to window. */
        if (StretchDIBits(context,
                  Left, Top, Right - Left, Bottom - Top,
                  Left, Height - Bottom, Right - Left, Bottom - Top,
                  Bits,
                  (BITMAPINFO *) info,
                  DIB_RGB_COLORS,
                  SRCCOPY) == 0)
        {
            /* Error. */
            break;
        }

        /* Success. */
        status = gcvSTATUS_OK;
    }
    while (gcvFALSE);

    /* Release the device context. */
    if (context != gcvNULL)
    {
        if (pDDS)
        {
            pDDS->ReleaseDC(context);
        }
        else
        {
            ReleaseDC(Window, context);
        }
    }

    /* Return result. */
    return status;
}

gceSTATUS
gcoOS_SetWindowFormat(
    IN HALNativeDisplayType Display,
    IN HALNativeWindowType Window,
    IN gceSURF_TYPE Type,
    IN gceSURF_FORMAT Format
    )
{
    /*
     * Possiable types:
     *   gcvSURF_BITMAP
     *   gcvSURF_RENDER_TARGET
     *   gcvSURF_RENDER_TARGET_NO_COMPRESSION
     *   gcvSURF_RENDER_TARGET_NO_TILE_STATUS
     */
    return gcvSTATUS_NOT_SUPPORTED;
}

#endif /* USE_CE_DIRECTDRAW */
#endif

