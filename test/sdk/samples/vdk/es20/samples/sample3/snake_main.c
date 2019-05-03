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


/*
*    History : 2009.01.25, Created by qizhuang.liu.
*              2009.02.01, Finished by qizhuang.liu.
*/
#include "vdk_sample_common.h"

/*
 * VDKS
*/
#ifndef ANDROID
vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth = 320;
int VDKS_Val_WindowsHeight = 240;

char* VDKS_ARG0 = NULL;
#endif
/*
*    Outside Routine
*/
extern VDKS_BOOL    SnakeInit();
extern void            SnakePass();

/*
*    Save Screen.
*/

int SaveResult = 0;

VDKS_BOOL Init()
{
    return SnakeInit();
}

void Run ()
{
    SnakePass();

    if (SaveResult)
    {
        char szBMPFile[MAX_PATH + 1];
#ifndef ANDROID
        VDKS_Func_GetCurrentDir(szBMPFile);
#else
        strcpy(szBMPFile,"/sdcard/sample/sample3/");
#endif
        strcat(szBMPFile, "vdksample3_es20_result.bmp");
        VDKS_Func_SaveScreen(szBMPFile);
    }
#ifndef ANDROID
    vdkSwapEGL(&VdkEgl);
#endif
}
#ifndef ANDROID
int main(int argc, char * argv[])
{
    int i = 0;
    int count = 1000;

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

    /*
     * VDKS
    */
    VDKS_ARG0 = argv[0];

    memset(&VdkEgl, 0, sizeof(vdkEGL));

    VDKS_Init(&VdkEgl);

    vdkShowWindow(VdkEgl.window);

    /*
     * App Data
    */

    if (VDKS_TRUE != Init())
    {
        printf("main : Failed to init case.");
        return 1;
    }

    for(i = 0; i < count; i++)
    {
        Run();
    }

    vdkFinishEGL(&VdkEgl);

    return 0;
}
#endif
