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


#ifndef __gc_vsc_utils_list_h_
#define __gc_vsc_utils_list_h_

BEGIN_EXTERN_C()

typedef struct _VSC_LIST_INFO
{
    gctUINT                   bCircle : 1;  /* Is this list circled ? */
    gctUINT                   count   : 31; /* (2^31-1) should be enough ???? */
}VSC_LIST_INFO;

/*
 * Uni-directional list
 */

typedef struct _VSC_UNI_LIST_NODE
{
    struct _VSC_UNI_LIST_NODE* pNextNode;
}VSC_UNI_LIST_NODE;

void vscULN_Initialize(VSC_UNI_LIST_NODE* pThisNode);
void vscULN_InsertAfter(VSC_UNI_LIST_NODE* pThisNode, VSC_UNI_LIST_NODE* pInsertedNode);
void vscULN_Finalize(VSC_UNI_LIST_NODE* pThisNode);
VSC_UNI_LIST_NODE* vscULN_GetNextNode(VSC_UNI_LIST_NODE* pThisNode);

typedef struct _VSC_UNI_LIST_NODE_EXT
{
    /* !!!VSC_UNI_LIST_NODE must be put at FIRST place */
    VSC_UNI_LIST_NODE         ulNode;
    VSC_BASE_NODE             baseNode;
}VSC_UNI_LIST_NODE_EXT;

void vscULNDEXT_Initialize(VSC_UNI_LIST_NODE_EXT* pThisExtNode, void* pUserData);
void vscULNDEXT_InsertAfter(VSC_UNI_LIST_NODE_EXT* pThisExtNode, VSC_UNI_LIST_NODE_EXT* pInsertedExtNode);
void vscULNDEXT_Finalize(VSC_UNI_LIST_NODE_EXT* pThisExtNode);
VSC_UNI_LIST_NODE_EXT* vscULNDEXT_GetNextNode(VSC_UNI_LIST_NODE_EXT* pThisExtNode);
void* vscULNDEXT_GetContainedUserData(VSC_UNI_LIST_NODE_EXT* pThisExtNode);

#define CAST_ULEN_2_ULN(pUlen)   (VSC_UNI_LIST_NODE*)(pUlen)
#define CAST_ULN_2_ULEN(pUln)    (VSC_UNI_LIST_NODE_EXT*)(pUln)

typedef struct _VSC_UNI_LIST
{
    VSC_UNI_LIST_NODE*        pHead;
    VSC_UNI_LIST_NODE*        pTail;

    VSC_LIST_INFO             info;
}VSC_UNI_LIST;

/* List node operations for common cases. Note that these functions also for case of container, but user
   needs maintain VSC_UNI_LIST_NODE_EXT and cast it to VSC_UNI_LIST_NODE before using following functions.
   TBD: Do we need support routines exclusively for special case of container as hash table?? */
void vscUNILST_Initialize(VSC_UNI_LIST* pList, gctBOOL bCircle);
void vscUNILST_Reset(VSC_UNI_LIST* pList);
void vscUNILST_InsertAfter(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhere, VSC_UNI_LIST_NODE* pWhat);
void vscUNILST_Append(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat);
void vscUNILST_Prepend(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat);
void vscUNILST_Remove(VSC_UNI_LIST* pList, VSC_UNI_LIST_NODE* pWhat);
gctBOOL vscUNILST_IsEmpty(VSC_UNI_LIST* pList);
void vscUNILST_Finalize(VSC_UNI_LIST* pList);
VSC_UNI_LIST_NODE* vscUNILST_GetHead(VSC_UNI_LIST* pList);
VSC_UNI_LIST_NODE* vscUNILST_GetTail(VSC_UNI_LIST* pList);
VSC_UNI_LIST_NODE* vscUNILST_RemoveHead(VSC_UNI_LIST* pList);
VSC_UNI_LIST_NODE* vscUNILST_RemoveTail(VSC_UNI_LIST* pList);
gctUINT vscUNILST_GetNodeCount(VSC_UNI_LIST* pList);

/*
 * Bi-directional list
 */

typedef struct _VSC_BI_LIST_NODE
{
    struct _VSC_BI_LIST_NODE*  pPrevNode;
    struct _VSC_BI_LIST_NODE*  pNextNode;
}VSC_BI_LIST_NODE;

void vscBLN_Initialize(VSC_BI_LIST_NODE* pThisNode);
void vscBLN_InsertAfter(VSC_BI_LIST_NODE* pThisNode, VSC_BI_LIST_NODE* pInsertedNode);
void vscBLN_InsertBefore(VSC_BI_LIST_NODE* pThisNode, VSC_BI_LIST_NODE* pInsertedNode);
void vscBLN_Finalize(VSC_BI_LIST_NODE* pThisNode);
VSC_BI_LIST_NODE* vscBLN_GetPrevNode(VSC_BI_LIST_NODE* pThisNode);
VSC_BI_LIST_NODE* vscBLN_GetNextNode(VSC_BI_LIST_NODE* pThisNode);

typedef struct _VSC_BI_LIST_NODE_EXT
{
    /* !!!VSC_BI_LIST_NODE must be put at FIRST place */
    VSC_BI_LIST_NODE          blNode;
    VSC_BASE_NODE             baseNode;
}VSC_BI_LIST_NODE_EXT;

void vscBLNDEXT_Initialize(VSC_BI_LIST_NODE_EXT* pThisExtNode, void* pUserData);
void vscBLNDEXT_InsertAfter(VSC_BI_LIST_NODE_EXT* pThisExtNode, VSC_BI_LIST_NODE_EXT* pInsertedExtNode);
void vscBLNDEXT_InsertBefore(VSC_BI_LIST_NODE_EXT* pThisExtNode, VSC_BI_LIST_NODE_EXT* pInsertedExtNode);
void vscBLNDEXT_Finalize(VSC_BI_LIST_NODE_EXT* pThisExtNode);
VSC_BI_LIST_NODE_EXT* vscBLNDEXT_GetPrevNode(VSC_BI_LIST_NODE_EXT* pThisExtNode);
VSC_BI_LIST_NODE_EXT* vscBLNDEXT_GetNextNode(VSC_BI_LIST_NODE_EXT* pThisExtNode);
void* vscBLNDEXT_GetContainedUserData(VSC_BI_LIST_NODE_EXT* pThisExtNode);

#define CAST_BLEN_2_BLN(pBlen)   (VSC_BI_LIST_NODE*)(pBlen)
#define CAST_BLN_2_BLEN(pBln)    (VSC_BI_LIST_NODE_EXT*)(pBln)

typedef struct _VSC_BI_LIST
{
    VSC_BI_LIST_NODE*         pHead;
    VSC_BI_LIST_NODE*         pTail;

    VSC_LIST_INFO             info;
}VSC_BI_LIST;

/* List node operations for common cases. Note that these functions also for case of container, but user
   needs maintain VSC_BI_LIST_NODE_EXT and cast it to VSC_BI_LIST_NODE before using following functions.
   TBD: Do we need support routines exclusively for special case of container as hash table?? */
void vscBILST_Initialize(VSC_BI_LIST* pList, gctBOOL bCircle);
void vscBILST_Reset(VSC_BI_LIST* pList);
void vscBILST_InsertAfter(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhere, VSC_BI_LIST_NODE* pWhat);
void vscBILST_InsertBefore(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhere, VSC_BI_LIST_NODE* pWhat);
void vscBILST_Append(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat);
void vscBILST_Prepend(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat);
void vscBILST_Remove(VSC_BI_LIST* pList, VSC_BI_LIST_NODE* pWhat);
gctBOOL vscBILST_IsEmpty(VSC_BI_LIST* pList);
void vscBILST_Finalize(VSC_BI_LIST* pList);
VSC_BI_LIST_NODE* vscBILST_GetHead(VSC_BI_LIST* pList);
VSC_BI_LIST_NODE* vscBILST_GetTail(VSC_BI_LIST* pList);
VSC_BI_LIST_NODE* vscBILST_RemoveHead(VSC_BI_LIST* pList);
VSC_BI_LIST_NODE* vscBILST_RemoveTail(VSC_BI_LIST* pList);
gctUINT vscBILST_GetNodeCount(VSC_BI_LIST* pList);

/* list iterators */

/* bi-list iterator */
typedef struct _VSC_BL_ITERATOR
{
    VSC_BI_LIST *       bl;
    VSC_BI_LIST_NODE *  curNode;
} VSC_BL_ITERATOR;

void vscBLIterator_Init(VSC_BL_ITERATOR *, VSC_BI_LIST *);
VSC_BI_LIST_NODE *vscBLIterator_First(VSC_BL_ITERATOR *);
VSC_BI_LIST_NODE *vscBLIterator_Next(VSC_BL_ITERATOR *);
VSC_BI_LIST_NODE *vscBLIterator_Prev(VSC_BL_ITERATOR *);
VSC_BI_LIST_NODE *vscBLIterator_Last(VSC_BL_ITERATOR *);

/* uni-list iterator */
typedef struct _VSC_UL_ITERATOR
{
    VSC_UNI_LIST *      ul;
    VSC_UNI_LIST_NODE * curNode;
} VSC_UL_ITERATOR;

void vscULIterator_Init(VSC_UL_ITERATOR *, VSC_UNI_LIST *);
VSC_UNI_LIST_NODE *vscULIterator_First(VSC_UL_ITERATOR *);
VSC_UNI_LIST_NODE *vscULIterator_Next(VSC_UL_ITERATOR *);
VSC_UNI_LIST_NODE *vscULIterator_Last(VSC_UL_ITERATOR *);


END_EXTERN_C()

#endif /* __gc_vsc_utils_list_h_ */

