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
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#if !defined(__USE_BSD)
#define __USE_BSD
#endif
#if !defined(__USE_MISC)
#define __USE_MISC
#endif
#include <dirent.h>

/*
 *
 * Vendor enumeration functions
 *
 */

/* go through the list of vendors in the two configuration files */
void khrIcdOsVendorsEnumerate(void)
{
    DIR *dir = NULL;
    struct dirent *dirEntry = NULL;
#ifdef ANDROID
    char *vendorPath = "/system/vendor/Khronos/OpenCL/vendors/";
#else
    char *vendorPath = "/etc/OpenCL/vendors/";
#endif

    /* open the directory */
    dir = opendir(vendorPath);
    if (NULL == dir)
    {
        KHR_ICD_TRACE("Failed to open path %s\n", vendorPath);
        goto Cleanup;
    }

    /* attempt to load all files in the directory */
    for (dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir) )
    {
        switch(dirEntry->d_type)
        {
        case DT_UNKNOWN:
        case DT_REG:
        case DT_LNK:
            {
                const char* extension = ".icd";
                FILE *fin = NULL;
                char* fileName = NULL;
                char* buffer = NULL;
                long bufferSize = 0;

                /* make sure the file name ends in .icd */
                if (strlen(extension) > strlen(dirEntry->d_name) )
                {
                    break;
                }
                if (strcmp(dirEntry->d_name + strlen(dirEntry->d_name) - strlen(extension), extension) )
                {
                    break;
                }

                /* allocate space for the full path of the vendor library name */
                fileName = malloc(strlen(dirEntry->d_name) + strlen(vendorPath) + 1);
                if (!fileName)
                {
                    KHR_ICD_TRACE("Failed allocate space for ICD file path\n");
                    break;
                }
                sprintf(fileName, "%s%s", vendorPath, dirEntry->d_name);

                /* open the file and read its contents */
                fin = fopen(fileName, "r");
                if (!fin)
                {
                    free(fileName);
                    break;
                }
                fseek(fin, 0, SEEK_END);
                bufferSize = ftell(fin);

                buffer = malloc(bufferSize+1);
                if (!buffer)
                {
                    free(fileName);
                    fclose(fin);
                    break;
                }
                memset(buffer, 0, bufferSize+1);
                fseek(fin, 0, SEEK_SET);
                if (bufferSize != (long)fread(buffer, 1, bufferSize, fin) )
                {
                    free(fileName);
                    free(buffer);
                    fclose(fin);
                    break;
                }
                /* ignore a newline at the end of the file */
                if (buffer[bufferSize-1] == '\n') buffer[bufferSize-1] = '\0';

                /* load the string read from the file */
                khrIcdVendorAdd(buffer);

                free(fileName);
                free(buffer);
                fclose(fin);
            }
            break;
        default:
            break;
        }
    }

Cleanup:

    /* free resources and exit */
    if (dir)
    {
        closedir(dir);
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
    return dlopen (libraryName, RTLD_NOW);
}

/* get a function pointer from a loaded library.  returns NULL on failure. */
void *khrIcdOsLibraryGetFunctionAddress(void *library, const char *functionName)
{
    return dlsym(library, functionName);
}

/* unload a library */
void khrIcdOsLibraryUnload(void *library)
{
    dlclose(library);
}

