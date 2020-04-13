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


#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

void
VIR_Lower_Initialize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternLowerContext  *Context,
    IN VSC_HW_CONFIG            *HwCfg,
    IN VSC_MM                   *pMM
    )
{
    Context->hwCfg = HwCfg;
    Context->pMM = pMM;

    Context->hasNEW_TEXLD = HwCfg->hwFeatureFlags.hasHalti2;

    if (HwCfg->hwFeatureFlags.hasSHEnhance2)
    {
        if (gcmOPT_NOIMMEDIATE())
            Context->generateImmediate  = gcvFALSE;
        else
            Context->generateImmediate  = gcvTRUE;
    }
    else
    {
        Context->generateImmediate  = gcvFALSE;
    }

    Context->isCL_X  = (gctBOOL)HwCfg->hwFeatureFlags.needCLXFixes;

    Context->hasCL   = Context->isCL_X || (gctBOOL)HwCfg->hwFeatureFlags.needCLXEFixes;

    Context->hasHalti1 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti1;
    Context->hasHalti2 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti2;
    Context->hasHalti3 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti3;
    Context->hasHalti4 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti4;
    Context->hasSHEnhancements2 = (gctBOOL)HwCfg->hwFeatureFlags.hasSHEnhance2;
}

gctBOOL
VIR_Lower_InstSupportFP16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Shader_CapabilityFP16(Context->shader);
}

gctBOOL
VIR_Lower_HasHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hasHalti4;
}


gctBOOL
VIR_Lower_HasNoHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !VIR_Lower_HasHalti4(Context, Inst);
}

gctBOOL
VIR_Lower_HasNoFloatingMadFix(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasFloatingMadFix;
}

gctBOOL
VIR_Lower_SetImm0xFFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0xFFFF;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetImm16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 16;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetImm0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleXAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleZAndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleZAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
    return gcvTRUE;
}

static VSC_ErrCode
VIR_Lower_SetOpndIndex1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = Context->shader;
    VIR_Symbol*         pSym = VIR_Operand_GetSymbol(Opnd);

    gcmASSERT(VIR_Operand_isSymbol(Opnd));

    /*
    ** 1) The operand is a temp register, just use the next one.
    ** 2) The operand is a uniform, use the constIndex.
    */
    if (VIR_Symbol_isVreg(pSym))
    {
        VIR_SymId       nextRegSymId = VIR_INVALID_ID;
        errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader,
                                                    VIR_Symbol_GetVregIndex(pSym) + 1,
                                                    &nextRegSymId);
        ON_ERROR(errCode, "get vreg symbol.");
        VIR_Operand_SetSym(Opnd, VIR_Shader_GetSymFromId(pShader, nextRegSymId));
    }
    else
    {
        VIR_Operand_SetIsConstIndexing(Opnd, gcvTRUE);
        VIR_Operand_SetRelIndex(Opnd, 1);
    }

OnError:
    return errCode;
}

gctBOOL
VIR_Lower_SetSwizzleXIndex_1AndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT(VIR_Operand_isSymbol(Opnd));
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
    VIR_Lower_SetOpndIndex1(Context, Inst, Opnd);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleYIndex_1AndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT(VIR_Operand_isSymbol(Opnd));
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);
    VIR_Lower_SetOpndIndex1(Context, Inst, Opnd);

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetSwizzleZIndex_1AndUintType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT(VIR_Operand_isSymbol(Opnd));
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);
    VIR_Lower_SetOpndIndex1(Context, Inst, Opnd);

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
VIR_Lower_SetEnableXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetEnableXAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
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
VIR_Lower_SetEnableYAndSrc0Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)));
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
VIR_Lower_SetEnableZAndIntType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
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
VIR_Lower_SetEnableXYZAndInt3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XYZ);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INTEGER_X3);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndNeg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_NegateOperand(Context->shader, Opnd);
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
    if(VIR_Lower_HasHalti4(Context, Inst) || VIR_Shader_IsCLFromLanguage(Context->shader))
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
VIR_Lower_SetUIntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 1,

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

    imm0.iValue = -1,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
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
        newSwizzle = VIR_SWIZZLE_XYYY;
        break;

    case VIR_TYPE_SAMPLER_CUBE_SHADOW:
    /* We treat 1DArray as a 2DArray. */
    case VIR_TYPE_SAMPLER_1D_ARRAY_SHADOW:
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
VIR_Lower_SetSwizzleXYZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYZZ);
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

/* Count the number of enables using the 4bit VIR_ENABLE_* as indexing to table */
static gctUINT
_enableCount[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

gctBOOL
VIR_Lower_IsDstOneEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    return _enableCount[enable] == 1;
}

gctBOOL
VIR_Lower_IsDstTwoEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    return _enableCount[enable] == 2;
}

gctBOOL
VIR_Lower_IsDstThreeEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    return _enableCount[enable] == 3;
}

gctBOOL
VIR_Lower_IsDstFourEnables(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Inst_GetEnable(Inst) == VIR_ENABLE_XYZW;
}

gctBOOL
VIR_Lower_IsDstMoreThanOneEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !VIR_Lower_IsDstOneEnable(Context, Inst);
}

gctBOOL
VIR_Lower_IsDstBool(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   typeId   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));

    gcmASSERT(typeId < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isBoolean(typeId);
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
VIR_Lower_IsDstFP16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId   typeId   = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));

    gcmASSERT(typeId < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return VIR_TypeId_isFloat(typeId) && VIR_GetTypeSize(VIR_GetTypeComponentType(typeId)) == 2;
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
VIR_Lower_IsDstMediumpOrLowp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Precision destPrecision = VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst));

    if (destPrecision != VIR_PRECISION_HIGH)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Lower_IsDstHighp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Precision destPrecision = VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst));

    if (destPrecision == VIR_PRECISION_HIGH)
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

    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

    if(VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISFLOAT)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Lower_IsSrc1FloatConstant(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseType;
    VIR_Operand *       src1 = VIR_Inst_GetSource(Inst, 1);

    gcmASSERT(src1 != gcvNULL);

    baseType = VIR_Lower_GetBaseType(Context->shader, src1);

    if ((VIR_GetTypeFlag(baseType) & VIR_TYFLAG_ISFLOAT) &&
       (VIR_Operand_isImm(src1) || VIR_Operand_isConst(src1)))
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
    return !VIR_Shader_IsCLFromLanguage(Context->shader);
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
VIR_Lower_IsI2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));
    VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

    if ((VIR_GetTypeFlag(baseTy0) & VIR_TYFLAG_ISINTEGER) &&
        (VIR_GetTypeFlag(baseTy1) & VIR_TYFLAG_ISINTEGER))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
VIR_Lower_SameSizeType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !VIR_Lower_NotSameSizeType(Context, Inst);
}

gctBOOL
VIR_Lower_NotSameSizeType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId dstTyId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));
    VIR_TypeId srcTyId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)));

    return (VIR_GetTypeSize(dstTyId) != VIR_GetTypeSize(srcTyId))
        || (VIR_Operand_GetRoundMode(VIR_Inst_GetSource(Inst, 0)) != VIR_ROUND_DEFAULT)
        || (VIR_Operand_GetModifier(VIR_Inst_GetSource(Inst, 0)) != VIR_MOD_NONE)
        || (VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst)) != VIR_MOD_NONE);
}

gctBOOL
VIR_Lower_SameType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctBOOL result = gcvFALSE;

    result = (VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)))
              == VIR_GetTypeComponentType(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)))) &&
             VIR_Operand_GetRoundMode(VIR_Inst_GetSource(Inst, 0)) == VIR_ROUND_DEFAULT        &&
             VIR_Operand_GetModifier(VIR_Inst_GetSource(Inst, 0)) == VIR_MOD_NONE              &&
             VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst)) == VIR_MOD_NONE;

    return result;
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
VIR_Lower_SetOpndUINT32HP(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_UINT32);
    VIR_Operand_SetPrecision(Opnd, VIR_PRECISION_HIGH);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetOpndHP(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetPrecision(Opnd, VIR_PRECISION_HIGH);
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

gctBOOL
VIR_Lower_SetPrecisionBaseOnSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand* src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand_SetPrecision(Opnd, VIR_Operand_GetPrecision(src0));
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetHighp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol          *pSym = gcvNULL;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(Opnd);

    if (opndKind == VIR_OPND_VIRREG ||
        opndKind == VIR_OPND_SYMBOL ||
        opndKind == VIR_OPND_SAMPLER_INDEXING)
    {
        pSym = VIR_Operand_GetSymbol(Opnd);
        VIR_Symbol_SetPrecision(pSym, VIR_PRECISION_HIGH);
    }

    VIR_Operand_SetPrecision(Opnd, VIR_PRECISION_HIGH);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_ReverseCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
    VIR_Inst_SetConditionOp(Inst, VIR_ConditionOp_Reverse(condOp));
    return gcvTRUE;
}

gctBOOL
VIR_Lower_ResetCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_ChangeSignedIntegerToUnsignedInteger(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId  opndTypeId = VIR_Operand_GetTypeId(Opnd);

    opndTypeId = VIR_TypeId_ConvertIntegerType(Context->shader, opndTypeId, gcvTRUE);
    VIR_Operand_SetTypeId(Opnd, opndTypeId);

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

gctBOOL
VIR_Lower_MatchDual16Req(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *DestOpnd,
    IN VIR_Operand        *SrcOpnd
    )
{
    VIR_TypeId ty0 = VIR_Operand_GetTypeId(DestOpnd);
    VIR_TypeId ty1 = VIR_Operand_GetTypeId(SrcOpnd);

    if (Context->shader->shaderKind != VIR_SHADER_FRAGMENT)
    {
        return gcvFALSE;
    }

    /* if dest and src0 are same type */
    if(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
        (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
       ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) &&
        (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER)))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL
VIR_Lower_SkipOperand(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return gcvTRUE;
}

