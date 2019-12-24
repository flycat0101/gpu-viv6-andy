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

typedef struct _cl_kernel_instance * clsKernelInstance_PTR;

typedef struct _cl_kernel_instance
{
    gctUINT8_PTR            binary;

    /* States info. */
    gcsPROGRAM_STATE        programState;

    /* Need to add key for reusage. */
    clsPatchDirective_PTR   patchDirective;

    clsKernelInstance_PTR     next;
}
clsKernelInstance;

typedef struct
{
    clsPlatformId_PTR              platform;
    KERNEL_EXECUTABLE_PROFILE      kep;
    VSC_HW_PIPELINE_SHADERS_STATES hwStates;
    gctUINT                        hashKey;
}clsKernelVIRInstance;

typedef struct __clsVIRInstanceHashRec * clsVIRInstanceHashRec_PTR;
typedef struct _cls_VIRInstance_KeyState_ * clsVIRInstanceKey_PTR;

typedef struct _cls_VIRInstance_KeyState_
{
    gctUINT                      key;
    gctUINT                      year;
    clsKernelVIRInstance         * virInstance;
    clsVIRInstanceKey_PTR        nextInstanceKey;
}clsVIRInstanceKey;

/* Hash definition */
typedef struct __clsVIRInstanceHashRec
{
    clsVIRInstanceKey           **ppHashTable;
    /* How many objects of each entry */
    gctUINT                     *pEntryCounts;
    /* Hash table entry number must be power of 2 (32, 64, etc.), so that
       we can use a simple bitmask (entryNum-1) to get the hash entry */
    gctUINT                     tbEntryNum;
    /* Max objects count that each hash table entry can hold. For memory
       footprint consideration, we don't hope too many objects are hold in
       each entry. */
    gctUINT                      maxEntryObjs;
    gctUINT                      year;
}clsVIRInstanceHashRec;

typedef struct _cl_kernel
{
    clsIcdDispatch_PTR      dispatch;
    cleOBJECT_TYPE          objectType;
    gctUINT                 id;

    /* clGetKernelInfo */
    gctUINT                 kernelNumArgs;
    gctSTRING               name;
    gcsATOM_PTR             referenceCount;
    clsContext_PTR          context;
    clsProgram_PTR          program;
    /*!TO_DO, kernel attribute */

    /* clGetKernelWorkGroupInfo */
    /*!TO_DO, CL_KERNEL_GLOBAL_WORK_SIZE*/
    size_t                  maxWorkGroupSize;
    size_t                  compileWorkGroupSize[3];
    cl_ulong                localMemSize;
    size_t                  preferredWorkGroupSizeMultiple;
    cl_ulong                privateMemSize;

    /* clGetKernelArgInfo, currently, not only the source arg here,
    all other argument are here. */
    clsArgument_PTR         args;
    gctUINT                 numArgs;
    gctPOINTER              argMutex;

    gctSIZE_T               constantMemSize;
    gctCHAR *               constantMemBuffer;

    clsKernelInstance       masterInstance;
    clsKernelInstance_PTR   recompileInstance;
    gctBOOL                 patchNeeded;

    /* indicate if OCL patch is pathed already */
    gctBOOL                 isPatched;
    gctBOOL                 hasPrintf;

    /* srcArgs size must match kernelNumArgs */
    clsSrcArgument_PTR      srcArgs;
    clsKernelVIRInstance     * virMasterInstance;
    clsKernelVIRInstance     * virCurrentInstance;
    clsVIRInstanceHashRec_PTR virCacheTable;
    gctPOINTER              cacheMutex;

    SHADER_HANDLE           shaderHandle;

    gctPOINTER              linkedDebugInfo;

    /* indicate if the kernel need re-compile when thread remapping */
    gctBOOL                 recompileThreadRemap;
}
clsKernel;

#define ARG_TYPE_NAME_LENTH 128

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
    gctUINT                 printThreadNum;
    gctUINT                 printBufferSizePerThread;
    gctCHAR                 typeName[ARG_TYPE_NAME_LENTH];
    cl_kernel_arg_address_qualifier addressQualifier;
    cl_kernel_arg_type_qualifier    typeQualifier;
    cl_kernel_arg_access_qualifier  accessQualifier;
}
clsArgument;

typedef struct _cl_src_argument
{
    size_t                  size;
    gctPOINTER              data;
    gctUINT                 argIndex;
    gctBOOL                 set;
    gctCHAR *               name;
    gctCHAR                 typeName[ARG_TYPE_NAME_LENTH];
    gctUINT                 type;
    gctBOOL                 isPointer;
    gctBOOL                 isSampler;
    gctBOOL                 isImage;
    gctBOOL                 isDuplicate;
    gctBOOL                 isLocal;
    gctBOOL                 isMemAlloc;
    cl_kernel_arg_address_qualifier addressQualifier;
    cl_kernel_arg_type_qualifier    typeQualifier;
    cl_kernel_arg_access_qualifier  accessQualifier;
}clsSrcArgument;

typedef struct _cl_mem_alloc_info
{
    gctUINT                 allocatedSize;
    gctUINT32               physical; /* GPU virutal address */
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
clfExecuteCommandNDRangeVIRKernel(
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
clfBuildKernelArgs(
    clsKernel_PTR       Kernel
    );

gctINT
clfDuplicateVIRKernelArgs(
    clsKernel_PTR       Kernel,
    clsSrcArgument_PTR *   Arguments
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

gctINT
clfFreeVIRKernelArgs(
    gctUINT            NumArgs,
    clsSrcArgument_PTR Args,
    gctUINT            localKernelArgSize, /* local kernel arg should be related with each NDRange only */
    gctBOOL            FreePrivateKernelArg
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


gctINT
clfReleaseKernel(
    cl_kernel   Kernel
    );

gctINT
clfRetainKernel(
    cl_kernel    Kernel
    );

gceSTATUS
clfRecompileVIRKernel(
    cl_kernel Kernel,
    gctUINT workGroupSize
    );

clsVIRInstanceKey *
clfAddInstanceKeyToHashTable(
    clsVIRInstanceHashRec_PTR pHash,
    clsKernelVIRInstance * pVIRInstance,
    gctUINT key
    );

#ifdef __cplusplus
}
#endif

#endif  /* __gc_cl_kernel_h_ */
