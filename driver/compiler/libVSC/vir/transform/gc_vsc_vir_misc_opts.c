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

DEF_QUERY_PASS_PROP(vscVIR_RemoveNop)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_PRE;
}

VSC_ErrCode vscVIR_RemoveNop(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;
        VIR_Instruction* nextinst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; )
        {
            nextinst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter);

            if (VIR_Inst_GetOpcode(inst) == VIR_OP_NOP)
            {
                VIR_Function_DeleteInstruction(func, inst);
            }

            inst = nextinst;
        }
    }

    return errCode;
}

/* To implement the partial def of VX instruction (e.g., select_add),
   we need to make select_add to be conditional def.
   But this conditional def is only valid for its real variable def, not the temp.
   select_add is implemented as a call. Thus the destination is always the return temp.
   we need to change the temp varaible to real variable (local variable).
   For example,
   bin0to3 = viv_intrinsic_vxmc_SelectAdd_us1(us0, us1,u32_bin0to3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));
   is generated as
   t1 = select_add();
   bin0to3 = t1;
   The conditional def is on bin0to3, not t1.
   */
DEF_QUERY_PASS_PROP(vscVIR_VX_ReplaceDest)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_PRE;
}
VSC_ErrCode vscVIR_VX_ReplaceDest(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
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
            if (VIR_OPCODE_isVXOnly(VIR_Inst_GetOpcode(inst)))
            {
                if (VIR_Inst_GetDest(inst))
                {
                    VIR_Symbol  *sym = VIR_Operand_GetSymbol(VIR_Inst_GetDest(inst));
                    VIR_Symbol  *varSym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetDest(inst));
                    VIR_Operand *varOpnd = gcvNULL;

                    if (VIR_Symbol_isVreg(sym))
                    {
                        /* we generate a global variable for the outparm for VX call return */
                        if (VIR_Symbol_isGlobalVar(varSym))
                        {
                            VIR_Instruction *next = VIR_Inst_GetNext(inst);
                            gctBOOL         found = gcvFALSE;
                            VIR_Symbol      *srcSym = gcvNULL;
                            VIR_Symbol      *dstSym = gcvNULL, *dstVarSym = gcvNULL;
                            while (next && VIR_Inst_GetOpcode(next) == VIR_OP_MOV)
                            {
                                dstSym = VIR_Operand_GetSymbol(VIR_Inst_GetDest(next));
                                dstVarSym = VIR_Operand_GetUnderlyingSymbol(VIR_Inst_GetDest(next));
                                srcSym = VIR_Operand_GetSymbol(VIR_Inst_GetSource(next, 0));

                                if (srcSym == sym)
                                {
                                    if (dstVarSym != gcvNULL)
                                    {
                                        varOpnd = VIR_Inst_GetDest(next);
                                        found = gcvTRUE;
                                        break;
                                    }
                                    else
                                    {
                                        sym = dstSym;
                                        next = VIR_Inst_GetNext(next);
                                    }
                                }
                                else
                                {
                                    break;
                                }
                            }
                            if (found)
                            {
                                VIR_Instruction *prev = next;
                                while (prev != inst)
                                {
                                    VIR_Inst_SetOpcode(prev, VIR_OP_NOP);
                                    VIR_Inst_SetSrcNum(prev, 0);
                                    VIR_Inst_SetDest(prev, gcvNULL);
                                    prev = VIR_Inst_GetPrev(prev);
                                }
                                VIR_Inst_SetDest(inst, varOpnd);
                            }
                        }
                    }
                }
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
        ImmValues->scalarVal.uValue = VIR_Operand_GetImmediateUint(Opnd);
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
                return !CAN_EXACTLY_CVT_S23E8_2_S11E8(VIR_Operand_GetImmediateUint(Opnd));
            }
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
            if (VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_S32_2_S16(VIR_Operand_GetImmediateInt(Opnd));
            }
            else
            {
                return !CAN_EXACTLY_CVT_S32_2_S20(VIR_Operand_GetImmediateInt(Opnd));
            }

        case VIR_TYPE_UINT32:
            if (VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_U32_2_U16(VIR_Operand_GetImmediateUint(Opnd));
            }
            else
            {
                return !CAN_EXACTLY_CVT_U32_2_U20(VIR_Operand_GetImmediateUint(Opnd));
            }
        default:
            return gcvFALSE;
        }
    }
    else if (VIR_Operand_GetOpKind(Opnd) == VIR_OPND_CONST)
    {
        constValue = (VIR_Const*)VIR_GetSymFromId(&pShader->constTable, VIR_Operand_GetConstId(Opnd));
        *ImmValues = constValue->value;
        *ImmTypeId = constValue->type;
        return gcvTRUE;
    }

    return gcvFALSE;
}

static gctBOOL _IsConstScalar(VIR_Shader* pShader,
                              VIR_Instruction* pInst,
                              VIR_Operand* pOpnd,
                              VIR_ConstVal *pConstVal)
{
    gctBOOL           isConstScalar = gcvFALSE;
    VIR_OperandInfo   srcInfo;
    VIR_Const*        pConst = gcvNULL;

    VIR_Operand_GetOperandInfo(pInst, pOpnd, &srcInfo);

    if (srcInfo.isUniform)
    {
        VIR_Symbol* uniformSym = VIR_Operand_GetSymbol(pOpnd);
        VIR_Uniform* uniform = VIR_Symbol_GetUniform(uniformSym);

        if (uniform && isSymUniformCompiletimeInitialized(uniformSym))
        {
            pConst = VIR_Shader_GetConstFromId(pShader, VIR_Uniform_GetInitializer(uniform));
            if (VIR_Type_isVector(VIR_Shader_GetTypeFromId(pShader, pConst->type)))
            {
                isConstScalar = gcvTRUE;
            }
        }
    }
    else if (srcInfo.isVecConst)
    {
        pConst = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(pOpnd));
        isConstScalar = gcvTRUE;
    }

    if (isConstScalar)
    {
        gcmASSERT(pConst);
        if (pConstVal)
        {
            *pConstVal = pConst->value;
        }
    }

    return isConstScalar;
}

DEF_QUERY_PASS_PROP(vscVIR_PutScalarConstToImm)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
}

VSC_ErrCode vscVIR_PutScalarConstToImm(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    gctUINT           srcInx;
    gctUINT           i, j;
    VSC_HW_CONFIG*    pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    if (!pHwCfg->hwFeatureFlags.hasSHEnhance2)
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
            for (srcInx = 0; srcInx < VIR_Inst_GetSrcNum(inst); ++srcInx)
            {
                VIR_Operand         *srcOpnd = VIR_Inst_GetSource(inst, srcInx);
                VIR_Modifier        srcModifier = VIR_Operand_GetModifier(srcOpnd);
                VIR_TypeId          srcType = VIR_Operand_GetType(srcOpnd);
                VIR_Swizzle         srcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);
                VIR_Swizzle         channelSwizzle[VIR_CHANNEL_COUNT] = {VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X};
                VIR_Swizzle         finalSwizzle = VIR_SWIZZLE_X;
                VIR_ConstVal        constValue;
                gctUINT             immedValue = 0;
                gctBOOL             hasSameValue = gcvTRUE;

                if (!_IsConstScalar(pShader, inst, srcOpnd, &constValue))
                {
                    continue;
                }

                srcType = VIR_GetTypeComponentType(srcType);

                /* Get the channel swizzle. */
                for (i = 0; i < VIR_CHANNEL_COUNT; i++)
                {
                    channelSwizzle[i] = VIR_Swizzle_GetChannel(srcSwizzle, i);
                }

                /* Check if two channels have the same swizzle or the same value. */
                for (i = 0; i < VIR_CHANNEL_COUNT - 1; i++)
                {
                    j = i + 1;
                    if (channelSwizzle[i] == channelSwizzle[j])
                    {
                        finalSwizzle = channelSwizzle[i];
                        channelSwizzle[j] = channelSwizzle[i];
                    }
                    else
                    {
                        if (constValue.vecVal.u32Value[channelSwizzle[i]] == constValue.vecVal.u32Value[channelSwizzle[j]])
                        {
                            finalSwizzle = channelSwizzle[i];
                            channelSwizzle[j] = channelSwizzle[i];
                        }
                        else
                        {
                            hasSameValue = gcvFALSE;
                            break;
                        }
                    }
                }

                if (!hasSameValue)
                {
                    continue;
                }
                gcmASSERT((finalSwizzle == channelSwizzle[0]) &&
                          (finalSwizzle == channelSwizzle[1]) &&
                          (finalSwizzle == channelSwizzle[2]) &&
                          (finalSwizzle == channelSwizzle[3]));

                /* Get the right data based on the modififer. */
                immedValue = constValue.vecVal.u32Value[finalSwizzle];
                switch (srcModifier)
                {
                case VIR_MOD_NEG:
                    switch (srcType)
                    {
                    case VIR_TYPE_FLOAT32:
                        immedValue = gcoMATH_FloatAsUInt(-constValue.vecVal.f32Value[finalSwizzle]);
                        break;

                    case VIR_TYPE_INT32:
                    case VIR_TYPE_UINT32:
                        immedValue = (gctUINT)(-constValue.vecVal.i32Value[finalSwizzle]);
                        break;

                    case VIR_TYPE_INT16:
                    case VIR_TYPE_UINT16:
                        immedValue = (gctUINT)(-constValue.vecVal.i16Value[finalSwizzle]);
                        break;

                    case VIR_TYPE_INT8:
                    case VIR_TYPE_UINT8:
                        immedValue = (gctUINT)(-constValue.vecVal.i8Value[finalSwizzle]);
                        break;

                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;

                case VIR_MOD_ABS:
                    switch (srcType)
                    {
                    case VIR_TYPE_FLOAT32:
                        immedValue = gcoMATH_FloatAsUInt(gcoMATH_Absolute(constValue.vecVal.f32Value[finalSwizzle]));
                        break;

                    case VIR_TYPE_INT32:
                    case VIR_TYPE_UINT32:
                        immedValue = (gctUINT32)((constValue.vecVal.i32Value[finalSwizzle] > 0) ?
                            constValue.vecVal.i32Value[finalSwizzle] : -constValue.vecVal.i32Value[finalSwizzle]);
                        break;

                    case VIR_TYPE_INT16:
                    case VIR_TYPE_UINT16:
                        immedValue = (gctUINT32)((constValue.vecVal.i16Value[finalSwizzle] > 0) ?
                            constValue.vecVal.i16Value[finalSwizzle] : -constValue.vecVal.i16Value[finalSwizzle]);
                        break;

                    case VIR_TYPE_INT8:
                    case VIR_TYPE_UINT8:
                        immedValue = (gctUINT32)((constValue.vecVal.i8Value[finalSwizzle] > 0) ?
                            constValue.vecVal.i8Value[finalSwizzle] : -constValue.vecVal.i8Value[finalSwizzle]);
                        break;

                    default:
                        gcmASSERT(gcvFALSE);
                        break;
                    }
                    break;

                default:
                    break;
                }

                /* Change the operand from CONST to IMMED. */
                VIR_Operand_SetImmediateUint(srcOpnd, immedValue);
                VIR_Operand_SetType(srcOpnd, srcType);
                VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);
                VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);
            }
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PutImmValueToUniform)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
}

VSC_ErrCode vscVIR_PutImmValueToUniform(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*    pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
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

            gcmASSERT(VIR_Inst_GetSrcNum(inst) <= VIR_MAX_SRC_NUM);

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
            if (gcUseFullNewLinker(pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2) && numChange > 1)
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
                        VIR_Operand_SetSym(opnd, sym);
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
                    VIR_Operand_SetSym(opnd, sym);
                    VIR_Operand_SetSwizzle(opnd, swizzle);
                }
            }
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_CheckCstRegFileReadPortLimitation)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    /* Only constant allocation is enable, we can check constant reg file read port limitation */
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;

    /* Temp invaliate rd flow here (before IS/RA) as dubo pass does not update du */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
}

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          func_node;
    VIR_Operand*               firstOpnd = gcvNULL;
    VIR_Operand*               opnd;
    gctUINT                    i, j, firstConstRegNo, thisConstRegNo, newDstRegNo;
    VIR_SymId                  newDstSymId;
    VIR_Symbol*                pSym = gcvNULL, *pNewSym = gcvNULL, *firstSym = gcvNULL;
    VIR_Uniform*               pUniform;
    VIR_Instruction*           pNewInsertedInst;
    gctBOOL                    bFirstConstRegIndexing, bHitReadPortLimitation;
    VIR_Operand *              newOpnd;

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
                        /* We need to check a sampler if it is treated as a constant register. */
                        else if (VIR_Symbol_GetKind(pSym) == VIR_SYM_SAMPLER &&
                                 isSymUniformTreatSamplerAsConst(pSym))
                        {
                            pUniform = VIR_Symbol_GetSampler(pSym);
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
                            firstOpnd = opnd;
                            firstSym  = pSym;
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
                            VIR_Precision precision;

                            /* decide which operand to replace, do not replace image uniform, 512 bit uniform */
                            if (VIR_Symbol_GetKind(pSym) == VIR_SYM_IMAGE ||
                                (VIR_OPCODE_U512_SrcNo(VIR_Inst_GetOpcode(inst)) > 0 &&
                                 VIR_OPCODE_U512_SrcNo(VIR_Inst_GetOpcode(inst)) == (gctINT)j) )
                            {
                                VIR_Operand * tmpOpnd = opnd;
                                VIR_Symbol *  tmpSym  = pSym;

                                /* swap current opnd/sym with firstOpnd/firstSym */
                                gcmASSERT(firstOpnd != gcvNULL);
                                opnd = firstOpnd;
                                pSym = firstSym;
                                firstOpnd = tmpOpnd;
                                firstSym = tmpSym;
                            }

                            precision = VIR_Operand_GetPrecision(opnd);

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
                            errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_Operand_GetType(opnd), inst, gcvTRUE, &pNewInsertedInst);
                            ON_ERROR(errCode, "Add instruction before");

                            /* dst */
                            newOpnd = VIR_Inst_GetDest(pNewInsertedInst);
                            VIR_Operand_SetSymbol(newOpnd, func, newDstSymId);
                            VIR_Operand_SetEnable(newOpnd, VIR_ENABLE_XYZW);
                            VIR_Symbol_SetPrecision(pNewSym, precision);
                            VIR_Operand_SetPrecision(newOpnd, precision);
                            if(precision == VIR_PRECISION_HIGH)
                            {
                                VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                            }

                            /* src */
                            newOpnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
                            VIR_Operand_SetSymbol(newOpnd, func, pSym->index);
                            VIR_Operand_SetSwizzle(newOpnd, VIR_SWIZZLE_XYZW);
                            VIR_Operand_SetType(newOpnd, VIR_Operand_GetType(opnd));
                            VIR_Operand_SetPrecision(newOpnd, VIR_Operand_GetPrecision(opnd));
                            VIR_Operand_SetMatrixConstIndex(newOpnd, VIR_Operand_GetMatrixConstIndex(opnd));
                            VIR_Operand_SetIsConstIndexing(newOpnd, VIR_Operand_GetIsConstIndexing(opnd));
                            VIR_Operand_SetRelAddrMode(newOpnd, VIR_Operand_GetRelAddrMode(opnd));
                            if (VIR_Operand_GetIsConstIndexing(opnd))
                            {
                                VIR_Operand_SetRelIndexingImmed(newOpnd, VIR_Operand_GetRelIndexing(opnd));
                            }
                            else
                            {
                                VIR_Operand_SetRelIndexing(newOpnd, VIR_Operand_GetRelIndexing(opnd));
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

                            vscVIR_AddNewDef(pDuInfo, pNewInsertedInst, newDstRegNo, 1, VIR_ENABLE_XYZW, VIR_HALF_CHANNEL_MASK_FULL,
                                             gcvNULL, gcvNULL);

                            vscVIR_AddNewUsageToDef(pDuInfo, pNewInsertedInst, inst, opnd,
                                                    gcvFALSE, newDstRegNo, 1, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opnd)),
                                                    VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
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
                                        VIR_DEF_USAGE_INFO* pDuInfo,
                                        VSC_MM* pMM)
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

    vscBV_Initialize(&tempMask, pMM, pDuInfo->baseTsDFA.baseDFA.flowSize);
    vscBV_Initialize(&depDefIdxMask, pMM, pDuInfo->baseTsDFA.baseDFA.flowSize);

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

DEF_QUERY_PASS_PROP(vscVIR_CheckPosAndDepthConflict)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedRdFlow = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

/* Pos and depth might be overlapped (conflicted). As HW needs postion is put into r0 and
   depth is put into r0.z, so if such overlapp is on Z, then we will encounter error. If
   such overlap happens, we need break it by inserting an extra MOV at the end of shader
   for depth (it is ok if inserting an extra MOV at the begin of shader for pos) */
VSC_ErrCode vscVIR_CheckPosAndDepthConflict(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
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
    VIR_Operand *              opnd;

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
    if (!_IsPosAndDepthConflicted(pShader, pPosSym, pDepthSym, pDuInfo, pPassWorker->basePassWorker.pMM))
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
    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
    depthTypeId = VIR_Type_GetBaseTypeId(VIR_Symbol_GetType(pDepthSym));
    VIR_Operand_SetTempRegister(opnd, pShader->mainFunction, newRegSymId, depthTypeId);
    VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pDepthSym));
    VIR_Symbol_SetPrecision(pNewRegSym, VIR_Symbol_GetPrecision(pDepthSym));

    /* dst */
    opnd = VIR_Inst_GetDest(pNewInsertedInst);
    VIR_Operand_SetSymbol(opnd, pShader->mainFunction, pDepthSym->index);
    VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pDepthSym));

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

                    opnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
                    vscVIR_AddNewUsageToDef(pDuInfo, inst, pNewInsertedInst, opnd,
                                            gcvFALSE, newRegNo, 1, VIR_ENABLE_X,
                                            VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
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

DEF_QUERY_PASS_PROP(vscVIR_AddOutOfBoundCheckSupport)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

VSC_ErrCode vscVIR_AddOutOfBoundCheckSupport(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode               errCode = VSC_ERR_NONE;
    VIR_Shader*               pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*       pDuInfo = pPassWorker->pDuInfo;
    VSC_HW_CONFIG*            pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
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
    VIR_Operand *             opnd;

    if (!(pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK))
    {
        return VSC_ERR_NONE;
    }

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
                !VIR_OPCODE_isAttrLd(VIR_Inst_GetOpcode(inst)) &&
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
                pUnderlyingSym = VIR_Shader_GetSymFromId(pShader, pMemBaseUniform->baseBindingUniform);

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
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, gcvTRUE, &pNewInsertedInstX);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                opnd = VIR_Inst_GetDest(pNewInsertedInstX);
                VIR_Operand_SetSymbol(opnd, func, newDstSymId);
                VIR_Operand_SetEnable(opnd, VIR_ENABLE_X);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                opnd = VIR_Inst_GetSource(pNewInsertedInstX, VIR_Operand_Src0);
                VIR_Operand_SetSymbol(opnd, func, pSym->index);
                VIR_Operand_SetSwizzle(opnd, VIR_Operand_GetSwizzle(pOpnd));
                VIR_Operand_SetType(opnd, VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

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
                                            opnd,
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
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, gcvTRUE, &pNewInsertedInstY);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                opnd = VIR_Inst_GetDest(pNewInsertedInstY);
                VIR_Operand_SetSymbol(opnd, func, newDstSymId);
                VIR_Operand_SetEnable(opnd, VIR_ENABLE_Y);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                opnd = VIR_Inst_GetSource(pNewInsertedInstY, VIR_Operand_Src0);
                VIR_Operand_SetSymbol(opnd, func, pMemBaseUniform->sym);
                VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_YYYY);
                VIR_Operand_SetType(opnd, VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

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
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT32, inst, gcvTRUE, &pNewInsertedInstZ);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                opnd = VIR_Inst_GetDest(pNewInsertedInstZ);
                VIR_Operand_SetSymbol(opnd, func, newDstSymId);
                VIR_Operand_SetEnable(opnd, VIR_ENABLE_Z);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                opnd = VIR_Inst_GetSource(pNewInsertedInstZ, VIR_Operand_Src0);
                VIR_Operand_SetSymbol(opnd, func, pMemBaseUniform->sym);
                VIR_Operand_SetSwizzle(opnd, VIR_SWIZZLE_ZZZZ);
                VIR_Operand_SetType(opnd, VIR_TYPE_UINT32);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

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
            VIR_OPCODE_isAttrLd(opcode)       ||
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
                *skipLowp = gcvFALSE;
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

DEF_QUERY_PASS_PROP(vscVIR_AdjustPrecision)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

/* Based on HW dual16 restriction, adjust operand/symbol's precision */
VSC_ErrCode vscVIR_AdjustPrecision(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                    errCode = VSC_ERR_NONE;
    VIR_Shader*                    pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*            pDuInfo = pPassWorker->pDuInfo;
    VSC_HW_CONFIG*                 pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_FuncIterator               func_iter;
    VIR_FunctionNode*              func_node;
    gctUINT                        dual16PrecisionRule = gcmOPT_DualFP16PrecisionRule();


    /* currently only PS is dual16-able */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_FRAGMENT &&
        VIR_Shader_GetClientApiVersion(pShader) != gcvAPI_OPENVK)
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
                    (VIR_GetTypeFlag(VIR_Operand_GetType(VIR_Inst_GetDest(inst))) & VIR_TYFLAG_ISINTEGER) &&
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

                /* HW Cvt2OutColFmt has issue with 0x2,
                   output should be high-precision, */
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
                                    if(VIR_Type_isInteger(VIR_Symbol_GetType(varSym)) ||
                                       (dual16PrecisionRule & Dual16_PrecisionRule_OUTPUT_HP))
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
                                    if(VIR_Type_isInteger(VIR_Symbol_GetType(sym)) ||
                                       (dual16PrecisionRule & Dual16_PrecisionRule_OUTPUT_HP))
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


VIR_Operand * _vscVIR_FindParentOperandFromIndex(VIR_Instruction* inst, VIR_Operand* index, gctUINT channel)
{
    VIR_Instruction* prev;

    for(prev = VIR_Inst_GetPrev(inst); prev; prev = VIR_Inst_GetPrev(prev))
    {
        VIR_Operand* dest;

        dest = VIR_Inst_GetDest(prev);
        if(VIR_Operand_GetSymbol(dest)->u1.vregIndex == VIR_Operand_GetSymbol(index)->u1.vregIndex &&
           VIR_Enable_Covers(VIR_Operand_GetEnable(dest), 1 << channel))
        {
            VIR_OpCode op = VIR_Inst_GetOpcode(prev);

            if(op == VIR_OP_LDARR)
            {
                index = VIR_Inst_GetSource(prev, 1);
                return _vscVIR_FindParentOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), 0));
            }
            else if(op == VIR_OP_MOVA)
            {
                index = VIR_Inst_GetSource(prev, 0);
                return _vscVIR_FindParentOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), channel));
            }
            else if(op == VIR_OP_GET_SAMPLER_IDX)
            {
                return VIR_Inst_GetSource(prev, 0);
            }
            else if (op == VIR_OP_MOV)
            {
                index = VIR_Inst_GetSource(prev, 0);
                gcmASSERT(VIR_Operand_isSymbol(index));

                if (VIR_Symbol_isImage(VIR_Operand_GetSymbol(index)))
                {
                    return index;
                }
                else
                {
                    return _vscVIR_FindParentOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), channel));
                }
            }
        }
    }

    return gcvNULL;
}

static VIR_Operand *
_ConvGetSamplerUniformOperand(
    IN VIR_Shader           *pShader,
    IN VIR_Instruction      *pInst
    )
{
    VIR_Operand             *samplerSrc = VIR_Inst_GetSource(pInst, 0);
    VIR_Symbol              *samplerSym;
    VIR_Instruction         *prev;

    gcmASSERT(VIR_Operand_isSymbol(samplerSrc));
    samplerSym = VIR_Operand_GetSymbol(samplerSrc);

    /* If the sampler sym is not a sampler, then it must be a function parameter, we need to find the parameter assignment. */
    if (!VIR_Symbol_isSampler(samplerSym))
    {
        gcmASSERT(VIR_Symbol_isVreg(samplerSym));
        for (prev = VIR_Inst_GetPrev(pInst); prev; prev = VIR_Inst_GetPrev(prev))
        {
            VIR_Operand* dest;
            VIR_Operand* src0;
            VIR_Symbol*  src0Sym;

            dest = VIR_Inst_GetDest(prev);
            if (VIR_Operand_GetSymbol(dest)->u1.vregIndex == samplerSym->u1.vregIndex)
            {
                gcmASSERT(VIR_Inst_GetOpcode(prev) == VIR_OP_MOV);

                src0 = VIR_Inst_GetSource(prev, 0);
                gcmASSERT(VIR_Operand_isSymbol(src0));

                src0Sym = VIR_Operand_GetSymbol(src0);
                if (VIR_Symbol_isSampler(src0Sym))
                {
                    return src0;
                }
                else
                {
                    return _ConvGetSamplerUniformOperand(pShader, prev);
                }
            }
        }
    }

    return samplerSrc;
}

static VSC_ErrCode
_ConvGetSamplerIdx(
    IN VIR_Shader           *pShader,
    IN VIR_Instruction      *pGetSamplerIdxInst,
    IN VIR_Instruction      *pInst
    )
{
    VSC_ErrCode              errCode = VSC_ERR_NONE;
    VIR_Instruction         *pNextInst;
    VIR_Operand             *dest = VIR_Inst_GetDest(pInst);

    for (pNextInst = VIR_Inst_GetNext(pInst);
         pNextInst && (VIR_Inst_GetFunction(pNextInst) == VIR_Inst_GetFunction(pGetSamplerIdxInst));
         pNextInst = VIR_Inst_GetNext(pNextInst))
    {
        VIR_Operand         *src;
        VIR_Symbol          *srcSym;
        VIR_VirRegId         virRegId;
        VIR_Operand         *pFromOpnd, *pToOpnd;
        VIR_OpCode           opcode = VIR_Inst_GetOpcode(pNextInst);
        gctUINT              srcIdx = (opcode == VIR_OP_LDARR) ? 1 : 0;

        src = VIR_Inst_GetSource(pNextInst, srcIdx);
        if (src == gcvNULL || !VIR_Operand_isSymbol(src))
        {
            continue;
        }
        srcSym = VIR_Operand_GetSymbol(src);
        virRegId = VIR_Symbol_GetVregIndex(srcSym);

        if (VIR_Operand_GetSymbol(dest)->u1.vregIndex == virRegId)
        {
            switch (opcode)
            {
            case VIR_OP_MOV:
            case VIR_OP_MOVA:
                errCode = _ConvGetSamplerIdx(pShader, pGetSamplerIdxInst, pNextInst);
                CHECK_ERROR(errCode, "_ConvGetSamplerIdx failed.");
                break;

            case VIR_OP_LDARR:
                errCode = _ConvGetSamplerIdx(pShader, pGetSamplerIdxInst, pNextInst);
                CHECK_ERROR(errCode, "_ConvGetSamplerIdx failed.");
                break;

            case VIR_OP_GET_SAMPLER_LBS:
            case VIR_OP_GET_SAMPLER_LMM:
            case VIR_OP_GET_SAMPLER_LS:
                /* Copy the texture from GET_SAMPLER_IDX. */
                pFromOpnd = VIR_Inst_GetSource(pGetSamplerIdxInst, 0);
                pToOpnd = VIR_Inst_GetSource(pNextInst, 0);
                VIR_Operand_Copy(pToOpnd, pFromOpnd);
                /* Copy the offset from GET_SAMPLER_IDX. */
                pFromOpnd = VIR_Inst_GetSource(pGetSamplerIdxInst, 1);
                pToOpnd = VIR_Inst_GetSource(pNextInst, 1);
                VIR_Operand_Copy(pToOpnd, pFromOpnd);
                break;

            default:
                if (VIR_OPCODE_isTexLd(opcode))
                {
                    /* Copy the texture from GET_SAMPLER_IDX. */
                    pFromOpnd = VIR_Inst_GetSource(pGetSamplerIdxInst, 0);
                    pToOpnd = VIR_Inst_GetSource(pNextInst, 0);
                    VIR_Operand_Copy(pToOpnd, pFromOpnd);

                    /* Set the indexing. */
                    pFromOpnd = VIR_Inst_GetSource(pGetSamplerIdxInst, 1);
                    VIR_Operand_SetIndexingFromOperand(pShader, pToOpnd, pFromOpnd);
                }
                break;
            }
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_ConvertVirtualInstructions)
{
   pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
}

VSC_ErrCode
vscVIR_ConvertVirtualInstructions(
    VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode         errCode  = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator    funcIter;
    VIR_FunctionNode    *funcNode;

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before Convert Virtual Instructions.", pShader, gcvTRUE);
    }

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
                /* GET_SAMPLER_XXX instructions should be existed in main function only. */
                case VIR_OP_GET_SAMPLER_IDX:
                {
                    VIR_Operand* samplerSrc = VIR_Inst_GetSource(inst, 0);

                    samplerSrc = _ConvGetSamplerUniformOperand(pShader, inst);
                    if (VIR_Inst_GetSource(inst, 0) != samplerSrc)
                    {
                        VIR_Operand_Copy(VIR_Inst_GetSource(inst, 0), samplerSrc);
                    }
                    errCode = _ConvGetSamplerIdx(pShader, inst, inst);
                    break;
                }

                case VIR_OP_GET_SAMPLER_LMM:
                {
                    VIR_Operand* samplerSrc = VIR_Inst_GetSource(inst, 0);
                    VIR_Operand* offsetSrc = VIR_Inst_GetSource(inst, 1);
                    VIR_Operand* parentSrc = gcvNULL;
                    VIR_Symbol* samplerSym;
                    VIR_Uniform* samplerUniform;
                    VIR_Type* samplerType = gcvNULL;
                    VIR_Type* dstType;
                    VIR_TypeId uniformTypeId = VIR_TYPE_FLOAT_X3;

                    dstType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(VIR_Inst_GetDest(inst)));

                    if (VIR_Type_isInteger(dstType))
                    {
                        uniformTypeId = VIR_TYPE_INTEGER_X3;
                    }

                    gcmASSERT(VIR_Operand_isSymbol(samplerSrc));
                    samplerSym = VIR_Operand_GetSymbol(samplerSrc);
                    if(!VIR_Symbol_isSampler(samplerSym))
                    {
                        parentSrc = _vscVIR_FindParentOperandFromIndex(inst, samplerSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerSrc), 0));
                        samplerSym = VIR_Operand_GetSymbol(parentSrc);
                    }
                    gcmASSERT(samplerSym && VIR_Symbol_isSampler(samplerSym));
                    samplerType = VIR_Symbol_GetType(samplerSym);
                    samplerUniform = VIR_Symbol_GetSampler(samplerSym);
                    if(samplerUniform->u.samplerOrImageAttr.lodMinMax == VIR_INVALID_ID)
                    {
                        VIR_Uniform* lodMinMaxUniform;
                        VIR_NameId nameId;
                        VIR_SymId llmSymID = VIR_INVALID_ID;
                        VIR_Symbol * sym;
                        gctCHAR name[128] = "#";

                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerSym));
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$LodMinMax");
                        errCode = VIR_Shader_AddString(pShader,
                                                       name,
                                                       &nameId);

                        if (VIR_Type_isArray(samplerType))
                        {
                            errCode = VIR_Shader_AddArrayType(pShader,
                                                              uniformTypeId,
                                                              VIR_Type_GetArrayLength(samplerType),
                                                              -1,
                                                              &uniformTypeId);
                        }
                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_UNIFORM,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(pShader, uniformTypeId),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &llmSymID);
                        samplerUniform->u.samplerOrImageAttr.lodMinMax = llmSymID;
                        sym = VIR_Shader_GetSymFromId(pShader, llmSymID);
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_COMPILER_GEN);
                        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_MEDIUM);
                        lodMinMaxUniform = VIR_Symbol_GetUniform(sym);
                        VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_LOD_MIN_MAX);
                        VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
                        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
                        lodMinMaxUniform->u.samplerOrImageAttr.parentSamplerSymId = VIR_Uniform_GetSymID(samplerUniform);
                        lodMinMaxUniform->u.samplerOrImageAttr.arrayIdxInParent = NOT_ASSIGNED;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerSrc, func, samplerUniform->u.samplerOrImageAttr.lodMinMax);
                    VIR_Operand_SetType(samplerSrc, uniformTypeId);

                    /* Convert offset operand to index if needed. */
                    if (offsetSrc && !VIR_Operand_isUndef(offsetSrc))
                    {
                        VIR_Operand_SetIndexingFromOperand(pShader, samplerSrc, offsetSrc);
                    }
                    else if (parentSrc)
                    {
                        samplerSrc->u.n.u2.vlInfo._isConstIndexing = parentSrc->u.n.u2.vlInfo._isConstIndexing;
                        samplerSrc->u.n.u2.vlInfo._relAddrMode = parentSrc->u.n.u2.vlInfo._relAddrMode;
                        samplerSrc->u.n.u2.vlInfo._matrixConstIndex = parentSrc->u.n.u2.vlInfo._matrixConstIndex;
                        samplerSrc->u.n.u2.vlInfo._relIndexing = parentSrc->u.n.u2.vlInfo._relIndexing;
                        samplerSrc->u.n.u2.vlInfo._isSymLocal = parentSrc->u.n.u2.vlInfo._isSymLocal;
                        samplerSrc->u.n.u2.vlInfo._relAddrLevel = parentSrc->u.n.u2.vlInfo._relAddrLevel;
                    }
                    break;
                }

                case VIR_OP_GET_SAMPLER_LBS:
                {
                    VIR_Operand* samplerOrImageSrc = VIR_Inst_GetSource(inst, 0);
                    VIR_Operand* offsetSrc = VIR_Inst_GetSource(inst, 1);
                    VIR_Operand* parentSrc = gcvNULL;
                    VIR_Symbol* samplerOrImageSym;
                    VIR_Type* samplerOrImageType = gcvNULL;
                    VIR_Uniform* samplerOrImageUniform;
                    VIR_TypeId uniformTypeId = VIR_TYPE_INTEGER_X4;

                    gcmASSERT(VIR_Operand_isSymbol(samplerOrImageSrc));
                    samplerOrImageSym = VIR_Operand_GetSymbol(samplerOrImageSrc);
                    if(!VIR_Symbol_isSampler(samplerOrImageSym) && !VIR_Symbol_isImage(samplerOrImageSym))
                    {
                        parentSrc = _vscVIR_FindParentOperandFromIndex(inst, samplerOrImageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerOrImageSrc), 0));
                        samplerOrImageSym = VIR_Operand_GetSymbol(parentSrc);
                    }
                    gcmASSERT(samplerOrImageSym &&
                              (VIR_Symbol_isSampler(samplerOrImageSym) || VIR_Symbol_isImage(samplerOrImageSym)));
                    samplerOrImageType = VIR_Symbol_GetType(samplerOrImageSym);
                    if (VIR_Symbol_isSampler(samplerOrImageSym))
                    {
                        samplerOrImageUniform = VIR_Symbol_GetSampler(samplerOrImageSym);
                    }
                    else
                    {
                        samplerOrImageUniform = VIR_Symbol_GetImage(samplerOrImageSym);
                    }
                    if(samplerOrImageUniform->u.samplerOrImageAttr.levelBaseSize == VIR_INVALID_ID)
                    {
                        VIR_Uniform* levelBaseSizeUniform;
                        VIR_NameId nameId;
                        VIR_SymId lbsSymID = VIR_INVALID_ID;
                        VIR_Symbol * sym;
                        gctCHAR name[128] = "#";

                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerOrImageSym));
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$LevelBaseSize");
                        errCode = VIR_Shader_AddString(pShader,
                                                       name,
                                                       &nameId);

                        if (VIR_Type_isArray(samplerOrImageType))
                        {
                            errCode = VIR_Shader_AddArrayType(pShader,
                                                              uniformTypeId,
                                                              VIR_Type_GetArrayLength(samplerOrImageType),
                                                              -1,
                                                              &uniformTypeId);
                        }

                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_UNIFORM,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(pShader, uniformTypeId),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &lbsSymID);
                        samplerOrImageUniform->u.samplerOrImageAttr.levelBaseSize = lbsSymID;
                        sym = VIR_Shader_GetSymFromId(pShader, lbsSymID);
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_COMPILER_GEN);
                        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_MEDIUM);
                        levelBaseSizeUniform = VIR_Symbol_GetUniform(sym);
                        VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_LEVEL_BASE_SIZE);
                        VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
                        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
                        levelBaseSizeUniform->u.samplerOrImageAttr.parentSamplerSymId = VIR_Uniform_GetSymID(samplerOrImageUniform);
                        levelBaseSizeUniform->u.samplerOrImageAttr.arrayIdxInParent = NOT_ASSIGNED;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerOrImageSrc, func, samplerOrImageUniform->u.samplerOrImageAttr.levelBaseSize);
                    VIR_Operand_SetType(samplerOrImageSrc, VIR_TYPE_INTEGER_X4);

                    /* Convert offset operand to index if needed. */
                    if (offsetSrc && !VIR_Operand_isUndef(offsetSrc))
                    {
                        VIR_Operand_SetIndexingFromOperand(pShader, samplerOrImageSrc, offsetSrc);
                    }
                    else if (parentSrc)
                    {
                    }
                    break;
                }

                case VIR_OP_GET_SAMPLER_LS:
                {
                    VIR_Operand* samplerOrImageSrc = VIR_Inst_GetSource(inst, 0);
                    VIR_Operand* offsetSrc = VIR_Inst_GetSource(inst, 1);
                    VIR_Operand* parentSrc = gcvNULL;
                    VIR_Symbol* samplerOrImageSym;
                    VIR_Type* samplerOrImageType = gcvNULL;
                    VIR_Uniform* samplerOrImageUniform;
                    VIR_TypeId uniformTypeId = VIR_TYPE_INTEGER_X4;

                    gcmASSERT(VIR_Operand_isSymbol(samplerOrImageSrc));
                    samplerOrImageSym = VIR_Operand_GetSymbol(samplerOrImageSrc);
                    if(!VIR_Symbol_isSampler(samplerOrImageSym) && !VIR_Symbol_isImage(samplerOrImageSym))
                    {
                        parentSrc = _vscVIR_FindParentOperandFromIndex(inst, samplerOrImageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerOrImageSrc), 0));
                        samplerOrImageSym = VIR_Operand_GetSymbol(parentSrc);
                    }
                    gcmASSERT(samplerOrImageSym &&
                              (VIR_Symbol_isSampler(samplerOrImageSym) || VIR_Symbol_isImage(samplerOrImageSym)));
                    samplerOrImageType = VIR_Symbol_GetType(samplerOrImageSym);
                    if (VIR_Symbol_isSampler(samplerOrImageSym))
                    {
                        samplerOrImageUniform = VIR_Symbol_GetSampler(samplerOrImageSym);
                    }
                    else
                    {
                        samplerOrImageUniform = VIR_Symbol_GetImage(samplerOrImageSym);
                    }
                    if(samplerOrImageUniform->u.samplerOrImageAttr.levelsSamples == VIR_INVALID_ID)
                    {
                        VIR_Uniform* levelsSamplesUniform;
                        VIR_NameId nameId;
                        VIR_SymId lbsSymID = VIR_INVALID_ID;
                        VIR_Symbol * sym;
                        gctCHAR name[128] = "#";

                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerOrImageSym));
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "LevelsSamples");
                        errCode = VIR_Shader_AddString(pShader,
                                                       name,
                                                       &nameId);

                        if (VIR_Type_isArray(samplerOrImageType))
                        {
                            errCode = VIR_Shader_AddArrayType(pShader,
                                                              uniformTypeId,
                                                              VIR_Type_GetArrayLength(samplerOrImageType),
                                                              -1,
                                                              &uniformTypeId);
                        }

                        errCode = VIR_Shader_AddSymbol(pShader,
                                                       VIR_SYM_UNIFORM,
                                                       nameId,
                                                       VIR_Shader_GetTypeFromId(pShader, uniformTypeId),
                                                       VIR_STORAGE_UNKNOWN,
                                                       &lbsSymID);
                        samplerOrImageUniform->u.samplerOrImageAttr.levelsSamples = lbsSymID;
                        sym = VIR_Shader_GetSymFromId(pShader, lbsSymID);
                        VIR_Symbol_SetFlag(sym, VIR_SYMFLAG_COMPILER_GEN);
                        VIR_Symbol_SetPrecision(sym, VIR_PRECISION_MEDIUM);
                        levelsSamplesUniform = VIR_Symbol_GetUniform(sym);
                        VIR_Symbol_SetUniformKind(sym, VIR_UNIFORM_LEVELS_SAMPLES);
                        VIR_Symbol_SetAddrSpace(sym, VIR_AS_CONSTANT);
                        VIR_Symbol_SetTyQualifier(sym, VIR_TYQUAL_CONST);
                        levelsSamplesUniform->u.samplerOrImageAttr.parentSamplerSymId = VIR_Uniform_GetSymID(samplerOrImageUniform);
                        levelsSamplesUniform->u.samplerOrImageAttr.arrayIdxInParent = NOT_ASSIGNED;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerOrImageSrc, func, samplerOrImageUniform->u.samplerOrImageAttr.levelsSamples);
                    VIR_Operand_SetType(samplerOrImageSrc, VIR_TYPE_INTEGER_X4);

                    /* Convert offset operand to index if needed. */
                    if (offsetSrc && !VIR_Operand_isUndef(offsetSrc))
                    {
                        VIR_Operand_SetIndexingFromOperand(pShader, samplerOrImageSrc, offsetSrc);
                    }
                    else if (parentSrc)
                    {
                        VIR_Operand_SetIsConstIndexing(samplerOrImageSrc, VIR_Operand_GetIsConstIndexing(parentSrc));
                        VIR_Operand_SetRelAddrMode(samplerOrImageSrc, VIR_Operand_GetRelAddrMode(parentSrc));
                        VIR_Operand_SetMatrixConstIndex(samplerOrImageSrc, VIR_Operand_GetMatrixConstIndex(parentSrc));
                        VIR_Operand_SetRelIndex(samplerOrImageSrc, VIR_Operand_GetRelIndexing(parentSrc));
                        VIR_Operand_SetIsSymLocal(samplerOrImageSrc, VIR_Operand_isSymLocal(parentSrc));
                        VIR_Operand_SetRelAddrLevel(samplerOrImageSrc, VIR_Operand_GetRelAddrLevel(parentSrc));
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Convert Virtual Instructions.", pShader, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode vscVIR_PrecisionUpdateSrc(VIR_Shader* shader, VIR_Operand* operand)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    gcmASSERT(operand);

    switch(VIR_Operand_GetOpKind(operand))
    {
        case VIR_OPND_SYMBOL:
        case VIR_OPND_VIRREG:
        case VIR_OPND_ARRAY:
        case VIR_OPND_FIELD:
        case VIR_OPND_SAMPLER_INDEXING:
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
            VIR_Operand *texldOperand = (VIR_Operand*)operand;

            for(k = 0; k < VIR_TEXLDMODIFIER_COUNT; ++k)
            {
                if(VIR_Operand_GetTexldModifier(texldOperand, k) != gcvNULL)
                {
                    vscVIR_PrecisionUpdateSrc(shader, VIR_Operand_GetTexldModifier(texldOperand, k));
                    break;
                }
            }
            break;
        }
        case VIR_OPND_PARAMETERS:
        {
            gctUINT k;
            VIR_ParmPassing *parm = VIR_Operand_GetParameters(operand);

            for(k = 0; k < parm->argNum; k++)
            {
                vscVIR_PrecisionUpdateSrc(shader, parm->args[k]);
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
            break;
        default:
            break;
    }

    return errCode;
}

static VSC_ErrCode vscVIR_PrecisionUpdateDst(VIR_Instruction* inst)
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

static VSC_ErrCode _PrecisionUpdate(VIR_Shader* pShader, VIR_Dumper* dumper)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    if(!VIR_Shader_IsFS(pShader) ||
        VIR_Shader_GetClientApiVersion(pShader) == gcvAPI_OPENVK)
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
            VIR_InstIterator inst_iter;
            VIR_Instruction* inst;

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                 inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                gctUINT j;
                for(j = 0; j < VIR_Inst_GetSrcNum(inst); j++)
                {
                    VIR_Operand* src = VIR_Inst_GetSource(inst, j);

                    vscVIR_PrecisionUpdateSrc(pShader, src);
                }

                vscVIR_PrecisionUpdateDst(inst);

                /* VIR_Inst_Dump(dumper, inst); */
            }
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
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

                        if(symKind == VIR_SYM_UNIFORM && isSymUniformCompiletimeInitialized(symbol))
                        {
                            VIR_Uniform* uniform = VIR_Symbol_GetUniform(symbol);
                            VIR_ConstId constId = VIR_Uniform_GetInitializer(uniform);
                            VIR_Const* constVal = VIR_Shader_GetConstFromId(pShader, constId);
                            VIR_TypeId constTypeId = constVal->type;
                            gctUINT constTypeComponents = VIR_GetTypeComponents(constTypeId);
                            VIR_TypeId newConstTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, constTypeComponents, 1);
                            VIR_ConstVal newConstVal;
                            gctUINT i;
                            VIR_ConstId newConstId;

                            if(VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_UNSIGNED_INT ||
                               VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_BOOLEAN)
                            {
                                for(i = 0; i < constTypeComponents; i++)
                                {
                                    newConstVal.vecVal.f32Value[i] = (gctFLOAT)constVal->value.vecVal.u32Value[i];
                                }
                            }
                            else if(VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_SIGNED_INT)
                            {
                                for(i = 0; i < constTypeComponents; i++)
                                {
                                    newConstVal.vecVal.f32Value[i] = (gctFLOAT)constVal->value.vecVal.i32Value[i];
                                }

                            }
                            VIR_Shader_AddConstant(pShader, newConstTypeId, &newConstVal, &newConstId);
                            VIR_Uniform_SetInitializer(uniform, newConstId);
                        }
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
                        break;

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
            VIR_ConstId constId = VIR_Operand_GetConstId(operand);
            VIR_Const* constVal = VIR_Shader_GetConstFromId(pShader, constId);
            VIR_TypeId constTypeId = constVal->type;
            gctUINT constTypeComponents = VIR_GetTypeComponents(constTypeId);
            VIR_TypeId newConstTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, constTypeComponents, 1);
            VIR_ConstVal newConstVal;
            gctUINT i;
            VIR_ConstId newConstId;

            if(VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_UNSIGNED_INT ||
               VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_BOOLEAN)
            {
                for(i = 0; i < constTypeComponents; i++)
                {
                    newConstVal.vecVal.f32Value[i] = (gctFLOAT)constVal->value.vecVal.u32Value[i];
                }
            }
            else if(VIR_GetTypeFlag(constTypeId) & VIR_TYFLAG_IS_SIGNED_INT)
            {
                for(i = 0; i < constTypeComponents; i++)
                {
                    newConstVal.vecVal.f32Value[i] = (gctFLOAT)constVal->value.vecVal.i32Value[i];
                }

            }
            VIR_Shader_AddConstant(pShader, newConstTypeId, &newConstVal, &newConstId);
            VIR_Operand_SetConst(operand, newConstTypeId, newConstId);
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

    if(VIR_OPCODE_hasDest(opcode) && !VIR_OPCODE_Stores(opcode) && toBeConvertedDest)
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

            if(VIR_GetTypeComponentType(VIR_Operand_GetType(dest)) == VIR_GetTypeComponentType(VIR_Operand_GetType(src)))
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

                        VIR_Function_AddInstructionBefore(func, VIR_OP_AQ_F2I, destTypeId, inst, gcvTRUE, &newInst);
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

                        VIR_Function_AddInstructionBefore(func, VIR_OP_SIGN, destTypeId, inst, gcvTRUE, &signInst);
                        VIR_Operand_Copy(VIR_Inst_GetSource(signInst, 0), VIR_Inst_GetSource(inst, 0));
                        VIR_Function_AddInstructionBefore(func, VIR_OP_ABS, destTypeId, inst, gcvTRUE,&absInst);
                        VIR_Operand_Copy(VIR_Inst_GetSource(absInst, 0), VIR_Inst_GetSource(inst, 0));
                        VIR_Inst_SetOpcode(inst, VIR_OP_FLOOR);
                        VIR_Function_AddInstructionAfter(func, VIR_OP_MUL, destTypeId, inst, gcvTRUE, &mulInst);
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

static VSC_ErrCode _ConvertIntegerToFloat(
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

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Converting Integer to Float.", pShader, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode _ConvertPatchVerticesInToUniform(
    IN OUT VIR_Shader* pShader
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_ShaderKind     shaderKind = VIR_Shader_GetKind(pShader);
    VIR_FuncIterator   func_iter;
    VIR_FunctionNode*  func_node;
    VIR_Symbol*        origSym = gcvNULL;
    VIR_NameId         nameId = VIR_INVALID_ID;
    VIR_SymId          uniformSymId = VIR_INVALID_ID;
    VIR_Symbol*        uniformSym = gcvNULL;
    VIR_ConstId        constId = VIR_INVALID_ID;
    VIR_ConstVal       constVal;

    if (shaderKind != VIR_SHADER_TESSELLATION_CONTROL &&
        shaderKind != VIR_SHADER_TESSELLATION_EVALUATION)
    {
        return errCode;
    }

    /* Check if use gl_PatchVerticesIn. */
    origSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_PATCH_VERTICES_IN);
    if (origSym == gcvNULL)
    {
        return errCode;
    }

    gcmASSERT(VIR_Symbol_isInput(origSym));

    /* Create a new uniform. */
    errCode = VIR_Shader_AddString(pShader,
                                   (shaderKind == VIR_SHADER_TESSELLATION_CONTROL) ?
                                        "#TcsPatchVerticesIn" : "#TesPatchVerticesIn",
                                   &nameId);
    ON_ERROR(errCode, "Add name string.");
    gcmASSERT(nameId != VIR_INVALID_ID);

    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_UNIFORM,
                                   nameId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
                                   VIR_STORAGE_GLOBAL,
                                   &uniformSymId);
    ON_ERROR(errCode, "Add uniform symbol.");
    gcmASSERT(uniformSymId != VIR_INVALID_ID);

    uniformSym = VIR_Shader_GetSymFromId(pShader, uniformSymId);
    VIR_Symbol_SetFlag(uniformSym, VIR_SYMFLAG_ENABLED | VIR_SYMUNIFORMFLAG_COMPILETIME_INITIALIZED);
    VIR_Symbol_SetAddrSpace(uniformSym, VIR_AS_CONSTANT);
    VIR_Symbol_SetTyQualifier(uniformSym, VIR_TYQUAL_CONST);
    VIR_Symbol_SetPrecision(uniformSym, VIR_PRECISION_HIGH);
    VIR_Symbol_SetUniformKind(uniformSym, VIR_UNIFORM_NORMAL);

    /* Set the constant from layout. */
    memset(&constVal, 0, sizeof(VIR_ConstVal));
    constVal.scalarVal.iValue = (shaderKind == VIR_SHADER_TESSELLATION_CONTROL) ?
        pShader->shaderLayout.tcs.tcsPatchInputVertices : pShader->shaderLayout.tes.tessPatchInputVertices;
    errCode = VIR_Shader_AddConstant(pShader,
                                     VIR_TYPE_INT32,
                                     &constVal,
                                     &constId);
    ON_ERROR(errCode, "Add constant.");

    VIR_Uniform_SetInitializer(VIR_Symbol_GetUniform(uniformSym), constId);

    /* Replace the original input symbol with the new uniform. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*       pFunc = func_node->function;
        VIR_Instruction*    pInst;
        VIR_InstIterator    instIter;
        gctUINT             i;

        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = VIR_InstIterator_First(&instIter);

        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
            {
                errCode = VIR_Operand_ReplaceSymbol(pShader,
                                                    pFunc,
                                                    VIR_Inst_GetSource(pInst, i),
                                                    origSym,
                                                    uniformSym);
                ON_ERROR(errCode, "Replace symbol");
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_GenCombinedSamplerOpnd(
    IN VIR_Shader       *pShader,
    IN VIR_Instruction  *pInst,
    IN VIR_Operand      *pOpnd
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Symbol  *pSymbol = VIR_Operand_GetSymbol(pOpnd);
    VIR_Uniform *pUniform = gcvNULL;
    VIR_Id      id ;
    VIR_Symbol  *uniformSym = gcvNULL, *origSym = gcvNULL;
    gctUINT i;

    if (VIR_Operand_GetOpKind(pOpnd) != VIR_OPND_SYMBOL ||
        !isSymCombinedSampler(pSymbol))
    {
        return errCode;
    }

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); i ++)
    {
        id  = VIR_IdList_GetId(&pShader->uniforms, i);
        uniformSym = VIR_Shader_GetSymFromId(pShader, id);

        if (VIR_Symbol_GetUniformKind(uniformSym) == VIR_UNIFORM_SAMPLED_IMAGE)
        {
            if (VIR_Symbol_GetSeparateImage(uniformSym) == VIR_Symbol_GetSeparateImage(pSymbol) &&
                VIR_Symbol_GetImgIdxRange(uniformSym) == VIR_Symbol_GetImgIdxRange(pSymbol) &&
                VIR_Symbol_GetSeparateSampler(uniformSym) == VIR_Symbol_GetSeparateSampler(pSymbol) &&
                VIR_Symbol_GetSamplerIdxRange(uniformSym) == VIR_Symbol_GetSamplerIdxRange(pSymbol))
            {
                pUniform = VIR_Symbol_GetSampler(uniformSym);
                break;
            }
        }
    }

    /* if not found, create one */
    if (pUniform == gcvNULL)
    {
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_SAMPLER,
                                       VIR_Symbol_GetName(pSymbol),
                                       VIR_Symbol_GetType(pSymbol),
                                       VIR_STORAGE_UNKNOWN,
                                       &id);

        uniformSym = VIR_Shader_GetSymFromId(pShader, id);
        VIR_Symbol_SetFlag(uniformSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER |
                                       VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetLocation(uniformSym, -1);

        VIR_Symbol_SetSeparateSamplerId(uniformSym, VIR_Symbol_GetSeparateSampler(pSymbol));
        VIR_Symbol_SetSamplerIdxRange(uniformSym, VIR_Symbol_GetSamplerIdxRange(pSymbol));
        VIR_Symbol_SetSeparateImageId(uniformSym, VIR_Symbol_GetSeparateImage(pSymbol));
        VIR_Symbol_SetImgIdxRange(uniformSym, VIR_Symbol_GetImgIdxRange(pSymbol));
        VIR_Symbol_SetUniformKind(uniformSym, VIR_UNIFORM_SAMPLED_IMAGE);

        /* clear the flag */
        origSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetSeparateSampler(pSymbol));
        VIR_Symbol_ClrFlag(origSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        origSym = VIR_Shader_GetSymFromId(pShader, VIR_Symbol_GetSeparateImage(pSymbol));
        VIR_Symbol_ClrFlag(origSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);

        pUniform = VIR_Symbol_GetSampler(uniformSym);
        pUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)) - 1;
    }

    VIR_Operand_SetSym(pOpnd, VIR_Shader_GetSymFromId(pShader, pUniform->sym));

    return errCode;
}

VSC_ErrCode
_GenCombinedSampler(IN OUT VIR_Shader* pShader)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function     *func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction  *inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SrcOperand_Iterator srcOpndIter;
            VIR_Operand*            srcOpnd;

            VIR_SrcOperand_Iterator_Init(inst, &srcOpndIter);
            srcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; srcOpnd != gcvNULL; srcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                if (VIR_Operand_isParameters(srcOpnd))
                {
                    VIR_ParmPassing *parm = VIR_Operand_GetParameters(srcOpnd);
                    gctUINT i;

                    for (i = 0; i < parm->argNum; i++)
                    {
                        _GenCombinedSamplerOpnd(pShader, inst, parm->args[i]);
                    }

                }
                else
                {
                    _GenCombinedSamplerOpnd(pShader, inst, srcOpnd);
                }
            }
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Generating combined sampler", pShader, gcvTRUE);
    }

    return errCode;
}

VSC_ErrCode _ConvertRetToJmpForFunction(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Function* pFunc
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_Instruction*   pInst;
    VIR_Instruction*   pNewInst;
    VIR_InstIterator   instIter;
    VIR_Instruction*   pInstLabel = gcvNULL;
    VIR_LabelId        labelId = VIR_INVALID_ID;
    VIR_Label*         pLabel = gcvNULL;
    VIR_Link*          pLink;
    gctBOOL            isMainFunc = (VIR_Function_GetFlags(pFunc) & VIR_FUNCFLAG_MAIN);

    if (VIR_Function_GetInstCount(pFunc) == 0)
    {
        return errCode;
    }

    /* Make sure that the last instruction of this function is a RET. */
    pInst = VIR_Function_GetInstEnd(pFunc);
    if (VIR_Inst_GetOpcode(pInst) != VIR_OP_RET)
    {
        errCode = VIR_Function_AddInstructionAfter(pFunc,
                                                   VIR_OP_RET,
                                                   VIR_TYPE_VOID,
                                                   pInst,
                                                   gcvTRUE,
                                                   &pNewInst);
        ON_ERROR(errCode, "VIR_Function_AddInstructionAfter");
    }

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = VIR_InstIterator_First(&instIter);

    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_RET)
        {
            if (pFunc->instList.pTail == pInst)
            {
                if (isMainFunc)
                {
                    errCode = VIR_Function_DeleteInstruction(pFunc, pInst);
                    ON_ERROR(errCode, "VIR_Function_DeleteInstruction");
                }
                break;
            }
            else
            {
                /* Create a label for the tail instruction of this function. */
                if (pLabel == gcvNULL)
                {
                    VIR_Function_AddLabel(pFunc, "#sh_FuncEnd", &labelId);

                    errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                                VIR_OP_LABEL,
                                                                VIR_TYPE_UNKNOWN,
                                                                pFunc->instList.pTail,
                                                                gcvTRUE,
                                                                &pInstLabel);
                    ON_ERROR(errCode, "VIR_Function_AddInstructionBefore");
                    pLabel = VIR_GetLabelFromId(pFunc, labelId);
                    pLabel->defined = pInstLabel;
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(pInstLabel), pLabel);
                }

                /* Insert a JMP to the tail instruction label. */
                errCode = VIR_Function_AddInstructionAfter(pFunc,
                                                           VIR_OP_JMP,
                                                           VIR_TYPE_VOID,
                                                           pInst,
                                                           gcvTRUE,
                                                           &pNewInst);
                ON_ERROR(errCode, "VIR_Function_AddInstructionAfter");
                VIR_Operand_SetLabel(VIR_Inst_GetDest(pNewInst), pLabel);
                VIR_Function_NewLink(pFunc, &pLink);
                VIR_Link_SetReference(pLink, (gctUINTPTR_T)pNewInst);
                VIR_Link_AddLink(&(pLabel->referenced), pLink);

                /* Let iterator go on one step to pNewInst */
                pNewInst = VIR_InstIterator_Next(&instIter);

                /* Remove the RET instruction. */
                errCode = VIR_Function_DeleteInstruction(pFunc, pInst);
                ON_ERROR(errCode, "VIR_Function_DeleteInstruction");

                pInst = pNewInst;
            }
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode _ConvertRetToJmpForFunctions(
    IN OUT VIR_Shader* pShader
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_FuncIterator   func_iter;
    VIR_FunctionNode*  func_node;
    VIR_Function*      func;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        errCode = _ConvertRetToJmpForFunction(pShader,
                                              func);
        ON_ERROR(errCode, "convert ret to jmp");

    }

OnError:
    return errCode;
}

VSC_ErrCode _MergeConstantOffsetForArrayInst(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Instruction* pInst
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_Operand* pBaseOperand = VIR_Inst_GetSource(pInst, 0);
    VIR_Operand* pOffsetOperand = VIR_Inst_GetSource(pInst, 1);
    VIR_Instruction* pMOVA = VIR_Inst_GetPrev(pInst);
    VIR_Operand* pMOVADest;
    VIR_Operand* pMOVASrc;
    VIR_Instruction* pADD;
    VIR_Operand* pADDDest;
    VIR_Operand* pADDtSrc1;
    VIR_Symbol* pBaseOperandSym;
    VIR_SymId pBaseOperandSymId;
    gctUINT baseOperandSymVReg;

    gcmASSERT(VIR_Inst_GetOpcode(pInst) == VIR_OP_LDARR || VIR_Inst_GetOpcode(pInst) == VIR_OP_STARR);

    pBaseOperandSym = VIR_Operand_GetSymbol(pBaseOperand);
    if(!VIR_Symbol_isVreg(pBaseOperandSym))
    {
        return errCode;
    }

    if(pMOVA == gcvNULL || VIR_Inst_GetOpcode(pMOVA) != VIR_OP_MOVA)
    {
        return errCode;
    }

    pMOVADest = VIR_Inst_GetDest(pMOVA);
    pMOVASrc = VIR_Inst_GetSource(pMOVA, 0);
    pADD = VIR_Inst_GetPrev(pMOVA);

    if(pADD == gcvNULL || VIR_Inst_GetOpcode(pADD) != VIR_OP_ADD)
    {
        return errCode;
    }

    pADDDest = VIR_Inst_GetDest(pADD);
    pADDtSrc1 = VIR_Inst_GetSource(pADD, 1);

    if(!VIR_Operand_Identical(pMOVADest, pOffsetOperand, pShader) || !VIR_Operand_Identical(pADDDest, pMOVASrc, pShader))
    {
        return errCode;
    }

    if(!VIR_Operand_isImm(pADDtSrc1))
    {
        return errCode;
    }

    baseOperandSymVReg = VIR_Symbol_GetVregIndex(pBaseOperandSym);
    baseOperandSymVReg += VIR_Operand_GetImmediateUint(pADDtSrc1);
    VIR_Shader_GetVirRegSymByVirRegId(pShader, baseOperandSymVReg, &pBaseOperandSymId);
    pBaseOperandSym = VIR_Shader_GetSymFromId(pShader, pBaseOperandSymId);
    VIR_Operand_SetSym(pBaseOperand, pBaseOperandSym);
    VIR_Operand_SetImmediateUint(pADDtSrc1, 0);

    return errCode;
}

VSC_ErrCode _MergeConstantOffsetForFunction(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Function* pFunc
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_Instruction*   pInst;
    VIR_InstIterator   instIter;

    if (VIR_Function_GetInstCount(pFunc) == 0)
    {
        return errCode;
    }

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = VIR_InstIterator_First(&instIter);

    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(pInst);

        if(opcode == VIR_OP_LDARR || opcode == VIR_OP_STARR)
        {
            errCode = _MergeConstantOffsetForArrayInst(pShader, pInst);
            ON_ERROR(errCode, "merge constant offset");
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode _MergeConstantOffset(
    IN OUT VIR_Shader* pShader
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_FuncIterator   func_iter;
    VIR_FunctionNode*  func_node;
    VIR_Function*      func;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        errCode = _MergeConstantOffsetForFunction(pShader,
                                              func);
        ON_ERROR(errCode, "merge constant offset");

    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Merging constant offset", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}

gctBOOL _CheckAlwaysInlineFunction(
    IN OUT VIR_Shader      *pShader,
    IN  VSC_HW_CONFIG      *pHwCfg,
    IN OUT VIR_Function    *pFunc
    )
{
    gctBOOL                 canSrc0OfImgRelatedBeTemp = pHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp;
    VIR_Instruction        *pInst;
    VIR_InstIterator        instIter;

    if (VIR_Function_GetInstCount(pFunc) == 0)
    {
        return gcvFALSE;
    }

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = VIR_InstIterator_First(&instIter);
    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        VIR_OpCode          opcode = VIR_Inst_GetOpcode(pInst);
        VIR_Operand        *pOperand = gcvNULL;
        VIR_Symbol         *pOpndSym = gcvNULL;

        /* Check if use a temp reg for a src0 of a image-related instructions. */
        if (VIR_OPCODE_isImgRelated(opcode))
        {
            gctBOOL         isSrc0Temp = gcvFALSE;
            pOperand = VIR_Inst_GetSource(pInst, 0);

            if (VIR_Operand_isVirReg(pOperand))
            {
                isSrc0Temp = gcvTRUE;
            }
            else if (VIR_Operand_isSymbol(pOperand))
            {
                pOpndSym = VIR_Operand_GetSymbol(pOperand);

                if (!VIR_Symbol_isImage(pOpndSym) &&
                    !VIR_Symbol_isUniform(pOpndSym))
                {
                    isSrc0Temp = gcvTRUE;
                }
            }
            else
            {
                gcmASSERT(gcvFALSE);
            }

            if (isSrc0Temp && !canSrc0OfImgRelatedBeTemp)
            {
                return gcvTRUE;
            }
        }
        /*
        ** If try to get info from a sampler, since HW can't support those instruction directly,
        ** we need to inline this function to get the exact sampler.
        */
        else if (VIR_OPCODE_isGetSamplerInfo(opcode))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

VSC_ErrCode _MarkFunctionAndAllCallerFunctions(
    IN OUT VIR_Shader      *pShader,
    IN  VIR_Function       *pFunc
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_FUNC_BLOCK             *pFuncBlk;
    VSC_ADJACENT_LIST_ITERATOR  edgeIter;
    VIR_CG_EDGE                *pEdge;

    /* Skip main function. */
    if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_MAIN))
    {
        return errCode;
    }

    /* Mark function itself. */
    VIR_Function_SetFlag(pFunc, VIR_FUNCFLAG_ALWAYSINLINE);

    /* Mark all its caller functions. */
    pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
    pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
        errCode = _MarkFunctionAndAllCallerFunctions(pShader, callerBlk->pVIRFunc);
        ON_ERROR(errCode, "mark function and all its caller functions. ");
    }

OnError:
    return errCode;
}

VSC_ErrCode _CheckAlwaysInlineFunctions(
    IN OUT VIR_Shader      *pShader,
    IN  VSC_HW_CONFIG      *pHwCfg
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode           *func_node;
    VIR_Function               *pFunc;
    gctBOOL                     alwaysInline = gcvFALSE;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        alwaysInline = gcvFALSE;
        pFunc = func_node->function;

        /* Skip main function. */
        if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_MAIN))
        {
            continue;
        }

        /* If this function is set before, then it means all its caller have been marked too. */
        if (VIR_Function_HasFlag(pFunc, VIR_FUNCFLAG_ALWAYSINLINE))
        {
            continue;
        }

        alwaysInline = _CheckAlwaysInlineFunction(pShader, pHwCfg, pFunc);

        /* Mark this function and all its caller functions. */
        if (alwaysInline)
        {
            errCode = _MarkFunctionAndAllCallerFunctions(pShader, pFunc);
            ON_ERROR(errCode, "mark function and all its caller functions.");
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode _ConvertScalarVectorConstToImm(
    IN OUT VIR_Shader* pShader
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_FuncIterator   func_iter;
    VIR_FunctionNode*  func_node;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*       pFunc = func_node->function;
        VIR_InstIterator    instIter;
        VIR_Instruction*    pInst;

        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = VIR_InstIterator_First(&instIter);

        for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
        {
            gctUINT         i;
            VIR_Operand*    pSrcOpnd;

            for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
            {
                pSrcOpnd = VIR_Inst_GetSource(pInst, i);

                if (VIR_Operand_isConst(pSrcOpnd) &&
                    VIR_Type_isScalar(VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetType(pSrcOpnd))))
                {
                    VIR_ScalarConstVal  scalarConstVal;
                    VIR_Const*          srcConst = VIR_Shader_GetConstFromId(pShader, VIR_Operand_GetConstId(pSrcOpnd));

                    if (VIR_Type_isScalar(VIR_Shader_GetTypeFromId(pShader, srcConst->type)))
                    {
                        scalarConstVal = srcConst->value.scalarVal;
                    }
                    else
                    {
                        gctUINT channel = VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(pSrcOpnd), 1);
                        scalarConstVal.fValue = srcConst->value.vecVal.f32Value[channel];
                    }

                    VIR_Operand_SetImmediate(pSrcOpnd, VIR_Operand_GetType(pSrcOpnd), scalarConstVal);
                }
            }
        }

    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PreprocessLLShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passFlag.resCreationReq.s.bNeedCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
}

VSC_ErrCode vscVIR_PreprocessLLShader(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader* pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG* pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    /* Convert integer to float, if hardware doesn't support integer */
    if (!pHwCfg->hwFeatureFlags.hasHalti0)
    {
        errCode = _ConvertIntegerToFloat(pShader,
                                         pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportInteger,
                                         pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags. hasSignFloorCeil);
        ON_ERROR(errCode, "Convert int to float");
    }

    /* Generate a provide uniform instead of temp register for PatchVerticesIn, if hardware doesn't support it. */
    if (!pHwCfg->hwFeatureFlags.supportPatchVerticesIn)
    {
        errCode = _ConvertPatchVerticesInToUniform(pShader);
        ON_ERROR(errCode, "Convert PatchVerticesIn to uniform");
    }

    /* Update Precision */
    errCode = _PrecisionUpdate(pShader, pPassWorker->basePassWorker.pDumper);
    ON_ERROR(errCode, "precision update");

    /* Generate combined sampler for separated samper and separated texture */
    errCode = _GenCombinedSampler(pShader);
    ON_ERROR(errCode, "Gen combined sampler");

    /* Change all RETs to JMPs and only keep one RET at the end of the function. */
    errCode = _ConvertRetToJmpForFunctions(pShader);
    ON_ERROR(errCode, "Convert RET to JMP for functions");

    /* Disable this opt, since it does not consider DU. For example,
       add t0, t1, c
       mova a0, t0
       ldarr t2, t3, a0
       ...
       add t4, t0, t5
       ==>
       add t0, t1, 0 ==> also, we need constant folding to change this to MOV to further optimize
       mova a0, t0
       ldarr t2, (t3 + c), a0
       ...
       add t4, t0, t5 ==> t0 is changed !!!
       TO-DO: since vulkan needs this, we enable it for vulkan only for now.
       Need better solution.
    */
    if (VIR_Shader_GetClientApiVersion(pShader) == gcvAPI_OPENVK)
    {
        errCode = _MergeConstantOffset(pShader);
        ON_ERROR(errCode, "Merge constant offset");
    }

    /* Check if a function is ALWAYSINLINE. */
    errCode = _CheckAlwaysInlineFunctions(pShader, pHwCfg);
    ON_ERROR(errCode, "Check always inline functions");

    /* Change all scalar vector constant source into immediate. */
    errCode = _ConvertScalarVectorConstToImm(pShader);
    ON_ERROR(errCode, "Convert Scalar vector constant to imm");

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After preprocess LL", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}

static void _SortAttributesOfDual16Shader(VIR_Shader *pShader, VSC_HW_CONFIG *pHwCfg)
{
    VIR_AttributeIdList     *pAttrs = VIR_Shader_GetAttributes(pShader);
    gctUINT                 attrCount = VIR_IdList_Count(pAttrs);
    gctUINT                 i, j, tempIdx;

    /* in dual16, we need to put all HP attributes in front of MP attributes,
       r0/r1: highp position
       highp varying:
        1) all vec3 and vec4 at 2 register each
        2) all vec1 and vec2 at 1 register
       mediump varying: 1 register each
    */

    /* put all highp to the front */
    for (i = 0; i < attrCount; i ++)
    {
        VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(
                           pShader, VIR_IdList_GetId(pAttrs, i));

        if (VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH)
        {
            continue;
        }
        else
        {
            for (j = i + 1; j < attrCount; j ++)
            {
                VIR_Symbol  *nextAttribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, j));

                if (VIR_Symbol_GetPrecision(nextAttribute) == VIR_PRECISION_HIGH)
                {
                    tempIdx = VIR_IdList_GetId(pAttrs, j);
                    VIR_IdList_SetId(pAttrs, j, VIR_IdList_GetId(pAttrs, i));
                    VIR_IdList_SetId(pAttrs, i, tempIdx);
                    break;
                }
            }
        }
    }

    if (pHwCfg->hwFeatureFlags.highpVaryingShift)
    {
        /* put all highp vec3/vec4 to the front */
        for (i = 0; i < attrCount; i ++)
        {
            VIR_Symbol  *attribute = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pAttrs, i));

            if (VIR_Symbol_GetPrecision(attribute) == VIR_PRECISION_HIGH &&
                VIR_Symbol_GetComponents(attribute) > 2)
            {
                continue;
            }
            else
            {
                for (j = i + 1; j < attrCount; j ++)
                {
                    VIR_Symbol  *nextAttribute = VIR_Shader_GetSymFromId(
                        pShader, VIR_IdList_GetId(pAttrs, j));

                    if (VIR_Symbol_GetPrecision(nextAttribute) == VIR_PRECISION_HIGH &&
                        VIR_Symbol_GetComponents(nextAttribute) > 2)
                    {
                        tempIdx = VIR_IdList_GetId(pAttrs, j);
                        VIR_IdList_SetId(pAttrs, j, VIR_IdList_GetId(pAttrs, i));
                        VIR_IdList_SetId(pAttrs, i, tempIdx);
                        break;
                    }
                }
            }
        }
    }
}

static gctBOOL
VIR_Inst_Dual16NotSupported(
    IN VIR_Instruction *  pInst
    )
{
    VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);

    if (VIR_OPCODE_isCall(opcode) || opcode == VIR_OP_RET ||
        opcode == VIR_OP_LOOP || opcode == VIR_OP_ENDLOOP ||
        opcode == VIR_OP_REP || opcode == VIR_OP_ENDREP)
    {
        return gcvTRUE;
    }
    return gcvFALSE;
}

DEF_QUERY_PASS_PROP(VIR_Shader_CheckDual16able)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_DUAL16;
}

VSC_ErrCode VIR_Shader_CheckDual16able(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader              *Shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_COMPILER_CONFIG     *compCfg = &pPassWorker->pCompilerParam->cfg;
    VSC_OPTN_DUAL16Options  *options = (VSC_OPTN_DUAL16Options*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper              *dumper = pPassWorker->basePassWorker.pDumper;
    gctUINT                 dual16Mode = gcGetDualFP16Mode(compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2);
    gctBOOL                 hasHalfDepFix = compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalfDepFix;
    gctUINT                 i = 0;
    VIR_Symbol              *pSym = gcvNULL;

    VIR_Shader_SetDual16Mode(Shader, gcvFALSE);

    /* only fragment shader can be dual16 shader,
    ** and exclude OpenVG shader due to precision issue
    */
    if (!compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportDual16 ||
        (VIR_Shader_GetKind(Shader) != VIR_SHADER_FRAGMENT)       ||
        VIR_Shader_GetClientApiVersion(Shader) == gcvAPI_OPENVK   ||
        VIR_Shader_IsOpenVG(Shader)                               ||
        VIR_Shader_Has32BitModulus(Shader)                        || /* gcsl expand the integer mod and div using 32bit way
                                                                        (_gcConvert32BitModulus) */
        VIR_Shader_HasOutputArrayHighp(Shader)                    || /* currently, we disable dual16 if the output array is highp */
        !VirSHADER_DoDual16(VIR_Shader_GetId(Shader))             ||
        gcmOPT_EnableDebug()                                         /* we disable dual16 for debugging mode */
        )
    {
        return errCode;
    }

    /* TODO: Need move this option to driver side */
    if (dual16Mode == DUAL16_AUTO_BENCH)
    {
        /* Enable dual16 auto-on mode for following games. */
        switch (compCfg->ctx.appNameId)
        {
        case gcvPATCH_GLBM21:
        case gcvPATCH_GLBM25:
        case gcvPATCH_GLBM27:
        case gcvPATCH_GFXBENCH:
        case gcvPATCH_MM07:
        case gcvPATCH_NENAMARK2:
        case gcvPATCH_LEANBACK:
        case gcvPATCH_ANGRYBIRDS:
            break;
        default:
            return errCode;
        }
    }

    /* dual16 does not support attribute components larger than 60 */
    {
        gctUINT attrCount = 0, highpAttrCount = 0, totalAttrCount = 0;
        gctUINT components = 0;
        for (i = 0; i < VIR_IdList_Count(VIR_Shader_GetAttributes(Shader)); i++)
        {
            VIR_Type        *symType = gcvNULL;

            pSym = VIR_Shader_GetSymFromId(Shader, VIR_IdList_GetId(VIR_Shader_GetAttributes(Shader), i));

            /* Only consider used one */
            if (isSymUnused(pSym) || isSymVectorizedOut(pSym))
            {
                continue;
            }

            symType = VIR_Symbol_GetType(pSym);
            components = VIR_Symbol_GetComponents(pSym);
            attrCount += VIR_Type_GetVirRegCount(Shader, symType, -1) * components;
            if (VIR_Symbol_GetPrecision(pSym) == VIR_PRECISION_HIGH)
            {
                highpAttrCount += VIR_Type_GetVirRegCount(Shader, symType, -1) * components;
            }
        }

        /* this formula is coming from cmodel */
        totalAttrCount = ((2 + 4 + attrCount + 3) >> 2) + ((2 + 4 + highpAttrCount + 3) >> 2);
        if (totalAttrCount > 15)
        {
            if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
            {
                VIR_LOG(dumper, "bails dual16 because shader(%d)'s attribute components exceed 60.\n", VIR_Shader_GetId(Shader));
                VIR_LOG_FLUSH(dumper);
            }
            return errCode;
        }
    }

    /* check the shader code */
    {
        VIR_ShaderCodeInfo codeInfo;
        VIR_FunctionNode*  pFuncNode;
        VIR_FuncIterator   funcIter;
        VIR_Instruction*   pInst;
        VIR_InstIterator   instIter;
        gctBOOL            dual16NotSupported  = gcvFALSE;
        gctINT             runSignleTInstCount = 0;
        gctINT             halfDepInst = 0;

        if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_INPUT))
        {
            VIR_Shader_Dump(gcvNULL, "before dual16 shader check.", Shader, gcvTRUE);
        }

        gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(codeInfo));
        /* Calculate HW inst count */
        VIR_FuncIterator_Init(&funcIter, &Shader->functions);
        pFuncNode = VIR_FuncIterator_First(&funcIter);

        for(; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
        {
            VIR_Function *pFunc = pFuncNode->function;

            VIR_InstIterator_Init(&instIter, &pFunc->instList);
            pInst = VIR_InstIterator_First(&instIter);

            for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
            {
                VIR_OpCode  opcode  =  VIR_Inst_GetOpcode(pInst);
                gctBOOL     needRunSingleT = gcvFALSE;

                if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
                {
                    VIR_Inst_Dump(dumper, pInst);
                }

                /* check dest */
                if (VIR_Inst_Dual16NotSupported(pInst))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
                    {
                        VIR_LOG(dumper, "inst not supported by dual16.\n", i);
                        VIR_LOG_FLUSH(dumper);
                    }
                    dual16NotSupported = gcvTRUE;
                    break;
                }

                VIR_Inst_Check4Dual16(pInst, &needRunSingleT, &dual16NotSupported, options, dumper);
                if(dual16NotSupported && VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
                {
                    VIR_LOG(dumper, "inst not supported by dual16.\n", i);
                    VIR_LOG_FLUSH(dumper);
                }
                gcmASSERT(VIR_OP_MAXOPCODE > opcode);
                codeInfo.codeCounter[(gctINT)opcode]++;
                if (opcode != VIR_OP_LABEL /* exclude non executive IRs */)
                {
                    codeInfo.estimatedInst++;
                }
                if (needRunSingleT)
                {
                    runSignleTInstCount++;
                    VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_32);

                    /* VIV when hw has no half-dep fix, there is false depedence among
                       texld.t0, r2, s0, r0.hp
                       texld.t1, r2, s0, r1.hp,
                       the performance will be bad */
                    if (!hasHalfDepFix &&
                        (VIR_OPCODE_isTexLd(opcode) ||
                         VIR_OPCODE_isMemLd(opcode) ||
                         VIR_OPCODE_isImgLd(opcode)) &&
                        VIR_Operand_GetPrecision(VIR_Inst_GetDest(pInst)) != VIR_PRECISION_HIGH)
                    {
                        halfDepInst++;
                    }
                }
            }
        }

        codeInfo.hasUnsupportDual16Inst = dual16NotSupported;
        codeInfo.runSignleTInstCount    = runSignleTInstCount;
        codeInfo.halfDepInstCount = halfDepInst;

        if (codeInfo.hasUnsupportDual16Inst     ||
            codeInfo.useInstanceID              ||
            codeInfo.useVertexID                ||
            codeInfo.destUseLocalStorage        ||
            (codeInfo.estimatedInst + codeInfo.runSignleTInstCount) > 1023 ||
            (dual16Mode < DUAL16_FORCE_ON &&
             !Shader->__IsMasterDual16Shader &&     /*in recompilation, if the master shader is dual16, force the shader to be dual16 if possible */
             (codeInfo.runSignleTInstCount > VSC_OPTN_DUAL16Options_GetPercentage(options) * codeInfo.estimatedInst ||
              codeInfo.halfDepInstCount > VSC_OPTN_DUAL16Options_GetHalfDepPercentage(options) * codeInfo.estimatedInst))
           )
        {
            if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
            {
                VIR_LOG(dumper, "dual16 not supported.\n", i);
                VIR_LOG_FLUSH(dumper);
            }
            return errCode;
        }
    }

    if(VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(Shader), VIR_Shader_GetId(Shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE) ||
       VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_OUTPUT))
    {
        VIR_Shader_Dump(gcvNULL, "After dual16 shader transform.", Shader, gcvTRUE);
    }

    /* In dual16, we need to put all HP attributes in front of MP attributes */
    _SortAttributesOfDual16Shader(Shader, &compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg);

    VIR_Shader_SetDual16Mode(Shader, gcvTRUE);

    return errCode;
}

static VSC_ErrCode
_CheckOperandForVarUsage(
    IN  VIR_Shader              *pShader,
    IN  VIR_Instruction         *pInst,
    IN  VIR_CHECK_VAR_USAGE     *CheckVarUsage,
    IN  VIR_Operand             *pOpnd
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_OperandInfo             opndInfo;
    VIR_Symbol                 *opndSym;
    VIR_VirRegId                regId;
    gctUINT                     i;

    if (pOpnd == gcvNULL)
        return errCode;

    /* Check texld parm if needed. */
    if (VIR_Operand_isTexldParm(pOpnd))
    {
        VIR_Operand *texldOperand = (VIR_Operand*)pOpnd;

        for (i = 0; i < VIR_TEXLDMODIFIER_COUNT; ++i)
        {
            _CheckOperandForVarUsage(pShader, pInst, CheckVarUsage,
                                     VIR_Operand_GetTexldModifier(texldOperand, i));
        }
    }

    /* Check operand ifself. */
    VIR_Operand_GetOperandInfo(pInst, pOpnd, &opndInfo);

    if ((opndInfo.isInput && (CheckVarUsage->checkInput || CheckVarUsage->checkPrePatchInput))
        ||
        (opndInfo.isOutput && (CheckVarUsage->checkOutput || CheckVarUsage->checkPrePatchOutput)))
    {
        gcmASSERT(opndInfo.isVreg);
        regId = opndInfo.u1.virRegInfo.virRegWithOffset;

        if (VIR_Operand_GetIsConstIndexing(pOpnd))
        {
            regId += VIR_Operand_GetConstIndexingImmed(pOpnd);
        }

        /* Clear the UNUSED flag. */
        opndSym = VIR_Shader_FindSymbolByTempIndex(pShader, regId);
        opndSym = VIR_Symbol_GetVregVariable(opndSym);
        /* it is possible that the underlying symbol is NULL, like invocationIndex and workgroupIndex,
           since they are replaced by invocationID and workgroupID */
        if (opndSym)
        {
            VIR_Symbol_ClrFlag(opndSym, VIR_SYMFLAG_UNUSED);
        }
    }

    return errCode;
}

static void
_InitUsageFlag(
    IN  VIR_Shader              *pShader,
    IN  VIR_IdList              *IdList,
    IN  VIR_SymFlag              Flag,
    IN  gctBOOL                  SetFlag
    )
{
    VIR_Symbol                  *sym;
    gctUINT                      i;

    for (i = 0; i < VIR_IdList_Count(IdList); i++)
    {
        sym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(IdList, i));

        if (SetFlag)
        {
            VIR_Symbol_SetFlag(sym, Flag);
        }
        else
        {
            VIR_Symbol_ClrFlag(sym, Flag);
        }
    }
}

DEF_QUERY_PASS_PROP(vscVIR_CheckVariableUsage)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
}

VSC_ErrCode vscVIR_CheckVariableUsage(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader                 *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_CHECK_VAR_USAGE        *checkVarUsage = (VIR_CHECK_VAR_USAGE *)pPassWorker->basePassWorker.pPrvData;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode           *func_node;

    /* Clean up the flag first. */
    if (checkVarUsage->checkInput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetAttributes(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE);
    }
    if (checkVarUsage->checkOutput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetOutputs(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE);
    }
    if (checkVarUsage->checkPrePatchInput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetPerpatchAttributes(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE);
    }
    if (checkVarUsage->checkPrePatchOutput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetPerpatchOutputs(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE);
    }
    if (checkVarUsage->checkUniform)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetUniforms(pShader), VIR_SYMUNIFORMFLAG_USED_IN_SHADER | VIR_SYMUNIFORMFLAG_USED_IN_LTC, gcvFALSE);
    }

    /* Process all instructions. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function           *func = func_node->function;
        VIR_InstIterator        inst_iter;
        VIR_Instruction        *inst;
        gctUINT                 i;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            errCode = _CheckOperandForVarUsage(pShader, inst, checkVarUsage, VIR_Inst_GetDest(inst));
            CHECK_ERROR(errCode, "_CheckOperandForVarUsage failed.");

            for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
            {
                errCode = _CheckOperandForVarUsage(pShader, inst, checkVarUsage, VIR_Inst_GetSource(inst, i));
                CHECK_ERROR(errCode, "_CheckOperandForVarUsage failed.");
            }
        }
    }

    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_GEOMETRY)
    {
        VIR_Symbol  *outputSym = gcvNULL;
        VIR_SymId    symId;

        /* if last output is not used, then safely remove it */
        symId = VIR_IdList_GetId(VIR_Shader_GetOutputs(pShader),
                    VIR_IdList_Count(VIR_Shader_GetOutputs(pShader)) - 1);
        outputSym = VIR_Shader_GetSymFromId(pShader, symId);

        if (isSymUnused(outputSym))
        {
            VIR_IdList_DeleteByIndex(VIR_Shader_GetOutputs(pShader),
                VIR_IdList_Count(VIR_Shader_GetOutputs(pShader)) - 1);
        }
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_InitializeVariables)
{
    /* Will put it to HL when DFA can fully support on HL */
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

VSC_ErrCode vscVIR_InitializeVariables(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader*                 pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*         pDuInfo = pPassWorker->pDuInfo;
    VSC_BLOCK_TABLE*            pUsageTable = &pDuInfo->usageTable;
    VIR_USAGE*                  pUsage;
    VIR_DEF*                    pDef;
    gctUINT                     i, usageCount, usageIdx, defIdx;
    VIR_OperandInfo             operandInfo, operandInfo1;
    gctUINT                     firstRegNo = 0, regNoRange = 0;
    VIR_Enable                  defEnableMask = 0;
    gctUINT8                    halfChannelMask = 0;
    VIR_Instruction*            pNewMovInst;
    VIR_Symbol*                 pSym = gcvNULL;
    gctBOOL                     bNeedInitialization;
    VIR_Operand *               opnd;

    VIR_Shader_RenumberInstId(pShader);

    usageCount = BT_GET_MAX_VALID_ID(&pDuInfo->usageTable);
    for (usageIdx = 0; usageIdx < usageCount; usageIdx ++)
    {
        pUsage = GET_USAGE_BY_IDX(pUsageTable, usageIdx);
        if (IS_VALID_USAGE(pUsage))
        {
            /* If the usage has no def, we must initialize it */
            bNeedInitialization = (UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain) == 0);

            /* Even usage has defs, however, some of pathes might have no definition. So far,
               only loop is checked.

               TODO: need more complicated solution to check all pathes
            */
            if (!bNeedInitialization)
            {
                if (pUsage->usageKey.pUsageInst < VIR_OUTPUT_USAGE_INST)
                {
                    bNeedInitialization = gcvTRUE;

                    for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                    {
                        defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                        gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                        pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                        if (pDef->defKey.pDefInst == VIR_HW_SPECIAL_DEF_INST ||
                            pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
                        {
                            bNeedInitialization = gcvFALSE;
                            break;
                        }

                        if (VIR_Inst_GetFunction(pDef->defKey.pDefInst) != VIR_Inst_GetFunction(pUsage->usageKey.pUsageInst))
                        {
                            bNeedInitialization = gcvFALSE;
                            break;
                        }

                        if (pDef->defKey.pDefInst < VIR_OUTPUT_USAGE_INST)
                        {
                            if (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pUsage->usageKey.pUsageInst))
                            {
                                bNeedInitialization = gcvFALSE;
                                break;
                            }
                        }
                    }
                }
            }

            if (bNeedInitialization)
            {
                VIR_Operand_GetOperandInfo(pUsage->usageKey.pUsageInst,
                                           pUsage->usageKey.pOperand,
                                           &operandInfo);

                if (pUsage->usageKey.bIsIndexingRegUsage)
                {
                    gcmASSERT(operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO);

                    firstRegNo = operandInfo.indexingVirRegNo;
                    regNoRange = 1;
                    defEnableMask = (1 << operandInfo.componentOfIndexingVirRegNo);
                    halfChannelMask = (gctUINT8)operandInfo.halfChannelMaskOfIndexingVirRegNo;
                }
                else
                {
                    if (VIR_Inst_GetOpcode(pUsage->usageKey.pUsageInst) == VIR_OP_LDARR)
                    {
                        if (pUsage->usageKey.pOperand == VIR_Inst_GetSource(pUsage->usageKey.pUsageInst, VIR_Operand_Src0))
                        {
                            VIR_Operand_GetOperandInfo(pUsage->usageKey.pUsageInst,
                                                       VIR_Inst_GetSource(pUsage->usageKey.pUsageInst, VIR_Operand_Src1),
                                                       &operandInfo1);

                            if (operandInfo1.isImmVal)
                            {
                                firstRegNo = operandInfo.u1.virRegInfo.virReg + operandInfo1.u1.immValue.iValue;
                                regNoRange = 1;
                            }
                            else
                            {
                                firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
                                regNoRange = operandInfo.u1.virRegInfo.virRegCount;
                            }

                            defEnableMask = VIR_Operand_GetRealUsedChannels(pUsage->usageKey.pOperand,
                                                                            pUsage->usageKey.pUsageInst,
                                                                            gcvNULL);
                            halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
                        }
                        else if (pUsage->usageKey.pOperand == VIR_Inst_GetSource(pUsage->usageKey.pUsageInst, VIR_Operand_Src1))
                        {
                            firstRegNo = operandInfo.u1.virRegInfo.virReg;
                            regNoRange = 1;
                            defEnableMask = VIR_Operand_GetRealUsedChannels(pUsage->usageKey.pOperand,
                                                                            pUsage->usageKey.pUsageInst,
                                                                            gcvNULL);
                            halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
                        }
                        else
                        {
                            gcmASSERT(gcvFALSE);
                        }
                    }
                    else
                    {
                        if (operandInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
                        {
                            firstRegNo = operandInfo.u1.virRegInfo.startVirReg;
                            regNoRange = operandInfo.u1.virRegInfo.virRegCount;
                        }
                        else
                        {
                            firstRegNo = operandInfo.u1.virRegInfo.virReg;
                            regNoRange = 1;
                        }

                        defEnableMask = VIR_Operand_GetRealUsedChannels(pUsage->usageKey.pOperand,
                                                                        pUsage->usageKey.pUsageInst,
                                                                        gcvNULL);
                        halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
                    }

                    pSym = VIR_Operand_GetSymbol(pUsage->usageKey.pOperand);
                }

                /* Insert 'MOV reg, 0' at the begin of function */
                VIR_Function_PrependInstruction(VIR_Inst_GetFunction(pUsage->usageKey.pUsageInst),
                                                VIR_OP_MOV,
                                                VIR_Operand_GetType(pUsage->usageKey.pOperand),
                                                &pNewMovInst);

                /* Dst */
                opnd = VIR_Inst_GetDest(pNewMovInst);
                VIR_Operand_SetSymbol(opnd, VIR_Inst_GetFunction(pUsage->usageKey.pUsageInst), pSym->index);
                VIR_Operand_SetEnable(opnd, defEnableMask);
                VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pSym));

                /* Src */
                opnd = VIR_Inst_GetSource(pNewMovInst, VIR_Operand_Src0);
                if (VIR_GetTypeFlag(VIR_Operand_GetType(pUsage->usageKey.pOperand)) & VIR_TYFLAG_IS_UNSIGNED_INT ||
                    VIR_GetTypeFlag(VIR_Operand_GetType(pUsage->usageKey.pOperand)) & VIR_TYFLAG_IS_BOOLEAN)
                {
                    VIR_Operand_SetImmediateUint(opnd, 0);
                }
                else if (VIR_GetTypeFlag(VIR_Operand_GetType(pUsage->usageKey.pOperand)) & VIR_TYFLAG_IS_SIGNED_INT)
                {
                     VIR_Operand_SetImmediateInt(opnd, 0);
                }
                else
                {
                    VIR_Operand_SetImmediateFloat(opnd, 0);
                }
                VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pSym));

                vscVIR_AddNewDef(pDuInfo, pNewMovInst, firstRegNo, regNoRange,
                                 defEnableMask, halfChannelMask, gcvNULL, gcvNULL);

                vscVIR_AddNewUsageToDef(pDuInfo, pNewMovInst, pUsage->usageKey.pUsageInst, pUsage->usageKey.pOperand,
                                        pUsage->usageKey.bIsIndexingRegUsage, firstRegNo, regNoRange, defEnableMask,
                                        halfChannelMask, gcvNULL);
            }
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After variable initialization", pShader, gcvTRUE);
    }

    return errCode;
}


