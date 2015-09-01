/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "vir/transform/gc_vsc_vir_fcp.h"

void _VIR_FCP_ReplaceLDARR(
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst)
{
    VIR_Operand             *pOpnd;
    VIR_OperandInfo         operandInfo;

    VIR_DEF                 *pDef;
    VIR_DEF_KEY             defKey;
    VIR_USAGE               *pUsage;
    VIR_Operand             *pUseOpnd;
    VSC_DU_ITERATOR          duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
    VIR_Instruction         *pUseInst;
    VIR_OperandInfo          src1OpndInfo;
    gctUINT                  defIdx = VIR_INVALID_DEF_INDEX;

    VIR_Swizzle              src1_swizzle = VIR_SWIZZLE_X;

    gctUINT                  indexSymId = VIR_INVALID_ID;
    VIR_Symbol               *baseSym = gcvNULL;
    VIR_Symbol               *indexSym = gcvNULL;

    gctBOOL hasOtherDef = gcvFALSE;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR);

    /* replace LDARR only at its indexing use
    2: ldarr dst1, src0, t2.x
    ...
    4: texld dst, sampler dst1, XXX   <== dst1 is only defined by ldarr
    ==>
    2: (removed)
    ...
    4: texld dst, src0[t2.x], XXX
    */

    VIR_Operand_GetOperandInfo(pInst, pInst->src[VIR_Operand_Src1], &src1OpndInfo);

    if(!VIR_OpndInfo_Is_Virtual_Reg(&src1OpndInfo) &&
       !src1OpndInfo.isImmVal)
    {
        return;
    }

    /* relIndex is sym id */
    if (VIR_OpndInfo_Is_Virtual_Reg(&src1OpndInfo))
    {
        indexSymId  = VIR_Operand_GetSymbolId_(pInst->src[VIR_Operand_Src1]);
        indexSym = VIR_Shader_GetSymFromId(pFunc->hostShader, indexSymId);
        /* don't replace if the index is an attribute, since vir->gcsl need
           a temp in the index*/
        if (indexSym &&
            VIR_Symbol_GetKind(indexSym) == VIR_SYM_VARIABLE &&
            VIR_Symbol_GetStorageClass(indexSym) == VIR_STORAGE_INPUT)
        {
            return;
        }
    }

    /* get one component of ldarr's src1 swizzle */
    src1_swizzle = VIR_Operand_GetSwizzle(pInst->src[VIR_Operand_Src1]) & 0x3;

    /* get the base src0 */
    pOpnd = pInst->src[VIR_Operand_Src0];
    baseSym  = VIR_Operand_GetSymbol(pOpnd);

    if(VIR_MOD_NONE != VIR_Operand_GetModifier(pInst->dest) ||
        VIR_MOD_NONE != VIR_Operand_GetModifier(pInst->src[VIR_Operand_Src1]) ||
        VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(pInst->dest) ||
        VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(pInst->src[VIR_Operand_Src1])
        )
    {
        return;
    }

    if (baseSym)
    {
        /* get ldarr's dst LR */
        VIR_Operand_GetOperandInfo(pInst,
                                   pInst->dest,
                                   &operandInfo);

        if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
        {
            defKey.pDefInst = pInst;
            defKey.regNo = operandInfo.u1.virRegInfo.virReg;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
        }

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            VIR_OperandId    opndId;
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

            /* go through all the uses */
            VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
            pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
            for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
            {
                VIR_Swizzle   useSwizzle  = VIR_SWIZZLE_INVALID;
                VIR_RoundMode useRound    = VIR_ROUND_DEFAULT;
                VIR_Modifier  useModifier = VIR_MOD_NONE;

                pUsage   = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
                pUseInst = pUsage->usageKey.pUsageInst;
                pUseOpnd = pUsage->usageKey.pOperand;

                if (pUseInst == VIR_OUTPUT_USAGE_INST)
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                /* already replaced*/
                if (VIR_Operand_GetRelAddrMode(pUseOpnd) != VIR_INDEXED_NONE)
                {
                    continue;
                }

                if(!vscVIR_IsUniqueDefInstOfUsageInst(
                            pDuInfo,
                            pUseInst,
                            pUsage->usageKey.pOperand,
                            pInst,
                            gcvNULL))
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                if(VIR_Operand_GetRelAddrMode(pUseOpnd) != VIR_INDEXED_NONE)
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                if(VIR_Inst_GetOpcode(pUseInst) == VIR_OP_LDARR ||
                   VIR_Inst_GetOpcode(pUseInst) == VIR_OP_STARR)
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                if(VIR_OpndInfo_Is_Virtual_Reg(&src1OpndInfo) &&
                    VIR_Operand_GetConstIndexingImmed(pUseOpnd) != 0)
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                useSwizzle  = VIR_Operand_GetSwizzle(pUseOpnd);
                useRound    = VIR_Operand_GetRoundMode(pUseOpnd);
                useModifier = VIR_Operand_GetModifier(pUseOpnd);

                opndId = VIR_Operand_GetIndex(pUseOpnd);
                *pUseOpnd = *pOpnd;
                VIR_Operand_SetIndex(pUseOpnd, opndId);
                /* we need to set useOpnd's type to the LDARR's dst type */

                VIR_Operand_SetType(pUseOpnd, VIR_Operand_GetType(pInst->dest));

                /* use the swizzle of pUseOpnd
                   add t1, base[i].yzyz, t2
                   after converter ==>
                   LDARR t3.yz, base.yyzz, i
                   add t1, t3.yzyz, t2
                   after fcp ==>
                   add t1, base[i].yzyz, t2
                   */
                VIR_Operand_SetSwizzle(pUseOpnd, useSwizzle);
                VIR_Operand_SetRoundMode(pUseOpnd, useRound);
                VIR_Operand_SetModifier(pUseOpnd, useModifier);

                if(VIR_OpndInfo_Is_Virtual_Reg(&src1OpndInfo))
                {
                    gcmASSERT(indexSymId < VIR_INVALID_ID);

                    VIR_Operand_SetRelIndexing(pUseOpnd, indexSymId);
                    VIR_Operand_SetRelAddrMode(pUseOpnd, src1_swizzle + 1);
                }
                else
                {
                    VIR_TypeId src1Type = VIR_Operand_GetType(pInst->src[1]);
                    gctINT src1Imm = 0;

                    gcmASSERT(src1OpndInfo.isImmVal);
                    gcmASSERT(indexSymId == VIR_INVALID_ID);

                    if (src1Type == VIR_TYPE_INT32)
                    {
                        src1Imm = src1OpndInfo.u1.immValue.iValue;
                    }else if (src1Type == VIR_TYPE_FLOAT32)
                    {
                        src1Imm = (gctINT) src1OpndInfo.u1.immValue.fValue;
                    }else if (src1Type == VIR_TYPE_UINT32)
                    {
                        src1Imm = (gctINT) src1OpndInfo.u1.immValue.uValue;
                    }
                    else
                    {
                        gcmASSERT(gcvFALSE);
                    }

                    gcmASSERT(src1Imm >= 0);

                    VIR_Operand_SetRelIndexingImmed(pUseOpnd,
                        src1Imm + VIR_Operand_GetConstIndexingImmed(pUseOpnd));
                    VIR_Operand_SetRelAddrMode(pUseOpnd, VIR_INDEXED_NONE);
                }
            }

            defIdx = pDef->nextDefIdxOfSameRegNo;
        }

        if (!hasOtherDef)
        {
            VIR_Function_RemoveInstruction(pFunc, pInst);
        }
    }
}

void _VIR_FCP_ReplaceDUAL32(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst)
{
    VIR_Instruction     *newInst = gcvNULL;
    VIR_Operand         *newOpnd;
    gctUINT             i;

    VIR_Function_AddInstructionAfter(pFunc,
                VIR_Inst_GetOpcode(pInst),
                VIR_TYPE_UINT16,
                pInst,
                &newInst);

    if (pInst->_parentUseBB)
    {
        VIR_Inst_SetBasicBlock(newInst, VIR_Inst_GetBasicBlock(pInst));
    }
    else
    {
        VIR_Inst_SetFunction(newInst, VIR_Inst_GetFunction(pInst));
    }

    /* copy source */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_Function_DupOperand(pFunc, pInst->src[i], &newOpnd);
        if (VIR_Operand_GetPrecision(pInst->src[i]) == VIR_PRECISION_HIGH)
        {
            gcmASSERT(VIR_Operand_GetHIHwRegId(pInst->src[i]) != VIR_FCP_INVALID_REG);
            VIR_Operand_SetHwRegId(newOpnd, VIR_Operand_GetHIHwRegId(pInst->src[i]));
            VIR_Operand_SetHwShift(newOpnd, VIR_Operand_GetHIHwShift(pInst->src[i]));
        }
        if (VIR_Operand_GetRelAddrMode(newOpnd) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(newOpnd));
            gcmASSERT(sym);
            if (VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_HIGH)
            {
                /* newOpnd's relAddrMode saves the low shift, add the difference of
                   high shift and low shift */
                gctUINT relMode = VIR_Operand_GetRelAddrMode(newOpnd);
                relMode = relMode + VIR_Symbol_GetHIHwShift(sym) - VIR_Symbol_GetHwShift(sym);
                VIR_Operand_SetRelAddrMode(newOpnd, relMode);
            }
        }
        newInst->src[i] = newOpnd;
    }
    VIR_Inst_SetSrcNum(newInst, VIR_Inst_GetSrcNum(pInst));

    /* duplicate dest and copy dest */
    if (pInst->dest)
    {
        VIR_Function_DupOperand(pFunc, pInst->dest, &newOpnd);
        if (VIR_Operand_GetPrecision(pInst->dest) == VIR_PRECISION_HIGH)
        {
            gcmASSERT(VIR_Operand_GetHIHwRegId(pInst->dest) != VIR_FCP_INVALID_REG);
            VIR_Operand_SetHwRegId(newOpnd, VIR_Operand_GetHIHwRegId(pInst->dest));
            VIR_Operand_SetHwShift(newOpnd, VIR_Operand_GetHIHwShift(pInst->dest));
        }
        if (VIR_Operand_GetRelAddrMode(newOpnd) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(newOpnd));
            gcmASSERT(sym);
            if (VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_HIGH)
            {
                /* newOpnd's relAddrMode saves the low shift, add the difference of
                   high shift and low shift */
                gctUINT relMode = VIR_Operand_GetRelAddrMode(newOpnd);
                relMode = relMode + VIR_Symbol_GetHIHwShift(sym) - VIR_Symbol_GetHwShift(sym);
                VIR_Operand_SetRelAddrMode(newOpnd, relMode);
            }
        }
        newInst->dest = newOpnd;
    }

    VIR_Inst_SetConditionOp(newInst, VIR_Inst_GetConditionOp(pInst));

    /* set thread mode*/
    VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_SINGLE_T0);
    VIR_Inst_SetThreadMode(newInst, VIR_THREAD_D16_SINGLE_T1);
}

static void
_changeConvSrc1(IN VIR_Instruction    *Inst)
{
    VIR_ScalarConstVal  imm0;
    VIR_Operand         *opnd = gcvNULL;
    VIR_TypeId          ty, componentTy;
    gctBOOL            needChangeSrc1 = gcvFALSE;

    if (VIR_Inst_GetOpcode(Inst) == VIR_OP_AQ_I2I)
    {
        opnd = VIR_Inst_GetDest(Inst);
    }
    else
    {
        opnd = VIR_Inst_GetSource(Inst, 0);
    }
    ty = VIR_Operand_GetType(opnd);
    componentTy = VIR_GetTypeComponentType(ty);

    imm0.iValue = 0x1;

    if (VIR_Operand_GetPrecision(opnd) != VIR_PRECISION_HIGH)
    {
        switch (componentTy)
        {
        case VIR_TYPE_FLOAT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x1;
            break;
        case VIR_TYPE_INT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x3;
            break;
        case VIR_TYPE_UINT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x6;
            break;
        case VIR_TYPE_BOOLEAN:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x3;
            break;
        default:
            break;
        }

        if (VIR_Inst_GetOpcode(Inst) == VIR_OP_AQ_I2I)
        {
            imm0.iValue <<= 4;
        }
    }

    if (needChangeSrc1)
    {
        VIR_Operand_SetImmediate(Inst->src[1],
            VIR_TYPE_INT32,
            imm0);
    }
}

VSC_ErrCode VIR_FCP_PerformOnShader(
    VIR_Shader          *pShader,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VSC_OPTN_FCPOptions *pOptions,
     VIR_Dumper         *pDumper
    )
{
    VSC_ErrCode       status = VSC_ERR_NONE;

    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    if (VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetTrace(pOptions),
        VSC_OPTN_FCPOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Final Cleanup Phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    if (VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetOPTS(pOptions),
                        VSC_OPTN_FCPOptions_REPL_LDARR))
    {

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
        for (func_node = VIR_FuncIterator_First(&func_iter);
                func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function*    func = func_node->function;
            VIR_InstIterator inst_iter;
            VIR_Instruction* inst;

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                    inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
                {
                    _VIR_FCP_ReplaceLDARR(pDuInfo, func, inst);
                }
            }
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetOPTS(pOptions),
                        VSC_OPTN_FCPOptions_SPLIT_DUAL32))
    {
        if (VIR_Shader_isDual16Mode(pShader))
        {
            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
            for (func_node = VIR_FuncIterator_First(&func_iter);
                    func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
            {
                VIR_Function*    func = func_node->function;
                VIR_InstIterator inst_iter;
                VIR_Instruction* inst;

                VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
                for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                        inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
                {
                    if (VIR_Inst_GetThreadMode(inst) == VIR_THREAD_D16_DUAL_32)
                    {
                        _VIR_FCP_ReplaceDUAL32(pShader, func, inst);
                    }

                    /* correctly setting src1 for conv/i2i instruction  */
                    if (VIR_Inst_GetOpcode(inst) == VIR_OP_AQ_CONV ||
                        VIR_Inst_GetOpcode(inst) == VIR_OP_AQ_I2I)
                    {
                        _changeConvSrc1(inst);
                    }
                }
            }
        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetTrace(pOptions),
        VSC_OPTN_FCPOptions_TRACE_OUTPUT) || gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after Final Cleanup Phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return status;
}


