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


/*
 * Copyright (c) 2012 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software source and associated documentation files (the "Materials"),
 * to use, copy, modify and compile the Materials to create a binary under the
 * following terms and conditions:
 *
 * 1. The Materials shall NOT be distributed to any third party;
 *
 * 2. The binary may be distributed without restriction, including without
 * limitation the rights to use, copy, merge, publish, distribute, sublicense,
 * and/or sell copies, and to permit persons to whom the binary is furnished to
 * do so;
 *
 * 3. All modifications to the Materials used to create a binary that is
 * distributed to third parties shall be provided to Khronos with an
 * unrestricted license to use for the purposes of implementing bug fixes and
 * enhancements to the Materials;
 *
 * 4. If the binary is used as part of an OpenCL(TM) implementation, whether
 * binary is distributed together with or separately to that implementation,
 * then recipient must become an OpenCL Adopter and follow the published OpenCL
 * conformance process for that implementation, details at:
 * http://www.khronos.org/conformance/;
 *
 * 5. The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS IN
 * THE MATERIALS.
 *
 * OpenCL is a trademark of Apple Inc. used under license by Khronos.
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

static void
_ModuleDestructor(
    void
    )
{
    INT32 reference = 0;

    KHRicdVendor * vendor = NULL;

    if(platforms)
    {
        free(platforms);
        platforms = NULL;
    }
    for(vendor = khrIcdState.vendors; vendor != NULL; )
    {
        KHRicdVendor * next = vendor->next;
        if(vendor->suffix)
        {
            free(vendor->suffix);
            vendor->suffix = NULL;
        }
        if(vendor)
        {
            free(vendor);
        }
        vendor = next;
    }
}

BOOL WINAPI
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
        _ModuleDestructor();
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return 1;
}
