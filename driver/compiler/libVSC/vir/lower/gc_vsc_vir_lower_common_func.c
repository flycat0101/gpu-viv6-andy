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


#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

gctBOOL
VIR_Lower_SetOpndNeg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Modifier           modifier = VIR_Operand_GetModifier(Opnd);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NEG | modifier);

    return gcvTRUE;
}

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
    if(VIR_Lower_HasHalt4(Context, Inst) || VIR_Shader_IsCL(Context->shader))
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
VIR_Lower_SetIntHighBitOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x80000000;

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

    imm0.fValue = 1.0f;

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

    imm0.fValue = -1.0f;

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

    imm0.iValue = 1;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetUIntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 1;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
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

    imm0.iValue = -1;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableXYZWAndSymType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol*     symbol = VIR_Operand_GetSymbol(Opnd);
    VIR_TypeId      typeId =  VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Opnd));

    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XYZW);
    VIR_Symbol_SetTypeId(symbol, VIR_TypeId_ComposeNonOpaqueType(typeId, 4, 1));
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
    VIR_TypeId samplerType = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
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

    VIR_Operand_SetSwizzle(Opnd, VIR_Enable_2_Swizzle(VIR_Operand_GetEnable(VIR_Inst_GetDest(prevInst))));
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
VIR_Lower_SetSwizzleXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYYY);
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
        VIR_Operand_GetTypeId(Opnd));
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

    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISINTEGER)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Lower_IsDstFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   typeId   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));

    gcmASSERT(typeId < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isFloat(typeId);
}

gctBOOL
VIR_Lower_IsSrc0Unsigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand  *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId   typeId;

    gcmASSERT(src0);
    if(src0 == gcvNULL) return gcvFALSE;

    typeId= VIR_Operand_GetTypeId(src0);

    gcmASSERT(typeId < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isUnSignedInteger(typeId);
}

gctBOOL
VIR_Lower_IsDstInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isInteger(ty);
}

gctBOOL
VIR_Lower_IsDstInt32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_GetTypeComponentType(ty) == VIR_TYPE_INT32
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_UINT32;
}

gctBOOL
VIR_Lower_IsDstInt16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_GetTypeComponentType(ty) == VIR_TYPE_INT16
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_UINT16;
}

gctBOOL
VIR_Lower_IsDstIntPacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isInteger(ty) && VIR_TypeId_isPacked(ty);
}

gctBOOL
VIR_Lower_IsDstNotIntPacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !VIR_Lower_IsDstIntPacked(Context, Inst);
}

gctBOOL
VIR_Lower_IsDstSigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_GetTypeComponentType(ty) == VIR_TYPE_INT32
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_INT16
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_INT8;
}

gctBOOL
VIR_Lower_IsDstUnsigned(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_GetTypeComponentType(ty) == VIR_TYPE_UINT32
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_UINT16
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_UINT8
        || VIR_GetTypeComponentType(ty) == VIR_TYPE_BOOLEAN;
}

gctBOOL
VIR_Lower_IsFloatOpcode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseType;

    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

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
    return !VIR_Shader_IsCL(Context->shader);
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
    return gcUseFullNewLinker(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2);
}

gctBOOL
VIR_Lower_disableFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !gcUseFullNewLinker(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2);
}

gctBOOL
VIR_Lower_HasTexldModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_OpCode             opcode = VIR_Inst_GetOpcode(Inst);
    VIR_Operand           *opnd = gcvNULL;
    VIR_ParmPassing       *parmOpnd = gcvNULL;

    /* For intrinsic, the texld modifier should be saved in 3nd param.*/
    if (opcode == VIR_OP_INTRINSIC)
    {
        opnd = VIR_Inst_GetSource(Inst, 1);
        parmOpnd = VIR_Operand_GetParameters(opnd);

        if (parmOpnd->argNum > 2)
        {
            opnd = parmOpnd->args[2];
            if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_TEXLDPARM)
            {
                return gcvTRUE;
            }
        }
    }
    /* For other, the modifier should be saved in 3nd source. */
    else if (VIR_Inst_GetSrcNum(Inst) > 2)
    {
        opnd = VIR_Inst_GetSource(Inst, 2);
        if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_TEXLDPARM)
        {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

gctBOOL
VIR_Lower_label_set_jmp_n(
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

gctBOOL
VIR_Lower_label_set_jmp_neg2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -2);
}

gctBOOL
VIR_Lower_label_set_jmp_neg3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -3);
}

gctBOOL
VIR_Lower_label_set_jmp_neg3_6(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if (!VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -3))
    {
        return gcvFALSE;
    }

    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -6);
}

gctBOOL
VIR_Lower_label_set_jmp_neg10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -10);
}

gctBOOL
VIR_Lower_label_set_jmp_neg22(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -22);
}

static gctBOOL
_SetValueType0(
    IN VIR_PatternContext  *Context,
    IN VIR_Operand         *Opnd,
    IN VIR_PrimitiveTypeId  Format
    )
{
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(Format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndUINT32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_UINT32);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndINT32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_INT32);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndUINT16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_UINT16);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndINT16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_INT16);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
_SetEnableBaseOnSrc(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *SrcOpnd,
    OUT VIR_Operand       *Opnd)
{
    VIR_TypeId            opndTypeId;
    VIR_Enable            enable = VIR_ENABLE_NONE;
    VIR_ParmPassing       *parm;

    if (VIR_Operand_isParameters(SrcOpnd))
    {
        parm = VIR_Operand_GetParameters(SrcOpnd);
        gcmASSERT(parm->argNum >= 1);

        opndTypeId = VIR_Operand_GetTypeId(parm->args[0]);
    }
    else
    {
        opndTypeId = VIR_Operand_GetTypeId(SrcOpnd);
    }

    /* Change enable and type. */
    switch(VIR_GetTypeComponents(opndTypeId))
    {
    case 1:
        enable = VIR_ENABLE_X;
        break;
    case 2:
        enable = VIR_ENABLE_XY;
        break;
    case 3:
        enable = VIR_ENABLE_XYZ;
        break;
    case 4:
        enable = VIR_ENABLE_XYZW;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VIR_Operand_SetEnable(Opnd, enable);
    VIR_Operand_SetTypeId(Opnd, opndTypeId);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableBaseOnSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _SetEnableBaseOnSrc(Context, Inst, VIR_Inst_GetSource(Inst, 0), Opnd);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableBaseOnSrc1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    _SetEnableBaseOnSrc(Context, Inst, VIR_Inst_GetSource(Inst, 1), Opnd);

    return gcvTRUE;
}

VIR_TexModifier_Flag
VIR_Lower_GetTexModifierKind(
    IN VIR_Operand        *Opnd
    )
{
    if (Opnd == gcvNULL || VIR_Operand_GetOpKind(Opnd) != VIR_OPND_TEXLDPARM)
    {
        return VIR_TMFLAG_NONE;
    }

    return (VIR_TexModifier_Flag)VIR_Operand_GetTexModifierFlag(Opnd);
}

