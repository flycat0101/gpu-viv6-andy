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


#include "gc_vsc.h"
#include "vir/transform/gc_vsc_vir_misc_opts.h"
#include "vir/transform/gc_vsc_vir_loop.h"

DEF_QUERY_PASS_PROP(vscVIR_RemoveNop)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_PRE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_RemoveNop)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_RemoveNop(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    gctBOOL           bInvalidCfg = gcvFALSE;

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
                VIR_Pass_DeleteInstruction(func, inst, &bInvalidCfg);
            }

            inst = nextinst;
        }
    }

    if (bInvalidCfg)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = bInvalidCfg;
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_RecordInstructionStatus)
{
    /* We can call this pass in any level. */
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ALL;
}

DEF_SH_NECESSITY_CHECK(vscVIR_RecordInstructionStatus)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_RecordInstructionStatus(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_ShLevel             curShLevel = VIR_Shader_GetLevel(pShader);
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode*       pFunc_node;
    VIR_MemoryAccessFlag    memoryAccessFlag = VIR_MA_FLAG_NONE;
    VIR_FlowControlFlag     flowControlFlag = VIR_FC_FLAG_NONE;
    VIR_TexldFlag           texldFlag = VIR_TEXLD_FLAG_NONE;

    /* Now we only record them at these two levels. */
    gcmASSERT(curShLevel == VIR_SHLEVEL_Pre_Low || curShLevel == VIR_SHLEVEL_Post_Machine);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (pFunc_node = VIR_FuncIterator_First(&func_iter);
         pFunc_node != gcvNULL;
         pFunc_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    pFunc = pFunc_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* pInst;
        VIR_OpCode       opCode;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            opCode = VIR_Inst_GetOpcode(pInst);

            if (VIR_OPCODE_isMemLd(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_LOAD;
            }
            else if (VIR_OPCODE_isMemSt(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_STORE;
            }
            else if (VIR_OPCODE_isImgLd(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_IMG_READ;
            }
            else if (VIR_OPCODE_isImgSt(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_IMG_WRITE;
            }
            else if (VIR_OPCODE_isAtom(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_ATOMIC;
            }
            else if (VIR_OPCODE_isBarrier(opCode))
            {
                memoryAccessFlag |= VIR_MA_FLAG_BARRIER;
            }
            else if (opCode == VIR_OP_VX_ATOMICADD)
            {
                memoryAccessFlag |= VIR_MA_FLAG_EVIS_ATOMADD;
            }
            else if (VIR_OPCODE_isBranch(opCode))
            {
                flowControlFlag |= VIR_FC_FLAG_JMP;
            }
            else if (VIR_OPCODE_isCall(opCode))
            {
                flowControlFlag |= VIR_FC_FLAG_CALL;
            }
            else if (opCode == VIR_OP_KILL)
            {
                flowControlFlag |= VIR_FC_FLAG_KILL;
            }
            else if (VIR_OPCODE_isTexLd(opCode))
            {
                texldFlag |= VIR_TEXLD_FLAG_TEXLD;
            }
        }
    }

    /* Save the result. */
    pShader->memoryAccessFlag[curShLevel] = memoryAccessFlag;
    pShader->flowControlFlag[curShLevel] = flowControlFlag;
    pShader->texldFlag[curShLevel] = texldFlag;

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

DEF_SH_NECESSITY_CHECK(vscVIR_VX_ReplaceDest)
{
    return gcvTRUE;
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
            if (VIR_OPCODE_isVX(VIR_Inst_GetOpcode(inst)))
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

static gctBOOL _ChangePackedConstValue(VIR_Shader* pShader,
                                       VIR_Operand* pOpnd)
{
    gctBOOL         bChanged = gcvFALSE;
    VIR_TypeId      opndTypeId = VIR_Operand_GetTypeId(pOpnd);
    VIR_TypeId      elementTypeId = VIR_TYPE_UNKNOWN;
    gctUINT32       elementSizePerByte;
    gctUINT32       elementCountPerChannel;
    gctUINT32       componentSizePerByte = 4;
    gctUINT32       minElementBitCount = 8;

    /* Skip non-packed type. */
    if (!(VIR_TypeId_isPrimitive(opndTypeId) && VIR_TypeId_isPacked(opndTypeId)))
    {
        return bChanged;
    }

    elementTypeId = VIR_GetTypeComponentType(opndTypeId);
    elementSizePerByte = VIR_GetTypeSize(elementTypeId);
    elementCountPerChannel = componentSizePerByte / elementSizePerByte;

    if (VIR_Operand_isImm(pOpnd))
    {
        gctUINT32   newValue = 0, uValue = VIR_Operand_GetImmediateUint(pOpnd);
        gctUINT32   i;

        for (i = 0; i < elementCountPerChannel; i++)
        {
            switch (elementSizePerByte)
            {
            case 2:
                newValue |= ((uValue & 0xFFFF) << (elementSizePerByte * minElementBitCount * i));
                break;

            case 1:
                newValue |= ((uValue & 0xFF) << (elementSizePerByte * minElementBitCount * i));
                break;

            default:
                gcmASSERT(gcvFALSE);
                newValue = uValue;
                break;
            }
        }

        VIR_Operand_SetImmUint(pOpnd, newValue);
        bChanged = gcvTRUE;
    }

    return bChanged;
}

static gctBOOL _NeedPutImmValue2Uniform(VIR_Shader* pShader,
                                        VIR_Operand *Opnd,
                                        VSC_HW_CONFIG* pHwCfg,
                                        gctBOOL bIsInDual16Check,
                                        gctBOOL bPackedChanged,
                                        VIR_ConstVal* ImmValues,
                                        VIR_TypeId* ImmTypeId)
{
    VIR_Const*  constValue;

    memset(ImmValues, 0, sizeof(VIR_ConstVal));

    if (VIR_Operand_isImm(Opnd))
    {
        ImmValues->scalarVal.uValue = VIR_Operand_GetImmediateUint(Opnd);
        gcmASSERT(VIR_TypeId_isPrimitive(VIR_Operand_GetTypeId(Opnd)));
        *ImmTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Opnd));

        /* For chips that do not support imm values, we must put imm to uniform */
        if (!pHwCfg->hwFeatureFlags.hasSHEnhance2)
        {
            return gcvTRUE;
        }

        /* For non-dual16 shader, imm supported by HW is only 20-bit;
           For dual16 shader, imm supported by HW is only 16-bit (two 16-bits pack into one 32-bits) */
        switch (VIR_GetTypeComponentType(VIR_Operand_GetTypeId(Opnd)))
        {
        case VIR_TYPE_FLOAT32:
            if (bIsInDual16Check || VIR_Shader_isDual16Mode(pShader))
            {
                return gcvTRUE;
            }
            else
            {
                return !CAN_EXACTLY_CVT_S23E8_2_S11E8(VIR_Operand_GetImmediateUint(Opnd));
            }
        case VIR_TYPE_INT32:
        case VIR_TYPE_BOOLEAN:
            if (bIsInDual16Check || VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_S32_2_S16(VIR_Operand_GetImmediateInt(Opnd));
            }
            else
            {
                return !CAN_EXACTLY_CVT_S32_2_S20(VIR_Operand_GetImmediateInt(Opnd));
            }

        case VIR_TYPE_UINT32:
            if (bIsInDual16Check || VIR_Shader_isDual16Mode(pShader))
            {
                return !CAN_EXACTLY_CVT_U32_2_U16(VIR_Operand_GetImmediateUint(Opnd));
            }
            else
            {
                return !CAN_EXACTLY_CVT_U32_2_U20(VIR_Operand_GetImmediateUint(Opnd));
            }
        default:
            if (bPackedChanged)
            {
                return !CAN_EXACTLY_CVT_U32_2_U20(VIR_Operand_GetImmediateUint(Opnd));
            }
            return gcvFALSE;
        }
    }
    else if (VIR_Operand_isConst(Opnd))
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

        if (uniform && isSymUniformCompiletimeInitialized(uniformSym) && VIR_Operand_GetRelAddrMode(pOpnd) == VIR_INDEXED_NONE)
        {
            VIR_ConstId constId;
            VIR_Type*   pConstType;

            constId = VIR_Operand_GetConstValForUniform(pShader,
                                                        pOpnd,
                                                        uniformSym,
                                                        uniform,
                                                        0);
            pConst = VIR_Shader_GetConstFromId(pShader, constId);
            pConstType = VIR_Shader_GetTypeFromId(pShader, pConst->type);

            if (VIR_Type_isVector(pConstType) || VIR_Type_isScalar(pConstType))
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

static gctBOOL _IsNeedToPropagate(VIR_Instruction*  pInst,
                                  gctUINT           SrcIdx)
{
    VIR_OpCode      opCode = VIR_Inst_GetOpcode(pInst);

    if (opCode == VIR_OP_SUBSAT && SrcIdx == 1)
    {
        return gcvFALSE;
    }
    /* Don't do propagte for LDARR. */
    else if (opCode == VIR_OP_LDARR && SrcIdx == 0)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctUINT _GetCompCountFromEnable(gctUINT8 enable)
{
    if (enable & VIR_ENABLE_W)
    {
        return VIR_CHANNEL_COUNT;
    }
    else if (enable & VIR_ENABLE_Z)
    {
        return VIR_CHANNEL_COUNT - 1;
    }
    else if (enable & VIR_ENABLE_Y)
    {
        return VIR_CHANNEL_COUNT - 2;
    }
    else if (enable & VIR_ENABLE_X)
    {
        return VIR_CHANNEL_COUNT - 3;
    }

    return VIR_CHANNEL_COUNT;
}

DEF_QUERY_PASS_PROP(vscVIR_PutScalarConstToImm)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PutScalarConstToImm)
{
    VSC_HW_CONFIG*    pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    if (!pHwCfg->hwFeatureFlags.hasSHEnhance2)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_PutScalarConstToImm(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    gctBOOL           bChanged = gcvFALSE;
    gctUINT           srcInx;
    gctUINT           i, j;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;
        VIR_OpCode       opCode;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            opCode = VIR_Inst_GetOpcode(inst);

            for (srcInx = 0; srcInx < VIR_Inst_GetSrcNum(inst); ++srcInx)
            {
                VIR_Operand         *srcOpnd = VIR_Inst_GetSource(inst, srcInx);
                VIR_Modifier        srcModifier = VIR_Operand_GetModifier(srcOpnd);
                VIR_TypeId          srcType = VIR_Operand_GetTypeId(srcOpnd);
                VIR_Swizzle         srcSwizzle = VIR_Operand_GetSwizzle(srcOpnd);
                VIR_Swizzle         channelSwizzle[VIR_CHANNEL_COUNT] = {VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X, VIR_SWIZZLE_X};
                VIR_Swizzle         finalSwizzle = VIR_SWIZZLE_X;
                VIR_ConstVal        constValue;
                gctUINT             immedValue = 0;
                gctBOOL             hasSameValue = gcvTRUE;

                if (!_IsNeedToPropagate(inst, srcInx))
                {
                    continue;
                }

                if (!_IsConstScalar(pShader, inst, srcOpnd, &constValue) ||
                    VIR_TypeId_isPacked(srcType))
                {
                    continue;
                }

                /* Don't propagate a 512bit uniform source. */
                if (VIR_OPCODE_U512_SrcNo(opCode) > 0 && VIR_OPCODE_U512_SrcNo(opCode) == (gctINT)srcInx)
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
                VIR_Operand_SetTypeId(srcOpnd, srcType);
                VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);
                VIR_Operand_SetModifier(srcOpnd, VIR_MOD_NONE);

                /* Mark change happen. */
                bChanged = gcvTRUE;
            }
        }
    }

    if (bChanged &&
        VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After put scalar const to imm", pShader, gcvTRUE);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PutImmValueToUniform)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PutImmValueToUniform)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_PutImmValueToUniform(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode       errCode = VSC_ERR_NONE;
    VIR_Shader*       pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG*    pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL           bChanged = gcvFALSE;
    VIR_FuncIterator  func_iter;
    VIR_FunctionNode* func_node;
    VIR_Operand*      opnd;
    gctUINT           i;
    VIR_Const         virConst;
    VIR_ConstVal      ImmValues[VIR_MAX_SRC_NUM];
    VIR_TypeId        immTypeId[VIR_MAX_SRC_NUM];
    gctBOOL           needChange[VIR_MAX_SRC_NUM];
    gctBOOL           hasChanged[VIR_MAX_SRC_NUM];
    gctBOOL           bPackedChanged = gcvFALSE;
    VIR_Uniform*      pImmUniform;
    VIR_Symbol*       sym;
    VIR_Swizzle       swizzle = VIR_SWIZZLE_XXXX;
    gctUINT           numChange;
    VIR_PrimitiveTypeId dstElemType = VIR_TYPE_VOID, dstType = VIR_TYPE_VOID;

    memset(&ImmValues, 0, sizeof(VIR_ConstVal) * VIR_MAX_SRC_NUM);
    memset(&immTypeId, 0, sizeof(VIR_TypeId) * VIR_MAX_SRC_NUM);
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
                bPackedChanged = gcvFALSE;
                opnd = VIR_Inst_GetSource(inst, i);

                if (opnd == gcvNULL)
                {
                    continue;
                }

                /*
                ** If the source is a packed type, we need to fill a single-full-channel(right now it is 32bit) data with this constant.
                ** For example, we need to do the following change:
                **      006: MOV.pack           half8P temp(28){r4.<0}, short8P 0xFBFF
                **      -->
                **      006: MOV.pack           half8P temp(28){r4.<0}, short8P 0xFBFFFBFF
                */
                bPackedChanged = _ChangePackedConstValue(pShader, opnd);

                /* check whether we need put imm value to uniform */
                if (_NeedPutImmValue2Uniform(pShader, opnd, pHwCfg, gcvFALSE, bPackedChanged, &ImmValues[i], &immTypeId[i]))
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
            if (gcUseFullNewLinker(pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2) &&
                numChange > 1 &&
                numChange <= VIR_CHANNEL_COUNT &&
                !VIR_OPCODE_isVX(VIR_Inst_GetOpcode(inst)))
            {
                /* generate a const vector */
                VIR_ConstVal    new_const_val;
                VIR_ConstId     new_const_id;
                VIR_Const       *new_const;
                VIR_Swizzle     new_swizzle, swizzle = VIR_SWIZZLE_XXXX;
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
                VIR_Shader_AddInitializedUniform(pShader, new_const, &pImmUniform, &swizzle);
                sym = VIR_Shader_GetSymFromId(pShader, pImmUniform->sym);

                constChannel = 0;
                for (i = 0; i < VIR_Inst_GetSrcNum(inst); ++i)
                {
                    opnd = VIR_Inst_GetSource(inst, i);

                    if (needChange[i] && immTypeId[i] == dstElemType)
                    {
                        new_swizzle = VIR_Swizzle_Extract_Single_Channel_Swizzle(swizzle, constChannel);
                        VIR_Operand_SetOpKind(opnd, VIR_OPND_SYMBOL);
                        VIR_Operand_SetSym(opnd, sym);
                        VIR_Operand_SetTypeId(opnd, dstType);
                        VIR_Operand_SetSwizzle(opnd, new_swizzle);
                        constChannel++;
                        hasChanged[i] = gcvTRUE;

                        /* Mark change happen. */
                        bChanged = gcvTRUE;
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

                    /* Mark change happen. */
                    bChanged = gcvTRUE;
                }
            }
        }
    }

    if (bChanged &&
        VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After put imm value to uniform", pShader, gcvTRUE);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_CheckCstRegFileReadPortLimitation)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    /* Only constant allocation is enable, we can check constant reg file read port limitation */
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedCfg = gcvTRUE;

    /* Temp invaliate rd flow here (before IS/RA) as dubo pass does not update du */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_CheckCstRegFileReadPortLimitation)
{
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    /*
    ** When a chip has no const reg read port limitation, it only means that it can support 4*128 constant read bandwidth.
    ** So for a evis instruction that uses a uniform512 source, it can't use any more constant for the other source, we also need to check this.
    */
    if (pHwCfg->hwFeatureFlags.noOneConstLimit)
    {
        if (!VIR_Shader_HasVivVxExtension(pShader))
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_CheckCstRegFileReadPortLimitation(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          func_node;
    VIR_Operand*               firstOpnd = gcvNULL;
    VIR_Operand*               opnd;
    gctUINT                    i, j, firstConstRegNo, thisConstRegNo, newDstRegNo;
    VIR_SymId                  newDstSymId;
    VIR_Symbol*                pSym = gcvNULL, *pNewSym = gcvNULL, *firstSym = gcvNULL;
    VIR_Type*                  pType = gcvNULL;
    VIR_Uniform*               pUniform;
    VIR_Instruction*           pNewInsertedInst;
    gctBOOL                    bFirstConstRegIndexing, bHitReadPortLimitation;
    VIR_Operand *              newOpnd;
    gctBOOL                    bHasOneConstFix = pHwCfg->hwFeatureFlags.noOneConstLimit;
    gctBOOL                    bChanged = gcvFALSE;
    VIR_BASIC_BLOCK            *pBB        = gcvNULL;
    CFG_ITERATOR               cfgIter;
    VSC_HASH_TABLE*            replacedUniformSet = gcvNULL; /* record the map <Sym, defInst> */

    gcmASSERT(VIR_Shader_isConstRegAllocated(pShader));

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    replacedUniformSet = vscHTBL_Create(pPassWorker->basePassWorker.pMM, vscHFUNC_Default, vscHKCMP_Default, 256);

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* inst;
        VIR_OpCode       opCode;
        CFG_ITERATOR_INIT(&cfgIter, &func->pFuncBlock->cfg);
        for (pBB = CFG_ITERATOR_FIRST(&cfgIter); pBB != gcvNULL;
             pBB = CFG_ITERATOR_NEXT(&cfgIter))
        {
            if(BB_GET_LENGTH(pBB) == 0)
            {
                continue;
            }

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
            for (inst = BB_GET_START_INST(pBB); inst != VIR_Inst_GetNext(BB_GET_END_INST(pBB));
                inst = VIR_Inst_GetNext(inst))
            {
                /* Only have one or zero source, just bail out. */
                if (VIR_Inst_GetSrcNum(inst) < 2)
                {
                    continue;
                }

                opCode = VIR_Inst_GetOpcode(inst);

                /* This instruction has not a uniform512 source, just bail out. */
                if (bHasOneConstFix && VIR_OPCODE_U512_SrcNo(opCode) == 0)
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
                            pUniform = VIR_Symbol_GetUniformPointer(pShader, pSym);
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
                                VIR_TypeId newTempRegTypeId;

                                /* decide which operand to replace, do not replace image uniform, 512 bit uniform */
                                if (VIR_Symbol_GetKind(pSym) == VIR_SYM_IMAGE ||
                                    VIR_Symbol_GetKind(pSym) == VIR_SYM_IMAGE_T ||
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
                                gcmASSERT(pSym);
                                pNewInsertedInst = gcvNULL;
                                newDstSymId = 0;
                                newDstRegNo = 0;
                                precision = VIR_Operand_GetPrecision(opnd);
                                /* if the symbol has the corresponding defInst and the operand has no indexing and relmode,
                                 * use the dest symbol of the defInst in opnd to reduce a MOV instruction
                                 */
                                if (vscHTBL_DirectTestAndGet(replacedUniformSet, (void*)pSym, (void**)&pNewInsertedInst))
                                {
                                    VIR_Operand *dest = VIR_Inst_GetDest(pNewInsertedInst);

                                    pNewSym = VIR_Operand_GetSymbol(dest);
                                    gcmASSERT(pNewSym && VIR_Symbol_GetKind(pNewSym) == VIR_SYM_VIRREG);

                                    newTempRegTypeId = VIR_Symbol_GetTypeId(pNewSym);

                                    if (VIR_Operand_GetPrecision(opnd) == VIR_Symbol_GetPrecision(pNewSym) &&
                                        (!VIR_Operand_GetIsConstIndexing(opnd)) &&
                                        (!VIR_Operand_GetMatrixConstIndex(opnd)) &&
                                        (VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE) &&
                                        /* In case the inserted MOV instruction is not cover this channel, insert a new one. */
                                        (VIR_GetTypeComponents(newTempRegTypeId) >= _GetCompCountFromEnable(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opnd)))))
                                    {
                                        newDstSymId = VIR_Symbol_GetIndex(pNewSym);
                                        newDstRegNo = VIR_Symbol_GetVregIndex(pNewSym);
                                    }
                                    else
                                    {
                                        pNewInsertedInst = gcvNULL;
                                    }
                                }
                                if (pNewInsertedInst == gcvNULL)
                                {
                                    /* Try to use the uniform type first so we can cover all channels. */
                                    if (VIR_TypeId_isPrimitive(VIR_Symbol_GetTypeId(pSym)))
                                    {
                                        newTempRegTypeId = VIR_Symbol_GetTypeId(pSym);
                                    }
                                    else
                                    {
                                        pType = VIR_Symbol_GetType(pSym);
                                        if (VIR_Type_isArray(pType))
                                        {
                                            while (VIR_Type_isArray(pType))
                                            {
                                                pType = VIR_Shader_GetTypeFromId(pShader, VIR_Type_GetBaseTypeId(pType));
                                            }
                                            if (VIR_Type_isPrimitive(pType))
                                            {
                                                newTempRegTypeId = VIR_Type_GetIndex(pType);
                                            }
                                            else
                                            {
                                                newTempRegTypeId = VIR_Operand_GetTypeId(opnd);
                                            }
                                        }
                                        else
                                        {
                                            newTempRegTypeId = VIR_Operand_GetTypeId(opnd);
                                        }
                                    }

                                    /* make new created symbol type consistent with src's swizzle channel
                                     * if newTempRegTypeId is float32 computed by operand type while .w is used like following inst
                                     * MUL                hp temp(3).hp.w, hp  uColor.hp.w, hp  uTexCombScale[2].hp.w
                                     * extend newTempRegTypeId to be float32_x3
                                     */
                                    if (VIR_GetTypeComponents(newTempRegTypeId) < _GetCompCountFromEnable(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opnd))))
                                    {
                                        VIR_TypeId componentTypeId = VIR_GetTypeComponentType(newTempRegTypeId);
                                        newTempRegTypeId = VIR_TypeId_ComposeNonOpaqueType(componentTypeId,
                                                                                           _GetCompCountFromEnable(VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opnd))),
                                                                                           1);
                                    }
                                    precision = VIR_Operand_GetPrecision(opnd);

                                    /* Add a new-temp-reg number */
                                    newDstRegNo = VIR_Shader_NewVirRegId(pShader, 1);

                                    errCode = VIR_Shader_AddSymbol(pShader,
                                                                   VIR_SYM_VIRREG,
                                                                   newDstRegNo,
                                                                   VIR_Shader_GetTypeFromId(pShader, newTempRegTypeId),
                                                                   VIR_STORAGE_UNKNOWN,
                                                                   &newDstSymId);
                                    ON_ERROR(errCode, "Add symbol");
                                    pNewSym = VIR_Shader_GetSymFromId(pShader, newDstSymId);

                                    /* Add following inst just before current inst:

                                        mov new-temp-reg, uniform
                                    */
                                    errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, newTempRegTypeId, inst, gcvTRUE, &pNewInsertedInst);
                                    ON_ERROR(errCode, "Add instruction before");

                                    /* dst */
                                    newOpnd = VIR_Inst_GetDest(pNewInsertedInst);
                                    VIR_Operand_SetSymbol(newOpnd, func, newDstSymId);
                                    VIR_Operand_SetEnable(newOpnd, VIR_TypeId_Conv2Enable(newTempRegTypeId));
                                    VIR_Symbol_SetPrecision(pNewSym, precision);
                                    VIR_Operand_SetPrecision(newOpnd, precision);
                                    if(precision == VIR_PRECISION_HIGH)
                                    {
                                        VIR_Inst_SetThreadMode(pNewInsertedInst, VIR_THREAD_D16_DUAL_32);
                                    }

                                    /* src */
                                    newOpnd = VIR_Inst_GetSource(pNewInsertedInst, VIR_Operand_Src0);
                                    VIR_Operand_SetSymbol(newOpnd, func, pSym->index);
                                    VIR_Operand_SetSwizzle(newOpnd, VIR_TypeId_Conv2Swizzle(newTempRegTypeId));
                                    VIR_Operand_SetTypeId(newOpnd, newTempRegTypeId);
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
                                    if ((!VIR_Operand_GetIsConstIndexing(opnd)) &&
                                        (!VIR_Operand_GetMatrixConstIndex(opnd)) &&
                                        (VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE))
                                    {
                                        vscHTBL_DirectSet(replacedUniformSet, (void*)pSym, (void*)pNewInsertedInst);
                                    }
                                }
                                /* Change operand of current inst to new-temp-reg */
                                VIR_Operand_SetMatrixConstIndex(opnd, 0);
                                VIR_Operand_SetRelAddrMode(opnd, VIR_INDEXED_NONE);
                                VIR_Operand_SetRelIndexing(opnd, 0);
                                VIR_Operand_SetTempRegister(opnd, func, newDstSymId, VIR_Operand_GetTypeId(opnd));
                                if (precision == VIR_PRECISION_HIGH)
                                {
                                    VIR_Inst_SetThreadMode(inst, VIR_THREAD_D16_DUAL_32);
                                }

                                vscVIR_AddNewDef(pDuInfo, pNewInsertedInst, newDstRegNo, 1, VIR_ENABLE_XYZW, VIR_HALF_CHANNEL_MASK_FULL,
                                                 gcvNULL, gcvNULL);

                                vscVIR_AddNewUsageToDef(pDuInfo, pNewInsertedInst, inst, opnd,
                                                        gcvFALSE, newDstRegNo, 1, VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(opnd)),
                                                        VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);

                                /* Change happens. */
                                bChanged = gcvTRUE;
                            }
                        }
                    }
                }
            }
            vscHTBL_Reset(replacedUniformSet);
        }
    }

    if (bChanged && VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After fix const reg read port limitation", pShader, gcvTRUE);
    }

OnError:
    vscHTBL_Destroy(replacedUniformSet);
    return errCode;
}

static VSC_ErrCode
_FixSwizzleRestrictForLocalStorageAddr(
    VIR_Shader*         pShader,
    VIR_DEF_USAGE_INFO* pDuInfo,
    VIR_Function*       pFunc,
    VIR_Instruction*    pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Operand*        pOrigSrc = VIR_Inst_GetSource(pInst, 0);
    VIR_Symbol*         pLocalStorageAddr = VIR_Operand_GetSymbol(pOrigSrc);
    VIR_Instruction*    pNewInst = gcvNULL;
    VIR_Operand*        pNewOpnd = gcvNULL;
    VIR_VirRegId        destRegId = VIR_INVALID_ID;
    VIR_SymId           destSymId = VIR_INVALID_ID;
    VIR_TypeId          typeId = VIR_Operand_GetTypeId(pOrigSrc);
    VIR_Enable          enable = VIR_TypeId_Conv2Enable(typeId);

    /* Delete usage first. */
    vscVIR_DeleteUsage(pDuInfo,
                       VIR_INPUT_DEF_INST,
                       pInst,
                       pOrigSrc,
                       gcvFALSE,
                       VIR_Symbol_GetVariableVregIndex(pLocalStorageAddr),
                       1,
                       VIR_ENABLE_W,
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

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
                            VIR_INPUT_DEF_INST,
                            pNewInst,
                            pNewOpnd,
                            gcvFALSE,
                            VIR_Symbol_GetVariableVregIndex(pLocalStorageAddr),
                            1,
                            VIR_ENABLE_W,
                            VIR_HALF_CHANNEL_MASK_FULL,
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

DEF_QUERY_PASS_PROP(vscVIR_CheckEvisInstSwizzleRestriction)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    /* Only constant allocation is enable, we can check constant reg file read port limitation */
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;

    /* Temp invaliate rd flow here (before IS/RA) as dubo pass does not update du */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_CheckEvisInstSwizzleRestriction)
{
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    /* If it is not OCL, just bail out */
    if (!VIR_Shader_IsCL(pShader))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_CheckEvisInstSwizzleRestriction(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VSC_HW_CONFIG*             pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          func_node;
    VIR_Operand*               opnd;
    gctUINT                    j;
    VIR_Symbol*                pSym = gcvNULL;

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
            if (!VIR_OPCODE_isVX(VIR_Inst_GetOpcode(inst)))
            {
                continue;
            }

            for (j = 0; j < VIR_Inst_GetSrcNum(inst); ++j)
            {
                opnd = VIR_Inst_GetSource(inst, j);

                /* For a VX instruction, the swizzle of source0 is used for sourceBin, we can't shift it. */
                if (pHwCfg->hwFeatureFlags.useSrc0SwizzleAsSrcBin &&
                    j == 0 &&
                    (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL ||
                     VIR_Operand_GetOpKind(opnd) == VIR_OPND_VIRREG))
                {
                    pSym = VIR_Operand_GetSymbol(opnd);
                    /* If the source is local storage addr, HW saves it to the w channel, so we need to insert a MOV to replace it. */
                    if (VIR_Symbol_GetName(pSym) == VIR_NAME_LOCAL_INVOCATION_ID && VIR_Operand_GetSwizzle(opnd) == VIR_SWIZZLE_WWWW)
                    {
                        _FixSwizzleRestrictForLocalStorageAddr(pShader, pDuInfo, func, inst);
                        opnd = VIR_Inst_GetSource(inst, j);
                        pSym = VIR_Operand_GetSymbol(opnd);
                    }
                    VIR_Symbol_SetCannotShift(pSym, gcvTRUE);
                }

                if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
                {
                    pSym = VIR_Operand_GetSymbol(opnd);

                    if (VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
                    {
                        /* set the symbol as cannotShift if the symbol is used in EVIS source,
                         * since the swizzle bits are used for EVIS states */
                        VIR_Symbol_SetCannotShift(pSym, gcvTRUE);
                    }
                }
            }
        }
    }

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

    vscBV_Initialize(&tempMask, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);
    vscBV_Initialize(&depDefIdxMask, pDuInfo->baseTsDFA.baseDFA.pScratchMemPool, pDuInfo->baseTsDFA.baseDFA.flowSize);

    /* Collect all depth defs and mask them */
    depDefIdx = vscVIR_FindFirstDefIndex(pDuInfo, pDepthSym->u2.tempIndex);
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

DEF_SH_NECESSITY_CHECK(vscVIR_CheckPosAndDepthConflict)
{
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    if (pShader->shaderKind != VIR_SHADER_FRAGMENT)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
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
    errCode = VIR_Function_AddInstruction(pShader->mainFunction, VIR_OP_MOV,
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

DEF_QUERY_PASS_PROP(vscVIR_ConvFrontFacing)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedRdFlow = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_ConvFrontFacing)
{
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Symbol*                pFrontFacingSym = gcvNULL;

    if (pShader->shaderKind != VIR_SHADER_FRAGMENT)
    {
        return gcvFALSE;
    }

    /* There is no integer support for ES11, so we always treat FrontFacing as a floating variable. */
    if (VIR_Shader_IsES11Compiler(pShader))
    {
        return gcvFALSE;
    }

    /* Check if frontFacing is used. */
    pFrontFacingSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_FRONT_FACING);
    if (pFrontFacingSym == gcvNULL || isSymUnused(pFrontFacingSym))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

/* HW always use a floating variable to save the FrontFacing, so we need to convert it to a boolean variable to match the spec. */
VSC_ErrCode vscVIR_ConvFrontFacing(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                errCode = VSC_ERR_NONE;
    VIR_Shader*                pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
    gctUINT                    symId = VIR_INVALID_ID, newRegNo, newRegSymId = VIR_INVALID_ID;
    VIR_NameId                 nameId = VIR_INVALID_ID;
    VIR_Symbol*                pRegSym = gcvNULL;
    VIR_Symbol*                pVarSym = gcvNULL;
    VIR_Symbol*                pFrontFacingSym = gcvNULL;
    VIR_FuncIterator           func_iter;
    VIR_FunctionNode*          pFunc_node;
    VIR_Function*              pFunc;
    VIR_Instruction*           pConvInst = gcvNULL;
    VIR_Operand*               pOpnd = gcvNULL;

    gcmASSERT(pShader->shaderKind == VIR_SHADER_FRAGMENT);

    /* Check if frontFacing is used. */
    pFrontFacingSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_FRONT_FACING);
    if (pFrontFacingSym == gcvNULL || isSymUnused(pFrontFacingSym))
    {
        gcmASSERT(gcvFALSE);
        return VSC_ERR_NONE;
    }

    /* Create a temp register to hold the integer value of FrontFacing. */
    errCode = VIR_Shader_AddString(pShader,
                                   "#int_frontFacing",
                                   &nameId);
    ON_ERROR(errCode, "Add nameString");

    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VARIABLE,
                                   nameId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_BOOLEAN),
                                   VIR_STORAGE_UNKNOWN,
                                   &symId);
    pVarSym = VIR_Shader_GetSymFromId(pShader, symId);

    if (errCode == VSC_ERR_REDEFINITION)
    {
        newRegNo = VIR_Symbol_GetVariableVregIndex(pVarSym);
        errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader,
                                                    newRegNo,
                                                    &newRegSymId);
        pRegSym = VIR_Shader_GetSymFromId(pShader, newRegSymId);
    }
    else
    {
        newRegNo = VIR_Shader_NewVirRegId(pShader, 1);
        VIR_Symbol_SetVariableVregIndex(pVarSym, newRegNo);

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       newRegNo,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_BOOLEAN),
                                       VIR_STORAGE_UNKNOWN,
                                       &newRegSymId);
        ON_ERROR(errCode, "Add symbol");
        pRegSym = VIR_Shader_GetSymFromId(pShader, newRegSymId);
        VIR_Symbol_SetVregVarSymId(pRegSym, symId);

        /* Insert a CONVERT instruction for FrontFacing. */
        errCode = VIR_Function_PrependInstruction(pShader->mainFunction,
                                                  VIR_OP_F2I,
                                                  VIR_TYPE_BOOLEAN,
                                                  &pConvInst);
        ON_ERROR(errCode, "add instruction");

        /* Set dst. */
        pOpnd = VIR_Inst_GetDest(pConvInst);
        VIR_Operand_SetSymbol(pOpnd, VIR_Shader_GetMainFunction(pShader), newRegSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        /* Set src0. */
        pOpnd = VIR_Inst_GetSource(pConvInst, VIR_Operand_Src0);
        VIR_Operand_SetSymbol(pOpnd, VIR_Shader_GetMainFunction(pShader), VIR_Symbol_GetIndex(pFrontFacingSym));
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);
        VIR_Operand_SetTypeId(pOpnd, VIR_TYPE_FLOAT32);

        /* Update DU. */
        vscVIR_AddNewDef(pDuInfo, pConvInst, newRegNo, 1, VIR_ENABLE_X, VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL, gcvNULL);

        vscVIR_AddNewUsageToDef(pDuInfo, VIR_INPUT_DEF_INST, pConvInst,
                                pOpnd,
                                gcvFALSE, VIR_Symbol_GetVariableVregIndex(pFrontFacingSym), 1, VIR_ENABLE_X,
                                VIR_HALF_CHANNEL_MASK_FULL, gcvNULL);
    }

    /* Find if any operand using gl_FrontFace and use the new temp register to replace it. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (pFunc_node = VIR_FuncIterator_First(&func_iter);
         pFunc_node != gcvNULL; pFunc_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator       inst_iter;
        VIR_Instruction*       pInst = gcvNULL;

        pFunc = pFunc_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SrcOperand_Iterator srcOpndIter;
            gctBOOL                 convertInst = gcvFALSE;

            pOpnd = VIR_Inst_GetDest(pInst);
            if (pOpnd && VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL && VIR_Operand_GetSymbol(pOpnd) == pRegSym)
            {
                convertInst = gcvTRUE;
                pConvInst = pInst;
            }

            VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);
            pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; pOpnd != gcvNULL; pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL &&
                    VIR_Symbol_GetName(VIR_Operand_GetSymbol(pOpnd)) == VIR_NAME_FRONT_FACING)
                {
                    if (convertInst)
                    {
                        break;
                    }
                    else
                    {
                        /* Delete usage. */
                        vscVIR_DeleteUsage(pDuInfo,
                                           VIR_INPUT_DEF_INST,
                                           pInst,
                                           pOpnd,
                                           gcvFALSE,
                                           VIR_Symbol_GetVariableVregIndex(pFrontFacingSym),
                                           1,
                                           VIR_ENABLE_X,
                                           VIR_HALF_CHANNEL_MASK_FULL,
                                           gcvNULL);

                        VIR_Operand_SetTempRegister(pOpnd, pFunc, newRegSymId, VIR_TYPE_BOOLEAN);

                        /* Add the new usage. */
                        vscVIR_AddNewUsageToDef(pDuInfo,
                                                pConvInst,
                                                pInst,
                                                pOpnd,
                                                gcvFALSE,
                                                newRegNo,
                                                1,
                                                VIR_ENABLE_X,
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                gcvNULL);
                    }
                }
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ChangeUniformTypeToUvec3(VIR_Shader* pShader,
                                             VIR_Symbol* pUniformSym)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Uniform*                pUniform = gcvNULL;
    VIR_Symbol*                 pUnderlyingSym = gcvNULL;
    VIR_Type*                   pUniformSymType = gcvNULL;
    VIR_Type*                   pNewSymType = gcvNULL;
    VIR_TypeId                  newSymTypeId;

    /* Change uniform type to vec4 to hold lower/upper bounds in x/y and mem-size in W channel. */
    if (VIR_Symbol_HasFlag(pUniformSym, VIR_SYMUNIFORMFLAG_ATOMIC_COUNTER))
    {
        pUniform = VIR_Symbol_GetUniform(pUniformSym);
        pUnderlyingSym = VIR_Shader_GetSymFromId(pShader, pUniform->baseBindingUniform);

        pUniformSymType = VIR_Symbol_GetType(pUnderlyingSym);
        gcmASSERT((VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(pUniformSymType)) & VIR_TYFLAG_ISINTEGER));
        if (VIR_Type_isArray(pUniformSymType))
        {
            errCode = VIR_Shader_AddArrayType(pShader, VIR_TYPE_ATOMIC_UINT4,
                                                VIR_Type_GetArrayLength(pUniformSymType), 0, &newSymTypeId);
            ON_ERROR(errCode, "Add array type");
            pNewSymType = VIR_Shader_GetTypeFromId(pShader, newSymTypeId);
            VIR_Symbol_SetType(pUnderlyingSym, pNewSymType);
        }
        else
        {
            VIR_Symbol_SetType(pUnderlyingSym, VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_ATOMIC_UINT4));
        }
    }
    else
    {
        pUniformSymType = VIR_Symbol_GetType(pUniformSym);
        gcmASSERT((VIR_GetTypeFlag(VIR_Type_GetBaseTypeId(pUniformSymType)) & VIR_TYFLAG_ISINTEGER));

        if (VIR_Type_isArray(pUniformSymType))
        {
            errCode = VIR_Shader_AddArrayType(pShader, VIR_TYPE_UINT_X4,
                                                VIR_Type_GetArrayLength(pUniformSymType), 0, &newSymTypeId);
            ON_ERROR(errCode, "Add array type");
            pNewSymType = VIR_Shader_GetTypeFromId(pShader, newSymTypeId);
            VIR_Symbol_SetType(pUniformSym, pNewSymType);
        }
        else
        {
            VIR_Symbol_SetType(pUniformSym, VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4));
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _InitializeUniformCopy(VIR_DEF_USAGE_INFO* pDuInfo,
                                          VIR_Shader* pShader,
                                          VIR_SymId* pResultSymId,
                                          VIR_Operand* pUniformOpnd,
                                          VIR_Symbol* pUniformSym)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Function*               pMainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_VirRegId                virRegId = VIR_Shader_NewVirRegId(pShader, 1);
    VIR_SymId                   resultSymId;
    VIR_Instruction*            pNewInst;
    VIR_Operand*                pOpnd;
    VIR_OperandInfo             operandInfo;
    gctBOOL                     bIsImage = gcvFALSE;

    /*
    ** If the base uniform is an image, then the address of LOAD/STORE/ATOMIC have already been calculated
    ** and valided, so we just set the YZ to the full range.
    */
    if (VIR_Symbol_isImage(pUniformSym) || VIR_Symbol_isImageT(pUniformSym))
    {
        bIsImage = gcvTRUE;
    }

    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   virRegId,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &resultSymId);
    ON_ERROR(errCode, "Add result symbol");

    if (bIsImage)
    {
        /* Insert the MOV-X instruction. */
        errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                  VIR_OP_MOV,
                                                  VIR_TYPE_UINT32,
                                                  &pNewInst);
        ON_ERROR(errCode, "Prepend instruction");

        /* Set DEST. */
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, resultSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);
        VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

        /* Update def. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewInst,
                         operandInfo.u1.virRegInfo.virReg,
                         1,
                         VIR_ENABLE_X,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Set SOURCE. */
        pOpnd = VIR_Inst_GetSource(pNewInst, 0);

        /*
        ** HW has underflow bug which can not use 0 as lower bound: it used (lowerB - sz + 1) to check OOB access.
        ** so we just set it to the base start address.
        */
        VIR_Operand_Copy(pOpnd, pUniformOpnd);
        VIR_Operand_SetTypeId(pOpnd, VIR_TYPE_UINT32);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

        /* Insert the MOV-Y instruction. */
        errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                  VIR_OP_MOV,
                                                  VIR_TYPE_UINT32,
                                                  &pNewInst);
        ON_ERROR(errCode, "Prepend instruction");

        /* Set DEST. */
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, resultSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_Y);
        VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

        /* Update def. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewInst,
                         operandInfo.u1.virRegInfo.virReg,
                         1,
                         VIR_ENABLE_Y,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Set SOURCE. */
        pOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetImmediateUint(pOpnd, 0xFFFFFFFF);
    }
    else
    {
        /* Change the type first. */
        errCode = _ChangeUniformTypeToUvec3(pShader, pUniformSym);
        ON_ERROR(errCode, "Change the uniform type.");

        /* Insert the MOV-XY instruction. */
        errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                  VIR_OP_MOV,
                                                  VIR_TYPE_UINT_X2,
                                                  &pNewInst);
        ON_ERROR(errCode, "Prepend instruction");

        /* Set DEST. */
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, resultSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XY);
        VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

        /* Update def. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewInst,
                         operandInfo.u1.virRegInfo.virReg,
                         1,
                         VIR_ENABLE_XY,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Set SOURCE. */
        pOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_Copy(pOpnd, pUniformOpnd);
        VIR_Operand_SetTypeId(pOpnd, VIR_TYPE_UINT_X2);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_YZZZ);
    }

    if (pResultSymId)
    {
        *pResultSymId = resultSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode _InsertAddressCopy(VIR_DEF_USAGE_INFO* pDuInfo,
                                      VIR_Shader* pShader,
                                      VIR_Instruction* pInstPosition,
                                      VIR_SymId* pResultSymId,
                                      VIR_SymId sourceSymId)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_SymId                   resultSymId;
    VIR_Instruction*            pNewInst;
    VIR_Operand*                pOpnd;
    VIR_Function*               pFunc = VIR_Inst_GetFunction(pInstPosition);
    VIR_OperandInfo             operandInfo;

    gcmASSERT(pResultSymId);
    resultSymId = *pResultSymId;

    /* Allocate the result reg symbol for this working instruction if no exist. */
    if (resultSymId == VIR_INVALID_ID)
    {
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       VIR_Shader_NewVirRegId(pShader, 1),
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X2),
                                       VIR_STORAGE_UNKNOWN,
                                       &resultSymId);
        ON_ERROR(errCode, "Add result symbol");

        *pResultSymId = resultSymId;
    }

    errCode = VIR_Function_AddInstructionAfter(pFunc,
                                               VIR_OP_MOV,
                                               VIR_TYPE_UINT_X2,
                                               pInstPosition,
                                               gcvTRUE,
                                               &pNewInst);
    ON_ERROR(errCode, "Add instruction after");

    /* Set DEST. */
    pOpnd = VIR_Inst_GetDest(pNewInst);
    VIR_Operand_SetSymbol(pOpnd, pFunc, resultSymId);
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XY);
    VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

    /* Update def. */
    vscVIR_AddNewDef(pDuInfo,
                     pNewInst,
                     operandInfo.u1.virRegInfo.virReg,
                     1,
                     VIR_ENABLE_XY,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     gcvNULL,
                     gcvNULL);

    /* Set SOURCE. */
    pOpnd = VIR_Inst_GetSource(pNewInst, 0);
    VIR_Operand_SetSymbol(pOpnd, pFunc, sourceSymId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYYY);
    VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

    /* Update usage. */
    vscVIR_AddNewUsageToDef(pDuInfo,
                            VIR_ANY_DEF_INST,
                            pNewInst,
                            pOpnd,
                            gcvFALSE,
                            operandInfo.u1.virRegInfo.virReg,
                            1,
                            VIR_ENABLE_XY,
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);

OnError:
    return errCode;
}

static VSC_ErrCode _InsertCompareAddressCopy(VIR_DEF_USAGE_INFO* pDuInfo,
                                             VIR_Shader* pShader,
                                             VIR_Instruction* pCompInst,
                                             VIR_SymId* pResultSymId,
                                             VIR_SymId sourceSymIds[2])
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_SymId                   resultSymId;
    VIR_Instruction*            pNewInst;
    VIR_Operand*                pOpnd;
    VIR_Function*               pFunc = VIR_Inst_GetFunction(pCompInst);
    VIR_OperandInfo             operandInfo;
    gctUINT                     i;

    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   VIR_Shader_NewVirRegId(pShader, 1),
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X2),
                                   VIR_STORAGE_UNKNOWN,
                                   &resultSymId);
    ON_ERROR(errCode, "Add result symbol");

    errCode = VIR_Function_AddInstructionAfter(pFunc,
                                               VIR_Inst_GetOpcode(pCompInst),
                                               VIR_TYPE_UINT_X2,
                                               pCompInst,
                                               gcvTRUE,
                                               &pNewInst);
    ON_ERROR(errCode, "Add instruction after");

    VIR_Inst_SetConditionOp(pNewInst, VIR_Inst_GetConditionOp(pCompInst));

    /* Set DEST. */
    pOpnd = VIR_Inst_GetDest(pNewInst);
    VIR_Operand_SetSymbol(pOpnd, pFunc, resultSymId);
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XY);
    VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

    /* Update def. */
    vscVIR_AddNewDef(pDuInfo,
                     pNewInst,
                     operandInfo.u1.virRegInfo.virReg,
                     1,
                     VIR_ENABLE_XY,
                     VIR_HALF_CHANNEL_MASK_FULL,
                     gcvNULL,
                     gcvNULL);

    /* Copy the condition operand. */
    pOpnd = VIR_Inst_GetSource(pNewInst, 0);
    VIR_Operand_Copy(pOpnd, VIR_Inst_GetSource(pCompInst, 0));
    VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

    /* Update usage. */
    if (operandInfo.isVreg)
    {
        vscVIR_AddNewUsageToDef(pDuInfo,
                                VIR_ANY_DEF_INST,
                                pNewInst,
                                pOpnd,
                                gcvFALSE,
                                operandInfo.u1.virRegInfo.virReg,
                                1,
                                VIR_ENABLE_XY,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);
    }
    /* Set SOURCEs. */
    for (i = 0; i < 2; i++)
    {
        pOpnd = VIR_Inst_GetSource(pNewInst, i + 1);
        VIR_Operand_SetSymbol(pOpnd, pFunc, sourceSymIds[i]);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYYY);
        VIR_Operand_GetOperandInfo(pNewInst, pOpnd, &operandInfo);

        /* Update usage. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                VIR_ANY_DEF_INST,
                                pNewInst,
                                pOpnd,
                                gcvFALSE,
                                operandInfo.u1.virRegInfo.virReg,
                                1,
                                VIR_ENABLE_XY,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);
    }

    if (pResultSymId)
    {
        *pResultSymId = resultSymId;
    }
OnError:
    return errCode;
}

static VSC_ErrCode _GetLowerUpperVirRegSymId(VSC_MM* pMM,
                                             VIR_Shader* pShader,
                                             VIR_DEF_USAGE_INFO* pDuInfo,
                                             VSC_HASH_TABLE* pWorkingSet,
                                             VIR_Instruction* pWorkingInst,
                                             VIR_Operand* pOpnd,
                                             VIR_SymId* pVirRegSymId)
    {
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_DEF*                    pDef;
    VIR_GENERAL_UD_ITERATOR     udIter;
    gctUINT                     i;
    VIR_Symbol*                 pResultRegSym = gcvNULL;
    VIR_SymId                   resultRegSymId = VIR_INVALID_ID, firstRegSymId = VIR_INVALID_ID;
    VIR_Instruction*            pFirstDefInst = gcvNULL;
    gctBOOL                     bFound = gcvFALSE, bHasOnlyOneDef = gcvTRUE;

    /* We already generate a symbol for this instruction. */
    if (vscHTBL_DirectTestAndGet(pWorkingSet, (void *)pWorkingInst, (void **)&pResultRegSym))
    {
        resultRegSymId = VIR_Symbol_GetIndex(pResultRegSym);
        if (pVirRegSymId)
        {
            *pVirRegSymId = resultRegSymId;
        }
        return errCode;
    }

    /* Find all the defs. */
    vscVIR_InitGeneralUdIterator(&udIter, pDuInfo, pWorkingInst, pOpnd, gcvFALSE, gcvFALSE);
    for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
         pDef != gcvNULL;
         pDef = vscVIR_GeneralUdIterator_Next(&udIter))
    {
        VIR_Instruction*        pDefInst = pDef->defKey.pDefInst;
        VIR_Operand*            pSrcOpnd;
        VIR_Symbol*             pSrcSym = gcvNULL;
        VIR_OperandInfo         operandInfo;
        VIR_OpCode              opCode;
        gctBOOL                 bCompareInst = gcvFALSE, bCompareInstAlsoFirstDef = gcvFALSE;
        gctBOOL                 bFoundForThisDefSrc = gcvFALSE;
        VIR_SymId               workingRegSymId = VIR_INVALID_ID;
        VIR_SymId               compareSymIds[VIR_MAX_SRC_NUM] = { VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID, VIR_INVALID_ID };

        if (VIR_IS_SPECIAL_INST(pDefInst))
        {
            continue;
        }
        opCode = VIR_Inst_GetOpcode(pDefInst);

        /* So far only check CSELECT for the compare instruction. */
        if (opCode == VIR_OP_CSELECT)
        {
            bCompareInst = gcvTRUE;
        }

        /* We already generate a symbol for this def instruction. */
        if (vscHTBL_DirectTestAndGet(pWorkingSet, (void *)pDefInst, (void **)&pResultRegSym))
        {
            workingRegSymId = VIR_Symbol_GetIndex(pResultRegSym);
            if (!bFound)
            {
                bFound = gcvTRUE;
                pFirstDefInst = pDefInst;
                firstRegSymId = workingRegSymId;
            }
            else
        {
                /* It has multi-def, so we need to allocate a new result reg for this working instruction. */
                bHasOnlyOneDef = gcvFALSE;

                /* Copy the address. */
                errCode = _InsertAddressCopy(pDuInfo,
                                             pShader,
                                             pDefInst,
                                             &resultRegSymId,
                                             workingRegSymId);
                ON_ERROR(errCode, "Insert address assign.");
            }

            continue;
        }

        /* Check all sources. */
        opCode = VIR_Inst_GetOpcode(pDefInst);
        for (i = 0; i < VIR_Inst_GetSrcNum(pDefInst); ++i)
        {
            workingRegSymId = VIR_INVALID_ID;
            bFoundForThisDefSrc = gcvFALSE;

            if (opCode == VIR_OP_MAD ||
                opCode == VIR_OP_IMADLO0 ||
                opCode == VIR_OP_IMADLO1)
            {
                /* only check src2 if it is MAD operation */
                i = 2;
            }
            else if (bCompareInst)
            {
                i = (i == 0) ? 1 : i;
            }

            pSrcOpnd = VIR_Inst_GetSource(pDefInst, i);
            pSrcSym = VIR_Operand_GetSymbol(pSrcOpnd);

            if (VIR_Operand_GetOpKind(pSrcOpnd) == VIR_OPND_SYMBOL &&
                (VIR_Symbol_isUniform(pSrcSym)   ||
                 VIR_Symbol_isImage(pSrcSym)     ||
                 VIR_Symbol_isImageT(pSrcSym)))
            {
                /* Check if we already generate a symbol for this uniform. */
                if (!vscHTBL_DirectTestAndGet(pWorkingSet, (void *)pSrcSym, (void **)&pResultRegSym))
                {
                    /* Copy the .yz channel of this uniform. */
                    errCode = _InitializeUniformCopy(pDuInfo,
                                                     pShader,
                                                     &workingRegSymId,
                                                     pSrcOpnd,
                                                     pSrcSym);
                    ON_ERROR(errCode, "Initialize uniform copy.");

                    /* Update the hash. */
                    pResultRegSym = VIR_Shader_GetSymFromId(pShader, workingRegSymId);
                    vscHTBL_DirectSet(pWorkingSet, (void *)pSrcSym, (void *)pResultRegSym);
                }
                else
            {
                    workingRegSymId = VIR_Symbol_GetIndex(pResultRegSym);
                }
                gcmASSERT(workingRegSymId != VIR_INVALID_ID);
                bFoundForThisDefSrc = gcvTRUE;
                }
                else
                {
                VIR_Operand_GetOperandInfo(pDefInst, pSrcOpnd, &operandInfo);

                if (VIR_OpndInfo_Is_Virtual_Reg(&operandInfo))
                {
                    errCode = _GetLowerUpperVirRegSymId(pMM,
                                                        pShader,
                                                        pDuInfo,
                                                        pWorkingSet,
                                                        pDefInst,
                                                        pSrcOpnd,
                                                        &workingRegSymId);
                    ON_ERROR(errCode, "Get lower upper virRegSymId.");

                    if (workingRegSymId != VIR_INVALID_ID)
                    {
                        bFoundForThisDefSrc = gcvTRUE;
                    }
                }
            }

            /* Find one matched reg sym id. */
            if (bFoundForThisDefSrc)
            {
                gcmASSERT(workingRegSymId != VIR_INVALID_ID);
                if (!bFound)
                {
                    bFound = gcvTRUE;
                    if (!bCompareInst)
                    {
                        pFirstDefInst = pDefInst;
                        firstRegSymId = workingRegSymId;
                    }
                    else
                    {
                        compareSymIds[i - 1] = workingRegSymId;
                        bCompareInstAlsoFirstDef = gcvTRUE;
                    }
                }
                else
                {
                    /* It has multi-def, so we need to allocate a new result reg for this working instruction. */
                    bHasOnlyOneDef = gcvFALSE;

                    if (!bCompareInst)
                    {
                        /* Copy the address. */
                        errCode = _InsertAddressCopy(pDuInfo,
                                                     pShader,
                                                     pDefInst,
                                                     &resultRegSymId,
                                                     workingRegSymId);
                        ON_ERROR(errCode, "Insert address assign.");
                    }
                    else
                    {
                        compareSymIds[i - 1] = workingRegSymId;
                    }
                }

                /* Only check one source for a non-compare instruction. */
                if (!bCompareInst)
                {
                    break;
                }
            }
        }

        if (bFoundForThisDefSrc)
        {
            /* If this match-def is a compare instruction, we need to insert a compare instruction too. */
            if (bCompareInst)
            {
                gcmASSERT(compareSymIds[0] != VIR_INVALID_ID && compareSymIds[1] != VIR_INVALID_ID);

                errCode = _InsertCompareAddressCopy(pDuInfo,
                                                    pShader,
                                                    pDefInst,
                                                    &workingRegSymId,
                                                    compareSymIds);
                ON_ERROR(errCode, "Insert compare address assign.");

                if (bCompareInstAlsoFirstDef)
                    {
                    pFirstDefInst = VIR_Inst_GetNext(pDefInst);
                    firstRegSymId = workingRegSymId;
                }
            }

            pResultRegSym = VIR_Shader_GetSymFromId(pShader, workingRegSymId);
            vscHTBL_DirectSet(pWorkingSet, (void *)pDefInst, (void *)pResultRegSym);
                }
            }

    gcmASSERT(bFound);

    /* Copy the first def if there are multi-defs, otherwise just use the first def. */
    if (!bHasOnlyOneDef)
    {
        gcmASSERT(firstRegSymId != VIR_INVALID_ID);

        /* Copy the address. */
        errCode = _InsertAddressCopy(pDuInfo,
                                     pShader,
                                     pFirstDefInst,
                                     &resultRegSymId,
                                     firstRegSymId);
        ON_ERROR(errCode, "Insert address assign. ");
                    }
    else
    {
        resultRegSymId = firstRegSymId;
                    }

    gcmASSERT(resultRegSymId != VIR_INVALID_ID);
    pResultRegSym = VIR_Shader_GetSymFromId(pShader, resultRegSymId);

    if (pVirRegSymId)
    {
        *pVirRegSymId = resultRegSymId;
                }

OnError:
    return errCode;
            }

DEF_QUERY_PASS_PROP(vscVIR_AddOutOfBoundCheckSupport)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_AddOutOfBoundCheckSupport)
{
    if (!(pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_AddOutOfBoundCheckSupport(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode               errCode = VSC_ERR_NONE;
    VSC_MM*                   pMM = pPassWorker->basePassWorker.pMM;
    VSC_HASH_TABLE*           pWorkingSet = gcvNULL;
    VIR_Shader*               pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*       pDuInfo = pPassWorker->pDuInfo;
    VIR_FuncIterator          func_iter;
    VIR_FunctionNode*         func_node;
    VIR_Function*             func;
    VIR_InstIterator          inst_iter;
    VIR_Instruction*          inst;
    VIR_Operand*              pOpnd;
    VIR_Symbol*               pSym = gcvNULL, *pNewSym = gcvNULL;
    gctBOOL                   bNeedInsertMov;
    VIR_OperandInfo           operandInfo;
    gctUINT                   newDstRegNo, i;
    VIR_SymId                 newDstSymId;
    VIR_Instruction*          pNewInsertedInstX;
    VIR_Instruction*          pNewInsertedInstYZ;
    VIR_OperandInfo           srcInfo;
    VIR_GENERAL_UD_ITERATOR   udIter;
    VIR_DEF*                  pDef;
    VIR_Operand *             opnd;
    gctBOOL                   bChanged = gcvFALSE;

    if (VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Before out of bound check", pShader, gcvTRUE);
    }

    /* Change the data type of the base address uniform. */
    for(i = 0; i < VIR_IdList_Count(&pShader->uniforms); ++i)
    {
        VIR_Id      id  = VIR_IdList_GetId(&pShader->uniforms, i);
        VIR_Symbol *sym = VIR_Shader_GetSymFromId(pShader, id);

        errCode = VIR_Shader_ChangeAddressUniformTypeToFatPointer(pShader, sym);
        ON_ERROR0(errCode);
    }

    /* Initialize the working set. */
    pWorkingSet = vscHTBL_Create(pMM, vscHFUNC_Default, vscHKCMP_Default, 256);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_SymId virRegSymId = VIR_INVALID_ID;
            VIR_Symbol* pVirRegSym = gcvNULL;

            bNeedInsertMov = gcvFALSE;

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
            pSym = VIR_Operand_GetSymbol(pOpnd);

            /* Try to find corresponding mem-base-addr uniform virreg */
            if (VIR_Operand_GetOpKind(pOpnd) == VIR_OPND_SYMBOL && VIR_Symbol_GetKind(pSym) == VIR_SYM_UNIFORM)
            {
                if (VIR_Symbol_isImage(pSym) || VIR_Symbol_isImageT(pSym))
            {
                    bNeedInsertMov = gcvTRUE;

                    /* Check if we already generate a symbol for this uniform. */
                    if (vscHTBL_DirectTestAndGet(pWorkingSet, (void*)&pSym, (void**)&pVirRegSym))
                {
                        virRegSymId = VIR_Symbol_GetIndex(pVirRegSym);
                }
                else
                {
                        /* Copy the .yz channel of this uniform. */
                        errCode = _InitializeUniformCopy(pDuInfo,
                                                         pShader,
                                                         &virRegSymId,
                                                         pOpnd,
                                                         pSym);
                        ON_ERROR(errCode, "Initialize uniform copy.");

                        /* Update the hash. */
                        pVirRegSym = VIR_Shader_GetSymFromId(pShader, virRegSymId);
                        vscHTBL_DirectSet(pWorkingSet, (void*)&pSym, (void**)&pVirRegSym);
                    }
                    }
                    else
                    {
                    _ChangeUniformTypeToUvec3(pShader, pSym);
                    }
                }
            else if (VIR_Symbol_isInput(pSym) || VIR_Symbol_isOutput(pSym))
                {
                continue;
            }
            else
            {
                _GetLowerUpperVirRegSymId(pMM,
                                          pShader,
                                          pDuInfo,
                                          pWorkingSet,
                                          inst,
                                          pOpnd,
                                          &virRegSymId);
                bNeedInsertMov = gcvTRUE;
                    }

            /* If src0 is not mem-base uniform, insert following 3 movs, and cast src0 to new dst-of-mov
               1. mov from src0.x,
               2. mov from Y channel of mem-base-addr uniform (low-limit)
               2. mov from Z channel of mem-base-addr uniform (upper-limit) */
            if (bNeedInsertMov)
            {
                gcmASSERT(virRegSymId != VIR_INVALID_ID);

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
                VIR_Operand_SetTypeId(opnd, VIR_TYPE_UINT32);
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

                   mov new-temp-reg.yz, pMemBaseUniformReg.xxy
                */
                errCode = VIR_Function_AddInstructionBefore(func, VIR_OP_MOV, VIR_TYPE_UINT_X2, inst, gcvTRUE, &pNewInsertedInstYZ);
                ON_ERROR(errCode, "Add instruction before");

                /* dst */
                opnd = VIR_Inst_GetDest(pNewInsertedInstYZ);
                VIR_Operand_SetSymbol(opnd, func, newDstSymId);
                VIR_Operand_SetEnable(opnd, VIR_ENABLE_YZ);
                VIR_Operand_SetPrecision(opnd, VIR_Operand_GetPrecision(pOpnd));

                /* src */
                opnd = VIR_Inst_GetSource(pNewInsertedInstYZ, VIR_Operand_Src0);
                VIR_Operand_SetSymbol(opnd, func, virRegSymId);
                VIR_Operand_SetSwizzle(opnd, virmSWIZZLE(X, X, Y, Y));

                vscVIR_AddNewDef(pDuInfo,
                                 pNewInsertedInstYZ,
                                 newDstRegNo,
                                 1,
                                 VIR_ENABLE_YZ,
                                 VIR_HALF_CHANNEL_MASK_FULL,
                                 gcvNULL,
                                 gcvNULL);

                VIR_Operand_GetOperandInfo(pNewInsertedInstYZ, opnd, &operandInfo);
                vscVIR_AddNewUsageToDef(pDuInfo,
                                        VIR_ANY_DEF_INST,
                                        pNewInsertedInstYZ,
                                        opnd,
                                        gcvFALSE,
                                        operandInfo.u1.virRegInfo.virReg,
                                        1,
                                        VIR_ENABLE_YZ,
                                        VIR_HALF_CHANNEL_MASK_FULL,
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
                                        pNewInsertedInstYZ,
                                        inst,
                                        pOpnd,
                                        gcvFALSE,
                                        newDstRegNo,
                                        1,
                                        VIR_ENABLE_YZ,
                                        VIR_HALF_CHANNEL_MASK_FULL,
                                        gcvNULL);
            }

            /* Change the swizzle of src0 to XYZ */
            VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYZZ);

            bChanged = gcvTRUE;
        }
    }

    /* Dump the shader if changed. */
    if (bChanged && VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After out of bound check", pShader, gcvTRUE);
    }

OnError:
    /* Finalize the working set. */
    if (pWorkingSet)
    {
        vscHTBL_Reset(pWorkingSet);
    }

    return errCode;
}

void
_Inst_ChangeOpnd2HP(
    VIR_Instruction     *pInst,
    VIR_Operand         *pOpnd,
    gctBOOL             isDest,
    gctBOOL             skipLowP,
    gctBOOL             setDefRecursively,
    VIR_DEF_USAGE_INFO  *pDuInfo
    )
{
    VIR_USAGE_KEY       usageKey;
    VIR_DEF_KEY         defKey;
    gctUINT             usageIdx, defIdx, i;
    VIR_USAGE           *pUsage = gcvNULL;
    VIR_DEF*            pDef = gcvNULL;
    VIR_OperandKind     opndKind = VIR_Operand_GetOpKind(pOpnd);

    VIR_OperandInfo     operandInfo;
    VSC_DU_ITERATOR     duIter;
    VIR_DU_CHAIN_USAGE_NODE *pUsageNode;

    if (!(opndKind == VIR_OPND_VIRREG || opndKind == VIR_OPND_SYMBOL))
    {
        VIR_Operand_SetPrecision(pOpnd, VIR_PRECISION_HIGH);
        return;
    }

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
                    VIR_Instruction * pDefInst;
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
                    pDefInst = pDef->defKey.pDefInst;

                    if (pDefInst != VIR_INPUT_DEF_INST)
                    {
                        VIR_Operand_SetPrecision(VIR_Inst_GetDest(pDefInst),
                                                 VIR_PRECISION_HIGH);
                        if (setDefRecursively)
                        {
                            /* set def inst's source operands to highp */
                            gctUINT j;
                            for (j = 0; j < VIR_Inst_GetSrcNum(pDefInst); j++)
                            {
                                VIR_Operand *src = VIR_Inst_GetSource(pDefInst, j);
                                if (src && VIR_Operand_GetPrecision(src) != VIR_PRECISION_HIGH)
                                {
                                    _Inst_ChangeOpnd2HP(pDefInst, src, gcvFALSE, skipLowP, gcvTRUE, pDuInfo);
                                }
                            }
                        }
                    }

                    /* change def's uses to highp */
                    VSC_DU_ITERATOR_INIT(&duIter, &pDef->duChain);
                    pUsageNode = VSC_DU_ITERATOR_FIRST(&duIter);
                    for (; pUsageNode != gcvNULL; pUsageNode = VSC_DU_ITERATOR_NEXT(&duIter))
                    {
                        pUsage = GET_USAGE_BY_IDX(&pDuInfo->usageTable, pUsageNode->usageIdx);
                        if (!VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
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
                    if (!VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
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
            (VIR_OPCODE_isImgLd(opcode) && forceChange) ||
            (VIR_OPCODE_BITWISE(opcode) && forceChange))
        {
            return gcvTRUE;
        }

        if (opcode == VIR_OP_CSELECT &&
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
        if (opcode == VIR_OP_FRAC || opcode == VIR_OP_MOD)
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

        if (opcode == VIR_OP_CSELECT)
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
            VIR_OPCODE_isImgRelated(opcode)  ||
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

        if (opcode == VIR_OP_CONVERT)
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
        /* MOD and FRAC are precision sensitive, need to make the instruction and its
         * operands' def-use chain to high precision */
        if (opcode == VIR_OP_FRAC || opcode == VIR_OP_MOD)
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
                                _Inst_ChangeOpnd2HP(pDef->defKey.pDefInst, movSrcOpnd, gcvFALSE, gcvTRUE, gcvFALSE, pDuInfo);
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
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML | VSC_PASS_LEVEL_LL;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_AdjustPrecision)
{
    VIR_Shader*                    pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    /* currently only PS is dual16-able */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_FRAGMENT &&
        !VIR_Shader_IsVulkan(pShader) &&
        !VIR_Shader_IsDesktopGL(pShader))
    {
        return gcvTRUE;
    }

    return gcvFALSE;
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

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction *inst;
        gctUINT         i;
        gctBOOL         forceChange = gcvFALSE, skipLowp = gcvTRUE;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             inst != gcvNULL;
             inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

            /* bitwise operation: if any src is highp, the other src and dest has to be highp */
            if (VIR_OPCODE_BITWISE(opcode))
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

            if (VIR_OPCODE_isTexLd(opcode) || VIR_OPCODE_isImgLd(opcode))
            {
                VIR_Operand*    pSrc0Opnd = VIR_Inst_GetSource(inst, 0);

                if (VIR_Operand_GetPrecision(pSrc0Opnd) == VIR_PRECISION_HIGH)
                {
                    forceChange = gcvTRUE;
                }
            }

            /* HW has restriction on select instruction - only one instruction type for comparison and type conversion
                we may have problem when dest and src0 type is different. For example,
                select fp32, int32, fp16, fp16,
                we need to set it instruction type to int32, thus it will use integer conversion
                to convert fp16 to fp32, which is wrong. Thus, we make dest and all src to be highp if
                there is one highp source or dest */
            if (opcode == VIR_OP_CSELECT)
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
                        VIR_TypeId ty1 = VIR_Operand_GetTypeId(VIR_Inst_GetSource(inst, 0));

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

                            VIR_Operand_GetOperandInfo(inst, VIR_Inst_GetDest(inst), &operandInfo);

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

                                    if (!VIR_IS_OUTPUT_USAGE_INST(pUsage->usageKey.pUsageInst))
                                    {
                                        ty0 = VIR_Operand_GetTypeId(pUsage->usageKey.pOperand);
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

            if (VIR_OPCODE_isTexLd(opcode) &&
                (VIR_GetTypeFlag(VIR_Operand_GetTypeId(VIR_Inst_GetDest(inst))) & VIR_TYFLAG_ISINTEGER) &&
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
                    gctBOOL setDefRecursively = (opcode == VIR_OP_MOD);
                    _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetSource(inst, i), gcvFALSE, skipLowp, setDefRecursively, pDuInfo);
                }
            }

            if (_Inst_RequireHPDest(inst, forceChange))
            {
                _Inst_ChangeOpnd2HP(inst, VIR_Inst_GetDest(inst), gcvTRUE, skipLowp, gcvFALSE, pDuInfo);
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

    if (VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "After Adjust Precision", pShader, gcvTRUE);
    }

    return errCode;
}


static
VIR_Operand * _vscVIR_FindParentImgOperandFromIndex(VIR_Instruction* inst, VIR_Operand* index, gctUINT channel)
{
    VIR_Instruction* prev;

    for(prev = VIR_Inst_GetPrev(inst); prev; prev = VIR_Inst_GetPrev(prev))
    {
        VIR_Operand* dest;

        dest = VIR_Inst_GetDest(prev);
        if(dest &&
           VIR_Operand_GetSymbol(dest)->u1.vregIndex == VIR_Operand_GetSymbol(index)->u1.vregIndex &&
           VIR_Enable_Covers(VIR_Operand_GetEnable(dest), 1 << channel))
        {
            VIR_OpCode op = VIR_Inst_GetOpcode(prev);

            if(op == VIR_OP_LDARR)
            {
                index = VIR_Inst_GetSource(prev, 1);
                return _vscVIR_FindParentImgOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), 0));
            }
            else if(op == VIR_OP_MOVA)
            {
                index = VIR_Inst_GetSource(prev, 0);
                return _vscVIR_FindParentImgOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), channel));
            }
            else if(op == VIR_OP_GET_SAMPLER_IDX)
            {
                return VIR_Inst_GetSource(prev, 0);
            }
            else if (op == VIR_OP_MOV)
            {
                index = VIR_Inst_GetSource(prev, 0);

                if (VIR_Operand_isSymbol(index) && !VIR_Symbol_isVreg(VIR_Operand_GetSymbol(index)))
                {
                    return index;
                }
                else if(VIR_Operand_isImm(index))
                {
                    return index;
                }
                else
                {
                    return _vscVIR_FindParentImgOperandFromIndex(prev, index, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(index), channel));
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

    /* If the sampler sym is not a sampler, then find the parent. */
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
                gcmASSERT(VIR_Inst_GetOpcode(prev) == VIR_OP_MOV ||
                          VIR_Inst_GetOpcode(prev) == VIR_OP_LDARR);

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

        /*if met another get sample idx, stop searching nextInst*/
        if (opcode == VIR_OP_GET_SAMPLER_IDX)
        {
            return errCode;
        }
        src = VIR_Inst_GetSource(pNextInst, srcIdx);
        if (src == gcvNULL || !VIR_Operand_isSymbol(src))
        {
            continue;
        }
        srcSym = VIR_Operand_GetSymbol(src);

        /* If this is a baseSampler+offset, we need to check the offset. */
        if (VIR_Symbol_GetIndex(srcSym) == VIR_Shader_GetBaseSamplerId(pShader) &&
            VIR_Operand_GetRelAddrMode(src) != VIR_INDEXED_NONE)
        {
            srcSym = VIR_Shader_GetSymFromId(pShader, VIR_Operand_GetRelIndexing(src));
        }
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

static VIR_Symbol*
_getImageSymPairedWithSamplerValue(
    IN VIR_Shader *pShader,
    IN VIR_Symbol*          imageSym,
    IN VIR_Symbol*          samplerSym,
    IN VSC_SamplerValue     samplerValue
    )
{
    VIR_Uniform *   imageUniform;
    VIR_Type *      imageType = VIR_Symbol_GetType(imageSym);
    VIR_TypeId      imageTypeId = VIR_Type_GetIndex(imageType);
    VIR_Symbol*     origImageSym = imageSym;

    if (VIR_TypeId_isImage(imageTypeId))
    {
        gcmASSERT(VIR_TypeId_isImage(imageTypeId) && samplerValue != VSC_IMG_SAMPLER_INVALID_VALUE);
    }

    /* loop through the image tandem chain to find if the pair existed or not
     * if not, create a image symbol paired with sampler value */
    for (; imageSym != gcvNULL; )
    {
        VIR_SymId nextTandemSymId;
        imageUniform = VIR_Symbol_GetImage(imageSym);
        /* check if the uniform has the same sampler value assigned, or if value is unknown they
         * are using the same sampler symbol */
        if (VIR_Uniform_GetImageSamplerValue(imageUniform) == samplerValue &&
            ((samplerValue != VSC_IMG_SAMPLER_UNKNOWN_VALUE) ||
             (samplerSym && VIR_Symbol_GetIndex(samplerSym) == VIR_Uniform_GetImageSamplerSymId(imageUniform))))
        {
            return imageSym;
        }
        else if (VIR_Uniform_GetImageSamplerValue(imageUniform) == VSC_IMG_SAMPLER_INVALID_VALUE)
        {
            /* the image uniform is first time been paired with sampler */
            VIR_Uniform_SetImageSamplerValue(imageUniform, samplerValue);
            VIR_Uniform_SetImageSamplerSymId(imageUniform, samplerSym ? VIR_Symbol_GetIndex(samplerSym)
                                                                      : VIR_INVALID_ID);
            VIR_Uniform_SetImageNextTandemSymId(imageUniform, VIR_INVALID_ID);
            VIR_Uniform_SetImageSymId(imageUniform, VIR_Symbol_GetIndex(origImageSym));
            return imageSym;
        }
        /* check the next tandem symbol for the image */
        nextTandemSymId = VIR_Uniform_GetImageNextTandemSymId(imageUniform);
        if (VIR_Id_isInvalid(nextTandemSymId))
            {
            /* no same value sampler for this image found, create a new image symbol and
             * corresponding image uniform which has the new sampler value paired to it
             *  <imageSym, samplerValue>, and tandem it to previous image uniform
             */
            VIR_Symbol*     newImageSym;
            VIR_Uniform*    newImageUniform;
            VIR_NameId      nameId;
            VIR_SymId       newImageSymId = VIR_INVALID_ID;
            gctCHAR         name[256] = "#";

            /* create a new name */
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, origImageSym));
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$");
            if(samplerSym)
                {
                gcoOS_StrCatSafe(name, gcmSIZEOF(name), VIR_Shader_GetSymNameString(pShader, samplerSym));
            }
            else
            {
                gcoOS_PrintStrSafe(name+strlen(name), 127 - strlen(name), gcvNULL, "%x", samplerValue);
                }
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "$glImage");
            ON_ERROR(VIR_Shader_AddString(pShader, name, &nameId), "Failed to add name %s", name);

            /* add image symbol with the new name and proper type etc. info */
           ON_ERROR(VIR_Shader_AddSymbol(pShader,
                                         VIR_SYM_IMAGE_T,
                                         nameId,
                                         imageType,
                                         VIR_Symbol_GetStorageClass(imageSym),
                                         &newImageSymId), "Failed to add new image symbol");
            newImageSym = VIR_Shader_GetSymFromId(pShader, newImageSymId);
            VIR_Symbol_SetFlag(newImageSym, VIR_SYMFLAG_COMPILER_GEN);
            VIR_Symbol_SetPrecision(newImageSym, VIR_Symbol_GetPrecision(imageSym));
            newImageUniform = VIR_Symbol_GetImage(newImageSym);
            VIR_Symbol_SetUniformKind(newImageSym, VIR_UNIFORM_GL_IMAGE_FOR_IMAGE_T);
            VIR_Symbol_SetAddrSpace(newImageSym, VIR_AS_CONSTANT);
            VIR_Symbol_SetTyQualifier(newImageSym, VIR_TYQUAL_CONST);

            /* <image, sampler> pair info */
            VIR_Uniform_SetImageSymId(newImageUniform, VIR_Symbol_GetIndex(origImageSym));
            VIR_Uniform_SetImageSamplerValue(newImageUniform, samplerValue);
            VIR_Uniform_SetImageSamplerSymId(newImageUniform, samplerSym ? VIR_Symbol_GetIndex(samplerSym)
                                                                         : VIR_INVALID_ID);
            VIR_Uniform_SetImageLibFuncName(newImageUniform, gcvNULL);

            /* chain newImageSym to the previous imageSym */
            VIR_Uniform_SetImageNextTandemSymId(imageUniform, newImageSymId);
            VIR_Uniform_SetImageNextTandemSymId(newImageUniform, VIR_INVALID_ID);
            return newImageSym;
            }
        else
        {
            imageSym = VIR_Shader_GetSymFromId(pShader, nextTandemSymId);
        }
    }
OnError:

    return gcvNULL;
}

/* check if the sampler value used by the image read instruciton is using
 * the same sampler value as assigned in the image uniform it accessed,
 * if it is, no action is needed
 * otherwise, check the next tandem image uniform,
 *     if the same sampler value is used,
 *        change the instruction to use that image uniform
 *     otherwise continue the search untill found one or to the end
 *    if non image uniform with the same sampler value is found
 *    then create a new image uniform with that sampler value
 */
void
_vscGenerateNewImageUniformIfNeeded(
    VSC_SH_PASS_WORKER  *pPassWorker,
    VIR_Instruction     *pInst
    )
{
    VIR_Operand*    imageSrc;
    VIR_Operand*    samplerSrc;
    VIR_Operand*    parentSrc = gcvNULL;
    VIR_Symbol*     imageSym;
    VIR_Symbol*     samplerSym = gcvNULL;
    VSC_SamplerValue  samplerTValue = VSC_IMG_SAMPLER_INVALID_VALUE;  /* default value if the sampler is set by kernel arg */
    VIR_Symbol*     newImageSym;
    VIR_Shader *    pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_OpCode      opcode = VIR_Inst_GetOpcode(pInst);
    gctBOOL         coordIntType = VIR_TypeId_isInteger(VIR_Operand_GetTypeId(VIR_Inst_GetSource(pInst, 1)));

    gcmASSERT(VIR_OPCODE_isImgLd(opcode));

    /* get image uniform and sampler info used by the inst */
    imageSrc = VIR_Inst_GetSource(pInst, 0);  /* image always the fisrt operand*/
    gcmASSERT(VIR_Operand_isSymbol(imageSrc));

    imageSym = VIR_Operand_GetSymbol(imageSrc);

    if (!(VIR_Symbol_isImageT(imageSym) || VIR_Symbol_isImage(imageSym)))
    {
        parentSrc = _vscVIR_FindParentImgOperandFromIndex(pInst, imageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(imageSrc), 0));
        imageSym = VIR_Operand_GetSymbol(parentSrc);
    }
    gcmASSERT(imageSym && (VIR_Symbol_isImageT(imageSym)|| VIR_Symbol_isImage(imageSym)));

    /* sampler info */
    samplerSrc = VIR_OPCODE_isImgRead(opcode) ? VIR_Inst_GetSource(pInst, 2)  /* source 2 for IMG_READ */
                                               : VIR_Inst_GetSource(pInst, 3) ; /* source 3 for IMG_LOAD*/
    /* VX_IMG_xxx instructions only support samplerless image read*/
    if (VIR_OPCODE_isVX(opcode))
    {
            samplerSym = gcvNULL;
            samplerTValue = VSC_IMG_SAMPLER_DEFAULT_VALUE;
    }
    else if (samplerSrc && !VIR_Operand_isUndef(samplerSrc))
    {
        if(VIR_Operand_isSymbol(samplerSrc))
        {
            VIR_Uniform * sampler = gcvNULL;
            samplerSym = VIR_Operand_GetSymbol(samplerSrc);
            if(VIR_Symbol_isSamplerT(samplerSym))
            {
                sampler = VIR_Symbol_GetSampler(samplerSym);
            }
            else
            {
                parentSrc = _vscVIR_FindParentImgOperandFromIndex(pInst, samplerSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerSrc), 0));
                if(VIR_Operand_isImm(parentSrc))
                {
                    samplerSym = gcvNULL;
                    samplerTValue = VIR_Operand_GetImmediateUint(parentSrc);
                }
                else
                {
                    samplerSym = VIR_Operand_GetSymbol(parentSrc);
                    sampler = VIR_Symbol_GetSampler(samplerSym);
                }
            }
            if (sampler)
            {
                /* get the sampler value stored in the sampler uniform */
                samplerTValue = VIR_Uniform_GetSamplerValue(sampler);
            }
        }
        else
        {
            gcmASSERT(VIR_Operand_isImm(samplerSrc));
            samplerSym = gcvNULL;
            samplerTValue = VIR_Operand_GetImmediateUint(samplerSrc);
        }
    }

    if (samplerTValue == VSC_IMG_SAMPLER_INVALID_VALUE)
    {
        /* samplerless or unknown sampler value, use default sampler value*/
        samplerTValue = VSC_IMG_SAMPLER_DEFAULT_VALUE;
    }

    /* add int coorinate info to the sampler value */
    if (coordIntType)
    {
        samplerTValue |= VSC_IMG_SAMPLER_INT_COORDS_TRUE;
    }

    newImageSym = _getImageSymPairedWithSamplerValue(pShader, imageSym, samplerSym, samplerTValue);

    /* update the inst to use new image sym if it is different with the one used in src0 */
    if (imageSym != newImageSym)
    {
        VIR_Operand_SetSym(imageSrc, newImageSym);
    }
    return;
}

typedef union __ImgSampler_KeyState{
    struct {
       gctUINT baseLib : 2;         /* 2 = ceil(log(VSC_OCLImgLibKind_Counts)/log(2))*/
       gctUINT imgType : 3;         /* 3 = ceil(log(VIR_IMG_TY_COUNT)/log(2))*/
       gctUINT channelDataType : 4; /* 4 = ceil(log(VIR_IMG_CT_COUNT)/log(2))*/
       gctUINT channelOrder : 4;    /* 4 = ceil(log(VIR_IMG_CO_COUNT)/log(2))*/
       gctUINT normalizedCoord : 1; /* 0: NORMALIZED_COORDS_FALSE, 1: NORMALIZED_COORDS_TRUE */
       gctUINT filterMode : 1;      /* 0: FILTER_NEAREST, 1: FILTER_LINEAR */
       gctUINT addressMode : 3;     /* 3 = ceil(log(VSC_IMG_SAMPLER_ADDRESS_COUNT)/log(2))*/
       gctUINT coordType : 1;       /*0: float coord, 1 integer coord*/
       gctUINT imgDataType : 2;     /* 2 = ceil(log(3)/log(2))*/
       gctUINT imgAccessType : 1;   /* 0: image for write; 1:image for read */
    }u;
    gctUINT data;
}ImgSamplerKeyState;

/* struct ImgSamplerKeyTable and struct ImgReadNameStr should have the same index value and should match with each other for the corresponding field */
struct ImgSamplerKeyTable{
    gctUINT imgType[VIR_IMG_TY_COUNT];
    gctUINT imgChannelDataType[VIR_IMG_CT_COUNT];
    gctUINT imgChannelOrder[VIR_IMG_CO_COUNT];
    gctUINT normalizedCoord[2];
    gctUINT filterMode[2];
    gctUINT addressMode[VSC_IMG_SAMPLER_ADDRESS_COUNT];
    gctUINT coordType[2];
    gctUINT imgDataType[3];
}imgSamplerKeyWordsTable[VSC_OCLImgLibKind_Counts] =
{
    /* VSC_OCLImgLibKind_UseLoadStore */
    {
        {VIR_IMG_TY_BUFFER, VIR_IMG_TY_2D, VIR_IMG_TY_3D, VIR_IMG_TY_2D_ARRAY, VIR_IMG_TY_1D, VIR_IMG_TY_1D_ARRAY, VIR_IMG_TY_1D_BUFFER},
        {VIR_IMG_CT_SNORM_INT8, VIR_IMG_CT_SNORM_INT16, VIR_IMG_CT_UNORM_INT8, VIR_IMG_CT_UNORM_INT16, VIR_IMG_CT_UNORM_SHORT_565, VIR_IMG_CT_UNORM_SHORT_555, VIR_IMG_CT_UNORM_INT_101010,
         VIR_IMG_CT_SIGNED_INT8, VIR_IMG_CT_SIGNED_INT16, VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT8, VIR_IMG_CT_UNSIGNED_INT16, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_HALF_FLOAT, VIR_IMG_CT_FLOAT},
        {VIR_IMG_CO_R, VIR_IMG_CO_A, VIR_IMG_CO_RG, VIR_IMG_CO_RA, VIR_IMG_CO_RGB, VIR_IMG_CO_RGBA, VIR_IMG_CO_BGRA, VIR_IMG_CO_ARGB,
         VIR_IMG_CO_INTENSITY, VIR_IMG_CO_LUMINANCE, VIR_IMG_CO_Rx, VIR_IMG_CO_RGx, VIR_IMG_CO_RGBx},
        {0, 1},
        {0, 1},
        {VSC_IMG_SAMPLER_ADDRESS_NONE, VSC_IMG_SAMPLER_ADDRESS_CLAMP_TO_EDGE, VSC_IMG_SAMPLER_ADDRESS_CLAMP, VSC_IMG_SAMPLER_ADDRESS_REPEAT, VSC_IMG_SAMPLER_ADDRESS_MIRRORED_REPEAT},
        {0, 1},
        {VSC_ImageValueFloat, VSC_ImageValueInt, VSC_ImageValueUint},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldU */
    {
        {VIR_IMG_TY_2D, VIR_IMG_TY_2D, VIR_IMG_TY_3D, VIR_IMG_TY_3D, VIR_IMG_TY_2D, VIR_IMG_TY_2D, VIR_IMG_TY_2D},
        {VIR_IMG_CT_SNORM_INT8, VIR_IMG_CT_SNORM_INT16, VIR_IMG_CT_UNORM_INT8, VIR_IMG_CT_UNORM_INT16, VIR_IMG_CT_UNORM_SHORT_565, VIR_IMG_CT_UNORM_SHORT_555, VIR_IMG_CT_UNORM_INT_101010,
         VIR_IMG_CT_SIGNED_INT8, VIR_IMG_CT_SIGNED_INT16, VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT8, VIR_IMG_CT_UNSIGNED_INT16, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_HALF_FLOAT, VIR_IMG_CT_FLOAT},
        {VIR_IMG_CO_R, VIR_IMG_CO_A, VIR_IMG_CO_RG, VIR_IMG_CO_RA, VIR_IMG_CO_RGB, VIR_IMG_CO_RGBA, VIR_IMG_CO_BGRA, VIR_IMG_CO_ARGB,
         VIR_IMG_CO_INTENSITY, VIR_IMG_CO_LUMINANCE, VIR_IMG_CO_Rx, VIR_IMG_CO_RGx, VIR_IMG_CO_RGBx},
        {0, 1},
        {0, 1},
        {VSC_IMG_SAMPLER_ADDRESS_NONE, VSC_IMG_SAMPLER_ADDRESS_CLAMP_TO_EDGE, VSC_IMG_SAMPLER_ADDRESS_CLAMP, VSC_IMG_SAMPLER_ADDRESS_REPEAT, VSC_IMG_SAMPLER_ADDRESS_MIRRORED_REPEAT},
        {0, 1},
        {VSC_ImageValueFloat, VSC_ImageValueInt, VSC_ImageValueUint},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldUXY */
    {
        {VIR_IMG_TY_2D, VIR_IMG_TY_2D, VIR_IMG_TY_3D, VIR_IMG_TY_3D, VIR_IMG_TY_2D, VIR_IMG_TY_2D, VIR_IMG_TY_2D},
        {VIR_IMG_CT_SNORM_INT8, VIR_IMG_CT_SNORM_INT16, VIR_IMG_CT_UNORM_INT8, VIR_IMG_CT_UNORM_INT16, VIR_IMG_CT_UNORM_SHORT_565, VIR_IMG_CT_UNORM_SHORT_555, VIR_IMG_CT_UNORM_INT_101010,
         VIR_IMG_CT_SIGNED_INT8, VIR_IMG_CT_SIGNED_INT16, VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT8, VIR_IMG_CT_UNSIGNED_INT16, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_HALF_FLOAT, VIR_IMG_CT_FLOAT},
        {VIR_IMG_CO_R, VIR_IMG_CO_A, VIR_IMG_CO_RG, VIR_IMG_CO_RA, VIR_IMG_CO_RGB, VIR_IMG_CO_RGBA, VIR_IMG_CO_BGRA, VIR_IMG_CO_ARGB,
         VIR_IMG_CO_INTENSITY, VIR_IMG_CO_LUMINANCE, VIR_IMG_CO_Rx, VIR_IMG_CO_RGx, VIR_IMG_CO_RGBx},
        {0, 1},
        {0, 1},
        {VSC_IMG_SAMPLER_ADDRESS_NONE, VSC_IMG_SAMPLER_ADDRESS_CLAMP_TO_EDGE, VSC_IMG_SAMPLER_ADDRESS_CLAMP, VSC_IMG_SAMPLER_ADDRESS_REPEAT, VSC_IMG_SAMPLER_ADDRESS_MIRRORED_REPEAT},
        {0, 1},
        {VSC_ImageValueFloat, VSC_ImageValueInt, VSC_ImageValueUint},
    },
    /* VSC_OCLImgLibKind_UseImgLoadVIP */
    {
        {VIR_IMG_TY_2D, VIR_IMG_TY_2D, VIR_IMG_TY_3D, VIR_IMG_TY_2D_ARRAY, VIR_IMG_TY_1D, VIR_IMG_TY_1D_ARRAY, VIR_IMG_TY_1D},
        {VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT, VIR_IMG_CT_FLOAT,
         VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_SIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_UNSIGNED_INT32, VIR_IMG_CT_HALF_FLOAT, VIR_IMG_CT_FLOAT},
        {VIR_IMG_CO_COUNT, VIR_IMG_CO_A, VIR_IMG_CO_RG, VIR_IMG_CO_RA, VIR_IMG_CO_RGB, VIR_IMG_CO_COUNT, VIR_IMG_CO_COUNT, VIR_IMG_CO_ARGB,
         VIR_IMG_CO_INTENSITY, VIR_IMG_CO_LUMINANCE, VIR_IMG_CO_Rx, VIR_IMG_CO_RGx, VIR_IMG_CO_RGBx},
        {0, 1},
        {0, 1},
        {VSC_IMG_SAMPLER_ADDRESS_NONE, VSC_IMG_SAMPLER_ADDRESS_CLAMP_TO_EDGE, VSC_IMG_SAMPLER_ADDRESS_CLAMP, VSC_IMG_SAMPLER_ADDRESS_REPEAT, VSC_IMG_SAMPLER_ADDRESS_MIRRORED_REPEAT},
        {0, 1},
        {VSC_ImageValueUint, VSC_ImageValueUint, VSC_ImageValueUint},
    },
};

struct ImgReadNameStr {
    gctSTRING baseName;
    gctSTRING imgTypeStr[VIR_IMG_TY_COUNT];                         /* 0: buffer, 1: 2d, 2: 3d, 3: 2darray, 4: 1d, 5: 1darray, 6:1dbuffer */
    gctSTRING imgChannelTypeStr[VIR_IMG_CT_COUNT];
    gctSTRING imgChannelOrderStr[VIR_IMG_CO_COUNT];
    gctSTRING samplerNormalizedCoordStr[2];                         /* 0: NORMALIZED_COORDS_FALSE, 1: NORMALIZED_COORDS_TRUE */
    gctSTRING samplerFilterStr[2];                                  /* 0: FILTER_NEAREST, 1: FILTER_LINEAR */
    gctSTRING samplerAddressModeStr[VSC_IMG_SAMPLER_ADDRESS_COUNT]; /* 0: NODE, 1: EDGE, 2: CLAMP, 3: REPEAT, 4: MIRROR */
    gctSTRING coordType[2];                                         /*0: float coord, 1 integer coord*/
    gctSTRING imageValueType[3];                                    /* match imageValueType in VSC_ImageDesc */
} imgReadNamesInfo[VSC_OCLImgLibKind_Counts] =
{
    /* VSC_OCLImgLibKind_UseLoadStore */
    {
        "_read_image_with_load",
        {"_buffer", "_2d", "_3d", "_2darray", "_1d", "_1darray", "_1dbuffer"},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"_unnormCoord", "_normCoord"},
        {"_nearest", "_linear"},
        {"_none", "_clamp", "_border", "_wrap", "_mirror"},
        {"_floatcoord", "_intcoord"},
        {"_retf", "_reti", "_retui"},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldU */
    {
        "_read_image_with_texldu",
        {"", "", "_3d", "_3d", "", "", ""},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"", "_normCoord"},
        {"_nearest", "_linear"},
        {"", "_clamp", "_border", "_wrap", "_mirror"},
        {"", ""},
        {"", "", ""},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldUXY */
    {
        "_read_image_with_texlduxy",
        {"", "", "_3d", "_3d", "", "", ""},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"", "_normCoord"},
        {"_nearest", "_linear"},
        {"", "_clamp", "_border", "_wrap", "_mirror"},
        {"", ""},
        {"", "", ""},
    },
    /* VSC_OCLImgLibKind_UseImgLoadVIP */
    {
        "_read_image_width_imgread",
        {"", "_2d", "_3d", "_2darray", "_1d", "_1darray", "_1d"},
        {"_float", "_float", "_float", "_float", "_float", "_float", "_float",
         "_int", "_int", "_int", "_uint", "_uint", "_uint", "_float", "_float"},
        {"", "_A", "_RG", "_RA", "_RGB", "", "", "",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"_unnormCoord", "_normCoord"},
        {"_nearest", "_linear"},
        {"_none", "_clamp", "_border", "_wrap", "_mirror"},
        {"_floatcoord", "_intcoord"},
        {"", "", ""},
    },
};

struct ImgWriteNameStr {
    gctSTRING baseName;
    gctSTRING imgTypeStr[VIR_IMG_TY_COUNT];                         /* 0: buffer, 1: 2d, 2: 3d, 3: 2darray, 4: 1d, 5: 1darray, 6:1dbuffer */
    gctSTRING imgChannelTypeStr[VIR_IMG_CT_COUNT];
    gctSTRING imgChannelOrderStr[VIR_IMG_CO_COUNT];
    gctSTRING imageValueType[3];                                    /* match imageValueType in VSC_ImageDesc */
} imgWriteNamesInfo[VSC_OCLImgLibKind_Counts] =
{
    /* VSC_OCLImgLibKind_UseLoadStore */
    {
        "_write_image_with_store",
        {"_buffer", "_2d", "_3d", "_2darray", "_1d", "_1darray", "_1dbuffer"},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"_retf", "_reti", "_retui"},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldU */
    {
        "_write_image_with_img_store",
        {"", "", "_3d", "_3d", "", "", ""},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"", "", ""},
    },
    /* VSC_OCLImgLibKind_UseImgLoadTexldUXY */
    {
        "_write_image_with_img_store",
        {"", "", "_3d", "_3d", "", "", ""},
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"},
        {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"", "", ""},
    },
    /* VSC_OCLImgLibKind_UseImgLoadVIP */
    {
        "_write_image_with_imgwrite",
        {"", "_2d", "_3d", "_3d", "_1d", "_2d", "_1d"},
        {"_float", "_float", "_float", "_float", "_float", "_float", "_float",
         "_int", "_int", "_int", "_uint", "_uint", "_uint", "_float", "_float"},
        {"", "_A", "_RG", "_RA", "_RGB", "", "", "",
         "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"},
        {"", "", ""},
    },
};

/* For given image descriptorfor HW cfg, do we
 * need to do recompilation for the image write ?  */
gctBOOL
vscImageWriteNeedLibFuncForHWCfg(
    void *                  pImageDesc,
    VSC_HW_CONFIG *         pHwCfg,
    VSC_OCLImgLibKind *     pImgLibKind, /* the image lib kind to be used */
    gctUINT *               KeyofImgSampler
    )
{
    gctBOOL doRecompile = gcvFALSE;
    ImgSamplerKeyState keyState;
    VSC_ImageDesc * ImageDescHandle = (VSC_ImageDesc *)pImageDesc;
    struct ImgSamplerKeyTable * tmpImgSamplerKeyTable;

    /* initialize the value of key state to 0 */
    keyState.data = 0;

    if (ImageDescHandle->sd.imageType != 0)
    {
        /* check which base lib to use: if the HW supports Image write, use HW instruction,
         * otherwise to store for image write*/
        if (!pHwCfg->hwFeatureFlags.supportImgAddr)
        {
            keyState.u.baseLib = VSC_OCLImgLibKind_UseLoadStore;
            if (pImgLibKind)
            {
                *pImgLibKind = VSC_OCLImgLibKind_UseLoadStore;
            }
            doRecompile = gcvTRUE;
        }

        tmpImgSamplerKeyTable = &imgSamplerKeyWordsTable[keyState.u.baseLib];
        /* check the state of image-sampler base on the base lib used for the pair */
        keyState.u.imgType = tmpImgSamplerKeyTable->imgType[VSC_CLImageType_To_VIRImageType(ImageDescHandle->sd.imageType)];
        keyState.u.imgDataType = tmpImgSamplerKeyTable->imgDataType[ImageDescHandle->sd.imageValueType];
        keyState.u.channelDataType = tmpImgSamplerKeyTable->imgChannelDataType[VSC_CLChannelDataType_To_VIRChannelDataType(ImageDescHandle->sd.channelDataType)];
        keyState.u.channelOrder = tmpImgSamplerKeyTable->imgChannelOrder[VSC_CLChannelOrder_To_VIRChannelOrder(ImageDescHandle->sd.channelOrder)];
        keyState.u.imgAccessType = 0; /* for kernel write*/
    }

    if(KeyofImgSampler)
    {
        *KeyofImgSampler = keyState.data;
    }
    return doRecompile;
}

/* For given image descriptor and sampler value for HW cfg, do we
 * need to do recompilation for the image read ?*/
gctBOOL
vscImageSamplerNeedLibFuncForHWCfg(
    void *                  pImageDesc,
    gctUINT                 imageSamplerValue,
    VSC_HW_CONFIG *         pHwCfg,
    VSC_OCLImgLibKind *     pImgLibKind, /* the image lib kind to be used */
    gctBOOL *               UseTexld, /* set true if need to use texld for image_read */
    gctUINT *               KeyofImgSampler
    )
{
    gctBOOL             supportedByHWInst = gcvFALSE;
    ImgSamplerKeyState  keyState;
    VSC_ImageDesc *     ImageDescHandle = (VSC_ImageDesc *)pImageDesc;
    struct ImgSamplerKeyTable * tmpImgSamplerKeyTable;
    VIR_IMG_Type        imageType = VSC_CLImageType_To_VIRImageType(ImageDescHandle->sd.imageType);
    vscImageValueType   imageValueType = (vscImageValueType)ImageDescHandle->sd.imageValueType;
    VIR_IMG_ChannelDataType channelType = VSC_CLChannelDataType_To_VIRChannelDataType(ImageDescHandle->sd.channelDataType);
    VIR_IMG_ChannelOrder channelOrder = VSC_CLChannelOrder_To_VIRChannelOrder(ImageDescHandle->sd.channelOrder);
    gctBOOL             isCoordsIntType = VIR_IMG_isSamplerIntCoords(imageSamplerValue);
    gctBOOL             isLinearFilterMode = VIR_IMG_isSamplerLinearFilter(imageSamplerValue);
    gctBOOL             isNormalizedCoords = VIR_IMG_isSamplerNormalizedCoords(imageSamplerValue);
    VSC_SamplerValue    addressMode = VIR_IMG_GetSamplerAddressMode(imageSamplerValue);
    VSC_OCLImgLibKind   libKind = VSC_OCLImgLibKind_UseLoadStore;

    /* initialize the value of key state to 0 */
    keyState.data = 0;

    if(ImageDescHandle->sd.imageType != 0)
    {
        /* check which base lib to use */
        libKind = vscGetOCLImgLibKindForHWCfg(pHwCfg);
        keyState.u.baseLib = libKind;
        tmpImgSamplerKeyTable = &imgSamplerKeyWordsTable[keyState.u.baseLib];
        /* check the state of image-sampler base on the base lib used for the pair */
        keyState.u.imgType = tmpImgSamplerKeyTable->imgType[imageType];
        keyState.u.imgDataType = tmpImgSamplerKeyTable->imgDataType[imageValueType];
        keyState.u.channelDataType = tmpImgSamplerKeyTable->imgChannelDataType[channelType];
        keyState.u.channelOrder = tmpImgSamplerKeyTable->imgChannelOrder[channelOrder];
        keyState.u.filterMode = tmpImgSamplerKeyTable->filterMode[isLinearFilterMode];
        keyState.u.normalizedCoord = tmpImgSamplerKeyTable->normalizedCoord[isNormalizedCoords];
        keyState.u.coordType = tmpImgSamplerKeyTable->coordType[isCoordsIntType];
        keyState.u.addressMode = tmpImgSamplerKeyTable->addressMode[addressMode];
        keyState.u.imgAccessType = 1; /* for kernel read*/
    }
    switch (libKind) {
    case VSC_OCLImgLibKind_UseImgLoadVIP:       /* only use img_load/img_load_3d inst. */
        if (!isLinearFilterMode &&
            imageType == VIR_IMG_TY_2D &&      /* ?? other image type supported?? */
            (addressMode == VSC_IMG_SAMPLER_ADDRESS_NONE ||
             addressMode == VSC_IMG_SAMPLER_ADDRESS_CLAMP ||
             addressMode == VSC_IMG_SAMPLER_ADDRESS_CLAMP_TO_EDGE) &&
            !isNormalizedCoords &&
            isCoordsIntType &&
            pHwCfg->hwFeatureFlags.supportImgLDSTClamp)
        {
            supportedByHWInst = gcvTRUE;  /* use img_load or f2i(coord) + img_load for float type coordinate */
        }
        break;
    case VSC_OCLImgLibKind_UseImgLoadTexldUXY:  /* use texld_u_x_y */
        if ((channelType == VIR_IMG_CT_SNORM_INT8 ||
             channelType == VIR_IMG_CT_UNORM_INT8 ||
             channelType == VIR_IMG_CT_SIGNED_INT8 ||
             channelType == VIR_IMG_CT_SIGNED_INT16 ||
             channelType == VIR_IMG_CT_UNSIGNED_INT8 ||
             channelType == VIR_IMG_CT_UNSIGNED_INT16 ||
             channelType == VIR_IMG_CT_HALF_FLOAT)      )
        {
            supportedByHWInst = gcvTRUE;  /* use texld_u_x_y for 16 bit channel data type,
                                           * SNORM16 and UNORM16 are not supported */
        }
        break;
    case VSC_OCLImgLibKind_UseImgLoadTexldU:    /* use load for now  */
#if _SUPPORT_TEDLX_U_IMG_LIB
        if ((channelType == VIR_IMG_CT_SNORM_INT8 ||
             channelType == VIR_IMG_CT_UNORM_INT8 ||
             channelType == VIR_IMG_CT_SIGNED_INT8 ||
             channelType == VIR_IMG_CT_SIGNED_INT16 ||
             channelType == VIR_IMG_CT_UNSIGNED_INT8 ||
             channelType == VIR_IMG_CT_UNSIGNED_INT16 ||
             channelType == VIR_IMG_CT_HALF_FLOAT)      )
        {
            supportedByHWInst = gcvTRUE;  /* use texld_u_x_y for 16 bit channel data type,
                                           * SNORM16 and UNORM16 are not supported */
        }
        break;
#else
        /* fall through */
#endif
    case VSC_OCLImgLibKind_UseLoadStore:        /* no HW inst support, need to use load */
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if(KeyofImgSampler)
    {
        *KeyofImgSampler = keyState.data;
    }
    if (pImgLibKind)
    {
        *pImgLibKind = libKind;
    }
    return !supportedByHWInst;
}

gceSTATUS
vscConstructImageReadLibFuncName(
    VSC_ImageDesc *         ImageDescHandle, /* VSC_ImageDesc */
    gctUINT                 SamplerValue, /* VSC_SamplerValue */
    VSC_HW_CONFIG *         pHwCfg,
    gctSTRING *             pLibFuncName, /* returned lib function name if needed */
    VSC_OCLImgLibKind *     pImgLibKind, /* the image lib kind to be used */
    gctBOOL *               UseTexld                /* set true if need to use texld for image_read */
)
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VSC_ImageDesc *             pImageDesc = (VSC_ImageDesc *)ImageDescHandle;
    VSC_SamplerValue            imageSamplerValue = (VSC_SamplerValue)SamplerValue;
    VIR_IMG_ChannelDataType     channelDataType = VIR_IMG_CT_SNORM_INT8; /* default value, assuemd */
    VIR_IMG_ChannelOrder        channelOrder = VIR_IMG_CO_RGBA;           /* default value, assuemd */
    gctUINT                     addressMode;
    gctUINT                     filterMode;         /* 1: linear filter mode; 0: nearest filter mode */
    gctUINT                     normalizedCoord ;   /* 1: normalized coordinate; 0: unnormalized coordinate */
    VIR_IMG_Type                imageType = pImageDesc ? VSC_CLImageType_To_VIRImageType(pImageDesc->sd.imageType)
                                                       : VIR_IMG_TY_2D;
    VSC_OCLImgLibKind           imgLibKind;
    gceSTATUS                   status = gcvSTATUS_OK;
    gctBOOL                     coordIntType = VIR_IMG_isSamplerIntCoords(imageSamplerValue);

    if (pImageDesc != gcvNULL && pImageDesc->sd.width != 0) /* valid desc */
    {
        /* get the image info from image desciptor */
        channelDataType = VSC_CLChannelDataType_To_VIRChannelDataType(pImageDesc->sd.channelDataType);
        channelOrder    = VSC_CLChannelOrder_To_VIRChannelOrder(pImageDesc->sd.channelOrder);
        imageType       = VSC_CLImageType_To_VIRImageType(pImageDesc->sd.imageType);
    }
    else
    {
        gcmASSERT(pImageDesc != gcvNULL);
        /* assume channel data type bu image value type */
        switch (pImageDesc->sd.imageValueType)
        {
            case VSC_ImageValueFloat:
                channelDataType = VIR_IMG_CT_UNORM_INT8;
                break;

            case VSC_ImageValueInt:
                channelDataType = VIR_IMG_CT_SIGNED_INT8;
                break;

            case VSC_ImageValueUint:
                channelDataType = VIR_IMG_CT_UNSIGNED_INT8;
                break;

            default:
                gcmASSERT(0);
                break;
        }

        /* set assumed default value to ImageDescHandle so driver can know what value is
         * used for default */
        gcmASSERT(pImageDesc != gcvNULL);
        pImageDesc->sd.channelDataType = VSC_VIRChannelDataType_To_CLChannelDataType(channelDataType);
        pImageDesc->sd.channelOrder    = VSC_VIRChannelOrder_To_CLChannelOrder(channelOrder);
        pImageDesc->sd.imageType       = VSC_VIRImageType_To_CLImageType(imageType);
    }

    if (imageSamplerValue == VSC_IMG_SAMPLER_UNKNOWN_VALUE || imageSamplerValue == VSC_IMG_SAMPLER_INVALID_VALUE)
    {
        /* treat unknown and invalid value as default value */
        imageSamplerValue = VSC_IMG_SAMPLER_DEFAULT_VALUE;
    }

    if (vscImageSamplerNeedLibFuncForHWCfg(pImageDesc, imageSamplerValue, pHwCfg, pImgLibKind, UseTexld, gcvNULL))
    {
        gctCHAR                 name[256] = "";
        struct ImgReadNameStr * imgReadNameInfo;

        imgLibKind = *pImgLibKind;
        gcmASSERT(imgLibKind < VSC_OCLImgLibKind_Counts);

        addressMode = imageSamplerValue & 0x0F;
        filterMode = (imageSamplerValue & VSC_IMG_SAMPLER_FILTER_LINEAR) ? 1 : 0;
        normalizedCoord = (imageSamplerValue & VSC_IMG_SAMPLER_NORMALIZED_COORDS_TRUE) ? 1 : 0;
        gcmASSERT(addressMode < VSC_IMG_SAMPLER_ADDRESS_COUNT);

        imgReadNameInfo = &imgReadNamesInfo[imgLibKind];
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->baseName));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->samplerFilterStr[filterMode]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->samplerNormalizedCoordStr[normalizedCoord]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->samplerAddressModeStr[addressMode]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->coordType[coordIntType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->imageValueType[pImageDesc->sd.imageValueType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->imgChannelTypeStr[channelDataType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->imgTypeStr[imageType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgReadNameInfo->imgChannelOrderStr[channelOrder]));

        /* dup the name to pLibFuncName */
        gcmONERROR(gcoOS_StrDup(gcvNULL, name, pLibFuncName));
    }
    else
    {
        /* no lib func needed */
        *pLibFuncName = gcvNULL;
    }

OnError:
    if (errCode != VSC_ERR_NONE)
    {
        status = vscERR_CastErrCode2GcStatus(errCode);
    }
    return status;
}

gceSTATUS
vscConstructImageWriteLibFuncName(
    VSC_ImageDesc *         ImageDescHandle, /* VSC_ImageDesc */
    VSC_HW_CONFIG *         pHwCfg,
    gctSTRING *             pLibFuncName, /* returned lib function name if needed */
    VSC_OCLImgLibKind *     pImgLibKind             /* the image lib kind to be used */
)
{
    VSC_ErrCode                 errCode  = VSC_ERR_NONE;
    VSC_ImageDesc *             pImageDesc = (VSC_ImageDesc *)ImageDescHandle;
    VIR_IMG_ChannelDataType     channelDataType = VIR_IMG_CT_SNORM_INT8; /* default value, assuemd */
    VIR_IMG_ChannelOrder        channelOrder = VIR_IMG_CO_RGBA;           /* default value, assuemd */
    VIR_IMG_Type                imageType = pImageDesc ? VSC_CLImageType_To_VIRImageType(pImageDesc->sd.imageType)
                                                       : VIR_IMG_TY_2D;
    VSC_OCLImgLibKind           imgLibKind;
    gceSTATUS                   status = gcvSTATUS_OK;

    if (pImageDesc != gcvNULL && pImageDesc->sd.width != 0) /* valid desc */
    {
        /* get the image info from image desciptor */
        channelDataType = VSC_CLChannelDataType_To_VIRChannelDataType(pImageDesc->sd.channelDataType);
        channelOrder    = VSC_CLChannelOrder_To_VIRChannelOrder(pImageDesc->sd.channelOrder);
        imageType       = VSC_CLImageType_To_VIRImageType(pImageDesc->sd.imageType);
    }
    else
    {
        gcmASSERT(pImageDesc != gcvNULL);
        /* assume channel data type bu image value type */
        switch (pImageDesc->sd.imageValueType)
        {
            case VSC_ImageValueFloat:
                channelDataType = VIR_IMG_CT_UNORM_INT8;
                break;

            case VSC_ImageValueInt:
                channelDataType = VIR_IMG_CT_SIGNED_INT8;
                break;

            case VSC_ImageValueUint:
                channelDataType = VIR_IMG_CT_UNSIGNED_INT8;
                break;

            default:
                gcmASSERT(0);
                break;
        }

        /* set assumed default value to ImageDescHandle so driver can know what value is
         * used for default */
        gcmASSERT(pImageDesc != gcvNULL);
        pImageDesc->sd.channelDataType = VSC_VIRChannelDataType_To_CLChannelDataType(channelDataType);
        pImageDesc->sd.channelOrder    = VSC_VIRChannelOrder_To_CLChannelOrder(channelOrder);
        pImageDesc->sd.imageType       = VSC_VIRImageType_To_CLImageType(imageType);
    }

    if (vscImageWriteNeedLibFuncForHWCfg(pImageDesc,pHwCfg, pImgLibKind, gcvNULL))
    {
        gctCHAR                 name[256] = "";
        struct ImgWriteNameStr * imgWriteNameInfo;
        imgLibKind = *pImgLibKind;
        gcmASSERT(imgLibKind < VSC_OCLImgLibKind_Counts);
        imgWriteNameInfo = &imgWriteNamesInfo[imgLibKind];
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgWriteNameInfo->baseName));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgWriteNameInfo->imageValueType[pImageDesc->sd.imageValueType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgWriteNameInfo->imgChannelTypeStr[channelDataType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgWriteNameInfo->imgTypeStr[imageType]));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imgWriteNameInfo->imgChannelOrderStr[channelOrder]));

        /* dup the name to pLibFuncName */
        gcmONERROR(gcoOS_StrDup(gcvNULL, name, pLibFuncName));
    }
    else
    {
        /* no lib func needed */
        *pLibFuncName = gcvNULL;
    }

OnError:
    if (errCode != VSC_ERR_NONE)
    {
        status = vscERR_CastErrCode2GcStatus(errCode);
    }
    return status;
}

vscImageValueType
_virType2ImageValueType(VIR_TypeId ty)
{
    gcmASSERT(VIR_TypeId_isPrimitive(ty));
    return VIR_TypeId_isFloat(ty) ? VSC_ImageValueFloat :
           VIR_TypeId_isUnSignedInteger(ty) ? VSC_ImageValueUint : VSC_ImageValueInt;
}

/* check if we need to  the image read instruction to lib function call:
 * 1. if the sampler value and image data are unknown, we assume they are using default
 *    value, so there is no lib function needed for the image read if the HW can handle
 *    default image read
 * 2. if at run-time it turns out the assumption is not correct, the driver will
 *    fill the image and sampler with correct value and the re-compilation
 * 3. we know for each HW arch, which sampler and image format can be natively
 *    handled, for these image read, we do not transform it to lib func call
 * 4. if the image read cannot be handled natively, we construct a name which
 *    uniquely identify the corresponding lib function which can perform the image
 *    read with the sampler and image format
 * 5. do the conversion:
 *       IMG_READ  dest, image, coord, sampler
 *    ==>
 *       EXTCALL  dest, libFuncName, PARAMETERS<image, coord, sampler>
 * 6. return TRUE if IMG_READ is transformed, otherwise return FALSE
 */
gctBOOL
_vscTransformImgReadToLibFuncCall(
    VSC_SH_PASS_WORKER  *pPassWorker,
    VIR_Instruction     *pInst
    )
{
    VIR_Operand*        imageSrc;
    VIR_Operand*        parentSrc = gcvNULL;
    VIR_Symbol*         imageSym;
    VIR_Shader *        pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_OpCode          opcode = VIR_Inst_GetOpcode(pInst);
    gctSTRING           libFuncName = gcvNULL;
    VIR_Uniform *       imageUniform;
    VSC_HW_CONFIG *     pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_OCLImgLibKind   imgLibKind;
    gctBOOL             useTexld = gcvFALSE;
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_TypeId          valueType = VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst));
    vscImageValueType   imgValTy  = _virType2ImageValueType(valueType) ;
    VSC_ImageDesc *     imgDesc;
    VIR_IMG_Type        imageType;
    if (!VIR_OPCODE_isImgLd(opcode))
    {
        gcmASSERT(gcvFALSE);
    }

    if (VIR_OPCODE_isVX(opcode))
    {
        /* VX_IMG_LOAD/VX_IMG_LOAD_3D is sampleless image read, no need to use lib function */
        return gcvFALSE;
    }

    /* get image uniform and sampler info used by the inst */
    imageSrc = VIR_Inst_GetSource(pInst, 0);  /* image always the fisrt operand*/
    gcmASSERT(VIR_Operand_isSymbol(imageSrc));

    imageSym = VIR_Operand_GetSymbol(imageSrc);
    if (!(VIR_Symbol_isImageT(imageSym) || VIR_Symbol_isImage(imageSym)))
    {
        parentSrc = _vscVIR_FindParentImgOperandFromIndex(pInst, imageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(imageSrc), 0));
        imageSym = VIR_Operand_GetSymbol(parentSrc);
    }
    gcmASSERT(imageSym && (VIR_Symbol_isImageT(imageSym)|| VIR_Symbol_isImage(imageSym)));
    imageUniform = VIR_Symbol_GetImage(imageSym);
    imgDesc = &VIR_Uniform_GetImageDesc(imageUniform);
    /* set the image value type by instruction's dest type */
    imgDesc->sd.imageValueType = imgValTy;

    /* set the image type by imageSym's type */
    switch (VIR_Symbol_GetTypeId(imageSym)) {
    case VIR_TYPE_IMAGE_1D_T:
        imageType = VIR_IMG_TY_1D;
        break;
    case VIR_TYPE_IMAGE_1D_BUFFER_T:
        imageType = VIR_IMG_TY_1D_BUFFER;
        break;
    case VIR_TYPE_IMAGE_1D_ARRAY_T:
        imageType = VIR_IMG_TY_1D_ARRAY;
        break;
    case VIR_TYPE_IMAGE_2D_ARRAY_T:
        imageType = VIR_IMG_TY_2D_ARRAY;
        break;
    case VIR_TYPE_IMAGE_3D_T:
        imageType = VIR_IMG_TY_3D;
        break;
    case VIR_TYPE_IMAGE_2D_T:
    default:
        imageType = VIR_IMG_TY_2D;
        break;
    }
    imgDesc->sd.imageType = VSC_VIRImageType_To_CLImageType(imageType);

    gcmONERROR(vscConstructImageReadLibFuncName(imgDesc,
                                              VIR_Uniform_GetImageSamplerValue(imageUniform),
                                              pHwCfg,
                                              &libFuncName,
                                              &imgLibKind,
                                              &useTexld));
    if (libFuncName != gcvNULL)
    {
        /* a lib function should be called */
        VIR_Function * func = VIR_Inst_GetFunction(pInst);
        VIR_Operand *  funcName;
        VIR_NameId     funcNameId;
        VIR_Operand *  parameters;
        VIR_ParmPassing *parm;
        gctUINT i, argCount = VIR_OPCODE_GetSrcOperandNum(opcode);

        /* set the lib func name to image uniform */
        VIR_Uniform_SetImageLibFuncName(imageUniform, libFuncName);

        ON_ERROR(VIR_Function_NewOperand(func, &funcName), "Failed to new operand");
        ON_ERROR(VIR_Shader_AddString(pShader, libFuncName, &funcNameId), "");
        VIR_Operand_SetName(funcName, funcNameId);
        ON_ERROR(VIR_Function_NewOperand(func, &parameters), "Failed to new operand");

        /* create a new VIR_ParmPassing */
        ON_ERROR(VIR_Function_NewParameters(func, argCount, &parm), "Failed to copy operand");
        for (i = 0; i < argCount; i++)
        {
            VIR_Operand_Copy(parm->args[i], VIR_Inst_GetSource(pInst, i));
        }

        /* set parameters to new operand */
        VIR_Operand_SetParameters(parameters, parm);

        VIR_Inst_SetSource(pInst, 0, funcName);
        VIR_Inst_SetSource(pInst, 1, parameters);
        VIR_Inst_SetOpcode(pInst, VIR_OP_EXTCALL);
        VIR_Inst_SetSrcNum(pInst, 2);

        gcoOS_Free(gcvNULL, libFuncName);

        return gcvTRUE;
    }
    else if (useTexld)
    {
    }
OnError:
    return gcvFALSE;
}

/* check if we need to  the image write instruction to lib function call:
 * 1. if the image data are unknown, we assume they are using default
 *    value, so there is no lib function needed for the image write if the HW can handle
 *    default image write
 * 2. if at run-time it turns out the assumption is not correct, the driver will
 *    fill the image with correct value and the re-compilation
 * 3. we know for each HW arch, which image format can be natively
 *    handled, for these image write, we do not transform it to lib func call
 * 4. if the image write cannot be handled natively, we construct a name which
 *    uniquely identify the corresponding lib function which can perform the image
 *    write with the image format
 * 5. do the conversion:
 *       IMG_STORE[_3D]  dest, image, coord, color
 *    ==>
 *       EXTCALL  dest, libFuncName, PARAMETERS<image, coord, color>
 * 6. return TRUE if IMG_STORE[_3D] is transformed, otherwise return FALSE
 */
gctBOOL
_vscTransformImgWriteToLibFuncCall(
    VSC_SH_PASS_WORKER  *pPassWorker,
    VIR_Instruction     *pInst
    )
{
    VIR_Operand*        imageSrc;
    VIR_Operand*        parentSrc = gcvNULL;
    VIR_Symbol*         imageSym;
    VIR_Shader *        pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_OpCode          opcode = VIR_Inst_GetOpcode(pInst);
    gctSTRING           libFuncName = gcvNULL;
    VIR_Uniform *       imageUniform;
    VSC_HW_CONFIG *     pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VSC_OCLImgLibKind   imgLibKind;
    gceSTATUS           status = gcvSTATUS_OK;
    VIR_TypeId          valueType = VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst));
    vscImageValueType   imgValTy  = _virType2ImageValueType(valueType) ;
    VSC_ImageDesc *     imgDesc;
    VIR_IMG_Type        imageType;

    if (!VIR_OPCODE_isImgSt(opcode))
    {
        gcmASSERT(gcvFALSE);
    }

    /* get image uniform and sampler info used by the inst */
    imageSrc = VIR_Inst_GetSource(pInst, 0);  /* image always the fisrt operand*/
    gcmASSERT(VIR_Operand_isSymbol(imageSrc));

    imageSym = VIR_Operand_GetSymbol(imageSrc);
    if (!(VIR_Symbol_isImageT(imageSym) || VIR_Symbol_isImage(imageSym)))
    {
        parentSrc = _vscVIR_FindParentImgOperandFromIndex(pInst, imageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(imageSrc), 0));
        imageSym = VIR_Operand_GetSymbol(parentSrc);
    }
    gcmASSERT(imageSym && (VIR_Symbol_isImageT(imageSym)|| VIR_Symbol_isImage(imageSym)));
    imageUniform = VIR_Symbol_GetImage(imageSym);
    imgDesc = &VIR_Uniform_GetImageDesc(imageUniform);
    /* set the image value type by instruction's dest type */
    imgDesc->sd.imageValueType = imgValTy;

    /* set the image type by imageSym's type */
    switch (VIR_Symbol_GetTypeId(imageSym)) {
    case VIR_TYPE_IMAGE_1D_T:
        imageType = VIR_IMG_TY_1D;
        break;
    case VIR_TYPE_IMAGE_1D_BUFFER_T:
        imageType = VIR_IMG_TY_1D_BUFFER;
        break;
    case VIR_TYPE_IMAGE_1D_ARRAY_T:
        imageType = VIR_IMG_TY_1D_ARRAY;
        break;
    case VIR_TYPE_IMAGE_2D_ARRAY_T:
        imageType = VIR_IMG_TY_2D_ARRAY;
        break;
    case VIR_TYPE_IMAGE_3D_T:
        imageType = VIR_IMG_TY_3D;
        break;
    case VIR_TYPE_IMAGE_2D_T:
    default:
        imageType = VIR_IMG_TY_2D;
        break;
    }
    imgDesc->sd.imageType = VSC_VIRImageType_To_CLImageType(imageType);

    gcmONERROR(vscConstructImageWriteLibFuncName(imgDesc,
                                              pHwCfg,
                                              &libFuncName,
                                              &imgLibKind));
    if (libFuncName != gcvNULL)
    {
        /* a lib function should be called */
        VIR_Function * func = VIR_Inst_GetFunction(pInst);
        VIR_Operand *  funcName;
        VIR_NameId     funcNameId;
        VIR_Operand *  parameters;
        VIR_ParmPassing *parm;
        gctUINT i, argCount = VIR_OPCODE_GetSrcOperandNum(opcode);

        /* set the lib func name to image uniform */
        VIR_Uniform_SetImageLibFuncName(imageUniform, libFuncName);

        ON_ERROR(VIR_Function_NewOperand(func, &funcName), "Failed to new operand");
        ON_ERROR(VIR_Shader_AddString(pShader, libFuncName, &funcNameId), "");
        VIR_Operand_SetName(funcName, funcNameId);
        ON_ERROR(VIR_Function_NewOperand(func, &parameters), "Failed to new operand");

        /* create a new VIR_ParmPassing */
        ON_ERROR(VIR_Function_NewParameters(func, argCount, &parm), "Failed to copy operand");
        for (i = 0; i < argCount; i++)
        {
            VIR_Operand_Copy(parm->args[i], VIR_Inst_GetSource(pInst, i));
        }

        /* set parameters to new operand */
        VIR_Operand_SetParameters(parameters, parm);

        VIR_Inst_SetSource(pInst, 0, funcName);
        VIR_Inst_SetSource(pInst, 1, parameters);
        VIR_Inst_SetOpcode(pInst, VIR_OP_EXTCALL);
        VIR_Inst_SetSrcNum(pInst, 2);

        gcoOS_Free(gcvNULL, libFuncName);

        return gcvTRUE;
    }

OnError:
    return gcvFALSE;
}

DEF_QUERY_PASS_PROP(vscVIR_ConvertVirtualInstructions)
{
   pPassProp->supportedLevels = VSC_PASS_LEVEL_ML | VSC_PASS_LEVEL_LL;
}

DEF_SH_NECESSITY_CHECK(vscVIR_ConvertVirtualInstructions)
{
    return gcvTRUE;
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
                    VIR_TypeId uniformTypeId = VIR_TYPE_FLOAT_X4;

                    dstType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(VIR_Inst_GetDest(inst)));

                    if (VIR_Type_isInteger(dstType))
                    {
                        uniformTypeId = VIR_TYPE_INTEGER_X4;
                    }

                    gcmASSERT(VIR_Operand_isSymbol(samplerSrc));
                    samplerSym = VIR_Operand_GetSymbol(samplerSrc);
                    if(!VIR_Symbol_isSampler(samplerSym))
                    {
                        parentSrc = _vscVIR_FindParentImgOperandFromIndex(inst, samplerSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerSrc), 0));
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
                        lodMinMaxUniform->u.samplerOrImageAttr.texelBufferToImageSymId = VIR_INVALID_ID;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerSrc, func, samplerUniform->u.samplerOrImageAttr.lodMinMax);
                    VIR_Operand_SetTypeId(samplerSrc, uniformTypeId);

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
                        parentSrc = _vscVIR_FindParentImgOperandFromIndex(inst, samplerOrImageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerOrImageSrc), 0));
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
                        levelBaseSizeUniform->u.samplerOrImageAttr.texelBufferToImageSymId = VIR_INVALID_ID;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerOrImageSrc, func, samplerOrImageUniform->u.samplerOrImageAttr.levelBaseSize);
                    VIR_Operand_SetTypeId(samplerOrImageSrc, VIR_TYPE_INTEGER_X4);

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
                        parentSrc = _vscVIR_FindParentImgOperandFromIndex(inst, samplerOrImageSrc, VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(samplerOrImageSrc), 0));
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
                        levelsSamplesUniform->u.samplerOrImageAttr.texelBufferToImageSymId = VIR_INVALID_ID;
                    }
                    VIR_Inst_SetOpcode(inst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(inst, 1);
                    VIR_Operand_SetSymbol(samplerOrImageSrc, func, samplerOrImageUniform->u.samplerOrImageAttr.levelsSamples);
                    VIR_Operand_SetTypeId(samplerOrImageSrc, VIR_TYPE_INTEGER_X4);

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

    if (VIR_Shader_IsDesktopGL(shader))
    {
        return errCode;
    }
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
            if (!(VIR_Symbol_isSampler(sym) &&
                  gcoOS_StrCmp(VIR_Shader_GetSymNameString(shader, sym), "#BaseSamplerSym") == gcvSTATUS_OK))
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
        case VIR_OPND_NAME:
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
                if (VIR_Symbol_isSampler(opndSym0) &&
                    gcoOS_StrCmp(VIR_Shader_GetSymNameString(VIR_Inst_GetShader(inst), opndSym0), "#BaseSamplerSym") == gcvSTATUS_OK)
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

/* Update precision and pack mode. */
static VSC_ErrCode _UpdatePrecisionAndPackMode(VIR_Shader* pShader)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode*       pFunc_node = gcvNULL;
    gctBOOL                 bUpdatePrecision = gcvTRUE;

    /* Check if we need to update the precision. */
    if(!(VIR_Shader_IsFS(pShader) || VIR_Shader_IsCL(pShader)) || VIR_Shader_IsVulkan(pShader) || VIR_Shader_IsDesktopGL(pShader))
    {
        bUpdatePrecision = gcvFALSE;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (pFunc_node = VIR_FuncIterator_First(&func_iter);
         pFunc_node != gcvNULL;
         pFunc_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*       pFunc = pFunc_node->function;
        VIR_InstIterator    inst_iter;
        VIR_Instruction*    pInst = gcvNULL;
        gctUINT             i;

        /* Update the precision for the function parameters. */
        if (bUpdatePrecision)
        {
            for (i = 0; i < VIR_IdList_Count(&pFunc->paramters); i++)
            {
                VIR_Id      id = VIR_IdList_GetId(&pFunc->paramters, i);
                VIR_Symbol* pParam = VIR_Function_GetSymFromId(pFunc, id);
                VIR_Symbol* pVirReg = VIR_Shader_FindSymbolByTempIndex(pShader, VIR_Symbol_GetVariableVregIndex(pParam));

                gcmASSERT(VIR_Symbol_GetPrecision(pParam) != VIR_PRECISION_DEFAULT || VIR_Shader_IsCL(pShader));
                /* gcmASSERT(VIR_Symbol_GetVregVariable(virReg) == param); */ /* virReg's variable will be reset to another sym which came from old variable for function input/output */
                if(VIR_Symbol_GetPrecision(pParam) == VIR_PRECISION_ANY)
                {
                    VIR_Symbol_SetCurrPrecision(pParam, VIR_PRECISION_HIGH);
                    VIR_Symbol_SetCurrPrecision(pVirReg, VIR_PRECISION_HIGH);
                }
            }
        }

        /* Check all instructions. */
        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            if (bUpdatePrecision)
            {
                for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
                {
                    VIR_Operand*    pSrcOpnd = VIR_Inst_GetSource(pInst, i);

                    vscVIR_PrecisionUpdateSrc(pShader, pSrcOpnd);
                }

                vscVIR_PrecisionUpdateDst(pInst);
            }

            /* set pack mode if the instruction uses packed type.
             * adjust immediate date type for componentwise instruction */
            VIR_Inst_CheckAndSetPakedMode(pInst);
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Update precision and pack mode.", pShader, gcvTRUE);
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
                                VIR_TypeId newBaseTypeId = VIR_TypeId_ComposeNonOpaqueArrayedType(pShader, VIR_TYPE_FLOAT32, symBaseTypeIdComponentCount, symBaseTypeIdRowCount, VIR_Type_GetArrayLength(symType));
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
            VIR_TypeId operandTypeId = VIR_Operand_GetTypeId(operand);

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
        VIR_TypeId operandTypeId = VIR_Operand_GetTypeId(operand);

        if(VIR_TypeId_isPrimitive(operandTypeId) && VIR_TypeId_isInteger(operandTypeId))
        {
            gctUINT operandComponentCount = VIR_GetTypeComponents(operandTypeId);
            VIR_TypeId newTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, operandComponentCount, 1);

            gcmASSERT(VIR_GetTypeRows(operandTypeId) == 1);
            VIR_Operand_SetTypeId(operand, newTypeId);
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
        case VIR_OP_CSELECT:
        {
            VIR_ConditionOp conditionOp = VIR_Inst_GetConditionOp(inst);
            if(conditionOp == VIR_COP_EQUAL)
            {
                toBeConvertedSource[0] = gcvFALSE;
                toBeConvertedSource[1] = gcvFALSE;
            }
            break;
        }
        case VIR_OP_COMPARE:
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
        case VIR_OP_CONVERT:
        {
            VIR_Operand* dest = VIR_Inst_GetDest(inst);
            VIR_Operand* src = VIR_Inst_GetSource(inst, 0);

            if(VIR_GetTypeComponentType(VIR_Operand_GetTypeId(dest)) == VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src)))
            {
                if(ConvertedDest && !ConvertedSource[0])
                {
                    VIR_Function* func = VIR_Inst_GetFunction(inst);
                    VIR_TypeId destTypeId = VIR_Operand_GetTypeId(dest);
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

                        VIR_Function_AddInstructionBefore(func, VIR_OP_F2I, destTypeId, inst, gcvTRUE, &newInst);
                        VIR_Operand_SetSymbol(VIR_Inst_GetDest(newInst), func, newSymId);
                        VIR_Operand_SetEnable(VIR_Inst_GetDest(newInst), newEnable);
                        VIR_Operand_Copy(VIR_Inst_GetSource(newInst, 0), VIR_Inst_GetSource(inst, 0));

                        VIR_Inst_SetOpcode(inst, VIR_OP_I2F);
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

VSC_ErrCode _ConvertRetToJmpForFunction(
    IN OUT VIR_Shader* pShader,
    IN OUT VIR_Function* pFunc,
    IN OUT gctBOOL*    pInvalidCfg
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
        errCode = VIR_Function_AddInstructionAfter(pFunc, VIR_OP_RET,
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
                    errCode = VIR_Pass_DeleteInstruction(pFunc, pInst, pInvalidCfg);
                    ON_ERROR(errCode, "delete instruction");
                }
                break;
            }
            else
            {
                /* Create a label for the tail instruction of this function. */
                if (pLabel == gcvNULL)
                {
                    VIR_Function_AddLabel(pFunc, "#sh_FuncEnd", &labelId);

                    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_LABEL,
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
                errCode = VIR_Function_AddInstructionAfter(pFunc, VIR_OP_JMP,
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
                errCode = VIR_Pass_DeleteInstruction(pFunc, pInst, pInvalidCfg);
                ON_ERROR(errCode, "delete instruction");

                pInst = pNewInst;
            }
        }
    }

OnError:
    return errCode;
}

VSC_ErrCode _ConvertRetToJmpForFunctions(
    IN OUT VIR_Shader* pShader,
    IN OUT gctBOOL*    pInvalidCfg
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
                                              func,
                                              pInvalidCfg);
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
    IN gctBOOL              bNeedToCutDownWorkGroupSize,
    IN OUT VIR_Function    *pFunc,
    INOUT gctBOOL          *pHasFuncNeedToForceInline
    )
{
    gctBOOL                 canSrc0OfImgRelatedBeTemp = pHwCfg->hwFeatureFlags.canSrc0OfImgLdStBeTemp;
    VIR_Instruction        *pInst;
    VIR_InstIterator        instIter;

    if (VIR_Function_GetInstCount(pFunc) == 0)
    {
        return gcvFALSE;
    }

    if (bNeedToCutDownWorkGroupSize)
    {
        if (pHasFuncNeedToForceInline)
        {
            *pHasFuncNeedToForceInline = gcvTRUE;
        }
        return gcvTRUE;
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
                    !VIR_Symbol_isImageT(pOpndSym) &&
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

gctBOOL _CheckMLLevelAlwaysInlineFunction(
    IN OUT VIR_Shader      *pShader,
    IN OUT VIR_Function    *pFunc
    )
{
    VIR_Instruction        *pInst;
    VIR_InstIterator        instIter;
    gctBOOL                 needInLine = gcvFALSE;

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
        VIR_OperandInfo     opndInfo;
        /*
        ** If source0 of a TEXLD/image-related opcode is not a uniform,
        ** then we need to inline it because it may need to be recompiled.
        */
        if (VIR_OPCODE_isTexLd(opcode) || VIR_OPCODE_isGetSamplerInfo(opcode) || VIR_OPCODE_isImgRelated(opcode))
        {
            pOperand = VIR_Inst_GetSource(pInst, 0);
            VIR_Operand_GetOperandInfo(pInst, pOperand, &opndInfo);

            if (!opndInfo.isUniform)
            {
                needInLine = gcvTRUE;
                break;
            }
        }
        else if (opcode == VIR_OP_INTRINSIC)
        {
            VIR_IntrinsicsKind  intrinsicKind = VIR_Operand_GetIntrinsicKind(VIR_Inst_GetSource(pInst, 0));
            VIR_ParmPassing*    pParmOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pInst, 1));

            /* If it is an image load/store intrinsic and the base is not a uniform, mark it as must-inline. */
            if (VIR_Intrinsics_isImageLoad(intrinsicKind) || VIR_Intrinsics_isImageStore(intrinsicKind))
            {
                pOperand = pParmOpnd->args[0];
                VIR_Operand_GetOperandInfo(pInst, pOperand, &opndInfo);

                if (!opndInfo.isUniform)
                {
                    needInLine = gcvTRUE;
                    break;
                }
            }
        }
    }

    return needInLine;
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
    IN  VSC_HW_CONFIG      *pHwCfg,
    INOUT gctBOOL          *pHasFuncNeedToForceInline
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode           *func_node;
    VIR_Function               *pFunc;
    gctBOOL                     alwaysInline = gcvFALSE;
    gctBOOL                     bNeedToCutDownWorkGroupSize = VIR_Shader_NeedToCutDownWorkGroupSize(pShader, pHwCfg);

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

        alwaysInline = _CheckAlwaysInlineFunction(pShader, pHwCfg, bNeedToCutDownWorkGroupSize, pFunc, pHasFuncNeedToForceInline);

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

VSC_ErrCode _CheckMLLevelAlwaysInlineFunctions(
    IN OUT VIR_Shader      *pShader
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

        alwaysInline = _CheckMLLevelAlwaysInlineFunction(pShader, pFunc);

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
                    VIR_Type_isScalar(VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(pSrcOpnd))))
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

                    VIR_Operand_SetImmediate(pSrcOpnd, VIR_Operand_GetTypeId(pSrcOpnd), scalarConstVal);
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

DEF_SH_NECESSITY_CHECK(vscVIR_PreprocessLLShader)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_PreprocessLLShader(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader* pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG* pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL bInvalidCfg = gcvFALSE;
    gctBOOL     bHasFuncNeedToForceInline = gcvFALSE;

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

    /* Update precision and pack mode. */
    errCode = _UpdatePrecisionAndPackMode(pShader);
    ON_ERROR(errCode, "Update precision and pack mode.");

    /* Change all RETs to JMPs and only keep one RET at the end of the function. */
    errCode = _ConvertRetToJmpForFunctions(pShader, &bInvalidCfg);
    ON_ERROR(errCode, "Convert RET to JMP for functions");

    if (VIR_Shader_IsVulkan(pShader))
    {
        errCode = _MergeConstantOffset(pShader);
        ON_ERROR(errCode, "Merge constant offset");
    }

    /* Check if a function is ALWAYSINLINE. */
    errCode = _CheckAlwaysInlineFunctions(pShader, pHwCfg, &bHasFuncNeedToForceInline);
    ON_ERROR(errCode, "Check always inline functions");

    /* Change all scalar vector constant source into immediate. */
    errCode = _ConvertScalarVectorConstToImm(pShader);
    ON_ERROR(errCode, "Convert Scalar vector constant to imm");

    if (bInvalidCfg)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = bInvalidCfg;
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After preprocess LL", pShader, gcvTRUE);
    }

    if (pPassWorker->basePassWorker.pPassSpecificData != gcvNULL)
    {
        (*(VSC_PRELL_PASS_DATA *)pPassWorker->basePassWorker.pPassSpecificData).bHasFuncNeedToForceInline = bHasFuncNeedToForceInline;
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_CheckMustInlineFuncForML)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
    pPassProp->passFlag.resCreationReq.s.bNeedCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_CheckMustInlineFuncForML)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_CheckMustInlineFuncForML(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Shader* pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    /* Check if a function is ALWAYSINLINE, no need to check a patch lib. */
    errCode = _CheckMLLevelAlwaysInlineFunctions(pShader);
    ON_ERROR(errCode, "Check ML level always inline functions");

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
        opcode == VIR_OP_REP || opcode == VIR_OP_ENDREP || opcode == VIR_OP_TEXLD_GATHER)
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

DEF_SH_NECESSITY_CHECK(VIR_Shader_CheckDual16able)
{
    VSC_COMPILER_CONFIG     *compCfg = &pPassWorker->pCompilerParam->cfg;
    VIR_Shader              *Shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    VIR_Shader_SetDual16Mode(Shader, gcvFALSE);

    /* only fragment shader or ocl with VC_OPTION=DUAL16:num>0 can be dual16 shader,
    ** and exclude OpenVG shader due to precision issue
    */
    if (!compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportDual16 ||
        (VIR_Shader_GetKind(Shader) != VIR_SHADER_FRAGMENT)       ||
        (!VIR_Shader_IsFS(Shader) &&
         !(VIR_Shader_IsCL(Shader) && gcGetDualFP16Mode(compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.hasHalti2)))    ||
        VIR_Shader_IsDesktopGL(Shader)                            ||
        VIR_Shader_IsVulkan(Shader)                               ||
        VIR_Shader_IsOpenVG(Shader)                               ||
        VIR_Shader_HasOutputArrayHighp(Shader)                    || /* currently, we disable dual16 if the output array is highp */
        !VirSHADER_DoDual16(VIR_Shader_GetId(Shader))             ||
        gcmOPT_DisableOPTforDebugger()                               /* we disable dual16 for debugging mode */
        )
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctINT
_CheckCstRegFileReadPortLimitation(VIR_Shader* pShader, VIR_Instruction* pInst, gctBOOL bHasOneConstFix)
{

    gctINT                     addedInstCount = 0;
    VIR_OpCode                 opCode;
    VIR_Uniform*               pUniform;
    VIR_Operand*               opnd;
    gctUINT                    i, j, firstUniformId, thisUniformId;
    VIR_Symbol*                pSym = gcvNULL;
    gctBOOL                    bFirstConstRegIndexing, bHitReadPortLimitation;

    /* Only have one or zero source, just bail out. */
    if (VIR_Inst_GetSrcNum(pInst) < 2)
        return addedInstCount;

    opCode = VIR_Inst_GetOpcode(pInst);

    /* This instruction has not a uniform512 source, just bail out. */
    if (bHasOneConstFix && VIR_OPCODE_U512_SrcNo(opCode) == 0)
    {
        return addedInstCount;
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pInst) - 1; ++i)
    {
        firstUniformId = NOT_ASSIGNED;
        bFirstConstRegIndexing = gcvFALSE;

        for (j = 0; j < VIR_Inst_GetSrcNum(pInst); ++j)
        {
            opnd = VIR_Inst_GetSource(pInst, j);

            pUniform = gcvNULL;
            if (VIR_Operand_GetOpKind(opnd) == VIR_OPND_SYMBOL)
            {
                pSym = VIR_Operand_GetSymbol(opnd);
                pUniform = VIR_Symbol_GetUniformPointer(pShader, pSym);
                }

            if (pUniform)
            {
                bHitReadPortLimitation = gcvFALSE;

                thisUniformId = pUniform->index +
                                    VIR_Operand_GetMatrixConstIndex(opnd) +
                                    ((VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE) ?
                                    VIR_Operand_GetRelIndexing(opnd) : 0);

                /* Check whether shader hits the limitation that HW does not support two read port on reg file. */
                if (firstUniformId == NOT_ASSIGNED)
                {
                    firstUniformId = thisUniformId;
                    bFirstConstRegIndexing = (VIR_Operand_GetRelAddrMode(opnd) != VIR_INDEXED_NONE);
                }
                else
                {
                    if (!bFirstConstRegIndexing && VIR_Operand_GetRelAddrMode(opnd) == VIR_INDEXED_NONE)
                    {
                        if (firstUniformId != thisUniformId)
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
                    /* when hitting read port limitation, a MOV instruction will be inserted
                        and if the operand to be replaced is high precision, two instructions will be added due to dual16 mode. */
                    addedInstCount++;
                    if (VIR_Operand_GetPrecision(opnd) == VIR_PRECISION_HIGH)
                        addedInstCount++;
                }
            }
        }
    }
    return addedInstCount;
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
    gctBOOL                 isAppConformance = (compCfg->ctx.appNameId == gcvPATCH_GTFES30 || compCfg->ctx.appNameId == gcvPATCH_DEQP);
    VSC_HW_CONFIG*          pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctBOOL                 bHasOneConstFix = pHwCfg->hwFeatureFlags.noOneConstLimit;

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
    /* support triangle test for dual16 */
    if(!VSC_OPTN_InRange(VIR_Shader_GetId(Shader), VSC_OPTN_DUAL16Options_BeforeShader(options), VSC_OPTN_DUAL16Options_AfterShader(options)))
    {
        if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "dual16 skip shader(%d) NOT in option (as %d bs %d)\n", VIR_Shader_GetId(Shader),
                    VSC_OPTN_DUAL16Options_AfterShader(options), VSC_OPTN_DUAL16Options_BeforeShader(options));
            VIR_LOG_FLUSH(dumper);
        }
        return errCode;
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
                gctBOOL     isDual16Highpvec2 = gcvFALSE;
                if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
                {
                    VIR_Inst_Dump(dumper, pInst);
                }

                /* count the MOV instructions which may be added in vscVIR_CheckCstRegFileReadPortLimitation() */
                runSignleTInstCount += _CheckCstRegFileReadPortLimitation(Shader, pInst, bHasOneConstFix);

                /* check dest */
                if (VIR_Inst_Dual16NotSupported(pInst)  ||
                    /* A WAR to disable dual16 for some CTS cases because our HW can't support denormalize F16. */
                    (isAppConformance && (VIR_Inst_GetOpcode(pInst) == VIR_OP_SINPI || VIR_Inst_GetOpcode(pInst) == VIR_OP_COSPI)))
                {
                    if(VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_DETAIL))
                    {
                        VIR_LOG(dumper, "inst not supported by dual16.\n", i);
                        VIR_LOG_FLUSH(dumper);
                    }
                    dual16NotSupported = gcvTRUE;
                    break;
                }

                VIR_Inst_Check4Dual16(pInst, &needRunSingleT, &dual16NotSupported, &isDual16Highpvec2, options, dumper, HWSUPPORTDUAL16HIGHVEC2);
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
                    if (opcode == VIR_OP_JMPC || opcode == VIR_OP_JMP_ANY)
                    {
                        gctUINT            j;
                        VIR_Operand*       opnd;
                        VIR_ConstVal       immValues[VIR_MAX_SRC_NUM];
                        VIR_TypeId         immTypeId[VIR_MAX_SRC_NUM];

                        /* see _InsertCMPInst, cmp will be inserted before jmpc/jmp */
                        runSignleTInstCount = runSignleTInstCount + 2;

                        /* see _NeedPutImmValue2Uniform(), immediate numbers may be converted to uniforms in dual16 mode.
                           Then in vscVIR_CheckCstRegFileReadPortLimitation(), a MOV instruction may be inserted due to read port limitation.
                           Here we count 2 more instructions for the inserted MOV.t0t1 instruction can be splited to two insructions. */
                        for (j = 0; j < VIR_Inst_GetSrcNum(pInst); ++j)
                        {
                            opnd = VIR_Inst_GetSource(pInst, i);
                            if (opnd && _NeedPutImmValue2Uniform(Shader, opnd, pHwCfg, gcvTRUE, gcvFALSE, &immValues[i], &immTypeId[i]))
                            {
                                runSignleTInstCount = runSignleTInstCount + 2;
                                break;
                            }
                        }
                    }
                    else
                    {
                        runSignleTInstCount++;
                    }
                    VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_32);

                    /* VIV when hw has no half-dep fix, there is force depedence among
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
                else if (isDual16Highpvec2)
                {
                    VIR_Inst_SetThreadMode(pInst, VIR_THREAD_D16_DUAL_HIGHPVEC2);
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

    /* In dual16, we need to put all HP attributes in front of MP attributes */
    _SortAttributesOfDual16Shader(Shader, &compCfg->ctx.pSysCtx->pCoreSysCtx->hwCfg);

    VIR_Shader_SetDual16Mode(Shader, gcvTRUE);

    if(VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(Shader), VIR_Shader_GetId(Shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE) ||
       VSC_UTILS_MASK(VSC_OPTN_DUAL16Options_GetTrace(options), VSC_OPTN_DUAL16Options_TRACE_OUTPUT))
    {
        VIR_Shader_Dump(gcvNULL, "After dual16 shader transform.", Shader, gcvTRUE);
    }

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
    /* Check parameters. */
    else if (VIR_Operand_isParameters(pOpnd))
    {
        VIR_ParmPassing *parm = VIR_Operand_GetParameters(pOpnd);

        for (i = 0; i < parm->argNum; i++)
        {
            _CheckOperandForVarUsage(pShader, pInst, CheckVarUsage,
                                     parm->args[i]);
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
    IN  gctBOOL                  SetFlag,
    IN  gctBOOL                  CheckPerVertex
    )
{
    VIR_Symbol                  *sym;
    gctUINT                      i;

    for (i = 0; i < VIR_IdList_Count(IdList); i++)
    {
        sym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(IdList, i));

        if (isSymArrayedPerVertex(sym) && !CheckPerVertex)
        {
            continue;
        }

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

DEF_SH_NECESSITY_CHECK(vscVIR_CheckVariableUsage)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_CheckVariableUsage(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader                 *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_CHECK_VAR_USAGE        *checkVarUsage = (VIR_CHECK_VAR_USAGE *)pPassWorker->basePassWorker.pPassSpecificData;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode           *func_node;

    if (checkVarUsage->checkInput)
    {
        /* we check per-vertex attribute for TCS, since VS shader output is always checked */
        if (VIR_Shader_GetKind(pShader) == VIR_SHADER_TESSELLATION_CONTROL ||
            VIR_Shader_GetKind(pShader) == VIR_SHADER_GEOMETRY)
        {
            _InitUsageFlag(pShader, VIR_Shader_GetAttributes(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE, gcvTRUE);
        }
        else
        {
            _InitUsageFlag(pShader, VIR_Shader_GetAttributes(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE, gcvFALSE);
        }
    }
    if (checkVarUsage->checkOutput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetOutputs(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE, gcvFALSE);
    }
    if (checkVarUsage->checkPrePatchInput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetPerpatchAttributes(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE, gcvFALSE);
    }
    if (checkVarUsage->checkPrePatchOutput)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetPerpatchOutputs(pShader), VIR_SYMFLAG_UNUSED, gcvTRUE, gcvFALSE);
    }
    if (checkVarUsage->checkUniform)
    {
        _InitUsageFlag(pShader, VIR_Shader_GetUniforms(pShader), VIR_SYMUNIFORMFLAG_USED_IN_SHADER | VIR_SYMUNIFORMFLAG_USED_IN_LTC, gcvFALSE, gcvFALSE);
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


    return errCode;
}

static VSC_ErrCode _InsertInitializeInst(
    VIR_DEF_USAGE_INFO*         pDuInfo,
    VIR_Shader*                 pShader,
    VIR_Function*               pFunc,
    VIR_Symbol*                 pSym,
    VIR_USAGE*                  pUsage,
    VIR_TypeId                  typeId,
    gctUINT                     firstRegNo,
    gctUINT                     regNoRange,
    VIR_Enable                  defEnableMask,
    gctUINT8                    halfChannelMask,
    gctUINT                     initValue,
    gctBOOL                     bOutUage
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Instruction*            pNewMovInst = gcvNULL;
    VIR_Operand*                opnd = gcvNULL;

    /* Insert 'MOV reg, 0' at the begin of function */
    VIR_Function_PrependInstruction(pFunc,
                                    VIR_OP_MOV,
                                    typeId,
                                    &pNewMovInst);

    /* Dst */
    opnd = VIR_Inst_GetDest(pNewMovInst);
    VIR_Operand_SetSymbol(opnd, pFunc, pSym->index);
    VIR_Operand_SetEnable(opnd, defEnableMask);
    VIR_Operand_SetPrecision(opnd, VIR_Symbol_GetPrecision(pSym));
    VIR_Operand_SetTypeId(opnd, typeId);

    /* Src */
    opnd = VIR_Inst_GetSource(pNewMovInst, VIR_Operand_Src0);
    if (VIR_TypeId_isSignedInteger(typeId) ||
        VIR_TypeId_isBoolean(typeId))
    {
        VIR_Operand_SetImmediateUint(opnd, initValue);
    }
    else if (VIR_TypeId_isUnSignedInteger(typeId))
    {
        VIR_Operand_SetImmediateInt(opnd, initValue);
    }
    else
    {
        VIR_Operand_SetImmediateFloat(opnd, (gctFLOAT)initValue);
    }
    VIR_Operand_SetPrecision(opnd, VIR_PRECISION_HIGH);

    vscVIR_AddNewDef(pDuInfo, pNewMovInst, firstRegNo, regNoRange,
                     defEnableMask, halfChannelMask, gcvNULL, gcvNULL);

    if (pUsage != gcvNULL)
    {
        vscVIR_AddNewUsageToDef(pDuInfo,
                                pNewMovInst,
                                pUsage->usageKey.pUsageInst,
                                pUsage->usageKey.pOperand,
                                pUsage->usageKey.bIsIndexingRegUsage,
                                firstRegNo,
                                regNoRange,
                                defEnableMask,
                                halfChannelMask, gcvNULL);
    }
    else if (bOutUage)
    {
        vscVIR_AddNewUsageToDef(pDuInfo,
                                pNewMovInst,
                                VIR_OUTPUT_USAGE_INST,
                                (VIR_Operand*)(gctUINTPTR_T)firstRegNo,
                                gcvFALSE,
                                firstRegNo,
                                regNoRange,
                                defEnableMask,
                                halfChannelMask,
                                gcvNULL);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_InitializeVariables)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML | VSC_PASS_LEVEL_LL;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_InitializeVariables)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_InitializeVariables(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader*                 pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*         pDuInfo = pPassWorker->pDuInfo;
    VSC_BLOCK_TABLE*            pUsageTable = &pDuInfo->usageTable;
    VIR_USAGE*                  pUsage;
    VIR_Operand*                pUsageOpnd = gcvNULL;
    VIR_Instruction*            pUsageInst = gcvNULL;
    VIR_Swizzle                 usageSwizzle;
    VIR_DEF*                    pDef;
    gctUINT                     i, j, usageCount, usageIdx, defIdx;
    VIR_OperandInfo             operandInfo, operandInfo1;
    gctUINT                     firstRegNo = 0, regNoRange = 0;
    VIR_Enable                  defEnableMask = VIR_ENABLE_NONE;
    gctUINT8                    halfChannelMask = 0;
    VIR_Symbol*                 pSym = gcvNULL;
    gctBOOL                     bNeedInitialization;
    gctBOOL                     bNeedToCheckDef;
    gctBOOL                     bChanged = gcvFALSE;
    gctUINT                     initValue = 0;

    VIR_Shader_RenumberInstId(pShader);

    /* Check all usages. */
    usageCount = BT_GET_MAX_VALID_ID(&pDuInfo->usageTable);
    for (usageIdx = 0; usageIdx < usageCount; usageIdx ++)
    {
        pUsage = GET_USAGE_BY_IDX(pUsageTable, usageIdx);
        if (IS_VALID_USAGE(pUsage))
        {
            gctBOOL bChannelInitialized[VIR_CHANNEL_COUNT] = {gcvTRUE, gcvTRUE, gcvTRUE, gcvTRUE};

            pUsageInst = pUsage->usageKey.pUsageInst;
            pUsageOpnd = pUsage->usageKey.pOperand;

            /* If the usage has no def, we must initialize it */
            bNeedInitialization = (UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain) == 0);
            bNeedToCheckDef = gcvTRUE;

            /* Don't need to a combined sampler. */
            if (bNeedInitialization &&
                !VIR_IS_IMPLICIT_USAGE_INST(pUsageInst) &&
                pUsageOpnd &&
                VIR_Operand_isSymbol(pUsageOpnd))
            {
                VIR_Symbol*     pOpndSym = VIR_Operand_GetSymbol(pUsageOpnd);

                if (isSymCombinedSampler(pOpndSym))
                {
                    bNeedInitialization = gcvFALSE;
                    bNeedToCheckDef = gcvFALSE;
                }
            }

            /* Even usage has defs, however, some of pathes might have no definition. So far,
               only loop is checked. */

            if (!bNeedInitialization && bNeedToCheckDef && !VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
            {
                gctBOOL bSkipCheck = gcvFALSE;

                usageSwizzle = VIR_Operand_GetSwizzle(pUsageOpnd);
                bNeedInitialization = gcvTRUE;

                /* Only check the used channel. */
                for (i = 0; i < VIR_CHANNEL_COUNT; i++)
                {
                    if (VIR_Inst_isComponentwise(pUsageInst))
                    {
                        if ((1 << i) & pUsage->realChannelMask)
                        {
                            bChannelInitialized[VIR_Swizzle_GetChannel(usageSwizzle, i)] = gcvFALSE;
                        }
                    }
                    else
                    {
                        bChannelInitialized[VIR_Swizzle_GetChannel(usageSwizzle, i)] = gcvFALSE;
                    }
                }

                for (i = 0; i < UD_CHAIN_GET_DEF_COUNT(&pUsage->udChain); i ++)
                {
                    defIdx = UD_CHAIN_GET_DEF(&pUsage->udChain, i);
                    gcmASSERT(VIR_INVALID_DEF_INDEX != defIdx);
                    pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);

                    /* Skip some checks. */
                    if (VIR_IS_IMPLICIT_DEF_INST(pDef->defKey.pDefInst))
                    {
                        bNeedInitialization = gcvFALSE;
                        bSkipCheck = gcvTRUE;
                        break;
                    }

                    if (VIR_Inst_GetFunction(pDef->defKey.pDefInst) != VIR_Inst_GetFunction(pUsageInst))
                    {
                        bNeedInitialization = gcvFALSE;
                        bSkipCheck = gcvTRUE;
                        break;
                    }

                    if (!VIR_IS_OUTPUT_USAGE_INST(pDef->defKey.pDefInst))
                    {
                        if (VIR_Inst_GetId(pDef->defKey.pDefInst) < VIR_Inst_GetId(pUsageInst))
                        {
                            bNeedInitialization = gcvFALSE;
                        }
                    }

                    for (j = 0; j < VIR_CHANNEL_COUNT; j++)
                    {
                        if (pDef->OrgEnableMask & (VIR_ENABLE_X << j))
                        {
                            bChannelInitialized[j] = gcvTRUE;
                        }
                    }
                }

                /* Check if any channel is uninitialized. */
                if (!bNeedInitialization && !bSkipCheck)
                {
                    for (i = 0; i < VIR_CHANNEL_COUNT; i++)
                    {
                        if (!bChannelInitialized[i])
                        {
                            bNeedInitialization = gcvTRUE;
                            break;
                        }
                    }
                }
            }

            if (bNeedInitialization)
            {
                bChanged = gcvTRUE;

                VIR_Operand_GetOperandInfo(pUsageInst,
                                           pUsageOpnd,
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
                    if (VIR_Inst_GetOpcode(pUsageInst) == VIR_OP_LDARR)
                    {
                        if (pUsageOpnd == VIR_Inst_GetSource(pUsageInst, VIR_Operand_Src0))
                        {
                            VIR_Operand_GetOperandInfo(pUsageInst,
                                                       VIR_Inst_GetSource(pUsageInst, VIR_Operand_Src1),
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

                            defEnableMask = VIR_Operand_GetRealUsedChannels(pUsageOpnd,
                                                                            pUsageInst,
                                                                            gcvNULL);
                            halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
                        }
                        else if (pUsageOpnd == VIR_Inst_GetSource(pUsageInst, VIR_Operand_Src1))
                        {
                            firstRegNo = operandInfo.u1.virRegInfo.virReg;
                            regNoRange = 1;
                            defEnableMask = VIR_Operand_GetRealUsedChannels(pUsageOpnd,
                                                                            pUsageInst,
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

                        defEnableMask = VIR_Operand_GetRealUsedChannels(pUsageOpnd,
                                                                        pUsageInst,
                                                                        gcvNULL);
                        halfChannelMask = (gctUINT8)operandInfo.halfChannelMask;
                    }

                    pSym = VIR_Operand_GetSymbol(pUsageOpnd);
                }

                /* Insert 'MOV reg, 0' at the begin of function */
                errCode = _InsertInitializeInst(pDuInfo,
                                                pShader,
                                                VIR_Inst_GetFunction(pUsageInst),
                                                pSym,
                                                pUsage,
                                                VIR_Operand_GetTypeId(pUsageOpnd),
                                                firstRegNo,
                                                regNoRange,
                                                defEnableMask,
                                                halfChannelMask,
                                                initValue,
                                                gcvFALSE);
                ON_ERROR(errCode, "Add MOV instruction.");

                bChanged = gcvTRUE;
            }
        }
    }

    /*
    ** Check if there is any no-assignment output.
    ** Right now only check gl_PointSize for GS to fix dEQP-VK.glsl.builtin.function.common.abs.float_highp_geometry.
    *  Another check gl_PointSize in vertex shader if it has define or not
    */
    if (VIR_Shader_GetKind(pShader) == VIR_SHADER_GEOMETRY ||
        VIR_Shader_GetKind(pShader) == VIR_SHADER_VERTEX)
    {
        VIR_OutputIdList*       pOutputList = VIR_Shader_GetOutputs(pShader);
        gctUINT                 outputCount = VIR_IdList_Count(pOutputList);
        gctUINT                 defIdx;
        VIR_SymId               vregSymId = VIR_INVALID_ID;

        for (i = 0; i < outputCount; i++)
        {
            initValue = 0;
            pSym = VIR_Shader_GetSymFromId(pShader, VIR_IdList_GetId(pOutputList, i));

            if (VIR_Symbol_GetName(pSym) == VIR_NAME_POINT_SIZE)
            {
                defIdx = vscVIR_FindFirstDefIndex(pDuInfo, VIR_Symbol_GetVregIndex(pSym));

                if (VIR_Shader_GetKind(pShader) == VIR_SHADER_VERTEX)
                {
                    /* if vertex shader, do nothing except set UNDef flag */
                    if (VIR_INVALID_DEF_INDEX == defIdx)
                    {
                        VIR_Symbol_SetFlag(pSym, VIR_SYMFLAG_UNDEF);
                    }
                    else
                    {
                        VIR_Symbol_ClrFlag(pSym, VIR_SYMFLAG_UNDEF);
                    }
                    continue;
                }
                if (VIR_INVALID_DEF_INDEX != defIdx)
                {
                    continue;
                }

                /* The default value of PointSize is 1.0. */
                initValue = 1;

                /* Insert 'MOV reg, 1' at the begin of function */
                errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader,
                                                            VIR_Symbol_GetVregIndex(pSym),
                                                            &vregSymId);
                ON_ERROR(errCode, "Get vreg symbol.");

                errCode = _InsertInitializeInst(pDuInfo,
                                                pShader,
                                                VIR_Shader_GetMainFunction(pShader),
                                                VIR_Shader_GetSymFromId(pShader, vregSymId),
                                                gcvNULL,
                                                VIR_Symbol_GetTypeId(pSym),
                                                VIR_Symbol_GetVregIndex(pSym),
                                                1,
                                                VIR_TypeId_Conv2Enable(VIR_Symbol_GetTypeId(pSym)),
                                                VIR_HALF_CHANNEL_MASK_FULL,
                                                initValue,
                                                gcvTRUE);
                ON_ERROR(errCode, "Add MOV instruction.");

                bChanged = gcvTRUE;
            }
        }
    }

    /* Basicly if we meet a uninitialized variable, there must be something wrong with the optimization. */
    if (bChanged)
    {
        WARNING_REPORT(VSC_ERR_INVALID_DATA, "Get some uninitialized variables, please double check!!!");
    }

    if (bChanged &&
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After variable initialization", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}

static gctBOOL
_NeedToCalculateOffset(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Operand      *pOffsetOpnd
    )
{
    gctBOOL needToCalculate = gcvTRUE;

    switch (VIR_Operand_GetOpKind(pOffsetOpnd))
    {
    case VIR_OPND_IMMEDIATE:
        if (VIR_Operand_GetImmediateInt(pOffsetOpnd) == 0)
        {
            needToCalculate = gcvFALSE;
        }
        break;

    case VIR_OPND_CONST:
    {
        VIR_Const   *vConst = gcvNULL;
        gctINT      i;

        vConst = VIR_Shader_GetConstFromId(pShader,
            VIR_Operand_GetConstId(pOffsetOpnd));
        for (i = 0; i < 16; i++)
        {
            if (vConst->value.vecVal.i32Value[i] != 0)
            {
                break;
            }
        }
        if (i == 16)
        {
            needToCalculate = gcvFALSE;
        }
    }
    break;

    default:
        break;
    }

    return needToCalculate;
}

static VSC_ErrCode
_GetSamplerInfo(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    INOUT   VIR_SymId        *LevelBaseSizeSymId,
    INOUT   VIR_SymId        *LodMinMaxSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand                *pSamplerOpnd = VIR_Inst_GetSource(pInst, 0);
    VIR_SymId                   levelBaseSizeSymId = VIR_INVALID_ID, lodMinMaxSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;

    /* Create a symbol to save levelBaseSizeSymId. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X4),
        VIR_STORAGE_UNKNOWN,
        &levelBaseSizeSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    /* Get the levelBaseSize. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_GET_SAMPLER_LBS,
        VIR_TYPE_INTEGER_X4,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        levelBaseSizeSymId,
        VIR_TYPE_INTEGER_X4);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XYZW);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pSamplerOpnd);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateUint(operand, 0);

    /* Create a symbol to save lodMinMaxSymId. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INTEGER_X4),
        VIR_STORAGE_UNKNOWN,
        &lodMinMaxSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    /* Get the lodMinMax. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_GET_SAMPLER_LMM,
        VIR_TYPE_INTEGER_X4,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        lodMinMaxSymId,
        VIR_TYPE_INTEGER_X4);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XYZW);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pSamplerOpnd);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateUint(operand, 0);

    /* Save the result. */
    if (LevelBaseSizeSymId)
    {
        *LevelBaseSizeSymId = levelBaseSizeSymId;
    }
    if (LodMinMaxSymId)
    {
        *LodMinMaxSymId = lodMinMaxSymId;
    }

OnError:
    return errCode;
}

/* lod = floor(clamp(lod, min, max) + 0.5). */
static VSC_ErrCode
_ClampFloatLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LodMinMaxSymId,
    IN      VIR_Operand      *pLodOpnd,
    INOUT   VIR_SymId        *pFloatLodSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_SymId                   floatLodSymId = VIR_INVALID_ID;
    VIR_SymId                   floatLodMinMaxSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;

    if (pFloatLodSymId)
    {
        floatLodSymId = *pFloatLodSymId;
    }

    if (floatLodSymId == VIR_INVALID_ID)
    {
        /* Create a reg to clamp the LOD. */
        regId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
            VIR_SYM_VIRREG,
            regId,
            VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT32),
            VIR_STORAGE_UNKNOWN,
            &floatLodSymId);
        ON_ERROR(errCode, "VIR_Shader_AddSymbol failed.");
    }

    /* Convert the lodMinMax to float. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X2),
        VIR_STORAGE_UNKNOWN,
        &floatLodMinMaxSymId);
    ON_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_CONVERT,
        VIR_TYPE_FLOAT_X2,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodMinMaxSymId,
        VIR_TYPE_FLOAT_X2);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_XY);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        LodMinMaxSymId,
        VIR_TYPE_INTEGER_X2);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XYYY);

    /* Clamp the float lod.*/
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MAX,
        VIR_TYPE_FLOAT32,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    if (pLodOpnd)
    {
        VIR_Operand_Copy(operand, pLodOpnd);
    }
    else
    {
        gcmASSERT(floatLodSymId != VIR_INVALID_ID);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            floatLodSymId,
            VIR_TYPE_FLOAT32);
    }
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodMinMaxSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MIN,
        VIR_TYPE_FLOAT32,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodMinMaxSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_YYYY);

    /* ADD: floatLod, floatLod, 0.5. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_ADD,
        VIR_TYPE_FLOAT32,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    operand = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetImmediateFloat(operand, 0.5);

    /* FLOOR: floatLod, floatLod. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_FLOOR,
        VIR_TYPE_FLOAT32,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        floatLodSymId,
        VIR_TYPE_FLOAT32);
    VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

    if (pFloatLodSymId)
    {
        *pFloatLodSymId = floatLodSymId;
    }
OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateFloatLodByGrad(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *pFloatLodSymId
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_VirRegId        regId = VIR_INVALID_ID;
    VIR_Operand * texldModifier = (VIR_Operand *)VIR_Inst_GetSource(pInst, 2);
    gctUINT             coordCompCount;
    VIR_SymId           dpdxSymId = VIR_INVALID_ID, dpdySymId = VIR_INVALID_ID;
    VIR_TypeId          gradTypeId = VIR_INVALID_ID;
    VIR_Instruction*    inst = gcvNULL;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_SymId           floatLevelBaseSizeSymId = VIR_INVALID_ID;

    coordCompCount = VIR_TypeId_GetSamplerCoordComponentCount(VIR_Operand_GetTypeId(VIR_Inst_GetSource(pInst, 0)));
    gradTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, coordCompCount, 1);

    /* Create two regs to hold dpdx/dpdy. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, gradTypeId),
        VIR_STORAGE_UNKNOWN,
        &dpdxSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, gradTypeId),
        VIR_STORAGE_UNKNOWN,
        &dpdySymId);
    ON_ERROR(errCode, "Add symbol failed.");

    /* Get the dpdx and dpdy. */
    if (VIR_Operand_hasGradFlag(texldModifier))
    {
        VIR_Operand*    pDpdxOpnd = gcvNULL;
        VIR_Operand*    pDpdyOpnd = gcvNULL;

        pDpdxOpnd = VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDX);
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
            gradTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            dpdxSymId,
            gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pDpdxOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

        pDpdyOpnd = VIR_Operand_GetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_DPDY);
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
            gradTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            dpdySymId,
            gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pDpdyOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));
    }
    else
    {
        VIR_Operand*    pCoordOpnd = VIR_Inst_GetSource(pInst, 1);

        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DSX,
            gradTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            dpdxSymId,
            gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pCoordOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DSY,
            gradTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            dpdySymId,
            gradTypeId);
        VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

        pOpnd = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(pOpnd, pCoordOpnd);
        VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));
    }

    /* Convert the levelBaseSize to float. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_FLOAT_X3),
        VIR_STORAGE_UNKNOWN,
        &floatLevelBaseSizeSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_CONVERT,
        VIR_TYPE_FLOAT_X3,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        floatLevelBaseSizeSymId,
        VIR_TYPE_FLOAT_X3);
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_XYZ);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        LevelBaseSizeSymId,
        VIR_TYPE_INTEGER_X3);
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XYZZ);

    /* MUL, dpdx, dpdx, size. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MUL,
        gradTypeId,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        gradTypeId);
    VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        floatLevelBaseSizeSymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* MUL, dpdy, dpdy, size. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MUL,
        gradTypeId,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        gradTypeId);
    VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        floatLevelBaseSizeSymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* DOT, dpdx.x, dpdx, dpdx. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DOT,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* SQRT dpdx.x, dpdx.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_SQRT,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* DOT, dpdy.x, dpdy, dpdy. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DOT,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        gradTypeId);
    VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(gradTypeId));

    /* SQRT, dpdy.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_SQRT,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* LOG2, dpdx.x, dpdx.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_LOG2,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* LOG2, dpdy.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_LOG2,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* MAX, dpdx.x, dpdx.x, dpdy.x. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MAX,
        VIR_GetTypeComponentType(gradTypeId),
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    pOpnd = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

    pOpnd = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdxSymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    pOpnd = VIR_Inst_GetSource(inst, 1);
    VIR_Operand_SetTempRegister(pOpnd,
        pFunc,
        dpdySymId,
        VIR_GetTypeComponentType(gradTypeId));
    VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

    /* Clamp the lod. */
    errCode = _ClampFloatLod(pShader,
        pFunc,
        pInst,
        LodMinMaxSymId,
        gcvNULL,
        &dpdxSymId);
    ON_ERROR(errCode, "Clamp float lod");

    if (pFloatLodSymId)
    {
        *pFloatLodSymId = dpdxSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateIntegerLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *pIntLodSymId
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Instruction*    pCalcLodInst = gcvNULL;
    VIR_Operand*        pParamSrc;
    VIR_Operand*        pSamplerOpnd = gcvNULL;
    VIR_Operand*        pCoordOpnd = gcvNULL;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_VirRegId        lodRegId = VIR_INVALID_ID;
    VIR_SymId           intLodSymId = VIR_INVALID_ID;
    VIR_SymId           newCoordSymId = VIR_INVALID_ID;
    VIR_TypeId          coordTypeId;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    gctBOOL             isTexldProj;

    isTexldProj = (opCode == VIR_OP_TEXLDPROJ || opCode == VIR_OP_TEXLDPCFPROJ);

    pParamSrc = VIR_Inst_GetSource(pInst, 2);

    /* Get the sampler and coord operand. */
    pSamplerOpnd = VIR_Inst_GetSource(pInst, 0);
    pCoordOpnd = VIR_Inst_GetSource(pInst, 1);

    /* Get coord type. */
    coordTypeId = VIR_Operand_GetTypeId(pCoordOpnd);

    /* Create a reg to hold the LOD. */
    lodRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        lodRegId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
        VIR_STORAGE_UNKNOWN,
        &intLodSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    /*
    ** 1) With explicit gradients, then we need to calculate lod with these gradients.
    ** 2) Without explicit gradients and in FS, use LODQ to get the lod.
    ** 3) Otherwise, use 0 as LOD.
    */
    if (VIR_Operand_hasGradFlag(pParamSrc))
    {
        VIR_SymId   floatLodSymId = VIR_INVALID_ID;
        VIR_Instruction* convInst = gcvNULL;

        errCode = _CalculateFloatLodByGrad(pShader,
            pFunc,
            pInst,
            LevelBaseSizeSymId,
            LodMinMaxSymId,
            &floatLodSymId);
        ON_ERROR(errCode, "Calculate lod by grad failed.");

        /* Conv the float LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_CONVERT,
            VIR_TYPE_INT32,
            pInst,
            gcvTRUE,
            &convInst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(convInst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            intLodSymId,
            VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(convInst, 0);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            floatLodSymId,
            VIR_TYPE_FLOAT32);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);
    }
    else if (VIR_Shader_IsFS(pShader))
    {
        /* If this is a texldproj, we need to div the q component. */
        if (isTexldProj)
        {
            VIR_Swizzle     swizzle;
            VIR_Instruction*pCoordInst = gcvNULL;
            VIR_VirRegId    coordRegId = VIR_INVALID_ID;
            VIR_TypeId      newCoordTypeId;

            /* Create a reg to save the coord. */
            coordRegId = VIR_Shader_NewVirRegId(pShader, 1);
            errCode = VIR_Shader_AddSymbol(pShader,
                VIR_SYM_VIRREG,
                coordRegId,
                VIR_Shader_GetTypeFromId(pShader, coordTypeId),
                VIR_STORAGE_UNKNOWN,
                &newCoordSymId);
            ON_ERROR(errCode, "Add symbol failed.");

            errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
                coordTypeId,
                pInst,
                gcvTRUE,
                &pCoordInst);
            ON_ERROR(errCode, "Add instruction failed.");

            /* Set Dest. */
            pOpnd = VIR_Inst_GetDest(pCoordInst);
            VIR_Operand_SetTempRegister(pOpnd,
                pFunc,
                newCoordSymId,
                coordTypeId);
            VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(coordTypeId));

            /* Set Source0. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 0);
            VIR_Operand_Copy(pOpnd, pCoordOpnd);

            /* Div the q component. */
            newCoordTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(coordTypeId),
                VIR_GetTypeComponents(coordTypeId) - 1,
                1);

            errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DIV,
                newCoordTypeId,
                pInst,
                gcvTRUE,
                &pCoordInst);
            ON_ERROR(errCode, "Add instruction failed.");

            /* Set Dest. */
            pOpnd = VIR_Inst_GetDest(pCoordInst);
            VIR_Operand_SetTempRegister(pOpnd,
                pFunc,
                newCoordSymId,
                newCoordTypeId);
            VIR_Operand_SetEnable(pOpnd, VIR_TypeId_Conv2Enable(newCoordTypeId));

            /* Set Source0. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 0);
            VIR_Operand_SetTempRegister(pOpnd,
                pFunc,
                newCoordSymId,
                newCoordTypeId);
            VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(newCoordTypeId));

            /* Set Source1. */
            pOpnd = VIR_Inst_GetSource(pCoordInst, 1);
            VIR_Operand_Copy(pOpnd, pCoordOpnd);
            swizzle = VIR_Operand_GetSwizzle(pCoordOpnd);
            swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
            VIR_Operand_SetSwizzle(pOpnd, VIR_Enable_2_Swizzle_WShift((VIR_Enable)(VIR_ENABLE_X << swizzle)));
            VIR_Operand_SetTypeId(pOpnd, VIR_GetTypeComponentType(coordTypeId));
        }

        /* Insert a intrinsic call to get LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_LODQ,
            VIR_TYPE_INT32,
            pInst,
            gcvTRUE,
            &pCalcLodInst);
        ON_ERROR(errCode, "Add instruction failed.");

        VIR_Inst_SetResOpType(pCalcLodInst, VIR_RES_OP_TYPE_LODQ);

        pOpnd = VIR_Inst_GetDest(pCalcLodInst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            intLodSymId,
            VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 0);
        VIR_Operand_Copy(pOpnd, pSamplerOpnd);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 1);
        if (isTexldProj)
        {
            VIR_Operand_SetTempRegister(pOpnd,
                pFunc,
                newCoordSymId,
                coordTypeId);
            VIR_Operand_SetSwizzle(pOpnd, VIR_TypeId_Conv2Swizzle(coordTypeId));
        }
        else
        {
            VIR_Operand_Copy(pOpnd, pCoordOpnd);
        }

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 2);
        if (VIR_Operand_hasBiasFlag(pParamSrc))
        {
            VIR_Operand * texldModifier = (VIR_Operand *)pParamSrc;

            VIR_Operand_Copy(pOpnd, VIR_Operand_GetTexldBias(pParamSrc));
            /* Remove the bias. */
            VIR_Operand_ClrTexModifierFlag(pParamSrc, VIR_TMFLAG_BIAS);
            VIR_Operand_SetTexldModifier(texldModifier, VIR_TEXLDMODIFIER_BIAS, gcvNULL);
        }
        else
        {
            VIR_Operand_SetImmediateFloat(pOpnd, 0.0);
        }
    }
    else
    {
        /* Use 0 as LOD. */
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
            VIR_TYPE_INT32,
            pInst,
            gcvTRUE,
            &pCalcLodInst);
        ON_ERROR(errCode, "Add instruction failed.");

        pOpnd = VIR_Inst_GetDest(pCalcLodInst);
        VIR_Operand_SetTempRegister(pOpnd,
            pFunc,
            intLodSymId,
            VIR_TYPE_INT32);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pCalcLodInst, 0);
        VIR_Operand_SetImmediateInt(pOpnd, 0);
    }

    if (pIntLodSymId)
    {
        *pIntLodSymId = intLodSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_GetIntegerLod(
    IN      VIR_Shader       *pShader,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst,
    IN      VIR_SymId         LevelBaseSizeSymId,
    IN      VIR_SymId         LodMinMaxSymId,
    INOUT   VIR_SymId        *IntgerLodSymId
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand  *pTexldModifier = (VIR_Operand *)VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pLodOpnd = VIR_Operand_GetTexldLod(pTexldModifier);
    VIR_SymId                   intLodSymId = VIR_INVALID_ID, floatLodSymId = VIR_INVALID_ID;
    VIR_VirRegId                regId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;
    gctBOOL                     isFloatLod;

    /* If there is not lod modifier, then we need to calculate it. */
    if (pLodOpnd == gcvNULL)
    {
        errCode = _CalculateIntegerLod(pShader,
            pFunc,
            pInst,
            LevelBaseSizeSymId,
            LodMinMaxSymId,
            IntgerLodSymId);
        ON_ERROR(errCode, "Calculate lod failed.");

        return errCode;
    }

    isFloatLod = VIR_TypeId_isFloat(VIR_Operand_GetTypeId(pLodOpnd));

    /* Create a temp to hold the integer LOD. */
    regId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        regId,
        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_INT32),
        VIR_STORAGE_UNKNOWN,
        &intLodSymId);
    ON_ERROR(errCode, "Add symbol failed.");

    if (isFloatLod)
    {
        /* If the LOD is floating variable from shader source, we need to clamp it. */
        errCode = _ClampFloatLod(pShader,
            pFunc,
            pInst,
            LodMinMaxSymId,
            pLodOpnd,
            &floatLodSymId);
        ON_ERROR(errCode, "Clamp float lod failed.");
    }

    /* Move the integer LOD. */
    errCode = VIR_Function_AddInstructionBefore(pFunc, isFloatLod ? VIR_OP_CONVERT : VIR_OP_MOV,
        VIR_TYPE_INT32,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction failed.");

    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        intLodSymId,
        VIR_TYPE_INT32);
    VIR_Operand_SetEnable(operand, VIR_ENABLE_X);

    operand = VIR_Inst_GetSource(inst, 0);
    if (!isFloatLod)
    {
        VIR_Operand_Copy(operand, pLodOpnd);
    }
    else
    {
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            floatLodSymId,
            VIR_TYPE_FLOAT32);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);
    }

    /* Return the LOD symbol ID. */
    if (IntgerLodSymId)
    {
        *IntgerLodSymId = intLodSymId;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateOffsetCoord(
    IN      VIR_Shader       *pShader,
    IN      VSC_HW_CONFIG    *HwCfg,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_OpCode                  opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand                *pCoordOpnd = VIR_Inst_GetSource(pInst, 1);
    VIR_TypeId                  coordTypeId = VIR_Operand_GetTypeId(pCoordOpnd);
    VIR_TypeId                  newCoordTypeId = VIR_INVALID_ID;
    VIR_Type                   *pCoordType = VIR_Shader_GetTypeFromId(pShader, coordTypeId);
    VIR_Operand                *pParamOpnd = VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pOffsetOpnd = VIR_Operand_GetTexldOffset(pParamOpnd);
    VIR_TypeId                  offsetTypeId = VIR_Operand_GetTypeId(pOffsetOpnd);
    VIR_Type                   *pOffsetType = VIR_Shader_GetTypeFromId(pShader, offsetTypeId);
    VIR_SymId                   intLodSymId = VIR_INVALID_ID;
    VIR_SymId                   levelBaseSizeSymId = VIR_INVALID_ID, lodMinMaxSymId = VIR_INVALID_ID;
    VIR_SymId                   newCoordSymId = VIR_INVALID_ID, sizeSymId, newOffsetSymId = VIR_INVALID_ID;
    VIR_VirRegId                tempRegId = VIR_INVALID_ID;
    VIR_Instruction            *inst = gcvNULL;
    VIR_Operand                *operand = gcvNULL;
    gctUINT                     offsetCompCount = VIR_GetTypeComponents(offsetTypeId);
    gctBOOL                     addOffsetOnly = gcvFALSE;
    gctBOOL                     isTexldProj = gcvFALSE;
    gctBOOL                     isTexGather = gcvFALSE;

    if (VIR_Type_isInteger(pCoordType) && VIR_Type_isInteger(pOffsetType))
    {
        gcmASSERT(opCode == VIR_OP_TEXLD_U);

        if (HwCfg->hwFeatureFlags.hasUniversalTexldV2 &&
            HwCfg->hwFeatureFlags.hasTexldUFix)
        {
            addOffsetOnly = gcvTRUE;
        }
    }

    if (opCode == VIR_OP_TEXLDPROJ || opCode == VIR_OP_TEXLDPCFPROJ)
    {
        isTexldProj = gcvTRUE;
        newCoordTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_GetTypeComponentType(coordTypeId),
            VIR_GetTypeComponents(coordTypeId) - 1,
            1);
    }

    if (VIR_Operand_hasGatherFlag(pParamOpnd))
    {
        isTexGather = gcvTRUE;
    }

    if (!addOffsetOnly)
    {
        /* Get sampler info. */
        errCode = _GetSamplerInfo(pShader,
            pFunc,
            pInst,
            &levelBaseSizeSymId,
            &lodMinMaxSymId);
        ON_ERROR(errCode, "Get sampler info.");

        /* Get the LOD. */
        errCode = _GetIntegerLod(pShader,
            pFunc,
            pInst,
            levelBaseSizeSymId,
            lodMinMaxSymId,
            &intLodSymId);
        ON_ERROR(errCode, "get integer lod failed.");
        gcmASSERT(intLodSymId != VIR_INVALID_ID);
    }

    /* Move the coord into a new reg. */
    tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
        VIR_SYM_VIRREG,
        tempRegId,
        VIR_Shader_GetTypeFromId(pShader, coordTypeId),
        VIR_STORAGE_UNKNOWN,
        &newCoordSymId);
    CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

    errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
        coordTypeId,
        pInst,
        gcvTRUE,
        &inst);
    ON_ERROR(errCode, "Add instruction.");

    /* Set Dest. */
    operand = VIR_Inst_GetDest(inst);
    VIR_Operand_SetTempRegister(operand,
        pFunc,
        newCoordSymId,
        coordTypeId);
    VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(coordTypeId));

    /* Set Source0. */
    operand = VIR_Inst_GetSource(inst, 0);
    VIR_Operand_Copy(operand, pCoordOpnd);

    /* If it is a projective texld, we need to div the coordinate with the q component of the coordinate. */
    if (isTexldProj)
    {
        VIR_Swizzle                 swizzle;
        VIR_Operand  *pTexldModifier = (VIR_Operand *)VIR_Inst_GetSource(pInst, 2);

        if (VIR_Operand_GetTexldGather_refz(pTexldModifier))
        {
            errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MOV,
                VIR_GetTypeComponentType(coordTypeId),
                pInst,
                gcvTRUE,
                &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                pFunc,
                newCoordSymId,
                VIR_GetTypeComponentType(coordTypeId));
            VIR_Operand_SetEnable(operand, (VIR_Enable)(VIR_ENABLE_X << (VIR_GetTypeComponents(newCoordTypeId) - 1)));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_Copy(operand, VIR_Operand_GetTexldGather_refz(pTexldModifier));

            /* Clean the drefZ. */
            VIR_Operand_SetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_GATHERREFZ, gcvNULL);
            if (VIR_Operand_GetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_GATHERCOMP) == gcvNULL)
            {
                VIR_Operand_ClrTexModifierFlag(pTexldModifier, VIR_TMFLAG_GATHER);
            }
        }

        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DIV,
            newCoordTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            newCoordTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(newCoordTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            newCoordTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(newCoordTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            VIR_GetTypeComponentType(coordTypeId));
        swizzle = VIR_Operand_GetSwizzle(pCoordOpnd);
        swizzle = VIR_Swizzle_GetChannel(swizzle, 3);
        VIR_Operand_SetSwizzle(operand, VIR_Enable_2_Swizzle_WShift((VIR_Enable)(VIR_ENABLE_X << swizzle)));
        VIR_Operand_SetTypeId(operand, VIR_GetTypeComponentType(coordTypeId));
    }

    /* Calculate the new coordinate value with offset. */
    if (addOffsetOnly)
    {
        /* ADD: newCoord, coord, offset */
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_ADD,
            offsetTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            offsetTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(offsetTypeId));

        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(operand, pCoordOpnd);

        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_Copy(operand, pOffsetOpnd);
    }
    else
    {
        VIR_TypeId tempTypeId;

        /* RSHIFT: size, levelBaseSize, LOD. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, offsetCompCount, 1);
        tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
            VIR_SYM_VIRREG,
            tempRegId,
            VIR_Shader_GetTypeFromId(pShader, tempTypeId),
            VIR_STORAGE_UNKNOWN,
            &sizeSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_RSHIFT,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            levelBaseSizeSymId,
            VIR_TYPE_INTEGER_X4);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            intLodSymId,
            VIR_TYPE_INT32);
        VIR_Operand_SetSwizzle(operand, VIR_SWIZZLE_XXXX);

        /* MAX: size, size, 1. */
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MAX,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetImmediateInt(operand, 1);

        /* I2F: size, size. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_CONVERT,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_INT32, offsetCompCount, 1);
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* I2F: newOffset, offset*/
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        tempRegId = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
            VIR_SYM_VIRREG,
            tempRegId,
            VIR_Shader_GetTypeFromId(pShader, tempTypeId),
            VIR_STORAGE_UNKNOWN,
            &newOffsetSymId);
        CHECK_ERROR(errCode, "VIR_Shader_AddSymbol failed.");

        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_CONVERT,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newOffsetSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_Copy(operand, pOffsetOpnd);

        /* If this offset is for textureGather, we need to clamp the offset. */
        if (isTexGather)
        {
            gctFLOAT minOffset = -8.0, maxOffset = 7.0;

            errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MAX,
                tempTypeId,
                pInst,
                gcvTRUE,
                &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                pFunc,
                newOffsetSymId,
                tempTypeId);
            VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_SetImmediateFloat(operand, minOffset);

            /* Set Source1. */
            operand = VIR_Inst_GetSource(inst, 1);
            VIR_Operand_SetTempRegister(operand,
                pFunc,
                newOffsetSymId,
                tempTypeId);
            VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

            errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_MIN,
                tempTypeId,
                pInst,
                gcvTRUE,
                &inst);
            ON_ERROR(errCode, "Add instruction.");

            /* Set Dest. */
            operand = VIR_Inst_GetDest(inst);
            VIR_Operand_SetTempRegister(operand,
                pFunc,
                newOffsetSymId,
                tempTypeId);
            VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

            /* Set Source0. */
            operand = VIR_Inst_GetSource(inst, 0);
            VIR_Operand_SetImmediateFloat(operand, maxOffset);

            /* Set Source1. */
            operand = VIR_Inst_GetSource(inst, 1);
            VIR_Operand_SetTempRegister(operand,
                pFunc,
                newOffsetSymId,
                tempTypeId);
            VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));
        }

        /* DIV: newOffset, newOffset, size. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_DIV,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newOffsetSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newOffsetSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            sizeSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* ADD: newCoord, newCoord, newOffset. */
        tempTypeId = VIR_TypeId_ComposeNonOpaqueType(VIR_TYPE_FLOAT32, offsetCompCount, 1);
        errCode = VIR_Function_AddInstructionBefore(pFunc, VIR_OP_ADD,
            tempTypeId,
            pInst,
            gcvTRUE,
            &inst);
        ON_ERROR(errCode, "Add instruction.");

        /* Set Dest. */
        operand = VIR_Inst_GetDest(inst);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            tempTypeId);
        VIR_Operand_SetEnable(operand, VIR_TypeId_Conv2Enable(tempTypeId));

        /* Set Source0. */
        operand = VIR_Inst_GetSource(inst, 0);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newCoordSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));

        /* Set Source1. */
        operand = VIR_Inst_GetSource(inst, 1);
        VIR_Operand_SetTempRegister(operand,
            pFunc,
            newOffsetSymId,
            tempTypeId);
        VIR_Operand_SetSwizzle(operand, VIR_TypeId_Conv2Swizzle(tempTypeId));
    }

    /* Update the coord operand. */
    VIR_Operand_SetSymbol(pCoordOpnd, pFunc, newCoordSymId);
    if (isTexldProj)
    {
        /* We don't need the proj after this process. */
        VIR_Operand_SetTypeId(pCoordOpnd, newCoordTypeId);
        VIR_Operand_SetSwizzle(pCoordOpnd, VIR_TypeId_Conv2Swizzle(newCoordTypeId));
        VIR_Inst_SetOpcode(pInst, (opCode == VIR_OP_TEXLDPROJ) ? VIR_OP_TEXLD : VIR_OP_TEXLDPCF);
    }
    else
    {
        VIR_Operand_SetSwizzle(pCoordOpnd, VIR_TypeId_Conv2Swizzle(coordTypeId));
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_CalculateCoordWithOffset(
    IN      VIR_Shader       *pShader,
    IN      VSC_HW_CONFIG    *HwCfg,
    IN      VIR_Function     *pFunc,
    IN      VIR_Instruction  *pInst
    )
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Operand                *pParamSrc = VIR_Inst_GetSource(pInst, 2);
    VIR_Operand                *pOffsetOpnd = gcvNULL;
    VIR_Operand  *pTexldModifier = (VIR_Operand *)pParamSrc;

    /* Skip non-param operand. */
    if (pParamSrc == gcvNULL || VIR_Operand_GetOpKind(pParamSrc) != VIR_OPND_TEXLDPARM)
    {
        return errCode;
    }

    /* Skip non-offset param. */
    if (!VIR_Operand_hasOffsetFlag(pTexldModifier))
    {
        return errCode;
    }

    /* Get the offset operand. */
    pOffsetOpnd = VIR_Operand_GetTexldOffset(pTexldModifier);

    /* Check if we need to calculate the offset. */
    if (_NeedToCalculateOffset(pShader, pFunc, pOffsetOpnd))
    {
        errCode = _CalculateOffsetCoord(pShader,
            HwCfg,
            pFunc,
            pInst);
        ON_ERROR(errCode, "Calculate offset coord");
    }

    /* Clean up the offset param. */
    VIR_Operand_ClrTexModifierFlag(pTexldModifier, VIR_TMFLAG_OFFSET);
    VIR_Operand_SetTexldModifier(pTexldModifier, VIR_TEXLDMODIFIER_OFFSET, gcvNULL);

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_FixTexldOffset)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
}

DEF_SH_NECESSITY_CHECK(vscVIR_FixTexldOffset)
{
    VSC_HW_CONFIG              *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    /* Calcuate the new coordinate with offset, if hardware doesn't support it directly. */
    if (pHwCfg->hwFeatureFlags.supportTexldCoordOffset)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_FixTexldOffset(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VIR_Shader                 *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_HW_CONFIG              *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode           *func_node;

    /* Process all instructions. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function           *func = func_node->function;
        VIR_InstIterator        inst_iter;
        VIR_Instruction        *inst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(func));
        for (inst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
            inst != gcvNULL; inst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            if (VIR_OPCODE_isTexLd(VIR_Inst_GetOpcode(inst)))
            {
                _CalculateCoordWithOffset(pShader, pHwCfg, func, inst);
            }
        }
    }

    return errCode;
}

static VSC_ErrCode _CalculateIndexForPrivateMemory(
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Shader          *pShader,
    gctBOOL             bNeedOOBCheck,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Instruction     *pModInst = gcvNULL;
    VIR_Operand         *dstOpnd = gcvNULL;
    VIR_Operand         *srcOpnd = gcvNULL;
    VIR_OperandInfo     srcOpndInfo;
    VIR_NameId          nameId;
    VIR_SymId           workThreadCountSymId = VIR_INVALID_ID;
    VIR_Symbol          *workThreadCountSym = gcvNULL;
    VIR_Symbol          *privateMemSym = gcvNULL;
    VIR_VirRegId        newVirRegIdx = VIR_INVALID_ID;
    VIR_SymId           newVirRegSymId = VIR_INVALID_ID;
    VIR_Symbol*         pThreadIdMemAddrSym = gcvNULL;
    VIR_Uniform*        pThreadIdMemAddrUniform = gcvNULL;
    VIR_Instruction*    pAtomAddInst = gcvNULL;
    VIR_Operand*        pNewOpnd = gcvNULL;

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
                                       _sldWorkThreadCountName,
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

        VIR_Shader_SetFlag(pShader, VIR_SHFLAG_USE_PRIVATE_MEM);

        newVirRegIdx = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_VIRREG,
                                       newVirRegIdx,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                       VIR_STORAGE_UNKNOWN,
                                       &newVirRegSymId);
        ON_ERROR(errCode, "Add symbol");

        if (opCode == VIR_OP_ADD)
        {
            /*
            ** 004: LSHIFT             uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            ** -->
            ** 004: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 005: LSHIFT             uint temp(9).x{r0.<3}, uint temp(10).x{r0.<0}, int 8
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            **
            ** Or
            **
            ** 004: MUL                uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #private_address.x
            ** -->
            ** 004: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 005: MUL                uint temp(9).x{r0.<3}, uint temp(10).x{r0.<0}, uint 300
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
                                                    &pModInst);
            VIR_Inst_SetOpcode(pModInst, VIR_OP_IMOD);

            VIR_Inst_SetInstType(pModInst, VIR_TYPE_UINT16);

            dstOpnd = VIR_Inst_GetDest(pModInst);
            VIR_Operand_SetSymbol(dstOpnd, pFunc, newVirRegSymId);
            VIR_Operand_SetEnable(dstOpnd, VIR_ENABLE_X);

            srcOpnd = VIR_Inst_GetSource(pModInst, VIR_Operand_Src0);
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(pModInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workThreadCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Update the DU for IMOD. */
            VIR_Operand_GetOperandInfo(pModInst, VIR_Inst_GetSource(pModInst, VIR_Operand_Src0), &srcOpndInfo);
            vscVIR_AddNewDef(pDuInfo,
                             pModInst,
                             newVirRegIdx,
                             1,
                             VIR_Operand_GetEnable(dstOpnd),
                             VIR_HALF_CHANNEL_MASK_FULL,
                             gcvNULL,
                             gcvNULL);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    pModInst,
                                    VIR_Inst_GetSource(pModInst, VIR_Operand_Src0),
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(pModInst, VIR_Operand_Src0))),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);

            /* Change the SRC0 of LSHIFT/MUL. */
            srcOpnd = VIR_Inst_GetSource(shiftOrMulInst, VIR_Operand_Src0);
            VIR_Operand_GetOperandInfo(shiftOrMulInst, srcOpnd, &srcOpndInfo);

            /* Update the DU for LSHIFT/MUL. */
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               shiftOrMulInst,
                               srcOpnd,
                               gcvFALSE,
                               srcOpndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
            VIR_Operand_GetOperandInfo(shiftOrMulInst, srcOpnd, &srcOpndInfo);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    shiftOrMulInst,
                                    srcOpnd,
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }
        else
        {
            /*
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(8).x{r0.<3}, int 340, uint #private_address.x
            ** -->
            ** 003: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #WorkThreadCount.x
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(10).x{r0.<0}, int 340, uint #private_address.x
            */
            /* Insert a MOD before MAD. */
            VIR_Function_AddInstructionBefore(pFunc,
                                              VIR_OP_IMOD,
                                              VIR_TYPE_UINT16,
                                              pInst,
                                              gcvTRUE,
                                              &pModInst);
            dstOpnd = VIR_Inst_GetDest(pModInst);
            VIR_Operand_SetSymbol(dstOpnd, pFunc, newVirRegSymId);
            VIR_Operand_SetEnable(dstOpnd, VIR_ENABLE_X);

            srcOpnd = VIR_Inst_GetSource(pModInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(pModInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workThreadCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Update the DU for IMOD. */
            VIR_Operand_GetOperandInfo(pModInst, VIR_Inst_GetSource(pModInst, VIR_Operand_Src0), &srcOpndInfo);
            vscVIR_AddNewDef(pDuInfo,
                             pModInst,
                             newVirRegIdx,
                             1,
                             VIR_Operand_GetEnable(dstOpnd),
                             VIR_HALF_CHANNEL_MASK_FULL,
                             gcvNULL,
                             gcvNULL);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    pModInst,
                                    VIR_Inst_GetSource(pModInst, VIR_Operand_Src0),
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(pModInst, VIR_Operand_Src0))),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);

            /* Change the SRC0 of MAD. */
            srcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
            VIR_Operand_GetOperandInfo(pInst, srcOpnd, &srcOpndInfo);

            /* Update the DU for MAD. */
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               pInst,
                               srcOpnd,
                               gcvFALSE,
                               srcOpndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
            VIR_Operand_GetOperandInfo(pInst, srcOpnd, &srcOpndInfo);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    pInst,
                                    srcOpnd,
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }

        /* Start to generate instructions to calculate the offset for the private memory. */
        /* Find uniform #threadIdMemAddr, if not found, create one. */
        pThreadIdMemAddrSym = VIR_Shader_FindSymbolByName(pShader, VIR_SYM_UNIFORM, _sldThreadIdMemoryAddressName);
        if (pThreadIdMemAddrSym == gcvNULL)
        {
            errCode = VIR_Shader_AddNamedUniform(pShader,
                                                 _sldThreadIdMemoryAddressName,
                                                 VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                                 &pThreadIdMemAddrSym);

            VIR_Symbol_SetUniformKind(pThreadIdMemAddrSym, VIR_UNIFORM_THREAD_ID_MEM_ADDR);
            VIR_Symbol_SetFlag(pThreadIdMemAddrSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
            VIR_Symbol_SetFlag(pThreadIdMemAddrSym, VIR_SYMFLAG_COMPILER_GEN);
            VIR_Symbol_SetLocation(pThreadIdMemAddrSym, -1);
            VIR_Symbol_SetPrecision(pThreadIdMemAddrSym, VIR_PRECISION_HIGH);

            pThreadIdMemAddrUniform = VIR_Symbol_GetUniform(pThreadIdMemAddrSym);
            pThreadIdMemAddrUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)) - 1;
        }

        /* Create a ATOMIC_ADD to get the thread ID. */
        errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                    VIR_OP_ATOMADD_S,
                                                    VIR_TYPE_UINT32,
                                                    pModInst,
                                                    gcvTRUE,
                                                    &pAtomAddInst);
        ON_ERROR(errCode, "Insert a ATOMIC_ADD instruction.");

        /* Set DEST. */
        pNewOpnd = VIR_Inst_GetDest(pAtomAddInst);
        VIR_Operand_SetSymbol(pNewOpnd, pFunc, newVirRegSymId);
        VIR_Operand_SetTypeId(pNewOpnd, VIR_TYPE_UINT32);
        VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_X);

        /* src0 - base */
        pNewOpnd = VIR_Inst_GetSource(pAtomAddInst, 0);
        VIR_Operand_SetSymbol(pNewOpnd, pFunc, VIR_Symbol_GetIndex(pThreadIdMemAddrSym));
        VIR_Operand_SetSwizzle(pNewOpnd, bNeedOOBCheck ? VIR_SWIZZLE_XYZZ : VIR_SWIZZLE_XXXX);

        /* src1 - 0 */
        pNewOpnd = VIR_Inst_GetSource(pAtomAddInst, 1);
        VIR_Operand_SetImmediateUint(pNewOpnd, 0);

        /* src2 - 1 */
        pNewOpnd = VIR_Inst_GetSource(pAtomAddInst, 2);
        VIR_Operand_SetImmediateUint(pNewOpnd, 1);

        /* Update the DU. */
        vscVIR_AddNewDef(pDuInfo,
                         pAtomAddInst,
                         newVirRegIdx,
                         1,
                         VIR_ENABLE_X,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Change the SRC0 of MOD instruction. */
        pNewOpnd = VIR_Inst_GetSource(pModInst, 0);
        VIR_Operand_GetOperandInfo(pModInst, pNewOpnd, &srcOpndInfo);

        /* Delete the usage first. */
        if (srcOpndInfo.isVreg)
        {
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               pModInst,
                               pNewOpnd,
                               gcvFALSE,
                               srcOpndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pNewOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);
        }

        VIR_Operand_SetSymbol(pNewOpnd, pFunc, newVirRegSymId);
        VIR_Operand_SetTypeId(pNewOpnd, VIR_TYPE_UINT16);
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XXXX);

        /* Update the DU for MAD. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                pAtomAddInst,
                                pModInst,
                                pNewOpnd,
                                gcvFALSE,
                                newVirRegIdx,
                                1,
                                VIR_ENABLE_X,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);
    }

OnError:
    return errCode;
}

static VSC_ErrCode _CalculateIndexForLocalMemory(
    VIR_DEF_USAGE_INFO  *pDuInfo,
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
    VIR_OperandInfo     srcOpndInfo;
    VIR_NameId          nameId;
    VIR_SymId           workGroupCountSymId = VIR_INVALID_ID;
    VIR_Symbol          *workGroupCountSym = gcvNULL;
    VIR_Symbol          *localMemSym = gcvNULL;
    VIR_VirRegId        newVirRegIdx = VIR_INVALID_ID;
    VIR_SymId           newVirRegSymId = VIR_INVALID_ID;

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
        strcmp(VIR_Shader_GetSymNameString(pShader, localMemSym), _sldLocalStorageAddressName) == 0)
    {
        /* Add a new uniform to save the workGroupCount. */
        errCode = VIR_Shader_AddString(pShader,
                                       _sldWorkGroupCountName,
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

        newVirRegIdx = VIR_Shader_NewVirRegId(pShader, 1);
        errCode = VIR_Shader_AddSymbol(pShader,
                                        VIR_SYM_VIRREG,
                                        newVirRegIdx,
                                        VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT16),
                                        VIR_STORAGE_UNKNOWN,
                                        &newVirRegSymId);
        ON_ERROR(errCode, "Add symbol");

        if (opCode == VIR_OP_ADD)
        {
            /*
            ** 004: LSHIFT             uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            ** -->
            ** 004: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #workGroupCount.x
            ** 005: LSHIFT             uint temp(9).x{r0.<3}, uint temp(10).x{r0.<0}, int 8
            ** 006: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            **
            ** Or
            **
            ** 004: MUL                uint temp(9).x{r0.<3}, uint temp(8).x{r0.<3}, int 8
            ** 005: ADD                uint temp(1).x{r0.<3}, uint temp(9).x{r0.<3}, uint #local_address.x
            ** -->
            ** 004: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #workGroupCount.x
            ** 005: MUL                uint temp(9).x{r0.<3}, uint temp(10).x{r0.<0}, uint 300
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

            VIR_Inst_SetInstType(newInst, VIR_TYPE_UINT16);

            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetSymbol(dstOpnd, pFunc, newVirRegSymId);
            VIR_Operand_SetEnable(dstOpnd, VIR_ENABLE_X);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workGroupCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Update the DU for IMOD. */
            VIR_Operand_GetOperandInfo(newInst, VIR_Inst_GetSource(newInst, VIR_Operand_Src0), &srcOpndInfo);
            vscVIR_AddNewDef(pDuInfo,
                             newInst,
                             newVirRegIdx,
                             1,
                             VIR_Operand_GetEnable(dstOpnd),
                             VIR_HALF_CHANNEL_MASK_FULL,
                             gcvNULL,
                             gcvNULL);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    newInst,
                                    VIR_Inst_GetSource(newInst, VIR_Operand_Src0),
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(newInst, VIR_Operand_Src0))),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);

            /* Change the SRC0 of LSHIFT/MUL. */
            srcOpnd = VIR_Inst_GetSource(shiftOrMulInst, VIR_Operand_Src0);
            VIR_Operand_GetOperandInfo(shiftOrMulInst, srcOpnd, &srcOpndInfo);

            /* Update the DU for LSHIFT/MUL. */
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               shiftOrMulInst,
                               srcOpnd,
                               gcvFALSE,
                               srcOpndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
            VIR_Operand_GetOperandInfo(shiftOrMulInst, srcOpnd, &srcOpndInfo);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    shiftOrMulInst,
                                    srcOpnd,
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }
        else
        {
            /*
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(8).x{r0.<3}, int 340, uint #local_address.x
            ** -->
            ** 003: IMOD               ushort temp(10).x{r0.<0}, ushort temp(8).x{r0.<3}, ushort #WorkGroupCount.x
            ** 004: IMADLO0            uint temp(1).x{r0.<3}, uint temp(10).x{r0.<0}, int 340, uint #local_address.x
            */
            /* Insert a MOD before MAD. */
            VIR_Function_AddInstructionBefore(pFunc,
                                              VIR_OP_IMOD,
                                              VIR_TYPE_UINT16,
                                              pInst,
                                              gcvTRUE,
                                              &newInst);
            dstOpnd = VIR_Inst_GetDest(newInst);
            VIR_Operand_SetSymbol(dstOpnd, pFunc, newVirRegSymId);
            VIR_Operand_SetEnable(dstOpnd, VIR_ENABLE_X);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src0);
            VIR_Operand_Copy(srcOpnd, VIR_Inst_GetSource(pInst, VIR_Operand_Src0));
            VIR_Operand_SetTypeId(srcOpnd, VIR_TYPE_UINT16);

            srcOpnd = VIR_Inst_GetSource(newInst, VIR_Operand_Src1);
            VIR_Operand_SetSymbol(srcOpnd, pFunc, workGroupCountSymId);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_SWIZZLE_XXXX);

            /* Update the DU for IMOD. */
            VIR_Operand_GetOperandInfo(newInst, VIR_Inst_GetSource(newInst, VIR_Operand_Src0), &srcOpndInfo);
            vscVIR_AddNewDef(pDuInfo,
                             newInst,
                             newVirRegIdx,
                             1,
                             VIR_Operand_GetEnable(dstOpnd),
                             VIR_HALF_CHANNEL_MASK_FULL,
                             gcvNULL,
                             gcvNULL);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    newInst,
                                    VIR_Inst_GetSource(newInst, VIR_Operand_Src0),
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(newInst, VIR_Operand_Src0))),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);

            /* Change the SRC0 of MAD. */
            srcOpnd = VIR_Inst_GetSource(pInst, VIR_Operand_Src0);
            VIR_Operand_GetOperandInfo(pInst, srcOpnd, &srcOpndInfo);

            /* Update the DU for MAD. */
            vscVIR_DeleteUsage(pDuInfo,
                               VIR_ANY_DEF_INST,
                               pInst,
                               srcOpnd,
                               gcvFALSE,
                               srcOpndInfo.u1.virRegInfo.virReg,
                               1,
                               VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                               VIR_HALF_CHANNEL_MASK_FULL,
                               gcvNULL);

            VIR_Operand_Copy(srcOpnd, dstOpnd);
            VIR_Operand_SetLvalue(srcOpnd, gcvFALSE);
            VIR_Operand_SetSwizzle(srcOpnd, VIR_Enable_2_Swizzle_WShift(VIR_Operand_GetEnable(dstOpnd)));
            VIR_Operand_GetOperandInfo(pInst, srcOpnd, &srcOpndInfo);

            vscVIR_AddNewUsageToDef(pDuInfo,
                                    VIR_ANY_DEF_INST,
                                    pInst,
                                    srcOpnd,
                                    gcvFALSE,
                                    srcOpndInfo.u1.virRegInfo.virReg,
                                    1,
                                    VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(srcOpnd)),
                                    VIR_HALF_CHANNEL_MASK_FULL,
                                    gcvNULL);
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode _ReplaceLocalMemoryAddress(
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Symbol          *localMemAddrSym = gcvNULL;
    VIR_Operand         *pOpnd = gcvNULL;
    VIR_Symbol          *localMemSym = gcvNULL;
    gctBOOL             isOCL = VIR_Shader_IsCL(pShader);

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
        ((isOCL && strcmp(VIR_Shader_GetSymNameString(pShader, localMemSym), _sldLocalStorageAddressName) == 0)
         ||
         (!isOCL && strcmp(VIR_Shader_GetSymNameString(pShader, localMemSym), _sldSharedVariableStorageBlockName) == 0)))
    {
        localMemAddrSym = VIR_Operand_GetSymbol(VIR_Inst_GetDest(pInst));
        if (localMemAddrSym && VIR_Symbol_isVreg(localMemAddrSym))
        {
            localMemAddrSym = VIR_Symbol_GetVregVariable(localMemAddrSym);
        }
    }

    if (localMemAddrSym &&
        strcmp(VIR_Shader_GetSymNameString(pShader, localMemAddrSym), _sldLocalMemoryAddressName) == 0)
    {
        VIR_Symbol          *pLocIdSym = gcvNULL;
        VIR_SymId           outRegId, outRegSymId;
        VIR_AttributeIdList *pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
        gctUINT             attrCount = VIR_IdList_Count(pAttrIdLsts);
        gctUINT             attrIdx;
        gctUINT             nextAttrLlSlot = 0;
        gctUINT             i;
        VIR_NATIVE_DEF_FLAGS nativeDefFlags;

        pLocIdSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_LOCAL_INVOCATION_ID);

        if (pLocIdSym == gcvNULL)
        {
            for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
            {
                VIR_SymId       attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
                VIR_Symbol      *pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);
                gctUINT         thisOutputRegCount;

                if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
                {
                    gcmASSERT(VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED);

                    thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);
                    if (nextAttrLlSlot < (VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount))
                    {
                        nextAttrLlSlot = VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount;
                    }
                }
            }

            pLocIdSym = VIR_Shader_AddBuiltinAttribute(pShader, VIR_TYPE_UINT_X4, gcvFALSE, VIR_NAME_LOCAL_INVOCATION_ID);
            outRegId = VIR_Shader_NewVirRegId(pShader, 1);
            VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VIRREG,
                                 outRegId,
                                 VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4),
                                 VIR_STORAGE_UNKNOWN,
                                 &outRegSymId);
            VIR_Symbol_SetVariableVregIndex(pLocIdSym, outRegId);
            VIR_Symbol_SetVregVariable(VIR_Shader_GetSymFromId(pShader, outRegSymId), pLocIdSym);
            VIR_Symbol_SetIndexRange(pLocIdSym, outRegId + 1);
            VIR_Symbol_SetIndexRange(VIR_Shader_GetSymFromId(pShader, outRegSymId), outRegId + 1);
            VIR_Symbol_SetFirstSlot(pLocIdSym, nextAttrLlSlot);
        }
        else
        {
            outRegId = VIR_Symbol_GetVregIndex(pLocIdSym);
        }

        /* Delete the usage first. */
        for (i = 0; i < VIR_Inst_GetSrcNum(pInst); i++)
        {
            VIR_Operand     *pOpnd;
            VIR_OperandInfo  opndInfo;

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

        /* Change this instruction to MOV. */
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetSrcNum(pInst, 1);

        pOpnd = VIR_Inst_GetSource(pInst, 0);
        VIR_Operand_SetSymbol(pOpnd, pFunc, VIR_Symbol_GetIndex(pLocIdSym));
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_WWWW);

        /* Add def. */
        memset(&nativeDefFlags, 0, sizeof(nativeDefFlags));
        nativeDefFlags.bIsInput = gcvTRUE;
        vscVIR_AddNewDef(pDuInfo,
                         VIR_INPUT_DEF_INST,
                         outRegId,
                         1,
                         VIR_ENABLE_W,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         &nativeDefFlags,
                         gcvNULL);

        /* Add usage. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                VIR_INPUT_DEF_INST,
                                pInst,
                                pOpnd,
                                gcvFALSE,
                                outRegId,
                                1,
                                VIR_ENABLE_W,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

        /* Set the flag. */
        VIR_Shader_SetUseHwManagedLS(pShader, gcvTRUE);

        /* Change the uniform to unused. */
        VIR_Symbol_ClrFlag(localMemSym, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        VIR_Symbol_SetFlag(localMemSym, VIR_SYMFLAG_INACTIVE);
    }

    return errCode;
}

static VSC_ErrCode _UpdateLocalMemoryCalc(
    VIR_DEF_USAGE_INFO  *pDuInfo,
    VSC_HW_CONFIG       *pHwCfg,
    VIR_Shader          *pShader,
    VIR_Function        *pFunc,
    VIR_Instruction     *pInst
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;

    /* If HW can suport managed_ls, it means we need to use a reserved register for #sh_localMemoryAddress, and remove all its defined. */
    if (pHwCfg->hwFeatureFlags.supportHWManagedLS   &&
        VIR_Shader_UseLocalMem(pShader)             &&
        !VIR_Shader_IsUseHwManagedLS(pShader))
    {
        errCode = _ReplaceLocalMemoryAddress(pDuInfo, pShader, pFunc, pInst);
        ON_ERROR(errCode, "Replace local memory address.");
    }
    else
    {
        /* Generate a uniform to save the workGroupCount, we need to do this for OCL only because the totalNumGroups of CS is fixed. */
        if (VIR_Shader_IsCL(pShader) &&
            !VIR_Shader_IsVulkan(pShader) &&
            VIR_Shader_GetShareMemorySize(pShader) > 0)
        {
            errCode = _CalculateIndexForLocalMemory(pDuInfo, pShader, pFunc, pInst);
            ON_ERROR(errCode, "Create concurrent workGroupCount.");
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_UpdateLocMemAndPrivMem(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    VSC_HW_CONFIG       *pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_DEF_USAGE_INFO  *pDuInfo = pPassWorker->pDuInfo;
    gctBOOL             bNeedOOBCheck = (pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK) != 0;

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
            /* Only support OCL now. */
            if (VIR_Shader_IsCL(pShader) &&
                !VIR_Shader_IsVulkan(pShader) &&
                VIR_Shader_GetPrivateMemorySize(pShader) > 0)
            {
                errCode = _CalculateIndexForPrivateMemory(pDuInfo, pShader, bNeedOOBCheck, func, inst);
                ON_ERROR(errCode, "Create concurrent workThreadCount.");
            }

            /* Convert local memory if local memory is used. */
            if ((VIR_Shader_IsCL(pShader) || VIR_Shader_IsGlCompute(pShader))
                &&
                VIR_Shader_GetShareMemorySize(pShader) > 0)
            {
                errCode = _UpdateLocalMemoryCalc(pDuInfo, pHwCfg, pShader, func, inst);
                ON_ERROR(errCode, "Update local memory calculation.");
            }
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_UpdateWorkGroupIdForMultiGPU(
    VSC_SH_PASS_WORKER* pPassWorker,
    gctBOOL*            bChanged
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO*     pDuInfo = pPassWorker->pDuInfo;
    VIR_GENERAL_DU_ITERATOR duIter;
    VIR_USAGE*              pUsage = gcvNULL;
    VIR_Instruction*        pUsageInst = gcvNULL;
    VIR_Operand*            pUsageOpnd = gcvNULL;
    VIR_AttributeIdList*    pAttrIdLists = VIR_Shader_GetAttributes(pShader);
    VIR_UniformIdList*      pUniformIdList = VIR_Shader_GetUniforms(pShader);
    VIR_SymId               symId = VIR_INVALID_ID;
    VIR_SymId               workGroupIdOffsetSymId = VIR_INVALID_ID, globalIdOffsetSymId = VIR_INVALID_ID;
    VIR_Symbol*             pSym = gcvNULL;
    VIR_Symbol*             pWorkGroupIdSym = gcvNULL;
    VIR_Symbol*             pWorkGroupIdOffsetSym = gcvNULL;
    VIR_Symbol*             pGlobalIdSym = gcvNULL;
    VIR_Symbol*             pGlobalIdOffsetSym = gcvNULL;
    VIR_Function*           pMainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_NameId              nameId = VIR_INVALID_ID;
    VIR_Instruction*        pNewAddInst = gcvNULL;
    VIR_Operand*            pNewOpnd = gcvNULL;
    gctUINT                 i, defIdx = 0;
    VIR_DEF*                pDef;
    gctBOOL                 bUseLocalMemory = (VIR_Shader_GetShareMemorySize(pShader) > 0);
    gctBOOL                 bUpdateGlobalId = gcvFALSE;

    /* Find the workGroupId and globalInvocationId. */
    for (i = 0; i < VIR_IdList_Count(pAttrIdLists); i++)
    {
        symId = VIR_IdList_GetId(pAttrIdLists, i);
        pSym = VIR_Shader_GetSymFromId(pShader, symId);

        if (isSymUnused(pSym))
        {
            continue;
        }

        if (VIR_Symbol_GetName(pSym) == VIR_NAME_WORK_GROUP_ID)
        {
            pWorkGroupIdSym = pSym;
        }
        else if (bUpdateGlobalId && VIR_Symbol_GetName(pSym) == VIR_NAME_GLOBAL_INVOCATION_ID)
        {
            pGlobalIdSym = pSym;
        }

        if (pWorkGroupIdSym != gcvNULL && pGlobalIdSym != gcvNULL)
        {
            break;
        }
    }

    if (pWorkGroupIdSym == gcvNULL && pGlobalIdSym == gcvNULL)
    {
        return errCode;
    }

    for (i = 0; i < VIR_IdList_Count(pUniformIdList); i++)
    {
        symId = VIR_IdList_GetId(pUniformIdList, i);
        pSym = VIR_Shader_GetSymFromId(pShader, symId);

        if (pSym && VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_WORK_GROUP_ID_OFFSET)
        {
            pWorkGroupIdOffsetSym = pSym;
            workGroupIdOffsetSymId = VIR_Symbol_GetIndex(pWorkGroupIdOffsetSym);
        }
        else if (pSym && VIR_Symbol_GetUniformKind(pSym) == VIR_UNIFORM_GLOBAL_INVOCATION_ID_OFFSET)
        {
            pGlobalIdOffsetSym = pSym;
            globalIdOffsetSymId = VIR_Symbol_GetIndex(pGlobalIdOffsetSym);
        }

        if (pWorkGroupIdOffsetSym != gcvNULL && pGlobalIdOffsetSym != gcvNULL)
        {
            break;
        }
    }

    if (!((pWorkGroupIdSym != gcvNULL && pWorkGroupIdOffsetSym == gcvNULL)
         ||
         (pGlobalIdSym != gcvNULL && pGlobalIdOffsetSym == gcvNULL)))
    {
        return errCode;
    }

    /* Create workGroupIdOffset. */
    if (pWorkGroupIdSym != gcvNULL && pWorkGroupIdOffsetSym == gcvNULL)
    {
        errCode = VIR_Shader_AddString(pShader,
                                       _sldWorkGroupIdOffsetName,
                                       &nameId);

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                       VIR_STORAGE_UNKNOWN,
                                       &workGroupIdOffsetSymId);
        ON_ERROR(errCode, "Add workGroupIdOffset uniform. ");

        pWorkGroupIdOffsetSym = VIR_Shader_GetSymFromId(pShader, workGroupIdOffsetSymId);
        VIR_Symbol_SetFlag(pWorkGroupIdOffsetSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(pWorkGroupIdOffsetSym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetUniformKind(pWorkGroupIdOffsetSym, VIR_UNIFORM_WORK_GROUP_ID_OFFSET);
        VIR_Symbol_SetAddrSpace(pWorkGroupIdOffsetSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(pWorkGroupIdOffsetSym, VIR_TYQUAL_CONST);

        /* Insert a ADD instruction: workGroupId = workGroupId + workGroupOffset. */
        /*
        ** If this shader don't use LocalMemory, insert this ADD in the beginning of the shader,
        ** if no, insert this ADD after the local memory address calculation.
        */
        if (bUseLocalMemory)
        {
            VIR_InstIterator    inst_iter;
            VIR_Instruction*    pInst;
            VIR_Instruction*    pInsertPosInst = gcvNULL;
            VIR_OperandInfo     opndInfo;

            VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(VIR_Shader_GetMainFunction(pShader)));
            for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
                 pInst != gcvNULL;
                 pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
            {
                if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pInst)))
                {
                    VIR_Operand_GetOperandInfo(pInst, VIR_Inst_GetDest(pInst), &opndInfo);

                    if (opndInfo.isVreg && opndInfo.u1.virRegInfo.virReg == VIR_OCL_LocalMemoryAddressRegIndex)
                    {
                        pInsertPosInst = pInst;
                        break;
                    }
                }
            }

            if (pInsertPosInst)
            {
                errCode = VIR_Function_AddInstructionAfter(pMainFunc,
                                                           VIR_OP_ADD,
                                                           VIR_TYPE_UINT_X3,
                                                           pInsertPosInst,
                                                           gcvTRUE,
                                                           &pNewAddInst);
                ON_ERROR(errCode, "Insert a ADD instruction.");
            }
            else
            {
                gcmASSERT(gcvFALSE);

                errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                          VIR_OP_ADD,
                                                          VIR_TYPE_UINT_X3,
                                                          &pNewAddInst);
                ON_ERROR(errCode, "Insert a ADD instruction.");
            }
        }
        else
        {
            errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                      VIR_OP_ADD,
                                                      VIR_TYPE_UINT_X3,
                                                      &pNewAddInst);
            ON_ERROR(errCode, "Insert a ADD instruction.");
        }

        /* Set DEST. */
        pNewOpnd = VIR_Inst_GetDest(pNewAddInst);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pWorkGroupIdSym));
        VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_XYZ);

        /* Set SRC0. */
        pNewOpnd = VIR_Inst_GetSource(pNewAddInst, 0);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pWorkGroupIdSym));
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZZ);

        /* Set SRC1. */
        pNewOpnd = VIR_Inst_GetSource(pNewAddInst, 1);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, workGroupIdOffsetSymId);
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZZ);

        /* Add a new def. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewAddInst,
                         VIR_Symbol_GetVregIndex(pWorkGroupIdSym),
                         1,
                         VIR_ENABLE_XYZ,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Add source0 usage. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                VIR_INPUT_DEF_INST,
                                pNewAddInst,
                                VIR_Inst_GetSource(pNewAddInst, VIR_Operand_Src0),
                                gcvFALSE,
                                VIR_Symbol_GetVregIndex(pWorkGroupIdSym),
                                1,
                                VIR_ENABLE_XYZ,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

        /* Add a new def for all its usages(channel XYZ). */
        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, VIR_Symbol_GetVregIndex(pWorkGroupIdSym));
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
            gcmASSERT(pDef);

            if (pDef->defKey.channel < VIR_CHANNEL_W && pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
            {
                duIter.pDuInfo = pDuInfo;
                duIter.bSameBBOnly = gcvFALSE;
                duIter.defKey.pDefInst = pDef->defKey.pDefInst;
                duIter.defKey.regNo = VIR_Symbol_GetVregIndex(pWorkGroupIdSym);
                duIter.defKey.channel = pDef->defKey.channel;
                VSC_DU_ITERATOR_INIT(&(duIter.duIter), &pDef->duChain);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter);
                     pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    pUsageInst = pUsage->usageKey.pUsageInst;
                    pUsageOpnd = pUsage->usageKey.pOperand;

                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pNewAddInst,
                                            pUsageInst,
                                            pUsageOpnd,
                                            gcvFALSE,
                                            VIR_Symbol_GetVregIndex(pWorkGroupIdSym),
                                            1,
                                            (VIR_Enable)(VIR_ENABLE_X << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    /* Create globalInvocationIdOffset. */
    if (pGlobalIdSym != gcvNULL && pGlobalIdOffsetSym == gcvNULL)
    {
        errCode = VIR_Shader_AddString(pShader,
                                       _sldGlobalIdOffsetName,
                                       &nameId);

        errCode = VIR_Shader_AddSymbol(pShader,
                                       VIR_SYM_UNIFORM,
                                       nameId,
                                       VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                       VIR_STORAGE_UNKNOWN,
                                       &globalIdOffsetSymId);
        ON_ERROR(errCode, "Add globalIdOffset uniform. ");

        pGlobalIdOffsetSym = VIR_Shader_GetSymFromId(pShader, globalIdOffsetSymId);
        VIR_Symbol_SetFlag(pGlobalIdOffsetSym, VIR_SYMFLAG_COMPILER_GEN);
        VIR_Symbol_SetPrecision(pGlobalIdOffsetSym, VIR_PRECISION_HIGH);
        VIR_Symbol_SetUniformKind(pGlobalIdOffsetSym, VIR_UNIFORM_GLOBAL_INVOCATION_ID_OFFSET);
        VIR_Symbol_SetAddrSpace(pGlobalIdOffsetSym, VIR_AS_CONSTANT);
        VIR_Symbol_SetTyQualifier(pGlobalIdOffsetSym, VIR_TYQUAL_CONST);

        /* Insert a ADD instruction: globalInvocationId = globalInvocationId + globalInvocationIdOffset. */
        errCode = VIR_Function_PrependInstruction(pMainFunc,
                                                  VIR_OP_ADD,
                                                  VIR_TYPE_UINT_X3,
                                                  &pNewAddInst);
        ON_ERROR(errCode, "Insert a ADD instruction.");

        /* Set DEST. */
        pNewOpnd = VIR_Inst_GetDest(pNewAddInst);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pGlobalIdSym));
        VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_XYZ);

        /* Set SRC0. */
        pNewOpnd = VIR_Inst_GetSource(pNewAddInst, 0);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pGlobalIdSym));
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZZ);

        /* Set SRC1. */
        pNewOpnd = VIR_Inst_GetSource(pNewAddInst, 1);
        VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, globalIdOffsetSymId);
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZZ);

        /* Add a new def. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewAddInst,
                         VIR_Symbol_GetVregIndex(pGlobalIdSym),
                         1,
                         VIR_ENABLE_XYZ,
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Add source0 usage. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                VIR_INPUT_DEF_INST,
                                pNewAddInst,
                                VIR_Inst_GetSource(pNewAddInst, VIR_Operand_Src0),
                                gcvFALSE,
                                VIR_Symbol_GetVregIndex(pGlobalIdSym),
                                1,
                                VIR_ENABLE_XYZ,
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

        /* Add a new def for all its usages(channel XYZ). */
        defIdx = vscVIR_FindFirstDefIndex(pDuInfo, VIR_Symbol_GetVregIndex(pGlobalIdSym));
        while (VIR_INVALID_DEF_INDEX != defIdx)
        {
            pDef = GET_DEF_BY_IDX(&pDuInfo->defTable, defIdx);
            gcmASSERT(pDef);

            if (pDef->defKey.channel < VIR_CHANNEL_W && pDef->defKey.pDefInst == VIR_INPUT_DEF_INST)
            {
                duIter.pDuInfo = pDuInfo;
                duIter.bSameBBOnly = gcvFALSE;
                duIter.defKey.pDefInst = pDef->defKey.pDefInst;
                duIter.defKey.regNo = VIR_Symbol_GetVregIndex(pGlobalIdSym);
                duIter.defKey.channel = pDef->defKey.channel;
                VSC_DU_ITERATOR_INIT(&(duIter.duIter), &pDef->duChain);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter);
                     pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    pUsageInst = pUsage->usageKey.pUsageInst;
                    pUsageOpnd = pUsage->usageKey.pOperand;

                    vscVIR_AddNewUsageToDef(pDuInfo,
                                            pNewAddInst,
                                            pUsageInst,
                                            pUsageOpnd,
                                            gcvFALSE,
                                            VIR_Symbol_GetVregIndex(pGlobalIdSym),
                                            1,
                                            (VIR_Enable)(VIR_ENABLE_X << pDef->defKey.channel),
                                            VIR_HALF_CHANNEL_MASK_FULL,
                                            gcvNULL);
                }
            }

            /* Get next def with same regNo */
            defIdx = pDef->nextDefIdxOfSameRegNo;
        }
    }

    /* Set the flag. */
    VIR_Shader_SetFlagExt1(pShader, VIR_SHFLAG_EXT1_ENABLE_MULTI_GPU);

    if (bChanged)
    {
        *bChanged = gcvTRUE;
    }

OnError:
    return errCode;
}

typedef VSC_ErrCode
(*UPDATE_SUBGROUP_VARIABLE_FUNC)(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst,
    VIR_Operand*            pSrcOpnd,
    VIR_Symbol*             pSrcSym,
    void*                   pPrivateData
    );

typedef struct _UPDATE_SUBGROUP_VARIABLE_INFO
{
    VIR_Symbol*                     pVariableSym;
    UPDATE_SUBGROUP_VARIABLE_FUNC   updateFunc;
    void*                           pPrivateData;
} UPDATE_SUBGROUP_VARIABLE_INFO;

static VSC_ErrCode
_UpdateSubGroupSize(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst,
    VIR_Operand*            pSrcOpnd,
    VIR_Symbol*             pSrcSym,
    void*                   pPrivateData
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    /* Delete usage. */
    vscVIR_DeleteUsage(pDuInfo,
                       VIR_INPUT_DEF_INST,
                       pInst,
                       pSrcOpnd,
                       gcvFALSE,
                       VIR_Symbol_GetVariableVregIndex(pSrcSym),
                       1,
                       VIR_ENABLE_X,
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

    VIR_Operand_SetImmediateUint(pSrcOpnd, 1);

    return errCode;
}

static VSC_ErrCode
_UpdateSubGroupNum(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst,
    VIR_Operand*            pSrcOpnd,
    VIR_Symbol*             pSrcSym,
    void*                   pPrivateData
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    /* Delete usage. */
    vscVIR_DeleteUsage(pDuInfo,
                       VIR_INPUT_DEF_INST,
                       pInst,
                       pSrcOpnd,
                       gcvFALSE,
                       VIR_Symbol_GetVariableVregIndex(pSrcSym),
                       1,
                       VIR_ENABLE_X,
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

    VIR_Operand_SetImmediateUint(pSrcOpnd, VIR_Shader_GetWorkGroupSize(pShader));

    return errCode;
}

static VSC_ErrCode
_UpdateSubGroupId(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst,
    VIR_Operand*            pSrcOpnd,
    VIR_Symbol*             pSrcSym,
    void*                   pPrivateData
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Symbol*             pLocalInvocationId = (VIR_Symbol*)pPrivateData;

    /* Delete usage. */
    vscVIR_DeleteUsage(pDuInfo,
                       VIR_INPUT_DEF_INST,
                       pInst,
                       pSrcOpnd,
                       gcvFALSE,
                       VIR_Symbol_GetVariableVregIndex(pSrcSym),
                       1,
                       VIR_ENABLE_X,
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

    VIR_Operand_SetSymbol(pSrcOpnd, VIR_Inst_GetFunction(pInst), VIR_Symbol_GetIndex(pLocalInvocationId));

    /* Add new usage. */
    vscVIR_AddNewUsageToDef(pDuInfo,
                            VIR_INPUT_DEF_INST,
                            pInst,
                            pSrcOpnd,
                            gcvFALSE,
                            VIR_Symbol_GetVariableVregIndex(pLocalInvocationId),
                            1,
                            VIR_ENABLE_X,
                            VIR_HALF_CHANNEL_MASK_FULL,
                            gcvNULL);
    return errCode;
}

static VSC_ErrCode
_UpdateSubGroupInvocationId(
    VIR_DEF_USAGE_INFO*     pDuInfo,
    VIR_Shader*             pShader,
    VIR_Instruction*        pInst,
    VIR_Operand*            pSrcOpnd,
    VIR_Symbol*             pSrcSym,
    void*                   pPrivateData
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    /* Delete usage. */
    vscVIR_DeleteUsage(pDuInfo,
                       VIR_INPUT_DEF_INST,
                       pInst,
                       pSrcOpnd,
                       gcvFALSE,
                       VIR_Symbol_GetVariableVregIndex(pSrcSym),
                       1,
                       VIR_ENABLE_X,
                       VIR_HALF_CHANNEL_MASK_FULL,
                       gcvNULL);

    VIR_Operand_SetImmediateUint(pSrcOpnd, 0);

    return errCode;
}

static VSC_ErrCode
_CollectSubGroupVariables(
    VSC_SH_PASS_WORKER*     pPassWorker,
    UPDATE_SUBGROUP_VARIABLE_INFO* psubGroupVariableInfo
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Symbol*             pSubGroupSizeSym = gcvNULL;
    VIR_Symbol*             pSubGroupNumSym = gcvNULL;
    VIR_Symbol*             pSubGroupIdSym = gcvNULL;
    VIR_Symbol*             pSubGroupInvocationIdSym = gcvNULL;


    /* Find all variables. */
    pSubGroupSizeSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_SUBGROUP_SIZE);
    pSubGroupNumSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_SUBGROUP_NUM);
    pSubGroupIdSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_SUBGROUP_ID);
    pSubGroupInvocationIdSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_SUBGROUP_INVOCATION_ID);

    if (!(pSubGroupSizeSym || pSubGroupNumSym || pSubGroupIdSym || pSubGroupInvocationIdSym))
    {
        return errCode;
    }

    /*
    **
    **    gl_SubgroupSize           --->    1
    **    gl_NumSubgroups           --->    gl_workGroupSize
    **    gl_SubgroupID             --->    gl_LocalInvocationIndex
    **    gl_SubgroupInvocationID   --->    0
    */

    psubGroupVariableInfo[0].pVariableSym = pSubGroupSizeSym;
    psubGroupVariableInfo[0].updateFunc = _UpdateSubGroupSize;

    psubGroupVariableInfo[1].pVariableSym = pSubGroupNumSym;
    psubGroupVariableInfo[1].updateFunc = _UpdateSubGroupNum;

    psubGroupVariableInfo[2].pVariableSym = pSubGroupIdSym;
    psubGroupVariableInfo[2].updateFunc = _UpdateSubGroupId;
    if (pSubGroupIdSym !=  gcvNULL)
    {
        VIR_Symbol*         pLocalInvocationIndex = gcvNULL;
        VIR_Symbol*         pVregSym = gcvNULL;

        pLocalInvocationIndex = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_LOCALINVOCATIONINDEX);

        if (pLocalInvocationIndex == gcvNULL)
        {
            VIR_AttributeIdList*pAttrIdLsts = VIR_Shader_GetAttributes(pShader);
            gctUINT             attrCount = VIR_IdList_Count(pAttrIdLsts);
            gctUINT             attrIdx;
            gctUINT             nextAttrLlSlot = 0;
            VIR_VirRegId        outRegId;
            VIR_SymId           outRegSymId;

            for (attrIdx = 0; attrIdx < attrCount; attrIdx ++)
            {
                VIR_SymId       attrSymId = VIR_IdList_GetId(pAttrIdLsts, attrIdx);
                VIR_Symbol      *pAttrSym = VIR_Shader_GetSymFromId(pShader, attrSymId);
                gctUINT         thisOutputRegCount;

                if (!isSymUnused(pAttrSym) && !isSymVectorizedOut(pAttrSym))
                {
                    gcmASSERT(VIR_Symbol_GetFirstSlot(pAttrSym) != NOT_ASSIGNED);

                    thisOutputRegCount = VIR_Symbol_GetVirIoRegCount(pShader, pAttrSym);
                    if (nextAttrLlSlot < (VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount))
                    {
                        nextAttrLlSlot = VIR_Symbol_GetFirstSlot(pAttrSym) + thisOutputRegCount;
                    }
                }
            }

            pLocalInvocationIndex = VIR_Shader_AddBuiltinAttribute(pShader,
                                                                   VIR_TYPE_UINT32,
                                                                   gcvFALSE,
                                                                   VIR_NAME_LOCALINVOCATIONINDEX);
            outRegId = VIR_Shader_NewVirRegId(pShader, 1);
            VIR_Shader_AddSymbol(pShader,
                                 VIR_SYM_VIRREG,
                                 outRegId,
                                 VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X4),
                                 VIR_STORAGE_UNKNOWN,
                                 &outRegSymId);

            pVregSym = VIR_Shader_GetSymFromId(pShader, outRegSymId);

            VIR_Symbol_SetVariableVregIndex(pLocalInvocationIndex, outRegId);
            VIR_Symbol_SetVregVariable(pVregSym, pLocalInvocationIndex);
            VIR_Symbol_SetIndexRange(pLocalInvocationIndex, outRegId + 1);
            VIR_Symbol_SetIndexRange(pVregSym, outRegId + 1);
            VIR_Symbol_SetFirstSlot(pLocalInvocationIndex, nextAttrLlSlot);

            /* gl_LocalInvocationIndex is never used before, we need to calculate it later. */
            VIR_Shader_SetFlagExt1(pShader, VIR_SHFLAG_EXT1_CALC_LOCAL_INVOCATION_INDEX);
        }
        else
        {
            pVregSym = VIR_Shader_FindSymbolByTempIndex(pShader, VIR_Symbol_GetVariableVregIndex(pLocalInvocationIndex));
        }

        psubGroupVariableInfo[2].pPrivateData = pVregSym;
    }

    psubGroupVariableInfo[3].pVariableSym = pSubGroupInvocationIdSym;
    psubGroupVariableInfo[3].updateFunc = _UpdateSubGroupInvocationId;

    return errCode;
}

static VSC_ErrCode
_UpdateSubGroup(
    VSC_SH_PASS_WORKER*     pPassWorker,
    UPDATE_SUBGROUP_VARIABLE_INFO* psubGroupVariableInfo,
    gctBOOL*                pChanged
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_DEF_USAGE_INFO*     pDuInfo = pPassWorker->pDuInfo;
    VIR_Shader*             pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode*       pFunc_node;
    gctBOOL                 bChanged = gcvFALSE, bMeetSubGroupId = gcvFALSE;
    gctUINT                 i;

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (pFunc_node = VIR_FuncIterator_First(&func_iter);
         pFunc_node != gcvNULL;
         pFunc_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_InstIterator        inst_iter;
        VIR_Function*           pFunc = pFunc_node->function;
        VIR_Instruction*        pInst = gcvNULL;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
            VIR_Operand*        pOpnd;
            VIR_SrcOperand_Iterator srcOpndIter;

            if (VIR_OPCODE_isNonUniform(opCode))
            {
                /* Always return TRUE for NONUNIFORM_ELECT. */
                if (opCode == VIR_OP_NONUNIFORM_ELECT)
                {
                    VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
                    VIR_Inst_SetSrcNum(pInst, 1);
                    VIR_Function_NewOperand(pFunc, &pOpnd);
                    VIR_Operand_SetImmediateBoolean(pOpnd, 1);
                    VIR_Inst_SetSource(pInst, 0, pOpnd);

                    bChanged = gcvTRUE;
                    continue;
                }
            }

            /* Find if any operand using subGroup variables and replace them */
            VIR_SrcOperand_Iterator_Init(pInst, &srcOpndIter);

            for (pOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);
                 pOpnd != gcvNULL;
                 pOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                if (!VIR_Operand_isSymbol(pOpnd))
                {
                    continue;
                }

                for (i = 0; i < 4; i++)
                {
                    if (psubGroupVariableInfo[i].pVariableSym != gcvNULL && VIR_Operand_GetSymbol(pOpnd) == psubGroupVariableInfo[i].pVariableSym)
                    {
                        (psubGroupVariableInfo[i].updateFunc)(pDuInfo,
                                                             pShader,
                                                             pInst,
                                                             pOpnd,
                                                             VIR_Operand_GetSymbol(pOpnd),
                                                             psubGroupVariableInfo[i].pPrivateData);

                        if (psubGroupVariableInfo[i].updateFunc == _UpdateSubGroupId)
                        {
                            bMeetSubGroupId = gcvTRUE;
                        }
                        bChanged = gcvTRUE;
                        break;
                    }
                }
            }
        }
    }

    /* Mark all variables as unused. */
    for (i = 0; i < 4; i++)
    {
        if (psubGroupVariableInfo[i].pVariableSym != gcvNULL)
        {
            VIR_Symbol_SetFlag(psubGroupVariableInfo[i].pVariableSym, VIR_SYMFLAG_UNUSED);
        }
    }

    if (bMeetSubGroupId)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateDu = gcvTRUE;
    }

    if (pChanged)
    {
        *pChanged = bChanged;
    }

    return errCode;
}

static VSC_ErrCode
_UpdateSubGroupOperations(
    VSC_SH_PASS_WORKER*     pPassWorker,
    gctBOOL*                pChanged
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    gctBOOL                 bChanged = gcvFALSE;
    UPDATE_SUBGROUP_VARIABLE_INFO subGroupVariableInfo[4];

    gcoOS_ZeroMemory(subGroupVariableInfo, gcmSIZEOF(UPDATE_SUBGROUP_VARIABLE_INFO) * 4);

    /* I: Collect subgroup variables. */
    errCode = _CollectSubGroupVariables(pPassWorker, subGroupVariableInfo);
    ON_ERROR(errCode, "Collect subgroup variables.");

    /* II: Update subgroup variables/instructions. */
    errCode = _UpdateSubGroup(pPassWorker, subGroupVariableInfo, &bChanged);
    ON_ERROR(errCode, "Update subgroup.");

    if (pChanged)
    {
        *pChanged = bChanged;
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PreprocessMCShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PreprocessMCShader)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_PreprocessMCShader(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    VIR_Uniform*    spillMemUniform;
    VIR_Shader*     pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    gctBOOL         bChanged = gcvFALSE;
    gctBOOL         needBoundsCheck =
                      (pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK) != 0;

    /* add temp register spill addr uniform (should be done before dubo) */
    spillMemUniform = VIR_Shader_GetTempRegSpillAddrUniform(pShader, needBoundsCheck);
    if (spillMemUniform == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }

    /* Update the local memory and the private memory */
    _UpdateLocMemAndPrivMem(pPassWorker);

    /* If enable the multi-GPU, we need to re-calculate the workGroupID. */
    if (pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_ENABLE_MULTI_GPU
        &&
        pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.hwFeatureFlags.supportMultiGPU
        &&
        VIR_Shader_IsCL(pShader))
    {
        _UpdateWorkGroupIdForMultiGPU(pPassWorker, &bChanged);
    }

    /* Handle subgroup-related variables/instructions. */
    if (VIR_Shader_IsVulkan(pShader) && VIR_Shader_IsGlCompute(pShader))
    {
        _UpdateSubGroupOperations(pPassWorker, &bChanged);
    }

    if (bChanged &&
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After preprocess MC shader.", pShader, gcvTRUE);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_PreprocessCGShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
}

DEF_SH_NECESSITY_CHECK(vscVIR_PreprocessCGShader)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_PreprocessCGShader(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader          *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Dumper          *pDumper = pPassWorker->basePassWorker.pDumper;
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode*   func_node;
    gctBOOL             bChanged = gcvFALSE;

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
            if (VIR_Shader_isDual16Mode(pShader))
            {
                gctBOOL bNeedRunSingleT = gcvFALSE;
                gctBOOL bDual16NotSupported = gcvFALSE;

                VIR_Inst_Check4Dual16(inst,
                                      &bNeedRunSingleT,
                                      &bDual16NotSupported,
                                      gcvNULL,
                                      gcvNULL,
                                      gcvNULL,
                                      HWSUPPORTDUAL16HIGHVEC2);

                /* We need to set this status before RA because RA uses this to set LR live range. */
                if (bNeedRunSingleT)
                {
                    VIR_Inst_SetThreadMode(inst, VIR_THREAD_D16_DUAL_32);
                    bChanged = gcvTRUE;
                }
            }
        }
    }

    if (bChanged && VirSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after preprocess CG", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return errCode;
}

typedef struct VSC_DYNAMIC_IDX_USAGE
{
    VIR_SymId           symId;
    VIR_Instruction*    pDefInst;
    VIR_BB*             pBB;
} VSC_DynamicIdx_Usage;

static gctUINT _VSC_DynamicIdx_Usage_HFUNC(const void *ptr)
{
    VSC_DynamicIdx_Usage* pDynamicIdxUsage = (VSC_DynamicIdx_Usage*)ptr;
    gctUINT hashKey = 0;

    if (pDynamicIdxUsage->pBB)
    {
        hashKey = (pDynamicIdxUsage->pBB->globalBbId & 0xFFFF) << 16;
    }

    hashKey |= (pDynamicIdxUsage->symId & 0xFFFF);

    return hashKey;
}

static gctBOOL _VSC_DynamicIdx_Usage_HKCMP(const void *pHashKey1, const void *pHashKey2)
{
    VSC_DynamicIdx_Usage* pDynamicIdxUsage1 = (VSC_DynamicIdx_Usage*)pHashKey1;
    VSC_DynamicIdx_Usage* pDynamicIdxUsage2 = (VSC_DynamicIdx_Usage*)pHashKey2;

    return (pDynamicIdxUsage1->symId == pDynamicIdxUsage2->symId)   &&
           (pDynamicIdxUsage1->pBB == pDynamicIdxUsage2->pBB);
}

static VSC_DynamicIdx_Usage* _VSC_DynamicIdx_NewUsage(
    VSC_MM*             pMM,
    VIR_SymId           symId,
    VIR_Instruction*    pDefInst,
    VIR_BB*             pBB
    )
{
    VSC_DynamicIdx_Usage   *pUsage = (VSC_DynamicIdx_Usage*)vscMM_Alloc(pMM, sizeof(VSC_DynamicIdx_Usage));

    pUsage->symId = symId;
    pUsage->pDefInst = pDefInst;
    pUsage->pBB = pBB;
    return pUsage;
}

static gctBOOL
_NeedToCheckDynamicIndexing(
    VIR_OpCode Opcode
    )
{
    gctBOOL needToCheck = gcvFALSE;

    if (VIR_OPCODE_isMemLd(Opcode)  ||
        VIR_OPCODE_isTexLd(Opcode)  ||
        VIR_OPCODE_isAtom(Opcode))
    {
        needToCheck = gcvTRUE;
    }

    return needToCheck;
}

DEF_QUERY_PASS_PROP(vscVIR_FixDynamicIdxDep)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_MC;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedRdFlow = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(vscVIR_FixDynamicIdxDep)
{
    VSC_HW_CONFIG*              pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;

    if (pHwCfg->hwFeatureFlags.hasDynamicIdxDepFix)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

VSC_ErrCode vscVIR_FixDynamicIdxDep(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode                 errCode = VSC_ERR_NONE;
    VSC_MM*                     pMM = pPassWorker->basePassWorker.pMM;
    VIR_DEF_USAGE_INFO*         pDuInfo = pPassWorker->pDuInfo;
    VIR_Shader*                 pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator            func_iter;
    VIR_FunctionNode*           func_node;
    VSC_HASH_TABLE*             usageInstHT = gcvNULL;
    VSC_HASH_ITERATOR           iter;
    VSC_DIRECT_HNODE_PAIR       pair;
    gctBOOL                     bChanged = gcvFALSE;

    /* Renumber instruction ID. */
    VIR_Shader_RenumberInstId(pShader);

    /* Create a HT to save the usage instruction. */
    usageInstHT = vscHTBL_Create(pMM,
                                 _VSC_DynamicIdx_Usage_HFUNC,
                                 _VSC_DynamicIdx_Usage_HKCMP,
                                 512);

    /* Find all the usages. */
    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function*    pFunc = func_node->function;
        VIR_InstIterator inst_iter;
        VIR_Instruction* pInst;

        VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pFunc));
        for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
             pInst != gcvNULL;
             pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
        {
            VIR_OpCode                  opCode = VIR_Inst_GetOpcode(pInst);
            VIR_Operand*                pDst = VIR_Inst_GetDest(pInst);
            VIR_SymId                   dstSymId = VIR_INVALID_ID;
            VIR_OperandInfo             dstOpndInfo;
            VIR_GENERAL_DU_ITERATOR     duIter;
            VIR_USAGE*                  pUsage = gcvNULL;
            VIR_Instruction*            pUsageInst = gcvNULL;
            VIR_OpCode                  usageInstOpcode;
            gctUINT8                    enable, channel;

            /* Check if we need to check the usage. */
            if (!_NeedToCheckDynamicIndexing(opCode))
            {
                continue;
            }

            VIR_Operand_GetOperandInfo(pInst, pDst, &dstOpndInfo);
            if (!VIR_OpndInfo_Is_Virtual_Reg(&dstOpndInfo))
            {
                continue;
            }
            dstSymId = VIR_Operand_GetSymbolId_(pDst);

            /* Find all usages. */
            enable = VIR_Inst_GetEnable(pInst);
            for (channel = VIR_CHANNEL_X; channel < VIR_CHANNEL_NUM; channel ++)
            {
                if (!VSC_UTILS_TST_BIT(enable, channel))
                {
                    continue;
                }

                vscVIR_InitGeneralDuIterator(&duIter,
                                             pDuInfo,
                                             pInst,
                                             dstOpndInfo.u1.virRegInfo.virReg,
                                             channel,
                                             gcvFALSE);

                for (pUsage = vscVIR_GeneralDuIterator_First(&duIter);
                     pUsage != gcvNULL;
                     pUsage = vscVIR_GeneralDuIterator_Next(&duIter))
                {
                    VSC_DynamicIdx_Usage* pDynamicIdxUsage = gcvNULL;
                    VIR_Instruction*      pPrevUsageInst = gcvNULL;

                    pUsageInst = pUsage->usageKey.pUsageInst;
                    /* Skip output usage. */
                    if (VIR_IS_OUTPUT_USAGE_INST(pUsageInst))
                    {
                        continue;
                    }

                    usageInstOpcode = VIR_Inst_GetOpcode(pUsageInst);
                    /* Skip non-dynamic-indexing instruction. */
                    if ((usageInstOpcode != VIR_OP_LDARR && usageInstOpcode != VIR_OP_STARR)
                        ||
                        (VIR_Inst_GetSource(pUsageInst, 0) != pUsage->usageKey.pOperand))
                    {
                        continue;
                    }

                    pDynamicIdxUsage = _VSC_DynamicIdx_NewUsage(pMM, dstSymId, pInst, VIR_Inst_GetBasicBlock(pUsageInst));

                    if (vscHTBL_DirectTestAndGet(usageInstHT,
                                                 (void*)pDynamicIdxUsage,
                                                 (void**)&pPrevUsageInst))
                    {
                        /* Update the data. */
                        if (VIR_Inst_GetId(pPrevUsageInst) > VIR_Inst_GetId(pUsageInst))
                        {
                            vscHTBL_DirectSet(usageInstHT,
                                             (void *)pDynamicIdxUsage,
                                             (void *)pUsageInst);
                        }
                        vscMM_Free(pMM, pDynamicIdxUsage);
                    }
                    else
                    {
                        vscHTBL_DirectSet(usageInstHT,
                                         (void *)pDynamicIdxUsage,
                                         (void *)pUsageInst);
                    }
                }
            }
        }
    }

    /* Insert a MOV before the usage instruction. */
    vscHTBLIterator_Init(&iter, usageInstHT);
    for (pair = vscHTBLIterator_DirectFirst(&iter);
         IS_VALID_DIRECT_HNODE_PAIR(&pair);
         pair = vscHTBLIterator_DirectNext(&iter))
    {
        VSC_DynamicIdx_Usage* pDynamicIdxUsage = (VSC_DynamicIdx_Usage*)VSC_DIRECT_HNODE_PAIR_FIRST(&pair);
        VIR_Instruction*      pDefInst = pDynamicIdxUsage->pDefInst;
        VIR_Instruction*      pUsageInst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_SECOND(&pair);
        VIR_Operand*          pUsageOpnd = VIR_Inst_GetSource(pUsageInst, 0);
        VIR_Symbol*           pSym = VIR_Shader_GetSymFromId(pShader, pDynamicIdxUsage->symId);
        VIR_Instruction*      pNewInst = gcvNULL;
        VIR_Operand*          pNewOpnd = gcvNULL;
        VIR_OperandInfo       opndInfo;

        /* Insert MOV. */
        errCode = VIR_Function_AddInstructionBefore(VIR_Inst_GetFunction(pUsageInst),
                                                    VIR_OP_MOV,
                                                    VIR_Symbol_GetTypeId(pSym),
                                                    pUsageInst,
                                                    gcvTRUE,
                                                    &pNewInst);
        ON_ERROR(errCode, "Add instruction before");

        pNewOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetOpKind(pNewOpnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(pNewOpnd, VIR_Symbol_GetTypeId(pSym));
        VIR_Operand_SetSym(pNewOpnd, pSym);
        VIR_Operand_SetPrecision(pNewOpnd, VIR_Symbol_GetPrecision(pSym));
        VIR_Operand_SetEnable(pNewOpnd, VIR_TypeId_Conv2Enable(VIR_Symbol_GetTypeId(pSym)));

        pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetOpKind(pNewOpnd, VIR_OPND_SYMBOL);
        VIR_Operand_SetTypeId(pNewOpnd, VIR_Symbol_GetTypeId(pSym));
        VIR_Operand_SetSym(pNewOpnd, pSym);
        VIR_Operand_SetPrecision(pNewOpnd, VIR_Symbol_GetPrecision(pSym));
        VIR_Operand_SetSwizzle(pNewOpnd, VIR_TypeId_Conv2Swizzle(VIR_Symbol_GetTypeId(pSym)));

        /* This MOV must be generated. */
        VIR_Inst_SetFlag(pNewInst, VIR_INSTFLAG_FORCE_GEN);

        /* Update DU. */
        VIR_Operand_GetOperandInfo(pNewInst, pNewOpnd, &opndInfo);
        gcmASSERT(VIR_OpndInfo_Is_Virtual_Reg(&opndInfo));

        /* Add a new def for MOV instruction. */
        vscVIR_AddNewDef(pDuInfo,
                         pNewInst,
                         opndInfo.u1.virRegInfo.virReg,
                         1,
                         VIR_TypeId_Conv2Enable(VIR_Symbol_GetTypeId(pSym)),
                         VIR_HALF_CHANNEL_MASK_FULL,
                         gcvNULL,
                         gcvNULL);

        /* Add a new usage for MOV instruction. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                pNewInst,
                                pUsageInst,
                                pUsageOpnd,
                                gcvFALSE,
                                opndInfo.u1.virRegInfo.virReg,
                                1,
                                VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pUsageOpnd)),
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

        /* Delete the old usage instruction. */
        vscVIR_DeleteUsage(pDuInfo,
                           pDefInst,
                           pUsageInst,
                           pUsageOpnd,
                           gcvFALSE,
                           opndInfo.u1.virRegInfo.virReg,
                           1,
                           VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pUsageOpnd)),
                           VIR_HALF_CHANNEL_MASK_FULL,
                           gcvNULL);

        /* Add a new usage for original def instruction. */
        vscVIR_AddNewUsageToDef(pDuInfo,
                                pDefInst,
                                pNewInst,
                                pNewOpnd,
                                gcvFALSE,
                                opndInfo.u1.virRegInfo.virReg,
                                1,
                                VIR_Swizzle_2_Enable(VIR_Operand_GetSwizzle(pNewOpnd)),
                                VIR_HALF_CHANNEL_MASK_FULL,
                                gcvNULL);

        /* Free dynamic idx usage node. */
        vscMM_Free(pMM, (void*)pDynamicIdxUsage);
        bChanged = gcvTRUE;
    }

    if (bChanged &&
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After fix dynamic indexing dependency issue.", pShader, gcvTRUE);
    }

OnError:
    /* Destroy the HT. */
    vscHTBL_Destroy(usageInstHT);

    return errCode;
}

static VIR_Symbol *
_FindSampledImageSym(
    IN VIR_Shader*      pShader,
    IN VIR_Instruction* pInst,
    IN VIR_Symbol*      pSym,
    IN gctBOOL          bIsImage
    )
{
    VIR_VirRegId        virRegId = VIR_Symbol_GetVregIndex(pSym);
    VIR_Instruction*    pInstIter = gcvNULL;

    for (pInstIter = VIR_Inst_GetPrev(pInst);
         pInstIter && (VIR_Inst_GetFunction(pInstIter) == VIR_Inst_GetFunction(pInst));
         pInstIter = VIR_Inst_GetPrev(pInstIter))
    {
        VIR_Operand*    pDest = VIR_Inst_GetDest(pInstIter);
        VIR_OpCode      opCode = VIR_Inst_GetOpcode(pInstIter);
        VIR_Operand*    pOpnd = gcvNULL;
        VIR_Symbol*     pOpndSym = gcvNULL;

        if (pDest && VIR_Symbol_GetVregIndex(VIR_Operand_GetSymbol(pDest)) == virRegId)
        {
            if (opCode == VIR_OP_GET_SAMPLER_IDX)
            {
                gcmASSERT(!bIsImage);
                return VIR_Operand_GetSymbol(VIR_Inst_GetSource(pInstIter, 0));
            }
            else if (opCode == VIR_OP_MOV)
            {
                pOpnd = VIR_Inst_GetSource(pInstIter, 0);
                if (VIR_Operand_isSymbol(pOpnd))
                {
                    pOpndSym = VIR_Operand_GetSymbol(pOpnd);

                    if (bIsImage && VIR_Symbol_isImage(pOpndSym))
                    {
                        return pOpndSym;
                    }
                    else if (VIR_Symbol_isSampler(pOpndSym))
                    {
                        return pOpndSym;
                    }
                    else
                    {
                        return _FindSampledImageSym(pShader, pInstIter, pOpndSym, bIsImage);
                    }
                }
            }
        }
    }

    return gcvNULL;
}

static VSC_ErrCode
_UpdateSampledImageAssignment(
    IN VIR_Shader*      pShader,
    IN VIR_Instruction* pInst,
    IN VIR_Symbol*      pSym,
    IN gctBOOL          bIsImage
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Symbol*         pSampledImageSym = bIsImage ? VIR_Symbol_GetSeparateImage(pShader, pSym) : VIR_Symbol_GetSeparateSampler(pShader, pSym);
    VIR_Symbol*         pUpdatedSym = gcvNULL;
    gctBOOL             bNeedToUpdate = gcvFALSE;

    if (bIsImage)
    {
        bNeedToUpdate = !VIR_Symbol_isImage(pSampledImageSym);
    }
    else
    {
        bNeedToUpdate = !VIR_Symbol_isSampler(pSampledImageSym);
    }

    if (bNeedToUpdate)
    {
        pUpdatedSym = _FindSampledImageSym(pShader, pInst, pSampledImageSym, bIsImage);
        gcmASSERT(pUpdatedSym);

        if (bIsImage)
        {
            gcmASSERT(VIR_Symbol_isImage(pUpdatedSym));
            VIR_Symbol_SetSeparateImageId(pSym, VIR_Symbol_GetIndex(pUpdatedSym), VIR_INVALID_ID);
        }
        else
        {
            gcmASSERT(VIR_Symbol_isSampler(pUpdatedSym));
            VIR_Symbol_SetSeparateSamplerId(pSym, VIR_Symbol_GetIndex(pUpdatedSym), VIR_INVALID_ID);
        }
    }

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
    VIR_Symbol  *pSymbol = gcvNULL;
    VIR_Uniform *pUniform = gcvNULL, *pSeparateImageUniform = gcvNULL, *pSeparateSamplerUniform = gcvNULL;
    VIR_Id      id ;
    VIR_Symbol  *uniformSym = gcvNULL, *separateImage = gcvNULL, *separateSampler = gcvNULL;
    gctUINT i;

    if (!VIR_Operand_isSymbol(pOpnd) && !VIR_Operand_isVirReg(pOpnd))
    {
        return errCode;
    }

    pSymbol = VIR_Operand_GetSymbol(pOpnd);
    if (VIR_Symbol_isVreg(pSymbol))
    {
        pSymbol = VIR_Symbol_GetVregVariable(pSymbol);
    }

    if (pSymbol == gcvNULL || !isSymCombinedSampler(pSymbol))
    {
        return errCode;
    }

    /* Check if we need to find the real sampledImage. */
    _UpdateSampledImageAssignment(pShader, pInst, pSymbol, gcvTRUE);
    _UpdateSampledImageAssignment(pShader, pInst, pSymbol, gcvFALSE);

    for (i = 0; i < (gctINT) VIR_IdList_Count(&pShader->uniforms); i ++)
    {
        id  = VIR_IdList_GetId(&pShader->uniforms, i);
        uniformSym = VIR_Shader_GetSymFromId(pShader, id);

        if (VIR_Symbol_GetUniformKind(uniformSym) == VIR_UNIFORM_SAMPLED_IMAGE)
        {
            if (VIR_Symbol_GetSeparateImageId(uniformSym) == VIR_Symbol_GetSeparateImageId(pSymbol) &&
                VIR_Symbol_GetSeparateImageFuncId(uniformSym) == VIR_Symbol_GetSeparateImageFuncId(pSymbol) &&
                VIR_Symbol_GetImgIdxRange(uniformSym) == VIR_Symbol_GetImgIdxRange(pSymbol) &&
                VIR_Symbol_GetSeparateSamplerId(uniformSym) == VIR_Symbol_GetSeparateSamplerId(pSymbol) &&
                VIR_Symbol_GetSeparateSamplerFuncId(uniformSym) == VIR_Symbol_GetSeparateSamplerFuncId(pSymbol) &&
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

        VIR_Symbol_SetSeparateSamplerId(uniformSym, VIR_Symbol_GetSeparateSamplerId(pSymbol), VIR_Symbol_GetSeparateSamplerFuncId(pSymbol));
        VIR_Symbol_SetSamplerIdxRange(uniformSym, VIR_Symbol_GetSamplerIdxRange(pSymbol));
        VIR_Symbol_SetSeparateImageId(uniformSym, VIR_Symbol_GetSeparateImageId(pSymbol), VIR_Symbol_GetSeparateImageFuncId(pSymbol));
        VIR_Symbol_SetImgIdxRange(uniformSym, VIR_Symbol_GetImgIdxRange(pSymbol));
        VIR_Symbol_SetUniformKind(uniformSym, VIR_UNIFORM_SAMPLED_IMAGE);

        pUniform = VIR_Symbol_GetSampler(uniformSym);
        pUniform->index = VIR_IdList_Count(VIR_Shader_GetUniforms(pShader)) - 1;

        /* Get the separate sampler/image. */
        separateSampler = VIR_Symbol_GetSeparateSampler(pShader, pSymbol);
        separateImage = VIR_Symbol_GetSeparateImage(pShader, pSymbol);

        gcmASSERT(separateSampler && separateImage);

        pSeparateSamplerUniform = VIR_Symbol_GetUniformPointer(pShader, separateSampler);
        pSeparateImageUniform = VIR_Symbol_GetUniformPointer(pShader, separateImage);

        /* clear the flag */
        VIR_Symbol_ClrFlag(separateSampler, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);
        VIR_Symbol_ClrFlag(separateImage, VIR_SYMUNIFORMFLAG_USED_IN_SHADER);

        /* Save the sampledImage symbol ID. */
        pSeparateSamplerUniform->u.samplerOrImageAttr.sampledImageSymId = id;
        pSeparateImageUniform->u.samplerOrImageAttr.sampledImageSymId = id;
    }

    VIR_Operand_SetSym(pOpnd, VIR_Shader_GetSymFromId(pShader, pUniform->sym));

    return errCode;
}

DEF_QUERY_PASS_PROP(vscVIR_GenCombinedSampler)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
}

DEF_SH_NECESSITY_CHECK(vscVIR_GenCombinedSampler)
{
    return gcvTRUE;
}

VSC_ErrCode vscVIR_GenCombinedSampler(VSC_SH_PASS_WORKER* pPassWorker)
{
    VIR_Shader *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
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
            VIR_OpCode              opcode = VIR_Inst_GetOpcode(inst);

            /* for OpenCL image read/load, generate combined <image, sampler> */
            if (VIR_OPCODE_isImgLd(opcode))
            {
                _vscGenerateNewImageUniformIfNeeded(pPassWorker, inst);
                /* check if the combined <image, sampler> need to transform to lib function call */
                if (_vscTransformImgReadToLibFuncCall(pPassWorker, inst))
                {
                    continue;
                }
            }
            else if (VIR_OPCODE_isImgSt(opcode) )
            {
                /* check if the img_store need to transform to lib function call */
                if (_vscTransformImgWriteToLibFuncCall(pPassWorker, inst))
                {
                    continue;
                }
            }

            /* Now find the combinedSampler for vulkan. */
            if (!VIR_Shader_IsVulkan(pShader))
            {
                continue;
                }
            VIR_SrcOperand_Iterator_Init(inst, &srcOpndIter);
            srcOpnd = VIR_SrcOperand_Iterator_First(&srcOpndIter);

            for (; srcOpnd != gcvNULL; srcOpnd = VIR_SrcOperand_Iterator_Next(&srcOpndIter))
            {
                _GenCombinedSamplerOpnd(pShader, inst, srcOpnd);
            }
        }
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Generating combined sampler", pShader, gcvTRUE);
    }

    return errCode;
}

/* dfs recursive to check src0 of ATOM* instruction is same address for all threads in a shader group.
 * if src0 is defined by const/uniform/attribute-VIR_NAME_WORK_GROUP_ID, it would be fine
 * otherwise return false
 */
static gctBOOL _vscVIR_CheckAtomSrcIsSameAddrForAllthreads(
    VIR_Instruction *inst,
    VIR_Operand *srcOpnd,
    VIR_DEF_USAGE_INFO*        pDuInfo)
{
    VIR_OperandInfo         srcOpndInfo;
    VIR_Symbol *srcSym = gcvNULL;

    gcmASSERT(inst && srcOpnd);

    if (VIR_Operand_isSymbol(srcOpnd))
    {
        srcSym = VIR_Operand_GetSymbol(srcOpnd);
    }

    if (VIR_Operand_isImm(srcOpnd) || VIR_Operand_isConst(srcOpnd) ||
        (srcSym && VIR_Symbol_isUniform(srcSym)))
    {
        return gcvTRUE;
    }

    VIR_Operand_GetOperandInfo(inst, srcOpnd, &srcOpndInfo);
    if (VIR_OpndInfo_Is_Virtual_Reg(&srcOpndInfo))
    {
        /* if src is indexing, return false */
        if (srcOpndInfo.indexingVirRegNo != VIR_INVALID_REG_NO)
        {
            return gcvFALSE;
        }
        /* check def symbol recursively */
        {
            VIR_GENERAL_UD_ITERATOR udIter;
            VIR_DEF             *pDef;
            gctBOOL             earlyExit = gcvFALSE;
            vscVIR_InitGeneralUdIterator(&udIter,
                                          pDuInfo, inst, srcOpnd, gcvFALSE, gcvFALSE);
            for (pDef = vscVIR_GeneralUdIterator_First(&udIter);
                 pDef != gcvNULL;
                 pDef = vscVIR_GeneralUdIterator_Next(&udIter))
            {
                VIR_Instruction *pDefInst = pDef->defKey.pDefInst;
                gcmASSERT(pDefInst);
                /* if src is gl_WorkGroupID, return true since it's same value for a workgroup */
                if (VIR_IS_IMPLICIT_DEF_INST(pDefInst))
                {
                    VIR_Symbol *attr = VIR_Operand_GetSymbol(srcOpnd);
                    if (attr && (VIR_Symbol_GetName(attr) == VIR_NAME_WORK_GROUP_ID))
                    {
                        continue;
                    }
                    /* other input, return false now */
                    earlyExit = gcvTRUE;
                    break;
                }
                /* if definition is memory instruction, return false */
                else if (pDefInst > 0 && VIR_OPCODE_isMemLd(VIR_Inst_GetOpcode(pDefInst)))
                {
                    /* value from memory load is complex, return false */
                    earlyExit = gcvTRUE;
                    break;
                }
                else
                {
                    gctUINT i;
                    for (i = 0; i < VIR_Inst_GetSrcNum(pDefInst); i++)
                    {
                        if(!_vscVIR_CheckAtomSrcIsSameAddrForAllthreads(pDefInst, VIR_Inst_GetSource(pDefInst, i), pDuInfo))
                        {
                            earlyExit = gcvTRUE;
                            break;
                        }
                    }
                }
                if (earlyExit)
                {
                    break; /* return false */
                }
            }
            /* all source are checked */
            if (!earlyExit)
            {
                return gcvTRUE;
            }
        }
    }

    return gcvFALSE;
}


static void _vscVIR_ConstructAtomFuncName(
    VIR_Instruction *pInst,
    gctUINT         maxcore,
    gctBOOL         ocl,
    OUT gctSTRING   *pLibFuncName          /* returned lib function name if needed */
    )
{
    gctUINT srcNum = VIR_Inst_GetSrcNum(pInst);
    gctUINT i;
    gctCHAR name[256] = "";
    gctBOOL destTypeIsInt = gcvFALSE;

    switch (VIR_Inst_GetOpcode(pInst))
    {
        case VIR_OP_ATOMADD: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomadd"); break;
        case VIR_OP_ATOMSUB: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomsub"); break;
        case VIR_OP_ATOMXCHG: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomxchg"); break;
        case VIR_OP_ATOMCMPXCHG: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomcmpxchg"); break;
        case VIR_OP_ATOMMIN: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atommin"); break;
        case VIR_OP_ATOMMAX: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atommax"); break;
        case VIR_OP_ATOMOR: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomor"); break;
        case VIR_OP_ATOMAND: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomand"); break;
        case VIR_OP_ATOMXOR: gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_atomxor"); break;
        default: gcmASSERT(0); break; /*unsupported atom */
    }

    /* get dest type */
    if (VIR_Inst_GetDest(pInst) && (VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst)) == VIR_TYPE_UINT32)) {
        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_uint");
    }
    else
    {
        destTypeIsInt = gcvTRUE;
        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_int");
    }
    /* skip last _undef src in ML*/
    for (i = 0; i < srcNum-1; i++)
    {
        VIR_Operand *src = VIR_Inst_GetSource(pInst, i);
        VIR_TypeId srcTypeId = VIR_GetTypeComponentType(VIR_Operand_GetTypeId(src));;
        if (srcTypeId == VIR_TYPE_UINT32)
        {
            if (i == srcNum-2)
            {
                /* special deal to atomcmpxchg, src2 store two values */
                if (VIR_Inst_GetOpcode(pInst) == VIR_OP_ATOMCMPXCHG)
                {
                    if (ocl)
                    {
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_uint2");
                    }
                    else
                    {
                        gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_uvec2");
                    }
                }
                else if (destTypeIsInt)
                {
                    /* convert "atom*_int_uint_uint" to "atom*_int_uint_int" case
                     * to reduce library function implementation
                     */
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_int");
                }
                else
                {
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_uint");
                }
            }
            else
            {
                gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_uint");
            }
        }
        else if (srcTypeId == VIR_TYPE_INT32)
        {
            /* special deal to atomcmpxchg, src2 store two values */
            if ((i == srcNum-2) && VIR_Inst_GetOpcode(pInst) == VIR_OP_ATOMCMPXCHG)
            {
                if (ocl)
                {
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_int2");
                }
                else
                {
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_vec2");
                }
            }
            else
            {
                gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_int");
            }
        }
        else if (srcTypeId == VIR_TYPE_FLOAT32)
        {
            /* special deal to atomcmpxchg, src2 store two values */
            if ((i == srcNum-2) && VIR_Inst_GetOpcode(pInst) == VIR_OP_ATOMCMPXCHG)
            {
                if (ocl)
                {
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_float2");
                }
                else
                {
                    gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_vec2");
                }
            }
            else
            {
                gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_float");
            }
        }
        else
        {
            gcmASSERT(0);
        }
    }

    switch(maxcore)
    {
        case 1:
        {
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_core1");
            break;
        }
        case 2:
        {
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_core2");
            break;
        }
        case 4:
        {
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_core4");
            break;
        }
        case 8:
        {
            gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_core8");
            break;
        }
        default:
            gcmASSERT(0); break;
    }

    gcoOS_StrDup(gcvNULL, name, pLibFuncName);

}

static VSC_ErrCode _vscVIR_ReplaceAtomWithExtCall(
    VIR_Instruction *pInst,
    gctUINT         maxcoreCount,
    gctBOOL         ocl)
{
    VSC_ErrCode   errCode = VSC_ERR_NONE;
    gctSTRING     libFuncName = gcvNULL;
    VIR_Function *func = VIR_Inst_GetFunction(pInst);
    VIR_Operand  *funcName;
    VIR_NameId    funcNameId;
    VIR_Operand  *parameters;
    VIR_ParmPassing *parm;
    gctINT argCount = VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(pInst)) - 1; /* last argument is undef in ML IR */
    gctINT   i;

    gcmASSERT(func != gcvNULL);

    {
        _vscVIR_ConstructAtomFuncName(pInst, maxcoreCount, ocl, &libFuncName);
        /* create FuncOperand*/
        ON_ERROR(VIR_Function_NewOperand(func, &funcName), "Failed to new operand");
        ON_ERROR(VIR_Shader_AddString(func->hostShader, libFuncName, &funcNameId), "");
        VIR_Operand_SetName(funcName, funcNameId);
        ON_ERROR(VIR_Function_NewOperand(func, &parameters), "Failed to new operand");

        /* create a new VIR_ParmPassing */
        ON_ERROR(VIR_Function_NewParameters(func, argCount, &parm), "Failed to copy operand");
        for (i = 0; i < argCount; i++)
        {
             VIR_Operand_Copy(parm->args[i], VIR_Inst_GetSource(pInst, i));
        }

        /* set parameters to new operand */
        VIR_Operand_SetParameters(parameters, parm);

        VIR_Inst_SetSource(pInst, 0, funcName);
        VIR_Inst_SetSource(pInst, 1, parameters);
        VIR_Inst_SetOpcode(pInst, VIR_OP_EXTCALL);
        VIR_Inst_SetSrcNum(pInst, 2);

        gcmOS_SAFE_FREE(gcvNULL, libFuncName);

        return errCode;
    }
OnError:
    if (libFuncName != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, libFuncName);
    }
    return errCode;
}

DEF_SH_NECESSITY_CHECK(vscVIR_GenExternalAtomicCall)
{
    VSC_HW_CONFIG*   pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    gctSTRING env = gcvNULL;
    VIR_Shader *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    gctBOOL  isOpenCV = gcvFALSE;
    gctBOOL  hasAtomInst = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VIV_ENABLE_OPENCV_WORKGROUPSIZE", &env);
    isOpenCV = (env && (gcoOS_StrCmp(env, "1") == 0));

    if ((!pHwCfg->hwFeatureFlags.supportUSC) || pHwCfg->hwFeatureFlags.hasUSCAtomicFix2)
    {
        return gcvFALSE;
    }
    /* apply patch to identified appNameId or VIR_ENABLE_OPENCV_WORKGROUP = 1 is set*/
    if ((pPassWorker->pCompilerParam->cfg.ctx.appNameId != gcvPATCH_OPENCV_ATOMIC) && (!isOpenCV))
    {
        return gcvFALSE;
    }
    /* check shader has atom instruction */
    {
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
                if (VIR_Inst_GetOpcode(inst) >= VIR_OP_ATOMADD && VIR_Inst_GetOpcode(inst) <= VIR_OP_ATOMXOR)
                {
                    hasAtomInst = gcvTRUE;
                    break;
                }
            }
        }
    }

    return hasAtomInst;
}

DEF_QUERY_PASS_PROP(vscVIR_GenExternalAtomicCall)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
    /* enable/disable according to the value of "VSC_PASS_OPTN_TYPE_ILF_LINK" except env control */
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_ATOM_PATCH;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

VSC_ErrCode vscVIR_GenExternalAtomicCall(VSC_SH_PASS_WORKER* pPassWorker)
{
    VIR_Shader *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    gctUINT     maxcoreCount = pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg.maxCoreCount;
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_DEF_USAGE_INFO*        pDuInfo = pPassWorker->pDuInfo;
    VIR_FuncIterator func_iter;
    VIR_FunctionNode* func_node;
    gctBOOL           extcallgenerated = gcvFALSE;

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
            if (VIR_Inst_GetOpcode(inst) >= VIR_OP_ATOMADD && VIR_Inst_GetOpcode(inst) <= VIR_OP_ATOMXOR)
            {
                /* check operand */
                if (!_vscVIR_CheckAtomSrcIsSameAddrForAllthreads(inst, VIR_Inst_GetSource(inst, 0), pDuInfo))
                {
                    _vscVIR_ReplaceAtomWithExtCall(inst, maxcoreCount, (pPassWorker->pCompilerParam->cfg.ctx.clientAPI == gcvAPI_OPENCL));
                    extcallgenerated = gcvTRUE;
                }
            }
        }
    }
    /* invalid callgraph if extcall is generated */
    if (extcallgenerated)
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCg = gcvTRUE;
    }

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Generating Atom EXTCALL", pShader, gcvTRUE);
    }

    return errCode;
}

static VSC_ErrCode _vscVIR_InsertBoundCheckBefore(
    VIR_Shader* pShader,
    VIR_Function *pFunc,
    VIR_Instruction *inst, /* insert before here */
    VIR_Label *labelelse)
{
    VIR_VirRegId addrRegNo, newRegNo;
    VIR_SymId    addrSymId, newSymId;
    VIR_Instruction *addInst = gcvNULL, *addUpInst = gcvNULL;
    VIR_Instruction *jmpcInst = gcvNULL, *jmpc1Inst = gcvNULL;
    VIR_Operand* baseOper = VIR_Inst_GetSource(inst, 0);
    VIR_Operand* offsetOper = VIR_Inst_GetSource(inst, 1);
    VIR_Operand *newdestOper, *src0Oper, *src1Oper;
    VSC_ErrCode errCode;
    VIR_TypeId destOperTypeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(inst));
    gctUINT     destOperTypeSize;

    addrRegNo = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   addrRegNo,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &addrSymId);
    /* add t1.x, base.x, offset */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_ADD,
                VIR_TYPE_UINT32,
                inst,
                gcvTRUE,
                &addInst);
    newdestOper = VIR_Inst_GetDest(addInst);
    VIR_Operand_SetSymbol(newdestOper, pFunc, addrSymId);
    VIR_Operand_SetEnable(newdestOper, VIR_ENABLE_X);

    src0Oper = VIR_Inst_GetSource(addInst, 0);
    VIR_Operand_Copy(src0Oper, baseOper);
    VIR_Operand_SetSwizzle(src0Oper, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetTypeId(src0Oper, VIR_TYPE_UINT32);

    src1Oper = VIR_Inst_GetSource(addInst, 1);
    VIR_Operand_Copy(src1Oper, offsetOper);
    VIR_Operand_SetSwizzle(src1Oper, VIR_Operand_GetSwizzle(offsetOper));

    /* JMPC.lt labelelse, t1.x, base.y*/
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                  VIR_OP_JMPC,
                  VIR_TYPE_UNKNOWN,
                  inst,
                  gcvTRUE,
                  &jmpcInst);

    src0Oper = VIR_Inst_GetSource(jmpcInst, 0);
    VIR_Operand_SetSymbol(src0Oper, pFunc, addrSymId);
    VIR_Operand_SetSwizzle(src0Oper, VIR_SWIZZLE_XXXX);

    src1Oper = VIR_Inst_GetSource(jmpcInst, 1);
    VIR_Operand_Copy(src1Oper, baseOper);
    VIR_Operand_SetSwizzle(src1Oper, VIR_SWIZZLE_YYYY);
    VIR_Operand_SetTypeId(src1Oper, VIR_TYPE_UINT32);

    VIR_Operand_SetLabel(VIR_Inst_GetDest(jmpcInst), labelelse);
    VIR_Inst_SetConditionOp(jmpcInst, VIR_COP_LESS);

    gcmASSERT(VIR_TypeId_isPrimitive(destOperTypeId));
    destOperTypeSize = VIR_GetTypeSize(destOperTypeId);

    /*add t2.x, t1.x, (destOperTypeSize -1)*/
    newRegNo = VIR_Shader_NewVirRegId(pShader, 1);
    errCode = VIR_Shader_AddSymbol(pShader,
                                   VIR_SYM_VIRREG,
                                   newRegNo,
                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT32),
                                   VIR_STORAGE_UNKNOWN,
                                   &newSymId);
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                VIR_OP_ADD,
                VIR_TYPE_UINT32,
                inst,
                gcvTRUE,
                &addUpInst);
    newdestOper = VIR_Inst_GetDest(addUpInst);
    VIR_Operand_SetSymbol(newdestOper, pFunc, newSymId);
    VIR_Operand_SetEnable(newdestOper, VIR_ENABLE_X);

    src0Oper = VIR_Inst_GetSource(addUpInst, 0);
    VIR_Operand_SetSymbol(src0Oper, pFunc, addrSymId);
    VIR_Operand_SetSwizzle(src0Oper, VIR_SWIZZLE_XXXX);
    VIR_Operand_SetTypeId(src0Oper, VIR_TYPE_UINT32);

    src1Oper = VIR_Inst_GetSource(addUpInst, 1);
    VIR_Operand_SetImmediateUint(src1Oper, (destOperTypeSize-1));
    VIR_Operand_SetSwizzle(src1Oper, VIR_SWIZZLE_XXXX);

    /*JMPC.GT labelelse, t2.x, base.z */
    errCode = VIR_Function_AddInstructionBefore(pFunc,
                  VIR_OP_JMPC,
                  VIR_TYPE_UNKNOWN,
                  inst,
                  gcvTRUE,
                  &jmpc1Inst);

    src0Oper = VIR_Inst_GetSource(jmpc1Inst, 0);
    VIR_Operand_SetSymbol(src0Oper, pFunc, newSymId);
    VIR_Operand_SetSwizzle(src0Oper, VIR_SWIZZLE_XXXX);

    src1Oper = VIR_Inst_GetSource(jmpc1Inst, 1);
    VIR_Operand_Copy(src1Oper, baseOper);
    VIR_Operand_SetSwizzle(src1Oper, VIR_SWIZZLE_ZZZZ);
    VIR_Operand_SetTypeId(src1Oper, VIR_TYPE_UINT32);

    VIR_Operand_SetLabel(VIR_Inst_GetDest(jmpc1Inst), labelelse);
    VIR_Inst_SetConditionOp(jmpc1Inst, VIR_COP_GREATER);

    return errCode;
}

static VSC_ErrCode _vscVIR_GenerateLoadBoundCheck(
    VIR_Shader      *pShader,
    VIR_Function    *pFunc,
    VIR_Instruction *inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction *labelelse_inst = gcvNULL, *labelmerge_inst = gcvNULL;
    VIR_Label       *labelelse      = gcvNULL, *labelmerge = gcvNULL;
    VIR_LabelId      labelelse_id, labelmerge_id;
    VIR_Instruction *movInst = gcvNULL;
    VIR_Instruction *jmpInst = gcvNULL;
    /* else branch after inst */
    VIR_Function_AddInstructionAfter(pFunc,
                                     VIR_OP_LABEL,
                                     VIR_TYPE_UNKNOWN,
                                     inst,
                                     gcvTRUE,
                                     &labelelse_inst);
    VIR_Function_AddLabel(pFunc, gcvNULL, &labelelse_id);
    labelelse = VIR_GetLabelFromId(pFunc, labelelse_id);
    labelelse->defined = labelelse_inst;
    VIR_Operand_SetLabel(VIR_Inst_GetDest(labelelse_inst), labelelse);

    /* lowBound check
     * if base.x+ offset < base.y
     *   goto labelelse
     * if base.x + offset + (sizePtr-1) > base.z
     *   goto labelelse
     *    load
     *    jmp labelmerge;
     * labelelse:
     *    mov
     * labelmerge:
     */
    errCode = _vscVIR_InsertBoundCheckBefore(pShader, pFunc, inst, labelelse);

    /*else branch : mov dest, 0 */
    {
        VIR_Operand *origDest = VIR_Inst_GetDest(inst);
        VIR_Operand *movDest = gcvNULL;
        VIR_Type *origDestType = VIR_Shader_GetTypeFromId(pShader, VIR_Operand_GetTypeId(origDest));
        VIR_Function_AddInstructionAfter(pFunc,
                                         VIR_OP_MOV,
                                         VIR_Operand_GetTypeId(origDest),
                                         labelelse_inst,
                                         gcvTRUE,
                                         &movInst);
        movDest = VIR_Inst_GetDest(movInst);
        VIR_Operand_Copy(movDest, origDest);
        VIR_Operand_SetEnable(movDest, VIR_Operand_GetEnable(origDest));

        if (VIR_Type_isInteger(origDestType))
        {
            VIR_Operand_SetImmediateInt(VIR_Inst_GetSource(movInst, 0), 0);
        }
        else if (VIR_Type_isFloat(origDestType))
        {
            VIR_Operand_SetImmediateFloat(VIR_Inst_GetSource(movInst, 0), 0.0);
        }
        else
        {
            gcmASSERT(0);
        }
    }
    /* label merge: */
    {
        VIR_Function_AddInstructionAfter(pFunc,
                                         VIR_OP_LABEL,
                                         VIR_TYPE_UNKNOWN,
                                         movInst,
                                         gcvTRUE,
                                         &labelmerge_inst);
        VIR_Function_AddLabel(pFunc, gcvNULL, &labelmerge_id);
        labelmerge = VIR_GetLabelFromId(pFunc, labelmerge_id);
        labelmerge->defined = labelmerge_inst;
        VIR_Operand_SetLabel(VIR_Inst_GetDest(labelmerge_inst), labelmerge);
    }

    /* insert jmp after inst
     *        inst
     *        jmp labelmerge
     */
    {
        errCode = VIR_Function_AddInstructionAfter(pFunc,
                    VIR_OP_JMP,
                    VIR_TYPE_UNKNOWN,
                    inst,
                    gcvTRUE,
                    &jmpInst);
        VIR_Operand_SetLabel(VIR_Inst_GetDest(jmpInst), labelmerge);
    }
    return errCode;
}

static VSC_ErrCode _vscVIR_GenerateStoreBoundCheck(
    VIR_Shader      *pShader,
    VIR_Function    *pFunc,
    VIR_Instruction *inst
    )
{
    VSC_ErrCode errCode;
    VIR_Instruction *labelmerge_inst = gcvNULL;
    VIR_Label       *labelmerge = gcvNULL;
    VIR_LabelId      labelmerge_id;

    /* else branch after inst */
    VIR_Function_AddInstructionAfter(pFunc,
                                     VIR_OP_LABEL,
                                     VIR_TYPE_UNKNOWN,
                                     inst,
                                     gcvTRUE,
                                     &labelmerge_inst);
    VIR_Function_AddLabel(pFunc, gcvNULL, &labelmerge_id);
    labelmerge = VIR_GetLabelFromId(pFunc, labelmerge_id);
    labelmerge->defined = labelmerge_inst;
    VIR_Operand_SetLabel(VIR_Inst_GetDest(labelmerge_inst), labelmerge);

    /* Bound check
     *  if base.x < base.y-offset
     *      jmp labelmerge
     *  if base.x + offset + (size-1) > base.z
     *      jmp labelmerge
     *  store
     * labelmerge:
     */
    errCode = _vscVIR_InsertBoundCheckBefore(pShader, pFunc, inst, labelmerge);

    return errCode;
}

DEF_SH_NECESSITY_CHECK(vscVIR_GenRobustBoundCheck)
{
    VSC_HW_CONFIG*   pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    /* if vscVIR_AddOutOfBoundCheckSupport is not called, or HW support OOB-check
     * skip generating bound check
     */
    if ((!(pPassWorker->pCompilerParam->cfg.cFlags & VSC_COMPILER_FLAG_NEED_OOB_CHECK)) ||
        pHwCfg->hwFeatureFlags.supportOOBCheck)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

DEF_QUERY_PASS_PROP(vscVIR_GenRobustBoundCheck)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

VSC_ErrCode vscVIR_GenRobustBoundCheck(VSC_SH_PASS_WORKER* pPassWorker)
{
    VIR_Shader *pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
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
            VIR_OpCode opc = VIR_Inst_GetOpcode(inst);
            if (VIR_OPCODE_isMemLd(opc) ||
                VIR_OPCODE_isAttrLd(opc) ||
                VIR_OPCODE_isAtom(opc) ||
                VIR_OPCODE_isMemSt(opc))
            {
                VIR_Operand *baseOper = VIR_Inst_GetSource(inst, 0);
                VIR_Symbol *pSym = VIR_Operand_GetSymbol(baseOper);
                if (VIR_Operand_GetSwizzle(baseOper) != VIR_SWIZZLE_XYZZ ||
                    (pSym && (VIR_Symbol_isInput(pSym) || VIR_Symbol_isOutput(pSym))))
                {
                    /* if base address is not a flat pointer or input/output, skip */
                    continue;
                }
                if (VIR_OPCODE_isMemSt(opc))
                {
                    /*if out of bound, skip the store instruction */
                    _vscVIR_GenerateStoreBoundCheck(pShader, func, inst);
                }
                else
                {
                    _vscVIR_GenerateLoadBoundCheck(pShader, func, inst);
                }
            }
        }
    }

    pPassWorker->pResDestroyReq->s.bInvalidateCfg = gcvTRUE;

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After Generating Robust bound check ", pShader, gcvTRUE);
    }

    return errCode;
}

DEF_SH_NECESSITY_CHECK(vscVIR_ClampPointSize)
{
    VSC_HW_CONFIG*      pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader*         pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_Symbol*         pPointSizeSym = gcvNULL;

    /* Check for VS only. */
    if (!VIR_Shader_IsVS(pShader))
    {
        return gcvFALSE;
    }

    /* Have HW fix, skip it. */
    if (pHwCfg->hwFeatureFlags.hasPointSizeFix)
    {
        return gcvFALSE;
    }

    /* Find gl_PointSize. */
    pPointSizeSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_POINT_SIZE);
    if (pPointSizeSym)
    {
        return gcvTRUE;
    }

    return gcvFALSE;
}

DEF_QUERY_PASS_PROP(vscVIR_ClampPointSize)
{
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;

    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
}

VSC_ErrCode vscVIR_ClampPointSize(VSC_SH_PASS_WORKER* pPassWorker)
{
    VIR_DEF_USAGE_INFO* pDuInfo = pPassWorker->pDuInfo;
    VSC_HW_CONFIG*      pHwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
    VIR_Shader*         pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Symbol*         pPointSizeSym = gcvNULL;
    VIR_Instruction*    pNewInst = gcvNULL;
    VIR_Operand*        pOpnd = gcvNULL;
    VIR_Function*       pMainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_SymId           vregSymId = VIR_INVALID_ID;
    gctUINT             defIdx;
    gctBOOL             bHasDef = gcvTRUE;

    /* Find gl_PointSize. */
    pPointSizeSym = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_POINT_SIZE);

    /* Not found, just return. */
    if (pPointSizeSym == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return errCode;
    }

    errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader,
                                                VIR_Symbol_GetVregIndex(pPointSizeSym),
                                                &vregSymId);
    ON_ERROR(errCode, "Get vreg symbol.");

    /* Check if PointSize have any assignment. */
    defIdx = vscVIR_FindFirstDefIndex(pDuInfo, VIR_Symbol_GetVregIndex(pPointSizeSym));
    if (VIR_INVALID_DEF_INDEX == defIdx)
    {
        bHasDef = gcvFALSE;
    }

    /*
    ** If it have any assignment, clamp it; otherwise just assign it with 1.0f.
    */
    if (bHasDef)
    {
        /*
        ** Insert the clamp instructions:
        **      PointSize = min(max(PointSize, minPointSize), maxPointSize)
        */
        errCode = VIR_Function_AddInstruction(pMainFunc,
                                              VIR_OP_MAX,
                                              VIR_TYPE_FLOAT32,
                                              &pNewInst);
        ON_ERROR(errCode, "Insert MAX instruction.");
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, vregSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, vregSymId);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

        pOpnd = VIR_Inst_GetSource(pNewInst, 1);
        VIR_Operand_SetImmediateFloat(pOpnd, pHwCfg->minPointSize);

        errCode = VIR_Function_AddInstruction(pMainFunc,
                                              VIR_OP_MIN,
                                              VIR_TYPE_FLOAT32,
                                              &pNewInst);
        ON_ERROR(errCode, "Insert MIN instruction.");
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, vregSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, vregSymId);
        VIR_Operand_SetSwizzle(pOpnd, VIR_SWIZZLE_XXXX);

        pOpnd = VIR_Inst_GetSource(pNewInst, 1);
        VIR_Operand_SetImmediateFloat(pOpnd, pHwCfg->maxPointSize);
    }
    else
    {
        errCode = VIR_Function_AddInstruction(pMainFunc,
                                              VIR_OP_MOV,
                                              VIR_TYPE_FLOAT32,
                                              &pNewInst);
        ON_ERROR(errCode, "Insert MOV instruction.");
        pOpnd = VIR_Inst_GetDest(pNewInst);
        VIR_Operand_SetSymbol(pOpnd, pMainFunc, vregSymId);
        VIR_Operand_SetEnable(pOpnd, VIR_ENABLE_X);

        pOpnd = VIR_Inst_GetSource(pNewInst, 0);
        VIR_Operand_SetImmediateFloat(pOpnd, 1.0f);
    }

OnError:
    return errCode;
}

/*-------------------------------------------Cut down workGroupSize-------------------------------------------*/
typedef struct VSC_CUTDOWN_WORKGROUPSIZE
{
    VSC_MM*             pMM;
    VSC_HW_CONFIG*      pHwCfg;
    VIR_Shader*         pShader;
    gctUINT16           activeDimension;
    gctUINT16           activeFactor;
    VSC_HASH_TABLE**    ppTempSet;
    VSC_HASH_TABLE**    ppLabelSet;
    VSC_HASH_TABLE**    ppJmpSet;
    /* We don't need to duplication the initialized instructions. */
    VIR_Instruction*    pInitializationStartInst;
    VIR_Instruction*    pInitializationEndInst;

    /* We need a loop to check that in/out flow. */
    VIR_LoopOpts        loopOpts;

    /* Multiple threads use the same conditional branch. */
    VSC_HASH_TABLE*     pSameJmpBBSet;

} VSC_CutDownWGS;

static void
_vscVIR_CutDownWorkGroupSize(
    IN VSC_CutDownWGS*  pContext
    )
{
    VIR_Shader*         pShader = pContext->pShader;
    gctUINT16           activeFactor = pContext->activeFactor;

    if (VIR_Shader_IsGlCompute(pShader))
    {
        pShader->shaderLayout.compute.workGroupSize[pContext->activeDimension] /= activeFactor;
    }
    else
    {
        if (!VIR_Shader_IsWorkGroupSizeAdjusted(pShader) && pShader->shaderLayout.compute.workGroupSize[0] != 0)
        {
            pShader->shaderLayout.compute.workGroupSize[pContext->activeDimension] /= activeFactor;
        }
        else
        {
            VIR_Shader_SetAdjustedWorkGroupSize(pShader, VIR_Shader_GetAdjustedWorkGroupSize(pShader) / activeFactor);
        }
    }

    /* Save the factor. */
    VIR_Shader_SetWorkGroupSizeFactor(pShader, pContext->activeDimension, activeFactor);
}

static void
_vscVIR_InitializeCutDownWGS(
    IN VSC_CutDownWGS*  pContext,
    IN VIR_DEF_USAGE_INFO* pDuInfo,
    IN VSC_MM*          pMM,
    IN VSC_HW_CONFIG*   pHwCfg,
    IN VIR_Shader*      pShader,
    IN VSC_OPTN_LoopOptsOptions *pLoopOptsOptions
    )
{
    gctUINT             i;
    gctBOOL             bCheckFactor4 = (pHwCfg->maxCoreCount == 1);

    gcoOS_ZeroMemory(pContext, sizeof(VSC_CutDownWGS));

    pContext->pMM = pMM;
    pContext->pHwCfg = pHwCfg;
    pContext->pShader = pShader;

    /* Check OCL and CS. */
    if (pShader->shaderLayout.compute.workGroupSize[0] == 0)
    {
        gctUINT workGroupSize = VIR_Shader_GetWorkGroupSize(pShader);

        if (bCheckFactor4 && workGroupSize % 4 == 0)
        {
            pContext->activeFactor = 4;
        }
        else
        {
            pContext->activeFactor = 2;
        }
        pContext->activeDimension = 0;
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            if (bCheckFactor4 && pShader->shaderLayout.compute.workGroupSize[i] % 4 == 0)
            {
                pContext->activeFactor = 4;
                pContext->activeDimension = (gctUINT16)i;
                break;
            }
            else if (pShader->shaderLayout.compute.workGroupSize[i] % 2 == 0)
            {
                pContext->activeFactor = 2;
                pContext->activeDimension = (gctUINT16)i;
                break;
            }
        }
    }

    gcmASSERT(pContext->activeFactor != 0);

    pContext->ppTempSet = (VSC_HASH_TABLE**)vscMM_Alloc(pContext->pMM, pContext->activeFactor * sizeof(VSC_HASH_TABLE*));
    pContext->ppLabelSet = (VSC_HASH_TABLE**)vscMM_Alloc(pContext->pMM, pContext->activeFactor * sizeof(VSC_HASH_TABLE*));
    pContext->ppJmpSet = (VSC_HASH_TABLE**)vscMM_Alloc(pContext->pMM, pContext->activeFactor * sizeof(VSC_HASH_TABLE*));

    for (i = 0; i < pContext->activeFactor; i++)
    {
        pContext->ppTempSet[i] = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM,
                                                                 vscHFUNC_Default,
                                                                 vscHKCMP_Default,
                                                                 64);
        pContext->ppLabelSet[i] = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM,
                                                                 vscHFUNC_Default,
                                                                 vscHKCMP_Default,
                                                                 64);
        pContext->ppJmpSet[i] = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM,
                                                                 vscHFUNC_Default,
                                                                 vscHKCMP_Default,
                                                                 64);
    }

    pContext->pInitializationStartInst = gcvNULL;
    pContext->pInitializationEndInst = gcvNULL;

    /* Initialize the loopOpts. */
    VIR_LoopOpts_Init(&pContext->loopOpts,
                      pDuInfo,
                      pShader,
                      VIR_Shader_GetMainFunction(pShader),
                      pLoopOptsOptions,
                      VIR_Shader_GetDumper(pShader),
                      pMM,
                      pHwCfg);

    pContext->pSameJmpBBSet = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM, vscHFUNC_Default, vscHKCMP_Default, 4);
}

static void
_vscVIR_FinalizeCutDownWGS(
    IN VSC_CutDownWGS*  pContext
    )
{
    gctUINT             i;

    if (pContext->ppTempSet)
    {
        for (i = 0; i < pContext->activeFactor; i++)
        {
            vscHTBL_Destroy(pContext->ppTempSet[i]);
        }
        vscMM_Free(pContext->pMM, pContext->ppTempSet);
    }

    if (pContext->ppLabelSet)
    {
        for (i = 0; i < pContext->activeFactor; i++)
        {
            /* destroy the hash table and free memory */
            vscHTBL_Destroy(pContext->ppLabelSet[i]);
        }
        vscMM_Free(pContext->pMM, pContext->ppLabelSet);
    }

    if (pContext->ppJmpSet)
    {
        for (i = 0; i < pContext->activeFactor; i++)
        {
            /* destroy the hash table and free memory */
            vscHTBL_Destroy(pContext->ppJmpSet[i]);
        }
        vscMM_Free(pContext->pMM, pContext->ppJmpSet);
    }

    /* Finalize the loopOpts. */
    VIR_LoopOpts_Final(&pContext->loopOpts);
    vscHTBL_Destroy(pContext->pSameJmpBBSet);
}

static VSC_ErrCode
_vscVIR_MoveJmpOutOfBB(
    IN VIR_Function*    pFunc,
    IN VIR_BB*          pBB
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* pCfg = VIR_Function_GetCFG(pFunc);
    VIR_BB*             pNewBB = gcvNULL;
    VSC_ADJACENT_LIST_ITERATOR  prevEdgeIter;
    VIR_CFG_EDGE*       pPrevEdge;
    VIR_Instruction*    pIterInst = gcvNULL;
    VIR_Instruction*    pMoveInst = gcvNULL;

    /* Create a new BB to hold the left instructions. */
    errCode = VIR_BB_InsertBBBefore(pBB,
                                    VIR_OP_NOP,
                                    &pNewBB);
    ON_ERROR(errCode, "Insert a new BB.");
    BB_FLAGS_SET(pNewBB, BB_FLAGS_GET(pBB));
    BB_FLAGS_SET(pBB, VIR_BBFLAG_NONE);

    /* Move the instructions. */
    pIterInst = BB_GET_END_INST(pNewBB);
    pMoveInst = VIR_Inst_GetPrev(BB_GET_END_INST(pBB));
    while (gcvTRUE)
    {
        errCode = VIR_Function_MoveInstructionBefore(pFunc,
                                                     pIterInst,
                                                     pMoveInst);
        ON_ERROR(errCode, "Move a instruction");

        if (BB_GET_END_INST(pBB) == BB_GET_START_INST(pBB))
        {
            break;
        }

        pIterInst = VIR_Inst_GetPrev(pIterInst);
        pMoveInst = VIR_Inst_GetPrev(BB_GET_END_INST(pBB));
    };

    /* Update the edges. */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&prevEdgeIter, &pBB->dgNode.predList);
    pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&prevEdgeIter);
    for (; pPrevEdge != gcvNULL; pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&prevEdgeIter))
    {
        errCode = vscVIR_AddEdgeToCFG(pCfg, CFG_EDGE_GET_TO_BB(pPrevEdge), pNewBB, CFG_EDGE_GET_TYPE(pPrevEdge));
        ON_ERROR(errCode, "Add a new edge.");
    }

    VSC_ADJACENT_LIST_ITERATOR_INIT(&prevEdgeIter, &pNewBB->dgNode.predList);
    pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&prevEdgeIter);
    for (; pPrevEdge != gcvNULL; pPrevEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&prevEdgeIter))
    {
        errCode = vscVIR_RemoveEdgeFromCFG(pCfg, CFG_EDGE_GET_TO_BB(pPrevEdge), pBB);
        ON_ERROR(errCode, "Remove a old edge.");
    }

    errCode = vscVIR_AddEdgeToCFG(pCfg, pNewBB, pBB, VIR_CFG_EDGE_TYPE_ALWAYS);
    ON_ERROR(errCode, "Add a new edge.");

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_DetectSingleLoopInfo(
    IN VSC_CutDownWGS*  pContext,
    IN VIR_LoopInfo*    pLoopInfo,
    IN VSC_HASH_TABLE*  pSameJmpBBSet,
    IN VSC_HASH_TABLE*  pLoopInfoWorkingSet,
    IN VSC_HASH_TABLE*  pBBWorkingSet
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_LoopInfo_BBIterator bbIter = {gcvNULL, 0, gcvNULL, 0, gcvNULL};
    VIR_BB*             pWorkingBB = gcvNULL;
    VIR_BB*             pLoopHeadBB = VIR_LoopInfo_GetLoopHead(pLoopInfo);
    VIR_BB*             pLoopEndBB = VIR_LoopInfo_GetLoopEnd(pLoopInfo);
    VIR_BB*             pSuccBBOfLoopEnd = gcvNULL;
    VIR_Instruction*    pInst = gcvNULL;
    gctBOOL             bHasBarrier = gcvFALSE;
    VSC_ADJACENT_LIST_ITERATOR  edgeIter, edgeIter2;
    VIR_CFG_EDGE*       pEdge;
    VIR_CFG_EDGE*       pEdge2;
    VIR_Function*       pFunc = BB_GET_FUNC(pLoopHeadBB);
    gctBOOL             bHasInitJmp = gcvFALSE;

    /* Skip the processed loop info. */
    if (vscHTBL_DirectTestAndGet(pLoopInfoWorkingSet, (void *)pLoopInfo, gcvNULL))
    {
        return errCode;
    }

    /* Check child loop first. */
    if(VIR_LoopInfo_GetChildLoopCount(pLoopInfo))
    {
        VSC_UL_ITERATOR         iter;
        VSC_UNI_LIST_NODE_EXT*  pNode;

        vscULIterator_Init(&iter, VIR_LoopInfo_GetChildLoopSet(pLoopInfo));
        for (pNode = CAST_ULN_2_ULEN(vscULIterator_First(&iter));
             pNode != gcvNULL;
             pNode = CAST_ULN_2_ULEN(vscULIterator_Next(&iter)))
        {
            VIR_LoopInfo* pChildLoopInfo = (VIR_LoopInfo*)vscULNDEXT_GetContainedUserData(pNode);

            _vscVIR_DetectSingleLoopInfo(pContext, pChildLoopInfo, pSameJmpBBSet, pLoopInfoWorkingSet, pBBWorkingSet);
        }
    }

    /* Get the other successful BB of the loop end. */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pLoopEndBB->dgNode.succList);
    for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
         pEdge != gcvNULL;
         pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        pWorkingBB = CFG_EDGE_GET_TO_BB(pEdge);
        if (pWorkingBB != pLoopHeadBB)
        {
            pSuccBBOfLoopEnd = pWorkingBB;
            break;
        }
    }
    gcmASSERT(pSuccBBOfLoopEnd);

    /* Check the left BB. */
    VIR_LoopInfo_BBIterator_Init(&bbIter, pLoopInfo, VIR_LoopInfo_BBIterator_Type_Arbitrary);
    for (pWorkingBB = VIR_LoopInfo_BBIterator_First(&bbIter);
         pWorkingBB != gcvNULL;
         pWorkingBB = VIR_LoopInfo_BBIterator_Next(&bbIter))
    {
        /* Skip the processed BB, they are handled in the child loop. */
        if (vscHTBL_DirectTestAndGet(pBBWorkingSet, (void *)pWorkingBB, gcvNULL))
        {
            continue;
        }

        /* We need to go through all BBs. */
        if (!bHasBarrier)
        {
            pInst = BB_GET_START_INST(pWorkingBB);
            while (gcvTRUE)
            {
                if (VIR_Inst_IsHWBarrier(pInst))
                {
                    bHasBarrier = gcvTRUE;
                    break;
                }

                if (pInst == BB_GET_END_INST(pWorkingBB))
                {
                    break;
                }
                pInst = VIR_Inst_GetNext(pInst);
            };
        }

        vscHTBL_DirectSet(pBBWorkingSet, (void *)pWorkingBB, gcvNULL);
    }

    vscHTBL_DirectSet(pLoopInfoWorkingSet, (void *)pLoopInfo, gcvNULL);

    if (!bHasBarrier)
    {
        return errCode;
    }

    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pLoopHeadBB->dgNode.predList);
    for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
         pEdge != gcvNULL;
         pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        pWorkingBB = CFG_EDGE_GET_TO_BB(pEdge);

        /* The initialize jmp should have the same successful list as the loop end. */
        if (pWorkingBB != pLoopEndBB)
        {
            gctBOOL     bValid = gcvFALSE;

            VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter2, &pWorkingBB->dgNode.succList);
            for (pEdge2 = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter2);
                 pEdge2 != gcvNULL;
                 pEdge2 = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter2))
            {
                if (CFG_EDGE_GET_TO_BB(pEdge2) == pSuccBBOfLoopEnd)
                {
                    bValid = gcvTRUE;
                    break;
                }
            }

            if (!bValid)
            {
                continue;
            }
            bHasInitJmp = gcvTRUE;
        }

        /* Move the other instructions to another BB. */
        if (VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(BB_GET_END_INST(pWorkingBB))))
        {
            errCode = _vscVIR_MoveJmpOutOfBB(VIR_Shader_GetMainFunction(pContext->pShader),
                                             pWorkingBB);
            ON_ERROR(errCode, "Move JMP out of BB.");
        }

        vscHTBL_DirectSet(pSameJmpBBSet, (void *)pWorkingBB, gcvNULL);
    }

    /* If there is no initialized JMP, we need to create a fake one. */
    if (!bHasInitJmp && VIR_Inst_GetOpcode(BB_GET_START_INST(pLoopHeadBB)) == VIR_OP_LABEL)
    {
        VIR_BB*             pNewBB = gcvNULL;
        VIR_Instruction*    pNewLabelInst = gcvNULL;
        VIR_Instruction*    pJmpInst = gcvNULL;
        VIR_Label*          pOrigLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(BB_GET_START_INST(pLoopHeadBB)));
        VIR_Label*          pLabel = gcvNULL;
        VIR_Link*           pLink = gcvNULL;
        VIR_Label*          pNewLabel = gcvNULL;
        gctBOOL             bMatched = gcvFALSE;

        errCode = VIR_BB_InsertBBAfter(VIR_Inst_GetBasicBlock(VIR_Inst_GetPrev(BB_GET_START_INST(pLoopHeadBB))),
                                       VIR_OP_NOP,
                                       &pNewBB);
        ON_ERROR(errCode, "Insert a new BB.");

        errCode = VIR_Function_AddCopiedInstructionAfter(pFunc,
                                                         BB_GET_START_INST(pLoopHeadBB),
                                                         BB_GET_START_INST(pNewBB),
                                                         gcvTRUE,
                                                         &pNewLabelInst);
        ON_ERROR(errCode, "Insert a new instruction.");
        pNewLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pNewLabelInst));

        /* Update the label. */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pLoopHeadBB->dgNode.predList);
        for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
             pEdge != gcvNULL;
             pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            pWorkingBB = CFG_EDGE_GET_TO_BB(pEdge);

            if (pWorkingBB == pLoopEndBB)
            {
                continue;
            }

            pJmpInst = BB_GET_END_INST(pWorkingBB);

            if (!VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pJmpInst)) &&
                pEdge->type == VIR_CFG_EDGE_TYPE_ALWAYS)
            {
                vscHTBL_DirectSet(pSameJmpBBSet, (void *)pNewBB, gcvNULL);
                continue;
            }

            if (!VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pJmpInst)))
            {
                continue;
            }

            pLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pJmpInst));
            if (pLabel != pOrigLabel)
            {
                continue;
            }

            VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(pLabel), (gctUINTPTR_T)pJmpInst);
            VIR_Operand_SetLabel(VIR_Inst_GetDest(pJmpInst), pNewLabel);
            VIR_Function_NewLink(pFunc, &pLink);
            VIR_Link_SetReference(pLink, (gctUINTPTR_T)pJmpInst);
            VIR_Link_AddLink(VIR_Label_GetReferenceAddr(pNewLabel), pLink);
            bMatched = gcvTRUE;
        }

        if (bMatched)
        {
            vscHTBL_DirectSet(pSameJmpBBSet, (void *)pLoopHeadBB, gcvNULL);
        }
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_AnalyzeMultiPathWithBarrier(
    IN VSC_CutDownWGS*  pContext
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pContext->pShader;
    VIR_Function*       pFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_CONTROL_FLOW_GRAPH* pCfg = VIR_Function_GetCFG(pFunc);
    VIR_Instruction*    pInst = gcvNULL;
    VIR_Label*          pLabel = gcvNULL;
    VIR_Instruction*    pLabelInst = gcvNULL;
    VIR_Label*          pNewLabel = gcvNULL;
    VIR_Instruction*    pNewLabelInst = gcvNULL;
    VIR_Link*           pLink = gcvNULL;
    VIR_BB*             pTargetBB = gcvNULL;
    VIR_BB*             pJmpBB = gcvNULL;
    VIR_BB*             pNewReturnBB = gcvNULL;
    VIR_BB*             pExitBB = gcvNULL;
    VSC_ADJACENT_LIST_ITERATOR  edgeIter;
    VIR_CFG_EDGE*       pEdge;
    VIR_CFG_EDGE_TYPE   edgeType = VIR_CFG_EDGE_TYPE_ALWAYS;

    /* A temp WAR for Vulkan Sascha, will add a general path later. */
    if (!(VIR_Shader_IsVulkan(pShader) && VIR_Shader_IsGlCompute(pShader)))
    {
        return errCode;
    }

    for (pInst = VIR_Function_GetInstStart(pFunc); pInst != gcvNULL; pInst = VIR_Inst_GetNext(pInst))
    {
        if (!VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pInst)))
        {
            continue;
        }

        pLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pInst));
        pLabelInst = VIR_Label_GetDefInst(pLabel);
        pJmpBB = VIR_Inst_GetBasicBlock(pInst);
        pTargetBB = VIR_Inst_GetBasicBlock(pLabelInst);

        if (BB_GET_LENGTH(pTargetBB) != 1)
        {
            continue;
        }

        /* Update the JMP BB. */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pJmpBB->dgNode.succList);
        for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
             pEdge != gcvNULL;
             pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            if (CFG_EDGE_GET_TO_BB(pEdge) == pTargetBB)
            {
                edgeType = pEdge->type;
                AJLST_REMOVE_EDGE(&pJmpBB->dgNode.succList, pEdge);
                break;
            }
        }

        /* Update the TARGET BB. */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pTargetBB->dgNode.predList);
        for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
             pEdge != gcvNULL;
             pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            if (CFG_EDGE_GET_TO_BB(pEdge) == pJmpBB)
            {
                AJLST_REMOVE_EDGE(&pTargetBB->dgNode.predList, pEdge);
                break;
            }
        }
        /* Get the EXIT BB. */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pTargetBB->dgNode.succList);
        for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
             pEdge != gcvNULL;
             pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            if (CFG_EDGE_GET_TO_BB(pEdge) == CFG_GET_EXIT_BB(pCfg))
            {
                pExitBB = CFG_GET_EXIT_BB(pCfg);
                AJLST_REMOVE_EDGE(&pTargetBB->dgNode.succList, pEdge);
                break;
            }
        }

        if (pExitBB)
        {
            VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pExitBB->dgNode.predList);
            for (pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
                 pEdge != gcvNULL;
                 pEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
            {
                if (CFG_EDGE_GET_TO_BB(pEdge) == pTargetBB)
                {
                    AJLST_REMOVE_EDGE(&pExitBB->dgNode.predList, pEdge);
                    break;
                }
            }
        }
        else
        {
            gcmASSERT(gcvFALSE);
        }

        /* Insert a new BB after the RETURN BB and replace it. */
        errCode = VIR_BB_InsertBBAfter(pTargetBB,
                                       VIR_OP_LABEL,
                                       &pNewReturnBB);
        ON_ERROR(errCode, "Insert a new BB.");

        /* Update the edge. */
        errCode = vscVIR_AddEdgeToCFG(pCfg, pTargetBB, pNewReturnBB, VIR_CFG_EDGE_TYPE_ALWAYS);
        ON_ERROR(errCode, "Add a new edge. ");

        errCode = vscVIR_AddEdgeToCFG(pCfg, pJmpBB, pNewReturnBB, edgeType);
        ON_ERROR(errCode, "Add a new edge. ");

        errCode = vscVIR_AddEdgeToCFG(pCfg, pNewReturnBB, pExitBB, edgeType);
        ON_ERROR(errCode, "Add a new edge. ");

        /* Update the label. */
        pNewLabelInst = BB_GET_START_INST(pNewReturnBB);
        VIR_Inst_Copy(pNewLabelInst, pLabelInst, gcvFALSE);
        pNewLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pNewLabelInst));

        VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(pLabel), (gctUINTPTR_T)pInst);

        VIR_Operand_SetLabel(VIR_Inst_GetDest(pInst), pNewLabel);
        VIR_Function_NewLink(pFunc, &pLink);
        VIR_Link_SetReference(pLink, (gctUINTPTR_T)pInst);
        VIR_Link_AddLink(VIR_Label_GetReferenceAddr(pNewLabel), pLink);

        vscHTBL_DirectSet(pContext->pSameJmpBBSet, (void *)pNewReturnBB, gcvNULL);
        break;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_DetectBarrierWithinLoop(
    IN VSC_CutDownWGS*  pContext
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_HASH_TABLE*     pSameJmpBBSet = pContext->pSameJmpBBSet;
    VSC_HASH_TABLE*     pLoopInfoWorkingSet = gcvNULL;
    VSC_HASH_TABLE*     pBBWorkingSet = gcvNULL;
    VIR_LoopOpts*       pLoopOpts = &pContext->loopOpts;
    VIR_LoopInfoMgr*    pLoopInfoMgr = gcvNULL;
    VIR_LoopInfo*       pLoopInfo = gcvNULL;
    VSC_UL_ITERATOR     iter;

    pLoopInfoWorkingSet = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM, vscHFUNC_Default, vscHKCMP_Default, 16);
    pBBWorkingSet = (VSC_HASH_TABLE*)vscHTBL_Create(pContext->pMM, vscHFUNC_Default, vscHKCMP_Default, 16);

    /* Initialize and detect all LOOPs. */
    VIR_LoopOpts_NewLoopInfoMgr(pLoopOpts);
    if (VIR_LoopOpts_DetectNaturalLoops(pLoopOpts))
    {
        /* Compute the loop body and some other information, we need to call these after add a new loopInfo. */
        VIR_LoopOpts_ComputeLoopBodies(pLoopOpts);
        VIR_LoopOpts_ComputeLoopTree(pLoopOpts);
        VIR_LoopOpts_IdentifyBreakContinues(pLoopOpts);

        /*
        ** Find all loops with BARRIER, please note that if a child loop has BARRIER but the parent loop has no BARRIR,
        ** only record the child loop.
        */
        pLoopInfoMgr = VIR_LoopOpts_GetLoopInfoMgr(pLoopOpts);
        vscULIterator_Init(&iter, VIR_LoopInfoMgr_GetLoopInfos(pLoopInfoMgr));
        for (pLoopInfo = (VIR_LoopInfo*)vscULIterator_First(&iter);
             pLoopInfo != gcvNULL;
             pLoopInfo = (VIR_LoopInfo*)vscULIterator_Next(&iter))
        {
            errCode = _vscVIR_DetectSingleLoopInfo(pContext, pLoopInfo, pSameJmpBBSet, pLoopInfoWorkingSet, pBBWorkingSet);
            ON_ERROR(errCode, "Detect single loop info.");
        }
    }

OnError:
    if (pLoopInfoWorkingSet)
    {
        vscHTBL_Destroy(pLoopInfoWorkingSet);
    }

    if (pBBWorkingSet)
    {
        vscHTBL_Destroy(pBBWorkingSet);
    }

    return errCode;
}

static VSC_ErrCode
_vscVIR_MoveBarrierOutOfBB(
    IN VSC_CutDownWGS*  pContext
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pContext->pShader;
    VIR_Function*       pMainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_CONTROL_FLOW_GRAPH* pCfg = VIR_Function_GetCFG(pMainFunc);
    VIR_Instruction*    pBarrierInst = gcvNULL;
    VIR_Instruction*    pInst = gcvNULL;
    VIR_Instruction*    pTempInst = gcvNULL;
    VIR_Instruction*    pNewInst = gcvNULL;
    VIR_BASIC_BLOCK*    pBarrierBB = gcvNULL;
    VIR_BASIC_BLOCK*    pLeftBB = gcvNULL;
    VIR_BASIC_BLOCK*    pCurrentBB = gcvNULL;

    for (pInst = VIR_Function_GetInstStart(pMainFunc); pInst != gcvNULL; pInst = VIR_Inst_GetNext(pInst))
    {
        VSC_ADJACENT_LIST_ITERATOR   succEdgeIter;
        VIR_CFG_EDGE*                pSuccEdge;

        if (!VIR_Inst_IsHWBarrier(pInst))
        {
            continue;
        }

        pCurrentBB = VIR_Inst_GetBasicBlock(pInst);
        pBarrierInst = pInst;

        /* Merge the consecutive BARRIERs to one BARRIER. */
        for (pInst = VIR_Inst_GetNext(pInst); pInst != gcvNULL; pInst = VIR_Inst_GetNext(pInst))
        {
            if (VIR_Inst_GetOpcode(pInst) == VIR_OP_NOP)
            {
                continue;
            }

            if (!VIR_Inst_IsHWBarrier(pInst))
            {
                break;
            }
            else
            {
                VIR_Function_ChangeInstToNop(pMainFunc, pInst);
            }
        }

        /* Create a new BB to hold the BARRIER. */
        errCode = VIR_BB_InsertBBAfter(pCurrentBB,
                                       VIR_OP_NOP,
                                       &pBarrierBB);
        ON_ERROR(errCode, "Insert a new BB.");

        /* Create a new BB to hold the left instructions. */
        errCode = VIR_BB_InsertBBAfter(pBarrierBB,
                                       VIR_OP_NOP,
                                       &pLeftBB);
        ON_ERROR(errCode, "Insert a new BB.");

        /* Update the edges. */
        errCode = vscVIR_AddEdgeToCFG(pCfg, pCurrentBB, pBarrierBB, VIR_CFG_EDGE_TYPE_ALWAYS);
        ON_ERROR(errCode, "Add a new edge.");

        errCode = vscVIR_AddEdgeToCFG(pCfg, pBarrierBB, pLeftBB, VIR_CFG_EDGE_TYPE_ALWAYS);
        ON_ERROR(errCode, "Add a new edge.");

        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pCurrentBB->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            if (CFG_EDGE_GET_TO_BB(pSuccEdge) == pBarrierBB)
            {
                continue;
            }
            errCode = vscVIR_AddEdgeToCFG(pCfg, pLeftBB, CFG_EDGE_GET_TO_BB(pSuccEdge), CFG_EDGE_GET_TYPE(pSuccEdge));
            ON_ERROR(errCode, "Add a new edge.");
        }

        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pLeftBB->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            errCode = vscVIR_RemoveEdgeFromCFG(pCfg, pCurrentBB, CFG_EDGE_GET_TO_BB(pSuccEdge));
            ON_ERROR(errCode, "Remove an old edge.");
        }

        /* Update the BB flags. */
        BB_FLAGS_SET_HAS_BARRIER(pBarrierBB);
        BB_FLAGS_RESET_HAS_BARRIER(pCurrentBB);

        /* Copy the BARRIER to the BARRIER BB. */
        errCode = VIR_Function_AddCopiedInstructionAfter(pMainFunc,
                                                         pBarrierInst,
                                                         BB_GET_END_INST(pBarrierBB),
                                                         gcvTRUE,
                                                         &pNewInst);
        ON_ERROR(errCode, "Copy the BARRIER.");

        /* Copy the left instructions to the left BB. */
        pInst = VIR_Inst_GetNext(pBarrierInst);
        while (gcvTRUE)
        {
            errCode = VIR_Function_AddCopiedInstructionAfter(pMainFunc,
                                                             pInst,
                                                             BB_GET_END_INST(pLeftBB),
                                                             gcvTRUE,
                                                             &pNewInst);
            ON_ERROR(errCode, "Copy the BARRIER.");

            if (VIR_Inst_IsHWBarrier(pNewInst))
            {
                BB_FLAGS_SET_HAS_BARRIER(pLeftBB);
            }

            if (pInst == BB_GET_END_INST(pCurrentBB))
            {
                break;
            }
            pInst = VIR_Inst_GetNext(pInst);
        };

        /* Remove all left instructions. */
        pInst = pBarrierInst;
        while (gcvTRUE)
        {
            pTempInst = VIR_Inst_GetNext(pInst);

            if (pInst == BB_GET_END_INST(pCurrentBB))
            {
                errCode = VIR_Function_RemoveInstruction(pMainFunc, pInst, gcvTRUE);
                ON_ERROR(errCode, "Remove a instruction.");

                pInst = BB_GET_START_INST(pLeftBB);
                break;
            }

            errCode = VIR_Function_RemoveInstruction(pMainFunc, pInst, gcvTRUE);
            ON_ERROR(errCode, "Remove a instruction.");

            pInst = pTempInst;
        };
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_RecalculateBuiltinAttributes(
    IN VSC_CutDownWGS*  pContext
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = pContext->pShader;
    gctUINT16           activeFactor = pContext->activeFactor;
    VSC_HASH_TABLE**    ppTempSet = pContext->ppTempSet;
    gctUINT             i, j;
    VIR_Symbol*         pIdSym[2] = { gcvNULL, gcvNULL };
    VIR_Symbol*         pVirRegSym = gcvNULL;
    VIR_VirRegId        newVirRegId = VIR_INVALID_ID;
    VIR_SymId           newVirSymId = VIR_INVALID_ID;
    VIR_Symbol*         pNewVirSym = gcvNULL;
    VIR_Function*       pMainFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_Instruction*    pStartInst = VIR_Function_GetInstStart(pMainFunc);
    VIR_Instruction*    pNewInst = gcvNULL;
    VIR_Operand*        pNewOpnd = gcvNULL;
    VIR_Instruction*    pInitializationStartInst = gcvNULL;
    VIR_Instruction*    pInitializationEndInst = gcvNULL;
    VIR_Enable          activeEnable = (VIR_Enable)(VIR_ENABLE_X << pContext->activeDimension);
    VIR_Swizzle         activeSwizzle = VIR_Enable_2_Swizzle_WShift(activeEnable);

    pIdSym[0] = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_LOCAL_INVOCATION_ID);
    pIdSym[1] = VIR_Shader_FindSymbolById(pShader, VIR_SYM_VARIABLE, VIR_NAME_GLOBAL_INVOCATION_ID);

    /*
    ** Re-caculate the LocalInvocationId, GlobalInvocationId and LocalInvocationIndex:
    **
    **      LocalId_in_program0 = LocalId_in_hw * factor
    **      LocalIdin_programi = LocalId_in_hw * factor + i
    **
    **      GlobalId_in_program0 = GlobalId_in_hw * factor
    **      GlobalId_in_programi = GlobalId_in_hw * factor + i
    **
    **      LocalIndex:
    **          We don't need to re-calculate LocalIndex because we have used the original LocalId and WorkGroupSize to calcualte it before.
    **
    ** For LocalId/GlobalId:
    **    1) For thread[0]: Id_in_hw = Id_in_hw * factor, so we can reuse the instruction set for thread[0].
    **    2) For thread[1]~thread[factor-1]: use new temp registers instread of attributes to save these builtin attributes,
    **       and replace them in the instructions.
    */
    for (i = 0; i < activeFactor; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (pIdSym[j] != gcvNULL && !isSymUnused(pIdSym[j]))
            {
                /* For thread[0]: Id = Id * factor. */
                if (i == 0)
                {
                    pVirRegSym = VIR_Shader_FindSymbolByTempIndex(pShader, VIR_Symbol_GetVariableVregIndex(pIdSym[j]));
                    gcmASSERT(pVirRegSym != gcvNULL);

                    errCode = VIR_Function_AddInstructionBefore(pMainFunc,
                                                                VIR_OP_MUL,
                                                                VIR_Symbol_GetTypeId(pIdSym[j]),
                                                                pStartInst,
                                                                gcvTRUE,
                                                                &pNewInst);
                    ON_ERROR(errCode, "Add new instruction.");
                    if (pInitializationStartInst == gcvNULL)
                    {
                        pInitializationStartInst = pNewInst;
                    }
                    pInitializationEndInst = pNewInst;

                    pNewOpnd = VIR_Inst_GetDest(pNewInst);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pVirRegSym));
                    VIR_Operand_SetEnable(pNewOpnd, activeEnable);

                    pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pIdSym[j]));
                    VIR_Operand_SetSwizzle(pNewOpnd, activeSwizzle);

                    pNewOpnd = VIR_Inst_GetSource(pNewInst, 1);
                    VIR_Operand_SetImmediateUint(pNewOpnd, activeFactor);
                }
                /* For thread[1]~thread[factor-1]: Id_temp = Id + i. */
                else
                {
                    newVirRegId = VIR_Shader_NewVirRegId(pShader, 1);
                    errCode = VIR_Shader_AddSymbol(pShader,
                                                   VIR_SYM_VIRREG,
                                                   newVirRegId,
                                                   VIR_Shader_GetTypeFromId(pShader, VIR_TYPE_UINT_X3),
                                                   VIR_STORAGE_UNKNOWN,
                                                   &newVirSymId);
                    ON_ERROR(errCode, "Add new symbol.");
                    pNewVirSym = VIR_Shader_GetSymFromId(pShader, newVirSymId);

                    /* mov, newTemp, index. */
                    errCode = VIR_Function_AddInstructionBefore(pMainFunc,
                                                                VIR_OP_MOV,
                                                                VIR_Symbol_GetTypeId(pIdSym[j]),
                                                                pStartInst,
                                                                gcvTRUE,
                                                                &pNewInst);
                    ON_ERROR(errCode, "Add new instruction.");
                    pInitializationEndInst = pNewInst;

                    pNewOpnd = VIR_Inst_GetDest(pNewInst);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, newVirSymId);
                    VIR_Operand_SetEnable(pNewOpnd, VIR_ENABLE_XYZ);

                    pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pIdSym[j]));
                    VIR_Operand_SetSwizzle(pNewOpnd, VIR_SWIZZLE_XYZZ);

                    /* add, newTemp.x, newTemp.x, 1*/
                    errCode = VIR_Function_AddInstructionBefore(pMainFunc,
                                                                VIR_OP_ADD,
                                                                VIR_GetTypeComponentType(VIR_Symbol_GetTypeId(pIdSym[j])),
                                                                pStartInst,
                                                                gcvTRUE,
                                                                &pNewInst);
                    ON_ERROR(errCode, "Add new instruction.");
                    pInitializationEndInst = pNewInst;

                    pNewOpnd = VIR_Inst_GetDest(pNewInst);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, newVirSymId);
                    VIR_Operand_SetEnable(pNewOpnd, activeEnable);

                    pNewOpnd = VIR_Inst_GetSource(pNewInst, 0);
                    VIR_Operand_SetSymbol(pNewOpnd, pMainFunc, VIR_Symbol_GetIndex(pIdSym[j]));
                    VIR_Operand_SetSwizzle(pNewOpnd, activeSwizzle);

                    pNewOpnd = VIR_Inst_GetSource(pNewInst, 1);
                    VIR_Operand_SetImmediateUint(pNewOpnd, i);

                    vscHTBL_DirectSet(ppTempSet[i], pIdSym[j], (void*)pNewVirSym);
                }
            }
        }
    }

    /* Save the instruction range of initialization. */
    pContext->pInitializationStartInst = pInitializationStartInst;
    pContext->pInitializationEndInst = pInitializationEndInst;

OnError:
    return errCode;
}

static gctBOOL
_vscVIR_NeedToRemap(
    IN VSC_CutDownWGS*      pContext,
    IN VIR_Symbol*          pSymbol
    )
{
    VIR_SymbolKind          symbolKind = VIR_Symbol_GetKind(pSymbol);
    VIR_StorageClass        storageClass = VIR_Symbol_GetStorageClass(pSymbol);
    VIR_NameId              nameId = VIR_Symbol_GetName(pSymbol);

    /* Only check some builtin attributes and all temp registers. */
    if (symbolKind != VIR_SYM_VARIABLE && symbolKind != VIR_SYM_VIRREG)
    {
        return gcvFALSE;
    }

    if (storageClass == VIR_STORAGE_INPUT)
    {
        if (nameId != VIR_NAME_LOCAL_INVOCATION_ID && nameId != VIR_NAME_GLOBAL_INVOCATION_ID)
        {
            return gcvFALSE;
        }
    }

    return gcvTRUE;
}

static VSC_ErrCode
_vscVIR_RemapOperand(
    IN VSC_CutDownWGS*      pContext,
    IN VIR_Instruction*     pInst,
    IN VIR_Operand*         pOpnd,
    IN gctBOOL              bIsDest,
    IN VSC_HASH_TABLE*      pTempSet
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Shader*             pShader = pContext->pShader;
    VIR_Symbol*             pOpndSym = gcvNULL;
    VIR_Symbol*             pVarSym = gcvNULL;
    VIR_Symbol*             pRegSym = gcvNULL;
    VIR_Symbol*             pMapVarSym = gcvNULL;
    VIR_Symbol*             pMapRegSym = gcvNULL;
    VIR_Symbol*             pMapSym = gcvNULL;
    VIR_SymId               newSymId = VIR_INVALID_ID;
    gctUINT                 i, regCount = 0, indexRange = 0;
    VIR_VirRegId            regId;
    VIR_TypeId              opndTypeId = VIR_Operand_GetTypeId(pOpnd);

    /* Check symbol only. */
    if (!(VIR_Operand_isSymbol(pOpnd) || VIR_Operand_isVirReg(pOpnd)))
    {
        return errCode;
    }

    /* Check if this symbol need to be remapped. */
    pOpndSym = VIR_Operand_GetSymbol(pOpnd);
    if (!_vscVIR_NeedToRemap(pContext, pOpndSym))
    {
        return errCode;
    }

    /* Check if this symbol is mapped before, if not, create a new one. */
    if (!vscHTBL_DirectTestAndGet(pTempSet, (void *)(pOpndSym), (void **)(&pMapSym)))
    {
        /* Create a new symbol. */
        if (VIR_Symbol_isVariable(pOpndSym))
        {
            pVarSym = pOpndSym;
        }
        else
        {
            pVarSym = VIR_Symbol_GetVregVariable(pOpndSym);
            pRegSym = pOpndSym;
        }

        /* If this is a variable, add this variable and the related temp register. */
        if (pVarSym != gcvNULL)
        {
            /* Add this variable. */
            errCode = VIR_Shader_DuplicateVariableFromSymbol(pShader, pVarSym, &newSymId);
            ON_ERROR(errCode, "Duplicate symbol.");

            /* Save the variable mapping. */
            pMapVarSym = VIR_Shader_GetSymFromId(pShader, newSymId);
            vscHTBL_DirectSet(pTempSet, (void *)pVarSym, (void *)pMapVarSym);

            /* Add the related temp register. */
            regCount = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pVarSym), -1);
            regId = VIR_Shader_NewVirRegId(pShader, regCount);
            indexRange = regId + regCount;
            VIR_Symbol_SetVariableVregIndex(pMapVarSym, regId);
            VIR_Symbol_SetIndexRange(pMapVarSym, indexRange);

            for (i = 0; i < regCount; i++)
            {
                errCode = VIR_Shader_AddSymbol(pShader,
                                               VIR_SYM_VIRREG,
                                               regId + i,
                                               VIR_Type_GetRegIndexType(pShader, VIR_Symbol_GetType(pVarSym), regId),
                                               VIR_STORAGE_UNKNOWN,
                                               &newSymId);
                ON_ERROR(errCode, "Add symbol failed.");

                pMapRegSym = VIR_Shader_GetSymFromId(pShader, newSymId);
                gcmASSERT(pMapRegSym);

                VIR_Symbol_SetVregVariable(pMapRegSym, pMapVarSym);
                VIR_Symbol_SetPrecision(pMapRegSym, VIR_Symbol_GetPrecision(pMapVarSym));
                VIR_Symbol_SetIndexRange(pMapRegSym, indexRange);

                /* Save the temp register mapping. */
                errCode = VIR_Shader_GetVirRegSymByVirRegId(pShader, VIR_Symbol_GetVregIndex(pVarSym) + i, &newSymId);
                ON_ERROR(errCode, "Get vir reg symbol.");

                pRegSym = VIR_Shader_GetSymFromId(pShader, newSymId);

                vscHTBL_DirectSet(pTempSet, (void *)pRegSym, (void *)pMapRegSym);
            }

            /* Set the correct mapping symbol. */
            if (VIR_Symbol_isVariable(pOpndSym))
            {
                pMapSym = pMapVarSym;
            }
            else
            {
                if (!vscHTBL_DirectTestAndGet(pTempSet, (void *)pOpndSym, (void **)(&pMapSym)))
                {
                    gcmASSERT(gcvFALSE);
                }
            }
        }
        else
        {
            gcmASSERT(pRegSym);

            regCount = VIR_Type_GetVirRegCount(pShader, VIR_Symbol_GetType(pRegSym), -1);
            regId = VIR_Shader_NewVirRegId(pShader, regCount);
            indexRange = regId + regCount;

            errCode = VIR_Shader_AddSymbol(pShader,
                                           VIR_SYM_VIRREG,
                                           regId,
                                           VIR_Symbol_GetType(pRegSym),
                                           VIR_STORAGE_UNKNOWN,
                                           &newSymId);
            ON_ERROR(errCode, "Add symbol failed.");

            pMapRegSym = VIR_Shader_GetSymFromId(pShader, newSymId);
            gcmASSERT(pMapRegSym);

            VIR_Symbol_SetPrecision(pMapRegSym, VIR_Symbol_GetPrecision(pRegSym));
            VIR_Symbol_SetIndexRange(pMapRegSym, indexRange);

            /* Save the temp register mapping. */
            vscHTBL_DirectSet(pTempSet, (void *)pRegSym, (void *)pMapRegSym);

            pMapSym = pMapRegSym;
        }
    }

    /* Update the operand. */
    VIR_Operand_SetSymbol(pOpnd, VIR_Inst_GetFunction(pInst), VIR_Symbol_GetIndex(pMapSym));
    VIR_Operand_SetTypeId(pOpnd, opndTypeId);

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_DuplicateInstruction(
    IN VSC_CutDownWGS*      pContext,
    IN gctBOOL              bSameJmpBB,
    IN gctBOOL              bLastInst,
    IN VIR_Instruction*     pCopyFromInst,
    IN VIR_Instruction*     pCurrentInstPos,
    IN VSC_HASH_TABLE*      pTempSet,
    IN VSC_HASH_TABLE*      pLabelSet,
    IN VSC_HASH_TABLE*      pJmpSet,
    IN VIR_Instruction**    ppNewInst
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_Function*           pFunc = VIR_Inst_GetFunction(pCopyFromInst);
    VIR_Instruction*        pNewInst = gcvNULL;
    VIR_OpCode              opCode = VIR_Inst_GetOpcode(pCopyFromInst);
    gctUINT                 i;

    errCode = VIR_Function_AddCopiedInstructionAfter(pFunc,
                                                     pCopyFromInst,
                                                     pCurrentInstPos,
                                                     gcvTRUE,
                                                     &pNewInst);
    ON_ERROR(errCode, "copy instruction.");

    /* Save the label mapping. */
    if (opCode == VIR_OP_LABEL)
    {
        VIR_Label*  pOrigLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pCopyFromInst));

        vscHTBL_DirectSet(pLabelSet,
                          VIR_Function_GetSymFromId(pFunc, pOrigLabel->sym),
                          (void*)VIR_Operand_GetLabel(VIR_Inst_GetDest(pNewInst)));
    }
    else if (VIR_OPCODE_isBranch(opCode))
    {
        if (!(bSameJmpBB && bLastInst))
        {
            vscHTBL_DirectSet(pJmpSet,
                              (void *)pNewInst,
                              gcvNULL);
        }
    }

    /* Remap the temp register. */
    if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(pNewInst)))
    {
        errCode = _vscVIR_RemapOperand(pContext,
                                       pNewInst,
                                       VIR_Inst_GetDest(pNewInst),
                                       gcvTRUE,
                                       pTempSet);
        ON_ERROR(errCode, "Remap operand.");
    }

    for (i = 0; i < VIR_Inst_GetSrcNum(pNewInst); i++)
    {
        errCode = _vscVIR_RemapOperand(pContext,
                                       pNewInst,
                                       VIR_Inst_GetSource(pNewInst, i),
                                       gcvFALSE,
                                       pTempSet);
        ON_ERROR(errCode, "Remap operand.");
    }

    if (ppNewInst)
    {
        *ppNewInst = pNewInst;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_DuplicateBBs(
    IN VSC_CutDownWGS*      pContext,
    IN VIR_Function*        pFunc,
    IN VIR_BASIC_BLOCK*     pBB,
    IN VIR_IdList*          pWorkingBBList
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VSC_MM*                 pMM = pContext->pMM;
    VSC_HASH_TABLE*         pSameJmpBBSet = pContext->pSameJmpBBSet;
    gctUINT                 i;
    gctUINT16               factorIndex, activeFactor = pContext->activeFactor;
    gctBOOL                 bHasBarrier = BB_FLAGS_HAS_BARRIER(pBB);
    VIR_BASIC_BLOCK*        pCurrentWorkingBB = gcvNULL;
    VIR_Instruction*        pCurrentWorkingInst = gcvNULL;
    VIR_Instruction*        pOrigBBStartInst = BB_GET_START_INST(pBB);
    VIR_Instruction*        pOrigBBEndInst = BB_GET_END_INST(pBB);
    VIR_Instruction**       ppNewStartInst = gcvNULL;
    VIR_Instruction**       ppNewEndInst = gcvNULL;

    if (pWorkingBBList->count == 0)
    {
        return errCode;
    }

    /* Allocate the start/end instruction. */
    ppNewStartInst = (VIR_Instruction **)vscMM_Alloc(pMM, (activeFactor - 1) * sizeof(VIR_Instruction*));
    ppNewEndInst = (VIR_Instruction **)vscMM_Alloc(pMM, (activeFactor - 1) * sizeof(VIR_Instruction*));

    /* Duplicate "factor-1" copies: Copy the instructions before BARRIER. */
    for (factorIndex = 0; factorIndex < activeFactor - 1; factorIndex++)
    {
        VSC_HASH_TABLE*     pTempSet = pContext->ppTempSet[factorIndex + 1];
        VSC_HASH_TABLE*     pLabelSet = pContext->ppLabelSet[factorIndex + 1];
        VSC_HASH_TABLE*     pJmpSet = pContext->ppJmpSet[factorIndex + 1];
        VIR_Instruction*    pNewStartInst = gcvNULL;
        VIR_Instruction*    pNewEndInst = gcvNULL;
        VIR_Instruction*    pNewInst = gcvNULL;
        gctBOOL             bSameJmpBB = gcvFALSE, bLastInst = gcvFALSE;

        /* Add a NOP in the beginning, just easy to check. */
        if (bHasBarrier)
        {
            errCode = VIR_Function_AddInstructionBefore(pFunc,
                                                        VIR_OP_NOP,
                                                        VIR_TYPE_UNKNOWN,
                                                        pOrigBBStartInst,
                                                        gcvTRUE,
                                                        &pNewInst);
        }
        else
        {
            errCode = VIR_Function_AddInstructionAfter(pFunc,
                                                       VIR_OP_NOP,
                                                       VIR_TYPE_UNKNOWN,
                                                       pOrigBBEndInst,
                                                       gcvTRUE,
                                                       &pNewInst);
        }
        ON_ERROR(errCode, "Add a NOP.");
        pNewStartInst = pNewEndInst = pNewInst;

        /* Start to copy the instructions. */
        for (i = 0; i < VIR_IdList_Count(pWorkingBBList); i++)
        {
            pCurrentWorkingBB = CFG_GET_BB_BY_ID(VIR_Function_GetCFG(pFunc), VIR_IdList_GetId(pWorkingBBList, i));
            pCurrentWorkingInst = BB_GET_START_INST(pCurrentWorkingBB);

            bSameJmpBB = vscHTBL_DirectTestAndGet(pSameJmpBBSet, (void *)pCurrentWorkingBB, gcvNULL);

            while (gcvTRUE)
            {
                /* We don't need to copy the initialized instructions. */
                if (VIR_Inst_GetId(pCurrentWorkingInst) >= VIR_Inst_GetId(pContext->pInitializationStartInst)
                    &&
                    VIR_Inst_GetId(pCurrentWorkingInst) <= VIR_Inst_GetId(pContext->pInitializationEndInst))
                {
                    if (pCurrentWorkingInst == BB_GET_END_INST(pCurrentWorkingBB))
                    {
                        break;
                    }

                    pCurrentWorkingInst = VIR_Inst_GetNext(pCurrentWorkingInst);
                    continue;
                }

                if ((pCurrentWorkingBB == pBB && pCurrentWorkingInst == pOrigBBEndInst)
                    ||
                    (pCurrentWorkingInst == BB_GET_END_INST(pCurrentWorkingBB)))
                {
                    bLastInst = gcvTRUE;
                }
                else
                {
                    bLastInst = gcvFALSE;
                }

                errCode = _vscVIR_DuplicateInstruction(pContext,
                                                       bSameJmpBB,
                                                       bLastInst,
                                                       pCurrentWorkingInst,
                                                       pNewEndInst,
                                                       pTempSet,
                                                       pLabelSet,
                                                       pJmpSet,
                                                       &pNewInst);
                ON_ERROR(errCode, "Duplicate instruction");
                pNewEndInst = pNewInst;

                if (bLastInst)
                {
                    break;
                }

                pCurrentWorkingInst = VIR_Inst_GetNext(pCurrentWorkingInst);
            }

            /*
            ** For those JMPs that cross the BARRIER, theoretically all threads within a local group have the same result,
            ** so we only execute one JMP at last, and change all the other to NOPs.
            */
            if (bLastInst && bSameJmpBB)
            {
                VIR_OpCode      opCode = VIR_Inst_GetOpcode(pCurrentWorkingInst);

                gcmASSERT(opCode == VIR_OP_NOP || VIR_OPCODE_isBranch(opCode) || opCode == VIR_OP_LABEL);

                if (VIR_OPCODE_isBranch(opCode))
                {
                    if (factorIndex != 0)
                    {
                        VIR_Function_ChangeInstToNop(pFunc, pNewInst);
                    }

                    if (factorIndex + 1== activeFactor - 1)
                    {
                        VIR_Function_ChangeInstToNop(pFunc, pCurrentWorkingInst);
                    }
                }
            }
        }

        ppNewStartInst[factorIndex] = pNewStartInst;
        ppNewEndInst[factorIndex] = pNewEndInst;
    }

OnError:
    return errCode;
}

static VSC_ErrCode
_vscVIR_CopyInstsAndRemapVariables(
    IN VSC_CutDownWGS*      pContext
    )
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VSC_MM*                 pMM = pContext->pMM;
    VIR_Shader*             pShader = pContext->pShader;
    VSC_HASH_TABLE*         pSameJmpBBSet = pContext->pSameJmpBBSet;
    VIR_Function*           pFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_Instruction*        pInst = gcvNULL;
    VIR_BASIC_BLOCK*        pBB;
    VIR_BASIC_BLOCK*        pPrevBB;
    VIR_IdList*             pWorkingBBList = gcvNULL;
    gctBOOL                 bLastInst = gcvFALSE, bSameJmpBB = gcvFALSE;
    gctUINT16               factorIndex, activeFactor = pContext->activeFactor;
    VSC_HASH_ITERATOR       jmpInstIter;
    VSC_DIRECT_HNODE_PAIR   jmpInstPair;

    /* Create the  working List. */
    VIR_IdList_Init(pMM, 8, &pWorkingBBList);

    for (pInst = VIR_Function_GetInstStart(pFunc); pInst != gcvNULL; pInst = VIR_Inst_GetNext(pInst))
    {
        pBB = VIR_Inst_GetBasicBlock(pInst);
        bLastInst = (BB_GET_END_INST(pBB) == VIR_Function_GetInstEnd(pFunc));
        bSameJmpBB = vscHTBL_DirectTestAndGet(pSameJmpBBSet, (void *)pBB, gcvNULL);

        if (bSameJmpBB)
        {
            /* Duplicate the existed BBs. */
            if (VIR_IdList_Count(pWorkingBBList) > 0)
            {
                pPrevBB = CFG_GET_BB_BY_ID(VIR_Function_GetCFG(pFunc), VIR_IdList_GetId(pWorkingBBList, VIR_IdList_Count(pWorkingBBList) - 1));
                errCode = _vscVIR_DuplicateBBs(pContext, pFunc, pPrevBB, pWorkingBBList);
                ON_ERROR(errCode, "Duplicate basic blocks.");

                pWorkingBBList->count = 0;
            }

            /* Duplicate this BB. */
            VIR_IdList_Add(pWorkingBBList, BB_GET_ID(pBB));
            errCode = _vscVIR_DuplicateBBs(pContext, pFunc, pBB, pWorkingBBList);
            ON_ERROR(errCode, "Duplicate basic blocks.");

            pWorkingBBList->count = 0;
            if (bLastInst)
            {
                break;
            }
        }
        else if (BB_FLAGS_HAS_BARRIER(pBB) || bLastInst)
        {
            if (!BB_FLAGS_HAS_BARRIER(pBB))
            {
                VIR_IdList_Add(pWorkingBBList, BB_GET_ID(pBB));
            }
            errCode = _vscVIR_DuplicateBBs(pContext, pFunc, pBB, pWorkingBBList);
            ON_ERROR(errCode, "Duplicate basic blocks.");

            pWorkingBBList->count = 0;
            if (bLastInst)
            {
                break;
            }
        }
        else
        {
            VIR_IdList_Add(pWorkingBBList, BB_GET_ID(pBB));
        }

        pInst = BB_GET_END_INST(pBB);
    }

    /* Update the label. */
    for (factorIndex = 0; factorIndex < activeFactor - 1; factorIndex++)
    {
        VSC_HASH_TABLE*     pLabelSet = pContext->ppLabelSet[factorIndex + 1];
        VSC_HASH_TABLE*     pJmpSet = pContext->ppJmpSet[factorIndex + 1];
        VIR_Label*          pLabel;
        VIR_Link*           pLink;
        VIR_Label*          pNewLabel = gcvNULL;

        /* Clean the in/out of the BBs in the current loop. */
        vscHTBLIterator_Init(&jmpInstIter, pJmpSet);
        for (jmpInstPair = vscHTBLIterator_DirectFirst(&jmpInstIter);
             IS_VALID_DIRECT_HNODE_PAIR(&jmpInstPair);
             jmpInstPair = vscHTBLIterator_DirectNext(&jmpInstIter))
        {
            VIR_Instruction*pJmpInst = (VIR_Instruction *)VSC_DIRECT_HNODE_PAIR_FIRST(&jmpInstPair);
            VIR_Function*   pFunc = VIR_Inst_GetFunction(pJmpInst);

            gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pJmpInst)));

            pLabel = VIR_Operand_GetLabel(VIR_Inst_GetDest(pJmpInst));

            if (vscHTBL_DirectTestAndGet(pLabelSet, (void *)(VIR_Function_GetSymFromId(pFunc, pLabel->sym)), (void **)(&pNewLabel)))
            {
                VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(pLabel), (gctUINTPTR_T)pJmpInst);

                VIR_Operand_SetLabel(VIR_Inst_GetDest(pJmpInst), pNewLabel);
                VIR_Function_NewLink(pFunc, &pLink);
                VIR_Link_SetReference(pLink, (gctUINTPTR_T)pJmpInst);
                VIR_Link_AddLink(VIR_Label_GetReferenceAddr(pNewLabel), pLink);
            }
        }
    }

OnError:
    VIR_IdList_Finalize(pWorkingBBList);

    return errCode;
}

DEF_SH_NECESSITY_CHECK(vscVIR_CutDownWorkGroupSize)
{
    return gcvTRUE;
}

DEF_QUERY_PASS_PROP(vscVIR_CutDownWorkGroupSize)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_CG;

    /* Only constant allocation is enable, we can check constant reg file read port limitation */
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_RA;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedCg = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedWeb = gcvTRUE;
    pPassProp->passFlag.resCreationReq.s.bNeedLvFlow = gcvTRUE;

    /* We need to invalid cfg. */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
}

VSC_ErrCode vscVIR_CutDownWorkGroupSize(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VSC_CutDownWGS      context;
    VIR_Shader*         pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO* pDuInfo = pPassWorker->pDuInfo;
    VSC_OPTN_LoopOptsOptions loopOptions;

    memset(&loopOptions, 0, sizeof(VSC_OPTN_LoopOptsOptions));

    VIR_Shader_RenumberInstId(pShader);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before cut down workGroupSize.", pShader, gcvTRUE);
    }

    /* I: Initialize the working context. */
    gcoOS_ZeroMemory(&context, gcmSIZEOF(VSC_CutDownWGS));
    _vscVIR_InitializeCutDownWGS(&context,
                                 pDuInfo,
                                 pPassWorker->basePassWorker.pMM,
                                 &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg,
                                 pShader,
                                 &loopOptions);

    /* II: Cut down the workGroupSize. */
    _vscVIR_CutDownWorkGroupSize(&context);

    /* III: Move all BARRIERs to a single BB. */
    errCode = _vscVIR_MoveBarrierOutOfBB(&context);
    ON_ERROR(errCode, "Move BARRIERs out of BB.");

    /* IV: Analyze multi-path with BARRIER. */
    errCode = _vscVIR_AnalyzeMultiPathWithBarrier(&context);
    ON_ERROR(errCode, "Analyze multi-path with BARRIER.");

    /* V: Detect any BARRIERs within a loop. */
    errCode = _vscVIR_DetectBarrierWithinLoop(&context);
    ON_ERROR(errCode, "Detect BARRIER within the loop.");

    /* VI: Re-caculate the builtin attributes. */
    errCode = _vscVIR_RecalculateBuiltinAttributes(&context);
    ON_ERROR(errCode, "Recalculate builtin attributes.");

    /* VII: Copy instructions and remap all variables. */
    errCode = _vscVIR_CopyInstsAndRemapVariables(&context);
    ON_ERROR(errCode, "Copy instructions and remap variables.");

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After cut down workGroupSize.", pShader, gcvTRUE);
    }

    VIR_Shader_RenumberInstId(pShader);

OnError:
    _vscVIR_FinalizeCutDownWGS(&context);
    return errCode;
}

/*-------------------------------------------Process the image-format mismatch-------------------------------------------*/
static gctBOOL
_vscVIR_IsImageReadOrWrite(
    IN VIR_Shader*      pShader,
    IN VIR_Instruction* pInst,
    INOUT gctBOOL*      pImageRead,
    INOUT gctBOOL*      pImageWrite,
    INOUT VIR_Operand** ppImageSrcOpnd
    )
{
    gctBOOL             bImageRead = gcvFALSE, bImageWrite = gcvFALSE;
    VIR_OpCode          opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Operand*        pImageSrcOpnd = gcvNULL;

    if (opCode == VIR_OP_INTRINSIC)
    {
        VIR_IntrinsicsKind  ik = VIR_Intrinsic_GetFinalIntrinsicKind(pInst);
        VIR_ParmPassing*    pParmOpnd = VIR_Operand_GetParameters(VIR_Inst_GetSource(pInst, 1));

        if (VIR_Intrinsics_isImageLoad(ik))
        {
            bImageRead = gcvTRUE;
            pImageSrcOpnd = pParmOpnd->args[0];
        }
        else if (VIR_Intrinsics_isImageStore(ik))
        {
            bImageWrite = gcvTRUE;
            pImageSrcOpnd = pParmOpnd->args[0];
        }
    }
    else if (VIR_OPCODE_isImgLd(opCode))
    {
        bImageRead = gcvTRUE;
        pImageSrcOpnd = VIR_Inst_GetSource(pInst, 0);
    }
    else if (VIR_OPCODE_isImgSt(opCode))
    {
        bImageWrite = gcvTRUE;
        pImageSrcOpnd = VIR_Inst_GetSource(pInst, 0);
    }

    if (pImageRead)
    {
        *pImageRead = bImageRead;
    }
    if (pImageWrite)
    {
        *pImageWrite = bImageWrite;
    }
    if (ppImageSrcOpnd)
    {
        *ppImageSrcOpnd = pImageSrcOpnd;
    }

    return bImageRead | bImageWrite;
}

static VSC_ErrCode
_vscVIR_ReplaceImageFormatMismatchInst(
    IN VIR_Shader*      pShader,
    IN VIR_Function*    pFunc,
    IN VIR_Instruction* pInst,
    IN gctBOOL          bImageRead,
    IN gctBOOL          bImageWrite
    )
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;

    if (bImageRead)
    {
        VIR_Operand*    pSrc0Opnd = VIR_Inst_GetSource(pInst, 0);
        VIR_TypeId      dataTypeId = VIR_Operand_GetTypeId(VIR_Inst_GetDest(pInst));
        gctUINT         i;
        VIR_ScalarConstVal constVal = { 0 };

        /* Free the sources. */
        for (i = 1; i < VIR_Inst_GetSrcNum(pInst); i++)
        {
            VIR_Inst_FreeSource(pInst, i);
        }

        /* Change to MOV. */
        VIR_Inst_SetOpcode(pInst, VIR_OP_MOV);
        VIR_Inst_SetConditionOp(pInst, VIR_COP_ALWAYS);
        VIR_Inst_SetSrcNum(pInst, 1);

        /* Set the data. */
        VIR_Operand_SetImmediate(pSrc0Opnd, VIR_GetTypeComponentType(dataTypeId), constVal);
        VIR_Operand_SetSwizzle(pSrc0Opnd, VIR_SWIZZLE_XXXX);
    }
    else if (bImageWrite)
    {
        VIR_Function_ChangeInstToNop(pFunc, pInst);
    }

    return errCode;
}

DEF_SH_NECESSITY_CHECK(vscVIR_ProcessImageFormatMismatch)
{
    VIR_Shader*         pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    /* Check for Vulkan only. */
    if (!VIR_Shader_IsVulkan(pShader))
    {
        return gcvFALSE;
    }

    if (!VIR_Shader_HasImageFormatMismatch(pShader))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

DEF_QUERY_PASS_PROP(vscVIR_ProcessImageFormatMismatch)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_ML;
}

VSC_ErrCode vscVIR_ProcessImageFormatMismatch(VSC_SH_PASS_WORKER* pPassWorker)
{
    VSC_ErrCode         errCode = VSC_ERR_NONE;
    VIR_Shader*         pShader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    /* Here we only need to process the main function because all image-related instructions should be inlined to the main function now. */
    VIR_Function*       pCurrentFunc = VIR_Shader_GetMainFunction(pShader);
    VIR_InstIterator    inst_iter;
    VIR_Instruction*    pInst;
    gctBOOL             bChanged = gcvFALSE;

    /*
    ** According to vulkan specs:
    **  1) If the image format of the OpTypeImage is not compatible with the VkImageView’s format,
    **     the write causes the contents of the image’s memory to become undefined.
    **  2) The Sampled Type of an OpTypeImage declaration must match the numeric format of the corresponding resource in type and signedness,
    **     as shown in the SPIR-V Sampled Type column of the Interpretation of Numeric Format table,
    **     or the values obtained by reading or sampling from this image are undefined.
    **
    **  So for any instructions that operate on such an image, we do:
    **  1) For a image read, use a "MOV 0" to replace it.
    **  2) For a image write, just remove it.
    */

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before process image format mismatch.", pShader, gcvTRUE);
    }

    VIR_InstIterator_Init(&inst_iter, VIR_Function_GetInstList(pCurrentFunc));
    for (pInst = (VIR_Instruction*)VIR_InstIterator_First(&inst_iter);
         pInst != gcvNULL;
         pInst = (VIR_Instruction*)VIR_InstIterator_Next(&inst_iter))
    {
        gctBOOL         bImageRead = gcvFALSE, bImageWrite = gcvFALSE;
        VIR_Operand*    pImageSrcOpnd = gcvNULL;
        VIR_Symbol*     pImageSrcSym = gcvNULL;

        /* Check if this instruction is an image read or write. */
        if (!_vscVIR_IsImageReadOrWrite(pShader, pInst, &bImageRead, &bImageWrite, &pImageSrcOpnd))
        {
            continue;
        }

        gcmASSERT(pImageSrcOpnd && VIR_Operand_isSymbol(pImageSrcOpnd));

        /* Find the image uniform. */
        pImageSrcSym = VIR_Operand_GetSymbol(pImageSrcOpnd);
        if (!(VIR_Symbol_isImageT(pImageSrcSym) || VIR_Symbol_isImage(pImageSrcSym)))
        {
            VIR_Operand*    pParentSrc = gcvNULL;
            pParentSrc = _vscVIR_FindParentImgOperandFromIndex(pInst,
                                                               pImageSrcOpnd,
                                                               VIR_Swizzle_GetChannel(VIR_Operand_GetSwizzle(pImageSrcOpnd), 0));
            pImageSrcSym = VIR_Operand_GetSymbol(pParentSrc);
        }
        gcmASSERT(pImageSrcSym && (VIR_Symbol_isImageT(pImageSrcSym)|| VIR_Symbol_isImage(pImageSrcSym)));

        /* Check if this uniform is image format mismatch. */
        if (!isSymUniformImageFormatMismatch(pImageSrcSym))
        {
            continue;
        }

        /* Replace this image read or write. */
        errCode = _vscVIR_ReplaceImageFormatMismatchInst(pShader, pCurrentFunc, pInst, bImageRead, bImageWrite);
        ON_ERROR(errCode, "Replace image format mistmatch instruction.");

        bChanged = gcvTRUE;
    }

    if (bChanged &&
        VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(pShader), VIR_Shader_GetId(pShader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After process image format mismatch.", pShader, gcvTRUE);
    }

OnError:
    return errCode;
}

