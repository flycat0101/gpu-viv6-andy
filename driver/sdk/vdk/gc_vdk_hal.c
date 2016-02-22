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


#include "gc_vdk_hal.h"
#include <stdlib.h>
#include <stdio.h>
#if !gcdSTATIC_LINK
#if defined(WIN32)
#include <windows.h>
#ifdef UNDER_CE
#define Enter_Addr(Module,FunName ) GetProcAddressA(Module,FunName)
#else
#define Enter_Addr(Module,FunName ) GetProcAddress(Module,FunName)
#endif
HMODULE module;
#elif defined(LINUX) || defined(__QNXNTO__) || defined(__APPLE__)
#include <dlfcn.h>
#define Enter_Addr(Module,FunName ) dlsym(Module,FunName)
void *  module;
#endif
#else
#include "gc_hal_eglplatform.h"
#endif

GAL_API  * GAL = galNULL;

int HAL_Constructor() {
    GAL =(GAL_API *) malloc(sizeof(GAL_API));
    if(GAL == galNULL)
    {
        printf("Allocate memory failed\n");
        return 1;
    }
#if !gcdSTATIC_LINK
#if defined(WIN32)
    module = LoadLibrary(TEXT("libGAL.dll"));
#elif defined(LINUX)
    module = dlopen("libGAL.so", RTLD_NOW);
#elif defined(__APPLE__)
    module = dlopen("libGAL.dylib", RTLD_NOW);
#elif defined(__QNXNTO__)
    module = dlopen("libGAL-"gcdQNX_GAL_NAME_SUFFIX".so", RTLD_NOW);
#endif
    if(module == galNULL)
    {
        printf("Load LibGAL error\n");
        return 1;
    }
    GAL->GAL_LoadEGLLibrary=(_GAL_LoadEGLLibrary) Enter_Addr(module,"gcoOS_LoadEGLLibrary");
    GAL->GAL_FreeEGLLibrary=(_GAL_FreeEGLLibrary) Enter_Addr(module,"gcoOS_FreeEGLLibrary");
    GAL->GAL_GetDisplayByIndex=(_GAL_GetDisplayByIndex) Enter_Addr(module,"gcoOS_GetDisplayByIndex");
    GAL->GAL_GetDisplayInfo=(_GAL_GetDisplayInfo) Enter_Addr(module,"gcoOS_GetDisplayInfo");
    GAL->GAL_GetDisplayVirtual=(_GAL_GetDisplayVirtual) Enter_Addr(module,"gcoOS_GetDisplayVirtual");
    GAL->GAL_GetDisplayInfoEx=(_GAL_GetDisplayInfoEx) Enter_Addr(module,"gcoOS_GetDisplayInfoEx");
    GAL->GAL_GetDisplayVirtual=(_GAL_GetDisplayVirtual) Enter_Addr(module,"gcoOS_GetDisplayVirtual");
    GAL->GAL_GetDisplayBackbuffer=(_GAL_GetDisplayBackbuffer) Enter_Addr(module,"gcoOS_GetDisplayBackbuffer");
    GAL->GAL_SetDisplayVirtual=(_GAL_SetDisplayVirtual) Enter_Addr(module,"gcoOS_SetDisplayVirtual");
    GAL->GAL_DestroyDisplay=(_GAL_DestroyDisplay) Enter_Addr(module,"gcoOS_DestroyDisplay");

    /*******************************************************************************
    ** Windows. ********************************************************************
    */

    GAL->GAL_CreateWindow=(_GAL_CreateWindow) Enter_Addr(module,"gcoOS_CreateWindow");
    GAL->GAL_DestroyWindow=(_GAL_DestroyWindow) Enter_Addr(module,"gcoOS_DestroyWindow");
    GAL->GAL_DrawImage=(_GAL_DrawImage) Enter_Addr(module,"gcoOS_DrawImage");
    GAL->GAL_GetWindowInfoEx=(_GAL_GetWindowInfoEx) Enter_Addr(module,"gcoOS_GetWindowInfoEx");

    /*******************************************************************************
    ** Pixmaps. ********************************************************************
    */

    GAL->GAL_CreatePixmap=(_GAL_CreatePixmap) Enter_Addr(module,"gcoOS_CreatePixmap");
    GAL->GAL_GetPixmapInfo=(_GAL_GetPixmapInfo) Enter_Addr(module,"gcoOS_GetPixmapInfo");
    GAL->GAL_DrawPixmap=(_GAL_DrawPixmap) Enter_Addr(module,"gcoOS_DrawPixmap");
    GAL->GAL_DestroyPixmap=(_GAL_DestroyPixmap) Enter_Addr(module,"gcoOS_DestroyPixmap");

    /*******************************************************************************
    ** OS relative. ****************************************************************
    */

    GAL->GAL_LoadEGLLibrary=(_GAL_LoadEGLLibrary) Enter_Addr(module,"gcoOS_LoadEGLLibrary");
    GAL->GAL_FreeEGLLibrary=(_GAL_FreeEGLLibrary) Enter_Addr(module,"gcoOS_FreeEGLLibrary");
    GAL->GAL_ShowWindow=(_GAL_ShowWindow) Enter_Addr(module,"gcoOS_ShowWindow");
    GAL->GAL_HideWindow=(_GAL_HideWindow) Enter_Addr(module,"gcoOS_HideWindow");
    GAL->GAL_SetWindowTitle=(_GAL_SetWindowTitle) Enter_Addr(module,"gcoOS_SetWindowTitle");
    GAL->GAL_CapturePointer=(_GAL_CapturePointer) Enter_Addr(module,"gcoOS_CapturePointer");
    GAL->GAL_GetEvent=(_GAL_GetEvent) Enter_Addr(module,"gcoOS_GetEvent");
    GAL->GAL_CreateClientBuffer=(_GAL_CreateClientBuffer) Enter_Addr(module,"gcoOS_CreateClientBuffer");
    GAL->GAL_GetClientBufferInfo=(_GAL_GetClientBufferInfo) Enter_Addr(module,"gcoOS_GetClientBufferInfo");
    GAL->GAL_DestroyClientBuffer=(_GAL_DestroyClientBuffer) Enter_Addr(module,"gcoOS_DestroyClientBuffer");
    GAL->GAL_GetProcAddress=(_GAL_GetProcAddress) Enter_Addr(module,"gcoOS_GetProcAddress");

    /*----- Time -----------------------------------------------------------------*/
    /* Get the number of milliseconds since the system started. */

    GAL->GAL_GetTicks=(_GAL_GetTicks) Enter_Addr(module,"gcoOS_GetTicks");

    if(GAL->GAL_GetTicks == galNULL)
    {
        printf("Get Address Error\n");
        return 1;
    }
#else
    GAL->GAL_LoadEGLLibrary=(_GAL_LoadEGLLibrary)gcoOS_LoadEGLLibrary;
    GAL->GAL_FreeEGLLibrary=(_GAL_FreeEGLLibrary)gcoOS_FreeEGLLibrary;
    GAL->GAL_GetDisplayByIndex=(_GAL_GetDisplayByIndex)gcoOS_GetDisplayByIndex;
    GAL->GAL_GetDisplayInfo=(_GAL_GetDisplayInfo)gcoOS_GetDisplayInfo;
    GAL->GAL_GetDisplayVirtual=(_GAL_GetDisplayVirtual)gcoOS_GetDisplayVirtual;
    GAL->GAL_GetDisplayInfoEx=(_GAL_GetDisplayInfoEx)gcoOS_GetDisplayInfoEx;
    GAL->GAL_GetDisplayVirtual=(_GAL_GetDisplayVirtual)gcoOS_GetDisplayVirtual;
    GAL->GAL_GetDisplayBackbuffer=(_GAL_GetDisplayBackbuffer)gcoOS_GetDisplayBackbuffer;
    GAL->GAL_SetDisplayVirtual=(_GAL_SetDisplayVirtual)gcoOS_SetDisplayVirtual;
    GAL->GAL_DestroyDisplay=(_GAL_DestroyDisplay)gcoOS_DestroyDisplay;

    /*******************************************************************************
    ** Windows. ********************************************************************
    */

    GAL->GAL_CreateWindow=(_GAL_CreateWindow)gcoOS_CreateWindow;
    GAL->GAL_DestroyWindow=(_GAL_DestroyWindow)gcoOS_DestroyWindow;
    GAL->GAL_DrawImage=(_GAL_DrawImage)gcoOS_DrawImage;
    GAL->GAL_GetWindowInfoEx=(_GAL_GetWindowInfoEx)gcoOS_GetWindowInfoEx;
    /*******************************************************************************
    ** Pixmaps. ********************************************************************
    */

    GAL->GAL_CreatePixmap=(_GAL_CreatePixmap)gcoOS_CreatePixmap;
    GAL->GAL_GetPixmapInfo=(_GAL_GetPixmapInfo)gcoOS_GetPixmapInfo;
    GAL->GAL_DrawPixmap=(_GAL_DrawPixmap)gcoOS_DrawPixmap;
    GAL->GAL_DestroyPixmap=(_GAL_DestroyPixmap)gcoOS_DestroyPixmap;

    /*******************************************************************************
    ** OS relative. ****************************************************************
    */

    GAL->GAL_LoadEGLLibrary=(_GAL_LoadEGLLibrary)gcoOS_LoadEGLLibrary;
    GAL->GAL_FreeEGLLibrary=(_GAL_FreeEGLLibrary)gcoOS_FreeEGLLibrary;
    GAL->GAL_ShowWindow=(_GAL_ShowWindow)gcoOS_ShowWindow;
    GAL->GAL_HideWindow=(_GAL_HideWindow)gcoOS_HideWindow;
    GAL->GAL_SetWindowTitle=(_GAL_SetWindowTitle)gcoOS_SetWindowTitle;
    GAL->GAL_CapturePointer=(_GAL_CapturePointer)gcoOS_CapturePointer;
    GAL->GAL_GetEvent=(_GAL_GetEvent)gcoOS_GetEvent;
    GAL->GAL_CreateClientBuffer=(_GAL_CreateClientBuffer)gcoOS_CreateClientBuffer;
    GAL->GAL_GetClientBufferInfo=(_GAL_GetClientBufferInfo)gcoOS_GetClientBufferInfo;
    GAL->GAL_DestroyClientBuffer=(_GAL_DestroyClientBuffer)gcoOS_DestroyClientBuffer;
    GAL->GAL_GetProcAddress=(_GAL_GetProcAddress)gcoOS_GetProcAddress;

    /*----- Time -----------------------------------------------------------------*/
    /* Get the number of milliseconds since the system started. */

    GAL->GAL_GetTicks=(_GAL_GetTicks)gcoOS_GetTicks;
#endif
    return 0;
}

void HAL_Destructor()
{
#if !gcdSTATIC_LINK
    if(module != galNULL)
    {
#if defined(WIN32)
    FreeLibrary(module);
#elif defined(LINUX) || defined(__QNXNTO__)
    dlclose(module);
#endif
    }
#endif
    if(GAL != galNULL)
        free(GAL);
}
