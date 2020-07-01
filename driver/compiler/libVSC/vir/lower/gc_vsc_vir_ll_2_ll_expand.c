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


#include "vir/lower/gc_vsc_vir_ml_2_ll.h"

#include "vir/lower/gc_vsc_vir_lower_common_func.h"

static gctBOOL
_isOperandScalar(
    IN VIR_PatternContext *Context,
    IN VIR_Operand *Opnd
    );

gctBOOL
_checkToSetFullDefFlag(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *MaskValueOperand
    );

gctBOOL
_copyFullDefFlag(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );


static gctBOOL
_destPackedLE4Components(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_destChar_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_destChar_P16Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_destShort_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_setNonpackedTypeByPatternDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
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
_setRowOrder0PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setRowOrder1PackedMaskValue(
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
_setColumn2PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn3PackedMaskValue(
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
_setRowOrder0PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setRowOrder1PackedSwizzle(
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
_setColumn2PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn3PackedSwizzle(
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
_setRowOrder0UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setRowOrder1UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumnUnPackedMaskValue(
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
_setRowOrder0UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setRowOrder1UnPackedSwizzle(
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
_setColumn2UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    );

static gctBOOL
_setColumn3UnPackedSwizzle(
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
    VIR_Operand_SetModifier(Opnd, VIR_MOD_ABS | VIR_Operand_GetModifier(Opnd));
    if (VIR_Operand_GetModifier(Opnd) & VIR_MOD_NEG)
    {
        VIR_Operand_ClrOneModifier(Opnd, VIR_MOD_NEG); /* clear .neg modifier when abs is set after */
    }
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
_isSrcFloatZero(
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
    if(VIR_Operand_GetImmediateFloat(src) != 0.0)
        return gcvFALSE;

    return gcvTRUE;
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
_isSrcIntOne(
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
    if(VIR_Operand_GetImmediateInt(src) != 1)
        return gcvFALSE;

    return gcvTRUE;
}

static gctBOOL
_isSrc0FloatZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcFloatZero(Context, Inst, 0);
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
_isSrc0IntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcIntOne(Context, Inst, 0);
}

static gctBOOL
_isSrc1IntOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcIntOne(Context, Inst, 1);
}

static gctBOOL
_isSrc1IntZero
    (
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcZero(Context, Inst, 1);
}

static gctBOOL
_isSrc2IntZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcZero(Context, Inst, 2);
}

static gctBOOL
_isSrc1FloatOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcFloatOne(Context, Inst, 1);
}

static gctBOOL
_isSrc2FloatZero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcFloatZero(Context, Inst, 2);
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
    return VIR_Shader_IsCLFromLanguage(Context->shader);
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
_supportVectorB0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _hasB0(Context, Inst) && (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportVectorB0);
}

static gctBOOL
_supportScalarB0Only(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _hasB0(Context, Inst) && !(((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportVectorB0);
}

static gctBOOL
_hasB0_VG(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _hasB0(Context, Inst) && VIR_Shader_IsOpenVG(Context->shader);
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
_hasInteger_long_ulong(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    );

static gctBOOL
_hasInteger_long_ulong_isRAEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _hasInteger_long_ulong(Context, Inst) &&
           VIR_Shader_isRAEnabled(Context->shader);
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
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (pOpnd == gcvNULL || !VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)) &&
            (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_hasInteger_long_ulong_isRAEnabled_src0_uniform_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (pOpnd == gcvNULL || !VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    return (_hasInteger_long_ulong(Context, Inst) &&
            VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)) &&
            (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_isRAEnabled_src0_uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (pOpnd == gcvNULL || !VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)));
}

static gctBOOL
_hasInteger_long_ulong_isRAEnabled_src0_uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(Inst, 0);

    if (pOpnd == gcvNULL || !VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    return (_hasInteger_long_ulong(Context, Inst) &&
            VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)));
}

static gctBOOL
_isRAEnabled_src1_uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Operand         *pOpnd = VIR_Inst_GetSource(Inst, 1);

    if (pOpnd == gcvNULL || !VIR_Operand_isSymbol(pOpnd))
    {
        return gcvFALSE;
    }

    return (VIR_Shader_isRAEnabled(Context->shader) &&
            VIR_Symbol_isUniform(VIR_Operand_GetSymbol(pOpnd)));
}

/* sampler index should always be integer */
static gctBOOL
_isRAEnabled_src0_not_sampler_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
           !VIR_TypeId_isSampler(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0))) &&
           (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

/* sampler index should always be integer */
static gctBOOL
_hasInteger_long_ulong_isRAEnabled_src0_not_sampler_src1_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (_hasInteger_long_ulong(Context, Inst) &&
            VIR_Shader_isRAEnabled(Context->shader) &&
           !VIR_TypeId_isSampler(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0))) &&
           (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_hasInteger_long_ulong_isRAEnabled_dest_not_sampler_src0_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (_hasInteger_long_ulong(Context, Inst) &&
            VIR_Shader_isRAEnabled(Context->shader) &&
            !VIR_TypeId_isSampler(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst))) &&
            (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0))) & VIR_TYFLAG_ISFLOAT));
}

static gctBOOL
_isRAEnabled_dest_not_sampler_src0_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_Shader_isRAEnabled(Context->shader) &&
        !VIR_TypeId_isSampler(VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst))) &&
        (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0))) & VIR_TYFLAG_ISFLOAT));
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

    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;

    imm0.iValue = 4;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
int_value_type0_const_1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_ScalarConstVal imm0;

    imm0.iValue = 1;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
    VIR_Operand_SetEnable(dest, VIR_ENABLE_X);

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
    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
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
    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
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
    VIR_Operand_SetTypeId(dest, VIR_TYPE_FLOAT32);
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
_setMOVAEnableXIntUniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand*    pDest = VIR_Inst_GetDest(Inst);

    _setMOVAEnableInt(Context, Inst, Opnd);
    VIR_Operand_SetEnable(pDest, VIR_ENABLE_X);
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
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(Inst, 1), VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setLDARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(Inst, 1), VIR_TYPE_FLOAT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(Inst, 0), VIR_TYPE_INT32);
    return gcvTRUE;
}

static gctBOOL
_setSTARRSwizzleFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(Inst, 0), VIR_TYPE_FLOAT32);
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
    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_CONVERT);

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
    gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_CONVERT);
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

    VIR_Operand_SetTypeId(VIR_Inst_GetDest(Inst), VIR_TypeId_ComposeNonOpaqueType(format,
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
    VIR_TypeId typeId;
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

    if(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(dest)))
    {
        components = VIR_GetTypePackedComponents(VIR_Lower_GetBaseType(Context->shader, dest));
    }
    else
    {
        components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, dest));
    }
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(Inst), typeId);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), VIR_TypeId_Conv2Enable(typeId));

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
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(src0)));
    dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(dest);

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));

    components = VIR_GetTypeSize(VIR_Operand_GetTypeId(dest)) / VIR_GetTypeSize(format);
    gcmASSERT(components <= 16);
    VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposePackedNonOpaqueType(format, components));
    tyId = VIR_Operand_GetTypeId(dest);
    _changeEnableByTyId(tyId, dest);

    return gcvTRUE;
}

static gctBOOL
_setDestTypeFromSrc0Unpacked(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_PrimitiveTypeId  format;
    VIR_Operand *src0;
    VIR_Operand *dest;
    VIR_TypeId tyId;
    VIR_Enable enable = VIR_ENABLE_X;
    gctUINT components;
    VIR_Type *type;

    src0 = VIR_Inst_GetSource(Inst, 0);
    tyId = VIR_Operand_GetTypeId(src0);
    dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(dest);

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type))  {  /* change swizzle to mov for scalar source */
        gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_SWIZZLE);
        VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(Inst, 1);
        VIR_Operand_SetTypeId(dest, tyId);
    }
    else {
        gcmASSERT(VIR_TypeId_isPacked(tyId));

        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));

        components = VIR_GetTypePackedComponents(tyId);
        if(components == 3) components++;
        components *= 4 / VIR_GetTypeSize(format);

        gcmASSERT(components <= 16);
        VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposePackedNonOpaqueType(format, components));
        switch(VIR_GetTypePackedComponents(tyId)) {
        case 2:
            enable = VIR_ENABLE_XY;
            break;

        case 3:
            enable = VIR_ENABLE_XYZ;
            break;

        case 4:
        case 8:
        case 16:
            enable = VIR_ENABLE_XYZW;
            break;

        default:
            gcmASSERT(0);
            break;
        }
    }
    VIR_Inst_SetInstType(Inst, VIR_Operand_GetTypeId(dest));
    VIR_Inst_SetEnable(Inst, enable);

    return gcvTRUE;
}

static gctBOOL
_setDestTypeFromSrc0RowUnpacked(
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
    VIR_Type *type;

    src0 = VIR_Inst_GetSource(Inst, 0);
    dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(dest);

    tyId = VIR_Operand_GetTypeId(src0);
    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type))  {  /* change swizzle to mov for scalar source */
        gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_SWIZZLE);
        VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(Inst, 1);
        VIR_Operand_SetTypeId(dest, tyId);
    }
    else {
        gcmASSERT(VIR_TypeId_isPacked(tyId));

        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));

        components = 4 / VIR_GetTypeSize(format);
        components *= components;
        gcmASSERT(components <= 16);
        VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposePackedNonOpaqueType(format, components));
        tyId = VIR_Operand_GetTypeId(dest);
    }
    _changeEnableByTyId(tyId, dest);
    return gcvTRUE;
}

static gctBOOL
_setDestTypeFromSrc0ColumnUnpacked(
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
    VIR_Type *type;

    src0 = VIR_Inst_GetSource(Inst, 0);
    dest = VIR_Inst_GetDest(Inst);
    gcmASSERT(dest);

    tyId = VIR_Operand_GetTypeId(src0);
    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type))  {  /* change swizzle to mov for scalar source */
        gcmASSERT(VIR_Inst_GetOpcode(Inst) == VIR_OP_SWIZZLE);
        VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(Inst, 1);
        VIR_Operand_SetTypeId(dest, tyId);
    }
    else {
        gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(src0)));
        tyId = VIR_Lower_GetBaseType(Context->shader, src0);
        components = VIR_GetTypeComponents(tyId);
        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));

        components *= (4 / VIR_GetTypeSize(format));
        gcmASSERT(components <= 16);
        VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposePackedNonOpaqueType(format, components));
        tyId = VIR_Operand_GetTypeId(dest);
    }
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
value_type0_32bit_reset_sat(
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
    return gcvTRUE;
}

static gctBOOL
_reset_sat_rounding(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
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
    VIR_PrimitiveTypeId format;
    VIR_Operand *src0;

    src0 = VIR_Inst_GetSource(Inst, 0);
    if(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(src0))) return gcvTRUE;

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, src0));
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

    VIR_Operand_SetTypeId(src0, VIR_TypeId_ComposeNonOpaqueType(format,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, src0)), 1));

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

    VIR_Operand_SetTypeId(VIR_Inst_GetDest(Inst), VIR_Operand_GetTypeId(Opnd));

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

    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));

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
        VIR_TypeId prevSrc0TypeID = VIR_Operand_GetTypeId(prevSrc0);

        VIR_Operand_SetTypeId(Opnd, prevSrc0TypeID);
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
        format == VIR_TYPE_FLOAT32 ? VIR_TYPE_FLOAT32 : format,
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
        format == VIR_TYPE_FLOAT32 ? VIR_TYPE_FLOAT32 : format,
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

    VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
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
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(Opnd))) {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
        VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
    }
    else {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);
        VIR_Operand_SetTypeId(dest, VIR_TYPE_UINT32);
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
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(Opnd))) {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT32);
        VIR_Operand_SetTypeId(dest, VIR_TYPE_INT32);
    }
    else {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);
        VIR_Operand_SetTypeId(dest, VIR_TYPE_UINT32);
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
uint_value_type0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);

    VIR_Operand_SetTypeId(dest, VIR_TYPE_UINT32);
    VIR_Inst_SetInstType(Inst, VIR_TYPE_UINT32);
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
_destUnsignedRankHigher_setSrcToZero_elseNop(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId format, srcFormat;
    gctBOOL rankHigher = gcvFALSE;

    if(VIR_TypeId_isUnSignedInteger(VIR_Operand_GetTypeId(dest))) {
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
            if(srcFormat == VIR_TYPE_INT8 ||
               srcFormat == VIR_TYPE_UINT8 ||
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
    }

    if(rankHigher) {
        VIR_ScalarConstVal imm0;
        gctUINT components;

        imm0.uValue = 0;

        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
        components = VIR_GetTypeComponents(VIR_Operand_GetTypeId(dest));
        VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT32, components, 1));
    }
    else
    {
        VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
        VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(Inst, 0);
        VIR_Inst_SetDest(Inst, gcvNULL);
    }
    return gcvTRUE;
}

/* If dest data type is signed rank higher than the src, set src to shift count
   other nop the instruction */
static gctBOOL
_destSignedRankHigher_setSrcToShiftCount_elseNop(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId format, srcFormat;
    gctUINT shiftCount = 0;

    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(dest))) {
        format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
        srcFormat = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
        switch(format) {
        case VIR_TYPE_INT16:
            if(srcFormat == VIR_TYPE_INT8) {
                shiftCount = 8;
            }
            break;

        case VIR_TYPE_INT32:
            if(srcFormat == VIR_TYPE_INT8) {
                shiftCount = 24;
            }
            else if(srcFormat == VIR_TYPE_INT16) {
                shiftCount = 16;
            }
            break;

        default:
           break;
        }
    }

    if(shiftCount) {
        VIR_ScalarConstVal imm0;

        imm0.uValue = shiftCount;
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_UINT32,
                                 imm0);
    }
    else
    {
        VIR_Inst_SetOpcode(Inst, VIR_OP_NOP);
        VIR_Inst_SetConditionOp(Inst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(Inst, 0);
        VIR_Inst_SetDest(Inst, gcvNULL);
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
all_source_single_value(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;
    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        VIR_TypeId src_typeid = VIR_Operand_GetTypeId(src);
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
        VIR_TypeId src_typeid = VIR_Operand_GetTypeId(src);
        if(!(VIR_GetTypeFlag(src_typeid) & VIR_TYFLAG_ISFLOAT))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gctBOOL
all_source_not_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;
    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        VIR_TypeId src_typeid = VIR_Operand_GetTypeId(src);
        if(VIR_GetTypeFlag(src_typeid) & VIR_TYFLAG_ISFLOAT)
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static gctBOOL
all_source_integer(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctUINT i;
    for(i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        VIR_Operand* src = VIR_Inst_GetSource(Inst, i);
        VIR_TypeId src_typeid = VIR_Operand_GetTypeId(src);
        if(!(VIR_GetTypeFlag(src_typeid) & VIR_TYFLAG_ISINTEGER))
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
jmp_2_succ2_resCondOp_float(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId ty0, ty1;
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);

    ty0 = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    ty1 = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);
    /* Texkill only supports float. */
    return VIR_Lower_jmp_2_succ2(Context, Inst) &&
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

    ty0 = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    ty1 = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));

    gcmASSERT(ty0 < VIR_TYPE_PRIMITIVETYPE_COUNT &&
        ty1 < VIR_TYPE_PRIMITIVETYPE_COUNT);
    return (VIR_Swizzle_Channel_Count(swizzle0) == 1 || VIR_Operand_isImm(VIR_Inst_GetSource(Inst, 0))) &&
           (VIR_Swizzle_Channel_Count(swizzle1) == 1 || VIR_Operand_isImm(VIR_Inst_GetSource(Inst, 1))) &&
           VIR_Lower_jmp_2_succ2(Context, Inst) &&
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
           VIR_Lower_jmp_2_succ3(Context, Inst);
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
        VIR_TypeId destTypeID = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(dest));
        VIR_TypeId prevSrc0TypeID = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(prevSrc0));

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
        VIR_TypeId destTypeID = VIR_Operand_GetTypeId(dest);
        VIR_TypeId prevSrc0TypeID = VIR_Operand_GetTypeId(prevSrc0);

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
    gctUINT      components = VIR_GetTypeComponents(VIR_Operand_GetTypeId(Opnd));

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
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_BOOLEAN,
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
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32,
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
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32,
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
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, compCount, 1));

    return gcvTRUE;
}

static gctBOOL
_isBiasTexModifierAndCubeArrayShadow(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId samplerType = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));

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

    newType = VIR_Operand_GetTypeId(coord);
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, newType));
    VIR_Operand_SetTypeId(dest, newType);
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

    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(Opnd))) {
        typeId = VIR_TYPE_INT16_P4;
    }
    else {
        typeId = VIR_TYPE_UINT16_P4;
    }
    VIR_Symbol_SetType(VIR_Operand_GetSymbol(dest), VIR_Shader_GetTypeFromId(Context->shader, typeId));
    VIR_Operand_SetTypeId(dest, typeId);
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
    VIR_Operand_SetTypeId(dest, VIR_GetTypeComponentType(destSymTypeId));

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
    VIR_Operand_SetTypeId(dest, VIR_GetTypeComponentType(destSymTypeId));

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
    VIR_Operand_SetTypeId(Opnd, coordSymTypeId);

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
    VIR_TypeId typeId = VIR_Operand_GetTypeId(Opnd);
    VIR_Type *type = VIR_Shader_GetTypeFromId(shader, typeId);

    typeId = VIR_Type_GetBaseTypeId(type);
    typeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_BOOLEAN, VIR_GetTypeComponents(typeId), 1);

    VIR_Operand_SetTypeId(Opnd, typeId);

    return gcvTRUE;
}

static gctBOOL
_isLongUlong(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *Opnd
)
{
    VIR_PrimitiveTypeId format;

    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    return format == VIR_TYPE_INT64 || format == VIR_TYPE_UINT64;
}

static gctBOOL
_hasInteger_long_ulong(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctBOOL ok;

    ok = _isCLShader(Context, Inst) && gcmOPT_oclInt64InVIR() &&
         _isLongUlong(Context, VIR_Inst_GetDest(Inst));

    return ok && ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportInteger;
}

static gctBOOL
_hasInteger_long_ulong_src0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctBOOL ok;

    ok = _isCLShader(Context, Inst) && gcmOPT_oclInt64InVIR() &&
         _isLongUlong(Context, VIR_Inst_GetSource(Inst, 0));

    return ok && ((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportInteger;
}

static gctINT
_getCL_Long_ulong_store_count(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gctINT storeCount = 0;

    if (_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Enable enable = VIR_Inst_GetEnable(Inst);

        if((enable & VIR_ENABLE_XY) && (enable & VIR_ENABLE_ZW))
        {
            storeCount = 4;
        }
        else
        {
            storeCount = 2;
        }
    }

    return storeCount;
}

static gctBOOL
_isCL_Long_ulong_2_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (_getCL_Long_ulong_store_count(Context, Inst) == 2);
}

static gctBOOL
_isCL_Long_ulong_4_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (_getCL_Long_ulong_store_count(Context, Inst) == 4);
}

#define _gcdLongUlongStoreOneComponentEnable   VIR_ENABLE_X
#define _gcdLongUlongStoreTwoComponentEnable   VIR_ENABLE_XZ

#define _gcdLongUlongComponentByteOffset(Component) ((Component) << 3)

#define _gcdLongUlongSingleEnableByteOffset(Enable)  \
   (((Enable) & VIR_ENABLE_W) ? _gcdLongUlongComponentByteOffset(3) \
                               : _gcdLongUlongComponentByteOffset((Enable) >> 1))


static gcSL_SWIZZLE
_longUlongOneComponentSwizzleMap[4] =
{
    VIR_SWIZZLE_XXXX, /* x */
    VIR_SWIZZLE_YYYY, /* y */
    VIR_SWIZZLE_ZZZZ, /* z */
    VIR_SWIZZLE_WWWW    /* w */
};

static gcSL_SWIZZLE
_longUlongTwoComponentSwizzleMap[16] =
{
    virmSWIZZLE(X, X, X, X), /* xx */
    virmSWIZZLE(Y, Y, X, X), /* yx */
    virmSWIZZLE(Z, Z, X, X), /* zx */
    virmSWIZZLE(W, W, X, X), /* wx */
    virmSWIZZLE(X, X, Y, Y), /* xy */
    virmSWIZZLE(Y, Y, Y, Y), /* yy */
    virmSWIZZLE(Z, Z, Y, Y), /* zy */
    virmSWIZZLE(W, W, Y, Y), /* wy */
    virmSWIZZLE(X, X, Z, Z), /* xz */
    virmSWIZZLE(Y, Y, Z, Z), /* yz */
    virmSWIZZLE(Z, Z, Z, Z), /* zz */
    virmSWIZZLE(W, W, Z, Z), /* wz */
    virmSWIZZLE(X, X, W, W), /* xw */
    virmSWIZZLE(Y, Y, W, W), /* yw */
    virmSWIZZLE(Z, Z, W, W), /* zw */
    virmSWIZZLE(W, W, W, W)    /* ww */
};

static gctBOOL
_SetLongUlongInstTypeOnly(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);

    if(format == VIR_TYPE_INT64 || format == VIR_TYPE_UINT64) format = VIR_TYPE_UINT32;
    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);
    VIR_Inst_SetInstType(Inst, typeId);
    return gcvTRUE;
}

gctBOOL
VIR_Lower_SetLongUlongInstType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(_SetLongUlongInstTypeOnly(Context, Inst, Opnd)) {
        VIR_Operand *dest = VIR_Inst_GetDest(Inst);
        VIR_TypeId typeId = VIR_Operand_GetTypeId(dest);

        if(Opnd)
        {
            VIR_Operand_SetTypeId(Opnd, typeId);
        }
        else
        {
            gctINT i, numSrc;
            VIR_Operand *opnd;

            numSrc = VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(Inst));
            if(numSrc < 4) return gcvFALSE;
            for(i = 0; i < numSrc; i++)
            {
                opnd = VIR_Inst_GetSource(Inst, i);
                VIR_Operand_SetTypeId(opnd, typeId);
            }
        }
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_SetLongUlongDestNextRegInstType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT       rowOffset;
    VIR_TypeId    tyId;
    VIR_Operand   *dest = VIR_Inst_GetDest(Inst);

    tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;
    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    if(VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd))
    {
        VIR_Operand_SetTempRegister(dest,
                                    VIR_Inst_GetFunction(Inst),
                                    symId,
                                    VIR_Operand_GetTypeId(dest));
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctUINT
_GetUpperLongValue(
    gctUINT64 Ulong
    )
{
    VIR_ConstVal constVal;

    constVal.scalarVal.ulValue = Ulong;
    return constVal.vecVal.u32Value[1];
}

VSC_ErrCode
VIR_Lower_ChangeOperandByOffset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction   *Inst,
    IN VIR_Operand       *Opnd,
    IN gctUINT           rowOffset
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    VIR_TypeId    tyId;
    VIR_PrimitiveTypeId  format;
    VIR_ScalarConstVal imm;
    gctUINT       indexOffset;

    if(VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        imm.uValue = _GetUpperLongValue(VIR_Operand_GetImmediateUint64(Opnd));
        VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);
    }
    else {
        sym = VIR_Operand_GetSymbol(Opnd);
        gcmASSERT(sym);
        switch(VIR_Symbol_GetKind(sym)) {
        case VIR_SYM_CONST:
            imm.uValue = 0;
            tyId = VIR_Lower_GetBaseType(Context->shader, Opnd);
            format = VIR_GetTypeComponentType(tyId);
            if(format == VIR_TYPE_UINT32 ||
               format == VIR_TYPE_UINT16 ||
               format == VIR_TYPE_UINT8)
            {
                ;
            }
            else
            {
                VIR_ConstId   constId;
                constId = VIR_Operand_GetConstId(Opnd);

                if(constId == VIR_INVALID_ID) {
                    gcmASSERT(0);
                }
                else {
                    VIR_Const  *vConst = gcvNULL;

                    vConst = VIR_Shader_GetConstFromId(Context->shader,
                                                       constId);
                    if(format == VIR_TYPE_INT32 ||
                       format == VIR_TYPE_INT16 ||
                       format == VIR_TYPE_INT8)
                    {
                        if(vConst->value.scalarVal.iValue < 0)
                        {
                            imm.uValue = 0xFFFFFFFF;
                        }
                    }
                    else  /* long/ulong assumed */
                    {
                         imm.uValue = vConst->value.vecVal.u32Value[1];
                    }
                }
            }
            VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);
            break;

        case VIR_SYM_UNIFORM:
        case VIR_SYM_IMAGE_T:
        case VIR_SYM_IMAGE:
        case VIR_SYM_SAMPLER_T:
        case VIR_SYM_SAMPLER:
            indexOffset = VIR_Operand_GetConstIndexingImmed(Opnd) + rowOffset;
            VIR_Operand_SetRelIndexingImmed(Opnd, indexOffset);
            break;

        default:
            regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
            virErrCode = VIR_Shader_GetVirRegSymByVirRegId(Context->shader,
                                                           regId,
                                                           &symId);
            if(virErrCode != VSC_ERR_NONE) return virErrCode;
            if(symId == VIR_INVALID_ID) {
                virErrCode = VIR_Shader_AddSymbol(Context->shader,
                                                  VIR_SYM_VIRREG,
                                                  regId,
                                                  VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_UNKNOWN),
                                                  VIR_STORAGE_UNKNOWN,
                                                  &symId);
                if(virErrCode != VSC_ERR_NONE) return virErrCode;
            }

            VIR_Operand_SetTempRegister(Opnd,
                                        VIR_Inst_GetFunction(Inst),
                                        symId,
                                        VIR_Operand_GetTypeId(Opnd));
            break;
        }
    }
    return virErrCode;
}

static gctBOOL
_long_ulong_first_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle orgSwizzle;
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable orgEnable;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;
    VIR_Operand *offset, *dest;
    VIR_ScalarConstVal imm0;

    dest = VIR_Inst_GetDest(Inst);

    orgSwizzle = VIR_Operand_GetSwizzle(Opnd) & 0xF; /* get the lower half swizzle */
    orgEnable = VIR_Operand_GetEnable(dest);
    offset = VIR_Inst_GetSource(Inst, 1);
    switch(orgEnable)
    {
    case VIR_ENABLE_X:
    case VIR_ENABLE_Y:
    case VIR_ENABLE_Z:
    case VIR_ENABLE_W:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(enable);
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);

        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_ZW:
        imm0.iValue = ((orgEnable & VIR_ENABLE_X)
                         ?  _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_X)
                         : ((orgEnable & VIR_ENABLE_Y)
                             ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Y)
                             : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z)));
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);

        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(orgEnable & VIR_ENABLE_XY);
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);

        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(dest, enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_second_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle orgSwizzle;
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable orgEnable;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;
    VIR_Operand *offset, *dest;
    VIR_ScalarConstVal imm0;

    dest = VIR_Inst_GetDest(Inst);

    orgSwizzle = VIR_Operand_GetSwizzle(Opnd) & 0xF; /* get the lower half swizzle */
    orgEnable = VIR_Operand_GetEnable(dest);
    offset = VIR_Inst_GetSource(Inst, 1);

    switch(orgEnable)
    {
    case VIR_ENABLE_X:
    case VIR_ENABLE_Y:
    case VIR_ENABLE_Z:
    case VIR_ENABLE_W:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(enable) + 4,
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_ZW:
        imm0.iValue = ((orgEnable & VIR_ENABLE_X)
                          ?  _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_X)
                          : ((orgEnable & VIR_ENABLE_Y)
                              ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Y)
                              : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z))) + 4,
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);
        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(orgEnable & VIR_ENABLE_XY) + 4,
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        gctUINT       rowOffset;
        VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

        gcmASSERT(VIR_GetTypeRows(tyId) > 1);
        rowOffset = VIR_GetTypeRows(tyId) >> 1;

        virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                       Inst,
                                       Opnd,
                                       rowOffset);
        if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(dest, enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_third_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable orgEnable;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;
    VIR_Operand *offset, *dest;
    VIR_ScalarConstVal imm0;

    dest = VIR_Inst_GetDest(Inst);

    offset = VIR_Inst_GetSource(Inst, 1);
    orgEnable = VIR_Operand_GetEnable(dest);
    switch(orgEnable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZW:
    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = (orgEnable & VIR_ENABLE_Z)
                          ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z)
                          : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_W);
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);

        swizzle = VIR_Operand_GetSwizzle(Opnd) >> 4; /* get the upper half swizzle */
        if((orgEnable & VIR_ENABLE_Z) && (orgEnable & VIR_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(dest, enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_fourth_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable orgEnable;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;
    VIR_Operand *offset, *dest;
    VIR_ScalarConstVal imm0;

    dest = VIR_Inst_GetDest(Inst);

    offset = VIR_Inst_GetSource(Inst, 1);
    orgEnable = VIR_Inst_GetEnable(Inst);
    switch(orgEnable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZW:
    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = ((orgEnable & VIR_ENABLE_Z)
                          ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z)
                          : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_W)) + 4;  /* last 3 bytes */
        VIR_Operand_SetImmediate(offset,
                                 VIR_TYPE_INT32,
                                 imm0);

        swizzle = VIR_Operand_GetSwizzle(Opnd) >> 4; /* get the upper half swizzle */
        if((orgEnable & VIR_ENABLE_Z) && (orgEnable & VIR_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        gctUINT       rowOffset;
        VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

        gcmASSERT(VIR_GetTypeRows(tyId) > 1);
        rowOffset = VIR_GetTypeRows(tyId) >> 1;

        virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                       Inst,
                                       Opnd,
                                       rowOffset);
        if(virErrCode != VSC_ERR_NONE) return gcvFALSE;
    }
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(dest, enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_lower_offset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable;
    VIR_Swizzle swizzle = VIR_SWIZZLE_XXXX;
    VIR_ScalarConstVal imm0;

    enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst));
    switch(enable)
    {
    case VIR_ENABLE_X:
    case VIR_ENABLE_Y:
    case VIR_ENABLE_Z:
    case VIR_ENABLE_W:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(enable);
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_INT32,
                                 imm0);
        break;

    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_ZW:
        imm0.iValue = ((enable & VIR_ENABLE_X)
                          ?  _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_X)
                          : ((enable & VIR_ENABLE_Y)
                               ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Y)
                               : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z)));
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_INT32,
                                 imm0);
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = _gcdLongUlongSingleEnableByteOffset(enable & VIR_ENABLE_XY),
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_INT32,
                                 imm0);
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    return int_value_type0(Context,
                           Inst,
                           Opnd);
}

static gctBOOL
_long_ulong_upper_offset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable enable;
    VIR_ScalarConstVal imm0;

    enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst));
    switch(enable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZW:
    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        imm0.iValue = (enable & VIR_ENABLE_Z)
                          ? _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_Z)
                          : _gcdLongUlongSingleEnableByteOffset(VIR_ENABLE_W);
        VIR_Operand_SetImmediate(Opnd,
                                 VIR_TYPE_INT32,
                                 imm0);
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    return int_value_type0(Context,
                           Inst,
                           Opnd);
}

static gctBOOL
_long_ulong_first_add_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT8 orgSwizzle;
    gctUINT8 swizzle = VIR_SWIZZLE_XYYY;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;

    orgSwizzle = VIR_Operand_GetSwizzle(Opnd) & 0xF; /* get the lower half swizzle */
    switch(VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst)))
    {
    case VIR_ENABLE_X:
    case VIR_ENABLE_Y:
    case VIR_ENABLE_Z:
    case VIR_ENABLE_W:
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    case VIR_ENABLE_XY:
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZ:
    case VIR_ENABLE_ZW:
        swizzle = _longUlongTwoComponentSwizzleMap[orgSwizzle];
        enable = _gcdLongUlongStoreTwoComponentEnable;
        break;

    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:
        swizzle = _longUlongOneComponentSwizzleMap[orgSwizzle & 0x3];
        enable = _gcdLongUlongStoreOneComponentEnable;
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_second_add_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    gctUINT       rowOffset;

    tyId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   Opnd,
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    return _long_ulong_first_add_store(Context,
                                       Inst,
                                       Opnd);
}

static gctBOOL
_long_ulong_third_add_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Swizzle swizzle = VIR_SWIZZLE_XYYY;
    VIR_Operand *dest;
    VIR_Enable orgEnable;
    VIR_Enable enable = _gcdLongUlongStoreOneComponentEnable;

    dest =  VIR_Inst_GetDest(Inst);
    orgEnable = VIR_Inst_GetEnable(Inst);
    switch(orgEnable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
    case VIR_ENABLE_YZW:
    case VIR_ENABLE_XZ:
    case VIR_ENABLE_XW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YW:

        swizzle = VIR_Operand_GetSwizzle(Opnd) >> 4; /* get the upper half swizzle */
        if((orgEnable & VIR_ENABLE_Z) && (orgEnable & VIR_ENABLE_W))
        {
            swizzle = _longUlongTwoComponentSwizzleMap[swizzle];
            enable = _gcdLongUlongStoreTwoComponentEnable;
        }
        else
        {
            swizzle = _longUlongOneComponentSwizzleMap[swizzle & 0x3];
            enable = _gcdLongUlongStoreOneComponentEnable;
        }
        break;

    default:
        gcmASSERT(0);
        break;
    }

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(dest, enable);
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_fourth_add_store(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    gctUINT       rowOffset;

    tyId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   Opnd,
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    return  _long_ulong_third_add_store(Context,
                                        Inst,
                                        Opnd);
}

static gctBOOL
_long_ulong_first_logical_op(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_SetLongUlongInstType(Context, Inst, gcvNULL);
}

static gctBOOL
_long_ulong_second_logical_op(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode      virErrCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    VIR_Operand   *opnd;
    gctUINT     rowOffset;

    tyId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    opnd = VIR_Inst_GetSource(Inst, 0);
    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   VIR_Inst_GetSource(Inst, 0),
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   VIR_Inst_GetSource(Inst, 1),
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    opnd = VIR_Inst_GetDest(Inst);
    sym = VIR_Operand_GetSymbol(opnd);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    VIR_Operand_SetTempRegister(opnd,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                VIR_Operand_GetTypeId(opnd));

    _long_ulong_first_logical_op(Context,
                                 Inst,
                                 Opnd);

    return gcvTRUE;
}

static gctBOOL
_long_ulong_first_logical_not_op(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _long_ulong_first_logical_op(Context,
                                        Inst,
                                        Opnd);
}

static gctBOOL
_long_ulong_second_logical_not_op(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    VIR_SymId     symId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_Operand   *opnd;
    gctUINT     rowOffset;

    tyId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   VIR_Inst_GetSource(Inst, 0),
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    opnd = VIR_Inst_GetDest(Inst);
    sym = VIR_Operand_GetSymbol(opnd);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    VIR_Operand_SetTempRegister(opnd,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                VIR_Operand_GetTypeId(opnd));

    _long_ulong_first_logical_op(Context,
                                 Inst,
                                 Opnd);

    return gcvTRUE;
}

static gctBOOL
_long_ulong_first_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_second_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Symbol    *sym = VIR_Operand_GetSymbol(Opnd);
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT       rowOffset;
    VIR_Operand   *dest;
    VIR_TypeId    tyId;

    dest = VIR_Inst_GetDest(Inst);
    tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    virErrCode = VIR_Lower_ChangeOperandByOffset(Context,
                                   Inst,
                                   Opnd,
                                   rowOffset);
    if(virErrCode != VSC_ERR_NONE) return gcvFALSE;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    if(VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd))
    {
        VIR_Operand_SetTempRegister(dest,
                                    VIR_Inst_GetFunction(Inst),
                                    symId,
                                    VIR_Operand_GetTypeId(dest));
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctUINT
_CountEnables(
    IN gcSL_ENABLE Enable
    )
{
    VIR_Enable enable = Enable;
    gctUINT count = 0;

    while(enable)
    {
        if(enable & 0x1) count++;
        enable >>= 1;
    }
    return count;
}

static gctBOOL
_isCL_Long_ulong_one_load_two_moves(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Enable enable = VIR_Inst_GetEnable(Inst);
        return enable == VIR_ENABLE_XY ||
               _CountEnables(enable) == 1;
    }
    else return gcvFALSE;
}

static gctBOOL
_isCL_Long_ulong_two_load_four_moves(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Enable enable = VIR_Inst_GetEnable(Inst);
        return (enable & 0x3) &&
               (enable & 0xc);
    }
    else return gcvFALSE;
}

static gctBOOL
_long_ulong_first_load_to_temp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable;

    enable = VIR_Inst_GetEnable(Inst) & 0x3;
    if(!enable)
    {
        enable = VIR_Inst_GetEnable(Inst) & 0xc;
    }
    switch(enable)
    {
    case VIR_ENABLE_X:
    case VIR_ENABLE_Y:
    case VIR_ENABLE_Z:
    case VIR_ENABLE_W:
        enable = VIR_ENABLE_XY;
        break;

    case VIR_ENABLE_XY:
        enable = VIR_ENABLE_XYZW;
        break;

    default:
        break;
    }

    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_second_load_to_temp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable = VIR_Inst_GetEnable(Inst);

    gcmASSERT(enable & 0x3);
    enable &= ~0x3;
    switch(enable)
    {
    case VIR_ENABLE_Z:
        enable = VIR_ENABLE_XY;
        break;

    case VIR_ENABLE_W:
    case VIR_ENABLE_ZW:
        enable = VIR_ENABLE_XYZW;
        break;

    default:
        break;
    }

    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_first_load_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable;
    VIR_Swizzle swizzle = virmSWIZZLE(X, Z, Z, Z);

    enable = VIR_Inst_GetEnable(Inst);
    switch(enable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
        enable &= 0x3;
        break;

    case VIR_ENABLE_Y:
    case VIR_ENABLE_YW:
    case VIR_ENABLE_YZ:
        swizzle = virmSWIZZLE(X, X, Z, Z);
        break;

    case VIR_ENABLE_YZW:
        enable = VIR_ENABLE_YZ;
        swizzle = virmSWIZZLE(X, X, Z, Z);
        break;

    case VIR_ENABLE_Z:
    case VIR_ENABLE_ZW:
        swizzle = virmSWIZZLE(X, X, X, Z);
        break;

    case VIR_ENABLE_W:
        swizzle = virmSWIZZLE(X, X, X, X);
        break;

    default:
        break;
    }
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_second_load_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT       rowOffset;
    VIR_Operand   *dest = VIR_Inst_GetDest(Inst);
    VIR_Enable enable = VIR_ENABLE_XY;
    VIR_Swizzle swizzle = virmSWIZZLE(Y, W, W, W);

    enable = VIR_Inst_GetEnable(Inst);
    switch(enable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XYZW:
        enable &= 0x3;
        break;

    case VIR_ENABLE_Y:
    case VIR_ENABLE_YW:
    case VIR_ENABLE_YZ:
        swizzle = virmSWIZZLE(Y, Y, W, W);
        break;

    case VIR_ENABLE_YZW:
        enable = VIR_ENABLE_YZ;
        swizzle = virmSWIZZLE(Y, Y, W, W);
        break;

    case VIR_ENABLE_Z:
    case VIR_ENABLE_ZW:
        swizzle = virmSWIZZLE(Y, Y, Y, W);
        break;

    case VIR_ENABLE_W:
        swizzle = virmSWIZZLE(Y, Y, Y, Y);
        break;

    default:
        break;
    }

    tyId = VIR_Operand_GetTypeId(dest);
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    VIR_Operand_SetTempRegister(dest,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                VIR_Operand_GetTypeId(dest));

    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_third_load_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Enable enable;
    VIR_Swizzle swizzle = virmSWIZZLE(X, X, X, X);

    enable = VIR_Inst_GetEnable(Inst);
    switch(enable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYZW:
        enable &= ~0x3;
        swizzle = virmSWIZZLE(X, X, X, Z);
        break;

    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZW:
        enable = VIR_ENABLE_W;
        break;

    default:
        gcmASSERT(0);
        enable = VIR_ENABLE_NONE;
        break;
    }
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_fourth_load_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode      virErrCode = VSC_ERR_NONE;
    VIR_TypeId  tyId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    VIR_Operand   *opnd;
    gctUINT     rowOffset;
    VIR_Enable enable;
    VIR_Swizzle swizzle = virmSWIZZLE(Y, Y, Y, Y);

    enable = VIR_Inst_GetEnable(Inst);
    switch(enable)
    {
    case VIR_ENABLE_XYZ:
    case VIR_ENABLE_XYZW:
        enable &= ~0x3;
        swizzle = gcmSWIZZLE(Y, Y, Y, W);
        break;

    case VIR_ENABLE_XYW:
    case VIR_ENABLE_XZW:
    case VIR_ENABLE_YZW:
        enable = VIR_ENABLE_W;
        break;

    default:
        gcmASSERT(0);
        enable = VIR_ENABLE_NONE;
        break;
    }

    opnd = VIR_Inst_GetDest(Inst);
    tyId = VIR_Operand_GetTypeId(opnd);
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    sym = VIR_Operand_GetSymbol(opnd);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    VIR_Operand_SetTempRegister(opnd,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                VIR_Operand_GetTypeId(Opnd));
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), enable);
    return _SetLongUlongInstTypeOnly(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_lower(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_upper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_TypeId    tyId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT     rowOffset;
    gctUINT     indexOffset;
    VIR_PrimitiveTypeId  format;
    VIR_ScalarConstVal imm;

    tyId = VIR_Operand_GetTypeId(Opnd);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    sym = VIR_Operand_GetSymbol(Opnd);
    switch(VIR_Symbol_GetKind(sym)) {
    case VIR_SYM_CONST:
        imm.uValue = 0;
        tyId = VIR_Lower_GetBaseType(Context->shader, Opnd);
        format = VIR_GetTypeComponentType(tyId);
        if(format == VIR_TYPE_INT32 ||
           format == VIR_TYPE_INT16 ||
           format == VIR_TYPE_INT8 ||
           format == VIR_TYPE_INT64)
        {
            VIR_ConstId   constId;
            constId = VIR_Operand_GetConstId(Opnd);

            if(constId == VIR_INVALID_ID) {
                gcmASSERT(0);
            }
            else {
                VIR_Const  *vConst = gcvNULL;

                vConst = VIR_Shader_GetConstFromId(Context->shader,
                                                   constId);
                if(vConst->value.scalarVal.iValue < 0)
                {
                    imm.uValue = 0xFFFFFFFF;
                }
            }
        }
        VIR_Operand_SetImmediate(Opnd, VIR_TYPE_UINT32, imm);
        break;

    case VIR_SYM_UNIFORM:
        indexOffset = VIR_Operand_GetConstIndexingImmed(Opnd) + rowOffset;
        VIR_Operand_SetRelIndexingImmed(Opnd, indexOffset);
        break;

    default:
        regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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
                                    VIR_Operand_GetTypeId(Opnd));
        break;
    }

    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_set_lower(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

static gctBOOL
_long_ulong_set_upper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Operand   *dest;
    VIR_TypeId    tyId;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT     rowOffset;

    dest = VIR_Inst_GetDest(Inst);
    tyId = VIR_Operand_GetTypeId(dest);
    gcmASSERT(VIR_GetTypeRows(tyId) > 1);
    rowOffset = VIR_GetTypeRows(tyId) >> 1;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    VIR_Operand_SetTempRegister(dest,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                VIR_Operand_GetTypeId(dest));
    return VIR_Lower_SetLongUlongInstType(Context, Inst, Opnd);
}

/* singned char/short/int to ulong convert with _sat mode enable */
static gctBOOL
_isI2I_int2ulong_sat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        if (VIR_Operand_GetModifier(dest) == VIR_MOD_NONE) return gcvFALSE;

        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);
        if((srcFormat == VIR_TYPE_INT32 ||
            srcFormat == VIR_TYPE_INT16 ||
            srcFormat == VIR_TYPE_INT8) &&
            (dstFormat == VIR_TYPE_UINT64)) return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_isI2I_int2long_sat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        if (VIR_Operand_GetModifier(dest) == VIR_MOD_NONE) return gcvFALSE;

        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);
        if((srcFormat == VIR_TYPE_INT32 ||
            srcFormat == VIR_TYPE_INT16 ||
            srcFormat == VIR_TYPE_INT8) &&
            (dstFormat == VIR_TYPE_INT64)) return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
int2ulong_sat_cmp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId typeId;
    gctUINT components;
    VIR_ScalarConstVal imm0;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);

    imm0.iValue = 0;
    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    return gcvTRUE;
}

/* singend char/short/int 2 long or ulong */
static gctBOOL
_isI2I_int2longulong(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        if (VIR_Operand_GetModifier(dest) != VIR_MOD_NONE &&
            (dstFormat == VIR_TYPE_UINT64 || dstFormat == VIR_TYPE_INT64)) return gcvFALSE;

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);

        if((srcFormat == VIR_TYPE_INT32 ||
            srcFormat == VIR_TYPE_INT16 ||
            srcFormat == VIR_TYPE_INT8) &&
            (dstFormat == VIR_TYPE_INT64 || dstFormat == VIR_TYPE_UINT64)) return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
int2int32conv(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);
    gcmASSERT(format == VIR_TYPE_INT64);

    if(format == VIR_TYPE_INT64) format = VIR_TYPE_INT32;

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);
    VIR_Inst_SetInstType(Inst, typeId);
    return gcvTRUE;
}

static gctBOOL
int2long_sign_bit_set(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_Operand *src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand *src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT     rowOffset;

    /* dest = src0 == src1 ? src1 : src2 */
    VIR_Operand_SetImmediateInt(src1, 0);
    VIR_Operand_SetImmediateInt(src2, 0xFFFFFFFF);

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);
    gcmASSERT(format == VIR_TYPE_INT64);
    if(format == VIR_TYPE_INT64) format = VIR_TYPE_INT32;

    gcmASSERT(VIR_GetTypeRows(typeId) > 1);
    rowOffset = VIR_GetTypeRows(typeId) >> 1;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);

    VIR_Operand_SetTempRegister(dest,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                typeId);
    VIR_Operand_SetModifier(dest, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(dest, VIR_ROUND_DEFAULT);
    return gcvTRUE;
}

static gctBOOL
int2longulong_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);

    if(format == VIR_TYPE_INT64) format = VIR_TYPE_INT32;
    else if(format == VIR_TYPE_UINT64) format = VIR_TYPE_UINT32;

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);
    VIR_Inst_SetInstType(Inst, typeId);
    return gcvTRUE;
}

static gctBOOL
int2longulong_rshift(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId typeId;
    gctUINT components;
    VIR_ScalarConstVal imm0;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);

    imm0.iValue = 31;
    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    return gcvTRUE;
}

static gctBOOL
int2longulong_sign_bit_set(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT     rowOffset;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);

    if(format == VIR_TYPE_INT64) format = VIR_TYPE_INT32;
    else format = VIR_TYPE_UINT32;

    gcmASSERT(VIR_GetTypeRows(typeId) > 1);
    rowOffset = VIR_GetTypeRows(typeId) >> 1;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);

    VIR_Operand_SetTempRegister(dest,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                typeId);

    return gcvTRUE;
}

/* uchar/ushort/uint 2 long or ulong */
static gctBOOL
_isI2I_uint2longulong(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);

        if((srcFormat == VIR_TYPE_UINT32 ||
            srcFormat == VIR_TYPE_UINT16 ||
            srcFormat == VIR_TYPE_UINT8) &&
            (dstFormat == VIR_TYPE_INT64 || dstFormat == VIR_TYPE_UINT64)) return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
uint2longulong_first_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return uint_value_type0(Context, Inst, Opnd);
}

static gctBOOL
uint2longulong_second_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VSC_ErrCode   virErrCode = VSC_ERR_NONE;
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId typeId;
    gctUINT components;
    VIR_Symbol    *sym;
    VIR_VirRegId  regId;
    VIR_SymId     symId;
    gctUINT     rowOffset;
    VIR_ScalarConstVal imm0;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);

    gcmASSERT(VIR_GetTypeRows(typeId) > 1);
    rowOffset = VIR_GetTypeRows(typeId) >> 1;

    sym = VIR_Operand_GetSymbol(dest);
    regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_UINT32, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);

    VIR_Operand_SetTempRegister(dest,
                                VIR_Inst_GetFunction(Inst),
                                symId,
                                typeId);

    imm0.iValue = 0;
    VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    return gcvTRUE;
}

/* longulong to sus convert */
static gctBOOL
_isI2I_longulong2sus(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong_src0(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        if (VIR_Operand_GetModifier(dest) != VIR_MOD_NONE) return gcvFALSE;
        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);

        if((srcFormat == VIR_TYPE_UINT64 || srcFormat == VIR_TYPE_INT64) &&
            (dstFormat == VIR_TYPE_INT32 || dstFormat == VIR_TYPE_UINT32 ||
             dstFormat == VIR_TYPE_INT16 || dstFormat == VIR_TYPE_UINT16 ||
             dstFormat == VIR_TYPE_INT8 || dstFormat == VIR_TYPE_UINT8))
            return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
longulong2usu_mov(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);

    if(format == VIR_TYPE_INT64) format = VIR_TYPE_INT32;
    else if(format == VIR_TYPE_UINT64) format = VIR_TYPE_UINT32;

    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);
    VIR_Inst_SetInstType(Inst, typeId);
    VIR_Operand_SetTypeId(Opnd, typeId);

    return gcvTRUE;
}

/* long ulong convert while _sat mode disable */
static gctBOOL
_isI2I_longulongConvert(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if(_hasInteger_long_ulong(Context, Inst))
    {
        VIR_Operand *dest, *src;
        VIR_PrimitiveTypeId  dstFormat, srcFormat;
        VIR_TypeId typeId;

        dest = VIR_Inst_GetDest(Inst);
        if (VIR_Operand_GetModifier(dest) != VIR_MOD_NONE) return gcvFALSE;
        typeId = VIR_Lower_GetBaseType(Context->shader, dest);
        dstFormat = VIR_GetTypeComponentType(typeId);

        src = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Lower_GetBaseType(Context->shader, src);
        srcFormat = VIR_GetTypeComponentType(typeId);

        if((srcFormat == VIR_TYPE_UINT64 || srcFormat == VIR_TYPE_INT64) &&
            (dstFormat == VIR_TYPE_INT64 || dstFormat == VIR_TYPE_UINT64))
            return gcvTRUE;
    }
    return gcvFALSE;
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
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destPackedLE4Components }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst0[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0Unpacked, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0Unpacked, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setNonpackedTypeByPatternDest, _setNonpackedTypeByPatternDest, _setNonpackedTypeByPatternDest } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, _revise2PackedTypeAndSwizzle, _setPackedSwizzle, _setPackedMaskValue } },
};

static VIR_PatternMatchInst _divPatInst1[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destChar_P8Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst1[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder0UnPackedSwizzle, _setRowOrder0UnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder0UnPackedSwizzle, _setRowOrder0UnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setRowOrder0PackedSwizzle, _setRowOrder0PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder1UnPackedSwizzle, _setRowOrder1UnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder1UnPackedSwizzle, _setRowOrder1UnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setRowOrder1PackedSwizzle, _setRowOrder1PackedMaskValue } },
};

static VIR_PatternMatchInst _divPatInst2[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destChar_P16Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst2[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn1PackedSwizzle, _setColumn1PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn2UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn2UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn2PackedSwizzle, _setColumn2PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn3UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn3UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn3PackedSwizzle, _setColumn3PackedMaskValue } },
};

static VIR_PatternMatchInst _divPatInst3[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destShort_P8Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst3[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_DIV, 0, 0, { -3, -1, -2, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, revise_operand_type_by_dest_type, _setColumn1PackedSwizzle, _setColumn1PackedMaskValue } },
};

static VIR_PatternMatchInst _divPatInst4[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasNEW_SIN_COS_LOG_DIV, VIR_Lower_IsFloatOpcode, _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _divRepInst4[] = {
    { VIR_OP_RCP, 0, 0, { 1, 3, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _divPatInst5[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsIntOpcode, _hasNEW_SIN_COS_LOG_DIV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst5[] = {
    { VIR_OP_DIV, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

    /*
        DIV 1, 2, 3
            rcp TEMP1, 0, 0, 3
            mul 1, 2, TEMP1, 0
    */
static VIR_PatternMatchInst _divPatInst6[] = {
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_enableFullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _divRepInst6[] = {
    { VIR_OP_RCP, 0, 0, { -1, 3, 0, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -1, 0 }, { 0 } },
};

static VIR_Pattern _divPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_div, 5) },
    /* now gcSL CG cannot handle expanded PRE_DIV, enable this if use new VIR CG */
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE, CODEPATTERN(_div, 6) },
    { VIR_PATN_FLAG_NONE }
};

/*
        MOD 1, 2, 3
            imod 1, 2, 3, 0, 0
    { 1, gcSL_MOD, 1, 2, 3, 0, 0, _IntOpcode },
        { -1, 0x48, 1, 2, 3, 0, 0, value_type0 },
*/

static VIR_PatternMatchInst _modPatInst0[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destPackedLE4Components }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst0[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0Unpacked, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0Unpacked, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setNonpackedTypeByPatternDest, _setNonpackedTypeByPatternDest, _setNonpackedTypeByPatternDest } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, _revise2PackedTypeAndSwizzle, _setPackedSwizzle, _setPackedMaskValue } },
};

static VIR_PatternMatchInst _modPatInst1[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destChar_P8Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst1[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder0UnPackedSwizzle, _setRowOrder0UnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder0UnPackedSwizzle, _setRowOrder0UnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setRowOrder0PackedSwizzle, _setRowOrder0PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder1UnPackedSwizzle, _setRowOrder1UnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0RowUnpacked, _setRowOrder1UnPackedSwizzle, _setRowOrder1UnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setRowOrder1PackedSwizzle, _setRowOrder1PackedMaskValue } },
};

static VIR_PatternMatchInst _modPatInst2[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destChar_P16Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst2[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn1PackedSwizzle, _setColumn1PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn2UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn2UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn2PackedSwizzle, _setColumn2PackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn3UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn3UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { _setDestTypeFromSrc0 } },

    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, 0, _setColumn3PackedSwizzle, _setColumn3PackedMaskValue } },
};

static VIR_PatternMatchInst _modPatInst3[] = {
    { VIR_OP_MOD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _destShort_P8Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _modRepInst3[] = {
    { VIR_OP_MOD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_SWIZZLE, 0, 0, { -2, 3, 3, 3 }, { 0, _setDestTypeFromSrc0ColumnUnpacked, _setColumn1UnPackedSwizzle, _setColumnUnPackedMaskValue } },
    { VIR_OP_MOD, 0, 0, { -3, -1, -2, 0 }, { 0 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 2, 2 }, { 0, revise_operand_type_by_dest_type, _setColumn1PackedSwizzle, _setColumn1PackedMaskValue } },
};

static VIR_Pattern _modPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mod, 3) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sinPatInst0[] = {
    { VIR_OP_SIN, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasNEW_SIN_COS_LOG_DIV, _requireSIN_COS_TAN_Precision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _sinRepInst0[] = {
    { VIR_OP_JMPC, VIR_COP_LESS, 0, { 0, 2, 0, 0 }, { 0, _set_ABS, _115_pi } },
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { VIR_Lower_SetHighp, 0, rcppi } },
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
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { VIR_Lower_SetHighp, 0, rcppi } },
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
    { VIR_OP_MULLO, 0, 0, { -1, 2, 0, 0 }, { VIR_Lower_SetHighp, 0, rcppi } },
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
    { VIR_OP_SET, VIR_COP_GREATER, 0, { -1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetOne} },
    { VIR_OP_RCP, 0, 0, { -2, 2, 0, 0 }, { 0 } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -2, -1, -2, 2 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { -3, -2, -2, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, 0, 0 }, { 0, 0, atan9_1_atan7_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, atan5_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, atan3_2 } },
    { VIR_OP_MAD, 0, 0, { -7, -3, -7, 0 }, { 0, 0, 0, VIR_Lower_SetOne } },
    { VIR_OP_MUL, 0, 0, { -7, -2, -7, 0 }, { 0 } },
    { VIR_OP_ABS, 0, 0, { -4, -7, 0, 0 }, { 0 } },
    { VIR_OP_SUB, 0, 0, { -2, 0, -4, 0 }, { 0, half_pi } },
    { VIR_OP_NEG, 0, 0, { -5, -2, 0, 0 }, { 0 } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { -2, 2, -5, -2 }, { 0 } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, {  1, -1, -2, -7 }, { 0 } },
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
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform, _supportVectorB0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst1[] = {
    { VIR_OP_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _setOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -3, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -2, -3, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst2[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform, _supportScalarB0Only }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst2[] = {
    { VIR_OP_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _setOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -3, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -2, -3, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst3[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst3[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloatUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _setOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableFloatUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst4[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform, _isRAEnabled_src1_uniform, _supportScalarB0Only }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst4[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _setOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst5[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst5[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _setOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst6[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst6[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, VIR_Lower_SetLongUlongInstType, _resetOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, _SetLongUlongDestNextRegInstType, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst7[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_uniform }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst7[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType, _resetOperandUniformIndex } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst8[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_src0_not_sampler_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst8[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, VIR_Lower_SetLongUlongInstType } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, _SetLongUlongDestNextRegInstType } },
};

static VIR_PatternMatchInst _ldarrPatInst9[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst9[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, VIR_Lower_SetLongUlongInstType } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_1 } },
    { VIR_OP_MOVA, 0, 0, { -1, -2, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, _SetLongUlongDestNextRegInstType} },
};

static VIR_PatternMatchInst _ldarrPatInst10[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _hasB0_VG }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst10[] = {
    { VIR_OP_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

/* b0 has to be interger */
static VIR_PatternMatchInst _ldarrPatInst11[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform, _supportVectorB0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst11[] = {
    { VIR_OP_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst12[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform, _supportScalarB0Only }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst12[] = {
    { VIR_OP_F2I, 0, 0, { -1, 3, 0, 0 }, { _setEnableInt } },
    { VIR_OP_MOVA, 0, 0, { -2, -1, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -2, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst13[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst13[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloatUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst14[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform, _isRAEnabled_src1_uniform, _supportScalarB0Only }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst14[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableXIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst15[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform, _isRAEnabled_src1_uniform }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ldarrRepInst15[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableIntUniform } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, 0, _setOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst16[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst16[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat, 0, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst17[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_uniform }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst17[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleInt, 0, _resetOperandUniformIndex } },
};

static VIR_PatternMatchInst _ldarrPatInst18[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_src0_not_sampler_src1_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst18[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 3, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_LDARR, 0, 0, {  1, 2, -1, 0 }, { _setLDARRSwizzleFloat } },
};

static VIR_PatternMatchInst _ldarrPatInst19[] = {
    { VIR_OP_LDARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _ldarrRepInst19[] = {
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
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 9) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 10) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 11) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 12) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 13) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 14) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 15) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 16) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 17) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 18) },
    { VIR_PATN_FLAG_SET_TEMP_IN_FUNC, CODEPATTERN(_ldarr, 19) },
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
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled_dest_not_sampler_src0_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst1[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { 0, _setSTARRSwizzleFloat, _long_ulong_first_mov } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { 0, _setSTARRSwizzleFloat, _long_ulong_second_mov } },
};

static VIR_PatternMatchInst _starrPatInst2[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled_dest_not_sampler_src0_float }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst2[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { _setSTARRSwizzleFloat } },
};

static VIR_PatternMatchInst _starrPatInst3[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _hasInteger_long_ulong_isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst3[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableFloat } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { 0, _setSTARRSwizzleInt, _long_ulong_first_mov} },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { 0, _setSTARRSwizzleInt, _long_ulong_second_mov } },
};


static VIR_PatternMatchInst _starrPatInst4[] = {
    { VIR_OP_STARR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isRAEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _starrRepInst4[] = {
    { VIR_OP_MOVA, 0, 0, { -1, 2, 0, 0 }, { _setMOVAEnableInt } },
    { VIR_OP_STARR, 0, 0, {  1, -1, 3, 0 }, { _setSTARRSwizzleInt } },
};

static VIR_Pattern _starrPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_starr, 4) },
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
    { VIR_OP_SELECT, VIR_COP_GREATER_ZERO, 0, { -1, 2, 2, 0 }, { 0, 0, 0, VIR_Lower_SetZeroOrSamllestPositive } },
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
    { VIR_OP_SELECT, VIR_COP_GREATER_ZERO, 0, { -3, -2, -1, -2 }, { 0 } },
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
    { VIR_OP_SELECT, VIR_COP_GREATER_ZERO, 0, { 1, 2, 0, 2 }, { 0, 0, VIR_Lower_SetOne} },
    { VIR_OP_SELECT, VIR_COP_EQUAL, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_SELECT, VIR_COP_LESS_ZERO, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetMinusOne } },
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
    { VIR_OP_SELECT, VIR_COP_GREATER_ZERO, 0, { 1, 2, 0, 2 }, { 0, 0, VIR_Lower_SetIntOne } },
    { VIR_OP_SELECT, VIR_COP_EQUAL, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_SELECT, VIR_COP_LESS_ZERO, 0, { 1, 2, 0, 1 }, { 0, 0, VIR_Lower_SetIntMinusOne } },
};

static VIR_Pattern _signPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sign, 2) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _sqrtPatInst0[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 1, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 1, 3, 1, 0 }, { 0,_isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst0[] = {
    { VIR_OP_RSQ, 0, 0, { 1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _sqrtPatInst1[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 1, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 2, 3, 1, 0 }, { 0,_isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst1[] = {
    { VIR_OP_RSQ, 0, 0, { 2, 1, 0, 0 }, { 0 } },
    { VIR_OP_SQRT, 0, 0, { 1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _sqrtPatInst2[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_DIV, VIR_PATTERN_ANYCOND, 0, { 3, 4, 1, 0 }, { 0,_isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst2[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_RSQ, 0, 0, { 3, 2, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _sqrtPatInst3[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 1, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_RCP, VIR_PATTERN_ANYCOND, 0, { 1, 1, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst3[] = {
    { VIR_OP_RSQ, 0, 0, { 1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _sqrtPatInst4[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 1, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_RCP, VIR_PATTERN_ANYCOND, 0, { 2, 1, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst4[] = {
    { VIR_OP_RSQ, 0, 0, { 2, 1, 0, 0 }, { 0 } },
    { VIR_OP_SQRT, 0, 0, { 1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _sqrtPatInst5[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_RCP, VIR_PATTERN_ANYCOND, 0, { 3, 1, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst5[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
    { VIR_OP_RSQ, 0, 0, { 3, 2, 0, 0 }, { 0 } },
};

    /*
        SQRT 1, 2
            rsq 1, 0, 0, 2, 0
            rcp 1, 0, 0, 1, 0
    */
static VIR_PatternMatchInst _sqrtPatInst6[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _hasSQRT_TRIG }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst6[] = {
    { VIR_OP_SQRT, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

    /*
        SQRT 1, 2
            rsq 1, 0, 0, 2, 0
            rcp 1, 0, 0, 1, 0
    */
static VIR_PatternMatchInst _sqrtPatInst7[] = {
    { VIR_OP_SQRT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _sqrtRepInst7[] = {
    { VIR_OP_RSQ, 0, 0, { -1, 2, 0, 0 }, { 0 } },
    { VIR_OP_RCP, 0, 0, {  1, -1, 0, 0 }, { 0 } },
};

static VIR_Pattern _sqrtPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_sqrt, 7) },
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

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtp) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x26, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst0[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat_Rtp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst0[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_CEIL, 0, 2, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_CONVERT, 0, 0, {  1, -3, 0, 0 }, { value_type0_32bit_reset_sat_rounding } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat_Rtn) },
        { -4, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -3, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -2, 0x25, 1, 0, 0, 1},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst1[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat_Rtn }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst1[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_FLOOR, 0, 2, { -3, -2, 0, 0 }, { 0 } },
    { VIR_OP_CONVERT, 0, 0, {  1, -3, 0, 0 }, { value_type0_32bit_reset_sat_rounding } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtp) },
        { -2, 0x26, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */

static VIR_PatternMatchInst _convPatInst2[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Rtp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst2[] = {
    { VIR_OP_CEIL, 0, 0, { 1, 2, 0, 0 }, { 0, revise_dest_type_by_operand_type } },
    { VIR_OP_CONVERT, 0, 0, { 1, 1, 0, 0 }, { value_type0_32bit_reset_sat_rounding, set_opnd_type_prevInst_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Rtn) },
        { -2, 0x25, 1, 0, 0, 2},
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst3[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Rtn }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst3[] = {
    { VIR_OP_FLOOR, 0, 0, { 1, 2, 0, 0 }, { 0, revise_dest_type_by_operand_type } },
    { VIR_OP_CONVERT, 0, 0, { 1, 1, 0, 0 }, { value_type0_32bit_reset_sat_rounding, set_opnd_type_prevInst_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isF2I_Sat) },
        { -3, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -2, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
        { -1, 0x2E, 1, 1, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst4[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I_Sat }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst4[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER,0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_CONVERT, 0, 0, {  1, -2, 0, 0 }, { value_type0_32bit_reset_sat } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isF2I },
        { -1, 0x2E, 1, 2, 0, 0, 0, value_type0_32bit },
    */
static VIR_PatternMatchInst _convPatInst5[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destPackedType, _isF2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst5[] = {
    { VIR_OP_CONVERT, -1, 0, { -1, 2, 0, 0 }, { value_type0_32bit_pattern_dest } },
    { VIR_OP_SWIZZLE, 0, 0, {  1, -1, 1, 1 }, { 0, _revise2PackedTypeAndSwizzle, _setPackedSwizzle, _setPackedMaskValue } },
};

static VIR_PatternMatchInst _convPatInst6[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst6[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2, 0, 0 }, { value_type0_32bit } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2F },
        { -1, 0x2D, 1, 2, 0, 0, 0, _value_type0_32bit_from_src0 },
    */
static VIR_PatternMatchInst _convPatInst7[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _src0PackedType, _isI2F }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst7[] = {
    { VIR_OP_SWIZZLE, 0, 0, { -1, 2, 2, 2 }, { value_type0_from_src0_unpacked, 0, _setUnPackedSwizzle, _setUnPackedMaskValue } },
    { VIR_OP_CONVERT, -1, 0, {  1, -1, 0, 0 }, { 0, _revise2UnPackedTypeAndSwizzle } },
};

static VIR_PatternMatchInst _convPatInst8[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2F }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst8[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2, 0, 0 }, { _value_type0_32bit_from_src0 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2us) },
        { -2, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
        { -1, 0x0F, 1, 1, gcSL_CG_CONSTANT, 1, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst9[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destOrSrc0PackedType, _isI2I_Sat_s2us }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst9[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER, 0, { -1, 2, 0, 2 }, { max_type0_const, _setDestTypeFromSrc0 } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { -2, -1, 0, -1 }, { min_type0_const, _setDestTypeFromSrc0 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -2, 2, 2 }, { _reset_sat_rounding, _equatePackedTypeForDestOrSrc, _setConvPackedSwizzle, _setConvPackedMaskValue } },
};
static VIR_PatternMatchInst _convPatInst10[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst10[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER, 0, { -1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
    { VIR_OP_SELECT, VIR_COP_LESS, 0, {  1, -1, 0, -1 }, { min_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_u2us) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, max_type0_const_conditionGT },
    */
static VIR_PatternMatchInst _convPatInst11[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destOrSrc0PackedType, _isI2I_Sat_u2us }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst11[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER, 0, { -1, 2, 0, 2 }, { max_type0_const, _setDestTypeFromSrc0 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -1, 2, 2 }, { _reset_sat_rounding, _equatePackedTypeForDestOrSrc, _setConvPackedSwizzle, _setConvPackedMaskValue } },
};

static VIR_PatternMatchInst _convPatInst12[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_u2us }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst12[] = {
    { VIR_OP_SELECT, VIR_COP_GREATER, 0, { 1, 2, 0, 2 }, { max_type0_const, revise_dest_type_by_operand_type } },
};
    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isI2I_Sat_s2u) },
        { -1, 0x0F, 1, 2, gcSL_CG_CONSTANT, 2, 0, min_type0_const_conditionLT },
    */
static VIR_PatternMatchInst _convPatInst13[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_Sat_s2u }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst13[] = {
    { VIR_OP_SELECT, VIR_COP_LESS, 0, { 1, 2, 0, 2 }, { min_type0_const, revise_dest_type_by_operand_type } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, _isI2I },
        { -1, 0x2C, 1, 2, gcSL_CG_CONSTANT, 0, 0, value_types_I2I },
    */
static VIR_PatternMatchInst _convPatInst14[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _destOrSrc0PackedType, _isI2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst14[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { _reset_sat_rounding, _destUnsignedRankHigher_setSrcToZero_elseNop } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 2, 2 }, { _reset_sat_rounding, _equatePackedTypeForDestOrSrc, _setConvPackedSwizzle, _setConvPackedMaskValue } },
    { VIR_OP_LSHIFT, 0, 0, { -1, 1, 2, 0 }, { 0, 0, _destSignedRankHigher_setSrcToShiftCount_elseNop } },
    { VIR_OP_RSHIFT, 0, 0, { 1, -1, 2, 0 }, { 0, VIR_Lower_SkipOperand, _destSignedRankHigher_setSrcToShiftCount_elseNop } },
};

static VIR_PatternMatchInst _convPatInst15[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_int2ulong_sat }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst15[] = {
    { VIR_OP_CMP, VIR_COP_GREATER, 0, { -1, 2, 2, 2 }, { 0, 0, int2ulong_sat_cmp} },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, uint2longulong_first_mov } },
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, uint2longulong_second_mov } },
};

    /*
      CONVERT            long temp(6).sat.rtz.x, char temp(5).x
      ==>
      CONVERT            int temp(6).sat.rtz.x, char temp(5).x
      RSHIFT             int temp(272).x, char temp(5).x, int 31
      SELECT.eq          int temp(7).x, int temp(272).x, int 0, int -1
     */
static VIR_PatternMatchInst _convPatInst16[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_int2long_sat }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst16[] = {
    { VIR_OP_CONVERT, 0, 0, { 1, 2, 0, 0 }, { int2int32conv } },
    { VIR_OP_SELECT, VIR_COP_GREATER_OR_EQUAL_ZERO, 0, { 1, 2, 0, 0 }, { 0, int2long_sign_bit_set } },
};

static VIR_PatternMatchInst _convPatInst17[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_int2longulong }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst17[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, int2longulong_mov } },
    { VIR_OP_RSHIFT, 0, 0, { -1, 2, 2, 0 }, { 0, 0, int2longulong_rshift } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, int2longulong_sign_bit_set } },
};

static VIR_PatternMatchInst _convPatInst18[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_uint2longulong }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst18[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, uint2longulong_first_mov } },
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, uint2longulong_second_mov } },
};

static VIR_PatternMatchInst _convPatInst19[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_longulong2sus }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst19[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, longulong2usu_mov } },
};

static VIR_PatternMatchInst _convPatInst20[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I_longulongConvert }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst20[] = {
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, _long_ulong_first_mov } },
    { VIR_OP_MOV, 0, 0, { 1, 2, 0, 0 }, { 0, _long_ulong_second_mov } },
};

static VIR_PatternMatchInst _convPatInst21[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst21[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Signed_8_16) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24_16 },
    */
static VIR_PatternMatchInst _convPatInst22[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Signed_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst22[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_24_16 } },
    { VIR_OP_RSHIFT, 0, 0, { 1, -1, 0, 0 }, { int_value_type0_const_24_16 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_isCL_X_Unsigned_8_16) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, uint_value_type0_const_FF_FFFF },
    */
static VIR_PatternMatchInst _convPatInst23[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X_Unsigned_8_16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst23[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { uint_value_type0_const_FF_FFFF } },
};


    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_8bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FF },
    */
static VIR_PatternMatchInst _convPatInst24[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_8bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst24[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, short_value_type0_const_8 },
    */
static VIR_PatternMatchInst _convPatInst25[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst25[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { short_value_type0_const_8 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { short_value_type0_const_8 } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_16bit) },
        { -1, 0x5D, 1, 2, 0, gcSL_CG_CONSTANT, 0, value_type0_const_FFFF },
    */
static VIR_PatternMatchInst _convPatInst26[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_16bit }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst26[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 0, 0 }, { value_type0_const_FFFF } },
};

    /*
    { 1, gcSL_CONV, 1, 2, 0, 0, 0, USE_WITH_VIR(_is_dest_32bit_src_int8) },
        { -2, 0x59, 1, 2, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
        { -1, 0x5A, 1, 1, 0, gcSL_CG_CONSTANT, 0, int_value_type0_const_24 },
    */
static VIR_PatternMatchInst _convPatInst27[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int8 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst27[] = {
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
static VIR_PatternMatchInst _convPatInst28[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _is_dest_32bit_src_int16 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst28[] = {
    { VIR_OP_LSHIFT, 0, 0, { -1, 2, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_RSHIFT, 0, 0, {  1, -1, 0, 0 }, { int_value_type0_const_16 } },
};

static VIR_PatternMatchInst _convPatInst29[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { supportCONV }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst29[] = {
    { VIR_OP_CONVERT, -1, 0, { 1, 2, 0, 0 }, { 0 } },
};


static VIR_PatternMatchInst _convPatInst32[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF16_2_F32_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst32[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { float16_exp } },
    { VIR_OP_LSHIFT, 0, 0, { -1, -1, 0, 0 }, { float16_man_bits } },
    { VIR_OP_COMPARE, VIR_COP_EQUAL, 0, { -2, -1, 0, 0 }, { _setBooleanType, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -2, -2, 0, 0 }, { float16_exp_iszero } },
    { VIR_OP_AND_BITWISE, 0, 0, { -3, -1, 0, 0 }, { float16_exp_isnan } },
    { VIR_OP_COMPARE, VIR_COP_NOT_EQUAL,0, { -4, -3, 0, 0 }, { _setBooleanType, float16_exp_isnan } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -2, -4, -2, 0 }, { float16_exp_isaddnanNZ } },
    { VIR_OP_ADD, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } }, /* NOTE!!!!!!!!! */
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_sign } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { value_types_32 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -2, 2, 0, 0 }, { float16_man } },
    { VIR_OP_LSHIFT, 0, 0, { -2, -2, 0, 0 }, { float16_man_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { value_types_32 } },
};

static VIR_PatternMatchInst _convPatInst33[] = {
    { VIR_OP_CONVERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF32_2_F16_hasCMP_FullNewLinker }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst33[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { value_types_32, 0, float32_exp } }, /* -1 is 8 bit exp */
    { VIR_OP_COMPARE, VIR_COP_LESS_OR_EQUAL, 0, { -2, -1, 0, 0 }, { _setBooleanType, 0, float32_exp_underflow } }, /* -2 is exp_underflow */
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -3, -2, -1, 0 }, { value_types_32, 0, 0, float32_exp_underflow } }, /* -3 is to-sub */
    { VIR_OP_SUB, 0, 0, { -4, -1, -3, 0 }, { value_types_32 } }, /* -4 is 5 bit exp */
    { VIR_OP_COMPARE, VIR_COP_GREATER_OR_EQUAL, 0, { -5, -1, 0, 0 }, { _setBooleanType, 0, float32_exp_overflow } }, /* -5 is exp_overflow */
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -4, -5, 0, -4 }, { value_types_32, 0, float32_exp_isoverflow } },
    { VIR_OP_RSHIFT, 0, 0, { -4, -4, 0, 0 }, { value_types_32, 0, float32_man_bits } },
    { VIR_OP_AND_BITWISE, 0, 0, { -6, 2, 0, 0 }, { value_types_32, 0, float32_sign } }, /* -6 is sign bit */
    { VIR_OP_RSHIFT, 0, 0, { -6, -6, 0, 0 }, { float32_exp_bits } },
    { VIR_OP_OR_BITWISE, 0, 0, { -4, -4, -6, 0 }, { value_types_16 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -7, 2, 0, 0 }, { value_types_32, 0, float32_man } }, /* -7 is 23 bit man */
    { VIR_OP_RSHIFT, 0, 0, { -8, -7, 0, 0 }, { value_types_32, 0, float32_man_bits } }, /* -8 is 10 bit man */
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -8, -5, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntZero } },
    { VIR_OP_COMPARE, VIR_COP_GREATER_OR_EQUAL, 0, { -9, -1, 0, 0 }, { _setBooleanType, 0, float32_exp } }, /* -9 is exp_overflow */
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { -8, -9, 0, -8 }, { value_types_32, 0, VIR_Lower_SetIntOne } },
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 26) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 27) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 28) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 29) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 32) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 33) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
supportCMP_single_value_jmp_2_succ2_resCondOp(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    gcmASSERT(Inst->_opcode == VIR_OP_JMPC || Inst->_opcode == VIR_OP_JMP_ANY);
    return supportCMP(Context, Inst) &&
           all_source_single_value(Context, Inst) &&
           VIR_Lower_jmp_2_succ2(Context, Inst) &&
           VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(Inst));
}

static gctBOOL
notDual16Req(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !VIR_Lower_MatchDual16Req(Context, VIR_Inst_GetDest(Inst), VIR_Inst_GetSource(VIR_Inst_GetPrev(Inst), 0));
}

/*
** In LL2MC lower, we may use CMP&SELECT to replace CMOV, but if src2 of CMOV is just the dest of CMOV, it may cause un-def usage
** and cause some issues in RA/LV, so we just disable CMOV for those cases that may be changed to CMP&SELECT.
** The best solution is analyze the DU and add an initialization for this CMOV if needed, right now we just change CMOV back to JMP.
*/
static VIR_PatternMatchInst _jmpcPatInst0[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_HasHalti4, supportCMP_single_value_jmp_2_succ2_resCondOp }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { notDual16Req }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst0[] = {
    { VIR_OP_CMOV, -1, 0, { 4, 2, 3, 5 }, { VIR_Lower_ReverseCondOp } }
};

static VIR_PatternMatchInst _jmpcPatInst1[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_float, all_source_single_value, VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst1[] = {
    { VIR_OP_SET, -1, 0, { 4, 2, 3, }, { VIR_Lower_ReverseCondOp } },
};

static VIR_PatternMatchInst _jmpcPatInst2[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_float, all_source_single_value, VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst2[] = {
    { VIR_OP_SET, -1, 0, { 4, 2, 3, }, { 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst3[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCLShader, all_source_integer, all_source_single_value, VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0IntOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst3[] = {
    { VIR_OP_CMP, -1, 0, {  4, 2, 3, 0 }, { VIR_Lower_ReverseCondOp, 0, 0, VIR_Lower_SetIntOne } },
};

static VIR_PatternMatchInst _jmpcPatInst4[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCLShader, all_source_integer, all_source_single_value, VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _isSrc0IntOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst4[] = {
    { VIR_OP_CMP, -1, 0, { 4, 2, 3, 0 }, { 0, 0, 0, VIR_Lower_SetIntOne } },
};

static VIR_PatternMatchInst _jmpcPatInst5[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, _isSrc0Zero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { canBeMergedToSelect1, _dstSrcSamePrecsion}, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst5[] = {
    { VIR_OP_CSELECT, -1, 0, { 4, 3, 8, 5 }, { setSwitchedCondOpCompareWithZero } },
};

static VIR_PatternMatchInst _jmpcPatInst6[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, _isSrc1Zero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { canBeMergedToSelect0, _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { _dstSrcSamePrecsion }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst6[] = {
    { VIR_OP_CSELECT, -1, 0, { 4, 2, 8, 5 }, { setCondOpCompareWithZero } },
};

static VIR_PatternMatchInst _jmpcPatInst7[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3, VIR_Lower_enableFullNewLinker }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { dest_type_less_than_prev_jmp_src0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

/* CMP, if dest.type is not float,
   dest = cond_op(src0, src1) ? 0xFFFFFFFF: 0,
   thus, we can use SELMSB condition for select.
   Using ALLMSB, we can resolve the issue that select has one instruction type to control
   comparison and implicit converstion */
static VIR_PatternReplaceInst _jmpcRepInst7[] = {
    { VIR_OP_COMPARE, -1, 2, { -1, 2, 3, 0 }, { _setBooleanType } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, {  4, -1, 8, 5 }, { 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst8[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { supportCMP_single_value_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 5, 0, 0 }, { dest_type_less_than_prev_jmp_src0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 6, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 7, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 4, 8, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst8[] = {
    { VIR_OP_COMPARE, -1, 2, { -1, 2, 3, 0 }, { _setIntegerType } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, {  4, -1, 8, 5 }, { 0 } },
};


static VIR_PatternMatchInst _jmpcPatInst9[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { jmp_2_succ2_resCondOp_float }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_KILL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { no_source }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst9[] = {
    { VIR_OP_KILL, -1, 0, { 0, 2, 3, 0 }, { VIR_Lower_ReverseCondOp } }
};

/* JMPC.ne          #sh_383, bool hp  temp(4272).hp.x, bool false
   JMP              #sh_385
   LABEL            #sh_383:
   MOV              hp temp(4268).hp.x, 1.000000[3f800000]
   JMP              #sh_384
   LABEL            #sh_385:
   MOV              hp temp(4268).hp.x, 0.000000[0]
   LABEL            #sh_384:
   ==> replaced by
   CMP.ne           hp temp(4268).hp.x, bool hp  temp(4272).hp.x, bool false, 1.000000[3f800000]
*/
static VIR_PatternMatchInst _jmpcPatInst10[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_single_value, VIR_Lower_jmp_2_succ2 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { VIR_Lower_jmp_2_succ4 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 5, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 6, 7, 0, 0 }, { _isSrc0FloatOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 8, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 6, 10, 0, 0 }, { _isSrc0FloatZero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 11, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst10[] = {
     { VIR_OP_CMP, -1, 0, { 6, 2, 3, 7 }, { 0, 0, 0, 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst11[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_not_float, all_source_single_value, VIR_Lower_jmp_2_succ2 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { VIR_Lower_jmp_2_succ4 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 5, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 6, 7, 0, 0 }, { _isSrc0IntOne }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 8, 0, 0, 0 }, { VIR_Lower_jmp_2_succ3 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 9, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 6, 10, 0, 0 }, { _isSrc0Zero }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 11, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst11[] = {
     { VIR_OP_CMP, -1, 0, { 6, 2, 3, 7 }, { 0, 0, 0, 0 } },
};

static VIR_PatternMatchInst _jmpcPatInst12[] = {
    { VIR_OP_JMPC, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { jmp_2_succ2_resCondOp_singleChannel }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 4, 0, 0, 0 }, { no_source }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 5, 0, 0, 0 }, { VIR_Lower_label_only_one_jmp }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpcRepInst12[] = {
    { VIR_OP_JMPC, -1, 0, { 4, 2, 3, 0 }, { VIR_Lower_ReverseCondOp } }
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 10) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 11) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_jmpc, 12) },
    { VIR_PATN_FLAG_NONE }
};


/* this is a WAR pattern, after recompiling old optimizer insert
   MOV before CMP when dst and src are in same Reg. This causes
   CMP.NOT and CMP.NZ could not be merged into SELECT*/
static VIR_PatternMatchInst _movPatInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 3, 1, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 5, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 3, 5, 6, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _movRepInst0[] = {
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 0, {  5, 2, 0, 0 }, { 0 } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, {  3, 2, 6, 4 }, { 0 } },
};

/*
    MOV 1, 2
        { -2, 0x09, 1, 0, 0, 2, 0, _long_ulong_first_mov },
        { -1, 0x09, 1, 0, 0, 2, 0, _long_ulong_second_mov },
*/
static VIR_PatternMatchInst _movPatInst1[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {_hasInteger_long_ulong}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _movRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _long_ulong_first_mov } },
    { VIR_OP_MOV, 0, 0, {  1, 2, 0, 0 }, { 0, _long_ulong_second_mov } },
};

static VIR_Pattern _movPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mov, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mov, 1) },
    { VIR_PATN_FLAG_NONE }
};

/* VIR_OP_COMPARE patterns. */
static VIR_PatternMatchInst _cmpPatInst0[] = {
    { VIR_OP_COMPARE, VIR_COP_NOT, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst0[] = {
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, {  1, 2, 5, 3 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst1[] = {
    { VIR_OP_COMPARE, VIR_COP_SELMSB, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_COMPARE, VIR_COP_SELMSB, 0, { 1, 2, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst1[] = {
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, {  1, 2, 3, 5 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst2[] = {
    { VIR_OP_COMPARE, VIR_COP_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst2[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst3[] = {
    { VIR_OP_COMPARE, VIR_COP_NOT_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst3[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NOT_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst4[] = {
    { VIR_OP_COMPARE, VIR_COP_LESS_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst4[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_LESS, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst5[] = {
    { VIR_OP_COMPARE, VIR_COP_GREATER_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst5[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_GREATER, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst6[] = {
    { VIR_OP_COMPARE, VIR_COP_LESS_OR_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst6[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_LESS_OR_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _cmpPatInst7[] = {
    { VIR_OP_COMPARE, VIR_COP_GREATER_OR_EQUAL_UQ, 0, { 1, 2, 3, 0 }, { _isHWNotSupportUnordBranch }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst7[] = {
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -1, 2, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -1, -1, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_NAN, 0, { -2, 3, 0, 0 }, { 0, } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { -2, -2, 0, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, VIR_Lower_SetUIntOne, VIR_Lower_SetZero } },
    { VIR_OP_COMPARE, VIR_COP_GREATER_OR_EQUAL, 0, {  1, 2, 3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { -1, -1, -2, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 1, -1, 0 }, { _updateOperandTypeToBool, _updateOperandTypeToBool, _updateOperandTypeToBool } },
    { VIR_OP_COMPARE, VIR_COP_NOT_ZERO, 0, {  1, 1, 0, 0 }, { 0 } },
};

/*
    COMPARE.lt       bool hp temp(4123).hp.x, hp  temp(4122).hp.x, 0.050000[3d4ccccd]
    CSELECT.selmsb   bool hp temp(4123).hp.x, bool hp  temp(4123).hp.x, uint 1, uint 0
    JMPC.ne          #sh_127, bool hp  temp(4123).hp.x, bool false
    ==> replaced by
    JMPC.lt          #sh_127, hp  temp(4122).hp.x, 0.050000[3d4ccccd]
*/

static VIR_PatternMatchInst _cmpPatInst8[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_single_value, VIR_Lower_IsDstBool }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { 1, 1, 4, 5 }, { _isSrc1IntOne, _isSrc2IntZero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 6, 1, 7, 0 }, { _isSrc1IntZero }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst8[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { 1, 1, 4, 5 }, { 0, 0, 0, 0 } },
    { VIR_OP_JMPC, -1, 0, { 6, 2, 3, 0 }, { 0, 0, 0, 0 } },
};

/*
  COMPARE.eq       bvec3 hp temp(4253).hp.xyz, uvec3 hp  temp(4251).hp.xyz, uvec3 hp  temp(4252).hp.xyz
  CSELECT.selmsb   bvec3 hp temp(4253).hp.xyz, bvec3 hp  temp(4253).hp.xyz, uint 1, uint 0
  ALL              bool hp temp(4254).hp.x, bvec3 hp  temp(4253).hp.xyz
  ==> replaced by
  VIR_OP_COMPARE   bvec3 hp temp(4253).hp.xyz, uvec3 hp  temp(4251).hp.xyz, uvec3 hp  temp(4252).hp.xyz
  CSELECT.allmsb   bool hp temp(4254).hp.x, bvec3 hp temp(4253).hp.xyz, uint 1, uint 0
*/
static VIR_PatternMatchInst _cmpPatInst9[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { 1, 1, 4, 5 }, { _isSrc1IntOne, _isSrc2IntZero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 6, 1, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR},

};

static VIR_PatternReplaceInst _cmpRepInst9[] = {
    { VIR_OP_COMPARE, -1, 0, { 1, 2, 3, 0 }, { 0, 0, 0, 0 } },
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, { 6, 1, 4, 5 }, { 0, 0, 0, 0 } },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { 1, 1, 4, 5 }, { 0, 0, 0, 0 } },
};

static VIR_PatternMatchInst _cmpPatInst10[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { all_source_float, VIR_Lower_IsDstBool }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_CSELECT, VIR_COP_SELMSB, 0, { 1, 1, 4, 5 }, { _isSrc1IntOne, _isSrc2IntZero }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 6, 1, 7, 8 }, { VIR_Lower_IsDstFloat, _isSrc1FloatOne, _isSrc2FloatZero}, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _cmpRepInst10[] = {
    { VIR_OP_COMPARE, VIR_PATTERN_ANYCOND, 0, { 6, 2, 3, 0 }, { 0, 0, 0, 0 } },
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
    { VIR_PATN_FLAG_RECURSIVE_SCAN_NEWINST, CODEPATTERN(_cmp, 8) },
    { VIR_PATN_FLAG_RECURSIVE_SCAN_NEWINST, CODEPATTERN(_cmp, 9) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 10) },
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
    return VIR_TypeId_isFloat(VIR_Operand_GetTypeId(coord));
}

static gctBOOL
_isCoordSignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * coord = VIR_Inst_GetSource(Inst, 1);
    return VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(coord));
}

static gctBOOL
_isCoordUnSignedInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * coord = VIR_Inst_GetSource(Inst, 1);
    return VIR_TypeId_isUnSignedInteger(VIR_Operand_GetTypeId(coord));
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
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

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
    /* WEBGL spec is inconsistent with OES spec when fetch a texel outside of the texture's size.
     * OES would return undefined result but webgl hopes(0,0,0,0), use ADDRESSING_BORDER for webGL by default
     * adopt the WebGL behavior as the default GLES driver behavior instead of a WebGL patch.
     * The WebGL Spec requirement:
     * Texel fetches that have undefined results in the OpenGL ES 3.0 API must return zero,
     * or a texture source color of (0, 0, 0, 1) in the case of a texel fetch from an incomplete texture in the WebGL 2.0 API.
     * also conforms to GLES Spec which allows the result as "undefined" (any value is fine).
     */
    gctUINT addressingType = 0x1;

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)));

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
    /* WEBGL spec is inconsistent with OES spec when fetch a texel outside of the texture's size.
     * OES would return undefined result but webgl hopes(0,0,0,0), use ADDRESSING_BORDER for webGL by default
     * adopt the WebGL behavior as the default GLES driver behavior instead of a WebGL patch.
     * The WebGL Spec requirement:
     * Texel fetches that have undefined results in the OpenGL ES 3.0 API must return zero,
     * or a texture source color of (0, 0, 0, 1) in the case of a texel fetch from an incomplete texture in the WebGL 2.0 API.
     * also conforms to GLES Spec which allows the result as "undefined" (any value is fine).
     */
    gctUINT addressingType = 0x1;
    gctUINT magFilter = 0x0; /* 0x0 */
    gctUINT minFilter = 0x0; /* 0x0 */

    imm.uValue = ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 1:0) - (0 ?
 1:0) + 1))))))) << (0 ?
 1:0))) | (((gctUINT32) ((gctUINT32) (dataType) & ((gctUINT32) ((((1 ?
 1:0) - (0 ?
 1:0) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 1:0) - (0 ? 1:0) + 1))))))) << (0 ? 1:0)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 13:12) - (0 ?
 13:12) + 1))))))) << (0 ?
 13:12))) | (((gctUINT32) ((gctUINT32) (mode) & ((gctUINT32) ((((1 ?
 13:12) - (0 ?
 13:12) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 13:12) - (0 ? 13:12) + 1))))))) << (0 ? 13:12)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 9:8) - (0 ?
 9:8) + 1))))))) << (0 ?
 9:8))) | (((gctUINT32) ((gctUINT32) (addressingType) & ((gctUINT32) ((((1 ?
 9:8) - (0 ?
 9:8) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 9:8) - (0 ? 9:8) + 1))))))) << (0 ? 9:8)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 4:4) - (0 ?
 4:4) + 1))))))) << (0 ?
 4:4))) | (((gctUINT32) ((gctUINT32) (magFilter) & ((gctUINT32) ((((1 ?
 4:4) - (0 ?
 4:4) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 4:4) - (0 ? 4:4) + 1))))))) << (0 ? 4:4)))
               | ((((gctUINT32) (0)) & ~(((gctUINT32) (((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ?
 5:5) - (0 ?
 5:5) + 1))))))) << (0 ?
 5:5))) | (((gctUINT32) ((gctUINT32) (minFilter) & ((gctUINT32) ((((1 ?
 5:5) - (0 ?
 5:5) + 1) == 32) ?
 ~0U : (~(~0U << ((1 ? 5:5) - (0 ? 5:5) + 1))))))) << (0 ? 5:5)));

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

static VIR_PatternMatchInst _texlduPatInst3[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 0 }, { gcvNULL, _isCoordSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst3[] = {
    { VIR_OP_TEXLD_U_S_L, 0, 0, { 1, 2, 3, 0, 0 }, { 0, 0, 0, VIR_Lower_SetZero, _genIntegeroordDataLod } },
};

/* Generate TEXLD_U_U_L. */
static VIR_PatternMatchInst _texlduPatInst4[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 4 }, { _isLodTexModifier, _isCoordUnSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst4[] = {
    { VIR_OP_TEXLD_U_U_L, 0, 0, { 1, 2, 3, 4, 0 }, { 0, 0, 0, 0, _genIntegeroordDataLod } },
};

static VIR_PatternMatchInst _texlduPatInst5[] = {
    { VIR_OP_TEXLD_U, -1, 0, { 1, 2, 3, 0 }, { gcvNULL, _isCoordUnSignedInt }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _texlduRepInst5[] = {
    { VIR_OP_TEXLD_U_U_L, 0, 0, { 1, 2, 3, 0, 0 }, { 0, 0, 0, VIR_Lower_SetZero, _genIntegeroordDataLod } },
};


VIR_Pattern _texlduPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_texldu, 5) },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0 } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0 } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -2, -3, 0 }, { 0 } },
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
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcf, 0) },
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcf, 1) },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
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
    { VIR_OP_SET, VIR_COP_LESS_OR_EQUAL, 0, {  1, -1, -2, 0 }, { 0, VIR_Lower_SetSwizzleZEx } },
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
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcfproj, 0) },
    { VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE | VIR_PATN_FLAG_SET_TEMP_IN_FUNC | VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_texldpcfproj, 1) },
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
_setRowOrder0PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
        case 8:
        case 16:
            imm0.iValue = 0x000F;
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
        case 8:
            imm0.iValue = 0x00FF;
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
_setRowOrder0PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 2:
            imm0.iValue = 0x0040;
            break;

        case 3:
            imm0.iValue = 0x0840;
            break;

        case 4:
        case 8:
        case 16:
            imm0.iValue = 0xC840;
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
            imm0.iValue = 0x0020;
            break;

        case 3:
            imm0.iValue = 0x0420;
            break;

        case 4:
        case 8:
            imm0.iValue = 0x6420;
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
_setRowOrder1PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
            break;

        case 8:
        case 16:
            imm0.iValue = 0x00F0;
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
        case 4:
            break;

        case 8:
            imm0.uValue = 0xFF00;
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
_setRowOrder1PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.uValue = 0;
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 8:
        case 16:
            imm0.uValue = 0xC8400000;
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
        case 4:
            break;

        case 8:
            imm0.uValue = 0x64200000;
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);
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
_setColumn2PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components =VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 3:
        case 4:
            imm0.iValue = 0x0004;
            break;

        case 8:
            imm0.iValue = 0x0044;
            break;

        case 16:
            imm0.iValue = 0x4444;
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
_setColumn2PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);
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
        case 3:
        case 4:
            imm0.iValue = 0x0000;
            break;

        case 8:
            imm0.iValue = 0x04000000;
            break;

        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x04000000;
            vConst.value.vecVal.u32Value[1] = 0x0C000800;
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
_setColumn3PackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    gcmASSERT(VIR_TypeId_isPacked(tyId));

    imm0.iValue = 0;
    components =VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, dest));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 4:
            imm0.iValue = 0x0008;
            break;

        case 8:
            imm0.iValue = 0x0088;
            break;

        case 16:
            imm0.iValue = 0x8888;
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
_setColumn3PackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);
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
        case 4:
            imm0.iValue = 0x0000;
            break;

        case 8:
            imm0.iValue = 0x40000000;
            break;

        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x40000000;
            vConst.value.vecVal.u32Value[1] = 0xC0008000;
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

    tyId = VIR_Operand_GetTypeId(dest);
    imm0.iValue = 0;
    if(VIR_TypeId_isPacked(tyId)) {
        components = VIR_GetTypePackedComponents(tyId);
    }
    else components = VIR_GetTypeComponents(tyId);
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(Opnd)));
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

    tyId = VIR_Operand_GetTypeId(dest);
    imm0.uValue = 0;
    if(VIR_TypeId_isPacked(tyId)) {
        components = VIR_GetTypePackedComponents(tyId);
    }
    else components = VIR_GetTypeComponents(tyId);
    gcmASSERT(VIR_TypeId_isPacked(VIR_Operand_GetTypeId(Opnd)));
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
_setRowOrder0UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

    imm0.iValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
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
        case 8:
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
            imm0.iValue = 0x0033;
            break;
        case 3:
            imm0.iValue = 0x0333;
            break;
        case 4:
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
_setRowOrder1UnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

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
            break;

        case 8:
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
        case 4:
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
_setRowOrder0UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;
    imm0.uValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
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
        case 8:
        case 16:
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
        case 8:
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
_setRowOrder1UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

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
            break;

        case 8:
        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00050004;
            vConst.value.vecVal.u32Value[1] = 0x00070006;
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
        case 4:
            break;

        case 8:
            imm0.uValue = 0x07060504;
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
_setColumnUnPackedMaskValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;
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
            vConst.value.vecVal.u32Value[1] = 0x000D0009;
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

static gctBOOL
_setColumn2UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

    imm0.uValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 3:
        case 4:
            imm0.uValue = 0x00000002;
            break;

        case 8:
            imm0.uValue = 0x00060002;
            break;

        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00060002;
            vConst.value.vecVal.u32Value[1] = 0x000E000A;
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
_setColumn3UnPackedSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
    VIR_ScalarConstVal imm0;
    gctUINT32  components;
    VIR_PrimitiveTypeId format;
    VIR_Const          vConst;
    gctBOOL            useImm = gcvTRUE;
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;

    imm0.uValue = 0;
    gcmASSERT(VIR_TypeId_isPacked(tyId));
    components = VIR_GetTypePackedComponents(tyId);
    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
    switch(format) {
    case VIR_TYPE_INT8:
    case VIR_TYPE_UINT8:
        switch(components) {
        case 4:
            imm0.uValue = 0x00000003;
            break;

        case 8:
            imm0.uValue = 0x00070003;
            break;

        case 16:
            useImm = gcvFALSE;
            vConst.value.vecVal.u32Value[0] = 0x00070003;
            vConst.value.vecVal.u32Value[1] = 0x000F000B;
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);
    VIR_TypeId srcTyId = VIR_Operand_GetTypeId(Opnd);
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

    if(components == 6) components = 8;
    else if(components == 12) components = 16;
    if(changeDest) {
        dest = VIR_Inst_GetDest(Inst);
        gcmASSERT(dest);
        gcmASSERT(components);
        VIR_Operand_SetTypeId(dest, VIR_TypeId_ComposePackedNonOpaqueType(srcFormat, components));
    }
    if(changeSrc) {
        gcmASSERT(components);
        VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposePackedNonOpaqueType(destFormat, components));
    }
    VIR_Operand_SetSwizzle(Opnd, VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(Opnd)));
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
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT_X2);
    }
    return gcvTRUE;
}

static gctBOOL
_destChar_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    return tyId == VIR_TYPE_INT8_P8 ||
           tyId == VIR_TYPE_UINT8_P8;
}

static gctBOOL
_destChar_P16Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    return tyId == VIR_TYPE_INT8_P16 ||
           tyId == VIR_TYPE_UINT8_P16;
}

static gctBOOL
_destShort_P8Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);


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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    return VIR_TypeId_isPacked(tyId);
}

static gctBOOL
_destPackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    if(_isLongUlong(Context, dest))
    {
       gctUINT componentCount;

       componentCount = VIR_GetTypeComponents(tyId);
       return componentCount > 4;
    }

    return VIR_GetTypeSize(tyId) > 16;
}

static gctBOOL
_destPackedGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) > 16);
}

static gctBOOL
_destPackedLE4Components(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    return VIR_TypeId_isPacked(tyId) && VIR_GetTypePackedComponents(tyId) <= 4;
}

static gctBOOL
_isOperandScalar(
    IN VIR_PatternContext *Context,
    IN VIR_Operand *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);
    VIR_Type *type;

    type = VIR_Shader_GetTypeFromId(Context->shader, tyId);
    if(VIR_Type_isScalar(type)) return gcvTRUE;
    if(VIR_Operand_isLvalue(Opnd)) return gcvFALSE;
    if(!VIR_TypeId_isPacked(tyId)) {
        return VIR_Operand_GetSwizzle(Opnd) == VIR_SWIZZLE_XXXX;
    }
    return gcvFALSE;
}

static gctBOOL
_destScalarOrPackedLE16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(dest);

    if(!(VIR_TypeId_isPacked(tyId) && VIR_GetTypeSize(tyId) <= 16)) {
        return _isOperandScalar(Context,
                                dest);
    }
    return gcvTRUE;
}

static gctBOOL
_src0PackedGT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    return VIR_TypeId_isPacked(tyId) && (VIR_GetTypeSize(tyId) > 16);
}

static gctBOOL
_src0GT16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    return VIR_GetTypeSize(tyId) > 16;
}

static gctBOOL
_src0ScalarOrPackedLE16Bytes(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    if(!(VIR_TypeId_isPacked(tyId) && VIR_GetTypeSize(tyId) <= 16)) {
        return _isOperandScalar(Context,
                                src);
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);

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

    case VIR_TYPE_FLOAT16_P2:
        tyId = VIR_TYPE_FLOAT32;
        break;
    case VIR_TYPE_FLOAT16_P4:
        tyId = VIR_TYPE_FLOAT_X2;
        break;
    case VIR_TYPE_FLOAT16_P8:
        tyId = VIR_TYPE_FLOAT_X4;
        break;
    case VIR_TYPE_FLOAT16_P16:
        tyId = VIR_TYPE_FLOAT_X8;
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;

    }

    VIR_Operand_SetTypeId(Opnd, tyId);
    VIR_Operand_SetTypeId(VIR_Inst_GetDest(Inst), tyId);
    /* set load/store enable too */
    _changeEnableByTyId(tyId, VIR_Inst_GetDest(Inst));

    return gcvTRUE;
}

static gctBOOL
_setNonpackedTypeByPatternDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT components;
    VIR_Instruction *patternInst = VIR_Inst_GetNext(Inst);
    VIR_Operand *patternDest = VIR_Inst_GetDest(patternInst);
    VIR_TypeId typeId;
    VIR_PrimitiveTypeId format;

    typeId = VIR_Lower_GetBaseType(Context->shader, patternDest);
    gcmASSERT(VIR_TypeId_isPacked(typeId));
    format = VIR_GetTypeComponentType(typeId);
    components = VIR_GetTypePackedComponents(typeId);
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(format, components, 1));

    if(VIR_Operand_isLvalue(Opnd)) {
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetTypeId(Opnd));
        VIR_Operand_SetEnable(Opnd, VIR_TypeId_Conv2Enable(VIR_Operand_GetTypeId(Opnd)));
    }
    else {
        VIR_Operand_SetSwizzle(Opnd, VIR_TypeId_Conv2Swizzle(VIR_Operand_GetTypeId(Opnd)));
    }

    return gcvTRUE;
}


static gctBOOL
_revise2PackedTypeAndSwizzle(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);
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

    VIR_Operand_SetTypeId(Opnd, tyId);
    VIR_Operand_SetSwizzle(Opnd, VIR_TypeId_Conv2Swizzle(tyId));

    return gcvTRUE;
}

static gctUINT32
_getSwizzleOperandValue(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *SwizzleOpnd,
    IN gctINT             Upper,
    OUT gctBOOL           *OpndUpdated
)
{
    gctUINT32 swizzle = 0;
    VIR_OperandKind operandKind;

    gcmASSERT(OpndUpdated);
    *OpndUpdated = gcvFALSE;
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
                if(VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG) {
                    VIR_PrimitiveTypeId format;
                    VIR_Swizzle   orgSwizzle;

                    format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, SwizzleOpnd));

                    VIR_Operand_SetTypeId(SwizzleOpnd,
                                          VIR_TypeId_ComposeNonOpaqueType(format, 1, 1));

                    orgSwizzle = VIR_Operand_GetSwizzle(SwizzleOpnd);
                    if(Upper)
                    {
                        VIR_Operand_SetSwizzle(SwizzleOpnd,
                                               VIR_Swizzle_Extract_Single_Channel_Swizzle(orgSwizzle, 1));
                    }
                    else
                    {
                        VIR_Operand_SetSwizzle(SwizzleOpnd,
                                               VIR_Swizzle_Extract_Single_Channel_Swizzle(orgSwizzle, 0));
                    }

                    *OpndUpdated = gcvTRUE;
                    return 0;
                }

                gcmASSERT(VIR_Symbol_HasFlag(sym, VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED));
                uniform = VIR_Symbol_GetUniform(sym);
                gcmASSERT(uniform);
                if(VIR_Operand_GetRelAddrMode(SwizzleOpnd) == VIR_INDEXED_NONE) {
                    VIR_Type *symType;

                    symType = VIR_Symbol_GetType(sym);
                    if(VIR_Type_isArray(symType)) {
                        gctINT arrayIndex;

                        arrayIndex = VIR_Operand_GetConstIndexingImmed(SwizzleOpnd) +
                                     VIR_Operand_GetMatrixConstIndex(SwizzleOpnd);
                        constId = *(VIR_Uniform_GetInitializerPtr(uniform) + arrayIndex);
                    }
                    else {
                        constId = VIR_Uniform_GetInitializer(uniform);
                    }
                }
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
_copyFullDefFlag(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *MaskValueOperand
    )
{
    VIR_Instruction *swizzleInst = VIR_Inst_GetNext(Inst);
    if (VIR_Inst_GetFlags(swizzleInst) & VIR_INSTFLAG_FULL_DEF) {
        VIR_Inst_SetFlag(Inst, VIR_INSTFLAG_FULL_DEF);
    }
    return gcvTRUE;
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
    gctUINT fullEnable[33] = { 0x0,
                               0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF,
                               0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
                               0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF, 0x1FFFFF, 0x3FFFFF, 0x7FFFFF, 0xFFFFFF,
                               0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF, 0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF };

    typeId = VIR_Operand_GetTypeId(dest);
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

    gcmASSERT(components <= 32);
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
    gctBOOL opndUpdated = gcvFALSE;

    typeId = VIR_Operand_GetTypeId(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      0,
                                      &opndUpdated);
    if(opndUpdated) {
        VIR_Operand *srcOpnd;
        srcOpnd = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Operand_GetTypeId(srcOpnd);
        if(VIR_TypeId_isPacked(typeId) && (VIR_GetTypeSize(typeId) > 16)) {
            gcmASSERT(0);
            return gcvFALSE;
        }
        constVal.uValue = enable & 0xFFFF;
        VIR_Operand_SetImmediate(enableOpnd,
                                 VIR_TYPE_UINT32,
                                 constVal);
    }
    else {
        typeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(swizzleInst));
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
    }
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
    gctBOOL opndUpdated;

    typeId = VIR_Operand_GetTypeId(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      0,
                                      &opndUpdated);
    if(opndUpdated) {
        VIR_Operand *srcOpnd;
        srcOpnd = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Operand_GetTypeId(srcOpnd);
        if(VIR_TypeId_isPacked(typeId) && (VIR_GetTypeSize(typeId) > 16)) {
            gcmASSERT(0);
            return gcvFALSE;
        }
        constVal.uValue = enable & 0xFFFF;
        VIR_Operand_SetImmediate(enableOpnd,
                                 VIR_TYPE_UINT32,
                                 constVal);
    }
    else {
        typeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(swizzleInst));
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
    }
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
    gctBOOL opndUpdated;

    typeId = VIR_Operand_GetTypeId(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = (VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF0000) >> 16;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      1,
                                      &opndUpdated);
    if(opndUpdated) {
        VIR_Operand *srcOpnd;
        srcOpnd = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Operand_GetTypeId(srcOpnd);
        if(VIR_TypeId_isPacked(typeId) && (VIR_GetTypeSize(typeId) > 16)) {
            gcmASSERT(0);
            return gcvFALSE;
        }
        constVal.uValue = enable >> 16;
        VIR_Operand_SetImmediate(enableOpnd,
                                 VIR_TYPE_UINT32,
                                 constVal);
    }
    else {
        typeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(swizzleInst));
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
    }
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
    gctBOOL opndUpdated;

    typeId = VIR_Operand_GetTypeId(SwizzleOpnd);
    enableOpnd = VIR_Inst_GetSource(Inst, 2);
    enable = (VIR_Operand_GetImmediateUint(enableOpnd) & 0xFFFF0000) >> 16;

    swizzle = _getSwizzleOperandValue(Context,
                                      SwizzleOpnd,
                                      1,
                                      &opndUpdated);
    if(opndUpdated) {
        VIR_Operand *srcOpnd;
        srcOpnd = VIR_Inst_GetSource(Inst, 0);
        typeId = VIR_Operand_GetTypeId(srcOpnd);
        if(VIR_TypeId_isPacked(typeId) && (VIR_GetTypeSize(typeId) > 16)) {
            gcmASSERT(0);
            return gcvFALSE;
        }
        constVal.uValue = enable >>16;
        VIR_Operand_SetImmediate(enableOpnd,
                                 VIR_TYPE_UINT32,
                                 constVal);
    }
    else {
        typeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(swizzleInst));
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
    }
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);
    VIR_PrimitiveTypeId  format;
    gctUINT components, rows;

    if(_isOperandScalar(Context, Opnd)) return gcvTRUE;
    if(VIR_TypeId_isImage(tyId) || VIR_TypeId_isImageT(tyId))
    {
        tyId = VIR_TYPE_UINT_X4;
    }
    else {
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
                    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
                    break;

                default:
                    gcmASSERT(0);
                    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);
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
        case VIR_TYPE_INTEGER_X8:
            tyId = VIR_TYPE_INTEGER_X4;
            break;
        case VIR_TYPE_UINT_X8:
            tyId = VIR_TYPE_UINT_X4;
            break;
        case VIR_TYPE_INTEGER_X16:
            tyId = VIR_TYPE_INTEGER_X8;
            break;
        case VIR_TYPE_UINT_X16:
            tyId = VIR_TYPE_UINT_X8;
            break;
        case VIR_TYPE_FLOAT16_P16:
            tyId = VIR_TYPE_FLOAT16_P8;
            break;
        default:
            gcmASSERT(VIR_TypeId_isPrimitive(tyId));
            format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
            components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd));
            rows = VIR_GetTypeRows(VIR_Lower_GetBaseType(Context->shader, Opnd));
            if(rows > 1) {
                gcmASSERT(components < 8);
            }
            else components >>= 1;
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
    }

    VIR_Operand_SetTypeId(Opnd, tyId);

    return gcvTRUE;
}

gctBOOL
_split32BytePackedType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);
    VIR_PrimitiveTypeId  format;
    gctUINT components, rows;

    if(_isOperandScalar(Context, Opnd)) return gcvTRUE;
    if(VIR_TypeId_isImage(tyId) || VIR_TypeId_isImageT(tyId))
    {
        tyId = VIR_TYPE_UINT_X4;
    }
    else {
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
                    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
                    break;

                default:
                    gcmASSERT(0);
                    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);
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
        case VIR_TYPE_INTEGER_X8:
            tyId = VIR_TYPE_INTEGER_X4;
            break;
        case VIR_TYPE_UINT_X8:
            tyId = VIR_TYPE_UINT_X4;
            break;
        case VIR_TYPE_INTEGER_X16:
            tyId = VIR_TYPE_INTEGER_X8;
            break;
        case VIR_TYPE_UINT_X16:
            tyId = VIR_TYPE_UINT_X8;
            break;
        case VIR_TYPE_FLOAT16_P16:
            tyId = VIR_TYPE_FLOAT16_P8;
            break;
        default:
            if(VIR_TypeId_isPacked(tyId)) return gcvTRUE;
            gcmASSERT(VIR_TypeId_isPrimitive(tyId));
            gcmASSERT(VIR_TypeId_isPrimitive(tyId));
            format = VIR_GetTypeComponentType(VIR_Lower_GetBaseType(Context->shader, Opnd));
            components = VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd));
            rows = VIR_GetTypeRows(VIR_Lower_GetBaseType(Context->shader, Opnd));
            if(rows > 1) {
                gcmASSERT(components < 8);
            }
            else components >>= 1;
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
    }

    VIR_Operand_SetTypeId(Opnd, tyId);

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
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetTypeId(Dest));
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
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetTypeId(Dest));
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);

    if(_isOperandScalar(Context, Opnd)) return gcvTRUE;
    if(_split32BytePackedTypeUpper(Context,
                                   Inst,
                                   Opnd) &&
       VIR_GetTypeSize(tyId) > 16) {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        VIR_Symbol    *sym = VIR_Operand_GetSymbol(Opnd);
        VIR_VirRegId  regId;
        VIR_SymId     symId;
        gctUINT       indexOffset;
        gctUINT       rowOffset;

        gcmASSERT(VIR_GetTypeRows(tyId) > 1);
        rowOffset = VIR_GetTypeRows(tyId) >> 1;

        switch(VIR_Symbol_GetKind(sym)) {
        case VIR_SYM_CONST:
            break;

        case VIR_SYM_UNIFORM:
        case VIR_SYM_IMAGE:
        case VIR_SYM_IMAGE_T:
            indexOffset = VIR_Operand_GetConstIndexingImmed(Opnd) + rowOffset;
            VIR_Operand_SetRelIndexingImmed(Opnd, indexOffset);
            break;

        default:
            regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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
                                        VIR_Operand_GetTypeId(Opnd));
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Dest);

    if(_split32BytePackedTypeDestUpper(Context,
                                       Inst,
                                       Dest) &&
       VIR_GetTypeSize(tyId) > 16) {
        VSC_ErrCode   virErrCode = VSC_ERR_NONE;
        VIR_Symbol    *sym = VIR_Operand_GetSymbol(Dest);
        VIR_VirRegId  regId;
        VIR_SymId     symId;
        gctUINT       rowOffset;

        gcmASSERT(VIR_GetTypeRows(tyId) > 1);
        rowOffset = VIR_GetTypeRows(tyId) >> 1;

        gcmASSERT(VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG);
        regId = VIR_Symbol_GetOffsetTempIndex(sym, rowOffset);
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
                                    VIR_Operand_GetTypeId(Dest));
        VIR_Inst_SetInstType(Inst, VIR_Operand_GetTypeId(Dest));
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);

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

    VIR_Operand_SetTypeId(VIR_Inst_GetDest(Inst), tyId);
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
        VIR_TypeId    operandType = VIR_Operand_GetTypeId(dest);
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

    tyId = VIR_Operand_GetTypeId(Opnd);
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

    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(format, components, 1));
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
    tyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(prevInst, 0));

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

    VIR_Operand_SetTypeId(Opnd, tyId);
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
    tyId = VIR_Operand_GetTypeId(src0);
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

    VIR_Operand_SetTypeId(Opnd, tyId);
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
    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(prevDest));

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

static VIR_PatternMatchInst _loadPatInst13[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_Long_ulong_one_load_two_moves }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst13[] = {
    { VIR_OP_LOAD, 0, 0, { -1, 2, 3, 0 }, { 0, 0, _long_ulong_first_load_to_temp } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_first_load_mov } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_second_load_mov } },
};

static VIR_PatternMatchInst _loadPatInst14[] = {
    { VIR_OP_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_Long_ulong_two_load_four_moves }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _loadRepInst14[] = {
    { VIR_OP_LOAD, 0, 0, { -1, 2, 3, 0 }, { 0, 0, _long_ulong_first_load_to_temp } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_first_load_mov } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_second_load_mov } },
    { VIR_OP_ADD, 0, 0, { -2, 3, 0, 0 }, { int_value_type0_const_16 } },
    { VIR_OP_LOAD, 0, 0, { -1, 2, -2, 0 }, { 0, 0, _long_ulong_second_load_to_temp } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_third_load_mov } },
    { VIR_OP_MOV, 0, 0, { 1, -1, 0, 0 }, { 0, _long_ulong_fourth_load_mov } },
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
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 13) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_load, 14) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_src2Char_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src2);

    return tyId == VIR_TYPE_INT8_P3 || tyId == VIR_TYPE_UINT8_P3;
}

static gctBOOL
_src0Char_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    return tyId == VIR_TYPE_INT8_P3 || tyId == VIR_TYPE_UINT8_P3;
}

static gctBOOL
_src0Short_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src);

    return tyId == VIR_TYPE_INT16_P3 || tyId == VIR_TYPE_UINT16_P3;
}

static gctBOOL
_setSrcChar_P3Type(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(Opnd))) {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT8_P3);
    }
    else {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT8_P3);
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
    if(VIR_TypeId_isSignedInteger(VIR_Operand_GetTypeId(Opnd))) {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT16_P3);
    }
    else {
        VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT16_P3);
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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src2);

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
    VIR_TypeId tyId = VIR_Operand_GetTypeId(src2);

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

static VIR_PatternMatchInst _storePatInst4[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _isSrc1Zero, _isCL_Long_ulong_2_store }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _storeRepInst4[] = {
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_first_store } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_second_store } },
};

static VIR_PatternMatchInst _storePatInst5[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _isSrc1Zero, _isCL_Long_ulong_4_store }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _storeRepInst5[] = {
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_first_store } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_second_store } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_third_store } },
    { VIR_OP_STORE, 0, 0, {  1, 2, 3, 4 }, { 0, 0, 0, _long_ulong_fourth_store } },
};

static VIR_PatternMatchInst _storePatInst6[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _isCL_Long_ulong_2_store }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst6[] = {
    { VIR_OP_ADD, 0, 0, {  -1, 3, 3, 0 }, { 0, 0, _long_ulong_lower_offset } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -1, 4 }, { 0, 0, 0, _long_ulong_first_add_store } },
    { VIR_OP_ADD, 0, 0, {  -2, -1, 3, 0 }, { 0, 0, int_value_type0_const_4 } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -2, 4 }, { 0, 0, 0, _long_ulong_second_add_store } },
};

static VIR_PatternMatchInst _storePatInst7[] = {
    { VIR_OP_STORE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _isCL_Long_ulong_4_store }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _storeRepInst7[] = {
    { VIR_OP_ADD, 0, 0, {  -1, 3, 3, 0 }, { 0, 0, _long_ulong_lower_offset } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -1, 4 }, { 0, 0, 0, _long_ulong_first_add_store } },
    { VIR_OP_ADD, 0, 0, {  -2, -1, 3, 0 }, { 0, 0, int_value_type0_const_4 } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -2, 4 }, { 0, 0, 0, _long_ulong_second_add_store } },
    { VIR_OP_ADD, 0, 0, {  -1, 3, 3, 0 }, { 0, 0, _long_ulong_upper_offset } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -1, 4 }, { 0, 0, 0, _long_ulong_third_add_store } },
    { VIR_OP_ADD, 0, 0, {  -2, -1, 3, 0 }, { 0, 0, int_value_type0_const_4 } },
    { VIR_OP_STORE, 0, 0, {  1, 2, -2, 4 }, { 0, 0, 0, _long_ulong_fourth_add_store } },
};

static VIR_Pattern _storePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_store, 7) },
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
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, { -1, 2, 0, 0 }, { int_value_type0, _split32BytePackedType, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, { -2, 2, 0, 0 }, { int_value_type0, _split32BytePackedTypeAndNextReg, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _anyPatInst1[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Char_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _anyRepInst1[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { -1, 2, 0, 0 }, { 0, int_value_type0_src_const_0xFFFFFF } },
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcChar_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _anyPatInst2[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Short_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _anyRepInst2[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 2, 2, 2 }, { 0, _setDestShort_P4TypeFromSrc, _setDefaultPackedSwizzle, _setShort_P4MaskValue } },
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcShort_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _anyPatInst3[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _anyRepInst3[] = {
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, { 1, 2, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};


static VIR_PatternMatchInst _anyPatInst4[] = {
    { VIR_OP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _anyRepInst4[] = {
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, {  -1, 2, 0, 0 }, { VIR_Lower_SetEnableBaseOnSrc0, 0, VIR_Lower_SetIntHighBitOne, VIR_Lower_SetIntZero } },
    { VIR_OP_CSELECT, VIR_COP_ANYMSB, 0, {   1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
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
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, { -1, 2, 0, 0 }, { int_value_type0, _split32BytePackedType, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, { -2, 2, 0, 0 }, { int_value_type0, _split32BytePackedTypeAndNextReg, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
    { VIR_OP_AND_BITWISE, 0, 0, {  1, -1, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _allPatInst1[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Char_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _allRepInst1[] = {
    { VIR_OP_OR_BITWISE, 0, 0, { -1, 2, 0, 0 }, { 0, int_value_type0_src_const_0xFF000000 } },
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcChar_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst2[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader, _src0Short_P3Type }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _allRepInst2[] = {
    { VIR_OP_SWIZZLE, 0, 0, {  -1, 2, 2, 2 }, { 0, _setDestShort_P4TypeFromSrc, _setDefaultPackedSwizzle, _setShort_P4MaskValue } },
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, { 1, -1, 0, 0 }, { 0, _setSrcShort_P3Type, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst3[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { _isCLShader }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _allRepInst3[] = {
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, {   1, 2, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
};

static VIR_PatternMatchInst _allPatInst4[] = {
    { VIR_OP_ALL, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _allRepInst4[] = {
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, {  -1, 2, 0, 0 }, { VIR_Lower_SetEnableBaseOnSrc0, 0, VIR_Lower_SetIntHighBitOne, VIR_Lower_SetIntZero } },
    { VIR_OP_CSELECT, VIR_COP_ALLMSB, 0, {   1, -1, 0, 0 }, { 0, 0, VIR_Lower_SetIntOne, VIR_Lower_SetIntZero } },
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
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _destScalarOrPackedLE16Bytes, _src0PackedGT16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _swizzleRepInst0[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { 0, _split32BytePackedType, _lowerSwizzleLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { 0, _split32BytePackedTypeAndNextReg, _lowerSwizzleNextRegLowerEnable } },
};

static VIR_PatternMatchInst _swizzlePatInst1[] = {
    { VIR_OP_SWIZZLE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _destPackedGT16Bytes, _src0ScalarOrPackedLE16Bytes }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _swizzleRepInst1[] = {
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDest, 0, _lowerSwizzleLowerEnable } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _split32BytePackedTypeDestAndNextReg, 0, _upperSwizzleUpperEnable } },
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
    { VIR_OP_SWIZZLE, 0, 0, { 1, 2, 3, 4 }, { _copyFullDefFlag, 0, 0, _checkToSetFullDefFlag } },
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

static gctBOOL
_isEnableSingleComponent(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Enable enable    = VIR_Inst_GetEnable(Inst);

    return (enable == VIR_ENABLE_X ||
            enable == VIR_ENABLE_Y ||
            enable == VIR_ENABLE_Z ||
            enable == VIR_ENABLE_W);
}

gctBOOL
_SetSwizzleByPrev2Inst(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *prev0      = VIR_Inst_GetPrev(Inst);
    VIR_Instruction *prev1      = VIR_Inst_GetPrev(prev0);
    VIR_Enable       enable0    = VIR_Inst_GetEnable(prev0);
    VIR_Enable       enable1    = VIR_Inst_GetEnable(prev1);
    VIR_Swizzle      swizzle0   = VIR_NormalizeSwizzleByEnable(enable0,
                                     VIR_Operand_GetSwizzle(VIR_Inst_GetSource(prev0, 0)));
    VIR_Swizzle      swizzle1   = VIR_NormalizeSwizzleByEnable(enable1,
                                     VIR_Operand_GetSwizzle(VIR_Inst_GetSource(prev1, 0)));
    VIR_Swizzle      newSwizzle = (swizzle1 & 0x03)|((swizzle0 << 2) & 0xFF);
    VIR_Operand_SetSwizzle(Opnd, newSwizzle);
    return gcvTRUE;
}

gctBOOL
_SetSwizzleByPrevInst(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Instruction *prev0      = VIR_Inst_GetPrev(Inst);
    VIR_Enable       enable0    = VIR_Inst_GetEnable(prev0);
    VIR_Enable       enable     = VIR_Inst_GetEnable(Inst);
    VIR_Swizzle      swizzle0   = VIR_NormalizeSwizzleByEnable(enable0,
                                     VIR_Operand_GetSwizzle(VIR_Inst_GetSource(prev0, 0)));
    VIR_Swizzle      swizzle    = VIR_NormalizeSwizzleByEnable(enable,
                                     VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0)));
    VIR_Swizzle      newSwizzle = (swizzle0 & 0x03)|((swizzle << 2) & 0xFF);
    VIR_Operand_SetSwizzle(Opnd, newSwizzle);
    return gcvTRUE;
}

static gctBOOL
_sameSrc0AsPrevSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    VIR_Instruction *prev = VIR_Inst_GetPrev(Inst);
    VIR_Operand *src0, *prevSrc0;

    gcmASSERT(prev);
    src0 = VIR_Inst_GetSource(Inst, 0);
    prevSrc0 = VIR_Inst_GetSource(prev, 0);

    if (VIR_Operand_GetOpKind(src0) == VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(prevSrc0) == VIR_OPND_SYMBOL)
    {
        return (VIR_Operand_GetSymbol(src0) == VIR_Operand_GetSymbol(prevSrc0));
    }

    return gcvFALSE;
}

static gctBOOL
_AreSrc2_3_Const(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst)
{
    gctUINT numSrc;
    VIR_Operand *src2, *src3;

    numSrc = VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(Inst));
    if(numSrc < 4) return gcvFALSE;
    src2 = VIR_Inst_GetSource(Inst, 2);
    src3 = VIR_Inst_GetSource(Inst, 3);

    return (VIR_Operand_isImm(src2) || VIR_Operand_isConst(src2)) &&
           (VIR_Operand_isImm(src3) || VIR_Operand_isConst(src3));
}

static VIR_PatternMatchInst _mulPatInst0[] = {
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 2, 0 }, { _isEnableSingleComponent }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 4, 3, 3, 0 }, { _isEnableSingleComponent, _sameSrc0AsPrevSrc0 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 5, 1, 4, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _mulRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { 1, 2, 2, 0 }, { 0 } },
    { VIR_OP_MUL, 0, 0, { 4, 3, 3, 0 }, { 0 } },
    { VIR_OP_DP2, 0, 0, { 5, 2, 2, 0 }, { 0, _SetSwizzleByPrev2Inst, _SetSwizzleByPrev2Inst } },
};

static VIR_PatternMatchInst _mulPatInst1[] = {
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 2, 0 }, { _isEnableSingleComponent }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 4, 3, 3, 0 }, { _isEnableSingleComponent, _sameSrc0AsPrevSrc0 }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 5, 4, 1, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _mulRepInst1[] = {
    { VIR_OP_MUL, 0, 0, { 1, 2, 2, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { 4, 3, 3, 0 }, { 0 } },
    { VIR_OP_DP2, 0, 0, { 5, 2, 2, 0 }, { 0, _SetSwizzleByPrev2Inst, _SetSwizzleByPrev2Inst } },
};

static VIR_PatternMatchInst _mulPatInst2[] = {
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 2, 0 }, { _isEnableSingleComponent }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_MAD, VIR_PATTERN_ANYCOND, 0, { 4, 3, 3, 1 }, { _isEnableSingleComponent, _sameSrc0AsPrevSrc0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _mulRepInst2[] = {
    { VIR_OP_MUL, 0, 0, { 1, 2, 2, 0 }, { 0 } },
    { VIR_OP_DP2, 0, 0, { 5, 2, 2, 0 }, { 0, _SetSwizzleByPrevInst, _SetSwizzleByPrevInst } },
};

static VIR_Pattern _mulPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mul, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mul, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mul, 2) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_isSrc1IntegerImmAndSrc0NotSameAsDest(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand  *src  = VIR_Inst_GetSource(Inst, 1);
    VIR_Operand  *src0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand  *dest = VIR_Inst_GetDest(Inst);

    if(src == gcvNULL || !VIR_Operand_isImm(src))
        return gcvFALSE;

    if(!VIR_TypeId_isInteger(VIR_Operand_GetTypeId(src)))
        return gcvFALSE;
    if (VIR_Operand_isSymbol(src0) && VIR_Operand_isSymbol(dest) &&
        VIR_Operand_GetSymbol(src0) == VIR_Operand_GetSymbol(dest))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gctBOOL
_ChangeImmToPow2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   shift = VIR_Operand_GetImmediateUint(Opnd);
    VIR_ScalarConstVal imm0;

    gcmASSERT(VIR_Operand_isImm(Opnd));

    if (shift > 31)
    {
        shift = 31;;
    }

    imm0.uValue = 1 << shift;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    return gcvTRUE;
}

static VIR_PatternMatchInst _lshiftPatInst0[] = {
    { VIR_OP_LSHIFT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1IntegerImmAndSrc0NotSameAsDest }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 4, 1, 5, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _lshiftRepInst0[] = {
    { VIR_OP_LSHIFT, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { 4, 2, 3, 5 }, { 0, 0, _ChangeImmToPow2 } },
};

static VIR_PatternMatchInst _lshiftPatInst1[] = {
    { VIR_OP_LSHIFT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1IntegerImmAndSrc0NotSameAsDest }, VIR_PATN_MATCH_FLAG_AND },
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _lshiftRepInst1[] = {
    { VIR_OP_LSHIFT, 0, 0, { 1, 2, 3, 0 }, { 0 } },
    { VIR_OP_MAD, 0, 0, { 4, 2, 3, 5 }, { 0, 0, _ChangeImmToPow2 } },
};

static VIR_Pattern _lshiftPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_lshift, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_lshift, 1) },
    { VIR_PATN_FLAG_NONE }
};

/*
    AND_BITWISE 1, 2, 3
        { -2, 0x5D, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5D, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
*/
static VIR_PatternMatchInst _andbitwisePatInst0[] = {
    { VIR_OP_AND_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, {_hasInteger_long_ulong}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _andbitwiseRepInst0[] = {
    { VIR_OP_AND_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_first_logical_op } },
    { VIR_OP_AND_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_second_logical_op } },
};

static VIR_Pattern _andbitwisePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_andbitwise, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    OR_BITWISE 1, 2, 3
        { -2, 0x5C, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5C, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
*/
static VIR_PatternMatchInst _orbitwisePatInst0[] = {
    { VIR_OP_OR_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, {_hasInteger_long_ulong}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _orbitwiseRepInst0[] = {
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_first_logical_op } },
    { VIR_OP_OR_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_second_logical_op } },
};

static VIR_Pattern _orbitwisePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_orbitwise, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    XOR_BITWISE 1, 2, 3
        { -2, 0x5E, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5E, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
*/
static VIR_PatternMatchInst _xorbitwisePatInst0[] = {
    { VIR_OP_XOR_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, {_hasInteger_long_ulong}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _xorbitwiseRepInst0[] = {
    { VIR_OP_XOR_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_first_logical_op } },
    { VIR_OP_XOR_BITWISE, 0, 0, {  1, 2, 3, 0 }, { _long_ulong_second_logical_op } },
};

static VIR_Pattern _xorbitwisePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_xorbitwise, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    NOT_BITWISE 1, 2, 3
        { -2, 0x5F, 1, 2, 0, 3, 0, long_ulong_first_logical_op },
        { -1, 0x5F, 1, 2, 0, 3, 0, long_ulong_second_logical_op },
*/
static VIR_PatternMatchInst _notbitwisePatInst0[] = {
    { VIR_OP_NOT_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {_hasInteger_long_ulong}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _notbitwiseRepInst0[] = {
    { VIR_OP_NOT_BITWISE, 0, 0, {  1, 2, 0, 0 }, { _long_ulong_first_logical_not_op } },
    { VIR_OP_NOT_BITWISE, 0, 0, {  1, 2, 0, 0 }, { _long_ulong_second_logical_not_op } },
};

static VIR_Pattern _notbitwisePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_notbitwise, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    LONGLO 1, 2
        mov 1, 2
*/
static VIR_PatternMatchInst _longloPatInst0[] = {
    { VIR_OP_LONGLO, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {0}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _longloRepInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {0, _long_ulong_lower} },
};

static VIR_Pattern _longloPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_longlo, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    LONGHI 1, 2
        mov 1, 2
*/
static VIR_PatternMatchInst _longhiPatInst0[] = {
    { VIR_OP_LONGHI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {0}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _longhiRepInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0, _long_ulong_upper} },
};

static VIR_Pattern _longhiPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_longhi, 0) },
    { VIR_PATN_FLAG_NONE }
};

/*
    MOV_LONG 1, 2, 3
        mov 1, 2
        mov 1, 3
*/
static VIR_PatternMatchInst _movlongPatInst0[] = {
    { VIR_OP_MOV_LONG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, {0}, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _movlongRepInst0[] = {
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, {_long_ulong_set_lower} },
    { VIR_OP_MOV, VIR_PATTERN_ANYCOND, 0, { 1, 3, 0, 0 }, {_long_ulong_set_upper} },
};

static VIR_Pattern _movlongPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_movlong, 0) },
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
    { VIR_OP_SET, VIR_COP_UCARRY, 0, { 1, 2, 3, 0 }, { 0 } },
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
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _AreSrc2_3_Const }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitInsertRepInst0[] = {
    { VIR_OP_LSHIFT, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 5, 0, 0, 0 }, { 0, 0, eight } },
    { VIR_OP_OR_BITWISE, 0, VIR_PATTERN_TEMP_TYPE_X, { -2, -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_BITINSERT1, 0, 0, {  1, 2, 3, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _bitInsertPatInst1[] = {
    { VIR_OP_BITINSERT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _bitInsertRepInst1[] = {
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_X, { -1, 4, 0, 0 }, { 0 } },
    { VIR_OP_MOV, 0, VIR_PATTERN_TEMP_TYPE_Y, { -1, 5, 0, 0 }, { 0 } },
    { VIR_OP_BITINSERT2, 0, 0, {  1, 2, 3, -1, 0 }, { 0, 0, 0, VIR_Lower_SetSwizzleXY } },
};

VIR_Pattern _bitInsertPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitInsert, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitInsert, 1) },
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

/*
** Here we use the operand type to select which DP instruction we should use,
** so we need to make sure the all optimizations don't change the operand type.
*/
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

static gctBOOL
_isActOn2DImg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand         *opndSrc0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Symbol          *sym = VIR_Operand_GetSymbol(opndSrc0);
    VIR_PrimitiveTypeId primTypeId;

    if (VIR_Operand_GetOpKind(opndSrc0) == VIR_OPND_SYMBOL &&
        (VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE || VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE_T))
    {
        primTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(sym));

        if (VIR_TypeId_isImage2D(primTypeId))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_isActOn3DImg(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand         *opndSrc0 = VIR_Inst_GetSource(Inst, 0);
    VIR_Symbol          *sym = VIR_Operand_GetSymbol(opndSrc0);
    VIR_PrimitiveTypeId primTypeId;

    if (VIR_Operand_GetOpKind(opndSrc0) == VIR_OPND_SYMBOL &&
        (VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE || VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE_T))
    {
        primTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(sym));

        if (VIR_TypeId_isImage3D(primTypeId))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}


static VIR_PatternMatchInst _imgaddrPatInst0[] = {
    { VIR_OP_IMG_ADDR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3 }, { VIR_Lower_HasHalti4, _isActOn3DImg }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgaddrRepInst0[] = {
    { VIR_OP_IMG_ADDR_3D, -1, 0, { 1, 2, 3, 0 }, { 0 } }
};

static VIR_Pattern _imgaddrPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgaddr, 0) },
    { VIR_PATN_FLAG_NONE }
};


static
gctBOOL _evisFilterMax(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    return filter == VXC_FM_Max;
}

static
gctBOOL _evisFilterMin(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    return filter == VXC_FM_Min;
}

static
gctBOOL _evisFilterMedian(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    return filter == VXC_FM_Median;
}

static
gctBOOL _evisFilterSobelX(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    if (filter == VXC_FM_SobelX)
    {
        VIR_TypeId opnd0TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        VIR_TypeId opnd1TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));
        if (VIR_GetTypeComponentType(opnd0TyId) == VIR_TYPE_UINT8 &&
            VIR_GetTypeComponentType(opnd1TyId) == VIR_TYPE_UINT8)
        {
            return gcvTRUE;
        }
        gcmASSERT(gcvFALSE);
    }

    return gcvFALSE;
}

static
gctBOOL _evisFilterSobelY(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    if (filter == VXC_FM_SobelY)
    {
        VIR_TypeId opnd0TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        VIR_TypeId opnd1TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));
        if (VIR_GetTypeComponentType(opnd0TyId) == VIR_TYPE_UINT8 &&
            VIR_GetTypeComponentType(opnd1TyId) == VIR_TYPE_UINT8)
        {
            return gcvTRUE;
        }
        gcmASSERT(gcvFALSE);
    }

    return gcvFALSE;
}

static
gctBOOL _evisFilterBox(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    if (filter == VXC_FM_BOX)
    {
        VIR_TypeId opnd0TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        VIR_TypeId opnd1TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));
        if (VIR_GetTypeComponentType(opnd0TyId) == VIR_TYPE_UINT8 &&
            VIR_GetTypeComponentType(opnd1TyId) == VIR_TYPE_UINT8)
        {
            return gcvTRUE;
        }
        gcmASSERT(gcvFALSE);
    }

    return gcvFALSE;
}

static
gctBOOL _evisFilterGuassian(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    if (filter == VXC_FM_Guassian)
    {
        VIR_TypeId opnd0TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0));
        VIR_TypeId opnd1TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 1));
        if (VIR_GetTypeComponentType(opnd0TyId) == VIR_TYPE_UINT8 &&
            VIR_GetTypeComponentType(opnd1TyId) == VIR_TYPE_UINT8)
        {
            return gcvTRUE;
        }
        gcmASSERT(gcvFALSE);
    }

    return gcvFALSE;
}

static
gctBOOL _evisFilterScharrX(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    return (filter == VXC_FM_ScharrX);
}

static
gctBOOL _evisFilterScharrY(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst
    )
{
    /* get filter mode */
    VIR_Operand *vxcModifier = VIR_Inst_GetSource(Inst, 3);
    vxc_filter_mode filter;
    gcmASSERT(VIR_Operand_isEvisModifier(vxcModifier) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(vxcModifier)));
    filter = (vxc_filter_mode)((VIR_Operand_GetEvisModifier(vxcModifier) & VXC_FILTER_BITMASK) >> 16);
    return (filter == VXC_FM_ScharrY);
}

gctBOOL
_setScharrXUniform1To4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x00210210, 0x11321321,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x0000fffd, 0xfff60003, 0x000a0000, 0x00000000,
                                       0x0000fffd, 0xfff60003, 0x000a0000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setScharrXUniform5To8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x22432432, 0x33543543,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x0000fffd, 0xfff60003, 0x000a0000, 0x00000000,
                                       0x0000fffd, 0xfff60003, 0x000a0000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setScharrXUniform9To12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0xd5d5d5d5, 0x40404040, 0x13210210, 0x35432432,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x0000fffd, 0x00000003, 0x0000fffd, 0x00000003,
                                       0x0000fffd, 0x00000003, 0x0000fffd, 0x00000003};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setScharrYUniform1To4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x00210210, 0x11321321,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0xfff6fffd, 0x0000fffd, 0x00000000, 0x00000000,
                                       0xfff6fffd, 0x0000fffd, 0x00000000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setScharrYUniform5To8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = { 0x55555555, 0x05400540, 0x22432432, 0x33543543,
                                        0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                        0xfff6fffd, 0x0000fffd, 0x00000000, 0x00000000,
                                        0xfff6fffd, 0x0000fffd, 0x00000000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setScharrYUniform9To12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = { 0xd5d5d5d5, 0x40404040, 0x13210210, 0x35432432,
                                        0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                        0x000a0003, 0x00000003, 0x000a0003, 0x00000003,
                                        0x000a0003, 0x00000003, 0x000a0003, 0x00000003};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setBoxUniform1To4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = { 0x55555555, 0x05400540, 0x00210210, 0x11321321,
                                        0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                        0x00010001, 0x00010001, 0x00010001, 0x00000000,
                                        0x00010001, 0x00010001, 0x00010001, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setBoxUniform5To8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = { 0x55555555, 0x05400540, 0x22432432, 0x33543543,
                                        0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                        0x00010001, 0x00010001, 0x00010001, 0x00000000,
                                        0x00010001, 0x00010001, 0x00010001, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setBoxUniform9To12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = { 0xd5d5d5d5, 0x40404040, 0x13210210, 0x35432432,
                                        0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                        0x00010001, 0x00000001, 0x00010001, 0x00000001,
                                        0x00010001, 0x00000001, 0x00010001, 0x00000001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setBoxUniform13(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[4] = { 0x60402000, 0x00000000, 0x08080808, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X4;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
_setFloatCompType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId  opndTyId = VIR_Operand_GetTypeId(Opnd);
    VIR_Operand_SetTypeId(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32,
        VIR_GetTypeComponents(opndTyId),
        VIR_GetTypeRows(opndTyId)));

    return gcvTRUE;
}

static gctBOOL
_setFloatPointOne(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 0.111084f,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setStartBin0EndBin15_7  (
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId destTy = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    VIR_TypeId compTy = VIR_GetTypeComponentType(destTy);
    gcmASSERT(VIR_Operand_isEvisModifier(Opnd) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(Opnd)));
    gcmASSERT(VIR_TypeId_isInteger(compTy) && (VIR_GetTypeSize(compTy) == 2 || VIR_GetTypeSize(compTy) == 1));
    VIR_Operand_SetEvisModifier(Opnd, VXC_MODIFIER_BIN(0, VIR_GetTypeSize(compTy) == 1 ? 15 : 7, 0));
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_EVIS_MODIFIER);

    return gcvTRUE;
}

gctBOOL
_setStartBin0EndBin13_5(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId destTy = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
    VIR_TypeId compTy = VIR_GetTypeComponentType(destTy);
    gcmASSERT(VIR_Operand_isEvisModifier(Opnd) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(Opnd)));
    gcmASSERT(VIR_TypeId_isInteger(compTy) && (VIR_GetTypeSize(compTy) == 2 || VIR_GetTypeSize(compTy) == 1));
    VIR_Operand_SetEvisModifier(Opnd, VXC_MODIFIER_BIN(0, VIR_GetTypeSize(compTy) == 1 ? 13 : 5, 0));
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_EVIS_MODIFIER);
    return gcvTRUE;
}

/*
 filter.max
    Original code is  filter.max.u8 [0,13] r4, r1, r2,r3

    VX2 code is         vertMax3.u8 [0,15] r4, r1, r2, r3
                        horzMax3.u8[0, 13] r4, r4
 */
static VIR_PatternMatchInst _evisFilterPatInst0[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterMax }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst0[] = {
    { VIR_OP_VX_VERTMAX3, 0, 0, {  1, 2, 3, 4, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7 } },
    { VIR_OP_VX_HORZMAX3, 0, 0, {  1, 1, 5, 0, 0 }, {0, 0, _setStartBin0EndBin13_5 } },
};
/*
 filter.min
    Original code is  filter.mmi.u8 [0,13] r4, r1, r2,r3

    VX2 code is         vertMin3.u8 [0,15] r4, r1, r2, r3
                        horzMin3.u8[0, 13] r4, r4
 */

static VIR_PatternMatchInst _evisFilterPatInst1[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterMin }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst1[] = {
    { VIR_OP_VX_VERTMIN3, 0, 0, {  1, 2, 3, 4, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7 } },
    { VIR_OP_VX_HORZMIN3, 0, 0, {  1, 1, 5, 0, 0 }, {0, 0, _setStartBin0EndBin13_5 } },
};
/*
 filter.median
    Original code is  filter.median.u8 [0,13] r4, r1, r2,r3

    VX2 code is         vertMedian3.u8 [0,15] r4, r1, r2, r3
                        horzMedian3.u8[0, 13] r4, r4
                        vertMax3.u8[0,15]     t5, r1, r2, r3
                        vertMin3.u8 [0,15]    t6, r1, r2, r3
                        horzMax3.u8[0, 13]    t6, t6
                        horzMin3.u8[0, 13]    r5, t5
                        vertMedian3.u8 [0,13] r4, t5, r6, t4
 */
static VIR_PatternMatchInst _evisFilterPatInst2[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterMedian }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst2[] = {
    { VIR_OP_VX_VERTMED3, 0, 0, {  1, 2, 3, 4, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7   } },
    { VIR_OP_VX_HORZMED3, 0, 0, {  1, 1, 5, 0, 0 }, { 0, 0, _setStartBin0EndBin13_5 } },
    { VIR_OP_VX_VERTMAX3, 0, 0, {  -1, 2, 3, 4, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7   } },
    { VIR_OP_VX_VERTMIN3, 0, 0, {  -2, 2, 3, 4, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7   } },
    { VIR_OP_VX_HORZMAX3, 0, 0, {  -2, -2, 5, 0, 0 }, { 0, 0, _setStartBin0EndBin13_5 } },
    { VIR_OP_VX_HORZMIN3, 0, 0, {  -1, -1, 5, 0, 0 }, { 0, 0, _setStartBin0EndBin13_5 } },
    { VIR_OP_VX_VERTMED3, 0, 0, {  1, -1, -2, 1, 5 }, { 0, 0, 0, 0, _setStartBin0EndBin15_7   } },
};

gctBOOL
_setStartBinEndBinSrcBin(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startBin,
    IN gctUINT            endBin,
    IN gctUINT            srcBin
    )
{
    gctUINT     binMask;

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd) && VIR_TypeId_isInteger(VIR_Operand_GetTypeId(Opnd)));

    binMask = (((startBin) << 12) & VXC_START_BIN_BITMASK)              |
              (((endBin) << 8) & VXC_END_BIN_BITMASK)                   |
              (((srcBin) << 4) & VXC_SOURCE_BIN_BITMASK)                |
              (VIR_Operand_GetEvisModifier(Opnd) & VXC_CLAMP_BITMASK)  |
              (VIR_Operand_GetEvisModifier(Opnd) & VXC_ROUNDING_MODE_BITMASK);

    VIR_Operand_SetEvisModifier(Opnd, binMask);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_EVIS_MODIFIER);

    return gcvTRUE;
}

gctBOOL
_setStartBin0EndBin1SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, 1, 4);
}

gctBOOL
_setStartBin2EndBin3SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 2, 3, 4);
}

gctBOOL
_setStartBin4EndBin5SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 4, 5, 4);
}

gctBOOL
_setStartBin12EndBin13SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 12, 13, 4);
}

gctBOOL
_setStartBin6EndBin7SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 6, 7, 4);
}

gctBOOL
_setStartBin0EndBin3SrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, 3, 0);
}

gctBOOL
_setStartBin0EndBin3SrcBinN(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   srcBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_SOURCE_BIN_BITMASK) >> 4);

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, 3, srcBin);
}

gctBOOL
_setStartBin0EndBin3SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, 3, 4);
}

gctBOOL
_setStartBin8EndBin11SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 8, 11, 4);
}
gctBOOL
_setStartBin4EndBin7SrcBin4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 4, 7, 4);
}

gctBOOL
_setStartBin0EndBinN1SrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   endBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_END_BIN_BITMASK) >> 8);

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    gcmASSERT(endBin < 6);

    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, endBin + 1, 0);
}


gctBOOL
_setStartBin0EndBinNSrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   endBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_END_BIN_BITMASK) >> 8);

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    gcmASSERT(endBin < 6);

   return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, endBin, 0);
}

gctBOOL
_setBiUniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[4] = { 0x00000000, 0x00000008, 0x00000010, 0x00000018};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X4;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
_setUIntEight(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    imm0.uValue = 8;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setTypeAsDst(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));

    return gcvTRUE;
}

gctBOOL
_setTypeAsDstAndSwizzleY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));
    VIR_Operand_SetSwizzle(Opnd, VIR_SWIZZLE_YYYY);

    return gcvTRUE;
}

/* vx_bi_linear.es  7.srcbin0.u8 r3.{0, n}, r1, r2, r4.xy

    ==>
    vx_lerp.es 3F.srcbin0.u8    r5.{0, n+1}, r1, r2, r4.y
    bit_extract.pack.u32        r5, r5.x, c2, 8
    vx_lerp.es 3F.srcbin0.u8    r3.{0, n}, r5.x, r5.y, r4.x

    c2:    0x00000000 0x00000008 0x00000010 0x00000018
*/
static VIR_PatternMatchInst _biLinearPatInst0[] = {
    { VIR_OP_VX_BILINEAR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _biLinearRepInst0[] = {
    { VIR_OP_VX_LERP, 0, 0, { -1, 2, 3, 4, 5 }, { 0, 0, 0, VIR_Lower_SetSwizzleY, _setStartBin0EndBinN1SrcBin0} },
    { VIR_OP_BITEXTRACT, 0, 0, { -1, -1, 0, 0 }, { VIR_Lower_SetOpndUINT32, VIR_Lower_SetSwizzleX, _setBiUniform, _setUIntEight } },
    { VIR_OP_VX_LERP, 0, 0, { 1, -1, -1, 4, 5 }, { 0, _setTypeAsDst, _setTypeAsDstAndSwizzleY, VIR_Lower_SetSwizzleX, _setStartBin0EndBinNSrcBin0 } },
};

static VIR_Pattern _biLinearPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_biLinear, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_isSrc2Zero(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isSrcZero(Context, Inst, 2);
}

static gctBOOL
_isDst16bit(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_PrimitiveTypeId baseType = VIR_Lower_GetBaseType(Context->shader, VIR_Inst_GetDest(Inst));

    if (VIR_GetTypeComponentType(baseType) == VIR_TYPE_INT16 ||
        VIR_GetTypeComponentType(baseType) == VIR_TYPE_UINT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

gctBOOL
_setIAddUniform1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x44444444, 0x33221100, 0x77665544,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00003300,
                                       0x00010001, 0x00010001, 0x00010001, 0x00010001,
                                       0x00010001, 0x00010001, 0x00010001, 0x00010001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setIAddUniform2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x44444444, 0xbbaa9988, 0xffeeddcc,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00003300,
                                       0x00010001, 0x00010001, 0x00010001, 0x00010001,
                                       0x00010001, 0x00010001, 0x00010001, 0x00010001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setStartBin0EndBin7SrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 0, 7, 0);
}

    gctBOOL
_setStartBin8EndBin15SrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, 8, 15, 0);
}

static VIR_PatternMatchInst _iAddPatInst0[] = {
    { VIR_OP_VX_IADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _isSrc2Zero, _isDst16bit }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _iAddRepInst0[] = {
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -1, -1, -1, 5 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst } },
};


static VIR_PatternMatchInst _iAddPatInst1[] = {
    { VIR_OP_VX_IADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _isSrc2Zero }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _iAddRepInst1[] = {
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin8EndBin15SrcBin0, _setIAddUniform2 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -1, -1, -1, 5 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst } },
};

static VIR_PatternMatchInst _iAddPatInst2[] = {
    { VIR_OP_VX_IADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _isDst16bit }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _iAddRepInst2[] = {
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_DP2X8, 0, 0, { -2, 4, -1, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -2, -2, -2, 5 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst } },
};

static VIR_PatternMatchInst _iAddPatInst3[] = {
    { VIR_OP_VX_IADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _iAddRepInst3[] = {
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_DP2X8, 0, 0, { -2, 4, -1, 5, 0 }, { 0, 0, 0, _setStartBin0EndBin7SrcBin0, _setIAddUniform1 } },
    { VIR_OP_VX_DP2X8, 0, 0, { -1, 2, 3, 5, 0 }, { 0, 0, 0, _setStartBin8EndBin15SrcBin0, _setIAddUniform2 } },
    { VIR_OP_VX_DP2X8, 0, 0, { -2, 4, -1, 5, 0 }, { 0, 0, 0, _setStartBin8EndBin15SrcBin0, _setIAddUniform2 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -2, -2, -2, 5 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst } },
};

static VIR_Pattern _iAddPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_iAdd, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_iAdd, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_iAdd, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_iAdd, 3) },
    { VIR_PATN_FLAG_NONE }
};

gctBOOL
_setTypeAsFloatX4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X4);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloatX4EnableX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloatX4EnableY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Y);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloatX4EnableZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_Z);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloatX4EnableW(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X4);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_W);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloat(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_X);

    return gcvTRUE;
}

gctBOOL
_setTypeAsFloatX2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_FLOAT_X2);
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);

    return gcvTRUE;
}

gctBOOL
_setTypeAsUINT16X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT16_X4);

    return gcvTRUE;
}

gctBOOL
_setTypeAsINT16X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT16_X4);

    return gcvTRUE;
}

gctBOOL
_setTypeAsINT8X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_INT8_X4);

    return gcvTRUE;
}

gctBOOL
_setTypeAsUINT8X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT8_X4);

    return gcvTRUE;
}

gctBOOL
_setUIntEightThree(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 0x83,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setUIntThree(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.iValue = 3,

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setPhaseFloat1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;

    imm0.fValue = 6.2832f;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setPhaseFloat2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   rangePi = ((VIR_Operand_GetImmediateUint(Opnd) & VXC_RANGEPI_BITMASK) >> 20);

    VIR_ScalarConstVal imm0;

    if (rangePi)
    {
        imm0.fValue = 40.744f * 2;
    }
    else
    {
        imm0.fValue = 40.744f;
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_FLOAT32,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setPhaseConst1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_ScalarConstVal imm0;
    VIR_TypeId tyId = VIR_Operand_GetTypeId(Opnd);

    if (tyId == VIR_TYPE_INT16_P8 ||
        tyId == VIR_TYPE_UINT16_P8)
    {
        imm0.iValue = 0x10001;
    }
    else
    {
        imm0.iValue = 0x01010101;
    }

    VIR_Operand_SetImmediate(Opnd,
        tyId,
        imm0);
    return gcvTRUE;
}

gctBOOL
_setINT16Uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[4] = {0x60402000, 0x00000000, 0x10101010, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X4;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);

    return gcvTRUE;
}

gctBOOL
_setINT8Uniform(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[4] = {0x60402000, 0x00000000, 0x08080808, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X4;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static gctBOOL
_label_set_jmp_neg3_4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    if (!VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -3))
    {
        return gcvFALSE;
    }

    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -4);
}

static gctBOOL
_label_set_jmp_neg11(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    return VIR_Lower_label_set_jmp_n(Context, Inst, Opnd, -11);
}

gctBOOL
_setStartBinNEndBinNSrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   startBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_START_BIN_BITMASK) >> 12);
    gctUINT   endBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_END_BIN_BITMASK) >> 8);

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    /* similar as magphase cmodel */
    if (endBin - startBin > 3)
    {
        endBin = endBin - 4;
    }

    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, startBin, endBin, 0);
}

gctBOOL
_setStartBinN4EndBinN4SrcBin0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT   startBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_START_BIN_BITMASK) >> 12);
    gctUINT   endBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_END_BIN_BITMASK) >> 8);

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    if (endBin - startBin > 3)
    {
        endBin = endBin - 4;
    }

    return _setStartBinEndBinSrcBin(Context, Inst, Opnd, startBin+4, endBin+4, 0);
}

static VIR_PatternMatchInst _magPhasePatInst0[] = {
    { VIR_OP_VX_MAGPHASE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, }, { _isDst16bit }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _magPhaseRepInst0[] = {
    /* mag: put into startBin - endBin'
       if (endBin - startBin) > 3
            endBin' = endBin - 3;
       else
            endBin' = endBin;
    */
    { VIR_OP_VX_MULSHIFT, 0, 0, { -1, 2, 2, 0, 4 }, { _setTypeAsFloatX4, 0, 0, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_VX_MULSHIFT, 0, 0, { -2, 3, 3, 0, 4 }, { _setTypeAsFloatX4, 0, 0, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_ADD, 0, 0, { -3, -1, -2, 0 }, { _setTypeAsFloatX4 } },
    { VIR_OP_SQRT, 0, 0, { -3, -3, 0, 0 }, { _setTypeAsFloatX4 } },
    { VIR_OP_CONVERT, 0, 0, { -3, -3, 0, 0 }, { _setTypeAsINT16X4 } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -3, -3, 0, 0, 4 }, { _setTypeAsUINT16X4, 0, VIR_Lower_SetIntZero, _setINT16Uniform, _setStartBin0EndBin3SrcBin0 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -3, -3, -3, 4 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst, _setStartBinNEndBinNSrcBin0} },

    /* mag: put into startBin + 4 - endBin' + 4
    */
    { VIR_OP_VX_MULSHIFT, 0, 0, { -5, 2, 2, 0, 4 }, { _setTypeAsFloatX4, 0, _setPhaseConst1, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_VX_MULSHIFT, 0, 0, { -6, 3, 3, 0, 4 }, { _setTypeAsFloatX4, 0, _setPhaseConst1, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },

    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetZero } }, /* goto label1*/
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetZero } }, /* goto label1*/
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } }, /* goto label2 */
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } }, /* label1 */
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleX, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetSwizzleX, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleY, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleY, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetSwizzleY, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleZ, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleZ, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleZ, VIR_Lower_SetSwizzleZ, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetSwizzleZ, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleW, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleW, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleW, VIR_Lower_SetSwizzleW, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetSwizzleW, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_MUL, 0, 0, { -12, -11, 4, 0 }, { _setTypeAsFloatX4, VIR_Lower_SetSwizzleXYZW, _setPhaseFloat2 } },
    { VIR_OP_CONVERT, 0, 0, { -12, -12, 0, 0 }, { _setTypeAsUINT16X4, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -12, -12, 0, 0, 4 }, { _setTypeAsUINT16X4, 0, VIR_Lower_SetIntZero, _setINT16Uniform, _setStartBin0EndBin3SrcBin0 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -12, -12, -12, 4 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst, _setStartBinN4EndBinN4SrcBin0} },
};

static VIR_PatternMatchInst _magPhasePatInst1[] = {
    { VIR_OP_VX_MAGPHASE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _magPhaseRepInst1[] = {
    /* mag: put into startBin - endBin'
       if (endBin - startBin) > 3
            endBin' = endBin - 3;
       else
            endBin' = endBin;
    */
    { VIR_OP_VX_MULSHIFT, 0, 0, { -1, 2, 2, 0, 4 }, { _setTypeAsFloatX4, 0, 0, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_VX_MULSHIFT, 0, 0, { -2, 3, 3, 0, 4 }, { _setTypeAsFloatX4, 0, 0, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_ADD, 0, 0, { -3, -1, -2, 0 }, { _setTypeAsFloatX4 } },
    { VIR_OP_SQRT, 0, 0, { -3, -3, 0, 0 }, { _setTypeAsFloatX4 } },
    { VIR_OP_CONVERT, 0, 0, { -3, -3, 0, 0 }, { _setTypeAsINT8X4 } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -3, -3, 0, 0, 4 }, { _setTypeAsUINT8X4, 0, VIR_Lower_SetIntZero, _setINT8Uniform, _setStartBin0EndBin3SrcBin0 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -3, -3, -3, 4 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst, _setStartBinNEndBinNSrcBin0} },

    /* mag: put into startBin + 4 - endBin' + 4
    */
    { VIR_OP_VX_MULSHIFT, 0, 0, { -5, 2, 2, 0, 4 }, { _setTypeAsFloatX4, 0, _setPhaseConst1, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },
    { VIR_OP_VX_MULSHIFT, 0, 0, { -6, 3, 3, 0, 4 }, { _setTypeAsFloatX4, 0, _setPhaseConst1, VIR_Lower_SetIntZero, _setStartBin0EndBin3SrcBinN } },

    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetZero } }, /* goto label1*/
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleX, VIR_Lower_SetZero } }, /* goto label1*/
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } }, /* goto label2 */
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } }, /* label1 */
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleX, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableX, VIR_Lower_SetSwizzleX, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleY, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleY, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableY, VIR_Lower_SetSwizzleY, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleZ, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleZ, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleZ, VIR_Lower_SetSwizzleZ, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableZ, VIR_Lower_SetSwizzleZ, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -5, 0, 0 }, { 0, VIR_Lower_SetSwizzleW, VIR_Lower_SetZero } },
    { VIR_OP_JMPC, VIR_COP_NOT_EQUAL, 0, { 0, -6, 0, 0 }, { 0, VIR_Lower_SetSwizzleW, VIR_Lower_SetZero } },
    { VIR_OP_MOV, 0, 0, { -11, 0, 0, 0 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetZero, } },
    { VIR_OP_JMP, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { 0 } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg3_4 } },
    { VIR_OP_ARCTRIG, 0, 0, { -7, -5, -6, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleW, VIR_Lower_SetSwizzleW, _setUIntEightThree } },
    { VIR_OP_DP2, 0, 0, { -8, -7, -7, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_RSQ, 0, 0, { -9, -8, 0, 0 }, { _setTypeAsFloat, 0, 0 } },
    { VIR_OP_MUL, 0, 0, { -9, -7, -9, 0 }, { _setTypeAsFloatX2, 0, 0 } },
    { VIR_OP_ARCTRIG, 0, 0, { -10, -9, -9, 0 }, { _setTypeAsFloatX2, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, _setUIntThree } },
    { VIR_OP_MAD, 0, 0, { -11, -10, -9, -10 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetSwizzleX, VIR_Lower_SetSwizzleY, VIR_Lower_SetSwizzleY } },
    { VIR_OP_JMPC, VIR_COP_GREATER_OR_EQUAL, 0, { 0, -11, 0, 0 }, { 0, 0, VIR_Lower_SetZero } },
    { VIR_OP_ADD, 0, 0, { -11, -11, 0, 0 }, { _setTypeAsFloatX4EnableW, VIR_Lower_SetSwizzleW, _setPhaseFloat1, } },
    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { VIR_Lower_label_set_jmp_neg2 } },

    { VIR_OP_LABEL, VIR_PATTERN_ANYCOND, 0, { 0, 0, 0, 0 }, { _label_set_jmp_neg11 } },
    { VIR_OP_MUL, 0, 0, { -12, -11, 4, 0 }, { _setTypeAsFloatX4, VIR_Lower_SetSwizzleXYZW, _setPhaseFloat2 } },
    { VIR_OP_CONVERT, 0, 0, { -12, -12, 0, 0 }, { _setTypeAsUINT8X4, VIR_Lower_SetSwizzleXYZW } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -12, -12, 0, 0, 4 }, { _setTypeAsUINT8X4, 0, VIR_Lower_SetIntZero, _setINT8Uniform, _setStartBin0EndBin3SrcBin0 } },
    { VIR_OP_VX_CLAMP, 0, 0, { 1, -12, -12, -12, 4 }, { 0, _setTypeAsDst, _setTypeAsDst, _setTypeAsDst, _setStartBinN4EndBinN4SrcBin0} },
};

static VIR_Pattern _magPhasePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_magPhase, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_magPhase, 1) },
    { VIR_PATN_FLAG_NONE }
};

gctBOOL
_setBitRplConst1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    virConst.type = VIR_TYPE_UINT_X4;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);
    VIR_Operand_SetTypeId(Opnd, VIR_TYPE_UINT32);

    return gcvTRUE;
}

static VIR_PatternMatchInst _bitReplacePatInst0[] = {
    { VIR_OP_VX_BITREPLACE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _bitReplaceRepInst0[] = {
    { VIR_OP_MOV, 0, 0, { -1, 0, 0, 0, 0 }, { 0, _setBitRplConst1 } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -1, 3, 3, 4, 5 }, { 0, _setBitRplConst1, _setBitRplConst1 } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, { -2, 3, 3, 4, 5 }, { 0 } },
    { VIR_OP_NOT_BITWISE, 0, 0, { -3, -1, 0, 0 }, { 0 } },
    { VIR_OP_AND_BITWISE, 0, 0, { -4, 2, -3, 0 }, { 0 } },
    { VIR_OP_OR_BITWISE, 0, 0, { 1, -4, -2, 0 }, { 0 } },
};

static VIR_Pattern _bitReplacePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_bitReplace, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_setSwizzleCommon(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand     *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId      tyId = VIR_Operand_GetTypeId(dest);

    if (tyId == VIR_TYPE_INT8_P16 ||
        tyId == VIR_TYPE_UINT8_P16)
    {
        VIR_Const          vConst;
        VIR_Uniform*       pImmUniform;
        VIR_Symbol*        sym;
        VIR_Swizzle        swizzle = VIR_SWIZZLE_XYYY;

        vConst.type = VIR_TYPE_UINT_X2;
        vConst.index = VIR_INVALID_ID;

        vConst.value.vecVal.u32Value[0] = 0x76543210;
        vConst.value.vecVal.u32Value[1] = 0xFEDCBA98;

        VIR_Shader_AddInitializedUniform(Context->shader, &vConst, &pImmUniform, &swizzle);

        /* Set this uniform as operand and set correct swizzle */
        sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
        VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetSym(Opnd, sym);
        VIR_Operand_SetSwizzle(Opnd, swizzle);
    }
    else if (tyId == VIR_TYPE_INT16_P8 ||
             tyId == VIR_TYPE_UINT16_P8)
    {
        VIR_ScalarConstVal imm0;
        imm0.iValue = 0x76543210;
        VIR_Operand_SetImmediate(Opnd,
                             VIR_TYPE_INT32,
                             imm0);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    return gcvTRUE;
}

static gctBOOL
_setMaskValueCommon(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startIn
    )
{
    gctUINT   startBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_START_BIN_BITMASK) >> 12);
    gctUINT   endBin = ((VIR_Operand_GetEvisModifier(Opnd) & VXC_END_BIN_BITMASK) >> 8);
    VIR_ScalarConstVal imm0;
    gctUINT i;
    VIR_Operand     *dest = VIR_Inst_GetDest(Inst);
    VIR_TypeId      tyId = VIR_Operand_GetTypeId(dest);

    gctUINT startComp = startBin + startIn;
    gctUINT endComp = (startComp + 3 < endBin) ? (startComp + 3) : endBin;

    gcmASSERT(VIR_Operand_isEvisModifier(Opnd));

    imm0.iValue = 0;

    if (tyId == VIR_TYPE_INT8_P16 ||
        tyId == VIR_TYPE_UINT8_P16)
    {
        for (i = startComp; i <= endComp; i++)
        {
            imm0.iValue = imm0.iValue | (0x1 << (i));
        }
    }
    else if(tyId == VIR_TYPE_INT16_P8 ||
            tyId == VIR_TYPE_UINT16_P8)
    {
        for (i = startComp; i <= endComp; i++)
        {
            imm0.iValue = imm0.iValue | ((0x1 << (i*2)) | (0x1 << (i*2+1)));
        }
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_setMaskValueCommon0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setMaskValueCommon(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

static gctBOOL
_setMaskValueCommon4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setMaskValueCommon(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

static gctBOOL
_setMaskValueCommon8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setMaskValueCommon(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

static gctBOOL
_setMaskValueCommon12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setMaskValueCommon(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

static void
_changeByte2Byte3(
    IN gctUINT startBin,
    IN gctBOOL DP8X2,
    OUT gctUINT *byteSwzl1,
    OUT gctUINT *byteSwzl2
    )
{
    gctUINT i, curComp;

    /* change byte2 and byte3 */
    curComp = startBin;
    if (DP8X2)
    {
        /* DP8X2 byte2 and byte3 */
        for (i = 0; i < 8; i++)
        {
            if (curComp > 15)
            {
                curComp = 0;
            }

            *byteSwzl1 |= (curComp << (i * 4));
            *byteSwzl2 |= ((curComp+1) << (i * 4));

            curComp ++;

            if (i == 2 || i == 5 || i == 6)
            {
                curComp = startBin;
            }
        }
    }
    else
    {
        /* DP4X4 byte2 and byte3 */
        for (i = 0; i < 3; i++)
        {
            *byteSwzl1 |= (curComp << (i * 4));
            *byteSwzl2 |= ((curComp+2) << (i * 4));

            curComp ++;
        }

        *byteSwzl1 |= (0) << (3 * 4);
        *byteSwzl2 |= (2) << (3 * 4);

        curComp = startBin + 1;
        for (i = 4; i < 7; i++)
        {
            *byteSwzl1 |= (curComp << (i * 4));
            *byteSwzl2 |= ((curComp+2) << (i * 4));

            curComp ++;
        }

        *byteSwzl1 |= (1) << (7 * 4);
        *byteSwzl2 |= (3) << (7 * 4);
    }
}

/* functions for sobelX */
gctBOOL
_setSobelXUniform8X2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startIn
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x0000ffff, 0xfffe0001, 0x00020000, 0x00000000,
                                       0x0000ffff, 0xfffe0001, 0x00020000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    _changeByte2Byte3(startIn, gcvTRUE, &byteSwzl[2], &byteSwzl[3]);

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform4X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startIn
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0xd5d5d5d5, 0x40404040, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x0000ffff, 0x00000001, 0x0000ffff, 0x00000001,
                                       0x0000ffff, 0x00000001, 0x0000ffff, 0x00000001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    _changeByte2Byte3(startIn, gcvFALSE /* dp4X4*/, &byteSwzl[2], &byteSwzl[3]);

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 2);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform5(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 6);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform7(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 10);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform11(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform8X2(Context, Inst, Opnd, 14);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform4X4(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform6(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform4X4(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform9(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform4X4(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setSobelXUniform12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelXUniform4X4(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

gctBOOL
_setTypeAsSrc0(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd)
{
    VIR_Operand_SetTypeId(Opnd, VIR_Operand_GetTypeId(VIR_Inst_GetSource(Inst, 0)));

    return gcvTRUE;
}

/*
 filter.sobelX
    Original code is  filter.s16 [0,3] r4, r1, r2, r3

    VX2 code is         vx_dp8x2.s32 [0,1]  r5, r1, r2, c1-4
                        vx_dp8x2.s32 [2,3]  r5, r1, r2, c5-8
                        vx_dp4x4.s16 [0,3]  r4, r3, r5, c9-12
 */

static VIR_PatternMatchInst _evisFilterPatInst3[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterSobelX }, VIR_PATN_MATCH_FLAG_OR },
};

/* max output 14 */
static VIR_PatternReplaceInst _evisFilterRepInst3[] = {
    /* 4 output */
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelXUniform1  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelXUniform2 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setSobelXUniform3 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon0 } },

    /* 8 output */
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelXUniform4 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelXUniform5 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin4EndBin7SrcBin4, _setSobelXUniform6 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon4 } },

    /* 12 output */
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelXUniform7  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelXUniform8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setSobelXUniform9 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon8 } },

    /* 14 output */
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelXUniform10 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelXUniform11 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin4EndBin7SrcBin4, _setSobelXUniform12 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon12 } },

};

/* functions for sobelY */
gctBOOL
_setSobelYUniform8X2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startIn
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0xfffeffff, 0x0000ffff, 0x00000000, 0x00000000,
                                       0xfffeffff, 0x0000ffff, 0x00000000, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    _changeByte2Byte3(startIn, gcvTRUE, &byteSwzl[2], &byteSwzl[3]);

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform4X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startIn
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0xd5d5d5d5, 0x40404040, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x00020001, 0x00000001, 0x00020001, 0x00000001,
                                       0x00020001, 0x00000001, 0x00020001, 0x00000001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    _changeByte2Byte3(startIn, gcvFALSE /* dp4X4*/, &byteSwzl[2], &byteSwzl[3]);

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 2);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform5(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 6);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform7(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 10);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform11(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform8X2(Context, Inst, Opnd, 14);

    return gcvTRUE;
}
gctBOOL
_setSobelYUniform3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform4X4(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform6(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform4X4(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform9(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform4X4(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setSobelYUniform12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setSobelYUniform4X4(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

/*
 filter.sobelY
    Original code is  filter.s16 [0,5] r4, r1, r2, r3

    VX2 code is         vx_dp8x2.s32 [0,1]  r5, r1, r2, c1-4
                        vx_dp8x2.s32 [2,3]  r5, r1, r2, c5-8
                        vx_dp4x4.s16 [0,3]  r4, r3, r5, c9-12

 */

static VIR_PatternMatchInst _evisFilterPatInst4[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterSobelY }, VIR_PATN_MATCH_FLAG_OR },
};

/* max 14 outputs */
static VIR_PatternReplaceInst _evisFilterRepInst4[] = {
    /* 4 outputs */
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelYUniform1  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelYUniform2 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setSobelYUniform3 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon0 } },

    /* 8 outputs */
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelYUniform4 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelYUniform5 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin4EndBin7SrcBin4, _setSobelYUniform6 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon4 } },

    /* 12 outputs */
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelYUniform7  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelYUniform8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setSobelYUniform9 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon8 } },

    /* 14 outputs */
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setSobelYUniform10 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setSobelYUniform11 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin4EndBin7SrcBin4, _setSobelYUniform12 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon12 } },
};

/* filter Gaussian functions */
gctBOOL
_setGaussianUniform8X2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT           startBin
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0x55555555, 0x05400540, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005300,
                                       0x00020001, 0x00020001, 0x00020004, 0x00000000,
                                       0x00020001, 0x00020001, 0x00020004, 0x00000000};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    _changeByte2Byte3(startBin, gcvTRUE, &byteSwzl[2], &byteSwzl[3]);

    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform4X4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd,
    IN gctUINT            startBin
    )
{
    VIR_Const          virConst;
    gctUINT            byteSwzl[16] = {0xd5d5d5d5, 0x40404040, 0x0, 0x0,
                                       0xaaaaaaaa, 0x00000000, 0x00000000, 0x00005304,
                                       0x00020001, 0x00000001, 0x00020001, 0x00000001,
                                       0x00020001, 0x00000001, 0x00020001, 0x00000001};
    VIR_Uniform*       pImmUniform;
    VIR_Symbol*        sym;
    VIR_Swizzle        swizzle = VIR_SWIZZLE_XXXX;

    /* change byte2 and byte3 */
   _changeByte2Byte3(startBin, gcvFALSE /* DP4X4*/, &byteSwzl[2], &byteSwzl[3]);


    virConst.type = VIR_TYPE_UINT_X16;
    virConst.index = VIR_INVALID_ID;
    memcpy(virConst.value.vecVal.u32Value, byteSwzl, sizeof(byteSwzl));

    VIR_Shader_AddInitializedUniform(Context->shader, &virConst, &pImmUniform, &swizzle);

    /* Set this uniform as operand and set correct swizzle */
    sym = VIR_Shader_GetSymFromId(Context->shader, pImmUniform->sym);
    VIR_Operand_SetOpKind(Opnd, VIR_OPND_SYMBOL);
    VIR_Operand_SetSym(Opnd, sym);
    VIR_Operand_SetSwizzle(Opnd, swizzle);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform1(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 2);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform5(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 6);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform7(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform8(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 10);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform10(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform11(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    _setGaussianUniform8X2(Context, Inst, Opnd, 14);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform3(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setGaussianUniform4X4(Context, Inst, Opnd, 0);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform6(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setGaussianUniform4X4(Context, Inst, Opnd, 4);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform9(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setGaussianUniform4X4(Context, Inst, Opnd, 8);

    return gcvTRUE;
}

gctBOOL
_setGaussianUniform12(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
   _setGaussianUniform4X4(Context, Inst, Opnd, 12);

    return gcvTRUE;
}

/*
 filter.gaussian
    Original code is    filter.u8 [0,3] r4, r1, r2, r3

    for every 4 outputs
    VX2 code is         vx_dp8x2.s32 [0,1]  r5, r1, r2, c1-4
                        vx_dp8x2.s32 [2,3]  r5, r1, r2, c5-8
                        vx_dp4x4.u8 [0,3]   r4, r3, r5, c9-12
                        swizzle
 */

static VIR_PatternMatchInst _evisFilterPatInst5[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterGuassian }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst5[] = {
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setGaussianUniform1  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setGaussianUniform2 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setGaussianUniform3 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon0 } },

    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setGaussianUniform4 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setGaussianUniform5 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin4EndBin7SrcBin4, _setGaussianUniform6 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon4 } },

    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setGaussianUniform7  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setGaussianUniform8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin8EndBin11SrcBin4, _setGaussianUniform9 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon8 } },

    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setGaussianUniform10 } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -2, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setGaussianUniform11 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  -3, 4, -2, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin12EndBin13SrcBin4, _setGaussianUniform12 } },
    { VIR_OP_SWIZZLE, 0, 0, { 1, -3, 5, 5 }, { 0, 0, _setSwizzleCommon, _setMaskValueCommon12 } },
};

/*
 filter.box
    Original code is    filter.u8 [0,15] r4, r1, r2, r3

    VX2 code:

    vx_dp8x2.es 3F.srcbin4.s32  r4.{0, 1}, r1.x, r2, c1-4
    vx_dp8x2.es 3F.srcbin4.s32  r4.{2, 3}, r1.x, r2, c5-9
    vx_dp4x4.es 3F.srcbin4.s32  r5.{0, 3}, r3.x, r4, c9-12
    conv.pack.f32               r5, r5, 5
    mul.pack                    r5, r5, 0.111084
    conv.pack.u8                r5, r5, 0
    vx_bit_extract.es 3F.srcbin0.u8 r5.{0, 3}, r5.x, 0, c13

 */

static VIR_PatternMatchInst _evisFilterPatInst6[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterBox }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst6[] = {
    { VIR_OP_VX_DP8X2, 0, 0, {  1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setBoxUniform1To4  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setBoxUniform5To8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  1, 4, 1, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin3SrcBin4, _setBoxUniform9To12 } },
    { VIR_OP_I2F, 0, 0, { -1, 1, 0, 0 }, { _setFloatCompType, VIR_Lower_SetOpndINT32 } },
    { VIR_OP_MUL, 0, 0, { -2, -1, 0, 0, 0 }, { _setFloatCompType, _setFloatCompType, _setFloatPointOne } },
    { VIR_OP_F2I, 0, 0, { -3, -2, 0, 0 }, { 0, _setFloatCompType } },
    { VIR_OP_VX_BITEXTRACT, 0, 0, {  1, -3, 0, 0, 5 }, { 0, 0, VIR_Lower_SetIntZero, _setBoxUniform13, _setStartBin0EndBin3SrcBin0 } },
};

/*
 filter.ScharrX
    Original code is    filter.u8 [0,15] r4, r1, r2, r3

    VX2 code is         vx_dp8x2.s32 [0,1]  r5, r1, r2, c1-4
                        vx_dp8x2.s32 [2,3]  r5, r1, r2, c5-8
                        vx_dp4x4.u8 [0,3]   r4, r3, r5, c9-12

 */
static VIR_PatternMatchInst _evisFilterPatInst7[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterScharrX }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst7[] = {
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setScharrXUniform1To4  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setScharrXUniform5To8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  1, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setScharrXUniform9To12 } },
};

/*
 filter.ScharrY
    Original code is    filter.u8 [0,15] r4, r1, r2, r3

    VX2 code is         vx_dp8x2.s32 [0,1]  r5, r1, r2, c1-4
                        vx_dp8x2.s32 [2,3]  r5, r1, r2, c5-8
                        vx_dp4x4.u8 [0,3]   r4, r3, r5, c9-12

 */
static VIR_PatternMatchInst _evisFilterPatInst8[] = {
    { VIR_OP_VX_FILTER, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _evisFilterScharrY }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _evisFilterRepInst8[] = {
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin0EndBin1SrcBin4, _setScharrYUniform1To4  } },
    { VIR_OP_VX_DP8X2, 0, 0, {  -1, 2, 3, 5, 0 }, { VIR_Lower_SetOpndINT32, 0, 0, _setStartBin2EndBin3SrcBin4, _setScharrYUniform5To8 } },
    { VIR_OP_VX_DP4X4, 0, 0, {  1, 4, -1, 5, 0 }, { 0, 0, _setTypeAsSrc0, _setStartBin0EndBin3SrcBin4, _setScharrYUniform9To12 } },
};

static VIR_Pattern _evisFilterPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_evisFilter, 8) },
    { VIR_PATN_FLAG_NONE }
};

gctBOOL
_setImageDescTypeValue(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand* imageSrc = VIR_Inst_GetSource(Inst, 0);
    VIR_Symbol*  imageSym;
    gctUINT typeValue = 0xffffffff;
    VIR_ScalarConstVal imm0;

    gcmASSERT(VIR_Operand_isSymbol(imageSrc));
    imageSym = VIR_Operand_GetSymbol(imageSrc);
    gcmASSERT(imageSym && VIR_Symbol_isImage(imageSym));

    switch(VIR_Symbol_GetTypeId(imageSym))
    {
        case VIR_TYPE_IMAGE_1D_T:
            typeValue = 0;
            break;
        case VIR_TYPE_IMAGE_1D_BUFFER_T:
            typeValue = 1;
            break;
        case VIR_TYPE_IMAGE_1D_ARRAY_T:
            typeValue = 2;
            break;
        case VIR_TYPE_IMAGE_2D_T:
            typeValue = 3;
            break;
        case VIR_TYPE_IMAGE_2D_ARRAY_T:
            typeValue = 4;
            break;
        case VIR_TYPE_IMAGE_3D_T:
            typeValue = 5;
            break;
        default:
            gcmASSERT(0);
    }


    imm0.iValue = typeValue;

    VIR_Operand_SetImmediate(Opnd,
        VIR_TYPE_UINT32,
        imm0);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);
    return gcvTRUE;
}

/* IMG_WIDTH dest, image
 *     AND_BITWISE   dest, image.z, 0xFFFF
 */
static VIR_PatternMatchInst _imgWidthPatInst0[] = {
    { VIR_OP_IMG_WIDTH, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgWidthRepInst0[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm0xFFFF } }
};

static VIR_Pattern _imgWidthPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgWidth, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_HEIGHT dest, image
 *     RSHIFT   dest, image.z, 16
 */
static VIR_PatternMatchInst _imgHeightPatInst0[] = {
    { VIR_OP_IMG_HEIGHT, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgHeightRepInst0[] = {
    { VIR_OP_RSHIFT, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm16 } }
};

static VIR_Pattern _imgHeightPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgHeight, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_DEPTH dest, image
 *     AND_BITWISE   dest, image.s5, 0xFFFF
 */
static VIR_PatternMatchInst _imgDepthPatInst0[] = {
    { VIR_OP_IMG_DEPTH, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgDepthRepInst0[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleYIndex_1AndUintType, VIR_Lower_SetImm0xFFFF } }
};

static VIR_Pattern _imgDepthPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgDepth, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_DIM dest, image_2d
 *     AND_BITWISE   dest.x, image.z, 0xFFFF
 *     RSHIFT        dest.y, image.z, 16
 */
static VIR_PatternMatchInst _imgDimPatInst0[] = {
    { VIR_OP_IMG_DIM, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0, _isActOn2DImg }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgDimRepInst0[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableX, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm0xFFFF } },
    { VIR_OP_RSHIFT, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableY, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm16 } },
};

/* IMG_DIM dest, image_3d
 *     AND_BITWISE   dest.x, image.z, 0xFFFF
 *     RSHIFT        dest.y, image.z, 16
 *     AND_BITWISE   dest.z, image.s5, 0xFFFF
 *     MOV           dest.w, 0
 */static VIR_PatternMatchInst _imgDimPatInst1[] = {
    { VIR_OP_IMG_DIM, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0, _isActOn3DImg }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgDimRepInst1[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableX, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm0xFFFF } },
    { VIR_OP_RSHIFT, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableY, VIR_Lower_SetSwizzleZAndUintType, VIR_Lower_SetImm16 } },
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableZ, VIR_Lower_SetSwizzleYIndex_1AndUintType, VIR_Lower_SetImm0xFFFF } },
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, }, { VIR_Lower_SetEnableW, VIR_Lower_SetImm0 } },
};

static VIR_Pattern _imgDimPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgDim, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgDim, 1) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_FORMAT dest, image
 *     RSHIFT   dest, image.s6, 16
 */
static VIR_PatternMatchInst _imgFormatPatInst0[] = {
    { VIR_OP_IMG_FORMAT, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgFormatRepInst0[] = {
    { VIR_OP_RSHIFT, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleZIndex_1AndUintType, VIR_Lower_SetImm16 } }
};

static VIR_Pattern _imgFormatPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgFormat, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_ORDER dest, image
 *     AND_BITWISE   dest, image.s6, 0xFFFF
 */
static VIR_PatternMatchInst _imgOrderPatInst0[] = {
    { VIR_OP_IMG_ORDER, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgOrderRepInst0[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleZIndex_1AndUintType, VIR_Lower_SetImm0xFFFF } }
};

static VIR_Pattern _imgOrderPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgOrder, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_ARRAY_SIZE dest, image
 *     AND_BITWISE   dest, image.s5, 0xFFFF
 */
static VIR_PatternMatchInst _imgArraySizePatInst0[] = {
    { VIR_OP_IMG_ARRAY_SIZE, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgArraySizeRepInst0[] = {
    { VIR_OP_AND_BITWISE, -1, 0, { 1, 2, 0, }, { 0, VIR_Lower_SetSwizzleYIndex_1AndUintType, VIR_Lower_SetImm0xFFFF } },
};

static VIR_Pattern _imgArraySizePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgArraySize, 0) },
    { VIR_PATN_FLAG_NONE }
};

/* IMG_TYPE dest, image
 *     MOV   dest, imageType info
 */
static VIR_PatternMatchInst _imgTypePatInst0[] = {
    { VIR_OP_IMG_TYPE, VIR_PATTERN_ANYCOND, 0, { 1, 2 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgTypeRepInst0[] = {
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, }, { 0, _setImageDescTypeValue } }
};

static VIR_Pattern _imgTypePattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgType, 0) },
    { VIR_PATN_FLAG_NONE }
};

gctBOOL
_SetDestTypeInt(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand *dest = VIR_Inst_GetDest(Inst);
    VIR_PrimitiveTypeId  format;
    VIR_TypeId typeId;
    gctUINT components;

    typeId = VIR_Lower_GetBaseType(Context->shader, dest);
    format = VIR_GetTypeComponentType(typeId);

    format = VIR_TYPE_INT32;
    components = VIR_GetTypeComponents(typeId);
    typeId = VIR_TypeId_ComposeNonOpaqueType(format, components, 1);
    VIR_Operand_SetTypeId(dest, typeId);
    return gcvTRUE;
}

static VIR_PatternMatchInst _ctzPatInst0[] = {
    { VIR_OP_CTZ, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_IsLongOpcode }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _ctzRepInst0[] = {
    { VIR_OP_LONGLO, 0, 0, { -1, 2, 0, 0 }, { VIR_Lower_SetLongUlongDestTypeOnly } },
    { VIR_OP_CTZ, 0, 0, { -2,-1, 0, 0 }, { VIR_Lower_SetLongUlongDestTypeOnly } },
    { VIR_OP_LONGHI, 0, 0, { -3, 2, 0, 0 }, { VIR_Lower_SetLongUlongDestTypeOnly } },
    { VIR_OP_CTZ, 0, 0, { -4, -3, 0, 0 }, { VIR_Lower_SetLongUlongDestTypeOnly } },
    { VIR_OP_ADD, 0, 0, { -5, -2, -4, 0 }, { VIR_Lower_SetLongUlongDestTypeOnly } },
    { VIR_OP_CSELECT, VIR_COP_NOT_ZERO, 0, { 1, -1, -2, -5 }, { _SetDestTypeInt } },
    { VIR_OP_MOV, 0, 0, { 1, 0, 0, 0 }, { VIR_Lower_SetLongUlongSecondDest, VIR_Lower_SetIntZero } }
};

static VIR_Pattern _ctzPattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_ctz, 0) },
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

    /* handle source operand */
    for (i = 0; i < srcNum; i++)
    {
        VIR_Symbol *sym;

        origSrc = VIR_Inst_GetSource(Inst, i);
        sym = VIR_Operand_GetSymbol(origSrc);

        if(sym && (VIR_Symbol_isImage(sym) || VIR_Symbol_isImageT(sym)))
        {
            VIR_TypeId typeId;
            typeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst));
            VIR_Operand_SetTypeId(origSrc, typeId);
        }
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

    return gcvTRUE;
}

static VIR_Pattern*
_GetLowerPatternPhaseExpand(
    IN  VIR_PatternContext * Context,
    IN VIR_Instruction     * Inst
    )
{
    VIR_OpCode opCode = VIR_Inst_GetOpcode(Inst);
    gctBOOL hasDest = VIR_OPCODE_hasDest(opCode);

    if(VIR_Inst_isComponentwise(Inst) &&
       ((hasDest && _destPackedGT16Bytes(Context, Inst)) ||
        ((!hasDest || _destPackedType(Context, Inst)) && _src0GT16Bytes(Context, Inst)) ||
        ((!hasDest || _destGT16Bytes(Context, Inst)) && _src0PackedType(Context, Inst)) ||
        ((!hasDest || _destGT16Bytes(Context, Inst)) && _src0GT16Bytes(Context, Inst)) ||
        (_isCLShader(Context, Inst) && (VIR_Inst_GetOpcode(Inst) == VIR_OP_MOV) && _destGT16Bytes(Context, Inst)))) {
        _SplitPackedGT16ByteInst(Context, Inst);
        return _nullPattern;
    }
    else {
        switch(opCode)
        {
        case VIR_OP_CTZ:
            return _ctzPattern;
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
        case VIR_OP_CONVERT:
            return _convPattern;
        case VIR_OP_LOG2:
            return _logPattern;
        case VIR_OP_DIV:
            return _divPattern;
        case VIR_OP_MOD:
            return _modPattern;
        case VIR_OP_JMPC:
            return _jmpcPattern;
        case VIR_OP_MOV:
            return _movPattern;
        case VIR_OP_COMPARE:
            return _cmpPattern;
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
        case VIR_OP_MUL:
            return _mulPattern;
        case VIR_OP_LSHIFT:
            return _lshiftPattern;
        case VIR_OP_AND_BITWISE:
            return _andbitwisePattern;
        case VIR_OP_OR_BITWISE:
            return _orbitwisePattern;
        case VIR_OP_XOR_BITWISE:
            return _xorbitwisePattern;
        case VIR_OP_NOT_BITWISE:
            return _notbitwisePattern;
        case VIR_OP_LONGLO:
            return _longloPattern;
        case VIR_OP_LONGHI:
            return _longhiPattern;
        case VIR_OP_MOV_LONG:
            return _movlongPattern;
        case  VIR_OP_IMG_WIDTH:
            return _imgWidthPattern;
        case  VIR_OP_IMG_HEIGHT:
            return _imgHeightPattern;
        case  VIR_OP_IMG_DEPTH:
            return _imgDepthPattern;
        case  VIR_OP_IMG_DIM:
            return _imgDimPattern;
        case  VIR_OP_IMG_FORMAT:
            return _imgFormatPattern;
        case  VIR_OP_IMG_ORDER:
            return _imgOrderPattern;
        case  VIR_OP_IMG_ARRAY_SIZE:
            return _imgArraySizePattern;
        case  VIR_OP_IMG_TYPE:
            return _imgTypePattern;
        case VIR_OP_IMG_ADDR:
            return _imgaddrPattern;

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
        if (gcHWCaps.hwFeatureFlags.supportEVISVX2)
        {
            switch(VIR_Inst_GetOpcode(Inst))
            {
            case VIR_OP_VX_FILTER:
                return _evisFilterPattern;
            case VIR_OP_VX_BILINEAR:
                return _biLinearPattern;
            case VIR_OP_VX_IADD:
                return _iAddPattern;
            case VIR_OP_VX_MAGPHASE:
                return _magPhasePattern;
            case VIR_OP_VX_BITREPLACE:
                return _bitReplacePattern;
            case VIR_OP_VX_SELECTADD:
            case VIR_OP_VX_DP2X16:
                gcoOS_Print("Error: opcode %s is not supported in VX2", VIR_OPCODE_GetName(VIR_Inst_GetOpcode(Inst)));
                return gcvNULL;
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

/* functions used in VIR_Lower_ArraryIndexing_To_LDARR_STARR */
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
    VIR_TypeId       typeId = VIR_Operand_GetTypeId(Opnd);
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
        VIR_Shader_GetTypeFromId(Shader, typeId),
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

        dest = VIR_Inst_GetDest(stArrInst);
        VIR_Operand_Copy(dest, Opnd);
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
        dest = VIR_Inst_GetDest(stArrInst);
        VIR_Operand_Copy(dest, Opnd);
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

    /* for debug tessellation purpose only,
       manually set gl_TessLevelOuter/gl_TessLevelInner
    */
    if ((VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_OUTER ||
         VIR_Symbol_GetName(sym) == VIR_NAME_TESS_LEVEL_INNER) &&
         gcmOPT_TESSLEVEL()  != GC_DEFAULT_TESS_LEVEL)
    {
        VIR_Instruction *cmovInst = gcvNULL;

        virErrCode = VIR_Function_AddInstructionAfter(
            Func,
            VIR_OP_CMOV,
            typeId,
            Inst,
            gcvTRUE,
            &cmovInst);
        if (virErrCode != VSC_ERR_NONE) return virErrCode;

        /* set value */
        VIR_Operand_SetTempRegister(VIR_Inst_GetSource(cmovInst, 0), Func, symId, typeId);
        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(cmovInst, 0),
            VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd)));
        VIR_Operand_SetPrecision(VIR_Inst_GetSource(cmovInst, 0),
            VIR_Symbol_GetPrecision(VIR_Shader_GetSymFromId(Shader, symId)));

        VIR_Operand_SetImmediateFloat(VIR_Inst_GetSource(cmovInst, 1), (gctFLOAT) gcmOPT_TESSLEVEL());

        VIR_Operand_SetImmediateFloat(VIR_Inst_GetSource(cmovInst, 2), (gctFLOAT) gcmOPT_TESSLEVEL());

        VIR_Operand_SetTempRegister(VIR_Inst_GetDest(cmovInst), Func, symId, typeId);

        VIR_Operand_SetEnable(VIR_Inst_GetDest(cmovInst), VIR_Operand_GetEnable(Opnd));

        VIR_Inst_SetConditionOp(cmovInst, VIR_COP_GREATER);
    }

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

    gctUINT  RegCount = VIR_Symbol_GetVirIoRegCount(Shader, sym);
    VIR_VirRegId     regId      = VIR_Shader_NewVirRegId(Shader, RegCount);
    VIR_SymId        symId;

    VIR_SymId        relSymId   = VIR_Operand_GetRelIndexing(Opnd);
    VIR_Indexed      relIndexed = VIR_Operand_GetRelAddrMode(Opnd);

    VIR_Type        *symType    = VIR_Symbol_GetType(sym);
    VIR_TypeId       typeId     = VIR_Type_GetBaseTypeId(symType);
    VIR_TypeId       newVirRegTypeId;

    VIR_Swizzle     src0Swizzle = VIR_SWIZZLE_INVALID;

    if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
    {
        sym = VIR_Symbol_GetVregVariable(sym);
        if (sym)
        {
            symType = VIR_Shader_GetTypeFromId(Shader,
                                               VIR_Operand_GetTypeId(Opnd));
            typeId     = VIR_Type_GetBaseTypeId(symType);
        }
    }

    if ((sym &&
        (VIR_Symbol_GetIndex(sym) == VIR_Shader_GetBaseSamplerId(Shader))) ||
        VIR_Inst_GetOpcode(Inst) == VIR_OP_STORE)
    {
        typeId = VIR_Operand_GetTypeId(Opnd);
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
        src = VIR_Inst_GetSource(ldArrInst, 0);
        VIR_Operand_Copy(src, Opnd);
        VIR_Operand_SetSwizzle(src, src0Swizzle);

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
        /* we check all the const input vertex access, if the const
           index larger than geoMaxAccessVertices, we change
           geoMaxAccessVertices */
        else if (Shader->shaderKind == VIR_SHADER_GEOMETRY)
        {
            if (sym &&
                VIR_Symbol_isInput(sym) &&
                (idxNo + 1 > Shader->geoMaxAccessVertices))
            {
                Shader->geoMaxAccessVertices = idxNo + 1;
            }
        }
    }
    else
    {
        /* src[0] */
        src = VIR_Inst_GetSource(ldArrInst, 0);
        VIR_Operand_Copy(src, Opnd);
        VIR_Operand_SetSwizzle(src, src0Swizzle);
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

VSC_ErrCode
VIR_Lower_ArraryIndexing_To_LDARR_STARR(
    IN  VIR_Shader              *Shader,
    IN  VSC_HW_CONFIG           *HwCfg,
    INOUT gctBOOL               *Changed
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    gctBOOL bChanged = gcvFALSE;

    /* insert LDARR/STARR for indexed access and
       insert ATTR_LD/ATTR_ST for memory access (per-patch, per-vertex ...) */
    {
        /* insert STARR/LDARR for indexed operands */
        VIR_FuncIterator    func_iter;
        VIR_FunctionNode   *func_node;

        if (Shader->shaderKind == VIR_SHADER_GEOMETRY)
        {
            Shader->geoMaxAccessVertices = 0;
        }

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
                    if ((opcode == VIR_OP_RESTART0 || opcode == VIR_OP_RESTART_STREAM) &&
                        (Shader->shaderLayout.geo.geoOutPrimitive != VIR_GEO_POINTS))
                    {
                        VIR_Shader_SetFlag(Shader, VIR_SHFLAG_GS_HAS_RESTART_OP);
                        bChanged = gcvTRUE;
                    }
                }

                if ((opcode == VIR_OP_ATTR_LD ||
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
                        /* When loading data from a per-patch variable, the index is saved in src2.*/
                        if (VIR_Symbol_isPerPatch(sym))
                        {
                            src = VIR_Inst_GetSource(inst, 1);
                            VIR_Inst_SetSource(inst, 1, VIR_Inst_GetSource(inst, 2));
                            VIR_Inst_SetSource(inst, 2, src);
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

                    bChanged = gcvTRUE;
                }

                /* set flags for attr_ld */
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

                    if (Shader->shaderKind == VIR_SHADER_TESSELLATION_CONTROL)
                    {
                        VIR_Symbol  *baseSym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetSource(inst, 0));

                        if (baseSym && isSymArrayedPerVertex(baseSym) && VIR_Symbol_isOutput(baseSym))
                        {
                            Shader->shaderLayout.tcs.hasOutputVertexAccess = gcvTRUE;
                        }
                    }

                    if (Shader->shaderKind == VIR_SHADER_GEOMETRY)
                    {
                        src = VIR_Inst_GetSource(inst, 1);
                        if (VIR_Operand_GetOpKind(src) == VIR_OPND_IMMEDIATE)
                        {
                            if (VIR_Operand_GetImmediateInt(src) + 1 > Shader->geoMaxAccessVertices)
                            {
                                Shader->geoMaxAccessVertices = VIR_Operand_GetImmediateInt(src) + 1;
                            }
                        }
                    }

                    bChanged = gcvTRUE;
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
                        bChanged = gcvTRUE;
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
                    bChanged = gcvTRUE;
                } while (0);
            }
        }
    }

    if (Changed)
    {
        *Changed = bChanged;
    }

    return errCode;
}

