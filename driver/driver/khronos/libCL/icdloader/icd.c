/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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
#include "icd_dispatch.h"
#include <stdlib.h>
#include <string.h>

KHRicdState khrIcdState = {0};

/* entrypoint to initialize the ICD and add all vendors */
void khrIcdInitialize(void)
{
    /* make sure we don't double-initialize */
    /* TODO: this should use an atomic exchange to be thread-safe */
    if (khrIcdState.initialized)
    {
        return;
    }
    khrIcdState.initialized = CL_TRUE;

    /* enumerate vendors present on the system */
    khrIcdOsVendorsEnumerate();
}

void khrIcdVendorAdd(const char *libraryName)
{
    void *library = NULL;
    cl_int result = CL_SUCCESS;
    pfn_clGetExtensionFunctionAddress p_clGetExtensionFunctionAddress = NULL;
    pfn_clIcdGetPlatformIDs p_clIcdGetPlatformIDs = NULL;
    cl_uint i = 0;
    cl_uint platformCount = 0;
    cl_platform_id *platforms = NULL;

    /* require that the library name be valid */
    if (!libraryName)
    {
        goto Done;
    }
    KHR_ICD_TRACE("attempting to add vendor %s...\n", libraryName);

    /* load its library and query its function pointers */
    library = khrIcdOsLibraryLoad(libraryName);
    if (!library)
    {
        KHR_ICD_TRACE("failed to load library %s\n", libraryName);
        goto Done;
    }

    /* get the library's clGetExtensionFunctionAddress pointer */
    p_clGetExtensionFunctionAddress = khrIcdOsLibraryGetFunctionAddress(library, "clGetExtensionFunctionAddress");
    if (!p_clGetExtensionFunctionAddress)
    {
        KHR_ICD_TRACE("failed to get function address clGetExtensionFunctionAddress\n");
        goto Done;
    }

    /* use that function to get the clIcdGetPlatformIDsKHR function pointer */
    p_clIcdGetPlatformIDs = p_clGetExtensionFunctionAddress("clIcdGetPlatformIDsKHR");
    if (!p_clIcdGetPlatformIDs)
    {
        KHR_ICD_TRACE("failed to get extension function address clIcdGetPlatformIDsKHR\n");
        goto Done;
    }

    /* query the number of platforms available and allocate space to store them */
    result = p_clIcdGetPlatformIDs(0, NULL, &platformCount);
    if (CL_SUCCESS != result)
    {
        KHR_ICD_TRACE("failed clIcdGetPlatformIDs\n");
        goto Done;
    }
    platforms = (cl_platform_id *)malloc(platformCount * sizeof(cl_platform_id) );
    if (!platforms)
    {
        KHR_ICD_TRACE("failed to allocate memory\n");
        goto Done;
    }
    memset(platforms, 0, platformCount * sizeof(cl_platform_id) );
    result = p_clIcdGetPlatformIDs(platformCount, platforms, NULL);
    if (CL_SUCCESS != result)
    {
        KHR_ICD_TRACE("failed clIcdGetPlatformIDs\n");
        goto Done;
    }

    /* for each platform, add it */
    for (i = 0; i < platformCount; ++i)
    {
        KHRicdVendor* vendor = NULL;
        char *suffix;
        size_t suffixSize;

        /* call clGetPlatformInfo on the returned platform to get the suffix */
        if (!platforms[i])
        {
            continue;
        }
        result = platforms[i]->dispatch->clGetPlatformInfo(
            platforms[i],
            CL_PLATFORM_ICD_SUFFIX_KHR,
            0,
            NULL,
            &suffixSize);
        if (CL_SUCCESS != result)
        {
            continue;
        }
        suffix = (char *)malloc(suffixSize);
        if (!suffix)
        {
            continue;
        }
        result = platforms[i]->dispatch->clGetPlatformInfo(
            platforms[i],
            CL_PLATFORM_ICD_SUFFIX_KHR,
            suffixSize,
            suffix,
            NULL);
        if (CL_SUCCESS != result)
        {
            free(suffix);
            continue;
        }

        /* allocate a structure for the vendor */
        vendor = (KHRicdVendor*)malloc(sizeof(*vendor) );
        if (!vendor)
        {
            free(suffix);
            KHR_ICD_TRACE("failed to allocate memory\n");
            continue;
        }
        memset(vendor, 0, sizeof(*vendor) );

        /* populate vendor data */
        vendor->library = khrIcdOsLibraryLoad(libraryName);
        if (!vendor->library)
        {
            free(suffix);
            free(vendor);
            KHR_ICD_TRACE("failed get platform handle to library\n");
            continue;
        }
        vendor->clGetExtensionFunctionAddress = p_clGetExtensionFunctionAddress;
        vendor->platform = platforms[i];
        vendor->suffix = suffix;

        /* add this vendor to the list of vendors at the tail */
        {
            KHRicdVendor **prevNextPointer = NULL;
            for (prevNextPointer = &khrIcdState.vendors; *prevNextPointer; prevNextPointer = &( (*prevNextPointer)->next) );
            *prevNextPointer = vendor;
        }

        KHR_ICD_TRACE("successfully added vendor %s with suffix %s\n", libraryName, suffix);

    }

Done:

    if (library)
    {
        khrIcdOsLibraryUnload(library);
    }
}

void khrIcdContextPropertiesGetPlatform(const cl_context_properties *properties, cl_platform_id *outPlatform)
{
    const cl_context_properties *property = (cl_context_properties *)NULL;
    *outPlatform = NULL;
    for (property = properties; property && property[0]; property += 2)
    {
        if ((cl_context_properties)CL_CONTEXT_PLATFORM == property[0])
        {
            *outPlatform = (cl_platform_id)property[1];
        }
    }
}

