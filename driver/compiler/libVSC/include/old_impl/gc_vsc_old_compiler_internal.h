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


#ifndef __gc_vsc_old_compiler_internal_h_
#define __gc_vsc_old_compiler_internal_h_

BEGIN_EXTERN_C()

#define GC_MAX_NUM_RENDER_TARGETS    4

#ifndef GC_ENABLE_DUAL_FP16_PHASE
#define GC_ENABLE_DUAL_FP16_PHASE   1
#endif

/* allocator/deallocator function pointer */
typedef gceSTATUS (*gctAllocatorFunc)(
    IN gctUINT32 Bytes,
    OUT gctPOINTER * Memory
    );

typedef gceSTATUS (*gctDeallocatorFunc)(
    IN gctPOINTER Memory
    );

typedef gctBOOL (*compareFunc) (
     IN void *    data,
     IN void *    key
     );

typedef struct _gcsListNode gcsListNode;
struct _gcsListNode
{
    gcsListNode *       next;
    void *              data;
};

typedef struct _gcsAllocator
{
    gctAllocatorFunc    allocate;
    gctDeallocatorFunc  deallocate;
} gcsAllocator;

/* simple map structure */
typedef struct _SimpleMap SimpleMap;
struct _SimpleMap
{
    gctUINT32     key;
    gctUINT32     val;
    SimpleMap    *next;
    gcsAllocator *allocator;

};

/* SimpleMap Operations */
/* return -1 if not found, otherwise return the mapped value */
gctUINT32
gcSimpleMap_Find(
     IN SimpleMap *Map,
     IN gctUINT32    Key
     );

gceSTATUS
gcSimpleMap_Destory(
     IN SimpleMap *    Map,
     IN gcsAllocator * Allocator
     );

/* Add a pair <Key, Val> to the Map head, the user should be aware that the
 * map pointer is always changed when adding a new node :
 *
 *   gcSimpleMap_AddNode(&theMap, key, val, allocator);
 *
 */
gceSTATUS
gcSimpleMap_AddNode(
     IN SimpleMap **   Map,
     IN gctUINT32      Key,
     IN gctUINT32      Val,
     IN gcsAllocator * Allocator
     );

/* gcsList data structure and related operations */
typedef struct _gcsList
{
    gcsListNode  *head;
    gcsListNode  *tail;
    gctINT        count;
    gcsAllocator *allocator;
} gcsList;

/* List operations */
void
gcList_Init(
    IN gcsList *list,
    IN gcsAllocator *allocator
    );

gceSTATUS
gcList_CreateNode(
    IN void *             Data,
    IN gctAllocatorFunc   Allocator,
    OUT gcsListNode **    ListNode
    );

gceSTATUS
gcList_Clean(
    IN gcsList *          List,
    IN gctBOOL            FreeData
    );

gcsListNode *
gcList_FindNode(
    IN gcsList *      List,
    IN void *         Key,
    IN compareFunc    compare
    );

gceSTATUS
gcList_AddNode(
    IN gcsList *          List,
    IN void *             Data
    );

gceSTATUS
gcList_RemoveNode(
    IN gcsList *          List,
    IN gcsListNode *      Node
    );

/*  link list structure for code list */
typedef gcsList gcsCodeList;
typedef gcsCodeList * gctCodeList;
typedef gcsListNode gcsCodeListNode;

/* block data table structures */
typedef gctUINT gctDATA_ID;

typedef struct _gcsBlockNode
{
    gctUINT32 availableSize;
    void *    data;
} *gcsBlockNode;

typedef struct _gcsBlockTable
{
    gctUINT32     blockSize;
    gctUINT32     elementAlignment;
    gctUINT32     numOfBlocks;
    gcsBlockNode  blocks;

} * gcsBlockTable;

gceSTATUS
gcBLOCKTABLE_Init(
    IN gcsBlockTable Tbl,
    IN gctUINT32     BlockSize,
    IN gctUINT32     NumBlocks,
    IN gctUINT32     Align
    );

gceSTATUS
gcBLOCKTABLE_AddData(
    IN  gcsBlockTable       Tbl,
    IN  const void *        Data,
    IN  gctUINT32           Len,
    OUT gctDATA_ID*         Id);

gceSTATUS
gcBLOCKTABLE_GetAddress(
    IN  const gcsBlockTable Tbl,
    IN  gctDATA_ID Id,
    OUT gctPOINTER * Addr);

gceSTATUS
gcBLOCKTABLE_Destory(gcsBlockTable tbl);

#define gcmBlockUsedSize(Tbl, Idx) ((Tbl)->blockSize - (Tbl)->blocks[Idx].availableSize)
#define gcmGetBlockId(Tbl, Idx) ((Idx) * (Tbl)->blockSize + gcmBlockUsedSize(Tbl, Idx))


typedef struct _gcConstant_Value
{
    gcSHADER_TYPE        type;     /* the type of the constant value */
    gcsValue             val;
} gcConstant_Value;

typedef struct _gcBuiltinsTempIndex
{
   gctINT       PositionTempIndex;
   gctINT       PointSizeTempIndex;
   gctINT       ColorTempIndex;
   gctINT       FrontFacingTempIndex;
   gctINT       PointCoordTempIndex;
   gctINT       PositionWTempIndex;
   gctINT       DepthTempIndex;
   gctINT       FogFragCoordTempIndex;
   gctINT       InstanceIDTempIndex;
   gctINT       VertexIDTempIndex;
   gctINT       NumWorkGroupsTempIndex;
   gctINT       WorkGroupIDTempIndex;
   gctINT       LocalInvocationIDTempIndex;
   gctINT       GlobalInvocationIDTempIndex;
   gctINT       LocalInvocationIndexTempIndex;
   gctINT       GlobalInvocationIndexTempIndex;
   gctINT       HelperInvocationTempIndex;
   gctINT       FrontColorTempIndex;
   gctINT       BackColorTempIndex;
   gctINT       FrontColorSecondaryTempIndex;
   gctINT       BackColorSecondaryTempIndex;
   gctINT       SubsampleDepthTempIndex;
   gctINT       InvocationIDTempIndex;
   gctINT       PatchVerticesInTempIndex;
   gctINT       PrimitiveIDTempIndex;
   gctINT       TessLevelOuterTempIndex;
   gctINT       TessLevelInnerTempIndex;
   gctINT       LayerTempIndex;
   gctINT       PrimitiveIDInTempIndex;
   gctINT       TessCoordTempIndex;
   gctINT       SampleIdTempIndex;
   gctINT       SamplePositionTempIndex;
   gctINT       SampleMaskInTempIndex;
   gctINT       SampleMaskTempIndex;
   gctINT       InPositionTempIndex;
   gctINT       InPointSizeTempIndex;
   gctINT       BoundingBoxTempIndex;
   gctINT       LastFragDataTempIndex;
   gctINT       ClusterIDTempIndex;
   gctINT       ClipDistanceTempIndex;
   gctINT       SecondaryColorTempIndex;
   gctINT       NormalTempIndex;
   gctINT       FogCoordTempIndex;
   gctINT       VertexTempIndex;
   gctINT       MultiTexCoord0TempIndex;
   gctINT       MultiTexCoord1TempIndex;
   gctINT       MultiTexCoord2TempIndex;
   gctINT       MultiTexCoord3TempIndex;
   gctINT       MultiTexCoord4TempIndex;
   gctINT       MultiTexCoord5TempIndex;
   gctINT       MultiTexCoord6TempIndex;
   gctINT       MultiTexCoord7TempIndex;
} gcBuiltinsTempIndex;

typedef struct _gcShaderCodeInfo
{
    gctUINT                 codeCounter[gcSL_MAXOPCODE];
    gcBuiltinsTempIndex     builtinsTempIndex;
    gctBOOL                 hasLoop;
    gctBOOL                 hasBranch;
    gctBOOL                 hadBiasedTexld;
    gctBOOL                 hasLodTexld;
    gctBOOL                 useInstanceID;
    gctBOOL                 useVertexID;
    gctBOOL                 hasFragDepth;      /* has glFragDepth in PS */
    gctBOOL                 usePosition;       /* use glFragCoord in PS */
    gctBOOL                 useFace;           /* use gl_FrontFacing in PS */
    gctBOOL                 useSubsampleDepth; /* inplicit subsample depth register */
    gctBOOL                 useLocalStorage;
    gctBOOL                 useHighPrecision;  /* true if use high precision data
                                                  other than position */
    gctBOOL                 hasInt32OrUint32;  /* true if instruction has int32 or
                                                  uint32 operands*/
    gctINT                  effectiveTexld;    /* the estimated effective
                                                  dynamic texld count */
    gctINT                  estimatedInst;     /* estimated GPU instruction count */
}
gcShaderCodeInfo;

#define SOURCE_ENCODE_CHAR  0xAA

/******************************************************************************\
|************************* gcSL_BRANCH_LIST structure. ************************|
\******************************************************************************/

typedef struct _gcSL_BRANCH_LIST * gcSL_BRANCH_LIST;

struct _gcSL_BRANCH_LIST
{
    /* Pointer to next gcSL_BRANCH_LIST structure in list. */
    gcSL_BRANCH_LIST            next;

    /* Pointer to generated instruction. */
    gctUINT                     ip;

    /* Target instruction for branch. */
    gctUINT                     target;

    /* Flag whether this is a branch or a call. */
    gctBOOL                     call;

    /* Flag whether the branch is duplicated for dual16 T0-T1 */
    gctBOOL                     duplicatedT0T1;
};

/******************************************************************************\
|**************************** gcLINKTREE structure. ***************************|
\******************************************************************************/

typedef struct _gcsLINKTREE_LIST *    gcsLINKTREE_LIST_PTR;

/* Structure that defines the linked list of dependencies. */
typedef struct _gcsLINKTREE_LIST
{
    /* Pointer to next dependent register. */
    gcsLINKTREE_LIST_PTR            next;

    /* Type of dependent register. */
    gcSL_TYPE                       type;

    /* Index of dependent register. */
    gctINT                          index;

    /* Reference counter. */
    gctINT                          counter;
}
gcsLINKTREE_LIST;

/* Structure that defines the dependencies for an attribute. */
typedef struct _gcLINKTREE_ATTRIBUTE
{
    /* In-use flag. */
    gctBOOL                         inUse;

    /* Instruction location the attribute was last used. */
    gctINT                          lastUse;

    /* A linked list of all temporary registers using this attribute. */
    gcsLINKTREE_LIST_PTR            users;
}
* gcLINKTREE_ATTRIBUTE;

typedef struct _gcsCROSS_FUNCTION_LIST *    gcsCROSS_FUNCTION_LIST_PTR;

/* Structure that defines the function call list. */
typedef struct _gcsCROSS_FUNCTION_LIST
{
    /* Pointer to next dependent register. */
    gcsCROSS_FUNCTION_LIST_PTR      next;

    /* Index of function call. */
    gctINT                          callIndex;
}
*gcCROSS_FUNCTION_LIST;

/* Structure that defines the dependencies for a temporary register. */
typedef struct _gcLINKTREE_TEMP
{
    gctINT                          index;  /* index of the register */
    /* In-use flag. */
    gctUINT                         inUse           : 1;
    /* True if the register is used as an index. */
    gctUINT                         isIndex         : 1;
    /* True if the register is used as a normal source. */
    gctUINT                         usedAsNormalSrc : 1;
    /* True if the register is indexing. */
    gctUINT                         isIndexing      : 1;
    gctUINT                         isOwnerKernel   : 1;
    /* True if the register is upper half of a 64bit variable */
    gctUINT                         isPaired64BitUpper : 1;

    /* Usage flags for the temporary register. */
    gctUINT8                        usage   : 8;
    /* Format. */
    gcSL_FORMAT                     format   : 12;
    /* Precision. */
    gcSL_PRECISION                  precision : 8;
    /* Physical register this temporary register is assigned to. */
    gctINT                          assigned  : 8;
    gctUINT                         swizzle   : 8;
    gctINT                          shift     : 8;


    /* Instruction locations that defines the temporary register. */
    gcsLINKTREE_LIST_PTR            defined;

    /* Instruction location the temporary register was last used. */
    gctINT                          lastUse;
    /*  the back jmp index this temp crossed */
    gctINT                          crossLoopIdx;

    /* Dependencies for the temporary register. */
    gcsLINKTREE_LIST_PTR            dependencies;

    /* Whether the register holds a constant. */
    gctINT8                         constUsage[4];
    gcuFLOAT_UINT32                 constValue[4];

    /* A linked list of all registers using this temporary register. */
    gcsLINKTREE_LIST_PTR            users;


    /* Function arguments. */
    gctPOINTER                      owner;

    gceINPUT_OUTPUT                 inputOrOutput;

    /* Variable in shader's symbol table. */
    gcVARIABLE                      variable;

    /* The function list that this temp register must allocate before them. */
    gcsCROSS_FUNCTION_LIST_PTR      crossFuncList;
}
* gcLINKTREE_TEMP;

typedef struct _gcLINKTREE_TEMP_LIST *gcLINKTREE_TEMP_LIST;
struct _gcLINKTREE_TEMP_LIST
{
    gcLINKTREE_TEMP_LIST        next;
    gcLINKTREE_TEMP             temp;
};

enum PackingMode
{
    PackingVec2_2,
    PackingVec3_1,
    PackingVec2_1_1,
    PackingVec1_1_1_1
};

/* Structure that defines the outputs. */
typedef struct _gcLINKTREE_OUTPUT
{
    gctUINT             inUse               : 1;  /* In-use flag. */
    gctUINT             isArray             : 1;  /* true if the corresponding varying is array */
    gctUINT             isPacked            : 1;  /* true if it is packed with other varying */
    gctUINT             isTransformFeedback : 1;  /* true if it is a transform feedback output */
    gctINT              packedWith          : 8;  /* the output index which packed with, if it the
                                                     first output in the pack, the value is itself */
    gctINT              packingMode         : 8; /* the packing mode for packed output */

    /* the number of components in the output */
    gctINT              components;
    gctINT              rows;
    gctINT              elementInArray;

    /* Temporary register holding the output value. */
    gctINT              tempHolding;
    gctINT              vsOutputIndex;

    /* Fragment attribute linked to this vertex output. */
    gctINT              fragmentAttribute;
    gctINT              fragmentIndex;
    gctINT              fragmentIndexEnd;
    gctINT              skippedFragmentAttributeRows;
}
* gcLINKTREE_OUTPUT;

typedef enum _gcComponentsMapping
{
    gcCMAP_UNCHANGED, /* the components position unchanged */
    gcCMAP_XY2ZW, /* map .xy to .zw */
    gcCMAP_X2Y, /* map .x to .y */
    gcCMAP_X2Z, /* map .x to .z */
    gcCMAP_X2W, /* map .x to .w */
    gcCMAP_Y2Z, /* map .y to .z */
    gcCMAP_Y2W, /* map .y to .w */
    gcCMAP_Z2W, /* map .z to .w */
} gcComponentsMapping;

typedef struct _gcVaryingPackInfo
{
    gcLINKTREE_OUTPUT    treeOutput;
    gcSL_ENABLE          enabled;      /* the enabled components in the packed
                                          varying output */
    gcComponentsMapping  compmap; /* the components mapping after packing */
} gcVaryingPackInfo;

typedef struct _gcVaryingPacking
{
    enum PackingMode mode;
    union {
        gcVaryingPackInfo pack4[1];        /* vec4, */
        gcVaryingPackInfo pack2_2[2];      /* (vec2, vec2) */
        gcVaryingPackInfo pack3_1[2];      /* (vec3, vec1) */
        gcVaryingPackInfo pack2_1_1[3];    /* (vec2, vec1, vec1) */
        gcVaryingPackInfo pack1_1_1_1[4];  /* (vec1, vec1, vec1, vec1) */
    } u;
} gcVaryingPacking;

typedef struct _gcArgumentPacking
{
    gctBOOL needPacked          : 2;
    gctBOOL isPacked            : 2;
    gcComponentsMapping mapping : 4;
    gctUINT32 newIndex          : 26;
    gcsFUNCTION_ARGUMENT argument;
} gcArgumentPacking;

typedef struct _gcsCODE_CALLER    *gcsCODE_CALLER_PTR;
typedef struct _gcsCODE_CALLER
{
    gcsCODE_CALLER_PTR            next;

    gctINT                        caller;
}
gcsCODE_CALLER;

typedef struct _gcsCODE_HINT
{
    /* Pointer to function or kernel function to which this code belongs. */
    gctPOINTER            owner;

    gctINT                callNest;        /* Nesting of call. */
    gcsCODE_CALLER_PTR    callers;         /* Callers to this instruction. */
    gcLINKTREE_TEMP_LIST  liveTemps;       /* the list of temps which a alive when enter the
                                            *  instruction,their live range start before this
                                            * instruction (loop head or function head)
                                            */
    gctINT                lastUseForTemp;  /* Last use for temp register used in code gen. */

    gctINT                lastLoadUser;
    gctINT                loadDestIndex;

    gctBOOL               isOwnerKernel    : 2;  /* Is the owner a kernel function?. */
    gctBOOL               isBackJump       : 2;
    gctBOOL               isCall           : 2;  /* Is this instruction a CALL. */
    gctBOOL               isBackJumpTarget : 2;  /* Is this instruction a target of a back jump, we can
                                                  * also use it to check if it is a header of a LOOP. */
    gctBOOL               psHasDiscard     : 2;  /* Flag whether the PS code has discard. */
    gctBOOL               uploadedUBO      : 2;  /* Flag to indicate uniform block uploaded. */
}
gcsCODE_HINT, *gcsCODE_HINT_PTR;

/* Structure that defines the entire life and dependency for a shader. */
typedef struct _gcLINKTREE
{
    /* Pointer to the gcSHADER object. */
    gcSHADER                        shader;

    VSC_HW_CONFIG                   hwCfg;

    /* Process name of this shader. */
    gcePATCH_ID                     patchID;

    /* Link flags */
    gceSHADER_FLAGS                 flags;

    /* Number of attributes. */
    gctUINT32                       attributeCount;

    /* Attributes. */
    gcLINKTREE_ATTRIBUTE            attributeArray;

    /* Number of temporary registers. */
    gctUINT32                       tempCount;

    /* Temporary registers. */
    gcLINKTREE_TEMP                 tempArray;

    /* Number of outputs. */
    gctUINT32                       outputCount;
    /* Number of outputs be packed with output hence be removed from output array */
    gctUINT32                       packedAwayOutputCount;

    /* Outputs. */
    gcLINKTREE_OUTPUT               outputArray;

    /* Uniform usage. */
    gctUINT32                       uniformUsage;

    /* Resource allocation passed. */
    gctBOOL                         physical;

    /* Branch list. */
    gcSL_BRANCH_LIST                branch;

    /* Code hints. */
    gcsCODE_HINT_PTR                hints;

    /* Flag whether use ICache to store instructions. */
    gctBOOL                         useICache;

#if gcdUSE_WCLIP_PATCH
    /* Strict WClip match. */
    gctBOOL                         strictWClipMatch;
    gctINT                          MVPCount;

    gctBOOL                         WChannelEqualToZ;
#endif
}
* gcLINKTREE;

/***************************************************************************
 *  Float20 format has 1 sign bit, 8 exponent bits and 11 mantisa bits with a
 *  hidden 1 in the mantisa bit.
 *  Float32 format has 1 sign bit, 8 exponent bits and 23 mantisa bits with a
 *  hidden 1 in the mantisa bit.
 ***************************************************************************/
#define BitsMask(Bits)   ((gctUINT)((1 << (Bits)) - 1))

#define FP20_SignBits           1
#define FP20_ExponentBits       8
#define FP20_MantissaBits       11
#define FP20_SignStartPos       19
#define FP20_ExponentStartPos   (FP20_SignStartPos - FP20_SignBits)
#define FP20_MantissaStartPos   (FP20_ExponentStartPos - FP20_ExponentBits)
#define FP20_SignBitMask        BitsMask(FP20_SignBits)
#define FP20_ExponentBitMask    BitsMask(FP20_ExponentBits)
#define FP20_MantissaBitMask    BitsMask(FP20_MantissaBits)

#define FP32_SignBits           1
#define FP32_ExponentBits       8
#define FP32_MantissaBits       23
#define FP32_SignStartPos       31
#define FP32_ExponentStartPos   (FP32_SignStartPos - FP32_SignBits)         /* 30 */
#define FP32_MantissaStartPos   (FP32_ExponentStartPos - FP32_ExponentBits) /* 22 */
#define FP32_SignBitMask        BitsMask(FP32_SignBits)
#define FP32_ExponentBitMask    BitsMask(FP32_ExponentBits)
#define FP32_MantissaBitMask    BitsMask(FP32_MantissaBits)

#define UINT20_MAX               (0xFFFFF)
#define UINT20_MIN               (0x0)
#define INT20_MAX                (0x7FFFF)
#define INT20_MIN                (-0x80000)

#ifndef UINT16_MAX
#define UINT16_MAX               (0xFFFF)
#endif
#define UINT16_MIN               (0x0)
#ifndef INT16_MAX
#define INT16_MAX                (0x7FFF)
#endif
#ifndef INT16_MIN
#define INT16_MIN                (-0x8000)
#endif

typedef gctUINT32 gctFP32BINARY;
typedef gctUINT32 gctFP20BINARY;

 typedef struct FLOAT20
{
    gctUINT   Sign     :1;
    gctUINT   Exponent :8;
    gctUINT   Mantissa :11;
} gcsFLOAT20;

#define GetFP20Binary(f20) (((f20)->Sign << 19) | ((f20)->Exponent << 11) | (f20)->Mantissa)
#define GetFP32BianryFromFP20Binary(f20) ((gctUINT32)(((f20)>>19) & FP20_SignBitMask)<<31     | \
                                          (gctUINT32)(((f20)>>11) & FP20_ExponentBitMask)<<23 | \
                                          (gctUINT32)(((f20)>>0)  & FP20_MantissaBitMask)<<12 )

typedef struct
{
    gcSL_FORMAT         ty;
    gctBOOL             fit20bits;   /* true if the value fits 20 bits */
    union {
        gctFLOAT        f;
        gctINT32        i;
        gctUINT32       u;
        gctUINT16       u16;
        gctINT16        i16;
    } value;
} gcsConstantValue;

/* Temp usage structure. */
typedef struct _gcsSL_USAGE           gcsSL_USAGE;
typedef struct _gcsSL_USAGE         * gcsSL_USAGE_PTR;
struct _gcsSL_USAGE
{
    /* Last instruction register is used. */
    gctINT                          lastUse[4];
};

typedef struct _gcsSL_CONSTANT_TABLE   gcsSL_CONSTANT_TABLE;
typedef struct _gcsSL_CONSTANT_TABLE * gcsSL_CONSTANT_TABLE_PTR;
struct _gcsSL_CONSTANT_TABLE
{
    /* Pointer to next constant. */
    gcsSL_CONSTANT_TABLE_PTR        next;

    /* Constant value. */
    gctINT                          count;
    gctFLOAT                        constant[4];

    /* Uniform index. */
    gctINT                          index;

    /* Uniform swizzle. */
    gctUINT8                        swizzle;

    /* Flag to indicate this constant is from UBO or not. */
    gctBOOL                         fromUBO;
};

/* Generate hardware states. */
gceSTATUS
gcLINKTREE_GenerateStates(
    IN OUT gcLINKTREE                  *pTree,
    IN gceSHADER_FLAGS                  Flags,
    IN gcsSL_USAGE_PTR                  UniformUsage,
    IN gcsSL_CONSTANT_TABLE_PTR         ConstUsage,
    IN OUT gcsPROGRAM_STATE             *ProgramState
    );

void gcCGUpdateMaxRegister(
    IN void *                 CodeGen,
    IN gctUINT                Regs,
    IN gcLINKTREE             Tree
    );

gceSTATUS
gcEncodeSourceImmediate20(
    IN OUT gctUINT32         States[4],
    IN gctUINT               Source,
    IN gcsConstantValue *    ConstValue
    );

void
gcSetSrcABS(
     IN OUT gctUINT32 * States,
     IN gctUINT         Src
     );

void
gcSetSrcNEG(
     IN OUT gctUINT32 * States,
     IN gctUINT         Src
     );

gctINT
gcGetSrcType(
     IN gctUINT32 * States,
     IN gctUINT     Src
     );

gctBOOL
isSourceImmediateValue(
     IN gctUINT32 * States,
     IN gctUINT     Src
     );

gctBOOL
gcExtractSource20BitImmediate(
    IN  gctUINT32  States[4],
    IN  gctUINT    Source,
    OUT gctUINT32* Immediate,
    OUT gctUINT32* ImmType
    );

typedef struct _gcsCODE_GENERATOR    gcsCODE_GENERATOR;
typedef struct _gcsCODE_GENERATOR    * gcsCODE_GENERATOR_PTR;

gctUINT
gcsCODE_GENERATOR_GetIP(
    gcsCODE_GENERATOR_PTR CodeGen
    );

typedef gctBOOL (*gctSL_FUNCTION_PTR)(
    IN gcLINKTREE Tree,
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION Instruction,
    IN OUT gctUINT32 * States
    );

typedef struct _gcsSL_PATTERN               gcsSL_PATTERN;
typedef const struct _gcsSL_PATTERN       * gcsSL_PATTERN_PTR;
struct _gcsSL_PATTERN
{
    /* Positive: search index, aproaching zero.
       Negative: code generation index aproaching zero. */
    gctINT                          count;

    /* Opcode. */
    gctUINT                         opcode;

    /* Destination reference number. */
    gctINT8                         dest;

    /* Source 0 reference number. */
    gctINT8                         source0;

    /* Source 1 reference number. */
    gctINT8                         source1;

    /* Source 2 reference number. */
    gctINT8                         source2;

    /* Sampler reference number. */
    gctINT8                         sampler;

    /* Code generation function. */
    gctSL_FUNCTION_PTR              function;
};

/*******************************************************************************
************************************************************** Shader Hinting **
*******************************************************************************/

typedef struct _gcsSL_REFERENCE       gcsSL_REFERENCE;
typedef struct _gcsSL_REFERENCE     * gcsSL_REFERENCE_PTR;
struct _gcsSL_REFERENCE
{
    /* Reference index number. */
    gctINT                          index;

    /* Instruction for reference. */
    gcSL_INSTRUCTION                instruction;

    /* Source index for reference. */
    gctINT                          sourceIndex;
};

/* Physical code. */
typedef struct _gcsSL_PHYSICAL_CODE   gcsSL_PHYSICAL_CODE;
typedef struct _gcsSL_PHYSICAL_CODE * gcsSL_PHYSICAL_CODE_PTR;
struct _gcsSL_PHYSICAL_CODE
{
    /* Pointer to next physical code structure. */
    gcsSL_PHYSICAL_CODE_PTR         next;

    /* Maximum number of instructions this structure can hold. */
    gctUINT32                       maxCount;

    /* Number of instructions this structure holds. */
    gctUINT32                       count;

    /* At least 1 instruction per structure. */
    gctUINT32                       states[4];
};

typedef struct _gcsSL_ADDRESS_REG_COLORING
{
    /* HW address register has 4 channels, each can be used */
    /* as relative address access. This structure is used to */
    /* codegen-time address register coloring. The channel */
    /* coloring here is not aggressive since we have no channel-based */
    /* live analysis for address register, so redundant mova may be */
    /* introduced for certain cases. */

    struct TMP2ADDRMap
    {
        gctINT      indexedReg; /* -1 means no assignment to real reg, but space is occupied,
                                   so startChannelInAddressReg is meanful */
        gctINT      startChannelInIndexedReg; /* zero-based */
        gctUINT8    startChannelInAddressReg; /* zero-based */
        gctINT      channelCount;
    } tmp2addrMap[4];

    /* For current instruction, LSB 4bits is valid, 1-x, 2-y, 4-z, 8-w */
    gctUINT8 localAddrChannelUsageMask;

    gctINT countOfMap;

}gcsSL_ADDRESS_REG_COLORING, *gcsSL_ADDRESS_REG_COLORING_PTR;

typedef struct _gcsSL_FUNCTION_CODE
{
    /* References. */
    gcsSL_REFERENCE                 references[16];
    struct _gcSL_INSTRUCTION        tempInstruction[3];
    gcsSL_REFERENCE                 tempReferences[3];
    gctINT                          tempShift[3];

    /* Branches. */
    gcSL_BRANCH_LIST                branch;

    /* Register last used for indexing. */
    gcsSL_ADDRESS_REG_COLORING      addrRegColoring;

    /* Base and current IP. */
    gctUINT                         ipBase;
    gctUINT                         ip;

    /* Physical code. */
    gcsSL_PHYSICAL_CODE_PTR         root;
    gcsSL_PHYSICAL_CODE_PTR         code;
}
gcsSL_FUNCTION_CODE,
*gcsSL_FUNCTION_CODE_PTR;

typedef struct _gcsSL_CODE_MAP
{
    gcsSL_FUNCTION_CODE_PTR         function;
    gctUINT                         location;
}
gcsSL_CODE_MAP,
* gcsSL_CODE_MAP_PTR;

struct _gcsCODE_GENERATOR
{
    /* Link flags. */
    gceSHADER_FLAGS                 flags;

    /* Uniform usage table. */
    gcsSL_USAGE_PTR                 uniformUsage;
    gctUINT32                       uniformCountWithoutLTC;
    gctUINT32                       uniformBase;
    gctUINT32                       maxUniform;
    gctBOOL                         unifiedUniform;

    /* dummy sampler id used for power management */
    gctINT                          dummySamplerId;

    /* Output variable */
    gctUINT                         maxVaryingVectors;

    gctUINT                         maxExtraVaryingVectors;

    /* Max VS attributes */
    gctUINT                         maxAttributes;

    /* Constant table. */
    gcsSL_CONSTANT_TABLE_PTR        constants;

    /* Register usage table. */
    gcsSL_USAGE_PTR                 registerUsage;
    gctUINT32                       registerCount;
    gctUINT                         maxRegister;

    /* Generated code. */
    gcsSL_FUNCTION_CODE_PTR         functions;
    gcsSL_FUNCTION_CODE_PTR         current;
    gcsSL_CODE_MAP_PTR              codeMap;
    gctUINT                         nextSource;
    gctUINT                         endMain;
    gctUINT                         endPC;
    gctUINT                         instCount;   /* instruction count for the shader */
    gctUINT32_PTR                   previousCode;

    /* State buffer management. */
    gctPOINTER                      stateBuffer;
    gctUINT32                       stateBufferSize;
    gctUINT32                       stateBufferOffset;
    gctUINT32 *                     lastStateCommand;
    gctUINT32                       lastStateCount;
    gctUINT32                       lastStateAddress;
    gctUINT32                       stateDeltaSize;
    gctUINT32*                      stateDeltaBuffer;
    gctUINT32                       stateDeltaBufferOffset;
    gctUINT32*                      lastStateDeltaBatchEnd;
    gctUINT32*                      lastStateDeltaBatchHead;

    /* FragCoord usage. */
    gctBOOL                         usePosition;
    gctINT                          positionIndex;
    gctUINT                         positionPhysical;


    /* FrontFacing usage. */
    gctBOOL                         useFace;
    gctUINT                         facePhysical;
    gctUINT8                        faceSwizzle;

    /* PointCoord usage. */
    gctBOOL                         usePointCoord;
    gctUINT                         pointCoordPhysical;

    /* SubsampleDepth usage. */
    gctBOOL                         subsampleDepthRegIncluded; /* no need to add implicit reg if true */
    gctINT                          subsampleDepthIndex;
    gctUINT                         subsampleDepthPhysical;

    /* Number of Shader cores */
    gctUINT32                       shaderCoreCount;

    /* Hardware flags. */
    gctBOOL                         hasSIGN_FLOOR_CEIL;
    gctBOOL                         hasSQRT_TRIG;
    gctBOOL                         hasNEW_SIN_COS_LOG_DIV;
    gctBOOL                         hasNEW_TEXLD;
    gctBOOL                         hasCL;
    gctBOOL                         hasICache;
    gctBOOL                         hasInteger;
    gctBOOL                         hasLongUlong;
    gctBOOL                         hasDual16;
    gctBOOL                         hasHalti3;
    gctBOOL                         hasHalti4;
    gctBOOL                         hasHalti5;
    gctBOOL                         hasBugFixes7;
    gctBOOL                         hasUSC;

    /* Use ICache. */
    gctBOOL                         useICache;

    /* Type of shader. */
    gcSHADER_KIND                   shaderType;

    /* OpenCL shader. */
    gctBOOL                         clShader;

    /* DX shader. */
    gctBOOL                         dxShader;

    /* compute shader. */
    gctBOOL                         computeShader;

    /* HALTI shader. */
    gctBOOL                         haltiShader;

    /* Is dual-16 shader. */
    gctBOOL                         isDual16Shader;

    /* psInputControlHighpPosition setting */
    gctUINT                         psInputControlHighpPosition;

    /* Flags for CL_X:  gc2100, gc880.
                 CL_EX: gc4000, gc2300, gc2500, gc5000, gc5200.
     */
    gctBOOL                         isCL_X;
    gctBOOL                         isCL_XE;

    /* Flags for bug fix for gc2100 and gc4000. */

    /* BugFixes10:
        - L1 cache "write address - data" pairing error (#732)
        - Resource Check Bug (#742)
        - Back-to-back flushes hang (#744)
        - PE Linear mode 16-bit dithering bug. (gc4000 only)
        - Shader L1 cache sideband FIFO full signal blocks the
            interface properly (#706)
        - L1 cache and local storage performance degrades when request
            size is not aligned to 16-byte boundary (#727)
        - Shader Pulse Eater Performance Bug (#746)
    ********************************************************************/
    gctBOOL                         hasBugFixes10;

    /* BugFixes11:
        - Proper rounding for texture arrays (#634)
        - L1 cache return data ID masking bug fix (#705)
        - Pop count support (#687)
        - 8/16 vec1 integer support (gc2100 only)
        - Load/Store PS support (gc2100 only)
    ********************************************************************/
    gctBOOL                         hasBugFixes11;

    /* SHEnhancements2 feature set:
        - Immediate constants are present (#673)
        - 32 bit global ID and work group ID (#600)
            (1 of the components is 32 bit, others 16)
        - Relative addressing with adaptive algorithm, allowing a
            bigger access to local storage (#656)
        - Access to PC register by extending the InstanceID
        - Branch targets could come from SRC2 as well
        - RTNE support (#764)
        - RCP precision (#675)
        - Bit reversal (#715)
        - Native FP divide
        - Precision improvement for OCL FP
            (rcp, sin, log2, exp, sqrt, rsqrt)
        - I2I support (#685)
    ********************************************************************/
    gctBOOL                         hasSHEnhancements2;

    /* GCMinorFeatures4MediumPrecision feature set:
        - Integer supoort for texkill.cond
    ********************************************************************/
    gctBOOL                         hasMediumPrecision;

    gctBOOL                         generateImmediate;
    gctBOOL                         forceGenImmediate;

    /* SHEnhancements3 feature set:
         - FP16 medium precision
    *******************************************************************/
    gctBOOL                         hasSHEnhancements3;

    /* Reserved register for LOAD for LOAD bug (bugFixes10). */
    gctUINT                         reservedRegForLoad;
    gctINT                          loadDestIndex;
    gctINT                          origAssigned;
    gctINT                          lastLoadUser;

    /* BigEndian flag */
    gctBOOL                         isBigEndian;

#if gcdALPHA_KILL_IN_SHADER
    /* End PC for alpha and color kill. */
    gctUINT                         endPCAlphaKill;
    gctUINT                         endPCColorKill;

    /* Shader instruction for alpha and color kill. */
    gctUINT32                       alphaKillInstruction[4];
    gctUINT32                       colorKillInstruction[4];
#endif

    /* Flag whether or not a KILL instruction has been emitted. */
    gctBOOL                         kill;

    /* hint from vs shader on vertex or instance id being used */
    gctBOOL                         vsHasVertexInstId;
    gctINT                          vertexIdIndex;
    gctINT                          instanceIdIndex;

    gctBOOL                         isConstOutOfMemory;
    gctBOOL                         isRegOutOfResource;
};

/****************************************************************
 *                                                              *
 *        20bit Immediate related help functions                *
 *                                                              *
 ****************************************************************/

/* return true if CodeGen can generate for Instruction's OperandNo
 *   OperandNo 0: source0,
 *             1: source1,
 *            -1: destination
 */
gctBOOL
Generate20BitsImmediate(
    IN gcsCODE_GENERATOR_PTR CodeGen,
    IN gcSL_INSTRUCTION      Instruction,
    IN gctINT                OperandNo
    );

/* return true if the Value can be fit into 20bits,
   considering it may be negated or absoluted later
  */
gctBOOL
ValueFit20Bits(
    IN  gcSL_FORMAT         Format,
    IN  gctUINT32           Hex32
    );

gctBOOL
gcSHADER_CheckBugFixes10(
    void
    );

gctBOOL gcDumpMachineCode(void);

void
gcDump_Shader(
    IN gctFILE          File,
    IN gctCONST_STRING  Text,
    IN gctPOINTER       Optimizer,
    IN gcSHADER         Shader,
    IN gctBOOL          PrintHeaderFooter
    );

gceSTATUS
gcSHADER_CountCode(
    IN  gcSHADER           Shader,
    OUT gcShaderCodeInfo * CodeInfo
    );

gctCONST_STRING
_PredefinedName(
    IN gcSHADER  Shader,
    IN gctUINT32 Length,
    IN gctBOOL UseInstanceName
    );

gctBOOL
gcSHADER_IsDual16Shader(
    IN  gcSHADER           Shader,
    IN  gcShaderCodeInfo * CodeInfoPtr
    );

void
gcSHADER_SetFlag(
    IN OUT  gcSHADER           Shader,
    IN      gcSHADER_FLAGS     Flag
    );

void
gcSHADER_ResetFlag(
    IN OUT  gcSHADER           Shader,
    IN      gcSHADER_FLAGS     Flag
    );

gcUNIFORM
gcSHADER_GetUniformBySamplerIndex(
    IN gcSHADER             Shader,
    IN gctUINT              SamplerIndex,
    OUT gctINT *            ArrayIndex
    );

gctBOOL
gcOPT_doVaryingPackingForShader(
    IN gcSHADER         Shader
    );

gctBOOL
gcOPT_getLoadBalanceForShader(
    IN gcSHADER Shader,
    OUT gctINT * Min,
    OUT gctINT * Max
    );

gceSTATUS
gcSHADER_OptimizeJumps(
    IN gcoOS Os,
    IN gcSHADER Shader
    );

gceSTATUS
gcSHADER_CheckUniformUsage(
    IN OUT gcSHADER           Shader,
    IN gceSHADER_FLAGS        Flags
    );

gceSTATUS
gcSHADER_ConvertIntOrUIntAttribute(
    IN OUT gcSHADER           VertexShader,
    IN OUT gcSHADER           FragmentShader
    );

gceSTATUS
gcSHADER_EvaluateLTCValueWithinLinkTime(
    IN gcSHADER              Shader
    );

/* Pass to convert between gcSL and VIR each other */
gceSTATUS
gcSHADER_Conv2VIR(
    IN gcSHADER Shader,
    IN VSC_HW_CONFIG * hwCfg,
    IN OUT SHADER_HANDLE hVirShader
    );

gceSTATUS
gcSHADER_ConvFromVIR(
    IN OUT gcSHADER Shader,
    IN SHADER_HANDLE hVirShader,
    IN gceSHADER_FLAGS Flags
    );

/* function declaration for compiling/linking built-in functions*/
typedef enum _gcLibType
{
    gcLIB_BUILTIN,
    gcLIB_BLEND_EQUATION,
    gcLIB_DX_BUILTIN,
    gcLIB_CL_BUILTIN,
    gcLIB_CL_LONG_ULONG_FUNCS,
}   gcLibType;

/* compile the builtin function library */
gceSTATUS
gcSHADER_CompileBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    );

/* compile the OpenCL builtin function library */
gceSTATUS
gcSHADER_CompileCLBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    );

/* initialize Shader's temp register mapping table for the LinkedToShader */
gcLibraryList *
gcSHADER_InitMappingTable(
    IN OUT gcSHADER Shader,
    IN     gcSHADER LinkedToShader
    );

/* after the Shader linked with libraries, reset the temp register mapping
 * mapping table in all libraries the Shader links to, so the libraries
 * can be linked with other shaders
 */
gceSTATUS
gcSHADER_ResetLibraryMappingTable(
    IN OUT gcSHADER Shader
    );

/* link the builtin function library into the shader */
gceSTATUS
gcSHADER_LinkBuiltinLibrary(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gcLibType        LibType
    );

/* link a builtin function */
gceSTATUS
gcSHADER_LinkLibFunction(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gctCONST_STRING  FunctionName,
    OUT    gcFUNCTION *     Function
    );
/* find a builtin function */
gceSTATUS
gcSHADER_FindLibFunction(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gctCONST_STRING  FunctionName,
    OUT    gcFUNCTION *     Function
    );

gceSTATUS
gcSHADER_PatchCentroidVaryingAsCenter(
    IN OUT gcSHADER         Shader
    );

gceSTATUS
gcSHADER_PatchInt64(
    IN OUT gcSHADER         Shader
    );

gceSTATUS
gcSHADER_MergeCompileTimeInitializedUniforms(
    IN gcSHADER Shader,
    IN gcSHADER LibShader
    );

gceSTATUS
gcSHADER_FindMainFunction(
    IN   gcSHADER           Shader,
    OUT  gctINT *           StartCode,
    OUT  gctINT *           EndCode
    );

gceSTATUS
gcSHADER_AddOutputLocation(
    IN gcSHADER             Shader,
    IN gctINT               Location,
    IN gctUINT32            Length
    );

gctUINT
gcSHADER_GetFunctionByCodeId(
    IN gcSHADER             Shader,
    IN gctUINT              codeID,
    OUT gctBOOL            *IsKernelFunc
    );

gctUINT
gcSHADER_GetFunctionByFuncHead(
    IN gcSHADER             Shader,
    IN gctUINT              codeID,
    OUT gctBOOL            *IsKernelFunc
    );

gceSTATUS
gcDoPreprocess(
    IN gcSHADER *           Shaders,
    IN gceSHADER_FLAGS      Flags
    );

VIRCGKind
gcGetVIRCGKind(
    IN gctBOOL              HasHalti2
    );

gctUINT
gcGetDualFP16Mode(
    IN gctBOOL              HasHalti2
    );

gctBOOL
gcIsInstHWBarrier(
    IN gcSHADER             Shader,
    IN gcSL_INSTRUCTION     Code,
    IN gctBOOL              bGenerateMC
    );

#define _MASSAGE_MAX_UNIFORM_FOR_OES30(vsUniform, psUniform)   \
    do {                                                       \
        if (gcPatchId == gcvPATCH_GTFES30)                     \
        {                                                      \
            vsUniform = gcmMAX(vsUniform, 256);                \
            psUniform = gcmMAX(psUniform, 224);                \
        }                                                      \
    }                                                          \
    while(0)                                                   \

#define _MASSAGE_MAX_VS_UNIFORM_FOR_OES30(vsUniform)           \
    do {                                                       \
        if (gcPatchId == gcvPATCH_GTFES30)                     \
        {                                                      \
            vsUniform = gcmMAX(vsUniform, 256);                \
        }                                                      \
    }                                                          \
    while(0)

#define _MASSAGE_MAX_PS_UNIFORM_FOR_OES30(psUniform)           \
    do {                                                       \
        if (gcPatchId == gcvPATCH_GTFES30)                     \
        {                                                      \
            psUniform = gcmMAX(psUniform, 224);                \
        }                                                      \
    }                                                          \
    while(0)

END_EXTERN_C()

#endif /* __gc_vsc_old_compiler_internal_h_ */

