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

VSC_ErrCode vscVIR_RemoveNop(VIR_Shader* pShader)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

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
            if (VIR_Inst_GetOpcode(inst) == VIR_OP_NOP)
            {
                VIR_Function_RemoveInstruction(func, inst);
            }
        }
    }

    return errCode;
}

static gctBOOL _NeedPutImmValue2Uniform(VIR_Shader* pShader,
                                        VIR_Operand *Opnd,
                                        VSC_HW_CONFIG* pHwCfg,
                                        VIR_ConstVal* ImmValues,
                                        VIR_TypeId* ImmTypeId)
{
    VIR_Const*  constValue;

    memset(ImmValues, 0, sizeof(VIR_ConstVal));

    if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_IMMEDIATE)
    {
        ImmValues->scalarVal.uValue = Opnd->u1.uConst;
        gcmASSERT(VIR_TypeId_isPrimitive(VIR_Operand_GetType(Opnd)));
        *ImmTypeId = VIR_GetTypeComponentType(VIR_Operand_GetType(Opnd));

        /* For chips that do not support imm values, we must put imm to uniform */
        if (!pHwCfg->hwFeatureFlags.hasSHEnhance2)
        {
            return gcvTRUE;
        }

        /* For non-dual16 shader, imm supported by HW is only 20-bit;
           For dual16 shader, imm supported by HW is only 16-bit (two 16-bits pack into one 32-bits) */
        switch (VIR_GetTypeComponentType(VIR_Operand_GetType(Opnd)))
        {
        case VIR_TYPE_FLOAT32:
            if (VIR_Shader_isDual16Mode(pShader))
            {
                return gcvTRUE;
            }
            else
            {
                return !CAN_EXACTLY_CVT_S23E8_2_S11E8(Opnd->u1.uConst);
            }
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
            if (VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_S32_2_S16(Opnd->u1.iConst);
            }
            else
            {
                return !CAN_EXACTLY_CVT_S32_2_S20(Opnd->u1.iConst);
            }

        case VIR_TYPE_UINT32:
            if (VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_U32_2_U16(Opnd->u1.uConst);
            }
            else
            {
                return !CAN_EXACTLY_CVT_U32_2_U20(Opnd->u1.uConst);
            }
        default:
            return gcvFALSE;
        }
    }
    else if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
    {
        constValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, Opnd->u1.constId);
        *ImmValues = constValue->value;
        *ImmTypeId = constValue->type;
        return gcvTRUE;
    }

    return gcvFALSE;
}

VSC_ErrCode vscVIR_PutImmValueToUniform(VIR_Shader* pShader, VSC_HW_CONFIG* pHwCfg)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_Operand*      opnd;
    gctUINT           i;
    VIR_Const         virConst;
    VIR_ConstVal      ImmValues[VIR_MAX_SRC_NUM];
    VIR_TypeId        immTypeId[VIR_MAX_SRC_NUM];
    gctBOOL           needChange[VIR_MAX_SRC_NUM];
    gctBOOL           hasChanged[VIR_MAX_SRC_NUM];
    VIR_Uniform*      pImmUniform;
    VIR_Symbol*       sym;
    VIR_Swizzle       swizzle = VIR_SWIZZLE_XXXX;
    gctUINT           numChange;
    VIR_PrimitiveTypeId dstElemType = VIR_TYPE_VOID, dstType = VIR_TYPE_VOID;

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
            numChange = 0;
            dstElemType = VIR_TYPE_VOID;

            for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
            {
                needChange[i] = gcvFALSE;
                hasChanged[i] = gcvFALSE;
                opnd = VIR_Inst_GetSource(inst, i);

                /* check whether we need put imm value to uniform */
                if (_NeedPutImmValue2Uniform(pShader, opnd, pHwCfg, &ImmValues[i], &immTypeId[i]))
                {
                    needChange[i] = gcvTRUE;

                    /* make sure they are the same type immediate */
                    if ((dstElemType == VIR_TYPE_VOID ||
                         dstElemType == immTypeId[i]) &&
                         VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
                    {
                        dstElemType = immTypeId[i];
                        numChange++;
                    }
                }
            }
            /* to improve performance, we put the (same type) immediate into one uniform (const vector) */
            if (ENABLE_FULL_NEW_LINKER && numChange > 1)
            {
                /* generate a const vector */
                VIR_ConstVal    new_const_val;
                VIR_ConstId     new_const_id;
                VIR_Const       *new_const;
                VIR_Swizzle     new_swizzle;
                gctUINT8        constChannel = 0;

                memset(&new_const_val, 0, sizeof(VIR_ConstVal));

                for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                {
                    opnd = VIR_Inst_GetSource(inst, i);

                    /* only merge the immediate for now */
                    if (needChange[i] &&
                        immTypeId[i] == dstElemType &&
                        VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE)
                    {
                        switch (immTypeId[i])
                        {
                            case VIR_TYPE_FLOAT32:
                                new_const_val.vecVal.f32Value[constChannel] = ImmValues[i].scalarVal.fValue;
                                break;
                            case VIR_TYPE_INT32:
                            case VIR_TYPE_INT16:
                            case VIR_TYPE_INT8:
                            case VIR_TYPE_UINT32:
                            case VIR_TYPE_UINT16:
                            case VIR_TYPE_UINT8:
                            case VIR_TYPE_BOOLEAN:
                                new_const_val.vecVal.u32Value[constChannel] = ImmValues[i].scalarVal.uValue;
                                break;
                            default:
                                gcmASSERT(gcvFALSE);
                                break;
                        }
                        constChannel ++;
                    }
                }

                gcmASSERT(constChannel == numChange);

                dstType = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(dstElemType), numChange, 1);
                VIR_Shader_AddConstant(pShader, dstType, &new_const_val, &new_const_id);
                new_const = VIR_Shader_GetConstFromId(pShader, new_const_id);
                new_const->type = dstType;
                VIR_Shader_AddInitializedConstUniform(pShader, new_const, &pImmUniform);
                sym = VIR_Shader_GetSymFromId(pShader, pImmUniform->sym);

                constChannel = 0;
                for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                {
                    opnd = VIR_Inst_GetSource(inst, i);

                    if (needChange[i] && immTypeId[i] == dstElemType)
                    {
                        if (constChannel == 0)
                        {
                            new_swizzle = VIR_SWIZZLE_XXXX;
                        }
                        else if (constChannel == 1)
                        {
                            new_swizzle = VIR_SWIZZLE_YYYY;
                        }
                        else if (constChannel == 2)
                        {
                            new_swizzle = VIR_SWIZZLE_ZZZZ;
                        }
                        else
                        {
                            new_swizzle = VIR_SWIZZLE_WWWW;
                        }
                        VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
                        opnd->u1.sym = sym;
                        VIR_Operand_SetType(opnd, dstType);
                        VIR_Operand_SetSwizzle(opnd, new_swizzle);
                        constChannel++;
                        hasChanged[i] = gcvTRUE;
                    }
                }
            }

            for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
            {
                opnd = VIR_Inst_GetSource(inst, i);

                /* If we need put imm value to uniform, just do it right now */
                if (needChange[i] && !hasChanged[i])
                {
                    gcmASSERT(VIR_Operand_GetOpKind(opnd) == VIR_OPND_IMMEDIATE ||
                              VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST);

                    /* Try to search uniform that holds this imm value, if not found,
                        create a new uniform */
                    virConst.index = VIR_INVALID_ID;
                    virConst.type = immTypeId[i];
                    virConst.value = ImmValues[i];
                    VIR_Shader_AddInitializedUniform(pShader, &virConst, &pImmUniform, &swizzle);
                    /* Mapping the swizzle for a VECTOR constant. */
                    if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_CONST)
                    {
                        swizzle = VIR_Swizzle_MergeMappingSwizzles(VIR_Operand_GetSwizzle(opnd),
                                                                    swizzle);
                    }
                    /* Set this uniform as operand and set correct swizzle */
                    sym = VIR_Shader_GetSymFromId(pShader, pImmUniform->sym);
                    VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
                    opnd->u1.sym = sym;
                    VIR_Operand_SetSwizzle(opnd, swizzle);
                }
            }
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VIR_Shader* pShader, VSC_HW_CONFIG* pHwCfg)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_Operand*      opnd;
    gctUINT           i, j, firstConstRegNo, thisConstRegNo, newDstRegNo;
    VIR_SymId         newDstSymId;
    VIR_Symbol*       pSym = gcvNULL, *pNewSym = gcvNULL;
    VIR_Uniform*      pUniform;
    VIR_Instruction*  pNewInsertedInst;
    gctBOOL           bFirstConstRegIndexing, bHitReadPortLimitation;

    gcmASSERT(VIR_Shader_isConstRegAllocated(pShader));

    /* If has no const reg read port limitation, just bail out */
    if (pHwCfg->hwFeatureFlags.noOneConstLimit)
    {
        return errCode;
    }

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
            if (VIR_Inst_GetSrcNum(inst) < 2)
            {
                continue;
            }

            for (i = 0; i < VIR_Inst_GetSrcNum(inst) - 1; ++i)
            {
                firstConstRegNo = NOT_ASSIGNED;
                bFirstConstRegIndexing = gcvFALSE;

                for (j = 0; j < VIR_Inst_GetSrcNum(inst); ++j)
                {
                    opnd = VIR_Inst_GetSource(inst, j);

                    pUniform = gcvNULL;
                    if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
                    {
                        pSym = VIR_Operand_GetSymbol(opnd);

                        if (VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
                        {
                            pUniform = VIR_Symbol_GetUniform(pSym);
                        }
                        else if (VIR_Symbol_GetKind(pSym) == VIR_SYM_IMAGE)
                        {
                            pUniform = VIR_Symbol_GetImage(pSym);
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if (pUniform)
                    {
                        bHitReadPortLimitation = gcvFALSE;

                        thisConstRegNo = pUniform->physical +
                                         VIR_Operand_GetMatrixConstIndex(opnd) +
                                         ((VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE) ?
                                          VIR_Operand_GetRelIndexing(opnd) : 0);

                        /* Check whether shader hits the limitation that HW does not support two read port on reg file. */
                        if (firstConstRegNo == NOT_ASSIGNED)
                        {
                            firstConstRegNo = thisConstRegNo;
                            bFirstConstRegIndexing = (VIR_Operand_GetRelAddrMode(opnd) != VIR_INDEXED_NONE);
                        }
                        else
                        {
                            if (!bFirstConstRegIndexing && VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE)
                            {
                                if (firstConstRegNo != thisConstRegNo)
                                {
                                    bHitReadPortLimitation = gcvTRUE;
                                }
                            }
                            else if (bFirstConstRegIndexing || VIR_Operand_GetRelAddrMode(opnd) != VIR_INDEXED_NONE)
                            {
                                bHitReadPortLimitation = gcvTRUE;
                            }
                        }

                        if (bHitReadPortLimitation)
                        {
                            VIR_Precision precision = VIR_Operand_GetPrecision(opnd);

                            /* Add a new-temp-reg number */
                            newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
                            errCode = VIR_Shader_AddSymbol(pShader,
                                                            VIR_SYM_VIRREG,
                                                            newDstRegNo,
                                                            VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(opnd)),
                                                            VIR_STORAGE_UNKNOWN,
                                                            &newDstSymId);
                            ON_ERROR(errCode, "Add symbol");
                            pNewSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

                            /* Add following inst just before current inst:

                                mov new-temp-reg, uniform
                            */
                            errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_Operand_GetType(opnd), inst, &pNewInsertedInst);
                            ON_ERROR(errCode, "Add instruction before");

                            /* dst */
                            VIR_Operand_SetSymbol(pNewInsertedInst->dest, func, newDstSymId);
                            VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_XYZW);
                            /*if (!(VIR_Operand_isSymbol(opnd) &&
                                  VIR_Symbol_isUniform(VIR_Operand_GetSymbol(opnd)) &&
                                  (VIR_GetTypeFlag(VIR_Operand_GetType(opnd)) & VIR_TYFLAG_ISFLOAT) != 0) &&
                                VIR_Operand_GetPrecision(opnd) == VIR_PRECISION_HIGH)
                            {
                                VIR_Symbol_SetPrecision(pNewSym, VIR_PRECISION_HIGH);
                                VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                            }*/
                            VIR_Symbol_SetPrecision(pNewSym, precision);
                            VIR_Operand_SetPrecision(pNewInsertedInst->dest, precision);
                            if(precision == VIR_PRECISION_HIGH)
                            {
                                VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                            }

                            /* src */
                            VIR_Operand_SetSymbol(pNewInsertedInst->src[VIR_Operand_Src0], func, pSym->index);
                            VIR_Operand_SetSwizzle(pNewInsertedInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XYZW);
                            VIR_Operand_SetType(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetType(opnd));
                            VIR_Operand_SetPrecision(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetPrecision(opnd));
                            VIR_Operand_SetMatrixConstIndex(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetMatrixConstIndex(opnd));
                            VIR_Operand_SetIsConstIndexing(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetIsConstIndexing(opnd));
                            VIR_Operand_SetRelAddrMode(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetRelAddrMode(opnd));
                            if (VIR_Operand_GetIsConstIndexing(opnd))
                            {
                                VIR_Operand_SetRelIndexingImmed(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetRelIndexing(opnd));
                            }
                            else
                            {
                                VIR_Operand_SetRelIndexing(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Operand_GetRelIndexing(opnd));
                            }

                            /* Change operand of current inst to new-temp-reg */
                            VIR_Operand_SetMatrixConstIndex(opnd, 0);
                            VIR_Operand_SetRelAddrMode(opnd, VIR_INDEXED_NONE);
                            VIR_Operand_SetRelIndexing(opnd, 0);
                            VIR_Operand_SetTempRegister(opnd, func, newDstSymId, VIR_Operand_GetType(opnd));
                            if (precision == VIR_PRECISION_HIGH)
                            {
                                VIR_Inst_SetThreadMode(inst, VIR_THREAD_D16_DUAL_32);
                            }
                        }
                    }
                }
            }
        }
    }

OnError:
    return errCode;
}

static gctBOOL _IsPosAndDepthConflicted(VIR_Shader* pShader,
                                        VIR_Symbol* pPosSym,
                                        VIR_Symbol* pDepthSym,
                                        VIR_DEF_USAGE_INFO* pDuInfo)
{
    gctBOOL                  bIsConflicted = gcvFALSE;
    VIR_DEF_KEY              defKey;
    gctUINT                  posZDefIdx, depDefIdx;
    VIR_USAGE*               pUsage = gcvNULL;
    VIR_DEF*                 pPosZDef = gcvNULL;
    VIR_DEF*                 pDepDef = gcvNULL;
    VSC_DU_ITERATOR          duIter;
    VIR_DU_CHAIN_USAGE_NODE* pUsageNode;
    VSC_BIT_VECTOR           depDefIdxMask;
    VSC_BIT_VECTOR           tempMask;
    VIR_TS_FUNC_FLOW*        pFuncFlow;
    VIR_TS_BLOCK_FLOW*       pBlockFlow;
    VIR_Instruction*         pInst;
    VIR_Instruction*         pUsageInst;
    VIR_Operand*             pDstOpnd;
    VIR_Symbol*              pDstSym;

    defKey.pDefInst = VIR_INPUT_DEF_INST;
    defKey.regNo = pPosSym->u2.tempIndex;
    defKey.channel = VIR_CHANNEL_Z;
    posZDefIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

    /* If no pos_z is defined, just bail out */
    if (VIR_INVALID_DEF_INDEX == posZDefIdx)
    {
        return gcvFALSE;
    }

    /* If pos_z is defined, but not used, just bail out */
    pPosZDef = GET_DEF_BY_IDX(&pDuInfo->defTable, posZDefIdx);
    gcmASSERT(pPosZDef);
    if (DU_CHAIN_GET_USAGE_COUNT(&pPosZDef->duChain) == 0)
    {
        return gcvFALSE;
    }

    /* If reach-def flow is invalidated, just always consider they are conflicted each other */
    if (pDuInfo->baseTsDFA.baseDFA.cmnDfaFlags.bFlowInvalidated)
    {
        return gcvTRUE;
    }

    vscBV_Initialize(&tempMask, &pShader->mempool, pDuInfo->baseTsDFA.baseDFA.flowSize);
    vscBV_Initialize(&depDefIdxMask, &pShader->mempool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    /* Collect all depth defs and mask them */
    defKey.pDefInst = VIR_ANY_DEF_INST;
    defKey.regNo = pDepthSym->u2.tempIndex;
    defKey.channel = VIR_CHANNEL_ANY;
    depDefIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);
    while (VIR_INVALID_DEF_INDEX != depDefIdx)
    {
        pDepDef = GET_DEF_BY_IDX(&pDuInfo->defTable, depDefIdx);
        gcmASSERT(pDepDef);

        if (pDepDef->flags.nativeDefFlags.bIsOutput)
        {
            vscBV_SetBit(&depDefIdxMask, depDefIdx);
        }

        depDefIdx = pDepDef->nextDefIdxOfSameRegNo;
    }

    /* For each usage of pos_z, check wether it is conflicted with depth */
    VSC_DU_ITERATOR_INIT(&duIter, &pPosZDef->duChain);
    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
    {
        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
        pUsageInst = pUsage->usageKey.pUsageInst;

        pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pDuInfo->baseTsDFA.tsFuncFlowArray,
                                                           VIR_Inst_GetFunction(pUsageInst)->pFuncBlock->dgNode.id);
        pBlockFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pFuncFlow->tsBlkFlowArray,
                                                             VIR_Inst_GetBasicBlock(pUsageInst)->dgNode.id);

        vscBV_And2(&tempMask, &pBlockFlow->inFlow, &depDefIdxMask);

        if (vscBV_Any(&tempMask))
        {
            bIsConflicted = gcvTRUE;
            break;
        }
        else
        {
            depDefIdx = 0;
            while ((depDefIdx = vscBV_FindSetBitForward(&depDefIdxMask, depDefIdx)) != (gctUINT)INVALID_BIT_LOC)
            {
                pDepDef = GET_DEF_BY_IDX(&pDuInfo->defTable, depDefIdx);
                gcmASSERT(pDepDef);

                if (VIR_Inst_GetBasicBlock(pUsageInst) == VIR_Inst_GetBasicBlock(pDepDef->defKey.pDefInst))
                {
                    pInst = BB_GET_START_INST(VIR_Inst_GetBasicBlock(pUsageInst));

                    while (pInst)
                    {
                        if (pInst == pUsageInst)
                        {
                            /* For dual16 shader, as RA has treated t0t1 dst as one web, we have to consider
                               the case that depth-def and usage of pos_z are in the same inst as an candidate.
                               We should always break here later when RA uses correct dual16-related DFA later */
                            if (!VIR_Shader_isDual16Mode(pShader))
                            {
                                break;
                            }
                        }

                        pDstOpnd = VIR_Inst_GetDest(pInst);

                        if (!pDstOpnd)
                        {
                            continue;
                        }

                        pDstSym = VIR_Operand_GetSymbol(pDstOpnd);

                        if (VIR_Symbol_GetKind(pDstSym) == VIR_SYM_VIRREG)
                        {
                            pDstSym = VIR_Symbol_GetVregVariable(pDstSym);
                            if (pDstSym && VIR_Symbol_GetName(pDstSym) == VIR_NAME_DEPTH)
                            {
                                bIsConflicted = gcvTRUE;
                                break;
                            }
                        }

                        /* If current inst is the last inst of block, just bail out */
                        if (pInst == BB_GET_END_INST(VIR_Inst_GetBasicBlock(pUsageInst)) ||
                            pInst == pUsageInst)
                        {
                            break;
                        }

                        /* Move to next inst */
                        pInst = VIR_Inst_GetNext(pInst);
                    }

                    if (bIsConflicted == gcvTRUE)
                    {
                        break;
                    }
                }

                depDefIdx ++;
            }

            if (bIsConflicted == gcvTRUE)
            {
                break;
            }
        }
    }

    vscBV_Finalize(&tempMask);
    vscBV_Finalize(&depDefIdxMask);

    return bIsConflicted;
}

/* Pos and depth might be overlapped (conflicted). As HW needs postion is put into r0 and
   depth is put into r0.z, so if such overlapp is on Z, then we will encounter error. If
   such overlap happens, we need break it by inserting an extra MOV at the end of shader
   for depth (it is ok if inserting an extra MOV at the begin of shader for pos) */
VSC_ErrCode vscVIR_CheckPosAndDepthConflict(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_AttributeIdList*       pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
    VIR_OutputIdList*          pOutputIdLsts = VIR_Shader_GetOutputs(pShader);
    VIR_Symbol*                pIoSym = gcvNULL;
    VIR_Symbol*                pDepthSym = gcvNULL;
    VIR_Symbol*                pPosSym = gcvNULL;
    gctUINT                    ioIdx, ioSymId = VIR_INVALID_ID, newRegNo, newRegSymId = VIR_INVALID_ID;
    VIR_Symbol*                pNewRegSym = gcvNULL;
    VIR_Instruction*           pNewInsertedInst = gcvNULL;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          func_node;
    VIR_TypeId                 depthTypeId;
    VIR_NATIVE_DEF_FLAGS       nativeDefFlags;

    if (pShader->shaderKind != VIR_SHADER_FRAGMENT)
    {
        return VSC_ERR_NONE;
    }

    /* Does shader have pos? */
    for (ioIdx = 0; ioIdx < VIR_IdList_Count(pAttrIdLsts); ioIdx ++)
    {
        ioSymId = VIR_IdList_GetId(pAttrIdLsts, ioIdx);
        pIoSym = VIR_Shader_GetSymFromId(pShader, ioSymId);

        if (!isSymUnused(pIoSym) && !isSymVectorizedOut(pIoSym))
        {
            if (VIR_Symbol_GetName(pIoSym) == VIR_NAME_POSITION)
            {
                pPosSym = pIoSym;
                break;
            }
        }
    }

    if (ioIdx == VIR_IdList_Count(pAttrIdLsts))
    {
        return VSC_ERR_NONE;
    }

    /* Does shader have depth? */
    for (ioIdx = 0; ioIdx < VIR_IdList_Count(pOutputIdLsts); ioIdx ++)
    {
        ioSymId = VIR_IdList_GetId(pOutputIdLsts, ioIdx);
        pIoSym = VIR_Shader_GetSymFromId(pShader, ioSymId);

        if (!isSymUnused(pIoSym) && !isSymVectorizedOut(pIoSym))
        {
            if (VIR_Symbol_GetName(pIoSym) == VIR_NAME_DEPTH)
            {
                pDepthSym = pIoSym;
                break;
            }
        }
    }

    if (ioIdx == VIR_IdList_Count(pOutputIdLsts))
    {
        return VSC_ERR_NONE;
    }

    /* Check whether position and depth is conflicted */
    if (!_IsPosAndDepthConflicted(pShader, pPosSym, pDepthSym, pDuInfo))
    {
        return VSC_ERR_NONE;
    }

    /* Add following inst at the end of main routine:

       mov depth, temp
    */
    errCode = VIR_Function_AddInstruction(pShader->mainFunction,
                                          VIR_OP_MOV,
                                          VIR_TYPE_FLOAT32,
                                          &pNewInsertedInst);
    ON_ERROR(errCode, "add instruction");

    newRegNo = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   newRegNo,
                                   VIR_Symbol_GetType(pDepthSym),
                                   VIR_STORAGE_UNKNOWN,
                                   &newRegSymId);
    ON_ERROR(errCode, "Add symbol");
    pNewRegSym = VIR_Shader_GetSymFromId(pShader, newRegSymId);

    /* src0 */
    depthTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pDepthSym));
    VIR_Operand_SetTempRegister(pNewInsertedInst->src[VIR_Operand_Src0], pShader->mainFunction, newRegSymId, depthTypeId);
    VIR_Operand_SetSwizzle(pNewInsertedInst->src[VIR_Operand_Src0], VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(pNewInsertedInst->src[VIR_Operand_Src0], VIR_Symbol_GetPrecision(pDepthSym));
    VIR_Symbol_SetPrecision(pNewRegSym, VIR_Symbol_GetPrecision(pDepthSym));

    /* dst */
    VIR_Operand_SetSymbol(pNewInsertedInst->dest, pShader->mainFunction, pDepthSym->index);
    VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(pNewInsertedInst->dest, VIR_Symbol_GetPrecision(pDepthSym));

    memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
    nativeDefFlags.bIsOutput = gcvTRUE;
    vscVIR_AddNewDef(pDuInfo, pNewInsertedInst, pDepthSym->u2.tempIndex, 1, VIR_ENABLE_X, VIR_HALF_CHANNEL_MASK_FULL,
                     &nativeDefFlags, gcvNULL);

    vscVIR_AddNewUsageToDef(pDuInfo, pNewInsertedInst, VIR_OUTPUT_USAGE_INST,
                            (VIR_Operand*)(gctUINTPTR_T)pDepthSym->u2.tempIndex,
                            gcvFALSE, pDepthSym->u2.tempIndex, 1, VIR_ENABLE_X,
                            VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

    /* Set dual16 thread mode */
    if (VIR_Shader_isDual16Mode(pShader))
    {
        VIR_Inst_SetThreadMode(pNewInsertedInst, (VIR_Symbol_GetPrecision(pDepthSym) == VIR_PRECISION_HIGH) ?
                                                 VIR_THREAD_D16_DUAL_32 : VIR_THREAD_D16_DUAL_16);
    }

    /* Change the def of depth to new-reg */
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
            VIR_Operand *dstOpnd = VIR_Inst_GetDest(inst);
            VIR_Symbol  *dstSym = gcvNULL;

            if (inst == pNewInsertedInst || !VIR_Inst_GetDest(inst))
            {
                continue;
            }

            dstSym = VIR_Operand_GetSymbol(dstOpnd);

            if (VIR_Symbol_GetKind(dstSym) == VIR_SYM_VIRREG)
            {
                dstSym = VIR_Symbol_GetVregVariable(dstSym);
                if (dstSym && VIR_Symbol_GetName(dstSym) == VIR_NAME_DEPTH)
                {
                    gcmASSERT(VIR_Operand_GetEnable(dstOpnd) == VIR_ENABLE_X);
                    VIR_Operand_SetTempRegister(dstOpnd, func, newRegSymId, depthTypeId);

                    vscVIR_DeleteDef(pDuInfo, inst, pDepthSym->u2.tempIndex, 1, VIR_ENABLE_X,
                                     VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

                    memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
                    vscVIR_AddNewDef(pDuInfo, inst, newRegNo, 1, VIR_ENABLE_X, VIR_HALF_CHANNEL_MASK_FULL,
                                     &nativeDefFlags, gcvNULL);

                    vscVIR_AddNewUsageToDef(pDuInfo, inst, pNewInsertedInst, pNewInsertedInst->src[VIR_Operand_Src0],
                                            gcvFALSE, newRegNo, 1, VIR_ENABLE_X, VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
                }
            }
        }
    }

OnError:
    return errCode;
}

static VIR_Uniform* _FindMemBaseUniform(VIR_DEF_USAGE_INFO* pDuInfo,
                                        VIR_Instruction* pMemInst,
                                        VIR_Operand* pOpnd)
{
    VIR_DEF*                  pDef;
    VIR_GENERAL_UD_ITERATOR   udIter;
    gctUINT                   i;
    VIR_Operand*              pThisOpnd;
    VIR_Symbol*               pSym = gcvNULL;
    VIR_OperandInfo           operandInfo;

    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pMemInst, pOpnd, gcvFALSE, gcvFALSE);

    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
         pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        for (i = 0; i < VIR_Inst_GetSrcNum(pDef->defKey.pDefInst); ++i)
        {
            pThisOpnd = VIR_Inst_GetSource(pDef->defKey.pDefInst, i);
            pSym = VIR_Operand_GetSymbol(pThisOpnd);

            if (VIR_Operand_GetOpKind(pThisOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
            {
                return VIR_Symbol_GetUniform(pSym);
            }
            else
            {
                VIR_Operand_GetOperandInfo(pDef->defKey.pDefInst, pThisOpnd, &operandInfo);

                if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
                {
                    return _FindMemBaseUniform(pDuInfo, pDef->defKey.pDefInst, pThisOpnd);
                }
            }
        }
    }

    return gcvNULL;
}

VSC_ErrCode vscVIR_AddOutOfBoundCheckSupport(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo, VSC_HW_CONFIG* pHwCfg)
{
    VSC_ErrCode               errCode = VSC_ERR_NONE;
    VIR_FuncIterator          func_iter;
    VIR_FunctionNode*         func_node;
    VIR_Function*             func;
    VIR_InstIterator          inst_iter;
    VIR_Instruction*          inst;
    VIR_Operand*              pOpnd;
    VIR_Uniform*              pMemBaseUniform;
    VIR_Symbol*               pSym = gcvNULL, *pNewSym = gcvNULL, *pSymForMemBaseUniform;
    VIR_Symbol*               pUnderlyingSym = gcvNULL;
    gctBOOL                   bNeedInsertMov;
    VIR_OperandInfo           operandInfo;
    gctUINT                   newDstRegNo, i;
    VIR_SymId                 newDstSymId;
    VIR_Instruction*          pNewInsertedInstX;
    VIR_Instruction*          pNewInsertedInstY;
    VIR_Instruction*          pNewInsertedInstZ;
    VIR_OperandInfo           srcInfo;
    VIR_GENERAL_UD_ITERATOR   udIter;
    VIR_DEF*                  pDef;

    /* If HW does not support OOB-check, just bail out */
    if (!pHwCfg->hwFeatureFlags.supportOOBCheck)
    {
        return VSC_ERR_NONE;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            /* Uniform for SSBO base-addr has Y channel to store size of SSBO, now with OOB enable,
               we need move it to W channel because XYZ will be use as base/low-limit/upper-limit */
            for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++ i)
            {
                pOpnd = VIR_Inst_GetSource(inst, i);
                pSym = VIR_Operand_GetSymbol(pOpnd);

                if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
                {
                    if (VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_STORAGE_BLOCK_ADDRESS)
                    {
                        if (VIR_Operand_GetSwizzle(pOpnd) == VIR_SWIZZLE_YYYY)
                        {
                            VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_WWWW);
                        }
                    }
                }
            }

            /* Only consider L/S/Atom */
            if (!VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(inst)) &&
                !VIR_OPCODE_isMemSt(VIR_Inst_GetOpcode(inst)) &&
                !VIR_OPCODE_isAtom(VIR_Inst_GetOpcode(inst)))
            {
                continue;
            }

            pOpnd = VIR_Inst_GetSource(inst, 0);

            /* Try to find corresponding mem-base-addr uniform */
            pMemBaseUniform = gcvNULL;
            pSym = VIR_Operand_GetSymbol(pOpnd);
            bNeedInsertMov = gcvFALSE;
            if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
            {
                pMemBaseUniform = VIR_Symbol_GetUniform(pSym);
            }
            else
            {
                VIR_Operand_GetOperandInfo(inst, pOpnd, &operandInfo);

                if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
                {
                    pMemBaseUniform = _FindMemBaseUniform(pDuInfo, inst, pOpnd);
                    bNeedInsertMov = gcvTRUE;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }
            }

            gcmASSERT(pMemBaseUniform);

            pSymForMemBaseUniform = VIR_Shader_GetSymFromId(pShader, pMemBaseUniform->sym);

            /* Change uniform type to vec2 to hold mem-size in Y channel. */
            if (VIR_Symbol_HasFlag(pSymForMemBaseUniform, VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER))
            {
                pUnderlyingSym = VIR_Shader_GetSymFromId(pShader, pMemBaseUniform->baseBindingUniform->sym);

                gcmASSERT((VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pUnderlyingSym))) &
                           VIR_TYFLAG_ISINTEGER));

                VIR_Symbol_SetType(pUnderlyingSym, VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_ATOMIC_UINT4));
            }
            else
            {
                gcmASSERT((VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pSymForMemBaseUniform))) &
                           VIR_TYFLAG_ISINTEGER));

                VIR_Symbol_SetType(pSymForMemBaseUniform, VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4));
            }

            /* If src0 is not mem-base uniform, insert following 3 movs, and cast src0 to new dst-of-mov
               1. mov from src0.x,
               2. mov from Y channel of mem-base-addr uniform (low-limit)
               2. mov from Z channel of mem-base-addr uniform (upper-limit) */
            if (bNeedInsertMov)
            {
                /* Add a new-temp-reg number */
                newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);
                errCode = VIR_Shader_AddSymbol(pShader,
                                               VIR_SYM_VIRREG,
                                               newDstRegNo,
                                               VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                               VIR_STORAGE_UNKNOWN,
                                               &newDstSymId);
                ON_ERROR(errCode, "Add symbol");
                pNewSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

                VIR_Symbol_SetPrecision(pNewSym, VIR_Operand_GetPrecision(pOpnd));

                /* Add following inst just before current inst:

                   mov new-temp-reg.x, src0.x of L/S/Atom
                */
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, &pNewInsertedInstX);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                VIR_Operand_SetSymbol(pNewInsertedInstX->dest, func, newDstSymId);
                VIR_Operand_SetEnable(pNewInsertedInstX->dest, VIR_ENABLE_X);
                VIR_Operand_SetPrecision(pNewInsertedInstX->dest, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                VIR_Operand_SetSymbol(pNewInsertedInstX->src[VIR_Operand_Src0], func, pSym->index);
                VIR_Operand_SetSwizzle(pNewInsertedInstX->src[VIR_Operand_Src0], VIR_Operand_GetSwizzle(pOpnd));
                VIR_Operand_SetType(pNewInsertedInstX->src[VIR_Operand_Src0], VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(pNewInsertedInstX->src[VIR_Operand_Src0], VIR_Operand_GetPrecision(pOpnd));

                vscVIR_AddNewDef(pDuInfo,
                                 pNewInsertedInstX,
                                 newDstRegNo,
                                 1,
                                 VIR_ENABLE_X,
                                 VIR_HALF_CHANNEL_MASK_FULL,
                                 gcvNULL,
                                 gcvNULL);

                VIR_Operand_GetOperandInfo(inst, pOpnd, &srcInfo);

                vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, inst, pOpnd, gcvFALSE, gcvFALSE);

                for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                     pDef != gcvNULL;
                     pDef = vscVIR_GeneralUdIterator_Next(&udIter))
                {
                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pNewInsertedInstX,
                                            pNewInsertedInstX->src[VIR_Operand_Src0],
                                            gcvFALSE,
                                            srcInfo.u1.virRegInfo.virReg,
                                            1,
                                            1 << pDef->defKey.channel,
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }

                /* Add following inst just before current inst:

                   mov new-temp-reg.y, pMemBaseUniform.y
                */
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, &pNewInsertedInstY);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                VIR_Operand_SetSymbol(pNewInsertedInstY->dest, func, newDstSymId);
                VIR_Operand_SetEnable(pNewInsertedInstY->dest, VIR_ENABLE_Y);
                VIR_Operand_SetPrecision(pNewInsertedInstY->dest, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                VIR_Operand_SetSymbol(pNewInsertedInstY->src[VIR_Operand_Src0], func, pMemBaseUniform->sym);
                VIR_Operand_SetSwizzle(pNewInsertedInstY->src[VIR_Operand_Src0], VIR_SWIZZLE_YYYY);
                VIR_Operand_SetType(pNewInsertedInstY->src[VIR_Operand_Src0], VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(pNewInsertedInstY->src[VIR_Operand_Src0], VIR_Operand_GetPrecision(pOpnd));

                vscVIR_AddNewDef(pDuInfo,
                                 pNewInsertedInstY,
                                 newDstRegNo,
                                 1,
                                 VIR_ENABLE_Y,
                                 VIR_HALF_CHANNEL_MASK_FULL,
                                 gcvNULL,
                                 gcvNULL);

                /* Add following inst just before current inst:

                   mov new-temp-reg.z, pMemBaseUniform.z
                */
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, &pNewInsertedInstZ);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                VIR_Operand_SetSymbol(pNewInsertedInstZ->dest, func, newDstSymId);
                VIR_Operand_SetEnable(pNewInsertedInstZ->dest, VIR_ENABLE_Z);
                VIR_Operand_SetPrecision(pNewInsertedInstZ->dest, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                VIR_Operand_SetSymbol(pNewInsertedInstZ->src[VIR_Operand_Src0], func, pMemBaseUniform->sym);
                VIR_Operand_SetSwizzle(pNewInsertedInstZ->src[VIR_Operand_Src0], VIR_SWIZZLE_ZZZZ);
                VIR_Operand_SetType(pNewInsertedInstZ->src[VIR_Operand_Src0], VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(pNewInsertedInstZ->src[VIR_Operand_Src0], VIR_Operand_GetPrecision(pOpnd));

                vscVIR_AddNewDef(pDuInfo,
                                 pNewInsertedInstZ,
                                 newDstRegNo,
                                 1,
                                 VIR_ENABLE_Z,
                                 VIR_HALF_CHANNEL_MASK_FULL,
                                 gcvNULL,
                                 gcvNULL);

                /* Change operand of current inst to new-temp-reg */
                VIR_Operand_SetMatrixConstIndex(pOpnd, 0);
                VIR_Operand_SetRelAddrMode(pOpnd, VIR_INDEXED_NONE);
                VIR_Operand_SetRelIndexing(pOpnd, 0);
                VIR_Operand_SetTempRegister(pOpnd, func, newDstSymId, VIR_TYPE_UINT_X3);

                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pNewInsertedInstX,
                                        inst,
                                        pOpnd,
                                        gcvFALSE,
                                        newDstRegNo,
                                        1,
                                        VIR_ENABLE_X,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);

                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pNewInsertedInstY,
                                        inst,
                                        pOpnd,
                                        gcvFALSE,
                                        newDstRegNo,
                                        1,
                                        VIR_ENABLE_Y,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);

                vscVIR_AddNewUsageToDef(pDuInfo,
                                        pNewInsertedInstZ,
                                        inst,
                                        pOpnd,
                                        gcvFALSE,
                                        newDstRegNo,
                                        1,
                                        VIR_ENABLE_Z,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }

            /* Change the swizzle of src0 to XYZ */
            VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYZZ);
        }
    }

OnError:
    return errCode;
}

void
_Inst_ChangeOpnd2HP(
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    gctBOOL             isDest,
    gctBOOL             skipLowP,
    VIR_DEF_USAGE_INFO  *pDuInfo
    )
{
    VIR_USAGE_KEY       usageKey;
    VIR_DEF_KEY         defKey;
    gctUINT             usageIdx, defIdx, i;
    VIR_USAGE           *pUsage = gcvNULL;
    VIR_DEF*            pDef = gcvNULL;

    VIR_OperandInfo     operandInfo;
    VSC_DU_ITERATOR     duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;

    VIR_Operand_GetOperandInfo(
        pInst,
        pOpnd,
        &operandInfo);

    /* we don't change the precision for lowp */
    if (VIR_Operand_GetPrecision(pOpnd) != VIR_PRECISION_HIGH &&
        (!skipLowP || VIR_Operand_GetPrecision(pOpnd) != VIR_PRECISION_LOW))
    {
        VIR_Symbol  *sym = VIR_Operand_GetSymbol(pOpnd);
        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
        if (VIR_Symbol_isVreg(sym) && VIR_Symbol_GetVregVariable(sym))
        {
            VIR_Symbol_SetPrecision(VIR_Symbol_GetVregVariable(sym),
                VIR_PRECISION_HIGH);
        }
        VIR_Operand_SetPrecision(pOpnd, VIR_PRECISION_HIGH);

        if (!isDest)
        {
            /* change all its def to highp */
            usageKey.pUsageInst = pInst;
            usageKey.pOperand = pOpnd;
            usageKey.bIsIndexingRegUsage = gcvFALSE;
            usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);

            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
                gcmASSERT(pUsage);

                for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                    if (pDef->defKey.pDefInst != VIR_INPUT_DEF_INST)
                    {
                        VIR_Operand_SetPrecision(VIR_Inst_GetDest(pDef->defKey.pDefInst),
                                                 VIR_PRECISION_HIGH);
                    }

                    /* change def's uses to highp */
                    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                    {
                        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
                        if (pUsage->usageKey.pUsageInst != VIR_OUTPUT_USAGE_INST)
                        {
                            VIR_Operand_SetPrecision(pUsage->usageKey.pOperand, VIR_PRECISION_HIGH);
                        }
                    }
                }
            }
        }
        else
        {
            /* change all its uses to highp */
            defKey.pDefInst = pInst;
            defKey.regNo = operandInfo.u1.virRegInfo.virReg;
            defKey.channel = VIR_CHANNEL_ANY;
            defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

            while (VIR_INVALID_DEF_INDEX != defIdx)
            {
                pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                /* go through all the uses */
                VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                {
                    pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
                    if (pUsage->usageKey.pUsageInst != VIR_OUTPUT_USAGE_INST)
                    {
                        VIR_Operand_SetPrecision(pUsage->usageKey.pOperand, VIR_PRECISION_HIGH);
                    }
                }

                defIdx = pDef->nextDefIdxOfSameRegNo;
            }
        }
    }
}

gctBOOL
_Inst_RequireHPDest(
    VIR_Instruction *pInst,
    gctBOOL         forceChange)
{
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(pInst);
    gctUINT         dual16PrecisionRule = gcmOPT_DualFP16PrecisionRule();

    /* promoting to highp per HW requirements */
    {
        if (VIR_OPCODE_isMemSt(opcode)  ||
            opcode == VIR_OP_IMG_STORE  ||
            opcode == VIR_OP_VX_IMG_STORE  ||
            opcode == VIR_OP_IMG_STORE_3D ||
            opcode == VIR_OP_MOVA ||
            (VIR_OPCODE_isTexLd(opcode) && forceChange) ||
            (VIR_OPCODE_BITWISE(opcode) && forceChange))
        {
            return gcvTRUE;
        }

        if (opcode == VIR_OP_SELECT &&
            forceChange &&
            VIR_Inst_GetConditionOp(pInst) != VIR_COP_SELMSB &&
            VIR_Inst_GetConditionOp(pInst) != VIR_COP_ALLMSB &&
            VIR_Inst_GetConditionOp(pInst) != VIR_COP_ANYMSB)
        {
            return gcvTRUE;
        }
    }

    if (dual16PrecisionRule & Dual16_PrecisionRule_RCP_HP)
    {
        if (opcode == VIR_OP_RCP)
        {
            return gcvTRUE;
        }
    }

    if (dual16PrecisionRule & Dual16_PrecisionRule_FRAC_HP)
    {
        if (opcode == VIR_OP_FRAC)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* According to several rules, promote the source to be highp.
   For some cases, the source's precision change implies the destination
   needs to change to highp too. In that case, set forceChange to true.
*/
gctBOOL
_Inst_RequireHPSrc(
    VIR_Instruction     *pInst,
    gctUINT             sourceIdx,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    gctBOOL             *forceChange,
    gctBOOL             *skipLowp)
{
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand     *srcOpnd = VIR_Inst_GetSource(pInst, sourceIdx);
    VIR_Symbol      *srcSym = VIR_Operand_GetUnderlyingSymbol(srcOpnd);
    gctUINT         dual16PrecisionRule = gcmOPT_DualFP16PrecisionRule();

    if (VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_VIRREG &&
        VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SAMPLER_INDEXING)
    {
        return gcvFALSE;
    }

    /* promoting to highp per HW requirements */
    {
        if (VIR_OPCODE_BITWISE(opcode))
        {
            if (*forceChange)
            {
                return gcvTRUE;
            }
        }

        if (opcode == VIR_OP_SELECT)
        {
            if (VIR_Inst_GetConditionOp(pInst) == VIR_COP_SELMSB ||
                VIR_Inst_GetConditionOp(pInst) == VIR_COP_ALLMSB ||
                VIR_Inst_GetConditionOp(pInst) == VIR_COP_ANYMSB
                )
            {
                if (*forceChange && sourceIdx == 0)
                {
                    return gcvTRUE;
                }
            }
            else
            {
                if (*forceChange)
                {
                    return gcvTRUE;
                }
            }
        }

        /* Sample mask should always be in HIGHP. So sample ID and sample position should always be in HIGHP as well.
           Because,
           o   Medium position mode doesn’t support mask LOCATION_W  (can’t)and LOCATION_Z (make it simple).
           o   Sample depth is always high precision. So LOCATION_SUB_Z is also high precision. */
        if (srcSym &&
            (VIR_Symbol_GetName(srcSym) == VIR_NAME_POSITION ||
             VIR_Symbol_GetName(srcSym) == VIR_NAME_SUBSAMPLE_DEPTH ||
             VIR_Symbol_GetName(srcSym) == VIR_NAME_SAMPLE_ID ||
             VIR_Symbol_GetName(srcSym) == VIR_NAME_SAMPLE_POSITION ||
             VIR_Symbol_GetName(srcSym) == VIR_NAME_SAMPLE_MASK_IN ||
             VIR_Symbol_GetName(srcSym) == VIR_NAME_SAMPLE_MASK))
        {
            *skipLowp = gcvFALSE;
            return gcvTRUE;
        }

        if (VIR_OPCODE_isMemSt(opcode)       ||
            VIR_OPCODE_isMemLd(opcode)       ||
            opcode == VIR_OP_IMG_LOAD        ||
            opcode == VIR_OP_VX_IMG_LOAD     ||
            opcode == VIR_OP_IMG_LOAD_3D     ||
            opcode == VIR_OP_VX_IMG_LOAD_3D  ||
            opcode == VIR_OP_IMG_STORE       ||
            opcode == VIR_OP_VX_IMG_STORE    ||
            opcode == VIR_OP_IMG_STORE_3D    ||
            opcode == VIR_OP_VX_IMG_STORE_3D ||
            opcode == VIR_OP_IMG_ADDR        ||
            opcode == VIR_OP_IMG_ADDR_3D     ||
            VIR_OPCODE_isAtom(opcode)
            )
        {
            if (sourceIdx == 0)
            {
                return gcvTRUE;
            }
        }

        if (opcode == VIR_OP_IMG_LOAD_3D  ||
            opcode == VIR_OP_IMG_STORE_3D ||
            opcode == VIR_OP_IMG_ADDR_3D
            )
        {
            if (sourceIdx == 1)
            {
                return gcvTRUE;
            }
        }

        if (VIR_OPCODE_isMemSt(opcode)    ||
            opcode == VIR_OP_IMG_STORE    ||
            opcode == VIR_OP_IMG_STORE_3D
            )
        {
            if (sourceIdx == 2)
            {
                return gcvTRUE;
            }
        }

        if (opcode == VIR_OP_CONV)
        {
            if (srcSym && VIR_Symbol_isInput(srcSym) &&
                VIR_Symbol_GetName(srcSym) != VIR_NAME_FRONT_FACING)
            {
                return gcvTRUE;
            }
            /* convert should not do two things:
               for example, int32 <- uint16 is not doing sign extension.
               we should do it in two steps int16 <- uint16; int32 <- int16 or using highp all the way */
            if (VIR_Symbol_GetPrecision(VIR_Operand_GetSymbol(VIR_Inst_GetDest(pInst))) == VIR_PRECISION_HIGH)
            {
                return gcvTRUE;
            }
        }

        /* texld_gather has to be highp */
        if (opcode == VIR_OP_TEXLD_GATHER ||
            opcode == VIR_OP_TEXLD_GATHER_PCF)
        {
            *forceChange = gcvTRUE;
            return gcvTRUE;
        }
    }

    if (dual16PrecisionRule & Dual16_PrecisionRule_RCP_HP)
    {
        if (opcode == VIR_OP_RCP)
        {
            if (sourceIdx == 0)
            {
                return gcvTRUE;
            }
        }
    }

    if (dual16PrecisionRule & Dual16_PrecisionRule_FRAC_HP)
    {
        if (opcode == VIR_OP_FRAC)
        {
            if (sourceIdx == 0)
            {
                return gcvTRUE;
            }
        }
    }

    if (VIR_OPCODE_isTexLd(opcode))
    {
        VIR_USAGE_KEY       usageKey;
        gctUINT             usageIdx, defIdx, i;
        VIR_USAGE           *pUsage = gcvNULL;
        VIR_DEF*            pDef = gcvNULL;

        /* src1 coordinate is from attribute */
        if (dual16PrecisionRule & Dual16_PrecisionRule_TEXLD_COORD_HP)
        {
            if (sourceIdx == 1 &&
                srcSym &&
                VIR_Symbol_isInput(srcSym) &&
                VIR_Symbol_GetName(srcSym) != VIR_NAME_POINT_COORD)
            {
                return gcvTRUE;
            }
        }

        usageKey.pUsageInst = pInst;
        usageKey.pOperand = srcOpnd;
        usageKey.bIsIndexingRegUsage = gcvFALSE;
        usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);

        if (VIR_INVALID_USAGE_INDEX != usageIdx)
        {
            pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
            gcmASSERT(pUsage);

            for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
            {
                defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                if (pDef->defKey.pDefInst != VIR_INPUT_DEF_INST)
                {
                    /* src1 cordindate is from mov attribute */
                    if (dual16PrecisionRule & Dual16_PrecisionRule_TEXLD_COORD_HP)
                    {
                        if (VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_MOV)
                        {
                            VIR_Operand *movSrcOpnd = VIR_Inst_GetSource(pDef->defKey.pDefInst, 0);
                            VIR_Symbol  *movSrcSym = VIR_Operand_GetUnderlyingSymbol(movSrcOpnd);

                            if (sourceIdx == 1 &&
                                movSrcSym && VIR_Symbol_isInput(movSrcSym))
                            {
                                /* change MOV's src */
                                _Inst_ChangeOpnd2HP(pDef->defKey.pDefInst, movSrcOpnd, gcvFALSE, gcvTRUE, pDuInfo);
                                return gcvTRUE;
                            }
                        }
                    }

                    /* src0 sampler is from LDARR, indirect has to be highp per HW requirement */
                    if (sourceIdx == 0 &&
                        VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_LDARR)
                    {
                        *forceChange = gcvTRUE;
                        return gcvTRUE;
                    }
                }
            }
        }
    }

    return gcvFALSE;
}

/* Based on HW dual16 restriction, adjust operand/symbol's precision */
VSC_ErrCode vscVIR_AdjustPrecision(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo, VSC_HW_CONFIG* pHwCfg)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_FuncIterator               func_iter;
    VIR_FunctionNode*              func_node;

    /* currently only PS is dual16-able */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_FRAGMENT)
    {
        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
        for (func_node = VIR_FuncIterator_First(&func_iter);
             func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function    *func = func_node->function;
            VIR_InstIterator inst_iter;
            VIR_Instruction *inst;
            gctUINT         i;
            gctBOOL         forceChange = gcvFALSE, skipLowp = gcvTRUE;

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                 inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                /* bitwise operation: if any src is highp, the other src and dest has to be highp */
                if (VIR_OPCODE_BITWISE(VIR_Inst_GetOpcode(inst)))
                {
                    for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                    {
                        if ((VIR_Operand_GetOpKind(VIR_Inst_GetSource(inst, i)) == VIR_OPND_IMMEDIATE &&
                            !VIR_Opnd_ValueFit16Bits(VIR_Inst_GetSource(inst, i))) ||
                            VIR_Operand_GetPrecision(VIR_Inst_GetSource(inst, i)) == VIR_PRECISION_HIGH)
                        {
                            forceChange = gcvTRUE;
                            break;
                        }
                    }
                }

                /* HW has restriction on select instruction - only one instruction type for comparison and type conversion
                   we may have problem when dest and src0 type is different. For example,
                   select fp32, int32, fp16, fp16,
                   we need to set it instruction type to int32, thus it will use integer conversion
                   to convert fp16 to fp32, which is wrong. Thus, we make dest and all src to be highp if
                   there is one highp source or dest */
                if (VIR_Inst_GetOpcode(inst) == VIR_OP_SELECT)
                {
                    /* cmp.uint16 t1, t2, t3
                       select.selmsb.fp32, t1.uint16, fp16, fp16 has problem
                       since t1 will be -1 in uint16 (0xFFFF), while select.selmsb
                       the instruction is fp32, which will check bit31 to be one or not*/
                    if (VIR_Inst_GetConditionOp(inst) == VIR_COP_SELMSB ||
                        VIR_Inst_GetConditionOp(inst) == VIR_COP_ALLMSB ||
                        VIR_Inst_GetConditionOp(inst) == VIR_COP_ANYMSB)
                    {
                        if (VIR_Operand_GetPrecision(VIR_Inst_GetDest(inst)) == VIR_PRECISION_HIGH &&
                            VIR_Operand_GetPrecision(VIR_Inst_GetSource(inst, 0)) != VIR_PRECISION_HIGH)
                        {
                            forceChange = gcvTRUE;
                        }
                    }
                    else
                    {
                        VIR_Symbol  *destSym = VIR_Operand_GetSymbol(VIR_Inst_GetDest(inst));
                        gctBOOL     needHighp = gcvFALSE;
                        if (destSym)
                        {
                            VIR_TypeId ty0 = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(destSym));
                            VIR_TypeId ty1 = VIR_Operand_GetType(VIR_Inst_GetSource(inst, 0));

                            if(!(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
                                (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
                                ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) &&
                                (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER))))
                            {
                                needHighp = gcvTRUE;
                            }

                            /* it is possible that destSym is used as a different type */
                            if (!needHighp)
                            {
                                VIR_DEF_KEY   defKey;
                                gctUINT       defIdx;
                                VIR_USAGE     *pUsage = gcvNULL;
                                VIR_DEF       *pDef = gcvNULL;
                                VIR_OperandInfo     operandInfo;
                                VSC_DU_ITERATOR     duIter;
                                VIR_DU_CHAIN_USAGE_NODE *pUsageNode;

                                VIR_Operand_GetOperandInfo(
                                    inst,
                                    VIR_Inst_GetDest(inst),
                                    &operandInfo);

                                defKey.pDefInst = inst;
                                defKey.regNo = operandInfo.u1.virRegInfo.virReg;
                                defKey.channel = VIR_CHANNEL_ANY;
                                defIdx = vscBT_HashSearch(&pDuInfo->defTable, &defKey);

                                while (VIR_INVALID_DEF_INDEX != defIdx)
                                {
                                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                                    /* go through all the uses */
                                    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                                    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                                    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                                    {
                                        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
                                        if (pUsage->usageKey.pUsageInst != VIR_OUTPUT_USAGE_INST)
                                        {
                                            ty0 = VIR_Operand_GetType(pUsage->usageKey.pOperand);
                                            if(!(((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISFLOAT) &&
                                                (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISFLOAT)) ||
                                                ((VIR_GetTypeFlag(ty0) & VIR_TYFLAG_ISINTEGER) &&
                                                (VIR_GetTypeFlag(ty1) & VIR_TYFLAG_ISINTEGER))))
                                            {
                                                needHighp = gcvTRUE;
                                                break;
                                            }
                                        }
                                    }

                                    defIdx = pDef->nextDefIdxOfSameRegNo;
                                    if (needHighp)
                                    {
                                        break;
                                    }
                                }
                            }
                            if (needHighp)
                            {
                                if (VIR_Operand_GetPrecision(VIR_Inst_GetDest(inst)) == VIR_PRECISION_HIGH)
                                {
                                    forceChange = gcvTRUE;
                                }
                                else
                                {
                                    for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                                    {
                                        if (VIR_Operand_GetPrecision(VIR_Inst_GetSource(inst, i)) == VIR_PRECISION_HIGH)
                                        {
                                            forceChange = gcvTRUE;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)) &&
                    (VIR_GetTypeFlag(VIR_Operand_GetType(inst->dest)) & VIR_TYFLAG_ISINTEGER) &&
                    VIR_Operand_GetPrecision(VIR_Inst_GetDest(inst)) != VIR_PRECISION_HIGH &&
                    !pHwCfg->hwFeatureFlags.hasHalti5)
                {
                    forceChange = gcvTRUE;
                    skipLowp = gcvFALSE;
                }

                for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                {
                    if (_Inst_RequireHPSrc(inst, i, pDuInfo, &forceChange, &skipLowp))
                    {
                        _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetSource(inst, i), gcvFALSE, skipLowp, pDuInfo);
                    }
                }

                if (_Inst_RequireHPDest(inst, forceChange))
                {
                    _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetDest(inst), gcvTRUE, skipLowp, pDuInfo);
                }

                /* integer output should be high-precision,
                   HW Cvt2OutColFmt has issue with 0x2*/
                {
                    VIR_Operand* dest = VIR_Inst_GetDest(inst);
                    if(dest)
                    {
                        if(VIR_Operand_isVirReg(dest) || VIR_Operand_isSymbol(dest))
                        {
                            VIR_Symbol* sym = VIR_Operand_GetSymbol(dest);
                            if(VIR_Symbol_isVreg(sym))
                            {
                                VIR_Symbol* varSym = VIR_Symbol_GetVregVariable(sym);
                                if(VIR_Symbol_isOutput(varSym))
                                {
                                    if(VIR_Type_isInteger(VIR_Symbol_GetType(varSym)))
                                    {
                                        VIR_Symbol_SetPrecision(varSym, VIR_PRECISION_HIGH);
                                        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
                                        VIR_Operand_SetPrecision(dest, VIR_PRECISION_HIGH);
                                    }
                                }
                            }
                            else if(VIR_Symbol_isVariable(sym))
                            {
                                if(VIR_Symbol_isOutput(sym))
                                {
                                    if(VIR_Type_isInteger(VIR_Symbol_GetType(sym)))
                                    {
                                        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
                                        VIR_Operand_SetPrecision(dest, VIR_PRECISION_HIGH);
                                    }
                                }
                            }
                        }
                    }
                }
                /* HW has bug on conversion for flat varying, thus promote it to highp */
                {
                    gctUINT i;
                    for(i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(inst, i);
                        if (VIR_Operand_GetOpKind(src) == VIR_OPND_SYMBOL)
                        {
                            VIR_Symbol * sym = VIR_Operand_GetSymbol(src);
                            if(VIR_Symbol_isInput(sym) && isSymFlat(sym))
                            {
                                VIR_Operand_SetPrecision(src, VIR_PRECISION_HIGH);
                                VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
                            }
                        }
                        else if (VIR_Operand_GetOpKind(src) == VIR_OPND_VIRREG)
                        {
                            VIR_Symbol * sym = VIR_Operand_GetSymbol(src);
                            VIR_Symbol * varSym = VIR_Symbol_GetVregVariable(sym);
                            if(VIR_Symbol_isInput(varSym) && isSymFlat(varSym))
                            {
                                VIR_Operand_SetPrecision(src, VIR_PRECISION_HIGH);
                                VIR_Symbol_SetPrecision(sym, VIR_PRECISION_HIGH);
                                VIR_Symbol_SetPrecision(varSym, VIR_PRECISION_HIGH);
                            }
                        }
                    }
                }
            }
        }
    }

    if (VirSHADER_DumpCodeGenVerbose(pShader->_id))
    {
        VIR_Shader_Dump(gcvNULL, "After Adjust Precision", pShader, gcvTRUE);
    }

    return errCode;
}


VIR_Symbol* _vscVIR_FindSamplerFromIndex(VIR_Instruction* inst, VIR_Operand* index)
{
    VIR_Instruction* prev;

    for(prev = VIR_Inst_GetPrev(inst); prev; prev = VIR_Inst_GetPrev(prev))
    {
        VIR_Operand* dest;

        dest = VIR_Inst_GetDest(prev);
        if(dest->u1.sym->u1.vregIndex == index->u1.sym->u1.vregIndex &&
           VIR_Operand_GetEnable(dest) == VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(index)))
        {
            VIR_OpCode op = VIR_Inst_GetOpcode(prev);

            if(op == VIR_OP_LDARR)
            {
                return _vscVIR_FindSamplerFromIndex(prev, VIR_Inst_GetSource(prev, 1));
            }
            else if(op == VIR_OP_MOVA)
            {
                return _vscVIR_FindSamplerFromIndex(prev, VIR_Inst_GetSource(prev, 0));
            }
            else if(op == VIR_OP_GET_SAMPLER_IDX)
            {
                VIR_Operand* src = VIR_Inst_GetSource(prev, 0);
                return src->u1.sym;
            }
        }
    }

    return gcvNULL;
}

VSC_ErrCode vscVIR_ConvertVirtualInstructions(VIR_Shader* pShader)
{
    VSC_ErrCode errCode  = VSC_ERR_NONE;
    VIR_FuncIterator funcIter;
    VIR_FunctionNode* funcNode;

    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(pShader));
    for(funcNode = VIR_FuncIterator_First(&funcIter);
        funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function* func = funcNode->function;
        VIR_Instruction* inst;

        for(inst = VIR_Function_GetInstStart(func); inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
            switch(opcode)
            {
                case VIR_OP_GET_SAMPLER_LMM:
                {
                    VIR_Operand* samplerSrc = VIR_Inst_GetSource(inst, 0);
                    VIR_Symbol* samplerSym;
                    VIR_Uniform* samplerUniform;

                    gcmASSERT(VIR_Operand_isSymbol(samplerSrc));
                    samplerSym = VIR_Operand_GetSymbol(samplerSrc);
                    if(!VIR_Symbol_isSampler(samplerSym))
                    {
                        samplerSym = _vscVIR_FindSamplerFromIndex(inst, samplerSrc);
                    }
                    gcmASSERT(samplerSym && VIR_Symbol_isSampler(samplerSym));
                    samplerUniform = VIR_Symbol_GetSampler(samplerSym);
                    if(samplerUniform->u.samplerOrImageAttr.lodMinMax == gcvNULL)
                    {
                        VIR_Uniform* lodMinMaxUniform;
                        VIR_NameId nameId;
                        VIR_SymId llmSymID = VIR_INVALID_ID;
                        gctCHAR name[128] = "#";

                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerSym));
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$LodMinMax");
                        errCode = VIR_Shader_AddString(pShader,
                                                       name,
                                                       &nameId);
                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_UNIFORM,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X3),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &llmSymID);
                        samplerUniform->u.samplerOrImageAttr.lodMinMax = VIR_Shader_GetSymFromId(pShader, llmSymID);
                        VIR_Symbol_SetFlag(samplerUniform->u.samplerOrImageAttr.lodMinMax, VIR_SYMFLAG_COMPILER_GEN);
                        VIR_Symbol_SetPrecision(samplerUniform->u.samplerOrImageAttr.lodMinMax, VIR_PRECISION_MEDIUM);
                        lodMinMaxUniform = VIR_Symbol_GetUniform(samplerUniform->u.samplerOrImageAttr.lodMinMax);
                        VIR_Symbol_SetUniformKind(samplerUniform->u.samplerOrImageAttr.lodMinMax, VIR_UNIFORM_LOD_MIN_MAX);
                        VIR_Symbol_SetAddrSpace(samplerUniform->u.samplerOrImageAttr.lodMinMax, VIR_AS_CONSTANT);
                        VIR_Symbol_SetTyQualifier(samplerUniform->u.samplerOrImageAttr.lodMinMax, VIR_TYQUAL_CONST);
                        lodMinMaxUniform->u.parentSampler = samplerUniform;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerSrc, func, samplerUniform->u.samplerOrImageAttr.lodMinMax->index);
                    VIR_Operand_SetType(samplerSrc, VIR_TYPE_FLOAT_X3);
                    break;
                }
                case VIR_OP_GET_SAMPLER_LBS:
                {
                    VIR_Operand* samplerSrc = VIR_Inst_GetSource(inst, 0);
                    VIR_Symbol* samplerSym;
                    VIR_Uniform* samplerUniform;

                    gcmASSERT(VIR_Operand_isSymbol(samplerSrc));
                    samplerSym = VIR_Operand_GetSymbol(samplerSrc);
                    if(!VIR_Symbol_isSampler(samplerSym))
                    {
                        samplerSym = _vscVIR_FindSamplerFromIndex(inst, samplerSrc);
                    }
                    gcmASSERT(samplerSym && VIR_Symbol_isSampler(samplerSym));
                    samplerUniform = VIR_Symbol_GetSampler(samplerSym);
                    if(samplerUniform->u.samplerOrImageAttr.levelBaseSize == gcvNULL)
                    {
                        VIR_Uniform* levelBaseSizeUniform;
                        VIR_NameId nameId;
                        VIR_SymId lbsSymID = VIR_INVALID_ID;
                        gctCHAR name[128] = "#";

                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerSym));
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$LevelBaseSize");
                        errCode = VIR_Shader_AddString(pShader,
                                                       name,
                                                       &nameId);
                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_UNIFORM,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X4),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &lbsSymID);
                        samplerUniform->u.samplerOrImageAttr.levelBaseSize = VIR_Shader_GetSymFromId(pShader, lbsSymID);
                        VIR_Symbol_SetFlag(samplerUniform->u.samplerOrImageAttr.levelBaseSize, VIR_SYMFLAG_COMPILER_GEN);
                        VIR_Symbol_SetPrecision(samplerUniform->u.samplerOrImageAttr.levelBaseSize, VIR_PRECISION_MEDIUM);
                        levelBaseSizeUniform = VIR_Symbol_GetUniform(samplerUniform->u.samplerOrImageAttr.levelBaseSize);
                        VIR_Symbol_SetUniformKind(samplerUniform->u.samplerOrImageAttr.levelBaseSize, VIR_UNIFORM_LEVEL_BASE_SIZE);
                        VIR_Symbol_SetAddrSpace(samplerUniform->u.samplerOrImageAttr.levelBaseSize, VIR_AS_CONSTANT);
                        VIR_Symbol_SetTyQualifier(samplerUniform->u.samplerOrImageAttr.levelBaseSize, VIR_TYQUAL_CONST);
                        levelBaseSizeUniform->u.parentSampler = samplerUniform;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerSrc, func, samplerUniform->u.samplerOrImageAttr.levelBaseSize->index);
                    VIR_Operand_SetType(samplerSrc, VIR_TYPE_INTEGER_X4);
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After Convert Virtual Instructions.", pShader, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode vscVIR_PrecisionUpdateSrc(VIR_Shader* shader, VIR_Operand* operand, VSC_OPTN_PUOptions* puOptions)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(operand && puOptions);

    switch(VIR_Operand_GetOpKind(operand))
    {
        case VIR_OPND_SYMBOL:
        case VIR_OPND_VIRREG:
        case VIR_OPND_ARRAY:
        case VIR_OPND_FIELD:
        case VIR_OPND_SAMPLER_INDEXING:
        case VIR_OPND_PARAMETERS:
        {
            VIR_Symbol* sym = VIR_Operand_GetSymbol(operand);

            gcmASSERT(VIR_Operand_GetPrecision(operand) != VIR_PRECISION_DEFAULT
                                 || VIR_Shader_IsCL(shader));
            if(gcoOS_StrCmp(VIR_Shader_GetSymNameString(shader, sym), "#BaseSamplerSym") != gcvSTATUS_OK)
            {
                if(VIR_Operand_GetPrecision(operand) == VIR_PRECISION_ANY)
                {
                    gcmASSERT((VIR_Symbol_GetCurrPrecision(sym) != VIR_PRECISION_ANY &&
                               VIR_Symbol_GetCurrPrecision(sym) != VIR_PRECISION_DEFAULT)
                              || VIR_Shader_IsCL(shader));

                    VIR_Operand_SetPrecision(operand, VIR_Symbol_GetCurrPrecision(sym));
                }
                else
                {
                    gcmASSERT(VIR_Symbol_GetPrecision(sym) == VIR_Operand_GetPrecision(operand) ||
                              VIR_Symbol_GetPrecision(sym) == VIR_PRECISION_ANY
                              || VIR_Shader_IsCL(shader));

                }
            }
            break;
        }

        case VIR_OPND_TEXLDPARM:
        {
            gctUINT k;
            VIR_Operand_TexldModifier *texldOperand = (VIR_Operand_TexldModifier*)operand;

            for(k = 0; k < VIR_TEXLDMODIFIER_COUNT; ++k)
            {
                if(texldOperand->tmodifier[k] != gcvNULL)
                {
                    vscVIR_PrecisionUpdateSrc(shader, texldOperand->tmodifier[k], puOptions);
                    break;
                }
            }
            break;
        }
        case VIR_OPND_IMMEDIATE:
        case VIR_OPND_CONST:
        case VIR_OPND_ADDRESS_OF:
            gcmASSERT(VIR_Operand_GetPrecision(operand) == VIR_PRECISION_HIGH);
            break;
        case VIR_OPND_INTRINSIC:
        case VIR_OPND_UNUSED:
        case VIR_OPND_EVIS_MODIFIER:
            break;
        case VIR_OPND_LABEL:
        case VIR_OPND_FUNCTION:
        default:
            break;
    }

    return errCode;
}

static VSC_ErrCode vscVIR_PrecisionUpdateDst(VIR_Instruction* inst, VSC_OPTN_PUOptions* puOptions)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Operand* dst = VIR_Inst_GetDest(inst);
    VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

    if(VIR_OPCODE_ExpectedResultPrecision(opcode))
    {
        gcmASSERT(VIR_Operand_GetPrecision(dst) != VIR_PRECISION_DEFAULT
            || VIR_Shader_IsCL(VIR_Inst_GetShader(inst)));

        if(VIR_Operand_GetPrecision(dst) == VIR_PRECISION_ANY)
        {
            VIR_Precision precision = VIR_Inst_GetExpectedResultPrecision(inst);
            VIR_Symbol* sym = VIR_Operand_GetSymbol(dst);

            if(VIR_Inst_GetOpcode(inst) == VIR_OP_LDARR)
            {
                VIR_Symbol* opndSym0 = VIR_Operand_GetSymbol(VIR_Inst_GetSource(inst, 0));
                if(gcoOS_StrCmp(VIR_Shader_GetSymNameString(VIR_Inst_GetShader(inst), opndSym0), "#BaseSamplerSym") == gcvSTATUS_OK)
                {
                    precision = VIR_Operand_GetPrecision(VIR_Inst_GetSource(inst, 1));
                }
            }
            VIR_Operand_SetPrecision(dst, precision);
            VIR_Symbol_SetCurrPrecision(sym, precision);
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_PrecisionUpdate(VIR_Shader* pShader, VSC_OPTN_PUOptions* puOptions, VIR_Dumper* dumper)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    if(!VIR_Shader_IsFS(pShader))
    {
        return errCode;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;

        /* */
        {
            gctUINT i;
            for(i = 0; i < VIR_IdList_Count(&func->paramters); ++i)
            {
                VIR_Id id = VIR_IdList_GetId(&func->paramters, i);
                VIR_Symbol* param = VIR_Function_GetSymFromId(func, id);
                VIR_Symbol* virReg = VIR_Shader_FindSymbolByTempIndex(pShader, VIR_Symbol_GetVariableVregIndex(param));

                gcmASSERT(VIR_Symbol_GetPrecision(param) != VIR_PRECISION_DEFAULT || VIR_Shader_IsCL(pShader));
                /* gcmASSERT(VIR_Symbol_GetVregVariable(virReg) == param); */ /* virReg's variable will be reset to another sym which came from old variable for function input/output */
                if(VIR_Symbol_GetPrecision(param) == VIR_PRECISION_ANY)
                {
                    VIR_Symbol_SetCurrPrecision(param, VIR_PRECISION_HIGH);
                    VIR_Symbol_SetCurrPrecision(virReg, VIR_PRECISION_HIGH);
                }
            }
        }

        /* */
        {
            VIR_CONTROL_FLOW_GRAPH* cfg = VIR_Function_GetCFG(func);
            CFG_ITERATOR cfg_iter;
            VIR_BASIC_BLOCK* bb;

            CFG_ITERATOR_INIT(&cfg_iter, cfg);
            for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL; bb = CFG_ITERATOR_NEXT(&cfg_iter))
            {
                gctUINT i;
                VIR_Instruction* inst;

                for(i = 0, inst = BB_GET_START_INST(bb); i < BB_GET_LENGTH(bb); i++)
                {
                    gctUINT j;
                    for(j = 0; j < VIR_Inst_GetSrcNum(inst); j++)
                    {
                        VIR_Operand* src = VIR_Inst_GetSource(inst, j);

                        vscVIR_PrecisionUpdateSrc(pShader, src, puOptions);
                    }

                    vscVIR_PrecisionUpdateDst(inst, puOptions);

                    if(VSC_OPTN_PUOptions_GetTrace(puOptions))
                    {
                        VIR_Inst_Dump(dumper, inst);
                    }
                    inst = VIR_Inst_GetNext(inst);
                }
            }
        }
    }

    if (gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After Precision Update.", pShader, gcvTRUE);
    }

    return errCode;
}

static gctBOOL _vscVIR_ConvertIntegerSymbolToFloat(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Symbol* symbol
    )
{
    gctBOOL changed = gcvFALSE;
    VIR_SymbolKind symKind = VIR_Symbol_GetKind(symbol);

    switch(symKind)
    {
        case VIR_SYM_UNIFORM:
        case VIR_SYM_VARIABLE:
        case VIR_SYM_VIRREG:
        {
            VIR_Type* symType = VIR_Symbol_GetType(symbol);
            VIR_TypeKind symTypeKind = VIR_Type_GetKind(symType);

            switch(symTypeKind)
            {
                case VIR_TY_SCALAR:
                case VIR_TY_VECTOR:
                case VIR_TY_MATRIX:
                {
                    VIR_TypeId symTypeId = VIR_Type_GetIndex(symType);
                    gctUINT32 symTypeIdComponentCount = VIR_GetTypeComponents(symTypeId);
                    gctUINT32 symTypeIdRowCount = VIR_GetTypeRows(symTypeId);

                    gcmASSERT(VIR_TypeId_isPrimitive(symTypeId));
                    if(VIR_TypeId_isInteger(symTypeId))
                    {
                        VIR_TypeId newTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, symTypeIdComponentCount, symTypeIdRowCount);
                        VIR_Type* newType = VIR_Shader_GetTypeFromId(pShader, newTypeId);

                        VIR_Symbol_SetType(symbol, newType);
                        changed = gcvTRUE;
                    }
                    break;
                }
                case VIR_TY_SAMPLER:
                case VIR_TY_IMAGE:
                case VIR_TY_POINTER:
                {
                    break;
                }
                case VIR_TY_ARRAY:
                {
                    VIR_TypeId symBaseTypeId = VIR_Type_GetBaseTypeId(symType);
                    VIR_Type* symBaseType = VIR_Shader_GetTypeFromId(pShader, symBaseTypeId);
                    VIR_TypeKind symBaseTypeKind = VIR_Type_GetKind(symBaseType);

                    switch(symBaseTypeKind)
                    {
                        case VIR_TY_SCALAR:
                        case VIR_TY_VECTOR:
                        case VIR_TY_MATRIX:
                        {
                            if(VIR_GetTypeFlag(symBaseTypeId) & VIR_TYFLAG_ISINTEGER)
                            {
                                gctUINT32 symBaseTypeIdComponentCount = VIR_GetTypeComponents(symBaseTypeId);
                                gctUINT32 symBaseTypeIdRowCount = VIR_GetTypeRows(symBaseTypeId);
                                VIR_TypeId newBaseTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, symBaseTypeIdComponentCount, symBaseTypeIdRowCount);
                                VIR_Type* newBaseType = VIR_Shader_GetTypeFromId(pShader, newBaseTypeId);

                                VIR_Symbol_SetType(symbol, newBaseType);
                                changed = gcvTRUE;
                            }
                        }
                        default:
                            break;
                    }

                    break;
                }
                default:
                    gcmASSERT(0);
            }
            break;
        }
        default:
            break;
    }

    return changed;
}

static gctBOOL _vscVIR_ConvertIntegerOperandToFloat(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Operand* operand
    )
{
    gctUINT sourceKind = VIR_Operand_GetOpKind(operand);
    gctBOOL changed = gcvFALSE;

    switch(sourceKind)
    {
        case VIR_OPND_SYMBOL:
        case VIR_OPND_VIRREG:
        {
            VIR_Symbol* operandSym = VIR_Operand_GetSymbol(operand);

            _vscVIR_ConvertIntegerSymbolToFloat(pShader, operandSym);
            break;
        }
        case VIR_OPND_TEXLDPARM:
            break;
        case VIR_OPND_IMMEDIATE:
        {
            VIR_TypeId operandTypeId = VIR_Operand_GetType(operand);

            if(VIR_GetTypeFlag(operandTypeId) & VIR_TYFLAG_IS_UNSIGNED_INT ||
               VIR_GetTypeFlag(operandTypeId) & VIR_TYFLAG_IS_BOOLEAN)
            {
                gctINT32 uint32Val = VIR_Operand_GetImmediateUint(operand);
                gctFLOAT floatVal = (gctFLOAT)uint32Val;

                VIR_Operand_SetImmediateFloat(operand, floatVal);
            }
            else if(VIR_GetTypeFlag(operandTypeId) & VIR_TYFLAG_IS_SIGNED_INT)
            {
                gctINT32 int32Val = VIR_Operand_GetImmediateInt(operand);
                gctFLOAT floatVal = (gctFLOAT)int32Val;

                VIR_Operand_SetImmediateFloat(operand, floatVal);
            }
            break;
        }
        case VIR_OPND_CONST:
        {
            break;
        }
        default:
            break;
    }

    {
        VIR_TypeId operandTypeId = VIR_Operand_GetType(operand);

        if(VIR_TypeId_isPrimitive(operandTypeId) && VIR_TypeId_isInteger(operandTypeId))
        {
            gctUINT operandComponentCount = VIR_GetTypeComponents(operandTypeId);
            VIR_TypeId newTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, operandComponentCount, 1);

            gcmASSERT(VIR_GetTypeRows(operandTypeId) == 1);
            VIR_Operand_SetType(operand, newTypeId);
            changed = gcvTRUE;
        }
    }

    return changed;
}

static void _vscVIR_ConvertIntegerInstructionToFloat(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Instruction* inst,
    IN gctBOOL supportInteger,
    IN gctBOOL hasSignFloorCeil
    )
{
    VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
    gctUINT i;
    gctBOOL toBeConvertedSource[VIR_MAX_SRC_NUM] = {gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};
    gctBOOL toBeConvertedDest = gcvTRUE;
    gctBOOL ConvertedSource[VIR_MAX_SRC_NUM] = {gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE, gcvFALSE};
    gctBOOL ConvertedDest = gcvFALSE;

    switch(opcode)
    {
        case VIR_OP_CMOV:
        case VIR_OP_SELECT:
        {
            VIR_ConditionOp conditionOp = VIR_Inst_GetConditionOp(inst);
            if(conditionOp == VIR_COP_EQUAL)
            {
                toBeConvertedSource[0] = gcvFALSE;
                toBeConvertedSource[1] = gcvFALSE;
            }
            break;
        }
        case VIR_OP_CMP:
        {
            VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

            if(VIR_Inst_GetConditionOp(inst) == VIR_COP_SELMSB)
            {
                toBeConvertedSource[0] = gcvFALSE;
            }
            if(nextInst && VIR_Inst_GetOpcode(nextInst) && VIR_Inst_GetConditionOp(nextInst) == VIR_COP_SELMSB)
            {
                toBeConvertedDest = gcvFALSE;
            }
            break;
        }
        case VIR_OP_ARCTRIG:
            return;
        default:
            break;
    }


    for(i = 0; i < VIR_OPCODE_GetSrcOperandNum(opcode) && toBeConvertedSource[i]; i++)
    {
        VIR_Operand* source = VIR_Inst_GetSource(inst, i);

        ConvertedSource[i] = _vscVIR_ConvertIntegerOperandToFloat(pShader, source);
    }

    if(VIR_OPCODE_hasDest(opcode) && toBeConvertedDest)
    {
        VIR_Operand* dest = VIR_Inst_GetDest(inst);

        ConvertedDest = _vscVIR_ConvertIntegerOperandToFloat(pShader, dest);
    }

    switch(opcode)
    {
        case VIR_OP_CONV:
        {
            VIR_Operand* dest = VIR_Inst_GetDest(inst);
            VIR_Operand* src = VIR_Inst_GetSource(inst, 0);

            if(VIR_Operand_GetType(dest) == VIR_Operand_GetType(src))
            {
                if(ConvertedDest && !ConvertedSource[0])
                {
                    VIR_Function* func = VIR_Inst_GetFunction(inst);
                    VIR_TypeId destTypeId = VIR_Operand_GetType(dest);
                    gctUINT destTypeComponents = VIR_GetTypeComponents(destTypeId);
                    VIR_Enable newEnable = VIR_Operand_GetEnable(VIR_Inst_GetDest(inst));
                    VIR_Swizzle newSwizzle = VIR_Enable_2_Swizzle_WShift(newEnable);

                    gcmASSERT(VIR_GetTypeComponentType(destTypeId) == VIR_TYPE_FLOAT32);

                    if(supportInteger)
                    {
                        VIR_TypeId newSymTypeId = VIR_TypeId_ComposePackedNonOpaqueType(VIR_TYPE_INT32, destTypeComponents);
                        VIR_VirRegId newRegId;
                        VIR_SymId newSymId;
                        VIR_Symbol* newSym;
                        VIR_Instruction* newInst;

                        newRegId = VIR_Shader_NewVirRegId(pShader, 1);
                        VIR_Shader_AddSymbol(pShader,
                                             VIR_SYM_VIRREG,
                                             newRegId,
                                             VIR_Shader_GetTypeFromId(pShader, newSymTypeId),
                                             VIR_STORAGE_UNKNOWN,
                                             &newSymId);
                        newSym = VIR_Shader_GetSymFromId(pShader, newSymId);
                        VIR_Symbol_SetPrecision(newSym, VIR_Operand_GetPrecision(VIR_Inst_GetDest(inst)));

                        VIR_Function_AddInstructionBefore(func, VIR_OP_AQ_F2I, destTypeId, inst, &newInst);
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(newInst), func, newSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), newEnable);
                        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Inst_GetSource(inst, 0));

                        VIR_Inst_SetOpcode(inst, VIR_OP_AQ_I2F);
                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(inst, 0), func, newSymId);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(inst, 0), newSwizzle);
                    }
                    else if(hasSignFloorCeil)
                    {
                        VIR_VirRegId newRegId;
                        VIR_SymId signDestSymId, absDestSymId;
                        VIR_Symbol* signDestSym, *absDestSym;
                        VIR_Instruction* signInst, *absInst, *mulInst;

                        VIR_Function_AddInstructionBefore(func, VIR_OP_SIGN, destTypeId, inst, &signInst);
                        VIR_Operand_Copy(VIR_Inst_GetSource(signInst, 0), VIR_Inst_GetSource(inst, 0));
                        VIR_Function_AddInstructionBefore(func, VIR_OP_ABS, destTypeId, inst, &absInst);
                        VIR_Operand_Copy(VIR_Inst_GetSource(absInst, 0), VIR_Inst_GetSource(inst, 0));
                        VIR_Inst_SetOpcode(inst, VIR_OP_FLOOR);
                        VIR_Function_AddInstructionAfter(func, VIR_OP_MUL, destTypeId, inst, &mulInst);
                        VIR_Operand_Copy(VIR_Inst_GetDest(mulInst), VIR_Inst_GetDest(inst));

                        /* update the dest of sign */
                        newRegId = VIR_Shader_NewVirRegId(pShader, 1);
                        VIR_Shader_AddSymbol(pShader,
                                             VIR_SYM_VIRREG,
                                             newRegId,
                                             VIR_Shader_GetTypeFromId(pShader, destTypeId),
                                             VIR_STORAGE_UNKNOWN,
                                             &signDestSymId);
                        signDestSym = VIR_Shader_GetSymFromId(pShader, signDestSymId);
                        VIR_Symbol_SetPrecision(signDestSym, VIR_PRECISION_MEDIUM);
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(signInst), func, signDestSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(signInst), newEnable);

                        /* update the dest of abs */
                        newRegId = VIR_Shader_NewVirRegId(pShader, 1);
                        VIR_Shader_AddSymbol(pShader,
                                             VIR_SYM_VIRREG,
                                             newRegId,
                                             VIR_Shader_GetTypeFromId(pShader, destTypeId),
                                             VIR_STORAGE_UNKNOWN,
                                             &absDestSymId);
                        absDestSym = VIR_Shader_GetSymFromId(pShader, absDestSymId);
                        VIR_Symbol_SetPrecision(absDestSym, VIR_Operand_GetPrecision(VIR_Inst_GetSource(absInst, 0)));
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(absInst), func, absDestSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(absInst), newEnable);

                        /* update the dest and src of floor */
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(inst), func, absDestSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(inst), newEnable);
                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(inst, 0), func, absDestSymId);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(inst, 0), newSwizzle);

                        /* update src0 and src1 of abs */
                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(mulInst, 0), func, signDestSymId);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulInst, 0), newSwizzle);
                        VIR_Operand_SetSymbol(VIR_Inst_GetSource(mulInst, 1), func, absDestSymId);
                        VIR_Operand_SetSwizzle(VIR_Inst_GetSource(mulInst, 1), newSwizzle);
                    }
                    else
                    {
                        VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    }
                }
                else
                {
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                }
            }
            break;
        }
        default:
            break;
    }
}

VSC_ErrCode vscVIR_ConvertIntegerToFloat(
    IN OUT VIR_Shader* pShader,
    IN gctBOOL supportInteger,
    IN gctBOOL hasSignFloorCeil
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function* func = func_node->function;
        VIR_Instruction* inst = VIR_Function_GetInstStart(func);

        while(inst)
        {
            _vscVIR_ConvertIntegerInstructionToFloat(pShader, inst, supportInteger, hasSignFloorCeil);
            if(inst == VIR_Function_GetInstEnd(func))
            {
                break;
            }
            inst = VIR_Inst_GetNext(inst);
        }
    }

    if (gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After Converting Integer to Float.", pShader, gcvTRUE);
    }

    return errCode;
}

