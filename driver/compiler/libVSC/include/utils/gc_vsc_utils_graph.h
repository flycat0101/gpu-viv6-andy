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


#ifndef __gc_vsc_utils_graph_h_
#define __gc_vsc_utils_graph_h_

/* We define two types of graph here, directed graph and undirected graph. For undirected
   graph, because it is only used for RA when building interference graph, so design it with
   adjacent matrix. For directed graph, since it will be the most graph that VSC uses, such
   as CFG/DFG/DAG, etc, so we will use adjacent list to maintain */

BEGIN_EXTERN_C()

typedef VSC_BI_LIST VSC_GNODE_LIST;

#define GNODE_HASH_TABLE_SIZE            32
gctUINT _HFUNC_PassThroughNodeId(const void* pKey);

/* Linear list iterator, not the graph traversal iterator which is defined for each graph
   later in this header file */
typedef VSC_BL_ITERATOR VSC_GNODE_LIST_ITERATOR;
#define VSC_GNODE_LIST_ITERATOR_INIT(iter, pGraph)    vscBLIterator_Init((iter), &(pGraph)->nodeList)
#define VSC_GNODE_LIST_ITERATOR_FIRST(iter)           vscBLIterator_First((iter))
#define VSC_GNODE_LIST_ITERATOR_NEXT(iter)            vscBLIterator_Next((iter))
#define VSC_GNODE_LIST_ITERATOR_PREV(iter)            vscBLIterator_Prev((iter))
#define VSC_GNODE_LIST_ITERATOR_LAST(iter)            vscBLIterator_Last((iter))

#define INVALID_GNODE_ID   0xFFFFFFFF

#define MAX_EDGE_COUNT_TO_USE_RECURSION_FOR_DFS     (2056)

typedef enum _VSC_GRAPH_SEARCH_MODE
{
    VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST               = 0, /* By choosing recursive or iterative automatically. */
    VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE     = 1,
    VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE     = 2,
    VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW      = 3,
    VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE        = 4,
}VSC_GRAPH_SEARCH_MODE;

typedef enum _VSC_GRAPH_TRAVERSAL_ORDER
{
    VSC_GRAPH_TRAVERSAL_ORDER_PREV,
    VSC_GRAPH_TRAVERSAL_ORDER_POST
}VSC_GRAPH_TRAVERSAL_ORDER;

/* Internally used by iterator only */
typedef struct _VSC_GNODE_ORDER
{
    struct _VSC_DG_NODE** ppGNodeOrder;
    gctUINT               totalCount;
    gctUINT               curIndex;
}VSC_GNODE_ORDER;

/*
 * Undirected graph
 */

typedef struct _VSC_UDG_NODE
{
    /* Next/prev node in nodeList that graph maintains. It must be put at FIRST place!!!! */
    VSC_BI_LIST_NODE          biListNode;

    gctUINT                   id;

    /* Neighbour count */
    gctUINT                   degree;
}VSC_UDG_NODE;

void vscUDGND_Initialize(VSC_UDG_NODE* pNode);
void vscUDGND_Finalize(VSC_UDG_NODE* pNode);
#define CAST_UDGND_2_BLN(pNode)               (VSC_BI_LIST_NODE*)(pNode)
#define CAST_BLN_2_UDGND(pBln)                (VSC_UDG_NODE*)(pBln)
#define UDGND_GET_PREV_NODE(pNode)            CAST_BLN_2_UDGND(vscBLN_GetPrevNode(CAST_UDGND_2_BLN(pNode)))
#define UDGND_GET_NEXT_NODE(pNode)            CAST_BLN_2_UDGND(vscBLN_GetNextNode(CAST_UDGND_2_BLN(pNode)))
#define UDGND_GET_DEGREE(pNode)               ((pNode)->degree)

typedef struct _VSC_UDG_NODE_EXT
{
    /* !!!!VSC_UDG_NODE must be put at FIRST place */
    VSC_UDG_NODE              udgNode;
    VSC_BASE_NODE             baseNode;
}VSC_UDG_NODE_EXT;

void vscUDGNDEXT_Initialize(VSC_UDG_NODE_EXT* pExtNode, void* pUserData);
void vscUDGNDEXT_Finalize(VSC_UDG_NODE_EXT* pExtNode);
void* vscUDGNDEXT_GetContainedUserData(VSC_UDG_NODE_EXT* pExtNode);
#define CAST_UDGNDEXT_2_UDGND(pUdgExtNode)    (VSC_UDG_NODE*)(pUdgExtNode)
#define CAST_UDGND_2_UDGNDEXT(pUdgNode)       (VSC_UDG_NODE_EXT*)(pUdgNode)

/* UDG-node list */
#define UDGNLST_INITIALIZE(pNodeList)         vscBILST_Initialize((pNodeList), gcvFALSE)
#define UDGNLST_FINALIZE(pNodeList)           vscBILST_Finalize((pNodeList))
#define UDGNLST_ADD_NODE(pNodeList, pNode)    vscBILST_Append((pNodeList), CAST_UDGND_2_BLN(pNode))
#define UDGNLST_GET_FIRST_NODE(pNodeList)     CAST_BLN_2_UDGND(vscBILST_GetHead((pNodeList)))
#define UDGNLST_REMOVE_NODE(pNodeList, pNode) vscBILST_Remove((pNodeList), CAST_UDGND_2_BLN(pNode))
#define UDGNLST_GET_NODE_COUNT(pNodeList)     vscBILST_GetNodeCount(pNodeList)

typedef struct _VSC_UNDIRECTED_GRAPH
{
    /* All nodes that this graph maintains are collected into list */
    VSC_GNODE_LIST            nodeList;

    /* Node id generator */
    gctUINT                   nextNodeId;

    /* Adjacent bit matrix */
    VSC_BIT_MATRIX            bitMatrix;
    gctUINT                   matrixWidth;

    /* To hash nodes by id */
    VSC_HASH_TABLE            ndHashTable;

    /* What type of MM are this graph built on? */
    VSC_MM*                   pMM;
}VSC_UNDIRECTED_GRAPH;

/* Creation and destroy */
VSC_UNDIRECTED_GRAPH* vscUDG_Create(VSC_MM* pMM, gctUINT initNodeCount);
void vscUDG_Initialize(VSC_UNDIRECTED_GRAPH* pUDG, VSC_MM* pMM, gctUINT initNodeCount);
void vscUDG_Finalize(VSC_UNDIRECTED_GRAPH* pUDG);
void vscUDG_Destroy(VSC_UNDIRECTED_GRAPH* pUDG);

/* Build graph */
void vscUDG_AddNode(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode);
void vscUDG_RemoveNode(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode);
void vscUDG_ConnectTwoNodes(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode1, VSC_UDG_NODE* pNode2);

VSC_UDG_NODE* vscUDG_GetNodeById(VSC_UNDIRECTED_GRAPH* pUDG, gctUINT nodeId);
gctUINT vscUDG_GetNodeCount(VSC_UNDIRECTED_GRAPH* pUDG);
gctUINT vscUDG_GetHistNodeCount(VSC_UNDIRECTED_GRAPH* pUDG);

/*
 * Directed graph
 */
typedef struct _VSC_DG_NODE   VSC_DG_NODE;

typedef struct _VSC_DG_EDGE
{
    /* Next edge of adjacent list. It must be put at FIRST place!!!! */
    VSC_UNI_LIST_NODE         uniLstNode;

    gctUINT                   id;

    /* Tail node of edge */
    VSC_DG_NODE*              pFromNode;

    /* Head node of edge */
    VSC_DG_NODE*              pToNode;
}VSC_DG_EDGE;

void vscDGEG_Initialize(VSC_DG_EDGE* pEdge, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode);
void vscDGEG_Finalize(VSC_DG_EDGE* pEdge);
#define CAST_DGEG_2_ULN(pEdge)             (VSC_UNI_LIST_NODE*)(pEdge)
#define CAST_ULN_2_DGEG(pUln)              (VSC_DG_EDGE*)(pUln)
#define DGEG_GET_NEXT_EDGE(pEdge)          CAST_ULN_2_DGEG(vscULN_GetNextNode(CAST_DGEG_2_ULN(pEdge)))

typedef struct _VSC_DG_EDGE_EXT
{
    /* !!!!VSC_DG_EDGE must be put at FIRST place */
    VSC_DG_EDGE               dgEdge;
    VSC_BASE_NODE             baseNode;
}VSC_DG_EDGE_EXT;

void vscDGEGEXT_Initialize(VSC_DG_EDGE_EXT* pExtEdge, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, void* pUserData);
void vscDGEGEXT_Finalize(VSC_DG_EDGE_EXT* pExtEdge);
void* vscDGEGEXT_GetContainedUserData(VSC_DG_EDGE_EXT* pExtEdge);
#define CAST_DGEGEXT_2_DGEG(pDgExtEdge)    (VSC_DG_EDGE*)(pDgExtEdge)
#define CAST_DGEG_2_DGEGEXT(pDgEdge)       (VSC_DG_EDGE_EXT*)(pDgEdge)

/* Adjacent (edge) list */
typedef VSC_UNI_LIST VSC_ADJACENT_LIST;
#define AJLST_INITIALIZE(pAdjList)         vscUNILST_Initialize((pAdjList), gcvFALSE)
#define AJLST_FINALIZE(pAdjList)           vscUNILST_Finalize((pAdjList))
#define AJLST_ADD_EDGE(pAdjList, pEdge)    vscUNILST_Append((pAdjList), CAST_DGEG_2_ULN((pEdge)))
#define AJLST_GET_FIRST_EDGE(pAdjList)     CAST_ULN_2_DGEG(vscUNILST_GetHead((pAdjList)))
#define AJLST_GET_LAST_EDGE(pAdjList)      CAST_ULN_2_DGEG(vscUNILST_GetTail((pAdjList)))
#define AJLST_REMOVE_EDGE(pAdjList, pEdge) vscUNILST_Remove((pAdjList), CAST_DGEG_2_ULN((pEdge)))
#define AJLST_CHECK_EMPTY(pAdjList)        vscUNILST_IsEmpty((pAdjList))
#define AJLST_GET_EDGE_COUNT(pAdjList)     vscUNILST_GetNodeCount((pAdjList))

typedef VSC_UL_ITERATOR VSC_ADJACENT_LIST_ITERATOR;
#define VSC_ADJACENT_LIST_ITERATOR_INIT(iter, pAdjList)  vscULIterator_Init((iter), pAdjList)
#define VSC_ADJACENT_LIST_ITERATOR_FIRST(iter)           vscULIterator_First((iter))
#define VSC_ADJACENT_LIST_ITERATOR_NEXT(iter)            vscULIterator_Next((iter))
#define VSC_ADJACENT_LIST_ITERATOR_LAST(iter)            vscULIterator_Last((iter))

struct _VSC_DG_NODE
{
    /* Next/prev node in nodeList that graph maintains. It must be put at FIRST place!!!! */
    VSC_BI_LIST_NODE          biListNode;

    gctUINT                   id;

    /* Out adjacent list of VSC_DG_EDGE, the edge count is out degree */
    VSC_ADJACENT_LIST         succList;

    /* In adjacent list of VSC_DG_EDGE, the edge count is in degree */
    VSC_ADJACENT_LIST         predList;

    /* Wether this node has been visited when traversal */
    gctBOOL                   bVisited;
};

void vscDGND_Initialize(VSC_DG_NODE* pNode);
void vscDGND_Finalize(VSC_DG_NODE* pNode);
#define CAST_DGND_2_BLN(pNode)               (VSC_BI_LIST_NODE*)(pNode)
#define CAST_BLN_2_DGND(pBln)                (VSC_DG_NODE*)(pBln)
#define DGND_GET_PREV_NODE(pNode)            CAST_BLN_2_DGND(vscBLN_GetPrevNode(CAST_DGND_2_BLN(pNode)))
#define DGND_GET_NEXT_NODE(pNode)            CAST_BLN_2_DGND(vscBLN_GetNextNode(CAST_DGND_2_BLN(pNode)))
#define DGND_GET_IN_DEGREE(pNode)            AJLST_GET_EDGE_COUNT(&(pNode)->predList)
#define DGND_GET_OUT_DEGREE(pNode)           AJLST_GET_EDGE_COUNT(&(pNode)->succList)

typedef VSC_ADJACENT_LIST_ITERATOR VSC_ADJACENT_NODE_ITERATOR;
void vscDGNDAJNIterator_Init(VSC_ADJACENT_NODE_ITERATOR*, VSC_DG_NODE*, gctBOOL);
VSC_DG_NODE* vscDGNDAJNIterator_First(VSC_ADJACENT_NODE_ITERATOR*);
VSC_DG_NODE* vscDGNDAJNIterator_Next(VSC_ADJACENT_NODE_ITERATOR*);
VSC_DG_NODE* vscDGNDAJNIterator_Last(VSC_ADJACENT_NODE_ITERATOR*);

typedef struct _VSC_DG_NODE_EXT
{
    /* !!!!VSC_DG_NODE must be put at FIRST place */
    VSC_DG_NODE               dgNode;
    VSC_BASE_NODE             baseNode;
}VSC_DG_NODE_EXT;

void vscDGNDEXT_Initialize(VSC_DG_NODE_EXT* pExtNode, void* pUserData);
void vscDGNDEXT_Finalize(VSC_DG_NODE_EXT* pExtNode);
void* vscDGNDEXT_GetContainedUserData(VSC_DG_NODE_EXT* pExtNode);
#define CAST_DGNDEXT_2_DGND(pDgExtNode)      (VSC_DG_NODE*)(pDgExtNode)
#define CAST_DGND_2_DGNDEXT(pDgNode)         (VSC_DG_NODE_EXT*)(pDgNode)

/* DG-node list */
#define DGNLST_INITIALIZE(pNodeList)         vscBILST_Initialize((pNodeList), gcvFALSE)
#define DGNLST_FINALIZE(pNodeList)           vscBILST_Finalize((pNodeList))
#define DGNLST_ADD_NODE(pNodeList, pNode)    vscBILST_Append((pNodeList), CAST_DGND_2_BLN(pNode))
#define DGNLST_GET_FIRST_NODE(pNodeList)     CAST_BLN_2_DGND(vscBILST_GetHead((pNodeList)))
#define DGNLST_REMOVE_NODE(pNodeList, pNode) vscBILST_Remove((pNodeList), CAST_DGND_2_BLN(pNode))
#define DGNLST_GET_NODE_COUNT(pNodeList)     vscBILST_GetNodeCount(pNodeList)

 /* Element comparator for array that maintains element of <VSC_DG_NODE*> */
gctBOOL DG_NODE_CMP(void* pNode1, void* pNode2);

typedef struct _VSC_DIRECTED_GRAPH
{
    /* All nodes that this graph maintains are collected into list */
    VSC_GNODE_LIST             nodeList;

    /* Node id generator */
    gctUINT                    nextNodeId;

    /* Edge id generator */
    gctUINT                    nextEdgeId;

    /* Edge data size. !!!!User-derived edge must put VSC_DG_EDGE as first member. */
    gctUINT                    edgeAllocSize;

    /* All entry nodes array */
    VSC_SIMPLE_RESIZABLE_ARRAY rootNodeArray;

    /* All exit nodes array */
    VSC_SIMPLE_RESIZABLE_ARRAY tailNodeArray;

    /* To hash nodes by id */
    VSC_HASH_TABLE             ndHashTable;

    /* What type of MM are this graph built on? */
    VSC_MM*                    pMM;
}VSC_DIRECTED_GRAPH;

/* To avoid user rewrite traversal routines, all traversal functions with callback can use
   user predefined node-handler to process user's data. Within handler, user should cast
   VSC_DG_NODE to its derived node to access members of derived node. !!!Also, it should be
   very CAREFUL if you want to change 'bVisited' of VSC_DG_NODE because that member controls
   traversal (It may cause deadlock or get wrong traversal result) !!!!!!!! */
typedef gctBOOL (*PFN_DG_NODE_HANLDER)(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode, void* pParam);
typedef void    (*PFN_DG_EDGE_HANLDER)(VSC_DIRECTED_GRAPH* pDG, VSC_DG_EDGE* pEdge, void* pParam);

#define SAFE_CALL_DG_NODE_HANDLER_RETURN(dgNodeHandler, pNode, pParam)   \
    if ((dgNodeHandler))                                                 \
    {                                                                    \
        if ((dgNodeHandler)(pDG, (pNode), (pParam)))                     \
        {                                                                \
            return;                                                      \
        }                                                                \
    }

#define SAFE_CALL_DG_NODE_HANDLER_CONTINUE(dgNodeHandler, pNode, pParam) \
    if ((dgNodeHandler))                                                 \
    {                                                                    \
        if ((dgNodeHandler)(pDG, (pNode), (pParam)))                     \
        {                                                                \
            continue;                                                    \
        }                                                                \
    }

#define SAFE_CALL_DG_NODE_HANDLER_CHECK(dgNodeHandler, pNode, pParam)    \
    ((dgNodeHandler) && ((dgNodeHandler)(pDG, (pNode), (pParam))))

#define SAFE_CALL_DG_NODE_HANDLER(dgNodeHandler, pNode, pParam)          \
    if ((dgNodeHandler))                                                 \
    {                                                                    \
        (dgNodeHandler)(pDG, (pNode), (pParam));                         \
    }

#define SAFE_CALL_DG_EDGE_HANDLER(dgEdgeHandler, pEdge, pParam)          \
    if ((dgEdgeHandler))                                                 \
    {                                                                    \
        (dgEdgeHandler)(pDG, (pEdge), (pParam));                         \
    }

#define DG_GET_ROOT_ARRAY_P(pDG)          (&((pDG)->rootNodeArray))
#define DG_GET_TAIL_ARRAY_P(pDG)          (&((pDG)->tailNodeArray))

/* Creation and destroy */
VSC_DIRECTED_GRAPH* vscDG_Create(VSC_MM* pMM, gctUINT rootInitAllocCount, gctUINT tailInitAllocCount, gctUINT edgeAllocSize);
void vscDG_Initialize(VSC_DIRECTED_GRAPH* pDG, VSC_MM* pMM, gctUINT rootInitAllocCount, gctUINT tailInitAllocCount, gctUINT edgeAllocSize);
void vscDG_Finalize(VSC_DIRECTED_GRAPH* pDG);
void vscDG_Destroy(VSC_DIRECTED_GRAPH* pDG);

/* Build graph */

void vscDG_AddNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode);
/* Return 2 VSC_DG_EDGEs which are successive, first one is successor edge, and 2nd is predecessor edge. */
VSC_DG_EDGE* vscDG_AddEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, gctBOOL* pIsNewEdge);
VSC_DG_EDGE* vscDG_GetEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode);
VSC_DG_EDGE* vscDG_ReplaceEdgeFromNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, VSC_DG_NODE* pNewFromNode);
VSC_DG_EDGE* vscDG_ReplaceEdgeToNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, VSC_DG_NODE* pNewToNode);
void vscDG_RemoveEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode);
void vscDG_RemoveNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode);

VSC_DG_NODE* vscDG_GetNodeById(VSC_DIRECTED_GRAPH* pDG, gctUINT nodeId);

gctUINT vscDG_GetNodeCount(VSC_DIRECTED_GRAPH* pDG);
gctUINT vscDG_GetHistNodeCount(VSC_DIRECTED_GRAPH* pDG);
gctUINT vscDG_GetHistEdgeCount(VSC_DIRECTED_GRAPH* pDG);
gctUINT vscDG_GetRootCount(VSC_DIRECTED_GRAPH* pDG);
gctUINT vscDG_GetTailCount(VSC_DIRECTED_GRAPH* pDG);

/* Traversal */

/* We support following traversal methods:
   1. Non-iterator without CB (returns expected traversal order)
        a. PVO + DFS   : implemented with recursive calls
        b. PVO + BFS_N : implemented with recursive calls
        c. PVO + BFS_W : implemented with queue, no recursive calls
        d. POO + DFS   : implemented with recursive calls
        e. POO + BFS_N : implemented with recursive calls
        f. POO + BFS_W : implemented with reverse of PVO + BFS_W, can we find a native solution????

   2. Non-iterator with CB
        Provide pre- and post- callbacks for own node and descendant nodes. In this method, no concept of pre-order
        and post-order is provided.

   3. Iterator
        a. PVO + DFS   : implemented with stack
        b. PVO + BFS_N : implemented with node array returned by couterpart function of non-iterator, can we find a native solution????
        c. PVO + BFS_W : implemented with queue
        d. POO + DFS   : implemented with stack
        e. POO + BFS_N : implemented with node array returned by couterpart function of non-iterator, can we find a native solution????
        f. POO + BFS_W : implemented with node array returned by reverse of couterpart function of non-iterator, can we find a native solution????

   All traversal can be started from leaves of graph.
*/

/* ppRetNodeOrder must be allocated with node count of graph which can be got from nodeList.info.count.
   This returned node order is what requested order */
void vscDG_PreOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                             VSC_GRAPH_SEARCH_MODE searchMode,
                             gctBOOL bFromTail,
                             gctBOOL bReverseResult,
                             VSC_DG_NODE** ppRetNodeOrder);
void vscDG_PstOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                             VSC_GRAPH_SEARCH_MODE searchMode,
                             gctBOOL bFromTail,
                             gctBOOL bReverseResult,
                             VSC_DG_NODE** ppRetNodeOrder);

/* This function won't return node order to user, instead, user can pass into 5 handlers to process
   node when node is accessing */
void vscDG_TraversalCB(VSC_DIRECTED_GRAPH* pDG,
                       VSC_GRAPH_SEARCH_MODE searchMode,
                       gctBOOL bFromTail,
                       PFN_DG_NODE_HANLDER pfnHandlerStarter,
                       PFN_DG_NODE_HANLDER pfnHandlerOwnPre,
                       PFN_DG_NODE_HANLDER pfnHandlerOwnPost,
                       PFN_DG_NODE_HANLDER pfnHandlerDescendantPre,
                       PFN_DG_NODE_HANLDER pfnHandlerDescendantPost,
                       PFN_DG_EDGE_HANLDER pfnHandlerDFSEdgeOnRevisit, /* Only for DFS */
                       void* pParam);

/* Analyze */


/* An iterator for directed-graph to traversal its nodes */
typedef struct _VSC_DG_ITERATOR
{
    VSC_DIRECTED_GRAPH*       pDG;
    VSC_GRAPH_SEARCH_MODE     searchMode;
    VSC_GRAPH_TRAVERSAL_ORDER traversalOrder;
    gctBOOL                   bFromTail;
    gctUINT                   curIndexOfRootTailArray;

    /* DFS will use stack, BFS_wide(prev) will use queue, and BFS_narrow and BFS_wide(post) will use
       node-order. Actually, if you want, you can use node-order for any search mode */
    union
    {
        VSC_SIMPLE_STACK      dgNodeStack;
        VSC_SIMPLE_QUEUE      dgNodeQueue;
        VSC_GNODE_ORDER       dgNodeOrder;
    }nodeTraversalStatus;
}VSC_DG_ITERATOR;

/* It will use pMM of pDG to allocate memory */
VSC_DG_ITERATOR* vscDG_ITERATOR_Create(VSC_DIRECTED_GRAPH* pDG,
                                       VSC_GRAPH_SEARCH_MODE searchMode,
                                       VSC_GRAPH_TRAVERSAL_ORDER traversalOrder,
                                       gctBOOL bFromTail);
void vscDG_ITERATOR_Initialize(VSC_DG_ITERATOR* pDGIterator,
                               VSC_DIRECTED_GRAPH* pDG,
                               VSC_GRAPH_SEARCH_MODE searchMode,
                               VSC_GRAPH_TRAVERSAL_ORDER traversalOrder,
                               gctBOOL bFromTail);
void vscDG_ITERATOR_Finalize(VSC_DG_ITERATOR* pDGIterator);
void vscDG_ITERATOR_Destory(VSC_DG_ITERATOR* pDGIterator);

/* Operations for DG iterator */
VSC_DG_NODE* vscDG_ITERATOR_Begin(VSC_DG_ITERATOR* pDGIterator);
VSC_DG_NODE* vscDG_ITERATOR_Next(VSC_DG_ITERATOR* pDGIterator);
void vscDG_ITERATOR_End(VSC_DG_ITERATOR* pDGIterator);

END_EXTERN_C()

#endif /* __gc_vsc_utils_graph_h_ */


