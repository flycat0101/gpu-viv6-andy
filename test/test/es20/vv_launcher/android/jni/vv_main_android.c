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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include <nativehelper/JNIHelp.h>
#include <nativehelper/jni.h>
#include "com_vivantecorp_graphics_Native.h"

#include <glutils/log.h>
#include "LauncherApp.h"

int  appWidth    = 640;
int  appHeight    = 480;
int  flipY        = 0;


int
Run(
    char const * Command
    )
{
    /* TODO: Current do not know how to start a external application. */
    return 0;
}


#    define RETURN_SUCCESS   0
#    define RETURN_ERROR 1

static int finished;
static volatile int key;
static LauncherApp * app;
static unsigned int lastTime;

JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_init
  (JNIEnv * Env, jobject jo, jint id, jint w, jint h)
{
    key = 0;
    appWidth  = w;
    appHeight = h;

    // Create the application class.
    app = LauncherAppConstruct(appWidth, appHeight);
    flipVertical(app, flipY);

    // Get the current time.
    {
    struct timeval tv;

    /* Return the time of day in milliseconds. */
    gettimeofday(&tv, 0);
    lastTime = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    }

    // Main loop.
    finished = 0;
}


JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_fini
  (JNIEnv * Env, jobject jo)
{
    // Delete the application.
    LauncherAppDestroy(app);
    app = NULL;
}


enum
{
    KEYCODE_BACK = 0x04,
    KEYCODE_0 = 0x07,
    KEYCODE_1,
    KEYCODE_2,
    KEYCODE_3,
    KEYCODE_4,
    KEYCODE_5,
    KEYCODE_6,
    KEYCODE_7,
    KEYCODE_8,
    KEYCODE_9,
    KEYCODE_Q = 0x2d
};

JNIEXPORT jboolean JNICALL Java_com_vivantecorp_graphics_Native_repaint
  (JNIEnv * Env, jobject jo)
{
    if (!finished)
    {
        // Test for events.
        if (key != 0)
        {
            // Test for keyboard event.
            // Check the scancode.
            switch (key)
            {
            case KEYCODE_BACK:
                // Exit program.
                finished = 1;
                break;

            case KEYCODE_0:
            case KEYCODE_1:
            case KEYCODE_2:
            case KEYCODE_3:
            case KEYCODE_4:
            case KEYCODE_5:
            case KEYCODE_6:
            case KEYCODE_7:
            case KEYCODE_8:
            case KEYCODE_9:
                {
                    // Numeric key selects a menu entry.
                    int k = key - KEYCODE_0;
                    LauncherAppSelect(app, k - 1);
                }
                break;

            case KEYCODE_Q:
                finished = 1;

            default:
                break;
            }

            key = 0;
        }
        else
        {
            // Compute the time delta in seconds.
            char const * command;
            unsigned int now;
            float delta;
            struct timeval tv;

            /* Return the time of day in milliseconds. */
            gettimeofday(&tv, 0);
            now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

            delta = (float)(now - lastTime) / 1000.0f;

            // Clamp delta to 1/20th of a second.
            if (delta > 1.0f / 20.0f) delta = 1.0f / 20.0f;
            lastTime = now;

            // Move the application to the next tick.
            command = LauncherAppTick(app, delta);

            // Do we need to launch an application?
            if (command != NULL)
            {
                // Launch the application.
                if (!Run(command))
                {
                    LogError("Command %s could not be executed\n", command);
                }
                // Unselect current menu item.
                // TODO: Enable this after launch implemented.
                // LauncherAppSelect(app, -1);
            }

            // Render one frame.
            LauncherAppDraw(app, appWidth, appHeight);
        }

        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_key
  (JNIEnv * Env, jobject jo, jint k, jboolean down)
{
    if (down != 0)
    {
        key = k;
    }
}

JNIEXPORT void JNICALL Java_com_vivantecorp_graphics_Native_touch
  (JNIEnv * Env, jobject jo, jint x, jint y, jboolean down)
{
    if (down == 1)
    {
        /*
        Let me simulate a key.
        Area:
            1 2 3
            4 5 6
            7 8 9
        */

        int hori = x / (appWidth  / 3);
        int vert = y / (appHeight / 3);

        key = KEYCODE_1 + vert * 3 + hori;
    }
}

