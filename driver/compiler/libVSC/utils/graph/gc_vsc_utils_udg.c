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

void vscUDGND_Initialize(VSC_UDG_NODE* pNode)
{
    vscBLN_Initialize(&pNode->biListNode);

    pNode->id = INVALID_GNODE_ID;
    pNode->degree = 0;
}

void vscUDGND_Finalize(VSC_UDG_NODE* pNode)
{
    vscBLN_Finalize(&pNode->biListNode);

    pNode->id = INVALID_GNODE_ID;
    pNode->degree = 0;
}

void vscUDGNDEXT_Initialize(VSC_UDG_NODE_EXT* pExtNode, void* pUserData)
{
    vscBSNODE_Initialize(&pExtNode->baseNode, pUserData);
    vscUDGND_Initialize(&pExtNode->udgNode);
}

void vscUDGNDEXT_Finalize(VSC_UDG_NODE_EXT* pExtNode)
{
    vscUDGND_Finalize(&pExtNode->udgNode);
}

void* vscUDGNDEXT_GetContainedUserData(VSC_UDG_NODE_EXT* pExtNode)
{
    return vscBSNODE_GetContainedUserData(&pExtNode->baseNode);
}

VSC_UNDIRECTED_GRAPH* vscUDG_Create(VSC_MM* pMM, gctUINT initNodeCount)
{
    VSC_UNDIRECTED_GRAPH*   pUDG = gcvNULL;

    pUDG = (VSC_UNDIRECTED_GRAPH*)vscMM_Alloc(pMM, sizeof(VSC_UNDIRECTED_GRAPH));
    vscUDG_Initialize(pUDG, pMM, initNodeCount);

    return pUDG;
}

void vscUDG_Initialize(VSC_UNDIRECTED_GRAPH* pUDG, VSC_MM* pMM, gctUINT initNodeCount)
{
    pUDG->pMM = pMM;
    pUDG->nextNodeId = 0;
    DGNLST_INITIALIZE(&pUDG->nodeList);
    vscHTBL_Initialize(&pUDG->ndHashTable, pMM, _HFUNC_PassThroughNodeId, gcvNULL, GNODE_HASH_TABLE_SIZE);
    vscBM_Initialize(&pUDG->bitMatrix, pMM, initNodeCount, initNodeCount);
    pUDG->matrixWidth = initNodeCount;
}

void vscUDG_Finalize(VSC_UNDIRECTED_GRAPH* pUDG)
{
    DGNLST_FINALIZE(&pUDG->nodeList);
    vscHTBL_Finalize(&pUDG->ndHashTable);
    vscBM_Finalize(&pUDG->bitMatrix);
}

void vscUDG_Destroy(VSC_UNDIRECTED_GRAPH* pUDG)
{
    if (pUDG)
    {
        vscUDG_Finalize(pUDG);

        /* Free dg itself */
        vscMM_Free(pUDG->pMM, pUDG);
        pUDG = gcvNULL;
    }
}

void vscUDG_AddNode(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode)
{
    /* A node can not be added into graph twice */
    gcmASSERT(pNode->id == INVALID_GNODE_ID);

    /* Currently, we dose not support resize of bit-matrix */
    if (pUDG->matrixWidth <= pUDG->nextNodeId)
    {
        gcmASSERT(gcvFALSE);
    }

    UDGNLST_ADD_NODE(&pUDG->nodeList, pNode);
    pNode->id = pUDG->nextNodeId ++;

    vscHTBL_DirectSet(&pUDG->ndHashTable, (void*)(gctUINTPTR_T)pNode->id, pNode);
}

void vscUDG_RemoveNode(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode)
{
    gctUINT i;

    gcmASSERT(pNode->id != INVALID_GNODE_ID);

    /* Remove all connections from adjacent nodes */
    for (i = 0; i < pUDG->matrixWidth; i ++)
    {
        if (i < pNode->id)
        {
            vscBM_ClearBit(&pUDG->bitMatrix, pNode->id, i);
        }
        else
        {
            vscBM_ClearBit(&pUDG->bitMatrix, i, pNode->id);
        }
    }

    /* Now remove node from nodeList */
    UDGNLST_REMOVE_NODE(&pUDG->nodeList, pNode);

    vscHTBL_DirectRemove(&pUDG->ndHashTable, (void*)(gctUINTPTR_T)pNode->id);

    /* If graph has been empty, we can reset id pool */
    if (UDGNLST_GET_NODE_COUNT(&pUDG->nodeList) == 0)
    {
        pUDG->nextNodeId = 0;
    }
}

void vscUDG_ConnectTwoNodes(VSC_UNDIRECTED_GRAPH* pUDG, VSC_UDG_NODE* pNode1, VSC_UDG_NODE* pNode2)
{
    gctUINT bitOrdinalX, bitOrdinalY;

    /* We use upper half maxtrix */
    bitOrdinalX = vscMAX(pNode1->id, pNode2->id);
    bitOrdinalY = vscMIN(pNode1->id, pNode2->id);
    vscBM_SetBit(&pUDG->bitMatrix, bitOrdinalX, bitOrdinalY);
}

VSC_UDG_NODE* vscUDG_GetNodeById(VSC_UNDIRECTED_GRAPH* pUDG, gctUINT nodeId)
{
    return (VSC_UDG_NODE*)vscHTBL_DirectGet(&pUDG->ndHashTable, (void*)(gctUINTPTR_T)nodeId);
}

gctUINT vscUDG_GetNodeCount(VSC_UNDIRECTED_GRAPH* pUDG)
{
    return vscBILST_GetNodeCount(&pUDG->nodeList);
}

gctUINT vscUDG_GetHistNodeCount(VSC_UNDIRECTED_GRAPH* pUDG)
{
    return pUDG->nextNodeId;
}

