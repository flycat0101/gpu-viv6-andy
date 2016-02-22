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


#include "vir/lower/gc_vsc_vir_pattern.h"

#define VIR_DEBUG_PATTERN 0
#define VIR_Covers(bits1, bits2)       \
    ((((bits1) ^ (bits2)) | (bits1)) == (bits1))

#if VIR_DEBUG_PATTERN
static void
_Pattern_Dump_Replaced(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst,
    IN VIR_Pattern          *Pattern
    )
{
    VIR_Dumper *dumper     = Context->shader->dumper;
    gctSIZE_T   i  = 0;

    for (i = 0; i < Pattern->repalceCount; ++i)
    {
        Inst = VIR_Inst_GetPrev(Inst);
    }

    VIR_LOG(dumper, "After pattern:");
    VIR_LOG_FLUSH(dumper);
    for (i = 0; i < Pattern->repalceCount; ++i)
    {
        VIR_Inst_Dump(dumper, Inst);
        VIR_LOG_FLUSH(dumper);
        Inst = VIR_Inst_GetNext(Inst);
    }

    VIR_LOG_FLUSH(dumper);
}

static void
_Pattern_Dump_Pattern(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst,
    IN VIR_Pattern          *Pattern
    )
{
    VIR_Dumper *dumper     = Context->shader->dumper;
    gctSIZE_T   i  = 0;

    VIR_LOG(dumper, "Before pattern:");
    VIR_LOG_FLUSH(dumper);
    for (i = 0; i < Pattern->matchCount; ++i)
    {
        VIR_Inst_Dump(dumper, Inst);
        VIR_LOG_FLUSH(dumper);
        Inst = VIR_Inst_GetNext(Inst);
    }
}
#endif

VSC_ErrCode
VIR_PatternContext_Initialize(
    IN OUT VIR_PatternContext      *Context,
    IN VIR_Shader                  *Shader,
    IN VIR_PatnContextFlag          Flag,
    IN VIR_PATTERN_GET_PATTERN_PTR  GetPatrernPtr,
    IN VIR_PATTERN_CMP_OPCODE_PTR   CmpOpcodePtr,
    IN gctSIZE_T                    Size
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    Context->baseTmpRegNo = 0;
    Context->maxTmpRegNo  = 0;
    Context->changed      = 0;
    Context->shader       = Shader;
    Context->flag         = Flag;
    Context->getPattern   = GetPatrernPtr;
    Context->cmpOpcode    = CmpOpcodePtr;

    memset(Context->tmpRegSymbol, 0, sizeof(Context->tmpRegSymbol));

    if (Size > 0)
    {
        vscPMP_Intialize(&Context->memPool, gcvNULL, Size, sizeof(char), gcvTRUE/*pooling*/);
    }

    return errCode;
}

VSC_ErrCode
VIR_PatternContext_Finalize(
    IN OUT VIR_PatternContext      *Context,
    IN gctSIZE_T                    Size
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    if (Size > 0)
    {
        vscPMP_Finalize(&Context->memPool);
    }
    return errCode;
}

static void
_Pattern_SetOperand(
    IN VIR_Instruction *Inst,
    IN gctINT           No,
    IN VIR_Operand     *Opnd
    )
{
    VIR_OperandId index;
    gcmASSERT(Inst != gcvNULL && Opnd != gcvNULL);

    if (No == 0)
    {
        index = VIR_Operand_GetIndex(Inst->dest);
        *Inst->dest = *Opnd;
        VIR_Operand_SetIndex(Inst->dest, index);

        if (!VIR_Operand_isLvalue(Opnd))
        {
            VIR_Operand_SetLvalue(Inst->dest, gcvTRUE);
            VIR_Operand_SetEnable(Inst->dest,
                VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd)));
            VIR_Operand_SetRoundMode(Inst->dest, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(Inst->dest, VIR_MOD_NONE);
        }
    }
    else
    {
        gcmASSERT(No - 1 < VIR_MAX_SRC_NUM);

        /* copy operand */
        index = VIR_Operand_GetIndex(Inst->src[No - 1]);
        *Inst->src[No - 1] = *Opnd;
        VIR_Operand_SetIndex(Inst->src[No - 1], index);

        if (VIR_Operand_isLvalue(Opnd))
        {
            /* the operand is copied from pervious inst's dest,
               reset its lvalue attribute */
            VIR_Operand_SetLvalue(Inst->src[No - 1], gcvFALSE);
            /* convert dest operand's enable to new operand's swizzzle */
            VIR_Operand_SetSwizzle(Inst->src[No - 1],
                VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd)));
            VIR_Operand_SetRoundMode(Inst->src[No - 1], VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(Inst->src[No - 1], VIR_MOD_NONE);
        }
    }
}

static VIR_Operand*
_Pattern_GetOperand(
    IN VIR_Instruction *Inst,
    IN gctSIZE_T        No
    )
{
    if (Inst == gcvNULL)
    {
        return gcvNULL;
    }

    if (No == 0)
    {
        return Inst->dest;
    }
    else
    {
        gcmASSERT(No - 1 < VIR_MAX_SRC_NUM);
        return Inst->src[No - 1];
    }
}


static VIR_Operand*
_Pattern_GetTempOperandByPattern(
    IN VIR_PatternReplaceInst  *Pattern,
    IN gctINT                 Num,
    IN VIR_Instruction       *Inst
    )
{
    gcmASSERT(Inst != gcvNULL &&
        Pattern != gcvNULL);

    Inst = VIR_Inst_GetPrev(Inst);
    --Pattern;

    while(Pattern != gcvNULL)
    {
        gctSIZE_T i = 0;
        gcmASSERT(Inst != gcvNULL);
        if (Pattern->opnd[0] == Num)
        {
            return _Pattern_GetOperand(Inst, i);
        }

        Inst = VIR_Inst_GetPrev(Inst);
        --Pattern;
    }

    gcmASSERT(0);
    return gcvNULL;
}

static VIR_Operand*
_Pattern_GetOperandByPattern(
    IN VIR_PatternMatchInst  *Pattern,
    IN gctINT            Num,
    IN VIR_Instruction  *Inst
    )
{
    while(Pattern != gcvNULL)
    {
        gctSIZE_T               i = 0;
        VIR_Operand_Iterator    opndIter;
        VIR_Operand            *opnd;

        if (Inst == gcvNULL)
        {
            return gcvNULL;
        }

        VIR_Operand_Iterator_Init(Inst, &opndIter, gcvFALSE);

        for (i = 0, opnd = VIR_Operand_Iterator_First(&opndIter);
            i < VIR_PATTERN_OPND_COUNT && opnd != gcvNULL;
            ++i, opnd = VIR_Operand_Iterator_Next(&opndIter))
        {
            if (Pattern->opnd[i] == Num)
            {
                return opnd;
            }
        }

        Inst = VIR_Inst_GetNext(Inst);
        ++Pattern;
    }

    gcmASSERT(0);
    return gcvNULL;
}

static gctBOOL
_Pattern_CmpOperand(
    IN VIR_Shader   *Shader,
    IN VIR_Operand  *Opnd0,
    IN VIR_Operand  *Opnd1
    )
{

    if (Opnd0 == Opnd1)
    {
        return gcvTRUE;
    }

    if (Opnd0 == gcvNULL || Opnd1 == gcvNULL)
    {
        return gcvFALSE;
    }

    if (VIR_Operand_GetOpKind(Opnd0) != VIR_Operand_GetOpKind(Opnd1))
    {
        return gcvFALSE;
    }

    switch (VIR_Operand_GetOpKind(Opnd0))
    {
    case VIR_OPND_NONE:
    case VIR_OPND_UNDEF:
    case VIR_OPND_UNUSED:
        return gcvTRUE;
    case VIR_OPND_IMMEDIATE:
        {
            VIR_TypeId ty0 = VIR_Operand_GetType(Opnd0);

            VIR_TypeId ty1 = VIR_Operand_GetType(Opnd1);

            if (ty0 != ty1)
            {
                return gcvFALSE;
            }

            return Opnd0->u1.uConst == Opnd1->u1.uConst;
        }
    case VIR_OPND_SYMBOL:
    case VIR_OPND_SAMPLER_INDEXING:
        {
            VIR_TypeId  ty0   = VIR_Operand_GetType(Opnd0);
            VIR_Symbol *sym0  = VIR_Operand_GetSymbol(Opnd0);
            VIR_Modifier mod0 = VIR_Operand_GetModifier(Opnd0);
            VIR_RoundMode rnd0 = VIR_Operand_GetRoundMode(Opnd0);

            VIR_TypeId  ty1   = VIR_Operand_GetType(Opnd1);
            VIR_Symbol *sym1  = VIR_Operand_GetSymbol(Opnd1);
            VIR_Modifier mod1 = VIR_Operand_GetModifier(Opnd1);
            VIR_RoundMode rnd1 = VIR_Operand_GetRoundMode(Opnd1);

            if (ty0 < VIR_TYPE_LAST_PRIMITIVETYPE &&
                ty1 < VIR_TYPE_LAST_PRIMITIVETYPE)
            {
                ty0 = VIR_GetTypeComponentType(ty0);
                ty1 = VIR_GetTypeComponentType(ty1);

                if (ty0 != ty1)
                {
                    return gcvFALSE;
                }
            }
            else
            {
                if (ty0 != ty1)
                {
                    return gcvFALSE;
                }
            }

            if (VIR_Operand_isLvalue(Opnd0) == VIR_Operand_isLvalue(Opnd1))
            {
                if (!VIR_Operand_isLvalue(Opnd0))
                {
                    VIR_Swizzle swiz0 = VIR_Operand_GetSwizzle(Opnd0);
                    VIR_Swizzle swiz1 = VIR_Operand_GetSwizzle(Opnd1);
                    if (swiz0 != swiz1)
                    {
                        return gcvFALSE;
                    }
                }
                else
                {
                    VIR_Enable enable0 = VIR_Operand_GetEnable(Opnd0);
                    VIR_Enable enable1 = VIR_Operand_GetEnable(Opnd1);
                    if (enable0 != enable1)
                    {
                        return gcvFALSE;
                    }
                }
            }
            else
            {
                VIR_Swizzle swiz = VIR_Operand_isLvalue(Opnd0) ? VIR_Operand_GetSwizzle(Opnd1)
                    : VIR_Operand_GetSwizzle(Opnd0);

                VIR_Enable  enable = VIR_Operand_isLvalue(Opnd0) ? VIR_Operand_GetEnable(Opnd0)
                    : VIR_Operand_GetEnable(Opnd1);

                gctSIZE_T i;

                for (i = 0; i < VIR_CHANNEL_COUNT; i++)
                {
                    if (enable & (1 << i))
                    {
                        if ((VIR_Swizzle)i != VIR_Swizzle_GetChannel(swiz, i))
                        {
                            return gcvFALSE;
                        }
                    }
                }
            }

            if (rnd0 != rnd1)
            {
                return gcvFALSE;
            }

            if (VIR_Operand_isLvalue(Opnd0) != VIR_Operand_isLvalue(Opnd1))
            {
                if (mod0 != VIR_MOD_NONE || mod1 != VIR_MOD_NONE)
                {
                    return gcvFALSE;
                }
            }

            if (mod0 != mod1)
            {
                return gcvFALSE;
            }

            return sym0 == sym1;
        }
    case VIR_OPND_CONST:
    case VIR_OPND_PARAMETERS:
    case VIR_OPND_FUNCTION:
    case VIR_OPND_LABEL:
    case VIR_OPND_INTRINSIC:
    case VIR_OPND_FIELD:
    case VIR_OPND_ARRAY:
    case VIR_OPND_TEXLDPARM:
    default:
        return gcvFALSE;
    }
}

static VIR_Instruction *
_Pattern_GetInstruction(
    IN VIR_Instruction  *Inst,
    IN gctINT            Num
    )
{
    gcmASSERT(Num >= 0);
    while(Num--)
    {
        Inst = VIR_Inst_GetNext(Inst);
    }
    return Inst;
}


static VIR_Symbol*
_GetNewTempRegister(
    IN VIR_PatternContext *Context
    )
{
    VSC_ErrCode    errCode      = VSC_ERR_NONE;
    VIR_VirRegId   virRegId     = VIR_Shader_NewVirRegId(Context->shader, 1);
    VIR_SymId      virSymId;
    errCode = VIR_Shader_AddSymbol(Context->shader,
        VIR_SYM_VIRREG,
        virRegId,
        VIR_Shader_GetTypeFromId(Context->shader, VIR_TYPE_UNKNOWN),
        VIR_STORAGE_UNKNOWN,
        &virSymId);

    if (errCode == VSC_ERR_NONE) {
        return VIR_Shader_GetSymFromId(Context->shader, virSymId);
    }

    return gcvNULL;
}

static void
_Pattern_SetTypeByComponents(
    IN VIR_PatternContext *Context,
    IN VIR_Operand        *Operand,
    IN gctUINT             Component
    )
{
    VIR_Type            *ty    = VIR_Shader_GetTypeFromId(Context->shader, VIR_Operand_GetType(Operand));
    VIR_PrimitiveTypeId  baseType;
    VIR_PrimitiveTypeId  dstTy = VIR_TYPE_UNKNOWN;

    gcmASSERT(VIR_Type_isPrimitive(ty));

    baseType = VIR_Type_GetBaseTypeId(ty);

    if (baseType != VIR_TYPE_UNKNOWN)
    {
        dstTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(baseType), Component, 1);
    }

    VIR_Operand_SetType(Operand, dstTy);
}

static gctBOOL
_Pattern_isMatched(
    IN VIR_PatternContext   *Context,
    IN VIR_Instruction      *Inst,
    IN OUT VIR_Pattern      **Pattern
    )
{

    VIR_Pattern      *curPattern = *Pattern;
    VIR_Instruction  *curInst    =  gcvNULL;

    while(curPattern->matchCount != 0)
    {
        gctSIZE_T i         = 0;
        gctBOOL   isMatched = gcvTRUE;

        curInst     = Inst;

        for (i = 0; i < curPattern->matchCount; ++i, curInst = VIR_Inst_GetNext(curInst))
        {
            gctSIZE_T              j            = 0;
            VIR_PatternMatchInst  *patternInst  = curPattern->matchInsts + i;
            VIR_Operand_Iterator   opndIter;
            VIR_Operand           *curOpnd      = gcvNULL;

            gcmASSERT(Context->cmpOpcode != gcvNULL);

            if (curInst == gcvNULL)
            {
                isMatched = gcvFALSE;
                break;
            }

            isMatched &= Context->cmpOpcode(Context, patternInst, curInst);

            if (isMatched && patternInst->condOp != VIR_PATTERN_ANYCOND)
            {
                isMatched &= ((gctINT)(patternInst->condOp) == (gctINT)VIR_Inst_GetConditionOp(curInst));
            }

            if (!isMatched)   break;

            if (patternInst->flag == VIR_PATN_MATCH_FLAG_OR)
            {
                gctBOOL anyTrue = gcvFALSE;
                gctUINT anyRun  = gcvFALSE;

                for (j = 0; j < VIR_PATTERN_OPND_COUNT; ++j)
                {
                    if (patternInst->function[j] != gcvNULL)
                    {
                        anyTrue |= patternInst->function[j](Context, curInst);
                        anyRun   = gcvTRUE;

                        if (anyTrue) break;
                    }
                }

                isMatched = anyRun ? anyTrue : gcvTRUE;
            }
            else if (patternInst->flag == VIR_PATN_MATCH_FLAG_AND)
            {
                for (j = 0; j < VIR_PATTERN_OPND_COUNT; ++j)
                {
                    if (patternInst->function[j] != gcvNULL)
                    {
                        isMatched &= patternInst->function[j](Context, curInst);

                        if (!isMatched) break;
                    }
                }
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            if (!isMatched)   break;


            VIR_Operand_Iterator_Init(curInst, &opndIter, gcvFALSE);

            for (j = 0, curOpnd = VIR_Operand_Iterator_First(&opndIter);
                j < VIR_PATTERN_OPND_COUNT;
                ++j, curOpnd = VIR_Operand_Iterator_Next(&opndIter))
            {
                gctINT ptnOpnd = patternInst->opnd[j];

                VIR_Operand *orgOpnd = gcvNULL;

                if (ptnOpnd == 0 &&
                    (curOpnd == gcvNULL || VIR_Operand_GetOpKind(curOpnd) == VIR_OPND_UNDEF))
                {
                    break;
                }

                if (ptnOpnd == 0 || curOpnd == gcvNULL)
                {
                    isMatched = gcvFALSE;
                }

                if (!isMatched)   break;

                orgOpnd = _Pattern_GetOperandByPattern(curPattern->matchInsts, ptnOpnd, Inst);

                isMatched &= _Pattern_CmpOperand(Context->shader, curOpnd, orgOpnd);
                if (!isMatched)   break;
            }

            if (!isMatched)       break;
        }

        if (isMatched) break;
        ++curPattern;
    }

    *Pattern = curPattern;

    if (curPattern->matchCount == 0)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VSC_ErrCode
_Pattern_FreeMatchedInsts(
    IN VIR_PatternContext   *Context,
    IN VIR_Function         *Function,
    IN OUT VIR_Instruction **Inst,
    IN VIR_Pattern          *Pattern
    )
{
    gctSIZE_T         i        = 0;

    VSC_ErrCode       errCode  = VSC_ERR_NONE;
    VIR_Instruction  *curInst  = *Inst;
    VIR_Instruction  *nxtInst  = gcvNULL;

    gcmASSERT(*Inst != gcvNULL);
    *Inst = VIR_Inst_GetPrev(curInst);
    for (i = 0; i < Pattern->matchCount; ++i)
    {
        gcmASSERT(curInst != gcvNULL);

        if (!(Pattern->flags & VIR_PATN_FLAG_DELETE_MANUAL))
        {
            gctSIZE_T j = 0;
            for (j = 0; j < VIR_PATTERN_OPND_COUNT; ++j)
            {
                VIR_Operand *opnd = _Pattern_GetOperand(curInst, j);
                if (opnd != gcvNULL)
                {
                    VIR_Function_FreeOperand(Function, opnd);
                }
            }
        }

        nxtInst = VIR_Inst_GetNext(curInst);
        VIR_Function_RemoveInstruction(Function, curInst);
        curInst = nxtInst;
    }


    return errCode;
}

static VSC_ErrCode
_Pattern_ReplaceNormal(
    IN VIR_PatternContext *Context,
    IN VIR_Function       *Function,
    IN VIR_Instruction    *Inst,
    IN VIR_Pattern        *Pattern,
    IN gctINT              Component
    )
{
    gctSIZE_T               i = 0;
    VIR_PatternReplaceInst *replacedPtnInst = gcvNULL;
    VSC_ErrCode             errCode         = VSC_ERR_NONE;

#if VIR_DEBUG_PATTERN
        _Pattern_Dump_Pattern(Context, Inst, Pattern);
#endif

    for (i = 0; i < Pattern->repalceCount; ++i)
    {
        VIR_Instruction  *insertedInst = gcvNULL;
        gctINT            j = 0;

        replacedPtnInst  = Pattern->replaceInsts + i;

        errCode = VIR_Function_AddInstructionBefore(Function,
            replacedPtnInst->opcode,
            VIR_TYPE_UNKNOWN,
            Inst,
            &insertedInst);
        if (errCode != VSC_ERR_NONE) return errCode;

        if (VIR_Inst_GetOpcode(insertedInst) == VIR_OP_LABEL)
        {
            VIR_LabelId labelID;
            VIR_Label *virLabel;

            errCode = VIR_Function_AddLabel(Function, gcvNULL, &labelID);
            virLabel = VIR_GetLabelFromId(Function, labelID);
            virLabel->defined = insertedInst;
            VIR_Operand_SetLabel(insertedInst->dest, virLabel);
        }

        if (Inst->_parentUseBB)
        {
            VIR_Inst_SetBasicBlock(insertedInst, VIR_Inst_GetBasicBlock(Inst));
        }
        else
        {
            VIR_Inst_SetFunction(insertedInst, VIR_Inst_GetFunction(Inst));
        }

        for (j = VIR_PATTERN_OPND_COUNT - 1; j >= 0; --j)
        {
            gctINT       ptnOpnd = replacedPtnInst->opnd[j];
            VIR_Operand *opnd    = gcvNULL;

            if (ptnOpnd == 0)
            {
                /* newly generated sources need to be generated first */
                if (j > 0 && replacedPtnInst->function[j] != gcvNULL)
                {
                    replacedPtnInst->function[j](Context, insertedInst, _Pattern_GetOperand(insertedInst, j));
                }
                continue;
            }
            /* Temp register */
            else if (ptnOpnd < 0)
            {
                VIR_Symbol *sym       = gcvNULL;
                gctINT      regNo     = -(1 + ptnOpnd);
                gcmASSERT(regNo < VIR_PATTERN_TEMP_COUNT);

                if (Context->flag & VIR_PATN_CONTEXT_FLAG_RECURSION)
                {
                    regNo += Context->baseTmpRegNo;

                    gcmASSERT(regNo < VIR_PATTERN_TEMP_COUNT);

                    if (regNo > Context->maxTmpRegNo)
                    {
                        Context->maxTmpRegNo = regNo;
                    }
                }

                if (j == 0)
                {
                    if (Context->tmpRegSymbol[regNo] == gcvNULL)
                    {
                        VIR_Precision expectedPrecision = VIR_Inst_GetExpectedResultPrecision(insertedInst);
                        Context->tmpRegSymbol[regNo] = _GetNewTempRegister(Context);
                        VIR_Symbol_SetPrecision(Context->tmpRegSymbol[regNo], expectedPrecision);
                    }

                    sym  = Context->tmpRegSymbol[regNo];
                    opnd = _Pattern_GetOperand(insertedInst, j);

                    VIR_Operand_SetLvalue(opnd, gcvTRUE);
                    VIR_Operand_SetSym(opnd, sym);
                    VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
                    VIR_Operand_SetEnable(opnd, VIR_ENABLE_XYZW);
                    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(sym));

                    /* to-do: make the pattern more flexible to set the dst's type
                       based on the flag. Currently, there is only a simple flag
                       VIR_PATN_FLAG_SET_TEMP_IN_FUNC to set the type/enable in the
                       pattern func.*/
                    if (!(Pattern->flags & VIR_PATN_FLAG_SET_TEMP_IN_FUNC))
                    {
                        VIR_Operand     *templeteOpnd = gcvNULL;

                        gcmASSERT(Pattern->matchCount >= 1);

                        if (VIR_PATTERN_DefaultTempType(replacedPtnInst))
                        {
                            VIR_Instruction *lastPatternInst = _Pattern_GetInstruction(Inst, Pattern->matchCount - 1);
                            templeteOpnd = lastPatternInst->dest;
                        }
                        else if (VIR_PATTERN_SpecialTempType(replacedPtnInst))
                        {
                            if (VIR_Covers(VIR_PATTERN_TEMP_TYPE_XYZW, VIR_PATTERN_GetTempType(replacedPtnInst)))
                            {
                                VIR_Instruction *lastPatternInst = _Pattern_GetInstruction(Inst, Pattern->matchCount - 1);
                                templeteOpnd = lastPatternInst->dest;
                            }
                            else
                            {
                                gcmASSERT(0);
                            }
                        }
                        else
                        {
                            templeteOpnd = _Pattern_GetOperandByPattern(Pattern->matchInsts, VIR_PATTERN_GetReferNo(replacedPtnInst), Inst);
                        }

                        if (templeteOpnd != gcvNULL)
                        {
                            VIR_Operand_SetType(opnd, VIR_Operand_GetType(templeteOpnd));
                            VIR_Symbol_SetType(sym, VIR_Shader_GetTypeFromId(Context->shader, VIR_Operand_GetType(templeteOpnd)));
                            if (VIR_Operand_isLvalue(templeteOpnd))
                            {
                                VIR_Operand_SetEnable(opnd, VIR_Operand_GetEnable(templeteOpnd));
                            }
                            else
                            {
                                VIR_Operand_SetEnable(opnd, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(templeteOpnd)));
                            }
                            VIR_Operand_SetRoundMode(opnd, VIR_ROUND_DEFAULT);
                            VIR_Operand_SetModifier(opnd, VIR_MOD_NONE);

                            if (VIR_PATTERN_SpecialTempType(replacedPtnInst) &&
                                VIR_Covers(VIR_PATTERN_TEMP_TYPE_XYZW, VIR_PATTERN_GetTempType(replacedPtnInst)))
                            {
                                VIR_Enable dstEnable = (VIR_Enable)(VIR_PATTERN_GetTempType(replacedPtnInst) & VIR_ENABLE_XYZW);
                                _Pattern_SetTypeByComponents(Context, opnd, vscFindPopulation(dstEnable));
                                VIR_Operand_SetEnable(opnd, dstEnable);
                            }
                        }
                        else
                        {
                            gcmASSERT(0);
                        }
                    }
                }
                else
                {
                    gcmASSERT(Context->tmpRegSymbol[regNo] != gcvNULL);
                    opnd = _Pattern_GetTempOperandByPattern(replacedPtnInst, ptnOpnd, insertedInst);
                }
            }
            else
            {
                opnd = _Pattern_GetOperandByPattern(Pattern->matchInsts, ptnOpnd, Inst);
            }

            _Pattern_SetOperand(insertedInst, j, opnd);

            if (Pattern->flags & VIR_PATN_FLAG_EXPAND)
            {
                if (Pattern->flags & VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O)
                {
                    opnd = _Pattern_GetOperand(insertedInst, j);
                    if (VIR_Operand_isLvalue(opnd))
                    {
                        VIR_Operand_SetEnable(opnd, (VIR_Enable)(0x1 << Component));
                    }
                    else
                    {
                        VIR_Swizzle element = (VIR_Swizzle)((VIR_Operand_GetSwizzle(opnd) >> (Component * 2)) & 0x3);
                        VIR_Swizzle newSwizzle = (VIR_Swizzle)(element | (element << 2) | (element << 4) | (element << 6));
                        VIR_Operand_SetSwizzle(opnd, newSwizzle);
                    }
                    _Pattern_SetTypeByComponents(Context, opnd, 1);
                }
                else
                {
                    gcmASSERT(0);
                }
            }
        }

        if (replacedPtnInst->condOp >= 0)
        {
            VIR_Inst_SetConditionOp(insertedInst, replacedPtnInst->condOp);
        }
        else
        {
            VIR_Inst_SetConditionOp(insertedInst,
                VIR_Inst_GetConditionOp(_Pattern_GetInstruction(Inst, -replacedPtnInst->condOp - 1)));
        }

        for (j = 0; j < VIR_PATTERN_OPND_COUNT; ++j)
        {
            if (replacedPtnInst->function[j] != gcvNULL)
            {
                replacedPtnInst->function[j](Context, insertedInst, _Pattern_GetOperand(insertedInst, j));
            }
        }
    }

#if VIR_DEBUG_PATTERN
        _Pattern_Dump_Replaced(Context, Inst, Pattern);
#endif

    return errCode;
}

static VIR_Pattern *
_Pattern_ClonePattern(
    IN VIR_PatternContext   *Context,
    IN VIR_Pattern          *Pattern
    )
{
    VIR_Pattern *clonedPattern = gcvNULL;

    clonedPattern = (VIR_Pattern *)vscMM_Alloc(&Context->memPool.mmWrapper, sizeof(VIR_Pattern));
    memcpy(clonedPattern, Pattern, sizeof(VIR_Pattern));

    clonedPattern->matchInsts = (VIR_PatternMatchInst *)vscMM_Alloc(&Context->memPool.mmWrapper, sizeof(VIR_PatternMatchInst));
    memcpy(clonedPattern->matchInsts, Pattern->matchInsts, sizeof(VIR_PatternMatchInst) * Pattern->matchCount);

    clonedPattern->replaceInsts = (VIR_PatternReplaceInst *)vscMM_Alloc(&Context->memPool.mmWrapper, sizeof(VIR_PatternReplaceInst));
    memcpy(clonedPattern->replaceInsts, Pattern->replaceInsts, sizeof(VIR_PatternReplaceInst) * Pattern->repalceCount);

    return clonedPattern;
}

static void
_Pattern_DestroyClonedPattern(
    IN VIR_PatternContext   *Context,
    IN VIR_Pattern          *Pattern
    )
{
    vscMM_Free(&Context->memPool.mmWrapper, Pattern->matchInsts);
    vscMM_Free(&Context->memPool.mmWrapper, Pattern->replaceInsts);
    vscMM_Free(&Context->memPool.mmWrapper, Pattern);
}

static gctBOOL
_Pattern_NotExpand(
    IN VIR_Shader      *Shader,
    IN VIR_Instruction *Inst
    )
{
    gctSIZE_T i = 0;
    gctBOOL   shouldExpand = gcvFALSE;

    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); ++i)
    {
        VIR_OperandInfo opndInfo;
        VIR_Swizzle     swiz;
        VIR_Operand_GetOperandInfo(Inst, Inst->src[i], &opndInfo);

        if (opndInfo.isImmVal)
        {
            continue;
        }

        if (opndInfo.isVecConst)
        {
            return gcvFALSE;
        }

        swiz = VIR_Operand_GetSwizzle(Inst->src[i]);

        if (!(swiz == VIR_SWIZZLE_XXXX ||
           swiz == VIR_SWIZZLE_YYYY ||
           swiz == VIR_SWIZZLE_ZZZZ ||
           swiz == VIR_SWIZZLE_WWWW))
        {
            shouldExpand = gcvTRUE;
            break;
        }
    }

    return !shouldExpand;
}

static gctBOOL
_Pattern_NotAddMov(
    IN VIR_Instruction    *Inst
    )
{
    gctBOOL     notSame = gcvTRUE;
    gctSIZE_T   i       = 0;


    gcmASSERT(Inst != gcvNULL &&
        Inst->dest != gcvNULL);

    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        if (VIR_Operand_SameSymbol(Inst->dest, Inst->src[i]))
        {
            notSame = gcvFALSE;
            break;
        }
    }

    if (notSame)
    {
        return gcvTRUE;
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        gctSIZE_T   j       = 0;
        VIR_Enable  enable  = VIR_Operand_GetEnable(Inst->dest);
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(Inst->src[i]);

        for (j = 0; j < VIR_CHANNEL_COUNT; j++)
        {
            VIR_Swizzle channel = VIR_Swizzle_GetChannel(swizzle, j);
            if (enable & (1 << channel))
            {
                return gcvFALSE;
            }
        }
    }

    return gcvTRUE;
}


static VSC_ErrCode
_Pattern_ReplaceInline(
    IN VIR_PatternContext *Context,
    IN VIR_Function       *Function,
    IN VIR_Instruction    *Inst,
    IN VIR_Pattern        *Pattern
    )
{
    VSC_ErrCode     errCode             = VSC_ERR_NONE;
    gctUINT         component           = 1;

    VIR_Instruction *lastReplacedInst   = gcvNULL;
    VIR_Instruction *movTemp2Dest       = gcvNULL;
    gctBOOL          notAddMov          = gcvTRUE;
    gctSIZE_T        i                  = 0;
    VIR_Enable       expandEnable       = VIR_ENABLE_NONE;

    gcmASSERT(Inst->dest != gcvNULL &&
        Pattern->matchCount == 1);

    if (_Pattern_NotExpand(Context->shader, Inst))
    {
        /*
        VIR_Dumper *dumper     = Context->shader->dumper;
        VIR_LOG(dumper, "_Pattern_NotExpand:");
        VIR_LOG_FLUSH(dumper);
        */

        Pattern = _Pattern_ClonePattern(Context, Pattern);
        Pattern->flags = (VIR_PatnFlag)(Pattern->flags ^ VIR_PATN_FLAG_EXPAND_MODE_COMPONENT_O2O);
        Pattern->flags = (VIR_PatnFlag)(Pattern->flags ^ VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE);

        errCode = _Pattern_ReplaceNormal(Context, Function, Inst, Pattern, 0);

        _Pattern_DestroyClonedPattern(Context, Pattern);
        return errCode;
    }

    notAddMov = _Pattern_NotAddMov(Inst);

    if (!notAddMov)
    {
        /*
        VIR_Dumper *dumper     = Context->shader->dumper;
        VIR_LOG(dumper, "_Pattern_ClonePattern:");
        VIR_LOG_FLUSH(dumper);
        */

        Pattern = _Pattern_ClonePattern(Context, Pattern);
        Pattern->replaceInsts[Pattern->repalceCount - 1].opnd[0] = -(VIR_PATTERN_TEMP_COUNT - 1);
    }

    if (Pattern->flags & VIR_PATN_FLAG_EXPAND_COMP_INLINE_NOT_BY_DST)
    {
        expandEnable = VIR_ENABLE_XYZW;
    }
    else
    {
        expandEnable = VIR_Operand_GetEnable(Inst->dest);
    }

    for (component = 0x1, i = 0; i < 4; component <<= 1, ++i)
    {
        gcmASSERT(Inst->dest != gcvNULL &&
            VIR_Operand_isLvalue(Inst->dest));

        if (expandEnable & component)
        {
            errCode = _Pattern_ReplaceNormal(Context, Function, Inst, Pattern, i);
            if (errCode != VSC_ERR_NONE) break;
        }
    }

    if (Pattern->replaceInsts[Pattern->repalceCount - 1].opnd[0] < 0)
    {
        gcmASSERT(Pattern->matchCount >= 1);
        lastReplacedInst = VIR_Inst_GetPrev(Inst);

        errCode = VIR_Function_AddInstructionBefore(Function,
            VIR_OP_MOV,
            VIR_TYPE_UNKNOWN,
            Inst,
            &movTemp2Dest);
        if (errCode != VSC_ERR_NONE) return errCode;

        _Pattern_SetOperand(movTemp2Dest, 0, Inst->dest);
        _Pattern_SetOperand(movTemp2Dest, 1, lastReplacedInst->dest);

        VIR_Operand_SetType(movTemp2Dest->src[0], VIR_Operand_GetType(Inst->dest));
        VIR_Operand_SetSwizzle(movTemp2Dest->src[0],
            VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Inst->dest)));
    }

    if (!notAddMov)
    {
        _Pattern_DestroyClonedPattern(Context, Pattern);
    }

    return errCode;
}

static VSC_ErrCode
_Pattern_Replace(
    IN VIR_PatternContext *Context,
    IN VIR_Function       *Function,
    IN VIR_Instruction    *Inst,
    IN VIR_Pattern        *Pattern
    )
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;

    if (Pattern->flags & VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE)
    {
        errCode = _Pattern_ReplaceInline(Context, Function, Inst, Pattern);
    }
    else if (!(Pattern->flags & VIR_PATN_FLAG_EXPAND))
    {
        errCode = _Pattern_ReplaceNormal(Context, Function, Inst, Pattern, 0);
    }
    else
    {
        gcmASSERT(0);
    }

    return errCode;
}

static VSC_ErrCode
_Pattern_TransformByInstruction(
    IN VIR_PatternContext   *Context,
    IN VIR_Function         *Function,
    IN OUT VIR_Instruction  **Inst
    )
{
    VSC_ErrCode       errCode  = VSC_ERR_NONE;
    VIR_Pattern      *patterns = Context->getPattern(Context, *Inst);
    VIR_Instruction **preInst  = gcvNULL;

    if (patterns == gcvNULL ||
        _Pattern_isMatched(Context, *Inst, &patterns) == gcvFALSE)
    {
        return errCode;
    }

    gcmASSERT(patterns->matchCount != 0 &&
        patterns->repalceCount != 0);

    ++Context->changed;

    if (VIR_Inst_GetPrev(*Inst) != gcvNULL)
    {
        VIR_Instruction *tempInst = VIR_Inst_GetPrev(*Inst);
        preInst = &tempInst;
    }

    errCode = _Pattern_Replace(Context, Function, *Inst, patterns);
    if (errCode != VSC_ERR_NONE)        return errCode;

    if ((Context->flag & VIR_PATN_CONTEXT_FLAG_RECURSION) &&
        !(patterns->flags & VIR_PATN_FLAG_NO_RECURSION))
    {
        gctINT preBaseTmpRegNo = Context->baseTmpRegNo;

        Context->baseTmpRegNo = Context->maxTmpRegNo + 1;

        if (preInst != gcvNULL)
        {
            VIR_Instruction *tempInst = VIR_Inst_GetNext(*Inst);
            preInst = &tempInst;
        }
        else
        {
            preInst = &Function->instList.pHead;
        }

        for (; *preInst != VIR_Inst_GetNext(*Inst); *preInst = VIR_Inst_GetNext(*preInst))
        {
            errCode = _Pattern_TransformByInstruction(Context, Function, preInst);
            if (errCode != VSC_ERR_NONE)        return errCode;
        }

        memset(Context->tmpRegSymbol + Context->baseTmpRegNo, 0,
            sizeof(Context->tmpRegSymbol[0]) * (VIR_PATTERN_TEMP_COUNT - Context->baseTmpRegNo));

        Context->baseTmpRegNo = preBaseTmpRegNo;
        Context->maxTmpRegNo  = preBaseTmpRegNo;
    }
    else
    {
        memset(Context->tmpRegSymbol, 0, sizeof(Context->tmpRegSymbol));
    }

    errCode = _Pattern_FreeMatchedInsts(Context, Function, Inst, patterns);
    if (errCode != VSC_ERR_NONE)        return errCode;

    return errCode;
}

static VSC_ErrCode
_Pattern_TransformByFunction(
    IN  VIR_PatternContext *Context,
    IN  VIR_Function       *Function
    )
{
    VIR_Instruction  *inst      = Function->instList.pHead;
    VSC_ErrCode       errCode   = VSC_ERR_NONE;

    for (; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
    {
        _Pattern_TransformByInstruction(Context, Function, &inst);
    }

    return errCode;
}

VSC_ErrCode
VIR_Pattern_Transform(
    IN VIR_PatternContext *Context
    )
{
    VSC_ErrCode       errCode  = VSC_ERR_NONE;
    VIR_Function     *func     = gcvNULL;
    VIR_FuncIterator  iter;
    VIR_FunctionNode *funcNode = gcvNULL;

    gcmASSERT(Context != gcvNULL &&
        Context->getPattern != gcvNULL &&
        Context->shader != gcvNULL);

    Context->changed      = 0;
    Context->baseTmpRegNo = 0;
    Context->maxTmpRegNo  = 0;

    VIR_FuncIterator_Init(&iter, &Context->shader->functions);
    funcNode = VIR_FuncIterator_First(&iter);
    for (; funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
    {
        func = funcNode->function;

        errCode = _Pattern_TransformByFunction(Context, func);
        if (errCode != VSC_ERR_NONE)        break;
    }
    return errCode;
}


