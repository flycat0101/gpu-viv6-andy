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


#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

gctBOOL
VIR_Lower_SetZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 0.0f;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSmallestPositive(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 1.175494351e-038f;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetZeroOrSamllestPositive(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_Lower_HasHalt4(Context, Inst) || VIR_Shader_GetKind(Context->shader) == VIR_SHADER_CL)
    {
        VIR_Lower_SetZero(Context, Inst, Opnd);
    }
    else
    {
        VIR_Lower_SetSmallestPositive(Context, Inst, Opnd);
    }
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetIntZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 1.0f,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetMinusOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = -1.0f,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetIntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 1,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}


gctBOOL
VIR_Lower_SetIntMinusOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = -1,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_AdjustCoordSwizzleForShadow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId samplerType = Inst->src[0]->_opndType;
    VIR_Swizzle newSwizzle = VIR_Operand_GetSwizzle(Opnd);

    switch (samplerType)
    {
    case VIR_TYPE_SAMPLER_2D_SHADOW:
    case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
        newSwizzle = VIR_SWIZZLE_XYYY;
        break;

    case VIR_TYPE_SAMPLER_CUBE_SHADOW:
    case VIR_TYPE_SAMPLER_2D_ARRAY_SHADOW:
        newSwizzle = VIR_SWIZZLE_XYZZ;
        break;

    case VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW:
        gcmASSERT((VIR_TexModifier_Flag)VIR_Operand_GetTexModifierFlag(Opnd) == VIR_TMFLAG_NONE);
        newSwizzle = VIR_SWIZZLE_XYZZ;
        break;

    default:
        break;
    }
    VIR_Operand_SetSwizzle(Opnd, newSwizzle);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleByCoord(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction * prevInst = VIR_Inst_GetPrev(Inst);

    VIR_Operand_SetSwizzle(Opnd, VIR_Enable_2_Swizzle(VIR_Operand_GetEnable(prevInst->dest)));
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleXYZW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYZW);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleXEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swiz       = VIR_Operand_GetSwizzle(Opnd);

    swiz = VIR_Swizzle_GetChannel(swiz, 0);

    swiz |= swiz << 2;
    swiz |= swiz << 4;

    VIR_Operand_SetSwizzle(Opnd, swiz);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleYEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swiz       = VIR_Operand_GetSwizzle(Opnd);

    swiz = VIR_Swizzle_GetChannel(swiz, 1);

    swiz |= swiz << 2;
    swiz |= swiz << 4;

    VIR_Operand_SetSwizzle(Opnd, swiz);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleZEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swiz       = VIR_Operand_GetSwizzle(Opnd);

    swiz = VIR_Swizzle_GetChannel(swiz, 2);

    swiz |= swiz << 2;
    swiz |= swiz << 4;

    VIR_Operand_SetSwizzle(Opnd, swiz);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleWEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swiz       = VIR_Operand_GetSwizzle(Opnd);

    swiz = VIR_Swizzle_GetChannel(swiz, 3);

    swiz |= swiz << 2;
    swiz |= swiz << 4;

    VIR_Operand_SetSwizzle(Opnd, swiz);

    return gcvTRUE;
}


VIR_PrimitiveTypeId
VIR_Lower_GetBaseType(
    IN VIR_Shader  *Shader,
    IN VIR_Operand *Opnd
    )
{
    VIR_Type           *type     = gcvNULL;
    VIR_PrimitiveTypeId baseType;

    gcmASSERT(Opnd != gcvNULL);

    type = VIR_Shader_GetTypeFromId(Shader,
        VIR_Operand_GetType(Opnd));
    gcmASSERT(VIR_Type_isPrimitive(type));

    baseType = VIR_Type_GetBaseTypeId(type);

    return baseType;
}


gctBOOL
VIR_Lower_IsIntOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseType;

    gcmASSERT(Inst->dest != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}


gctBOOL
VIR_Lower_IsFloatOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseType;

    gcmASSERT(Inst->dest != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISFLOAT)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}


gctBOOL
VIR_Lower_IsNotCLShader(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (Context->shader->shaderKind != VIR_SHADER_CL);
}

gctBOOL
VIR_Lower_HasHalt4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hasHalti4;
}

gctBOOL
VIR_Lower_enableFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ENABLE_FULL_NEW_LINKER;
}

gctBOOL
VIR_Lower_disableFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !ENABLE_FULL_NEW_LINKER;
}

