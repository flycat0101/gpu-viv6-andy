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


#ifndef __gc_vsc_vir_cfa_h_
#define __gc_vsc_vir_cfa_h_

/* Here we define all control flow related stuff based on VIR, such as CG/CFG/DOM/CDG/DFG, etc.
   All these are built on graph or tree. */

BEGIN_EXTERN_C()

typedef struct _VIR_CONTROL_FLOW_GRAPH  VIR_CONTROL_FLOW_GRAPH;
typedef struct _VIR_CALL_GRAPH          VIR_CALL_GRAPH;
typedef struct _VIR_BASIC_BLOCK         VIR_BASIC_BLOCK;
typedef struct _VIR_FUNC_BLOCK          VIR_FUNC_BLOCK;
typedef struct _VIR_LOOP_TREE_NODE      VIR_LOOP_TREE_NODE;

#include "gc_vsc_vir_dfa.h"

/*
 *   (post-) DOM tree definition
 */
typedef struct _VIR_DOM_TREE_NODE
{
    /* Tree node. It must be put at FIRST place!!!! */
    VSC_TREE_NODE             treeNode;

    /* Owner of tree node */
    VIR_BASIC_BLOCK*          pOwnerBB;
}VIR_DOM_TREE_NODE;

typedef struct _VIR_DOM_TREE
{
    /* Tree-node for dom tree which is made of VIR_DOM_TREE_NODE. It must be put at FIRST place!!!! */
    VSC_TREE                  tree;

    /* Owner control flow graph of DOM tree */
    VIR_CONTROL_FLOW_GRAPH*   pOwnerCFG;
}VIR_DOM_TREE;

typedef VSC_TNODE_LIST_ITERATOR DOM_TREE_ITERATOR;
#define DOM_TREE_ITERATOR_ITERATOR_INIT(iter, pDomTree)       VSC_TNODE_LIST_ITERATOR_INIT((iter), &pDomTree->tree)
#define DOM_TREE_ITERATOR_ITERATOR_FIRST(iter)                (VIR_DOM_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_FIRST((iter))
#define DOM_TREE_ITERATOR_ITERATOR_NEXT(iter)                 (VIR_DOM_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_NEXT((iter))
#define DOM_TREE_ITERATOR_ITERATOR_PREV(iter)                 (VIR_DOM_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_PREV((iter))
#define DOM_TREE_ITERATOR_ITERATOR_LAST(iter)                 (VIR_DOM_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_LAST((iter))

typedef VSC_CHILD_LIST_ITERATOR DOM_TREE_NODE_CHILD_ITERATOR;
#define DOM_TREE_NODE_CHILD_ITERATOR_INIT(iter, pDomTreeNode) VSC_CHILD_LIST_ITERATOR_INIT((iter), &(pDomTreeNode)->treeNode)
#define DOM_TREE_NODE_CHILD_ITERATOR_FIRST(iter)              (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_FIRST((iter))
#define DOM_TREE_NODE_CHILD_ITERATOR_NEXT(iter)               (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_NEXT((iter))
#define DOM_TREE_NODE_CHILD_ITERATOR_LAST(iter)               (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_LAST((iter))

#define LOOP_HIERARCHY_SUPPORT_IN_CFG  0

#if LOOP_HIERARCHY_SUPPORT_IN_CFG
/*
 *   Loop hierarchy definition
 */
typedef struct _VIR_LOOP_HIERARCHY_INFO
{
    /* Loop head and tail */
    VIR_BASIC_BLOCK*           pLoopHead;
    VIR_BASIC_BLOCK*           pLoopTail;

    /* Break and continue blocks inside this loop */
    VSC_SIMPLE_RESIZABLE_ARRAY breakSet;
    VSC_SIMPLE_RESIZABLE_ARRAY continueSet;

    /* Which loop tree node holds this loop hierarchy info */
    VIR_LOOP_TREE_NODE*        pLoopTreeNode;
}VIR_LOOP_HIERARCHY_INFO;

struct _VIR_LOOP_TREE_NODE
{
    /* Tree node. It must be put at FIRST place!!!! */
    VSC_TREE_NODE              treeNode;

    /* Hierarchy info stored in loop tree node */
    VIR_LOOP_HIERARCHY_INFO    loopHierarchyInfo;
};

typedef struct _VIR_LOOP_TREE
{
    /* Tree-node for loop tree which is made of VIR_LOOP_TREE_NODE. It must be put at FIRST place!!!! */
    VSC_TREE                   tree;
}VIR_LOOP_TREE;

typedef struct _VIR_LOOP_HIERARCHY
{
    /* All loop-trees maintained in loop-hierarchy of cfg */
    VSC_SIMPLE_RESIZABLE_ARRAY loopTreeArray;

    /* Owner control flow graph of loop hierarchy */
    VIR_CONTROL_FLOW_GRAPH*    pOwnerCFG;
}VIR_LOOP_HIERARCHY;

typedef VSC_TNODE_LIST_ITERATOR LOOP_TREE_ITERATOR;
#define LOOP_TREE_ITERATOR_ITERATOR_INIT(iter, pLoopTree)      VSC_TNODE_LIST_ITERATOR_INIT((iter), &pLoopTree->tree)
#define LOOP_TREE_ITERATOR_ITERATOR_FIRST(iter)                (VIR_LOOP_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_FIRST((iter))
#define LOOP_TREE_ITERATOR_ITERATOR_NEXT(iter)                 (VIR_LOOP_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_NEXT((iter))
#define LOOP_TREE_ITERATOR_ITERATOR_PREV(iter)                 (VIR_LOOP_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_PREV((iter))
#define LOOP_TREE_ITERATOR_ITERATOR_LAST(iter)                 (VIR_LOOP_TREE_NODE*)VSC_TNODE_LIST_ITERATOR_LAST((iter))

typedef VSC_CHILD_LIST_ITERATOR LOOP_TREE_NODE_CHILD_ITERATOR;
#define LOOP_TREE_NODE_CHILD_ITERATOR_INIT(iter, pLoopTreeNode) VSC_CHILD_LIST_ITERATOR_INIT((iter), &(pLoopTreeNode)->treeNode)
#define LOOP_TREE_NODE_CHILD_ITERATOR_FIRST(iter)              (VIR_LOOP_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_FIRST((iter))
#define LOOP_TREE_NODE_CHILD_ITERATOR_NEXT(iter)               (VIR_LOOP_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_NEXT((iter))
#define LOOP_TREE_NODE_CHILD_ITERATOR_LAST(iter)               (VIR_LOOP_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_LAST((iter))
#endif

/*
 *   Control flow graph definition
 */

typedef enum _VIR_FLOW_TYPE
{
    VIR_FLOW_TYPE_NONE       = 0, /* Means there is no control flow inst at end of bb */
    VIR_FLOW_TYPE_ENTRY,
    VIR_FLOW_TYPE_EXIT,
    VIR_FLOW_TYPE_JMP,
    VIR_FLOW_TYPE_JMPC,
    VIR_FLOW_TYPE_CALL,
    VIR_FLOW_TYPE_RET,
} VIR_FLOW_TYPE;

typedef enum _VIR_CFG_EDGE_TYPE
{
    VIR_CFG_EDGE_TYPE_ALWAYS    = 0,
    VIR_CFG_EDGE_TYPE_TRUE,
    VIR_CFG_EDGE_TYPE_FALSE,
    VIR_CFG_EDGE_TYPE_COUNT,
}VIR_CFG_EDGE_TYPE;

typedef enum _VIR_CFG_DFS_EDGE_TYPE
{
    VIR_CFG_DFS_EDGE_TYPE_NORMAL     = 0,
    VIR_CFG_DFS_EDGE_TYPE_FORWARD,
    VIR_CFG_DFS_EDGE_TYPE_BACKWARD,
    VIR_CFG_DFS_EDGE_TYPE_CROSS,
}VIR_CFG_DFS_EDGE_TYPE;

typedef struct _VIR_CFG_EDGE
{
    VSC_DG_EDGE               dgEdge;
    VIR_CFG_EDGE_TYPE         type;

    /* Only valid for building DFS spanning tree */
    VIR_CFG_DFS_EDGE_TYPE     dfsType;
}VIR_CFG_EDGE;

typedef struct _VIR_BB_REACH_RELATION
{
    /* Forwardly, which BB can reach to this BB (reachInBB) and which BB can this BB reach to (reachOutBB) */
    VSC_BIT_VECTOR            fwdReachInBBSet;
    VSC_BIT_VECTOR            fwdReachOutBBSet;

    /* Backwardly, which BB can reach to this BB (reachInBB) and which BB can this BB reach to (reachOutBB) */
    VSC_BIT_VECTOR            bwdReachInBBSet;
    VSC_BIT_VECTOR            bwdReachOutBBSet;
}VIR_BB_REACH_RELATION;

#define CFG_EDGE_GET_FROM_BB(pCfgEdge)  ((VIR_BASIC_BLOCK*)((pCfgEdge)->dgEdge.pFromNode))
#define CFG_EDGE_GET_TO_BB(pCfgEdge)    ((VIR_BASIC_BLOCK*)((pCfgEdge)->dgEdge.pToNode))
#define CFG_EDGE_GET_TYPE(pCfgEdge)     ((pCfgEdge)->type)
#define CFG_EDGE_SET_TYPE(pCfgEdge, t)  ((pCfgEdge)->type = (t))

#define VIR_BB_FLAG_HavingLLI          0x1

typedef enum _VIR_BB_FLAG
{
    VIR_BBFLAG_NONE                     = 0x0000,
    VIR_BBFLAG_HAS_LLI                  = 0x0001,
    VIR_BBFLAG_HAS_BARRIER              = 0x0002,
} VIR_BB_FLAG;

struct _VIR_BASIC_BLOCK
{
    /* Graph node. It must be put at FIRST place!!!! */
    VSC_DG_NODE               dgNode;

    /* CG-wide global BB id */
    gctUINT                   globalBbId;

    /* Owner control flow graph of BB */
    VIR_CONTROL_FLOW_GRAPH*   pOwnerCFG;

    /* Instruction list in the block, note that the real instructions are maintained by VIR_Function */
    VIR_Instruction*          pStartInst;
    VIR_Instruction*          pEndInst;

    /* How many insts do belong to this BB */
    gctUINT32                 instCount;

    /* What kind of type for this basic block */
    VIR_FLOW_TYPE             flowType;

    /* This is only used when doing iterative analysis with work list algorithm for CFA and DFA */
    gctBOOL                   bInWorklist;

    /* When doing iterative data flow analysis for a certain data flow type, this type of block flow
       will be selected to be as working data flow which iterative analyser will be used. Note that
       we have two kinds of data flow, TS (two-states) and MS (multi-states). */
    VIR_TS_BLOCK_FLOW*        pTsWorkDataFlow;
    VIR_MS_BLOCK_FLOW*        pMsWorkDataFlow;

    /* For building DFS spanning tree */
    gctUINT                   dfsPreVisitOrderIdx;
    gctUINT                   dfsPostVisitOrderIdx;

    /* DOM and postDOM set indicating who (post-) dominate the BB */
    VSC_BIT_VECTOR            domSet;
    VSC_BIT_VECTOR            postDomSet;

    /* Node in DOM tree and postDOM tree */
    VIR_DOM_TREE_NODE*        pDomTreeNode;
    VIR_DOM_TREE_NODE*        pPostDomTreeNode;

    /* Who are this BB control (reversed-) dependent on?? */
    VSC_BIT_VECTOR            dfSet;
    VSC_BIT_VECTOR            cdSet;

    /* Local (function-wide, search with dgNode.id) and global (CG-wide, search with globalBbId) basic
       block reach relation info */
    VIR_BB_REACH_RELATION     localReachSet;
    VIR_BB_REACH_RELATION     globalReachSet;

    /* BB flags */
    VIR_BB_FLAG               flags;
};

#define BB_GET_ID(pBB)          ((pBB)->dgNode.id)
#define BB_GET_CFG(pBB)         ((pBB)->pOwnerCFG)
#define BB_GET_FUNC(pBB)        ((pBB)->pOwnerCFG->pOwnerFuncBlk->pVIRFunc)
#define BB_GET_START_INST(pBB)  ((pBB)->pStartInst)
#define BB_GET_END_INST(pBB)    ((pBB)->pEndInst)
#define BB_GET_LENGTH(pBB)      ((pBB)->instCount)
#define BB_GET_FLOWTYPE(pBB)    ((pBB)->flowType)
#define BB_SET_FLOWTYPE(pBB, t) ((pBB)->flowType = (t))
#define BB_GET_IN_DEGREE(pBB)   DGND_GET_IN_DEGREE(&(pBB)->dgNode)
#define BB_GET_OUT_DEGREE(pBB)  DGND_GET_OUT_DEGREE(&(pBB)->dgNode)

#define BB_INC_LENGTH(pBB)      ((pBB)->instCount ++)
#define BB_DEC_LENGTH(pBB)      ((pBB)->instCount --)

#define BB_SET_START_INST(pBB, pInst)  ((pBB)->pStartInst = (pInst))
#define BB_SET_END_INST(pBB, pInst)    ((pBB)->pEndInst = (pInst))

#define BB_FLAGS_GET(pBB)               ((pBB)->flags)
#define BB_FLAGS_SET(pBB, V)            ((pBB)->flags = (V))
#define BB_FLAGS_GET_LLI(pBB)           ((pBB)->flags & VIR_BBFLAG_HAS_LLI)
#define BB_FLAGS_SET_LLI(pBB)           ((pBB)->flags |= VIR_BBFLAG_HAS_LLI)
#define BB_FLAGS_RESET_LLI(pBB)         ((pBB)->flags &= ~((gctUINT32)VIR_BBFLAG_HAS_LLI))
#define BB_FLAGS_HAS_BARRIER(pBB)       ((pBB)->flags & VIR_BBFLAG_HAS_BARRIER)
#define BB_FLAGS_SET_HAS_BARRIER(pBB)   ((pBB)->flags |= VIR_BBFLAG_HAS_BARRIER)
#define BB_FLAGS_RESET_HAS_BARRIER(pBB) ((pBB)->flags &= ~((gctUINT32)VIR_BBFLAG_HAS_BARRIER))

#define BB_GET_IDOM(pBB)        (((VIR_DOM_TREE_NODE *)((pBB)->pDomTreeNode->treeNode.pParentNode))->pOwnerBB)
#define BB_GET_IPDOM(pBB)       (((VIR_DOM_TREE_NODE *)((pBB)->pPostDomTreeNode->treeNode.pParentNode))->pOwnerBB)
#define BB_IS_DOM(pBBDominator, pBBDominatee)   \
        vscBV_TestBit(&(pBBDominatee)->domSet, (pBBDominator)->dgNode.id)
#define BB_IS_IDOM(pBBDominator, pBBDominatee)  \
        (BB_GET_IDOM((pBBDominatee)) == (pBBDominator))
#define BB_IS_SDOM(pBBDominator, pBBDominatee)  \
        (BB_IS_DOM((pBBDominator), (pBBDominatee)) && ((pBBDominator) != (pBBDominatee)))
#define BB_IS_PDOM(pBBDominator, pBBDominatee)  \
        vscBV_TestBit(&(pBBDominatee)->postDomSet, (pBBDominator)->dgNode.id)
#define BB_IS_IPDOM(pBBDominator, pBBDominatee) \
        (BB_GET_IPDOM((pBBDominatee)) == (pBBDominator))
#define BB_IS_SPDOM(pBBDominator, pBBDominatee) \
        (BB_IS_PDOM((pBBDominator), (pBBDominatee)) && ((pBBDominator) != (pBBDominatee)))

typedef VSC_ADJACENT_NODE_ITERATOR PRED_BASIC_BLOCK_ITERATOR;
#define PRED_BASIC_BLOCK_ITERATOR_ITERATOR_INIT(iter, pBasicBlk) vscDGNDAJNIterator_Init((iter), &(pBasicBlk)->dgNode, gcvTRUE)
#define PRED_BASIC_BLOCK_ITERATOR_ITERATOR_FIRST(iter)           (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_First((iter))
#define PRED_BASIC_BLOCK_ITERATOR_ITERATOR_NEXT(iter)            (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_Next((iter))
#define PRED_BASIC_BLOCK_ITERATOR_ITERATOR_LAST(iter)            (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_Last((iter))

typedef VSC_ADJACENT_NODE_ITERATOR SUCC_BASIC_BLOCK_ITERATOR;
#define SUCC_BASIC_BLOCK_ITERATOR_ITERATOR_INIT(iter, pBasicBlk) vscDGNDAJNIterator_Init((iter), &(pBasicBlk)->dgNode, gcvFALSE)
#define SUCC_BASIC_BLOCK_ITERATOR_ITERATOR_FIRST(iter)           (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_First((iter))
#define SUCC_BASIC_BLOCK_ITERATOR_ITERATOR_NEXT(iter)            (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_Next((iter))
#define SUCC_BASIC_BLOCK_ITERATOR_ITERATOR_LAST(iter)            (VIR_BASIC_BLOCK*)vscDGNDAJNIterator_Last((iter))

/* Basic block worklist and its item for some algorithm, not only for CFA, but for DFA */
typedef VSC_SIMPLE_QUEUE  VIR_BB_WORKLIST;
typedef VSC_QUEUE_STACK_ENTRY  VIR_BB_WORKITEM;

#define BB_WORKITEM_INIT(pWorkItem, pBB)               SQE_INITIALIZE((pWorkItem), (pBB))
#define BB_WORKITEM_FINALIZE(pWorkItem)                SQE_FINALIZE((pWorkItem))
#define BB_WORKITEM_GET_BB(pWorkItem)                  (VIR_BASIC_BLOCK*)SQE_GET_CONTENT((pWorkItem))

#define BB_WORKLIST_INIT(pWorkList)                    QUEUE_INITIALIZE((pWorkList))
#define BB_WORKLIST_FINALIZE(pWorkList)                QUEUE_FINALIZE((pWorkList))
#define BB_WORKLIST_ADD_WORKITEM(pWorkList, pWorkItem) QUEUE_PUT_ENTRY((pWorkList), (pWorkItem))
#define BB_WORKLIST_GET_WORKITEM(pWorkList)            QUEUE_GET_ENTRY((pWorkList))
#define BB_WORKLIST_IS_EMPTY(pWorkList)                QUEUE_CHECK_EMPTY((pWorkList))

void vscBBWKL_AddBBToWorkItemList(VIR_BB_WORKLIST* pWorkItemList, VIR_BB_WORKITEM* pWorkItem, VIR_BASIC_BLOCK* pBB);
VIR_BASIC_BLOCK* vscBBWKL_RemoveBBFromWorkItemList(VIR_BB_WORKLIST* pWorkItemList);

struct _VIR_CONTROL_FLOW_GRAPH
{
    /* DG for CFG which is made of VIR_BASIC_BLOCK. It must be put at FIRST place!!!! */
    VSC_DIRECTED_GRAPH        dgGraph;

    /* This CFG is belonging to which func-block */
    VIR_FUNC_BLOCK*           pOwnerFuncBlk;

    /* DOM and postDOM tree of this CFG */
    VIR_DOM_TREE              domTree;
    VIR_DOM_TREE              postDomTree;

    /* Memory pool that this CFG is built on */
    VSC_PRIMARY_MEM_POOL      pmp;
    /* scratch mem pool can reuse memory be freed */
    VSC_MM*                   pScratchMemPool;
};

typedef VSC_GNODE_LIST_ITERATOR CFG_ITERATOR;
#define CFG_ITERATOR_INIT(iter, pCFG)    VSC_GNODE_LIST_ITERATOR_INIT((iter), &(pCFG)->dgGraph)
#define CFG_ITERATOR_FIRST(iter)         (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_FIRST((iter))
#define CFG_ITERATOR_NEXT(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_NEXT((iter))
#define CFG_ITERATOR_PREV(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_PREV((iter))
#define CFG_ITERATOR_LAST(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_LAST((iter))

#define CFG_GET_ENTRY_BB(pCFG)           (*(VIR_BASIC_BLOCK**)vscSRARR_GetElement      \
                                          (&(pCFG)->dgGraph.rootNodeArray, 0))

#define CFG_GET_EXIT_BB(pCFG)            (*(VIR_BASIC_BLOCK**)vscSRARR_GetElement      \
                                          (&(pCFG)->dgGraph.tailNodeArray, 0))

#define CFG_GET_BB_BY_ID(pCFG, id)       (VIR_BASIC_BLOCK*)vscDG_GetNodeById(&(pCFG)->dgGraph, (id))
#define CFG_GET_DOM_TREE(pCFG)           (&(pCFG)->domTree)
#define CFG_GET_PDOM_TREE(pCFG)          (&(pCFG)->postDomTree)

/*
 *   Call graph definition
 */

#define VIR_INFINITE_CALL_DEPTH     0xFFFFFFFF

typedef struct _VIR_CG_EDGE
{
    VSC_DG_EDGE                dgEdge;

    /* If func A calls func B, inside of func A, it maybe has seraval sites
       calling to func B, so this array records all call sites. Note seq of
       all call sites are recorded as they appears in shader. */
    VSC_SIMPLE_RESIZABLE_ARRAY callSiteArray;
}VIR_CG_EDGE;

#define CG_PRED_EDGE_TO_SUCC_EDGE(pPredEdge)    ((VIR_CG_EDGE*)(pPredEdge) - 1)
#define CG_SUCC_EDGE_TO_PRED_EDGE(pSuccEdge)    ((VIR_CG_EDGE*)(pSuccEdge) + 1)

#define CG_EDGE_GET_FROM_FB(pCgEdge)    ((VIR_FUNC_BLOCK*)((pCgEdge)->dgEdge.pFromNode))
#define CG_EDGE_GET_TO_FB(pCgEdge)      ((VIR_FUNC_BLOCK*)((pCgEdge)->dgEdge.pToNode))

struct _VIR_FUNC_BLOCK
{
    /* Graph node. It must be put at FIRST place!!!! */
    VSC_DG_NODE                dgNode;

    /* VIR function */
    VIR_Function*              pVIRFunc;

    /* Owner call graph of FB */
    VIR_CALL_GRAPH*            pOwnerCG;

    /* BB based control flow graph that this function block maintains */
    VIR_CONTROL_FLOW_GRAPH     cfg;

    /* Call depth of this function block in call graph */
    gctUINT                    minCallDepth;
    gctUINT                    maxCallDepth;

    /* Although we have callsite for each callee in edge, but sometimes we need
       callsite of all callees in order of how they appears in shader. */
    VSC_SIMPLE_RESIZABLE_ARRAY mixedCallSiteArray;
};

struct _VIR_CALL_GRAPH
{
    /* DG for CG which is made of VIR_FUNC_BLOCK. It must be put at FIRST place!!!! */
    VSC_DIRECTED_GRAPH        dgGraph;

    /* This CG is belonging to which shader */
    VIR_Shader*               pOwnerShader;

    /* Global (CG-wide) BB id (VIR_BASIC_BLOCK::globalBbId) generator */
    gctUINT                   nextGlobalBbId;

    /* To hash BB by VIR_BASIC_BLOCK::globalBbId */
    VSC_HASH_TABLE            globalBbHashTable;

    /* Memory pool that this CG is built on */
    VSC_PRIMARY_MEM_POOL      pmp;
    /* scratch mem pool can reuse memory be freed */
    VSC_MM*                   pScratchMemPool;
};

typedef VSC_GNODE_LIST_ITERATOR CG_ITERATOR;
#define CG_ITERATOR_INIT(iter, pCG)    VSC_GNODE_LIST_ITERATOR_INIT((iter), &(pCG)->dgGraph)
#define CG_ITERATOR_FIRST(iter)        VSC_GNODE_LIST_ITERATOR_FIRST((iter))
#define CG_ITERATOR_NEXT(iter)         VSC_GNODE_LIST_ITERATOR_NEXT((iter))
#define CG_ITERATOR_PREV(iter)         VSC_GNODE_LIST_ITERATOR_PREV((iter))
#define CG_ITERATOR_LAST(iter)         VSC_GNODE_LIST_ITERATOR_LAST((iter))

#define CG_GET_HIST_GLOBAL_BB_COUNT(pCG) (pCG)->nextGlobalBbId
#define CG_GET_BB_BY_GLOBAL_ID(pCG, globalBbId)                             \
         (VIR_BASIC_BLOCK*)vscHTBL_DirectGet(&pCG->globalBbHashTable, (void*)(gctUINTPTR_T)globalBbId)

#define CG_GET_MAIN_FUNC(pCG) ((*(VIR_FUNC_BLOCK**)vscSRARR_GetElement      \
                               (&(pCG)->dgGraph.rootNodeArray, 0))->pVIRFunc)



/* CG related functions */
VSC_ErrCode vscVIR_BuildCallGraph(VSC_MM* pScratchMemPool, VIR_Shader* pShader, VIR_CALL_GRAPH* pCg);
VSC_ErrCode vscVIR_DestroyCallGraph(VIR_CALL_GRAPH* pCg);
gctBOOL vscVIR_IsCallGraphBuilt(VIR_CALL_GRAPH* pCg);
VSC_ErrCode vscVIR_RemoveFuncBlockFromCallGraph(VIR_CALL_GRAPH* pCg,
                                                VIR_FUNC_BLOCK* pFuncBlk,
                                                gctBOOL bRemoveFuncInShader);

/* CFG related functions */
VSC_ErrCode vscVIR_BuildCFGPerFunc(VSC_MM* pScratchMemPool, VIR_Function* pFunc);
VSC_ErrCode vscVIR_DestroyCFGPerFunc(VIR_Function* pFunc);
VSC_ErrCode vscVIR_BuildCFG(VSC_MM* pScratchMemPool, VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyCFG(VIR_Shader* pShader);
gctBOOL vscVIR_IsCFGBuilt(VIR_Shader* pShader);
VIR_BASIC_BLOCK* vscVIR_AddBasicBlockToCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                           VIR_Instruction* pStartInst,
                                           VIR_Instruction* pEndInst,
                                           VIR_FLOW_TYPE flowType);
VSC_ErrCode vscVIR_RemoveBasicBlockFromCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                           VIR_BASIC_BLOCK* pBasicBlk,
                                           gctBOOL bDeleteInst);
VSC_ErrCode vscVIR_AddEdgeToCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                VIR_BASIC_BLOCK* pFromBasicBlk,
                                VIR_BASIC_BLOCK* pToBasicBlk,
                                VIR_CFG_EDGE_TYPE edgeType);
VSC_ErrCode vscVIR_RemoveEdgeFromCFG(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                     VIR_BASIC_BLOCK* pFromBasicBlk,
                                     VIR_BASIC_BLOCK* pToBasicBlk);
VIR_CFG_EDGE* vscVIR_GetCfgEdge(VIR_CONTROL_FLOW_GRAPH* pCFG,
                                VIR_BASIC_BLOCK* pFromBasicBlk,
                                VIR_BASIC_BLOCK* pToBasicBlk);

/* Followings CFG related functions are TO-BE-REFINE!!! */
VIR_BASIC_BLOCK* VIR_BB_GetFirstSuccBB(VIR_BASIC_BLOCK* bb);
VIR_BASIC_BLOCK* VIR_BB_GetSecondSuccBB(VIR_BASIC_BLOCK* bb);
VIR_BASIC_BLOCK* VIR_BB_GetLeadingBB(VIR_BASIC_BLOCK* bb);
VIR_BASIC_BLOCK* VIR_BB_GetFollowingBB(VIR_BASIC_BLOCK* bb);
VIR_BASIC_BLOCK* VIR_BB_GetJumpToBB(VIR_BASIC_BLOCK* bb);
VSC_ErrCode
VIR_BB_ChangeSuccBBs(
    VIR_BASIC_BLOCK* bb,
    VIR_BASIC_BLOCK* newJmpTo,
    VIR_BASIC_BLOCK* newFallThru
    );
void
VIR_BB_RemoveBranch(
    VIR_BASIC_BLOCK* bb,
    gctBOOL setNop
    );
VSC_ErrCode
VIR_BB_Append(
    VIR_BASIC_BLOCK* target,
    VIR_BASIC_BLOCK* source,
    gctBOOL beforeTargetBranch,
    gctBOOL sourceBranchExcluded
    );
VSC_ErrCode
VIR_BB_CopyBBBefore(
    VIR_BASIC_BLOCK* source,
    VIR_BASIC_BLOCK* before,
    VIR_BASIC_BLOCK** copy
    );
VSC_ErrCode
VIR_BB_CopyBBAfter(
    VIR_BASIC_BLOCK* source,
    VIR_BASIC_BLOCK* after,
    VIR_BASIC_BLOCK** copy
    );
VSC_ErrCode
VIR_BB_InsertBBBefore(
    VIR_BB* before,
    VIR_OpCode opcode,
    VIR_BB** newBB
    );
VSC_ErrCode
VIR_BB_InsertBBAfter(
    VIR_BB* after,
    VIR_OpCode opcode,
    VIR_BB** newBB
    );

/* (post-) DOM related functions */
VSC_ErrCode vscVIR_BuildDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_DestroyDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_BuildPostDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_DestroyPostDOMTreePerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_BuildDOMTree(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyDOMTree(VIR_Shader* pShader);
VSC_ErrCode vscVIR_BuildPostDOMTree(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyPostDOMTree(VIR_Shader* pShader);

/* (Reversed-) control dependency related functions */
VSC_ErrCode vscVIR_BuildControlDepPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_DestroyControlDepPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_BuildDomFrontierPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_DestroyDomFrontierPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG);
VSC_ErrCode vscVIR_BuildControlDep(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyControlDep(VIR_Shader* pShader);
VSC_ErrCode vscVIR_BuildDomFrontier(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyDomFrontier(VIR_Shader* pShader);

/* BB reach-relation functions */
VSC_ErrCode vscVIR_BuildBbReachRelation(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyBbReachRelation(VIR_Shader* pShader);

VSC_ErrCode vscVIR_CleanDfsVisitOrderIdxOnFunc(VIR_Function* pFunc);
VSC_ErrCode vscVIR_CleanDfsVisitOrderIdxOnShader(VIR_Shader* pShader);

#if LOOP_HIERARCHY_SUPPORT_IN_CFG
/* Loop hierarchy related functions */
VSC_ErrCode vscVIR_BuildLoopHierarchyPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_LOOP_HIERARCHY* pLoopHierarchy);
VSC_ErrCode vscVIR_DestroyLoopHierarchyPerCFG(VIR_CONTROL_FLOW_GRAPH* pCFG, VIR_LOOP_HIERARCHY* pLoopHierarchy);
VSC_ErrCode vscVIR_BuildLoopHierarchy(VIR_Shader* pShader, VIR_LOOP_HIERARCHY* pLoopHierarchy);
VSC_ErrCode vscVIR_DestroyLoopHierarchy(VIR_Shader* pShader, VIR_LOOP_HIERARCHY* pLoopHierarchy);
#endif

END_EXTERN_C()

#endif /* __gc_vsc_vir_cfa_h_ */

