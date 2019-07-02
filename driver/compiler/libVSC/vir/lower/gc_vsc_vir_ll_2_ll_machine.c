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


#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

#include "vir/lower/gc_vsc_vir_lower_common_func.h"

static gctBOOL
_hasNEW_SIN_COS_LOG_DIV(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasNewSinCosLogDiv;
}

static gctBOOL
_SetUIntFloatFour(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0xf0000004,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_hasNot32IntDIV(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.support32BitIntDiv);
}

static gctBOOL
_Lower_label_set_jmp_neg21(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -21);
}

static gctBOOL
_adjustPrecisionByNextInstDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction     *nextInst = VIR_Inst_GetNext(Inst);
    VIR_Operand         *nextDest;

    gcmASSERT(nextInst && VIR_Inst_GetDest(nextInst));

    nextDest = VIR_Inst_GetDest(nextInst);
    if (VIR_Operand_GetPrecision(nextDest) == VIR_PRECISION_HIGH)
    {
        return VIR_Lower_SetHighp(Context, Inst, Opnd);
    }
    return gcvTRUE;
}

static gctBOOL
_Is32BitSignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_IsIntOpcode(Context, Inst) && VIR_Lower_IsDstInt32(Context, Inst) && VIR_Lower_IsDstSigned(Context, Inst);
}

static gctBOOL
_Is32BitUnsignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_IsIntOpcode(Context, Inst) && VIR_Lower_IsDstInt32(Context, Inst) && VIR_Lower_IsDstUnsigned(Context, Inst);
}

static gctBOOL
_set_RCP_value(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT((VIR_Operand_isImm(Opnd) || VIR_Operand_isConst(Opnd)));

    gcmASSERT((VIR_GetTypeFlag(VIR_Operand_GetTypeId(Opnd)) & VIR_TYFLAG_ISFLOAT) != 0 );

    if (VIR_Operand_isImm(Opnd))
    {
        VIR_Operand_SetImmFloat(Opnd, 1.0f / VIR_Operand_GetImmediateFloat(Opnd));
    }
    else
    {
        VIR_Const    *pConst = VIR_Shader_GetConstFromId(Context->shader, VIR_Operand_GetConstId(Opnd));
        VIR_ConstVal  rcpConstVal;
        VIR_ConstId   rcpConstId;
        gctUINT       componentCount = VIR_GetTypeComponents(pConst->type);

        gcmASSERT(VIR_TypeId_isPrimitive(pConst->type) &&
                  componentCount > 0 && componentCount < VIR_CONST_MAX_CHANNEL_COUNT);
        if (componentCount == 1)
        {
            rcpConstVal.scalarVal.fValue = 1.0f / pConst->value.scalarVal.fValue;
        }
        else
        {
            gctUINT i;
            for (i = 0; i < componentCount; i++)
            {
                rcpConstVal.vecVal.f32Value[i] = 1.0f / pConst->value.vecVal.f32Value[i];
            }
        }
        VIR_Shader_AddConstant(Context->shader, pConst->type, &rcpConstVal, &rcpConstId);
        VIR_Operand_SetConstId(Opnd, rcpConstId);
    }
    return gcvTRUE;
}

static VIR_PatternMatchInst _divPatInst0[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsSrc1FloatConstant }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst0[] = {
    { VIR_OP_MUL, 0, 0, {  1, 2, 3, 0 }, { 0, 0, _set_RCP_value } },
};


static VIR_PatternMatchInst _divPatInst1[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNEW_SIN_COS_LOG_DIV, VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst1[] = {
    { VIR_OP_PRE_DIV, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 3, 0 }, { _adjustPrecisionByNextInstDest } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

/*
** For integer division:
**  32bit signed-->
**          1. mediump-->highp-->RECURSIVE_SCAN
**          2. highp-->convert
**
**  32bit unsigned-->
**          1. mediump-->div.u16
**          2. highp-->convert
*/
static VIR_PatternMatchInst _divPatInst2[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitSignedInt, VIR_Lower_IsDstMediumpOrLowp }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst2[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { VIR_Lower_SetHighp, VIR_Lower_SetHighp, VIR_Lower_SetHighp } },
};

static VIR_PatternMatchInst _divPatInst3[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst3[] = {
    { VIR_OP_SIGN, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_SIGN, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -1, -1, -2, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -3, 2, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ABS, 0, 0, { -4, 3, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_I2F, 0, 0, { -5, -4, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, -5, 0, 0 }, { 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat, VIR_Lower_SetOpndFloat } },
    { VIR_OP_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_SUB, 0, 0, { -9, 0, -4, 0 }, { VIR_Lower_SetOpndUINT32HP, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -13, -12, -4, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, -3, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -14, -3, -14, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, -4, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { 1, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { 1, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _Lower_label_set_jmp_neg21 } },
    { VIR_OP_MOV, 0, 0, { 1, -3, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
    { VIR_OP_MUL, 0, 0, { 1, -1, 1, 0 }, { 0 } },
};

static VIR_PatternMatchInst _divPatInst4[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitUnsignedInt, VIR_Lower_IsDstMediumpOrLowp }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst4[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { VIR_Lower_SetOpndUINT16, VIR_Lower_SetOpndUINT16, VIR_Lower_SetOpndUINT16 } },
};

static VIR_PatternMatchInst _divPatInst5[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitUnsignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst5[] = {
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, 3, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne } },
    { VIR_OP_I2F, 0, 0, { -5, 3, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat, VIR_Lower_SetOpndFloat } },
    { VIR_OP_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_SUB, 0, 0, { -9, 0, 3, 0 }, { VIR_Lower_SetOpndUINT32HP, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -13, -12, 3, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, 2, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -14, 2, -14, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, 3, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { 1, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { 1, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg22 } },
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
};

static VIR_Pattern _divPrePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 2) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern _divPostPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 5) },
    { VIR_PATN_FLAG_NONE }
};

/*
** MOD, temp(1), temp(2), temp(3)
** -->
** DIV, temp(4), temp(2), temp(3)
** FLOOR, temp(4), temp(4)
** MAD, temp(1), -temp(3),temp(4), temp(2)
*/
static VIR_PatternMatchInst _modPatInst0[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { -1, 2, 3, 0 }, { VIR_Lower_SetHighp } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { VIR_Lower_SetHighp } },
    { VIR_OP_MAD, 0, 0, { 1, 3, -1, 2 }, { 0, VIR_Lower_SetOpndNeg, VIR_Lower_SetHighp, 0 } },
};

/*
** For integer mod:
**  32bit signed-->
**          1. mediump-->highp-->RECURSIVE_SCAN
**          2. highp-->convert
**
**  32bit unsigned-->
**          1. mediump-->mod.u16
**          2. highp-->convert
*/
static VIR_PatternMatchInst _modPatInst1[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitSignedInt, VIR_Lower_IsDstMediumpOrLowp }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst1[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { VIR_Lower_SetHighp, VIR_Lower_SetHighp, VIR_Lower_SetHighp } },
};

static VIR_PatternMatchInst _modPatInst2[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst2[] = {
    { VIR_OP_SIGN, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_SIGN, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -1, -1, -2, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -3, 2, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ABS, 0, 0, { -4, 3, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, -4, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne } },
    { VIR_OP_I2F, 0, 0, { -5, -4, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat, VIR_Lower_SetOpndFloat } },
    { VIR_OP_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_SUB, 0, 0, { -9, 0, -4, 0 }, { VIR_Lower_SetOpndUINT32HP, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -13, -12, -4, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, -3, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -14, -3, -14, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, -4, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -15, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { -15, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg22 } },
    { VIR_OP_MOV, 0, 0, { -15, -3, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
    { VIR_OP_MUL, 0, 0, { -15, -1, -15, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -15, 3, -15, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { 1, 2, -15, 0 }, { 0 } },
};

static VIR_PatternMatchInst _modPatInst3[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitUnsignedInt, VIR_Lower_IsDstMediumpOrLowp }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst3[] = {
    { VIR_OP_IMOD, 0, 0, { 1, 2, 3, 0 }, { VIR_Lower_SetOpndUINT16, VIR_Lower_SetOpndUINT16, VIR_Lower_SetOpndUINT16 } },
};

static VIR_PatternMatchInst _modPatInst4[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNot32IntDIV, _Is32BitUnsignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst4[] = {
    { VIR_OP_JMPC, VIR_COP_LESS_OR_EQUAL, 0, { 0, 3, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne } },
    { VIR_OP_I2F, 0, 0, { -5, 3, 0, 0 }, { VIR_Lower_SetOpndFloat } },
    { VIR_OP_ADD, 0, 0, { -6, -5, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, _SetUIntFloatFour } },
    { VIR_OP_RCP, 0, 0, { -7, -6, 0, 0 }, { VIR_Lower_SetOpndFloat, VIR_Lower_SetOpndFloat } },
    { VIR_OP_F2I, 0, 0, { -8, -7, 0, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_SUB, 0, 0, { -9, 0, 3, 0 }, { VIR_Lower_SetOpndUINT32HP, VIR_Lower_SetIntZero} },
    { VIR_OP_MUL, 0, 0, { -10, -9, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MULHI, 0, 0, { -11, -10, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -12, -11, -8, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -13, -12, 3, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, -13, -9, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -12, -12, 0, 0 }, { VIR_Lower_SetOpndUINT32HP, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },
    { VIR_OP_MULHI, 0, 0, { -13, -12, 2, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_MUL, 0, 0, { -14, -13, -9, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_ADD, 0, 0, { -14, 2, -14, 0 }, { VIR_Lower_SetOpndUINT32HP } },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -14, 3, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -15, -13, 0, 0 }, { 0, 0, VIR_Lower_SetUIntOne} },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_MOV, 0, 0, { -15, -13, 0, 0 }, { 0 } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg22 } },
    { VIR_OP_MOV, 0, 0, { -15, 2, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, 0, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6 } },
    { VIR_OP_MUL, 0, 0, { -15, 3, -15, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { 1, 2, -15, 0 }, { 0 } },
};

static VIR_PatternMatchInst _modPatInst5[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst5[] = {
    { VIR_OP_IMOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _modPrePattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_mod, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern _modPostPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 5) },
    { VIR_PATN_FLAG_NONE }
};

/*
** FIX, temp(1), temp(2)
** -->
** JMP.G    1, temp(2), 0
** CEIL     temp(1), temp(2)
** JMP      2
** LABEL    1
** FLOOR    temp(1), temp(2)
** LABEL    2
*/
static VIR_PatternMatchInst _fixPatInst0[] = {
    { VIR_OP_FIX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _fixRepInst0[] = {
    { VIR_OP_JMPC, VIR_COP_GREATER, 0, { 0, 2, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_CEIL, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_JMP, 0, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_FLOOR, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
};

static VIR_Pattern _fixPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_fix, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sinpiPatInst0[] = {
    { VIR_OP_SINPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sinpiRepInst0[] = {
    { VIR_OP_SINPI, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _sinpiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sinpi, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _cospiPatInst0[] = {
    { VIR_OP_COSPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _cospiRepInst0[] = {
    { VIR_OP_COSPI, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _cospiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cospi, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _logPatInst0[] = {
    { VIR_OP_PRE_LOG2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _logRepInst0[] = {
    { VIR_OP_PRE_LOG2, 0, VIR_PATTERN_TEMP_TYPE_XY, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _logPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_log, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _convPatInst0[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_SameType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst0[] = {
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};


static VIR_Pattern _convertPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetLowerPatternPhaseMachinePre(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_DIV:
        return _divPrePattern;
    case VIR_OP_MOD:
        return _modPrePattern;
    case VIR_OP_FIX:
        return _fixPattern;
    case VIR_OP_PRE_LOG2:
        return _logPattern;
    case VIR_OP_SINPI:
        return _sinpiPattern;
    case VIR_OP_COSPI:
        return _cospiPattern;
    case VIR_OP_CONVERT:
        return _convertPattern;
    default:
        break;
    }
    return gcvNULL;
}

static VIR_Pattern*
_GetLowerPatternPhaseMachinePost(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_DIV:
        return _divPostPattern;
    case VIR_OP_MOD:
        return _modPostPattern;
    default:
        break;
    }
    return gcvNULL;
}

static gctBOOL
_CmpInstuction(
    IN VIR_PatternContext   *Context,
    IN VIR_PatternMatchInst *Inst0,
    IN VIR_Instruction      *Inst1
    )
{
    return Inst0->opcode == Inst1->_opcode;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine_Pre(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, VscContext, Shader, Context->pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                  _GetLowerPatternPhaseMachinePre, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Machine_Pre failed.");

    VIR_PatternContext_Finalize(&Context->header);

    return errCode;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Machine_Post(
    IN  VIR_Shader              *Shader,
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, VscContext, Shader, Context->pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                  _GetLowerPatternPhaseMachinePost, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Machine_Post failed.");

    VIR_PatternContext_Finalize(&Context->header);

    return errCode;
}


