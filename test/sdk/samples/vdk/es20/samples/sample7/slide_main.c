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


#include "vdk_sample_common.h"

extern VDKS_BOOL SlideInit();
extern void SlideRun();

/*
 * VDKS
*/
#ifndef ANDROID
vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth = 0;
int VDKS_Val_WindowsHeight = 0;

char* VDKS_ARG0 = NULL;

/*
*    Save Screen.
*/
#endif
int SaveResult = 0;

VDKS_BOOL Init()
{
    return SlideInit();
}

void Run ()
{
    SlideRun();

    if (SaveResult)
    {
        char szBMPFile[MAX_PATH + 1];
#ifndef ANDROID
        VDKS_Func_GetCurrentDir(szBMPFile);
#else
        strcpy(szBMPFile, "/sdcard/sample/sample7/");
#endif
        strcat(szBMPFile, "vdksample7_es20_result.bmp");
        VDKS_Func_SaveScreen(szBMPFile);
    }
#ifndef ANDROID
    vdkSwapEGL(&VdkEgl);
#endif
}

int main(int argc, char * argv[])
{
    int i = 0;
    int count = 1;
#ifndef ANDROID
    vdkEvent vdk_event;
#endif
    /*
     * Miscellaneous
    */
    if (argc == 2)
    {
        count = atoi(argv[1]);
    }
    else if(argc == 4)
    {
        count = atoi(argv[1]);
        VDKS_Val_WindowsWidth = atoi(argv[2]);
        VDKS_Val_WindowsHeight = atoi(argv[3]);
    }
    else if(argc == 5)
    {
        count = atoi(argv[1]);
        VDKS_Val_WindowsWidth = atoi(argv[2]);
        VDKS_Val_WindowsHeight = atoi(argv[3]);
        SaveResult = atoi(argv[4]);;
    }
#ifndef ANDROID
    /*
     * VDKS
    */
    VDKS_ARG0 = argv[0];

    memset(&VdkEgl, 0, sizeof(vdkEGL));

    VDKS_Init(&VdkEgl);

    vdkShowWindow(VdkEgl.window);
#endif
    /*
    * App Data
    */

    if (VDKS_TRUE != Init())
    {
        printf("main : Failed to init case.");
        return 1;
    }

    if(count)
    {
        for(i = 0; i < count; i++)
        {
            Run();
        }
    }
    else
    {
        int done = 0;
        while(!done)
        {
#ifndef ANDROID
            if(0 != vdkGetEvent(VdkEgl.window, &vdk_event))
            {
                if(vdk_event.type == VDK_CLOSE)
                {
                    done = 1;
                }
            }
            else
            {
                Run();
            }
#endif
        }
    }
#ifndef ANDROID
    vdkFinishEGL(&VdkEgl);
#endif
    return 0;
}



