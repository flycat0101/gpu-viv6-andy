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


#ifndef __gc_vsc_vir_lower_common_func_
#define __gc_vsc_vir_lower_common_func_

#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_pattern.h"

BEGIN_EXTERN_C()

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
VIR_Lower_SetEnableW(
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
VIR_Lower_IsDstFloat(
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
VIR_Lower_IsNotCLShader(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

gctBOOL
VIR_Lower_HasHalt4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
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
VIR_Lower_label_set_jmp_n(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctINT32           n
    );

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
VIR_Lower_SetEnableBaseOnSrc1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

VIR_TexModifier_Flag
VIR_Lower_GetTexModifierKind(
    IN VIR_Operand        *Opnd
    );

END_EXTERN_C()
#endif



