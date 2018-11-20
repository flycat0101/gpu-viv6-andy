/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/lower/gc_vsc_vir_hl_2_ml.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

#define _PI                ((gctFLOAT)3.14159265358979323846)
#define _LOG2_E            ((gctFLOAT)1.44269504088896340736)
#define _RCP_OF_LOG2_E     ((gctFLOAT)0.69314718055994530942)
#define _SMALLEST_F        ((gctFLOAT)1.175494351e-038f)

static gctBOOL
_setDestTypeFromSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand *coord = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId newType;

    newType = VIR_Operand_GetTypeId(coord);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, newType));
    VIR_Operand_SetTypeId(dest, newType);
    VIR_Operand_SetEnable(dest, VIR_TypeId_Conv2Enable(newType));

    return gcvTRUE;
}

static gctBOOL
label_set_jmp_n(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctINT32           n
    )
{
    VIR_Instruction* jmp_inst = Inst;
    VIR_Operand* jmp_dest;
    VIR_Label* label;
    VIR_Link * link = gcvNULL;

    gcmASSERT(Inst->_opcode == VIR_OP_LABEL && n != 0);

    if(n > 0)
    {
        while(n != 0)
        {
            jmp_inst = VIR_Inst_GetNext(jmp_inst);
            --n;
        }
    }
    else
    {
        while(n != 0)
        {
            jmp_inst = VIR_Inst_GetPrev(jmp_inst);
            ++n;
        }
    }

    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(jmp_inst)));

    label = VIR_Operand_GetLabel(Opnd);
    jmp_dest = VIR_Inst_GetDest(jmp_inst);
    VIR_Operand_SetLabel(jmp_dest, label);
    VIR_Function_NewLink(VIR_Inst_GetFunction(Inst), &link);
    VIR_Link_SetReference(link, (gctUINTPTR_T)jmp_inst);
    VIR_Link_AddLink(&(label->referenced), link);
    return gcvTRUE;
}

static gctBOOL
label_set_jmp_neg2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return label_set_jmp_n(Context, Inst, Opnd, -2);
}

static gctBOOL
label_set_jmp_neg5(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return label_set_jmp_n(Context, Inst, Opnd, -5);
}

static gctBOOL
label_set_jmp_neg7(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return label_set_jmp_n(Context, Inst, Opnd, -7);
}

/* set current operand as constant 0.5. */
static gctBOOL
_constf_o_point_five(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = 0.5f;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant 0 */
static gctBOOL
_constf_zero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = (gctFLOAT) 0.0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant 1.0 */
static gctBOOL
_constf_one(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = (gctFLOAT) 1.0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant 2.0 */
static gctBOOL
_constf_two(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = (gctFLOAT) 2.0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant 3.0 */
static gctBOOL
_constf_three(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = (gctFLOAT) 3.0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant _PI / 180.0 */
static gctBOOL
_constf_PI_OVER_180(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = _PI / (gctFLOAT) 180.0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant 180.0 / _PI */
static gctBOOL
_constf_180_OVER_PI(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = (gctFLOAT) 180.0 / _PI;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant LOG2_E */
static gctBOOL
_constf_LOG2_E(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = _LOG2_E;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as constant RCP_LOG2_E */
static gctBOOL
_constf_RCP_LOG2_E(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = _RCP_OF_LOG2_E;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

/* set current operand as smallest constant*/
static gctBOOL
_constf_smallest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.fValue = _SMALLEST_F;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_FLOAT32, imm);

    return gcvTRUE;
}

static gctBOOL
_constb_false(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.uValue = 0;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_BOOLEAN, imm);

    return gcvTRUE;
}

static gctBOOL
_constb_true(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal     imm;
    imm.uValue = 1;

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_BOOLEAN, imm);

    return gcvTRUE;
}

/* duplicate 1st parameter and set it to this source. */
static gctBOOL
_dup1stParm(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ParmPassing     *parm;

    gcmASSERT(VIR_Operand_isParameters(Opnd));
    parm = VIR_Operand_GetParameters(Opnd);
    gcmASSERT(parm->argNum >= 1);

    VIR_Operand_Copy(Opnd, parm->args[0]);

    return gcvTRUE;
}

/* duplicate 1st parameter as neg and set it to this source. */
static gctBOOL
_dup1stParmAsNeg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Modifier modifer;

    _dup1stParm(Context, Inst, Opnd);
    modifer = VIR_Operand_GetModifier(Opnd);
    modifer |= VIR_MOD_NEG;
    VIR_Operand_SetModifier(Opnd, modifer);

    return gcvTRUE;
}

static gctBOOL
_dup1stParmSwizzleX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _dup1stParm(Context, Inst, Opnd);
    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_X));

    return gcvTRUE;
}

static gctBOOL
_dup1stParmSwizzleY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _dup1stParm(Context, Inst, Opnd);
    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_Y));

    return gcvTRUE;
}

static gctBOOL
_dup1stParmSwizzleZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _dup1stParm(Context, Inst, Opnd);
    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_Z));
    return gcvTRUE;
}

static gctBOOL
_dup1stParmSwizzleW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _dup1stParm(Context, Inst, Opnd);
    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_W));
    return gcvTRUE;
}

/* duplicate 2nd parameter and set it to this source. */
static gctBOOL
_dup2ndParm(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ParmPassing     *parm;

    gcmASSERT(VIR_Operand_isParameters(Opnd));
    parm = VIR_Operand_GetParameters(Opnd);
    gcmASSERT(parm->argNum >= 2);

    VIR_Operand_Copy(Opnd, parm->args[1]);

    return gcvTRUE;
}

/* duplicate 2nd parameter and set it to this source
   set swizzle */
static gctBOOL
_dup2ndParmSwizzleX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _dup2ndParm(Context, Inst, Opnd);

    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_X));

    return gcvTRUE;
}

static gctBOOL
_dup2ndParmSwizzleY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _dup2ndParm(Context, Inst, Opnd);

    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_Y));

    return gcvTRUE;
}

static gctBOOL
_dup2ndParmSwizzleZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _dup2ndParm(Context, Inst, Opnd);

    VIR_Operand_SetSwizzle(Opnd, VIR_Swizzle_Extract_Single_Channel_Swizzle(VIR_Operand_GetSwizzle(Opnd), VIR_CHANNEL_Z));

    return gcvTRUE;
}

/* duplicate 3rd parameter and set it to this source. */
static gctBOOL
_dup3rdParm(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ParmPassing     *parm;

    gcmASSERT(VIR_Operand_isParameters(Opnd));
    parm = VIR_Operand_GetParameters(Opnd);
    gcmASSERT(parm->argNum >= 3);

    VIR_Operand_Copy(Opnd, parm->args[2]);

    return gcvTRUE;
}

static gctBOOL
_dup2ndParmAndSetDestTypeFromSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _dup2ndParm(Context, Inst, Opnd);
    _setDestTypeFromSrc0(Context, Inst, Opnd);

    return gcvTRUE;
}

/* set current opnd as neg. */
static gctBOOL
_setSourceNeg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand *src = gcvNULL;
    VIR_Modifier modifer;

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    src = VIR_Inst_GetSource(Inst, srcIdx);

    modifer = VIR_Operand_GetModifier(src);
    modifer |= VIR_MOD_NEG;
    VIR_Operand_SetModifier(src, modifer);

    return gcvTRUE;
}

static gctBOOL
_setRtz(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_RTZ);

    return gcvTRUE;
}

static gctBOOL
_setEnableXFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *destOperand= VIR_Inst_GetDest(Inst);

    /* Change enable and type. */
    VIR_Operand_SetEnable(destOperand, VIR_ENABLE_X);
    VIR_Operand_SetTypeId(destOperand, VIR_TYPE_FLOAT32);

    return gcvTRUE;
}

static gctBOOL
_setEnableYFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *destOperand= VIR_Inst_GetDest(Inst);

    /* Change enable and type. */
    VIR_Operand_SetEnable(destOperand, VIR_ENABLE_Y);
    VIR_Operand_SetTypeId(destOperand, VIR_TYPE_FLOAT32);

    return gcvTRUE;
}

static gctBOOL
_setEnableZFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *destOperand= VIR_Inst_GetDest(Inst);

    /* Change enable and type. */
    VIR_Operand_SetEnable(destOperand, VIR_ENABLE_Z);
    VIR_Operand_SetTypeId(destOperand, VIR_TYPE_FLOAT32);

    return gcvTRUE;
}

static gctBOOL
_setEnableWFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *destOperand= VIR_Inst_GetDest(Inst);

    /* Change enable and type. */
    VIR_Operand_SetEnable(destOperand, VIR_ENABLE_W);
    VIR_Operand_SetTypeId(destOperand, VIR_TYPE_FLOAT32);

    return gcvTRUE;
}

static gctBOOL
_setSwizzleXInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT                srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand           *srcOperand = VIR_Inst_GetSource(Inst, srcIdx);

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    /* Change swizzle and type. */
    VIR_Operand_SetSwizzle(srcOperand, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetTypeId(srcOperand, VIR_TYPE_INT32);

    return gcvTRUE;
}

static gctBOOL
_setSwizzleZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT                srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand           *srcOperand = VIR_Inst_GetSource(Inst, srcIdx);
    VIR_TypeId             opndTypeId = VIR_Operand_GetTypeId(Opnd);

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    /* Change swizzle and type. */
    VIR_Operand_SetSwizzle(srcOperand, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetTypeId(srcOperand, VIR_GetTypeComponentType(opndTypeId));

    return gcvTRUE;
}

static gctBOOL
_setSwizzleXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT                srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand           *srcOperand = VIR_Inst_GetSource(Inst, srcIdx);
    VIR_TypeId             opndTypeId = VIR_Operand_GetTypeId(Opnd);
    VIR_TypeId             vectorTypeId;

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    vectorTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(opndTypeId), 2, 1);

    /* Change swizzle and type. */
    VIR_Operand_SetSwizzle(srcOperand, VIR_SWIZZLE_XYYY);
    VIR_Operand_SetTypeId(srcOperand, vectorTypeId);

    return gcvTRUE;
}

static gctBOOL
_setSwizzleXYZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT                srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand           *srcOperand = VIR_Inst_GetSource(Inst, srcIdx);
    VIR_TypeId             opndTypeId = VIR_Operand_GetTypeId(Opnd);
    VIR_TypeId             vectorTypeId;

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    vectorTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(opndTypeId), 3, 1);

    /* Change swizzle and type. */
    VIR_Operand_SetSwizzle(srcOperand, VIR_SWIZZLE_XYZZ);
    VIR_Operand_SetTypeId(srcOperand, vectorTypeId);

    return gcvTRUE;
}

static gctBOOL
_setSwizzleXYZW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT                srcIdx = VIR_Inst_GetSourceIndex(Inst, Opnd);
    VIR_Operand           *srcOperand = VIR_Inst_GetSource(Inst, srcIdx);
    VIR_TypeId             opndTypeId = VIR_Operand_GetTypeId(Opnd);
    VIR_TypeId             vectorTypeId;

    gcmASSERT(srcIdx < VIR_MAX_SRC_NUM);

    vectorTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(opndTypeId), 4, 1);

    /* Change swizzle and type. */
    VIR_Operand_SetSwizzle(srcOperand, VIR_SWIZZLE_XYZW);
    VIR_Operand_SetTypeId(srcOperand, vectorTypeId);

    return gcvTRUE;
}

static gctBOOL
_setSwizzleAndTypeFromSymType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol            *srcSym = VIR_Operand_GetSymbol(Opnd);
    VIR_TypeId             symTypeId = VIR_Symbol_GetTypeId(srcSym);
    VIR_Swizzle            srcSwizzle;

    srcSwizzle = VIR_Swizzle_GenSwizzleByComponentCount(VIR_GetTypeComponents(symTypeId));

    VIR_Operand_SetSwizzle(Opnd, srcSwizzle);
    VIR_Operand_SetTypeId(Opnd, symTypeId);

    return gcvTRUE;
}

static gctBOOL
_destLE16Byte(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    if (VIR_TypeId_isPrimitive(tyId))
    {
        return VIR_GetTypeSize(tyId) <= 16;
    }
    return gcvFALSE;
}

static gctBOOL
_isDstBoolean(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_TypeId_isBoolean(VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)))));
}

/* clamp(x, minVal, maxVal)
   t1 = max(x, minVal)
   result = min(t1, maxVal) */
static VIR_PatternMatchInst _intrinClampPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinClampRepInst0[] = {
    { VIR_OP_MAX, 0, 0, { -1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
    { VIR_OP_MIN, 0, 0, {  1, -1, 3, 0 }, { 0, 0, _dup3rdParm } },
};

static VIR_Pattern _intrinClampPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinClamp, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* min(x, y) */
static VIR_PatternMatchInst _intrinMinPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinMinRepInst0[] = {
    { VIR_OP_MIN, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
};

static VIR_Pattern _intrinMinPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinMin, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* max(x, y) */
static VIR_PatternMatchInst _intrinMaxPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinMaxRepInst0[] = {
    { VIR_OP_MAX, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
};

static VIR_Pattern _intrinMaxPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinMax, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* abs(x) */
static VIR_PatternMatchInst _intrinAbsPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinAbsRepInst0[] = {
    { VIR_OP_ABS, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinAbsPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinAbs, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* sign(x) */
static VIR_PatternMatchInst _intrinSignPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSignRepInst0[] = {
    { VIR_OP_SIGN, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinSignPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSign, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* floor(x) */
static VIR_PatternMatchInst _intrinFloorPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinFloorRepInst0[] = {
    { VIR_OP_FLOOR, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinFloorPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinFloor, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* ceil(x) */
static VIR_PatternMatchInst _intrinCeilPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinCeilRepInst0[] = {
    { VIR_OP_CEIL, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinCeilPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinCeil, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* fract(x) */
static VIR_PatternMatchInst _intrinFractPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinFractRepInst0[] = {
    { VIR_OP_FRAC, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinFractPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinFract, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* mix(x, y, a):
   t1 = sub y, x
   t2 = mul t1, a
   result = add x, t2 */
static VIR_PatternMatchInst _intrinMixPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinMixRepInst0[] = {
    { VIR_OP_SUB, 0, 0, { -1, 3, 3, 0 }, { 0, _dup2ndParm, _dup1stParm } },
    { VIR_OP_MUL, 0, 0, { -2, -1, 3, 0 }, { 0, 0, _dup3rdParm } },
    { VIR_OP_ADD, 0, 0, { 1, 3, -2, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinMixPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinMix, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* round(x) = sign(x) * floor((abs(x) + 0.5)) */
static VIR_PatternMatchInst _intrinRoundPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinRoundRepInst0[] = {
    { VIR_OP_SIGN, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm } },
    { VIR_OP_ABS, 0, 0, { -2, 3, 0, 0 }, { 0, _dup1stParm } },
    { VIR_OP_ADD, 0, 0, { -2, -2, 0, 0 }, { _setRtz, 0, _constf_o_point_five } },
    { VIR_OP_FLOOR, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 1, -1, -2, 0 }, { 0 } },
};

static VIR_Pattern _intrinRoundPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinRound, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* trunc(x) = sign(x) * floor(abs(x)) */
static VIR_PatternMatchInst _intrinTruncPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinTruncRepInst0[] = {
    { VIR_OP_SIGN, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm } },
    { VIR_OP_ABS, 0, 0, { -2, 3, 0, 0 }, { 0, _dup1stParm } },
    { VIR_OP_FLOOR, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 1, -1, -2, 0 }, { 0 } },
};

static VIR_Pattern _intrinTruncPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinTrunc, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* pow(x, y) */
static VIR_PatternMatchInst _intrinPowPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinPowRepInst0[] = {
    { VIR_OP_POW, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
};

static VIR_Pattern _intrinPowPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinPow, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* exp(x) = exp2(x * LOG2_E)*/
static VIR_PatternMatchInst _intrinExpPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinExpRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm, _constf_LOG2_E } },
    { VIR_OP_EXP2, 0, 0, { 1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _intrinExpPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinExp, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* exp2(x)*/
static VIR_PatternMatchInst _intrinExp2PatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinExp2RepInst0[] = {
    { VIR_OP_EXP2, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinExp2Pattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinExp2, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* log(x) = _RCP_OF_LOG2_E * log2(x)*/
static VIR_PatternMatchInst _intrinLogPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinLogRepInst0[] = {
    { VIR_OP_LOG2, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm } },
    { VIR_OP_MUL, 0, 0, { 1, -1, 0, 0 }, { 0, 0, _constf_RCP_LOG2_E } },
};

static VIR_Pattern _intrinLogPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinLog, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* log2(x)*/
static VIR_PatternMatchInst _intrinLog2PatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinLog2RepInst0[] = {
    { VIR_OP_LOG2, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinLog2Pattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinLog2, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* sqrt(x)*/
static VIR_PatternMatchInst _intrinSqrtPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSqrtRepInst0[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinSqrtPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSqrt, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* inversesqrt(x)*/
static VIR_PatternMatchInst _intrinRSQPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinRSQRepInst0[] = {
    { VIR_OP_RSQ, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinRSQPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinRSQ, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* step(edge0, x) */
static VIR_PatternMatchInst _intrinStepPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinStepRepInst0[] = {
    { VIR_OP_STEP, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
};

static VIR_Pattern _intrinStepPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinStep, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* smoothstep(edge0, edge1, x) */
/* sub t0, x (operand2), edge0 (operand0) */
/* sub t1, edge1 (operand1), edge0 (operand0) */
/* div t2, t0, t1 */
/* sat t3, t2 */
/* mul t4, t3, t3 */
/* add t5, t3, t3 */
/* sub t6, 3.0, t5 */
/* mul result, t4, t6 */
static VIR_PatternMatchInst _intrinSmoothstepPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSmoothstepRepInst0[] = {
    { VIR_OP_SUB, 0, 0, { -1, 3, 3, 0 }, { 0, _dup3rdParm, _dup1stParm } },
    { VIR_OP_SUB, 0, 0, {-2, 3, 3, 0 }, { 0, _dup2ndParm, _dup1stParm } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { 0 } },
    { VIR_OP_SAT, 0, 0, { -4, -3, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -5, -4, -4, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -6, -4, -4, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -7, 0, -6, 0 }, { 0, _constf_three } },
    { VIR_OP_MUL, 0, 0, { 1, -5, -7, 0 }, { 0 } },
};

static VIR_Pattern _intrinSmoothstepPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSmoothstep, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* findLSB(x)*/
static VIR_PatternMatchInst _intrinFindLSBPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinFindLSBRepInst0[] = {
    { VIR_OP_BITFIND_LSB, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinFindLSBPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinFindLSB, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* findMSB(x)*/
static VIR_PatternMatchInst _intrinFindMSBPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinFindMSBRepInst0[] = {
    { VIR_OP_BITFIND_MSB, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinFindMSBPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinFindMSB, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* radians = x * (_PI / 180.0) */
static VIR_PatternMatchInst _intrinRadiansPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinRadiansRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm, _constf_PI_OVER_180} },
};

static VIR_Pattern _intrinRadiansPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinRadians, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* degrees = x * (180.0 / _PI) */
static VIR_PatternMatchInst _intrinDegreesPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinDegreesRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm, _constf_180_OVER_PI} },
};

static VIR_Pattern _intrinDegreesPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinDegrees, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* sin(x) */
static VIR_PatternMatchInst _intrinSinPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSinRepInst0[] = {
    { VIR_OP_SIN, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinSinPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSin, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* cos(x) */
static VIR_PatternMatchInst _intrinCosPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinCosRepInst0[] = {
    { VIR_OP_COS, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinCosPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinCos, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* tan(x) */
static VIR_PatternMatchInst _intrinTanPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinTanRepInst0[] = {
    { VIR_OP_TAN, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinTanPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinTan, 0) },
    { VIR_PATN_FLAG_NONE }
};


/* atan(x) */
static VIR_PatternMatchInst _intrinATanPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinATanRepInst0[] = {
    { VIR_OP_ATAN, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinATanPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinATan, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* sinh(x):
 t0 = mul x, _LOG2_E
 t1 = exp2 t0
 t2 = sub 0.0, x
 t3 = mul t2, _LOG2_E
 t4 = exp2, t3
 t5 = sub t1, t4
 result = div t5, 2.0
 */
static VIR_PatternMatchInst _intrinSinhPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSinhRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm, _constf_LOG2_E} },
    { VIR_OP_EXP2, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -3, 0, 3, 0 }, { 0, _constf_zero, _dup1stParm } },
    { VIR_OP_MUL, 0, 0, { -4, -3, 0, 0 }, { 0, 0, _constf_LOG2_E } },
    { VIR_OP_EXP2, 0, 0, { -5, -4, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -6, -2, -5, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, { 1, -6, 0, 0 }, { 0, 0, _constf_two } },
};

static VIR_Pattern _intrinSinhPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSinh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* cosh(x):
 t0 = mul x, _LOG2_E
 t1 = exp2 t0
 t2 = sub 0.0, x
 t3 = mul t2, _LOG2_E
 t4 = exp2, t3
 t5 = add t1, t4
 result = div t5, 2.0
 */
static VIR_PatternMatchInst _intrinCoshPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinCoshRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm, _constf_LOG2_E} },
    { VIR_OP_EXP2, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -3, 0, 3, 0 }, { 0, _constf_zero, _dup1stParm } },
    { VIR_OP_MUL, 0, 0, { -4, -3, 0, 0 }, { 0, 0, _constf_LOG2_E } },
    { VIR_OP_EXP2, 0, 0, { -5, -4, 0, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -6, -2, -5, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, { 1, -6, 0, 0 }, { 0, 0, _constf_two } },
};

static VIR_Pattern _intrinCoshPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinCosh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* tanh(x): sinh(x) / cosh(X)
 t0 = mul x, _LOG2_E
 t1 = exp2 t0
 t2 = sub 0.0, x
 t3 = mul t2, _LOG2_E
 t4 = exp2, t3
 t5 = sub t1, t4
 t6 = div t5, 2.0
 t7 = add t1, t4
 t8 = div t7, 2.0
 result = div t6, t8
 */
static VIR_PatternMatchInst _intrinTanhPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinTanhRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm, _constf_LOG2_E} },
    { VIR_OP_EXP2, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -3, 0, 3, 0 }, { 0, _constf_zero, _dup1stParm } },
    { VIR_OP_MUL, 0, 0, { -4, -3, 0, 0 }, { 0, 0, _constf_LOG2_E } },
    { VIR_OP_EXP2, 0, 0, { -5, -4, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -6, -2, -5, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, { -7, -6, 0, 0 }, { 0, 0, _constf_two } },
    { VIR_OP_ADD, 0, 0, { -8, -2, -5, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, { -9, -8, 0, 0 }, { 0, 0, _constf_two } },
    { VIR_OP_DIV, 0, 0, { 1, -7, -9, 0 }, { 0 } },
};

static VIR_Pattern _intrinTanhPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinTanh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* asinh(x)
   t1 = mul x, x
   t2 = add t1, 1.0
   t3 = sqrt t2
   t4 = add x, t3
   t5 = log2 t4
   result = mul t5, _RCP_OF_LOG2_E
   */
static VIR_PatternMatchInst _intrinASinhPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinASinhRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 3, 3, 0 }, { 0, _dup1stParm, _dup1stParm} },
    { VIR_OP_ADD, 0, 0, { -2, -1, 0, 0 }, { 0, 0, _constf_one } },
    { VIR_OP_SQRT, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -4, 3, -3, 0 }, { 0, _dup1stParm } },
    { VIR_OP_LOG2, 0, 0, { -5, -4, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 1, -5, 0, 0 }, { 0, 0, _constf_RCP_LOG2_E } },
};

static VIR_Pattern _intrinASinhPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinASinh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* acosh(x): log(x + sqrt((x + 1.0) * (x - 1.0)))
   t1 = add x, 1.0
   t2 = sub x, 1.0
   t3 = mul t1, t2
   t4 = sqrt t3
   t5 = add x, t4
   t6 = log2 t5
   result = mul t6, _RCP_OF_LOG2_E
   */
static VIR_PatternMatchInst _intrinACoshPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinACoshRepInst0[] = {
    { VIR_OP_ADD, 0, 0, { -1, 3, 0, 0 }, { 0, _dup1stParm, _constf_one } },
    { VIR_OP_SUB, 0, 0, { -2, 3, 0, 0 }, { 0, _dup1stParm, _constf_one } },
    { VIR_OP_MUL, 0, 0, { -3, -1, -2, 0 }, { 0 } },
    { VIR_OP_SQRT, 0, 0, { -4, -3, 0, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { -5, 3, -4, 0 }, { 0, _dup1stParm } },
    { VIR_OP_LOG2, 0, 0, { -6, -5, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 1, -6, 0, 0 }, { 0, 0, _constf_RCP_LOG2_E } },
};

static VIR_Pattern _intrinACoshPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinACosh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* atanh(x):
   t1 = add 1.0, x
   t2 = sub 1.0, x
   t3 = max t1, small_constant
   t4 = log2 t3
   t5 = mul t4, _RCP_OF_LOG2_E
   t6 = max t2, small_constant
   t7 = log2 t6
   t8 = mul t7, _RCP_OF_LOG2_E
   t9 = sub t5, t8
   result = div t9, 2.0
   */
static VIR_PatternMatchInst _intrinATanhPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinATanhRepInst0[] = {
    { VIR_OP_ADD, 0, 0, { -1, 0, 3, 0 }, { 0, _constf_one, _dup1stParm } },
    { VIR_OP_SUB, 0, 0, { -2, 0, 3, 0 }, { 0, _constf_one, _dup1stParm } },
    { VIR_OP_MAX, 0, 0, { -3, -1, 0, 0 }, { 0, 0, _constf_smallest} },
    { VIR_OP_LOG2, 0, 0, { -4, -3, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -5, -4, 0, 0 }, { 0, 0, _constf_RCP_LOG2_E } },
    { VIR_OP_MAX, 0, 0, { -6, -2, 0, 0 }, { 0, 0, _constf_smallest} },
    { VIR_OP_LOG2, 0, 0, { -7, -6, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -8, -7, 0, 0 }, { 0, 0, _constf_RCP_LOG2_E } },
    { VIR_OP_SUB, 0, 0, { -9, -5, -8, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, { 1, -9, 0, 0 }, { 0, 0, _constf_two} },
};

static VIR_Pattern _intrinATanhPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinATanh, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* cross(x, y) */
static VIR_PatternMatchInst _intrinCrossPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinCrossRepInst0[] = {
    { VIR_OP_CROSS, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm } },
};

static VIR_Pattern _intrinCrossPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinCross, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* length(x) */
static VIR_PatternMatchInst _intrinLengthPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinLengthRepInst0[] = {
    { VIR_OP_LENGTH, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinLengthPattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_intrinLength, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* distance(x, y):
   t1 = sub(x, y)
   result = length(t1) */
static VIR_PatternMatchInst _intrinDstPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinDstRepInst0[] = {
    { VIR_OP_SUB, 0, 0, { -1, 3, 3, 0 }, { VIR_Lower_SetEnableBaseOnSrc1, _dup1stParm, _dup2ndParm } },
    { VIR_OP_LENGTH, 0, 0, { 1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _intrinDstPattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN | VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_intrinDst, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* normalize(x) */
static VIR_PatternMatchInst _intrinNormalizePatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinNormalizeRepInst0[] = {
    { VIR_OP_NORM, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParm } },
};

static VIR_Pattern _intrinNormalizePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinNormalize, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* faceforward(x, y, z):
   t0 = dot, z, y
   t0 = select.lt, t0, 0.0, 1.0
   result = select.nz, t0, x, -x
*/
static VIR_PatternMatchInst _intrinFaceForwardPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinFaceForwardRepInst0[] = {
    { VIR_OP_DOT, 0, 0, { -1, 3, 3, 0}, { 0, _dup3rdParm, _dup2ndParm } },
    { VIR_OP_CSELECT, VIR_COP_LESS, 0, { -1, -1, 0, 0 }, { 0, 0, _constf_zero, _constf_one } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 1, -1, 3, 3 }, {0, 0, _dup1stParmAsNeg, _dup1stParm } },
};

static VIR_Pattern _intrinFaceForwardPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinFaceForward, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* refract(I, N, eta):
t0 --> -1, t1 --> -2, t2 --> -3
   t0 = dot, N, I
   t1 = mad, -t0, t0, 1.0
   t2 = mul, eta, eta
   t2 = mad, -t2, t1, 1.0
   t1 = 0.0
   jmp, label1, t2 < 0.0
   t1 = sqrt, t2
   t1 = mad, eta, t0, t1
   t0 = mul, eta, I
   t1 = mad, -t1, N, t0
   label1
   result = t1
*/
static VIR_PatternMatchInst _intrinRefractPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinRefractRepInst0[] = {
    { VIR_OP_DOT, 0, 0, { -1, 3, 3, 0 }, { 0, _dup2ndParm, _dup1stParm } },
    { VIR_OP_MAD, 0, 0, { -2, -1, -1, 0}, { 0, _setSourceNeg, 0, _constf_one} },
    { VIR_OP_MUL, 0, 0, { -3, 3, 3, 0 }, { 0, _dup3rdParm, _dup3rdParm } },
    { VIR_OP_MAD, 0, 0, { -3, -3, -2, 0}, { 0, _setSourceNeg, 0, _constf_one} },
    { VIR_OP_MOV, 0, 0, { -2, 0, 0, 0}, { 0, _constf_zero} },
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, -3, 0, 0 }, { 0, 0, _constf_zero } },
    { VIR_OP_SQRT, 0, 0, { -2, -3, 0, 0}, { 0 } },
    { VIR_OP_MAD, 0, 0, { -2, 3, -1, -2}, { 0, _dup3rdParm, 0 } },
    { VIR_OP_MUL, 0, 0, { -1, 3, 3, 0 }, { 0, _dup3rdParm, _dup1stParm } },
    { VIR_OP_MAD, 0, 0, { -2, -2, 3, -1}, { 0, _setSourceNeg, _dup2ndParm } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg5 } },
    { VIR_OP_MOV, 0, 0, { 1, -2, 0, 0 }, { 0 } },
};

static VIR_Pattern _intrinRefractPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinRefract, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* select(a, b, c) */
static VIR_PatternMatchInst _intrinSelectPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destLE16Byte }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinSelectRepInst0[] = {
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 1, 3, 3, 3 }, { 0, _dup1stParm, _dup2ndParm, _dup3rdParm } },
};

static VIR_Pattern _intrinSelectPattern[] = {
{ VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinSelect, 0) },
{ VIR_PATN_FLAG_NONE }
};

/* bitselect(a, b, c): res = (a & ~c) | (b & c) */
static VIR_PatternMatchInst _intrinBitselectPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 /* no HW bitselect */ }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinBitselectRepInst0[] = {
    { VIR_OP_NOT_BITWISE, 0, 0, { -1, 3, 0, 0 }, { VIR_Lower_SetEnableBaseOnSrc1, _dup3rdParm } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 3, -1, 0 }, { VIR_Lower_SetEnableBaseOnSrc1, _dup1stParm } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, 3, 3, 0 }, { VIR_Lower_SetEnableBaseOnSrc1, _dup2ndParm, _dup3rdParm } },
    { VIR_OP_OR_BITWISE, 0, 0, { 1, -2, -3, 0 }, { 0 } },
};

static VIR_Pattern _intrinBitselectPattern[] = {
{ VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinBitselect, 0) },
{ VIR_PATN_FLAG_NONE }
};


/* select(a, b, c) */
static VIR_PatternMatchInst _intrinMadsatPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destLE16Byte }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _intrinMadsatRepInst0[] = {
    { VIR_OP_MADSAT, VIR_COP_NOT_ZERO, 0, { 1, 3, 3, 3 }, { 0, _dup1stParm, _dup2ndParm, _dup3rdParm } },
};

static VIR_Pattern _intrinMadsatPattern[] = {
{ VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinMadsat, 0) },
{ VIR_PATN_FLAG_NONE }
};


/* query lod intrinsic function.*/
static gctBOOL
_isNotCubeSampler(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_ParmPassing     *parm;
    VIR_Operand         *parmOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand         *samplerOpnd;
    VIR_TypeId           samplerTypeId;

    gcmASSERT(VIR_Operand_isParameters(parmOpnd));
    parm = VIR_Operand_GetParameters(parmOpnd);
    samplerOpnd = parm->args[0];
    samplerTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(VIR_Operand_GetSymbol(samplerOpnd)));

    if (VIR_TypeId_isImageCube(samplerTypeId) || VIR_TypeId_isSamplerCube(samplerTypeId))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_isLODQFixAndNotCubeSampler(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasLODQFix
            &&
            _isNotCubeSampler(Context, Inst));
}

/* LODQ has precision issue when calculating the LOD for a samplerCube, so skip it. */
static VIR_PatternMatchInst _intrinQueryLodPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isLODQFixAndNotCubeSampler }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinQueryLodRepInst0[] = {
    { VIR_OP_LODQ, 0, 0, { 1, 3, 3, 0 }, { 0, _dup1stParm, _dup2ndParm, VIR_Lower_SetZero } },
    { VIR_OP_CONVERT, 0, 0, { 1, 1, 0, 0 }, { _setEnableXFloat, _setSwizzleXInt } },

};

static VIR_Pattern _intrinQueryLodPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinQueryLod, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* query levels intrinsic function.*/
static VIR_PatternMatchInst _intrinQueryLevelsPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinQueryLevelsRepInst0[] = {
    { VIR_OP_GET_SAMPLER_LS, 0, 0, { -1, 3, 0, 0 }, { VIR_Lower_SetEnableXYZWAndSymType, _dup1stParm, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, {  1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleX } },

};

static VIR_Pattern _intrinQueryLevelsPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinQueryLevels, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* query samples intrinsic function.*/
static VIR_PatternMatchInst _intrinQuerySamplesPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinQuerySamplesRepInst0[] = {
    { VIR_OP_GET_SAMPLER_LS, 0, 0, { -1, 3, 0, 0 }, { VIR_Lower_SetEnableXYZWAndSymType, _dup1stParm, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, {  1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleY } },

};

static VIR_Pattern _intrinQuerySamplesPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinQuerySamples, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* texld intrinsic function. */
static gctBOOL
_isIntrinSamplerArray(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *samplerOpnd;
    VIR_ParmPassing         *parm;

    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    samplerOpnd = parm->args[0];

    if (VIR_TypeId_isSamplerArray(VIR_Operand_GetTypeId(samplerOpnd)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isIntrinSampler1DArray(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *samplerOpnd;
    VIR_ParmPassing         *parm;

    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    samplerOpnd = parm->args[0];

    if (VIR_TypeId_isSampler1D(VIR_Operand_GetTypeId(samplerOpnd)) &&
        VIR_TypeId_isSamplerArray(VIR_Operand_GetTypeId(samplerOpnd)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isCoordFloat(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *samplerOpnd;
    VIR_ParmPassing         *parm;

    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    samplerOpnd = parm->args[1];

    if (VIR_TypeId_isFloat(VIR_Operand_GetTypeId(samplerOpnd)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_dupTexldModifierFrom3rdParm(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * texldOperand;
    VIR_ParmPassing     *parm;

    gcmASSERT(VIR_Operand_isParameters(Opnd));

    parm = VIR_Operand_GetParameters(Opnd);

    if (parm->argNum >= 3)
    {
        VIR_Operand_Copy(Opnd, parm->args[2]);
    }
    else
    {
        /* Initialize a empty texld modifier. */
        texldOperand = (VIR_Operand *)Opnd;
        gcoOS_ZeroMemory((gctPOINTER)texldOperand->u.tmodifier, VIR_TEXLDMODIFIER_COUNT * gcmSIZEOF(VIR_Operand *));

        VIR_Operand_SetOpKind(Opnd, VIR_OPND_TEXLDPARM);
        VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);
        VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    }

    return gcvTRUE;
}

static gctBOOL
_isIntrinParam0Vec4(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst)
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *vecOpnd;
    VIR_ParmPassing         *parm;
    VIR_TypeId               vecTypeId;
    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    vecOpnd = parm->args[0];
    vecTypeId = VIR_Operand_GetTypeId(vecOpnd);
    if (!VIR_TypeId_isPacked(vecTypeId) && /* not support packed mode yet*/
        VIR_GetTypeComponents(vecTypeId) == 4)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isIntrinParam0Vec3(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst)
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *vecOpnd;
    VIR_ParmPassing         *parm;
    VIR_TypeId               vecTypeId;
    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    vecOpnd = parm->args[0];
    vecTypeId = VIR_Operand_GetTypeId(vecOpnd);

    if (!VIR_TypeId_isPacked(vecTypeId) && /* not support packed mode yet*/
        VIR_GetTypeComponents(vecTypeId) == 3)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isIntrinParam0Vec2(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst)
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand             *vecOpnd;
    VIR_ParmPassing         *parm;
    VIR_TypeId               vecTypeId;
    gcmASSERT(VIR_Operand_isParameters(pOpnd));

    parm = VIR_Operand_GetParameters(pOpnd);
    vecOpnd = parm->args[0];
    vecTypeId = VIR_Operand_GetTypeId(vecOpnd);

    if (!VIR_TypeId_isPacked(vecTypeId) && /* not support packed mode yet*/
        VIR_GetTypeComponents(vecTypeId) == 2)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/* Construct the coord for 1DArray first, then calculate the coord. */
static VIR_PatternMatchInst _intrinTexldPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinSampler1DArray, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinTexldRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat, _dup2ndParmSwizzleX } },
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableZFloat, _dup2ndParmSwizzleY } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_ADD, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ, _constf_o_point_five } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ } },
    { VIR_OP_MAX, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ, _constf_zero } },
    { VIR_OP_TEXLD, 0, 0, { 1, 3, -1, 3 }, { 0, _dup1stParm, _setSwizzleXYZ, _dupTexldModifierFrom3rdParm } },
};


/* Calculate coord for a samplerArray.*/
static VIR_PatternMatchInst _intrinTexldPatInst1[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinSamplerArray, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinTexldRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { 0, _dup2ndParmAndSetDestTypeFromSrc0 } },
    { VIR_OP_ADD, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ, _constf_o_point_five } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat } },
    { VIR_OP_MAX, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, 0, _constf_zero } },
    { VIR_OP_TEXLD, 0, 0, { 1, 3, -1, 3 }, { 0, _dup1stParm, _setSwizzleAndTypeFromSymType, _dupTexldModifierFrom3rdParm } },
};

static VIR_Pattern _intrinTexldPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinTexld, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_intrinTexld, 1) },
    { VIR_PATN_FLAG_NONE }
};

/* texlpcf intrinsic function. */
static VIR_PatternMatchInst _intrinTexldpcfPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinSampler1DArray, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinTexldpcfRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat, _dup2ndParmSwizzleX } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableZFloat, _dup2ndParmSwizzleY } },
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableWFloat, _dup2ndParmSwizzleZ } },
    { VIR_OP_ADD, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ, _constf_o_point_five } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat } },
    { VIR_OP_MAX, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, 0, _constf_zero } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 3, -1, 3 }, { 0, _dup1stParm, _setSwizzleXYZW, _dupTexldModifierFrom3rdParm } },
};

static VIR_PatternMatchInst _intrinTexldpcfPatInst1[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinSamplerArray, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinTexldpcfRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { 0, _dup2ndParmAndSetDestTypeFromSrc0 } },
    { VIR_OP_ADD, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, _setSwizzleZ, _constf_o_point_five } },
    { VIR_OP_FLOOR, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat } },
    { VIR_OP_MAX, 0, 0, { -1, -1, 0, 0 }, { _setEnableZFloat, 0, _constf_zero } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 3, -1, 3 }, { 0, _dup1stParm, _setSwizzleAndTypeFromSymType, _dupTexldModifierFrom3rdParm } },
};

static VIR_Pattern _intrinTexldpcfPattern[] = {
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_intrinTexldpcf, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_intrinTexldpcf, 1) },
    { VIR_PATN_FLAG_NONE }
};

/* vecget instrinsic function param0 is vec4
    vecget d1, vec4 src0, src1
=>
    t0 = src1->secondparameter
    d1 = src0.x
    jmpc label2, t0 == 1
    jmpc label3, t0 == 2
    jmpc lable4, t0 == 3
label1:
    d1 = src0.y
    jmp label4
label2:
    d1 = src0.z
    jmp label4
label3:
    d1 = src0.w
label4:
*/
static VIR_PatternMatchInst _intrinVecGetPatInst0[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinParam0Vec4 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinVecGetRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { 0, _dup2ndParm } },
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleX } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntOne, 0 } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntTwo, 0 } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntThree, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } }, /*label1*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleY } },
    { VIR_OP_JMP, VIR_COP_ALWAYS, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg5 } }, /*label2*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleZ } },
    { VIR_OP_JMP, VIR_COP_ALWAYS, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg7 } }, /*label3*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleW } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3_6_9 } }, /*label4*/
};

/* vecget instrinsic function param0 is vec3
    vecget d1, vec3 src0, src1
=>
    t0 = src1->secondparameter
    d1 = src0.x
    jmpc label1, t0 == 1
    jmpc label2, t0 == 2
label1:
    d1 = src0.y
    jmp label3
label2:
    d1 = src0.z
label3:
*/
static VIR_PatternMatchInst _intrinVecGetPatInst1[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinParam0Vec3 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinVecGetRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { 0, _dup2ndParm } },
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleX } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntOne, 0 } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntTwo, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg2 } }, /*label1*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleY } },
    { VIR_OP_JMP, VIR_COP_ALWAYS, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg4 } }, /*label2*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleZ } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } }, /*label3*/
};

/* vecget instrinsic function param0 is vec2
    vecget d1, vec2 src0, src1
=>
    t0 = src1->secondparameter
    jmpc label1, t0 == 1
    d1 = src0.x
    jmp label2;
label1:
    d1 = src0.y
label2:
*/
static VIR_PatternMatchInst _intrinVecGetPatInst2[] = {
    { VIR_OP_INTRINSIC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isIntrinParam0Vec2 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _intrinVecGetRepInst2[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { 0, _dup2ndParm } },
    { VIR_OP_JMPC, VIR_COP_EQUAL, 0, { 0, -1, 0, 0 }, { 0, VIR_Lower_SetOpndINT32, VIR_Lower_SetIntOne, 0 } },
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleX } },
    { VIR_OP_JMP, VIR_COP_ALWAYS, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } }, /*label1*/
    { VIR_OP_MOV, 0, 0, { 1, 3, 0, 0 }, { 0, _dup1stParmSwizzleY } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } }, /*label2*/
};

static VIR_Pattern _intrinVecGetPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinVecGet, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinVecGet, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_intrinVecGet, 2) },
    { VIR_PATN_FLAG_NONE }
};

/* should synchronize with gc_vsc_vir_intrisic_kind.def.h */
static VIR_Pattern* _intrisicPatterns[] = {
    gcvNULL, /* (NONE), */
    gcvNULL, /* (UNKNOWN), */

    gcvNULL, /* evis_begin */
    gcvNULL, /* evis_abs_diff */
    gcvNULL, /* evis_iadd */
    gcvNULL, /* evis_iacc_sq */
    gcvNULL, /* evis_lerp */
    gcvNULL, /* evis_filter */
    gcvNULL, /* evis_mag_phase */
    gcvNULL, /* evis_mul_shift */
    gcvNULL, /* evis_dp16x1 */
    gcvNULL, /* evis_dp8x2 */
    gcvNULL, /* evis_dp4x4 */
    gcvNULL, /* evis_dp2x8 */
    gcvNULL, /* evis_clamp */
    gcvNULL, /* evis_bi_linear */
    gcvNULL, /* evis_select_add */
    gcvNULL, /* evis_atomic_add */
    gcvNULL, /* evis_bit_extract */
    gcvNULL, /* evis_bit_replace */
    gcvNULL, /* evis_dp32x1 */
    gcvNULL, /* evis_dp16x2 */
    gcvNULL, /* evis_dp8x4 */
    gcvNULL, /* evis_dp4x8 */
    gcvNULL, /* evis_dp2x16 */
    gcvNULL, /* evis_dp32x1_b */
    gcvNULL, /* evis_dp16x2_b */
    gcvNULL, /* evis_dp8x4_b */
    gcvNULL, /* evis_dp4x8_b */
    gcvNULL, /* evis_dp2x16_b */
    gcvNULL, /* evis_img_load */
    gcvNULL, /* evis_img_load_3d */
    gcvNULL, /* evis_img_store */
    gcvNULL, /* evis_img_store_3d */
    gcvNULL, /* evis_vload2 */
    gcvNULL, /* evis_vload3 */
    gcvNULL, /* evis_vload4 */
    gcvNULL, /* evis_vload8 */
    gcvNULL, /* evis_vload16 */
    gcvNULL, /* evis_vstore2 */
    gcvNULL, /* evis_vstore3 */
    gcvNULL, /* evis_vstore4 */
    gcvNULL, /* evis_vstore8 */
    gcvNULL, /* evis_vstore16 */
    gcvNULL, /* evis_index_add */
    gcvNULL, /* evis_vert_min3 */
    gcvNULL, /* evis_vert_max3 */
    gcvNULL, /* evis_vert_med3 */
    gcvNULL, /* evis_horz_min3 */
    gcvNULL, /* evis_horz_max3 */
    gcvNULL, /* evis_horz_med3 */
    gcvNULL, /* evis_error */
    gcvNULL, /* evis_bit_extract */
    gcvNULL, /* evis_dp16x1_b */
    gcvNULL, /* evis_dp8x2_b */
    gcvNULL, /* evis_dp4x4_b */
    gcvNULL, /* evis_dp2x8_b */
    gcvNULL, /* evis_end */

    /* common functions */
    _intrinClampPattern, /* (clamp) */
    _intrinMinPattern, /* (min) */
    _intrinMaxPattern, /* (max) */
    gcvNULL, /* (nclamp)*/
    gcvNULL, /* (nmin)*/
    gcvNULL, /* (nmax)*/
    _intrinAbsPattern, /* (abs) */
    _intrinSignPattern, /* (sign) */
    _intrinFloorPattern, /* (floor) */
    _intrinCeilPattern, /* (ceil) */
    _intrinFractPattern, /* (fract) */
    _intrinMixPattern, /* (mix) */
    _intrinRoundPattern, /* (round) */
    gcvNULL, /* (roundEven), link it from a library later. */
    _intrinTruncPattern, /* (trunc) */
    _intrinPowPattern, /* (pow) */
    _intrinExpPattern, /* (exp) */
    _intrinExp2Pattern, /* (exp2) */
    _intrinLogPattern, /* (log) */
    _intrinLog2Pattern, /* (log2) */
    _intrinSqrtPattern, /* (sqrt) */
    _intrinRSQPattern, /* (inversesqrt) */
    gcvNULL, /* (modf), link it from a library later. */
    gcvNULL, /* (modfstruct), should be split into several MODFs. */
    _intrinStepPattern, /* (step) */
    _intrinSmoothstepPattern,/* (smoothstep) */
    gcvNULL, /* (fma), link it from a library later. */
    gcvNULL, /* (frexp), link it from a library later. */
    gcvNULL, /* (frexpstruct), link it from library. */
    gcvNULL, /* (ldexp), link it from a library later. */

    /* Matrix-related functions */
    gcvNULL, /* (determinant), link it from a library later. */
    gcvNULL, /* (matrixinverse), link it from a library later. */
    gcvNULL, /* (matrixCompMult) */

    /* Integer Functions */
    _intrinFindLSBPattern, /* (findLSB) */
    _intrinFindMSBPattern, /* (findMSB) */

    /* Angle and Trigonometry Functions */
    _intrinRadiansPattern, /* (radians) */
    _intrinDegreesPattern, /* (degrees) */
    _intrinSinPattern, /* (sin), */
    _intrinCosPattern, /* (cos), */
    _intrinTanPattern, /* (tan), */
    gcvNULL, /* (asin), link from library */
    gcvNULL, /* (acos), link from library */
    _intrinATanPattern, /* (atan), */

    _intrinSinhPattern, /* (sinh), */
    _intrinCoshPattern, /* (cosh), */
    _intrinTanhPattern, /* (tanh), */
    _intrinASinhPattern, /* (asinh), */
    _intrinACoshPattern, /* (acosh), */
    _intrinATanhPattern, /* (atanh), */
    gcvNULL, /* (atan2), link it from a library later. */

    /* Floating-Point Pack and Unpack Functions */
    gcvNULL, /* (packsnorm4x8), link it from a library later. */
    gcvNULL, /* (packunorm4x8), link it from a library later. */
    gcvNULL, /* (packsnorm2x16), link it from a library later. */
    gcvNULL, /* (packunorm2x16), link it from a library later. */
    gcvNULL, /* (packhalf2x16), link it from a library later. */
    gcvNULL, /* (packdouble2x32), link it from a library later. */
    gcvNULL, /* (unpacksnorm2x16), link it from a library later. */
    gcvNULL, /* (unpackunorm2x16), link it from a library later. */
    gcvNULL, /* (unpackhalf2x16), link it from a library later. */
    gcvNULL, /* (unpacksnorm4x8), link it from a library later. */
    gcvNULL, /* (unpackunorm4x8), link it from a library later. */
    gcvNULL, /* (unpackdouble2x32), link it from a library later. */

    /* Geometric Functions */
    _intrinCrossPattern, /* (cross) */
    _intrinLengthPattern, /* (length) */
    _intrinDstPattern, /* (distance) */
    _intrinNormalizePattern, /* (normalize) */
    _intrinFaceForwardPattern, /* (faceforward) */
    gcvNULL, /* (reflect), link it from a library later. */
    _intrinRefractPattern, /* (refract) */

    /* Vector Relational Functions */
    gcvNULL, /* (isequal) */
    gcvNULL, /* (isnotequal) */
    gcvNULL, /* (isgreater) */
    gcvNULL, /* (isgreaterequal) */
    gcvNULL, /* (isless) */
    gcvNULL, /* (islessequal) */
    gcvNULL, /* (islessgreater) */
    gcvNULL, /* (isordered) */
    gcvNULL, /* (isunordered) */
    gcvNULL, /* (isfinite) */
    gcvNULL, /* (isnan) */
    gcvNULL, /* (isinf) */
    gcvNULL, /* (isnormal) */
    gcvNULL, /* (signbit) */
    gcvNULL, /* (lgamma) */
    gcvNULL, /* (lgamma_r) */
    gcvNULL, /* (shuffle) */
    gcvNULL, /* (shuffle2) */
    _intrinSelectPattern, /* (select) */
    _intrinBitselectPattern, /* (bitselect) */
    gcvNULL, /* (any) */
    gcvNULL, /* (all) */

    /* Async copy and prefetch */
    gcvNULL, /* (async_work_group_copy) */
    gcvNULL, /* (async_work_group_strided_copy) */
    gcvNULL, /* (wait_group_events) */
    gcvNULL, /* (prefetch) */

    /* Atomic Functions */
    gcvNULL, /* (atomic_add) */
    gcvNULL, /* (atomic_sub) */
    gcvNULL, /* (atomic_inc) */
    gcvNULL, /* (atomic_dec) */
    gcvNULL, /* (atomic_xchg) */
    gcvNULL, /* (atomic_cmpxchg) */
    gcvNULL, /* (atomic_min) */
    gcvNULL, /* (atomic_max) */
    gcvNULL, /* (atomic_or) */
    gcvNULL, /* (atomic_and) */
    gcvNULL, /* (atomic_xor) */

    /* work-item functions */
    gcvNULL, /* (get_global_id) */
    gcvNULL, /* (get_local_id) */
    gcvNULL, /* (get_group_id) */
    gcvNULL, /* (get_work_dim) */
    gcvNULL, /* (get_global_size) */
    gcvNULL, /* (get_local_size) */
    gcvNULL, /* (get_global_offset) */
    gcvNULL, /* (get_num_groups) */

    /* Interpolation Functions. */
    gcvNULL, /* (interpolateAtCentroid), link it from a library later. */
    gcvNULL, /* (interpolateAtSample), link it from a library later. */
    gcvNULL, /* (interpolateAtOffset), link it from a library later. */

    gcvNULL, /*  (barrier) */

    /* matrix operations */
    gcvNULL, /* (transpose), matrix transpose */
    gcvNULL, /* (matrix_times_scalar), matrix * scalar */
    gcvNULL, /* (mattrix_times_vector), matrix * vector */
    gcvNULL, /* (matrix_times_matrix), matrix * matrix */
    gcvNULL, /* (vector_times_matrix), vector * matrix */
    gcvNULL, /* (outer_product), matrix outer product */

    /* Image-related functions. */
    gcvNULL, /* (imageStore) */
    gcvNULL, /* (imageLoad) */

    _intrinVecGetPattern, /* vecGet (get a componet from a vec), link from lib */
    gcvNULL, /* vecSet (set a componet to a vec), link from lib */

    gcvNULL, /* add carry */
    gcvNULL, /* sub borrow */
    gcvNULL, /* umul extended */
    gcvNULL, /* imul extended */

    gcvNULL, /* quantizeToF16 */

    gcvNULL, /* image fetch */
    gcvNULL, /* image fetch for sampler */
    gcvNULL, /* image address */
    gcvNULL, /* image_query_format */
    gcvNULL, /* image_query_order */
    gcvNULL, /* image_query_size_lod */
    gcvNULL, /* image_query_size */
    _intrinQueryLodPattern, /* image_query_lod */
    _intrinQueryLevelsPattern, /* image_query_levels */
    _intrinQuerySamplesPattern, /* image_query_samples */
    gcvNULL, /* image_get_width */
    gcvNULL, /* image_get_height */
    gcvNULL, /* image_get_depth */
    gcvNULL, /* image_get_array_size */

    _intrinTexldPattern, /* texld */
    _intrinTexldpcfPattern, /* texldpcf */
    gcvNULL, /* texld_proj */
    gcvNULL, /* texld_gather */
    gcvNULL, /* texld_fetch_ms */

    /* three operand instructions */
    gcvNULL, /* swizzle */
    _intrinMadsatPattern, /* madsat */
    gcvNULL, /* swizzle_full_def */
    gcvNULL, /* imadhi0 */
    gcvNULL, /* imadlo0 */
    gcvNULL, /* bitextract */
    gcvNULL, /* bitInsert */
};

char _checkIntrisicPatternsSize[(sizeof(_intrisicPatterns) / sizeof(VIR_Pattern*)) == VIR_IK_LAST];

static
gctBOOL _componentX(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))) == 1);
}

static VIR_PatternMatchInst _lengthPatInst0[] = {
    { VIR_OP_LENGTH, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _componentX }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _lengthRepInst0[] = {
    { VIR_OP_ABS, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _lengthPatInst1[] = {
    { VIR_OP_LENGTH, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _lengthRepInst1[] = {
    { VIR_OP_DOT, 0, 0, { -1, 2, 2, 0 }, { 0 } },
    { VIR_OP_SQRT, 0, 0, { 1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _lengthPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_length, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_length, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
** Just change UNREACHABLE to NOP.
*/
static VIR_PatternMatchInst _unreachablePatInst0[] = {
    { VIR_OP_UNREACHABLE, VIR_PATTERN_ANYCOND, 0, { 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _unreachableRepInst0[] = {
    { VIR_OP_NOP, VIR_PATTERN_ANYCOND, 0, { 0 }, { 0 } },
};

static VIR_Pattern _unreachablePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_unreachable, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
** When load a boolean variable from a buffer, we need to make that the value of this variable is TRUE or FALSE.
*/
static VIR_PatternMatchInst _loadPatInst0[] = {
    { VIR_OP_LOAD, 0, 0, { 1, 2, 3, 0 }, { _isDstBoolean }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst0[] = {
    { VIR_OP_LOAD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 1, 1, 0, 0 }, { 0, 0, _constb_true, _constb_false } },
};

static VIR_Pattern _loadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_isSampler1D(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (VIR_TypeId_isSampler1D(VIR_Operand_GetTypeId(pOpnd)) &&
        !VIR_TypeId_isSamplerArray(VIR_Operand_GetTypeId(pOpnd)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isSampler1DShadow(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    VIR_Operand             *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (VIR_TypeId_isSampler1D(VIR_Operand_GetTypeId(pOpnd)) &&
        VIR_TypeId_isSamplerShadow(VIR_Operand_GetTypeId(pOpnd)) &&
        !VIR_TypeId_isSamplerArray(VIR_Operand_GetTypeId(pOpnd)))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/*
** texld pattern.
*/
/* Construct a vec2 coordinate */
static VIR_PatternMatchInst _texldPatInst0[] = {
    { VIR_OP_TEXLD, 0, 0, { 1, 2, 3, 4 }, { _isSampler1D, VIR_Lower_HasTexldModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_TEXLD, 0, 0, { 1, 2, -1, 4 }, { 0, 0, _setSwizzleXY } },
};

static VIR_PatternMatchInst _texldPatInst1[] = {
    { VIR_OP_TEXLD, 0, 0, { 1, 2, 3, 0 }, { _isSampler1D }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_TEXLD, 0, 0, { 1, 2, -1, 0 }, { 0, 0, _setSwizzleXY } },
};

static VIR_Pattern _texldPattern[] = {
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE, CODEPATTERN(_texld, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
** texldproj pattern.
*/
/* Construct a vec3 coordinate */
static VIR_PatternMatchInst _texldprojPatInst0[] = {
    { VIR_OP_TEXLDPROJ, 0, 0, { 1, 2, 3, 4 }, { _isSampler1D, VIR_Lower_HasTexldModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableZFloat, VIR_Lower_SetSwizzleY } },
    { VIR_OP_TEXLDPROJ, 0, 0, { 1, 2, -1, 4 }, { 0, 0, _setSwizzleXYZ } },
};

static VIR_PatternMatchInst _texldprojPatInst1[] = {
    { VIR_OP_TEXLDPROJ, 0, 0, { 1, 2, 3, 0 }, { _isSampler1D }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableZFloat, VIR_Lower_SetSwizzleY } },
    { VIR_OP_TEXLDPROJ, 0, 0, { 1, 2, -1, 0 }, { 0, 0, _setSwizzleXYZ } },
};

static VIR_Pattern _texldprojPattern[] = {
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE, CODEPATTERN(_texldproj, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
** texldpcf pattern.
*/
/* Construct a vec3 coordinate */
static VIR_PatternMatchInst _texldpcfPatInst0[] = {
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, 3, 4 }, { _isSampler1DShadow, VIR_Lower_HasTexldModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableZFloat, _constf_zero } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, -1, 4 }, { 0, 0, _setSwizzleXYZ } },
};

static VIR_PatternMatchInst _texldpcfPatInst1[] = {
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, 3, 0 }, { _isSampler1DShadow }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3, 0, 0 }, { _setEnableXFloat } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableYFloat, _constf_zero } },
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0 }, { _setEnableZFloat, _constf_zero } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, -1, 0 }, { 0, 0, _setSwizzleXYZ } },
};

static VIR_Pattern _texldpcfPattern[] = {
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE, CODEPATTERN(_texldpcf, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetHL2MLPatternPhaseExpand(
    IN  VIR_PatternContext * Context,
    IN VIR_Instruction     * Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    /* intrinsic built-in function call. */
    case VIR_OP_INTRINSIC:
        return _intrisicPatterns[VIR_Operand_GetIntrinsicKind(VIR_Inst_GetSource(Inst, 0))];

    case VIR_OP_LENGTH:
        return _lengthPattern;

    case VIR_OP_UNREACHABLE:
        return _unreachablePattern;

    case VIR_OP_LOAD:
        return _loadPattern;

    case VIR_OP_TEXLD:
        return _texldPattern;

    case VIR_OP_TEXLDPROJ:
        return _texldprojPattern;

    case VIR_OP_TEXLDPCF:
        return _texldpcfPattern;

    default:
        break;
    }

    return gcvNULL;
}

static gctBOOL
_CmpInstuction(
    IN VIR_PatternContext  *Context,
    IN VIR_PatternMatchInst     *Inst0,
    IN VIR_Instruction     *Inst1
    )
{
    return Inst0->opcode == Inst1->_opcode;
}

VSC_ErrCode
_processEvisIntrinsic(
    IN VIR_Shader         * pShader,
    IN VIR_Instruction    * pInst,
    VIR_IntrinsicsKind      Ik
    )
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Function *    func = VIR_Inst_GetFunction(pInst);
    VIR_Operand  *    src0 = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand  *    src1 = VIR_Inst_GetSource(pInst, 1);
    VIR_OpCode        opCode = VIR_OP_NOP;
    gctBOOL           isVX2 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_EVIS_VX2);
    gctUINT           i;
    VIR_ParmPassing * argList;
    gctINT            evisSrcNo;
    VIR_Operand *     modifier;
    gctUINT           val;
    gctUINT           startBin;
    gctUINT           endBin;
    gctINT            storeElements = 0;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_INTRINSIC);
    if (VIR_Operand_GetOpKind(src1) != VIR_OPND_PARAMETERS)
    {
        return errCode;
    }

    switch (Ik) {
    case VIR_IK_evis_abs_diff:
        opCode = VIR_OP_VX_ABSDIFF;
        break;
    case VIR_IK_evis_iadd:
        opCode = VIR_OP_VX_IADD;
        break;
    case VIR_IK_evis_iacc_sq:
        opCode = VIR_OP_VX_IACCSQ;
        break;
    case VIR_IK_evis_lerp:
        opCode = VIR_OP_VX_LERP;
        break;
    case VIR_IK_evis_filter:
        opCode = VIR_OP_VX_FILTER;
        break;
    case VIR_IK_evis_mag_phase:
        opCode = VIR_OP_VX_MAGPHASE;
        break;
    case VIR_IK_evis_mul_shift:
        opCode = VIR_OP_VX_MULSHIFT;
        break;
    case VIR_IK_evis_dp16x1:
        opCode = VIR_OP_VX_DP16X1;
        break;
    case VIR_IK_evis_dp8x2:
        opCode = VIR_OP_VX_DP8X2;
        break;
    case VIR_IK_evis_dp4x4:
        opCode = VIR_OP_VX_DP4X4;
        break;
    case VIR_IK_evis_dp2x8:
        opCode = VIR_OP_VX_DP2X8;
        break;
   case VIR_IK_evis_dp16x1_b:
        opCode = VIR_OP_VX_DP16X1_B;
        break;
    case VIR_IK_evis_dp8x2_b:
        opCode = VIR_OP_VX_DP8X2_B;
        break;
    case VIR_IK_evis_dp4x4_b:
        opCode = VIR_OP_VX_DP4X4_B;
        break;
    case VIR_IK_evis_dp2x8_b:
        opCode = VIR_OP_VX_DP2X8_B;
        break;
    case VIR_IK_evis_clamp:
        opCode = VIR_OP_VX_CLAMP;
        break;
    case VIR_IK_evis_bi_linear:
        opCode = VIR_OP_VX_BILINEAR;
        break;
    case VIR_IK_evis_select_add:
        opCode = VIR_OP_VX_SELECTADD;
        break;
    case VIR_IK_evis_atomic_add:
        opCode = VIR_OP_VX_ATOMICADD;
        break;
    case VIR_IK_evis_bit_extract:
        opCode = VIR_OP_VX_BITEXTRACT;
        break;
    case VIR_IK_evis_bit_replace:
        opCode = VIR_OP_VX_BITREPLACE;
        break;
    case VIR_IK_evis_dp32x1:
        opCode = VIR_OP_VX_DP32X1;
        break;
    case VIR_IK_evis_dp16x2:
        opCode = VIR_OP_VX_DP16X2;
        break;
    case VIR_IK_evis_dp8x4:
        opCode = VIR_OP_VX_DP8X4;
        break;
    case VIR_IK_evis_dp4x8:
        opCode = VIR_OP_VX_DP4X8;
        break;
    case VIR_IK_evis_dp2x16:
        opCode = VIR_OP_VX_DP2X16;
        break;
   case VIR_IK_evis_dp32x1_b:
        opCode = VIR_OP_VX_DP32X1_B;
        break;
    case VIR_IK_evis_dp16x2_b:
        opCode = VIR_OP_VX_DP16X2_B;
        break;
    case VIR_IK_evis_dp8x4_b:
        opCode = VIR_OP_VX_DP8X4_B;
        break;
    case VIR_IK_evis_dp4x8_b:
        opCode = VIR_OP_VX_DP4X8_B;
        break;
    case VIR_IK_evis_dp2x16_b:
        opCode = VIR_OP_VX_DP2X16_B;
        break;
    case VIR_IK_evis_img_load:
        opCode = VIR_OP_VX_IMG_LOAD;
        break;
    case VIR_IK_evis_img_load_3d:
        opCode = VIR_OP_VX_IMG_LOAD_3D;
        break;
    case VIR_IK_evis_img_store:
        opCode = VIR_OP_VX_IMG_STORE;
        break;
    case VIR_IK_evis_img_store_3d:
        opCode = VIR_OP_VX_IMG_STORE_3D;
        break;
    case VIR_IK_evis_vload2:
    case VIR_IK_evis_vload3:
    case VIR_IK_evis_vload4:
    case VIR_IK_evis_vload8:
    case VIR_IK_evis_vload16:
        opCode = VIR_OP_LOAD;
        break;
    case VIR_IK_evis_vstore2:
        storeElements = 2;
        opCode = VIR_OP_STORE;
        break;
    case VIR_IK_evis_vstore3:
        storeElements = 3;
        opCode = VIR_OP_STORE;
        break;
    case VIR_IK_evis_vstore4:
        storeElements = 4;
        opCode = VIR_OP_STORE;
        break;
    case VIR_IK_evis_vstore8:
        storeElements = 8;
        opCode = VIR_OP_STORE;
        break;
    case VIR_IK_evis_vstore16:
        storeElements = 16;
        opCode = VIR_OP_STORE;
        break;
    case VIR_IK_evis_index_add:
        opCode = VIR_OP_VX_INDEXADD;
        break;
    case VIR_IK_evis_vert_min3:
        opCode = VIR_OP_VX_VERTMIN3;
        break;
    case VIR_IK_evis_vert_max3:
        opCode = VIR_OP_VX_VERTMAX3;
        break;
    case VIR_IK_evis_vert_med3:
        opCode = VIR_OP_VX_VERTMED3;
        break;
    case VIR_IK_evis_horz_min3:
        opCode = VIR_OP_VX_HORZMIN3;
        break;
    case VIR_IK_evis_horz_max3:
        opCode = VIR_OP_VX_HORZMAX3;
        break;
    case VIR_IK_evis_horz_med3:
        opCode = VIR_OP_VX_HORZMED3;
        break;
    case VIR_IK_evis_error:
        opCode = VIR_OP_ERROR;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* set op code */
    VIR_Inst_SetOpcode(pInst, opCode);

    /* set operands */
    argList = VIR_Operand_GetParameters(src1);
    gcmASSERT(argList->argNum <= VIR_OPCODE_GetSrcOperandNum(opCode));
    VIR_Inst_SetSrcNum(pInst, VIR_OPCODE_GetSrcOperandNum(opCode));
    for (i=0; i<argList->argNum; i++)
    {
        VIR_Operand * arg = argList->args[i];
        VIR_Inst_SetSource(pInst, i, arg);
        /* change the immediate to EvisModifier for EVIS inst if it is modifier operand */
        if (VIR_OPCODE_isVXOnly(opCode))
        {
            int evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(opCode);
            /* VIR_OP_VX_IADD can have two sources and three sources, if the src2 is 0
             * make sure it is supported type  */
            if (opCode == VIR_OP_VX_IADD && i == 2 && VIR_Operand_isValueZero(pShader, VIR_Inst_GetSource(pInst, i)))
            {
                VIR_Operand_SetTypeId(VIR_Inst_GetSource(pInst, i),
                                    VIR_Operand_GetTypeId(VIR_Inst_GetSource(pInst, 0)));
            }
            gcmASSERT(evisSrcNo >= 0 && (gctUINT)evisSrcNo < VIR_Inst_GetSrcNum(pInst));

            if (evisSrcNo == (int)i)
            {
                /* set newSrc to EVISModifier operand */
                VIR_Operand_SetOpKind(VIR_Inst_GetSource(pInst, i), VIR_OPND_EVIS_MODIFIER);
            }
            else if ((opCode == VIR_OP_VX_IMG_LOAD || opCode == VIR_OP_VX_IMG_LOAD_3D) && i == 2)
            {
                /* check img_load offset operand */
                VIR_Operand * opnd = VIR_Inst_GetSource(pInst, i);
                VIR_IMG_LOAD_SetImmOffset(pShader, pInst,opnd, gcvTRUE /* encoded */);
            }
        }
    }

    ON_ERROR(VIR_Function_FreeOperand(func, src0), "VIR_Function_FreeOperand");
    ON_ERROR(VIR_Function_FreeOperand(func, src1), "VIR_Function_FreeOperand");
    /* handling extra dest for some inst */
    if (!VIR_OPCODE_hasDest(opCode) && VIR_Inst_GetDest(pInst) != gcvNULL)
    {
        VIR_Function_FreeOperand(func, VIR_Inst_GetDest(pInst));
        VIR_Inst_SetDest(pInst, gcvNULL);
    }
    /* add undef operand for unset sources */
    for(i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
    {
        if (pInst->src[i] == gcvNULL)
        {
            ON_ERROR0(VIR_Function_NewOperand(func, &pInst->src[i]));
        }
    }

    /* change enable based on store width */
    if (storeElements > 0)
    {
        VIR_Operand  *  data = VIR_Inst_GetSource(pInst, 2);
        VIR_TypeId      tyId = VIR_Operand_GetTypeId(data);
        VIR_TypeId      componentTyId, newTyId;
        VIR_Enable      enable;
        VIR_Operand  *  dest = VIR_Inst_GetDest(pInst);
        gcmASSERT(VIR_TypeId_isPrimitive(tyId) && dest);
        componentTyId = VIR_GetTypeComponentType(tyId);
        newTyId = VIR_TypeId_ComposePackedNonOpaqueType(componentTyId, storeElements);
        enable = VIR_TypeId_Conv2Enable(newTyId);
        VIR_Operand_SetEnable(dest, enable);
    }
    evisSrcNo = VIR_OPCODE_EVISModifier_SrcNo(opCode);
    switch (opCode) {
    case VIR_OP_ERROR:
        {
            modifier = VIR_Inst_GetSource(pInst, 0);
            gcmASSERT(VIR_Operand_isImm(modifier));
            val = VIR_Operand_GetImmediateUint(modifier);
            switch (val) {
            case ERROR_DP2x16_NOT_SUPPORTED:
                fprintf(stderr, "Error: VXC_DP2x16() is not supported");
                break;
            case ERROR_IADD_NOT_SUPPORTED:
                fprintf(stderr, "Error: VXC_IAdd() is not supported");
                break;
            case ERROR_SELECTADD_NOT_SUPPORTED:
                fprintf(stderr, "Error: VXC_SelectAdd() is not supported");
                break;
            case ERROR_BITREPLACE_NOT_SUPPORTED:
                fprintf(stderr, "Error: VXC_BitReplace() is not supported");
                break;
            default:
                fprintf(stderr, "Error: unknown VXC operator is not supported");
                break;
            }
        }
        return VSC_ERR_NOT_SUPPORTED;
    case VIR_OP_VX_BILINEAR:
        /* BiLinear only support up to 7 outputs*/
        modifier = VIR_Inst_GetSource(pInst, evisSrcNo);
        val = VIR_Operand_GetImmediateUint(modifier);
        startBin =  VXC_GET_START_BIN(val);
        endBin   =  VXC_GET_END_BIN(val);
        if (endBin - startBin >= 7)
        {
            fprintf(stderr, "Error: %s [StartBin:%d, EndBin:%d] using more than 7 bins is not supported",
                            VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)), startBin, endBin);
            return VSC_ERR_NOT_SUPPORTED;
        }
        break;
   case VIR_OP_VX_MULSHIFT:
        /* NulShift only support up to 8 outputs*/
        modifier = VIR_Inst_GetSource(pInst, evisSrcNo);
        val = VIR_Operand_GetImmediateUint(modifier);
        startBin =  VXC_GET_START_BIN(val);
        endBin   =  VXC_GET_END_BIN(val);
        if (endBin - startBin >= 8)
        {
            fprintf(stderr, "Error: %s [StartBin:%d, EndBin:%d] using more than 8 bins is not supported",
                            VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)), startBin, endBin);
            return VSC_ERR_NOT_SUPPORTED;
        }
        break;
    default:
        break;
    }
    if (isVX2)
    {
        switch (opCode) {
        case VIR_OP_VX_SELECTADD:
        case VIR_OP_VX_DP2X16:
            fprintf(stderr, "Error: opcode %s is not supported in VX2", VIR_OPCODE_GetName(opCode));
            return VSC_ERR_NOT_SUPPORTED;
        case VIR_OP_VX_IACCSQ:
            /* IAccSQ will only support up to 8 outputs*/
            modifier = VIR_Inst_GetSource(pInst, evisSrcNo);
            val = VIR_Operand_GetImmediateUint(modifier);
            startBin =  VXC_GET_START_BIN(val);
            endBin   =  VXC_GET_END_BIN(val);
            if (endBin - startBin >= 8)
            {
                fprintf(stderr, "Error: %s [StartBin:%d, EndBin:%d] using more than 8 bins is not supported in VX2",
                                VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)), startBin, endBin);
                return VSC_ERR_NOT_SUPPORTED;
            }
            break;
        case VIR_OP_VX_LERP:
            /* LERP will only support up to 7 outputs */
            modifier = VIR_Inst_GetSource(pInst, evisSrcNo);
            val = VIR_Operand_GetImmediateUint(modifier);
            startBin =  VXC_GET_START_BIN(val);
            endBin   =  VXC_GET_END_BIN(val);
            if (endBin - startBin >= 7)
            {
                fprintf(stderr, "Error: %s [StartBin:%d, EndBin:%d] using more than 7 bins is not supported in VX2",
                                VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)), startBin, endBin);
                return VSC_ERR_NOT_SUPPORTED;
            }
            break;
        default:
            break;
        }
    }
    else /* VX1 */
    {
        switch (opCode) {
        case VIR_OP_VX_SELECTADD:
            /* SelectAdd only support up to 8 outputs*/
            modifier = VIR_Inst_GetSource(pInst, evisSrcNo);
            val = VIR_Operand_GetImmediateUint(modifier);
            startBin =  VXC_GET_START_BIN(val);
            endBin   =  VXC_GET_END_BIN(val);
            if (endBin - startBin >= 8)
            {
                fprintf(stderr, "Error: %s [StartBin:%d, EndBin:%d] using more than 8 bins is not supported",
                                VIR_OPCODE_GetName(VIR_Inst_GetOpcode(pInst)), startBin, endBin);
                return VSC_ERR_NOT_SUPPORTED;
            }
            break;
        case VIR_OP_VX_INDEXADD:
        case VIR_OP_VX_VERTMIN3:
        case VIR_OP_VX_VERTMAX3:
        case VIR_OP_VX_VERTMED3:
        case VIR_OP_VX_HORZMIN3:
        case VIR_OP_VX_HORZMAX3:
        case VIR_OP_VX_HORZMED3:
            fprintf(stderr, "Error: VX2 opcode %s is not supported in non-VX2 chip", VIR_OPCODE_GetName(opCode));
            return VSC_ERR_NOT_SUPPORTED;
        default:
            break;
        }
    }

    if (opCode == VIR_OP_VX_DP16X1_B ||
        opCode == VIR_OP_VX_DP8X2_B  ||
        opCode == VIR_OP_VX_DP4X4_B  ||
        opCode == VIR_OP_VX_DP2X8_B  ||
        opCode == VIR_OP_VX_DP32X1_B ||
        opCode == VIR_OP_VX_DP16X2_B ||
        opCode == VIR_OP_VX_DP8X4_B  ||
        opCode == VIR_OP_VX_DP4X8_B  ||
        opCode == VIR_OP_VX_DP2X16_B)
    {
        /* set the src0 to be higher part of temp 256 register pair */
        VIR_Operand_SetFlag(VIR_Inst_GetSource(pInst, 0), VIR_OPNDFLAG_TEMP256_HIGH);
        /* set the src1 to be lower part of temp 256 register pair */
        VIR_Operand_SetFlag(VIR_Inst_GetSource(pInst, 1), VIR_OPNDFLAG_TEMP256_LOW);
    }

OnError:
    return errCode;
}

VSC_ErrCode
_processIntrinsic(
    IN VIR_Shader         * pShader,
    IN VIR_Instruction    * pInst,
    VIR_OpCode            opCode
    )
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Function *    func = VIR_Inst_GetFunction(pInst);
    VIR_Operand  *    src0 = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand  *    src1 = VIR_Inst_GetSource(pInst, 1);
    gctUINT           i;
    VIR_ParmPassing * argList;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_INTRINSIC);
    if (VIR_Operand_GetOpKind(src1) != VIR_OPND_PARAMETERS)
    {
        return errCode;
    }

    /* set op code */
    VIR_Inst_SetOpcode(pInst, opCode);

    /* set operands */
    argList = VIR_Operand_GetParameters(src1);
    gcmASSERT(argList->argNum <= VIR_OPCODE_GetSrcOperandNum(opCode));
    VIR_Inst_SetSrcNum(pInst, VIR_OPCODE_GetSrcOperandNum(opCode));
    for (i=0; i<argList->argNum; i++)
    {
        VIR_Inst_SetSource(pInst, i, argList->args[i]);
    }

    ON_ERROR(VIR_Function_FreeOperand(func, src0), "VIR_Function_FreeOperand");
    ON_ERROR(VIR_Function_FreeOperand(func, src1), "VIR_Function_FreeOperand");
    /* handling extra dest for some inst */
    if (!VIR_OPCODE_hasDest(opCode) && VIR_Inst_GetDest(pInst) != gcvNULL)
    {
        VIR_Function_FreeOperand(func, VIR_Inst_GetDest(pInst));
        VIR_Inst_SetDest(pInst, gcvNULL);
    }
    /* add undef operand for unset sources */
    for(i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
    {
        if (pInst->src[i] == gcvNULL)
        {
            ON_ERROR0(VIR_Function_NewOperand(func, &pInst->src[i]));
        }
    }
OnError:
    return errCode;
}

/*********** At Middle Level Process intrinsics *************/

VSC_ErrCode
VIR_Lower_MiddleLevel_Process_Intrinsics(
    IN  VIR_Shader   *Shader
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode        *func_node;
    VIR_Function            *func;

    if(!VIR_Shader_IsCL(Shader)) return errCode;
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator            inst_iter;
        VIR_Instruction             *inst;

        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
                {
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_INTRINSIC)
            {
                VIR_Operand *   src0 = VIR_Inst_GetSource(inst, 0);
                VIR_IntrinsicsKind ik = VIR_Operand_GetIntrinsicKind(src0);

                if (ik > VIR_IK_evis_begin && ik < VIR_IK_evis_end)
                {
                    /* process EVIS instructions */
                    ON_ERROR0(_processEvisIntrinsic(Shader, inst, ik));
                    continue;
                }
                else
                {
                    VIR_OpCode opCode = VIR_OP_NOP;
                    switch (ik)
                    {
                    case VIR_IK_swizzle:
                        opCode = VIR_OP_SWIZZLE;
                        break;

                    case VIR_IK_swizzle_full_def:
                        opCode = VIR_OP_SWIZZLE;
                        VIR_Inst_SetFlag(inst, VIR_INSTFLAG_FULL_DEF);
                        break;

                    case VIR_IK_any:
                        opCode = VIR_OP_ANY;
                        break;

                    case VIR_IK_all:
                        opCode = VIR_OP_ALL;
                        break;

                    case VIR_IK_madsat:
                        opCode = VIR_OP_MADSAT;
                        break;

                    case VIR_IK_imadhi0:
                        opCode = VIR_OP_IMADHI0;
                        break;

                    case VIR_IK_imadlo0:
                        opCode = VIR_OP_IMADLO0;
                        break;

                    case VIR_IK_bitextract:
                        opCode = VIR_OP_BITEXTRACT;
                        break;

                    case VIR_IK_bitinsert:
                        opCode = VIR_OP_BITINSERT;
                        break;

                    default:
                        break;
                    }

                    if (opCode != VIR_OP_NOP)
                    {
                        ON_ERROR0(_processIntrinsic(Shader, inst, opCode));
                        continue;
                    }
                }
            }
        }
    }
OnError:
    return errCode;
}

/* HL2ML: includes lowering the intrinsic function.
   Most cases, it is a one to one mapping or simple one to N mapping.
   Lowering that depends on HW features will be in ML2LL. Complicated lowering
   will be in ML2LL as well, through linking builtin library.
*/
VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  VIR_PatternHL2MLContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, Context->vscContext, Shader, Context->pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                  _GetHL2MLPatternPhaseExpand, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_LowLevel_Expand failed.");

    VIR_PatternContext_Finalize(&Context->header);

    return errCode;
}

