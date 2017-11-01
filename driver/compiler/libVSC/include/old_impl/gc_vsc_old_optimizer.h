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


/*
**    Include file the defines the front- and back-end compilers, as well as the
**    objects they use.
*/

#ifndef __gc_vsc_old_optimizer_h_
#define __gc_vsc_old_optimizer_h_

BEGIN_EXTERN_C()

#define __DUMP_OPTIMIZER__              0  /* DEBUG !!! */
#define __DUMP_OPTIMIZER_DETAILS__      0

/* Right now, all the chips have the same call stack limitation, 4. */
#define _MAX_CALL_STACK_DEPTH_          3

#define _REMAP_TEMP_INDEX_FOR_FUNCTION_ 0

/*******************************************************************************
**                            gcOptimizer Constants
*******************************************************************************/
#define gcvOPT_INPUT_REGISTER           -1
#define gcvOPT_OUTPUT_REGISTER          -2
#define gcvOPT_GLOBAL_REGISTER          -3
#define gcvOPT_UNDEFINED_REGISTER       -4
#define gcvOPT_JUMPUNDEFINED_REGISTER   -5

#define gcdHasOptimization(Flags, Opt)  ((Flags & (gctUINT)Opt) == (gctUINT)Opt)

typedef struct _gcOPTIMIZER *           gcOPTIMIZER;
typedef struct _gcOPT_LIST *            gcOPT_LIST;
typedef struct _gcOPT_TEMP_DEFINE *     gcOPT_TEMP_DEFINE;
typedef struct _gcOPT_CODE *            gcOPT_CODE;
typedef struct _gcOPT_TEMP *            gcOPT_TEMP;
typedef struct _gcOPT_GLOBAL_USAGE *    gcOPT_GLOBAL_USAGE;
typedef struct _gcOPT_FUNCTION *        gcOPT_FUNCTION;

/*******************************************************************************
**                            Data Flow Data Structures
*******************************************************************************/
/* Structure that defines the linked list of dependencies. */
struct _gcOPT_LIST
{
    /* Pointer to next dependent register. */
    gcOPT_LIST                  next;

    /* Index of instruction. */
    /*        -1:        input variable/register. */
    /*        -2:        output variable/register. */
    /*        -3:        global variable/register. */
    /*        -4:        undefined variable/register. */
    /*    others:        instruction pointer. */
    gctINT                      index;

    /* Pointer to code. */
    gcOPT_CODE                  code;
};

struct _gcOPT_TEMP_DEFINE
{
    gcOPT_LIST                  xDefines;
    gcOPT_LIST                  yDefines;
    gcOPT_LIST                  zDefines;
    gcOPT_LIST                  wDefines;
};

struct _gcOPT_CODE
{
    /* Next and Prev for linked list. */
    gcOPT_CODE                  next;
    gcOPT_CODE                  prev;

    /* Number sequence of the code. */
    gctUINT                     id;

    /* A working id */
    gctUINT                     workingId;

    /* Instruction. */
    struct _gcSL_INSTRUCTION    instruction;

    /* Pointer to the function to which this code belongs. */
    gcOPT_FUNCTION              function;

    /* Callers/jumpers to this instruction. */
    gcOPT_LIST                  callers;

    /* Call/jump target code. */
    gcOPT_CODE                  callee;

    /* Flags for data flow construction. */
    gctBOOL                     backwardJump : 2;
    gctBOOL                     handled      : 2;

    /* the index to Loadtime Expression array */
    gctINT                      ltcArrayIdx  : 20;

    /* Temp define info for label. */
    gcOPT_TEMP_DEFINE           tempDefine;

    /* Dependencies for the instruction. */
    gcOPT_LIST                  dependencies0;
    gcOPT_LIST                  dependencies1;

    /* Users of the instruction. */
    gcOPT_LIST                  users;

    /* Previous define instructions. */
    gcOPT_LIST                  prevDefines;

    /* Next define instructions. */
    gcOPT_LIST                  nextDefines;
};

/* Structure that defines the entire life and dependency for a shader. */
struct _gcOPT_TEMP
{
    /* In-use flag. */
    gctBOOL                     inUse       : 2;

    /* Is-global flag. */
    gctBOOL                     isGlobal    : 2;

    /* True if the register is used as an index. */
    gctBOOL                     isIndex     : 2;

    gctBOOL                     setFunction : 2;

    /* Usage flags for the temporary register. */
    gctUINT                     usage       : 8;

    /* Precision */
    gcSHADER_PRECISION          precision   : 4;

    /* Data format for the temporary register. */
    gctINT                      format      : 12;     /* -1: unknown, -2: union. */

    /**/
    gcVARIABLE                  arrayVariable;

    /*
    ** 1) Pointer to the function if it is used as a function argument.
    ** 2) Pointer to the function if it is used as a local temp register.
    ** 3) Otherwise it is NULL.
    */
    gcOPT_FUNCTION              function;

    /* Pointer to the argument if it is used as a function argument. */
    gcsFUNCTION_ARGUMENT_PTR    argument;

    /* Temp interger. */
    gctINT                      tempInt;
};

/* Structure that save the usage of global variables in function. */
struct _gcOPT_GLOBAL_USAGE
{
    /* Pointer to next global usage. */
    gcOPT_GLOBAL_USAGE          next;

    /* Global register index. */
    gctUINT                     index;

    /* Usage direction in function. */
    gceINPUT_OUTPUT             direction;

    /* Def & Use channels in enable form */
    gctUINT                     defEnable;
    gctUINT                     useEnable;
};

struct _gcOPT_FUNCTION
{
    /* Code list. */
    gcOPT_CODE                  codeHead;
    gcOPT_CODE                  codeTail;

    /* Global variable usages. */
    gcOPT_GLOBAL_USAGE          globalUsage;

    /* Pointer to shader function. */
    gcFUNCTION                  shaderFunction;

    /* Pointer to shader function. */
    gcKERNEL_FUNCTION           kernelFunction;

    /* Number of arguments. */
    gctUINT32                   argumentCount;

    /* Pointer to shader's function's arguments. */
    gcsFUNCTION_ARGUMENT_PTR    arguments;

    /* temp register start index, end index and count */
    gctUINT32                   tempIndexStart;

    gctUINT32                   tempIndexEnd;

    gctUINT32                   tempIndexCount;

    /* Updated for global usage. */
    gctBOOL                     updated;

    /* Max call stack depth. */
    gctINT                      maxDepthValue;

    /* Max call stack depth callee's function. */
    gcOPT_FUNCTION              maxDepthFunc;

    /* Function has EMIT code inside. */
    gctBOOL                     hasEmitCode;
};

/*  link list structure for code list */
typedef gcsList gcsTempRegisterList;
typedef gcsTempRegisterList * gctTempRegisterList;
typedef gcsListNode gcsTempRegisterListNode;

#define ltcRegisterWithComponents(RegIndex, Components)             \
          (((Components) << 16) | (gctUINT16)(RegIndex))

#define ltcGetRegister(Value)   ((gctUINT16)((Value) & 0xffff))

#define ltcGetComponents(Value) ((gctUINT16)((Value) >> 16))

#define ltcRemoveComponents(Value, Components)                 \
          (ltcRegisterWithComponents(ltcGetRegister(Value), \
           ltcGetComponents(Value) & ~(Components)))

#define ltcAddComponents(Value, Components)                    \
          (ltcRegisterWithComponents(ltcGetRegister(Value), \
           ltcGetComponents(Value) | (Components)))

#define ltcHasComponents(Value, Components)                    \
          (ltcGetComponents(Value) & (Components)))

typedef struct _gcsLTCExpression gcsLTCExpression;

struct _gcsLTCExpression
{
    gctINT             flag;
    gctFLOAT *         theValue;     /* the pointer to the calculated constant value */
    gctINT             codeCount;    /* number of instructions in the expr */
    gcSL_INSTRUCTION   code_list;
    gcsLTCExpression * next;
} ;

typedef struct _gcsDummyUniformInfo
{
   gctINT                 shaderKind;   /* vertex or fragment shader */
   gcUNIFORM *            uniform;      /* pointer to the load-time constant uniform */
   gcsLTCExpression *     expr;         /* the expression used to evaluate the uniform value at
                                           load-time */
   gctINT                 codeIndex;    /* the code index in LTCExpression for the dummy uniform */
   struct _gcsDummyUniformInfo *  next;
} gcsDummyUniformInfo;

gceSTATUS
gcOPT_OptimizeLoadtimeConstant(
    IN gcOPTIMIZER Optimizer
    );

extern gcsAllocator ltcAllocator;

typedef struct _gcsConstantAssignment
{
    gctINT                  tempIndex;
    struct {
        gcSL_INSTRUCTION    inst;
        union {
            gctFLOAT        f32;
            gctINT32        i32;
            gctUINT32       u32;
        } val;
    } compnents[4];
} gcsConstantAssignment;

struct _gcOPTIMIZER
{
    /* Pointer to the gcSHADER object. */
    gcSHADER                    shader;

    /* Patch ID */
    gcePATCH_ID                 patchID;

    /* Number of instructons in shader. */
    gctUINT                     codeCount;
    gctUINT                     jmpCount;

    /* Code list. */
    gcOPT_CODE                  codeHead;
    gcOPT_CODE                  codeTail;

    /* Code list. */
    gcOPT_CODE                  freeCodeList;

    /* Number of temporary registers. */
    gctUINT                     tempCount;

    /* Temporary registers. */
    gcOPT_TEMP                  tempArray;

    /* Main program. */
    gcOPT_FUNCTION              main;

    /* Number of functions. */
    gctUINT                     functionCount;

    /* Function array. */
    gcOPT_FUNCTION              functionArray;

    /* Current function being processed. */
    gcOPT_FUNCTION              currentFunction;

    /* Global variables. */
    gcOPT_LIST                  global;

    /* Points back to shader's outputs. */
    gctUINT32                   outputCount;
    gcOUTPUT *                  outputs;

    /* Kernel function is merged with main. */
    gctBOOL                     isMainMergeWithKerenel;

    gcsTempRegisterList         theLTCTempRegList;
    gcsCodeList                 theLTCCodeList;
    gcsCodeList                 theLTCRemoveCodeList;
    gcsTempRegisterList         indexedVariableListForLTC;
    SimpleMap *                 tempRegisterMap;

    gcOPTIMIZER_OPTION *        globalOptions;
    gctUINT                     option;

    /* Memory pools. */
    gcsMEM_FS_MEM_POOL          codeMemPool;
    gcsMEM_FS_MEM_POOL          listMemPool;
    gcsMEM_FS_MEM_POOL          usageMemPool;

    gcsMEM_AFS_MEM_POOL         functionArrayMemPool;
    gcsMEM_AFS_MEM_POOL         codeArrayMemPool;
    gcsMEM_AFS_MEM_POOL         tempDefineArrayMemPool;

    /* hardware capabilities */
    VSC_HW_CONFIG               hwCfg;
    gctBOOL                     isHalti2;
    gctBOOL                     supportImmediate;
    gctUINT                     maxVertexUniforms;
    gctUINT                     maxFragmentUniforms;
    gctUINT                     maxVaryings;
    gctUINT                     maxShaderCoreCount;
    gctUINT                     maxThreadCount;
    gctUINT                     maxVertexInstructionCount;
    gctUINT                     maxFragmentInstructionCount;

    /* Log file. */
    gctFILE                     logFile;

    gctBOOL                     isCTSInline;
    gctBOOL                     checkOutputUse;
};



#define gcvINSTR_SIZE    gcmSIZEOF(struct _gcSL_INSTRUCTION)

extern const struct _gcSL_INSTRUCTION gcvSL_NOP_INSTR;

/*******************************************************************************
**                            gcOptimizer Supporting Functions
*******************************************************************************/

/*******************************************************************************
**                            gcOpt_ConstructOptimizer
********************************************************************************
**
**    Construct a new gcOPTIMIZER structure.
**
**    INPUT:
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
**
**    OUT:
**
**        gcOPTIMIZER * Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_ConstructOptimizer(
    IN gcSHADER Shader,
    OUT gcOPTIMIZER * Optimizer
    );

/*******************************************************************************
**                            gcOpt_ReconstructOptimizer
********************************************************************************
**
**    Reconstruct a gcOPTIMIZER structure.
**
**    INPUT:
**
**        gcSHADER Shader
**            Pointer to a gcSHADER object holding information about the compiled shader.
**
**    OUT:
**
**        gcOPTIMIZER * Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_ReconstructOptimizer(
    IN gcSHADER            Shader,
    OUT gcOPTIMIZER *      OptimizerPtr
    );

/*******************************************************************************
**                            gcOpt_DestroyOptimizer
********************************************************************************
**
**    Destroy a gcOPTIMIZER structure.
**
**    IN OUT:
**
**        gcOPTIMIZER Optimizer
**            Pointer to a gcOPTIMIZER structure.
*/
gceSTATUS
gcOpt_DestroyOptimizer(
    IN OUT gcOPTIMIZER Optimizer
    );

gceSTATUS
gcOpt_CopyInShader(
    IN gcOPTIMIZER Optimizer,
    IN gcSHADER Shader
    );

gceSTATUS
gcOpt_CopyOutShader(
    IN gcOPTIMIZER Optimizer,
    IN gcSHADER Shader
    );

gceSTATUS
gcOpt_RebuildFlowGraph(
    IN gcOPTIMIZER Optimizer
    );

gceSTATUS
gcOpt_RemoveNOP(
    IN gcOPTIMIZER Optimizer
    );

gceSTATUS
gcOpt_UpdateCallStackDepth(
    IN gcOPTIMIZER Optimizer,
    IN gctBOOL     ForceUpdate
    );

gceSTATUS
gcOpt_CalculateStackCallDepth(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    OUT gctINT          *Depth
    );

gctBOOL
gcOpt_MoveCodeListBefore(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeFirst,
    IN gcOPT_CODE       SrcCodeLast,
    IN gcOPT_CODE       DestCode
    );

void
gcOpt_MoveCodeListAfter(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeFirst,
    IN gcOPT_CODE       SrcCodeLast,
    IN gcOPT_CODE       DestCode,
    IN gctBOOL          MergeToUpper
    );

gceSTATUS
gcOpt_AddCodeAfter(
    IN  gcOPTIMIZER     Optimizer,
    IN  gcOPT_CODE      Code,
    OUT gcOPT_CODE*     NewCodePtr
    );

gceSTATUS
gcOpt_AddCodeBefore(
    IN  gcOPTIMIZER     Optimizer,
    IN  gcOPT_CODE      Code,
    OUT gcOPT_CODE*     NewCodePtr
    );

gceSTATUS
gcOpt_CopyCodeListAfter(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       SrcCodeFirst,
    IN gcOPT_CODE       SrcCodeLast,
    IN gcOPT_CODE       DestCode
    );

gceSTATUS
gcOpt_RemoveCodeList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       CodeFirst,
    IN gcOPT_CODE       CodeLast
    );

gceSTATUS
gcOpt_ChangeCodeToNOP(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       Code
    );

gctBOOL
gcOpt_UpdateIndex(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctINT*          TempIndexMappingArray,
    IN gctINT*          CurrentTempIndex,
    OUT gctUINT32 *     IndexPtr
    );

gctBOOL
gcOpt_UpdateIndexed(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctINT*          TempIndexMappingArray,
    IN gctINT*          CurrentTempIndex,
    OUT gctUINT16 *     IndexPtr
    );

gctBOOL
gcOpt_RemapTempIndexForCode(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_CODE       Code,
    IN gcOPT_FUNCTION   Function,
    IN gctINT*          TempIndexMappingArray,
    IN gctINT*          CurrentTempIndex
    );

gceSTATUS
gcOpt_RemapTempIndexForFunction(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctINT           NewTempIndexStart
    );

gceSTATUS
gcOpt_DestroyCodeDependency(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    );

gceSTATUS
gcOpt_DeleteFunction(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_FUNCTION   Function,
    IN gctBOOL          RebuildDF,
    IN gctBOOL          DeleteVariable
    );

void
gcOpt_UpdateCodeId(
    IN gcOPTIMIZER      Optimizer
    );

gctBOOL
gcOpt_IsCodeBelongToFunc(
    IN gcOPTIMIZER       Optimizer,
    IN gcOPT_CODE        Code,
    OUT gcOPT_FUNCTION * Function
    );

void
gcOpt_UpdateCodeFunction(
    IN gcOPTIMIZER      Optimizer
    );

gceSTATUS
gcOpt_AddIndexToList(
    IN gcOPTIMIZER Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctINT           Index
    );

gceSTATUS
gcOpt_CheckListHasUndefined(
    IN gcOPTIMIZER Optimizer,
    IN gcOPT_LIST  Root
    );

gceSTATUS
gcOpt_ReplaceIndexInList(
    IN gcOPTIMIZER Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctUINT          Index,
    IN gctUINT          NewIndex
    );

gceSTATUS
gcOpt_DeleteIndexFromList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gctINT           Index
    );

gceSTATUS
gcOpt_FreeList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * List
    );

gceSTATUS
gcOpt_FindCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       Root,
    IN gcOPT_CODE       Code
    );

gceSTATUS
gcOpt_AddCodeToList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    );

gceSTATUS
gcOpt_ReplaceCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code,
    IN gcOPT_CODE       NewCode
    );

gceSTATUS
gcOpt_CheckCodeInList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    );

gceSTATUS
gcOpt_DeleteCodeFromList(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_LIST * Root,
    IN gcOPT_CODE       Code
    );

gceSTATUS
gcOpt_AddListToList(
    IN gcOPTIMIZER      Optimizer,
    IN gcOPT_LIST       SrcList,
    IN gctBOOL          IsJump,
    IN OUT gcOPT_LIST * Root
    );

gceSTATUS
gcOpt_DestroyList(
    IN gcOPTIMIZER Optimizer,
    IN OUT gcOPT_LIST * Root
    );

gcSL_ENABLE
gcGetUsedComponents(
    IN gcSL_INSTRUCTION Instruction,
    IN gctINT           SourceNo
    );

gctBOOL
gcOpt_isCodeInSameBB(
    IN gcOPT_CODE       FirstCode,
    IN gcOPT_CODE       SecondCode
    );

/* return true if the FristCode is dominated by the SecondCode */
gctBOOL
gcOpt_dominatedBy(
    IN gcOPT_CODE       FirstCode,
    IN gcOPT_CODE       SecondCode
    );

gctBOOL
gcOpt_isRedefKillsAllPrevDef(
      IN gcOPT_LIST     Dependencies,
      IN gcSL_ENABLE    EanbledComponents
      );

gctBOOL
gcOpt_hasMultipleDependencyForSameTemp(
      IN gcOPT_LIST     Dependencies,
      IN gcSL_ENABLE    EanbledComponents
      );

gceSTATUS
gcOpt_BuildTempArray(
    IN gcOPTIMIZER      Optimizer
    );

gceSTATUS
gcOpt_DestroyTempArray(
    IN gcOPTIMIZER      Optimizer
    );

gceSTATUS
gcOpt_RebuildTempArray(
    IN gcOPTIMIZER      Optimizer
    );

gceSTATUS
gcOpt_RemapTempIndex(
    INOUT gcOPTIMIZER * OptimizerPtr
    );

/*******************************************************************************
**                                gcOptimizer Logging
*******************************************************************************/
void
gcOpt_Dump(
    IN gctFILE          File,
    IN gctCONST_STRING  Text,
    IN gcOPTIMIZER      Optimizer,
    IN gcSHADER         Shader
    );

void
gcOpt_DumpMessage(
    IN gcoOS            Os,
    IN gctFILE          File,
    IN gctSTRING        Message
    );

#define DUMP_OPTIMIZER(Text, Optimizer)                                \
   do {                                                             \
     if (gcSHADER_DumpOptimizerVerbose((Optimizer)->shader) ) {  \
       gcOpt_Dump(Optimizer->logFile, Text, Optimizer, gcvNULL);    \
     }                                                              \
   } while (gcvFALSE)

#define DUMP_SHADER(File, Text, Shader)                                \
   do {                                                             \
     if (gcSHADER_DumpOptimizer(Shader)) {           \
       gcOpt_Dump(File, Text, gcvNULL, Shader);                     \
     }                                                              \
   } while (gcvFALSE)


/* dump optimizer to stdout */
void dbg_dumpOptimizer(gcOPTIMIZER Optimizer);
/* dump shader to stdout */
void dbg_dumpShader(gcSHADER Shader);

void dbg_dumpCode(gcOPT_CODE Code);

void
gcOpt_GenShader(
    IN gcSHADER         Shader,
    IN gctFILE          File
    );

void
gcOpt_DestroyCodeFlowData(
    IN gcOPTIMIZER      Optimizer,
    IN OUT gcOPT_CODE   code
    );

END_EXTERN_C()

#endif /* __gc_vsc_old_optimizer_h_ */

