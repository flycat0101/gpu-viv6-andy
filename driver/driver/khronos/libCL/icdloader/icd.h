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

#ifndef _ICD_H_
#define _ICD_H_

#include <CL/cl.h>
#include <CL/cl_ext.h>

/*
 * type definitions
 */

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_clIcdGetPlatformIDs)(
    cl_uint num_entries,
    cl_platform_id *platforms,
    cl_uint *num_platforms) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_clGetPlatformInfo)(
    cl_platform_id   platform,
    cl_platform_info param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void *(CL_API_CALL *pfn_clGetExtensionFunctionAddress)(
    const char *function_name)  CL_API_SUFFIX__VERSION_1_0;

typedef struct KHRicdVendorRec KHRicdVendor;
typedef struct KHRicdStateRec  KHRicdState;

/*
 * KHRicdVendor
 *
 * Data for a single ICD vendor platform.
 */
struct KHRicdVendorRec
{
    /* the loaded library object (true type varies on Linux versus Windows) */
    void *library;

    /* the extension suffix for this platform */
    char *suffix;

    /* function pointer to the ICD platform IDs extracted from the library */
    pfn_clGetExtensionFunctionAddress clGetExtensionFunctionAddress;

    /* the platform retrieved from clGetIcdPlatformIDsKHR */
    cl_platform_id platform;

    /* next vendor in the list vendors */
    KHRicdVendor *next;
};


/*
 * KHRicdState
 *
 * The global state of all vendors
 *
 * TODO: write access to this structure needs to be protected via a mutex
 */

struct KHRicdStateRec
{
    /* has this structure been initialized */
    cl_bool initialized;

    /* the list of vendors which have been loaded */
    KHRicdVendor *vendors;
};

/* the global state */
extern KHRicdState khrIcdState;

/*
 * khrIcd interface
 */

/* read vendors from system configuration and store the data */
/* loaded into khrIcdState.  this will call the OS-specific */
/* function khrIcdEnumerateVendors.  this is called at every */
/* dispatch function which may be a valid first call into the */
/* API (e.g, getPlatformIDs, etc). */
void khrIcdInitialize(void);

/* go through the list of vendors (in /etc/OpenCL.conf or through  */
/* the registry) and call khrIcdVendorAdd for each vendor encountered */
/* n.b, this call is OS-specific */
void khrIcdOsVendorsEnumerate(void);

/* add a vendor's implementation to the list of libraries */
void khrIcdVendorAdd(const char *libraryName);

/* dynamically load a library.  returns NULL on failure */
/* n.b, this call is OS-specific */
void *khrIcdOsLibraryLoad(const char *libraryName);

/* get a function pointer from a loaded library.  returns NULL on failure. */
/* n.b, this call is OS-specific */
void *khrIcdOsLibraryGetFunctionAddress(void *library, const char *functionName);

/* unload a library. */
/* n.b, this call is OS-specific */
void khrIcdOsLibraryUnload(void *library);

/* parse properties and determine the platform to use from them */
void khrIcdContextPropertiesGetPlatform(
    const cl_context_properties *properties,
    cl_platform_id *outPlatform);

/* internal tracing macros */
    #define KHR_ICD_TRACE(...)
    #define KHR_ICD_ASSERT(x)

/* if handle is NULL then return invalid_handle_error_code */
#define KHR_ICD_VALIDATE_HANDLE_RETURN_ERROR(handle,invalid_handle_error_code) \
    do \
    { \
        if (!handle) \
        { \
            return invalid_handle_error_code; \
        } \
    } while (0)

/* if handle is NULL then set errcode_ret to invalid_handle_error and return NULL */
/* (NULL being an invalid handle) */
#define KHR_ICD_VALIDATE_HANDLE_RETURN_HANDLE(handle,invalid_handle_error) \
    do \
    { \
        if (!handle) \
        { \
            if (errcode_ret) \
            { \
                *errcode_ret = invalid_handle_error; \
            } \
            return NULL; \
        } \
    } while (0)


#endif

