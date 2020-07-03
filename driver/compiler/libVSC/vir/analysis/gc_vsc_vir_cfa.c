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

/*
 *    Call graph related code
 */

#define _HFUNC_PassThroughGlobalBbId _HFUNC_PassThroughNodeId
#define GLOBAL_BB_HASH_TABLE_SIZE    GNODE_HASH_TABLE_SIZE
#define INVALID_GLOBAL_BB_ID         INVALID_GNODE_ID

static VSC_ErrCode _IntializeCallGraph(VSC_MM* pScratchMemPool, VIR_CALL_GRAPH* pCg, VIR_Shader* pShader)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    gctINT instCount = BT_GET_MAX_VALID_ID(&pShader->instTable);
    gctINT hashTblSize;
    vscPMP_Intialize(&pCg->pmp, gcvNULL, 20*(sizeof(VIR_FUNC_BLOCK)+4*sizeof(VIR_CG_EDGE)), sizeof(void*), gcvTRUE);
    errCode = vscDG_Initialize(&pCg->dgGraph, &pCg->pmp.mmWrapper, 2, 4, sizeof(VIR_CG_EDGE));
    ON_ERROR0(errCode);
    pCg->pOwnerShader = pShader;
    pCg->nextGlobalBbId = 0;
    pCg->pScratchMemPool = pScratchMemPool;
    hashTblSize = (instCount/5 > GLOBAL_BB_HASH_TABLE_SIZE) ? instCount/5 : GLOBAL_BB_HASH_TABLE_SIZE;
    errCode = vscHTBL_Initialize(&pCg->globalBbHashTable, &pCg->pmp.mmWrapper, _HFUNC_PassThroughGlobalBbId,
                                 gcvNULL, hashTblSize);
    ON_ERROR0(errCode);
OnError:
    return errCode;
}

static void _FinalizeCallGraph(VIR_CALL_GRAPH* pCg)
{
    vscDG_Finalize(&pCg->dgGraph);
    vscHTBL_Finalize(&pCg->globalBbHashTable);
    vscPMP_Finalize(&pCg->pmp);
}

gctBOOL CALL_SITE_CMP(void* pNode1, void* pNode2)
{
    VIR_Instruction** ppNode1 = (VIR_Instruction**)pNode1;
    VIR_Instruction** ppNode2 = (VIR_Instruction**)pNode2;

    return (*ppNode1 == *ppNode2);
}

static VIR_FUNC_BLOCK* _TryAddFuncBlockToCallGraph(VIR_CALL_GRAPH* pCg, VIR_Function* pVIRFunc)
{
    VIR_FUNC_BLOCK*   pFuncBlk = pVIRFunc->pFuncBlock;

    if (pFuncBlk == gcvNULL)
    {
        pFuncBlk = (VIR_FUNC_BLOCK*)vscMM_Alloc(&pCg->pmp.mmWrapper, sizeof(VIR_FUNC_BLOCK));
        if (pFuncBlk == gcvNULL)
        {
            ERR_REPORT(VSC_ERR_OUT_OF_MEMORY, "Fail to add function block");
            return pFuncBlk;
        }
        vscDGND_Initialize(&pFuncBlk->dgNode);
        pFuncBlk->pVIRFunc = pVIRFunc;
        pFuncBlk->pOwnerCG = pCg;
        pVIRFunc->pFuncBlock = pFuncBlk;
        pFuncBlk->maxCallDepth = 0;
        pFuncBlk->minCallDepth = VIR_INFINITE_CALL_DEPTH;
        memset(&pFuncBlk->cfg, 0, sizeof(VIR_CONTROL_FLOW_GRAPH));
        if(vscSRARR_Initialize(&pFuncBlk->mixedCallSiteArray, &pCg->pmp.mmWrapper, 2, sizeof(VIR_Instruction*), CALL_SITE_CMP) != VSC_ERR_NONE)
            return gcvNULL;
        vscDG_AddNode(&pCg->dgGraph, &pFuncBlk->dgNode);
    }

    return pFuncBlk;
}

/* Callback for unreachability of function */
static gctBOOL _RootFuncBlkHandlerDFSPost(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, void* pNonParam)
{
    if (pFuncBlk->pVIRFunc == pCg->pOwnerShader->mainFunction)
    {
        /* Only start traversaling from main-function */
        return gcvFALSE;
    }

    return gcvTRUE;
}

static void _RemoveFuncBlockFromCallGraph(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, gctBOOL bRemoveFuncInShader)
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
    vscDG_RemoveNode(&pCg->dgGraph, &pFuncBlk->dgNode);

    /* Remove func from shader */
    if (bRemoveFuncInShader)
    {
        VIR_Shader_RemoveFunction(pCg->pOwnerShader, pFuncBlk->pVIRFunc);
    }

    /* VIR func now is no longer associated with this func block */
    pFuncBlk->pVIRFunc->pFuncBlock = gcvNULL;

    vscDGND_Finalize(&pFuncBlk->dgNode);

    /* Free this node */
    vscMM_Free(&pCg->pmp.mmWrapper, pFuncBlk);
}

static void _RemoveUnreachableFunctions(VIR_CALL_GRAPH* pCg)
{
    CG_ITERATOR             funcBlkIter;
    VIR_FUNC_BLOCK*         pFuncBlk;

    /* Firstly do traversal CG to find unreachable func-blocks which are not visisted ones */
    vscDG_TraversalCB(&pCg->dgGraph,
                      VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                      gcvFALSE,
                      (PFN_DG_NODE_HANLDER)_RootFuncBlkHandlerDFSPost,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL);

    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        /* Any unvisited one is unreachable, so remove it from graph */
        if (!pFuncBlk->dgNode.bVisited)
        {
            _RemoveFuncBlockFromCallGraph(pCg, pFuncBlk, gcvTRUE);
        }
    }
}

/* Parameter passing into traversal callbacks of CG */
typedef struct _VSC_CALL_DEPTH_HELPER
{
    VIR_FUNC_BLOCK** ppCallStack;
    gctUINT          callDepth;
}VSC_CALL_DEPTH_HELPER;

/* Following 4 functions are callbacks for DFS traversal of CG, they are for determining recursive and call depth calc */
static gctBOOL _OwnFuncBlkHandlerDFSPre(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, VSC_CALL_DEPTH_HELPER* pCallDepth)
{
    gctUINT                i;
    VIR_Function*          pFunc;
    VIR_FUNC_BLOCK*        pThisFuncBlk;

    if (pFuncBlk->dgNode.bVisited)
    {
        i = pCallDepth->callDepth;

        do
        {
            pThisFuncBlk = pCallDepth->ppCallStack[i];
            pFunc = pThisFuncBlk->pVIRFunc;

            /* Mark func as recursive */
            pFunc->flags |= VIR_FUNCFLAG_RECURSIVE;
            pThisFuncBlk->maxCallDepth = VIR_INFINITE_CALL_DEPTH;

        } while(pCallDepth->ppCallStack[--i] != pFuncBlk);

        /* Allow it can be visited multiple times */
        pFuncBlk->dgNode.bVisited = gcvFALSE;

        /* We need break iteration */
        return gcvTRUE;
    }

    /* Set proper min and max call depth */
    pFuncBlk->minCallDepth = vscMIN(pFuncBlk->minCallDepth, pCallDepth->callDepth);
    pFuncBlk->maxCallDepth = vscMAX(pFuncBlk->maxCallDepth, pCallDepth->callDepth);

    return gcvFALSE;
}

static gctBOOL _OwnFuncBlkHandlerDFSPost(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, VSC_CALL_DEPTH_HELPER* pCallDepth)
{
    /* Allow it can be visited multiple times */
    pFuncBlk->dgNode.bVisited = gcvFALSE;

    return gcvFALSE;
}

static gctBOOL _SuccFuncBlkHandlerDFSPre(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, VSC_CALL_DEPTH_HELPER* pCallDepth)
{
    pCallDepth->callDepth ++;
    pCallDepth->ppCallStack[pCallDepth->callDepth] = pFuncBlk;

    return gcvFALSE;
}

static gctBOOL _SuccFuncBlkHandlerDFSPost(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFuncBlk, VSC_CALL_DEPTH_HELPER* pCallDepth)
{
    pCallDepth->callDepth --;

    return gcvFALSE;
}

static VIR_CG_EDGE* _AddEdgeForCG(VIR_CALL_GRAPH* pCg, VIR_FUNC_BLOCK* pFromFB,
                          VIR_FUNC_BLOCK* pToFB, VIR_Instruction* pCallSiteInst)
{
    VIR_CG_EDGE*           pEdge;
    gctBOOL                bIsNewEdge;

    /* We only consider successor edge */
    pEdge = (VIR_CG_EDGE*)vscDG_AddEdge(&pCg->dgGraph, &pFromFB->dgNode, &pToFB->dgNode, &bIsNewEdge);
    if (!pEdge)
        return gcvNULL;

    if (bIsNewEdge)
    {
        vscSRARR_Initialize(&pEdge->callSiteArray, &pCg->pmp.mmWrapper, 2, sizeof(VIR_Instruction*), CALL_SITE_CMP);
    }

    /* Add this call site */
    vscSRARR_AddElement(&pEdge->callSiteArray, (void*)&pCallSiteInst);
    return pEdge;
}

VSC_ErrCode vscVIR_BuildCallGraph(VSC_MM* pScratchMemPool, VIR_Shader* pShader, VIR_CALL_GRAPH* pCg)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_Function*          pCalleeFunc;
    VIR_Function*          pMainFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;
    VIR_InstIterator       instIter;
    VIR_FUNC_BLOCK*        pThisFuncBlk;
    VIR_FUNC_BLOCK*        pCalleeFuncBlk;
    VIR_Instruction*       pInst;
    VSC_CALL_DEPTH_HELPER  callDepth;
    VIR_CG_EDGE*           pEdge;

    /* So far when we building the call grap we need to remove all unused functions, which is unacceptable for some shaders. */
    gcmASSERT(VIR_Shader_CanRemoveUnusedFunctions(pShader));

    /* Intialize call graph */
    _IntializeCallGraph(pScratchMemPool, pCg, pShader);

    /* Go through all functions to build function blocks and call graph */
    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;

        /* Check wether this func has been added into graph, if no, create a func-block (graph node)
           and add into graph */
        pThisFuncBlk = _TryAddFuncBlockToCallGraph(pCg, pFunc);
        if (!pThisFuncBlk)
        {
            errCode = VSC_ERR_INVALID_DATA;
            CHECK_ERROR(errCode, "Build call graph");
        }

        /* Now, we can search all 'call' opcodes in current func to find out all callees and connect caller
           and callee together in call graph */
        VIR_InstIterator_Init(&instIter, &pFunc->instList);
        pInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
        for (; pInst != gcvNULL; pInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
        {
            if (VIR_Inst_GetOpcode(pInst) == VIR_OP_CALL)
            {
                pCalleeFunc = VIR_Inst_GetCallee(pInst);
                pCalleeFuncBlk = _TryAddFuncBlockToCallGraph(pCg, pCalleeFunc);
                if (!pCalleeFuncBlk)
                {
                    errCode = VSC_ERR_INVALID_DATA;
                    CHECK_ERROR(errCode, "Build call graph");
                }

                /* OK, add this call relation to CG */
                pEdge = _AddEdgeForCG(pCg, pThisFuncBlk, pCalleeFuncBlk, pInst);
                if (!pEdge)
                {
                    errCode = VSC_ERR_INVALID_DATA;
                    CHECK_ERROR(errCode, "Build call graph");
                }

                vscSRARR_AddElement(&pThisFuncBlk->mixedCallSiteArray, (void*)&pInst);

                /* We need to remove all these connections when remove this call instruction. */
            }
        }
    }

    /* At this time, we have CG built, let's remove all unreachable functions from main routine */
    _RemoveUnreachableFunctions(pCg);

    pMainFunc = (VIR_Function*)CG_GET_MAIN_FUNC(pCg);
    gcmASSERT(vscDG_GetRootCount(&pCg->dgGraph) == 1 && pShader->mainFunction == pMainFunc);

    /* Now we can do one pass DFS traversal to determine recursive func and call depth of each func */
    callDepth.callDepth = 0;
    callDepth.ppCallStack = (VIR_FUNC_BLOCK**)vscMM_Alloc(&pCg->pmp.mmWrapper, vscDG_GetNodeCount(&pCg->dgGraph)*sizeof(VIR_FUNC_BLOCK*));
    if (callDepth.ppCallStack == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build call graph");
    }
    callDepth.ppCallStack[0] = pMainFunc->pFuncBlock;
    errCode = vscDG_TraversalCB(&pCg->dgGraph,
                      VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                      gcvFALSE,
                      gcvNULL,
                      (PFN_DG_NODE_HANLDER)_OwnFuncBlkHandlerDFSPre,
                      (PFN_DG_NODE_HANLDER)_OwnFuncBlkHandlerDFSPost,
                      (PFN_DG_NODE_HANLDER)_SuccFuncBlkHandlerDFSPre,
                      (PFN_DG_NODE_HANLDER)_SuccFuncBlkHandlerDFSPost,
                      gcvNULL,
                      &callDepth);
    CHECK_ERROR(errCode, "Build call graph");

    vscMM_Free(&pCg->pmp.mmWrapper, callDepth.ppCallStack);

    return errCode;
}

VSC_ErrCode vscVIR_DestroyCallGraph(VIR_CALL_GRAPH* pCg)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    CG_ITERATOR             funcBlkIter;
    VIR_FUNC_BLOCK*         pThisFuncBlk;
    VIR_FUNC_BLOCK*         pNextFuncBlk;

    /* If CG has not been built, just return */
    if (!vscVIR_IsCallGraphBuilt(pCg))
    {
        return errCode;
    }

    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pThisFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pThisFuncBlk != gcvNULL;)
    {
        pNextFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter);

        _RemoveFuncBlockFromCallGraph(pCg, pThisFuncBlk, gcvFALSE);

        pThisFuncBlk = pNextFuncBlk;
    }

    _FinalizeCallGraph(pCg);

    /* Detach shader from CG now */
    pCg->pOwnerShader = gcvNULL;

    return errCode;
}

gctBOOL vscVIR_IsCallGraphBuilt(VIR_CALL_GRAPH* pCg)
{
    return (pCg->pOwnerShader != gcvNULL);
}

VSC_ErrCode vscVIR_RemoveFuncBlockFromCallGraph(VIR_CALL_GRAPH* pCg,
                                                VIR_FUNC_BLOCK* pFuncBlk,
                                                gctBOOL bRemoveFuncInShader)
{
    _RemoveFuncBlockFromCallGraph(pCg, pFuncBlk, bRemoveFuncInShader);

    return VSC_ERR_NONE;
}

/*
 *    Control flow graph related code
 */

static VSC_ErrCode _IntializeCFG(VIR_CONTROL_FLOW_GRAPH* pCfg, VSC_MM* pScratchMemPool, VIR_FUNC_BLOCK* pFuncBlk)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    vscPMP_Intialize(&pCfg->pmp, gcvNULL,
                     20*(sizeof(VIR_BASIC_BLOCK) + 2*sizeof(VIR_DOM_TREE_NODE) + 4*sizeof(VIR_CFG_EDGE)),
                     sizeof(void*), gcvTRUE);
    vscDG_Initialize(&pCfg->dgGraph, &pCfg->pmp.mmWrapper, 10, 10, sizeof(VIR_CFG_EDGE));

    ON_ERROR0(vscTREE_Initialize(&pCfg->domTree.tree, &pCfg->pmp.mmWrapper, 4));
    ON_ERROR0(vscTREE_Initialize(&pCfg->postDomTree.tree, &pCfg->pmp.mmWrapper, 4));

    pCfg->domTree.pOwnerCFG = pCfg;
    pCfg->postDomTree.pOwnerCFG = pCfg;

    pCfg->pOwnerFuncBlk = pFuncBlk;
    pCfg->pScratchMemPool = pScratchMemPool;

OnError:
    return errCode;
}

static void _FinalizeCFG(VIR_CONTROL_FLOW_GRAPH* pCfg)
{
    vscDG_Finalize(&pCfg->dgGraph);

    vscTREE_Finalize(&pCfg->domTree.tree);
    vscTREE_Finalize(&pCfg->postDomTree.tree);

    vscPMP_Finalize(&pCfg->pmp);

    pCfg->domTree.pOwnerCFG = gcvNULL;
    pCfg->postDomTree.pOwnerCFG = gcvNULL;

    pCfg->pOwnerFuncBlk = gcvNULL;
}

static VIR_FLOW_TYPE _GetFlowType(VIR_OpCode opcode)
{
    switch (opcode)
    {
    case VIR_OP_JMP:
        return VIR_FLOW_TYPE_JMP;
    case VIR_OP_JMPC:
    case VIR_OP_JMP_ANY:
        return VIR_FLOW_TYPE_JMPC;
    case VIR_OP_RET:
        return VIR_FLOW_TYPE_RET;
    case VIR_OP_CALL:
    case VIR_OP_ICALL:
        return VIR_FLOW_TYPE_CALL;
    default:
        return VIR_FLOW_TYPE_NONE;
    }
}

static void _InitializeBbReachRelation(VIR_BB_REACH_RELATION* pBbReachRelation, VSC_MM* pMM, gctINT totalBbCount)
{
    vscBV_Initialize(&pBbReachRelation->fwdReachInBBSet, pMM, totalBbCount);
    vscBV_Initialize(&pBbReachRelation->fwdReachOutBBSet, pMM, totalBbCount);
    vscBV_Initialize(&pBbReachRelation->bwdReachInBBSet, pMM, totalBbCount);
    vscBV_Initialize(&pBbReachRelation->bwdReachOutBBSet, pMM, totalBbCount);
}

static void _FinalizeBbReachRelation(VIR_BB_REACH_RELATION* pBbReachRelation)
{
    vscBV_Finalize(&pBbReachRelation->fwdReachInBBSet);
    vscBV_Finalize(&pBbReachRelation->fwdReachOutBBSet);
    vscBV_Finalize(&pBbReachRelation->bwdReachInBBSet);
    vscBV_Finalize(&pBbReachRelation->bwdReachOutBBSet);
}

static VIR_BASIC_BLOCK* _AddBasicBlockToCFG(VIR_CONTROL_FLOW_GRAPH* pCfg)
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_BASIC_BLOCK* pBasicBlock = gcvNULL;

    pBasicBlock = (VIR_BASIC_BLOCK*)vscMM_Alloc(&pCfg->pmp.mmWrapper, sizeof(VIR_BASIC_BLOCK));
    if (!pBasicBlock)
    {
        ERR_REPORT(VSC_ERR_OUT_OF_MEMORY, "Fail to add bb");
        return pBasicBlock;
    }
    vscDGND_Initialize(&pBasicBlock->dgNode);

    pBasicBlock->pOwnerCFG = pCfg;
    pBasicBlock->pStartInst = gcvNULL;
    pBasicBlock->pEndInst = gcvNULL;
    pBasicBlock->flowType = VIR_FLOW_TYPE_NONE;
    pBasicBlock->instCount = 0;
    pBasicBlock->bInWorklist = gcvFALSE;
    pBasicBlock->pTsWorkDataFlow = gcvNULL;
    pBasicBlock->pMsWorkDataFlow = gcvNULL;
    pBasicBlock->pDomTreeNode = gcvNULL;
    pBasicBlock->pPostDomTreeNode = gcvNULL;
    pBasicBlock->flags = 0;
    pBasicBlock->globalBbId = pCfg->pOwnerFuncBlk->pOwnerCG->nextGlobalBbId ++;
    pBasicBlock->dfsPreVisitOrderIdx = NOT_ASSIGNED;
    pBasicBlock->dfsPostVisitOrderIdx = NOT_ASSIGNED;
    errCode = vscHTBL_DirectSet(&pCfg->pOwnerFuncBlk->pOwnerCG->globalBbHashTable,
                                (void*)(gctUINTPTR_T)pBasicBlock->globalBbId, pBasicBlock);

    errCode = vscBV_Initialize(&pBasicBlock->domSet, gcvNULL, 0);
    if(errCode != VSC_ERR_NONE)
        return gcvNULL;
    errCode = vscBV_Initialize(&pBasicBlock->postDomSet, gcvNULL, 0);
    if(errCode != VSC_ERR_NONE)
        return gcvNULL;
    errCode = vscBV_Initialize(&pBasicBlock->dfSet, gcvNULL, 0);
    if(errCode != VSC_ERR_NONE)
        return gcvNULL;
    errCode = vscBV_Initialize(&pBasicBlock->cdSet, gcvNULL, 0);
    if(errCode != VSC_ERR_NONE)
        return gcvNULL;

    _InitializeBbReachRelation(&pBasicBlock->globalReachSet, gcvNULL, 0);
    _InitializeBbReachRelation(&pBasicBlock->localReachSet, gcvNULL, 0);

    vscDG_AddNode(&pCfg->dgGraph, &pBasicBlock->dgNode);

    return pBasicBlock;
}

static void _AssociateAnInstToBasicBlock(VIR_BASIC_BLOCK* pBasicBlock, VIR_Instruction* pInst)
{
    VIR_OpCode      opCode = VIR_Inst_GetOpcode(pInst);
    VIR_Function*   pFunc = VIR_Inst_GetFunction(pInst);

    VIR_Inst_SetBasicBlock(pInst, pBasicBlock);
    pBasicBlock->instCount ++;

    if (VIR_OPCODE_isTexLd(opCode) ||
        VIR_OPCODE_isMemLd(opCode) ||
        VIR_OPCODE_isImgLd(opCode) ||
        VIR_OPCODE_isAttrLd(opCode))
    {
        BB_FLAGS_SET_LLI(pBasicBlock);
    }

    if (VIR_Inst_IsHWBarrier(pInst, gcvFALSE))
    {
        BB_FLAGS_SET_HAS_BARRIER(pBasicBlock);
        if (pFunc)
        {
            VIR_Function_SetFlag(pFunc, VIR_FUNCFLAG_HAS_BARRIER);
        }
    }
}

static VSC_ErrCode _AddBasicBlocksToCFG(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_Function* pFunc)
{
    VIR_BASIC_BLOCK*        pEntryBlock;
    VIR_BASIC_BLOCK*        pExitBlock;
    VIR_BASIC_BLOCK*        pThisBlock;
    VIR_Instruction*        pThisInst = gcvNULL;
    VIR_Instruction*        pPrevInst = gcvNULL;
    VIR_InstIterator        instIter;
    VSC_ErrCode             errCode  = VSC_ERR_NONE;

    /* Firstly add an entry block which has id == 0 */
    pEntryBlock = _AddBasicBlockToCFG(pCFG);
    if (!pEntryBlock)
    {
        errCode = VSC_ERR_INVALID_DATA;
        CHECK_ERROR(errCode, "Add basic block");
    }
    pEntryBlock->flowType = VIR_FLOW_TYPE_ENTRY;

    /* Then add an exit block which has id == 1 */
    pExitBlock = _AddBasicBlockToCFG(pCFG);
    if (!pExitBlock)
    {
        errCode = VSC_ERR_INVALID_DATA;
        CHECK_ERROR(errCode, "Add basic block");
    }
    pExitBlock->flowType = VIR_FLOW_TYPE_EXIT;

    /* If no inst in function, just return. So the CFG will only have entry + exit */
    if (VIR_Inst_Count(&pFunc->instList) == 0)
    {
        return errCode;
    }

    /* First normal basic block */
    pThisBlock = _AddBasicBlockToCFG(pCFG);
    if (!pThisBlock)
    {
        errCode = VSC_ERR_INVALID_DATA;
        CHECK_ERROR(errCode, "Add basic block");
    }

    /* Go through all instructions with this function to build basic blocks and add it to CFG */
    VIR_InstIterator_Init(&instIter, &pFunc->instList);
    pThisInst = (VIR_Instruction *)VIR_InstIterator_First(&instIter);
    pThisBlock->pStartInst = pThisInst;
    for (; pThisInst != gcvNULL; pThisInst = (VIR_Instruction *)VIR_InstIterator_Next(&instIter))
    {
        /* End current basic block and start a new basic block */
        if ((pPrevInst != gcvNULL) &&
            (
            /* Call is regarded as an individual BB for potential IPA */
            VIR_OPCODE_isCall(VIR_Inst_GetOpcode(pPrevInst))   ||
            VIR_OPCODE_isCall(VIR_Inst_GetOpcode(pThisInst))   ||
            /* Branch will end a bb, while branch-target will start a bb */
            VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pPrevInst)) ||
            (VIR_Inst_GetOpcode(pThisInst) == VIR_OP_LABEL)
            ))
        {
            pThisBlock->pEndInst = pPrevInst;
            pThisBlock->flowType = _GetFlowType(VIR_Inst_GetOpcode(pPrevInst));

            /* Start a new block */
            pThisBlock = _AddBasicBlockToCFG(pCFG);
            if (!pThisBlock)
            {
                errCode = VSC_ERR_INVALID_DATA;
                CHECK_ERROR(errCode, "Add basic block");
            }
            pThisBlock->pStartInst = pThisInst;
        }

        _AssociateAnInstToBasicBlock(pThisBlock, pThisInst);

        pPrevInst = pThisInst;
    }

    /* The last block must be got an end */
    pThisBlock->pEndInst = pPrevInst;
    pThisBlock->flowType = _GetFlowType(VIR_Inst_GetOpcode(pPrevInst));
    return errCode;
}

static VIR_CFG_EDGE* _AddEdgeForCFG(VIR_CONTROL_FLOW_GRAPH* pCfg, VIR_BASIC_BLOCK* pFromBB,
                           VIR_BASIC_BLOCK* pToBB, VIR_CFG_EDGE_TYPE edgeType)
{
    VIR_CFG_EDGE*           pEdge;

    /* We only consider successor edge */
    pEdge = (VIR_CFG_EDGE*)vscDG_AddEdge(&pCfg->dgGraph, &pFromBB->dgNode, &pToBB->dgNode, gcvNULL);
    if(!pEdge)
        return gcvNULL;
    pEdge->type = edgeType;
    (pEdge + 1)->type = edgeType;
    pEdge->dfsType = VIR_CFG_DFS_EDGE_TYPE_NORMAL;
    (pEdge + 1)->dfsType = VIR_CFG_DFS_EDGE_TYPE_NORMAL;

    return pEdge;
}

static void _AddEdgesForCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pEntryBlock = (VIR_BASIC_BLOCK*)DGNLST_GET_FIRST_NODE(&pCFG->dgGraph.nodeList);
    VIR_BASIC_BLOCK*        pExitBlock = (VIR_BASIC_BLOCK*)DGND_GET_NEXT_NODE(&pEntryBlock->dgNode);
    VIR_BASIC_BLOCK*        pStartBlock = (VIR_BASIC_BLOCK*)DGND_GET_NEXT_NODE(&pExitBlock->dgNode);
    VIR_BASIC_BLOCK*        pThisBlock;
    VIR_BASIC_BLOCK*        pBranchTargetBB;

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        /* Don't do anything for entry and exit at this time */
        if (pThisBlock == pEntryBlock || pThisBlock == pExitBlock)
        {
            continue;
        }

        /* We must add an edge between entry block and first block of this function */
        if (pThisBlock == pStartBlock)
        {
            _AddEdgeForCFG(pCFG, pEntryBlock, pThisBlock, VIR_CFG_EDGE_TYPE_ALWAYS);
        }

        gcmASSERT(pThisBlock->pEndInst != gcvNULL);

        /* Ok, we can handle normal blocks now */
        if (VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(pThisBlock->pEndInst)))
        {
            pBranchTargetBB = VIR_Inst_GetBranchTargetBB(pThisBlock->pEndInst);

            /* Conditional branch case */
            if (VIR_OPCODE_isConditionBranch(VIR_Inst_GetOpcode(pThisBlock->pEndInst)))
            {
                /* Add a TRUE edge to next */
                _AddEdgeForCFG(pCFG, pThisBlock,
                               DGND_GET_NEXT_NODE(&pThisBlock->dgNode) ?
                               (VIR_BASIC_BLOCK*)DGND_GET_NEXT_NODE(&pThisBlock->dgNode) : pExitBlock,
                               VIR_CFG_EDGE_TYPE_TRUE);

                /* Add a FALSE edge to its branch target */
                _AddEdgeForCFG(pCFG, pThisBlock,
                               pBranchTargetBB,
                               VIR_CFG_EDGE_TYPE_FALSE);
            }
            /* Non-conditional branch case */
            else
            {
                /* Just add ALWAYS edge to branch target */
                _AddEdgeForCFG(pCFG, pThisBlock,
                               pBranchTargetBB,
                               VIR_CFG_EDGE_TYPE_ALWAYS);
            }
        }
        else if (VIR_Inst_GetOpcode(pThisBlock->pEndInst) == VIR_OP_RET)
        {
            /* Add a ALWAYS edge to exit block from RET basic block */
            _AddEdgeForCFG(pCFG, pThisBlock, pExitBlock, VIR_CFG_EDGE_TYPE_ALWAYS);
        }
        else if (DGND_GET_NEXT_NODE(&pThisBlock->dgNode))
        {
            /* Add a ALWAYS edge to next */
            _AddEdgeForCFG(pCFG, pThisBlock,
                           (VIR_BASIC_BLOCK*)DGND_GET_NEXT_NODE(&pThisBlock->dgNode),
                           VIR_CFG_EDGE_TYPE_ALWAYS);
        }
    }

    /* Connect end block with EXIT */
    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        /* Exit must be at the end */
        if (pThisBlock == pExitBlock)
        {
            continue;
        }

        if (DGND_GET_OUT_DEGREE(&pThisBlock->dgNode) == 0)
        {
            _AddEdgeForCFG(pCFG, pThisBlock,
                           pExitBlock,
                           VIR_CFG_EDGE_TYPE_ALWAYS);
            break;
        }
    }
}

/* Callback for unreachability of basic block */
static gctBOOL _RootBasicBlkHandlerDFSPostFromHead(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_BASIC_BLOCK* pBasicBlk, void* pNonParam)
{
    if (pBasicBlk == (VIR_BASIC_BLOCK*)DGNLST_GET_FIRST_NODE(&pCFG->dgGraph.nodeList))
    {
        /* Only start traversaling from entry block */
        return gcvFALSE;
    }

    return gcvTRUE;
}

static gctBOOL _RootBasicBlkHandlerDFSPostFromTail(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_BASIC_BLOCK* pBasicBlk, void* pNonParam)
{
    VSC_BI_LIST_NODE*       pSecondNode = pCFG->dgGraph.nodeList.pHead->pNextNode;

    if (pSecondNode == gcvNULL)
    {
        return gcvTRUE;
    }

    if (pBasicBlk == (VIR_BASIC_BLOCK*)CAST_BLN_2_DGND(pSecondNode))
    {
        /* Only start traversaling from exit block */
        return gcvFALSE;
    }

    return gcvTRUE;
}

/* Remove basic block from CFG. */
static void _RemoveBasicBlockFromCFG(
    VIR_CONTROL_FLOW_GRAPH* pCFG,
    VIR_BASIC_BLOCK* pBasicBlk,
    gctBOOL bDeleteInst)
{
    VIR_Instruction*       pInst = BB_GET_START_INST(pBasicBlk);
    VIR_Instruction*       pEndInst = BB_GET_END_INST(pBasicBlk);
    VIR_Instruction*       pNextInst;

    /* Remove node from graph */
    vscDG_RemoveNode(&pCFG->dgGraph, &pBasicBlk->dgNode);

    vscBV_Finalize(&pBasicBlk->domSet);
    vscBV_Finalize(&pBasicBlk->postDomSet);
    vscBV_Finalize(&pBasicBlk->dfSet);
    vscBV_Finalize(&pBasicBlk->cdSet);

    _FinalizeBbReachRelation(&pBasicBlk->globalReachSet);
    _FinalizeBbReachRelation(&pBasicBlk->localReachSet);

    vscHTBL_DirectRemove(&pCFG->pOwnerFuncBlk->pOwnerCG->globalBbHashTable,
                         (void*)(gctUINTPTR_T)pBasicBlk->globalBbId);
    pBasicBlk->globalBbId = INVALID_GLOBAL_BB_ID;

    while (pInst)
    {
        pNextInst = VIR_Inst_GetNext(pInst);

        if (bDeleteInst)
        {
            VIR_OpCode  opCode = VIR_Inst_GetOpcode(pInst);

            /*
            ** If this instruction is a CALL, we need to remove it from:
            ** 1) The call site array of the callee function.
            ** 2) The mixed call site array of the caller function.
            **
            */
            if (opCode == VIR_OP_CALL)
            {
                VIR_Function*               pCalleeFunc = VIR_Inst_GetCallee(pInst);
                VIR_FUNC_BLOCK*             pCalleeFuncBlk = VIR_Function_GetFuncBlock(pCalleeFunc);
                VIR_Function*               pCallerFunc = VIR_Inst_GetFunction(pInst);
                VIR_FUNC_BLOCK*             pCallerFuncBlk = VIR_Function_GetFuncBlock(pCallerFunc);

                if (pCalleeFuncBlk)
                {
                    VIR_CG_EDGE*                pCallerEdge;
                    VSC_ADJACENT_LIST_ITERATOR  callerIter;
                    gctBOOL                     bFound = gcvFALSE;

                    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pCalleeFuncBlk->dgNode.predList);

                    for (pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&callerIter);
                         pCallerEdge != gcvNULL;
                         pCallerEdge = (VIR_CG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&callerIter))
                    {
                        /* Call site info is only stored at successor edge */
                        pCallerEdge = CG_PRED_EDGE_TO_SUCC_EDGE(pCallerEdge);

                        bFound = vscSRARR_RemoveElementByContent(&pCallerEdge->callSiteArray, (void*)&pInst);
                        if (bFound)
                        {
                            break;
                        }
                    }
                }

                if (pCallerFuncBlk)
                {
                    vscSRARR_RemoveElementByContent(&pCallerFuncBlk->mixedCallSiteArray, (void*)&pInst);
                }
            }
            /* We need to make sure that no references or change all references to NOPs. */
            else if (opCode == VIR_OP_LABEL)
            {
                VIR_Label*          pLabel = VIR_Inst_GetJmpLabel(pInst);
                VIR_Instruction*    pJmpInst = gcvNULL;
                VIR_Link*           pLink = VIR_Label_GetReference(pLabel);

                while (pLink)
                {
                    pJmpInst = (VIR_Instruction*)VIR_Link_GetReference(pLink);
                    pLink = pLink->next;

                    VIR_Function_ChangeInstToNop(pCFG->pOwnerFuncBlk->pVIRFunc, pJmpInst);
                }
            }

            VIR_Function_DeleteInstruction(pCFG->pOwnerFuncBlk->pVIRFunc, pInst);
        }
        else
        {
            VIR_Inst_SetFunction(pInst, pCFG->pOwnerFuncBlk->pVIRFunc);
        }

        /* If current inst is the last inst of block, just bail out */
        if (pInst == pEndInst)
        {
            break;
        }

        /* Move to next inst */
        pInst = pNextInst;
    }

    vscDGND_Finalize(&pBasicBlk->dgNode);

    /* Free this node */
    vscMM_Free(&pCFG->pmp.mmWrapper, pBasicBlk);
}

static gctBOOL _RemoveUnreachableBasicBlocks(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VIR_Shader*             pShader = VIR_Function_GetShader(pCFG->pOwnerFuncBlk->pVIRFunc);
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pThisBlock;
    gctBOOL                 bRemoved = gcvFALSE;

    /* I: do the post-order CFG from the head node: for those not-visisted nodes, they are unreachable. */
    vscDG_TraversalCB(&pCFG->dgGraph,
                      VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                      gcvFALSE,
                      (PFN_DG_NODE_HANLDER)_RootBasicBlkHandlerDFSPostFromHead,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL);

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        /* Any unvisited one is unreachable, so remove it from graph */
        if (!pThisBlock->dgNode.bVisited)
        {
            _RemoveBasicBlockFromCFG(pCFG, pThisBlock, gcvTRUE);

            bRemoved = gcvTRUE;
        }
    }

    /*
    ** II: do the post-order CFG from the tail node:
    **  for those not-visisted nodes, they may be reachable, but they can't reach the exit node, so we can still remove them.
    */
    /* So far we only enable this optimization for spir-v case only. */
    if (!VIR_Shader_IsVulkan(pShader))
    {
        return bRemoved;
    }

    vscDG_TraversalCB(&pCFG->dgGraph,
                      VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                      gcvTRUE,
                      (PFN_DG_NODE_HANLDER)_RootBasicBlkHandlerDFSPostFromTail,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL,
                      gcvNULL);

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
        /* Any unvisited one is unreachable, so remove it from graph */
        if (!pThisBlock->dgNode.bVisited)
        {
            _RemoveBasicBlockFromCFG(pCFG, pThisBlock, gcvTRUE);

            bRemoved = gcvTRUE;
        }
    }

    return bRemoved;
}

VSC_ErrCode vscVIR_BuildCFGPerFunc(VSC_MM* pScratchMemPool, VIR_Function* pFunc)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* pCFG;

    /* If no CG built, just return */
    if (pFunc->pFuncBlock == gcvNULL)
    {
        return VSC_ERR_CG_NOT_BUILT;
    }

    pCFG = &pFunc->pFuncBlock->cfg;

    /* Intialize CFG */
    _IntializeCFG(pCFG, pScratchMemPool, pFunc->pFuncBlock);

    /* Try add all basic blocks to CFG */
    errCode = _AddBasicBlocksToCFG(pCFG, pFunc);
    CHECK_ERROR(errCode, "Build CFG per function");

    /* Go through all basic blocks in CFG to build topology by branch info */
    _AddEdgesForCFG(pCFG);

    /* Remove all unreachable basic blocks */
    _RemoveUnreachableBasicBlocks(pCFG);

    /* A cfg must have only one entry block and exit block */
    gcmASSERT(vscDG_GetRootCount(&pCFG->dgGraph) == 1 && vscDG_GetTailCount(&pCFG->dgGraph) == 1);

    return errCode;
}

VSC_ErrCode vscVIR_DestroyCFGPerFunc(VIR_Function* pFunc)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pThisBlock;
    VIR_BASIC_BLOCK*        pNextBlock;
    VIR_CONTROL_FLOW_GRAPH* pCFG;

    /* If no CG built, just return */
    if (pFunc->pFuncBlock == gcvNULL)
    {
        return errCode;
    }

    pCFG = &pFunc->pFuncBlock->cfg;

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; )
    {
        pNextBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter);

        _RemoveBasicBlockFromCFG(pCFG, pThisBlock, gcvFALSE);

        pThisBlock = pNextBlock;
    }

    _FinalizeCFG(pCFG);

    return errCode;
}

VSC_ErrCode vscVIR_BuildCFG(VSC_MM* pScratchMemPool, VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;

        errCode = vscVIR_BuildCFGPerFunc(pScratchMemPool, pFunc);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return vscVIR_BuildBbReachRelation(pShader);
}

VSC_ErrCode vscVIR_DestroyCFG(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    if (!vscVIR_IsCFGBuilt(pShader))
    {
        return errCode;
    }

    errCode = vscVIR_DestroyBbReachRelation(pShader);
    if (errCode != VSC_ERR_NONE)
    {
        return errCode;
    }

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;

        errCode = vscVIR_DestroyCFGPerFunc(pFunc);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

gctBOOL vscVIR_IsCFGBuilt(VIR_Shader* pShader)
{
    if (pShader && pShader->mainFunction && pShader->mainFunction->pFuncBlock)
    {
        return (pShader->mainFunction->pFuncBlock->cfg.pOwnerFuncBlk != gcvNULL);
    }

    return gcvFALSE;
}

VIR_BASIC_BLOCK* vscVIR_AddBasicBlockToCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                           VIR_Instruction*        pStartInst,
                                           VIR_Instruction*        pEndInst,
                                           VIR_FLOW_TYPE           flowType)
{
    VIR_Instruction*        pThisInst = pStartInst;
    VIR_BASIC_BLOCK*        pNewBasicBlk = gcvNULL;

    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);
    gcmASSERT(VIR_Inst_GetBasicBlock(pStartInst) == gcvNULL && VIR_Inst_GetBasicBlock(pEndInst) == gcvNULL);

    pNewBasicBlk = _AddBasicBlockToCFG(pCFG);

    pNewBasicBlk->pStartInst = pStartInst;
    pNewBasicBlk->pEndInst = pEndInst;
    pNewBasicBlk->flowType = flowType;

    while (pThisInst)
    {
        _AssociateAnInstToBasicBlock(pNewBasicBlk, pThisInst);

        /* If current inst is the last inst of block, just bail out */
        if (pThisInst == pEndInst)
        {
            break;
        }

        /* Move to next inst */
        pThisInst = VIR_Inst_GetNext(pThisInst);
    }

    return pNewBasicBlk;
}

VSC_ErrCode vscVIR_RemoveBasicBlockFromCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                           VIR_BASIC_BLOCK* pBasicBlk,
                                           gctBOOL bDeleteInst)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);

    _RemoveBasicBlockFromCFG(pCFG, pBasicBlk, bDeleteInst);

    return errCode;
}

VSC_ErrCode vscVIR_AddEdgeToCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                VIR_BASIC_BLOCK*        pFromBasicBlk,
                                VIR_BASIC_BLOCK*        pToBasicBlk,
                                VIR_CFG_EDGE_TYPE       edgeType)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;
    VIR_CFG_EDGE*           pEdge;

    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);

    pEdge = _AddEdgeForCFG(pCFG, pFromBasicBlk, pToBasicBlk, edgeType);

    if(pEdge == gcvNULL)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        return errCode;
    }

    return errCode;
}

VSC_ErrCode vscVIR_RemoveEdgeFromCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                     VIR_BASIC_BLOCK*        pFromBasicBlk,
                                     VIR_BASIC_BLOCK*        pToBasicBlk)
{
    VSC_ErrCode             errCode = VSC_ERR_NONE;

    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);
    gcmASSERT(vscVIR_GetCfgEdge(pCFG, pFromBasicBlk, pToBasicBlk));

    vscDG_RemoveEdge(&pCFG->dgGraph, &pFromBasicBlk->dgNode, &pToBasicBlk->dgNode);

    return errCode;
}

VIR_CFG_EDGE* vscVIR_GetCfgEdge(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                VIR_BASIC_BLOCK* pFromBasicBlk,
                                VIR_BASIC_BLOCK* pToBasicBlk)
{
    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);

    return (VIR_CFG_EDGE*)vscDG_GetEdge(&pCFG->dgGraph, &pFromBasicBlk->dgNode, &pToBasicBlk->dgNode);
}

VIR_BASIC_BLOCK* VIR_BB_GetFirstSuccBB(VIR_BASIC_BLOCK* bb)
{
    gcmASSERT(BB_GET_OUT_DEGREE(bb) <= 2);

    if(BB_GET_OUT_DEGREE(bb))
    {
        return CFG_EDGE_GET_TO_BB((VIR_CFG_EDGE*)bb->dgNode.succList.pHead);
    }

    return gcvNULL;
}

VIR_BASIC_BLOCK* VIR_BB_GetSecondSuccBB(VIR_BASIC_BLOCK* bb)
{
    gcmASSERT(BB_GET_OUT_DEGREE(bb) <= 2);

    if(BB_GET_OUT_DEGREE(bb) == 2)
    {
        return CFG_EDGE_GET_TO_BB((VIR_CFG_EDGE*)bb->dgNode.succList.pTail);
    }

    return gcvNULL;
}

VIR_BASIC_BLOCK* VIR_BB_GetLeadingBB(VIR_BASIC_BLOCK* bb)
{
    if(BB_GET_FLOWTYPE(bb) == VIR_FLOW_TYPE_EXIT)
    {
        return VIR_Inst_GetBasicBlock(VIR_Function_GetInstEnd(BB_GET_FUNC(bb)));
    }
    else
    {
        VIR_Instruction* bbStart = BB_GET_START_INST(bb);
        VIR_Instruction* bbStartPrev;

        gcmASSERT(bbStart);

        bbStartPrev = VIR_Inst_GetPrev(bbStart);

        if(bbStartPrev == gcvNULL)
        {
            return CFG_GET_ENTRY_BB(BB_GET_CFG(bb));
        }
        else
        {
            return VIR_Inst_GetBasicBlock(bbStartPrev);
        }
    }
}

VIR_BASIC_BLOCK* VIR_BB_GetFollowingBB(VIR_BASIC_BLOCK* bb)
{
    if(BB_GET_FLOWTYPE(bb) == VIR_FLOW_TYPE_ENTRY)
    {
        return VIR_Inst_GetBasicBlock(VIR_Function_GetInstStart(BB_GET_FUNC(bb)));
    }
    else
    {
        VIR_Instruction* bbEnd = BB_GET_END_INST(bb);
        VIR_Instruction* bbEndNext;

        gcmASSERT(bbEnd);

        bbEndNext = VIR_Inst_GetNext(bbEnd);

        if(bbEndNext == gcvNULL)
        {
            return CFG_GET_EXIT_BB(BB_GET_CFG(bb));
        }
        else
        {
            return VIR_Inst_GetBasicBlock(bbEndNext);
        }
    }
}

VIR_BASIC_BLOCK* VIR_BB_GetJumpToBB(VIR_BASIC_BLOCK* bb)
{
    VIR_Instruction* bbEnd = BB_GET_END_INST(bb);

    if(bbEnd == gcvNULL || !VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(bbEnd)))
    {
        return gcvNULL;
    }
    else
    {
        return VIR_Inst_GetBranchTargetBB(bbEnd);
    }
}

VSC_ErrCode
VIR_BB_ChangeSuccBBs(
    VIR_BASIC_BLOCK* bb,
    VIR_BASIC_BLOCK* newJmpTo,
    VIR_BASIC_BLOCK* newFallThru
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = BB_GET_FUNC(bb);
    VIR_Instruction* bbEnd = BB_GET_END_INST(bb);
    VIR_Operand* bbEndDest = VIR_Inst_GetDest(bbEnd);
    VIR_OpCode bbEndOp = VIR_Inst_GetOpcode(bbEnd);

    VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
    VIR_CFG_EDGE* succEdge;

    /* update bb's succ bb */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &bb->dgNode.succList);
    succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
    for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
    {
        VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);
        gctBOOL remove = gcvFALSE;

        if((newJmpTo && CFG_EDGE_GET_TYPE(succEdge) == VIR_CFG_EDGE_TYPE_FALSE))
        {
            gcmASSERT(bbEndOp == VIR_OP_JMPC);
            remove = gcvTRUE;
        }
        else if((newJmpTo && CFG_EDGE_GET_TYPE(succEdge) == VIR_CFG_EDGE_TYPE_ALWAYS))
        {
            gcmASSERT(bbEndOp == VIR_OP_JMP);
            remove = gcvTRUE;
        }
        else if((newFallThru && CFG_EDGE_GET_TYPE(succEdge) == VIR_CFG_EDGE_TYPE_ALWAYS))
        {
            gcmASSERT(!VIR_OPCODE_isBranch(bbEndOp));
            remove = gcvTRUE;
        }
        else if((newFallThru && CFG_EDGE_GET_TYPE(succEdge) == VIR_CFG_EDGE_TYPE_TRUE))
        {
            gcmASSERT(bbEndOp == VIR_OP_JMPC);
            remove = gcvTRUE;
        }

        if(remove)
        {
            vscVIR_RemoveEdgeFromCFG(BB_GET_CFG(bb), bb, succBB);
        }
    }

    /* add new edges */
    if(newJmpTo)
    {
        VIR_Instruction* newJmpToStart = BB_GET_START_INST(newJmpTo);
        VIR_Label* label;
        VIR_Link* link;

        gcmASSERT(bbEndOp == VIR_OP_JMP ||
                  bbEndOp == VIR_OP_JMPC);

        /* remove old link */
        label = VIR_Operand_GetLabel(bbEndDest);
        if(label)
        {
            link = VIR_Link_RemoveLink(VIR_Label_GetReferenceAddr(label), (gctUINTPTR_T)bbEnd);

            if(link)
            {
                VIR_Function_FreeLink(func, link);
            }
        }

        errCode = vscVIR_AddEdgeToCFG(BB_GET_CFG(bb),
                                      bb,
                                      newJmpTo,
                                      bbEndOp == VIR_OP_JMP ? VIR_CFG_EDGE_TYPE_ALWAYS : VIR_CFG_EDGE_TYPE_FALSE);
        if(errCode)
        {
            return errCode;
        }

        if(bbEndOp == VIR_OP_JMP && newJmpTo == VIR_BB_GetFollowingBB(bb))
        {
            VIR_Function_ChangeInstToNop(func, bbEnd);  /*it's better to keep the instruction here, because removing it may empty the bb */
            BB_SET_FLOWTYPE(bb, VIR_FLOW_TYPE_NONE);
        }
        else
        {
            /* reset label in jmp instruction */
            if(VIR_Inst_GetOpcode(newJmpToStart) == VIR_OP_LABEL)
            {
                label = VIR_Operand_GetLabel(VIR_Inst_GetDest(newJmpToStart));
            }
            else
            {
                VIR_LabelId newLabelId;
                VIR_Label* newLabel;
                VIR_Instruction* newLabelInst;

                VIR_Function_AddLabel(func, gcvNULL, &newLabelId);
                newLabel = VIR_Function_GetLabelFromId(func, newLabelId);
                VIR_Function_AddInstructionBefore(func, VIR_OP_LABEL, VIR_TYPE_UNKNOWN, newJmpToStart, gcvTRUE, &newLabelInst);
                VIR_Operand_SetLabel(VIR_Inst_GetDest(newLabelInst), newLabel);
                VIR_Label_SetDefInst(newLabel, newLabelInst);
                label = newLabel;
            }
            VIR_Operand_SetLabel(bbEndDest, label);
            VIR_Function_NewLink(func, &link);
            VIR_Link_SetReference(link, (gctUINTPTR_T)bbEnd);
            VIR_Link_AddLink(VIR_Label_GetReferenceAddr(label), link);
        }
    }

    if(newFallThru)
    {
        gcmASSERT(VIR_BB_GetFollowingBB(bb) == newFallThru);

        errCode = vscVIR_AddEdgeToCFG(BB_GET_CFG(bb),
                                      bb,
                                      newFallThru,
                                      VIR_Inst_GetOpcode(bbEnd) == VIR_OP_JMPC ? VIR_CFG_EDGE_TYPE_TRUE : VIR_CFG_EDGE_TYPE_ALWAYS);
        if(errCode)
        {
            return errCode;
        }
    }

    return errCode;
}

void
VIR_BB_RemoveBranch(
    VIR_BASIC_BLOCK* bb,
    gctBOOL setNop
    )
{
    VIR_Instruction* bbEnd = BB_GET_END_INST(bb);

    gcmASSERT(VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(bbEnd)));

    if(setNop)
    {
        VIR_Function_ChangeInstToNop(BB_GET_FUNC(bb), bbEnd);
    }
    else
    {
        VIR_Function_RemoveInstruction(BB_GET_FUNC(bb), bbEnd, gcvTRUE);
    }
    {
        VSC_ADJACENT_LIST_ITERATOR succEdgeIter;
        VIR_CFG_EDGE* succEdge;


        /* update bb's succ bb */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &bb->dgNode.succList);
        succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; succEdge != gcvNULL; succEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            VIR_BB* succBB = CFG_EDGE_GET_TO_BB(succEdge);

            vscVIR_RemoveEdgeFromCFG(BB_GET_CFG(bb), bb, succBB);
        }
        vscVIR_AddEdgeToCFG(BB_GET_CFG(bb), bb, VIR_BB_GetFollowingBB(bb), VIR_CFG_EDGE_TYPE_ALWAYS);
    }
}

VSC_ErrCode
VIR_BB_Append(
    VIR_BASIC_BLOCK* target,
    VIR_BASIC_BLOCK* source,
    gctBOOL beforeTargetBranch,
    gctBOOL sourceBranchExcluded
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = BB_GET_FUNC(target);
    VIR_Instruction* targetEnd = BB_GET_END_INST(target);
    VIR_Instruction* sourceIter = BB_GET_START_INST(source);

    while(VIR_Inst_GetOpcode(sourceIter) == VIR_OP_LABEL)
    {
        sourceIter = VIR_Inst_GetNext(sourceIter);
    }

    if(beforeTargetBranch && VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(targetEnd)))
    {
        targetEnd = VIR_Inst_GetPrev(targetEnd);
        gcmASSERT(!VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(targetEnd)));
    }

    while(gcvTRUE)
    {
        if(sourceBranchExcluded && VIR_OPCODE_isBranch(VIR_Inst_GetOpcode(sourceIter)))
        {
            break;
        }

        errCode = VIR_Function_AddCopiedInstructionAfter(func, sourceIter, targetEnd, gcvTRUE, &targetEnd);

        if(errCode == VSC_ERR_NONE && sourceIter == BB_GET_END_INST(source))
        {
            break;
        }

        sourceIter = VIR_Inst_GetNext(sourceIter);
    }

    return errCode;
}

VSC_ErrCode
VIR_BB_CopyBBBefore(
    VIR_BASIC_BLOCK* source,
    VIR_BASIC_BLOCK* before,
    VIR_BASIC_BLOCK** copy
    )
{
    if(BB_GET_FLOWTYPE(before) == VIR_FLOW_TYPE_EXIT)
    {
        return VIR_BB_CopyBBAfter(source, VIR_BB_GetLeadingBB(before), copy);
    }
    else
    {
        VSC_ErrCode errCode = VSC_ERR_NONE;
        VIR_Function* func = BB_GET_FUNC(source);
        VIR_Instruction* newBBStart = gcvNULL;
        VIR_Instruction* newBBEnd = gcvNULL;
        VIR_Instruction* bbInst = BB_GET_START_INST(source);
        VIR_Instruction* beforeHead = BB_GET_START_INST(before);
        VIR_BB* newBB;

        gcmASSERT(func == BB_GET_FUNC(before));
        gcmASSERT(BB_GET_LENGTH(source));

        while(gcvTRUE)
        {
            VIR_Instruction* newBBInst;

            errCode = VIR_Function_AddCopiedInstructionBefore(func, bbInst, beforeHead, gcvFALSE, &newBBInst);
            if(errCode)
            {
                return errCode;
            }

            if(newBBStart == gcvNULL)
            {
                newBBStart = newBBInst;
            }

            if(bbInst == BB_GET_END_INST(source))
            {
                newBBEnd = newBBInst;
                break;
            }
            else
            {
                bbInst = VIR_Inst_GetNext(bbInst);
            }
        }

        newBB = vscVIR_AddBasicBlockToCFG(BB_GET_CFG(source), newBBStart, newBBEnd, BB_GET_FLOWTYPE(source));

        if(copy)
        {
            *copy = newBB;
        }

        return errCode;
    }
}

VSC_ErrCode
VIR_BB_CopyBBAfter(
    VIR_BASIC_BLOCK* source,
    VIR_BASIC_BLOCK* after,
    VIR_BASIC_BLOCK** copy
    )
{
    if(BB_GET_FLOWTYPE(after) == VIR_FLOW_TYPE_ENTRY)
    {
        return VIR_BB_CopyBBBefore(source, VIR_BB_GetFollowingBB(after), copy);
    }
    else
    {
        VSC_ErrCode errCode = VSC_ERR_NONE;
        VIR_Function* func = BB_GET_FUNC(source);
        VIR_Instruction* newBBStart = gcvNULL;
        VIR_Instruction* newBBEnd = gcvNULL;
        VIR_Instruction* bbInst = BB_GET_END_INST(source);
        VIR_Instruction* afterEnd = BB_GET_END_INST(after);
        VIR_BB* newBB;

        gcmASSERT(func == BB_GET_FUNC(after));

        while(gcvTRUE)
        {
            VIR_Instruction* newBBInst;

            errCode = VIR_Function_AddCopiedInstructionAfter(func, bbInst, afterEnd, gcvFALSE, &newBBInst);
            if(errCode)
            {
                return errCode;
            }

            if(newBBEnd == gcvNULL)
            {
                newBBEnd = newBBInst;
            }

            if(bbInst == BB_GET_START_INST(source))
            {
                newBBStart = newBBInst;
                break;
            }
            else
            {
                bbInst = VIR_Inst_GetPrev(bbInst);
            }
        }

        newBB = vscVIR_AddBasicBlockToCFG(BB_GET_CFG(source), newBBStart, newBBEnd, BB_GET_FLOWTYPE(source));

        if(copy)
        {
            *copy = newBB;
        }

        return errCode;
    }
}

VSC_ErrCode
VIR_BB_InsertBBBefore(
    VIR_BB* before,
    VIR_OpCode opcode,
    VIR_BB** newBB
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = BB_GET_FUNC(before);
    VIR_Instruction* newInst;
    VIR_BB* bb;
    VIR_FLOW_TYPE flowType;

    gcmASSERT(opcode == VIR_OP_NOP ||
              opcode == VIR_OP_LABEL ||
              opcode == VIR_OP_JMP ||
              opcode == VIR_OP_JMPC);

    errCode = VIR_Function_AddInstructionBefore(func, opcode, VIR_TYPE_UNKNOWN, BB_GET_START_INST(before), gcvFALSE, &newInst);
    if(errCode)
    {
        return errCode;
    }

    switch(opcode)
    {
        case VIR_OP_JMP:
            flowType = VIR_FLOW_TYPE_JMP;
            break;
        case VIR_OP_JMPC:
            flowType = VIR_FLOW_TYPE_JMPC;
            break;
        case VIR_OP_LABEL:
        {
            VIR_LabelId newLabelId;
            VIR_Label* newLabel;

            VIR_Function_AddLabel(func, gcvNULL, &newLabelId);
            newLabel = VIR_Function_GetLabelFromId(func, newLabelId);
            VIR_Operand_SetLabel(VIR_Inst_GetDest(newInst), newLabel);
            VIR_Label_SetDefInst(newLabel, newInst);
        }
        default:
            flowType = VIR_FLOW_TYPE_NONE;
    }
    bb = vscVIR_AddBasicBlockToCFG(BB_GET_CFG(before), newInst, newInst, flowType);

    if(bb)
    {
        if(newBB)
        {
            *newBB = bb;
        }
    }
    else
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }

    return errCode;
}

VSC_ErrCode
VIR_BB_InsertBBAfter(
    VIR_BB* after,
    VIR_OpCode opcode,
    VIR_BB** newBB
    )
{
    VSC_ErrCode errCode = VSC_ERR_NONE;
    VIR_Function* func = BB_GET_FUNC(after);
    VIR_Instruction* newInst;
    VIR_BB* bb;
    VIR_FLOW_TYPE flowType;

    gcmASSERT(opcode == VIR_OP_NOP ||
              opcode == VIR_OP_LABEL ||
              opcode == VIR_OP_JMP ||
              opcode == VIR_OP_JMPC);

    errCode = VIR_Function_AddInstructionAfter(func, opcode, VIR_TYPE_UNKNOWN, BB_GET_END_INST(after), gcvFALSE, &newInst);
    if(errCode)
    {
        return errCode;
    }

    switch(opcode)
    {
        case VIR_OP_JMP:
            flowType = VIR_FLOW_TYPE_JMP;
            break;
        case VIR_OP_JMPC:
            flowType = VIR_FLOW_TYPE_JMPC;
            break;
        default:
            flowType = VIR_FLOW_TYPE_NONE;
    }
    bb = vscVIR_AddBasicBlockToCFG(BB_GET_CFG(after), newInst, newInst, flowType);

    if(bb)
    {
        if(newBB)
        {
            *newBB = bb;
        }
    }
    else
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
    }

    return errCode;
}

void vscBBWKL_AddBBToWorkItemList(VIR_BB_WORKLIST* pWorkItemList, VIR_BB_WORKITEM* pWorkItem, VIR_BASIC_BLOCK* pBB)
{
    BB_WORKITEM_INIT(pWorkItem, pBB);
    BB_WORKLIST_ADD_WORKITEM(pWorkItemList, pWorkItem);
    pBB->bInWorklist = gcvTRUE;
}

VIR_BASIC_BLOCK* vscBBWKL_RemoveBBFromWorkItemList(VIR_BB_WORKLIST* pWorkItemList)
{
    VIR_BB_WORKITEM*             pWorkItem;
    VIR_BASIC_BLOCK*             pBB;

    pWorkItem = BB_WORKLIST_GET_WORKITEM(pWorkItemList);
    pBB = BB_WORKITEM_GET_BB(pWorkItem);
    pBB->bInWorklist = gcvFALSE;
    BB_WORKITEM_FINALIZE(pWorkItem);

    return pBB;
}

/*
 * (post-) DOM related code
 */

static VIR_DOM_TREE_NODE* _AddDomNodeToDomTree(VIR_DOM_TREE* pDomTree, VIR_DOM_TREE_NODE* pParentDomNode,
                                               VIR_BASIC_BLOCK* pBasicBlock, gctBOOL bPostDom)
{
    VIR_DOM_TREE_NODE* pDomTreeNode = gcvNULL;

    pDomTreeNode = (VIR_DOM_TREE_NODE*)vscMM_Alloc(&pDomTree->pOwnerCFG->pmp.mmWrapper,
                                                   sizeof(VIR_DOM_TREE_NODE));
    if (pDomTreeNode == gcvNULL)
    {
        return gcvNULL;
    }

    vscTREEND_Initialize(&pDomTreeNode->treeNode);
    pDomTreeNode->pOwnerBB = pBasicBlock;

    if (bPostDom)
    {
        pBasicBlock->pPostDomTreeNode = pDomTreeNode;
    }
    else
    {
        pBasicBlock->pDomTreeNode = pDomTreeNode;
    }

    vscTREE_AddSubTree(&pDomTree->tree,
                       pParentDomNode ? &pParentDomNode->treeNode : gcvNULL,
                       &pDomTreeNode->treeNode);

    return pDomTreeNode;
}

static void _RemoveDomNodeFromDomTree(VIR_DOM_TREE* pDomTree, VIR_DOM_TREE_NODE* pDomTreeNode, gctBOOL bPostDom)
{
    if (bPostDom)
    {
        pDomTreeNode->pOwnerBB->pPostDomTreeNode = gcvNULL;
        vscBV_Finalize(&pDomTreeNode->pOwnerBB->postDomSet);
    }
    else
    {
        pDomTreeNode->pOwnerBB->pDomTreeNode = gcvNULL;
        vscBV_Finalize(&pDomTreeNode->pOwnerBB->domSet);
    }

    vscTREEND_Finalize(&pDomTreeNode->treeNode);

    /* DON'T call vscTREE_RemoveSubTree since it make all nodes as descendants of pDomTreeNode be removed
       from tree, thus make remove be complex */

    vscMM_Free(&pDomTree->pOwnerCFG->pmp.mmWrapper, pDomTreeNode);
}

VSC_ErrCode vscVIR_BuildDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK**            ppBasicBlkPO;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    gctUINT                      hisCountOfBasicBlk = vscDG_GetHistNodeCount(&pCFG->dgGraph);
    VIR_BB_WORKITEM*             pWorkItemArray;
    VIR_BB_WORKLIST              workItemList;
    VSC_MM*                      pScratchMemPool = gcvNULL;
    gctUINT                      bbIdx, idomBBIdx, domCount;
    gctINT                       bbIdxS, bbIdxT;
    VSC_BIT_VECTOR               workingSet;
    VSC_BIT_VECTOR*              pIdomSetArray;
    VSC_BIT_VECTOR*              pDomSetArray;
    VSC_BIT_VECTOR*              pCurIdomSet;
    VSC_BIT_VECTOR*              pDomSetS;
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VIR_BASIC_BLOCK**            ppHisBlockArray;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter, succEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_CFG_EDGE*                pSuccEdge;

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* If tree has been already built, just bail out */
    if (vscTREE_GetNodeCount(&pCFG->domTree.tree) != 0)
    {
        return errCode;
    }

    gcmASSERT(pCFG->pOwnerFuncBlk != gcvNULL);
    pScratchMemPool = pCFG->pOwnerFuncBlk->pOwnerCG->pScratchMemPool;
    gcmASSERT(pScratchMemPool);

    /*
     *  Firstly resolve dom set of each block
     */

    /* Get PO of CFG */
    ppBasicBlkPO = (VIR_BASIC_BLOCK**)vscMM_Alloc(pScratchMemPool, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    if (!ppBasicBlkPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build DOM tree");
    }
    errCode = vscDG_PreOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvFALSE,
                            gcvFALSE,
                            (VSC_DG_NODE**)ppBasicBlkPO);
    CHECK_ERROR(errCode, "Build DOM tree");

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    pWorkItemArray = (VIR_BB_WORKITEM*)vscMM_Alloc(pScratchMemPool,
                                                   sizeof(VIR_BB_WORKITEM)*
                                                   hisCountOfBasicBlk);
    if (!pWorkItemArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build DOM tree");
    }

    BB_WORKLIST_INIT(&workItemList);

    /* Initialize workitem list and dom-set of each BB */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        errCode = vscBV_Initialize(&ppBasicBlkPO[bbIdx]->domSet, pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build DOM tree");

        /* Initialize each workitem, and add it to workitem list. Note that entry block won't
           be added into workitem list because it won't be changed when iterating */
        if (ppBasicBlkPO[bbIdx]->flowType == VIR_FLOW_TYPE_ENTRY)
        {
            /* Entry is only dominated by itself */
            vscBV_SetBit(&ppBasicBlkPO[bbIdx]->domSet, ppBasicBlkPO[bbIdx]->dgNode.id);
        }
        else
        {
            vscBBWKL_AddBBToWorkItemList(&workItemList,
                                         &pWorkItemArray[ppBasicBlkPO[bbIdx]->dgNode.id],
                                         ppBasicBlkPO[bbIdx]);

            /* Suppose non-entry BB is dominated by every BB */
            vscBV_SetAll(&ppBasicBlkPO[bbIdx]->domSet);
        }
    }

    /* Prepare working set */
    errCode = vscBV_Initialize(&workingSet, pScratchMemPool, hisCountOfBasicBlk);
    CHECK_ERROR(errCode, "Build DOM tree");

    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(&workItemList);

        vscBV_SetAll(&workingSet);

        /* Iterative resolve each predecessor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
            vscBV_And2(&workingSet, &workingSet, &pPredBasicBlk->domSet);
        }

        /* Each block dominates itself */
        vscBV_SetBit(&workingSet, pThisBasicBlk->dgNode.id);

        /* Check whether working set is same as original set of this BB */
        if (!vscBV_Equal(&workingSet, &pThisBasicBlk->domSet))
        {
            vscBV_Copy(&pThisBasicBlk->domSet, &workingSet);

            VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
            pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
            for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
            {
                pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
                if (!pSuccBasicBlk->bInWorklist)
                {
                    vscBBWKL_AddBBToWorkItemList(&workItemList, &pWorkItemArray[pSuccBasicBlk->dgNode.id], pSuccBasicBlk);
                }
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(&workItemList));

    vscBV_Finalize(&workingSet);

    /*
     * Now we can calc idom
     */

    /* Allocate idom set array corresponding basic block. Note as algorithm will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    pIdomSetArray = (VSC_BIT_VECTOR*)vscMM_Alloc(pScratchMemPool,
                                                   sizeof(VSC_BIT_VECTOR)*
                                                   hisCountOfBasicBlk);
    if (!pIdomSetArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build DOM tree");
    }
    pDomSetArray = (VSC_BIT_VECTOR*)vscMM_Alloc(pScratchMemPool,
                                                sizeof(VSC_BIT_VECTOR)*
                                                hisCountOfBasicBlk);
    if (!pDomSetArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build DOM tree");
    }
    ppHisBlockArray = (VIR_BASIC_BLOCK**)vscMM_Alloc(pScratchMemPool,
                                                     sizeof(VIR_BASIC_BLOCK*)*
                                                     hisCountOfBasicBlk);
    if (!ppHisBlockArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build DOM tree");
    }

    /* Initilize idom set with self bit removed */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        errCode = vscBV_Initialize(&pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build DOM tree");
        vscBV_Copy(&pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], &ppBasicBlkPO[bbIdx]->domSet);
        vscBV_ClearBit(&pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], ppBasicBlkPO[bbIdx]->dgNode.id);

        errCode = vscBV_Initialize(&pDomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build DOM tree");
        vscBV_Copy(&pDomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], &pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);

        /* Initialize worklist again for tree build in last stage */
        vscBBWKL_AddBBToWorkItemList(&workItemList,
                                     &pWorkItemArray[ppBasicBlkPO[bbIdx]->dgNode.id],
                                     ppBasicBlkPO[bbIdx]);
        ppHisBlockArray[ppBasicBlkPO[bbIdx]->dgNode.id] = ppBasicBlkPO[bbIdx];
    }

    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        if (ppBasicBlkPO[bbIdx]->flowType == VIR_FLOW_TYPE_ENTRY)
        {
            continue;
        }

        pCurIdomSet = &pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id];
        domCount = vscBV_CountBits(pCurIdomSet);

        if (domCount == 1)
        {
            continue;
        }

        for (bbIdxS = (gctINT)(bbIdx - 1); bbIdxS >= 0; bbIdxS --)
        {
            if (vscBV_TestBit(pCurIdomSet, ppBasicBlkPO[bbIdxS]->dgNode.id))
            {
                pDomSetS = &pDomSetArray[ppBasicBlkPO[bbIdxS]->dgNode.id];

                for (bbIdxT = bbIdxS - 1; bbIdxT >= 0; bbIdxT --)
                {
                    if (vscBV_TestBit(pDomSetS, ppBasicBlkPO[bbIdxT]->dgNode.id) &&
                        vscBV_TestBit(pCurIdomSet, ppBasicBlkPO[bbIdxT]->dgNode.id))
                    {
                        vscBV_ClearBit(pCurIdomSet, ppBasicBlkPO[bbIdxT]->dgNode.id);
                        domCount --;

                        if (domCount == 1)
                        {
                            break;
                        }
                    }
                }
            }

            if (domCount == 1)
            {
                break;
            }
        }
    }

    /*
     * Finally build tree based on idom
     */

    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(&workItemList);

        if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_ENTRY)
        {
            /* Entry is always the root */
            if (_AddDomNodeToDomTree(&pCFG->domTree, gcvNULL, pThisBasicBlk, gcvFALSE) == gcvNULL)
            {
                errCode = VSC_ERR_OUT_OF_MEMORY;
                CHECK_ERROR(errCode, "Failed in AddDomNodeToDomTree.");
            }
        }
        else
        {
            /* Check if bb of idom is has been in tree, if so, just add this one, otherwise, put it back to work list */
            idomBBIdx = vscBV_FindSetBitForward(&pIdomSetArray[pThisBasicBlk->dgNode.id], 0);
            if (ppHisBlockArray[idomBBIdx]->pDomTreeNode)
            {
                if (_AddDomNodeToDomTree(&pCFG->domTree, ppHisBlockArray[idomBBIdx]->pDomTreeNode, pThisBasicBlk, gcvFALSE) == gcvNULL)
                {
                    errCode = VSC_ERR_OUT_OF_MEMORY;
                    CHECK_ERROR(errCode, "Failed in AddDomNodeToDomTree.");
                }
            }
            else
            {
                vscBBWKL_AddBBToWorkItemList(&workItemList, &pWorkItemArray[pThisBasicBlk->dgNode.id], pThisBasicBlk);
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(&workItemList));

    BB_WORKLIST_FINALIZE(&workItemList);

    /*
     * Delete resources
     */

    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        vscBV_Finalize(&pIdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);
        vscBV_Finalize(&pDomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);
    }
    vscMM_Free(pScratchMemPool, pIdomSetArray);
    vscMM_Free(pScratchMemPool, pDomSetArray);
    vscMM_Free(pScratchMemPool, ppBasicBlkPO);
    vscMM_Free(pScratchMemPool, pWorkItemArray);
    vscMM_Free(pScratchMemPool, ppHisBlockArray);

    return errCode;
}

VSC_ErrCode vscVIR_DestroyDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VSC_TNODE_LIST_ITERATOR treeNodeIter;
    VIR_DOM_TREE*           pDomTree = &pCFG->domTree;
    VIR_DOM_TREE_NODE*      pDomTreeNode;
    VIR_DOM_TREE_NODE*      pLastDomTreeNode = gcvNULL;

    VSC_TNODE_LIST_ITERATOR_INIT(&treeNodeIter, &pDomTree->tree);
    pDomTreeNode = (VIR_DOM_TREE_NODE *)VSC_TNODE_LIST_ITERATOR_FIRST(&treeNodeIter);
    for (; pDomTreeNode != gcvNULL; pDomTreeNode = (VIR_DOM_TREE_NODE *)VSC_TNODE_LIST_ITERATOR_NEXT(&treeNodeIter))
    {
        if(pLastDomTreeNode)
        {
            _RemoveDomNodeFromDomTree(pDomTree, pLastDomTreeNode, gcvFALSE);
        }

        pLastDomTreeNode = pDomTreeNode;
    }
    if(pLastDomTreeNode)
    {
        _RemoveDomNodeFromDomTree(pDomTree, pLastDomTreeNode, gcvFALSE);
    }
    vscTREE_Finalize(&pDomTree->tree);

    return errCode;
}

VSC_ErrCode vscVIR_BuildPostDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_BASIC_BLOCK**            ppBasicBlkPO;
    gctUINT                      countOfBasicBlk = vscDG_GetNodeCount(&pCFG->dgGraph);
    gctUINT                      hisCountOfBasicBlk = vscDG_GetHistNodeCount(&pCFG->dgGraph);
    VIR_BB_WORKITEM*             pWorkItemArray;
    VIR_BB_WORKLIST              workItemList;
    VSC_MM*                      pScratchMemPool;
    gctUINT                      bbIdx, ipdomBBIdx, pdomCount;
    gctINT                       bbIdxS, bbIdxT;
    VSC_BIT_VECTOR               workingSet;
    VSC_BIT_VECTOR*              pIpdomSetArray;
    VSC_BIT_VECTOR*              pPdomSetArray;
    VSC_BIT_VECTOR*              pCurIpdomSet;
    VSC_BIT_VECTOR*              pPdomSetS;
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VIR_BASIC_BLOCK**            ppHisBlockArray;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter, succEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_CFG_EDGE*                pSuccEdge;

    if (countOfBasicBlk == 0)
    {
        return errCode;
    }

    /* If tree has been already built, just bail out */
    if (vscTREE_GetNodeCount(&pCFG->postDomTree.tree) != 0)
    {
        return errCode;
    }

    pScratchMemPool = pCFG->pOwnerFuncBlk->pOwnerCG->pScratchMemPool;
    gcmASSERT(pScratchMemPool);

    /*
     *  Firstly resolve pdom set of each block
     */

    /* Get PO of CFG */
    ppBasicBlkPO = (VIR_BASIC_BLOCK**)vscMM_Alloc(pScratchMemPool, sizeof(VIR_BASIC_BLOCK*)*countOfBasicBlk);
    if (!ppBasicBlkPO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build post DOM tree");
    }

    errCode = vscDG_PreOrderTraversal(&pCFG->dgGraph,
                            VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST,
                            gcvTRUE,
                            gcvFALSE,
                            (VSC_DG_NODE**)ppBasicBlkPO);
    CHECK_ERROR(errCode, "Build post DOM tree");

    /* Allocate workitem array corresponding basic block. Note as iterative analyzer will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    pWorkItemArray = (VIR_BB_WORKITEM*)vscMM_Alloc(pScratchMemPool,
                                                   sizeof(VIR_BB_WORKITEM)*hisCountOfBasicBlk);
    if (!pWorkItemArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build post DOM tree");
    }

    BB_WORKLIST_INIT(&workItemList);

    /* Initialize workitem list and pdom-set of each BB */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        errCode = vscBV_Initialize(&ppBasicBlkPO[bbIdx]->postDomSet, pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build post DOM tree");

        /* Initialize each workitem, and add it to workitem list. Note that exit block won't
           be added into workitem list because it won't be changed when iterating */
        if (ppBasicBlkPO[bbIdx]->flowType == VIR_FLOW_TYPE_EXIT)
        {
            /* Exit is only post dominated by itself */
            vscBV_SetBit(&ppBasicBlkPO[bbIdx]->postDomSet, ppBasicBlkPO[bbIdx]->dgNode.id);
        }
        else
        {
            vscBBWKL_AddBBToWorkItemList(&workItemList,
                                         &pWorkItemArray[ppBasicBlkPO[bbIdx]->dgNode.id],
                                         ppBasicBlkPO[bbIdx]);

            /* Suppose non-exit BB post is dominated by every BB */
            vscBV_SetAll(&ppBasicBlkPO[bbIdx]->postDomSet);
        }
    }

    /* Prepare working set */
    errCode = vscBV_Initialize(&workingSet, pScratchMemPool, hisCountOfBasicBlk);
    CHECK_ERROR(errCode, "Build post DOM tree");

    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(&workItemList);

        vscBV_SetAll(&workingSet);

        /* Iterative resolve each successor */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);
            vscBV_And2(&workingSet, &workingSet, &pSuccBasicBlk->postDomSet);
        }

        /* Each block post-dominates itself */
        vscBV_SetBit(&workingSet, pThisBasicBlk->dgNode.id);

        /* Check whether working set is same as original set of this BB */
        if (!vscBV_Equal(&workingSet, &pThisBasicBlk->postDomSet))
        {
            vscBV_Copy(&pThisBasicBlk->postDomSet, &workingSet);

            VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
            pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
            for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
            {
                pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
                if (!pPredBasicBlk->bInWorklist)
                {
                    vscBBWKL_AddBBToWorkItemList(&workItemList, &pWorkItemArray[pPredBasicBlk->dgNode.id], pPredBasicBlk);
                }
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(&workItemList));

    vscBV_Finalize(&workingSet);

    /*
     * Now we can calc ipdom
     */

    /* Allocate ipdom set array corresponding basic block. Note as algorithm will use id of
       basicblk to index workitem, history node count will be used to allocate these contents */
    pIpdomSetArray = (VSC_BIT_VECTOR*)vscMM_Alloc(pScratchMemPool,
                                                   sizeof(VSC_BIT_VECTOR)*
                                                   hisCountOfBasicBlk);
    if (!pIpdomSetArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build post DOM tree");
    }
    pPdomSetArray = (VSC_BIT_VECTOR*)vscMM_Alloc(pScratchMemPool,
                                                 sizeof(VSC_BIT_VECTOR)*
                                                 hisCountOfBasicBlk);
    if (!pPdomSetArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build post DOM tree");
    }
    ppHisBlockArray = (VIR_BASIC_BLOCK**)vscMM_Alloc(pScratchMemPool,
                                                     sizeof(VIR_BASIC_BLOCK*)*
                                                     hisCountOfBasicBlk);
    if (!ppHisBlockArray)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build post DOM tree");
    }

    /* Initilize ipdom set with self bit removed */
    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        errCode = vscBV_Initialize(&pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build post DOM tree");
        vscBV_Copy(&pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], &ppBasicBlkPO[bbIdx]->postDomSet);
        vscBV_ClearBit(&pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], ppBasicBlkPO[bbIdx]->dgNode.id);

        errCode = vscBV_Initialize(&pPdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], pScratchMemPool, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build post DOM tree");
        vscBV_Copy(&pPdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id], &pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);

        /* Initialize worklist again for tree build in last stage */
        vscBBWKL_AddBBToWorkItemList(&workItemList,
                                     &pWorkItemArray[ppBasicBlkPO[bbIdx]->dgNode.id],
                                     ppBasicBlkPO[bbIdx]);
        ppHisBlockArray[ppBasicBlkPO[bbIdx]->dgNode.id] = ppBasicBlkPO[bbIdx];
    }

    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        if (ppBasicBlkPO[bbIdx]->flowType == VIR_FLOW_TYPE_EXIT)
        {
            continue;
        }

        pCurIpdomSet = &pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id];
        pdomCount = vscBV_CountBits(pCurIpdomSet);

        if (pdomCount == 1)
        {
            continue;
        }

        for (bbIdxS = (gctINT)(bbIdx - 1); bbIdxS >= 0; bbIdxS --)
        {
            if (vscBV_TestBit(pCurIpdomSet, ppBasicBlkPO[bbIdxS]->dgNode.id))
            {
                pPdomSetS = &pPdomSetArray[ppBasicBlkPO[bbIdxS]->dgNode.id];

                for (bbIdxT = bbIdxS - 1; bbIdxT >= 0; bbIdxT --)
                {
                    if (vscBV_TestBit(pPdomSetS, ppBasicBlkPO[bbIdxT]->dgNode.id) &&
                        vscBV_TestBit(pCurIpdomSet, ppBasicBlkPO[bbIdxT]->dgNode.id))
                    {
                        vscBV_ClearBit(pCurIpdomSet, ppBasicBlkPO[bbIdxT]->dgNode.id);
                        pdomCount --;

                        if (pdomCount == 1)
                        {
                            break;
                        }
                    }
                }
            }

            if (pdomCount == 1)
            {
                break;
            }
        }
    }

    /*
     * Finally build tree based on ipdom
     */

    do
    {
        pThisBasicBlk = vscBBWKL_RemoveBBFromWorkItemList(&workItemList);

        if (pThisBasicBlk->flowType == VIR_FLOW_TYPE_EXIT)
        {
            /* Exit is always the root */
            if (_AddDomNodeToDomTree(&pCFG->postDomTree, gcvNULL, pThisBasicBlk, gcvTRUE) == gcvNULL)
            {
                errCode = VSC_ERR_OUT_OF_MEMORY;
                CHECK_ERROR(errCode, "Failed in AddDomNodeToDomTree.");
            }
        }
        else
        {
            /* Check if bb of ipdom is has been in tree, if so, just add this one, otherwise, put it back to work list */
            ipdomBBIdx = vscBV_FindSetBitForward(&pIpdomSetArray[pThisBasicBlk->dgNode.id], 0);
            if (ppHisBlockArray[ipdomBBIdx]->pPostDomTreeNode)
            {
                if (_AddDomNodeToDomTree(&pCFG->postDomTree, ppHisBlockArray[ipdomBBIdx]->pPostDomTreeNode, pThisBasicBlk, gcvTRUE) == gcvNULL)
                {
                    errCode = VSC_ERR_OUT_OF_MEMORY;
                    CHECK_ERROR(errCode, "Failed in AddDomNodeToDomTree.");
                }
            }
            else
            {
                vscBBWKL_AddBBToWorkItemList(&workItemList, &pWorkItemArray[pThisBasicBlk->dgNode.id], pThisBasicBlk);
            }
        }
    }
    while (!BB_WORKLIST_IS_EMPTY(&workItemList));

    BB_WORKLIST_FINALIZE(&workItemList);

    /*
     * Delete resources
     */

    for (bbIdx = 0; bbIdx < countOfBasicBlk; bbIdx ++)
    {
        vscBV_Finalize(&pIpdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);
        vscBV_Finalize(&pPdomSetArray[ppBasicBlkPO[bbIdx]->dgNode.id]);
    }
    vscMM_Free(pScratchMemPool, pIpdomSetArray);
    vscMM_Free(pScratchMemPool, pPdomSetArray);
    vscMM_Free(pScratchMemPool, ppBasicBlkPO);
    vscMM_Free(pScratchMemPool, pWorkItemArray);
    vscMM_Free(pScratchMemPool, ppHisBlockArray);

    return errCode;
}

VSC_ErrCode vscVIR_DestroyPostDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VSC_TNODE_LIST_ITERATOR treeNodeIter;
    VIR_DOM_TREE*           pPostDomTree = &pCFG->postDomTree;
    VIR_DOM_TREE_NODE*      pPostDomTreeNode;

    VSC_TNODE_LIST_ITERATOR_INIT(&treeNodeIter, &pPostDomTree->tree);
    pPostDomTreeNode = (VIR_DOM_TREE_NODE *)VSC_TNODE_LIST_ITERATOR_FIRST(&treeNodeIter);
    for (; pPostDomTreeNode != gcvNULL; pPostDomTreeNode = (VIR_DOM_TREE_NODE *)VSC_TNODE_LIST_ITERATOR_NEXT(&treeNodeIter))
    {
        _RemoveDomNodeFromDomTree(pPostDomTree, pPostDomTreeNode, gcvTRUE);
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildDOMTree(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            return VSC_ERR_CG_NOT_BUILT;
        }

        errCode = vscVIR_BuildDOMTreePerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestroyDOMTree(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            continue;
        }

        errCode = vscVIR_DestroyDOMTreePerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildPostDOMTree(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            return VSC_ERR_CG_NOT_BUILT;
        }

        errCode = vscVIR_BuildPostDOMTreePerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestroyPostDOMTree(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            continue;
        }

        errCode = vscVIR_DestroyPostDOMTreePerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

/*
 * (Reversed-) control dependency related code
 */

VSC_ErrCode vscVIR_BuildControlDepPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_DOM_TREE*                pPostDomTree = &pCFG->postDomTree;
    VIR_DOM_TREE_NODE**          ppPostDomTreePO;
    gctUINT                      countOfPostDomTreeNode = vscTREE_GetNodeCount(&pPostDomTree->tree);
    gctUINT                      hisCountOfBasicBlk = vscDG_GetHistNodeCount(&pCFG->dgGraph);
    VSC_MM*                      pMM = &pCFG->pmp.mmWrapper;
    gctUINT                      tnIdx;
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VIR_BASIC_BLOCK*             pChildBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    DOM_TREE_NODE_CHILD_ITERATOR childIter;
    VIR_DOM_TREE_NODE*           pThisChildNode;
    gctUINT                      childCDIdx;

    if (countOfPostDomTreeNode == 0)
    {
        return errCode;
    }

    /* Get PO of postdom-tree */
    ppPostDomTreePO = (VIR_DOM_TREE_NODE**)vscMM_Alloc(pMM, sizeof(VIR_DOM_TREE_NODE*)*countOfPostDomTreeNode);
    if (!ppPostDomTreePO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build control dependency");
    }

    vscTREE_PstOrderTraversal(&pPostDomTree->tree, (VSC_TREE_NODE**)ppPostDomTreePO);

    /* Let's bottom-up to get cd of each bb */
    for (tnIdx = 0; tnIdx < countOfPostDomTreeNode; tnIdx ++)
    {
        pThisBasicBlk = ppPostDomTreePO[tnIdx]->pOwnerBB;

        /* An init of VB has an implicit full clear */
        errCode = vscBV_Initialize(&pThisBasicBlk->cdSet, pMM, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build control dependency");

        /* Local cd based cfg */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pThisBasicBlk->dgNode.predList);
        pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
        for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
        {
            pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);

            if (!BB_IS_IPDOM(pThisBasicBlk, pPredBasicBlk))
            {
                vscBV_SetBit(&pThisBasicBlk->cdSet, pPredBasicBlk->dgNode.id);
            }
        }

        /* Up cd based on post-dom tree */
        DOM_TREE_NODE_CHILD_ITERATOR_INIT(&childIter, pThisBasicBlk->pPostDomTreeNode);
        pThisChildNode = DOM_TREE_NODE_CHILD_ITERATOR_FIRST(&childIter);
        for (; pThisChildNode != gcvNULL; pThisChildNode = DOM_TREE_NODE_CHILD_ITERATOR_NEXT(&childIter))
        {
            childCDIdx = vscBV_FindSetBitForward(&pThisChildNode->pOwnerBB->cdSet, 0);
            while (childCDIdx != (gctUINT)INVALID_BIT_LOC)
            {
                pChildBasicBlk = CFG_GET_BB_BY_ID(pCFG, childCDIdx);

                if (!BB_IS_IPDOM(pThisBasicBlk, pChildBasicBlk))
                {
                    vscBV_SetBit(&pThisBasicBlk->cdSet, pChildBasicBlk->dgNode.id);
                }

                childCDIdx = vscBV_FindSetBitForward(&pThisChildNode->pOwnerBB->cdSet, childCDIdx + 1);
            }
        }
    }

    vscMM_Free(pMM, ppPostDomTreePO);
    return VSC_ERR_NONE;
}

VSC_ErrCode vscVIR_DestroyControlDepPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pThisBlock;
    VSC_ErrCode             errCode  = VSC_ERR_NONE;

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
         vscBV_Finalize(&pThisBlock->cdSet);
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildDomFrontierPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    VSC_ErrCode                  errCode  = VSC_ERR_NONE;
    VIR_DOM_TREE*                pDomTree = &pCFG->domTree;
    VIR_DOM_TREE_NODE**          ppDomTreePO;
    gctUINT                      countOfDomTreeNode = vscTREE_GetNodeCount(&pDomTree->tree);
    gctUINT                      hisCountOfBasicBlk = vscDG_GetHistNodeCount(&pCFG->dgGraph);
    VSC_MM*                      pMM = &pCFG->pmp.mmWrapper;
    gctUINT                      tnIdx;
    VIR_BASIC_BLOCK*             pThisBasicBlk;
    VIR_BASIC_BLOCK*             pSuccBasicBlk;
    VIR_BASIC_BLOCK*             pChildBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   succEdgeIter;
    VIR_CFG_EDGE*                pSuccEdge;
    DOM_TREE_NODE_CHILD_ITERATOR childIter;
    VIR_DOM_TREE_NODE*           pThisChildNode;
    gctUINT                      childDFIdx;

    if (countOfDomTreeNode == 0)
    {
        return errCode;
    }

    /* Get PO of dom-tree */
    ppDomTreePO = (VIR_DOM_TREE_NODE**)vscMM_Alloc(pMM, sizeof(VIR_DOM_TREE_NODE*)*countOfDomTreeNode);
    if (!ppDomTreePO)
    {
        errCode = VSC_ERR_OUT_OF_MEMORY;
        CHECK_ERROR(errCode, "Build dom frontier");
    }
    vscTREE_PstOrderTraversal(&pDomTree->tree, (VSC_TREE_NODE**)ppDomTreePO);

    /* Let's bottom-up to get df of each bb */
    for (tnIdx = 0; tnIdx < countOfDomTreeNode; tnIdx ++)
    {
        pThisBasicBlk = ppDomTreePO[tnIdx]->pOwnerBB;

        /* An init of VB has an implicit full clear */
        errCode = vscBV_Initialize(&pThisBasicBlk->dfSet, pMM, hisCountOfBasicBlk);
        CHECK_ERROR(errCode, "Build dom frontier");

        /* Local df based cfg */
        VSC_ADJACENT_LIST_ITERATOR_INIT(&succEdgeIter, &pThisBasicBlk->dgNode.succList);
        pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&succEdgeIter);
        for (; pSuccEdge != gcvNULL; pSuccEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&succEdgeIter))
        {
            pSuccBasicBlk = CFG_EDGE_GET_TO_BB(pSuccEdge);

            if (!BB_IS_IDOM(pThisBasicBlk, pSuccBasicBlk))
            {
                vscBV_SetBit(&pThisBasicBlk->dfSet, pSuccBasicBlk->dgNode.id);
            }
        }

        /* Up df based on dom tree */
        DOM_TREE_NODE_CHILD_ITERATOR_INIT(&childIter, pThisBasicBlk->pDomTreeNode);
        pThisChildNode = DOM_TREE_NODE_CHILD_ITERATOR_FIRST(&childIter);
        for (; pThisChildNode != gcvNULL; pThisChildNode = DOM_TREE_NODE_CHILD_ITERATOR_NEXT(&childIter))
        {
            childDFIdx = vscBV_FindSetBitForward(&pThisChildNode->pOwnerBB->dfSet, 0);
            while (childDFIdx != (gctUINT)INVALID_BIT_LOC)
            {
                pChildBasicBlk = CFG_GET_BB_BY_ID(pCFG, childDFIdx);

                if (!BB_IS_IDOM(pThisBasicBlk, pChildBasicBlk))
                {
                    vscBV_SetBit(&pThisBasicBlk->dfSet, pChildBasicBlk->dgNode.id);
                }

                childDFIdx = vscBV_FindSetBitForward(&pThisChildNode->pOwnerBB->dfSet, childDFIdx + 1);
            }
        }
    }

    vscMM_Free(pMM, ppDomTreePO);
    return VSC_ERR_NONE;
}

VSC_ErrCode vscVIR_DestroyDomFrontierPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG)
{
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pThisBlock;
    VSC_ErrCode             errCode  = VSC_ERR_NONE;

    CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
    pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
    for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
    {
         vscBV_Finalize(&pThisBlock->dfSet);
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildControlDep(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            return VSC_ERR_CG_NOT_BUILT;
        }

        errCode = vscVIR_BuildControlDepPerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestroyControlDep(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            continue;
        }

        errCode = vscVIR_DestroyControlDepPerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_BuildDomFrontier(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            return VSC_ERR_CG_NOT_BUILT;
        }

        errCode = vscVIR_BuildDomFrontierPerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_DestroyDomFrontier(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_Function*          pFunc;
    VIR_FunctionNode*      pFuncNode;
    VIR_FuncIterator       funcIter;

    VIR_FuncIterator_Init(&funcIter, &pShader->functions);
    pFuncNode = VIR_FuncIterator_First(&funcIter);
    for (; pFuncNode != gcvNULL; pFuncNode = VIR_FuncIterator_Next(&funcIter))
    {
        pFunc = pFuncNode->function;
        if (pFunc->pFuncBlock == gcvNULL)
        {
            continue;
        }

        errCode = vscVIR_DestroyDomFrontierPerCFG(&pFunc->pFuncBlock->cfg);
        if (errCode != VSC_ERR_NONE)
        {
            return errCode;
        }
    }

    return errCode;
}

/*
 * BB reach-relation functions
 */

static VSC_ErrCode _BbReach_Local_GenKill_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    vscBV_SetBit(&pTsBlockFlow->genFlow, pTsBlockFlow->pOwnerBB->globalBbId);
    return VSC_ERR_NONE;
}

static void _BbReach_Init_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    /* Nothing needs to do */
}

static gctBOOL _BbReach_Iterate_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VSC_BIT_VECTOR*        pInFlow = &pTsBlockFlow->inFlow;
    VSC_BIT_VECTOR*        pOutFlow = &pTsBlockFlow->outFlow;
    VSC_BIT_VECTOR*        pGenFlow = &pTsBlockFlow->genFlow;
    VSC_BIT_VECTOR*        pKillFlow = &pTsBlockFlow->killFlow;
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    vscBV_Initialize(&tmpFlow, (pBaseTsDFA->baseDFA).pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* Out = Gen U (In - Kill) */
    vscBV_Minus2(&tmpFlow, pInFlow, pKillFlow);
    vscBV_Or1(&tmpFlow, pGenFlow);

    bChanged = !vscBV_Equal(&tmpFlow, pOutFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _BbReach_Combine_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pTsBlockFlow)
{
    VIR_BASIC_BLOCK*             pPredBasicBlk;
    VSC_ADJACENT_LIST_ITERATOR   predEdgeIter;
    VIR_CFG_EDGE*                pPredEdge;
    VIR_BASIC_BLOCK*             pBasicBlock = pTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*              pInFlow = &pTsBlockFlow->inFlow;
    VSC_BIT_VECTOR               tmpFlow;
    gctBOOL                      bChanged = gcvFALSE;

    /* If there is no predecessors, then just reture FALSE */
    if (DGND_GET_IN_DEGREE(&pBasicBlock->dgNode) == 0)
    {
        return gcvFALSE;
    }

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* In = U all-pred-Outs */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&predEdgeIter, &pBasicBlock->dgNode.predList);
    pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_FIRST(&predEdgeIter);
    for (; pPredEdge != gcvNULL; pPredEdge = (VIR_CFG_EDGE *)VSC_ADJACENT_LIST_ITERATOR_NEXT(&predEdgeIter))
    {
        pPredBasicBlk = CFG_EDGE_GET_TO_BB(pPredEdge);
        vscBV_Or1(&tmpFlow, &pPredBasicBlk->pTsWorkDataFlow->outFlow);
    }

    bChanged = !vscBV_Equal(&tmpFlow, pInFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _BbReach_Block_Flow_Combine_From_Callee_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_BLOCK_FLOW* pCallerTsBlockFlow)
{
    VIR_BASIC_BLOCK*       pBasicBlock = pCallerTsBlockFlow->pOwnerBB;
    VSC_BIT_VECTOR*        pOutFlow = &pCallerTsBlockFlow->outFlow;
    VSC_BIT_VECTOR*        pInFlow = &pCallerTsBlockFlow->inFlow;
    VIR_FUNC_BLOCK*        pCallee = VIR_Inst_GetCallee(pBasicBlock->pStartInst)->pFuncBlock;
    VIR_TS_FUNC_FLOW*      pCalleeFuncFlow = (VIR_TS_FUNC_FLOW*)vscSRARR_GetElement(&pBaseTsDFA->tsFuncFlowArray, pCallee->dgNode.id);
    VSC_BIT_VECTOR         tmpFlow;
    gctBOOL                bChanged = gcvFALSE;

    gcmASSERT(pBasicBlock->flowType == VIR_FLOW_TYPE_CALL);

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* Out = In U (out flow of callee) */
    vscBV_Or2(&tmpFlow, pInFlow, &pCalleeFuncFlow->outFlow);

    bChanged = !vscBV_Equal(pOutFlow, &tmpFlow);
    if (bChanged)
    {
        vscBV_Copy(pOutFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

static gctBOOL _BbReach_Func_Flow_Combine_From_Callers_Resolver(VIR_BASE_TS_DFA* pBaseTsDFA, VIR_TS_FUNC_FLOW* pCalleeTsFuncFlow)
{
    gctUINT                      callerIdx;
    VIR_BASIC_BLOCK*             pCallerBasicBlk;
    VIR_Instruction*             pCallSiteInst;
    VIR_FUNC_BLOCK*              pCalleeFuncBlock = pCalleeTsFuncFlow->pOwnerFB;
    VSC_BIT_VECTOR*              pInFlow = &pCalleeTsFuncFlow->inFlow;
    VSC_ADJACENT_LIST_ITERATOR   callerIter;
    VIR_CG_EDGE*                 pCallerEdge;
    VSC_BIT_VECTOR               tmpFlow;
    gctBOOL                      bChanged = gcvFALSE;

    vscBV_Initialize(&tmpFlow, pBaseTsDFA->baseDFA.pScratchMemPool, pBaseTsDFA->baseDFA.flowSize);

    /* U all in flow of caller at every call site */
    VSC_ADJACENT_LIST_ITERATOR_INIT(&callerIter, &pCalleeFuncBlock->dgNode.predList);
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

            vscBV_Or1(&tmpFlow, &pCallerBasicBlk->pTsWorkDataFlow->inFlow);
        }
    }

    bChanged = !vscBV_Equal(&tmpFlow, pInFlow);
    if (bChanged)
    {
        vscBV_Copy(pInFlow, &tmpFlow);
    }

    vscBV_Finalize(&tmpFlow);

    return bChanged;
}

VSC_ErrCode vscVIR_BuildBbReachRelation(VIR_Shader* pShader)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_CALL_GRAPH*         pCg = gcvNULL;
    gctUINT                 globalBbCount, globalFromBbIdx;
    CG_ITERATOR             funcBlkIter;
    VIR_FUNC_BLOCK*         pFuncBlk;
    CFG_ITERATOR            basicBlkIter;
    VIR_BASIC_BLOCK*        pThisBlock;
    VIR_BASIC_BLOCK*        pFromBlock;
    VIR_BASE_TS_DFA         baseTsDFA;
    VIR_CONTROL_FLOW_GRAPH* pCFG;
    VSC_BIT_VECTOR*         pInFlow;
    VIR_TS_DFA_RESOLVERS    tsDfaResolvers = {
                                              _BbReach_Local_GenKill_Resolver,
                                              _BbReach_Init_Resolver,
                                              _BbReach_Iterate_Resolver,
                                              _BbReach_Combine_Resolver,
                                              _BbReach_Block_Flow_Combine_From_Callee_Resolver,
                                              _BbReach_Func_Flow_Combine_From_Callers_Resolver
                                             };

    if (pShader->mainFunction->pFuncBlock == gcvNULL)
    {
        return VSC_ERR_CG_NOT_BUILT;
    }

    /* We try to borrow iterative DFA solution to build bb reach relation. For this special
       flow, each object of flow is basic-block (identified by globalBbId which is also a
       flow index in flow). */

    pCg = pShader->mainFunction->pFuncBlock->pOwnerCG;
    globalBbCount = CG_GET_HIST_GLOBAL_BB_COUNT(pCg);

    /* 1. We firstly get raw global BB reach relation by using DFA solution */

    vscVIR_InitializeBaseTsDFA(&baseTsDFA,
                               pCg,
                               VIR_DFA_TYPE_HELPER,
                               globalBbCount,
                               &pCg->pmp.mmWrapper,
                               &tsDfaResolvers);

    errCode = vscVIR_DoForwardIterativeTsDFA(pCg, &baseTsDFA, gcvTRUE);
    ON_ERROR(errCode, "Build raw bb reach relation");

    /* 2. We now can generate our own global and local BB reach relation based on raw info.
          Note based on the fact because A can reach to B forwardly, then B must reach to A
          backwardly, we will directly generate bwd reach relation with raw info got by fwd
          iterative */
    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        pCFG = &pFuncBlk->cfg;
        CFG_ITERATOR_INIT(&basicBlkIter, pCFG);
        pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
        for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
        {
            if (!BV_IS_VALID(&pThisBlock->globalReachSet.bwdReachInBBSet))
            {
                _InitializeBbReachRelation(&pThisBlock->globalReachSet,
                                           &pCFG->pmp.mmWrapper,
                                           globalBbCount);
                _InitializeBbReachRelation(&pThisBlock->localReachSet,
                                           &pCFG->pmp.mmWrapper,
                                           vscDG_GetHistNodeCount(&pCFG->dgGraph));
            }

            /* Note we only use inFlow of BB to get reach-relation because outFlow make all BB
               self-reached which is wrong. Only body of loop or other similiar back-edge cfg
               have bb self-reached. */
            pInFlow = &pThisBlock->pTsWorkDataFlow->inFlow;

            globalFromBbIdx = 0;
            while ((globalFromBbIdx = vscBV_FindSetBitForward(pInFlow, globalFromBbIdx)) != (gctUINT)INVALID_BIT_LOC)
            {
                pFromBlock = CG_GET_BB_BY_GLOBAL_ID(pCg, globalFromBbIdx);
                gcmASSERT(pFromBlock->globalBbId == globalFromBbIdx);

                if (!BV_IS_VALID(&pFromBlock->globalReachSet.bwdReachInBBSet))
                {
                    _InitializeBbReachRelation(&pFromBlock->globalReachSet,
                                               &pFromBlock->pOwnerCFG->pmp.mmWrapper,
                                               globalBbCount);
                    _InitializeBbReachRelation(&pFromBlock->localReachSet,
                                               &pFromBlock->pOwnerCFG->pmp.mmWrapper,
                                               vscDG_GetHistNodeCount(&pFromBlock->pOwnerCFG->dgGraph));
                }

                /* Fwd */
                vscBV_SetBit(&pThisBlock->globalReachSet.fwdReachInBBSet, globalFromBbIdx);
                vscBV_SetBit(&pFromBlock->globalReachSet.fwdReachOutBBSet, pThisBlock->globalBbId);
                if (pFromBlock->pOwnerCFG == pThisBlock->pOwnerCFG)
                {
                    vscBV_SetBit(&pThisBlock->localReachSet.fwdReachInBBSet, pFromBlock->dgNode.id);
                    vscBV_SetBit(&pFromBlock->localReachSet.fwdReachOutBBSet, pThisBlock->dgNode.id);
                }

                /* Bwd */
                vscBV_SetBit(&pFromBlock->globalReachSet.bwdReachInBBSet, pThisBlock->globalBbId);
                vscBV_SetBit(&pThisBlock->globalReachSet.bwdReachOutBBSet, globalFromBbIdx);
                if (pFromBlock->pOwnerCFG == pThisBlock->pOwnerCFG)
                {
                    vscBV_SetBit(&pFromBlock->localReachSet.bwdReachInBBSet, pThisBlock->dgNode.id);
                    vscBV_SetBit(&pThisBlock->localReachSet.bwdReachOutBBSet, pFromBlock->dgNode.id);
                }

                globalFromBbIdx ++;
            }
        }
    }

OnError:
    vscVIR_FinalizeBaseTsDFA(&baseTsDFA);
    return errCode;
}

VSC_ErrCode vscVIR_DestroyBbReachRelation(VIR_Shader* pShader)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;
    VIR_CALL_GRAPH*        pCg = gcvNULL;
    CG_ITERATOR            funcBlkIter;
    VIR_FUNC_BLOCK*        pFuncBlk;
    CFG_ITERATOR           basicBlkIter;
    VIR_BASIC_BLOCK*       pThisBlock;

    if (pShader->mainFunction->pFuncBlock == gcvNULL)
    {
        return errCode;
    }

    pCg = pShader->mainFunction->pFuncBlock->pOwnerCG;

    CG_ITERATOR_INIT(&funcBlkIter, pCg);
    pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_FIRST(&funcBlkIter);
    for (; pFuncBlk != gcvNULL; pFuncBlk = (VIR_FUNC_BLOCK *)CG_ITERATOR_NEXT(&funcBlkIter))
    {
        CFG_ITERATOR_INIT(&basicBlkIter, &pFuncBlk->cfg);
        pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_FIRST(&basicBlkIter);
        for (; pThisBlock != gcvNULL; pThisBlock = (VIR_BASIC_BLOCK *)CFG_ITERATOR_NEXT(&basicBlkIter))
        {
            _FinalizeBbReachRelation(&pThisBlock->globalReachSet);
            _FinalizeBbReachRelation(&pThisBlock->localReachSet);
        }
    }

    return errCode;
}

VSC_ErrCode vscVIR_CleanDfsVisitOrderIdxOnFunc(VIR_Function* pFunc)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_CONTROL_FLOW_GRAPH* pCfg = VIR_Function_GetCFG(pFunc);
    VIR_BASIC_BLOCK*        pBB = gcvNULL;
    CFG_ITERATOR            cfgIter;

    CFG_ITERATOR_INIT(&cfgIter, pCfg);
    for (pBB = CFG_ITERATOR_FIRST(&cfgIter);
         pBB != gcvNULL;
         pBB = CFG_ITERATOR_NEXT(&cfgIter))
    {
        pBB->dfsPostVisitOrderIdx = NOT_ASSIGNED;
        pBB->dfsPreVisitOrderIdx = NOT_ASSIGNED;
    }

    return errCode;
}

VSC_ErrCode vscVIR_CleanDfsVisitOrderIdxOnShader(VIR_Shader* pShader)
{
    VSC_ErrCode             errCode  = VSC_ERR_NONE;
    VIR_FuncIterator        func_iter;
    VIR_FunctionNode*       pFuncNode  = gcvNULL;
    VIR_Function*           pFunc = gcvNULL;

    if (!vscVIR_IsCFGBuilt(pShader))
    {
        return errCode;
    }

    VIR_FuncIterator_Init(&func_iter, VIR_Shader_GetFunctions(pShader));
    for(pFuncNode = VIR_FuncIterator_First(&func_iter);
        pFuncNode != gcvNULL;
        pFuncNode = VIR_FuncIterator_Next(&func_iter))
    {
        pFunc = pFuncNode->function;

        errCode = vscVIR_CleanDfsVisitOrderIdxOnFunc(pFunc);
        ON_ERROR(errCode, "Clean DFS visit order index on function.");
    }

OnError:
    return errCode;
}

#if LOOP_HIERARCHY_SUPPORT_IN_CFG
/*
 * Loop hierarchy related functions
 */

VSC_ErrCode vscVIR_BuildLoopHierarchyPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_LOOP_HIERARCHY* pLoopHierarchy)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;

    return errCode;
}

VSC_ErrCode vscVIR_DestroyLoopHierarchyPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_LOOP_HIERARCHY* pLoopHierarchy)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;

    return errCode;
}

VSC_ErrCode vscVIR_BuildLoopHierarchy(VIR_Shader* pShader, VIR_LOOP_HIERARCHY* pLoopHierarchy)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;

    return errCode;
}

VSC_ErrCode vscVIR_DestroyLoopHierarchy(VIR_Shader* pShader, VIR_LOOP_HIERARCHY* pLoopHierarchy)
{
    VSC_ErrCode            errCode  = VSC_ERR_NONE;

    return errCode;
}
#endif


