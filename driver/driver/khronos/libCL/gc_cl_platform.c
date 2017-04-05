/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     000005

static struct _cl_platform_id _platform =
{
    gcvNULL,                    /* Dispatch. */
    clvOBJECT_PLATFORM,         /* Object type. */
    0,                          /* Id. */
    gcvNULL,                    /* Reference count object. */

    0,                          /* Number of devices. */
    gcvNULL,                    /* Devices. */
    gcvNULL,                    /* Contexts. */
    "Vivante OpenCL Platform"   /* Platform name. */,
    "Vivante Corporation"       /* Vendor name. */,
#if BUILD_OPENCL_12
    "OpenCL 1.2 V" gcvVERSION_STRING               /* Version name. */,
    "OpenCL C 1.2 "             /* C Version name. */,
#else
    "OpenCL 1.1 V" gcvVERSION_STRING               /* Version name. */,
    "OpenCL C 1.1 "             /* C Version name. */,
#endif
#if BUILD_OPENCL_FP
    "FULL_PROFILE",             /* Profile          */
#else
    "EMBEDDED_PROFILE",         /* Profile          */
#endif
    "cl_khr_icd",               /* Extensions. */
    "viv",                      /* ICD Extensioin suffix. */

    {{0}},                       /* hwcfg */
    gcvPATCH_INVALID,           /* patchId */
    gcvNULL,                    /* Compiler mutex. */
    gcvNULL,                    /* Compiler dll. */
    gcvNULL,                    /* Compiler pointer. */
    gcvNULL,                    /* loadCompiler pointer. */
    gcvNULL,                    /* unloadCompiler pointer. */
};

cl_platform_id clgDefaultPlatform = gcvNULL;

gcsATOM_PTR clgGlobalId = gcvNULL;

gceTRACEMODE vclTraceMode = gcvTRACEMODE_NONE;

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
cl_bool clfInitTracerDispatchTable()
{
    cl_bool ret = CL_TRUE;

    switch (vclTraceMode)
    {
    case gcvTRACEMODE_FULL:
        {
            /* Set veglTracerDispatchTable[] to simple log functions */
            vclTracerDispatchTable = vclLogFunctionTable;
        }
        break;

     case gcvTRACEMODE_NONE:
     default:
         {
             /* Clear veglTracerDispatchTable[] */
             memset(&vclTracerDispatchTable, 0, sizeof(clsTracerDispatchTableStruct));
             break;
         }
    }

    return ret;
}

void
clfSetTraceMode(
    void
    )
{
    static gctBOOL Once = gcvFALSE;

    if (!Once)
    {
        gctSTRING tracemode = gcvNULL;

        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_TRACE", &tracemode)) && tracemode)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "0")))
            {
                vclTraceMode = gcvTRACEMODE_NONE;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "1")))
            {
                vclTraceMode = gcvTRACEMODE_FULL;
            }
            else
            {
                gcmPRINT("OCL: unsupported trace mode");
            }

            clfInitTracerDispatchTable();
        }

        Once = gcvTRUE;
    }
}

void
clfGetDefaultPlatformID(
    clsPlatformId_PTR * Platform
    )
{
    gctINT           status;

    gcmHEADER_ARG("Platform=%p", Platform);

    if (clgDefaultPlatform == gcvNULL)
    {
        static gcsATOM_PTR delay = gcvNULL;
        static gctINT32  delayCount = -1;
#if BUILD_OPENCL_FP
        gctSTRING epProfile = "EMBEDDED_PROFILE";
        gceCHIPMODEL  chipModel;
        gctUINT32 chipRevision;
        gctBOOL chipEnableFP = gcvFALSE;
#endif

        clgDefaultPlatform = &_platform;
        if (delay == gcvNULL)
        {
           clmONERROR(gcoOS_AtomConstruct(gcvNULL, &delay), CL_INVALID_VALUE);
           clmONERROR(gcoOS_AtomIncrement(gcvNULL, delay, gcvNULL), CL_INVALID_VALUE);
           if (++delayCount == 0)
           {
              clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                           &clgDefaultPlatform->compilerMutex),
                         CL_OUT_OF_HOST_MEMORY);
           }
           clmONERROR(gcoOS_AtomDestroy(gcvNULL, delay), CL_INVALID_VALUE);
        }

        gcoCL_SetHardware();
#if BUILD_OPENCL_FP
        gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);
        chipEnableFP = ((chipModel == gcv2500 && chipRevision == 0x5422) ||
                        (chipModel == gcv3000 && chipRevision == 0x5435) ||
                        (chipModel == gcv7000));
        if((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC) != gcvSTATUS_TRUE) ||
           (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE) != gcvSTATUS_TRUE)   ||
           (chipEnableFP == gcvFALSE))
        {
            /*if the features on the platform are not availble, still report embedded profile even if BUILD_OPENCL_FP is 1*/
            clgDefaultPlatform->profile = epProfile;
        }
#endif
        clmONERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &clgDefaultPlatform->hwCfg), CL_INVALID_VALUE);
        clmONERROR(gcoHAL_GetPatchID(gcvNULL, &clgDefaultPlatform->patchId), CL_INVALID_VALUE);
    }

    if (clgGlobalId == gcvNULL)
    {
        clmONERROR(gcoOS_AtomConstruct(gcvNULL, &clgGlobalId), CL_INVALID_VALUE);
    }

    if (Platform)
    {
        *Platform = clgDefaultPlatform;
    }

    gcmFOOTER_ARG("*Platforms=0x%x", gcmOPT_POINTER(Platform));
    return;

OnError:
    gcmFOOTER();
    return;
}

/*****************************************************************************\
|*                          OpenCL Platform API                              *|
\*****************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(
    cl_uint          NumEntries,
    cl_platform_id * Platforms,
    cl_uint *        NumPlatforms
    )
{
    gctINT           status;

    gcmHEADER_ARG("NumEntries=%u", NumEntries);
    gcmDUMP_API("${OCL clGetPlatformIDs %d}", NumEntries);

    if (Platforms && NumEntries == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-000000: argument Platforms is not NULL but argument NumEntries is 0 in clGetPlatformIDs.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clfSetTraceMode();

    clfGetDefaultPlatformID(Platforms);

    if (NumPlatforms)
    {
        *NumPlatforms = (clgDefaultPlatform ? 1 : 0);
    }

    VCL_TRACE_API(GetPlatformIDs)(NumEntries, Platforms, NumPlatforms);
    gcmFOOTER_ARG("%d *Platforms=0x%x *NumPlatforms=%u",
                  CL_SUCCESS, gcmOPT_POINTER(Platforms),
                  gcmOPT_VALUE(NumPlatforms));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(
    cl_platform_id   Platform,
    cl_platform_info ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    gctSTRING        retParam = NULL;
    gctSIZE_T        retParamSize = 0;
    gctINT           status;

    gcmHEADER_ARG("Platform=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Platform, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetPlatformInfo 0x%x, 0x%x}", Platform, ParamName);

    if (Platform == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-000001: argument Platform in clGetPlatformInfo is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_PLATFORM);
    }

    if (Platform != clgDefaultPlatform)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-000002: argument Platform in clGetPlatformInfo is not valid.\n");
        clmRETURN_ERROR(CL_INVALID_PLATFORM);
    }

    switch (ParamName)
    {
    case CL_PLATFORM_PROFILE:
        retParam = Platform->profile;
        break;

    case CL_PLATFORM_VERSION:
        retParam = Platform->version;
        break;

    case CL_PLATFORM_NAME:
        retParam = Platform->name;
        break;

    case CL_PLATFORM_VENDOR:
        retParam = Platform->vendor;
        break;

    case CL_PLATFORM_EXTENSIONS:
        retParam = Platform->extensions;
        break;

    case CL_PLATFORM_ICD_SUFFIX_KHR:
        retParam = Platform->suffix;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-000003: argument ParamName (0x%x) in clGetPlatformInfo is not valid.\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    retParamSize = gcoOS_StrLen(retParam, gcvNULL) + 1;
    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-000004: argument ParamValueSize (%d) in clGetPlatformInfo is less than ParamValueSizeRet (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }

        gcmVERIFY_OK(gcoOS_StrCopySafe(ParamValue, retParamSize, retParam));
    }

    VCL_TRACE_API(GetPlatformInfo)(Platform, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
