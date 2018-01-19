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


#include "vir/transform/gc_vsc_vir_dce.h"

static const VSC_DCE_Mark emptyMark = { 0 };

static gctUINT
_VSC_DCE_GetInstChannelNum(
    IN VIR_OpCode opcode
    )
{
    switch (opcode)
    {
    case VIR_OP_DP2:
    case VIR_OP_NORM_DP2:
        return 2;

    case VIR_OP_DP3:
    case VIR_OP_NORM_DP3:
        return 3;

    case VIR_OP_DP4:
    case VIR_OP_NORM_DP4:
        return 4;

    case VIR_OP_CROSS:
    case VIR_OP_NORM:
        return 4;

    case VIR_OP_TEXLD:
    case VIR_OP_TEXLD_U:
    case VIR_OP_TEXLDPROJ:
    case VIR_OP_TEXLDPCF:
    case VIR_OP_TEXLDPCFPROJ:
        return 4;

    default:
        if (VIR_OPCODE_isMemSt(opcode))
        {
            return 4;
        }
        return 4;
    }
}

static void
_VSC_DCE_DumpWorkListNode(
    IN VSC_DCE          *dce,
    IN VIR_Instruction  *inst
    )
{
    VIR_Enable  enable = (VIR_Enable)VSC_DCE_GetMarkByInst(dce, inst).isAlive;
    VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);

    VIR_Inst_Dump(dumper, inst);
    VIR_LOG(dumper, "\t{");
    VIR_Enable_Dump(dumper, enable);
    VIR_LOG(dumper, "}");
    VIR_LOG_FLUSH(dumper);
}

static void
_VSC_DCE_DumpWorkList(
    IN VSC_DCE *dce
    )
{
    VSC_QUEUE_STACK_ENTRY *worklist = (VSC_QUEUE_STACK_ENTRY *)QUEUE_PEEK_HEAD_ENTRY(&VSC_DCE_GetWorkList(dce));
    VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);

    VIR_LOG(dumper, "WorkList:");
    VIR_LOG_FLUSH(dumper);

    while(worklist)
    {
        _VSC_DCE_DumpWorkListNode(dce, (VIR_Instruction *)vscULNDEXT_GetContainedUserData(worklist));

        worklist = (VSC_QUEUE_STACK_ENTRY *)SQE_GET_NEXT_ENTRY(worklist);
    }
}

static void
_VSC_DCE_WorkListQueue(
    IN VSC_DCE         *dce,
    IN VIR_Instruction *inst
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)vscMM_Alloc(VSC_DCE_GetMM(dce),
        sizeof(VSC_UNI_LIST_NODE_EXT));
    VSC_OPTN_DCEOptions  *options      = VSC_DCE_GetOptions(dce);

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
        VSC_OPTN_DCEOptions_TRACE_ADDED))
    {
        VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);

        VIR_LOG(dumper, "Adding:");
        VIR_LOG_FLUSH(dumper);

        _VSC_DCE_DumpWorkListNode(dce, inst);
    }
    vscULNDEXT_Initialize(worklistNode, inst);
    QUEUE_PUT_ENTRY(&VSC_DCE_GetWorkList(dce), worklistNode);
}

static void
_VSC_DCE_JmpListAdd(
    IN VSC_DCE         *dce,
    IN VIR_Instruction *inst
    )
{
    VSC_BI_LIST_NODE_EXT *jmpListNode = (VSC_BI_LIST_NODE_EXT *)vscMM_Alloc(VSC_DCE_GetMM(dce),
        sizeof(VSC_BI_LIST_NODE_EXT));
    vscBLNDEXT_Initialize(jmpListNode, inst);
    vscBILST_Append(&VSC_DCE_GetJmpList(dce), CAST_BLEN_2_BLN((jmpListNode)));
}

static void
_VSC_DCE_WorkListDequeue(
    IN  VSC_DCE            *dce,
    OUT VIR_Instruction   **inst
    )
{
    VSC_UNI_LIST_NODE_EXT *worklistNode = (VSC_UNI_LIST_NODE_EXT *)QUEUE_GET_ENTRY(&VSC_DCE_GetWorkList(dce));

    *inst = (VIR_Instruction *)vscULNDEXT_GetContainedUserData(worklistNode);

    vscMM_Free(VSC_DCE_GetMM(dce), worklistNode);
}

static void
_VSC_DCE_Init(
    IN OUT VSC_DCE          *dce,
    IN VIR_Shader           *shader,
    IN VIR_DEF_USAGE_INFO   *du_info,
    IN VSC_OPTN_DCEOptions  *options,
    IN VIR_Dumper           *dumper,
    IN VSC_MM               *pMM
    )
{
    VIR_FuncIterator    func_iter;
    VIR_FunctionNode   *func_node;
    gctINT              maxInstId   = 0;
    gctUINT             maxBBId     = 0;
    gctUINT             maxFuncId   = 0;

    VSC_DCE_SetShader(dce, shader);
    VSC_DCE_SetDUInfo(dce, du_info);
    VSC_DCE_SetOptions(dce, options);
    VSC_DCE_SetDumper(dce, dumper);
    dce->pMM = pMM;
    VSC_DCE_SetOptCount(dce, 0);

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));

    maxInstId = VIR_Shader_RenumberInstId(shader);

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func       = func_node->function;

        {
            CFG_ITERATOR            cfg_iter;
            VIR_CONTROL_FLOW_GRAPH *pCFG;
            VIR_BASIC_BLOCK        *bb;

            pCFG = &func->pFuncBlock->cfg;

            CFG_ITERATOR_INIT(&cfg_iter, pCFG);
            for(bb = CFG_ITERATOR_FIRST(&cfg_iter); bb != gcvNULL;
                bb = CFG_ITERATOR_NEXT(&cfg_iter))
            {
                if (maxBBId < bb->dgNode.id)
                {
                    maxBBId = bb->dgNode.id;
                }
            }
        }

        if (maxFuncId < func->pFuncBlock->dgNode.id)
        {
            maxFuncId = func->pFuncBlock->dgNode.id;
        }
    }

    QUEUE_INITIALIZE(&VSC_DCE_GetWorkList(dce));
    vscBILST_Initialize(&VSC_DCE_GetJmpList(dce), gcvFALSE);

    if (maxInstId > 0)
    {
        VSC_DCE_SetMark(dce, (VSC_DCE_Mark *)vscMM_Alloc(VSC_DCE_GetMM(dce),
            sizeof(VSC_DCE_Mark) * maxInstId));
        memset(VSC_DCE_GetMark(dce), 0, sizeof(VSC_DCE_Mark) * maxInstId);
    }

    maxBBId++;
    maxFuncId++;

    dce->maxBBCount = maxBBId;
    dce->maxInstCount = maxInstId;

    VSC_DCE_SetBBInfo(dce, (VSC_DCE_BBInfo *)vscMM_Alloc(VSC_DCE_GetMM(dce),
        sizeof(VSC_DCE_BBInfo) * maxBBId * maxFuncId));
    memset(VSC_DCE_GetBBInfo(dce), 0, sizeof(VSC_DCE_BBInfo) * maxBBId * maxFuncId);

    dce->rebuildCFG = gcvFALSE;
}

static void
_VSC_DCE_Final(
    IN OUT VSC_DCE  *dce
    )
{
    VSC_DCE_SetShader(dce, gcvNULL);
    VSC_DCE_SetOptions(dce, gcvNULL);
    VSC_DCE_SetDumper(dce, gcvNULL);
    QUEUE_FINALIZE(&VSC_DCE_GetWorkList(dce));
    vscBILST_Finalize(&VSC_DCE_GetJmpList(dce));
}

static void
_VSC_DCE_MarkAndQueueOutput(
    IN VSC_DCE         *dce,
    IN VIR_Instruction *inst,
    IN VIR_OperandInfo *dest_info
    )
{
    VIR_Operand        *dest  = VIR_Inst_GetDest(inst);
    gctUINT8            channel = 0;
    VIR_Enable          enable;
    VIR_Symbol*         sym;

    if(!dest)
    {
        return;
    }


    if(!VIR_OpndInfo_Is_Virtual_Reg(dest_info))
    {
        return;
    }

    enable = VIR_Operand_GetEnable(dest);

    if(!dest_info->isOutput)
    {
        return;
    }

    sym = VIR_Operand_GetSymbol(dest);
    if (VIR_Symbol_GetKind(sym) == VIR_SYM_VIRREG)
    {
        sym = VIR_Symbol_GetVregVariable(sym);
    }

    if (isSymUnused(sym) || isSymVectorizedOut(sym))
    {
        return;
    }

    /* This inst has been marked. */
    if((gctINT8)enable == VSC_DCE_GetMarkByInst(dce, inst).isAlive)
    {
        return;
    }

    /* Check if it's a real output. */
    for(channel = 0; channel < VIR_CHANNEL_NUM; channel ++)
    {
        VIR_GENERAL_DU_ITERATOR  du_iter;
        VIR_USAGE               *use        = gcvNULL;

        if(!(enable & (1 << channel)))
        {
            continue;
        }

        vscVIR_InitGeneralDuIterator(
            &du_iter,
            VSC_DCE_GetDUInfo(dce),
            inst,
            dest_info->u1.virRegInfo.virReg,
            channel,
            gcvFALSE);

        for(use = vscVIR_GeneralDuIterator_First(&du_iter);
            use != gcvNULL;
            use = vscVIR_GeneralDuIterator_Next(&du_iter))
        {
            VIR_Instruction *use_inst   = use->usageKey.pUsageInst;

            if (VIR_IS_OUTPUT_USAGE_INST(use_inst))
            {
                VSC_DCE_SetAliveByInst(dce, inst,
                    VSC_DCE_GetMarkByInst(dce, inst).isAlive | (1 << channel));
                break;
            }
        }
    }

    if(VSC_DCE_GetMarkByInst(dce, inst).isAlive)
    {
        _VSC_DCE_WorkListQueue(dce, inst);
    }

    return;
}

static void
_VSC_DCE_MarkInstAll(
    IN     VSC_DCE          *dce,
    IN     VIR_Instruction  *inst,
    IN     VIR_OperandInfo  *dest_info
    )
{
    VIR_Enable enable = VIR_ENABLE_XYZW;
    VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

    if(VIR_OPCODE_hasDest(opcode) &&
       !VIR_OPCODE_isVXOnly(opcode))
    {
        VIR_Operand        *dest  = VIR_Inst_GetDest(inst);
        if(VIR_OpndInfo_Is_Virtual_Reg(dest_info))
        {
            enable = VIR_Operand_GetEnable(dest);
        }
    }
    else
    {
        switch (opcode)
        {
        case VIR_OP_NOP:
            enable = VIR_ENABLE_NONE;
            break;
        default:
            break;
        }
    }

   _VSC_DCE_WorkListQueue(dce, inst);
    VSC_DCE_SetAliveByInst(dce, inst, enable);
}

static void
_VSC_DCE_InitDCEOnFunction(
    IN     VSC_DCE      *dce,
    IN OUT VIR_Function *func
    )
{
    VIR_Instruction     *inst    = func->instList.pHead;
    VSC_OPTN_DCEOptions *options = VSC_DCE_GetOptions(dce);

    while(inst != gcvNULL)
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);

        if (VIR_OPCODE_isVXOnly(opcode))
        {
            _VSC_DCE_MarkInstAll(dce, inst, gcvNULL);
        }
        else if (!VIR_OPCODE_hasDest(opcode))
        {
            _VSC_DCE_MarkInstAll(dce, inst, gcvNULL);
        }
        else
        {
            VIR_Operand        *dest  = VIR_Inst_GetDest(inst);
            VIR_OperandInfo     dest_info;

            gcmASSERT(dest);

            VIR_Operand_GetOperandInfo(inst, dest, &dest_info);

            if (VIR_OPCODE_LoadsOrStores(opcode) &&
                !VIR_OPCODE_LoadsOnly(opcode))
            {
                _VSC_DCE_MarkInstAll(dce, inst, &dest_info);
            }
            else if (VIR_OPCODE_isBranch(opcode))
            {
                if (VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetOPTS(options),
                    VSC_OPTN_DCEOptions_OPTS_CONTROL))
                {
                    _VSC_DCE_JmpListAdd(dce, inst);
                }
                else
                {
                    _VSC_DCE_MarkInstAll(dce, inst, &dest_info);
                }
            }
            else if(VIR_OpndInfo_Is_Virtual_Reg(&dest_info))
            {
                /* If inst's dest is output, mark it. */
                _VSC_DCE_MarkAndQueueOutput(dce, inst, &dest_info);
            }
            else if (opcode == VIR_OP_LABEL)
            {
                /* skip */
            }
            else
            {
                _VSC_DCE_MarkInstAll(dce, inst, &dest_info);
            }
        }

        inst = VIR_Inst_GetNext(inst);
    }
}

static VSC_ErrCode
_VSC_DCE_MarkAndQueueAllDefs(
    IN VSC_DCE          *dce,
    IN VIR_Instruction  *inst,
    IN VIR_Operand      *opnd
    )
{
    VSC_ErrCode              errCode        = VSC_ERR_NONE;
    gctINT8                  instSrcAlive   = VIR_ENABLE_NONE;
    VIR_Swizzle              inst_src_swiz;
    VIR_GENERAL_UD_ITERATOR  ud_iter;
    VIR_DEF                 *def            = gcvNULL;
    VIR_OperandInfo          inst_src_info;
    gctSIZE_T                channel        = 0;
    VIR_OpCode               opcode         = VIR_Inst_GetOpcode(inst);

    VIR_Operand_GetOperandInfo(inst, opnd, &inst_src_info);

    if(!VIR_OpndInfo_Is_Virtual_Reg(&inst_src_info))
    {
        return errCode;
    }

    vscVIR_InitGeneralUdIterator(&ud_iter, VSC_DCE_GetDUInfo(dce), inst, opnd, gcvFALSE, gcvFALSE);

    inst_src_swiz = VIR_Operand_GetSwizzle(opnd);
    if(!VIR_Inst_isComponentwise(inst))
    {
        gctUINT channelNum = _VSC_DCE_GetInstChannelNum(opcode);
        for(channel = 0; channel < channelNum; ++channel)
        {
            instSrcAlive |= 1 << VIR_Swizzle_GetChannel(inst_src_swiz, channel);
        }
    }
    else
    {
        for(channel = 0; channel < VIR_CHANNEL_NUM; ++channel)
        {
            if(VSC_DCE_GetMarkByInst(dce, inst).isAlive & (1 << channel))
            {
                instSrcAlive |= 1 << VIR_Swizzle_GetChannel(inst_src_swiz, channel);
            }
        }
    }

    for(def = vscVIR_GeneralUdIterator_First(&ud_iter);
        def != gcvNULL;
        def = vscVIR_GeneralUdIterator_Next(&ud_iter))
    {
        VIR_Instruction *def_inst        = def->defKey.pDefInst;
        gctINT8          isAlive         = 0;

        if (VIR_IS_IMPLICIT_DEF_INST(def_inst))
        {
            continue;
        }

        isAlive = VSC_DCE_GetMarkByInst(dce, def_inst).isAlive;

        VSC_DCE_SetAliveByInst(dce, def_inst,
            VSC_DCE_GetMarkByInst(dce, def_inst).isAlive | ((1 << def->defKey.channel) & instSrcAlive));

        if (VSC_DCE_GetMarkByInst(dce, def_inst).isAlive != 0 &&
            VSC_DCE_GetMarkByInst(dce, def_inst).isAlive != isAlive)
        {
            _VSC_DCE_WorkListQueue(dce, def_inst);
        }
    }

    return errCode;
}

static VSC_ErrCode
_VSC_DCE_AnalysisDeadCodeOnShader(
    IN      VSC_DCE         *dce
    )
{
    VSC_ErrCode             errCode    = VSC_ERR_NONE;
    VSC_OPTN_DCEOptions    *options    = VSC_DCE_GetOptions(dce);

    while(!QUEUE_CHECK_EMPTY(&VSC_DCE_GetWorkList(dce)))
    {
        VIR_Instruction *inst           = gcvNULL;
        VIR_BASIC_BLOCK *bb             = gcvNULL;
        VIR_Operand     *opnd           = gcvNULL;

        VIR_SrcOperand_Iterator opndIter;

        _VSC_DCE_WorkListDequeue(dce, &inst);

        bb     = VIR_Inst_GetBasicBlock(inst);

        if (bb == gcvNULL)
        {
            gcmASSERT(gcvFALSE);
            return VSC_ERR_INVALID_DATA;
        }

        if(!VSC_DCE_GetBBInfoByBB(dce, bb).calculated)
        {
            VSC_BI_LIST_NODE_EXT *jmpList = (VSC_BI_LIST_NODE_EXT *)vscBILST_GetHead(&VSC_DCE_GetJmpList(dce));

            VSC_DCE_GetBBInfoByBB(dce, bb).calculated = gcvTRUE;
            while(jmpList)
            {
                VIR_Instruction      *jmpInst   = (VIR_Instruction *)vscBLNDEXT_GetContainedUserData(jmpList);
                VIR_BASIC_BLOCK      *jmpBB     = VIR_Inst_GetBasicBlock(jmpInst);
                VSC_BI_LIST_NODE_EXT *nxtJmp    = (VSC_BI_LIST_NODE_EXT *)vscBLNDEXT_GetNextNode(jmpList);

                gcmASSERT(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetOPTS(options),
                    VSC_OPTN_DCEOptions_OPTS_CONTROL));

                gcmASSERT(jmpBB != gcvNULL);

                if(VIR_Inst_GetFunction(inst) == VIR_Inst_GetFunction(jmpInst) &&
                    vscBV_TestBit(&bb->cdSet, jmpBB->dgNode.id))
                {
                    /* Add to workList */
                    _VSC_DCE_WorkListQueue(dce, jmpInst);
                     VSC_DCE_SetAliveByInst(dce, jmpInst, VIR_ENABLE_XYZW);

                    /* Delete from jmpList */
                    vscBILST_Remove(&VSC_DCE_GetJmpList(dce), CAST_BLEN_2_BLN(jmpList));
                    vscMM_Free(VSC_DCE_GetMM(dce), jmpList);
                }

                jmpList = nxtJmp;
            }
        }

        if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
            VSC_OPTN_DCEOptions_TRACE_ANALYSIS))
        {
            VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);

            VIR_LOG(dumper, "Analysis:");
            VIR_LOG_FLUSH(dumper);

            _VSC_DCE_DumpWorkListNode(dce, inst);
        }

        VIR_SrcOperand_Iterator_Init(inst, &opndIter);
        opnd = VIR_SrcOperand_Iterator_First(&opndIter);

        for (; opnd != gcvNULL; opnd = VIR_SrcOperand_Iterator_Next(&opndIter))
        {
            _VSC_DCE_MarkAndQueueAllDefs(dce, inst, opnd);
        }
    }

    return errCode;
}

static VSC_ErrCode
_VSC_DCE_DeleteUsage(
    IN VSC_DCE          *dce,
    IN VIR_Instruction  *inst,
    IN VIR_Operand      *opnd,
    IN VIR_Enable        coverMask,
    IN gctUINT8          coverFlag
    )
{
    VSC_ErrCode     errCode             = VSC_ERR_NONE;
    VIR_Operand    *inst_src            = opnd;
    VIR_OperandInfo inst_src_info;
    VIR_Enable      delete_src_enable   = VIR_ENABLE_NONE;
    VIR_Swizzle     inst_src_swizzle    = VIR_Operand_GetSwizzle(inst_src);
    gctSIZE_T       channel             = 0;

    VIR_Operand_GetOperandInfo(inst, inst_src, &inst_src_info);

    if (!VIR_OpndInfo_Is_Virtual_Reg(&inst_src_info) &&
        VIR_Operand_GetOpKind(opnd) != VIR_OPND_LABEL)
    {
        return errCode;
    }

    if(VIR_Operand_GetOpKind(opnd) == VIR_OPND_LABEL)
    {
        VIR_Label *label = VIR_Operand_GetLabel(opnd);

        gcmASSERT(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetOPTS(VSC_DCE_GetOptions(dce)),
            VSC_OPTN_DCEOptions_OPTS_CONTROL));

        VIR_Link_RemoveLink(&label->referenced, (gctUINTPTR_T)inst);
    }
    else
    {
        for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
        {
            if(!(coverMask & (1 << channel)))
            {
                continue;
            }

            if (!(coverFlag & (1 << channel)))
            {
                /* delete the crossponding unused channel */
                delete_src_enable |= (1<< VIR_Swizzle_GetChannel(inst_src_swizzle, channel));
            }
        }

        for(channel = 0; channel < VIR_CHANNEL_NUM; channel++)
        {
            if(!(coverMask & (1 << channel)))
            {
                continue;
            }

            if (coverFlag & (1 << channel))
            {
                /* the channel in the used channel should not be deleted */
                delete_src_enable &= (~(1<< VIR_Swizzle_GetChannel(inst_src_swizzle, channel)));
            }
        }

        if (delete_src_enable != VIR_ENABLE_NONE)
        {
            /* delete the usage of inst_src */
            vscVIR_DeleteUsage(
                VSC_DCE_GetDUInfo(dce),
                VIR_ANY_DEF_INST,
                inst,
                inst_src,
                gcvFALSE,
                inst_src_info.u1.virRegInfo.virReg,
                1,
                delete_src_enable,
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);
        }
    }

    return errCode;
}

static VIR_Instruction*
_VSC_DCE_GetNextInst(
    IN VSC_DCE          *dce,
    IN VIR_Function     *func,
    IN VIR_Instruction  *inst,
    IN VSC_DCE_Mark      flag
    )
{
    VSC_OPTN_DCEOptions *options    = VSC_DCE_GetOptions(dce);

    if(flag.isAlive == 0)
    {
        VIR_Instruction *nxtInst = VIR_Inst_GetNext(inst);

        if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
            VSC_OPTN_DCEOptions_TRACE_REMOVED))
        {
            VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);

            VIR_LOG(dumper, "Deleting:");
            VIR_LOG_FLUSH(dumper);

            _VSC_DCE_DumpWorkListNode(dce, inst);
        }

        VIR_Function_RemoveInstruction(func, inst);
        inst = nxtInst;
    }
    else
    {
        inst = VIR_Inst_GetNext(inst);
    }

    return inst;
}

static VSC_ErrCode
_VSC_DCE_RemoveDeadCodeOnFunction(
    IN     VSC_DCE      *dce,
    IN OUT VIR_Function *func
    )
{
    VSC_ErrCode         errCode     = VSC_ERR_NONE;
    VIR_Instruction    *inst        = gcvNULL;

    inst = func->instList.pHead;

    while (inst != gcvNULL)
    {
        VSC_DCE_Mark     flag           = VSC_DCE_GetMarkByInst(dce, inst);
        VIR_Operand     *inst_dest      = VIR_Inst_GetDest(inst);
        VIR_Enable       inst_enable    = VIR_ENABLE_XYZW;
        VIR_OpCode       opcode         = VIR_Inst_GetOpcode(inst);
        VIR_OperandInfo  destInfo       = { 0 };

        if(flag.isAlive == VIR_ENABLE_XYZW)
        {
            inst = VIR_Inst_GetNext(inst);
            continue;
        }

        if(inst_dest != gcvNULL)
        {
            VIR_Operand_GetOperandInfo(inst, inst_dest, &destInfo);

            if(VIR_OpndInfo_Is_Virtual_Reg(&destInfo))
            {
                inst_enable = VIR_Inst_GetEnable(inst);
            }
        }

        /* HW has restrictions on NORM_DPx and NORM_MUL that src component for pre-normalize
           must be same and enable of NORM_MUL must be xy/xyz/xyzw, so let's keep it as original. */
        if (VIR_Inst_GetOpcode(inst) == VIR_OP_NORM     ||
            VIR_Inst_GetOpcode(inst) == VIR_OP_NORM_DP2 ||
            VIR_Inst_GetOpcode(inst) == VIR_OP_NORM_DP3 ||
            VIR_Inst_GetOpcode(inst) == VIR_OP_NORM_DP4 ||
            VIR_Inst_GetOpcode(inst) == VIR_OP_NORM_MUL ||
            VIR_OPCODE_isVXOnly(VIR_Inst_GetOpcode(inst)) ||
            (VIR_OPCODE_LoadsOrStores(VIR_Inst_GetOpcode(inst)) && VIR_OPCODE_LoadsOnly(VIR_Inst_GetOpcode(inst))))
        {
            if (VSC_DCE_GetMarkByInst(dce, inst).isAlive != 0)
            {
                VSC_DCE_SetAliveByInst(dce, inst, inst_enable);
                flag = VSC_DCE_GetMarkByInst(dce, inst);
            }
        }

        if(flag.isAlive == (gctINT8)inst_enable)
        {
            inst = VIR_Inst_GetNext(inst);
            continue;
        }

        gcmASSERT(VIR_Enable_Covers(inst_enable, flag.isAlive));

        VSC_DCE_GetOptCount(dce)+= vscFindPopulation(inst_enable ^ flag.isAlive);

        if (inst_dest == gcvNULL)
        {
            inst = _VSC_DCE_GetNextInst(dce, func, inst, flag);
            continue;
        }
        else if(VIR_OPCODE_isBranch(opcode))
        {
            VIR_Enable               coverMask   = inst_enable;
            gctUINT8                 coverFlag   = flag.isAlive;
            VIR_BASIC_BLOCK         *curBB       = VIR_Inst_GetBasicBlock(inst);
            VSC_TREE_NODE           *treeNode;
            VIR_Label               *target_lb   = gcvNULL;
            VIR_Operand             *target_dest = gcvNULL;
            VIR_Instruction         *target_inst = gcvNULL;
            VIR_BASIC_BLOCK         *target_bb   = gcvNULL;
            VIR_Operand             *opnd        = gcvNULL;
            VIR_SrcOperand_Iterator  opndIter;

            gcmASSERT(curBB != gcvNULL);
            gcmASSERT(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetOPTS(VSC_DCE_GetOptions(dce)),
                VSC_OPTN_DCEOptions_OPTS_CONTROL));

            opcode = VIR_Inst_GetOpcode(inst);

            treeNode = curBB->pPostDomTreeNode->treeNode.pParentNode;
            gcmASSERT(treeNode);

            while(VSC_DCE_GetBBInfoByBB(dce, ((VIR_DOM_TREE_NODE *)treeNode)->pOwnerBB).calculated == gcvFALSE &&
                  ((VIR_DOM_TREE_NODE *)treeNode)->pOwnerBB->flowType != VIR_FLOW_TYPE_EXIT)
            {
                treeNode = treeNode->pParentNode;
                gcmASSERT(treeNode);
            }

            target_bb = ((VIR_DOM_TREE_NODE *)treeNode)->pOwnerBB;

            if (target_bb == VIR_Inst_GetBranchTargetBB(inst) &&
                !VIR_OPCODE_isConditionBranch(opcode))
            {
                inst = VIR_Inst_GetNext(inst);
                continue;
            }

            VIR_SrcOperand_Iterator_Init(inst, &opndIter);
            opnd = VIR_SrcOperand_Iterator_First(&opndIter);

            for (; opnd != gcvNULL; opnd = VIR_SrcOperand_Iterator_Next(&opndIter))
            {
                _VSC_DCE_DeleteUsage(dce, inst, opnd, coverMask, coverFlag);
            }

            /* JMP's dest is also the usage of Label. */
            _VSC_DCE_DeleteUsage(dce, inst, inst->dest, coverMask, coverFlag);

            if(target_bb->flowType != VIR_FLOW_TYPE_EXIT)
            {
                if (VIR_Inst_GetOpcode(target_bb->pStartInst) != VIR_OP_LABEL)
                {
                    VIR_Instruction *label_inst = gcvNULL;
                    VIR_Label       *label      = gcvNULL;
                    VIR_LabelId      label_id;

                    gcmASSERT(target_bb->pStartInst);
                    VIR_Function_AddInstructionBefore(func,
                                                      VIR_OP_LABEL,
                                                      VIR_TYPE_UNKNOWN,
                                                      target_bb->pStartInst,
                                                      gcvTRUE,
                                                      &label_inst);

                    VIR_Function_AddLabel(func, gcvNULL, &label_id);

                    label = VIR_GetLabelFromId(func, label_id);
                    label->defined = label_inst;

                    VIR_Operand_SetLabel(VIR_Inst_GetDest(label_inst), label);
                }
                target_inst = target_bb->pStartInst;
            }
            else
            {
                VIR_Instruction* lastInst = VIR_Function_GetInstEnd(func);
                if(lastInst == inst)
                {
                    VIR_Function_RemoveInstruction(func, inst);
                    break;
                }
                else if(VIR_Inst_GetOpcode(lastInst) != VIR_OP_LABEL)
                {
                    VIR_Instruction *label_inst = gcvNULL;
                    VIR_Label       *label      = gcvNULL;
                    VIR_LabelId      label_id;

                    VIR_Function_AddInstructionAfter(func,
                                                     VIR_OP_LABEL,
                                                     VIR_TYPE_UNKNOWN,
                                                     lastInst,
                                                     gcvTRUE,
                                                     &label_inst);

                    VIR_Function_AddLabel(func, gcvNULL, &label_id);

                    label = VIR_GetLabelFromId(func, label_id);
                    label->defined = label_inst;

                    VIR_Operand_SetLabel(VIR_Inst_GetDest(label_inst), label);
                }
                target_inst = VIR_Function_GetInstEnd(func);
            }

            target_dest = VIR_Inst_GetDest(target_inst);
            target_lb   = VIR_Operand_GetLabel(target_dest);

            {
                VIR_Link        *link       = gcvNULL;

                VIR_Operand_SetLabel(VIR_Inst_GetDest(inst), target_lb);
                VIR_Function_NewLink(func, &link);
                VIR_Link_SetReference(link, (gctUINTPTR_T)inst);
                VIR_Link_AddLink(&(target_lb->referenced), link);
            }

            if (VIR_OPCODE_isConditionBranch(opcode))
            {
                VIR_Inst_SetOpcode(inst, VIR_OP_JMP);
                VIR_Inst_SetConditionOp(inst, VIR_COP_ALWAYS);

                opnd = VIR_SrcOperand_Iterator_First(&opndIter);
                for (; opnd != gcvNULL; opnd = VIR_SrcOperand_Iterator_Next(&opndIter))
                {
                    VIR_Function_FreeOperand(func, opnd);
                }

                VIR_Inst_SetSrcNum(inst, 0);
            }
            else
            {
                VIR_Inst_SetOpcode(inst, VIR_OP_JMP);
            }

            gcmASSERT(flag.isAlive == 0);

            dce->rebuildCFG = gcvTRUE;

            inst = VIR_Inst_GetNext(inst);
            continue;
        }
        else if(VIR_OP_LABEL == opcode)
        {
            inst = VIR_Inst_GetNext(inst);
            continue;
        }
        else if(VIR_OpndInfo_Is_Virtual_Reg(&destInfo))
        {
            VIR_Enable               coverMask  = inst_enable;
            gctUINT8                 coverFlag  = flag.isAlive;
            VIR_Operand             *opnd       = gcvNULL;
            VIR_SrcOperand_Iterator  opndIter;

            if (!VIR_Inst_isComponentwise(inst))
            {
                coverMask = (1 << _VSC_DCE_GetInstChannelNum(opcode)) - 1;
                coverFlag = coverMask;
            }

            VIR_SrcOperand_Iterator_Init(inst, &opndIter);
            opnd = VIR_SrcOperand_Iterator_First(&opndIter);

            for (; opnd != gcvNULL; opnd = VIR_SrcOperand_Iterator_Next(&opndIter))
            {
                _VSC_DCE_DeleteUsage(dce, inst, opnd, coverMask, coverFlag);
            }

            vscVIR_DeleteDef(
                VSC_DCE_GetDUInfo(dce),
                inst,
                destInfo.u1.virRegInfo.virReg,
                1,
                inst_enable & (~flag.isAlive),
                VIR_HALF_CHANNEL_MASK_FULL,
                gcvNULL);

            VIR_Inst_SetEnable(inst, (VIR_Enable)flag.isAlive);
            /* normalize swizzle by enable */
            if (VIR_Inst_isComponentwise(inst))
            {
                VIR_Enable enable = VIR_Inst_GetEnable(inst);
                VIR_Operand * operand;
                VIR_Swizzle normalizedSwizzle;
                gctUINT i;

                for (i = 0; i < VIR_Inst_GetSrcNum(inst); i++)
                {
                    operand = VIR_Inst_GetSource(inst, i);
                    if (operand == gcvNULL
                        || VIR_Operand_GetOpKind(operand) == VIR_OPND_IMMEDIATE
                       )
                        continue;
                    normalizedSwizzle = VIR_NormalizeSwizzleByEnable(enable,
                                         VIR_Operand_GetSwizzle(operand));
                    VIR_Operand_SetSwizzle(operand, normalizedSwizzle);
                }
            }
            inst = _VSC_DCE_GetNextInst(dce, func, inst, flag);
            continue;
        }
        else
        {
            gcmASSERT(0);
        }
    }

    return errCode;
}

static VSC_ErrCode
_VSC_DCE_PerformOnShader(
    IN OUT VSC_DCE *dce
    )
{
    VSC_ErrCode          errcode    = VSC_ERR_NONE;
    VIR_Shader          *shader     = VSC_DCE_GetShader(dce);
    VIR_FuncIterator     func_iter;
    VIR_FunctionNode    *func_node  = gcvNULL;
    VSC_OPTN_DCEOptions *options    = VSC_DCE_GetOptions(dce);

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
        VSC_OPTN_DCEOptions_TRACE_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "DCE Begin", shader, gcvTRUE);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetOPTS(options),
        VSC_OPTN_DCEOptions_OPTS_CONTROL))
    {
        vscVIR_BuildPostDOMTree(shader);
        vscVIR_BuildControlDep(shader);

        if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
            VSC_OPTN_DCEOptions_TRACE_CONTROLFLOW))
        {
            VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
            for(func_node = VIR_FuncIterator_First(&func_iter);
                func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
            {
                VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);
                VIR_Function *func = func_node->function;
                VIR_CFG_Dump(dumper, &func->pFuncBlock->cfg, gcvTRUE);
            }
        }
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(shader));
    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function *func = func_node->function;
        _VSC_DCE_InitDCEOnFunction(dce, func);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
        VSC_OPTN_DCEOptions_TRACE_WORKLIST))
    {
        _VSC_DCE_DumpWorkList(dce);
    }

    _VSC_DCE_AnalysisDeadCodeOnShader(dce);

    for(func_node = VIR_FuncIterator_First(&func_iter);
        func_node != gcvNULL; func_node = VIR_FuncIterator_Next(&func_iter))
    {
        VIR_Function    *func = func_node->function;

        _VSC_DCE_RemoveDeadCodeOnFunction(dce, func);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
        VSC_OPTN_DCEOptions_TRACE_REMOVED) &&
        VSC_DCE_GetOptCount(dce) > 0)
    {
        VIR_Dumper *dumper = VSC_DCE_GetDumper(dce);
        VIR_LOG(dumper, "Delete %d components.", VSC_DCE_GetOptCount(dce));
        VIR_LOG_FLUSH(dumper);
    }

    if(VSC_UTILS_MASK(VSC_OPTN_DCEOptions_GetTrace(options),
        VSC_OPTN_DCEOptions_TRACE_OUTPUT) ||
       VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "DCE End", shader, gcvTRUE);
    }

    return errcode;
}

DEF_QUERY_PASS_PROP(VSC_DCE_Perform)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL |
                                 VSC_PASS_LEVEL_MC;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_DCE;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    pPassProp->passFlag.resCreationReq.s.bNeedDu = gcvTRUE;
}

VSC_ErrCode
VSC_DCE_Perform(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VIR_Shader           *shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_DEF_USAGE_INFO   *du_info = pPassWorker->pDuInfo;
    VSC_OPTN_DCEOptions  *options = (VSC_OPTN_DCEOptions*)pPassWorker->basePassWorker.pBaseOption;
    VIR_Dumper           *dumper = pPassWorker->basePassWorker.pDumper;
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_DCE dce;

    _VSC_DCE_Init(&dce, shader, du_info, options, dumper, pPassWorker->basePassWorker.pMM);
    errCode = _VSC_DCE_PerformOnShader(&dce);
    _VSC_DCE_Final(&dce);

    pPassWorker->pResDestroyReq->s.bInvalidateCfg = dce.rebuildCFG;

    return errCode;
}

