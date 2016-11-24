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

gctBOOL
_checkToSetFullDefFlag(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *MaskValueOperand
    );


static gctBOOL
_destShort_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_revise2UnPackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

gctBOOL
_change2NonpackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_src0ScalarOrPackedLE16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_destOrSrc0PackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_equatePackedTypeForDestOrSrc(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setConvPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setConvPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );
static void
_changeEnableByTyId(
    IN VIR_TypeId          TyId,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_src0PackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_destPackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_revise2PackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn1PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn1PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setUnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn1UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setUnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn1UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

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
    return (Context->vscContext->appNameId == gcvPATCH_DEQP);
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_FLOAT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
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
    VIR_Operand           *src = VIR_Inst_GetSource(Inst, SrcIndex);

    gcmASSERT(SrcIndex < VIR_MAX_SRC_NUM);
    if(src == gcvNULL)
        return gcvFALSE;
    if(VIR_Operand_GetOpKind(src) != VIR_OPND_IMMEDIATE)
        return gcvFALSE;
    if(VIR_Operand_GetImmediateUint(src) != 0)
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
    VIR_Operand           *src = VIR_Inst_GetSource(Inst, SrcIndex);

    gcmASSERT(SrcIndex < VIR_MAX_SRC_NUM);
    if(src == gcvNULL)
        return gcvFALSE;
    if(VIR_Operand_GetOpKind(src) != VIR_OPND_IMMEDIATE)
        return gcvFALSE;
    if(VIR_Operand_GetImmediateFloat(src) != 1.0)
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
        gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

        return VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst));
    }

    return gcvFALSE;
}

static gctBOOL
_isCLShader(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Shader_IsCL(Context->shader);
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
    VIR_Swizzle swizzle0 = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0));
    VIR_Swizzle swizzle1 = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 1));

    swizzle0 = (((swizzle0 >> 4) & 3) << 0)
             | (((swizzle0 >> 0) & 3) << 2)
             | (((swizzle0 >> 2) & 3) << 4)
             | (((swizzle0 >> 2) & 3) << 6);
    swizzle1 = (((swizzle1 >> 2) & 3) << 0)
             | (((swizzle1 >> 4) & 3) << 2)
             | (((swizzle1 >> 0) & 3) << 4)
             | (((swizzle1 >> 0) & 3) << 6);

    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(Inst, 0), swizzle0);
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(Inst, 1), swizzle1);

    return gcvTRUE;
}

static gctBOOL
_hasB0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasUniformB0);
}

static gctBOOL
_hasB0_VG(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _hasB0(Context, Inst) && (Context->shader->compilerVersion[0] & 0xffff) == _SHADER_VG_TYPE;
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
            (VIR_Operand_IsPerPatch(VIR_Inst_GetSource(Inst, 0)) ||
             VIR_Operand_IsArrayedPerVertex(VIR_Inst_GetSource(Inst, 0))));
}

static gctBOOL
_isRAEnabled_dest_prim_ctp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            (VIR_Operand_IsPerPatch(VIR_Inst_GetDest(Inst)) ||
             VIR_Operand_IsArrayedPerVertex(VIR_Inst_GetDest(Inst))));
}

static gctBOOL
_isRAEnabled_src0_uniform_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(Inst, 0))) &&
            (VIR_GetTypeFlag(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_isRAEnabled_src0_uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(Inst, 0))));
}

static gctBOOL
_isRAEnabled_src1_uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(VIR_Inst_GetSource(Inst, 1))));
}

/* sampler index should always be integer */
static gctBOOL
_isRAEnabled_src0_not_sampler_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
           !VIR_TypeId_isSampler(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0))) &&
           (VIR_GetTypeFlag(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_isRAEnabled_dest_not_sampler_src0_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
        !VIR_TypeId_isSampler(VIR_Operand_GetType(VIR_Inst_GetDest(Inst))) &&
        (VIR_GetTypeFlag(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0))) & VIR_TYFLAG_ISFLOAT));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_16(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;

    imm0.iValue = 16;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetType(dest, VIR_TYPE_INT32);
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand_SetType(dest, VIR_TYPE_INT32);
    VIR_Operand_SetEnable(dest,
        VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));
    gcmASSERT(VIR_Symbol_isVreg(VIR_Operand_GetSymbol(dest)));
    VIR_Symbol_SetStorageClass(VIR_Operand_GetSymbol(dest), VIR_STORAGE_INDEX_REGISTER);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_INT32));
    return gcvTRUE;
}

static gctBOOL
_setEnableInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand_SetType(dest, VIR_TYPE_INT32);
    VIR_Operand_SetEnable(dest,
        VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));
    gcmASSERT(VIR_Symbol_isVreg(VIR_Operand_GetSymbol(dest)));
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_INT32));
    return gcvTRUE;
}

static gctBOOL
_setMOVAEnableFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand_SetType(dest, VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(dest,
       VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));
    gcmASSERT(VIR_Symbol_isVreg(VIR_Operand_GetSymbol(dest)));
    VIR_Symbol_SetStorageClass(VIR_Operand_GetSymbol(dest), VIR_STORAGE_INDEX_REGISTER);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_FLOAT32));
    return gcvTRUE;
}

static gctBOOL
_setMOVAEnableIntUniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setMOVAEnableInt(Context, Inst, Opnd);
    VIR_Operand_SetFlag(Opnd, VIR_OPNDFLAG_UNIFORM_INDEX);

    return gcvTRUE;
}

static gctBOOL
_setMOVAEnableFloatUniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setMOVAEnableFloat(Context, Inst, Opnd);
    VIR_Operand_SetFlag(Opnd, VIR_OPNDFLAG_UNIFORM_INDEX);

    return gcvTRUE;
}

static gctBOOL
_setOperandUniformIndex(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetFlag(Opnd, VIR_OPNDFLAG_UNIFORM_INDEX);

    return gcvTRUE;
}

static gctBOOL
_resetOperandUniformIndex(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_ResetFlag(Opnd, VIR_OPNDFLAG_UNIFORM_INDEX);

    return gcvTRUE;
}

static gctBOOL
_setLDARRSwizzleInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(VIR_Inst_GetSource(Inst, 1), VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setLDARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(VIR_Inst_GetSource(Inst, 1), VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(VIR_Inst_GetSource(Inst, 0), VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(VIR_Inst_GetSource(Inst, 0), VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
destEnableW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand_SetEnable(dest, VIR_ENABLE_W);
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
        VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));
        VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
    VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));
    VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
    VIR_PrimitiveTypeId baseTy0 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));
    VIR_PrimitiveTypeId baseTy1 = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(Inst != gcvNULL &&
              dest != gcvNULL);

    if (VIR_Operand_GetModifier(dest) != VIR_MOD_NONE &&
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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);

    if(VIR_Operand_GetRoundMode(dest) == VIR_ROUND_RTP &&
        VIR_Operand_GetModifier(dest) != VIR_MOD_NONE &&
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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);

    if(VIR_Operand_GetModifier(dest) != VIR_MOD_NONE &&
        VIR_Operand_GetRoundMode(dest) == VIR_ROUND_RTN &&
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
    if(VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst)) == VIR_ROUND_RTP &&
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
    if(VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst)) == VIR_ROUND_RTN &&
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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);

    if (!VIR_Operand_GetModifier(dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);

    if (!VIR_Operand_GetModifier(dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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
    VIR_Operand           *dest = VIR_Inst_GetDest(Inst);

    if (!VIR_Operand_GetModifier(dest))
    {
        return gcvFALSE;
    }

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT16 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT16)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT32 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT32)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

        if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT32 ||
            VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT32)
        {
            VIR_PrimitiveTypeId srcType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

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
        VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

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

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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
float32_sign(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x80000000u;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
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

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
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

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
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
    VIR_Lower_SetOpndUINT16(Context, Inst, VIR_Inst_GetDest(Inst));
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

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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

    format  = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
    format0 = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));

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

    VIR_Operand_SetType(VIR_Inst_GetDest(Inst), VIR_TypeId_ComposeNonOpaqueType(format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst))), 1));

    return gcvTRUE;
}

static gctBOOL
value_type0_32bit_pattern_dest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst;
    VIR_Operand *dest;
    VIR_PrimitiveTypeId  format;
    gctUINT components;

    patternInst = VIR_Inst_GetNext(Inst);
    dest = VIR_Inst_GetDest(patternInst);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));

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

    if(VIR_TypeId_isPacked(VIR_Operand_GetType(dest)))
    {
        components = VIR_GetTypePackedComponents(VIR_Lower_GetBaseType(Context->shader, dest));
    }
    else
    {
        components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, dest));
    }
    VIR_Operand_SetType(VIR_Inst_GetDest(Inst),
                        VIR_TypeId_ComposeNonOpaqueType(format, components, 1));

    return gcvTRUE;
}

static gctBOOL
value_type0_from_src0_unpacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId  format;
    VIR_Operand *src0;
    VIR_Operand *dest;
    VIR_TypeId tyId;
    gctUINT components;

    src0 = VIR_Inst_GetSource(Inst, 0);
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetType(src0)));
    dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(dest);

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));

    components = VIR_GetTypeSize(VIR_Operand_GetType(dest)) / VIR_GetTypeSize(format);
    gcmASSERT(components <= 16);
    VIR_Operand_SetType(dest, VIR_TypeId_ComposePackedNonOpaqueType(format, components));
    tyId = VIR_Operand_GetType(dest);
    _changeEnableByTyId(tyId, dest);

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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));

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

    VIR_Operand_SetType(VIR_Inst_GetSource(Inst, 0), VIR_TypeId_ComposeNonOpaqueType(format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))), 1));

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
revise_operand_type_by_dest_type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL);

    VIR_Operand_SetType(Opnd, VIR_Operand_GetType(VIR_Inst_GetDest(Inst)));

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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
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

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));
    if (format == VIR_TYPE_FLOAT32)
    {
        value.fValue = (gctFLOAT) value.uValue;
    }
    else
    {
        _value_type0_32bit_from_src0(Context, Inst, Opnd);
    }

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));
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

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0)));
    if (format == VIR_TYPE_FLOAT32)
    {
        value.fValue = (gctFLOAT) value.iValue;
    }
    else
    {
        _value_type0_32bit_from_src0(Context, Inst, Opnd);
    }

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        format == VIR_TYPE_FLOAT32 ? VIR_TYPE_FLOAT32 : VIR_TYPE_INT32,
        value);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);
    return gcvTRUE;
}

static gctBOOL
int_value_type0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);

    VIR_Operand_SetType(dest, VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
int_value_type0_src_const_0xFFFFFF(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);

    imm0.iValue = 0xFFFFFF;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(Opnd))) {
        VIR_Operand_SetType(Opnd, VIR_TYPE_INT32);
        VIR_Operand_SetType(dest, VIR_TYPE_INT32);
    }
    else {
        VIR_Operand_SetType(Opnd, VIR_TYPE_UINT32);
        VIR_Operand_SetType(dest, VIR_TYPE_UINT32);
    }
    return gcvTRUE;
}

static gctBOOL
int_value_type0_src_const_0xFF000000(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);

    imm0.iValue = 0xFF000000;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(Opnd))) {
        VIR_Operand_SetType(Opnd, VIR_TYPE_INT32);
        VIR_Operand_SetType(dest, VIR_TYPE_INT32);
    }
    else {
        VIR_Operand_SetType(Opnd, VIR_TYPE_UINT32);
        VIR_Operand_SetType(dest, VIR_TYPE_UINT32);
    }
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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));

    VIR_ScalarConstVal imm0;

    if (format == VIR_TYPE_INT8)
    {
        imm0.uValue = 24;
    }
    else
    {
        imm0.uValue = 16;
    }

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);


    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));

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
        VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst)));

    VIR_ScalarConstVal imm0;

    if (format == VIR_TYPE_INT8)
    {
        imm0.uValue = 0x000000FF;
    }
    else
    {
        imm0.uValue = 0x0000FFFF;
    }

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndINT16(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndINT32(Context, Inst, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}

/* If dest data type rank higher than the src, set src to constant zero
   other nop the instruction */
static gctBOOL
_destTypeRankHigher_setSrcToZero_elseNop(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId format, srcFormat;
    gctBOOL rankHigher = gcvFALSE;

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    srcFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
    case VIR_TYPE_BOOLEAN:
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_FLOAT16:
        if(srcFormat == VIR_TYPE_INT8 &&
           srcFormat == VIR_TYPE_UINT8 &&
           srcFormat == VIR_TYPE_BOOLEAN) {
            rankHigher = gcvTRUE;
        }
        break;

    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
    case VIR_TYPE_FLOAT32:
        if(srcFormat != VIR_TYPE_INT32 ||
           srcFormat != VIR_TYPE_UINT32 ||
           srcFormat != VIR_TYPE_FLOAT32) {
            rankHigher = gcvTRUE;
        }
        break;

    default:
       break;
    }

    if(rankHigher) {
        VIR_ScalarConstVal imm0;

        imm0.uValue = 0;

        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
    }
    else {
        VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
    }
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);
    VIR_Lower_SetOpndUINT16(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));

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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndUINT16(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);
    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);
    VIR_Lower_SetOpndUINT16(Context, Inst, VIR_Inst_GetDest(Inst));
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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
        VIR_TYPE_UINT32,
        imm1);
    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));

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

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_UINT32,
        imm0);

    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));
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

    imm0.uValue = 0x70000000u;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 2),
        VIR_TYPE_UINT32,
        imm0);
    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}

static gctBOOL
float32_exp_underflow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 112 << 23;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
float32_exp_overflow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 143 << 23;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
float32_exp_isoverflow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.uValue = 0x1f << 23;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
value_types_32(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Lower_SetOpndUINT32(Context, Inst, VIR_Inst_GetDest(Inst));
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
jmp_2_succ2_resCondOp_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId ty0, ty1;
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);

    ty0 = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));
    ty1 = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 1));

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);
    /* Texkill only supports float. */
    return jmp_2_succ(Context, Inst, 2) &&
           VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(Inst)) &&
           (VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
           (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT);
}

static gctBOOL
jmp_2_succ2_resCondOp_singleChannel(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId ty0, ty1;
    VIR_Swizzle swizzle0 = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0));
    VIR_Swizzle swizzle1 = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 1));

    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);

    ty0 = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));
    ty1 = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 1));

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);
    return (VIR_Swizzle_Channel_Count(swizzle0) == 1 || VIR_Operand_isImm(VIR_Inst_GetSource(Inst, 0))) &&
           (VIR_Swizzle_Channel_Count(swizzle1) == 1 || VIR_Operand_isImm(VIR_Inst_GetSource(Inst, 1))) &&
           jmp_2_succ(Context, Inst, 2) &&
           VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(Inst)) &&
           !VIR_TypeId_isFloat(ty0) &&
           !VIR_TypeId_isFloat(ty1);
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

    if(components <= 4)
    {
        nswiz = VIR_Swizzle_GetChannel(swiz, components - 1);
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


    compCount = VIR_Enable_Channel_Count(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0))));

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

static gctBOOL
_isBiasTexModifierAndCubeArrayShadow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId samplerType = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));

    if (!VIR_TypeId_isPrimitive(samplerType))
    {
        samplerType = VIR_Type_GetBaseTypeId(VIR_Shader_GetTypeFromId(Context->shader, samplerType));
    }

    if (samplerType == VIR_TYPE_SAMPLER_CUBE_ARRAY_SHADOW && VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_BIAS)
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
    return VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_BIAS;
}

static gctBOOL
_isLodTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_LOD;
}

static gctBOOL
_isGradTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_GRAD;
}


static gctBOOL
_isGatherTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_GATHER;
}

static gctBOOL
_isGatherAndLodTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TexModifier_Flag flag = VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2));
    return (flag == (VIR_TMFLAG_GATHER | VIR_TMFLAG_LOD));
}

static gctBOOL
_isGatherAndBiasTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TexModifier_Flag flag = VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2));
    return (flag == (VIR_TMFLAG_GATHER | VIR_TMFLAG_BIAS));
}

static gctBOOL
_isFetchMsTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Lower_GetTexModifierKind(VIR_Inst_GetSource(Inst, 2)) == VIR_TMFLAG_FETCHMS;
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

/* texldPcf patterns. */
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

    newType = VIR_Operand_GetType(coord);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, newType));
    VIR_Operand_SetType(dest, newType);
    VIR_Operand_SetEnable(dest, VIR_TypeId_Conv2Enable(newType));

    return gcvTRUE;
}

static gctBOOL
_setDestShort_P4TypeFromSrc(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId typeId;

    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(Opnd))) {
        typeId = VIR_TYPE_INT16_P4;
    }
    else {
        typeId = VIR_TYPE_UINT16_P4;
    }
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, typeId));
    VIR_Operand_SetType(dest, typeId);
    VIR_Operand_SetEnable(dest, VIR_TypeId_Conv2Enable(typeId));

    return gcvTRUE;
}

static gctBOOL
_hasDrefAndOtherTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand           *opnd = VIR_Inst_GetSource(Inst, 2);
    VIR_Operand *texldModifier = (VIR_Operand *)opnd;

    if ((VIR_Lower_GetTexModifierKind(opnd) & VIR_TMFLAG_GATHER)
        &&
        (VIR_Lower_GetTexModifierKind(opnd) != VIR_TMFLAG_GATHER)
        &&
        (VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ) != gcvNULL))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_onlyHasDrefTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand           *opnd = VIR_Inst_GetSource(Inst, 2);
    VIR_Operand *texldModifier = (VIR_Operand *)opnd;

    if ((VIR_Lower_GetTexModifierKind(opnd) == VIR_TMFLAG_GATHER)
        &&
        (VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ) != gcvNULL)
        &&
        (VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERCOMP) == gcvNULL))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_setDrefToLastButOneComponent(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *texldModifier = (VIR_Operand *)Opnd;
    VIR_Operand *dRefOpnd = VIR_Operand_GetTexldModifier(texldModifier,VIR_TEXLDMODIFIER_GATHERREFZ);
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_Symbol  *destSym = VIR_Operand_GetSymbol(dest);
    VIR_TypeId   destSymTypeId = VIR_Symbol_GetTypeId(destSym);
    gctUINT componentCount;

    /* Use Dref as source0. */
    VIR_Inst_SetSource(Inst, 0, dRefOpnd);

    /* Change type and swizzle. */
    componentCount = VIR_GetTypeComponents(destSymTypeId);
    VIR_Operand_SetEnable(dest, (VIR_Enable)(VIR_ENABLE_X << (componentCount - 2)));
    VIR_Operand_SetType(dest, VIR_GetTypeComponentType(destSymTypeId));

    return gcvTRUE;
}

static gctBOOL
_setDrefToLastComponent(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *texldModifier = (VIR_Operand *)Opnd;
    VIR_Operand *dRefOpnd = VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ);
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_Symbol  *destSym = VIR_Operand_GetSymbol(dest);
    VIR_TypeId   destSymTypeId = VIR_Symbol_GetTypeId(destSym);
    gctUINT componentCount;

    /* Use Dref as source0. */
    VIR_Inst_SetSource(Inst, 0, dRefOpnd);

    /* Change type and swizzle. */
    componentCount = VIR_GetTypeComponents(destSymTypeId);
    VIR_Operand_SetEnable(dest, (VIR_Enable)(VIR_ENABLE_X << (componentCount - 1)));
    VIR_Operand_SetType(dest, VIR_GetTypeComponentType(destSymTypeId));

    return gcvTRUE;
}

static gctBOOL _setNewCoordWithDref(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Symbol  *coordSym = VIR_Operand_GetSymbol(Opnd);
    VIR_TypeId   coordSymTypeId = VIR_Symbol_GetTypeId(coordSym);

    VIR_Operand_SetSwizzle(Opnd, VIR_Enable_2_Swizzle_WShift(VIR_TypeId_Conv2Enable(coordSymTypeId)));
    VIR_Operand_SetType(Opnd, coordSymTypeId);

    return gcvTRUE;
}

static gctBOOL
_cleanDrefTexModifier(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *texldModifier = (VIR_Operand *)Opnd;

    VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERREFZ, gcvNULL);
    if (VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_GATHERCOMP) == gcvNULL)
    {
        VIR_Operand_ClrTexModifierFlag(texldModifier, VIR_TMFLAG_GATHER);
    }

    return gcvTRUE;
}

static gctBOOL
_isHWNotSupportUnordBranch(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
)
{
    if (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportUnOrdBranch)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_updateOperandTypeToBool(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Shader *shader = Context->shader;
    VIR_TypeId typeId = VIR_Operand_GetType(Opnd);
    VIR_Type *type = VIR_Shader_GetTypeFromId(shader, typeId);

    typeId = VIR_Type_GetBaseTypeId(type);
    typeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_BOOLEAN, VIR_GetTypeComponents(typeId), 1);

    VIR_Operand_SetType(Opnd, typeId);

    return gcvTRUE;
}

/*
    Artificial NULL pattern bypassing the individual pattern match and replace
 */
static VIR_PatternMatchInst _nullPatInst0[] = {
    { VIR_OP_NOP, VIR_PATTERN_ANYCOND, 0, { 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _nullRepInst0[] = {
    { VIR_OP_NOP, 0, 0, { 0 }, { 0 } },
};

static VIR_Pattern _nullPattern[] = {
    { VIR_PATN_FLAG_ALREADY_MATCHED_AND_REPLACED | VIR_PATN_FLAG_RECURSIVE_SCAN_NEWINST, CODEPATTERN(_null, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
        DIV 1, 2, 3
            idiv 1, 2, 3, 0, 0
    { 1, gcSL_DIV, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x44, 1, 2, 3, 0, 0, value_type0 },
*/

static VIR_PatternMatchInst _divPatInst0[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destShort_P8Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, revise_dest_type_by_operand_type, _setColumn1UnPackedSwizzle, _setColumn1UnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, revise_dest_type_by_operand_type, _setColumn1UnPackedSwizzle, _setColumn1UnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { 0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, revise_operand_type_by_dest_type, _setColumn1PackedSwizzle, _setColumn1PackedMaskValue } },
};

static VIR_PatternMatchInst _divPatInst1[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsIntOpcode, _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst1[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

    /*
        DIV 1, 2, 3
            rcp TEMP1, 0, 0, 3
            mul 1, 2, TEMP1, 0
    */
static VIR_PatternMatchInst _divPatInst2[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_enableFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst2[] = {
    { VIR_OP_RCP, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -1, 0 }, { 0 } },
};

static VIR_Pattern _divPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 1) },
    /* now gcSL CG cannot handle expanded PRE_DIV, enable this if use new VIR CG */
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_div, 2) },
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
    { VIR_OP_ABS, 0, 0, { -4, -2, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -5, -4, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -6, -5,-3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -7, -6, 0,-1 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -7, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_SINPI, -1, 0, {  1, -7, 0, 0 }, { 0 } },
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
    { VIR_OP_MAD, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi2_1_dot5_2 } },
    { VIR_OP_FRAC, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -3, -2, 0, 0 }, { 0, 0, pi2_1_pi_2 } },
    { VIR_OP_MUL, 0, 0, { -4, -3, -3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -5, -4, 0, 0 }, { 0, 0, factor9_1_factor7_2 } },
    { VIR_OP_MAD, 0, 0, { -6, -4, -5, 0 }, { 0, 0, 0, factor5_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -4, -6, 0 }, { 0, 0, 0, factor3_2 } },
    { VIR_OP_MAD, 0, 0, { -8, -4, -7, 0 }, { 0, 0, 0, sin_one_2 } },
    { VIR_OP_MUL, 0, 0, {  1, -8, -3, 0 }, { 0 } },
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
    { VIR_OP_MAD, 0, 0, { -2, 2, 0,-1 }, { 0, 0, rcppi_low } },
    { VIR_OP_MUL, 0, 0, { -3, 2, 0, 0 }, { _set_RTZ_HighPrecision, 0, rcp2pi } },
    { VIR_OP_SIGN, 0, 0, { -4, -3, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -5, -3, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -6, -5, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -7, -6,-4, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -8, -7, 0,-2 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -8, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_COSPI, -1, 0, {  1, -8, 0, 0 }, { 0 } },
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
    { VIR_OP_MAD, 0, 0, { -1, 2, 0, 0 }, { 0, 0, rcppi2_1_dot5_2 } },
    { VIR_OP_FRAC, 0, 0, { -2, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -3, -2, 0, 0 }, { 0, 0, pi2_1_pi_2 } },
    { VIR_OP_MUL, 0, 0, { -4, -3, -3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -5, -4, 0, 0 }, { 0, 0, factor8_1_factor6_2 } },
    { VIR_OP_MAD, 0, 0, { -6, -4, -5, 0 }, { 0, 0, 0, factor4_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -4, -6, 0 }, { 0, 0, 0, factor2_2 } },
    { VIR_OP_MAD, 0, 0, {  1, -4, -7, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
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
    { VIR_OP_MAD, 0, 0, { -2, 2, 0,-1 }, { 0, 0, rcppi_low } },
    { VIR_OP_MUL, 0, 0, { -3, 2, 0, 0 }, { _set_RTZ_HighPrecision, 0, rcp2pi } },
    { VIR_OP_SIGN, 0, 0, { -4, -3, 0, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -5, -3, 0, 0 }, { 0 } },
    { VIR_OP_FRAC, 0, 0, { -6, -5, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -7, -6,-4, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -8, -7, 0,-2 }, { 0, 0, two } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg10 } },
    { VIR_OP_MUL, 0, 0, { -8, 2, 0, 0 }, { 0, 0, rcppi } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg3 } },
    { VIR_OP_SINPI, 0, 0, { -9, -8, 0, 0 }, { 0 } },
    { VIR_OP_COSPI, 0, 0, { -10, -8, 0, 0 }, { 0 } },
    { VIR_OP_DIV, 0, 0, {  1, -9, -10, 0 }, { 0 } },
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
    { VIR_OP_SINPI, 0, 0, { -3, -1, 0, 0 }, { 0 } },
    { VIR_OP_RCP, 0, 0, { -4, -2, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, -3, -4, 0 }, { 0 } },
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
    { VIR_OP_MAD, 0, 0, { -3, -2, 0, 0 }, { 0, 0, tan9_1_tan7_2 } },
    { VIR_OP_MAD, 0, 0, { -4, -2, -3, 0 }, { 0, 0, 0, tan5_2 } },
    { VIR_OP_MAD, 0, 0, { -5, -2, -4, 0 }, { 0, 0, 0, tan3_2 } },
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
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _hasB0_VG }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst1[] = {
    { VIR_OP_AQ_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

/* b0 has to be interger */
static VIR_PatternMatchInst _ldarrPatInst2[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform, _hasB0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst2[] = {
    { VIR_OP_AQ_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst3[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst3[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloatUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst4[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst4[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst5[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst5[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, 0, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst6[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst6[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, 0, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst7[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_not_sampler_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst7[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat } },
};

static VIR_PatternMatchInst _ldarrPatInst8[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst8[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt } },
};

static VIR_Pattern _ldarrPattern[] = {
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 1) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 2) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 3) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 4) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 5) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 6) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 7) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 8) },
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
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destPackedType, _isF2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst5[] = {
    { VIR_OP_CONV, -1, 0, { -1, 2, 0, 0 }, { value_type0_32bit_pattern_dest } },
    { VIR_OP_SWIZZLE, 0, 0, {  1, -1, 1, 1 }, { 0, _revise2PackedTypeAndSwizzle, _setPackedSwizzle, _setPackedMaskValue } },
};

static VIR_PatternMatchInst _convPatInst6[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst6[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { value_type0_32bit } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2F },
        { -1, 0x2D, 1, 2, 0, 0, 0, _value_type0_32bit_from_src0 },
    */
static VIR_PatternMatchInst _convPatInst7[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _src0PackedType, _isI2F }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst7[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { value_type0_from_src0_unpacked, 0, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_CONV, -1, 0, {  1, -1, 0, 0 }, { 0, _revise2UnPackedTypeAndSwizzle } },
};

static VIR_PatternMatchInst _convPatInst8[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2F }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst8[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { _value_type0_32bit_from_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2us) },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -1, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst9[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst9[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, {  1, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_u2us) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
    */
static VIR_PatternMatchInst _convPatInst10[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_u2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst10[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_GREATER, 0, { 1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2u) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst11[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2u }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst11[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_LESS, 0, { 1, 2, 0, 2 }, { min_type0_const, revise_dest_type_by_operand_type } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I },
        { -1, 0x2C, 1, 2, gcSL_CG_CONSTANT, 0, 0, value_types_I2I },
    */
static VIR_PatternMatchInst _convPatInst12[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destOrSrc0PackedType, _isI2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst12[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, _destTypeRankHigher_setSrcToZero_elseNop } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 2, 2 }, { 0, _equatePackedTypeForDestOrSrc, _setConvPackedSwizzle, _setConvPackedMaskValue } },
};


static VIR_PatternMatchInst _convPatInst13[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst13[] = {
    { VIR_OP_CONV, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Signed_8_16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
    */
static VIR_PatternMatchInst _convPatInst14[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Signed_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst14[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_24_16 } },
    { VIR_OP_RSHIFT, 0, 0, { 1, -1, 0, 0 }, { int_value_type0_const_24_16 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Unsigned_8_16) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },
    */
static VIR_PatternMatchInst _convPatInst15[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Unsigned_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst15[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { uint_value_type0_const_FF_FFFF } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_8bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FF },
    */
static VIR_PatternMatchInst _convPatInst16[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_8bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst16[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
    */
static VIR_PatternMatchInst _convPatInst17[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst17[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { short_value_type0_const_8 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { short_value_type0_const_8 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FFFF },
    */
static VIR_PatternMatchInst _convPatInst18[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst18[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FFFF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
    */
static VIR_PatternMatchInst _convPatInst19[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst19[] = {
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
static VIR_PatternMatchInst _convPatInst20[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst20[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { int_value_type0_const_16 } },
};

static VIR_PatternMatchInst _convPatInst21[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { supportCONV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst21[] = {
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
static VIR_PatternMatchInst _convPatInst22[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF16_2_F32_hasCMP_NotFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst22[] = {
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
static VIR_PatternMatchInst _convPatInst23[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF32_2_F16_hasCMP_NotFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst23[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { value_types_32, 0, float32_exp } }, /* -1 is 8 bit exp */
    { VIR_OP_CMP, VIR_COP_LESS_OR_EQUAL, 0, { -2, -1, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, float32_exp_underflow } }, /* -2 is exp_underflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -3, -2, -1, 0 }, { value_types_32, 0, 0, float32_exp_underflow } }, /* -3 is to-sub */
    { VIR_OP_SUB, 0, 0, { -4, -1, -3, 0 }, { value_types_32 } }, /* -4 is 5 bit exp */
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL, 0, { -5, -1, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, float32_exp_overflow } }, /* -5 is exp_overflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -4, -5, 0, -4 }, { value_types_32, 0, float32_exp_isoverflow } },
    { VIR_OP_RSHIFT, 0, 0, { -4, -4, 0, 0 }, { value_types_32, 0, float32_man_bits } },
    { VIR_OP_AND_BITWISE, 0, 0, { -6, 2, 0, 0 }, { value_types_32, 0, float32_sign } }, /* -6 is sign bit */
    { VIR_OP_RSHIFT, 0, 0, { -6, -6, 0, 0 }, { float32_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -4, -4, -6, 0 }, { value_types_16 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -7, 2, 0, 0 }, { value_types_32, 0, float32_man } }, /* -7 is 23 bit man */
    { VIR_OP_RSHIFT, 0, 0, { -8, -7, 0, 0 }, { value_types_32, 0, float32_man_bits } }, /* -8 is 10 bit man */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -8, -5, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL, 0, { -9, -1, 0, 0 }, { VIR_Lower_SetOpndUINT32, 0, float32_exp } }, /* -9 is exp_overflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -8, -9, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntOne } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -4, -8, 0 }, { value_types_16 } },
};

static VIR_PatternMatchInst _convPatInst24[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF16_2_F32_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst24[] = {
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

static VIR_PatternMatchInst _convPatInst25[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF32_2_F16_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst25[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { value_types_32, 0, float32_exp } }, /* -1 is 8 bit exp */
    { VIR_OP_CMP, VIR_COP_LESS_OR_EQUAL, 0, { -2, -1, 0, 0 }, { _setBooleanType, 0, float32_exp_underflow } }, /* -2 is exp_underflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -3, -2, -1, 0 }, { value_types_32, 0, 0, float32_exp_underflow } }, /* -3 is to-sub */
    { VIR_OP_SUB, 0, 0, { -4, -1, -3, 0 }, { value_types_32 } }, /* -4 is 5 bit exp */
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL, 0, { -5, -1, 0, 0 }, { _setBooleanType, 0, float32_exp_overflow } }, /* -5 is exp_overflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -4, -5, 0, -4 }, { value_types_32, 0, float32_exp_isoverflow } },
    { VIR_OP_RSHIFT, 0, 0, { -4, -4, 0, 0 }, { value_types_32, 0, float32_man_bits } },
    { VIR_OP_AND_BITWISE, 0, 0, { -6, 2, 0, 0 }, { value_types_32, 0, float32_sign } }, /* -6 is sign bit */
    { VIR_OP_RSHIFT, 0, 0, { -6, -6, 0, 0 }, { float32_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -4, -4, -6, 0 }, { value_types_16 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -7, 2, 0, 0 }, { value_types_32, 0, float32_man } }, /* -7 is 23 bit man */
    { VIR_OP_RSHIFT, 0, 0, { -8, -7, 0, 0 }, { value_types_32, 0, float32_man_bits } }, /* -8 is 10 bit man */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -8, -5, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL, 0, { -9, -1, 0, 0 }, { _setBooleanType, 0, float32_exp } }, /* -9 is exp_overflow */
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, { -8, -9, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntOne } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -4, -8, 0 }, { value_types_16 } },
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 23) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 24) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 25) },
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
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { jmp_2_succ2_resCondOp_float }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_KILL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { no_source }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst7[] = {
    { VIR_OP_KILL, -1, 0, { 0, 2, 3, 0 }, { reverseCondOp } }
};

static VIR_PatternMatchInst _jmpcPatInst8[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { jmp_2_succ2_resCondOp_singleChannel }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { no_source }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 5, 0, 0, 0 }, { label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst8[] = {
    { VIR_OP_JMPC, -1, 0, { 4, 2, 3, 0 }, { reverseCondOp } }
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 8) },
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

/* VIR_OP_CMP patterns. */
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

static VIR_PatternMatchInst _cmpPatInst2[] = {
    { VIR_OP_CMP, VIR_COP_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst2[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst3[] = {
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst3[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NOT_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst4[] = {
    { VIR_OP_CMP, VIR_COP_LESS_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst4[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_LESS, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst5[] = {
    { VIR_OP_CMP, VIR_COP_GREATER_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst5[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_GREATER, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst6[] = {
    { VIR_OP_CMP, VIR_COP_LESS_OR_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst6[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_LESS_OR_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst7[] = {
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst7[] = {
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_CMP, VIR_COP_GREATER_OR_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_CMP, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_Pattern _cmpPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 7) },
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
_genFloatCoordDataBias(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    gctUINT dataType = 0x1; /* 0x1 */
    gctUINT mode = 0x2; /* 0x2 */
    gctUINT addressingType = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)));

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);

    return gcvTRUE;
}

static gctBOOL
_genFloatCoordDataLod(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    gctUINT dataType = 0x1; /* 0x1 */
    gctUINT mode = 0x1; /* 0x1 */
    gctUINT addressingType = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)));

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);

    return gcvTRUE;
}

static gctBOOL
_genIntegeroordDataLod(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm;
    gctUINT dataType = 0x0; /* 0x0 */
    gctUINT mode = 0x1; /* 0x1 */
    gctUINT addressingType = 0x0; /* 0x0 */
    gctUINT magFilter = 0x0; /* 0x0 */
    gctUINT minFilter = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ? 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ? 1:0) - (0 ?
 1:0) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ?
 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ? 13:12) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 13:12) - (0 ?
 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ? 9:8) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ?
 9:8)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (magFilter) & ((gctUINT32) ((((1 ?
 4:4) - (0 ? 4:4) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ?
 4:4)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (minFilter) & ((gctUINT32) ((((1 ?
 5:5) - (0 ? 5:5) + 1) == 32) ? ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ?
 5:5)));

    VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);

    return gcvTRUE;
}

/* Generate TEXLD_U_F_L. */
static VIR_PatternMatchInst _texlduPatInst0[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst0[] = {
    { VIR_OP_TEXLD_U_F_L, 0, 0, { 1, 2, 3, 4, 0 }, { 0, 0, 0, 0, _genFloatCoordDataLod } },
};

/* Generate TEXLD_U_F_B. */
static VIR_PatternMatchInst _texlduPatInst1[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _isCoordFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst1[] = {
    { VIR_OP_TEXLD_U_F_B, 0, 0, { 1, 2, 3, 4, 0 }, { 0, 0, 0, 0, _genFloatCoordDataBias } },
};

/* Generate TEXLD_U_S_L. */
static VIR_PatternMatchInst _texlduPatInst2[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst2[] = {
    { VIR_OP_TEXLD_U_S_L, 0, 0, { 1, 2, 3, 4, 0 }, { 0, 0, 0, 0, _genIntegeroordDataLod } },
};

/* Generate TEXLD_U_U_L. */
static VIR_PatternMatchInst _texlduPatInst3[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordUnSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst3[] = {
    { VIR_OP_TEXLD_U_U_L, 0, 0, { 1, 2, 3, 4, 0 }, { 0, 0, 0, 0, _genIntegeroordDataLod } },
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

/* texldpcf patterns. */
/* Move the Dref into last component of coord and hold the other modifier. */
static VIR_PatternMatchInst _texldpcfPatInst0[] = {
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, 3, 4 }, { _hasDrefAndOtherTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3 }, { 0, _setDestTypeFromSrc0 } },
    { VIR_OP_MOV, 0, 0, { -1, 4 }, { 0, _setDrefToLastComponent } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, -1, 4 }, { 0, 0, _setNewCoordWithDref, _cleanDrefTexModifier } },
};

static VIR_PatternMatchInst _texldpcfPatInst1[] = {
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, 3, 4 }, { _onlyHasDrefTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3 }, { 0, _setDestTypeFromSrc0 } },
    { VIR_OP_MOV, 0, 0, { -1, 4 }, { 0, _setDrefToLastComponent } },
    { VIR_OP_TEXLDPCF, 0, 0, { 1, 2, -1 }, { 0, 0, _setNewCoordWithDref } },
};

/*
  { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -1, 0x6F, 1, 3, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, zero_1_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst2[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst2[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst3[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst3[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst4[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst4[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_PCF, 0, 0, {  1, 2, 3, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_AdjustCoordSwizzleForShadow } },
};

/*
    { 1, gcSL_TEXLDPCF, 1, 2, 3 },
        { -3, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW },
        { -2, 0x18, 1, 3, 0, 0, 2, sample_swizzleX },
        { -1, 0x10, 1, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 1, 0, 0, conditionLE },
*/
static VIR_PatternMatchInst _texldpcfPatInst5[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst5[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst6[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst6[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst7[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst7[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_XYZ, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, 3, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLDL, 0, 0, { -3, 2, -1, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_AQ_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst8[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow, _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst8[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, 3, 0, -1 }, { 0, 0, VIR_Lower_AdjustCoordSwizzleForShadow, VIR_Lower_SetZero } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst9[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow, _hasNEW_TEXLD, }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst9[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_PCF, 0, 0, {  1, 2, 3, 0, -1 }, { 0, 0, VIR_Lower_AdjustCoordSwizzleForShadow} },
};

/*
    { 2, gcSL_TEXBIAS, 0, 2, 4 },
    { 1, gcSL_TEXLDPCF, 1, 2, 3, 0, 0, _hasNEW_TEXLD },
        { -2, 0x09, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 0, 0, 3, 0, saturate_swizzle2ZorW_from_next_inst },
        { -1, 0x18, 1, 3, 4, gcSL_CG_TEMP1_X_NO_SRC_SHIFT, 2, sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfPatInst10[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst10[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 3, 0, 0, 0 }, { 0, _SetSwizzleByType } },
    { VIR_OP_TEXLD_BIAS_PCF, 0, 0, {  1, 2, 3, 4, -1 }, { 0, VIR_Lower_SetSwizzleX } },
};

/* For a cubeArrayShadow, the bias is compare value. */
static VIR_PatternMatchInst _texldpcfPatInst11[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifierAndCubeArrayShadow }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst11[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst12[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst12[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst13[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfRepInst13[] = {
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
static VIR_PatternMatchInst _texldpcfPatInst14[] = {
    { VIR_OP_TEXLDPCF, -1, 0, { 1, 2, 3, 4, 5 }, { _isGatherTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfRepInst14[] = {
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 5, 0, 0, 0 }, { 0 } },
    { VIR_OP_TEXLD_GATHER_PCF, 0, 0, {  1, 2, 3, 4, -1 }, { 0 } },
};

VIR_Pattern _texldpcfPattern[] = {
    { VIR_PATN_FLAG_NOT_EXPAND_SPECIAL_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcf, 0) },
    { VIR_PATN_FLAG_NOT_EXPAND_SPECIAL_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcf, 1) },
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 13) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcf, 14) },
    { VIR_PATN_FLAG_NONE }
};

/* texldprcproj patterns. */
/* Move the Dref into last but one component(the last one component is for the projective division ) of coord and hold the other modifier. */
static VIR_PatternMatchInst _texldpcfprojPatInst0[] = {
    { VIR_OP_TEXLDPCFPROJ, 0, 0, { 1, 2, 3, 4 }, { _hasDrefAndOtherTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3 }, { 0, _setDestTypeFromSrc0 } },
    { VIR_OP_MOV, 0, 0, { -1, 4 }, { 0, _setDrefToLastButOneComponent } },
    { VIR_OP_TEXLDPCFPROJ, 0, 0, { 1, 2, -1, 4 }, { 0, 0, _setNewCoordWithDref, _cleanDrefTexModifier } },
};

static VIR_PatternMatchInst _texldpcfprojPatInst1[] = {
    { VIR_OP_TEXLDPCFPROJ, 0, 0, { 1, 2, 3, 4 }, { _onlyHasDrefTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst1[] = {
    { VIR_OP_MOV, 0, 0, { -1, 3 }, { 0, _setDestTypeFromSrc0 } },
    { VIR_OP_MOV, 0, 0, { -1, 4 }, { 0, _setDrefToLastButOneComponent } },
    { VIR_OP_TEXLDPCFPROJ, 0, 0, { 1, 2, -1 }, { 0, 0, _setNewCoordWithDref } },
};

/*
    { 1, gcSL_TEXLDPCFPROJ, 1, 2, 3, 0, 0, _hasNEW_TEXLD_isVertex },
        { -4, 0x0C, gcSL_CG_TEMP1_XYZW, 0, 0, 3, 0, enable_w_swizzle2W },
        { -3, 0x03, gcSL_CG_TEMP1_XYZW, 3, gcSL_CG_TEMP1_XYZW, 0, 0, swizzle1W },
        { -2, 0x09, gcSL_CG_TEMP1_XYZW, 0, 0, gcSL_CG_TEMP1_XYZW, 0, enable_z_saturate_swizzle2Z },
        { -1, 0x6F, 1, gcSL_CG_TEMP1_XYZW, gcSL_CG_CONSTANT, gcSL_CG_TEMP1_XYZW, 2, zero_1_swizzle2Z_sample_swizzleX },
*/
static VIR_PatternMatchInst _texldpcfprojPatInst2[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD, _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst2[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst3[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _isNeedToSetLod }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst3[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst4[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst4[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst5[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst5[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst6[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst6[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst7[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst7[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst8[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst8[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst9[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4 }, { _isBiasTexModifier }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst9[] = {
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
static VIR_PatternMatchInst _texldpcfprojPatInst10[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 4, 5 }, { _isGradTexModifier, _hasNEW_TEXLD }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst10[] = {
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_ADD, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -2, 4, -1, 0 }, { _setFloatType, 0, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_ADD, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -3, 5, -1, 0 }, { _setFloatType, 0, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_TEXLD_G_PCF, 0, 0, {  1, 2, -1, -2, -3 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW } },
};

static VIR_PatternMatchInst _texldpcfprojPatInst11[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 5 }, { _hasNEW_TEXLD, _isGatherTexModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst11[] = {
    { VIR_OP_MOV, 0, 0, {  3, 5, 0, 0 }, { VIR_Lower_SetEnableZ } },
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_PCF, 0, 0, {  1, 2, -1, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, VIR_Lower_SetSwizzleZEx } },
};

static VIR_PatternMatchInst _texldpcfprojPatInst12[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 5, 6 }, { _hasNEW_TEXLD, _isGatherAndLodTexModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst12[] = {
    { VIR_OP_MOV, 0, 0, {  3, 6, 0, 0 }, { VIR_Lower_SetEnableZ } },
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_LOD_PCF, 0, 0, {  1, 2, -1, 5, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, 0, VIR_Lower_SetSwizzleZEx } },
};

static VIR_PatternMatchInst _texldpcfprojPatInst13[] = {
    { VIR_OP_TEXLDPCFPROJ, -1, 0, { 1, 2, 3, 5, 6 }, { _hasNEW_TEXLD, _isGatherAndBiasTexModifier }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texldpcfprojRepInst13[] = {
    { VIR_OP_MOV, 0, 0, {  3, 6, 0, 0 }, { VIR_Lower_SetEnableZ } },
    { VIR_OP_RCP, 0, VIR_PATTERN_TEMP_TYPE_W, { -1, 3, 0, 0 }, { 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_MUL, 0, VIR_PATTERN_TEMP_TYPE_XYZW,{ -1, 3, -1, 0 }, { 0, 0, VIR_Lower_SetSwizzleWEx } },
    { VIR_OP_SAT, 0, VIR_PATTERN_TEMP_TYPE_Z, { -1, -1, 0, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
    { VIR_OP_TEXLD_BIAS_PCF, 0, 0, {  1, 2, -1, 5, -1 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleXYZW, 0, VIR_Lower_SetSwizzleZEx } },
};

VIR_Pattern _texldpcfprojPattern[] = {
    { VIR_PATN_FLAG_NOT_EXPAND_SPECIAL_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcfproj, 0) },
    { VIR_PATN_FLAG_NOT_EXPAND_SPECIAL_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcfproj, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 8) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 10) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 11) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 12) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldpcfproj, 13) },
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

static gctBOOL
_destChar_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return tyId == VIR_TYPE_INT8_P3 || tyId == VIR_TYPE_UINT8_P3;
}

gctBOOL
_setChar_P3MaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x07;  /* last 3 bytes */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

gctBOOL
_setChar_P3Swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x0840;   /* byte 0 -> byte 0; byte 4 -> byte 1; byte 8 -> byte 2; */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_setPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components =VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
            imm0.iValue = 0x0003;
            break;

        case 3:
            imm0.iValue = 0x0007;
            break;

        case 4:
            imm0.iValue = 0x000F;
            break;

        case 8:
            imm0.iValue = 0x00FF;
            break;

        case 16:
            imm0.iValue = 0xFFFF;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
            imm0.iValue = 0x000F;
            break;

        case 3:
            imm0.iValue = 0x003F;
            break;

        case 4:
            imm0.iValue = 0x00FF;
            break;

        case 8:
            imm0.iValue = 0xFFFF;
            break;

        case 16:
            /* should have been split already */
            gcmASSERT(0);
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);

    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

static gctBOOL
_setPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.uValue = 0;
    components =VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
            imm0.uValue = 0x0040;   /* byte 0 -> byte 0; byte 4 -> byte 1; */
            break;

        case 3:
            imm0.uValue = 0x0840;   /* byte 0 -> byte 0; byte 4 -> byte 1; byte 8 -> byte 2; */
            break;

        case 4:
            imm0.uValue = 0xC840;   /* byte 0 -> byte 0; byte 4 -> byte 1; byte 8 -> byte 2; byte 12 -> byte 3*/
            break;

        case 8:
            imm0.uValue = 0xC8400000;   /* second register byte 0 -> byte 4; byte 4 -> byte 5; byte 8 -> byte 6; byte 12 -> byte 8*/
            break;

        case 16:
            imm0.uValue = 0xC8400000;   /* fourth register byte 0 -> byte 12; byte 4 -> byte 13; byte 8 -> byte 14; byte 12 -> byte 15*/
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
            imm0.uValue = 0x0020;   /* short 0 -> short 0; short 2 -> short 1; */
            break;

        case 3:
            imm0.uValue = 0x0420;   /* short 0 -> short 0; short 2 -> short 1; short 4 -> short 2; */
            break;

        case 4:
            imm0.uValue = 0x6420;   /* short 0 -> short 0; short 2 -> short 1; short 4 -> short 2; short 6 -> short 3*/
            break;

        case 8:
            imm0.uValue = 0x64200000;   /* second register short 0 -> short 4; short 2 -> short 5; short 4 -> short 6; short 6 -> short 7*/
            break;

        case 16:
            imm0.uValue = 0x64200000;   /* fourth register short 0 -> short 12; short 2 -> short 13; short 4 -> short 14; short 6 -> short 15*/
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_setColumn1PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components =VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
        case 3:
        case 4:
            imm0.iValue = 0x002;
            break;

        case 8:
            imm0.iValue = 0x0022;
            break;

        case 16:
            imm0.iValue = 0x2222;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
        case 3:
            imm0.iValue = 0x000C;
            break;

        case 4:
            imm0.iValue = 0x00CC;
            break;

        case 8:
            imm0.iValue = 0xCCCC;
            break;

        case 16:
            /* should have been split already */
            gcmASSERT(0);
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_setColumn1PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
        case 3:
        case 4:
            imm0.iValue = 0x0000;
            break;

        case 8:
            imm0.iValue = 0x00400000;
            break;

        case 16:
            imm0.iValue = 0xC840;
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00400000;
            vConst.value.vecVal.u32Value[1] = 0x00C00080;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
        case 3:
            imm0.iValue = 0x0000;
            break;

        case 4:
            imm0.iValue = 0x2000;
            break;

        case 8:
            imm0.iValue = 0x60402000;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    if(useImm)
    {
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_INT32,
                                 imm0);
    }
    else
    {
        VIR_Uniform*       pImmUniform;
        VIR_Symbol*        sym;
        VIR_Swizzle        swizzle = VIR_SWIZZLE_XYYY;

        vConst.type = VIR_TYPE_UINT_X2;
        vConst.index = VIR_INVALID_ID;

        VIR_Shader_AddInitializedUniform(Context->shader, &vConst, &pImmUniform, &swizzle);

        /* Set this uniform as operand and set correct swizzle */
        sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
        VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(Opnd, sym);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }
    return gcvTRUE;
}

static gctBOOL
_setUnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand * dest = VIR_Inst_GetDest(patternInst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;

    imm0.iValue = 0;
    components = VIR_GetTypeComponents(tyId);
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetType(Opnd)));
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
            imm0.iValue = 0x0011;
            break;

        case 3:
            imm0.iValue = 0x0111;
            break;

        case 4:
            imm0.iValue = 0x1111;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
            imm0.iValue = 0x0033;
            break;

        case 3:
            imm0.iValue = 0x0333;
            break;

        case 4:
            imm0.iValue = 0x3333;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

static gctBOOL
_setUnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand * dest = VIR_Inst_GetDest(patternInst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;

    imm0.uValue = 0;
    components = VIR_GetTypeComponents(tyId);
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetType(Opnd)));
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
            imm0.uValue = 0x00010000;
            break;

        case 3:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00010000;
            vConst.value.vecVal.u32Value[1] = 0x00000002;
            break;

        case 4:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00010000;
            vConst.value.vecVal.u32Value[1] = 0x00030002;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
            imm0.uValue = 0x00000100;
            break;

        case 3:
            imm0.uValue = 0x00020100;
            break;

        case 4:
            imm0.uValue = 0x03020100;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    if(useImm)
    {
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
    }
    else
    {
        VIR_Uniform*       pImmUniform;
        VIR_Symbol*        sym;
        VIR_Swizzle        swizzle = VIR_SWIZZLE_XYYY;

        vConst.type = VIR_TYPE_UINT_X2;
        vConst.index = VIR_INVALID_ID;

        VIR_Shader_AddInitializedUniform(Context->shader, &vConst, &pImmUniform, &swizzle);

        /* Set this uniform as operand and set correct swizzle */
        sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
        VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(Opnd, sym);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }
    return gcvTRUE;
}

static gctBOOL
_setColumn1UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;

    imm0.iValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
        case 3:
        case 4:
            imm0.iValue = 0x0001;
            break;

        case 8:
            imm0.iValue = 0x0011;
            break;

        case 16:
            imm0.iValue = 0x1111;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
        case 3:
            imm0.iValue = 0x0003;
            break;

        case 4:
            imm0.iValue = 0x0033;
            break;

        case 8:
            imm0.iValue = 0x3333;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

static gctBOOL
_setColumn1UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;

    imm0.uValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
        case 3:
        case 4:
            imm0.uValue = 0x00000001;
            break;

        case 8:
            imm0.uValue = 0x00050001;
            break;

        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00050001;
            vConst.value.vecVal.u32Value[1] = 0x000C0009;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(components) {
        case 2:
        case 3:
            imm0.uValue = 0x00000001;
            break;

        case 4:
            imm0.uValue = 0x00000301;
            break;

        case 8:
            imm0.uValue = 0x07050301;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    if(useImm)
    {
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
    }
    else
    {
        VIR_Uniform*       pImmUniform;
        VIR_Symbol*        sym;
        VIR_Swizzle        swizzle = VIR_SWIZZLE_XYYY;

        vConst.type = VIR_TYPE_UINT_X2;
        vConst.index = VIR_INVALID_ID;

        VIR_Shader_AddInitializedUniform(Context->shader, &vConst, &pImmUniform, &swizzle);

        /* Set this uniform as operand and set correct swizzle */
        sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
        VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(Opnd, sym);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }
    return gcvTRUE;
}

static gctUINT
_getOperandEnableComponentCount(
    IN VIR_PatternContext *Context,
    IN VIR_Operand *Opnd
    )
{
    gctUINT compCount = 0;
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);

    if(VIR_Operand_isLvalue(Opnd))
    {
       compCount = VIR_Enable_Channel_Count(VIR_Operand_GetEnable(Opnd));
    }
    else
    {
       compCount = VIR_Enable_Channel_Count(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd)));
    }
    if(VIR_TypeId_isPacked(tyId))
    {
        gctUINT components = VIR_GetTypePackedComponents(tyId);
        VIR_PrimitiveTypeId format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));

        switch(format) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            if(components == 3) {
                gcmASSERT(compCount == 1);
                compCount = 3;
            }
            else {
                compCount <<= 2;
                gcmASSERT(compCount <= 16);
            }
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_FLOAT16:
            if(components == 3) {
                gcmASSERT(compCount < 3);
                if(compCount == 1) compCount = 2;
                else compCount = 3;
            }
            else {
                compCount <<= 1;
                gcmASSERT(compCount <= 8);
            }
            break;

        default:
            gcmASSERT(0);
            compCount = 0;
        }
    }
    return compCount;
}

static gctBOOL
_equatePackedTypeForDestOrSrc(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand * dest = VIR_Inst_GetDest(patternInst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);
    VIR_TypeId srcTyId = VIR_Operand_GetType(Opnd);
    VIR_PrimitiveTypeId srcFormat, destFormat;
    gctUINT components = 0;
    gctBOOL   changeSrc = gcvFALSE, changeDest = gcvFALSE;

    destFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    srcFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(destFormat) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            changeSrc = gcvTRUE;
            components = VIR_GetTypePackedComponents(srcTyId) << 1;
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            changeSrc = gcvTRUE;
            components = VIR_GetTypeComponents(srcTyId) << 2;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            changeDest = gcvTRUE;
            components = VIR_GetTypePackedComponents(tyId) << 1;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            changeSrc = gcvTRUE;
            components = VIR_GetTypeComponents(srcTyId) << 1;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            changeDest = gcvTRUE;
            components = VIR_GetTypeComponents(tyId) << 2;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            changeDest = gcvTRUE;
            components = VIR_GetTypeComponents(tyId) << 1;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    if(changeDest) {
        dest = VIR_Inst_GetDest(Inst);
        gcmASSERT(dest);
        gcmASSERT(components);
        VIR_Operand_SetType(dest, VIR_TypeId_ComposePackedNonOpaqueType(srcFormat, components));
    }
    if(changeSrc) {
        gcmASSERT(components);
        VIR_Operand_SetType(Opnd, VIR_TypeId_ComposePackedNonOpaqueType(destFormat, components));
    }
    return gcvTRUE;
}

static gctBOOL
_setConvPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand * dest = VIR_Inst_GetDest(patternInst);
    VIR_ScalarConstVal imm0;
    gctUINT32  components, i;
    VIR_PrimitiveTypeId srcFormat, destFormat;
    gctUINT    nextSlot = 1;
    gctUINT    startChannel = 0;
    gctUINT    componentPerChannel = 1;
    gctUINT    curComponent;
    VIR_Enable enable;
    gctUINT    maskShift;
    gctUINT    maskValue = 0x1;
    gctUINT    maskWidth = 1;

    imm0.iValue = 0;
    components = _getOperandEnableComponentCount(Context,
                                                 dest);
    enable = VIR_Operand_GetEnable(dest);
    if(enable & VIR_ENABLE_X) startChannel = 0;
    else if(enable & VIR_ENABLE_Y) startChannel = 1;
    else if(enable & VIR_ENABLE_Z) startChannel = 2;
    else startChannel = 3;

    destFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    srcFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(destFormat) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            componentPerChannel = 4;
            break;
        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            componentPerChannel = 4;
            nextSlot = 2;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            componentPerChannel = 2;
            maskValue = 0x3;
            maskWidth = 2;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            componentPerChannel = 4;
            nextSlot = 4;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            componentPerChannel = 2;
            nextSlot = 2;
            maskValue = 0x3;
            maskWidth = 2;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    curComponent = startChannel * componentPerChannel;
    maskShift = 0;
    for(i = 0; i < components; i++) {
        maskShift = curComponent * maskWidth;
        gcmASSERT(maskShift < 16);
        imm0.iValue |= maskValue << maskShift;
        curComponent += nextSlot;
    }

    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    return _checkToSetFullDefFlag(Context,
                                  Inst,
                                  Opnd);
}

static gctBOOL
_setConvPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand * dest = VIR_Inst_GetDest(patternInst);
    VIR_ScalarConstVal imm0;
    gctUINT32  components, i;
    VIR_Const  vConst;
    gctUINT    stride;
    gctUINT    nextSlot = 1;
    gctUINT    channel;
    gctUINT    startChannel = 0;
    gctUINT    componentPerChannel = 1;
    gctUINT    curComponent;
    gctUINT    swizzleShift;
    gctUINT    shiftIncr;
    VIR_Enable enable;
    gctBOOL    useImm = gcvTRUE;
    VIR_PrimitiveTypeId srcFormat, destFormat;

    gcoOS_ZeroMemory(&vConst, sizeof(vConst));
    imm0.uValue = 0;
    components = _getOperandEnableComponentCount(Context,
                                                 dest);
    enable = VIR_Operand_GetEnable(dest);
    if(enable & VIR_ENABLE_X) startChannel = 0;
    else if(enable & VIR_ENABLE_Y) startChannel = 1;
    else if(enable & VIR_ENABLE_Z) startChannel = 2;
    else startChannel = 3;

    destFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    srcFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(destFormat) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        componentPerChannel = 4;
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            stride = 1;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            stride = 2;
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            stride = 4;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            stride = 1;
            nextSlot = 2;
            componentPerChannel = 4;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            stride = 1;
            componentPerChannel = 2;
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            stride = 2;
            componentPerChannel = 2;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    case VIR_TYPE_INT32:
    case VIR_TYPE_UINT32:
        switch(srcFormat) {
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            componentPerChannel = 4;
            stride = 1;
            nextSlot = 4;
            break;

        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
            componentPerChannel = 2;
            stride = 1;
            nextSlot = 2;
            break;

        case VIR_TYPE_INT32:
        case VIR_TYPE_UINT32:
            stride = 1;
            break;

        default:
            gcmASSERT(0);
            return gcvFALSE;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    channel = VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(Opnd), 0);
    swizzleShift = (startChannel * componentPerChannel) << 2;
    shiftIncr = nextSlot << 2;
    curComponent = channel * componentPerChannel;
    for(i = 0; i < components; i++) {
        if(swizzleShift >= 32) {
            swizzleShift -= 32;
            vConst.value.vecVal.u32Value[0] = imm0.uValue;
            useImm = gcvFALSE;
            imm0.uValue = 0;
        }
        imm0.uValue |=  curComponent << swizzleShift;
        swizzleShift += shiftIncr;
        curComponent += stride;
    }

    if(useImm)
    {
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
    }
    else
    {
        VIR_Uniform*       pImmUniform;
        VIR_Symbol*        sym;
        VIR_Swizzle        swizzle = VIR_SWIZZLE_XYYY;

        vConst.value.vecVal.u32Value[1] = imm0.uValue; /* copy over the second word */
        vConst.type = VIR_TYPE_UINT_X2;
        vConst.index = VIR_INVALID_ID;

        VIR_Shader_AddInitializedUniform(Context->shader, &vConst, &pImmUniform, &swizzle);

        /* Set this uniform as operand and set correct swizzle */
        sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
        VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(Opnd, sym);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
        VIR_Operand_SetType(Opnd, VIR_TYPE_UINT_X2);
    }
    return gcvTRUE;
}

static gctBOOL
_destShort_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);


    return tyId == VIR_TYPE_INT16_P8 ||
           tyId == VIR_TYPE_UINT16_P8;
}

static gctBOOL
_destShort_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return tyId == VIR_TYPE_INT16_P3 || tyId == VIR_TYPE_UINT16_P3;
}

gctBOOL
_setShort_P3MaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x03F;  /* last 3 words */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

gctBOOL
_setShort_P4MaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x0FF;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    return gcvTRUE;
}

gctBOOL
_setShort_P3Swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x0420;   /* word 0 -> word 0; word 2 -> word 1; word 4 -> word 2; */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

/* get default packed swizzle only up to 8 components */
gctBOOL
_setDefaultPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0;
    switch(tyId) {
    case VIR_TYPE_INT8_P3:
    case VIR_TYPE_UINT8_P3:
    case VIR_TYPE_INT16_P3:
    case VIR_TYPE_UINT16_P3:
        imm0.iValue = 0x2210;
        break;

    case VIR_TYPE_INT8_P2:
    case VIR_TYPE_UINT8_P2:
    case VIR_TYPE_INT16_P2:
    case VIR_TYPE_UINT16_P2:
        imm0.iValue = 0x10;
        break;

    case VIR_TYPE_INT8_P4:
    case VIR_TYPE_UINT8_P4:
    case VIR_TYPE_INT16_P4:
    case VIR_TYPE_UINT16_P4:
        imm0.iValue = 0x3210;
        break;

    case VIR_TYPE_INT8_P8:
    case VIR_TYPE_UINT8_P8:
    case VIR_TYPE_INT16_P8:
    case VIR_TYPE_UINT16_P8:
        imm0.iValue = 0x76543210;
        break;

    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_src0PackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return VIR_TypeId_isPacked(tyId);
}

static gctBOOL
_destPackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return VIR_TypeId_isPacked(tyId);
}

static gctBOOL
_destOrSrc0PackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _destPackedType(Context, Inst) || _src0PackedType(Context, Inst);
}

static gctBOOL
_destGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return VIR_GetTypeSize(tyId) > 16;
}

static gctBOOL
_destPackedGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) > 16);
}

static gctBOOL
_destPackedLE16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetType(dest);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) <= 16);
}

static gctBOOL
_src0PackedGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) > 16);
}

static gctBOOL
_src0GT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return VIR_GetTypeSize(tyId) > 16;
}

static gctBOOL
_src0ScalarOrPackedLE16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    if(!(VIR_TypeId_isPacked(tyId) && VIR_GetTypeSize(tyId) <= 16)) {
        VIR_Type *type;
        type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
        return VIR_Type_isScalar(type);
    }
    return gcvTRUE;
}

static gctBOOL
_src2PackedGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 2);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) > 16);
}

static void
_changeEnableByTyId(
    IN VIR_TypeId          TyId,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_ENABLE_X;
    gcmASSERT (Opnd != gcvNULL);

    gcmASSERT(VIR_Operand_isLvalue(Opnd));

    enable = VIR_TypeId_Conv2Enable(TyId);

    VIR_Operand_SetEnable(Opnd, enable);

    return;
}

gctBOOL
_change2NonpackedTypeForLoadStore(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);

    switch (tyId)
    {
    case VIR_TYPE_INT8_P3:
        tyId = VIR_TYPE_INT8_X3;
        break;
    case VIR_TYPE_UINT8_P3:
        tyId = VIR_TYPE_UINT8_X3;
        break;
    case VIR_TYPE_INT16_P3:
        tyId = VIR_TYPE_INT16_X3;
        break;
    case VIR_TYPE_UINT16_P3:
        tyId = VIR_TYPE_UINT16_X3;
        break;

    /* other packed types */
    case VIR_TYPE_INT8_P2:
        tyId = VIR_TYPE_INT16;
        break;
    case VIR_TYPE_INT8_P4:
        tyId = VIR_TYPE_INT32;
        break;
    case VIR_TYPE_INT8_P8:
        tyId = VIR_TYPE_INTEGER_X2;
        break;
    case VIR_TYPE_INT8_P16:
        tyId = VIR_TYPE_INTEGER_X4;
        break;
    case VIR_TYPE_UINT8_P2:
        tyId = VIR_TYPE_UINT16;
        break;
    case VIR_TYPE_UINT8_P4:
        tyId = VIR_TYPE_UINT32;
        break;
    case VIR_TYPE_UINT8_P8:
        tyId = VIR_TYPE_UINT_X2;
        break;
    case VIR_TYPE_UINT8_P16:
        tyId = VIR_TYPE_UINT_X4;
        break;

    case VIR_TYPE_INT16_P2:
        tyId = VIR_TYPE_INT32;
        break;
    case VIR_TYPE_INT16_P4:
        tyId = VIR_TYPE_INTEGER_X2;
        break;
    case VIR_TYPE_INT16_P8:
        tyId = VIR_TYPE_INTEGER_X4;
        break;
    case VIR_TYPE_INT16_P16:
        tyId = VIR_TYPE_INTEGER_X8;
        break;
    case VIR_TYPE_UINT16_P2:
        tyId = VIR_TYPE_UINT32;
        break;
    case VIR_TYPE_UINT16_P4:
        tyId = VIR_TYPE_UINT_X2;
        break;
    case VIR_TYPE_UINT16_P8:
        tyId = VIR_TYPE_UINT_X4;
        break;
    case VIR_TYPE_UINT16_P16:
        tyId = VIR_TYPE_UINT_X8;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;

    }

    VIR_Operand_SetType(Opnd, tyId);
    VIR_Operand_SetType(VIR_Inst_GetDest(Inst), tyId);
    /* set load/store enable too */
    _changeEnableByTyId(tyId, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}


static gctBOOL
_revise2PackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    gctUINT32  components =VIR_GetTypeComponents(tyId);
    VIR_PrimitiveTypeId format;

    gcmASSERT(!VIR_TypeId_isPacked(tyId));

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
        switch (components)
        {
        case 1:
            tyId = VIR_TYPE_INT8_P4;
            break;

        case 2:
            tyId = VIR_TYPE_INT8_P8;
            break;

        case 3:
        case 4:
            tyId = VIR_TYPE_INT8_P16;
            break;

        case 8:
        case 16:
            tyId = VIR_TYPE_INT8_P16;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_UINT8:
        switch (components)
        {
        case 1:
            tyId = VIR_TYPE_UINT8_P4;
            break;

        case 2:
            tyId = VIR_TYPE_UINT8_P8;
            break;

        case 3:
        case 4:
            tyId = VIR_TYPE_UINT8_P16;
            break;

        case 8:
        case 16:
            tyId = VIR_TYPE_UINT8_P16;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;


    case VIR_TYPE_INT16:
        switch (components)
        {
        case 1:
            tyId = VIR_TYPE_INT16_P2;
            break;

        case 2:
            tyId = VIR_TYPE_INT16_P4;
            break;

        case 3:
        case 4:
            tyId = VIR_TYPE_INT16_P8;
            break;

        case 8:
        case 16:
            tyId = VIR_TYPE_INT16_P16;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    case VIR_TYPE_UINT16:
        switch (components)
        {
        case 1:
            tyId = VIR_TYPE_UINT16_P2;
            break;

        case 2:
            tyId = VIR_TYPE_UINT16_P4;
            break;

        case 3:
        case 4:
            tyId = VIR_TYPE_UINT16_P8;
            break;

        case 8:
        case 16:
            tyId = VIR_TYPE_UINT16_P16;
            break;

        default:
            gcmASSERT(0);
            break;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetType(Opnd, tyId);

    return gcvTRUE;
}

static gctUINT32
_getSwizzleOperandValue(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *SwizzleOpnd,
    IN gctINT             Upper
)
{
    gctUINT32 swizzle = 0;
    VIR_OperandKind operandKind;

    operandKind = VIR_Operand_GetOpKind(SwizzleOpnd);
    switch(operandKind)
    {
    case VIR_OPND_IMMEDIATE:
        if(!Upper) swizzle = VIR_Operand_GetImmediateUint(SwizzleOpnd);
        break;

    case VIR_OPND_CONST:
    case VIR_OPND_SYMBOL:
        {
            VIR_ConstId   constId = VIR_INVALID_ID;

            if(operandKind == VIR_OPND_SYMBOL) {
                VIR_Symbol    *sym;
                VIR_Uniform   *uniform;

                sym = VIR_Operand_GetSymbol(SwizzleOpnd);
                gcmASSERT(VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED));
                uniform = VIR_Symbol_GetUniform(sym);
                gcmASSERT(uniform);
                constId = VIR_Uniform_GetInitializer(uniform);
            }
            else {
                constId = VIR_Operand_GetConstId(SwizzleOpnd);
            }
            if(constId == VIR_INVALID_ID) {
                gcmASSERT(0);
            }
            else {
                VIR_Const   *vConst = gcvNULL;
                VIR_Type *type;

                vConst = VIR_Shader_GetConstFromId(Context->shader,
                                                   constId);

                type = VIR_Shader_GetTypeFromId(Context->shader, vConst->type);
                if(VIR_Type_isScalar(type) && !Upper) {
                    swizzle = vConst->value.scalarVal.uValue;
                }
                else {
                    swizzle = vConst->value.vecVal.u32Value[Upper];
                }
            }
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    return swizzle;
}

gctBOOL
_checkToSetFullDefFlag(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *MaskValueOperand
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId  typeId;
    gctUINT components;
    VIR_PrimitiveTypeId  format;
    gctUINT fullEnable[17] = { 0x0,
                               0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF,
                               0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

    typeId = VIR_Operand_GetType(dest);
    if (!VIR_TypeId_isPacked(typeId))
    {
        return gcvFALSE;
    }
    components = VIR_GetTypePackedComponents(typeId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
    case VIR_TYPE_FLOAT16:
        components <<= 1;
        break;

    default:
        break;
    }

    gcmASSERT(components <= 16);
    if(components > 16) return gcvFALSE;
    if(fullEnable[components] == VIR_Operand_GetImmediateUint(MaskValueOperand)) {
        VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    }
    return gcvTRUE;
}

gctBOOL
_lowerSwizzleLowerEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *SwizzleOpnd
    )
{
    /* src2 of instruction is enable */
    VIR_Instruction *swizzleInst = VIR_Inst_GetNext(Inst);
    VIR_TypeId  typeId;
    gctUINT i, components;
    VIR_Operand *enableOpnd;
    gctUINT swizzle, enable;
    gctUINT curSwizzle, curEnable;
    gctUINT lowerEnable = 0, lowerSwizzle = 0;
    gctUINT enableMask = 0x3;
    VIR_ScalarConstVal constVal;

    typeId = VIR_Operand_GetType(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      0);
    typeId = VIR_Operand_GetType(VIR_Inst_GetDest(swizzleInst));
    components = VIR_GetTypePackedComponents(typeId);

    for(i = 0; i < components;  i++) {
        curEnable = enable & enableMask;
        if(curEnable) {
            curSwizzle = swizzle & 0xF;
            if(curSwizzle < 8) {
                lowerSwizzle |= (curSwizzle << (i * 4));
                lowerEnable |= curEnable;
            }
        }
        swizzle >>= 4;
        enableMask <<= 2;
    }

    constVal.uValue = lowerSwizzle;
    VIR_Operand_SetImmediate(SwizzleOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    constVal.uValue = lowerEnable;
    VIR_Operand_SetImmediate(enableOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    return _checkToSetFullDefFlag(Context,
                                  Inst,
                                  enableOpnd);
}

gctBOOL
_lowerSwizzleNextRegLowerEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *SwizzleOpnd
    )
{
    /* src2 of instruction is enable */
    VIR_Instruction *swizzleInst = VIR_Inst_GetNext(Inst);
    VIR_TypeId  typeId;
    gctUINT i, components;
    VIR_Operand *enableOpnd;
    gctUINT swizzle, enable;
    gctUINT curSwizzle, curEnable;
    gctUINT lowerEnable = 0, lowerSwizzle = 0;
    gctUINT enableMask = 0x3;
    VIR_ScalarConstVal constVal;

    typeId = VIR_Operand_GetType(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      0);
    typeId = VIR_Operand_GetType(VIR_Inst_GetDest(swizzleInst));
    components = VIR_GetTypePackedComponents(typeId);

    for(i = 0; i < components;  i++) {
        curEnable = enable & enableMask;
        if(curEnable) {
            curSwizzle = swizzle & 0xF;
            if(curSwizzle > 7) {
                curSwizzle = curSwizzle - 8;
                lowerSwizzle |= (curSwizzle << (i * 4));
                lowerEnable |= curEnable;
            }
        }
        swizzle >>= 4;
        enableMask <<= 2;
    }

    constVal.uValue = lowerSwizzle;
    VIR_Operand_SetImmediate(SwizzleOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    constVal.uValue = lowerEnable;
    VIR_Operand_SetImmediate(enableOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    return _checkToSetFullDefFlag(Context,
                                  Inst,
                                  enableOpnd);
}

gctBOOL
_upperSwizzleUpperEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *SwizzleOpnd
    )
{
    /* src2 of instruction is enable */
    VIR_Instruction *swizzleInst = VIR_Inst_GetNext(Inst);
    VIR_TypeId  typeId;
    gctUINT i, components;
    VIR_Operand *enableOpnd;
    gctUINT swizzle, enable;
    gctUINT curSwizzle, curEnable;
    gctUINT upperEnable = 0, upperSwizzle = 0;
    gctUINT enableMask = 0x3;
    VIR_ScalarConstVal constVal;

    typeId = VIR_Operand_GetType(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = (VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF0000) >> 16;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      1);
    typeId = VIR_Operand_GetType(VIR_Inst_GetDest(swizzleInst));
    components = VIR_GetTypePackedComponents(typeId);

    for(i = 0; i < components;  i++) {
        curEnable = enable & enableMask;
        if(curEnable) {
            curSwizzle = swizzle & 0xF;
            if(curSwizzle < 8) {
                upperSwizzle |= (curSwizzle << (i * 4));
                upperEnable |= curEnable;
            }
        }
        swizzle >>= 4;
        enableMask <<= 2;
    }

    constVal.uValue = upperSwizzle;
    VIR_Operand_SetImmediate(SwizzleOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    constVal.uValue = upperEnable;
    VIR_Operand_SetImmediate(enableOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    return _checkToSetFullDefFlag(Context,
                                  Inst,
                                  enableOpnd);
}

gctBOOL
_upperSwizzleNextRegUpperEnable(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *SwizzleOpnd
    )
{
    /* src2 of instruction is enable */
    VIR_Instruction *swizzleInst = VIR_Inst_GetNext(Inst);
    VIR_TypeId  typeId;
    gctUINT i, components;
    VIR_Operand *enableOpnd;
    gctUINT swizzle, enable;
    gctUINT curSwizzle, curEnable;
    gctUINT upperEnable = 0, upperSwizzle = 0;
    gctUINT enableMask = 0x3;
    VIR_ScalarConstVal constVal;

    typeId = VIR_Operand_GetType(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = (VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF0000) >> 16;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      1);
    typeId = VIR_Operand_GetType(VIR_Inst_GetDest(swizzleInst));
    components = VIR_GetTypePackedComponents(typeId);

    for(i = 0; i < components;  i++) {
        curEnable = enable & enableMask;
        if(curEnable) {
            curSwizzle = swizzle & 0xF;
            if(curSwizzle > 7) {
                curSwizzle = curSwizzle - 8;
                upperSwizzle |= (curSwizzle << (i * 4));
                upperEnable |= curEnable;
            }
        }
        swizzle >>= 4;
        enableMask <<= 2;
    }

    constVal.uValue = upperSwizzle;
    VIR_Operand_SetImmediate(SwizzleOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    constVal.uValue = upperEnable;
    VIR_Operand_SetImmediate(enableOpnd,
                             VIR_TYPE_UINT32,
                             constVal);
    return _checkToSetFullDefFlag(Context,
                                  Inst,
                                  enableOpnd);
}

gctBOOL
_split32BytePackedTypeUpper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    VIR_PrimitiveTypeId  format;
    gctUINT components;

    if(VIR_Type_isScalar(VIR_Shader_GetTypeFromId(Context->shader, tyId))) return gcvTRUE;
    switch (tyId)
    {
    case VIR_TYPE_INT16_P16:
        tyId = VIR_TYPE_INT16_P8;
        break;
    case VIR_TYPE_UINT16_P16:
        tyId = VIR_TYPE_UINT16_P8;
        break;
    case VIR_TYPE_INT16_P8:
    case VIR_TYPE_UINT16_P8:
    case VIR_TYPE_FLOAT16_P8:
    case VIR_TYPE_INT8_P16:
    case VIR_TYPE_UINT8_P16:
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
                break;

            case VIR_ENABLE_ZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
                break;

            case VIR_ENABLE_XYZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_ZW);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_ZW);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
                break;

            case VIR_SWIZZLE_ZWWW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
                break;

            case VIR_SWIZZLE_XYZW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZWWW);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZWWW);
                break;
            }
        }
        break;
    case VIR_TYPE_INT8_P8:
    case VIR_TYPE_UINT8_P8:
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetSwizzle(Opnd, VIR_ENABLE_Y);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_ENABLE_Y);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
                break;
            }
        }
        break;
    case VIR_TYPE_FLOAT16_P16:
        tyId = VIR_TYPE_FLOAT16_P8;
        break;
    default:
        gcmASSERT(VIR_TypeId_isPrimitive(tyId));
        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
        components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd));
        components >>= 1;
        gcmASSERT(components);
        if(components == 0) components = 1;

        tyId =  VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
                break;

            case VIR_ENABLE_YZ:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
                break;

            case VIR_ENABLE_ZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);
                break;

            case VIR_ENABLE_XYZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_ZW);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_ZW);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
                break;

            case VIR_SWIZZLE_YZZZ:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
                break;

            case VIR_SWIZZLE_ZWWW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_WWWW);
                break;

            case VIR_SWIZZLE_XYZW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZWWW);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZWWW);
                break;
            }
        }
        break;
    }

    VIR_Operand_SetType(Opnd, tyId);

    return gcvTRUE;
}

gctBOOL
_split32BytePackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);
    VIR_PrimitiveTypeId  format;
    gctUINT components;

    if(VIR_Type_isScalar(VIR_Shader_GetTypeFromId(Context->shader, tyId))) return gcvTRUE;
    switch (tyId)
    {
    case VIR_TYPE_INT16_P16:
        tyId = VIR_TYPE_INT16_P8;
        break;
    case VIR_TYPE_UINT16_P16:
        tyId = VIR_TYPE_UINT16_P8;
        break;

    case VIR_TYPE_INT16_P8:
    case VIR_TYPE_UINT16_P8:
    case VIR_TYPE_UINT8_P16:
    case VIR_TYPE_INT8_P16:
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
                break;

            case VIR_ENABLE_ZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
                break;

            case VIR_ENABLE_XYZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
                break;

            case VIR_SWIZZLE_ZWWW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
                break;

            case VIR_SWIZZLE_XYZW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYYY);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYYY);
                break;
            }
        }
        break;
    case VIR_TYPE_INT8_P8:
    case VIR_TYPE_UINT8_P8:
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetSwizzle(Opnd, VIR_ENABLE_X);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_ENABLE_X);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
                break;
            }
        }
        break;
    case VIR_TYPE_FLOAT16_P16:
        tyId = VIR_TYPE_FLOAT16_P8;
        break;
    default:
        gcmASSERT(VIR_TypeId_isPrimitive(tyId));
        gcmASSERT(VIR_TypeId_isPrimitive(tyId));
        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
        components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd));
        components >>= 1;
        gcmASSERT(components);
        if(components == 0) components = 1;

        tyId =  VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
        if(VIR_Operand_isLvalue(Opnd)) {
            switch(VIR_Operand_GetEnable(Opnd)) {
            case VIR_ENABLE_XY:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
                break;

            case VIR_ENABLE_YZ:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
                break;

            case VIR_ENABLE_ZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);
                break;

            case VIR_ENABLE_XYZW:
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
                break;
            }
        }
        else {
            switch(VIR_Operand_GetSwizzle(Opnd)) {
            case VIR_SWIZZLE_XYYY:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XXXX);
                break;

            case VIR_SWIZZLE_YZZZ:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);
                break;

            case VIR_SWIZZLE_ZWWW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_ZZZZ);
                break;

            case VIR_SWIZZLE_XYZW:
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYYY);
                break;

            default:
                gcmASSERT(0);
                VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_XYYY);
                break;
            }
        }
        break;
    }

    VIR_Operand_SetType(Opnd, tyId);

    return gcvTRUE;
}

gctBOOL
_split32BytePackedTypeDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Dest
    )
{
    if(_split32BytePackedType(Context,
                              Inst,
                              Dest)) {
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetType(Dest));
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL
_split32BytePackedTypeDestUpper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Dest
    )
{
    if(_split32BytePackedTypeUpper(Context,
                                   Inst,
                                   Dest)) {
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetType(Dest));
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL
_split32BytePackedTypeAndNextReg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);

    if(VIR_Type_isScalar(VIR_Shader_GetTypeFromId(Context->shader, tyId))) return gcvTRUE;
    if(_split32BytePackedTypeUpper(Context,
                                   Inst,
                                   Opnd) &&
       VIR_GetTypeSize(tyId) > 16) {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        VIR_Symbol    *sym = VIR_Operand_GetSymbol(Opnd);
        VIR_VirRegId  regId;
        VIR_SymId     symId;
        gctUINT       indexOffset;

        switch(VIR_Symbol_GetKind(sym)) {
        case VIR_SYM_CONST:
            break;

        case VIR_SYM_UNIFORM:
            indexOffset = VIR_Operand_GetConstIndexingImmed(Opnd) + 1;
            VIR_Operand_SetRelIndexingImmed(Opnd, indexOffset);
            break;

        default:
            regId = VIR_Symbol_GetOffsetTempIndex(sym, 1);
            virErrCode = VIR_Shader_GetVirRegSymByVirRegId(Context->shader,
                                                           regId,
                                                           &symId);
            if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
            if(symId == VIR_INVALID_ID) {
                virErrCode = VIR_Shader_AddSymbol(Context->shader,
                                                  VIR_SYM_VIRREG,
                                                  regId,
                                                  VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_UNKNOWN),
                                                  VIR_STORAGE_UNKNOWN,
                                                  &symId);
                if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
            }

            VIR_Operand_SetTempRegister(Opnd,
                                        VIR_Inst_GetFunction(Inst),
                                        symId,
                                        VIR_Operand_GetType(Opnd));
            break;
        }
    }
    return gcvFALSE;
}

gctBOOL
_split32BytePackedTypeDestAndNextReg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Dest
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Dest);

    if(_split32BytePackedTypeDestUpper(Context,
                                       Inst,
                                       Dest) &&
       VIR_GetTypeSize(tyId) > 16) {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        VIR_Symbol    *sym = VIR_Operand_GetSymbol(Dest);
        VIR_VirRegId  regId;
        VIR_SymId     symId;

        gcmASSERT(VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG);
        regId = VIR_Symbol_GetOffsetTempIndex(sym, 1);
        virErrCode = VIR_Shader_GetVirRegSymByVirRegId(Context->shader,
                                                       regId,
                                                       &symId);
        if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
        if(symId == VIR_INVALID_ID) {
            virErrCode = VIR_Shader_AddSymbol(Context->shader,
                                              VIR_SYM_VIRREG,
                                              regId,
                                              VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_UNKNOWN),
                                              VIR_STORAGE_UNKNOWN,
                                              &symId);
            if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
        }

        VIR_Operand_SetTempRegister(Dest,
                                    VIR_Inst_GetFunction(Inst),
                                    symId,
                                    VIR_Operand_GetType(Dest));
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetType(Dest));
    }
    return gcvTRUE;
}

gctBOOL
_split32BytePackedType2NonpackedTypeForLoadStore(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetType(Opnd);

    switch (tyId)
    {
    case VIR_TYPE_INT16_P16:
        tyId = VIR_TYPE_INTEGER_X4;
        break;
    case VIR_TYPE_UINT16_P16:
        tyId = VIR_TYPE_UINT_X4;
        break;
    case VIR_TYPE_FLOAT16_P16:
        tyId = VIR_TYPE_FLOAT16_X4;
        break;
    default:
        gcmASSERT(gcvFALSE);
        return gcvFALSE;
        break;
    }

    VIR_Operand_SetType(VIR_Inst_GetDest(Inst), tyId);
    /* set load/store enable too */
    _changeEnableByTyId(tyId, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}

gctBOOL
_split32BytePackedType2NonpackedTypeForLoadAndNextReg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(_split32BytePackedType2NonpackedTypeForLoadStore(Context,
                                                        Inst,
                                                        Opnd)) {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        VIR_Operand   *dest = VIR_Inst_GetDest(Inst);
        VIR_TypeId    operandType = VIR_Operand_GetType(dest);
        VIR_Symbol    *sym = VIR_Operand_GetSymbol(dest);
        VIR_VirRegId  regId = VIR_Symbol_GetOffsetTempIndex(sym, 1);
        VIR_SymId     symId;

        virErrCode = VIR_Shader_GetVirRegSymByVirRegId(Context->shader,
                                                       regId,
                                                       &symId);
        if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
        if(symId == VIR_INVALID_ID) {
            virErrCode = VIR_Shader_AddSymbol(Context->shader,
                                              VIR_SYM_VIRREG,
                                              regId,
                                              VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_UNKNOWN),
                                              VIR_STORAGE_UNKNOWN,
                                              &symId);
            if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
        }

        VIR_Operand_SetTempRegister(Opnd,
                                    VIR_Inst_GetFunction(Inst),
                                    symId,
                                    operandType);
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL
_split32BytePackedTypeToNonpackedTypeForStore(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(_split32BytePackedType2NonpackedTypeForLoadStore(Context,
                                                        Inst,
                                                        Opnd)) {
        return _split32BytePackedType(Context,
                                      Inst,
                                      Opnd);
    }

    return gcvFALSE;
}

gctBOOL
_split32BytePackedTypeToNonpackedTypeForStoreAndNextReg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(_split32BytePackedType2NonpackedTypeForLoadStore(Context,
                                                        Inst,
                                                        Opnd)) {
        return _split32BytePackedTypeAndNextReg(Context,
                                                Inst,
                                                Opnd);
    }
    return gcvFALSE;
}

static gctBOOL
_revise2UnPackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId;
    VIR_PrimitiveTypeId  format;
    gctUINT components;
    gctBOOL adjustFormat;

    tyId = VIR_Operand_GetType(Opnd);
    gcmASSERT(VIR_TypeId_isPacked(tyId));

    components = VIR_GetTypePackedComponents(tyId);

    adjustFormat = ((VIR_PatternLowerContext *)Context)->isCL_X &&
                   !((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.hasBugFix11;

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch (format)
    {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        components >>= 2;
        gcmASSERT(components <= 4);
        if(components > 4) components = 4;
        if(adjustFormat)
        {
           if(format == VIR_TYPE_INT8) format = VIR_TYPE_INT32;
           else format = VIR_TYPE_UINT32;
        }
        break;

    case VIR_TYPE_INT16:
    case VIR_TYPE_UINT16:
        components >>= 1;
        gcmASSERT(components <= 4);
        if(components > 4) components = 4;
        if(adjustFormat)
        {
           if(format == VIR_TYPE_INT16) format = VIR_TYPE_INT32;
           else format = VIR_TYPE_UINT32;
        }
        break;

    default:
        gcmASSERT(0);
        return gcvFALSE;
    }

    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(format, components, 1));
    VIR_Operand_SetSwizzle(Opnd, VIR_TypeId_Conv2Swizzle(tyId));

    return gcvTRUE;
}

gctBOOL
_change2NonpackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId;
    VIR_Instruction* prevInst;
    VIR_Swizzle      swzl = VIR_SWIZZLE_XXXX;

    prevInst= VIR_Inst_GetPrev(Inst);
    tyId = VIR_Operand_GetType(VIR_Inst_GetSource(prevInst, 0));

    switch (tyId)
    {
    /* packed vector3 types */
    case VIR_TYPE_INT8_P3:
        tyId = VIR_TYPE_INT8_X3;
        swzl = VIR_SWIZZLE_XYZZ;
        break;
    case VIR_TYPE_UINT8_P3:
        tyId = VIR_TYPE_UINT8_X3;
        swzl = VIR_SWIZZLE_XYZZ;
        break;
    case VIR_TYPE_INT16_P3:
        tyId = VIR_TYPE_INT16_X3;
        swzl = VIR_SWIZZLE_XYZZ;
        break;
    case VIR_TYPE_UINT16_P3:
        tyId = VIR_TYPE_UINT16_X3;
        swzl = VIR_SWIZZLE_XYZZ;
        break;

    /* other packed types */
    case VIR_TYPE_INT8_P2:
        tyId = VIR_TYPE_INT16;
        swzl = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_INT8_P4:
        tyId = VIR_TYPE_INT32;
        swzl = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_INT8_P8:
        tyId = VIR_TYPE_INTEGER_X2;
        swzl = VIR_SWIZZLE_XYYY;
        break;
    case VIR_TYPE_INT8_P16:
        tyId = VIR_TYPE_INTEGER_X4;
        swzl = VIR_SWIZZLE_XYZW;
        break;
    case VIR_TYPE_UINT8_P2:
        tyId = VIR_TYPE_UINT16;
        break;
    case VIR_TYPE_UINT8_P4:
        tyId = VIR_TYPE_UINT32;
        swzl = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_UINT8_P8:
        tyId = VIR_TYPE_UINT_X2;
        swzl = VIR_SWIZZLE_XYYY;
        break;
    case VIR_TYPE_UINT8_P16:
        tyId = VIR_TYPE_UINT_X4;
        swzl = VIR_SWIZZLE_XYZW;
        break;

    case VIR_TYPE_INT16_P2:
        tyId = VIR_TYPE_INT32;
        swzl = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_INT16_P4:
        tyId = VIR_TYPE_INTEGER_X2;
        swzl = VIR_SWIZZLE_XYYY;
        break;
    case VIR_TYPE_INT16_P8:
        tyId = VIR_TYPE_INTEGER_X4;
        swzl = VIR_SWIZZLE_XYZW;
        break;
    case VIR_TYPE_UINT16_P2:
        tyId = VIR_TYPE_UINT32;
        swzl = VIR_SWIZZLE_XXXX;
        break;
    case VIR_TYPE_UINT16_P4:
        tyId = VIR_TYPE_UINT_X2;
        swzl = VIR_SWIZZLE_XYYY;
        break;
    case VIR_TYPE_UINT16_P8:
        tyId = VIR_TYPE_UINT_X4;
        swzl = VIR_SWIZZLE_XYZW;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;

    }

    VIR_Operand_SetType(Opnd, tyId);
    VIR_Operand_SetSwizzle(Opnd, swzl);
    return gcvTRUE;
}

gctBOOL
_changeEnableBySrc0Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId;
    VIR_Operand * src0;
    gcmASSERT (Opnd != gcvNULL);

    gcmASSERT(VIR_Operand_isLvalue(Opnd));
    src0 = VIR_Inst_GetSource(Inst, 0);
    tyId = VIR_Operand_GetType(src0);
    switch (tyId)
    {
    case VIR_TYPE_INT8_P3:
        tyId = VIR_TYPE_INT8_X3;
        break;
    case VIR_TYPE_UINT8_P3:
        tyId = VIR_TYPE_UINT8_X3;
        break;
    case VIR_TYPE_INT16_P3:
        tyId = VIR_TYPE_INT16_X3;
        break;
    case VIR_TYPE_UINT16_P3:
        tyId = VIR_TYPE_UINT16_X3;
        break;

    /* other packed types */
    case VIR_TYPE_INT8_P2:
        tyId = VIR_TYPE_INT16;
        break;
    case VIR_TYPE_INT8_P4:
        tyId = VIR_TYPE_INT32;
        break;
    case VIR_TYPE_INT8_P8:
        tyId = VIR_TYPE_INTEGER_X2;
        break;
    case VIR_TYPE_INT8_P16:
        tyId = VIR_TYPE_INTEGER_X4;
        break;
    case VIR_TYPE_UINT8_P2:
        tyId = VIR_TYPE_UINT16;
        break;
    case VIR_TYPE_UINT8_P4:
        tyId = VIR_TYPE_UINT32;
        break;
    case VIR_TYPE_UINT8_P8:
        tyId = VIR_TYPE_UINT_X2;
        break;
    case VIR_TYPE_UINT8_P16:
        tyId = VIR_TYPE_UINT_X4;
        break;

    case VIR_TYPE_INT16_P2:
        tyId = VIR_TYPE_INT32;
        break;
    case VIR_TYPE_INT16_P4:
        tyId = VIR_TYPE_INTEGER_X2;
        break;
    case VIR_TYPE_INT16_P8:
        tyId = VIR_TYPE_INTEGER_X4;
        break;
    case VIR_TYPE_UINT16_P2:
        tyId = VIR_TYPE_UINT32;
        break;
    case VIR_TYPE_UINT16_P4:
        tyId = VIR_TYPE_UINT_X2;
        break;
    case VIR_TYPE_UINT16_P8:
        tyId = VIR_TYPE_UINT_X4;
        break;    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    VIR_Operand_SetType(Opnd, tyId);
    _changeEnableByTyId(tyId, Opnd);

    return gcvTRUE;
}

gctBOOL
_changeDestEnableByTypeId(
    IN VIR_Instruction    *Inst,
    IN VIR_TypeId          TyId
    )
{
    _changeEnableByTyId(TyId, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}

gctBOOL
_changeEnableByPrevStmtDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand * prevDest;
    VIR_Instruction* prevInst;

    gcmASSERT(VIR_Operand_isLvalue(Opnd));

    prevInst= VIR_Inst_GetPrev(Inst);
    prevDest =VIR_Inst_GetDest(prevInst);

    VIR_Operand_SetEnable(Opnd, VIR_Operand_GetEnable(prevDest));
    VIR_Operand_SetType(Opnd, VIR_Operand_GetType(prevDest));

    return gcvTRUE;
}
/*
 * load dest.char_p3, base, offset         // packed char3 load
 *   load temp.char3.xyz, base, offset    // unpacked char3 load
 *   swizzle dest.char_p3, temp.char3.xyz, swizzle_setting, 0x007
 */
static VIR_PatternMatchInst _loadPatInst9[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destChar_P3Type }, VIR_PATN_MATCH_FLAG_OR },
};


static VIR_PatternReplaceInst _loadRepInst9[] = {
    { VIR_OP_LOAD, 0, 0, { -1, 2, 3, 0 }, { _change2NonpackedTypeForLoadStore } },
    { VIR_OP_SWIZZLE, 0, 0, {  1, -1, 3, 3 }, { 0, 0, _setChar_P3Swizzle, _setChar_P3MaskValue } },
};

/*
 * load dest.short_p3, base, offset         // packed short3 load
 *   load temp.short3.xyz, base, offset     // unpacked short3 load
 *   swizzle dest.short_p3, temp.short3.xyz, swizzle_setting, 0x03F
 */
static VIR_PatternMatchInst _loadPatInst10[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destShort_P3Type }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst10[] = {
    { VIR_OP_LOAD, 0, 0, {  -1, 2, 3, 0 }, { _change2NonpackedTypeForLoadStore } },
    { VIR_OP_SWIZZLE, 0, 0, {  1, -1, 3, 3 }, { 0, 0, _setShort_P3Swizzle, _setShort_P3MaskValue } },
};

/*
 * load dest.packed_type, base, offset           // packed type load of > 16 bytes: such as short_P16 ...
 *   load dest.non_packed_type, base, offset     // unpacked type load of lower half
 *   add t1, offset + 16
 *   load dest.non_packed_type, base, t1         // unpacked type load of upper half
 */
static VIR_PatternMatchInst _loadPatInst11[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destPackedGT16Bytes }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst11[] = {
    { VIR_OP_LOAD, 0, 0, {  1, 2, 3, 0 }, { _split32BytePackedType2NonpackedTypeForLoadStore } },
    { VIR_OP_ADD, 0, 0, {  -1, 3, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_LOAD, 0, 0, {  1, 2, -1, 0 }, { _split32BytePackedType2NonpackedTypeForLoadAndNextReg } },
};

/*
 * load dest.packed_type, base, offset           // non vector 3 packed type load
 *   load dest.non_packed_type, base, offset     // unpacked type load
 */
static VIR_PatternMatchInst _loadPatInst12[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destPackedType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst12[] = {
    { VIR_OP_LOAD, 0, 0, {  1, 2, 3, 0 }, { _change2NonpackedTypeForLoadStore } },
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 10) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 11) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 12) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_src2Char_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_TypeId tyId = VIR_Operand_GetType(src2);

    return tyId == VIR_TYPE_INT8_P3 || tyId == VIR_TYPE_UINT8_P3;
}

static gctBOOL
_src0Char_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return tyId == VIR_TYPE_INT8_P3 || tyId == VIR_TYPE_UINT8_P3;
}

static gctBOOL
_src0Short_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetType(src);

    return tyId == VIR_TYPE_INT16_P3 || tyId == VIR_TYPE_UINT16_P3;
}

static gctBOOL
_setSrcChar_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(Opnd))) {
        VIR_Operand_SetType(Opnd, VIR_TYPE_INT8_P3);
    }
    else {
        VIR_Operand_SetType(Opnd, VIR_TYPE_UINT8_P3);
    }
    return gcvTRUE;
}

static gctBOOL
_setSrcShort_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetType(Opnd))) {
        VIR_Operand_SetType(Opnd, VIR_TYPE_INT16_P3);
    }
    else {
        VIR_Operand_SetType(Opnd, VIR_TYPE_UINT16_P3);
    }
    return gcvTRUE;
}

gctBOOL
_setChar3MaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x0111;  /* bytes 0, 4, 8 */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setChar3UnpackSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctINT             byteSwzl[2] = {0x00010000, 0x00000002}; /* little endian */
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_INTEGER_X2;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.i8Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

static gctBOOL
_src2Short_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_TypeId tyId = VIR_Operand_GetType(src2);

    return tyId == VIR_TYPE_INT16_P3 || tyId == VIR_TYPE_UINT16_P3;
}

gctBOOL
_setShort3MaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x0333;  /* word 0, 2, 4 */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setShort3Swizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.iValue = 0x00020100;   /* word 0 -> word 0; word 1 -> word 2; word 2 -> word 4; */

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);
    return gcvTRUE;
}

static gctBOOL
_src2PackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_TypeId tyId = VIR_Operand_GetType(src2);

    return VIR_TypeId_isPacked(tyId);
}

/*
 * store [dest.char_p3], base, offset, val.char_p3         // packed char3 store
 *   swizzle temp.char3, val.char_p3, swizzle_setting, 0x0111
 *   store [dest.char_p3], base, offset, temp.char3        // unpacked char3 store
 */
static VIR_PatternMatchInst _storePatInst0[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _src2Char_P3Type }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst0[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 4, 3, 3 }, { _changeEnableBySrc0Type, 0, _setChar3UnpackSwizzle, _setChar3MaskValue } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, -1 }, { _changeEnableByPrevStmtDest, 0, 0, _change2NonpackedTypeAndSwizzle } },
};

/*
 * store dest.short_p3, base, offset         // packed short3 store
 *   store temp.short3.xyz, base, offset     // unpacked short3 store
 *   swizzle dest.short_p3, temp.short3.xyz, swizzle_setting, 0x0333
 */
static VIR_PatternMatchInst _storePatInst1[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _src2Short_P3Type }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst1[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 4, 3, 3 }, { _changeEnableBySrc0Type, 0, _setShort3Swizzle, _setShort3MaskValue } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, -1 }, { _changeEnableByPrevStmtDest, 0, 0, _change2NonpackedTypeAndSwizzle } },
};

/*
 * store dest.packed_type, base, offset          // packed type store of > 16 bytes: such as short_P16 ...
 *   store dest.non_packed_type, base, offset     // unpacked type store of lower half
 *   add t1, offset + 16
 *   store dest.non_packed_type, base, t1         // unpacked type store of upper half
 */
static VIR_PatternMatchInst _storePatInst2[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _src2PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst2[] = {
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _split32BytePackedTypeToNonpackedTypeForStore } },
    { VIR_OP_ADD, 0, 0, {  -1, 3, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -1, 4 }, { 0, 0, 0, _split32BytePackedTypeToNonpackedTypeForStoreAndNextReg } },
};

/*
 * store dest.packed_type, base, offset           // other packed type store
 *   store dest.non_packed_type, base, offset     // unpacked type store
 */
static VIR_PatternMatchInst _storePatInst3[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _src2PackedType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst3[] = {
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _change2NonpackedTypeForLoadStore } },
};

static VIR_Pattern _storePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 3) },
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

static VIR_PatternMatchInst _anyPatInst0[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _anyRepInst0[] = {
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, { -1, 2, 0, 0 }, { int_value_type0, _split32BytePackedType, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, { -2, 2, 0, 0 }, { int_value_type0, _split32BytePackedTypeAndNextReg, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _anyPatInst1[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Char_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _anyRepInst1[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { 0, int_value_type0_src_const_0xFFFFFF } },
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcChar_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _anyPatInst2[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Short_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _anyRepInst2[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 2, 2, 2 }, { 0, _setDestShort_P4TypeFromSrc, _setDefaultPackedSwizzle, _setShort_P4MaskValue } },
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcShort_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _anyPatInst3[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _anyRepInst3[] = {
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, { 1, 2, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};


static VIR_PatternMatchInst _anyPatInst4[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _anyRepInst4[] = {
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, {  -1, 2, 0, 0 }, { VIR_Lower_SetEnableBaseOnSrc0, 0, VIR_Lower_SetIntHighBitOne, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_ANYMSB, 0, {   1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _anyPattern[] = {
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_any, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_any, 1) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_any, 2) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_any, 3) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_any, 4) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _allPatInst0[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _allRepInst0[] = {
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, { -1, 2, 0, 0 }, { int_value_type0, _split32BytePackedType, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, { -2, 2, 0, 0 }, { int_value_type0, _split32BytePackedTypeAndNextReg, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_AND_BITWISE, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _allPatInst1[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Char_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _allRepInst1[] = {
    { VIR_OP_OR_BITWISE, 0, 0, { -1, 2, 0, 0 }, { 0, int_value_type0_src_const_0xFF000000 } },
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcChar_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst2[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Short_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _allRepInst2[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 2, 2, 2 }, { 0, _setDestShort_P4TypeFromSrc, _setDefaultPackedSwizzle, _setShort_P4MaskValue } },
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcShort_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst3[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _allRepInst3[] = {
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, {   1, 2, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst4[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _allRepInst4[] = {
    { VIR_OP_SELECT, VIR_COP_NOT_ZERO, 0, {  -1, 2, 0, 0 }, { VIR_Lower_SetEnableBaseOnSrc0, 0, VIR_Lower_SetIntHighBitOne, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_ALLMSB, 0, {   1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_Pattern _allPattern[] = {
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_all, 0) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_all, 1) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_all, 2) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_all, 3) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_all, 4) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _swizzlePatInst0[] = {
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _destPackedLE16Bytes, _src0PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _swizzleRepInst0[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { 0, _split32BytePackedType, _lowerSwizzleLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { 0, _split32BytePackedTypeAndNextReg,_lowerSwizzleNextRegLowerEnable } },
};

static VIR_PatternMatchInst _swizzlePatInst1[] = {
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _destPackedGT16Bytes, _src0ScalarOrPackedLE16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _swizzleRepInst1[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDest, _split32BytePackedType, _lowerSwizzleLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDestAndNextReg, _split32BytePackedType, _upperSwizzleUpperEnable } },
};

static VIR_PatternMatchInst _swizzlePatInst2[] = {
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _destPackedGT16Bytes, _src0PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _swizzleRepInst2[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDest, _split32BytePackedType, _lowerSwizzleLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDestAndNextReg, _split32BytePackedType, _upperSwizzleUpperEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDest, _split32BytePackedTypeAndNextReg, _lowerSwizzleNextRegLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDestAndNextReg, _split32BytePackedTypeAndNextReg, _upperSwizzleNextRegUpperEnable } },
};

static VIR_PatternMatchInst _swizzlePatInst3[] = {
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _swizzleRepInst3[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { 0, 0, 0, _checkToSetFullDefFlag } },
};

static VIR_Pattern _swizzlePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_swizzle, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_swizzle, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_swizzle, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_swizzle, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _bitCastPatInst0[] = {
    { VIR_OP_BITCAST, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitCastRepInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 } },
};

static VIR_Pattern _bitCastPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitCast, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
** REM, temp(1), temp(2), temp(3)
** -->
** DIV, temp(4), temp(2), temp(3)
** FIX, temp(4), temp(4)
** MAD, temp(1), -temp(3),temp(4), temp(2)
*/
static VIR_PatternMatchInst _remPatInst0[] = {
    { VIR_OP_REM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstFloat }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _remRepInst0[] = {
    { VIR_OP_DIV, 0, 0, { -1, 2, 3, 0 }, { 0 } },
    { VIR_OP_FIX, 0, 0, { -1, -1, 0, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { 1, 3, -1, 2 }, { 0, VIR_Lower_SetOpndNeg, 0, 0 } },
};

static VIR_PatternMatchInst _remPatInst1[] = {
    { VIR_OP_REM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _remRepInst1[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _remPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_rem, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_rem, 1) },
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
gctBOOL _componentX(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))) == 1);
}

static
gctBOOL _componentXY(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))) == 2);
}

static
gctBOOL _componentXYZ(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))) == 3);
}

static
gctBOOL _componentXYZW(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    return (VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetSource(Inst, 0))) == 4);
}

static VIR_PatternMatchInst _dotPatInst0[] = {
    { VIR_OP_DOT, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 0, 0}, { _componentX }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _dotRepInst0[] = {
    { VIR_OP_MUL, 0, 0, {  1, 2, 3 ,0 }, {0 } },
};

static VIR_PatternMatchInst _dotPatInst1[] = {
    { VIR_OP_DOT, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 0, 0}, { _componentXY }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _dotRepInst1[] = {
    { VIR_OP_DP2, 0, 0, {  1, 2, 3 ,0 }, {0 } },
};

static VIR_PatternMatchInst _dotPatInst2[] = {
    { VIR_OP_DOT, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 0, 0}, { _componentXYZ }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _dotRepInst2[] = {
    { VIR_OP_DP3, 0, 0, {  1, 2, 3 ,0 }, {0 } },
};

static VIR_PatternMatchInst _dotPatInst3[] = {
    { VIR_OP_DOT, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 0, 0}, { _componentXYZW }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _dotRepInst3[] = {
    { VIR_OP_DP4, 0, 0, {  1, 2, 3 ,0 }, {0 } },
};

static VIR_Pattern _dotPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_dot, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_dot, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_dot, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_dot, 3) },
    { VIR_PATN_FLAG_NONE }
};

gctBOOL
_SplitPackedGT16ByteInst(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VSC_ErrCode      errCode = VSC_ERR_NONE;
    VIR_Instruction  *insertedInst = gcvNULL;
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(Inst);
    gctUINT         i, srcNum = VIR_OPCODE_GetSrcOperandNum(opcode);
    VIR_Operand     *dest, *src;
    VIR_Operand     *origDest = gcvNULL, *origSrc = gcvNULL;

    errCode = VIR_Function_AddInstructionBefore(VIR_Inst_GetFunction(Inst),
                                                VIR_Inst_GetOpcode(Inst),
                                                VIR_TYPE_UNKNOWN,
                                                Inst,
                                                gcvTRUE,
                                                &insertedInst);
    if (errCode != VSC_ERR_NONE) return gcvFALSE;

    insertedInst->sourceLoc = Inst->sourceLoc;
    insertedInst->_isPatternRep = gcvTRUE;

    if (Inst->_parentUseBB)
    {
        VIR_Inst_SetBasicBlock(insertedInst, VIR_Inst_GetBasicBlock(Inst));
    }
    else
    {
        VIR_Inst_SetFunction(insertedInst, VIR_Inst_GetFunction(Inst));
    }

    VIR_Inst_SetConditionOp(insertedInst, VIR_Inst_GetConditionOp(Inst));

    VIR_Inst_SetResOpType(insertedInst, VIR_Inst_GetResOpType(Inst));

    /* handle dest operand */
    if (VIR_OPCODE_hasDest(opcode))
    {
        dest = VIR_Inst_GetDest(insertedInst);
        origDest = VIR_Inst_GetDest(Inst);
        VIR_Operand_Copy(dest, origDest);

         _split32BytePackedTypeDest(Context,
                                    insertedInst,
                                    dest);

        _split32BytePackedTypeDestAndNextReg(Context,
                                             Inst,
                                             origDest);
    }

    /* handle source operand */
    for (i = 0; i < srcNum; i++)
    {
        origSrc = VIR_Inst_GetSource(Inst, i);
        src = VIR_Inst_GetSource(insertedInst, i);
        /* copy operand */
        VIR_Operand_Copy(src, origSrc);
        _split32BytePackedType(Context,
                               insertedInst,
                               src);
        _split32BytePackedTypeAndNextReg(Context,
                                         Inst,
                                         origSrc);
    }

    return gcvTRUE;
}

static VIR_Pattern*
_GetLowerPatternPhaseExpand(
    IN  VIR_PatternContext * Context,
    IN VIR_Instruction     * Inst
    )
{
    if(VIR_Inst_isComponentwise(Inst) &&
       (_destPackedGT16Bytes(Context, Inst) ||
        (_destPackedType(Context, Inst) && _src0GT16Bytes(Context, Inst)) ||
        (_destGT16Bytes(Context, Inst) && _src0PackedType(Context, Inst)))) {
        _SplitPackedGT16ByteInst(Context, Inst);
        return _nullPattern;
    }
    else {
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
        case VIR_OP_STORE:
            return _storePattern;
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
        case VIR_OP_ANY:
            return _anyPattern;
        case VIR_OP_ALL:
            return _allPattern;
        case VIR_OP_SWIZZLE:
            return _swizzlePattern;
        case VIR_OP_BITCAST:
            return _bitCastPattern;
        case VIR_OP_REM:
            return _remPattern;
        default:
            break;
        }

        if (gcUseFullNewLinker(Context->vscContext->pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2))
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
            case VIR_OP_DOT:
                return _dotPattern;
            default:
                break;
            }
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
    IN  PVSC_CONTEXT            VscContext,
    IN  VIR_PatternLowerContext *Context
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;



    VIR_PatternContext_Initialize(&Context->header, VscContext, Shader, Context->pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                  _GetLowerPatternPhaseExpand, _CmpInstuction, 512);

    errCode = VIR_Pattern_Transform((VIR_PatternContext *)Context);
    CHECK_ERROR(errCode, "VIR_Lower_MiddleLevel_To_LowLevel_Expand failed.");

    VIR_PatternContext_Finalize(&Context->header);

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

    VIR_Symbol      *sym = VIR_Operand_GetSymbol(Opnd);
    VIR_Type        *type       = VIR_Symbol_GetType(sym);
    VIR_TypeId       typeId     = VIR_Type_GetBaseTypeId(type);
    VIR_OperandId    opndId;
    VIR_Operand     *dest = gcvNULL;
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
        gcvTRUE,
        &stArrInst);
    if(virErrCode != VSC_ERR_NONE) return virErrCode;
    /* set value */
    VIR_Operand_SetTempRegister(
        needAttrSt ? VIR_Inst_GetSource(stArrInst, 2) : VIR_Inst_GetSource(stArrInst, 1),
        Func,
        symId,
        typeId);
    VIR_Operand_SetSwizzle(needAttrSt ? VIR_Inst_GetSource(stArrInst, 2) : VIR_Inst_GetSource(stArrInst, 1),
        VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd)));
    VIR_Operand_SetPrecision(needAttrSt ? VIR_Inst_GetSource(stArrInst, 2) : VIR_Inst_GetSource(stArrInst, 1),
         VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));

    /* per patch output always index InvocationIndex 0 */
    if (VIR_Operand_GetRelAddrMode(Opnd) == VIR_INDEXED_NONE)
    {
        gctINT immIdxNo = VIR_Operand_GetMatrixConstIndex(Opnd);

        if (VIR_Operand_GetIsConstIndexing(Opnd))
        {
            immIdxNo += VIR_Operand_GetConstIndexingImmed(Opnd);
        }

        opndId = VIR_Operand_GetIndex(VIR_Inst_GetDest(stArrInst));
        *stArrInst->dest = *Opnd;
        dest = VIR_Inst_GetDest(stArrInst);
        VIR_Operand_SetIndex(dest, opndId);
        VIR_Operand_SetMatrixConstIndex(dest, 0);
        VIR_Operand_SetRelIndexingImmed(dest, 0);
        VIR_Operand_SetRelAddrMode(dest, VIR_INDEXED_NONE);
        VIR_Operand_SetIsConstIndexing(dest, 0);

        /* src[0] for ST_ARR and ArrayedPerVertex ATTR_ST, src[1] for PerPatch ATTR_ST:
         * create a new Immediate Value*/
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(stArrInst, 1) : VIR_Inst_GetSource(stArrInst, 0),
            immIdxNo);
    }
    else
    {
        opndId = VIR_Operand_GetIndex(VIR_Inst_GetDest(stArrInst));
        *stArrInst->dest = *Opnd;
        dest = VIR_Inst_GetDest(stArrInst);
        VIR_Operand_SetIndex(dest, opndId);
        VIR_Operand_SetMatrixConstIndex(dest, VIR_Operand_GetMatrixConstIndex(Opnd));
        VIR_Operand_SetRelIndexingImmed(dest, 0);
        VIR_Operand_SetRelAddrMode(dest, VIR_INDEXED_NONE);

        /* index */
        VIR_Operand_SetTempRegister(
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(stArrInst, 1) : VIR_Inst_GetSource(stArrInst, 0),
            Func,
            relSymId,
            VIR_Symbol_GetTypeId(VIR_Shader_GetSymFromId(Shader, relSymId)));
        VIR_Operand_SetSwizzle(VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(stArrInst, 1) : VIR_Inst_GetSource(stArrInst, 0),
            relAddr2Swizzle[relIndexed].swizzle);
        VIR_Operand_SetPrecision(VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(stArrInst, 1) : VIR_Inst_GetSource(stArrInst, 0),
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
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(stArrInst, 0) : VIR_Inst_GetSource(stArrInst, 1),
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
    VIR_Operand     *dest = gcvNULL;
    VIR_Operand     *src = gcvNULL;
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
        gcvTRUE,
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
    dest = VIR_Inst_GetDest(ldArrInst);
    VIR_Symbol_SetType(VIR_Function_GetSymFromId(Func, symId), VIR_Shader_GetTypeFromId(Shader, newVirRegTypeId));
    VIR_Operand_SetTempRegister(
        dest,
        Func,
        symId,
        newVirRegTypeId);
    VIR_Operand_SetEnable(dest, ldArrEnable);
    VIR_Operand_SetPrecision(dest, VIR_Operand_GetPrecision(Opnd));

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
        opndId = VIR_Operand_GetIndex(VIR_Inst_GetSource(ldArrInst, 0));
        *ldArrInst->src[0] = *Opnd;
        src = VIR_Inst_GetSource(ldArrInst, 0);
        VIR_Operand_SetSwizzle(src, src0Swizzle);
        VIR_Operand_SetIndex(src, opndId);
        if (isMatrix)
        {
            VIR_Operand_SetMatrixConstIndex(src, immIdxNo);
        }
        else
        {
            VIR_Operand_SetMatrixConstIndex(src, 0);
        }
        VIR_Operand_SetRelIndexingImmed(src, 0);
        VIR_Operand_SetRelAddrMode(src, VIR_INDEXED_NONE);
        VIR_Operand_SetIsConstIndexing(src, 0);

        /* src[1], create a new Immediate Value*/
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(ldArrInst, 2) : VIR_Inst_GetSource(ldArrInst, 1),
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
        opndId = VIR_Operand_GetIndex(VIR_Inst_GetSource(ldArrInst, 0));
        *ldArrInst->src[0] = *Opnd;
        src = VIR_Inst_GetSource(ldArrInst, 0);
        VIR_Operand_SetSwizzle(src, src0Swizzle);
        VIR_Operand_SetIndex(src, opndId);
        VIR_Operand_SetMatrixConstIndex(src, VIR_Operand_GetMatrixConstIndex(Opnd));
        VIR_Operand_SetRelIndexingImmed(src, 0);
        VIR_Operand_SetRelAddrMode(src, VIR_INDEXED_NONE);

        /* src[1] */
        VIR_Operand_SetTempRegister(
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(ldArrInst, 2) : VIR_Inst_GetSource(ldArrInst, 1),
            Func,
            relSymId,
            VIR_Symbol_GetTypeId(VIR_Shader_GetSymFromId(Shader, relSymId)));
        VIR_Operand_SetSwizzle(VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(ldArrInst, 2) : VIR_Inst_GetSource(ldArrInst, 1), relAddr2Swizzle[relIndexed].swizzle);
        VIR_Operand_SetPrecision(VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(ldArrInst, 2) : VIR_Inst_GetSource(ldArrInst, 1), VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, relSymId)));
    }
    if (sym &&
        (isSymArrayedPerVertex(sym) || VIR_Symbol_isPerPatch(sym)))
    {
        /* add immediate 0 to src[2] for ArrayedPerVertex ATTR_ST, src[1] for PerPatch ATTR_ST */
        gcmASSERT(needAttrLd);
        VIR_Operand_SetImmediateInt(
            VIR_Symbol_isPerPatch(sym) ? VIR_Inst_GetSource(ldArrInst, 1) : VIR_Inst_GetSource(ldArrInst, 2),
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

static gctBOOL
_NeedToCalculateOffset(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Operand      *pOffsetOpnd
    )
{
    gctBOOL needToCalculate = gcvTRUE;

    switch(VIR_Operand_GetOpKind(pOffsetOpnd))
    {
        case VIR_OPND_IMMEDIATE:
            if (VIR_Operand_GetImmediateInt(pOffsetOpnd) == 0)
            {
                needToCalculate = gcvFALSE;
            }
            break;

        case VIR_OPND_CONST:
            {
                VIR_Const   *vConst = gcvNULL;
                gctINT      i;

                vConst = VIR_Shader_GetConstFromId(pShader,
                                                   VIR_Operand_GetConstId(pOffsetOpnd));
                for (i = 0; i < 16; i++)
                {
                    if (vConst->value.vecVal.i32Value[i] != 0)
                    {
                        break;
                    }
                }
                if (i == 16)
                {
                    needToCalculate = gcvFALSE;
                }
            }
            break;

        default:
            break;
    }

    return needToCalculate;
}

static VSC_ErrCode
_GetSamplerInfo(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    INOUT   VIR_SymId        *LevelBaseSizeSymId,
    INOUT   VIR_SymId        *LodMinMaxSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand                *pSamplerOpnd = VIR_Inst_GetSource(pInst, 0);
    VIR_SymId                   levelBaseSizeSymId = VIR_INVALID_ID, lodMinMaxSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;

    /* Create a symbol to save levelBaseSizeSymId. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X4),
                                   VIR_STORAGE_UNKNOWN,
                                   &levelBaseSizeSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    /* Get the levelBaseSize. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_GET_SAMPLER_LBS,
                                                VIR_TYPE_INTEGER_X4,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                levelBaseSizeSymId,
                                VIR_TYPE_INTEGER_X4);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XYZW);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pSamplerOpnd);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateUint(operand, 0);

    /* Create a symbol to save lodMinMaxSymId. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X4),
                                   VIR_STORAGE_UNKNOWN,
                                   &lodMinMaxSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    /* Get the lodMinMax. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_GET_SAMPLER_LMM,
                                                VIR_TYPE_INTEGER_X4,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                lodMinMaxSymId,
                                VIR_TYPE_INTEGER_X4);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XYZW);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pSamplerOpnd);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateUint(operand, 0);

    /* Save the result. */
    if (LevelBaseSizeSymId)
    {
        *LevelBaseSizeSymId = levelBaseSizeSymId;
    }
    if (LodMinMaxSymId)
    {
        *LodMinMaxSymId = lodMinMaxSymId;
    }

OnError:
    return errCode;
}

/* lod = floor(clamp(lod, min, max) + 0.5). */
static VSC_ErrCode
_ClampFloatLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LodMinMaxSymId,
    IN      VIR_Operand      *pLodOpnd,
    INOUT   VIR_SymId        *pFloatLodSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_SymId                   floatLodSymId = VIR_INVALID_ID;
    VIR_SymId                   floatLodMinMaxSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;

    if (pFloatLodSymId)
    {
        floatLodSymId = *pFloatLodSymId;
    }

    if (floatLodSymId == VIR_INVALID_ID)
    {
        /* Create a reg to clamp the LOD. */
        regId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       regId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT32),
                                       VIR_STORAGE_UNKNOWN,
                                       &floatLodSymId);
        ON_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
    }

    /* Convert the lodMinMax to float. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X2),
                                   VIR_STORAGE_UNKNOWN,
                                   &floatLodMinMaxSymId);
    ON_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_CONV,
                                                VIR_TYPE_FLOAT_X2,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodMinMaxSymId,
                                VIR_TYPE_FLOAT_X2);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XY);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                LodMinMaxSymId,
                                VIR_TYPE_INTEGER_X2);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYYY);

    /* Clamp the float lod.*/
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MAX,
                                                VIR_TYPE_FLOAT32,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    if (pLodOpnd)
    {
        VIR_Operand_Copy(operand, pLodOpnd);
    }
    else
    {
        gcmASSERT(floatLodSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    floatLodSymId,
                                    VIR_TYPE_FLOAT32);
    }
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodMinMaxSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MIN,
                                                VIR_TYPE_FLOAT32,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodMinMaxSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_YYYY);

    /* ADD: floatLod, floatLod, 0.5. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_ADD,
                                                VIR_TYPE_FLOAT32,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateFloat(operand, 0.5);

    /* FLOOR: floatLod, floatLod. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_FLOOR,
                                                VIR_TYPE_FLOAT32,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                floatLodSymId,
                                VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    if (pFloatLodSymId)
    {
        *pFloatLodSymId = floatLodSymId;
    }
OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateFloatLodByGrad(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *pFloatLodSymId
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_VirRegId        regId = VIR_INVALID_ID;
    VIR_Operand * texldModifier = (VIR_Operand *)VIR_Inst_GetSource(pInst, 2);
    gctUINT             coordCompCount;
    VIR_SymId           dpdxSymId = VIR_INVALID_ID, dpdySymId = VIR_INVALID_ID;
    VIR_TypeId          gradTypeId = VIR_INVALID_ID;
    VIR_Instruction*    inst = gcvNULL;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_SymId           floatLevelBaseSizeSymId = VIR_INVALID_ID;

    coordCompCount = VIR_TypeId_GetSamplerCoordComponentCount(VIR_Operand_GetType(VIR_Inst_GetSource(pInst, 0)));
    gradTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, coordCompCount, 1);

    /* Create two regs to hold dpdx/dpdy. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, gradTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &dpdxSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, gradTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &dpdySymId);
    ON_ERROR(errCode, "Add symbol failed.");

    /* Get the dpdx and dpdy. */
    if (VIR_Operand_hasGradFlag(texldModifier))
    {
        VIR_Operand*    pDpdxOpnd = gcvNULL;
        VIR_Operand*    pDpdyOpnd = gcvNULL;

        pDpdxOpnd = VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDX);
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_MOV,
                                                    gradTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    dpdxSymId,
                                    gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pDpdxOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

        pDpdyOpnd = VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDY);
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_MOV,
                                                    gradTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    dpdySymId,
                                    gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pDpdyOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));
    }
    else
    {
        VIR_Operand*    pCoordOpnd = VIR_Inst_GetSource(pInst, 1);

        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_DSX,
                                                    gradTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    dpdxSymId,
                                    gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pCoordOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_DSY,
                                                    gradTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    dpdySymId,
                                    gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pCoordOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));
    }

    /* Convert the levelBaseSize to float. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   regId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X3),
                                   VIR_STORAGE_UNKNOWN,
                                   &floatLevelBaseSizeSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_CONV,
                                                VIR_TYPE_FLOAT_X3,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                floatLevelBaseSizeSymId,
                                VIR_TYPE_FLOAT_X2);
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XYZ);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                LevelBaseSizeSymId,
                                VIR_TYPE_INTEGER_X3);
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYWW);

    /* MUL, dpdx, dpdx, size. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MUL,
                                                gradTypeId,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                gradTypeId);
    VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                floatLevelBaseSizeSymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* MUL, dpdy, dpdy, size. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MUL,
                                                gradTypeId,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                gradTypeId);
    VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                floatLevelBaseSizeSymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* DOT, dpdx.x, dpdx, dpdx. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_DOT,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* SQRT dpdx.x, dpdx.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_SQRT,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* DOT, dpdy.x, dpdy, dpdy. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_DOT,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* SQRT, dpdy.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_SQRT,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* LOG2, dpdx.x, dpdx.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_LOG2,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* LOG2, dpdy.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_LOG2,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* MAX, dpdx.x, dpdx.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MAX,
                                                VIR_GetTypeComponentType(gradTypeId),
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdxSymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
                                pFunc,
                                dpdySymId,
                                VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* Clamp the lod. */
    errCode = _ClampFloatLod(pShader,
                             pFunc,
                             pInst,
                             LodMinMaxSymId,
                             gcvNULL,
                             &dpdxSymId);
    ON_ERROR(errCode, "Clamp float lod");

    if (pFloatLodSymId)
    {
        *pFloatLodSymId = dpdxSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateIntegerLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *pIntLodSymId
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction*    pCalcLodInst = gcvNULL;
    VIR_Operand*        pParamSrc;
    VIR_Operand*        pSamplerOpnd = gcvNULL;
    VIR_Operand*        pCoordOpnd = gcvNULL;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_VirRegId        lodRegId = VIR_INVALID_ID;
    VIR_SymId           intLodSymId = VIR_INVALID_ID;
    VIR_SymId           newCoordSymId = VIR_INVALID_ID;
    VIR_TypeId          coordTypeId;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    gctBOOL             isTexldProj;

    isTexldProj = (opCode == VIR_OP_TEXLDPROJ || opCode == VIR_OP_TEXLDPCFPROJ);

    pParamSrc = VIR_Inst_GetSource(pInst, 2);

    /* Get the sampler and coord operand. */
    pSamplerOpnd = VIR_Inst_GetSource(pInst, 0);
    pCoordOpnd = VIR_Inst_GetSource(pInst, 1);

    /* Get coord type. */
    coordTypeId = VIR_Operand_GetType(pCoordOpnd);

    /* Create a reg to hold the LOD. */
    lodRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                    VIR_SYM_VIRREG,
                                    lodRegId,
                                    VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                    VIR_STORAGE_UNKNOWN,
                                    &intLodSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    /*
    ** 1) With explicit gradients, then we need to calculate lod with these gradients.
    ** 2) Without explicit gradients and in FS, use LODQ to get the lod.
    ** 3) Otherwise, use 0 as LOD.
    */
    if (VIR_Operand_hasGradFlag(pParamSrc))
    {
        VIR_SymId   floatLodSymId = VIR_INVALID_ID;
        VIR_Instruction* convInst = gcvNULL;

        errCode = _CalculateFloatLodByGrad(pShader,
                                           pFunc,
                                           pInst,
                                           LevelBaseSizeSymId,
                                           LodMinMaxSymId,
                                           &floatLodSymId);
        ON_ERROR(errCode, "Calculate lod by grad failed.");

        /* Conv the float LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_CONV,
                                                    VIR_TYPE_INT32,
                                                    pInst,
                                                    gcvTRUE,
                                                    &convInst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(convInst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    intLodSymId,
                                    VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(convInst, 0);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    floatLodSymId,
                                    VIR_TYPE_FLOAT32);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);
    }
    else if (VIR_Shader_IsFS(pShader))
    {
        /* If this is a texldproj, we need to div the q component. */
        if (isTexldProj)
        {
            VIR_Swizzle     swizzle;
            VIR_Instruction*pCoordInst = gcvNULL;
            VIR_VirRegId    coordRegId = VIR_INVALID_ID;
            VIR_TypeId      newCoordTypeId;

            /* Create a reg to save the coord. */
            coordRegId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           coordRegId,
                                           VIR_Shader_GetTypeFromId(pShader, coordTypeId),
                                           VIR_STORAGE_UNKNOWN,
                                           &newCoordSymId);
            ON_ERROR(errCode, "Add symbol failed.");

            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_MOV,
                                                        coordTypeId,
                                                        pInst,
                                                        gcvTRUE,
                                                        &pCoordInst);
            ON_ERROR(errCode, "Add instruction failed.");

            /* Set Dest. */
            pOpnd = VIR_Inst_GetDest(pCoordInst);
            VIR_Operand_SetTempRegister(pOpnd,
                                        pFunc,
                                        newCoordSymId,
                                        coordTypeId);
            VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(coordTypeId));

            /* Set Source0. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 0);
            VIR_Operand_Copy(pOpnd, pCoordOpnd);

            /* Div the q component. */
            newCoordTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(coordTypeId),
                                                                VIR_GetTypeComponents(coordTypeId) - 1,
                                                                1);

            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_DIV,
                                                        newCoordTypeId,
                                                        pInst,
                                                        gcvTRUE,
                                                        &pCoordInst);
            ON_ERROR(errCode, "Add instruction failed.");

            /* Set Dest. */
            pOpnd = VIR_Inst_GetDest(pCoordInst);
            VIR_Operand_SetTempRegister(pOpnd,
                                        pFunc,
                                        newCoordSymId,
                                        newCoordTypeId);
            VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(newCoordTypeId));

            /* Set Source0. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 0);
            VIR_Operand_SetTempRegister(pOpnd,
                                        pFunc,
                                        newCoordSymId,
                                        newCoordTypeId);
            VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(newCoordTypeId));

            /* Set Source1. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 1);
            VIR_Operand_Copy(pOpnd, pCoordOpnd);
            swizzle = VIR_Operand_GetSwizzle(pCoordOpnd);
            swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
            VIR_Operand_SetSwizzle(pOpnd, VIR_Enable_2_Swizzle_WShift((VIR_Enable)(VIR_ENABLE_X << swizzle)));
            VIR_Operand_SetType(pOpnd, VIR_GetTypeComponentType(coordTypeId));
        }

        /* Insert a intrinsic call to get LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_LODQ,
                                                    VIR_TYPE_INT32,
                                                    pInst,
                                                    gcvTRUE,
                                                    &pCalcLodInst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(pCalcLodInst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    intLodSymId,
                                    VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 0);
        VIR_Operand_Copy(pOpnd, pSamplerOpnd);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 1);
        if (isTexldProj)
        {
            VIR_Operand_SetTempRegister(pOpnd,
                                        pFunc,
                                        newCoordSymId,
                                        coordTypeId);
            VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(coordTypeId));
        }
        else
        {
            VIR_Operand_Copy(pOpnd, pCoordOpnd);
        }

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 2);
        if (VIR_Operand_hasBiasFlag(pParamSrc))
        {
            VIR_Operand * texldModifier = (VIR_Operand *)pParamSrc;

            VIR_Operand_Copy(pOpnd, VIR_Operand_GetTexldBias(pParamSrc));
            /* Remove the bias. */
            VIR_Operand_ClrTexModifierFlag(pParamSrc, VIR_TMFLAG_BIAS);
            VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_BIAS, gcvNULL);
        }
        else
        {
            VIR_Operand_SetImmediateFloat(pOpnd, 0.0);
        }
    }
    else
    {
        /* Use 0 as LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_MOV,
                                                    VIR_TYPE_INT32,
                                                    pInst,
                                                    gcvTRUE,
                                                    &pCalcLodInst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(pCalcLodInst);
        VIR_Operand_SetTempRegister(pOpnd,
                                    pFunc,
                                    intLodSymId,
                                    VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 0);
        VIR_Operand_SetImmediateInt(pOpnd, 0);
    }

    if (pIntLodSymId)
    {
        *pIntLodSymId = intLodSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_GetIntegerLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *IntgerLodSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand  *pTexldModifier = (VIR_Operand *)VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pLodOpnd = VIR_Operand_GetTexldLod(pTexldModifier);
    VIR_SymId                   intLodSymId = VIR_INVALID_ID, floatLodSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;
    gctBOOL                     isFloatLod;

    /* If there is not lod modifier, then we need to calculate it. */
    if (pLodOpnd == gcvNULL)
    {
        errCode = _CalculateIntegerLod(pShader,
                                       pFunc,
                                       pInst,
                                       LevelBaseSizeSymId,
                                       LodMinMaxSymId,
                                       IntgerLodSymId);
        ON_ERROR(errCode, "Calculate lod failed.");

        return errCode;
    }

    isFloatLod = VIR_TypeId_isFloat(VIR_Operand_GetType(pLodOpnd));

    /* Create a temp to hold the integer LOD. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                    VIR_SYM_VIRREG,
                                    regId,
                                    VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                    VIR_STORAGE_UNKNOWN,
                                    &intLodSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    if (isFloatLod)
    {
        /* If the LOD is floating variable from shader source, we need to clamp it. */
        errCode = _ClampFloatLod(pShader,
                                 pFunc,
                                 pInst,
                                 LodMinMaxSymId,
                                 pLodOpnd,
                                 &floatLodSymId);
        ON_ERROR(errCode, "Clamp float lod failed.");
    }

    /* Move the integer LOD. */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                isFloatLod ? VIR_OP_CONV : VIR_OP_MOV,
                                                VIR_TYPE_INT32,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                intLodSymId,
                                VIR_TYPE_INT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    if (!isFloatLod)
    {
        VIR_Operand_Copy(operand, pLodOpnd);
    }
    else
    {
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    floatLodSymId,
                                    VIR_TYPE_FLOAT32);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
    }

    /* Return the LOD symbol ID. */
    if (IntgerLodSymId)
    {
        *IntgerLodSymId = intLodSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateOffsetCoord(
    IN      VIR_Shader       *pShader,
    IN      VSC_HW_CONFIG    *HwCfg,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_OpCode                  opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand                *pCoordOpnd = VIR_Inst_GetSource(pInst, 1);
    VIR_TypeId                  coordTypeId = VIR_Operand_GetType(pCoordOpnd);
    VIR_TypeId                  newCoordTypeId = VIR_INVALID_ID;
    VIR_Type                   *pCoordType = VIR_Shader_GetTypeFromId(pShader, coordTypeId);
    VIR_Operand                *pParamOpnd = VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pOffsetOpnd = VIR_Operand_GetTexldOffset(pParamOpnd);
    VIR_TypeId                  offsetTypeId = VIR_Operand_GetType(pOffsetOpnd);
    VIR_Type                   *pOffsetType = VIR_Shader_GetTypeFromId(pShader, offsetTypeId);
    VIR_SymId                   intLodSymId = VIR_INVALID_ID;
    VIR_SymId                   levelBaseSizeSymId = VIR_INVALID_ID, lodMinMaxSymId = VIR_INVALID_ID;
    VIR_SymId                   newCoordSymId = VIR_INVALID_ID, sizeSymId, newOffsetSymId = VIR_INVALID_ID;
    VIR_VirRegId                tempRegId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;
    gctUINT                     offsetCompCount = VIR_GetTypeComponents(offsetTypeId);
    gctBOOL                     addOffsetOnly = gcvFALSE;
    gctBOOL                     isTexldProj = gcvFALSE;
    gctBOOL                     isTexGather = gcvFALSE;

    if (VIR_Type_isInteger(pCoordType) && VIR_Type_isInteger(pOffsetType))
    {
        gcmASSERT(opCode == VIR_OP_TEXLD_U);

        if (HwCfg->hwFeatureFlags.hasUniversalTexldV2 &&
            HwCfg->hwFeatureFlags.hasTexldUFix)
        {
            addOffsetOnly = gcvTRUE;
        }
    }

    if (opCode == VIR_OP_TEXLDPROJ || opCode == VIR_OP_TEXLDPCFPROJ)
    {
        isTexldProj = gcvTRUE;
        newCoordTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(coordTypeId),
                                                         VIR_GetTypeComponents(coordTypeId) - 1,
                                                         1);
    }

    if (VIR_Operand_hasGatherFlag(pParamOpnd))
    {
        isTexGather = gcvTRUE;
    }

    if (!addOffsetOnly)
    {
        /* Get sampler info. */
        errCode = _GetSamplerInfo(pShader,
                                  pFunc,
                                  pInst,
                                  &levelBaseSizeSymId,
                                  &lodMinMaxSymId);
        ON_ERROR(errCode, "Get sampler info.");

        /* Get the LOD. */
        errCode = _GetIntegerLod(pShader,
                                 pFunc,
                                 pInst,
                                 levelBaseSizeSymId,
                                 lodMinMaxSymId,
                                 &intLodSymId);
        ON_ERROR(errCode, "get integer lod failed.");
        gcmASSERT(intLodSymId != VIR_INVALID_ID);
    }

    /* Move the coord into a new reg. */
    tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   tempRegId,
                                   VIR_Shader_GetTypeFromId(pShader, coordTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &newCoordSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MOV,
                                                coordTypeId,
                                                pInst,
                                                gcvTRUE,
                                                &inst);
    ON_ERROR(errCode, "Add instruction.");

    /* Set Dest. */
    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
                                pFunc,
                                newCoordSymId,
                                coordTypeId);
    VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(coordTypeId));

    /* Set Source0. */
    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pCoordOpnd);

    /* If it is a projective texld, we need to div the coordinate with the q component of the coordinate. */
    if (isTexldProj)
    {
        VIR_Swizzle                 swizzle;
        VIR_Operand  *pTexldModifier = (VIR_Operand *) VIR_Inst_GetSource(pInst, 2);

        if (VIR_Operand_GetTexldGather_refz(pTexldModifier))
        {
            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_MOV,
                                                        VIR_GetTypeComponentType(coordTypeId),
                                                        pInst,
                                                        gcvTRUE,
                                                        &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                                        pFunc,
                                        newCoordSymId,
                                        VIR_GetTypeComponentType(coordTypeId));
            VIR_Operand_SetEnable(operand, (VIR_Enable)(VIR_ENABLE_X << (VIR_GetTypeComponents(newCoordTypeId) - 1)));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_Copy(operand, VIR_Operand_GetTexldGather_refz(pTexldModifier));

            /* Clean the drefZ. */
            VIR_Operand_SetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_GATHERREFZ, gcvNULL);
            if (VIR_Operand_GetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_GATHERCOMP) == gcvNULL)
            {
                VIR_Operand_ClrTexModifierFlag(pTexldModifier, VIR_TMFLAG_GATHER);
            }
        }

        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_DIV,
                                                    newCoordTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    newCoordTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(newCoordTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    newCoordTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(newCoordTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    VIR_GetTypeComponentType(coordTypeId));
        swizzle = VIR_Operand_GetSwizzle(pCoordOpnd);
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        VIR_Operand_SetSwizzle(operand, VIR_Enable_2_Swizzle_WShift((VIR_Enable)(VIR_ENABLE_X << swizzle)));
        VIR_Operand_SetType(operand, VIR_GetTypeComponentType(coordTypeId));
    }

    /* Calculate the new coordinate value with offset. */
    if (addOffsetOnly)
    {
        /* ADD: newCoord, coord, offset */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_ADD,
                                                    offsetTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    offsetTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(offsetTypeId));

        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(operand, pCoordOpnd);

        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_Copy(operand, pOffsetOpnd);
    }
    else
    {
        VIR_TypeId tempTypeId;

        /* RSHIFT: size, levelBaseSize, LOD. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, offsetCompCount, 1);
        tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       tempRegId,
                                       VIR_Shader_GetTypeFromId(pShader, tempTypeId),
                                       VIR_STORAGE_UNKNOWN,
                                       &sizeSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_RSHIFT,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    levelBaseSizeSymId,
                                    VIR_TYPE_INTEGER_X4);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    intLodSymId,
                                    VIR_TYPE_INT32);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

        /* MAX: size, size, 1. */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_MAX,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetImmediateInt(operand, 1);

        /* I2F: size, size. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_CONV,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, offsetCompCount, 1);
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* I2F: newOffset, offset*/
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       tempRegId,
                                       VIR_Shader_GetTypeFromId(pShader, tempTypeId),
                                       VIR_STORAGE_UNKNOWN,
                                       &newOffsetSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_CONV,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newOffsetSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(operand, pOffsetOpnd);

        /* If this offset is for textureGather, we need to clamp the offset. */
        if (isTexGather)
        {
            gctFLOAT minOffset = -8.0, maxOffset = 7.0;

            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_MAX,
                                                        tempTypeId,
                                                        pInst,
                                                        gcvTRUE,
                                                        &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                                        pFunc,
                                        newOffsetSymId,
                                        tempTypeId);
            VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_SetImmediateFloat(operand, minOffset);

            /* Set Source1. */
            operand = VIR_Inst_GetSource(inst, 1);
            VIR_Operand_SetTempRegister(operand,
                                        pFunc,
                                        newOffsetSymId,
                                        tempTypeId);
            VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_MIN,
                                                        tempTypeId,
                                                        pInst,
                                                        gcvTRUE,
                                                        &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                                        pFunc,
                                        newOffsetSymId,
                                        tempTypeId);
            VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_SetImmediateFloat(operand, maxOffset);

            /* Set Source1. */
            operand = VIR_Inst_GetSource(inst, 1);
            VIR_Operand_SetTempRegister(operand,
                                        pFunc,
                                        newOffsetSymId,
                                        tempTypeId);
            VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));
        }

        /* DIV: newOffset, newOffset, size. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_DIV,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newOffsetSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newOffsetSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    sizeSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* ADD: newCoord, newCoord, newOffset. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_ADD,
                                                    tempTypeId,
                                                    pInst,
                                                    gcvTRUE,
                                                    &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newCoordSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
                                    pFunc,
                                    newOffsetSymId,
                                    tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));
    }

    /* Update the coord operand. */
    VIR_Operand_SetSymbol(pCoordOpnd, pFunc, newCoordSymId);
    if (isTexldProj)
    {
        /* We don't need the proj after this process. */
        VIR_Operand_SetType(pCoordOpnd, newCoordTypeId);
        VIR_Operand_SetSwizzle(pCoordOpnd, VIR_TypeId_Conv2Swizzle(newCoordTypeId));
        VIR_Inst_SetOpcode(pInst, (opCode == VIR_OP_TEXLDPROJ) ? VIR_OP_TEXLD : VIR_OP_TEXLDPCF);
    }
    else
    {
        VIR_Operand_SetSwizzle(pCoordOpnd, VIR_TypeId_Conv2Swizzle(coordTypeId));
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateCoordWithOffset(
    IN      VIR_Shader       *pShader,
    IN      VSC_HW_CONFIG    *HwCfg,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand                *pParamSrc = VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pOffsetOpnd = gcvNULL;
    VIR_Operand  *pTexldModifier = (VIR_Operand *)pParamSrc;

    /* Skip non-param operand. */
    if (pParamSrc == gcvNULL || VIR_Operand_GetOpKind(pParamSrc) != VIR_OPND_TEXLDPARM)
    {
        return errCode;
    }

    /* Skip non-offset param. */
    if (!VIR_Operand_hasOffsetFlag(pTexldModifier))
    {
        return errCode;
    }

    /* Get the offset operand. */
    pOffsetOpnd =  VIR_Operand_GetTexldOffset(pTexldModifier);

    /* Check if we need to calculate the offset. */
    if (_NeedToCalculateOffset(pShader, pFunc, pOffsetOpnd))
    {
        errCode = _CalculateOffsetCoord(pShader,
                                        HwCfg,
                                        pFunc,
                                        pInst);
        ON_ERROR(errCode, "Calculate offset coord");
    }

    /* Clean up the offset param. */
    VIR_Operand_ClrTexModifierFlag(pTexldModifier, VIR_TMFLAG_OFFSET);
    VIR_Operand_SetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_OFFSET, gcvNULL);

OnError:
    return errCode;
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
                VIR_OpCode       opcode = VIR_Inst_GetOpcode(inst);

                /* set restart flag for the shader,
                   opt:
                   1) if the output is point, we don't need to set the flag */
                if (!VIR_Shader_GS_HasRestartOp(Shader))
                {
                    if ((opcode == VIR_OP_RESTART) &&
                        (Shader->shaderLayout.geo.geoOutPrimitive != VIR_GEO_POINTS))
                    {
                        VIR_Shader_SetFlag(Shader, VIR_SHFLAG_GS_HAS_RESTART_OP);
                    }
                }

                /* Calcuate the new coordinate with offset, if hardware doesn't support it directly. */
                if (!HwCfg->hwFeatureFlags.supportTexldOffset &&
                    VIR_OPCODE_isTexLd(opcode))
                {
                    _CalculateCoordWithOffset(Shader, HwCfg, func, inst);
                }
                else if ((opcode == VIR_OP_ATTR_LD ||
                          opcode == VIR_OP_ATTR_ST) &&
                         VIR_Operand_isUndef(VIR_Inst_GetSource(inst, 2)))
                {
                    gctINT  indexConstValue;   /* constIndexing + matrixIndexing */
                    VIR_Symbol *sym;

                    if (opcode == VIR_OP_ATTR_LD)
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
                        /* VIR_Inst_GetSource(inst, 2), create a new Immediate Value*/
                        VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(inst, 2), indexConstValue);
                    }
                    /* FE won't generate indexed for matrix, if this src has indexed, it must be an error*/
                    else
                    {
                        VIR_SymId symId = VIR_Operand_GetRelIndexing(src);

                        gcmASSERT(!VIR_Operand_GetIsConstIndexing(src) );

                        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(inst, 2), func, symId, VIR_TYPE_INT32);
                        VIR_Operand_SetPrecision(VIR_Inst_GetSource(inst, 2), VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));
                    }

                    /* reset src indexing info */
                    VIR_Operand_SetMatrixConstIndex(src, 0);
                    VIR_Operand_SetRelIndexingImmed(src, 0);
                    VIR_Operand_SetRelAddrMode(src, VIR_INDEXED_NONE);
                    VIR_Operand_SetIsConstIndexing(src, 0);
                    if (opcode == VIR_OP_ATTR_LD)
                    {
                        /* set the tcsPatchInputVertices for const indexing, index is source 1 */
                        if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL &&
                            !VIR_Shader_TCS_UseDriverInput(Shader))
                        {
                            src = VIR_Inst_GetSource(inst, 1);
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
                            src = VIR_Inst_GetSource(inst, 1);
                            VIR_Inst_SetSource(inst, 1, VIR_Inst_GetSource(inst, 2));
                            VIR_Inst_SetSource(inst, 2, src);
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
                        src = VIR_Inst_GetSource(inst, 0);
                        if (VIR_Symbol_isPerPatch(sym))
                        {
                            VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 2));
                        }
                        else
                        {
                            VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 1));
                            VIR_Inst_SetSource(inst, 1, VIR_Inst_GetSource(inst, 2));
                        }
                        VIR_Inst_SetSource(inst, 2, src);
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

                        if (opcode == VIR_OP_ATTR_LD &&
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
                                 opcode != VIR_OP_LDARR)
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
                    if (opcode == VIR_OP_ATTR_ST)
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
                             opcode != VIR_OP_STARR)
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

