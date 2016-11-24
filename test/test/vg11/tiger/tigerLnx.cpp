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


#define old_tiger 1
#if old_tiger
#include <EGL/egl.h>
#include <VG/openvg.h>
#if USE_VDK
#include <gc_vdk.h>
#endif
#include "test_tiger_paths.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define COUNTOF(a) \
    (sizeof(a) / sizeof(a[0]))

// Our GL initialization function
VGboolean InitOVG(NativeDisplayType dpy, NativeWindowType win);

// Our Render function.
void Render();

// Our clean function. It will clean all used resources.
void Clean();

EGLDisplay eglDisplay;  // EGL display
EGLSurface eglSurface;     // EGL rendering surface
EGLContext eglContext;     // EGL rendering context

VGPath *tigerPaths;
VGPaint tigerStroke;
VGPaint tigerFill;

int width  = 640;
int height = 480;
int count  = 360;
int rgba  = 5650;

#ifndef SAMPLES
#    define SAMPLES 0
#endif

#define SHOW_FPS 1

#if !defined(LINUX)
#    undef SHOW_FPS
#    define SHOW_FPS 0
#endif




char szAppName[] = "OpenVG tiger";

int exposed = 0;

#if USE_VDK
vdkEGL egl;
#else
#ifdef EGL_API_FB
NativeDisplayType display;
NativeWindowType  window;
#endif
#endif

void catchKill(int signal)
{
    Clean();
}

#if SHOW_FPS
unsigned int get_time(void)
{
    timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}
#endif

void getCmdParam(int cmdArgc, char *cmdArgv[])
{
    switch (cmdArgc)
    {
        case 1:
        case 2:
            count = (cmdArgc == 2) ? atoi(cmdArgv[1]) : 360;
            break;
        case 3:
            width = atoi(cmdArgv[1]);
            height = atoi(cmdArgv[2]);
            break;
        case 4:
            count = atoi(cmdArgv[1]);
            width = atoi(cmdArgv[2]);
            height = atoi(cmdArgv[3]);
            break;
        case 5:
            count = atoi(cmdArgv[1]);
            width = atoi(cmdArgv[2]);
            height = atoi(cmdArgv[3]);
            rgba  = atoi(cmdArgv[4]);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    getCmdParam(argc, argv);
#if USE_VDK
    egl.vdk = vdkInitialize();
    if (egl.vdk == NULL) printf("ERROR: vdkInitialize() failed.\n");
    egl.display = vdkGetDisplay(egl.vdk);
    if (egl.display == NULL) printf("ERROR: vdkInitialize() failed.\n");
    egl.window = vdkCreateWindow(egl.display, -1, -1, width, height );
    if (egl.window == NULL) printf("ERROR: vdkCreateWindow failed.\n");

    vdkSetWindowTitle(egl.window, szAppName);

    vdkShowWindow(egl.window);

    if (!InitOVG((NativeDisplayType)(egl.display), (NativeWindowType)(egl.window))) return 1;
#if SHOW_FPS
    unsigned int start = 0;
    unsigned int diff = 0;
#endif

    for (int i = 0; i < count; ++i)
    {
#if SHOW_FPS
        if (i == 5)
            start = get_time();
#endif

        Render();
    }

#if SHOW_FPS
    if (count > 5)
    {
        diff = get_time() - start;
        printf("time:%u -- frames:%d -- fps:%f\n", diff, count-5, 1000.0f / diff * (count-5));
    }
    else
    {
        printf("count of frame < 6, can not compute fps!\n");
    }
#endif

    //Clean up all

    Clean();

    return 0;

#else
#ifdef EGL_API_FB
    display = fbGetDisplay(NULL);
    if (display == NULL) return 1;

    window = fbCreateWindow(display, 0, 0, -1, -1);
    if (window == NULL) return 1;

    int x, y;
    fbGetWindowGeometry(window, &x, &y, &width, &height);

    if (!InitOVG(display, window))
    {
        return 1;
    }

    unsigned int start = 0;
    unsigned int diff = 0;

    for (int i = 0; i < count; ++i)
    {
        if (i == 5)
            start = get_time();
        Render();
    }

    if (count > 5)
    {
        diff = get_time() - start;

        printf("time:%u -- frames:%d -- fps:%f\n", diff, count-5, 1000.0f / diff * (count-5));
    }
    else
    {
        printf("count of frame < 6, can not compute fps!\n");
    }
#else
    XSetWindowAttributes  xswa;

    Display *display;

    Window win;


    Screen  *screen;

    XEvent  event;


    int exposed = 0;

    display = XOpenDisplay(NULL);

    screen = XDefaultScreenOfDisplay(display);

    xswa.background_pixel = XWhitePixelOfScreen(screen);

    xswa.event_mask = KeyPressMask | StructureNotifyMask  | VisibilityChangeMask  |ResizeRedirectMask | ExposureMask | ButtonPressMask;

    win = XCreateWindow(display, XRootWindowOfScreen(screen),

                        width, height, width, height, 0,

                        DefaultDepthOfScreen(screen), InputOutput,

                        DefaultVisualOfScreen(screen), CWEventMask | CWBackPixel, &xswa);




    XMapWindow(display, win);



    signal(SIGINT | SIGKILL | SIGTERM, catchKill);



    if(!InitOVG(display, win)) return 1;

    unsigned int start = 0;
    unsigned int diff = 0;

    for (int i = 0; i < count; ++i)
    {
        if (i == 5)
            start = get_time();
        Render();
    }
    if (count > 5)
    {
        diff = get_time() - start;

        printf("time:%u -- frames:%d -- fps:%f\n", diff, count-5, 1000.0f / diff * (count-5));
    }
    else
    {
        printf("count of frame < 6, can not compute fps!\n");
    }

    //Clean up all

    Clean();

    return 0;
#endif
#endif
}

//----------------------------------------------------------------------------
VGboolean InitOVG(NativeDisplayType dpy, NativeWindowType win)
{
    EGLConfig configs[10];
    EGLint matchingConfigs;
    EGLBoolean result;
    EGLConfig  eglConfig = 0;
    int rBits, gBits, bBits, aBits;
    int i;

    /* configAttribs is a integers list that holds the desired format of
    our framebuffer. We will ask for a framebuffer with 24 bits of
    color and 16 bits of z-buffer. We also ask for a window buffer, not
    a pbuffer or pixmap buffer */
    rBits = rgba / 1000;
    gBits = (rgba % 1000) / 100;
    bBits = (rgba % 100) / 10;
    aBits = rgba % 10;

    const EGLint configAttribs[] =
    {
        EGL_RED_SIZE,          rBits,
        EGL_GREEN_SIZE,        gBits,
        EGL_BLUE_SIZE,         bBits,
        EGL_ALPHA_SIZE,        aBits,
        EGL_RENDERABLE_TYPE,   EGL_OPENVG_BIT,
        EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
        EGL_NONE,              EGL_NONE
    };

    // Ask for an available display.
    eglDisplay = eglGetDisplay(dpy);

    // Display initialization (we don't care about the OGLES version numbers).
    result = eglInitialize(eglDisplay, NULL, NULL);

    if (!result)
    {
        return VG_FALSE;
    }

    eglBindAPI(EGL_OPENVG_API);

    // Ask for the framebuffer configuration that best fits our parameters.
    result = eglChooseConfig(
        eglDisplay, configAttribs,
        &configs[0], COUNTOF(configs),
        &matchingConfigs
        );

    if (!result)
    {
        return VG_FALSE;
    }

    if (matchingConfigs < 1)
    {
        return VG_FALSE;
    }

    for (i = 0; i < matchingConfigs; i++)
    {
        int redSize = 0;
        int greenSize = 0;
        int blueSize = 0;
        int alphaSize = 0;


        eglGetConfigAttrib(eglDisplay, configs[i], EGL_RED_SIZE, &redSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_GREEN_SIZE, &greenSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_BLUE_SIZE, &blueSize);
        eglGetConfigAttrib(eglDisplay, configs[i], EGL_ALPHA_SIZE, &alphaSize);

        if ( (redSize == rBits)   &&
             (greenSize == gBits) &&
             (blueSize == bBits)   &&
             (alphaSize == aBits)
            )
        {
            eglConfig = configs[i];
            break;
        }
    }

    if (i == matchingConfigs)
    {
        printf("%s","no available config is choosed.\n");
        return VG_FALSE;
    }

    /* eglCreateWindowSurface creates an onscreen EGLSurface and returns
    a handle  to it. Any EGL rendering context created with a
    compatible EGLConfig can be used to render into this surface.*/
    eglSurface = eglCreateWindowSurface(
        eglDisplay, eglConfig, win, configAttribs
        );

    if (!eglSurface)
    {
        return VG_FALSE;
    }

    // Let's create our rendering context
    eglContext = eglCreateContext(eglDisplay, configs[0], 0, NULL);

    if (!eglContext)
    {
        return VG_FALSE;
    }

    // Now we will activate the context for rendering
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    tigerPaths = (VGPath*) malloc(pathCount * sizeof(VGPath));
    if (!tigerPaths)
    {
        return VG_FALSE;
    }

    for (int i = 0; i < pathCount; ++i)
    {
        tigerPaths[i] = vgCreatePath(
            VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
            1,0,0,0, VG_PATH_CAPABILITY_ALL
            );

        vgAppendPathData(
            tigerPaths[i], commandCounts[i],
            commandArrays[i], dataArrays[i]
        );
    }

    tigerStroke = vgCreatePaint();
    tigerFill = vgCreatePaint();
    vgSetPaint(tigerStroke, VG_STROKE_PATH);
    vgSetPaint(tigerFill, VG_FILL_PATH);

    vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    vgTranslate((VGfloat)(width/2 - 50), (VGfloat)(height/2 + 50));
//    vgScale(1.0,-1.0);
//    vgScale(0.5,0.5);
    if (width > height)    //Landscape
        vgScale(width / 640.0f, height / (-480.0f));
    else
        vgScale(width / 640.0f, height / (-852.0f));

    VGfloat clearColor[] = {1,1,1,1};
    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);

    return VG_TRUE;
}

//----------------------------------------------------------------------------

VGboolean  zoomOut = VG_FALSE;
VGint    scaleCount = 0;

void animateTiger()
{
    if (zoomOut)
    {
        vgScale((VGfloat)1.25, (VGfloat)1.25);
        if (0 == --scaleCount) zoomOut = VG_FALSE;

    }
    else
    {
        vgScale((VGfloat)0.8, (VGfloat)0.8);
        if (5 == ++scaleCount) zoomOut = VG_TRUE;
    }

    vgRotate(5);
}

void Render()
{
    const VGfloat * style;

    vgClear(0, 0, width, height);

    for (int i = 0; i < pathCount; ++i)
    {
        style = styleArrays[i];
        vgSetParameterfv(tigerStroke, VG_PAINT_COLOR, 4, &style[0]);
        vgSetParameterfv(tigerFill, VG_PAINT_COLOR, 4, &style[4]);
        vgSetf(VG_STROKE_LINE_WIDTH, style[8]);
        vgDrawPath((VGPath)tigerPaths[i], (VGint)style[9]);
    }

    eglSwapBuffers(eglDisplay, eglSurface);

    animateTiger();
}

//----------------------------------------------------------------------------

void Clean()
{
    for (int i = 0; i < pathCount; ++i)
    {
        vgDestroyPath(tigerPaths[i]);
    }

    if (tigerPaths)
    {
        free(tigerPaths);
    }

    vgDestroyPaint(tigerStroke);
    vgDestroyPaint(tigerFill);

    if(eglDisplay)
    {
        eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
        if (eglContext) eglDestroyContext(eglDisplay, eglContext);
        if (eglSurface) eglDestroySurface(eglDisplay, eglSurface);
        eglTerminate(eglDisplay);
    }

#if USE_VDK
    if (egl.window != NULL)  vdkDestroyWindow(egl.window);
    if (egl.display != NULL) vdkDestroyDisplay(egl.display);
    if (egl.vdk != NULL)     vdkExit(egl.vdk);
#else
#ifdef EGL_API_FB
    if (window != NULL)
    {
        fbDestroyWindow(window);
    }

    if (display != NULL)
    {
        fbDestroyDisplay(display);
    }
#endif
#endif
}
#endif
