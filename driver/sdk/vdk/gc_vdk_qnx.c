/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#define _QNX_SOURCE
#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <semaphore.h>
#include <screen/screen.h>
#include <unistd.h>
#include <dlfcn.h>

#include <sys/time.h>


/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$QNX\n";

struct _vdkPrivate
{
    vdkDisplay  display;
    void *      egl;
};

static vdkPrivate _vdk = NULL;

static screen_context_t screen_ctx = NULL;

/* Structure that defined keyboard mapping. */
typedef struct _vdkKeyMap
{
    /* Normal key. */
    vdkKeys normal;

    /* Extended key. */
    vdkKeys extended;
}
vdkKeyMap;

static vdkKeyMap keys[] =
{
    /* 00 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 01 */ { VDK_ESCAPE,          VDK_UNKNOWN     },
    /* 02 */ { VDK_1,               VDK_UNKNOWN     },
    /* 03 */ { VDK_2,               VDK_UNKNOWN     },
    /* 04 */ { VDK_3,               VDK_UNKNOWN     },
    /* 05 */ { VDK_4,               VDK_UNKNOWN     },
    /* 06 */ { VDK_5,               VDK_UNKNOWN     },
    /* 07 */ { VDK_6,               VDK_UNKNOWN     },
    /* 08 */ { VDK_7,               VDK_UNKNOWN     },
    /* 09 */ { VDK_8,               VDK_UNKNOWN     },
    /* 0A */ { VDK_9,               VDK_UNKNOWN     },
    /* 0B */ { VDK_0,               VDK_UNKNOWN     },
    /* 0C */ { VDK_HYPHEN,          VDK_UNKNOWN     },
    /* 0D */ { VDK_EQUAL,           VDK_UNKNOWN     },
    /* 0E */ { VDK_BACKSPACE,       VDK_UNKNOWN     },
    /* 0F */ { VDK_TAB,             VDK_UNKNOWN     },
    /* 10 */ { VDK_Q,               VDK_UNKNOWN     },
    /* 11 */ { VDK_W,               VDK_UNKNOWN     },
    /* 12 */ { VDK_E,               VDK_UNKNOWN     },
    /* 13 */ { VDK_R,               VDK_UNKNOWN     },
    /* 14 */ { VDK_T,               VDK_UNKNOWN     },
    /* 15 */ { VDK_Y,               VDK_UNKNOWN     },
    /* 16 */ { VDK_U,               VDK_UNKNOWN     },
    /* 17 */ { VDK_I,               VDK_UNKNOWN     },
    /* 18 */ { VDK_O,               VDK_UNKNOWN     },
    /* 19 */ { VDK_P,               VDK_UNKNOWN     },
    /* 1A */ { VDK_LBRACKET,        VDK_UNKNOWN     },
    /* 1B */ { VDK_RBRACKET,        VDK_UNKNOWN     },
    /* 1C */ { VDK_ENTER,           VDK_PAD_ENTER   },
    /* 1D */ { VDK_LCTRL,           VDK_RCTRL       },
    /* 1E */ { VDK_A,               VDK_UNKNOWN     },
    /* 1F */ { VDK_S,               VDK_UNKNOWN     },
    /* 20 */ { VDK_D,               VDK_UNKNOWN     },
    /* 21 */ { VDK_F,               VDK_UNKNOWN     },
    /* 22 */ { VDK_G,               VDK_UNKNOWN     },
    /* 23 */ { VDK_H,               VDK_UNKNOWN     },
    /* 24 */ { VDK_J,               VDK_UNKNOWN     },
    /* 25 */ { VDK_K,               VDK_UNKNOWN     },
    /* 26 */ { VDK_L,               VDK_UNKNOWN     },
    /* 27 */ { VDK_SEMICOLON,       VDK_UNKNOWN     },
    /* 28 */ { VDK_SINGLEQUOTE,     VDK_UNKNOWN     },
    /* 29 */ { VDK_BACKQUOTE,       VDK_UNKNOWN     },
    /* 2A */ { VDK_LSHIFT,          VDK_UNKNOWN     },
    /* 2B */ { VDK_BACKSLASH,       VDK_UNKNOWN     },
    /* 2C */ { VDK_Z,               VDK_UNKNOWN     },
    /* 2D */ { VDK_X,               VDK_UNKNOWN     },
    /* 2E */ { VDK_C,               VDK_UNKNOWN     },
    /* 2F */ { VDK_V,               VDK_UNKNOWN     },
    /* 30 */ { VDK_B,               VDK_UNKNOWN     },
    /* 31 */ { VDK_N,               VDK_UNKNOWN     },
    /* 32 */ { VDK_M,               VDK_UNKNOWN     },
    /* 33 */ { VDK_COMMA,           VDK_UNKNOWN     },
    /* 34 */ { VDK_PERIOD,          VDK_UNKNOWN     },
    /* 35 */ { VDK_SLASH,           VDK_PAD_SLASH   },
    /* 36 */ { VDK_RSHIFT,          VDK_UNKNOWN     },
    /* 37 */ { VDK_PAD_ASTERISK,    VDK_PRNTSCRN    },
    /* 38 */ { VDK_LALT,            VDK_RALT        },
    /* 39 */ { VDK_SPACE,           VDK_UNKNOWN     },
    /* 3A */ { VDK_CAPSLOCK,        VDK_UNKNOWN     },
    /* 3B */ { VDK_F1,              VDK_UNKNOWN     },
    /* 3C */ { VDK_F2,              VDK_UNKNOWN     },
    /* 3D */ { VDK_F3,              VDK_UNKNOWN     },
    /* 3E */ { VDK_F4,              VDK_UNKNOWN     },
    /* 3F */ { VDK_F5,              VDK_UNKNOWN     },
    /* 40 */ { VDK_F6,              VDK_UNKNOWN     },
    /* 41 */ { VDK_F7,              VDK_UNKNOWN     },
    /* 42 */ { VDK_F8,              VDK_UNKNOWN     },
    /* 43 */ { VDK_F9,              VDK_UNKNOWN     },
    /* 44 */ { VDK_F10,             VDK_UNKNOWN     },
    /* 45 */ { VDK_NUMLOCK,         VDK_UNKNOWN     },
    /* 46 */ { VDK_SCROLLLOCK,      VDK_BREAK       },
    /* 47 */ { VDK_PAD_7,           VDK_HOME        },
    /* 48 */ { VDK_PAD_8,           VDK_UP          },
    /* 49 */ { VDK_PAD_9,           VDK_PGUP        },
    /* 4A */ { VDK_PAD_HYPHEN,      VDK_UNKNOWN     },
    /* 4B */ { VDK_PAD_4,           VDK_LEFT        },
    /* 4C */ { VDK_PAD_5,           VDK_UNKNOWN     },
    /* 4D */ { VDK_PAD_6,           VDK_RIGHT       },
    /* 4E */ { VDK_PAD_PLUS,        VDK_UNKNOWN     },
    /* 4F */ { VDK_PAD_1,           VDK_END         },
    /* 50 */ { VDK_PAD_2,           VDK_DOWN        },
    /* 51 */ { VDK_PAD_3,           VDK_PGDN        },
    /* 52 */ { VDK_PAD_0,           VDK_INSERT      },
    /* 53 */ { VDK_PAD_PERIOD,      VDK_DELETE      },
    /* 54 */ { VDK_SYSRQ,           VDK_UNKNOWN     },
    /* 55 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 56 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 57 */ { VDK_F11,             VDK_UNKNOWN     },
    /* 58 */ { VDK_F12,             VDK_UNKNOWN     },
    /* 59 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 5A */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 5B */ { VDK_UNKNOWN,         VDK_LWINDOW     },
    /* 5C */ { VDK_UNKNOWN,         VDK_RWINDOW     },
    /* 5D */ { VDK_UNKNOWN,         VDK_MENU        },
    /* 5E */ { VDK_UNKNOWN,         VDK_POWER       },
    /* 5F */ { VDK_UNKNOWN,         VDK_SLEEP       },
    /* 60 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 61 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 62 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 63 */ { VDK_UNKNOWN,         VDK_WAKE        },
    /* 64 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 65 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 66 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 67 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 68 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 69 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6A */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6B */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6C */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6D */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6E */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 6F */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 70 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 71 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 72 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 73 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 74 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 75 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 76 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 77 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 78 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 79 */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7A */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7B */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7C */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7D */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7E */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
    /* 7F */ { VDK_UNKNOWN,         VDK_UNKNOWN     },
};

/*******************************************************************************
** Initialization.
*/

VDKAPI vdkPrivate VDKLANG
vdkInitialize(
    void
    )
{
    vdkPrivate  vdk = NULL;
    int rc;

    if (_vdk)
    {
        return _vdk;
    }

    vdk = (vdkPrivate) malloc(sizeof(struct _vdkPrivate));

    if (vdk == NULL)
    {
        return NULL;
    }

#if gcdSTATIC_LINK
    vdk->egl = NULL;
#else
    vdk->egl = dlopen("libEGL.so.1", RTLD_LAZY);
#endif

    rc = screen_create_context(&screen_ctx, 0);

    if (rc)
    {
        fprintf(stderr, "screen_create_context failed: error=%d (%s)\n",
                errno, strerror(errno));
    }


    vdk->display = (EGLNativeDisplayType) 0;

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

    if (screen_ctx)
    {
        screen_destroy_context(screen_ctx);
        screen_ctx = NULL;
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
    if (!Private)
    {
        return 0;
    }

    if (Private->display != (EGLNativeDisplayType) 0)
    {
        return Private->display;
    }

    Private->display = (NativeDisplayType) 1;
    return Private->display;
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
    screen_display_t screen_disp = NULL;
    screen_display_t *screen_displays;
    int count = 0;
    int size[2] = {0, 0};
    int i = 0;
    int val = 0;
    int rc = 0;

    rc = screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &count);
    if (rc)
    {
        return -1;
    }

    if (count > 0)
    {
        screen_displays = calloc(count, sizeof(screen_display_t));
        screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void**)screen_displays);

        for (i = 0; i < count; i++)
        {
            screen_get_display_property_iv(screen_displays[i], SCREEN_PROPERTY_ID, &val);
            if (val == (int)Display)
            {
                screen_disp =screen_displays[i];
                break;
            }
        }
        free(screen_displays);
    }

    if (!screen_disp)
    {
        return -1;
    }

    rc =screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, size);
    if (rc)
    {
        return -1;
    }

    if (Width)
    {
        *Width = size[0];
    }

    if (Height)
    {
        *Height = size[1];
    }

    if (Physical)
    {
        *Physical = ~0;
    }

    if (Stride)
    {
        *Stride = 0;
    }

    if (BitsPerPixel)
    {
        *BitsPerPixel = 0;
    }

    return 0;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    if (_vdk->display == Display)
    {
        _vdk->display = 0;
    }
}

/*******************************************************************************
** Windows
*/

static int
_GetBitsPerPixel(
    int ScreenFormat
    )
{
    switch (ScreenFormat)
    {
    case SCREEN_FORMAT_BYTE:
        return 8;

    case SCREEN_FORMAT_RGBA4444:
    case SCREEN_FORMAT_RGBX4444:
    case SCREEN_FORMAT_RGBA5551:
    case SCREEN_FORMAT_RGBX5551:
    case SCREEN_FORMAT_RGB565:
    case SCREEN_FORMAT_RGB888:
        return 16;

    case SCREEN_FORMAT_RGBA8888:
    case SCREEN_FORMAT_RGBX8888:
        return 32;
    default:
        return 0;
    }
}

vdkWindow
vdkCreateWindow(
    vdkDisplay Display,
    int X,
    int Y,
    int Width,
    int Height
    )
{
    int pos[2];
    int size[2];
    screen_window_t window = (NativeDisplayType) 0;
    int screen_format = SCREEN_FORMAT_RGBX8888;
    int screen_transparency = SCREEN_TRANSPARENCY_NONE;
    int screen_usage = SCREEN_USAGE_OPENGL_ES1
                     | SCREEN_USAGE_OPENGL_ES2
                     | SCREEN_USAGE_OPENVG;

    /* Use 0 for no-vsync, and 1 for vsync limited. */
    int screen_swap_interval = 0;
    int rc;


    /* Create window strcture. */
    rc = screen_create_window(&window, screen_ctx);

    if (rc)
    {
        fprintf(stderr,
                "screen_create_window failed: error=%d (%s)\n",
                errno, strerror(errno));

        return NULL;
    }

    /* Set window pximap format. */
    rc = screen_set_window_property_iv(window,
            SCREEN_PROPERTY_FORMAT, &screen_format);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT) failed: error %d (%s)\n",
                 errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Set window usage. */
    rc = screen_set_window_property_iv(window, SCREEN_PROPERTY_USAGE, &screen_usage);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_USAGE) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Get fullscreen window size. */
    rc = screen_get_window_property_iv(window, SCREEN_PROPERTY_SIZE, size);

    if (rc)
    {
        fprintf(stderr,
                "screen_get_window_property_iv(SCREEN_PROPERTY_SIZE) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Disable transparency. Due to a bug in Screen, this must be set after format. */
    rc = screen_set_window_property_iv(window,
            SCREEN_PROPERTY_TRANSPARENCY, &screen_transparency);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_TRANSPARENCY) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Set swap interval. */
    rc = screen_set_window_property_iv(window,
            SCREEN_PROPERTY_SWAP_INTERVAL, &screen_swap_interval);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_SWAP_INTERVAL) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Test for zero width. */
    if (Width == 0)
    {
        Width = size[0];
    }

    /* Test for zero height. */
    if (Height == 0)
    {
        Height = size[1];
    }

    /* Test for auto-center X coordinate. */
    if (X == -1)
    {
        X = (size[0] - Width) / 2;
    }

    /* Test for auto-center X coordinate. */
    if (Y == -1)
    {
        Y = (size[1] - Height) / 2;
    }

    /* Resize the window. */
    size[0] = Width;
    size[1] = Height;

    rc = screen_set_window_property_iv(window, SCREEN_PROPERTY_SIZE, size);
    if (rc)
    {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_SIZE) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Create window buffer. */
    /* Second argument is the number of back buffers to be used. */
    rc = screen_create_window_buffers(window, gcdDISPLAY_BACK_BUFFERS);
    if (rc)
    {
        fprintf(stderr,
                "screen_create_window_buffers failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    /* Move window position. */
    pos[0] = X;
    pos[1] = Y;

    rc = screen_set_window_property_iv(window, SCREEN_PROPERTY_POSITION, pos);
    if (rc) {
        fprintf(stderr,
                "screen_set_window_property_iv(SCREEN_PROPERTY_POSITION) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_window(window);
        return NULL;
    }

    return window;
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
    int rc, size[2], format;

    if (Window == NULL)
    {
        /* Window is not a valid window data structure pointer. */
        return 0;
    }

    if (X != NULL)
    {
        *X = 0;
    }

    if (Y != NULL)
    {
        *Y = 0;
    }

    if ((Width != NULL) || (Height != NULL))
    {
        rc = screen_get_window_property_iv((screen_window_t)Window, SCREEN_PROPERTY_BUFFER_SIZE, size);

        if (rc)
        {
            return 0;
        }

        if (Width != NULL)
        {
            *Width = size[0];
        }

        if (Height != NULL)
        {
            *Height = size[1];
        }
    }

    if (BitsPerPixel != NULL)
    {
        rc = screen_get_window_property_iv(Window, SCREEN_PROPERTY_FORMAT, &format);

        if (rc)
        {
            return 0;
        }

        *BitsPerPixel = _GetBitsPerPixel(format);

        if (*BitsPerPixel == 0)
        {
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
    screen_destroy_window(Window);
}

VDKAPI int VDKLANG
vdkShowWindow(
    vdkWindow Window
    )
{
    return 1;
}

VDKAPI int VDKLANG
vdkHideWindow(
    vdkWindow Window
    )
{
    return 1;
}

VDKAPI void VDKLANG
vdkSetWindowTitle(
    vdkWindow Window,
    const char * Title
    )
{
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
    screen_event_t screen_evt;
    int rc;
    int ret = 0;

    rc = screen_create_event(&screen_evt);

    if (rc)
    {
        fprintf(stderr, "screen_create_event failed: error %d (%s)\n",
                errno, strerror(errno));
        return 0;
    }

    while (screen_get_event(screen_ctx, screen_evt, 0L) == 0)
    {
        int type;

        screen_get_event_property_iv(screen_evt, SCREEN_PROPERTY_TYPE, &type);

        if (type == SCREEN_EVENT_CLOSE)
        {
            Event->type = VDK_CLOSE;
            ret = 1;
            break;
        }
        else if (type == SCREEN_EVENT_POINTER)
        {
            static int last_buttons;
            int buttons;
            int pointer[2];

            screen_get_event_property_iv(screen_evt, SCREEN_PROPERTY_BUTTONS, &buttons);
            screen_get_event_property_iv(screen_evt, SCREEN_PROPERTY_POSITION, pointer);

            if (buttons != last_buttons)
            {
                Event->type = VDK_BUTTON;
                Event->data.button.left   = (buttons & 0x0001);
                Event->data.button.x = pointer[0];
                Event->data.button.y = pointer[1];

                last_buttons = buttons;
            }
            else
            {
                Event->type = VDK_POINTER;
                Event->data.pointer.x = pointer[0];
                Event->data.pointer.y = pointer[1];
            }

            ret = 1;
            break;
        }
        else if (type == SCREEN_EVENT_KEYBOARD)
        {
            int buffer;
            int scancode;
            static int prefix;
#if _SCREEN_VERSION_MAJOR >= 2
            screen_get_event_property_iv(screen_evt, SCREEN_PROPERTY_SCAN, &buffer);
#else
            screen_get_event_property_iv(screen_evt, SCREEN_PROPERTY_KEY_SCAN, &buffer);
#endif
            if ((buffer == 0xE0) || (buffer == 0xE1))
            {
                prefix = buffer;
                continue;
            }

            if (prefix)
            {
                scancode = keys[buffer & 0x7F].extended;
                prefix = 0;
            }
            else
            {
                scancode = keys[buffer & 0x7F].normal;
            }

            if (scancode == VDK_UNKNOWN)
            {
                continue;
            }

            Event->type                   = VDK_KEYBOARD;
            Event->data.keyboard.scancode = scancode;
            Event->data.keyboard.pressed  = buffer < 0x80;
            Event->data.keyboard.key      = ((scancode < VDK_SPACE)
                || (scancode >= VDK_F1)
                )
                ? 0
                : (char) scancode;
            ret = 1;
            break;
        }
        else if (type == SCREEN_EVENT_NONE)
        {
            break;
        }
        else
        {
            break;
        }
    }

    screen_destroy_event(screen_evt);

    return ret;
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

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    int size[2];
    int screen_format = SCREEN_FORMAT_RGBA8888;
    int screen_usage  = SCREEN_USAGE_OPENGL_ES1
                      | SCREEN_USAGE_OPENGL_ES2
                      | SCREEN_USAGE_OPENVG;
    NativePixmapType pixmap = NULL;
    int rc;

    if ((Width <= 0) || (Height <= 0) || (BitsPerPixel <= 0))
    {
        return NULL;
    }

    switch (BitsPerPixel)
    {
    case 8:
        screen_format = SCREEN_FORMAT_BYTE;
        break;

    case 16:
        screen_format = SCREEN_FORMAT_RGB565;
        break;

    case 24:
        screen_format = SCREEN_FORMAT_RGBX8888;
        break;

    case 32:
        screen_format = SCREEN_FORMAT_RGBA8888;
        break;

    default:
        return NULL;
    }

    /* Create pixmap structure. */
    rc = screen_create_pixmap(&pixmap, screen_ctx);

    if (rc)
    {
        fprintf(stderr, "screen_create_pixmap failed: error %d (%s)\n",
                errno, strerror(errno));
        return NULL;
    }

    /* Set pximap format. */
    rc = screen_set_pixmap_property_iv(pixmap,
            SCREEN_PROPERTY_FORMAT, &screen_format);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_pixmap_property_iv(SCREEN_PROPERTY_FORMAT) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_pixmap(pixmap);
        return NULL;
    }

    /* Set pixmap usage. */
    rc = screen_set_pixmap_property_iv(pixmap, SCREEN_PROPERTY_USAGE, &screen_usage);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_pixmap_property_iv(SCREEN_PROPERTY_USAGE) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_pixmap(pixmap);
        return NULL;
    }

    /* Resize the pixmap. */
    size[0] = Width;
    size[1] = Height;

    rc = screen_set_pixmap_property_iv(pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);

    if (rc)
    {
        fprintf(stderr,
                "screen_set_pixmap_property_iv(SCREEN_PROPERTY_BUFFER_SIZE) failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_pixmap(pixmap);
        return NULL;
    }

    /* Create pixmap buffer. */
    rc = screen_create_pixmap_buffer(pixmap);

    if (rc)
    {
        fprintf(stderr, "screen_create_pixmap_buffer failed: error %d (%s)\n",
                errno, strerror(errno));

        screen_destroy_pixmap(pixmap);
        return NULL;
    }

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
    int rc, size[2], format;
    screen_buffer_t buf[2];

    if (Pixmap == NULL)
    {
        /* Pixmap is not a valid pixmap data structure pointer. */
        return 0;
    }

    if ((Width != NULL) || (Height != NULL))
    {
        rc = screen_get_pixmap_property_iv(Pixmap, SCREEN_PROPERTY_BUFFER_SIZE, size);
        if (rc)
        {
            return 0;
        }

        if (Width != NULL)
        {
            *Width = size[0];
        }

        if (Height != NULL)
        {
            *Height = size[1];
        }
    }

    if (BitsPerPixel != NULL)
    {
        rc = screen_get_pixmap_property_iv(Pixmap, SCREEN_PROPERTY_FORMAT, &format);
        if (rc)
        {
            return 0;
        }

        *BitsPerPixel = _GetBitsPerPixel(format);

        if (*BitsPerPixel == 0)
        {
            return 0;
        }
    }

    rc = screen_get_pixmap_property_pv(Pixmap, SCREEN_PROPERTY_RENDER_BUFFERS, (void **) &buf);
    if (rc)
    {
        return 0;
    }

    if (Stride != NULL)
    {
        rc = screen_get_buffer_property_iv(buf[0], SCREEN_PROPERTY_STRIDE, (int *) Stride);
        if (rc)
        {
            return 0;
        }
    }

    if (Bits != NULL)
    {
        rc = screen_get_buffer_property_pv(buf[0], SCREEN_PROPERTY_POINTER, (void **) Bits);
        if (rc)
        {
            return 0;
        }
    }

    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    if (Pixmap != NULL)
    {
        screen_destroy_pixmap(Pixmap);
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
