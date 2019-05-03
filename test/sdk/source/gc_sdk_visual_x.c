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


#if defined(LINUX) && defined(X11)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

/* Silly X people. */
#undef Status

#include <gc_sdk.h>
#include "gc_sdk_internal.h"
#include "gc_sdk_internal_linux.h"


/******************************************************************************\
**************************** X Specific Information. ***************************
\******************************************************************************/

struct _gcsVISUAL
{
    /* Include the common visual structure. */
    struct _gcsCOMMONVISUAL common;

    /* Connection to the X server. */
    Display* display;

    /* Default screen associated with the display. */
    Screen* screen;

    /* Default visual associated with the screen. */
    Visual* visual;

    /* Root window ID and attributes. */
    Window rootWindowID;

    /* Main window properties. */
    Window mainWindowID;
    XImage* mainWindowImage;
    gctSTRING mainWindowTitle;
    gctINT mainWindowTitleLen;

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
    if (visual->mainWindowImage != gcvNULL)
    {
        /* Destroy the image. */
        XDestroyImage(visual->mainWindowImage);
        visual->mainWindowImage = gcvNULL;
    }
}

static void
_DrawMainWindowImage(
    void
    )
{
    GC gc;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have an image to draw? */
    if (visual->mainWindowImage != gcvNULL)
    {
        /* Create the graphics context. */
        gc = XCreateGC(
            visual->display,
            visual->mainWindowID,
            0,
            gcvNULL
            );

        /* Draw the image. */
        XPutImage(
            visual->display,
            visual->mainWindowID,
            gc,
            visual->mainWindowImage,
            0, 0,            /* Source origin. */
            0, 0,            /* Destination origin. */
            visual->common.mainWindowImageWidth,
            visual->common.mainWindowImageHeight
            );

        /* Flush the output buffer. */
        XFlush(visual->display);

        /* Release the graphics context. */
        XFreeGC(visual->display, gc);
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

static gctUINT
_TranslateState(
    gctUINT State
    )
{
    gctUINT mask = 0;

    if ((State & Button1Mask) != 0)
    {
        mask |= gcvLEFTBUTTON;
    }

    if ((State & Button2Mask) != 0)
    {
        mask |= gcvMIDDLEBUTTON;
    }

    if ((State & Button3Mask) != 0)
    {
        mask |= gcvRIGHTBUTTON;
    }

    if ((State & ControlMask) != 0)
    {
        mask |= gcvCTRL;
    }

    if ((State & Mod1Mask) != 0)
    {
        mask |= gcvALT;
    }

    if ((State & ShiftMask) != 0)
    {
        mask |= gcvSHIFT;
    }

    return mask;
}

static gctUINT
_TranslateButton(
    gctUINT Code
    )
{
    switch (Code)
    {
    case Button1:
        return gcvLEFTBUTTON;

    case Button2:
        return gcvMIDDLEBUTTON;

    case Button3:
        return gcvRIGHTBUTTON;
    }

    return 0;
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

gctBOOL
vdkInitVisual(
    void
    )
{
    gctBOOL result;

    do
    {
        XWindowAttributes rootWindowAttributes;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Create main window mutex. */
        visual->common.visualMutex = vdkCreateMutex();

        if (visual->common.visualMutex == gcvNULL)
        {
            gcmFATAL("Failed to create a visual mutex.");
            result = gcvFALSE;
            break;
        }

        /* Open a connection to the X server that controls a display. */
        visual->display = XOpenDisplay(gcvNULL);

        if (visual->display == gcvNULL)
        {
            gcmFATAL("Failed to open a connection to the X server.");
            result = gcvFALSE;
            break;
        }

        /* Get the default screen. */
        visual->screen = XDefaultScreenOfDisplay(visual->display);

        /* Get the default visual. */
        visual->visual = DefaultVisualOfScreen(visual->screen);

        /* Get the root window ID. */
        visual->rootWindowID = XRootWindowOfScreen(visual->screen);

        /* Get the root window's parameters. */
        XGetWindowAttributes(
            visual->display,
            visual->rootWindowID,
            &rootWindowAttributes
            );

        /* Init desktop size. */
        visual->common.desktopWidth  = rootWindowAttributes.width;
        visual->common.desktopHeight = rootWindowAttributes.height;

        /* Set invalid window ID. */
        visual->mainWindowID = BadWindow;

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
    if (visual->display != gcvNULL)
    {
        XCloseDisplay(visual->display);
        visual->display = gcvNULL;
    }

    /* Release the visual mutex. */
    ivdkUnlockVisual();

    /* Destroy the mutex. */
    if (visual->common.visualMutex != gcvNULL)
    {
        vdkCloseMutex(visual->common.visualMutex);
        visual->common.visualMutex = gcvNULL;
    }
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
        static gctSTRING defaultTitle = "";
        XSetWindowAttributes xswa;
        int screenDepth;
        Window wnd;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Message handlers. */
        visual->common.eventArgument   = EventArgument;
        visual->common.keyboardEvent   = KeyboardEvent;
        visual->common.mousePressEvent = MousePressEvent;
        visual->common.mouseMoveEvent  = MouseMoveEvent;
        visual->common.timerEvent      = TimerEvent;
        visual->common.timerDelay      = TimerDelay;

        /* Set the window title. */
        visual->mainWindowTitleLen = 0;
        visual->mainWindowTitle    = defaultTitle;

        if ((Title != gcvNULL) && (*Title != 0))
        {
            /* Copy the title. */
            visual->mainWindowTitle = strdup(Title);
            if (visual->mainWindowTitle == NULL)
            {
                result = gcvFALSE;
                break;
            }

            /* Determine the length. */
            visual->mainWindowTitleLen = strlen(Title);
        }

        /* Handle the auto placement option. */
        if ((Width == 0) || (Width > visual->common.desktopWidth))
        {
            Width = visual->common.desktopWidth;
        }

        if ((Height == 0) || (Height > visual->common.desktopHeight))
        {
            Height = visual->common.desktopHeight;
        }

        if (X < 0)
        {
            X = (visual->common.desktopWidth - Width) / 2;
        }

        if (Y < 0)
        {
            Y = (visual->common.desktopHeight - Height) / 2;
        }

        /* Get screen pixel depth. */
        screenDepth = DefaultDepthOfScreen(visual->screen);

        /* Set window parameters. */
        xswa.background_pixel = XWhitePixelOfScreen(visual->screen);
        xswa.event_mask = StructureNotifyMask
                        | VisibilityChangeMask
                        | ResizeRedirectMask
                        | ExposureMask
                        | ButtonPressMask
                        | KeyPressMask;

        /* Create window. */
        wnd = XCreateWindow(
            visual->display,
            visual->rootWindowID,
            0, 0,                /* Window origin; ignored by X. */
            Width,                /* Inside width of the window. */
            Height,                /* Inside heght of the window. */
            0,                    /* Border width. */
            screenDepth,
            InputOutput,
            visual->visual,
            CWEventMask | CWBackPixel,
            &xswa
            );

        /* Validate the result. */
        if ((wnd == BadAlloc)  ||
            (wnd == BadColor)  ||
            (wnd == BadCursor) ||
            (wnd == BadMatch)  ||
            (wnd == BadPixmap) ||
            (wnd == BadValue)  ||
            (wnd == BadWindow))
        {
            result = gcvFALSE;
            break;
        }

        /* Set the main window ID. */
        visual->mainWindowID = wnd;

        /* Get window pixel depth. */
        visual->common.mainWindowDepth = screenDepth;

        /* Map the window thus making it eligible for display. */
        XMapWindow(visual->display, visual->mainWindowID);

        /* Move the window to the specified position. */
        XMoveWindow(visual->display, visual->mainWindowID, X, Y);

        /* Success. */
        result = gcvTRUE;
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();

    /* Set the name of the window. */
    vdkSetMainWindowPostTitle(gcvNULL);

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
        if (visual->mainWindowID != BadWindow)
        {
            XDestroyWindow(visual->display, visual->mainWindowID);
            visual->mainWindowID = BadWindow;
        }

        /* Free the title. */
        if (visual->mainWindowTitleLen != 0)
        {
            free(visual->mainWindowTitle);
            visual->mainWindowTitleLen = 0;
            visual->mainWindowTitle    = gcvNULL;
        }
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

    return (gctHANDLE) visual->display;
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

    return (gctHANDLE) visual->mainWindowID;
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
        XWindowAttributes attr;
        gctUINT width  = 0;
        gctUINT height = 0;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Query window attributes. */
        if (XGetWindowAttributes(visual->display,
                                 visual->mainWindowID,
                                 &attr))
        {
            width  = attr.width;
            height = attr.height;
        }

        /* Set the output. */
        if (Width != gcvNULL)
        {
            *Width = width;
        }

        if (Height != gcvNULL)
        {
            *Height = height;
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
    /* Acquire the visual mutex. */
    ivdkLockVisual();

    do
    {
        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Point to the existing title. */
        gctINT    titleLen       = visual->mainWindowTitleLen;
        gctSTRING title          = visual->mainWindowTitle;
        gctBOOL   titleAllocated = gcvFALSE;

        /* Determine post-title length. */
        gctINT postTitleLength = (PostTitle == gcvNULL)
            ? 0
            : strlen(PostTitle);

        /* Is there a post-title? */
        if (postTitleLength != 0)
        {
            /* Allocate new title. */
            gctINT newTitleLen = visual->mainWindowTitleLen + postTitleLength;
            gctSTRING newTitle = (gctSTRING) malloc(newTitleLen + 1);

            /* Verify allocation. */
            if (newTitle != gcvNULL)
            {
                /* Mark as allocated. */
                titleAllocated = gcvTRUE;

                /* Set the title. */
                strcpy(newTitle, visual->mainWindowTitle);
                strcat(newTitle, PostTitle);

                /* Proceed to setting the new title. */
                titleLen = newTitleLen;
                title    = newTitle;
            }
        }

        /* Set the new title. */
        XChangeProperty(
            visual->display,
            visual->mainWindowID,
            XA_WM_NAME,                    /* Property name. */
            XA_STRING,                    /* Type of the property. */
            8,                            /* 8-bit data. */
            PropModeReplace,            /* Replace the existing setting. */
            (unsigned char*) title,
            titleLen
            );

        /* Cleanup. */
        if (titleAllocated)
        {
            free(title);
        }
    }
    while (gcvFALSE);

    /* Release the visual mutex. */
    ivdkUnlockVisual();
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
        visual->mainWindowImage = XCreateImage(
            visual->display,
            visual->visual,
            visual->common.mainWindowDepth,
            ZPixmap,        /* Format. */
            0,                /* Offset. */
            image,            /* Data. */
            AlignedWidth, AlignedHeight,
            8,                /* Quantum of a scanline. */
            0                /* Bytes per scanline. */
            );

        if (visual->mainWindowImage != gcvNULL)
        {
            /* Draw the image. */
            _DrawMainWindowImage();

            break;
        }

        /* Cleanup. */
        free(image);
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
    Bool eventMatched;
    XSetWindowAttributes xswa;
    XEvent event;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Set handler for the ALARM signal. */
    signal(SIGALRM, _TimerHandler);

    /* Determine the timer enable flag. */
    visual->timerExpired = (visual->common.timerEvent != gcvNULL)
        ? gcvTRUE
        : gcvFALSE;

    /* Enter the loop. */
    while (visual->mainWindowID != BadWindow)
    {
        /* Check window events. */
        xswa.event_mask = StructureNotifyMask
                        | VisibilityChangeMask
                        | ResizeRedirectMask
                        | ExposureMask
                        | ButtonPressMask
                        | KeyPressMask;

        /* Acquire the visual mutex. */
        ivdkLockVisual();

        eventMatched = XCheckWindowEvent(
            visual->display,
            visual->mainWindowID,
            xswa.event_mask,
            &event
            );

        /* Release the visual mutex. */
        ivdkUnlockVisual();

        /* Analyze events. */
        if (eventMatched)
        {
            /* Has the window been exposed? */
            if (event.type == Expose)
            {
                /* Acquire the visual mutex. */
                ivdkLockVisual();

                /* Redraw. */
                _DrawMainWindowImage();

                /* Release the visual mutex. */
                ivdkUnlockVisual();
            }

            /* Keyboard? */
            else if ((event.type == KeyPress) && (visual->common.keyboardEvent != gcvNULL))
            {
                gctUINT8 code;
                gctBOOL result;

                /* Acquire the visual mutex. */
                ivdkLockVisual();

                /* Get the corresponding KeySym code. */
                code = (gctUINT8) XKeycodeToKeysym(
                    visual->display,
                    event.xkey.keycode,
                    0
                    );

                /* Release the visual mutex. */
                ivdkUnlockVisual();

                /* Call the user-defined handler. */
                result = (*visual->common.keyboardEvent)(
                    visual->common.eventArgument,
                    _TranslateState(event.xkey.state),
                    code
                    );

                if (!result)
                {
                    break;
                }
            }

            /* Mouse button? */
            else if ((event.type == ButtonPress) && (visual->common.mousePressEvent != gcvNULL))
            {
                gctBOOL result = (*visual->common.mousePressEvent)(
                    visual->common.eventArgument,
                    event.xbutton.x,
                    event.xbutton.y,
                    _TranslateState(event.xbutton.state),
                    _TranslateButton(event.xbutton.button)
                    );

                if (!result)
                {
                    break;
                }
            }

            /* Mouse moved? */
            else if ((event.type == MotionNotify) && (visual->common.mouseMoveEvent != gcvNULL))
            {
                gctBOOL result = (*visual->common.mouseMoveEvent)(
                    visual->common.eventArgument,
                    event.xmotion.x,
                    event.xmotion.y,
                    _TranslateState(event.xmotion.state)
                    );

                if (!result)
                {
                    break;
                }
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

/* if defined(LINUX) && defined(X11)*/
#endif
