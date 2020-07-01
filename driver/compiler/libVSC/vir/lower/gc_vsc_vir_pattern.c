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


#include "vir/lower/gc_vsc_vir_pattern.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"

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
    IN PVSC_CONTEXT                vscContext,
    IN VIR_Shader                  *Shader,
    IN VSC_MM                      *pMM,
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
    Context->vscContext   = vscContext;

    memset(Context->tmpRegSymbol, 0, sizeof(Context->tmpRegSymbol));

    Context->pMM = pMM;

    return errCode;
}

VSC_ErrCode
VIR_PatternContext_Finalize(
    IN OUT VIR_PatternContext      *Context
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    return errCode;
}

static void
_Pattern_SetOperand(
    IN VIR_Instruction *Inst,
    IN gctINT           No,
    IN VIR_Operand     *Opnd
    )
{
    gcmASSERT(Inst != gcvNULL && Opnd != gcvNULL);

    if (No == 0)
    {
        VIR_Operand     *dest = VIR_Inst_GetDest(Inst);
        VIR_Operand_Copy(dest, Opnd);

        if (!VIR_Operand_isLvalue(Opnd))
        {
            VIR_Operand_SetLvalue(dest, gcvTRUE);
            VIR_Operand_SetEnable(dest,
                VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(Opnd)));
            VIR_Operand_SetRoundMode(dest, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(dest, VIR_MOD_NONE);
        }
    }
    else
    {
        VIR_Operand     *src;
        gcmASSERT(No - 1 < VIR_MAX_SRC_NUM);

        src = VIR_Inst_GetSource(Inst, No - 1);
        /* copy operand */
        VIR_Operand_Copy(src, Opnd);

        if (VIR_Operand_isLvalue(Opnd))
        {
            /* the operand is copied from pervious inst's dest,
               reset its lvalue attribute */
            VIR_Operand_SetLvalue(src, gcvFALSE);
            /* convert dest operand's enable to new operand's swizzzle */
            VIR_Operand_SetSwizzle(src,
                VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(Opnd)));
            VIR_Operand_SetRoundMode(src, VIR_ROUND_DEFAULT);
            VIR_Operand_SetModifier(src, VIR_MOD_NONE);
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
        return VIR_Inst_GetDest(Inst);
    }
    else
    {
        gcmASSERT(No - 1 < VIR_MAX_SRC_NUM);
        return VIR_Inst_GetSource(Inst, No - 1);
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
    IN VIR_Pattern      *Pattern,
    IN gctINT            Num,
    IN VIR_Instruction  *Inst
    )
{
    VIR_PatternMatchInst  *matchInsts = Pattern->matchInsts;
    VIR_SrcOperand_Iter_ExpandFlag expandFlag = VIR_SRCOPERAND_FLAG_DEFAULT;

    if (Pattern->flags & VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE)
    {
        expandFlag &= ~VIR_SRCOPERAND_FLAG_EXPAND_TEXLD_PARM_NODE;
    }

    while(matchInsts != gcvNULL)
    {
        gctSIZE_T               i = 0;
        VIR_Operand_Iterator    opndIter;
        VIR_Operand            *opnd;

        if (Inst == gcvNULL)
        {
            return gcvNULL;
        }

        VIR_Operand_Iterator_Init(Inst, &opndIter, expandFlag, gcvFALSE);

        for (i = 0, opnd = VIR_Operand_Iterator_First(&opndIter);
            i < VIR_PATTERN_OPND_COUNT && opnd != gcvNULL;
            ++i, opnd = VIR_Operand_Iterator_Next(&opndIter))
        {
            if (matchInsts->opnd[i] == Num)
            {
                return opnd;
            }
        }

        Inst = VIR_Inst_GetNext(Inst);
        ++matchInsts;
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
            VIR_TypeId ty0 = VIR_Operand_GetTypeId(Opnd0);

            VIR_TypeId ty1 = VIR_Operand_GetTypeId(Opnd1);

            if (ty0 != ty1)
            {
                return gcvFALSE;
            }

            return VIR_Operand_GetImmediateUint(Opnd0) ==
                        VIR_Operand_GetImmediateUint(Opnd1);
        }
    case VIR_OPND_SYMBOL:
    case VIR_OPND_SAMPLER_INDEXING:
        {
            VIR_TypeId  ty0   = VIR_Operand_GetTypeId(Opnd0);
            VIR_Symbol *sym0  = VIR_Operand_GetSymbol(Opnd0);
            VIR_Modifier mod0 = VIR_Operand_GetModifier(Opnd0);
            VIR_RoundMode rnd0 = VIR_Operand_GetRoundMode(Opnd0);

            VIR_TypeId  ty1   = VIR_Operand_GetTypeId(Opnd1);
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
    case VIR_OPND_NAME:
        return VIR_Operand_GetNameId(Opnd0) == VIR_Operand_GetNameId(Opnd1);
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
    VIR_Type            *ty    = VIR_Shader_GetTypeFromId(Context->shader, VIR_Operand_GetTypeId(Operand));
    VIR_PrimitiveTypeId  baseType;
    VIR_PrimitiveTypeId  dstTy = VIR_TYPE_UNKNOWN;

    gcmASSERT(VIR_Type_isPrimitive(ty));

    baseType = VIR_Type_GetBaseTypeId(ty);

    if (baseType != VIR_TYPE_UNKNOWN)
    {
        dstTy = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(baseType), Component, 1);
    }

    VIR_Operand_SetTypeId(Operand, dstTy);
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

    if ((curPattern->flags & VIR_PATN_FLAG_ALREADY_MATCHED_AND_REPLACED) !=0)
        return gcvTRUE;

    while(curPattern->matchCount != 0)
    {
        gctSIZE_T i         = 0;
        gctBOOL   isMatched = gcvTRUE;
        VIR_SrcOperand_Iter_ExpandFlag expandFlag = VIR_SRCOPERAND_FLAG_DEFAULT;

        if (curPattern->flags & VIR_PATN_FLAG_NOT_EXPAND_TEXLD_PARM_NODE)
        {
            expandFlag &= ~VIR_SRCOPERAND_FLAG_EXPAND_TEXLD_PARM_NODE;
        }

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

            isMatched = isMatched && Context->cmpOpcode(Context, patternInst, curInst);

            if (isMatched && patternInst->condOp != VIR_PATTERN_ANYCOND)
            {
                isMatched = ((gctINT)(patternInst->condOp) == (gctINT)VIR_Inst_GetConditionOp(curInst));
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
                        isMatched = isMatched && patternInst->function[j](Context, curInst);

                        if (!isMatched) break;
                    }
                }
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            if (!isMatched)   break;


            VIR_Operand_Iterator_Init(curInst, &opndIter, expandFlag, gcvFALSE);

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

                orgOpnd = _Pattern_GetOperandByPattern(curPattern, ptnOpnd, Inst);

                isMatched = isMatched && _Pattern_CmpOperand(Context->shader, curOpnd, orgOpnd);
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

        nxtInst = VIR_Inst_GetNext(curInst);
        if ((Pattern->flags & VIR_PATN_FLAG_DELETE_MANUAL))
        {
            gctSIZE_T j = 0;
            /* reset all sources */
            for (j = 0; j < VIR_Inst_GetSrcNum(curInst); ++j)
            {
                VIR_Inst_SetSource(curInst, j, gcvNULL);
            }
            /* reset dest */
            VIR_Inst_SetDest(curInst, gcvNULL);
        }

        VIR_Function_DeleteInstruction(Function, curInst);
        curInst = nxtInst;
    }

    if (*Inst == gcvNULL)
    {
        *Inst = Function->instList.pHead;;
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
    VIR_RES_OP_TYPE         resOpType = VIR_Inst_GetResOpType(Inst);

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
            gcvTRUE,
            &insertedInst);
        if (errCode != VSC_ERR_NONE) return errCode;

        insertedInst->sourceLoc = Inst->sourceLoc;
        insertedInst->_isPatternRep = gcvTRUE;

        /* Update the resOpType. */
        if (VIR_OPCODE_isTexLd(replacedPtnInst->opcode))
        {
            VIR_Inst_SetResOpType(insertedInst, resOpType);
        }

        if (VIR_Inst_GetOpcode(insertedInst) == VIR_OP_LABEL)
        {
            VIR_LabelId labelID;
            VIR_Label *virLabel;

            errCode = VIR_Function_AddLabel(Function, gcvNULL, &labelID);
            virLabel = VIR_GetLabelFromId(Function, labelID);
            virLabel->defined = insertedInst;
            VIR_Operand_SetLabel(VIR_Inst_GetDest(insertedInst), virLabel);
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

                    if (!(Pattern->flags & VIR_PATN_FLAG_SET_TEMP_IN_FUNC))
                    {
                        VIR_Operand     *templeteOpnd = gcvNULL;

                        gcmASSERT(Pattern->matchCount >= 1);

                        if (VIR_PATTERN_DefaultTempType(replacedPtnInst))
                        {
                            VIR_Instruction *lastPatternInst = _Pattern_GetInstruction(Inst, Pattern->matchCount - 1);
                            templeteOpnd = VIR_Inst_GetDest(lastPatternInst);
                        }
                        else if (VIR_PATTERN_SpecialTempType(replacedPtnInst))
                        {
                            if (VIR_Covers(VIR_PATTERN_TEMP_TYPE_XYZW, VIR_PATTERN_GetTempType(replacedPtnInst)))
                            {
                                VIR_Instruction *lastPatternInst = _Pattern_GetInstruction(Inst, Pattern->matchCount - 1);
                                templeteOpnd = VIR_Inst_GetDest(lastPatternInst);
                            }
                            else
                            {
                                gcmASSERT(0);
                            }
                        }
                        else
                        {
                            templeteOpnd = _Pattern_GetOperandByPattern(Pattern, VIR_PATTERN_GetReferNo(replacedPtnInst), Inst);
                        }

                        if (templeteOpnd != gcvNULL)
                        {
                            VIR_Operand_SetTypeId(opnd, VIR_Operand_GetTypeId(templeteOpnd));
                            VIR_Symbol_SetType(sym, VIR_Shader_GetTypeFromId(Context->shader, VIR_Operand_GetTypeId(templeteOpnd)));
                            if (VIR_Operand_isLvalue(templeteOpnd))
                            {
                                VIR_Operand_SetEnable(opnd, VIR_Operand_GetEnable(templeteOpnd));
                            }
                            else
                            {
                                VIR_Operand_SetEnable(opnd, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(templeteOpnd)));
                            }
                            if (replacedPtnInst->flag & VIR_PATN_FLAG_COPY_ROUNDING_MODE)
                            {
                                VIR_Operand_SetRoundMode(opnd, VIR_Operand_GetRoundMode(templeteOpnd));
                            }
                            else
                            {
                               VIR_Operand_SetRoundMode(opnd, VIR_ROUND_DEFAULT);
                            }
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
                if(!opnd && replacedPtnInst->function[j] == VIR_Lower_SkipOperand )
                {
                    continue;
                }
            }
            else
            {
                opnd = _Pattern_GetOperandByPattern(Pattern, ptnOpnd, Inst);
            }

            _Pattern_SetOperand(insertedInst, j, opnd);

            if (Pattern->flags & VIR_PATN_FLAG_EXPAND)
            {
                if (Pattern->flags & VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE)
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

        if(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(insertedInst)))
        {
            VIR_Label* label = VIR_Operand_GetLabel(VIR_Inst_GetDest(insertedInst));
            if(label)   /* label may be filled later because it may not be generated yet here */
            {
                VIR_Link* link;

                errCode = VIR_Function_NewLink(Function, &link);
                if(errCode)
                {
                    return errCode;
                }
                VIR_Link_SetReference(link, (gctUINTPTR_T)insertedInst);
                VIR_Link_AddLink(VIR_Label_GetReferenceAddr(label), link);
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

    clonedPattern = (VIR_Pattern *)vscMM_Alloc(Context->pMM, sizeof(VIR_Pattern));
    memcpy(clonedPattern, Pattern, sizeof(VIR_Pattern));

    clonedPattern->matchInsts = (VIR_PatternMatchInst *)vscMM_Alloc(Context->pMM, sizeof(VIR_PatternMatchInst));
    memcpy(clonedPattern->matchInsts, Pattern->matchInsts, sizeof(VIR_PatternMatchInst) * Pattern->matchCount);

    clonedPattern->replaceInsts = (VIR_PatternReplaceInst *)vscMM_Alloc(Context->pMM, sizeof(VIR_PatternReplaceInst));
    memcpy(clonedPattern->replaceInsts, Pattern->replaceInsts, sizeof(VIR_PatternReplaceInst) * Pattern->repalceCount);

    return clonedPattern;
}

static void
_Pattern_DestroyClonedPattern(
    IN VIR_PatternContext   *Context,
    IN VIR_Pattern          *Pattern
    )
{
    vscMM_Free(Context->pMM, Pattern->matchInsts);
    vscMM_Free(Context->pMM, Pattern->replaceInsts);
    vscMM_Free(Context->pMM, Pattern);
}

static gctBOOL
_Pattern_NotExpandForSameComponentValue(
    IN VIR_Pattern*     pPattern,
    IN VIR_Shader*      pShader,
    IN VIR_Instruction* pInst
    )
{
    gctUINT8            i, j;
    gctBOOL             bShouldExpand = gcvFALSE;
    VIR_Enable          enable = VIR_Inst_GetEnable(pInst);

    gcmASSERT(VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pInst)));

    /*
    ** If the source uses the same swizzle for all enabled channels, then we don't need to expand this instruction,
    ** we just need to re-swizzle the source operand, for example:
    **  1) DIV, r0, r1.xxxx, r2.xxxx     ----> not expand.
    **  2) DIV, r0.xz   r1.xyxx, r2.yxyy     ----> not epxand and w-shift to:
    **     DIV, r0.xz   r1.xxxx, r2.yyyy
    */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_OperandInfo opndInfo;
        VIR_Swizzle     swiz, currentChannelSwizzle, prevEnableSwizzle = VIR_SWIZZLE_X;
        gctBOOL         bFirstEnable = gcvTRUE;
        VIR_Operand*    pSrcOpnd = VIR_Inst_GetSource(pInst, i);
        gctBOOL         bVecConst = gcvFALSE;
        VIR_Const*      pConst = gcvNULL;

        VIR_Operand_GetOperandInfo(pInst, pSrcOpnd, &opndInfo);

        if (opndInfo.isImmVal)
        {
            continue;
        }

        bVecConst = opndInfo.isVecConst;
        swiz = VIR_Operand_GetSwizzle(pSrcOpnd);

        for (j = 0; j < VIR_CHANNEL_COUNT; j++)
        {
            /* Skip the non-enabled channel. */
            if (!(enable & (1 << j)))
            {
                continue;
            }

            currentChannelSwizzle = VIR_Swizzle_GetChannel(swiz, j);
            if (bFirstEnable)
            {
                prevEnableSwizzle = currentChannelSwizzle;
                bFirstEnable = gcvFALSE;
            }
            else if (currentChannelSwizzle != prevEnableSwizzle)
            {
                /*
                ** For a vector constant, if two different channels have the same constant value,
                ** we can still treat it as non-expand.
                */
                if (bVecConst)
                {
                    pConst = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(pSrcOpnd));
                    gcmASSERT(pConst);
                    if (pConst->value.vecVal.u32Value[currentChannelSwizzle] == pConst->value.vecVal.u32Value[prevEnableSwizzle])
                    {
                        continue;
                    }
                }

                bShouldExpand = gcvTRUE;
                break;
            }
        }

        /* Check if we should expand this source operand. */
        if (bShouldExpand)
        {
            break;
        }

        /* Use the same swizzle for all channels. */
        for (j = 0; j < VIR_CHANNEL_COUNT; j++)
        {
            VIR_Swizzle_SetChannel(swiz, j, prevEnableSwizzle);
        }
        VIR_Operand_SetSwizzle(pSrcOpnd, swiz);
    }

    return !bShouldExpand;
}

static gctBOOL
_Pattern_NotExpand(
    IN VIR_Pattern     *Pattern,
    IN VIR_Shader      *Shader,
    IN VIR_Instruction *Inst
    )
{
    /* Check if we need to expand the instruction for the same component value pattern. */
    if (Pattern->flags & VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE)
    {
        return _Pattern_NotExpandForSameComponentValue(Pattern, Shader, Inst);
    }

    return gcvTRUE;
}

static gctBOOL
_Pattern_NotAddMov(
    IN VIR_Instruction    *Inst
    )
{
    gctBOOL     notSame = gcvTRUE;
    gctSIZE_T   i       = 0;


    gcmASSERT(Inst != gcvNULL &&
        VIR_Inst_GetDest(Inst) != gcvNULL);

    for (i = 0; i < VIR_Inst_GetSrcNum(Inst); i++)
    {
        if (VIR_Operand_SameSymbol(VIR_Inst_GetDest(Inst), VIR_Inst_GetSource(Inst, i)))
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
        VIR_Enable  enable  = VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst));
        VIR_Swizzle swizzle = VIR_Operand_GetSwizzle(VIR_Inst_GetSource(Inst, i));

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

    gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL && Pattern->matchCount == 1);

    /* Check if we need to expand this instruction. */
    if (_Pattern_NotExpand(Pattern, Context->shader, Inst))
    {
        /*
        VIR_Dumper *dumper     = Context->shader->dumper;
        VIR_LOG(dumper, "_Pattern_NotExpand:");
        VIR_LOG_FLUSH(dumper);
        */

        Pattern = _Pattern_ClonePattern(Context, Pattern);

        /* Reset the flags. */
        if (Pattern->flags & VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE)
        {
            Pattern->flags = (VIR_PatnFlag)(Pattern->flags ^ VIR_PATN_FLAG_EXPAND_MODE_SAME_COMPONENT_VALUE);
        }
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
        expandEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst));
    }

    for (component = 0x1, i = 0; i < 4; component <<= 1, ++i)
    {
        gcmASSERT(VIR_Inst_GetDest(Inst) != gcvNULL &&
            VIR_Operand_isLvalue(VIR_Inst_GetDest(Inst)));

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
            gcvTRUE,
            &movTemp2Dest);
        if (errCode != VSC_ERR_NONE) return errCode;
        movTemp2Dest->_isPatternRep = gcvTRUE;

        _Pattern_SetOperand(movTemp2Dest, 0, VIR_Inst_GetDest(Inst));
        _Pattern_SetOperand(movTemp2Dest, 1, VIR_Inst_GetDest(lastReplacedInst));

        VIR_Operand_SetTypeId(VIR_Inst_GetSource(movTemp2Dest, 0), VIR_Operand_GetTypeId(VIR_Inst_GetDest(Inst)));
        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(movTemp2Dest, 0),
            VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(VIR_Inst_GetDest(Inst))));
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

    if (Pattern->flags & VIR_PATN_FLAG_ALREADY_MATCHED_AND_REPLACED)
    {
        return errCode;
    }
    else if (Pattern->flags & VIR_PATN_FLAG_EXPAND_COMPONENT_INLINE)
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
    IN OUT VIR_Instruction  **Inst,
    OUT gctBOOL             *MoveToNextInst
    )
{
    VSC_ErrCode       errCode  = VSC_ERR_NONE;
    VIR_Pattern      *patterns;
    VIR_Instruction  *preInst;

    *MoveToNextInst = gcvTRUE;
    preInst = VIR_Inst_GetPrev(*Inst);
    patterns = Context->getPattern(Context, *Inst);
    if (patterns == gcvNULL ||
        _Pattern_isMatched(Context, *Inst, &patterns) == gcvFALSE)
    {
        return errCode;
    }

    gcmASSERT(patterns->matchCount != 0 &&
        patterns->repalceCount != 0);

    ++Context->changed;

    errCode = _Pattern_Replace(Context, Function, *Inst, patterns);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }
    memset(Context->tmpRegSymbol, 0, sizeof(Context->tmpRegSymbol));

    if ((patterns->flags & VIR_PATN_FLAG_ALREADY_MATCHED_AND_REPLACED) == 0)
    {
        errCode = _Pattern_FreeMatchedInsts(Context, Function, Inst, patterns);
        if (errCode != VSC_ERR_NONE)        return errCode;
    }

    if ((patterns->flags & VIR_PATN_FLAG_RECURSIVE_SCAN_NEWINST) !=0)
    {
        VIR_Instruction * nextInstToTransform;
        *MoveToNextInst = gcvFALSE;
        /* find the first replace inst */
        if (preInst)
        {
            nextInstToTransform = VIR_Inst_GetNext(preInst);
        }
        else
        {
            /* get the first inst of the function */
            nextInstToTransform = Function->instList.pHead;
        }
        *Inst = nextInstToTransform;  /* need to rescan the instruction from new added inst */
    }
    else if ((patterns->flags & VIR_PATN_FLAG_RECURSIVE_SCAN) != 0 )
    {
        VIR_Instruction * nextInstToTransform;
        *MoveToNextInst = gcvFALSE;
        /* find the first replace inst */
        if (preInst)
        {
            nextInstToTransform = preInst;
        }
        else
        {
            /* get the first inst of the function */
            nextInstToTransform = Function->instList.pHead;
        }
        *Inst = nextInstToTransform;  /* need to rescan the instruction from new added inst */
    }

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
    gctBOOL           moveToNextInst = gcvTRUE;

    while (inst != gcvNULL)
    {
        _Pattern_TransformByInstruction(Context, Function, &inst, &moveToNextInst);
        if (inst == gcvNULL)
        {
            break;
        }
        if (moveToNextInst)
        {
             inst = VIR_Inst_GetNext(inst);
        }
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
        if (errCode != VSC_ERR_NONE) break;
    }
    return errCode;
}


