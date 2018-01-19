/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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
    VIR_Instruction     *pInst)
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
    TO-DO: could be smarter to compute the benefits and costs.
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
            VIR_Function_RemoveInstruction(pFunc, pInst);
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
    IN VIR_Dumper* dumper
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
        VIR_OPCODE_isVXOnly(VIR_Inst_GetOpcode(usage_inst_of_icast)))
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
        VIR_Function_RemoveInstruction(func, icast);
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
    IN VIR_Dumper* dumper
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
        VIR_OPCODE_isVXOnly(VIR_Inst_GetOpcode(def_vx_inst)))
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
            VIR_Function_RemoveInstruction(func, icast);
        }
        return errCode;
    }
    /* otherwise change icast to mov */
    VIR_Inst_SetOpcode(icast, VIR_OP_MOV);
    return errCode;
}

#if __ENABLE_OPTIMIZE_FOR_PRI_MEMORY__
static VSC_ErrCode _CalculateIndexForPrivateMemory(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_Operand         *dstOpnd = gcvNULL;
    VIR_Operand         *srcOpnd = gcvNULL;
    VIR_NameId          nameId;
    VIR_SymId           workThreadCountSymId = VIR_INVALID_ID;
    VIR_Symbol          *workThreadCountSym = gcvNULL;
    VIR_Symbol          *privateMemSym = gcvNULL;

    if (opCode == VIR_OP_ADD && VIR_Operand_isSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src1)))
    {
        privateMemSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src1));
    }
    else if (opCode == VIR_OP_IMADLO0 && VIR_Operand_isSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src2)))
    {
        privateMemSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src2));
    }

    if (privateMemSym &&
        VIR_Symbol_isUniform(privateMemSym) &&
        strcmp(VIR_Shader_GetSymNameString(pShader, privateMemSym), "#private_address") == 0)
    {
        /* Add a new uniform to save the workThreadCount. */
        errCode = VIR_Shader_AddString(pShader,
                                       "#WorkThreadCount",
                                       &nameId);

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                       VIR_STORAGE_UNKNOWN,
                                       &workThreadCountSymId);
        ON_ERROR(errCode, "Add workThreadCount uniform. ");

        workThreadCountSym = VIR_Shader_GetSymFromId(pShader, workThreadCountSymId);
        VIR_Symbol_SetFlag(workThreadCountSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(workThreadCountSym, VIR_PRECISION_MEDIUM);
        VIR_Symbol_SetUniformKind(workThreadCountSym, VIR_UNIFORM_WORK_THREAD_COUNT);
        VIR_Symbol_SetAddrSpace(workThreadCountSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(workThreadCountSym, VIR_TYQUAL_CONST);

        if (opCode == VIR_OP_ADD)
        {
            /*
            ** 004: LSHIFT             uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            ** -->
            ** 004: IMOD               ushort temp(9).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 005: LSHIFT             uint temp(9).x{r0.<3}, uint temp(9).x{r0.<3}, int 8
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            **
            ** Or
            **
            ** 004: MUL                uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            ** -->
            ** 004: IMOD               ushort temp(9).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 005: MUL                uint temp(9).x{r0.<3}, uint temp(9).x{r0.<3}, uint 300
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            */
            VIR_Instruction     *shiftOrMulInst = gcvNULL;

            /* Insert a MOD before LSHIFT/MUL. */
            shiftOrMulInst = VIR_Inst_GetPrev(pInst);
            gcmASSERT(VIR_Inst_GetOpcode(shiftOrMulInst) == VIR_OP_LSHIFT || VIR_Inst_GetOpcode(shiftOrMulInst) == VIR_OP_MUL);

            VIR_Function_AddCopiedInstructionBefore(pFunc,
                                                    shiftOrMulInst,
                                                    shiftOrMulInst,
                                                    gcvTRUE,
                                                    &newInst);
            VIR_Inst_SetOpcode(newInst, VIR_OP_IMOD);

            /*
            ** Change the instType to UINT16.
            */
            VIR_Inst_SetInstType(newInst, VIR_TYPE_UINT16);

            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetTypeId(dstOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workThreadCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Change the SRC0 of LSHIFT/MUL. */
            srcOpnd = VIR_Inst_GetSource(shiftOrMulInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
        }
        else
        {
            /*
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(8).x{r0.<3}, int 340, uint #private_address.x
            ** -->
            ** 003: IMOD               ushort temp(1).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(1).x{r0.<3}, int 340, uint #private_address.x
            */
            /* Insert a MOD before MAD. */
            VIR_Function_AddInstructionBefore(pFunc,
                                              VIR_OP_IMOD,
                                              VIR_TYPE_UINT16,
                                              pInst,
                                              gcvTRUE,
                                              &newInst);
            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_Copy(dstOpnd, VIR_Inst_GetDest(pInst));
            VIR_Operand_SetTypeId(dstOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workThreadCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Change the SRC0 of MAD. */
            srcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
        }
    }

OnError:
    return errCode;
}
#endif

#if __ENABLE_OPTIMIZE_FOR_SHARE_MEMORY__
static VSC_ErrCode _CalculateIndexForLocalMemory(
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Instruction     *newInst = gcvNULL;
    VIR_Operand         *dstOpnd = gcvNULL;
    VIR_Operand         *srcOpnd = gcvNULL;
    VIR_NameId          nameId;
    VIR_SymId           workGroupCountSymId = VIR_INVALID_ID;
    VIR_Symbol          *workGroupCountSym = gcvNULL;
    VIR_Symbol          *localMemSym = gcvNULL;

    if (opCode == VIR_OP_ADD && VIR_Operand_isSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src1)))
    {
        localMemSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src1));
    }
    else if (opCode == VIR_OP_IMADLO0 && VIR_Operand_isSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src2)))
    {
        localMemSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInst, VIR_Operand_Src2));
    }

    if (localMemSym &&
        VIR_Symbol_isUniform(localMemSym) &&
        strcmp(VIR_Shader_GetSymNameString(pShader, localMemSym), "#local_address") == 0)
    {
        /* Add a new uniform to save the workGroupCount. */
        errCode = VIR_Shader_AddString(pShader,
                                       "#workGroupCount",
                                       &nameId);

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                       VIR_STORAGE_UNKNOWN,
                                       &workGroupCountSymId);
        ON_ERROR(errCode, "Add workGroupCount uniform. ");

        workGroupCountSym = VIR_Shader_GetSymFromId(pShader, workGroupCountSymId);
        VIR_Symbol_SetFlag(workGroupCountSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(workGroupCountSym, VIR_PRECISION_MEDIUM);
        VIR_Symbol_SetUniformKind(workGroupCountSym, VIR_UNIFORM_WORK_GROUP_COUNT);
        VIR_Symbol_SetAddrSpace(workGroupCountSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(workGroupCountSym, VIR_TYQUAL_CONST);

        if (opCode == VIR_OP_ADD)
        {
            /*
            ** 004: LSHIFT             uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            ** -->
            ** 004: IMOD               ushort temp(9).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #workGroupCount.x
            ** 005: LSHIFT             uint temp(9).x{r0.<3}, uint temp(9).x{r0.<3}, int 8
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            **
            ** Or
            **
            ** 004: MUL                uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            ** -->
            ** 004: IMOD               ushort temp(9).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #workGroupCount.x
            ** 005: MUL                uint temp(9).x{r0.<3}, uint temp(9).x{r0.<3}, uint 300
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            */
            VIR_Instruction     *shiftOrMulInst = gcvNULL;

            /* Insert a MOD before LSHIFT/MUL. */
            shiftOrMulInst = VIR_Inst_GetPrev(pInst);
            gcmASSERT(VIR_Inst_GetOpcode(shiftOrMulInst) == VIR_OP_LSHIFT || VIR_Inst_GetOpcode(shiftOrMulInst) == VIR_OP_MUL);

            VIR_Function_AddCopiedInstructionBefore(pFunc,
                                                    shiftOrMulInst,
                                                    shiftOrMulInst,
                                                    gcvTRUE,
                                                    &newInst);
            VIR_Inst_SetOpcode(newInst, VIR_OP_IMOD);

            /*
            ** Change the instType to UINT16.
            */
            VIR_Inst_SetInstType(newInst, VIR_TYPE_UINT16);

            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetTypeId(dstOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workGroupCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Change the SRC0 of LSHIFT/MUL. */
            srcOpnd = VIR_Inst_GetSource(shiftOrMulInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
        }
        else
        {
            /*
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(8).x{r0.<3}, int 340, uint #local_address.x
            ** -->
            ** 003: IMOD               ushort temp(1).x{r0.<3}, ushort temp(8).x{r0.<3}, ushort #WorkGroupCount.x
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(1).x{r0.<3}, int 340, uint #local_address.x
            */
            /* Insert a MOD before MAD. */
            VIR_Function_AddInstructionBefore(pFunc,
                                              VIR_OP_IMOD,
                                              VIR_TYPE_UINT16,
                                              pInst,
                                              gcvTRUE,
                                              &newInst);
            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_Copy(dstOpnd, VIR_Inst_GetDest(pInst));
            VIR_Operand_SetTypeId(dstOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workGroupCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Change the SRC0 of MAD. */
            srcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
        }
    }

OnError:
    return errCode;
}
#endif

static VSC_ErrCode _FixEnableRestrictionForImageLoad(
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Operand         *pOrigDest = VIR_Inst_GetDest(pInst);
    VIR_Precision       precision = VIR_Operand_GetPrecision(pOrigDest);
    VIR_Enable          enable = VIR_Operand_GetEnable(pOrigDest);
    VIR_TypeId          dstTypeId = VIR_Operand_GetTypeId(pOrigDest);
    VIR_TypeId          dstCompTypeId = VIR_GetTypeComponentType(dstTypeId);
    VIR_TypeId          newTypeId = VIR_TypeId_ComposeNonOpaqueType(dstCompTypeId, VIR_CHANNEL_NUM, 1);
    VIR_VirRegId        newRegId = VIR_INVALID_ID;
    VIR_SymId           newRegSymId = VIR_INVALID_ID;
    VIR_Instruction     *pNewMovInst = gcvNULL;
    VIR_Operand         *pOpnd = gcvNULL;
    VIR_OperandInfo     opndInfo;
    gctUINT             i;

    /* Check if we need to adjust the enable, now we only support full channel enable, XYZW. */
    if (enable == VIR_ENABLE_XYZW)
    {
        return errCode;
    }

    /*
    ** IMG_LOAD  r1.x, ...
    **  -->
    ** IMG_LOAD  r2.xyzw, ...
    ** MOV       r1.x, r2.xyzw
    */
    /* Create a new temp register to save the IMG_LOAD. */
    newRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   newRegId,
                                   VIR_Shader_GetTypeFromId(pShader, newTypeId),
                                   VIR_STORAGE_UNKNOWN,
                                   &newRegSymId);
    ON_ERROR(errCode, "Add new symbol. ");

    /*--------------Insert a IMG_LOAD instruction--------------*/
    errCode = VIR_Function_AddCopiedInstructionBefore(pFunc,
                                                      pInst,
                                                      pInst,
                                                      gcvTRUE,
                                                      &pNewMovInst);
    ON_ERROR(errCode, "Copy instruction IMG_LOAD. ");

    /* Update DEST. */
    pOpnd = VIR_Inst_GetDest(pNewMovInst);
    memset(pOpnd, 0, sizeof(VIR_Operand));
    VIR_Operand_SetLvalue(pOpnd, gcvTRUE);
    VIR_Operand_SetTempRegister(pOpnd, pFunc, newRegSymId, newTypeId);
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XYZW);
    VIR_Operand_SetPrecision(pOpnd, precision);

    /* Add a new def for IMG_LOAD instruction. */
    vscVIR_AddNewDef(pDuInfo,
                     pNewMovInst,
                     newRegId,
                     1,
                     VIR_ENABLE_XYZW,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     gcvNULL,
                     gcvNULL);

    for (i = 0; i < VIR_Inst_GetSrcNum(pNewMovInst); i++)
    {
        pOpnd = VIR_Inst_GetSource(pNewMovInst, i);
        VIR_Operand_GetOperandInfo(pNewMovInst, pOpnd, &opndInfo);

        if (opndInfo.isVreg)
        {
            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    pNewMovInst,
                                    pOpnd,
                                    gcvFALSE,
                                    opndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd)),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }
    }

    /*--------------Change IMG_LOAD to MOV--------------*/
    /* Delete all usages first. */
    for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
    {
        pOpnd = VIR_Inst_GetSource(pInst, i);
        VIR_Operand_GetOperandInfo(pInst, pOpnd, &opndInfo);

        if (opndInfo.isVreg)
        {
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               pInst,
                               pOpnd,
                               gcvFALSE,
                               opndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);
        }
    }

    /* Change SrcNum and Opcode. */
    VIR_Inst_SetSrcNum(pInst, 1);
    VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);

    /* Use the new temp as SRC0. */
    pOpnd = VIR_Inst_GetSource(pInst, 0);
    memset(pOpnd, 0, sizeof(VIR_Operand));
    VIR_Operand_SetTempRegister(pOpnd, pFunc, newRegSymId, newTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYZW);
    VIR_Operand_SetPrecision(pOpnd, precision);

    /* Add the usage for SRC0. */
    vscVIR_AddNewUsageToDef(pDuInfo,
                            pNewMovInst,
                            pInst,
                            VIR_Inst_GetSource(pInst, 0),
                            gcvFALSE,
                            newRegId,
                            1,
                            enable,
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PreCleanup)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_FCP;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

/*********************************************
* vscVIR_PreCleanup
* At the beginnig of code generation, perform
  1) replace LDARR/STARR
  2) dual16 shader, we need to patch the code for some cases:
  2.1) insert precison conv for implicit type interpretion
  3) Create a uniform to save the threadCount and insert a MOD to calculate the globalIndex for private memory.
   ...
*********************************************/
VSC_ErrCode vscVIR_PreCleanup(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_HW_CONFIG       *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO  *pDuInfo = pPassWorker->pDuInfo;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VSC_OPTN_FCPOptions *pOptions = (VSC_OPTN_FCPOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    VSC_MM*             pMM = pPassWorker->basePassWorker.pMM;
    gctBOOL             bRAEnabled = *(gctBOOL*)pPassWorker->basePassWorker.pPrvData;

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
            VIR_OpCode opCode = VIR_Inst_GetOpcode(inst);

            /* disable this when RA enabled for now, since DU and RA has not supported indexed opnd yet */
            if (!bRAEnabled)
            {
                if (opCode == VIR_OP_LDARR)
                {
                    _VIR_ReplaceLDARR(pShader, func, pDuInfo, inst);
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
                    _VIR_MergeICASTP(pDuInfo, pShader, pOptions, inst, pMM, pDumper);
                }
                else if (opCode == VIR_OP_VX_ICASTD)
                {
                    _VIR_MergeICASTD(pDuInfo, pShader, pOptions, inst, pMM, pDumper);
                }
            }

            if (VIR_Shader_isDual16Mode(pShader))
            {
                errCode = _InsertPrecisionConvInst(pShader, func, inst, pDuInfo, pMM);
                ON_ERROR(errCode, "Insert precision conversion inst");

                errCode = _InsertCMPInst(pShader, func, inst, pDuInfo, pMM);
                ON_ERROR(errCode, "Insert precision conversion inst");
            }

#if __ENABLE_OPTIMIZE_FOR_PRI_MEMORY__
            /* Only support OCL now. */
            if (VIR_Shader_IsCL(pShader) &&
                VIR_Shader_GetClientApiVersion(pShader) != gcvAPI_OPENVK &&
                VIR_Shader_GetPrivateMemorySize(pShader) > 0)
            {
                errCode = _CalculateIndexForPrivateMemory(pShader, func, inst);
                ON_ERROR(errCode, "Create concurrent workThreadCount. ");
            }
#endif

#if __ENABLE_OPTIMIZE_FOR_SHARE_MEMORY__
            /* Only support OCL now. */
            if (VIR_Shader_IsCL(pShader) &&
                VIR_Shader_GetClientApiVersion(pShader) != gcvAPI_OPENVK &&
                VIR_Shader_GetShareMemorySize(pShader) > 0)
            {
                errCode = _CalculateIndexForLocalMemory(pShader, func, inst);
                ON_ERROR(errCode, "Create concurrent workGroupCount. ");
            }
#endif
            if (!pHwCfg->hwFeatureFlags.hasImageLoadEnableFix &&
                VIR_OPCODE_isImgLd(opCode))
            {
                errCode = _FixEnableRestrictionForImageLoad(pDuInfo, pShader, func, inst);
                ON_ERROR(errCode, "Fix enable restriction for imageLoad. ");
            }
        }
    }

    if (VirSHADER_DumpCodeGenVerbose(pShader->_id))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after pre-cleanup phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
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
_changeConvSrc1(IN VIR_Instruction    *Inst)
{
    VIR_ScalarConstVal  imm0;
    VIR_Operand         *opnd = gcvNULL;
    VIR_TypeId          ty, componentTy;
    gctBOOL            needChangeSrc1 = gcvFALSE;

    if (VIR_Inst_GetOpcode(Inst) == VIR_OP_I2I)
    {
        opnd = VIR_Inst_GetDest(Inst);
    }
    else
    {
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

        if (VIR_Inst_GetOpcode(Inst) == VIR_OP_I2I)
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

DEF_QUERY_PASS_PROP(vscVIR_PostCleanup)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
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

    return VIR_Sampler_UpdateResOpBits(pShader,
                                       VIR_Symbol_GetSampler(sym),
                                       VIR_Inst_GetResOpType(texldInst),
                                       index);
}

static gctBOOL
_IsSrcFloat16(
    IN VIR_Shader*          pShader,
    IN VIR_Instruction*     pInst
    )
{
    VIR_Operand*            pSrc0Opnd = VIR_Inst_GetSource(pInst, 0);
    VIR_TypeId              typeId = VIR_Operand_GetTypeId(pSrc0Opnd);
    VIR_TypeId              componentTypeId = VIR_GetTypeComponentType(typeId);

    if (VIR_Shader_isDual16Mode(pShader) &&
        VIR_Inst_GetThreadMode(pInst) == VIR_THREAD_D16_DUAL_16)
    {
        return gcvFALSE;
    }
    else if (componentTypeId == VIR_TYPE_FLOAT16)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

/*********************************************
* vscVIR_PostCleanup
* After register allocation, perform
  1) for dual16, change the dual32 instruction
   ...
*********************************************/
VSC_ErrCode vscVIR_PostCleanup(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         status = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    VIR_Instruction*    instGetSamplerIdx = gcvNULL;

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
            if (VIR_Shader_isDual16Mode(pShader))
            {
                if (VIR_Inst_GetThreadMode(inst) == VIR_THREAD_D16_DUAL_32)
                {
                    _VIR_FCP_ReplaceDUAL32(pShader, func, inst);
                }

                /* correctly setting src1 for conv/i2i instruction  */
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_CONV ||
                    VIR_Inst_GetOpcode(inst) == VIR_OP_I2I)
                {
                    _changeConvSrc1(inst);
                }
            }

            /* Change MOV.f16 to CONV.f16. */
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_MOV && _IsSrcFloat16(pShader, inst))
            {
                VIR_Operand*    pNewOpnd = gcvNULL;
                VIR_Inst_SetOpcode(inst, VIR_OP_CONV);

                VIR_Function_NewOperand(func, &pNewOpnd);
                VIR_Inst_SetSrcNum(inst, 2);
                VIR_Inst_SetSource(inst, 1, pNewOpnd);

                VIR_Operand_SetImmediateUint(pNewOpnd, 0x1);
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
        }
    }

    if (VirSHADER_DumpCodeGenVerbose(pShader->_id))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after post-cleanup Phase\n", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return status;
}


