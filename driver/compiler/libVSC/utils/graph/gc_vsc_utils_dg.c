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

void vscDGEG_Initialize(VSC_DG_EDGE* pEdge, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode)
{
    vscULN_Initialize(&pEdge->uniLstNode);
    pEdge->pFromNode = pFromNode;
    pEdge->pToNode = pToNode;
}

void vscDGEG_Finalize(VSC_DG_EDGE* pEdge)
{
    vscULN_Finalize(&pEdge->uniLstNode);
}

void vscDGEGEXT_Initialize(VSC_DG_EDGE_EXT* pExtEdge, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, void* pUserData)
{
    vscBSNODE_Initialize(&pExtEdge->baseNode, pUserData);
    vscDGEG_Initialize(&pExtEdge->dgEdge, pFromNode, pToNode);
}

void vscDGEGEXT_Finalize(VSC_DG_EDGE_EXT* pExtEdge)
{
    vscDGEG_Finalize(&pExtEdge->dgEdge);
}

void* vscDGEGEXT_GetContainedUserData(VSC_DG_EDGE_EXT* pExtEdge)
{
    return vscBSNODE_GetContainedUserData(&pExtEdge->baseNode);
}

void vscDGND_Initialize(VSC_DG_NODE* pNode)
{
    vscBLN_Initialize(&pNode->biListNode);

    pNode->id = INVALID_GNODE_ID;
    AJLST_INITIALIZE(&pNode->succList);
    AJLST_INITIALIZE(&pNode->predList);
    pNode->bVisited = gcvFALSE;
}

void vscDGND_Finalize(VSC_DG_NODE* pNode)
{
    vscBLN_Finalize(&pNode->biListNode);

    AJLST_FINALIZE(&pNode->succList);
    AJLST_FINALIZE(&pNode->predList);
    pNode->id = INVALID_GNODE_ID;
    pNode->bVisited = gcvFALSE;
}

void vscDGNDAJNIterator_Init(VSC_ADJACENT_NODE_ITERATOR* pIter, VSC_DG_NODE* pNode, gctBOOL bPred)
{
    VSC_ADJACENT_LIST_ITERATOR_INIT(pIter, bPred ? &pNode->predList : &pNode->succList);
}

VSC_DG_NODE* vscDGNDAJNIterator_First(VSC_ADJACENT_NODE_ITERATOR* pIter)
{
    VSC_DG_EDGE* pEdge = (VSC_DG_EDGE*)VSC_ADJACENT_LIST_ITERATOR_FIRST(pIter);
    return pEdge ? pEdge->pToNode : gcvNULL;
}

VSC_DG_NODE* vscDGNDAJNIterator_Next(VSC_ADJACENT_NODE_ITERATOR* pIter)
{
    VSC_DG_EDGE* pEdge = (VSC_DG_EDGE*)VSC_ADJACENT_LIST_ITERATOR_NEXT(pIter);
    return pEdge ? pEdge->pToNode : gcvNULL;
}

VSC_DG_NODE* vscDGNDAJNIterator_Last(VSC_ADJACENT_NODE_ITERATOR* pIter)
{
    VSC_DG_EDGE* pEdge = (VSC_DG_EDGE*)VSC_ADJACENT_LIST_ITERATOR_LAST(pIter);
    return pEdge ? pEdge->pToNode : gcvNULL;
}

void vscDGNDEXT_Initialize(VSC_DG_NODE_EXT* pExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pExtNode->baseNode, pUserData);
    vscDGND_Initialize(&pExtNode->dgNode);
}

void vscDGNDEXT_Finalize(VSC_DG_NODE_EXT* pExtNode)
{
    vscDGND_Finalize(&pExtNode->dgNode);
}

void* vscDGNDEXT_GetContainedUserData(VSC_DG_NODE_EXT* pExtNode)
{
    return vscBSNODE_GetContainedUserData(&pExtNode->baseNode);
}

gctBOOL DG_NODE_CMP(void* pNode1, void* pNode2)
{
    VSC_DG_NODE** ppNode1 = (VSC_DG_NODE**)pNode1;
    VSC_DG_NODE** ppNode2 = (VSC_DG_NODE**)pNode2;

    return (*ppNode1 == *ppNode2);
}

VSC_DIRECTED_GRAPH* vscDG_Create(VSC_MM* pMM, gctUINT rootInitAllocCount, gctUINT tailInitAllocCount, gctUINT edgeAllocSize)
{
    VSC_DIRECTED_GRAPH*   pDG = gcvNULL;

    pDG = (VSC_DIRECTED_GRAPH*)vscMM_Alloc(pMM, sizeof(VSC_DIRECTED_GRAPH));
    vscDG_Initialize(pDG, pMM, rootInitAllocCount, tailInitAllocCount, edgeAllocSize);

    return pDG;
}

gctUINT _HFUNC_PassThroughNodeId(const void* pKey)
{
    gctUINT nodeId = (gctUINT)(gctUINTPTR_T)pKey;

    /* Just pass through node id */
    return nodeId;
}

void vscDG_Initialize(VSC_DIRECTED_GRAPH* pDG, VSC_MM* pMM, gctUINT rootInitAllocCount, gctUINT tailInitAllocCount, gctUINT edgeAllocSize)
{
    pDG->pMM = pMM;
    pDG->nextNodeId = 0;
    pDG->nextEdgeId = 0;
    pDG->edgeAllocSize = edgeAllocSize;
    DGNLST_INITIALIZE(&pDG->nodeList);
    vscSRARR_Initialize(&pDG->rootNodeArray, pMM, rootInitAllocCount, sizeof(VSC_DG_NODE*), DG_NODE_CMP);
    vscSRARR_Initialize(&pDG->tailNodeArray, pMM, tailInitAllocCount, sizeof(VSC_DG_NODE*), DG_NODE_CMP);
    vscHTBL_Initialize(&pDG->ndHashTable, pMM, _HFUNC_PassThroughNodeId, gcvNULL, GNODE_HASH_TABLE_SIZE);
}

void vscDG_Finalize(VSC_DIRECTED_GRAPH* pDG)
{
    DGNLST_FINALIZE(&pDG->nodeList);
    vscSRARR_Finalize(&pDG->rootNodeArray);
    vscSRARR_Finalize(&pDG->tailNodeArray);
    vscHTBL_Finalize(&pDG->ndHashTable);
}

void vscDG_Destroy(VSC_DIRECTED_GRAPH* pDG)
{
    if (pDG)
    {
        vscDG_Finalize(pDG);

        /* Free dg itself */
        vscMM_Free(pDG->pMM, pDG);
        pDG = gcvNULL;
    }
}

static void _UpdateRootArray(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode)
{
    gcmASSERT(pNode->id != INVALID_GNODE_ID);

    /* Root array */
    if (DGND_GET_IN_DEGREE(pNode) == 0)
    {
        if (vscSRARR_GetElementIndexByContent(&pDG->rootNodeArray, (void*)&pNode) ==
            VSC_INVALID_ARRAY_INDEX)
        {
            vscSRARR_AddElement(&pDG->rootNodeArray, (void*)&pNode);
        }
    }
    else
    {
        vscSRARR_RemoveElementByContent(&pDG->rootNodeArray, (void*)&pNode);
    }
}

static void _UpdateTailArray(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode)
{
    gcmASSERT(pNode->id != INVALID_GNODE_ID);

    /* Tail array */
    if (DGND_GET_OUT_DEGREE(pNode) == 0)
    {
        if (vscSRARR_GetElementIndexByContent(&pDG->tailNodeArray, (void*)&pNode) ==
            VSC_INVALID_ARRAY_INDEX)
        {
            vscSRARR_AddElement(&pDG->tailNodeArray, (void*)&pNode);
        }
    }
    else
    {
        vscSRARR_RemoveElementByContent(&pDG->tailNodeArray, (void*)&pNode);
    }
}

static void _UpdateRootTailArray(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode)
{
    _UpdateRootArray(pDG, pNode);
    _UpdateTailArray(pDG, pNode);
}

void vscDG_AddNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode)
{
    /* A node can not be added into graph twice */
    gcmASSERT(pNode->id == INVALID_GNODE_ID);

    DGNLST_ADD_NODE(&pDG->nodeList, pNode);
    pNode->id = pDG->nextNodeId ++;

    vscHTBL_DirectSet(&pDG->ndHashTable, (void*)(gctUINTPTR_T)pNode->id, pNode);

    /* Since this is fresh node, so it must be as both root and tail. */
    _UpdateRootTailArray(pDG, pNode);
}

VSC_DG_EDGE* vscDG_GetEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode)
{
    VSC_DG_EDGE* pSuccEdge;

    /* An edge must have header and tail */
    if (pFromNode == gcvNULL || pToNode == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return gcvNULL;
    }

    /* Check whether from-node and to-node are in graph */
    if (pFromNode->id == INVALID_GNODE_ID || pToNode->id == INVALID_GNODE_ID)
    {
        return gcvNULL;
    }

    /* Check whether there is an edge linking these two nodes, if yes, just return this edge */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pFromNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = DGEG_GET_NEXT_EDGE(pSuccEdge))
    {
        if (pSuccEdge->pFromNode == pFromNode && pSuccEdge->pToNode == pToNode)
        {
            return pSuccEdge;
        }
    }

    return gcvNULL;
}

VSC_DG_EDGE* vscDG_AddEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, gctBOOL* pIsNewEdge)
{
    VSC_DG_EDGE* pEdges;
    VSC_DG_EDGE* pSuccEdge;
    VSC_DG_EDGE* pPredEdge;

    if (pIsNewEdge)
    {
        *pIsNewEdge = gcvTRUE;
    }

    /* An edge must have header and tail */
    if (pFromNode == gcvNULL || pToNode == gcvNULL)
    {
        gcmASSERT(gcvFALSE);
        return gcvNULL;
    }

    /* Check whether from-node and to-node are in graph, if not, add them */
    if (pFromNode->id == INVALID_GNODE_ID)
    {
        vscDG_AddNode(pDG, pFromNode);
    }

    if (pToNode->id == INVALID_GNODE_ID)
    {
        vscDG_AddNode(pDG, pToNode);
    }

    /* If edge is already existed, just return this edge */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pFromNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = DGEG_GET_NEXT_EDGE(pSuccEdge))
    {
        if (pSuccEdge->pFromNode == pFromNode && pSuccEdge->pToNode == pToNode)
        {
            if (pIsNewEdge)
            {
                *pIsNewEdge = gcvFALSE;
            }

            return pSuccEdge;
        }
    }

    /* Alloc edges and initialize */
    pEdges = (VSC_DG_EDGE*)vscMM_Alloc(pDG->pMM, 2*pDG->edgeAllocSize);
    pSuccEdge = pEdges;
    pPredEdge = (VSC_DG_EDGE*)((gctUINT8*)pEdges + pDG->edgeAllocSize);
    vscDGEG_Initialize(pSuccEdge, pFromNode, pToNode);
    pSuccEdge->id = pDG->nextEdgeId;
    vscDGEG_Initialize(pPredEdge, pToNode, pFromNode);
    pPredEdge->id = pDG->nextEdgeId;
    pDG->nextEdgeId ++;

    /* Now add adjacent list */
    AJLST_ADD_EDGE(&pFromNode->succList, pSuccEdge);
    AJLST_ADD_EDGE(&pToNode->predList, pPredEdge);

    /* Lastly, update root and tail array */
    _UpdateTailArray(pDG, pFromNode);
    _UpdateRootArray(pDG, pToNode);

    return pEdges;
}

VSC_DG_EDGE* vscDG_ReplaceEdgeFromNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, VSC_DG_NODE* pNewFromNode)
{
    VSC_DG_EDGE* pSuccEdge = gcvNULL;
    VSC_DG_EDGE* pPredEdge = gcvNULL;

    gcmASSERT(pFromNode->id != INVALID_GNODE_ID);
    gcmASSERT(pToNode->id != INVALID_GNODE_ID);

    /* Find successor edge and then remove */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pFromNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = DGEG_GET_NEXT_EDGE(pSuccEdge))
    {
        if (pSuccEdge->pFromNode == pFromNode && pSuccEdge->pToNode == pToNode)
        {
            AJLST_REMOVE_EDGE(&pFromNode->succList, pSuccEdge);
            break;
        }
    }
    gcmASSERT(pSuccEdge);

    /* Find predecessor edge and remove */
    for (pPredEdge = AJLST_GET_FIRST_EDGE(&pToNode->predList);
         pPredEdge != NULL;
         pPredEdge = DGEG_GET_NEXT_EDGE(pPredEdge))
    {
        if (pPredEdge->pToNode == pFromNode && pPredEdge->pFromNode == pToNode)
        {
            break;
        }
    }
    gcmASSERT(pPredEdge);

    /* Free edges, note that pSuccEdge and pPredEdge are successive and pSuccEdge points whole entity */
    pSuccEdge->pFromNode = pNewFromNode;
    pPredEdge->pToNode = pNewFromNode;
    AJLST_ADD_EDGE(&pNewFromNode->succList, pSuccEdge);

    /* Lastly, update root and tail array */
    _UpdateTailArray(pDG, pFromNode);
    _UpdateTailArray(pDG, pNewFromNode);

    return pSuccEdge;
}

VSC_DG_EDGE* vscDG_ReplaceEdgeToNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode, VSC_DG_NODE* pNewToNode)
{
    VSC_DG_EDGE* pSuccEdge = gcvNULL;
    VSC_DG_EDGE* pPredEdge = gcvNULL;

    gcmASSERT(pFromNode->id != INVALID_GNODE_ID);
    gcmASSERT(pToNode->id != INVALID_GNODE_ID);

    /* Find successor edge and then remove */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pFromNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = DGEG_GET_NEXT_EDGE(pSuccEdge))
    {
        if (pSuccEdge->pFromNode == pFromNode && pSuccEdge->pToNode == pToNode)
        {
            break;
        }
    }
    gcmASSERT(pSuccEdge);

    /* Find predecessor edge and remove */
    for (pPredEdge = AJLST_GET_FIRST_EDGE(&pToNode->predList);
         pPredEdge != NULL;
         pPredEdge = DGEG_GET_NEXT_EDGE(pPredEdge))
    {
        if (pPredEdge->pToNode == pFromNode && pPredEdge->pFromNode == pToNode)
        {
            AJLST_REMOVE_EDGE(&pToNode->predList, pPredEdge);
            break;
        }
    }
    gcmASSERT(pPredEdge);

    /* Free edges, note that pSuccEdge and pPredEdge are successive and pSuccEdge points whole entity */
    pSuccEdge->pToNode = pNewToNode;
    pPredEdge->pFromNode = pNewToNode;
    AJLST_ADD_EDGE(&pNewToNode->predList, pPredEdge);

    /* Lastly, update root and tail array */
    _UpdateRootArray(pDG, pToNode);
    _UpdateRootArray(pDG, pNewToNode);

    return pSuccEdge;
}

void vscDG_RemoveEdge(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pFromNode, VSC_DG_NODE* pToNode)
{
    VSC_DG_EDGE* pSuccEdge = gcvNULL;
    VSC_DG_EDGE* pPredEdge = gcvNULL;

    gcmASSERT(pFromNode->id != INVALID_GNODE_ID);
    gcmASSERT(pToNode->id != INVALID_GNODE_ID);

    /* Find successor edge and then remove */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pFromNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = DGEG_GET_NEXT_EDGE(pSuccEdge))
    {
        if (pSuccEdge->pFromNode == pFromNode && pSuccEdge->pToNode == pToNode)
        {
            AJLST_REMOVE_EDGE(&pFromNode->succList, pSuccEdge);
            break;
        }
    }
    gcmASSERT(pSuccEdge);

    /* Find predecessor edge and remove */
    for (pPredEdge = AJLST_GET_FIRST_EDGE(&pToNode->predList);
         pPredEdge != NULL;
         pPredEdge = DGEG_GET_NEXT_EDGE(pPredEdge))
    {
        if (pPredEdge->pToNode == pFromNode && pPredEdge->pFromNode == pToNode)
        {
            AJLST_REMOVE_EDGE(&pToNode->predList, pPredEdge);
            break;
        }
    }
    gcmASSERT(pPredEdge);

    /* Free edges, note that pSuccEdge and pPredEdge are successive and pSuccEdge points whole entity */
    vscDGEG_Finalize(pSuccEdge);
    vscDGEG_Finalize(pPredEdge);
    gcmASSERT((gctUINT8*)pSuccEdge + pDG->edgeAllocSize == (gctUINT8*)pPredEdge);
    vscMM_Free(pDG->pMM, pSuccEdge);

    /* Lastly, update root and tail array */
    _UpdateTailArray(pDG, pFromNode);
    _UpdateRootArray(pDG, pToNode);
}

void vscDG_RemoveNode(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE* pNode)
{
    VSC_DG_EDGE* pSuccEdge = gcvNULL;
    VSC_DG_EDGE* pPredEdge = gcvNULL;

    gcmASSERT(pNode->id != INVALID_GNODE_ID);

    /* Remove all successor edges */
    for (pSuccEdge = AJLST_GET_FIRST_EDGE(&pNode->succList);
         pSuccEdge != NULL;
         pSuccEdge = AJLST_GET_FIRST_EDGE(&pNode->succList))
    {
        vscDG_RemoveEdge(pDG, pSuccEdge->pFromNode, pSuccEdge->pToNode);
    }

    /* Remove all predecessor edges */
    for (pPredEdge = AJLST_GET_FIRST_EDGE(&pNode->predList);
         pPredEdge != NULL;
         pPredEdge = AJLST_GET_FIRST_EDGE(&pNode->predList))
    {
        vscDG_RemoveEdge(pDG, pPredEdge->pToNode, pPredEdge->pFromNode);
    }

    /* Remove it from root and tail array */
    vscSRARR_RemoveElementByContent(&pDG->rootNodeArray, (void*)&pNode);
    vscSRARR_RemoveElementByContent(&pDG->tailNodeArray, (void*)&pNode);

    /* Now remove node from nodeList */
    DGNLST_REMOVE_NODE(&pDG->nodeList, pNode);

    vscHTBL_DirectRemove(&pDG->ndHashTable, (void*)(gctUINTPTR_T)pNode->id);

    /* If graph has been empty, we can reset id pool */
    if (DGNLST_GET_NODE_COUNT(&pDG->nodeList) == 0)
    {
        pDG->nextNodeId = 0;
    }
}

VSC_DG_NODE* vscDG_GetNodeById(VSC_DIRECTED_GRAPH* pDG, gctUINT nodeId)
{
    return (VSC_DG_NODE*)vscHTBL_DirectGet(&pDG->ndHashTable, (void*)(gctUINTPTR_T)nodeId);
}

gctUINT vscDG_GetNodeCount(VSC_DIRECTED_GRAPH* pDG)
{
    return vscBILST_GetNodeCount(&pDG->nodeList);
}

gctUINT vscDG_GetHistNodeCount(VSC_DIRECTED_GRAPH* pDG)
{
    return pDG->nextNodeId;
}

gctUINT vscDG_GetHistEdgeCount(VSC_DIRECTED_GRAPH* pDG)
{
    return pDG->nextEdgeId;
}

gctUINT vscDG_GetRootCount(VSC_DIRECTED_GRAPH* pDG)
{
    return vscSRARR_GetElementCount(&pDG->rootNodeArray);
}

gctUINT vscDG_GetTailCount(VSC_DIRECTED_GRAPH* pDG)
{
    return vscSRARR_GetElementCount(&pDG->tailNodeArray);
}

/* Node stack with edge. */
typedef struct _VSC_DG_NODE_WITH_EDGE
{
    VSC_DG_NODE*    pNode;
    VSC_DG_EDGE*    pEdge;
    gctUINT         nextEdgeIndex;
} VSC_DG_NODE_WITH_EDGE;

static void _PushStackWithEdge(VSC_SIMPLE_STACK* pStack, VSC_DG_NODE* pNode, VSC_DG_EDGE* pEdge, gctUINT nextEdgeIndex, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY* pStackEntry;
    VSC_DG_NODE_WITH_EDGE*  pNodeWithEdge;

    /* Allocate the node with edge. */
    pNodeWithEdge = (VSC_DG_NODE_WITH_EDGE*)vscMM_Alloc(pMM, sizeof(VSC_DG_NODE_WITH_EDGE));
    pNodeWithEdge->pNode = pNode;
    pNodeWithEdge->pEdge = pEdge;
    pNodeWithEdge->nextEdgeIndex = nextEdgeIndex;

    /* Allocate the stack entry. */
    pStackEntry = (VSC_QUEUE_STACK_ENTRY*)vscMM_Alloc(pMM, sizeof(VSC_QUEUE_STACK_ENTRY));

    /* Push the entry. */
    SQE_INITIALIZE(pStackEntry, pNodeWithEdge);
    STACK_PUSH_ENTRY(pStack, pStackEntry);
}

static void* _PopStackWithEdge(VSC_SIMPLE_STACK* pStack, VSC_DG_NODE_WITH_EDGE* pNodeWithEdge, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY*       pStackEntry;
    VSC_DG_NODE_WITH_EDGE*       pRetNode;

    pStackEntry = STACK_POP_ENTRY(pStack);
    pRetNode = (VSC_DG_NODE_WITH_EDGE*)SQE_GET_CONTENT(pStackEntry);

    if (pNodeWithEdge)
    {
        pNodeWithEdge->pNode = pRetNode->pNode;
        pNodeWithEdge->pEdge = pRetNode->pEdge;
        pNodeWithEdge->nextEdgeIndex = pRetNode->nextEdgeIndex;
    }

    vscMM_Free(pMM, pRetNode);
    vscMM_Free(pMM, pStackEntry);

    return pRetNode;
}

static VSC_DG_NODE_WITH_EDGE* _TopStackWithEdge(VSC_SIMPLE_STACK* pStack)
{
    VSC_QUEUE_STACK_ENTRY*       pStackEntry;
    VSC_DG_NODE_WITH_EDGE*       pRetNode;

    pStackEntry = STACK_PEEK_TOP_ENTRY(pStack);
    pRetNode = (VSC_DG_NODE_WITH_EDGE*)SQE_GET_CONTENT(pStackEntry);

    return pRetNode;
}

/* Node stack for DG_NODE. */
static void _PushStack(VSC_SIMPLE_STACK* pStack, VSC_DG_NODE* pNode, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY* pStackEntry;

    pStackEntry = (VSC_QUEUE_STACK_ENTRY*)vscMM_Alloc(pMM, sizeof(VSC_QUEUE_STACK_ENTRY));
    SQE_INITIALIZE(pStackEntry, pNode);
    STACK_PUSH_ENTRY(pStack, pStackEntry);
}

static VSC_DG_NODE* _PopStack(VSC_SIMPLE_STACK* pStack, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY*       pStackEntry;
    VSC_DG_NODE*                 pRetNode;

    pStackEntry = STACK_POP_ENTRY(pStack);
    pRetNode = (VSC_DG_NODE*)SQE_GET_CONTENT(pStackEntry);
    vscMM_Free(pMM, pStackEntry);

    return pRetNode;
}

static void _EnQueue(VSC_SIMPLE_QUEUE* pQueue, VSC_DG_NODE* pNode, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY* pQueueEntry;

    pQueueEntry = (VSC_QUEUE_STACK_ENTRY*)vscMM_Alloc(pMM, sizeof(VSC_QUEUE_STACK_ENTRY));
    SQE_INITIALIZE(pQueueEntry, pNode);
    QUEUE_PUT_ENTRY(pQueue, pQueueEntry);
}

static VSC_DG_NODE* _DeQueue(VSC_SIMPLE_QUEUE* pQueue, VSC_MM* pMM)
{
    VSC_QUEUE_STACK_ENTRY*       pQueueEntry;
    VSC_DG_NODE*                 pRetNode;

    pQueueEntry = QUEUE_GET_ENTRY(pQueue);
    pRetNode = (VSC_DG_NODE*)SQE_GET_CONTENT(pQueueEntry);
    vscMM_Free(pMM, pQueueEntry);

    return pRetNode;
}

static VSC_SIMPLE_RESIZABLE_ARRAY* _PrepareTraversal(VSC_DIRECTED_GRAPH* pDG,
                                                     VSC_GRAPH_SEARCH_MODE searchMode, gctBOOL bFromTail)
{
    VSC_DG_NODE* pNode = gcvNULL;

    for (pNode = DGNLST_GET_FIRST_NODE(&pDG->nodeList);
         pNode != NULL;
         pNode = DGND_GET_NEXT_NODE(pNode))
    {
        pNode->bVisited = gcvFALSE;
    }

    /* Determine from which array we start */
    return (bFromTail) ? &pDG->tailNodeArray : &pDG->rootNodeArray;
}

static void _ReverseResult(VSC_DIRECTED_GRAPH* pDG, VSC_DG_NODE** ppRetNodeOrder)
{
    gctUINT      i, nodeCount = DGNLST_GET_NODE_COUNT(&pDG->nodeList);
    VSC_DG_NODE* pTmpNode;

    for (i = 0; i < (nodeCount/2); i ++)
    {
        pTmpNode = *(ppRetNodeOrder + i);
        *(ppRetNodeOrder + i) = *(ppRetNodeOrder + nodeCount - i - 1);
        *(ppRetNodeOrder + nodeCount - i - 1) = pTmpNode;
    }
}

static gctBOOL _UseRecursiveTraversalForDFS(VSC_DIRECTED_GRAPH* pDG,
                                            VSC_GRAPH_SEARCH_MODE searchMode)
{
    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST)
    {
        if (pDG->nextEdgeId > MAX_EDGE_COUNT_TO_USE_RECURSION_FOR_DFS)
        {
            return gcvFALSE;
        }
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE)
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

static VSC_GRAPH_SEARCH_MODE _ChooseImplementSearchMode(VSC_DIRECTED_GRAPH* pDG,
                                                        VSC_GRAPH_SEARCH_MODE searchMode)
{
    /* We have the resursive/iterative implementations for DFS. */
    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST             ||
        searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE   ||
        searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE)
    {
        if (_UseRecursiveTraversalForDFS(pDG, searchMode))
        {
            return VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE;
        }
        else
        {
            return VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE;
        }
    }

    return searchMode;
}

static void _DoPreOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                                 VSC_DG_NODE* pNode,
                                 VSC_GRAPH_SEARCH_MODE searchMode,
                                 gctBOOL bFromTail,
                                 VSC_DG_NODE** ppRetNodeOrder,
                                 gctUINT *pPreOrderIdx)
{
    VSC_DG_EDGE*       pEdge = gcvNULL;
    VSC_ADJACENT_LIST* pAdjList;

    /* Traversal descendants based on search mode now */
    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE)
    {
        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        pNode->bVisited = gcvTRUE;

        /* Record pre-order seq */
        ppRetNodeOrder[(*pPreOrderIdx) ++] = pNode;

        /* Visit descendants before siblings */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                _DoPreOrderTraversal(pDG, pEdge->pToNode, searchMode, bFromTail, ppRetNodeOrder, pPreOrderIdx);
            }
        }
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE)
    {
        VSC_SIMPLE_STACK        stack;
        VSC_DG_NODE*            pPopNode;

        STACK_INITIALIZE(&stack);

        _PushStack(&stack, pNode, pDG->pMM);

        while (!STACK_CHECK_EMPTY(&stack))
        {
            pPopNode = _PopStack(&stack, pDG->pMM);
            if (pPopNode->bVisited)
            {
                continue;
            }
            pPopNode->bVisited = gcvTRUE;

            /* Record pre-order seq */
            ppRetNodeOrder[(*pPreOrderIdx) ++] = pPopNode;

            /* Determine direction */
            pAdjList = (bFromTail) ? &pPopNode->predList : &pPopNode->succList;

            vscUNILST_Reverse(pAdjList);

            /* Visit descendants before siblings */
            for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
                 pEdge != NULL;
                 pEdge = DGEG_GET_NEXT_EDGE(pEdge))
            {
                gcmASSERT(pEdge->pFromNode == pPopNode);

                if (!pEdge->pToNode->bVisited)
                {
                    _PushStack(&stack, pEdge->pToNode, pDG->pMM);
                }
            }

            /* Reverse the list again. */
            vscUNILST_Reverse(pAdjList);
        }

        STACK_FINALIZE(&stack);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY unvisitedDescendantArray;
        gctUINT                    i;
        VSC_DG_NODE*               pTmpNode;

        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        vscSRARR_Initialize(&unvisitedDescendantArray, pDG->pMM, 16, sizeof(VSC_DG_NODE*), DG_NODE_CMP);

        /* Visit siblings before descendants */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                pEdge->pToNode->bVisited = gcvTRUE;

                /* Record pre-order seq */
                ppRetNodeOrder[(*pPreOrderIdx) ++] = pEdge->pToNode;

                vscSRARR_AddElement(&unvisitedDescendantArray, pEdge->pToNode);
            }
        }

        for (i = 0; i < vscSRARR_GetElementCount(&unvisitedDescendantArray); i ++)
        {
            pTmpNode = *(VSC_DG_NODE**)vscSRARR_GetElement(&unvisitedDescendantArray, i);
            _DoPreOrderTraversal(pDG, pTmpNode, searchMode, bFromTail, ppRetNodeOrder, pPreOrderIdx);
        }

        vscSRARR_Finalize(&unvisitedDescendantArray);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE)
    {
        VSC_SIMPLE_QUEUE  queue;
        VSC_DG_NODE*      pDequeuedNode;

        QUEUE_INITIALIZE(&queue);

        pNode->bVisited = gcvTRUE;

        _EnQueue(&queue, pNode, pDG->pMM);
        while (!QUEUE_CHECK_EMPTY(&queue))
        {
            pDequeuedNode = _DeQueue(&queue, pDG->pMM);

            /* Record pre-order seq */
            ppRetNodeOrder[(*pPreOrderIdx) ++] = pDequeuedNode;

            /* Determine direction */
            pAdjList = (bFromTail) ? &pDequeuedNode->predList : &pDequeuedNode->succList;

            /* Visit siblings before descendants */
            for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
                 pEdge != NULL;
                 pEdge = DGEG_GET_NEXT_EDGE(pEdge))
            {
                gcmASSERT(pEdge->pFromNode == pDequeuedNode);

                if (!pEdge->pToNode->bVisited)
                {
                    pEdge->pToNode->bVisited = gcvTRUE;

                    _EnQueue(&queue, pEdge->pToNode, pDG->pMM);
                }
            }
        }

        QUEUE_FINALIZE(&queue);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

void vscDG_PreOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                             VSC_GRAPH_SEARCH_MODE searchMode,
                             gctBOOL bFromTail,
                             gctBOOL bReverseResult,
                             VSC_DG_NODE** ppRetNodeOrder)
{
    gctUINT                     i, preOrderIdx = 0;
    VSC_DG_NODE*                pStartNode;
    VSC_SIMPLE_RESIZABLE_ARRAY* pStartNodeArray;

    searchMode = _ChooseImplementSearchMode(pDG, searchMode);

    /* Prepare firstly */
    pStartNodeArray = _PrepareTraversal(pDG, searchMode, bFromTail);

    /* Start from root or tail list to traversal */
    for (i = 0; i < vscSRARR_GetElementCount(pStartNodeArray); i ++)
    {
        pStartNode = *(VSC_DG_NODE**)vscSRARR_GetElement(pStartNodeArray, i);

        if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
        {
            pStartNode->bVisited = gcvTRUE;
            ppRetNodeOrder[preOrderIdx ++] = pStartNode;
        }

        /* Start iterately traversal */
        _DoPreOrderTraversal(pDG, pStartNode, searchMode, bFromTail, ppRetNodeOrder, &preOrderIdx);
    }

    /* Reverse result if required */
    if (bReverseResult)
    {
        _ReverseResult(pDG, ppRetNodeOrder);
    }
}

static VSC_DG_EDGE* _GetEdgeByIndex(VSC_DIRECTED_GRAPH* pDG,
                                    VSC_DG_NODE* pNode,
                                    VSC_ADJACENT_LIST* pAdjList,
                                    gctUINT edgeIndex)
{
    VSC_DG_EDGE*       pEdge = gcvNULL;
    gctUINT            i;

    /* Visit descendants before siblings */
    for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList), i = 0;
         pEdge != NULL;
         pEdge = DGEG_GET_NEXT_EDGE(pEdge), i++)
    {
        gcmASSERT(pEdge->pFromNode == pNode);

        if (i == edgeIndex)
        {
            return pEdge;
        }
    }

    return gcvNULL;
}

static void _DoPostOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                                  VSC_DG_NODE* pNode,
                                  VSC_GRAPH_SEARCH_MODE searchMode,
                                  gctBOOL bFromTail,
                                  VSC_DG_NODE** ppRetNodeOrder,
                                  gctUINT *pPostOrderIdx)
{
    VSC_DG_EDGE*       pEdge = gcvNULL;
    VSC_ADJACENT_LIST* pAdjList;

    /* Traversal descendants based on search mode now */
    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE)
    {
        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        pNode->bVisited = gcvTRUE;

        /* Visit descendants before siblings */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                _DoPostOrderTraversal(pDG, pEdge->pToNode, searchMode, bFromTail, ppRetNodeOrder, pPostOrderIdx);
            }
        }

        /* Record post-order seq */
        ppRetNodeOrder[(*pPostOrderIdx) ++] = pNode;
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE)
    {
        VSC_SIMPLE_STACK        stack;

        STACK_INITIALIZE(&stack);

        _PushStackWithEdge(&stack, pNode, gcvNULL, 0, pDG->pMM);

        while (!STACK_CHECK_EMPTY(&stack))
        {
            VSC_DG_NODE_WITH_EDGE*  pPopNodeWithEdge = _TopStackWithEdge(&stack);
            VSC_DG_NODE*            pPopNode = pPopNodeWithEdge->pNode;
            gctUINT                 nextEdgeIndex = pPopNodeWithEdge->nextEdgeIndex;
            gctBOOL                 bPopNode = gcvFALSE;

            pPopNode->bVisited = gcvTRUE;

            /* Determine direction */
            pAdjList = (bFromTail) ? &pPopNode->predList : &pPopNode->succList;

            /* Get the next visit edge. */
            while (gcvTRUE)
            {
                pEdge = _GetEdgeByIndex(pDG, pPopNode, pAdjList, nextEdgeIndex);
                nextEdgeIndex++;

                /* All edges have been visited, we can pop this node. */
                if (pEdge == gcvNULL)
                {
                    bPopNode = gcvTRUE;
                    break;
                }
                /* Revisit, find next one. */
                else if (pEdge->pToNode->bVisited == gcvTRUE)
                {
                }
                else
                {
                    _PushStackWithEdge(&stack, pEdge->pToNode, pEdge, 0, pDG->pMM);
                    break;
                }
            };

            if (bPopNode)
            {
                _PopStackWithEdge(&stack, gcvNULL, pDG->pMM);

                /* Record post-order seq */
                ppRetNodeOrder[(*pPostOrderIdx) ++] = pPopNode;
            }
            else
            {
                pPopNodeWithEdge->nextEdgeIndex = nextEdgeIndex;
            }
        }

        STACK_FINALIZE(&stack);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY unvisitedDescendantArray;
        gctUINT                    i;
        VSC_DG_NODE*               pTmpNode;

        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        vscSRARR_Initialize(&unvisitedDescendantArray, pDG->pMM, 16, sizeof(VSC_DG_NODE*), DG_NODE_CMP);

        /* Visit siblings before descendants */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                pEdge->pToNode->bVisited = gcvTRUE;

                vscSRARR_AddElement(&unvisitedDescendantArray, pEdge->pToNode);
            }
        }

        for (i = 0; i < vscSRARR_GetElementCount(&unvisitedDescendantArray); i ++)
        {
            pTmpNode = *(VSC_DG_NODE**)vscSRARR_GetElement(&unvisitedDescendantArray, i);
            _DoPostOrderTraversal(pDG, pTmpNode, searchMode, bFromTail, ppRetNodeOrder, pPostOrderIdx);
        }

        vscSRARR_Finalize(&unvisitedDescendantArray);

        /* Record post-order seq */
        ppRetNodeOrder[(*pPostOrderIdx) ++] = pNode;
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

void vscDG_PstOrderTraversal(VSC_DIRECTED_GRAPH* pDG,
                             VSC_GRAPH_SEARCH_MODE searchMode,
                             gctBOOL bFromTail,
                             gctBOOL bReverseResult,
                             VSC_DG_NODE** ppRetNodeOrder)
{
    gctUINT                     i, postOrderIdx = 0;
    VSC_DG_NODE*                pStartNode;
    VSC_SIMPLE_RESIZABLE_ARRAY* pStartNodeArray;

    searchMode = _ChooseImplementSearchMode(pDG, searchMode);

    /* For post order with BFS_wide, we just do reversed preorder with BFS_wide */
    if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE)
    {
        vscDG_PreOrderTraversal(pDG, VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE, bFromTail, !bReverseResult, ppRetNodeOrder);
        return;
    }

    /* Prepare firstly */
    pStartNodeArray = _PrepareTraversal(pDG, searchMode, bFromTail);

    /* Start from root or tail list to traversal */
    for (i = 0; i < vscSRARR_GetElementCount(pStartNodeArray); i ++)
    {
        pStartNode = *(VSC_DG_NODE**)vscSRARR_GetElement(pStartNodeArray, i);

        if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
        {
            pStartNode->bVisited = gcvTRUE;
        }

        /* Start iterately traversal */
        _DoPostOrderTraversal(pDG, pStartNode, searchMode, bFromTail, ppRetNodeOrder, &postOrderIdx);

        if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
        {
            ppRetNodeOrder[postOrderIdx ++] = pStartNode;
        }
    }

    /* Reverse result if required */
    if (bReverseResult)
    {
        _ReverseResult(pDG, ppRetNodeOrder);
    }
}

static void _DoTraversalCB(VSC_DIRECTED_GRAPH* pDG,
                           VSC_DG_NODE* pNode,
                           VSC_GRAPH_SEARCH_MODE searchMode,
                           gctBOOL bFromTail,
                           PFN_DG_NODE_HANLDER pfnHandlerOwnPre,
                           PFN_DG_NODE_HANLDER pfnHandlerOwnPost,
                           PFN_DG_NODE_HANLDER pfnHandlerDescendantPre,
                           PFN_DG_NODE_HANLDER pfnHandlerDescendantPost,
                           PFN_DG_EDGE_HANLDER pfnHandlerDFSEdgeOnRevisit, /* Only for DFS */
                           void* pParam)
{
    VSC_DG_EDGE*       pEdge = gcvNULL;
    VSC_ADJACENT_LIST* pAdjList;

    /* Traversal descendants based on search mode now */
    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_RECURSIVE)
    {
        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        SAFE_CALL_DG_NODE_HANDLER_RETURN(pfnHandlerOwnPre, pNode, pParam);

        pNode->bVisited = gcvTRUE;

        /* Visit descendants before siblings */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPre, pEdge->pToNode, pParam);
                _DoTraversalCB(pDG, pEdge->pToNode,
                               searchMode, bFromTail,
                               pfnHandlerOwnPre,
                               pfnHandlerOwnPost,
                               pfnHandlerDescendantPre,
                               pfnHandlerDescendantPost,
                               pfnHandlerDFSEdgeOnRevisit,
                               pParam);
                SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPost, pEdge->pToNode, pParam);
            }
            else
            {
                SAFE_CALL_DG_EDGE_HANDLER(pfnHandlerDFSEdgeOnRevisit, pEdge, pParam);
            }
        }

        SAFE_CALL_DG_NODE_HANDLER_RETURN(pfnHandlerOwnPost, pNode, pParam);
    }
    /* We need to use post-order to implement this here so that we can call all node handlers correctly. */
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST_ITERATIVE)
    {
        VSC_SIMPLE_STACK        stack;

        STACK_INITIALIZE(&stack);

        _PushStackWithEdge(&stack, pNode, gcvNULL, 0, pDG->pMM);

        while (!STACK_CHECK_EMPTY(&stack))
        {
            VSC_DG_EDGE*            pEdge;
            VSC_DG_NODE_WITH_EDGE*  pPopNodeWithEdge = _TopStackWithEdge(&stack);
            VSC_DG_NODE*            pPopNode = pPopNodeWithEdge->pNode;
            VSC_DG_EDGE*            pPopEdge = pPopNodeWithEdge->pEdge;
            gctUINT                 nextEdgeIndex = pPopNodeWithEdge->nextEdgeIndex;
            gctBOOL                 bPopNode = gcvFALSE;

            if (!pPopNode->bVisited)
            {
                if (pPopEdge != gcvNULL
                    &&
                    SAFE_CALL_DG_NODE_HANDLER_CHECK(pfnHandlerDescendantPre, pPopNode, pParam))
                {
                    _PopStackWithEdge(&stack, gcvNULL, pDG->pMM);
                }

                if (SAFE_CALL_DG_NODE_HANDLER_CHECK(pfnHandlerOwnPre, pPopNode, pParam))
                {
                    _PopStackWithEdge(&stack, gcvNULL, pDG->pMM);
                }

                pPopNode->bVisited = gcvTRUE;
            }

            /* Determine direction */
            pAdjList = (bFromTail) ? &pPopNode->predList : &pPopNode->succList;

            /* Get the next visit edge. */
            while (gcvTRUE)
            {
                pEdge = _GetEdgeByIndex(pDG, pPopNode, pAdjList, nextEdgeIndex);
                nextEdgeIndex++;

                /* All edges have been visited, we can pop this node. */
                if (pEdge == gcvNULL)
                {
                    bPopNode = gcvTRUE;
                    break;
                }
                /* Revisit, find next one. */
                else if (pEdge->pToNode->bVisited == gcvTRUE)
                {
                    SAFE_CALL_DG_EDGE_HANDLER(pfnHandlerDFSEdgeOnRevisit, pEdge, pParam);
                }
                else
                {
                    _PushStackWithEdge(&stack, pEdge->pToNode, pEdge, 0, pDG->pMM);
                    break;
                }
            };

            if (bPopNode)
            {
                SAFE_CALL_DG_NODE_HANDLER(pfnHandlerOwnPost, pPopNode, pParam);
                if (pPopEdge != gcvNULL)
                {
                    SAFE_CALL_DG_NODE_HANDLER(pfnHandlerDescendantPost, pPopNode, pParam);
                }

                _PopStackWithEdge(&stack, gcvNULL, pDG->pMM);
            }
            else
            {
                pPopNodeWithEdge->nextEdgeIndex = nextEdgeIndex;
            }
        }

        STACK_FINALIZE(&stack);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        VSC_SIMPLE_RESIZABLE_ARRAY unvisitedDescendantArray;
        gctUINT                    i;
        VSC_DG_NODE*               pTmpNode;

        /* Determine direction */
        pAdjList = (bFromTail) ? &pNode->predList : &pNode->succList;

        vscSRARR_Initialize(&unvisitedDescendantArray, pDG->pMM, 16, sizeof(VSC_DG_NODE*), DG_NODE_CMP);

        /* Visit siblings before descendants */
        for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
             pEdge != NULL;
             pEdge = DGEG_GET_NEXT_EDGE(pEdge))
        {
            gcmASSERT(pEdge->pFromNode == pNode);

            if (!pEdge->pToNode->bVisited)
            {
                SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerOwnPre, pNode, pParam);
                pEdge->pToNode->bVisited = gcvTRUE;
                vscSRARR_AddElement(&unvisitedDescendantArray, pEdge->pToNode);
            }
        }

        for (i = 0; i < vscSRARR_GetElementCount(&unvisitedDescendantArray); i ++)
        {
            pTmpNode = *(VSC_DG_NODE**)vscSRARR_GetElement(&unvisitedDescendantArray, i);

            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPre, pTmpNode, pParam);
            _DoTraversalCB(pDG, pTmpNode,
                           searchMode, bFromTail,
                           pfnHandlerOwnPre,
                           pfnHandlerOwnPost,
                           pfnHandlerDescendantPre,
                           pfnHandlerDescendantPost,
                           gcvNULL,
                           pParam);
            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPost, pTmpNode, pParam);
        }

        vscSRARR_Finalize(&unvisitedDescendantArray);

        SAFE_CALL_DG_NODE_HANDLER_RETURN(pfnHandlerOwnPost, pNode, pParam);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE)
    {
        VSC_SIMPLE_QUEUE  queue;
        VSC_DG_NODE*      pDequeuedNode;

        QUEUE_INITIALIZE(&queue);

        pNode->bVisited = gcvTRUE;

        _EnQueue(&queue, pNode, pDG->pMM);
        while (!QUEUE_CHECK_EMPTY(&queue))
        {
            pDequeuedNode = _DeQueue(&queue, pDG->pMM);

            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerOwnPre, pDequeuedNode, pParam);

            /* Determine direction */
            pAdjList = (bFromTail) ? &pDequeuedNode->predList : &pDequeuedNode->succList;

            /* Visit siblings before descendants */
            for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
                 pEdge != NULL;
                 pEdge = DGEG_GET_NEXT_EDGE(pEdge))
            {
                gcmASSERT(pEdge->pFromNode == pDequeuedNode);

                if (!pEdge->pToNode->bVisited)
                {
                    SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPre, pEdge->pToNode, pParam);

                    pEdge->pToNode->bVisited = gcvTRUE;

                    _EnQueue(&queue, pEdge->pToNode, pDG->pMM);

                    SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerDescendantPost, pEdge->pToNode, pParam);
                }
            }

            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerOwnPost, pDequeuedNode, pParam);
        }

        QUEUE_FINALIZE(&queue);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }
}

void vscDG_TraversalCB(VSC_DIRECTED_GRAPH* pDG,
                       VSC_GRAPH_SEARCH_MODE searchMode,
                       gctBOOL bFromTail,
                       PFN_DG_NODE_HANLDER pfnHandlerStarter,
                       PFN_DG_NODE_HANLDER pfnHandlerOwnPre,
                       PFN_DG_NODE_HANLDER pfnHandlerOwnPost,
                       PFN_DG_NODE_HANLDER pfnHandlerDescendantPre,
                       PFN_DG_NODE_HANLDER pfnHandlerDescendantPost,
                       PFN_DG_EDGE_HANLDER pfnHandlerDFSEdgeOnRevisit, /* Only for DFS */
                       void* pParam)
{
    gctUINT                     i;
    VSC_DG_NODE*                pStartNode;
    VSC_SIMPLE_RESIZABLE_ARRAY* pStartNodeArray;

    searchMode = _ChooseImplementSearchMode(pDG, searchMode);

    /* Prepare firstly */
    pStartNodeArray = _PrepareTraversal(pDG, searchMode, bFromTail);

    /* Start from root or tail list to traversal */
    for (i = 0; i < vscSRARR_GetElementCount(pStartNodeArray); i ++)
    {
        pStartNode = *(VSC_DG_NODE**)vscSRARR_GetElement(pStartNodeArray, i);

        SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerStarter, pStartNode, pParam);

        if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
        {
            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerOwnPre, pStartNode, pParam);
            pStartNode->bVisited = gcvTRUE;
        }

        /* Start iterately traversal */
        _DoTraversalCB(pDG, pStartNode,
                       searchMode, bFromTail,
                       pfnHandlerOwnPre,
                       pfnHandlerOwnPost,
                       pfnHandlerDescendantPre,
                       pfnHandlerDescendantPost,
                       pfnHandlerDFSEdgeOnRevisit,
                       pParam);

        if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
        {
            SAFE_CALL_DG_NODE_HANDLER_CONTINUE(pfnHandlerOwnPost, pStartNode, pParam);
        }
    }
}

/*
 * DG iterator implementation
 */
VSC_DG_ITERATOR* vscDG_ITERATOR_Create(VSC_DIRECTED_GRAPH* pDG,
                                       VSC_GRAPH_SEARCH_MODE searchMode,
                                       VSC_GRAPH_TRAVERSAL_ORDER traversalOrder,
                                       gctBOOL bFromTail)
{
    VSC_DG_ITERATOR*   pDGIterator = gcvNULL;

    pDGIterator = (VSC_DG_ITERATOR*)vscMM_Alloc(pDG->pMM, sizeof(VSC_DG_ITERATOR));
    vscDG_ITERATOR_Initialize(pDGIterator, pDG, searchMode, traversalOrder, bFromTail);

    return pDGIterator;
}

void vscDG_ITERATOR_Initialize(VSC_DG_ITERATOR* pDGIterator,
                               VSC_DIRECTED_GRAPH* pDG,
                               VSC_GRAPH_SEARCH_MODE searchMode,
                               VSC_GRAPH_TRAVERSAL_ORDER traversalOrder,
                               gctBOOL bFromTail)
{
    pDGIterator->pDG = pDG;
    pDGIterator->searchMode = searchMode;
    pDGIterator->traversalOrder = traversalOrder;
    pDGIterator->bFromTail = bFromTail;
    pDGIterator->curIndexOfRootTailArray = 0;

    if (searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST)
    {
        STACK_INITIALIZE(&pDGIterator->nodeTraversalStatus.dgNodeStack);
    }
    else if (searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex = 0;
        pDGIterator->nodeTraversalStatus.dgNodeOrder.totalCount = 0;
        pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder = gcvNULL;
    }
    else
    {
        gcmASSERT(searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE);

        if (traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_POST)
        {
            pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex = 0;
            pDGIterator->nodeTraversalStatus.dgNodeOrder.totalCount = 0;
            pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder = gcvNULL;
        }
        else
        {
            QUEUE_INITIALIZE(&pDGIterator->nodeTraversalStatus.dgNodeQueue);
        }
    }
}

void vscDG_ITERATOR_Finalize(VSC_DG_ITERATOR* pDGIterator)
{
    if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST)
    {
        STACK_FINALIZE(&pDGIterator->nodeTraversalStatus.dgNodeStack);
    }
    else if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        if (pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder)
        {
            vscMM_Free(pDGIterator->pDG->pMM,
                       pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);
            pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder = gcvNULL;
        }
    }
    else
    {
        gcmASSERT(pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE);

        if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_POST)
        {
            if (pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder)
            {
                vscMM_Free(pDGIterator->pDG->pMM,
                           pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);
                pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder = gcvNULL;
            }
        }
        else
        {
            QUEUE_FINALIZE(&pDGIterator->nodeTraversalStatus.dgNodeQueue);
        }
    }
}

void vscDG_ITERATOR_Destory(VSC_DG_ITERATOR* pDGIterator)
{
    if (pDGIterator)
    {
        vscDG_ITERATOR_Finalize(pDGIterator);

        /* Free dg itself */
        vscMM_Free(pDGIterator->pDG->pMM, pDGIterator);
        pDGIterator = gcvNULL;
    }
}

VSC_DG_NODE* vscDG_ITERATOR_Begin(VSC_DG_ITERATOR* pDGIterator)
{
    VSC_SIMPLE_RESIZABLE_ARRAY* pStartNodeArray;

    /* Prepare firstly */
    pStartNodeArray = _PrepareTraversal(pDGIterator->pDG, pDGIterator->searchMode, pDGIterator->bFromTail);

    /* If there is no start nodes, just bail out */
    if (vscSRARR_GetElementCount(pStartNodeArray) == 0)
    {
        return gcvNULL;
    }

    /* Reset cur index of root-tail array */
    pDGIterator->curIndexOfRootTailArray = 0;

    if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW ||
        (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE &&
         pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_POST))
    {
        gcmASSERT(pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder == gcvNULL);
        pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder = (VSC_DG_NODE**)vscMM_Alloc(pDGIterator->pDG->pMM,
                                          sizeof(VSC_DG_NODE*) * DGNLST_GET_NODE_COUNT(&pDGIterator->pDG->nodeList));

        pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex = 0;
        pDGIterator->nodeTraversalStatus.dgNodeOrder.totalCount = DGNLST_GET_NODE_COUNT(&pDGIterator->pDG->nodeList);

        if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_PREV)
        {
            vscDG_PreOrderTraversal(pDGIterator->pDG, VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW, pDGIterator->bFromTail,
                                    gcvFALSE, pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);
        }
        else
        {
            vscDG_PstOrderTraversal(pDGIterator->pDG, VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW, pDGIterator->bFromTail,
                                    gcvTRUE, pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);
        }
    }

    return vscDG_ITERATOR_Next(pDGIterator);
}

static void _DepthGreedyPushToStack(VSC_DG_ITERATOR* pDGIterator, VSC_DG_NODE* pStartNode)
{
    VSC_DG_EDGE*                 pEdge = gcvNULL;
    VSC_ADJACENT_LIST*           pAdjList;

    pAdjList = (pDGIterator->bFromTail) ? &pStartNode->predList : &pStartNode->succList;

    /* Check whether all its descendants are visited, if no, pick unvisited one into stack and
       then greedy pushing for other down-levels */
    for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
         pEdge != NULL;
         pEdge = DGEG_GET_NEXT_EDGE(pEdge))
    {
        gcmASSERT(pEdge->pFromNode == pStartNode);

        if (!pEdge->pToNode->bVisited)
        {
            pEdge->pToNode->bVisited = gcvTRUE;
            _PushStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                                pEdge->pToNode,
                                pDGIterator->pDG->pMM);

            _DepthGreedyPushToStack(pDGIterator, pEdge->pToNode);

            /* Every time, we always greedy push one unvisited node path to stack */
            break;
        }
    }
}

VSC_DG_NODE* vscDG_ITERATOR_Next(VSC_DG_ITERATOR* pDGIterator)
{
    VSC_SIMPLE_RESIZABLE_ARRAY*  pStartNodeArray;
    VSC_DG_NODE*                 pStartNode;
    VSC_DG_NODE*                 pTmpNode;
    VSC_QUEUE_STACK_ENTRY*       pStackEntry;
    VSC_DG_EDGE*                 pEdge = gcvNULL;
    VSC_ADJACENT_LIST*           pAdjList;

    if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST)
    {
        pStartNodeArray = (pDGIterator->bFromTail) ?
                          &pDGIterator->pDG->tailNodeArray :
                          &pDGIterator->pDG->rootNodeArray;

        if (STACK_CHECK_EMPTY(&pDGIterator->nodeTraversalStatus.dgNodeStack))
        {
            /* If stack has been empty, then check we have more start candidates, if so, go on */
            if (pDGIterator->curIndexOfRootTailArray < vscSRARR_GetElementCount(pStartNodeArray))
            {
                pStartNode = *(VSC_DG_NODE**)vscSRARR_GetElement(pStartNodeArray,
                                                                 pDGIterator->curIndexOfRootTailArray ++);

                pStartNode->bVisited = gcvTRUE;
                _PushStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                                   pStartNode,
                                   pDGIterator->pDG->pMM);

                /* Preorder just always return current node after it is pushed into stack */
                if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_PREV)
                {
                    return pStartNode;
                }
                else
                {
                    _DepthGreedyPushToStack(pDGIterator, pStartNode);
                    return _PopStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                                             pDGIterator->pDG->pMM);
                }
            }
        }
        else
        {
            pStackEntry = STACK_PEEK_TOP_ENTRY(&pDGIterator->nodeTraversalStatus.dgNodeStack);
            pTmpNode = (VSC_DG_NODE*)SQE_GET_CONTENT(pStackEntry);
            pAdjList = (pDGIterator->bFromTail) ? &pTmpNode->predList : &pTmpNode->succList;

            /* Check whether all its descendants are visited, if no, push into stack, and return it */
            for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
                 pEdge != NULL;
                 pEdge = DGEG_GET_NEXT_EDGE(pEdge))
            {
                gcmASSERT(pEdge->pFromNode == pTmpNode);

                if (!pEdge->pToNode->bVisited)
                {
                    pEdge->pToNode->bVisited = gcvTRUE;
                    _PushStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                                       pEdge->pToNode,
                                       pDGIterator->pDG->pMM);

                    /* Preorder just always return current node after it is pushed into stack */
                    if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_PREV)
                    {
                        return pEdge->pToNode;
                    }
                    else
                    {
                        _DepthGreedyPushToStack(pDGIterator, pEdge->pToNode);
                        break;
                    }
                }
            }

            /* We can pop up now */
            pTmpNode = _PopStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                                         pDGIterator->pDG->pMM);

            /* Postorder just always return current node after it is popped from stack */
            if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_POST)
            {
                return pTmpNode;
            }
        }
    }
    else if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        if (pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex == DGNLST_GET_NODE_COUNT(&pDGIterator->pDG->nodeList))
        {
            return gcvNULL;
        }
        else
        {
            return pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder[pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex ++];
        }
    }
    else
    {
        gcmASSERT(pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE);

        if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_PREV)
        {
            pStartNodeArray = (pDGIterator->bFromTail) ?
                              &pDGIterator->pDG->tailNodeArray :
                              &pDGIterator->pDG->rootNodeArray;

            if (QUEUE_CHECK_EMPTY(&pDGIterator->nodeTraversalStatus.dgNodeQueue))
            {
                /* If queue has been empty, then check we have more start candidates, if so, go on */
                if (pDGIterator->curIndexOfRootTailArray < vscSRARR_GetElementCount(pStartNodeArray))
                {
                    pStartNode = *(VSC_DG_NODE**)vscSRARR_GetElement(pStartNodeArray,
                                                                     pDGIterator->curIndexOfRootTailArray ++);

                    pStartNode->bVisited = gcvTRUE;

                    _EnQueue(&pDGIterator->nodeTraversalStatus.dgNodeQueue, pStartNode, pDGIterator->pDG->pMM);
                }
            }

            if (QUEUE_CHECK_EMPTY(&pDGIterator->nodeTraversalStatus.dgNodeQueue))
            {
                return gcvNULL;
            }

            pTmpNode = _DeQueue(&pDGIterator->nodeTraversalStatus.dgNodeQueue, pDGIterator->pDG->pMM);

            if (pTmpNode)
            {
                /* Determine direction */
                pAdjList = (pDGIterator->bFromTail) ? &pTmpNode->predList : &pTmpNode->succList;

                /* Visit siblings before descendants */
                for (pEdge = AJLST_GET_FIRST_EDGE(pAdjList);
                     pEdge != NULL;
                     pEdge = DGEG_GET_NEXT_EDGE(pEdge))
                {
                    gcmASSERT(pEdge->pFromNode == pTmpNode);

                    if (!pEdge->pToNode->bVisited)
                    {
                        pEdge->pToNode->bVisited = gcvTRUE;

                        _EnQueue(&pDGIterator->nodeTraversalStatus.dgNodeQueue, pEdge->pToNode, pDGIterator->pDG->pMM);
                    }
                }
            }

            return pTmpNode;
        }
        else
        {
            return pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder[pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex ++];
        }
    }

    return gcvNULL;
}

void vscDG_ITERATOR_End(VSC_DG_ITERATOR* pDGIterator)
{
    /* Re-intialize iterator for next time use */

    pDGIterator->curIndexOfRootTailArray = 0;

    if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_DEPTH_FIRST)
    {
        while (!STACK_CHECK_EMPTY(&pDGIterator->nodeTraversalStatus.dgNodeStack))
        {
            _PopStack(&pDGIterator->nodeTraversalStatus.dgNodeStack,
                              pDGIterator->pDG->pMM);
        }

        STACK_INITIALIZE(&pDGIterator->nodeTraversalStatus.dgNodeStack);
    }
    else if (pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_NARROW)
    {
        vscMM_Free(pDGIterator->pDG->pMM, pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);

        pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex = 0;
        pDGIterator->nodeTraversalStatus.dgNodeOrder.totalCount = 0;
    }
    else
    {
        gcmASSERT(pDGIterator->searchMode == VSC_GRAPH_SEARCH_MODE_BREADTH_FIRST_WIDE);

        if (pDGIterator->traversalOrder == VSC_GRAPH_TRAVERSAL_ORDER_POST)
        {
            vscMM_Free(pDGIterator->pDG->pMM, pDGIterator->nodeTraversalStatus.dgNodeOrder.ppGNodeOrder);

            pDGIterator->nodeTraversalStatus.dgNodeOrder.curIndex = 0;
            pDGIterator->nodeTraversalStatus.dgNodeOrder.totalCount = 0;
        }
        else
        {
            while (!QUEUE_CHECK_EMPTY(&pDGIterator->nodeTraversalStatus.dgNodeQueue))
            {
                _DeQueue(&pDGIterator->nodeTraversalStatus.dgNodeQueue,
                                  pDGIterator->pDG->pMM);
            }

            QUEUE_INITIALIZE(&pDGIterator->nodeTraversalStatus.dgNodeQueue);
        }
    }
}

