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


#ifndef __gc_cl_kernel_h_
#define __gc_cl_kernel_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
|************************* Kernel Object Definition ***************************|
\******************************************************************************/

typedef struct _clsPatchGlobalWorkSize
{
    gcUNIFORM  globalWidth;
    gcUNIFORM  groupWidth;
    gctBOOL    patchRealGlobalWorkSize;
    gctUINT    realGlobalWorkSize;
}
clsPatchGlobalWorkSize;

typedef struct _clsPatchReadImage
{
    clsImageHeader_PTR      imageHeader;
    gctUINT                 samplerNum;
    gctUINT                 samplerValue;
    gctUINT                 channelDataType;
    gctUINT                 channelOrder;
    gceTILING               tiling;
    gctUINT                 imageDataIndex;
    gctUINT                 imageSizeIndex;
}
clsPatchReadImage;

typedef struct _clsPatchWriteImage
{
    clsImageHeader_PTR      imageHeader;
    gctUINT                 imageNum;
    gctUINT                 channelDataType;
    gctUINT                 channelOrder;
    gceTILING               tiling;
    gctUINT                 imageDataIndex;
    gctUINT                 imageSizeIndex;
}
clsPatchWriteImage;

typedef struct _clsPatchLongULong
{
    gcSL_INSTRUCTION        instruction;
    gctUINT                 instructionIndex;
    gctUINT                 channelCount;   /* How many channel enabled in target. */
    gctUINT                 channelCountIndex;  /* The corresponding uniform index. */
}
clsPatchLongULong;

typedef struct _clsRecompileDirective * clsPatchDirective_PTR;
typedef struct _clsRecompileDirective
{
    enum gceRecompileKind   kind;
    union
    {
        clsPatchGlobalWorkSize *    globalWorkSize;
        clsPatchReadImage *         readImage;
        clsPatchWriteImage *        writeImage;
        clsPatchLongULong       *   longULong;
    } patchValue;
    clsPatchDirective_PTR   next;  /* pointer to next patch directive */
}
clsPatchDirective;

typedef struct _cl_kernel_states * clsKernelStates_PTR;

typedef struct _cl_kernel_states
{
    gctUINT8_PTR            binary;
    gctUINT                 numArgs;

    /* States info. */
    gctSIZE_T               stateBufferSize;
    gctPOINTER              stateBuffer;
    gcsHINT_PTR             hints;

    /* Need to add key for reusage. */
    clsPatchDirective_PTR   patchDirective;

    clsKernelStates_PTR     next;
}
clsKernelStates;

typedef struct _cl_kernel
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;
    gcsATOM_PTR             referenceCount;

    clsProgram_PTR          program;
    clsContext_PTR          context;

    gctSTRING               name;
    size_t                  maxWorkGroupSize;
    size_t                  compileWorkGroupSize[3];
    size_t                  preferredWorkGroupSizeMultiple;
    cl_ulong                localMemSize;
    cl_ulong                privateMemSize;
    gctSIZE_T               constantMemSize;
    gctCHAR *               constantMemBuffer;

    gctUINT                 numArgs;
    clsArgument_PTR         args;

    clsKernelStates         states;
    clsKernelStates_PTR     patchedStates;
    gctBOOL                 patchNeeded;

    gctUINT                 attributeCount;
    gctUINT                 tempCount;

    gctBOOL                 useLocalRegisters;
    gctBOOL                 alreadyUsedLocalSource;
    gctINT                  localRegLoadedLastCycle;

    gctPOINTER              argMutex;

    /* Constant range that define constants starting from 0. */
    gctINT                  constRegUsedAsSrc;
    gctINT                  firstProgression;
    gctINT                  lastProgression;

    /* Constant range that define LOAD/STORE base addresses. */
    gctINT                  firstBase;
    gctINT                  lastBase;

    gctINT                  idivImodMov;
    /* indicate if OCL patch is pathed already */
    gctBOOL                 isPatched;
}
clsKernel;

typedef struct _cl_argument
{
    gcUNIFORM               uniform;
    size_t                  size;
    gctPOINTER              data;
    gctBOOL                 set;
    gctBOOL                 isMemAlloc;
    gctBOOL                 isPointer;
    gctBOOL                 isMemObj;
    gctBOOL                 needImageSampler;
    clsMem_PTR              image;
    gctUINT                 samplerValue;
}
clsArgument;

typedef struct _cl_mem_alloc_info
{
    gctUINT                 allocatedSize;
    gctPHYS_ADDR            physical;
    gctPOINTER              logical;
    gcsSURF_NODE_PTR        node;
    gctPOINTER              data;
}
clsMemAllocInfo;

typedef clsMemAllocInfo * clsMemAllocInfo_PTR;


/*****************************************************************************\
|*                         Supporting functions                              *|
\*****************************************************************************/

gctINT
clfExecuteCommandNDRangeKernel(
    clsCommand_PTR      Command
    );

gctINT
clfExecuteCommandTask(
    clsCommand_PTR      Command
    );

gctINT
clfExecuteCommandNativeKernel(
    clsCommand_PTR      Command
    );

gctINT
clfAllocateKernelArgs(
    clsKernel_PTR       Kernel
    );

gctINT
clfDuplicateKernelArgs(
    clsKernel_PTR       Kernel,
    clsArgument_PTR *   Arguments
    );

gctINT
clfReallocateKernelArgs(
    gctUINT             OrgNumArgs,
    gctUINT             NewNumArgs,
    clsArgument_PTR *   Args
    );

gctINT
clfFreeKernelArgs(
    gctUINT             NumArgs,
    clsArgument_PTR     Args,
    gctBOOL             FreeAllocData
    );

clsArgument_PTR
clfGetKernelArg(
    clsKernel_PTR       Kernel,
    gctUINT             Index,
    gctBOOL *           isLocal,
    gctBOOL *           isPrivate,
    gctBOOL *           isSampler
    );

gceSTATUS
clfDestroyPatchDirective(
    IN OUT clsPatchDirective ** PatchDirectivePtr
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_kernel_h_ */
