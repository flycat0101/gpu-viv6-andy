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


#include "gc_es_context.h"
#include "g_lcomp.h"
#include "gc_es_debug.h"

/* Macro to check if privateData.resource.handle is NULL */
#define __GL_DP_RESOURCE_HANDLE(privateData)   (*(GLuint*)(privateData))

extern const GLubyte *__glle_Sentinel(__GLcontext *gc, const GLubyte *PC);
/* To add the next */
extern GLvoid __glDrawDlistPrimitive(__GLcontext *gc, __GLPrimBegin *primBegin);
extern GLvoid __glAddNameToNameList(__GLcontext *gc, __GLdlistNameNode **nameListHead, GLuint name);
extern GLvoid __glGeneratePrimIndexStream(__GLcontext *gc, GLuint vertexCount, GLuint currentIndex,
                                        __GLPrimBegin *primBegin, GLenum currentPrimType);

GLvoid APIENTRY __glim_CallList_Cache(__GLcontext *gc, GLuint list);
GLvoid APIENTRY __glim_CallLists_Cache(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists);


__GL_INLINE GLvoid __glDlistUpdateVertexState(__GLcontext *gc, __GLPrimBegin *primBegin)
{
    GLfloat *lastVertexData;
    GLuint i, inputMask;

    lastVertexData = (GLfloat *)(primBegin + 1) +
                    (primBegin->vertexCount - 1) * primBegin->totalStrideDW;

    inputMask = (primBegin->primInputMask & ~(__GL_INPUT_VERTEX | __GL_INPUT_EDGEFLAG));

    i = 0;
    while(inputMask)
    {
        if (inputMask & 0x1)
        {
            GLfloat *src, *dst;
            src = lastVertexData + primBegin->elemOffsetDW[i];
            dst = (GLfloat*)&gc->state.current.currentState[i];
            switch(primBegin->elemSizeDW[i])
            {
            case 4:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = *(src + 2);
                *(dst + 3) = *(src + 3);
                break;
            case 3:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = *(src + 2);
                *(dst + 3) = 1.0;
                break;
            case 2:
                *(dst) = *(src);
                *(dst + 1) = *(src + 1);
                *(dst + 2) = 0.0;
                *(dst + 3) = 1.0;
                break;
            case 1:
                if (i == __GL_INPUT_DIFFUSE_INDEX)
                {
                    GLubyte *ubcolor = (GLubyte *)src;
                    *(dst) = __GL_UB_TO_FLOAT(*(ubcolor));                /* R */
                    *(dst + 1) = __GL_UB_TO_FLOAT(*(ubcolor + 1));        /* G */
                    *(dst + 2) = __GL_UB_TO_FLOAT(*(ubcolor + 2));        /* B */
                    *(dst + 3) = __GL_UB_TO_FLOAT(*(ubcolor + 3));        /* A */
                }
                else {
                    *(dst) = *(src);
                    *(dst + 1) = 0.0;
                    *(dst + 2) = 0.0;
                    *(dst + 3) = 1.0;
                }
                break;
            default:
                GL_ASSERT(0);
                break;
            }
        }
        i += 1;
        inputMask >>= 1;
    }

    if (primBegin->primInputMask & __GL_INPUT_EDGEFLAG)
    {
        gc->state.current.edgeflag = primBegin->edgeflagBuffer[primBegin->vertexCount - 1];
    }

    /* Use the current color to update material state if color material is enabled */
    if (primBegin->primInputMask & __GL_INPUT_DIFFUSE &&
        gc->state.enables.lighting.colorMaterial)
    {
        __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
            gc->state.light.colorMaterialParam, (GLfloat *)&gc->state.current.color);
    }
}

const GLubyte *__glle_Primitive(__GLcontext *gc, const GLubyte *PC)
{

    __GLPrimBegin *primBegin = (__GLPrimBegin *) PC;
    GLuint dataSize = primBegin->vertexCount * (primBegin->totalStrideDW << 2);

    /* Draw the consistent primitive */
    __glDrawDlistPrimitive(gc, primBegin);

    /* Use the last vertex's data to update current vertex attributes in gc */
    __glDlistUpdateVertexState(gc, primBegin);

    return (PC + sizeof(__GLPrimBegin) + dataSize);
}

GLvoid __glExecuteDisplayList(__GLcontext *gc, __GLdlist *dlist)
{
    GLubyte *PC = dlist->segment;
    __GLlistExecFunc *func = *((__GLlistExecFunc **) PC);

    __GL_DLIST_BUFFER_FLUSH(gc);

    /* Execute all the GL calls in the display list */
    while (func != (__GLlistExecFunc *)__glle_Sentinel)
    {
        PC = (GLubyte *)(*func)(gc, PC + sizeof(__GLlistExecFunc *));
        func = *((__GLlistExecFunc **)PC);
    }
}

__GL_INLINE GLvoid __glConcatListBatchEnd(__GLcontext *gc)
{
    __GLDlistConcatDraw *listConcatDraw = NULL;
    __GLPrimBegin *primBegin;
    __GLdlist *dlist;
    GLuint dataSize;
    GLint index;

    if (gc->dlist.concatListCount > 1)
    {
        primBegin = (__GLPrimBegin *)gc->input.defaultDataBuffer;

        /* There is no need to cache index buffer if indexCount equals vertexCount (GL_TRIANGLES) */
        if (primBegin->indexCount == primBegin->vertexCount)
        {
            primBegin->indexCount = 0;
            primBegin->indexBuffer = NULL;
        }

        /* Flush the concatenated dlists and use the last vertex's data
        ** to update current vertex attributes in gc.
        */
        __glDrawDlistPrimitive(gc, primBegin);
        __glDlistUpdateVertexState(gc, primBegin);

        /* Cache the concatenated dlist's info if the batch is successfully cached in device memory.
        ** If fail, cache the concatenated dlist's info and the vertex data in system memory.
        */
        if (primBegin->privateData && __GL_DP_RESOURCE_HANDLE(primBegin->privateData))
        {
            dataSize = sizeof(__GLPrimBegin);
        }
        else
        {
            dataSize = sizeof(__GLPrimBegin) + ((primBegin->vertexCount * primBegin->totalStrideDW) << 2);
        }

        /* Allocate and initialize a listConcatDraw structure */
        listConcatDraw = (__GLDlistConcatDraw *)(*gc->imports.malloc)(gc, sizeof(__GLDlistConcatDraw) );

        if(listConcatDraw == NULL)
        {
            goto cache_fail;
        }

        listConcatDraw->primBegin = (__GLPrimBegin *)(*gc->imports.malloc)(gc, dataSize );

        if(listConcatDraw->primBegin == NULL)
        {
            goto cache_fail;
        }

        __GL_MEMCOPY(listConcatDraw->primBegin, primBegin, dataSize);

        if (primBegin->indexCount)
        {
            /* Allocate and copy indexBuffer to listConcatDraw->primBegin.indexBuffer */
            dataSize = primBegin->indexCount * sizeof(GLushort);
            listConcatDraw->primBegin->indexBuffer = (GLushort *)(*gc->imports.malloc)(gc, dataSize );

            if(listConcatDraw->primBegin->indexBuffer == NULL)
            {
                goto cache_fail;
            }

            __GL_MEMCOPY(listConcatDraw->primBegin->indexBuffer, gc->input.defaultIndexBuffer, dataSize);

            /* Allocate and copy concatIndexCount[] to listConcatDraw->concatIndexCount */
            dataSize = gc->dlist.concatListCount * sizeof(GLint);
            listConcatDraw->concatIndexCount = (GLint *)(*gc->imports.malloc)(gc, dataSize );
            if(listConcatDraw->concatIndexCount == NULL)
            {
                goto cache_fail;
            }

            __GL_MEMCOPY(listConcatDraw->concatIndexCount, gc->dlist.concatIndexCount, dataSize);
        }
        else
        {
            listConcatDraw->primBegin->indexBuffer = NULL;
            listConcatDraw->concatIndexCount = NULL;
        }

        listConcatDraw->concatListCount = gc->dlist.concatListCount;

        /* Allocate and copy concatDlistPtrs[] to listConcatDraw->concatDlistPtrs */
        dataSize = gc->dlist.concatListCount * sizeof(__GLdlist *);
        listConcatDraw->concatDlistPtrs = (__GLdlist **)(*gc->imports.malloc)(gc, dataSize );
        if(listConcatDraw->concatDlistPtrs == NULL)
        {
            goto cache_fail;
        }
        __GL_MEMCOPY(listConcatDraw->concatDlistPtrs, gc->dlist.concatDlistPtrs, dataSize);

        /* Allocate and copy concatVertexCount[] to listConcatDraw->concatVertexCount */
        dataSize = gc->dlist.concatListCount * sizeof(GLint);
        listConcatDraw->concatVertexCount = (GLint *)(*gc->imports.malloc)(gc, dataSize );
        if(listConcatDraw->concatVertexCount == NULL)
        {
            goto cache_fail;
        }
        __GL_MEMCOPY(listConcatDraw->concatVertexCount, gc->dlist.concatVertexCount, dataSize);

        /* Use the first list's name as the concatenate list batch name */
        listConcatDraw->listBatchName = gc->dlist.concatDlistPtrs[0]->name;

        /* Save listConcatDraw in gc->dlist.concatListCache[] */
        index = __GL_CONCAT_DLIST_CACHE_INDEX(listConcatDraw->listBatchName);
        listConcatDraw->next = gc->dlist.concatListCache[index];
        gc->dlist.concatListCache[index] = listConcatDraw;
        if (gc->dlist.maxConcatListCacheIdx < index)
        {
            gc->dlist.maxConcatListCacheIdx = index;
        }

        /* Save the listConcatDraw->listBatchName in all the lists in the batch */
        for (index = 0; index < gc->dlist.concatListCount; index++)
        {
            dlist = gc->dlist.concatDlistPtrs[index];
            switch (dlist->batchUsageFlag)
            {
            case __GL_LIST_NOT_USED_IN_BATCH:
                dlist->listBatchName = listConcatDraw->listBatchName;
                dlist->batchUsageFlag = __GL_LIST_USED_IN_SINGLE_BATCH;
                break;
            case __GL_LIST_USED_IN_SINGLE_BATCH:
                __glAddNameToNameList(gc, &dlist->batchNameList, dlist->listBatchName);
                __glAddNameToNameList(gc, &dlist->batchNameList, listConcatDraw->listBatchName);
                dlist->batchUsageFlag = __GL_LIST_USED_IN_MULTI_BATCHES;
                break;
            case __GL_LIST_USED_IN_MULTI_BATCHES:
                __glAddNameToNameList(gc, &dlist->batchNameList, listConcatDraw->listBatchName);
                break;
            }
        }
    }
    else
    {
        dlist = gc->dlist.concatDlistPtrs[0];

        /* Draw the previous single list without caching it.
        */
        primBegin = (__GLPrimBegin *)(dlist->segment + sizeof(__GLlistExecFunc *));
        __glDrawDlistPrimitive(gc, primBegin);
        __glDlistUpdateVertexState(gc, primBegin);

        /* Set dlist->concatenatable to FALSE so it never comes into this function again */
        dlist->concatenatable = GL_FALSE;
    }

    /* Reset everything */
    gc->dlist.currentConcatDraw = NULL;
    gc->dlist.concatListCount = 0;
    gc->input.beginMode = __GL_NOT_IN_BEGIN;

    return;

cache_fail:

    if(listConcatDraw && listConcatDraw->concatVertexCount)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatVertexCount);
    }

    if(listConcatDraw && listConcatDraw->concatDlistPtrs)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatDlistPtrs);
    }

    if(listConcatDraw && listConcatDraw->concatIndexCount)
    {
        (*gc->imports.free)(gc, listConcatDraw->concatIndexCount);
    }

    if(listConcatDraw && listConcatDraw->primBegin && listConcatDraw->primBegin->indexBuffer)
    {
        (*gc->imports.free)(gc, listConcatDraw->primBegin->indexBuffer);
    }

    if(listConcatDraw && listConcatDraw->primBegin)
    {
        (*gc->imports.free)(gc, listConcatDraw->primBegin);
    }

    if(listConcatDraw)
    {
        (*gc->imports.free)(gc, listConcatDraw);
    }

    /* Reset everything */
    gc->dlist.currentConcatDraw = NULL;
    gc->dlist.concatListCount = 0;
    gc->input.beginMode = __GL_NOT_IN_BEGIN;
}

__GL_INLINE GLvoid __glConcatListCacheEnd(__GLcontext *gc)
{
    __GLDlistConcatDraw *listConcatDraw;
    __GLPrimBegin *primBegin;
    GLint vertCount, idxCount;
    __GLdlist *lastDlist;

    if (gc->dlist.concatListCount)
    {
        /* Fetch the primBegin from the current concatenate dlist cache */
        listConcatDraw = gc->dlist.currentConcatDraw;
        primBegin = listConcatDraw->primBegin;

        /* Set primBegin->vertexCount/indexCount to the current counts */
        vertCount = primBegin->vertexCount;
        idxCount = primBegin->indexCount;
        primBegin->vertexCount = listConcatDraw->concatVertexCount[gc->dlist.concatListCount - 1];
        if (primBegin->indexCount)
        {
            primBegin->indexCount = listConcatDraw->concatIndexCount[gc->dlist.concatListCount - 1];
        }
        lastDlist = listConcatDraw->concatDlistPtrs[gc->dlist.concatListCount - 1];

        /* Draw the concatenated dlists using the cached vertex buffer */
        __glDrawDlistPrimitive(gc, primBegin);

        /* Restore the original vertexCount and indexCount */
        primBegin->vertexCount = vertCount;
        primBegin->indexCount = idxCount;

        /* Use the last dlist's last vertex data to update current vertex attributes in gc */
        primBegin = (__GLPrimBegin *)(lastDlist->segment + sizeof(__GLlistExecFunc *));
        __glDlistUpdateVertexState(gc, primBegin);
    }

    /* Reset everything */
    gc->dlist.currentConcatDraw = NULL;
    gc->dlist.concatListCount = 0;
    gc->input.beginMode = __GL_NOT_IN_BEGIN;

    /* Restore the generic CallList dispatch entry function */
    gc->immedModeDispatch.CallList = __glim_CallList;
    gc->immedModeDispatch.CallLists = __glim_CallLists;
}

GLvoid __glDisplayListBatchEnd(__GLcontext *gc)
{
    if (gc->dlist.currentConcatDraw)
    {
        /* Draw the partially matched concatenated dlists using the cached vertex bufer */
        __glConcatListCacheEnd(gc);
    }
    else {
        /* End the current dlist batching */
        __glConcatListBatchEnd(gc);
    }
}

GLvoid __glConcatenateDlistPrims(__GLcontext *gc, __GLdlist *dlist)
{
    __GLPrimBegin *newBegin = (__GLPrimBegin *)(dlist->segment + sizeof(__GLlistExecFunc *));
    __GLPrimBegin *primBegin;
    __GLDlistConcatDraw *listConcatDraw;
    GLuint dataSize, concatDlists;
    GLuint64 mask;
    GLint index;

    /* Draw dlist immediately if polygon mode is not fill mode */
    if (newBegin->primType >= GL_TRIANGLES && !gc->state.polygon.bothFaceFill)
    {
        __GL_DLIST_BUFFER_FLUSH(gc);

        /* Draw the dlist and then update current vertex attributes */
        __glDrawDlistPrimitive(gc, newBegin);
        __glDlistUpdateVertexState(gc, newBegin);

        return;
    }
Restart:

    /* For the first dlist of the current dlist concatenate batch.
    */
    if (gc->dlist.concatListCount == 0)
    {
        /* See if there is a matching cached dlist batch.
        */
        index = __GL_CONCAT_DLIST_CACHE_INDEX(dlist->name);
        listConcatDraw = gc->dlist.concatListCache[index];
        if (listConcatDraw)
        {
            while (listConcatDraw) {
                if (listConcatDraw->listBatchName == dlist->name) {
                    break;
                }
                listConcatDraw = listConcatDraw->next;
            }
            if (listConcatDraw) {
                /* Simply return if we find a matching dlist batch */
                gc->dlist.currentConcatDraw = listConcatDraw;
                gc->dlist.concatListCount++;
                gc->input.beginMode = __GL_SMALL_LIST_BATCH;

                /* Overwrite CallList dispatch entry with __glim_CallList_Cache function */
                gc->immedModeDispatch.CallList = __glim_CallList_Cache;
                gc->immedModeDispatch.CallLists = __glim_CallLists_Cache;

                return;
            }
        }

        /* If there is no matching dlist batch in the cache then
        ** copy the dlist vertex data buffer into gc->input.defaultDataBuffer.
        */
        dataSize = ((newBegin->vertexCount * newBegin->totalStrideDW) << 2) + sizeof(__GLPrimBegin);
        __GL_MEMCOPY(gc->input.defaultDataBuffer, newBegin, dataSize);

        /* Copy dlist index buffer into gc->input.defaultDataBuffer */
        dataSize = newBegin->indexCount * sizeof(GLushort);
        __GL_MEMCOPY(gc->input.defaultIndexBuffer, newBegin->indexBuffer, dataSize);

        primBegin = (__GLPrimBegin *)gc->input.defaultDataBuffer;
        primBegin->indexBuffer = gc->input.defaultIndexBuffer;
        primBegin->primStartAddr = NULL;
        primBegin->primVertCount = NULL;
        primBegin->privateData = NULL;
        primBegin->ibPrivateData = NULL;
        primBegin->edgeflagBuffer = NULL;
        primBegin->primCount = 1;

        /* Save the list pointer in the concatDlistPtrs array */
        gc->dlist.currentConcatDraw = NULL;
        gc->dlist.concatDlistPtrs[gc->dlist.concatListCount] = dlist;
        gc->dlist.concatVertexCount[gc->dlist.concatListCount] = newBegin->vertexCount;
        gc->dlist.concatIndexCount[gc->dlist.concatListCount] = newBegin->indexCount;
        gc->dlist.concatListCount++;
        gc->input.beginMode = __GL_SMALL_LIST_BATCH;

        return;
    }

    /* Fetch the first dlist's primBegin information */
    primBegin = (__GLPrimBegin *)gc->input.defaultDataBuffer;

    /* Check if the new list has the same primType and format with previous lists */
    concatDlists = GL_FALSE;
    if ((primBegin->primType == newBegin->primType ||
        (primBegin->primType >= GL_TRIANGLES && primBegin->primType <= GL_TRIANGLE_FAN &&
         newBegin->primType >= GL_TRIANGLES && newBegin->primType <= GL_TRIANGLE_FAN)) &&
        primBegin->primitiveFormat == newBegin->primitiveFormat &&
        gc->dlist.concatListCount < __GL_MAX_CONCAT_DLIST_COUNT)
    {
        GLint totalVertexCount = primBegin->vertexCount + newBegin->vertexCount;
        GLint totalVertexDataSize = (totalVertexCount * primBegin->totalStrideDW) << 2;

        if (totalVertexCount < __GL_MAX_VERTEX_NUMBER &&
            totalVertexDataSize < (__GL_DEFAULT_VERTEX_BUFFER_SIZE - sizeof(__GLPrimBegin)))
        {
            if (primBegin->elementCount <= __GL_MAX_PRIM_ELEMENT_NUMBER)
            {
                if (primBegin->primElemSequence == newBegin->primElemSequence)
                {
                    concatDlists = GL_TRUE;
                }
            }
            else
            {
                concatDlists = GL_TRUE;
                index = 0;
                mask = primBegin->primInputMask;
                while (mask) {
                    if (mask & 0x1) {
                        if (primBegin->elemOffsetDW[index] != newBegin->elemOffsetDW[index]) {
                            concatDlists = GL_FALSE;
                            break;
                        }
                    }
                    mask >>= 1;
                    index += 1;
                }
            }
        }
    }

    /* If the new dlist has exactly the same type of primitive then we can
    ** concatenate the new dlist primitive with the previous dlist primitives.
    */
    if (concatDlists)
    {
        /* Concatenate dlist vertex data buffer into gc->input.defaultDataBuffer */
        GLubyte *bufPtr = (GLubyte *)gc->input.defaultDataBuffer + sizeof(__GLPrimBegin)
                        + ((primBegin->vertexCount * primBegin->totalStrideDW) << 2);
        dataSize = (newBegin->vertexCount * newBegin->totalStrideDW) << 2;
        __GL_MEMCOPY(bufPtr, (newBegin + 1), dataSize);

        if (primBegin->indexCount && newBegin->indexCount)
        {
            /* Concatenate dlist index buffer into gc->input.defaultIndexBuffer */
            for (index = 0; index < newBegin->indexCount; index++)
            {
                gc->input.defaultIndexBuffer[primBegin->indexCount + index] =
                                        newBegin->indexBuffer[index] + (GLushort)primBegin->vertexCount;
            }
        }
        else
        {
            if (primBegin->indexCount == 0 && primBegin->primType == GL_TRIANGLES)
            {
                __glGeneratePrimIndexStream(gc, primBegin->vertexCount, 0, primBegin, GL_TRIANGLES);

                for (index = 0; index < newBegin->indexCount; index++)
                {
                    gc->input.defaultIndexBuffer[primBegin->indexCount + index] =
                                            newBegin->indexBuffer[index] + (GLushort)primBegin->vertexCount;
                }
            }
            if (newBegin->indexCount == 0 && newBegin->primType == GL_TRIANGLES)
            {
                __glGeneratePrimIndexStream(gc, newBegin->vertexCount, primBegin->vertexCount, primBegin, GL_TRIANGLES);
            }
        }

        primBegin->vertexCount += newBegin->vertexCount;
        primBegin->indexCount += newBegin->indexCount;

        /* Save the list pointer in the concatDlistPtrs array */
        gc->dlist.concatDlistPtrs[gc->dlist.concatListCount] = dlist;
        gc->dlist.concatVertexCount[gc->dlist.concatListCount] = primBegin->vertexCount;
        gc->dlist.concatIndexCount[gc->dlist.concatListCount] = primBegin->indexCount;
        gc->dlist.concatListCount++;
        gc->input.beginMode = __GL_SMALL_LIST_BATCH;

        return;
    }
    else
    {
        /* End the current dlist batching */
        __glConcatListBatchEnd(gc);

        /* Go back to "Restart" to process the current dlist again */
        goto Restart;
    }
}

/*
** __glim_ for CallList cache entry function
*/
GLvoid APIENTRY __glim_CallList_Cache(__GLcontext *gc, GLuint list)
{




    /* If there is a current concatenate dlist draw cache, then we just need to check if the incoming
    ** dlist name matches the cached dlist name, and simply return if the dlist name matches.
    */
    if (gc->dlist.currentConcatDraw->concatDlistPtrs[gc->dlist.concatListCount]->name == list)
    {
        gc->dlist.concatListCount++;

        /* Draw the concatenated dlists immediately if "list" matches the last list in the batch */
        if (gc->dlist.concatListCount == gc->dlist.currentConcatDraw->concatListCount)
        {
            __glConcatListCacheEnd(gc);
        }
    }
    else
    {
        /* Draw the partially matched concatenated dlists using the cached vertex bufer */
        __glConcatListCacheEnd(gc);

        /* Go back to __glim_CallList to process the current list again */
        __glim_CallList(gc, list);
    }
}

/*
** __glim_ for Calllists cache entry function
*/
GLvoid APIENTRY __glim_CallLists_Cache(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists)
{

    GLuint list, base;
    GLint i;
    GLuint temp;



    base = gc->state.list.listBase;

    switch (type) {
    case GL_BYTE:
        {
            GLbyte *p = (GLbyte *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *p = (GLubyte *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_SHORT:
        {
            GLshort *p = (GLshort *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *p = (GLushort *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_INT:
        {
            GLint *p = (GLint *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_UNSIGNED_INT:
        {
            GLuint *p = (GLuint *)lists;
            for (i = 0; i < n; i++) {
                list = (*p++) + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    case GL_FLOAT:
        {
            GLfloat *p = (GLfloat *)lists;
            for (i = 0; i < n; i++) {
                list = (GLuint)(*p++) + base;
                __glim_CallList_Cache(gc, list);
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
                list = temp + base;
                __glim_CallList_Cache(gc, list);
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
                list = temp + base;
                __glim_CallList_Cache(gc, list);
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
                list = temp + base;
                __glim_CallList_Cache(gc, list);
            }
        }
        break;
    default:
        __glSetError(gc ,GL_INVALID_ENUM);
        break;
    }
}
