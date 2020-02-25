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


#ifndef __gc_glsl_built_ins_def_h_
#define __gc_glsl_built_ins_def_h_

#include "gc_glsl_built_ins.h"

#define MIX_AS_INTRINCIS 0

/* how many kinds of param datatype constructor could be there for the param set.
   currently define it to 3, mapping to T_TYPE_MATCH_CALLBACK0, T_TYPE_MATCH_CALLBACK1
   and T_TYPE_MATCH_CALLBACK2 */
#define PARAMDATATYPECONSTRUCTORCOUNT 3

typedef gceSTATUS (*paramDataTypeConstructor)(
    IN sloCOMPILER,
    OUT slsDATA_TYPE**
    );

typedef gceSTATUS
(* sltBUILT_IN_EVALUATE_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

typedef gceSTATUS
(* sltBUILT_IN_GEN_CODE_FUNC_PTR)(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

/*******************************evaluate/gen function pointer*******************************/
gceSTATUS
_ConvertCoordForSampler1D(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * NewCoordIOperand
    );

gceSTATUS
_GenTexture1DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureCubeLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureCubeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture3DLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture3DProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture3DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture3DProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture1DCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture1DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenShadow1DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenShadow2DArrayCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture1DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexture2DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenShadow1DArrayLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjLodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexelFetchCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTexelFetchOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureLodOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjLodOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureGradOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjGradCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenTextureProjGradOffsetCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenVivTextureGatherCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenVivArrayLengthMethodCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateRadians(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenRadiansCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateDegrees(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenDegreesCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateSin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenSinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateCos(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenCosCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateTan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenTanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAsin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateAcos(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAcosCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAcoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAsinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAtanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAtan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateSinh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenSinhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateCosh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenCoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateTanh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenTanhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAsinh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAsinhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAcoshCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAcosh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateAtanh(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAtanhCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluatePow(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenPowCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateExp(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenExpCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateLog(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenLogCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateExp2(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenExp2Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateLog2(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenLog2Code(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateSqrt(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenSqrtCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateInverseSqrt(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenInverseSqrtCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAbs(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAbsCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateSign(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenSignCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateFloor(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenFloorCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateTrunc(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenTruncCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateRound(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateRoundEven(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateCeil(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenCeilCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateFract(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenFractCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateMod(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenModCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateMin(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenMinCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateMax(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenMaxCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateClamp(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenClampCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateMix(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenMixCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateStep(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenStepCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateSmoothStep(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenSmoothStepCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateIsNan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenIsNanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateIsInf(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenIsInfCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateFloatBitsToInteger(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenFloatBitsToIntCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenFloatBitsToUintCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateIntegerBitsToFloat(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenIntBitsToFloatCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenUintBitsToFloatCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluatePackSnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateUnpackSnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluatePackUnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateUnpackUnorm2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluatePackHalf2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateUnpackHalf2x16(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateLength(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenLengthCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateDistance(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenDistanceCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateDot(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenDotCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateCross(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenCrossCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateNormalize(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenNormalizeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateFaceForward(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenFaceForwardCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateReflect(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_EvaluateRefract(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenRefractCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateMatrixCompMult(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenMatrixCompMultCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateOuterProduct(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenOuterProductCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateTranspose(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenTransposeCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateDeterminant(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenDeterminantCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateInverse(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenInverseCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateLessThan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenLessThanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateLessThanEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenLessThanEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateGreaterThan(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenGreaterThanCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateGreaterThanEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenGreaterThanEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateNotEqual(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenNotEqualCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAny(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAnyCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateAll(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenAllCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateNot(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenNotCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_EvaluateDerivatives(
    IN sloCOMPILER Compiler,
    IN gctUINT OperandCount,
    IN sloIR_CONSTANT * OperandConstants,
    IN OUT sloIR_CONSTANT ResultConstant
    );

gceSTATUS
_GenDFdxCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenDFdyCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenFwidthCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenBitCountCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAtomicCounterCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenAtomicOpCode(
    IN sloCOMPILER              Compiler,
    IN sloCODE_GENERATOR        CodeGenerator,
    IN sloIR_POLYNARY_EXPR      PolynaryExpr,
    IN gctUINT                  OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND *            IOperand
    );

gceSTATUS
_GenBarrierOpCode(
    IN sloCOMPILER              Compiler,
    IN sloCODE_GENERATOR        CodeGenerator,
    IN sloIR_POLYNARY_EXPR      PolynaryExpr,
    IN gctUINT                  OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND *            IOperand
    );

gceSTATUS
_GenFtransformCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenEmitVertexCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenEndPrimitiveCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenEmitStreamVertexCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

gceSTATUS
_GenEndStreamPrimitiveCode(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator,
    IN sloIR_POLYNARY_EXPR PolynaryExpr,
    IN gctUINT OperandCount,
    IN slsGEN_CODE_PARAMETERS * OperandsParameters,
    IN slsIOPERAND * IOperand
    );

/*******************************function flag def*******************************/
#define ATOMIC              slvFUNC_ATOMIC
#define MEM_ACCESS          slvFUNC_HAS_MEM_ACCESS
#define HAS_VAR_ARG         slvFUNC_HAS_VAR_ARG
#define HAS_VOID_PARA       slvFUNC_HAS_VOID_PARA
#define INTRINSIC           slvFUNC_IS_INTRINSIC
#define TREAT_F_AS_I        slvFUNC_TREAT_FLOAT_AS_INT

/*******************************Normal functions*******************************/
/* Built-In Functions */
typedef struct _slsBUILT_IN_FUNCTION
{
    gctINT                          extension;
    gctCONST_STRING                 symbol;
    sltBUILT_IN_EVALUATE_FUNC_PTR   evaluate;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode;
    gctINT                          returnType;
    gctUINT                         paramCount;
    gctINT                          paramTypes[slmMAX_BUILT_IN_PARAMETER_COUNT];
    sltMEMORY_ACCESS_QUALIFIER      paramMemoryAccess[slmMAX_BUILT_IN_PARAMETER_COUNT];
    paramDataTypeConstructor        paramDataTypeConstructors[PARAMDATATYPECONSTRUCTORCOUNT];
    sltFUNC_FLAG                    flags;
}
slsBUILT_IN_FUNCTION;

static slsBUILT_IN_FUNCTION VSBuiltInFunctions[] =
{
    /* Texture Lookup Functions */
    {slvEXTENSION1_NON_HALTI,     "texture2DLod", gcvNULL, _GenTexture2DLodCode,         T_VEC4,     3, {T_SAMPLER2D,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProjLod", gcvNULL, _GenTexture2DProjLodCode,     T_VEC4,     3, {T_SAMPLER2D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProjLod", gcvNULL, _GenTexture2DProjLodCode,     T_VEC4,     3, {T_SAMPLER2D,    T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "textureCubeLod", gcvNULL, _GenTextureCubeLodCode,       T_VEC4,     3, {T_SAMPLERCUBE,  T_VEC3,     T_FLOAT}, {0}, {0}},

    /* 3D Texture Lookup Functions */
    {slvEXTENSION1_TEXTURE_3D, "texture3DLod", gcvNULL, _GenTexture3DLodCode,         T_VEC4,   3, {T_SAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_3D, "texture3DProjLod", gcvNULL, _GenTexture3DProjLodCode,     T_VEC4,   3, {T_SAMPLER3D,    T_VEC4,     T_FLOAT}, {0}, {0}},

    /* Texture array Lookup Functions */
    {slvEXTENSION1_TEXTURE_ARRAY, "texture1DArrayLod", gcvNULL, _GenTexture1DArrayLodCode, T_VEC4,   3, {T_SAMPLER1DARRAY,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "texture2DArrayLod", gcvNULL, _GenTexture2DArrayLodCode, T_VEC4,   3, {T_SAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "shadow1DArrayLod", gcvNULL, _GenShadow1DArrayLodCode,  T_VEC4,   3, {T_SAMPLER1DARRAYSHADOW, T_VEC3,  T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_NONE,          "ftransform", gcvNULL, _GenFtransformCode,        T_VEC4,   0, {0}, {0}, {0}},
};

static gctUINT VSBuiltInFunctionCount =
                    sizeof(VSBuiltInFunctions) / sizeof(slsBUILT_IN_FUNCTION);

static slsBUILT_IN_FUNCTION FSBuiltInFunctions[] =
{
    /* Texture Lookup Functions */
    {slvEXTENSION1_NON_HALTI,     "texture2D", gcvNULL, _GenTexture2DCode,            T_VEC4,     3, {T_SAMPLER2D,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     3, {T_SAMPLER2D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     3, {T_SAMPLER2D,    T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "textureCube", gcvNULL, _GenTextureCubeCode,          T_VEC4,     3, {T_SAMPLERCUBE,  T_VEC3,     T_FLOAT}, {0}, {0}},

    /* 3D Texture Lookup Functions */
    {slvEXTENSION1_TEXTURE_3D,     "texture3D", gcvNULL, _GenTexture3DCode,      T_VEC4,     3, {T_SAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_3D,     "texture3DProj", gcvNULL, _GenTexture3DProjCode,  T_VEC4,     3, {T_SAMPLER3D,    T_VEC4,     T_FLOAT}, {0}, {0}},

    /* Texture array Lookup Functions */
    {slvEXTENSION1_TEXTURE_ARRAY, "texture1DArray", gcvNULL, _GenTexture1DArrayCode, T_VEC4,     3, {T_SAMPLER1DARRAY,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "texture2DArray", gcvNULL, _GenTexture2DArrayCode, T_VEC4,     3, {T_SAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "shadow1DArray", gcvNULL, _GenShadow1DArrayCode,  T_VEC4,     3, {T_SAMPLER1DARRAYSHADOW, T_VEC3,  T_FLOAT}, {0}, {0}},

    /* HALTI Texture Lookup Functions */
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     3, {T_SAMPLER2D,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     3, {T_SAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     3, {T_SAMPLERCUBE,  T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     3, {T_SAMPLERCUBEARRAY,  T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    3, {T_SAMPLER2DSHADOW,   T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    3, {T_SAMPLERCUBESHADOW, T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    3, {T_SAMPLERCUBEARRAYSHADOW, T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     3, {T_SAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    3, {T_ISAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    3, {T_ISAMPLERCUBE,  T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    3, {T_ISAMPLERCUBEARRAY,  T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    3, {T_ISAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    3, {T_USAMPLER2D,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    3, {T_USAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    3, {T_USAMPLERCUBE,  T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    3, {T_USAMPLERCUBEARRAY,  T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    3, {T_USAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_SUPPORT_OGL, "shadow2D", gcvNULL, _GenTextureCode,         T_VEC4,     2, {T_SAMPLER2DSHADOW,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_SUPPORT_OGL, "shadow2D", gcvNULL, _GenTextureCode,         T_VEC4,     3, {T_SAMPLER2DSHADOW,   T_VEC3,  T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     3, {T_SAMPLER2D,    T_VEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     3, {T_SAMPLER2D,    T_VEC4, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     3, {T_SAMPLER3D,    T_VEC4, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_FLOAT,    3, {T_SAMPLER2DSHADOW, T_VEC4, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC4, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    3, {T_ISAMPLER3D,    T_VEC4, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    3, {T_USAMPLER2D,    T_VEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    3, {T_USAMPLER2D,    T_VEC4, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    3, {T_USAMPLER3D,    T_VEC4, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     4, {T_SAMPLER2D,    T_VEC2, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     4, {T_SAMPLER3D,    T_VEC3, T_IVEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_FLOAT,    4, {T_SAMPLER2DSHADOW,   T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     4, {T_SAMPLER2DARRAY,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    4, {T_ISAMPLER2D,    T_VEC2, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    4, {T_ISAMPLER3D,    T_VEC3, T_IVEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    4, {T_ISAMPLER2DARRAY,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    4, {T_USAMPLER2D,    T_VEC2, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    4, {T_USAMPLER3D,    T_VEC3, T_IVEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    4, {T_USAMPLER2DARRAY,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_VEC4,     4, {T_SAMPLER2D,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_VEC4,     4, {T_SAMPLER2D,    T_VEC4, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_VEC4,     4, {T_SAMPLER3D,    T_VEC4, T_IVEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_FLOAT,    4, {T_SAMPLER2DSHADOW, T_VEC4, T_IVEC2, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_IVEC4,    4, {T_ISAMPLER2D,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_IVEC4,    4, {T_ISAMPLER2D,    T_VEC4, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_IVEC4,    4, {T_ISAMPLER3D,    T_VEC4, T_IVEC3, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_UVEC4,    4, {T_USAMPLER2D,    T_VEC3, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_UVEC4,    4, {T_USAMPLER2D,    T_VEC4, T_IVEC2, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,        T_UVEC4,    4, {T_USAMPLER3D,    T_VEC4, T_IVEC3, T_FLOAT}, {0}, {0}},

    /* 3D Texture Lookup Functions */
    {slvEXTENSION1_TEXTURE_3D,     "texture3D", gcvNULL, _GenTexture3DCode,      T_VEC4,     3, {T_SAMPLER3D,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_3D,     "texture3DProj", gcvNULL, _GenTexture3DProjCode,  T_VEC4,     3, {T_SAMPLER3D,    T_VEC4,     T_FLOAT}, {0}, {0}},

    /* Texture array Lookup Functions */
    {slvEXTENSION1_TEXTURE_ARRAY, "texture1DArray", gcvNULL, _GenTexture1DArrayCode, T_VEC4,     3, {T_SAMPLER1DARRAY,    T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "texture2DArray", gcvNULL, _GenTexture2DArrayCode, T_VEC4,     3, {T_SAMPLER2DARRAY,    T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "shadow1DArray", gcvNULL, _GenShadow1DArrayCode,  T_VEC4,     3, {T_SAMPLER1DARRAYSHADOW, T_VEC3,  T_FLOAT}, {0}, {0}},

    /* Derivative Functions */
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdx", _EvaluateDerivatives, _GenDFdxCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdx", _EvaluateDerivatives, _GenDFdxCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdx", _EvaluateDerivatives, _GenDFdxCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdx", _EvaluateDerivatives, _GenDFdxCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdy", _EvaluateDerivatives, _GenDFdyCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdy", _EvaluateDerivatives, _GenDFdyCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdy", _EvaluateDerivatives, _GenDFdyCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "dFdy", _EvaluateDerivatives, _GenDFdyCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "fwidth", _EvaluateDerivatives, _GenFwidthCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "fwidth", _EvaluateDerivatives, _GenFwidthCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "fwidth", _EvaluateDerivatives, _GenFwidthCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {(slvEXTENSION1_STANDARD_DERIVATIVES | slvEXTENSION1_HALTI),     "fwidth", _EvaluateDerivatives, _GenFwidthCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},
};

static gctUINT FSBuiltInFunctionCount =
                    sizeof(FSBuiltInFunctions) / sizeof(slsBUILT_IN_FUNCTION);

static slsBUILT_IN_FUNCTION GSBuiltInFunctions[] =
{
    {slvEXTENSION1_EXT_GEOMETRY_SHADER,     "EmitVertex", gcvNULL, _GenEmitVertexCode,            T_VOID,     0, {0}, {0}},
    {slvEXTENSION1_EXT_GEOMETRY_SHADER,     "EndPrimitive", gcvNULL, _GenEndPrimitiveCode,          T_VOID,     0, {0}, {0}},
    {slvEXTENSION1_EXT_GEOMETRY_SHADER | slvEXTENSION1_SUPPORT_OGL,     "EmitStreamVertex", gcvNULL, _GenEmitStreamVertexCode,            T_VOID,     1, {T_INT}, {0}},
    {slvEXTENSION1_EXT_GEOMETRY_SHADER | slvEXTENSION1_SUPPORT_OGL,     "EndStreamPrimitive", gcvNULL, _GenEndStreamPrimitiveCode,          T_VOID,     1, {T_INT}, {0}},
};

static gctUINT GSBuiltInFunctionCount =
                    sizeof(GSBuiltInFunctions) / sizeof(slsBUILT_IN_FUNCTION);

static slsBUILT_IN_FUNCTION CommonBuiltInFunctions[] =
{
    /* Angle and Trigonometry Functions */
    {slvEXTENSION1_NONE,     "radians", _EvaluateRadians, _GenRadiansCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "radians", _EvaluateRadians, _GenRadiansCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "radians", _EvaluateRadians, _GenRadiansCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "radians", _EvaluateRadians, _GenRadiansCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "degrees", _EvaluateDegrees, _GenDegreesCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "degrees", _EvaluateDegrees, _GenDegreesCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "degrees", _EvaluateDegrees, _GenDegreesCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "degrees", _EvaluateDegrees, _GenDegreesCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "sin", _EvaluateSin, _GenSinCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sin", _EvaluateSin, _GenSinCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sin", _EvaluateSin, _GenSinCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sin", _EvaluateSin, _GenSinCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "cos", _EvaluateCos, _GenCosCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "cos", _EvaluateCos, _GenCosCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "cos", _EvaluateCos, _GenCosCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "cos", _EvaluateCos, _GenCosCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "sinh", _EvaluateSinh, _GenSinhCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "sinh", _EvaluateSinh, _GenSinhCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "sinh", _EvaluateSinh, _GenSinhCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "sinh", _EvaluateSinh, _GenSinhCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "cosh", _EvaluateCosh, _GenCoshCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "cosh", _EvaluateCosh, _GenCoshCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "cosh", _EvaluateCosh, _GenCoshCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "cosh", _EvaluateCosh, _GenCoshCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "tanh", _EvaluateTanh, _GenTanhCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "tanh", _EvaluateTanh, _GenTanhCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "tanh", _EvaluateTanh, _GenTanhCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "tanh", _EvaluateTanh, _GenTanhCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "asinh", _EvaluateAsinh, _GenAsinhCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "asinh", _EvaluateAsinh, _GenAsinhCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "asinh", _EvaluateAsinh, _GenAsinhCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "asinh", _EvaluateAsinh, _GenAsinhCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "atanh", _EvaluateAtanh, _GenAtanhCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "atanh", _EvaluateAtanh, _GenAtanhCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "atanh", _EvaluateAtanh, _GenAtanhCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "atanh", _EvaluateAtanh, _GenAtanhCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    /* Exponential Functions */
    {slvEXTENSION1_NONE,     "pow", _EvaluatePow, _GenPowCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "pow", _EvaluatePow, _GenPowCode, T_VEC2,     2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "pow", _EvaluatePow, _GenPowCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "pow", _EvaluatePow, _GenPowCode, T_VEC4,     2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "exp", _EvaluateExp, _GenExpCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp", _EvaluateExp, _GenExpCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp", _EvaluateExp, _GenExpCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp", _EvaluateExp, _GenExpCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "log", _EvaluateLog, _GenLogCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log", _EvaluateLog, _GenLogCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log", _EvaluateLog, _GenLogCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log", _EvaluateLog, _GenLogCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "exp2", _EvaluateExp2, _GenExp2Code, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp2", _EvaluateExp2, _GenExp2Code, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp2", _EvaluateExp2, _GenExp2Code, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "exp2", _EvaluateExp2, _GenExp2Code, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "log2", _EvaluateLog2, _GenLog2Code, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log2", _EvaluateLog2, _GenLog2Code, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log2", _EvaluateLog2, _GenLog2Code, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "log2", _EvaluateLog2, _GenLog2Code, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "sqrt", _EvaluateSqrt, _GenSqrtCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sqrt", _EvaluateSqrt, _GenSqrtCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sqrt", _EvaluateSqrt, _GenSqrtCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "sqrt", _EvaluateSqrt, _GenSqrtCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE, "sqrt", _EvaluateSqrt, _GenSqrtCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "sqrt", _EvaluateSqrt, _GenSqrtCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "sqrt", _EvaluateSqrt, _GenSqrtCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "sqrt", _EvaluateSqrt, _GenSqrtCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE, "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "inversesqrt", _EvaluateInverseSqrt, _GenInverseSqrtCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    /* Common Functions */
    {slvEXTENSION1_NONE,     "abs", _EvaluateAbs, _GenAbsCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "abs", _EvaluateAbs, _GenAbsCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "abs", _EvaluateAbs, _GenAbsCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "abs", _EvaluateAbs, _GenAbsCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,  "abs", _EvaluateAbs, _GenAbsCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,  "abs", _EvaluateAbs, _GenAbsCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,  "abs", _EvaluateAbs, _GenAbsCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,  "abs", _EvaluateAbs, _GenAbsCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "abs", _EvaluateAbs, _GenAbsCode, T_INT,      1, {T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "abs", _EvaluateAbs, _GenAbsCode, T_IVEC2,    1, {T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "abs", _EvaluateAbs, _GenAbsCode, T_IVEC3,    1, {T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "abs", _EvaluateAbs, _GenAbsCode, T_IVEC4,    1, {T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "sign", _EvaluateSign, _GenSignCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "sign", _EvaluateSign, _GenSignCode, T_VEC2,     1, {T_VEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "sign", _EvaluateSign, _GenSignCode, T_VEC3,     1, {T_VEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "sign", _EvaluateSign, _GenSignCode, T_VEC4,     1, {T_VEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_HALTI,    "sign", _EvaluateSign, _GenSignCode, T_INT,      1, {T_INT}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_HALTI,    "sign", _EvaluateSign, _GenSignCode, T_IVEC2,    1, {T_IVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_HALTI,    "sign", _EvaluateSign, _GenSignCode, T_IVEC3,    1, {T_IVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_HALTI,    "sign", _EvaluateSign, _GenSignCode, T_IVEC4,    1, {T_IVEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "sign", _EvaluateSign, _GenSignCode, T_DOUBLE,   1, {T_DOUBLE}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "sign", _EvaluateSign, _GenSignCode, T_DVEC2,    1, {T_DVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "sign", _EvaluateSign, _GenSignCode, T_DVEC3,    1, {T_DVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "sign", _EvaluateSign, _GenSignCode, T_DVEC4,    1, {T_DVEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_NONE,     "floor", _EvaluateFloor, _GenFloorCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "floor", _EvaluateFloor, _GenFloorCode, T_VEC2,     1, {T_VEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "floor", _EvaluateFloor, _GenFloorCode, T_VEC3,     1, {T_VEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "floor", _EvaluateFloor, _GenFloorCode, T_VEC4,     1, {T_VEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "floor", _EvaluateFloor, _GenFloorCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "floor", _EvaluateFloor, _GenFloorCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "floor", _EvaluateFloor, _GenFloorCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "floor", _EvaluateFloor, _GenFloorCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_HALTI,    "trunc", _EvaluateTrunc, _GenTruncCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "trunc", _EvaluateTrunc, _GenTruncCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "trunc", _EvaluateTrunc, _GenTruncCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "trunc", _EvaluateTrunc, _GenTruncCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "trunc", _EvaluateTrunc, _GenTruncCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "trunc", _EvaluateTrunc, _GenTruncCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "trunc", _EvaluateTrunc, _GenTruncCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "trunc", _EvaluateTrunc, _GenTruncCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "ceil", _EvaluateCeil, _GenCeilCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "ceil", _EvaluateCeil, _GenCeilCode, T_VEC2,     1, {T_VEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "ceil", _EvaluateCeil, _GenCeilCode, T_VEC3,     1, {T_VEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "ceil", _EvaluateCeil, _GenCeilCode, T_VEC4,     1, {T_VEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "ceil", _EvaluateCeil, _GenCeilCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "ceil", _EvaluateCeil, _GenCeilCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "ceil", _EvaluateCeil, _GenCeilCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "ceil", _EvaluateCeil, _GenCeilCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_NONE,     "fract", _EvaluateFract, _GenFractCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "fract", _EvaluateFract, _GenFractCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "fract", _EvaluateFract, _GenFractCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "fract", _EvaluateFract, _GenFractCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "fract", _EvaluateFract, _GenFractCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "fract", _EvaluateFract, _GenFractCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "fract", _EvaluateFract, _GenFractCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "fract", _EvaluateFract, _GenFractCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC2,     2, {T_VEC2,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC3,     2, {T_VEC3,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC4,     2, {T_VEC4,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC2,     2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "mod", _EvaluateMod, _GenModCode, T_VEC4,     2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DOUBLE,    2, {T_DOUBLE,        T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC2,     2, {T_DVEC2,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC3,     2, {T_DVEC3,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC4,     2, {T_DVEC4,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC2,     2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mod", _EvaluateMod, _GenModCode, T_DVEC4,     2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC2,     2, {T_VEC2,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC3,     2, {T_VEC3,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC4,     2, {T_VEC4,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC2,     2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "min", _EvaluateMin, _GenMinCode, T_VEC4,     2, {T_VEC4,         T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_INT,      2, {T_INT,          T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC2,    2, {T_IVEC2,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC3,    2, {T_IVEC3,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC4,    2, {T_IVEC4,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_IVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UINT,     2, {T_UINT,         T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC2,    2, {T_UVEC2,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC3,    2, {T_UVEC3,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC4,    2, {T_UVEC4,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "min", _EvaluateMin, _GenMinCode, T_UVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DOUBLE,    2, {T_DOUBLE,        T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC2,     2, {T_DVEC2,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC3,     2, {T_DVEC3,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC4,     2, {T_DVEC4,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC2,     2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "min", _EvaluateMin, _GenMinCode, T_DVEC4,     2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC2,     2, {T_VEC2,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC3,     2, {T_VEC3,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC4,     2, {T_VEC4,         T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC2,     2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "max", _EvaluateMax, _GenMaxCode, T_VEC4,     2, {T_VEC4,         T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_INT,      2, {T_INT,          T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC2,    2, {T_IVEC2,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC3,    2, {T_IVEC3,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC4,    2, {T_IVEC4,        T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_IVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UINT,     2, {T_UINT,         T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC2,    2, {T_UVEC2,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC3,    2, {T_UVEC3,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC4,    2, {T_UVEC4,        T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "max", _EvaluateMax, _GenMaxCode, T_UVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DOUBLE,    2, {T_DOUBLE,        T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC2,     2, {T_DVEC2,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC3,     2, {T_DVEC3,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC4,     2, {T_DVEC4,         T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC2,     2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "max", _EvaluateMax, _GenMaxCode, T_DVEC4,     2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC2,     3, {T_VEC2,         T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC3,     3, {T_VEC3,         T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC4,     3, {T_VEC4,         T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "clamp", _EvaluateClamp, _GenClampCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_INT,      3, {T_INT,          T_INT,      T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC2,     3, {T_IVEC2,       T_INT,      T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC3,     3, {T_IVEC3,       T_INT,      T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC4,     3, {T_IVEC4,       T_INT,      T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC2,     3, {T_IVEC2,       T_IVEC2,    T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC3,     3, {T_IVEC3,       T_IVEC3,    T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_IVEC4,     3, {T_IVEC4,       T_IVEC4,    T_IVEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UINT,      3, {T_UINT,        T_UINT,     T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC2,     3, {T_UVEC2,       T_UINT,     T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC3,     3, {T_UVEC3,       T_UINT,     T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC4,     3, {T_UVEC4,       T_UINT,     T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC2,     3, {T_UVEC2,       T_UVEC2,    T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC3,     3, {T_UVEC3,       T_UVEC3,    T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "clamp", _EvaluateClamp, _GenClampCode, T_UVEC4,     3, {T_UVEC4,       T_UVEC4,    T_UVEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC2,     3, {T_DVEC2,         T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC3,     3, {T_DVEC3,         T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC4,     3, {T_DVEC4,         T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "clamp", _EvaluateClamp, _GenClampCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DVEC4}, {0}, {0}},

#if !MIX_AS_INTRINCIS
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_FLOAT}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_FLOAT}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_FLOAT}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_FLOAT}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_VEC2}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_VEC3}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, _GenMixCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_VEC4}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_DOUBLE}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DOUBLE}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DOUBLE}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DOUBLE}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DVEC2}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DVEC3}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "mix", _EvaluateMix, _GenMixCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DVEC4}, {0}},
#endif

    {slvEXTENSION1_HALTI,    "mix", _EvaluateMix, _GenMixCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_BOOL}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "mix", _EvaluateMix, _GenMixCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "mix", _EvaluateMix, _GenMixCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "mix", _EvaluateMix, _GenMixCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_BVEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "mix", _EvaluateMix, _GenMixCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_BOOL}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "mix", _EvaluateMix, _GenMixCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "mix", _EvaluateMix, _GenMixCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "mix", _EvaluateMix, _GenMixCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_BVEC4}, {0}, {0}},

    /*ES31 integer mix */
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_INT,       3, {T_INT,         T_INT,      T_BOOL}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_IVEC2,     3, {T_IVEC2,       T_IVEC2,    T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_IVEC3,     3, {T_IVEC3,       T_IVEC3,    T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_IVEC4,     3, {T_IVEC4,       T_IVEC4,    T_BVEC4}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_UINT,      3, {T_UINT,        T_UINT,     T_BOOL}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_UVEC2,     3, {T_UVEC2,       T_UVEC2,    T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_UVEC3,     3, {T_UVEC3,       T_UVEC3,    T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_UVEC4,     3, {T_UVEC4,       T_UVEC4,    T_BVEC4}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_BOOL,      3, {T_BOOL,        T_BOOL,     T_BOOL}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_BVEC2,     3, {T_BVEC2,       T_BVEC2,    T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_BVEC3,     3, {T_BVEC3,       T_BVEC3,    T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_INTEGER_MIX,     "mix", _EvaluateMix, _GenMixCode, T_BVEC4,     3, {T_BVEC4,       T_BVEC4,    T_BVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acos",   _EvaluateAcos,   _GenAcosCode,   T_FLOAT,    1, {T_FLOAT}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acos",   _EvaluateAcos,   _GenAcosCode,   T_VEC2,     1, {T_VEC2}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acos",   _EvaluateAcos,   _GenAcosCode,   T_VEC3,     1, {T_VEC3}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acos",   _EvaluateAcos,   _GenAcosCode,   T_VEC4,     1, {T_VEC4}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acosh",  _EvaluateAcosh,  _GenAcoshCode,  T_FLOAT,    1, {T_FLOAT}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acosh",  _EvaluateAcosh,  _GenAcoshCode,  T_VEC2,     1, {T_VEC2}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acosh",  _EvaluateAcosh,  _GenAcoshCode,  T_VEC3,     1, {T_VEC3}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "acosh",  _EvaluateAcosh,  _GenAcoshCode,  T_VEC4,     1, {T_VEC4}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "asin",   _EvaluateAsin,   _GenAsinCode,   T_FLOAT,    1, {T_FLOAT}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "asin",   _EvaluateAsin,   _GenAsinCode,   T_VEC2,     1, {T_VEC2}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "asin",   _EvaluateAsin,   _GenAsinCode,   T_VEC3,     1, {T_VEC3}, {0}},
    {slvEXTENSION1_HALTI5_WITH_FMA_SUPPORT,     "asin",   _EvaluateAsin,   _GenAsinCode,   T_VEC4,     1, {T_VEC4}, {0}},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC2,     2, {T_VEC2,         T_VEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC4,     2, {T_VEC4,         T_VEC4}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC2,     2, {T_FLOAT,        T_VEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC3,     2, {T_FLOAT,        T_VEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_NONE,     "step", _EvaluateStep, _GenStepCode, T_VEC4,     2, {T_FLOAT,        T_VEC4}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DOUBLE,    2, {T_DOUBLE,        T_DOUBLE}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC2,     2, {T_DVEC2,         T_DVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC4,     2, {T_DVEC4,         T_DVEC4}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC2,     2, {T_DOUBLE,        T_DVEC2}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC3,     2, {T_DOUBLE,        T_DVEC3}, {0}, {0}, TREAT_F_AS_I},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "step", _EvaluateStep, _GenStepCode, T_DVEC4,     2, {T_DOUBLE,        T_DVEC4}, {0}, {0}, TREAT_F_AS_I},

    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_VEC4}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC2,     3, {T_FLOAT,        T_FLOAT,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC3,     3, {T_FLOAT,        T_FLOAT,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_VEC4,     3, {T_FLOAT,        T_FLOAT,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DVEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC2,     3, {T_DOUBLE,        T_DOUBLE,    T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC3,     3, {T_DOUBLE,        T_DOUBLE,    T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "smoothstep", _EvaluateSmoothStep, _GenSmoothStepCode, T_DVEC4,     3, {T_DOUBLE,        T_DOUBLE,    T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BOOL,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC4,     1, {T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BOOL,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isnan", _EvaluateIsNan, _GenIsNanCode, T_BVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BOOL,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC4,     1, {T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BOOL,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "isinf", _EvaluateIsInf, _GenIsInfCode, T_BVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "floatBitsToInt", _EvaluateFloatBitsToInteger, _GenFloatBitsToIntCode, T_INT,      1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToInt", _EvaluateFloatBitsToInteger, _GenFloatBitsToIntCode, T_IVEC2,    1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToInt", _EvaluateFloatBitsToInteger, _GenFloatBitsToIntCode, T_IVEC3,    1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToInt", _EvaluateFloatBitsToInteger, _GenFloatBitsToIntCode, T_IVEC4,    1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "floatBitsToUint", _EvaluateFloatBitsToInteger, _GenFloatBitsToUintCode, T_UINT,      1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToUint", _EvaluateFloatBitsToInteger, _GenFloatBitsToUintCode, T_UVEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToUint", _EvaluateFloatBitsToInteger, _GenFloatBitsToUintCode, T_UVEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "floatBitsToUint", _EvaluateFloatBitsToInteger, _GenFloatBitsToUintCode, T_UVEC4,     1, {T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "intBitsToFloat", _EvaluateIntegerBitsToFloat, _GenIntBitsToFloatCode, T_FLOAT,     1, {T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "intBitsToFloat", _EvaluateIntegerBitsToFloat, _GenIntBitsToFloatCode, T_VEC2,      1, {T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "intBitsToFloat", _EvaluateIntegerBitsToFloat, _GenIntBitsToFloatCode, T_VEC3,      1, {T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "intBitsToFloat", _EvaluateIntegerBitsToFloat, _GenIntBitsToFloatCode, T_VEC4,      1, {T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "uintBitsToFloat", _EvaluateIntegerBitsToFloat, _GenUintBitsToFloatCode, T_FLOAT,    1, {T_UINT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "uintBitsToFloat", _EvaluateIntegerBitsToFloat, _GenUintBitsToFloatCode, T_VEC2,     1, {T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "uintBitsToFloat", _EvaluateIntegerBitsToFloat, _GenUintBitsToFloatCode, T_VEC3,     1, {T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "uintBitsToFloat", _EvaluateIntegerBitsToFloat, _GenUintBitsToFloatCode, T_VEC4,     1, {T_UVEC4}, {0}, {0}},

    /* Geometric Functions */
    {slvEXTENSION1_NONE,     "length", _EvaluateLength, _GenLengthCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "length", _EvaluateLength, _GenLengthCode, T_FLOAT,    1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "length", _EvaluateLength, _GenLengthCode, T_FLOAT,    1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "length", _EvaluateLength, _GenLengthCode, T_FLOAT,    1, {T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "length", _EvaluateLength, _GenLengthCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "length", _EvaluateLength, _GenLengthCode, T_DOUBLE,    1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "length", _EvaluateLength, _GenLengthCode, T_DOUBLE,    1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "length", _EvaluateLength, _GenLengthCode, T_DOUBLE,    1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "distance", _EvaluateDistance, _GenDistanceCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "distance", _EvaluateDistance, _GenDistanceCode, T_FLOAT,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "distance", _EvaluateDistance, _GenDistanceCode, T_FLOAT,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "distance", _EvaluateDistance, _GenDistanceCode, T_FLOAT,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE, "distance", _EvaluateDistance, _GenDistanceCode, T_DOUBLE,    2, {T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "distance", _EvaluateDistance, _GenDistanceCode, T_DOUBLE,    2, {T_DVEC2,     T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "distance", _EvaluateDistance, _GenDistanceCode, T_DOUBLE,    2, {T_DVEC3,     T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE, "distance", _EvaluateDistance, _GenDistanceCode, T_DOUBLE,    2, {T_DVEC4,     T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "dot", _EvaluateDot, _GenDotCode, T_FLOAT,    2, {T_FLOAT,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "dot", _EvaluateDot, _GenDotCode, T_FLOAT,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "dot", _EvaluateDot, _GenDotCode, T_FLOAT,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "dot", _EvaluateDot, _GenDotCode, T_FLOAT,    2, {T_VEC4,         T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "dot", _EvaluateDot, _GenDotCode, T_DOUBLE,    2, {T_DOUBLE,        T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "dot", _EvaluateDot, _GenDotCode, T_DOUBLE,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "dot", _EvaluateDot, _GenDotCode, T_DOUBLE,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "dot", _EvaluateDot, _GenDotCode, T_DOUBLE,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "cross", _EvaluateCross, _GenCrossCode, T_VEC3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "cross", _EvaluateCross, _GenCrossCode, T_DVEC3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}},

    {slvEXTENSION1_NONE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_FLOAT,    1, {T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_VEC2,     1, {T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_VEC3,     1, {T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_VEC4,     1, {T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_DOUBLE,    1, {T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_DVEC2,     1, {T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_DVEC3,     1, {T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "normalize", _EvaluateNormalize, _GenNormalizeCode, T_DVEC4,     1, {T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_VEC4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "faceforward", _EvaluateFaceForward, _GenFaceForwardCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "refract", _EvaluateRefract, _GenRefractCode, T_FLOAT,    3, {T_FLOAT,        T_FLOAT,    T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "refract", _EvaluateRefract, _GenRefractCode, T_VEC2,     3, {T_VEC2,         T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "refract", _EvaluateRefract, _GenRefractCode, T_VEC3,     3, {T_VEC3,         T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_NONE,     "refract", _EvaluateRefract, _GenRefractCode, T_VEC4,     3, {T_VEC4,         T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "refract", _EvaluateRefract, _GenRefractCode, T_DOUBLE,    3, {T_DOUBLE,        T_DOUBLE,    T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "refract", _EvaluateRefract, _GenRefractCode, T_DVEC2,     3, {T_DVEC2,         T_DVEC2,     T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "refract", _EvaluateRefract, _GenRefractCode, T_DVEC3,     3, {T_DVEC3,         T_DVEC3,     T_DOUBLE}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "refract", _EvaluateRefract, _GenRefractCode, T_DVEC4,     3, {T_DVEC4,         T_DVEC4,     T_DOUBLE}, {0}, {0}},

    /* Matrix Functions */
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT2,     2, {T_MAT2,         T_MAT2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT3,     2, {T_MAT3,         T_MAT3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT4,     2, {T_MAT4,         T_MAT4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT2X3,   2, {T_MAT2X3,       T_MAT2X3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT3X2,   2, {T_MAT3X2,       T_MAT3X2}, {0}, {0}},

    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT2X4,   2, {T_MAT2X4,       T_MAT2X4}, {0}, {0}},
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT4X2,   2, {T_MAT4X2,       T_MAT4X2}, {0}, {0}},

    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT3X4,   2, {T_MAT3X4,       T_MAT3X4}, {0}, {0}},
    {slvEXTENSION1_NONE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_MAT4X3,   2, {T_MAT4X3,       T_MAT4X3}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT2,     2, {T_DMAT2,         T_DMAT2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT3,     2, {T_DMAT3,         T_DMAT3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT4,     2, {T_DMAT4,         T_DMAT4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT2X3,   2, {T_DMAT2X3,       T_DMAT2X3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT3X2,   2, {T_DMAT3X2,       T_DMAT3X2}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT2X4,   2, {T_DMAT2X4,       T_DMAT2X4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT4X2,   2, {T_DMAT4X2,       T_DMAT4X2}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT3X4,   2, {T_DMAT3X4,       T_DMAT3X4}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "matrixCompMult", _EvaluateMatrixCompMult, _GenMatrixCompMultCode, T_DMAT4X3,   2, {T_DMAT4X3,       T_DMAT4X3}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT2,     2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT3,     2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT4,     2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT2X3,   2, {T_VEC3,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT3X2,   2, {T_VEC2,         T_VEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT2X4,   2, {T_VEC4,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT4X2,   2, {T_VEC2,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT3X4,   2, {T_VEC4,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_MAT4X3,   2, {T_VEC3,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT2,     2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT3,     2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT4,     2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT2X3,   2, {T_DVEC3,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT3X2,   2, {T_DVEC2,         T_DVEC3}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT2X4,   2, {T_DVEC4,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT4X2,   2, {T_DVEC2,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT3X4,   2, {T_DVEC4,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "outerProduct", _EvaluateOuterProduct, _GenOuterProductCode, T_DMAT4X3,   2, {T_DVEC3,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT2,     1, {T_MAT2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT3,     1, {T_MAT3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT4,     1, {T_MAT4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT2X3,   1, {T_MAT3X2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT3X2,   1, {T_MAT2X3}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT2X4,   1, {T_MAT4X2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT4X2,   1, {T_MAT2X4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT3X4,   1, {T_MAT4X3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_MAT4X3,   1, {T_MAT3X4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT2,     1, {T_DMAT2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT3,     1, {T_DMAT3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT4,     1, {T_DMAT4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT2X3,   1, {T_DMAT3X2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT3X2,   1, {T_DMAT2X3}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT2X4,   1, {T_DMAT4X2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT4X2,   1, {T_DMAT2X4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT3X4,   1, {T_DMAT4X3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "transpose", _EvaluateTranspose, _GenTransposeCode, T_DMAT4X3,   1, {T_DMAT3X4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_FLOAT,    1, {T_MAT2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_FLOAT,    1, {T_MAT3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_FLOAT,    1, {T_MAT4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_DOUBLE,    1, {T_DMAT2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_DOUBLE,    1, {T_DMAT3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "determinant", _EvaluateDeterminant, _GenDeterminantCode, T_DOUBLE,    1, {T_DMAT4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "inverse", _EvaluateInverse, _GenInverseCode, T_MAT2,     1, {T_MAT2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "inverse", _EvaluateInverse, _GenInverseCode, T_MAT3,     1, {T_MAT3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "inverse", _EvaluateInverse, _GenInverseCode, T_MAT4,     1, {T_MAT4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "inverse", _EvaluateInverse, _GenInverseCode, T_DMAT2,     1, {T_DMAT2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "inverse", _EvaluateInverse, _GenInverseCode, T_DMAT3,     1, {T_DMAT3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "inverse", _EvaluateInverse, _GenInverseCode, T_DMAT4,     1, {T_DMAT4}, {0}, {0}},

    /* Vector Relational Functions */
    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThan", _EvaluateLessThan, _GenLessThanCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "lessThanEqual", _EvaluateLessThanEqual, _GenLessThanEqualCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThan", _EvaluateGreaterThan, _GenGreaterThanCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "greaterThanEqual", _EvaluateGreaterThanEqual, _GenGreaterThanEqualCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "equal", _EvaluateEqual, _GenEqualCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "equal", _EvaluateEqual, _GenEqualCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "equal", _EvaluateEqual, _GenEqualCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC2,    2, {T_BVEC2,        T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC3,    2, {T_BVEC3,        T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC4,    2, {T_BVEC4,        T_BVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "equal", _EvaluateEqual, _GenEqualCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC2,    2, {T_VEC2,         T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC3,    2, {T_VEC3,         T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC4,    2, {T_VEC4,         T_VEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC2,    2, {T_IVEC2,        T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC3,    2, {T_IVEC3,        T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC4,    2, {T_IVEC4,        T_IVEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,    "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC2,    2, {T_UVEC2,        T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC3,    2, {T_UVEC3,        T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,    "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC4,    2, {T_UVEC4,        T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC2,    2, {T_BVEC2,        T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC3,    2, {T_BVEC3,        T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC4,    2, {T_BVEC4,        T_BVEC4}, {0}, {0}},

    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC2,    2, {T_DVEC2,         T_DVEC2}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC3,    2, {T_DVEC3,         T_DVEC3}, {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "notEqual", _EvaluateNotEqual, _GenNotEqualCode, T_BVEC4,    2, {T_DVEC4,         T_DVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "any", _EvaluateAny, _GenAnyCode, T_BOOL,     1, {T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "any", _EvaluateAny, _GenAnyCode, T_BOOL,     1, {T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "any", _EvaluateAny, _GenAnyCode, T_BOOL,     1, {T_BVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "all", _EvaluateAll, _GenAllCode, T_BOOL,     1, {T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "all", _EvaluateAll, _GenAllCode, T_BOOL,     1, {T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "all", _EvaluateAll, _GenAllCode, T_BOOL,     1, {T_BVEC4}, {0}, {0}},

    {slvEXTENSION1_NONE,     "not", _EvaluateNot, _GenNotCode, T_BVEC2,    1, {T_BVEC2}, {0}, {0}},
    {slvEXTENSION1_NONE,     "not", _EvaluateNot, _GenNotCode, T_BVEC3,    1, {T_BVEC3}, {0}, {0}},
    {slvEXTENSION1_NONE,     "not", _EvaluateNot, _GenNotCode, T_BVEC4,    1, {T_BVEC4}, {0}, {0}},

    /* Texture Lookup Functions */
    {slvEXTENSION1_NON_HALTI,     "texture2D", gcvNULL, _GenTexture2DCode,            T_VEC4,     2, {T_SAMPLER2D,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     2, {T_SAMPLER2D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     2, {T_SAMPLER2D,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_NON_HALTI,     "textureCube", gcvNULL, _GenTextureCubeCode,          T_VEC4,     2, {T_SAMPLERCUBE,  T_VEC3}, {0}, {0}},

    {slvEXTENSION1_EGL_IMAGE_EXTERNAL,     "texture2D",     gcvNULL, _GenTexture2DCode,            T_VEC4,     2, {T_SAMPLEREXTERNALOES,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     2, {T_SAMPLEREXTERNALOES,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL,     "texture2DProj", gcvNULL, _GenTexture2DProjCode,        T_VEC4,     2, {T_SAMPLEREXTERNALOES,    T_VEC4}, {0}, {0}},

    {slvEXTENSION1_EGL_IMAGE_EXTERNAL_ESSL3,     "texture",     gcvNULL, _GenTextureCode,          T_VEC4,      2, {T_SAMPLEREXTERNALOES,   T_VEC2}, {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL_ESSL3,     "textureProj", gcvNULL, _GenTextureProjCode,      T_VEC4,      2, {T_SAMPLEREXTERNALOES,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL_ESSL3,     "textureProj", gcvNULL, _GenTextureProjCode,      T_VEC4,      2, {T_SAMPLEREXTERNALOES,   T_VEC4}, {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL_ESSL3,     "texelFetch",  gcvNULL, _GenTexelFetchCode,       T_VEC4,      3, {T_SAMPLEREXTERNALOES,   T_IVEC2, T_INT}, {0}, {0}},

    /* 3D Texture Lookup Functions */
    {slvEXTENSION1_TEXTURE_3D, "texture3D", gcvNULL, _GenTexture3DCode,          T_VEC4,     2, {T_SAMPLER3D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_3D, "texture3DProj", gcvNULL, _GenTexture3DProjCode,      T_VEC4,     2, {T_SAMPLER3D,    T_VEC4}, {0}, {0}},

    /* Texture array Lookup Functions */
    {slvEXTENSION1_TEXTURE_ARRAY, "texture1DArray", gcvNULL, _GenTexture1DArrayCode,  T_VEC4,     2, {T_SAMPLER1DARRAY,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "texture2DArray", gcvNULL, _GenTexture2DArrayCode,  T_VEC4,     2, {T_SAMPLER2DARRAY,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "shadow1DArray", gcvNULL, _GenShadow1DArrayCode,   T_VEC4,     2, {T_SAMPLER1DARRAYSHADOW,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_TEXTURE_ARRAY, "shadow2DArray", gcvNULL, _GenShadow2DArrayCode,   T_VEC4,     2, {T_SAMPLER2DARRAYSHADOW,   T_VEC4}, {0}, {0}},

    /* HALTI Texture array Lookup Functions */
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTexture1DCode,          T_VEC4,     2, {T_SAMPLER1D,        T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTexture1DArrayCode,     T_VEC4,     2, {T_SAMPLER1DARRAY,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     2, {T_SAMPLER2D,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     2, {T_SAMPLER3D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     2, {T_SAMPLERCUBE,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     2, {T_SAMPLERCUBEARRAY,  T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    2, {T_SAMPLER2DSHADOW,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    2, {T_SAMPLER2DRECTSHADOW,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    2, {T_SAMPLERCUBESHADOW, T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    3, {T_SAMPLERCUBEARRAYSHADOW, T_VEC4, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_VEC4,     2, {T_SAMPLER2DARRAY,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_FLOAT,    2, {T_SAMPLER2DARRAYSHADOW,   T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    2, {T_ISAMPLER2D,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    2, {T_ISAMPLER3D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    2, {T_ISAMPLERCUBE,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    2, {T_ISAMPLERCUBEARRAY,  T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_IVEC4,    2, {T_ISAMPLER2DARRAY,    T_VEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    2, {T_USAMPLER2D,    T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    2, {T_USAMPLER3D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    2, {T_USAMPLERCUBE,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    2, {T_USAMPLERCUBEARRAY,  T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture", gcvNULL, _GenTextureCode,            T_UVEC4,    2, {T_USAMPLER2DARRAY,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture",  gcvNULL, _GenTextureCode,           T_FLOAT,    2, {T_SAMPLER1DSHADOW,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture",  gcvNULL, _GenTextureCode,           T_FLOAT,    3, {T_SAMPLER1DSHADOW,    T_VEC3, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture",  gcvNULL, _GenTextureCode,           T_FLOAT,    2, {T_SAMPLER1DARRAYSHADOW, T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texture",  gcvNULL, _GenTextureCode,           T_FLOAT,    3, {T_SAMPLER1DARRAYSHADOW, T_VEC3, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     2, {T_SAMPLER2D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     2, {T_SAMPLER2D,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_VEC4,     2, {T_SAMPLER3D,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_FLOAT,    2, {T_SAMPLER2DSHADOW,   T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_FLOAT,    2, {T_SAMPLER2DRECTSHADOW,   T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    2, {T_ISAMPLER2D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    2, {T_ISAMPLER2D,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_IVEC4,    2, {T_ISAMPLER3D,    T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    2, {T_USAMPLER2D,    T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    2, {T_USAMPLER2D,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProj", gcvNULL, _GenTextureProjCode,        T_UVEC4,    2, {T_USAMPLER3D,    T_VEC4}, {0}, {0}},

    {slvEXTENSION1_HALTI,      "textureProj",  gcvNULL, _GenTextureCode,         T_FLOAT,    2, {T_SAMPLER1DSHADOW,    T_VEC4}, {0}, {0}},
    {slvEXTENSION1_HALTI,      "textureProj",  gcvNULL, _GenTextureCode,         T_FLOAT,    3, {T_SAMPLER1DSHADOW,    T_VEC4, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     3, {T_SAMPLER2D,    T_VEC2, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     3, {T_SAMPLER3D,    T_VEC3, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_FLOAT,    3, {T_SAMPLER2DSHADOW,   T_VEC3, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_FLOAT,    3, {T_SAMPLER2DRECTSHADOW,   T_VEC3, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_VEC4,     3, {T_SAMPLER2DARRAY,    T_VEC3, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC2, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    3, {T_ISAMPLER3D,    T_VEC3, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_IVEC4,    3, {T_ISAMPLER2DARRAY,    T_VEC3, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    3, {T_USAMPLER2D,    T_VEC2, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    3, {T_USAMPLER3D,    T_VEC3, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset", gcvNULL, _GenTextureOffsetCode,      T_UVEC4,    3, {T_USAMPLER2DARRAY,    T_VEC3, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureOffset",  gcvNULL, _GenTextureOffsetCode,     T_FLOAT,    3, {T_SAMPLER1DSHADOW,    T_VEC3, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset",  gcvNULL, _GenTextureOffsetCode,     T_FLOAT,    4, {T_SAMPLER1DSHADOW,    T_VEC3, T_INT, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset",  gcvNULL, _GenTextureOffsetCode,     T_FLOAT,    3, {T_SAMPLER1DARRAYSHADOW, T_VEC3, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureOffset",  gcvNULL, _GenTextureOffsetCode,     T_FLOAT,    4, {T_SAMPLER1DARRAYSHADOW, T_VEC3, T_INT, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_VEC4,     3, {T_SAMPLER2D,    T_VEC3, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_VEC4,     3, {T_SAMPLER2D,    T_VEC4, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_VEC4,     3, {T_SAMPLER3D,    T_VEC4, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_FLOAT,    3, {T_SAMPLER2DSHADOW, T_VEC4, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_FLOAT,    3, {T_SAMPLER2DRECTSHADOW, T_VEC4, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC3, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_IVEC4,    3, {T_ISAMPLER2D,    T_VEC4, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_IVEC4,    3, {T_ISAMPLER3D,    T_VEC4, T_IVEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_UVEC4,    3, {T_USAMPLER2D,    T_VEC3, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_UVEC4,    3, {T_USAMPLER2D,    T_VEC4, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset", gcvNULL, _GenTextureProjOffsetCode,  T_UVEC4,    3, {T_USAMPLER3D,    T_VEC4, T_IVEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjOffset",  gcvNULL, _GenTextureProjOffsetCode, T_FLOAT,    3, {T_SAMPLER1DSHADOW,    T_VEC4, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjOffset",  gcvNULL, _GenTextureProjOffsetCode, T_FLOAT,    4, {T_SAMPLER1DSHADOW,    T_VEC4, T_INT, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_VEC4,     4, {T_SAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_VEC4,     4, {T_SAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_VEC4,     4, {T_SAMPLERCUBE,          T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_VEC4,     4, {T_SAMPLERCUBEARRAY,     T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_FLOAT,    4, {T_SAMPLER2DSHADOW,      T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_FLOAT,    4, {T_SAMPLER2DRECTSHADOW,      T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_FLOAT,    4, {T_SAMPLERCUBESHADOW,    T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_VEC4,     4, {T_SAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_FLOAT,    4, {T_SAMPLER2DARRAYSHADOW, T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_IVEC4,    4, {T_ISAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_IVEC4,    4, {T_ISAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_IVEC4,    4, {T_ISAMPLERCUBE,          T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_IVEC4,    4, {T_ISAMPLERCUBEARRAY,     T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_IVEC4,    4, {T_ISAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_UVEC4,    4, {T_USAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_UVEC4,    4, {T_USAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_UVEC4,    4, {T_USAMPLERCUBE,          T_VEC3,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_UVEC4,    4, {T_USAMPLERCUBEARRAY,     T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad", gcvNULL, _GenTextureGradCode,        T_UVEC4,    4, {T_USAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad",  gcvNULL, _GenTextureGradCode,       T_FLOAT,    4, {T_SAMPLER1DSHADOW,    T_VEC3, T_FLOAT, T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGrad",  gcvNULL, _GenTextureGradCode,       T_FLOAT,    4, {T_SAMPLER1DARRAYSHADOW,    T_VEC3, T_FLOAT, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_VEC4,     5, {T_SAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_VEC4,     5, {T_SAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_FLOAT,    5, {T_SAMPLER2DSHADOW,      T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_FLOAT,    5, {T_SAMPLER2DRECTSHADOW,      T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_VEC4,     5, {T_SAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_FLOAT,    5, {T_SAMPLER2DARRAYSHADOW, T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_IVEC4,    5, {T_ISAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_IVEC4,    5, {T_ISAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_IVEC4,    5, {T_ISAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_UVEC4,    5, {T_USAMPLER2D,            T_VEC2,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_UVEC4,    5, {T_USAMPLER3D,            T_VEC3,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset", gcvNULL, _GenTextureGradOffsetCode,  T_UVEC4,    5, {T_USAMPLER2DARRAY,       T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset",  gcvNULL, _GenTextureGradOffsetCode, T_FLOAT,    5, {T_SAMPLER1DSHADOW,       T_VEC3, T_FLOAT, T_FLOAT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureGradOffset",  gcvNULL, _GenTextureGradOffsetCode, T_FLOAT,    5, {T_SAMPLER1DARRAYSHADOW,  T_VEC3, T_FLOAT, T_FLOAT, T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_VEC4,     4, {T_SAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_VEC4,     4, {T_SAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_VEC4,     4, {T_SAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_FLOAT,    4, {T_SAMPLER2DSHADOW,      T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_FLOAT,    4, {T_SAMPLER2DRECTSHADOW,      T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_FLOAT,    4, {T_SAMPLER1DSHADOW,      T_VEC4,  T_FLOAT, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_IVEC4,    4, {T_ISAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_IVEC4,    4, {T_ISAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_IVEC4,    4, {T_ISAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_UVEC4,    4, {T_USAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_UVEC4,    4, {T_USAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGrad", gcvNULL, _GenTextureProjGradCode,    T_UVEC4,    4, {T_USAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_VEC4,     5, {T_SAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_VEC4,     5, {T_SAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_VEC4,     5, {T_SAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_FLOAT,    5, {T_SAMPLER2DSHADOW,      T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_FLOAT,    5, {T_SAMPLER2DRECTSHADOW,      T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_FLOAT,    5, {T_SAMPLER1DSHADOW,      T_VEC4,  T_FLOAT, T_FLOAT, T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_IVEC4,    5, {T_ISAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_IVEC4,    5, {T_ISAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_IVEC4,    5, {T_ISAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_UVEC4,    5, {T_USAMPLER2D,            T_VEC3,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_UVEC4,    5, {T_USAMPLER2D,            T_VEC4,  T_VEC2,  T_VEC2,  T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjGradOffset", gcvNULL, _GenTextureProjGradOffsetCode,    T_UVEC4,    5, {T_USAMPLER3D,            T_VEC4,  T_VEC3,  T_VEC3,  T_IVEC3}, {0}, {0}},

    /* ES 3.1 Texture Lookup Functions */
    /* VIV internal function. */
    {slvEXTENSION1_ES_31,     "viv_texture", gcvNULL, _GenTextureCode,              T_VEC4,     1, {T_GEN_SAMPLER},  {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_texture", gcvNULL, _GenTextureCode,              T_IVEC4,    1, {T_GEN_ISAMPLER}, {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_texture", gcvNULL, _GenTextureCode,              T_UVEC4,    1, {T_GEN_USAMPLER}, {0}, {0}, HAS_VAR_ARG},

    {slvEXTENSION1_ES_31,     "viv_textureLod", gcvNULL, _GenTextureLodCode,           T_VEC4,     1, {T_GEN_SAMPLER},  {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureLod", gcvNULL, _GenTextureLodCode,           T_IVEC4,    1, {T_GEN_ISAMPLER}, {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureLod", gcvNULL, _GenTextureLodCode,           T_UVEC4,    1, {T_GEN_USAMPLER}, {0}, {0}, HAS_VAR_ARG},

    {slvEXTENSION1_ES_31,     "viv_textureGrad", gcvNULL, _GenTextureGradCode,          T_VEC4,     1, {T_GEN_SAMPLER},  {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureGrad", gcvNULL, _GenTextureGradCode,          T_IVEC4,    1, {T_GEN_ISAMPLER}, {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureGrad", gcvNULL, _GenTextureGradCode,          T_UVEC4,    1, {T_GEN_USAMPLER}, {0}, {0}, HAS_VAR_ARG},

    {slvEXTENSION1_ES_31,     "viv_textureGather", gcvNULL, _GenVivTextureGatherCode,        T_VEC4,     1, {T_GEN_SAMPLER},  {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureGather", gcvNULL, _GenVivTextureGatherCode,        T_IVEC4,    1, {T_GEN_ISAMPLER}, {0}, {0}, HAS_VAR_ARG},
    {slvEXTENSION1_ES_31,     "viv_textureGather", gcvNULL, _GenVivTextureGatherCode,        T_UVEC4,    1, {T_GEN_USAMPLER}, {0}, {0}, HAS_VAR_ARG},

    {slvEXTENSION1_ES_31,     "_viv_arrayLengthMethod", gcvNULL, _GenVivArrayLengthMethodCode,   T_INT,      0, {T_VOID}, {0}, {0}, HAS_VAR_ARG},

    /* HALTI texture lookup functions with explicit lod */
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_VEC4,     3, {T_SAMPLER2D,       T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_VEC4,     3, {T_SAMPLER3D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_FLOAT,    3, {T_SAMPLER2DSHADOW, T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_VEC4,     3, {T_SAMPLERCUBE,     T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_VEC4,     3, {T_SAMPLERCUBEARRAY,T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_VEC4,     3, {T_SAMPLER2DARRAY,  T_VEC3,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_IVEC4,    3, {T_ISAMPLER2D,       T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_IVEC4,    3, {T_ISAMPLER3D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_IVEC4,    3, {T_ISAMPLERCUBE,     T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_IVEC4,    3, {T_ISAMPLERCUBEARRAY,T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_IVEC4,    3, {T_ISAMPLER2DARRAY,  T_VEC3,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_UVEC4,    3, {T_USAMPLER2D,       T_VEC2,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_UVEC4,    3, {T_USAMPLER3D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_UVEC4,    3, {T_USAMPLERCUBE,     T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_UVEC4,    3, {T_USAMPLERCUBEARRAY,T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_UVEC4,    3, {T_USAMPLER2DARRAY,  T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_FLOAT,    3, {T_SAMPLER1DSHADOW,  T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLod", gcvNULL, _GenTextureLodCode,          T_FLOAT,    3, {T_SAMPLER1DARRAYSHADOW, T_VEC3, T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_VEC4,     3, {T_SAMPLER2D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_VEC4,     3, {T_SAMPLER2D,       T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_VEC4,     3, {T_SAMPLER3D,       T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_FLOAT,    3, {T_SAMPLER2DSHADOW, T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_FLOAT,    3, {T_SAMPLER2DRECTSHADOW, T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_FLOAT,    3, {T_SAMPLER1DSHADOW, T_VEC4,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_IVEC4,    3, {T_ISAMPLER2D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_IVEC4,    3, {T_ISAMPLER2D,       T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_IVEC4,    3, {T_ISAMPLER3D,       T_VEC4,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_UVEC4,    3, {T_USAMPLER2D,       T_VEC3,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_UVEC4,    3, {T_USAMPLER2D,       T_VEC4,     T_FLOAT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLod", gcvNULL, _GenTextureProjLodCode,      T_UVEC4,    3, {T_USAMPLER3D,       T_VEC4,     T_FLOAT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4,     3, {T_SAMPLER2D,       T_IVEC2,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4,     3, {T_SAMPLER3D,       T_IVEC3,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4,     3, {T_SAMPLER2DARRAY,  T_IVEC3,     T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    3, {T_ISAMPLER2D,       T_IVEC2,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    3, {T_ISAMPLER3D,       T_IVEC3,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    3, {T_ISAMPLER2DARRAY,  T_IVEC3,     T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    3, {T_USAMPLER2D,       T_IVEC2,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    3, {T_USAMPLER3D,       T_IVEC3,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    3, {T_USAMPLER2DARRAY,  T_IVEC3,     T_INT}, {0}, {0}},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4,     2, {T_SAMPLERBUFFER,      T_INT}, {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    2, {T_ISAMPLERBUFFER,     T_INT}, {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    2, {T_USAMPLERBUFFER,     T_INT}, {0}, {0}},

    /* OGL texelFetch functions with more sampler types */
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4 ,    3, {T_SAMPLER1D,        T_INT,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    3, {T_ISAMPLER1D,       T_INT,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    3, {T_USAMPLER1D,       T_INT,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_VEC4 ,    3, {T_SAMPLER1DARRAY,        T_IVEC2,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_IVEC4,    3, {T_ISAMPLER1DARRAY,       T_IVEC2,     T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetch", gcvNULL, _GenTexelFetchCode,          T_UVEC4,    3, {T_USAMPLER1DARRAY,       T_IVEC2,     T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_VEC4,     4, {T_SAMPLER2D,       T_IVEC2,     T_INT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_VEC4,     4, {T_SAMPLER3D,       T_IVEC3,     T_INT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_VEC4,     4, {T_SAMPLER2DARRAY,  T_IVEC3,     T_INT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_IVEC4,    4, {T_ISAMPLER2D,       T_IVEC2,     T_INT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_IVEC4,    4, {T_ISAMPLER3D,       T_IVEC3,     T_INT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_IVEC4,    4, {T_ISAMPLER2DARRAY,  T_IVEC3,     T_INT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_UVEC4,    4, {T_USAMPLER2D,       T_IVEC2,     T_INT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_UVEC4,    4, {T_USAMPLER3D,       T_IVEC3,     T_INT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,    T_UVEC4,    4, {T_USAMPLER2DARRAY,  T_IVEC3,     T_INT, T_IVEC2}, {0}, {0}},

    /* OGL texelFetchOffset functions with more sampler types */
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_VEC4 ,    4, {T_SAMPLER1D,  T_INT,       T_INT, T_INT},{0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_IVEC4,    4, {T_ISAMPLER1D, T_INT,       T_INT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_UVEC4,    4, {T_USAMPLER1D, T_INT,       T_INT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_VEC4 ,    4, {T_SAMPLER1DARRAY,  T_IVEC2,       T_INT, T_INT},{0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_IVEC4,    4, {T_ISAMPLER1DARRAY, T_IVEC2,       T_INT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "texelFetchOffset", gcvNULL, _GenTexelFetchOffsetCode,          T_UVEC4,    4, {T_USAMPLER1DARRAY, T_IVEC2,       T_INT, T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_VEC4,     4, {T_SAMPLER2D,       T_VEC2, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_VEC4,     4, {T_SAMPLER3D,       T_VEC3, T_FLOAT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_FLOAT,    4, {T_SAMPLER2DSHADOW, T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_FLOAT,    4, {T_SAMPLER1DSHADOW, T_VEC3, T_FLOAT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_FLOAT,    4, {T_SAMPLER1DARRAYSHADOW, T_VEC3, T_FLOAT, T_INT}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_VEC4,     4, {T_SAMPLER2DARRAY,  T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_IVEC4,     4, {T_ISAMPLER2D,       T_VEC2, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_IVEC4,     4, {T_ISAMPLER3D,       T_VEC3, T_FLOAT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_IVEC4,     4, {T_ISAMPLER2DARRAY,  T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_UVEC4,     4, {T_USAMPLER2D,       T_VEC2, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_UVEC4,     4, {T_USAMPLER3D,       T_VEC3, T_FLOAT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureLodOffset", gcvNULL, _GenTextureLodOffsetCode,    T_UVEC4,     4, {T_USAMPLER2DARRAY,  T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_VEC4,     4, {T_SAMPLER2D,       T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_VEC4,     4, {T_SAMPLER2D,       T_VEC4, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_VEC4,     4, {T_SAMPLER3D,       T_VEC4, T_FLOAT, T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_FLOAT,    4, {T_SAMPLER2DSHADOW, T_VEC4, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_FLOAT,    4, {T_SAMPLER1DSHADOW, T_VEC4, T_FLOAT, T_INT}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_IVEC4,    4, {T_ISAMPLER2D,       T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_IVEC4,    4, {T_ISAMPLER2D,       T_VEC4, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_IVEC4,    4, {T_ISAMPLER3D,       T_VEC4, T_FLOAT, T_IVEC3}, {0}, {0}},

    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_UVEC4,    4, {T_USAMPLER2D,       T_VEC3, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_UVEC4,    4, {T_USAMPLER2D,       T_VEC4, T_FLOAT, T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_HALTI,     "textureProjLodOffset", gcvNULL, _GenTextureProjLodOffsetCode,    T_UVEC4,    4, {T_USAMPLER3D,       T_VEC4, T_FLOAT, T_IVEC3}, {0}, {0}},

    /* ES 3.1 bit operation builtin functions */
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_INT,      1, {T_INT}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC2,    1, {T_IVEC2}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC3,    1, {T_IVEC3}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC4,    1, {T_IVEC4}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_INT,      1, {T_UINT}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC2,    1, {T_UVEC2}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC3,    1, {T_UVEC3}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "bitCount", gcvNULL, _GenBitCountCode, T_IVEC4,    1, {T_UVEC4}, {0}, {0}},

    {slvEXTENSION1_ES_31,     "atomicCounterIncrement", gcvNULL, _GenAtomicCounterCode, T_UINT,     1, {T_ATOMIC_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicCounterDecrement", gcvNULL, _GenAtomicCounterCode, T_UINT,     1, {T_ATOMIC_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicCounter", gcvNULL, _GenAtomicCounterCode,          T_UINT,     1, {T_ATOMIC_UINT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicAdd", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicAdd", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicMin", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicMin", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicMax", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicMax", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicAnd", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicAnd", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicOr", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicOr", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicXor", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicXor", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicExchange", gcvNULL, _GenAtomicOpCode, T_UINT,     2, {T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicExchange", gcvNULL, _GenAtomicOpCode, T_INT,      2, {T_INT, T_INT}, {0}, {0}, ATOMIC},

    {slvEXTENSION1_ES_31,     "atomicCompSwap", gcvNULL, _GenAtomicOpCode, T_UINT,     3, {T_UINT, T_UINT, T_UINT}, {0}, {0}, ATOMIC},
    {slvEXTENSION1_ES_31,     "atomicCompSwap", gcvNULL, _GenAtomicOpCode, T_INT,      3, {T_INT, T_INT, T_INT}, {0}, {0}, ATOMIC},

    /* barrier functions. */
    {slvEXTENSION1_ES_31,     "barrier", gcvNULL, _GenBarrierOpCode,                     T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "memoryBarrier", gcvNULL, _GenBarrierOpCode,               T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "memoryBarrierAtomicCounter", gcvNULL, _GenBarrierOpCode,  T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "memoryBarrierBuffer", gcvNULL, _GenBarrierOpCode,         T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "memoryBarrierImage", gcvNULL, _GenBarrierOpCode,          T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "memoryBarrierShared", gcvNULL, _GenBarrierOpCode,         T_VOID,     0, {T_VOID}, {0}, {0}},
    {slvEXTENSION1_ES_31,     "groupMemoryBarrier", gcvNULL, _GenBarrierOpCode,          T_VOID,     0, {T_VOID}, {0}, {0}},

};

static gctUINT CommonBuiltInFunctionCount =
                    sizeof(CommonBuiltInFunctions) / sizeof(slsBUILT_IN_FUNCTION);

static slsBUILT_IN_FUNCTION ExtensionBuiltInFunctions[] =
{
    {slvEXTENSION1_SHADOW_SAMPLER,     "shadow2DEXT", gcvNULL, _GenTextureCode,            T_FLOAT,    2, {T_SAMPLER2DSHADOW,   T_VEC3}, {0}, {0}},
    {slvEXTENSION1_SHADOW_SAMPLER,     "shadow2DProjEXT", gcvNULL, _GenTextureProjCode,        T_FLOAT,    2, {T_SAMPLER2DSHADOW,   T_VEC4}, {0}, {0}},
};

static gctUINT ExtensionBuiltInFunctionCount =
                    sizeof(ExtensionBuiltInFunctions) / sizeof(slsBUILT_IN_FUNCTION);

typedef struct _slsBUILTIN_FUNCTION_EXTENSION
{
    gctUINT                  extension;
    gctCONST_STRING          symbol;
}
slsBUILTIN_FUNCTION_EXTENSION;

static slsBUILTIN_FUNCTION_EXTENSION builtinFunctionExtensionTable[] =
{
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicAdd" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicMin" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicMax" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicAnd" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicOr" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicXor" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicExchange" },
    { slvEXTENSION1_IMAGE_ATOMIC, "imageAtomicCompSwap" },
};

static gctUINT builtinFunctionExtensionCount =
                sizeof(builtinFunctionExtensionTable) / sizeof(slsBUILTIN_FUNCTION_EXTENSION);

/*******************************Intrinsic functions*******************************/
/* builtin functions that used intrinsic mechanism to implement
   the old builtin function assume the parameters are only slvSTORAGE_QUALIFIER_IN.
   ES31 builtin functions has slvSTORAGE_QUALIFIER_OUT parameters. Thus adding
   paramQualifiers fields. */
typedef struct _slsINTRINSIC_BUILTIN_FUNCTION
{
    gctUINT                     extension;
    gctCONST_STRING             symbol;
    sltBUILT_IN_EVALUATE_FUNC_PTR   evaluate;
    sltBUILT_IN_GEN_CODE_FUNC_PTR   genCode;
    gctINT                      returnType;
    sltPRECISION_QUALIFIER      returnTypePrecision;
    gctUINT                     paramCount;
    gctINT                      paramTypes[slmMAX_BUILT_IN_PARAMETER_COUNT];
    sltSTORAGE_QUALIFIER        paramQualifiers[slmMAX_BUILT_IN_PARAMETER_COUNT];
    sltPRECISION_QUALIFIER      paramPrecisions[slmMAX_BUILT_IN_PARAMETER_COUNT];
    gceINTRINSICS_KIND          intrinsicKind;
    gctCONST_STRING             nameInLibrary;          /* "mangled" name inside the library */
    sltMEMORY_ACCESS_QUALIFIER  paramMemoryAccess[slmMAX_BUILT_IN_PARAMETER_COUNT];
    paramDataTypeConstructor    paramDataTypeConstructors[PARAMDATATYPECONSTRUCTORCOUNT];
    slsBuiltInFuncCheck         function;
    sltFUNC_FLAG                flags;
}
slsINTRINSIC_BUILTIN_FUNCTION;

#define TREAT_ANYP_AS_DP 0
#define _IN slvSTORAGE_QUALIFIER_IN
#define _OT slvSTORAGE_QUALIFIER_OUT
#if TREAT_ANYP_AS_DP
#define _DP slvPRECISION_QUALIFIER_DEFAULT
#define _HP slvPRECISION_QUALIFIER_HIGH
#define _MP slvPRECISION_QUALIFIER_MEDIUM
#define _LP slvPRECISION_QUALIFIER_LOW
#define ANY slvPRECISION_QUALIFIER_DEFAULT
#else
#define _DP slvPRECISION_QUALIFIER_DEFAULT
#define _HP slvPRECISION_QUALIFIER_HIGH
#define _MP slvPRECISION_QUALIFIER_MEDIUM
#define _LP slvPRECISION_QUALIFIER_LOW
#define ANY slvPRECISION_QUALIFIER_ANY
#endif

static slsINTRINSIC_BUILTIN_FUNCTION CommonIntrinsicBuiltInFunctions[] =
{
    {slvEXTENSION1_NONE,     "tan", _EvaluateTan, gcvNULL,             T_FLOAT,    ANY,  1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_tan_1", {0}, {0}},
    {slvEXTENSION1_NONE,     "tan", _EvaluateTan, gcvNULL,             T_VEC2,     ANY,  1, {T_VEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_tan_2", {0}, {0}},
    {slvEXTENSION1_NONE,     "tan", _EvaluateTan, gcvNULL,             T_VEC3,     ANY,  1, {T_VEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_tan_3", {0}, {0}},
    {slvEXTENSION1_NONE,     "tan", _EvaluateTan, gcvNULL,             T_VEC4,     ANY,  1, {T_VEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_tan_4", {0}, {0}},

    {slvEXTENSION1_NONE,     "asin", _EvaluateAsin, gcvNULL,           T_FLOAT,    ANY,  1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_asin_float", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "asin", _EvaluateAsin, gcvNULL,           T_VEC2,     ANY,  1, {T_VEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_asin_vec2", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "asin", _EvaluateAsin, gcvNULL,           T_VEC3,     ANY,  1, {T_VEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_asin_vec3", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "asin", _EvaluateAsin, gcvNULL,           T_VEC4,     ANY,  1, {T_VEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_asin_vec4", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},

    {slvEXTENSION1_NONE,     "acos", _EvaluateAcos, gcvNULL,           T_FLOAT,    ANY,  1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_acos_float", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acos", _EvaluateAcos, gcvNULL,           T_VEC2,     ANY,  1, {T_VEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_acos_vec2", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acos", _EvaluateAcos, gcvNULL,           T_VEC3,     ANY,  1, {T_VEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_acos_vec3", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acos", _EvaluateAcos, gcvNULL,           T_VEC4,     ANY,  1, {T_VEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_acos_vec4", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acosh", _EvaluateAcosh, gcvNULL,         T_FLOAT,    ANY,  1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_acosh_1", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acosh", _EvaluateAcosh, gcvNULL,         T_VEC2,     ANY,  1, {T_VEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_acosh_2", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acosh", _EvaluateAcosh, gcvNULL,         T_VEC3,     ANY,  1, {T_VEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_acosh_3", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},
    {slvEXTENSION1_NONE,     "acosh", _EvaluateAcosh, gcvNULL,         T_VEC4,     ANY,  1, {T_VEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_acosh_4", {0}, {0}, slFuncCheckForAtrigAsIntrinsic},

    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_FLOAT,    ANY,  1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_atan_float", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC2,     ANY,  1, {T_VEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_atan_vec2", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC3,     ANY,  1, {T_VEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_atan_vec3", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC4,     ANY,  1, {T_VEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_atan_vec4", {0}, {0}},

    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_FLOAT,    ANY,  2, {T_FLOAT, T_FLOAT}, {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_atan2_float", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC2,     ANY,  2, {T_VEC2, T_VEC2}, {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_atan2_vec2", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC3,     ANY,  2, {T_VEC3, T_VEC3}, {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_atan2_vec3", {0}, {0}},
    {slvEXTENSION1_NONE,     "atan", _EvaluateAtan, gcvNULL,           T_VEC4,     ANY,  2, {T_VEC4, T_VEC4}, {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_atan2_vec4", {0}, {0}},
#if MIX_AS_INTRINCIS
    /* genFType mix (genFType x, genFType y, float a); */
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_FLOAT,     ANY, 3, {T_FLOAT, T_FLOAT, T_FLOAT}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_float", {0}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC2,      ANY, 3, {T_VEC2, T_VEC2, T_FLOAT}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec2", {0}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC3,      ANY, 3, {T_VEC3, T_VEC3, T_FLOAT}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec3", {0}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC4,      ANY, 3, {T_VEC4, T_VEC4, T_FLOAT}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec4", {0}, {0}},
    /* genFType mix (genFType x, genFType y, genFType a); */
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC2,      ANY, 3, {T_VEC2, T_VEC2, T_VEC2}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec2_vec2", {0}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC3,      ANY, 3, {T_VEC3, T_VEC3, T_VEC3}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec3_vec3", {0}, {0}},
    {slvEXTENSION1_NONE,     "mix", _EvaluateMix, gcvNULL,             T_VEC4,      ANY, 3, {T_VEC4, T_VEC4, T_VEC4}, {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_mix_vec4_vec4", {0}, {0}},
#endif

    {slvEXTENSION1_HALTI,    "round", _EvaluateRound, gcvNULL,         T_FLOAT,  ANY,   1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_round_float", {0}, {0}},
    {slvEXTENSION1_HALTI,    "round", _EvaluateRound, gcvNULL,         T_VEC2,   ANY,   1, {T_VEC2},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "round", _EvaluateRound, gcvNULL,         T_VEC3,   ANY,   1, {T_VEC3},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec3", {0}, {0}},
    {slvEXTENSION1_HALTI,    "round", _EvaluateRound, gcvNULL,         T_VEC4,   ANY,   1, {T_VEC4},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec4", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "round", _EvaluateRound, gcvNULL,                 T_DOUBLE,  ANY,   1, {T_DOUBLE}, {_IN}, {ANY},gceINTRIN_source, "_viv_round_float", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "round", _EvaluateRound, gcvNULL,                 T_DVEC2,   ANY,   1, {T_DVEC2},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec2", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "round", _EvaluateRound, gcvNULL,                 T_DVEC3,   ANY,   1, {T_DVEC3},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec3", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "round", _EvaluateRound, gcvNULL,                 T_DVEC4,   ANY,   1, {T_DVEC4},  {_IN}, {ANY},gceINTRIN_source, "_viv_round_vec4", {0}, {0}},

    {slvEXTENSION1_HALTI,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_FLOAT, ANY,   1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_float", {0}, {0}},
    {slvEXTENSION1_HALTI,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_VEC2,  ANY,   1, {T_VEC2},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_VEC3,  ANY,   1, {T_VEC3},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec3", {0}, {0}},
    {slvEXTENSION1_HALTI,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_VEC4,  ANY,   1, {T_VEC4},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec4", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_DOUBLE, ANY,   1, {T_DOUBLE}, {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_float", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_DVEC2,  ANY,   1, {T_DVEC2},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec2", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_DVEC3,  ANY,   1, {T_DVEC3},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec3", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "roundEven", _EvaluateRoundEven, gcvNULL,             T_DVEC4,  ANY,   1, {T_DVEC4},  {_IN}, {ANY},gceINTRIN_source, "_viv_roundEven_vec4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_INT,  _LP,    1, {T_INT}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_1", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC2,_LP,    1, {T_IVEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC3,_LP,    1, {T_IVEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC4,_LP,    1, {T_IVEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_INT,  _LP,    1, {T_UINT}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_5", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC2,_LP,    1, {T_UVEC2}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_6", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC3,_LP,    1, {T_UVEC3}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_7", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findLSB", gcvNULL, gcvNULL,                T_IVEC4,_LP,    1, {T_UVEC4}, {_IN}, {ANY},gceINTRIN_source, "_viv_findLSB_8", {0}, {0}},

    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_INT,  _LP,    1, {T_INT}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_1", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC2,_LP,    1, {T_IVEC2}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC3,_LP,    1, {T_IVEC3}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC4,_LP,    1, {T_IVEC4}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_INT,  _LP,    1, {T_UINT}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_5", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC2,_LP,    1, {T_UVEC2}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_6", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC3,_LP,    1, {T_UVEC3}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_7", {0}, {0}},
    {slvEXTENSION1_ES_31,    "findMSB", gcvNULL, gcvNULL,                T_IVEC4,_LP,    1, {T_UVEC4}, {_IN}, {_HP},gceINTRIN_source, "_viv_findMSB_8", {0}, {0}},

    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_INT,  _HP,    1, {T_INT}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_1", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_IVEC2,_HP,    1, {T_IVEC2}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_IVEC3,_HP,    1, {T_IVEC3}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_IVEC4,_HP,    1, {T_IVEC4}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_UINT, _HP,    1, {T_UINT}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_5", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_UVEC2,_HP,    1, {T_UVEC2}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_6", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_UVEC3,_HP,    1, {T_UVEC3}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_7", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldReverse", gcvNULL, gcvNULL,        T_UVEC4,_HP,    1, {T_UVEC4}, {_IN}, {_HP},gceINTRIN_source, "_viv_bitfieldReverse_8", {0}, {0}},

    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_INT,  ANY,    3, {T_INT, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_int", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_IVEC2,ANY,    3, {T_IVEC2, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_ivec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_IVEC3,ANY,    3, {T_IVEC3, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_ivec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_IVEC4,ANY,    3, {T_IVEC4, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_ivec4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_UINT, ANY,    3, {T_UINT, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_UVEC2,ANY,    3, {T_UVEC2, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_uvec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_UVEC3,ANY,    3, {T_UVEC3, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_uvec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldExtract", gcvNULL, gcvNULL,        T_UVEC4,ANY,    3, {T_UVEC4, T_INT, T_INT},  {_IN,  _IN,  _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldExtract_uvec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_INT,  ANY,    4, {T_INT,   T_INT,   T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_int", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_IVEC2,ANY,    4, {T_IVEC2, T_IVEC2, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_ivec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_IVEC3,ANY,    4, {T_IVEC3, T_IVEC3, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_ivec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_IVEC4,ANY,    4, {T_IVEC4, T_IVEC4, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_ivec4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_UINT, ANY,    4, {T_UINT,  T_UINT,  T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_UVEC2,ANY,    4, {T_UVEC2, T_UVEC2, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_uvec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_UVEC3,ANY,    4, {T_UVEC3, T_UVEC3, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_uvec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "bitfieldInsert", gcvNULL, gcvNULL,         T_UVEC4,ANY,    4, {T_UVEC4, T_UVEC4, T_INT, T_INT},  {_IN,  _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_source, "_viv_bitfieldInsert_uvec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "uaddCarry", gcvNULL, gcvNULL,              T_UINT, _HP,    3, {T_UINT,  T_UINT,  T_UINT},  {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_uaddCarry_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "uaddCarry", gcvNULL, gcvNULL,              T_UVEC2,_HP,    3, {T_UVEC2, T_UVEC2, T_UVEC2},  {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_uaddCarry_uvec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "uaddCarry", gcvNULL, gcvNULL,              T_UVEC3,_HP,    3, {T_UVEC3, T_UVEC3, T_UVEC3},  {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_uaddCarry_uvec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "uaddCarry", gcvNULL, gcvNULL,              T_UVEC4,_HP,    3, {T_UVEC4, T_UVEC4, T_UVEC4},  {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_uaddCarry_uvec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "usubBorrow", gcvNULL, gcvNULL,             T_UINT, _HP,    3, {T_UINT,  T_UINT,  T_UINT},  {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_usubBorrow_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "usubBorrow", gcvNULL, gcvNULL,             T_UVEC2,_HP,    3, {T_UVEC2, T_UVEC2, T_UVEC2}, {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_usubBorrow_uvec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "usubBorrow", gcvNULL, gcvNULL,             T_UVEC3,_HP,    3, {T_UVEC3, T_UVEC3, T_UVEC3}, {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_usubBorrow_uvec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "usubBorrow", gcvNULL, gcvNULL,             T_UVEC4,_HP,    3, {T_UVEC4, T_UVEC4, T_UVEC4}, {_IN,  _IN,  _OT}, {_HP, _HP, _LP},gceINTRIN_source, "_viv_usubBorrow_uvec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "ldexp", gcvNULL, gcvNULL,                  T_FLOAT,_HP,     2, {T_FLOAT,  T_INT}, {_IN,  _IN}, {_HP, _HP},gceINTRIN_source, "_viv_ldexp_float", {0}, {0}},
    {slvEXTENSION1_ES_31,    "ldexp", gcvNULL, gcvNULL,                  T_VEC2, _HP,     2, {T_VEC2, T_IVEC2}, {_IN,  _IN}, {_HP, _HP},gceINTRIN_source, "_viv_ldexp_vec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "ldexp", gcvNULL, gcvNULL,                  T_VEC3, _HP,     2, {T_VEC3, T_IVEC3}, {_IN,  _IN}, {_HP, _HP},gceINTRIN_source, "_viv_ldexp_vec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "ldexp", gcvNULL, gcvNULL,                  T_VEC4, _HP,     2, {T_VEC4, T_IVEC4}, {_IN,  _IN}, {_HP, _HP},gceINTRIN_source, "_viv_ldexp_vec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "frexp", gcvNULL, gcvNULL,                  T_FLOAT,_HP,     2, {T_FLOAT,  T_INT}, {_IN,  _OT}, {_HP, _HP},gceINTRIN_source, "_viv_frexp_float", {0}, {0}},
    {slvEXTENSION1_ES_31,    "frexp", gcvNULL, gcvNULL,                  T_VEC2, _HP,     2, {T_VEC2, T_IVEC2}, {_IN,  _OT}, {_HP, _HP},gceINTRIN_source, "_viv_frexp_vec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "frexp", gcvNULL, gcvNULL,                  T_VEC3, _HP,     2, {T_VEC3, T_IVEC3}, {_IN,  _OT}, {_HP, _HP},gceINTRIN_source, "_viv_frexp_vec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "frexp", gcvNULL, gcvNULL,                  T_VEC4, _HP,     2, {T_VEC4, T_IVEC4}, {_IN,  _OT}, {_HP, _HP},gceINTRIN_source, "_viv_frexp_vec4", {0}, {0}},

    /* Floating-Point Pack and Unpack Functions for es3.0 */
    {slvEXTENSION1_HALTI,    "packUnorm2x16", _EvaluatePackUnorm2x16, gcvNULL,          T_UINT, _HP,     1, {T_VEC2},          {_IN}, {ANY},gceINTRIN_source, "_viv_packUnorm2x16_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "packSnorm2x16", _EvaluatePackSnorm2x16, gcvNULL,          T_UINT, _HP,     1, {T_VEC2},          {_IN}, {ANY},gceINTRIN_source, "_viv_packSnorm2x16_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "unpackUnorm2x16", _EvaluateUnpackUnorm2x16, gcvNULL,        T_VEC2, _HP,     1, {T_UINT},          {_IN}, {_HP},gceINTRIN_source, "_viv_unpackUnorm2x16_uint", {0}, {0}},
    {slvEXTENSION1_HALTI,    "unpackSnorm2x16", _EvaluateUnpackSnorm2x16, gcvNULL,        T_VEC2, _HP,     1, {T_UINT},          {_IN}, {_HP},gceINTRIN_source, "_viv_unpackSnorm2x16_uint", {0}, {0}},
    {slvEXTENSION1_HALTI,    "packHalf2x16", _EvaluatePackHalf2x16, gcvNULL,           T_UINT, _HP,     1, {T_VEC2},          {_IN}, {_MP},gceINTRIN_source, "_viv_packHalf2x16_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "unpackHalf2x16", _EvaluateUnpackHalf2x16, gcvNULL,         T_VEC2, _MP,     1, {T_UINT},          {_IN}, {_HP},gceINTRIN_source, "_viv_unpackHalf2x16_uint", {0}, {0}},

    /* Floating-Point Pack and Unpack Functions for es3.1 */
    {slvEXTENSION1_ES_31,    "packUnorm4x8", gcvNULL, gcvNULL,           T_UINT, _HP,     1, {T_VEC4},          {_IN}, {_MP},gceINTRIN_source, "_viv_packUnorm4x8_vec4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "packSnorm4x8", gcvNULL, gcvNULL,           T_UINT, _HP,     1, {T_VEC4},          {_IN}, {_MP},gceINTRIN_source, "_viv_packSnorm4x8_vec4", {0}, {0}},
    {slvEXTENSION1_ES_31,    "unpackUnorm4x8", gcvNULL, gcvNULL,         T_VEC4, _MP,     1, {T_UINT},          {_IN}, {_HP},gceINTRIN_source, "_viv_unpackUnorm4x8_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "unpackSnorm4x8", gcvNULL, gcvNULL,         T_VEC4, _MP,     1, {T_UINT},          {_IN}, {_HP},gceINTRIN_source, "_viv_unpackSnorm4x8_uint", {0}, {0}},

    {slvEXTENSION1_ES_31,    "umulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_UINT,  T_UINT,  T_UINT,  T_UINT},   {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_umulExtended_uint", {0}, {0}},
    {slvEXTENSION1_ES_31,    "umulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_UVEC2, T_UVEC2, T_UVEC2, T_UVEC2},  {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_umulExtended_uvec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "umulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_UVEC3, T_UVEC3, T_UVEC3, T_UVEC3},  {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_umulExtended_uvec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "umulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_UVEC4, T_UVEC4, T_UVEC4, T_UVEC4},  {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_umulExtended_uvec4", {0}, {0}},

    {slvEXTENSION1_ES_31,    "imulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_INT, T_INT, T_INT, T_INT},              {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_imulExtended_int", {0}, {0}},
    {slvEXTENSION1_ES_31,    "imulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_IVEC2, T_IVEC2, T_IVEC2, T_IVEC2},      {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_imulExtended_ivec2", {0}, {0}},
    {slvEXTENSION1_ES_31,    "imulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_IVEC3, T_IVEC3, T_IVEC3, T_IVEC3},      {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_imulExtended_ivec3", {0}, {0}},
    {slvEXTENSION1_ES_31,    "imulExtended", gcvNULL, gcvNULL,           T_VOID, _DP,     4, {T_IVEC4, T_IVEC4, T_IVEC4, T_IVEC4},      {_IN, _IN, _OT, _OT}, {_HP, _HP, _HP, _HP},gceINTRIN_source, "_viv_imulExtended_ivec4", {0}, {0}},

    {slvEXTENSION1_HALTI,    "modf", gcvNULL, gcvNULL,                   T_FLOAT,ANY,     2, {T_FLOAT, T_FLOAT},  {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_float", {0}, {0}},
    {slvEXTENSION1_HALTI,    "modf", gcvNULL, gcvNULL,                   T_VEC2, ANY,     2, {T_VEC2,  T_VEC2},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec2", {0}, {0}},
    {slvEXTENSION1_HALTI,    "modf", gcvNULL, gcvNULL,                   T_VEC3, ANY,     2, {T_VEC3,  T_VEC3},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec3", {0}, {0}},
    {slvEXTENSION1_HALTI,    "modf", gcvNULL, gcvNULL,                   T_VEC4, ANY,     2, {T_VEC4,  T_VEC4},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec4", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "modf", gcvNULL, gcvNULL,        T_DOUBLE,ANY,    2, {T_DOUBLE, T_DOUBLE},  {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_float", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "modf", gcvNULL, gcvNULL,        T_DVEC2, ANY,    2, {T_DVEC2,  T_DVEC2},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec2", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "modf", gcvNULL, gcvNULL,        T_DVEC3, ANY,    2, {T_DVEC3,  T_DVEC3},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec3", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,    "modf", gcvNULL, gcvNULL,        T_DVEC4, ANY,    2, {T_DVEC4,  T_DVEC4},   {_IN,  _OT}, {ANY, ANY},gceINTRIN_source, "_viv_modf_vec4", {0}, {0}},

    {slvEXTENSION1_GPU_SHADER5,  "fma", gcvNULL, gcvNULL,                 T_FLOAT,ANY,     3, {T_FLOAT, T_FLOAT, T_FLOAT},  {_IN,  _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_fma_float", {0}, {0}},
    {slvEXTENSION1_GPU_SHADER5,  "fma", gcvNULL, gcvNULL,                 T_VEC2, ANY,     3, {T_VEC2,  T_VEC2,  T_VEC2},   {_IN,  _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_fma_vec2", {0}, {0}},
    {slvEXTENSION1_GPU_SHADER5,  "fma", gcvNULL, gcvNULL,                 T_VEC3, ANY,     3, {T_VEC3,  T_VEC3,  T_VEC3},   {_IN,  _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_fma_vec3", {0}, {0}},
    {slvEXTENSION1_GPU_SHADER5,  "fma", gcvNULL, gcvNULL,                 T_VEC4, ANY,     3, {T_VEC4,  T_VEC4,  T_VEC4},   {_IN,  _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_source, "_viv_fma_vec4", {0}, {0}},

    {slvEXTENSION1_NONE,     "reflect", _EvaluateReflect, gcvNULL,                T_FLOAT,ANY,     2, {T_FLOAT, T_FLOAT},  {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_float", {0}, {0}},
    {slvEXTENSION1_NONE,     "reflect", _EvaluateReflect, gcvNULL,                T_VEC2, ANY,     2, {T_VEC2,  T_VEC2},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec2", {0}, {0}},
    {slvEXTENSION1_NONE,     "reflect", _EvaluateReflect, gcvNULL,                T_VEC3, ANY,     2, {T_VEC3,  T_VEC3},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec3", {0}, {0}},
    {slvEXTENSION1_NONE,     "reflect", _EvaluateReflect, gcvNULL,                T_VEC4, ANY,     2, {T_VEC4,  T_VEC4},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec4", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "reflect", _EvaluateReflect, gcvNULL,    T_DOUBLE,ANY,    2, {T_DOUBLE, T_DOUBLE},  {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_float", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "reflect", _EvaluateReflect, gcvNULL,    T_DVEC2, ANY,    2, {T_DVEC2,  T_DVEC2},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec2", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "reflect", _EvaluateReflect, gcvNULL,    T_DVEC3, ANY,    2, {T_DVEC3,  T_DVEC3},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec3", {0}, {0}},
    {slvEXTENSION1_DOUBLE_DATA_TYPE,     "reflect", _EvaluateReflect, gcvNULL,    T_DVEC4, ANY,    2, {T_DVEC4,  T_DVEC4},   {_IN,  _IN}, {ANY, ANY},gceINTRIN_source, "_viv_reflect_vec4", {0}, {0}},

    /* texture size function. */
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLER2D,    T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_SAMPLER3D,    T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_3D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLERCUBE,  T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_Cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_SAMPLERCUBEARRAY,  T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_CubeArray", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLER2DSHADOW,   T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2DShadow", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLERCUBESHADOW, T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_CubeShadow", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_SAMPLERCUBEARRAYSHADOW, T_INT},           {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_CubeArrayShadow", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_SAMPLER2DARRAY,    T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_Array", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_SAMPLER2DARRAYSHADOW,   T_INT},           {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_ArrayShadow", {0}, {0}},

    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_ISAMPLER2D,    T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_ISAMPLER3D,    T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_3D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_ISAMPLERCUBE,  T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_Cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_ISAMPLERCUBEARRAY,  T_INT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_CubeArray", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_ISAMPLER2DARRAY,   T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_Array", {0}, {0}},

    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_USAMPLER2D,    T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_USAMPLER3D,    T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_3D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_USAMPLERCUBE,  T_INT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_Cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_USAMPLERCUBEARRAY,  T_INT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_CubeArray", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC3,_HP,     2, {T_USAMPLER2DARRAY,   T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_Array", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     1, {T_SAMPLER2DMS},                             {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_MS", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     1, {T_ISAMPLER2DMS},                            {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_MS", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     1, {T_USAMPLER2DMS},                            {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_MS", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     1, {T_SAMPLER2DRECT},                           {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_MS", {0}, {0}}, /* TODO */
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     1, {T_SAMPLER2DRECTSHADOW},                     {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_MS", {0}, {0}}, /* TODO */


    {slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY, "textureSize", gcvNULL, gcvNULL,    T_IVEC3, _HP,   1, {T_SAMPLER2DMSARRAY},   {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_MSArray", {0}, {0}},
    {slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY, "textureSize", gcvNULL, gcvNULL,    T_IVEC3, _HP,   1, {T_ISAMPLER2DMSARRAY},  {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_MSArray", {0}, {0}},
    {slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY, "textureSize", gcvNULL, gcvNULL,    T_IVEC3, _HP,   1, {T_USAMPLER2DMSARRAY},  {_IN},   {ANY},               gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_MSArray", {0}, {0}},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,    1, {T_SAMPLERBUFFER},                    {_IN}, {ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_buffer", {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,    1, {T_ISAMPLERBUFFER},                   {_IN}, {ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_int_buffer", {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,    1, {T_USAMPLERBUFFER},                   {_IN}, {ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_uint_buffer", {0}, {0}},

    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,     2, {T_SAMPLER1D,       T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_1D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,     2, {T_ISAMPLER1D,      T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_1D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,     2, {T_USAMPLER1D,      T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_1D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_INT, _HP,     2, {T_SAMPLER1DSHADOW, T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_1D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2, _HP,   2, {T_SAMPLER1DARRAYSHADOW, T_INT},                 {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_1D", {0}, {0}},

    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLER2DRECT,  T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_ISAMPLER2DRECT, T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_USAMPLER2DRECT, T_INT},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureSize", gcvNULL, gcvNULL,            T_IVEC2,_HP,     2, {T_SAMPLER2DRECTSHADOW, T_INT},                {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},
    {slvEXTENSION1_EGL_IMAGE_EXTERNAL_ESSL3,     "textureSize", gcvNULL,  gcvNULL,    T_IVEC2,_HP,2, {T_SAMPLEREXTERNALOES,    T_INT},      {_IN, _IN}, {ANY, ANY},gceINTRIN_create_size_for_sampler, "_viv_textureSize_float_2D", {0}, {0}},

    /* texture gather functions. */
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   2, {T_SAMPLER2D,         T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_2D_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   2, {T_SAMPLER2DRECT,     T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_float_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   2, {T_SAMPLER2DARRAY,    T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   2, {T_SAMPLERCUBE,       T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_Cube_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2D,         T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_2D_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2DRECT,     T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_float_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2DARRAY,    T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_2DArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLERCUBE,       T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_Cube_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2DSHADOW,   T_VEC2,     T_FLOAT},   {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_2DShadow", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2DRECTSHADOW, T_VEC2,     T_FLOAT},   {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_2DRectShadow", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLER2DARRAYSHADOW,T_VEC3,   T_FLOAT},   {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_2DArrayShadow", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLERCUBESHADOW, T_VEC3,     T_FLOAT},   {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_CubeShadow", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   2, {T_ISAMPLER2D,        T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_2D_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   2, {T_ISAMPLER2DRECT,    T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_int_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   2, {T_ISAMPLER2DARRAY,   T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   2, {T_ISAMPLERCUBE,      T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_Cube_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   3, {T_ISAMPLER2D,        T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_2D_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   3, {T_ISAMPLER2DRECT,    T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_int_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   3, {T_ISAMPLER2DARRAY,   T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_2DArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   3, {T_ISAMPLERCUBE,      T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_Cube_WithComp", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   2, {T_USAMPLER2D,        T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_2D_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   2, {T_USAMPLER2DRECT,    T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_uint_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   2, {T_USAMPLER2DARRAY,   T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   2, {T_USAMPLERCUBE,      T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_Cube_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   3, {T_USAMPLER2D,        T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_2D_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   3, {T_USAMPLER2DRECT,    T_VEC2,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_2DRect, "_viv_textureGather_uint_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   3, {T_USAMPLER2DARRAY,   T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_2DArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   3, {T_USAMPLERCUBE,      T_VEC3,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_Cube_WithComp", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   2, {T_SAMPLERCUBEARRAY,       T_VEC4},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_CubeArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLERCUBEARRAY,       T_VEC4,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_CubeArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   2, {T_ISAMPLERCUBEARRAY,      T_VEC4},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_CubeArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_IVEC4, ANY,   3, {T_ISAMPLERCUBEARRAY,      T_VEC4,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_int_CubeArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   2, {T_USAMPLERCUBEARRAY,      T_VEC4},                {_IN, _IN}, {ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_CubeArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_UVEC4, ANY,   3, {T_USAMPLERCUBEARRAY,      T_VEC4,     T_INT},     {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_uint_CubeArray_WithComp", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGather", gcvNULL, gcvNULL,          T_VEC4,  ANY,   3, {T_SAMPLERCUBEARRAYSHADOW, T_VEC4,     T_FLOAT},   {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather, "_viv_textureGather_float_CubeArrayShadow", {0}, {0}},

    /* texture gather offset functions. */
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   3, {T_SAMPLER2D,             T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   3, {T_SAMPLER2DRECT,         T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_float_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   3, {T_SAMPLER2DARRAY,        T_VEC3, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2D,             T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2DRECT,         T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_float_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2DARRAY,        T_VEC3, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_2DArray_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2DSHADOW,       T_VEC2, T_FLOAT, T_IVEC2},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_2DShadow", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2DRECTSHADOW,   T_VEC2, T_FLOAT, T_IVEC2},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_float_2DRectShadow", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_VEC4,  ANY,   4, {T_SAMPLER2DARRAYSHADOW,  T_VEC3, T_FLOAT, T_IVEC2},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_float_2DArrayShadow", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2D,            T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_int_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2DRECT,        T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_int_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2DARRAY,       T_VEC3, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_int_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2D,            T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_int_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2DRECT,        T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_int_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2DARRAY,       T_VEC3, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_int_2DArray_WithComp", {0}, {0}},

    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2D,            T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_uint_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2DRECT,        T_VEC2, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_uint_2DRect_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2DARRAY,       T_VEC3, T_IVEC2},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_uint_2DArray_NoComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2D,            T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_uint_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2DRECT,        T_VEC2, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset_2DRect, "_viv_textureGatherOffset_uint_2DRect_WithComp", {0}, {0}},
    {slvEXTENSION1_ES_31,    "textureGatherOffset", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2DARRAY,       T_VEC3, T_IVEC2, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offset, "_viv_textureGatherOffset_uint_2DArray_WithComp", {0}, {0}},

    /* texture gather offsets functions. */
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    3, {T_SAMPLER2D,             T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    3, {T_SAMPLER2DRECT,         T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_float_2DRect_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    3, {T_SAMPLER2DARRAY,        T_VEC3, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_2DArray_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2D,             T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2DRECT,         T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_float_2DRect_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2DARRAY,        T_VEC3, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_2DArray_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2DSHADOW,       T_VEC2, T_FLOAT, T_TYPE_MATCH_CALLBACK0},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_2DShadow", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2DRECTSHADOW,   T_VEC2, T_FLOAT, T_TYPE_MATCH_CALLBACK0},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_float_2DRectShadow", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_VEC4, ANY,    4, {T_SAMPLER2DARRAYSHADOW,  T_VEC3, T_FLOAT, T_TYPE_MATCH_CALLBACK0},   {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_float_2DArrayShadow", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },

    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2D,            T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_int_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2DRECT,        T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_int_2DRect_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   3, {T_ISAMPLER2DARRAY,       T_VEC3, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_int_2DArray_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2D,            T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_int_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2DRECT,        T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_int_2DRect_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_IVEC4, ANY,   4, {T_ISAMPLER2DARRAY,       T_VEC3, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_int_2DArray_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },

    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2D,            T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_uint_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2DRECT,        T_VEC2, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_uint_2DRect_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   3, {T_USAMPLER2DARRAY,       T_VEC3, T_TYPE_MATCH_CALLBACK0},            {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_uint_2DArray_NoComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2D,            T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_uint_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2DRECT,        T_VEC2, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets_2DRect, "_viv_textureGatherOffsets_uint_2DRect_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },
    {slvEXTENSION1_GPU_SHADER5,  "textureGatherOffsets", gcvNULL, gcvNULL,    T_UVEC4, ANY,   4, {T_USAMPLER2DARRAY,       T_VEC3, T_TYPE_MATCH_CALLBACK0, T_INT},     {_IN, _IN, _IN, _IN}, {ANY, ANY, ANY, ANY},gceINTRIN_texture_gather_offsets, "_viv_textureGatherOffsets_uint_2DArray_WithComp", {0}, {slConstructIVEC2Array4}, slFuncCheckForTextureGatherOffsets },

    /* texelFetch function for MSAA. */
    {slvEXTENSION1_ES_31,    "texelFetch", gcvNULL, _GenTexelFetchCode,             T_VEC4,   ANY,  3, {T_SAMPLER2DMS,           T_IVEC2,     T_INT},                                        {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_float", {0}, {0}},
    {slvEXTENSION1_ES_31,    "texelFetch", gcvNULL, _GenTexelFetchCode,             T_IVEC4,  ANY,  3, {T_ISAMPLER2DMS,          T_IVEC2,     T_INT},                                        {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_int", {0}, {0}},
    {slvEXTENSION1_ES_31,    "texelFetch", gcvNULL, _GenTexelFetchCode,             T_UVEC4,  ANY,  3, {T_USAMPLER2DMS,          T_IVEC2,     T_INT},                                        {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_uint", {0}, {0}},

    {slvEXTENSION1_ES_31 | slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY,    "texelFetch", gcvNULL, _GenTexelFetchCode,    T_VEC4,  ANY,   3, {T_SAMPLER2DMSARRAY,    T_IVEC3,   T_INT},  {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_float_array", {0}, {0}},
    {slvEXTENSION1_ES_31 | slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY,    "texelFetch", gcvNULL, _GenTexelFetchCode,    T_IVEC4, ANY,   3, {T_ISAMPLER2DMSARRAY,   T_IVEC3,   T_INT},  {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_int_array", {0}, {0}},
    {slvEXTENSION1_ES_31 | slvEXTENSION1_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY,    "texelFetch", gcvNULL, _GenTexelFetchCode,    T_UVEC4,  ANY,  3, {T_USAMPLER2DMSARRAY,   T_IVEC3,   T_INT},  {_IN, _IN, _IN}, {ANY, ANY, ANY},gceINTRIN_texelFetch_for_MSAA, "_viv_texelFetch_uint_array", {0}, {0}},

    /* ES 3.1 Image Functions */
    /* Image size functions. */
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2, _HP,    1, {T_IMAGE2D},        {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_1", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IMAGE3D},        {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_2", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2,  _HP,   1, {T_IMAGECUBE},      {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_3", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IMAGE2DARRAY},   {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_4", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2, _HP,    1, {T_IIMAGE2D},       {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_5", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IIMAGE3D},       {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_6", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2, _HP,    1, {T_IIMAGECUBE},     {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_7", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IIMAGE2DARRAY},  {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_8", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2, _HP,    1, {T_UIMAGE2D},       {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_9", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_UIMAGE3D},       {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_10", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC2, _HP,    1, {T_UIMAGECUBE},     {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_11", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_UIMAGE2DARRAY},  {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_12", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IMAGECUBEARRAY}, {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_13", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_IIMAGECUBEARRAY},{_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_14", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},
    {slvEXTENSION1_ES_31,    "imageSize", gcvNULL, gcvNULL,               T_IVEC3, _HP,    1, {T_UIMAGECUBEARRAY},{_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_15", {slvMEMORY_ACCESS_QUALIFIER_READONLY|slvMEMORY_ACCESS_QUALIFIER_WRITEONLY}, {0}},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageSize", gcvNULL, gcvNULL,  T_INT,   _HP,    1, {T_IMAGEBUFFER},    {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_16", {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageSize", gcvNULL, gcvNULL,  T_INT,   _HP,    1, {T_IIMAGEBUFFER},   {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_17", {0}, {0}},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageSize", gcvNULL, gcvNULL,  T_INT,   _HP,    1, {T_UIMAGEBUFFER},   {_IN}, {_HP}, gceINTRIN_image_size, "_viv_image_size_18", {0}, {0}},


    /* Image load functions. */
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_VEC4,  _HP,    2, {T_IMAGE2D,       T_IVEC2}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_image_2d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_IVEC4, _HP,    2, {T_IIMAGE2D,      T_IVEC2}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimage_2d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_UVEC4, _HP,    2, {T_UIMAGE2D,      T_IVEC2}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimage_2d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_VEC4,  _HP,    2, {T_IMAGE3D,       T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_image_3d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_IVEC4, _HP,    2, {T_IIMAGE3D,      T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimage_3d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_UVEC4, _HP,    2, {T_UIMAGE3D,      T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimage_3d", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_VEC4,  _HP,    2, {T_IMAGECUBE,     T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_image_cube", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_IVEC4, _HP,    2, {T_IIMAGECUBE,    T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimage_cube", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_UVEC4, _HP,    2, {T_UIMAGECUBE,    T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimage_cube", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_VEC4,  _HP,    2, {T_IMAGE2DARRAY,  T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_image_2d_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_IVEC4, _HP,    2, {T_IIMAGE2DARRAY, T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimage_2d_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_UVEC4, _HP,    2, {T_UIMAGE2DARRAY, T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimage_2d_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_VEC4,  _HP,    2, {T_IMAGECUBEARRAY,  T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_image_cube_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_IVEC4, _HP,    2, {T_IIMAGECUBEARRAY, T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimage_cube_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageLoad", gcvNULL, gcvNULL,               T_UVEC4, _HP,    2, {T_UIMAGECUBEARRAY, T_IVEC3}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimage_cube_array", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageLoad", gcvNULL, gcvNULL,  T_VEC4,  _HP,    2, {T_IMAGEBUFFER, T_INT},  {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_imageBuffer", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageLoad", gcvNULL, gcvNULL,  T_IVEC4, _HP,    2, {T_IIMAGEBUFFER, T_INT}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_iimageBuffer", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageLoad", gcvNULL, gcvNULL,  T_UVEC4, _HP,    2, {T_UIMAGEBUFFER, T_INT}, {_IN, _IN}, {_HP, ANY},gceINTRIN_image_load, "_viv_image_load_uimageBuffer", {slvMEMORY_ACCESS_QUALIFIER_READONLY, 0}, {0}, gcvNULL, MEM_ACCESS},

     /* Image store function. */
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IMAGE2D, T_IVEC2, T_VEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_image_2d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IIMAGE2D, T_IVEC2, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimage_2d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_UIMAGE2D, T_IVEC2, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimage_2d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IMAGE3D, T_IVEC3, T_VEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_image_3d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IIMAGE3D, T_IVEC3, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimage_3d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_UIMAGE3D, T_IVEC3, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimage_3d", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IMAGECUBE, T_IVEC3, T_VEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_image_cube", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IIMAGECUBE, T_IVEC3, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimage_cube", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_UIMAGECUBE, T_IVEC3, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimage_cube", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IMAGE2DARRAY, T_IVEC3, T_VEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_image_2d_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimage_2d_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimage_2d_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IMAGECUBEARRAY, T_IVEC3, T_VEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_image_cube_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_IIMAGECUBEARRAY, T_IVEC3, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimage_cube_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageStore", gcvNULL, gcvNULL,              T_VOID,  _DP,   3, {T_UIMAGECUBEARRAY, T_IVEC3, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimage_cube_array", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageStore", gcvNULL, gcvNULL, T_VOID,  _DP,   3, {T_IMAGEBUFFER,  T_INT, T_VEC4},  {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_imageBuffer", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageStore", gcvNULL, gcvNULL, T_VOID,  _DP,   3, {T_IIMAGEBUFFER, T_INT, T_IVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_iimageBuffer", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,   "imageStore", gcvNULL, gcvNULL, T_VOID,  _DP,   3, {T_UIMAGEBUFFER, T_INT, T_UVEC4}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_store, "_viv_image_store_uimageBuffer", {slvMEMORY_ACCESS_QUALIFIER_WRITEONLY, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* Image atomic functions. */
    /* imageAtomicAddXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicAdd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_add_buffer_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicMinXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMin", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicMin", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicMin", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_min_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicMaxXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicMax", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicMax", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicMax", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_max_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicAnd */
    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicAnd", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_and_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicOrXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicOr", gcvNULL, gcvNULL,          T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicOr", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicOr", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_or_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicXorXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicXor", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicXor", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicXor", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xor_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicExchangeXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_INT,   _HP,   3, {T_IIMAGE2D, T_IVEC2, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_UINT,  _HP,   3, {T_UIMAGE2D, T_IVEC2, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_FLOAT, _HP,   3, {T_IMAGE2D, T_IVEC2, T_FLOAT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2D_float", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_INT,   _HP,   3, {T_IIMAGE3D, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_UINT,  _HP,   3, {T_UIMAGE3D, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_FLOAT, _HP,   3, {T_IMAGE3D, T_IVEC3, T_FLOAT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_3D_float", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_INT,   _HP,   3, {T_IIMAGECUBE, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_UINT,  _HP,   3, {T_UIMAGECUBE, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_FLOAT, _HP,   3, {T_IMAGECUBE, T_IVEC3, T_FLOAT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_CUBE_float", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_INT,   _HP,   3, {T_IIMAGE2DARRAY, T_IVEC3, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_UINT,  _HP,   3, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicExchange", gcvNULL, gcvNULL,    T_FLOAT, _HP,   3, {T_IMAGE2DARRAY, T_IVEC3, T_FLOAT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_2DARRAY_float", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicExchange", gcvNULL, gcvNULL,         T_INT,   _HP,   3, {T_IIMAGEBUFFER, T_INT, T_INT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicExchange", gcvNULL, gcvNULL,         T_UINT,  _HP,   3, {T_UIMAGEBUFFER, T_INT, T_UINT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicExchange", gcvNULL, gcvNULL,         T_FLOAT, _HP,   3, {T_IMAGEBUFFER, T_INT, T_FLOAT}, {_IN, _IN, _IN}, {_HP, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_xchg_buffer_float", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    /* imageAtomicCompSwapXXX */
    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_INT,   _HP,   4, {T_IIMAGE2D, T_IVEC2, T_INT, T_INT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_2D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_UINT,  _HP,   4, {T_UIMAGE2D, T_IVEC2, T_UINT, T_UINT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_2D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_INT,   _HP,   4, {T_IIMAGE3D, T_IVEC3, T_INT, T_INT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_3D_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_UINT,  _HP,   4, {T_UIMAGE3D, T_IVEC3, T_UINT, T_UINT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_3D_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_INT,   _HP,   4, {T_IIMAGECUBE, T_IVEC3, T_INT, T_INT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_CUBE_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_UINT,  _HP,   4, {T_UIMAGECUBE, T_IVEC3, T_UINT, T_UINT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_CUBE_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_INT,   _HP,   4, {T_IIMAGE2DARRAY, T_IVEC3, T_INT, T_INT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_2DARRAY_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_ES_31,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_UINT,  _HP,   4, {T_UIMAGE2DARRAY, T_IVEC3, T_UINT, T_UINT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_2DARRAY_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},

    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_INT,   _HP,   4, {T_IIMAGEBUFFER, T_INT, T_INT, T_INT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_buffer_int", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
    {slvEXTENSION1_EXT_TEXTURE_BUFFER,    "imageAtomicCompSwap", gcvNULL, gcvNULL,    T_UINT,  _HP,   4, {T_UIMAGEBUFFER, T_INT, T_UINT, T_UINT}, {_IN, _IN, _IN, _IN}, {_HP, ANY, ANY, ANY},gceINTRIN_image_atomic, "_viv_image_atomic_cmpxchg_buffer_uint", {slvMEMORY_ACCESS_QUALIFIER_COHERENT, 0, 0, 0}, {0}, gcvNULL, MEM_ACCESS},
};

static gctUINT CommonIntrinsicBuiltInFunctionCount =
                    sizeof(CommonIntrinsicBuiltInFunctions) / sizeof(slsINTRINSIC_BUILTIN_FUNCTION);

static slsINTRINSIC_BUILTIN_FUNCTION FSIntrinsicBuiltInFunctions[] =
{
    /* shader multiple-sample interpolation functions.*/
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtCentroid", gcvNULL, gcvNULL,    T_FLOAT, ANY,    1, {T_FLOAT}, {_IN}, {ANY},gceINTRIN_MS_interpolate_at_centroid, "_viv_interpolateAtCentroid_float", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtCentroid", gcvNULL, gcvNULL,    T_VEC2,  ANY,    1, {T_VEC2},  {_IN}, {ANY},gceINTRIN_MS_interpolate_at_centroid, "_viv_interpolateAtCentroid_vec2", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtCentroid", gcvNULL, gcvNULL,    T_VEC3,  ANY,    1, {T_VEC3},  {_IN}, {ANY},gceINTRIN_MS_interpolate_at_centroid, "_viv_interpolateAtCentroid_vec3", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtCentroid", gcvNULL, gcvNULL,    T_VEC4,  ANY,    1, {T_VEC4},  {_IN}, {ANY},gceINTRIN_MS_interpolate_at_centroid, "_viv_interpolateAtCentroid_vec4", {0}, {0}, slFuncCheckForInterpolate},

    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtSample", gcvNULL, gcvNULL,      T_FLOAT, ANY,    2, {T_FLOAT, T_INT}, {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_sample, "_viv_interpolateAtSample_float", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtSample", gcvNULL, gcvNULL,      T_VEC2,  ANY,    2, {T_VEC2, T_INT},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_sample, "_viv_interpolateAtSample_vec2", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtSample", gcvNULL, gcvNULL,      T_VEC3,  ANY,    2, {T_VEC3, T_INT},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_sample, "_viv_interpolateAtSample_vec3", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtSample", gcvNULL, gcvNULL,      T_VEC4,  ANY,    2, {T_VEC4, T_INT},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_sample, "_viv_interpolateAtSample_vec4", {0}, {0}, slFuncCheckForInterpolate},

    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtOffset", gcvNULL, gcvNULL,      T_FLOAT, ANY,    2, {T_FLOAT, T_VEC2}, {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_offset, "_viv_interpolateAtOffset_float", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtOffset", gcvNULL, gcvNULL,      T_VEC2,  ANY,    2, {T_VEC2, T_VEC2},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_offset, "_viv_interpolateAtOffset_vec2", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtOffset", gcvNULL, gcvNULL,      T_VEC3,  ANY,    2, {T_VEC3, T_VEC2},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_offset, "_viv_interpolateAtOffset_vec3", {0}, {0}, slFuncCheckForInterpolate},
    {slvEXTENSION1_SHADER_MULTISAMPLE_INTERPOLATION,     "interpolateAtOffset", gcvNULL, gcvNULL,      T_VEC4,  ANY,    2, {T_VEC4, T_VEC2},  {_IN, _IN}, {ANY, ANY},gceINTRIN_MS_interpolate_at_offset, "_viv_interpolateAtOffset_vec4", {0}, {0}, slFuncCheckForInterpolate},

    /* texture query lod function. */
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER1D,    T_FLOAT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLER1D,   T_FLOAT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLER1D,   T_FLOAT},                    {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER2D,    T_VEC2},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLER2D,   T_VEC2},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLER2D,   T_VEC2},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER3D,    T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_3d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLER3D,   T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_3d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLER3D,   T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_3d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLERCUBE,  T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLERCUBE, T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLERCUBE, T_VEC3},                     {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER1DARRAY,    T_FLOAT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLER1DARRAY,   T_FLOAT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLER1DARRAY,   T_FLOAT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER2DARRAY,    T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLER2DARRAY,   T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLER2DARRAY,   T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLERCUBEARRAY,  T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_ISAMPLERCUBEARRAY, T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_USAMPLERCUBEARRAY, T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER1DSHADOW,   T_FLOAT},               {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER2DSHADOW,   T_VEC2},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLERCUBESHADOW, T_VEC3},                {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER1DARRAYSHADOW,   T_FLOAT},          {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_1d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLER2DARRAYSHADOW,   T_VEC2},           {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_2d", {0}, {0}},
    {slvEXTENSION1_HALTI,    "textureQueryLod", gcvNULL, gcvNULL,            T_VEC2,_HP,     2, {T_SAMPLERCUBEARRAYSHADOW, T_VEC3},           {_IN, _IN}, {ANY, ANY},gceINTRIN_source, "_viv_image_query_lod_cube", {0}, {0}},
};

static gctUINT FSIntrinsicBuiltInFunctionCount =
                    sizeof(FSIntrinsicBuiltInFunctions) / sizeof(slsINTRINSIC_BUILTIN_FUNCTION);

#undef TREAT_ANYP_AS_DP
#undef _IN
#undef _OT
#undef _DP
#undef _HP
#undef _MP
#undef _LP
#undef ANY

#undef ATOMIC
#undef MEM_ACCESS
#undef HAS_VAR_ARG
#undef HAS_VOID_PARA
#undef INTRINSIC
#undef TREAT_F_AS_I

/*******************************Normal variables*******************************/
typedef gceSTATUS
(*sltBUILT_IN_VAR_UPDATE_FUNC_PTR)(
    IN sloCOMPILER,
    OUT slsDATA_TYPE**
    );

/* Built-In Variables */
typedef struct _slsBUILT_IN_VARIABLE
{
    gctUINT                         extension;
    gctCONST_STRING                 symbol;
    gctCONST_STRING                 implSymbol;
    sltPRECISION_QUALIFIER          precision;
    sltSTORAGE_QUALIFIER            qualifier;
    gctINT                          type;
    gctINT                          arrayLength;
    sltSTORAGE_QUALIFIER            implQualifier;
    struct _slsBUILT_IN_VARIABLE *  fieldVariables;
    gctCONST_STRING                 blockSymbol;
    gctUINT                         fieldCount;
    gctBOOL                         isUnSized;
    sltBUILT_IN_VAR_UPDATE_FUNC_PTR updateVarFunc;
}
slsBUILT_IN_VARIABLE;

gceSTATUS
updateForFragData(
    IN sloCOMPILER,
    OUT slsDATA_TYPE**
    );

gceSTATUS
updateForTexCoord(
    IN sloCOMPILER,
    OUT slsDATA_TYPE**
    );

gceSTATUS
updateForSampleMask(
    IN sloCOMPILER,
    OUT slsDATA_TYPE**
    );

gceSTATUS
updateForClipDistance(
    IN sloCOMPILER Compiler,
    OUT slsDATA_TYPE** DataType
    );

static slsBUILT_IN_VARIABLE PerVertexVariables[] =
{
    {slvEXTENSION1_NONE,  "gl_Position",              "#In.Position",      slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_VEC4,     0,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PointSize",             "#In.PointSize",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_FLOAT,    0,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static slsBUILT_IN_VARIABLE VSBuiltInVariables[] =
{
    {slvEXTENSION1_NONE,  gcvNULL,                    "#Out",              slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,   T_IO_BLOCK,   0,   slvSTORAGE_QUALIFIER_OUT_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Position",              "#Position",         slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PointSize",             "#PointSize",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_ClipDistance",          "#ClipDistance",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_FLOAT,    1,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE, updateForClipDistance},
    {slvEXTENSION1_HALTI, "gl_VertexID",              "#VertexID",         slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VERTEX_ID,    T_INT,      0,    slvSTORAGE_QUALIFIER_VERTEX_ID, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_HALTI, "gl_InstanceID",            "#InstanceID",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_INSTANCE_ID,  T_INT,      0,    slvSTORAGE_QUALIFIER_INSTANCE_ID, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_NONE,  "gl_DepthRange.near",       "#DepthRange.near",  slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange.far",        "#DepthRange.far",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange.diff",       "#DepthRange.diff",  slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange",            "#DepthRange",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_STRUCT,   (gctUINT)-1,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_NONE,  "gl_ModelViewProjectionMatrix",             "#ff_MVP_Matrix",      slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_MAT4,     0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Color",                 "#AttrColor",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_SecondaryColor",        "#AttrSecondaryColor",slvPRECISION_QUALIFIER_HIGH,   slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Normal",                "#Normal",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC3,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Vertex",                "#Vertex",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_FogCoord",              "#FogCoord",         slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord0",        "#MultiTexCoord0",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord1",        "#MultiTexCoord1",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord2",        "#MultiTexCoord2",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord3",        "#MultiTexCoord3",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord4",        "#MultiTexCoord4",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord5",        "#MultiTexCoord5",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord6",        "#MultiTexCoord6",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_MultiTexCoord7",        "#MultiTexCoord7",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_ATTRIBUTE,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_ATTRIBUTE, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_NONE,  "gl_FrontColor",          "#FrontColor",         slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_BackColor",           "#BackColor",          slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_FrontSecondaryColor", "#FrontSecondaryColor",slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_BackSecondaryColor",  "#BackSecondaryColor", slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_TexCoord",            "#TexCoord",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     8,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE, updateForTexCoord},
    {slvEXTENSION1_NONE,  "gl_FogFragCoord",        "#FogFragCoord",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static gctUINT VSBuiltInVariableCount =
                    sizeof(VSBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

static slsBUILT_IN_VARIABLE FSBuiltInVariables[] =
{
    {slvEXTENSION1_NONE,      "gl_FragCoord",        "#Position",    slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_FrontFacing",      "#FrontFacing", slvPRECISION_QUALIFIER_MEDIUM,   slvSTORAGE_QUALIFIER_VARYING_IN,    T_BOOL,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_ClipDistance",     "#ClipDistance", slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,    T_FLOAT,    1,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE, updateForClipDistance},
    {slvEXTENSION1_NON_HALTI, "gl_FragColor",        "#Color",       slvPRECISION_QUALIFIER_MEDIUM,   slvSTORAGE_QUALIFIER_FRAGMENT_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_FRAGMENT_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NON_HALTI, "gl_FragData",         "#Color",       slvPRECISION_QUALIFIER_MEDIUM,   slvSTORAGE_QUALIFIER_FRAGMENT_OUT,  T_VEC4,     1,    slvSTORAGE_QUALIFIER_FRAGMENT_OUT, gcvNULL, gcvNULL, 0, gcvFALSE, updateForFragData},
    {slvEXTENSION1_SHADER_FRAMEBUFFER_FETCH, "gl_LastFragData",     "#LastFragData",       slvPRECISION_QUALIFIER_MEDIUM,   slvSTORAGE_QUALIFIER_VARYING_IN,  T_VEC4,     1,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE, updateForFragData},
    {slvEXTENSION1_NONE,      "gl_PointCoord",       "#PointCoord",  slvPRECISION_QUALIFIER_MEDIUM,   slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC2,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_FRAG_DEPTH,"gl_FragDepthEXT",     "#Depth",       slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_FRAGMENT_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_FRAGMENT_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_HALTI | slvEXTENSION1_SUPPORT_OGL,     "gl_FragDepth",        "#Depth",       slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_FRAGMENT_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_FRAGMENT_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_NONE,      "gl_DepthRange.near",  "#DepthRange.near",slvPRECISION_QUALIFIER_HIGH,  slvSTORAGE_QUALIFIER_UNIFORM,       T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_DepthRange.far",   "#DepthRange.far", slvPRECISION_QUALIFIER_HIGH,  slvSTORAGE_QUALIFIER_UNIFORM,       T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_DepthRange.diff",  "#DepthRange.diff",slvPRECISION_QUALIFIER_HIGH,  slvSTORAGE_QUALIFIER_UNIFORM,       T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_DepthRange",       "#DepthRange",     slvPRECISION_QUALIFIER_HIGH,  slvSTORAGE_QUALIFIER_UNIFORM,       T_STRUCT,   (gctUINT)-1,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_NONE,      "gl_FrontColor",       "#FrontColor",  slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_BackColor",        "#BackColor",   slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_FrontSecondaryColor","#FrontSecondaryColor",slvPRECISION_QUALIFIER_HIGH, slvSTORAGE_QUALIFIER_VARYING_IN,  T_VEC4,  0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_BackSecondaryColor", "#BackSecondaryColor",slvPRECISION_QUALIFIER_HIGH,  slvSTORAGE_QUALIFIER_VARYING_IN,  T_VEC4,  0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_TexCoord",         "#TexCoord",    slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC4,     8,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE, updateForTexCoord},
    {slvEXTENSION1_NONE,      "gl_FogFragCoord",     "#FogFragCoord",slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_FLOAT,    0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_Color",            "#VaryingColor",slvPRECISION_QUALIFIER_HIGH,     slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,      "gl_SecondaryColor",   "#VaryingSecondaryColor",slvPRECISION_QUALIFIER_HIGH,slvSTORAGE_QUALIFIER_VARYING_IN,T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31,     "gl_HelperInvocation", "#HelperInvocation", slvPRECISION_QUALIFIER_MEDIUM, slvSTORAGE_QUALIFIER_VARYING_IN,T_BOOL,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},

    /* sample shading extension. */
    {slvEXTENSION1_SAMPLE_VARIABLES,     "gl_SampleID",       "#SampleID",       slvPRECISION_QUALIFIER_LOW,    slvSTORAGE_QUALIFIER_VARYING_IN,    T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_SAMPLE_VARIABLES,     "gl_SamplePosition", "#SamplePosition", slvPRECISION_QUALIFIER_MEDIUM, slvSTORAGE_QUALIFIER_VARYING_IN,    T_VEC2,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_SAMPLE_VARIABLES,     "gl_SampleMaskIn",   "#SampleMaskIn",   slvPRECISION_QUALIFIER_HIGH,   slvSTORAGE_QUALIFIER_VARYING_IN,    T_INT,      1,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE, updateForSampleMask},
    {slvEXTENSION1_SAMPLE_VARIABLES,     "gl_SampleMask",     "#SampleMask",     slvPRECISION_QUALIFIER_HIGH,   slvSTORAGE_QUALIFIER_VARYING_OUT,   T_INT,      1,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE, updateForSampleMask},
    {slvEXTENSION1_SAMPLE_VARIABLES,     "gl_NumSamples",     "#NumSamples",     slvPRECISION_QUALIFIER_LOW,    slvSTORAGE_QUALIFIER_UNIFORM,       T_INT,      0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},

    {slvEXTENSION1_EXT_GEOMETRY_SHADER,  "gl_PrimitiveID",           "#PrimitiveID",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,  T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_EXT_GEOMETRY_SHADER,  "gl_Layer",                 "#Layer",              slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,  T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static gctUINT FSBuiltInVariableCount =
                    sizeof(FSBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

static slsBUILT_IN_VARIABLE CSBuiltInVariables[] =
{
    {slvEXTENSION1_ES_31, "gl_NumWorkGroups",        "#num_groups",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_UVEC3,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31, "gl_WorkGroupSize",        "#WorkGroupSize",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_NONE,         T_UVEC3,    0,    slvSTORAGE_QUALIFIER_CONST, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31, "gl_WorkGroupID",          "#group_id",             slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_UVEC3,    0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31, "gl_LocalInvocationID",    "#local_id",             slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_UVEC3,    0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31, "gl_GlobalInvocationID",   "#global_id",            slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_UVEC3,    0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_ES_31, "gl_LocalInvocationIndex", "#LocalInvocationIndex", slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_NONE,         T_UINT,     0,    slvSTORAGE_QUALIFIER_NONE, gcvNULL, gcvNULL, 0, gcvFALSE}
};

static gctUINT CSBuiltInVariableCount =
                    sizeof(CSBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

static slsBUILT_IN_VARIABLE TCSBuiltInVariables[] =
{
    /* input. */
    {slvEXTENSION1_NONE,  "gl_in",                    "#In",                 slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK,   T_IO_BLOCK,   32,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_Position",        "#In_Position",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_VEC4,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_PointSize",       "#In_PointSize",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_FLOAT,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PatchVerticesIn",       "#TcsPatchVerticesIn", slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM   ,   T_INT,      0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PrimitiveID",           "#PrimitiveID",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_InvocationID",          "#InvocationID",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    /* output. */
    {slvEXTENSION1_NONE,  "gl_TessLevelOuter",        "#TessLevelOuter",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT,  T_FLOAT,    4,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_TessLevelInner",        "#TessLevelInner",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT,  T_FLOAT,    2,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_out",                   "#Out",                slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,  T_IO_BLOCK,   32,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvTRUE},
    {slvEXTENSION1_NONE,  "gl_out.gl_Position",       "#Position",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER,   T_VEC4,   0,   slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_out.gl_PointSize",      "#PointSize",          slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER,   T_FLOAT,   0,   slvSTORAGE_QUALIFIER_OUT_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    /* a special patch output for GL_EXT_primitive_bounding_box. */
    {slvEXTENSION1_EXT_PRIMITIVE_BOUNDING_BOX,  "gl_BoundingBoxEXT",           "#BoundingBox",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT,  T_VEC4,    2,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_EXT_PRIMITIVE_BOUNDING_BOX,  "gl_BoundingBox",              "#BoundingBox",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_OUT,  T_VEC4,    2,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static gctUINT TCSBuiltInVariableCount =
                    sizeof(TCSBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

static slsBUILT_IN_VARIABLE TESBuiltInVariables[] =
{
    /* input. */
    {slvEXTENSION1_NONE,  "gl_in",                    "#In",                 slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK,   T_IO_BLOCK,   32,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_Position",        "#In_Position",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_VEC4,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_PointSize",       "#In_PointSize",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_FLOAT,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PatchVerticesIn",       "#TesPatchVerticesIn", slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_INT,      0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PrimitiveID",           "#PrimitiveID",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_TessCoord",             "#TessCoord",          slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_VEC3,     0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_TessLevelOuter",        "#TessLevelOuter",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_IN,   T_FLOAT,    4,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_TessLevelInner",        "#TessLevelInner",     slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_PATCH_IN,   T_FLOAT,    2,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    /* output. */
    {slvEXTENSION1_NONE,  gcvNULL,                    "#Out",                slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,   T_IO_BLOCK,   0,   slvSTORAGE_QUALIFIER_OUT_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Position",              "#Position",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PointSize",             "#PointSize",          slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static gctUINT TESBuiltInVariableCount =
                    sizeof(TESBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

static slsBUILT_IN_VARIABLE GSBuiltInVariables[] =
{
    /* input. */
    {slvEXTENSION1_NONE,  "gl_in",                    "#In",                 slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK,   T_IO_BLOCK,   -1,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_Position",        "#In_Position",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_VEC4,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_in.gl_PointSize",       "#In_PointSize",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER,   T_FLOAT,   0,   slvSTORAGE_QUALIFIER_IN_IO_BLOCK_MEMBER, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PrimitiveIDIn",         "#PrimitiveIDIn",      slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_InvocationID",          "#InvocationID",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_IN,   T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_IN, gcvNULL, gcvNULL, 0, gcvFALSE},
    /* output. */
    {slvEXTENSION1_NONE,  gcvNULL,                    "#Out",                slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_OUT_IO_BLOCK,   T_IO_BLOCK,   0,   slvSTORAGE_QUALIFIER_OUT_IO_BLOCK, PerVertexVariables, "gl_PerVertex", 2, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Position",              "#Position",           slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_VEC4,     0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PointSize",             "#PointSize",          slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_FLOAT,    0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_PrimitiveID",           "#PrimitiveID",        slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_Layer",                 "#Layer",              slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_VARYING_OUT,  T_INT,      0,    slvSTORAGE_QUALIFIER_VARYING_OUT, gcvNULL, gcvNULL, 0, gcvFALSE},
    /* uniform state */
    {slvEXTENSION1_NONE,  "gl_DepthRange.near",       "#DepthRange.near",  slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange.far",        "#DepthRange.far",   slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange.diff",       "#DepthRange.diff",  slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_FLOAT,    0,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
    {slvEXTENSION1_NONE,  "gl_DepthRange",            "#DepthRange",       slvPRECISION_QUALIFIER_HIGH,    slvSTORAGE_QUALIFIER_UNIFORM,      T_STRUCT,   (gctUINT)-1,    slvSTORAGE_QUALIFIER_UNIFORM, gcvNULL, gcvNULL, 0, gcvFALSE},
};

static gctUINT GSBuiltInVariableCount =
                    sizeof(GSBuiltInVariables) / sizeof(slsBUILT_IN_VARIABLE);

#endif /* __gc_glsl_built_ins_def_h_ */
