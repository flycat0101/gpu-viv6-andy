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


#include "render.h"
#include <stdlib.h>
#include <stdio.h>

// Global variable that holds the VDK information.
vdkEGL egl;

bool pauseFlag = false;
int x = -1;
int y = -1;
int width = 0;
int height = 0;
int sampling = 0;
int frameCount = 0;

int argCount = 6;
char argSpec = '-';
char argNames[] = {'x', 'y', 'w', 'h', 's', 'f'};
char argValues[][255] = {
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
};
char argDescs[][255] = {
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
};
int noteCount = 1;
char argNotes[][255] = {
    "Exit: [ESC] or the frame count reached."
};

// Program entry point.
int main(int argc, char * argv[])
{
    // EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.
    EGLint configAttribs[] =
    {
        EGL_SAMPLES,        4,
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     16,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE,
    };

    // Parse the command line.
    if (!ParseCommandLine(argc, argv))
    {
        PrintHelp();
        return 0;
    }

    // Set multi-sampling.
    configAttribs[1] = sampling;

    // Initialize VDK, EGL, and GLES.
    if (!vdkSetupEGL(x, y, width, height, configAttribs, NULL, NULL, &egl))
    {
        return 1;
    }

    // Set window title and show the window.
    vdkSetWindowTitle(egl.window, "OpenGL ES 1.1 Tutorial 7");
    vdkShowWindow(egl.window);

    // Initialize the GLES.
    if (InitGLES())
    {
        unsigned int start = vdkGetTicks();
        int frames = 0;

        // Main loop.
        for (bool done = false; !done;)
        {
            // Get an event.
            vdkEvent event;
            if (vdkGetEvent(egl.window, &event))
            {
                // Test for Keyboard event.
                if ((event.type == VDK_KEYBOARD)
                &&  event.data.keyboard.pressed
                )
                {
                    // Test for key.
                    switch (event.data.keyboard.scancode)
                    {
                    case VDK_SPACE:
                        // Use SPACE to pauseFlag.
                        pauseFlag = !pauseFlag;
                        break;

                    case VDK_ESCAPE:
                        // Use ESCAPE to quit.
                        done = true;
                        break;

                    default:
                        break;
                    }
                }

                // Test for Close event.
                else if (event.type == VDK_CLOSE)
                {
                    done = true;
                }
            }
            else if (!pauseFlag)
            {
                unsigned int time;

                // Render one frame if there is no event.
                Render();
                frames ++;

                if (!(frames & 0xFF))
                {
                    if ((time = vdkGetTicks()) - start > 1000)
                    {
                        printf("fps: %.2f\n", frames * 1000.0f / (time - start));

                        frames = 0;
                        start = time;
                    }
                }

                if ((frameCount > 0) && (--frameCount == 0))
                {
                    done = true;
                }
            }
        }
    }

    // Cleanup the GLES.
    CleanGLES();

    // Clean up the VDK, EGL, and GLES.
    vdkFinishEGL(&egl);
    return 0;
}

/*******************************************************************************
*******************************************************************************/

int ParseCommandLine(int argc, char * argv[])
{
    int result = 1;
    // Walk all command line arguments.
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'x':
                // x<x> for x position (defaults to -1).
                if (++i < argc)
                    x = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'y':
                // y<y> for y position (defaults to -1).
                if (++i < argc)
                    y = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'w':
                // w<width> for width (defaults to 0).
                if (++i < argc)
                    width = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'h':
                // h<height> for height (defaults to 0).
                if (++i < argc)
                    height = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 's':
                // s<samples> for samples (defaults to 0).
                if (++i < argc)
                    sampling = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'f':
                // f<count> for number of frames (defaults to 0).
                if (++i < argc)
                    frameCount = atoi(&argv[i][0]);
                else
                    result = 0;
                break;
            default:
                result = 0;
                break;
            }

            if (result == 0)
            {
                break;
            }
        }
        else
        {
            result = 0;
            break;
        }
    }

    return result;
}

void PrintHelp()
{
    int i;
    printf("Usage: ");
    printf("command + [arguments]\n");
    printf("Argument List:\n");

    for (i = 0; i < argCount; i++)
    {
        printf("\t");
        printf("%c%c %s\t%s\n", argSpec, argNames[i], argValues[i], argDescs[i]);
    }

    for (i = 0; i < noteCount; i++)
    {
        printf("%s\n", argNotes[i]);
    }
}
