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


#ifndef __gc_vsc_utils_tree_h_
#define __gc_vsc_utils_tree_h_

BEGIN_EXTERN_C()

typedef VSC_BI_LIST VSC_TREENODE_LIST;
typedef VSC_UNI_LIST VSC_CHILDNODE_LIST;

#define INVALID_TNODE_ID   0xFFFFFFFF
#define INVALID_TREE_DEPTH 0xFFFFFFFF

typedef struct _VSC_TREE_NODE
{
    /* Next/prev node in nodeList that tree maintains. It must be put at FIRST place!!!! */
    VSC_BI_LIST_NODE          biListNode;

    gctUINT                   id;

    /* All children nodes of this node */
    VSC_CHILDNODE_LIST        childrenList;

    /* When it is as child node of its parent, this designates the next child of same parent */
    VSC_UNI_LIST_NODE_EXT     asSiblingNode;

    /* Which node is the ancestor of this node */
    struct _VSC_TREE_NODE*    pParentNode;

    /* Depth in tree, root is zero */
    gctUINT                   depth;
}VSC_TREE_NODE;

void vscTREEND_Initialize(VSC_TREE_NODE* pNode);
void vscTREEND_Finalize(VSC_TREE_NODE* pNode);
#define CAST_TREEND_2_BLN(pNode)             (VSC_BI_LIST_NODE*)(pNode)
#define CAST_BLN_2_TREEND(pBln)              (VSC_TREE_NODE*)(pBln)
#define TREEND_GET_PREV_NODE(pNode)          CAST_BLN_2_TREEND(vscBLN_GetPrevNode(CAST_TREEND_2_BLN(pNode)))
#define TREEND_GET_NEXT_NODE(pNode)          CAST_BLN_2_TREEND(vscBLN_GetNextNode(CAST_TREEND_2_BLN(pNode)))
#define TREEND_GET_NEXT_SIBLING(pNode)       (VSC_TREE_NODE*)vscULNDEXT_GetContainedUserData((VSC_UNI_LIST_NODE_EXT*)  \
                                             vscULN_GetNextNode(&(pNode)->asSiblingNode))

typedef VSC_UL_ITERATOR VSC_CHILD_LIST_ITERATOR;
#define VSC_CHILD_LIST_ITERATOR_INIT(iter, pNode)     vscULIterator_Init((iter), &(pNode)->childrenList)
#define VSC_CHILD_LIST_ITERATOR_FIRST(iter)           (VSC_TREE_NODE*)vscULNDEXT_GetContainedUserData((VSC_UNI_LIST_NODE_EXT*) \
                                                      vscULIterator_First((iter)))
#define VSC_CHILD_LIST_ITERATOR_NEXT(iter)            (VSC_TREE_NODE*)vscULNDEXT_GetContainedUserData((VSC_UNI_LIST_NODE_EXT*) \
                                                      vscULIterator_Next((iter)))
#define VSC_CHILD_LIST_ITERATOR_LAST(iter)            (VSC_TREE_NODE*)vscULNDEXT_GetContainedUserData((VSC_UNI_LIST_NODE_EXT*) \
                                                      vscULIterator_Last((iter)))

typedef struct _VSC_TREE_NODE_EXT
{
    /* !!!!VSC_TREE_NODE must be put at FIRST place */
    VSC_TREE_NODE             treeNode;
    VSC_BASE_NODE             baseNode;
}VSC_TREE_NODE_EXT;

void vscTREENDEXT_Initialize(VSC_TREE_NODE_EXT* pExtNode, void* pUserData);
void vscTREENDEXT_Finalize(VSC_TREE_NODE_EXT* pExtNode);
void* vscTREENDEXT_GetContainedUserData(VSC_TREE_NODE_EXT* pExtNode);
#define CAST_TREENDEXT_2_TREEND(pExtNode)    (VSC_TREE_NODE*)(pExtEdge)
#define CAST_TREEND_2_TREENDEXT(pNode)       (VSC_TREE_NODE_EXT*)(pEdge)

/* Tree node-list */
#define TNLST_INITIALIZE(pNodeList)         vscBILST_Initialize((pNodeList), gcvFALSE)
#define TNLST_FINALIZE(pNodeList)           vscBILST_Finalize((pNodeList))
#define TNLST_ADD_NODE(pNodeList, pNode)    vscBILST_Append((pNodeList), CAST_TREEND_2_BLN(pNode))
#define TNLST_GET_FIRST_NODE(pNodeList)     CAST_BLN_2_TREEND(vscBILST_GetHead((pNodeList)))
#define TNLST_REMOVE_NODE(pNodeList, pNode) vscBILST_Remove((pNodeList), CAST_TREEND_2_BLN(pNode))
#define TNLST_GET_NODE_COUNT(pNodeList)     vscBILST_GetNodeCount(pNodeList)

typedef VSC_BL_ITERATOR VSC_TNODE_LIST_ITERATOR;
#define VSC_TNODE_LIST_ITERATOR_INIT(iter, pTree)     vscBLIterator_Init((iter), &(pTree)->nodeList)
#define VSC_TNODE_LIST_ITERATOR_FIRST(iter)           vscBLIterator_First((iter))
#define VSC_TNODE_LIST_ITERATOR_NEXT(iter)            vscBLIterator_Next((iter))
#define VSC_TNODE_LIST_ITERATOR_PREV(iter)            vscBLIterator_Prev((iter))
#define VSC_TNODE_LIST_ITERATOR_LAST(iter)            vscBLIterator_Last((iter))

typedef struct _VSC_TREE
{
    /* All nodes that this tree maintains are collected into list */
    VSC_TREENODE_LIST          nodeList;

    /* Node id generator */
    gctUINT                    nextNodeId;

    /* Root node */
    VSC_TREE_NODE*             pRootNode;

    /* All leaf nodes array */
    VSC_SIMPLE_RESIZABLE_ARRAY leafNodeArray;

    /* What type of MM are this tree built on? */
    VSC_MM*                    pMM;
}VSC_TREE;

/* To avoid user rewrite traversal routines, all traversal functions with callback can use
   user predefined node-handler to process user's data. Within handler, user should cast
   VSC_TREE_NODE to its derived node to access members of derived node. */
typedef gctBOOL (*PFN_TREE_NODE_HANLDER)(VSC_TREE* pTree, VSC_TREE_NODE* pNode, void* pParam);

#define SAFE_CALL_TREE_NODE_HANDLER_RETURN(treeNodeHandler, pNode, pParam)   \
    if ((treeNodeHandler))                                                   \
    {                                                                        \
        if ((treeNodeHandler)(pTree, (pNode), (pParam)))                     \
        {                                                                    \
            return;                                                          \
        }                                                                    \
    }

#define SAFE_CALL_TREE_NODE_HANDLER_CONTINUE(treeNodeHandler, pNode, pParam) \
    if ((treeNodeHandler))                                                   \
    {                                                                        \
        if ((treeNodeHandler)(pTree, (pNode), (pParam)))                     \
        {                                                                    \
            continue;                                                        \
        }                                                                    \
    }

/* Creation and destroy */
VSC_TREE* vscTREE_Create(VSC_MM* pMM, gctUINT leafInitAllocCount);
void vscTREE_Initialize(VSC_TREE* pTree, VSC_MM* pMM, gctUINT leafInitAllocCount);
void vscTREE_Finalize(VSC_TREE* pTree);
void vscTREE_Destroy(VSC_TREE* pTree);

/* Build tree */

/* vscTREE_AddSubTree can add a homeless sub-tree (that's why is not represented by VSC_TREE) into tree. while
   vscTREE_RemoveSubTree can make a part of tree as homeless sub-tree (that's why is not represented by VSC_TREE).
   For a homeless tree, although it has no VSC_TREE to manage its nodes, it still has tree topoloy, so you can
   still traversal it. The most important charater of homeless tree is id of each node is INVALID_TNODE_ID. */
void vscTREE_AddSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pGraftPoint, VSC_TREE_NODE* pRootOfSubTree);
void vscTREE_RemoveSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pRootOfSubTree);
void vscTREE_MoveSubTree(VSC_TREE* pTree, VSC_TREE_NODE* pNewGraftPoint, VSC_TREE_NODE* pRootOfSubTree);
void vscTREE_MergeTwoTrees(VSC_TREE* pDstTree, VSC_TREE* pSrcTree, VSC_TREE_NODE* pGraftPointInDstTree);

gctUINT vscTREE_GetNodeCount(VSC_TREE* pTree);
gctUINT vscTREE_GetHistNodeCount(VSC_TREE* pTree);
gctUINT vscTREE_GetLeafCount(VSC_TREE* pTree);

/* Traversal */

/* ppRetNodeOrder must be allocated with node count of tree which can be got from nodeList.info.count.
   This returned node order is what requested order */
void vscTREE_PreOrderTraversal(VSC_TREE* pTree, VSC_TREE_NODE** ppRetNodeOrder);
void vscTREE_PstOrderTraversal(VSC_TREE* pTree, VSC_TREE_NODE** ppRetNodeOrder);

/* This function won't return node order to user, instead, user can pass into 5 handlers to process
   node when node is accessing */
void vscTREE_TraversalCB(VSC_TREE* pTree,
                         PFN_TREE_NODE_HANLDER pfnHandlerOwnPre,
                         PFN_TREE_NODE_HANLDER pfnHandlerOwnPost,
                         PFN_TREE_NODE_HANLDER pfnHandlerDescendantPre,
                         PFN_TREE_NODE_HANLDER pfnHandlerDescendantPost,
                         void* pParam);

/* Analyze */
VSC_TREE_NODE* vscTREE_GetLeastCommAncestor(VSC_TREE* pTree, VSC_TREE_NODE* pTreeNodeA, VSC_TREE_NODE* pTreeNodeB);

END_EXTERN_C()

#endif /* __gc_vsc_utils_tree_h_ */

