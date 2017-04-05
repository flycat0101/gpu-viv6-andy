/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_utils_queuestack_h_
#define __gc_vsc_utils_queuestack_h_

BEGIN_EXTERN_C()

/*
 * A simple queue/stack implemented with uni-directional list
 */

typedef VSC_UNI_LIST_NODE_EXT VSC_QUEUE_STACK_ENTRY;

#define SQE_INITIALIZE(pSqEntry, pUserData)  vscULNDEXT_Initialize((pSqEntry), (pUserData))
#define SQE_FINALIZE(pSqEntry)               vscULNDEXT_Finalize((pSqEntry))
#define SQE_GET_NEXT_ENTRY(pSqEntry)         vscULNDEXT_GetNextNode(pSqEntry)
#define SQE_GET_CONTENT(pSqEntry)            vscULNDEXT_GetContainedUserData((pSqEntry))

/* Queue that holds entry VSC_QUEUE_STACK_ENTRY */
typedef VSC_UNI_LIST VSC_SIMPLE_QUEUE;
#define QUEUE_INITIALIZE(pQueue)             vscUNILST_Initialize((pQueue), gcvFALSE)
#define QUEUE_FINALIZE(pQueue)               vscUNILST_Finalize((pQueue))
#define QUEUE_PUT_ENTRY(pQueue, pEntry)      vscUNILST_Append((pQueue), CAST_ULEN_2_ULN((pEntry)))
#define QUEUE_GET_ENTRY(pQueue)              CAST_ULN_2_ULEN(vscUNILST_RemoveHead((pQueue)))
#define QUEUE_PEEK_HEAD_ENTRY(pQueue)        CAST_ULN_2_ULEN(vscUNILST_GetHead((pQueue)))
#define QUEUE_CHECK_EMPTY(pQueue)            vscUNILST_IsEmpty((pQueue))

/* Stack that holds entry VSC_QUEUE_STACK_ENTRY */
typedef VSC_UNI_LIST VSC_SIMPLE_STACK;
#define STACK_INITIALIZE(pStack)             vscUNILST_Initialize((pStack), gcvFALSE)
#define STACK_FINALIZE(pStack)               vscUNILST_Finalize((pStack))
#define STACK_PUSH_ENTRY(pStack, pEntry)     vscUNILST_Append((pStack), CAST_ULEN_2_ULN((pEntry)))
#define STACK_POP_ENTRY(pStack)              CAST_ULN_2_ULEN(vscUNILST_RemoveTail((pStack)))
#define STACK_PEEK_TOP_ENTRY(pStack)         CAST_ULN_2_ULEN(vscUNILST_GetTail((pStack)))
#define STACK_CHECK_EMPTY(pStack)            vscUNILST_IsEmpty((pStack))

/*
 *  An array-based circle queue
 */

END_EXTERN_C()

#endif /* __gc_vsc_utils_queuestack_h_ */

