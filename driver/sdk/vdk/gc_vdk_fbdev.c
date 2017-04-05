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


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <gc_vdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dlfcn.h>

#include <sys/time.h>
#include <linux/input.h>

#include <fcntl.h>
#include <errno.h>


/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_PLATFORM = "\n\0$PLATFORM$FBDEV\n";

struct _vdkPrivate
{
    vdkDisplay  display;
    int xres;
    int yres;

    void *      egl;

    int mice;
    int keyboard;
};

static vdkPrivate _vdk = NULL;

static int
_DetectKeyboard(
    void
    )
{
    const char *template = "/sys/class/input/event%d/device/name";
    char path[64];
    char name[256];
    int id;

    for (id = 0; id < 16; id++)
    {
        ssize_t len;
        int fd;

        snprintf(path, sizeof (path), template, id);

        fd = open(path, O_RDONLY);
        if (fd < 0)
            continue;

        /* Avoid out of bounds access.*/
        len = read(fd, name, sizeof (name) - 1);
        close(fd);

        if (len > 0)
        {
            name[len] = '\0';

            if (strcasestr(name, "keyboard"))
            {
                /* This is a keyboard. */
                break;
            }
        }
    }

    if (id < 16)
    {
        snprintf(path, sizeof (path), "/dev/input/event%d", id);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        return fd;
    }

    return -1;
}

static int _keycodes[] =
{
    VDK_UNKNOWN,        /* #define KEY_RESERVED     0 */
    VDK_ESCAPE,         /* #define KEY_ESC          1 */
    VDK_1,              /* #define KEY_1            2 */
    VDK_2,              /* #define KEY_2            3 */
    VDK_3,              /* #define KEY_3            4 */
    VDK_4,              /* #define KEY_4            5 */
    VDK_5,              /* #define KEY_5            6 */
    VDK_6,              /* #define KEY_6            7 */
    VDK_7,              /* #define KEY_7            8 */
    VDK_8,              /* #define KEY_8            9 */
    VDK_9,              /* #define KEY_9            10 */
    VDK_0,              /* #define KEY_0            11 */
    VDK_HYPHEN,         /* #define KEY_MINUS        12 */
    VDK_EQUAL,          /* #define KEY_EQUAL        13 */
    VDK_BACKSPACE,      /* #define KEY_BACKSPACE    14 */
    VDK_TAB,            /* #define KEY_TAB          15 */
    VDK_Q,              /* #define KEY_Q            16 */
    VDK_W,              /* #define KEY_W            17 */
    VDK_E,              /* #define KEY_E            18 */
    VDK_R,              /* #define KEY_R            19 */
    VDK_T,              /* #define KEY_T            20 */
    VDK_Y,              /* #define KEY_Y            21 */
    VDK_U,              /* #define KEY_U            22 */
    VDK_I,              /* #define KEY_I            23 */
    VDK_O,              /* #define KEY_O            24 */
    VDK_P,              /* #define KEY_P            25 */
    VDK_LBRACKET,       /* #define KEY_LEFTBRACE    26 */
    VDK_RBRACKET,       /* #define KEY_RIGHTBRACE   27 */
    VDK_ENTER,          /* #define KEY_ENTER        28 */
    VDK_LCTRL,          /* #define KEY_LEFTCTRL     29 */
    VDK_A,              /* #define KEY_A            30 */
    VDK_S,              /* #define KEY_S            31 */
    VDK_D,              /* #define KEY_D            32 */
    VDK_F,              /* #define KEY_F            33 */
    VDK_G,              /* #define KEY_G            34 */
    VDK_H,              /* #define KEY_H            35 */
    VDK_J,              /* #define KEY_J            36 */
    VDK_K,              /* #define KEY_K            37 */
    VDK_L,              /* #define KEY_L            38 */
    VDK_SEMICOLON,      /* #define KEY_SEMICOLON    39 */
    VDK_SINGLEQUOTE,    /* #define KEY_APOSTROPHE   40 */
    VDK_BACKQUOTE,      /* #define KEY_GRAVE        41 */
    VDK_LSHIFT,         /* #define KEY_LEFTSHIFT    42 */
    VDK_BACKSLASH,      /* #define KEY_BACKSLASH    43 */
    VDK_Z,              /* #define KEY_Z            44 */
    VDK_X,              /* #define KEY_X            45 */
    VDK_C,              /* #define KEY_C            46 */
    VDK_V,              /* #define KEY_V            47 */
    VDK_B,              /* #define KEY_B            48 */
    VDK_N,              /* #define KEY_N            49 */
    VDK_M,              /* #define KEY_M            50 */
    VDK_COMMA,          /* #define KEY_COMMA        51 */
    VDK_PERIOD,         /* #define KEY_DOT          52 */
    VDK_SLASH,          /* #define KEY_SLASH        53 */
    VDK_RSHIFT,         /* #define KEY_RIGHTSHIFT   54 */
    VDK_PAD_ASTERISK,   /* #define KEY_KPASTERISK   55 */
    VDK_LALT,           /* #define KEY_LEFTALT      56 */
    VDK_SPACE,          /* #define KEY_SPACE        57 */
    VDK_CAPSLOCK,       /* #define KEY_CAPSLOCK     58 */
    VDK_F1,             /* #define KEY_F1           59 */
    VDK_F2,             /* #define KEY_F2           60 */
    VDK_F3,             /* #define KEY_F3           61 */
    VDK_F4,             /* #define KEY_F4           62 */
    VDK_F5,             /* #define KEY_F5           63 */
    VDK_F6,             /* #define KEY_F6           64 */
    VDK_F7,             /* #define KEY_F7           65 */
    VDK_F8,             /* #define KEY_F8           66 */
    VDK_F9,             /* #define KEY_F9           67 */
    VDK_F10,            /* #define KEY_F10          68 */
    VDK_NUMLOCK,        /* #define KEY_NUMLOCK      69 */
    VDK_SCROLLLOCK,     /* #define KEY_SCROLLLOCK   70 */
    VDK_PAD_7,          /* #define KEY_KP7          71 */
    VDK_PAD_8,          /* #define KEY_KP8          72 */
    VDK_PAD_9,          /* #define KEY_KP9          73 */
    VDK_PAD_HYPHEN,     /* #define KEY_KPMINUS      74 */
    VDK_PAD_4,          /* #define KEY_KP4          75 */
    VDK_PAD_5,          /* #define KEY_KP5          76 */
    VDK_PAD_6,          /* #define KEY_KP6          77 */
    VDK_PAD_PLUS,       /* #define KEY_KPPLUS       78 */
    VDK_PAD_1,          /* #define KEY_KP1          79 */
    VDK_PAD_2,          /* #define KEY_KP2          80 */
    VDK_PAD_3,          /* #define KEY_KP3          81 */
    VDK_PAD_0,          /* #define KEY_KP0          82 */
    VDK_PAD_PERIOD,     /* #define KEY_KPDOT        83 */
    VDK_UNKNOWN,        /* 84 */
    VDK_UNKNOWN,        /* #define KEY_ZENKAKUHANKAKU 85 */
    VDK_UNKNOWN,        /* #define KEY_102ND        86 */
    VDK_F11,            /* #define KEY_F11          87 */
    VDK_F12,            /* #define KEY_F12          88 */
    VDK_UNKNOWN,        /* #define KEY_RO           89 */
    VDK_UNKNOWN,        /* #define KEY_KATAKANA     90 */
    VDK_UNKNOWN,        /* #define KEY_HIRAGANA     91 */
    VDK_UNKNOWN,        /* #define KEY_HENKAN       92 */
    VDK_UNKNOWN,        /* #define KEY_KATAKANAHIRAGANA 93 */
    VDK_UNKNOWN,        /* #define KEY_MUHENKAN     94 */
    VDK_UNKNOWN,        /* #define KEY_KPJPCOMMA    95 */
    VDK_PAD_ENTER,      /* #define KEY_KPENTER      96 */
    VDK_RCTRL,          /* #define KEY_RIGHTCTRL    97 */
    VDK_PAD_SLASH,      /* #define KEY_KPSLASH      98 */
    VDK_SYSRQ,          /* #define KEY_SYSRQ        99 */
    VDK_RALT,           /* #define KEY_RIGHTALT     100 */
    VDK_UNKNOWN,        /* #define KEY_LINEFEED     101 */
    VDK_HOME,           /* #define KEY_HOME         102 */
    VDK_UP,             /* #define KEY_UP           103 */
    VDK_PGUP,           /* #define KEY_PAGEUP       104 */
    VDK_LEFT,           /* #define KEY_LEFT         105 */
    VDK_RIGHT,          /* #define KEY_RIGHT        106 */
    VDK_END,            /* #define KEY_END          107 */
    VDK_DOWN,           /* #define KEY_DOWN         108 */
    VDK_PGDN,           /* #define KEY_PAGEDOWN     109 */
    VDK_INSERT,         /* #define KEY_INSERT       110 */
    VDK_DELETE,         /* #define KEY_DELETE       111 */
    VDK_UNKNOWN,        /* #define KEY_MACRO        112 */
    VDK_MUTE,           /* #define KEY_MUTE         113 */
    VDK_VOLUMEDOWN,     /* #define KEY_VOLUMEDOWN   114 */
    VDK_VOLUMEUP,       /* #define KEY_VOLUMEUP     115 */
    VDK_POWER,          /* #define KEY_POWER        116 */
    VDK_EQUAL,          /* #define KEY_KPEQUAL      117 */
    VDK_UNKNOWN,        /* #define KEY_KPPLUSMINUS  118 */
    VDK_BREAK,          /* #define KEY_PAUSE        119 */
    VDK_UNKNOWN,        /* #define KEY_SCALE        120 */
    VDK_COMMA,          /* #define KEY_KPCOMMA      121 */
    VDK_UNKNOWN,        /* #define KEY_HANGEUL      122 */
    VDK_UNKNOWN,        /* #define KEY_HANJA        123 */
    VDK_UNKNOWN,        /* #define KEY_YEN          124 */
    VDK_UNKNOWN,        /* #define KEY_LEFTMETA     125 */
    VDK_UNKNOWN,        /* #define KEY_RIGHTMETA    126 */
    VDK_UNKNOWN,        /* #define KEY_COMPOSE      127 */
    VDK_UNKNOWN,        /* #define KEY_STOP         128 */
    VDK_UNKNOWN,        /* #define KEY_AGAIN        129 */
    VDK_UNKNOWN,        /* #define KEY_PROPS        130 */
    VDK_UNKNOWN,        /* #define KEY_UNDO         131 */
    VDK_UNKNOWN,        /* #define KEY_FRONT        132 */
    VDK_UNKNOWN,        /* #define KEY_COPY         133 */
    VDK_UNKNOWN,        /* #define KEY_OPEN         134 */
    VDK_UNKNOWN,        /* #define KEY_PASTE        135 */
    VDK_UNKNOWN,        /* #define KEY_FIND         136 */
    VDK_UNKNOWN,        /* #define KEY_CUT          137 */
    VDK_UNKNOWN,        /* #define KEY_HELP         138 */
    VDK_MENU,           /* #define KEY_MENU         139 */
    VDK_UNKNOWN,        /* #define KEY_CALC         140 */
    VDK_UNKNOWN,        /* #define KEY_SETUP        141 */
    VDK_SLEEP,          /* #define KEY_SLEEP        142 */
    VDK_WAKE,           /* #define KEY_WAKEUP       143 */
    VDK_UNKNOWN,        /* #define KEY_FILE         144 */
    VDK_UNKNOWN,        /* #define KEY_SENDFILE     145 */
    VDK_UNKNOWN,        /* #define KEY_DELETEFILE   146 */
    VDK_UNKNOWN,        /* #define KEY_XFER         147 */
    VDK_UNKNOWN,        /* #define KEY_PROG1        148 */
    VDK_UNKNOWN,        /* #define KEY_PROG2        149 */
    VDK_UNKNOWN,        /* #define KEY_WWW          150 */
    VDK_UNKNOWN,        /* #define KEY_MSDOS        151 */
    VDK_UNKNOWN,        /* #define KEY_COFFEE       152 */
    VDK_UNKNOWN,        /* #define KEY_DIRECTION    153 */
    VDK_UNKNOWN,        /* #define KEY_CYCLEWINDOWS 154 */
    VDK_UNKNOWN,        /* #define KEY_MAIL         155 */
    VDK_UNKNOWN,        /* #define KEY_BOOKMARKS    156 */
    VDK_UNKNOWN,        /* #define KEY_COMPUTER     157 */
    VDK_BACK,           /* #define KEY_BACK         158 */
    VDK_FORWARD,        /* #define KEY_FORWARD      159 */
    VDK_UNKNOWN,        /* #define KEY_CLOSECD      160 */
    VDK_UNKNOWN,        /* #define KEY_EJECTCD      161 */
    VDK_UNKNOWN,        /* #define KEY_EJECTCLOSECD 162 */
    VDK_UNKNOWN,        /* #define KEY_NEXTSONG     163 */
    VDK_UNKNOWN,        /* #define KEY_PLAYPAUSE    164 */
    VDK_UNKNOWN,        /* #define KEY_PREVIOUSSONG 165 */
    VDK_UNKNOWN,        /* #define KEY_STOPCD       166 */
    VDK_UNKNOWN,        /* #define KEY_RECORD       167 */
    VDK_UNKNOWN,        /* #define KEY_REWIND       168 */
    VDK_UNKNOWN,        /* #define KEY_PHONE        169 */
    VDK_UNKNOWN,        /* #define KEY_ISO          170 */
    VDK_UNKNOWN,        /* #define KEY_CONFIG       171 */
    VDK_UNKNOWN,        /* #define KEY_HOMEPAGE     172 */
    VDK_UNKNOWN,        /* #define KEY_REFRESH      173 */
    VDK_UNKNOWN,        /* #define KEY_EXIT         174 */
    VDK_UNKNOWN,        /* #define KEY_MOVE         175 */
    VDK_UNKNOWN,        /* #define KEY_EDIT         176 */
    VDK_UNKNOWN,        /* #define KEY_SCROLLUP     177 */
    VDK_UNKNOWN,        /* #define KEY_SCROLLDOWN   178 */
    VDK_UNKNOWN,        /* #define KEY_KPLEFTPAREN  179 */
    VDK_UNKNOWN,        /* #define KEY_KPRIGHTPAREN 180 */
    VDK_UNKNOWN,        /* #define KEY_NEW          181 */
    VDK_UNKNOWN,        /* #define KEY_REDO         182 */
    VDK_UNKNOWN,        /* #define KEY_F13          183 */
    VDK_UNKNOWN,        /* #define KEY_F14          184 */
    VDK_UNKNOWN,        /* #define KEY_F15          185 */
    VDK_UNKNOWN,        /* #define KEY_F16          186 */
    VDK_UNKNOWN,        /* #define KEY_F17          187 */
    VDK_UNKNOWN,        /* #define KEY_F18          188 */
    VDK_UNKNOWN,        /* #define KEY_F19          189 */
    VDK_UNKNOWN,        /* #define KEY_F20          190 */
    VDK_UNKNOWN,        /* #define KEY_F21          191 */
    VDK_UNKNOWN,        /* #define KEY_F22          192 */
    VDK_UNKNOWN,        /* #define KEY_F23          193 */
    VDK_UNKNOWN,        /* #define KEY_F24          194 */
    VDK_UNKNOWN,        /* 195  */
    VDK_UNKNOWN,        /* 196  */
    VDK_UNKNOWN,        /* 197  */
    VDK_UNKNOWN,        /* 198  */
    VDK_UNKNOWN,        /* 199  */
    VDK_UNKNOWN,        /* #define KEY_PLAYCD       200 */
    VDK_UNKNOWN,        /* #define KEY_PAUSECD      201 */
    VDK_UNKNOWN,        /* #define KEY_PROG3        202 */
    VDK_UNKNOWN,        /* #define KEY_PROG4        203 */
    VDK_UNKNOWN,        /* #define KEY_DASHBOARD    204 */
    VDK_UNKNOWN,        /* #define KEY_SUSPEND      205 */
    VDK_UNKNOWN,        /* #define KEY_CLOSE        206 */
    VDK_UNKNOWN,        /* #define KEY_PLAY         207 */
    VDK_UNKNOWN,        /* #define KEY_FASTFORWARD  208 */
    VDK_UNKNOWN,        /* #define KEY_BASSBOOST    209 */
    VDK_UNKNOWN,        /* #define KEY_PRINT        210 */
    VDK_UNKNOWN,        /* #define KEY_HP           211 */
    VDK_UNKNOWN,        /* #define KEY_CAMERA       212 */
    VDK_UNKNOWN,        /* #define KEY_SOUND        213 */
    VDK_UNKNOWN,        /* #define KEY_QUESTION     214 */
    VDK_UNKNOWN,        /* #define KEY_EMAIL        215 */
    VDK_UNKNOWN,        /* #define KEY_CHAT         216 */
    VDK_UNKNOWN,        /* #define KEY_SEARCH       217 */
    VDK_UNKNOWN,        /* #define KEY_CONNECT      218 */
    VDK_UNKNOWN,        /* #define KEY_FINANCE      219 */
    VDK_UNKNOWN,        /* #define KEY_SPORT        220 */
    VDK_UNKNOWN,        /* #define KEY_SHOP         221 */
    VDK_UNKNOWN,        /* #define KEY_ALTERASE     222 */
    VDK_UNKNOWN,        /* #define KEY_CANCEL       223 */
    VDK_UNKNOWN,        /* #define KEY_BRIGHTNESSDOWN 224 */
    VDK_UNKNOWN,        /* #define KEY_BRIGHTNESSUP 225 */
    VDK_UNKNOWN,        /* #define KEY_MEDIA        226 */
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

    vdk = (vdkPrivate) malloc(sizeof (struct _vdkPrivate));

    if (vdk == NULL)
    {
        return NULL;
    }

#if gcdSTATIC_LINK
    vdk->egl = NULL;
#else
    vdk->egl = dlopen("libEGL.so", RTLD_LAZY);
#endif

    vdk->display = (EGLNativeDisplayType) NULL;

    vdk->mice     = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
    vdk->keyboard = _DetectKeyboard();

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
    if (!Private)
    {
        return NULL;
    }

    /* Support only one display currently. */
    if (Private->display != (EGLNativeDisplayType) NULL)
    {
        return Private->display;
    }

    Private->display = fbGetDisplayByIndex(DisplayIndex);

    if (Private->display)
    {
        fbGetDisplayInfo(Private->display,
            &Private->xres, &Private->yres, NULL, NULL, NULL);
    }

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
    if (!Display)
    {
        return 0;
    }

    fbGetDisplayInfo(Display, Width, Height, Physical, Stride, BitsPerPixel);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyDisplay(
    vdkDisplay Display
    )
{
    fbDestroyDisplay(Display);

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
    NativeWindowType win;
    win = fbCreateWindow(Display, X, Y, Width, Height);

    return win;
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
    fbGetWindowInfo(Window, X, Y, Width, Height, BitsPerPixel, Offset);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyWindow(
    vdkWindow Window
    )
{
    fbDestroyWindow(Window);
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
    if (_vdk == NULL)
    {
        return 0;
    }

{
    if (_vdk->keyboard == -1)
    {
        static int frames;

        if (++frames > 100)
        {
            /* Detect keyboarad event 100 frames. */
            _vdk->keyboard = _DetectKeyboard();
            frames = 0;
        }
    }

    if (_vdk->keyboard >= 0)
    {
        ssize_t len;
        struct input_event evt;

        len = read(_vdk->keyboard, &evt, sizeof (evt));

        if (len < 0)
        {
            if (errno != EAGAIN)
            {
                /* Keyboard is disconnected? */
                close(_vdk->keyboard);
                _vdk->keyboard = _DetectKeyboard();
            }
        }
        else if (len == (ssize_t) sizeof (evt))
        {
            if (evt.type == EV_KEY)
            {
                int scancode;
                char key;

                if (evt.code < sizeof (_keycodes) / sizeof (_keycodes[0]))
                {
                    scancode = _keycodes[evt.code];
                }
                else
                {
                    scancode = VDK_UNKNOWN;
                }

                key = ((scancode < VDK_SPACE) || (scancode >= VDK_F1))
                    ? 0 : (char) scancode;

                Event->type = VDK_KEYBOARD;
                Event->data.keyboard.scancode = scancode;
                Event->data.keyboard.pressed   = (evt.value == 1);
                Event->data.keyboard.key = key;

                return 1;
            }
        }
    }
}

    if (_vdk->mice >= 0)
    {
        signed char mouse[3];
        static int x, y;
        static char left, right, middle;

        if (read(_vdk->mice, mouse, 3) == 3)
        {
            char l, m, r;

            x += mouse[1];
            y -= mouse[2];

            x = (x < 0) ? 0 : x;
            x = (x > _vdk->xres) ? _vdk->xres : x;

            y = (y < 0) ? 0 : y;
            y = (y > _vdk->yres) ? _vdk->yres : y;

            l = mouse[0] & 0x01;
            r = mouse[0] & 0x02;
            m = mouse[0] & 0x04;

            if ((l ^ left) || (r ^ right) || (m ^ middle))
            {
                Event->type                 = VDK_BUTTON;
                Event->data.button.left     = left      = l;
                Event->data.button.right    = right     = r;
                Event->data.button.middle   = middle    = m;
                Event->data.button.x        = x;
                Event->data.button.y        = y;
            }
            else
            {
                Event->type                 = VDK_POINTER;
                Event->data.pointer.x       = x;
                Event->data.pointer.y       = y;
            }

            return 1;
        }
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

VDKAPI vdkPixmap VDKLANG
vdkCreatePixmap(
    vdkDisplay Display,
    int Width,
    int Height,
    int BitsPerPixel
    )
{
    return fbCreatePixmapWithBpp(Display,
                                 Width, Height, BitsPerPixel);
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
    fbGetPixmapInfo(Pixmap,
                    Width, Height, BitsPerPixel, Stride, Bits);
    return 1;
}

VDKAPI void VDKLANG
vdkDestroyPixmap(
    vdkPixmap Pixmap
    )
{
    fbDestroyPixmap(Pixmap);
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
