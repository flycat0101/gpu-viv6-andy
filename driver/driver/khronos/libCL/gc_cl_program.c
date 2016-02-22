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


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     006030

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

/*****************************************************************************\
|*                       OpenCL Program Object API                           *|
\*****************************************************************************/
CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithSource(
    cl_context      Context,
    cl_uint         Count,
    const char **   Strings,
    const size_t *  Lengths,
    cl_int *        ErrcodeRet
    )
{
    clsProgram_PTR  program = gcvNULL;
    gctUINT         size = 0;
    gctUINT         length;
    gctUINT *       sizes = gcvNULL;
    gctUINT         i;
    gctSTRING       source;
    gctPOINTER      pointer = gcvNULL;
    gctINT          status;

    gcmHEADER_ARG("Context=0x%x Count=%u Strings=0x%x Lengths=0x%x",
                  Context, Count, Strings, Lengths);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006000: (clCreateProgramWithSource) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Count == 0 || Strings == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006001: (clCreateProgramWithSource) Count is 0 or Strngs is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Allocate an array for lengths of strings. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctUINT) * Count, &pointer), CL_OUT_OF_HOST_MEMORY);
    sizes = (gctUINT *) pointer;

    for (i = 0; i < Count; i++)
    {
        if (Strings[i] == gcvNULL)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006002: (clCreateProgramWithSource) Strings[%d] is NULL.\n",
                i);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }

        if (Lengths == gcvNULL || Lengths[i] == 0)
        {
            length = (gctUINT) gcoOS_StrLen(Strings[i], gcvNULL);
            sizes[i] = length;
            size += length;
        }
        else
        {
            sizes[i] = Lengths[i];
            size += Lengths[i];
        }
    }

    /* Allocate source. */
    clmONERROR(gcoOS_Allocate(gcvNULL, size + 1, &pointer), CL_OUT_OF_HOST_MEMORY);
    source = (gctSTRING) pointer;

    /* Allocate program. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsProgram), &pointer), CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, sizeof(clsProgram));

    program                  = (clsProgram_PTR) pointer;
    program->dispatch        = Context->dispatch;
    program->objectType      = clvOBJECT_PROGRAM;
    program->context         = Context;
    program->kernels         = gcvNULL;
    program->source          = source;
    program->binarySize      = 0;
    program->binary          = gcvNULL;
    program->buildOptions    = gcvNULL;
    program->buildLog        = gcvNULL;
    program->buildStatus     = CL_BUILD_NONE;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &program->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, program->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&program->id), CL_INVALID_VALUE);

    /* Allocate device pointers. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsDeviceId_PTR) * Context->numDevices,
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    program->numDevices      = Context->numDevices;
    program->devices         = (clsDeviceId_PTR *) pointer;

    for (i = 0; i < Context->numDevices; i++)
    {
        program->devices[i] = Context->devices[i];
    }

    /* Copy source. */
    for (i = 0; i < Count; i++)
    {
        if (sizes[i] > 0) {
            gcoOS_MemCopy(source, Strings[i], sizes[i]);
            source += sizes[i];
        }
    }
    source[0] = '\0';

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    gcoOS_Free(gcvNULL, sizes);

    gcmFOOTER_ARG("%d program=0x%x", CL_SUCCESS, program);
    return program;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006003: (clCreateProgramWithSource) cannot create program.  Maybe run out of memory.\n");
    }

    if (sizes) gcoOS_Free(gcvNULL, sizes);
    if(program != gcvNULL && program->devices != gcvNULL) gcoOS_Free(gcvNULL, program->devices);
    if(program != gcvNULL) gcoOS_Free(gcvNULL, program);

    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(
    cl_context                     Context,
    cl_uint                        NumDevices,
    const cl_device_id *           DeviceList,
    const size_t *                 Lengths,
    const unsigned char **         Binaries,
    cl_int *                       BinaryStatus,
    cl_int *                       ErrcodeRet
    )
{
    clsProgram_PTR  program = gcvNULL;
    gctPOINTER      pointer = gcvNULL;
    gctINT          status;
    gctUINT         i;
    gcSHADER        binary;

    gcmHEADER_ARG("Context=0x%x NumDevices=%u Binaries=0x%x Lengths=0x%x",
                  Context, NumDevices, Binaries, Lengths);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006004: (clCreateProgramWithBinary) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (NumDevices == 0 || DeviceList == gcvNULL || Lengths == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006005: (clCreateProgramWithBinary) NumDevices is 0, or DeviceList is gcvNULL, or Lengths is gcvNULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    for (i = 0; i < NumDevices; i++)
    {
        if (Lengths[i] == 0 || Binaries[i] == gcvNULL)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006006: (clCreateProgramWithBinary) Lengths[%d] is 0, or Binaries[%d] is NULL.\n",
                i, i);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    /* Allocate program. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsProgram), &pointer), CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, sizeof(clsProgram));

    /* TODO - support multiple devices */
    clmASSERT(NumDevices == 1, CL_INVALID_VALUE);

    program                  = (clsProgram_PTR) pointer;
    program->dispatch        = Context->dispatch;
    program->objectType      = clvOBJECT_PROGRAM;
    program->context         = Context;
    program->kernels         = gcvNULL;
    program->source          = gcvNULL;
    program->binarySize      = Lengths[0];
    program->buildOptions    = gcvNULL;
    program->buildLog        = gcvNULL;
    program->buildStatus     = CL_BUILD_NONE;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &program->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, program->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&program->id), CL_INVALID_VALUE);

    /* Allocate device pointers. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsDeviceId_PTR) * NumDevices,
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    program->numDevices      = NumDevices;
    program->devices         = (clsDeviceId_PTR *) pointer;

    for (i = 0; i < NumDevices; i++)
    {
        program->devices[i] = DeviceList[i];
    }

    /* Construct binary. */
    clmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_CL, &binary),
               CL_OUT_OF_HOST_MEMORY);

    /* Load binary */
    status = gcSHADER_LoadEx(binary, (gctPOINTER)Binaries[0], Lengths[0]);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006007: (clCreateProgramWithBinary) invalid binary.\n");
        clmRETURN_ERROR(CL_INVALID_BINARY);
    }

    program->binary = (gctUINT8_PTR) binary;

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    if (BinaryStatus)
    {
        BinaryStatus[0] = CL_SUCCESS;
    }

    gcmFOOTER_ARG("%d program=0x%x", CL_SUCCESS, program);
    return program;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006008: (clCreateProgramWithSource) cannot create program.  Maybe run out of memory.\n");
    }

    if (program != gcvNULL && program->devices != gcvNULL)
    {
        gcoOS_Free(gcvNULL, program->devices);
    }
    if (program != gcvNULL)
    {
        gcoOS_Free(gcvNULL, program);
    }

    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    if (BinaryStatus)
    {
        BinaryStatus[0] = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

#if BUILD_OPENCL_12
CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBuiltInKernels(
    cl_context             Context ,
    cl_uint                NumDevices ,
    const cl_device_id *   DeviceList ,
    const char *           KernelNames ,
    cl_int *               ErrcodeRet
    )
{
    return gcvNULL;
}
#endif

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(
    cl_program Program
    )
{
    gctINT              status;

    gcmHEADER_ARG("Program=0x%x", Program);

    if (Program == gcvNULL ||
        Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006009: (clRetainProgram) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Program->referenceCount, gcvNULL));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(
    cl_program Program
    )
{
    gctINT              status;
    gctINT32            oldReference;

    gcmHEADER_ARG("Program=0x%x", Program);

    if (Program == gcvNULL ||
        Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006010: (clReleaseProgram) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Program->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        Program->objectType = clvOBJECT_UNKNOWN;

        if (Program->buildOptions) gcoOS_Free(gcvNULL, Program->buildOptions);
        if (Program->linkOptions) gcoOS_Free(gcvNULL, Program->linkOptions);
        if (Program->compileOptions) gcoOS_Free(gcvNULL, Program->compileOptions);
        if (Program->buildLog) gcoOS_Free(gcvNULL, Program->buildLog);
        if (Program->source) gcoOS_Free(gcvNULL, Program->source);
        if (Program->devices) gcoOS_Free(gcvNULL, Program->devices);
        if (Program->binary) gcSHADER_Destroy((gcSHADER)Program->binary);

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Program->referenceCount));
        Program->referenceCount = gcvNULL;

        gcoOS_Free(gcvNULL, Program);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}



gceSTATUS
clfLoadCompiler(
    cl_platform_id platform
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();
    {
        gcmERR_RETURN(gcoOS_AcquireMutex(gcvNULL,
                                    platform->compilerMutex,
                                     gcvINFINITE));

        if (platform->compiler == gcvNULL)
        {
            status = gcoOS_LoadLibrary(gcvNULL,
#if defined(__APPLE__)
                                       "libCLC.dylib",
#else
                                       "libCLC",
#endif
                                       &platform->dll);

            if (gcmIS_ERROR(status))
            {
                goto OnError;
            }

            gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          platform->dll,
                                          "gcCLCompileProgram",
                                          (gctPOINTER*)&platform->compiler));

             gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          platform->dll,
                                          "gcCompileKernel",
                                          (gctPOINTER*)&platform->compiler11));

            gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          platform->dll,
                                          "gcLoadKernelCompiler",
                                          (gctPOINTER*)&platform->loadCompiler));

            gcmONERROR(gcoOS_GetProcAddress(gcvNULL,
                                          platform->dll,
                                          "gcUnloadKernelCompiler",
                                          (gctPOINTER*)&platform->unloadCompiler));


            gcmVERIFY_OK((*platform->loadCompiler)());
        }
    }

OnError:
    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, platform->compilerMutex));

    gcmFOOTER_NO();
    return status;
}


CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(
    cl_program           Program,
    cl_uint              NumDevices,
    const cl_device_id * DeviceList,
    const char *         Options,
    void                 (CL_CALLBACK *  PfnNotify)(cl_program, void *),
    void *               UserData
    )
{
    clsPlatformId_PTR   platform = gcvNULL;
    gctPOINTER          pointer;
    gctSIZE_T           length;
    gcSHADER            binary;
    gctUINT             binarySize;
    gctINT              status;

    gcmHEADER_ARG("Program=0x%x NumDevices=%u DeviceList=0x%x Options=%s",
                  Program, NumDevices, DeviceList, Options);

    gcoCL_InitializeHardware();

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006011: (clBuildProgram) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    if ((NumDevices == 0 && DeviceList) ||
        (NumDevices != 0 && DeviceList == gcvNULL) ||
        (PfnNotify == gcvNULL && UserData))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006012: (clBuildProgram) invalid device specification or PfnNotify/UserData specification.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* TODO - Check devices, compiler, etc. */

    if ((Program->binary != gcvNULL) && (Program->source != gcvNULL))
    {
        /* This program was built before
         * Clean up before building again
         */
        gcSHADER_Destroy((gcSHADER)Program->binary);

        if (Program->buildOptions) gcoOS_Free(gcvNULL, Program->buildOptions);
        if (Program->buildLog) gcoOS_Free(gcvNULL, Program->buildLog);

        Program->binary          = gcvNULL;
        Program->buildOptions    = gcvNULL;
        Program->buildLog        = gcvNULL;
        Program->buildStatus     = CL_BUILD_NONE;
    }

    /* Copy build options */
    if (Options)
    {
        length = gcoOS_StrLen(Options, gcvNULL) + 1;
        status = gcoOS_Allocate(gcvNULL, length, &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006013: (clBuildProgram) Run out of memory.\n");
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        gcmVERIFY_OK(gcoOS_StrCopySafe(pointer, length, Options));
        Program->buildOptions = (gctSTRING) pointer;
    }
    else
    {
        Program->buildOptions = gcvNULL;
    }

    Program->buildStatus = CL_BUILD_IN_PROGRESS;

    platform = Program->context->platform;

    clmONERROR(clfLoadCompiler(platform), CL_BUILD_PROGRAM_FAILURE);

    if (Program->binary == gcvNULL)
    {
        status = (*platform->compiler11)(gcvNULL,
                                       0,
                                       Program->source,
                                       Program->buildOptions,
                                       &binary,
                                       &Program->buildLog);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006017: (clBuildProgram) compilaton error:\n%s\n",
                Program->buildLog);
            clmRETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
        }


        Program->binary = (unsigned char *) binary;
        clmONERROR(gcSHADER_SaveEx(binary,  gcvNULL,  &binarySize), CL_INVALID_VALUE);
        Program->binarySize = binarySize;
#if BUILD_OPENCL_12
        Program->binaryType = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
#endif
    }

    status = CL_SUCCESS;

OnError:

    if(Program != gcvNULL)
    {
        Program->buildStatus = (status == CL_SUCCESS) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
    }

    if (PfnNotify)
    {
        PfnNotify(Program, UserData);
    }

    gcmFOOTER_ARG("%d Program=0x%x", status, Program);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clUnloadCompiler(
    void
    )
{
    clsPlatformId_PTR   platform = gcvNULL;
    gctINT              status;
    gctBOOL             acquired = gcvFALSE;

    gcmHEADER();

    clfGetDefaultPlatformID(&platform);

    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                        platform->compilerMutex,
                                        gcvINFINITE));
        acquired = gcvTRUE;

        if (platform->unloadCompiler != gcvNULL)
        {
            clmONERROR((*platform->unloadCompiler)(), CL_INVALID_PLATFORM);

            gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, platform->dll));

            platform->dll = gcvNULL;
            platform->compiler = gcvNULL;
            platform->compiler11 = gcvNULL;
            platform->loadCompiler = gcvNULL;
            platform->unloadCompiler = gcvNULL;
        }

        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                        platform->compilerMutex));
        acquired = gcvFALSE;
    }

    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, platform->compilerMutex));
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12

CL_API_ENTRY cl_int CL_API_CALL
clCompileProgram(
    cl_program            Program ,
    cl_uint               NumDevices ,
    const cl_device_id *  DeviceList ,
    const char *          Options ,
    cl_uint               NumInputHeaders ,
    const cl_program *    InputHeaders ,
    const char **         HeaderIncludeNames ,
    void (CL_CALLBACK *   PfnNotify )(cl_program  , void * ),
    void *                UserData
    )
{
    clsPlatformId_PTR   platform = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    gctSIZE_T           length;
    gcSHADER            binary = gcvNULL;
    gctUINT             binarySize;
    gctINT              status = CL_SUCCESS;

    gcmHEADER_ARG("Program=0x%x NumDevices=%u DeviceList=0x%x Options=%s",
                  Program, NumDevices, DeviceList, Options);

    gcoCL_InitializeHardware();

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    if ((NumInputHeaders != 0 && ( !InputHeaders || !HeaderIncludeNames)) ||
        (NumInputHeaders == 0 && (InputHeaders || HeaderIncludeNames)))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((NumDevices == 0 && DeviceList) ||
        (NumDevices != 0 && DeviceList == gcvNULL) ||
        (PfnNotify == gcvNULL && UserData))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* TODO - Check devices, compiler, etc. */
    /* Do we need rebuild?*/
    if ((Program->binary != gcvNULL) && (Program->source != gcvNULL))
    {
        /* This program was built before
         * Clean up before building again
         */
        gcSHADER_Destroy((gcSHADER)Program->binary);

        if (Program->compileOptions) gcoOS_Free(gcvNULL, Program->compileOptions);
        if (Program->buildLog) gcoOS_Free(gcvNULL, Program->buildLog);

        Program->binary             = gcvNULL;
        Program->compileOptions     = gcvNULL;
        Program->buildLog           = gcvNULL;
        Program->buildStatus        = CL_BUILD_NONE;
    }

    /* Copy build options */
    if (Options)
    {
        length = gcoOS_StrLen(Options, gcvNULL) + 1;
        status = gcoOS_Allocate(gcvNULL, length, &pointer);
        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        gcmVERIFY_OK(gcoOS_StrCopySafe((gctSTRING)pointer, length, Options));
        Program->compileOptions = (gctSTRING) pointer;
    }
    else
    {
        Program->compileOptions = gcvNULL;
    }

    platform = Program->context->platform;
    gcmASSERT(platform);

    clmONERROR(clfLoadCompiler(platform), CL_COMPILE_PROGRAM_FAILURE);

    /* construct a combined source with header files */



    Program->buildStatus = CL_BUILD_IN_PROGRESS;

    if (Program->binary == gcvNULL)
    {
        char ** headerSources = gcvNULL;
        gctUINT i;

        if(NumInputHeaders)
        {
            clmONERROR(gcoOS_Allocate(gcvNULL,
                                      sizeof(char **) * NumInputHeaders,
                                      (gctPOINTER *)&headerSources),
                                      CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(headerSources, sizeof(char **) * NumInputHeaders);
            for(i = 0; i < NumInputHeaders; i++)
            {
                headerSources[i] = InputHeaders[i]->source;
            }
        }

        status = (*platform->compiler)(gcvNULL,
                                       Program->source ?  gcoOS_StrLen(Program->source, gcvNULL) : 0,
                                       Program->source,
                                       Program->compileOptions,
                                       NumInputHeaders,
                                       (const char **)headerSources,
                                       HeaderIncludeNames,
                                       &binary,
                                       &Program->buildLog);

        if(headerSources)
        {
            gcoOS_Free(gcvNULL, headerSources);
        }

        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_COMPILE_PROGRAM_FAILURE);
        }

        Program->binary = (unsigned char *) binary;
        clmONERROR(gcSHADER_SaveEx(binary,  gcvNULL,  &binarySize), CL_INVALID_VALUE);
        Program->binarySize = binarySize;
    }

    Program->buildStatus = CL_BUILD_SUCCESS;

    Program->binaryType = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;

    if (PfnNotify)
    {
        PfnNotify(Program, UserData);
    }

    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:
    Program->buildStatus = CL_BUILD_ERROR;

    if (PfnNotify)
    {
        PfnNotify(Program, UserData);
    }

    /* free some resources */
    if (Program->compileOptions)
    {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, Program->compileOptions));
        Program->compileOptions = gcvNULL;
    }

    if (Program->binary)
    {
        gcmVERIFY_OK(gcSHADER_Destroy((gcSHADER)Program->binary));
        Program->binary = gcvNULL;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_program CL_API_CALL
clLinkProgram(
    cl_context            Context ,
    cl_uint               NumDevices ,
    const cl_device_id *  DeviceList ,
    const char *          Options ,
    cl_uint               NumInputPrograms ,
    const cl_program *    InputPrograms ,
    void (CL_CALLBACK *   PfnNotify )(cl_program  , void *  ),
    void *                UserData ,
    cl_int *              ErrcodeRet
    )
{
    gctPOINTER          pointer = gcvNULL;
    gctSIZE_T           length;
    gcSHADER            binary = gcvNULL;
    gctUINT             binarySize;
    gctINT              status = CL_SUCCESS;
    cl_program          program = NULL;
    gctUINT              i = 0;

    gcmHEADER_ARG("Context=0x%x NumDevices=%u DeviceList=0x%x Options=%s",
                  Context, NumDevices, DeviceList, Options);

    gcoCL_InitializeHardware();

    if ((NumInputPrograms != 0 && !InputPrograms) ||
        (NumInputPrograms == 0 && InputPrograms))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((NumDevices == 0 && DeviceList) ||
        (NumDevices != 0 && DeviceList == gcvNULL) ||
        (PfnNotify == gcvNULL && UserData))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

   /* Allocate program. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsProgram), (gctPOINTER*)&program),
                               CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(program, sizeof(clsProgram));

    program->dispatch        = Context->dispatch;
    program->objectType      = clvOBJECT_PROGRAM;
    program->context         = Context;
    program->kernels         = gcvNULL;
    program->source          = gcvNULL;
    program->binarySize      = 0;
    program->binary          = gcvNULL;
    program->buildOptions    = gcvNULL;
    program->buildLog        = gcvNULL;
    program->buildStatus     = CL_BUILD_NONE;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &program->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, program->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&program->id), CL_INVALID_VALUE);

    /* Allocate device pointers. */
    clmONERROR(gcoOS_Allocate(gcvNULL,
                              sizeof(clsDeviceId_PTR) * Context->numDevices,
                              &pointer),
               CL_OUT_OF_HOST_MEMORY);

    program->numDevices      = Context->numDevices;
    program->devices         = (clsDeviceId_PTR *) pointer;

    for (i = 0; i < Context->numDevices; i++)
    {
        program->devices[i] = Context->devices[i];
    }

    /* TODO - Check devices, compiler, etc. */

    /* Copy link options */
    if (Options)
    {
        length = gcoOS_StrLen(Options, gcvNULL) + 1;
        status = gcoOS_Allocate(gcvNULL, length, &pointer);
        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        gcmVERIFY_OK(gcoOS_StrCopySafe((gctSTRING)pointer, length, Options));
        program->linkOptions = (gctSTRING) pointer;
    }
    else
    {
        program->linkOptions = gcvNULL;
    }

    program->buildStatus = CL_BUILD_IN_PROGRESS;

    if (program->binary == gcvNULL)
    {
        gcSHADER *shaderArray = gcvNULL;
        clmONERROR(gcoOS_Allocate(gcvNULL, NumInputPrograms * gcmSIZEOF(gcSHADER*), (gctPOINTER*)&shaderArray),
                                    CL_OUT_OF_HOST_MEMORY);

        for(i = 0; i < NumInputPrograms; i++)
        {
            shaderArray[i] = (gcSHADER)InputPrograms[i]->binary;
        }

        status = gcSHADER_MergeKernel(NumInputPrograms, shaderArray, &binary);

        gcmVERIFY_OK(gcoOS_Free(gcvNULL, shaderArray));

        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_LINK_PROGRAM_FAILURE);
        }

        program->binary = (unsigned char *) binary;
        clmONERROR(gcSHADER_SaveEx(binary,  gcvNULL,  &binarySize), CL_INVALID_VALUE);
        program->binarySize = binarySize;
    }

    program->buildStatus = CL_BUILD_SUCCESS;

    /*TODO: check the link option to idcate type CL_PROGRAM_BINARY_TYPE_LIBRARY*/
    program->binaryType = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;

    if (PfnNotify)
    {
        PfnNotify(program, UserData);
    }

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }


    gcmFOOTER_NO();

    return program;

OnError:

    if (PfnNotify)
    {
        PfnNotify(NULL, UserData);
    }

    /* free some resources */
    if (program)
    {
        if (program->linkOptions)
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, program->linkOptions));

        if (program->devices)
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, program->devices));

        if (program->binary)
            gcmVERIFY_OK(gcSHADER_Destroy((gcSHADER)program->binary));

        gcmVERIFY_OK(gcoOS_Free(gcvNULL, program));
    }

    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    gcmFOOTER_ARG("%d", status);
    return NULL;
}



CL_API_ENTRY cl_int CL_API_CALL
clUnloadPlatformCompiler(
    cl_platform_id  Platform
    )
{
    gctINT              status;
    gctBOOL             acquired = gcvFALSE;

    gcmHEADER();

    if (Platform == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006029: argument Platform in clUnloadPlatformCompiler is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_PLATFORM);
    }

    {
        gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL,
                                        Platform->compilerMutex,
                                        gcvINFINITE));
        acquired = gcvTRUE;

        if (Platform->unloadCompiler != gcvNULL)
        {
            clmONERROR((*Platform->unloadCompiler)(), CL_INVALID_PLATFORM);

            gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, Platform->dll));

            Platform->dll = gcvNULL;
            Platform->compiler = gcvNULL;
            Platform->compiler11 = gcvNULL;
            Platform->loadCompiler = gcvNULL;
            Platform->unloadCompiler = gcvNULL;
        }

        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL,
                                        Platform->compilerMutex));
        acquired = gcvFALSE;
    }

    gcmFOOTER_NO();
    return CL_SUCCESS;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Platform->compilerMutex));
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramInfo(
    cl_program          Program,
    cl_program_info     ParamName,
    size_t              ParamValueSize,
    void *              ParamValue,
    size_t *            ParamValueSizeRet
    )
{
    gctSIZE_T           retParamSize = 0;
    gctPOINTER          retParamPtr = NULL;
    size_t              retValue_size_t[2];
    gctINT              status;
    gctUINT             i;
    gctINT32            referenceCount;

    gcmHEADER_ARG("Program=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Program, ParamName, ParamValueSize, ParamValue);

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006020: (clGetProgramInfo) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    switch (ParamName)
    {
    case CL_PROGRAM_CONTEXT:
        retParamSize = gcmSIZEOF(Program->context);
        retParamPtr = &Program->context;
        break;

    case CL_PROGRAM_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, Program->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    case CL_PROGRAM_NUM_DEVICES:
        retParamSize = gcmSIZEOF(Program->numDevices);
        retParamPtr = &Program->numDevices;
        break;

    case CL_PROGRAM_DEVICES:
        retParamSize = Program->numDevices * gcmSIZEOF(*Program->devices);
        retParamPtr = Program->devices;
        break;

    case CL_PROGRAM_SOURCE:
        if (Program->source != gcvNULL)
        {
            retParamSize = gcoOS_StrLen(Program->source, gcvNULL) + 1;
            retParamPtr = Program->source;
        }
        else
        {
            retParamSize = 1;
            retParamPtr = clgEmptyStr;
        }
        break;

    case CL_PROGRAM_BINARY_SIZES:
        /*retParamSize = gcmSIZEOF(Program->binarySize);
        retParamPtr = &Program->binarySize;*/
        /* TODO: Need to enhance for multiple devices. */
        retValue_size_t[0] = Program->binarySize;
        retParamSize = gcmSIZEOF(retValue_size_t[0]) * 1;
        retParamPtr = retValue_size_t;
        break;

    case CL_PROGRAM_BINARIES:
        retParamSize = gcmSIZEOF(Program->binary);
        retParamPtr = Program->binary;
        break;
#if BUILD_OPENCL_12
    case CL_PROGRAM_NUM_KERNELS:
        retParamSize = gcmSIZEOF(GetShaderKernelFunctionCount((gcSHADER)(Program->binary)));
        retParamPtr = &GetShaderKernelFunctionCount((gcSHADER)(Program->binary));
        break;
    case CL_PROGRAM_KERNEL_NAMES:
        {
            gctPOINTER pointer;
            gctSTRING  name;
            size_t     strLen = 0;
            size_t     totalLen = 0;
            gctUINT    i;
            gcSHADER   programBinary= (gcSHADER) Program->binary;
            gctUINT32  kernelFunctionCount = GetShaderKernelFunctionCount(programBinary);
            if ( kernelFunctionCount == 0)
            {
                retParamSize = 1;
                if (ParamValue)
                {
                    clmONERROR(gcoOS_Allocate(gcvNULL, 1, &pointer), CL_OUT_OF_HOST_MEMORY);
                    name = (gctCHAR*) pointer;
                    *name = '\0';
                    retParamPtr = pointer;
                }
                break;
            }

            for ( i = 0; i < kernelFunctionCount; i++)
            {
                totalLen += gcoOS_StrLen(GetShaderKernelFunction(programBinary, i)->name, gcvNULL);
            }
            totalLen += kernelFunctionCount; /*Add kernelFunctionCount-1 ';' and 1 '\0' Z*/
            retParamSize = totalLen;

            if (ParamValue)
            {
                clmONERROR(gcoOS_Allocate(gcvNULL, totalLen, &pointer), CL_OUT_OF_HOST_MEMORY);
                name = (gctCHAR*) pointer;
                for ( i = 0; i < kernelFunctionCount - 1; i++)
                {
                    strLen = gcoOS_StrLen(GetShaderKernelFunction(programBinary, i)->name, gcvNULL);
                    gcoOS_MemCopy((gctPOINTER)name, (gctPOINTER)GetShaderKernelFunction(programBinary, i)->name, strLen);
                    name+= strLen;
                    *name++ = ';';
                }
                strLen = gcoOS_StrLen(GetShaderKernelFunction(programBinary, i)->name, gcvNULL);
                gcoOS_MemCopy((gctPOINTER)name, (gctPOINTER)GetShaderKernelFunction(programBinary, i)->name, strLen);
                name+= strLen;
                *name = '\0';
                retParamPtr = pointer;
            }
        }
        break;
#endif

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006021: (clGetProgramInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006022: (clGetProgramInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            if (ParamName == CL_PROGRAM_BINARIES)
            {
                for (i = 0; i < Program->numDevices; i++)
                {
                    gctUINT binarySize = Program->binarySize;
                    status = gcSHADER_SaveEx(retParamPtr, ((gctSTRING *)ParamValue)[i], &binarySize);
                    if (gcmIS_ERROR(status))
                    {
                        gcmUSER_DEBUG_ERROR_MSG(
                            "OCL-006023: (clGetProgramInfo) Cannot save program binary.\n");
                        clmRETURN_ERROR(CL_BUILD_PROGRAM_FAILURE);
                    }
                }
            }
#if BUILD_OPENCL_12
            else if (ParamName == CL_PROGRAM_KERNEL_NAMES)
            {
                gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
                gcoOS_Free(gcvNULL, retParamPtr);
            }
#endif
            else
            {
                gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
            }
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetProgramBuildInfo(
    cl_program            Program,
    cl_device_id          Device,
    cl_program_build_info ParamName,
    size_t                ParamValueSize,
    void *                ParamValue,
    size_t *              ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;

    gcmHEADER_ARG("Program=0x%x Device=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Program, Device, ParamName, ParamValueSize, ParamValue);

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006024: (clGetProgramBuildInfo) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    if (Device == gcvNULL || Device->objectType != clvOBJECT_DEVICE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006025: (clGetProgramBuildInfo) invalid Device.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }

    switch (ParamName)
    {
    case CL_PROGRAM_BUILD_STATUS:
        retParamSize = gcmSIZEOF(Program->buildStatus);
        retParamPtr = &Program->buildStatus;
        break;

    case CL_PROGRAM_BUILD_OPTIONS:
        if (Program->buildOptions != gcvNULL)
        {
            retParamSize = gcoOS_StrLen(Program->buildOptions, gcvNULL) + 1;
            retParamPtr = Program->buildOptions;
        }
        else
        {
            retParamSize = 1;
            retParamPtr = clgEmptyStr;
        }
        break;

    case CL_PROGRAM_BUILD_LOG:
        if (Program->buildLog != gcvNULL)
        {
            retParamSize = gcoOS_StrLen(Program->buildLog, gcvNULL) + 1;
            retParamPtr = Program->buildLog;
        }
        else
        {
            retParamSize = 1;
            retParamPtr = clgEmptyStr;
        }
        break;
#if BUILD_OPENCL_12
    case CL_PROGRAM_BINARY_TYPE:
        retParamSize = gcmSIZEOF(Program->binaryType);
        retParamPtr = &Program->binaryType;
        break;
#endif
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-006026: (clGetProgramBuildInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-006027: (clGetProgramBuildInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
