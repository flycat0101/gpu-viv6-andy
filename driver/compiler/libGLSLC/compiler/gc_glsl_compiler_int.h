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


#ifndef __gc_glsl_compiler_int_h_
#define __gc_glsl_compiler_int_h_

#include "gc_glsl_compiler.h"
#include "gc_glsl_preprocessor.h"

/* Debug macros */
/*#define SL_SCAN_ONLY*/
/*#define SL_SCAN_NO_ACTION*/
/*#define SL_SCAN_NO_PREPROCESSOR*/

typedef    gctUINT            gctLABEL;

struct _sloIR_LABEL;
struct _slsNAME;

gceSTATUS
sloCOMPILER_Lock(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_Unlock(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_EmptyMemoryPool(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_SetUnspecifiedInputLocationExist(
IN sloCOMPILER Compiler
);

gceSTATUS
sloCOMPILER_SetUnspecifiedOutputLocationExist(
IN sloCOMPILER Compiler
);

gceSTATUS
sloCOMPILER_SetInputLocationInUse(
IN sloCOMPILER Compiler,
IN gctINT Location,
IN gctSIZE_T Length,
OUT gctBOOL *InUseAlready
);

gceSTATUS
sloCOMPILER_SetOutputLocationInUse(
IN sloCOMPILER Compiler,
IN gctINT Location,
IN gctSIZE_T Length,
OUT gctBOOL *InUseAlready
);

gceSTATUS
sloCOMPILER_SetUniformLocationInUse(
IN sloCOMPILER Compiler,
IN gctINT Location,
IN gctSIZE_T Length,
IN gctBOOL IsPostDecidedArray,
OUT gctBOOL *InUseAlready
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
slCreateSharedVariableStorageBlock(
    IN sloCOMPILER Compiler,
    OUT struct _slsNAME **Block
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

sloPREPROCESSOR
sloCOMPILER_GetPreprocessor(
    IN sloCOMPILER Compiler
    );

sloCODE_EMITTER
sloCOMPILER_GetCodeEmitter(
    IN sloCOMPILER Compiler
    );

sloCODE_GENERATOR
sloCOMPILER_GetCodeGenerator(
    IN sloCOMPILER Compiler
    );

gctBOOL
sloCOMPILER_OptimizationEnabled(
    IN sloCOMPILER Compiler,
    IN sleOPTIMIZATION_OPTION OptimizationOption
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
sloCOMPILER_LoadingBuiltIns(
    IN sloCOMPILER Compiler,
    IN gctBOOL LoadingBuiltIns
    );

gceSTATUS
sloCOMPILER_MainDefined(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_LoadBuiltIns(
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
sloCOMPILER_SetLayout(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_SetNotStagesRelatedLinkError(
    IN sloCOMPILER Compiler,
    IN gceSTATUS   NotStagesRelatedLinkError
    );

gceSTATUS
sloCOMPILER_GetNotStagesRelatedLinkError(
    IN sloCOMPILER Compiler
    );

#endif /* __gc_glsl_compiler_int_h_ */
