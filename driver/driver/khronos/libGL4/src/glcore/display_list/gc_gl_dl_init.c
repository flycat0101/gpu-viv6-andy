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
        gcmOS_SAFE_FREE(gcvNULL, listConcatDraw->concatDlistPtrs);
        listConcatDraw->concatDlistPtrs = NULL;
    }

    if (listConcatDraw->concatVertexCount)
    {
        gcmOS_SAFE_FREE(gcvNULL, listConcatDraw->concatVertexCount);
        listConcatDraw->concatVertexCount = NULL;
    }

    if (listConcatDraw->concatIndexCount)
    {
        gcmOS_SAFE_FREE(gcvNULL, listConcatDraw->concatIndexCount);
        listConcatDraw->concatIndexCount = NULL;
    }

    if (listConcatDraw->primBegin)
    {
        if (listConcatDraw->primBegin->indexBuffer)
        {
            gcmOS_SAFE_FREE(gcvNULL, listConcatDraw->primBegin->indexBuffer);
            listConcatDraw->primBegin->indexBuffer = NULL;
        }

        if (listConcatDraw->primBegin->privateData)
        {
            /* Notify Dp to delete cached vertex buffer */
            (*gc->dp.deletePrimData)(gc, listConcatDraw->primBegin->privateData);
            listConcatDraw->primBegin->privateData = NULL;
        }

        if (listConcatDraw->primBegin->ibPrivateData)
        {
            (*gc->dp.deletePrimData)(gc, listConcatDraw->primBegin->ibPrivateData);
            listConcatDraw->primBegin->ibPrivateData = NULL;
        }

        gcmOS_SAFE_FREE(gcvNULL, listConcatDraw->primBegin);
        listConcatDraw->primBegin = NULL;
    }

    gcmOS_SAFE_FREE(gcvNULL, listConcatDraw);
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
        gcmOS_SAFE_FREE(gcvNULL, gc->dlist.arena);
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

        gcmOS_SAFE_FREE(gcvNULL, dlist->freefunc);
        dlist->freefunc = NULL;
    }

    /*
    ** Free the segment after calling the free functions in case
    ** any of them depend on data stored in the segment.
    */
    if (dlist->segment)
    {
        gcmOS_SAFE_FREE(gcvNULL, dlist->segment);
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
                gcmOS_SAFE_FREE(gcvNULL, nameNode);
                nameNode = nextNode;
            }
            break;
        }
    }

    gcmOS_SAFE_FREE(gcvNULL, dlist);

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
GLboolean __glInitDlistState(__GLcontext *gc)
{
    __GLdlistMachine *dlist = &gc->dlist;

    /*
    ** Add code to check video memory size, if it is too small we can turn off video memory cache
    ** but still leave cancatlist on. will add code later.
    */
    dlist->origConcatListCacheFlag = dlist->enableConcatListCache = __GL_DLIST_CACHE_ENABLE;

    dlist->maxConcatListCacheIdx = -1;

    if (dlist->shared == NULL)
    {
        __GL_DLIST_SEMAPHORE_LOCK();

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
            sizeof(__GLsharedObjectMachine),
            (gctPOINTER*)&dlist->shared)))
        {
             return GL_FALSE;
        }
        gcoOS_ZeroMemory(dlist->shared, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for display lists */
        dlist->shared->maxLinearTableSize = __GL_MAX_DLIST_LINEAR_TABLE_SIZE;
        dlist->shared->linearTableSize = __GL_DEFAULT_DLIST_LINEAR_TABLE_SIZE;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
            dlist->shared->linearTableSize * sizeof(GLvoid *),
            (gctPOINTER*)&dlist->shared->linearTable)))
        {
            gcmOS_SAFE_FREE(gcvNULL, dlist->shared);
             return GL_FALSE;
        }
        gcoOS_ZeroMemory(dlist->shared->linearTable, dlist->shared->linearTableSize * sizeof(GLvoid *));

        dlist->shared->hashSize = __GL_MAX_DLIST_HASH_TABLE_SIZE;
        dlist->shared->hashMask = __GL_MAX_DLIST_HASH_TABLE_SIZE - 1;
        dlist->shared->refcount = 1;
        dlist->shared->deleteObject = __glDeleteDlist;

        __GL_DLIST_SEMAPHORE_UNLOCK();
    }

    return GL_TRUE;
}
