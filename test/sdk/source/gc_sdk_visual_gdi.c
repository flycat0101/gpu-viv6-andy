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


#include <gc_sdk.h>
#include "gc_sdk_internal.h"
#include "gc_sdk_internal_windows.h"


/******************************************************************************\
*************************** GDI Specific Information. **************************
\******************************************************************************/

struct _gcsVISUAL
{
    /* Include the common visual structure. */
    struct _gcsCOMMONVISUAL common;

    /* Main window properties. */
    HWND mainWindow;
    HBITMAP mainWindowBitmap;
    PTCHAR mainWindowTitle;
    gctINT mainWindowTitleLen;
};


/******************************************************************************\
****************************** Support Functions ******************************
\******************************************************************************/

static void
_DestroyMainWindowBitmap(
    void
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have an image to destroy? */
    if (visual->mainWindowBitmap != gcvNULL)
    {
        /* Destroy the image. */
        DeleteObject(visual->mainWindowBitmap);
        visual->mainWindowBitmap = gcvNULL;
    }
}

static gctBOOL
_DrawMainWindowBitmap(
    HWND Wnd
    )
{
    RECT rect;
    HDC hDC, hMemDC;
    PAINTSTRUCT ps;
    HBITMAP hOldBitmap;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Get device context. */
    hDC = BeginPaint(Wnd, &ps);

    /* Get the client area of the window. */
    GetClientRect(Wnd, &rect);

    /* Do we have an image to draw? */
    if (visual->mainWindowBitmap == gcvNULL)
    {
        FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));
    }
    else
    {
        /* Create a compatible device context for the bitmap. */
        hMemDC = CreateCompatibleDC(hDC);

        /* Select the bitmap into the context. */
        hOldBitmap = SelectObject(hMemDC, visual->mainWindowBitmap);

        /* Draw the bitmap. */
        StretchBlt(
            hDC,
            0, 0,                        /* Destination origin. */
            rect.right  - rect.left,    /* Destination width. */
            rect.bottom - rect.top,        /* Destination height. */
            hMemDC,
            0, 0,                        /* Source origin. */
            visual->common.mainWindowImageWidth,
            visual->common.mainWindowImageHeight,
            SRCCOPY
            );

        /* Select back the old bitmap. */
        SelectObject(hMemDC, hOldBitmap);

        /* Destroy the memory context. */
        DeleteDC(hMemDC);
    }

    /* Validate the cllient area. */
    EndPaint(Wnd, &ps);

    /* Success. */
    return gcvTRUE;
}

static gctUINT
_GetState(
    void
    )
{
    gctUINT mask = 0;

    if (GetKeyState(VK_LBUTTON) < 0)
    {
        mask |= gcvLEFTBUTTON;
    }

    if (GetKeyState(VK_MBUTTON) < 0)
    {
        mask |= gcvMIDDLEBUTTON;
    }

    if (GetKeyState(VK_RBUTTON) < 0)
    {
        mask |= gcvRIGHTBUTTON;
    }

    if (GetKeyState(VK_CONTROL) < 0)
    {
        mask |= gcvCTRL;
    }

    if (GetKeyState(VK_MENU) < 0)
    {
        mask |= gcvALT;
    }

    if (GetKeyState(VK_SHIFT) < 0)
    {
        mask |= gcvSHIFT;
    }

    return mask;
}

static gctBOOL
_ServeKeyPress(
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have a keyboard handler? */
    if (visual->common.keyboardEvent == gcvNULL)
    {
        return gcvTRUE;
    }

    /* Was the key just pressed? */
    if ((LParam & (1 << 30)) != 0)
    {
        return gcvTRUE;
    }

    /* Call the handler. */
    return (*visual->common.keyboardEvent) (
        visual->common.eventArgument,
        _GetState(),
        (gctUINT8) WParam
        );
}

static gctBOOL
_ServeMouseClick(
    LPARAM LParam,
    gctUINT Button
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have a mouse handler? */
    if (visual->common.mousePressEvent == gcvNULL)
    {
        return gcvTRUE;
    }

    /* Call the handler. */
    return (*visual->common.mousePressEvent)(
        visual->common.eventArgument,
        (gctINT16) LParam,
        (gctINT16) (LParam >> 16),
        _GetState(),
        Button
        );
}

static gctBOOL
_ServeMouseMove(
    LPARAM LParam
    )
{
    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Do we have a mouse handler? */
    if (visual->common.mouseMoveEvent == gcvNULL)
    {
        return gcvTRUE;
    }

    /* Call the handler. */
    return (*visual->common.mouseMoveEvent)(
        visual->common.eventArgument,
        (gctINT16) LParam,
        (gctINT16) (LParam >> 16),
        _GetState()
        );
}

static LRESULT CALLBACK
_WindowProc(
    IN HWND Window,
    IN UINT Message,
    IN WPARAM WParam,
    IN LPARAM LParam
    )
{
    /* Timer identifier. */
    const UINT timerID = 1;

    /* Assume success. */
    gctBOOL result = gcvTRUE;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Dispatch. */
    switch (Message)
    {
    case WM_CREATE:
        /* Set the main window ID. */
        visual->mainWindow = Window;

        /* Timer handler specified? */
        if (visual->common.timerEvent != gcvNULL)
        {
            /* Create timer */
            SetTimer(Window, timerID, visual->common.timerDelay * 1000, gcvNULL);

            /* Post a message to get immediate initial hit. */
            PostMessage(Window, WM_TIMER, (WPARAM) timerID, (LPARAM) gcvNULL);
        }
        break;

    case WM_PAINT:
        /* Acquire the visual mutex. */
        ivdkLockVisual();

        /* Paint the image. */
        result = _DrawMainWindowBitmap(Window);

        /* Release the visual mutex. */
        ivdkUnlockVisual();
        break;

    case WM_TIMER:
        result = ivdkServeTimer();
        break;

    case WM_KEYDOWN:
        result = _ServeKeyPress(WParam, LParam);
        break;

    case WM_LBUTTONDOWN:
        result = _ServeMouseClick(LParam, gcvLEFTBUTTON);
        break;

    case WM_MBUTTONDOWN:
        result = _ServeMouseClick(LParam, gcvMIDDLEBUTTON);
        break;

    case WM_RBUTTONDOWN:
        result = _ServeMouseClick(LParam, gcvRIGHTBUTTON);
        break;

    case WM_MOUSEMOVE:
        result = _ServeMouseMove(LParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(Window, Message, WParam, LParam);
    }

    /* Destroy the window if a message handler requested so. */
    if (!result)
    {
        PostMessage(Window, WM_DESTROY, 0, 0);
    }

    return 0;
}

#if defined(UNICODE)

#    define INITSTRING(String) \
        _ConvertToUnicode(String)

    static PWCHAR
    _ConvertToUnicode(
        gctSTRING String
        )
    {
        int wideLength;
        PWCHAR wideBuffer = NULL;

        /* Determine the length of the buffer needed. */
        wideLength = MultiByteToWideChar(
            CP_ACP,            /* ANSI code page. */
            0,                /* No special flags. */
            String, -1,        /* Zero-terminated source string. */
            NULL,
            0
            );

        /* Convert. */
        if (wideLength != 0)
        {
            /* Allocate the buffer. */
            wideBuffer = (PWCHAR) malloc(wideLength * sizeof(WCHAR));
            if (wideBuffer != NULL)
            {
                /* Do the conversion. */
                MultiByteToWideChar(
                    CP_ACP,            /* ANSI code page. */
                    0,                /* No special flags. */
                    String, -1,        /* Zero-terminated source string. */
                    wideBuffer,        /* Result buffer. */
                    wideLength        /* Length of the string. */
                    );
            }
        }

        /* Return result. */
        return wideBuffer;
    }

#else

#    define INITSTRING(String) \
        _strdup(String)

#endif


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
    static struct _gcsVISUAL visual = {0};

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

        /* Determine the desktop size. */
        visual->common.desktopWidth  = GetSystemMetrics(SM_CXSCREEN);
        visual->common.desktopHeight = GetSystemMetrics(SM_CYSCREEN);

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
    /* Set the window style. */
    const int wndStyle
        = WS_VISIBLE
        | WS_BORDER
        | WS_DLGFRAME
        | WS_SYSMENU;

    gctBOOL result;
    gctINT windowShow;
    WNDCLASS wc;
    RECT rect;
    HWND wnd;
    HDC hDC;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Get a pointer to the application object. */
    gcsAPPLICATION_PTR app = ivdkGetApplication();

    do
    {
        /* Store message handlers. */
        visual->common.eventArgument   = EventArgument;
        visual->common.keyboardEvent   = KeyboardEvent;
        visual->common.mousePressEvent = MousePressEvent;
        visual->common.mouseMoveEvent  = MouseMoveEvent;
        visual->common.timerEvent      = TimerEvent;
        visual->common.timerDelay      = TimerDelay;

        /* Copy the title. */
        visual->mainWindowTitle    = INITSTRING(Title);
        visual->mainWindowTitleLen = (gctINT) _tcslen(visual->mainWindowTitle);

        /* Register window class. */
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = _WindowProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = app->instance;
#if defined(IDI_APPLICATION)
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
#else
        wc.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(32512));
#endif
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = visual->mainWindowTitle;

        if (!RegisterClass(&wc))
        {
            result = gcvFALSE;
            break;
        }

        /* Handle the auto placement option. */
        windowShow = app->windowShow;

        if ((Width == 0) || (Width > visual->common.desktopWidth))
        {
            Width = visual->common.desktopWidth;
            windowShow = SW_SHOWMAXIMIZED;
        }

        if ((Height == 0) || (Height > visual->common.desktopHeight))
        {
            Height = visual->common.desktopHeight;
            windowShow = SW_SHOWMAXIMIZED;
        }

        if (X < 0)
        {
            X = (visual->common.desktopWidth - Width) / 2;
        }

        if (Y < 0)
        {
            Y = (visual->common.desktopHeight - Height) / 2;
        }

        /* Construct the rectangle. */
        rect.left   = X;
        rect.top    = Y;
        rect.right  = rect.left + Width;
        rect.bottom = rect.top  + Height;

        /* Adjust to get the outside window coordinates. */
        if (!AdjustWindowRectEx(&rect, wndStyle, gcvFALSE, 0))
        {
            result = gcvFALSE;
            break;
        }

        /* Validate the coordinates. */
        if (rect.left < 0)
        {
            rect.left = 0;
            windowShow = SW_SHOWMAXIMIZED;
        }

        if (rect.top < 0)
        {
            rect.top = 0;
            windowShow = SW_SHOWMAXIMIZED;
        }

        if (rect.right > (gctINT) visual->common.desktopWidth)
        {
            rect.right = visual->common.desktopWidth;
            windowShow = SW_SHOWMAXIMIZED;
        }

        if (rect.bottom > (gctINT) visual->common.desktopHeight)
        {
            rect.bottom = visual->common.desktopHeight;
            windowShow = SW_SHOWMAXIMIZED;
        }

        /* Create the main window. */
        wnd = CreateWindow(
            wc.lpszClassName,
            visual->mainWindowTitle,
            wndStyle,
            rect.left,
            rect.top,
            rect.right  - rect.left,
            rect.bottom - rect.top,
            NULL,                    /* Parent. */
            NULL,                    /* Menu. */
            app->instance,
            NULL                    /* LParam. */
            );

        if (wnd == gcvNULL)
        {
            result = gcvFALSE;
            break;
        }

        /* Show the window. */
        ShowWindow(wnd, windowShow);
        UpdateWindow(wnd);

        /* Get window pixel depth. */
        hDC = GetDC(wnd);
        visual->common.mainWindowDepth = GetDeviceCaps(hDC, BITSPIXEL);
        ReleaseDC(wnd, hDC);

        /* Success. */
        result = gcvTRUE;
    }
    while (gcvFALSE);

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
**        Nothing.
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
        _DestroyMainWindowBitmap();

        /* A valid window? */
        if (IsWindow(visual->mainWindow))
        {
            PostMessage(visual->mainWindow, WM_DESTROY, 0, 0);
        }

        visual->mainWindow = gcvNULL;

        /* Free the title. */
        if (visual->mainWindowTitle != gcvNULL)
        {
            free(visual->mainWindowTitle);
            visual->mainWindowTitle = gcvNULL;
            visual->mainWindowTitleLen = 0;
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
    return (gctHANDLE) GetDC(NULL);
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
    ReleaseDC(NULL, (HDC) Context);
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

    return (gctHANDLE) visual->mainWindow;
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
    gctUINT width  = 0;
    gctUINT height = 0;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* A valid window? */
    if (IsWindow(visual->mainWindow))
    {
        RECT rect;

        /* Is the size needed? */
        if ((Width != gcvNULL) || (Height != gcvNULL))
        {
            /* Query the size. */
            GetClientRect(visual->mainWindow, &rect);

            /* Set the output. */
            width  = rect.right  - rect.left;
            height = rect.bottom - rect.top;
        }
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
        g = 5;
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
    gctINT postTitleLength;

    /* Get a pointer to the visual object. */
    gcsVISUAL_PTR visual = ivdkGetVisual();

    /* Determine post-title length. */
    postTitleLength = (PostTitle == gcvNULL)
        ? 0
        : (gctINT) strlen(PostTitle);

    /* Are we deleting the post-title? */
    if (postTitleLength == 0)
    {
        /* Set the original title. */
        SetWindowText(visual->mainWindow, visual->mainWindowTitle);
    }
    else
    {
        /* Determine new title length. */
        gctINT titleLen = visual->mainWindowTitleLen + postTitleLength + 1;

        /* Allocate enough space. */
        PTCHAR title = malloc(titleLen * sizeof(TCHAR));

        /* Convert the post-title as necessary. */
        PTCHAR postTitle = INITSTRING(PostTitle);

        /* Create new titie. */
        _tcscpy(title, visual->mainWindowTitle);
        _tcscat(title, postTitle);

        /* Set the new title. */
        SetWindowText(visual->mainWindow, title);

        /* Free memory. */
        free(postTitle);
        free(title);
    }
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
        gctUINT stride;
        BITMAPINFO bitmap;
        gctPOINTER bits;
        HDC hDC;

        /* Get a pointer to the visual object. */
        gcsVISUAL_PTR visual = ivdkGetVisual();

        /* Destroy previously created bitmap if any. */
        _DestroyMainWindowBitmap();

        /* Create a device context. */
        hDC = GetDC(visual->mainWindow);

        /* Set the new image size. */
        visual->common.mainWindowImageWidth  = Width;
        visual->common.mainWindowImageHeight = Height;

        /* Determine the stride. */
        stride = AlignedWidth * visual->common.mainWindowDepth / 8;

        /* Fill in the BITMAPINFOHEADER information. */
        bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
        bitmap.bmiHeader.biWidth = AlignedWidth;
        bitmap.bmiHeader.biHeight = -(gctINT) AlignedHeight;
        bitmap.bmiHeader.biPlanes = 1;
        bitmap.bmiHeader.biBitCount = visual->common.mainWindowDepth;
        bitmap.bmiHeader.biCompression = BI_RGB;
        bitmap.bmiHeader.biSizeImage = stride * AlignedHeight;
        bitmap.bmiHeader.biXPelsPerMeter = 0;
        bitmap.bmiHeader.biYPelsPerMeter = 0;
        bitmap.bmiHeader.biClrUsed = 0;
        bitmap.bmiHeader.biClrImportant = 0;

        /* Create a new bitmap. */
        visual->mainWindowBitmap = CreateDIBSection(
            hDC,
            &bitmap,
            DIB_RGB_COLORS,
            &bits,
            gcvNULL,
            0
            );

        /* Copy the image bits. */
        memcpy(bits, Image, bitmap.bmiHeader.biSizeImage);

        /* Release the DC. */
        ReleaseDC(visual->mainWindow, hDC);

        /* Force a redraw. */
        InvalidateRect(visual->mainWindow, NULL, FALSE);
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
    MSG message;

    /* Enter the message loop. */
    while (GetMessage(&message, NULL, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    /* Return the exit code. */
    return (int) message.wParam;
}
