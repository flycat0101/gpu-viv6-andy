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


#include "vir/transform/gc_vsc_vir_simplification.h"

void VSC_SIMP_Simplification_Init(
    IN VSC_SIMP_Simplification* simp,
    IN VIR_Shader* shader,
    IN VIR_Function* currFunc,
    IN VSC_OPTN_SIMPOptions* options,
    IN VIR_Dumper* dumper
    )
{
    VSC_SIMP_Simplification_SetShader(simp, shader);
    VSC_SIMP_Simplification_SetCurrFunc(simp, currFunc);
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
static
gctBOOL _VSC_SIMP_DestSrc0Identical(
    IN VIR_Instruction* inst
    )
{
    VIR_Operand* dest = VIR_Inst_GetDest(inst);
    VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
    if(VIR_Operand_GetOpKind(dest) == VIR_OPND_SYMBOL && VIR_Operand_GetOpKind(src0) == VIR_OPND_SYMBOL)
    {
        VIR_Enable enable = VIR_Operand_GetEnable(dest);
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(src0);
        VIR_Swizzle mappingSwizzle = 0;
        gctUINT i;
        gctBOOL result;

        if (VIR_Enable_Channel_Count(enable) != VIR_Swizzle_Channel_Count(swizzle))
        {
            return gcvFALSE;
        }

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                VIR_Swizzle_SetChannel(mappingSwizzle, i, i);
            }
        }

        if(swizzle != mappingSwizzle)
        {
            return gcvFALSE;
        }

        result = memcmp(VIR_Operand_GetSymbol(dest), VIR_Operand_GetSymbol(src0), sizeof(VIR_Symbol)) == 0;
        return result;
    }
    return gcvFALSE;
}

static
gctBOOL _VSC_SIMP_CanGetConditionResult(
    IN VIR_Instruction* inst
    )
{
    return VIR_Inst_CanGetConditionResult(inst);
}

static
gctBOOL _VSC_SIMP_ConstantConditionAllTrue(
    IN VIR_Instruction* inst
    )
{
    gctBOOL channelResults[VIR_CHANNEL_NUM];

    VIR_Inst_EvaluateConditionResult(inst, channelResults);

    return channelResults[0] == gcvTRUE &&
           channelResults[1] == gcvTRUE &&
           channelResults[2] == gcvTRUE &&
           channelResults[3] == gcvTRUE;
}

static
gctBOOL _VSC_SIMP_ConstantConditionAllFalse(
    IN VIR_Instruction* inst
    )
{
    gctBOOL channelResults[VIR_CHANNEL_NUM];

    VIR_Inst_EvaluateConditionResult(inst, channelResults);

    return channelResults[0] == gcvFALSE &&
           channelResults[1] == gcvFALSE &&
           channelResults[2] == gcvFALSE &&
           channelResults[3] == gcvFALSE;
}

/* _VSC_SIMP_STEPS_SRC_CHECK */
static
gctBOOL _VSC_SIMP_ImmZero(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    return VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE && VIR_Operand_GetImmediateInt(opnd) == 0;
}

static
gctBOOL _VSC_SIMP_ChannelwiseConstOrImmZero(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(!VIR_Operand_ContainsConstantValue(opnd))
    {
        return gcvFALSE;
    }
    else
    {
        VIR_Enable enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
        gctUINT i;

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                gctUINT constValue = VIR_Operand_ExtractOneChannelConstantValue(opnd, VIR_Inst_GetShader(inst), i, gcvNULL);

                if(constValue)
                {
                    return gcvFALSE;
                }
            }
        }
    }

    return gcvTRUE;
}

static
gctBOOL _VSC_SIMP_ChannelwiseConstOrImmOne(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(!VIR_Operand_ContainsConstantValue(opnd))
    {
        return gcvFALSE;
    }
    else
    {
        VIR_Enable enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
        gctUINT i;
        VIR_TypeId typeId;

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                gctUINT constValue = VIR_Operand_ExtractOneChannelConstantValue(opnd, VIR_Inst_GetShader(inst), i, &typeId);

                switch (typeId)
                {
                case VIR_TYPE_FLOAT32:
                    if(constValue != 0x3f800000)
                    {
                        return gcvFALSE;
                    }
                    break;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_INT32:
                    if(constValue != 1)
                    {
                        return gcvFALSE;
                    }
                    break;
                default:
                    return gcvFALSE;
                    break;
                }
            }
        }
    }

    return gcvTRUE;
}

static
gctBOOL _VSC_SIMP_ChannelwiseConstOrImmFFFFFFFF(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(!VIR_Operand_ContainsConstantValue(opnd))
    {
        return gcvFALSE;
    }
    else
    {
        VIR_Enable enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
        gctUINT i;
        VIR_TypeId typeId;

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                gctUINT constValue = VIR_Operand_ExtractOneChannelConstantValue(opnd, VIR_Inst_GetShader(inst), i, &typeId);

                switch (typeId)
                {
                case VIR_TYPE_FLOAT32:
                    gcmASSERT(0);
                    break;
                case VIR_TYPE_UINT32:
                case VIR_TYPE_INT32:
                    if(constValue != 0xffffffff)
                    {
                        return gcvFALSE;
                    }
                    break;
                default:
                    return gcvFALSE;
                    break;
                }
            }
        }
    }

    return gcvTRUE;
}

static
gctBOOL _VSC_SIMP_TypeIs16BitOrLess(
    VIR_TypeId typeId
    )
{
    gctBOOL result = gcvFALSE;
    switch (typeId)
    {
        case VIR_TYPE_INT16:
        case VIR_TYPE_UINT16:
        case VIR_TYPE_INT8:
        case VIR_TYPE_UINT8:
            result = gcvTRUE;
            break;
        default:
            break;
    }
    return result;
}

static
gctBOOL _VSC_SIMP_ChannelwiseTypeIs16BitOrLess(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    VIR_TypeId typeId = VIR_Operand_GetTypeId(opnd);
    if (VIR_TypeId_isPrimitive(typeId) && _VSC_SIMP_TypeIs16BitOrLess(VIR_GetTypeComponentType(typeId)))
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/*same implementation as _VSC_SIMP_ChannelwiseConstOrImmFFFFFFFF*/
static
gctBOOL _VSC_SIMP_ChannelwiseConstOrImmFFFF(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(!VIR_Operand_ContainsConstantValue(opnd))
    {
        return gcvFALSE;
    }

    {
        VIR_Enable enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
        gctUINT i;
        VIR_TypeId typeId;

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                gctUINT constValue = VIR_Operand_ExtractOneChannelConstantValue(opnd, VIR_Inst_GetShader(inst), i, &typeId);

                switch (typeId)
                {
                    case VIR_TYPE_FLOAT32:
                        gcmASSERT(0);
                        break;
                    case VIR_TYPE_UINT32:
                    case VIR_TYPE_INT32:
                    case VIR_TYPE_INT16:
                    case VIR_TYPE_UINT16:
                        if(constValue != 0xffff)
                        {
                            return gcvFALSE;
                        }
                        break;
                    default:
                        return gcvFALSE;
                }
            }
        }
    }
    return gcvTRUE;
}

static
gctBOOL _VSC_SIMP_TypeIs8BitOrLess(
    VIR_TypeId typeId
    )
{
    gctBOOL result = gcvFALSE;
    if (typeId == VIR_TYPE_INT8 || typeId == VIR_TYPE_UINT8)
    {
        result = gcvTRUE;
    }

    return result;
}

static
gctBOOL _VSC_SIMP_ChannelwiseTypeIs8BitOrLess(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    VIR_TypeId typeId = VIR_Operand_GetTypeId(opnd);
    if (VIR_TypeId_isPrimitive(typeId) && _VSC_SIMP_TypeIs8BitOrLess(VIR_GetTypeComponentType(typeId)))
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

/*same implementation as _VSC_SIMP_ChannelwiseConstOrImmFFFFFFFF*/
static
gctBOOL _VSC_SIMP_ChannelwiseConstOrImmFF(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if(!VIR_Operand_ContainsConstantValue(opnd))
    {
        return gcvFALSE;
    }

    {
        VIR_Enable enable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
        gctUINT i;
        VIR_TypeId typeId;

        for(i = 0; i < VIR_CHANNEL_NUM; i++)
        {
            if(enable & (1 << i))
            {
                gctUINT constValue = VIR_Operand_ExtractOneChannelConstantValue(opnd, VIR_Inst_GetShader(inst), i, &typeId);

                switch (typeId)
                {
                    case VIR_TYPE_FLOAT32:
                        gcmASSERT(0);
                        break;
                    case VIR_TYPE_UINT32:
                    case VIR_TYPE_INT32:
                    case VIR_TYPE_INT16:
                    case VIR_TYPE_UINT16:
                    case VIR_TYPE_INT8:
                    case VIR_TYPE_UINT8:
                        if(constValue != 0xff)
                        {
                            return gcvFALSE;
                        }
                        break;
                    default:
                        return gcvFALSE;
                }
            }
        }
    }
    return gcvTRUE;
}

static
gctBOOL _VSC_SIMP_OperandIsInteger(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    VIR_TypeId typeId = VIR_Operand_GetTypeId(opnd);

    return VIR_TypeId_isInteger(typeId);
}

/* now only deal with positive integer */
static
gctBOOL _VSC_SIMP_ImmPowerOf2(
    IN VIR_Instruction* inst,
    IN VIR_Operand* opnd
    )
{
    if ((VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE) &&
        (VIR_TypeId_isInteger(VIR_Operand_GetTypeId(opnd))))
    {
        gctINT imm = VIR_Operand_GetImmediateInt(opnd);
        return (imm > 0) && (!(imm&(imm-1)));
    }
    return gcvFALSE;
}

/* _VSC_SIMP_STEPS_TRANS */
static
void _VSC_SIMP_MOVDestSrc0(
    IN OUT VIR_Instruction* inst
    )
{
    gctUINT i;

    for(i = 1; i < VIR_Inst_GetSrcNum(inst); i++)
    {
        VIR_Inst_FreeSource(inst, i);
    }
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(inst, 1);
}

static
void _VSC_SIMP_MOVDestSrc1(
    IN OUT VIR_Instruction* inst
    )
{
    gctUINT i;
    VIR_Operand* temp = VIR_Inst_GetSource(inst, 0);

    VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 1));
    VIR_Inst_SetSource(inst, 1, temp);

    for(i = 1; i < VIR_Inst_GetSrcNum(inst); i++)
    {
        VIR_Inst_FreeSource(inst, i);
    }

    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(inst, 1);
}

static
void _VSC_SIMP_MOVDestSrc2(
    IN OUT VIR_Instruction* inst
    )
{
    gctUINT i;
    VIR_Operand* temp = VIR_Inst_GetSource(inst, 0);

    VIR_Inst_SetSource(inst, 0, VIR_Inst_GetSource(inst, 2));
    VIR_Inst_SetSource(inst, 2, temp);

    for(i = 1; i < VIR_Inst_GetSrcNum(inst); i++)
    {
        VIR_Inst_FreeSource(inst, i);
    }

    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(inst, 1);
}

static
void _VSC_SIMP_MOVDestZero(
    IN OUT VIR_Instruction* inst
    )
{
    gctUINT i;

    for(i = 1; i < VIR_Inst_GetSrcNum(inst); i++)
    {
        VIR_Inst_FreeSource(inst, i);
    }
    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
    VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
    VIR_Inst_SetSrcNum(inst, 1);
    VIR_Operand_SetImmediateUint(VIR_Inst_GetSource(inst, 0), 0);
    VIR_Operand_SetTypeId(VIR_Inst_GetSource(inst, 0), VIR_Operand_GetTypeId(VIR_Inst_GetDest(inst)));
}

static
void _VSC_SIMP_Change2NOP(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Function_ChangeInstToNop(VIR_Inst_GetFunction(inst), inst);
}

/* now only deal with positive integer */
static
gctINT _VSC_SIMP_LOG2(
    gctINT powerof2
    )
{
    gctINT i = 0;
    while (powerof2 > 1)
    {
        i++;
        powerof2 = powerof2 >> 1;
    }
    return i;
}

static
void _VSC_SIMP_ChangeDIV2RShift(
    IN OUT VIR_Instruction* inst
    )
{
    VIR_Operand *src1 = VIR_Inst_GetSource(inst, 1);
    VIR_Operand *newsrc1;
    gctINT imm, log2value;
    gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);
    imm = VIR_Operand_GetImmediateInt(src1);
    VIR_Function_DupOperand(VIR_Inst_GetFunction(inst), src1, &newsrc1);
    log2value = _VSC_SIMP_LOG2(imm);
    VIR_Operand_SetImmediateInt(newsrc1, log2value);
    VIR_Inst_SetOpcode(inst, VIR_OP_RSHIFT);
    VIR_Inst_SetSource(inst, 1, newsrc1);
}

static
void _VSC_SIMP_ChangeMOD2AndBitwise(
    IN OUT VIR_Instruction* inst
)
{
    VIR_Operand *src1 = VIR_Inst_GetSource(inst, 1);
    VIR_Operand *newsrc1;
    gctINT imm;
    gcmASSERT(VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE);
    imm = VIR_Operand_GetImmediateInt(src1);
    VIR_Function_DupOperand(VIR_Inst_GetFunction(inst), src1, &newsrc1);
    VIR_Operand_SetImmediateInt(newsrc1, imm-1);
    VIR_Inst_SetOpcode(inst, VIR_OP_AND_BITWISE);
    VIR_Inst_SetSource(inst, 1, newsrc1);
}

/* Simplification Steps */

_VSC_SIMP_Steps ADD_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_OperandIsInteger}}, /* sometimes this add is used for denorm */
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_OperandIsInteger}}, /* sometimes this add is used for denorm */
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps AND_BITWISE_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFFFFFFFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFFFFFFFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseTypeIs16BitOrLess}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFFFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseTypeIs16BitOrLess}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFFFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseTypeIs8BitOrLess}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseTypeIs8BitOrLess}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmFF}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps SELECT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllTrue}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllFalse}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps CMOV_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllTrue}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllFalse}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_Change2NOP}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps LSHIFT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps MAD_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps MOV_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_DestSrc0Identical}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_Change2NOP}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps MUL_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmOne}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmOne}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC0_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps RSHIFT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps CSELECT_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllTrue}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc1}},
    {_VSC_SIMP_STEPS_COUNT, {3}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_CanGetConditionResult}},
    {_VSC_SIMP_STEPS_INST_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ConstantConditionAllFalse}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc2}},
    {_VSC_SIMP_STEPS_END, {0}},
};

_VSC_SIMP_Steps SUB_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_DestSrc0Identical}}, /*?*/
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestZero}},
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ChannelwiseConstOrImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_MOVDestSrc0}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps SWIZZLE_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC2_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ImmZero}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_Change2NOP}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps DIV_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ImmPowerOf2}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_ChangeDIV2RShift}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps MOD_Steps[] = {
    {_VSC_SIMP_STEPS_COUNT, {2}},
    {_VSC_SIMP_STEPS_SRC1_CHECK, {(gctUINTPTR_T)_VSC_SIMP_ImmPowerOf2}},
    {_VSC_SIMP_STEPS_TRANS, {(gctUINTPTR_T)_VSC_SIMP_ChangeMOD2AndBitwise}},
    {_VSC_SIMP_STEPS_END, {0}}
};

_VSC_SIMP_Steps* _VSC_SIMP_GetSteps(
    IN VIR_OpCode opc)
{
    switch(opc)
    {
        case VIR_OP_ADD:
            return ADD_Steps;
        case VIR_OP_MUL:
            return MUL_Steps;
        case VIR_OP_AND_BITWISE:
            return AND_BITWISE_Steps;
        case VIR_OP_SELECT:
            return SELECT_Steps;
        case VIR_OP_CMOV:
            return CMOV_Steps;
        case VIR_OP_LSHIFT:
            return LSHIFT_Steps;
        case VIR_OP_MAD:
            return MAD_Steps;
        case VIR_OP_MOV:
            return MOV_Steps;
        case VIR_OP_RSHIFT:
            return RSHIFT_Steps;
        case VIR_OP_CSELECT:
            return CSELECT_Steps;
        case VIR_OP_SWIZZLE:
            return SWIZZLE_Steps;
        case VIR_OP_DIV:
            return DIV_Steps;
        case VIR_OP_MOD:
            return MOD_Steps;
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
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_OpCode opc = VIR_Inst_GetOpcode(inst);
    gctBOOL chg = gcvFALSE;
    VSC_OPTN_SIMPOptions* options = gcvNULL;

    if(simp)
    {
        options = VSC_SIMP_Simplification_GetOptions(simp);
    }

    if(VIR_Inst_CanGetConstantResult(inst))
    {
        VIR_Operand* dest = VIR_Inst_GetDest(inst);
        VIR_TypeId destTypeId = VIR_Operand_GetTypeId(dest);
        VIR_TypeId destCompTypeId = VIR_GetTypeComponentType(destTypeId);
        VIR_Operand* src0 = VIR_Inst_GetSource(inst, 0);
        VIR_Enable enable = VIR_Operand_GetEnable(dest);
        gctUINT constResult[VIR_CHANNEL_NUM];
        gctUINT i;

        if(options && VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION))
        {
            VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
            VIR_LOG(dumper, "before SIMP:\n");
            VIR_Inst_Dump(dumper, inst);
        }

        VIR_Inst_EvaluateConstantResult(inst, constResult);



        if(VIR_OPCODE_isComponentwise(opc))
        {
            VIR_ConstVal constVal;
            gctUINT constChannelCount = 0;
            VIR_Swizzle constSwizzle = VIR_SWIZZLE_XXXX;

            for(i = 0; i < VIR_CHANNEL_NUM; i++)
            {
                if(enable & (1 << i))
                {
                    gctUINT j;

                    for(j = 0; j < constChannelCount; j++)
                    {
                        if(constVal.vecVal.u32Value[j] == constResult[i])
                        {
                            VIR_Swizzle_SetChannel(constSwizzle, i, j);
                            break;
                        }
                    }

                    if(j >= constChannelCount)
                    {
                        constVal.vecVal.u32Value[constChannelCount] = constResult[i];
                        VIR_Swizzle_SetChannel(constSwizzle, i, constChannelCount);
                        constChannelCount++;
                    }
                }
            }

            gcmASSERT(constChannelCount > 0 && constChannelCount <= VIR_CHANNEL_NUM);

            if(constChannelCount == 1)
            {
                gcmASSERT(constSwizzle == VIR_SWIZZLE_XXXX);

                switch(destCompTypeId)
                {
                case VIR_TYPE_FLOAT32:
                    VIR_Operand_SetImmediateFloat(src0, constVal.vecVal.f32Value[0]);
                    break;
                case VIR_TYPE_INT32:
                    VIR_Operand_SetImmediateInt(src0, constVal.vecVal.i32Value[0]);
                    break;
                case VIR_TYPE_UINT32:
                    VIR_Operand_SetImmediateUint(src0, constVal.vecVal.u32Value[0]);
                    break;
                default:
                    gcmASSERT(0);
                }
            }
            else
            {
                if(VIR_Shader_isRAEnabled(VSC_SIMP_Simplification_GetShader(simp)))
                {
                    VIR_ConstId constId;

                    VIR_Shader_AddConstant(VIR_Inst_GetShader(inst), VIR_TypeId_ComposeNonOpaqueType(destCompTypeId, constChannelCount, 1), &constVal, &constId);
                    VIR_Operand_SetConst(src0, destTypeId, constId);
                    VIR_Operand_SetSwizzle(src0, constSwizzle);
                }
                else
                {
                    if(options && VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION))
                    {
                        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
                        VIR_LOG(dumper, "bail out since new RA is not enabled and converting a constant vector to gcsl is too complex.\n");
                        VIR_Inst_Dump(dumper, inst);
                    }

                    if(changed)
                    {
                        *changed = chg;
                    }
                    return errCode;
                }
            }
        }

        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
        VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);
        for(i = 1; i < VIR_Inst_GetSrcNum(inst); i++)
        {
            VIR_Inst_FreeSource(inst, i);
        }
        VIR_Inst_SetSrcNum(inst, 1);
        chg = gcvTRUE;

        if(options && VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_TRANSFORMATION))
        {
            VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
            VIR_LOG(dumper, "after SIMP:\n");
            VIR_Inst_Dump(dumper, inst);
        }
    }
    else
    {
        _VSC_SIMP_Steps* steps = _VSC_SIMP_GetSteps(opc);

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
    }

    if(changed)
    {
        *changed = chg;
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

    /* dump input function */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_INPUT_FUNCTION))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification: input function %s\n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
    }

    /* scan instruction list of function */
    {
        VIR_InstIterator        inst_iter;
        VIR_Instruction        *inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            errCode = VSC_SIMP_Simplification_PerformOnInst(simp, inst, gcvNULL);
            CHECK_ERROR(errCode, "VSC_SIMP_Simplification_PerformOnInst failed");
        }
    }

    /* dump output */
    if(VSC_UTILS_MASK(VSC_OPTN_SIMPOptions_GetTrace(options), VSC_OPTN_SIMPOptions_TRACE_OUTPUT_FUNCTION))
    {
        VIR_Dumper* dumper = VSC_SIMP_Simplification_GetDumper(simp);
        VIR_LOG(dumper, "%s\nSimplification: output function %s: \n%s\n",
            VSC_TRACE_STAR_LINE, VIR_Function_GetNameString(func), VSC_TRACE_STAR_LINE);
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
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
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML |
                                 VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_SIMP;

    /*  destroy data flow info if instruction is replaced by NOP
     *  TODO:  only set destroy info if _VSC_SIMP_Change2NOP is called
     */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
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

    VSC_SIMP_Simplification_Init(&simp, shader, gcvNULL, options, dumper);

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

