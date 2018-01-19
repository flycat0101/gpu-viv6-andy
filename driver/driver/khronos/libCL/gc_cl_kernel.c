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
#include "gc_hal_user_math.h"
#include "../user/gc_hal_user_shader.h"

#define __NEXT_MSG_ID__     007032

extern gctBOOL gcSHADER_GoVIRPass(gcSHADER Shader);


/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/
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
        /* Release arguments */
        gcmVERIFY_OK(clfFreeKernelArgs(Kernel->numArgs, Kernel->args, gcvTRUE));

        /* Release mutex. */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, Kernel->argMutex));
        Kernel->argMutex = gcvNULL;

        Kernel->objectType = clvOBJECT_UNKNOWN;
        clfReleaseProgram(Kernel->program);

        while (Kernel->patchedStates)
        {
            clsKernelStates_PTR states = Kernel->patchedStates;
            gcmASSERT(states->programState.hints != Kernel->states.programState.hints);
            /* Release states */
            Kernel->patchedStates = states->next;
            gcFreeProgramState(states->programState);
            if (states->binary) gcSHADER_Destroy((gcSHADER)states->binary);
            if (states->patchDirective) clfDestroyPatchDirective(&states->patchDirective);
            gcmOS_SAFE_FREE(gcvNULL, states);
        }
        gcFreeProgramState(Kernel->states.programState);
        if (Kernel->states.binary) gcSHADER_Destroy((gcSHADER)Kernel->states.binary);
        if (Kernel->name) gcmOS_SAFE_FREE(gcvNULL, Kernel->name);

        /* Destroy the reference count object */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, Kernel->referenceCount));
        Kernel->referenceCount = gcvNULL;

        gcmOS_SAFE_FREE(gcvNULL, Kernel);
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
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
    clsKernel_PTR        Kernel,
    clsKernelStates_PTR  States,
    clsArgument_PTR      Arg,
    gctUINT              WorkDim,
    size_t               GlobalWorkOffset[3],
    size_t               GlobalWorkSize[3],
    size_t               LocalWorkSize[3],
    clsArgument_PTR      baseArg,
    gctUINT              numArg,
    clsPrivateBuffer_PTR *privateBufList
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
    gcSHADER             Shader = (gcSHADER)States->binary;
    gcsHINT_PTR          hints = States->programState.hints;

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
            if (!memObj && length == 1)
            {
                static int zero = 0;
                data = &zero;
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);
                status = CL_SUCCESS;
                gcmFOOTER_ARG("%d", status);
                return status;
            }

            gcmASSERT(memObj);
            if (memObj->type == CL_MEM_OBJECT_BUFFER)
            {
                if (memObj->u.buffer.wrapped && ((memObj->flags & CL_MEM_USE_UNCACHED_HOST_MEMORY_VIV) == 0))
                {
                    gcoOS_CacheInvalidate(gcvNULL, 0, memObj->u.buffer.logical, memObj->u.buffer.allocatedSize);
                }

                /* Is this buffer for read or write?  Get render fence, and wait from render->blt engine result */
                gcoCL_MemWaitAndGetFence(memObj->u.buffer.node,gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

                data = (gctINT *) &memObj->u.buffer.physical;
            }
            else
            {
                data = (gctINT *) &memObj->u.image.physical;

                /* fence record at image header, is this for read or write? */
                gcoCL_MemWaitAndGetFence(memObj->u.image.node, gcvENGINE_RENDER,gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

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
                        tmpData[2] = (memObj->u.image.width) | (memObj->u.image.width<<16);
                        sliceSize = memObj->u.image.width;
                        depth = 1;
                        type = 0;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D:
                        tmpData[2] = (memObj->u.image.width) | (memObj->u.image.height<<16);
                        sliceSize = memObj->u.image.width * memObj->u.image.height;
                        depth = 1;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE3D:
                        tmpData[2] = (memObj->u.image.width) | (memObj->u.image.height<<16);
                        sliceSize = memObj->u.image.width * memObj->u.image.height;
                        depth = memObj->u.image.depth;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
                        tmpData[2] = (memObj->u.image.width) | (memObj->u.image.width<<16);
                        sliceSize = memObj->u.image.width;
                        depth = memObj->u.image.arraySize;
                        type = 1;
                        break;
                    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
                        tmpData[2] = (memObj->u.image.width) | (memObj->u.image.height<<16);
                        sliceSize = memObj->u.image.width * memObj->u.image.height;
                        depth = memObj->u.image.arraySize;
                        type = 1;
                        break;
                    default:
                        break;
                    }

                    switch(memObj->u.image.format.image_channel_data_type)
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

                    switch(memObj->u.image.format.image_channel_order)
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
                                        switch(memObj->u.image.format.image_channel_order)
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
                                switch(memObj->u.image.format.image_channel_order)
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
                        if(memObj->u.image.vxcaddressingMode == CL_ADDRESS_CLAMP_TO_EDGE)
                            addressMode = 3;
                        if(memObj->u.image.vxcaddressingMode == CL_ADDRESS_CLAMP)
                            addressMode = 1;
                    }

                    tmpData[3] = shift | (addressMode<<4) | (format<<6) | (tiling<<10) | (type<<12)
                        | (componentCount<<14) | (swizzleR<<16) | (swizzleG<<20) | (swizzleB<<24) | (swizzleA<<28);
                    tmpData[4] = sliceSize;
                    tmpData[5] = depth;
                    tmpData[6] = ((gctUINT)(memObj->u.image.format.image_channel_data_type << 16) | (gctUINT)(memObj->u.image.format.image_channel_order));
                    data = (gctINT *)tmpData;

                    if(Arg->uniform->arraySize == 2)
                    {
                        length = 8;
                    }
                    else
                    {
                        length = 4;
                    }
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
                                    clsImageHeader_PTR imageHeader = (clsImageHeader_PTR) memObj->u.image.logical;
                                    image[0] = imageHeader->width;
                                    image[1] = imageHeader->rowPitch;
                                    image[2] = imageHeader->width - 1;
                                    image[3] = imageHeader->height - 1;
                                    gcmERR_BREAK(gcUNIFORM_SetValue(uniform, 1, image));
                                    data = (gctINT *) &imageHeader->physical;
                                    break;
                                }
                            }
                        }
                    }
                    while (gcvFALSE);
                }
#endif
            }

            clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                       CL_INVALID_VALUE);
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
                clmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
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

                        status = gcUNIFORM_SetValue(Arg->uniform, length, pointer);
                        gcoOS_Free(gcvNULL, data);
                        if (gcmIS_ERROR(status))
                        {
                            clmRETURN_ERROR(CL_INVALID_VALUE);
                        }

                    }
                    break;

                default:
                    clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, Arg->data),
                               CL_INVALID_VALUE);
                    break;
                }
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

                clmONERROR(gcoSHADER_ProgramUniformEx(gcvNULL, physicalAddress,
                                                        numCols, numRows, arraySize, gcvTRUE,
                                                        sizeof(cl_long),
                                                        4*sizeof(cl_long),
                                                        pData, gcvUNIFORMCVT_NONE, GetUniformShaderKind(Arg->uniform)), CL_INVALID_VALUE);
                break;

            case gcSHADER_IMAGE_2D:
            case gcSHADER_IMAGE_3D:
            case gcSHADER_IMAGE_1D:
            case gcSHADER_IMAGE_1D_ARRAY:
            case gcSHADER_IMAGE_1D_BUFFER:
            case gcSHADER_IMAGE_2D_ARRAY:
                {
                clsMem_PTR image = *(clsMem_PTR *) Arg->data;
                gctINT * data = (gctINT *) &image->u.image.physical;

                gcoCL_MemWaitAndGetFence(image->u.image.node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);

                clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                              length,
                                              data),
                           CL_INVALID_VALUE);
                }
                break;

            case gcSHADER_SAMPLER:
                {
                gcmASSERT(length == 1);
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
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

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformGlobalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, GlobalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 globalWorkSize[3];

        globalWorkSize[0] = GlobalWorkSize[0];
        globalWorkSize[1] = GlobalWorkSize[1];
        globalWorkSize[2] = GlobalWorkSize[2];
        gcoOS_MemCopy(Arg->data, globalWorkSize, gcmSIZEOF(gctUINT32) * 3);

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
    else if (isUniformLocalSize(Arg->uniform))
    {
        /* TODO - For 64-bit GPU. */
        /*gcoOS_MemCopy(Arg->data, LocalWorkSize, gcmSIZEOF(size_t) * 3);*/
        gctUINT32 localWorkSize[3];

        localWorkSize[0] = LocalWorkSize[0];
        localWorkSize[1] = LocalWorkSize[1];
        localWorkSize[2] = LocalWorkSize[2];
        gcoOS_MemCopy(Arg->data, localWorkSize, gcmSIZEOF(gctUINT32) * 3);

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
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

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);

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

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
#if __ENABLE_OPTIMIZE_FOR_SHARE_MEMORY__
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
        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
#endif
    else if (isUniformLocalAddressSpace(Arg->uniform))
    {
        clsMemAllocInfo_PTR memAllocInfo = (clsMemAllocInfo_PTR) Arg->data;

        /* Size may be zero if declared but never used */
        if (memAllocInfo->allocatedSize > 0)
        {
            gctINT * data;
            gctINT totalNumGroups = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1)
                                  * (WorkDim > 1 ? (GlobalWorkSize[1] / (LocalWorkSize[1] ? LocalWorkSize[1] : 1)) : 1)
                                  * (WorkDim > 2 ? (GlobalWorkSize[2] / (LocalWorkSize[2] ? LocalWorkSize[2] : 1)) : 1);

            clmASSERT(Arg->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

#if __ENABLE_OPTIMIZE_FOR_SHARE_MEMORY__
            /* Compare the workGroupCount with total group number and choose the smaller one. */
            if (hints->workGroupCount != 0 &&
                (gctINT)hints->workGroupCount < totalNumGroups)
            {
                totalNumGroups = hints->workGroupCount;
            }
#endif

            memAllocInfo->allocatedSize *= totalNumGroups;

            /* Allocate the physical buffer */
            clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                            &memAllocInfo->physical,
                                            &memAllocInfo->logical,
                                            &memAllocInfo->node,
                                            0),
                       CL_INVALID_VALUE);

            if (gcmOPT_hasFeature(FB_FORCE_LS_ACCESS) /* triage option */
                || (strcmp(Arg->uniform->name, "#local_address") == 0  &&
                    gcShaderUseLocalMem(Shader))
                )
            {
                /* Local memory address starts at 0. */
                gctPHYS_ADDR physical = 0;
                data = (gctINT *) &physical;
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);
            }
            else
            {
                data = (gctINT *) &memAllocInfo->physical;
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);
                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
            }
        }
    }
#if __ENABLE_OPTIMIZE_FOR_PRI_MEMORY__
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
        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);
    }
#endif
    else if(isUniformPrivateAddressSpace(Arg->uniform))
    {
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

#if __ENABLE_OPTIMIZE_FOR_PRI_MEMORY__
            /* Compare the workThreadCount with total item number and choose the smaller one. */
            if (hints->workThreadCount != 0 &&
                (gctINT)hints->workThreadCount < totalNumItems)
            {
                memAllocInfo->allocatedSize *= hints->workThreadCount;
            }
            else
#endif
            {
                memAllocInfo->allocatedSize *= totalNumItems;
            }

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
                                         privateBuf->node), CL_INVALID_VALUE);

                        /* re-allocate the physical buffer */
                        clmONERROR(gcoCL_AllocateMemory(&memAllocInfo->allocatedSize,
                                                        &memAllocInfo->physical,
                                                        &memAllocInfo->logical,
                                                        &memAllocInfo->node,
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
                                                  0);
                    if(gcmIS_ERROR(status))
                    {
                        if(tempNode != gcvNULL)
                            gcmOS_SAFE_FREE(gcvNULL, tempNode);
                        if (privateBuf != gcvNULL)
                            gcmOS_SAFE_FREE(gcvNULL, privateBuf);
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
                || (strcmp(Arg->uniform->name, "#local_address") == 0 &&
                    gcShaderUseLocalMem(Shader))
                )
            {
                /* Local memory address starts at 0. */
                gctPHYS_ADDR physical = 0;
                data = (gctINT *) &physical;
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);
            }
            else
            {
                data = (gctINT *) &memAllocInfo->physical;
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                           CL_INVALID_VALUE);

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
                                        0),
                   CL_INVALID_VALUE);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
                   CL_INVALID_VALUE);

        gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_READ, gcvFENCE_TYPE_READ);

        /* Copy the constant data to the buffer */
        gcoOS_MemCopy(memAllocInfo->logical, Kernel->constantMemBuffer, Kernel->constantMemSize);

        gcmDUMP_BUFFER(gcvNULL,
                       "memory",
                       gcmPTR2INT(memAllocInfo->physical),
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
                                        0),
                   CL_INVALID_VALUE);

        /* Initialize printf buffer with 0xFF so we can use it to check if it is written. */
        gcoOS_MemFill(memAllocInfo->logical, 0xFF, memAllocInfo->allocatedSize);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
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

        clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                      length,
                                      Arg->data),
                   CL_INVALID_VALUE);

        Arg->printBufferSizePerThread = printBufferSize;
    }
    else if (isUniformKernelArgSampler(Arg->uniform))
    {
        if(Kernel->program->buildOptions == gcvNULL
            || (Kernel->program->buildOptions != gcvNULL
                 && strstr(Kernel->program->buildOptions, "-cl-viv-vx-extension") == gcvNULL
                 && (Arg->uniform->address != 0)
                )
           )
        {
                gcmASSERT(length == 1);
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform,
                                              length,
                                              Arg->data),
                           CL_INVALID_VALUE);
        }

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
                                        0),
                   CL_INVALID_VALUE);

        data = (gctINT *) &memAllocInfo->physical;
        clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, data),
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
            clmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
                                           length,
                                           Arg->data),
                       CL_INVALID_VALUE);
            break;

        case gcSHADER_IMAGE_2D:
            clmONERROR(gcUNIFORM_SetValueF(Arg->uniform,
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
                    status = gcUNIFORM_SetValue(Arg->uniform, length, pointer);
                    gcoOS_Free(gcvNULL, data);
                    if (gcmIS_ERROR(status)) clmRETURN_ERROR(CL_INVALID_VALUE);

                }
                break;

            default:
                clmONERROR(gcUNIFORM_SetValue(Arg->uniform, length, Arg->data),
                           CL_INVALID_VALUE);
                break;
            }
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

                clmONERROR(gcoSHADER_ProgramUniformEx(gcvNULL, physicalAddress,
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
    gctPHYS_ADDR        physical = gcvNULL;
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
        clmONERROR(gcoCL_AllocateMemory(&allocatedSize, &physical, &logical, &node, 0), CL_INVALID_VALUE);

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
                memAllocInfo->physical = (gctPHYS_ADDR)((gctUINTPTR_T)physical + totalArgAlignSize);

                if(totalArgSize == 0) memAllocInfo->node = node;
                totalArgSize += gcmALIGN(memAllocInfo->allocatedSize, 4);

                data = (gctINT *) &memAllocInfo->physical;
                clmONERROR(gcUNIFORM_SetValue(argument->uniform, length, data), CL_INVALID_VALUE);

                gcoCL_MemWaitAndGetFence(memAllocInfo->node, gcvENGINE_RENDER, gcvFENCE_TYPE_ALL, gcvFENCE_TYPE_ALL);
            }
            else if (isUniformKernelArgLocalMemSize(argument->uniform))
            {
                memAllocInfo = (clsMemAllocInfo_PTR) argument->data;

                clmASSERT(argument->isMemAlloc == gcvTRUE, CL_INVALID_VALUE);

                memAllocInfo->allocatedSize = allocatedSize;

                data = (gctINT *) &totalSize;
                clmONERROR(gcUNIFORM_SetValue(argument->uniform, length, data), CL_INVALID_VALUE);
            }
        }
    }

    status = CL_SUCCESS;

OnError:
    if(node && clmIS_ERROR(status))
    {
        gcoCL_FreeMemory(physical, logical, allocatedSize, node);
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

        gcmTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_CL,
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
            size_t *WidthLeave;
            size_t *HeightLeave;
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

            gcoOS_Allocate(gcvNULL, count*sizeof(size_t), (gctPOINTER*)&WidthLeave);
            gcoOS_Allocate(gcvNULL, count*sizeof(size_t), (gctPOINTER*)&HeightLeave);

            WidthAlign = InputGlobalWorkSize[0]%preferredWorkGroupSize == 0;
            HeightAlign = InputGlobalWorkSize[1]%preferredWorkGroupSize == 0;

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
            gcoOS_Free(gcvNULL, (gctPOINTER)WidthLeave);
            gcoOS_Free(gcvNULL, (gctPOINTER)HeightLeave);
        }
        break;
    case 3:
        gcmASSERT(-1);/* TODO, need support 3D WAR */
        break;
    default:
        break;
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


gctINT
clfGetKernelValueOrder(
    clsKernel_PTR       Kernel,
    gctUINT_PTR         ValueOrder
    )
{
    gctINT          status;
    gctUINT         i, j, k;
    gcSHADER        binary;
    gcATTRIBUTE     attribute;
    gctCONST_STRING attributeName;
    gctUINT map[3] = {0, 0, 0};
    const gctSTRING id[3] = {"#global_id", "#group_id", "#local_id"};

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    binary = (gcSHADER) Kernel->states.binary;
    clmASSERT(binary, CL_INVALID_VALUE);

    for (i = 0, j = 0; (i < Kernel->attributeCount) && (j < 3); i++)
    {
        clmONERROR(gcSHADER_GetAttribute(binary, i, &attribute), CL_INVALID_VALUE);
        clmONERROR(gcATTRIBUTE_GetName(binary, attribute, gcvFALSE, gcvNULL, &attributeName), CL_INVALID_VALUE);
        for (k = 0; k < 3; k++)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(attributeName, id[k])))
            {
                map[j++] = k;
            }
        }
    }

    switch (map[0])
    {
    case 0:
        if (map[1] == 1)
        {
            *ValueOrder = 2; /* GWL -> LGW */
        }
        else
        {
            *ValueOrder = 3; /* GLW -> WGL */
        }
        break;

    case 1:
        if (map[1] == 0)
        {
            *ValueOrder = 4; /* WGL -> LWG */
        }
        else
        {
            *ValueOrder = 1; /* WLG -> GWL */
        }
        break;

    case 2:
        if (map[1] == 0)
        {
            *ValueOrder = 5; /* LGW -> WLG */
        }
        else
        {
            *ValueOrder = 0; /* LWG -> GLW */
        }
        break;

    default:
        *ValueOrder = 0;     /* LWG -> GLW */
        break;
    }

    status = CL_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

gctINT
clfExecuteKernel(
    clsKernel_PTR       Kernel,
    clsKernelStates_PTR States,
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
    gcsTHREAD_WALKER_INFO   info;
    gctUINT                 i;
    gctINT                  status;

    gcmHEADER_ARG("Kernel=0x%x", Kernel);

    gcoOS_ZeroMemory(&info, gcmSIZEOF(info));

    /* Load kernel states. */
    clmONERROR(gcoCL_LoadKernel(States->programState),
               CL_OUT_OF_RESOURCES);

    /* Adjust local work sizes if not specified by the application. */
    clmONERROR(clfAdjustLocalWorkSize(Kernel,
                                      WorkDim,
                                      GlobalWorkOffset,
                                      GlobalWorkSize,
                                      LocalWorkSize),
               CL_INVALID_VALUE);

    if (States->patchDirective == gcvNULL)
    {
        /* Load argument values. */
        for (i = 0; i < NumArgs; i++)
        {
            if (Args[i].uniform && !isUniformInactive(Args[i].uniform))
            {
                clmONERROR(clfLoadKernelArgValues(Kernel,
                                                  States,
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

        for (i = 0; i < WorkDim; i++)
        {
            patchedGlobalWorkOffset[i] = GlobalWorkOffset[i];
            patchedGlobalWorkSize[i]   = GlobalWorkSize[i];
            patchedLocalWorkSize[i]    = LocalWorkSize[i];
        }

        for (patchDirective = States->patchDirective;
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

                    clmONERROR(gcUNIFORM_SetValue(globalWorkSize->globalWidth,
                                                  1,
                                                  uniGlobalWidth),
                               CL_INVALID_VALUE);
                }

                if (globalWorkSize->groupWidth &&
                    !isUniformInactive(globalWorkSize->groupWidth))
                {
                    gctINT  uniGroupWidth  = GlobalWorkSize[0] / (LocalWorkSize[0] ? LocalWorkSize[0] : 1);
                    groupSize = globalWorkSize->groupWidth;
                    clmONERROR(gcUNIFORM_SetValue(globalWorkSize->groupWidth,
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
                                                  States,
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
    }

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
                                  States->programState.hints->valueOrder,
                                  States->programState.hints->threadGroupSync));


    gcmFOOTER_ARG("%d info=0x%x",
                  CL_SUCCESS, info);
    return CL_SUCCESS;

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
    gctBOOL             globalWorkSizeWorkaroundNeed = gcvFALSE;

    gcmHEADER_ARG("Command=0x%x", Command);

    clmASSERT(Command && Command->objectType == clvOBJECT_COMMAND, CL_INVALID_VALUE);

    clmASSERT(Command->type == clvCOMMAND_NDRANGE_KERNEL, CL_INVALID_VALUE);

    EVENT_SET_GPU_RUNNING(Command, gcvENGINE_RENDER);

#if VIVANTE_PROFILER
    if (Command->commandQueue->profiler.perClfinish == gcvFALSE)
        clfBeginProfiler(Command->commandQueue);
#endif

    NDRangeKernel = &Command->u.NDRangeKernel;

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
        while(i < GetShaderUniformCount((gcSHADER)NDRangeKernel->states->binary))
        {
            gcUNIFORM uniform = GetShaderUniform((gcSHADER)NDRangeKernel->states->binary, i);
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
        while (i < GetShaderAttributeCount((gcSHADER)NDRangeKernel->states->binary)) {
            gctCONST_STRING attributeName=gcvNULL;
            gcATTRIBUTE attribute = GetShaderAttribute((gcSHADER)NDRangeKernel->states->binary, i);
            if (attribute == gcvNULL) break;
            gcATTRIBUTE_GetName((gcSHADER)NDRangeKernel->states->binary, attribute, gcvFALSE, gcvNULL, &attributeName);
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
        if(NDRangeKernel->states->programState.hints)
        {
            /* relax the maxworkgroupsize while shader used barrier as HW limit*/
            for (i = 0; i < GetShaderCodeCount((gcSHADER)NDRangeKernel->states->binary); i++)
            {
                gcSL_INSTRUCTION inst   = GetShaderInstruction((gcSHADER)NDRangeKernel->states->binary, i);
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

    if (globalWorkSizeWorkaroundNeed)
    {
        clfCalcLocalWorkSize(NDRangeKernel->kernel, NDRangeKernel->workDim,
                                NDRangeKernel->globalWorkOffset,NDRangeKernel->globalWorkSize, NDRangeKernel->localWorkSize,
                                globalWorkOffset, globalWorkSize, localWorkSize);
        for(i = 0; i < 9; i+=3)
        {
            if(globalWorkSize[i]==0) break;
            clmONERROR(clfExecuteKernel(NDRangeKernel->kernel,
                                        NDRangeKernel->states,
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
        clmONERROR(clfExecuteKernel(NDRangeKernel->kernel,
                                    NDRangeKernel->states,
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


    /* A special flag 0x4 to verify MultiGpu sync */
    if (Command->commandQueue->properties & 0x4)
    {
        gctUINT  coreCount;
        gctUINT  chipIDs[4] = {0xffffffff};
        clmONERROR(gcoHAL_QueryCoreCount(gcvNULL, gcvHARDWARE_3D, &coreCount, chipIDs), CL_INVALID_VALUE);
        clmONERROR(gcoCL_MultiGPUSync(coreCount, chipIDs), CL_INVALID_VALUE);
    }


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
                                task->states,
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

gctINT
clfAllocateKernelArgs(
    clsKernel_PTR   Kernel
    )
{
    gceSTATUS       status;
    gctPOINTER      pointer;
    gctSIZE_T       bytes;
    clsArgument_PTR argument;
    gctUINT         i;

    if (Kernel->numArgs == 0)
    {
        Kernel->args = gcvNULL;
        return gcvSTATUS_OK;
    }

    /* Allocate the array of arguments. */
    bytes = Kernel->numArgs * gcmSIZEOF(clsArgument);
    clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
    gcoOS_ZeroMemory(pointer, bytes);

    Kernel->args = argument = pointer;
    for (i = 0; i < Kernel->numArgs; i++, argument++)
    {
        gcUNIFORM uniform;
        gcSHADER_TYPE type;
        gcSL_FORMAT format;
        gctBOOL isPointer;
        gctUINT length;
        gctSIZE_T bytes;

        clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->states.binary, i, &uniform), CL_INVALID_VALUE);

        if (!uniform) continue;
        if (isUniformCompiletimeInitialized(uniform)) continue;

        clmONERROR(gcUNIFORM_GetType(uniform, &type, &length), CL_INVALID_VALUE);
        clmONERROR(gcUNIFORM_GetFormat(uniform, &format, &isPointer), CL_INVALID_VALUE);

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
            isUniformPrintfAddress(uniform))
        {
            clsMemAllocInfo_PTR memAllocInfo;
            gctPOINTER pointer;

            bytes = sizeof(clsMemAllocInfo);

            /* Allocate the memory allocation info. */
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetLocalMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->localMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetPrivateMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetConstantMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ), CL_INVALID_VALUE);
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;
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
                case gcSHADER_SAMPLER:
                case gcSHADER_IMAGE_1D:
                case gcSHADER_IMAGE_1D_ARRAY:
                case gcSHADER_IMAGE_1D_BUFFER:
                case gcSHADER_IMAGE_2D_ARRAY:
                    bytes = 1 * gcmSIZEOF(cl_uint) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
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

OnError:
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
            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->states.binary, i, &uniform), CL_INVALID_VALUE);
            if (!uniform) continue;
            argument->uniform    = uniform;
            continue;
        }

        clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->states.binary, i, &uniform), CL_INVALID_VALUE);

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
            clsMemAllocInfo_PTR memAllocInfo;
            gctPOINTER pointer;

            bytes = sizeof(clsMemAllocInfo);

            /* Allocate the memory allocation info. */
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &pointer), CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(pointer, bytes);
            memAllocInfo = pointer;

            /* Get the required memory size.
             * For local/private kernel arguments it will be set by the application.
             */
            if (isUniformLocalAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetLocalMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->localMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformPrivateAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetPrivateMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize), CL_INVALID_VALUE);
                Kernel->privateMemSize += memAllocInfo->allocatedSize;
            }
            else if (isUniformConstantAddressSpace(uniform))
            {
                clmONERROR(gcSHADER_GetConstantMemorySize((gcSHADER) Kernel->states.binary, &memAllocInfo->allocatedSize, &Kernel->constantMemBuffer ), CL_INVALID_VALUE);
                Kernel->constantMemSize += memAllocInfo->allocatedSize;
            }

            argument->data       = memAllocInfo;
            argument->uniform    = uniform;
            argument->size       = bytes;
            argument->set        = gcvFALSE;
            argument->isMemAlloc = gcvTRUE;
            argument->isPointer  = gcvFALSE;
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
                case gcSHADER_SAMPLER:
                case gcSHADER_IMAGE_1D:
                case gcSHADER_IMAGE_1D_ARRAY:
                case gcSHADER_IMAGE_1D_BUFFER:
                case gcSHADER_IMAGE_2D_ARRAY:
                    bytes = 1 * gcmSIZEOF(cl_uint) * length;
                    break;

                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                    bytes = 1 * gcmSIZEOF(cl_float) * length;
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

    return status;
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

                             memAllocInfo->node);
            }
            if (FreeAllocData && memAllocInfo->data) gcmOS_SAFE_FREE(gcvNULL, memAllocInfo->data);
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
    gcsPROGRAM_STATE programState = {0};
    gcSHADER        pgmBinary, kernelBinary = gcvNULL;
    gcSHADER        tmpBinary = gcvNULL;
    gctUINT         tmpBinarySize = 0;
    gctPOINTER      savedTmpBinary = gcvNULL;
    gctUINT         binarySize;
    gctUINT         i, j;
    gctUINT         count, propertySize = 0;
    gctINT          propertyType = 0;
    gctUINT         propertyValues[3] = {0};
    gceSHADER_FLAGS flags;
    gcKERNEL_FUNCTION kernelFunction;
    gceCHIPMODEL    chipModel;
    gctUINT32       chipRevision;
    gcePATCH_ID     patchId = gcvPATCH_INVALID;
    gctBOOL         hasBarrier = gcvFALSE, hasImageWrite = gcvFALSE;
    gctBOOL         supportImageInst = Program->context->devices[0]->deviceInfo.supportIMGInstr;
    gctUINT32       maxRegCount = Program->context->devices[0]->deviceInfo.maxRegisterCount;
    gctUINT32_PTR   comVersion;

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
    kernel->preferredWorkGroupSizeMultiple = Program->devices[0]->deviceInfo.maxComputeUnits * 4;
    kernel->patchNeeded       = gcvTRUE;    /* Force to check patch instructions for the moment. May hurt compiler performance. */
    kernel->states.binary     = gcvNULL;
    kernel->isPatched         = gcvFALSE;

    /* Create a reference count object and set it to 1. */
    clmONERROR(gcoOS_AtomConstruct(gcvNULL, &kernel->referenceCount),
               CL_OUT_OF_HOST_MEMORY);

    gcmVERIFY_OK(gcoOS_AtomIncrement(gcvNULL, kernel->referenceCount, gcvNULL));

    clmONERROR(gcoOS_AtomIncrement(gcvNULL, clgGlobalId, (gctINT*)&kernel->id), CL_INVALID_VALUE);

    clfRetainProgram(Program);

    /* Copy kernel name */
    length = gcoOS_StrLen(KernelName, gcvNULL) + 1;
    clmONERROR(gcoOS_Allocate(gcvNULL, length, &pointer), CL_OUT_OF_HOST_MEMORY);
    gcmVERIFY_OK(gcoOS_StrCopySafe(pointer, length, KernelName));
    kernel->name = (gctSTRING) pointer;
    /* Save program binary into buffer */
    pgmBinary = (gcSHADER) Program->binary;
    clmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize), CL_INVALID_VALUE);
    clmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize), CL_INVALID_VALUE);

    /* Construct kernel binary. */
    clmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary), CL_OUT_OF_HOST_MEMORY);

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
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }
    gcmOS_SAFE_FREE(gcvNULL, pointer);
    /* Load kernel binary uniforms with the given kernel name */
    status = gcSHADER_LoadKernel(kernelBinary, kernel->name);
    if (gcmIS_ERROR(status))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-007004: (clCreateKernel) Cannot extract kernel (%s) from program.\n",
            kernel->name);
        clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
    }

    /* Set the required work group size. */
    clmONERROR(gcSHADER_GetKernelFunctionByName(kernelBinary, kernel->name, &kernelFunction), CL_INVALID_KERNEL_NAME);
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, &propertySize, &propertyType, (gctINT *)propertyValues);

        if (propertyType == gcvPROPERTY_REQD_WORK_GRP_SIZE)
        {
            kernel->compileWorkGroupSize[0] = propertyValues[0];
            kernel->compileWorkGroupSize[1] = propertyValues[1];
            kernel->compileWorkGroupSize[2] = propertyValues[2];

            if (!(propertyValues[0] == 0 &&
                  propertyValues[1] == 0 &&
                  propertyValues[2] == 0))
            {
                gcoOS_MemCopy(kernelBinary->shaderLayout.compute.workGroupSize,
                    propertyValues,
                    gcmSIZEOF(gctINT) * propertySize);

                kernelBinary->shaderLayout.compute.isWorkGroupSizeFixed = gcvTRUE;
            }
        }
        else if (propertyType == gcvPROPERTY_WORK_GRP_SIZE_HINT)
        {
            /* if reqd_work_grp_size is not set */
            if ((kernelBinary->shaderLayout.compute.workGroupSize[0] == 0 &&
                 kernelBinary->shaderLayout.compute.workGroupSize[1] == 0 &&
                 kernelBinary->shaderLayout.compute.workGroupSize[2] == 0))
            {
                gcoOS_MemCopy(kernelBinary->shaderLayout.compute.workGroupSize,
                    propertyValues,
                    gcmSIZEOF(gctINT) * propertySize);
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
            if (kernelFunction && GetKFunctionIsMain(kernelFunction))
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
                    if ((type == gcSHADER_IMAGE_2D) || (type == gcSHADER_IMAGE_3D) ||
                        (type == gcSHADER_IMAGE_1D) || (type == gcSHADER_IMAGE_1D_ARRAY) ||
                        (type == gcSHADER_IMAGE_1D_BUFFER) || (type == gcSHADER_IMAGE_2D_ARRAY))
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
                if ((type == gcSHADER_IMAGE_2D) || (type == gcSHADER_IMAGE_3D) ||
                    (type == gcSHADER_IMAGE_1D) || (type == gcSHADER_IMAGE_1D_ARRAY) ||
                    (type == gcSHADER_IMAGE_1D_BUFFER) || (type == gcSHADER_IMAGE_2D_ARRAY))
                {
                    break;
                }
            }

            if (i < GetShaderUniformCount(kernelBinary))
            {
                if((supportImageInst == gcvTRUE) && (type == gcSHADER_IMAGE_2D || type == gcSHADER_IMAGE_1D))
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
                status = gcLoadCLPatchLibrary(gcvNULL);
                if (gcmIS_ERROR(status))
                {
                    /* TODO - Patching is not allowed. */
                }
            }
        }
    }

    gcmASSERT(kernel->context->platform->compiler11);
    gcSetCLCompiler(kernel->context->platform->compiler11);
    chipModel = kernel->context->devices[0]->deviceInfo.chipModel;
    chipRevision = kernel->context->devices[0]->deviceInfo.chipRevision;

    clmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &tmpBinarySize), CL_INVALID_VALUE);
    clmONERROR(gcoOS_Allocate(gcvNULL, tmpBinarySize, &savedTmpBinary), CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcSHADER_SaveEx(pgmBinary, savedTmpBinary, &tmpBinarySize), CL_INVALID_VALUE);

    status = gcLinkKernel(kernelBinary,
                          flags | gcvSHADER_REMOVE_UNUSED_UNIFORMS,
                          &programState);

    if((status == gcvSTATUS_NOT_FOUND || status == gcvSTATUS_OUT_OF_RESOURCES) &&  gcmOPT_INLINELEVEL() != 4)
    {
        gcmASSERT(savedTmpBinary);

        /* Construct kernel binary. */
        if(tmpBinary == gcvNULL)
            clmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &tmpBinary), CL_OUT_OF_HOST_MEMORY);

        /* Load kernel binary from program binary */
        status = gcSHADER_LoadEx(tmpBinary, savedTmpBinary, tmpBinarySize);
        if (gcmIS_ERROR(status))
        {
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
        }
        /* Load temp binary uniforms with the given kernel name */
        clmONERROR(gcSHADER_LoadKernel(tmpBinary, kernel->name), CL_OUT_OF_HOST_MEMORY);

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
            clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
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
        clmRETURN_ERROR(CL_OUT_OF_RESOURCES);
    }

OnSkipOutOfSampler:


    kernel->states.binary          = (gctUINT8_PTR) kernelBinary;
    kernel->states.programState    = programState;

    if(gcShaderHasInt64(kernelBinary))
        kernel->patchNeeded = gcvTRUE;

    if(programState.hints)
    {
        /* relax the maxworkgroupsize while shader used barrier as HW limit*/
        for (j = 0; j < GetShaderCodeCount(kernelBinary); j++)
        {
            gcSL_INSTRUCTION inst   = GetShaderInstruction(kernelBinary, j);
            if(gcmSL_OPCODE_GET(inst->opcode, Opcode) == gcSL_BARRIER)
            {
                hasBarrier = gcvTRUE;
            }

            if(gcSL_isOpcodeImageWrite(gcmSL_OPCODE_GET(inst->opcode, Opcode)))
            {
                hasImageWrite = gcvTRUE;
            }
        }

        if (programState.hints->threadWalkerInPS)
        {
            if(hasBarrier && hasImageWrite)
            {
                 kernel->maxWorkGroupSize = (gctUINT32)(maxRegCount / gcmMAX(2, kernel->states.programState.hints->fsMaxTemp+3)) *
                    4 * kernel->program->devices[0]->deviceInfo.maxComputeUnits;
            }
            else
            {
                kernel->maxWorkGroupSize = (gctUINT32)(maxRegCount / gcmMAX(2, kernel->states.programState.hints->fsMaxTemp)) *
                    4 * kernel->program->devices[0]->deviceInfo.maxComputeUnits;
            }
        }
        else
        {
            if(hasBarrier && hasImageWrite)
            {
                kernel->maxWorkGroupSize = (gctUINT32)(maxRegCount / gcmMAX(2, kernel->states.programState.hints->vsMaxTemp+3)) *
                                       4 * kernel->program->devices[0]->deviceInfo.maxComputeUnits;
            }
            else
            {
                kernel->maxWorkGroupSize = (gctUINT32)(maxRegCount / gcmMAX(2, kernel->states.programState.hints->vsMaxTemp)) *
                                       4 * kernel->program->devices[0]->deviceInfo.maxComputeUnits;
            }
        }
    }

    /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
    if (kernel->maxWorkGroupSize > Program->devices[0]->deviceInfo.maxWorkGroupSize)
    {
        kernel->maxWorkGroupSize = Program->devices[0]->deviceInfo.maxWorkGroupSize;
    }

    {
        if((chipModel == gcv4000) && (chipRevision == 0x5245)
            && (patchId == gcvPATCH_OCLCTS))
        {
             kernel->maxWorkGroupSize = gcmMIN(kernel->maxWorkGroupSize, 480);
        }
    }

    /* Set attributes - global id, local id, and group id. */

    /* Get the number of vertex attributes. */

    /* Get the number of attributes. */

    gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &kernel->attributeCount));


    /* Get the number of uniforms. */
    gcmVERIFY_OK(gcSHADER_GetKernelUniformCount(kernelBinary, &kernel->numArgs));

    /* Allocate kernel arguments. */
    clmONERROR(clfAllocateKernelArgs(kernel), CL_OUT_OF_HOST_MEMORY);

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

        if(kernel->referenceCount) gcoOS_AtomDestroy(gcvNULL, kernel->referenceCount);

        if(kernel->name) kernel->name = gcvNULL;

        if(kernel->argMutex) gcoOS_DeleteMutex(gcvNULL, kernel->argMutex);

        gcmOS_SAFE_FREE(gcvNULL, kernel);
    }

    if(pointer != gcvNULL) gcmOS_SAFE_FREE(gcvNULL, pointer);

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

    for (i = 0; i < kernelCount; i++)
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

        if(Kernel->context->devices[0]->deviceInfo.supportIMGInstr &&
          (Kernel->patchNeeded == gcvFALSE))
        {
            gcKERNEL_FUNCTION   kernelFunction = gcvNULL;
            unsigned int i = 0;
            if(Kernel->program->buildOptions != gcvNULL && strstr(Kernel->program->buildOptions, "-cl-viv-vx-extension") != gcvNULL)
            {
                for(i = 0; i<Kernel->numArgs; i++)
                {
                    if(Kernel->args[i].uniform != gcvNULL && isUniformImage2D(Kernel->args[i].uniform))
                    {
                        clsMem_PTR memObj = *(clsMem_PTR *)Kernel->args[i].data;
                        memObj->u.image.vxcaddressingMode = sampler->addressingMode;
                        memObj->u.image.vxcfilterMode = sampler->filterMode;
                        memObj->u.image.vxcnormalizedCoords = sampler->normalizedCoords;
                        break;
                    }
                }
            }
            else
            {
                gctUINT32           orgArgNum = 0, binarySize = 0;
                gctPOINTER          pointer;
                gctUINT32           i;
                gcsPROGRAM_STATE    programState = {0};
                gcSHADER            pgmBinary, kernelBinary;
                gctUINT32_PTR       comVersion;

                Kernel->patchNeeded = gcvTRUE;
                /* Save program binary into buffer */
                pgmBinary = (gcSHADER) Kernel->program->binary;
                clmONERROR(gcSHADER_SaveEx(pgmBinary, gcvNULL, &binarySize), CL_INVALID_VALUE);
                clmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer), CL_OUT_OF_HOST_MEMORY);
                clmONERROR(gcSHADER_SaveEx(pgmBinary, pointer, &binarySize), CL_INVALID_VALUE);

                /* Construct kernel binary. */
                clmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary), CL_OUT_OF_HOST_MEMORY);
                gcmONERROR(gcSHADER_GetCompilerVersion(pgmBinary, &comVersion));
                gcmONERROR(gcSHADER_SetCompilerVersion(kernelBinary, comVersion));

                /* Load kernel binary from program binary */
                status = gcSHADER_LoadEx(kernelBinary, pointer, binarySize);
                if (gcmIS_ERROR(status))
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-007003: (clCreateKernel) Cannot extract kernel from program.\n");
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
                }
                gcoOS_Free(gcvNULL, pointer);

                /* Load kernel binary uniforms with the given kernel name */
                status = gcSHADER_LoadKernel(kernelBinary, Kernel->name);
                if (gcmIS_ERROR(status))
                {
                    clmRETURN_ERROR(CL_OUT_OF_HOST_MEMORY);
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

                gcmVERIFY_OK(gcSHADER_GetKernelUniformCount((gcSHADER)(Kernel->states.binary), &orgArgNum));

                gcmASSERT(Kernel->context->platform->compiler11);
                gcSetCLCompiler(Kernel->context->platform->compiler11);
                status = gcLinkKernel(kernelBinary,
                                      gcvSHADER_RESOURCE_USAGE | gcvSHADER_OPTIMIZER | gcvSHADER_IMAGE_PATCHING | gcvSHADER_REMOVE_UNUSED_UNIFORMS | gcSHADER_HAS_IMAGE_IN_KERNEL,
                                      &programState);
                if (gcmIS_ERROR(status))
                {
                    clmRETURN_ERROR(CL_OUT_OF_RESOURCES);
                }

                if(Kernel->states.binary)
                {
                    gcSHADER_Destroy((gcSHADER)(Kernel->states.binary));
                }
                gcFreeProgramState(Kernel->states.programState);

                Kernel->states.binary          = (gctUINT8_PTR) kernelBinary;
                Kernel->states.programState    = programState;

                gcmVERIFY_OK(gcSHADER_GetAttributeCount(kernelBinary, &Kernel->attributeCount));
                gcmVERIFY_OK(gcSHADER_GetKernelUniformCount((gcSHADER)(Kernel->states.binary), &Kernel->numArgs));

                /* Allocate kernel arguments. */
                clmONERROR(clfReallocateKernelUinformArgs(orgArgNum, Kernel), CL_OUT_OF_HOST_MEMORY);

                argument = clfGetKernelArg(Kernel, ArgIndex, gcvNULL, gcvNULL, gcvNULL);
            }
        }

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
        numArg = clfGetKernelNumArg(Kernel);
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
    cl_kernel_arg_address_qualifier     addQualifier;
    cl_kernel_arg_type_qualifier        typeQualifier = CL_KERNEL_ARG_TYPE_NONE;
    cl_kernel_arg_access_qualifier      accessQualifier = CL_KERNEL_ARG_ACCESS_NONE;
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
        {
            gceUNIFORM_FLAGS                    flags;
            clmONERROR(gcUNIFORM_GetFlags(argument->uniform, &flags),
                      CL_INVALID_VALUE);
            retParamSize = gcmSIZEOF(cl_kernel_arg_address_qualifier);
            switch (flags & 0xff)  /*low 8 bits stand for uniform kind*/
            {
            case gcvUNIFORM_KIND_KERNEL_ARG_CONSTANT:
                addQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG_LOCAL:
                addQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG_PRIVATE:
                addQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            case gcvUNIFORM_KIND_KERNEL_ARG:
                {
                    gctBOOL isPointer;
                    clmONERROR(gcUNIFORM_GetFormat(argument->uniform, gcvNULL, &isPointer),
                              CL_INVALID_VALUE);
                    if (isPointer)
                    {
                        addQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
                    }
                    else
                    {
                        addQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                    }
                }
                break;
            default:
                /* default value if address qualifier is not set */
                addQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
                break;
            }
        }
        retParamPtr = &addQualifier;
        break;

    case CL_KERNEL_ARG_ACCESS_QUALIFIER:
        {
            gcSHADER_TYPE type;
            clmONERROR(gcUNIFORM_GetType(argument->uniform, &type, gcvNULL),
                       CL_INVALID_VALUE);
            switch (type)
            {
            case gcSHADER_IMAGE_2D:
            case gcSHADER_IMAGE_3D:
            case gcSHADER_IMAGE_2D_ARRAY:
            case gcSHADER_IMAGE_1D:
            case gcSHADER_IMAGE_1D_ARRAY:
            case gcSHADER_IMAGE_1D_BUFFER:
                {
                    gctTYPE_QUALIFIER aqualifier = argument->uniform->qualifier;

                    if (aqualifier & gcvTYPE_QUALIFIER_READ_ONLY)
                        accessQualifier = CL_KERNEL_ARG_ACCESS_READ_ONLY;
                    if (aqualifier & gcvTYPE_QUALIFIER_WRITE_ONLY)
                        accessQualifier = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
                }
                break;
            default:
                break;
            }
            retParamSize = gcmSIZEOF(cl_kernel_arg_access_qualifier);
            retParamPtr = &accessQualifier;
        }
        break;

    case CL_KERNEL_ARG_TYPE_NAME:
        {
            #define typeNameLen 64
            gcSL_FORMAT   format;
            gcSHADER_TYPE type;
            gctCHAR       typeName[typeNameLen]={0};
            gctUINT32     length;
            gctSIZE_T     nameOffset = 0;

            clmONERROR(gcUNIFORM_GetType(argument->uniform, &type, &length),
                       CL_INVALID_VALUE);
            clmONERROR(gcUNIFORM_GetFormat(argument->uniform, &format, gcvNULL),
                      CL_INVALID_VALUE);
            nameOffset = GetUniformTypeNameOffset(argument->uniform);

            #define CASE(CONDITION,CLC_TYPE) \
            case CONDITION: \
                gcoOS_StrCopySafe(typeName,typeNameLen,#CLC_TYPE); \
                break;
            #define DEFAULT \
            default: \
                gcmASSERT(gcvFALSE); \
                break;
            if (nameOffset == -1)
            {
                switch (type)
                {
                CASE(gcSHADER_SAMPLER_1D,sampler_t);
                CASE(gcSHADER_IMAGE_2D,image2d_t);
                CASE(gcSHADER_IMAGE_3D,image3d_t);
                CASE(gcSHADER_SAMPLER,sampler_t);
                CASE(gcSHADER_IMAGE_2D_ARRAY,image2d_array_t);
                CASE(gcSHADER_IMAGE_1D,image1d_t);
                CASE(gcSHADER_IMAGE_1D_ARRAY,image1d_array_t);
                CASE(gcSHADER_IMAGE_1D_BUFFER,image1d_buffer_t);

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
                           gctUINT offset = gcoOS_StrLen(typeName,gcvNULL);

                           gcmVERIFY_OK(gcoOS_PrintStrSafe(typeName,
                                                           typeNameLen,
                                                           &offset,
                                                           "%d*",
                                                           vectorSize));
                        }
                        else
                        {
                            gcoOS_StrCatSafe(typeName,typeNameLen,"*");
                        }
                    }
                    else
                    {
                        switch (type)
                        {
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
                gctCHAR* name = ((gcSHADER)(Kernel->states.binary))->typeNameBuffer + nameOffset;
                switch (GetFormatSpecialType(format))
                {
                case gcSL_STRUCT:
                    gcoOS_StrCopySafe(typeName,typeNameLen,"struct ");
                    break;
                case gcSL_UNION:
                    gcoOS_StrCopySafe(typeName,typeNameLen,"union ");
                    break;
                case gcSL_ENUM:
                    gcoOS_StrCopySafe(typeName,typeNameLen,"enum ");
                    break;
                }
                gcoOS_StrCatSafe(typeName,typeNameLen,name);
                if(isUniformPointer(argument->uniform))
                {
                    gcoOS_StrCatSafe(typeName,typeNameLen,"*");
                }
            }
            retParamSize = gcoOS_StrLen(typeName,gcvNULL)+1;
            retParamPtr = typeName;
        }
        break;

    case CL_KERNEL_ARG_TYPE_QUALIFIER:
        {
            gctTYPE_QUALIFIER tqualifier = argument->uniform->qualifier;
            retParamSize = gcmSIZEOF(cl_kernel_arg_type_qualifier);

            if (tqualifier & gcvTYPE_QUALIFIER_VOLATILE)
                typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
            if (tqualifier & gcvTYPE_QUALIFIER_CONST)
                typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
            if (tqualifier & gcvTYPE_QUALIFIER_RESTRICT)
                typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;

             retParamPtr = &typeQualifier;
        }
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

