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


#ifndef __gc_vsc_vir_lower_common_func_
#define __gc_vsc_vir_lower_common_func_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

void
VIR_Lower_Initialize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternLowerContext  *Context,
    IN VSC_HW_CONFIG            *HwCfg,
    IN VSC_MM                   *pMM
    );

VSC_ErrCode
VIR_Lower_ArraryIndexing_To_LDARR_STARR(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    INOUT gctBOOL               *bChanged
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Scalar(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine_Pre(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    );

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine_Post(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    );

gctBOOL
VIR_Lower_SetLongUlongDestTypeOnly(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetLongUlongSecondDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndNeg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSmallestPositive(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetZeroOrSamllestPositive(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetIntZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetIntHighBitOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetMinusOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetIntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetUIntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetIntMinusOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableXAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableZAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableXYZAndInt3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableXYZWAndSymType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_AdjustCoordSwizzleForShadow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleXYZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleByCoord(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleXYZW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleXEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleYEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleZEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleWEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

VIR_PrimitiveTypeId
VIR_Lower_GetBaseType(
    IN VIR_Shader  *Shader,
    IN VIR_Operand *Opnd
    );

gctBOOL
VIR_Lower_IsIntOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsLongOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstOneEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstTwoEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstThreeEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstFourEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstMoreThanOneEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstBool(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstFP16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstInt32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstIntPacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstNotIntPacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstSigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstUnsigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstMediumpOrLowp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstHighp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsDstInt16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsFloatOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsSrc0Unsigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsSrc1FloatConstant(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsNotCLShader(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_InstSupportFP16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_HasHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_HasNoHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_SetImm0xFFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
);

gctBOOL
VIR_Lower_SetImm16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
);

gctBOOL
VIR_Lower_SetImm0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
);

gctBOOL
VIR_Lower_SetSwizzleXAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleZAndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleZAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleXIndex_1AndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleYIndex_1AndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetSwizzleZIndex_1AndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_enableFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_disableFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_HasTexldModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_IsI2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_SameSizeType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_NotSameSizeType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_SameType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_label_set_jmp_n(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctINT32             n);

gctBOOL
VIR_Lower_label_set_jmp_neg2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_label_set_jmp_neg3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_label_set_jmp_neg3_6(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_label_set_jmp_neg10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_label_set_jmp_neg22(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndUINT32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndUINT32HP(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndHP(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndINT32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndUINT16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndINT16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetOpndFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableBaseOnSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableYAndSrc0Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetEnableBaseOnSrc1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

VIR_TexModifier_Flag
VIR_Lower_GetTexModifierKind(
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetLongUlongInstType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetPrecisionBaseOnSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_SetHighp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_ReverseCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_ResetCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
VIR_Lower_ChangeSignedIntegerToUnsignedInteger(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

VSC_ErrCode
VIR_Lower_ChangeOperandByOffset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction   *Inst,
    IN VIR_Operand       *Opnd,
    IN gctUINT           rowOffset
    );

gctBOOL
VIR_Lower_MatchDual16Req(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *DestOpnd,
    IN VIR_Operand        *SrcOpnd
    );

/* JMP check functions. */
gctBOOL
VIR_Lower_label_only_one_jmp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_jmp_2_succ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN gctUINT            seq
    );

gctBOOL
VIR_Lower_jmp_2_succ2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_jmp_2_succ3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_jmp_2_succ4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

END_EXTERN_C()
#endif



