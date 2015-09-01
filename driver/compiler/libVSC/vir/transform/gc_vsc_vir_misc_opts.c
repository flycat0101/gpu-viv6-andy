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
                            if (!(VIR_Operand_isSymbol(opnd) &&
                                  VIR_Symbol_isUniform(VIR_Operand_GetSymbol(opnd)) &&
                                  (VIR_GetTypeFlag(VIR_Operand_GetType(opnd)) & VIR_TYFLAG_ISFLOAT) != 0) &&
                                VIR_Operand_GetPrecision(opnd) == VIR_PRECISION_HIGH)
                            {
                                VIR_Symbol_SetPrecision(pNewSym, VIR_PRECISION_HIGH);
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
                            if (VIR_Operand_GetPrecision(opnd) == VIR_PRECISION_HIGH)
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

    if (opcode == VIR_OP_STORE  ||
        opcode == VIR_OP_IMG_STORE  ||
        opcode == VIR_OP_VX_IMG_STORE  ||
        opcode == VIR_OP_IMG_STORE_3D ||
        opcode == VIR_OP_MOVA ||
        opcode == VIR_OP_FRAC ||
        opcode == VIR_OP_RCP ||
        (VIR_OPCODE_isTexLd(opcode) && forceChange) ||
        (VIR_OPCODE_BITWISE(opcode) && forceChange))
    {
        return gcvTRUE;
    }

    if (opcode == VIR_OP_SELECT &&
        VIR_Inst_GetConditionOp(pInst) != VIR_COP_SELMSB &&
        VIR_Inst_GetConditionOp(pInst) != VIR_COP_ALLMSB &&
        VIR_Inst_GetConditionOp(pInst) != VIR_COP_ANYMSB)
    {
        if (forceChange)
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

/* According to several rules, change the source to be highp.
   For some cases, the source's precision change, also implies that the destination
   needs to change to highp too.
*/
gctBOOL
_Inst_RequireHPSrc(
    VIR_Instruction     *pInst,
    gctUINT             sourceIdx,
    VIR_DEF_USAGE_INFO  *pDuInfo,
    gctBOOL             *forceChange)
{
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand     *srcOpnd = VIR_Inst_GetSource(pInst, sourceIdx);
    VIR_Symbol      *srcSym = VIR_Operand_GetUnderlyingSymbol(srcOpnd);
    gcePATCH_ID     patchID = gcvPATCH_INVALID;
    gctBOOL         qualityMode = gcvTRUE;

    gcoHAL_GetPatchID(gcvNULL, &patchID);

    /* dual16 quality mode: more restrict mode for higher quality
       e.g., promote texld coordinate (coming from varying) to highp */
    switch (patchID)
    {
    case gcvPATCH_GLBM21:
    case gcvPATCH_GLBM25:
    case gcvPATCH_GLBM27:
    case gcvPATCH_GFXBENCH:
    case gcvPATCH_MM07:
    case gcvPATCH_NENAMARK2:
    case gcvPATCH_LEANBACK:
    case gcvPATCH_ANGRYBIRDS:
        qualityMode = gcvFALSE;
        break;
    default:
        break;
    }

    if (VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_VIRREG &&
        VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SYMBOL &&
        VIR_Operand_GetOpKind(srcOpnd) != VIR_OPND_SAMPLER_INDEXING)
    {
        return gcvFALSE;
    }

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

    if (srcSym &&
        (VIR_Symbol_GetName(srcSym) == VIR_NAME_POSITION ||
         VIR_Symbol_GetName(srcSym) == VIR_NAME_SUBSAMPLE_DEPTH))
    {
        return gcvTRUE;
    }

    if (opcode == VIR_OP_LOAD            ||
        opcode == VIR_OP_STORE           ||
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
        VIR_OPCODE_isAtom(opcode)        ||
        opcode == VIR_OP_FRAC            ||
        opcode == VIR_OP_RCP
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

    if (opcode == VIR_OP_STORE        ||
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
        if (srcSym && VIR_Symbol_isInput(srcSym))
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

    if (VIR_OPCODE_isTexLd(opcode))
    {
        VIR_USAGE_KEY       usageKey;
        gctUINT             usageIdx, defIdx, i;
        VIR_USAGE           *pUsage = gcvNULL;
        VIR_DEF*            pDef = gcvNULL;

        /* texld_gather has to be highp */
        if (opcode == VIR_OP_TEXLD_GATHER ||
            opcode == VIR_OP_TEXLD_GATHER_PCF)
        {
            *forceChange = gcvTRUE;
            return gcvTRUE;
        }

        /* src1 cordinate is from attribute */
        if (qualityMode &&
            sourceIdx == 1 &&
            srcSym &&
            VIR_Symbol_isInput(srcSym))
        {
            return gcvTRUE;
        }

        usageKey.pUsageInst = pInst;
        usageKey.pOperand = srcOpnd;
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
                    if (qualityMode &&
                        sourceIdx == 1 &&
                        VIR_Inst_GetOpcode(pDef->defKey.pDefInst) == VIR_OP_MOV)
                    {
                        VIR_Operand *movSrcOpnd = VIR_Inst_GetSource(pDef->defKey.pDefInst, 0);
                        VIR_Symbol  *movSrcSym = VIR_Operand_GetUnderlyingSymbol(movSrcOpnd);

                        if (movSrcSym && VIR_Symbol_isInput(movSrcSym))
                        {
                            /* change MOV's src */
                            _Inst_ChangeOpnd2HP(pDef->defKey.pDefInst, movSrcOpnd, gcvFALSE, gcvTRUE, pDuInfo);
                            return gcvTRUE;
                        }
                    }

                    /* src0 sampler is from LDARR */
                    if (qualityMode &&
                        sourceIdx == 0 &&
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
                    if (_Inst_RequireHPSrc(inst, i, pDuInfo, &forceChange))
                    {
                        _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetSource(inst, i), gcvFALSE, gcvTRUE, pDuInfo);
                    }
                }

                if (_Inst_RequireHPDest(inst, forceChange))
                {
                    _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetDest(inst), gcvTRUE, skipLowp, pDuInfo);
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
    VSC_PRIMARY_MEM_POOL    *pMP)
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
    VIR_Operand         *newOpnd = gcvNULL;
    VIR_Enable          srcEnable = VIR_ENABLE_NONE;
    VIR_OperandInfo     srcInfo;
    VIR_Symbol          *pSym = gcvNULL;
    gctUINT             *defIdxArray = gcvNULL, defCount = 0;

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
            usageIdx = vscBT_HashSearch(&pDuInfo->usageTable, &usageKey);

            if (VIR_INVALID_USAGE_INDEX != usageIdx)
            {
                pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, usageIdx);
                gcmASSERT(pUsage);

                /* save the defIdx for the udChain, since the update inside the loop
                   will change the udChain */
                defCount = UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain);
                defIdxArray = (gctUINT*)vscMM_Alloc(&pMP->mmWrapper, defCount * sizeof(gctUINT));
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
                        VIR_TypeId ty0 = VIR_Operand_GetType(defDest);
                        VIR_TypeId ty1 = VIR_Operand_GetType(srcOpnd);

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
                                            newOpnd,
                                            srcInfo.u1.virRegInfo.virReg,
                                            1,
                                            (1 << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);

                                    vscVIR_DeleteUsage(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pInst,
                                            srcOpnd,
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
                                                                VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(defDest)),
                                                                VIR_STORAGE_UNKNOWN,
                                                                &newDstSymId);
                                pSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

                                /*
                                    mov new-temp-reg (dest precision), srcOpnd
                                */
                                errCode = VIR_Function_AddInstructionBefore(pFunc,
                                    VIR_OP_MOV, VIR_Operand_GetType(srcOpnd),
                                    pInst,
                                    &pNewInsertedInst);

                                /* dst */
                                VIR_Operand_SetSymbol(pNewInsertedInst->dest, pFunc, newDstSymId);
                                VIR_Operand_SetEnable(pNewInsertedInst->dest, VIR_ENABLE_XYZW);
                                VIR_Operand_SetPrecision(pNewInsertedInst->dest, VIR_Operand_GetPrecision(destOpnd));
                                VIR_Symbol_SetPrecision(pSym, VIR_Operand_GetPrecision(destOpnd));
                                if (VIR_Operand_GetPrecision(srcOpnd) == VIR_PRECISION_HIGH ||
                                    VIR_Operand_GetPrecision(destOpnd) == VIR_PRECISION_HIGH)
                                {
                                    VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                                }

                                vscVIR_AddNewDef(pDuInfo, pNewInsertedInst, newDstRegNo, 1,
                                    VIR_ENABLE_XYZW,
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvFALSE, gcvFALSE, gcvNULL);

                                /* src */
                                VIR_Function_DupOperand(pFunc, srcOpnd, &newOpnd);
                                pNewInsertedInst->src[VIR_Operand_Src0] = newOpnd;
                                VIR_Operand_SetType(newOpnd, VIR_Operand_GetType(defDest));

                                if (srcEnable & (1 << pDef->defKey.channel))
                                {
                                    vscVIR_DeleteUsage(pDuInfo,
                                                    pDef->defKey.pDefInst,
                                                    pInst,
                                                    srcOpnd,
                                                    srcInfo.u1.virRegInfo.virReg,
                                                    1,
                                                    (1 << pDef->defKey.channel),
                                                    VIR_HALF_CHANNEL_MASK_FULL,
                                                    gcvNULL);

                                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pDef->defKey.pDefInst,
                                            pNewInsertedInst,
                                            newOpnd,
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
                        VIR_Operand_GetType(srcOpnd));
                    VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XYZW);

                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pNewInsertedInst,
                                            pInst,
                                            srcOpnd,
                                            newDstRegNo,
                                            1,
                                            VIR_ENABLE_XYZW,
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }
            if (defIdxArray)
            {
                vscMM_Free(&pMP->mmWrapper, defIdxArray);
            }
        }
    }

    return errCode;
}

/* For dual16 shader, we need to patch the code for some cases:
   1) insert precison conv for implicit type interpretion
*/
VSC_ErrCode vscVIR_PatchDual16Shader(VIR_Shader* pShader, VIR_DEF_USAGE_INFO* pDuInfo)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_FuncIterator               func_iter;
    VIR_FunctionNode*              func_node;
    VSC_PRIMARY_MEM_POOL           pmp;

    if (VIR_Shader_isDual16Mode(pShader))
    {
        vscPMP_Intialize(&pmp, gcvNULL, 512, sizeof(void*), gcvTRUE);

        VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
        for (func_node = VIR_FuncIterator_First(&func_iter);
             func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
        {
            VIR_Function    *func = func_node->function;
            VIR_InstIterator inst_iter;
            VIR_Instruction *inst;

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                 inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                errCode = _InsertPrecisionConvInst(pShader, func, inst, pDuInfo, &pmp);

                if (errCode != VSC_ERR_NONE)
                {
                    goto OnError;
                }
            }
        }

        vscPMP_Finalize(&pmp);

        if (VirSHADER_DumpCodeGenVerbose(pShader->_id))
        {
            VIR_Shader_Dump(gcvNULL, "After Patching Dual16 Shader", pShader, gcvTRUE);
        }
    }

OnError:
    return errCode;
}

