/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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
        if(clgDispatchTable)
        {
            free(clgDispatchTable);
            clgDispatchTable = NULL;
        }

        if(clgDevices) gcmOS_SAFE_FREE(gcvNULL, clgDevices);

        if(clgGlobalId)
        {
            gcoOS_AtomDestroy(gcvNULL, clgGlobalId);
        }

        break;

    case DLL_THREAD_DETACH:
        break;
    }
    return gcvTRUE;
}
#elif defined(__linux__) || defined(ANDROID) || defined(__QNXNTO__)
static void __attribute__((destructor)) _ModuleDestructor(void);

static void _ModuleDestructor(void)
{
    if(clgDispatchTable)
    {
        free(clgDispatchTable);
        clgDispatchTable = NULL;
    }

    if(clgDevices) gcmOS_SAFE_FREE(gcvNULL, clgDevices);

    if(clgGlobalId)
    {
        gcoOS_AtomDestroy(gcvNULL, clgGlobalId);
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
#ifdef GIT_STRING
                           ":"gcmTXT2STR(GIT_STRING)
#endif
                           "$\n";

/******************************************************************************\
****************************** Global Variables *******************************
\******************************************************************************/

/* Helper string for query functions to return empty null-terminated string */
const gctSTRING      clgEmptyStr = "";
