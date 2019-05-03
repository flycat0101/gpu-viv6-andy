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


#ifndef __gc_spirv_h_
#define __gc_spirv_h_

#define VIV_SPIRV_CONVERTER         1         /* flag to specify headers below are included for spirv converter */
#define VIV_TREAT_8BIT_16BIT_PUSH_CONSTANT_AS_BUFFER    1
#include "utils/gc_vsc_utils_base.h"
#include "utils/gc_vsc_err.h"
#include "utils/gc_vsc_utils_list.h"
#include "utils/gc_vsc_utils_mm.h"
#include "utils/gc_vsc_utils_bv.h"
#include "utils/gc_vsc_utils_hash.h"
#include "utils/gc_vsc_utils_table.h"
#include "utils/gc_vsc_utils_io.h"
#include "old_impl/gc_vsc_old_gcsl.h"
#include "vir/passmanager/gc_vsc_options.h"
#include "vir/ir/gc_vsc_vir_ir.h"
#include "gc_hal_user_precomp.h"
#include <vulkan/spirv.h>
#include <vulkan/vulkan.h>
#include "gc_spirv_mempool.h"
#include "gc_spirv_types.h"
#include "gc_spirv_disassemble.h"

#define SPV_MAGIC_NUM               0
#define SPV_GENERATOR               2
#define SPV_BOUND_ID                3
#define SPV_SCHEMA                  4
#define SPV_INSTRUCTION_START       5

#define SPV_DESCRIPTOR_PAGESIZE     30
#define SPV_GENERAL_PAGESIZE        20

#define SPV_INVALID_LABEL           0xFFFFFFFF

#define SPV_VIR_NAME_SIZE           256
#define SPV_MAX_INST_EMIT_PER_SPV   4
#define SPV_MAX_INVALID_ENTRY_COUNT 16

#define SPV_DEFAULT_PARAM_NAME      "param"
#define SPV_DEFAULT_PARAM_LENGTH    5

#define SPV_STRUCT_FIELD_NUM        16

#define SPV_SKIP_NAME_CHECK         1

typedef struct
{
    VIR_Instruction      *inst;
    VIR_Operand          *operand;
} SpvUnhandleInfo;

typedef struct
{
    SpvUnhandleInfo      *infoList;
    gctUINT               listCount;
    gctUINT               listSize;
}SpvUnhandleOperands;

typedef struct
{
    SpvUnhandleOperands unHandledOperands;
    /* So far for SpvSymDescriptor and SpvConstDescriptor*/
    VIR_SymId virSymId;
}SpvDescriptorHeader;

typedef struct {
    gctBOOL hasName;
    VIR_NameId field;
    gctUINT member;
} SpvStructMembers;

typedef struct _SpvTypeDescriptor
{
    SpvDescriptorHeader descriptorHeader;

    struct {
        gctUINT isFloat                 : 1;
        gctUINT isInteger               : 1;
        gctUINT isSignedInteger         : 1;
        gctUINT isUnsignedInteger       : 1;
        gctUINT isBoolean               : 1;
        gctUINT isScalar                : 1;
        gctUINT isVector                : 1;
        gctUINT isMatrix                : 1;
        gctUINT isSampler               : 1;
        gctUINT isImage                 : 1;
        gctUINT isVoid                  : 1;
        gctUINT isPointer               : 1;
        gctUINT isArray                 : 1;
        gctUINT hasUnSizedArray         : 1;
        gctUINT isStruct                : 1;
        gctUINT isFunction              : 1;
        gctUINT isMeta                  : 1;
        gctUINT isBlock                 : 1;
        gctUINT has8BitType             : 1;
        gctUINT has16BitType            : 1;
        gctUINT reserved                : 12;
    }typeFlags;

    union
    {
        struct
        {
            gctUINT width;
            gctBOOL  sign;
        } integer;

        struct
        {
            gctUINT width;
        }floatf;

        /* vector size */
        struct
        {
            gctUINT compType;
            gctUINT num;
        }vector;

        struct
        {
            gctUINT colType;
            gctUINT colCount;
        }matrix;

        struct
        {
            gctUINT returnType;
            gctUINT *argType;
            gctUINT argNum;
        }func;

        struct
        {
            SpvStorageClass storageClass;
            gctUINT objType;
        }pointer;

        struct
        {
            gctUINT maxNumField;
            SpvStructMembers *fields;
        }st;

        struct
        {
            SpvId baseTypeId;
            gctUINT length;
            /* corresponding type for sampledImage. */
            VIR_TypeId sampledImageType;
        }array;

        struct
        {
            gctUINT sampledType;
            SpvDim dimension;
            gctUINT depth;
            gctUINT arrayed;
            gctUINT msaa;
            gctUINT sampled;
            SpvImageFormat format;
            SpvAccessQualifier qualifier;
            /* the vir type used for opTypeSampledImage. */
            VIR_TypeId sampledImageType;
        }image;
    }u;
}SpvTypeDescriptor;

typedef struct _SpvLinkageAttrib
{
    gctUINT stringName;
    SpvLinkageType type;
}SpvLinkageAttrib;

typedef enum _SPV_INTERPOLATION_QUALIFIER
{
    SPV_INTERPOLATION_NONE          = 0,
    SPV_INTERPOLATION_NOPERSPECTIVE = 1,
    SPV_INTERPOLATION_FLAT          = 2,
} SpvInterpolationQualifier;

typedef enum _SPV_AUXILIARY_QUALIFIER
{
    SPV_AUXILIARY_NONE              = 0,
    SPV_AUXILIARY_CENTROID          = 1,
    SPV_AUXILIARY_SAMPLE            = 2,
    SPV_AUXILIARY_PATCH             = 3,
} SpvAuxiliaryQualifier;

typedef enum _SPV_LAYOUT_QUALIFIER
{
    SPV_LAYOUT_NONE                 = 0x0000,
    SPV_LAYOUT_ROW_MAJOR            = 0x0001,
    SPV_LAYOUT_COL_MAJOR            = 0x0002,
    SPV_LAYOUT_GLSL_SHARED          = 0x0004,
    SPV_LAYOUT_GLSL_PACKED          = 0x0008,
    SPV_LAYOUT_C_PACKED             = 0x0010,
} SpvLayoutQualifier;

typedef enum _SPV_VARIANCE_QUALIFIER
{
    SPV_VARIANCE_NONE               = 0,
    SPV_VARIANCE_INVARIANT          = 1,
} SpvVarianceQualifier;

typedef enum _SPV_PRECISE_QUALIFIER
{
    SPV_PRECISE_DISABLE            = 0,
    SPV_PRECISE_ENABLE             = 1,
}SpvPreciseQualifier;

typedef struct _SpvConvDecorationData
{
    gctBOOL                         isRelaxedPrecision;
    gctBOOL                         isBlock;            /* For UBO and IOB. */
    gctBOOL                         isBufferBlock;      /* For SBO. */
    /* For those decorations need extra Operands. */
    gctINT                          specId;
    gctINT                          arrayStride;
    gctINT                          matrixStride;
    gctINT                          builtIn;
    gctINT                          stream;
    gctINT                          location;
    gctINT                          component;
    gctINT                          index;
    gctINT                          binding;
    gctINT                          descriptorSet;
    gctINT                          offset;
    gctINT                          xfbBuffer;
    gctINT                          xfbStride;
    SpvFunctionParameterAttribute   funcParamAttrib;
    SpvFPRoundingMode               fpRoundingMode;
    SpvFPFastMathModeMask           fpFastMathMode;
    SpvLinkageAttrib                linkageAttrib;
    gctINT                          inputAttachmentIndex;
    gctINT                          alignment;
    /* For those decorations have no extra operands. */
    SpvInterpolationQualifier       interpolationQualifier;
    SpvAuxiliaryQualifier           auxiliaryQualifier;
    SpvLayoutQualifier              layoutQualifier;
    SpvVarianceQualifier            varianceQualifier;
    SpvPreciseQualifier             preciseQualifier;
}SpvConvDecorationData;

#define SPV_DECORATION_Initialize(Decoration) \
    do \
    {  \
        (Decoration)->location              = SPV_DEFAULT_LOCATION;     \
        (Decoration)->component             = -1;                       \
        (Decoration)->binding               = -1;                       \
        (Decoration)->arrayStride           = -1;                       \
        (Decoration)->matrixStride          = -1;                       \
        (Decoration)->offset                = -1;                       \
        (Decoration)->alignment             = -1;                       \
        (Decoration)->inputAttachmentIndex  = -1;                       \
        (Decoration)->descriptorSet         = -1;                       \
        (Decoration)->builtIn               = -1;                       \
    } \
    while (gcvFALSE)

typedef struct _SpvCovDecorator
{
    gctUINT                         target;
    gctINT                          memberIndex; /* Only valid for MemberDecoration, for normal Decoration, the value is -1. */
    SpvConvDecorationData           decorationData;
    struct _SpvCovDecorator        *next;
}SpvCovDecorator;

typedef enum{
    SPV_ID_TYPE_UNKNOW = 0,
    SPV_ID_TYPE_SYMBOL,
    SPV_ID_TYPE_CONST,
    SPV_ID_TYPE_TYPE,
    SPV_ID_TYPE_COMPARE,
    SPV_ID_TYPE_FUNC_DEFINE,
    SPV_ID_TYPE_LABEL,
}SpvIDType;

typedef enum{
    SPV_PARA_NONE   = 0,
    SPV_PARA_IN     = 1,
    SPV_PARA_OUT    = 2,
    SPV_PARA_INOUT  = 3,
    SPV_PARA_CONST  = 4,
}SpvParameterStorage;

typedef enum{
    SpvLabelFunctionCall,
    SpvLabelPhi,
}SpvLabelType;

typedef enum{
    SPV_FRAGCOORD = 0,
    SPV_MAX_BUILTINVAR,
}SpvBuiltinVariable;

typedef struct
{
    SpvDescriptorHeader descriptorHeader;

    VIR_Instruction **virInst;
    gctUINT instSize;
    VIR_Operand **virDest;
    gctUINT destSize;
    gctUINT destCount;


    VIR_PhiOperand **phiOperands;
    gctUINT phiCount;
    gctUINT phiSize;

} SpvLabelDescriptor;

typedef struct
{
    SpvDescriptorHeader descriptorHeader;

    /* This is the base type ID, not pointer type. */
    SpvId spvBaseTypeId;

    /* This is the pointer type ID, if it is not a pointer, then it equals with the base type ID. */
    SpvId spvPointerTypeId;

    gctBOOL isFuncParam;
    VIR_Function *virFunc;
    gctBOOL isWorkGroup;

    /* Use it to check the function parameter:
    *  If this spvId has been stored before a function call, then it is a assignment for a in/inout paramter.
    *  If this spvId has been loaded before a function call, then it is a assignment for a const paramter.
    */
    gctBOOL usedStoreAsDest;
    gctBOOL usedLoadAsDest;
    gctBOOL isSampledImage;
    gctBOOL isPerVertex;
    gctBOOL isPerPatch;
    gctBOOL isPushConstUBO;

    /* For inputAttachment. */
    Spv_AttachmentFlag attachmentFlag;

    SpvStorageClass storageClass;

    SpvId image;
    SpvId sampler;

    /* for pointer offset, OpAccessChain:
    ** For a IB base variable:
    **     the blockOffset is array index of this IB, and the offset is the memory offset.
    ** For the rest variables:
    **     the blockOffset is alway zero, and the offset is the register offset.
    */
    SpvId srcSpvTypeId;
    struct
    {
        VIR_AC_OFFSET_INFO virAcOffsetInfo;
        VIR_SymId   uboArrayVirSymId[20];
        SpvId acDynamicIndexing;
    } offsetInfo;

    /* If this symbol is created by a accesschain, save the base spv symbol ID. */
    SpvId baseSpvSymId;

    SpvId paramToFunc;
}SpvSymDescriptor;

typedef struct
{
    gctUINT callerArgNum;
    SpvId * spvCallerArg;
    VIR_Instruction  *virCallerInst;
    VIR_Instruction  *paramInst;
    SpvId spvRet;
    VIR_Function * virFunction;
}SpvFunctionCallerDescriptor;

typedef struct
{
    SpvDescriptorHeader descriptorHeader;

    SpvId label;
    VIR_Label * virLabel;
    VIR_Function * virFunction;
    SpvFunctionControlMask funcControl;
    SpvId typeId;

    /* argument */
    gctUINT argNum;
    SpvId * spvArg;
    SpvParameterStorage * argStorage;

    /* caller */
    gctUINT callerNum;
    /* For all caller, we need,
        #1, mov resultSymbol back to caller destId,
        #2, for output/inoutput parameter, add mov instruction to move back
        #3, fix the caller label
    */
    SpvFunctionCallerDescriptor *caller;
    gctUINT callerSize;

    /* return symbol */
    VIR_Symbol * virRetSymbol;
}SpvFuncDescriptor;

typedef struct
{
    SpvDescriptorHeader descriptorHeader;

    VIR_ConstVal virConst;
    VIR_ConstId virConstId;
    SpvId spvTypeId;
    SpvId vecSpvId[16];
}SpvConstDescriptor;

typedef struct
{
    SpvDescriptorHeader descriptorHeader;

    VIR_ConditionOp virCond;
    SpvId op[2];
}SpvCondDescriptor;

typedef struct
{
    SpvOp opcode;
    SpvId resultId;
    SpvId resultTypeId;
    SpvId operandSize;
    SpvId operands[SPV_MAX_OPERAND_NUM];
}SpvInternalInstruction;

typedef struct {
    gctUINT inVert : 1;
    gctUINT inTesc : 1;
    gctUINT inTese : 1;
    gctUINT inGeom : 1;
    gctUINT inFrag : 1;
    gctUINT inComp : 1;
    gctUINT inKernel : 1;
} SpvInterfaceFlag;

typedef struct
{
    /* Type of this ID */
    SpvIDType idType;

    /* vir info of this Id, sym name or type name */
    VIR_NameId virNameId;
    VIR_TypeId virTypeId;

    /* if this ID has been initialzied before.*/
    gctBOOL initialized;

    /* Whether this is used to calculate the memory address. */
    gctBOOL isMemAddrCalc;

    union{
        SpvSymDescriptor sym;
        SpvConstDescriptor cst;
        SpvTypeDescriptor type;
        SpvLabelDescriptor label;
        SpvCondDescriptor cond;
        SpvFuncDescriptor func;
    }u;

    SpvInterfaceFlag interfaceFlag;

}SpvCovIDDescriptor;

typedef struct
{
    gctUINT entryID;
    SpvExecutionMode exeMode;
    gctUINT dimension;
    gctUINT extraOp[3]; /* max extra op is 3 now */
}SpvExeModeDescriptor;

typedef struct {
    SpvExecutionModel model;
    gctCHAR * entryName;
} SpvEntryInfo;

typedef struct {

    /* create a shared SBO for compute shader */
    VIR_TypeId                  sharedSboTypeId;
    VIR_SymId                   sharedSboSymId;
    VIR_NameId                  sharedSboNameId;

    /* build in invocation ID */
    VIR_SymId                   invocationIdSymId;

    /* group offset is invocationId * groupSize */
    VIR_NameId                  groupOffsetNameId;
    VIR_SymId                   groupOffsetSymId;

    /* save inst to set group size */
    VIR_Instruction             *offsetCalVirInst;

    gctUINT32                   groupSize;

    SpvId                       sboMembers[SPV_MAX_OPERAND_NUM];
    gctUINT32                   sboMemberOffset[SPV_MAX_OPERAND_NUM];
    gctUINT32                   memberCount;

    VIR_SymId                   curOffsetSymId;
} SpvWorkGroupInfo;

#define SPV_MAX_SCOPE_ID_NUM         2
#define SPV_MAX_MEM_SEMANTIC_ID_NUM  2
#define SPV_INTERNAL_ID_NUM          10

typedef struct SpvFuncCallInfo{
    gctUINT funcId;
    gctBOOL isEntry;

    gctUINT *calleeIds;
    gctUINT calleeCount;
    gctUINT calleeAllocated;

    gctUINT *varIds;
    gctUINT varCount;
    gctUINT varAllocated;
}SpvFuncCallInfo;

typedef struct SpvFuncCallTable{
    SpvMemPool * memPool;
    SpvFuncCallInfo ** funcs;
    gctUINT funcCount;
    gctUINT funcAllocated;
}SpvFuncCallTable;


struct _gcSPV
{
    gcsOBJECT                   object;
    gctUINT *                   src;
    SpvMemPool *                spvMemPool;

    /* VSC memory pool. */
    VSC_PRIMARY_MEM_POOL        pmp;

    /* Size in gctUINT of SPV binary */
    gctUINT                     size;
    gctUINT                     spvId;

    gctBOOL                     isInValidArea;
    Spv_SpecFlag                spvSpecFlag; /* internal usage flag */

    /* version, generator, bound */
    gctUINT                     version;
    gctUINT                     generator;
    gctUINT                     bound;

    /* Internal spv id list. */
    SpvId *                     internalIds;
    VSC_BIT_VECTOR              internalIdMask;
    gctUINT                     nextValidInternelIdIndex;

    /* Internal symbol pointer. */
    VIR_Symbol *                internalSym;

    struct{
        gctUINT SpvCapabilityMatrix                                :1;
        gctUINT SpvCapabilityShader                                :1;
        gctUINT SpvCapabilityGeometry                              :1;
        gctUINT SpvCapabilityTessellation                          :1;
        gctUINT SpvCapabilityAddresses                             :1;
        gctUINT SpvCapabilityLinkage                               :1;
        gctUINT SpvCapabilityKernel                                :1;
        gctUINT SpvCapabilityVector16                              :1;
        gctUINT SpvCapabilityFloat16Buffer                         :1;
        gctUINT SpvCapabilityFloat16                               :1;
        gctUINT SpvCapabilityFloat64                               :1;
        gctUINT SpvCapabilityInt64                                 :1;
        gctUINT SpvCapabilityInt64Atomics                          :1;
        gctUINT SpvCapabilityImageBasic                            :1;
        gctUINT SpvCapabilityImageReadWrite                        :1;
        gctUINT SpvCapabilityImageMipmap                           :1;
        gctUINT SpvCapabilityImageSRGBWrite                        :1;
        gctUINT SpvCapabilityPipes                                 :1;
        gctUINT SpvCapabilityGroups                                :1;
        gctUINT SpvCapabilityDeviceEnqueue                         :1;
        gctUINT SpvCapabilityLiteralSampler                        :1;
        gctUINT SpvCapabilityAtomicStorage                         :1;
        gctUINT SpvCapabilityInt16                                 :1;
        gctUINT SpvCapabilityTessellationPointSize                 :1;
        gctUINT SpvCapabilityGeometryPointSize                     :1;
        gctUINT SpvCapabilityImageGatherExtended                   :1;
        gctUINT SpvCapabilityStorageImageExtendedFormats           :1;
        gctUINT SpvCapabilityStorageImageMultisample               :1;
        gctUINT SpvCapabilityUniformBufferArrayDynamicIndexing     :1;
        gctUINT SpvCapabilitySampledImageArrayDynamicIndexing      :1;
        gctUINT SpvCapabilityStorageBufferArrayDynamicIndexing     :1;
        gctUINT SpvCapabilityStorageImageArrayDynamicIndexing      :1;
        gctUINT SpvCapabilityClipDistance                          :1;
        gctUINT SpvCapabilityCullDistance                          :1;
        gctUINT SpvCapabilityImageCubeArray                        :1;
        gctUINT SpvCapabilitySampleRateShading                     :1;
        gctUINT SpvCapabilityImageRect                             :1;
        gctUINT SpvCapabilitySampledRect                           :1;
        gctUINT SpvCapabilityGenericPointer                        :1;
        gctUINT SpvCapabilityInt8                                  :1;
        gctUINT SpvCapabilityInputAttachment                       :1;
        gctUINT SpvCapabilityInputTarget                           :1;
        gctUINT SpvCapabilitySparseResidency                       :1;
        gctUINT SpvCapabilityMinLod                                :1;
        gctUINT SpvCapabilitySampled1D                             :1;
        gctUINT SpvCapabilityImage1D                               :1;
        gctUINT SpvCapabilitySampledCubeArray                      :1;
        gctUINT SpvCapabilitySampledBuffer                         :1;
        gctUINT SpvCapabilityImageBuffer                           :1;
        gctUINT SpvCapabilityImageMSArray                          :1;
        gctUINT SpvCapabilityAdvancedFormats                       :1;
        gctUINT SpvCapabilityImageQuery                            :1;
        gctUINT SpvCapabilityDerivativeControl                     :1;
        gctUINT SpvCapabilityInterpolationFunction                 :1;
        gctUINT SpvCapabilityTransformFeedback                     :1;
        gctUINT SpvCapabilityGeometryStreams                       :1;
        gctUINT SpvCapabilityStorageImageReadWithoutFormat         :1;
        gctUINT SpvCapabilityStorageImageWriteWithoutFormat        :1;
        gctUINT SpvCapabilityMultiViewport                         :1;
    }capability;

    /* variable give basic discription of Shader */
    SpvSourceLanguage           srcLanguage;
    gctUINT                     srcLanguageVersion;
    gctCHAR *                   srcLanguageExternsion;
    gctCHAR *                   srcExtension;
    gctCHAR *                   srcExtInstImport;
    SpvAddressingModel          srcAddrMode;
    SpvMemoryModel              srcMemoryMode;
    gctUINT8                    shaderStage;

    SpvExeModeDescriptor *      exeModeDescriptor;
    gctUINT                     exeModeSize; /* total size */
    gctUINT                     exeModeCount; /* current indexing */

    /* If client version is set. */
    gctBOOL                     setClientVersion;

    /* variable used to analysis instructions */
    gctUINT                     shaderId;
    gctUINT                     nameId;
    gctUINT                     word;
    gctUINT                     nextInst;
    gctUINT                     numOperands;
    SpvId                       resultId;
    SpvId                       resultTypeId;
    SpvOp                       opCode;
    /* Some instruction have two scope, or to memSematic */
    SpvScope                    scope[SPV_MAX_SCOPE_ID_NUM];
    SpvMemorySemanticsMask      memSematic[SPV_MAX_MEM_SEMANTIC_ID_NUM];

    VIR_Function *              virFunction;
    VIR_Function *              initFunction;
    VIR_SymId                   builtinVariable[SPV_MAX_BUILTINVAR];
    SpvId                       func;
    gctUINT                     argIndex;
    gctCHAR                     virName[SPV_VIR_NAME_SIZE];
    gctCHAR                     tempName[SPV_VIR_NAME_SIZE];
    gctUINT *                   operands;
    gctUINT                     operandSize;
    gctUINT                     maxOperandSize;
    gctUINT                     unknowId;

    gctUINT                     entryID;
    SpvExecutionModel           entryExeMode;
    gctCHAR *                   entryPointName;
    gctUINT                     entryPointNameLength;
    gctUINT                     invalidEntryId[SPV_MAX_INVALID_ENTRY_COUNT];
    gctUINT                     invalidEntryCount;
    gctBOOL                     isMultiEntry;

    SpvCovIDDescriptor          *idDescriptor;
    gctUINT                     idDescSize;

    /* used to record every instruction type (Typedefine? Operator? variable define? */
    SpvCovDecorator             *decorationList;

    SpvInternalInstruction      *cachedInst;
    gctUINT                     cachedInstCount;
    gctUINT                     cachedInstSize;
    gctBOOL                     isCacheInst;

    VkSpecializationInfo        *specInfo;
    SpvEntryInfo                entryInfo;
    SpvRenderPassInfo          *renderpassInfo;
    gctUINT subPass;

    SpvWorkGroupInfo            *workgroupInfo; /* compute shader */
    gctBOOL                     hasWorkGroup;

    gctBOOL                     isLibraryShader;

    gctUINT                     localSize[3];

    gctUINT                     tcsInputVertices;

    SpvFuncCallTable            *funcTable;
};

typedef struct _gcSPV *              gcSPV;

typedef VSC_ErrCode (* _SpvOpCodeFuncAddr)(gcSPV spv, VIR_Shader * virShader);
typedef VSC_ErrCode (* _SpvOperandFuncAddr)(gcSPV spv, VIR_Shader * virShader);

struct SpvInstructionParameters
{
    /* Whether this opcode has left operand. */
    gctBOOL             hasLOperand;
    /* Whether this opcode has a result type. */
    gctBOOL             typePresent;
    /* Whether this opcode has a spv id. */
    gctBOOL             resultPresent;
    SpvOpcodeClass      opClass;
    /* Function handle operators */
    _SpvOpCodeFuncAddr  opFunc;

    /* Should I make this array danamic? */
    gctUINT             oprandSize;
    SpvOperandClass     operandClass[SPV_MAX_OPERAND_NUM];
    gctCHAR *           operandDesc[SPV_MAX_OPERAND_NUM];
    /* Can we have a general interface for all operand class? */
    _SpvOperandFuncAddr operandFunc[SPV_MAX_OPERAND_NUM];

    /* copy from conv2VirsVirOpcodeMap */
    VIR_OpCode          virOpCode;

    VIR_TypeId          virOpFormat;
    VIR_Modifier        modifier;
};

gceSTATUS
gcSPV_Conv2VIR(
    IN gcSPV Spv,
    IN SHADER_HANDLE hVirShader
    );

#endif
