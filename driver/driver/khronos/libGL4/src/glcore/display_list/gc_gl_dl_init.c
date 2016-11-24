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


#include <stdlib.h>
#include "gc_es_context.h"
#include "memmgr.h"

extern GLboolean __glDeleteParentChildLists(__GLcontext *gc, __GLdlist *dlist);

/*
** Display list execution function table
*/
extern const GLubyte * (*__glListExecFuncTable[])(__GLcontext *, const GLubyte *);

__GL_INLINE GLvoid __glFreeListConcatDraw(__GLcontext *gc, __GLDlistConcatDraw *listConcatDraw)
{

    if (listConcatDraw->concatDlistPtrs)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatDlistPtrs);
        listConcatDraw->concatDlistPtrs = NULL;
    }

    if (listConcatDraw->concatVertexCount)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatVertexCount);
        listConcatDraw->concatVertexCount = NULL;
    }

    if (listConcatDraw->concatIndexCount)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatIndexCount);
        listConcatDraw->concatIndexCount = NULL;
    }

    if (listConcatDraw->primBegin)
    {
        if (listConcatDraw->primBegin->indexBuffer)
        {
            (*gc->imports.free)(gc, listConcatDraw->primBegin->indexBuffer);
            listConcatDraw->primBegin->indexBuffer = NULL;
        }

        if (listConcatDraw->primBegin->privateData)
        {
            /* Notify Dp to delete cached vertex buffer */
            /* to do */
            /*(*gc->dp.deletePrimData)(gc, listConcatDraw->primBegin->privateData);*/
            listConcatDraw->primBegin->privateData = NULL;
        }

        if(listConcatDraw->primBegin->ibPrivateData)
        {
            /* to do */
            /*(*gc->dp.deletePrimData)(gc, listConcatDraw->primBegin->ibPrivateData);*/
            listConcatDraw->primBegin->ibPrivateData = NULL;
        }

        if (listConcatDraw->primBegin->privStreamInfo)
        {
            /* to do */
            /*(*gc->dp.deleteStreamInfo)(gc, listConcatDraw->primBegin->privStreamInfo);*/
            listConcatDraw->primBegin->privStreamInfo = NULL;
        }

        (*gc->imports.free)(gc, listConcatDraw->primBegin);
        listConcatDraw->primBegin = NULL;
    }

    (*gc->imports.free)(gc, listConcatDraw);
}

GLvoid __glFreeConcatDlistCache(__GLcontext *gc)
{
    __GLDlistConcatDraw *listConcatDraw;
    GLint i;

    for (i = 0; i <= gc->dlist.maxConcatListCacheIdx; i++)
    {
        listConcatDraw = gc->dlist.concatListCache[i];

        while (listConcatDraw)
        {
            gc->dlist.concatListCache[i] = listConcatDraw->next;

            __glFreeListConcatDraw(gc, listConcatDraw);

            listConcatDraw = gc->dlist.concatListCache[i];
        }
    }
}

GLvoid __glFreeConcatDlistDrawBatch(__GLcontext *gc, GLuint batchName)
{
    __GLDlistConcatDraw *listConcatDraw, *prevConcatDraw;
    GLint index;

    index = __GL_CONCAT_DLIST_CACHE_INDEX(batchName);
    prevConcatDraw = listConcatDraw = gc->dlist.concatListCache[index];
    if (listConcatDraw)
    {
        while (listConcatDraw) {
            if (listConcatDraw->listBatchName == batchName) {
                break;
            }
            prevConcatDraw = listConcatDraw;
            listConcatDraw = listConcatDraw->next;
        }
        if (listConcatDraw) {
            if (listConcatDraw == gc->dlist.concatListCache[index]) {
                gc->dlist.concatListCache[index] = listConcatDraw->next;
            }
            else {
                prevConcatDraw->next = listConcatDraw->next;
            }
            __glFreeListConcatDraw(gc, listConcatDraw);
        }
    }
}

GLvoid __glFreeDlistState(__GLcontext *gc)
{
    __glFreeConcatDlistCache(gc);

    if (gc->dlist.arena) {
        /* Free linked list memory */
        __glArenaFreeAll(gc->dlist.arena);
        /* Free the remaining first block */
        DeleteBlock(gc, gc->dlist.arena->firstBlock);
        /* Free the arena structure */
        (*gc->imports.free)(gc, gc->dlist.arena);
        gc->dlist.arena = NULL;
    }

    /* Free shared display list table */
    __glFreeSharedObjectState(gc, gc->dlist.shared);
}

GLboolean __glDeleteDlist(__GLcontext *gc, GLvoid *obj)
{
    GLuint freeCount;
    __GLDlistFreeFns *freeRec;
    __GLdlistNameNode *nameNode, *nextNode;
    __GLdlist * dlist = obj;

    /* Delete the parent and child name lists from dlist */
    __glDeleteParentChildLists(gc, dlist);

    if (dlist->freefunc)
    {
        freeCount = *(GLuint *)dlist->freefunc;
        freeRec = (__GLDlistFreeFns *)((GLuint *)dlist->freefunc + 1);

        /* call any free functions */
        while (freeCount--)
        {
            (*freeRec->freeFn)(gc, freeRec->data);
            freeRec->data = NULL;
            freeRec++;
        }

        (*gc->imports.free)(gc, dlist->freefunc);
        dlist->freefunc = NULL;
    }

    /*
    ** Free the segment after calling the free functions in case
    ** any of them depend on data stored in the segment.
    */
    if (dlist->segment)
    {
        (*gc->imports.free)(gc, dlist->segment);
        dlist->segment = NULL;
    }

    if (dlist->concatenatable)
    {
        switch (dlist->batchUsageFlag)
        {
        case __GL_LIST_USED_IN_SINGLE_BATCH:
            __glFreeConcatDlistDrawBatch(gc, dlist->listBatchName);
            break;
        case __GL_LIST_USED_IN_MULTI_BATCHES:
            nameNode = dlist->batchNameList;
            while (nameNode)
            {
                nextNode = nameNode->next;
                __glFreeConcatDlistDrawBatch(gc, nameNode->name);
                (*gc->imports.free)(gc, nameNode);
                nameNode = nextNode;
            }
            break;
        }
    }

    (*gc->imports.free)(gc, dlist);

    return GL_TRUE;
}

/*
** Used to share display lists between two different contexts.
*/
GLvoid __glShareDlists(__GLcontext *dst, __GLcontext *src)
{
    /* First get rid of our private display list state */
    if (dst->dlist.shared)
    {
        __glFreeSharedObjectState(dst, dst->dlist.shared);
    }

    dst->dlist.shared = src->dlist.shared;
    dst->dlist.shared->refcount++;
}

/*
*** Initialize display list, this function is called in context initialization stage.
*/
GLvoid __glInitDlistState(__GLcontext *gc)
{
    __GLdlistMachine *dlist = &gc->dlist;

    /* Disable dlist concatCache if videoMemorySize >= 64MB and systemMemorySize >= 512 MB.
    */
    dlist->origConcatListCacheFlag = dlist->enableConcatListCache = GL_FALSE;

    dlist->maxConcatListCacheIdx = -1;

    if (dlist->shared == NULL)
    {
        __GL_DLIST_SEMAPHORE_LOCK();

        dlist->shared = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine) );

        /* Initialize a linear lookup table for display lists */
        dlist->shared->maxLinearTableSize = __GL_MAX_DLIST_LINEAR_TABLE_SIZE;
        dlist->shared->linearTableSize = __GL_DEFAULT_DLIST_LINEAR_TABLE_SIZE;
        dlist->shared->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, dlist->shared->linearTableSize * sizeof(GLvoid *) );

        dlist->shared->hashSize = __GL_MAX_DLIST_HASH_TABLE_SIZE;
        dlist->shared->hashMask = __GL_MAX_DLIST_HASH_TABLE_SIZE - 1;
        dlist->shared->refcount = 1;
        dlist->shared->deleteObject = __glDeleteDlist;

        __GL_DLIST_SEMAPHORE_UNLOCK();
    }
}
