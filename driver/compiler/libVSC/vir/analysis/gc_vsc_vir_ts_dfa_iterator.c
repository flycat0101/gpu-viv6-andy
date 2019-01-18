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

void vscVIR_InitializeBaseDFA(VIR_BASE_DFA* pBaseDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                              gctINT flowSize, VSC_MM* pMM, VSC_MM* pScratchMemPool)
{
    pBaseDFA->dfaType = dfaType;
    pBaseDFA->flowSize = flowSize;
    pBaseDFA->cmnDfaFlags.bFlowBuilt = gcvFALSE;
    pBaseDFA->cmnDfaFlags.bFlowInvalidated = gcvFALSE;
    pBaseDFA->pOwnerCG = pCg;
    pBaseDFA->pMM = pMM;
    pBaseDFA->pScratchMemPool = pScratchMemPool;
}

void vscVIR_FinalizeBaseDFA(VIR_BASE_DFA* pBaseDFA)
{
    /* Finalization must mean the dfa flow becomes invalid now */
    pBaseDFA->cmnDfaFlags.bFlowBuilt = gcvFALSE;

    pBaseDFA->cmnDfaFlags.bFlowInvalidated = gcvFALSE;
}

void vscVIR_UpdateBaseDFAFlowSize(VIR_BASE_DFA* pBaseDFA, gctINT newFlowSize)
{
    pBaseDFA->flowSize = newFlowSize;
}

void vscVIR_SetDFAFlowBuilt(VIR_BASE_DFA* pBaseDFA, gctBOOL bFlowBuilt)
{
    pBaseDFA->cmnDfaFlags.bFlowBuilt = bFlowBuilt;
}

gctBOOL vscVIR_CheckDFAFlowBuilt(VIR_BASE_DFA* pBaseDFA)
{
    return pBaseDFA->cmnDfaFlags.bFlowBuilt;
}

void vscVIR_InitializeTsBlockFlow(VIR_TS_BLOCK_FLOW* pTsBlkFlow, VIR_BASIC_BLOCK* pOwnerBB, VSC_MM* pMM, gctINT flowSize)
{
    pTsBlkFlow->pOwnerBB = pOwnerBB;

    pOwnerBB->pTsWorkDataFlow = pTsBlkFlow;
    pOwnerBB->bInWorklist = gcvFALSE;

    vscBV_Initialize(&pTsBlkFlow->genFlow, pMM, flowSize);
    vscBV_Initialize(&pTsBlkFlow->killFlow, pMM, flowSize);
    vscBV_Initialize(&pTsBlkFlow->inFlow, pMM, flowSize);
    vscBV_Initialize(&pTsBlkFlow->outFlow, pMM, flowSize);
}

void vscVIR_UpdateTsBlockFlowSize(VIR_TS_BLOCK_FLOW* pTsBlkFlow, gctINT newFlowSize)
{
    vscBV_Resize(&pTsBlkFlow->genFlow, newFlowSize, gcvTRUE);
    vscBV_Resize(&pTsBlkFlow->killFlow, newFlowSize, gcvTRUE);
    vscBV_Resize(&pTsBlkFlow->inFlow, newFlowSize, gcvTRUE);
    vscBV_Resize(&pTsBlkFlow->outFlow, newFlowSize, gcvTRUE);
}

void vscVIR_FinalizeTsBlockFlow(VIR_TS_BLOCK_FLOW* pTsBlkFlow)
{
    vscBV_Finalize(&pTsBlkFlow->genFlow);
    vscBV_Finalize(&pTsBlkFlow->killFlow);
    vscBV_Finalize(&pTsBlkFlow->inFlow);
    vscBV_Finalize(&pTsBlkFlow->outFlow);

    pTsBlkFlow->pOwnerBB->pTsWorkDataFlow = gcvNULL;
}

void vscVIR_InitializeTsFuncFlow(VIR_TS_FUNC_FLOW* pTsFuncFlow, VIR_FUNC_BLOCK* pOwnerFB, VSC_MM* pMM, gctINT flowSize)
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    pTsFuncFlow->pOwnerFB = pOwnerFB;

    vscBV_Initialize(&pTsFuncFlow->inFlow, pMM, flowSize);
    vscBV_Initialize(&pTsFuncFlow->outFlow, pMM, flowSize);

    /* Intialize block flow array. Note as iterative analyzer will use id of basicblk to index
       block-flow, history node count will be used to allocate these contents */
    vscSRARR_Initialize(&pTsFuncFlow->tsBlkFlowArray,
                        pMM,
                        vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph),
                        sizeof(VIR_TS_BLOCK_FLOW),
                        gcvNULL);

    /* Directly mark all elements are used because we control index of array by ourself (i.e
       id of graph node) */
    vscSRARR_SetElementCount(&pTsFuncFlow->tsBlkFlowArray, vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph));

    CFG_ITERATOR_INIT(&basicBlkIter, &pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_TS_BLOCK_FLOW* pBlkFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pTsFuncFlow->tsBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        vscVIR_InitializeTsBlockFlow(pBlkFlow, pBasicBlk, pMM, flowSize);
    }
}

void vscVIR_UpdateTsFuncFlowSize(VIR_TS_FUNC_FLOW* pTsFuncFlow, gctINT newFlowSize)
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    vscBV_Resize(&pTsFuncFlow->inFlow, newFlowSize, gcvTRUE);
    vscBV_Resize(&pTsFuncFlow->outFlow, newFlowSize, gcvTRUE);

    CFG_ITERATOR_INIT(&basicBlkIter, &pTsFuncFlow->pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_TS_BLOCK_FLOW* pBlkFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pTsFuncFlow->tsBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        vscVIR_UpdateTsBlockFlowSize(pBlkFlow, newFlowSize);
    }
}

void vscVIR_FinalizeTsFuncFlow(VIR_TS_FUNC_FLOW* pTsFuncFlow)
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    vscBV_Finalize(&pTsFuncFlow->inFlow);
    vscBV_Finalize(&pTsFuncFlow->outFlow);

    CFG_ITERATOR_INIT(&basicBlkIter, &pTsFuncFlow->pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_TS_BLOCK_FLOW* pBlkFlow = (VIR_TS_BLOCK_FLOW*)vscSRARR_GetElement(&pTsFuncFlow->tsBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        if (pBlkFlow)
        {
            vscVIR_FinalizeTsBlockFlow(pBlkFlow);
        }
    }

    vscSRARR_Finalize(&pTsFuncFlow->tsBlkFlowArray);
}

void vscVIR_InitializeBaseTsDFA(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                                gctINT flowSize, VSC_MM* pMM, VIR_TS_DFA_RESOLVERS* pTsDfaResolvers)
{
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    vscVIR_InitializeBaseDFA(&pBaseTsDFA->baseDFA, pCg, dfaType, flowSize, pMM, pCg->pScratchMemPool);

    /* Set DFA resolvers */
    pBaseTsDFA->tsDfaResolvers.ts_combineBlockFlow_resolver = pTsDfaResolvers->ts_combineBlockFlow_resolver;
    pBaseTsDFA->tsDfaResolvers.ts_iterateBlockFlow_resolver = pTsDfaResolvers->ts_iterateBlockFlow_resolver;
    pBaseTsDFA->tsDfaResolvers.ts_initBlockFlow_resolver = pTsDfaResolvers->ts_initBlockFlow_resolver;
    pBaseTsDFA->tsDfaResolvers.ts_localGenKill_resolver = pTsDfaResolvers->ts_localGenKill_resolver;
    pBaseTsDFA->tsDfaResolvers.ts_combineBlockFlowFromCallee_resolver = pTsDfaResolvers->ts_combineBlockFlowFromCallee_resolver;
    pBaseTsDFA->tsDfaResolvers.ts_combineFuncFlowFromCallers_resolver = pTsDfaResolvers->ts_combineFuncFlowFromCallers_resolver;

    /* Initialize func-flow array. Note as iterative analyzer will use id of funcblk to index
       func-flow, history node count will be used to allocate these contents */
    vscSRARR_Initialize(&pBaseTsDFA->tsFuncFlowArray,
                        pMM,
                        vscDG_GetHistNodeCount(&pCg->dgGraph),
                        sizeof(VIR_TS_FUNC_FLOW),
                        gcvNULL);

    /* Directly mark all elements are used because we control index of array by ourself (i.e
       id of graph node) */
    vscSRARR_SetElementCount(&pBaseTsDFA->tsFuncFlowArray, vscDG_GetHistNodeCount(&pCg->dgGraph));

    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_TS_FUNC_FLOW* pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        vscVIR_InitializeTsFuncFlow(pFuncFlow, pFuncBlk, pMM, flowSize);
    }
}

void vscVIR_UpdateBaseTsDFAFlowSize(VIR_BASE_TS_DFA* pBaseTsDFA, gctINT newFlowSize)
{
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    vscVIR_UpdateBaseDFAFlowSize(&pBaseTsDFA->baseDFA, newFlowSize);

    CG_ITERATOR_INIT(&funcBlkIter, pBaseTsDFA->baseDFA.pOwnerCG);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_TS_FUNC_FLOW* pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        vscVIR_UpdateTsFuncFlowSize(pFuncFlow, newFlowSize);
    }
}

void vscVIR_FinalizeBaseTsDFA(VIR_BASE_TS_DFA* pBaseTsDFA)
{
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    CG_ITERATOR_INIT(&funcBlkIter, pBaseTsDFA->baseDFA.pOwnerCG);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_TS_FUNC_FLOW* pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        vscVIR_FinalizeTsFuncFlow(pFuncFlow);
    }

    vscSRARR_Finalize(&pBaseTsDFA->tsFuncFlowArray);

    vscVIR_FinalizeBaseDFA(&pBaseTsDFA->baseDFA);
}

void vscVIR_UpdateTsFlow(VSC_BIT_VECTOR* pTsFlow, VSC_BIT_VECTOR* pDeltaTsFlow, gctBOOL bClearFlow)
{
    if (bClearFlow)
    {
        vscBV_Not(pTsFlow, pDeltaTsFlow);
    }
    else
    {
        vscBV_Or1(pTsFlow, pDeltaTsFlow);
    }
}

static VSC_ErrCode _InitializeForwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                           VIR_BASE_TS_DFA* pTsDFA,
                                                           gctBOOL bIPA,
                                                           VIR_BB_WORKITEM** ppWorkItemArray,
                                                           VIR_BB_WORKLIST* pWorkItemList,
                                                           VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK**            ppBasicBlkRPO;
    VIR_CONTROL_FLOW_GRAPH*      pCFG = &pFuncBlk->cfg;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    gctUINT                      bbIdx;
    VIR_BB_WORKITEM*             pThisWorkItemArray;
    VIR_BB_WORKLIST*             pThisWorkItemList;
    VIR_TS_FUNC_FLOW*            pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pTsDFA->tsFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bMainFunc = (CG_GET_MAIN_FUNC(pFuncBlk->pOwnerCG) == pFuncBlk->pVIRFunc);

    ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    BB_WORKLIST_INIT(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of CFG */
    pppBasicBlkRPO[pFuncBlk->dgNode.id] = (VIR_BASIC_BLOCK**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    vscDG_PstOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)pppBasicBlkRPO[pFuncBlk->dgNode.id]);

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    ppWorkItemArray[pFuncBlk->dgNode.id] = (VIR_BB_WORKITEM*)vscMM_Alloc(pTsDFA->baseDFA.pMM,
                                                                         sizeof(VIR_BB_WORKITEM)*
                                                                         vscDG_GetHistNodeCount(&pCFG->dgGraph));

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];
    ppBasicBlkRPO = pppBasicBlkRPO[pFuncBlk->dgNode.id];

    /*  Do local analyze to resolve local gen/kill and initialize inFlow or/and outFlow.
        Also initialize workitem list */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        pTsDFA->tsDfaResolvers.ts_localGenKill_resolver(pTsDFA, ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow);
        pTsDFA->tsDfaResolvers.ts_initBlockFlow_resolver(pTsDFA, ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow);

        /* Initialize each workitem, and add it to workitem list. Note that entry block won't be
           added into workitem list because it won't be changed when iterating */
        if (bIPA && bMainFunc &&
            ppBasicBlkRPO[bbIdx]->flowType == VIR_FLOW_TYPE_ENTRY)
        {
            vscBV_Copy(&pFuncFlow->inFlow, &ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow->inFlow);
        }
        else
        {
            vscBBWKL_AddBBToWorkItemList(pThisWorkItemList,
                                         &pThisWorkItemArray[ppBasicBlkRPO[bbIdx]->dgNode.id],
                                         ppBasicBlkRPO[bbIdx]);
        }
    }

    return errCode;
}

static VSC_ErrCode _DoForwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                   VIR_BASE_TS_DFA* pTsDFA,
                                                   gctBOOL bIPA,
                                                   VIR_BB_WORKITEM** ppWorkItemArray,
                                                   VIR_BB_WORKLIST* pWorkItemList,
                                                   VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH*      pCFG = &pFuncBlk->cfg;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter, succEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_CFG_EDGE*                pSuccEdge;
    VIR_BB_WORKITEM*             pThisWorkItemArray;
    VIR_BB_WORKLIST*             pThisWorkItemList;
    VIR_TS_FUNC_FLOW*            pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pTsDFA->tsFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bInFlowOfEntryChanged;
    gctUINT                      bbIdx, callerIdx, countOfBasicBlkInCallee;
    VIR_BASIC_BLOCK**            ppCalleeBasicBlkRPO;
    VIR_FUNC_BLOCK*              pCallee;
    VIR_BB_WORKITEM*             pCalleeWorkItemArray;
    VIR_BB_WORKLIST*             pCalleeWorkItemList;
    VIR_BASIC_BLOCK*             pCalleeBasicBlk;
    VIR_FUNC_BLOCK*              pCaller;
    VIR_BB_WORKITEM*             pCallerWorkItemArray;
    VIR_BB_WORKLIST*             pCallerWorkItemList;
    VIR_BASIC_BLOCK*             pCallerBasicBlk;
    VIR_Instruction*             pCallSiteInst;
    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];

    /* Do core global iterative analyze by workitem list */
    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(pThisWorkItemList);
        bInFlowOfEntryChanged = gcvFALSE;

        if (bIPA)
        {
            if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_ENTRY)
            {
                bInFlowOfEntryChanged = pTsDFA->tsDfaResolvers.ts_combineFuncFlowFromCallers_resolver(pTsDFA, pFuncFlow);
                vscBV_Copy(&pThisBasicBlk->pTsWorkDataFlow->inFlow, &pFuncFlow->inFlow);
            }
        }

        /* Iterative resolve each predecessor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
            pTsDFA->tsDfaResolvers.ts_iterateBlockFlow_resolver(pTsDFA, pPredBasicBlk->pTsWorkDataFlow);

            if (bIPA)
            {
                if (pPredBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pTsDFA->tsDfaResolvers.ts_combineBlockFlowFromCallee_resolver(pTsDFA, pPredBasicBlk->pTsWorkDataFlow);
                }
            }
        }

        /* Check combination of predecessors to get inFlow of this bb. If there is a change, then add all
           successors that are not in workitem list to workitem list */
        if ((bIPA && bInFlowOfEntryChanged) ||
            pTsDFA->tsDfaResolvers.ts_combineBlockFlow_resolver(pTsDFA, pThisBasicBlk->pTsWorkDataFlow))
        {
            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
            pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
            for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
            {
                pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
                if (!pSuccBasicBlk->bInWorklist)
                {
                    vscBBWKL_AddBBToWorkItemList(pThisWorkItemList, &pThisWorkItemArray[pSuccBasicBlk->dgNode.id], pSuccBasicBlk);
                }
            }

            if (bIPA)
            {
                if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pCallee = VIR_Inst_GetCallee(pThisBasicBlk->pStartInst)->pFuncBlock;
                    ppCalleeBasicBlkRPO = pppBasicBlkRPO[pCallee->dgNode.id];
                    pCalleeWorkItemArray = ppWorkItemArray[pCallee->dgNode.id];
                    pCalleeWorkItemList = &pWorkItemList[pCallee->dgNode.id];
                    countOfBasicBlkInCallee = vscDG_GetNodeCount(&pCallee->cfg.dgGraph);

                    for (bbIdx = 0; bbIdx < countOfBasicBlkInCallee; bbIdx ++)
                    {
                        pCalleeBasicBlk = ppCalleeBasicBlkRPO[bbIdx];

                        if (!pCalleeBasicBlk->bInWorklist)
                        {
                            vscBBWKL_AddBBToWorkItemList(pCalleeWorkItemList, &pCalleeWorkItemArray[pCalleeBasicBlk->dgNode.id], pCalleeBasicBlk);
                        }
                    }
                }
            }

            if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_EXIT)
            {
                pTsDFA->tsDfaResolvers.ts_iterateBlockFlow_resolver(pTsDFA, pThisBasicBlk->pTsWorkDataFlow);
                vscBV_Copy(&pFuncFlow->outFlow, &pThisBasicBlk->pTsWorkDataFlow->outFlow);

                if (bIPA)
                {
                    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pFuncBlk->dgNode.predList);
                    pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
                    for (; pCallerEdge != gcvNULL; pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
                    {
                        /* Call site info is only stored at successor edge */
                        pCallerEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pCallerEdge);

                        for (callerIdx = 0; callerIdx < vscSRARR_GetElementCount(&pCallerEdge->callSiteArray); callerIdx ++)
                        {
                            pCallSiteInst = *(VIR_Instruction**)vscSRARR_GetElement(&pCallerEdge->callSiteArray, callerIdx);
                            pCallerBasicBlk = VIR_Inst_GetBasicBlock(pCallSiteInst);

                            if (pCallerBasicBlk == gcvNULL)
                            {
                                gcmASSERT(gcvFALSE);
                                continue;
                            }
                            else
                            {
                                pCaller = pCallerBasicBlk->pOwnerCFG->pOwnerFuncBlk;
                            }

                            pCallerWorkItemArray = ppWorkItemArray[pCaller->dgNode.id];
                            pCallerWorkItemList = &pWorkItemList[pCaller->dgNode.id];

                            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pCallerBasicBlk->dgNode.succList);
                            pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
                            for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
                            {
                                pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
                                if (!pSuccBasicBlk->bInWorklist)
                                {
                                    vscBBWKL_AddBBToWorkItemList(pCallerWorkItemList, &pCallerWorkItemArray[pSuccBasicBlk->dgNode.id], pSuccBasicBlk);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(pThisWorkItemList));

    return errCode;
}

static void _FinalizeForwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                  VIR_BASE_TS_DFA* pTsDFA,
                                                  VIR_BB_WORKITEM** ppWorkItemArray,
                                                  VIR_BB_WORKLIST* pWorkItemList,
                                                  VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    BB_WORKLIST_FINALIZE(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (ppWorkItemArray[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pTsDFA->baseDFA.pMM, ppWorkItemArray[pFuncBlk->dgNode.id]);
        ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    }

    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pTsDFA->baseDFA.pMM, pppBasicBlkRPO[pFuncBlk->dgNode.id]);
        pppBasicBlkRPO[pFuncBlk->dgNode.id] = gcvNULL;
    }
}

static VSC_ErrCode _InitializeBackwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                            VIR_BASE_TS_DFA* pTsDFA,
                                                            gctBOOL bIPA,
                                                            VIR_BB_WORKITEM** ppWorkItemArray,
                                                            VIR_BB_WORKLIST* pWorkItemList,
                                                            VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK**            ppBasicBlkRPO;
    VIR_CONTROL_FLOW_GRAPH*      pCFG = &pFuncBlk->cfg;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    gctUINT                      bbIdx;
    VIR_BB_WORKITEM*             pThisWorkItemArray;
    VIR_BB_WORKLIST*             pThisWorkItemList;
    VIR_TS_FUNC_FLOW*            pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pTsDFA->tsFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bMainFunc = (CG_GET_MAIN_FUNC(pFuncBlk->pOwnerCG) == pFuncBlk->pVIRFunc);

    ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    BB_WORKLIST_INIT(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of reversed CFG */
    pppBasicBlkRPO[pFuncBlk->dgNode.id] = (VIR_BASIC_BLOCK**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    vscDG_PstOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)pppBasicBlkRPO[pFuncBlk->dgNode.id]);

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    ppWorkItemArray[pFuncBlk->dgNode.id] = (VIR_BB_WORKITEM*)vscMM_Alloc(pTsDFA->baseDFA.pMM,
                                                                         sizeof(VIR_BB_WORKITEM)*
                                                                         vscDG_GetHistNodeCount(&pCFG->dgGraph));

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];
    ppBasicBlkRPO = pppBasicBlkRPO[pFuncBlk->dgNode.id];

    /*  Do local analyze to resolve local gen/kill and initialize inFlow or/and outFlow.
        Also initialize workitem list */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        pTsDFA->tsDfaResolvers.ts_localGenKill_resolver(pTsDFA, ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow);
        pTsDFA->tsDfaResolvers.ts_initBlockFlow_resolver(pTsDFA, ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow);

        /* Initialize each workitem, and add it to workitem list. Note that exit block won't be
           added into workitem list because it won't be changed when iterating */
        if (bIPA && bMainFunc &&
            ppBasicBlkRPO[bbIdx]->flowType == VIR_FLOW_TYPE_EXIT)
        {
            vscBV_Copy(&pFuncFlow->outFlow, &ppBasicBlkRPO[bbIdx]->pTsWorkDataFlow->outFlow);
        }
        else
        {
            vscBBWKL_AddBBToWorkItemList(pThisWorkItemList,
                                         &pThisWorkItemArray[ppBasicBlkRPO[bbIdx]->dgNode.id],
                                         ppBasicBlkRPO[bbIdx]);
        }
    }

    return errCode;
}

static VSC_ErrCode _DoBackwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                    VIR_BASE_TS_DFA* pTsDFA,
                                                    gctBOOL bIPA,
                                                    VIR_BB_WORKITEM** ppWorkItemArray,
                                                    VIR_BB_WORKLIST* pWorkItemList,
                                                    VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH*      pCFG = &pFuncBlk->cfg;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter, succEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_CFG_EDGE*                pSuccEdge;
    VIR_BB_WORKITEM*             pThisWorkItemArray;
    VIR_BB_WORKLIST*             pThisWorkItemList;
    VIR_TS_FUNC_FLOW*            pFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pTsDFA->tsFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bOutFlowOfExitChanged;
    gctUINT                      bbIdx, callerIdx, countOfBasicBlkInCallee;
    VIR_BASIC_BLOCK**            ppCalleeBasicBlkRPO;
    VIR_FUNC_BLOCK*              pCallee;
    VIR_BB_WORKITEM*             pCalleeWorkItemArray;
    VIR_BB_WORKLIST*             pCalleeWorkItemList;
    VIR_BASIC_BLOCK*             pCalleeBasicBlk;
    VIR_FUNC_BLOCK*              pCaller;
    VIR_BB_WORKITEM*             pCallerWorkItemArray;
    VIR_BB_WORKLIST*             pCallerWorkItemList;
    VIR_BASIC_BLOCK*             pCallerBasicBlk;
    VIR_Instruction*             pCallSiteInst;
    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];

    /* Do core global iterative analyze by workitem list */
    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(pThisWorkItemList);
        bOutFlowOfExitChanged = gcvFALSE;

        if (bIPA)
        {
            if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_EXIT)
            {
                bOutFlowOfExitChanged = pTsDFA->tsDfaResolvers.ts_combineFuncFlowFromCallers_resolver(pTsDFA, pFuncFlow);
                vscBV_Copy(&pThisBasicBlk->pTsWorkDataFlow->outFlow, &pFuncFlow->outFlow);
            }
        }

        /* Iterative resolve each successor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
            pTsDFA->tsDfaResolvers.ts_iterateBlockFlow_resolver(pTsDFA, pSuccBasicBlk->pTsWorkDataFlow);

            if (bIPA)
            {
                if (pSuccBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pTsDFA->tsDfaResolvers.ts_combineBlockFlowFromCallee_resolver(pTsDFA, pSuccBasicBlk->pTsWorkDataFlow);
                }
            }
        }

        /* Check combination of successors to get outFlow of this bb. If there is a change, then add all
           predecessors that are not in workitem list to workitem list */
        if ((bIPA && bOutFlowOfExitChanged) ||
            pTsDFA->tsDfaResolvers.ts_combineBlockFlow_resolver(pTsDFA, pThisBasicBlk->pTsWorkDataFlow))
        {
            VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
            pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
            for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
            {
                pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
                if (!pPredBasicBlk->bInWorklist)
                {
                    vscBBWKL_AddBBToWorkItemList(pThisWorkItemList, &pThisWorkItemArray[pPredBasicBlk->dgNode.id], pPredBasicBlk);
                }
            }

            if (bIPA)
            {
                if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pCallee = VIR_Inst_GetCallee(pThisBasicBlk->pStartInst)->pFuncBlock;
                    ppCalleeBasicBlkRPO = pppBasicBlkRPO[pCallee->dgNode.id];
                    pCalleeWorkItemArray = ppWorkItemArray[pCallee->dgNode.id];
                    pCalleeWorkItemList = &pWorkItemList[pCallee->dgNode.id];
                    countOfBasicBlkInCallee = vscDG_GetNodeCount(&pCallee->cfg.dgGraph);

                    for (bbIdx = 0; bbIdx < countOfBasicBlkInCallee; bbIdx ++)
                    {
                        pCalleeBasicBlk = ppCalleeBasicBlkRPO[bbIdx];

                        if (!pCalleeBasicBlk->bInWorklist)
                        {
                            vscBBWKL_AddBBToWorkItemList(pCalleeWorkItemList, &pCalleeWorkItemArray[pCalleeBasicBlk->dgNode.id], pCalleeBasicBlk);
                        }
                    }
                }
            }

            if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_ENTRY)
            {
                pTsDFA->tsDfaResolvers.ts_iterateBlockFlow_resolver(pTsDFA, pThisBasicBlk->pTsWorkDataFlow);
                vscBV_Copy(&pFuncFlow->inFlow, &pThisBasicBlk->pTsWorkDataFlow->inFlow);

                if (bIPA)
                {
                    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pFuncBlk->dgNode.predList);
                    pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
                    for (; pCallerEdge != gcvNULL; pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
                    {
                        /* Call site info is only stored at successor edge */
                        pCallerEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pCallerEdge);

                        for (callerIdx = 0; callerIdx < vscSRARR_GetElementCount(&pCallerEdge->callSiteArray); callerIdx ++)
                        {
                            pCallSiteInst = *(VIR_Instruction**)vscSRARR_GetElement(&pCallerEdge->callSiteArray, callerIdx);
                            pCallerBasicBlk = VIR_Inst_GetBasicBlock(pCallSiteInst);

                            if (pCallerBasicBlk == gcvNULL)
                            {
                                gcmASSERT(gcvFALSE);
                                continue;
                            }
                            else
                            {
                                pCaller = pCallerBasicBlk->pOwnerCFG->pOwnerFuncBlk;
                            }

                            pCallerWorkItemArray = ppWorkItemArray[pCaller->dgNode.id];
                            pCallerWorkItemList = &pWorkItemList[pCaller->dgNode.id];

                            VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pCallerBasicBlk->dgNode.predList);
                            pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
                            for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
                            {
                                pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
                                if (!pPredBasicBlk->bInWorklist)
                                {
                                    vscBBWKL_AddBBToWorkItemList(pCallerWorkItemList, &pCallerWorkItemArray[pPredBasicBlk->dgNode.id], pPredBasicBlk);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(pThisWorkItemList));

    return errCode;
}

static void _FinalizeBackwardIterativeTsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                   VIR_BASE_TS_DFA* pTsDFA,
                                                   VIR_BB_WORKITEM** ppWorkItemArray,
                                                   VIR_BB_WORKLIST* pWorkItemList,
                                                   VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    BB_WORKLIST_FINALIZE(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (ppWorkItemArray[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pTsDFA->baseDFA.pMM, ppWorkItemArray[pFuncBlk->dgNode.id]);
        ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    }

    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pTsDFA->baseDFA.pMM, pppBasicBlkRPO[pFuncBlk->dgNode.id]);
        pppBasicBlkRPO[pFuncBlk->dgNode.id] = gcvNULL;
    }
}

VSC_ErrCode vscVIR_DoForwardIterativeTsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_TS_DFA* pTsDFA, gctBOOL bIPA)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_FUNC_BLOCK**        ppFuncBlkRPO;
    gctUINT                 countOfFuncBlk = vscDG_GetNodeCount(&pCg->dgGraph);
    gctUINT                 funcIdx;
    VIR_BB_WORKITEM**       ppWorkItemArray;
    VIR_BB_WORKLIST*        pWorkItemList;
    VIR_BASIC_BLOCK***      pppBasicBlkRPO;
    gctBOOL                 bFlowChangedInFunc = gcvTRUE;

    if (countOfFuncBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of call graph */
    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);
    vscDG_PstOrderTraversal(&pCg->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    /* Create shader-wide working list */
    ppWorkItemArray = (VIR_BB_WORKITEM**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKITEM*) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    pWorkItemList = (VIR_BB_WORKLIST*)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKLIST) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    pppBasicBlkRPO = (VIR_BASIC_BLOCK***)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK**) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));

    /* Initialize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        errCode = _InitializeForwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA, bIPA,
                                                          ppWorkItemArray, pWorkItemList,
                                                          pppBasicBlkRPO);

        if (errCode != VSC_ERR_NONE)
        {
            goto On_Error;
        }
    }

    /* Analyze now */
    while (bFlowChangedInFunc)
    {
        bFlowChangedInFunc = gcvFALSE;

        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            if (!BB_WORKLIST_IS_EMPTY(&pWorkItemList[ppFuncBlkRPO[funcIdx]->dgNode.id]))
            {
                bFlowChangedInFunc = gcvTRUE;

                errCode = _DoForwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA, bIPA,
                                                          ppWorkItemArray, pWorkItemList,
                                                          pppBasicBlkRPO);

                if (errCode != VSC_ERR_NONE)
                {
                    goto On_Error;
                }
            }
        }
    };

On_Error:

    /* Finalize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        _FinalizeForwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA,
                                              ppWorkItemArray, pWorkItemList,
                                              pppBasicBlkRPO);
    }

    vscMM_Free(pTsDFA->baseDFA.pMM, ppFuncBlkRPO);
    vscMM_Free(pTsDFA->baseDFA.pMM, ppWorkItemArray);
    vscMM_Free(pTsDFA->baseDFA.pMM, pWorkItemList);
    vscMM_Free(pTsDFA->baseDFA.pMM, pppBasicBlkRPO);

    return errCode;
}

VSC_ErrCode vscVIR_DoBackwardIterativeTsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_TS_DFA* pTsDFA, gctBOOL bIPA)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_FUNC_BLOCK**        ppFuncBlkRPO;
    gctUINT                 countOfFuncBlk = vscDG_GetNodeCount(&pCg->dgGraph);
    gctUINT                 funcIdx;
    VIR_BB_WORKITEM**       ppWorkItemArray;
    VIR_BB_WORKLIST*        pWorkItemList;
    VIR_BASIC_BLOCK***      pppBasicBlkRPO;
    gctBOOL                 bFlowChangedInFunc = gcvTRUE;

    if (countOfFuncBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of reverse call graph */
    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);
    vscDG_PstOrderTraversal(&pCg->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);

    /* Create shader-wide working list */
    ppWorkItemArray = (VIR_BB_WORKITEM**)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKITEM*) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    pWorkItemList = (VIR_BB_WORKLIST*)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKLIST) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    pppBasicBlkRPO = (VIR_BASIC_BLOCK***)vscMM_Alloc(pTsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK**) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));

    /* Initialize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        errCode = _InitializeBackwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA, bIPA,
                                                           ppWorkItemArray, pWorkItemList,
                                                           pppBasicBlkRPO);

        if (errCode != VSC_ERR_NONE)
        {
            goto On_Error;
        }
    }

    /* Analyze now */
    while (bFlowChangedInFunc)
    {
        bFlowChangedInFunc = gcvFALSE;

        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            if (!BB_WORKLIST_IS_EMPTY(&pWorkItemList[ppFuncBlkRPO[funcIdx]->dgNode.id]))
            {
                bFlowChangedInFunc = gcvTRUE;

                errCode = _DoBackwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA, bIPA,
                                                           ppWorkItemArray, pWorkItemList,
                                                           pppBasicBlkRPO);

                if (errCode != VSC_ERR_NONE)
                {
                    goto On_Error;
                }
            }
        }
    };

On_Error:

    /* Finalize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        _FinalizeBackwardIterativeTsDFAPerFunc(ppFuncBlkRPO[funcIdx], pTsDFA,
                                               ppWorkItemArray, pWorkItemList,
                                               pppBasicBlkRPO);
    }

    vscMM_Free(pTsDFA->baseDFA.pMM, ppFuncBlkRPO);
    vscMM_Free(pTsDFA->baseDFA.pMM, ppWorkItemArray);
    vscMM_Free(pTsDFA->baseDFA.pMM, pWorkItemList);
    vscMM_Free(pTsDFA->baseDFA.pMM, pppBasicBlkRPO);

    return errCode;
}

