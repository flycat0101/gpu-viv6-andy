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
#include "vir/lower/gc_vsc_vir_hl_2_ml.h"
#include "vir/lower/gc_vsc_vir_lower_common_func.h"
#include "vir/transform/gc_vsc_vir_misc_opts.h"

static void
_Lower_Initialize(
    IN PVSC_CONTEXT             VscContext,
    IN VIR_Shader               *Shader,
    IN VSC_MM                   *pMM,
    IN VIR_PatternHL2MLContext  *Context
    )
{
    gcoOS_ZeroMemory(Context, sizeof(VIR_PatternHL2MLContext));

    Context->header.shader = Shader;

    /* Save the PMM. */
    Context->pMM = pMM;
    Context->vscContext = VscContext;
}

static void
_Lower_Finalize(
    IN VIR_Shader               *Shader,
    IN VIR_PatternHL2MLContext  *Context
    )
{
}

static VSC_ErrCode
_RemoveUnreachableBlockOnFunction(
    IN VIR_Shader               *Shader,
    IN VIR_Function             *Func
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Instruction             *inst = gcvNULL;

    inst = VIR_Function_GetInstStart(Func);
    for (; inst != gcvNULL; inst = VIR_Inst_GetNext(inst))
    {
        VIR_Instruction         *instIter = gcvNULL;
        VIR_Instruction         *labelInst = gcvNULL;
        VIR_Instruction         *callerInst = gcvNULL;
        VIR_Operand             *operand;
        VIR_Label               *label = gcvNULL;
        VIR_Link                *caller = gcvNULL;
        VIR_OpCode               opCode;

        opCode = VIR_Inst_GetOpcode(inst);

        if (opCode != VIR_OP_UNREACHABLE)
        {
            continue;
        }

        /* A block always starts with an OpLabel instruction, so find the VIR_OP_LABEL. */
        instIter = VIR_Inst_GetPrev(inst);
        for (; instIter != gcvNULL; instIter = VIR_Inst_GetPrev(instIter))
        {
            opCode = VIR_Inst_GetOpcode(instIter);

            if (opCode == VIR_OP_LABEL)
            {
                labelInst = instIter;
                break;
            }
        }

        if (labelInst == gcvNULL)
        {
            continue;
        }
        operand = VIR_Inst_GetDest(labelInst);
        label = VIR_Operand_GetLabel(operand);
        caller = label->referenced;

        /* Change all JMPs to this label to NOPs. */
        while (caller)
        {
            callerInst = (VIR_Instruction *)VIR_Link_GetReference(caller);

            VIR_Function_ChangeInstToNop(Func, callerInst);

            caller = caller->next;
        }

        /* Change all instructions in this block to NOPs*/
        for (instIter = labelInst; instIter && instIter != VIR_Inst_GetNext(inst); instIter = VIR_Inst_GetNext(instIter))
        {
            VIR_Function_ChangeInstToNop(Func, callerInst);
        }
    }

    return errCode;
}

static VSC_ErrCode
_RemoveUnreachableBlock(
    IN VIR_Shader               *Shader
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_Function                *func = gcvNULL;
    VIR_FuncIterator             iter;
    VIR_FunctionNode            *funcNode = gcvNULL;

    VIR_FuncIterator_Init(&iter, &Shader->functions);
    funcNode = VIR_FuncIterator_First(&iter);
    for (; funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&iter))
    {
        func = funcNode->function;

        errCode = _RemoveUnreachableBlockOnFunction(Shader,
                                                    func);
        ON_ERROR(errCode, "_RemoveUnreachableBlockOnFunction failed.");
    }

OnError:
    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_Lower_HighLevel_To_MiddleLevel)
{
    pPassProp->supportedLevels = (VSC_PASS_LEVEL)(VSC_PASS_LEVEL_HL | VSC_PASS_LEVEL_ML);
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;

    /* Lower must invalidate all analyzed resources */
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateCfg = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateRdFlow = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateDu = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateWeb = gcvTRUE;
    pPassProp->passFlag.resDestroyReq.s.bInvalidateLvFlow = gcvTRUE;
}

DEF_SH_NECESSITY_CHECK(VIR_Lower_HighLevel_To_MiddleLevel)
{
    VIR_Shader*                  shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;

    if (VIR_Shader_GetLevel(shader) != VIR_SHLEVEL_Post_High &&
        VIR_Shader_GetLevel(shader) != VIR_SHLEVEL_Pre_Medium)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VSC_ErrCode
_RemoveRedundantLabelForFunction(
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

    /*
    ** JMPC.ne            dp  layout() #sh_41, bool hp  layout() temp(270).hp.x, bool false
    ** LABEL              dp  layout() #sh_41
    ** JMP                dp  layout() #sh_28
    ** LABEL              dp  layout() #sh_42
    ** -->
    ** JMPC.ne            dp  layout() #sh_28, bool hp  layout() temp(270).hp.x, bool false
    ** JMP                dp  layout() #sh_42
    ** LABEL              dp  layout() #sh_42
    */
    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LABEL)
        {
            VIR_Instruction*   pNextNonNopInst = VIR_Inst_GetNext(pInst);
            VIR_Instruction*   pCallerInst = gcvNULL;
            VIR_Instruction*   pNextLabelInst = VIR_Inst_GetNext(pInst);
            VIR_Label*         pUpdateLabel;
            VIR_Label*         pOrigLabel;
            VIR_Link*          pLink;
            VIR_Link*          caller;

            /* Get the next non-NOP instruction. */
            while (pNextNonNopInst && VIR_Inst_GetOpcode(pNextNonNopInst) == VIR_OP_NOP)
            {
                pNextNonNopInst = VIR_Inst_GetNext(pNextNonNopInst);
            }
            if (pNextNonNopInst == gcvNULL) continue;

            /* Check if it is a direct branch. */
            if (VIR_Inst_GetOpcode(pNextNonNopInst) != VIR_OP_JMP) continue;

            /* Use the new label to replace the original label. */
            pOrigLabel = VIR_Inst_GetJmpLabel(pInst);
            pUpdateLabel = VIR_Inst_GetJmpLabel(pNextNonNopInst);

            /* Skip the loop here, actually this is a dead loop here. */
            if (pOrigLabel == pUpdateLabel)
            {
                continue;
            }

            caller = pOrigLabel->referenced;
            while (caller)
            {
                pCallerInst = (VIR_Instruction *)VIR_Link_GetReference(caller);

                if (VIR_Inst_GetOpcode(pCallerInst) != VIR_OP_NOP)
                {
                    /* Update the caller. */
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(pCallerInst), pUpdateLabel);
                    VIR_Function_NewLink(pFunc, &pLink);
                    VIR_Link_SetReference(pLink, (gctUINTPTR_T)pCallerInst);
                    VIR_Link_AddLink(&(pUpdateLabel->referenced), pLink);
                }
                caller = caller->next;

                /* Dereference instruction. */
                VIR_Link_RemoveLink(&pOrigLabel->referenced, (gctUINTPTR_T)pCallerInst);
            }

            /* Remove the instruction between this label and the next label. */
            while (pNextLabelInst && VIR_Inst_GetOpcode(pNextLabelInst) != VIR_OP_LABEL)
            {
                pNextLabelInst = VIR_Inst_GetNext(pNextLabelInst);
            }
            if (pNextLabelInst == gcvNULL) continue;

            for (; pInst != pNextLabelInst; pInst = VIR_Inst_GetNext(pInst))
            {
                VIR_Function_ChangeInstToNop(pFunc, pInst);
            }
        }
    }

    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = VIR_InstIterator_First(&instIter);
    /*
    ** JMP                dp  layout() #sh_27
    ** LABEL              dp  layout() #sh_27:
    **-->
    ** LABEL              dp  layout() #sh_27:
    */
    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_JMP)
        {
            VIR_Instruction*   pNextNonNopInst = VIR_Inst_GetNext(pInst);
            VIR_Label*         pJmpLabel = VIR_Inst_GetJmpLabel(pInst);

            /* Add a JMP to the next LABEL.  */
            while (pNextNonNopInst && VIR_Inst_GetOpcode(pNextNonNopInst) == VIR_OP_NOP)
            {
                pNextNonNopInst = VIR_Inst_GetNext(pNextNonNopInst);
            }
            if (pNextNonNopInst == gcvNULL) continue;

            if (VIR_Inst_GetOpcode(pNextNonNopInst) != VIR_OP_LABEL) continue;

            if (VIR_Inst_GetJmpLabel(pNextNonNopInst) == pJmpLabel)
            {
                /* Change JMP to NOP. */
                VIR_Function_ChangeInstToNop(pFunc, pInst);
            }
        }
    }

    /* Remove unuse LABELs. */
    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pInst = VIR_InstIterator_First(&instIter);
    for (; pInst != gcvNULL; pInst = VIR_InstIterator_Next(&instIter))
    {
        if (VIR_Inst_GetOpcode(pInst) == VIR_OP_LABEL)
        {
            VIR_Label*         pLabel = VIR_Inst_GetJmpLabel(pInst);

            if (pLabel->referenced == gcvNULL)
            {
                VIR_Function_ChangeInstToNop(pFunc, pInst);
            }
        }
    }

    return errCode;
}

static VSC_ErrCode
_RemoveRedundantLabelForFunctions(
    IN OUT VIR_Shader* pShader
    )
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    VIR_FuncIterator   func_iter;
    VIR_FunctionNode*  func_node;
    VIR_Function*      func;

    /* Temporary enable it for VK. */
    if (!VIR_Shader_IsVulkan(pShader))
    {
        return errCode;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));

    for (func_node = VIR_FuncIterator_First(&func_iter);
         func_node != gcvNULL;
         func_node = VIR_FuncIterator_Next(&func_iter))
    {
        func = func_node->function;

        errCode = _RemoveRedundantLabelForFunction(pShader,
                                                    func);
        ON_ERROR(errCode, "convert ret to jmp");

    }

OnError:
    return errCode;
}

VSC_ErrCode
VIR_Lower_HighLevel_To_MiddleLevel(
    IN VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_PatternHL2MLContext      context;
    VIR_Shader*                  shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VSC_MM                      *pmm = pPassWorker->basePassWorker.pMM;

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "Before HighLevel to MiddleLevel.", shader, gcvTRUE);
    }

    _Lower_Initialize(&pPassWorker->pCompilerParam->cfg.ctx, shader, pmm, &context);

    /* Remove block by checking VIR_OP_UNREACHABLE. */
    errCode = _RemoveUnreachableBlock(shader);
    CHECK_ERROR(errCode, "_RemoveUnreachableBlock failed.");

    /* Replace redundant labels. */
    errCode = _RemoveRedundantLabelForFunctions(shader);
    CHECK_ERROR(errCode, "_RemoveUnreachableBlock failed.");

    /* Expand HL instructions to ML opcodes. */
    errCode = VIR_Lower_HighLevel_To_MiddleLevel_Expand(shader, &context);
    CHECK_ERROR(errCode, "VIR_Lower_HighLevel_To_MiddleLevel_Expand failed.");

    /* Finalize context. */
    _Lower_Finalize(shader, &context);

    VIR_Shader_SetLevel(shader, VIR_SHLEVEL_Pre_Medium);
    {
        VSC_HW_CONFIG*   hwCfg = &pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg;
        errCode = VIR_Lower_ArraryIndexing_To_LDARR_STARR(shader, hwCfg, gcvNULL);
        CHECK_ERROR(errCode, "VIR_Lower_ArraryIndexing_To_LDARR_STARR failed.");
    }
    /* Renumber instruction ID. */
    VIR_Shader_RenumberInstId(shader);

    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE))
    {
        VIR_Shader_Dump(gcvNULL, "After HighLevel to MiddleLevel.", shader, gcvTRUE);
    }

    return errCode;
}


