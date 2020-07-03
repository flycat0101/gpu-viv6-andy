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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "gc_vdk.h"
#include "gc_hal_version.h"

#define vdkDEF2STRING_1(def)    # def
#define vdkDEF2STRING(def)      vdkDEF2STRING_1(def)

/*******************************************************************************
***** Version Signature *******************************************************/

const char * _VDK_VERSION = "\n\0$VERSION$"
                            vdkDEF2STRING(gcvVERSION_MAJOR) "."
                            vdkDEF2STRING(gcvVERSION_MINOR) "."
                            vdkDEF2STRING(gcvVERSION_PATCH) ":"
                            vdkDEF2STRING(gcvVERSION_BUILD)
#ifdef GIT_STRING
                           ":"vdkDEF2STRING(GIT_STRING)
#endif
                            "$\n";

/*******************************************************************************
** EGL support. ****************************************************************
*/

/*
    vdkSetupEGL

    Simple wrapper for an application to initialize both the VDK and EGL.

    PARAMETERS:

        int X
            X coordinate of the window.  If X is -1, the window will be centered
            on the display horizontally.

        int Y
            Y coordinate of the window.  If Y is -1, the window will be centered
            on the display vertically.

        int Width
            Width of the window.  If Width is 0, a window will be created with
            the width of the display.  The width is always clamped to the width
            of the display.

        int Height
            Height of the window.  If Height is 0, a window will be created with
            the height of the display.  The height is always clamped to the
            height of the display.

        const EGLint * ConfigurationAttributes
            Pointer to the EGL attributes for eglChooseConfiguration.

        const EGLint * SurfaceAttributes
            Pointer to the EGL attributes for eglCreateWindowSurface.

        const EGLint * ContextAttributes
            Pointer to the EGL attributes for eglCreateContext.

        vdkEGL * Egl
            Pointer to a vdkEGL structure that will be filled with VDK and EGL
            structure pointers.

    RETURNS:

        int
            1 if both the VDK and EGL have initialized successfully, or 0 if
            there is an error.
*/
VDKAPI int VDKLANG
vdkSetupEGL(
    int X,
    int Y,
    int Width,
    int Height,
    const EGLint * ConfigurationAttributes,
    const EGLint * SurfaceAttributes,
    const EGLint * ContextAttributes,
    vdkEGL * Egl
    )
{
    /* Valid configurations. */
    EGLint matchingConfigs;

    /* Make sure we have a valid Egl pointer. */
    if (Egl == NULL)
    {
        return 0;
    }

    /* Initialize the VDK. */
    if (Egl->vdk == NULL)
    {
        Egl->vdk = vdkInitialize();

        /* Test for error. */
        if (Egl->vdk == NULL)
        {
            return 0;
        }
    }

    if (Egl->display == 0)
    {
        /* Get the VDK display. */
        Egl->display = vdkGetDisplay(Egl->vdk);

        /* Test for error. */
        /* EGL display can be NULL (=EGL_DEFAULT_DISPLAY) in QNX. Hence, don't test for error. */
#if !defined(__QNXNTO__) && !defined(ANDROID)
        if (Egl->display == NULL)
        {
            return 0;
        }
#endif
    }

    /* Create the window. */
    if (Egl->window == 0)
    {
        Egl->window = vdkCreateWindow(Egl->display, X, Y, Width, Height);

        /* Test for error. */
        if (Egl->window == 0)
        {
            return 0;
        }
    }

    /* Get the EGL display. */
    if (Egl->eglDisplay == EGL_NO_DISPLAY)
    {
#ifdef __QNXNTO__
        Egl->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#else
        Egl->eglDisplay = eglGetDisplay(Egl->display);

        if (Egl->eglDisplay == EGL_NO_DISPLAY)
        {
            return 0;
        }
#endif

        /* Initialize the EGL and test for error. */
        if (!eglInitialize(Egl->eglDisplay,
                                &Egl->eglMajor,
                                &Egl->eglMinor))
        {
            return 0;
        }
    }

    if (ContextAttributes == VDK_CONTEXT_OPENGL)
    {
        if (!eglBindAPI(EGL_OPENGL_API))
        {
            return 0;
        }
    }
    else if (ContextAttributes == VDK_CONTEXT_OPENVG)
    {
        if (!eglBindAPI(EGL_OPENVG_API))
        {
            return 0;
        }
    }
    else
    {
        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            return 0;
        }
    }

    /* Choose a configuration and test for error. */
    if (Egl->eglConfig == NULL)
    {
        /* Default configuration. */
        EGLint configuration[] =
        {
            EGL_RED_SIZE,           8,
            EGL_GREEN_SIZE,         8,
            EGL_BLUE_SIZE,          8,
            EGL_DEPTH_SIZE,         24,
            EGL_SAMPLES,            0,
            EGL_RENDERABLE_TYPE,    EGL_DONT_CARE,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_ALPHA_SIZE,         0,
            EGL_STENCIL_SIZE,       0,
            EGL_NONE,
        };

        int defaultConfig = 0;

        /* When use VDK to run GL demo, it may need alpha channel and stencil buffer. */
        if (ConfigurationAttributes == VDK_CONFIG_RGBA8888_D24S8)
        {
            defaultConfig = 1;
            configuration[15] = 8;
            configuration[17] = 8;
        }

        /* When use VDK to run GL demo, it may need alpha channel and D16 Depth. */
        if (ConfigurationAttributes == VDK_CONFIG_RGBA8888_D16)
        {
            defaultConfig = 1;
            configuration[7] = 16;
            configuration[15] = 8;
        }

        /* Test for the default configuration. */
        if (ConfigurationAttributes == VDK_CONFIG_RGB888_D24)
        {
            defaultConfig = 1;
        }

        /* Test for RGB565 color. */
        if ((ConfigurationAttributes == VDK_CONFIG_RGB565_D16)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_D24)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_D24_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_AA)
        )
        {
            defaultConfig    = 1;
            configuration[1] = 5;
            configuration[3] = 6;
            configuration[5] = 5;
        }

        /* Test for no depth. */
        if ((ConfigurationAttributes == VDK_CONFIG_RGB565)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888_AA)
        )
        {
            defaultConfig    = 1;
            configuration[7] = 0;
        }

        /* Test for D16 depth. */
        if ((ConfigurationAttributes == VDK_CONFIG_RGB565_D16)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888_D16)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888_D16_AA)
        )
        {
            defaultConfig    = 1;
            configuration[7] = 16;
        }

        /* Test for Anti-Aliasing. */
        if ((ConfigurationAttributes == VDK_CONFIG_RGB565_D16_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB565_D24_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888_D16_AA)
        ||  (ConfigurationAttributes == VDK_CONFIG_RGB888_D24_AA)
        )
        {
            defaultConfig    = 1;
            configuration[9] = 4;
        }

        /* Test for OpenVG context. */
        if (ContextAttributes == VDK_CONTEXT_OPENVG)
        {
            defaultConfig     = 1;
            configuration[ 7] = EGL_DONT_CARE;
            configuration[ 9] = EGL_DONT_CARE;
            configuration[11] = EGL_OPENVG_BIT;
        }

        /* Test for OpenGL context. */
        if (ContextAttributes == VDK_CONTEXT_OPENGL)
        {
            defaultConfig     = 1;
            configuration[11] = EGL_OPENGL_BIT;
        }

        if (!eglChooseConfig(Egl->eglDisplay,
                                  defaultConfig
                                      ? configuration
                                      : ConfigurationAttributes,
                                  &Egl->eglConfig, 1,
                                  &matchingConfigs))
        {
            return 0;
        }

        /* Make sure we got at least 1 configuration. */
        if (matchingConfigs < 1)
        {
            return 0;
        }
    }

    /* Create the EGL surface. */
    if (Egl->eglSurface == EGL_NO_SURFACE)
    {
        Egl->eglSurface = eglCreateWindowSurface(Egl->eglDisplay,
                                                      Egl->eglConfig,
                                                      Egl->window,
                                                      SurfaceAttributes);

        /* Test for error. */
        if (Egl->eglSurface == EGL_NO_SURFACE)
        {
            return 0;
        }
    }

    /* Create the EGL context. */
    if (Egl->eglContext == EGL_NO_CONTEXT)
    {
        static const EGLint contextES20[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE,
        };
        static const EGLint contextGL40[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 4,
            EGL_NONE,
        };
        const EGLint *attib = NULL;

        if (ContextAttributes == VDK_CONTEXT_ES20)
        {
            attib = contextES20;
        }
        else if (ContextAttributes == VDK_CONTEXT_OPENGL)
        {
            attib = contextGL40;
        }
        else if (ContextAttributes == VDK_CONTEXT_OPENVG)
        {
            attib = NULL;
        }
        else
        {
            attib = ContextAttributes;
        }

        Egl->eglContext =
            eglCreateContext(Egl->eglDisplay, Egl->eglConfig, EGL_NO_CONTEXT, attib);
        /* Test for error. */
        if (Egl->eglContext == EGL_NO_CONTEXT)
        {
            return 0;
        }
    }

    /* Bind the surface and context to the display and test for error. */
    /* Skip eglMakeCurrent if eglContext is set to EGL_DONT_CARE       */
    if (Egl->eglContext != (EGLContext)EGL_DONT_CARE)
    {
        if (!eglMakeCurrent(Egl->eglDisplay,
                                 Egl->eglSurface,
                                 Egl->eglSurface,
                                 Egl->eglContext))
        {
            return 0;
        }
    }

    /* Success. */
    return 1;
}

/*
    vdkSwapEGL

    EGL wrapper to swap a surface to the display.

    PARAMETERS:

        vdkEGL * Egl
            Pointer to a vdkEGL structure filled in by vdkSetupEGL.

    RETURNS:

        int
            1 if the surface has been swapped successfully, or 0 if tehre is an
            erorr.
*/
VDKAPI int VDKLANG
vdkSwapEGL(
    vdkEGL * Egl
    )
{
    /* Make sure we have a valid Egl pointer. */
    if (Egl == NULL)
    {
        return 0;
    }

    /* Call EGL to swap the buffers. */
    return eglSwapBuffers(Egl->eglDisplay, Egl->eglSurface);
}

/*
    vdkFinishEGL

    EGL wrapper to release all resouces hold by the VDK and EGL.

    PARAMETERS:

        vdkEGL * Egl
            Pointer to a vdkEGL structure filled in by vdkSetupEGL.

    RETURNS:

        Nothing.
*/
VDKAPI void VDKLANG
vdkFinishEGL(
    vdkEGL * Egl
    )
{
    /* Only process a valid EGL pointer. */
    if (Egl != NULL)
    {
        /* Only process a valid EGLDisplay pointer. */
        if (Egl->eglDisplay != EGL_NO_DISPLAY)
        {
            /* Unbind everything from the EGL context. */
            eglMakeCurrent(Egl->eglDisplay, NULL, NULL, NULL);

            if (Egl->eglContext != EGL_NO_CONTEXT)
            {
                /* Destroy the EGL context. */
                eglDestroyContext(Egl->eglDisplay, Egl->eglContext);
                Egl->eglContext = EGL_NO_CONTEXT;
            }

            if (Egl->eglSurface != EGL_NO_SURFACE)
            {
                /* Destroy the EGL surface. */
                eglDestroySurface(Egl->eglDisplay, Egl->eglSurface);
                Egl->eglSurface = EGL_NO_SURFACE;
            }

            /* Terminate the EGL display. */
            eglTerminate(Egl->eglDisplay);
            Egl->eglDisplay = EGL_NO_DISPLAY;

            /* Release thread data. */
            eglReleaseThread();
        }

        if (Egl->window != 0)
        {
            /* Hide the window. */
            vdkHideWindow(
                Egl->window
                );

            /* Destroy the window. */
            vdkDestroyWindow(
                Egl->window
                );
            Egl->window = 0;
        }

        if (Egl->display != 0)
        {
            /* Destroy the display. */
            vdkDestroyDisplay(Egl->display);
            Egl->display = 0;
        }

        if (Egl->vdk != NULL)
        {
            /* Destroy the VDK. */
            vdkExit(Egl->vdk);
            Egl->vdk = NULL;
        }
    }
}

/*
    vdkSetSwapInterval

    EGL wrapper to set swap interval

    PARAMETERS:

        vdkEGL * Egl
            Pointer to a vdkEGL structure filled in by vdkSetupEGL.

        int Interval
            Interger that defines the swap interval.
    RETURNS:

        int
            1 if the set swap interval succeeds and 0 otherwise.
*/
VDKAPI int VDKLANG
vdkSetSwapIntervalEGL(
    vdkEGL * Egl,
    int Interval
    )
{
    /* Make sure we have a valid Egl pointer. */
    if (Egl == NULL)
    {
        return 0;
    }

    /* Call EGL to swap the buffers. */
    return eglSwapInterval(Egl->eglDisplay, Interval);
}

