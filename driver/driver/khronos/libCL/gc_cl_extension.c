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

#define __NEXT_MSG_ID__     012006

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

/* This is a generic function pointer type, whose name indicates it must
 * be cast to the proper type *and calling convention* before use.
 */

typedef void (* clFUNCTION)(void);

typedef struct _clsLOOKUP
{
    const char *    name;
    clFUNCTION      function;
}
clsLOOKUP;

#define clmMAKE_LOOKUP(function) \
    { #function, (clFUNCTION) function }

static clsLOOKUP clgLookup[] =
{
    clmMAKE_LOOKUP(clIcdGetPlatformIDsKHR),
    clmMAKE_LOOKUP(clGetGLContextInfoKHR),
    clmMAKE_LOOKUP(clCreateFromGLBuffer),
    clmMAKE_LOOKUP(clCreateFromGLTexture2D),
    clmMAKE_LOOKUP(clCreateFromGLTexture3D),
    clmMAKE_LOOKUP(clCreateFromGLRenderbuffer),
    clmMAKE_LOOKUP(clGetGLObjectInfo),
    clmMAKE_LOOKUP(clGetGLTextureInfo),
    clmMAKE_LOOKUP(clEnqueueAcquireGLObjects),
    clmMAKE_LOOKUP(clEnqueueReleaseGLObjects),
    { gcvNULL, gcvNULL }
};

static clFUNCTION
clfLookup(
    clsLOOKUP *     Lookup,
    const char *    Name
    )
{
    /* Test for lookup. */
    if (Lookup != gcvNULL)
    {
        /* Loop while there are entries in the lookup tabke. */
        while (Lookup->name != gcvNULL)
        {
            const char *p = Name;
            const char *q = Lookup->name;

            /* Compare the name and the lookup table. */
            while ((*p == *q) && (*p != '\0') && (*q != '\0'))
            {
                ++p;
                ++q;
            }

            /* See if we have a match. */
            if (*p == *q)
            {
                /* Return the function pointer. */
                return Lookup->function;
            }

            /* Next lookup entry. */
            ++Lookup;
        }
    }

    /* No match found. */
    return gcvNULL;
}

/*****************************************************************************\
|*                                OpenCL API                                 *|
\*****************************************************************************/

CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddress(
    const char *    FuncName
    )
{
    union cluFUNCTION
    {
        clFUNCTION  function;
        gctPOINTER  ptr;
    }
    function;

    gcmHEADER_ARG("FuncName=%s", FuncName);

    if ((FuncName == gcvNULL)
    ||  (FuncName[0] == '\0')
    )
    {
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    function.function = clfLookup(clgLookup, FuncName);

    gcmFOOTER_ARG("0x%x", function.ptr);
    return function.ptr;
}

#if BUILD_OPENCL_12
CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddressForPlatform(
    cl_platform_id Platform ,
    const char *   FuncName
    )
{
    union cluFUNCTION
    {
        clFUNCTION  function;
        gctPOINTER  ptr;
    }
    function;

    gcmHEADER_ARG("FuncName=%s", FuncName);

     if ((Platform == gcvNULL)
     || (Platform->objectType != clvOBJECT_PLATFORM))
    {
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    if ((FuncName == gcvNULL)
    ||  (FuncName[0] == '\0')
    )
    {
        gcmFOOTER_ARG("0x%x", gcvNULL);
        return gcvNULL;
    }

    function.function = clfLookup(clgLookup, FuncName);

    gcmFOOTER_ARG("0x%x", function.ptr);
    return function.ptr;
}
#endif


/*****************************************************************************\
|*                      OpenCL Extension cl_khr_icd API                      *|
\*****************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint              NumEntries,
                       cl_platform_id *     Platforms,
                       cl_uint *            NumPlatforms)
{
    static CLIicdDispatchTable* dispatchTable   = gcvNULL;
    gceSTATUS                   status          = CL_SUCCESS;

    gcmHEADER_ARG("NumEntries=%u", NumEntries);

    if (Platforms && NumEntries == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-012000: (clIcdGetPlatformIDsKHR) argument Platforms is not NULL but argument NumEntries is 0 in clGetPlatformIDs.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (dispatchTable == gcvNULL)
    {
        clfONERROR(cliIcdDispatchTableCreate(&dispatchTable));
    }

    clfGetDefaultPlatformID(Platforms);

    if (Platforms)
    {
        Platforms[0]->dispatch = dispatchTable;
    }

    if (NumPlatforms)
    {
        *NumPlatforms = 1;
    }

    gcmFOOTER_ARG("%d *Platforms=0x%x *NumPlatforms=%u",
                  CL_SUCCESS, gcmOPT_POINTER(Platforms),
                  gcmOPT_VALUE(NumPlatforms));
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}

/*****************************************************************************\
|*                  OpenCL Extension cl_khr_gl_sharing API                   *|
\*****************************************************************************/

extern CL_API_ENTRY cl_int CL_API_CALL
clGetGLContextInfoKHR(
    const cl_context_properties * Properties,
    cl_gl_context_info            ParamName,
    size_t                        ParamValueSize,
    void *                        ParamValue,
    size_t *                      ParamValueSizeRet
    )
{
    gctSIZE_T           retParamSize = 0;
    gctPOINTER          retParamPtr  = NULL;
    clsPlatformId_PTR   platform     = NULL;
    clsDeviceId_PTR     devices[8]   = {NULL};
    gctUINT             numDevices   = 0;
    gctINT              status       = 0;
    gctUINT             i            = 0;

    gcmHEADER_ARG("Properties=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Properties, ParamName, ParamValueSize, ParamValue);

    if (Properties)
    {
        for (i = 0; Properties[i]; i++)
        {
            switch (Properties[i])
            {
            case CL_CONTEXT_PLATFORM:
                i++;
                if (Properties[i] == 0 ||
                    ((clsPlatformId_PTR)Properties[i])->objectType != clvOBJECT_PLATFORM)
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-012001: (clGetGLContextInfoKHR) Properties[%d] not valid platform.\n", i);
                    clmRETURN_ERROR(CL_INVALID_PLATFORM);
                }
                break;

            case CL_GL_CONTEXT_KHR:
                i++;
                break;

            case CL_EGL_DISPLAY_KHR:
                i++;
                break;

            case CL_GLX_DISPLAY_KHR:
            case CL_WGL_HDC_KHR:
            case CL_CGL_SHAREGROUP_KHR:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-0120002: (clGetGLContextInfoKHR) Properties[%d] (0x%x) not supported.\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);

            default:
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-012003: (clGetGLContextInfoKHR) invalid Properties[%d] (0x%x).\n", i, Properties[i]);
                clmRETURN_ERROR(CL_INVALID_PROPERTY);
            }
        }
    }

    clGetDeviceIDs(gcvNULL, CL_DEVICE_TYPE_GPU, sizeof(devices), devices, &numDevices);
    gcmASSERT(numDevices > 0);
    platform = devices[0]->platform;

    switch (ParamName) {
    case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
        retParamSize = gcmSIZEOF(clsDeviceId_PTR);
        retParamPtr = &platform->devices;
        break;

    case CL_DEVICES_FOR_GL_CONTEXT_KHR:
        retParamSize = gcmSIZEOF(clsDeviceId_PTR) * platform->numDevices;
        retParamPtr = &platform->devices;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-012004: (clGetGLContextInfoKHR) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue) {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-012005: (clGetGLContextInfoKHR) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize) {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
    }

    if (ParamValueSizeRet) {
        *ParamValueSizeRet = retParamSize;
    }

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    /* Return the status. */
    gcmFOOTER_ARG("%d", status);
    return status;
}
