/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"
#if defined(__QNXNTO__)
#include <stdlib.h>
#endif

#ifdef WIN32
#include <windows.h>

void _DetachCL(void)
{
    if(clgDispatchTable)
    {
        free(clgDispatchTable);
        clgDispatchTable = NULL;
    }

    if(clgDevices)
    {
        gcmOS_SAFE_FREE(gcvNULL, clgDevices);
    }

    if(clgGlobalId)
    {
        gcoOS_AtomDestroy(gcvNULL, clgGlobalId);
    }

    if(clgDefaultPlatform)
    {
        if(clgDefaultPlatform->unloadCompiler)
        {
            /* Destroy CL patch library. Move it at Process exit */
            gcmVERIFY_OK(gcFreeCLPatchLibrary());

            /* Destroy vir intrinsic library. */
            gcmVERIFY_OK(vscFreeVirIntrinsicLib());

            (*clgDefaultPlatform->unloadCompiler)();

            gcoOS_FreeLibrary(gcvNULL, clgDefaultPlatform->dll);

            clgDefaultPlatform->dll = gcvNULL;
            clgDefaultPlatform->compiler = gcvNULL;
            clgDefaultPlatform->compiler11 = gcvNULL;
            clgDefaultPlatform->loadCompiler = gcvNULL;
            clgDefaultPlatform->unloadCompiler = gcvNULL;
        }

        if(clgDefaultPlatform->compilerMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, clgDefaultPlatform->compilerMutex);
        }

        if(clgDefaultPlatform->vscCoreSysCtx.hPrivData)
        {
            vscDestroyPrivateData(&clgDefaultPlatform->vscCoreSysCtx, clgDefaultPlatform->vscCoreSysCtx.hPrivData);
        }
    }
}

gctBOOL APIENTRY
DllMain(
    IN HINSTANCE Instance,
    IN DWORD Reason,
    IN LPVOID Reserved
)
{
    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        _DetachCL();
        break;

    case DLL_THREAD_DETACH:
        break;
    }
    return gcvTRUE;
}
#elif defined(__linux__) || defined(ANDROID) || defined(__QNXNTO__)
static void __attribute__((destructor)) _ModuleDestructor(void);

void _DetachCL(void);
static void _ModuleDestructor(void)
{
    _DetachCL();
}

void _DetachCL(void)
{
    if(clgDispatchTable)
    {
        free(clgDispatchTable);
        clgDispatchTable = NULL;
    }

    if(clgDevices)
    {
        gcmOS_SAFE_FREE(gcvNULL, clgDevices);
    }

    if(clgGlobalId)
    {
        gcoOS_AtomDestroy(gcvNULL, clgGlobalId);
    }
    if(clgDefaultPlatform)
    {
        if(clgDefaultPlatform->unloadCompiler)
        {
            /* Destroy CL patch library. */
            gcmVERIFY_OK(gcFreeCLPatchLibrary());

            /* Destroy vir intrinsic library. */
            gcmVERIFY_OK(vscFreeVirIntrinsicLib());

            (*clgDefaultPlatform->unloadCompiler)();

            gcoOS_FreeLibrary(gcvNULL, clgDefaultPlatform->dll);

            clgDefaultPlatform->dll = gcvNULL;
            clgDefaultPlatform->compiler = gcvNULL;
            clgDefaultPlatform->compiler11 = gcvNULL;
            clgDefaultPlatform->loadCompiler = gcvNULL;
            clgDefaultPlatform->unloadCompiler = gcvNULL;
        }

        if(clgDefaultPlatform->compilerMutex)
        {
            gcoOS_DeleteMutex(gcvNULL, clgDefaultPlatform->compilerMutex);
        }

        if(clgDefaultPlatform->vscCoreSysCtx.hPrivData)
        {
            vscDestroyPrivateData(&clgDefaultPlatform->vscCoreSysCtx, clgDefaultPlatform->vscCoreSysCtx.hPrivData);
        }
    }
}
#endif

/******************************************************************************\
|***** Version Signature ******************************************************|
\******************************************************************************/

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * clgVersion = "\n\0$VERSION$"
                           gcmTXT2STR(gcvVERSION_MAJOR) "."
                           gcmTXT2STR(gcvVERSION_MINOR) "."
                           gcmTXT2STR(gcvVERSION_PATCH) ":"
                           gcmTXT2STR(gcvVERSION_BUILD)
                           "$\n";

/******************************************************************************\
****************************** Global Variables *******************************
\******************************************************************************/

/* Helper string for query functions to return empty null-terminated string */
const gctSTRING      clgEmptyStr = "";
