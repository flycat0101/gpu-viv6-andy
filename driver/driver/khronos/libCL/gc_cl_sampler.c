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

#define __NEXT_MSG_ID__     005007

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

/*****************************************************************************\
|*                           OpenCL Sampler API                              *|
\*****************************************************************************/
CL_API_ENTRY cl_sampler CL_API_CALL
clCreateSampler(
    cl_context          Context,
    cl_bool             NormalizedCoords,
    cl_addressing_mode  AddressingMode,
    cl_filter_mode      FilterMode,
    cl_int *            ErrcodeRet
    )
{
    clsSampler_PTR      sampler;
    gctPOINTER          pointer;
    gctINT              status;

    gcmHEADER_ARG("Context=0x%x NormalizedCoords=%d AddressingMode=%d FilterMode=%d",
                  Context, NormalizedCoords, AddressingMode, FilterMode);

    if (Context == gcvNULL || Context->objectType != clvOBJECT_CONTEXT)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005000: (clCreateSampler) invalid Context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    NormalizedCoords = (NormalizedCoords ? CL_TRUE : CL_FALSE);

    /* Allocate command queue. */
    status = gcoOS_Allocate(gcvNULL, sizeof(clsSampler), &pointer);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005001: (clCreateSampler) cannot create sampler.  Maybe run out of memory.\n");
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    sampler                     = (clsSampler_PTR) pointer;
    sampler->dispatch           = Context->dispatch;
    sampler->objectType         = clvOBJECT_SAMPLER;
    sampler->context            = Context;
    sampler->normalizedCoords   = NormalizedCoords;
    sampler->addressingMode     = AddressingMode;
    sampler->filterMode         = FilterMode;

    sampler->samplerValue       = (((AddressingMode   & 0xF)      ) |
                                   ((FilterMode       & 0xF) <<  8) |
                                   ((NormalizedCoords      ) << 16));

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &sampler->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, sampler->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&sampler->id), CL_INVALID_VALUE);

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }
    gcmFOOTER_ARG("%d sampler=%lu",
                  CL_SUCCESS, sampler);
    return sampler;

OnError:
    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainSampler(
    cl_sampler Sampler
    )
{
    gctINT              status;

    gcmHEADER_ARG("Sampler=0x%x", Sampler);

    if (Sampler == gcvNULL ||
        Sampler->objectType != clvOBJECT_SAMPLER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005002: (clRetainSampler) invalid Sampler.\n");
        clmRETURN_ERROR(CL_INVALID_SAMPLER);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Sampler->referenceCount, gcvNULL));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseSampler(
    cl_sampler Sampler
    )
{
    gctINT              status;
    gctINT32            oldReference;

    gcmHEADER_ARG("Sampler=0x%x", Sampler);

    if (Sampler == gcvNULL ||
        Sampler->objectType != clvOBJECT_SAMPLER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005003: (clReleaseSampler) invalid Sampler.\n");
        clmRETURN_ERROR(CL_INVALID_SAMPLER);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Sampler->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Sampler->referenceCount));
        Sampler->referenceCount = gcvNULL;

        gcoOS_Free(gcvNULL, Sampler);
    }

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetSamplerInfo(
    cl_sampler         Sampler,
    cl_sampler_info    ParamName,
    size_t             ParamValueSize,
    void *             ParamValue,
    size_t *           ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;
    gctINT32         referenceCount;

    gcmHEADER_ARG("Sampler=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Sampler, ParamName, ParamValueSize, ParamValue);

    if (Sampler == gcvNULL || Sampler->objectType != clvOBJECT_SAMPLER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005004: (clGetSamplerInfo) invalid Sampler.\n");
        clmRETURN_ERROR(clvOBJECT_SAMPLER);
    }

    switch (ParamName)
    {
    case CL_SAMPLER_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, Sampler->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    case CL_SAMPLER_CONTEXT:
        retParamSize = gcmSIZEOF(Sampler->context);
        retParamPtr = &Sampler->context;
        break;

    case CL_SAMPLER_NORMALIZED_COORDS:
        retParamSize = gcmSIZEOF(Sampler->normalizedCoords);
        retParamPtr = &Sampler->normalizedCoords;
        break;

    case CL_SAMPLER_ADDRESSING_MODE:
        retParamSize = gcmSIZEOF(Sampler->addressingMode);
        retParamPtr = &Sampler->addressingMode;
        break;

    case CL_SAMPLER_FILTER_MODE:
        retParamSize = gcmSIZEOF(Sampler->filterMode);
        retParamPtr = &Sampler->filterMode;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-005005: (clGetSamplerInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-005006: (clGetSamplerInfo) ParamValueSize (%d) is less than required size (%d).\n",
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
