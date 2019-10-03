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


#include "vir/transform/gc_vsc_vir_fcp.h"

void _VIR_ReplaceIndexOpnd(
    VIR_Operand     *pIdxOpnd,
    VIR_Operand     *pUseOpnd,
    VIR_OperandInfo *opndInfo)
{
    VIR_Swizzle     idxSwizzle = VIR_SWIZZLE_INVALID;
    idxSwizzle = VIR_Operand_GetSwizzle(pIdxOpnd) & 0x3;

    /* change base pUseOpnd to base[index] */
    if(VIR_OpndInfo_Is_Virtual_Reg(opndInfo))
    {
        /* relIndex is sym id */
        gctUINT idxSymId = VIR_Operand_GetSymbolId_(pIdxOpnd);

        gcmASSERT(idxSymId < VIR_INVALID_ID);
        gcmASSERT(VIR_Operand_GetConstIndexingImmed(pUseOpnd) == 0);

        VIR_Operand_SetRelIndexing(pUseOpnd, idxSymId);
        VIR_Operand_SetRelAddrMode(pUseOpnd, idxSwizzle + 1);
    }
    else if (opndInfo->isImmVal)
    {
        VIR_TypeId src1Type = VIR_Operand_GetTypeId(pIdxOpnd);
        gctINT src1Imm = 0;

        if (src1Type == VIR_TYPE_INT32)
        {
            src1Imm = opndInfo->u1.immValue.iValue;
        }else if (src1Type == VIR_TYPE_FLOAT32)
        {
            src1Imm = (gctINT) opndInfo->u1.immValue.fValue;
        }else if (src1Type == VIR_TYPE_UINT32)
        {
            src1Imm = (gctINT) opndInfo->u1.immValue.uValue;
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
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

void _VIR_ReplaceLDARR(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Instruction     *pInst,
    gctBOOL             *pInvalidCfg)
{
    VIR_Operand             *pSrc1Opnd = VIR_Inst_GetSource(pInst, 1);
    VIR_Operand             *pSrc0Opnd = VIR_Inst_GetSource(pInst, 0);
    VIR_OperandInfo          dstOpndInfo, src1OpndInfo;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR);

    VIR_Operand_GetOperandInfo(pInst, pSrc1Opnd, &src1OpndInfo);

    /* replace LDARR at its indexing use
    case 1:
    2: ldarr dst1, src0, t2.x
    ...
    4: texld dst, dst1, XXX   <== dst1 is only defined by ldarr
    ==>
    2: (removed)
    ...
    4: texld dst, src0[t2.x], XXX

    case 2:
    2: ldarr dst1, src0, t2.x
    ...
    4: texld dst, dst1, XXX
    ==>
    2: mov dst1, src0[t2.x]
    ...
    4: texld dst, dst1, XXX

    For performance purpose, for dual16 shader, we always do as case2,
    since indexing should always be single-t.
    VIV:TODO: could be smarter to compute the benefits and costs.
    */

    if (VIR_Shader_isDual16Mode(pShader))
    {
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(pInst, 1);
        VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_32);

        /* relIndex is sym id */
        _VIR_ReplaceIndexOpnd(pSrc1Opnd, pSrc0Opnd, &src1OpndInfo);
    }
    else
    {
        VIR_DEF                 *pDef;
        VIR_DEF_KEY             defKey;
        VIR_USAGE               *pUsage;
        VIR_Operand             *pUseOpnd;
        VSC_DU_ITERATOR          duIter;
        VIR_DU_CHAIN_USAGE_NODE *pUsageNode;
        VIR_Instruction         *pUseInst;
        gctBOOL                 hasOtherDef = gcvFALSE;
        gctUINT                 defIdx = VIR_INVALID_DEF_INDEX, srcIndex;
        VIR_Operand *           opnd;

        opnd = VIR_Inst_GetDest(pInst);
        if(VIR_MOD_NONE != VIR_Operand_GetModifier(opnd) ||
           VIR_MOD_NONE != VIR_Operand_GetModifier(pSrc1Opnd) ||
           VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(opnd) ||
           VIR_ROUND_DEFAULT != VIR_Operand_GetRoundMode(pSrc1Opnd) ||
           !VIR_Operand_GetSymbol(pSrc0Opnd)
           )
        {
            return;
        }

        /* get ldarr's dst LR */
        VIR_Operand_GetOperandInfo(pInst, opnd, &dstOpndInfo);

        if (VIR_OpndInfo_Is_Virtual_Reg(&dstOpndInfo))
        {
            defKey.pDefInst = pInst;
            defKey.regNo = dstOpndInfo.u1.virRegInfo.virReg;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
        }

        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

            if (pDef->defKey.pDefInst != pInst)
            {
                defIdx = pDef->nextDefIdxOfSameRegNo;
                continue;
            }

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

                if (VIR_IS_OUTPUT_USAGE_INST(pUseInst))
                {
                    hasOtherDef = gcvTRUE;
                    continue;
                }

                /* already replaced*/
                if (VIR_Operand_GetRelAddrMode(pUseOpnd) != VIR_INDEXED_NONE)
                {
                    continue;
                }

                if (!vscVIR_IsUniqueDefInstOfUsageInst(
                            pDuInfo,
                            pUseInst,
                            pUsage->usageKey.pOperand,
                            pUsage->usageKey.bIsIndexingRegUsage,
                            pInst,
                            gcvNULL))
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

                srcIndex = VIR_Inst_GetSourceIndex(pUseInst, pUsage->usageKey.pOperand);
                gcmASSERT(srcIndex < VIR_MAX_SRC_NUM );

                useSwizzle  = VIR_Operand_GetSwizzle(pUseOpnd);
                useRound    = VIR_Operand_GetRoundMode(pUseOpnd);
                useModifier = VIR_Operand_GetModifier(pUseOpnd);

                VIR_Function_DupOperand(pFunc, pSrc0Opnd, &pUseOpnd);

                /* we need to set useOpnd's type to the LDARR's dst type */
                VIR_Operand_SetTypeId(pUseOpnd, VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst)));

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

                _VIR_ReplaceIndexOpnd(pSrc1Opnd, pUseOpnd, &src1OpndInfo);

                /* update the du - not complete yet
                   only delete the usage of t1, not add usage for base and offset */
                vscVIR_DeleteUsage(pDuInfo,
                        pInst,
                        pUseInst,
                        pUsage->usageKey.pOperand,
                        pUsage->usageKey.bIsIndexingRegUsage,
                        dstOpndInfo.u1.virRegInfo.virReg,
                        1,
                        VIR_Swizzle_2_Enable(useSwizzle),
                        VIR_HALF_CHANNEL_MASK_FULL,
                        gcvNULL);

                VIR_Inst_ChangeSource(pUseInst, srcIndex, pUseOpnd);
            }
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }

        if (!hasOtherDef)
        {
            /* case 1: remove the LDARR */
            VIR_Pass_RemoveInstruction(pFunc, pInst, pInvalidCfg);
        }
        else
        {
            /* case 2: change LDARR to MOV
               LDARR t1, base, offset ==>
               MOV   t1, base[offset] */
            /* relIndex is sym id */
            _VIR_ReplaceIndexOpnd(pSrc1Opnd, pSrc0Opnd, &src1OpndInfo);

            VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
            VIR_Inst_ChangeSrcNum(pInst, 1);
        }
    }
}

void _VIR_ReplaceSTARR(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Instruction     *pInst)
{
    VIR_Operand *       src0Opnd = VIR_Inst_GetSource(pInst, 0);
    VIR_OperandInfo     src0OpndInfo, destInfo;
    VIR_Operand *       opnd;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_STARR);

    VIR_Operand_GetOperandInfo(pInst, src0Opnd, &src0OpndInfo);

    if(VIR_OpndInfo_Is_Virtual_Reg(&src0OpndInfo) && !src0OpndInfo.isInput)
    {
        _VIR_ReplaceIndexOpnd(src0Opnd, VIR_Inst_GetDest(pInst), &src0OpndInfo);

        /*
            starr dest, src0.x, src1.x
            ==>
            mov dest[src0.x], src1.x
        */
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src1);
        VIR_Inst_SetSource(pInst, VIR_Operand_Src1, gcvNULL);
        VIR_Inst_ChangeSource(pInst, 0, opnd);
        VIR_Inst_ChangeSrcNum(pInst, 1);
    }
    else
    {
        /*
            starr dest, src0.x, src1.x
            ==>
            mov t0.x, src0.x
            mov dest[t0.x], src1.x
        */
        gctUINT     newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
        VIR_SymId   newDstSymId = VIR_INVALID_ID;
        VIR_Instruction *pNewInsertedInst;

        VIR_Shader_AddSymbol(pShader,
                            VIR_SYM_VIRREG,
                            newDstRegNo,
                            VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(src0Opnd)),
                            VIR_STORAGE_UNKNOWN,
                            &newDstSymId);

        /*
            mov new-temp-reg.x, src0
        */
        VIR_Function_AddInstructionBefore(pFunc,
            VIR_OP_MOV,
            VIR_Operand_GetTypeId(src0Opnd),
            pInst,
            gcvTRUE,
            &pNewInsertedInst);

        /* dst */
        opnd = VIR_Inst_GetDest(pNewInsertedInst);
        VIR_Operand_SetSymbol(opnd, pFunc, newDstSymId);
        VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
        VIR_Operand_GetOperandInfo(pNewInsertedInst, opnd, &destInfo);

        _VIR_ReplaceIndexOpnd(opnd, VIR_Inst_GetDest(pInst), &destInfo);

        /* src */
        opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
        VIR_Operand_Copy(opnd, src0Opnd);


        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        opnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src1);
        VIR_Inst_SetSource(pInst, 1, gcvNULL);
        VIR_Inst_ChangeSource(pInst, 0, opnd);
        VIR_Inst_ChangeSrcNum(pInst, 1);
    }
}

/* in dual16 shader, we need to insert precison conv for
   implicit type interpretion (like floatBitsToInt...),
   since spec says "For mediump and lowp, the value is first
   converted to highp floating point and the encoding of
   that value is returned." For example,
   sign t1.fp16, t2
   mov t3.uint32, t1.uint16

   we need to insert a MOV instruction:
   sign t1.fp16, t2
   mov t4.fp32, t1.fp16 (precison conv inserted)
   mov t3.uint32, t4.uint32
*/
VSC_ErrCode
_InsertPrecisionConvInst(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VSC_MM              *pMM)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         usageIdx, defIdx, newDstRegNo = VIR_INVALID_ID;
    gctUINT         i, j;
    VIR_SymId       newDstSymId = VIR_INVALID_ID;
    VIR_Operand     *destOpnd, *srcOpnd;

    VIR_USAGE_KEY       usageKey;
    VIR_USAGE           *pUsage = gcvNULL;
    VIR_DEF             *pDef = gcvNULL;
    VIR_Instruction     *pNewInsertedInst = gcvNULL;
    VIR_Enable          srcEnable = VIR_ENABLE_NONE;
    VIR_OperandInfo     srcInfo;
    VIR_Symbol          *pSym = gcvNULL;
    gctUINT             *defIdxArray = gcvNULL, defCount = 0;
    VIR_Operand         *opnd;

    destOpnd = VIR_Inst_GetDest(pInst);
    if (!destOpnd)
    {
        return errCode;
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
    {
        srcOpnd = VIR_Inst_GetSource(pInst, i);

        if (VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_VIRREG &&
            VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SYMBOL &&
            VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SAMPLER_INDEXING)
        {
            continue;
        }

        pNewInsertedInst = gcvNULL;

        VIR_Operand_GetOperandInfo(pInst, srcOpnd, &srcInfo);

        /* this srcOpnd has different precison than defOpnd, thus it needs conversion */
        if ((VIR_Operand_GetPrecision(srcOpnd) == VIR_PRECISION_HIGH &&
             VIR_Operand_GetPrecision(destOpnd) != VIR_PRECISION_HIGH) ||
             (VIR_Operand_GetPrecision(srcOpnd) != VIR_PRECISION_HIGH &&
             VIR_Operand_GetPrecision(destOpnd) == VIR_PRECISION_HIGH))
        {
            usageKey.pUsageInst = pInst;
            usageKey.pOperand = srcOpnd;
            usageKey.bIsIndexingRegUsage = gcvFALSE;
            usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);

            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
                gcmASSERT(pUsage);

                /* save the defIdx for the udChain, since the update inside the loop
                   will change the udChain */
                defCount = UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain);
                defIdxArray = (gctUINT*)vscMM_Alloc(pMM, defCount * sizeof(gctUINT));
                for (j = 0; j < defCount; j ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, j);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                    defIdxArray[j] = defIdx;
                }

                for (j = 0; j < defCount; j ++)
                {
                    defIdx = defIdxArray[j];
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                    if (pDef->defKey.pDefInst != VIR_INPUT_DEF_INST)
                    {
                        VIR_Operand *defDest = VIR_Inst_GetDest(pDef->defKey.pDefInst);
                        VIR_TypeId ty0 = VIR_Operand_GetTypeId(defDest);
                        VIR_TypeId ty1 = VIR_Operand_GetTypeId(srcOpnd);

                        /* this srcOpnd has different type than its def defDest (implicit conversion)
                           add t1.fp16, t2, t3
                           mov t4.uint32, t1.uint16
                           implicit convert from fp16 to uint16, but fp16 to fp32 and uint16 to uint32
                           conversion behavoir are different, thus we need to add an explicit conversion
                           ==>
                           add t1.fp16, t2, t3
                           mov t5.fp32, t1.fp16
                           mov t4.uint32, t5.uint32 */
                        if((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
                            (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER))
                        {
                            if (pNewInsertedInst)
                            {
                                if (srcEnable & (1 << pDef->defKey.channel))
                                {
                                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pNewInsertedInst,
                                            VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0),
                                            gcvFALSE,
                                            srcInfo.u1.virRegInfo.virReg,
                                            1,
                                            (1 << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);

                                    vscVIR_DeleteUsage(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pInst,
                                            srcOpnd,
                                            gcvFALSE,
                                            srcInfo.u1.virRegInfo.virReg,
                                            1,
                                            (1 << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                                }
                            }
                            else
                            {
                                srcEnable = VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd));

                                /* Add a new-temp-reg number, its type should coming from defDest,
                                   its precison should coming from destOpnd */
                                newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
                                errCode = VIR_Shader_AddSymbol(pShader,
                                                                VIR_SYM_VIRREG,
                                                                newDstRegNo,
                                                                VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(defDest)),
                                                                VIR_STORAGE_UNKNOWN,
                                                                &newDstSymId);
                                pSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

                                /*
                                    mov new-temp-reg (dest precision), srcOpnd
                                */
                                errCode = VIR_Function_AddInstructionBefore(pFunc,
                                    VIR_OP_MOV, VIR_Operand_GetTypeId(srcOpnd),
                                    pInst,
                                    gcvTRUE,
                                    &pNewInsertedInst);

                                /* dst */
                                opnd = VIR_Inst_GetDest(pNewInsertedInst);
                                VIR_Operand_SetSymbol(opnd, pFunc, newDstSymId);
                                VIR_Operand_SetEnable(opnd, VIR_ENABLE_XYZW);
                                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(destOpnd));
                                VIR_Symbol_SetPrecision(pSym, VIR_Operand_GetPrecision(destOpnd));
                                if (VIR_Operand_GetPrecision(srcOpnd) == VIR_PRECISION_HIGH ||
                                    VIR_Operand_GetPrecision(destOpnd) == VIR_PRECISION_HIGH)
                                {
                                    VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                                }

                                vscVIR_AddNewDef(pDuInfo, pNewInsertedInst, newDstRegNo, 1,
                                    VIR_ENABLE_XYZW,
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL, gcvNULL);

                                /* src */
                                opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
                                VIR_Operand_Copy(opnd, srcOpnd);
                                VIR_Operand_SetTypeId(opnd, VIR_Operand_GetTypeId(defDest));

                                if (srcEnable & (1 << pDef->defKey.channel))
                                {
                                    vscVIR_DeleteUsage(pDuInfo,
                                                    pDef->defKey.pDefInst,
                                                    pInst,
                                                    srcOpnd,
                                                    gcvFALSE,
                                                    srcInfo.u1.virRegInfo.virReg,
                                                    1,
                                                    (1 << pDef->defKey.channel),
                                                    VIR_HALF_CHANNEL_MASK_FULL,
                                                    gcvNULL);

                                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pNewInsertedInst,
                                            opnd,
                                            gcvFALSE,
                                            srcInfo.u1.virRegInfo.virReg,
                                            1,
                                            (1 << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                                }
                            }
                        }
                    }
                }

                if (pNewInsertedInst)
                {
                    /* Change operand of current inst to new-temp-reg */
                    gcmASSERT(newDstRegNo != VIR_INVALID_ID);
                    VIR_Operand_SetTempRegister(srcOpnd,
                        pFunc,
                        newDstSymId,
                        VIR_Operand_GetTypeId(srcOpnd));
                    VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XYZW);

                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pNewInsertedInst,
                                            pInst,
                                            srcOpnd,
                                            gcvFALSE,
                                            newDstRegNo,
                                            1,
                                            VIR_ENABLE_XYZW,
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }
            if (defIdxArray)
            {
                vscMM_Free(pMM, defIdxArray);
                defIdxArray = gcvNULL;
            }
        }
    }

    return errCode;
}

/* in dual16 shader, we need to insert CMP for single-t branch
branch.gt.t0 16, src0, src1
branch.gt.t1 16, src0, src1
==>
cmp.gt.t0 dst, src0, src1
cmp.gt.t1 dst, src0, src1
branch.gt 16, dst

!!Todo:: skip this function if hw could support highpvec2 dual16?
*/
VSC_ErrCode
_InsertCMPInst(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VSC_MM              *pMM)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    gctUINT         i;
    VIR_VirRegId regId;
    VIR_SymId    dstSymId;
    VIR_Operand  *srcOpnd = gcvNULL;
    VIR_GENERAL_UD_ITERATOR udIter;
    VIR_OperandInfo         operandInfo;
    VIR_DEF     *pDef = gcvNULL;
    VIR_Swizzle swizzle;
    VIR_TypeId  dstTy;
    VIR_Instruction *newInst = gcvNULL;

    if ((VIR_Inst_GetOpcode(pInst) == VIR_OP_JMPC ||
         VIR_Inst_GetOpcode(pInst) == VIR_OP_JMP_ANY) &&
         VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_32)
    {
        dstTy = VIR_Operand_GetTypeId(VIR_Inst_GetSource(pInst, 0));

            /* add a COMP instruction */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_CMP,
                dstTy,
                pInst,
                gcvTRUE,
                &newInst);
        CHECK_ERROR(errCode, "VIR_Function_AddInstructionBefore failed.");

        for (i = 0; i < VIR_Inst_GetSrcNum(pInst); ++i)
        {
            srcOpnd = VIR_Inst_GetSource(pInst, i);
            swizzle = VIR_Operand_GetSwizzle(srcOpnd);
            VIR_Operand_Copy(VIR_Inst_GetSource(newInst, i), srcOpnd);

            /* update du information*/
            vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, srcOpnd, gcvFALSE, gcvFALSE);

            VIR_Operand_GetOperandInfo(pInst,
                srcOpnd,
                &operandInfo);

            for (pDef = vscVIR_GeneralUdIterator_First(&udIter); pDef != gcvNULL;
                pDef = vscVIR_GeneralUdIterator_Next(&udIter))
            {
                vscVIR_AddNewUsageToDef(pDuInfo,
                    pDef->defKey.pDefInst,
                    newInst,
                    VIR_Inst_GetSource(newInst, i),
                    gcvFALSE,
                    operandInfo.u1.virRegInfo.virReg,
                    1,
                    (1 << pDef->defKey.channel),
                    VIR_HALF_CHANNEL_MASK_FULL,
                    gcvNULL);
            }

            vscVIR_DeleteUsage(pDuInfo,
                VIR_ANY_DEF_INST,
                pInst,
                srcOpnd,
                gcvFALSE,
                operandInfo.u1.virRegInfo.virReg,
                1,
                VIR_Swizzle_2_Enable(swizzle),
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);
        }

        if (VIR_GetTypeFlag(dstTy) & VIR_TYFLAG_ISFLOAT)
        {
            VIR_Const   virConst;
            VIR_Uniform *pImmUniform = gcvNULL;
            VIR_Symbol  *sym = gcvNULL;

            /* float should be putting into uniform */
            virConst.index = VIR_INVALID_ID;
            virConst.type = VIR_TYPE_FLOAT32;
            virConst.value.scalarVal.fValue = 1.0f;
            VIR_Shader_AddInitializedUniform(pShader, &virConst, &pImmUniform, &swizzle);
            /* Set this uniform as operand and set correct swizzle */
            sym = VIR_Shader_GetSymFromId(pShader, pImmUniform->sym);
            VIR_Operand_SetTypeId(VIR_Inst_GetSource(newInst, 2), VIR_TYPE_FLOAT32);
            VIR_Operand_SetOpKind(VIR_Inst_GetSource(newInst, 2), VIR_OPND_SYMBOL);
            VIR_Operand_SetSym(VIR_Inst_GetSource(newInst, 2), sym);
            VIR_Operand_SetSwizzle(VIR_Inst_GetSource(newInst, 2), swizzle);
        }
        else
        {
            VIR_ScalarConstVal imm0;

            imm0.iValue = -1;

            VIR_Operand_SetImmediate(VIR_Inst_GetSource(newInst, 2),
            VIR_TYPE_INT32,
            imm0);

        }
        regId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(
            pShader,
            VIR_SYM_VIRREG,
            regId,
            VIR_Shader_GetTypeFromId(pShader, dstTy),
            VIR_STORAGE_UNKNOWN,
            &dstSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
        VIR_Symbol_SetPrecision(VIR_Shader_GetSymFromId(pShader, dstSymId), VIR_PRECISION_MEDIUM);

        VIR_Operand_SetTempRegister(VIR_Inst_GetDest(newInst),
            pFunc,
            dstSymId,
            dstTy);
        VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), VIR_ENABLE_XYZW);
        VIR_Inst_SetConditionOp(newInst, VIR_Inst_GetConditionOp(pInst));

        vscVIR_AddNewDef(pDuInfo,
            newInst,
            regId,
            1,
            VIR_ENABLE_XYZW,
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL,
            gcvNULL);

        VIR_Inst_SetThreadMode(newInst, VIR_THREAD_D16_DUAL_32);

        /* change the original inst - make sure change everything needed */
        srcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
        VIR_Operand_SetTempRegister(srcOpnd,
            pFunc,
            dstSymId,
            dstTy);
        VIR_Operand_SetIsConstIndexing(srcOpnd, gcvFALSE);
        VIR_Operand_SetRelAddrMode(srcOpnd, 0);
        VIR_Operand_SetMatrixConstIndex(srcOpnd, 0);
        VIR_Operand_SetRelAddrLevel(srcOpnd, 0);
        VIR_Operand_SetRelIndex(srcOpnd, 0);
        VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);

        VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XYZW);
        VIR_Inst_SetConditionOp(pInst, VIR_COP_NOT_ZERO);
        VIR_Inst_ChangeSrcNum(pInst, 1);
        if (VIR_Inst_GetSrcNum(pInst) == 2)
        {
            VIR_Inst_FreeSource(pInst, VIR_Operand_Src1);
        }

        vscVIR_AddNewUsageToDef(pDuInfo,
            newInst,
            pInst,
            srcOpnd,
            gcvFALSE,
            regId,
            1,
            VIR_ENABLE_XYZW,
            VIR_HALF_CHANNEL_MASK_FULL,
            gcvNULL);

        VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_16);
    }

    return errCode;
}

static VSC_ErrCode _VIR_MergeICASTP(
    IN VIR_DEF_USAGE_INFO  *du_info,
    IN VIR_Shader* shader,
    IN VSC_OPTN_FCPOptions *options,
    IN VIR_Instruction* icast,
    IN VSC_MM*     pMM,
    IN VIR_Dumper* dumper,
    IN gctBOOL* pInvalidCfg
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VIR_Swizzle icast_src0_swizzle;
    VIR_Enable icast_enable, icast_src0_enable;
    VIR_Operand *icast_dest, *icast_src0, *usage_opnd_of_icast;
    VIR_Instruction* usage_inst_of_icast;
    VIR_OperandInfo icast_dest_info, icast_src0_info;
    gctBOOL bIsIndexingRegUsage;
    gctBOOL     trace = VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetTrace(options), VSC_OPTN_FCPOptions_TRACE_ICAST);

    if(trace)
    {
        VIR_LOG(dumper, "\nicast instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, icast);
        VIR_LOG_FLUSH(dumper);
    }

    icast_dest = VIR_Inst_GetDest(icast);
    icast_enable = VIR_Operand_GetEnable(icast_dest);
    icast_src0 = VIR_Inst_GetSource(icast, 0);
    VIR_Operand_GetOperandInfo(icast, icast_src0, &icast_src0_info);
    VIR_Operand_GetOperandInfo(icast, icast_dest, &icast_dest_info);
    icast_src0_swizzle = VIR_Operand_GetSwizzle(icast_src0);
    icast_src0_enable = VIR_Swizzle_2_Enable(icast_src0_swizzle);

    /* check if the icast's dest is uniquely used by a vx_inst, and vx_inst has only this icast as def */
    if (vscVIR_DoesDefInstHaveUniqueUsageInst(du_info, icast, gcvTRUE, &usage_inst_of_icast, &usage_opnd_of_icast, &bIsIndexingRegUsage) &&
        vscVIR_IsUniqueDefInstOfUsageInst(du_info, usage_inst_of_icast, usage_opnd_of_icast, bIsIndexingRegUsage, icast, gcvNULL) &&
        VIR_OPCODE_isVX(VIR_Inst_GetOpcode(usage_inst_of_icast)))
    {
        VSC_HASH_TABLE* def_inst_set0 = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 64);

        /* collect the def of the src0 of the icast inst, and delete the usage between icast's src0's def and icast's src0 */
        {
            VIR_GENERAL_UD_ITERATOR inst_ud_iter;
            VIR_DEF* def;

            vscVIR_InitGeneralUdIterator(&inst_ud_iter, du_info, icast, icast_src0, gcvFALSE, gcvFALSE);
            for(def = vscVIR_GeneralUdIterator_First(&inst_ud_iter); def != gcvNULL;
                def = vscVIR_GeneralUdIterator_Next(&inst_ud_iter))
            {
                VIR_Instruction* def_inst = def->defKey.pDefInst;
                vscHTBL_DirectSet(def_inst_set0, (void*)def_inst, gcvNULL);
            }
        }

        /* replace def_vx_inst's dest and update usage */
        {
            VIR_Instruction* vx_inst = usage_inst_of_icast;
            VIR_Operand* icast_dest_usage = usage_opnd_of_icast;
            VIR_Swizzle mapping_swizzle0 = VIR_Enable_GetMappingSwizzle(icast_enable, icast_src0_swizzle);
            VIR_Swizzle icast_dest_usage_swizzle = VIR_Operand_GetSwizzle(icast_dest_usage);
            VIR_Swizzle vx_inst_src0_swizzle = VIR_Swizzle_ApplyMappingSwizzle(icast_dest_usage_swizzle, mapping_swizzle0);
            VIR_Enable vx_inst_src0_enable = VIR_Swizzle_2_Enable(vx_inst_src0_swizzle);
            VIR_Enable icast_dest_usage_enable = VIR_Swizzle_2_Enable(icast_dest_usage_swizzle);
            VIR_Operand* vx_inst_src0;
            gctUINT srcNo;

            if(trace)
            {
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, vx_inst);
                VIR_LOG_FLUSH(dumper);
            }

            vscVIR_DeleteUsage(du_info, VIR_ANY_DEF_INST, vx_inst,
                               icast_dest_usage, gcvFALSE, icast_dest_info.u1.virRegInfo.virReg, 1,
                               icast_dest_usage_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            VIR_Function_DupOperand(func, icast_src0, &vx_inst_src0);
            VIR_Operand_SetSwizzle(vx_inst_src0, vx_inst_src0_swizzle);

            /* find and replace the vx_inst operand with icast_src0 */
            for (srcNo = 0; srcNo < VIR_Inst_GetSrcNum(vx_inst); srcNo++)
            {
                if (icast_dest_usage ==  VIR_Inst_GetSource(vx_inst, srcNo))
                {
                    VIR_Inst_FreeSource(vx_inst, srcNo);
                    VIR_Inst_SetSource(vx_inst, srcNo, vx_inst_src0);
                    break;
                }
            }
            gcmASSERT(srcNo != VIR_Inst_GetSrcNum(vx_inst)); /* must find the operand */

            /* add the use of vx_inst_src0 to the def of icast_src0 */
            {
                VSC_HASH_ITERATOR def_inst_set_iter;
                VSC_DIRECT_HNODE_PAIR def_inst_set_pair;
                vscHTBLIterator_Init(&def_inst_set_iter, def_inst_set0);
                for(def_inst_set_pair = vscHTBLIterator_DirectFirst(&def_inst_set_iter);
                    IS_VALID_DIRECT_HNODE_PAIR(&def_inst_set_pair); def_inst_set_pair = vscHTBLIterator_DirectNext(&def_inst_set_iter))
                {
                    VIR_Instruction* def_inst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&def_inst_set_pair);
                    VIR_Enable def_dest_enable;
                    if(!VIR_IS_IMPLICIT_DEF_INST(def_inst))
                    {
                        VIR_Operand* def_dest = VIR_Inst_GetDest(def_inst);
                        def_dest_enable = VIR_Operand_GetEnable(def_dest);
                    }
                    else
                    {
                        def_dest_enable = VIR_ENABLE_XYZW;
                    }

                    if (def_dest_enable & vx_inst_src0_enable)
                    {
                        vscVIR_AddNewUsageToDef(du_info, def_inst, vx_inst, vx_inst_src0, gcvFALSE,
                                                icast_src0_info.u1.virRegInfo.virReg, 1, (VIR_Enable)(def_dest_enable & vx_inst_src0_enable),
                                                VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                    }
                }
            }
            if(trace)
            {
                VIR_LOG(dumper, "into:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, vx_inst);
                VIR_LOG_FLUSH(dumper);
            }
        }

        /* remove the use of icast_src0 */
        {
            vscVIR_DeleteUsage(du_info, VIR_ANY_DEF_INST, icast,
                               icast_src0, gcvFALSE, icast_src0_info.u1.virRegInfo.virReg, 1,
                               icast_src0_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        /* remove the def of icast */
        {
            vscVIR_DeleteDef(du_info, icast, icast_dest_info.u1.virRegInfo.virReg,
                             1, icast_enable, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
        }
        VIR_Pass_RemoveInstruction(func, icast, pInvalidCfg);
        vscHTBL_Destroy(def_inst_set0);
        return errCode;
    }

    /* otherwise change icast to mov */
    VIR_Inst_SetOpcode(icast, VIR_OP_MOV);
    return errCode;
}

static VSC_ErrCode _VIR_MergeICASTD(
    IN VIR_DEF_USAGE_INFO  *du_info,
    IN VIR_Shader* shader,
    IN VSC_OPTN_FCPOptions *options,
    IN VIR_Instruction* icast,
    IN VSC_MM*     pMM,
    IN VIR_Dumper* dumper,
    IN gctBOOL* pInvalidCfg
    )
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_Function* func = VIR_Shader_GetCurrentFunction(shader);
    VIR_Enable icast_enable;
    VIR_Operand *icast_dest, *icast_src0;
    VIR_OperandInfo icast_dest_info, icast_src0_info;
    gctBOOL     trace = VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetTrace(options), VSC_OPTN_FCPOptions_TRACE_ICAST);

    VIR_Instruction * def_vx_inst = gcvNULL;

    if(trace)
    {
        VIR_LOG(dumper, "\nicast instruction:\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Inst_Dump(dumper, icast);
        VIR_LOG_FLUSH(dumper);
    }

    icast_dest = VIR_Inst_GetDest(icast);
    icast_enable = VIR_Operand_GetEnable(icast_dest);
    icast_src0 = VIR_Inst_GetSource(icast, 0);
    VIR_Operand_GetOperandInfo(icast, icast_src0, &icast_src0_info);
    VIR_Operand_GetOperandInfo(icast, icast_dest, &icast_dest_info);

    /* check if the icast's source is uniquely defined by a vx_inst and vx_inst has only this icast as use inst */
    if (vscVIR_DoesUsageInstHaveUniqueDefInst(du_info, icast, VIR_Inst_GetSource(icast, 0), gcvFALSE, &def_vx_inst) &&
        vscVIR_IsUniqueUsageInstOfDefInst(du_info, def_vx_inst, icast, gcvNULL, gcvFALSE, gcvNULL, gcvNULL, gcvNULL) &&
        VIR_OPCODE_isVX(VIR_Inst_GetOpcode(def_vx_inst)))
    {
        /* replace def_vx_inst's dest and update usage */
        /*  vx_inst  <ty1> vdest, src0, src1, ...
         *  icast    <ty2> dest1, <ty1> vdest
         *
         *  ==>
         *
         *  vx_inst  <ty2> dest1, src0, src1, ...
         */
        {
            VIR_Instruction* vx_inst = def_vx_inst;
            VIR_Operand*     vx_inst_dest = VIR_Inst_GetDest(vx_inst);
            VIR_Operand*     vx_inst_dest_new;
            VIR_OperandInfo  vx_inst_dest_info;
            gctUINT8         channel;
            gctUINT          firstRegNo, regNoRange;

            if(trace)
            {
                VIR_LOG(dumper, "merges with instruction:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, vx_inst);
                VIR_LOG_FLUSH(dumper);
            }

            VIR_Operand_GetOperandInfo(vx_inst, vx_inst_dest, &vx_inst_dest_info);

            vscVIR_DeleteUsage(du_info, vx_inst, icast,
                               icast_src0, gcvFALSE, icast_src0_info.u1.virRegInfo.virReg, 1,
                               VIR_Operand_GetEnable(vx_inst_dest), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            VIR_Function_DupOperand(func, icast_dest, &vx_inst_dest_new);

            /* remove the def of vx_inst */
            {
                vscVIR_DeleteDef(du_info, def_vx_inst, vx_inst_dest_info.u1.virRegInfo.virReg,
                                 1, VIR_Operand_GetEnable(vx_inst_dest), VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
            }

            /* replace the vx_inst's dest with icast's dest  */
            VIR_Inst_FreeDest(vx_inst);
            VIR_Inst_SetDest(vx_inst, vx_inst_dest_new);

            vscVIR_AddNewDef(du_info, vx_inst, icast_dest_info.u1.virRegInfo.virReg, 1,
                icast_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL, gcvNULL);

            firstRegNo  = icast_dest_info.u1.virRegInfo.startVirReg;
            regNoRange  = 1;

            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                VIR_GENERAL_DU_ITERATOR duIter;
                VIR_USAGE*              pUsage;

                if (!VSC_UTILS_TST_BIT(icast_enable, channel))
                {
                    continue;
                }

                vscVIR_InitGeneralDuIterator(&duIter,
                                             du_info,
                                             icast,
                                             icast_dest_info.u1.virRegInfo.virReg,
                                             channel,
                                             gcvFALSE);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter); pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    vscVIR_AddNewUsageToDef(du_info,
                                            vx_inst,
                                            pUsage->usageKey.pUsageInst,
                                            pUsage->usageKey.pOperand,
                                            pUsage->usageKey.bIsIndexingRegUsage,
                                            firstRegNo,
                                            regNoRange,
                                            (1 << channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);

                    vscVIR_DeleteUsage(du_info,
                                       icast,
                                       pUsage->usageKey.pUsageInst,
                                       pUsage->usageKey.pOperand,
                                       gcvFALSE,
                                       firstRegNo,
                                       regNoRange,
                                       (1 << channel),
                                       VIR_HALF_CHANNEL_MASK_FULL,
                                       gcvNULL);
                }
            }

            if(trace)
            {
                VIR_LOG(dumper, "into:\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Inst_Dump(dumper, vx_inst);
                VIR_LOG_FLUSH(dumper);
            }

            VIR_Pass_RemoveInstruction(func, icast, pInvalidCfg);
        }
        return errCode;
    }
    /* otherwise change icast to mov */
    VIR_Inst_SetOpcode(icast, VIR_OP_MOV);
    return errCode;
}

/* For chip which use USC instead of FIFO for buffering, HW atomic instruction need to use
 * dest register to buffer the value of source2, for some unspecified reason, it has
 * special requirement for source2 swizzle in order to copy source2 to dest temp register
 * correctly. Q1: do we need to consider the dest.enable to get the correct swizzle???
 * Q2: why do we get the vector if a scaler is needed?
 */
static VSC_ErrCode
_ConvSrc2SwizzleForAtomInst(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand        *pSrc2Opnd = VIR_Inst_GetSource(pInst, 2);
    VIR_Swizzle         src2Swizzle;

    if (pSrc2Opnd &&
        VIR_OPCODE_hasDest(opCode) &&
        VIR_OPCODE_isAtom(opCode))
    {
        src2Swizzle = VIR_Operand_GetSwizzle(pSrc2Opnd);

        if (VIR_OPCODE_isAtomCmpxChg(opCode))
        {
            VIR_Swizzle_SetChannel(src2Swizzle, 2, VIR_Swizzle_GetChannel(src2Swizzle, 0));
            VIR_Swizzle_SetChannel(src2Swizzle, 3, VIR_Swizzle_GetChannel(src2Swizzle, 1));
        }
        else
        {
            VIR_Swizzle_SetChannel(src2Swizzle, 1, VIR_Swizzle_GetChannel(src2Swizzle, 0));
            VIR_Swizzle_SetChannel(src2Swizzle, 2, VIR_Swizzle_GetChannel(src2Swizzle, 0));
            VIR_Swizzle_SetChannel(src2Swizzle, 3, VIR_Swizzle_GetChannel(src2Swizzle, 0));
        }

        VIR_Operand_SetSwizzle(pSrc2Opnd, src2Swizzle);
    }

    return errCode;
}

extern gctINT
_ConvType(
    IN VIR_TypeId Ty
    );

static VSC_ErrCode _ConvEvisInstForShader(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Shader         *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator  inst_iter;
        VIR_Instruction  *pInst;
        VIR_Instruction  *pMovConstBorderInst = gcvNULL;
        VIR_VirRegId      constBorderColorRegId = VIR_INVALID_ID;  /* constant border color temp reg id */

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL; pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
            VIR_Operand        *pSrc2Opnd = VIR_Inst_GetSource(pInst, 2);
            VIR_DEF_USAGE_INFO *pDuInfo = pPassWorker->pDuInfo;
            VIR_Instruction    *pNewInst = gcvNULL;
            VIR_VirRegId        regId;                     /* newly created temp reg id */
            VIR_SymId           regSymId;
            VIR_TypeId          regTypeId;
            /*
            ** For some EVIS instructions, HW doesn't decode src2's swizzle if src2 it is not a IMMEDIATE,
            ** so we insert a MOV instruction whose enable is XYZW to replace the src2.
            */
            if (pSrc2Opnd                       &&
                VIR_OPCODE_isVX(opCode)         &&
                opCode != VIR_OP_VX_IACCSQ      &&
                opCode != VIR_OP_VX_LERP        &&
                opCode != VIR_OP_VX_MULSHIFT    &&
                opCode != VIR_OP_VX_BILINEAR    &&
                opCode != VIR_OP_VX_ATOMICADD   &&
                !VIR_OPCODE_isImgRelated(opCode)
                )
            {
                VIR_TypeId       src2TypeId = VIR_Operand_GetTypeId(pSrc2Opnd);
                VIR_Swizzle      src2Swizzle = VIR_Operand_GetSwizzle(pSrc2Opnd);

                if ((VIR_Operand_isVirReg(pSrc2Opnd) || VIR_Operand_isSymbol(pSrc2Opnd) || VIR_Operand_isConst(pSrc2Opnd))
                    &&
                    src2Swizzle != VIR_SWIZZLE_XYZW)
                {
                    VIR_Operand             * pOpnd = gcvNULL;
                    VIR_OperandInfo           srcInfo;
                    VIR_GENERAL_UD_ITERATOR   udIter;
                    VIR_DEF*                  pDef;
                    VIR_Symbol              * src2Sym = VIR_Operand_isSymbol(pSrc2Opnd) ?
                                                            VIR_Operand_GetSymbol(pSrc2Opnd) : gcvNULL;
                    gctBOOL                   useConstBorderColor = gcvFALSE;

                    /* special optimization for constant border color */
                    if (opCode == VIR_OP_VX_CLAMP && src2Sym && VIR_Symbol_isUniform(src2Sym) &&
                        VIR_Symbol_GetUniformKind(src2Sym) == VIR_UNIFORM_CONST_BORDER_VALUE)
                    {
                        useConstBorderColor = gcvTRUE;
                        if (pMovConstBorderInst != gcvNULL)
                        {
                            /* already created mov constant border color Inst, use it */
                            /* Update the SRC2 of EVIS instruction. */
                            VIR_Operand_Copy(pSrc2Opnd, VIR_Inst_GetDest(pMovConstBorderInst));
                            VIR_Operand_SetLvalue(pSrc2Opnd, gcvFALSE);
                            VIR_Operand_SetSwizzle(pSrc2Opnd, VIR_SWIZZLE_XYZW);

                            /* update the usage of new temp */
                            vscVIR_AddNewUsageToDef(pDuInfo, pMovConstBorderInst, pInst,
                                                    pSrc2Opnd,
                                                    gcvFALSE, constBorderColorRegId, 1, VIR_ENABLE_XYZW,
                                                    VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                            continue;
                        }
                    }
                    /* Create a temp to hold the src2. */
                    regTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(src2TypeId), 4, 1);
                    regId = VIR_Shader_NewVirRegId(pShader, 1);
                    errCode = VIR_Shader_AddSymbol(pShader,
                                                   VIR_SYM_VIRREG,
                                                   regId,
                                                   VIR_Shader_GetTypeFromId(pShader, regTypeId),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &regSymId);
                    ON_ERROR(errCode, "Add symbol failed");

                    /* Insert a MOV. */
                    errCode = VIR_Function_AddInstructionBefore(
                                    func,
                                    VIR_OP_MOV,
                                    regTypeId,
                                    useConstBorderColor ? VIR_Function_GetInstStart(func) : pInst,
                                    gcvTRUE,
                                    &pNewInst);
                    ON_ERROR(errCode, "Insert instruction failed");

                    if (useConstBorderColor)
                    {
                        constBorderColorRegId = regId;
                        pMovConstBorderInst = pNewInst;
                    }

                    /* Set DEST. */
                    pOpnd = VIR_Inst_GetDest(pNewInst);
                    VIR_Operand_SetTempRegister(pOpnd,
                                                func,
                                                regSymId,
                                                regTypeId);
                    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XYZW);

                    /* Set SRC0. */
                    pOpnd = VIR_Inst_GetSource(pNewInst, 0);
                    VIR_Operand_Copy(pOpnd, pSrc2Opnd);

                    /* update du info for new instruction */
                    vscVIR_AddNewDef(pDuInfo,
                                     pNewInst,
                                     regId,
                                     1,
                                     VIR_ENABLE_XYZW,
                                     VIR_HALF_CHANNEL_MASK_FULL,
                                     gcvNULL,
                                     gcvNULL);

                    /* find the def of pOpnd and update it usage */
                    VIR_Operand_GetOperandInfo(pInst, pSrc2Opnd, &srcInfo);

                    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pInst, pSrc2Opnd, gcvFALSE, gcvFALSE);

                    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                         pDef != gcvNULL;
                         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                    {
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                                pDef->defKey.pDefInst,
                                                pNewInst,
                                                pOpnd,
                                                gcvFALSE,
                                                srcInfo.u1.virRegInfo.virReg,
                                                1,
                                                1 << pDef->defKey.channel,
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }

                    /* Update the SRC2 of EVIS instruction. */
                    VIR_Operand_Copy(pSrc2Opnd, VIR_Inst_GetDest(pNewInst));
                    VIR_Operand_SetLvalue(pSrc2Opnd, gcvFALSE);
                    VIR_Operand_SetSwizzle(pSrc2Opnd, VIR_SWIZZLE_XYZW);
                    /* This src must be strict, which means it is not component-wise */
                    VIR_Operand_SetFlag(pSrc2Opnd, VIR_OPNDFLAG_RESTRICT);

                    /* update the usage of new temp */
                    vscVIR_AddNewUsageToDef(pDuInfo, pNewInst, pInst,
                                            pSrc2Opnd,
                                            gcvFALSE, regId, 1, VIR_ENABLE_XYZW,
                                            VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                }
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateLocalInvocationIndex(
    VIR_Shader*         pShader,
    gctBOOL*            pChanged
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Symbol*         pLocalInvocationIndex = gcvNULL;
    gctBOOL             bChanged = gcvTRUE;

    if (!VIR_Shader_CalcLocalInvocationIndex(pShader))
    {
        return errCode;
    }

    pLocalInvocationIndex = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_LOCALINVOCATIONINDEX);
    errCode = VIR_Shader_GenInvocationIndex(pShader,
                                            VIR_Shader_GetMainFunction(pShader),
                                            pLocalInvocationIndex,
                                            gcvNULL,
                                            gcvTRUE);
    ON_ERROR(errCode, "Calcualte local invocation index.");

    VIR_Shader_ClrFlagExt1(pShader, VIR_SHFLAG_EXT1_CALC_LOCAL_INVOCATION_INDEX);
    if (pChanged)
    {
        *pChanged |= bChanged;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_ConvSingleTemp256Src(
    VIR_DEF_USAGE_INFO* pDuInfo,
    VIR_Shader*         pShader,
    VIR_Function*       pFunc,
    VIR_Instruction*    pInst,
    gctUINT             srcIdx)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Operand*        pOrigSrc = VIR_Inst_GetSource(pInst, srcIdx);
    VIR_Instruction*    pNewInst = gcvNULL;
    VIR_Operand*        pNewOpnd = gcvNULL;
    VIR_VirRegId        destRegId = VIR_INVALID_ID;
    VIR_SymId           destSymId = VIR_INVALID_ID;
    VIR_TypeId          typeId = VIR_Operand_GetTypeId(pOrigSrc);
    VIR_Enable          enable = VIR_TypeId_Conv2Enable(typeId);

    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                VIR_OP_MOV,
                                                typeId,
                                                pInst,
                                                gcvTRUE,
                                                &pNewInst);
    ON_ERROR(errCode, "Add MOV instruction failed.");

    /* Add a new vreg. */
    destRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   destRegId,
                                   VIR_Shader_GetTypeFromId(pShader, typeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &destSymId);
    ON_ERROR(errCode, "Add vreg failed.");

    /* Set DEST. */
    pNewOpnd = VIR_Inst_GetDest(pNewInst);
    VIR_Operand_SetTempRegister(pNewOpnd,
                                pFunc,
                                destSymId,
                                typeId);
    VIR_Operand_SetEnable(pNewOpnd, enable);

    /* Set SRC0. */
    pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
    VIR_Operand_Copy(pNewOpnd, pOrigSrc);

    /* Change the original source to the new virReg. */
    VIR_Operand_Copy(pOrigSrc, VIR_Inst_GetDest(pNewInst));
    VIR_Operand_Change2Src_WShift(pOrigSrc);

    /* Update DU info. */
    vscVIR_AddNewDef(pDuInfo,
                     pNewInst,
                     destRegId,
                     1,
                     enable,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     gcvNULL,
                     gcvNULL);

    vscVIR_AddNewUsageToDef(pDuInfo,
                            pNewInst,
                            pInst,
                            pOrigSrc,
                            gcvFALSE,
                            destRegId,
                            1,
                            enable,
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);
OnError:
    return errCode;
}

static VSC_ErrCode
_ConvTemp256Srcs(
    VIR_DEF_USAGE_INFO* pDuInfo,
    VIR_Shader*         pShader,
    VIR_Function*       pFunc,
    VIR_Instruction*    pInst)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    gctBOOL             bIsSrc0Src1Temp256 = VIR_OPCODE_Src0Src1Temp256(VIR_Inst_GetOpcode(pInst));
    gctUINT             srcIdx;
    VIR_Operand*        pHigherOpnd = bIsSrc0Src1Temp256 ? VIR_Inst_GetSource(pInst, 0) : VIR_Inst_GetSource(pInst, 1);
    VIR_Operand*        pLowerOpnd = bIsSrc0Src1Temp256 ? VIR_Inst_GetSource(pInst, 1) : VIR_Inst_GetSource(pInst, 2);
    VIR_OperandInfo     opndInfo;

    /* Check higher operand. */
    srcIdx = bIsSrc0Src1Temp256 ? 0 : 1;
    VIR_Operand_GetOperandInfo(pInst, pHigherOpnd, &opndInfo);
    if (!opndInfo.isVreg)
    {
        errCode = _ConvSingleTemp256Src(pDuInfo, pShader, pFunc, pInst, srcIdx);
        ON_ERROR(errCode, "Convert single temp 256 source.");
    }

    /* Check lower operand. */
    srcIdx = bIsSrc0Src1Temp256 ? 1 : 2;
    VIR_Operand_GetOperandInfo(pInst, pLowerOpnd, &opndInfo);
    if (!opndInfo.isVreg)
    {
        errCode = _ConvSingleTemp256Src(pDuInfo, pShader, pFunc, pInst, srcIdx);
        ON_ERROR(errCode, "Convert single temp 256 source.");
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_ProcessModOrder(
    VIR_DEF_USAGE_INFO* pDuInfo,
    VIR_Shader*         pShader,
    VIR_Function*       pFunc,
    VIR_Instruction*    pInst)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_TypeId          opndTypeId = VIR_TYPE_UNKNOWN;
    VIR_OperandKind     opndKind;
    VIR_ModifierOrder   modOrder = VIR_MODORDER_NONE;
    VIR_ModifierOrder   hwModOrder = VIR_MODORDER_ABS_NEG;
    gctBOOL             bHasAbs = gcvFALSE, bHasNeg = gcvFALSE;
    gctUINT             i;

    /*
    ** 1) Process the immediate operand.
    ** 2) HW takes absolute values first then takes negative, so we can only accept this order,
    **    for the order NEG-ABS, we need to insert a MOV to handle NEG first.
    */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        pOpnd = VIR_Inst_GetSource(pInst, i);
        if (pOpnd == gcvNULL)
        {
            continue;
        }

        opndTypeId = VIR_Operand_GetTypeId(pOpnd);
        opndKind = (VIR_OperandKind)VIR_Operand_GetOpKind(pOpnd);
        modOrder = (VIR_ModifierOrder)VIR_Operand_GetModOrder(pOpnd);
        bHasAbs = VIR_Operand_GetModifier(pOpnd) & VIR_MOD_ABS;
        bHasNeg = VIR_Operand_GetModifier(pOpnd) & VIR_MOD_NEG;

        if (opndKind == VIR_OPND_IMMEDIATE)
        {
            /* Skip none modifier. */
            if (!bHasAbs && !bHasNeg)
            {
                continue;
            }

            if (bHasAbs && bHasNeg)
            {
                if (modOrder == VIR_MODORDER_ABS_NEG)
                {
                    VIR_ScalarConstVal_GetAbs(opndTypeId,
                                              &VIR_Operand_GetScalarImmediate(pOpnd),
                                              &VIR_Operand_GetScalarImmediate(pOpnd));
                    VIR_ScalarConstVal_GetNeg(opndTypeId,
                                              &VIR_Operand_GetScalarImmediate(pOpnd),
                                              &VIR_Operand_GetScalarImmediate(pOpnd));
                }
                else if (modOrder == VIR_MODORDER_NEG_ABS)
                {
                    VIR_ScalarConstVal_GetNeg(opndTypeId,
                                              &VIR_Operand_GetScalarImmediate(pOpnd),
                                              &VIR_Operand_GetScalarImmediate(pOpnd));
                    VIR_ScalarConstVal_GetAbs(opndTypeId,
                                              &VIR_Operand_GetScalarImmediate(pOpnd),
                                              &VIR_Operand_GetScalarImmediate(pOpnd));
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }
            else if (bHasAbs)
            {
                VIR_ScalarConstVal_GetAbs(opndTypeId,
                                          &VIR_Operand_GetScalarImmediate(pOpnd),
                                          &VIR_Operand_GetScalarImmediate(pOpnd));
            }
            else
            {
                gcmASSERT(bHasNeg);
                VIR_ScalarConstVal_GetNeg(opndTypeId,
                                          &VIR_Operand_GetScalarImmediate(pOpnd),
                                          &VIR_Operand_GetScalarImmediate(pOpnd));
            }

            VIR_Operand_SetModOrder(pOpnd, VIR_MODORDER_NONE);
            if (bHasAbs)
            {
                VIR_Operand_SetModifier(pOpnd, VIR_MOD_ABS ^ VIR_Operand_GetModifier(pOpnd));
            }
            if (bHasNeg)
            {
                VIR_Operand_SetModifier(pOpnd, VIR_MOD_NEG ^ VIR_Operand_GetModifier(pOpnd));
            }
        }
        else if (opndKind == VIR_OPND_SYMBOL)
        {
            VIR_Instruction*    pNewInst = gcvNULL;
            VIR_Operand*        pNewOpnd = gcvNULL;
            VIR_VirRegId        regId;                     /* newly created temp reg id */
            VIR_SymId           regSymId;
            VIR_TypeId          regTypeId;
            VIR_Enable          newEnable;
            VIR_Swizzle         newSwizzle;
            VIR_OperandInfo     srcOpndInfo;

            /* Skip only one modifer. */
            if (!bHasAbs || !bHasNeg)
            {
                continue;
            }

            /* Skip the modifier order that HW can accept. */
            if (modOrder == hwModOrder)
            {
                continue;
            }

            VIR_Operand_GetOperandInfo(pInst, pOpnd, &srcOpndInfo);

            /* Add a new vreg. */
            regId = VIR_Shader_NewVirRegId(pShader, 1);
            regTypeId = VIR_Operand_GetTypeId(pOpnd);
            newEnable = VIR_TypeId_Conv2Enable(regTypeId);
            newSwizzle = VIR_TypeId_Conv2Swizzle(regTypeId);
            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           regId,
                                           VIR_Shader_GetTypeFromId(pShader, regTypeId),
                                           VIR_STORAGE_UNKNOWN,
                                           &regSymId);
            ON_ERROR(errCode, "Fail to add symbol.");

            if (modOrder == VIR_MODORDER_ABS_NEG || modOrder == VIR_MODORDER_NEG_ABS)
            {
                /* Insert a MOV, newReg, src.abs/src.neg */
                errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                            VIR_OP_MOV,
                                                            regTypeId,
                                                            pInst,
                                                            gcvTRUE,
                                                            &pNewInst);
                pNewOpnd = VIR_Inst_GetDest(pNewInst);
                VIR_Operand_SetSymbol(pNewOpnd, pFunc, regSymId);
                VIR_Operand_SetEnable(pNewOpnd, newEnable);

                pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                VIR_Operand_Copy(pNewOpnd, pOpnd);
                if (modOrder == VIR_MODORDER_ABS_NEG)
                {
                    VIR_Operand_SetModifier(pNewOpnd, VIR_MOD_NEG ^ VIR_Operand_GetModifier(pNewOpnd));
                }
                else
                {
                    VIR_Operand_SetModifier(pNewOpnd, VIR_MOD_ABS ^ VIR_Operand_GetModifier(pNewOpnd));
                }

                /* Add def. */
                vscVIR_AddNewDef(pDuInfo,
                                 pNewInst,
                                 regId,
                                 1,
                                 newEnable,
                                 VIR_HALF_CHANNEL_MASK_FULL,
                                 gcvNULL,
                                 gcvNULL);
                /* Add usage. */
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        VIR_ANY_DEF_INST,
                                        pNewInst,
                                        pNewOpnd,
                                        gcvFALSE,
                                        regId,
                                        1,
                                        newEnable,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
                if (srcOpndInfo.isVreg)
                {
                    /* Delete the old usage. */
                    vscVIR_DeleteUsage(pDuInfo,
                                       VIR_ANY_DEF_INST,
                                       pInst,
                                       pOpnd,
                                       gcvFALSE,
                                       srcOpndInfo.u1.virRegInfo.virReg,
                                       1,
                                       VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd)),
                                       VIR_HALF_CHANNEL_MASK_FULL,
                                       gcvNULL);
                }

                /* Change the original operand from src.abs.neg to newReg.neg/newReg.abs */
                VIR_Operand_SetSymbol(pOpnd, pFunc, regSymId);
                VIR_Operand_SetSwizzle(pOpnd, newSwizzle);
                if (modOrder == VIR_MODORDER_ABS_NEG)
                {
                    VIR_Operand_SetModifier(pOpnd, VIR_MOD_ABS ^ VIR_Operand_GetModifier(pOpnd));
                }
                else
                {
                    VIR_Operand_SetModifier(pOpnd, VIR_MOD_NEG ^ VIR_Operand_GetModifier(pOpnd));
                }

                /* Add usage. */
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pNewInst,
                                        pInst,
                                        pOpnd,
                                        gcvFALSE,
                                        regId,
                                        1,
                                        newEnable,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            VIR_Operand_SetModOrder(pOpnd, VIR_MODORDER_NONE);
        }
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PostMCCleanup)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_FCP;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PostMCCleanup)
{
    return gcvTRUE;
}

/*********************************************
* vscVIR_PostMCCleanup
* After MC level compile, perform
  1) replace LDARR/STARR
  2) dual16 shader, we need to patch the code for some cases:
  2.1) insert precison conv for implicit type interpretion
  3) fix the source2 swizzle for ATOM instructions.
  4) Create a uniform to save the threadCount and insert a MOD to calculate the globalIndex for private memory.
  5) Create a full-channel-used temp for the SRC2 of a VX instruction if needed.
   ...
*********************************************/
VSC_ErrCode vscVIR_PostMCCleanup(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO  *pDuInfo = pPassWorker->pDuInfo;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VSC_OPTN_FCPOptions *pOptions = (VSC_OPTN_FCPOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    VSC_MM*             pMM = pPassWorker->basePassWorker.pMM;
    gctBOOL             bRAEnabled = *(gctBOOL*)pPassWorker->basePassWorker.pPassSpecificData;
    VSC_HW_CONFIG       *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL             bInvalidCfg = gcvFALSE;
    gctBOOL             bInvalidDu = gcvFALSE;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode opCode = VIR_Inst_GetOpcode(inst);

            /* disable this when RA enabled for now, since DU and RA has not supported indexed opnd yet */
            if (!bRAEnabled)
            {
                if (opCode == VIR_OP_LDARR)
                {
                    _VIR_ReplaceLDARR(pShader, func, pDuInfo, inst, &bInvalidCfg);
                }

                if (opCode == VIR_OP_STARR)
                {
                    _VIR_ReplaceSTARR(pShader, func, pDuInfo, inst);
                }
            }

            if (VSC_UTILS_MASK(VSC_OPTN_FCPOptions_GetOPTS(pOptions), VSC_OPTN_FCPOptions_OPTS_ICAST))
            {
                if (opCode == VIR_OP_VX_ICASTP)
                {
                    _VIR_MergeICASTP(pDuInfo, pShader, pOptions, inst, pMM, pDumper, &bInvalidCfg);
                }
                else if (opCode == VIR_OP_VX_ICASTD)
                {
                    _VIR_MergeICASTD(pDuInfo, pShader, pOptions, inst, pMM, pDumper, &bInvalidCfg);
                }
            }

            if (VIR_Shader_isDual16Mode(pShader))
            {
                errCode = _InsertPrecisionConvInst(pShader, func, inst, pDuInfo, pMM);
                ON_ERROR(errCode, "Insert precision conversion inst");

                errCode = _InsertCMPInst(pShader, func, inst, pDuInfo, pMM);
                ON_ERROR(errCode, "Insert precision conversion inst");
            }

            if (pHwCfg->hwFeatureFlags.supportUSC)
            {
                errCode = _ConvSrc2SwizzleForAtomInst(pShader, func, inst);
                ON_ERROR(errCode, "Normalize swizzle for atomic instruction source2");
            }

            /* For a temp 256 register pair, we need to make sure that both operands are temp registers. */
            if (VIR_OPCODE_SrcsTemp256(opCode))
            {
                errCode = _ConvTemp256Srcs(pDuInfo, pShader, func, inst);
                ON_ERROR(errCode, "Convert temp 256 register pair.");
            }

            /* Process the modifier order. */
            errCode = _ProcessModOrder(pDuInfo, pShader, func, inst);
            ON_ERROR(errCode, "Process modifier order.");
        }
    }

    /*
    ** For some EVIS instructions, HW doesn't decode src2's swizzle if src2 it is not a IMMEDIATE,
    ** so we insert a MOV instruction whose enable is XYZW to replace the src2.
    */
    errCode = _ConvEvisInstForShader(pPassWorker);
    ON_ERROR(errCode, "Convert evis instruction");

    /* Use gl_LocalInvocationID to calculate the gl_LocalInvocationIndex. */
    errCode = _CalculateLocalInvocationIndex(pShader, &bInvalidDu);
    ON_ERROR(errCode, "Calculate local invocation index. ");

    if (VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after post-MC-cleanup phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    if (bInvalidDu)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;
    }
    if (bInvalidCfg)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = gcvTRUE;
    }

OnError:
    return errCode;
}

void _VIR_FCP_ReplaceDUAL32(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst)
{
    VIR_Instruction     *newInst = gcvNULL;
    gctUINT             i;

    VIR_Function_AddInstructionAfter(pFunc,
                VIR_Inst_GetOpcode(pInst),
                VIR_TYPE_UINT16,
                pInst,
                gcvTRUE,
                &newInst);
    VIR_Inst_SetConditionOp(newInst, VIR_Inst_GetConditionOp(pInst));

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
        VIR_Operand_Copy(newInst->src[i], pInst->src[i]);

        if (VIR_Operand_GetPrecision(pInst->src[i]) == VIR_PRECISION_HIGH)
        {
            gcmASSERT(VIR_Operand_GetHIHwRegId(pInst->src[i]) != VIR_FCP_INVALID_REG);
            VIR_Operand_SetHwRegId(newInst->src[i], VIR_Operand_GetHIHwRegId(pInst->src[i]));
            VIR_Operand_SetHwShift(newInst->src[i], VIR_Operand_GetHIHwShift(pInst->src[i]));
        }
        if (VIR_Operand_GetRelAddrMode(newInst->src[i]) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(newInst->src[i]));
            gcmASSERT(sym);
            if (VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_HIGH)
            {
                /* newOpnd's relAddrMode saves the low shift, add the difference of
                   high shift and low shift */
                gctUINT relMode = VIR_Operand_GetRelAddrMode(newInst->src[i]);
                relMode = relMode + VIR_Symbol_GetHIHwShift(sym) - VIR_Symbol_GetHwShift(sym);
                VIR_Operand_SetRelAddrMode(newInst->src[i], relMode);
            }
        }
    }

    /* duplicate dest and copy dest */
    if (pInst->dest)
    {
        VIR_Operand_Copy(newInst->dest, pInst->dest);

        if (VIR_Operand_GetPrecision(pInst->dest) == VIR_PRECISION_HIGH)
        {
            gcmASSERT(VIR_Operand_GetHIHwRegId(pInst->dest) != VIR_FCP_INVALID_REG);
            VIR_Operand_SetHwRegId(newInst->dest, VIR_Operand_GetHIHwRegId(pInst->dest));
            VIR_Operand_SetHwShift(newInst->dest, VIR_Operand_GetHIHwShift(pInst->dest));
        }
        if (VIR_Operand_GetRelAddrMode(newInst->dest) != VIR_INDEXED_NONE)
        {
            VIR_Symbol  *sym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(newInst->dest));
            gcmASSERT(sym);
            if (VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_HIGH)
            {
                /* newOpnd's relAddrMode saves the low shift, add the difference of
                   high shift and low shift */
                gctUINT relMode = VIR_Operand_GetRelAddrMode(newInst->dest);
                relMode = relMode + VIR_Symbol_GetHIHwShift(sym) - VIR_Symbol_GetHwShift(sym);
                VIR_Operand_SetRelAddrMode(newInst->dest, relMode);
            }
        }
    }

    /* set thread mode*/
    VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_SINGLE_T0);
    VIR_Inst_SetThreadMode(newInst, VIR_THREAD_D16_SINGLE_T1);
}

static void
_changeConvDataType(IN VIR_Instruction    *Inst)
{
    VIR_ScalarConstVal  imm0;
    VIR_Operand         *opnd = gcvNULL, *convDestOpnd = gcvNULL, *i2iSrc0Opnd = gcvNULL;
    VIR_TypeId          ty, componentTy, newComponentTy = VIR_TYPE_VOID, i2iSrc0Ty = VIR_TYPE_VOID;
    gctUINT32           componentCount;
    gctBOOL             needChangeSrc1 = gcvFALSE;
    gctBOOL             bChangeToMov = gcvFALSE;

    if (VIR_Inst_GetOpcode(Inst) == VIR_OP_I2I)
    {
        opnd = VIR_Inst_GetDest(Inst);
        i2iSrc0Opnd = VIR_Inst_GetSource(Inst, 0);
        i2iSrc0Ty = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(i2iSrc0Opnd));
    }
    else
    {
        convDestOpnd = VIR_Inst_GetDest(Inst);
        opnd = VIR_Inst_GetSource(Inst, 0);
    }
    ty = VIR_Operand_GetTypeId(opnd);
    componentTy = VIR_GetTypeComponentType(ty);

    imm0.iValue = 0x1;

    if (VIR_Operand_GetPrecision(opnd) != VIR_PRECISION_HIGH)
    {
        switch (componentTy)
        {
        case VIR_TYPE_FLOAT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x1;
            newComponentTy = VIR_TYPE_FLOAT16;
            break;
        case VIR_TYPE_INT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x3;
            newComponentTy = VIR_TYPE_INT16;
            break;
        case VIR_TYPE_UINT32:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x6;
            newComponentTy = VIR_TYPE_UINT16;;
            break;
        case VIR_TYPE_BOOLEAN:
            needChangeSrc1 = gcvTRUE;
            imm0.iValue = 0x3;
            newComponentTy = VIR_TYPE_INT16;
            break;
        default:
            break;
        }

        if (VIR_Inst_GetOpcode(Inst) == VIR_OP_I2I)
        {
            /*
            ** I2I can't support that DEST and SRC use the same data type, so we just change it to MOV.
            ** VIV:TODO: maybe we can move this check before RA so we may skip this MOV.
            */
            if (newComponentTy == i2iSrc0Ty)
            {
                bChangeToMov = gcvTRUE;
            }
            else
            {
                imm0.iValue <<= 4;
            }
        }
    }

    /* Change the instruction. */
    if (bChangeToMov)
    {
        VIR_Inst_SetOpcode(Inst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(Inst, 1);
    }
    else if (needChangeSrc1)
    {
        VIR_Operand_SetImmediate(Inst->src[1], VIR_TYPE_INT32, imm0);

        /* We need to change the data type of DEST. */
        if (convDestOpnd != gcvNULL)
        {
            ty = VIR_Operand_GetTypeId(convDestOpnd);
            componentTy = VIR_GetTypeComponentType(ty);
            componentCount = VIR_GetTypeComponents(ty);

            switch (componentTy)
            {
            case VIR_TYPE_FLOAT32:
                componentTy = VIR_TYPE_FLOAT16;
                break;

            case VIR_TYPE_INT32:
            case VIR_TYPE_BOOLEAN:
                componentTy = VIR_TYPE_INT16;
                break;

            case VIR_TYPE_UINT32:
                componentTy = VIR_TYPE_UINT16;
                break;

            default:
                break;
            }

            if (componentTy != VIR_GetTypeComponentType(ty))
            {
                ty = VIR_TypeId_ComposeNonOpaqueType(componentTy, componentCount, 1);
                VIR_Operand_SetTypeId(convDestOpnd, ty);
            }
        }
    }
}

static VSC_ErrCode _SetResOpBitsForSampler(VIR_Shader *pShader,
                                           VIR_Instruction* texldInst,
                                           VIR_Instruction* InstGetSamplerIdx)
{
    VIR_Operand* src0;
    VIR_Operand *src1;
    VIR_Symbol  *sym;
    gctUINT     index;

    if (InstGetSamplerIdx)
    {
        src0 = VIR_Inst_GetSource(InstGetSamplerIdx, 0);
        src1 = VIR_Inst_GetSource(InstGetSamplerIdx, 1);
        sym = VIR_Operand_GetSymbol(src0);

        gcmASSERT(VIR_Symbol_GetKind(sym) == VIR_SYM_SAMPLER);

        if (VIR_Operand_GetOpKind(src1) == VIR_OPND_IMMEDIATE)
        {
            index = VIR_Operand_GetImmediateUint(src1);
        }
        else
        {
            index = NOT_ASSIGNED;
        }
    }
    else
    {
        src0 = VIR_Inst_GetSource(texldInst, 0);
        sym = VIR_Operand_GetSymbol(src0);

        gcmASSERT(VIR_Symbol_GetKind(sym) == VIR_SYM_SAMPLER);

        if (VIR_Operand_GetRelAddrMode(src0) != VIR_INDEXED_NONE)
        {
            index = NOT_ASSIGNED;
        }
        else
        {
            index = VIR_Operand_GetMatrixConstIndex(src0) + VIR_Operand_GetRelIndexing(src0);
        }
    }

    return VIR_Uniform_UpdateResOpBits(pShader,
                                       VIR_Symbol_GetSampler(sym),
                                       VIR_Inst_GetResOpType(texldInst),
                                       index);
}

VSC_ErrCode _markEndOfBBFlag(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
            func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_CONTROL_FLOW_GRAPH* cfg;
        CFG_ITERATOR cfg_iter;
        VIR_BASIC_BLOCK* bb;
        cfg = VIR_Function_GetCFG(func);

        gcmASSERT(cfg != gcvNULL);

        /* For control flow instructions, such as BRANCH, BRANCH_ANY,
         * CALL, RET and TEXKILL, there is a target PC instruction which
         * is the control flow instruction jumps to if jump path is taken,
         * then the instruction just before this target PC instruction
         * will be the end of a basic block. cc8000 arch needs compiler
         * to explicitly tell hardware whether an instruction is the end
         * of a basic block if that instruction is not a control flow
         * instruction, then hardware will use this information to manage
         * 2-group fast reissue, see 2-Group-Fast-Reissue. */
        CFG_ITERATOR_INIT(&cfg_iter, cfg);
        for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
        {
            VIR_Instruction* prevBBLastInst;
            VIR_Instruction* firstInst;
            VIR_OpCode opCode;
            VIR_BASIC_BLOCK * jumpToBB = VIR_BB_GetJumpToBB(bb);
            if (jumpToBB)
            {
                firstInst = BB_GET_START_INST(jumpToBB);
                if (firstInst)
                {
                    prevBBLastInst = VIR_Inst_GetPrev(firstInst);
                    if (prevBBLastInst)
                    {
                        opCode = VIR_Inst_GetOpcode(prevBBLastInst);
                        if (!VIR_OPCODE_isBBSuffix(opCode) && !VIR_OPCODE_isCall(opCode) &&
                            opCode != VIR_OP_KILL)
                        {
                            VIR_Inst_SetEndOfBB(prevBBLastInst, gcvTRUE);
                        }
                    }
                }
            }
        }
    }

    return errCode;
}

VSC_ErrCode _markUSCUnallocFlag(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;

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
            VIR_OpCode opCode      = VIR_Inst_GetOpcode(inst);
            if (gcmOPT_hasFeature(FB_FORCE_USC_UNALLOC) &&
                (VIR_OPCODE_isMemLd(opCode) ||
                 VIR_OPCODE_isMemSt(opCode) ||
                 VIR_OPCODE_isImgLd(opCode) ||
                 VIR_OPCODE_isImgSt(opCode)))
            {
                VIR_Inst_SetUSCUnallocate(inst, gcvTRUE);
            }
        }
    }

    return errCode;
}

static gctBOOL
_IsOperandFloat16(
    IN VIR_Shader*          pShader,
    IN VIR_Instruction*     pInst,
    IN gctBOOL              bDst,
    IN gctUINT32            srcIndex
    )
{
    VIR_Operand*            pOpnd = bDst ?  VIR_Inst_GetDest(pInst) : VIR_Inst_GetSource(pInst, srcIndex);
    VIR_TypeId              typeId = VIR_Operand_GetTypeId(pOpnd);
    VIR_TypeId              componentTypeId = VIR_GetTypeComponentType(typeId);

    if (VIR_Shader_isDual16Mode(pShader) &&
        (VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_16 ||
         VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_HIGHPVEC2))
    {
        return gcvFALSE;
    }
    else if (componentTypeId == VIR_TYPE_FLOAT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL
_IsOperandInt16OrInt8(
    IN VIR_Shader*          pShader,
    IN VIR_Instruction*     pInst,
    IN gctBOOL              bDst,
    IN gctUINT32            srcIndex
    )
{
    VIR_Operand*            pOpnd = bDst ?  VIR_Inst_GetDest(pInst) : VIR_Inst_GetSource(pInst, srcIndex);
    VIR_TypeId              typeId = VIR_Operand_GetTypeId(pOpnd);
    VIR_TypeId              componentTypeId = VIR_GetTypeComponentType(typeId);

    if (componentTypeId == VIR_TYPE_INT16 || componentTypeId == VIR_TYPE_UINT16 ||
        componentTypeId == VIR_TYPE_UINT8 || componentTypeId == VIR_TYPE_INT8)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/*explicit show enable bits for highpvec2 in dual16 */
static VIR_Enable
_VIR_FCP_ExtendHighpvec2DefEnableInHighpvec2Dual16(
    VIR_Enable origEnable
    )
{
    VIR_Enable extend_enable = origEnable;
    switch (origEnable)
    {
        case VIR_ENABLE_X:
            extend_enable = VIR_ENABLE_XZ;
            break;
        case VIR_ENABLE_Y:
            extend_enable = VIR_ENABLE_YW;
            break;
        case VIR_ENABLE_XY:
            extend_enable = VIR_ENABLE_XYZW;
            break;
        default:
            gcmASSERT(gcvFALSE);  /* unsupported dest enable bits in dual16 highpvec2 */
            break;
    }
    return extend_enable;
}

static VIR_Swizzle
_VIR_FCP_ExtendUsageSwizzleInHighpvec2Dual16(
    VIR_Operand *src)
{
    VIR_Swizzle extend_swizzle = VIR_Operand_GetSwizzle(src);
    gctBOOL     isHighp = (VIR_Operand_GetPrecision(src) == VIR_PRECISION_HIGH);
    VIR_Symbol  *srcSym = VIR_Operand_GetSymbol(src);
    gctBOOL     isSymInput = gcvFALSE;
    if (srcSym)
    {
        isSymInput = VIR_Symbol_isInput(srcSym);  /*if highpvec2 src is input, the swizzle is like mp mode */
    }
    switch (extend_swizzle)
    {
        case VIR_SWIZZLE_X:
            if (isHighp && !isSymInput)
            {
                extend_swizzle = VIR_SWIZZLE_XXZZ;
            }
            else
            {
                extend_swizzle = VIR_SWIZZLE_XXXX;  /*swizzle "x" and "z"*/
            }
            break;
        case VIR_SWIZZLE_Y:
        case VIR_SWIZZLE_YYYY:
            if (isHighp && !isSymInput)
            {
                extend_swizzle = VIR_SWIZZLE_YYWW;
            }
            else
            {
                extend_swizzle = VIR_SWIZZLE_YYYY;  /*extend swizzle "y" to "yyyy"*/
            }
            break;
        case VIR_SWIZZLE_XYYY:
            if (isHighp && !isSymInput)
            {
                extend_swizzle = VIR_SWIZZLE_XYZW;
            }
            else
            {
                extend_swizzle = VIR_SWIZZLE_XYXY;
            }
            break;
        default:
            gcmASSERT(gcvFALSE);  /* unsupported dest enable bits in dual16 highpvec2 */
    }
    return extend_swizzle;
}

/* for highpvec2 sym def/use instruction in different thread modes
 *     thread mode of Define Inst   thread mode of Usage Instr         extend rule
 *        .highpvec2                 .highpvec2               explicit extend enable/swizzle bits, .xy -> .xyzw
 *        .highpvec2                 .t0t1                    explicit extend enable bits
 *        .t0t1                      .highpvec2               explicit extend swizzle bits
 *        .t0t1                      .t0t1                    original support, hw not support dual-t highpvec2  mode
 * for mpvec2 sym def/use instruction in different thread modes
 *     thread mode of Define Inst   thread mode of Usage Instr         extend rule
 *       .dual16                   .dual16                   original support
 *       .dual16                   .highpvec2                explicit extend swizzle bits, .xy -> .xyxy ??Does HW know high 16bit of are t1 defined value
 *       .dual16                   .t0t1                     original support
 *       .t0t1                     .dual16                   original support
 *       .t0t1                     .highpvec2                explicit extend swizzle bits, .xy -> .xyxy ??Does HW know high 16bit of zw are t1 defined value
 *       .t0t1                     .t0t1                     original support
 */
static void
_VIR_FCP_UpdateEnableSwizzleInHighpVec2Dual16(
    IN VIR_Instruction* pInst)
{
    VIR_Operand* dst = VIR_Inst_GetDest(pInst);
    gctUINT i = 0;
    gcmASSERT(VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_HIGHPVEC2);
    /*extend enable*/
    if (dst)
    {
        VIR_Enable extend_enable = _VIR_FCP_ExtendHighpvec2DefEnableInHighpvec2Dual16(VIR_Operand_GetEnable(dst));
        VIR_Operand_SetEnable(dst, extend_enable);
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        VIR_Operand *src = VIR_Inst_GetSource(pInst, i);
        VIR_Swizzle extend_swizzle = _VIR_FCP_ExtendUsageSwizzleInHighpvec2Dual16(src);
        VIR_Operand_SetSwizzle(src, extend_swizzle);
    }

}

static VSC_ErrCode
_VIR_FCP_ModifyFP16Instruction(
    IN VIR_Shader*      pShader,
    IN VIR_Function*    pFunc,
    IN VIR_Instruction* pInst,
    INOUT gctBOOL*      pChanged
    )
{
    VSC_ErrCode         status = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    gctBOOL             bChanged = gcvFALSE;

    /*
    ** Because HW can't support MOV.f16, we need to convert it to CONV.f16 or MOV.uint16/MOV.int16 based on the dest type.
    */
    if (opCode == VIR_OP_MOV && _IsOperandFloat16(pShader, pInst, gcvFALSE, 0))
    {
        VIR_Operand*    pDst = VIR_Inst_GetDest(pInst);
        VIR_TypeId      dstTypeId = VIR_Operand_GetTypeId(pDst);
        VIR_TypeId      dstCompTypeId = VIR_GetTypeComponentType(dstTypeId);
        VIR_Operand*    pSrc0 = VIR_Inst_GetSource(pInst, 0);
        VIR_TypeId      src0TypeId = VIR_Operand_GetTypeId(pSrc0);
        VIR_TypeId      src0CompTypeId = VIR_TYPE_FLOAT16;

        /* If dest and source have the same type, just change MOV to CONV. */
        if (dstCompTypeId == VIR_TYPE_FLOAT16)
        {
            VIR_Operand*    pNewOpnd = gcvNULL;
            VIR_Inst_SetOpcode(pInst, VIR_OP_CONV);

            VIR_Function_NewOperand(pFunc, &pNewOpnd);
            VIR_Inst_SetSrcNum(pInst, 2);
            VIR_Inst_SetSource(pInst, 1, pNewOpnd);

            VIR_Operand_SetImmediateUint(pNewOpnd, 0x1);
        }
        else
        {
            if (VIR_TypeId_isFloat(dstCompTypeId))
            {
                src0CompTypeId = VIR_TYPE_UINT16;
            }
            else if (VIR_TypeId_isUnSignedInteger(dstCompTypeId))
            {
                src0CompTypeId = VIR_TYPE_UINT16;
            }
            else
            {
                src0CompTypeId = VIR_TYPE_INT16;
            }
        }
        src0TypeId = VIR_TypeId_ComposeNonOpaqueType(src0CompTypeId,
                                                     VIR_GetTypeComponents(src0TypeId),
                                                     1);
        VIR_Operand_SetTypeId(pSrc0, src0TypeId);

        bChanged = gcvTRUE;
    }
    /* Change INT16/FP16/INT8 LOAD_ATTR/STORE_ATTR to INT32/FP32. */
    else if (VIR_OPCODE_isAttrSt(opCode) &&
            (_IsOperandFloat16(pShader, pInst, gcvFALSE, 2) || _IsOperandInt16OrInt8(pShader, pInst, gcvFALSE, 2)))
    {
        VIR_Operand*    pDst = VIR_Inst_GetDest(pInst);
        VIR_TypeId      dstTypeId = VIR_Operand_GetTypeId(pDst);
        VIR_Operand*    pSrc2 = VIR_Inst_GetSource(pInst, 2);
        VIR_TypeId      src2TypeId = VIR_Operand_GetTypeId(pSrc2);

        dstTypeId = VIR_TypeId_ComposeNonOpaqueType(_IsOperandInt16OrInt8(pShader, pInst, gcvFALSE, 2) ? VIR_TYPE_UINT32 : VIR_TYPE_FLOAT32,
                                                    VIR_GetTypeComponents(dstTypeId),
                                                    1);
        src2TypeId = VIR_TypeId_ComposeNonOpaqueType(_IsOperandInt16OrInt8(pShader, pInst, gcvFALSE, 2) ? VIR_TYPE_UINT32 : VIR_TYPE_FLOAT32,
                                                     VIR_GetTypeComponents(src2TypeId),
                                                     1);
        VIR_Operand_SetTypeId(pDst, dstTypeId);
        VIR_Operand_SetTypeId(pSrc2, src2TypeId);
        bChanged = gcvTRUE;
    }
    else if (VIR_OPCODE_isAttrLd(opCode) &&
            (_IsOperandFloat16(pShader, pInst, gcvTRUE, 0) || _IsOperandInt16OrInt8(pShader, pInst, gcvTRUE, 0)))
    {
        VIR_Operand*    pDst = VIR_Inst_GetDest(pInst);
        VIR_TypeId      dstTypeId = VIR_Operand_GetTypeId(pDst);

        dstTypeId = VIR_TypeId_ComposeNonOpaqueType(_IsOperandInt16OrInt8(pShader, pInst, gcvTRUE, 0) ? VIR_TYPE_UINT32 : VIR_TYPE_FLOAT32,
                                                    VIR_GetTypeComponents(dstTypeId),
                                                    1);
        VIR_Operand_SetTypeId(pDst, dstTypeId);
        bChanged = gcvTRUE;
    }

    if (pChanged)
    {
        *pChanged = bChanged;
    }

    return status;
}

DEF_QUERY_PASS_PROP(vscVIR_PostCGCleanup)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PostCGCleanup)
{
    return gcvTRUE;
}

/*********************************************
* vscVIR_PostCGCleanup
* After register allocation, perform
  1) for dual16, change the dual32 instruction
  2) set endOfBB flag for non-control flow instruction to help fast reissue
   ...
*********************************************/
VSC_ErrCode vscVIR_PostCGCleanup(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         status = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    VIR_Instruction*    instGetSamplerIdx = gcvNULL;
    VSC_HW_CONFIG       *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            gctBOOL bGenInst = gcvFALSE;
            gctBOOL bFP16Changed = gcvFALSE;

            if (VIR_Shader_isDual16Mode(pShader))
            {
                if (VIR_Inst_GetThreadMode(inst) == VIR_THREAD_D16_DUAL_32)
                {
                    _VIR_FCP_ReplaceDUAL32(pShader, func, inst);
                    bGenInst = gcvTRUE;
                }
                /* update enable/swizzle of highpvec2 dual16 */
                else if (HWSUPPORTDUAL16HIGHVEC2 &&
                         VIR_Inst_GetThreadMode(inst) == VIR_THREAD_D16_DUAL_HIGHPVEC2)
                {
                    _VIR_FCP_UpdateEnableSwizzleInHighpVec2Dual16(inst);
                }

                /* correctly setting dest/src1 for conv/i2i instruction  */
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_CONV ||
                    VIR_Inst_GetOpcode(inst) == VIR_OP_I2I)
                {
                    _changeConvDataType(inst);
                }
            }

            /* Modify FP16 instructions which HW can't native support. */
            status = _VIR_FCP_ModifyFP16Instruction(pShader, func, inst, &bFP16Changed);
            ON_ERROR(status, "Convert single temp 256 source.");
            if (bFP16Changed)
            {
                WARNING_REPORT(VSC_ERR_INVALID_DATA, "Modify some FP16 instructions, be careful.");
            }

            if (VIR_Inst_GetOpcode(inst) == VIR_OP_GET_SAMPLER_IDX)
            {
                instGetSamplerIdx = inst;
            }
            else if (VIR_Inst_GetResOpType(inst) != VIR_RES_OP_TYPE_UNKNOWN &&
                     VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)))
            {
                _SetResOpBitsForSampler(pShader, inst, instGetSamplerIdx);
                instGetSamplerIdx = gcvNULL;
            }

            if (bGenInst)
            {
                inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);
            }
        }
    }

    /* mark EndOfBB to help HW manage 2-group fast reissue */
    if (pHwCfg->hwFeatureFlags.supportEndOfBBReissue)
    {
        _markEndOfBBFlag(pPassWorker);
    }

    /* mark USCUnallocate */
    if (pHwCfg->hwFeatureFlags.supportUSCUnalloc)
    {
        _markUSCUnallocFlag(pPassWorker);
    }

    if (VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after post-CG-cleanup Phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

OnError:
    return status;
}


