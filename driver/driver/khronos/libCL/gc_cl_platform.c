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

#define __NEXT_MSG_ID__     000005
#define _GC_OBJ_ZONE        gcdZONE_CL_PLATFORM

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
    gcvNULL                     /* Version name. */,
    gcvNULL                     /* C Version name. */,
    "FULL_PROFILE",             /* Profile          */
    "cl_khr_icd",               /* Extensions. */
    "viv",                      /* ICD Extensioin suffix. */

    gcvPATCH_INVALID,           /* patchId */
    gcvNULL,                    /* Compiler mutex. */
    gcvNULL,                    /* Compiler dll. */
    gcvNULL,                    /* Compiler pointer. */
    gcvNULL,                    /* loadCompiler pointer. */
    gcvNULL,                    /* unloadCompiler pointer. */
    gcvFALSE,                   /* virShaderPath */
};

cl_platform_id clgDefaultPlatform = gcvNULL;

gcsATOM_PTR clgGlobalId = gcvNULL;

gceTRACEMODE vclTraceMode = gcvTRACEMODE_NONE;

#define __clApiNameStr(func)  #func
char *__clTracerFuncNames[] = {
    __CL_API_ENTRY(__clApiNameStr)
};

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
    case gcvTRACEMODE_LOGGER:
        {
            gctHANDLE trlib = gcvNULL;
            cl_int tableSize = 0;
            gctPOINTER funcPtr = gcvNULL;
            gceSTATUS status;
            char trApiName[80];
            int i = 0;

#if defined(_WIN32) || defined(_WIN32_WCE)
            if (gcvSTATUS_OK != gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.dll", &trlib))
#else
            if (gcvSTATUS_OK != gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.so", &trlib))
#endif
            {
                return CL_FALSE;
            }

            if (trlib  == gcvNULL)
            {
#if defined(_WIN32) || defined(_WIN32_WCE)
                gcoOS_Print("Failed to open libGLES_vlogger.dll!\n");
#else
                gcoOS_Print("Failed to open libGLES_vlogger.so!\n");
#endif

                /* Clear vclTracerDispatchTable[] */
                memset(&vclTracerDispatchTable, 0, sizeof(clsTracerDispatchTableStruct));

                return CL_FALSE;
            }
            tableSize = (cl_int)(sizeof(vclTracerDispatchTable) / sizeof(void*));

            for (i = 0; i < tableSize; ++i)
            {
                trApiName[0] = '\0';
                gcoOS_StrCatSafe(trApiName, 80, "TR_cl");
                gcoOS_StrCatSafe(trApiName, 80, __clTracerFuncNames[i]);
                status =  gcoOS_GetProcAddress(gcvNULL, trlib, trApiName, &funcPtr);

                if (status == gcvSTATUS_OK)
                {
                    ((void *(*))(&vclTracerDispatchTable))[i] = funcPtr;
                }
                else
                {
                    gcoOS_Print("Failed to initialize vclTracerDispatchTable: gl%s!\n", __clTracerFuncNames[i]);

                    /* Clear __glesTracerDispatchTable[] */
                    /*memset(&vclTracerDispatchTable, 0, sizeof(clsTracerDispatchTableStruct));*/

                    /*gcoOS_FreeLibrary(gcvNULL, trlib);*/
                    /*return CL_FALSE;*/
                }
            }
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
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "2")))
            {
                vclTraceMode = gcvTRACEMODE_LOGGER;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "3")))
            {
                vclTraceMode = gcvTRACEMODE_ALLZONE;
            }
            else
            {
                gcmPRINT("OCL: unsupported trace mode");
            }

            clfInitTracerDispatchTable();
        }

        if (vclTraceMode == gcvTRACEMODE_ALLZONE)
        {
            gcoOS_SetDebugLevel(gcvLEVEL_VERBOSE);
            gcoOS_SetDebugZone(gcdZONE_ALL);
        }

        Once = gcvTRUE;
    }
}

void clfAllocateVidMemoryCB(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctSIZE_T size,
    gctUINT32 align,
    gctPOINTER *opaqueNode,
    gctPOINTER *memory,
    gctUINT32 *physical,
    gctPOINTER initialData,
    gctBOOL zeroMemory
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT                 allocatedSize;
    gctUINT32               phy;
    gctPOINTER              logical;
    gcsSURF_NODE_PTR        node;

    allocatedSize = gcmALIGN(size, align);

    gcmONERROR(gcoCL_AllocateMemory(&allocatedSize,
                                     &phy,
                                     &logical,
                                     &node,
                                     type,
                                     0));

    gcmDUMP(gcvNULL, "#[info: video memory allocate for VSC %s]", tag);

    if (initialData)
    {
        gcoOS_MemCopy(logical, initialData, size);
    }
    else if (zeroMemory)
    {
        gcoOS_ZeroMemory(logical, size);
    }

    *physical = phy;
    *opaqueNode = (gctPOINTER)node;

    gcmDUMP_BUFFER(gcvNULL, gcvDUMP_BUFFER_MEMORY, *physical, logical, 0, size);

    if (memory)
    {
        *memory = logical;
    }
OnError:
    return;
}

void clfFreeVidMemoryCB(
    gctPOINTER context,
    gceSURF_TYPE type,
    gctSTRING tag,
    gctPOINTER opaqueNode
    )
{
    gcoCL_FreeMemory(0, gcvNULL, 0, (gcsSURF_NODE_PTR)(opaqueNode),type);
    return;
}

void
clfGetDefaultPlatformID(
    clsPlatformId_PTR * Platform
    )
{
    gctINT           status;
    static gcsATOM_PTR delay = gcvNULL;
    gcmDECLARE_SWITCHVARS ;

    gcmHEADER_ARG("Platform=%p", Platform);

    if (clgDefaultPlatform == gcvNULL)
    {
        static gctINT32  delayCount = -1;
        gctSTRING epProfile = "EMBEDDED_PROFILE";
        gceCHIPMODEL  chipModel = gcv200;
        gctUINT32 chipRevision = 0;
        gctBOOL chipEnableEP = gcvFALSE;
        gctBOOL version11 = gcvFALSE;

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


        gcoHAL_SetHardwareType(gcvNULL,gcvHARDWARE_3D); /* cl used the 3d type!*/
        gcmSWITCH_TO_DEFAULT();
        gcoCL_InitializeHardware(); /*Init cl hardware for default hw */
        gcoHAL_QueryChipIdentity(gcvNULL,&chipModel,&chipRevision,gcvNULL,gcvNULL);
        chipEnableEP = ((chipModel == gcv1500 && chipRevision == 0x5246) ||
                        (chipModel == gcv2000 && chipRevision == 0x5108) ||
                        (chipModel == gcv3000 && chipRevision == 0x5513) ||
                        (chipModel == gcv3000 && chipRevision == 0x5451) ||
                        (chipModel == gcv5000));
        if((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_ATOMIC) != gcvSTATUS_TRUE) ||
           (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SHADER_HAS_RTNE) != gcvSTATUS_TRUE) ||
           chipEnableEP)
        {
            /*if the features on the platform are not availble, still report embedded profile even if BUILD_OPENCL_FP is 1*/
            clgDefaultPlatform->profile = epProfile;
        }

         version11 = ((chipModel == gcv1500 && chipRevision == 0x5246) ||
                     (chipModel == gcv2000 && chipRevision == 0x5108) ||
                     (chipModel == gcv3000 && chipRevision == 0x5451) ||
                     (chipModel == gcv3000 && chipRevision == 0x5513));
        if(version11)
        {
            clgDefaultPlatform->version = clfVERSION11;
            clgDefaultPlatform->Cversion = clcVERSION11;
        }
        else
        {
            clgDefaultPlatform->version = clfVERSION12;
            clgDefaultPlatform->Cversion = clcVERSION12;
        }

        clmONERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &clgDefaultPlatform->vscCoreSysCtx.hwCfg), CL_INVALID_VALUE);
        clmONERROR(vscCreatePrivateData(&clgDefaultPlatform->vscCoreSysCtx, &clgDefaultPlatform->vscCoreSysCtx.hPrivData, gcvTRUE), CL_INVALID_VALUE);
        clgDefaultPlatform->vscSysCtx.pCoreSysCtx = &clgDefaultPlatform->vscCoreSysCtx;
        clgDefaultPlatform->vscSysCtx.drvCBs.pfnAllocVidMemCb = (PFN_ALLOC_VIDMEM_CB)clfAllocateVidMemoryCB;
        clgDefaultPlatform->vscSysCtx.drvCBs.pfnFreeVidMemCb = (PFN_FREE_VIDMEM_CB)clfFreeVidMemoryCB;
        clgDefaultPlatform->vscSysCtx.hDrv = gcvNULL;

        clmONERROR(gcoHAL_GetPatchID(gcvNULL, &clgDefaultPlatform->patchId), CL_INVALID_VALUE);

        /* initialize back-end caps in case front-end is skipped for shader-binary only  */
        *gcGetHWCaps() = clgDefaultPlatform->vscCoreSysCtx.hwCfg;
        *gcGetPatchId() = clgDefaultPlatform->patchId;
        clgDefaultPlatform->virShaderPath =
            gcUseFullNewLinker(gcGetHWCaps()->hwFeatureFlags.hasHalti2)
         && gcoHAL_GetOption(gcvNULL, gcvOPTION_OCL_VIR_SHADER);
        vscSetDriverVIRPath(clgDefaultPlatform->virShaderPath);

        gcmRESTORE_HW();
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
    if(delay) gcoOS_AtomDestroy(gcvNULL, delay);


    if(clgGlobalId) gcoOS_AtomDestroy(gcvNULL, clgGlobalId);

    if(clgDefaultPlatform->compilerMutex) gcoOS_DeleteMutex(gcvNULL, clgDefaultPlatform->compilerMutex);

    gcmRESTORE_HW();
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
