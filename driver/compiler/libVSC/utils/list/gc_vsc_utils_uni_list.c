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

void vscULN_Initialize(VSC_UNI_LIST_NODE* pThisNode)
{
    pThisNode->pNextNode = gcvNULL;
}

void vscULN_InsertAfter(VSC_UNI_LIST_NODE* pThisNode, VSC_UNI_LIST_NODE* pInsertedNode)
{
    pInsertedNode->pNextNode = pThisNode->pNextNode;
    pThisNode->pNextNode = pInsertedNode;
}

void vscULN_Finalize(VSC_UNI_LIST_NODE* pThisNode)
{
    /* Nothing to do */
}

VSC_UNI_LIST_NODE* vscULN_GetNextNode(VSC_UNI_LIST_NODE* pThisNode)
{
    return pThisNode->pNextNode;
}

void vscULNDEXT_Initialize(VSC_UNI_LIST_NODE_EXT* pThisExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pThisExtNode->baseNode, pUserData);
    vscULN_Initialize(&pThisExtNode->ulNode);
}

void vscULNDEXT_InsertAfter(VSC_UNI_LIST_NODE_EXT* pThisExtNode, VSC_UNI_LIST_NODE_EXT* pInsertedExtNode)
{
    vscULN_InsertAfter(CAST_ULEN_2_ULN(pThisExtNode), CAST_ULEN_2_ULN(pInsertedExtNode));
}

void vscULNDEXT_Finalize(VSC_UNI_LIST_NODE_EXT* pThisExtNode)
{
    vscULN_Finalize(CAST_ULEN_2_ULN(pThisExtNode));
}

VSC_UNI_LIST_NODE_EXT* vscULNDEXT_GetNextNode(VSC_UNI_LIST_NODE_EXT* pThisExtNode)
{
    return CAST_ULN_2_ULEN(vscULN_GetNextNode(CAST_ULEN_2_ULN(pThisExtNode)));
}

void* vscULNDEXT_GetContainedUserData(VSC_UNI_LIST_NODE_EXT* pThisExtNode)
{
    if (pThisExtNode == NULL) return NULL;

    return vscBSNODE_GetContainedUserData(&pThisExtNode->baseNode);
}

void vscUNILST_Initialize(VSC_UNI_LIST* pList, gctBOOL bCircle)
{
    vscUNILST_Reset(pList);

    pList->info.bCircle = bCircle;
}

void vscUNILST_Reset(VSC_UNI_LIST* pList)
{
    pList->pHead = gcvNULL;
    pList->pTail = gcvNULL;
    pList->info.count = 0;
}

static void _UpdateCircleList(VSC_UNI_LIST* pList)
{
    if (pList->info.bCircle)
    {
        pList->pTail->pNextNode = pList->pHead;
    }
}

void vscUNILST_InsertAfter(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhere, VSC_UNI_LIST_NODE* pWhat)
{
    vscULN_InsertAfter(pWhere, pWhat);

    if (pWhere == pList->pTail)
    {
        pList->pTail = pWhat;

        _UpdateCircleList(pList);
    }

    pList->info.count ++;
}

void vscUNILST_Append(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat)
{
    pList->info.count ++;

    pWhat->pNextNode = gcvNULL;

    if (pList->pHead == gcvNULL)
    {
        gcmASSERT(pList->pTail == gcvNULL);
        pList->pHead = pList->pTail = pWhat;
    }
    else
    {
        pList->pTail->pNextNode = pWhat;
        pList->pTail = pWhat;
    }

    _UpdateCircleList(pList);
}

void vscUNILST_Prepend(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat)
{
    pList->info.count ++;

    if (pList->pHead == gcvNULL)
    {
        gcmASSERT(pList->pTail == gcvNULL);
        pList->pHead = pList->pTail = pWhat;
        pWhat->pNextNode = gcvNULL;
    }
    else
    {
        pWhat->pNextNode = pList->pHead;
        pList->pHead = pWhat;
    }

    _UpdateCircleList(pList);
}

void vscUNILST_Remove(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat)
{
    VSC_UNI_LIST_NODE* pTempNode;
    VSC_UNI_LIST_NODE* pPreNode = gcvNULL;
    VSC_UNI_LIST_NODE* pNextNode = pWhat->pNextNode;

    for (pTempNode = pList->pHead; pTempNode != gcvNULL; pTempNode = pTempNode->pNextNode)
    {
        if (pTempNode == pWhat)
        {
            pList->info.count --;

            if (pList->pHead == pWhat)
            {
                pList->pHead = pNextNode;
            }

            if (pList->pTail == pWhat)
            {
                pList->pTail = pPreNode;
            }

            if (pPreNode)
            {
                pPreNode->pNextNode = pNextNode;
            }

            _UpdateCircleList(pList);

            break;
        }

        pPreNode  = pTempNode;
    }
}

gctBOOL vscUNILST_IsEmpty(VSC_UNI_LIST* pList)
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

void vscUNILST_Finalize(VSC_UNI_LIST* pList)
{
    /* Nothing to do */
}

VSC_UNI_LIST_NODE* vscUNILST_GetHead(VSC_UNI_LIST* pList)
{
    return pList->pHead;
}

VSC_UNI_LIST_NODE* vscUNILST_GetTail(VSC_UNI_LIST* pList)
{
    return pList->pTail;
}

VSC_UNI_LIST_NODE* vscUNILST_RemoveHead(VSC_UNI_LIST* pList)
{
    VSC_UNI_LIST_NODE* pHead = pList->pHead;

    if (pHead)
    {
        vscUNILST_Remove(pList, pHead);
    }

    return pHead;
}

VSC_UNI_LIST_NODE* vscUNILST_RemoveTail(VSC_UNI_LIST* pList)
{
    VSC_UNI_LIST_NODE* pTail = pList->pTail;

    if (pTail)
    {
        vscUNILST_Remove(pList, pTail);
    }

    return pTail;
}

gctUINT vscUNILST_GetNodeCount(VSC_UNI_LIST* pList)
{
    return pList->info.count;
}

/* iterator functions */
void vscULIterator_Init(VSC_UL_ITERATOR * pIter, VSC_UNI_LIST * pList)
{
    gcmASSERT(pIter && pList);
    pIter->ul       = pList;
    pIter->curNode  = gcvNULL;
}

VSC_UNI_LIST_NODE *vscULIterator_First(VSC_UL_ITERATOR * pIter)
{
    pIter->curNode = pIter->ul->pHead;
    return pIter->curNode;
}

VSC_UNI_LIST_NODE *vscULIterator_Next(VSC_UL_ITERATOR * pIter)
{
    if (pIter->curNode != gcvNULL)
    {
        pIter->curNode = pIter->curNode->pNextNode;
    }
    return pIter->curNode;
}


VSC_UNI_LIST_NODE *vscULIterator_Last(VSC_UL_ITERATOR * pIter)
{
    return pIter->ul->pTail;
}

