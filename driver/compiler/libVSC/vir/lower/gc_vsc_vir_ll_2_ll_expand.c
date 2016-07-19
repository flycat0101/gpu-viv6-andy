/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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
_requireSIN_COS_TAN_Precision(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Shader_GetPatchId(Context->shader) == gcvPATCH_DEQP;
}

static gctBOOL
_hasSQRT_TRIG(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasSqrtTrig;
}


static gctBOOL
_set_HighPrecision(
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

static gctBOOL
_set_RTZ_HighPrecision(
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

    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_RTZ);
    VIR_Operand_SetPrecision(Opnd, VIR_PRECISION_HIGH);
    return gcvTRUE;
}

static gctBOOL
_set_ABS(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_Operand_SetModifier(Opnd, VIR_MOD_ABS);
    return gcvTRUE;
}

static gctBOOL
rcppi2_1_dot5_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 1.0f / (2.0f * (float) M_PI);
    imm1.fValue = 0.5f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);

    return gcvTRUE;
}

static gctBOOL
pi2_1_pi_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 2.0f * (float) M_PI;
    imm1.fValue = -(float) M_PI;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);
    return gcvTRUE;
}

static gctBOOL
rcppi(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 1.0f / (float) M_PI;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
rcppi2_1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 2.0f / (float) M_PI;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
rcp2pi(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 0.5f / (float) M_PI;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
rcppi_low(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.uValue = 0x325c9c88;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
two(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = 2.0f;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
eight(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 8;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
half_pi(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = (float) M_PI / 2.0f;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_115_pi(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.fValue = (float) M_PI * 115.0f;    /* tuned for deqp precision tests for sin, cos & tan */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
sin_one_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue =0.999979376792907710000000f, /*1.0f,*/

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
tan9_1_tan7_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 5237760.0f / 239500800.0f;
    imm1.fValue = 65280.0f / 1209600.0f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);

    return gcvTRUE;
}

static gctBOOL
tan5_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 4032.0f / 30240.0f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
tan3_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 240.0f / 720.0f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
asin9_1_asin7_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 35.0f / 1152.0f;
    imm1.fValue = 5.0f / 112.0f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);
    return gcvTRUE;
}

static gctBOOL
asin5_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 3.0f / 40.0f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
asin3_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 1.0f / 6.0f,

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}


static gctBOOL
atan9_1_atan7_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 0.023060280510707944f;
    imm1.fValue = -0.09045060332177933f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);

    return gcvTRUE;
}

static gctBOOL
atan5_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 0.18449097954748866f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
atan3_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = -0.33168528523552876f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
factor9_1_factor7_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue =0.000002147873374269693200f;
    imm1.fValue =-0.000192650026292540130000f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);

    return gcvTRUE;
}

static gctBOOL
factor8_1_factor6_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.fValue = 0.000018929871657746844000f;
    imm1.fValue = 0.001342294854111969500000f;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm1);

    return gcvTRUE;
}

static gctBOOL
factor5_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue =  0.008308985270559787800000f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
factor4_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue =0.041518036276102066000000f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
factor3_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = -0.166624382138252260000000f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
factor2_2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = -0.499851584434509280000000f;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_FLOAT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_isSrcZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN gctUINT SrcIndex
    )
{
    gcmASSERT(SrcIndex < VIR_MAX_SRC_NUM);
    if(Inst->src[SrcIndex] == gcvNULL)
        return gcvFALSE;
    if(VIR_Operand_GetOpKind(Inst->src[SrcIndex]) != VIR_OPND_IMMEDIATE)
        return gcvFALSE;
    if(Inst->src[SrcIndex]->u1.uConst != 0)
        return gcvFALSE;

    return gcvTRUE;
}

static gctBOOL
_isSrc0Zero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcZero(Context, Inst, 0);
}

static gctBOOL
_isSrc1Zero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcZero(Context, Inst, 1);
}

static gctBOOL
_isSrcFloatOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN gctUINT SrcIndex
    )
{
    gcmASSERT(SrcIndex < VIR_MAX_SRC_NUM);
    if(Inst->src[SrcIndex] == gcvNULL)
        return gcvFALSE;
    if(VIR_Operand_GetOpKind(Inst->src[SrcIndex]) != VIR_OPND_IMMEDIATE)
        return gcvFALSE;
    if(Inst->src[SrcIndex]->u1.fConst != 1.0)
        return gcvFALSE;

    return gcvTRUE;
}

static gctBOOL
_isSrc0FloatOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcFloatOne(Context, Inst, 0);
}

static gctBOOL
_isNeedToSetLod(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (Context->shader->shaderKind != VIR_SHADER_FRAGMENT);
}

static gctBOOL
_hasNEW_TEXLD(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hasNEW_TEXLD;
}

static gctBOOL
_hasSIGN_FLOOR_CEIL(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasSignFloorCeil);
}

static gctBOOL
_has_getEXP_getMANT(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
#if _HAS_GETEXPT_GETMANT_
    return gcvTRUE;
#endif
    return gcvFALSE;
}

static gctBOOL
_hasRounding_mode(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasSHEnhance2)
    {
        gcmASSERT(Inst != gcvNULL);
        gcmASSERT(Inst->dest != gcvNULL);

        return VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(Inst->dest);
    }

    return gcvFALSE;
}

static gctBOOL
_isCLShader(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (Context->shader->shaderKind == VIR_SHADER_CL);
}

gctBOOL
_llDebugHelper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return gcvTRUE;
}

static gctBOOL
crossSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle0 = VIR_Operand_GetSwizzle(Inst->src[0]);
    VIR_Swizzle swizzle1 = VIR_Operand_GetSwizzle(Inst->src[1]);

    swizzle0 = (((swizzle0 >> 4) & 3) << 0)
             | (((swizzle0 >> 0) & 3) << 2)
             | (((swizzle0 >> 2) & 3) << 4)
             | (((swizzle0 >> 2) & 3) << 6);
    swizzle1 = (((swizzle1 >> 2) & 3) << 0)
             | (((swizzle1 >> 4) & 3) << 2)
             | (((swizzle1 >> 0) & 3) << 4)
             | (((swizzle1 >> 0) & 3) << 6);

    VIR_Operand_SetSwizzle(Inst->src[0], swizzle0);
    VIR_Operand_SetSwizzle(Inst->src[1], swizzle1);

    return gcvTRUE;
}

static gctBOOL
_isRAEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Shader_isRAEnabled(Context->shader);
}

static gctBOOL
_isRAEnabled_src0_prim_ctp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            (VIR_Operand_IsPerPatch(Inst->src[0]) ||
             VIR_Operand_IsArrayedPerVertex(Inst->src[0])));
}

static gctBOOL
_isRAEnabled_dest_prim_ctp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            (VIR_Operand_IsPerPatch(Inst->dest) ||
             VIR_Operand_IsArrayedPerVertex(Inst->dest)));
}

/* sampler index should always be integer */
static gctBOOL
_isRAEnabled_src0_not_sampler_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
           !VIR_TypeId_isSampler(VIR_Operand_GetType(Inst->src[0])) &&
           (VIR_GetTypeFlag(VIR_Operand_GetType(Inst->src[1])) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_isRAEnabled_dest_not_sampler_src0_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
        !VIR_TypeId_isSampler(VIR_Operand_GetType(Inst->dest)) &&
        (VIR_GetTypeFlag(VIR_Operand_GetType(Inst->src[0])) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
int_value_type0_const_7F800000(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0x7F800000;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_23(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 23;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_Minus127(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0xFFFFFF81;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_7FFFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0x007FFFFF;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_800000(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0x00800000;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_setMOVAEnableInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->dest, VIR_TYPE_INT32);
    VIR_Operand_SetEnable(Inst->dest,
        VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));
    gcmASSERT(VIR_Symbol_isVreg(VIR_Operand_GetSymbol(Inst->dest)));
    VIR_Symbol_SetStorageClass(VIR_Operand_GetSymbol(Inst->dest), VIR_STORAGE_INDEX_REGISTER);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(Inst->dest), VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_INT32));
    return gcvTRUE;
}

static gctBOOL
_setMOVAEnableFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->dest, VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(Inst->dest,
       VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));
    gcmASSERT(VIR_Symbol_isVreg(VIR_Operand_GetSymbol(Inst->dest)));
    VIR_Symbol_SetStorageClass(VIR_Operand_GetSymbol(Inst->dest), VIR_STORAGE_INDEX_REGISTER);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(Inst->dest), VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_FLOAT32));
    return gcvTRUE;
}


static gctBOOL
_setLDARRSwizzleInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->src[1], VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setLDARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->src[1], VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->src[0], VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Inst->src[0], VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
destEnableW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(Inst->dest, VIR_ENABLE_W);
    return gcvTRUE;
}

static gctBOOL
_isI2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasSHEnhance2)
    {
        VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, Inst->dest);
        VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

        if((VIR_GetTypeFlag(baseTy0) & VIR_TYFLAG_ISINTEGER) &&
            (VIR_GetTypeFlag(baseTy1) & VIR_TYFLAG_ISINTEGER))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, Inst->dest);
    VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

    if((VIR_GetTypeFlag(baseTy0) & VIR_TYFLAG_ISINTEGER) &&
        (VIR_GetTypeFlag(baseTy1) & VIR_TYFLAG_ISFLOAT))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isI2F(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, Inst->dest);
    VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

    if((VIR_GetTypeFlag(baseTy0) & VIR_TYFLAG_ISFLOAT) &&
        (VIR_GetTypeFlag(baseTy1) & VIR_TYFLAG_ISINTEGER))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Sat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(Inst != gcvNULL &&
        Inst->dest != gcvNULL);

    if (VIR_Operand_GetModifier(Inst->dest) != VIR_MOD_NONE &&
        _isF2I(Context, Inst))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Sat_Rtp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(VIR_Operand_GetRoundMode(Inst->dest) == VIR_ROUND_RTP &&
        VIR_Operand_GetModifier(Inst->dest) != VIR_MOD_NONE &&
        _isF2I(Context, Inst))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Sat_Rtn(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(VIR_Operand_GetModifier(Inst->dest) != VIR_MOD_NONE &&
        VIR_Operand_GetRoundMode(Inst->dest) == VIR_ROUND_RTN &&
        _isF2I(Context, Inst))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Rtp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(VIR_Operand_GetRoundMode(Inst->dest) == VIR_ROUND_RTP &&
        _isF2I(Context, Inst))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF2I_Rtn(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(VIR_Operand_GetRoundMode(Inst->dest) == VIR_ROUND_RTN &&
        _isF2I(Context, Inst))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isI2I_Sat_s2us(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT bits, bits0;
    VIR_PrimitiveTypeId format, format0;

    if (!VIR_Operand_GetModifier(Inst->dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        bits = 8;
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        bits = 16;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case VIR_TYPE_INT8:
        bits0 = 8;
        break;

    case VIR_TYPE_INT16:
        bits0 = 16;
        break;

    case VIR_TYPE_INT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 > bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_isI2I_Sat_u2us(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT bits, bits0;
    VIR_PrimitiveTypeId format, format0;

    if (!VIR_Operand_GetModifier(Inst->dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        bits = 8;
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        bits = 16;
        break;

    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
        bits = 32;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case VIR_TYPE_UINT8:
        bits0 = 8;
        break;

    case VIR_TYPE_UINT16:
        bits0 = 16;
        break;

    case VIR_TYPE_UINT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 >= bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}


static gctBOOL
_isI2I_Sat_s2u(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT bits, bits0;
    VIR_PrimitiveTypeId format, format0;

    if (!VIR_Operand_GetModifier(Inst->dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (format0 == format)
    {
        return gcvFALSE;
    }

    switch (format)
    {
    case VIR_TYPE_UINT8:
        bits = 8;
        break;

    case VIR_TYPE_UINT16:
        bits = 16;
        break;

    case VIR_TYPE_UINT32:
        bits = 32;
        break;

    default:
        return gcvFALSE;
    }

    switch (format0)
    {
    case VIR_TYPE_INT8:
        bits0 = 8;
        break;

    case VIR_TYPE_INT16:
        bits0 = 16;
        break;

    case VIR_TYPE_INT32:
        bits0 = 32;
        break;

    default:
        return gcvFALSE;
    }

    if (bits0 <= bits)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}


static gctBOOL
_isCL_X_Signed_8_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternLowerContext *)Context)->isCL_X &&
        !((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        switch (VIR_GetTypeComponentType(baseType))
        {
        case VIR_TYPE_INT8:
        case VIR_TYPE_INT16:
            return gcvTRUE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}


static gctBOOL
_isCL_X_Unsigned_8_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternLowerContext *)Context)->isCL_X &&
        !((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        switch (VIR_GetTypeComponentType(baseType))
        {
        case VIR_TYPE_UINT8:
        case VIR_TYPE_UINT16:
            return gcvTRUE;

        default:
            return gcvFALSE;
        }
    }
    else
    {
        return gcvFALSE;
    }
}


static gctBOOL
_is_dest_16bit_src_int8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_CONV);

    if (!((VIR_PatternLowerContext *)Context)->isCL_X ||
        ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT16 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT16)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

            if (VIR_GetTypeComponentType(srcType) == VIR_TYPE_INT8)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL
_is_dest_32bit_src_int8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_CONV);
    if (!((VIR_PatternLowerContext *)Context)->isCL_X ||
        ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT32 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT32)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

            if (VIR_GetTypeComponentType(srcType) == VIR_TYPE_INT8)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL
_is_dest_32bit_src_int16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (!((VIR_PatternLowerContext *)Context)->isCL_X ||
        ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT32 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT32)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, Inst->src[0]);

            if (VIR_GetTypeComponentType(srcType) == VIR_TYPE_INT16)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}

static gctBOOL
_is_dest_8bit(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (!((VIR_PatternLowerContext *)Context)->isCL_X ||
        ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT8 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT8)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}


static gctBOOL
_is_dest_16bit(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (!((VIR_PatternLowerContext *)Context)->isCL_X ||
        ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, Inst->dest);

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT16 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT16)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}


static gctBOOL
_isF16_2_F32_hasCMP_NotFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId format, format0;

    if (VIR_Lower_enableFullNewLinker(Context, Inst))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (!((VIR_PatternLowerContext *)Context)->hasCL)
    {
        return gcvFALSE;
    }

    if (format == VIR_TYPE_FLOAT32 &&
        format0 == VIR_TYPE_FLOAT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF16_2_F32_hasCMP_FullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId format, format0;

    if (!VIR_Lower_enableFullNewLinker(Context, Inst))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (!((VIR_PatternLowerContext *)Context)->hasCL)
    {
        return gcvFALSE;
    }

    if (format == VIR_TYPE_FLOAT32 &&
        format0 == VIR_TYPE_FLOAT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_SetValueType0(
    IN VIR_PatternContext  *Context,
    IN VIR_Operand         *Opnd,
    IN VIR_PrimitiveTypeId  Format
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(Format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));

    return gcvTRUE;
}

static gctBOOL
_setOperandUINT(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Opnd, VIR_TYPE_UINT32);
    return gcvTRUE;
}

static gctBOOL
float32_sign(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x80000000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    /*
    _SetValueType0(0x5, States);
    */
    return gcvTRUE;
}

static gctBOOL
float32_exp_isnan(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x7F800000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
/*
    _SetValueType0(0x5, States);
*/
    return gcvTRUE;
}

static gctBOOL
float32_exp_bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x10u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    /*
    _SetValueType0(0x5, States);
    */
    return gcvTRUE;
}

static gctBOOL
float32_exp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x7F800000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    /*
    _SetValueType0(0x5, States);
    */
    return gcvTRUE;
}

static gctBOOL
float32_man_bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0xDu;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    /*
    _SetValueType0(0x5, States);
    */
    return gcvTRUE;
}

static gctBOOL
float32_man(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x7FFFFFu;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    /*
    _SetValueType0(0x5, States);
    */
    return gcvTRUE;
}

static gctBOOL
value_types_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT16);
    /*
    _SetValueType0(0x6, States);
    */
    return gcvTRUE;
}


static gctBOOL
_isF32_2_F16_hasCMP_NotFullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId format, format0;

    if (VIR_Lower_enableFullNewLinker(Context, Inst))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (!((VIR_PatternLowerContext *)Context)->hasCL)
    {
        return gcvFALSE;
    }

    if (format == VIR_TYPE_FLOAT16 &&
        format0 == VIR_TYPE_FLOAT32)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isF32_2_F16_hasCMP_FullNewLinker(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId format, format0;
    if (!VIR_Lower_enableFullNewLinker(Context, Inst))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (!((VIR_PatternLowerContext *)Context)->hasCL)
    {
        return gcvFALSE;
    }

    if (format == VIR_TYPE_FLOAT16 &&
        format0 == VIR_TYPE_FLOAT32)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
value_type0_32bit(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));

    if (((VIR_PatternLowerContext *)Context)->isCL_X &&
        !((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        /* Convert it to 32-bit. */
        switch (format)
        {
        case VIR_TYPE_INT8:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT32:
            format = VIR_TYPE_INT32;
            break;

        case VIR_TYPE_UINT8:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT32:
            format = VIR_TYPE_UINT32;
            break;

        default:
            return gcvFALSE;
        }
    }

    VIR_Operand_SetType(Inst->dest, VIR_TypeId_ComposeNonOpaqueType(format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Inst->dest)), 1));

    return gcvTRUE;
}
static gctBOOL
value_type0_32bit_reset_sat_rounding(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if (!value_type0_32bit(Context, Inst, Opnd))
    {
        return gcvFALSE;
    }

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);
    return gcvTRUE;
}

static gctBOOL
_value_type0_32bit_from_src0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));

    if (((VIR_PatternLowerContext *)Context)->isCL_X &&
        !((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11)
    {
        /* Convert it to 32-bit. */
        switch (format)
        {
        case VIR_TYPE_INT8:
        case VIR_TYPE_INT16:
        case VIR_TYPE_INT32:
            format = VIR_TYPE_INT32;
            break;

        case VIR_TYPE_UINT8:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_UINT32:
            format = VIR_TYPE_UINT32;
            break;

        default:
            return gcvFALSE;
        }
    }

    VIR_Operand_SetType(Inst->src[0], VIR_TypeId_ComposeNonOpaqueType(format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Inst->src[0])), 1));

    return gcvTRUE;
}
static gctBOOL
revise_dest_type_by_operand_type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    VIR_Operand_SetType(VIR_Inst_GetDest(Inst), VIR_Operand_GetType(Opnd));

    return gcvTRUE;
}

static gctBOOL
set_opnd_type_prevInst_src0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction* prevInst;

    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    prevInst= VIR_Inst_GetPrev(Inst);
    if(prevInst)
    {
        VIR_Operand* prevSrc0 = VIR_Inst_GetSource(prevInst, 0);
        VIR_TypeId prevSrc0TypeID = VIR_Operand_GetType(prevSrc0);

        VIR_Operand_SetType(Opnd, prevSrc0TypeID);
    }
    return gcvTRUE;
}

static gctBOOL
max_type0_const(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    VIR_ScalarConstVal value;

    switch (format)
    {
    case VIR_TYPE_UINT8:
        value.uValue = 255;
        break;

    case VIR_TYPE_INT8:
        value.uValue = 127;
        break;

    case VIR_TYPE_UINT16:
        value.uValue = 65535;
        break;

    case VIR_TYPE_INT16:
        value.uValue = 32767;
        break;

    case VIR_TYPE_UINT32:
        value.uValue = 0xFFFFFFFF;
        break;

    case VIR_TYPE_INT32:
        value.uValue = 2147483647;
        break;

    default:
        return gcvFALSE;
    }

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));
    if (format == VIR_TYPE_FLOAT32)
    {
        value.fValue = (gctFLOAT) value.uValue;
    }
    else
    {
        _value_type0_32bit_from_src0(Context, Inst, Opnd);
    }

    VIR_Operand_SetImmediate(Inst->src[1],
        format == VIR_TYPE_FLOAT32 ? VIR_TYPE_FLOAT32 : VIR_TYPE_UINT32,
        value);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);

    return gcvTRUE;
}

static gctBOOL
min_type0_const(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));
    VIR_ScalarConstVal value;

    switch (format)
    {
    case VIR_TYPE_UINT8:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_UINT32:
        value.iValue = 0;
        break;

    case VIR_TYPE_INT8:
        value.iValue = -128;
        break;

    case VIR_TYPE_INT16:
        value.iValue = -32768;
        break;

    case VIR_TYPE_INT32:
        value.iValue = (-2147483647 - 1);
        break;

    default:
        return gcvFALSE;
    }

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->src[0]));
    if (format == VIR_TYPE_FLOAT32)
    {
        value.fValue = (gctFLOAT) value.iValue;
    }
    else
    {
        _value_type0_32bit_from_src0(Context, Inst, Opnd);
    }

    VIR_Operand_SetImmediate(Inst->src[1],
        format == VIR_TYPE_FLOAT32 ? VIR_TYPE_FLOAT32 : VIR_TYPE_INT32,
        value);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);
    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_24_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));

    VIR_ScalarConstVal imm0;

    if (format == VIR_TYPE_INT8)
    {
        imm0.uValue = 24;
    }
    else
    {
        imm0.uValue = 16;
    }

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);


    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
uint_value_type0_const_FF_FFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId format =
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Inst->dest));

    VIR_ScalarConstVal imm0;

    if (format == VIR_TYPE_INT8)
    {
        imm0.uValue = 0x000000FF;
    }
    else
    {
        imm0.uValue = 0x0000FFFF;
    }

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    return gcvTRUE;
}

static gctBOOL
value_type0_const_FF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x000000FF;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    /*
    value_type0(Tree, CodeGen, Instruction, States);
    */
    return gcvTRUE;
}

static gctBOOL
value_type0_const_FFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x0000FFFF;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

/*
    value_type0(Tree, CodeGen, Instruction, States);
*/
    return gcvTRUE;
}

static gctBOOL
short_value_type0_const_8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 8;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_INT16);
    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 16;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_24(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 24;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
float16_sign(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x8000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT16);
    return gcvTRUE;
}

static gctBOOL
float16_exp_bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x10u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
float16_exp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x7C00u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT16);
    return gcvTRUE;
}

static gctBOOL
float16_man_bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0xDu;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    return gcvTRUE;
}

static gctBOOL
float16_man(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x3FFu;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT16);
    return gcvTRUE;
}

static gctBOOL
float16_exp_iszero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_ScalarConstVal imm1;

    imm0.uValue = 0;
    imm1.uValue = 0x38000000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_UINT32,
        imm1);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
float16_exp_isnan(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x0F800000u;

    VIR_Operand_SetImmediate(Inst->src[1],
        VIR_TYPE_UINT32,
        imm0);

    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    return gcvTRUE;
}

static gctBOOL
float16_exp_isaddnanNZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x7000000u;

    VIR_Operand_SetImmediate(Inst->src[2],
        VIR_TYPE_UINT32,
        imm0);
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
value_types_32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _SetValueType0(Context, Inst->dest, VIR_TYPE_UINT32);
    return gcvTRUE;
}

static gctBOOL
jmp_2_succ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN gctUINT            seq
    )
{
    VIR_Instruction* succ;
    VIR_Operand* inst_dest, *succ_dest;
    VIR_Label* inst_label, *succ_label;
    gctUINT i;
    gcmASSERT(Inst->_opcode == VIR_OP_JMP
        || Inst->_opcode == VIR_OP_JMPC
        || Inst->_opcode == VIR_OP_JMP_ANY);

    succ = Inst;
    for(i = 0; i < seq; ++i)
    {
        succ = VIR_Inst_GetNext(succ);
        if(succ == gcvNULL)
        {
            return gcvFALSE;
        }
    }

    succ_dest = VIR_Inst_GetDest(succ);
    if(!succ_dest || (VIR_Operand_GetOpKind(succ_dest) != VIR_OPND_LABEL))
    {
        return gcvFALSE;
    }
    succ_label = VIR_Operand_GetLabel(succ_dest);
    inst_dest = VIR_Inst_GetDest(Inst);
    gcmASSERT((VIR_Operand_GetOpKind(inst_dest) == VIR_OPND_LABEL));
    inst_label = VIR_Operand_GetLabel(inst_dest);
    return inst_label == succ_label;
}
static gctBOOL
all_source_single_value(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;
    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        VIR_TypeId src_typeid = VIR_Operand_GetType(src);
        if(VIR_GetTypeTypeKind(src_typeid) != VIR_TY_SCALAR)
        {
            if(VIR_GetTypeTypeKind(src_typeid) != VIR_TY_VECTOR || VIR_Swizzle_Channel_Count(VIR_Operand_GetSwizzle(src)) > 1)
            {
                return gcvFALSE;
            }
        }
    }

    return gcvTRUE;
}

static gctBOOL
all_source_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;
    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        VIR_TypeId src_typeid = VIR_Operand_GetType(src);
        if(!(VIR_GetTypeFlag(src_typeid) & VIR_TYFLAG_ISFLOAT))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gctBOOL
supportCMP(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasHalti0;
}

static gctBOOL
supportCONV(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasHalti4;
}

static gctBOOL
jmp_2_succ3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return jmp_2_succ(Context, Inst, 3);
}

static gctBOOL
supportCMP_single_value_jmp_2_succ2_resCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);
    return supportCMP(Context, Inst) &&
           all_source_single_value(Context, Inst) &&
           jmp_2_succ(Context, Inst, 2) &&
           VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(Inst));
}

static gctBOOL
jmp_2_succ2_resCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId ty0, ty1;
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);

    ty0 = VIR_Operand_GetType(Inst->src[0]);
    ty1 = VIR_Operand_GetType(Inst->src[1]);

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);
    /* Texkill only supports float. */
    return jmp_2_succ(Context, Inst, 2) &&
           VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(Inst)) &&
           (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
           (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT);
}

static gctBOOL
supportCMP_single_value_jmp_2_succ3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);
    return supportCMP(Context, Inst) &&
           all_source_single_value(Context, Inst) &&
           jmp_2_succ(Context, Inst, 3);
}

static gctBOOL
dest_type_less_than_prev_jmp_src0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Instruction* prevInst = VIR_Inst_GetPrev(Inst);

    if (prevInst)
    {
        VIR_Operand* dest = VIR_Inst_GetDest(Inst);
        VIR_Operand* prevSrc0 = VIR_Inst_GetSource(prevInst, 0);
        VIR_TypeId destTypeID = VIR_GetTypeComponentType(VIR_Operand_GetType(dest));
        VIR_TypeId prevSrc0TypeID = VIR_GetTypeComponentType(VIR_Operand_GetType(prevSrc0));

        gcmASSERT(prevInst->_opcode == VIR_OP_JMPC);

        if (VIR_GetTypeSize(destTypeID) <= VIR_GetTypeSize(prevSrc0TypeID) &&
            VIR_Operand_GetPrecision(dest) <= VIR_Operand_GetPrecision(prevSrc0))
        {
            return gcvTRUE;
        }
        return gcvFALSE;
    }
    return gcvFALSE;
}

static gctBOOL
canBeMergedToSelect(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN gctUINT            SrcIndex
    )
{
    VIR_Instruction* prevInst = VIR_Inst_GetPrev(Inst);
    if(prevInst)
    {
        VIR_Operand* dest = VIR_Inst_GetDest(Inst);
        VIR_Operand* prevSrc0 = VIR_Inst_GetSource(prevInst, SrcIndex);
        VIR_TypeId destTypeID = VIR_Operand_GetType(dest);
        VIR_TypeId prevSrc0TypeID = VIR_Operand_GetType(prevSrc0);

        gcmASSERT(VIR_TypeId_isPrimitive(destTypeID) && VIR_TypeId_isPrimitive(prevSrc0TypeID));

        if((VIR_GetTypeFlag(destTypeID) & VIR_TYFLAG_ISFLOAT) &&
           (VIR_GetTypeFlag(prevSrc0TypeID) & VIR_TYFLAG_ISFLOAT))
        {
            return gcvTRUE;
        }
        if(VIR_GetTypeFlag(prevSrc0TypeID) & VIR_TYFLAG_ISINTEGER)
        {
            return gcvTRUE;
        }

        return gcvFALSE;
    }
    return gcvFALSE;
}

static gctBOOL
_dstSrcSamePrecsion(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_OperandInfo     opndInfo;
    VIR_Operand_GetOperandInfo(Inst, VIR_Inst_GetSource(Inst, 0), &opndInfo);

    if (Context->shader->shaderKind != VIR_SHADER_FRAGMENT ||
        (VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst)) == VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0)) &&
         !opndInfo.isUniform))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
canBeMergedToSelect0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return canBeMergedToSelect(Context, Inst, 0);
}

static gctBOOL
canBeMergedToSelect1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return canBeMergedToSelect(Context, Inst, 1);
}

static gctBOOL
label_only_one_jmp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand* inst_dest;
    VIR_Label* label;

    gcmASSERT(Inst->_opcode == VIR_OP_LABEL);

    inst_dest = VIR_Inst_GetDest(Inst);
    label = VIR_Operand_GetLabel(inst_dest);
    return VIR_Link_Count(label->referenced) == 1;
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
label_set_jmp_neg3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return label_set_jmp_n(Context, Inst, Opnd, -3);
}

static gctBOOL
label_set_jmp_neg10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return label_set_jmp_n(Context, Inst, Opnd, -10);
}

static gctBOOL
no_source(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;

    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        if(VIR_Operand_GetOpKind(src) != VIR_OPND_UNDEF)
        {
            return gcvFALSE;
        }
    }
    return gcvTRUE;
}

static gctBOOL
reverseCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
    VIR_Inst_SetConditionOp(Inst, VIR_ConditionOp_Reverse(condOp));
    return gcvTRUE;
}

static gctBOOL
setCondOpCompareWithZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
    VIR_Inst_SetConditionOp(Inst, VIR_ConditionOp_SetCompareWithZero(condOp));
    return gcvTRUE;
}

static gctBOOL
setSwitchedCondOpCompareWithZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ConditionOp condOp = VIR_Inst_GetConditionOp(Inst);
    VIR_Inst_SetConditionOp(Inst, VIR_ConditionOp_SetCompareWithZero(VIR_ConditionOp_SwitchLeftRight(condOp)));
    return gcvTRUE;
}

static gctBOOL
_SetSwizzleByType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swiz       = VIR_Operand_GetSwizzle(Opnd);
    VIR_Swizzle  nswiz      = VIR_SWIZZLE_XXXX;
    gctUINT      components = VIR_GetTypeComponents(VIR_Operand_GetType(Opnd));

    if(components == 3)
    {
        nswiz = VIR_Swizzle_GetChannel(swiz, 2);
    }
    else if(components == 4)
    {
        nswiz = VIR_Swizzle_GetChannel(swiz, 3);
    }
    else
    {
        gcmASSERT(0);
    }

    nswiz |= nswiz << 2;
    nswiz |= nswiz << 4;

    VIR_Operand_SetSwizzle(Opnd, nswiz);

    return gcvTRUE;
}

static gctBOOL
_Set_SwizzleXYEx(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle  swizzle = VIR_Operand_GetSwizzle(Opnd);
    VIR_Swizzle  xSwizzle, ySwizzle, newSwizzle;

    xSwizzle = swizzle & 0x3;
    ySwizzle = (swizzle >> 2) & 0x3;

    newSwizzle = ((xSwizzle << 0) |
        (ySwizzle << 2) |
        (ySwizzle << 4) |
        (ySwizzle << 6));

    VIR_Operand_SetSwizzle(Opnd, newSwizzle);

    return gcvTRUE;
}


static gctBOOL
_setBooleanType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_BOOLEAN,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));

    return gcvTRUE;
}

static gctBOOL
_setIntegerType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));

    return gcvTRUE;
}

static gctBOOL
_setFloatType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));

    return gcvTRUE;
}

static gctBOOL
_setGradType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT compCount;
    VIR_Enable enable;


    compCount = VIR_Enable_Channel_Count(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Inst->src[0])));

    switch (compCount)
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
        enable = VIR_ENABLE_XYZW;
        gcmASSERT(gcvFALSE);
        break;
    }

    VIR_Inst_SetEnable(Inst, enable);
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, compCount, 1));

    return gcvTRUE;
}

static VIR_TexModifier_Flag
_getTexModifierKind(
    IN VIR_Operand *Opnd
    )
{
    if (Opnd == gcvNULL || VIR_Operand_GetOpKind(Opnd) != VIR_OPND_TEXLDPARM)
    {
        return VIR_TMFLAG_NONE;
    }

    return (VIR_TexModifier_Flag)VIR_Operand_GetTexModifierFlag(Opnd);
}

static gctBOOL
_isBiasTexModifierAndCubeArrayShadow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId samplerType = Inst->src[0]->_opndType;

    if (samplerType == VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW && _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_BIAS)
    {
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

static gctBOOL
_isBiasTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_BIAS;
}

static gctBOOL
_isLodTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_LOD;
}

static gctBOOL
_isGradTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_GRAD;
}


static gctBOOL
_isGatherTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_GATHER;
}


static gctBOOL
_isFetchMsTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _getTexModifierKind(Inst->src[2]) == VIR_TMFLAG_FETCHMS;
}

static gctBOOL
_samePrecisionAsSource0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand* src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand_SetPrecision(Opnd, VIR_Operand_GetPrecision(src0));
    return gcvTRUE;
}

/*
        DIV 1, 2, 3
            idiv 1, 2, 3, 0, 0
    { 1, gcSL_DIV, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x44, 1, 2, 3, 0, 0, value_type0 },
*/

static VIR_PatternMatchInst _divPatInst0[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsIntOpcode, _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

    /*
        DIV 1, 2, 3
            rcp TEMP1, 0, 0, 3
            mul 1, 2, TEMP1, 0
    */
static VIR_PatternMatchInst _divPatInst1[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_enableFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst1[] = {
    { VIR_OP_RCP, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -1, 0 }, { 0 } },
};

static VIR_Pattern _divPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 0) },
    /* now gcSL CG cannot handle expanded PRE_DIV, enable this if use new VIR CG */
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_div, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sinPatInst0[] = {
    { VIR_OP_SIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV, _requireSIN_COS_TAN_Precision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sinRepInst0[] = {
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, 2, 0, 0 }, { 0, _set_ABS, _115_pi } },
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { _set_HighPrecision, 0, rcppi } },
    { VIR_OP_MAD, 0, 0, { -1, 2, 0,-1 }, { 0, 0, rcppi_low } },
    { VIR_OP_MUL, 0, 0, { -2, 2, 0, 0 }, { _set_RTZ_HighPrecision, 0, rcp2pi } },
    { VIR_OP_SIGN, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -2, -2,-3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -2, 0,-1 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg3 } },
    { VIR_OP_SINPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};

/*
        SIN 1, 2
            mul   1, 2, rcppi, 0
            sinpi 1, 1
    */
static VIR_PatternMatchInst _sinPatInst1[] = {
    { VIR_OP_SIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sinRepInst1[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi } },
    { VIR_OP_SINPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};
    /*
        SIN 1, 2
            mul   1, 2, rcppi2, 0
            sinpi 1, 1
    */
static VIR_PatternMatchInst _sinPatInst2[] = {
    { VIR_OP_SIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sinRepInst2[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi2_1 } },
    { VIR_OP_SINPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};

    /*
    { 1, gcSL_SIN, 1, 2 },
        { -9, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -8, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -7, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -6, 0x03, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -5, 0x02, 1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, factor9_1_factor7_2 },
        { -4, 0x02, 1, gcSL_CG_TEMP2, 1, gcSL_CG_CONSTANT, 0, factor5_2 },
        { -3, 0x02, 1, gcSL_CG_TEMP2, 1, -gcSL_CG_CONSTANT, 0, factor3_2 },
        { -2, 0x02, 1, gcSL_CG_TEMP2, 1, gcSL_CG_CONSTANT, 0, one_2 },
        { -1, 0x03, 1, 1, gcSL_CG_TEMP1, 0, 0 },

        SIN 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi
            mul TEMP2, TEMP1, TEMP1
            mad 1, TEMP2, factor9, -factor7
            mad 1, TEMP2, 1, factor5
            mad 1, TEMP2, 1, -factor3
            mad 1, TEMP2, 1, VIR_Lower_SetOne
            mul 1, 1, TEMP1
    */
static VIR_PatternMatchInst _sinPatInst3[] = {
    { VIR_OP_SIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sinRepInst3[] = {
    { VIR_OP_MAD, 0, 0, { -1, 2, 0, 0 }, { rcppi2_1_dot5_2 } },
    { VIR_OP_FRAC, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -1, 0, 0 }, { pi2_1_pi_2 } },
    { VIR_OP_MUL, 0, 0, { -2, -1, -1, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -3, -2, 0, 0 }, { factor9_1_factor7_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -2, -3, 0 }, { factor5_2 } },
    { VIR_OP_MAD, 0, 0, { -5, -2, -4, 0 }, { factor3_2 } },
    { VIR_OP_MAD, 0, 0, { -6, -2, -5, 0 }, { sin_one_2 } },
    { VIR_OP_MUL, 0, 0, {  1, -6, -1, 0 }, { 0 } },
};

static VIR_Pattern _sinPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sin, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sin, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sin, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sin, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _cosPatInst0[] = {
    { VIR_OP_COS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV, _requireSIN_COS_TAN_Precision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _cosRepInst0[] = {
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, 2, 0, 0 }, { 0, _set_ABS, _115_pi } },
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { _set_HighPrecision, 0, rcppi } },
    { VIR_OP_MAD, 0, 0, { -1, 2, 0,-1 }, { 0, 0, rcppi_low } },
    { VIR_OP_MUL, 0, 0, { -2, 2, 0, 0 }, { _set_RTZ_HighPrecision, 0, rcp2pi } },
    { VIR_OP_SIGN, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -2, -2,-3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -2, 0,-1 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg3 } },
    { VIR_OP_COSPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};

    /*
        COS 1, 2
            mul   1, 2, rcppi, 0
            cospi 1, 1
    */
static VIR_PatternMatchInst _cosPatInst1[] = {
    { VIR_OP_COS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cosRepInst1[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi } },
    { VIR_OP_COSPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};
    /*
        COS 1, 2
            mul   1, 2, rcppi2, 0
            cospi 1, 1
    */
static VIR_PatternMatchInst _cosPatInst2[] = {
    { VIR_OP_COS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cosRepInst2[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi2_1 } },
    { VIR_OP_COSPI, -1, 0, {  1, -1, 0, 0 }, { 0 } },
};

    /*
        COS 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi
            mul TEMP1, TEMP1, TEMP1
            mad 1, TEMP1, factor8, -factor6
            mad 1, TEMP1, 1, factor4
            mad 1, TEMP1, 1, -factor2
            mad 1, TEMP1, 1, VIR_Lower_SetOne
    { 1, gcSL_COS, 1, 2 },
        { -8, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -7, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -6, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -5, 0x03, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -4, 0x02, 1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, factor8_1_factor6_2 },
        { -3, 0x02, 1, gcSL_CG_TEMP1, 1, gcSL_CG_CONSTANT, 0, factor4_2 },
        { -2, 0x02, 1, gcSL_CG_TEMP1, 1, -gcSL_CG_CONSTANT, 0, factor2_2 },
        { -1, 0x02, 1, gcSL_CG_TEMP1, 1, gcSL_CG_CONSTANT, 0, one_2 },

    */
static VIR_PatternMatchInst _cosPatInst3[] = {
    { VIR_OP_COS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cosRepInst3[] = {
    { VIR_OP_MAD, 0, 0, { -1, 2, 0, 0 }, { rcppi2_1_dot5_2 } },
    { VIR_OP_FRAC, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -1, 0, 0 }, { pi2_1_pi_2 } },
    { VIR_OP_MUL, 0, 0, { -1, -1, -1, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -2, -1, 0, 0 }, { factor8_1_factor6_2 } },
    { VIR_OP_MAD, 0, 0, { -3, -1, -2, 0 }, { factor4_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -1, -3, 0 }, { factor2_2 } },
    { VIR_OP_MAD, 0, 0, {  1, -1, -4, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
};

static VIR_Pattern _cosPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cos, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cos, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cos, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cos, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _tanPatInst0[] = {
    { VIR_OP_TAN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV, _requireSIN_COS_TAN_Precision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _tanRepInst0[] = {
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, 2, 0, 0 }, { 0, _set_ABS, _115_pi } },
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { _set_HighPrecision, 0, rcppi } },
    { VIR_OP_MAD, 0, 0, { -1, 2, 0,-1 }, { 0, 0, rcppi_low } },
    { VIR_OP_MUL, 0, 0, { -2, 2, 0, 0 }, { _set_RTZ_HighPrecision, 0, rcp2pi } },
    { VIR_OP_SIGN, 0, 0, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -2, -2,-3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -2, 0,-1 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { label_set_jmp_neg3 } },
    { VIR_OP_SINPI, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_COSPI, 0, 0, { -3, -1, 0, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, {  1, -2, -3, 0 }, { 0 } },
};

    /*
        TAN 1, 2
            mul   temp1, 2, rcppi, 0
            sinpi temp2, 0, 0, temp1
            cospi temp3, 0, 0, temp1
            div   1, temp2, temp3 // tan(x) = sin(x)/cos(x)
    { 1, gcSL_TAN, 1, 2, 0, 0, 0, _hasNEW_SIN_COS_LOG_DIV },
        { -4, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi },
        { -3, 0x22, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },
        { -2, 0x23, gcSL_CG_TEMP3, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },
        { -1, 0x64, 1, 0, gcSL_CG_TEMP2, gcSL_CG_TEMP3, 0, set_new_sin_cos_log_div },
    */
static VIR_PatternMatchInst _tanPatInst1[] = {
    { VIR_OP_TAN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _tanRepInst1[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi } },
    { VIR_OP_SINPI, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_COSPI, 0, 0, { -3, -1, 0, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, {  1, -2, -3, 0 }, { 0 } },
};
    /*
        TAN 1, 2
            mul temp1, 2, rcppi2, 0
            cos temp2, 0, 0, temp1
            sin temp3, 0, 0, temp1
            rcp 1, temp2
            mul 1, 1, temp3 // tan(x) = sin(x)/cos(x)
    { 1, gcSL_TAN, 1, 2, 0, 0, 0, _hasSQRT_TRIG },
        { -5, 0x03, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, rcppi2_1 },
        { -4, 0x23, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP1},
        { -3, 0x22, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1},
        { -2, 0x0C, gcSL_CG_TEMP2, 0, 0, gcSL_CG_TEMP2, 0, },
        { -1, 0x03, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 0, 0, },
    */
static VIR_PatternMatchInst _tanPatInst2[] = {
    { VIR_OP_TAN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _tanRepInst2[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { _samePrecisionAsSource0, 0, rcppi2_1 } },
    { VIR_OP_COSPI, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_SINPI, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_RCP, 0, 0, { -2, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};
    /*
        TAN 1, 2
            mad TEMP1, 2, rcppi2, dot5
            frc TEMP1, TEMP1
            mad TEMP1, TEMP1, pi2, -pi

            tan(x) = x + 1/3 x^3 + 2/15 x^5 + 17/315 x^7 + 62/2835 x^9
                   = x (1 + x^2 (1/3 + x^2 (2/15 + x^2 (17/315 + 62/2835 x^2 ) ) ) )

            mul TEMP2, TEMP1, TEMP1
            mad 1, TEMP2, tan9, tan7
            mad 1, TEMP2, 1, tan5
            mad 1, TEMP2, 1, tan3
            mad 1, TEMP2, 1, VIR_Lower_SetOne
            mul 1, TEMP1, 1

    { 1, gcSL_TAN, 1, 2 },
        { -9, 0x02, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, rcppi2_1_dot5_2 },
        { -8, 0x13, gcSL_CG_TEMP1, 0, 0, gcSL_CG_TEMP1, 0 },
        { -7, 0x02, gcSL_CG_TEMP1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, pi2_1_pi_2 },
        { -6, 0x03, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, 0 },
        { -5, 0x02, 1, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, tan9_1_tan7_2 },
        { -4, 0x02, 1, gcSL_CG_TEMP2, 1, gcSL_CG_CONSTANT, 0, tan5_2 },
        { -3, 0x02, 1, gcSL_CG_TEMP2, 1, gcSL_CG_CONSTANT, 0, tan3_2 },
        { -2, 0x02, 1, gcSL_CG_TEMP2, 1, gcSL_CG_CONSTANT, 0, one_2 },
        { -1, 0x03, 1, gcSL_CG_TEMP1, 1, 0, 0, },
    */

static VIR_PatternMatchInst _tanPatInst3[] = {
    { VIR_OP_SINPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _tanRepInst3[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi2_1 } },
    { VIR_OP_FRAC, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -2, -1, -1, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -3, -2, 0, 0 }, { tan9_1_tan7_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -2, -3, 0 }, { tan5_2 } },
    { VIR_OP_MAD, 0, 0, { -5, -2, -4, 0 }, { tan3_2 } },
    { VIR_OP_MAD, 0, 0, { -6, -2, -5, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_MUL, 0, 0, {  1, -1, -6, 0 }, { 0 } },
};

static VIR_Pattern _tanPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_tan, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_tan, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_tan, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_tan, 3) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        TANPI 1, 2
            sin temp1, 0, 0, 2
            cos temp2, 0, 0, 2
            div 1, temp1, temp2 // tan(x) = sin(x)/cos(x)
    */
static VIR_PatternMatchInst _tanpiPatInst0[] = {
    { VIR_OP_TANPI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _tanpiRepInst0[] = {
    { VIR_OP_SINPI, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_COSPI, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};

static VIR_Pattern _tanpiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_tanpi, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        FWIDTH 1, 2
            dsx TEMP1, 2
            dsy TEMP2, 2
            abs TEMP3, TEMP1
            abs TEMP4, TEMP2
            add 1, TEMP3, TEMP4, 0
    */
static VIR_PatternMatchInst _fwidthPatInst0[] = {
    { VIR_OP_FWIDTH, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _fwidthRepInst0[] = {
    { VIR_OP_DSX, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_DSY, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -3, -1, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -4, -2, 0, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, {  1, -3, -4, 0 }, { 0 } },
};

static VIR_Pattern _fwidthPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_fwidth, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        ASIN 1, 2
            ASIN(x) = x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9
                    = x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2))))

            mul TEMP1, 2, 2
            mad 1, TEMP1, asin9, asin7
            mad 1, TEMP1, 1, asin5
            mad 1, TEMP1, 1, asin3
            mad 1, TEMP1, 1, VIR_Lower_SetOne
            mul 1, 2, 1
    */
static VIR_PatternMatchInst _asinPatInst0[] = {
    { VIR_OP_ASIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _asinRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 2, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -2, -1, 0, 0 }, { asin9_1_asin7_2 } },
    { VIR_OP_MAD, 0, 0, { -3, -1, -2, 0 }, { asin5_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -1, -3, 0 }, { asin3_2 } },
    { VIR_OP_MAD, 0, 0, { -5, -1, -4, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -5, 0 }, { 0 } },
};

static VIR_Pattern _asinPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_asin, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        ACOS 1, 2
            ACOS(x) = 1/2 pi - (x + 1/6 x^3 + 3/40 x^5 + 5/112 x^7 + 35/1152 x^9)
                    = 1/2 pi - (x (1 + x^2 (1/6 + x^2 (3/40 + x^2 (5/112 + 35/1152 x^2)))))

            mul TEMP1, 2, 2
            mad 1, TEMP1, asin9, asin7
            mad 1, TEMP1, 1, asin5
            mad 1, TEMP1, 1, asin3
            mad 1, TEMP1, 1, VIR_Lower_SetOne
            neg 1, 1
            mad 1, 2, 1, half_pi
    */
static VIR_PatternMatchInst _acosPatInst0[] = {
    { VIR_OP_ACOS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _acosRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 2, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -2, -1, 0, 0 }, { asin9_1_asin7_2 } },
    { VIR_OP_MAD, 0, 0, { -3, -1, -2, 0 }, { asin5_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -1, -3, 0 }, { asin3_2 } },
    { VIR_OP_MAD, 0, 0, { -5, -1, -4, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_NEG, 0, 0, { -6, -5, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, {  1, 2, -6, 0 }, { 0, 0, 0, half_pi } },
};

static VIR_Pattern _acosPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_acos, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        ATAN 1, 2

            if (|x| > 1) flag = 1; x = 1 / x; else flag = 0;

                set.gt TEMP1, |x|, 1
                rcp TEMP2, x
                select.nz TEMP2, TEMP1, TEMP2, x

            atan(x) = x - 1/3 x^3 + 1/5 x^5 - 1/7 x^7 + 1/9 x^9
                    = x (1 + x^2 (-1/3 + x^2 (1/5 + x^2 (-1/7 + 1/9 x^2 ) ) ) )

                mul TEMP3, TEMP2, TEMP2
                mad 1, TEMP3, atan9, -atan7
                mad 1, TEMP3, 1, atan5
                mad 1, TEMP3, 1, -atan3
                mad 1, TEMP3, 1, VIR_Lower_SetOne
                mul 1, TEMP2, 1, 0

            if (x < 0) t2 = -pi/2 - abs(atan); else t2 = pi/2 - abs(atan);

                add TEMP2, PI/2, 0, |atan|
                select.lt TEMP2, x, -TEMP2, TEMP2

            return flag ? t2 : atan;

                select.nz 1, TEMP1, TEMP2, 1


    { 1, gcSL_ATAN, 1, 2 },
        { -12, 0x10, gcSL_CG_TEMP1, 2, gcSL_CG_CONSTANT, 0, 0, gt_abs_0_one_1 },
        { -11, 0x0C, gcSL_CG_TEMP2, 0, 0, 2, 0, },
        { -10, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2, 0, conditionNZ },
        { -9, 0x03, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, 0, },
        { -8, 0x02, 1, gcSL_CG_TEMP3, gcSL_CG_CONSTANT, -gcSL_CG_CONSTANT, 0, atan9_1_atan7_2 },
        { -7, 0x02, 1, gcSL_CG_TEMP3, 1, gcSL_CG_CONSTANT, 0, atan5_2 },
        { -6, 0x02, 1, gcSL_CG_TEMP3, 1, -gcSL_CG_CONSTANT, 0, atan3_2 },
        { -5, 0x02, 1, gcSL_CG_TEMP3, 1, gcSL_CG_CONSTANT, 0, one_2 },
        { -4, 0x03, 1, gcSL_CG_TEMP2, 1, 0, 0, },
        { -3, 0x01, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, -1, 0, halfpi_0_abs_2 },
        { -2, 0x0F, gcSL_CG_TEMP2, 2, -gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, conditionLT },
        { -1, 0x0F, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 1, 0, conditionNZ },
    */
static VIR_PatternMatchInst _atanPatInst0[] = {
    { VIR_OP_ATAN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atanRepInst0[] = {
    { VIR_OP_ABS, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_AQ_SET, VIR_COP_GREATER, 0, { -1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetOne} },
    { VIR_OP_RCP, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -1, -2, 2 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -3, -2, -2, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, 0, 0 }, { 0, 0, atan9_1_atan7_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, atan5_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, atan3_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_MUL, 0, 0, { -7, -2, -7, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -4, -7, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -2, 0, -4, 0 }, { 0, half_pi } },
    { VIR_OP_NEG, 0, 0, { -5, -2, 0, 0 }, { 0 } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { -2, 2, -5, -2 }, { 0 } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, {  1, -1, -2, -7 }, { 0 } },
};

static VIR_Pattern _atanPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atan, 0) },
    { VIR_PATN_FLAG_NONE }
};


    /*
        LDARR 1, 2, 3
            MOVA  TEMP1, 3
            LDARR 1, 2, TEMP1

        If LDARR base is the per-patch/per-vertex data, no need for mova.
    */
static VIR_PatternMatchInst _ldarrPatInst0[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_prim_ctp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst0[] = {
    { VIR_OP_LDARR, 0, 0, {  1, 2, 3, 0 }, { 0 } },
};


static VIR_PatternMatchInst _ldarrPatInst1[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_not_sampler_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst1[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat } },
};

static VIR_PatternMatchInst _ldarrPatInst2[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst2[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt } },
};

static VIR_Pattern _ldarrPattern[] = {
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 1) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 2) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        STARR 1, 2, 3
            MOVA  2, 2
            STARR 1, 2, 3
    If STARR base is the per-patch/per-vertex data, no need for mova.
    */
static VIR_PatternMatchInst _starrPatInst0[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_dest_prim_ctp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst0[] = {
    { VIR_OP_STARR, 0, 0, {  1, 2, 3, 0 }, { 0 } },
};

static VIR_PatternMatchInst _starrPatInst1[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_dest_not_sampler_src0_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst1[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { _setSTARRSwizzleFloat } },
};

static VIR_PatternMatchInst _starrPatInst2[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst2[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { _setSTARRSwizzleInt } },
};

static VIR_Pattern _starrPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 2) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        SUB 1, 2, 3
            NEG  1, 3
    */
static VIR_PatternMatchInst _subPatInst0[] = {
    { VIR_OP_SUB, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _subRepInst0[] = {
    { VIR_OP_NEG, -1, 0, { 1, 3, 0, 0 }, { 0 } },
};

static VIR_Pattern _subPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sub, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        POW 1, 2, 3
            abs t1, 2
            log t2, 2
            mul t2, t2, 3
            exp  1, t2
    */
static VIR_PatternMatchInst _powPatInst0[] = {
    { VIR_OP_POW, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _powRepInst0[] = {
    { VIR_OP_ABS, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_PRE_LOG2, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -3, -2, 3, 0 }, { 0 } },
    { VIR_OP_EXP2, 0, 0, {  1, -3, 0, 0 }, { 0 } },
};

    /*
        POW 1, 2, 3
            log t1, 2
            mul t1, t1, 3
            exp  1, t1
    */
static VIR_PatternMatchInst _powPatInst1[] = {
    { VIR_OP_POW, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _powRepInst1[] = {
    { VIR_OP_PRE_LOG2, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -2, -1, 3, 0 }, { 0 } },
    { VIR_OP_EXP2, 0, 0, {  1, -2, 0, 0 }, { 0 } },
};

static VIR_Pattern _powPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_pow, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_pow, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*

        LOG 1, 2
            log 1, 0, 0, 2, 0

    { 1, gcSL_LOG, 1, 2, 0, 0, 0, _isCLShader_hasNEW_SIN_COS_LOG_DIV },
        { -1, 0x12, 1, 0, 0, 2, 0, set_new_sin_cos_log_div },
*/
static VIR_PatternMatchInst _logPatInst0[] = {
    { VIR_OP_LOG2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCLShader, _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _logRepInst0[] = {
    { VIR_OP_PRE_LOG2, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*
        LOG 1, 2
            select.gz 1, 2, 2, smallest0, 0
            log 1, 0, 0, 1, 0

    { 1, gcSL_LOG, 1, 2 },
        { -2, 0x0F, gcSL_CG_TEMP1, 2, 2, gcSL_CG_CONSTANT, 0, smallest0_2_GZ },
        { -1, 0x12, 1, 0, 0, gcSL_CG_TEMP1, 0, set_new_sin_cos_log_div },
*/
static VIR_PatternMatchInst _logPatInst1[] = {
    { VIR_OP_LOG2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _logRepInst1[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER_ZERO, 0, { -1, 2, 2, 0 }, { 0, 0, 0, VIR_Lower_SetZeroOrSamllestPositive } },
    { VIR_OP_PRE_LOG2, 0, 0, {  1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _logPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_log, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_log, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
    FRAC 1, 2
         frc 1, 0, 0, 2, 0
{ 1, gcSL_FRAC, 1, 2, 0, 0, 0, _isNotCLShader },
    { -1, 0x13, 1, 0, 0, 2, 0 },
*/

static VIR_PatternMatchInst _fracPatInst0[] = {
    { VIR_OP_FRAC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_IsNotCLShader }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _fracRepInst0[] = {
    { VIR_OP_FRAC, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*
    NOTE!!!! rounding mode
        FRAC 1, 2
            frc 1, 0, 0, 2, 0

    { 1, gcSL_FRAC, 1, 2, 0, 0, 0, _hasRounding_mode },
        { -1, 0x13, 1, 0, 0, 2, 0, rounding_mode },
*/
static VIR_PatternMatchInst _fracPatInst1[] = {
    { VIR_OP_FRAC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasRounding_mode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _fracRepInst1[] = {
    { VIR_OP_FRAC, 0, 0, {  1, 2, 0, 0 }, { 0 } },
};

/*

        FRAC 1, 2
            floor 1, 0, 0, 2, 0
            add 1, 2, 0, -1, 0

    { 1, gcSL_FRAC, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -2, 0x25, gcSL_CG_TEMP1, 0, 0, 2, 0 },
        { -1, 0x01, 1, 2, 0, -gcSL_CG_TEMP1, 0 },
*/
static VIR_PatternMatchInst _fracPatInst2[] = {
    { VIR_OP_FRAC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSIGN_FLOOR_CEIL }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _fracRepInst2[] = {
    { VIR_OP_FLOOR, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, {  1, 2, -1, 0 }, { 0 } },
};

/*
        FRAC 1, 2
            frc 1, 0, 0, 2, 0

    { 1, gcSL_FRAC, 1, 2 },
        { -1, 0x13, 1, 0, 0, 2, 0 },
*/
static VIR_PatternMatchInst _fracPatInst3[] = {
    { VIR_OP_FRAC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _fracRepInst3[] = {
    { VIR_OP_FRAC, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _fracPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_frac, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_frac, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_frac, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_frac, 3) },
    { VIR_PATN_FLAG_NONE }
};

/*
        FLOOR 1, 2
            floor 1, 0, 0, 2

    { 1, gcSL_FLOOR, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -1, 0x25, 1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _floorPatInst0[] = {
    { VIR_OP_FLOOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSIGN_FLOOR_CEIL }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _floorRepInst0[] = {
    { VIR_OP_FLOOR, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*

        FLOOR 1, 2
            frc 1, 0, 0, 2, 0
            add 1, 2, 0, -1, 0

    { 1, gcSL_FLOOR, 1, 2 },
        { -2, 0x13, gcSL_CG_TEMP1, 0, 0, 2, 0 },
        { -1, 0x01, 1, 2, 0, -gcSL_CG_TEMP2, 0 },
*/
static VIR_PatternMatchInst _floorPatInst1[] = {
    { VIR_OP_FLOOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _floorRepInst1[] = {
    { VIR_OP_FRAC, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, {  1, 2, -1, 0 }, { 0 } },
};

static VIR_Pattern _floorPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_floor, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_floor, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*

        CEIL 1, 2
            ceil 1, 0, 0, 2

    { 1, gcSL_CEIL, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL },
        { -1, 0x26, 1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _ceilPatInst0[] = {
    { VIR_OP_CEIL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSIGN_FLOOR_CEIL }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ceilRepInst0[] = {
    { VIR_OP_CEIL, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*
        CEIL 1, 2
            frc 1, 0, 0, 2, 0
            add TEMP1_XYZW, VIR_Lower_SetOne, 0, -1, 0
            select.gz 1, 1, TEMP1_XYZW, 1, 0
            add 1, 2, 0, 1, 0

    { 1, gcSL_CEIL, 1, 2 },
        { -4, 0x13, gcSL_CG_TEMP2, 0, 0, 2, 0 },
        { -3, 0x01, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, 0, -1, 0, one_0 },
        { -2, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP2, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2, 0, conditionGZ },
        { -1, 0x01, 1, 2, 0, gcSL_CG_TEMP2, 0 },
*/
static VIR_PatternMatchInst _ceilPatInst1[] = {
    { VIR_OP_CEIL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ceilRepInst1[] = {
    { VIR_OP_FRAC, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -1, 0, 1, 0 }, { 0, VIR_Lower_SetOne } },
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER_ZERO, 0, { -3, -2, -1, -2 }, { 0 } },
    { VIR_OP_ADD, 0, 0, {  1, 2, -3, 0 }, { 0 } },
};

static VIR_Pattern _ceilPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_ceil, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_ceil, 1) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        CROSS 1, 2, 3
            mul 1, 2.zxy, 3.yzx
            neg 1, 1
            mad 1, 3.zxy, 2.yzx, 1
    */
static VIR_PatternMatchInst _crossPatInst0[] = {
    { VIR_OP_CROSS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _crossRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { -1, 2, 3, 0 }, { crossSwizzle } },
    { VIR_OP_NEG, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, {  1, 3, 2, -2 }, { crossSwizzle } },
};

static VIR_Pattern _crossPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cross, 0) },
    { VIR_PATN_FLAG_NONE }
};


    /*
        SIGN 1, 2
            sign 1, 0, 0, 2

    { 1, gcSL_SIGN, 1, 2, 0, 0, 0, _hasSIGN_FLOOR_CEIL_and_float_type },
        { -1, 0x27, 1, 0, 0, 2, 0, value_type0 },
    */

static VIR_PatternMatchInst _signPatInst0[] = {
    { VIR_OP_SIGN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSIGN_FLOOR_CEIL, VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _signRepInst0[] = {
    { VIR_OP_SIGN, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

    /*
        SIGN 1, 2
            select.gz 1, 2, VIR_Lower_SetOne, 2
            select.eq 1, 1, VIR_Lower_SetZero, 1
            select.lz 1, 1, -VIR_Lower_SetOne, 2

    { 1, gcSL_SIGN, 1, 2, 0, 0, 0, _is_value_type_float },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, one_1_conditionGZ },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, zero_1_conditionEQ },
        { -1, 0x0F, 1, 2, -gcSL_CG_CONSTANT, 1, 0, one_1_conditionLZ },
    */
static VIR_PatternMatchInst _signPatInst1[] = {
    { VIR_OP_SIGN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _signRepInst1[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER_ZERO, 0, { 1, 2, 0, 2 }, { 0, 0, VIR_Lower_SetOne} },
    { VIR_OP_AQ_SELECT, VIR_COP_EQUAL, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS_ZERO, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetMinusOne } },
};

    /*
        SIGN 1, 2
            select.gz 1, 2, VIR_Lower_SetOne, 2
            select.eq 1, 1, VIR_Lower_SetZero, 1
            select.lz 1, 1, -VIR_Lower_SetOne, 2

    { 1, gcSL_SIGN, 1, 2 },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, int_one_1_conditionGZ },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, int_zero_1_conditionEQ },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 1, 0, int_minus_one_1_conditionLZ },
    */
static VIR_PatternMatchInst _signPatInst2[] = {
    { VIR_OP_SIGN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _signRepInst2[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER_ZERO, 0, { 1, 2, 0, 2 }, { 0, 0, VIR_Lower_SetIntOne } },
    { VIR_OP_AQ_SELECT, VIR_COP_EQUAL, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS_ZERO, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetIntMinusOne } },
};

static VIR_Pattern _signPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 2) },
    { VIR_PATN_FLAG_NONE }
};

    /*
        SQRT 1, 2
            rsq 1, 0, 0, 2, 0
            rcp 1, 0, 0, 1, 0
    */
static VIR_PatternMatchInst _sqrtPatInst0[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst0[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

    /*
        SQRT 1, 2
            rsq 1, 0, 0, 2, 0
            rcp 1, 0, 0, 1, 0
    */
static VIR_PatternMatchInst _sqrtPatInst1[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst1[] = {
    { VIR_OP_RSQ, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_RCP, 0, 0, {  1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _sqrtPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 1) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _getEXPPatInst0[] = {
    { VIR_OP_GETEXP, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _has_getEXP_getMANT }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _getEXPRepInst0[] = {
    { VIR_OP_GETEXP, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*
        { -3, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_7F800000 },
        { -2, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_23 },
        { -1, 0x01, 1, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_Minus127 },
*/
static VIR_PatternMatchInst _getEXPPatInst1[] = {
    { VIR_OP_GETEXP, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _getEXPRepInst1[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_7F800000 } },
    { VIR_OP_RSHIFT, 0, 0, { -2, -1, 0, 0 }, { int_value_type0_const_23 } },
    { VIR_OP_ADD, 0, 0, {  1, -2, 0, 0 }, { int_value_type0_const_Minus127 } },
};

static VIR_Pattern _getEXPPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_getEXP, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_getEXP, 1) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _getMANTPatInst0[] = {
    { VIR_OP_GETMANT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _has_getEXP_getMANT }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _getMANTRepInst0[] = {
    { VIR_OP_GETMANT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/*
    { 1, gcSL_GETMANT, 1, 2 },
        { -2, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_7FFFFF },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_800000 },
*/
static VIR_PatternMatchInst _getMANTPatInst1[] = {
    { VIR_OP_GETMANT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _getMANTRepInst1[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { uint_value_type0_const_7FFFFF } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, 0, 0 }, { uint_value_type0_const_800000 } },
};

static VIR_Pattern _getMANTPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_getMANT, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_getMANT, 1) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _dp2PatInst0[] = {
    { VIR_OP_DP2, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _dp2RepInst0[] = {
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW, { -1, 2, 3, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 0, { 1, -1, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY } },
};

static VIR_Pattern _dp2Pattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_dp2, 0) },
    { VIR_PATN_FLAG_NONE }
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtp) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x26, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst0[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat_Rtp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst0[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_CEIL, 0, 2, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_CONV, 0, 0, {  1, -3, 0, 0 }, { value_type0_32bit_reset_sat_rounding } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtn) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x25, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst1[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat_Rtn }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst1[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_FLOOR, 0, 2, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_CONV, 0, 0, {  1, -3, 0, 0 }, { value_type0_32bit_reset_sat_rounding } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtp) },
        { -2, 0x26, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */

static VIR_PatternMatchInst _convPatInst2[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Rtp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst2[] = {
    { VIR_OP_CEIL, 0, 0, { 1, 2, 0, 0 }, { 0, revise_dest_type_by_operand_type } },
    { VIR_OP_CONV, 0, 0, { 1, 1, 0, 0 }, { value_type0_32bit_reset_sat_rounding, set_opnd_type_prevInst_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtn) },
        { -2, 0x25, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst3[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Rtn }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst3[] = {
    { VIR_OP_FLOOR, 0, 0, { 1, 2, 0, 0 }, { 0, revise_dest_type_by_operand_type } },
    { VIR_OP_CONV, 0, 0, { 1, 1, 0, 0 }, { value_type0_32bit_reset_sat_rounding, set_opnd_type_prevInst_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat) },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -2, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst4[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst4[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_CONV, 0, 0, {  1, -2, 0, 0 }, { value_type0_32bit_reset_sat_rounding } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isF2I },
        { -1, 0x2E, 1, 2, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst5[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst5[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { value_type0_32bit } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2F },
        { -1, 0x2D, 1, 2, 0, 0, 0, _value_type0_32bit_from_src0 },
    */
static VIR_PatternMatchInst _convPatInst6[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2F }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst6[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { _value_type0_32bit_from_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2us) },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -1, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst7[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst7[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, {  1, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_u2us) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
    */
static VIR_PatternMatchInst _convPatInst8[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_u2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst8[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, { 1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2u) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst9[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2u }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst9[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { 1, 2, 0, 2 }, { min_type0_const, revise_dest_type_by_operand_type } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I },
        { -1, 0x2C, 1, 2, gcSL_CG_CONSTANT, 0, 0, value_types_I2I },
    */
static VIR_PatternMatchInst _convPatInst10[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst10[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Signed_8_16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
    */
static VIR_PatternMatchInst _convPatInst11[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Signed_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst11[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_24_16 } },
    { VIR_OP_RSHIFT, 0, 0, { 1, -1, 0, 0 }, { int_value_type0_const_24_16 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Unsigned_8_16) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },
    */
static VIR_PatternMatchInst _convPatInst12[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Unsigned_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst12[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { uint_value_type0_const_FF_FFFF } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_8bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FF },
    */
static VIR_PatternMatchInst _convPatInst13[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_8bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst13[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
    */
static VIR_PatternMatchInst _convPatInst14[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst14[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { short_value_type0_const_8 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { short_value_type0_const_8 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FFFF },
    */
static VIR_PatternMatchInst _convPatInst15[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst15[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FFFF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
    */
static VIR_PatternMatchInst _convPatInst16[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst16[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_24 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { int_value_type0_const_24 } },
};
    /*
        CONV 1, 2
            lshift 1, 2, 0, constant_16, 0
            rshift 1, 1, 0, constant_16, 0

    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_16 },
    */
static VIR_PatternMatchInst _convPatInst17[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst17[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { int_value_type0_const_16 } },
};

static VIR_PatternMatchInst _convPatInst18[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { supportCONV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst18[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF16_2_F32_hasCMP_NotFullNewLinker) },
        { -12, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, float16_exp },
        { -11, 0x59, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, float16_man_bits },
        { -10, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_iszero },
        { -9, 0x31, gcSL_CG_TEMP3, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_isnan },
        { -8, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, float16_exp_isaddnanNZ },
        { -7, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_32 },
        { -6, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float16_sign },
        { -5, 0x59, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float16_exp_bits },
        { -4, 0x5C, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_32 },
        { -3, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float16_man },
        { -2, 0x59, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float16_man_bits },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_32 },
    */
static VIR_PatternMatchInst _convPatInst19[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF16_2_F32_hasCMP_NotFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst19[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { float16_exp } },
    { VIR_OP_LSHIFT, 0, 0, { -1, -1, 0, 0 }, { float16_man_bits } },
    { VIR_OP_CMP, VIR_COP_EQUAL, 0, { -2, -1, 0, 0 }, { 0, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -2, 0, 0 }, { float16_exp_iszero } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, -1, 0, 0 }, { float16_exp_isnan } },
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL,0, { -4, -3, 0, 0 }, { float16_exp_isnan } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -4, -2, 0 }, { float16_exp_isaddnanNZ } },
    { VIR_OP_ADD, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } }, /* NOTE!!!!!!!!! */
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_sign } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_man } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_man_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { value_types_32 } },
};

/*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF32_2_F16_hasCMP_NotFullNewLinker) },
        { -12, 0x5D, gcSL_CG_TEMP1, 2, 0, gcSL_CG_CONSTANT, 0, float32_exp },
        { -11, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float16_exp_iszero },
        { -10, 0x31, gcSL_CG_TEMP3, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, gcSL_CG_CONSTANT, 0, float32_exp_isnan },
        { -9, 0x0F, gcSL_CG_TEMP2, gcSL_CG_TEMP3, gcSL_CG_TEMP2, gcSL_CG_CONSTANT, 0, float16_exp_isaddnanNZ },
        { -8, 0x01, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, -gcSL_CG_TEMP2, 0, value_types_32 },
        { -7, 0x5A, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_CONSTANT, 0, float32_man_bits },
        { -6, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float32_sign },
        { -5, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float32_exp_bits },
        { -4, 0x5C, gcSL_CG_TEMP1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_16 },
        { -3, 0x5D, gcSL_CG_TEMP2, 2, 0, gcSL_CG_CONSTANT, 0, float32_man },
        { -2, 0x5A, gcSL_CG_TEMP2, gcSL_CG_TEMP2, 0, gcSL_CG_CONSTANT, 0, float32_man_bits },
        { -1, 0x5C, 1, gcSL_CG_TEMP1, 0, gcSL_CG_TEMP2, 0, value_types_16 },
    */
static VIR_PatternMatchInst _convPatInst20[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF32_2_F16_hasCMP_NotFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst20[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { float32_exp } },
    { VIR_OP_CMP, VIR_COP_EQUAL, 0, { -2, -1, 0, 0 }, { _setOperandUINT, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -2, 0, 0 }, { float16_exp_iszero } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, -1, 0, 0 }, { float32_exp_isnan } },
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL, 0, { -4, -3, 0, 0 }, { float32_exp_isnan } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -4, -2, 0 }, { float16_exp_isaddnanNZ } },
    { VIR_OP_SUB, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } },
    { VIR_OP_RSHIFT, 0, 0, { -1, -1, 0, 0 }, { float32_man_bits } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float32_sign } },
    { VIR_OP_RSHIFT, 0, 0, { -2, -2, 0, 0 }, { float32_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { value_types_16 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float32_man } },
    { VIR_OP_RSHIFT, 0, 0, { -2, -2, 0, 0 }, { float32_man_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { value_types_16 } },
};

static VIR_PatternMatchInst _convPatInst21[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF16_2_F32_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst21[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { float16_exp } },
    { VIR_OP_LSHIFT, 0, 0, { -1, -1, 0, 0 }, { float16_man_bits } },
    { VIR_OP_CMP, VIR_COP_EQUAL, 0, { -2, -1, 0, 0 }, { _setBooleanType, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -2, 0, 0 }, { float16_exp_iszero } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, -1, 0, 0 }, { float16_exp_isnan } },
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL,0, { -4, -3, 0, 0 }, { _setBooleanType, float16_exp_isnan } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -4, -2, 0 }, { float16_exp_isaddnanNZ } },
    { VIR_OP_ADD, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } }, /* NOTE!!!!!!!!! */
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_sign } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_man } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_man_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { value_types_32 } },
};

static VIR_PatternMatchInst _convPatInst22[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF32_2_F16_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst22[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { float32_exp } },
    { VIR_OP_CMP, VIR_COP_EQUAL, 0, { -2, -1, 0, 0 }, { _setBooleanType, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -2, 0, 0 }, { float16_exp_iszero } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, -1, 0, 0 }, { float32_exp_isnan } },
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL, 0, { -4, -3, 0, 0 }, { _setBooleanType, float32_exp_isnan } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -2, -4, -2, 0 }, { float16_exp_isaddnanNZ } },
    { VIR_OP_SUB, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } },
    { VIR_OP_RSHIFT, 0, 0, { -1, -1, 0, 0 }, { float32_man_bits } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float32_sign } },
    { VIR_OP_RSHIFT, 0, 0, { -2, -2, 0, 0 }, { float32_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { value_types_16 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float32_man } },
    { VIR_OP_RSHIFT, 0, 0, { -2, -2, 0, 0 }, { float32_man_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { value_types_16 } },
};

static VIR_Pattern _convPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 8) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 10) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 11) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 12) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 13) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 14) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 15) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 16) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 17) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 18) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 19) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 20) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 21) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 22) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _jmpcPatInst0[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ2_resCondOp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst0[] = {
    { VIR_OP_CMOV, -1, 0, { 4, 2, 3, 5 }, { reverseCondOp } }
};

static VIR_PatternMatchInst _jmpcPatInst1[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_float, all_source_single_value, jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst1[] = {
    { VIR_OP_AQ_SET, -1, 0, { 4, 2, 3, }, { reverseCondOp } },
};

static VIR_PatternMatchInst _jmpcPatInst2[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_float, all_source_single_value, jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst2[] = {
    { VIR_OP_AQ_SET, -1, 0, { 4, 2, 3, }, { 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst3[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, _isSrc0Zero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { canBeMergedToSelect1, _dstSrcSamePrecsion}, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst3[] = {
    { VIR_OP_SELECT, -1, 0, { 4, 3, 8, 5 }, { setSwitchedCondOpCompareWithZero } },
};

static VIR_PatternMatchInst _jmpcPatInst4[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, _isSrc1Zero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { canBeMergedToSelect0, _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst4[] = {
    { VIR_OP_SELECT, -1, 0, { 4, 2, 8, 5 }, { setCondOpCompareWithZero } },
};

static VIR_PatternMatchInst _jmpcPatInst5[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, VIR_Lower_enableFullNewLinker }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { dest_type_less_than_prev_jmp_src0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

/* CMP, if dest.type is not float,
   dest = cond_op(src0, src1) ? 0xFFFFFFFF: 0,
   thus, we can use SELMSB condition for select.
   Using ALLMSB, we can resolve the issue that select has one instruction type to control
   comparison and implicit converstion */
static VIR_PatternReplaceInst _jmpcRepInst5[] = {
    { VIR_OP_CMP, -1, 2, { -1, 2, 3, 0 }, { _setBooleanType } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, {  4, -1, 8, 5 }, { 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst6[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { dest_type_less_than_prev_jmp_src0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst6[] = {
    { VIR_OP_CMP, -1, 2, { -1, 2, 3, 0 }, { _setIntegerType } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, {  4, -1, 8, 5 }, { 0 } },
};


static VIR_PatternMatchInst _jmpcPatInst7[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { jmp_2_succ2_resCondOp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_KILL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { no_source }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst7[] = {
    { VIR_OP_KILL, -1, 0, { 0, 2, 3, 0 }, { reverseCondOp } }
};

static VIR_Pattern _jmpcPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 7) },
    { VIR_PATN_FLAG_NONE }
};


/* this is a WAR pattern, after recompiling old optimizer insert
   MOV before CMP when dst and src are in same Reg. This causes
   CMP.NOT and CMP.NZ could not be merged into SELECT*/
static VIR_PatternMatchInst _movPatInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMP, VIR_COP_NOT, 0, { 3, 1, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 5, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, { 3, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _movRepInst0[] = {
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 0, {  5, 2, 0, 0 }, { 0 } },
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, {  3, 2, 6, 4 }, { 0 } },
};

static VIR_Pattern _movPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mov, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _cmpPatInst0[] = {
    { VIR_OP_CMP, VIR_COP_NOT, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst0[] = {
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, {  1, 2, 5, 3 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst1[] = {
    { VIR_OP_CMP, VIR_COP_SELMSB, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CMP, VIR_COP_SELMSB, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst1[] = {
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, {  1, 2, 3, 5 }, { 0 } },
};

static VIR_Pattern _cmpPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 1, gcSL_TEXLODQ, 1, 2, 3, 0, 0, _hasHalti3 },
        { -1, 0x7F, 1, 3, 0, gcSL_CG_CONSTANT, 2, set_extended_opcode_lodq },
*/
static VIR_PatternMatchInst _lodqPatInst0[] = {
    { VIR_OP_LODQ, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _lodqRepInst0[] = {
    { VIR_OP_LODQ, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXGRAD, 0, 4, 5, 0, 0, _hasHalti3 },
    { 1, gcSL_TEXLODQ, 1, 2, 3, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP1, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP2, 3, 0, 5, 0 },
        { -1, 0x7C, 1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2 },
*/
static VIR_PatternMatchInst _lodqPatInst1[] = {
    { VIR_OP_LODQ, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _lodqRepInst1[] = {
    { VIR_OP_ADD, -1, 0, { -1, 3, 4, 0 }, { _setFloatType } },
    { VIR_OP_ADD, -1, 0, { -2, 3, 5, 0 }, { _setFloatType } },
    { VIR_OP_LODQ, -1, 0, {  1, 2, 3, -1, -2 }, { 0 } },
};

VIR_Pattern _lodqPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_lodq, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_lodq, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, 0, 2, zero_1 },
*/
static VIR_PatternMatchInst _texldPatInst0[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst0[] = {
    { VIR_OP_TEXLD_LOD, -1, 0, { 1, 2, 3, 0 }, { 0, 0, 0, VIR_Lower_SetIntZero } },
};

/*
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _isVertex },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst1[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst1[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { 0, VIR_Lower_SetZero } },
    { VIR_OP_TEXLDL, 0, 0, {  1, 2, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x18, 1, 3, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst2[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst2[] = {
    { VIR_OP_TEXLD_PLAIN, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x6F, 1, 3, 4, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst3[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst3[] = {
    { VIR_OP_TEXLD_LOD, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst4[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst4[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLDL, 0, 0, {  1, 2, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -1, 0x18, 1, 3, 4, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst5[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst5[] = {
    { VIR_OP_TEXLD_BIAS, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -1, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst6[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst6[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLDB, 0, 0, {  1, 2, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x01, gcSL_CG_TEMP1, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP2, 3, 0, 5, 0 },
        { -1, 0x1A, 1, 3, gcSL_CG_TEMP1, gcSL_CG_TEMP2, 2 },
*/
static VIR_PatternMatchInst _texldPatInst7[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldRepInst7[] = {
    { VIR_OP_ADD, 0, 4, { -1, 4, 3, 0 }, { _setGradType } },
    { VIR_OP_ADD, 0, 5, { -2, 5, 3, 0 }, { _setGradType } },
    { VIR_OP_TEXLD_G, 0, 0, {  1, 2, 3, -1, -2 }, { 0 } },
};

/*
    { 2, gcSL_TEXGATHER, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3 },
        { -1, 0x7D, 1, 3, 4, 0, 2 },
*/
static VIR_PatternMatchInst _texldPatInst8[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4, 5 }, { _isGatherTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst8[] = {
    { VIR_OP_TEXLD_GATHER, 0, 0, { 1, 2, 3, 4, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXFETCH_MS, 0, 2, 4, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLD, 1, 2, 3, 0, 0 },
        { -1, 0x7F, 1, 3, 4, 0, 2, set_extended_opcode_fetchms },
*/
static VIR_PatternMatchInst _texldPatInst9[] = {
    { VIR_OP_TEXLD, -1, 0, { 1, 2, 3, 4, 0 }, { _isFetchMsTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldRepInst9[] = {
    { VIR_OP_TEXLD_FETCH_MS, 0, 0, { 1, 2, 3, 4, 0 }, { 0 } },
};

VIR_Pattern _texldPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 8) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texld, 9) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_isCoordFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * coord = VIR_Inst_GetSource(Inst, 1);
    return VIR_TypeId_isFloat(VIR_Operand_GetType(coord));
}

static gctBOOL
_isCoordSignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * coord = VIR_Inst_GetSource(Inst, 1);
    return VIR_TypeId_isSignedInteger(VIR_Operand_GetType(coord));
}

static gctBOOL
_isCoordUnSignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * coord = VIR_Inst_GetSource(Inst, 1);
    return VIR_TypeId_isUnSignedInteger(VIR_Operand_GetType(coord));
}

static gctBOOL
_genFloatCoordData(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    gctUINT dataType = 0x1; /* 0x1 */
    gctUINT addressingType = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)));

    VIR_Operand_SetImmediate(Inst->src[0],
        VIR_TYPE_UINT32,
        imm);

    return gcvTRUE;
}

static gctBOOL
_genIntegeroordData(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    gctUINT dataType = 0x0; /* 0x0 */
    gctUINT addressingType = 0x0; /* 0x0 */
    gctUINT magFilter = 0x0; /* 0x0 */
    gctUINT minFilter = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 10:8) - (0 ? 10:8) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 10:8) - (0 ? 10:8) + 1))))))) << (0 ?
 10:8)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (magFilter) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (minFilter) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0 : (~(~0 << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));

    VIR_Operand_SetImmediate(Inst->src[0],
        VIR_TYPE_UINT32,
        imm);

    return gcvTRUE;
}

static gctBOOL
_setSource3UnsignedZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    imm.uValue = 0;

    VIR_Operand_SetImmediate(Inst->src[3],
        VIR_TYPE_UINT32,
        imm);

    return gcvTRUE;
}

/* Generate TEXLD_U_F_L. */
static VIR_PatternMatchInst _texlduPatInst0[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst0[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { _genFloatCoordData } },
    { VIR_OP_TEXLD_U_F_L, 0, 0, { 1, 2, -1, 4, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW, 0, _setSource3UnsignedZero } },
};

/* Generate TEXLD_U_F_B. */
static VIR_PatternMatchInst _texlduPatInst1[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst1[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { _genFloatCoordData } },
    { VIR_OP_TEXLD_U_F_B, 0, 0, { 1, 2, -1, 4, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW, 0, _setSource3UnsignedZero } },
};

/* Generate TEXLD_U_S_L. */
static VIR_PatternMatchInst _texlduPatInst2[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst2[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { _genIntegeroordData } },
    { VIR_OP_TEXLD_U_S_L, 0, 0, { 1, 2, -1, 4, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW, 0, _setSource3UnsignedZero } },
};

/* Generate TEXLD_U_U_L. */
static VIR_PatternMatchInst _texlduPatInst3[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordUnSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst3[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { _genIntegeroordData } },
    { VIR_OP_TEXLD_U_U_L, 0, 0, { 1, 2, -1, 4, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW, 0, _setSource3UnsignedZero } },
};


VIR_Pattern _texlduPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 3) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -3, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW },
        { -2, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -1, 0x6F, 1, gcSL_CG_TEMP1, gcSL_CG_CONSTANT, 0, 2, zero_1 },
*/
static VIR_PatternMatchInst _texldprojPatInst0[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst0[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_TEXLD_LOD, 0, 0, {  1, 2, -1, 0 }, { 0, 0, 0, VIR_Lower_SetZero } },
};

/*
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _isVertex },
        { -4, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW },
        { -3, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst1[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst1[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_XYZ,{ -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZ,{ -2, 3, -1, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -2, 0, 0, 0 }, { 0, VIR_Lower_SetZero } },
    { VIR_OP_TEXLDL, 0, 0, {  1, 2, -2, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -3, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW },
        { -2, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -1, 0x18, 1, gcSL_CG_TEMP1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst2[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst2[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -2, 3, -1, 0 }, { 0 } },
    { VIR_OP_TEXLD, 0, 0, {  1, 2, -2, 0 }, { 0, 0, VIR_Lower_SetSwizzleByCoord } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -2, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -1, 0x18, 1, gcSL_CG_TEMP1, 4, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst3[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst3[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_TEXLD_BIAS, 0, 0, {  1, 2, -1, 4 }, { 0 } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -4, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -3, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 4, 0, enable_w },
        { -1, 0x19, 1, gcSL_CG_TEMP1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst4[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst4[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -2, 3, -1, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 4, { -2, 4, 0, 0 }, { destEnableW } },
    { VIR_OP_TEXLDB, 0, 0, {  1, 2, -2, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -5, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -4, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, 3, 0, 5, 0 },
        { -1, 0x1A, 1, gcSL_CG_TEMP1, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst5[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst5[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_ADD, 0, 4, { -2, 4, -1, 0 }, { _setGradType } },
    { VIR_OP_ADD, 0, 5, { -3, 5, -1, 0 }, { _setGradType } },
    { VIR_OP_TEXLD_G, 0, 0, {  1, 2, -1, -2, -3 }, { 0 } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -3, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -2, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -1, 0x6F, 1, gcSL_CG_TEMP1, 4, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst6[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldprojRepInst6[] = {
    { VIR_OP_RCP, 0, 3, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, 3, { -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_TEXLD_LOD, 0, 0, {  1, 2, -1, 4 }, { 0 } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPROJ, 1, 2, 3 },
        { -4, 0x0C, gcSL_CG_TEMP1, 0, 0, 3, 0, swizzle2ZorW_from_next_inst },
        { -3, 0x03, gcSL_CG_TEMP1, 3, gcSL_CG_TEMP1, 0, 0 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 4, 0, enable_w },
        { -1, 0x1B, 1, gcSL_CG_TEMP1, 0, 0, 2 },
*/
static VIR_PatternMatchInst _texldprojPatInst7[] = {
    { VIR_OP_TEXLDPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldprojRepInst7[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLDL, 0, 0, {  1, 2, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleXYZW } },
};

VIR_Pattern _texldprojPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldproj, 7) },
    { VIR_PATN_FLAG_NONE }
};

/*
  { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, zero_1_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst0[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst0[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, 3, 0, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_AdjustCoordSwizzleForShadow, VIR_Lower_SetZero } },
};

/*
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _isVertex },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -3, AQ_INST_OP_CODE_SAT, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
*/
static VIR_PatternMatchInst _texldpcfPatInst1[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst1[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { 0, VIR_Lower_SetZero } },
    { VIR_OP_TEXLDL, 0, 0, { -3, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_AdjustCoordSwizzleForShadow } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
};

/*
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -1, 0x18, 1, 3, 0, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst2[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst2[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_PCF, 0, 0, {  1, 2, 3, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_AdjustCoordSwizzleForShadow } },
};

/*
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -2, 0x18, 1, 3, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
*/
static VIR_PatternMatchInst _texldpcfPatInst3[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst3[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD, 0, 0, { -2, 2, 3, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_AdjustCoordSwizzleForShadow} },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -1, 0x6F, 1, 3, 4, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst4[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst4[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, 3, 4, -1 }, { 0, VIR_Lower_SetSwizzleX } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -3, 0x09, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
*/
static VIR_PatternMatchInst _texldpcfPatInst5[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst5[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLDL, 0, 0, { -3, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst6[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow, _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst6[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, 3, 0, -1 }, { 0, 0, VIR_Lower_AdjustCoordSwizzleForShadow, VIR_Lower_SetZero } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst7[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow, _hasNEW_TEXLD, }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst7[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_PCF, 0, 0, {  1, 2, 3, 0, -1 }, { 0, 0, VIR_Lower_AdjustCoordSwizzleForShadow} },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -1, 0x18, 1, 3, 4, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst8[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst8[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_BIAS_PCF, 0, 0, {  1, 2, 3, 4, -1 }, { 0, VIR_Lower_SetSwizzleX } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst9[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst9[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_PCF, 0, 0, { -2, 2, 3, 0 }, { 0 } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -3, 0x09, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP2_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
*/
static VIR_PatternMatchInst _texldpcfPatInst10[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst10[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLDB, 0, 0, { -3, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
};

/*
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -5, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0 },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_w_saturate_swizzle2ZorW_from_next_inst },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, 3, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, 3, 0, 5, 0 },
        { -1, 0x70, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst11[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst11[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZW, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, -1, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_ADD, 0, 4, { -2, 3, 4, 0, 0 }, { _setGradType } },
    { VIR_OP_ADD, 0, 5, { -3, 3, 5, 0, 0 }, { _setGradType } },
    { VIR_OP_TEXLD_G_PCF, 0, 0, {  1, 2, -1, -2, -3 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
};

/*
    { 2, gcSL_TEXGATHER, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -2, 0x09, gcSL_CG_TEMP1, 0, 0, 5, 0, saturate },
        { -1, 0x7D, 1, 3, 4, gcSL_CG_TEMP1_X, 2 },
*/
static VIR_PatternMatchInst _texldpcfPatInst12[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4, 5 }, { _isGatherTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst12[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 5, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_GATHER_PCF, 0, 0, {  1, 2, 3, 4, -1 }, { 0 } },
};

VIR_Pattern _texldpcfPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 8) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 10) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 11) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 12) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_XYZW, 2, zero_1_swizzle2Z_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst0[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst0[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, -1, 0, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, VIR_Lower_SetZero, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _isVertex },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_CONSTANT, 0, zero_2_enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst1[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst1[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 0, 0, 0 }, { 0, VIR_Lower_SetZero } },
    { VIR_OP_TEXLDL, 0, 0, { -2, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst2[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst2[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleW } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_PCF, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{  1, 2, -1, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -5, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -4, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -2, 0x18, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, swizzle0XY_sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst3[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst3[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD, 0, 0, { -2, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, _Set_SwizzleXYEx } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst4[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst4[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_LOD_PCF, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{  1, 2, -1, 4, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 2, gcSL_TEXLOD, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x1B, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst5[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst5[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLDL, 0, 0, { -2, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x18, 1, gcSL_CG_TEMP1_XYZW, 4, gcSL_CG_TEMP1_XYZW, 2, swizzle2Z_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst6[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst6[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0 } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_BIAS_PCF, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{  1, 2, -1, 4, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3 },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -3, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, 4, 0, enable_w },
        { -2, 0x19, 1, gcSL_CG_TEMP1_XYZW, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_XYZW, 1, 0, 0, conditionLE_swizzle0Z },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst7[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst7[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_TEXLDB, 0, 0, { -2, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
};

/*
    { 2, gcSL_TEXGRAD, 0, 4, 5 },
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -6, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -5, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -4, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_w_saturate_swizzle2Z },
        { -3, 0x01, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP1_XYZW, 0, 4, 0 },
        { -2, 0x01, gcSL_CG_TEMP3_XYZW, gcSL_CG_TEMP1_XYZW, 0, 5, 0 },
        { -1, 0x70, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_TEMP2_XYZW, gcSL_CG_TEMP3_XYZW, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst8[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst8[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_ADD, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -2, 4, -1, 0 }, { _setFloatType, 0, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_ADD, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -3, 5, -1, 0 }, { _setFloatType, 0, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_TEXLD_G_PCF, 0, 0, {  1, 2, -1, -2, -3 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
};

VIR_Pattern _texldpcfprojPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 8) },
    { VIR_PATN_FLAG_NONE }
};

/*
        LOAD 1, 2, 3
        ATOMADD 4, 1, 5
            atomadd 4, 2, 3, 5
*/
static VIR_PatternMatchInst _loadPatInst0[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMADD, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst0[] = {
    { VIR_OP_ATOMADD, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst1[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMSUB, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst1[] = {
    { VIR_OP_ATOMSUB, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst2[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMXCHG, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst2[] = {
    { VIR_OP_ATOMXCHG, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst3[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMMIN, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst3[] = {
    { VIR_OP_ATOMMIN, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst4[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMMAX, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst4[] = {
    { VIR_OP_ATOMMAX, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst5[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMOR, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst5[] = {
    { VIR_OP_ATOMOR, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst6[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMAND, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst6[] = {
    { VIR_OP_ATOMAND, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst7[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMXOR, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst7[] = {
    { VIR_OP_ATOMXOR, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _loadPatInst8[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_ATOMCMPXCHG, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst8[] = {
    { VIR_OP_ATOMCMPXCHG, 0, 0, {  4, 2, 3, 5 }, { 0 } },
};

static VIR_Pattern _loadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 8) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomaddPatInst0[] = {
    { VIR_OP_ATOMADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomaddRepInst0[] = {
    { VIR_OP_ATOMADD, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomaddPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomadd, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomsubPatInst0[] = {
    { VIR_OP_ATOMSUB, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomsubRepInst0[] = {
    { VIR_OP_ATOMSUB, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomsubPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomsub, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomandPatInst0[] = {
    { VIR_OP_ATOMAND, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomandRepInst0[] = {
    { VIR_OP_ATOMAND, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomandPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomand, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomorPatInst0[] = {
    { VIR_OP_ATOMOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomorRepInst0[] = {
    { VIR_OP_ATOMOR, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomorPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomor, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomxorPatInst0[] = {
    { VIR_OP_ATOMXOR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomxorRepInst0[] = {
    { VIR_OP_ATOMXOR, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomxorPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomxor, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atommaxPatInst0[] = {
    { VIR_OP_ATOMMAX, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atommaxRepInst0[] = {
    { VIR_OP_ATOMMAX, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atommaxPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atommax, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomminPatInst0[] = {
    { VIR_OP_ATOMMIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomminRepInst0[] = {
    { VIR_OP_ATOMMIN, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomminPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atommin, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomxchgPatInst0[] = {
    { VIR_OP_ATOMXCHG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomxchgRepInst0[] = {
    { VIR_OP_ATOMXCHG, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomxchgPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomxchg, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _atomcmpxchgPatInst0[] = {
    { VIR_OP_ATOMCMPXCHG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _atomcmpxchgRepInst0[] = {
    { VIR_OP_ATOMCMPXCHG, 0, 0, {  1, 2, 0, 3 }, { 0, 0, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _atomcmpxchgPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_atomcmpxchg, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 1, gcSL_UCARRY, 1, 2, 3, 0, 0, _hasHalti4 },
        { -1, 0x10, 1, 2, 3, 0, 0, conditionUCARRY},
*/
static VIR_PatternMatchInst _ucarryPatInst0[] = {
    { VIR_OP_UCARRY, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ucarryRepInst0[] = {
    { VIR_OP_AQ_SET, VIR_COP_UCARRY, 0, { 1, 2, 3, 0 }, { 0 } },
};

VIR_Pattern _ucarryPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_ucarry, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    { 2, gcSL_BITRANGE, 0, 4, 5, 0, 0, _hasHalti4 },
    { 1, gcSL_BITINSERT, 1, 2, 3 },
        { -3, 0x59, gcSL_CG_TEMP1_X, 5, 0, gcSL_CG_CONSTANT, 0, eight_2 },
        { -2, 0x5C, gcSL_CG_TEMP1_X, gcSL_CG_TEMP1_X, 0, 4, 0, value_type0 },
        { -1, 0x54, 1, 2, 3, gcSL_CG_TEMP1_X, 0, value_type0},
*/
static VIR_PatternMatchInst _bitInsertPatInst0[] = {
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitInsertRepInst0[] = {
    { VIR_OP_LSHIFT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 5, 0, 0, 0 }, { 0, 0, eight } },
    { VIR_OP_OR_BITWISE, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_BITINSERT, 0, 0, {  1, 2, 3, -2, 0 }, { 0 } },
};

VIR_Pattern _bitInsertPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitInsert, 0) },
    { VIR_PATN_FLAG_NONE }
};

static
gctBOOL _needSetConstBorderValue(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return gcmOPT_hasFeature(FB_ENABLE_CONST_BORDER);
}


static gctBOOL
_setConstBorderValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Shader *  shader = Context->shader;
    VIR_Uniform * uniform = VIR_Shader_GetConstBorderValueUniform(shader);
    VIR_TypeId    tyId = VIR_Operand_GetType(VIR_Inst_GetDest(Inst));
    VIR_TypeId    compTyId;
    VIR_Swizzle   swizzle = VIR_SWIZZLE_XXXX;

    VIR_Operand_SetUniform(Opnd, uniform, shader);
    /* set swizzle by dest's type: U8 => .x, U16 => .y, U32 => .z, F32 => .w */
    compTyId = VIR_GetTypeComponentType(tyId);
    switch(compTyId) {
    case VIR_TYPE_UINT8:
    case VIR_TYPE_INT8:
        swizzle = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_UINT16:
    case VIR_TYPE_INT16:
        swizzle = VIR_SWIZZLE_YYYY;
        break;
    case VIR_TYPE_UINT32:
    case VIR_TYPE_INT32:
        swizzle = VIR_SWIZZLE_ZZZZ;
        break;
    case VIR_TYPE_FLOAT32:
        swizzle = VIR_SWIZZLE_WWWW;
        break;
    default:
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

static VIR_PatternMatchInst _vxImgLoadPatInst0[] = {
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoadRepInst0[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_VX_IMG_LOAD, 0, 0, {  1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _vxImgLoadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgLoad3DPatInst0[] = {
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoad3DRepInst0[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, { 1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _vxImgLoad3DPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad3D, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetLowerPatternPhaseExpand(
    IN  VIR_PatternContext * Context,
    IN VIR_Instruction     * Inst
    )
{
    switch(VIR_Inst_GetOpcode(Inst))
    {
    case VIR_OP_POW:
        return _powPattern;
    case VIR_OP_STARR:
        return _starrPattern;
    case VIR_OP_LDARR:
        return _ldarrPattern;
    case VIR_OP_SIN:
        return _sinPattern;
    case VIR_OP_COS:
        return _cosPattern;
    case VIR_OP_TAN:
        return _tanPattern;
    case VIR_OP_FWIDTH:
        return _fwidthPattern;
    case VIR_OP_TANPI:
        return _tanpiPattern;
    case VIR_OP_ASIN:
        return _asinPattern;
    case VIR_OP_ACOS:
        return _acosPattern;
    case VIR_OP_ATAN:
        return _atanPattern;
    case VIR_OP_SUB:
        return _subPattern;
    case VIR_OP_FRAC:
        return _fracPattern;
    case VIR_OP_FLOOR:
        return _floorPattern;
    case VIR_OP_CEIL:
        return _ceilPattern;
    case VIR_OP_CROSS:
        return _crossPattern;
    case VIR_OP_SIGN:
        return _signPattern;
    case VIR_OP_SQRT:
        return _sqrtPattern;
    case VIR_OP_GETEXP:
        return _getEXPPattern;
    case VIR_OP_GETMANT:
        return _getMANTPattern;
    case VIR_OP_CONV:
        return _convPattern;
    case VIR_OP_LOG2:
        return _logPattern;
    case VIR_OP_DIV:
        return _divPattern;
    case VIR_OP_JMPC:
        return _jmpcPattern;
    case VIR_OP_MOV:
        return _movPattern;
    case VIR_OP_CMP:
        return _cmpPattern;
    case VIR_OP_DP2:
        return _dp2Pattern;
    case VIR_OP_LOAD:
        return _loadPattern;
    case VIR_OP_ATOMADD:
        return _atomaddPattern;
    case VIR_OP_ATOMSUB:
        return _atomsubPattern;
    case VIR_OP_ATOMAND:
        return _atomandPattern;
    case VIR_OP_ATOMOR:
        return _atomorPattern;
    case VIR_OP_ATOMXOR:
        return _atomxorPattern;
    case VIR_OP_ATOMMIN:
        return _atomminPattern;
    case VIR_OP_ATOMMAX:
        return _atommaxPattern;
    case VIR_OP_ATOMXCHG:
        return _atomxchgPattern;
    case VIR_OP_ATOMCMPXCHG:
        return _atomcmpxchgPattern;
    case VIR_OP_VX_IMG_LOAD:
        return _vxImgLoadPattern;
    case VIR_OP_VX_IMG_LOAD_3D:
        return _vxImgLoad3DPattern;
    default:
        break;
    }

    if (ENABLE_FULL_NEW_LINKER)
    {
        switch(VIR_Inst_GetOpcode(Inst))
        {
        case VIR_OP_LODQ:
            return _lodqPattern;
        case VIR_OP_TEXLD:
            return _texldPattern;
        case VIR_OP_TEXLD_U:
            return _texlduPattern;
        case VIR_OP_TEXLDPCF:
            return _texldpcfPattern;
        case VIR_OP_TEXLDPROJ:
            return _texldprojPattern;
        case VIR_OP_TEXLDPCFPROJ:
            return _texldpcfprojPattern;
        case VIR_OP_UCARRY:
            return _ucarryPattern;
        case VIR_OP_BITINSERT:
            return _bitInsertPattern;
        default:
            break;
        }
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
VIR_Lower_MiddleLevel_To_LowLevel_Expand(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    VIR_PatternContext_Initialize(&Context->header, Shader, VIR_PATN_CONTEXT_FLAG_NONE, _GetLowerPatternPhaseExpand, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Expand failed.");

    VIR_PatternContext_Finalize(&Context->header, 512);

    return errCode;
}

/* functions used in VIR_Lower_MiddleLevel_To_LowLevel_Preprocess */
typedef struct INDEXED_SWIZZLE
{
    VIR_Indexed indexed;
    VIR_Swizzle swizzle;
} IndexedSwizzle;

IndexedSwizzle relAddr2Swizzle[] =
{
    {VIR_INDEXED_NONE, VIR_SWIZZLE_INVALID}, /* NOT_INDEXED, 0 */
    {VIR_INDEXED_X, VIR_SWIZZLE_XXXX}, /* INDEXED_X, 1 */
    {VIR_INDEXED_Y, VIR_SWIZZLE_YYYY}, /* INDEXED_Y, 2 */
    {VIR_INDEXED_Z, VIR_SWIZZLE_ZZZZ}, /* INDEXED_Z, 3 */
    {VIR_INDEXED_W, VIR_SWIZZLE_WWWW}         /* INDEXED_W, 4 */
};

static VSC_ErrCode
_InsertSTARR(
    IN      VIR_Shader       *Shader,
    IN      VIR_Function     *Func,
    IN      VIR_Instruction  *Inst,
    IN      gctBOOL          needAttrSt,
    IN OUT  VIR_Operand      *Opnd
)
{
    VSC_ErrCode      virErrCode = VSC_ERR_NONE;

    VIR_Instruction *stArrInst;

    VIR_VirRegId     regId      = VIR_Shader_NewVirRegId(Shader, 1);
    VIR_SymId        symId;

    VIR_SymId        relSymId   = VIR_Operand_GetRelIndexing(Opnd);
    VIR_Indexed      relIndexed = VIR_Operand_GetRelAddrMode(Opnd);

    VIR_Symbol      *sym        = VIR_Operand_GetSymbol(Opnd);
    VIR_Type        *type       = VIR_Symbol_GetType(sym);
    VIR_TypeId       typeId     = VIR_Type_GetBaseTypeId(type);
    VIR_OperandId    opndId;
    VIR_Precision    precision;

    if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
    {
        /* set the sym to corresponding variable */
        sym = VIR_Symbol_GetVregVariable(sym);
    }

    virErrCode = VIR_Shader_AddSymbol(
        Shader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UNKNOWN),
        VIR_STORAGE_UNKNOWN,
        &symId);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    precision = VIR_Inst_GetExpectedResultPrecision(Inst);
    if(precision == VIR_PRECISION_DEFAULT)
    {
        gcmASSERT(VIR_Inst_GetDest(Inst));
        precision = VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst));
        gcmASSERT(precision != VIR_PRECISION_DEFAULT || !VIR_Shader_IsFS(Shader));
    }
    VIR_Symbol_SetPrecision(VIR_Shader_GetSymFromId(Shader, symId), precision);

    /* need to add VIR_OP_STARR instruction after current one */
    /* indexed array store: STARR dest, src0, src1 ==> dest[src0] = src1 */
    /* ATTR_ST  Output, InvocationIndex, offset, Value */
    virErrCode = VIR_Function_AddInstructionAfter(
        Func,
        needAttrSt ? VIR_OP_ATTR_ST: VIR_OP_STARR,
        typeId,
        Inst,
        &stArrInst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    /* set value */
    VIR_Operand_SetTempRegister(
        needAttrSt ? stArrInst->src[2] : stArrInst->src[1],
        Func,
        symId,
        typeId);
    VIR_Operand_SetSwizzle(needAttrSt ? stArrInst->src[2] : stArrInst->src[1],
        VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd)));
    VIR_Operand_SetPrecision(needAttrSt ? stArrInst->src[2] : stArrInst->src[1],
         VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));

    /* per patch output always index InvocationIndex 0 */
    if (VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
    {
        gctINT immIdxNo = VIR_Operand_GetMatrixConstIndex(Opnd);

        if (VIR_Operand_GetIsConstIndexing(Opnd))
        {
            immIdxNo += VIR_Operand_GetConstIndexingImmed(Opnd);
        }

        opndId = VIR_Operand_GetIndex(stArrInst->dest);
        *stArrInst->dest = *Opnd;
        VIR_Operand_SetIndex(stArrInst->dest, opndId);
        VIR_Operand_SetMatrixConstIndex(stArrInst->dest, 0);
        VIR_Operand_SetRelIndexingImmed(stArrInst->dest, 0);
        VIR_Operand_SetRelAddrMode(stArrInst->dest, VIR_INDEXED_NONE);
        VIR_Operand_SetIsConstIndexing(stArrInst->dest, 0);

        /* src[0] for ST_ARR and ArrayedPerVertex ATTR_ST, src[1] for PerPatch ATTR_ST:
         * create a new Immediate Value*/
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? stArrInst->src[1] : stArrInst->src[0],
            immIdxNo);
    }
    else
    {
        opndId = VIR_Operand_GetIndex(stArrInst->dest);
        *stArrInst->dest = *Opnd;
        VIR_Operand_SetIndex(stArrInst->dest, opndId);
        VIR_Operand_SetMatrixConstIndex(stArrInst->dest, VIR_Operand_GetMatrixConstIndex(Opnd));
        VIR_Operand_SetRelIndexingImmed(stArrInst->dest, 0);
        VIR_Operand_SetRelAddrMode(stArrInst->dest, VIR_INDEXED_NONE);

        /* index */
        VIR_Operand_SetTempRegister(
            VIR_Symbol_isPerPatch(sym) ? stArrInst->src[1] : stArrInst->src[0],
            Func,
            relSymId,
            VIR_Type_GetIndex(VIR_Shader_GetSymFromId(Shader, relSymId)->type));
        VIR_Operand_SetSwizzle(VIR_Symbol_isPerPatch(sym) ? stArrInst->src[1] : stArrInst->src[0],
            relAddr2Swizzle[relIndexed].swizzle);
        VIR_Operand_SetPrecision(VIR_Symbol_isPerPatch(sym) ? stArrInst->src[1] : stArrInst->src[0],
            VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, relSymId)));
    }

    if (sym &&
        (isSymArrayedPerVertex(sym) ||
         VIR_Symbol_isPerPatch(sym) ||
         (VIR_Symbol_isOutput(sym) && Shader->shaderKind == VIR_SHADER_GEOMETRY)))
    {
        /* add immediate 0 to src[1] for ArrayedPerVertex ATTR_ST, src[0] for PerPatch ATTR_ST */
        gcmASSERT(needAttrSt);
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? stArrInst->src[0] : stArrInst->src[1],
            0);
    }

    /* fix original Operand */
    VIR_Operand_SetTempRegister(Opnd, Func, symId, typeId);
    VIR_Operand_SetEnable(Opnd, VIR_Operand_GetEnable(Opnd));
    VIR_Operand_SetPrecision(Opnd, VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));
    VIR_Operand_SetMatrixConstIndex(Opnd, 0);
    VIR_Operand_SetRelAddrMode(Opnd, VIR_INDEXED_NONE);
    VIR_Operand_SetRelIndexingImmed(Opnd, 0);

    return virErrCode;
}

static VSC_ErrCode
_InsertLDARR(
    IN      VIR_Shader       *Shader,
    IN      VIR_Function     *Func,
    IN      VIR_Instruction  *Inst,
    IN      gctBOOL          needAttrLd,
    IN OUT  VIR_Operand      *Opnd
    )
{
    VSC_ErrCode      virErrCode = VSC_ERR_NONE;
    VIR_Instruction *ldArrInst  = gcvNULL;
    VIR_Enable       ldArrEnable= VIR_ENABLE_NONE;
    VIR_Symbol      *sym        = VIR_Operand_GetSymbol(Opnd);

    VIR_VirRegId     regId      = VIR_Shader_NewVirRegId(Shader, 1);
    VIR_SymId        symId;

    VIR_SymId        relSymId   = VIR_Operand_GetRelIndexing(Opnd);
    VIR_Indexed      relIndexed = VIR_Operand_GetRelAddrMode(Opnd);

    VIR_Type        *symType    = VIR_Symbol_GetType(sym);
    VIR_TypeId       typeId     = VIR_Type_GetBaseTypeId(symType);
    VIR_TypeId       newVirRegTypeId;
    VIR_OperandId    opndId;

    VIR_Swizzle     src0Swizzle = VIR_SWIZZLE_INVALID;

    if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
    {
        sym = VIR_Symbol_GetVregVariable(sym);
        if (sym)
        {
            symType = VIR_Symbol_GetType(sym);
            typeId     = VIR_Type_GetBaseTypeId(symType);
        }
    }

    if ((sym &&
        (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(Shader))) ||
        VIR_Inst_GetOpcode(Inst) == VIR_OP_STORE)
    {
        typeId = VIR_Operand_GetType(Opnd);
        symType = VIR_Shader_GetTypeFromId(Shader, typeId);
    }

    VIR_Shader_AddSymbol(
        Shader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(Shader, VIR_TYPE_UNKNOWN),
        VIR_STORAGE_UNKNOWN,
        &symId);
    VIR_Symbol_SetPrecision(VIR_Function_GetSymFromId(Func, symId), VIR_Operand_GetPrecision(Opnd));

    if (VIR_GetTypeRows(typeId) > 1)
    {
        newVirRegTypeId = VIR_TypeId_ComposeNonOpaqueArrayedType(Shader,
                                                                 VIR_GetTypeComponentType(typeId),
                                                                 VIR_GetTypeComponents(typeId),
                                                                 1,
                                                                 -1);
    }
    else
    {
        newVirRegTypeId = typeId;
    }

    /* need to add VIR_OP_LDARR/VIR_OP_ATTR_LD instruction before current one */
    /* LDARR    dest, src0, src1 ==> dest = src0[src1] */
    /* ATTR_LD  dest, Attribute, InvocationIndex, offset */
    virErrCode = VIR_Function_AddInstructionBefore(
        Func,
        needAttrLd? VIR_OP_ATTR_LD: VIR_OP_LDARR,
        newVirRegTypeId,
        Inst,
        &ldArrInst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;

    /*  the enable/swizzle setting:
        tmp1[i].yz ==>
        ldarr tmp3.yz, tmp1.yyzz, i (newInst)
        add tmp4, tmp3.yz, tmp2.xy
        attr_ld is the same. Generating the enable/swizzle in this way
        may save one extra mov in the later RA phase.
    */
    ldArrEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd));
    src0Swizzle = VIR_Enable_2_Swizzle_WShift(ldArrEnable);

    /* dest */
    VIR_Symbol_SetType(VIR_Function_GetSymFromId(Func, symId), VIR_Shader_GetTypeFromId(Shader, newVirRegTypeId));
    VIR_Operand_SetTempRegister(
        ldArrInst->dest,
        Func,
        symId,
        newVirRegTypeId);
    VIR_Operand_SetEnable(ldArrInst->dest, ldArrEnable);
    VIR_Operand_SetPrecision(ldArrInst->dest, VIR_Operand_GetPrecision(Opnd));

    if (VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
    {
        gctINT immIdxNo = VIR_Operand_GetMatrixConstIndex(Opnd);
        gctINT relIdxNo = 0;
        gctINT idxNo = 0;
        gctBOOL isMatrix = gcvFALSE;

        if (VIR_Type_isPrimitive(symType))
        {
            isMatrix = VIR_GetTypeRows(VIR_Type_GetIndex(symType)) > 1;
        }
        else
        {
            isMatrix = VIR_GetTypeRows(VIR_Type_GetBaseTypeId(symType)) > 1;
        }

        if (VIR_Operand_GetIsConstIndexing(Opnd))
        {
            relIdxNo = VIR_Operand_GetConstIndexingImmed(Opnd);
        }

        /* If this Opnd is a matrix, then vector index is saved by matrix constant index. */
        if (isMatrix)
        {
            idxNo = relIdxNo;
        }
        else
        {
            idxNo = relIdxNo + immIdxNo;
        }

        /* src[0], cannot have constant indexing here expect for matrix indexing. */
        opndId = VIR_Operand_GetIndex(ldArrInst->src[0]);
        *ldArrInst->src[0] = *Opnd;
        VIR_Operand_SetSwizzle(ldArrInst->src[0], src0Swizzle);
        VIR_Operand_SetIndex(ldArrInst->src[0], opndId);
        if (isMatrix)
        {
            VIR_Operand_SetMatrixConstIndex(ldArrInst->src[0], immIdxNo);
        }
        else
        {
            VIR_Operand_SetMatrixConstIndex(ldArrInst->src[0], 0);
        }
        VIR_Operand_SetRelIndexingImmed(ldArrInst->src[0], 0);
        VIR_Operand_SetRelAddrMode(ldArrInst->src[0], VIR_INDEXED_NONE);
        VIR_Operand_SetIsConstIndexing(ldArrInst->src[0], 0);

        /* src[1], create a new Immediate Value*/
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? ldArrInst->src[2] : ldArrInst->src[1],
            idxNo);

        /* we check all the const input vertex access, if the const
           index larger than the assumed tcsPatchInputVertices, we change
           tcsPatchInputVertices */
        if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL &&
            !VIR_Shader_TCS_UseDriverInput(Shader))
        {
            if (sym &&
                VIR_Symbol_isInput(sym) &&
                (idxNo + 1 > Shader->shaderLayout.tcs.tcsPatchInputVertices))
            {
                Shader->shaderLayout.tcs.tcsPatchInputVertices = idxNo + 1;
            }
        }
    }
    else
    {
        /* src[0] */
        opndId = VIR_Operand_GetIndex(ldArrInst->src[0]);
        *ldArrInst->src[0] = *Opnd;
        VIR_Operand_SetSwizzle(ldArrInst->src[0], src0Swizzle);
        VIR_Operand_SetIndex(ldArrInst->src[0], opndId);
        VIR_Operand_SetMatrixConstIndex(ldArrInst->src[0], VIR_Operand_GetMatrixConstIndex(Opnd));
        VIR_Operand_SetRelIndexingImmed(ldArrInst->src[0], 0);
        VIR_Operand_SetRelAddrMode(ldArrInst->src[0], VIR_INDEXED_NONE);

        /* src[1] */
        VIR_Operand_SetTempRegister(
            VIR_Symbol_isPerPatch(sym) ? ldArrInst->src[2] : ldArrInst->src[1],
            Func,
            relSymId,
            VIR_Type_GetIndex(VIR_Shader_GetSymFromId(Shader, relSymId)->type));
        VIR_Operand_SetSwizzle(VIR_Symbol_isPerPatch(sym) ? ldArrInst->src[2] : ldArrInst->src[1], relAddr2Swizzle[relIndexed].swizzle);
        VIR_Operand_SetPrecision(VIR_Symbol_isPerPatch(sym) ? ldArrInst->src[2] : ldArrInst->src[1], VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, relSymId)));
    }
    if (sym &&
        (isSymArrayedPerVertex(sym) || VIR_Symbol_isPerPatch(sym)))
    {
        /* add immediate 0 to src[2] for ArrayedPerVertex ATTR_ST, src[1] for PerPatch ATTR_ST */
        gcmASSERT(needAttrLd);
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? ldArrInst->src[1] : ldArrInst->src[2],
            0);
    }
    VIR_Operand_SetTempRegister(Opnd, Func, symId, newVirRegTypeId);
    VIR_Operand_SetRelAddrMode(Opnd, VIR_INDEXED_NONE);
    VIR_Operand_SetRelIndexingImmed(Opnd, 0);
    VIR_Operand_SetMatrixConstIndex(Opnd, 0);

    if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
    {
        if (sym &&
            isSymArrayedPerVertex(sym) &&
            VIR_Symbol_isOutput(sym))
        {
            Shader->shaderLayout.tcs.hasOutputVertexAccess = gcvTRUE;
        }
    }
    return virErrCode;
}

VSC_ErrCode
VIR_Lower_MiddleLevel_To_LowLevel_Preprocess(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;

    /* insert LDARR/STARR for indexed access and
       insert ATTR_LD/ATTR_ST for memory access (per-patch, per-vertex ...) */
    {
        /* insert STARR/LDARR for indexed operands */
        VIR_FuncIterator    func_iter;
        VIR_FunctionNode   *func_node;

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(Shader));

        for (func_node = VIR_FuncIterator_First(&func_iter);
            func_node != gcvNULL;
            func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function    *func = func_node->function;
            VIR_Instruction *inst = func->instList.pHead;

            for(; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
            {
                VIR_Operand     *src   = gcvNULL;

                /* set restart flag for the shader,
                   opt:
                   1) if the output is point, we don't need to set the flag */
                if (!VIR_Shader_GS_HasRestartOp(Shader))
                {
                    if ((VIR_Inst_GetOpcode(inst) == VIR_OP_RESTART) &&
                        (Shader->shaderLayout.geo.geoOutPrimitive != VIR_GEO_POINTS))
                    {
                        VIR_Shader_SetFlag(Shader, VIR_SHFLAG_GS_HAS_RESTART_OP);
                    }
                }

                if ((VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_LD ||
                    VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_ST) &&
                    VIR_Operand_isUndef(VIR_Inst_GetSource(inst, 2)))
                {
                    gctINT  indexConstValue;   /* constIndexing + matrixIndexing */
                    VIR_Symbol *sym;

                    if ((VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_LD))
                    {
                        src = VIR_Inst_GetSource(inst, 0);
                    }
                    else
                    {
                        /* normalize enable to start with x, calculate the shift */
                        src = VIR_Inst_GetDest(inst);
                    }

                    /* Get the variable symbol. */
                    sym = VIR_Operand_GetSymbol(src);
                    if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
                    {
                        sym = VIR_Symbol_GetVregVariable(sym);
                    }

                    indexConstValue = VIR_Operand_GetMatrixConstIndex(src);

                    if (VIR_Operand_GetRelAddrMode(src) == VIR_INDEXED_NONE)
                    {
                        if (VIR_Operand_GetIsConstIndexing(src))
                        {
                            indexConstValue += VIR_Operand_GetConstIndexingImmed (src);
                        }
                        /* inst->src[2], create a new Immediate Value*/
                        VIR_Operand_SetImmediateInt(inst->src[2], indexConstValue);
                    }
                    /* FE won't generate indexed for matrix, if this src has indexed, it must be an error*/
                    else
                    {
                        VIR_SymId symId = VIR_Operand_GetRelIndexing(src);

                        gcmASSERT(!VIR_Operand_GetIsConstIndexing(src) );

                        VIR_Operand_SetTempRegister(inst->src[2], func, symId, VIR_TYPE_INT32);
                        VIR_Operand_SetPrecision(inst->src[2], VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));
                    }

                    /* reset src indexing info */
                    VIR_Operand_SetMatrixConstIndex(src, 0);
                    VIR_Operand_SetRelIndexingImmed(src, 0);
                    VIR_Operand_SetRelAddrMode(src, VIR_INDEXED_NONE);
                    VIR_Operand_SetIsConstIndexing(src, 0);
                    if (VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_LD)
                    {
                        /* set the tcsPatchInputVertices for const indexing */
                        if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL &&
                            !VIR_Shader_TCS_UseDriverInput(Shader))
                        {
                            src = VIR_Inst_GetSource(inst, 0);
                            if (VIR_Operand_GetOpKind(src) == VIR_OPND_IMMEDIATE)
                            {
                                if (VIR_Operand_GetImmediateInt(src) + 1 > Shader->shaderLayout.tcs.tcsPatchInputVertices)
                                {
                                    Shader->shaderLayout.tcs.tcsPatchInputVertices = VIR_Operand_GetImmediateInt(src) + 1;
                                }
                            }
                        }

                        /* When loading data from a per-patch variable, the index is saved in src2.*/
                        if (VIR_Symbol_isPerPatch(sym))
                        {
                            src = inst->src[1];
                            inst->src[1] = inst->src[2];
                            inst->src[2] = src;
                        }

                        if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
                        {
                            if (isSymArrayedPerVertex(sym) && VIR_Symbol_isOutput(sym))
                            {
                                Shader->shaderLayout.tcs.hasOutputVertexAccess = gcvTRUE;
                            }
                        }
                    }
                    else
                    {
                        /* Reorder src to match expected seq like 'Output, InvocationIndex, offset, Value' */
                        src = inst->src[0];
                        if (VIR_Symbol_isPerPatch(sym))
                        {
                            inst->src[1] = inst->src[1];
                            inst->src[0] = inst->src[2];
                        }
                        else
                        {
                            inst->src[0] = inst->src[1];
                            inst->src[1] = inst->src[2];
                        }
                        inst->src[2] = src;
                    }
                }
                /* expand access to arrayed pervertex input/output to LDARR */
                do
                {
                    VIR_SrcOperand_Iterator opndIter;
                    gctBOOL          isReg  = gcvFALSE;
                    VIR_Operand     *dest   = gcvNULL;
                    gctBOOL     needAttrLd, needAttrSt;
                    VIR_Symbol * sym;

                    VIR_SrcOperand_Iterator_Init(inst, &opndIter);
                    src = VIR_SrcOperand_Iterator_First(&opndIter);

                    /* check source operands for indexed operand */
                    for (; src != gcvNULL; src = VIR_SrcOperand_Iterator_Next(&opndIter))
                    {
                        gctBOOL isReg = gcvFALSE;

                        if (VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_LD &&
                            src == VIR_Inst_GetSource(inst, 0))
                        {
                            /* donot check src[0] for VIR_OP_ATTR_LD */
                            continue;
                        }

                        needAttrLd = gcvFALSE;
                        sym = gcvNULL;
                        switch(VIR_Operand_GetOpKind(src))
                        {
                        case VIR_OPND_SAMPLER_INDEXING:
                        case VIR_OPND_TEXLDPARM:
                            isReg = gcvTRUE;
                            break;
                        case VIR_OPND_SYMBOL:
                        case VIR_OPND_VIRREG:
                            sym = VIR_Operand_GetSymbol(src);
                            if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
                            {
                                /* set the sym to corresponding variable */
                                sym = VIR_Symbol_GetVregVariable(sym);
                            }
                            if (sym &&
                                (isSymArrayedPerVertex(sym) ||
                                 VIR_Symbol_isPerPatch(sym)) &&
                                 VIR_Inst_GetOpcode(inst) != VIR_OP_LDARR)
                            {
                                needAttrLd = gcvTRUE;
                            }
                            isReg = gcvTRUE;
                            break;
                        default:
                            isReg = gcvFALSE;
                            break;
                        }

                        if(!isReg && !needAttrLd)
                        {
                            continue;
                        }

                        if(VIR_Operand_GetRelAddrMode(src) == VIR_INDEXED_NONE && !needAttrLd)
                        {
                            continue;
                        }

                        _InsertLDARR(Shader, func, inst, needAttrLd, src);
                    }

                    /* do not check dest for ATTR_ST */
                    if (VIR_Inst_GetOpcode(inst) == VIR_OP_ATTR_ST)
                        break;

                    /* check dest operands for indexed operand */
                    dest = VIR_Inst_GetDest(inst);

                    if(dest == gcvNULL)
                    {
                        continue;
                    }

                    needAttrSt = gcvFALSE;
                    sym = gcvNULL;
                    switch(VIR_Operand_GetOpKind(dest))
                    {
                    case VIR_OPND_SYMBOL:
                    case VIR_OPND_VIRREG:
                        sym = VIR_Operand_GetSymbol(dest);
                        if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
                        {
                            /* set the sym to corresponding variable */
                            sym = VIR_Symbol_GetVregVariable(sym);
                        }
                        if (sym &&
                            (isSymArrayedPerVertex(sym) ||
                             VIR_Symbol_isPerPatch(sym)) &&
                             VIR_Inst_GetOpcode(inst) != VIR_OP_STARR)
                        {
                            needAttrSt = gcvTRUE;
                        }
                        isReg = gcvTRUE;
                        break;
                    case VIR_OPND_SAMPLER_INDEXING:
                    case VIR_OPND_TEXLDPARM:
                        isReg = gcvTRUE;
                        break;
                    default:
                        isReg = gcvFALSE;
                        break;
                    }

                    if(!isReg && !needAttrSt)
                    {
                        continue;
                    }

                    if(VIR_Operand_GetRelAddrMode(dest) == VIR_INDEXED_NONE && !needAttrSt)
                    {
                        continue;
                    }

                    _InsertSTARR(Shader, func, inst, needAttrSt, dest);
                } while (0);
            }
        }
    }

    return errCode;
}

