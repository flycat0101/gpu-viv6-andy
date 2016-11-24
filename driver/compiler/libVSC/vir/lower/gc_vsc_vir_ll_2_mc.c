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


#include "gc_vsc.h"
#include "vir/lower/gc_vsc_vir_ll_2_mc.h"
#include "vir/lower/gc_vsc_vir_ml_2_ll.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

typedef struct _VIR_PATTERN_MC_LOWER_CONTEXT
{
    VIR_PatternContext          header;
    VSC_HW_CONFIG*              hwCfg;
    VSC_MM*                     pMM;
    gctBOOL                     generateImmediate;
    gctBOOL                     hasNEW_TEXLD;
    gctBOOL                     isCL_X;
    gctBOOL                     hasCL;
    gctBOOL                     hasHalti4;
    gctBOOL                     hasSHEnhancements2;
} VIR_PatternMCLowerContext;

static gctBOOL
_CmpInstuction(
    IN VIR_PatternContext   *Context,
    IN VIR_PatternMatchInst *Inst0,
    IN VIR_Instruction      *Inst1
    )
{
    return Inst0->opcode == Inst1->_opcode;
}

static gctBOOL
_hasHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternMCLowerContext *)Context)->hasHalti4;
}

static gctBOOL
_hasNotHalti4(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return !_hasHalti4(Context, Inst);
}

/* because of hw cmov restriction (one instruction type for control
   comparison and implicit converstion), we have to change cmov to
   cmp/select pair
   TODO: only changing to cmp/select pair for dual16 shader */
static gctBOOL
_dual16Req(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * destOpnd = VIR_Inst_GetDest(Inst);
    VIR_Operand * src0Opnd = VIR_Inst_GetSource(Inst, 0);
    VIR_TypeId ty0 = VIR_Operand_GetType(destOpnd);
    VIR_TypeId ty1 = VIR_Operand_GetType(src0Opnd);

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
_setIntegerType_EnableXY(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XY);
    return gcvTRUE;
}

static gctBOOL
_setIntegerType_EnableXYZ(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetType(Opnd, VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32,
        VIR_GetTypeComponents(VIR_Lower_GetBaseType(Context->shader, Opnd)), 1));
    VIR_Operand_SetEnable(Opnd, VIR_ENABLE_XYZ);
    return gcvTRUE;
}

static gctBOOL
_isCL_X(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_PatternMCLowerContext *)Context)->isCL_X;
}

static gctBOOL
_isHWNotSupportJmpAny(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternMCLowerContext *)Context)->hwCfg->hwFeatureFlags.hasHalti5)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL
_isTypeEqualTo(
    IN VIR_Operand *Opnd,
    IN VIR_TyFlag   TyFlag
    )
{
    VIR_TypeId   ty   = VIR_Operand_GetType(Opnd);
    gcmASSERT(ty < VIR_TYPE_PRIMITIVETYPE_COUNT);

    return (VIR_GetTypeFlag(ty) & TyFlag);
}

static gctBOOL
_sameType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (VIR_GetTypeComponentType(VIR_Operand_GetType(VIR_Inst_GetDest(Inst)))
        == VIR_GetTypeComponentType(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0))))
        && VIR_Operand_GetRoundMode(VIR_Inst_GetSource(Inst, 0)) == VIR_ROUND_DEFAULT
        && VIR_Operand_GetModifier(VIR_Inst_GetSource(Inst, 0)) == VIR_MOD_NONE
        && VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst)) == VIR_MOD_NONE;
}

static gctBOOL
_notSameSizeType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_TypeId dstTyId = VIR_GetTypeComponentType(VIR_Operand_GetType(VIR_Inst_GetDest(Inst)));
    VIR_TypeId srcTyId = VIR_GetTypeComponentType(VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0)));

    return (VIR_GetTypeSize(dstTyId) != VIR_GetTypeSize(srcTyId))
        || (VIR_Operand_GetRoundMode(VIR_Inst_GetSource(Inst, 0)) != VIR_ROUND_DEFAULT)
        || (VIR_Operand_GetModifier(VIR_Inst_GetSource(Inst, 0)) != VIR_MOD_NONE)
        || (VIR_Operand_GetModifier(VIR_Inst_GetDest(Inst)) != VIR_MOD_NONE);
}


static gctBOOL
_isSrc1FloatType(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isTypeEqualTo(VIR_Inst_GetSource(Inst, 1), VIR_TYFLAG_ISFLOAT);
}

static gctBOOL
_isF2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isTypeEqualTo(VIR_Inst_GetDest(Inst), VIR_TYFLAG_ISINTEGER) &&
        _isTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISFLOAT) &&
        (VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst)) == VIR_ROUND_DEFAULT||
         VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst)) == VIR_ROUND_RTZ );
}

static gctBOOL
_isI2F(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isTypeEqualTo(VIR_Inst_GetDest(Inst), VIR_TYFLAG_ISFLOAT) &&
        _isTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISINTEGER);
}

static gctBOOL
_hasSHEnhancements2(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    if (((VIR_PatternMCLowerContext *)Context)->hasSHEnhancements2)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_isI2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isTypeEqualTo(VIR_Inst_GetDest(Inst), VIR_TYFLAG_ISINTEGER) &&
        _isTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISINTEGER);
}

static gctBOOL
_isF2IRnd(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return _isTypeEqualTo(VIR_Inst_GetDest(Inst), VIR_TYFLAG_ISINTEGER) &&
        _isTypeEqualTo(VIR_Inst_GetSource(Inst, 0), VIR_TYFLAG_ISFLOAT) &&
        VIR_Operand_GetRoundMode(VIR_Inst_GetDest(Inst)) != VIR_ROUND_DEFAULT;
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
        VIR_Symbol_GetKind(sym) == VIR_SYM_IMAGE)
    {
        primTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(sym));

        if (VIR_TypeId_isImage3D(primTypeId))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctINT
_ConvType(
    IN VIR_TypeId Ty
    )
{
    switch (Ty)
    {
    case VIR_TYPE_FLOAT32:
        return 0x0;
    case VIR_TYPE_FLOAT16:
        return 0x1;
    case VIR_TYPE_INT32:
        return 0x2;
    case VIR_TYPE_INT16:
        return 0x3;
    case VIR_TYPE_INT8:
        return 0x4;
    case VIR_TYPE_UINT32:
        return 0x5;
    case VIR_TYPE_UINT16:
        return 0x6;
    case VIR_TYPE_UINT8:
        return 0x7;
    case VIR_TYPE_SNORM16:
        return 0xB;
    case VIR_TYPE_SNORM8:
        return 0xC;
    case VIR_TYPE_UNORM16:
        return 0xE;
    case VIR_TYPE_UNORM8:
        return 0xF;
    case VIR_TYPE_INT64:
        return 0xA;
    case VIR_TYPE_UINT64:
        return 0xD;
    case VIR_TYPE_FLOAT64:
        return 0x0;
    case VIR_TYPE_BOOLEAN:
        return 0x2;
    default:
        return 0x0;
    }
}

static gctBOOL
_setAbs(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_Operand_SetModifier(Opnd, VIR_MOD_ABS);
    return gcvTRUE;
}

static gctBOOL
_setConv(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_ScalarConstVal imm0;
    VIR_TypeId         ty           = VIR_Operand_GetType(VIR_Inst_GetSource(Inst, 0));
    VIR_TypeId         componentTy  = VIR_GetTypeComponentType(ty);

    imm0.iValue = _ConvType(componentTy),

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    return gcvTRUE;
}

static gctBOOL
_setI2I(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
)
{
    VIR_ScalarConstVal imm0;
    VIR_TypeId         ty           = VIR_Operand_GetType(VIR_Inst_GetDest(Inst));
    VIR_TypeId         componentTy  = VIR_GetTypeComponentType(ty);

    imm0.iValue = _ConvType(componentTy);
    imm0.iValue <<= 4;

    VIR_Operand_SetImmediate(VIR_Inst_GetSource(Inst, 1),
        VIR_TYPE_INT32,
        imm0);

    VIR_Operand_SetModifier(Opnd, VIR_MOD_NONE);
    VIR_Operand_SetRoundMode(Opnd, VIR_ROUND_DEFAULT);

    return gcvTRUE;
}

static gctBOOL
_shouldSetHelper(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
)
{
    VIR_Operand *src0        = VIR_Inst_GetSource(Inst, 0);
    VIR_Operand *src1        = VIR_Inst_GetSource(Inst, 1);

    VIR_Symbol  *src0_symbol = VIR_Operand_GetSymbol(src0);
    VIR_TypeId   ty0         = VIR_Operand_GetType(src0);
    VIR_TypeId   ty1         = VIR_Operand_GetType(src1);

    if (!VIR_Operand_isSymbol(src0))
    {
        return gcvFALSE;
    }

    if (!VIR_Symbol_isAttribute(src0_symbol))
    {
        return gcvFALSE;
    }

    if (VIR_Symbol_GetName(src0_symbol) != VIR_NAME_HELPER_INVOCATION)
    {
        return gcvFALSE;
    }

    if (VIR_Operand_GetOpKind(src1) != VIR_OPND_IMMEDIATE)
    {
        return gcvFALSE;
    }

    if (ty0 >= VIR_TYPE_LAST_PRIMITIVETYPE ||
        ty1 >= VIR_TYPE_LAST_PRIMITIVETYPE ||
        VIR_GetTypeComponentType(ty0) != VIR_TYPE_BOOLEAN ||
        VIR_GetTypeComponentType(ty1) != VIR_TYPE_BOOLEAN)
    {
        return gcvFALSE;
    }

    if (VIR_Operand_GetImmediateUint(src1) != 1)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

/* cmov.cond    dst, src0, src1, src2 */
    /* cmp.cond         dst0, src0, src1*/
    /* select.selmsb    dst, dst0, src1, src2  */
static VIR_PatternMatchInst _cmovPatInst0[] = {
    { VIR_OP_CMOV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { _hasNotHalti4, _dual16Req }, VIR_PATN_MATCH_FLAG_OR },
};

/* CMP, if dest.type is not float,
   dest = cond_op(src0, src1) ? 0xFFFFFFFF: 0,
   thus, we can use SELMSB condition for select.
   Using SELMSB, we can resolve the issue that select has one instruction type to control
   comparison and implicit converstion */
static VIR_PatternReplaceInst _cmovRepInst0[] = {
    { VIR_OP_CMP, -1, 0, { -1, 2, 3, 0 }, { _setIntegerType } },
    { VIR_OP_SELECT, VIR_COP_SELMSB, 0, {  1, -1, 4, 1 }, { 0 } }
};

static VIR_Pattern _cmovPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmov, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _imgaddrPatInst0[] = {
    { VIR_OP_IMG_ADDR, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3 }, { _hasHalti4, _isActOn3DImg }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _imgaddrRepInst0[] = {
    { VIR_OP_IMG_ADDR_3D, -1, 0, { 1, 2, 3, 0 }, { 0 } }
};

static VIR_Pattern _imgaddrPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgaddr, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _madPatInst0[] = {
    { VIR_OP_MAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { VIR_Lower_IsIntOpcode, VIR_Lower_IsDstInt16, _hasNotHalti4 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _madRepInst0[] = {
    { VIR_OP_AQ_IMADLO0, 0, 0, { 1, 2, 3, 4 }, { 0 } },
    { VIR_OP_AQ_IMADLO1, 0, 0, { 1, 2, 3, 4 }, { 0 } }
};

static VIR_PatternMatchInst _madPatInst1[] = {
    { VIR_OP_MAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { VIR_Lower_IsIntOpcode }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _madRepInst1[] = {
    { VIR_OP_AQ_IMADLO0, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _madPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mad, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_mad, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _cmpPatInst0[] = {
    { VIR_OP_CMP, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst0[] = {
    { VIR_OP_AQ_CMP, -1, 0, {  1, 2, 3, 0 }, { 0, 0, 0, VIR_Lower_SetOne } }
};

static VIR_PatternMatchInst _cmpPatInst1[] = {
    { VIR_OP_CMP, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _cmpRepInst1[] = {
    { VIR_OP_AQ_CMP, -1, 0, { 1, 2, 3, 0 }, { 0, 0, 0, VIR_Lower_SetIntMinusOne } }
};

static VIR_Pattern _cmpPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_cmp, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _selectPatInst0[] = {
    { VIR_OP_SELECT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR},
};

static VIR_PatternReplaceInst _selectRepInst0[] = {
    { VIR_OP_AQ_SELECT, -1, 0, {  1, 2, 3, 4 }, { 0 } }
};

static VIR_Pattern _selectPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_select, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _convPatInst0[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _sameType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst0[] = {
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};

static VIR_PatternMatchInst _convPatInst1[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2I }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst1[] = {
    { VIR_OP_AQ_F2I, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};

static VIR_PatternMatchInst _convPatInst2[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isF2IRnd }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst2[] = {
    { VIR_OP_AQ_F2IRND, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};

static VIR_PatternMatchInst _convPatInst3[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2F }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst3[] = {
    { VIR_OP_AQ_I2F, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};

static VIR_PatternMatchInst _convPatInst4[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I, _hasSHEnhancements2, _notSameSizeType }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst4[] = {
    { VIR_OP_AQ_I2I, -1, 0, { 1, 2, 0, 0 }, { _setI2I } }
};

static VIR_PatternMatchInst _convPatInst5[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isI2I }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _convRepInst5[] = {
    { VIR_OP_MOV, -1, 0, { 1, 2, 0, 0 }, { 0 } }
};

static VIR_PatternMatchInst _convPatInst6[] = {
    { VIR_OP_CONV, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _convRepInst6[] = {
    { VIR_OP_AQ_CONV, -1, 0, { 1, 2, 0, 0 }, { _setConv } }
};

static VIR_Pattern _convPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_conv, 6) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _setPatInst0[] = {
    { VIR_OP_AQ_SET, VIR_COP_EQUAL, 0, { 1, 2, 3, 0 }, { _shouldSetHelper }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _setRepInst0[] = {
    { VIR_OP_AQ_SET, VIR_COP_HELPER, 0, { 1, 2, 3, 0 }, { 0 } }
};

static VIR_Pattern _setPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_set, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _negPatInst0[] = {
    { VIR_OP_NEG, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_IsIntOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _negRepInst0[] = {
    { VIR_OP_SUB, -1, 0, { 1, 0, 2, 0 }, { 0, VIR_Lower_SetZero } }
};

static VIR_Pattern _negPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_neg, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _absPatInst0[] = {
    { VIR_OP_ABS, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { VIR_Lower_IsFloatOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _absRepInst0[] = {
    { VIR_OP_ADD, -1, 0, { 1, 0, 2, 0 }, { 0, VIR_Lower_SetZero, _setAbs } }
};

static VIR_Pattern _absPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_abs, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _mulhiSclPatInst0[] = {
    { VIR_OP_MULHI, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _mulhiSclRepInst0[] = {
    { VIR_OP_MULHI, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _mulhiSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_mulhiScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _mulloSclPatInst0[] = {
    { VIR_OP_MULLO, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _mulloSclRepInst0[] = {
    { VIR_OP_MULLO, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _mulloSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_mulloScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _mulSclPatInst0[] = {
    { VIR_OP_MUL, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstInt32 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _mulSclRepInst0[] = {
    { VIR_OP_MUL, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _mulSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_mulScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _mulsatSclPatInst0[] = {
    { VIR_OP_MULSAT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstInt32 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _mulsatSclRepInst0[] = {
    { VIR_OP_MULSAT, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _mulsatSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_mulsatScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctUINT
_getConstValueFit5Bits(
    VIR_Const *      pConstVal
    )
{
    gctUINT imm = 0;
    VIR_TypeId tyId  = pConstVal->type;
    if (VIR_TypeId_isPrimitive(tyId))
    {
        gctINT components = VIR_GetTypeComponents(tyId);
        int i;
        if (components > 3)
            return imm;

        for (i = 0; i < components; i++)
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = pConstVal->value.vecVal.i32Value[i];
                gcmASSERT (!((value >= 16) || (value < -16)));
                imm = (imm | (value & 0x1f) << (5*i));
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT value = pConstVal->value.vecVal.u32Value[i];
                gcmASSERT((value < 16));
                imm = (imm | (value & 0x0f) << (5*i));
            }
            else
            {
                gcmASSERT(0);
            }
        }
        /* all values are in 5 bits range */
    }
    return imm;
}

static gctBOOL
_isConstValueFit5Bits(
    VIR_Const *      pConstVal
    )
{
    VIR_TypeId tyId  = pConstVal->type;
    if (VIR_TypeId_isPrimitive(tyId))
    {
        gctINT components = VIR_GetTypeComponents(tyId);
        int i;
        if (components > 3)
            return gcvFALSE;

        for (i = 0; i < components; i++)
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = pConstVal->value.vecVal.i32Value[i];
                if ((value >= 16) || (value < -16))
                {
                    return gcvFALSE;
                }
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT value = pConstVal->value.vecVal.u32Value[i];
                if ((value >= 16))
                {
                    return gcvFALSE;
                }
            }
            else
            {
                return gcvFALSE;
            }
        }
        /* all values are in 5 bits range */
        return gcvTRUE;
    }
    return gcvFALSE;
}

static gctBOOL
_isSrc1ConstFit5Bits(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
    VIR_TypeId    tyId;
    if (VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetType(src1);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = VIR_Operand_GetImmediateInt(src1);
                return (value < 16) && (value >= -16);
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT uValue = VIR_Operand_GetImmediateUint(src1);
                return (uValue < 16);
            }
        }
    }
    else
    {
        if (VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST)
        {
            VIR_ConstId constID = VIR_Operand_GetConstId(src1);
            VIR_Const*  cValue  = VIR_Shader_GetConstFromId(Context->shader, constID);
            return _isConstValueFit5Bits(cValue);
        }
        else if (VIR_Operand_isSymbol(src1))
        {
            VIR_Symbol *  sym = VIR_Operand_GetSymbol(src1);
            if (VIR_Symbol_isUniform(sym) && isSymUniformCompiletimeInitialized(sym))
            {
                VIR_Const * cValue =
                    (VIR_Const *)VIR_GetSymFromId(&Context->shader->constTable,
                                              VIR_Uniform_GetInitializer(sym->u2.uniform));
                return _isConstValueFit5Bits(cValue);
            }
        }
    }
    return gcvFALSE;
}

static gctBOOL
_noOffsetAndPrevInstUseAllComponents(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    VIR_Operand * src2 = VIR_Inst_GetSource(Inst, 2);
    VIR_Instruction * prevInst;
    if (src2 && !VIR_Operand_isUndef(src2))
    {
        return gcvFALSE;
    }
    prevInst = VIR_Inst_GetPrev(Inst);
    if (prevInst)
    {
        /* check inst components are the same as coordinate components */
        VIR_Enable    prevEnable = VIR_Inst_GetEnable(prevInst);
        VIR_Operand * src1 = VIR_Inst_GetSource(Inst, 1);
        VIR_Swizzle   src1Swizzle = VIR_Operand_GetSwizzle(src1);
        VIR_Enable    src1Enable = VIR_Swizzle_2_Enable(src1Swizzle);
        gcmASSERT(VIR_Inst_GetOpcode(prevInst) == VIR_OP_ADD && src1);
        return VIR_Enable_Covers(prevEnable, src1Enable);
    }
    return gcvTRUE;
}
static gctBOOL
_SetImmOffset(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_TypeId    tyId;
    gctUINT imm = 0;
    if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        tyId = VIR_Operand_GetType(Opnd);
        if (VIR_TypeId_isPrimitive(tyId))
        {
            if (VIR_TypeId_isSignedInteger(tyId))
            {
                gctINT value = VIR_Operand_GetImmediateInt(Opnd);
                gcmASSERT ((value < 16) && (value >= -16));
                imm = ((value & 0x1f) << 5) | (value & 0x1f);
            }
            else if (VIR_TypeId_isUnSignedInteger(tyId))
            {
                gctUINT uValue = VIR_Operand_GetImmediateUint(Opnd);
                gcmASSERT (uValue < 16);
                imm = ((uValue & 0x0f) << 5) | (uValue & 0x0f);
            }
        }
    }
    else
    {
        if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
        {
            VIR_ConstId constID = VIR_Operand_GetConstId(Opnd);
            VIR_Const*  cValue  = VIR_Shader_GetConstFromId(Context->shader, constID);
            imm = _getConstValueFit5Bits(cValue);
        }
        else if (VIR_Operand_isSymbol(Opnd))
        {
            VIR_Symbol *  sym = VIR_Operand_GetSymbol(Opnd);
            if (VIR_Symbol_isUniform(sym) && isSymUniformCompiletimeInitialized(sym))
            {
                VIR_Const * cValue =
                    (VIR_Const *)VIR_GetSymFromId(&Context->shader->constTable,
                                              VIR_Uniform_GetInitializer(sym->u2.uniform));
                imm = _getConstValueFit5Bits(cValue);
            }
        }
    }
    if (imm != 0)
    {
        /* change operand to Immediate */
        VIR_Operand_SetImmediateInt(Opnd, imm);
    }
    return gcvTRUE;
}

/*
add      1, 2, 3
img_load   4, 5, 1, 6
    img_load 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst2[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst2[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_IMG_LOAD, 0, 0, { 4, 5, 2, 3 }, { 0, 0, 0, _SetImmOffset } },
};

/*
add      1, 2, 3
img_load_3d   4, 5, 1, 6
    img_load_3d 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst3[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst3[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_IMG_LOAD_3D, 0, 0, { 4, 5, 2, 3 }, { 0, 0, 0, _SetImmOffset } },
};

/*
add      1, 2, 3
vx_img_load   4, 5, 1, 6, 7
    vx_img_load 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst4[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6, 7 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst4[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_VX_IMG_LOAD, 0, 0, { 4, 5, 2, 3, 7 }, {0, 0, 0, _SetImmOffset } },
};

/*
add      1, 2, 3
vx_img_load_3d   4, 5, 1, 6, 7
    vx_img_load_3d 4, 5, 2, 3
*/
static VIR_PatternMatchInst _addPatInst5[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isSrc1ConstFit5Bits }, VIR_PATN_MATCH_FLAG_OR },
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 4, 5, 1, 6, 7 }, { _noOffsetAndPrevInstUseAllComponents }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _addRepInst5[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } }, /* keep the add in case it is used by other inst */
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, { 4, 5, 2, 3, 7 }, { 0, 0, 0, _SetImmOffset } },
};

static VIR_Pattern _addPattern[] = {
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_add, 2) },
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_add, 3) },
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_add, 4) },
    { VIR_PATN_FLAG_RECURSIVE_SCAN, CODEPATTERN(_add, 5) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _addSclPatInst0[] = {
    { VIR_OP_ADD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { VIR_Lower_IsDstInt32, _isCL_X }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _addSclRepInst0[] = {
    { VIR_OP_ADD, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _addSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_addScl, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _madSclPatInst0[] = {
    { VIR_OP_MAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { VIR_Lower_IsIntOpcode }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _madSclRepInst0[] = {
    { VIR_OP_MAD, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _madSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_madScl, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _madsatSclPatInst0[] = {
    { VIR_OP_MADSAT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { VIR_Lower_IsDstInt32 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _madsatSclRepInst0[] = {
    { VIR_OP_MADSAT, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _madsatSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_madsatScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _imadhi0SclPatInst0[] = {
    { VIR_OP_AQ_IMADHI0, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imadhi0SclRepInst0[] = {
    { VIR_OP_AQ_IMADHI0, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _imadhi0SclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_imadhi0Scl, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _imadhi1SclPatInst0[] = {
    { VIR_OP_AQ_IMADHI1, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imadhi1SclRepInst0[] = {
    { VIR_OP_AQ_IMADHI1, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _imadhi1SclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_imadhi1Scl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _imadlo0SclPatInst0[] = {
    { VIR_OP_AQ_IMADLO0, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4 }, { VIR_Lower_IsDstInt32 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imadlo0SclRepInst0[] = {
    { VIR_OP_AQ_IMADLO0, 0, 0, { 1, 2, 3, 4 }, { 0 } },
};

static VIR_Pattern _imadlo0SclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_imadlo0Scl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _f2iSclPatInst0[] = {
    { VIR_OP_AQ_F2I, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _f2iSclRepInst0[] = {
    { VIR_OP_AQ_F2I, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _f2iSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_f2iScl, 0) },
    { VIR_PATN_FLAG_NONE }
};


static VIR_PatternMatchInst _f2irndSclPatInst0[] = {
    { VIR_OP_AQ_F2IRND, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _f2irndSclRepInst0[] = {
    { VIR_OP_AQ_F2IRND, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _f2irndSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_f2irndScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _i2fSclPatInst0[] = {
    { VIR_OP_AQ_I2F, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _i2fSclRepInst0[] = {
    { VIR_OP_AQ_I2F, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _i2fSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_i2fScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _xorSclPatInst0[] = {
    { VIR_OP_XOR_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _xorSclRepInst0[] = {
    { VIR_OP_XOR_BITWISE, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _xorSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_xorScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _notSclPatInst0[] = {
    { VIR_OP_NOT_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _notSclRepInst0[] = {
    { VIR_OP_NOT_BITWISE, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

static VIR_Pattern _notSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_notScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

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

/*
** LOGICAL_NOT, temp(1), temp(2)
** ==>
** SELECT.nz, temp(1), temp(2), false, true
*/
static VIR_PatternMatchInst _logicalNotSclPatInst0[] = {
    { VIR_OP_LOGICAL_NOT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { 0 }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _logicalNotSclRepInst0[] = {
    { VIR_OP_AQ_SELECT, VIR_COP_NOT_ZERO, 0, { 1, 2, 0, 0 }, { 0, 0, _constb_false, _constb_true } },
};

static VIR_Pattern _logicalNotSclPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_logicalNotScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _andSclPatInst0[] = {
    { VIR_OP_AND_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _andSclRepInst0[] = {
    { VIR_OP_AND_BITWISE, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _andSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_andScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _orSclPatInst0[] = {
    { VIR_OP_OR_BITWISE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _orSclRepInst0[] = {
    { VIR_OP_OR_BITWISE, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _orSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_orScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _rotateSclPatInst0[] = {
    { VIR_OP_ROTATE, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _rotateSclRepInst0[] = {
    { VIR_OP_ROTATE, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _rotateSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_rotateScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _lshiftSclPatInst0[] = {
    { VIR_OP_LSHIFT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _lshiftSclRepInst0[] = {
    { VIR_OP_LSHIFT, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _lshiftSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_lshiftScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _rshiftSclPatInst0[] = {
    { VIR_OP_RSHIFT, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isCL_X }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _rshiftSclRepInst0[] = {
    { VIR_OP_RSHIFT, 0, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _rshiftSclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE | VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O, CODEPATTERN(_rshiftScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _jmpanySclPatInst0[] = {
    { VIR_OP_JMP_ANY, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 0 }, { _isHWNotSupportJmpAny }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _jmpanySclRepInst0[] = {
    { VIR_OP_JMPC, -1, 0, { 1, 2, 3, 0 }, { 0 } },
};

static VIR_Pattern _jmpanySclPattern[] = {
    { VIR_PATN_FLAG_EXPAND_COMP_O2O_SRC_ONLY_INLINE, CODEPATTERN(_jmpanyScl, 0) },
    { VIR_PATN_FLAG_NONE }
};

static gctBOOL
_HasDual16Support(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return (((VIR_PatternLowerContext *)Context)->hwCfg->hwFeatureFlags.supportDual16);
}

static gctBOOL
_OnlyXEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst)) == VIR_ENABLE_X;
}

static gctBOOL
_OnlyXYEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst)) == VIR_ENABLE_XY;
}

static gctBOOL
_OnlyXYZEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst)) == VIR_ENABLE_XYZ;
}

static gctBOOL
_XYZWEnabled(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst)) == VIR_ENABLE_XYZW;
}

static gctBOOL
_DstSrcDiffPrecision(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst
    )
{
    return ((VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst)) == VIR_PRECISION_HIGH &&
             VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0)) != VIR_PRECISION_HIGH) ||
            (VIR_Operand_GetPrecision(VIR_Inst_GetDest(Inst)) != VIR_PRECISION_HIGH &&
             VIR_Operand_GetPrecision(VIR_Inst_GetSource(Inst, 0)) == VIR_PRECISION_HIGH));
}

static gctBOOL
_destEnableX(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), VIR_ENABLE_X);
    return gcvTRUE;
}

static gctBOOL
_swizzleDxS0x(
    IN VIR_PatternContext *Context,
    IN VIR_Instruction    *Inst,
    IN VIR_Operand        *Opnd
    )
{
    gctUINT8            swizzle = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, 0)) & 0x3;
    VIR_Type            *ty    = VIR_Shader_GetTypeFromId(Context->shader, VIR_Operand_GetType(VIR_Inst_GetDest(Inst)));
    VIR_PrimitiveTypeId  baseType;
    VIR_PrimitiveTypeId  dstTy;

    gcmASSERT(VIR_Type_isPrimitive(ty));

    baseType = VIR_Type_GetBaseTypeId(ty);

    dstTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(baseType), 1, 1);

    VIR_Operand_SetType(VIR_Inst_GetDest(Inst), dstTy);

    VIR_Operand_SetEnable(VIR_Inst_GetDest(Inst), VIR_ENABLE_X);
    VIR_Operand_SetSwizzle(VIR_Inst_GetSource(Inst, 0),
        (VIR_Swizzle)(swizzle | (swizzle << 2) | (swizzle << 4) | (swizzle << 6)));

    return gcvTRUE;
}

static VIR_PatternMatchInst _normPatInst0[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _normRepInst0[] = {
    { VIR_OP_SIGN, 0, 0, { 1, 2, 0, 0 }, { 0 } },
};

/* the sequence of norm_dp, rsq, norm_mul must have the same thread type,
   if the src and dest have different precision, we need to have an extra MOV */
static VIR_PatternMatchInst _normPatInst1[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYEnabled, _HasDual16Support, _DstSrcDiffPrecision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst1[] = {
    { VIR_OP_NORM_DP2, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, { -3, 2, -2, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 0, {  1, -3, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst2[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYEnabled, _HasDual16Support }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst2[] = {
    { VIR_OP_NORM_DP2, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst3[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _normRepInst3[] = {
    { VIR_OP_DP2, 0, 0, { -1, 2, 2, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst4[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYZEnabled, _HasDual16Support, _DstSrcDiffPrecision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst4[] = {
    { VIR_OP_NORM_DP3, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, { -3, 2, -2, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 0, {  1, -3, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst5[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYZEnabled, _HasDual16Support }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst5[] = {
    { VIR_OP_NORM_DP3, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst6[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _OnlyXYZEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _normRepInst6[] = {
    { VIR_OP_DP3, 0, 0, { -1, 2, 2, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst7[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _XYZWEnabled, _HasDual16Support, _DstSrcDiffPrecision }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst7[] = {
    { VIR_OP_NORM_DP4, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, { -3, 2, -2, 0 }, { 0 } },
    { VIR_OP_MOV, 0, 0, {  1, -3, 0, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst8[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _XYZWEnabled, _HasDual16Support }, VIR_PATN_MATCH_FLAG_AND },
};

static VIR_PatternReplaceInst _normRepInst8[] = {
    { VIR_OP_NORM_DP4, 0, 0, { -1, 2, 0, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_NORM_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_PatternMatchInst _normPatInst9[] = {
    { VIR_OP_NORM, VIR_PATTERN_ANYCOND, 0, { 1, 2, 0, 0 }, { _XYZWEnabled }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _normRepInst9[] = {
    { VIR_OP_DP4, 0, 0, { -1, 2, 2, 0 }, { _destEnableX } },
    { VIR_OP_RSQ, 0, 0, { -2, -1, 0, 0 }, { _swizzleDxS0x } },
    { VIR_OP_MUL, 0, 0, {  1, 2, -2, 0 }, { 0 } },
};

static VIR_Pattern _normPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 1) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 2) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 3) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 4) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 5) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 6) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 7) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 8) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_norm, 9) },
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

    VIR_Operand_SetLvalue(Opnd, 0);
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
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, {_isSrc1FloatType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoadRepInst0[] = {
    { VIR_OP_CONV, 0, 0, {  -1, 3, 0 ,0 }, {_setIntegerType_EnableXY } },
    { VIR_OP_VX_IMG_LOAD, 0, 0, {  1, 2, -1, 4, 5 }, { 0 } },
};

static VIR_PatternMatchInst _vxImgLoadPatInst1[] = {
    { VIR_OP_VX_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoadRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_VX_IMG_LOAD, 0, 0, {  1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _vxImgLoadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _vxImgLoad3DPatInst0[] = {
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, {_isSrc1FloatType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoad3DRepInst0[] = {
    { VIR_OP_CONV, 0, 0, {  -1, 3, 0 ,0 }, {_setIntegerType_EnableXYZ } },
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, {  1, 2, -1, 4, 5 }, { 0 } },
};

static VIR_PatternMatchInst _vxImgLoad3DPatInst1[] = {
    { VIR_OP_VX_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _vxImgLoad3DRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_VX_IMG_LOAD_3D, 0, 0, { 1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _vxImgLoad3DPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad3D, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_vxImgLoad3D, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _imgLoadPatInst0[] = {
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, {_isSrc1FloatType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgLoadRepInst0[] = {
    { VIR_OP_CONV, 0, 0, {  -1, 3, 0 ,0 }, {_setIntegerType_EnableXY } },
    { VIR_OP_IMG_LOAD, 0, 0, {  1, 2, -1, 4, 5 }, { 0 } },
};

static VIR_PatternMatchInst _imgLoadPatInst1[] = {
    { VIR_OP_IMG_LOAD, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgLoadRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_IMG_LOAD, 0, 0, {  1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _imgLoadPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgLoad, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgLoad, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_PatternMatchInst _imgLoad3DPatInst0[] = {
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, { 1, 2, 3, 4, 5 }, {_isSrc1FloatType }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgLoad3DRepInst0[] = {
    { VIR_OP_CONV, 0, 0, {  -1, 3, 0 ,0 }, {_setIntegerType_EnableXYZ } },
    { VIR_OP_IMG_LOAD_3D, 0, 0, {  1, 2, -1, 4, 5 }, { 0 } },
};

static VIR_PatternMatchInst _imgLoad3DPatInst1[] = {
    { VIR_OP_IMG_LOAD_3D, VIR_PATTERN_ANYCOND, 0, {  1, 2, 3, 4, 5}, { _needSetConstBorderValue }, VIR_PATN_MATCH_FLAG_OR },
};

static VIR_PatternReplaceInst _imgLoad3DRepInst1[] = {
    { VIR_OP_MOV, 0, 0, {  1, 0, 0 ,0 }, {0, _setConstBorderValue } },
    { VIR_OP_IMG_LOAD_3D, 0, 0, { 1, 2, 3, 4, 5 }, { 0 } },
};

static VIR_Pattern _imgLoad3DPattern[] = {
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgLoad3D, 0) },
    { VIR_PATN_FLAG_NONE, CODEPATTERN(_imgLoad3D, 1) },
    { VIR_PATN_FLAG_NONE }
};

static VIR_Pattern*
_GetPattern0(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    switch(opcode)
    {
    case VIR_OP_CMOV:
        return _cmovPattern;
    case VIR_OP_MAD:
        return _madPattern;
    case VIR_OP_IMG_ADDR:
        return _imgaddrPattern;
    case VIR_OP_NORM:
        return _normPattern;
    default:
        break;
    }
    return gcvNULL;
}

static VIR_Pattern*
_GetPattern1(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    switch(opcode)
    {
    case VIR_OP_ADD:
        return _addPattern;
    case VIR_OP_CMP:
        return _cmpPattern;
    case VIR_OP_SELECT:
        return _selectPattern;
    case VIR_OP_CONV:
        {
            return _convPattern;
        }
    case VIR_OP_AQ_SET:
        return _setPattern;
    case VIR_OP_NEG:
        return _negPattern;
    case VIR_OP_ABS:
        return _absPattern;
    case VIR_OP_VX_IMG_LOAD:
        return _vxImgLoadPattern;
    case VIR_OP_VX_IMG_LOAD_3D:
        return _vxImgLoad3DPattern;
    case VIR_OP_IMG_LOAD:
        return _imgLoadPattern;
    case VIR_OP_IMG_LOAD_3D:
        return _imgLoad3DPattern;
    default:
        break;
    }
    return gcvNULL;
}

static VIR_Pattern*
_GetPatternScalar(
    IN VIR_PatternContext      *Context,
    IN VIR_Instruction         *Inst
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(Inst);
    switch(opcode)
    {
    case VIR_OP_MULHI:
        return _mulhiSclPattern;
    case VIR_OP_MULLO:
        return _mulloSclPattern;
    case VIR_OP_MUL:
        return _mulSclPattern;
    case VIR_OP_MULSAT:
        return _mulsatSclPattern;
    case VIR_OP_ADD:
        return _addSclPattern;
    case VIR_OP_AQ_IMADHI0:
        return _imadhi0SclPattern;
    case VIR_OP_AQ_IMADHI1:
        return _imadhi1SclPattern;
    case VIR_OP_AQ_IMADLO0:
        return _imadlo0SclPattern;
    case VIR_OP_MAD:
        return _madSclPattern;
    case VIR_OP_MADSAT:
        return _madsatSclPattern;
    case VIR_OP_AQ_F2I:
        return _f2iSclPattern;
    case VIR_OP_AQ_F2IRND:
        return _f2irndSclPattern;
    case VIR_OP_AQ_I2F:
        return _i2fSclPattern;
    case VIR_OP_XOR_BITWISE:
        return _xorSclPattern;
    case VIR_OP_AND_BITWISE:
        return _andSclPattern;
    case VIR_OP_OR_BITWISE:
        return _orSclPattern;
    case VIR_OP_NOT_BITWISE:
        return _notSclPattern;
    case VIR_OP_LOGICAL_NOT:
        return _logicalNotSclPattern;
    case VIR_OP_ROTATE:
        return _rotateSclPattern;
    case VIR_OP_LSHIFT:
        return _lshiftSclPattern;
    case VIR_OP_RSHIFT:
        return _rshiftSclPattern;
    case VIR_OP_JMP_ANY:
        return _jmpanySclPattern;
    default:
        break;
    }
    return gcvNULL;
}

static void
_Lower_Initialize(
    IN VIR_Shader                *Shader,
    IN VIR_PatternMCLowerContext *Context,
    IN VSC_HW_CONFIG             *HwCfg,
    IN VSC_MM                    *pMM
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

    Context->hasHalti4 = (gctBOOL)HwCfg->hwFeatureFlags.hasHalti4;

    Context->hasSHEnhancements2 = (gctBOOL)HwCfg->hwFeatureFlags.hasSHEnhance2;
}

DEF_QUERY_PASS_PROP(VIR_Lower_LowLevel_To_MachineCodeLevel)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    /* Lower must invalidate all analyzed resources */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateWeb = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateLvFlow = gcvTRUE;
}

VSC_ErrCode
VIR_Lower_LowLevel_To_MachineCodeLevel(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Shader * Shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    PVSC_CONTEXT VscContext = &pPassWorker->pCompilerParam->cfg.ctx;
    VSC_HW_CONFIG* HwCfg = &VscContext->pSysCtx->pCoreSysCtx->hwCfg;
    VIR_PatternMCLowerContext context;

    if (VIR_Shader_GetLevel(Shader) != VIR_SHLEVEL_Post_Low)
    {
        return errCode;
    }

    if (!gcUseFullNewLinker(HwCfg->hwFeatureFlags.hasHalti2))
    {
        VIR_Shader_SetLevel(Shader, VIR_SHLEVEL_Pre_Machine);
        return errCode;
    }

    _Lower_Initialize(Shader, &context, HwCfg, pPassWorker->basePassWorker.pMM);

    {
        VIR_PatternContext_Initialize(&context.header, VscContext, Shader, context.pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                      _GetPattern0, _CmpInstuction, 512);

        errCode = VIR_Pattern_Transform((VIR_PatternContext *)&context);
        CHECK_ERROR(errCode, "VIR_Lower_LowLevel_To_MachineCodeLevel failed.");

        VIR_PatternContext_Finalize(&context.header);
    }

    {
        VIR_PatternContext_Initialize(&context.header, VscContext, Shader, context.pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                      _GetPattern1, _CmpInstuction, 512);

        errCode = VIR_Pattern_Transform((VIR_PatternContext *)&context);
        CHECK_ERROR(errCode, "VIR_Lower_LowLevel_To_MachineCodeLevel failed.");

        VIR_PatternContext_Finalize(&context.header);
    }

    {
        VIR_PatternContext_Initialize(&context.header, VscContext, Shader, context.pMM, VIR_PATN_CONTEXT_FLAG_NONE,
                                      _GetPatternScalar, _CmpInstuction, 512);

        errCode = VIR_Pattern_Transform((VIR_PatternContext *)&context);
        CHECK_ERROR(errCode, "VIR_Lower_LowLevel_To_MachineCodeLevel failed.");

        VIR_PatternContext_Finalize(&context.header);
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(Shader), VIR_Shader_GetId(Shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Lowered to MachineLevel.", Shader, gcvTRUE);
    }

    VIR_Shader_SetLevel(Shader, VIR_SHLEVEL_Pre_Machine);

    return errCode;
}

