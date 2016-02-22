/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vsc.h"

void vscBLN_Initialize(VSC_BI_LIST_NODE* pThisNode)
{
    pThisNode->pPrevNode = gcvNULL;
    pThisNode->pNextNode = gcvNULL;
}

void vscBLN_InsertAfter(VSC_BI_LIST_NODE* pThisNode, VSC_BI_LIST_NODE* pInsertedNode)
{
    VSC_BI_LIST_NODE* pNextNode = pThisNode->pNextNode;

    /* Next of this and inserted one */
    if (pNextNode)
    {
        pNextNode->pPrevNode = pInsertedNode;
    }
    pInsertedNode->pNextNode = pNextNode;

    /* This and inserted one */
    pThisNode->pNextNode = pInsertedNode;
    pInsertedNode->pPrevNode = pThisNode;
}

void vscBLN_InsertBefore(VSC_BI_LIST_NODE* pThisNode, VSC_BI_LIST_NODE* pInsertedNode)
{
    VSC_BI_LIST_NODE* pPrevNode = pThisNode->pPrevNode;

    /* Previous of this and inserted one */
    if (pPrevNode)
    {
        pPrevNode->pNextNode = pInsertedNode;
    }
    pInsertedNode->pPrevNode = pPrevNode;

    /* This and inserted one */
    pThisNode->pPrevNode = pInsertedNode;
    pInsertedNode->pNextNode = pThisNode;
}

void vscBLN_Finalize(VSC_BI_LIST_NODE* pThisNode)
{
    /* Nothing to do */
}

VSC_BI_LIST_NODE* vscBLN_GetPrevNode(VSC_BI_LIST_NODE* pThisNode)
{
    return pThisNode->pPrevNode;
}

VSC_BI_LIST_NODE* vscBLN_GetNextNode(VSC_BI_LIST_NODE* pThisNode)
{
    return pThisNode->pNextNode;
}

void vscBLNDEXT_Initialize(VSC_BI_LIST_NODE_EXT* pThisExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pThisExtNode->baseNode, pUserData);
    vscBLN_Initialize(&pThisExtNode->blNode);
}

void vscBLNDEXT_InsertAfter(VSC_BI_LIST_NODE_EXT* pThisExtNode, VSC_BI_LIST_NODE_EXT* pInsertedExtNode)
{
    vscBLN_InsertAfter(CAST_BLEN_2_BLN(pThisExtNode), CAST_BLEN_2_BLN(pInsertedExtNode));
}

void vscBLNDEXT_InsertBefore(VSC_BI_LIST_NODE_EXT* pThisExtNode, VSC_BI_LIST_NODE_EXT* pInsertedExtNode)
{
    vscBLN_InsertBefore(CAST_BLEN_2_BLN(pThisExtNode), CAST_BLEN_2_BLN(pInsertedExtNode));
}

void vscBLNDEXT_Finalize(VSC_BI_LIST_NODE_EXT* pThisExtNode)
{
    vscBLN_Finalize(CAST_BLEN_2_BLN(pThisExtNode));
}

VSC_BI_LIST_NODE_EXT* vscBLNDEXT_GetPrevNode(VSC_BI_LIST_NODE_EXT* pThisExtNode)
{
    return CAST_BLN_2_BLEN(vscBLN_GetPrevNode(CAST_BLEN_2_BLN(pThisExtNode)));
}

VSC_BI_LIST_NODE_EXT* vscBLNDEXT_GetNextNode(VSC_BI_LIST_NODE_EXT* pThisExtNode)
{
    return CAST_BLN_2_BLEN(vscBLN_GetNextNode(CAST_BLEN_2_BLN(pThisExtNode)));
}

void* vscBLNDEXT_GetContainedUserData(VSC_BI_LIST_NODE_EXT* pThisExtNode)
{
    return vscBSNODE_GetContainedUserData(&pThisExtNode->baseNode);
}

void vscBILST_Initialize(VSC_BI_LIST* pList, gctBOOL bCircle)
{
    vscBILST_Reset(pList);

    pList->info.bCircle = bCircle;
}

void vscBILST_Reset(VSC_BI_LIST* pList)
{
    pList->pHead = gcvNULL;
    pList->pTail = gcvNULL;
    pList->info.count = 0;
}

static void _UpdateCircleList(VSC_BI_LIST* pList)
{
    if (pList->info.bCircle)
    {
        pList->pTail->pNextNode = pList->pHead;
        pList->pHead->pPrevNode = pList->pTail;
    }
}

void vscBILST_InsertAfter(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhere, VSC_BI_LIST_NODE* pWhat)
{
    vscBLN_InsertAfter(pWhere, pWhat);

    if (pWhere == pList->pTail)
    {
        pList->pTail = pWhat;

        _UpdateCircleList(pList);
    }

    pList->info.count ++;
}

void vscBILST_InsertBefore(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhere, VSC_BI_LIST_NODE* pWhat)
{
    vscBLN_InsertBefore(pWhere, pWhat);

    if (pWhere == pList->pHead)
    {
        pList->pHead = pWhat;

        _UpdateCircleList(pList);
    }

    pList->info.count ++;
}

void vscBILST_Append(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat)
{
    pList->info.count ++;

    pWhat->pNextNode = pWhat->pPrevNode = gcvNULL;

    if (pList->pHead == gcvNULL)
    {
        gcmASSERT(pList->pTail == gcvNULL);
        pList->pHead = pList->pTail = pWhat;
    }
    else
    {
        pList->pTail->pNextNode = pWhat;
        pWhat->pPrevNode = pList->pTail;
        pList->pTail = pWhat;
    }

    _UpdateCircleList(pList);
}

void vscBILST_Prepend(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat)
{
    pList->info.count ++;

    pWhat->pNextNode = pWhat->pPrevNode = gcvNULL;

    if (pList->pHead == gcvNULL)
    {
        gcmASSERT(pList->pTail == gcvNULL);
        pList->pHead = pList->pTail = pWhat;
    }
    else
    {
        pList->pHead->pPrevNode = pWhat;
        pWhat->pNextNode = pList->pHead;
        pList->pHead = pWhat;
    }

    _UpdateCircleList(pList);
}

void vscBILST_Remove(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat)
{
    VSC_BI_LIST_NODE* pPreNode = pWhat->pPrevNode;
    VSC_BI_LIST_NODE* pNextNode = pWhat->pNextNode;

    pList->info.count --;

    if (pList->pHead == pWhat)
    {
        pList->pHead = pNextNode;
    }

    if (pList->pTail == pWhat)
    {
        pList->pTail = pPreNode;
    }

    if (pNextNode)
    {
        pNextNode->pPrevNode = pPreNode;
    }

    if (pPreNode)
    {
        pPreNode->pNextNode = pNextNode;
    }

    _UpdateCircleList(pList);
}

gctBOOL vscBILST_IsEmpty(VSC_BI_LIST* pList)
{
    if (pList->info.count == 0)
    {
        gcmASSERT(pList->pHead == gcvNULL);
        gcmASSERT(pList->pTail == gcvNULL);

        return gcvTRUE;
    }
    else
    {
        gcmASSERT(pList->pHead != gcvNULL);
        gcmASSERT(pList->pTail != gcvNULL);

        return gcvFALSE;
    }
}

void vscBILST_Finalize(VSC_BI_LIST* pList)
{
    /* Nothing to do */
}

VSC_BI_LIST_NODE* vscBILST_GetHead(VSC_BI_LIST* pList)
{
    return pList->pHead;
}

VSC_BI_LIST_NODE* vscBILST_GetTail(VSC_BI_LIST* pList)
{
    return pList->pTail;
}

VSC_BI_LIST_NODE* vscBILST_RemoveHead(VSC_BI_LIST* pList)
{
    VSC_BI_LIST_NODE* pHead = pList->pHead;

    if (pHead)
    {
        vscBILST_Remove(pList, pHead);
    }

    return pHead;
}

VSC_BI_LIST_NODE* vscBILST_RemoveTail(VSC_BI_LIST* pList)
{
    VSC_BI_LIST_NODE* pTail = pList->pTail;

    if (pTail)
    {
        vscBILST_Remove(pList, pTail);
    }

    return pTail;
}

gctUINT vscBILST_GetNodeCount(VSC_BI_LIST* pList)
{
    return pList->info.count;
}

/* iterator functions */
void vscBLIterator_Init(VSC_BL_ITERATOR * pIter, VSC_BI_LIST * pList)
{
    gcmASSERT(pIter && pList);
    pIter->bl       = pList;
    pIter->curNode  = gcvNULL;
}

VSC_BI_LIST_NODE *vscBLIterator_First(VSC_BL_ITERATOR * pIter)
{
    pIter->curNode = pIter->bl->pHead;
    return pIter->curNode;
}

VSC_BI_LIST_NODE *vscBLIterator_Next(VSC_BL_ITERATOR * pIter)
{
    if (pIter->curNode != gcvNULL)
    {
        pIter->curNode = pIter->curNode->pNextNode;
    }
    return pIter->curNode;
}

VSC_BI_LIST_NODE *vscBLIterator_Prev(VSC_BL_ITERATOR * pIter)
{
    if (pIter->curNode != gcvNULL)
    {
        pIter->curNode = pIter->curNode->pPrevNode;
    }
    return pIter->curNode;
}

VSC_BI_LIST_NODE *vscBLIterator_Last(VSC_BL_ITERATOR * pIter)
{
    return pIter->bl->pTail;
}

