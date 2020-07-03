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


/*
** Basic display list routines.
**
*/
#include "gc_es_context.h"
#include "g_lcomp.h"
#include "memmgr.h"
#include "g_lcfncs.h"
#include "g_lefncs.h"
#include "g_asmoff.h"
#include "../gc_es_object_inline.c"
#include "gc_es_debug.h"

extern __GLdlist * __glAllocateDlist(__GLcontext *gc, GLuint segsize, GLuint freeCount, GLuint name);
extern __GLdlist * __glCompileDisplayList(__GLcontext *gc, __GLcompiledDlist *compDlist);
extern GLvoid __glOptimizeDisplaylist(__GLcontext *gc, __GLcompiledDlist *cdlist);
extern GLvoid __glExecuteDisplayList(__GLcontext *gc, __GLdlist *dlist);
extern GLvoid __glConcatenateDlistPrims(__GLcontext *gc, __GLdlist *dlist);
extern GLvoid __glDeleteDlist(__GLcontext *gc, GLvoid *obj);

GLvoid __glAddNameToNameList(__GLcontext *gc, __GLdlistNameNode **nameListHead, GLuint name)
{
    __GLdlistNameNode *nameNode;
    GLuint foundNode;

    nameNode = *nameListHead;
    foundNode = GL_FALSE;
    while (nameNode) {
        if (nameNode->name == name) {
            foundNode = GL_TRUE;
            break;
        }
        nameNode = nameNode->next;
    }

    if (!foundNode) {
        if (gcmIS_SUCCESS(gcoOS_Allocate(gcvNULL, sizeof(__GLdlistNameNode), (gctPOINTER*)&nameNode)))
        {
            nameNode->name = name;
            nameNode->next = *nameListHead;
            *nameListHead = nameNode;
        }
    }
}

GLvoid __glRemoveNameFrNameList(__GLcontext *gc, __GLdlistNameNode **nameListHead, GLuint name)
{
    __GLdlistNameNode *nameNode, **prevNode;

    prevNode = nameListHead;
    nameNode = *nameListHead;
    while (nameNode) {
        if (nameNode->name == name) {
            *prevNode = nameNode->next;
            gcmOS_SAFE_FREE(gcvNULL, nameNode);
            break;
        }
        prevNode = &nameNode->next;
        nameNode = nameNode->next;
    }
}

GLvoid __glAddParentChildLink(__GLcontext *gc, __GLsharedObjectMachine *shared, GLuint parent, GLuint child)
{
    __GLdlist *parentDlist, *childDlist;

    parentDlist = (__GLdlist *)__glGetObject(gc, shared, parent);
    if (parentDlist == NULL)
    {
        /* Allocate an empty __GLdlist structure */
        parentDlist = __glAllocateDlist(gc, 0, 0, parent);

        /* Insert parentDlist to the display list lookup table */
        if (__glAddObject(gc, gc->dlist.shared, parent, parentDlist) == GL_FALSE)
        {
            __GL_ERROR_RET(GL_OUT_OF_MEMORY);
        }
    }
    __glAddNameToNameList(gc, &parentDlist->child, child);

    childDlist = (__GLdlist *)__glGetObject(gc, shared, child);
    if (childDlist == NULL)
    {
        /* Allocate an empty __GLdlist structure */
        childDlist = __glAllocateDlist(gc, 0, 0, child);

        /* Insert childDlist to the display list lookup table */
        if (__glAddObject(gc, gc->dlist.shared, child, childDlist) == GL_FALSE)
        {
            __GL_ERROR_RET(GL_OUT_OF_MEMORY);
        }
    }
    __glAddNameToNameList(gc, &childDlist->parent, parent);
}

GLboolean __glDeleteParentChildLists(__GLcontext *gc, __GLdlist *dlist)
{
    __GLdlist *parentDlist, *childDlist;
    __GLdlistNameNode **ppNameNode, *nameNode;

    /* Free the dlist->parent list */
    ppNameNode = &dlist->parent;
    nameNode = *ppNameNode;
    while (nameNode) {
        /* Unlink the current __GLdlistNameNode from the list */
        *ppNameNode = nameNode->next;

        /* Remove the child node from parentItem->child list */
        parentDlist = (__GLdlist *)__glGetObject(gc, gc->dlist.shared, nameNode->name);
        if (parentDlist == NULL) {
            return GL_FALSE;
        }
        __glRemoveNameFrNameList(gc, &parentDlist->child, dlist->name);

        /* Free the current __GLdlistNameNode */
        gcmOS_SAFE_FREE(gcvNULL, nameNode);

        nameNode = *ppNameNode;
    }

    /* Free the item->child list */
    ppNameNode = &dlist->child;
    nameNode = *ppNameNode;
    while (nameNode) {
        /* Unlink the current __GLdlistNameNode from the list */
        *ppNameNode = nameNode->next;

        /* Remove the parent node from childItem->parent list */
        childDlist = (__GLdlist *)__glGetObject(gc, gc->dlist.shared, nameNode->name);
        if (childDlist == NULL) {
            return GL_FALSE;
        }
        __glRemoveNameFrNameList(gc, &childDlist->parent, dlist->name);

        /* Free the current __GLdlistNameNode */
        gcmOS_SAFE_FREE(gcvNULL, nameNode);

        nameNode = *ppNameNode;
    }

    return GL_TRUE;
}

/************************************************************************/
/*
*** Allocate memory from memory pool of arena
*/
__GLdlistOp *__glDlistAllocOp(__GLcontext *gc, GLuint size)
{
    __GLdlistOp *newDlistOp;
    size_t memsize;

    memsize = (size_t)(sizeof(__GLdlistOp) + size);
    newDlistOp = (__GLdlistOp *)__glArenaAlloc(gc->dlist.arena, (GLuint)memsize);
    if (newDlistOp == NULL)
    {
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return (NULL);
    }

    newDlistOp->next = NULL;
    newDlistOp->size = size;
    newDlistOp->dlistFree = NULL;
    newDlistOp->dlistFreePrivateData = NULL;
    newDlistOp->aligned = GL_FALSE;

    return (newDlistOp);
}

/************************************************************************/
/*
*** Append display list op to list
*/
GLvoid __glDlistAppendOp(__GLcontext *gc, __GLdlistOp *newop)
{
    __GLcompiledDlist *list = &gc->dlist.listData;

    if (list->lastDlist)
    {
        list->lastDlist->next = newop;
    }
    else
    {
        list->dlist = newop;
    }
    list->lastDlist = newop;

    if (newop->opcode == __glop_Primitive)
    {
        list->lastPrimNode = newop;
    }
    else if (newop->opcode != __glop_PrimContinue && newop->opcode != __glop_Skip)
    {
        list->lastPrimNode = NULL;
    }
}

/*
*** Execute display list.
*/
__GL_INLINE GLvoid DoCallList(__GLcontext *gc, GLuint list)
{
    __GLdlist *dlist;
    __GLdlist *dlistPre;

    __GL_DLIST_SEMAPHORE_LOCK();

    dlist = (__GLdlist *)__glGetObject(gc, gc->dlist.shared, list);

    if (dlist && dlist->segment)
    {
        if (dlist->concatenatable && gc->dlist.enableConcatListCache)
        {
            __glConcatenateDlistPrims(gc, dlist);
        }
        else
        {
            /* Set pCurrentDlist pointer and increment the nesting level */
            dlistPre = gc->dlist.pCurrentDlist;
            gc->dlist.pCurrentDlist = dlist;
            gc->dlist.nesting++;

            __glExecuteDisplayList(gc, dlist);

            /* Restore the previous pCurrentDlist pointer and nesting level */
            gc->dlist.pCurrentDlist = dlistPre;
            gc->dlist.nesting--;
        }
    }

    __GL_DLIST_SEMAPHORE_UNLOCK();
}

/************************************************************************
***  Display list management __glim_ APIs which include
***      Create a list
***      Destroy a list/Lists
***      Execute a list/lists
***      Generate list id
************************************************************************/

GLuint APIENTRY __glim_GenLists(__GLcontext *gc, GLsizei range)
{
    GLuint start;

    if (range < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return (0);
    }
    if (range == 0)
    {
        return (0);
    }

    __GL_DLIST_SEMAPHORE_LOCK();

    start = __glGenerateNames(gc, gc->dlist.shared, range);

    if (gc->dlist.shared->linearTable)
    {
        __glCheckLinearTableSize(gc, gc->dlist.shared, (start + range));
    }

    __GL_DLIST_SEMAPHORE_UNLOCK();

    return (start);
}

GLvoid APIENTRY __glim_ListBase(__GLcontext *gc, GLuint base)
{
    gc->state.list.listBase = base;
}

GLvoid APIENTRY __glim_DeleteLists(__GLcontext *gc, GLuint list, GLsizei range)
{
    GLuint i;

    if (range < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (range == 0)
    {
        return;
    }

    __GL_DLIST_BUFFER_FLUSH(gc);

    __GL_DLIST_SEMAPHORE_LOCK();

    __glDeleteNamesFrList(gc, gc->dlist.shared, list, range);

    /* Delete lists from dlist lookup table */
    for (i = list; i < (list + range); i++) {
        __glDeleteObject(gc, gc->dlist.shared, i);
    }

    __GL_DLIST_SEMAPHORE_UNLOCK();
}

GLvoid APIENTRY __glim_NewList(__GLcontext *gc, GLuint list, GLenum mode)
{
    switch (mode)
    {
    case GL_COMPILE:
    case GL_COMPILE_AND_EXECUTE:
        break;
    default:
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    /* Must call EndList before calling NewList again! */
    if (gc->dlist.currentList)
    {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }
    if (list == 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    /* Turn off display list concatenate caching during display list compilation.
    */
    gc->dlist.enableConcatListCache = GL_FALSE;

    if (__glMarkNameUsed(gc, gc->dlist.shared, list) < 0)
    {
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return;
    }

    if (gc->dlist.arena == NULL)
    {
        gc->dlist.arena = __glNewArena(gc);

        if (!gc->dlist.arena)
        {
            __glSetError(gc, GL_OUT_OF_MEMORY);
            return;
        }
    }

    gc->pSavedModeDispatch = gc->pModeDispatch;
    gc->pModeDispatch = &gc->dlCompileDispatch;
    if (!gc->apiProfile)
    {
        gc->pEntryDispatch = gc->pModeDispatch;
    }

    gc->dlist.currentList = list;
    gc->dlist.mode = mode;
    gc->dlist.listData.dlist = NULL;
    gc->dlist.listData.lastDlist = NULL;
    gc->dlist.listData.lastPrimNode = NULL;
}

GLvoid APIENTRY __glim_EndList(__GLcontext *gc)
{
    __GLdlist *dlist = NULL;
    __GLcompiledDlist *compDlist = NULL;

    /* Must call NewList() first! */
    if (gc->dlist.currentList == 0)
    {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }

    /* Optimize the display list by invoking the DL optimizer.
    */
    compDlist = &gc->dlist.listData;
    __glOptimizeDisplaylist(gc, compDlist);

    /* Compile the display list into a contiguous memory buffer.
    */
    dlist = __glCompileDisplayList(gc, compDlist);
    if (dlist == NULL)
    {
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return;
    }

    /* Free linked list memory */
    __glArenaFreeAll(gc->dlist.arena);

    compDlist->dlist = NULL;
    compDlist->lastDlist = NULL;
    compDlist->lastPrimNode = NULL;

    __GL_DLIST_SEMAPHORE_LOCK();

    /* Insert the compiled dlist to the display list lookup table */
    if (__glAddObject(gc, gc->dlist.shared, gc->dlist.currentList, dlist) == GL_FALSE)
    {
        __GL_DLIST_SEMAPHORE_UNLOCK();
        __glSetError(gc, GL_OUT_OF_MEMORY);
        return;
    }

    __GL_DLIST_SEMAPHORE_UNLOCK();

    gc->pModeDispatch = gc->pSavedModeDispatch;
    if (!gc->apiProfile)
    {
        gc->pEntryDispatch = gc->pModeDispatch;
    }

    gc->dlist.currentList = 0;
    gc->dlist.mode = 0;

    /* Turn on display list concatenate caching after display list compilation.
    */
    gc->dlist.enableConcatListCache = gc->dlist.origConcatListCacheFlag;
}

/*
** __glim entry points for list name management.
*/
GLboolean APIENTRY __glim_IsList(__GLcontext *gc, GLuint list)
{
    GLboolean exists;
    __GL_SETUP_NOT_IN_BEGIN_RET(gc,0);

    __GL_DLIST_SEMAPHORE_LOCK();
    exists = __glIsNameDefined(gc, gc->dlist.shared, list);
    __GL_DLIST_SEMAPHORE_UNLOCK();

    return (exists);
}

/*
** __glim_ entry point for call list.
*/
GLvoid APIENTRY __glim_CallList(__GLcontext *gc, GLuint list)
{

    if (list == 0) {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (gc->dlist.nesting < gc->constants.maxListNesting)
    {
        DoCallList(gc, list);
    }
}

/*
** __glim entry point for call lists.
*/
GLvoid APIENTRY __glim_CallLists(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists)
{
    GLuint list, base;
    GLint i;
    GLuint temp = 0;

    if (n <= 0) {
        if (n < 0) {
            __glSetError(gc, GL_INVALID_VALUE);
        }
        return;
    }

    if (gc->dlist.nesting < gc->constants.maxListNesting)
    {
        base = gc->state.list.listBase;

        switch (type) {
        case GL_BYTE:
            {
                GLbyte *p = (GLbyte *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_UNSIGNED_BYTE:
            {
                GLubyte *p = (GLubyte *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_SHORT:
            {
                GLshort *p = (GLshort *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_UNSIGNED_SHORT:
            {
                GLushort *p = (GLushort *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_INT:
            {
                GLint *p = (GLint *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_UNSIGNED_INT:
            {
                GLuint *p = (GLuint *)lists;
                for (i = 0; i < n; i++) {
                    list = (*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_FLOAT:
            {
                GLfloat *p = (GLfloat *)lists;
                for (i = 0; i < n; i++) {
                    list = (GLuint)(*p++) + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_2_BYTES:
            {
                GLubyte *p = (GLubyte *)lists;
                for (i = 0; i < n; i++) {
                    temp = (*p) << 8;
                    p++;
                    temp += (*p);
                    p++;
                    list = temp + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_3_BYTES:
            {
                GLubyte *p = (GLubyte *)lists;
                for (i = 0; i < n; i++) {
                    temp = (*p) << 16;
                    p++;
                    temp += ((*p)<<8);
                    p++;
                    temp += (*p);
                    p++;
                    list = temp + base;
                    DoCallList(gc, list);
                }
            }
            break;
        case GL_4_BYTES:
            {
                GLubyte *p = (GLubyte *)lists;
                for (i = 0; i < n; i++) {
                    temp = (*p) << 24;
                    p++;
                    temp += ((*p)<<16);
                    p++;
                    temp += ((*p)<<8);
                    p++;
                    temp += (*p);
                    p++;
                    list = temp + base;
                    DoCallList(gc, list);
                }
            }
            break;
        default:
            __glSetError(gc, GL_INVALID_ENUM);
            break;
        }
    }

}

/*
*** __gllc_ entry point for call list and call lists
*/
GLvoid APIENTRY __gllc_CallList(__GLcontext *gc, GLuint list)
{
    __GLdlistOp *dlop;
    struct __gllc_CallList_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CallList(gc, list);
    }

    if (list == 0)
    {
        __gllc_InvalidValue(gc);
        return;
    }

    dlop = __glDlistAllocOp(gc, sizeof(struct __gllc_CallList_Rec));
    if (dlop == NULL) return;
    dlop->opcode = __glop_CallList;
    data = (struct __gllc_CallList_Rec *)(dlop + 1);
    data->list = list;
    __glDlistAppendOp(gc, dlop);

    /* Add parent and child relationship between gc->dlist.currentList and list.
    */
    __glAddParentChildLink(gc, gc->dlist.shared, gc->dlist.currentList, list);
}

GLvoid APIENTRY __gllc_CallLists(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists)
{
    __GLdlistOp *dlop;
    GLuint size, *listOffs;
    GLint arraySize, i;
    GLuint temp = 0;
    struct __gllc_CallLists_Rec *data;


    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        __glim_CallLists(gc, n, type, lists);
    }

    if (n < 0)
    {
        __gllc_InvalidValue(gc);
        return;
    }
    if (type < GL_BYTE || type > GL_4_BYTES)
    {
        __gllc_InvalidEnum(gc);
        return;
    }

    arraySize = n * sizeof(GLuint);
    size = (GLuint)sizeof(struct __gllc_CallLists_Rec) + __GL_PAD(arraySize);
    dlop = __glDlistAllocOp(gc, size);
    if (dlop == NULL) return;
    dlop->opcode = __glop_CallLists;
    data = (struct __gllc_CallLists_Rec *)(dlop + 1);
    data->n = n;
    listOffs = (GLuint *)(data + 1);

    switch (type) {
      case GL_BYTE:
          {
              GLbyte *p = (GLbyte *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_UNSIGNED_BYTE:
          {
              GLubyte *p = (GLubyte *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_SHORT:
          {
              GLshort *p = (GLshort *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_UNSIGNED_SHORT:
          {
              GLushort *p = (GLushort *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_INT:
          {
              GLint *p = (GLint *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_UNSIGNED_INT:
          {
              GLuint *p = (GLuint *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (*p++);
              }
          }
          break;
      case GL_FLOAT:
          {
              GLfloat *p = (GLfloat *)lists;
              for (i = 0; i < n; i++) {
                  listOffs[i] = (GLuint)(*p++);
              }
          }
          break;
      case GL_2_BYTES:
          {
              GLubyte *p = (GLubyte *)lists;
              for (i = 0; i < n; i++) {
                  temp = (*p)<<8;
                  p++;
                  temp += (*p);
                  p++;
                  listOffs[i] = temp;
              }
          }
          break;
      case GL_3_BYTES:
          {
              GLubyte *p = (GLubyte *)lists;
              for (i = 0; i < n; i++) {
                  temp = (*p)<<16;
                  p++;
                  temp += ((*p)<<8);
                  p++;
                  temp += (*p);
                  p++;
                  listOffs[i] = temp;
              }
          }
          break;
      case GL_4_BYTES:
          {
              GLubyte *p = (GLubyte *)lists;
              for (i = 0; i < n; i++) {
                  temp = (*p)<<24;
                  p++;
                  temp += ((*p)<<16);
                  p++;
                  temp += ((*p)<<8);
                  p++;
                  temp += (*p);
                  p++;
                  listOffs[i] = temp;
              }
          }
          break;
    }

    __glDlistAppendOp(gc, dlop);
}

/*
*** __glle_ entry points for call list and call lists
*/
const GLubyte *__glle_CallList(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CallList_Rec *data;


    /* Turn off enableConcatListCache flag to avoid going into list concatCache path */
    gc->dlist.enableConcatListCache = GL_FALSE;

    if (gc->dlist.nesting < gc->constants.maxListNesting)
    {
        data = (struct __gllc_CallList_Rec *) PC;
        DoCallList(gc, data->list);
    }

    gc->dlist.enableConcatListCache = gc->dlist.origConcatListCacheFlag;

    return (PC + sizeof(struct __gllc_CallList_Rec));
}

const GLubyte *__glle_CallLists(__GLcontext *gc, const GLubyte *PC)
{
    struct __gllc_CallLists_Rec *data;
    GLuint base = gc->state.list.listBase;
    GLuint size = 0, *listOffs = NULL;
    GLint i;

    /* Turn off enableConcatListCache flag to avoid going into list concatCache path */
    gc->dlist.enableConcatListCache = GL_FALSE;

    if (gc->dlist.nesting < gc->constants.maxListNesting)
    {
        data = (struct __gllc_CallLists_Rec *) PC;
        listOffs = (GLuint*)((GLubyte*)data + sizeof(struct __gllc_CallLists_Rec));
        size = sizeof(struct __gllc_CallLists_Rec) + __GL_PAD(data->n * sizeof(GLuint));

        for (i = 0; i < data->n; i++) {
            DoCallList(gc, (base + listOffs[i]));
        }
    }

    gc->dlist.enableConcatListCache = gc->dlist.origConcatListCacheFlag;

    return (PC + size);
}

