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


#include "vir/transform/gc_vsc_vir_cfo.h"

void
VIR_CFO_Init(
    VIR_CFO* cfo,
    VIR_Shader* shader,
    VSC_HW_CONFIG* hwCfg,
    VSC_OPTN_CFOOptions* options,
    VIR_Dumper* dumper,
    VSC_MM* mm
    )
{
    VIR_CFO_SetShader(cfo, shader);
    VIR_CFO_SetHWCfg(cfo, hwCfg);
    VIR_CFO_SetOptions(cfo, options);
    VIR_CFO_SetDumper(cfo, dumper);
    VIR_CFO_SetMM(cfo, mm);
    VIR_CFO_SetInvalidCfg(cfo, gcvFALSE);
}

void
VIR_CFO_Final(
    VIR_CFO* cfo
    )
{
}

VSC_ErrCode
_VIR_CFO_PerformPatternTransformationOnFunction(
    VIR_CFO* cfo,
    VIR_Function* func,
    gctBOOL* changed
    )
{
    /* note: this is an iterative process */
    VSC_ErrCode errCode = VSC_ERR_NONE;

    VSC_OPTN_CFOOptions* options = VIR_CFO_GetOptions(cfo);
    VIR_Instruction* inst = VIR_Function_GetInstStart(func);
    gctBOOL localChanged = gcvFALSE;

    while(inst)
    {
        VIR_OpCode opcode = VIR_Inst_GetOpcode(inst);
        gctBOOL singleChange = gcvTRUE;

        do
        {
            if(opcode == VIR_OP_LABEL)
            {
                VIR_Operand* dest = VIR_Inst_GetDest(inst);
                VIR_Label* label = VIR_Operand_GetLabel(dest);
                VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

                if(VIR_Label_GetReference(label) == gcvNULL)
                {
                    /* usused label */
                    VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

                    if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                    {
                        VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                        VIR_LOG(dumper, "remove unused label instruction:\n");
                        VIR_Inst_Dump(dumper, inst);
                    }

                    VIR_Pass_DeleteInstruction(func, inst, &VIR_CFO_GetInvalidCfg(cfo));
                    inst = nextInst;
                    break;
                }
                else if(nextInst)
                {
                    if(VIR_Inst_GetOpcode(nextInst) == VIR_OP_LABEL)
                    {
                        VIR_Label* labelNext = VIR_Operand_GetLabel(VIR_Inst_GetDest(nextInst));
                        VIR_Link* startLink;

                        /* label label1
                           label label2 */
                        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                        {
                            VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                            VIR_LOG(dumper, "found continuous labels:\n");
                            VIR_LOG(dumper, "remove instruction:\n");
                            VIR_Inst_Dump(dumper, nextInst);
                        }
                        startLink = VIR_Label_GetReference(labelNext);
                        if(startLink)
                        {
                            VIR_Link* lastLink = gcvNULL;
                            VIR_Link* link = startLink;

                            while(link)
                            {
                                VIR_Instruction* refInst = (VIR_Instruction*)VIR_Link_GetReference(link);

                                VIR_Operand_SetLabel(VIR_Inst_GetDest(refInst), label);
                                lastLink = link;
                                link = VIR_Link_GetNext(link);
                            }
                            VIR_Link_SetNext(lastLink, VIR_Label_GetReference(label));
                            VIR_Label_SetReference(label, startLink);
                            VIR_Label_SetReference(labelNext, gcvNULL);
                        }
                        VIR_Pass_DeleteInstruction(func, nextInst, &VIR_CFO_GetInvalidCfg(cfo));
                        break;
                    }
                }
            }
            else if(opcode == VIR_OP_JMPC)
            {
                VIR_Label* labelJmpc = VIR_Operand_GetLabel(VIR_Inst_GetDest(inst));
                VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

                if(nextInst)
                {
                    if(VIR_Inst_GetOpcode(nextInst) == VIR_OP_LABEL)
                    {
                        VIR_Label* labelNext = VIR_Operand_GetLabel(VIR_Inst_GetDest(nextInst));

                        if(labelJmpc == labelNext)
                        {
                            /* jmpc labelname, s0, s1
                               label labelname */
                            if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                            {
                                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                VIR_LOG(dumper, "found pattern jmpc-to-next:\n");
                                VIR_LOG(dumper, "remove instruction:\n");
                                VIR_Inst_Dump(dumper, inst);
                            }
                            VIR_Pass_DeleteInstruction(func, inst, &VIR_CFO_GetInvalidCfg(cfo));
                            inst = nextInst;
                            break;
                        }
                    }
                    else if(VIR_Inst_GetOpcode(nextInst) == VIR_OP_JMP)
                    {
                        VIR_Label* labelJmp = VIR_Operand_GetLabel(VIR_Inst_GetDest(nextInst));
                        VIR_Instruction* next2Inst = VIR_Inst_GetNext(nextInst);

                        if(next2Inst &&
                           VIR_Inst_GetOpcode(next2Inst) == VIR_OP_LABEL)
                        {
                            VIR_Label* labelNext2 = VIR_Operand_GetLabel(VIR_Inst_GetDest(next2Inst));

                            if(labelJmpc == labelNext2 &&
                               VIR_ConditionOp_Reversable(VIR_Inst_GetConditionOp(inst)))
                            {
                                VIR_TypeId src0TyId = VIR_Operand_GetTypeId(VIR_Inst_GetSource(inst, 0));
                                VIR_TypeId src1TyId = VIR_Inst_GetSource(inst, 1) ? VIR_Operand_GetTypeId(VIR_Inst_GetSource(inst, 1)) : VIR_TYPE_UNKNOWN;
                                if(VIR_CFO_GetHWCfg(cfo)->hwFeatureFlags.supportUnOrdBranch)
                                {
                                    gcmASSERT(0);
                                }
                                else if((VIR_TypeId_isInteger(src0TyId) &&
                                         (VIR_Inst_GetSource(inst, 1) == gcvNULL || VIR_TypeId_isInteger(src1TyId))) ||
                                        (VIR_TypeId_isFloat(src0TyId) && VIR_GetTypeComponents(src0TyId) == 1 &&
                                         (VIR_Inst_GetSource(inst, 1) == gcvNULL ||
                                          (VIR_TypeId_isFloat(src1TyId) && VIR_GetTypeComponents(src0TyId) == 1))) )
                                {
                                    /* ok to transform scalar float point comparison */
                                    if(VIR_CFO_GetHWCfg(cfo)->hwFeatureFlags.hasHalti4 && VIR_GetTypeComponents(src0TyId) > 1)
                                    {
                                        /* jmpc.cond labelname, int s0, int s1
                                           jmp labelname2
                                           label labelname

                                           ==>

                                           jmp_any.rev_cond labelname2, int s0, int s1 */
                                        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                                        {
                                            VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                            VIR_LOG(dumper, "found pattern: \n");
                                            VIR_LOG(dumper, "jmpc.cond labelname, int s0, int s1\n");
                                            VIR_LOG(dumper, "jmp labelname2\n");
                                            VIR_LOG(dumper, "label labelname\n");
                                            VIR_LOG(dumper, "change jmpc instruction:\n");
                                            VIR_Inst_Dump(dumper, inst);
                                        }
                                        VIR_Inst_SetOpcode(inst, VIR_OP_JMP_ANY);
                                        VIR_Inst_SetConditionOp(inst, VIR_ConditionOp_Reverse(VIR_Inst_GetConditionOp(inst)));
                                        VIR_Inst_ChangeJmpTarget(inst, VIR_Label_GetDefInst(labelJmp));
                                        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                                        {
                                            VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                            VIR_LOG(dumper, "to:\n");
                                            VIR_Inst_Dump(dumper, inst);
                                            VIR_LOG(dumper, "remove instruction:\n");
                                            VIR_Inst_Dump(dumper, nextInst);
                                        }
                                        VIR_Pass_DeleteInstruction(func, nextInst, &VIR_CFO_GetInvalidCfg(cfo));
                                        break;
                                    }
                                    else if((VIR_Swizzle_Channel_Count(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(inst, 0))) == 1 &&
                                        (VIR_Inst_GetSource(inst, 1) == gcvNULL || VIR_Swizzle_Channel_Count(VIR_Operand_GetSwizzle(VIR_Inst_GetSource(inst, 1))) == 1)))
                                    {
                                        /* jmpc.cond labelname, int s0.xxxx, int s1.yyyy
                                           jmp labelname2
                                           label labelname

                                           ==>

                                           jmpc.rev_cond labelname2, int s0.xxxx, int s1.yyyy */
                                        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                                        {
                                            VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                            VIR_LOG(dumper, "found pattern: \n");
                                            VIR_LOG(dumper, "jmpc.cond labelname, int s0.xxxx, int s1.yyyy\n");
                                            VIR_LOG(dumper, "jmp labelname2\n");
                                            VIR_LOG(dumper, "label labelname\n");
                                            VIR_LOG(dumper, "change jmpc instruction:\n");
                                            VIR_Inst_Dump(dumper, inst);
                                        }
                                        VIR_Inst_SetConditionOp(inst, VIR_ConditionOp_Reverse(VIR_Inst_GetConditionOp(inst)));
                                        VIR_Inst_ChangeJmpTarget(inst, VIR_Label_GetDefInst(labelJmp));
                                        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                                        {
                                            VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                            VIR_LOG(dumper, "to:\n");
                                            VIR_Inst_Dump(dumper, inst);
                                            VIR_LOG(dumper, "remove instruction:\n");
                                            VIR_Inst_Dump(dumper, nextInst);
                                        }
                                        VIR_Pass_DeleteInstruction(func, nextInst, &VIR_CFO_GetInvalidCfg(cfo));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else if(opcode == VIR_OP_JMP)
            {
                VIR_Label* labelJmp = VIR_Operand_GetLabel(VIR_Inst_GetDest(inst));
                VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

                if(nextInst)
                {
                    if(VIR_Inst_GetOpcode(nextInst) == VIR_OP_LABEL)
                    {
                        VIR_Label* labelNext = VIR_Operand_GetLabel(VIR_Inst_GetDest(nextInst));

                        if(labelJmp == labelNext)
                        {
                            /* jmpc labelname, s0, s1
                               label labelname */
                            if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                            {
                                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                                VIR_LOG(dumper, "found pattern jmp-to-next:\n");
                                VIR_LOG(dumper, "remove instruction:\n");
                                VIR_Inst_Dump(dumper, inst);
                            }
                            VIR_Pass_DeleteInstruction(func, inst, &VIR_CFO_GetInvalidCfg(cfo));
                            inst = nextInst;
                            break;
                        }
                    }
                }
            }
            else if(opcode == VIR_OP_NOP)
            {
                /* nop */
                VIR_Instruction* nextInst = VIR_Inst_GetNext(inst);

                if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_DETAIL))
                {
                    VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);

                    VIR_LOG(dumper, "found nop:\n");
                    VIR_LOG(dumper, "remove instruction:\n");
                    VIR_Inst_Dump(dumper, inst);
                }
                VIR_Pass_DeleteInstruction(func, inst, &VIR_CFO_GetInvalidCfg(cfo));
                inst = nextInst;
                break;
            }
            singleChange = gcvFALSE;
        } while(gcvFALSE);

        if(!singleChange)
        {
            inst = VIR_Inst_GetNext(inst);
        }
        else
        {
            localChanged = gcvTRUE;
        }
    }

    if(changed)
    {
        *changed = localChanged;
    }

    return errCode;
}

VSC_ErrCode
_VIR_CFO_PerformSelectGenerationOnFunction(
    VIR_CFO* cfo,
    VIR_Function* func,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;

    return errCode;
}

VSC_ErrCode
VIR_CFO_PerformOnFunction(
    VIR_CFO* cfo,
    VIR_Function* func,
    gctBOOL* changed
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VSC_OPTN_CFOOptions* options = VIR_CFO_GetOptions(cfo);
    gctBOOL globalChanged = gcvFALSE;
    gctBOOL localChanged;

    if(VSC_OPTN_CFOOptions_GetOpts(options) == VSC_OPTN_CFOOptions_OPTS_NONE)
    {
        return errCode;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_FUNC_INPUT))
    {
        VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
        VIR_LOG(dumper, "CFO start for function\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
    }

    do
    {
        localChanged = gcvFALSE;

        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetOpts(options), VSC_OPTN_CFOOptions_OPTS_PATTERN))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_INPUT))
            {
                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
                VIR_LOG(dumper, "pattern transformation starts for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
            _VIR_CFO_PerformPatternTransformationOnFunction(cfo, func, &localChanged);
            if(VSC_UTILS_MASK(VSC_OPTN_LoopOptsOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_PATTERN_OUTPUT))
            {
                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
                VIR_LOG(dumper, "pattern transformation ends for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }

            globalChanged |= localChanged;
        }

        if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetOpts(options), VSC_OPTN_CFOOptions_OPTS_GEN_SELECT))
        {
            if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_GEN_SELECT_INPUT))
            {
                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
                VIR_LOG(dumper, "select generation starts for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }
            _VIR_CFO_PerformSelectGenerationOnFunction(cfo, func, &localChanged);
            if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_GEN_SELECT_OUTPUT))
            {
                VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
                VIR_LOG(dumper, "select generation ends for function\n");
                VIR_LOG_FLUSH(dumper);
                VIR_Function_Dump(dumper, func);
            }

            globalChanged |= localChanged;
        }
    } while(localChanged);

    if(changed)
    {
        *changed = globalChanged;
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_FUNC_OUTPUT))
    {
        VIR_Dumper* dumper = VIR_CFO_GetDumper(cfo);
        VIR_LOG(dumper, "CFO end for function\n");
        VIR_LOG_FLUSH(dumper);
        VIR_Function_Dump(dumper, func);
    }

    return errCode;
}

DEF_QUERY_PASS_PROP(VIR_CFO_PerformOnShader)
{
    pPassProp->supportedLevels = VSC_PASS_LEVEL_LL;
    pPassProp->passOptionType = VSC_PASS_OPTN_TYPE_CFO;
    pPassProp->memPoolSel = VSC_PASS_MEMPOOL_SEL_PRIVATE_PMP;
}

DEF_SH_NECESSITY_CHECK(VIR_CFO_PerformOnShader)
{
    return gcvTRUE;
}

VSC_ErrCode
VIR_CFO_PerformOnShader(
    VSC_SH_PASS_WORKER* pPassWorker
    )
{
    VSC_ErrCode errcode  = VSC_ERR_NONE;
    VIR_Shader* shader = (VIR_Shader*)pPassWorker->pCompilerParam->hShader;
    VIR_FuncIterator funcIter;
    VIR_FunctionNode* funcNode;
    VIR_CFO cfo;
    VSC_OPTN_CFOOptions* options = (VSC_OPTN_CFOOptions*)pPassWorker->basePassWorker.pBaseOption;
    gctBOOL globalChanged = gcvFALSE;

    if(!VSC_OPTN_InRange(VIR_Shader_GetId(shader), VSC_OPTN_CFOOptions_GetBeforeShader(options), VSC_OPTN_CFOOptions_GetAfterShader(options)))
    {
        if(VSC_OPTN_CFOOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Control Flow Optimizations(pass %d) skip shader(%d)\n", VSC_OPTN_CFOOptions_GetPassId(options), VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
        return errcode;
    }
    else
    {
        if(VSC_OPTN_CFOOptions_GetTrace(options))
        {
            VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
            VIR_LOG(dumper, "Control Flow Optimizations(pass %d) start for shader(%d)\n", VSC_OPTN_CFOOptions_GetPassId(options), VIR_Shader_GetId(shader));
            VIR_LOG_FLUSH(dumper);
        }
    }

    if(VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_SHADER_INPUT))
    {
        VIR_Shader_Dump(gcvNULL, "Before Control Flow Optimizations.", shader, gcvTRUE);
    }

    VIR_CFO_Init(&cfo, shader, &(pPassWorker->pCompilerParam->cfg.ctx.pSysCtx->pCoreSysCtx->hwCfg), options, pPassWorker->basePassWorker.pDumper, pPassWorker->basePassWorker.pMM);

    VIR_FuncIterator_Init(&funcIter, VIR_Shader_GetFunctions(shader));
    for(funcNode = VIR_FuncIterator_First(&funcIter);
        funcNode != gcvNULL; funcNode = VIR_FuncIterator_Next(&funcIter))
    {
        VIR_Function* func = funcNode->function;

        if(VIR_Function_GetInstCount(func))
        {
            gctBOOL localChanged;

            errcode = VIR_CFO_PerformOnFunction(&cfo, func, &localChanged);
            if(errcode)
            {
                break;
            }
            globalChanged |= localChanged;
        }
    }

    if(globalChanged || VIR_CFO_GetInvalidCfg(&cfo))
    {
        pPassWorker->pResDestroyReq->s.bInvalidateCfg = gcvTRUE;
    }

    VIR_CFO_Final(&cfo);

    if(VSC_OPTN_CFOOptions_GetTrace(options))
    {
        VIR_Dumper* dumper = pPassWorker->basePassWorker.pDumper;
        VIR_LOG(dumper, "Control Flow Optimizations(pass %d) end for shader(%d)\n", VSC_OPTN_CFOOptions_GetPassId(options), VIR_Shader_GetId(shader));
        VIR_LOG_FLUSH(dumper);
    }
    if (VSC_OPTN_DumpOptions_CheckDumpFlag(VIR_Shader_GetDumpOptions(shader), VIR_Shader_GetId(shader), VSC_OPTN_DumpOptions_DUMP_OPT_VERBOSE) ||
        VSC_UTILS_MASK(VSC_OPTN_CFOOptions_GetTrace(options), VSC_OPTN_CFOOptions_TRACE_SHADER_OUTPUT))
    {
        VIR_Shader_Dump(gcvNULL, "After Control Flow Optimizations.", shader, gcvTRUE);
    }

    return errcode;
}

