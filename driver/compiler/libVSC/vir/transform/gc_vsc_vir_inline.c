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


#include "vir/transform/gc_vsc_vir_inline.h"


/* ===========================================================================
   _VSC_IL_UpdateMaxCallDepth:
   Update the func block's max call depth
   ===========================================================================
*/
static void _VSC_IL_UpdateMaxCallDepth(
    VIR_Inliner       *pInliner,
    VIR_FUNC_BLOCK    *pFuncBlk
    )
{
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CG_EDGE*                 pEdge;

    pFuncBlk->maxCallDepth = 0;
    /* go through all its callers */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
    pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
        if (callerBlk->maxCallDepth + 1 > pFuncBlk->maxCallDepth)
        {
            pFuncBlk->maxCallDepth = callerBlk->maxCallDepth + 1;
        }
    }
}

/* ===========================================================================
   _VSC_IL_RemoveFuncBlockFromCallGraph:
   remove the func block from the call graph
   ===========================================================================
*/
static void _VSC_IL_RemoveFuncBlockFromCallGraph(
    VIR_CALL_GRAPH* pCG,
    VIR_FUNC_BLOCK* pFuncBlk)
{
    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CG_EDGE*                 pEdge;

    /* Uninitialize callsite array, note that we only use successors */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.succList);
    pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
    for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
    {
        vscSRARR_Finalize(&pEdge->callSiteArray);
    }

    vscSRARR_Finalize(&pFuncBlk->mixedCallSiteArray);

    /* Remove node from graph */
    vscDG_RemoveNode(&pCG->dgGraph, &pFuncBlk->dgNode);

    /* Remove func from shader */
    VIR_Shader_RemoveFunction(pCG->pOwnerShader, pFuncBlk->pVIRFunc);

    vscDGND_Finalize(&pFuncBlk->dgNode);

    /* Free this node */
    vscMM_Free(&pCG->pmp.mmWrapper, pFuncBlk);
}

/* Duplicate a new instruction in Function based on OrigInst */
VSC_ErrCode
VSC_IL_DupInstruction(
    IN  VIR_Function    *OrigFunction,
    IN  VIR_Function    *Function,
    IN  VIR_Instruction *OrigInst,
    IN  gctUINT         callerIdx,
    OUT VIR_Instruction **Inst,
    OUT VSC_HASH_TABLE  *pLabelSet,
    OUT VSC_HASH_TABLE  *pJmpSet
    )
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;

    VIR_Instruction *inst = (VIR_Instruction *)vscMM_Alloc(
                                               &Function->hostShader->mempool,
                                               sizeof(VIR_Instruction));
    gctUINT srcNum = VIR_OPCODE_GetSrcOperandNum(VIR_Inst_GetOpcode(OrigInst));
    gcmASSERT(srcNum <= VIR_MAX_SRC_NUM);

    *Inst = inst;
    if (inst == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }
    else
    {
        VIR_Operand *dest, *src, *origDest, *origSrc;
        gctUINT i;
        VIR_OpCode opcode;

        memset(inst, 0, sizeof(VIR_Instruction));
        VIR_Inst_SetOpcode(inst, VIR_Inst_GetOpcode(OrigInst));
        VIR_Inst_SetSrcNum(inst, srcNum);
        VIR_Inst_SetInstType(inst, VIR_Inst_GetInstType(OrigInst));
        VIR_Inst_SetConditionOp(inst, VIR_Inst_GetConditionOp(OrigInst));
        VIR_Inst_SetFunction(inst, Function);
        VIR_Inst_SetId(inst, VIR_Function_GetAndIncressLastInstId(Function));

        /* allocate dest operand */
        if (VIR_OPCODE_hasDest(VIR_Inst_GetOpcode(OrigInst)))
        {
            origDest = VIR_Inst_GetDest(OrigInst);
            errCode = VIR_Function_DupOperand(Function, origDest, &dest);
            VIR_Inst_SetDest(inst, dest);
        }

        /* allocate source operand */
        for (i = 0; i < srcNum; i++)
        {
            origSrc = VIR_Inst_GetSource(OrigInst, i);
            errCode = VIR_Function_DupOperand(Function, origSrc, &src);
            inst->src[i] = src;
        }

        /* reset rest source operands */
        for (i = srcNum; i < VIR_MAX_SRC_NUM; i++)
        {
            inst->src[i] = gcvNULL;
        }

        opcode = VIR_Inst_GetOpcode(OrigInst);

        /* special handling for some special instructions */
        if (opcode == VIR_OP_LABEL)
        {
            VIR_Label       *label = gcvNULL;
            VIR_Label       *newLabel = gcvNULL;
            gctUINT offset = 0;
            gctCHAR labelName[512];
            VIR_LabelId labelId;

            label = VIR_Operand_GetLabel(VIR_Inst_GetDest(OrigInst));
            gcmASSERT(label != gcvNULL);
            gcoOS_PrintStrSafe(labelName,
                               gcmSIZEOF(labelName),
                               &offset,
                               "%s_%u_%u",
                               VIR_Shader_GetSymNameString(Function->hostShader,
                               VIR_Function_GetSymbol(OrigFunction)),
                               callerIdx,
                               VIR_Label_GetId(label));

            errCode = VIR_Function_AddLabel(Function,
                                            labelName,
                                            &labelId);
            newLabel = VIR_Function_GetLabelFromId(Function, labelId);
            newLabel->defined = inst;
            VIR_Operand_SetLabel(VIR_Inst_GetDest(inst), newLabel);
            vscHTBL_DirectSet(pLabelSet, (void*) label, (void*) newLabel);
        }
        else if (VIR_OPCODE_isBranch(opcode))
        {
            VIR_Label       *label = VIR_Operand_GetLabel(VIR_Inst_GetDest(OrigInst));
            VIR_Label       *pNewLabel = gcvNULL;
            VIR_Link        *pNewLink     = gcvNULL;

            if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) label, (void **)&pNewLabel))
            {
                VIR_Operand_SetLabel(VIR_Inst_GetDest(inst), pNewLabel);
                VIR_Function_NewLink(Function, &pNewLink);
                VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)inst);
                VIR_Link_AddLink(&(pNewLabel->referenced), pNewLink);
            }
            else
            {
                /* we need to save the unchanged jmp into a list, its label willl be changed
                   at the end */
                vscHTBL_DirectSet(pJmpSet, (void*) inst, gcvNULL);
            }
        }
    }

    return errCode;
}

/* ===========================================================================
   VSC_IL_InlineSingleFunction:
   Inline a functon to its caller
   ===========================================================================
*/
VSC_ErrCode VSC_IL_InlineSingleFunction(
    VIR_Inliner       *pInliner,
    VIR_Function      *pCallerFunc,
    VIR_Function      *pCalleeFunc)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);
    VIR_FUNC_BLOCK      *pCallerBLK = VIR_Function_GetFuncBlock(pCallerFunc);
    VIR_FUNC_BLOCK      *pCalleeBLK = VIR_Function_GetFuncBlock(pCalleeFunc);

    /* save the mapping between the old label id with the new one*/
    VSC_HASH_TABLE       *pLabelSet;

    /* save the jmp instruction whose label is not set the new
       one during DupInstruction, we need to set its label at the end */
    VSC_HASH_TABLE       *pJmpSet;

    VSC_IL_INST_LIST     calleeInsts;

    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;
    gctUINT                      callerIdx;

    VIR_Instruction     *pInst;
    VIR_InstIterator    instIter;

    INST_LIST_INITIALIZE(&calleeInsts);

    /* add the callee instructions into a list */
    VIR_InstIterator_Init(&instIter, &pCalleeFunc->instList);
    pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    for (; pInst != gcvNULL;
        pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        VSC_IL_INST_LIST_NODE   *node =
            (VSC_IL_INST_LIST_NODE*)vscMM_Alloc(VSC_IL_GetMM(pInliner),
            sizeof(VSC_IL_INST_LIST_NODE));
        node->inst = pInst;
        INST_LIST_ADD_NODE(&calleeInsts, node);
    }


    pLabelSet = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                (PFN_VSC_HASH_FUNC)vscHFUNC_Label, (PFN_VSC_KEY_CMP)vcsHKCMP_Label, 512);

    pJmpSet = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                vscHFUNC_Default, vscHKCMP_Default, 512);

    /* go through all the caller to find the right one */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pCallerBLK->dgNode.succList);
    pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
    for (; pCallerEdge != gcvNULL;
        pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
    {
        if (CG_EDGE_GET_TO_FB(pCallerEdge) == pCalleeBLK)
        {
            /* for all the call sites */
            for (callerIdx = 0;
                callerIdx < vscSRARR_GetElementCount(&pCallerEdge->callSiteArray);
                callerIdx ++)
            {
                VIR_Instruction *pCallSiteInst =
                    *(VIR_Instruction**) vscSRARR_GetElement(&pCallerEdge->callSiteArray, callerIdx);

                VSC_IL_INST_LIST_ITERATOR calleeInstsIter;
                VSC_IL_INST_LIST_NODE     *calleeInstsNode;

                VSC_HASH_ITERATOR jmpSetIter;
                VSC_DIRECT_HNODE_PAIR jmpSetPair;
                VIR_Label       *callLabel = gcvNULL;

                vscHTBL_Reset(pLabelSet);
                vscHTBL_Reset(pJmpSet);

                /* change the call instruction to a LABEL instruction */
                if (VIR_Inst_GetOpcode(pCallSiteInst) == VIR_OP_CALL)
                {
                    gctUINT offset = 0;
                    gctCHAR labelName[512];
                    VIR_LabelId labelId;

                    VIR_Inst_SetOpcode(pCallSiteInst, VIR_OP_LABEL);

                    gcoOS_PrintStrSafe(labelName,
                                       gcmSIZEOF(labelName),
                                       &offset,
                                       "%s_%s_%u",
                                       VIR_Shader_GetSymNameString(pCallerFunc->hostShader,
                                       VIR_Function_GetSymbol(pCallerFunc)),
                                       VIR_Shader_GetSymNameString(pCallerFunc->hostShader,
                                       VIR_Function_GetSymbol(pCalleeFunc)),
                                       callerIdx);

                    retValue = VIR_Function_AddLabel(pCallerFunc,
                                                    labelName,
                                                    &labelId);
                    callLabel = VIR_Function_GetLabelFromId(pCallerFunc, labelId);
                    callLabel->defined = pCallSiteInst;
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(pCallSiteInst), callLabel);
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                }

                /* go through all the instructions in the callee,
                   except the last instruction, which should be RET */
                VSC_IL_INST_LIST_ITERATOR_INIT(&calleeInstsIter, &calleeInsts);
                calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_LAST(&calleeInstsIter);
                gcmASSERT (VIR_Inst_GetOpcode(calleeInstsNode->inst) == VIR_OP_RET);

                for(calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_FIRST(&calleeInstsIter);
                    calleeInstsNode != VSC_IL_INST_LIST_ITERATOR_LAST(&calleeInstsIter);
                    calleeInstsNode = VSC_IL_INST_LIST_ITERATOR_NEXT(&calleeInstsIter))
                {
                    VIR_Instruction *pNewInst = gcvNULL;
                    VIR_Link        *pNewLink = gcvNULL;

                    pInst = calleeInstsNode->inst;

                    /* change the RET instruction to a JMP to LABEL instruction
                       that is coming from the call instruction */
                    if(VIR_Inst_GetOpcode(calleeInstsNode->inst) == VIR_OP_RET)
                    {
                        retValue = VIR_Function_NewInstruction(pCallerFunc,
                            VIR_OP_JMP, VIR_TYPE_FLOAT32, &pNewInst);
                        gcmASSERT(callLabel != gcvNULL);
                        VIR_Operand_SetLabel(VIR_Inst_GetDest(pNewInst), callLabel);
                        VIR_Function_NewLink(pCallerFunc, &pNewLink);
                        VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)pNewInst);
                        VIR_Link_AddLink(&(callLabel->referenced), pNewLink);
                    }
                    else
                    {
                        retValue = VSC_IL_DupInstruction(pCalleeFunc, pCallerFunc,
                            pInst, callerIdx, &pNewInst, pLabelSet, pJmpSet);
                    }

                    vscBILST_InsertBefore((VSC_BI_LIST *)&pCallerFunc->instList,
                             (VSC_BI_LIST_NODE *) pCallSiteInst,
                             (VSC_BI_LIST_NODE *) pNewInst);
                }

                /* set the label for the jmp instruction */
                vscHTBLIterator_Init(&jmpSetIter, pJmpSet);
                for(jmpSetPair = vscHTBLIterator_DirectFirst(&jmpSetIter);
                    IS_VALID_DIRECT_HNODE_PAIR(&jmpSetPair); jmpSetPair = vscHTBLIterator_DirectNext(&jmpSetIter))
                {
                    VIR_Instruction* jmpInst = (VIR_Instruction*)VSC_DIRECT_HNODE_PAIR_FIRST(&jmpSetPair);
                    VIR_Label       *label = VIR_Operand_GetLabel(VIR_Inst_GetDest(jmpInst));
                    VIR_Label       *newLabel = gcvNULL;
                    VIR_Link        *pNewLink = gcvNULL;

                    if (vscHTBL_DirectTestAndGet(pLabelSet, (void*) label, (void **)&newLabel) != gcvTRUE)
                    {
                        gcmASSERT(gcvFALSE);
                    }
                    VIR_Operand_SetLabel(VIR_Inst_GetDest(jmpInst), newLabel);
                    VIR_Function_NewLink(pCallerFunc, &pNewLink);
                    VIR_Link_SetReference(pNewLink, (gctUINTPTR_T)jmpInst);
                    VIR_Link_AddLink(&(newLabel->referenced), pNewLink);
                }
            }
        }
    }

    /* update the call graph accordingly */
    vscDG_RemoveEdge((VSC_DIRECTED_GRAPH*)pCG,
        (VSC_DG_NODE*) VIR_Function_GetFuncBlock(pCallerFunc),
        (VSC_DG_NODE*) VIR_Function_GetFuncBlock(pCalleeFunc));

    INST_LIST_FINALIZE(&calleeInsts);
    vscHTBL_Destroy(pLabelSet);
    vscHTBL_Destroy(pJmpSet);

    /* dump */
    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE))
    {
        VIR_LOG(pDumper, "Caller [%s] after inlining callee [%s]\n\n",
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pCallerFunc)),
            VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pCalleeFunc)));
        VIR_Function_Dump(pDumper, pCallerFunc);
        VIR_LOG_FLUSH(pDumper);
    }

    return retValue;
}

/* ===========================================================================
   VSC_IL_SelectInlineFunctions:
   Select Inline functon candidates
   ===========================================================================
*/
VSC_ErrCode VSC_IL_SelectInlineFunctions(
    VIR_Inliner       *pInliner,
    VIR_Function      *pFunc
    )
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VSC_HASH_TABLE      *pCandidates = VSC_IL_GetCandidates(pInliner);
    VIR_FUNC_BLOCK      *pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
    gctINT              instCount = VIR_Function_GetInstCount(pFunc);
    gctINT              callSites = 0;

    VSC_ADJACENT_LIST_ITERATOR   edgeIter;
    VIR_CG_EDGE*                 pEdge;

    if (pFunc == CG_GET_MAIN_FUNC(pCG))
    {
        /* main func */
        VSC_IL_SetInlineBudget(pInliner,
                VSC_IL_GetInlineBudget(pInliner) - instCount);

    }
    else
    {
        /* go through all its callers */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
        pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
        for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
        {
            /* callsite only stores at the succ edge */
            pEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pEdge);
            callSites += vscSRARR_GetElementCount(&pEdge->callSiteArray);
        }
        instCount  = instCount * callSites;

        /* only use the code size as the heuristic for now */
        if (VSC_IL_GetInlineBudget(pInliner) - instCount > 0)
        {
            vscHTBL_DirectSet(pCandidates, (void*) pFunc, gcvNULL);

            VSC_IL_SetInlineBudget(pInliner,
                VSC_IL_GetInlineBudget(pInliner) - instCount);
        }
    }

    return retValue;
}

/* ===========================================================================
   VSC_IL_OptimizeCallStackDepth:
   Inline functon that are exceed the max call stack depth.
   ===========================================================================
*/
VSC_ErrCode VSC_IL_OptimizeCallStackDepth(
    VIR_Inliner       *pInliner)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);

    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;
    VIR_FUNC_BLOCK      *pFuncBlk;

    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);

    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(VSC_IL_GetMM(pInliner),
        sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);

    /* bottom up traverse of the call graph */
    vscDG_PstOrderTraversal(&pCG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        VSC_ADJACENT_LIST_ITERATOR   edgeIter;
        VIR_CG_EDGE*                 pEdge;

        pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
        pFuncBlk = VIR_Function_GetFuncBlock(pFunc);

        while (pFuncBlk->maxCallDepth > VSC_MAX_CALL_STACK_DEPTH)
        {
            /* dump */
            if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                VSC_OPTN_ILOptions_TRACE))
            {
                VIR_LOG(pDumper, "\nOptimize Call Stack Depth for Function:\t[%s] \n",
                    VIR_Shader_GetSymNameString(pShader,
                    VIR_Function_GetSymbol(pFunc)));
                VIR_LOG_FLUSH(pDumper);
            }

            /* go through all its callers */
            VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
            pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
            for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
            {
                VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
                if (callerBlk->maxCallDepth == pFuncBlk->maxCallDepth - 1)
                {
                    retValue = VSC_IL_InlineSingleFunction(pInliner, callerBlk->pVIRFunc, pFunc);
                }
            }

            _VSC_IL_UpdateMaxCallDepth(pInliner, pFuncBlk);

            if (pFuncBlk->maxCallDepth == 0)
            {
                /* remove this function block from the call graph */
                _VSC_IL_RemoveFuncBlockFromCallGraph(pCG, pFuncBlk);
            }
        }
    }

    vscMM_Free(VSC_IL_GetMM(pInliner), ppFuncBlkRPO);

    return retValue;
}

/* ===========================================================================
   VSC_IL_TopDownInline:
   top down traverse the call graph to select the inline candidates and
   perform the inline
   ===========================================================================
*/
VSC_ErrCode VSC_IL_TopDownInline(
    VIR_Inliner       *pInliner)
{

    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);
    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);

    VIR_FUNC_BLOCK      **ppFuncBlkRPO;
    gctUINT             funcIdx;
    VIR_Function        *pFunc;

    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(VSC_IL_GetMM(pInliner),
        sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);

    vscDG_PstOrderTraversal(&pCG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    /* Seperate the hueristic with the transformation */

    /* 1. select the inline candidates into the worklist */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
        if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
            VSC_OPTN_ILOptions_TRACE))
        {
            VIR_LOG(pDumper, "\nSelect Inline Candidate for Function:\t[%s]\n",
                VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
            VIR_LOG_FLUSH(pDumper);
        }
        VSC_IL_SelectInlineFunctions(pInliner, pFunc);
    }

    /* 2. do the inline transformation */
    {
        VSC_HASH_TABLE        *pCandidates = VSC_IL_GetCandidates(pInliner);
        VSC_ADJACENT_LIST_ITERATOR   edgeIter;
        VIR_CG_EDGE*                 pEdge;

        /* from bottom up to perform inline */
        vscDG_PstOrderTraversal(&pCG->dgGraph,
                                VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                                gcvTRUE,
                                gcvTRUE,
                                (VSC_DG_NODE**)ppFuncBlkRPO);

        /* 1. select the inline candidates into the worklist */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            VIR_Function *pFunc = ppFuncBlkRPO[funcIdx]->pVIRFunc;
            VIR_FUNC_BLOCK  *pFuncBlk = gcvNULL;
            if (vscHTBL_DirectTestAndGet(pCandidates, (void*) pFunc, gcvNULL))
            {
                pFuncBlk = VIR_Function_GetFuncBlock(pFunc);
                if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
                    VSC_OPTN_ILOptions_TRACE))
                {
                    VIR_LOG(pDumper, "\nPerform Inline for Function:\t[%s]\n",
                        VIR_Shader_GetSymNameString(pShader, VIR_Function_GetSymbol(pFunc)));
                    VIR_LOG_FLUSH(pDumper);
                }

                /* go through all its callers */
                VSC_ADJACENT_LIST_ITERATOR_INIT(&edgeIter, &pFuncBlk->dgNode.predList);
                pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&edgeIter);
                for (; pEdge != gcvNULL; pEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&edgeIter))
                {
                    VIR_FUNC_BLOCK *callerBlk = CG_EDGE_GET_TO_FB(pEdge);
                    retValue = VSC_IL_InlineSingleFunction(pInliner, callerBlk->pVIRFunc, pFunc);
                }

                _VSC_IL_UpdateMaxCallDepth(pInliner, pFuncBlk);

                if (pFuncBlk->maxCallDepth == 0)
                {
                    /* remove this function block from the call graph */
                    _VSC_IL_RemoveFuncBlockFromCallGraph(pCG, pFuncBlk);
                }
            }
        }
    }

    vscMM_Free(VSC_IL_GetMM(pInliner), ppFuncBlkRPO);

    return retValue;
}

void VSC_IL_Init(
    VIR_Inliner         *pInliner,
    VIR_Shader          *pShader,
    VSC_HW_CONFIG       *pHwCfg,
    VSC_OPTN_ILOptions  *pOptions,
    VIR_Dumper          *pDumper,
    VIR_CALL_GRAPH      *pCG)
{
    VSC_IL_SetShader(pInliner, pShader);
    VSC_IL_SetHwCfg(pInliner, pHwCfg);
    VSC_IL_SetDumper(pInliner, pDumper);
    VSC_IL_SetOptions(pInliner, pOptions);
    VSC_IL_SetCallGraph(pInliner, pCG);

    /* initialize the memory pool */
    vscPMP_Intialize(VSC_IL_GetPmp(pInliner), gcvNULL,
        VSC_IL_MEM_BLK_SIZE, sizeof(void*), gcvTRUE);

    pInliner->pCandidates = vscHTBL_Create(VSC_IL_GetMM(pInliner),
                vscHFUNC_Default, vscHKCMP_Default, 512);

    VSC_IL_SetInlineBudget(pInliner, pHwCfg->maxTotalInstCount);
}

void VSC_IL_Final(
    VIR_Inliner           *pInliner)
{

    VSC_IL_SetShader(pInliner, gcvNULL);
    VSC_IL_SetOptions(pInliner, gcvNULL);
    VSC_IL_SetDumper(pInliner, gcvNULL);
    VSC_IL_SetCallGraph(pInliner, gcvNULL);

    vscPMP_Finalize(VSC_IL_GetPmp(pInliner));
}

/* ===========================================================================
   VSC_IL_PerformOnShader:
   inliner on shader
   ===========================================================================
*/
VSC_ErrCode VSC_IL_PerformOnShader(
    VIR_Inliner       *pInliner)
{
    VSC_ErrCode         retValue  = VSC_ERR_NONE;
    VIR_Shader          *pShader = VSC_IL_GetShader(pInliner);
    VIR_CALL_GRAPH      *pCG = VSC_IL_GetCallGraph(pInliner);
    VIR_Dumper          *pDumper = VSC_IL_GetDumper(pInliner);
    VSC_OPTN_ILOptions  *pOption = VSC_IL_GetOptions(pInliner);
    gctUINT             countOfFuncBlk = vscDG_GetNodeCount(&pCG->dgGraph);

    /* dump */
    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE))
    {
        VIR_Shader_Dump(gcvNULL, "Shader before Inliner", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    if (countOfFuncBlk != 0)
    {
        /* inline functons that are exceed the max call stack depth*/
        if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetHeuristics(pOption),
            VSC_OPTN_ILOptions_CALL_DEPTH))
        {
            retValue = VSC_IL_OptimizeCallStackDepth(pInliner);
        }

        /* top down traverse the call graph */
        if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetHeuristics(pOption),
            VSC_OPTN_ILOptions_TOP_DOWN))
        {
            retValue = VSC_IL_TopDownInline(pInliner);

        }
    }

    if (VSC_UTILS_MASK(VSC_OPTN_ILOptions_GetTrace(pOption),
        VSC_OPTN_ILOptions_TRACE) || gcSHADER_DumpCodeGenVerbose(pShader))
    {
        VIR_Shader_Dump(gcvNULL, "Shader after Inliner", pShader, gcvTRUE);
        VIR_LOG_FLUSH(pDumper);
    }

    return retValue;
}

