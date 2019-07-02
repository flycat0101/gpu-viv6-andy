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


#ifndef __gc_cl_compiler_int_h_
#define __gc_cl_compiler_int_h_

#include "gc_cl_compiler.h"
#include "gc_cl_preprocessor.h"

/* Debug macros */
/*#define CL_SCAN_ONLY*/
/*#define CL_SCAN_NO_ACTION*/
/*#define CL_SCAN_NO_PREPROCESSOR*/

#define __USE_VSC_MP__              1

typedef    gctUINT            gctLABEL;

struct _cloIR_LABEL;
struct _cloIR_CONSTANT;
struct _clsNAME_SPACE;

/* Language version */
enum _cleLANGUAGE_VERSION
{
   clvCL_11    = 0x1,
   clvCL_12    = 0x2,
};

typedef gctUINT32 cltLANGUAGE_VERSION;

gctUINT32
cloGetDefaultLanguageVersion(
    );

#define clmHasRightLanguageVersion(Compiler, Version) \
      (cloCOMPILER_GetLanguageVersion(Compiler) >= (Version))

/* Maximum number of parameter chain variables in an intrinsic ASM call */
#define cldMaxParamChains   5

/* Prefix for a struct name */
#define cldSTRUCT_NAME_PREFIX  "struct$"
#define cldUNION_NAME_PREFIX  "union$"

gceSTATUS
cloCOMPILER_EmptyMemoryPool(
    IN cloCOMPILER Compiler
    );

/* Scanner State */
typedef enum _cleSCANNER_STATE
{
    clvSCANNER_NORMAL    = 0,
    clvSCANNER_AFTER_TYPE,
    clvSCANNER_IN_TYPEOF
}
cleSCANNER_STATE;

gceSTATUS
cloCOMPILER_SetScannerState(
    IN cloCOMPILER Compiler,
    IN cleSCANNER_STATE State
    );

cleSCANNER_STATE
cloCOMPILER_GetScannerState(
    IN cloCOMPILER Compiler
    );

struct _clsBUILTIN_FUNC_REFERENCED;
typedef struct _clsBUILTIN_FUNC_REFERENCED
{
    slsSLINK_NODE node;
    struct _cloIR_POLYNARY_EXPR *member;
}
clsBUILTIN_FUNC_REFERENCED;

gceSTATUS
cloCOMPILER_AddReferencedBuiltinFunc(
    IN cloCOMPILER Compiler,
    IN struct _cloIR_POLYNARY_EXPR *FuncCall
    );

slsSLINK_LIST *
cloCOMPILER_GetReferencedBuiltinFuncList(
    IN cloCOMPILER Compiler
    );

struct _clsCONSTANT_VARIABLE;
typedef struct _clsCONSTANT_VARIABLE
{
    slsSLINK_NODE node;
    struct _clsNAME *member;
}
clsCONSTANT_VARIABLE;

struct _clsNAME_POOL;
typedef struct _clsNAME_POOL
{
    slsSLINK_NODE node;
    struct _clsNAME *member;
}
clsNAME_POOL;

gceSTATUS
cloCOMPILER_AddConstantVariable(
    IN cloCOMPILER Compiler,
    IN struct _clsNAME *ConstantVariable
    );

gceSTATUS
cloCOMPILER_FindConstantVariable(
    IN cloCOMPILER Compiler,
    IN gctINT Offset,
    IN OUT struct _clsNAME **ConstantVariable
    );

struct _clsSWITCH_SCOPE;
typedef struct _clsSWITCH_SCOPE
{
    slsSLINK_NODE node;
    struct _cloIR_LABEL *cases;
}
clsSWITCH_SCOPE;

gceSTATUS
cloCOMPILER_SetSwitchScope(
    IN cloCOMPILER Compiler,
    IN struct _cloIR_LABEL *Cases
    );

gceSTATUS
cloCOMPILER_PushSwitchScope(
    IN cloCOMPILER Compiler,
    IN struct _cloIR_LABEL *Cases
    );

gceSTATUS
cloCOMPILER_PopSwitchScope(
    IN cloCOMPILER Compiler
    );

clsSWITCH_SCOPE *
cloCOMPILER_GetSwitchScope(
    IN cloCOMPILER Compiler
    );

struct _clsDESIGNATION_SCOPE;
typedef struct _clsDESIGNATION_SCOPE
{
    slsSLINK_NODE node;
    struct _cloIR_EXPR *designation;
}
clsDESIGNATION_SCOPE;

gceSTATUS
cloCOMPILER_SetDesignationScope(
    IN cloCOMPILER Compiler,
    IN struct _cloIR_EXPR *Designation
    );

gceSTATUS
cloCOMPILER_PushDesignationScope(
    IN cloCOMPILER Compiler,
    IN struct _cloIR_EXPR *Designation
    );

gceSTATUS
cloCOMPILER_PopDesignationScope(
    IN cloCOMPILER Compiler
    );

clsDESIGNATION_SCOPE *
cloCOMPILER_GetDesignationScope(
    IN cloCOMPILER Compiler
    );

cloPREPROCESSOR
cloCOMPILER_GetPreprocessor(
    IN cloCOMPILER Compiler
    );

cloCODE_EMITTER
cloCOMPILER_GetCodeEmitter(
    IN cloCOMPILER Compiler
    );

cloCODE_GENERATOR
cloCOMPILER_GetCodeGenerator(
    IN cloCOMPILER Compiler
    );

gctBOOL
cloCOMPILER_OptimizationEnabled(
    IN cloCOMPILER Compiler,
    IN cleOPTIMIZATION_OPTION OptimizationOption
    );

gctBOOL
cloCOMPILER_IsBasicTypePacked(
IN cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsLongUlongPatch(
IN cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsGcslDriverImage(
IN cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_ExtensionEnabled(
    IN cloCOMPILER Compiler,
    IN cleEXTENSION Extension
    );

cleEXTENSION
cloCOMPILER_GetExtension(
IN cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_AddLog(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING Log
    );

gceSTATUS
cloCOMPILER_GetChar(
    IN cloCOMPILER Compiler,
    OUT gctINT_PTR Char
    );

gceSTATUS
cloCOMPILER_MakeCurrent(
    IN cloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options
    );

gctINT
clInput(
    IN gctINT MaxSize,
    OUT gctSTRING Buffer
    );

gctPOINTER
clMalloc(
    IN gctSIZE_T Bytes
    );

gctPOINTER
clRealloc(
    IN gctPOINTER Memory,
    IN gctSIZE_T NewBytes
    );

void
clFree(
    IN gctPOINTER Memory
    );

void
clReport(
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleREPORT_TYPE Type,
    IN gctSTRING Message,
    IN ...
    );

gceSTATUS
cloCOMPILER_LoadingBuiltins(
    IN cloCOMPILER Compiler,
    IN gctBOOL LoadingBuiltins
    );

gceSTATUS
cloCOMPILER_LoadingGeneralBuiltins(
    IN cloCOMPILER Compiler,
    IN gctBOOL LoadingBuiltins
    );

gceSTATUS
cloCOMPILER_InKernelFunctionEpilog(
    IN cloCOMPILER Compiler,
    IN gctBOOL InKernelFunctionEpilog
    );

gceSTATUS
cloCOMPILER_MainDefined(
    IN cloCOMPILER Compiler
    );

gceSTATUS
cloCOMPILER_KernelFuncDefined(
    IN cloCOMPILER Compiler
    );

gceSTATUS
cloCOMPILER_LoadBuiltins(
    IN cloCOMPILER Compiler
    );

gceSTATUS
cloCOMPILER_LoadGeneralBuiltIns(
    IN cloCOMPILER Compiler
    );

gceSTATUS
cloCOMPILER_SetNeedConstantMemory(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsConstantMemoryNeeded(
cloCOMPILER Compiler
);

gctSIZE_T
cloCOMPILER_GetConstantMemoryNeeded(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetNeedPrivateMemory(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsPrivateMemoryNeeded(
cloCOMPILER Compiler
);

gctSIZE_T
cloCOMPILER_GetPrivateMemoryNeeded(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetHasImageQuery(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetNeedPrintfMemory(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsPrintfMemoryNeeded(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetNeedLocalMemory(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsLocalMemoryNeeded(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetHasLocalMemoryKernelArg(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_HasLocalMemoryKernelArg(
cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsExternSymbolsAllowed(
cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetMaxKernelFunctionArgs(
cloCOMPILER Compiler,
gctUINT numArgs
);

gceSTATUS
cloCOMPILER_Parse(
    IN cloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options
    );

gceSTATUS
cloCOMPILER_DetachFromMemoryPool(
IN cloCOMPILER Compiler,
IN gctPOINTER Memory
);

gceSTATUS
cloCOMPILER_MakeConstantName(
IN cloCOMPILER Compiler,
IN gctSTRING Prefix,
OUT cltPOOL_STRING *symbolInPool
);

gctBOOL
cloCOMPILER_IsNameSpaceUnnamed(
IN cloCOMPILER Compiler,
IN struct _clsNAME_SPACE *  NameSpace
);

gctBOOL
cloCOMPILER_IsNameSpaceGlobal(
IN cloCOMPILER Compiler,
IN struct _clsNAME_SPACE *  NameSpace
);

gctBOOL
cloCOMPILER_IsDumpOn(
IN cloCOMPILER Compiler,
IN cleDUMP_OPTION DumpOption
);

gceSTATUS
cloCOMPILER_DumpIR(
    IN cloCOMPILER Compiler
    );

gcSHADER clTuneKernel(IN gcSHADER Shader, IN gctCONST_STRING Source, IN gctCONST_STRING Options);

#endif /* __gc_cl_compiler_int_h_ */
