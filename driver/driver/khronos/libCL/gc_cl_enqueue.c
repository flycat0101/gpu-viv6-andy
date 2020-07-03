/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_cl_precomp.h"

#define __NEXT_MSG_ID__     010319
#define _GC_OBJ_ZONE        gcdZONE_CL_ENQUEUE

/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gceSTATUS
clfCreateGlobalWorkSizeDirective(
    gctUINT                     RealGlobalWorkSize,
    gctBOOL                     PatchRealGlobalWorkSize,
    OUT clsPatchDirective  **   PatchDirectivePtr
    )
{
    gceSTATUS                   status  = gcvSTATUS_OK;
    clsPatchDirective *         pointer = gcvNULL;
    clsPatchGlobalWorkSize *    cf      = gcvNULL;

    gcmHEADER_ARG("PatchDirectivePtr=0x%x", PatchDirectivePtr);
    gcmASSERT(PatchDirectivePtr != gcvNULL);

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchDirective),
                              (gctPOINTER *)&pointer),
               CL_OUT_OF_HOST_MEMORY);

    /* link the new directive to existing directive */
    pointer->next      = *PatchDirectivePtr;
    pointer->kind      = gceRK_PATCH_GLOBAL_WORK_SIZE;

    *PatchDirectivePtr = pointer;

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchGlobalWorkSize),
                              (gctPOINTER *)&cf),
               CL_OUT_OF_HOST_MEMORY);

    gcoOS_ZeroMemory(cf, gcmSIZEOF(clsPatchGlobalWorkSize));

    cf->patchRealGlobalWorkSize = PatchRealGlobalWorkSize;
    cf->realGlobalWorkSize = RealGlobalWorkSize;
    pointer->patchValue.globalWorkSize  = cf;

    gcmFOOTER();
    return status;


OnError:
    if(pointer){
    gcoOS_Free(gcvNULL,
               pointer);
    }
    if(cf){
    gcoOS_Free(gcvNULL,
               cf);
    }
    gcmFOOTER();
    return status;
}


#define NEED_PATCH_LONGULONG(inst, opcode, dstFormat) \
    ((opcode == gcSL_LSHIFT)  ||\
     (opcode == gcSL_RSHIFT)  ||\
     (opcode == gcSL_ADD)     ||\
     (opcode == gcSL_ADDLO)   ||\
     (opcode == gcSL_ADDSAT)  ||\
     (opcode == gcSL_SUB)     ||\
     (opcode == gcSL_SUBSAT)  ||\
     (opcode == gcSL_DIV)     ||\
     (opcode == gcSL_MUL)     ||\
     (opcode == gcSL_MULHI)   ||\
     (opcode == gcSL_MULLO)   ||\
     (opcode == gcSL_MULSAT)  ||\
     (opcode == gcSL_MADSAT) ||\
     (opcode == gcSL_MOD)  ||\
     (opcode == gcSL_I2F)     ||\
     (opcode == gcSL_F2I)     ||\
     (opcode == gcSL_CONV && (dstFormat == gcSL_INT8 || dstFormat == gcSL_UINT8 || dstFormat == gcSL_INT16 || dstFormat == gcSL_UINT16 || dstFormat == gcSL_INT32 || dstFormat == gcSL_UINT32 || dstFormat == gcSL_INT64 || dstFormat == gcSL_UINT64) && (gcmSL_OPCODE_GET((inst->opcode), Sat) == gcSL_SATURATE)) || \
     (opcode == gcSL_CONV && (dstFormat == gcSL_FLOAT || gcmSL_SOURCE_GET(inst->source0, Format) == gcSL_FLOAT)) ||\
     (opcode == gcSL_CMP) ||\
     (opcode == gcSL_ABS) ||\
     (opcode == gcSL_ROTATE) ||\
     (opcode == gcSL_POPCOUNT) ||\
     (opcode == gcSL_LEADZERO) ||\
     (opcode == gcSL_MAX) ||\
     (opcode == gcSL_MIN)\
    )
static gctUINT
_countbits(gctUINT bits, gctINT n)
{
    gctUINT count = 0;
    gctINT i;

    for (i = 0; i < n; i++)
    {
        count += (bits & 1);
        bits >>= 1;
    }

    return count;
}

gceSTATUS
clfCreateLongULongDirective(
    IN  gcSL_INSTRUCTION     Instruction,
    IN  gctUINT              InstructionIndex,
    OUT clsPatchDirective  **PatchDirectivePtr
)
{
    gceSTATUS                   status = gcvSTATUS_INVALID_ARGUMENT;
    clsPatchDirective *         pointer;
    clsPatchLongULong *         longULong;
    gctUINT                     channelBits;
    gctUINT dstFormat = gcmSL_TARGET_GET(GetInstTemp(Instruction), Format);
    gctUINT opcode = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);

    gcmHEADER_ARG("Instruction = %p, InstructionIndex = %u, PatchDirectivePtr = %p",
                  Instruction, InstructionIndex, PatchDirectivePtr);
    gcmASSERT(PatchDirectivePtr != gcvNULL);

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchDirective),
                              (gctPOINTER *)&pointer),
                              CL_OUT_OF_HOST_MEMORY);


    /* link the new directive to existing directive */
    pointer->next      = *PatchDirectivePtr;
    pointer->kind      = gceRK_PATCH_CL_LONGULONG;
    *PatchDirectivePtr = pointer;

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchLongULong),
                              (gctPOINTER *) &longULong),
                              CL_OUT_OF_HOST_MEMORY);

    pointer->patchValue.longULong = longULong;
    longULong->instruction      = Instruction;
    longULong->instructionIndex = InstructionIndex;

    if (NEED_PATCH_LONGULONG(Instruction, opcode, dstFormat))
    {
        channelBits = gcmSL_TARGET_GET(GetInstTemp(Instruction), Enable);
        longULong->channelCount = _countbits(channelBits, 4);
    }
OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
clfCreateReadImageDirective(
    IN clsImageHeader_PTR       ImageHeader,
    IN gctUINT                  SamplerNum,
    IN gctUINT                  SamplerValue,
    IN gctUINT                  ChannelDataType,
    IN gctUINT                  ChannelOrder,
    IN gceTILING                Tiling,
    OUT clsPatchDirective  **   PatchDirectivePtr
    )
{
    gceSTATUS                   status = gcvSTATUS_OK;
    clsPatchDirective *         pointer = gcvNULL;
    clsPatchReadImage *         readImage = gcvNULL;

    gcmHEADER_ARG("SamplerNum=%d SamplerValue=0x%x PatchDirectivePtr=0x%x",
                  SamplerNum, SamplerValue, PatchDirectivePtr);
    gcmASSERT(PatchDirectivePtr != gcvNULL);

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchDirective),
                             (gctPOINTER *)&pointer),
               CL_OUT_OF_HOST_MEMORY);

    /* link the new directive to existing directive */
    pointer->next      = *PatchDirectivePtr;
    pointer->kind      = gceRK_PATCH_READ_IMAGE;
    *PatchDirectivePtr = pointer;

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchReadImage),
                             (gctPOINTER *) &readImage),
               CL_OUT_OF_HOST_MEMORY);

    pointer->patchValue.readImage = readImage;
    readImage->imageHeader        = ImageHeader;
    readImage->samplerNum         = SamplerNum;
    readImage->samplerValue       = SamplerValue;
    readImage->channelDataType    = ChannelDataType;
    readImage->channelOrder       = ChannelOrder;
    readImage->tiling             = Tiling;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
clfCreateWriteImageDirective(
    IN clsImageHeader_PTR       ImageHeader,
    IN gctUINT                  ImageNum,
    IN gctUINT                  ChannelDataType,
    IN gctUINT                  ChannelOrder,
    IN gceTILING                Tiling,
    OUT clsPatchDirective  **   PatchDirectivePtr
    )
{
    gceSTATUS                   status = gcvSTATUS_OK;
    clsPatchDirective *         pointer = gcvNULL;
    clsPatchWriteImage *        writeImage = gcvNULL;

    gcmHEADER_ARG("ImageHeader=0x%x PatchDirectivePtr=0x%x",
                  ImageHeader, PatchDirectivePtr);
    gcmASSERT(PatchDirectivePtr != gcvNULL);

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchDirective),
                             (gctPOINTER *)&pointer),
               CL_OUT_OF_HOST_MEMORY);

    /* link the new directive to existing directive */
    pointer->next      = *PatchDirectivePtr;
    pointer->kind      = gceRK_PATCH_WRITE_IMAGE;
    *PatchDirectivePtr = pointer;

    clmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(clsPatchWriteImage),
                             (gctPOINTER *) &writeImage),
               CL_OUT_OF_HOST_MEMORY);

    pointer->patchValue.writeImage  = writeImage;
    writeImage->imageHeader         = ImageHeader;
    writeImage->imageNum            = ImageNum;
    writeImage->channelDataType     = ChannelDataType;
    writeImage->channelOrder        = ChannelOrder;
    writeImage->tiling              = Tiling;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
clfDestroyPatchDirective(
    IN OUT clsPatchDirective ** PatchDirectivePtr
    )
{
    gceSTATUS           status        = gcvSTATUS_OK;
    clsPatchDirective * curDirective  = *PatchDirectivePtr;
    clsPatchDirective * nextDirective;

    while (curDirective)
    {
        nextDirective = curDirective->next;

        switch(curDirective->kind)
        {
        case gceRK_PATCH_GLOBAL_WORK_SIZE:
            gcmONERROR(gcoOS_Free(gcvNULL,
                           curDirective->patchValue.globalWorkSize));
            break;

        case gceRK_PATCH_READ_IMAGE:
            gcmONERROR(gcoOS_Free(gcvNULL,
                           curDirective->patchValue.readImage));
            break;

        case gceRK_PATCH_WRITE_IMAGE:
            gcmONERROR(gcoOS_Free(gcvNULL,
                           curDirective->patchValue.writeImage));
            break;

        case gceRK_PATCH_CL_LONGULONG:
            gcmONERROR(gcoOS_Free(gcvNULL,
                           curDirective->patchValue.longULong));
            break;

        default:
            gcmASSERT(gcvFALSE);  /* not implemented yet */
            break;
        }
        gcmONERROR(gcoOS_Free(gcvNULL, curDirective));

        curDirective = nextDirective;
    }
    *PatchDirectivePtr = gcvNULL;

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS clfUpdateKernelArgs(
    cl_kernel               Kernel,
    gctUINT *               NumArgs,
    clsArgument_PTR *       Args,
    clsPatchDirective_PTR   RecompilePatchDirectives)
{
    gceSTATUS               status = gcvSTATUS_OK;
    clsPatchDirective_PTR   patchDirective, prePatchDirective;
    gctUINT                 numToUpdate = *NumArgs, i;
    struct _imageData {
        gctUINT             physical;   /* GPU address is 32-bit. */
        gctUINT             pitch;
        gctUINT             slice;      /* 3D/2D array. */
    } imageData;

    for (patchDirective = RecompilePatchDirectives, prePatchDirective = Kernel->recompileInstance->patchDirective;
         patchDirective;
         patchDirective = patchDirective->next, prePatchDirective = prePatchDirective->next)
    {
        clsPatchReadImage *     readImage;
        clsPatchWriteImage *    writeImage;
        clsImageHeader_PTR      imageHeader;
        clsArgument_PTR         argument;
        gctSIZE_T               bytes;
        gcUNIFORM               gcUniform1, gcUniform2;
        gctUINT                 oldNumArgs;

        switch (patchDirective->kind)
        {
        case gceRK_PATCH_READ_IMAGE:
            readImage = patchDirective->patchValue.readImage;
            imageHeader = readImage->imageHeader;

            /* Add new uniforms. */
            oldNumArgs = *NumArgs;
            *NumArgs += 2;
            clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                               *NumArgs,
                                               Args),
                       CL_OUT_OF_HOST_MEMORY);

            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->recompileInstance->binary, prePatchDirective->patchValue.readImage->imageDataIndex, &gcUniform1), CL_INVALID_VALUE);
            argument = &((*Args)[oldNumArgs]);
            bytes = sizeof(struct _imageData);
            imageData.physical = imageHeader->physical;
            imageData.pitch    = imageHeader->rowPitch;
            imageData.slice    = imageHeader->slicePitch;
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
            gcoOS_MemCopy(argument->data, &imageData, bytes);
            argument->uniform    = gcUniform1;
            argument->size       = bytes;
            argument->set        = gcvTRUE;

            /* Image's size. */
            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->recompileInstance->binary, prePatchDirective->patchValue.readImage->imageSizeIndex, &gcUniform2), CL_INVALID_VALUE);
            argument = &((*Args)[oldNumArgs + 1]);
            bytes = gcmSIZEOF(cl_uint) * 3;
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);

            if ((imageHeader->imageType != CL_MEM_OBJECT_IMAGE1D_ARRAY) && (imageHeader->imageType != CL_MEM_OBJECT_IMAGE2D_ARRAY))
            {
                gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
            }
            else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE1D_ARRAY)
            {
                cl_uint* p = (cl_uint*)argument->data;
                gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p++, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p, &imageHeader->depth, gcmSIZEOF(cl_uint));
            }
            else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY)
            {
                cl_uint* p = (cl_uint*)argument->data;
                gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p++, &imageHeader->height, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
            }
            argument->uniform    = gcUniform2;
            argument->size       = bytes;
            argument->set        = gcvTRUE;

            break;

        case gceRK_PATCH_WRITE_IMAGE:
            writeImage = patchDirective->patchValue.writeImage;
            imageHeader = writeImage->imageHeader;

            /* Add new uniforms. */
            oldNumArgs = *NumArgs;
            *NumArgs += 2;
            clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                               *NumArgs,
                                               Args),
                       CL_OUT_OF_HOST_MEMORY);

            /* TODO - Need to handle image3D. */

            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->recompileInstance->binary, prePatchDirective->patchValue.writeImage->imageDataIndex, &gcUniform1), CL_INVALID_VALUE);
            argument = &((*Args)[oldNumArgs]);
            bytes = sizeof(struct _imageData);
            imageData.physical = imageHeader->physical;
            imageData.pitch    = imageHeader->rowPitch;
            imageData.slice    = imageHeader->slicePitch;
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
            gcoOS_MemCopy(argument->data, &imageData, bytes);
            argument->uniform    = gcUniform1;
            argument->size       = bytes;
            argument->set        = gcvTRUE;

            /* Image's size. */
            clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->recompileInstance->binary, prePatchDirective->patchValue.writeImage->imageSizeIndex, &gcUniform2), CL_INVALID_VALUE);
            argument++;
            bytes = gcmSIZEOF(cl_uint) * 3;
            clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);

            if ((imageHeader->imageType != CL_MEM_OBJECT_IMAGE1D_ARRAY) && (imageHeader->imageType != CL_MEM_OBJECT_IMAGE2D_ARRAY))
            {
                gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
            }
            else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE1D_ARRAY)
            {
                cl_uint* p = (cl_uint*)argument->data;
                gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p++, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p, &imageHeader->depth, gcmSIZEOF(cl_uint));
            }
            else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY)
            {
                cl_uint* p = (cl_uint*)argument->data;
                gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p++, &imageHeader->height, gcmSIZEOF(cl_uint));
                gcoOS_MemCopy(p, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
            }

            gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
            argument->uniform    = gcUniform2;
            argument->size       = bytes;
            argument->set        = gcvTRUE;

            break;

        case gceRK_PATCH_CL_LONGULONG:
            break;

        default:
            gcmASSERT(gcvFALSE);  /* not implemented yet */
            break;
        }
    }

    for (i = 0; i < numToUpdate; i++)
    {
        gcUNIFORM uniform = gcvNULL;
        clsArgument_PTR tmpArgs = *Args;

        clmONERROR(gcSHADER_GetUniform((gcSHADER) Kernel->recompileInstance->binary, i, &uniform), CL_INVALID_VALUE);

        if(uniform && tmpArgs[i].uniform && (uniform->index == tmpArgs[i].uniform->index))
        {
            tmpArgs[i].uniform = uniform;
        }
    }

OnError:
    return status;
}

static gceSTATUS
clfDynamicPatchKernel(
    cl_kernel               Kernel,
    gctUINT *               NumArgs,
    clsArgument_PTR *       Args,
    clsPatchDirective_PTR   RecompilePatchDirectives
    )
{
    gceSTATUS               status;
    gctUINT                 binarySize;
    gcSHADER                kernelBinary;
    gctPOINTER              pointer = gcvNULL;
    gcsPROGRAM_STATE        programState = {0};
    clsKernelInstance_PTR   recompileInstance = gcvNULL;
    clsPatchDirective_PTR   patchDirective = gcvNULL;
    gcPatchDirective_PTR    gcPatchDirective = gcvNULL;
    gctUINT                 oldNumArgs, numToUpdate = *NumArgs, i;
    gceSHADER_FLAGS         flags = gcvSHADER_RESOURCE_USAGE | gcvSHADER_OPTIMIZER | gcvSHADER_REMOVE_UNUSED_UNIFORMS;
    gctUINT32_PTR           comVersion;
    gctBOOL                 reLinkNeed = gcvFALSE;
    gctPOINTER              orgArgs = gcvNULL;

    struct _imageData {
        gctUINT             physical;   /* GPU address is 32-bit. */
        gctUINT             pitch;
        gctUINT             slice;      /* 3D/2D array. */
    } imageData;

    if (RecompilePatchDirectives == gcvNULL)
    {
        return gcvSTATUS_OK;
    }

DoReLink:
    /* Save program binary into buffer */
    clmONERROR(gcSHADER_SaveEx((gcSHADER)Kernel->masterInstance.binary, gcvNULL, &binarySize),
               CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer),
               CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcSHADER_SaveEx((gcSHADER)Kernel->masterInstance.binary, pointer, &binarySize),
               CL_OUT_OF_HOST_MEMORY);

    /* Construct kernel binary. */
    clmONERROR(gcSHADER_Construct(gcSHADER_TYPE_CL, &kernelBinary),
               CL_OUT_OF_HOST_MEMORY);

    gcmONERROR(gcSHADER_GetCompilerVersion((gcSHADER)Kernel->masterInstance.binary, &comVersion));

    gcmONERROR(gcSHADER_SetCompilerVersion(kernelBinary, comVersion));

    /* Load kernel binary from program binary */
    clmONERROR(gcSHADER_LoadEx(kernelBinary, pointer, binarySize),
               CL_OUT_OF_HOST_MEMORY);

    gcmOS_SAFE_FREE(gcvNULL, pointer);

    clmONERROR(gcSHADER_LoadKernel(kernelBinary, Kernel->name),
               CL_OUT_OF_HOST_MEMORY);

    GetShaderMaxKernelFunctionArgs(kernelBinary) = 0;

    for (patchDirective = RecompilePatchDirectives;
         patchDirective;
         patchDirective = patchDirective->next)
    {
        clsPatchReadImage *     readImage;
        clsPatchWriteImage *    writeImage;
        clsImageHeader_PTR      imageHeader;
        clsArgument_PTR         argument;
        gctSIZE_T               bytes;
        gctSIZE_T               attribCount;
        gctBOOL                 found1, found2;
        gctUINT                 foundCount;
        gcUNIFORM               gcUniform1, gcUniform2/*, gcUniform3*/;
        gctBOOL                 halti5 = clgDefaultDevice->platform->vscCoreSysCtx.hwCfg.hwFeatureFlags.hasHalti5;

        switch (patchDirective->kind)
        {
        case gceRK_PATCH_GLOBAL_WORK_SIZE:
            gcUniform1 = gcUniform2 = gcvNULL;

            /* Check if global_id is used. */
            found1 = found2 = gcvFALSE;
            foundCount = 0;
            for (attribCount = 0; attribCount < GetShaderAttributeCount(kernelBinary); attribCount++)
            {
                if (gcSL_GLOBAL_INVOCATION_ID ==  GetATTRNameLength(GetShaderAttribute(kernelBinary, attribCount)))
                {
                    found1 = gcvTRUE;
                    foundCount++;
                }
            }

            if (!found1 &&
                patchDirective->patchValue.globalWorkSize->patchRealGlobalWorkSize)
            {
                foundCount++;
            }

            /* Check if group_id is used. */
            for (attribCount = 0; attribCount < GetShaderAttributeCount(kernelBinary); attribCount++)
            {
                if (gcSL_WORK_GROUP_ID == GetATTRNameLength(GetShaderAttribute(kernelBinary, attribCount)))
                {
                    found2 = gcvTRUE;
                    foundCount++;
                }
            }

            /* Add new uniforms. */
            oldNumArgs = *NumArgs;
            *NumArgs += foundCount;
            clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                               *NumArgs,
                                               Args),
                       CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(*Args + oldNumArgs, foundCount * gcmSIZEOF(clsArgument));
            argument = &((*Args)[oldNumArgs]);

            if (found1 || patchDirective->patchValue.globalWorkSize->patchRealGlobalWorkSize)
            {
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#global_width", gcSHADER_UINT_X2, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform1),
                           CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform1, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform1, gcSL_UINT32, gcvFALSE),
                           CL_OUT_OF_HOST_MEMORY);
                patchDirective->patchValue.globalWorkSize->globalWidth = gcUniform1;
                argument->uniform    = gcUniform1;
                argument->size       = 0;
                argument->set        = gcvTRUE;
                argument++;
            }

            if (found2)
            {
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#group_width", gcSHADER_UINT_X1, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform2),
                           CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform2, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform2, gcSL_UINT32, gcvFALSE),
                           CL_OUT_OF_HOST_MEMORY);
                patchDirective->patchValue.globalWorkSize->groupWidth = gcUniform2;
                argument->uniform    = gcUniform2;
                argument->size       = 0;
                argument->set        = gcvTRUE;
            }

            clmONERROR(gcCreateGlobalWorkSizeDirective(
                gcUniform1, gcUniform2,
                patchDirective->patchValue.globalWorkSize->patchRealGlobalWorkSize,
                &gcPatchDirective),
                       CL_OUT_OF_HOST_MEMORY);
            break;

        case gceRK_PATCH_READ_IMAGE:
            readImage = patchDirective->patchValue.readImage;
            imageHeader = readImage->imageHeader;

            if((!halti5) || (!NEW_READ_WRITE_IMAGE))
            {
                /* Add new uniforms. */
                oldNumArgs = *NumArgs;
                *NumArgs += 2;
                clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                                   *NumArgs,
                                                   Args),
                           CL_OUT_OF_HOST_MEMORY);

                /* TODO - Need to handle image3D. */

                /* Pack image's data pointer and pitch into one vector to avoid extra MOV. */
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#image_data", gcSHADER_INTEGER_X3, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform1),
                            CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform1, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform1, gcSL_UINT32, gcvFALSE),
                            CL_OUT_OF_HOST_MEMORY);
                argument = &((*Args)[oldNumArgs]);
                bytes = sizeof(struct _imageData);
                imageData.physical = imageHeader->physical;
                imageData.pitch    = imageHeader->rowPitch;
                imageData.slice    = imageHeader->slicePitch;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageData, bytes);
                argument->uniform    = gcUniform1;
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                /* Image's size. */
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#image_size", gcSHADER_INTEGER_X3, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform2),
                            CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform2, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform2, gcSL_UINT32, gcvFALSE),
                            CL_OUT_OF_HOST_MEMORY);
                argument = &((*Args)[oldNumArgs + 1]);
                bytes = gcmSIZEOF(cl_uint) * 3;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                if ((imageHeader->imageType != CL_MEM_OBJECT_IMAGE1D_ARRAY) && (imageHeader->imageType != CL_MEM_OBJECT_IMAGE2D_ARRAY))
                {
                    gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
                }
                else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE1D_ARRAY)
                {
                    cl_uint* p = (cl_uint*)argument->data;
                    gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p++, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p, &imageHeader->depth, gcmSIZEOF(cl_uint));
                }
                else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY)
                {
                    cl_uint* p = (cl_uint*)argument->data;
                    gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p++, &imageHeader->height, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                }
                argument->uniform    = gcUniform2;
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                readImage->imageDataIndex = GetUniformIndex(gcUniform1);
                readImage->imageSizeIndex = GetUniformIndex(gcUniform2);

                clmONERROR(gcCreateReadImageDirective(readImage->samplerNum,
                                                      GetUniformIndex(gcUniform1),
                                                      GetUniformIndex(gcUniform2),
                                                      readImage->samplerValue,
                                                      readImage->channelDataType & 0xF,
                                                      readImage->channelOrder & 0xF,
                                                      imageHeader->imageType,
                                                      &gcPatchDirective),
                           CL_OUT_OF_HOST_MEMORY);
            }
            else
            {
                clmONERROR(gcCreateReadImageDirective(readImage->samplerNum,
                                                  0,
                                                  0,
                                                  readImage->samplerValue,
                                                  readImage->channelDataType & 0xF,
                                                  readImage->channelOrder & 0xF,
                                                  imageHeader->imageType,
                                                  &gcPatchDirective),
                       CL_OUT_OF_HOST_MEMORY);
            }

            break;

        case gceRK_PATCH_WRITE_IMAGE:
            writeImage = patchDirective->patchValue.writeImage;
            imageHeader = writeImage->imageHeader;

            if((!halti5) || (!NEW_READ_WRITE_IMAGE))
            {
                /* Add new uniforms. */
                oldNumArgs = *NumArgs;
                *NumArgs += 2;
                clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                                   *NumArgs,
                                                   Args),
                           CL_OUT_OF_HOST_MEMORY);

                /* TODO - Need to handle image3D. */

                /* Pack image's data pointer and pitch into one vector to avoid extra MOV. */
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#image_data", gcSHADER_INTEGER_X3, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform1),
                            CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform1, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform1, gcSL_UINT32, gcvFALSE),
                            CL_OUT_OF_HOST_MEMORY);
                argument = &((*Args)[oldNumArgs]);
                bytes = sizeof(struct _imageData);
                imageData.physical = imageHeader->physical;
                imageData.pitch    = imageHeader->rowPitch;
                imageData.slice    = imageHeader->slicePitch;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageData, bytes);
                argument->uniform    = gcUniform1;
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                /* Image's size. */
                clmONERROR(gcSHADER_AddUniform(kernelBinary, "#image_size", gcSHADER_INTEGER_X3, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform2),
                           CL_OUT_OF_HOST_MEMORY);
                SetUniformFlag(gcUniform2, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                clmONERROR(gcUNIFORM_SetFormat(gcUniform2, gcSL_UINT32, gcvFALSE),
                           CL_OUT_OF_HOST_MEMORY);
                argument++;
                bytes = gcmSIZEOF(cl_uint) * 3;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                if ((imageHeader->imageType != CL_MEM_OBJECT_IMAGE1D_ARRAY) && (imageHeader->imageType != CL_MEM_OBJECT_IMAGE2D_ARRAY))
                {
                    gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
                }
                else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE1D_ARRAY)
                {
                    cl_uint* p = (cl_uint*)argument->data;
                    gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p++, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p, &imageHeader->depth, gcmSIZEOF(cl_uint));
                }
                else if (imageHeader->imageType == CL_MEM_OBJECT_IMAGE2D_ARRAY)
                {
                    cl_uint* p = (cl_uint*)argument->data;
                    gcoOS_MemCopy(p++, &imageHeader->width, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p++, &imageHeader->height, gcmSIZEOF(cl_uint));
                    gcoOS_MemCopy(p, &imageHeader->arraySize, gcmSIZEOF(cl_uint));
                }

                argument->uniform    = gcUniform2;
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                writeImage->imageDataIndex = GetUniformIndex(gcUniform1);
                writeImage->imageSizeIndex = GetUniformIndex(gcUniform2);

                clmONERROR(gcCreateWriteImageDirective(writeImage->imageNum,
                                                       GetUniformIndex(gcUniform1),
                                                       GetUniformIndex(gcUniform2),
                                                       writeImage->channelDataType & 0xF,
                                                       writeImage->channelOrder & 0xF,
                                                       writeImage->imageHeader->imageType,
                                                       &gcPatchDirective),
                                                       CL_OUT_OF_HOST_MEMORY);
            }
            else
            {
                clmONERROR(gcCreateWriteImageDirective(writeImage->imageNum,
                                                   0,
                                                   0,
                                                   writeImage->channelDataType & 0xF,
                                                   writeImage->channelOrder & 0xF,
                                                   writeImage->imageHeader->imageType,
                                                   &gcPatchDirective),
                                                   CL_OUT_OF_HOST_MEMORY);
            }
            break;

        case gceRK_PATCH_CL_LONGULONG:
            {
                clmONERROR(gcCreateCLLongULongDirective(patchDirective->patchValue.longULong->instructionIndex,
                                                        patchDirective->patchValue.longULong->channelCount,
                                                        &gcPatchDirective),
                                                        CL_OUT_OF_HOST_MEMORY);
            }
            break;

        default:
            gcmASSERT(gcvFALSE);  /* not implemented yet */
            break;
        }
    }

    if(reLinkNeed == gcvFALSE)
    {
        gcoOS_Allocate(gcvNULL,  numToUpdate * gcmSIZEOF(clsArgument), &orgArgs);
        gcoOS_MemCopy(orgArgs, *Args, numToUpdate * gcmSIZEOF(clsArgument));
    }

    /* Recompile shader */
    gcmASSERT(Kernel->context->platform->compiler11);
    gcSetCLCompiler(Kernel->context->platform->compiler11);

    clmONERROR(gcSHADER_DynamicPatch(kernelBinary,
                                     gcPatchDirective,
                                     0),
                                     CL_OUT_OF_HOST_MEMORY);

    /*gcPatchDirective will reset to NULL after this*/
    gcDestroyPatchDirective(&gcPatchDirective);

    /* Assume all dead code is removed by optimizer. */
    status = gcLinkKernel(kernelBinary,
                          flags,
                          &programState);

    if((status == gcvSTATUS_NOT_FOUND || status == gcvSTATUS_OUT_OF_RESOURCES) && gcmOPT_INLINELEVEL() != 4)
    {
        gcFreeProgramState(programState);
        gcSHADER_Destroy(kernelBinary);
        flags |= gcvSHADER_SET_INLINE_LEVEL_4;
        gcoOS_Free(gcvNULL, *Args);
        *Args = orgArgs;
        *NumArgs = numToUpdate;
        reLinkNeed = gcvTRUE;
        goto DoReLink;
    }
    if(reLinkNeed == gcvTRUE)
    {
        reLinkNeed = gcvFALSE;
    }
    else
    {
        gcmOS_SAFE_FREE(gcvNULL, orgArgs);
    }

    if (gcmIS_ERROR(status))
    {
        clmRETURN_ERROR(CL_OUT_OF_RESOURCES);
    }


    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsKernelInstance), &pointer),
               CL_OUT_OF_HOST_MEMORY);


    recompileInstance = (clsKernelInstance_PTR) pointer;
    recompileInstance->next             = Kernel->recompileInstance;
    Kernel->recompileInstance           = recompileInstance;
    recompileInstance->binary           = (gctUINT8_PTR)kernelBinary;
    recompileInstance->programState     = programState;
    recompileInstance->patchDirective   = RecompilePatchDirectives;

    for (i = 0; i < numToUpdate; i++)
    {
        gcUNIFORM uniform = gcvNULL;
        clsArgument_PTR tmpArgs = *Args;

        clmONERROR(gcSHADER_GetUniform((gcSHADER) kernelBinary, i, &uniform), CL_INVALID_VALUE);

        if(uniform && tmpArgs[i].uniform && (uniform->index == tmpArgs[i].uniform->index))
        {
            tmpArgs[i].uniform = uniform;
        }
    }

    return status;

OnError:

    if (gcPatchDirective)
    {
        gcDestroyPatchDirective(&gcPatchDirective);
    }

    /* In case a operation before free memory has error. pointer should free properly. */
    if (pointer)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
    }

    if(orgArgs)
    {
        gcmOS_SAFE_FREE(gcvNULL, orgArgs);
    }

    return status;
}

/*****************************************************************************\
|*                     OpenCL Enqueued Commands API                          *|
\*****************************************************************************/
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingRead,
    size_t              Offset,
    size_t              Cb,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandReadBuffer_PTR readBuffer;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x BlockingRead=%u Offset=%lu Ptr=0x%x",
                  CommandQueue, Buffer, BlockingRead, Offset, Ptr);
    gcmDUMP_API("${OCL clEnqueueReadBuffer 0x%x, 0x%x}", CommandQueue, Buffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010000: (clEnqueueReadBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010001: (clEnqueueReadBuffer) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010002: (clEnqueueReadBuffer) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010003: (clEnqueueReadBuffer) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010004: (clEnqueueReadBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010005: (clEnqueueReadBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingRead && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010006: (clEnqueueReadBuffer) BlockingRead is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    if (Buffer->u.buffer.size < Offset+Cb)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010007: (clEnqueueReadBuffer) (Offset + Cb) is larger than Buffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010210: (clEnqueueReadBuffer) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Buffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_READ_BUFFER;
    command->handler                = &clfExecuteCommandReadBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    readBuffer                      = &command->u.readBuffer;
    readBuffer->buffer              = Buffer;
    readBuffer->blockingRead        = BlockingRead;
    readBuffer->offset              = Offset;
    readBuffer->cb                  = Cb;
    readBuffer->ptr                 = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingRead),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueReadBuffer)(CommandQueue, Buffer, BlockingRead, Offset, Cb, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010008: (clEnqueueReadBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingRead,
    const size_t *      BufferOrigin,
    const size_t *      HostOrigin,
    const size_t *      Region,
    size_t              BufferRowPitch,
    size_t              BufferSlicePitch,
    size_t              HostRowPitch,
    size_t              HostSlicePitch,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    clsCommandReadBufferRect_PTR readBufferRect;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x BlockingRead=%u BufferOrigin=0x%x "
                  "HostOrigin=0x%x Region=0x%x Ptr=0x%x",
                  CommandQueue, Buffer, BlockingRead, BufferOrigin,
                  HostOrigin, Region, Ptr);
    gcmDUMP_API("${OCL clEnqueueReadBufferRect 0x%x, 0x%x}", CommandQueue, Buffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010009: (clEnqueueReadBufferRect) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010010: (clEnqueueReadBufferRect) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (Buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010211: (clEnqueueReadBufferRect) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010011: (clEnqueueReadBufferRect) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010012: (clEnqueueReadBufferRect) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010013: (clEnqueueReadBufferRect) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010014: (clEnqueueReadBufferRect) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingRead && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010015: (clEnqueueReadBufferRect) BlockingRead is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010016: (clEnqueueReadBufferRect) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferRowPitch == 0)
    {
        BufferRowPitch = Region[0];
    }
    if (BufferRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010017: (clEnqueueReadBufferRect) BufferRowPitch (%d) is less than Region[0] (%d).\n",
            BufferRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (HostRowPitch == 0)
    {
        HostRowPitch = Region[0];
    }
    if (HostRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010018: (clEnqueueReadBufferRect) HostRowPitch (%d) is less than Region[0] (%d).\n",
            HostRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferSlicePitch == 0)
    {
        BufferSlicePitch = Region[1] * BufferRowPitch;
    }
    if (BufferSlicePitch < Region[1] * BufferRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010019: (clEnqueueReadBufferRect) BufferSlicePitch (%d) is less than Region[1] (%d) * BufferRowPitch (%d).\n",
            BufferSlicePitch, Region[1], BufferRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (HostSlicePitch == 0)
    {
        HostSlicePitch = Region[1] * HostRowPitch;
    }
    if (HostSlicePitch < Region[1] * HostRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010020: (clEnqueueReadBufferRect) HostSlicePitch (%d) is less than Region[1] (%d) * HostRowPitch (%d).\n",
            HostSlicePitch, Region[1], HostRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if last byte is out of bounds */
    if (Buffer->u.buffer.size <
        BufferOrigin[0] + BufferOrigin[1]*BufferRowPitch + BufferOrigin[2]*BufferSlicePitch +
        Region[0] + (Region[1]-1)*BufferRowPitch + (Region[2]-1)*BufferSlicePitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010021: (clEnqueueReadBufferRect) last byte is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Buffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_READ_BUFFER_RECT;
    command->handler                = &clfExecuteCommandReadBufferRect;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    readBufferRect                  = &command->u.readBufferRect;
    readBufferRect->buffer          = Buffer;
    readBufferRect->blockingRead    = BlockingRead;
    readBufferRect->bufferOrigin[0] = BufferOrigin[0];
    readBufferRect->bufferOrigin[1] = BufferOrigin[1];
    readBufferRect->bufferOrigin[2] = BufferOrigin[2];
    readBufferRect->hostOrigin[0]   = HostOrigin[0];
    readBufferRect->hostOrigin[1]   = HostOrigin[1];
    readBufferRect->hostOrigin[2]   = HostOrigin[2];
    readBufferRect->region[0]       = Region[0];
    readBufferRect->region[1]       = Region[1];
    readBufferRect->region[2]       = Region[2];
    readBufferRect->bufferRowPitch  = BufferRowPitch;
    readBufferRect->bufferSlicePitch= BufferSlicePitch;
    readBufferRect->hostRowPitch    = HostRowPitch;
    readBufferRect->hostSlicePitch  = HostSlicePitch;
    readBufferRect->ptr             = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingRead),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueReadBufferRect)(CommandQueue, Buffer, BlockingRead, BufferOrigin, HostOrigin, Region, BufferRowPitch, BufferSlicePitch, HostRowPitch, HostSlicePitch, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010022: (clEnqueueReadBufferRect) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingWrite,
    size_t              Offset,
    size_t              Cb,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandWriteBuffer_PTR writeBuffer;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x BlockingWrite=%u Offset=%lu Ptr=0x%x",
                  CommandQueue, Buffer, BlockingWrite, Offset, Ptr);
    gcmDUMP_API("${OCL clEnqueueWriteBuffer 0x%x, 0x%x, %d}", CommandQueue, Buffer, BlockingWrite);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010023: (clEnqueueWriteBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010024: (clEnqueueWriteBuffer) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010025: (clEnqueueWriteBuffer) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010026: (clEnqueueWriteBuffer) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010027: (clEnqueueWriteBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010028: (clEnqueueWriteBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingWrite && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010029: (clEnqueueWriteBuffer) BlockingWrite is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    if (Buffer->u.buffer.size < Offset+Cb)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010030: (clEnqueueWriteBuffer) (Offset + Cb) is larger than Buffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010212: (clEnqueueWriteBuffer) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Buffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_WRITE_BUFFER;
    command->handler                = &clfExecuteCommandWriteBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    writeBuffer                     = &command->u.writeBuffer;
    writeBuffer->buffer             = Buffer;
    writeBuffer->blockingWrite      = BlockingWrite;
    writeBuffer->offset             = Offset;
    writeBuffer->cb                 = Cb;
    writeBuffer->ptr                = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingWrite),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueWriteBuffer)(CommandQueue, Buffer, BlockingWrite, Offset, Cb, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010031: (clEnqueueWriteBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingWrite,
    const size_t *      BufferOrigin,
    const size_t *      HostOrigin,
    const size_t *      Region,
    size_t              BufferRowPitch,
    size_t              BufferSlicePitch,
    size_t              HostRowPitch,
    size_t              HostSlicePitch,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandWriteBufferRect_PTR writeBufferRect;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x BlockingWrite=%u BufferOrigin=0x%x "
                  "HostOrigin=0x%x Region=0x%x Ptr=0x%x",
                  CommandQueue, Buffer, BlockingWrite, BufferOrigin,
                  HostOrigin, Region, Ptr);
    gcmDUMP_API("${OCL clEnqueueWriteBufferRect 0x%x, 0x%x}", CommandQueue, Buffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010032: (clEnqueueWriteBufferRect) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010033: (clEnqueueWriteBufferRect) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010213: (clEnqueueWriteBufferRect) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010034: (clEnqueueWriteBufferRect) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010035: (clEnqueueWriteBufferRect) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010036: (clEnqueueWriteBufferRect) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010037: (clEnqueueWriteBufferRect) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingWrite && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010038: (clEnqueueWriteBufferRect) BlockingWrite is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010039: (clEnqueueWriteBufferRect) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferRowPitch == 0)
    {
        BufferRowPitch = Region[0];
    }
    if (BufferRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010040: (clEnqueueWriteBufferRect) BufferRowPitch (%d) is less than Region[0] (%d).\n",
            BufferRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (HostRowPitch == 0)
    {
        HostRowPitch = Region[0];
    }
    if (HostRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010041: (clEnqueueWriteBufferRect) HostRowPitch (%d) is less than Region[0] (%d).\n",
            HostRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (BufferSlicePitch == 0)
    {
        BufferSlicePitch = Region[1] * BufferRowPitch;
    }
    if (BufferSlicePitch < Region[1] * BufferRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010042: (clEnqueueWriteBufferRect) BufferSlicePitch (%d) is less than Region[1] (%d) * BufferRowPitch (%d).\n",
            BufferSlicePitch, Region[1], BufferRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (HostSlicePitch == 0)
    {
        HostSlicePitch = Region[1] * HostRowPitch;
    }
    if (HostSlicePitch < Region[1] * HostRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010043: (clEnqueueWriteBufferRect) HostSlicePitch (%d) is less than Region[1] (%d) * HostRowPitch (%d).\n",
            HostSlicePitch, Region[1], HostRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if last byte is out of bounds */
    if (Buffer->u.buffer.size <
        BufferOrigin[0] + BufferOrigin[1]*BufferRowPitch + BufferOrigin[2]*BufferSlicePitch +
        Region[0] + (Region[1]-1)*BufferRowPitch + (Region[2]-1)*BufferSlicePitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010044: (clEnqueueWriteBufferRect) last byte is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Buffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_WRITE_BUFFER_RECT;
    command->handler                = &clfExecuteCommandWriteBufferRect;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    writeBufferRect                 = &command->u.writeBufferRect;
    writeBufferRect->buffer         = Buffer;
    writeBufferRect->blockingWrite  = BlockingWrite;
    writeBufferRect->bufferOrigin[0]= BufferOrigin[0];
    writeBufferRect->bufferOrigin[1]= BufferOrigin[1];
    writeBufferRect->bufferOrigin[2]= BufferOrigin[2];
    writeBufferRect->hostOrigin[0]  = HostOrigin[0];
    writeBufferRect->hostOrigin[1]  = HostOrigin[1];
    writeBufferRect->hostOrigin[2]  = HostOrigin[2];
    writeBufferRect->region[0]      = Region[0];
    writeBufferRect->region[1]      = Region[1];
    writeBufferRect->region[2]      = Region[2];
    writeBufferRect->bufferRowPitch = BufferRowPitch;
    writeBufferRect->bufferSlicePitch = BufferSlicePitch;
    writeBufferRect->hostRowPitch   = HostRowPitch;
    writeBufferRect->hostSlicePitch = HostSlicePitch;
    writeBufferRect->ptr            = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingWrite),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueWriteBufferRect)(CommandQueue, Buffer, BlockingWrite, BufferOrigin, HostOrigin, Region, BufferRowPitch, BufferSlicePitch, HostRowPitch, HostSlicePitch, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010045: (clEnqueueWriteBufferRect) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstBuffer,
    size_t              SrcOffset,
    size_t              DstOffset,
    size_t              Cb,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandCopyBuffer_PTR copyBuffer;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x SrcBuffer=0x%x DstBuffer=0x%x "
                  "SrcOffset=%u DstOffset=%u",
                  CommandQueue, SrcBuffer, DstBuffer, SrcOffset, DstOffset);
    gcmDUMP_API("${OCL clEnqueueCopyBuffer 0x%x, 0x%x, 0x%x}", CommandQueue, SrcBuffer, DstBuffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010046: (clEnqueueCopyBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (SrcBuffer == gcvNULL ||
        SrcBuffer->objectType != clvOBJECT_MEM ||
        SrcBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010047: (clEnqueueCopyBuffer) invalid SrcBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (DstBuffer == gcvNULL ||
        DstBuffer->objectType != clvOBJECT_MEM ||
        DstBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010048: (clEnqueueCopyBuffer) invalid DstBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != SrcBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010049: (clEnqueueCopyBuffer) CommandQueue's context is not the same as SrcBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }
    if (CommandQueue->context != DstBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010050: (clEnqueueCopyBuffer) CommandQueue's context is not the same as DstBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010051: (clEnqueueCopyBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010052: (clEnqueueCopyBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (SrcBuffer->u.buffer.size < SrcOffset+Cb)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010053: (clEnqueueCopyBuffer) (SrcOffset + Cb) is larger than SrcBuffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
    if (DstBuffer->u.buffer.size < DstOffset+Cb)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010054: (clEnqueueCopyBuffer) (DstOffset + Cb) is larger than DstBuffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcBuffer == DstBuffer &&
        ((SrcOffset > DstOffset) ? (SrcOffset - DstOffset < Cb) : (DstOffset - SrcOffset < Cb)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010055: (clEnqueueCopyBuffer) SrcBuffer is the same as DstBuffer, and regions are overlapped.\n");
        clmRETURN_ERROR(CL_MEM_COPY_OVERLAP);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(SrcBuffer);
    clfRetainMemObject(DstBuffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_COPY_BUFFER;
    command->handler                = &clfExecuteCommandCopyBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    copyBuffer                      = &command->u.copyBuffer;
    copyBuffer->srcBuffer           = SrcBuffer;
    copyBuffer->dstBuffer           = DstBuffer;
    copyBuffer->srcOffset           = SrcOffset;
    copyBuffer->dstOffset           = DstOffset;
    copyBuffer->cb                  = Cb;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueCopyBuffer)(CommandQueue, SrcBuffer, DstBuffer, SrcOffset, DstOffset, Cb, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010056: (clEnqueueCopyBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstBuffer,
    const size_t *      SrcOrigin,
    const size_t *      DstOrigin,
    const size_t *      Region,
    size_t              SrcRowPitch,
    size_t              SrcSlicePitch,
    size_t              DstRowPitch,
    size_t              DstSlicePitch,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandCopyBufferRect_PTR copyBufferRect;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x SrcBuffer=0x%x DstBuffer=0x%x "
                  "SrcOrigin=0x%x DstOrigin=0x%x Region=0x%x",
                  CommandQueue, SrcBuffer, DstBuffer, SrcOrigin, DstOrigin, Region);
    gcmDUMP_API("${OCL clEnqueueCopyBufferRect 0x%x, 0x%x, 0x%x}", CommandQueue, SrcBuffer, DstBuffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010057: (clEnqueueCopyBufferRect) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (SrcBuffer == gcvNULL ||
        SrcBuffer->objectType != clvOBJECT_MEM ||
        SrcBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010058: (clEnqueueCopyBufferRect) invalid SrcBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (DstBuffer == gcvNULL ||
        DstBuffer->objectType != clvOBJECT_MEM ||
        DstBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010059: (clEnqueueCopyBufferRect) invalid DstBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != SrcBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010060: (clEnqueueCopyBufferRect) CommandQueue's context is not the same as SrcBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (CommandQueue->context != DstBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010061: (clEnqueueCopyBufferRect) CommandQueue's context is not the same as DstBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010062: (clEnqueueCopyBufferRect) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010063: (clEnqueueCopyBufferRect) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010064: (clEnqueueCopyBufferRect) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcRowPitch == 0)
    {
        SrcRowPitch = Region[0];
    }
    if (SrcRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010065: (clEnqueueCopyBufferRect) SrcRowPitch (%d) is less than Region[0] (%d).\n",
            SrcRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstRowPitch == 0)
    {
        DstRowPitch = Region[0];
    }
    if (DstRowPitch < Region[0])
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010066: (clEnqueueCopyBufferRect) DstRowPitch (%d) is less than Region[0] (%d).\n",
            DstRowPitch, Region[0]);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcSlicePitch == 0)
    {
        SrcSlicePitch = Region[1] * SrcRowPitch;
    }
    if (SrcSlicePitch < Region[1] * SrcRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010067: (clEnqueueCopyBufferRect) SrcSlicePitch (%d) is less than Region[1] (%d) * SrcRowPitch (%d).\n",
            SrcSlicePitch, Region[1], SrcRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstSlicePitch == 0)
    {
        DstSlicePitch = Region[1] * DstRowPitch;
    }
    if (DstSlicePitch < Region[1] * DstRowPitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010068: (clEnqueueCopyBufferRect) DstSlicePitch (%d) is less than Region[1] (%d) * DstRowPitch (%d).\n",
            DstSlicePitch, Region[1], DstRowPitch);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if last byte is out of bounds */
    if (SrcBuffer->u.buffer.size <
        SrcOrigin[0] + SrcOrigin[1]*SrcRowPitch + SrcOrigin[2]*SrcSlicePitch +
        Region[0] + (Region[1]-1)*SrcRowPitch + (Region[2]-1)*SrcSlicePitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010069: (clEnqueueCopyBufferRect) last byte of source is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstBuffer->u.buffer.size <
        DstOrigin[0] + DstOrigin[1]*DstRowPitch + DstOrigin[2]*DstSlicePitch +
        Region[0] + (Region[1]-1)*DstRowPitch + (Region[2]-1)*DstSlicePitch)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010070: (clEnqueueCopyBufferRect) last byte of destination is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if src and dst overlap */
    if (SrcBuffer == DstBuffer)
    {
        size_t Ax1, Ay1, Az1, Bx1, By1, Bz1, Ax2, Ay2, Az2, Bx2, By2, Bz2;
        Ax1 = SrcOrigin[0];  Ax2 = SrcOrigin[0] + Region[0];
        Ay1 = SrcOrigin[1];  Ay2 = SrcOrigin[1] + Region[1];
        Az1 = SrcOrigin[2];  Az2 = SrcOrigin[2] + Region[2];
        Bx1 = DstOrigin[0];  Bx2 = DstOrigin[0] + Region[0];
        By1 = DstOrigin[1];  By2 = DstOrigin[1] + Region[1];
        Bz1 = DstOrigin[2];  Bz2 = DstOrigin[2] + Region[2];
        if (Ax1 < Bx2 && Ax2 > Bx1 &&
            Ay1 < By2 && Ay2 > By1 &&
            Az1 < Bz2 && Az2 > Bz1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010071: (clEnqueueCopyBufferRect) SrcBuffer is the same as DstBuffer, and regions are overlapped.\n");
            clmRETURN_ERROR(CL_MEM_COPY_OVERLAP);
        }
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(SrcBuffer);
    clfRetainMemObject(DstBuffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_COPY_BUFFER_RECT;
    command->handler                = &clfExecuteCommandCopyBufferRect;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    copyBufferRect                  = &command->u.copyBufferRect;
    copyBufferRect->srcBuffer       = SrcBuffer;
    copyBufferRect->dstBuffer       = DstBuffer;
    copyBufferRect->srcOrigin[0]    = SrcOrigin[0];
    copyBufferRect->srcOrigin[1]    = SrcOrigin[1];
    copyBufferRect->srcOrigin[2]    = SrcOrigin[2];
    copyBufferRect->dstOrigin[0]    = DstOrigin[0];
    copyBufferRect->dstOrigin[1]    = DstOrigin[1];
    copyBufferRect->dstOrigin[2]    = DstOrigin[2];
    copyBufferRect->region[0]       = Region[0];
    copyBufferRect->region[1]       = Region[1];
    copyBufferRect->region[2]       = Region[2];
    copyBufferRect->srcRowPitch     = SrcRowPitch;
    copyBufferRect->srcSlicePitch   = SrcSlicePitch;
    copyBufferRect->dstRowPitch     = DstRowPitch;
    copyBufferRect->dstSlicePitch   = DstSlicePitch;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueCopyBufferRect)(CommandQueue, SrcBuffer, DstBuffer, SrcOrigin, DstOrigin, Region, SrcRowPitch, SrcSlicePitch, DstRowPitch, DstSlicePitch, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010072: (clEnqueueCopyBufferRect) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingRead,
    const size_t *      Origin /*[3]*/,
    const size_t *      Region /*[3]*/,
    size_t              RowPitch,
    size_t              SlicePitch,
    void *              Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandReadImage_PTR readImage;
    size_t              rowPitch;
    size_t              slicePitch;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingRead=%u Origin=0x%x Region=0x%x "
                  "RowPitch=%u SlicePitch=%u Ptr=0x%x",
                  CommandQueue, Image, BlockingRead, Origin, Region, RowPitch, SlicePitch, Ptr);
    gcmDUMP_API("${OCL clEnqueueReadImage 0x%x, 0x%x}", CommandQueue, Image);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010073: (clEnqueueReadImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER ))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010214: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (Image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010215: (clEnqueueReadImage) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (CommandQueue->context != Image->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010075: (clEnqueueReadImage) CommandQueue's context is not the same as Image's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010076: (clEnqueueReadImage) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010077: (clEnqueueReadImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010078: (clEnqueueReadImage) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingRead && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010079: (clEnqueueReadImage) BlockingRead is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010216: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[1] != 0 || Origin[2] != 0 || Region[1] != 1 || Region[2] != 1 || SlicePitch != 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010217: (clEnqueueReadImage) Image is 1D, but Origin[1] or Origin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1 or SlicePitch is not 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010218: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010219: (clEnqueueReadImage) Image is 1D array, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010220: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1 || SlicePitch != 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010221: (clEnqueueReadImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1 or SlicePitch is not 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010080: (clEnqueueReadImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010222: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010080: (clEnqueueReadImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010223: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010080: (clEnqueueReadImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010224: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    rowPitch = Region[0] * Image->u.image.elementSize;
    if (RowPitch == 0)
    {
        RowPitch = rowPitch;
    }
    else
    {
        /* Check if RowPitch is large enough */
        if (RowPitch < rowPitch)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010083: (clEnqueueReadImage) RowPitch (%d) is less than required (%d).\n",
                RowPitch, rowPitch);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    slicePitch =  Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY ? Region[1] * rowPitch : rowPitch;
    if (SlicePitch == 0)
    {
        SlicePitch = slicePitch;
    }
    else
    {
        /* Check if InputRowPitch is large enough */
        if (SlicePitch < slicePitch)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010225: (clEnqueueReadImage) SlicePitch (%d) is less than required (%d).\n",
                SlicePitch, slicePitch);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Image);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_READ_IMAGE;
    command->handler                = &clfExecuteCommandReadImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    readImage                       = &command->u.readImage;
    readImage->image                = Image;
    readImage->blockingRead         = BlockingRead;
    readImage->origin[0]            = Origin[0];
    readImage->origin[1]            = Origin[1];
    readImage->origin[2]            = Origin[2];
    readImage->region[0]            = Region[0];
    readImage->region[1]            = Region[1];
    readImage->region[2]            = Region[2];
    readImage->rowPitch             = RowPitch;
    readImage->slicePitch           = SlicePitch;
    readImage->ptr                  = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingRead),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueReadImage)(CommandQueue, Image, BlockingRead, Origin, Region, RowPitch, SlicePitch, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010084: (clEnqueueReadImage) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingWrite,
    const size_t *      Origin,
    const size_t *      Region,
    size_t              InputRowPitch,
    size_t              InputSlicePitch,
    const void *        Ptr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandWriteImage_PTR writeImage;
    size_t              inputRowPitch;
    size_t              inputSlicePitch;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingWrite=%u Origin=0x%x Region=0x%x"
                  "InputRowPitch=%u InputSlicePitch=%u Ptr=0x%x",
                  CommandQueue, Image, BlockingWrite, Origin, Region,
                  InputRowPitch, InputSlicePitch, Ptr);
    gcmDUMP_API("${OCL clEnqueueWriteImage 0x%x, 0x%x, %d}", CommandQueue, Image, BlockingWrite);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010085: (clEnqueueWriteImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER ))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010226: (clEnqueueWriteImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (Image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010227: (clEnqueueWriteImage) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (CommandQueue->context != Image->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010087: (clEnqueueWriteImage) CommandQueue's context is not the same as Image's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Ptr == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010088: (clEnqueueWriteImage) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010089: (clEnqueueWriteImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010090: (clEnqueueWriteImage) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
            if (BlockingWrite && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010091: (clEnqueueWriteImage) BlockingWrite is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010228: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[1] != 0 || Origin[2] != 0 || Region[1] != 1 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010229: (clEnqueueWriteImage) Image is 1D, but Origin[1] or Origin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010230: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010231: (clEnqueueWriteImage) Image is 1D array, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010232: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010233: (clEnqueueWriteImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010234: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010092: (clEnqueueWriteImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010235: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010092: (clEnqueueWriteImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010236: (clEnqueueWriteImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    inputRowPitch = Region[0] * Image->u.image.elementSize;
    if (InputRowPitch == 0)
    {
        InputRowPitch = inputRowPitch;
    }
    else
    {
        /* Check if InputRowPitch is large enough */
        if (InputRowPitch < inputRowPitch)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010095: (clEnqueueWriteImage) InputRowPitch (%d) is less than required (%d).\n",
                InputRowPitch, inputRowPitch);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    inputSlicePitch = Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY ? Region[1] * inputRowPitch : inputRowPitch;
    if (InputSlicePitch == 0)
    {
        InputSlicePitch = inputSlicePitch;
    }
    else
    {
        /* Check if InputRowPitch is large enough */
        if (InputSlicePitch < inputSlicePitch)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010237: (clEnqueueWriteImage) InputSlicePitch (%d) is less than required (%d).\n",
                InputSlicePitch, inputSlicePitch);
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Image);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_WRITE_IMAGE;
    command->handler                = &clfExecuteCommandWriteImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    writeImage                      = &command->u.writeImage;
    writeImage->image               = Image;
    writeImage->blockingWrite       = BlockingWrite;
    writeImage->origin[0]           = Origin[0];
    writeImage->origin[1]           = Origin[1];
    writeImage->origin[2]           = Origin[2];
    writeImage->region[0]           = Region[0];
    writeImage->region[1]           = Region[1];
    writeImage->region[2]           = Region[2];
    writeImage->inputRowPitch       = InputRowPitch;
    writeImage->inputSlicePitch     = InputSlicePitch;
    writeImage->ptr                 = Ptr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingWrite),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueWriteImage)(CommandQueue, Image, BlockingWrite, Origin, Region, InputRowPitch, InputSlicePitch, Ptr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010096: (clEnqueueWriteImage) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(
    cl_command_queue    CommandQueue ,
    cl_mem              Image ,
    const void *        FillColor ,
    const size_t *      Origin,
    const size_t *      Region,
    cl_uint             NumEventsInWaitList ,
    const cl_event *    EventWaitList ,
    cl_event *          Event
    )
{
    gctINT              status;
    clsCommand_PTR      command = gcvNULL;
    clsCommandFillImage_PTR fillImage;
    gctPOINTER          pointer = gcvNULL;
    gctPOINTER          fillPtr = gcvNULL;
    gctSIZE_T           elementSize;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x FillColor=0x%x "
                  "Origin=0x%x Region=0x%x Region=0x%x",
                  CommandQueue, Image, FillColor, Origin, Region);
    gcmDUMP_API("${OCL clEnqueueFillImage 0x%x, 0x%x}", CommandQueue, Image);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010238: (clEnqueueFillImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (FillColor == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010239: (clEnqueueFillImage) invalid FillColor.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER ))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010240: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != Image->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010241: (clEnqueueFillImage) CommandQueue's context is not the same as SrcImage's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010242: (clEnqueueReadImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010243: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[1] != 0 || Origin[2] != 0 || Region[1] != 1 || Region[2] != 1 )
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010244: (clEnqueueReadImage) Image is 1D, but Origin[1] or Origin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010245: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010246: (clEnqueueReadImage) Image is 1D array, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010247: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1 )
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010248: (clEnqueueReadImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010249: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010250: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010251: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Image);

    if (clfImageFormat2GcFormat(&(Image->u.image.imageFormat), &elementSize, gcvNULL, gcvNULL))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010252: (clEnqueueReadImage) invalid format descriptor.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }



    clmONERROR(gcoOS_Allocate(gcvNULL, elementSize, &fillPtr), CL_OUT_OF_HOST_MEMORY);

    switch (Image->u.image.imageFormat.image_channel_data_type)
    {
    case CL_SIGNED_INT8:
    case CL_SIGNED_INT16:
    case CL_SIGNED_INT32:
        clfPackImagePixeli((cl_int*)FillColor, &Image->u.image.imageFormat, fillPtr);
        break;
    case CL_UNSIGNED_INT8:
    case CL_UNSIGNED_INT16:
    case CL_UNSIGNED_INT32:
        clfPackImagePixelui((cl_uint*)FillColor, &Image->u.image.imageFormat, fillPtr);
        break;
    default:
        clfPackImagePixelf((cl_float*)FillColor, &Image->u.image.imageFormat, fillPtr);
        break;

    }


    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_FILL_IMAGE;
    command->handler                = &clfExecuteCommandFillImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    fillImage                       = &command->u.fillImage;
    fillImage->image                = Image;
    fillImage->origin[0]            = Origin[0];
    fillImage->origin[1]            = Origin[1];
    fillImage->origin[2]            = Origin[2];
    fillImage->region[0]            = Region[0];
    fillImage->region[1]            = Region[1];
    fillImage->region[2]            = Region[2];
    fillImage->elementSize          = elementSize;
    fillImage->fillColorPtr         = fillPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueFillImage)(CommandQueue, Image, FillColor, Origin, Region, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;


OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010253: (clEnqueueFillImage) Run out of memory.\n");
    }

     if (fillPtr)
    {
        gcoOS_Free(gcvNULL, fillPtr);
        command->u.fillImage.fillColorPtr = gcvNULL;
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(
    cl_command_queue    CommandQueue,
    cl_mem              SrcImage,
    cl_mem              DstImage,
    const size_t *      SrcOrigin /*[3]*/,
    const size_t *      DstOrigin /*[3]*/,
    const size_t *      Region /*[3]*/,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandCopyImage_PTR copyImage;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x SrcImage=0x%x DstImage=0x%x "
                  "SrcOrigin=0x%x DstOrigin=0x%x Region=0x%x",
                  CommandQueue, SrcImage, DstImage, SrcOrigin, DstOrigin, Region);
    gcmDUMP_API("${OCL clEnqueueCopyImage 0x%x, 0x%x, 0x%x}", CommandQueue, SrcImage, DstImage);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010097: (clEnqueueCopyImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (SrcImage == gcvNULL ||
        SrcImage->objectType != clvOBJECT_MEM ||
        (SrcImage->type != CL_MEM_OBJECT_IMAGE2D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE3D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER ))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010254: (clEnqueueCopyImage) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (DstImage == gcvNULL ||
        DstImage->objectType != clvOBJECT_MEM ||
        (DstImage->type != CL_MEM_OBJECT_IMAGE2D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE3D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010255: (clEnqueueCopyImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != SrcImage->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010100: (clEnqueueCopyImage) CommandQueue's context is not the same as SrcImage's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (CommandQueue->context != DstImage->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010101: (clEnqueueCopyImage) CommandQueue's context is not the same as DstImage's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (SrcImage->u.image.imageFormat.image_channel_order !=
        DstImage->u.image.imageFormat.image_channel_order)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010102: (clEnqueueCopyImage) SrcImage's channel order is not the same as DstImage's.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (SrcImage->u.image.imageFormat.image_channel_data_type !=
        DstImage->u.image.imageFormat.image_channel_data_type)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010103: (clEnqueueCopyImage) SrcImage's channel data type is not the same as DstImage's.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010104: (clEnqueueCopyImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010105: (clEnqueueCopyImage) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    switch (SrcImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010256: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (SrcOrigin[1] != 0 || SrcOrigin[2] != 0 || Region[1] != 1 || Region[2] != 1 )
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010257: (clEnqueueCopyImage) Image is 1D, but SrcOrigin[1] or SrcOrigin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010258: (clEnqueueCopyImage) (SrcOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (SrcOrigin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010259: (clEnqueueCopyImage) Image is 1D array, but SrcOrigin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010260: (clEnqueueCopyImage) (SrcOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (SrcOrigin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010261: (clEnqueueCopyImage) Image is 2D, but SrcOrigin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010262: (clEnqueueCopyImage) (SrcOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010263: (clEnqueueCopyImage) (SrcOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010264: (clEnqueueCopyImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }
    switch (DstImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010265: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (DstOrigin[1] != 0 || DstOrigin[2] != 0 || Region[1] != 1 || Region[2] != 1 )
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010266: (clEnqueueCopyImage) Image is 1D, but DstOrigin[1] or DstOrigin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
         if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010267: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (DstOrigin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010268: (clEnqueueCopyImage) Image is 1D array, but DstOrigin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010269: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (DstOrigin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010270: (clEnqueueCopyImage) Image is 2D, but DstOrigin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010271: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010106: (clEnqueueCopyImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010272: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010106: (clEnqueueCopyImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010273: (clEnqueueCopyImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    /* Check if src and dst overlap */
    if (SrcImage == DstImage)
    {
        size_t Ax1, Ay1, Az1, Bx1, By1, Bz1, Ax2, Ay2, Az2, Bx2, By2, Bz2;
        Ax1 = SrcOrigin[0];  Ax2 = SrcOrigin[0] + Region[0];
        Ay1 = SrcOrigin[1];  Ay2 = SrcOrigin[1] + Region[1];
        Az1 = SrcOrigin[2];  Az2 = SrcOrigin[2] + Region[2];
        Bx1 = DstOrigin[0];  Bx2 = DstOrigin[0] + Region[0];
        By1 = DstOrigin[1];  By2 = DstOrigin[1] + Region[1];
        Bz1 = DstOrigin[2];  Bz2 = DstOrigin[2] + Region[2];
        if (Ax1 < Bx2 && Ax2 > Bx1 &&
            Ay1 < By2 && Ay2 > By1 &&
            Az1 < Bz2 && Az2 > Bz1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010111: (clEnqueueCopyImage) SrcBuffer is the same as DstBuffer, and regions are overlapped.\n");
            clmRETURN_ERROR(CL_MEM_COPY_OVERLAP);
        }
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(SrcImage);
    clfRetainMemObject(DstImage);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_COPY_IMAGE;
    command->handler                = &clfExecuteCommandCopyImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    copyImage                       = &command->u.copyImage;
    copyImage->srcImage             = SrcImage;
    copyImage->dstImage             = DstImage;
    copyImage->srcOrigin[0]         = SrcOrigin[0];
    copyImage->srcOrigin[1]         = SrcOrigin[1];
    copyImage->srcOrigin[2]         = SrcOrigin[2];
    copyImage->dstOrigin[0]         = DstOrigin[0];
    copyImage->dstOrigin[1]         = DstOrigin[1];
    copyImage->dstOrigin[2]         = DstOrigin[2];
    copyImage->region[0]            = Region[0];
    copyImage->region[1]            = Region[1];
    copyImage->region[2]            = Region[2];

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueCopyImage)(CommandQueue, SrcImage, DstImage, SrcOrigin, DstOrigin, Region, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010112: (clEnqueueCopyImage) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              SrcImage,
    cl_mem              DstBuffer,
    const size_t *      SrcOrigin,
    const size_t *      Region,
    size_t              DstOffset,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandCopyImageToBuffer_PTR copyImageToBuffer;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x SrcImage=0x%x DstBuffer=0x%x "
                  "SrcOrigin=0x%x DstOffset=%u Region=0x%x",
                  CommandQueue, SrcImage, DstBuffer, SrcOrigin, DstOffset, Region);
    gcmDUMP_API("${OCL clEnqueueCopyImageToBuffer 0x%x, 0x%x, 0x%x}", CommandQueue, SrcImage, DstBuffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010113: (clEnqueueCopyImageToBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (SrcImage == gcvNULL ||
        SrcImage->objectType != clvOBJECT_MEM ||
        (SrcImage->type != CL_MEM_OBJECT_IMAGE2D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE3D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER) ||
         (SrcImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER && SrcImage->u.image.imageDesc.buffer == DstBuffer))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010274: (clEnqueueCopyImageToBuffer) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (DstBuffer == gcvNULL ||
        DstBuffer->objectType != clvOBJECT_MEM ||
        DstBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010115: (clEnqueueCopyImageToBuffer) invalid DstBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != SrcImage->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010116: (clEnqueueCopyImageToBuffer) CommandQueue's context is not the same as SrcImage's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (CommandQueue->context != DstBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010117: (clEnqueueCopyImageToBuffer) CommandQueue's context is not the same as DstBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010118: (clEnqueueCopyImageToBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010119: (clEnqueueCopyImageToBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }


    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010120: (clEnqueueCopyImageToBuffer) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcImage->type == CL_MEM_OBJECT_IMAGE2D &&
        (SrcOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010121: (clEnqueueCopyImageToBuffer) SrcImage is 2D, but SrcOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((SrcImage->type == CL_MEM_OBJECT_IMAGE1D || SrcImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER) &&
        (SrcOrigin[1] != 0 || Region[1] != 1 || SrcOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010275: (clEnqueueCopyImageToBuffer) SrcImage is 1D or 1D buffer, but SrcOrigin[1] or SrcOrigin[2] is not 0, or Region [1} or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcImage->type == CL_MEM_OBJECT_IMAGE1D_ARRAY &&
        (SrcOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010276: (clEnqueueCopyImageToBuffer) SrcImage is 1D array, but SrcOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of SrcImage */
    switch (SrcImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010277: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010278: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010279: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010280: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.imageDesc.image_width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.imageDesc.image_height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010281: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010282: (clEnqueueCopyImageToBuffer) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    /* Check if region is outside of DstBuffer */
    if (DstBuffer->u.buffer.size <
        DstOffset + (Region[0] * Region[1] * Region[2] * SrcImage->u.image.elementSize))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010123: (clEnqueueCopyImageToBuffer) lastbyte is outside of DstBuffer's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(SrcImage);
    clfRetainMemObject(DstBuffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_COPY_IMAGE_TO_BUFFER;
    command->handler                = &clfExecuteCommandCopyImageToBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    copyImageToBuffer               = &command->u.copyImageToBuffer;
    copyImageToBuffer->srcImage     = SrcImage;
    copyImageToBuffer->dstBuffer    = DstBuffer;
    copyImageToBuffer->srcOrigin[0] = SrcOrigin[0];
    copyImageToBuffer->srcOrigin[1] = SrcOrigin[1];
    copyImageToBuffer->srcOrigin[2] = SrcOrigin[2];
    copyImageToBuffer->region[0]    = Region[0];
    copyImageToBuffer->region[1]    = Region[1];
    copyImageToBuffer->region[2]    = Region[2];
    copyImageToBuffer->dstOffset    = DstOffset;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueCopyImageToBuffer)(CommandQueue, SrcImage, DstBuffer, SrcOrigin, Region, DstOffset, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010124: (clEnqueueCopyImageToBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(
    cl_command_queue    CommandQueue,
    cl_mem              SrcBuffer,
    cl_mem              DstImage,
    size_t              SrcOffset,
    const size_t *      DstOrigin,
    const size_t *      Region,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandCopyBufferToImage_PTR copyBufferToImage;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x SrcBuffer=0x%x DstImage=0x%x "
                  "DstOrigin=0x%x SrcOffset=%u Region=0x%x",
                  CommandQueue, SrcBuffer, DstImage, DstOrigin, SrcOffset, Region);
    gcmDUMP_API("${OCL clEnqueueCopyBufferToImage 0x%x, 0x%x, 0x%x}", CommandQueue, SrcBuffer, DstImage);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010125: (clEnqueueCopyBufferToImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (SrcBuffer == gcvNULL ||
        SrcBuffer->objectType != clvOBJECT_MEM ||
        SrcBuffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010126: (clEnqueueCopyBufferToImage) invalid SrcBuffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }


    if (DstImage == gcvNULL ||
        DstImage->objectType != clvOBJECT_MEM ||
        (DstImage->type != CL_MEM_OBJECT_IMAGE2D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE3D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER) ||
         (DstImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER && DstImage->u.image.imageDesc.buffer == SrcBuffer))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010283: (clEnqueueCopyBufferToImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != SrcBuffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010128: (clEnqueueCopyBufferToImage) CommandQueue's context is not the same as SrcBuffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (CommandQueue->context != DstImage->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010129: (clEnqueueCopyBufferToImage) CommandQueue's context is not the same as DstImage's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010130: (clEnqueueCopyBufferToImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010131: (clEnqueueCopyBufferToImage) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010132: (clEnqueueCopyBufferToImage) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstImage->type == CL_MEM_OBJECT_IMAGE2D &&
        (DstOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010133: (clEnqueueCopyBufferToImage) DstImage is 2D, but DstOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((DstImage->type == CL_MEM_OBJECT_IMAGE1D || DstImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER) &&
        (DstOrigin[1] != 0 || Region[1] != 1 || DstOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010284: (clEnqueueCopyBufferToImage) DstImage is 1D or 1D buffer, but DstOrigin[1] or DstOrigin[2] is not 0, or Region [1} or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstImage->type == CL_MEM_OBJECT_IMAGE1D_ARRAY &&
        (DstOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010285: (clEnqueueCopyBufferToImage) DstImage is 1D array, but DstOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of DstImage */
    switch (DstImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010286: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010287: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010288: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010289: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.imageDesc.image_width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.imageDesc.image_height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010290: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010291: (clEnqueueCopyBufferToImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    /* Check if region is outside of SrcBuffer */
    if (SrcBuffer->u.buffer.size <
        SrcOffset + (Region[0] * Region[1] * Region[2] * DstImage->u.image.elementSize))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010135: (clEnqueueCopyBufferToImage) last byte of source is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(SrcBuffer);
    clfRetainMemObject(DstImage);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_COPY_BUFFER_TO_IMAGE;
    command->handler                = &clfExecuteCommandCopyBufferToImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    copyBufferToImage               = &command->u.copyBufferToImage;
    copyBufferToImage->srcBuffer    = SrcBuffer;
    copyBufferToImage->dstImage     = DstImage;
    copyBufferToImage->srcOffset    = SrcOffset;
    copyBufferToImage->dstOrigin[0] = DstOrigin[0];
    copyBufferToImage->dstOrigin[1] = DstOrigin[1];
    copyBufferToImage->dstOrigin[2] = DstOrigin[2];
    copyBufferToImage->region[0]    = Region[0];
    copyBufferToImage->region[1]    = Region[1];
    copyBufferToImage->region[2]    = Region[2];

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueCopyBufferToImage)(CommandQueue, SrcBuffer, DstImage, SrcOffset, DstOrigin, Region, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010136: (clEnqueueCopyBufferToImage) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapBuffer(
    cl_command_queue    CommandQueue,
    cl_mem              Buffer,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    size_t              Offset,
    size_t              Cb,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandMapBuffer_PTR mapBuffer;
    gctINT              status;
    gctPOINTER          pointer = gcvNULL;
    gctPOINTER          mappedPtr;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x BlockingMap=%u "
                  "MapFlags=0x%x Offset=%u Cb=%u",
                  CommandQueue, Buffer, BlockingMap, MapFlags, Offset, Cb);
    gcmDUMP_API("${OCL clEnqueueMapBuffer 0x%x, 0x%x}", CommandQueue, Buffer);
    VCL_TRACE_API(EnqueueMapBuffer_Pre)(CommandQueue, Buffer, BlockingMap, MapFlags, Offset, Cb, NumEventsInWaitList, EventWaitList, Event, ErrCodeRet);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010137: (clEnqueueMapBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010138: (clEnqueueMapBuffer) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (    (   (Buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
             && (MapFlags & CL_MAP_READ))
         || (   (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
             && (MapFlags & CL_MAP_WRITE))
         || (MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
       )
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010292: (clEnqueueMapBuffer) Map flags and Host flags are not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010139: (clEnqueueMapBuffer) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010140: (clEnqueueMapBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010141: (clEnqueueMapBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }

            if (BlockingMap && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010142: (clEnqueueMapBuffer) BlockingMap is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    if (Buffer->u.buffer.size < Offset+Cb)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010143: (clEnqueueMapBuffer) (Offset + Cb) is larger than Buffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if MapFlags is valid */
    if (MapFlags & ~((cl_map_flags)(CL_MAP_READ|CL_MAP_WRITE)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010144: (clEnqueueMapBuffer) invalid MapFlags (0x%llx).\n",
            MapFlags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_MAP_BUFFER;
    command->handler                = &clfExecuteCommandMapBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    mapBuffer                       = &command->u.mapBuffer;
    mapBuffer->buffer               = Buffer;
    mapBuffer->blockingMap          = BlockingMap;
    mapBuffer->mapFlags             = MapFlags;
    mapBuffer->offset               = Offset;
    mapBuffer->cb                   = Cb;

    mappedPtr = gcmINT2PTR((gcmPTR2SIZE(Buffer->u.buffer.logical) + Offset));
    mapBuffer->mappedPtr = mappedPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingMap),
               CL_OUT_OF_HOST_MEMORY);

    if (ErrCodeRet)
    {
        *ErrCodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(EnqueueMapBuffer_Post)(CommandQueue, Buffer, BlockingMap, MapFlags, Offset, Cb, NumEventsInWaitList, EventWaitList, Event, ErrCodeRet, mappedPtr);
    gcmFOOTER_ARG("%d Command=0x%x mappedPtr=0x%x",
                  CL_SUCCESS, command, mappedPtr);

    if ((Buffer->flags & CL_MEM_USE_HOST_PTR) &&
        (Buffer->host != gcvNULL))
    {
        mappedPtr = gcmINT2PTR((gcmPTR2SIZE(Buffer->host) + Offset));
    }

    return mappedPtr;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010145: (clEnqueueMapBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    if (ErrCodeRet) {
        *ErrCodeRet = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(
    cl_command_queue    CommandQueue,
    cl_mem              Image,
    cl_bool             BlockingMap,
    cl_map_flags        MapFlags,
    const size_t *      Origin,
    const size_t *      Region,
    size_t *            ImageRowPitch,
    size_t *            ImageSlicePitch,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event,
    cl_int *            ErrCodeRet
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandMapImage_PTR mapImage;
    gctINT              status;
    gctPOINTER          mappedPtr;
    size_t              stride;
    gctUINT             elementSize;
    gctPOINTER          pointer = gcvNULL;
    size_t              slicePitch;
    size_t              hostOffset = 0;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingMap=%u "
                  "MapFlags=0x%x Origin=%u Region=0x%x",
                  CommandQueue, Image, BlockingMap, MapFlags, Origin, Region);
    gcmDUMP_API("${OCL clEnqueueMapImage 0x%x, 0x%x}", CommandQueue, Image);
    VCL_TRACE_API(EnqueueMapImage_Pre)(CommandQueue, Image, BlockingMap, MapFlags, Origin, Region, ImageRowPitch, ImageSlicePitch, NumEventsInWaitList, EventWaitList, Event, ErrCodeRet);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010146: (clEnqueueMapImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         Image->type != CL_MEM_OBJECT_IMAGE1D_BUFFER))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010293: (clEnqueueMapImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (    (   (Image->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
             && (MapFlags & CL_MAP_READ))
         || (   (Image->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
             && (MapFlags & CL_MAP_WRITE))
         || (MapFlags & CL_MAP_WRITE_INVALIDATE_REGION)
       )
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010294: (clEnqueueMapImage) Map flags and Host flags are not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    if (CommandQueue->context != Image->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010148: (clEnqueueMapImage) CommandQueue's context is not the same as Image's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010149: (clEnqueueMapImage) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010150: (clEnqueueMapImage) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }

            if (BlockingMap && clfGetEventExecutionStatus(EventWaitList[i]) < 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010151: (clEnqueueMapImage) BlockingMap is set, but EventWaitList[%d]'s executionStatus is negative.\n",
                    i);
                clmRETURN_ERROR(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
            }
        }
    }

    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010295: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[1] != 0 || Origin[2] != 0 || Region[1] != 1 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010296: (clEnqueueMapImage) Image is 1D, but Origin[1] or Origin[2] is not 0 or Region[1] is not 1 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010297: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010298: (clEnqueueMapImage) Image is 1D array, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010299: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Origin[2] != 0 || Region[2] != 1)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010300: (clEnqueueMapImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_array_size)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010301: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010152: (clEnqueueMapImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.imageDesc.image_width ||
            Origin[1] + Region[1] > Image->u.image.imageDesc.image_height ||
            Origin[2] + Region[2] > Image->u.image.imageDesc.image_depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010302: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010152: (clEnqueueMapImage) One of Region's dimension size is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010303: (clEnqueueMapImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }

    /* Check if MapFlags is valid */
    if (MapFlags & ~((cl_map_flags)(CL_MAP_READ|CL_MAP_WRITE)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010155: (clEnqueueMapImage) invalid MapFlags (0x%llx).\n",
            MapFlags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (ImageRowPitch == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010156: (clEnqueueMapImage) ImageRowPitch is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Image->type == CL_MEM_OBJECT_IMAGE3D &&
                   ImageSlicePitch == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010157: (clEnqueueMapImage) Image is 3D, but ImageSlicePitch is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_MAP_IMAGE;
    command->handler                = &clfExecuteCommandMapImage;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    mapImage                        = &command->u.mapImage;
    mapImage->image                 = Image;
    mapImage->blockingMap           = BlockingMap;
    mapImage->mapFlags              = MapFlags;
    mapImage->origin[0]             = Origin[0];
    mapImage->origin[1]             = Origin[1];
    mapImage->origin[2]             = Origin[2];
    mapImage->region[0]             = Region[0];
    mapImage->region[1]             = Region[1];
    mapImage->region[2]             = Region[2];

    stride                          = Image->u.image.textureStride;
    elementSize                     = Image->u.image.elementSize;
    slicePitch                      = Image->u.image.textureSlicePitch;
    mappedPtr                       = (gctUINT8_PTR) Image->u.image.textureLogical + Origin[2]*slicePitch +
                                                     Origin[1] * stride +
                                                     Origin[0] * elementSize;

    if ((Image->flags & CL_MEM_USE_HOST_PTR) &&
        (Image->host != gcvNULL))
    {
        hostOffset                      = Origin[2] * Image->u.image.imageDesc.image_slice_pitch +
                                          Origin[1] * Image->u.image.imageDesc.image_row_pitch +
                                          Origin[0] * elementSize;
        mappedPtr = gcmINT2PTR(gcmPTR2SIZE(Image->host) + hostOffset);
        stride = Image->u.image.imageDesc.image_row_pitch;
        slicePitch = Image->u.image.imageDesc.image_slice_pitch;
    }

    mapImage->imageRowPitch         = stride;
    mapImage->imageSlicePitch       = slicePitch;
    if (ImageRowPitch)
    {
        *ImageRowPitch = stride;
    }
    if (ImageSlicePitch)
    {
        *ImageSlicePitch = slicePitch;
    }

    mapImage->mappedPtr             = mappedPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingMap),
               CL_OUT_OF_HOST_MEMORY);

    if (ErrCodeRet) {
        *ErrCodeRet = CL_SUCCESS;
    }

    VCL_TRACE_API(EnqueueMapImage_Post)(CommandQueue, Image, BlockingMap, MapFlags, Origin, Region, ImageRowPitch, ImageSlicePitch, NumEventsInWaitList, EventWaitList, Event, ErrCodeRet, mappedPtr);
    gcmFOOTER_ARG("%d Command=0x%x mappedPtr=0x%x",
                  CL_SUCCESS, command,mappedPtr);
    return mappedPtr;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010158: (clEnqueueMapImage) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    if (ErrCodeRet) {
        *ErrCodeRet = status;
    }
    gcmFOOTER_ARG("%d", status);
    return gcvNULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(
    cl_command_queue    CommandQueue,
    cl_mem              MemObj,
    void *              MappedPtr,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandUnmapMemObject_PTR  unmapMemObject;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x MemObj=0x%x MappedPtr=0x%x",
                  CommandQueue, MemObj, MappedPtr);
    gcmDUMP_API("${OCL clEnqueueUnmapMemObject 0x%x, 0x%x}", CommandQueue, MemObj);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010159: (clEnqueueUnmapMemObject) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (MemObj == gcvNULL ||
        MemObj->objectType != clvOBJECT_MEM)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010160: (clEnqueueUnmapMemObject) invalid MemObj.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != MemObj->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010161: (clEnqueueUnmapMemObject) CommandQueue's context is not the same as Image's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010162: (clEnqueueUnmapMemObject) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010163: (clEnqueueUnmapMemObject) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_UNMAP_MEM_OBJECT;
    command->handler                = &clfExecuteCommandUnmapMemObject;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    unmapMemObject                  = &command->u.unmapMemObject;
    unmapMemObject->memObj          = MemObj;
    unmapMemObject->mappedPtr       = MappedPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueUnmapMemObject)(CommandQueue, MemObj, MappedPtr, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010164: (clEnqueueUnmapMemObject) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(
    cl_command_queue        CommandQueue ,
    cl_uint                 NumMemObjects ,
    const cl_mem *          MemObjects ,
    cl_mem_migration_flags  Flags ,
    cl_uint                 NumEventsInWaitList ,
    const cl_event *        EventWaitList ,
    cl_event *              Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;
    gctUINT             i;

    gcmHEADER_ARG("CommandQueue=0x%x MemObjects=0x%x Flags=%u",
                  CommandQueue, MemObjects, Flags);
    gcmDUMP_API("${OCL clEnqueueMigrateMemObjects 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if ((NumMemObjects == 0) || (MemObjects == gcvNULL))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (MemObjects)
    {
        for (i = 0; i < NumMemObjects; i++)
        {
            if (MemObjects[i] == gcvNULL ||
                MemObjects[i]->objectType != clvOBJECT_MEM)
            {
                clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
            }

            if (CommandQueue->context != MemObjects[i]->context)
            {
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (Flags & ~((cl_mem_migration_flags)(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_MIGRATE_MEM_OBJECTS;
    command->handler                = &clfExecuteCommandMigrateMemObjects;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE), CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueMigrateMemObjects)(CommandQueue, NumMemObjects, MemObjects, Flags, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010304: (clEnqueueMigrateMemObjects) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}


#if _SUPPORT_LONG_ULONG_DATA_TYPE
/* To check whether an instruction has 64-bit operands. */
static
gctBOOL gcIs64Inst(
    gcSL_INSTRUCTION    Inst
)
{
    gctBOOL result = gcvFALSE;
    gctUINT src0Format, src1Format, dstFormat;
    gctUINT opcode;
    src0Format = gcmSL_SOURCE_GET(GetInstSource0(Inst), Format);
    src1Format = gcmSL_SOURCE_GET(GetInstSource1(Inst), Format);
    dstFormat  = gcmSL_TARGET_GET(GetInstTemp(Inst), Format);

    result = ((src0Format == gcSL_INT64) || (src0Format == gcSL_UINT64) ||
            (src1Format == gcSL_INT64) || (src1Format == gcSL_UINT64));

    opcode = gcmSL_OPCODE_GET(Inst->opcode, Opcode);
    if ((opcode == gcSL_F2I) ||
        (opcode == gcSL_CONV))
    {
        result = result || ((dstFormat == gcSL_INT64 || dstFormat == gcSL_UINT64) && (src0Format == gcSL_FLOAT));
    }

    return result;
}

/* To check whether the instruction needs to be recompiled. */
static
gctBOOL gcNeedRecomile64(
    gcSL_INSTRUCTION    Instruction
)
{
    gctBOOL recompile = gcvFALSE;
    gctUINT dstFormat = gcmSL_TARGET_GET(GetInstTemp(Instruction), Format);
    gctUINT opcode = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);
    gcSL_CONDITION condition = gcmSL_TARGET_GET(GetInstTemp(Instruction), Condition);

    recompile = NEED_PATCH_LONGULONG(Instruction, opcode, dstFormat) ||
               (opcode == gcSL_JMP &&
               (condition == gcSL_LESS_OR_EQUAL ||
                condition == gcSL_GREATER_OR_EQUAL ||
                condition == gcSL_LESS ||
                condition == gcSL_GREATER ||
                condition == gcSL_NOT_EQUAL ||
                condition == gcSL_EQUAL));

    return recompile;
}

#endif

static gceSTATUS
clfAddGlobalWorkSizeRecompile(
    cl_command_queue     CommandQueue,
    cl_kernel            Kernel,
    cl_uint *            WorkDim,
    const size_t **      GlobalWorkOffset,
    const size_t **      GlobalWorkSize,
    const size_t **      LocalWorkSize,
    size_t *             fakeGlobalWorkOffset,
    size_t *             fakeGlobalWorkSize,
    size_t *             fakeLocalWorkSize,
    clsPatchDirective_PTR * PatchDirective
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    /* convert 1D -> 2D */
    if((*WorkDim == 1) &&
       ((*GlobalWorkSize)[0] > CommandQueue->device->deviceInfo.maxGlobalWorkSize))
    {
        size_t  size0, size1;
        gctSIZE_T attribCount;
        gcSHADER kernelBinary = (gcSHADER)Kernel->masterInstance.binary;
        gctBOOL needShaderPatch = gcvFALSE;
        gctBOOL  matched = gcvFALSE, patchRealGlobalWorkSize = gcvFALSE;
        gctUINT  realGlobalWorkSize = (*GlobalWorkSize[0]);
        *WorkDim = 2;

        if ((*LocalWorkSize) && ((*LocalWorkSize[0]) != 0))
        {
            size1 = (*LocalWorkSize[0]);
        }
        else
        {
            size1 = 16;
        }

        do
        {
            size0 = (*GlobalWorkSize[0]) / size1;

            if ((*(GlobalWorkSize[0]) % size1) == 0)
            {
                if (size0 <= CommandQueue->device->deviceInfo.maxGlobalWorkSize)
                {
                    matched = gcvTRUE;
                    break;
                }
            }
            else
            {
                if (size0 < CommandQueue->device->deviceInfo.maxGlobalWorkSize)
                {
                    /* Need one more line if total work size is not multiple of size1 */
                    size0 += 1;
                    matched = gcvTRUE;
                    patchRealGlobalWorkSize = gcvTRUE;
                    needShaderPatch = gcvTRUE;
                    break;
                }
            }

            size1 *= 2;

        } while(size1 < size0);

        if (!matched)
        {
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }

        fakeGlobalWorkSize[0] = size1;
        fakeGlobalWorkSize[1] = size0;

        (*GlobalWorkSize) = fakeGlobalWorkSize;

        if ((*GlobalWorkOffset) != gcvNULL)
        {
            fakeGlobalWorkOffset[0] =  (*GlobalWorkOffset[0]) % fakeGlobalWorkSize[0];
            fakeGlobalWorkOffset[1] =  (*GlobalWorkOffset[0]) / fakeGlobalWorkSize[0];

            if (fakeGlobalWorkOffset[1] > fakeGlobalWorkSize[1])
            {
                clmRETURN_ERROR(CL_INVALID_GLOBAL_OFFSET);
            }

            *GlobalWorkOffset = fakeGlobalWorkOffset;
        }

        if ((*LocalWorkSize) != gcvNULL)
        {
            fakeLocalWorkSize[0] = (*LocalWorkSize[0]);
            fakeLocalWorkSize[1] = 1;
            *LocalWorkSize = fakeLocalWorkSize;
        }

        for (attribCount = 0; attribCount < GetShaderAttributeCount(kernelBinary); attribCount++)
        {
            if ((gcSL_GLOBAL_INVOCATION_ID == GetATTRNameLength(GetShaderAttribute(kernelBinary, attribCount)) ||
                 gcSL_WORK_GROUP_ID == GetATTRNameLength(GetShaderAttribute(kernelBinary, attribCount))) &&
                gcmATTRIBUTE_enabled(GetShaderAttribute(kernelBinary, attribCount)))
            {
                needShaderPatch = gcvTRUE;
                break;
            }
        }

        if (needShaderPatch)
        {
            clmONERROR(clfCreateGlobalWorkSizeDirective(realGlobalWorkSize,
                        patchRealGlobalWorkSize, PatchDirective),
                       CL_INVALID_GLOBAL_WORK_SIZE);
        }
        /* for type of globalworksize patch, always do recompile */
        Kernel->isPatched = gcvFALSE;
    }

OnError:
    return status;
}

static gceSTATUS
clfAddLongUlongRecompile(
    gcSHADER KernelBinary,
    clsPatchDirective_PTR * PatchDirective
    )
{
    gctUINT i;
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /* Check for LongULong opeartions for patch. */
    for (i = 0; i < GetShaderCodeCount(KernelBinary); i++)
    {
        gcSL_INSTRUCTION inst   = GetShaderInstruction(KernelBinary, i);

        if (gcIs64Inst(inst))
        {
            if (gcNeedRecomile64(inst))
            {
                clfCreateLongULongDirective(inst,
                                            i,
                                            PatchDirective);
            }
        }
    }
#endif

    return gcvSTATUS_OK;
}

static gceSTATUS
clfAddWriteImageRecompile(
    cl_kernel Kernel,
    gcSHADER KernelBinary,
    clsCommandNDRangeKernel_PTR NDRangeKernel,
    clsPatchDirective_PTR * PatchDirective
    )
{
    gctUINT uniformIndex[128] = {(gctUINT) ~0};
    gceSTATUS  status = gcvSTATUS_OK;
    clsArgument_PTR arg;
    gctUINT count = 0;
    gctUINT i;

    for (i = 0; i < GetShaderCodeCount(KernelBinary); i++)
    {
        gcSL_INSTRUCTION code   = GetShaderInstruction(KernelBinary, i);
        gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(GetInstOpcode(code), Opcode);
        if (gcSL_isOpcodeImageWrite(opcode))
        {
            gctUINT samplerId = gcmSL_INDEX_GET(GetInstSource0Index(code), Index);
            gctUINT j;

            for (j = 0; j < count; j++)
            {
                if (uniformIndex[j] == samplerId)
                {
                    break;
                }
            }

            if (j == count)
            {
                uniformIndex[count] = samplerId;
                count++;
                if (count == 32)
                {
                    break;
                }
            }
        }
    }

    for (i = 0; i < count; i++)
    {
        gcSHADER_TYPE       type;
        if (uniformIndex[i] == (gctUINT) ~0) continue;

        arg = &NDRangeKernel->args[uniformIndex[i]];
        gcmASSERT(arg->uniform);
        if (arg->uniform == gcvNULL) continue;

        clmONERROR(gcUNIFORM_GetType(arg->uniform, &type, gcvNULL),
                   CL_INVALID_VALUE);
        if (isOCLImageType(type))
        {
            clsMem_PTR              image;
            clsImageHeader_PTR      imageHeader;

            /* Check if recompilation is needed. */
            /* Get image object. */
            image = *(clsMem_PTR *) arg->data;
            gcmASSERT(image->objectType == clvOBJECT_MEM);
            gcmASSERT(image->type != CL_MEM_OBJECT_BUFFER);

            /* Get image header. */
            imageHeader = (clsImageHeader_PTR) image->u.image.headerLogical;

            /* Create patch directive. */
            clmONERROR(clfCreateWriteImageDirective(imageHeader,
                                                    (gctUINT)GetUniformIndex(arg->uniform),
                                                    imageHeader->channelDataType,
                                                    imageHeader->channelOrder,
                                                    imageHeader->tiling,
                                                    PatchDirective),
                       CL_OUT_OF_HOST_MEMORY);

            if((Kernel->recompileInstance) && (Kernel->isPatched == gcvTRUE))
            {
                clsPatchDirective_PTR tmpPatchDirective = gcvNULL;
                gctBOOL needPatch= gcvTRUE;

                for(tmpPatchDirective = Kernel->recompileInstance->patchDirective;
                    tmpPatchDirective != gcvNULL;
                    tmpPatchDirective = tmpPatchDirective->next)
                {
                    if(tmpPatchDirective->kind == gceRK_PATCH_WRITE_IMAGE)
                    {
                        if((tmpPatchDirective->patchValue.writeImage->channelDataType == (*PatchDirective)->patchValue.writeImage->channelDataType)
                            && (tmpPatchDirective->patchValue.writeImage->channelOrder == (*PatchDirective)->patchValue.writeImage->channelOrder))
                        {
                            needPatch = gcvFALSE;
                            break;
                        }
                    }
                }
                if(needPatch)
                {
                    Kernel->isPatched = gcvFALSE;
                }
            }
        }
    }

OnError:
    return status;
}

static gceSTATUS
clfAddReadImageRecompile(
    cl_command_queue CommandQueue,
    cl_kernel Kernel,
    gcSHADER  KernelBinary,
    gcKERNEL_FUNCTION KernelFunction,
    clsCommandNDRangeKernel_PTR NDRangeKernel,
    clsPatchDirective_PTR * PatchDirective
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    if (GetKFunctionISamplerCount(KernelFunction) > 0)
    {
        gctUINT uniformIndex[128];
        gctUINT count = 0;
        gctUINT i;
        gctUINT vsSamplers = 0, psSamplers = 0;
        gctINT maxSampler = 0, sampler = 0;
        gcSHADER_KIND shadeType = gcSHADER_TYPE_UNKNOWN;
        clsArgument_PTR arg;

        for (i = 0; i < GetKFunctionISamplerCount(KernelFunction); i++)
        {
            uniformIndex[i] = (gctUINT) ~0;
        }

        for (i = 0; i < GetShaderCodeCount(KernelBinary); i++)
        {
            gcSL_INSTRUCTION code   = GetShaderInstruction(KernelBinary, i);
            gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(GetInstOpcode(code), Opcode);
            if (opcode == gcSL_TEXLD)
            {
                gctUINT samplerId = gcmSL_INDEX_GET(GetInstSource0Index(code), Index);
                gctUINT j;

                for (j = 0; j < count; j++)
                {
                    if (uniformIndex[j] == samplerId)
                    {
                        break;
                    }
                }

                if (j == count)
                {
                    uniformIndex[count] = samplerId;
                    count++;
                    if (count == GetKFunctionISamplerCount(KernelFunction))
                    {
                        break;
                    }
                }
            }
        }

        if (CommandQueue->device->deviceInfo.computeOnlyGpu != gcvTRUE)  /*use texld*/
        {
            vsSamplers = CommandQueue->context->platform->vscCoreSysCtx.hwCfg.maxVSSamplerCount;
            psSamplers = CommandQueue->context->platform->vscCoreSysCtx.hwCfg.maxPSSamplerCount;

            /* Determine starting sampler index. */
            shadeType = CommandQueue->device->deviceInfo.psThreadWalker
                ? GetShaderType(KernelBinary) : gcSHADER_TYPE_VERTEX;

            if (CommandQueue->device->deviceInfo.supportSamplerBaseOffset)
            {
                sampler = 0;
            }
            else
            {
                sampler = (shadeType == gcSHADER_TYPE_VERTEX)
                        ? psSamplers
                        : 0;
            }

            /* Determine maximum sampler index. */
            /* Note that CL kernel can use all samplers if unified. */
            maxSampler = (shadeType == gcSHADER_TYPE_FRAGMENT)
                       ? psSamplers
                       : psSamplers + vsSamplers;
        }

        for (i = 0; i < GetKFunctionISamplerCount(KernelFunction); i++)
        {
            gcSHADER_TYPE       type;

            if (uniformIndex[i] == (gctUINT) ~0) continue;

            arg = &NDRangeKernel->args[uniformIndex[i]];
            gcmASSERT(arg->uniform);
            if (arg->uniform == gcvNULL) continue;

            clmONERROR(gcUNIFORM_GetType(arg->uniform, &type, gcvNULL),
                       CL_INVALID_VALUE);

            if (type == gcSHADER_SAMPLER_2D ||
                type == gcSHADER_SAMPLER_3D)
            {
                gcsIMAGE_SAMPLER_PTR    imageSampler;
                clsMem_PTR              image;
                clsImageHeader_PTR      imageHeader;
                gctUINT                 samplerValue;
                cleSAMPLER              normalizedMode = CLK_NORMALIZED_COORDS_FALSE;
                cleSAMPLER              filterMode = CLK_FILTER_NEAREST;
                gctUINT                 channelDataType;
                gctUINT                 channelOrder;
                gctBOOL                 isConstantSamplerType;

                /* Check if recompilation is needed. */

                /* Find the image sampler. */
                gcmASSERT(GetKFunctionISamplers(KernelFunction) &&
                          GetKFunctionISamplerCount(KernelFunction) > GetUniformImageSamplerIndex(arg->uniform));
                imageSampler = GetKFunctionISamplers(KernelFunction) + GetUniformImageSamplerIndex(arg->uniform);

                /* Get image object. */
                image = *(clsMem_PTR *) NDRangeKernel->args[GetImageSamplerImageNum(imageSampler)].data;
                gcmASSERT(image->objectType == clvOBJECT_MEM);
                gcmASSERT(image->type != CL_MEM_OBJECT_BUFFER);

                /* Get image header. */
                imageHeader = (clsImageHeader_PTR) image->u.image.headerLogical;
                channelDataType = imageHeader->channelDataType;
                channelOrder = imageHeader->channelOrder;

                /* Get sampler value. */
                isConstantSamplerType = GetImageSamplerIsConstantSamplerType(imageSampler);
                if (isConstantSamplerType)
                {
                    samplerValue = GetImageSamplerType(imageSampler);
                }
                else
                {
                    samplerValue = *((gctUINT *) NDRangeKernel->args[GetImageSamplerType(imageSampler)].data);
                }

                if (CommandQueue->device->deviceInfo.computeOnlyGpu != gcvTRUE)  /*use texld*/
                {
                    normalizedMode = samplerValue & CLK_NORMALIZED_COORDS_TRUE;
                    filterMode  = samplerValue & 0xF00;

                    {
                        /* Check channel data type. */
                        /* TODO - gc4000 can handle more data types. */
                        if ((sampler < maxSampler) &&
                            (channelDataType == CL_UNORM_INT8))
                        {
                            arg->needImageSampler = gcvTRUE;
                            arg->image            = image;
                            arg->samplerValue     = samplerValue;

                            if ((image->type != CL_MEM_OBJECT_IMAGE3D) &&
                                (image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY) &&
                                (image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
                                )
                            {
                                /* Check filter mode and normalized mode. */
                                if ((filterMode == CLK_FILTER_NEAREST) &&
                                    (normalizedMode == CLK_NORMALIZED_COORDS_TRUE) &&
                                    ((shadeType != gcSHADER_TYPE_VERTEX) || (samplerValue & 0xF) != CLK_ADDRESS_CLAMP))
                                {
                                    ++sampler;
                                    continue;
                                }
                            }
                        }
                    }
                }

                arg->needImageSampler = gcvFALSE;

                /* Create patch directive. */
                clmONERROR(clfCreateReadImageDirective(imageHeader,
                                                       (gctUINT)GetUniformIndex(arg->uniform),
                                                       samplerValue,
                                                       channelDataType,
                                                       channelOrder,
                                                       imageHeader->tiling,
                                                       PatchDirective),
                           CL_OUT_OF_HOST_MEMORY);

                if((Kernel->recompileInstance) && (Kernel->isPatched == gcvTRUE))
                {
                    clsPatchDirective_PTR tmpPatchDirective = gcvNULL;
                    gctBOOL needPatch= gcvTRUE;
                    for(tmpPatchDirective = Kernel->recompileInstance->patchDirective;
                        tmpPatchDirective != gcvNULL;
                        tmpPatchDirective = tmpPatchDirective->next)
                    {
                        if(tmpPatchDirective->kind == gceRK_PATCH_READ_IMAGE)
                        {
                            if((tmpPatchDirective->patchValue.readImage->channelDataType ==
                                (*PatchDirective)->patchValue.readImage->channelDataType) &&
                                (tmpPatchDirective->patchValue.readImage->channelOrder ==
                                (*PatchDirective)->patchValue.readImage->channelOrder)
                              )
                            {
                                needPatch = gcvFALSE;
                                break;
                            }
                        }
                    }
                    if(needPatch)
                    {
                        Kernel->isPatched = gcvFALSE;
                    }
                }
            }
        }
    }

OnError:
    return status;
}

static gceSTATUS
clfAddKernelRecompile(
    cl_command_queue CommandQueue,
    cl_kernel Kernel,
    clsCommandNDRangeKernel_PTR NDRangeKernel,
    clsPatchDirective_PTR * PatchDirective
    )
{
    gctUINT i;
    gceSTATUS status = gcvSTATUS_OK;

    if (Kernel->patchNeeded)
    {
        gcSHADER            kernelBinary = (gcSHADER) Kernel->masterInstance.binary;
        gcKERNEL_FUNCTION   kernelFunction = gcvNULL;

        /* Get kernel function. */
        gcmASSERT(GetShaderKernelFunctions(kernelBinary));

        for (i = 0; i < GetShaderKernelFunctionCount(kernelBinary); i++)
        {
            kernelFunction = GetShaderKernelFunction(kernelBinary, i);
            if (kernelFunction && GetKFunctionIsMain(kernelFunction))
            {
                break;
            }
        }

        gcmASSERT(kernelFunction);

        clfAddLongUlongRecompile(kernelBinary, PatchDirective);

        /* Patch for write_image funtions. */
        clmONERROR(clfAddWriteImageRecompile(Kernel,
                                               kernelBinary,
                                               NDRangeKernel,
                                               PatchDirective) ,
                   CL_INVALID_VALUE);

        /* Patch for read_image funtions. */
        clmONERROR(clfAddReadImageRecompile(CommandQueue,
                                              Kernel,
                                              kernelBinary,
                                              kernelFunction,
                                              NDRangeKernel,
                                              PatchDirective) ,
                   CL_INVALID_VALUE);
    }

OnError:
    return status;
}

gctUINT
clfEvaluateCRC32(
    gctPOINTER pData,
    gctUINT dataSizeInByte
    )
{
    gctUINT crc = 0xFFFFFFFF;
    gctINT8 *start = (gctINT8*)pData;
    gctINT8 *end = start + dataSizeInByte;

    static const gctUINT crc32Table[256] =
    {
        0x00000000,0x77073096,0xee0e612c,0x990951ba,
        0x076dc419,0x706af48f,0xe963a535,0x9e6495a3,
        0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988,
        0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,
        0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,
        0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,
        0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,
        0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5,
        0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,
        0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
        0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,
        0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,
        0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116,
        0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,
        0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
        0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,
        0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a,
        0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433,
        0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818,
        0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01,
        0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,
        0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,
        0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c,
        0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,
        0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,
        0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,
        0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,
        0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9,
        0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,
        0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
        0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,
        0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,
        0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a,
        0xead54739,0x9dd277af,0x04db2615,0x73dc1683,
        0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8,
        0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1,
        0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,
        0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7,
        0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,
        0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
        0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,
        0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,
        0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60,
        0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,
        0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,
        0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,
        0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,
        0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d,
        0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a,
        0x9c0906a9,0xeb0e363f,0x72076785,0x05005713,
        0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38,
        0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21,
        0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e,
        0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,
        0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,
        0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,
        0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,
        0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db,
        0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,
        0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
        0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,
        0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,
        0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94,
        0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d
    };
    gcmHEADER_ARG("pData=0x%x dataSizeInByte=%d",pData, dataSizeInByte);

    while (start < end)
    {
        gctUINT data = *start ++;
        data &= 0xFF;
        crc = crc32Table[(crc & 255) ^ data] ^ (crc >> 8);
    }
    gcmFOOTER_ARG("return=%u", ~crc);
    return ~crc;
}

clsVIRInstanceKey_PTR clfFindInstanceByKey(
                                clsVIRInstanceHashRec_PTR pHash,
                                gctUINT key)
{
    gctUINT entryId = key & (pHash->tbEntryNum - 1);
    clsVIRInstanceKey_PTR pObj = pHash->ppHashTable[entryId];
    clsVIRInstanceKey_PTR retObj = gcvNULL;

    gcmHEADER_ARG("pHash=0x%x key=%u", pHash, key);

    while (pObj)
    {
        if (pObj->key == key)
        {
            retObj = pObj;
            break;
        }

        pObj = pObj->nextInstanceKey;
    }

    /* Update year to be recent one */
    if (retObj)
    {
        retObj->year = pHash->year++;
    }

    gcmFOOTER_ARG("return=0x%x", gcvNULL);
    return retObj;
}

/* Loop KEP pair to decide if we need recompile */
static void
clfGetCurrentKeyState(
    clsCommandNDRangeVIRKernel_PTR NDRangeKernel,
    gctUINT                        * kernelKeyData,
    gctUINT                        * kernelKeySize
    )
{
    gctUINT                   i;
    cl_kernel                 Kernel = NDRangeKernel->kernel;
    KERNEL_EXECUTABLE_PROFILE * kep = &Kernel->virMasterInstance->kep;
    VSC_HW_CONFIG             * pHwCfg = &Kernel->context->platform->vscCoreSysCtx.hwCfg;
    gctUINT                   currentKeyState = 0;

    /* Get Pair from texLD table and Image table */
    if(kep->resourceTable.imageTable.countOfEntries > 0)
    {
        PROG_CL_IMAGE_TABLE_ENTRY * imageEntry;
        PROG_CL_ARG_ENTRY         * pArgsEntry;
        clsSrcArgument_PTR        argImage, argSampler;
        gctUINT                   currentSamplerValue=0;
        clsMem_PTR                image;
        gctBOOL                   isImageRead = gcvFALSE;

        for (i = 0; i < kep->resourceTable.imageTable.countOfEntries; i++)
        {
            imageEntry = &kep->resourceTable.imageTable.pImageEntries[i];
            pArgsEntry = &kep->argTable.pArgsEntries[imageEntry->imageArgIndex];

            if(pArgsEntry->typeQualifier & VIR_TYQUAL_READ_ONLY)
            {
                isImageRead = gcvTRUE;
                if (imageEntry->kernelHardcodeSampler)
                {
                    currentSamplerValue = imageEntry->constSamplerValue;
                }
                else
                {
                    clsSampler_PTR sampler;

                    argSampler = &Kernel->srcArgs[imageEntry->samplerArgIndex];
                    sampler = *(clsSampler_PTR *)argSampler->data;
                    currentSamplerValue = (sampler->samplerValue | imageEntry->assumedSamplerValue);
                }
            }
            else if(pArgsEntry->typeQualifier & VIR_TYQUAL_WRITE_ONLY)
            {
                isImageRead = gcvFALSE;
            }

            argImage = &Kernel->srcArgs[imageEntry->imageArgIndex];

            image = *(clsMem_PTR *)argImage->data;

            if(isImageRead)
            {
                NDRangeKernel->recompileType.doImgRecompile = vscImageSamplerNeedLibFuncForHWCfg(&image->u.image.imageDescriptor, currentSamplerValue, pHwCfg, gcvNULL, gcvNULL, &currentKeyState);
            }
            else
            {
                NDRangeKernel->recompileType.doImgRecompile = vscImageWriteNeedLibFuncForHWCfg(&image->u.image.imageDescriptor, pHwCfg, gcvNULL, &currentKeyState);
            }

            kernelKeyData[*kernelKeySize] = currentKeyState;
            *kernelKeySize += 1;
        }
    }

    if((NDRangeKernel->workDim == 1) && (NDRangeKernel->globalWorkSize[0] > clgDevices->deviceInfo.maxGlobalWorkSize))
    {
        size_t  size0, size1;
        gctBOOL matched = gcvFALSE;

        if (NDRangeKernel->localWorkSize[0] != 0)
        {
            size1 = NDRangeKernel->localWorkSize[0];
        }
        else
        {
            size1 = 16;
        }

        do
        {
            size0 = (NDRangeKernel->globalWorkSize[0]) / size1;

            if (((NDRangeKernel->globalWorkSize[0]) % size1) == 0)
            {
                if (size0 <= Kernel->context->devices[0]->deviceInfo.maxGlobalWorkSize)
                {
                    matched = gcvTRUE;
                    break;
                }
            }
            else
            {
                if (size0 < Kernel->context->devices[0]->deviceInfo.maxGlobalWorkSize)
                {
                    /* Need one more line if total work size is not multiple of size1 */
                    size0 += 1;
                    matched = gcvTRUE;
                    kernelKeyData[*kernelKeySize] = 0x1234;
                    *kernelKeySize += 1;
                    break;
                }
            }

            size1 *= 2;

        } while(size1 < size0);

        if (!matched)
        {
            gcmASSERT(0);
        }

        NDRangeKernel->globalWorkSize[0] = size1;
        NDRangeKernel->globalWorkSize[1] = size0;

        if (NDRangeKernel->globalWorkOffset[0] != 0)
        {
            size0 =  (NDRangeKernel->globalWorkOffset[0]) % NDRangeKernel->globalWorkSize[0];
            size1 =  (NDRangeKernel->globalWorkOffset[0]) / NDRangeKernel->globalWorkSize[0];

            if (size1 > NDRangeKernel->globalWorkSize[1])
            {
                gcmASSERT(0);
            }

            NDRangeKernel->globalWorkOffset[0] = size0;
            NDRangeKernel->globalWorkOffset[1] = size1;
        }

        if (NDRangeKernel->localWorkSize[0] != 0)
        {
            NDRangeKernel->localWorkSize[1] = 1;
        }

        kernelKeyData[*kernelKeySize] = 0x5678;
        *kernelKeySize += 1;

        NDRangeKernel->recompileType.doGlobalWorksizeRecompile = gcvTRUE;
    }
    return;
}

static gctBOOL
clfNeedRecompile(
    cl_kernel Kernel,
    clsPatchDirective_PTR  patchDirective
    )
{
    return (patchDirective && (Kernel->isPatched == gcvFALSE));
}

static gceSTATUS
clfRecompileKernel(
    cl_kernel Kernel,
    clsCommandNDRangeKernel_PTR NDRangeKernel,
    clsPatchDirective_PTR  patchDirective
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL halti5 = clgDefaultDevice->platform->vscCoreSysCtx.hwCfg.hwFeatureFlags.hasHalti5;

    /* Patch shader */
    gcmONERROR(clfDynamicPatchKernel(Kernel,
        &NDRangeKernel->numArgs,
        &NDRangeKernel->args,
        patchDirective));

    /* patch already insert to kernel, set to NULL */
    patchDirective = gcvNULL;

    NDRangeKernel->currentInstance = Kernel->recompileInstance;

    if((!halti5) || (!NEW_READ_WRITE_IMAGE)) Kernel->isPatched = gcvTRUE;

    if((Kernel->masterInstance.programState.hints == gcvNULL) && (Kernel->recompileInstance->programState.hints != gcvNULL))
    {
        gcSHADER shader = (gcSHADER)Kernel->recompileInstance->binary;
        gctUINT workGroupSize;

        gcmONERROR(gcSHADER_GetWorkGroupSize(shader, &workGroupSize));
        Kernel->maxWorkGroupSize = (size_t)workGroupSize;

        /* maxWorkGroupSize should not over the device's maxWorkGroupSize. */
        if (Kernel->maxWorkGroupSize > Kernel->program->devices[0]->deviceInfo.maxWorkGroupSize)
        {
            Kernel->maxWorkGroupSize = Kernel->program->devices[0]->deviceInfo.maxWorkGroupSize;
        }
    }

OnError:
    return status;
}

static void
clfGetVIRScaleHint(
    cl_kernel Kernel,
    clsCommandNDRangeVIRKernel_PTR NDRangeKernel
    )
{
    gctUINT i, j;
    KERNEL_EXECUTABLE_PROFILE * kep = &Kernel->virCurrentInstance->kep;

    for (i = 0 ; i < gcvPROPERTY_COUNT; i++)
    {
        if (kep->kernelHints.property[i].type == gcvPROPERTY_KERNEL_SCALE_HINT)
        {
            for (j = 0; j < kep->kernelHints.property[i].size; j++)
            {
                NDRangeKernel->globalScale[j] = (size_t)kep->kernelHints.property[i].value[j];
            }
            break;
        }
    }
}

static void
clfGetScaleHint(
    cl_kernel Kernel,
    clsCommandNDRangeKernel_PTR NDRangeKernel
    )
{
    gctUINT i, j;
    gcKERNEL_FUNCTION kernelFunction;
    gctUINT count;
    gctINT propertyType = 0;
    gctINT propertyValues[3] = {0};

    /* Set the required work group size. */
    gcSHADER_GetKernelFunctionByName((gcSHADER) Kernel->masterInstance.binary, Kernel->name, &kernelFunction);
    gcKERNEL_FUNCTION_GetPropertyCount(kernelFunction, &count);

    for (i = 0; i < count; i++)
    {
        gcKERNEL_FUNCTION_GetProperty(kernelFunction, i, gcvNULL, &propertyType, propertyValues);

        if (propertyType == gcvPROPERTY_KERNEL_SCALE_HINT)
        {
            for (j = 0; j < 3; j++)
            {
                NDRangeKernel->globalScale[j] = (size_t)propertyValues[j];
            }
        }
    }
}

static gctBOOL clfThreadRemapNeedRecompile(cl_kernel Kernel, cl_uint WorkDim, const size_t * LocalWorkSize)
 {
    gctUINT16    *factor;
    gctBOOL      doRecompile = gcvFALSE;

    if (Kernel->context->platform->virShaderPath)
    {
        factor = Kernel->virCurrentInstance ? Kernel->virCurrentInstance->hwStates.hints.workGroupSizeFactor : gcvNULL;
    }
    else
    {
        factor = Kernel->masterInstance.programState.hints ? Kernel->masterInstance.programState.hints->workGroupSizeFactor : gcvNULL;
    }

    if (factor && LocalWorkSize)
    {
        doRecompile = ((LocalWorkSize[0] % 2 != 0) && factor[0] != 0)
                        || ((WorkDim > 1) ? (LocalWorkSize[1] % 2 != 0) && factor[1] != 0 : gcvFALSE)
                        || ((WorkDim > 2) ? (LocalWorkSize[2] % 2 != 0) && factor[2] != 0 : gcvFALSE);
    }

    if (doRecompile == gcvTRUE)
        Kernel->recompileThreadRemap = gcvTRUE;

    return doRecompile;
}

static gctBOOL clfVIRNeedRecompile(clsCommandNDRangeVIRKernel_PTR NDRangeKernel,
                                         gctUINT                  * kernelKeyData,
                                         gctUINT                  * kernelKeySize,
                                         gctUINT                  * currentKey)
{
    gctBOOL   doRecompile = gcvFALSE;

    clfGetCurrentKeyState(NDRangeKernel, kernelKeyData, kernelKeySize);

    /* if the image test supportted by HW natively, use the mater instance directly, not sure if it exist any hole here if any change in compiler or HW!!! */
    if(NDRangeKernel->recompileType.doImgRecompile)
    {
        /* calculate the hash key of the kernel */
        *currentKey = clfEvaluateCRC32(kernelKeyData, (*kernelKeySize)*4);

        if(*currentKey != NDRangeKernel->kernel->virMasterInstance->hashKey)
        {
            doRecompile = gcvTRUE;
        }
    }

    if(NDRangeKernel->recompileType.doGlobalWorksizeRecompile)
    {
        doRecompile = gcvTRUE;
    }

    return doRecompile;
}

cl_int
clfEnqueueNDRangeVIRKernel(
    cl_command_queue    CommandQueue,
    cl_kernel           Kernel,
    cl_uint             WorkDim,
    const size_t *      GlobalWorkOffset,
    const size_t *      GlobalWorkSize,
    const size_t *      LocalWorkSize,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommandNDRangeVIRKernel_PTR NDRangeKernel;
    clsCommand_PTR              command = gcvNULL;
    clsSrcArgument_PTR          arg;
    gctUINT                     i;
    gctINT                      status;
    size_t                      workGroupSize = 1;
    gctPOINTER                  pointer = gcvNULL;
    gctUINT                     keyStateData[MAX_KEY_DATA_SIZE] = {0};
    gctUINT                     keyStateSize = 0;
    gctUINT                     currentKey = 0;
    gctBOOL                     aquired = gcvFALSE;

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010170: (clEnqueueNDRangeKernel) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (WorkDim < 1 || WorkDim > 3)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010171: (clEnqueueNDRangeKernel) invalid WorkDim (%d).\n",
            WorkDim);
        clmRETURN_ERROR(CL_INVALID_WORK_DIMENSION);
    }

    if (GlobalWorkSize == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010172: (clEnqueueNDRangeKernel) GlobalWorkSize is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
    }

    for (i = 0; i < WorkDim; i++)
    {
        if (GlobalWorkSize[i] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010173: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] is %d.\n",
                i, GlobalWorkSize[i]);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }
        if (GlobalWorkSize[i] > CommandQueue->device->deviceInfo.maxGlobalWorkSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010174: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) over hardware limit %lld.\n",
                i, GlobalWorkSize[i], CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }

        if (GlobalWorkOffset != gcvNULL &&
            (GlobalWorkSize[i] + GlobalWorkOffset[i] >
                CommandQueue->device->deviceInfo.maxGlobalWorkSize))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010175: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) + GlobalWorkOffset[%d] (%d) over hardware limit %lld.\n",
                i, GlobalWorkSize[i], i, GlobalWorkOffset[i],
                CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_OFFSET);
        }

        if (LocalWorkSize != gcvNULL)
        {
            if (LocalWorkSize[i] > CommandQueue->device->deviceInfo.maxWorkItemSizes[i])
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010176: (clEnqueueNDRangeKernel) LocalWorkSize[%d] (%d) is over maxWorkItemSize (%d).\n",
                    i, LocalWorkSize[i], CommandQueue->device->deviceInfo.maxWorkItemSizes[i]);
                clmRETURN_ERROR(CL_INVALID_WORK_ITEM_SIZE);
            }

            if (LocalWorkSize[i] == 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010177: (clEnqueueNDRangeKernel) LocalWorkSize[%d] is 0.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_WORK_ITEM_SIZE);
            }

            if (GlobalWorkSize[i] % LocalWorkSize[i] != 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010178: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) is not multiple of LocalWorkSize[%d] (%d).\n",
                    i, GlobalWorkSize[i], i, LocalWorkSize[i]);
                clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
            }
            if (Kernel->compileWorkGroupSize[0] != 0 && Kernel->compileWorkGroupSize[1] != 0 && Kernel->compileWorkGroupSize[2] != 0)
            {
                if (LocalWorkSize[i] != Kernel->compileWorkGroupSize[i])
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-010179: (clEnqueueNDRangeKernel) LocalWorkSize[%d] (%d) is not the same as specified in kernel (%d).\n",
                        i, LocalWorkSize[i], Kernel->compileWorkGroupSize[i]);
                    clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
                }
            }
            workGroupSize *= LocalWorkSize[i];
        }
    }

    if (workGroupSize > Kernel->maxWorkGroupSize)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010180: (clEnqueueNDRangeKernel) workGroupSize (%d) is larger than specified in kernel (%d).\n",
            workGroupSize, Kernel->maxWorkGroupSize);
        clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
    }
    else if (workGroupSize < 128)
    {
        /* We need a way to handle this in VIR */
    }

    for (i = 0; i < Kernel->kernelNumArgs; i++)
    {
        arg = &Kernel->srcArgs[i];
        if (arg && arg->set != gcvTRUE)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010181: (clEnqueueNDRangeKernel) argument %d is not set.\n",
                i);
            clmRETURN_ERROR(CL_INVALID_KERNEL_ARGS);
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_NDRANGE_VIR_KERNEL;
    command->handler                = &clfExecuteCommandNDRangeVIRKernel;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    if (Kernel->hasPrintf)
    {
        /* Create the release signal for the deferred command release. */
        clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                      &command->releaseSignal),
                   CL_OUT_OF_HOST_MEMORY);
        CommandQueue->needEnqueueNOP = gcvTRUE;
    }

    NDRangeKernel                       = &command->u.NDRangeVIRKernel;

    clfGetVIRScaleHint(Kernel, NDRangeKernel);

    NDRangeKernel->kernel               = Kernel;
    NDRangeKernel->workDim              = WorkDim;
    NDRangeKernel->globalWorkSize[0]    = GlobalWorkSize[0];
    NDRangeKernel->globalWorkSize[1]    = WorkDim>1 ? GlobalWorkSize[1] : 0;
    NDRangeKernel->globalWorkSize[2]    = WorkDim>2 ? GlobalWorkSize[2] : 0;
    NDRangeKernel->globalWorkOffset[0]  = GlobalWorkOffset ? GlobalWorkOffset[0] : 0;
    NDRangeKernel->globalWorkOffset[1]  = GlobalWorkOffset ? (WorkDim>1 ? GlobalWorkOffset[1] : 0) : 0;
    NDRangeKernel->globalWorkOffset[2]  = GlobalWorkOffset ? (WorkDim>2 ? GlobalWorkOffset[2] : 0) : 0;
    NDRangeKernel->localWorkSize[0]     = LocalWorkSize ? LocalWorkSize[0] : 0;
    NDRangeKernel->localWorkSize[1]     = LocalWorkSize ? (WorkDim>1 ? LocalWorkSize[1] : 0) : 0;
    NDRangeKernel->localWorkSize[2]     = LocalWorkSize ? (WorkDim>2 ? LocalWorkSize[2] : 0) : 0;

    clmONERROR(clfDuplicateVIRKernelArgs(Kernel, &NDRangeKernel->args),
               CL_OUT_OF_HOST_MEMORY);
    NDRangeKernel->numArgs = Kernel->kernelNumArgs;

    if((Kernel->virMasterInstance->hashKey != 0 && clfVIRNeedRecompile(NDRangeKernel, keyStateData, &keyStateSize, &currentKey))
        || Kernel->recompileThreadRemap == gcvTRUE)
    {
        clsVIRInstanceKey_PTR instance = gcvNULL;

        /* find the instance object by key if the kernel has been recompiled */
        instance = clfFindInstanceByKey(Kernel->virCacheTable, currentKey);

        if(instance == gcvNULL)
        {
            /* if not find in the cache table, recompile the kernel then store the instance into the cache table */
            clmONERROR(clfRecompileVIRKernel(Kernel, (gctUINT)workGroupSize), CL_LINK_PROGRAM_FAILURE);
            Kernel->virCurrentInstance->hashKey = currentKey;

            clmONERROR(gcoOS_AcquireMutex(gcvNULL, Kernel->cacheMutex, gcvINFINITE), CL_LINK_PROGRAM_FAILURE);
            aquired = gcvTRUE;

            clfAddInstanceKeyToHashTable(Kernel->virCacheTable, Kernel->virCurrentInstance, currentKey);

            clmONERROR(gcoOS_ReleaseMutex(gcvNULL, Kernel->cacheMutex), CL_LINK_PROGRAM_FAILURE);
            aquired = gcvFALSE;
        }
        else
        {
            /* as the kernel has been compiled before, just use the instance in the cache table */
            Kernel->virCurrentInstance = instance->virInstance;
        }
    }

    NDRangeKernel->currentInstance = Kernel->virCurrentInstance;

    /* Retain kernel. */
    clfRetainKernel(Kernel);

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010182: (clEnqueueNDRangeKernel) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }

    if(aquired)
    {
        gcoOS_ReleaseMutex(gcvNULL, Kernel->cacheMutex);
    }

    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(
    cl_command_queue    CommandQueue,
    cl_kernel           Kernel,
    cl_uint             WorkDim,
    const size_t *      GlobalWorkOffset,
    const size_t *      GlobalWorkSize,
    const size_t *      LocalWorkSize,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommandNDRangeKernel_PTR NDRangeKernel;
    clsCommand_PTR              command = gcvNULL;
    clsArgument_PTR             arg;
    gctUINT                     i;
    gctINT                      status;
    size_t                      workGroupSize = 1;
    clsPatchDirective_PTR       patchDirective = gcvNULL;
    gctPOINTER                  pointer = gcvNULL;
    size_t                      globalWorkOffset[3] = {0};
    size_t                      globalWorkSize[3] = {0};
    size_t                      localWorkSize[3] = {0};
    size_t                      shadowLocalWorkSize[3] = { 0 };
    size_t                      shadowGlobalWorkSize[3] = { 0 };
    gctUINT16                   *factor;


    gcmHEADER_ARG("CommandQueue=0x%x Kernel=0x%x "
                  "GlobalWorkOffset=0x%x GlobalWorkSize=0x%x GlobalWorkSize=0x%x",
                  CommandQueue, Kernel, GlobalWorkOffset, GlobalWorkSize, LocalWorkSize);
    gcmDUMP_API("${OCL clEnqueueNDRangeKernel 0x%x, 0x%x, %d}", CommandQueue, Kernel, WorkDim);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010165: (clEnqueueNDRangeKernel) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010166: (clEnqueueNDRangeKernel) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (Kernel->program == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010167: (clEnqueueNDRangeKernel) Kernel is not associate with any program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
    }

    if (CommandQueue->context != Kernel->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010168: (clEnqueueNDRangeKernel) CommandQueue's context is not the same as Kernel's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010169: (clEnqueueNDRangeKernel) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (Kernel->context->platform->virShaderPath)
    {
        factor = Kernel->virCurrentInstance ? Kernel->virCurrentInstance->hwStates.hints.workGroupSizeFactor : gcvNULL;
    }
    else
    {
        factor = Kernel->masterInstance.programState.hints ? Kernel->masterInstance.programState.hints->workGroupSizeFactor : gcvNULL;
    }

    if (factor)
    {
        if (clfThreadRemapNeedRecompile(Kernel, WorkDim, LocalWorkSize) == gcvFALSE)
        {
            for (i = 0; i < WorkDim; i++)
            {
                if (LocalWorkSize)
                {
                    shadowLocalWorkSize[i] = LocalWorkSize[i] / (factor[i] ? factor[i] : 1);
                }
                shadowGlobalWorkSize[i] = GlobalWorkSize[i] / (factor[i] ? factor[i] : 1);
            }

            GlobalWorkSize = shadowGlobalWorkSize;
            if (LocalWorkSize)
            {
                LocalWorkSize = shadowLocalWorkSize;
            }
        }
    }

    if (Kernel->context->platform->virShaderPath)
    {
        status= clfEnqueueNDRangeVIRKernel(CommandQueue,
                                           Kernel,
                                           WorkDim,
                                           GlobalWorkOffset,
                                           GlobalWorkSize,
                                           LocalWorkSize,
                                           NumEventsInWaitList,
                                           EventWaitList,
                                           Event);

        VCL_TRACE_API(EnqueueNDRangeKernel)(CommandQueue, Kernel, WorkDim, GlobalWorkOffset, GlobalWorkSize,
                                            LocalWorkSize, NumEventsInWaitList, EventWaitList, Event);
        gcmFOOTER_ARG("%d Command=0x%x", status, command);
        return status;
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010170: (clEnqueueNDRangeKernel) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (WorkDim < 1 || WorkDim > 3)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010171: (clEnqueueNDRangeKernel) invalid WorkDim (%d).\n",
            WorkDim);
        clmRETURN_ERROR(CL_INVALID_WORK_DIMENSION);
    }

    if (GlobalWorkSize == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010172: (clEnqueueNDRangeKernel) GlobalWorkSize is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
    }

    /* Check if we need convert 1D -> 2D, and update the Global variable,
       if convert, from here, we are an 2Dimension compute */
    clmONERROR(clfAddGlobalWorkSizeRecompile(
                                             CommandQueue,
                                             Kernel,
                                             &WorkDim,
                                             &GlobalWorkOffset,
                                             &GlobalWorkSize,
                                             &LocalWorkSize,
                                             globalWorkOffset,
                                             globalWorkSize,
                                             localWorkSize,
                                             &patchDirective
                                             ),
               CL_INVALID_GLOBAL_WORK_SIZE);

    for (i = 0; i < WorkDim; i++)
    {
        if (GlobalWorkSize[i] == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010173: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] is %d.\n",
                i, GlobalWorkSize[i]);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }
        if (GlobalWorkSize[i] > CommandQueue->device->deviceInfo.maxGlobalWorkSize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010174: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) over hardware limit %lld.\n",
                i, GlobalWorkSize[i], CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }

        if (GlobalWorkOffset != gcvNULL &&
            (GlobalWorkSize[i] + GlobalWorkOffset[i] >
                CommandQueue->device->deviceInfo.maxGlobalWorkSize))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010175: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) + GlobalWorkOffset[%d] (%d) over hardware limit %lld.\n",
                i, GlobalWorkSize[i], i, GlobalWorkOffset[i],
                CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_OFFSET);
        }

        if (LocalWorkSize != gcvNULL)
        {
            if (LocalWorkSize[i] > CommandQueue->device->deviceInfo.maxWorkItemSizes[i])
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010176: (clEnqueueNDRangeKernel) LocalWorkSize[%d] (%d) is over maxWorkItemSize (%d).\n",
                    i, LocalWorkSize[i], CommandQueue->device->deviceInfo.maxWorkItemSizes[i]);
                clmRETURN_ERROR(CL_INVALID_WORK_ITEM_SIZE);
            }

            if (LocalWorkSize[i] == 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010177: (clEnqueueNDRangeKernel) LocalWorkSize[%d] is 0.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_WORK_ITEM_SIZE);
            }

            if (GlobalWorkSize[i] % LocalWorkSize[i] != 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010178: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) is not multiple of LocalWorkSize[%d] (%d).\n",
                    i, GlobalWorkSize[i], i, LocalWorkSize[i]);
                clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
            }
            if (Kernel->compileWorkGroupSize[0] != 0 && Kernel->compileWorkGroupSize[1] != 0 && Kernel->compileWorkGroupSize[2] != 0)
            {
                if (LocalWorkSize[i] != Kernel->compileWorkGroupSize[i])
                {
                    gcmUSER_DEBUG_ERROR_MSG(
                        "OCL-010179: (clEnqueueNDRangeKernel) LocalWorkSize[%d] (%d) is not the same as specified in kernel (%d).\n",
                        i, LocalWorkSize[i], Kernel->compileWorkGroupSize[i]);
                    clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
                }
            }
            workGroupSize *= LocalWorkSize[i];
        }
    }

    if (workGroupSize > Kernel->maxWorkGroupSize)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010180: (clEnqueueNDRangeKernel) workGroupSize (%d) is larger than specified in kernel (%d).\n",
            workGroupSize, Kernel->maxWorkGroupSize);
        clmRETURN_ERROR(CL_INVALID_WORK_GROUP_SIZE);
    }
    else if (workGroupSize < 128)
    {
        gcSHADER    kernelBinary = (gcSHADER) Kernel->masterInstance.binary;

        /* For those chips that can't support PSCS throttle:
            during compile time, we compute the temp register count to fit the local memory size
            based on workgroupsize is 128. If the real workgroupSize is smaller than 128, we need
            to adjust hw temp count to fit the local memory requirements.
        */
        if (gcShaderUseLocalMem(kernelBinary) &&
            !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_PSCS_THROTTLE))
        {
            gcKERNEL_FUNCTION   kernelFunction = GetShaderCurrentKernelFunction(kernelBinary);
            gctUINT localMemoryReq = (gctUINT)(CommandQueue->device->deviceInfo.localMemSize / kernelFunction->localMemorySize);
            gctUINT maxFreeReg = 113;
            gctUINT threadCount = 4 * CommandQueue->device->deviceInfo.ShaderCoreCount;
            gctUINT hwRegCount = (maxFreeReg * threadCount) / (localMemoryReq * workGroupSize);
            kernelBinary->RARegWaterMark = gcmMAX(kernelBinary->RARegWaterMark, hwRegCount);
        }
    }


    for (i = 0; i < Kernel->numArgs; i++)
    {
        arg = clfGetKernelArg(Kernel, i, gcvNULL, gcvNULL, gcvNULL);
        if (arg && arg->set != gcvTRUE)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010181: (clEnqueueNDRangeKernel) argument %d is not set.\n",
                i);
            clmRETURN_ERROR(CL_INVALID_KERNEL_ARGS);
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_NDRANGE_KERNEL;
    command->handler                = &clfExecuteCommandNDRangeKernel;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    if (Kernel->hasPrintf)
    {
        /* Create the release signal for the deferred command release. */
        clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                      &command->releaseSignal),
                   CL_OUT_OF_HOST_MEMORY);
    }

    NDRangeKernel                       = &command->u.NDRangeKernel;

    clfGetScaleHint(Kernel, NDRangeKernel);

    NDRangeKernel->kernel               = Kernel;
    NDRangeKernel->workDim              = WorkDim;
    NDRangeKernel->globalWorkSize[0]    = GlobalWorkSize[0];
    NDRangeKernel->globalWorkSize[1]    = WorkDim>1 ? GlobalWorkSize[1] : 0;
    NDRangeKernel->globalWorkSize[2]    = WorkDim>2 ? GlobalWorkSize[2] : 0;
    NDRangeKernel->globalWorkOffset[0]  = GlobalWorkOffset ? GlobalWorkOffset[0] : 0;
    NDRangeKernel->globalWorkOffset[1]  = GlobalWorkOffset ? (WorkDim>1 ? GlobalWorkOffset[1] : 0) : 0;
    NDRangeKernel->globalWorkOffset[2]  = GlobalWorkOffset ? (WorkDim>2 ? GlobalWorkOffset[2] : 0) : 0;
    NDRangeKernel->localWorkSize[0]     = LocalWorkSize ? LocalWorkSize[0] : 0;
    NDRangeKernel->localWorkSize[1]     = LocalWorkSize ? (WorkDim>1 ? LocalWorkSize[1] : 0) : 0;
    NDRangeKernel->localWorkSize[2]     = LocalWorkSize ? (WorkDim>2 ? LocalWorkSize[2] : 0) : 0;

    clmONERROR(clfDuplicateKernelArgs(Kernel, &NDRangeKernel->args),
               CL_OUT_OF_HOST_MEMORY);
    NDRangeKernel->numArgs = Kernel->numArgs;

    clmONERROR(clfAddKernelRecompile(CommandQueue,Kernel,NDRangeKernel,&patchDirective),
               CL_INVALID_VALUE);

    if (clfNeedRecompile(Kernel, patchDirective))
    {
        clmONERROR(clfRecompileKernel(Kernel,NDRangeKernel, patchDirective),
            CL_INVALID_VALUE);
    }
    else if(patchDirective && (Kernel->isPatched == gcvTRUE))
    {
        clmONERROR(clfUpdateKernelArgs(Kernel,
                                       &NDRangeKernel->numArgs,
                                       &NDRangeKernel->args,
                                       patchDirective),
                   CL_INVALID_VALUE);

        NDRangeKernel->currentInstance = Kernel->recompileInstance;

        if(patchDirective)
        {
            clfDestroyPatchDirective(&patchDirective);
        }
    }
    else
    {
        NDRangeKernel->currentInstance = &Kernel->masterInstance;
    }

    /* Retain kernel. */
    clfRetainKernel(Kernel);

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueNDRangeKernel)(CommandQueue, Kernel, WorkDim, GlobalWorkOffset, GlobalWorkSize,
                                        LocalWorkSize, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010182: (clEnqueueNDRangeKernel) Run out of memory.\n");
    }

    if(patchDirective)
    {
        clfDestroyPatchDirective(&patchDirective);
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(
    cl_command_queue    CommandQueue,
    cl_kernel           Kernel,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandTask_PTR  task;
    gctINT              status;
    clsPatchDirective_PTR patchDirective = gcvNULL;
    gctPOINTER          pointer = gcvNULL;

    gcmHEADER_ARG("CommandQueue=0x%x Kernel=0x%x "
                  "NumEventsInWaitList=%u EventWaitList=0x%x Event=0x%x",
                  CommandQueue, Kernel, NumEventsInWaitList, EventWaitList, Event);
    gcmDUMP_API("${OCL clEnqueueTask 0x%x, 0x%x}", CommandQueue, Kernel);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010183: (clEnqueueTask) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Kernel == gcvNULL || Kernel->objectType != clvOBJECT_KERNEL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010184: (clEnqueueTask) invalid Kernel.\n");
        clmRETURN_ERROR(CL_INVALID_KERNEL);
    }

    if (Kernel->program == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010185: (clEnqueueTask) Kernel is not associate with any program.\n");
        clmRETURN_ERROR(CL_INVALID_PROGRAM_EXECUTABLE);
    }

    if (CommandQueue->context != Kernel->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010186: (clEnqueueTask) CommandQueue's context is not the same as Kernel's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010187: (clEnqueueTask) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    /*  accordding to ocl1.2 spec, clEnqueueTask is equivalent to calling clEnqueueNDRangeKernel with work_dim = 1,
        global_work_offset = NULL, global_work_size[0] set to 1 and local_work_size[0] set to 1.    */
    if (Kernel->context->platform->virShaderPath)
    {
        size_t                      globalWorkSize = {1};
        size_t                      localWorkSize = {1};

        status= clfEnqueueNDRangeVIRKernel(CommandQueue,
                                           Kernel,
                                           1,
                                           gcvNULL,
                                           &globalWorkSize,
                                           &localWorkSize,
                                           NumEventsInWaitList,
                                           EventWaitList,
                                           Event);

        VCL_TRACE_API(EnqueueTask)(CommandQueue, Kernel, NumEventsInWaitList, EventWaitList, Event);
        gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
        return status;
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010188: (clEnqueueTask) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    /* TODO - return CL_INVALID_WORK_GROUP_SIZE if workgroup size is not (1, 1, 1) */

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_TASK;
    command->handler                = &clfExecuteCommandTask;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    if (Kernel->hasPrintf)
    {
        /* Create the release signal for the deferred command release. */
        clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                      &command->releaseSignal),
                   CL_OUT_OF_HOST_MEMORY);
    }

    task                            = &command->u.task;
    task->kernel                    = Kernel;

    clmONERROR(clfDuplicateKernelArgs(Kernel, &task->args),
               CL_OUT_OF_HOST_MEMORY);
    task->numArgs = Kernel->numArgs;

    if (patchDirective)
    {
        /* Patch shader */
        clmONERROR(clfDynamicPatchKernel(Kernel,
                                         &task->numArgs,
                                         &task->args,
                                         patchDirective),
                   status);

        task->currentInstance = Kernel->recompileInstance;
    }
    else
    {
        task->currentInstance = &Kernel->masterInstance;
    }

    /* Retain kernel. */
    clfRetainKernel(Kernel);

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueTask)(CommandQueue, Kernel, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010189: (clEnqueueTask) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(
    cl_command_queue    CommandQueue,
    void                (CL_CALLBACK * UserFunc)(void *),
    void *              Args,
    size_t              CbArgs,
    cl_uint             NumMemObjects,
    const cl_mem *      MemList,
    const void **       ArgsMemLoc,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandNativeKernel_PTR nativeKernel;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x UserFunc=0x%x Args=0x%x",
                  CommandQueue, UserFunc, Args);
    gcmDUMP_API("${OCL clEnqueueNativeKernel 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010190: (clEnqueueNativeKernel) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010191: (clEnqueueNativeKernel) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010192: (clEnqueueNativeKernel) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (UserFunc == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010193: (clEnqueueNativeKernel) UserFunc is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (MemList == gcvNULL && NumMemObjects > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010194: (clEnqueueNativeKernel) invalid MemList.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (MemList)
    {
        gctUINT i = 0;

        if (NumMemObjects == 0)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010195: (clEnqueueNativeKernel) MemList is not NULL, but NumMemObjects is 0.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }

        for (i = 0; i < NumMemObjects; i++)
        {
            if (MemList[i] == gcvNULL ||
                MemList[i]->objectType != clvOBJECT_MEM ||
                MemList[i]->type != CL_MEM_OBJECT_BUFFER)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010196: (clEnqueueNativeKernel) MemList[%d] is invalid.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
            }
        }
    }

    if (Args == gcvNULL && (CbArgs > 0 || NumMemObjects > 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010197: (clEnqueueNativeKernel) Args is NULL, but CbArgs is not 0 or NumMemObjects is not 0).\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Args != gcvNULL && CbArgs == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010198: (clEnqueueNativeKernel) Args is not NULL, but CbArgs is 0).\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if ((CommandQueue->device->deviceInfo.execCapability & CL_EXEC_NATIVE_KERNEL) == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010199: (clEnqueueNativeKernel) device's cannot execute native kernel.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_NATIVE_KERNEL;
    command->handler                = &clfExecuteCommandNativeKernel;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    nativeKernel                    = &command->u.nativeKernel;
    nativeKernel->userFunc          = UserFunc;
    nativeKernel->args              = Args;
    nativeKernel->cbArgs            = CbArgs;
    nativeKernel->numMemObjects     = NumMemObjects;
    nativeKernel->memList           = MemList;
    nativeKernel->argsMemLoc        = ArgsMemLoc;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueNativeKernel)(CommandQueue, UserFunc, Args, CbArgs, NumMemObjects, MemList, ArgsMemLoc, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010200: (clEnqueueNativeKernel) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarker(
    cl_command_queue    CommandQueue,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Event=0x%x", CommandQueue, Event);
    gcmDUMP_API("${OCL clEnqueueMarker 0x%x, 0x%x}", CommandQueue, Event);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010201: (clEnqueueMarker) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Event == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010202: (clEnqueueMarker) Event is not NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    command->type       = clvCOMMAND_MARKER;
    command->handler    = &clfExecuteCommandMarker;
    command->outEvent   = Event;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueMarker)(CommandQueue, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010203: (clEnqueueMarker) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(
    cl_command_queue    CommandQueue,
    cl_uint             NumEventsInWaitList,
    const cl_event *    EventWaitList,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x NumEventsInWaitList=%u EventWaitList=0x%x Event=0x%x",
                  CommandQueue, NumEventsInWaitList, EventWaitList, Event);
    gcmDUMP_API("${OCL clEnqueueMarkerWithWaitList 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010305: (clEnqueueMarkerWithWaitList) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010306: (clEnqueueMarkerWithWaitList) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_MARKER;
    command->handler                = &clfExecuteCommandMarker;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueMarkerWithWaitList)(CommandQueue, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010307: (clEnqueueMarkerWithWaitList) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(
    cl_command_queue   CommandQueue ,
    cl_uint            NumEventsInWaitList ,
    const cl_event *   EventWaitList ,
    cl_event *         Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x NumEventsInWaitList=%u EventWaitList=0x%x Event=0x%x",
                  CommandQueue, NumEventsInWaitList, EventWaitList, Event);
    gcmDUMP_API("${OCL clEnqueueBarrierWithWaitList 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010308: (clEnqueueBarrierWithWaitList) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010309: (clEnqueueBarrierWithWaitList) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_BARRIER;
    command->handler                = &clfExecuteCommandBarrier;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueBarrierWithWaitList)(CommandQueue, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010310: (clEnqueueBarrierWithWaitList) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWaitForEvents(
    cl_command_queue    CommandQueue,
    cl_uint             NumEvents,
    const cl_event *    EventList
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x NumEvents=%u EventList=0x%x",
                  CommandQueue, NumEvents, EventList);
    gcmDUMP_API("${OCL clEnqueueWaitForEvents 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010204: (clEnqueueWaitForEvents) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (EventList == gcvNULL && NumEvents > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010205: (clEnqueueWaitForEvents) EventList is NULL, but NumEvents is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEvents == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i = 0; i < NumEvents; i++)
        {
            if (CommandQueue->context != EventList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010206: (clEnqueueWaitForEvents) EventList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventList && (NumEvents > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEvents, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventList, sizeof(gctPOINTER) * NumEvents);
    }

    command->type                   = clvCOMMAND_WAIT_FOR_EVENTS;
    command->handler                = &clfExecuteCommandWaitForEvents;
    command->outEvent               = gcvNULL;
    command->numEventsInWaitList    = NumEvents;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueWaitForEvents)(CommandQueue, NumEvents, EventList);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010207: (clEnqueueWaitForEvents) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrier(
    cl_command_queue    CommandQueue
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x", CommandQueue);
    gcmDUMP_API("${OCL clEnqueueBarrier 0x%x}", CommandQueue);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010208: (clEnqueueBarrier) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    command->type       = clvCOMMAND_BARRIER;
    command->handler    = &clfExecuteCommandBarrier;
    command->outEvent   = gcvNULL;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueBarrier)(CommandQueue);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010209: (clEnqueueBarrier) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(
    cl_command_queue    CommandQueue ,
    cl_mem              Buffer ,
    const void *        Pattern ,
    size_t              PatternSize ,
    size_t              Offset ,
    size_t              Size ,
    cl_uint             NumEventsInWaitList ,
    const cl_event *    EventWaitList ,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    clsCommandFillBuffer_PTR fillBuffer;
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Buffer=0x%x Pattern=0x%x Offset=%lu Size=%lu",
                  CommandQueue, Buffer, Pattern, Offset, Size);
    gcmDUMP_API("${OCL clEnqueueFillBuffer 0x%x, 0x%x}", CommandQueue, Buffer);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010310: (clEnqueueFillBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    if (Buffer == gcvNULL ||
        Buffer->objectType != clvOBJECT_MEM ||
        Buffer->type != CL_MEM_OBJECT_BUFFER)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010312: (clEnqueueFillBuffer) invalid Buffer.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (CommandQueue->context != Buffer->context)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010313: (clEnqueueFillBuffer) CommandQueue's context is not the same as Buffer's context.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (Pattern == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010314: (clEnqueueFillBuffer) Ptr is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010315: (clEnqueueFillBuffer) EventWaitList is NULL, but NumEventsInWaitList is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (EventWaitList)
    {
        gctUINT i = 0;
        clmCHECK_ERROR(NumEventsInWaitList == 0, CL_INVALID_EVENT_WAIT_LIST);
        for (i=0; i<NumEventsInWaitList; i++)
        {
            if (CommandQueue->context != EventWaitList[i]->context)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010316: (clEnqueueFillBuffer) EventWaitList[%d]'s context is not the same as CommandQueue's context.\n",
                    i);
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (Buffer->u.buffer.size < Offset+Size)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010317: (clEnqueueFillBuffer) (Offset + Size) is larger than Buffer's size.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Add reference count for memory object here to make sure the memory object will not be released in another thread, maybe refine later for performance issue */
    clfRetainMemObject(Buffer);

    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    if (EventWaitList && (NumEventsInWaitList > 0))
    {
        clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctPOINTER) * NumEventsInWaitList, &pointer), CL_OUT_OF_HOST_MEMORY);
        gcoOS_MemCopy(pointer, (gctPOINTER)EventWaitList, sizeof(gctPOINTER) * NumEventsInWaitList);
    }

    command->type                   = clvCOMMAND_FILL_BUFFER;
    command->handler                = &clfExecuteCommandFillBuffer;
    command->outEvent               = Event;
    command->numEventsInWaitList    = NumEventsInWaitList;
    command->eventWaitList          = (clsEvent_PTR *)pointer;

    fillBuffer                     = &command->u.fillBuffer;
    fillBuffer->buffer             = Buffer;
    fillBuffer->offset             = Offset;
    fillBuffer->size               = Size;
    fillBuffer->pattern_size       = PatternSize;
    fillBuffer->pattern            = Pattern;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    VCL_TRACE_API(EnqueueFillBuffer)(CommandQueue, Buffer, Pattern, PatternSize, Offset, Size, NumEventsInWaitList, EventWaitList, Event);
    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010318: (clEnqueueFillBuffer) Run out of memory.\n");
    }

    if(command != gcvNULL)
    {
        clfReleaseCommand(command);
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}
