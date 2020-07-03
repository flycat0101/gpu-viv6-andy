/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

void vscVIR_InitializeMsBlockFlow(VIR_MS_BLOCK_FLOW* pMsBlkFlow, VIR_BASIC_BLOCK* pOwnerBB,
                                  VSC_MM* pMM, gctINT flowSize, gctUINT stateCount)
{
    pMsBlkFlow->pOwnerBB = pOwnerBB;

    pOwnerBB->pMsWorkDataFlow = pMsBlkFlow;
    pOwnerBB->bInWorklist = gcvFALSE;

    vscSV_Initialize(&pMsBlkFlow->genFlow, pMM, flowSize, stateCount);
    vscSV_Initialize(&pMsBlkFlow->killFlow, pMM, flowSize, stateCount);
    vscSV_Initialize(&pMsBlkFlow->inFlow, pMM, flowSize, stateCount);
    vscSV_Initialize(&pMsBlkFlow->outFlow, pMM, flowSize, stateCount);
}

void vscVIR_UpdateMsBlockFlowSize(VIR_MS_BLOCK_FLOW* pMsBlkFlow, gctINT newFlowSize)
{
    vscSV_Resize(&pMsBlkFlow->genFlow, newFlowSize, gcvTRUE);
    vscSV_Resize(&pMsBlkFlow->killFlow, newFlowSize, gcvTRUE);
    vscSV_Resize(&pMsBlkFlow->inFlow, newFlowSize, gcvTRUE);
    vscSV_Resize(&pMsBlkFlow->outFlow, newFlowSize, gcvTRUE);
}

void vscVIR_FinalizeMsBlockFlow(VIR_MS_BLOCK_FLOW* pMsBlkFlow)
{
    vscSV_Finalize(&pMsBlkFlow->genFlow);
    vscSV_Finalize(&pMsBlkFlow->killFlow);
    vscSV_Finalize(&pMsBlkFlow->inFlow);
    vscSV_Finalize(&pMsBlkFlow->outFlow);

    pMsBlkFlow->pOwnerBB->pMsWorkDataFlow = gcvNULL;
}

VSC_ErrCode vscVIR_InitializeMsFuncFlow(VIR_MS_FUNC_FLOW* pMsFuncFlow, VIR_FUNC_BLOCK* pOwnerFB,
                                 VSC_MM* pMM, gctINT flowSize, gctUINT stateCount)
{
    VSC_ErrCode        errCode = VSC_ERR_NONE;
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    pMsFuncFlow->pOwnerFB = pOwnerFB;

    vscSV_Initialize(&pMsFuncFlow->inFlow, pMM, flowSize, stateCount);
    vscSV_Initialize(&pMsFuncFlow->outFlow, pMM, flowSize, stateCount);

    /* Intialize block flow array. Note as iterative analyzer will use id of basicblk to index
       block-flow, history node count will be used to allocate these contents */
    CHECK_ERROR0(vscSRARR_Initialize(&pMsFuncFlow->msBlkFlowArray,
                                     pMM,
                                     vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph),
                                     sizeof(VIR_MS_BLOCK_FLOW),
                                     gcvNULL));

    /* Directly mark all elements are used because we control index of array by ourself (i.e
       id of graph node) */
    errCode = vscSRARR_SetElementCount(&pMsFuncFlow->msBlkFlowArray, vscDG_GetHistNodeCount(&pOwnerFB->cfg.dgGraph));
    CHECK_ERROR(errCode, "Failed in vscSRARR_SetElementCount");

    CFG_ITERATOR_INIT(&basicBlkIter, &pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_MS_BLOCK_FLOW* pBlkFlow = (VIR_MS_BLOCK_FLOW*)vscSRARR_GetElement(&pMsFuncFlow->msBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        vscVIR_InitializeMsBlockFlow(pBlkFlow, pBasicBlk, pMM, flowSize, stateCount);
    }

    return errCode;
}

void vscVIR_UpdateMsFuncFlowSize(VIR_MS_FUNC_FLOW* pMsFuncFlow, gctINT newFlowSize)
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    vscSV_Resize(&pMsFuncFlow->inFlow, newFlowSize, gcvTRUE);
    vscSV_Resize(&pMsFuncFlow->outFlow, newFlowSize, gcvTRUE);

    CFG_ITERATOR_INIT(&basicBlkIter, &pMsFuncFlow->pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_MS_BLOCK_FLOW* pBlkFlow = (VIR_MS_BLOCK_FLOW*)vscSRARR_GetElement(&pMsFuncFlow->msBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        vscVIR_UpdateMsBlockFlowSize(pBlkFlow, newFlowSize);
    }
}

void vscVIR_FinalizeMsFuncFlow(VIR_MS_FUNC_FLOW* pMsFuncFlow)
{
    CFG_ITERATOR       basicBlkIter;
    VIR_BASIC_BLOCK*   pBasicBlk;

    vscSV_Finalize(&pMsFuncFlow->inFlow);
    vscSV_Finalize(&pMsFuncFlow->outFlow);

    CFG_ITERATOR_INIT(&basicBlkIter, &pMsFuncFlow->pOwnerFB->cfg);
    pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pBasicBlk != gcvNULL; pBasicBlk = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        VIR_MS_BLOCK_FLOW* pBlkFlow = (VIR_MS_BLOCK_FLOW*)vscSRARR_GetElement(&pMsFuncFlow->msBlkFlowArray,
                                                                              pBasicBlk->dgNode.id);

        vscVIR_FinalizeMsBlockFlow(pBlkFlow);
    }

    vscSRARR_Finalize(&pMsFuncFlow->msBlkFlowArray);
}

VSC_ErrCode vscVIR_InitializeBaseMsDFA(VIR_BASE_MS_DFA* pBaseMsDFA, VIR_CALL_GRAPH* pCg, VIR_DFA_TYPE dfaType,
                                gctINT flowSize, gctUINT stateCount, VSC_MM* pMM, VIR_MS_DFA_RESOLVERS* pMsDfaResolvers)
{
    VSC_ErrCode     errCode = VSC_ERR_NONE;
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    vscVIR_InitializeBaseDFA(&pBaseMsDFA->baseDFA, pCg, dfaType, flowSize, pMM, pCg->pScratchMemPool);

    pBaseMsDFA->stateCount = stateCount;

    /* Set DFA resolvers */
    pBaseMsDFA->msDfaResolvers.ms_combineBlockFlow_resolver = pMsDfaResolvers->ms_combineBlockFlow_resolver;
    pBaseMsDFA->msDfaResolvers.ms_iterateBlockFlow_resolver = pMsDfaResolvers->ms_iterateBlockFlow_resolver;
    pBaseMsDFA->msDfaResolvers.ms_initBlockFlow_resolver = pMsDfaResolvers->ms_initBlockFlow_resolver;
    pBaseMsDFA->msDfaResolvers.ms_localGenKill_resolver = pMsDfaResolvers->ms_localGenKill_resolver;
    pBaseMsDFA->msDfaResolvers.ms_combineBlockFlowFromCallee_resolver = pMsDfaResolvers->ms_combineBlockFlowFromCallee_resolver;
    pBaseMsDFA->msDfaResolvers.ms_combineFuncFlowFromCallers_resolver = pMsDfaResolvers->ms_combineFuncFlowFromCallers_resolver;

    /* Initialize func-flow array. Note as iterative analyzer will use id of funcblk to index
       func-flow, history node count will be used to allocate these contents */
    vscSRARR_Initialize(&pBaseMsDFA->msFuncFlowArray,
                        pMM,
                        vscDG_GetHistNodeCount(&pCg->dgGraph),
                        sizeof(VIR_MS_FUNC_FLOW),
                        gcvNULL);

    /* Directly mark all elements are used because we control index of array by ourself (i.e
       id of graph node) */
    errCode = vscSRARR_SetElementCount(&pBaseMsDFA->msFuncFlowArray, vscDG_GetHistNodeCount(&pCg->dgGraph));
    CHECK_ERROR(errCode, "Failed in vscSRARR_SetElementCount");

    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_MS_FUNC_FLOW* pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseMsDFA->msFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        CHECK_ERROR0(vscVIR_InitializeMsFuncFlow(pFuncFlow, pFuncBlk, pMM, flowSize, stateCount));
    }

    return errCode;
}

void vscVIR_UpdateBaseMsDFAFlowSize(VIR_BASE_MS_DFA* pBaseMsDFA, gctINT newFlowSize)
{
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    vscVIR_UpdateBaseDFAFlowSize(&pBaseMsDFA->baseDFA, newFlowSize);

    CG_ITERATOR_INIT(&funcBlkIter, pBaseMsDFA->baseDFA.pOwnerCG);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_MS_FUNC_FLOW* pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseMsDFA->msFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        vscVIR_UpdateMsFuncFlowSize(pFuncFlow, newFlowSize);
    }
}

void vscVIR_FinalizeBaseMsDFA(VIR_BASE_MS_DFA* pBaseMsDFA)
{
    CG_ITERATOR     funcBlkIter;
    VIR_FUNC_BLOCK* pFuncBlk;

    CG_ITERATOR_INIT(&funcBlkIter, pBaseMsDFA->baseDFA.pOwnerCG);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        VIR_MS_FUNC_FLOW* pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseMsDFA->msFuncFlowArray,
                                                                             pFuncBlk->dgNode.id);

        vscVIR_FinalizeMsFuncFlow(pFuncFlow);
    }

    vscSRARR_Finalize(&pBaseMsDFA->msFuncFlowArray);

    vscVIR_FinalizeBaseDFA(&pBaseMsDFA->baseDFA);
}

static VSC_ErrCode _InitializeForwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                           VIR_BASE_MS_DFA* pMsDFA,
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
    VIR_MS_FUNC_FLOW*            pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pMsDFA->msFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bMainFunc = (CG_GET_MAIN_FUNC(pFuncBlk->pOwnerCG) == pFuncBlk->pVIRFunc);

    ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    BB_WORKLIST_INIT(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of CFG */
    pppBasicBlkRPO[pFuncBlk->dgNode.id] = (VIR_BASIC_BLOCK**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Initialize Forward Iterative Ms DFA");
    }
    errCode = vscDG_PstOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)pppBasicBlkRPO[pFuncBlk->dgNode.id]);
    CHECK_ERROR(errCode, "Initialize Forward Iterative Ms DFA");

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    ppWorkItemArray[pFuncBlk->dgNode.id] = (VIR_BB_WORKITEM*)vscMM_Alloc(pMsDFA->baseDFA.pMM,
                                                                         sizeof(VIR_BB_WORKITEM)*
                                                                         vscDG_GetHistNodeCount(&pCFG->dgGraph));
    if (ppWorkItemArray[pFuncBlk->dgNode.id] == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Initialize Forward Iterative Ms DFA");
    }

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];
    ppBasicBlkRPO = pppBasicBlkRPO[pFuncBlk->dgNode.id];

    /*  Do local analyze to resolve local gen/kill and initialize inFlow or/and outFlow.
        Also initialize workitem list */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        pMsDFA->msDfaResolvers.ms_localGenKill_resolver(pMsDFA, ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow);
        pMsDFA->msDfaResolvers.ms_initBlockFlow_resolver(pMsDFA, ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow);

        /* Initialize each workitem, and add it to workitem list. Note that entry block won't be
           added into workitem list because it won't be changed when iterating */
        if (bIPA && bMainFunc &&
            ppBasicBlkRPO[bbIdx]->flowType == VIR_FLOW_TYPE_ENTRY)
        {
            vscSV_Copy(&pFuncFlow->inFlow, &ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow->inFlow);
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

static VSC_ErrCode _DoForwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                   VIR_BASE_MS_DFA* pMsDFA,
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
    VIR_MS_FUNC_FLOW*            pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pMsDFA->msFuncFlowArray, pFuncBlk->dgNode.id);
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
                bInFlowOfEntryChanged = pMsDFA->msDfaResolvers.ms_combineFuncFlowFromCallers_resolver(pMsDFA, pFuncFlow);
                vscSV_Copy(&pThisBasicBlk->pMsWorkDataFlow->inFlow, &pFuncFlow->inFlow);
            }
        }

        /* Iterative resolve each predecessor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
            pMsDFA->msDfaResolvers.ms_iterateBlockFlow_resolver(pMsDFA, pPredBasicBlk->pMsWorkDataFlow);

            if (bIPA)
            {
                if (pPredBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pMsDFA->msDfaResolvers.ms_combineBlockFlowFromCallee_resolver(pMsDFA, pPredBasicBlk->pMsWorkDataFlow);
                }
            }
        }

        /* Check combination of predecessors to get inFlow of this bb. If there is a change, then add all
           successors that are not in workitem list to workitem list */
        if ((bIPA && bInFlowOfEntryChanged) ||
            pMsDFA->msDfaResolvers.ms_combineBlockFlow_resolver(pMsDFA, pThisBasicBlk->pMsWorkDataFlow))
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
                pMsDFA->msDfaResolvers.ms_iterateBlockFlow_resolver(pMsDFA, pThisBasicBlk->pMsWorkDataFlow);
                vscSV_Copy(&pFuncFlow->outFlow, &pThisBasicBlk->pMsWorkDataFlow->outFlow);

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

static void _FinalizeForwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                  VIR_BASE_MS_DFA* pMsDFA,
                                                  VIR_BB_WORKITEM** ppWorkItemArray,
                                                  VIR_BB_WORKLIST* pWorkItemList,
                                                  VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    BB_WORKLIST_FINALIZE(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (ppWorkItemArray[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pMsDFA->baseDFA.pMM, ppWorkItemArray[pFuncBlk->dgNode.id]);
        ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    }

    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pMsDFA->baseDFA.pMM, pppBasicBlkRPO[pFuncBlk->dgNode.id]);
        pppBasicBlkRPO[pFuncBlk->dgNode.id] = gcvNULL;
    }
}

static VSC_ErrCode _InitializeBackwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                            VIR_BASE_MS_DFA* pMsDFA,
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
    VIR_MS_FUNC_FLOW*            pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pMsDFA->msFuncFlowArray, pFuncBlk->dgNode.id);
    gctBOOL                      bMainFunc = (CG_GET_MAIN_FUNC(pFuncBlk->pOwnerCG) == pFuncBlk->pVIRFunc);

    ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    BB_WORKLIST_INIT(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of reversed CFG */
    pppBasicBlkRPO[pFuncBlk->dgNode.id] = (VIR_BASIC_BLOCK**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Initialize Backward Iterative Ms DFA");
    }
    errCode = vscDG_PstOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)pppBasicBlkRPO[pFuncBlk->dgNode.id]);
    CHECK_ERROR(errCode, "Initialize Backward Iterative Ms DFA");

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    ppWorkItemArray[pFuncBlk->dgNode.id] = (VIR_BB_WORKITEM*)vscMM_Alloc(pMsDFA->baseDFA.pMM,
                                                                         sizeof(VIR_BB_WORKITEM)*
                                                                         vscDG_GetHistNodeCount(&pCFG->dgGraph));
    if (ppWorkItemArray[pFuncBlk->dgNode.id] == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Initialize Backward Iterative Ms DFA");
    }

    /* Deference */
    pThisWorkItemArray = ppWorkItemArray[pFuncBlk->dgNode.id];
    pThisWorkItemList = &pWorkItemList[pFuncBlk->dgNode.id];
    ppBasicBlkRPO = pppBasicBlkRPO[pFuncBlk->dgNode.id];

    /*  Do local analyze to resolve local gen/kill and initialize inFlow or/and outFlow.
        Also initialize workitem list */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        pMsDFA->msDfaResolvers.ms_localGenKill_resolver(pMsDFA, ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow);
        pMsDFA->msDfaResolvers.ms_initBlockFlow_resolver(pMsDFA, ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow);

        /* Initialize each workitem, and add it to workitem list. Note that exit block won't be
           added into workitem list because it won't be changed when iterating */
        if (bIPA && bMainFunc &&
            ppBasicBlkRPO[bbIdx]->flowType == VIR_FLOW_TYPE_EXIT)
        {
            vscSV_Copy(&pFuncFlow->outFlow, &ppBasicBlkRPO[bbIdx]->pMsWorkDataFlow->outFlow);
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

static VSC_ErrCode _DoBackwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                    VIR_BASE_MS_DFA* pMsDFA,
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
    VIR_MS_FUNC_FLOW*            pFuncFlow = (VIR_MS_FUNC_FLOW*)vscSRARR_GetElement(&pMsDFA->msFuncFlowArray, pFuncBlk->dgNode.id);
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
                bOutFlowOfExitChanged = pMsDFA->msDfaResolvers.ms_combineFuncFlowFromCallers_resolver(pMsDFA, pFuncFlow);
                vscSV_Copy(&pThisBasicBlk->pMsWorkDataFlow->outFlow, &pFuncFlow->outFlow);
            }
        }

        /* Iterative resolve each successor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
            pMsDFA->msDfaResolvers.ms_iterateBlockFlow_resolver(pMsDFA, pSuccBasicBlk->pMsWorkDataFlow);

            if (bIPA)
            {
                if (pSuccBasicBlk->flowType == VIR_FLOW_TYPE_CALL)
                {
                    pMsDFA->msDfaResolvers.ms_combineBlockFlowFromCallee_resolver(pMsDFA, pSuccBasicBlk->pMsWorkDataFlow);
                }
            }
        }

        /* Check combination of successors to get outFlow of this bb. If there is a change, then add all
           predecessors that are not in workitem list to workitem list */
        if ((bIPA && bOutFlowOfExitChanged) ||
            pMsDFA->msDfaResolvers.ms_combineBlockFlow_resolver(pMsDFA, pThisBasicBlk->pMsWorkDataFlow))
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
                pMsDFA->msDfaResolvers.ms_iterateBlockFlow_resolver(pMsDFA, pThisBasicBlk->pMsWorkDataFlow);
                vscSV_Copy(&pFuncFlow->inFlow, &pThisBasicBlk->pMsWorkDataFlow->inFlow);

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

static void _FinalizeBackwardIterativeMsDFAPerFunc(VIR_FUNC_BLOCK* pFuncBlk,
                                                   VIR_BASE_MS_DFA* pMsDFA,
                                                   VIR_BB_WORKITEM** ppWorkItemArray,
                                                   VIR_BB_WORKLIST* pWorkItemList,
                                                   VIR_BASIC_BLOCK*** pppBasicBlkRPO)
{
    BB_WORKLIST_FINALIZE(&pWorkItemList[pFuncBlk->dgNode.id]);

    if (ppWorkItemArray[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pMsDFA->baseDFA.pMM, ppWorkItemArray[pFuncBlk->dgNode.id]);
        ppWorkItemArray[pFuncBlk->dgNode.id] = gcvNULL;
    }

    if (pppBasicBlkRPO[pFuncBlk->dgNode.id] != gcvNULL)
    {
        vscMM_Free(pMsDFA->baseDFA.pMM, pppBasicBlkRPO[pFuncBlk->dgNode.id]);
        pppBasicBlkRPO[pFuncBlk->dgNode.id] = gcvNULL;
    }
}

VSC_ErrCode vscVIR_DoForwardIterativeMsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_MS_DFA* pMsDFA, gctBOOL bIPA)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_FUNC_BLOCK**        ppFuncBlkRPO = gcvNULL;
    gctUINT                 countOfFuncBlk = vscDG_GetNodeCount(&pCg->dgGraph);
    gctUINT                 funcIdx;
    VIR_BB_WORKITEM**       ppWorkItemArray = gcvNULL;
    VIR_BB_WORKLIST*        pWorkItemList = gcvNULL;
    VIR_BASIC_BLOCK***      pppBasicBlkRPO = gcvNULL;
    gctBOOL                 bFlowChangedInFunc = gcvTRUE;

    if (countOfFuncBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of call graph */
    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);
    if (!ppFuncBlkRPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    errCode = vscDG_PstOrderTraversal(&pCg->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);
    if (errCode != VSC_ERR_NONE)
        goto On_Error;

    /* Create shader-wide working list */
    ppWorkItemArray = (VIR_BB_WORKITEM**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKITEM*) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!ppWorkItemArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    pWorkItemList = (VIR_BB_WORKLIST*)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKLIST) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!pWorkItemList)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    pppBasicBlkRPO = (VIR_BASIC_BLOCK***)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK**) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!pppBasicBlkRPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }

    /* Initialize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        errCode = _InitializeForwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA, bIPA,
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

                errCode = _DoForwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA, bIPA,
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
    if (ppFuncBlkRPO)
    {
        /* Finalize DFA per func */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            _FinalizeForwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA,
                                                  ppWorkItemArray, pWorkItemList,
                                                  pppBasicBlkRPO);
        }

        vscMM_Free(pMsDFA->baseDFA.pMM, ppFuncBlkRPO);
    }
    if (ppWorkItemArray)
        vscMM_Free(pMsDFA->baseDFA.pMM, ppWorkItemArray);
    if (pWorkItemList)
        vscMM_Free(pMsDFA->baseDFA.pMM, pWorkItemList);
    if (pppBasicBlkRPO)
        vscMM_Free(pMsDFA->baseDFA.pMM, pppBasicBlkRPO);

    return errCode;
}

VSC_ErrCode vscVIR_DoBackwardIterativeMsDFA(VIR_CALL_GRAPH* pCg, VIR_BASE_MS_DFA* pMsDFA, gctBOOL bIPA)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_FUNC_BLOCK**        ppFuncBlkRPO = gcvNULL;
    gctUINT                 countOfFuncBlk = vscDG_GetNodeCount(&pCg->dgGraph);
    gctUINT                 funcIdx;
    VIR_BB_WORKITEM**       ppWorkItemArray = gcvNULL;
    VIR_BB_WORKLIST*        pWorkItemList = gcvNULL;
    VIR_BASIC_BLOCK***      pppBasicBlkRPO = gcvNULL;
    gctBOOL                 bFlowChangedInFunc = gcvTRUE;

    if (countOfFuncBlk == 0)
    {
        return errCode;
    }

    /* Get RPO of reverse call graph */
    ppFuncBlkRPO = (VIR_FUNC_BLOCK**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_FUNC_BLOCK*)*countOfFuncBlk);
    if (!ppFuncBlkRPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    errCode = vscDG_PstOrderTraversal(&pCg->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvTRUE,
                            (VSC_DG_NODE**)ppFuncBlkRPO);
    if (errCode != VSC_ERR_NONE)
        goto On_Error;

    /* Create shader-wide working list */
    ppWorkItemArray = (VIR_BB_WORKITEM**)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKITEM*) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!ppWorkItemArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    pWorkItemList = (VIR_BB_WORKLIST*)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BB_WORKLIST) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!pWorkItemList)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }
    pppBasicBlkRPO = (VIR_BASIC_BLOCK***)vscMM_Alloc(pMsDFA->baseDFA.pMM, sizeof(VIR_BASIC_BLOCK**) *
                                                     vscDG_GetHistNodeCount(&pCg->dgGraph));
    if (!pppBasicBlkRPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        goto On_Error;
    }

    /* Initialize DFA per func */
    for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
    {
        errCode = _InitializeBackwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA, bIPA,
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

                errCode = _DoBackwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA, bIPA,
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
    if (ppFuncBlkRPO)
    {
        /* Finalize DFA per func */
        for (funcIdx = 0; funcIdx < countOfFuncBlk; funcIdx ++)
        {
            _FinalizeBackwardIterativeMsDFAPerFunc(ppFuncBlkRPO[funcIdx], pMsDFA,
                                                   ppWorkItemArray, pWorkItemList,
                                                   pppBasicBlkRPO);
        }

        vscMM_Free(pMsDFA->baseDFA.pMM, ppFuncBlkRPO);
    }
    if (ppWorkItemArray)
        vscMM_Free(pMsDFA->baseDFA.pMM, ppWorkItemArray);
    if (pWorkItemList)
        vscMM_Free(pMsDFA->baseDFA.pMM, pWorkItemList);
    if (pppBasicBlkRPO)
        vscMM_Free(pMsDFA->baseDFA.pMM, pppBasicBlkRPO);

    return errCode;
}

