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


#include "vir/transform/gc_vsc_vir_simplification.h"

void VSC_SIMP_Simplification_Init(
    IN VSC_SIMP_Simplification* simp,
    IN VIR_Shader* shader,
    IN VIR_Function* currFunc,
    IN VIR_BASIC_BLOCK* currBB,
    IN VSC_OPTN_SIMPOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VSC_SIMP_Simplification_SetShader(simp, shader);
    VSC_SIMP_Simplification_SetCurrFunc(simp, currFunc);
    VSC_SIMP_Simplification_SetCurrBB(simp, currBB);
    VSC_SIMP_Simplification_SetOptions(simp, options);
    VSC_SIMP_Simplification_SetDumper(simp, dumper);
}

void VSC_SIMP_Simplification_Final(
    IN OUT VSC_SIMP_Simplification* simp
    )
{}

typedef gctBOOL (*_VSC_SIMP_InstValidate)(IN VIR_Instruction* inst);
typedef gctBOOL (*_VSC_SIMP_OpndValidate)(IN VIR_Instruction* inst, IN VIR_Operand* opnd);
typedef void (*_VSC_SIMP_Transform)(IN OUT VIR_Instruction* inst);

typedef enum _VSC_SIMP_STEPS_TYPE
{
    _VSC_SIMP_STEPS_COUNT,
    _VSC_SIMP_STEPS_INST_CHECK,
    _VSC_SIMP_STEPS_DEST_CHECK,
    _VSC_SIMP_STEPS_SRC0_CHECK,
    _VSC_SIMP_STEPS_SRC1_CHECK,
    _VSC_SIMP_STEPS_SRC2_CHECK,
    _VSC_SIMP_STEPS_TRANS,
    _VSC_SIMP_STEPS_END
} _VSC_SIMP_Steps_Type;

typedef struct _VSC_SIMP_STEPS
{
    _VSC_SIMP_Steps_Type type;
    union
    {
        gctUINTPTR_T count;
        _VSC_SIMP_InstValidate inst_vali;
        _VSC_SIMP_OpndValidate opnd_vali;
        _VSC_SIMP_Transform trans;
    } u;
} _VSC_SIMP_Steps;

#define VSC_SIMP_Steps_GetType(step)        ((step)->type)
#define VSC_SIMP_Steps_GetCount(step)       ((step)->u.count)
#define VSC_SIMP_Steps_GetInstVali(step)    ((step)->u.inst_vali)
#define VSC_SIMP_Steps_GetOpndVali(step)    ((step)->u.opnd_vali)
#define VSC_SIMP_Steps_GetTrans(step)       ((step)->u.trans)

/* _VSC_SIMP_STEPS_INST_CHECK functions */
gctBOOL _GZCond(
    IN VIR_Instruction* inst
    )
{
    return VIR_Inst_GetConditionOp(inst) == VIR_COP_GREATER_ZERO;
}

gctBOOL _NZCond(
    IN VIR_Instruction* inst
    )
{
    return VIR_Inst_GetConditionOp(inst) == VIR_COP_NOT_ZERO;
}

gctBOOL _Src0Src1Identical(
    IN VIR_Instruction* inst
    )
{
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);
    return memcmp(src0, src1, sizeof(VIR_Operand)) == 0;
}

gctBOOL _DestSrc0Identical(
    IN VIR_Instruction* inst
    )
{
    VIR_Operand* dest = VIR_Inst_GetDest(inst);
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    if(VIR_Operand_GetOpKind(src0) == VIR_OPND_SYMBOL && VIR_Operand_GetOpKind(src0) == VIR_OPND_SYMBOL)
    {
        VIR_Enable enable = VIR_Operand_GetEnable(dest);
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(src0);
        if(swizzle != VIR_Enable_2_Swizzle(enable))
        {
            return gcvFALSE;
        }
        return memcmp(VIR_Operand_GetSymbol(dest), VIR_Operand_GetSymbol(src0), sizeof(VIR_Symbol)) == 0;
    }
    return gcvFALSE;
}

/* _VSC_SIMP_STEPS_DEST_CHECK */

/* _VSC_SIMP_STEPS_SRC_CHECK */
gctBOOL _ABSModifier(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    return VIR_Operand_GetModifier(opnd) == VIR_MOD_ABS;
}

gctBOOL _ImmZero(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    return VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE && VIR_Operand_GetImmediateInt(opnd) == 0;
}

gctBOOL _ConstOrImm(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    return VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST || VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE;
}

gctBOOL _ConstOrImmZero(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
    {
        return VIR_Operand_GetImmediateInt(opnd) == 0;
    }
    else if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(opnd);
        VIR_Const* cur_const = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(opnd));
        return VIR_VecConstVal_AllZero(type, &cur_const->value.vecVal);
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL _ConstOrImmOne(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
    {
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(opnd);
        return VIR_ScalarConstVal_One(type, (VIR_ScalarConstVal*)&(opnd->u.n.u1));
    }
    else if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(opnd);
        VIR_Const* cur_const = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(opnd));
        return VIR_VecConstVal_AllOne(type, &cur_const->value.vecVal);
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL _ConstOrImm4294967295(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
    {
        return VIR_Operand_GetImmediateUint(opnd) == 0xFFFFFFFF;
    }
    else if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_Const* cur_const = VIR_Shader_GetConstFromId(shader, VIR_Operand_GetConstId(opnd));
        gctUINT i;
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(opnd);
        for(i = 0; i < VIR_CHANNEL_COUNT; i++)
        {
            if(cur_const->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle, i)] != 0xFFFFFFFF)
            return gcvFALSE;
        }
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL _Is16BitOrLess(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if (VIR_GetTypeSize(VIR_Operand_GetType(opnd)) <= 2)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL _ConstOrImm65535(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
    {
        return VIR_Operand_GetImmediateUint(opnd) == 65535;
    }
    else if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_Const* cur_const = VIR_Shader_GetConstFromId(shader, VIR_Operand_GetConstId(opnd));
        gctUINT i;
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(opnd);
        for(i = 0; i < VIR_CHANNEL_COUNT; i++)
        {
            if(cur_const->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle, i)] != 65535)
            return gcvFALSE;
        }
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

gctBOOL _Is8BitOrLess(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if (VIR_GetTypeSize(VIR_Operand_GetType(opnd)) <= 1)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

gctBOOL _ConstOrImm255(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
    {
        return VIR_Operand_GetImmediateUint(opnd) == 255;
    }
    else if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_Const* cur_const = VIR_Shader_GetConstFromId(shader, VIR_Operand_GetConstId(opnd));
        gctUINT i;
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(opnd);
        for(i = 0; i < VIR_CHANNEL_COUNT; i++)
        {
            if(cur_const->value.vecVal.u32Value[VIR_Swizzle_GetChannel(swizzle, i)] != 255)
            return gcvFALSE;
        }
        return gcvTRUE;
    }
    else
    {
        return gcvFALSE;
    }
}

/* _VSC_SIMP_STEPS_TRANS */
void _MOVDestSrc0(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetSrcNum(inst, 1);
}

void _MOVDestSrc1(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 1));
    VIR_Inst_SetSrcNum(inst, 1);
}

void _MOVDestSrc2(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 2));
    VIR_Inst_SetSrcNum(inst, 1);
}

void _MOVDestSumSrc0Src1(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);

    if(VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
    {
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_ScalarConstVal_AddScalarConstVal(type, (VIR_ScalarConstVal*)&(src0->u.n.u1), (VIR_ScalarConstVal*)&(src1->u.n.u1), (VIR_ScalarConstVal*)&(src0->u.n.u1));
    }
    else if((VIR_Operand_GetOpKind(src0) == VIR_OPND_CONST && VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
        || (VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST))
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_Const* cur_const;
        VIR_ConstVal new_const;
        VIR_ConstId new_const_id;

        memset(&new_const, 0, sizeof(VIR_ConstVal));

        if((VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST))
        {
            VIR_Inst_SetSource(inst, 0, src1);
            VIR_Inst_SetSource(inst, 1, src0);
            src0 = src1;
            src1 = VIR_Inst_GetSource(inst, 0);
        }
        cur_const = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src0));

        VIR_VecConstVal_AddScalarConstVal(type, &cur_const->value.vecVal, (VIR_ScalarConstVal*)&(src1->u.n.u1), &new_const.vecVal);
        VIR_Shader_AddConstant(shader, type, &new_const, &new_const_id);
        VIR_Operand_SetConstId(src0, new_const_id);
    }
    else
    {
        VIR_Shader* shader = VIR_Function_GetShader(VIR_Inst_GetFunction(inst));
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_Const* cur_const0 = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src0));
        VIR_Const* cur_const1 = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src1));
        VIR_ConstVal new_const;
        VIR_ConstId new_const_id;

        memset(&new_const, 0, sizeof(VIR_ConstVal));

        gcmASSERT(VIR_Operand_GetOpKind(src0) == VIR_OPND_CONST && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST);

        VIR_VecConstVal_AddVecConstVal(type, &cur_const0->value.vecVal, &cur_const1->value.vecVal, &new_const.vecVal);
        VIR_Shader_AddConstant(shader, type, &new_const, &new_const_id);
        VIR_Operand_SetConstId(src0, new_const_id);
    }
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetSrcNum(inst, 1);
}

void _MOVDestProductSrc0Src1(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* src1 = VIR_Inst_GetSource(inst, 1);

    if(VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
    {
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_ScalarConstVal_MulScalarConstVal(type, (VIR_ScalarConstVal*)&(src0->u.n.u1), (VIR_ScalarConstVal*)&(src1->u.n.u1), (VIR_ScalarConstVal*)&(src0->u.n.u1));
    }
    else if((VIR_Operand_GetOpKind(src0) == VIR_OPND_CONST && VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
        || (VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST))
    {
        VIR_Shader* shader = VIR_Inst_GetShader(inst);
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_Const* cur_const;
        VIR_ConstVal new_const;
        VIR_ConstId new_const_id;

        memset(&new_const, 0, sizeof(VIR_ConstVal));

        if((VIR_Operand_GetOpKind(src0) == VIR_OPND_IMMEDIATE && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST))
        {
            VIR_Inst_SetSource(inst, 0, src1);
            VIR_Inst_SetSource(inst, 1, src0);
            src0 = src1;
            src1 = VIR_Inst_GetSource(inst, 0);
        }
        cur_const = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src0));

        VIR_VecConstVal_MulScalarConstVal(type, &cur_const->value.vecVal, (VIR_ScalarConstVal*)&(src1->u.n.u1), &new_const.vecVal);
        VIR_Shader_AddConstant(shader, type, &new_const, &new_const_id);
        VIR_Operand_SetConstId(src0, new_const_id);
    }
    else
    {
        VIR_Shader* shader = VIR_Function_GetShader(VIR_Inst_GetFunction(inst));
        VIR_PrimitiveTypeId type = VIR_Operand_GetType(src0);
        VIR_Const* cur_const0 = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src0));
        VIR_Const* cur_const1 = (VIR_Const *)VIR_Shader_GetSymFromId(shader, VIR_Operand_GetConstId(src1));
        VIR_ConstVal new_const;
        VIR_ConstId new_const_id;

        memset(&new_const, 0, sizeof(VIR_ConstVal));

        gcmASSERT(VIR_Operand_GetOpKind(src0) == VIR_OPND_CONST && VIR_Operand_GetOpKind(src1) == VIR_OPND_CONST);

        VIR_VecConstVal_MulVecConstVal(type, &cur_const0->value.vecVal, &cur_const1->value.vecVal, &new_const.vecVal);
        VIR_Shader_AddConstant(shader, type, &new_const, &new_const_id);
        VIR_Operand_SetConstId(src0, new_const_id);
    }
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetSrcNum(inst, 1);
}

void _NOP(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Inst_SetOpcode(inst, VIR_OP_NOP);
    VIR_Inst_SetSrcNum(inst, 0);
}

/* Simplification Steps */

_VSC_SIMP_Steps ADD_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImm}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImm}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSumSrc0Src1}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps AND_BITWISE_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImm4294967295}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImm4294967295}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_Is16BitOrLess}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImm65535}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_Is16BitOrLess}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImm65535}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_Is8BitOrLess}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImm255}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_Is8BitOrLess}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImm255}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps LSHIFT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps MAD_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps MOV_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_DestSrc0Identical}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_NOP}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps MUL_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImm}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImm}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestProductSrc0Src1}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ConstOrImmOne}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImmOne}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps RSHIFT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_ConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps SELECT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {5}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_NZCond}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_ABSModifier}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_Src0Src1Identical}},
    {_VSC_SIMP_STEPS_SRC2_CHECK, {(gctUINTPTR_T)_ImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps SWIZZLE_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC2_CHECK, {(gctUINTPTR_T)_ImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_NOP}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps* _VSC_SIMP_GetSteps(
    IN VIR_OpCode opc)
{
    switch(opc)
    {
        case VIR_OP_SWIZZLE:
            return SWIZZLE_Steps;
        default:
            return gcvNULL;
    }
}

VSC_ErrCode VSC_SIMP_Simplification_PerformOnInst(
    IN VSC_SIMP_Simplification* simp,
    IN OUT VIR_Instruction* inst,
    OUT gctBOOL* changed
    )
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_OpCode opc = VIR_Inst_GetOpcode(inst);
    _VSC_SIMP_Steps* steps = _VSC_SIMP_GetSteps(opc);
    gctBOOL chg = gcvFALSE;

    if(steps == gcvNULL)
    {
        return errCode;
    }

    while(VSC_SIMP_Steps_GetType(steps) != _VSC_SIMP_STEPS_END)
    {
        gctUINT count;
        gctBOOL check = gcvTRUE;
        gcmASSERT(VSC_SIMP_Steps_GetType(steps) == _VSC_SIMP_STEPS_COUNT);
        count = (gctUINT)VSC_SIMP_Steps_GetCount(steps);
        ++steps;
        while(count > 0)
        {
            switch(VSC_SIMP_Steps_GetType(steps))
            {
                case _VSC_SIMP_STEPS_INST_CHECK:
                {
                    _VSC_SIMP_InstValidate inst_vali = VSC_SIMP_Steps_GetInstVali(steps);
                    check = (*inst_vali)(inst);
                    break;
                }
                case _VSC_SIMP_STEPS_DEST_CHECK:
                {
                    _VSC_SIMP_OpndValidate dest_vali = VSC_SIMP_Steps_GetOpndVali(steps);
                    check = (*dest_vali)(inst, VIR_Inst_GetDest(inst));
                    break;
                }
                case _VSC_SIMP_STEPS_SRC0_CHECK:
                {
                    _VSC_SIMP_OpndValidate src0_vali = VSC_SIMP_Steps_GetOpndVali(steps);
                    check = (*src0_vali)(inst, VIR_Inst_GetSource(inst, 0));
                    break;
                }
                case _VSC_SIMP_STEPS_SRC1_CHECK:
                {
                    _VSC_SIMP_OpndValidate src1_vali = VSC_SIMP_Steps_GetOpndVali(steps);
                    check = (*src1_vali)(inst, VIR_Inst_GetSource(inst, 1));
                    break;
                }
                case _VSC_SIMP_STEPS_SRC2_CHECK:
                {
                    _VSC_SIMP_OpndValidate src2_vali = VSC_SIMP_Steps_GetOpndVali(steps);
                    check = (*src2_vali)(inst, VIR_Inst_GetSource(inst, 2));
                    break;
                }
                case _VSC_SIMP_STEPS_TRANS:
                {
                    _VSC_SIMP_Transform trans = VSC_SIMP_Steps_GetTrans(steps);
                    VSC_OPTN_SIMPOptions* options = gcvNULL;
                    if(simp)
                    {
                        options = VSC_SIMP_Simplification_GetOptions(simp);
                    }
                    if(options && VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION))
                    {
                        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
                        VIR_LOG(dumper, "before SIMP:\n");
                        VIR_Inst_Dump(dumper, inst);
                    }
                    (*trans)(inst);
                    chg = gcvTRUE;
                    if(options && VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION))
                    {
                        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
                        VIR_LOG(dumper, "after SIMP:\n");
                        VIR_Inst_Dump(dumper, inst);
                    }
                    /* recursively simplify */
                    return VSC_SIMP_Simplification_PerformOnInst(simp, inst, gcvNULL);
                }
                default:
                {
                    gcmASSERT(0);
                }
            }
            if(check)
            {
                ++steps;
                --count;
            }
            else
            {
                steps += count;
                break;
            }
        }
    }

    if(changed)
    {
        *changed = chg;
    }
    return errCode;
}

VSC_ErrCode VSC_SIMP_Simplification_PerformOnBB(
    IN VSC_SIMP_Simplification* simp
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VSC_OPTN_SIMPOptions* options = VSC_SIMP_Simplification_GetOptions(simp);
    VIR_BASIC_BLOCK* bb = VSC_SIMP_Simplification_GetCurrBB(simp);
    VIR_Instruction* inst;

    /* dump input bb */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_INPUT_BB))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification Start for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, bb->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvFALSE);
    }

    inst = BB_GET_START_INST(bb);
    while(inst != VIR_Inst_GetNext(BB_GET_END_INST(bb)))
    {
        VSC_SIMP_Simplification_PerformOnInst(simp, inst, gcvNULL);
        inst = VIR_Inst_GetNext(inst);
    }

    /* dump output bb */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_OUTPUT_BB))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification End for BB %d\n%s\n",
            VSC_TRACE_STAR_LINE, bb->dgNode.id, VSC_TRACE_STAR_LINE);
        VIR_BasicBlock_Dump(dumper, bb, gcvFALSE);
    }

    return errCode;
}

VSC_ErrCode VSC_SIMP_Simplification_PerformOnFunction(
    IN OUT VSC_SIMP_Simplification* simp
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VSC_OPTN_SIMPOptions* options = VSC_SIMP_Simplification_GetOptions(simp);
    VIR_Function* func;
    VIR_CONTROL_FLOW_GRAPH* cfg;
    CFG_ITERATOR cfg_iter;
    VIR_BASIC_BLOCK* bb;
    static gctUINT32 counter = 0;

    if(!VSC_OPTN_InRange(counter, VSC_OPTN_SIMPOptions_GetBeforeFunc(options), VSC_OPTN_SIMPOptions_GetAfterFunc(options)))
    {
        if(VSC_OPTN_SIMPOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
            VIR_LOG(dumper, "Simplification skips function(%d)\n", counter);
            VIR_LOG_FLUSH(dumper);
        }
        counter++;
        return errCode;
    }

    func = VSC_SIMP_Simplification_GetCurrFunc(simp);

    if(VSC_OPTN_SIMPOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification starts for function %s(%d)\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), counter, VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }

    cfg = VIR_Function_GetCFG(func);

    /* dump input cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_INPUT_FUNCTION))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification: input cfg of function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }

    if (VIR_Inst_Count(&func->instList) > 1)
    {
        CFG_ITERATOR_INIT(&cfg_iter, cfg);
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            if(BB_GET_LENGTH(bb) != 0)
            {
                VSC_SIMP_Simplification_SetCurrBB(simp, bb);
                errCode = VSC_SIMP_Simplification_PerformOnBB(simp);
            }

            if(errCode)
            {
                return errCode;
            }
        }
    }

    /* dump output cfg */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_OUTPUT_FUNCTION))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification: output cfg of function %s: \n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_CFG_Dump(dumper, cfg, gcvTRUE);
    }
    if(VSC_OPTN_SIMPOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification ends for function %s(%d)\n%s\n",
            VSC_TRACE_BAR_LINE, VIR_Function_GetNameString(func), counter, VSC_TRACE_BAR_LINE);
        VIR_LOG_FLUSH(dumper);
    }
    counter++;

    return errCode;
}

DEF_QUERY_PASS_PROP(VSC_SIMP_Simplification_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_SIMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;
}

VSC_ErrCode VSC_SIMP_Simplification_PerformOnShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader           *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_OPTN_SIMPOptions  *options = (VSC_OPTN_SIMPOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper           *dumper = pPassWorker->basePassWorker.pDumper;
    VSC_SIMP_Simplification simp;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_SIMPOptions_GetBeforeShader(options), VSC_OPTN_SIMPOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_SIMPOptions_GetTrace(options))
        {
            VIR_LOG(dumper, "Simplification skips shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_SIMPOptions_GetTrace(options))
        {
            VIR_LOG(dumper, "Simplification starts for shader(%d)\n", VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    VSC_SIMP_Simplification_Init(&simp, shader, gcvNULL, gcvNULL, options, dumper);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        VSC_SIMP_Simplification_SetCurrFunc(&simp, func);
        errcode = VSC_SIMP_Simplification_PerformOnFunction(&simp);
        if(errcode)
        {
            break;
        }
    }

    VSC_SIMP_Simplification_Final(&simp);
    if(VSC_OPTN_SIMPOptions_GetTrace(options))
    {
        VIR_LOG(dumper, "Simplification ends for shader(%d)\n", VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Simplification.", shader, gcvTRUE);
    }

    return errcode;
}

