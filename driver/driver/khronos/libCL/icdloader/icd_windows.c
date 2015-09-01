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


/*
 * Copyright (c) 2009 The Khronos Group Inc.  All rights reserved.
 *
 * NOTICE TO KHRONOS MEMBER:
 *
 * NVIDIA has assigned the copyright for this object code to Khronos.
 * This object code is subject to Khronos ownership rights under U.S. and
 * international Copyright laws.
 *
 * Permission is hereby granted, free of charge, to any Khronos Member
 * obtaining a copy of this software and/or associated documentation files
 * (the "Materials"), to use, copy, modify and merge the Materials in object
 * form only and to publish, distribute and/or sell copies of the Materials
 * solely in object code form as part of conformant OpenCL API implementations,
 * subject to the following conditions:
 *
 * Khronos Members shall ensure that their respective ICD implementation,
 * that is installed over another Khronos Members' ICD implementation, will
 * continue to support all OpenCL devices (hardware and software) supported
 * by the replaced ICD implementation. For the purposes of this notice, "ICD"
 * shall mean a library that presents an implementation of the OpenCL API for
 * the purpose routing API calls to different vendor implementation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * KHRONOS AND NVIDIA MAKE NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
 * SOURCE CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  KHRONOS AND NVIDIA DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL KHRONOS OR NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH
 * THE USE OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting of
 * "commercial computer software" and "commercial computer software
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 */

#include "icd.h"
#include <stdio.h>
#include <windows.h>
#include <winreg.h>

/*
 *
 * Vendor enumeration functions
 *
 */

/* go through the list of vendors in the registry and call khrIcdVendorAdd */
/* for each vendor encountered */
void khrIcdOsVendorsEnumerate()
{
    LONG result;
    const char* platformsName = "SOFTWARE\\Khronos\\OpenCL\\Vendors";
    HKEY platformsKey = NULL;
    DWORD dwIndex;

    KHR_ICD_TRACE("Opening key HKLM\\%s...\n", platformsName);
#ifdef UNDER_CE
    result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SOFTWARE\\Khronos\\OpenCL\\Vendors"),
        0,
        KEY_READ,
        &platformsKey);
#else
    result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        platformsName,
        0,
        KEY_READ,
        &platformsKey);
#endif
    if (ERROR_SUCCESS != result)
    {
        KHR_ICD_TRACE("Failed to open platforms key %s, continuing\n", platformsName);
        return;
    }

    /* for each value */
    for (dwIndex = 0;; ++dwIndex)
    {
#ifdef UNDER_CE
        TCHAR wcszLibraryName[256];
#endif
        char cszLibraryName[256] = {0};
        DWORD dwLibraryNameSize = sizeof(cszLibraryName);
        DWORD dwLibraryNameType = 0;
        DWORD dwValue = 0;
        DWORD dwValueSize = sizeof(dwValue);

        /* read the value name */
        KHR_ICD_TRACE("Reading value %d...\n", dwIndex);
#ifdef UNDER_CE
        dwLibraryNameSize = sizeof(TCHAR) * 256;
        memset(wcszLibraryName, 0, dwLibraryNameSize);
        result = RegEnumValue(
              platformsKey,
              dwIndex,
              wcszLibraryName,
              &dwLibraryNameSize,
              NULL,
              &dwLibraryNameType,
              (LPBYTE)&dwValue,
              &dwValueSize);
        wcstombs(cszLibraryName, wcszLibraryName, _tcsclen(wcszLibraryName));
#else
        result = RegEnumValueA(
              platformsKey,
              dwIndex,
              cszLibraryName,
              &dwLibraryNameSize,
              NULL,
              &dwLibraryNameType,
              (LPBYTE)&dwValue,
              &dwValueSize);
#endif
        /* if RegEnumKeyEx fails, we are done with the enumeration */
        if (ERROR_SUCCESS != result)
        {
            KHR_ICD_TRACE("Failed to read value %d, done reading key.\n", dwIndex);
            break;
        }

        KHR_ICD_TRACE("Value %s found...\n", cszLibraryName);

        /* Require that the value be a DWORD and equal zero */
        if (REG_DWORD != dwLibraryNameType)
        {
            KHR_ICD_TRACE("Value not a DWORD, skipping\n");
            continue;
        }
        if (dwValue)
        {
            KHR_ICD_TRACE("Value not zero, skipping\n");
            continue;
        }

        /* add the library */
        khrIcdVendorAdd(cszLibraryName);
    }

    result = RegCloseKey(platformsKey);
    if (ERROR_SUCCESS != result)
    {
        KHR_ICD_TRACE("Failed to close platforms key %s, ignoring\n", platformsName);
    }
}

/*
 *
 * Dynamic library loading functions
 *
 */

/* dynamically load a library.  returns NULL on failure */
void *khrIcdOsLibraryLoad(const char *libraryName)
{
#ifdef UNDER_CE
    TCHAR name[256];
    memset(name, 0, sizeof(TCHAR) * 256);
    mbstowcs(name, libraryName, strlen(libraryName));
    return (void *)LoadLibrary(name);
#else
    return (void *)LoadLibraryA(libraryName);
#endif
}

/* get a function pointer from a loaded library.  returns NULL on failure. */
void *khrIcdOsLibraryGetFunctionAddress(void *library, const char *functionName)
{
#ifdef UNDER_CE
    TCHAR name[256];
#endif

    if (!library || !functionName)
    {
        return NULL;
    }
#ifdef UNDER_CE
    memset(name, 0, sizeof(TCHAR) * 256);
    mbstowcs(name, functionName, strlen(functionName));

    /* Get the address of the function. */
    return GetProcAddress((HMODULE)library, name);
#else
    return GetProcAddress((HMODULE)library, functionName);
#endif
}

/* unload a library. */
void khrIcdOsLibraryUnload(void *library)
{
    FreeLibrary( (HMODULE)library);
}

