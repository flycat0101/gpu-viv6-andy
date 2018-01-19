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


#include "gc_vsc.h"

void vscTREEND_Initialize(VSC_TREE_NODE* pNode)
{
    vscBLN_Initialize(&pNode->biListNode);
    vscULNDEXT_Initialize(&pNode->asSiblingNode, pNode);
    vscUNILST_Initialize(&pNode->childrenList, gcvFALSE);

    pNode->id = INVALID_TNODE_ID;
    pNode->depth = INVALID_TREE_DEPTH;
    pNode->pParentNode = gcvNULL;
}

void vscTREEND_Finalize(VSC_TREE_NODE* pNode)
{
    vscBLN_Finalize(&pNode->biListNode);
    vscULNDEXT_Finalize(&pNode->asSiblingNode);
    vscUNILST_Finalize(&pNode->childrenList);

    pNode->id = INVALID_TNODE_ID;
    pNode->depth = INVALID_TREE_DEPTH;
    pNode->pParentNode = gcvNULL;
}

void vscTREENDEXT_Initialize(VSC_TREE_NODE_EXT* pExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pExtNode->baseNode, pUserData);
    vscTREEND_Initialize(&pExtNode->treeNode);
}

void vscTREENDEXT_Finalize(VSC_TREE_NODE_EXT* pExtNode)
{
    vscTREEND_Finalize(&pExtNode->treeNode);
}

void* vscTREENDEXT_GetContainedUserData(VSC_TREE_NODE_EXT* pExtNode)
{
    return vscBSNODE_GetContainedUserData(&pExtNode->baseNode);
}

gctBOOL TREE_NODE_CMP(void* pNode1, void* pNode2)
{
    VSC_TREE_NODE** ppNode1 = (VSC_TREE_NODE**)pNode1;
    VSC_TREE_NODE** ppNode2 = (VSC_TREE_NODE**)pNode2;

    return (*ppNode1 == *ppNode2);
}

VSC_TREE* vscTREE_Create(VSC_MM* pMM, gctUINT leafInitAllocCount)
{
    VSC_TREE*   pTree = gcvNULL;

    pTree = (VSC_TREE*)vscMM_Alloc(pMM, sizeof(VSC_TREE));
    vscTREE_Initialize(pTree, pMM, leafInitAllocCount);

    return pTree;
}

void vscTREE_Initialize(VSC_TREE* pTree, VSC_MM* pMM, gctUINT leafInitAllocCount)
{
    pTree->pMM = pMM;
    pTree->nextNodeId = 0;
    pTree->pRootNode = gcvNULL;
    TNLST_INITIALIZE(&pTree->nodeList);
    vscSRARR_Initialize(&pTree->leafNodeArray, pMM, leafInitAllocCount, sizeof(VSC_TREE_NODE*), TREE_NODE_CMP);
}

void vscTREE_Finalize(VSC_TREE* pTree)
{
    TNLST_FINALIZE(&pTree->nodeList);
    pTree->nextNodeId = 0;
    pTree->pRootNode = gcvNULL;
    vscSRARR_Finalize(&pTree->leafNodeArray);
    pTree->pMM = gcvNULL;
}

void vscTree_Destroy(VSC_TREE* pTree)
{
    if (pTree)
    {
        vscTREE_Finalize(pTree);

        /* Free dg itself */
        vscMM_Free(pTree->pMM, pTree);
        pTree = gcvNULL;
    }
}

static void _AddSubTreeNodesToNodeList(VSC_TREE* pTree, VSC_TREE_NODE* pRootOfSubTree,
                                       gctUINT depth, gctBOOL bUpdateDepthOnly)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;

    if (!bUpdateDepthOnly)
    {
        if (pRootOfSubTree->id == INVALID_TNODE_ID)
        {
            TNLST_ADD_NODE(&pTree->nodeList, pRootOfSubTree);
            pRootOfSubTree->id = pTree->nextNodeId ++;
        }

        if (vscUNILST_GetNodeCount(&pRootOfSubTree->childrenList) == 0)
        {
            vscSRARR_AddElement(&pTree->leafNodeArray, (void*)&pRootOfSubTree);
        }
    }

    pRootOfSubTree->depth = depth;

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pRootOfSubTree);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        _AddSubTreeNodesToNodeList(pTree, pThisChildNode, depth + 1, bUpdateDepthOnly);
    }
}

static void _RemoveSubTreeNodesFromNodeList(VSC_TREE* pTree, VSC_TREE_NODE* pRootOfSubTree, gctUINT depthDelta)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;

    gcmASSERT(pRootOfSubTree->id != INVALID_TNODE_ID);

    TNLST_REMOVE_NODE(&pTree->nodeList, pRootOfSubTree);
    pRootOfSubTree->id = INVALID_TNODE_ID;

    if (vscUNILST_GetNodeCount(&pRootOfSubTree->childrenList) == 0)
    {
        vscSRARR_RemoveElementByContent(&pTree->leafNodeArray, (void*)&pRootOfSubTree);
    }

    pRootOfSubTree->depth -= depthDelta;

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pRootOfSubTree);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        _RemoveSubTreeNodesFromNodeList(pTree, pThisChildNode, depthDelta);
    }
}

static gctBOOL _IsNodeInSubTree(VSC_TREE_NODE* pSubTree, VSC_TREE_NODE* pNode)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;

    if (pSubTree == pNode)
    {
        return gcvTRUE;
    }

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pSubTree);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        if (_IsNodeInSubTree(pThisChildNode, pNode))
        {
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

void vscTREE_AddSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pGraftPoint, VSC_TREE_NODE* pRootOfSubTree)
{
    gcmASSERT(TNLST_GET_NODE_COUNT(&pTree->nodeList) != 0 || pGraftPoint == gcvNULL);
    gcmASSERT(_IsNodeInSubTree(pTree->pRootNode, pGraftPoint));

    /* Add all nodes in sub-tree to tree node list */
    _AddSubTreeNodesToNodeList(pTree, pRootOfSubTree,
                               (pGraftPoint == gcvNULL) ? 0 : (pGraftPoint->depth + 1), gcvFALSE);

    if (pGraftPoint == gcvNULL)
    {
        /* We are adding root of tree */
        gcmASSERT(pTree->pRootNode == gcvNULL);

        pTree->pRootNode = pRootOfSubTree;
        pRootOfSubTree->pParentNode = gcvNULL;
    }
    else
    {
        gcmASSERT(pGraftPoint->id != INVALID_TNODE_ID);

        vscUNILST_Append(&pGraftPoint->childrenList,
                     (VSC_UNI_LIST_NODE*)&pRootOfSubTree->asSiblingNode);
        if (vscUNILST_GetNodeCount(&pGraftPoint->childrenList) == 1)
        {
            vscSRARR_RemoveElementByContent(&pTree->leafNodeArray, (void*)&pGraftPoint);
        }

        pRootOfSubTree->pParentNode = pGraftPoint;
    }
}

void vscTREE_RemoveSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pRootOfSubTree)
{
    /* If sub-tree is already decoupled from tree, just return */
    if (pRootOfSubTree->id == INVALID_TNODE_ID)
    {
        return;
    }

    /* Topoloy update */
    if (pRootOfSubTree->pParentNode)
    {
        vscUNILST_Remove(&pRootOfSubTree->pParentNode->childrenList,
                         (VSC_UNI_LIST_NODE*)&pRootOfSubTree->asSiblingNode);

        if (vscUNILST_GetNodeCount(&pRootOfSubTree->pParentNode->childrenList) == 0)
        {
            vscSRARR_AddElement(&pTree->leafNodeArray, (void*)&pRootOfSubTree->pParentNode);
        }

        pRootOfSubTree->pParentNode = gcvNULL;
    }

    /* Remove all nodes in sub-tree from tree node list */
    _RemoveSubTreeNodesFromNodeList(pTree, pRootOfSubTree, pRootOfSubTree->depth);

    /* If tree has been empty, we can reset id pool */
    if (TNLST_GET_NODE_COUNT(&pTree->nodeList) == 0)
    {
        pTree->pRootNode =  gcvNULL;
        pTree->nextNodeId = 0;
    }
}

void vscTREE_MoveSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pNewGraftPoint, VSC_TREE_NODE* pRootOfSubTree)
{
    /* If sub-tree or graft point is already decoupled from tree, just return */
    if (pRootOfSubTree->id == INVALID_TNODE_ID || pNewGraftPoint->id == INVALID_TNODE_ID)
    {
        gcmASSERT(gcvFALSE);
        return;
    }

    /* No need to move for whole tree or same graft point */
    if ((pRootOfSubTree->pParentNode == gcvNULL) || (pNewGraftPoint == pRootOfSubTree->pParentNode))
    {
        return;
    }

    /* Check whether new graft point is inside of sub-tree, if so, just abort it */
    if (_IsNodeInSubTree(pRootOfSubTree, pNewGraftPoint))
    {
        gcmASSERT(gcvFALSE);
        return;
    }

    /* Remove from old graft point */
    vscUNILST_Remove(&pRootOfSubTree->pParentNode->childrenList,
                     (VSC_UNI_LIST_NODE*)&pRootOfSubTree->asSiblingNode);
    if (vscUNILST_GetNodeCount(&pRootOfSubTree->pParentNode->childrenList) == 0)
    {
        vscSRARR_AddElement(&pTree->leafNodeArray, (void*)&pRootOfSubTree->pParentNode);
    }

    /* Add to new graft point */
    vscUNILST_Append(&pNewGraftPoint->childrenList,
                     (VSC_UNI_LIST_NODE*)&pRootOfSubTree->asSiblingNode);
    if (vscUNILST_GetNodeCount(&pNewGraftPoint->childrenList) == 1)
    {
        vscSRARR_RemoveElementByContent(&pTree->leafNodeArray, (void*)&pNewGraftPoint);
    }

    pRootOfSubTree->pParentNode = pNewGraftPoint;

    /* Update depth */
    _AddSubTreeNodesToNodeList(pTree, pRootOfSubTree, pNewGraftPoint->depth + 1, gcvTRUE);
}

void vscTREE_MergeTwoTrees(VSC_TREE* pDstTree, VSC_TREE* pSrcTree, VSC_TREE_NODE* pGraftPointInDstTree)
{
    VSC_TREE_NODE* pRootOfSrc = pSrcTree->pRootNode;

    gcmASSERT(_IsNodeInSubTree(pDstTree->pRootNode, pGraftPointInDstTree));

    vscTREE_RemoveSubTree(pSrcTree, pRootOfSrc);
    vscTREE_AddSubTree(pDstTree, pGraftPointInDstTree, pRootOfSrc);
}

gctUINT vscTREE_GetNodeCount(VSC_TREE* pTree)
{
    return TNLST_GET_NODE_COUNT(&pTree->nodeList);
}

gctUINT vscTREE_GetHistNodeCount(VSC_TREE* pTree)
{
    return pTree->nextNodeId;
}

gctUINT vscTREE_GetLeafCount(VSC_TREE* pTree)
{
    return vscSRARR_GetElementCount(&pTree->leafNodeArray);
}

static void _DoTraversal(VSC_TREE* pTree,
                         VSC_TREE_NODE* pNode,
                         gctBOOL bPostOrder,
                         VSC_TREE_NODE** ppRetNodeOrder,
                         gctUINT *pOrderIdx)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;

    if (!bPostOrder)
    {
        ppRetNodeOrder[(*pOrderIdx) ++] = pNode;
    }

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pNode);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        _DoTraversal(pTree, pThisChildNode, bPostOrder, ppRetNodeOrder, pOrderIdx);
    }

    if (bPostOrder)
    {
        ppRetNodeOrder[(*pOrderIdx) ++] = pNode;
    }
}

void vscTREE_PreOrderTraversal(VSC_TREE* pTree, VSC_TREE_NODE** ppRetNodeOrder)
{
     gctUINT                     preOrderIdx = 0;

    _DoTraversal(pTree, pTree->pRootNode, gcvFALSE, ppRetNodeOrder, &preOrderIdx);
}

void vscTREE_PstOrderTraversal(VSC_TREE* pTree, VSC_TREE_NODE** ppRetNodeOrder)
{
    gctUINT                     preOrderIdx = 0;

    _DoTraversal(pTree, pTree->pRootNode, gcvTRUE, ppRetNodeOrder, &preOrderIdx);
}

static void _DoTraversalCB(VSC_TREE* pTree,
                           VSC_TREE_NODE* pNode,
                           PFN_TREE_NODE_HANLDER pfnHandlerOwnPre,
                           PFN_TREE_NODE_HANLDER pfnHandlerOwnPost,
                           PFN_TREE_NODE_HANLDER pfnHandlerDescendantPre,
                           PFN_TREE_NODE_HANLDER pfnHandlerDescendantPost,
                           void* pParam)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;

    SAFE_CALL_TREE_NODE_HANDLER_RETURN(pfnHandlerOwnPre, pNode, pParam);

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pNode);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        SAFE_CALL_TREE_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPre, pThisChildNode, pParam);

        _DoTraversalCB(pTree, pThisChildNode,
                       pfnHandlerOwnPre,
                       pfnHandlerOwnPost,
                       pfnHandlerDescendantPre,
                       pfnHandlerDescendantPost,
                       pParam);

        SAFE_CALL_TREE_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPost, pThisChildNode, pParam);
    }

    SAFE_CALL_TREE_NODE_HANDLER_RETURN(pfnHandlerOwnPost, pNode, pParam);
}

void vscTREE_TraversalCB(VSC_TREE* pTree,
                         PFN_TREE_NODE_HANLDER pfnHandlerOwnPre,
                         PFN_TREE_NODE_HANLDER pfnHandlerOwnPost,
                         PFN_TREE_NODE_HANLDER pfnHandlerDescendantPre,
                         PFN_TREE_NODE_HANLDER pfnHandlerDescendantPost,
                         void* pParam)
{
    _DoTraversalCB(pTree, pTree->pRootNode,
                   pfnHandlerOwnPre,
                   pfnHandlerOwnPost,
                   pfnHandlerDescendantPre,
                   pfnHandlerDescendantPost,
                   pParam);
}

typedef struct _FLCA_PARAM
{
    VSC_TREE_NODE* pTreeNodeA;
    VSC_TREE_NODE* pTreeNodeB;
    VSC_TREE_NODE* pPotentialNode;
}FLCA_PARAM;

static VSC_TREE_NODE* _FindLeastCommAncestor(VSC_TREE* pTree, VSC_TREE_NODE* pThisNode, FLCA_PARAM* pParam)
{
    VSC_CHILD_LIST_ITERATOR childIter;
    VSC_TREE_NODE*          pThisChildNode;
    VSC_TREE_NODE*          pRet = gcvNULL;

    if (pThisNode == pParam->pTreeNodeA || pThisNode == pParam->pTreeNodeB)
    {
        if (pParam->pPotentialNode == gcvNULL)
        {
            /* If A or B was not identified before, we identify it now */
            pParam->pPotentialNode = pThisNode->pParentNode;
            return gcvNULL;
        }
        else
        {
            /* If A or B has been identified before, and we find another, ok, we find least common
               ancestor, so just return. */
            return pParam->pPotentialNode;
        }
    }

    VSC_CHILD_LIST_ITERATOR_INIT(&childIter, pThisNode);
    pThisChildNode = VSC_CHILD_LIST_ITERATOR_FIRST(&childIter);
    for (; pThisChildNode != gcvNULL; pThisChildNode = VSC_CHILD_LIST_ITERATOR_NEXT(&childIter))
    {
        /* If we has found the least common ancestor, just return, no need to traversal other nodes */
        pRet = _FindLeastCommAncestor(pTree, pThisChildNode, pParam);
        if (pRet)
        {
            return pRet;
        }
    }

    /* Up-propagate potential node */
    if (pParam->pPotentialNode == pThisNode)
    {
        pParam->pPotentialNode = pThisNode->pParentNode;
    }

    return gcvNULL;
}

VSC_TREE_NODE* vscTREE_GetLeastCommAncestor(VSC_TREE* pTree, VSC_TREE_NODE* pTreeNodeA, VSC_TREE_NODE* pTreeNodeB)
{
    FLCA_PARAM param =
    {
        pTreeNodeA,
        pTreeNodeB,
        gcvNULL
    };

    return _FindLeastCommAncestor(pTree, pTree->pRootNode, &param);
}

