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


/*******************************************************************************
**
**    ivdkGetApplication
**
**    Query a pointer to the applcation object.
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
**        gcsAPPLICATION_PTR
**            Pointer to the applcation object.
*/

gcsAPPLICATION_PTR
ivdkGetApplication(
    void
    )
{
    /* Define the application object. */
    static struct _gcsAPPLICATION app = {0};

    /* Return a pointer to the object. */
    return &app;
}


/*******************************************************************************
**
**    WinMain
**
**    Application entry point.
**
**    INPUT:
**
**        HINSTANCE Instance
**            Handle to the current instance of the application.
**
**        HINSTANCE PrevInstance
**            Handle to the previous instance of the application.
**
**        LPTSTR CommandLine
**            Pointer to a null-terminated string specifying the command line
**            for the application, excluding the program name.
**
**        int Show
**            Specifies how the window is to be shown.
**
**    OUTPUT:
**
**        Nothing.
**
**    RETURN:
**
**        int
**            Exit value.
*/

int WINAPI
WinMain(
    IN HINSTANCE Instance,
    IN HINSTANCE PrevInstance,
    IN LPTSTR CommandLine,
    IN int Show
    )
{
    /* Get a pointer to the application object. */
    gcsAPPLICATION_PTR app = ivdkGetApplication();

    /* Init the object. */
    app->instance    = Instance;
    app->commandLine = CommandLine;
    app->windowShow  = Show;

    /* Run the application */
    return vdkAppEntry();
}
