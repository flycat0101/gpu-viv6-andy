/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#include "gc_vdk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>

#include <binder/IServiceManager.h>

#include <cutils/properties.h>

#include <utils/Log.h>
#include <ui/PixelFormat.h>
#include <ui/DisplayInfo.h>

#if ANDROID_SDK_VERSION < 24
# include <ui/FramebufferNativeWindow.h>
#endif

#if ANDROID_SDK_VERSION <= 7
# include <surfaceflinger/SurfaceComposerClient.h>
# include <ui/ISurfaceFlingerClient.h>

#elif ANDROID_SDK_VERSION == 8
# include <surfaceflinger/SurfaceComposerClient.h>
# include <surfaceflinger/ISurfaceFlingerClient.h>

#elif ANDROID_SDK_VERSION < 16
# include <surfaceflinger/SurfaceComposerClient.h>
# include <surfaceflinger/ISurfaceComposerClient.h>

#elif ANDROID_SDK_VERSION == 16
# include <gui/SurfaceComposerClient.h>
# include <gui/ISurfaceComposerClient.h>

#elif ANDROID_SDK_VERSION == 17
# include <gui/ISurfaceComposer.h>
# include <gui/Surface.h>
# include <gui/SurfaceComposerClient.h>

#elif ANDROID_SDK_VERSION == 18
# include <gui/ISurfaceComposer.h>
# include <gui/Surface.h>
# include <gui/SurfaceComposerClient.h>

#else
# include <gui/ISurfaceComposer.h>
# include <gui/Surface.h>
# include <gui/SurfaceComposerClient.h>

#endif

#if ANDROID_SDK_VERSION > 23
# warning "Not verified for ANDROID_SDK_VERSION > 23"
#endif

using namespace android;

class DisplayContainer
{
public:
    DisplayContainer(int index,
        void * displayId, int width, int height, int format);

    ~DisplayContainer() {}

    /* Check if is container of specified native display type. */
    bool isContainerOf(void * displayId)
    {
        return (mDisplayId == displayId);
    }

    /* Get display info. */
    bool getDisplayInfo(int * width, int * height, int * format);

private:
    int             mIndex;
    int             mWidth;
    int             mHeight;
    int             mStride;
    int             mFormat;
    void *          mDisplayId;
};

DisplayContainer::DisplayContainer(int index,
        void * displayId, int width, int height, int format):
        mIndex(index), mWidth(width), mHeight(height), mFormat(format),
        mDisplayId(displayId)
{
}

bool DisplayContainer::getDisplayInfo(int *width, int *height, int *format)
{
    if (width)
        *width = mWidth;

    if (height)
        *height = mHeight;

    if (format)
        *format = mFormat;

    return true;
}


class WindowContainer
{
public:
    WindowContainer(sp<SurfaceControl> control, sp<Surface> surface,
            int x, int y, int width, int height, int format);
    ~WindowContainer();

    /* Check if is container of specified native display type. */
    bool isContainerOf(ANativeWindow *win)
    {
        return (static_cast<ANativeWindow *>(mSurface.get()) == win);
    }

    /* Window Info. */
    void getWindowInfo(int * x, int * y, int * width, int * height, int * format);

private:
    int                mX, mY;
    int                mWidth;
    int                mHeight;
    int                mFormat;
    sp<SurfaceControl> mControl;
    sp<Surface>        mSurface;
};

/* Window */
WindowContainer::WindowContainer(sp<SurfaceControl> control, sp<Surface> surface,
            int x, int y, int width, int height, int format):
    mX(x), mY(y), mWidth(width), mHeight(height), mFormat(format),
    mControl(control), mSurface(surface)
{
}

WindowContainer::~WindowContainer()
{
    mControl = 0;
    mSurface = 0;
}

void WindowContainer::getWindowInfo(int * x, int * y, int * width, int * height, int * format)
{
    if (x)
        *x = mX;

    if (y)
        *y = mY;

    if (width)
        *width = mWidth;

    if (height)
        *height = mHeight;

    if (format)
        *format = mFormat;
}


class PixmapContainer
{
public:
    PixmapContainer() {}
    ~PixmapContainer() {}

private:
    egl_native_pixmap_t * mPixmap;
};


class Manager
{
public:
    static Manager * getInstance();
    static void freeInstance();

    /* Display */
    void * getDisplay(int id);
    void destroyDisplay(void * displayId);

    bool getDisplayInfo(void * displayId,
            int * width, int * height, int * format);

    /* Window */
    ANativeWindow * createWindow(const char * name, int x, int y,
            int width, int height, int format);

    void destroyWindow(ANativeWindow * win);

    bool getWindowInfo(ANativeWindow * win, int * x, int * y,
            int * width, int * height, int * format);

    /* Pixmap */
    egl_native_pixmap_t * createPixmap(int width, int height, int format);
    void destroyPixmap(egl_native_pixmap_t * pixmap);

private:
    Manager();
    ~Manager();

    /* Containers. */
    Vector<DisplayContainer *>  mDisplayList;
    Vector<WindowContainer *>   mWindowList;
    Vector<PixmapContainer *>   mPixmapList;
    WindowContainer *           mFbWindow;
    Mutex                       mMutex;

    /* false for framebuffer mode(linux mode). */
    /* true for composition mode(android mode). */
    bool                        mCompositionMode;
    sp<SurfaceComposerClient>   mSession;
#if ANDROID_SDK_VERSION < 24
    sp<FramebufferNativeWindow> mFramebuffer;
#endif

    static Manager *            mSelf;
};

Manager * Manager::mSelf(NULL);

Manager* Manager::getInstance()
{
    if (!mSelf)
    {
        mSelf = new Manager();
    }

    return mSelf;
}

void Manager::freeInstance()
{
    if (mSelf)
    {
        delete mSelf;
        mSelf = NULL;
    }
}

Manager::Manager()
{
    char value[PROPERTY_VALUE_MAX];
    status_t err = NO_ERROR;

    /* Initialize the private data structure. */
    property_get("vdk.window_mode", value, "0");

    do
    {
        /* Safe mode is framebuffer mode. */
        mCompositionMode = false;

        if (value[0] != '0')
        {
            /* Set framebuffer mode. */
            break;
        }

        /* Try composition mode. */
        sp<IBinder> binder;
        sp<IServiceManager> sm = defaultServiceManager();

        /* 1. Find surfaceflinger service. */
        binder = sm->checkService(String16("SurfaceFlinger"));
        if (binder == 0)
        {
            /* Fail back to framebuffer mode. */
            break;
        }

        /* 2. Create session. */
        mSession = new SurfaceComposerClient();

        /* Init check. */
        err = mSession->initCheck();

        if (err == NO_INIT)
        {
            /* Composition not started. */
            mSession->dispose();
            mSession = NULL;


            printf("Try \'Composition\' mode failed: "
                   "Composition not started.\n");

            /* Fail back to framebuffer mode. */
            break;
        }

        /* Set composition mode success. */
        mCompositionMode = true;
    }
    while (0);

    if (mCompositionMode)
    {
        printf("Running in \'Composition\' mode\n");
        printf("Composition is enabled for this native window\n");
    }
    else
    {
#if ANDROID_SDK_VERSION < 24
        printf("Running in \'FramebufferNativeWindow\' mode\n");
        printf("Directly to framebuffer, bypass composition\n");

        /* Get display surface immediately if using mode 1. */
        mFramebuffer = static_cast<FramebufferNativeWindow *>(android_createDisplaySurface());
#else
        printf("No \'FramebufferNativeWindow\' mode in this android version\n");
        printf("Force \'Composition\' mode\n");
        mCompositionMode = true;
#endif
    }
}

Manager::~Manager()
{
    /* Window. */
    for (size_t i = 0; i < mWindowList.size(); i++)
    {
        WindowContainer *w = mWindowList[i];
        delete w;
        mWindowList.replaceAt(NULL, i);
    }
    mWindowList.clear();

    /* Pixmap. */
    for (size_t i = 0; i < mPixmapList.size(); i++)
    {
        PixmapContainer *p = mPixmapList[i];
        delete p;
        mPixmapList.replaceAt(NULL, i);
    }
    mPixmapList.clear();

    /* Display. */
    for (size_t i = 0; i < mDisplayList.size(); i++)
    {
        DisplayContainer *d = mDisplayList[i];
        delete d;
        mDisplayList.replaceAt(NULL, i);
    }
    mDisplayList.clear();

    if (mFbWindow)
    {
        delete mFbWindow;
        mFbWindow = NULL;
    }

    if (mCompositionMode)
    {
        /* Composition mode. */
        mSession->dispose();
        mSession = 0;
    }
    else
    {
#if ANDROID_SDK_VERSION < 24
        /* Framebuffer mode. */
        mFramebuffer = 0;
#endif
    }
}

void * Manager::getDisplay(int index)
{
    DisplayContainer *d;
    (void) index;
    void * displayId = /* EGL_DEFAULT_DISPLAY */ (void *) 0;

    int width  = 1;
    int height = 1;
    int format = HAL_PIXEL_FORMAT_RGBX_8888;

    for (size_t i = 0; i < mDisplayList.size(); i++)
    {
        d = mDisplayList[i];

        if (d->isContainerOf(displayId))
        {
            return displayId;
        }
    }

    if (mCompositionMode)
    {
        /* Composition mode. */
        DisplayInfo info;
        status_t status;

#if (ANDROID_SDK_VERSION >= 17)
        sp<IBinder> token(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
        status = SurfaceComposerClient::getDisplayInfo(token, &info);
#else
        status = mSession->getDisplayInfo(0, &info);
#endif
        if (status != NO_ERROR)
        {
            return NULL;
        }

        width    = info.w;
        height   = info.h;
#if (ANDROID_SDK_VERSION < 19)
        format   = info.pixelFormatInfo.format;
#else
        format   = HAL_PIXEL_FORMAT_RGBX_8888;
#endif
    }
    else
    {
#if ANDROID_SDK_VERSION < 24
        /* Framebuffer mode. */
        ANativeWindow * fb =
            static_cast<ANativeWindow *>(mFramebuffer.get());

        fb->query(fb, NATIVE_WINDOW_WIDTH,  &width);
        fb->query(fb, NATIVE_WINDOW_HEIGHT, &height);
        fb->query(fb, NATIVE_WINDOW_FORMAT, &format);
#else
        width  = 0;
        height = 0;
        format = 0;
#endif
    }

    {
        Mutex::Autolock lock(mMutex);
        d = new DisplayContainer(index, displayId, width, height, format);
        mDisplayList.add(d);
    }

    return displayId;
}

void Manager::destroyDisplay(void * displayId)
{
    for (size_t i = 0; i < mDisplayList.size(); i++)
    {
        DisplayContainer * d = mDisplayList[i];

        if (d->isContainerOf(displayId))
        {
            Mutex::Autolock lock(mMutex);
            mDisplayList.removeAt(i);
            delete d;
            break;
        }
    }
}

bool Manager::getDisplayInfo(void * displayId, int *width, int *height, int *format)
{
    for (size_t i = 0; i < mDisplayList.size(); i++)
    {
        DisplayContainer * d = mDisplayList[i];

        if (d->isContainerOf(displayId))
        {
            return d->getDisplayInfo(width, height, format);
        }
    }

    /* Display not found. */
    return false;
}

ANativeWindow * Manager::createWindow(const char *name, int x, int y, int width, int height, int format)
{
    WindowContainer *w;
    sp<SurfaceControl> control;
    sp<Surface> surface;

    if (mCompositionMode)
    {
        /* Create surface control. */
#if ANDROID_SDK_VERSION < 14
        control = mSession->createSurface(
                getpid(), 0, width, height, format);
#elif ANDROID_SDK_VERSION <= 16
        control = mSession->createSurface(
                0, width, height, format);
#else
        control = mSession->createSurface(
                String8(name), width, height, format, 0);
#endif

        /* Set Z order and position. */
#if ANDROID_SDK_VERSION < 14
        mSession->openTransaction();
#else
        SurfaceComposerClient::openGlobalTransaction();
#endif

        control->setLayer(0x40000000);
        control->setPosition(x, y);

#if ANDROID_SDK_VERSION < 14
        mSession->closeTransaction();
#else
        SurfaceComposerClient::closeGlobalTransaction();
#endif
        /* Get surface. */
        surface = control->getSurface();

        {
            Mutex::Autolock lock(mMutex);
            w = new WindowContainer(control, surface, x, y, width, height, format);
            mWindowList.add(w);
        }

        return static_cast<ANativeWindow *>(surface.get());
    }
    else
    {
#if ANDROID_SDK_VERSION < 24
        Mutex::Autolock lock(mMutex);
        ANativeWindow *win = static_cast<ANativeWindow *>(mFramebuffer.get());

        if (!mFbWindow)
        {
            win->query(win, NATIVE_WINDOW_WIDTH,  &width);
            win->query(win, NATIVE_WINDOW_HEIGHT, &height);
            win->query(win, NATIVE_WINDOW_FORMAT, &format);
            w = new WindowContainer(control, surface, 0, 0, width, height, format);

            mFbWindow = w;
        }

        return win;
#else
        return NULL;
#endif
    }
}

void Manager::destroyWindow(ANativeWindow *win)
{
    Mutex::Autolock lock(mMutex);

    if (mCompositionMode)
    {
        for (size_t i = 0; i < mWindowList.size(); i++)
        {
            WindowContainer *w = mWindowList[i];

            if (w->isContainerOf(win))
            {
                mWindowList.removeAt(i);
                delete w;
                break;
            }
        }
    }
    else
    {
        delete mFbWindow;
        mFbWindow = NULL;
    }
}

bool Manager::getWindowInfo(ANativeWindow *win,
        int *x, int *y, int *width, int *height, int *format)
{
    WindowContainer * w = NULL;

    if (mCompositionMode)
    {
        for (size_t i = 0; i < mWindowList.size(); i++)
        {
            if (mWindowList[i]->isContainerOf(win))
            {
                w = mWindowList[i];
                break;
            }
        }
    }
    else
    {
        w = mFbWindow;
    }

    if (!w)
        return false;

    w->getWindowInfo(x, y, width, height, format);
    return true;
}



class ManagerFinalizer
{
    public:
    ManagerFinalizer() {}
    ~ManagerFinalizer() { Manager::freeInstance(); }
};

/* dtor will be automatically called when exit. */
static ManagerFinalizer __managerFinalizer;



#include <EGL/egl.h>
#include <EGL/eglext.h>

/*******************************************************************************
** Default keyboard map. *******************************************************
*/

static void *_egl;

static __attribute__((destructor)) void
_onexit(
    void
    )
{
    if (_egl)
    {
        dlclose(_egl);
        _egl = NULL;
    }

    Manager::freeInstance();
}

/*******************************************************************************
** Initialization. *************************************************************
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    if (_egl == NULL)
    {
        _egl = dlopen("libEGL.so", RTLD_NOW);
    }

    atexit(_onexit);

    return (vdkPrivate) Manager::getInstance();
}

VDKAPI void VDKLANG
vdkExit(
    vdkPrivate Private
    )
{
    if (_egl)
    {
        dlclose(_egl);
        _egl = NULL;
    }

    Manager::freeInstance();
}

/*******************************************************************************
** Display. ********************************************************************
*/

VDKAPI vdkDisplay VDKLANG
vdkGetDisplay(
    vdkPrivate Private
    )
{
    Manager * manager = Manager::getInstance();
    return (vdkDisplay) manager->getDisplay(0);
}

VDKAPI vdkDisplay VDKLANG
vdkGetDisplayByIndex(
    vdkPrivate Private,
    int DisplayIndex
    )
{
    return vdkGetDisplay(Private);
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
    int width;
    int height;
    int format;
    int bpp;
    bool ret;

    Manager * manager = Manager::getInstance();

    ret = manager->getDisplayInfo((void *) Display, &width, &height, &format);

    if (!ret)
    {
        return 0;
    }

    /* Obtain format information. */
    switch (format)
    {
    case PIXEL_FORMAT_RGBA_8888:
    case PIXEL_FORMAT_RGBX_8888:
    case PIXEL_FORMAT_BGRA_8888:
        bpp = 32;
        break;

    case PIXEL_FORMAT_RGB_888:
        bpp = 24;
        break;

    case PIXEL_FORMAT_RGB_565:
        bpp = 16;
        break;

    default:
        return 0;
    }

    if (Width != NULL)
    {
        *Width = width;
    }

    if (Height != NULL)
    {
        *Height = height;
    }

    if (Physical != NULL)
    {
        *Physical = ~0U;
    }

    if (Stride != NULL)
    {
        *Stride = width * bpp / 8;
    }

    if (BitsPerPixel != NULL)
    {
        *BitsPerPixel = bpp;
    }

    /* Success. */
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    Manager * manager = Manager::getInstance();
    manager->destroyDisplay((void *) Display);
}

/*******************************************************************************
** Windows. ********************************************************************
*/

VDKAPI vdkWindow VDKLANG
vdkCreateWindow(
    vdkDisplay Display,
    int X,
    int Y,
    int Width,
    int Height
    )
{
    int xres, yres;
    int format;
    int ret;
    char name[64] = "vdk window";
    int fd;

    Manager * manager = Manager::getInstance();

    ret = manager->getDisplayInfo((void *) 0, &xres, &yres, &format);

    switch (format)
    {
    case HAL_PIXEL_FORMAT_RGB_565:
        break;
    default:
        format = HAL_PIXEL_FORMAT_RGBX_8888;
        break;
    }

    if (!ret)
    {
        return NULL;
    }

    if (Width == 0)
    {
        Width = xres;
    }

    if (Height == 0)
    {
        Height = yres;
    }

    if (X == -1)
    {
        X = ((xres - Width) / 2) & ~7;
    }

    if (Y == -1)
    {
        Y = ((yres - Height) / 2) & ~7;
    }

    if (X < 0) X = 0;
    if (Y < 0) Y = 0;
    if (X + Width  > xres)  Width  = xres - X;
    if (Y + Height > yres)  Height = yres - Y;

    fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd > 0)
    {
        ssize_t len = read(fd, name, 64);

        if (len > 0)
        {
            name[len] = '\0';
        }

        close(fd);
    }

    return manager->createWindow(name, X, Y, Width, Height, format);
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
    int x, y;
    int width, height;
    int format;
    int ret;

    Manager * manager = Manager::getInstance();

    ret = manager->getWindowInfo((ANativeWindow *) Window, &x, &y, &width, &height, &format);

    if (!ret)
        return 0;

    if (X != NULL)
    {
        *X = x;
    }

    if (Y != NULL)
    {
        *Y = y;
    }

    if (Width != NULL)
    {
        *Width = width;
    }

    if (Height != NULL)
    {
        *Height = height;
    }

    if (BitsPerPixel != NULL)
    {
        switch (format)
        {
        case PIXEL_FORMAT_RGBA_8888:
        case PIXEL_FORMAT_RGBX_8888:
        case PIXEL_FORMAT_BGRA_8888:
            *BitsPerPixel = 32;
            break;

        case PIXEL_FORMAT_RGB_888:
            *BitsPerPixel = 24;
            break;

        case PIXEL_FORMAT_RGB_565:
        case PIXEL_FORMAT_RGBA_5551:
        case PIXEL_FORMAT_RGBA_4444:
            *BitsPerPixel = 16;
            break;

        default:
            return 0;
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
    Manager * manager = Manager::getInstance();
    manager->destroyWindow((ANativeWindow *) Window);
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    (void) Window;

    /* Not support. */
    return 1;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    (void) Window;

    /* Not support. */
    return 1;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
    (void) Window;
    (void) Title;
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
    if (_egl == NULL)
    {
        return NULL;
    }

    return (EGL_ADDRESS) dlsym(_egl, Function);
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

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    printf("Warning: Pixmap is not supported on android.\n");
    return NULL;
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
    return 0;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    printf("Warning: Pixmap is not supported on android.\n");
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
    /* Error. */
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

