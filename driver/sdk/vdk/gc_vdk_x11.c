/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef EGL_API_DRI
#  include <drm_sarea.h>
#  include "drmgl.h"
#  include <xf86dri.h>
#endif

/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$X11\n";

typedef struct _PixmapList PixmapList;

struct _PixmapList
{
    NativePixmapType    pixmap;

    /* Parameters. */
    int                 width;
    int                 height;
    int                 bitsPerPixel;

    /* Contents. */
    int                 stride;
    void *              memory;
    uint32_t            address;
    uint32_t            node;

    PixmapList *        prev;
    PixmapList *        next;
};

struct _vdkPrivate
{
    vdkDisplay          display;
    void *              egl;

    PixmapList *        pixmapList;
    pthread_mutex_t     pixmapMutex;
};

static vdkPrivate _vdk = NULL;

/* Structure that defined keyboard mapping. */
typedef struct _keyMap
{
    /* Normal key. */
    vdkKeys normal;

    /* Extended key. */
    vdkKeys extended;
}
keyMap;

static keyMap keys[] =
{
    /* 00 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 01 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 02 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 03 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 04 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 05 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 06 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 07 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 08 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 09 */ { VDK_ESCAPE,          VDK_UNKNOWN },
    /* 0A */ { VDK_1,               VDK_UNKNOWN },
    /* 0B */ { VDK_2,               VDK_UNKNOWN },
    /* 0C */ { VDK_3,               VDK_UNKNOWN },
    /* 0D */ { VDK_4,               VDK_UNKNOWN },
    /* 0E */ { VDK_5,               VDK_UNKNOWN },
    /* 0F */ { VDK_6,               VDK_UNKNOWN },
    /* 10 */ { VDK_7,               VDK_UNKNOWN },
    /* 11 */ { VDK_8,               VDK_UNKNOWN },
    /* 12 */ { VDK_9,               VDK_UNKNOWN },
    /* 13 */ { VDK_0,               VDK_UNKNOWN },
    /* 14 */ { VDK_HYPHEN,          VDK_UNKNOWN },
    /* 15 */ { VDK_EQUAL,           VDK_UNKNOWN },
    /* 16 */ { VDK_BACKSPACE,       VDK_UNKNOWN },
    /* 17 */ { VDK_TAB,             VDK_UNKNOWN },
    /* 18 */ { VDK_Q,               VDK_UNKNOWN },
    /* 19 */ { VDK_W,               VDK_UNKNOWN },
    /* 1A */ { VDK_E,               VDK_UNKNOWN },
    /* 1B */ { VDK_R,               VDK_UNKNOWN },
    /* 1C */ { VDK_T,               VDK_UNKNOWN },
    /* 1D */ { VDK_Y,               VDK_UNKNOWN },
    /* 1E */ { VDK_U,               VDK_UNKNOWN },
    /* 1F */ { VDK_I,               VDK_UNKNOWN },
    /* 20 */ { VDK_O,               VDK_UNKNOWN },
    /* 21 */ { VDK_P,               VDK_UNKNOWN },
    /* 22 */ { VDK_LBRACKET,        VDK_UNKNOWN },
    /* 23 */ { VDK_RBRACKET,        VDK_UNKNOWN },
    /* 24 */ { VDK_ENTER,           VDK_UNKNOWN },
    /* 25 */ { VDK_LCTRL,           VDK_UNKNOWN },
    /* 26 */ { VDK_A,               VDK_UNKNOWN },
    /* 27 */ { VDK_S,               VDK_UNKNOWN },
    /* 28 */ { VDK_D,               VDK_UNKNOWN },
    /* 29 */ { VDK_F,               VDK_UNKNOWN },
    /* 2A */ { VDK_G,               VDK_UNKNOWN },
    /* 2B */ { VDK_H,               VDK_UNKNOWN },
    /* 2C */ { VDK_J,               VDK_UNKNOWN },
    /* 2D */ { VDK_K,               VDK_UNKNOWN },
    /* 2E */ { VDK_L,               VDK_UNKNOWN },
    /* 2F */ { VDK_SEMICOLON,       VDK_UNKNOWN },
    /* 30 */ { VDK_SINGLEQUOTE,     VDK_UNKNOWN },
    /* 31 */ { VDK_BACKQUOTE,       VDK_UNKNOWN },
    /* 32 */ { VDK_LSHIFT,          VDK_UNKNOWN },
    /* 33 */ { VDK_BACKSLASH,       VDK_UNKNOWN },
    /* 34 */ { VDK_Z,               VDK_UNKNOWN },
    /* 35 */ { VDK_X,               VDK_UNKNOWN },
    /* 36 */ { VDK_C,               VDK_UNKNOWN },
    /* 37 */ { VDK_V,               VDK_UNKNOWN },
    /* 38 */ { VDK_B,               VDK_UNKNOWN },
    /* 39 */ { VDK_N,               VDK_UNKNOWN },
    /* 3A */ { VDK_M,               VDK_UNKNOWN },
    /* 3B */ { VDK_COMMA,           VDK_UNKNOWN },
    /* 3C */ { VDK_PERIOD,          VDK_UNKNOWN },
    /* 3D */ { VDK_SLASH,           VDK_UNKNOWN },
    /* 3E */ { VDK_RSHIFT,          VDK_UNKNOWN },
    /* 3F */ { VDK_PAD_ASTERISK,    VDK_UNKNOWN },
    /* 40 */ { VDK_LALT,            VDK_UNKNOWN },
    /* 41 */ { VDK_SPACE,           VDK_UNKNOWN },
    /* 42 */ { VDK_CAPSLOCK,        VDK_UNKNOWN },
    /* 43 */ { VDK_F1,              VDK_UNKNOWN },
    /* 44 */ { VDK_F2,              VDK_UNKNOWN },
    /* 45 */ { VDK_F3,              VDK_UNKNOWN },
    /* 46 */ { VDK_F4,              VDK_UNKNOWN },
    /* 47 */ { VDK_F5,              VDK_UNKNOWN },
    /* 48 */ { VDK_F6,              VDK_UNKNOWN },
    /* 49 */ { VDK_F7,              VDK_UNKNOWN },
    /* 4A */ { VDK_F8,              VDK_UNKNOWN },
    /* 4B */ { VDK_F9,              VDK_UNKNOWN },
    /* 4C */ { VDK_F10,             VDK_UNKNOWN },
    /* 4D */ { VDK_NUMLOCK,         VDK_UNKNOWN },
    /* 4E */ { VDK_SCROLLLOCK,      VDK_UNKNOWN },
    /* 4F */ { VDK_PAD_7,           VDK_UNKNOWN },
    /* 50 */ { VDK_PAD_8,           VDK_UNKNOWN },
    /* 51 */ { VDK_PAD_9,           VDK_UNKNOWN },
    /* 52 */ { VDK_PAD_HYPHEN,      VDK_UNKNOWN },
    /* 53 */ { VDK_PAD_4,           VDK_UNKNOWN },
    /* 54 */ { VDK_PAD_5,           VDK_UNKNOWN },
    /* 55 */ { VDK_PAD_6,           VDK_UNKNOWN },
    /* 56 */ { VDK_PAD_PLUS,        VDK_UNKNOWN },
    /* 57 */ { VDK_PAD_1,           VDK_UNKNOWN },
    /* 58 */ { VDK_PAD_2,           VDK_UNKNOWN },
    /* 59 */ { VDK_PAD_3,           VDK_UNKNOWN },
    /* 5A */ { VDK_PAD_0,           VDK_UNKNOWN },
    /* 5B */ { VDK_PAD_PERIOD,      VDK_UNKNOWN },
    /* 5C */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 5D */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 5E */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 5F */ { VDK_F11,             VDK_UNKNOWN },
    /* 60 */ { VDK_F12,             VDK_UNKNOWN },
    /* 61 */ { VDK_HOME,            VDK_UNKNOWN },
    /* 62 */ { VDK_UP,              VDK_UNKNOWN },
    /* 63 */ { VDK_PGUP,            VDK_UNKNOWN },
    /* 64 */ { VDK_LEFT,            VDK_UNKNOWN },
    /* 65 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 66 */ { VDK_RIGHT,           VDK_UNKNOWN },
    /* 67 */ { VDK_END,             VDK_UNKNOWN },
    /* 68 */ { VDK_DOWN,            VDK_UNKNOWN },
    /* 69 */ { VDK_PGDN,            VDK_UNKNOWN },
    /* 6A */ { VDK_INSERT,          VDK_UNKNOWN },
    /* 6B */ { VDK_DELETE,          VDK_UNKNOWN },
    /* 6C */ { VDK_PAD_ENTER,       VDK_UNKNOWN },
    /* 6D */ { VDK_RCTRL,           VDK_UNKNOWN },
    /* 6E */ { VDK_BREAK,           VDK_UNKNOWN },
    /* 6F */ { VDK_PRNTSCRN,        VDK_UNKNOWN },
    /* 70 */ { VDK_PAD_SLASH,       VDK_UNKNOWN },
    /* 71 */ { VDK_RALT,            VDK_UNKNOWN },
    /* 72 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 73 */ { VDK_LWINDOW,         VDK_UNKNOWN },
    /* 74 */ { VDK_RWINDOW,         VDK_UNKNOWN },
    /* 75 */ { VDK_MENU,            VDK_UNKNOWN },
    /* 76 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 77 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 78 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 79 */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7A */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7B */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7C */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7D */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7E */ { VDK_UNKNOWN,         VDK_UNKNOWN },
    /* 7F */ { VDK_UNKNOWN,         VDK_UNKNOWN },
};

static int _terminate = 0;

static void
_SignalHandler(
    int SigNum
    )
{
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    _terminate = 1;
}

#include "gc_hal_base.h"
#include "gc_hal_profiler.h"
#include "gc_hal_driver.h"

static int (* _ImportVideoMemory)(uint32_t name,
                                  uint32_t *node);

static int (* _WrapUserMemory)(gcsUSER_MEMORY_DESC *desc,
                               uint32_t *node);

static int (* _ReleaseVideoMemory)(uint32_t node);

static int (* _LockVideoMemory)(uint32_t node,
                                int cacheable,
                                int engine,
                                uint32_t *address,
                                void **memory);

static int (* _UnlockVideoMemory)(uint32_t node,
                                  int type,
                                  int engine);

/*******************************************************************************
** Initialization.
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    vdkPrivate  vdk = NULL;

    if (_vdk != NULL)
    {
        return _vdk;
    }

    vdk = (vdkPrivate) malloc(sizeof(struct _vdkPrivate));

    if (vdk == NULL)
    {
        return NULL;
    }

    XInitThreads();
    signal(SIGINT,  _SignalHandler);
    signal(SIGQUIT, _SignalHandler);
    signal(SIGTERM, _SignalHandler);

#if gcdSTATIC_LINK
    vdk->egl = NULL;
#else
    vdk->egl = dlopen("libEGL.so", RTLD_LAZY);
#endif

    /* Require these 4 functions from Vivante GAL. */
    _ImportVideoMemory  = dlsym(RTLD_DEFAULT, "gcoHAL_ImportVideoMemory");
    _WrapUserMemory     = dlsym(RTLD_DEFAULT, "gcoHAL_WrapUserMemory");
    _ReleaseVideoMemory = dlsym(RTLD_DEFAULT, "gcoHAL_ReleaseVideoMemory");
    _LockVideoMemory    = dlsym(RTLD_DEFAULT, "gcoHAL_LockVideoMemory");
    _UnlockVideoMemory  = dlsym(RTLD_DEFAULT, "gcoHAL_UnlockVideoMemory");

    vdk->display = (EGLNativeDisplayType) NULL;

    pthread_mutex_init(&vdk->pixmapMutex, NULL);

    _vdk = vdk;
    return vdk;


}

VDKAPI void VDKLANG
vdkExit(
    vdkPrivate Private
    )
{
    if (Private != NULL)
    {
        if (_vdk == Private)
        {
            _vdk = NULL;
        }

        if (Private->egl != NULL)
        {
            dlclose(Private->egl);
            Private->egl = NULL;
        }

        free(Private);
    }
}

/*******************************************************************************
** Display.
*/

VDKAPI vdkDisplay VDKLANG
vdkGetDisplayByIndex(
    vdkPrivate Private,
    int DisplayIndex
    )
{
    XImage *  image;
    Display * dpy = NULL;

    if (!Private)
    {
        return NULL;
    }

    if (Private->display != (EGLNativeDisplayType) NULL)
    {
        return Private->display;
    }

    dpy = XOpenDisplay(NULL);

    if (dpy == NULL)
    {
        fprintf(stderr,
                "Can not open display: %s\n", getenv("DISPLAY"));

        return NULL;
    }

    image = XGetImage(dpy,
                      DefaultRootWindow(dpy),
                      0, 0, 1, 1, AllPlanes, ZPixmap);

    if (image == NULL)
    {
        /* Error. */
        XCloseDisplay(dpy);
        return NULL;
    }

    XDestroyImage(image);
    Private->display = dpy;

    return dpy;
}

VDKAPI vdkDisplay VDKLANG
vdkGetDisplay(
    vdkPrivate Private
    )
{
    return vdkGetDisplayByIndex(Private, 0);
}

VDKAPI int VDKLANG
vdkGetDisplayInfo(
    vdkDisplay Display,
    int * Width,
    int * Height,
    unsigned long * Physical,
    int * Stride,
    int * BitsPerPixel
    )
{
    Screen * screen;
    XImage *image;

    if (Display == NULL)
    {
        return 0;
    }

    screen = XScreenOfDisplay(Display, DefaultScreen(Display));

    if (Width != NULL)
    {
        *Width = XWidthOfScreen(screen);
    }

    if (Height != NULL)
    {
        *Height = XHeightOfScreen(screen);
    }

    if (Physical != NULL)
    {
        *Physical = ~0;
    }

    if (Stride != NULL)
    {
        *Stride = 0;
    }

    if (BitsPerPixel != NULL)
    {
        image = XGetImage(Display,
            DefaultRootWindow(Display),
            0, 0, 1, 1, AllPlanes, ZPixmap);

        if (image != NULL)
        {
            *BitsPerPixel = image->bits_per_pixel;
            XDestroyImage(image);
        }

    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    if (Display != NULL)
    {
        XCloseDisplay(Display);
    }

    if (_vdk->display == Display)
    {
        _vdk->display = NULL;
    }
}

/*******************************************************************************
** Windows
*/

vdkWindow
vdkCreateWindow(
    vdkDisplay Display,
    int X,
    int Y,
    int Width,
    int Height
    )
{
    Window window = 0;
    XSetWindowAttributes attr;
    Screen * screen;
    int width, height;
    int ignoreDisplaySize = 0;
    char * p;

    /* Test if we have a valid display data structure pointer. */
    if (Display == NULL)
    {
        return 0;
    }

    do
    {
        screen = XScreenOfDisplay(Display, DefaultScreen(Display));
        width  = XWidthOfScreen (screen);
        height = XHeightOfScreen(screen);

        attr.background_pixmap = 0;
        attr.border_pixel      = 0;
        attr.event_mask        = ButtonPressMask
                               | ButtonReleaseMask
                               | KeyPressMask
                               | KeyReleaseMask
                               | ResizeRedirectMask
                               | PointerMotionMask;

        p = getenv("X_IGNORE_DISPLAY_SIZE");
        if (p != NULL)
        {
            ignoreDisplaySize = atoi(p);
        }

        /* Test for zero width. */
        if (Width == 0)
        {
            /* Use display width instead. */
            Width = width;
        }

        /* Test for zero height. */
        if (Height == 0)
        {
            /* Use display height instead. */
            Height = height;
        }

        /* Test for auto-center X coordinate. */
        if (X == -1)
        {
            /* Center the window horizontally. */
            X = (width - Width) / 2;
        }

        /* Test for auto-center Y coordinate. */
        if (Y == -1)
        {
            /* Center the window vertically. */
            Y = (height - Height) / 2;
        }

        /* Clamp coordinates to display. */
        if (X < 0) X = 0;
        if (Y < 0) Y = 0;
        if (!ignoreDisplaySize)
        {
            if (X + Width  > width)  Width  = width  - X;
            if (Y + Height > height) Height = height - Y;
        }

        window = XCreateWindow(Display,
                               RootWindow(Display,
                                          DefaultScreen(Display)),
                               X, Y,
                               Width, Height,
                               0,
                               DefaultDepth(Display, DefaultScreen(Display)),
                               InputOutput,
                               DefaultVisual(Display, DefaultScreen(Display)),
                               CWBackPixel
                               | CWBorderPixel
                               | CWEventMask,
                               &attr);

        if (!window)
        {
            break;
        }

        XMoveWindow(Display, window, X, Y);
        return window;
    }
    while (0);

    if (window)
    {
        XUnmapWindow(Display, window);
        XDestroyWindow(Display, window);
    }

    return 0;
}

VDKAPI int VDKLANG
vdkGetWindowInfo(
    vdkWindow Window,
    int * X,
    int * Y,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    unsigned int * Offset
    )
{
    Display * dpy = _vdk->display;
    XWindowAttributes attr;
    XImage *image;

    if (dpy == NULL || Window == 0)
    {
        return 0;
    }

    XGetWindowAttributes(dpy, Window, &attr);

    if (X != NULL)
    {
        *X = attr.x;
    }

    if (Y != NULL)
    {
        *Y = attr.y;
    }

    if (Width != NULL)
    {
        *Width = attr.width;
    }

    if (Height != NULL)
    {
        *Height = attr.height;
    }

    if (BitsPerPixel != NULL)
    {
        image = XGetImage(dpy,
            DefaultRootWindow(dpy),
            0, 0, 1, 1, AllPlanes, ZPixmap);

        if (image != NULL)
        {
            *BitsPerPixel = image->bits_per_pixel;
            XDestroyImage(image);
        }
    }

    if (Offset != NULL)
    {
        *Offset = 0;
    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    if ((_vdk->display == NULL) || (Window == 0))
    {
        return;
    }

    XUnmapWindow(_vdk->display, Window);
    XDestroyWindow(_vdk->display, Window);
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    if ((_vdk->display == NULL) || (Window == 0))
    {
        return 0;
    }

    XMapRaised(_vdk->display, Window);
    return 1;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    if ((_vdk->display == NULL) || (Window == 0))
    {
        return 0;
    }

    XUnmapWindow(_vdk->display, Window);
    return 1;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
    XTextProperty tp;

    if ((_vdk->display == NULL) || (Window == 0))
    {
        return;
    }

    XStringListToTextProperty((char**) &Title, 1, &tp);

    XSetWMProperties(_vdk->display, Window,
                     &tp, &tp,
                     NULL, 0, NULL, NULL, NULL);
}

VDKAPI void VDKLANG
vdkCapturePointer(
    vdkWindow Window
    )
{
}

/*******************************************************************************
** Events.
*/
VDKAPI int VDKLANG
vdkGetEvent(
    vdkWindow Window,
    vdkEvent * Event
    )
{
    Display * dpy = _vdk->display;
    XEvent event;
    vdkKeys scancode;

    if ((dpy == NULL) || (Window == 0) || (Event == NULL))
    {
        return 0;
    }

    while (XPending(dpy))
    {
        XNextEvent(dpy, &event);

        switch (event.type)
        {
        case MotionNotify:
            Event->type = VDK_POINTER;
            Event->data.pointer.x = event.xmotion.x;
            Event->data.pointer.y = event.xmotion.y;
            return 1;

        case ButtonRelease:
            Event->type = VDK_BUTTON;
            Event->data.button.left   = event.xbutton.state & Button1Mask;
            Event->data.button.middle = event.xbutton.state & Button2Mask;
            Event->data.button.right  = event.xbutton.state & Button3Mask;
            Event->data.button.x      = event.xbutton.x;
            Event->data.button.y      = event.xbutton.y;
            return 1;

        case KeyPress:
        case KeyRelease:
            scancode = keys[event.xkey.keycode].normal;

            if (scancode == VDK_UNKNOWN)
            {
                break;
            }

            Event->type                   = VDK_KEYBOARD;
            Event->data.keyboard.pressed  = (event.type == KeyPress);
            Event->data.keyboard.scancode = scancode;
            Event->data.keyboard.key      = (  (scancode < VDK_SPACE)
                || (scancode >= VDK_F1)
                )
                ? 0
                : (char) scancode;
            return 1;
        }
    }

    if (_terminate)
    {
        _terminate = 0;
        Event->type = VDK_CLOSE;
        return 1;
    }

    return 0;
}

/*******************************************************************************
** EGL Support. ****************************************************************
*/

EGL_ADDRESS
vdkGetAddress(
    vdkPrivate Private,
    const char * Function
    )
{
#if gcdSTATIC_LINK
    return (EGL_ADDRESS) eglGetProcAddress(Function);
#else
    return (EGL_ADDRESS) dlsym(Private->egl, Function);
 #endif
}

/*******************************************************************************
** Time. ***********************************************************************
*/

/*
    vdkGetTicks

    Get the number of milliseconds since the system started.

    PARAMETERS:

        None.

    RETURNS:

        unsigned int
            The number of milliseconds the system has been running.
*/
VDKAPI unsigned int VDKLANG
vdkGetTicks(
    void
    )
{
    struct timeval tv;

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/*******************************************************************************
** Pixmaps. ********************************************************************
*/
#if defined(EGL_API_DRI)

#include <X11/Xlibint.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>

#define X_VIVEXTQueryVersion            0
#define X_VIVEXTPixmapPhysaddr          1
#define X_VIVEXTDrawableFlush           2
#define X_VIVEXTDrawableInfo            3
#define X_VIVEXTFULLScreenInfo          4


#define VIVEXTNumberEvents              0

#define VIVEXTClientNotLocal            0
#define VIVEXTOperationNotSupported     1
#define VIVEXTNumberErrors        (VIVEXTOperationNotSupported + 1)



#define VIVEXTNAME "vivext"


#define VIVEXT_MAJOR_VERSION    1
#define VIVEXT_MINOR_VERSION    0
#define VIVEXT_PATCH_VERSION    0

typedef struct _VIVEXTQueryVersion {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
} xVIVEXTQueryVersionReq;
#define sz_xVIVEXTQueryVersionReq    4


typedef struct {
    BYTE      type;/* X_Reply */
    BYTE      pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    CARD16    majorVersion B16;        /* major version of vivEXT protocol */
    CARD16    minorVersion B16;        /* minor version of vivEXT protocol */
    CARD32    patchVersion B32;        /* patch version of vivEXT protocol */
    CARD32    pad3 B32;
    CARD32    pad4 B32;
    CARD32    pad5 B32;
    CARD32    pad6 B32;
} xVIVEXTQueryVersionReply;
#define sz_xVIVEXTQueryVersionReply    32


typedef struct _VIVEXTDrawableFlush {
    CARD8    reqType;        /* always vivEXTReqCode */
    CARD8    vivEXTReqType;        /* always X_vivEXTDrawableFlush */
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTDrawableFlushReq;
#define sz_xVIVEXTDrawableFlushReq    12

typedef struct _VIVEXTDrawableInfo {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTDrawableInfoReq;
#define sz_xVIVEXTDrawableInfoReq    12

typedef struct {
    BYTE    type;/* X_Reply */
    BYTE    pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    INT16    drawableX B16;
    INT16    drawableY B16;
    INT16    drawableWidth B16;
    INT16    drawableHeight B16;
    CARD32    numClipRects B32;
    INT16       relX B16;
    INT16       relY B16;
    CARD32      alignedWidth B32;
    CARD32      alignedHeight B32;
    CARD32      stride B32;
    CARD32      nodeName B32;
    CARD32      phyAddress B32;
} xVIVEXTDrawableInfoReply;

#define sz_xVIVEXTDrawableInfoReply    44


typedef struct _VIVEXTFULLScreenInfo {
    CARD8    reqType;
    CARD8    vivEXTReqType;
    CARD16    length B16;
    CARD32    screen B32;
    CARD32    drawable B32;
} xVIVEXTFULLScreenInfoReq;
#define sz_xVIVEXTFULLScreenInfoReq    12

typedef struct {
    BYTE    type;            /* X_Reply */
    BYTE    pad1;
    CARD16    sequenceNumber B16;
    CARD32    length B32;
    CARD32    fullscreenCovered B32;    /* if fullscreen is covered by windows, set to 1 otherwise 0 */
    CARD32    pad3 B32;
    CARD32    pad4 B32;
    CARD32    pad5 B32;
    CARD32    pad6 B32;
    CARD32    pad7 B32;        /* bytes 29-32 */
} xVIVEXTFULLScreenInfoReply;
#define    sz_xVIVEXTFULLScreenInfoReply 32


static XExtensionInfo _VIVEXT_info_data;
static XExtensionInfo *VIVEXT_info = &_VIVEXT_info_data;
static char *VIVEXT_extension_name = VIVEXTNAME;

#define VIVEXTCheckExtension(dpy, i) \
  XextSimpleCheckExtension(dpy, i, VIVEXT_extension_name)

/*****************************************************************************
 *                                                                           *
 *                           private utility routines                          *
 *                                                                           *
 *****************************************************************************/

static int close_display(Display *dpy, XExtCodes *extCodes);
static /* const */ XExtensionHooks VIVEXT_extension_hooks = {
    NULL,                                /* create_gc */
    NULL,                                /* copy_gc */
    NULL,                                /* flush_gc */
    NULL,                                /* free_gc */
    NULL,                                /* create_font */
    NULL,                                /* free_font */
    close_display,                        /* close_display */
    NULL,                                /* wire_to_event */
    NULL,                                /* event_to_wire */
    NULL,                                /* error */
    NULL,                                /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, VIVEXT_info,
                                   VIVEXT_extension_name,
                                   &VIVEXT_extension_hooks,
                                   0, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, VIVEXT_info)


static void
_LockPixmap(
    Display * dpy,
    PixmapList * Pix
    )
{
    int width;
    int height;
    int stride;
    uint32_t name;
    uint32_t node = 0;
    uint32_t address;
    void * memory;
    int status;

    /* Require these functions. */
    if (!_ImportVideoMemory ||
        !_ReleaseVideoMemory ||
        !_LockVideoMemory ||
        !_UnlockVideoMemory)
    {
        return;
    }

    XExtDisplayInfo *info = find_display (dpy);
    xVIVEXTDrawableInfoReply rep;
    xVIVEXTDrawableInfoReq *req;
    int extranums = 0;

    VIVEXTCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq(VIVEXTDrawableInfo, req);
    req->reqType = info->codes->major_opcode;
    req->vivEXTReqType = X_VIVEXTDrawableInfo;
    req->screen = DefaultScreen(dpy);
    req->drawable = Pix->pixmap;

    extranums = ( sizeof(xVIVEXTDrawableInfoReply) - 32 ) / 4;

    if (!_XReply(dpy, (xReply *)&rep, extranums , xFalse))
    {
        UnlockDisplay(dpy);
        SyncHandle();
        return;
    }

    width   = rep.drawableWidth;
    height  = rep.drawableHeight;
    stride  = rep.stride;
    name    = rep.nodeName;
    address = rep.phyAddress;

    UnlockDisplay(dpy);
    SyncHandle();
    status = _ImportVideoMemory(name, &node);

    if (status != 0)
    {
        return;
    }

    status = _LockVideoMemory(node, 0, 0, &address, &memory);

    if (status != 0)
    {
        /* Try again without cache. */
        status = _LockVideoMemory(node, 1, 0, &address, &memory);
    }

    if (status != 0)
    {
        _ReleaseVideoMemory(node);
        return;
    }

    if (width != Pix->width)
    {
        fprintf(stderr, "VDK: width mismatch: %d - %d\n", width, Pix->width);
    }

    if (height != Pix->height)
    {
        fprintf(stderr, "VDK: height mismatch: %d - %d\n", height, Pix->height);
    }

    if (rep.phyAddress != address)
    {
        fprintf(stderr, "VDK: address mismatch: %x - %x\n", (uint32_t)rep.phyAddress, address);
    }

    Pix->stride  = stride;
    Pix->memory  = memory;
    Pix->address = address;
    Pix->node    = node;
}

static void
_UnlockPixmap(
    Display * dpy,
    PixmapList * Pix
    )
{
    if (Pix->node)
    {
        _UnlockVideoMemory(Pix->node, 6, 0);
        _ReleaseVideoMemory(Pix->node);

        Pix->memory  = NULL;
        Pix->address = ~0U;
        Pix->node    = 0;
    }
}

#elif defined(X11_DRI3)

#include <X11/Xlibint.h>
#include <xcb/xcb.h>
#include <xcb/present.h>
#include <xcb/xcbext.h>
#include <xcb/dri3.h>
#include <xcb/present.h>
#include <xcb/sync.h>
#include <X11/Xlib-xcb.h>

static int create_fd_from_pixmap(xcb_connection_t *c, Pixmap pixmap, int *stride) {
    int *fd;
    xcb_dri3_buffer_from_pixmap_cookie_t cookie;
    xcb_dri3_buffer_from_pixmap_reply_t *reply;

    cookie = xcb_dri3_buffer_from_pixmap(c, pixmap);
    reply = xcb_dri3_buffer_from_pixmap_reply(c, cookie, NULL);
    if (!reply)
        return -1;

    if (reply->nfd != 1)
        return -1;

    *stride = reply->stride;
    fd = xcb_dri3_buffer_from_pixmap_reply_fds(c, reply);
    free(reply);
    return fd[0];
}

static void
_LockPixmap(
    Display * dpy,
    PixmapList * Pix
    )
{
    int stride;
    int pixmapfd;
    uint32_t node = 0;
    uint32_t address;
    void * memory;
    gcsUSER_MEMORY_DESC desc;
    int status;

    /* Require these functions. */
    if (!_WrapUserMemory ||
        !_ReleaseVideoMemory ||
        !_LockVideoMemory ||
        !_UnlockVideoMemory)
    {
        return;
    }

    pixmapfd = create_fd_from_pixmap(XGetXCBConnection(dpy), Pix->pixmap, &stride);
    xcb_flush(XGetXCBConnection(dpy));
    if (pixmapfd < 0)
    {
        return;
    }

    desc.flag = gcvALLOC_FLAG_DMABUF;
    desc.handle = pixmapfd;
    status = _WrapUserMemory(&desc, &node);
    close(pixmapfd);
    if (status != 0)
    {
        return;
    }

    status = _LockVideoMemory(node, 0, 0, &address, &memory);
    if (status != 0)
    {
        /* Try again without cache. */
        status = _LockVideoMemory(node, 1, 0, &address, &memory);
    }

    if (status != 0)
    {
        _ReleaseVideoMemory(node);
        return;
    }

    Pix->stride  = stride;
    Pix->memory  = memory;
    Pix->address = address;
    Pix->node    = node;
}

static void
_UnlockPixmap(
    Display * dpy,
    PixmapList * Pix
    )
{
    if (Pix->node)
    {
        _UnlockVideoMemory(Pix->node, 6, 0);
        _ReleaseVideoMemory(Pix->node);

        Pix->memory  = NULL;
        Pix->address = ~0U;
        Pix->node    = 0;
    }
}
#endif

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    /* Get default root window. */
    XImage *image;
    Window rootWindow;
    Pixmap pixmap = 0;
    PixmapList *pix;

    /* Test if we have a valid display data structure pointer. */
    if (Display == NULL)
    {
        return 0;
    }

    if ((Width <= 0) || (Height <= 0))
    {
        return 0;
    }

    rootWindow = DefaultRootWindow(Display);

    /* Check BitsPerPixel, only support 16bit and 32bit now. */
    switch (BitsPerPixel)
    {
    case 0:
        image = XGetImage(Display,
            rootWindow,
            0, 0, 1, 1, AllPlanes, ZPixmap);

        if (image == NULL)
        {
            /* Error. */
            return 0;
        }
        BitsPerPixel = image->bits_per_pixel;
        break;

    case 16:
    case 32:
        break;

    default:
        return 0;
    }

    /* Create native pixmap, ie X11 Pixmap. */
    pixmap = XCreatePixmap(Display, rootWindow,
                           Width, Height, BitsPerPixel);

    if (pixmap == 0)
    {
        return 0;
    }

    /* Flush command buffer. */
    XFlush(Display);

    /* Record it into pixmap list. */
    pix = (PixmapList *) malloc(sizeof (PixmapList));

    if (!pix)
    {
        XFreePixmap(Display, pixmap);
        XFlush(Display);
        return 0;
    }

    pix->pixmap       = pixmap;
    pix->width        = Width;
    pix->height       = Height;
    pix->bitsPerPixel = BitsPerPixel;

    pix->stride       = 0;
    pix->memory       = NULL;
    pix->address      = ~0U;
    pix->node         = 0;

    /* Add into pixmap linked list. */
    pthread_mutex_lock(&_vdk->pixmapMutex);

    pix->next        = _vdk->pixmapList;
    _vdk->pixmapList = pix;

    pthread_mutex_unlock(&_vdk->pixmapMutex);

    return pixmap;
}

VDKAPI int VDKLANG
vdkGetPixmapInfo(
    vdkPixmap Pixmap,
    int * Width,
    int * Height,
    int * BitsPerPixel,
    int * Stride,
    void ** Bits
    )
{
    Display * dpy = _vdk->display;
    PixmapList * pix = NULL;

    if (dpy == NULL || Pixmap == 0)
    {
        /* Pixmap is not a valid pixmap data structure pointer. */
        return 0;
    }

    pthread_mutex_lock(&_vdk->pixmapMutex);

    if (_vdk->pixmapList)
    {
        for (pix= _vdk->pixmapList; pix!= NULL; pix = pix->next)
        {
            if (pix->pixmap == Pixmap)
            {
                break;
            }
        }
    }

    pthread_mutex_unlock(&_vdk->pixmapMutex);

    if (!pix)
    {
        /* Not created by vdkCreatePixmap?? */
        return 0;
    }

    if (Stride != NULL || Bits != NULL)
    {
#if defined(EGL_API_DRI) || defined(X11_DRI3)
        if (pix->stride == 0 || pix->memory == NULL)
        {
            _LockPixmap(dpy, pix);
        }
#endif
        if (pix->stride == 0 || pix->memory == NULL)
        {
            /* Can not get stride or Bits from Pixmap. */
            fprintf(stderr, "%s: can not obtain pixmap bits\n", __func__);
            return 0;
        }

        if (Stride != NULL)
        {
            *Stride = pix->stride;
        }

        if (Bits != NULL)
        {
            *Bits = pix->memory;
        }
    }

    /* Set back values. */
    if (Width != NULL)
    {
        *Width = pix->width;
    }

    if (Height != NULL)
    {
        *Height = pix->height;
    }

    if (BitsPerPixel != NULL)
    {
        *BitsPerPixel = pix->bitsPerPixel;
    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    Display * dpy = _vdk->display;
    PixmapList * pix = NULL;

    pthread_mutex_lock(&_vdk->pixmapMutex);

    if (_vdk->pixmapList)
    {
        PixmapList * prev;

        pix = _vdk->pixmapList;

        for (prev = pix; pix != NULL; pix = pix->next)
        {
            if (pix->pixmap == Pixmap)
            {
                break;
            }

            prev = pix;
        }

        if (pix)
        {
            if (pix == _vdk->pixmapList)
            {
                _vdk->pixmapList = pix->next;
            }
            else
            {
                prev->next = pix->next;
            }
#if defined(EGL_API_DRI) || defined(X11_DRI3)
            if (pix->stride != 0 || pix->memory != NULL)
            {
                _UnlockPixmap(dpy, pix);
            }
#endif
            free(pix);
        }
    }

    pthread_mutex_unlock(&_vdk->pixmapMutex);

    if (!pix)
    {
        /* Shall we return or just ignore? */
        fprintf(stderr, "%s: Warning: may destroy invalid pixmap\n", __func__);
    }

    if (dpy != NULL && Pixmap != 0)
    {
        XFreePixmap(_vdk->display, Pixmap);
        XFlush(dpy);
    }
}

/*******************************************************************************
** ClientBuffers. **************************************************************
*/

VDKAPI vdkClientBuffer VDKLANG
vdkCreateClientBuffer(
    int Width,
    int Height,
    int Format,
    int Type
    )
{
    return NULL;
}

VDKAPI int VDKLANG
vdkGetClientBufferInfo(
    vdkClientBuffer ClientBuffer,
    int * Width,
    int * Height,
    int * Stride,
    void ** Bits
    )
{
    return 0;
}

VDKAPI int VDKLANG
vdkDestroyClientBuffer(
    vdkClientBuffer ClientBuffer
    )
{
    return 0;
}
