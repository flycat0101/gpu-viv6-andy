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

#define __NEXT_MSG_ID__     010319

#define _USE_HW_TEXLD_      1

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
    clsPatchDirective *         pointer;
    clsPatchReadImage *         readImage;

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
    clsPatchDirective *         pointer;
    clsPatchWriteImage *        writeImage;

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
    gctUINT                 bufferSize = 0;
    gctPOINTER              buffer, pointer = gcvNULL;
    gcsHINT_PTR             hints = gcvNULL;
    clsKernelStates_PTR     patchedStates = gcvNULL;
    clsPatchDirective_PTR   patchDirective;
    clsPatchDirective_PTR   patchDirective1;
    gcPatchDirective_PTR    gcPatchDirective = gcvNULL;
    gctUINT                 oldNumArgs, numToUpdate = *NumArgs, i;

    struct _imageData {
        gctUINT             physical;   /* GPU address is 32-bit. */
        gctUINT             pitch;
        gctUINT             slice;      /* 3D/2D array. */
    } imageData;

    if (RecompilePatchDirectives == gcvNULL)
    {
        return gcvSTATUS_OK;
    }


    if (patchedStates)
    {
        gctUINT                 oldNumArgs;

        if (patchedStates != Kernel->patchedStates)
        {
            clsKernelStates_PTR prevPatchedStates;

            /* Move the matched states to the front. */
            for (prevPatchedStates = Kernel->patchedStates;
                 prevPatchedStates->next != patchedStates;
                 prevPatchedStates = prevPatchedStates->next) ;

            prevPatchedStates->next = patchedStates->next;
            Kernel->patchedStates = patchedStates;
        }

        /* Add new uniforms. */
        kernelBinary = (gcSHADER)patchedStates->binary;
        oldNumArgs = *NumArgs;
        *NumArgs = patchedStates->numArgs;
        clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                           *NumArgs,
                                           Args),
                   CL_OUT_OF_HOST_MEMORY);
        gcoOS_ZeroMemory(*Args + oldNumArgs, (*NumArgs - oldNumArgs) * gcmSIZEOF(clsArgument));

        /* Set arguments' value. */
        for (patchDirective  = RecompilePatchDirectives,
             patchDirective1 = patchedStates->patchDirective;
             patchDirective && patchDirective1;
             patchDirective  = patchDirective->next,
             patchDirective1 = patchDirective1->next)
        {
            if (patchDirective->kind == gceRK_PATCH_READ_IMAGE)
            {
                clsPatchReadImage * readImage = patchDirective->patchValue.readImage;
                clsPatchReadImage * readImage1 = patchDirective1->patchValue.readImage;
                clsImageHeader_PTR  imageHeader = readImage->imageHeader;
                clsArgument_PTR     argument;
                gctSIZE_T           bytes;

                argument = &((*Args)[oldNumArgs]);
                bytes = sizeof(struct _imageData);
                imageData.physical = imageHeader->physical;
                imageData.pitch    = imageHeader->rowPitch;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageData, bytes);
                argument->uniform    = GetShaderUniform(kernelBinary, readImage1->imageDataIndex);
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                argument = &((*Args)[oldNumArgs + 1]);
                bytes = gcmSIZEOF(cl_uint) * 2;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
                argument->uniform    = GetShaderUniform(kernelBinary, readImage1->imageSizeIndex);
                argument->size       = bytes;
                argument->set        = gcvTRUE;
            }
            else if (patchDirective->kind == gceRK_PATCH_WRITE_IMAGE)
            {
                clsPatchWriteImage * writeImage = patchDirective->patchValue.writeImage;
                clsPatchWriteImage * writeImage1 = patchDirective1->patchValue.writeImage;
                clsImageHeader_PTR  imageHeader = writeImage->imageHeader;
                clsArgument_PTR     argument;
                gctSIZE_T           bytes;

                argument = &((*Args)[oldNumArgs]);
                bytes = sizeof(struct _imageData);
                imageData.physical = imageHeader->physical;
                imageData.pitch    = imageHeader->rowPitch;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageData, bytes);
                argument->uniform    = GetShaderUniform(kernelBinary, writeImage1->imageDataIndex);
                argument->size       = bytes;
                argument->set        = gcvTRUE;

                argument = &((*Args)[oldNumArgs + 1]);
                bytes = gcmSIZEOF(cl_uint) * 2;
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
                argument->uniform    = GetShaderUniform(kernelBinary, writeImage1->imageSizeIndex);
                argument->size       = bytes;
                argument->set        = gcvTRUE;
            }
            else if (patchDirective->kind == gceRK_PATCH_CL_LONGULONG)
            {
                clsArgument_PTR     argument;
                gctSIZE_T           bytes;

                argument = &((*Args)[oldNumArgs]);
                bytes = sizeof(cl_uint);
                clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);
                *((cl_uint*)argument->data) = patchDirective1->patchValue.longULong->channelCount;
                argument->uniform    = GetShaderUniform(kernelBinary, patchDirective1->patchValue.longULong->channelCountIndex);
                argument->size       = bytes;
                argument->set        = gcvTRUE;
            }
        }

        return gcvSTATUS_OK;
    }

   /* Save program binary into buffer */
    clmONERROR(gcSHADER_SaveEx((gcSHADER)Kernel->states.binary, gcvNULL, &binarySize),
               CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcoOS_Allocate(gcvNULL, binarySize, &pointer),
               CL_OUT_OF_HOST_MEMORY);
    clmONERROR(gcSHADER_SaveEx((gcSHADER)Kernel->states.binary, pointer, &binarySize),
               CL_OUT_OF_HOST_MEMORY);

    /* Construct kernel binary. */
    clmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_CL, &kernelBinary),
               CL_OUT_OF_HOST_MEMORY);

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
        gctUINT dstFormat;
        gctUINT opcode;

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

            /* Add new uniforms. */
            oldNumArgs = *NumArgs;
            *NumArgs += 2;
            clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                               *NumArgs,
                                               Args),
                       CL_OUT_OF_HOST_MEMORY);
            gcoOS_ZeroMemory(*Args + oldNumArgs, 2 * gcmSIZEOF(clsArgument));

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
#if BUILD_OPENCL_12
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
#else
            gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
#endif
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
            gcoOS_ZeroMemory(*Args + oldNumArgs, 2 * gcmSIZEOF(clsArgument));

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
#if BUILD_OPENCL_12
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
#else
            gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
#endif
            gcoOS_MemCopy(argument->data, &imageHeader->width, bytes);
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
            break;

        case gceRK_PATCH_CL_LONGULONG:
            {
                dstFormat = gcmSL_TARGET_GET(GetInstTemp(patchDirective->patchValue.longULong->instruction), Format);
                opcode = gcmSL_OPCODE_GET(patchDirective->patchValue.longULong->instruction->opcode, Opcode);
                if (NEED_PATCH_LONGULONG(patchDirective->patchValue.longULong->instruction, opcode, dstFormat))
                {
                    /* Add a patch argument. */
                    oldNumArgs = *NumArgs;
                    *NumArgs += 1;
                    clmONERROR(clfReallocateKernelArgs(oldNumArgs,
                                                       *NumArgs,
                                                       Args),
                               CL_OUT_OF_HOST_MEMORY);
                    gcoOS_ZeroMemory(*Args + oldNumArgs, gcmSIZEOF(clsArgument));

                    clmONERROR(gcSHADER_AddUniform(kernelBinary, "#channel_count", gcSHADER_INTEGER_X1, 1, gcSHADER_PRECISION_DEFAULT, &gcUniform1),
                                CL_OUT_OF_HOST_MEMORY);
                    SetUniformFlag(gcUniform1, gcvUNIFORM_FLAG_KERNEL_ARG_PATCH);
                    clmONERROR(gcUNIFORM_SetFormat(gcUniform1, gcSL_UINT32, gcvFALSE), CL_OUT_OF_HOST_MEMORY);
                    argument = &((*Args)[oldNumArgs]);
                    bytes = gcmSIZEOF(cl_uint);
                    clmONERROR(gcoOS_Allocate(gcvNULL, bytes, &argument->data), CL_OUT_OF_HOST_MEMORY);

                    *((gctUINT*)argument->data) = patchDirective->patchValue.longULong->channelCount;
                    argument->uniform    = gcUniform1;
                    argument->size       = bytes;
                    argument->set        = gcvTRUE;
                    patchDirective->patchValue.longULong->channelCountIndex = GetUniformIndex(gcUniform1);
                }

                clmONERROR(gcCreateCLLongULongDirective(patchDirective->patchValue.longULong->instructionIndex,
                                                        patchDirective->patchValue.longULong->channelCountIndex,
                                                        &gcPatchDirective),
                                                        CL_OUT_OF_HOST_MEMORY);
            }
            break;

        default:
            gcmASSERT(gcvFALSE);  /* not implemented yet */
            break;
        }
    }

    /* Recompile shader */
    gcmASSERT(Kernel->context->platform->compiler11);
    gcSetCLCompiler(Kernel->context->platform->compiler11);

    clmONERROR(gcSHADER_DynamicPatch(kernelBinary,
                                     gcPatchDirective),
                                     CL_OUT_OF_HOST_MEMORY);

    gcDestroyPatchDirective(&gcPatchDirective);

    /* No need to recalculate binarySize. */
    /*clmONERROR(gcSHADER_SaveEx(kernelBinary,
                               gcvNULL,
                               &binarySize),
                               CL_OUT_OF_HOST_MEMORY);*/

    /* Assume all dead code is removed by optimizer. */
    clmONERROR(gcLinkKernel(kernelBinary,
                            gcvSHADER_RESOURCE_USAGE /*| gcvSHADER_DEAD_CODE*/ | gcvSHADER_OPTIMIZER | gcvSHADER_REMOVE_UNUSED_UNIFORMS,
                            &bufferSize,
                            &buffer,
                            &hints),
                            CL_OUT_OF_RESOURCES);

    clmONERROR(gcoOS_Allocate(gcvNULL, sizeof(clsKernelStates), &pointer),
               CL_OUT_OF_HOST_MEMORY);

    patchedStates = (clsKernelStates_PTR) pointer;
    patchedStates->next             = Kernel->patchedStates;
    Kernel->patchedStates           = patchedStates;
    patchedStates->binary           = (gctUINT8_PTR)kernelBinary;
    patchedStates->numArgs          = *NumArgs;
    patchedStates->stateBuffer      = buffer;
    patchedStates->stateBufferSize  = bufferSize;
    patchedStates->hints            = hints;
    patchedStates->patchDirective   = RecompilePatchDirectives;

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
    /* In case a operation before free memory has error. pointer should free properly. */
    if (pointer)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
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

#if BUILD_OPENCL_12
    if (Buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010210: (clEnqueueReadBuffer) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }
#endif

    gcoCL_SetHardware();
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

#if cldSEQUENTIAL_EXECUTION
    clmONERROR(clfExecuteCommandReadBuffer(command),
               CL_OUT_OF_HOST_MEMORY);
#else
    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingRead),
               CL_OUT_OF_HOST_MEMORY);
#endif

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010008: (clEnqueueReadBuffer) Run out of memory.\n");
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

#if BUILD_OPENCL_12
    if (Buffer->flags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010211: (clEnqueueReadBufferRect) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }
#endif

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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010022: (clEnqueueReadBufferRect) Run out of memory.\n");
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

#if BUILD_OPENCL_12
    if (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010212: (clEnqueueWriteBuffer) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }
#endif
    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010031: (clEnqueueWriteBuffer) Run out of memory.\n");
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

#if BUILD_OPENCL_12
    if (Buffer->flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010213: (clEnqueueWriteBufferRect) Host flag is not compatible.\n");
        clmRETURN_ERROR(CL_INVALID_OPERATION);
    }
#endif

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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010045: (clEnqueueWriteBufferRect) Run out of memory.\n");
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010056: (clEnqueueCopyBuffer) Run out of memory.\n");
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
    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010072: (clEnqueueCopyBufferRect) Run out of memory.\n");
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
#if BUILD_OPENCL_12
    size_t              slicePitch;
#endif
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingRead=%u Origin=0x%x Region=0x%x "
                  "RowPitch=%u SlicePitch=%u Ptr=0x%x",
                  CommandQueue, Image, BlockingRead, Origin, Region, RowPitch, SlicePitch, Ptr);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010073: (clEnqueueReadImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }
#if BUILD_OPENCL_12
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
#else
    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010074: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif

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

#if BUILD_OPENCL_12
    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.width)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.arraySize)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height)
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
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010222: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010223: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010224: (clEnqueueReadImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }
#else
    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010080: (clEnqueueReadImage) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Image->type == CL_MEM_OBJECT_IMAGE2D &&
        (Origin[2] != 0 || Region[2] != 1 || SlicePitch != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010081: (clEnqueueReadImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1 or SlicePitch is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of Image */
    if (Origin[0] + Region[0] > Image->u.image.width ||
        Origin[1] + Region[1] > Image->u.image.height ||
        Origin[2] + Region[2] > Image->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010082: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif

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

#if BUILD_OPENCL_12
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
#endif
    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010084: (clEnqueueReadImage) Run out of memory.\n");
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
#if BUILD_OPENCL_12
    size_t              inputSlicePitch;
#endif
    gctPOINTER          pointer = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingWrite=%u Origin=0x%x Region=0x%x"
                  "InputRowPitch=%u InputSlicePitch=%u Ptr=0x%x",
                  CommandQueue, Image, BlockingWrite, Origin, Region,
                  InputRowPitch, InputSlicePitch, Ptr);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010085: (clEnqueueWriteImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

#if BUILD_OPENCL_12
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
#else
    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010086: (clEnqueueWriteImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif

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

#if BUILD_OPENCL_12
    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.width)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.arraySize)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010234: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010235: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010236: (clEnqueueWriteImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }
#else
    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010092: (clEnqueueWriteImage) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Image->type == CL_MEM_OBJECT_IMAGE2D &&
        (Origin[2] != 0 || Region[2] != 1 || InputSlicePitch != 0))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010093: (clEnqueueWriteImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1 or InputSlicePitch is not 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of Image */
    if (Origin[0] + Region[0] > Image->u.image.width ||
        Origin[1] + Region[1] > Image->u.image.height ||
        Origin[2] + Region[2] > Image->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010094: (clEnqueueWriteImage) (Origina + Region) is outside of Image's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif

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

#if BUILD_OPENCL_12
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
#endif
    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010096: (clEnqueueWriteImage) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12
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
        if (Origin[0] + Region[0] > Image->u.image.width)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.arraySize)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010249: (clEnqueueReadImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.depth)
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

    if (clfImageFormat2GcFormat(&(Image->u.image.format), &elementSize, gcvNULL, gcvNULL))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010252: (clEnqueueReadImage) invalid format descriptor.\n");
        clmRETURN_ERROR(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }



    clmONERROR(gcoOS_Allocate(gcvNULL, elementSize, &fillPtr), CL_OUT_OF_HOST_MEMORY);

    switch (Image->u.image.format.image_channel_data_type)
    {
    case CL_SIGNED_INT8:
    case CL_SIGNED_INT16:
    case CL_SIGNED_INT32:
        clfPackImagePixeli((cl_int*)FillColor, &Image->u.image.format, fillPtr);
        break;
    case CL_UNSIGNED_INT8:
    case CL_UNSIGNED_INT16:
    case CL_UNSIGNED_INT32:
        clfPackImagePixelui((cl_uint*)FillColor, &Image->u.image.format, fillPtr);
        break;
    default:
        clfPackImagePixelf((cl_float*)FillColor, &Image->u.image.format, fillPtr);
        break;

    }


    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;


OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010253: (clEnqueueFillImage) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

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

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010097: (clEnqueueCopyImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }
#if BUILD_OPENCL_12
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
#else
    if (SrcImage == gcvNULL ||
        SrcImage->objectType != clvOBJECT_MEM ||
        (SrcImage->type != CL_MEM_OBJECT_IMAGE2D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010098: (clEnqueueCopyImage) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }

    if (DstImage == gcvNULL ||
        DstImage->objectType != clvOBJECT_MEM ||
        (DstImage->type != CL_MEM_OBJECT_IMAGE2D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010099: (clEnqueueCopyImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif


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

    if (SrcImage->u.image.format.image_channel_order !=
        DstImage->u.image.format.image_channel_order)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010102: (clEnqueueCopyImage) SrcImage's channel order is not the same as DstImage's.\n");
        clmRETURN_ERROR(CL_INVALID_CONTEXT);
    }

    if (SrcImage->u.image.format.image_channel_data_type !=
        DstImage->u.image.format.image_channel_data_type)
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

#if BUILD_OPENCL_12
    switch (SrcImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width)
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
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.arraySize)
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
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height)
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
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010262: (clEnqueueCopyImage) (SrcOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.depth)
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
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width)
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
         if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.arraySize)
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
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height)
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
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010271: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010272: (clEnqueueCopyImage) (DstOrigina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010273: (clEnqueueCopyImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }
#else
    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010106: (clEnqueueCopyImage) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (SrcImage->type == CL_MEM_OBJECT_IMAGE2D &&
        (SrcOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010107: (clEnqueueCopyImage) SrcImage is 2D, but SrcOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (DstImage->type == CL_MEM_OBJECT_IMAGE2D &&
        (DstOrigin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010108: (clEnqueueCopyImage) DstImage is 2D, but DstOrigin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of SrcImage */
    if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
        SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
        SrcOrigin[2] + Region[2] > SrcImage->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010109: (clEnqueueCopyImage) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of DstImage */
    if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
        DstOrigin[1] + Region[1] > DstImage->u.image.height ||
        DstOrigin[2] + Region[2] > DstImage->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010110: (clEnqueueCopyImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif

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
    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010112: (clEnqueueCopyImage) Run out of memory.\n");
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

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010113: (clEnqueueCopyImageToBuffer) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

#if BUILD_OPENCL_12
    if (SrcImage == gcvNULL ||
        SrcImage->objectType != clvOBJECT_MEM ||
        (SrcImage->type != CL_MEM_OBJECT_IMAGE2D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE3D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER) ||
         (SrcImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER && SrcImage->u.image.buffer == DstBuffer))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010274: (clEnqueueCopyImageToBuffer) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#else
    if (SrcImage == gcvNULL ||
        SrcImage->objectType != clvOBJECT_MEM ||
        (SrcImage->type != CL_MEM_OBJECT_IMAGE2D &&
         SrcImage->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010114: (clEnqueueCopyImageToBuffer) invalid SrcImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif

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

#if BUILD_OPENCL_12
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
#endif

    /* Check if region is outside of SrcImage */
#if BUILD_OPENCL_12
    switch (SrcImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010277: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010278: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010279: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010280: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
            SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
            SrcOrigin[2] + Region[2] > SrcImage->u.image.depth)
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
#else
    if (SrcOrigin[0] + Region[0] > SrcImage->u.image.width ||
        SrcOrigin[1] + Region[1] > SrcImage->u.image.height ||
        SrcOrigin[2] + Region[2] > SrcImage->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010122: (clEnqueueCopyImageToBuffer) (SrcOrigin + Region) is outside of SrcImage's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif

    /* Check if region is outside of DstBuffer */
    if (DstBuffer->u.buffer.size <
        DstOffset + (Region[0] * Region[1] * Region[2] * SrcImage->u.image.elementSize))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010123: (clEnqueueCopyImageToBuffer) lastbyte is outside of DstBuffer's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010124: (clEnqueueCopyImageToBuffer) Run out of memory.\n");
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

#if BUILD_OPENCL_12
    if (DstImage == gcvNULL ||
        DstImage->objectType != clvOBJECT_MEM ||
        (DstImage->type != CL_MEM_OBJECT_IMAGE2D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE3D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE2D_ARRAY &&
         DstImage->type != CL_MEM_OBJECT_IMAGE1D_BUFFER) ||
         (DstImage->type == CL_MEM_OBJECT_IMAGE1D_BUFFER && DstImage->u.image.buffer == SrcBuffer))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010283: (clEnqueueCopyBufferToImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#else
    if (DstImage == gcvNULL ||
        DstImage->objectType != clvOBJECT_MEM ||
        (DstImage->type != CL_MEM_OBJECT_IMAGE2D &&
         DstImage->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010127: (clEnqueueCopyBufferToImage) invalid DstImage.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif

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

#if BUILD_OPENCL_12
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
#endif

    /* Check if region is outside of DstImage */
#if BUILD_OPENCL_12
    switch (DstImage->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010286: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010287: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                 "OCL-010288: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010289: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
            DstOrigin[1] + Region[1] > DstImage->u.image.height ||
            DstOrigin[2] + Region[2] > DstImage->u.image.depth)
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
#else
    if (DstOrigin[0] + Region[0] > DstImage->u.image.width ||
        DstOrigin[1] + Region[1] > DstImage->u.image.height ||
        DstOrigin[2] + Region[2] > DstImage->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010134: (clEnqueueCopyBufferToImage) (DstOrigin + Region) is outside of DstImage's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif


    /* Check if region is outside of SrcBuffer */
    if (SrcBuffer->u.buffer.size <
        SrcOffset + (Region[0] * Region[1] * Region[2] * DstImage->u.image.elementSize))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010135: (clEnqueueCopyBufferToImage) last byte of source is out of bounds.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010136: (clEnqueueCopyBufferToImage) Run out of memory.\n");
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

#if BUILD_OPENCL_12
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
#endif

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
            "OCL-010144: (clEnqueueMapBuffer) invalid MapFlags (0x%x).\n",
            MapFlags);
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    gcoCL_SetHardware();
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

    mappedPtr = gcmINT2PTR((gcmPTR2INT(Buffer->u.buffer.logical) + Offset));
    mapBuffer->mappedPtr = mappedPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingMap),
               CL_OUT_OF_HOST_MEMORY);

    if (ErrCodeRet)
    {
        *ErrCodeRet = CL_SUCCESS;
    }

    gcmFOOTER_ARG("%d Command=0x%x mappedPtr=0x%x",
                  CL_SUCCESS, command, mappedPtr);
#if BUILD_OPENCL_12
    if ((Buffer->flags & CL_MEM_USE_HOST_PTR) &&
        (Buffer->host != gcvNULL))
    {
        mappedPtr = gcmINT2PTR((gcmPTR2INT(Buffer->host) + Offset));
    }
#endif

    return mappedPtr;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010145: (clEnqueueMapBuffer) Run out of memory.\n");
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
#if BUILD_OPENCL_12
    size_t              slicePitch;
    size_t              hostOffset = 0;
#endif

    gcmHEADER_ARG("CommandQueue=0x%x Image=0x%x BlockingMap=%u "
                  "MapFlags=0x%x Origin=%u Region=0x%x",
                  CommandQueue, Image, BlockingMap, MapFlags, Origin, Region);

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010146: (clEnqueueMapImage) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }
#if BUILD_OPENCL_12
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

#else
    if (Image == gcvNULL ||
        Image->objectType != clvOBJECT_MEM ||
        (Image->type != CL_MEM_OBJECT_IMAGE2D &&
         Image->type != CL_MEM_OBJECT_IMAGE3D))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010147: (clEnqueueMapImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
    }
#endif

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

#if BUILD_OPENCL_12
    switch (Image->type)
    {
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        if (Origin[0] + Region[0] > Image->u.image.width)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.arraySize)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height)
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
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.arraySize)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010301: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    case CL_MEM_OBJECT_IMAGE3D:
        if (Origin[0] + Region[0] > Image->u.image.width ||
            Origin[1] + Region[1] > Image->u.image.height ||
            Origin[2] + Region[2] > Image->u.image.depth)
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010302: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
            clmRETURN_ERROR(CL_INVALID_VALUE);
        }
        break;
    default:
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010303: (clEnqueueMapImage) invalid Image.\n");
        clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);

    }
#else

    if (Region[0] == 0 || Region[1] == 0 || Region[2] == 0)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010152: (clEnqueueMapImage) One of Region's dimension size is 0.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (Image->type == CL_MEM_OBJECT_IMAGE2D &&
        (Origin[2] != 0 || Region[2] != 1))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010153: (clEnqueueMapImage) Image is 2D, but Origin[2] is not 0 or Region[2] is not 1.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    /* Check if region is outside of Image */
    if (Origin[0] + Region[0] > Image->u.image.width ||
        Origin[1] + Region[1] > Image->u.image.height ||
        Origin[2] + Region[2] > Image->u.image.depth)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010154: (clEnqueueMapImage) (Origina + Region) is outside of Image's boundary.\n");
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }
#endif

    /* Check if MapFlags is valid */
    if (MapFlags & ~((cl_map_flags)(CL_MAP_READ|CL_MAP_WRITE)))
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010155: (clEnqueueMapImage) invalid MapFlags (0x%x).\n",
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

    gcoCL_SetHardware();
    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);
    clfONERROR(clfRetainCommand(command));
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
#if BUILD_OPENCL_12
    slicePitch                      = Image->u.image.textureSlicePitch;
    mappedPtr                       = (gctUINT8_PTR) Image->u.image.textureLogical + Origin[2]*slicePitch +
                                                     Origin[1] * stride +
                                                     Origin[0] * elementSize;

    if ((Image->flags & CL_MEM_USE_HOST_PTR) &&
        (Image->host != gcvNULL))
    {
        hostOffset                      = Origin[2] * Image->u.image.slicePitch +
                                          Origin[1] * Image->u.image.rowPitch +
                                          Origin[0] * elementSize;
        mappedPtr = gcmINT2PTR(gcmPTR2INT(Image->host) + hostOffset);
        stride = Image->u.image.rowPitch;
        slicePitch = Image->u.image.slicePitch;
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
#else
    mappedPtr                       = (gctUINT8_PTR) Image->u.image.textureLogical +
                                                     Origin[1] * stride +
                                                     Origin[0] * elementSize;
    mapImage->imageSlicePitch       = 0;
    mapImage->imageRowPitch         = stride;
    if (ImageRowPitch)
    {
        *ImageRowPitch = stride;
    }
    if (ImageSlicePitch)
    {
        *ImageSlicePitch = 0;
    }
#endif
    mapImage->mappedPtr             = mappedPtr;

    clmONERROR(clfSubmitCommand(CommandQueue, command, BlockingMap),
               CL_OUT_OF_HOST_MEMORY);

    if (ErrCodeRet) {
        *ErrCodeRet = CL_SUCCESS;
    }

    clfONERROR(clfReleaseCommand(command));

    gcmFOOTER_ARG("%d Command=0x%x mappedPtr=0x%x",
                  CL_SUCCESS, command,mappedPtr);
    return mappedPtr;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010158: (clEnqueueMapImage) Run out of memory.\n");
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010164: (clEnqueueUnmapMemObject) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12
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
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x MemObjects=0x%x Flags=%u",
                  CommandQueue, MemObjects, Flags);

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
        gctUINT i = 0;

        for (i = 0; i < NumMemObjects; i++)
        {
            if (MemObjects[i] == gcvNULL ||
                MemObjects[i]->objectType != clvOBJECT_MEM )
            {
                clmRETURN_ERROR(CL_INVALID_MEM_OBJECT);
            }

            if (CommandQueue->context != MemObjects[i]->context)
            {
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }

    if (Flags & ~((cl_mem_migration_flags)(CL_MIGRATE_MEM_OBJECT_HOST|CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        clmRETURN_ERROR(CL_INVALID_VALUE);
    }

    if (EventWaitList == gcvNULL && NumEventsInWaitList > 0)
    {
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
                clmRETURN_ERROR(CL_INVALID_CONTEXT);
            }
        }
    }
    gcmFOOTER_ARG("%d", CL_SUCCESS);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010304: (clEnqueueMigrateMemObjects) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

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
/* To check the vectory size of a type (vec1/2/3/4, 1 means scalar). */
static
gctUINT gcGetInstSrcComponentCount(
    gcSHADER    Shader,
    gctUINT     Type,
    gctUINT16   Index,
    gctUINT     Swizzle
)
{
    gctUINT count = 0;
    gcSHADER_TYPE   shaderType;
    gctUINT32       arrayLen = 0;

    switch (Type)
    {
    case gcSL_TEMP:
        {
            /* For a temp source, always 4. */
            return 4;
        }
        break;

    case gcSL_UNIFORM:
        {
            gcUNIFORM   uniform;
            gcSHADER_GetUniform(Shader, Index, &uniform);
            gcUNIFORM_GetType(uniform, &shaderType, &arrayLen);
        }
        break;

    case gcSL_ATTRIBUTE:
        {
            gcATTRIBUTE attrib;
            gcSHADER_GetAttribute(Shader, Index, &attrib);
            gcATTRIBUTE_GetType(Shader, attrib, &shaderType, &arrayLen);
        }
        break;

    case gcSL_CONSTANT:
        {
            /* Per Shuxi, constant is always scalar? */
            return 1;
        }
        break;

    default:
        gcmASSERT(0);
        return 0;
        break;
    }

    if ((shaderType >= gcSHADER_INT64_X1) &&
        (shaderType <= gcSHADER_INT64_X4))
    {
        count = (gctUINT)(shaderType - gcSHADER_INT64_X1);
    }
    else
    if ((shaderType >= gcSHADER_UINT64_X1) &&
        (shaderType <= gcSHADER_UINT64_X4))
    {
        count = (gctUINT)(shaderType - gcSHADER_UINT64_X1);
    }
    else
    {
        count = 0;
        gcmASSERT(0);   /* Only works for long/ulong. */
    }
    count *= arrayLen;

    return count;
}
#endif

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

    gcmHEADER_ARG("CommandQueue=0x%x Kernel=0x%x "
                  "GlobalWorkOffset=0x%x GlobalWorkSize=0x%x GlobalWorkSize=0x%x",
                  CommandQueue, Kernel, GlobalWorkOffset, GlobalWorkSize, GlobalWorkSize);

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
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    if (GlobalWorkSize == gcvNULL)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010172: (clEnqueueNDRangeKernel) GlobalWorkSize is NULL.\n");
        clmRETURN_ERROR(CL_INVALID_EVENT_WAIT_LIST);
    }

    /* convert 1D -> 2D */
    if((WorkDim == 1) &&
       (GlobalWorkSize[0] > CommandQueue->device->deviceInfo.maxGlobalWorkSize))
    {
        size_t  fakeGlobalWorkOffset[2] = {0};
        size_t  fakeGlobalWorkSize[2] = {0};
        size_t  fakeLocalWorkSize[2] = {0};
        size_t  size0, size1;
        gctSIZE_T attribCount;
        gcSHADER kernelBinary = (gcSHADER)Kernel->states.binary;
        gctBOOL needShaderPatch = gcvFALSE;
        gctBOOL  matched = gcvFALSE, patchRealGlobalWorkSize = gcvFALSE;
        gctUINT  realGlobalWorkSize = GlobalWorkSize[0];
        WorkDim = 2;

        if (LocalWorkSize && (LocalWorkSize[0] != 0))
        {
            size1 = LocalWorkSize[0];
        }
        else
        {
            size1 = 16;
        }

        do
        {
            size0 = GlobalWorkSize[0] / size1;

            if ((GlobalWorkSize[0] % size1) == 0)
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

        GlobalWorkSize = fakeGlobalWorkSize;

        if (GlobalWorkOffset != gcvNULL)
        {
            fakeGlobalWorkOffset[0] =  GlobalWorkOffset[0] % fakeGlobalWorkSize[0];
            fakeGlobalWorkOffset[1] =  GlobalWorkOffset[0] / fakeGlobalWorkSize[0];

            if (fakeGlobalWorkOffset[1] > fakeGlobalWorkSize[1])
            {
                clmRETURN_ERROR(CL_INVALID_GLOBAL_OFFSET);
            }

            GlobalWorkOffset = fakeGlobalWorkOffset;
        }

        if (LocalWorkSize != gcvNULL)
        {
            fakeLocalWorkSize[0] = LocalWorkSize[0];
            fakeLocalWorkSize[1] = 1;
            LocalWorkSize = fakeLocalWorkSize;
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
                        patchRealGlobalWorkSize, &patchDirective),
                       CL_INVALID_GLOBAL_WORK_SIZE);
        }
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
                "OCL-010174: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) over hardware limit %d.\n",
                i, GlobalWorkSize[i], CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_WORK_SIZE);
        }

        if (GlobalWorkOffset != gcvNULL &&
            (GlobalWorkSize[i] + GlobalWorkOffset[i] >
                CommandQueue->device->deviceInfo.maxGlobalWorkSize))
        {
            gcmUSER_DEBUG_ERROR_MSG(
                "OCL-010175: (clEnqueueNDRangeKernel) GlobalWorkSize[%d] (%d) + GlobalWorkOffset[%d] (%d) over hardware limit %d.\n",
                i, GlobalWorkSize[i], i, GlobalWorkOffset[i],
                CommandQueue->device->deviceInfo.maxGlobalWorkSize);
            clmRETURN_ERROR(CL_INVALID_GLOBAL_OFFSET);
        }

        if (LocalWorkSize != gcvNULL)
        {
            if (LocalWorkSize[i] > CommandQueue->device->deviceInfo.maxWorkItemSizes[i])
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010176: (clEnqueueNDRangeKernel) LocalWorkSize[i] (%d) is over maxWorkItemSize (%d).\n",
                    i, LocalWorkSize[i], CommandQueue->device->deviceInfo.maxWorkItemSizes[i]);
                clmRETURN_ERROR(CL_INVALID_WORK_ITEM_SIZE);
            }

            if (LocalWorkSize[i] == 0)
            {
                gcmUSER_DEBUG_ERROR_MSG(
                    "OCL-010177: (clEnqueueNDRangeKernel) LocalWorkSize[i] is 0.\n",
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

    gcoCL_SetHardware();
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

    /* Create the release signal for the deferred command release. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &command->releaseSignal),
               CL_OUT_OF_HOST_MEMORY);

    NDRangeKernel                       = &command->u.NDRangeKernel;
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

    if (Kernel->patchNeeded)
    {
        gcSHADER            kernelBinary = (gcSHADER) Kernel->states.binary;
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

        {
#if BUILD_OPENCL_FP
            gctUINT uniformIndex[128] = {(gctUINT) ~0};
#else
            gctUINT uniformIndex[32] = {(gctUINT) ~0};
#endif
            gctUINT count = 0;
            gctUINT i;

#if _SUPPORT_LONG_ULONG_DATA_TYPE
            /* Check for LongULong opeartions for patch. */
            for (i = 0; i < GetShaderCodeCount(kernelBinary); i++)
            {
                gcSL_INSTRUCTION inst   = GetShaderInstruction(kernelBinary, i);

                if (gcIs64Inst(inst))
                {
                    if (gcNeedRecomile64(inst))
                    {
                        clfCreateLongULongDirective(inst,
                                                    i,
                                                    &patchDirective);
                    }
                }
            }
#endif

            for (i = 0; i < GetShaderCodeCount(kernelBinary); i++)
            {
                gcSL_INSTRUCTION code   = GetShaderInstruction(kernelBinary, i);
                gcSL_OPCODE      opcode = gcmSL_OPCODE_GET(GetInstOpcode(code), Opcode);
                if (opcode == gcSL_IMAGE_WR)
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
#if BUILD_OPENCL_12
                if (type == gcSHADER_IMAGE_2D ||
                    type == gcSHADER_IMAGE_3D ||
                    type == gcSHADER_IMAGE_1D ||
                    type == gcSHADER_IMAGE_1D_ARRAY ||
                    type == gcSHADER_IMAGE_1D_BUFFER ||
                    type == gcSHADER_IMAGE_2D_ARRAY
                    )
#else
                if (type == gcSHADER_IMAGE_2D ||
                    type == gcSHADER_IMAGE_3D)
#endif
                {
                    clsMem_PTR              image;
                    clsImageHeader_PTR      imageHeader;

                    /* Check if recompilation is needed. */
                    /* Get image object. */
                    image = *(clsMem_PTR *) arg->data;
                    gcmASSERT(image->objectType == clvOBJECT_MEM);
                    gcmASSERT(image->type != CL_MEM_OBJECT_BUFFER);

                    /* Get image header. */
                    imageHeader = (clsImageHeader_PTR) image->u.image.logical;

                    /* Create patch directive. */
                    clmONERROR(clfCreateWriteImageDirective(imageHeader,
                                                            (gctUINT)GetUniformIndex(arg->uniform),
                                                            imageHeader->channelDataType,
                                                            imageHeader->channelOrder,
                                                            imageHeader->tiling,
                                                            &patchDirective),
                               CL_OUT_OF_HOST_MEMORY);
                }
            }
        }

        /* Patch for read_image funtions. */
        if (GetKFunctionISamplerCount(kernelFunction) > 0)
        {
#if BUILD_OPENCL_FP
            gctUINT uniformIndex[128];
#else
            gctUINT uniformIndex[32];
#endif
            gctUINT count = 0;
            gctUINT i;
#if _USE_HW_TEXLD_
            gctUINT vsSamplers = 0, psSamplers = 0;
            gctINT maxSampler = 0, sampler = 0;
            gcSHADER_KIND shadeType;
#endif

            for (i = 0; i < GetKFunctionISamplerCount(kernelFunction); i++)
            {
                uniformIndex[i] = (gctUINT) ~0;
            }

            for (i = 0; i < GetShaderCodeCount(kernelBinary); i++)
            {
                gcSL_INSTRUCTION code   = GetShaderInstruction(kernelBinary, i);
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
                        if (count == GetKFunctionISamplerCount(kernelFunction))
                        {
                            break;
                        }
                    }
                }
            }

#if _USE_HW_TEXLD_
            gcmONERROR(
                gcoHAL_QuerySamplerBase(gcvNULL,
                                        &vsSamplers,
                                        gcvNULL,
                                        &psSamplers,
                                        gcvNULL));

            /* Determine starting sampler index. */
            shadeType = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_CL_PS_WALKER)
                ? GetShaderType(kernelBinary) : gcSHADER_TYPE_VERTEX;

            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SAMPLER_BASE_OFFSET))
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
#endif

            for (i = 0; i < GetKFunctionISamplerCount(kernelFunction); i++)
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
#if _USE_HW_TEXLD_
                    cleSAMPLER              normalizedMode;
                    cleSAMPLER              filterMode;
#endif
                    gctUINT                 channelDataType;
                    gctUINT                 channelOrder;

                    /* Check if recompilation is needed. */

                    /* Find the image sampler. */
                    gcmASSERT(GetKFunctionISamplers(kernelFunction) &&
                              GetKFunctionISamplerCount(kernelFunction) > GetUniformImageSamplerIndex(arg->uniform));
                    imageSampler = GetKFunctionISamplers(kernelFunction) + GetUniformImageSamplerIndex(arg->uniform);

                    /* Get image object. */
                    image = *(clsMem_PTR *) NDRangeKernel->args[GetImageSamplerImageNum(imageSampler)].data;
                    gcmASSERT(image->objectType == clvOBJECT_MEM);
                    gcmASSERT(image->type != CL_MEM_OBJECT_BUFFER);

                    /* Get image header. */
                    imageHeader = (clsImageHeader_PTR) image->u.image.logical;
                    channelDataType = imageHeader->channelDataType;
                    channelOrder = imageHeader->channelOrder;

                    /* Get sampler value. */
                    if (GetImageSamplerIsConstantSamplerType(imageSampler))
                    {
                        samplerValue = GetImageSamplerType(imageSampler);
                    }
                    else
                    {
                        samplerValue = *((gctUINT *) NDRangeKernel->args[GetImageSamplerType(imageSampler)].data);
                    }
#if _USE_HW_TEXLD_
                    normalizedMode = samplerValue & CLK_NORMALIZED_COORDS_TRUE;
                    filterMode  = samplerValue & 0xF00;

                    /* Check channel data type. */
                    /* TODO - gc4000 can handle more data types. */
                    if ((sampler < maxSampler) &&
                        (channelDataType == CL_UNORM_INT8))
                    {
                        arg->needImageSampler = gcvTRUE;
                        arg->image            = image;
                        arg->samplerValue     = samplerValue;

                        if ((image->type != CL_MEM_OBJECT_IMAGE3D)
#if BUILD_OPENCL_12
                            &&
                            (image->type != CL_MEM_OBJECT_IMAGE2D_ARRAY)
                            &&
                            (image->type != CL_MEM_OBJECT_IMAGE1D_ARRAY)
#endif
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
#endif

                    /* Use SW function to replace texld. */
                    arg->needImageSampler = gcvFALSE;

                    /* Create patch directive. */
                    clmONERROR(clfCreateReadImageDirective(imageHeader,
                                                           (gctUINT)GetUniformIndex(arg->uniform),
                                                           samplerValue,
                                                           channelDataType,
                                                           channelOrder,
                                                           imageHeader->tiling,
                                                           &patchDirective),
                               CL_OUT_OF_HOST_MEMORY);
                }
            }
        }
    }

    if (patchDirective)
    {
        /* Patch shader */
        clmONERROR(clfDynamicPatchKernel(Kernel,
                                         &NDRangeKernel->numArgs,
                                         &NDRangeKernel->args,
                                         patchDirective),
                   status);

        NDRangeKernel->states = Kernel->patchedStates;
    }
    else
    {
        NDRangeKernel->states = &Kernel->states;
    }

    /* Retain kernel. */
    clRetainKernel(Kernel);

#if cldSEQUENTIAL_EXECUTION
    clmONERROR(clfExecuteCommandNDRangeKernel(command),
               CL_OUT_OF_HOST_MEMORY);
#else
    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);
#endif

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010182: (clEnqueueNDRangeKernel) Run out of memory.\n");
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

    gcoCL_SetHardware();
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

    /* Create the release signal for the deferred command release. */
    clmONERROR(gcoCL_CreateSignal(gcvTRUE,
                                  &command->releaseSignal),
               CL_OUT_OF_HOST_MEMORY);

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

        task->states = Kernel->patchedStates;
    }
    else
    {
        task->states = &Kernel->states;
    }

    /* Retain kernel. */
    clRetainKernel(Kernel);

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010189: (clEnqueueTask) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010200: (clEnqueueNativeKernel) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#else
CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(
    cl_command_queue    CommandQueue,
    void                (*UserFunc)(void *),
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010200: (clEnqueueNativeKernel) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarker(
    cl_command_queue    CommandQueue,
    cl_event *          Event
    )
{
    clsCommand_PTR      command = gcvNULL;
    gctINT              status;

    gcmHEADER_ARG("CommandQueue=0x%x Event=0x%x", CommandQueue, Event);

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

    gcoCL_SetHardware();
    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    command->type       = clvCOMMAND_MARKER;
    command->handler    = &clfExecuteCommandMarker;
    command->outEvent   = Event;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010203: (clEnqueueMarker) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010307: (clEnqueueMarkerWithWaitList) Run out of memory.\n");
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010310: (clEnqueueBarrierWithWaitList) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010207: (clEnqueueWaitForEvents) Run out of memory.\n");
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

    if (CommandQueue == gcvNULL || CommandQueue->objectType != clvOBJECT_COMMAND_QUEUE)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010208: (clEnqueueBarrier) invalid CommandQueue.\n");
        clmRETURN_ERROR(CL_INVALID_COMMAND_QUEUE);
    }

    gcoCL_SetHardware();
    clmONERROR(clfAllocateCommand(CommandQueue, &command), CL_OUT_OF_HOST_MEMORY);

    command->type       = clvCOMMAND_BARRIER;
    command->handler    = &clfExecuteCommandBarrier;
    command->outEvent   = gcvNULL;

    clmONERROR(clfSubmitCommand(CommandQueue, command, gcvFALSE),
               CL_OUT_OF_HOST_MEMORY);

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010209: (clEnqueueBarrier) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

#if BUILD_OPENCL_12
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

    gcoCL_SetHardware();
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

    gcmFOOTER_ARG("%d Command=0x%x", CL_SUCCESS, command);
    return CL_SUCCESS;

OnError:
    if (status == CL_OUT_OF_HOST_MEMORY)
    {
        gcmUSER_DEBUG_ERROR_MSG(
            "OCL-010318: (clEnqueueFillBuffer) Run out of memory.\n");
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif
