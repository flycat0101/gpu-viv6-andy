/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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


/*
 *
 * FB graphics API.
 *
 */

#if defined(LINUX) && defined(FBDriver)

#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <termio.h>
#include <unistd.h>
#include <gc_sdk.h>
#include "gc_sdk_internal.h"
#include "gc_sdk_internal_linux.h"

#define Output(...)
#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#define MIN(a, b) ((a) > (b)) ? (b) : (a)

/******************************************************************************\
************************* Keyboard Specific Information. *************************
\******************************************************************************/
static struct termios old_setting, local_setting;
static unsigned char peek_ch = -1;

static int InitKB()
{
    if (tcgetattr(0, &old_setting) == -1)
        return -1;

    local_setting = old_setting;
    local_setting.c_lflag &= ~ICANON;
    local_setting.c_lflag &= ~ISIG;
    local_setting.c_lflag &= ~ECHO;
    local_setting.c_cc[VMIN] = 1;
    local_setting.c_cc[VTIME] = 1;

    return tcsetattr(0, TCSANOW, &local_setting);
}

static int CloseKB()
{
    return tcsetattr(0, TCSANOW, &old_setting);
}

static int KBHit()
{
    int nread;
    unsigned char c;

    local_setting.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &local_setting);
    nread = read(0, &c, 1);
    local_setting.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &local_setting);

    if (nread == 1)
    {
        peek_ch = c;
        return 1;
    }
    else
    {
        return 0;
    }
}

static unsigned char Readch()
{
    unsigned char c;

    if (peek_ch != -1)
    {
        c = peek_ch;
        peek_ch = -1;
    }
    else
    {
        read(0, &c, 1);
    }

    return c;
}

/******************************************************************************\
**************************** FB Specific Information. ***************************
\******************************************************************************/

struct _gcsVISUAL
{
    /* Include the common visual structure. */
    struct _gcsCOMMONVISUAL common;

    /* Handle of frame buffer device. */
    int fdFB;

    /* Default screen associated with the display. */
    struct Screen
    {
        gctINT vSize;
        gctINT hSize;
        gctINT bits_per_pixel;
        gctUINT32 phaysAddrFB;
        gctUINT32 stride;
        gctUINT32 screenSize;
        unsigned char *pBuffer; /* Buffer of frame buffer. */
    }screen;

    /* main window. */
    struct Window
    {
        gctINT x;
        gctINT y;
        gctINT width;
        gctINT height;
    }MainWindow;

    /* main image */
    struct Image
    {
        gctUINT stride;
        gctUINT size;
        gctPOINTER bits;
    }MainImage;

    /* Timer flag. */
    gctBOOL timerExpired;
};


/******************************************************************************\
****************************** Support Functions ******************************
\******************************************************************************/

static void
_DestroyMainWindowImage(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have an image to destroy? */
    if (visual->MainImage.bits != gcvNULL)
    {
        /* Destroy the image. */
        free(visual->MainImage.bits);
        visual->MainImage.bits = gcvNULL;
        visual->MainImage.size = 0;
        visual->MainImage.stride = 0;
    }
}

static void
_DrawMainWindowImage(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have an image to draw? */
    if (visual->MainImage.bits != gcvNULL)
    {
        gctINT n;
        gctINT pixelSize = visual->screen.bits_per_pixel/8;
        gctINT windowX = visual->MainWindow.x;
        gctINT windowY = visual->MainWindow.y;
        gctINT windowWidth = MIN(visual->MainWindow.width, visual->common.mainWindowImageWidth);
        gctINT windowHeight = MIN(visual->MainWindow.height, visual->common.mainWindowImageHeight);

        gctINT x = MAX(windowX, 0);
        gctINT y = MAX(windowY, 0);
        gctINT w = MIN(windowX + windowWidth, visual->screen.hSize) - x;
        gctINT h = MIN(windowY + windowHeight, visual->screen.vSize) - y;

        if (w <= 0 || h <= 0)
            return;

        windowX = x - windowX;
        windowY = y - windowY;

        for (n = 0; n < h; n++)
        {
            memcpy(visual->screen.pBuffer + n * visual->screen.stride + x * pixelSize,
                (unsigned char*)visual->MainImage.bits + (windowY + n) * visual->MainImage.stride + windowX * pixelSize,
                w * pixelSize);
        }
    }
}

static void
_TimerHandler(
    int Signal
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Set the time out flag. */
    visual->timerExpired = gcvTRUE;
}


/******************************************************************************\
****************************** API Implementation *****************************
\******************************************************************************/

/*******************************************************************************
**
**    ivdkGetVisual
**
**    Query a pointer to the visual object.
**
**    INPUT:
**
**        Nothing.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gcsVISUAL_PTR
**            Pointer to the visual object.
*/

gcsVISUAL_PTR
ivdkGetVisual(
    void
    )
{
    /* Define the visual object. */
    static struct _gcsVISUAL visual = {{0}};

    /* Return a pointer to the object. */
    return &visual;
}


/*******************************************************************************
**
**  vdkInitVisual
**
**  Initialize all prerequisites necessary for normal functioning of the visual
**    or window system. Should be called first before any other call to a visual
**    API.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
**
**    RETURN:
**
**        gctBOOL
**            gcvTRUE if the function succeedes.
*/
/* Frame buffer device. */
#define DEVFB "/dev/fb0"

gctBOOL
vdkInitVisual(
    void
    )
{
    gctBOOL result;
    char *devfb;

    do
    {
        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();
        struct fb_fix_screeninfo fInfo;
        struct fb_var_screeninfo vInfo;

        /* Init Keyboard */
        if (InitKB() == -1)
        {
            gcmFATAL("Failed to initialize keyboard.");
            result = gcvFALSE;
            break;
        }
        /* Create main window mutex. */
        visual->common.visualMutex = vdkCreateMutex();

        if (visual->common.visualMutex == gcvNULL)
        {
            gcmFATAL("Failed to create a visual mutex.");
            result = gcvFALSE;
            break;
        }

        /* Open the framebuffer device. */
        visual->fdFB = -1;

        devfb = getenv("VDK_FRAMEBUFFER");
        if (devfb != NULL)
        {
            visual->fdFB = open(devfb, O_RDWR);
        }

        if (visual->fdFB < 0)
        {
            visual->fdFB = open(DEVFB, O_RDWR);

            if (visual->fdFB < 0)
            {
                Output("*ERROR* Cannot open framebuffer device.\n");
                return gcvFALSE;
            }
        }

        /* Get fixed framebuffer information. */
        if (ioctl(visual->fdFB, FBIOGET_FSCREENINFO, &fInfo) < 0)
        {
            Output("*ERROR* Reading fixed framebuffer information.\n");
            return gcvFALSE;
        }

        visual->screen.phaysAddrFB = fInfo.smem_start;
        visual->screen.stride = fInfo.line_length;

        /* Get variable framebuffer information. */
        if (ioctl(visual->fdFB, FBIOGET_VSCREENINFO, &vInfo) < 0)
        {
            Output("*ERROR* Reading variable framebuffer information.\n");
            return gcvFALSE;
        }

        visual->screen.hSize = vInfo.xres;
        visual->screen.vSize = vInfo.yres;
        visual->screen.bits_per_pixel = vInfo.bits_per_pixel;

        /* Init desktop size. */
        visual->common.desktopWidth  = visual->screen.hSize;
        visual->common.desktopHeight = visual->screen.vSize;

        visual->common.mainWindowDepth = visual->screen.bits_per_pixel;

        /* Figure out the size of the screen in bytes. */
        visual->screen.screenSize = vInfo.xres * vInfo.yres * vInfo.bits_per_pixel / 8;

        Output("vSize:%d, hSize:%d, BPP:%d\n", vInfo.xres, vInfo.yres, vInfo.bits_per_pixel);

        /* Map the framebuffer device to memory. */
        visual->screen.pBuffer = (unsigned char *)mmap(0, visual->screen.screenSize, PROT_READ | PROT_WRITE, MAP_SHARED, visual->fdFB, 0);
        if (visual->screen.pBuffer == MAP_FAILED)
        {
            Output("*ERROR* Failed to map framebuffer device to memory.\n");
            return gcvFALSE;
        }

        /* Success. */
        result = gcvTRUE;
    }
    while (gcvFALSE);

    /* Return result. */
    return result;
}

/*******************************************************************************
**
**  vdkCleanupVisual
**
**  Should be paired with vdkInitVisual and called before the application
**    terminates.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**      Nothing.
**
**    RETURN:
**
**        Nothing.
**
*/

void
vdkCleanupVisual(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Destroy the main window. */
    vdkDestroyMainWindow();

    /* Acquire the visual mutex. */
    ivdkLockVisual();

    /* Close connection if open. */
    if (visual->fdFB != 0)
    {
        munmap((void *)(visual->fdFB), (size_t)(visual->screen.screenSize));
        close(visual->fdFB);
        visual->screen.bits_per_pixel = 0;
        visual->screen.hSize = 0;
        visual->screen.vSize = 0;
        visual->screen.pBuffer = gcvNULL;
        visual->screen.phaysAddrFB = 0;
        visual->screen.stride = 0;
        visual->screen.screenSize = 0;
        visual->fdFB = 0;
    }

    /* Release the visual mutex. */
    ivdkUnlockVisual();

    /* Destroy the mutex. */
    if (visual->common.visualMutex != gcvNULL)
    {
        vdkCloseMutex(visual->common.visualMutex);
        visual->common.visualMutex = gcvNULL;
    }

    /* Close Keyboard */
    CloseKB();

}

/*******************************************************************************
**
**  vdkCreateMainWindow
**
**  Create the main application window.
**
**  INPUT:
**
**        gctINT X, Y
**            Origin of the client areat of the window.
**
**        gctUINT Width, Height
**            Size of the client area.
**
**        gctSTRING Title
**            Title bar text.
**
**        gctPOINTER EventArgument
**            Argument to be passed to event handler functions.
**
**        gctKEYBOARDEVENT KeyboardEvent
**            Pointer to a key press handler. Set to gcvNULL if not needed.
**
**        gctMOUSEPRESSEVENT MousePressEvent
**            Pointer to a mouse click handler. Set to gcvNULL if not needed.
**
**        gctMOUSEMOVEEVENT MouseMoveEvent
**            Pointer to a mouse move handler. Set to gcvNULL if not needed.
**
**        gctTIMERFUNC TimerEvent
**            Pointer to a timer routine. Set to gcvNULL if not needed.
**
**        gctUINT TimerDelay
**            Timer routine interval in seconds.
**
**  OUTPUT:
**
**      Nothing.
**
**    RETURN:
**
**        gctBOOL
**            gcvTRUE if the functuion succeedes.
**
*/

gctBOOL
vdkCreateMainWindow(
    IN gctINT X,
    IN gctINT Y,
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctSTRING Title,
    IN gctPOINTER EventArgument,
    IN gctKEYBOARDEVENT KeyboardEvent,
    IN gctMOUSEPRESSEVENT MousePressEvent,
    IN gctMOUSEMOVEEVENT MouseMoveEvent,
    IN gctTIMERFUNC TimerEvent,
    IN gctUINT TimerDelay
    )
{
    gctBOOL result;

    /* Destroy the main window if previously created. */
    vdkDestroyMainWindow();

    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Message handlers. */
        visual->common.eventArgument   = EventArgument;
        visual->common.keyboardEvent   = KeyboardEvent;
        visual->common.mousePressEvent = MousePressEvent;
        visual->common.mouseMoveEvent  = MouseMoveEvent;
        visual->common.timerEvent      = TimerEvent;
        visual->common.timerDelay      = TimerDelay;

        /* Handle the auto placement option. */
        if ((Width == 0) || (Width > visual->common.desktopWidth))
        {
            visual->MainWindow.width = visual->common.desktopWidth;
        }
        else
        {
            visual->MainWindow.width = Width;
        }

        if ((Height == 0) || (Height > visual->common.desktopHeight))
        {
            visual->MainWindow.height = visual->common.desktopHeight;
        }
        else
        {
            visual->MainWindow.height = Height;
        }

        if (X < 0)
        {
            if (visual->common.desktopWidth > Width)
                visual->MainWindow.x = (visual->common.desktopWidth - Width) / 2;
            else
                visual->MainWindow.x = 0;
        }
        else
        {
            visual->MainWindow.x = X;
        }

        if (Y < 0)
        {
            if (visual->common.desktopHeight > Height)
                visual->MainWindow.y = (visual->common.desktopHeight - Height) / 2;
            else
                visual->MainWindow.y = 0;
        }
        else
        {
            visual->MainWindow.y = Y;
        }

        /* Success. */
        result = gcvTRUE;
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();

    /* Return result. */
    return result;
}

/*******************************************************************************
**
**  vdkDestroyMainWindow
**
**  Destroy the main application window.
**
**  INPUT:
**
**        Nothing.
**
**  OUTPUT:
**
*        Nothing.
**
**    RETURN:
**
**        Nothing.
**
*/

void
vdkDestroyMainWindow(
    void
    )
{
    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Destroy the image. */
        _DestroyMainWindowImage();

        /* Destroy the window. */
        visual->MainWindow.x = 0;
        visual->MainWindow.y = 0;
        visual->MainWindow.width = 0;
        visual->MainWindow.height = 0;
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();
}

/*******************************************************************************
**
**  vdkGetDisplayContext
**
**  Query the display device context.
**
**  INPUT:
**
**        Nothing.
**
**  OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gctHANDLE
**            Display device context handle.
**
*/

gctHANDLE
vdkGetDisplayContext(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    return (gctHANDLE) visual->fdFB;
}

/*******************************************************************************
**
**  vdkReleaseDisplayContext
**
**  Release the display device context previously returned by
**    vdkGetDisplayContext.
**
**  INPUT:
**
**        gctHANDLE Context
**            Handle to the display device context.
**
**  OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        Nothing.
**
*/

void
vdkReleaseDisplayContext(
    gctHANDLE Context
    )
{
}

/*******************************************************************************
**
**  vdkGetMainWindowHandle
**
**  Query the handle of the main window.
**
**  INPUT:
**
**        Nothing.
**
**  OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        gctHANDLE
**            Display device context handle.
**
*/

gctHANDLE
vdkGetMainWindowHandle(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    return (gctHANDLE) &visual->MainWindow;
}

/*******************************************************************************
**
**  vdkGetMainWindowSize
**
**  Get the size of the client area of the main window.
**
**  INPUT:
**
**      Nothing.
**
**  OUTPUT:
**
**        gctUINT* Width
**        gctUINT* Height
**            Pointers to the area size.
**
*/

void
vdkGetMainWindowSize(
    IN OUT gctUINT* Width,
    IN OUT gctUINT* Height
    )
{
    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Set the output. */
        if (Width != gcvNULL)
        {
            *Width = visual->MainWindow.width;
        }

        if (Height != gcvNULL)
        {
            *Height = visual->MainWindow.height;
        }
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();
}

/*******************************************************************************
**
**  vdkGetMainWindowColorBits
**
**  Query number of bits per color component for the main window.
**
**  INPUT:
**
**        Nothing.
**
**  OUTPUT:
**
**        gctUINT* RedCount
**        gctUINT* GreenCount
**        gctUINT* BlueCount
**            Image depth in bits per pixel per color component.
**
*/

void
vdkGetMainWindowColorBits(
    IN OUT gctUINT* RedCount,
    IN OUT gctUINT* GreenCount,
    IN OUT gctUINT* BlueCount
    )
{
    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        gctUINT r = 0;
        gctUINT g = 0;
        gctUINT b = 0;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Dispatch based on the window depth. */
        switch (visual->common.mainWindowDepth)
        {
        case 16:
            r = 5;
            g = 6;
            b = 5;
            break;

        case 32:
            r = 8;
            g = 8;
            b = 8;
            break;
        }

        /* Set the output. */
        if (RedCount != gcvNULL)
        {
            *RedCount = r;
        }

        if (GreenCount != gcvNULL)
        {
            *GreenCount = g;
        }

        if (BlueCount != gcvNULL)
        {
            *BlueCount = b;
        }
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();
}

/*******************************************************************************
**
**  vdkSetMainWindowPostTitle
**
**  Set a string to be shown in the title bar after the main title.
**
**  INPUT:
**
**        gctSTRING PostTitle
**            Pointer to the post-title string.
**
**  OUTPUT:
**
**        Nothing.
**
*/

void
vdkSetMainWindowPostTitle(
    gctSTRING PostTitle
    )
{
}

/*******************************************************************************
**
**  vdkSetMainWindowImage
**
**  Set an image to be displayed in the main window.
**
**  INPUT:
**
**        IN gctUINT Width
**        IN gctUINT Height
**            Size of the image.
**
**        IN gctUINT AlignedWidth
**        IN gctUINT AlignedHeight
**            Aligned size of the image.
**
**        IN gctPOINTER Image
**            Pointer to the image.
**
**  OUTPUT:
**
**        Nothing.
**
*/

void
vdkSetMainWindowImage(
    IN gctUINT Width,
    IN gctUINT Height,
    IN gctUINT AlignedWidth,
    IN gctUINT AlignedHeight,
    IN gctPOINTER Image
    )
{
    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        gctUINT imageStride;
        gctUINT imageSize;
        gctSTRING image;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Destroy previously created image if any. */
        _DestroyMainWindowImage();

        /* Set the new image size. */
        visual->common.mainWindowImageWidth  = Width;
        visual->common.mainWindowImageHeight = Height;

        /* Allocate image data. */
        imageStride = (AlignedWidth * visual->common.mainWindowDepth) / 8;
        imageSize = imageStride * AlignedHeight;
        image = malloc(imageSize);

        if (image == gcvNULL)
        {
            break;
        }

        /* Copy the image data. */
        memcpy(image, Image, imageSize);

        /* Create a new image. */
        visual->MainImage.stride = imageStride;
        visual->MainImage.size = imageSize;
        visual->MainImage.bits = image;

        if (visual->MainImage.bits != gcvNULL)
        {
            /* Draw the image. */
            _DrawMainWindowImage();
        }
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();
}

/*******************************************************************************
**
**  vdkEnterMainWindowLoop
**
**  Enter the window message loop.
**
**  INPUT:
**
**        Nothing.
**
**  OUTPUT:
**
**        Nothing.
**
*/

int
vdkEnterMainWindowLoop(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Set handler for the ALARM signal. */
    signal(SIGALRM, _TimerHandler);

    /* Determine the timer enable flag. */
    visual->timerExpired = (visual->common.timerEvent != gcvNULL)
        ? gcvTRUE
        : gcvFALSE;

    /* Enter the loop. */
    while (visual->MainWindow.height != 0 &&
        visual->MainWindow.width != 0)
    {

        /* Is there a KBHit */
        if (KBHit())
        {
            /* Call the user-defined handler. */
            gctBOOL result = (*visual->common.keyboardEvent)(
                visual->common.eventArgument,
                0,
                Readch()
                );

            if (!result)
            {
                break;
            }
        }

        /* Is there a timer routine? */
        if (visual->timerExpired)
        {
            /* Call the timer routine. */
            if (!ivdkServeTimer())
            {
                break;
            }

            /* Reset the timeout. */
            visual->timerExpired = gcvFALSE;

            /* Schedule next time event. */
            alarm(visual->common.timerDelay);
        }
    }

    /* Cancel the timer. */
    alarm(0);
    signal(SIGALRM, SIG_DFL);

    /* Success. */
    return 0;
}

/* if defined(LINUX) && defined(FBDriver)*/
#endif
