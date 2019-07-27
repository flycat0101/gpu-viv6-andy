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
#include "gc_hal_user_math.h"

#define __NEXT_MSG_ID__     007032
#define _GC_OBJ_ZONE        gcdZONE_CL_KERNEL

extern gctBOOL gcSHADER_GoVIRPass(gcSHADER Shader);

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
static gceSTATUS
clfAllocateVIRKernelInstance(clsKernelVIRInstance ** instance)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (instance)
    {
        gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(clsKernelVIRInstance), (gctPOINTER *) instance));
        gcoOS_ZeroMemory(*instance, gcmSIZEOF(clsKernelVIRInstance));
    }

OnError:
    return status;
}

static void
clfFreeVIRKernelInstance(clsKernelVIRInstance * instance)
{
    if (instance)
    {
        vscFinalizeKEP(&instance->kep);
        vscFinalizeHwPipelineShadersStates(&instance->platform->vscSysCtx, &instance->hwStates);
        gcoOS_Free(gcvNULL, instance);
    }
}

gctINT
clfRetainKernel(
    cl_kernel    Kernel
    )
{
    gctINT status = CL_SUCCESS;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007011: (clfRetainKernel) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, Kernel->referenceCount, gcvNULL));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static void
clfDeleteHashInstanceKey(
    clsVIRInstanceHashRec_PTR pHash,
    clsVIRInstanceKey_PTR pObj
    )
{
    gctUINT entryId = pObj->key & (pHash->tbEntryNum - 1);
    clsVIRInstanceKey_PTR pCurObj = pHash->ppHashTable[entryId];
    clsVIRInstanceKey_PTR pPreObj = gcvNULL;

    gcmHEADER_ARG("pHash=0x%x pObj=0x%x", pHash, pObj);

    while (pCurObj)
    {
        if (pCurObj == pObj)
        {
            break;
        }

        pPreObj = pCurObj;
        pCurObj = pCurObj->nextInstanceKey;
    }

    if (pPreObj == gcvNULL)
    {
        pHash->ppHashTable[entryId] = pCurObj->nextInstanceKey;
    }
    else
    {
        pPreObj->nextInstanceKey = pCurObj->nextInstanceKey;
    }

    --pHash->pEntryCounts[entryId];

    if(pObj)
    {
        if(pObj->virInstance)
        {
            clfFreeVIRKernelInstance(pObj->virInstance);
            pObj->virInstance = gcvNULL;
        }
        gcmOS_SAFE_FREE(gcvNULL, pObj);
    }

    gcmFOOTER_NO();
}

static void
clfDeleteHashAllInstanceKey(
    clsVIRInstanceHashRec_PTR pHash
    )
{
    gctUINT entryId;
    gcmHEADER_ARG("pHash=0x%x", pHash);

    for (entryId = 0; entryId < pHash->tbEntryNum; entryId ++)
    {
        while (pHash->ppHashTable[entryId])
        {
            clfDeleteHashInstanceKey(pHash, pHash->ppHashTable[entryId]);
        }
    }

    if(pHash->ppHashTable)
    {
        gcmOS_SAFE_FREE(gcvNULL, pHash->ppHashTable);

    }

    if(pHash->pEntryCounts)
    {
        gcmOS_SAFE_FREE(gcvNULL, pHash->pEntryCounts);
    }

    gcmOS_SAFE_FREE(gcvNULL, pHash);

    gcmFOOTER_NO();
}

gctINT
clfReleaseKernel(
    cl_kernel   Kernel
    )
{
    gctINT status = CL_SUCCESS;
    gctINT32  oldReference;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007011: (clfReleaseKernel) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, Kernel->referenceCount, &oldReference));

    if (oldReference == 1)
    {
        if (Kernel->context->platform->virShaderPath)
        {
            gcmVERIFY_OK(clfFreeVIRKernelArgs(Kernel->kernelNumArgs, Kernel->srcArgs, 0, gcvTRUE));
        }
        else
        {
            /* Release arguments */
            gcmVERIFY_OK(clfFreeKernelArgs(Kernel->numArgs, Kernel->args, gcvTRUE));
        }

        if(Kernel->virCacheTable)
        {
            clfDeleteHashAllInstanceKey(Kernel->virCacheTable);
        }

        /* Release mutex. */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, Kernel->argMutex));
        Kernel->argMutex = gcvNULL;
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, Kernel->cacheMutex));
        Kernel->cacheMutex = gcvNULL;

        Kernel->objectType = clvOBJECT_UNKNOWN;

        clfReleaseProgram(Kernel->program);

        while (Kernel->recompileInstance)
        {
            clsKernelInstance_PTR instance = Kernel->recompileInstance;
            gcmASSERT(instance->programState.hints != Kernel->masterInstance.programState.hints);
            /* Release states */
            Kernel->recompileInstance = instance->next;
            gcFreeProgramState(instance->programState);
            if (instance->binary) gcSHADER_Destroy((gcSHADER)instance->binary);
            if (instance->patchDirective) clfDestroyPatchDirective(&instance->patchDirective);
            gcmOS_SAFE_FREE(gcvNULL, instance);
        }
        gcFreeProgramState(Kernel->masterInstance.programState);
        if (Kernel->masterInstance.binary) gcSHADER_Destroy((gcSHADER)Kernel->masterInstance.binary);
        if (Kernel->name)
        {
            gcmOS_SAFE_FREE(gcvNULL, Kernel->name);
        }

        if (Kernel->virMasterInstance)
        {
            clfFreeVIRKernelInstance(Kernel->virMasterInstance);
            Kernel->virMasterInstance = gcvNULL;
        }

        if (Kernel->shaderHandle)
        {
            vscDestroyShader(Kernel->shaderHandle);
            Kernel->shaderHandle = gcvNULL;
        }

        clfReleaseContext(Kernel->context);
        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Kernel->referenceCount));
        Kernel->referenceCount = gcvNULL;

        gcmOS_SAFE_FREE(gcvNULL, Kernel);
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
clfSetImageValue(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN const gctINT * Value
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows;

    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Value);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Value != gcvNULL);

    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);
    rows *= gcmMIN((gctINT) Count, Uniform->arraySize);

    /* Program the uniform. */
    status = gcoSHADER_BindUniform(gcvNULL,
                                   Uniform->address,
                                   GetUniformPhysical(Uniform),
                                   columns, rows,
                                   ((Uniform->arraySize)*2),gcvFALSE,
                                   columns * 4,
                                   16,
                                   (gctPOINTER) Value,
                                   gcvUNIFORMCVT_NONE,
                                   Uniform->shaderKind);

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

gceSTATUS
clfSetUniformValue(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN const gctINT * Value
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows;

    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Value);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Value != gcvNULL);

    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);
    rows *= gcmMIN((gctINT) Count, Uniform->arraySize);

    /* Program the uniform. */
    status = gcoSHADER_BindUniform(gcvNULL,
                                   Uniform->address,
                                   GetUniformPhysical(Uniform),
                                   columns, rows,
                                   1,gcvFALSE,
                                   columns * 4,
                                   0,
                                   (gctPOINTER) Value,
                                   gcvUNIFORMCVT_NONE,
                                   Uniform->shaderKind);

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

gceSTATUS
clfSetUniformValueCombinedMode(
    IN gcUNIFORM Uniform,
    IN gctUINT32 Count,
    IN  gctINT * Values[],
    IN gctUINT ValuesCount
    )
{
#if gcdNULL_DRIVER < 2
    gceSTATUS status;
    gctUINT32 columns, rows;
    gcmHEADER_ARG("Uniform=0x%x Count=%lu Value=0x%x", Uniform, Count, Values);

    /* Verify the arguments. */
    gcmVERIFY_OBJECT(Uniform, gcvOBJ_UNIFORM);
    gcmDEBUG_VERIFY_ARGUMENT(Count > 0);
    gcmDEBUG_VERIFY_ARGUMENT(Values != gcvNULL);
    gcTYPE_GetTypeInfo(Uniform->u.type, &columns, &rows, 0);

    if(isUniformTempRegSpillAddress(Uniform))  /*Special handle TemRegSpill Uniform, the type is UINTX_2, but only used the first component in shader*/
    {
        columns = 1;
    }

    rows *= gcmMIN((gctINT) Count, Uniform->arraySize);

    /* Program the uniform. */
    status = gcoSHADER_BindUniformCombinedMode(gcvNULL,
                                   Uniform->address,
                                   GetUniformPhysical(Uniform),
                                   columns, rows,
                                   1,gcvFALSE,
                                   columns * 4,
                                   0,
                                   (gctPOINTER) Values,
                                   ValuesCount,
                                   gcvUNIFORMCVT_NONE,
                                   Uniform->shaderKind
                                   );

    gcmFOOTER();
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

gctINT
clfSetupImageSampler(
    clsKernel_PTR       Kernel,
    clsArgument_PTR     Arg
    )
{
    clsMem_PTR          image;
    cleSAMPLER          samplerValue;
    gctUINT             filterMode;
    gctUINT             addressMode;
    gctINT              status;

    static const gceTEXTURE_ADDRESSING addressXlate[] =
    {
        gcvTEXTURE_INVALID,
        gcvTEXTURE_CLAMP,
        gcvTEXTURE_BORDER,
        gcvTEXTURE_WRAP,
        gcvTEXTURE_MIRROR
    };

    static const gceTEXTURE_FILTER filterXlate[] =
    {
        gcvTEXTURE_POINT,
        gcvTEXTURE_LINEAR
    };

    gcmHEADER_ARG("Kernel=0x%x Arg=0x%x",
                  Kernel, Arg);

    /* Get image object. */
    image = Arg->image;
    gcmASSERT(image->objectType == clvOBJECT_MEM);
    gcmASSERT(image->type != CL_MEM_OBJECT_BUFFER);

    /* Get sampler value. */
    samplerValue = Arg->samplerValue;

    /*gcmASSERT((samplerValue & CLK_NORMALIZED_COORDS_TRUE)
        == CLK_NORMALIZED_COORDS_TRUE);*/
    filterMode = (samplerValue & 0xF00) >> 8;
    addressMode = samplerValue & 0xF;

    /* Set up image sampler. */
    clmONERROR(gcoCL_SetupTexture(image->u.image.texture,
                                  image->u.image.surface,
                                  (gctUINT)GetUniformPhysical(Arg->uniform),
                                  addressXlate[addressMode],
                                  filterXlate[filterMode]),
               CL_OUT_OF_HOST_MEMORY);

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* Return how many uniform entries will be reported to app */
static gctUINT
clfGetUniformArrayInfo(
    gcUNIFORM uniform,
    gctUINT *maxNameLen, /* max possible name len of this entry, exclude bottom-level "[0]" */
    gctBOOL *isArray,
    gctUINT *arraySize
)
{
    gctINT j;
    gctUINT32 length;
    gctUINT entries = 1;

    gcUNIFORM_GetName(uniform, &length, gcvNULL);

    /* Multiple entries will be reported for array of arrays.
    ** If a uniform is an array, or array of array, its name length should be added
    ** with several dim of array indices, like "name[x]...[x][0]".
    */
    for (j = 0; j < uniform->arrayLengthCount - 1; ++j)
    {
        gctUINT decimalLen = 1;
        gctINT arrayLen = uniform->arrayLengthList[j];

        gcmASSERT(arrayLen > 0);
        entries *= arrayLen;

        arrayLen--;    /* Get max arrayIndex */
        while (arrayLen >= 10)
        {
            ++decimalLen;
            arrayLen /= 10;
        }

        length += (decimalLen + 2);
    }

    if (maxNameLen)
    {
        *maxNameLen = length;
    }

    if (isArray)
    {
        *isArray = uniform->arrayLengthCount > 0 ? gcvTRUE : gcvFALSE;
    }

    if (arraySize)
    {
        *arraySize = uniform->arrayLengthCount > 0
                   ? (uniform->arrayLengthList[uniform->arrayLengthCount - 1] > 0 ?
                      uniform->arrayLengthList[uniform->arrayLengthCount - 1] : 0)
                   : 1;
    }

    return entries;
}


static gctINT
clfLoadKernelArgValues(
    clsKernel_PTR         Kernel,
    clsKernelInstance_PTR Instance,
    clsArgument_PTR       Arg,
    gctUINT               WorkDim,
    size_t                GlobalWorkOffset[3],
    size_t                GlobalWorkSize[3],
    size_t                LocalWorkSize[3],
    clsArgument_PTR       baseArg,
    gctUINT               numArg,
    clsPrivateBuffer_PTR  *privateBufList
    )
{
    gcSHADER_TYPE        type;
    gctUINT              length;
    gcSL_FORMAT          format;
    gctBOOL              isPointer;
    gceUNIFORM_FLAGS     flags;
    gctINT               status;
    clsMemAllocInfo_PTR  privateBuf = gcvNULL;
    clsPrivateBuffer_PTR tempNode = gcvNULL;
    gcSHADER             Shader = (gcSHADER)Instance->binary;
    gcsHINT_PTR          hints = Instance->programState.hints;

    gcmHEADER_ARG("Kernel=0x%x Arg=0x%x WorkDim=%d",
                  Kernel, Arg, WorkDim);

    if (Arg->uniform == gcvNULL)
    {
        status = CL_SUCCESS;
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    clmONERROR(gcUNIFORM_GetType(Arg->uniform, &type, &length),
               CL_INVALID_VALUE);

    clmONERROR(gcUNIFORM_GetFormat(Arg->uniform, &format, &isPointer),
              CL_INVALID_VALUE);

    clmONERROR(gcUNIFORM_GetFlags(Arg->uniform, &flags),
              CL_INVALID_VALUE);

    if (isUniformCompiletimeInitialized(Arg->uniform))
    {
       ;
    }
    else if (isUniformKernelArg(Arg->uniform) ||
             isUniformKernelArgConstant(Arg->uniform))
    {
        if (isPointer)
        {
            clsMem_PTR memObj = *(clsMem_PTR *) Arg->data;
            gctINT * data;
            gctUINT32 tmpData[8] = {0};

            /* Handle NULL buffer object */
            if (!memObj)
            {
               if (length == 1)
                {
                    static int zero = 0;
                    data = &zero;
                    clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                               CL_INVALID_VALUE);
                    status = CL_SUCCESS;
                    gcmFOOTER_ARG("%d", status);
                    return status;
                }
                else
                {
                    gcmFOOTER_NO();
                    return CL_INVALID_ARG_VALUE;
                }
            }

            if (memObj->type == CL_MEM_OBJECT_BUFFER)
            {
                if (memObj->u.buffer.wrapped && ((memObj->flags & CL_MEM_USE_UNCACHED_HOST_MEMORY_VIV) == 0))
                {
                    gcsSURF_NODE_PTR surfNode = memObj->u.buffer.node;

                    gcoOS_CacheInvalidate(gcvNULL, surfNode->u.normal.node, memObj->u.buffer.logical, memObj->u.buffer.allocatedSize);
                }

                /* Is this buffer for read or write?  Get render fence, and wait from render->blt engine result */
                gcoCL_MemWaitAndGetFence(memObj->u.buffer.node,gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

                data = (gctINT *) &memObj->u.buffer.physical;

                clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                       CL_INVALID_VALUE);
            }
            else
            {
                data = (gctINT *) &memObj->u.image.headerPhysical;

                /* fence record at image header, is this for read or write? */
                gcoCL_MemWaitAndGetFence(memObj->u.image.headerNode, gcvENGINE_RENDER,gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

#if cldTUNING
                if(Kernel->context->devices[0]->deviceInfo.supportIMGInstr &&
                  (Kernel->patchNeeded == gcvFALSE) &&
                  gcSHADER_GoVIRPass(Shader))
                {
                    gctUINT addressMode = 0, format, tiling, type=0, componentCount=0,
                            swizzleR=0, swizzleG=0, swizzleB=0, swizzleA=0, i=0;
                    gctUINT sliceSize = 0, depth = 0;
                    clsArgument_PTR tmpArg = gcvNULL;
                    gcKERNEL_FUNCTION kernelFunction = gcvNULL;
                    gcsIMAGE_SAMPLER_PTR imageSampler = gcvNULL;
                    gctUINT shift = (unsigned int)(gcoMATH_Log((gctFLOAT)memObj->u.image.elementSize)/gcoMATH_Log(2.0));

                    tmpData[0] = (gctUINT32 ) memObj->u.image.texturePhysical;
                    tmpData[1] = memObj->u.image.textureStride;

                    switch(memObj->type)
                    {
                    case CL_MEM_OBJECT_IMAGE1D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_width<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width;
                        depth = 1;
                        type = 0;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width * memObj->u.image.imageDesc.image_height;
                        depth = 1;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE3D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width * memObj->u.image.imageDesc.image_height;
                        depth = memObj->u.image.imageDesc.image_depth;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_width<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width;
                        depth = memObj->u.image.imageDesc.image_array_size;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width * memObj->u.image.imageDesc.image_height;
                        depth = memObj->u.image.imageDesc.image_array_size;
                        type = 1;
                        break;
                    default:
                        break;
                    }
                    switch(memObj->u.image.imageFormat.image_channel_data_type)
                    {
                    case CL_UNORM_INT8:
                        format = 0xF;
                        break;
                    case CL_UNORM_INT16:
                        format = 0xE;
                        break;
                    case CL_SNORM_INT8:
                        format = 0xC;
                        break;
                    case CL_SNORM_INT16:
                        format = 0xB;
                        break;
                    case CL_UNORM_SHORT_565:
                        format = 9;
                        break;
                    case CL_UNORM_SHORT_555:
                        format = 8;
                        break;
                    case CL_UNORM_INT_101010:
                        format = 0xA;
                        break;
                    case CL_SIGNED_INT8:
                        format = 4;
                        break;
                    case CL_SIGNED_INT16:
                        format = 3;
                        break;
                    case CL_SIGNED_INT32:
                        format = 2;
                        break;
                    case CL_UNSIGNED_INT8:
                        format = 7;
                        break;
                    case CL_UNSIGNED_INT16:
                        format = 6;
                        break;
                    case CL_UNSIGNED_INT32:
                        format = 5;
                        break;
                    case CL_HALF_FLOAT:
                        format = 1;
                        break;
                    case CL_FLOAT:
                    default:
                        format = 0;
                        break;
                    }
                    tiling = (memObj->u.image.tiling == gcvLINEAR ? 0 : memObj->u.image.tiling == gcvTILED ? 1 : memObj->u.image.tiling == gcvSUPERTILED ? 2 : 3);
                    switch(memObj->u.image.imageFormat.image_channel_order)
                    {
                    case CL_R:
                    case CL_Rx:
                        componentCount = 1;
                        swizzleR = 0;
                        swizzleG = 4;
                        swizzleB = 4;
                        swizzleA = 5;
                        break;
                    case CL_A:
                        componentCount = 1;
                        swizzleR = 4;
                        swizzleG = 4;
                        swizzleB = 4;
                        swizzleA = 3;
                        break;
                    case CL_RG:
                    case CL_RGx:
                        componentCount = 2;
                        swizzleR = 0;
                        swizzleG = 1;
                        swizzleB = 4;
                        swizzleA = 5;
                        break;
                    case CL_RA:
                        componentCount = 2;
                        swizzleR = 0;
                        swizzleG = 4;
                        swizzleB = 4;
                        swizzleA = 3;
                        break;
                    case CL_RGB:
                        componentCount = 3;
                        swizzleR = 0;
                        swizzleG = 1;
                        swizzleB = 2;
                        swizzleA = 5;
                        break;
                    case CL_RGBA:
                        componentCount = 0;
                        swizzleR = 0;
                        swizzleG = 1;
                        swizzleB = 2;
                        swizzleA = 3;
                        break;
                    case CL_BGRA:
                        componentCount = 0;
                        swizzleR = 2;
                        swizzleG = 1;
                        swizzleB = 0;
                        swizzleA = 3;
                        break;
                    case CL_ARGB:
                        componentCount = 0;
                        swizzleR = 1;
                        swizzleG = 2;
                        swizzleB = 3;
                        swizzleA = 0;
                        break;
                    case CL_INTENSITY:
                        componentCount = 0;
                        swizzleR = 0;
                        swizzleG = 0;
                        swizzleB = 0;
                        swizzleA = 0;
                        break;
                    case CL_LUMINANCE:
                        componentCount = 1;
                        swizzleR = 0;
                        swizzleG = 0;
                        swizzleB = 0;
                        swizzleA = 5;
                        break;
                    default:
                        componentCount = 0;
                        break;
                    }

                    /* Get main kernel function. */
                    gcmASSERT(Shader->kernelFunctions);
                    for (i = 0; i < Shader->kernelFunctionCount; i++)
                    {
                        if (Shader->kernelFunctions[i] == gcvNULL) continue;

                        if (Shader->kernelFunctions[i]->isMain)
                        {
                            kernelFunction = Shader->kernelFunctions[i];
                            break;
                        }
                    }

                    gcmASSERT(kernelFunction);
                    clmCHECK_ERROR(kernelFunction == gcvNULL, CL_INVALID_VALUE);
                    imageSampler = &kernelFunction->imageSamplers[Arg->uniform->imageSamplerIndex];

                    if(imageSampler != gcvNULL)
                    {
                        if(imageSampler->isConstantSamplerType == gcvFALSE)
                        {
                            for(tmpArg=baseArg, i=0; i<numArg; tmpArg++, i++)
                            {
                                if(tmpArg->uniform != gcvNULL && isUniformKernelArgSampler(tmpArg->uniform))
                                {
                                    cleSAMPLER sampler = *(cleSAMPLER *) tmpArg->data;
                                    if((sampler & CLK_ADDRESS_CLAMP) == CLK_ADDRESS_CLAMP)
                                    {
                                        switch(memObj->u.image.imageFormat.image_channel_order)
                                        {
                                        case CL_A:
                                        case CL_INTENSITY:
                                        case CL_Rx:
                                        case CL_RA:
                                        case CL_RGx:
                                        default:
                                            addressMode = 1;
                                            break;
                                        case CL_R:
                                        case CL_RG:
                                        case CL_RGB:
                                        case CL_LUMINANCE:
                                            addressMode = 2;
                                            break;
                                        }
                                    }
                                    else if((sampler & CLK_ADDRESS_CLAMP_TO_EDGE) == CLK_ADDRESS_CLAMP_TO_EDGE)
                                    {
                                        addressMode = 3;
                                    }
                                    else
                                    {
                                        addressMode = 0;
                                    }
                                    break;
                                }
                            }
                        }
                        else
                        {
                            if((imageSampler->samplerType & 0xF) == CLK_ADDRESS_CLAMP)
                            {
                                switch(memObj->u.image.imageFormat.image_channel_order)
                                {
                                    case CL_A:
                                    case CL_INTENSITY:
                                    case CL_Rx:
                                    case CL_RA:
                                    case CL_RGx:
                                    default:
                                        addressMode = 1;
                                        break;
                                    case CL_R:
                                    case CL_RG:
                                    case CL_RGB:
                                    case CL_LUMINANCE:
                                        addressMode = 2;
                                        break;
                                }
                            }
                            else if((imageSampler->samplerType & 0xF) == CLK_ADDRESS_CLAMP_TO_EDGE)
                            {
                                addressMode = 3;
                            }
                        }
                    }
                    else if(Kernel->program->buildOptions != gcvNULL && strstr(Kernel->program->buildOptions, "-cl-viv-vx-extension") != gcvNULL)
                    {
                        addressMode = 1;
                    }

                    tmpData[3] =  shift | (addressMode<<4) | (format<<6) | (tiling<<10) | (type<<12)
                        | (componentCount<<14) | (swizzleR<<16) | (swizzleG<<20) | (swizzleB<<24) | (swizzleA<<28);
                                  /*gcmSETFIELD(0, GCREG_SH_IMAGE, SHIFT, shift)
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (addressMode) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (tiling) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentCount) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (swizzleR) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (swizzleG) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (swizzleB) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                                  | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (swizzleA) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));*/

                    tmpData[4] = sliceSize;
                    tmpData[5] = depth;
                    tmpData[6] = ((gctUINT)(memObj->u.image.imageFormat.image_channel_data_type << 16) | (gctUINT)(memObj->u.image.imageFormat.image_channel_order));
                    data = (gctINT *)tmpData;

                    clmONERROR(clfSetImageValue(Arg->uniform, length, data), CL_INVALID_VALUE);
                }
                else
                {
                    do
                    {
                        gctUINT count, i;
                        gctCONST_STRING name;
                        char nameImage[128];
                        gcUNIFORM uniform;
                        gceSTATUS status;

                        gcmERR_BREAK(gcSHADER_GetUniformCount(Shader, &count));
                        gcmERR_BREAK(gcUNIFORM_GetName(Arg->uniform, gcvNULL, &name));
                        gcmERR_BREAK(gcoOS_StrCopySafe(nameImage, gcmSIZEOF(nameImage), name));
                        gcmERR_BREAK(gcoOS_StrCatSafe(nameImage, gcmSIZEOF(nameImage), "#size"));

                        for (i = 0; i < count; i++)
                        {
                            gcmERR_BREAK(gcSHADER_GetUniform(Shader, i, &uniform));
                            if (uniform != gcvNULL)
                            {
                                gcmERR_BREAK(gcUNIFORM_GetName(uniform, gcvNULL, &name));
                                if (gcoOS_StrCmp(name, nameImage) == gcvSTATUS_OK)
                                {
                                    gctINT image[4];
                                    clsImageHeader_PTR imageHeader = (clsImageHeader_PTR) memObj->u.image.headerLogical;
                                    image[0] = imageHeader->width;
                                    image[1] = imageHeader->rowPitch;
                                    image[2] = imageHeader->width - 1;
                                    image[3] = imageHeader->height - 1;
                                    gcmERR_BREAK(clfSetUniformValue(uniform, 1, image));
                                    data = (gctINT *) &imageHeader->physical;
                                    break;
                                }
                            }
                        }
                    }
                    while (gcvFALSE);

                    clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                       CL_INVALID_VALUE);
                }
#endif
            }
        }
        else
        {
            gctUINT arraySize;
            gctUINT32 physicalAddress, numCols, numRows;
            gctUINT8_PTR pData = (gctUINT8_PTR)Arg->data;
            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT16_X1:
            case gcSHADER_FLOAT16_X2:
            case gcSHADER_FLOAT16_X3:
            case gcSHADER_FLOAT16_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
            case gcSHADER_FLOAT16_P2:
            case gcSHADER_FLOAT16_P3:
            case gcSHADER_FLOAT16_P4:
            case gcSHADER_FLOAT16_P8:
            case gcSHADER_FLOAT16_P16:
                clmONERROR(clfSetUniformValue(Arg->uniform,
                                               length,
                                               Arg->data),
                           CL_INVALID_VALUE);
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
            case gcSHADER_INT8_X1:
            case gcSHADER_INT8_X2:
            case gcSHADER_INT8_X3:
            case gcSHADER_INT8_X4:
            case gcSHADER_INT16_X1:
            case gcSHADER_INT16_X2:
            case gcSHADER_INT16_X3:
            case gcSHADER_INT16_X4:
            case gcSHADER_UINT_X1:
            case gcSHADER_UINT_X2:
            case gcSHADER_UINT_X3:
            case gcSHADER_UINT_X4:
            case gcSHADER_UINT8_X1:
            case gcSHADER_UINT8_X2:
            case gcSHADER_UINT8_X3:
            case gcSHADER_UINT8_X4:
            case gcSHADER_UINT16_X1:
            case gcSHADER_UINT16_X2:
            case gcSHADER_UINT16_X3:
            case gcSHADER_UINT16_X4:

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                case gcSL_INT16:
                case gcSL_UINT16:
                    {
                        /* Unpack argument data into 4B chunks */
                        gctUINT32 *data= gcvNULL;
                        gctUINT32 signMask, signExt;
                        gctPOINTER pointer= gcvNULL;
                        gctSIZE_T i, elemSize, numElem, bytes;

                        elemSize =  ((format == gcSL_INT8)   ? 1 :
                                     (format == gcSL_UINT8)  ? 1 :
                                     (format == gcSL_INT16)  ? 2 :
                                     (format == gcSL_UINT16) ? 2 : 0);

                        signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                     (format == gcSL_INT16)  ? 0x8000 : 0);

                        signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                     (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                        numElem = Arg->size / elemSize;
                        bytes = numElem * 4;

                        clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
                        gcoOS_ZeroMemory(pointer, bytes);
                        data = (gctUINT32 *) pointer;
                        for (i=0; i<numElem; i++) {
                            pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                            gcoOS_MemCopy(&data[i], pointer, elemSize);
                            if (data[i]&signMask) {
                                data[i] |= signExt;
                            }
                        }
                        pointer = data;

                        status = clfSetUniformValue(Arg->uniform, length, pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status))
                        {
                            clmRETURN_ERROR(CL_INVALID_VALUE);
                        }

                    }
                    break;
                default:
                    clmONERROR(clfSetUniformValue(Arg->uniform, length, Arg->data),
                               CL_INVALID_VALUE);
                    break;
                }
                break;
            case gcSHADER_INT8_P2:
            case gcSHADER_UINT8_P2:
            case gcSHADER_INT16_P2:
            case gcSHADER_UINT16_P2:
            case gcSHADER_INT8_P3:
            case gcSHADER_UINT8_P3:
            case gcSHADER_INT16_P3:
            case gcSHADER_UINT16_P3:
            case gcSHADER_INT8_P4:
            case gcSHADER_UINT8_P4:
            case gcSHADER_INT16_P4:
            case gcSHADER_UINT16_P4:
            case gcSHADER_INT8_P8:
            case gcSHADER_UINT8_P8:
            case gcSHADER_INT16_P8:
            case gcSHADER_UINT16_P8:
            case gcSHADER_INT8_P16:
            case gcSHADER_UINT8_P16:
            case gcSHADER_INT16_P16:
            case gcSHADER_UINT16_P16:
                clmONERROR(clfSetUniformValue(Arg->uniform, length, Arg->data),
                            CL_INVALID_VALUE);
                break;

            case gcSHADER_INT64_X1:
            case gcSHADER_INT64_X2:
            case gcSHADER_INT64_X3:
            case gcSHADER_INT64_X4:
            case gcSHADER_UINT64_X1:
            case gcSHADER_UINT64_X2:
            case gcSHADER_UINT64_X3:
            case gcSHADER_UINT64_X4:
                clmONERROR(gcSHADER_ComputeUniformPhysicalAddress(hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress), CL_INVALID_VALUE);

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                arraySize = Arg->uniform->arraySize;

                clmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress,GetUniformPhysical(Arg->uniform),
                                                  numCols, numRows, arraySize, gcvTRUE,
                                                  sizeof(cl_long),
                                                  4*sizeof(cl_long),
                                                  pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)), CL_INVALID_VALUE);
                break;

            case gcSHADER_IMAGE_2D_T:
            case gcSHADER_IMAGE_3D_T:
            case gcSHADER_IMAGE_1D_T:
            case gcSHADER_IMAGE_1D_ARRAY_T:
            case gcSHADER_IMAGE_1D_BUFFER_T:
            case gcSHADER_IMAGE_2D_ARRAY_T:
                {
                clsMem_PTR image = *(clsMem_PTR *) Arg->data;
                gctINT * data = (gctINT *) &image->u.image.headerPhysical;

                gcoCL_MemWaitAndGetFence(image->u.image.headerNode, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

                clmONERROR(clfSetUniformValue(Arg->uniform,
                                              length,
                                              data),
                           CL_INVALID_VALUE);
                }
                break;

            case gcSHADER_SAMPLER_T:
                {
                gcmASSERT(length == 1);
                clmONERROR(clfSetUniformValue(Arg->uniform,
                                              length,
                                              Arg->data),
                           CL_INVALID_VALUE);
                }
                break;

            default:
                break;
            }
        }
    }
    else if (isUniformWorkDim(Arg->uniform))
    {
        gcoOS_MemCopy(Arg->data, &WorkDim, gcmSIZEOF(WorkDim));

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformGlobalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 globalWorkSize[3];

        globalWorkSize[0] = GlobalWorkSize[0] ? GlobalWorkSize[0] : 1;
        globalWorkSize[1] = GlobalWorkSize[1] ? GlobalWorkSize[1] : 1;
        globalWorkSize[2] = GlobalWorkSize[2] ? GlobalWorkSize[2] : 1;
        gcoOS_MemCopy(Arg->data, globalWorkSize, gcmSIZEOF(gctUINT32) * 3);

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformLocalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, LocalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 localWorkSize[3];

        localWorkSize[0] = LocalWorkSize[0] ? LocalWorkSize[0] : 1;
        localWorkSize[1] = LocalWorkSize[1] ? LocalWorkSize[1] : 1;
        localWorkSize[2] = LocalWorkSize[2] ? LocalWorkSize[2] : 1;
        gcoOS_MemCopy(Arg->data, localWorkSize, gcmSIZEOF(gctUINT32) * 3);

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformNumGroups(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*size_t numGroups[3];*/
        gctUINT32 numGroups[3];

        numGroups[0] = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1);
        numGroups[1] = WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 0;
        numGroups[2] = WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 0;

        gcoOS_MemCopy(Arg->data, numGroups, gcmSIZEOF(numGroups));

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);

    }
    else if (isUniformNumGroupsForSingleGPU(Arg->uniform))
    {
        gctUINT32 gpuCount,usedGpu;
        gctUINT i = 0, j = 0, maxDimIndex = 0;
        gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0 };
        gctUINT eachGPUWorkGroupNum[gcdMAX_3DGPU_COUNT][3] = {{ 0 }};
        gctUINT eachGPUGroupCount, restGroupCount;
        gctINT *datas[gcdMAX_3DGPU_COUNT] = {gcvNULL};
        gctUINT maxWorkGroupCount = GlobalWorkSize[0] / LocalWorkSize[0];
        gctBOOL atomic_access = (hints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL] & gceMA_FLAG_ATOMIC) != 0;
        /* TODO - For 64-bit GPU. */
        /*size_t numGroups[3];*/
        gctUINT32 numGroups[3]    = { 0 };

        gcoCL_GetHWConfigGpuCount(&gpuCount);
        usedGpu = gpuCount;

        numGroups[0] = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1);
        numGroups[1] = WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 0;
        numGroups[2] = WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 0;

        if (gpuCount > 1 && !atomic_access)
        {
            for(i = 1; i < WorkDim; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
            {
                if(GlobalWorkSize[i] / LocalWorkSize[i] > maxWorkGroupCount)
                {
                    maxWorkGroupCount = GlobalWorkSize[i] / LocalWorkSize[i];
                    maxDimIndex = i;
                }
            }

            eachGPUGroupCount = maxWorkGroupCount / gpuCount;
            restGroupCount = maxWorkGroupCount % gpuCount;

            for(i = 0 ;i < gpuCount; i++)
            {
                eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
            }

            for(i = 0 ;i <restGroupCount; i++)
            {
                eachGPUWorkGroupSizes[i]++;
            }

            if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

            for (i = 0; i < WorkDim; i++)
            {
                if (i == maxDimIndex)
                {
                    for (j = 0; j < usedGpu; j++)
                    {
                        eachGPUWorkGroupNum[j][i] = eachGPUWorkGroupSizes[j];
                    }
                }
                else
                {
                    for (j = 0; j < usedGpu; j++)
                    {
                        eachGPUWorkGroupNum[j][i] = numGroups[i];
                    }
                }
            }

            for(i = 0; i < gpuCount; i++)
            {
                datas[i] = (gctINT *) eachGPUWorkGroupNum[i];
            }

           clmONERROR(clfSetUniformValueCombinedMode(Arg->uniform,
                                          length,
                                          datas,
                                          gpuCount),
                      CL_INVALID_VALUE);
        }
        else
        {
            gcoOS_MemCopy(Arg->data, numGroups, gcmSIZEOF(numGroups));

            clmONERROR(clfSetUniformValue(Arg->uniform,
                                          length,
                                          Arg->data),
                       CL_INVALID_VALUE);
        }

    }
    else if (isUniformGlobalOffset(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkOffset, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 glocalWorkOffset[3];

        glocalWorkOffset[0] = GlobalWorkOffset[0];
        glocalWorkOffset[1] = GlobalWorkOffset[1];
        glocalWorkOffset[2] = GlobalWorkOffset[2];
        gcoOS_MemCopy(Arg->data, glocalWorkOffset, gcmSIZEOF(gctUINT32) * 3);

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformWorkGroupCount(Arg->uniform))
    {
        gctUINT16 workGroupCount = 0;
        gctINT totalNumGroups = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                                * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                                * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1);

        /*
        ** If workGroupCount is chosen, set the value to workGroupCount;
        ** otherwise use 0 because (i MOD 0) == i.
        */
        if (hints->workGroupCount != 0 &&
            (gctINT)hints->workGroupCount < totalNumGroups)
        {
            workGroupCount = hints->workGroupCount;
        }

        gcoOS_MemCopy(Arg->data, &workGroupCount, gcmSIZEOF(gctUINT16));
        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformLocalAddressSpace(Arg->uniform))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;
        gctINT * data;

        /* If use HW local address, then local memory address starts at 0. */
        if (gcmOPT_hasFeature(FB_FORCE_LS_ACCESS) /* triage option */
            ||
            (strcmp(Arg->uniform->name, _sldLocalStorageAddressName) == 0 && gcShaderUseLocalMem(Shader))
            )
        {
            /* If this shader use HW local storage directly, we should not load this uniform. */
            gcmASSERT(gcvFALSE);
        }
        /* Size may be zero if declared but never used */
        else if (memAllocInfo->allocatedSize > 0)
        {
            gctUINT32 gpuCount, perGpuMemSize;
            gctINT totalNumGroups = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                                  * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                                  * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1);

            clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

            gcmASSERT(!hints->sharedMemAllocByCompiler);

            gcoCL_GetHWConfigGpuCount(&gpuCount);

            /* Compare the workGroupCount with total group number and choose the smaller one. */
            if (hints->workGroupCount != 0 &&
                (gctINT)hints->workGroupCount < totalNumGroups)
            {
                totalNumGroups = hints->workGroupCount;
            }

            memAllocInfo->allocatedSize *= totalNumGroups;
            memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 128);
            perGpuMemSize = memAllocInfo->allocatedSize;
            memAllocInfo->allocatedSize *= gpuCount;

            /* Allocate the physical buffer */
            clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->node,
                                            gcvSURF_INDEX,
                                            0),
                       CL_INVALID_VALUE);

            data = (gctINT *) &memAllocInfo->physical;

            if(gpuCount == 1)
            {
                clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                    CL_INVALID_VALUE);
            }
            else
            {
                gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                gctINT * datas[gcdMAX_3DGPU_COUNT];
                gctUINT32 index = 0;
                gpuPhysicals[0] = memAllocInfo->physical;
                datas[0] = (gctINT *) &gpuPhysicals[0];

                for(index = 1; index < gpuCount; ++index)
                {
                    gpuPhysicals[index] = gpuPhysicals[index-1] + perGpuMemSize;
                    datas[index] = (gctINT *) &gpuPhysicals[index];
                }

                clfSetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount);

            }

            gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_WRITE);
        }
    }
    else if (isUniformWorkThreadCount(Arg->uniform))
    {
        gctUINT16 workThreadCount = 0;
        gctINT totalNumItems  = GlobalWorkSize[0]
                                * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                * (WorkDim > 2 ? GlobalWorkSize[2] : 1);

        /*
        ** If workThreadCount is chosen, set the value to workThreadCount;
        ** otherwise use 0 because (i MOD 0) == i.
        */
        if (hints->workThreadCount != 0 &&
            (gctINT)hints->workThreadCount < totalNumItems)
        {
            workThreadCount = hints->workThreadCount;
        }

        gcoOS_MemCopy(Arg->data, &workThreadCount, gcmSIZEOF(gctUINT16));
        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else  if(isUniformTempRegSpillAddress(Arg->uniform))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;

        if (memAllocInfo->allocatedSize > 0)
        {
            gctINT* datas[gcdMAX_3DGPU_COUNT];
            gctUINT32 physicals[gcdMAX_3DGPU_COUNT];
            gctUINT32 i;
            gctUINT32 gpuCount;
            gctUINT32 perGpuMemSize;
            gctUINT32 physicalAddress;
            gcoCL_GetHWConfigGpuCount(&gpuCount);
            memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 64);
            perGpuMemSize =  memAllocInfo->allocatedSize;
            memAllocInfo->allocatedSize *= gpuCount;

            clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                              &memAllocInfo->physical,
                                              &memAllocInfo->logical,
                                              &memAllocInfo->node,
                                              gcvSURF_INDEX,
                                              0),
                                              CL_INVALID_VALUE
                                              );

            physicals[0] = memAllocInfo->physical;
            datas[0] = (gctINT *) &physicals[0];

            for(i = 1; i < gpuCount; i++)
            {
                physicals[i] = physicals[i-1] + perGpuMemSize;
                datas[i] = (gctINT *) &physicals[i];
            }

            clmONERROR(gcSHADER_ComputeUniformPhysicalAddress(hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress), CL_INVALID_VALUE);
            clmONERROR(clfSetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount),
                       CL_INVALID_VALUE);
        }

    }
    else if(isUniformWorkGroupIdOffset(Arg->uniform))
    {
        gctUINT32 gpuCount,usedGpu;
        gctUINT i = 0, maxDimIndex = 0;
        gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0 };
        gctUINT eachGPUWorkGroupIDOffsets[gcdMAX_3DGPU_COUNT][3] = {{ 0 }};
        gctUINT eachGPUGroupCount, restGroupCount;
        gctINT *datas[gcdMAX_3DGPU_COUNT] = {gcvNULL};
        gctUINT maxWorkGroupCount = GlobalWorkSize[0] / LocalWorkSize[0];

        gcoCL_GetHWConfigGpuCount(&gpuCount);
        usedGpu = gpuCount;

        gcmASSERT(gpuCount > 1) ; /* We can assure it 's combined mode */

        for(i = 1; i < WorkDim; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
        {
            if(GlobalWorkSize[i] / LocalWorkSize[i] > maxWorkGroupCount)
            {
                maxWorkGroupCount = GlobalWorkSize[i] / LocalWorkSize[i];
                maxDimIndex = i;
            }
        }

        eachGPUGroupCount = maxWorkGroupCount / gpuCount;
        restGroupCount = maxWorkGroupCount % gpuCount;

        for(i = 0 ;i < gpuCount; i++)
        {
            eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
        }

        for(i = 0 ;i <restGroupCount; i++)
        {
            eachGPUWorkGroupSizes[i]++;
        }

        if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

        eachGPUWorkGroupIDOffsets[0][maxDimIndex] = 0;

        for(i = 1; i < usedGpu; i++)
        {
            eachGPUWorkGroupIDOffsets[i][maxDimIndex] = eachGPUWorkGroupSizes[i - 1] +  eachGPUWorkGroupIDOffsets[i-1][maxDimIndex];

        }

        for(i = 0; i < gpuCount; i++)
        {
            datas[i] = (gctINT *) eachGPUWorkGroupIDOffsets[i];
        }

       clmONERROR(clfSetUniformValueCombinedMode(Arg->uniform,
                                      length,
                                      datas,
                                      gpuCount),
                  CL_INVALID_VALUE);

    }
    else if(isUniformPrivateAddressSpace(Arg->uniform))
    {
        gctUINT32 gpuCount, perGpuMemSize;
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;

        /* Size may be zero if declared but never used */
        if (memAllocInfo->allocatedSize > 0)
        {
            gctINT * data;
            gctUINT argIdx;
            clsArgument_PTR tempArg;
            gctBOOL isFirstPrivateArg = gcvTRUE;

            gctINT totalNumItems  = GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1);

            clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

            /* Compare the workThreadCount with total item number and choose the smaller one. */
            if (hints->workThreadCount != 0 &&
                (gctINT)hints->workThreadCount < totalNumItems)
            {
                memAllocInfo->allocatedSize *= hints->workThreadCount;
            }
            else
            {
                memAllocInfo->allocatedSize *= totalNumItems;
            }

            gcoCL_GetHWConfigGpuCount(&gpuCount);

            memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 128);
            perGpuMemSize =  memAllocInfo->allocatedSize;
            memAllocInfo->allocatedSize *= gpuCount;

            if ((*privateBufList) == gcvNULL)
            {
                clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsPrivateBuffer), (gctPOINTER *)privateBufList), CL_OUT_OF_HOST_MEMORY);

                /* Initialize Buffer List */
                gcoOS_ZeroMemory((*privateBufList), sizeof(clsPrivateBuffer));
                (*privateBufList)->kernel = Kernel;

                clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsMemAllocInfo), (gctPOINTER *)&privateBuf), CL_OUT_OF_HOST_MEMORY);

                /* Allocate the physical buffer */

                status = gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                              &memAllocInfo->physical,
                                              &memAllocInfo->logical,
                                              &memAllocInfo->node,
                                              gcvSURF_INDEX,
                                              0);
                if(gcmIS_ERROR(status))
                {
                    if(privateBuf != gcvNULL)
                    {
                        gcmOS_SAFE_FREE(gcvNULL, privateBuf);
                    }
                    clmONERROR(status, CL_INVALID_VALUE);
                }

                gcoOS_MemCopy(privateBuf, memAllocInfo, sizeof(clsMemAllocInfo));
                (*privateBufList)->buffer = privateBuf;
                (*privateBufList)->next = gcvNULL;
                (*privateBufList)->kernel = Kernel;
                (*privateBufList)->uniform = Arg->uniform;
            }
            else
            {
                clsPrivateBuffer_PTR node = gcvNULL;
                gctBOOL found = gcvFALSE;

                /* check previous arguments to see if it's the first private argument in the kernel */
                for (argIdx = 0; argIdx < numArg; argIdx++)
                {
                    tempArg = baseArg + argIdx;
                    if (!(tempArg->uniform) || isUniformInactive(tempArg->uniform))
                        continue;
                    if (isUniformKernelArg(tempArg->uniform) || isUniformKernelArgConstant(tempArg->uniform))
                        continue;
                    if (isUniformWorkDim(tempArg->uniform))
                        continue;
                    if (isUniformGlobalSize(tempArg->uniform))
                        continue;
                    if (isUniformNumGroups(tempArg->uniform))
                        continue;
                    if (isUniformNumGroupsForSingleGPU(tempArg->uniform))
                        continue;
                    if (isUniformLocalSize(tempArg->uniform))
                        continue;
                    if (isUniformGlobalOffset(tempArg->uniform))
                        continue;
                    if (isUniformLocalAddressSpace(tempArg->uniform))
                        continue;
                    if (isUniformPrivateAddressSpace(tempArg->uniform))
                    {
                        if (tempArg!= Arg)
                        {
                            isFirstPrivateArg = gcvFALSE;
                            break;
                        }
                    }
                }

                /* search in the list */
                for (node = (*privateBufList); node != gcvNULL; node = node->next)
                {
                    if (isFirstPrivateArg)
                    {
                        /* re-use the first buffer in the list */
                        found = gcvTRUE;
                        privateBuf = node->buffer;
                        /* update node */
                        node->kernel = Kernel;
                        node->uniform = Arg->uniform;
                        break;
                    }
                    else
                    {
                        /* find the same uniform in the same kernel */
                        if (Kernel == node->kernel && Arg->uniform == node->uniform)
                        {
                            found = gcvTRUE;
                            privateBuf =  node->buffer;
                            break;
                        }
                    }
                }

                if (found)
                {
                    if (memAllocInfo->allocatedSize > privateBuf->allocatedSize)
                    {
                        /* flush first */
                        clmONERROR(gcoCL_Flush(gcvTRUE), CL_INVALID_VALUE);

                        clmONERROR(gcoCL_FreeMemory(privateBuf->physical,
                                         privateBuf->logical,
                                         privateBuf->allocatedSize,
                                         privateBuf->node,
                                         gcvSURF_INDEX), CL_INVALID_VALUE);

                        /* re-allocate the physical buffer */
                        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                                        &memAllocInfo->physical,
                                                        &memAllocInfo->logical,
                                                        &memAllocInfo->node,
                                                        gcvSURF_INDEX,
                                                        0),
                                   CL_INVALID_VALUE);
                        gcoOS_MemCopy(privateBuf, memAllocInfo, sizeof(clsMemAllocInfo));

                        node->buffer = privateBuf;
                    }
                    else
                    {
                        memAllocInfo = privateBuf;
                    }
                }
                else
                {
                    /* append a new buffer to the list */
                    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsPrivateBuffer), (gctPOINTER *)&tempNode), CL_OUT_OF_HOST_MEMORY);

                    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsMemAllocInfo), (gctPOINTER *)&privateBuf), CL_OUT_OF_HOST_MEMORY);
                    status = gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                                  &memAllocInfo->physical,
                                                  &memAllocInfo->logical,
                                                  &memAllocInfo->node,
                                                  gcvSURF_INDEX,
                                                  0);
                    if(gcmIS_ERROR(status))
                    {
                        if(tempNode != gcvNULL)
                        {
                            gcmOS_SAFE_FREE(gcvNULL, tempNode);
                        }
                        if (privateBuf != gcvNULL)
                        {
                            gcmOS_SAFE_FREE(gcvNULL, privateBuf);
                        }
                        clmONERROR(status, CL_INVALID_VALUE);
                    }

                    gcoOS_MemCopy(privateBuf, memAllocInfo, sizeof(clsMemAllocInfo));

                    tempNode->buffer = privateBuf;
                    tempNode->next = gcvNULL;
                    tempNode->kernel = Kernel;
                    tempNode->uniform = Arg->uniform;

                    for (node = (*privateBufList); node != gcvNULL; node = node->next)
                    {
                        if (node->next == gcvNULL)
                        {
                            node->next = tempNode;
                            break;
                        }
                    }
                }
            }

            if (gcmOPT_hasFeature(FB_FORCE_LS_ACCESS) /* triage option */
                || (strcmp(Arg->uniform->name, _sldLocalStorageAddressName) == 0 &&
                    gcShaderUseLocalMem(Shader))
                )
            {
                /* Local memory address starts at 0. */
                gctINT physical = 0;
                data = (gctINT *) &physical;
                clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);
            }
            else
            {
                gctINT* datas[gcdMAX_3DGPU_COUNT];
                gctUINT32 physicals[gcdMAX_3DGPU_COUNT];
                gctUINT index;
                data = (gctINT *) &memAllocInfo->physical;
                if(gpuCount == 1)
                {
                    clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                        CL_INVALID_VALUE);
                }
                else
                {
                    physicals[0] = memAllocInfo->physical;
                    datas[0] = (gctINT*) &physicals[0];

                    for(index = 1; index < gpuCount; index++)
                    {
                        physicals[index] = physicals[index-1] + perGpuMemSize;
                        datas[index] = (gctINT*) &physicals[index];
                    }

                    clmONERROR(clfSetUniformValueCombinedMode(Arg->uniform, length, datas, gpuCount),
                               CL_INVALID_VALUE);
                }

                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
            }
        }
    }
    else if (isUniformConstantAddressSpace(Arg->uniform) && (GetUniformPhysical(Arg->uniform) != -1))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;
        gctINT * data;

        clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
        clmASSERT(memAllocInfo->allocatedSize > 0, CL_INVALID_VALUE);

        /* Allocate the physical buffer */
        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node,
                                        gcvSURF_INDEX,
                                        0),
                   CL_INVALID_VALUE);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                   CL_INVALID_VALUE);

        gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_READ);

        /* Copy the constant data to the buffer */
        gcoOS_MemCopy(memAllocInfo->logical, Kernel->constantMemBuffer, Kernel->constantMemSize);

        gcoCL_FlushMemory(memAllocInfo->node, memAllocInfo->logical, memAllocInfo->allocatedSize);

        gcmDUMP(gcvNULL, "#[info: constant memory");

        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       gcmPTR2SIZE(memAllocInfo->physical),
                       memAllocInfo->logical,
                       0,
                       Kernel->constantMemSize);
    }
    else if (isUniformPrintfAddress(Arg->uniform))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;
        gctINT * data;
        gctINT totalNumItems  = GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1);

        clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
        memAllocInfo->allocatedSize = Kernel->context->devices[0]->deviceInfo.maxPrintfBufferSize;

        /* Allocate the physical buffer */
        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node,
                                        gcvSURF_INDEX,
                                        0),
                   CL_INVALID_VALUE);

        /* Initialize printf buffer with 0xFF so we can use it to check if it is written. */
        gcoOS_MemFill(memAllocInfo->logical, 0xFF, memAllocInfo->allocatedSize);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                   CL_INVALID_VALUE);
        Arg->printThreadNum = totalNumItems;

        gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_WRITE);
    }
    else if (isUniformWorkItemPrintfBufferSize(Arg->uniform))
    {
        gctINT totalNumItems  = GlobalWorkSize[0]
                                  * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                                  * (WorkDim > 2 ? GlobalWorkSize[2] : 1);

        gctINT printBufferSize = Kernel->context->devices[0]->deviceInfo.maxPrintfBufferSize / totalNumItems;
        gcoOS_MemCopy(Arg->data, &printBufferSize, gcmSIZEOF(printBufferSize));

        clmONERROR(clfSetUniformValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);

        Arg->printBufferSizePerThread = printBufferSize;
    }
    else if (isUniformKernelArgSampler(Arg->uniform))
    {
        gcmASSERT(length == 1);
        clmONERROR(clfSetUniformValue(Arg->uniform,
                                        length,
                                        Arg->data),
                    CL_INVALID_VALUE);

    }
    else if (isUniformKernelArgLocal(Arg->uniform) ||
                isUniformKernelArgLocalMemSize(Arg->uniform))
    {
        /* Special case, handled separately, do nothing now */
    }
    else if (isUniformKernelArgPrivate(Arg->uniform))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;
        gctINT * data;
        gctINT i;
        gctPOINTER pointer;

        gctINT totalNumItems  = GlobalWorkSize[0]
                              * (WorkDim > 1 ? GlobalWorkSize[1] : 1)
                              * (WorkDim > 2 ? GlobalWorkSize[2] : 1);

        clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
        clmASSERT(memAllocInfo->allocatedSize > 0, CL_INVALID_VALUE);

        memAllocInfo->allocatedSize *= totalNumItems;

        /* Allocate the physical buffer */
        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                        &memAllocInfo->physical,
                                        &memAllocInfo->logical,
                                        &memAllocInfo->node,
                                        gcvSURF_INDEX,
                                        0),
                   CL_INVALID_VALUE);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(clfSetUniformValue(Arg->uniform, length, data),
                   CL_INVALID_VALUE);

        gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

        /* Copy the private data to the buffer */
        for (i = 0; i < totalNumItems; i++)
        {
            pointer = (gctPOINTER)((gctUINTPTR_T)memAllocInfo->logical + i*Arg->size);
            gcoOS_MemCopy(pointer, memAllocInfo->data, Arg->size);
        }
    }
    else if (isUniformKernelArgPatch(Arg->uniform))
    {
        switch (type)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT16_X1:
        case gcSHADER_FLOAT16_X2:
        case gcSHADER_FLOAT16_X3:
        case gcSHADER_FLOAT16_X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_4X4:
            clmONERROR(clfSetUniformValue(Arg->uniform,
                                           length,
                                           Arg->data),
                       CL_INVALID_VALUE);
            break;

        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_INT8_X1:
        case gcSHADER_INT8_X2:
        case gcSHADER_INT8_X3:
        case gcSHADER_INT8_X4:
        case gcSHADER_INT16_X1:
        case gcSHADER_INT16_X2:
        case gcSHADER_INT16_X3:
        case gcSHADER_INT16_X4:
        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
        case gcSHADER_UINT8_X1:
        case gcSHADER_UINT8_X2:
        case gcSHADER_UINT8_X3:
        case gcSHADER_UINT8_X4:
        case gcSHADER_UINT16_X1:
        case gcSHADER_UINT16_X2:
        case gcSHADER_UINT16_X3:
        case gcSHADER_UINT16_X4:

            switch (format)
            {
            case gcSL_INT8:
            case gcSL_UINT8:
            case gcSL_INT16:
            case gcSL_UINT16:
                {
                    /* Unpack argument data into 4B chunks */
                    gctUINT32 *data= gcvNULL;
                    gctUINT32 signMask, signExt;
                    gctPOINTER pointer= gcvNULL;
                    gctSIZE_T i, elemSize, numElem, bytes;

                    elemSize =  ((format == gcSL_INT8)   ? 1 :
                                 (format == gcSL_UINT8)  ? 1 :
                                 (format == gcSL_INT16)  ? 2 :
                                 (format == gcSL_UINT16) ? 2 : 0);

                    signMask =  ((format == gcSL_INT8)   ? 0x80 :
                                 (format == gcSL_INT16)  ? 0x8000 : 0);

                    signExt =   ((format == gcSL_INT8)   ? 0xFFFFFF00 :
                                 (format == gcSL_INT16)  ? 0xFFFF0000 : 0);

                    numElem = Arg->size / elemSize;
                    bytes = numElem * 4;

                    clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
                    gcoOS_ZeroMemory(pointer, bytes);
                    data = (gctUINT32 *) pointer;
                    for (i=0; i<numElem; i++) {
                        pointer = (gctPOINTER)(((gctUINTPTR_T)Arg->data)+(i*elemSize));
                        gcoOS_MemCopy(&data[i], pointer, elemSize);
                        if (data[i]&signMask) {
                            data[i] |= signExt;
                        }
                    }
                    pointer = data;
                    status = clfSetUniformValue(Arg->uniform, length, pointer);
                    gcoOS_Free(gcvNULL, data);
                    if (gcmIS_ERROR(status)) clmRETURN_ERROR(CL_INVALID_VALUE);

                }
                break;

            default:
                clmONERROR(clfSetUniformValue(Arg->uniform, length, Arg->data),
                           CL_INVALID_VALUE);
                break;
            }
            break;

        case gcSHADER_INT8_P2:
        case gcSHADER_UINT8_P2:
        case gcSHADER_INT16_P2:
        case gcSHADER_UINT16_P2:
        case gcSHADER_INT8_P3:
        case gcSHADER_UINT8_P3:
        case gcSHADER_INT16_P3:
        case gcSHADER_UINT16_P3:
        case gcSHADER_INT8_P4:
        case gcSHADER_UINT8_P4:
        case gcSHADER_INT16_P4:
        case gcSHADER_UINT16_P4:
        case gcSHADER_INT8_P8:
        case gcSHADER_UINT8_P8:
        case gcSHADER_INT16_P8:
        case gcSHADER_UINT16_P8:
        case gcSHADER_INT8_P16:
        case gcSHADER_UINT8_P16:
        case gcSHADER_INT16_P16:
        case gcSHADER_UINT16_P16:
            clmONERROR(clfSetUniformValue(Arg->uniform, length, Arg->data),
                        CL_INVALID_VALUE);
            break;

        default:
            break;
        }
    }
    else
    {
        gcsUNIFORM_BLOCK uniformBlock = gcvNULL;
        gcUNIFORM bUniform = gcvNULL;
        gctINT16   blockIndex = GetUniformBlockID(Arg->uniform);
        gctUINT32 physicalAddress = 0;
        gctUINT      entries, arraySize;
        gctUINT32    numCols = 0, numRows = 0;
        if ((blockIndex >= 0) && (GetUniformPhysical(Arg->uniform) != -1))
        {
            clmONERROR(gcSHADER_GetUniformBlock(Shader, blockIndex, &uniformBlock), CL_INVALID_VALUE);
            clmONERROR(gcSHADER_GetUniform(Shader, GetUBIndex(uniformBlock), &bUniform), CL_INVALID_VALUE);
            if(isUniformConstantAddressSpace(bUniform))
            {
                gctUINT8_PTR pData = (gctUINT8_PTR)(Kernel->constantMemBuffer) + GetUniformOffset(Arg->uniform);

                clmONERROR(gcSHADER_ComputeUniformPhysicalAddress(hints->hwConstRegBases,
                                                                   Arg->uniform,
                                                                   &physicalAddress), CL_INVALID_VALUE);

                gcTYPE_GetTypeInfo(GetUniformType(Arg->uniform), &numCols, &numRows, gcvNULL);

                entries = clfGetUniformArrayInfo(Arg->uniform, gcvNULL, gcvNULL, &arraySize);
                arraySize *= entries;   /* Expand array size for array of arrays to one dimension */

                clmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress,GetUniformPhysical(Arg->uniform),
                                                  numCols, numRows, arraySize, gcvFALSE,
                                                  (gctSIZE_T)GetUniformMatrixStride(Arg->uniform),
                                                  (gctSIZE_T)GetUniformArrayStride(Arg->uniform),
                                                  pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)), CL_INVALID_VALUE);
            }
        }
        else
        {

            switch (type)
            {
            case gcSHADER_SAMPLER_2D:
            case gcSHADER_SAMPLER_3D:
                if (Arg->needImageSampler)
                {
                    /* Set up image sampler to use texture unit. */
                    clmONERROR(clfSetupImageSampler(Kernel, Arg),
                               CL_INVALID_VALUE);
                }
                break;

            default:
    #if cldTUNING
                {
                    gctCONST_STRING name;
                    gctSTRING special;
                    if (gcmIS_SUCCESS(gcUNIFORM_GetName(Arg->uniform, gcvNULL, &name))
                        && gcmIS_SUCCESS(gcoOS_StrFindReverse(name, '#', &special))
                        )
                    {
                        break;
                    }
                }
    #endif
                clmRETURN_ERROR(CL_INVALID_VALUE);
            }
        }
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfLoadKernelArgLocalMemValues(
    clsKernel_PTR       Kernel,
    gctUINT             NumArgs,
    clsArgument_PTR     Args,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gcSHADER_TYPE       type;
    gctUINT             length;
    clsArgument_PTR     argument;
    gctINT              totalNumGroups;
    gctINT              totalSize = 0;
    gctINT              totalAlignSize = 0;
    gctINT              totalArgSize = 0;
    gctINT              totalArgAlignSize = 0;
    gctUINT             i;
    clsMemAllocInfo_PTR memAllocInfo;
    gctINT *            data;
    gctUINT             allocatedSize = 0;
    gctUINT32           physical = 0; /* gpu virtual address */
    gctPOINTER          logical = gcvNULL;
    gcsSURF_NODE_PTR    node = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    /* Add up sizes for all local kernel args. */
    for (i = 0; i < NumArgs; i++)
    {
        argument = &Args[i];

        if (argument->uniform == gcvNULL) continue;

        if (isUniformInactive(argument->uniform)) continue;

        if (isUniformKernelArgLocal(argument->uniform))
        {
            memAllocInfo = (clsMemAllocInfo_PTR) argument->data;

            clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
            clmASSERT(memAllocInfo->allocatedSize > 0, CL_INVALID_VALUE);

            totalSize += gcmALIGN(memAllocInfo->allocatedSize, 4);
            totalAlignSize = gcmALIGN(totalSize, memAllocInfo->allocatedSize);
            totalSize = totalAlignSize;
        }
    }

    if (totalSize > 0)
    {
        totalNumGroups = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                       * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                       * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1);

        allocatedSize = totalSize * totalNumGroups;

        /* Allocate the physical buffer */
        clmONERROR(gcoCL_AllocateMemory(&allocatedSize, &physical, &logical, &node, gcvSURF_INDEX, 0), CL_INVALID_VALUE);

        /* Set up relative address for all local kernel args. */
        for (i = 0; i < NumArgs; i++)
        {
            argument = &Args[i];

            if (argument->uniform == gcvNULL) continue;

            clmONERROR(gcUNIFORM_GetType(argument->uniform, &type, &length), CL_INVALID_VALUE);

            if (isUniformInactive(argument->uniform)) continue;

            if (isUniformKernelArgLocal(argument->uniform))
            {
                memAllocInfo = (clsMemAllocInfo_PTR) argument->data;

                clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
                clmASSERT(memAllocInfo->allocatedSize > 0, CL_INVALID_VALUE);

                totalArgAlignSize = gcmALIGN(totalArgSize, memAllocInfo->allocatedSize);
                memAllocInfo->physical = physical + totalArgAlignSize;

                if(totalArgSize == 0) memAllocInfo->node = node;
                totalArgSize += gcmALIGN(memAllocInfo->allocatedSize, 4);

                data = (gctINT *) &memAllocInfo->physical;
                clmONERROR(clfSetUniformValue(argument->uniform, length, data), CL_INVALID_VALUE);

                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
            }
            else if (isUniformKernelArgLocalMemSize(argument->uniform))
            {
                memAllocInfo = (clsMemAllocInfo_PTR) argument->data;

                clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

                memAllocInfo->allocatedSize = allocatedSize;

                data = (gctINT *) &totalSize;
                clmONERROR(clfSetUniformValue(argument->uniform, length, data), CL_INVALID_VALUE);
            }
        }
    }

    status = CL_SUCCESS;

OnError:
    if(node && clmIS_ERROR(status))
    {
        gcoCL_FreeMemory(physical, logical, allocatedSize, node, gcvSURF_INDEX);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfAdjustLocalWorkSize(
    clsKernel_PTR       Kernel,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3]
    )
{
    gctINT              status = CL_SUCCESS;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    if (LocalWorkSize[0] == 0 &&
        (WorkDim < 2 || LocalWorkSize[1] == 0) &&
        (WorkDim < 3 || LocalWorkSize[2] == 0))
    {
        gctSIZE_T           preferredWorkGroupSize;
        gctSIZE_T           workGroupSize = 1;
        gctUINT             i;

        /* Find the largest workgroup size which has a common multiple with
         * preferred workgroup size but still less than max work group size
         */
        for (i = 0; i < WorkDim; i++)
        {
            preferredWorkGroupSize = Kernel->preferredWorkGroupSizeMultiple;
            while (preferredWorkGroupSize % 2 == 0)
            {
                if ((GlobalWorkSize[i] % preferredWorkGroupSize == 0) &&
                    (workGroupSize * preferredWorkGroupSize <= Kernel->maxWorkGroupSize))
                {
                    LocalWorkSize[i] = preferredWorkGroupSize;
                    workGroupSize *= LocalWorkSize[i];
                    break;
                }
                preferredWorkGroupSize /= 2;
            }
        }

        if (workGroupSize == 1)
        {

            /* No common multiple found
             * Try adjusting wrt global work size
             */
            for (i=0; i<WorkDim; i++)
            {
                if (workGroupSize * GlobalWorkSize[i] <= Kernel->maxWorkGroupSize)
                {
                    LocalWorkSize[i] = GlobalWorkSize[i];
                    workGroupSize *= LocalWorkSize[i];
                }
            }
        }

        if (workGroupSize == 1)
        {
            for (i=0; i<WorkDim; i++)
            {
                if (((GlobalWorkSize[i] % 381) == 0) && (workGroupSize * 381 <= Kernel->maxWorkGroupSize))
                {
                    LocalWorkSize[i] = 381;
                    workGroupSize *= LocalWorkSize[i];
                }
            }
        }

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcdZONE_CL,
            "Adjusted LocalWorkSize: [%d, %d, %d]\n",
            LocalWorkSize[0], LocalWorkSize[1], LocalWorkSize[2]);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfCalcLocalWorkSize(
    clsKernel_PTR       Kernel,
    gctUINT             WorkDim,
    size_t              InputGlobalWorkOffset[3],
    size_t              InputGlobalWorkSize[3],
    size_t              InputLocalWorkSize[3],
    size_t              OutputGlobalWorkOffset[9],
    size_t              OutputGlobalWorkSize[9],
    size_t              OutputLocalWorkSize[9]
    )
{
    gctINT              status = CL_SUCCESS;
    size_t              i,j;
    size_t              preferredWorkGroupSize;
    size_t              *WidthLeave  = gcvNULL;
    size_t              *HeightLeave = gcvNULL;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);
    switch (WorkDim)
    {
    case 1:
        if ((InputGlobalWorkSize[0] > Kernel->preferredWorkGroupSizeMultiple) && (InputGlobalWorkSize[0] % Kernel->preferredWorkGroupSizeMultiple != 0))
        {
            OutputGlobalWorkOffset[0] = InputGlobalWorkOffset[0];
            OutputGlobalWorkSize[0] = InputGlobalWorkSize[0] - InputGlobalWorkSize[0] % Kernel->preferredWorkGroupSizeMultiple;
            OutputLocalWorkSize[0] = Kernel->preferredWorkGroupSizeMultiple;

            if (OutputGlobalWorkSize[0] < InputGlobalWorkSize[0])
            {
                OutputGlobalWorkOffset[3] = OutputGlobalWorkSize[0];
                OutputGlobalWorkSize[3] = InputGlobalWorkSize[0]-OutputGlobalWorkSize[0];
                OutputLocalWorkSize[3] = InputLocalWorkSize[0]; /* NULL */
            }
        }
        else
        {
            OutputGlobalWorkOffset[0] = InputGlobalWorkOffset[0];
            OutputGlobalWorkSize[0] = InputGlobalWorkSize[0];
            OutputLocalWorkSize[0] = InputLocalWorkSize[0]; /* NULL */
        }
        break;
    case 2:
        {
            gctINT WidthGroupSize=Kernel->preferredWorkGroupSizeMultiple;
            gctINT HeightGroupSize=Kernel->preferredWorkGroupSizeMultiple;
            size_t TotalLeave = 0xCfffffff;
            size_t count=0;
            gctBOOL HeightAlign=gcvFALSE,WidthAlign=gcvFALSE;
            preferredWorkGroupSize = Kernel->preferredWorkGroupSizeMultiple;
            while(preferredWorkGroupSize>=2)
            {
                count++;
                preferredWorkGroupSize=preferredWorkGroupSize>>1;
            }

            preferredWorkGroupSize = Kernel->preferredWorkGroupSizeMultiple;
            /* Assume GPU Core Count is 2^N */
            gcmASSERT(preferredWorkGroupSize == (size_t)(1 << count));

            clmONERROR(gcoOS_Allocate(gcvNULL, count*sizeof(size_t), (gctPOINTER*)&WidthLeave), CL_OUT_OF_HOST_MEMORY);
            clmONERROR(gcoOS_Allocate(gcvNULL, count*sizeof(size_t), (gctPOINTER*)&HeightLeave), CL_OUT_OF_HOST_MEMORY);

            WidthAlign = InputGlobalWorkSize[0]%preferredWorkGroupSize == 0;
            HeightAlign = InputGlobalWorkSize[1]%preferredWorkGroupSize == 0;
            if (WidthLeave != gcvNULL && HeightLeave != gcvNULL)
            {
                for (i = 0; i < count; i++)
                {
                    WidthLeave[i] = InputGlobalWorkSize[0]%(preferredWorkGroupSize>>i);
                    HeightLeave[i] = InputGlobalWorkSize[1]%(preferredWorkGroupSize>>i);
                }
                for (i = 0; i < count; i++)
                {
                    for (j = 0; j < count; j++)
                    {
                        if ((size_t)((preferredWorkGroupSize>>i)*(preferredWorkGroupSize>>j)) % preferredWorkGroupSize == 0)
                        {
                            if (TotalLeave > (WidthLeave[i]*InputGlobalWorkSize[1] + HeightLeave[j]*InputGlobalWorkSize[0]  - WidthLeave[i]*HeightLeave[j]))
                            {
                                /* TODO: can refine to choose logic like P1, Both Align, P2, WidthAlign, P3, HeightAlign, P4, No Align */
                                TotalLeave = WidthLeave[i]*InputGlobalWorkSize[1] + HeightLeave[j]*InputGlobalWorkSize[0]  - WidthLeave[i]*HeightLeave[j];
                                WidthGroupSize = preferredWorkGroupSize>>i;
                                HeightGroupSize = preferredWorkGroupSize>>j;
                                WidthAlign = InputGlobalWorkSize[0]%WidthGroupSize== 0;
                                HeightAlign = InputGlobalWorkSize[1]%HeightGroupSize== 0;
                            }
                        }
                    }
                }
                if (InputGlobalWorkSize[0] < (size_t)WidthGroupSize || InputGlobalWorkSize[1] < (size_t)HeightGroupSize)
                {
                    OutputGlobalWorkOffset[0] = InputGlobalWorkOffset[0];
                    OutputGlobalWorkOffset[1] = InputGlobalWorkOffset[1];
                    OutputGlobalWorkSize[0] = InputGlobalWorkSize[0];
                    OutputGlobalWorkSize[1] = InputGlobalWorkSize[1];
                    OutputLocalWorkSize[0] = InputLocalWorkSize[0]; /* NULL */
                    OutputLocalWorkSize[1] = InputLocalWorkSize[1]; /* NULL */
                }
                else
                {
                    OutputGlobalWorkOffset[0] = InputGlobalWorkOffset[0];
                    OutputGlobalWorkOffset[1] = InputGlobalWorkOffset[1];
                    OutputGlobalWorkSize[0] = InputGlobalWorkSize[0] - InputGlobalWorkSize[0] % WidthGroupSize;
                    OutputGlobalWorkSize[1] = InputGlobalWorkSize[1] - InputGlobalWorkSize[1] % HeightGroupSize;
                    OutputLocalWorkSize[0] = WidthGroupSize;
                    OutputLocalWorkSize[1] = HeightGroupSize;
                    if(WidthAlign && HeightAlign)
                    {
                        OutputLocalWorkSize[0] = InputLocalWorkSize[0]; /* NULL */
                        OutputLocalWorkSize[1] = InputLocalWorkSize[1]; /* NULL */
                    }
                    else if (WidthAlign)
                    {
                        /* Process remainder Height */
                        OutputGlobalWorkOffset[3] = InputGlobalWorkOffset[0];
                        OutputGlobalWorkOffset[4] = OutputGlobalWorkSize[1];
                        OutputGlobalWorkSize[3] = InputGlobalWorkSize[0];
                        OutputGlobalWorkSize[4] = InputGlobalWorkSize[1] - OutputGlobalWorkSize[1];
                        OutputLocalWorkSize[3] = InputLocalWorkSize[0]; /* NULL */
                        OutputLocalWorkSize[4] = InputLocalWorkSize[1]; /* NULL */
                    }
                    else if (HeightAlign)
                    {
                        /* Process remainder Width */
                        OutputGlobalWorkOffset[3] = OutputGlobalWorkSize[0];
                        OutputGlobalWorkOffset[4] = InputGlobalWorkOffset[1];
                        OutputGlobalWorkSize[3] = InputGlobalWorkSize[0]- OutputGlobalWorkSize[0];
                        OutputGlobalWorkSize[4] = InputGlobalWorkSize[1];
                        OutputLocalWorkSize[3] = InputLocalWorkSize[0]; /* NULL */
                        OutputLocalWorkSize[4] = InputLocalWorkSize[1]; /* NULL */
                    }
                    else  /* Not Align in Height and Width */
                    {
                        /* Process remainder Height */
                        OutputGlobalWorkOffset[3] = InputGlobalWorkOffset[0];
                        OutputGlobalWorkOffset[4] = OutputGlobalWorkSize[1];
                        OutputGlobalWorkSize[3] = InputGlobalWorkSize[0];
                        OutputGlobalWorkSize[4] = InputGlobalWorkSize[1] - OutputGlobalWorkSize[1];
                        OutputLocalWorkSize[3] = InputLocalWorkSize[0]; /* NULL */
                        OutputLocalWorkSize[4] = InputLocalWorkSize[1]; /* NULL */

                        /* Process remainder Width not include common part of Height*/
                        OutputGlobalWorkOffset[6] = OutputGlobalWorkSize[0];
                        OutputGlobalWorkOffset[7] = InputGlobalWorkOffset[1];
                        OutputGlobalWorkSize[6] = InputGlobalWorkSize[0] - OutputGlobalWorkSize[0];
                        OutputGlobalWorkSize[7] = OutputGlobalWorkSize[1];
                        OutputLocalWorkSize[6] = InputLocalWorkSize[0]; /* NULL */
                        OutputLocalWorkSize[7] = InputLocalWorkSize[1]; /* NULL */
                    }
                }
            }
            gcoOS_Free(gcvNULL, (gctPOINTER)WidthLeave);
            WidthLeave = gcvNULL;
            gcoOS_Free(gcvNULL, (gctPOINTER)HeightLeave);
            HeightLeave = gcvNULL;
        }
        break;
    case 3:
        gcmASSERT(-1);/* TODO, need support 3D WAR */
        break;
    default:
        break;
    }

OnError:
    if (WidthLeave != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)WidthLeave);
    }
    if (HeightLeave != gcvNULL)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)HeightLeave);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfCalcLocalQuarterWorkSize(
    clsKernel_PTR       Kernel,
    gctUINT             WorkDim,
    size_t              InputGlobalWorkOffset[3],
    size_t              InputGlobalWorkSize[3],
    size_t              InputLocalWorkSize[3],
    size_t              OutputGlobalWorkOffset[12],
    size_t              OutputGlobalWorkSize[12],
    size_t              OutputLocalWorkSize[12]
    )
{
    gctINT              status = CL_SUCCESS;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);
    switch (WorkDim)
    {
    case 2:
        OutputGlobalWorkOffset[0] = InputGlobalWorkOffset[0];
        OutputGlobalWorkOffset[1] = InputGlobalWorkOffset[1];
        OutputGlobalWorkSize[0] = InputGlobalWorkSize[0] / 4;
        OutputGlobalWorkSize[1] = InputGlobalWorkSize[1] / 4;
        OutputLocalWorkSize[0] = InputLocalWorkSize[0];
        OutputLocalWorkSize[1] = InputLocalWorkSize[1];

        OutputGlobalWorkOffset[3] = InputGlobalWorkOffset[0];
        OutputGlobalWorkOffset[4] = OutputGlobalWorkSize[1];
        OutputGlobalWorkSize[3] = InputGlobalWorkSize[0] / 4;
        OutputGlobalWorkSize[4] = InputGlobalWorkSize[1] / 4;
        OutputLocalWorkSize[3] = InputLocalWorkSize[0];
        OutputLocalWorkSize[4] = InputLocalWorkSize[1];

        OutputGlobalWorkOffset[6] = OutputGlobalWorkSize[0];
        OutputGlobalWorkOffset[7] = InputGlobalWorkOffset[1];
        OutputGlobalWorkSize[6] = InputGlobalWorkSize[0] / 4;
        OutputGlobalWorkSize[7] = InputGlobalWorkSize[1] / 4;
        OutputLocalWorkSize[6] = InputLocalWorkSize[0];
        OutputLocalWorkSize[7] = InputLocalWorkSize[1];

        OutputGlobalWorkOffset[9] = OutputGlobalWorkSize[0];
        OutputGlobalWorkOffset[10] = OutputGlobalWorkSize[1];
        OutputGlobalWorkSize[9] = InputGlobalWorkSize[0] / 4;
        OutputGlobalWorkSize[10] = InputGlobalWorkSize[1] / 4;
        OutputLocalWorkSize[9] = InputLocalWorkSize[0];
        OutputLocalWorkSize[10] = InputLocalWorkSize[1];

        break;
    case 1:
    case 3:
        break;
    default:
        break;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gctUINT
clfGetFormat(cl_channel_type type)
{
    gctUINT format=0;
    switch(type)
    {
    case CL_UNORM_INT8:
        format = 0xF;
        break;
    case CL_UNORM_INT16:
        format = 0xE;
        break;
    case CL_SNORM_INT8:
        format = 0xC;
        break;
    case CL_SNORM_INT16:
        format = 0xB;
        break;
    case CL_UNORM_SHORT_565:
        format = 9;
        break;
    case CL_UNORM_SHORT_555:
        format = 8;
        break;
    case CL_UNORM_INT_101010:
        format = 0xA;
        break;
    case CL_SIGNED_INT8:
        format = 4;
        break;
    case CL_SIGNED_INT16:
        format = 3;
        break;
    case CL_SIGNED_INT32:
        format = 2;
        break;
    case CL_UNSIGNED_INT8:
        format = 7;
        break;
    case CL_UNSIGNED_INT16:
        format = 6;
        break;
    case CL_UNSIGNED_INT32:
        format = 5;
        break;
    case CL_HALF_FLOAT:
        format = 1;
        break;
    case CL_FLOAT:
    default:
        format = 0;
        break;
    }
    return format;
}

void
clfGetCompAndSwizzle(cl_channel_order type, gctUINT * componentCount, gctUINT * swizzleR, gctUINT * swizzleG, gctUINT * swizzleB, gctUINT * swizzleA)
{
    switch(type)
    {
    case CL_R:
    case CL_Rx:
        *componentCount = 1;
        *swizzleR = 0;
        *swizzleG = 4;
        *swizzleB = 4;
        *swizzleA = 5;
        break;
    case CL_A:
        *componentCount = 1;
        *swizzleR = 4;
        *swizzleG = 4;
        *swizzleB = 4;
        *swizzleA = 3;
        break;
    case CL_RG:
    case CL_RGx:
        *componentCount = 2;
        *swizzleR = 0;
        *swizzleG = 1;
        *swizzleB = 4;
        *swizzleA = 5;
        break;
    case CL_RA:
        *componentCount = 2;
        *swizzleR = 0;
        *swizzleG = 4;
        *swizzleB = 4;
        *swizzleA = 3;
        break;
    case CL_RGB:
        *componentCount = 3;
        *swizzleR = 0;
        *swizzleG = 1;
        *swizzleB = 2;
        *swizzleA = 5;
        break;
    case CL_RGBA:
        *componentCount = 0;
        *swizzleR = 0;
        *swizzleG = 1;
        *swizzleB = 2;
        *swizzleA = 3;
        break;
    case CL_BGRA:
        *componentCount = 0;
        *swizzleR = 2;
        *swizzleG = 1;
        *swizzleB = 0;
        *swizzleA = 3;
        break;
    case CL_ARGB:
        *componentCount = 0;
        *swizzleR = 1;
        *swizzleG = 2;
        *swizzleB = 3;
        *swizzleA = 0;
        break;
    case CL_INTENSITY:
        *componentCount = 0;
        *swizzleR = 0;
        *swizzleG = 0;
        *swizzleB = 0;
        *swizzleA = 0;
        break;
    case CL_LUMINANCE:
        *componentCount = 1;
        *swizzleR = 0;
        *swizzleG = 0;
        *swizzleB = 0;
        *swizzleA = 5;
        break;
    default:
        *componentCount = 0;
        break;
    }
}

static gceSTATUS
clfFlushKernelUniform(
    clsKernel_PTR       Kernel,
    clsKernelInstance_PTR Instance,
    gctUINT             NumArgs,
    clsArgument_PTR     Args,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3],
    clsPrivateBuffer_PTR *privateBufList
    )
{
    gctUINT    i;
    gceSTATUS  status = gcvSTATUS_OK;

    if (Instance->patchDirective == gcvNULL)
    {
        /* Load argument values. */
        for (i = 0; i < NumArgs; i++)
        {
            if (Args[i].uniform && !isUniformInactive(Args[i].uniform))
            {
                clmONERROR(clfLoadKernelArgValues(Kernel,
                                                  Instance,
                                                  &Args[i],
                                                  WorkDim,
                                                  GlobalWorkOffset,
                                                  GlobalWorkSize,
                                                  LocalWorkSize,
                                                  &Args[0],
                                                  NumArgs,
                                                  privateBufList),
                           CL_INVALID_VALUE);
            }
        }
    }
    else
    {
        clsPatchDirective_PTR   patchDirective             = gcvNULL;
        gctUINT                 patchedWorkDim             = WorkDim;
        size_t                  patchedGlobalWorkOffset[3] = {0};
        size_t                  patchedGlobalWorkSize[3]   = {0};
        size_t                  patchedLocalWorkSize[3]    = {0};
        gctUINT                 i;
        gcUNIFORM               globaSize = gcvNULL, groupSize = gcvNULL;
        gcSHADER                tmpBinary = (gcSHADER)Kernel->recompileInstance->binary;
        gctUINT                 uniformCount = (gctUINT)tmpBinary->uniformCount;
        gctBOOL                 halti5 = clgDefaultDevice->platform->vscCoreSysCtx.hwCfg.hwFeatureFlags.hasHalti5;

        for (i = 0; i < WorkDim; i++)
        {
            patchedGlobalWorkOffset[i] = GlobalWorkOffset[i];
            patchedGlobalWorkSize[i]   = GlobalWorkSize[i];
            patchedLocalWorkSize[i]    = LocalWorkSize[i];
        }

        for (patchDirective = Instance->patchDirective;
             patchDirective;
             patchDirective = patchDirective->next)
        {
            if (patchDirective->kind == gceRK_PATCH_GLOBAL_WORK_SIZE)
            {
                /*clsPatchGlobalWorkSize * patchGlobalWorkSize = patchDirective->patchValue.globalWorkSize;*/
                clsPatchGlobalWorkSize * globalWorkSize = patchDirective->patchValue.globalWorkSize;

                patchedWorkDim             = 1;

                patchedGlobalWorkOffset[0] = GlobalWorkOffset[0] +  GlobalWorkOffset[1] * GlobalWorkSize[0];
                patchedGlobalWorkSize[0]   = globalWorkSize->realGlobalWorkSize;/*GlobalWorkSize[0] * GlobalWorkSize[1];*/
                patchedLocalWorkSize[0]    = LocalWorkSize[0] * LocalWorkSize[1];

                if (globalWorkSize->globalWidth &&
                    !isUniformInactive(globalWorkSize->globalWidth))
                {
                    gctINT  uniGlobalWidth[2] = {GlobalWorkSize[0], globalWorkSize->realGlobalWorkSize};
                    globaSize = globalWorkSize->globalWidth;

                    clmONERROR(clfSetUniformValue(globalWorkSize->globalWidth,
                                                  1,
                                                  uniGlobalWidth),
                               CL_INVALID_VALUE);
                }

                if (globalWorkSize->groupWidth &&
                    !isUniformInactive(globalWorkSize->groupWidth))
                {
                    gctINT  uniGroupWidth  = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1);
                    groupSize = globalWorkSize->groupWidth;
                    clmONERROR(clfSetUniformValue(globalWorkSize->groupWidth,
                                                  1,
                                                  &uniGroupWidth),
                               CL_INVALID_VALUE);
                }
            }
        }

        /* Load argument values. */
        for (i = 0; i < NumArgs; i++)
        {
            if (Args[i].uniform && !isUniformInactive(Args[i].uniform) &&
                globaSize != Args[i].uniform && groupSize != Args[i].uniform)
            {
                clmONERROR(clfLoadKernelArgValues(Kernel,
                                                  Instance,
                                                  &Args[i],
                                                  patchedWorkDim,
                                                  patchedGlobalWorkOffset,
                                                  patchedGlobalWorkSize,
                                                  patchedLocalWorkSize,
                                                  &Args[0],
                                                  NumArgs,
                                                  privateBufList),
                           CL_INVALID_VALUE);
            }
        }

        if(halti5 && NEW_READ_WRITE_IMAGE && NumArgs < uniformCount)
        {
            gctUINT j;

            for(j = NumArgs; j < uniformCount; j++)
            {
                gcUNIFORM tmpUniform = tmpBinary->uniforms[NumArgs];
                if(tmpUniform->u.type == gcSHADER_VIV_GENERIC_GL_IMAGE && tmpUniform->_varCategory == gcSHADER_VAR_CATEGORY_GL_IMAGE_FOR_IMAGE_T)
                {
                    gctINT16             imageIndex = tmpUniform->parent, samplerIndex = tmpUniform->prevSibling;
                    gcUNIFORM            parentImage = tmpBinary->uniforms[imageIndex], parentSampler = gcvNULL;
                    clsMem_PTR           memObj = gcvNULL;
                    gcsIMAGE_SAMPLER_PTR imageSampler = gcvNULL;
                    gctUINT32            samplerType = 0xffffffff;
                    gcKERNEL_FUNCTION    kernelFunction = gcvNULL;
                    gctINT *             data;
                    gctUINT32            tmpData[8] = {0};
                    gctUINT              addressMode = 0, format, tiling, type=0, componentCount=0, swizzleR=0, swizzleG=0, swizzleB=0, swizzleA=0;
                    gctUINT              sliceSize = 0, depth = 0, dataLength = 0;
                    clsArgument          samplerArg = {0};
                    gctUINT              shift = 0;

                    for(i = 0; i < NumArgs; i++)
                    {
                        if(Args[i].uniform == parentImage)
                        {
                            memObj = *(clsMem_PTR *) Args[i].data;
                            shift = (unsigned int)(gcoMATH_Log((gctFLOAT)memObj->u.image.elementSize)/gcoMATH_Log(2.0));
                            break;
                        }
                    }
                    gcmASSERT(memObj);

                    /* Get main kernel function. */
                    gcmASSERT(tmpBinary->kernelFunctions);

                    for (i = 0; i < tmpBinary->kernelFunctionCount; i++)
                    {
                        if (tmpBinary->kernelFunctions[i] == gcvNULL) continue;

                        if (tmpBinary->kernelFunctions[i]->isMain)
                        {
                            kernelFunction = tmpBinary->kernelFunctions[i];
                            break;
                        }
                    }

                    gcmASSERT(kernelFunction);

                    if(samplerIndex == -1)
                    {
                        samplerType = tmpUniform->followingOffset;
                    }
                    else
                    {
                        parentSampler = tmpBinary->uniforms[samplerIndex];
                        imageSampler = &kernelFunction->imageSamplers[parentSampler->imageSamplerIndex];
                        for(i = 0; i < NumArgs; i++)
                        {
                            if(Args[i].uniform == parentSampler)
                            {
                                samplerArg = Args[i];
                                break;
                            }
                        }
                    }

                    tmpData[0] = (gctUINT32 ) memObj->u.image.texturePhysical;
                    tmpData[1] = memObj->u.image.textureStride;

                    switch(memObj->type)
                    {
                    case CL_MEM_OBJECT_IMAGE1D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width;
                        depth = 1;
                        type = 0;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width * memObj->u.image.imageDesc.image_height;
                        depth = 1;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE3D:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.textureSlicePitch;
                        depth = memObj->u.image.imageDesc.image_height;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_array_size<<16);
                        sliceSize = memObj->u.image.imageDesc.image_width;
                        depth = memObj->u.image.imageDesc.image_array_size;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                        tmpData[2] = (memObj->u.image.imageDesc.image_width) | (memObj->u.image.imageDesc.image_height<<16);
                        sliceSize = memObj->u.image.textureSlicePitch;
                        depth = memObj->u.image.imageDesc.image_array_size;
                        type = 1;
                        break;
                    default:
                        break;
                    }
                    format = clfGetFormat(memObj->u.image.imageFormat.image_channel_data_type);
                    tiling = (memObj->u.image.tiling == gcvLINEAR ? 0 : memObj->u.image.tiling == gcvTILED ? 1 : memObj->u.image.tiling == gcvSUPERTILED ? 2 : 3);
                    clfGetCompAndSwizzle(memObj->u.image.imageFormat.image_channel_order, &componentCount, &swizzleR, &swizzleG, &swizzleB, &swizzleA);

                    {
                        if(samplerIndex != -1 && imageSampler->isConstantSamplerType == gcvFALSE)
                        {
                            if(parentSampler != gcvNULL && isUniformKernelArgSampler(parentSampler))
                            {
                                cleSAMPLER sampler = *(cleSAMPLER *) samplerArg.data;
                                if((sampler & CLK_ADDRESS_CLAMP) == CLK_ADDRESS_CLAMP)
                                {
                                    switch(memObj->u.image.imageFormat.image_channel_order)
                                    {
                                    case CL_A:
                                    case CL_INTENSITY:
                                    case CL_Rx:
                                    case CL_RA:
                                    case CL_RGx:
                                    default:
                                        addressMode = 1;
                                        break;
                                    case CL_R:
                                    case CL_RG:
                                    case CL_RGB:
                                    case CL_LUMINANCE:
                                        addressMode = 2;
                                        break;
                                    }
                                }
                                else if((sampler & CLK_ADDRESS_CLAMP_TO_EDGE) == CLK_ADDRESS_CLAMP_TO_EDGE)
                                {
                                    addressMode = 3;
                                }
                                else
                                {
                                    addressMode = 0;
                                }
                            }
                        }
                        else
                        {
                            if((samplerType & 0xF) == CLK_ADDRESS_CLAMP)
                            {
                                switch(memObj->u.image.imageFormat.image_channel_order)
                                {
                                    case CL_A:
                                    case CL_INTENSITY:
                                    case CL_Rx:
                                    case CL_RA:
                                    case CL_RGx:
                                    default:
                                        addressMode = 1;
                                        break;
                                    case CL_R:
                                    case CL_RG:
                                    case CL_RGB:
                                    case CL_LUMINANCE:
                                        addressMode = 2;
                                        break;
                                }
                            }
                            else if((samplerType & 0xF) == CLK_ADDRESS_CLAMP_TO_EDGE)
                            {
                                addressMode = 3;
                            }
                        }
                    }

                    tmpData[3] =  shift | (addressMode<<4) | (format<<6) | (tiling<<10) | (type<<12)
                        | (componentCount<<14) | (swizzleR<<16) | (swizzleG<<20) | (swizzleB<<24) | (swizzleA<<28);
                                    /*gcmSETFIELD(0, GCREG_SH_IMAGE, SHIFT, shift)
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 3:3) - (0 ?
 3:3) + 1))))))) << (0 ?
 3:3))) | (((gctUINT32) (0x0 & ((gctUINT32) ((((1 ?
 3:3) - (0 ?
 3:3) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 3:3) - (0 ? 3:3) + 1))))))) << (0 ? 3:3)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:4) - (0 ?
 5:4) + 1))))))) << (0 ?
 5:4))) | (((gctUINT32) ((gctUINT32) (addressMode) & ((gctUINT32) ((((1 ?
 5:4) - (0 ?
 5:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:4) - (0 ? 5:4) + 1))))))) << (0 ? 5:4)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:6) - (0 ?
 9:6) + 1))))))) << (0 ?
 9:6))) | (((gctUINT32) ((gctUINT32) (format) & ((gctUINT32) ((((1 ?
 9:6) - (0 ?
 9:6) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:6) - (0 ? 9:6) + 1))))))) << (0 ? 9:6)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 11:10) - (0 ?
 11:10) + 1))))))) << (0 ?
 11:10))) | (((gctUINT32) ((gctUINT32) (tiling) & ((gctUINT32) ((((1 ?
 11:10) - (0 ?
 11:10) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 11:10) - (0 ? 11:10) + 1))))))) << (0 ? 11:10)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 12:12) - (0 ?
 12:12) + 1))))))) << (0 ?
 12:12))) | (((gctUINT32) ((gctUINT32) (type) & ((gctUINT32) ((((1 ?
 12:12) - (0 ?
 12:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 12:12) - (0 ? 12:12) + 1))))))) << (0 ? 12:12)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 15:14) - (0 ?
 15:14) + 1))))))) << (0 ?
 15:14))) | (((gctUINT32) ((gctUINT32) (componentCount) & ((gctUINT32) ((((1 ?
 15:14) - (0 ?
 15:14) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 15:14) - (0 ? 15:14) + 1))))))) << (0 ? 15:14)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 18:16) - (0 ?
 18:16) + 1))))))) << (0 ?
 18:16))) | (((gctUINT32) ((gctUINT32) (swizzleR) & ((gctUINT32) ((((1 ?
 18:16) - (0 ?
 18:16) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 18:16) - (0 ? 18:16) + 1))))))) << (0 ? 18:16)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 22:20) - (0 ?
 22:20) + 1))))))) << (0 ?
 22:20))) | (((gctUINT32) ((gctUINT32) (swizzleG) & ((gctUINT32) ((((1 ?
 22:20) - (0 ?
 22:20) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 22:20) - (0 ? 22:20) + 1))))))) << (0 ? 22:20)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 26:24) - (0 ?
 26:24) + 1))))))) << (0 ?
 26:24))) | (((gctUINT32) ((gctUINT32) (swizzleB) & ((gctUINT32) ((((1 ?
 26:24) - (0 ?
 26:24) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 26:24) - (0 ? 26:24) + 1))))))) << (0 ? 26:24)))
                                    | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 30:28) - (0 ?
 30:28) + 1))))))) << (0 ?
 30:28))) | (((gctUINT32) ((gctUINT32) (swizzleA) & ((gctUINT32) ((((1 ?
 30:28) - (0 ?
 30:28) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 30:28) - (0 ? 30:28) + 1))))))) << (0 ? 30:28)));*/

                    tmpData[4] = sliceSize;
                    tmpData[5] = depth;
                    tmpData[6] = ((gctUINT)(memObj->u.image.imageFormat.image_channel_data_type << 16) | (gctUINT)(memObj->u.image.imageFormat.image_channel_order));
                    data = (gctINT *)tmpData;

                    if(tmpUniform->arraySize == 2)
                    {
                        dataLength = 8;
                    }
                    else
                    {
                        dataLength = 4;
                    }

                    clmONERROR(clfSetUniformValue(tmpUniform, dataLength, data),
                       CL_INVALID_VALUE);
                }
            }

        }
    }

OnError:
    return status;
}

static void
clfUpdateImageDescriptor(clsCommandNDRangeVIRKernel_PTR NDRangeVIRKernel,
                               PROG_CL_IMAGE_TABLE_ENTRY * ImageEntry,
                               VSC_ImageDesc * Descriptor)
{
    gctUINT samplerValue;
    clsSrcArgument_PTR imageArg;
    clsSrcArgument_PTR samplerArg;
    gctUINT addressMode = 0;
    clsMem_PTR image;

    if (ImageEntry->kernelHardcodeSampler)
    {
        samplerValue = ImageEntry->constSamplerValue;
    }
    else
    {
        clsSampler_PTR sampler;

        samplerArg = &(NDRangeVIRKernel->args[ImageEntry->samplerArgIndex]);
        sampler = *(clsSampler_PTR *)samplerArg->data;
        samplerValue = sampler->samplerValue;
    }

    imageArg = &(NDRangeVIRKernel->args[ImageEntry->imageArgIndex]);
    image = *(clsMem_PTR *)imageArg->data;

    *Descriptor = image->u.image.imageDescriptor;

    if((NDRangeVIRKernel->kernel->program->buildOptions != gcvNULL) &&
       (gcvSTATUS_TRUE == gcoOS_StrStr(NDRangeVIRKernel->kernel->program->buildOptions, "-cl-viv-vx-extension", gcvNULL)))
   {
        addressMode = 1;
   }
   else
   {
       if((samplerValue & 0xF) == CLK_ADDRESS_CLAMP)
       {
           switch(image->u.image.imageFormat.image_channel_order)
           {
               case CL_A:
               case CL_INTENSITY:
               case CL_Rx:
               case CL_RA:
               case CL_RGx:
               default:
                   addressMode = 1;
                   break;
               case CL_R:
               case CL_RG:
               case CL_RGB:
               case CL_LUMINANCE:
                   addressMode = 2;
                   break;
           }
       }
       else if((samplerValue & 0xF) == CLK_ADDRESS_CLAMP_TO_EDGE)
       {
           addressMode = 3;
       }
   }

   Descriptor->sd.addressing = addressMode;
}

static gceSTATUS
clfFlushVIRKernelResource(
    clsCommandNDRangeVIRKernel_PTR NDRangeKernel
    )
{
    gceSTATUS  status = gcvSTATUS_OK;
    gctUINT i;
    KERNEL_EXECUTABLE_PROFILE * kep;
    clsSrcArgument_PTR arg;
    struct _gcsHINT *hints;
    gctPOINTER data;

    kep = &NDRangeKernel->currentInstance->kep;
    hints = &NDRangeKernel->currentInstance->hwStates.hints;

    if (kep)
    {
        PROG_CL_IMAGE_TABLE_ENTRY   * imageEntry;
        PROG_CL_UNIFORM_TABLE_ENTRY * uniformEntry;
        SHADER_EXECUTABLE_PROFILE   * sep = &kep->sep;
        gctUINT                     totalNumItems = NDRangeKernel->globalWorkSize[0]
                                                    * (NDRangeKernel->workDim > 1 ? NDRangeKernel->globalWorkSize[1] : 1)
                                                    * (NDRangeKernel->workDim > 2 ? NDRangeKernel->globalWorkSize[2] : 1);
        gctUINT                     totalNumGroups = NDRangeKernel->globalWorkSize[0] / (NDRangeKernel->localWorkSize[0] ? NDRangeKernel->localWorkSize[0] : 1)
                                                    * (NDRangeKernel->workDim > 1 ? (NDRangeKernel->globalWorkSize[1] / (NDRangeKernel->localWorkSize[1] ? NDRangeKernel->localWorkSize[1] : 1)) : 1)
                                                    * (NDRangeKernel->workDim > 2 ? (NDRangeKernel->globalWorkSize[2] / (NDRangeKernel->localWorkSize[2] ? NDRangeKernel->localWorkSize[2] : 1)) : 1);
        gctUINT                     localKernelArgSize = 0;
        gctUINT32                   physical = 0; /* gpu virtual address */
        gctPOINTER                  logical = gcvNULL;
        gcsSURF_NODE_PTR            node = gcvNULL;
        gctINT                      totalArgSize = 0;
        gctINT                      totalArgAlignSize = 0;

        /* collect the size of local kernel args */
        for (i = 0; i < kep->resourceTable.uniformTable.countOfEntries; i++)
        {
            uniformEntry = &kep->resourceTable.uniformTable.pUniformEntries[i];

            /* find the argIndex */
            if (uniformEntry->argIndex != VIR_CL_INVALID_ARG_INDEX)
            {
                if(uniformEntry->argIndex >= NDRangeKernel->numArgs)
                {
                    gcmONERROR(gcvSTATUS_INVALID_INDEX);
                }

                arg = &NDRangeKernel->args[uniformEntry->argIndex];
                if(arg->addressQualifier == CL_KERNEL_ARG_ADDRESS_LOCAL)
                {
                    clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR)arg->data;
                    localKernelArgSize += gcmALIGN(memAllocInfo->allocatedSize, 4);
                    localKernelArgSize = gcmALIGN(localKernelArgSize, memAllocInfo->allocatedSize);
                }
            }
        }

        /* Allocate the physical buffer */
        if(localKernelArgSize > 0)
        {
            NDRangeKernel->localKernelArgSize = localKernelArgSize * totalNumGroups;
            clmONERROR(gcoCL_AllocateMemory(&NDRangeKernel->localKernelArgSize, &physical, &logical, &node, gcvSURF_INDEX, 0), CL_INVALID_VALUE);
        }

        for (i = 0; i < kep->resourceTable.imageTable.countOfEntries; i++)
        {
            imageEntry = &kep->resourceTable.imageTable.pImageEntries[i];

            /* find the argIndex */
            if (imageEntry->imageArgIndex != VIR_CL_INVALID_ARG_INDEX)
            {
                if(imageEntry->imageArgIndex >= NDRangeKernel->numArgs)
                {
                    gcmONERROR(gcvSTATUS_INVALID_INDEX);
                }

                arg = &NDRangeKernel->args[imageEntry->imageArgIndex];

                /* TO_DO, check args and imageEntry type/size, etc */

                /* program uniform */
                if (arg->data)
                {
                    gctSIZE_T columns;
                    gctSIZE_T Rows;
                    VSC_ImageDesc imageDescriptor;
                    gctUINT hwConstRegNo;
                    gctUINT hwConstRegAddr;
                    clsMem_PTR imgObj = *(clsMem_PTR *) arg->data;
                    gctUINT j;

                    clfUpdateImageDescriptor(NDRangeKernel, imageEntry, &imageDescriptor);

                    gcmASSERT(imageEntry->hwMapping->subArrayRange == 2);

                    Rows = imageEntry->hwMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                    columns = 4;

                    for (j = 0; j < imageEntry->hwMapping->subArrayRange;j++)
                    {
                        hwConstRegNo = imageEntry->hwMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo + j * Rows;
                        hwConstRegAddr = (hints->hwConstRegBases[gcvPROGRAM_STAGE_OPENCL]) + (hwConstRegNo * 16)
                                         + imageEntry->hwMapping->hwFirstConstantLocation.firstValidHwChannel * 4;

                        gcmONERROR(gcoSHADER_BindUniform(gcvNULL,hwConstRegAddr, hwConstRegNo, columns, Rows,
                            1, gcvFALSE, columns * 4, 0, (gctPOINTER)(&imageDescriptor.rawbits[4* j]),gcvUNIFORMCVT_NONE,gcSHADER_TYPE_CL));
                    }

                    /* fence record at image header, is this for read or write? TO_DO, we will not have header*/
                    gcoCL_MemWaitAndGetFence(imgObj->u.image.headerNode, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
                }
            }
        }

        for (i = 0; i < kep->resourceTable.uniformTable.countOfEntries; i++)
        {
            uniformEntry = &kep->resourceTable.uniformTable.pUniformEntries[i];

            /* find the argIndex */
            if (uniformEntry->argIndex != VIR_CL_INVALID_ARG_INDEX)
            {
                gctSIZE_T Columns = 1;
                gctSIZE_T Rows =uniformEntry->common.hwMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
                gctPOINTER tmpPointer= gcvNULL;

                if(uniformEntry->argIndex >= NDRangeKernel->numArgs)
                {
                    gcmONERROR(gcvSTATUS_INVALID_INDEX);
                }

                arg = &NDRangeKernel->args[uniformEntry->argIndex];
                if((arg->addressQualifier == CL_KERNEL_ARG_ADDRESS_LOCAL) && (NDRangeKernel->localKernelArgSize > 0))
                {
                    clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR)arg->data;
                    clmASSERT(memAllocInfo->allocatedSize > 0, CL_INVALID_VALUE);
                    totalArgAlignSize = gcmALIGN(totalArgSize, memAllocInfo->allocatedSize);
                    memAllocInfo->physical = physical + totalArgAlignSize;
                    if(totalArgSize == 0) memAllocInfo->node = node;
                    totalArgSize += gcmALIGN(memAllocInfo->allocatedSize, 4);
                    data = (gctINT *) &memAllocInfo->physical;
                    gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
                }
                else if((arg->addressQualifier == CL_KERNEL_ARG_ADDRESS_PRIVATE) && arg->isMemAlloc)
                {
                    clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) arg->data;
                    gctINT i, privateItemsNum = 0;
                    gctPOINTER pointer;

                    if (hints->workThreadCount != 0 &&
                    (gctINT)hints->workThreadCount < totalNumItems)
                    {
                         privateItemsNum = hints->workThreadCount;
                    }
                    else
                    {
                        privateItemsNum = totalNumItems;
                    }

                    memAllocInfo->allocatedSize *= privateItemsNum;

                    /* Allocate the physical buffer */
                    gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                                    &memAllocInfo->physical,
                                                    &memAllocInfo->logical,
                                                    &memAllocInfo->node,
                                                    gcvSURF_INDEX,
                                                    0);

                    data = (gctPOINTER) &memAllocInfo->physical;

                    gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

                    /* Copy the private data to the buffer */
                    for (i = 0; i < privateItemsNum; i++)
                    {
                        pointer = (gctPOINTER)((gctUINTPTR_T)memAllocInfo->logical + i*arg->size);
                        gcoOS_MemCopy(pointer, memAllocInfo->data, arg->size);
                    }
                }
                else
                {
                    if (arg->isPointer)
                    {
                        clsMem_PTR buffer = *(clsMem_PTR *)arg->data;

                        /* Handle NULL buffer object */
                        if (!buffer)
                        {
                            static int zero = 0;
                            data = &zero;
                        }
                        else
                        {
                            data = (gctPOINTER)&(buffer->u.buffer.physical);
                            /* Is this buffer for read or write?  Get render fence, and wait from render->blt engine result */
                            gcoCL_MemWaitAndGetFence(buffer->u.buffer.node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
                        }
                    }
                    else
                    {
                        /* Unpack argument data */
                        gctUINT32 signMask = 0, signExt = 0;
                        gctSIZE_T i, elemSize = 0, bytes;
                        gctBOOL packedData = gcvFALSE;
                        gctBOOL reConstructData = gcvFALSE;

                        switch(arg->type)
                        {
                        case VSC_SHADER_DATA_TYPE_INT8:
                        case VSC_SHADER_DATA_TYPE_INT8_X2:
                        case VSC_SHADER_DATA_TYPE_INT8_X3:
                        case VSC_SHADER_DATA_TYPE_INT8_X4:
                        case VSC_SHADER_DATA_TYPE_INT8_X8:
                        case VSC_SHADER_DATA_TYPE_INT8_X16:
                            elemSize = 1;
                            signMask = 0x80;
                            signExt = 0xFFFFFF00;
                            packedData = gcvTRUE;
                            break;
                        case VSC_SHADER_DATA_TYPE_UINT8:
                        case VSC_SHADER_DATA_TYPE_UINT8_X2:
                        case VSC_SHADER_DATA_TYPE_UINT8_X3:
                        case VSC_SHADER_DATA_TYPE_UINT8_X4:
                        case VSC_SHADER_DATA_TYPE_UINT8_X8:
                        case VSC_SHADER_DATA_TYPE_UINT8_X16:
                            elemSize = 1;
                            signMask = 0;
                            signExt = 0;
                            packedData = gcvTRUE;
                            break;
                        case VSC_SHADER_DATA_TYPE_INT16:
                        case VSC_SHADER_DATA_TYPE_INT16_X2:
                        case VSC_SHADER_DATA_TYPE_INT16_X3:
                        case VSC_SHADER_DATA_TYPE_INT16_X4:
                        case VSC_SHADER_DATA_TYPE_INT16_X8:
                        case VSC_SHADER_DATA_TYPE_INT16_X16:
                            elemSize = 2;
                            signMask = 0x8000;
                            signExt = 0xFFFF0000;
                            packedData = gcvTRUE;
                            break;
                        case VSC_SHADER_DATA_TYPE_UINT16:
                        case VSC_SHADER_DATA_TYPE_UINT16_X2:
                        case VSC_SHADER_DATA_TYPE_UINT16_X3:
                        case VSC_SHADER_DATA_TYPE_UINT16_X4:
                        case VSC_SHADER_DATA_TYPE_UINT16_X8:
                        case VSC_SHADER_DATA_TYPE_UINT16_X16:
                            elemSize = 2;
                            signMask = 0;
                            signExt = 0;
                            packedData = gcvTRUE;
                            break;
                        case VSC_SHADER_DATA_TYPE_FLOAT_X2:
                        case VSC_SHADER_DATA_TYPE_INTEGER_X2:
                        case VSC_SHADER_DATA_TYPE_UINT_X2:
                            Columns = 2;
                            break;
                        case VSC_SHADER_DATA_TYPE_INT64_X2:
                        case VSC_SHADER_DATA_TYPE_UINT64_X2:
                        case VSC_SHADER_DATA_TYPE_INT64_X3:
                        case VSC_SHADER_DATA_TYPE_UINT64_X3:
                        case VSC_SHADER_DATA_TYPE_INT64_X4:
                        case VSC_SHADER_DATA_TYPE_UINT64_X4:
                        case VSC_SHADER_DATA_TYPE_INT64_X8:
                        case VSC_SHADER_DATA_TYPE_UINT64_X8:
                        case VSC_SHADER_DATA_TYPE_INT64_X16:
                        case VSC_SHADER_DATA_TYPE_UINT64_X16:
                            elemSize = 8;
                            reConstructData = gcvTRUE;
                            break;
                        case VSC_SHADER_DATA_TYPE_FLOAT_X3:
                        case VSC_SHADER_DATA_TYPE_INTEGER_X3:
                        case VSC_SHADER_DATA_TYPE_UINT_X3:
                            Columns = 3;
                            break;
                        case VSC_SHADER_DATA_TYPE_FLOAT_X4:
                        case VSC_SHADER_DATA_TYPE_INTEGER_X4:
                        case VSC_SHADER_DATA_TYPE_UINT_X4:
                        case VSC_SHADER_DATA_TYPE_FLOAT_X8:
                        case VSC_SHADER_DATA_TYPE_INTEGER_X8:
                        case VSC_SHADER_DATA_TYPE_UINT_X8:
                        case VSC_SHADER_DATA_TYPE_FLOAT_X16:
                        case VSC_SHADER_DATA_TYPE_INTEGER_X16:
                        case VSC_SHADER_DATA_TYPE_UINT_X16:
                            Columns = 4;
                            break;
                        }

                        if(packedData)
                        {
                            gctUINT32 *tmpData= gcvNULL;
                            Columns = arg->size / elemSize;
                            bytes = Columns * 4;

                            gcoOS_Allocate(gcvNULL, bytes, &tmpPointer);
                            gcoOS_ZeroMemory(tmpPointer, bytes);
                            tmpData = (gctUINT32 *) tmpPointer;
                            for (i=0; i<Columns; i++)
                            {
                                tmpPointer = (gctPOINTER)(((gctUINTPTR_T)arg->data)+(i*elemSize));
                                gcoOS_MemCopy(&tmpData[i], tmpPointer, elemSize);
                                if (tmpData[i]&signMask)
                                {
                                    tmpData[i] |= signExt;
                                }
                            }

                            /* as HW reg are 128 byte, limit the colums to 4 */
                            if(Columns > 4)
                            {
                                Columns = 4;
                            }

                            tmpPointer = tmpData;
                            data = tmpPointer;
                        }
                        else if(reConstructData)
                        {
                            gctUINT32 *tmpData= gcvNULL;
                            gctUINT32 j = 0, k;
                            gctUINT32 shift  = 1;

                            Columns = arg->size / elemSize;

                            gcoOS_Allocate(gcvNULL, arg->size, &tmpPointer);
                            gcoOS_ZeroMemory(tmpPointer, arg->size);
                            tmpData = (gctUINT32 *) tmpPointer;

                            /* as HW reg are 128 byte, limit the colums to 4 */
                            if(Columns > 4)
                            {
                                shift = Columns/4;
                                Columns = 4;
                            }

                            for(k = 0; k < shift; k++)
                            {
                                for (i=0, j=0; j<Columns; j++)
                                {
                                    tmpData[8*k+j]   = ((gctUINT32 *)arg->data)[8*k+i];
                                    tmpData[8*k+j+Columns] = ((gctUINT32 *)arg->data)[8*k+i+1];
                                    i += 2;
                                }
                            }

                            tmpPointer = tmpData;
                            data = tmpPointer;
                        }
                        else
                        {
                            data = (gctPOINTER)arg->data;
                        }
                    }
                }

                /* Program */
                if (data)
                {
                    gctUINT j = 0;

                    for (j = 0; j < uniformEntry->common.hwMapping->subArrayRange; j++)
                    {
                        gctUINT hwConstRegNo = uniformEntry->common.hwMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo + j * Rows;
                        gctUINT hwConstRegAddr = (hints->hwConstRegBases[gcvPROGRAM_STAGE_OPENCL]) + (hwConstRegNo * 16)
                                               + uniformEntry->common.hwMapping->hwFirstConstantLocation.firstValidHwChannel * 4;

                        gcmONERROR(gcoSHADER_BindUniform(gcvNULL,hwConstRegAddr, hwConstRegNo, Columns, Rows,
                                                        1, gcvFALSE, Columns * 4, 0, &(((gctUINT32 *)data)[Columns*j]),gcvUNIFORMCVT_NONE,gcSHADER_TYPE_CL));
                    }
                }

                if(tmpPointer)
                {
                    gcoOS_Free(gcvNULL, tmpPointer);
                    tmpPointer = gcvNULL;
                }
            }
        }

        for(i = 0; i < sep->staticPrivMapping.privConstantMapping.countOfEntries; i++)
        {
            SHADER_PRIV_CONSTANT_ENTRY * privEntry;
            /* scalar data */
            gctSIZE_T Columns = 1;
            gctSIZE_T Rows;
            gctUINT hwConstRegNo;
            gctUINT hwConstRegAddr;
            gctPOINTER data = gcvNULL;
            gctPOINTER pointer = gcvNULL;
            gctSIZE_T bytes = sizeof(clsMemAllocInfo);
            gctUINT32 workThreadCount, workGroupCount;
            gctUINT32 numGroups[3], globalWorkSize[3], localWorkSize[3], globalOffset[3];
            clsMemAllocInfo_PTR memAllocInfo;
            gctBOOL   isCombinedMode = gcvFALSE;

            privEntry = &sep->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i];
            Rows = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegRange;
            hwConstRegNo = privEntry->u.pSubCBMapping->hwFirstConstantLocation.hwLoc.constReg.hwRegNo;
            hwConstRegAddr = (hints->hwConstRegBases[gcvPROGRAM_STAGE_OPENCL]) + (hwConstRegNo * 16)
                                + privEntry->u.pSubCBMapping->hwFirstConstantLocation.firstValidHwChannel * 4;

            switch(privEntry->commonPrivm.privmKind)
            {
            case SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM:
                numGroups[0] = NDRangeKernel->globalWorkSize[0] / (NDRangeKernel->localWorkSize[0] ? NDRangeKernel->localWorkSize[0] : 1);
                numGroups[1] = NDRangeKernel->globalWorkSize[1] / (NDRangeKernel->localWorkSize[1] ? NDRangeKernel->localWorkSize[1] : 1);
                numGroups[2] = NDRangeKernel->globalWorkSize[2] / (NDRangeKernel->localWorkSize[2] ? NDRangeKernel->localWorkSize[2] : 1);
                data = (gctPOINTER)numGroups;
                Columns = 3;
                break;
            case SHS_PRIV_CONSTANT_KIND_COMPUTE_GROUP_NUM_FOR_SINGLE_GPU:
                {
                    gctUINT32 gpuCount,usedGpu;
                    gctUINT i = 0, j = 0, maxDimIndex = 0;
                    gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0};
                    gctUINT eachGPUWorkGroupNum[gcdMAX_3DGPU_COUNT][3] = {{ 0 }};
                    gctUINT eachGPUGroupCount, restGroupCount;
                    gctINT *datas[gcdMAX_3DGPU_COUNT] = {gcvNULL};
                    gctUINT maxWorkGroupCount = NDRangeKernel->globalWorkSize[0] / NDRangeKernel->localWorkSize[0];
                    gctBOOL atomic_access = (hints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL] & gceMA_FLAG_ATOMIC) != 0;

                    gcoCL_GetHWConfigGpuCount(&gpuCount);

                    numGroups[0] = NDRangeKernel->globalWorkSize[0] / (NDRangeKernel->localWorkSize[0] ? NDRangeKernel->localWorkSize[0] : 1);
                    numGroups[1] = NDRangeKernel->globalWorkSize[1] / (NDRangeKernel->localWorkSize[1] ? NDRangeKernel->localWorkSize[1] : 1);
                    numGroups[2] = NDRangeKernel->globalWorkSize[2] / (NDRangeKernel->localWorkSize[2] ? NDRangeKernel->localWorkSize[2] : 1);

                    if (gpuCount > 1 && !atomic_access)
                    {
                        usedGpu = gpuCount;
                        isCombinedMode = gcvTRUE;

                        for(i = 1; i < NDRangeKernel->workDim ; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
                        {
                            if(NDRangeKernel->globalWorkSize[i] / NDRangeKernel->localWorkSize[i] > maxWorkGroupCount)
                            {
                                maxWorkGroupCount = NDRangeKernel->globalWorkSize[i] / NDRangeKernel->localWorkSize[i];
                                maxDimIndex = i;
                            }
                        }

                        eachGPUGroupCount = maxWorkGroupCount / gpuCount;
                        restGroupCount = maxWorkGroupCount % gpuCount;

                        for(i = 0 ;i < gpuCount; i++)
                        {
                            eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
                        }

                        for(i = 0 ;i <restGroupCount; i++)
                        {
                            eachGPUWorkGroupSizes[i]++;
                        }
                        if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

                        for (i = 0; i < NDRangeKernel->workDim; i++)
                        {
                            if (i == maxDimIndex)
                            {
                                for (j = 0; j < usedGpu; j++)
                                {
                                    eachGPUWorkGroupNum[j][i] = eachGPUWorkGroupSizes[j];
                                }
                            }
                            else
                            {
                                for (j = 0; j < usedGpu; j++)
                                {
                                    eachGPUWorkGroupNum[j][i] = numGroups[i];
                                }
                            }
                        }

                        for(i = 0; i < gpuCount; i++)
                        {
                            datas[i] = (gctINT *) eachGPUWorkGroupNum[i];
                        }

                        Columns = 3;
                        gcmONERROR(gcoSHADER_BindUniformCombinedMode(gcvNULL, hwConstRegAddr, hwConstRegNo, Columns, Rows,
                            1, gcvFALSE, Columns * 4, 0, (gctPOINTER) datas, gpuCount,gcvUNIFORMCVT_NONE, gcSHADER_TYPE_CL));
                    }
                    else
                    {
                        data = (gctPOINTER)numGroups;
                        Columns = 3;
                    }
                }
                break;
            case SHS_PRIV_CONSTANT_KIND_LOCAL_ADDRESS_SPACE:
                {
                    gctUINT32 gpuCount, perGpuMemSize;
                    gctUINT32  conWorkGroupCount = totalNumGroups;

                    /* TODO: handle the case local memory is handled by compiler or HW directly, we don't need to allocate it. */
                    gcmASSERT(kep->kernelHints.localMemorySize > 0);
                    /* Allocate the memory allocation info. */
                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    NDRangeKernel->localAddressSpace = pointer;
                    memAllocInfo =  (clsMemAllocInfo_PTR) NDRangeKernel->localAddressSpace;
                    gcoCL_GetHWConfigGpuCount(&gpuCount);

                    if (hints->workGroupCount != 0 &&
                        (gctINT)hints->workGroupCount < totalNumGroups)
                    {
                        conWorkGroupCount = hints->workGroupCount;
                    }

                    memAllocInfo->allocatedSize = kep->kernelHints.localMemorySize * conWorkGroupCount;
                    memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 128);
                    perGpuMemSize = memAllocInfo->allocatedSize;
                    memAllocInfo->allocatedSize *= gpuCount;
                    /* Allocate the physical buffer */
                    clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                        &memAllocInfo->physical,
                        &memAllocInfo->logical,
                        &memAllocInfo->node,
                        gcvSURF_INDEX,
                        0),
                        CL_INVALID_VALUE);

                    data = (gctINT *) &memAllocInfo->physical;
                    gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_WRITE);

                    if(gpuCount > 1)
                    {
                        gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                        gctINT * datas[gcdMAX_3DGPU_COUNT];
                        gctUINT32 index = 0;
                        gpuPhysicals[0] = memAllocInfo->physical;
                        datas[0] = (gctINT *) &gpuPhysicals[0];
                        isCombinedMode = gcvTRUE;

                        for(index = 1; index < gpuCount; ++index)
                        {
                            gpuPhysicals[index] = gpuPhysicals[index-1] + perGpuMemSize;
                            datas[index] = (gctINT *) &gpuPhysicals[index];
                        }

                        gcmONERROR(gcoSHADER_BindUniformCombinedMode(gcvNULL, hwConstRegAddr, hwConstRegNo, Columns, Rows,
                            1, gcvFALSE, Columns * 4, 0, (gctPOINTER) datas, gpuCount, gcvUNIFORMCVT_NONE, gcSHADER_TYPE_CL));
                    }
                }
                break;
            case SHS_PRIV_CONSTANT_KIND_PRIVATE_ADDRESS_SPACE:
                {
                    gctUINT32 gpuCount, perGpuMemSize;
                    gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                    gctINT * datas[gcdMAX_3DGPU_COUNT];
                    gctUINT32 index = 0;

                    gcmASSERT(kep->kernelHints.privateMemorySize > 0);
                    /* Allocate the memory allocation info. */
                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    NDRangeKernel->privateAddressSpace = pointer;
                    memAllocInfo =  (clsMemAllocInfo_PTR) NDRangeKernel->privateAddressSpace;

                    /* Compare the workThreadCount with total item number and choose the smaller one. */
                    if (hints->workThreadCount != 0 &&
                        (gctINT)hints->workThreadCount < totalNumItems)
                    {
                        memAllocInfo->allocatedSize = kep->kernelHints.privateMemorySize * hints->workThreadCount;
                    }
                    else
                    {
                        memAllocInfo->allocatedSize = kep->kernelHints.privateMemorySize * totalNumItems;
                    }

                    gcoCL_GetHWConfigGpuCount(&gpuCount);
                    memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 128);
                    perGpuMemSize = memAllocInfo->allocatedSize;
                    memAllocInfo->allocatedSize *= gpuCount;

                    /* Allocate the physical buffer */
                    clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                        &memAllocInfo->physical,
                        &memAllocInfo->logical,
                        &memAllocInfo->node,
                        gcvSURF_INDEX,
                        0),
                        CL_INVALID_VALUE);

                    data = (gctINT *) &memAllocInfo->physical;
                    gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_WRITE);

                    if(gpuCount > 1)
                    {
                        gpuPhysicals[0] = memAllocInfo->physical;
                        datas[0] = (gctINT *) &gpuPhysicals[0];
                        isCombinedMode = gcvTRUE;

                        for(index = 1; index < gpuCount; ++index)
                        {
                            gpuPhysicals[index] = gpuPhysicals[index-1] + perGpuMemSize;
                            datas[index] = (gctINT *) &gpuPhysicals[index];
                        }

                        gcmONERROR(gcoSHADER_BindUniformCombinedMode(gcvNULL, hwConstRegAddr, hwConstRegNo, Columns, Rows,
                            1, gcvFALSE, Columns * 4, 0, (gctPOINTER) datas, gpuCount, gcvUNIFORMCVT_NONE, gcSHADER_TYPE_CL));
                    }
                }
                break;
            case SHS_PRIV_CONSTANT_KIND_CONSTANT_ADDRESS_SPACE:
                memAllocInfo =  (clsMemAllocInfo_PTR) privEntry->commonPrivm.pPrivateData;
                data = (gctINT *) &memAllocInfo->physical;
                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_WRITE);
                break;
            case SHS_PRIV_CONSTANT_KIND_GLOBAL_SIZE:
                globalWorkSize[0] = NDRangeKernel->globalWorkSize[0] ? NDRangeKernel->globalWorkSize[0] : 1;
                globalWorkSize[1] = NDRangeKernel->globalWorkSize[1] ? NDRangeKernel->globalWorkSize[1] : 1;
                globalWorkSize[2] = NDRangeKernel->globalWorkSize[2] ? NDRangeKernel->globalWorkSize[2] : 1;
                data = (gctPOINTER)globalWorkSize;
                Columns = 3;
                break;
            case SHS_PRIV_CONSTANT_KIND_LOCAL_SIZE:
                localWorkSize[0] = NDRangeKernel->localWorkSize[0] ? NDRangeKernel->localWorkSize[0] : 1;
                localWorkSize[1] = NDRangeKernel->localWorkSize[1] ? NDRangeKernel->localWorkSize[1] : 1;
                localWorkSize[2] = NDRangeKernel->localWorkSize[2] ? NDRangeKernel->localWorkSize[2] : 1;
                data = (gctPOINTER)localWorkSize;
                Columns = 3;
                break;
            case SHS_PRIV_CONSTANT_KIND_GLOBAL_OFFSET:
                globalOffset[0] = NDRangeKernel->globalWorkOffset[0];
                globalOffset[1] = NDRangeKernel->globalWorkOffset[1];
                globalOffset[2] = NDRangeKernel->globalWorkOffset[2];
                data = (gctPOINTER)globalOffset;
                Columns = 3;
                break;
            case SHS_PRIV_CONSTANT_KIND_WORK_DIM:
                data = (gctPOINTER)&NDRangeKernel->workDim;
                break;
            case SHS_PRIV_CONSTANT_KIND_PRINTF_ADDRESS:
                /* Allocate the memory allocation info. */
                gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                gcoOS_ZeroMemory(pointer, bytes);
                NDRangeKernel->printfBufferAddress = pointer;
                memAllocInfo =  (clsMemAllocInfo_PTR) NDRangeKernel->printfBufferAddress;
                memAllocInfo->allocatedSize = NDRangeKernel->kernel->context->devices[0]->deviceInfo.maxPrintfBufferSize;
                /* Allocate the physical buffer */
                clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                                &memAllocInfo->physical,
                                                &memAllocInfo->logical,
                                                &memAllocInfo->node,
                                                gcvSURF_INDEX,
                                                0),
                           CL_INVALID_VALUE);
                /* Initialize printf buffer with 0xFF so we can use it to check if it is written. */
                gcoOS_MemFill(memAllocInfo->logical, 0xFF, memAllocInfo->allocatedSize);
                data = (gctINT *) &memAllocInfo->physical;
                NDRangeKernel->printThreadNum = totalNumItems;
                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_WRITE, gcvFENCE_TYPE_WRITE);
                break;
            case SHS_PRIV_CONSTANT_KIND_WORKITEM_PRINTF_BUFFER_SIZE:
                NDRangeKernel->printbufferSize = NDRangeKernel->kernel->context->devices[0]->deviceInfo.maxPrintfBufferSize / totalNumItems;
                data = &NDRangeKernel->printbufferSize;
                break;
            case SHS_PRIV_CONSTANT_KIND_WORK_THREAD_COUNT:
                workThreadCount = 0;
                /*
                ** If workThreadCount is chosen, set the value to workThreadCount;
                ** otherwise use 0 because (i MOD 0) == i.
                */
                if (hints->workThreadCount != 0 &&
                    (gctINT)hints->workThreadCount < totalNumItems)
                {
                    workThreadCount = (gctUINT32)hints->workThreadCount;
                }
                data = &workThreadCount;
                break;
            case SHS_PRIV_CONSTANT_KIND_WORK_GROUP_COUNT:
                workGroupCount = 0;
                /*
                ** If workThreadCount is chosen, set the value to workThreadCount;
                ** otherwise use 0 because (i MOD 0) == i.
                */
                if (hints->workGroupCount != 0 &&
                    (gctINT)hints->workGroupCount < totalNumGroups)
                {
                    workGroupCount = (gctUINT32)hints->workGroupCount;
                }
                data = &workGroupCount;
                break;
            case SHS_PRIV_CONSTANT_KIND_LOCAL_MEM_SIZE:
                /* Allocate the memory allocation info. */
                gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                gcoOS_ZeroMemory(pointer, bytes);
                privEntry->commonPrivm.pPrivateData = pointer;
                memAllocInfo =  (clsMemAllocInfo_PTR) privEntry->commonPrivm.pPrivateData;
                memAllocInfo->allocatedSize = NDRangeKernel->localKernelArgSize;
                data = &localKernelArgSize;
                break;
            case SHS_PRIV_CONSTANT_KIND_WORK_GROUP_ID_OFFSET:
                {
                    gctUINT32 gpuCount,usedGpu;
                    gctUINT i = 0, maxDimIndex = 0;
                    gctUINT eachGPUWorkGroupSizes[gcdMAX_3DGPU_COUNT] = { 0};
                    gctUINT eachGPUWorkGroupIDOffsets[gcdMAX_3DGPU_COUNT][3] = {{ 0 }};
                    gctUINT eachGPUGroupCount, restGroupCount;
                    gctINT *datas[gcdMAX_3DGPU_COUNT] = {gcvNULL};
                    gctUINT maxWorkGroupCount = NDRangeKernel->globalWorkSize[0] / NDRangeKernel->localWorkSize[0];

                    gcoCL_GetHWConfigGpuCount(&gpuCount);
                    isCombinedMode = gcvTRUE;

                    gcmASSERT(gpuCount > 1);  /*GROUP_ID_OFFSET only generated for CombinedMode*/
                    usedGpu = gpuCount;

                    for(i = 1; i < NDRangeKernel->workDim ; i++)  /*The split method shoud match with gcoHardware_InvokThreadWalker  */
                    {
                        if(NDRangeKernel->globalWorkSize[i] / NDRangeKernel->localWorkSize[i] > maxWorkGroupCount)
                        {
                            maxWorkGroupCount = NDRangeKernel->globalWorkSize[i] / NDRangeKernel->localWorkSize[i];
                            maxDimIndex = i;
                        }
                    }

                    eachGPUGroupCount = maxWorkGroupCount / gpuCount;
                    restGroupCount = maxWorkGroupCount % gpuCount;

                    for(i = 0 ;i < gpuCount; i++)
                    {
                        eachGPUWorkGroupSizes[i] = eachGPUGroupCount;
                    }

                    for(i = 0 ;i <restGroupCount; i++)
                    {
                        eachGPUWorkGroupSizes[i]++;
                    }

                    if(eachGPUGroupCount == 0) usedGpu = restGroupCount;

                    eachGPUWorkGroupIDOffsets[0][maxDimIndex] = 0;

                    for(i = 1; i < usedGpu; i++)
                    {
                        eachGPUWorkGroupIDOffsets[i][maxDimIndex] = eachGPUWorkGroupSizes[i - 1] +  eachGPUWorkGroupIDOffsets[i-1][maxDimIndex];

                    }

                    for(i = 0; i < gpuCount; i++)
                    {
                        datas[i] = (gctINT *) eachGPUWorkGroupIDOffsets[i];
                    }

                    Columns = 3;
                    gcmONERROR(gcoSHADER_BindUniformCombinedMode(gcvNULL, hwConstRegAddr, hwConstRegNo, Columns, Rows,
                        1, gcvFALSE, Columns * 4, 0, (gctPOINTER) datas, gpuCount,gcvUNIFORMCVT_NONE, gcSHADER_TYPE_CL));
                }
                break;
            case SHS_PRIV_CONSTANT_KIND_TEMP_REG_SPILL_MEM_ADDRESS:
                {
                    gctUINT32 gpuCount, perGpuMemSize;
                    gctUINT32 gpuPhysicals[gcdMAX_3DGPU_COUNT] = {0};
                    gctINT * datas[gcdMAX_3DGPU_COUNT];

                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    NDRangeKernel->spillMemAddressSpace = (clsMemAllocInfo_PTR)  pointer;
                    memAllocInfo =  (clsMemAllocInfo_PTR) NDRangeKernel->spillMemAddressSpace;
                    memAllocInfo->allocatedSize = kep->sep.exeHints.derivedHints.globalStates.gprSpillSize;
                    isCombinedMode = gcvTRUE;
                    gcoCL_GetHWConfigGpuCount(&gpuCount);
                    gcmASSERT(gpuCount > 0);    /*SHS_PRIV_CONSTANT_KIND_TEMP_REG_SPILL_MEM_ADDRESS only generated for CombinedMode*/

                    if (memAllocInfo->allocatedSize > 0)
                    {
                        gctUINT32 i;

                        memAllocInfo->allocatedSize = gcmALIGN(memAllocInfo->allocatedSize, 64);
                        perGpuMemSize =  memAllocInfo->allocatedSize;
                        memAllocInfo->allocatedSize *= gpuCount;

                        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                            &memAllocInfo->physical,
                            &memAllocInfo->logical,
                            &memAllocInfo->node,
                            gcvSURF_INDEX,
                            0),
                            CL_INVALID_VALUE
                            );

                        gpuPhysicals[0] = memAllocInfo->physical;
                        datas[0] = (gctINT *) &gpuPhysicals[0];

                        for(i = 1; i < gpuCount; i++)
                        {
                            gpuPhysicals[i] = gpuPhysicals[i-1] + perGpuMemSize;
                            datas[i] = (gctINT *) &gpuPhysicals[i];
                        }

                        gcmONERROR(gcoSHADER_BindUniformCombinedMode(gcvNULL, hwConstRegAddr, hwConstRegNo, Columns, Rows,
                            1, gcvFALSE, Columns * 4, 0, (gctPOINTER) datas, gpuCount, gcvUNIFORMCVT_NONE, gcSHADER_TYPE_CL));

                    }
                    break;
                }
            default:
                break;
            }

            gcmASSERT(privEntry->u.pSubCBMapping->subArrayRange == 1);
            if(!isCombinedMode)   /*combined mode is processed in each switch case */
            {
                gcmONERROR(gcoSHADER_BindUniform(gcvNULL,hwConstRegAddr, hwConstRegNo, Columns, Rows,
                    1, gcvFALSE, Columns * 4, 0, data, gcvUNIFORMCVT_NONE,gcSHADER_TYPE_CL));
            }
        }
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

gctINT
clfExecuteVIRKernel(clsCommandNDRangeVIRKernel_PTR NDRangeKernel)
{
    gctINT                  status;
    clsKernel_PTR           kernel = NDRangeKernel->kernel;
    gctUINT                 workDim = NDRangeKernel->workDim;
    size_t *                globalWorkOffset = NDRangeKernel->globalWorkOffset;
    size_t *                globalScale = NDRangeKernel->globalScale;
    size_t *                globalWorkSize = NDRangeKernel->globalWorkSize;
    size_t *                localWorkSize = NDRangeKernel->localWorkSize;
    gcsPROGRAM_STATE        ProgramState;

    gcmHEADER_ARG("Kernel=0x%x", kernel);

    /* Load kernel states. */
    ProgramState.hints = &NDRangeKernel->currentInstance->hwStates.hints;
    ProgramState.stateBuffer = NDRangeKernel->currentInstance->hwStates.pStateBuffer;
    ProgramState.stateBufferSize = NDRangeKernel->currentInstance->hwStates.stateBufferSize;
    ProgramState.stateDelta = NDRangeKernel->currentInstance->hwStates.pStateDelta;
    ProgramState.stateDeltaSize = NDRangeKernel->currentInstance->hwStates.stateDeltaSize;
    clmONERROR(gcoCL_LoadKernel(ProgramState),
               CL_OUT_OF_RESOURCES);

    /* Adjust local work sizes if not specified by the application. */
    clmONERROR(clfAdjustLocalWorkSize(kernel,
                                      workDim,
                                      globalWorkOffset,
                                      globalWorkSize,
                                      localWorkSize),
               CL_INVALID_VALUE);

    /* Flush kernel uniform/image/sampler/buffer ..... */
    clmONERROR(clfFlushVIRKernelResource(NDRangeKernel), CL_INVALID_VALUE);

    gcmONERROR(gcoCL_InvokeKernel(workDim,
                                  globalWorkOffset,
                                  globalScale,
                                  globalWorkSize,
                                  localWorkSize,
                                  NDRangeKernel->currentInstance->hwStates.hints.valueOrder,
                                  NDRangeKernel->currentInstance->hwStates.hints.threadGroupSync,
                                  NDRangeKernel->currentInstance->hwStates.hints.memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL],
                                  NDRangeKernel->currentInstance->hwStates.hints.fsIsDual16));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteKernel(
    clsKernel_PTR       Kernel,
    clsKernelInstance_PTR Instance,
    gctUINT             NumArgs,
    clsArgument_PTR     Args,
    gctUINT             WorkDim,
    size_t              GlobalWorkOffset[3],
    size_t              GlobalScale[3],
    size_t              GlobalWorkSize[3],
    size_t              LocalWorkSize[3],
    clsPrivateBuffer_PTR *privateBufList
    )
{
    gctINT                  status;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    /* Load kernel states. */
    clmONERROR(gcoCL_LoadKernel(Instance->programState),
               CL_OUT_OF_RESOURCES);

    /* Adjust local work sizes if not specified by the application. */
    clmONERROR(clfAdjustLocalWorkSize(Kernel,
                                      WorkDim,
                                      GlobalWorkOffset,
                                      GlobalWorkSize,
                                      LocalWorkSize),
               CL_INVALID_VALUE);

    clmONERROR(clfFlushKernelUniform(Kernel,
                                     Instance,
                                     NumArgs,
                                     Args,
                                     WorkDim,
                                     GlobalWorkOffset,
                                     GlobalWorkSize,
                                     LocalWorkSize,
                                     privateBufList),
               CL_INVALID_VALUE);

    /* Set up local memory for kernel arguments. */
    clmONERROR(clfLoadKernelArgLocalMemValues(Kernel,
                                              NumArgs,
                                              Args,
                                              WorkDim,
                                              GlobalWorkOffset,
                                              GlobalWorkSize,
                                              LocalWorkSize),
               CL_INVALID_VALUE);

    gcmONERROR(gcoCL_InvokeKernel(WorkDim,
                                  GlobalWorkOffset,
                                  GlobalScale,
                                  GlobalWorkSize,
                                  LocalWorkSize,
                                  Instance->programState.hints->valueOrder,
                                  Instance->programState.hints->threadGroupSync,
                                  Instance->programState.hints->memoryAccessFlags[gcvSHADER_HIGH_LEVEL][gcvPROGRAM_STAGE_OPENCL],
                                  Instance->programState.hints->fsIsDual16));

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gctBOOL
clfNeedGlobalWorkSizeWorkaround(clsCommandNDRangeKernel_PTR NDRangeKernel)
{
    gctBOOL globalWorkSizeWorkaroundNeed = gcvFALSE;
    size_t i;

    do {
        gctBOOL localWorkGroupSet = gcvFALSE;
        gctBOOL globalWorkSizeAlign = gcvFALSE;
        /* check Hardware feature */

        globalWorkSizeWorkaroundNeed = !NDRangeKernel->kernel->context->devices[0]->deviceInfo.multiWGPack;
        if (!globalWorkSizeWorkaroundNeed) break;

        /* Check work Dimension, only support 1D and 2D. 3D calculate complex, skip it so far */
        if (NDRangeKernel->workDim >= 3)
        {
            globalWorkSizeWorkaroundNeed = gcvFALSE;
        }
        if (!globalWorkSizeWorkaroundNeed) break;
        /* check Application set local group size */
        for(i=0;i<NDRangeKernel->workDim;i++)
        {
            localWorkGroupSet=(NDRangeKernel->localWorkSize[i]!=0 ? gcvTRUE:gcvFALSE);
            if (localWorkGroupSet)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }
        }
        if (!globalWorkSizeWorkaroundNeed) break;

        /* check the global size is align or not. if already align, don't need continue */
        for(i=0;i<NDRangeKernel->workDim;i++)
        {
            globalWorkSizeAlign = NDRangeKernel->globalWorkSize[i] % NDRangeKernel->kernel->preferredWorkGroupSizeMultiple == 0 ? gcvTRUE:gcvFALSE;
            if (globalWorkSizeAlign == gcvFALSE)
            {
                globalWorkSizeWorkaroundNeed = gcvTRUE;
                break;
            }
            else
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
            }
        }
        if (!globalWorkSizeWorkaroundNeed) break;

        /* check Kernel use group size relate build-in function */
        i = 0;

        while(i < GetShaderUniformCount((gcSHADER)NDRangeKernel->currentInstance->binary))
        {
            gcUNIFORM uniform = GetShaderUniform((gcSHADER)NDRangeKernel->currentInstance->binary, i);
            gctCONST_STRING tmpName=gcvNULL;
            gctUINT8 uniformName[128]={0};
            if (uniform == gcvNULL) break;

            gcUNIFORM_GetName(uniform, gcvNULL, &tmpName);
            gcoOS_StrCopySafe((gctSTRING)uniformName, gcmSIZEOF(uniformName), tmpName);

            if (gcoOS_MemCmp(uniformName, "#global_size", 12) == gcvSTATUS_OK)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }

            if (gcoOS_MemCmp(uniformName, "#local_size", 11) == gcvSTATUS_OK)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }

            if (gcoOS_MemCmp(uniformName, "#num_groups",11) == gcvSTATUS_OK)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }

            if (gcoOS_MemCmp(uniformName, "#global_offset",14) == gcvSTATUS_OK)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }
            i++;
        }
        if (!globalWorkSizeWorkaroundNeed) break;

        /* check Kernel use local id */
        i = 0;
        while (i < GetShaderAttributeCount((gcSHADER)NDRangeKernel->currentInstance->binary)) {
            gctCONST_STRING attributeName=gcvNULL;
            gcATTRIBUTE attribute = GetShaderAttribute((gcSHADER)NDRangeKernel->currentInstance->binary, i);
            if (attribute == gcvNULL) break;
            gcATTRIBUTE_GetName((gcSHADER)NDRangeKernel->currentInstance->binary, attribute, gcvFALSE, gcvNULL, &attributeName);
            if (attributeName == gcvNULL) break;
            if (gcoOS_StrCmp(attributeName, "#local_id") == 0)
            {
                globalWorkSizeWorkaroundNeed = gcvFALSE;
                break;
            }
            i++;
        }
        if (!globalWorkSizeWorkaroundNeed) break;

        /* check barrier instruction */
        if(NDRangeKernel->currentInstance->programState.hints)
        {
            /* relax the maxworkgroupsize while shader used barrier as HW limit*/
            for (i = 0; i < GetShaderCodeCount((gcSHADER)NDRangeKernel->currentInstance->binary); i++)
            {
                gcSL_INSTRUCTION inst   = GetShaderInstruction((gcSHADER)NDRangeKernel->currentInstance->binary, i);
                /* barrier is design base on work group so we can't refine it*/
                if(gcmSL_OPCODE_GET(inst->opcode, Opcode) == gcSL_BARRIER)
                {
                    globalWorkSizeWorkaroundNeed = gcvFALSE;
                    break;
                }
            }
        }
        if (!globalWorkSizeWorkaroundNeed) break;

    } while (gcvFALSE);

    return globalWorkSizeWorkaroundNeed;
}

gctINT
clfExecuteCommandNDRangeVIRKernel(
    clsCommand_PTR  Command
    )
{
    clsCommandNDRangeVIRKernel_PTR     NDRangeKernel;
    gctINT              status = gcvSTATUS_OK;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_NDRANGE_VIR_KERNEL, CL_INVALID_VALUE);

    EVENT_SET_GPU_RUNNING(Command, gcvENGINE_RENDER);

#if VIVANTE_PROFILER
    if (Command->commandQueue->profiler.perClfinish == gcvFALSE)
        clfBeginProfiler(Command->commandQueue);
#endif

    NDRangeKernel = &Command->u.NDRangeVIRKernel;

    clmONERROR(clfExecuteVIRKernel(NDRangeKernel), status);

#if VIVANTE_PROFILER
    if (Command->commandQueue->profiler.perClfinish == gcvFALSE)
        clfEndProfiler(Command->commandQueue, NDRangeKernel->kernel);
#endif

    clmONERROR(gcoCL_Commit(gcvFALSE), CL_INVALID_VALUE);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandNDRangeKernel(
    clsCommand_PTR  Command
    )
{
    clsCommandNDRangeKernel_PTR     NDRangeKernel;
    gctINT              status = gcvSTATUS_OK;
    size_t              i;
    size_t              globalWorkOffset[9] = {0};
    size_t              globalWorkSize[9] = {0};
    size_t              localWorkSize[9] = {0};

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_NDRANGE_KERNEL, CL_INVALID_VALUE);

    EVENT_SET_GPU_RUNNING(Command, gcvENGINE_RENDER);

#if VIVANTE_PROFILER
    if (Command->commandQueue->profiler.perClfinish == gcvFALSE)
        clfBeginProfiler(Command->commandQueue);
#endif

    NDRangeKernel = &Command->u.NDRangeKernel;


    if (clfNeedGlobalWorkSizeWorkaround(NDRangeKernel))
    {
        clfCalcLocalWorkSize(NDRangeKernel->kernel, NDRangeKernel->workDim,
                                NDRangeKernel->globalWorkOffset,NDRangeKernel->globalWorkSize, NDRangeKernel->localWorkSize,
                                globalWorkOffset, globalWorkSize, localWorkSize);
        for(i = 0; i < 9; i+=3)
        {
            if(globalWorkSize[i]==0) break;
#if gcdFRAMEINFO_STATISTIC
            gcoHAL_FrameInfoOps(gcvNULL, gcvFRAMEINFO_COMPUTE_NUM, gcvFRAMEINFO_OP_INC, gcvNULL);
#endif
            clmONERROR(clfExecuteKernel(NDRangeKernel->kernel,
                                        NDRangeKernel->currentInstance,
                                        NDRangeKernel->numArgs,
                                        NDRangeKernel->args,
                                        NDRangeKernel->workDim,
                                        globalWorkOffset+i,
                                        NDRangeKernel->globalScale,
                                        globalWorkSize+i,
                                        localWorkSize+i,
                                        &(Command->commandQueue->privateBufList)),
                        status);
        }
    }
    else
    {
#if gcdFRAMEINFO_STATISTIC
        gcoHAL_FrameInfoOps(gcvNULL, gcvFRAMEINFO_COMPUTE_NUM, gcvFRAMEINFO_OP_INC, gcvNULL);
#endif
        clmONERROR(clfExecuteKernel(NDRangeKernel->kernel,
                                    NDRangeKernel->currentInstance,
                                    NDRangeKernel->numArgs,
                                    NDRangeKernel->args,
                                    NDRangeKernel->workDim,
                                    NDRangeKernel->globalWorkOffset,
                                    NDRangeKernel->globalScale,
                                    NDRangeKernel->globalWorkSize,
                                    NDRangeKernel->localWorkSize,
                                    &(Command->commandQueue->privateBufList)),
                    status);
    }

#if VIVANTE_PROFILER
    if (Command->commandQueue->profiler.perClfinish == gcvFALSE)
        clfEndProfiler(Command->commandQueue, NDRangeKernel->kernel);
#endif

    clmONERROR(gcoCL_Commit(gcvFALSE), CL_INVALID_VALUE);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandTask(
    clsCommand_PTR  Command
    )
{
    clsCommandTask_PTR      task;
    size_t                  globalWorkOffset[3] = {0, 0, 0};
    size_t                  globalScale[3] = {1, 1, 1};
    size_t                  globalWorkSize[3] = {1, 0, 0};
    size_t                  localWorkSize[3]  = {1, 0, 0};
    gctINT                  status;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_TASK, CL_INVALID_VALUE);

    EVENT_SET_GPU_RUNNING(Command, gcvENGINE_RENDER);

    task = &Command->u.task;

    clmONERROR(clfExecuteKernel(task->kernel,
                                task->currentInstance,
                                task->numArgs,
                                task->args,
                                1,
                                globalWorkOffset,
                                globalScale,
                                globalWorkSize,
                                localWorkSize,
                                &(Command->commandQueue->privateBufList)),
               status);

    clmONERROR(gcoCL_Commit(gcvFALSE), CL_INVALID_VALUE);
OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteCommandNativeKernel(
    clsCommand_PTR  Command
    )
{
    return CL_INVALID_VALUE;
}

/* Allocate and collect kernel arg info, these info will not change, we use these info to return APP's query
   All recompile special code need in recompile program instance or keep in NDRange inside only (dimension/global size, etc)
   So, for anything could make different by recompile or NDRange argument, we need consider how to get the right return of
   these argument (i.e, maxWorkGroupCount)
*/
gctINT
clfBuildKernelArgs(
    clsKernel_PTR   Kernel
    )
{
    gceSTATUS       status;
    gctPOINTER      pointer = gcvNULL;
    gctSIZE_T       bytes;
    clsArgument_PTR argument;
    gctUINT         i;
    clsMemAllocInfo_PTR memAllocInfo = gcvNULL;

    if (Kernel->numArgs == 0)
    {
        Kernel->args = gcvNULL;
        return gcvSTATUS_OK;
    }

    /* Allocate the array of arguments. */
    bytes = Kernel->numArgs * gcmSIZEOF(clsArgument);
    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
    gcoOS_ZeroMemory(pointer, bytes);

    Kernel->args = argument = (clsArgument_PTR)pointer;
    for (i = 0; i < Kernel->numArgs; i++, argument++)
    {
        gcUNIFORM uniform;
        gcSHADER_TYPE type;
        gcSL_FORMAT format;
        gctBOOL isPointer;
        gctUINT length;
        gctSIZE_T bytes;

        gcmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->masterInstance.binary, i, &uniform));

        if (!uniform) continue;
        if (isUniformCompiletimeInitialized(uniform)) continue;

        gcmONERROR(gcUNIFORM_GetType(uniform, &type, &length));
        gcmONERROR(gcUNIFORM_GetFormat(uniform, &format, &isPointer));

        /* add a hint to indicate "printf" */
        if (isUniformPrintfAddress(uniform))
        {
            Kernel->hasPrintf = gcvTRUE;
        }

        if (isUniformLocalAddressSpace(uniform) ||
            isUniformPrivateAddressSpace(uniform) ||
            isUniformConstantAddressSpace(uniform) ||
            isUniformKernelArgPrivate(uniform) ||
            isUniformKernelArgLocal(uniform) ||
            isUniformKernelArgLocalMemSize(uniform) ||
            isUniformPrintfAddress(uniform) ||
            isUniformTempRegSpillAddress(uniform)
            )
        {
            gctPOINTER pointer;

            bytes = sizeof(clsMemAllocInfo);

            /* Allocate the memory allocation info. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = (clsMemAllocInfo_PTR)pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                if ((strcmp(uniform->name, _sldLocalStorageAddressName) == 0 && gcShaderUseLocalMem((gcSHADER) Kernel->masterInstance.binary))
                    ||
                    Kernel->masterInstance.programState.hints->sharedMemAllocByCompiler)
                {
                    /* local memory is handled by compiler or HW directly, we don't need to allocate it. */
                }
                else
                {
                    gcmONERROR(gcSHADER_GetLocalMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize));
                    Kernel->localMemSize += memAllocInfo->allocatedSize;
                }
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetPrivateMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize));
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                gcmONERROR(gcSHADER_GetConstantMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ));
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }
            else if(isUniformTempRegSpillAddress(uniform))
            {
                 gcsSTORAGE_BLOCK storageBlock;
                 gctINT16   blockIndex = GetUniformBlockID(uniform);
                 gcSHADER_GetStorageBlock((gcSHADER) Kernel->masterInstance.binary, blockIndex, &storageBlock);
                 memAllocInfo->allocatedSize = GetSBBlockSize(storageBlock);
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;
            memAllocInfo = gcvNULL;
        }
        else
        {
            if (isPointer)
            {
                bytes = gcmSIZEOF(gctPOINTER);
            }
            else
            {
                switch (type)
                {
                case gcSHADER_FLOAT_X1:
                case gcSHADER_FLOAT16_X1:
                case gcSHADER_BOOLEAN_X1:
                case gcSHADER_INTEGER_X1:
                case gcSHADER_INT8_X1:
                case gcSHADER_INT16_X1:
                case gcSHADER_UINT_X1:
                case gcSHADER_UINT8_X1:
                case gcSHADER_UINT16_X1:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X2:
                case gcSHADER_FLOAT16_X2:
                case gcSHADER_BOOLEAN_X2:
                case gcSHADER_INTEGER_X2:
                case gcSHADER_INT8_X2:
                case gcSHADER_INT16_X2:
                case gcSHADER_UINT_X2:
                case gcSHADER_UINT8_X2:
                case gcSHADER_UINT16_X2:
                    bytes = 2 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X3:
                case gcSHADER_FLOAT16_X3:
                case gcSHADER_BOOLEAN_X3:
                case gcSHADER_INTEGER_X3:
                case gcSHADER_INT8_X3:
                case gcSHADER_INT16_X3:
                case gcSHADER_UINT_X3:
                case gcSHADER_UINT8_X3:
                case gcSHADER_UINT16_X3:
                    bytes = 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X4:
                case gcSHADER_FLOAT16_X4:
                case gcSHADER_BOOLEAN_X4:
                case gcSHADER_INTEGER_X4:
                case gcSHADER_INT8_X4:
                case gcSHADER_INT16_X4:
                case gcSHADER_UINT_X4:
                case gcSHADER_UINT8_X4:
                case gcSHADER_UINT16_X4:
                    bytes = 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_INT64_X1:
                case gcSHADER_UINT64_X1:
                    bytes = 1 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X2:
                case gcSHADER_UINT64_X2:
                    bytes = 2 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X3:
                case gcSHADER_UINT64_X3:
                    bytes = 4 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X4:
                case gcSHADER_UINT64_X4:
                    bytes = 4 * gcmSIZEOF(cl_long) * length;
                    break;

                case gcSHADER_FLOAT_2X2:
                    bytes = 2 * 2 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_3X3:
                    bytes = 3 * 3 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_4X4:
                    bytes = 4 * 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_IMAGE_2D:
                case gcSHADER_IMAGE_3D:
                case gcSHADER_SAMPLER_T:
                case gcSHADER_IMAGE_1D_T:
                case gcSHADER_IMAGE_1D_ARRAY_T:
                case gcSHADER_IMAGE_1D_BUFFER_T:
                case gcSHADER_IMAGE_2D_ARRAY:
                    bytes = 1 * gcmSIZEOF(cl_uint) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
                    break;
                case gcSHADER_INT8_P2:
                case gcSHADER_UINT8_P2:
                case gcSHADER_INT16_P2:
                case gcSHADER_UINT16_P2:
                    bytes = 1 * gcmSIZEOF(cl_int2) * length;
                    break;
                case gcSHADER_INT8_P3:
                case gcSHADER_UINT8_P3:
                case gcSHADER_INT16_P3:
                case gcSHADER_UINT16_P3:
                    bytes = 1 * gcmSIZEOF(cl_int3) * length;
                    break;
                case gcSHADER_INT8_P4:
                case gcSHADER_UINT8_P4:
                case gcSHADER_INT16_P4:
                case gcSHADER_UINT16_P4:
                    bytes = 1 * gcmSIZEOF(cl_int4) * length;
                    break;
                case gcSHADER_INT8_P8:
                case gcSHADER_UINT8_P8:
                case gcSHADER_INT16_P8:
                case gcSHADER_UINT16_P8:
                    bytes = 1 * gcmSIZEOF(cl_int8) * length;
                    break;
                case gcSHADER_INT8_P16:
                case gcSHADER_UINT8_P16:
                case gcSHADER_INT16_P16:
                case gcSHADER_UINT16_P16:
                    bytes = 1 * gcmSIZEOF(cl_int16) * length;
                    break;
                case gcSHADER_FLOAT16_P2:
                    bytes = 1 * gcmSIZEOF(cl_float2) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P3:
                    bytes = 1 * gcmSIZEOF(cl_float3) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P4:
                    bytes = 1 * gcmSIZEOF(cl_float4) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P8:
                    bytes = 1 * gcmSIZEOF(cl_float8) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P16:
                    bytes = 1 * gcmSIZEOF(cl_float16) * length >> 1;
                    break;
                default:
                    clmASSERT("Unknown shader type", CL_INVALID_VALUE);
                    bytes = 0;
                }

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                    bytes /= 4;
                    break;

                case gcSL_INT16:
                case gcSL_UINT16:
                    bytes /= 2;
                    break;

                default:
                    break;
                }
            }

            /* Allocate the data array. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data));

            gcoOS_ZeroMemory(argument->data, bytes);

            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvFALSE;
            argument->isPointer  = isPointer;
        }

        /* build arg qualifier */
        {
            gceUNIFORM_FLAGS flags;
            gcSHADER_TYPE type;
            gctTYPE_QUALIFIER tqualifier = argument->uniform->qualifier;

            clmONERROR(gcUNIFORM_GetFlags(argument->uniform, &flags),
                      CL_INVALID_VALUE);

            switch (flags & 0xff)  /*low 8 bits stand for uniform kind*/
            {
            case gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG_LOCAL:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG_PRIVATE:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG:
                {
                    gctBOOL isPointer;
                    clmONERROR(gcUNIFORM_GetFormat(argument->uniform, gcvNULL, &isPointer),
                              CL_INVALID_VALUE);
                    if (isPointer)
                    {
                        argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
                    }
                    else
                    {
                        argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                    }
                }
                break;
            default:
                /* default value if address qualifier is not set */
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            }

            clmONERROR(gcUNIFORM_GetType(argument->uniform, &type, gcvNULL),
                       CL_INVALID_VALUE);

            argument->accessQualifier = CL_KERNEL_ARG_ACCESS_NONE;

            switch (type)
            {
            case gcSHADER_IMAGE_2D_T:
            case gcSHADER_IMAGE_3D_T:
            case gcSHADER_IMAGE_2D_ARRAY_T:
            case gcSHADER_IMAGE_1D_T:
            case gcSHADER_IMAGE_1D_ARRAY_T:
            case gcSHADER_IMAGE_1D_BUFFER_T:
                {
                    gctTYPE_QUALIFIER aqualifier = argument->uniform->qualifier;

                    if (aqualifier & gcvTYPE_QUALIFIER_READ_ONLY)
                        argument->accessQualifier = CL_KERNEL_ARG_ACCESS_READ_ONLY;
                    if (aqualifier & gcvTYPE_QUALIFIER_WRITE_ONLY)
                        argument->accessQualifier = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
                }
                break;
            default:
                break;
            }

            argument->typeQualifier = CL_KERNEL_ARG_TYPE_NONE;

            if (tqualifier & gcvTYPE_QUALIFIER_VOLATILE)
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;

            if (tqualifier & gcvTYPE_QUALIFIER_CONST)
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;

            if (tqualifier & gcvTYPE_QUALIFIER_RESTRICT)
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
        }

        /* build arg type name */
        {
            gcSL_FORMAT   format;
            gcSHADER_TYPE type;
            gctUINT32     length;
            gctSIZE_T     nameOffset = 0;

            clmONERROR(gcUNIFORM_GetType(argument->uniform, &type, &length),
                       CL_INVALID_VALUE);
            clmONERROR(gcUNIFORM_GetFormat(argument->uniform, &format, gcvNULL),
                      CL_INVALID_VALUE);
            nameOffset = GetUniformTypeNameOffset(argument->uniform);

            #define CASE(CONDITION,CLC_TYPE) \
            case CONDITION: \
                gcoOS_StrCopySafe(argument->typeName,ARG_TYPE_NAME_LENTH,#CLC_TYPE); \
                break;
            #define DEFAULT \
            default: \
                break;
            if (nameOffset == -1)
            {
                switch (type)
                {
                CASE(gcSHADER_IMAGE_2D_T,image2d_t);
                CASE(gcSHADER_IMAGE_3D_T,image3d_t);
                CASE(gcSHADER_SAMPLER_T,sampler_t);
                CASE(gcSHADER_IMAGE_2D_ARRAY_T,image2d_array_t);
                CASE(gcSHADER_IMAGE_1D_T,image1d_t);
                CASE(gcSHADER_IMAGE_1D_ARRAY_T,image1d_array_t);
                CASE(gcSHADER_IMAGE_1D_BUFFER_T,image1d_buffer_t);

                default:
                    if(isUniformPointer(argument->uniform))
                    {
                        gctINT vectorSize;

                        switch (format)
                        {
                        CASE(gcSL_INTEGER,int);
                        CASE(gcSL_BOOLEAN,bool);
                        CASE(gcSL_UINT32,uint);
                        CASE(gcSL_INT8,char);
                        CASE(gcSL_UINT8,uchar);
                        CASE(gcSL_INT16,short);
                        CASE(gcSL_UINT16,ushort);
                        CASE(gcSL_INT64,long);
                        CASE(gcSL_UINT64,ulong);
                        CASE(gcSL_FLOAT,float);
                        CASE(gcSL_FLOAT16,half);
                        CASE(gcSL_FLOAT64,double);
                        CASE(gcSL_VOID, void);
                        DEFAULT;
                        }

                        vectorSize = GetUniformVectorSize(argument->uniform);
                        if(vectorSize > 0)
                        {
                           gctUINT offset = gcoOS_StrLen(argument->typeName,gcvNULL);

                           gcmVERIFY_OK(gcoOS_PrintStrSafe(argument->typeName,
                                                           ARG_TYPE_NAME_LENTH,
                                                           &offset,
                                                           "%d*",
                                                           vectorSize));
                        }
                        else
                        {
                            gcoOS_StrCatSafe(argument->typeName,ARG_TYPE_NAME_LENTH,"*");
                        }
                    }
                    else
                    {
                        switch (type)
                        {
                        CASE(gcSHADER_INT8_P2,char2);
                        CASE(gcSHADER_INT8_P3,char3);
                        CASE(gcSHADER_INT8_P4,char4);
                        CASE(gcSHADER_INT8_P8,char8);
                        CASE(gcSHADER_INT8_P16,char16);
                        CASE(gcSHADER_UINT8_P2,uchar2);
                        CASE(gcSHADER_UINT8_P3,uchar3);
                        CASE(gcSHADER_UINT8_P4,uchar4);
                        CASE(gcSHADER_UINT8_P8,uchar8);
                        CASE(gcSHADER_UINT8_P16,uchar16);
                        CASE(gcSHADER_INT16_P2,short2);
                        CASE(gcSHADER_INT16_P3,short3);
                        CASE(gcSHADER_INT16_P4,short4);
                        CASE(gcSHADER_INT16_P8,short8);
                        CASE(gcSHADER_INT16_P16,short16);
                        CASE(gcSHADER_UINT16_P2,ushort2);
                        CASE(gcSHADER_UINT16_P3,ushort3);
                        CASE(gcSHADER_UINT16_P4,ushort4);
                        CASE(gcSHADER_UINT16_P8,ushort8);
                        CASE(gcSHADER_UINT16_P16,ushort16);
                        CASE(gcSHADER_BOOLEAN_X1,bool);
                        CASE(gcSHADER_BOOLEAN_X2,bool2);
                        CASE(gcSHADER_BOOLEAN_X3,bool3);
                        case gcSHADER_BOOLEAN_X4:
                            switch (length)
                            {
                            CASE(1,bool4);
                            CASE(2,bool8);
                            CASE(4,bool16);
                            }
                            break;
                        case gcSHADER_FLOAT_X1:
                        case gcSHADER_FLOAT16_X1:
                            switch (format)
                            {
                            CASE(gcSL_FLOAT,float);
                            CASE(gcSL_FLOAT16,half);
                            CASE(gcSL_FLOAT64,double);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_FLOAT_X2:
                        case gcSHADER_FLOAT16_X2:
                            switch (format)
                            {
                            CASE(gcSL_FLOAT,float2);
                            CASE(gcSL_FLOAT16,half2);
                            CASE(gcSL_FLOAT64,double2);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_FLOAT_X3:
                        case gcSHADER_FLOAT16_X3:
                            switch (format)
                            {
                            CASE(gcSL_FLOAT,float3);
                            CASE(gcSL_FLOAT16,half3);
                            CASE(gcSL_FLOAT64,double3);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_FLOAT_X4:
                        case gcSHADER_FLOAT16_X4:
                            switch (format)
                            {
                            case gcSL_FLOAT:
                                switch (length)
                                {
                                CASE(1,float4);
                                CASE(2,float8);
                                CASE(4,float16);
                                }
                                break;
                            case gcSL_FLOAT16:
                                switch (length)
                                {
                                CASE(1,half4);
                                CASE(2,half8);
                                CASE(4,half16);
                                }
                                break;
                            case gcSL_FLOAT64:
                                switch (length)
                                {
                                CASE(1,double4);
                                CASE(2,double8);
                                CASE(4,double16);
                                }
                                break;
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_INTEGER_X1:
                        case gcSHADER_INT8_X1:
                        case gcSHADER_INT16_X1:
                        case gcSHADER_UINT_X1:
                        case gcSHADER_UINT8_X1:
                        case gcSHADER_UINT16_X1:
                        case gcSHADER_INT64_X1:
                        case gcSHADER_UINT64_X1:
                            switch (format)
                            {
                            CASE(gcSL_INTEGER,int);
                            CASE(gcSL_UINT32,uint);
                            CASE(gcSL_INT8,char);
                            CASE(gcSL_UINT8,uchar);
                            CASE(gcSL_INT16,short);
                            CASE(gcSL_UINT16,ushort);
                            CASE(gcSL_INT64,long);
                            CASE(gcSL_UINT64,ulong);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_INTEGER_X2:
                        case gcSHADER_INT8_X2:
                        case gcSHADER_INT16_X2:
                        case gcSHADER_UINT_X2:
                        case gcSHADER_UINT8_X2:
                        case gcSHADER_UINT16_X2:
                        case gcSHADER_INT64_X2:
                        case gcSHADER_UINT64_X2:
                            switch (format)
                            {
                            CASE(gcSL_INTEGER,int2);
                            CASE(gcSL_UINT32,uint2);
                            CASE(gcSL_INT8,char2);
                            CASE(gcSL_UINT8,uchar2);
                            CASE(gcSL_INT16,short2);
                            CASE(gcSL_UINT16,ushort2);
                            CASE(gcSL_INT64,long2);
                            CASE(gcSL_UINT64,ulong2);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_INTEGER_X3:
                        case gcSHADER_INT8_X3:
                        case gcSHADER_INT16_X3:
                        case gcSHADER_UINT_X3:
                        case gcSHADER_UINT8_X3:
                        case gcSHADER_UINT16_X3:
                        case gcSHADER_INT64_X3:
                        case gcSHADER_UINT64_X3:
                            switch (format)
                            {
                            CASE(gcSL_INTEGER,int3);
                            CASE(gcSL_UINT32,uint3);
                            CASE(gcSL_INT8,char3);
                            CASE(gcSL_UINT8,uchar3);
                            CASE(gcSL_INT16,short3);
                            CASE(gcSL_UINT16,ushort3);
                            CASE(gcSL_INT64,long3);
                            CASE(gcSL_UINT64,ulong3);
                            DEFAULT;
                            }
                            break;
                        case gcSHADER_INTEGER_X4:
                        case gcSHADER_INT8_X4:
                        case gcSHADER_INT16_X4:
                        case gcSHADER_UINT_X4:
                        case gcSHADER_UINT8_X4:
                        case gcSHADER_UINT16_X4:
                        case gcSHADER_INT64_X4:
                        case gcSHADER_UINT64_X4:
                            switch (format)
                            {
                            case gcSL_INTEGER:
                                switch (length)
                                {
                                CASE(1,int4);
                                CASE(2,int8);
                                CASE(4,int16);
                                }
                                break;
                            case gcSL_UINT32:
                                switch (length)
                                {
                                CASE(1,uint4);
                                CASE(2,uint8);
                                CASE(4,uint16);
                                }
                                break;
                            case gcSL_INT8:
                                switch (length)
                                {
                                CASE(1,char4);
                                CASE(2,char8);
                                CASE(4,char16);
                                }
                                break;
                            case gcSL_UINT8:
                                switch (length)
                                {
                                CASE(1,uchar4);
                                CASE(2,uchar8);
                                CASE(4,uchar16);
                                }
                                break;
                            case gcSL_INT16:
                                switch (length)
                                {
                                CASE(1,short4);
                                CASE(2,short8);
                                CASE(4,short16);
                                }
                                break;
                            case gcSL_UINT16:
                                switch (length)
                                {
                                CASE(1,ushort4);
                                CASE(2,ushort8);
                                CASE(4,ushort16);
                                }
                                break;
                            case gcSL_INT64:
                                switch (length)
                                {
                                CASE(1,long4);
                                CASE(2,long8);
                                CASE(4,long16);
                                }
                                break;
                            case gcSL_UINT64:
                                switch (length)
                                {
                                CASE(1,ulong4);
                                CASE(2,ulong8);
                                CASE(4,ulong16);
                                }
                                break;
                            DEFAULT;
                            }
                            break;
                        DEFAULT;
                        }
                    }
                    break;
                }
            }
            else
            {
                gctCHAR* name = ((gcSHADER)(Kernel->masterInstance.binary))->typeNameBuffer + nameOffset;
                switch (GetFormatSpecialType(format))
                {
                case gcSL_STRUCT:
                    gcoOS_StrCopySafe(argument->typeName,ARG_TYPE_NAME_LENTH,"struct ");
                    break;
                case gcSL_UNION:
                    gcoOS_StrCopySafe(argument->typeName,ARG_TYPE_NAME_LENTH,"union ");
                    break;
                case gcSL_ENUM:
                    gcoOS_StrCopySafe(argument->typeName,ARG_TYPE_NAME_LENTH,"enum ");
                    break;
                }
                gcoOS_StrCatSafe(argument->typeName,ARG_TYPE_NAME_LENTH,name);
                if(isUniformPointer(argument->uniform))
                {
                    gcoOS_StrCatSafe(argument->typeName,ARG_TYPE_NAME_LENTH,"*");
                }
            }


        }
    }

    return gcvSTATUS_OK;

OnError:
    if (memAllocInfo)
    {
        gcmOS_SAFE_FREE(gcvNULL, memAllocInfo);
    }
    return status;
}

gctINT
clfDuplicateVIRKernelArgs(
    clsKernel_PTR   Kernel,
    clsSrcArgument_PTR * Arguments
    )
{
    gceSTATUS          status;
    gctPOINTER         pointer;
    gctSIZE_T          bytes;
    clsSrcArgument_PTR orgArgument, newArgument;
    gctUINT            i = 0;
    gctBOOL            acquired = gcvFALSE;

    if (Kernel->srcArgs == gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    if (Arguments == gcvNULL)
    {
        return CL_INVALID_VALUE;
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Allocate the array of clsArgument structures. */
    bytes = Kernel->kernelNumArgs * gcmSIZEOF(clsSrcArgument);
    clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);

    /* Copy the data. */
    gcoOS_MemCopy(pointer, Kernel->srcArgs, bytes);

    orgArgument = Kernel->srcArgs;
    newArgument = (clsSrcArgument_PTR)pointer;
    for (i = 0; i < Kernel->kernelNumArgs; i++, orgArgument++, newArgument++)
    {
        if (orgArgument->data)
        {
            gctSIZE_T bytes;

            if(orgArgument->isMemAlloc)
            {
                bytes = sizeof(clsMemAllocInfo);
            }
            else
            {
                bytes = orgArgument->size;
            }

            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &newArgument->data), CL_OUT_OF_HOST_MEMORY);
            gcoOS_MemCopy(newArgument->data, orgArgument->data, bytes);

            /* Retain memObj if used as kernel argument. */
            if (newArgument->isPointer || newArgument->isImage)
            {
                cl_mem memObj = *((cl_mem *) newArgument->data);
                /* Handle NULL buffer object */
                if (memObj)
                {
                    gcmASSERT(memObj->objectType == clvOBJECT_MEM);
                }
                newArgument->isDuplicate = gcvTRUE;
                clfRetainMemObject(memObj);
            }
        }
        else if(orgArgument->addressQualifier == CL_KERNEL_ARG_ADDRESS_LOCAL)
        {
            clsMemAllocInfo_PTR memAllocInfo;
            gctUINT32 bytes;

            gcmASSERT(orgArgument->data == gcvNULL);
            bytes = sizeof(clsMemAllocInfo);
            /* Allocate the memory allocation info. */
            status = gcoOS_Allocate(gcvNULL, bytes, &newArgument->data);
            gcoOS_ZeroMemory(newArgument->data, bytes);
            memAllocInfo                = (clsMemAllocInfo_PTR)newArgument->data;
            memAllocInfo->allocatedSize = orgArgument->size;
            newArgument->size           = bytes;
            newArgument->isLocal        = gcvTRUE;
        }
    }

    *Arguments = (clsSrcArgument_PTR)pointer;

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    acquired = gcvFALSE;

    return gcvSTATUS_OK;

OnError:
    if (pointer)
    {
        gcoOS_Free(gcvNULL, pointer);
        *Arguments = gcvNULL;
    }

    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }

    return status;
}

gctINT
clfDuplicateKernelArgs(
    clsKernel_PTR   Kernel,
    clsArgument_PTR *Arguments
    )
{
    gceSTATUS       status;
    gctPOINTER      pointer;
    gctSIZE_T       bytes;
    clsArgument_PTR orgArgument, newArgument;
    gctUINT         i = 0;
    gctBOOL         acquired = gcvFALSE;

    if (Kernel->args == gcvNULL)
    {
        return gcvSTATUS_OK;
    }

    if (Arguments == gcvNULL)
    {
        return CL_INVALID_VALUE;
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Allocate the array of clsArgument structures. */
    bytes = Kernel->numArgs * gcmSIZEOF(clsArgument);
    clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);

    /* Copy the data. */
    gcoOS_MemCopy(pointer, Kernel->args, bytes);

    orgArgument = Kernel->args;
    newArgument = pointer;
    for (i = 0; i < Kernel->numArgs; i++, orgArgument++, newArgument++)
    {
        if (orgArgument->data)
        {
            gctSIZE_T bytes;

            if (orgArgument->isMemAlloc)
            {
                bytes = sizeof(clsMemAllocInfo);
            }
            else
            {
                bytes = orgArgument->size;
            }

            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &newArgument->data), CL_OUT_OF_HOST_MEMORY);
            gcoOS_MemCopy(newArgument->data, orgArgument->data, bytes);

            /* Retain memObj if used as kernel argument. */
            if (newArgument->isPointer)
            {
                cl_mem memObj = *((cl_mem *) newArgument->data);
                /* Handle NULL buffer object */
                if (memObj)
                {
                    gcmASSERT(memObj->objectType == clvOBJECT_MEM);
                }
                newArgument->isMemObj = gcvTRUE;
                clfRetainMemObject(memObj);
            }
        }
    }

    *Arguments = pointer;

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    acquired = gcvFALSE;

    return gcvSTATUS_OK;

OnError:
    if (pointer)
    {
        gcoOS_Free(gcvNULL, pointer);
        *Arguments = gcvNULL;
    }

    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }

    return status;
}

gctINT
clfReallocateKernelArgs(
    gctUINT             OrgNumArgs,
    gctUINT             NewNumArgs,
    clsArgument_PTR *   Args
    )
{
    gceSTATUS           status;
    gctPOINTER          pointer;

    if (NewNumArgs < OrgNumArgs || Args == gcvNULL)
    {
        return CL_INVALID_VALUE;
    }

    if (*Args == gcvNULL && OrgNumArgs != 0)
    {
        return CL_INVALID_VALUE;
    }

    /* Allocate the array of clsArgument structures. */
    clmONERROR(gcoOS_Allocate(gcvNULL, NewNumArgs * gcmSIZEOF(clsArgument), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    /* Copy the data. */
    if (*Args)
    {
        gcoOS_MemCopy(pointer, *Args, OrgNumArgs * gcmSIZEOF(clsArgument));
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Args));
    }

    *Args = pointer;
    return gcvSTATUS_OK;

OnError:
    return status;
}

gctINT
clfReallocateKernelUinformArgs(
    gctUINT             OrgNumArgs,
    clsKernel_PTR       Kernel
    )
{
    gceSTATUS           status;
    gctPOINTER          pointer;
    clsArgument_PTR     argument;
    gctUINT32           i;
    gctUINT             NewNumArgs = Kernel->numArgs;
    clsArgument_PTR *   Args = &(Kernel->args);
    clsMemAllocInfo_PTR memAllocInfo = gcvNULL;

    if (NewNumArgs < OrgNumArgs || Args == gcvNULL)
    {
        return CL_INVALID_VALUE;
    }

    if (*Args == gcvNULL && OrgNumArgs != 0)
    {
        return CL_INVALID_VALUE;
    }

    /* Allocate the array of clsArgument structures. */
    clmONERROR(gcoOS_Allocate(gcvNULL, NewNumArgs * gcmSIZEOF(clsArgument), &pointer),
               CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, NewNumArgs * gcmSIZEOF(clsArgument));

    argument = pointer;
    /* Copy the data. */
    if (*Args)
    {
        gcoOS_MemCopy(pointer, *Args, OrgNumArgs * gcmSIZEOF(clsArgument));
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, *Args));
    }
    for (i = 0; i < NewNumArgs; i++, argument++)
    {
        gcUNIFORM uniform;
        gcSHADER_TYPE type;
        gcSL_FORMAT format;
        gctBOOL isPointer;
        gctUINT length;
        gctSIZE_T bytes;

        if(i < OrgNumArgs)
        {
            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->masterInstance.binary, i, &uniform), CL_INVALID_VALUE);
            if (!uniform) continue;
            argument->uniform    = uniform;
            continue;
        }

        clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->masterInstance.binary, i, &uniform), CL_INVALID_VALUE);

        if (!uniform) continue;
        clmONERROR(gcUNIFORM_GetType(uniform, &type, &length), CL_INVALID_VALUE);
        clmONERROR(gcUNIFORM_GetFormat(uniform, &format, &isPointer), CL_INVALID_VALUE);

        if (isUniformLocalAddressSpace(uniform) ||
            isUniformPrivateAddressSpace(uniform) ||
            isUniformConstantAddressSpace(uniform) ||
            isUniformKernelArgPrivate(uniform) ||
            isUniformKernelArgLocal(uniform) ||
            isUniformKernelArgLocalMemSize(uniform) ||
            isUniformPrintfAddress(uniform))
        {
            gctPOINTER pointer;

            bytes = sizeof(clsMemAllocInfo);

            /* Allocate the memory allocation info. */
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = (clsMemAllocInfo_PTR)pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetLocalMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->localMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetPrivateMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetConstantMemorySize((gcSHADER) Kernel->masterInstance.binary, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ), CL_INVALID_VALUE);
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;
            memAllocInfo = gcvNULL;
        }
        else
        {
            if (isPointer)
            {
                bytes = gcmSIZEOF(gctPOINTER);
            }
            else
            {
                switch (type)
                {
                case gcSHADER_FLOAT_X1:
                case gcSHADER_FLOAT16_X1:
                case gcSHADER_BOOLEAN_X1:
                case gcSHADER_INTEGER_X1:
                case gcSHADER_INT8_X1:
                case gcSHADER_INT16_X1:
                case gcSHADER_UINT_X1:
                case gcSHADER_UINT8_X1:
                case gcSHADER_UINT16_X1:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X2:
                case gcSHADER_FLOAT16_X2:
                case gcSHADER_BOOLEAN_X2:
                case gcSHADER_INTEGER_X2:
                case gcSHADER_INT8_X2:
                case gcSHADER_INT16_X2:
                case gcSHADER_UINT_X2:
                case gcSHADER_UINT8_X2:
                case gcSHADER_UINT16_X2:
                    bytes = 2 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X3:
                case gcSHADER_FLOAT16_X3:
                case gcSHADER_BOOLEAN_X3:
                case gcSHADER_INTEGER_X3:
                case gcSHADER_INT8_X3:
                case gcSHADER_INT16_X3:
                case gcSHADER_UINT_X3:
                case gcSHADER_UINT8_X3:
                case gcSHADER_UINT16_X3:
                    bytes = 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_X4:
                case gcSHADER_FLOAT16_X4:
                case gcSHADER_BOOLEAN_X4:
                case gcSHADER_INTEGER_X4:
                case gcSHADER_INT8_X4:
                case gcSHADER_INT16_X4:
                case gcSHADER_UINT_X4:
                case gcSHADER_UINT8_X4:
                case gcSHADER_UINT16_X4:
                    bytes = 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_INT64_X1:
                case gcSHADER_UINT64_X1:
                    bytes = 1 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X2:
                case gcSHADER_UINT64_X2:
                    bytes = 2 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X3:
                case gcSHADER_UINT64_X3:
                    bytes = 4 * gcmSIZEOF(cl_long) * length;
                    break;
                case gcSHADER_INT64_X4:
                case gcSHADER_UINT64_X4:
                    bytes = 4 * gcmSIZEOF(cl_long) * length;
                    break;

                case gcSHADER_FLOAT_2X2:
                    bytes = 2 * 2 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_3X3:
                    bytes = 3 * 3 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_FLOAT_4X4:
                    bytes = 4 * 4 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_IMAGE_2D_T:
                case gcSHADER_IMAGE_3D_T:
                case gcSHADER_SAMPLER_T:
                case gcSHADER_IMAGE_1D_T:
                case gcSHADER_IMAGE_1D_ARRAY_T:
                case gcSHADER_IMAGE_1D_BUFFER_T:
                case gcSHADER_IMAGE_2D_ARRAY_T:
                    bytes = 1 * gcmSIZEOF(cl_uint) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
                    break;

                case gcSHADER_INT8_P2:
                case gcSHADER_UINT8_P2:
                case gcSHADER_INT16_P2:
                case gcSHADER_UINT16_P2:
                    bytes = 1 * gcmSIZEOF(cl_int2) * length;
                    break;
                case gcSHADER_INT8_P3:
                case gcSHADER_UINT8_P3:
                case gcSHADER_INT16_P3:
                case gcSHADER_UINT16_P3:
                    bytes = 1 * gcmSIZEOF(cl_int3) * length;
                    break;
                case gcSHADER_INT8_P4:
                case gcSHADER_UINT8_P4:
                case gcSHADER_INT16_P4:
                case gcSHADER_UINT16_P4:
                    bytes = 1 * gcmSIZEOF(cl_int4) * length;
                    break;
                case gcSHADER_INT8_P8:
                case gcSHADER_UINT8_P8:
                case gcSHADER_INT16_P8:
                case gcSHADER_UINT16_P8:
                    bytes = 1 * gcmSIZEOF(cl_int8) * length;
                    break;
                case gcSHADER_INT8_P16:
                case gcSHADER_UINT8_P16:
                case gcSHADER_INT16_P16:
                case gcSHADER_UINT16_P16:
                    bytes = 1 * gcmSIZEOF(cl_int16) * length;
                    break;
                case gcSHADER_FLOAT16_P2:
                    bytes = 1 * gcmSIZEOF(cl_float2) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P3:
                    bytes = 1 * gcmSIZEOF(cl_float3) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P4:
                    bytes = 1 * gcmSIZEOF(cl_float4) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P8:
                    bytes = 1 * gcmSIZEOF(cl_float8) * length >> 1;
                    break;
                case gcSHADER_FLOAT16_P16:
                    bytes = 1 * gcmSIZEOF(cl_float16) * length >> 1;
                    break;

                default:
                    clmASSERT("Unknown shader type", CL_INVALID_VALUE);
                    bytes = 0;
                }

                switch (format)
                {
                case gcSL_INT8:
                case gcSL_UINT8:
                    bytes /= 4;
                    break;

                case gcSL_INT16:
                case gcSL_UINT16:
                    bytes /= 2;
                    break;

                default:
                    break;
                }
            }

            /* Allocate the data array. */
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);

            gcoOS_ZeroMemory(argument->data, bytes);

            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvFALSE;
            argument->isPointer  = isPointer;
        }
    }

    *Args = pointer;
    return gcvSTATUS_OK;

OnError:
    if (pointer)
    {
        gcoOS_Free(gcvNULL, pointer);
    }
    if (memAllocInfo)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, memAllocInfo));
    }
    return status;
}


gctINT
clfFreeVIRKernelArgs(
    gctUINT            NumArgs,
    clsSrcArgument_PTR Args,
    gctUINT            localKernelArgSize, /* local kernel arg should be related with each NDRange only */
    gctBOOL            FreePrivateKernelArg
    )
{
    gctUINT         i;

    if (Args == gcvNULL || NumArgs == 0)
    {
        return gcvSTATUS_OK;
    }

    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].data)
        {
            if (Args[i].isDuplicate)
            {
                cl_mem memObj = *((cl_mem *) Args[i].data);
                /* Handle NULL buffer object */
                if (memObj)
                {
                    gcmASSERT(memObj->objectType == clvOBJECT_MEM);
                }
                clfReleaseMemObject(memObj);
            }
            else if(Args[i].isMemAlloc)
            {
                clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Args[i].data;

                gcoCL_FreeMemory(memAllocInfo->physical,
                                 memAllocInfo->logical,
                                 memAllocInfo->allocatedSize,
                                 memAllocInfo->node,
                                 gcvSURF_INDEX);

                if (FreePrivateKernelArg && memAllocInfo->data)
                {
                    gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data);
                }
            }
            else if(Args[i].isLocal)
            {
                clsMemAllocInfo_PTR MemObj = (clsMemAllocInfo_PTR) Args[i].data;

                gcoCL_FreeMemory(MemObj->physical,
                                MemObj->logical,
                                localKernelArgSize,
                                MemObj->node,
                                gcvSURF_INDEX);
            }
            gcmOS_SAFE_FREE(gcvNULL, Args[i].data);
        }
    }

    gcmOS_SAFE_FREE(gcvNULL, Args);

    return gcvSTATUS_OK;
}

gctINT
clfFreeKernelArgs(
    gctUINT         NumArgs,
    clsArgument_PTR Args,
    gctBOOL         FreeAllocData
    )
{
    gctUINT         i;

    if (Args == gcvNULL || NumArgs == 0)
    {
        return gcvSTATUS_OK;
    }

    for (i = 0; i < NumArgs; i++)
    {
        if (Args[i].isMemAlloc)
        {
            clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Args[i].data;
            if (!isUniformPrivateAddressSpace(Args[i].uniform))
            {

                gcoCL_FreeMemory(memAllocInfo->physical,

                             memAllocInfo->logical,

                             memAllocInfo->allocatedSize,

                             memAllocInfo->node,

                             gcvSURF_INDEX);
            }
            if (FreeAllocData && memAllocInfo->data)
            {
                gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data);
            }
        }
        if (Args[i].data)
        {
            if (Args[i].isMemObj)
            {
                cl_mem memObj = *((cl_mem *) Args[i].data);
                /* Handle NULL buffer object */
                if (memObj)
                {
                    gcmASSERT(memObj->objectType == clvOBJECT_MEM);
                }
                clfReleaseMemObject(memObj);
            }
            gcmOS_SAFE_FREE(gcvNULL, Args[i].data);
        }
    }
    gcmOS_SAFE_FREE(gcvNULL, Args);

    return gcvSTATUS_OK;
}

clsArgument_PTR
clfGetKernelArg(
    clsKernel_PTR   Kernel,
    gctUINT         Index,
    gctBOOL *       isLocal,
    gctBOOL *       isPrivate,
    gctBOOL *       isSampler
    )
{
    clsArgument_PTR     arg;
    gctUINT             i, argIndex = 0;

    for (i = 0; i < Kernel->numArgs; i++)
    {
        arg = &Kernel->args[i];
        if (arg->uniform == gcvNULL) continue;
        if (! hasUniformKernelArgKind(arg->uniform)) continue;
        if (argIndex == Index)
        {
            if (isLocal) *isLocal = isUniformKernelArgLocal(arg->uniform);
            if (isPrivate) *isPrivate = isUniformKernelArgPrivate(arg->uniform);
            if (isSampler) *isSampler = isUniformKernelArgSampler(arg->uniform);
            return arg;
        }
        argIndex++;
    }
    return gcvNULL;
}

gctUINT
clfGetKernelNumArg(
    clsKernel_PTR   Kernel
    )
{
    clsArgument_PTR     arg;
    gctUINT             i, numArg = 0;

    for (i = 0; i < Kernel->numArgs; i++)
    {
        arg = &Kernel->args[i];
        if (arg->uniform == gcvNULL) continue;
        if (! hasUniformKernelArgKind(arg->uniform)) continue;
        numArg++;
    }
    return numArg;
}


void doPatchCreateKernel(cl_program Program, cl_kernel Kernel, gctSTRING KernelName );
static gceSTATUS
clfBuildKernelInfos(
    cl_program      Program,
    clsKernel_PTR   kernel,
    gcSHADER shader)
{
    gceCHIPMODEL    chipModel = gcv200;
    gctUINT32       chipRevision = 0;
    gceSTATUS       status = gcvSTATUS_OK;

    if(gcShaderHasInt64(shader))
        kernel->patchNeeded = gcvTRUE;

    chipModel = kernel->context->devices[0]->deviceInfo.chipModel;
    chipRevision = kernel->context->devices[0]->deviceInfo.chipRevision;

    if(kernel->masterInstance.programState.hints)
    {
        gctUINT workGroupSize;

        gcmONERROR(gcSHADER_GetWorkGroupSize(shader, &workGroupSize));
        kernel->maxWorkGroupSize = (size_t)workGroupSize;

        /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
        if (kernel->maxWorkGroupSize > Program->devices[0]->deviceInfo.maxWorkGroupSize)
        {
            kernel->maxWorkGroupSize = Program->devices[0]->deviceInfo.maxWorkGroupSize;
        }

        if((chipModel == gcv4000) && (chipRevision == 0x5245)
            && (Program->context->platform->patchId == gcvPATCH_OCLCTS))
        {
             kernel->maxWorkGroupSize = gcmMIN(kernel->maxWorkGroupSize, 480);
        }
    }

    /* Get the number of uniforms. */
    gcmVERIFY_OK(gcSHADER_GetKernelUniformCount(shader, &kernel->numArgs));

    /* Allocate kernel arguments. */
    gcmONERROR(clfBuildKernelArgs(kernel));

    kernel->kernelNumArgs = clfGetKernelNumArg(kernel);

OnError:
    return status;
}

static gceSTATUS
clfCopyGCShader(gcSHADER src, gcSHADER * dst)
{
    gctUINT         binarySize;
    gctPOINTER      pointer     = gcvNULL;
    gctUINT32_PTR   comVersion;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmONERROR(gcSHADER_SaveEx(src, gcvNULL, &binarySize));
    gcmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer));
    gcmONERROR(gcSHADER_SaveEx(src, pointer, &binarySize));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, dst));

    gcmONERROR(gcSHADER_GetCompilerVersion(src,
                                           &comVersion));

    gcmONERROR(gcSHADER_SetCompilerVersion(*dst,
                                           comVersion));

    /* Load kernel binary from program binary */
    status = gcSHADER_LoadEx(*dst, pointer, binarySize);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007003: (clCreateKernel) Cannot extract kernel from program.\n");
        gcmONERROR(status);
    }

OnError:
    if (pointer)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
    }

    return status;
}

static gceSTATUS
clfLoadAndLinkGCShader(
    cl_program      Program,
    clsKernel_PTR   kernel
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcsPROGRAM_STATE programState = {0};
    gcSHADER        pgmBinary, kernelBinary = gcvNULL;
    gcSHADER        tmpBinary = gcvNULL;
    gctUINT         tmpBinarySize = 0;
    gctPOINTER      savedTmpBinary = gcvNULL;
    gctUINT         binarySize;
    gctUINT         i,j;
    gctUINT         count;
    gctUINT         gpuCount;
    gctINT          propertyType = 0;
    gctINT          propertyValues[3] = {0};
    gceSHADER_FLAGS flags;
    gcKERNEL_FUNCTION kernelFunction;
    gcePATCH_ID     patchId = gcvPATCH_INVALID;
    gctBOOL         supportImageInst = Program->context->devices[0]->deviceInfo.supportIMGInstr;
    gctUINT32_PTR   comVersion;
    gctPOINTER      pointer     = gcvNULL;

    doPatchCreateKernel(Program, kernel,kernel->name);
    /* Save program binary into buffer */
    pgmBinary = (gcSHADER) Program->binary;
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize));
    gcmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer));
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize));

    /* Construct kernel binary. */
    gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));

    gcmONERROR(gcSHADER_GetCompilerVersion(pgmBinary,
                                           &comVersion));

    gcmONERROR(gcSHADER_SetCompilerVersion(kernelBinary,
                                           comVersion));

    /* Load kernel binary from program binary */
    status = gcSHADER_LoadEx(kernelBinary, pointer, binarySize);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007003: (clCreateKernel) Cannot extract kernel from program.\n");
        gcmONERROR(status);
    }
    gcmOS_SAFE_FREE(gcvNULL, pointer);

    /* Load kernel binary uniforms with the given kernel name */
    status = gcSHADER_LoadKernel(kernelBinary, kernel->name);

    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007004: (clCreateKernel) Cannot extract kernel (%s) from program.\n",
            kernel->name);
        gcmONERROR(status);
    }

    /* Set the required work group size. */
    gcmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, kernel->name, &kernelFunction));
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, gcvNULL, &propertyType, propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            for (j = 0; j < 3; j++)
            {
                kernel->compileWorkGroupSize[j] = (size_t)propertyValues[j];
            }
        }
    }

    /* Assume all dead code is removed by optimizer. */
    flags = gcvSHADER_RESOURCE_USAGE /*| gcvSHADER_DEAD_CODE*/ | gcvSHADER_OPTIMIZER;

    /* Check if patching is needed. */
    {
        static gctBOOL      firstTime = gcvTRUE;
        gcKERNEL_FUNCTION   kernelFunction = gcvNULL;

        /* Set kernelFunction. */
        for (i = 0; i < GetShaderKernelFunctionCount(kernelBinary); i++)
        {
            kernelFunction = GetShaderKernelFunction(kernelBinary, i);
            if (kernelFunction && gcmIS_SUCCESS(gcoOS_StrCmp(GetKFunctionName(kernelFunction), kernel->name)))
            {
                gcmASSERT(gcmIS_SUCCESS(gcoOS_StrCmp(GetKFunctionName(kernelFunction), kernel->name)));
                break;
            }
        }
        gcmASSERT(kernelFunction);

        patchId = Program->context->platform->patchId;
        if(patchId == gcvPATCH_OCLCTS)
        {
            if (GetKFunctionISamplerCount(kernelFunction))
            {
                /* Delete the old imageSamplers. */
                SetKFunctionISamplerCount(kernelFunction, 0);

                /* Patching is needed. */
                kernel->patchNeeded = gcvTRUE;
                flags |= gcSHADER_HAS_IMAGE_IN_KERNEL;
            }
            else
            {
                gcSHADER_TYPE type = 0;

                for (i = 0; i < GetShaderUniformCount(kernelBinary); i++)
                {
                    if (GetShaderUniform(kernelBinary, i) == gcvNULL)
                    {
                        continue;
                    }

                    type = GetUniformType(GetShaderUniform(kernelBinary, i));
                    if (isOCLImageType(type))
                    {
                        break;
                    }
                }

                if (i < GetShaderUniformCount(kernelBinary))
                {
                    /* Patching is needed. */
                    kernel->patchNeeded = gcvTRUE;
                    flags |= gcSHADER_HAS_IMAGE_IN_KERNEL;
                    /* TODO - Check for IMAGE_WR. */
                }
            }
        }
        /* Check imageSampler. */
        else if (GetKFunctionISamplerCount(kernelFunction))
        {
            if(supportImageInst == gcvTRUE)
            {
                gctUINT count = GetKFunctionISamplerCount(kernelFunction);

                kernel->patchNeeded = gcvFALSE;

                for (i = 0; i < count; i++)
                {
                    gcsIMAGE_SAMPLER_PTR    imageSampler;
                    gctUINT                 samplerValue;

                    imageSampler = GetKFunctionISamplers(kernelFunction) + i;

                    if (GetImageSamplerIsConstantSamplerType(imageSampler))
                    {
                        samplerValue = GetImageSamplerType(imageSampler);

                        if (samplerValue & CLK_NORMALIZED_COORDS_TRUE)
                        {
                            kernel->patchNeeded = gcvTRUE;
                            break;
                        }
                    }
                }
            }
            else
            {
                /* Delete the old imageSamplers. */
                SetKFunctionISamplerCount(kernelFunction, 0);

                /* Patching is needed. */
                kernel->patchNeeded = gcvTRUE;
            }
            flags |= gcSHADER_HAS_IMAGE_IN_KERNEL;
        }
        else
        {
            gcSHADER_TYPE type = 0;

            for (i = 0; i < GetShaderUniformCount(kernelBinary); i++)
            {
                if (GetShaderUniform(kernelBinary, i) == gcvNULL)
                {
                    continue;
                }

                type = GetUniformType(GetShaderUniform(kernelBinary, i));
                if (isOCLImageType(type))
                {
                    break;
                }
            }

            if (i < GetShaderUniformCount(kernelBinary))
            {
                if((supportImageInst == gcvTRUE) && (type == gcSHADER_IMAGE_2D_T || type == gcSHADER_IMAGE_1D_T))
                {
                    kernel->patchNeeded = gcvFALSE;
                }
                else
                {
                    /* Patching is needed. */
                    kernel->patchNeeded = gcvTRUE;
                }
                flags |= gcSHADER_HAS_IMAGE_IN_KERNEL;
                /* TODO - Check for IMAGE_WR. */
            }
        }

        if (kernel->patchNeeded)
        {
            flags |= gcvSHADER_IMAGE_PATCHING;

            if (firstTime)
            {
                firstTime = gcvFALSE;
                status = gcLoadCLPatchLibrary(gcvNULL, 0);
                if (gcmIS_ERROR(status))
                {
                    /* TODO - Patching is not allowed. */
                }
            }
        }
    }

    gcmASSERT(kernel->context->platform->compiler11);
    gcSetCLCompiler(kernel->context->platform->compiler11);

    gcmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &tmpBinarySize));
    gcmONERROR(gcoOS_Allocate(gcvNULL, tmpBinarySize, &savedTmpBinary));
    gcmONERROR(gcSHADER_SaveEx(pgmBinary, savedTmpBinary, &tmpBinarySize));

    gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
    gcmONERROR(gcoCL_GetHWConfigGpuCount(&gpuCount));

    if(gpuCount > 1)
    {
        flags |= gcvSHADER_ENABLE_MULTI_GPU;
    }

    status = gcLinkKernel(kernelBinary,
                          flags | gcvSHADER_REMOVE_UNUSED_UNIFORMS,
                          &programState);

    if((status == gcvSTATUS_NOT_FOUND || status == gcvSTATUS_OUT_OF_RESOURCES) &&  gcmOPT_INLINELEVEL() != 4)
    {
        gcmASSERT(savedTmpBinary);

        /* Construct kernel binary. */
        if(tmpBinary == gcvNULL)
            gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &tmpBinary));

        /* Load kernel binary from program binary */
        status = gcSHADER_LoadEx(tmpBinary, savedTmpBinary, tmpBinarySize);
        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        /* Load temp binary uniforms with the given kernel name */
        gcmONERROR(gcSHADER_LoadKernel(tmpBinary, kernel->name));

        gcoOS_Free(gcvNULL, savedTmpBinary);
        savedTmpBinary = gcvNULL;
        tmpBinarySize = 0;
        gcFreeProgramState(programState);

        status = gcLinkKernel(tmpBinary,
                flags | gcvSHADER_REMOVE_UNUSED_UNIFORMS | gcvSHADER_SET_INLINE_LEVEL_4,
                &programState);

        if (gcmIS_ERROR(status))
        {
            gcFreeProgramState(programState);
            gcSHADER_Destroy(tmpBinary);
            gcmONERROR(status);
        }

        gcSHADER_Destroy(kernelBinary);
        kernelBinary = tmpBinary;
    }
    else
    {
        gcoOS_Free(gcvNULL, savedTmpBinary);
        savedTmpBinary = gcvNULL;
    }

    if(status == gcvSTATUS_OUT_OF_SAMPLER)
    {
        kernel->maxWorkGroupSize = kernel->preferredWorkGroupSizeMultiple;
        goto OnSkipOutOfSampler;
    }

    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007005: (clCreateKernel) Cannot link kernel (%s).\n",
            kernel->name);
        gcmONERROR(status);
    }

OnSkipOutOfSampler:


    kernel->masterInstance.binary          = (gctUINT8_PTR) kernelBinary;
    kernel->masterInstance.programState    = programState;

    gcmONERROR(clfBuildKernelInfos(Program, kernel, kernelBinary));

    return gcvSTATUS_OK;

OnError:
    if(savedTmpBinary)
    {
        gcoOS_Free(gcvNULL, savedTmpBinary);
        savedTmpBinary = gcvNULL;
    }

    if(kernelBinary)
    {
        gcSHADER_Destroy(kernelBinary);
    }

    if (kernel)
    {
        if (kernel->args)
        {
            /* Free prior data and arguments. */
            for (i = 0; i < kernel->numArgs; i++)
            {
                if (kernel->args[i].data)
                {
                    gcmOS_SAFE_FREE(gcvNULL, kernel->args[i].data);
                }
            }
            gcmOS_SAFE_FREE(gcvNULL, kernel->args);
        }
    }

    if(pointer != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
    }

    return status;
}

/* Allocate and collect kernel arg info, these info will not change, we use these info to return APP's query
   All recompile special code need in recompile program instance or keep in NDRange inside only (dimension/global size, etc)
   So, for anything could make different by recompile or NDRange argument, we need consider how to get the right return of
   these argument (i.e, maxWorkGroupCount)
   PS: the difference betweent his to clfBuildKernelArgs is, we only include what source args, all other info collected at KEP
       generated, not put here.
*/
gceSTATUS
clfBuildVIRKernelArgs(
    clsKernel_PTR   Kernel
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gctPOINTER      pointer = gcvNULL;
    gctSIZE_T       bytes;
    clsSrcArgument_PTR argument;
    gctUINT         i;
    KERNEL_EXECUTABLE_PROFILE * kep = &Kernel->virMasterInstance->kep;
    PROG_CL_ARG_ENTRY * virArg;

    /* Get the number of uniforms. */
    Kernel->kernelNumArgs = kep->argTable.countOfEntries;
    Kernel->hasPrintf = kep->kernelHints.hasPrintf;

    if(Kernel->kernelNumArgs > 0)
    {
        /* Allocate the array of arguments. */
        bytes = Kernel->kernelNumArgs* gcmSIZEOF(clsSrcArgument);
        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
        gcoOS_ZeroMemory(pointer, bytes);

        Kernel->srcArgs = (clsSrcArgument_PTR)pointer;

        argument = (clsSrcArgument_PTR)pointer;

        for (i = 0; i < Kernel->kernelNumArgs; i++, argument++)
        {
            virArg = &kep->argTable.pArgsEntries[i];

            argument->size       = 0;
            argument->set        = gcvFALSE;
            argument->data       = gcvNULL;
            argument->argIndex   = i;
            argument->type       = virArg->type;

            /*TO_DO, build arg type name */
            /*(argument->typeName = virArg->type + virArg->typeNameOffset)*/
            argument->isPointer = virArg->isPointer;
            argument->isSampler = virArg->isSampler;
            argument->isImage   = virArg->isImage;
            argument->name      = virArg->argName;
            gcoOS_StrCopySafe(argument->typeName,ARG_TYPE_NAME_LENTH, virArg->argTypeName);

            switch (virArg->addressQualifier) {
            case VIR_AS_CONSTANT:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
                break;
            case VIR_AS_LOCAL:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
                break;
            case VIR_AS_PRIVATE:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                if(argument->isPointer)
                {
                    clsMemAllocInfo_PTR memAllocInfo;
                    gctPOINTER pointer;

                    bytes = sizeof(clsMemAllocInfo);
                    /* Allocate the memory allocation info. */
                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer));
                    gcoOS_ZeroMemory(pointer, bytes);
                    memAllocInfo = (clsMemAllocInfo_PTR)pointer;
                    argument->data = memAllocInfo;
                    argument->isMemAlloc = gcvTRUE;
                    argument->isPointer = gcvFALSE;
                }
                break;
            case VIR_AS_GLOBAL:
                if (argument->isPointer || argument->isImage)
                {
                    argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
                }
                else
                {
                    argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                }
                break;
            default:
                argument->addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            }

            if ((virArg->accessQualifier & VIR_TYQUAL_READ_ONLY) &&
                (virArg->accessQualifier & VIR_TYQUAL_WRITE_ONLY))
            {
                argument->accessQualifier  = CL_KERNEL_ARG_ACCESS_READ_WRITE;
            }
            else if (virArg->accessQualifier & VIR_TYQUAL_READ_ONLY)
            {
                argument->accessQualifier  = CL_KERNEL_ARG_ACCESS_READ_ONLY;
            }
            else if (virArg->accessQualifier & VIR_TYQUAL_WRITE_ONLY)
            {
                argument->accessQualifier  = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
            }
            else
            {
                argument->accessQualifier  = CL_KERNEL_ARG_ACCESS_NONE;
            }

            argument->typeQualifier = CL_KERNEL_ARG_TYPE_NONE;
            if (virArg->typeQualifier & VIR_TYQUAL_CONST)
            {
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
            }
            if (virArg->typeQualifier & VIR_TYQUAL_RESTRICT)
            {
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
            }
            if (virArg->typeQualifier & VIR_TYQUAL_VOLATILE)
            {
                argument->typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
            }
        }
    }

OnError:
    return status;
}

static gceSTATUS
clfBuildVIRKernelInfos(
    cl_program      Program,
    clsKernel_PTR   kernel
    )
{
    gceCHIPMODEL    chipModel = gcv200;
    gctUINT32       chipRevision = 0;
    gceSTATUS       status = gcvSTATUS_OK;
    KERNEL_EXECUTABLE_PROFILE * kep = &kernel->virCurrentInstance->kep;
    SHADER_EXECUTABLE_PROFILE * sep = &kep->sep;
    gctUINT         i,j;

    chipModel = kernel->context->devices[0]->deviceInfo.chipModel;
    chipRevision = kernel->context->devices[0]->deviceInfo.chipRevision;

    kernel->maxWorkGroupSize = (size_t)kep->sep.exeHints.nativeHints.prvStates.gps.calculatedWorkGroupSize;

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > Program->devices[0]->deviceInfo.maxWorkGroupSize)
    {
        kernel->maxWorkGroupSize = Program->devices[0]->deviceInfo.maxWorkGroupSize;
    }

    if((chipModel == gcv4000) && (chipRevision == 0x5245)
        && (Program->context->platform->patchId == gcvPATCH_OCLCTS))
    {
        kernel->maxWorkGroupSize = gcmMIN(kernel->maxWorkGroupSize, 480);
    }

    for (i = 0; i < gcvPROPERTY_COUNT; i++)
    {
        if (kep->kernelHints.property[i].type == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            for (j = 0 ; j < kep->kernelHints.property[i].size; j++)
            {
                kernel->compileWorkGroupSize[j] = (size_t)(kep->kernelHints.property[i].value[j]);
            }
        }
    }

    /* Allocate kernel arguments. */
    gcmONERROR(clfBuildVIRKernelArgs(kernel));

    for(i = 0; i < sep->staticPrivMapping.privConstantMapping.countOfEntries; i++)
    {
        SHADER_PRIV_CONSTANT_ENTRY * privEntry = &sep->staticPrivMapping.privConstantMapping.pPrivmConstantEntries[i];
        if(privEntry->commonPrivm.privmKind == SHS_PRIV_CONSTANT_KIND_CONSTANT_ADDRESS_SPACE)
        {
            gctPOINTER pointer = gcvNULL;
            clsMemAllocInfo_PTR memAllocInfo;
            gcmASSERT(kep->kernelHints.constantMemorySize > 0);
            /* Allocate the memory allocation info. */
            gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsMemAllocInfo), &pointer));
            gcoOS_ZeroMemory(pointer, sizeof(clsMemAllocInfo));
            privEntry->commonPrivm.pPrivateData = pointer;
            memAllocInfo =  (clsMemAllocInfo_PTR) privEntry->commonPrivm.pPrivateData;
            memAllocInfo->allocatedSize = kep->kernelHints.constantMemorySize;
            /* Allocate the physical buffer */
            clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->node,
                                            gcvSURF_INDEX,
                                            0),
                        CL_INVALID_VALUE);
            /* Copy the constant data to the buffer */
            gcoOS_MemCopy(memAllocInfo->logical, kep->kernelHints.constantMemBuffer, kep->kernelHints.constantMemorySize);
            break;
        }
    }

OnError:
    return status;
}

static gctBOOL
clfGetMasterKeyState(
    cl_kernel Kernel,
    gctUINT   * kernelKeyData,
    gctUINT   * kernelKeySize
    )
{
    gctUINT                   instanceKeyState = 0;
    gctUINT i;
    KERNEL_EXECUTABLE_PROFILE * kep = &Kernel->virMasterInstance->kep;
    PROG_CL_IMAGE_TABLE_ENTRY * imageEntry;
    VSC_HW_CONFIG             * pHwCfg = &Kernel->context->platform->vscCoreSysCtx.hwCfg;
    gctUINT                   instanceSamplerVaue=0;
    gctBOOL                   isImageRead = gcvFALSE;
    PROG_CL_ARG_ENTRY         * pArgsEntry;

    /* Get Pair from texLD table and Image table */
    for (i = 0; i < kep->resourceTable.imageTable.countOfEntries; i++)
    {
        imageEntry = &kep->resourceTable.imageTable.pImageEntries[i];
        pArgsEntry = &kep->argTable.pArgsEntries[imageEntry->imageArgIndex];

        if(pArgsEntry->typeQualifier & VIR_TYQUAL_READ_ONLY)
        {
            isImageRead = gcvTRUE;
            if (imageEntry->kernelHardcodeSampler)
            {
                instanceSamplerVaue = imageEntry->constSamplerValue;
            }
            else
            {
                instanceSamplerVaue = imageEntry->assumedSamplerValue;
            }
        }
        else if(pArgsEntry->typeQualifier & VIR_TYQUAL_WRITE_ONLY)
        {
            isImageRead = gcvFALSE;
        }

        if(isImageRead)
        {
            vscImageSamplerNeedLibFuncForHWCfg(&imageEntry->imageDesc, instanceSamplerVaue, pHwCfg,gcvNULL, gcvNULL, &instanceKeyState);
        }
        else
        {
            vscImageWriteNeedLibFuncForHWCfg(&imageEntry->imageDesc, pHwCfg, gcvNULL, &instanceKeyState);
        }

        kernelKeyData[*kernelKeySize] = instanceKeyState;
        *kernelKeySize += 1;
    }
    return (instanceKeyState != 0);
}

static clsVIRInstanceHashRec_PTR
clfCreateVirInstanceHash(
    gctUINT tbEntryNum,
    gctUINT maxEntryObjs
    )
{
    clsVIRInstanceHashRec_PTR pHash = gcvNULL;
    gctPOINTER                pointer = gcvNULL;
    gcmHEADER_ARG("tbEntryNum=%d maxEntryObjs=%d",
                   tbEntryNum, maxEntryObjs);

    gcoOS_Allocate(gcvNULL, sizeof(clsVIRInstanceHashRec), &pointer);
    gcoOS_ZeroMemory(pointer, sizeof(clsVIRInstanceHashRec));
    pHash = (clsVIRInstanceHashRec_PTR)pointer;
    gcmASSERT(pHash != gcvNULL);

    if(pHash == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%x", pHash);
        return gcvNULL;
    }

    pHash->tbEntryNum = tbEntryNum;
    pHash->maxEntryObjs = maxEntryObjs;
    pHash->year = 0;

    gcoOS_Allocate(gcvNULL, tbEntryNum * sizeof(clsVIRInstanceKey_PTR), &pointer);
    gcoOS_ZeroMemory(pointer, tbEntryNum * sizeof(clsVIRInstanceKey_PTR));
    pHash->ppHashTable = (clsVIRInstanceKey_PTR *)pointer;
    gcoOS_Allocate(gcvNULL, tbEntryNum * sizeof(gctUINT), &pointer);
    gcoOS_ZeroMemory(pointer, tbEntryNum * sizeof(gctUINT));
    pHash->pEntryCounts = (gctUINT *)pointer;
    gcmASSERT(pHash->ppHashTable && pHash->pEntryCounts);

    gcmFOOTER_ARG("return=0x%x", pHash);
    return pHash;
}

clsVIRInstanceKey_PTR
clfAddInstanceKeyToHashTable(
    clsVIRInstanceHashRec_PTR pHash,
    clsKernelVIRInstance * pVIRInstance,
    gctUINT key
    )
{
    clsVIRInstanceKey_PTR pNewObj = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctUINT entryId = key & (pHash->tbEntryNum - 1);

    gcmHEADER_ARG("pHash=0x%x pVIRInstance=0x%x key=%u", pHash, pVIRInstance, key);

    gcoOS_Allocate(gcvNULL, sizeof(clsVIRInstanceKey), &pointer);
    pNewObj = (clsVIRInstanceKey_PTR )pointer;
    gcmASSERT(pNewObj);
    if (pNewObj == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    pNewObj->virInstance = pVIRInstance;
    pNewObj->key = key;
    pNewObj->year = pHash->year++;

    /* Check if objects of this hash slot exceeds */
    if (++pHash->pEntryCounts[entryId] > pHash->maxEntryObjs)
    {
        gctUINT earliestYear = 0xFFFFFFFF;
        clsVIRInstanceKey_PTR pOldestObj = gcvNULL;
        clsVIRInstanceKey_PTR pObj = pHash->ppHashTable[entryId];
        while (pObj)
        {
            /* Found oldest object except perpetual ones */
            if (earliestYear > pObj->year)
            {
                earliestYear = pObj->year;
                pOldestObj = pObj;
            }

            pObj = pObj->nextInstanceKey;
        }
        clfDeleteHashInstanceKey(pHash, pOldestObj);
    }

    pNewObj->nextInstanceKey = pHash->ppHashTable[entryId];
    pHash->ppHashTable[entryId] = pNewObj;

    gcmFOOTER_ARG("return=0x%x", pNewObj);
    return pNewObj;
}

static gceSTATUS
clfLoadAndLinkVIRShader(
    cl_program      Program,
    clsKernel_PTR   kernel
    )
{
    gceSTATUS       status = gcvSTATUS_OK;
    gcSHADER        pgmBinary = gcvNULL;
    gcSHADER        kernelBinary = gcvNULL;
    SHADER_HANDLE   virShader   = gcvNULL;
    clsPlatformId_PTR platform = kernel->context->platform;
    VSC_SHADER_COMPILER_PARAM  vscCompileParams;
    clsKernelVIRInstance *  masterInstance = gcvNULL;
    SHADER_HANDLE   localShader = gcvNULL;
    gcSHADER        Shaders[gcMAX_SHADERS_IN_LINK_GOURP] = {0, 0, 0, 0, 0, 0};
    gctUINT         keyStateData[MAX_KEY_DATA_SIZE] = {0};
    gctUINT         keyStateSize = 0;
    gctUINT32       gpuCount = 1;

    /* Save program binary into buffer */
    pgmBinary = (gcSHADER) Program->binary;

    gcmONERROR(clfCopyGCShader(pgmBinary, &kernelBinary));

    /* Load kernel binary uniforms with the given kernel name */
    status = gcSHADER_LoadKernel(kernelBinary, kernel->name);

    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007004: (clCreateKernel) Cannot extract kernel (%s) from program.\n",
            kernel->name);
        gcmONERROR(status);
    }

    /* TODO: move to vscLinkBuiltinLibs when ready */
    Shaders[gceSGSK_CL_SHADER] = kernelBinary;
    gcmASSERT(kernel->context->platform->compiler11);
    gcSetCLCompiler(kernel->context->platform->compiler11);
    gcmVERIFY_OK(gcSHADER_LinkBuiltinLibs(Shaders));
    /* Pack shader */
    gcmONERROR(gcSHADER_Pack(kernelBinary));
    gcmVERIFY_OK(gcSHADER_PackRegister(kernelBinary));
    gcmONERROR(vscConvertGcShader2VirShader(kernelBinary, &virShader));

    vscCopyShader(&localShader, virShader);

    gcmONERROR(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));
    gcmONERROR(gcoCL_QueryDeviceCount(gcvNULL, &gpuCount));
    gcoOS_ZeroMemory(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENCL;
    vscCompileParams.cfg.ctx.appNameId = gcvPATCH_INVALID;
    vscCompileParams.cfg.ctx.isPatchLib = gcvFALSE;
    vscCompileParams.cfg.ctx.pSysCtx = &platform->vscSysCtx;
    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC
                                | VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS
                                | VSC_COMPILER_FLAG_COMPILE_CODE_GEN;
    if(gpuCount > 1)
    {
         vscCompileParams.cfg.cFlags  |= VSC_COMPILER_FLAG_ENABLE_MULTI_GPU;
    }

    vscCompileParams.cfg.optFlags = VSC_COMPILER_OPT_FULL;
    vscCompileParams.hShader = localShader;
    vscCompileParams.pInMasterSEP = gcvNULL;
    vscCompileParams.pShLibLinkTable = gcvNULL;
    vscCompileParams.pShResourceLayout = gcvNULL;

    gcSetCLCompiler(kernel->context->platform->compiler11);

    gcmONERROR(clfAllocateVIRKernelInstance(&masterInstance));

    masterInstance->platform = platform;
    kernel->virMasterInstance = masterInstance;
    kernel->virCurrentInstance = masterInstance;

    gcmONERROR(vscCreateKernel(&vscCompileParams, &kernel->virMasterInstance->kep, &kernel->virMasterInstance->hwStates));

    gcmONERROR(clfBuildVIRKernelInfos(Program, kernel));


    if(localShader)
    {
        vscDestroyShader(localShader);
        localShader = gcvNULL;
    }

    if(kernel->virCacheTable == gcvNULL)
    {
        kernel->virCacheTable = clfCreateVirInstanceHash(__CL_INSTANCE_HASH_ENTRY_NUM, __CL_INSTANCE_HASH_ENTRY_SIZE);
    }
    /* Get the master's hash key */
    if(clfGetMasterKeyState(kernel, keyStateData, &keyStateSize))
    {
        /* calculate the master's hash key of the kernel */
        kernel->virMasterInstance->hashKey = clfEvaluateCRC32(keyStateData, keyStateSize*4);
    }

    kernel->shaderHandle = (SHADER_HANDLE)virShader;
    kernel->localMemSize = kernel->virMasterInstance->kep.kernelHints.localMemorySize;

    if (kernelBinary)
    {
        gcSHADER_Destroy(kernelBinary);
        kernelBinary = gcvNULL;
    }

    return gcvSTATUS_OK;

OnError:
    if (masterInstance)
    {
        clfFreeVIRKernelInstance(masterInstance);
        kernel->virMasterInstance = gcvNULL;
    }

    if (virShader)
    {
        vscDestroyShader(virShader);
    }

    if (kernelBinary)
    {
        gcSHADER_Destroy(kernelBinary);
        kernelBinary = gcvNULL;
    }

    return status;
}

/*****************************************************************************\
|*                       OpenCL Kernel Object API                            *|
\*****************************************************************************/
CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(
    cl_program      Program,
    const char *    KernelName,
    cl_int *        ErrcodeRet
    )
{
    clsKernel_PTR   kernel      = gcvNULL;
    gctPOINTER      pointer     = gcvNULL;
    gctSIZE_T       length;
    gceSTATUS       status;

    gcmHEADER_ARG("Program=0x%x KernelName=%s",
                  Program, KernelName);
    gcmDUMP_API("${OCL clCreateKernel 0x%x, %s}", Program, KernelName);
    VCL_TRACE_API(CreateKernel_Pre)(Program, KernelName, ErrcodeRet);

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007000: (clCreateKernel) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }
    if (Program->binary == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007001: (clCreateKernel) invalid program binary.  Maybe the program is not built yet.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
    }

    if (KernelName == gcvNULL || *KernelName == '\0')
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007002: (clCreateKernel) KerelName is NULL or empty string.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Allocate kernel. */
    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsKernel), &pointer), CL_OUT_OF_HOST_MEMORY);

    gcoOS_ZeroMemory(pointer, sizeof(clsKernel));

    kernel                    = (clsKernel_PTR) pointer;
    kernel->dispatch          = Program->dispatch;
    kernel->objectType        = clvOBJECT_KERNEL;
    kernel->program           = Program;
    kernel->context           = Program->context;
    kernel->numArgs           = 0;
    kernel->args              = gcvNULL;
    kernel->compileWorkGroupSize[0] = 0;
    kernel->compileWorkGroupSize[1] = 0;
    kernel->compileWorkGroupSize[2] = 0;
    kernel->localMemSize      = 0;
    kernel->privateMemSize    = 0;
    kernel->constantMemSize   = 0;
    kernel->constantMemBuffer = gcvNULL;
    kernel->preferredWorkGroupSizeMultiple = Program->devices[0]->deviceInfo.ShaderCoreCount * 4;
    kernel->patchNeeded       = gcvTRUE;    /* Force to check patch instructions for the moment. May hurt compiler performance. */
    kernel->masterInstance.binary     = gcvNULL;
    kernel->isPatched         = gcvFALSE;
    kernel->linkedDebugInfo   = gcvNULL;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &kernel->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, kernel->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&kernel->id), CL_INVALID_VALUE);

    clfRetainProgram(Program);
    clfRetainContext(kernel->context);

    /* Copy kernel name */
    length = gcoOS_StrLen(KernelName, gcvNULL) + 1;
    clmONERROR(gcoOS_Allocate(gcvNULL, length, &pointer), CL_OUT_OF_HOST_MEMORY);
    gcmVERIFY_OK(gcoOS_StrCopySafe(pointer, length, KernelName));
    kernel->name = (gctSTRING) pointer;

    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &kernel->cacheMutex),
               CL_OUT_OF_HOST_MEMORY);

    if (kernel->context->platform->virShaderPath)
    {
        clmONERROR(clfLoadAndLinkVIRShader(Program, kernel), CL_OUT_OF_HOST_MEMORY);
    }
    else
    {
        clmONERROR(clfLoadAndLinkGCShader(Program, kernel), CL_OUT_OF_HOST_MEMORY);
    }

    /* Create thread lock mutex for argument setting/using. */
    clmONERROR(gcoOS_CreateMutex(gcvNULL,
                                 &kernel->argMutex),
               CL_OUT_OF_HOST_MEMORY);

    if (ErrcodeRet)
    {
        *ErrcodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(CreateKernel_Post)(Program, KernelName, ErrcodeRet, kernel);
    gcmFOOTER_ARG("%d kernel=%lu",
                  CL_SUCCESS, kernel);
    return kernel;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007006: (clCreateKernel) cannot create kernel.  Maybe run out of memory.\n");
    }

    if (ErrcodeRet)
    {
        *ErrcodeRet = status;
    }

    if (kernel)
    {
        if(kernel->referenceCount) gcoOS_AtomDestroy(gcvNULL, kernel->referenceCount);

        if(kernel->name) kernel->name = gcvNULL;

        if(kernel->argMutex) gcoOS_DeleteMutex(gcvNULL, kernel->argMutex);

        if(kernel->cacheMutex) gcoOS_DeleteMutex(gcvNULL, kernel->cacheMutex);

        gcmOS_SAFE_FREE(gcvNULL, kernel);
    }

    if(pointer != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
    }

    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clCreateKernelsInProgram(
    cl_program     Program,
    cl_uint        NumKernels,
    cl_kernel *    Kernels,
    cl_uint *      NumKernelsRet
    )
{
    gceSTATUS       status;
    gctUINT         i;
    gctUINT         numKernels, kernelCount=0;
    gcSHADER        binary;
    gcKERNEL_FUNCTION kernelFunction;
    gctCONST_STRING kernelName;

    gcmHEADER_ARG("Program=0x%x NumKernels=%d Kernels=0x%x",
                  Program, NumKernels, Kernels);

    gcmDUMP_API("${OCL clCreateKernelsInProgram 0x%x, %d}", Program, NumKernels);

    if (Program == gcvNULL || Program->objectType != clvOBJECT_PROGRAM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007007: (clCreateKernelsInProgram) invalid Program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM);
    }

    if (Program->binary == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007008: (clCreateKernelsInProgram) invalid program binary.  Maybe the program is not built yet.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
    }

    binary = (gcSHADER)Program->binary;

    /* Get the number of kernel functions */
    gcmVERIFY_OK(gcSHADER_GetKernelFunctionCount(binary, &numKernels));

    if (numKernels < 1)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007009: (clCreateKernelsInProgram) no kernel function in program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
    }

    if (Kernels != gcvNULL && NumKernels < numKernels)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007010: (clCreateKernelsInProgram) NumKernels (%d) is less than the number (%d) of kernels in program.\n",
            NumKernels, numKernels);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (NumKernelsRet != gcvNULL)
    {
        *NumKernelsRet = numKernels;
    }

    kernelCount = Kernels ? numKernels : NumKernels;
    for (i = 0; i < kernelCount && Kernels != gcvNULL; i++)
    {
        gcmVERIFY_OK(gcSHADER_GetKernelFunction(binary, i, &kernelFunction));
        gcmVERIFY_OK(gcKERNEL_FUNCTION_GetName(kernelFunction, gcvNULL, &kernelName));
        Kernels[i] = clCreateKernel(Program, kernelName, (cl_int *)&status);
        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(status);
        }

#if cldTUNING
        if (gcmIS_SUCCESS(gcoOS_StrCmp(kernelName, "violaJones")))
        {
            Program->context->sortRects = gcvTRUE;
        }
#endif
    }

    VCL_TRACE_API(CreateKernelsInProgram)(Program, NumKernels, Kernels, NumKernelsRet);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(
    cl_kernel    Kernel
    )
{
    gctINT  status;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);
    gcmDUMP_API("${OCL clRetainKernel 0x%x}", Kernel);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007011: (clRetainKernel) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    clfONERROR(clfRetainKernel(Kernel));

    VCL_TRACE_API(RetainKernel)(Kernel);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(
    cl_kernel   Kernel
    )
{
    gctINT          status;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);
    gcmDUMP_API("${OCL clReleaseKernel 0x%x}", Kernel);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007012: (clReleaseKernel) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    clfONERROR(clfReleaseKernel(Kernel));

    VCL_TRACE_API(ReleaseKernel)(Kernel);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

static gceSTATUS
clfRecompileSetKernelArg(
    cl_kernel    Kernel,
    cl_uint      ArgIndex,
    clsSampler_PTR sampler,
    clsArgument_PTR * argument
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if(Kernel->context->devices[0]->deviceInfo.supportIMGInstr &&
        (Kernel->patchNeeded == gcvFALSE))
    {
        gcKERNEL_FUNCTION   kernelFunction = gcvNULL;

        {
            gctUINT32           orgArgNum = 0, binarySize = 0;
            gctPOINTER          pointer;
            gcsPROGRAM_STATE    programState = {0};
            gctUINT32           i;
            gcSHADER            pgmBinary, kernelBinary;
            gctUINT32_PTR       comVersion;

            Kernel->patchNeeded = gcvTRUE;
            /* Save program binary into buffer */
            pgmBinary = (gcSHADER) Kernel->program->binary;
            gcmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize));
            gcmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer));
            gcmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize));

            /* Construct kernel binary. */
            gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary));
            gcmONERROR(gcSHADER_GetCompilerVersion(pgmBinary, &comVersion));
            gcmONERROR(gcSHADER_SetCompilerVersion(kernelBinary, comVersion));

            /* Load kernel binary from program binary */
            status = gcSHADER_LoadEx(kernelBinary, pointer, binarySize);
            if (gcmIS_ERROR(status))
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-007003: (clCreateKernel) Cannot extract kernel from program.\n");
                gcmONERROR(status);
            }
            gcoOS_Free(gcvNULL, pointer);

            /* Load kernel binary uniforms with the given kernel name */
            status = gcSHADER_LoadKernel(kernelBinary, Kernel->name);
            if (gcmIS_ERROR(status))
            {
                gcmONERROR(status);
            }

            /* Set kernelFunction. */
            for (i = 0; i < GetShaderKernelFunctionCount(kernelBinary); i++)
            {
                kernelFunction = GetShaderKernelFunction(kernelBinary, i);
                if (kernelFunction && GetKFunctionIsMain(kernelFunction))
                {
                    gcmASSERT(gcmIS_SUCCESS(gcoOS_StrCmp(GetKFunctionName(kernelFunction), Kernel->name)));
                    break;
                }
            }
            gcmASSERT(kernelFunction);

            /* Check imageSampler. */
            if (GetKFunctionISamplerCount(kernelFunction))
            {
                /* Delete the old imageSamplers. */
                SetKFunctionISamplerCount(kernelFunction, 0);
            }

            gcmVERIFY_OK(gcSHADER_GetKernelUniformCount((gcSHADER)(Kernel->masterInstance.binary), &orgArgNum));

            gcmASSERT(Kernel->context->platform->compiler11);
            gcSetCLCompiler(Kernel->context->platform->compiler11);
            status = gcLinkKernel(kernelBinary,
                gcvSHADER_RESOURCE_USAGE | gcvSHADER_OPTIMIZER | gcvSHADER_IMAGE_PATCHING | gcvSHADER_REMOVE_UNUSED_UNIFORMS | gcSHADER_HAS_IMAGE_IN_KERNEL,
                &programState);
            gcmONERROR(status);

            if(Kernel->masterInstance.binary)
                gcoOS_Free(gcvNULL, Kernel->masterInstance.binary);

            gcFreeProgramState(Kernel->masterInstance.programState);

            Kernel->masterInstance.binary          = (gctUINT8_PTR) kernelBinary;
            Kernel->masterInstance.programState    = programState;

            gcmVERIFY_OK(gcSHADER_GetKernelUniformCount((gcSHADER)(Kernel->masterInstance.binary), &Kernel->numArgs));
            /* Allocate kernel arguments. */
            gcmONERROR(clfReallocateKernelUinformArgs(orgArgNum, Kernel));

            *argument = clfGetKernelArg(Kernel, ArgIndex, gcvNULL, gcvNULL, gcvNULL);
        }
    }

OnError:
    return status;
}

gceSTATUS
clfRecompileVIRKernel(
    cl_kernel Kernel
    )
{
    VSC_SHADER_COMPILER_PARAM  vscCompileParams;
    clsKernelVIRInstance *  instance = gcvNULL;
    VSC_SHADER_LIB_LINK_TABLE linkTable;
    VSC_SHADER_LIB_LINK_ENTRY linkEntry = {0};
    clsSrcArgument_PTR arg;
    gctUINT i;
    gctUINT imageIndex = 0, samplerIndex = 0;
    clsPlatformId_PTR platform = Kernel->context->platform;
    gceSTATUS status = gcvSTATUS_OK;
    KERNEL_EXECUTABLE_PROFILE * kep = &Kernel->virMasterInstance->kep;
    gctPOINTER ptrImage = gcvNULL;
    gctPOINTER ptrSampler = gcvNULL;
    SHADER_HANDLE localShader = gcvNULL;

    vscCopyShader(&localShader, (SHADER_HANDLE)Kernel->shaderHandle);

    gcoOS_ZeroMemory(&vscCompileParams, sizeof(VSC_SHADER_COMPILER_PARAM));
    vscCompileParams.cfg.ctx.clientAPI = gcvAPI_OPENCL;
    vscCompileParams.cfg.ctx.appNameId = gcvPATCH_INVALID;
    vscCompileParams.cfg.ctx.isPatchLib = gcvFALSE;
    vscCompileParams.cfg.ctx.pSysCtx = &platform->vscSysCtx;
    vscCompileParams.cfg.cFlags = VSC_COMPILER_FLAG_COMPILE_TO_ML
                                | VSC_COMPILER_FLAG_UNI_SAMPLER_UNIFIED_ALLOC
                                | VSC_COMPILER_FLAG_COMPILE_FULL_LEVELS
                                | VSC_COMPILER_FLAG_COMPILE_CODE_GEN;

    vscCompileParams.cfg.optFlags = VSC_COMPILER_OPT_FULL;
    vscCompileParams.hShader = localShader;
    vscCompileParams.pInMasterSEP = gcvNULL;
    vscCompileParams.pShLibLinkTable = &linkTable;
    linkTable.shLinkEntryCount = 1;
    linkTable.pShLibLinkEntries = &linkEntry;

    linkEntry.linkPointCount = 1;
    linkEntry.linkPoint[0].libLinkType = VSC_LIB_LINK_TYPE_IMAGE_READ_WRITE;
    linkEntry.linkPoint[0].u.imageReadWrite.imageCount = kep->kernelHints.imageCount;
    linkEntry.linkPoint[0].u.imageReadWrite.samplerCount = kep->kernelHints.samplerCount;

    if(kep->kernelHints.imageCount)
    {
        gcoOS_Allocate(gcvNULL, kep->kernelHints.imageCount * gcmSIZEOF(VSC_IMAGE_DESC_INFO), &ptrImage);
        gcoOS_ZeroMemory(ptrImage, kep->kernelHints.imageCount * gcmSIZEOF(VSC_IMAGE_DESC_INFO));
        linkEntry.linkPoint[0].u.imageReadWrite.imageInfo = (VSC_IMAGE_DESC_INFO *)ptrImage;
    }

    if (kep->kernelHints.samplerCount)
    {
        /* sampler count may be 0 */
        gcoOS_Allocate(gcvNULL, kep->kernelHints.samplerCount * gcmSIZEOF(VSC_SAMPLER_INFO), &ptrSampler);
        gcoOS_ZeroMemory(ptrSampler, kep->kernelHints.samplerCount * gcmSIZEOF(VSC_SAMPLER_INFO));
        linkEntry.linkPoint[0].u.imageReadWrite.samplerInfo = (VSC_SAMPLER_INFO *)ptrSampler;
    }

    for (i = 0; i < Kernel->kernelNumArgs; i++)
    {
        arg = &Kernel->srcArgs[i];

        if (arg->isImage)
        {
            clsMem_PTR image;

            image = *(clsMem_PTR *)arg->data;
            linkEntry.linkPoint[0].u.imageReadWrite.imageInfo[imageIndex].kernelArgNo = arg->argIndex;
            linkEntry.linkPoint[0].u.imageReadWrite.imageInfo[imageIndex].imageDesc = image->u.image.imageDescriptor;
            imageIndex++;
        }
        else if (arg->isSampler)
        {
            clsSampler_PTR sampler = *(clsSampler_PTR *) arg->data;
            gctINT data = sampler->samplerValue;

            gcmASSERT(samplerIndex < kep->kernelHints.samplerCount);

            linkEntry.linkPoint[0].u.imageReadWrite.samplerInfo[samplerIndex].kernelArgNo = arg->argIndex;
            linkEntry.linkPoint[0].u.imageReadWrite.samplerInfo[samplerIndex].sampleValue = data;
            samplerIndex++;
        }
    }

    vscCompileParams.pShResourceLayout = gcvNULL;

    gcSetCLCompiler(Kernel->context->platform->compiler11);

    gcmONERROR(clfAllocateVIRKernelInstance(&instance));

    instance->platform = platform;

    Kernel->virCurrentInstance = instance;

    gcmONERROR(vscCreateKernel(&vscCompileParams, &Kernel->virCurrentInstance->kep, &Kernel->virCurrentInstance->hwStates));



    vscDestroyShader(localShader);

    if(kep->kernelHints.imageCount)
    {
        gcmOS_SAFE_FREE(gcvNULL, ptrImage);
    }

    if (kep->kernelHints.samplerCount)
    {
        gcmOS_SAFE_FREE(gcvNULL, ptrSampler);
    }

OnError:
    return status;
}

cl_int clfSetVIRKernelArg(
    cl_kernel    Kernel,
    cl_uint      ArgIndex,
    size_t       ArgSize,
    const void * ArgValue
    )
{
    clsSrcArgument_PTR argument;
    gctINT          status;
    gctBOOL         acquired = gcvFALSE;

    if (ArgIndex > Kernel->kernelNumArgs)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007014: (clSetKernelArg) ArgIndex (%d) is larger than the number (%d) of kernel arguments.\n",
            ArgIndex, Kernel->numArgs);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
    acquired = gcvTRUE;

    argument = &Kernel->srcArgs[ArgIndex];
    if (argument == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007015: (clSetKernelArg) cannot get argument with ArgIndex (%d).\n",
            ArgIndex);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    /* according to spec1.2, If the argument is declared with the __local qualifier, the arg_value entry must beNULL. */
    if (argument->addressQualifier == CL_KERNEL_ARG_ADDRESS_LOCAL)
    {
        argument->size      = ArgSize;
        argument->data      = gcvNULL;
        argument->isPointer = gcvFALSE;
        Kernel->localMemSize += ArgSize;
    }
    else if((argument->addressQualifier == CL_KERNEL_ARG_ADDRESS_PRIVATE) && argument->isMemAlloc)
    {
        clsMemAllocInfo_PTR memAllocInfo;
        gctPOINTER pointer;

        memAllocInfo = (clsMemAllocInfo_PTR) argument->data;
        memAllocInfo->allocatedSize = ArgSize;
        argument->size = ArgSize;

        if(memAllocInfo->data)
        {
            gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data);
        }

        status = gcoOS_Allocate(gcvNULL, ArgSize, &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007018: (clSetKernelArg) Run out of memory when allocating memory with size (%d).\n",
                ArgSize);
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        memAllocInfo->data = pointer;

        gcoOS_MemCopy(memAllocInfo->data, ArgValue, ArgSize);
    }
    else
    {

        if (argument->size > 0)
        {
            gcoOS_Free(gcvNULL, argument->data);
            argument->size = 0;
            argument->data = gcvNULL;
        }

        argument->size = ArgSize;

        if (ArgSize > 0 && ArgValue)
        {
            status = gcoOS_Allocate(gcvNULL, ArgSize, &argument->data);
            if (gcmIS_ERROR(status))
            {
                clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
            }

            gcoOS_MemCopy(argument->data, ArgValue, ArgSize);
        }
    }

    argument->set = gcvTRUE;

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    acquired = gcvFALSE;

    return CL_SUCCESS;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }

    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(
    cl_kernel    Kernel,
    cl_uint      ArgIndex,
    size_t       ArgSize,
    const void * ArgValue
    )
{
    clsArgument_PTR argument;
    gctINT          status;
    gctBOOL         isLocal, isPrivate, isSampler;
    gctBOOL         acquired = gcvFALSE;

    gcmHEADER_ARG("Kernel=0x%x ArgIndex=%u ArgSize=%u ArgValue=0x%x",
                  Kernel, ArgIndex, ArgSize, ArgValue);
    gcmDUMP_API("${OCL clSetKernelArg 0x%x, %d}", Kernel, ArgIndex);
    VCL_TRACE_API(SetKernelArg)(Kernel, ArgIndex, ArgSize, ArgValue);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007013: (clSetKernelArg) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (Kernel->context->platform->virShaderPath)
    {
        status = clfSetVIRKernelArg(Kernel, ArgIndex, ArgSize, ArgValue);

        gcmFOOTER_ARG("%d", status);
        return status;
    }

    if (ArgIndex > Kernel->numArgs)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007014: (clSetKernelArg) ArgIndex (%d) is larger than the number (%d) of kernel arguments.\n",
            ArgIndex, Kernel->numArgs);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    gcmVERIFY_OK(gcoOS_AcquireMutex(gcvNULL, Kernel->argMutex, gcvINFINITE));
    acquired = gcvTRUE;

    argument = clfGetKernelArg(Kernel, ArgIndex, &isLocal, &isPrivate, &isSampler);

    if (argument == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007015: (clSetKernelArg) cannot get argument with ArgIndex (%d).\n",
            ArgIndex);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    if (isLocal)
    {
        if (ArgSize == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007016: (clSetKernelArg) argument (%d) is local, but ArgSize is 0.\n",
                ArgIndex);
            clmRETURN_ERROR(CL_INVALID_ARG_SIZE);
        }
        clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);
        ((clsMemAllocInfo_PTR)argument->data)->allocatedSize = ArgSize;
        argument->size = ArgSize;
        Kernel->localMemSize += ArgSize;
    }
    else if (isPrivate)
    {
        clsMemAllocInfo_PTR memAllocInfo;
        gctPOINTER pointer;

        if (ArgSize == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007017: (clSetKernelArg) argument (%d) is private, but ArgSize is 0.\n",
                ArgIndex);
            clmRETURN_ERROR(CL_INVALID_ARG_SIZE);
        }
        clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

        memAllocInfo = (clsMemAllocInfo_PTR) argument->data;
        memAllocInfo->allocatedSize = ArgSize;
        argument->size = ArgSize;

        status = gcoOS_Allocate(gcvNULL, ArgSize, &pointer);
        if (gcmIS_ERROR(status))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007018: (clSetKernelArg) Run out of memory when allocating memory with size (%d).\n",
                ArgSize);
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        memAllocInfo->data = pointer;

        gcoOS_MemCopy(memAllocInfo->data, ArgValue, ArgSize);
    }
    else if (isSampler)
    {
        clsSampler_PTR sampler = *(clsSampler_PTR *) ArgValue;
        gctINT * data = (gctINT *) &sampler->samplerValue;

        clmONERROR(clfRecompileSetKernelArg(Kernel, ArgIndex, sampler, &argument), CL_OUT_OF_HOST_MEMORY);

        if (ArgSize != sizeof(cl_sampler))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007019: (clSetKernelArg) argument (%d) is a sampler, but ArgSize (%d) is not sizeof(cl_sampler).\n",
                ArgIndex, ArgSize);
            clmRETURN_ERROR(CL_INVALID_ARG_SIZE);
        }
        /* TODO - Need to handle array of samplers. */
        gcmASSERT(argument->size == 4);
        gcoOS_MemCopy(argument->data, data, argument->size);
    }
    else
    {
        clmCHECK_ERROR(ArgSize != argument->size, CL_INVALID_ARG_SIZE);
        gcoOS_MemCopy(argument->data, ArgValue, ArgSize);
    }

    argument->set = gcvTRUE;

    gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    acquired = gcvFALSE;

    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (acquired)
    {
        gcmVERIFY_OK(gcoOS_ReleaseMutex(gcvNULL, Kernel->argMutex));
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelInfo(
    cl_kernel       Kernel,
    cl_kernel_info  ParamName,
    size_t          ParamValueSize,
    void *          ParamValue,
    size_t *        ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;
    gctUINT          numArg;
    gctINT32         referenceCount;

    gcmHEADER_ARG("Kernel=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Kernel, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetKernelInfo 0x%x, 0x%x}", Kernel, ParamName);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007020: (clGetKernelInfo) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    switch (ParamName)
    {
    case CL_KERNEL_NUM_ARGS:
        numArg = Kernel->kernelNumArgs;
        retParamSize = gcmSIZEOF(numArg);
        retParamPtr = &numArg;
        break;

    case CL_KERNEL_REFERENCE_COUNT:
        gcmVERIFY_OK(gcoOS_AtomGet(gcvNULL, Kernel->referenceCount, &referenceCount));
        retParamSize = gcmSIZEOF(referenceCount);
        retParamPtr = &referenceCount;
        break;

    case CL_KERNEL_CONTEXT:
        retParamSize = gcmSIZEOF(Kernel->context);
        retParamPtr = &Kernel->context;
        break;

    case CL_KERNEL_PROGRAM:
        retParamSize = gcmSIZEOF(Kernel->program);
        retParamPtr = &Kernel->program;
        break;

    case CL_KERNEL_FUNCTION_NAME:
        if (Kernel->name != gcvNULL)
        {
            retParamSize = gcoOS_StrLen(Kernel->name, gcvNULL) + 1;
            retParamPtr = Kernel->name;
        }
        else
        {
            retParamSize = 1;
            retParamPtr = clgEmptyStr;
        }
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007021: (clGetKernelInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007022: (clGetKernelInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetKernelInfo)(Kernel, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

cl_int
clfGetKernelSrcArgInfo(
    cl_kernel        Kernel,
    cl_uint          ArgIndx,
    cl_kernel_arg_info   ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    clsSrcArgument_PTR  argument;
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007027: (clGetKernelArgInfo) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (ArgIndx > Kernel->kernelNumArgs)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007028: (clGetKernelArgInfo) ArgIndex (%d) is larger than the number (%d) of kernel arguments.\n",
            ArgIndx, Kernel->numArgs);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    argument = &Kernel->srcArgs[ArgIndx];

    switch (ParamName)  /* Todo, the argument value is no correct */
    {
    case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_address_qualifier);
        retParamPtr = &argument->addressQualifier;
        break;

    case CL_KERNEL_ARG_ACCESS_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_access_qualifier);
        retParamPtr = &argument->accessQualifier;
        break;

    case CL_KERNEL_ARG_TYPE_NAME:
        retParamSize = gcoOS_StrLen(argument->typeName,gcvNULL)+1;
        retParamPtr = argument->typeName;
        break;

    case CL_KERNEL_ARG_TYPE_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_type_qualifier);
        retParamPtr = &argument->typeQualifier;
        break;

    case CL_KERNEL_ARG_NAME:
        retParamSize = gcoOS_StrLen(argument->name, gcvNULL) + 1;
        retParamPtr = argument->name;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007029: (clGetKernelArgInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007030: (clGetKernelArgInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
        else
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007031: (clGetKernelArgInfo) Param (0x%x) is not available for kernel.\n",
                ParamName);
            clmRETURN_ERROR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    return CL_SUCCESS;
OnError:
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelArgInfo(
    cl_kernel        Kernel,
    cl_uint          ArgIndx,
    cl_kernel_arg_info   ParamName,
    size_t           ParamValueSize,
    void *           ParamValue,
    size_t *         ParamValueSizeRet
    )
{
    clsArgument_PTR  argument;
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;

    gcmHEADER_ARG("Kernel=0x%x ArgIndx=%u ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Kernel, ArgIndx, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetKernelInfo 0x%x, %d, 0x%x}", Kernel, ArgIndx, ParamName);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007027: (clGetKernelArgInfo) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (Kernel->context->platform->virShaderPath)
    {
        status = clfGetKernelSrcArgInfo(Kernel,
                                        ArgIndx,
                                        ParamName,
                                        ParamValueSize,
                                        ParamValue,
                                        ParamValueSizeRet);

        VCL_TRACE_API(GetKernelArgInfo)(Kernel, ArgIndx, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
        gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                      status, gcmOPT_VALUE(ParamValueSizeRet));
        return status;
    }

    if (ArgIndx > Kernel->numArgs)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007028: (clGetKernelArgInfo) ArgIndex (%d) is larger than the number (%d) of kernel arguments.\n",
            ArgIndx, Kernel->numArgs);
        clmRETURN_ERROR(CL_INVALID_ARG_INDEX);
    }

    argument = clfGetKernelArg(Kernel, ArgIndx, gcvNULL, gcvNULL, gcvNULL);

    switch (ParamName)  /* Todo, the argument value is no correct */
    {
    case CL_KERNEL_ARG_ADDRESS_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_address_qualifier);
        retParamPtr = &argument->addressQualifier;
        break;

    case CL_KERNEL_ARG_ACCESS_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_access_qualifier);
        retParamPtr = &argument->accessQualifier;
        break;

    case CL_KERNEL_ARG_TYPE_NAME:
        retParamSize = gcoOS_StrLen(argument->typeName,gcvNULL)+1;
        retParamPtr = argument->typeName;
        break;

    case CL_KERNEL_ARG_TYPE_QUALIFIER:
        retParamSize = gcmSIZEOF(cl_kernel_arg_type_qualifier);
        retParamPtr = &argument->typeQualifier;
        break;

    case CL_KERNEL_ARG_NAME:
        retParamSize = gcoOS_StrLen(argument->uniform->name, gcvNULL) + 1;
        retParamPtr = argument->uniform->name;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007029: (clGetKernelArgInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007030: (clGetKernelArgInfo) ParamValueSize (%d) is less than required size (%d).\n",
                ParamValueSize, retParamSize);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (retParamSize)
        {
            gcoOS_MemCopy(ParamValue, retParamPtr, retParamSize);
        }
        else
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007031: (clGetKernelArgInfo) Param (0x%x) is not available for kernel.\n",
                ParamName);
            clmRETURN_ERROR(CL_KERNEL_ARG_INFO_NOT_AVAILABLE);
        }
    }

    if (ParamValueSizeRet)
    {
        *ParamValueSizeRet = retParamSize;
    }

    VCL_TRACE_API(GetKernelArgInfo)(Kernel, ArgIndx, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;
OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetKernelWorkGroupInfo(
    cl_kernel                  Kernel,
    cl_device_id               Device,
    cl_kernel_work_group_info  ParamName,
    size_t                     ParamValueSize,
    void *                     ParamValue,
    size_t *                   ParamValueSizeRet
    )
{
    gctSIZE_T        retParamSize = 0;
    gctPOINTER       retParamPtr = NULL;
    gctINT           status;

    gcmHEADER_ARG("Kernel=0x%x Device=0x%x ParamName=%u ParamValueSize=%lu ParamValue=0x%x",
                  Kernel, Device, ParamName, ParamValueSize, ParamValue);
    gcmDUMP_API("${OCL clGetKernelWorkGroupInfo 0x%x, 0x%x, 0x%x}", Kernel, Device, ParamName);

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007023: (clGetKernelWorkGroupInfo) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (Device == gcvNULL || Device->objectType != clvOBJECT_DEVICE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007024: (clGetKernelWorkGroupInfo) invalid Device.\n");
        clmRETURN_ERROR(CL_INVALID_DEVICE);
    }

    switch (ParamName)
    {
    case CL_KERNEL_WORK_GROUP_SIZE:
        retParamSize = gcmSIZEOF(Kernel->maxWorkGroupSize);
        retParamPtr = &Kernel->maxWorkGroupSize;
        break;

    case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
        retParamSize = gcmSIZEOF(Kernel->compileWorkGroupSize);
        retParamPtr = &Kernel->compileWorkGroupSize;
        break;

    case CL_KERNEL_LOCAL_MEM_SIZE:
        retParamSize = gcmSIZEOF(Kernel->localMemSize);
        retParamPtr = &Kernel->localMemSize;
        break;

    case CL_KERNEL_PRIVATE_MEM_SIZE:
        retParamSize = gcmSIZEOF(Kernel->privateMemSize);
        retParamPtr = &Kernel->privateMemSize;
        break;

    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
        retParamSize = gcmSIZEOF(Kernel->preferredWorkGroupSizeMultiple);
        retParamPtr = &Kernel->preferredWorkGroupSizeMultiple;
        break;

    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007025: (clGetKernelWorkGroupInfo) invalid ParamName (0x%x).\n",
            ParamName);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ParamValue)
    {
        if (ParamValueSize < retParamSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-007026: (clGetKernelWorkGroupInfo) ParamValueSize (%d) is less than required size (%d).\n",
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

    VCL_TRACE_API(GetKernelWorkGroupInfo)(Kernel, Device, ParamName, ParamValueSize, ParamValue, ParamValueSizeRet);
    gcmFOOTER_ARG("%d *ParamValueSizeRet=%lu",
                  CL_SUCCESS, gcmOPT_VALUE(ParamValueSizeRet));
    return CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}


typedef gctBOOL (*PatchKernelHandler)(cl_kernel Kernel, gctSTRING KernelName);

static gctBOOL CheckKernelName(gctSTRING KernelName, gctCONST_STRING *EncryptedName, gctINT num)
{
    gctINT    i = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR   *p = gcvNULL;
    gctSTRING EncrpytedKernelName = gcvNULL;
    gctSIZE_T len = gcoOS_StrLen(KernelName, gcvNULL) + 1;

    gcmONERROR(gcoOS_Allocate(gcvNULL, len, (gctPOINTER*)&EncrpytedKernelName));
    gcoOS_ZeroMemory(EncrpytedKernelName, len);
    gcmONERROR(gcoOS_StrCopySafe(EncrpytedKernelName, len, KernelName));

    p = EncrpytedKernelName;
    while (*p)
    {
        *p = ~(*p);
        p++;
    }

    status = gcvSTATUS_NOT_FOUND;
    for (i = 0; i < num; i++)
    {
        if (0 == gcoOS_MemCmp(EncrpytedKernelName, EncryptedName[i], len))
        {
            status = gcvSTATUS_OK;
            break;
        }
    }

OnError:
    if (EncrpytedKernelName)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)EncrpytedKernelName);
    }

    return status == gcvSTATUS_OK;
}


static gctBOOL clPatchOpenCVAtomic_HoughLine(cl_kernel Kernel, gctSTRING KernelName)
{
    static gctCONST_STRING names[] =
    {

        /* fill_accum_global */
        "\x99\x96\x93\x93\xa0\x9e\x9c\x9c\x8a\x92\xa0\x98\x93\x90\x9d\x9e\x93",

    };

    return CheckKernelName(KernelName, names, gcmSIZEOF(names) / gcmSIZEOF(names[0]));
}

static gctBOOL clPatchOpenCVAtomic_clAHE(cl_kernel Kernel, gctSTRING KernelName)
{
    static gctCONST_STRING names[] =
    {
        /* calcLut */
        "\x9c\x9e\x93\x9c\xb3\x8a\x8b",

    };

    return CheckKernelName(KernelName, names, gcmSIZEOF(names) / gcmSIZEOF(names[0]));
}

typedef struct cldPATCH_
{
    gctBOOL            encrypted;
    gctCONST_STRING    matchEncryptedStr;
    PatchKernelHandler handleKernel;
    gcePATCH_ID        patchID;       /* the patchID only set when matchEncrpytedStr match with program' Source and handleKenel is True*/
} cldPATCH;

cldPATCH cl_patch_array[ ] =
{
    {
        gcvFALSE,
        "",
        gcvNULL,
        gcvPATCH_INVALID
    }, /*index 0 is reserved */
    {
        gcvTRUE,
        "\xa0\xff\x94\xf1\x83\xed\x88\xe4\x92\xfd\x94\xf0\x96\xff\x93\xff"
        "\xa0\xc1\xa2\xc1\xb4\xd9\x86\xe1\x8d\xe2\x80\xe1\x8d\xa5\xfa\xa5"
        "\xc2\xae\xc1\xa3\xc2\xae\xcd\xa2\xcc\xbf\xcb\xbe\xdd\xb5\xd4\xa6"
        "\x8c\xe0\x89\xfa\x8e\xd1\xa1\xd5\xa7\x8b\xe2\x8c\xf8\x94\xfd\x8e"
        "\xfa\xa5\xd6\xa2\xc7\xb7\x9b\xf2\x9c\xe8\x84\xed\x9e\xea\xb5\xda"
        "\xbc\xda\xa9\xcc\xb8\x94\xcb\x94\xf3\x9f\xf0\x92\xf3\x9f\xea\x89"
        "\xe1\x80\xf2\xd8\xb9\xda\xb9\xcc\xa1\xfe\x8e\xfa\x88\xa4\xcd\xa3"
        "\xd7\xb6\xd5\xb6\xc3\xae\xf1\x82\xf6\x93\xe3\xcf\xa6\xc8\xbc\xdd"
        "\xbe\xdd\xa8\xc5\x9a\xf5\x93\xf5\x86\xe3\x97\xbb\xd2\xbc\xc8\xbc"
        "\xd3\xa7\xc6\xaa\xf5\x85\xea\x83\xed\x99\xea\xc6\xa0\xcc\xa3\xc2"
        "\xb6\xdf\xad\xc5\xaa\x86\xe0\x8c\xe3\x82\xf6\x82\xea\x8f\xfb\x9a"
        "\xb6\xdf\xb1\xc5\xab\xde\xb3\xc1\xa9\xc6\xea\x83\xed\x99\xf7\x82"
        "\xef\x8e\xe0\x87\xeb\x8e\xa7\xdc\xb5\xdb\xaf\xdb\xb3\xd6\xa2\xc3"
        "\x9c\xf5\x91\xe9\xd4\xb3\xd6\xa2\xfd\x9a\xf6\x99\xfb\x9a\xf6\xa9"
        "\xc0\xa4\x8c\xbd\x94\xaf\xc6\xa8\xdc\xbf\xd0\xa5\xcb\xbf\xe0\x89"
        "\xed\x95\xa8\xcf\xaa\xde\x81\xe6\x8a\xe5\x87\xe6\x8a\xd5\xbc\xd8"
        "\xf0\xc0\xe9\xd2\xbb\xd5\xa1\xc6\xaa\xc5\xa7\xf8\x8b\xe2\x98\xfd"
        "\xc0\xa7\xc2\xb6\xe9\x8e\xe2\x8d\xef\x8e\xe2\xbd\xce\xa7\xdd\xb8"
        "\x90\xa0\x89\xb2\xd4\xb8\xd7\xb6\xc2\xa1\xce\xbd\xeb\x8a\xe6\xdd"
        "\xbb\xd7\xb8\xd9\xad\xde\xb7\xd9\x8f\xee\x82\xbf\xcc\xa5\xcb\xa8"
        "\xc7\xb4\x9c\xe8\x80\xe5\x91\xf0\xda\xf2\xda\xbc\xd0\xbf\xde\xaa"
        "\x83\xf7\x9f\xfa\x8e\xef\xb0\xd9\xbd\xc5\xec\xc0\xe6\x85\xea\x99"
        "\xcf\xae\xc2\xeb\xd0\xa3\xca\xa4\xf2\x93\xff\xd5\xe8\x81\xf3\x9b"
        "\xf4\xcf\xac\xc3\xb0\xe6\x87\xeb\xc1\xfc\x95\xe7\x8f\xe0\xdb\x84"
        "\xdb\xbc\xd0\xbf\xdd\xbc\xd0\xb3\xdc\xb2\xc1\xb5\xdc\xb2\xc6\xec"
        "\x80\xe9\x9a\xee\xd3\xfb\xa4\xfb\x9c\xf0\x9f\xfd\x9c\xf0\x93\xfc"
        "\x92\xe1\x95\xfc\x92\xe6\xcc\xe5\xcd\xa1\xc8\xbb\xcf\x90\xe0\x94"
        "\xe6\xcd\xa1\xc8\xbb\xcf\x90\xff\x99\xff\x8c\xe9\x9d\xb4\x8f\xd0"
        "\x8f\xe8\x84\xeb\x89\xe8\x84\xed\x83\xf7\xdd\xbc\xdf\xbc\xc9\xa4"
        "\x99\xb1\xee\xb1\xd6\xba\xd5\xb7\xd6\xba\xd3\xbd\xc9\xe3\xca\xe2"
        "\x83\xe0\x83\xf6\x9b\xc4\xb4\xc0\xb2\x99\xf4\x95\xf1\xc3\xf7\xdf"
        "\xab\xc3\xa6\xd2\xb3\xec\x85\xe1\x99\xb2\x83\xaf\xce\xad\xce\xbb"
        "\xd6\x89\xfa\x8e\xeb\x9b\xb7\xd6\xb5\xd6\xa3\xce\x91\xfe\x98\xfe"
        "\x8d\xe8\x9c\xb5\x9c\xa7\xc4\xab\xc5\xb6\xc2\xab\xc5\xb1\xc2\xaa"
        "\xc3\xa5\xd1\xec\xc4\xaa\xdf\xb2\xc0\xa8\xc7\xea\xdb\xf2\xdd\xef"
        "\xd4\xbd\xdb\xf3\x87\xef\x8a\xfe\x9f\xc0\xa9\xcd\xb5\x89\xe7\x92"
        "\xff\x9e\xf0\x97\xfb\x9e\xb7\xcc\xaa\xc5\xb7\x9f\xf6\x98\xec\x85"
        "\xb8\xdb\xb4\xc1\xaf\xdb\x84\xed\x89\xf1\xca\xa3\x9f\xeb\x84\xf0"
        "\x91\xfd\xa2\xd2\xbd\xd4\xba\xce\xbd\x86\xef\xc4\xf9\x9e\xf2\x9d"
        "\xff\xa0\xd3\xba\xc0\xa5\x8c\xf7\x94\xfb\x95\xe6\x92\xfb\x95\xe1"
        "\x97\xf6\x9a\xa7\xcb\xa2\xd1\xa5\xfe\x97\xca\xf1\x92\xfd\x93\xe0"
        "\x94\xfd\x93\xe7\x9f\xa2\x8a\xfc\x9d\xf1\xd7\xe7\x9f\xd9\x9f\xd9"
        "\x9f\xb6\x8d\xee\x81\xef\x9c\xe8\x81\xef\x9b\xe2\xdf\xf7\x81\xe0"
        "\x8c\xb2\x8c\xbd\x8b\xa2\x84\xb4\xcc\x8a\xcc\x8a\xcc\xf7\x9e\xf0"
        "\x84\xf6\xcb\xa8\xc7\xa9\xdf\xba\xc8\xbc\xe3\x8a\xe4\x90\xcf\xbd"
        "\xc9\xac\x84\xe9\x88\xec\xc4\xec\x8a\xe6\x89\xe8\x9c\xb5\xcd\xe1"
        "\x82\xed\x9e\xc8\xa9\xc5\xe9\x90\xba\xc9\xa0\xce\x98\xf9\x95\xbc"
        "\x95\xbe\xcd\xa5\xcc\xaa\xde\xe5\x84\xf0\x9f\xf2\x9b\xf8\xa7\xce"
        "\xa0\xc3\xeb\x8a\xe9\x8a\xff\x92\xb9\xcb\xe0\xd1\xf8\xc3\xbe\xc3"
        "\xbe",
        clPatchOpenCVAtomic_HoughLine,
        gcvPATCH_OPENCV_ATOMIC
    },
    {

        gcvTRUE,
        "\xa0\xff\x94\xf1\x83\xed\x88\xe4\x92\xfd\x94\xf0\x93\xf2\x9e\xfd"
        "\xb1\xc4\xb0\x98\xc7\x98\xff\x93\xfc\x9e\xff\x93\xcc\x93\xf0\x9f"
        "\xf1\x82\xf6\x83\xe0\x88\xe9\x9b\xb1\xc2\xb0\xd3\xff\x9c\xf3\x9d"
        "\xee\x9a\xf3\x9d\xe9\x9a\xe8\x8b\xd8\xac\xc9\xb9\x95\xf6\x99\xf7"
        "\x84\xf0\x99\xf7\x83\xf0\x82\xe1\xbe\xd1\xb7\xd1\xa2\xc7\xb3\x9f"
        "\xc0\x9f\xf8\x94\xfb\x99\xf8\x94\xe1\x82\xea\x8b\xf9\xd3\xbf\xca"
        "\xbe\x92\xf1\x9e\xf0\x83\xf7\x9e\xf0\x84\xe0\x93\xe7\xb4\xc0\xa5"
        "\xd5\xf9\x9a\xf5\x9b\xe8\x9c\xf5\x9b\xef\x8b\xf8\x8c\xd3\xbc\xda"
        "\xbc\xcf\xaa\xde\xf2\x91\xfe\x90\xe3\x97\xfe\x90\xe4\xd6\xa2\xcb"
        "\xa7\xc2\x91\xf8\x82\xe7\xcb\xa8\xc7\xa9\xda\xae\xc7\xa9\xdd\xa9"
        "\xc0\xac\xc9\xba\xe2\xce\xad\xc2\xac\xdf\xab\xc2\xac\xd8\xbb\xd7"
        "\xbe\xce\x82\xeb\x86\xef\x9b\xb7\xd4\xbb\xd5\xa6\xd2\xb4\xd8\xb7"
        "\xd6\xa2\xce\xbb\xcf\x9c\xff\x9e\xf2\x97\xbe\xc5\x9a\xc5\xa9\xc6"
        "\xa5\xc4\xa8\xc1\xaf\xdb\xa8\xc5\xa0\xcd\x96\xa3\x92\xa0\xfd\xc6"
        "\xaf\xc1\xb5\xc1\xb9\x84\xe3\x86\xf2\xad\xca\xb8\xd7\xa2\xd2\x8d"
        "\xe4\x80\xa8\x98\xb1\x8a\xe3\x8d\xf9\x8d\xf4\xc9\xae\xcb\xbf\xe0"
        "\x87\xf5\x9a\xef\x9f\xc0\xa9\xcd\xe5\xd4\xfd\xc6\xaf\xc1\xb5\xc1"
        "\xa8\xcc\xf1\x96\xf3\x87\xd8\xb4\xdb\xb8\xd9\xb5\xea\x83\xe7\xcf"
        "\xfe\xd7\xfd\x9a\xff\x8b\xd4\xb8\xd7\xb4\xd5\xb9\xe6\x95\xfc\x86"
        "\xe3\xcb\xfb\xd2\xf9\x9e\xfb\x8f\xd0\xbc\xd3\xb0\xd1\xbd\xe2\x8b"
        "\xef\xc7\xf7\xde\xe5\x96\xfb\x9e\xf3\xa8\xdc\xb5\xd1\x8c\xb1\x81"
        "\xba\xd8\xb9\xcb\xb9\xd0\xb5\xc7\xef\xac\xe0\xab\xf4\xb8\xf7\xb4"
        "\xf5\xb9\xe6\xab\xee\xa3\xfc\xba\xff\xb1\xf2\xb7\x9e\xa5\xc3\xac"
        "\xde\xf6\x9f\xf1\x85\xec\xd1\xb6\xd3\xa7\xf8\x94\xfb\x98\xf9\x95"
        "\xca\xa3\xc7\xef\xde\xf7\xcc\xa5\x99\xed\x84\xe8\x8d\xde\xb7\xcd"
        "\xa8\x86\xff\xc4\xad\x86\xbb\xdc\xb9\xcd\x92\xfe\x91\xf2\x93\xff"
        "\xa0\xd3\xba\xc0\xa5\x8d\xbc\x95\xbc\xc7\x98\xc7\xa0\xcc\xa3\xc1"
        "\xa0\xcc\xaf\xc0\xae\xdd\xa9\xdc\xbf\xd7\xb6\xc4\xee\x9d\xef\x8c"
        "\xdc\xa8\xda\xe7\x94\xe6\x85\xae\xc3\xa2\xc6\xf4\xc0\xe8\x9c\xe5"
        "\xcf\xbb\xd2\xbe\xdb\x88\xe1\x9b\xfe\xd0\xa9\x82\xeb\xc7\xb4\xc6"
        "\xa5\xf6\x82\xe7\x97\xbb\xcf\xb7\x9d\xe9\x80\xec\x89\xda\xb3\xc9"
        "\xac\x82\xfa\xd1\xa2\xd0\xb3\xec\x83\xe5\x83\xf0\x95\xe1\xc8\xf3"
        "\x95\xfa\x88\xa0\xc9\xa7\xd3\xb9\x84\xe3\x86\xf2\xad\xc1\xae\xcd"
        "\xac\xc0\x9f\xf6\x92\xba\x8a\xa3\x98\xf2\xce\xba\xd3\xbf\xda\x89"
        "\xe0\x9a\xff\xd1\xa9\x92\xf8\xd3\xee\x89\xec\x98\xc7\xab\xc4\xa7"
        "\xc6\xaa\xf5\x86\xef\x95\xf0\xd8\xe8\xc1\xe8\x93\xf0\x9f\xf1\x82"
        "\xf6\x9f\xf1\x85\xe1\x80\xf4\x95\xa8\xdb\xa9\xca\x9a\xee\x9c\xc7"
        "\xad\xf0\xcb\xaa\xde\xb1\xdc\xb5\xd6\x89\xe0\x8e\xed\xc5\xe3\x90"
        "\xfd\x98\xf5\xae\xca\xab\xdf\xbe\xe3\xca\xf1\x8c\xf1\x93\xf2\x80"
        "\xf2\x9b\xfe\x8c\xa4\xe7\xab\xe0\xbf\xf3\xbc\xff\xbe\xf2\xad\xe0"
        "\xa5\xe8\xb7\xf1\xb4\xfa\xb9\xfc\xd5\xee",
        clPatchOpenCVAtomic_clAHE,
        gcvPATCH_OPENCV_ATOMIC
    },
};

static gctCONST_STRING
clfFindString(
    gctBOOL Encrypted,
    gctCONST_STRING String,
    gctCONST_STRING Search,
    gctINT_PTR SearchIndex
    )
{
    gctCONST_STRING source;
    gctINT          sourceIndex;
    gctCONST_STRING search;
    gctUINT8        key = Encrypted ? 0xFF : 0x00;

    gcmHEADER_ARG("Encrypted=%d, String=%s Search=%s SearchIndex=0x%x",
                  Encrypted, String, Search, SearchIndex);

    /* Start from the beginning of the source string. */
    source = String;

    /* Reset search index. */
    sourceIndex = 0;
    search      = Search;

    /* Loop until the end of the source string. */
    while (source[sourceIndex] != '\0')
    {
        gctCHAR ch = *search ^ key;

        /* Check if we match the current search character. */
        if (source[sourceIndex] == ch)
        {
            /* Increment the source index. */
            sourceIndex ++;

            if (Encrypted)
            {
                if ((*search ^ key) == 0)
                {
                    key ^= 0xFF;
                }
                key ^= ch;
            }

            /* Increment the search index and test for end of string. */
            if (*++search == '\0')
            {
                /* Return the current index. */
                *SearchIndex = sourceIndex;

                /* Return the location of the search string. */
                gcmFOOTER_ARG("return=%s", source);
                return source;
            }
        }

        /* Skip any white space in the source string. */
        else if ((source[sourceIndex] == ' ')  ||
                 (source[sourceIndex] == '\t') ||
                 (source[sourceIndex] == '\r') ||
                 (source[sourceIndex] == '\n') ||
                 (source[sourceIndex] == '\\'))
        {
            if (sourceIndex == 0)
            {
                /* Increment the source. */
                source++;
            }
            else
            {
                /* Increment the source index. */
                sourceIndex++;
            }
        }

        /* No match. */
        else
        {
            /* Next source character. */
            source++;

            /* Reset search index. */
            sourceIndex = 0;
            search      = Search;
            key         = Encrypted ? 0xFF : 0x00;
        }
    }

    /* No match. */
    gcmFOOTER_ARG("return=%s",gcvNULL);
    return gcvNULL;
}

void doPatchCreateProgram(cl_program Program )
{
    if(Program->source)
    {
        gctINT i ;
        for(i = 1 ; i < gcmSIZEOF(cl_patch_array) / gcmSIZEOF(cl_patch_array[0]) ; ++i)
        {
            gctINT searIndex = 0;
            if(clfFindString(cl_patch_array[i].encrypted, Program->source, cl_patch_array[i].matchEncryptedStr, &searIndex))
            {
                Program->patchIndex = i;
                break;
            }
        }
    }
}

/*called when clCreateKernel */
void doPatchCreateKernel(cl_program Program, cl_kernel Kernel, gctSTRING KernelName )
{

    if(Program->patchIndex)
    {
        if(cl_patch_array[Program->patchIndex].handleKernel && cl_patch_array[Program->patchIndex].handleKernel(Kernel, KernelName))
        {
            *gcGetPatchId() = cl_patch_array[Program->patchIndex].patchID;
        }
    }
}

