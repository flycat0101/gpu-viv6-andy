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


#ifndef __gc_cl_list_h_
#define __gc_cl_list_h_

#define INVALID_ADDRESS            ((gctPOINTER)gcvMAXUINTPTR_T)

/* slsDLINK_NODE */
typedef struct _slsDLINK_NODE
{
    struct _slsDLINK_NODE *    prev;
    struct _slsDLINK_NODE *    next;
}
slsDLINK_NODE;

#define slsDLINK_NODE_InsertNext(thisNode, nextNode) \
    do \
    { \
        (nextNode)->prev    = (thisNode); \
        (nextNode)->next    = (thisNode)->next; \
        (thisNode)->next->prev    = (nextNode); \
        (thisNode)->next    = (nextNode); \
    } \
    while (gcvFALSE)

#define slsDLINK_NODE_InsertPrev(thisNode, prevNode) \
    do \
    { \
        (prevNode)->prev    = (thisNode)->prev; \
        (prevNode)->next    = (thisNode); \
        (thisNode)->prev->next    = (prevNode); \
        (thisNode)->prev    = (prevNode); \
    } \
    while (gcvFALSE)

#if gcdDEBUG
#define slsDLINK_NODE_Invalid(node) \
    do \
    { \
        (node)->prev    = (slsDLINK_NODE *)INVALID_ADDRESS; \
        (node)->next    = (slsDLINK_NODE *)INVALID_ADDRESS; \
    } \
    while (gcvFALSE)
#else
#define slsDLINK_NODE_Invalid(node) \
    do \
    { \
    } \
    while (gcvFALSE)
#endif /* gcdDEBUG */

#define slsDLINK_NODE_Detach(node) \
    do \
    { \
        (node)->prev->next    = (node)->next; \
        (node)->next->prev    = (node)->prev; \
        slsDLINK_NODE_Invalid(node); \
    } \
    while (gcvFALSE)

#define slsDLINK_NODE_Next(node, nodeType)        ((nodeType *)((node)->next))

#define slsDLINK_NODE_Prev(node, nodeType)        ((nodeType *)((node)->prev))

/* slsDLINK_LIST */
typedef slsDLINK_NODE        slsDLINK_LIST;

#define slsDLINK_LIST_Invalid(list)            slsDLINK_NODE_Invalid(list)

#define slsDLINK_LIST_Initialize(list) \
    do \
    { \
        (list)->prev = (list); \
        (list)->next = (list); \
    } \
    while (gcvFALSE)

#define slsDLINK_LIST_IsEmpty(list)    \
    ((list)->next == (list))

#define slsDLINK_LIST_InsertFirst(list, firstNode) \
    slsDLINK_NODE_InsertNext((list), (firstNode))

#define slsDLINK_LIST_InsertLast(list, lastNode) \
    slsDLINK_NODE_InsertPrev((list), (lastNode))

#define slsDLINK_LIST_First(list, nodeType)    \
    slsDLINK_NODE_Next((list), nodeType)

#define slsDLINK_LIST_Last(list, nodeType) \
    slsDLINK_NODE_Prev((list), nodeType)

#define slsDLINK_LIST_DetachFirst(list, nodeType, firstNode) \
    do \
    { \
        *(firstNode) = slsDLINK_LIST_First((list), nodeType); \
        slsDLINK_NODE_Detach((slsDLINK_NODE *)*(firstNode)); \
    } \
    while (gcvFALSE)

#define slsDLINK_LIST_DetachLast(list, nodeType, lastNode) \
    do \
    { \
        *(lastNode) = slsDLINK_LIST_Last((list), nodeType); \
        slsDLINK_NODE_Detach((slsDLINK_NODE *)*(lastNode)); \
    } \
    while (gcvFALSE)

#define FOR_EACH_DLINK_NODE(list, nodeType, iter) \
    for ((iter) = (nodeType *)(list)->next; \
        (slsDLINK_NODE *)(iter) != (list); \
        (iter) = (nodeType *)((slsDLINK_NODE *)(iter))->next)

#define FOR_EACH_DLINK_NODE_REVERSELY(list, nodeType, iter) \
    for ((iter) = (nodeType *)(list)->prev; \
        (slsDLINK_NODE *)(iter) != (list); \
        (iter) = (nodeType *)((slsDLINK_NODE *)(iter))->prev)

#define slsDLINK_NODE_COUNT(list, count) \
    {\
        slsDLINK_NODE* iter; \
        count = 0; \
        for ((iter) = (list)->next; (iter) != (list); (iter) = (iter)->next)\
            count ++;\
    }

#define slmDLINK_LIST_Prepend(list, toList) \
      do { \
    if( !slsDLINK_LIST_IsEmpty(list) ) { \
           slsDLINK_NODE *first; \
           slsDLINK_NODE *last; \
           first = (toList)->next; \
           last = (list)->prev; \
           (toList)->next =  (list)->next; \
           (list)->next->prev = (toList); \
           last->next = first; \
           first->prev = last; \
    } \
      } while (gcvFALSE)

/* slsSLINK_NODE */
typedef struct _slsSLINK_NODE
{
    struct _slsSLINK_NODE *next;
}
slsSLINK_NODE;

/***
  The non-empty singly linked list points to the last element in the list
***/
#define slmSLINK_NODE_InsertNext(thisNode, nextNode) \
    do { \
        (nextNode)->next    = (thisNode)->next; \
        (thisNode)->next    = (nextNode); \
    } while (gcvFALSE)

#if gcdDEBUG
#define slmSLINK_NODE_Invalid(node) \
    do { \
        if(node) (node)->next = (slsSLINK_NODE *)INVALID_ADDRESS; \
    } while (gcvFALSE)
#else
#define slmSLINK_NODE_Invalid(node) \
    do { \
    }  while (gcvFALSE)
#endif /* gcdDEBUG */

#define slmSLINK_NODE_Next(node, nodeType)   ((nodeType *)((node)->next))

/* slsSLINK_LIST */
typedef slsSLINK_NODE    slsSLINK_LIST;

#define slmSLINK_LIST_Invalid(list)    slmSLINK_NODE_Invalid(list)

#define slmSLINK_LIST_Initialize(list)   (list) = gcvNULL

#define slmSLINK_LIST_IsEmpty(list)    \
    ((list) == gcvNULL)

#define slmSLINK_LIST_InsertFirst(list, firstNode) \
    do { \
       if(slmSLINK_LIST_IsEmpty(list)) { \
        (firstNode)->next = (firstNode); \
        (list) = (firstNode); \
       } \
       else { \
        slmSLINK_NODE_InsertNext((list), (firstNode)); \
       } \
    } while (gcvFALSE)

#define slmSLINK_LIST_InsertLast(list, lastNode) \
    do { \
       if(slmSLINK_LIST_IsEmpty(list)) { \
        (lastNode)->next = (lastNode); \
       } \
       else { \
        slmSLINK_NODE_InsertNext((list), (lastNode)); \
       } \
       (list) = (lastNode);  \
    } while (gcvFALSE)

#define slmSLINK_LIST_InsertAt(list, atNode, newNode) \
    do { \
       gcmASSERT(atNode); \
           slmSLINK_NODE_InsertNext((atNode), (newNode)); \
       if((list) == (atNode)) (list) = (newNode); \
    } while (gcvFALSE)

#define slmSLINK_LIST_Last(list, nodeType)  (nodeType *)(list)

#define slmSLINK_LIST_First(list, nodeType)  ((nodeType *)((list) ? (list)->next : gcvNULL))

#define slmSLINK_NODE_Detach(thisNode, prevNode) \
    do { \
        (prevNode)->next  = (thisNode)->next; \
        slmSLINK_NODE_Invalid(thisNode); \
    }  while (gcvFALSE)

#define slmSLINK_LIST_DetachFirst(list, nodeType, firstNode) \
    do { \
         if(!slmSLINK_LIST_IsEmpty(list)) { \
        *(firstNode) = slmSLINK_NODE_Next((list), nodeType); \
        slmSLINK_NODE_Detach((slsSLINK_NODE *)*(firstNode), (list)); \
        if(*(firstNode) == (nodeType *)(list)) (list) = gcvNULL; \
         } \
         else *(firstNode) = gcvNULL; \
    } while (gcvFALSE)

#define slmSLINK_LIST_Head(list) ((list) ? (list)->next : gcvNULL)

#define FOR_EACH_SLINK_NODE(list, nodeType, prev, iter) \
    for ((prev) = gcvNULL, (iter) = (nodeType *)((list) ? (list)->next : (list)); \
      (iter) != (prev); \
      (prev) = (nodeType *)(list)->next, (iter) = (nodeType *)((slsSLINK_NODE *)(iter))->next)

#define slmSLINK_LIST_Prepend(list, toList) \
      do { \
          if( !slmSLINK_LIST_IsEmpty(list) ) { \
            if(slmSLINK_LIST_IsEmpty(toList)) (toList) = (list); \
            else { \
              slsSLINK_NODE *tmp; \
              tmp = (list)->next; \
              (list)->next = (toList)->next; \
                  (toList)->next = tmp; \
            } \
          } \
      } while (gcvFALSE)


#define slmSLINK_LIST_Append(list, toList) \
      do { \
          if( !slmSLINK_LIST_IsEmpty(list) ) { \
            if(!slmSLINK_LIST_IsEmpty(toList)) { \
              slsSLINK_NODE *tmp; \
              tmp = (toList)->next; \
              (toList)->next = (list)->next; \
              (list)->next = tmp; \
            } \
            (toList) = (list); \
          } \
      } while (gcvFALSE)

#endif /* __gc_cl_list_h_ */
