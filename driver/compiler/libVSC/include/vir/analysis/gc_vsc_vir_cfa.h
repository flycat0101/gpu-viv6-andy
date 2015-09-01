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


#ifndef __gc_vsc_vir_cfa_h_
#define __gc_vsc_vir_cfa_h_

/* Here we define all control flow related stuff based on VIR, such as CG/CFG/DOM/CDG/DFG, etc.
   All these are built on graph or tree. */

BEGIN_EXTERN_C()

typedef struct _VIR_CONTROL_FLOW_GRAPH  VIR_CONTROL_FLOW_GRAPH;
typedef struct _VIR_CALL_GRAPH          VIR_CALL_GRAPH;
typedef struct _VIR_BASIC_BLOCK         VIR_BASIC_BLOCK;
typedef struct _VIR_FUNC_BLOCK          VIR_FUNC_BLOCK;

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
#define DOM_TREE_ITERATOR_ITERATOR_INIT(iter, pDomTree)    VSC_TNODE_LIST_ITERATOR_INIT((iter), pDomTree)
#define DOM_TREE_ITERATOR_ITERATOR_FIRST(iter)             VSC_TNODE_LIST_ITERATOR_FIRST((iter))
#define DOM_TREE_ITERATOR_ITERATOR_NEXT(iter)              VSC_TNODE_LIST_ITERATOR_NEXT((iter))
#define DOM_TREE_ITERATOR_ITERATOR_PREV(iter)              VSC_TNODE_LIST_ITERATOR_PREV((iter))
#define DOM_TREE_ITERATOR_ITERATOR_LAST(iter)              VSC_TNODE_LIST_ITERATOR_LAST((iter))

typedef VSC_CHILD_LIST_ITERATOR DOM_TREE_NODE_CHILD_ITERATOR;
#define DOM_TREE_NODE_CHILD_ITERATOR_INIT(iter, pDomTreeNode) VSC_CHILD_LIST_ITERATOR_INIT((iter), &(pDomTreeNode)->treeNode)
#define DOM_TREE_NODE_CHILD_ITERATOR_FIRST(iter)              (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_FIRST((iter))
#define DOM_TREE_NODE_CHILD_ITERATOR_NEXT(iter)               (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_NEXT((iter))
#define DOM_TREE_NODE_CHILD_ITERATOR_LAST(iter)               (VIR_DOM_TREE_NODE*)VSC_CHILD_LIST_ITERATOR_LAST((iter))

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
    VIR_FLOW_TYPE_LOOPHEAD,
    VIR_FLOW_TYPE_LOOPTAIL,
    VIR_FLOW_TYPE_BREAK,
    VIR_FLOW_TYPE_CONTINUE,
    VIR_FLOW_TYPE_CALL,
    VIR_FLOW_TYPE_RET,
} VIR_FLOW_TYPE;

typedef enum _VIR_CFG_EDGE_TYPE
{
    VIR_CFG_EDGE_TYPE_ALWAYS    = 0,
    VIR_CFG_EDGE_TYPE_TRUE,
    VIR_CFG_EDGE_TYPE_FALSE
}VIR_CFG_EDGE_TYPE;

typedef struct _VIR_CFG_EDGE
{
    VSC_DG_EDGE               dgEdge;
    VIR_CFG_EDGE_TYPE         type;
}VIR_CFG_EDGE;

#define CFG_EDGE_GET_TO_BB(pCfgEdge)    ((VIR_BASIC_BLOCK*)((pCfgEdge)->dgEdge.pToNode))

#define VIR_BB_FLAG_HavingLLI          0x1

struct _VIR_BASIC_BLOCK
{
    /* Graph node. It must be put at FIRST place!!!! */
    VSC_DG_NODE               dgNode;

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

    /* DOM and postDOM set indicating who (post-) dominate the BB */
    VSC_BIT_VECTOR            domSet;
    VSC_BIT_VECTOR            postDomSet;

    /* Node in DOM tree and postDOM tree */
    VIR_DOM_TREE_NODE*        pDomTreeNode;
    VIR_DOM_TREE_NODE*        pPostDomTreeNode;

    /* Who are this BB control (reversed-) dependent on?? */
    VSC_BIT_VECTOR            dfSet;
    VSC_BIT_VECTOR            cdSet;

    /* BB flags */
    gctUINT32                 flags;
};

#define BB_GET_ID(pBB)          ((pBB)->dgNode.id)
#define BB_GET_FUNC(pBB)        ((pBB)->pOwnerCFG->pOwnerFuncBlk->pVIRFunc)
#define BB_GET_START_INST(pBB)  ((pBB)->pStartInst)
#define BB_GET_END_INST(pBB)    ((pBB)->pEndInst)
#define BB_GET_LENGTH(pBB)      ((pBB)->instCount)
#define BB_GET_FLOWTYPE(pBB)    ((pBB)->flowType)

#define BB_INC_LENGTH(pBB)      ((pBB)->instCount ++)
#define BB_DEC_LENGTH(pBB)      ((pBB)->instCount --)

#define BB_SET_START_INST(pBB, pInst)  ((pBB)->pStartInst = (pInst))
#define BB_SET_END_INST(pBB, pInst)    ((pBB)->pEndInst = (pInst))

#define BB_FLAGS_GET_LLI(pBB)         ((pBB)->flags & VIR_BB_FLAG_HavingLLI)
#define BB_FLAGS_SET_LLI(pBB)         ((pBB)->flags |= VIR_BB_FLAG_HavingLLI)
#define BB_FLAGS_RESET_LLI(pBB)       ((pBB)->flags &= ~((gctUINT32)VIR_BB_FLAG_HavingLLI))

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
};

typedef VSC_GNODE_LIST_ITERATOR CFG_ITERATOR;
#define CFG_ITERATOR_INIT(iter, pCFG)    VSC_GNODE_LIST_ITERATOR_INIT((iter), &(pCFG)->dgGraph)
#define CFG_ITERATOR_FIRST(iter)         (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_FIRST((iter))
#define CFG_ITERATOR_NEXT(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_NEXT((iter))
#define CFG_ITERATOR_PREV(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_PREV((iter))
#define CFG_ITERATOR_LAST(iter)          (VIR_BASIC_BLOCK*)VSC_GNODE_LIST_ITERATOR_LAST((iter))

#define CFG_GET_ENTRY_BB(pCFG)     (*(VIR_BASIC_BLOCK**)vscSRARR_GetElement      \
                                    (&(pCFG)->dgGraph.rootNodeArray, 0))

#define CFG_GET_EXIT_BB(pCFG)      (*(VIR_BASIC_BLOCK**)vscSRARR_GetElement      \
                                    (&(pCFG)->dgGraph.tailNodeArray, 0))

#define CFG_GET_BB_BY_ID(pCFG, id) (VIR_BASIC_BLOCK*)vscDG_GetNodeById(&(pCFG)->dgGraph, (id))

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

    /* Memory pool that this CG is built on */
    VSC_PRIMARY_MEM_POOL      pmp;
};

typedef VSC_GNODE_LIST_ITERATOR CG_ITERATOR;
#define CG_ITERATOR_INIT(iter, pCG)    VSC_GNODE_LIST_ITERATOR_INIT((iter), &(pCG)->dgGraph)
#define CG_ITERATOR_FIRST(iter)        VSC_GNODE_LIST_ITERATOR_FIRST((iter))
#define CG_ITERATOR_NEXT(iter)         VSC_GNODE_LIST_ITERATOR_NEXT((iter))
#define CG_ITERATOR_PREV(iter)         VSC_GNODE_LIST_ITERATOR_PREV((iter))
#define CG_ITERATOR_LAST(iter)         VSC_GNODE_LIST_ITERATOR_LAST((iter))

#define CG_GET_MAIN_FUNC(pCG) ((*(VIR_FUNC_BLOCK**)vscSRARR_GetElement      \
                               (&(pCG)->dgGraph.rootNodeArray, 0))->pVIRFunc)



/* CG related functions */
VSC_ErrCode vscVIR_BuildCallGraph(VIR_Shader* pShader, VIR_CALL_GRAPH* pCg);
VSC_ErrCode vscVIR_DestroyCallGraph(VIR_CALL_GRAPH* pCg);

/* CFG related functions */
VSC_ErrCode vscVIR_BuildCFGPerFunc(VIR_Function* pFunc);
VSC_ErrCode vscVIR_DestroyCFGPerFunc(VIR_Function* pFunc);
VSC_ErrCode vscVIR_BuildCFG(VIR_Shader* pShader);
VSC_ErrCode vscVIR_DestroyCFG(VIR_Shader* pShader);

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

END_EXTERN_C()

#endif /* __gc_vsc_vir_cfa_h_ */

