/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"

#ifdef WIN32
#include <windows.h>

gctBOOL APIENTRY
DllMain(
    IN HINSTANCE Instance,
    IN DWORD Reason,
    IN LPVOID Reserved
)
{
    return gcvTRUE;
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
