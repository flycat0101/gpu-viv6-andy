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


#ifndef __gc_glsl_compiler_int_h_
#define __gc_glsl_compiler_int_h_

#include "gc_glsl_compiler.h"
#include "gc_glsl_preprocessor.h"
#include "gc_glsl_ir.h"

#include "debug/gc_vsc_debug.h"

#define __USE_VSC_MP__              0
#define __ENABLE_VSC_MP_POOL__      (gcvTRUE)
#define __VSC_GENERAL_MP_SIZE__     (512 * 1024)
#define __VSC_PRIVATE_SIZE__        (32 * 1024)

/* Debug macros */
/*#define SL_SCAN_ONLY*/
/*#define SL_SCAN_NO_ACTION*/
/*#define SL_SCAN_NO_PREPROCESSOR*/

struct _sloIR_LABEL;
struct _slsNAME;

gceSTATUS
sloCOMPILER_EmptyMemoryPool(
    IN sloCOMPILER Compiler,
    IN gctBOOL     IsGeneralMemoryPool
    );

gceSTATUS
sloCOMPILER_SetUnspecifiedOutputLocationExist(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_SetInputLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    );

gceSTATUS
sloCOMPILER_SetOutputLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    );

gceSTATUS
sloCOMPILER_SetUniformLocationInUse(
    IN sloCOMPILER Compiler,
    IN gctINT Location,
    IN gctSIZE_T Length
    );

/* Scanner State */
typedef enum _sleSCANNER_STATE
{
    slvSCANNER_NORMAL        = 0,

    slvSCANNER_AFTER_TYPE,

    slvSCANNER_NO_KEYWORD
}
sleSCANNER_STATE;

gceSTATUS
sloCOMPILER_SetScannerState(
    IN sloCOMPILER Compiler,
    IN sleSCANNER_STATE State
    );

sleSCANNER_STATE
sloCOMPILER_GetScannerState(
    IN sloCOMPILER Compiler
    );

struct _slsSWITCH_SCOPE;
typedef struct _slsSWITCH_SCOPE
{
    slsSLINK_NODE node;
    struct _sloIR_LABEL *cases;
}
slsSWITCH_SCOPE;

gceSTATUS
sloCOMPILER_PushSwitchScope(
    IN sloCOMPILER Compiler,
    IN struct _sloIR_LABEL *Cases
    );

gceSTATUS
sloCOMPILER_PopSwitchScope(
    IN sloCOMPILER Compiler
    );

slsSWITCH_SCOPE *
sloCOMPILER_GetSwitchScope(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_SetSwitchScope(
    IN sloCOMPILER Compiler,
    IN struct _sloIR_LABEL *Cases
    );

gceSTATUS
sloCOMPILER_GetSharedVariableList(
    IN sloCOMPILER Compiler,
    OUT slsSLINK_LIST **SharedVariableList
    );

gceSTATUS
sloCOMPILER_AddSharedVariable(
    IN sloCOMPILER Compiler,
    IN struct _slsNAME *VariableName
    );

gceSTATUS
sloCOMPILER_GetConstantVariableList(
    IN sloCOMPILER Compiler,
    OUT slsDLINK_LIST **ConstantVariableList
    );

gceSTATUS
sloCOMPILER_DestroyConstantVariableList(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_InitializeConstantBuffer(
    IN sloCOMPILER Compiler,
    INOUT gctCHAR* Buffer
    );

gceSTATUS
sloCOMPILER_AddConstantVariable(
    IN sloCOMPILER Compiler,
    IN sloIR_CONSTANT Constant
    );

typedef struct _slsBINDING_OFFSET_LIST
{
    slsSLINK_NODE node;
    gctUINT       offset;
}
slsBINDING_OFFSET_LIST;

struct _slsLAYOUT_OFFSET;
typedef struct _slsLAYOUT_OFFSET
{
    slsSLINK_NODE  node;
    gctUINT        binding;
    gctUINT        cur_offset;
    slsSLINK_LIST  offset_list;
}
slsLAYOUT_OFFSET;

gceSTATUS
sloCOMPILER_SearchLayoutOffset(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Binding,
    OUT slsLAYOUT_OFFSET **LayoutOffset
    );

gceSTATUS
sloCOMPILER_FindLayoutOffsetInBinding(
    IN  sloCOMPILER        Compiler,
    IN  slsLAYOUT_OFFSET  *LayoutOffset,
    IN  gctUINT            Offset,
    IN  gctUINT            DataTypeSize,
    IN  gctBOOL            HasIdentifier,
    OUT gctBOOL           *OverLaps
    );

gceSTATUS
sloCOMPILER_ConstructLayoutOffsetInBinding(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Offset,
    IN  slsLAYOUT_OFFSET  *LayoutOffset
    );

gceSTATUS
sloCOMPILER_ConstructLayoutOffset(
    IN  sloCOMPILER        Compiler,
    IN  gctUINT            Binding,
    OUT slsLAYOUT_OFFSET **LayoutOffset
    );

struct _slsSHARED_VARIABLE;
typedef struct _slsSHARED_VARIABLE
{
    slsSLINK_NODE    node;
    struct _slsNAME  *name;
}
slsSHARED_VARIABLE;

struct _slsCONSTANT_VARIABLE;
typedef struct _slsCONSTANT_VARIABLE
{
    slsDLINK_NODE    node;
    sloIR_CONSTANT   constantVar;
}
slsCONSTANT_VARIABLE;

gctBOOL
sloCOMPILER_ExpandNorm(
    IN sloCOMPILER Compiler
    );

gctBOOL
sloCOMPILER_ExtensionEnabled(
    IN sloCOMPILER Compiler,
    IN sleEXTENSION Extension
    );

gceSTATUS
sloCOMPILER_AddLog(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Log
    );

gceSTATUS
sloCOMPILER_GetChar(
    IN sloCOMPILER Compiler,
    OUT gctINT_PTR Char
    );

gceSTATUS
sloCOMPILER_MakeCurrent(
    IN sloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[]
    );

gctINT
slInput(
    IN gctINT MaxSize,
    OUT gctSTRING Buffer
    );

gctPOINTER
slMalloc(
    IN gctSIZE_T Bytes
    );

gctPOINTER
slRealloc(
    IN gctPOINTER Memory,
    IN gctSIZE_T NewBytes
    );

void
slFree(
    IN gctPOINTER Memory
    );

void
slReport(
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctSTRING Message,
    IN ...
    );

gceSTATUS
sloCOMPILER_BuiltinFuncEnabled(
    IN sloCOMPILER Compiler,
    IN gctSTRING Symbol
    );

gceSTATUS
sloCOMPILER_MainDefined(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_Parse(
    IN sloCOMPILER Compiler,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[]
    );

gceSTATUS
sloCOMPILER_DumpIR(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_CheckExtensions(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_SetLayout(
    IN sloCOMPILER Compiler
    );

typedef struct _sloApplyLayout
{
    gctBOOL                     bHasVariable;
    gctBOOL                     bApplyLayout;
    sleSTORAGE_QUALIFIER        storageQual;
} sloApplyLayout;

/* sloCOMPILER object. */
struct _sloCOMPILER
{
    slsOBJECT                   object;
#if __USE_VSC_MP__
    VSC_PRIMARY_MEM_POOL        generalPMP;
    VSC_PRIMARY_MEM_POOL        privatePMP;
    VSC_PRIMARY_MEM_POOL       *currentPMP;
#else
    slsDLINK_LIST               generalMemoryPool;
    slsDLINK_LIST               privateMemoryPool;
#endif
    gcePATCH_ID                 patchId;
    gctUINT                     langVersion;
    gceAPI                      clientApiVersion;
    sleSHADER_TYPE              shaderType;
    gcSHADER                    binary;
    gctSTRING                   log;
    gctUINT                     logBufSize;
    gctBOOL                     createDefaultUBO;
    struct
    {
        /* general variables. */
        slsNAME_SPACE *         generalBuiltinSpace;
        slsHASH_TABLE           generalStringPool;
        /* private variables. */
        gctUINT16               errorCount;
        gctUINT16               warnCount;
        slsHASH_TABLE           privateStringPool;
        sltOPTIMIZATION_OPTIONS optimizationOptions;
        sltEXTENSIONS           extensions;
        sltDUMP_OPTIONS         dumpOptions;
        gctUINT16               dumpOffset;
        sleSCANNER_STATE        scannerState;
        slsSLINK_LIST           switchScope;
        gctUINT                 stringCount;
        gctCONST_STRING *       strings;
        gctUINT                 currentLineNo;
        gctUINT                 currentStringNo;
        gctUINT                 currentCharNo;
        slsNAME_SPACE *         unnamedSpace;
        slsNAME_SPACE *         builtinSpace;
        slsNAME_SPACE *         globalSpace;
        slsNAME_SPACE *         auxGlobalSpace;
        slsNAME_SPACE *         currentSpace;
        slsLAYOUT_QUALIFIER     uniformDefaultLayout;
        slsLAYOUT_QUALIFIER     bufferDefaultLayout;

        slsLAYOUT_QUALIFIER     outDefaultLayout;
        sloApplyLayout          applyOutputLayout;

        slsLAYOUT_QUALIFIER     inDefaultLayout;
        sloApplyLayout          applyInputLayout;

        sloIR_SET               rootSet;
        gctUINT                 tempRegCount;
        gctUINT                 labelCount;
        gctBOOL                 loadingGeneralBuiltIns;
        gctBOOL                 loadingBuiltIns;
        gctBOOL                 redeclareBuiltInVar;
        gctBOOL                 mainDefined;
        gctBOOL                 debug;
        gctBOOL                 optimize;
        gctBOOL                 outputInvariant;
        gctUINT32               outputLocationSettings;
        gctUINT32               uniformLocationMaxLength;
        slsSLINK_LIST           layoutOffset;
        gctINT                  currentIterationCount;
        struct {
           slsDLINK_LIST        typeFloat[sldMAX_VECTOR_COMPONENT];
           slsDLINK_LIST        typeInt[sldMAX_VECTOR_COMPONENT];
           slsDLINK_LIST        typeUInt[sldMAX_VECTOR_COMPONENT];
           slsDLINK_LIST        typeBool[sldMAX_VECTOR_COMPONENT];
        } vecConstants;
        slsSLINK_LIST           sharedVariables;
        /* Use it to save the name of a type. */
        slsDLINK_LIST           dataTypeNameList;
        /* Use it to save the constant variables, e.g., uniforms with initializer, constant array. */
        slsDLINK_LIST           constantVariables;
        /* The size of constant buffer. */
        gctUINT                 constantBufferSize;
        gceSTATUS               hasNotStagesRelatedLinkError;
        sleCOMPILER_FLAGS       compilerFlags;
        VSC_DIContext *         debugInfo;
    }
    context;

    sloPREPROCESSOR             preprocessor;
    sloCODE_EMITTER             codeEmitter;
    sloCODE_GENERATOR           codeGenerator;
};

#endif /* __gc_glsl_compiler_int_h_ */
