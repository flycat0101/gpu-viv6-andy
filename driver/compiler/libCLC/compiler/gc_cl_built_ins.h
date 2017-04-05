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


#ifndef __gc_cl_built_ins_h_
#define __gc_cl_built_ins_h_

#include "gc_cl_gen_code.h"
#include "gc_cl_emit_code.h"

#define clmMAX_BUILT_IN_PARAMETER_COUNT        5
#define cldBUILT_IN_NAME_MANGLING_ENABLED       0
#define cldMAX_BUILT_IN_TYPE_MANGLED_NAME_CHAR_COUNT    10
#define cldMAX_BUILT_IN_FUNCTION_MANGLED_NAME_CHAR_COUNT    24

typedef gceSTATUS
(* cltBUILT_IN_EVALUATE_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN cloIR_CONSTANT * OperandConstants,
    IN OUT cloIR_CONSTANT ResultConstant
    );

typedef gceSTATUS
(* cltBUILT_IN_GEN_CODE_FUNC_PTR)(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN cloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    );

typedef struct _clsBUILTIN_FUNCTION_INFO
{
    gctCONST_STRING    symbol;
    gctBOOL        nameMangled;
    gctBOOL        handleVector;
    cltBUILT_IN_EVALUATE_FUNC_PTR evaluate;
    cltBUILT_IN_GEN_CODE_FUNC_PTR genCode;
}
clsBUILTIN_FUNCTION_INFO;

typedef struct _clsBUILTIN_DATATYPE_INFO
{
    gctINT    type;
    gctINT    dualType;  /* e.g. T_SHORT2 type and T_SHORT2_PACKED are duals of each other */
    gctINT    componentType;  /* component type of vector or matrix */
    clsGEN_CODE_DATA_TYPE dataType;
    clePOLYNARY_EXPR_TYPE constructorType;
    gctBOOL   isUnsigned;
    clsDATA_TYPE *typePtr[cldQUALIFIER_ACCESS_COUNT][cldQUALIFIER_ADDRESS_SPACE_COUNT];
    gctCONST_STRING mangledName;
    gctUINT    virPrimitiveType;
}
clsBUILTIN_DATATYPE_INFO;

extern clsBUILTIN_DATATYPE_INFO clBuiltinDataTypes[];
/* First built-in data type */
#define cldFirstBuiltinDataType   (T_VERY_FIRST_TERMINAL + 1)
#define clmBuiltinDataTypeInfo(t) clBuiltinDataTypes[(t) - cldFirstBuiltinDataType]
#define clmGenCodeDataType(t) clmBuiltinDataTypeInfo(t).dataType

gceSTATUS
clCleanupBuiltins(
void
);

gceSTATUS
clLoadBuiltins(
IN cloCOMPILER Compiler,
IN cleSHADER_TYPE ShaderType
);

clsBUILTIN_FUNCTION_INFO *
clGetBuiltinFunctionInfo(
IN gctCONST_STRING Symbol
);

cltPOOL_STRING
clGetFastRelaxedMathFunction(
IN cloCOMPILER Compiler,
IN gctCONST_STRING Symbol
);

gceSTATUS
clGetBuiltinVariableImplSymbol(
IN cloCOMPILER Compiler,
IN clsNAME *Name,
IN gctCONST_STRING Symbol,
OUT gctCONST_STRING * ImplSymbol
);

clsBUILTIN_DATATYPE_INFO *
clGetBuiltinDataTypeInfo(
IN gctINT Type
);

gctBOOL
clIsBuiltinDataType(IN gctINT Token);

gctBOOL
clIsTextureLookupFunction(
IN gctCONST_STRING Symbol
);

gceSTATUS
clEvaluateBuiltinFunction(
IN cloCOMPILER Compiler,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN cloIR_CONSTANT * OperandConstants,
OUT cloIR_CONSTANT * ResultConstant
);

gceSTATUS
clGenBaseMemoryAddressCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN clsNAME *KernelFunc
);


gceSTATUS
clGenDivCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    );

gceSTATUS
clGenInverseCode(
    IN cloCOMPILER Compiler,
    IN cloCODE_GENERATOR CodeGenerator,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctUINT OperandCount,
    IN clsGEN_CODE_PARAMETERS * OperandsParameters,
    IN clsIOPERAND * IOperand
    );
gceSTATUS
clGenBuiltinFunctionCode(
IN cloCOMPILER Compiler,
IN cloCODE_GENERATOR CodeGenerator,
IN cloIR_POLYNARY_EXPR PolynaryExpr,
IN gctUINT OperandCount,
IN clsGEN_CODE_PARAMETERS * OperandsParameters,
IN clsIOPERAND * IOperand,
IN OUT clsGEN_CODE_PARAMETERS * Parameters,
IN gctBOOL DoInlineCheck
);

#endif /* __gc_cl_built_ins_h_ */
