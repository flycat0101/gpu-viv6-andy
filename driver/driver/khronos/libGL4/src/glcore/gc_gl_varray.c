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


#include "gc_es_context.h"
#include "gc_es_object_inline.c"
#include "api/gc_gl_api_inline.c"

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE

extern GLboolean internalEndTable[];

extern void __glValidateImmedBegin(__GLcontext *gc, GLenum mode);
extern void __glGenerateVertexIndex(__GLcontext *gc);


__GL_INLINE GLuint __glBegin_ImmedDrawArrays(__GLcontext *gc, GLenum mode, GLsizei count, GLuint64 elemSequence)
{
    GLint lastIndex;
    GLuint retStatus = __GL_DRAWARRAYS_NEW_BEGIN;

    switch (gc->input.beginMode)
    {
    case __GL_NOT_IN_BEGIN:

        if (gc->input.deferredAttribDirty) {
            __glCopyDeferedAttribToCurrent(gc);
        }
        break;

    case __GL_IN_BEGIN:

        __glSetError(gc, GL_INVALID_OPERATION);
        return retStatus;

    case __GL_SMALL_LIST_BATCH:

        __glDisplayListBatchEnd(gc);
        break;

    case __GL_SMALL_DRAW_BATCH:

        if (gc->input.deferredAttribDirty)
        {
            /* If there are defered attribute changes, we have to flush the vertex buffer
            ** and then copy the defered attribute states to current attribute state.
            */
            if (gc->input.deferredAttribDirty & (__GL_DEFERED_ATTRIB_BIT | __GL_DEFERED_COLOR_MASK_BIT))
            {
                __glPrimitiveBatchEnd(gc);
                __glUpdateDeferedAttributes(gc);
                goto New_Begin;
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT &&
                !(gc->input.primitiveFormat & __GL_N3F_BIT))
            {
                /* If previous primitive has no normal (but needs it) in glBegin/glEnd
                ** and normal is really changed after glEnd then the verex buffer has to be
                ** flushed before the current normal is set.
                */
                if (gc->state.current.normal.f.x != gc->input.shadowCurrent.normal.f.x ||
                    gc->state.current.normal.f.y != gc->input.shadowCurrent.normal.f.y ||
                    gc->state.current.normal.f.z != gc->input.shadowCurrent.normal.f.z)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);
            }

            if (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT &&
                !(gc->input.primitiveFormat & (__GL_C3F_BIT | __GL_C4F_BIT | __GL_C4UB_BIT)))
            {
                /* If previous primitive has no color (but needs it) in glBegin/glEnd
                ** and color is really changed after glEnd then the verex buffer has to be
                ** flushed before the current color is set.
                */
                if (gc->state.current.color.r != gc->input.shadowCurrent.color.r ||
                    gc->state.current.color.g != gc->input.shadowCurrent.color.g ||
                    gc->state.current.color.b != gc->input.shadowCurrent.color.b ||
                    gc->state.current.color.a != gc->input.shadowCurrent.color.a)
                {
                    __glPrimitiveBatchEnd(gc);
                    goto New_Begin;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);
            }
        }

        /* Since only selected vertex array formats (at most 4 attributes per vertex) go to
        ** immediate mode path, we don't need to worry about__GL_DEFAULT_VERTEX_BUFFER_SIZE here.
        */
        if ((gc->input.primElemSequence == elemSequence) &&
            (gc->input.vertex.index + count) <= __GL_MAX_VERTEX_NUMBER)
        {
            if (gc->input.primMode == mode) {
                retStatus = __GL_DRAWARRAYS_CONT_BEGIN;
                goto Cont_Begin;
            }

            switch (gc->input.primMode)
            {
            case GL_TRIANGLE_STRIP:
            case GL_TRIANGLE_FAN:
            case GL_QUAD_STRIP:
            case GL_POLYGON:
                if (mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill) {
                    retStatus = __GL_DRAWARRAYS_CONT_BEGIN;
                    goto Cont_Begin;
                }
                break;
            case GL_TRIANGLES:
            case GL_QUADS:
                if (gc->input.vertex.index < 1000 && gc->input.indexBuffer &&
                    mode >= GL_TRIANGLES && gc->state.polygon.bothFaceFill)
                {
                    gc->input.currentPrimMode = gc->input.primMode;
                    gc->input.indexPrimEnabled = GL_TRUE;
                    lastIndex = gc->input.lastVertexIndex;
                    gc->input.lastVertexIndex = 0;
                    __glGenerateVertexIndex(gc);
                    gc->input.lastVertexIndex = lastIndex;
                    gc->input.primMode = GL_TRIANGLE_STRIP;

                    retStatus = __GL_DRAWARRAYS_CONT_BEGIN;
                    goto Cont_Begin;
                }
                break;
            case GL_LINE_STRIP:
            case GL_LINE_LOOP:
                if (mode >= GL_LINES && mode <= GL_LINE_STRIP) {
                    retStatus = __GL_DRAWARRAYS_CONT_BEGIN;
                    goto Cont_Begin;
                }
                break;
            case GL_LINES:
                if (gc->input.vertex.index < 1000 && gc->input.indexBuffer &&
                    mode >= GL_LINE_LOOP && mode <= GL_LINE_STRIP)
                {
                    gc->input.currentPrimMode = GL_LINES;
                    gc->input.indexPrimEnabled = GL_TRUE;
                    lastIndex = gc->input.lastVertexIndex;
                    gc->input.lastVertexIndex = 0;
                    __glGenerateVertexIndex(gc);
                    gc->input.lastVertexIndex = lastIndex;
                    gc->input.primMode = GL_LINE_STRIP;

                    retStatus = __GL_DRAWARRAYS_CONT_BEGIN;
                    goto Cont_Begin;
                }
                break;
            }
        }

        __glPrimitiveBatchEnd(gc);

        break;

    default:

        GL_ASSERT(0);
    }

New_Begin:

    if (gc->input.cacheBufferUsed) {
        retStatus = __GL_DRAWARRAYS_CACHE;
        return retStatus;
    }

    __glValidateImmedBegin(gc, mode);

    gc->input.primMode = mode;

Cont_Begin:

    gc->input.currentPrimMode = mode;
    gc->input.beginMode = __GL_IN_BEGIN;

    return retStatus;
}

__GL_INLINE void __glEnd_ImmedDrawArrays(__GLcontext *gc)
{
    __GLvertexInfo *vtxinfo;

    /* Save the __GL_DRAWARRAYS_END_TAG in gc->input.vertexInfoBuffer.
    */
    vtxinfo = gc->input.currentInfoBufPtr++;
    vtxinfo->inputTag = __GL_DRAWARRAYS_END_TAG;
    vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
    vtxinfo->appDataPtr = NULL;
    vtxinfo->ptePointer = NULL;

    if (gc->input.indexPrimEnabled) {
        __glGenerateVertexIndex(gc);
    }
    else {
        if (internalEndTable[gc->input.primMode]) {
            gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
            __glPrimitiveBatchEnd(gc);
        }
    }

    gc->input.lastVertexIndex = gc->input.vertex.index;

    gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
}

/*********************** DrawArrays *************************************************************/

GLvoid APIENTRY __glImmedDrawArrays_V2F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_Color_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint64 elemSequence;
    GLuint retStatus;

    if (first < 0 || count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    /* Clear __GL_DEFERED_COLOR_BIT to avoid copying the shadow color to current color */
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);

    /* Set shadowAttribUsage mask to indicate the preferance to use *_Color_V3F function */
    gc->input.shadowAttribUsage |= __GL_DEFERED_COLOR_BIT;

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_C4F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawArrays_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawArrays)(gc, mode, first, count);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 7;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_C4F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_DIFFUSE | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.color.offsetDW = 0;
        gc->input.color.sizeDW = 4;
        gc->input.color.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.color.pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.vertex.offsetDW = 4;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 4;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 4);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pColor, *pVertex, *buf;
        __GLvertexInfo *vtxinfo;
        GLuint64 *pteVerPtr, *lastPteVerPtr;
        GLint i;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->first = first;
        vtxinfo->count = count;

        lastPteVerPtr = NULL;

        for (i = first; i < first + count; i++)
        {
            pColor = (GLfloat *)&gc->input.shadowCurrent.color;
            gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.color.currentPtrDW;
            buf[0] = pColor[0];
            buf[1] = pColor[1];
            buf[2] = pColor[2];
            buf[3] = pColor[3];

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + i * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            if (pteVerPtr != lastPteVerPtr)
            {
                lastPteVerPtr = pteVerPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_C4F_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = first + count - i - 1;
                lastPteVerPtr = NULL;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawArrays_Color_V3F_Cache(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo, *beginInfo;
    GLuint pteStatus;
    GLuint *buf, *col;
    GLint remfirst, remcount;

    CHECK_VERTEX_COUNT();

    remfirst = first;
    remcount = count;

    /* Set shadowAttribUsage mask to indicate the preferance to use *_Color_V3F function */
    gc->input.shadowAttribUsage |= __GL_DEFERED_COLOR_BIT;
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawArrays_Color_V3F(gc, mode, first, count);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->first == first && vtxinfo->count == count)
    {
        beginInfo = vtxinfo;
        vtxinfo++;
    }
    else goto SwitchToDefaultBuffer;

    buf = (GLuint *)gc->pVertexDataBufPtr + beginInfo->offsetDW;
    col = (GLuint *)(&gc->input.shadowCurrent.color);

    /* Check if color in shadowCurrent is the same as cached color data */
    if (((col[0] ^ buf[0]) | (col[1] ^ buf[1]) | (col[2] ^ buf[2]) | (col[3] ^ buf[3])) != 0)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data stride and pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_C4F_V3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->vertex.stride ||
        vtxinfo->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_C4F_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else
                goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            remfirst = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            if (vtxinfo->inputTag != __GL_ARRAY_C4F_V3F_TAG)
            {
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawArrays_Color_V3F(gc, mode, remfirst, remcount );
}

GLvoid APIENTRY __glImmedDrawArrays_Normal_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint64 elemSequence;
    GLuint retStatus;

    if (first < 0 || count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    /* Clear __GL_DEFERED_NORMAL_BIT to avoid copying the shadow normal to current normal */
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);

    /* Set shadowAttribUsage mask to indicate the preferance to use *_Normal_V3F function */
    gc->input.shadowAttribUsage |= __GL_DEFERED_NORMAL_BIT;

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_N3F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawArrays_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawArrays)(gc, mode, first, count);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 6;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_N3F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_NORMAL | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.normal.offsetDW = 0;
        gc->input.normal.sizeDW = 3;
        gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.normal.pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.vertex.offsetDW = 3;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 3;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 3);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pNormal, *pVertex, *buf;
        __GLvertexInfo *vtxinfo;
        GLuint64 *pteVerPtr, *lastPteVerPtr;
        GLint i;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->first = first;
        vtxinfo->count = count;

        lastPteVerPtr = NULL;

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = first; i < first + count; i++)
        {
            pNormal = (GLfloat *)&gc->input.shadowCurrent.normal;
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.normal.currentPtrDW;
            buf[0] = pNormal[0];
            buf[1] = pNormal[1];
            buf[2] = pNormal[2];

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + i * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            if (pteVerPtr != lastPteVerPtr)
            {
                lastPteVerPtr = pteVerPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_N3F_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = first + count - i - 1;
                lastPteVerPtr = NULL;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawArrays_Normal_V3F_Cache(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo, *beginInfo;
    GLuint pteStatus;
    GLuint *buf, *nor;
    GLint remfirst, remcount;

    CHECK_VERTEX_COUNT();

    remfirst = first;
    remcount = count;

    /* Set shadowAttribUsage mask to indicate the preferance to use *_Normal_V3F function */
    gc->input.shadowAttribUsage |= __GL_DEFERED_NORMAL_BIT;
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawArrays_Normal_V3F(gc, mode, first, count);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->first == first && vtxinfo->count == count)
    {
        beginInfo = vtxinfo;
        vtxinfo++;
    }
    else goto SwitchToDefaultBuffer;

    buf = (GLuint *)gc->pVertexDataBufPtr + beginInfo->offsetDW;
    nor = (GLuint *)(&gc->input.shadowCurrent.normal);

    /* Check if normal in shadowCurrent is the same as cached normal data */
    if (((nor[0] ^ buf[0]) | (nor[1] ^ buf[1]) | (nor[2] ^ buf[2])) != 0)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data stride and pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_N3F_V3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->vertex.stride ||
        vtxinfo->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_N3F_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            remfirst = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            if (vtxinfo->inputTag != __GL_ARRAY_N3F_V3F_TAG)
            {
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawArrays_Normal_V3F(gc, mode, remfirst, remcount);
}

GLvoid APIENTRY __glImmedDrawArrays_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint64 elemSequence;
    GLuint retStatus;

    if (first < 0 || count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawArrays_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawArrays)(gc, mode, first, count);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 3;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.vertex.offsetDW = 0;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pVertex, *buf;
        GLuint64 *pteVerPtr, *lastPteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->first = first;
        vtxinfo->count = count;

        lastPteVerPtr = NULL;

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = first; i < first + count; i++)
        {
            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + i * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            if (pteVerPtr != lastPteVerPtr)
            {
                lastPteVerPtr = pteVerPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = first + count - i - 1;
                lastPteVerPtr = NULL;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawArrays_V3F_Cache(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint remfirst, remcount;

    CHECK_VERTEX_COUNT();

    remfirst = first;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawArrays_V3F(gc, mode, first, count);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->first == first && vtxinfo->count == count)
    {
        vtxinfo++;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data stride and pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_V3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->vertex.stride ||
        vtxinfo->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            remfirst = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            if (vtxinfo->inputTag != __GL_ARRAY_V3F_TAG)
            {
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawArrays_V3F(gc, mode, remfirst, remcount);
}

GLvoid APIENTRY __glImmedDrawArrays_V3F_Select(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint selectMask;

    selectMask = (gc->input.deferredAttribDirty | gc->input.shadowAttribUsage);
    selectMask &= (gc->input.requiredInputMask & __GL_DEFERED_NORCOL_MASK);

    if (gc->input.cacheBufferUsed)
    {
        switch (selectMask)
        {
        case __GL_DEFERED_NORMAL_BIT:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_Normal_V3F_Cache;
            break;
        case __GL_DEFERED_COLOR_BIT:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_Color_V3F_Cache;
            break;
        default:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_V3F_Cache;
            break;
        }
    }
    else
    {
        switch (selectMask)
        {
        case __GL_DEFERED_NORMAL_BIT:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_Normal_V3F;
            break;
        case __GL_DEFERED_COLOR_BIT:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_Color_V3F;
            break;
        default:
            gc->vertexArray.drawArraysFunc = __glImmedDrawArrays_V3F;
            break;
        }
    }

    gc->immedModeOutsideDispatch.DrawArrays = gc->vertexArray.drawArraysFunc;

    (*gc->immedModeOutsideDispatch.DrawArrays)(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_C4UB_V2F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_C4UB_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_C3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_N3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint64 elemSequence;
    GLuint retStatus;

    if (first < 0 || count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_N3F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawArrays_*_Cache if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawArrays)(gc, mode, first, count);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 6;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_N3F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_NORMAL | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.normal.offsetDW = 0;
        gc->input.normal.sizeDW = 3;
        gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.normal.pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.vertex.offsetDW = 3;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 3;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 3);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pNormal = NULL, *pVertex, *buf;
        GLuint64 *pteNorPtr, *pteVerPtr, *lastPteNorPtr, *lastPteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->first = first;
        vtxinfo->count = count;

        lastPteNorPtr = lastPteVerPtr = NULL;

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = first; i < first + count; i++)
        {
            pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + i * pV->normal.stride);
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.normal.currentPtrDW;
            buf[0] = pNormal[0];
            buf[1] = pNormal[1];
            buf[2] = pNormal[2];
            pteNorPtr = __glGetPageTableEntryPointer(gc, pNormal);
            if (pteNorPtr != lastPteNorPtr)
            {
                lastPteNorPtr = pteNorPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_N3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->normal.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->normal.pointer;
                vtxinfo->ptePointer = pteNorPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_NORMAL_INDEX);
            }

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + i * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            if (pteVerPtr != lastPteVerPtr)
            {
                lastPteVerPtr = pteVerPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = first + count - i - 1;
                lastPteNorPtr = lastPteVerPtr = NULL;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }

        /* Save the last normal in the shadow current state so that the next *_Normal_V3F
        ** function can get the correct normal.
        */
        if ((gc->input.shadowAttribUsage & __GL_DEFERED_NORMAL_BIT) && (pNormal == NULL))
        {
            pNormal = (GLfloat *)((GLubyte *)pNormal - pV->normal.stride);
            gc->input.shadowCurrent.normal.f.x = pNormal[0];
            gc->input.shadowCurrent.normal.f.y = pNormal[1];
            gc->input.shadowCurrent.normal.f.z = pNormal[2];
            gc->input.shadowCurrent.normal.f.w = 1.0;
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawArrays_N3F_V3F_Cache(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint remfirst, remcount;

    CHECK_VERTEX_COUNT();

    remfirst = first;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawArrays_N3F_V3F(gc, mode, first, count);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->first == first && vtxinfo->count == count)
    {
        vtxinfo++;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data stride and pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_N3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->normal.stride ||
        vtxinfo->appDataPtr != pV->normal.pointer ||
        (vtxinfo+1)->inputTag != __GL_ARRAY_V3F_TAG ||
        (vtxinfo+1)->offsetDW != (GLuint)pV->vertex.stride ||
        (vtxinfo+1)->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_N3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            remfirst = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            if (vtxinfo->inputTag != __GL_ARRAY_N3F_TAG ||
                (vtxinfo+1)->inputTag != __GL_ARRAY_V3F_TAG)
            {
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawArrays_N3F_V3F(gc, mode, remfirst, remcount);
}

GLvoid APIENTRY __glImmedDrawArrays_C4F_N3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T4F_V4F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_C4UB_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_C3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_N3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    GLuint64 elemSequence;
    GLuint retStatus;

    if (first < 0 || count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_TC2F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_N3F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawArrays_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawArrays)(gc, mode, first, count);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 8;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_TC2F_BIT | __GL_N3F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_TEX0 | __GL_INPUT_NORMAL | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.texture[0].offsetDW = 0;
        gc->input.texture[0].sizeDW = 2;
        gc->input.texture[0].currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.texture[0].pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.normal.offsetDW = 2;
        gc->input.normal.sizeDW = 3;
        gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 2;
        gc->input.normal.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 2);
        gc->input.vertex.offsetDW = 5;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 5;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 5);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pTexture, *pNormal = NULL, *pVertex, *buf;
        GLuint64 *pteTexPtr, *pteNorPtr, *pteVerPtr;
        GLuint64 *lastPteTexPtr, *lastPteNorPtr, *lastPteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->first = first;
        vtxinfo->count = count;

        lastPteTexPtr = lastPteNorPtr = lastPteVerPtr = NULL;

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = first; i < first + count; i++)
        {
            pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + i * pV->texture[0].stride);
            gc->input.texture[0].currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.texture[0].currentPtrDW;
            buf[0] = pTexture[0];
            buf[1] = pTexture[1];
            pteTexPtr = __glGetPageTableEntryPointer(gc, pTexture);
            if (pteTexPtr != lastPteTexPtr)
            {
                lastPteTexPtr = pteTexPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_TC2F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->texture[0].stride;
                vtxinfo->appDataPtr = (GLuint *)pV->texture[0].pointer;
                vtxinfo->ptePointer = pteTexPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_TEX0_INDEX);
            }

            pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + i * pV->normal.stride);
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.normal.currentPtrDW;
            buf[0] = pNormal[0];
            buf[1] = pNormal[1];
            buf[2] = pNormal[2];
            pteNorPtr = __glGetPageTableEntryPointer(gc, pNormal);
            if (pteNorPtr != lastPteNorPtr)
            {
                lastPteNorPtr = pteNorPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_N3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->normal.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->normal.pointer;
                vtxinfo->ptePointer = pteNorPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_NORMAL_INDEX);
            }

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + i * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            if (pteVerPtr != lastPteVerPtr)
            {
                lastPteVerPtr = pteVerPtr;
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = first + count - i - 1;
                lastPteTexPtr = lastPteNorPtr = lastPteVerPtr = NULL;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }

        /* Save the last normal in the shadow current state so that the next *_Normal_V3F
        ** function can get the correct normal.
        */
        if ((gc->input.shadowAttribUsage & __GL_DEFERED_NORMAL_BIT) && (pNormal != NULL))
        {
            pNormal = (GLfloat *)((GLubyte *)pNormal - pV->normal.stride);
            gc->input.shadowCurrent.normal.f.x = pNormal[0];
            gc->input.shadowCurrent.normal.f.y = pNormal[1];
            gc->input.shadowCurrent.normal.f.z = pNormal[2];
            gc->input.shadowCurrent.normal.f.w = 1.0;
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_N3F_V3F_Cache(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint remfirst, remcount;

    CHECK_VERTEX_COUNT();

    remfirst = first;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawArrays_T2F_N3F_V3F(gc, mode, first, count);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->first == first && vtxinfo->count == count)
    {
        vtxinfo++;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data stride and pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_TC2F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->texture[0].stride ||
        vtxinfo->appDataPtr != pV->texture[0].pointer ||
        (vtxinfo+1)->inputTag != __GL_ARRAY_N3F_TAG ||
        (vtxinfo+1)->offsetDW != (GLuint)pV->normal.stride ||
        (vtxinfo+1)->appDataPtr != pV->normal.pointer ||
        (vtxinfo+2)->inputTag != __GL_ARRAY_V3F_TAG ||
        (vtxinfo+2)->offsetDW != (GLuint)pV->vertex.stride ||
        (vtxinfo+2)->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_TC2F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_N3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            remfirst = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            if (vtxinfo->inputTag != __GL_ARRAY_TC2F_TAG ||
                (vtxinfo+1)->inputTag != __GL_ARRAY_N3F_TAG ||
                (vtxinfo+2)->inputTag != __GL_ARRAY_V3F_TAG)
            {
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawArrays_T2F_N3F_V3F(gc, mode, remfirst, remcount);
}

GLvoid APIENTRY __glImmedDrawArrays_T2F_C4F_N3F_V3F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

GLvoid APIENTRY __glImmedDrawArrays_T4F_C4F_N3F_V4F(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __glim_DrawArrays(gc, mode, first, count);
}

/*********************** DrawElements *************************************************************/

GLvoid APIENTRY __glImmedDrawElements_V2F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    GLuint64 elemSequence;
    GLuint retStatus;
    GLubyte *indexEnd, *indexPtr;

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            indexEnd = (GLubyte *)indices + count * sizeof(GLubyte);
            break;
        case GL_UNSIGNED_SHORT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLushort);
            break;
        case GL_UNSIGNED_INT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLuint);
            break;
        default:
            __glSetError(gc, GL_INVALID_ENUM);
            return;
    }
    if (count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawElements_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawElements)(gc, mode, count, type, indices);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 3;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.vertex.offsetDW = 0;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.vertex.pointer = (GLubyte*)gc->input.currentDataBufPtr;
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pVertex, *buf;
        GLuint64 *pteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i, index = 0, pteExist;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)indices;
        vtxinfo->count = count;

        /* Save __GLvertexInfo of "indices" array in gc->input.vertexInfoBuffer */
        indexPtr = (GLubyte *)indices;
        while (indexPtr < indexEnd)
        {
            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_ARRAY_INDEX_TAG;
            vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)indexPtr;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, (GLfloat *)indexPtr);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_WEIGHT_INDEX);
            indexPtr += __GL_PAGE_SIZE;
        }

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = 0; i < count; i++)
        {
            switch (type)
            {
            case GL_UNSIGNED_BYTE:
                index = ((GLubyte *)indices)[i];
                break;
            case GL_UNSIGNED_SHORT:
                index = ((GLushort *)indices)[i];
                break;
            case GL_UNSIGNED_INT:
                index = ((GLuint *)indices)[i];
                break;
            }
            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + index * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            pteExist = __glPteInfoExistInHashTable(gc, pteVerPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, pteVerPtr, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = count - i;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glClearPteInfoHashTable(gc, &gc->input.tempPteInfo, 0);
    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawElements_V3F_Cache(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint nextidx, remcount;
    GLvoid *remidxs = NULL;

    CHECK_VERTEX_COUNT();

    nextidx = 0;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawElements_V3F(gc, mode, count, type, indices);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->appDataPtr == indices && vtxinfo->count == count)
    {
        vtxinfo++;
        if (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
        {
            /* Check if "indices" array is changed */
            while (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
            {
                /* If the page is valid and the page dirty bit is not set */
                pteStatus = (GLuint)(*vtxinfo->ptePointer);
                if (__GL_PTE_NOT_DIRTY(pteStatus))
                {
                    /* Then index array on this page has not been changed */
                    vtxinfo++;
                }
                else goto SwitchToDefaultBuffer;
            }
        }
        else goto SwitchToDefaultBuffer;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_V3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->vertex.stride ||
        vtxinfo->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            nextidx = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            switch (vtxinfo->inputTag)
            {
            case __GL_ARRAY_V3F_TAG:
            case __GL_BATCH_END_TAG:
            case __GL_DRAWARRAYS_END_TAG:
                break;
            default:
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            remidxs = &((GLubyte *)indices)[nextidx];
            break;
        case GL_UNSIGNED_SHORT:
            remidxs = &((GLushort *)indices)[nextidx];
            break;
        case GL_UNSIGNED_INT:
            remidxs = &((GLuint *)indices)[nextidx];
            break;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawElements_V3F(gc, mode, remcount, type, remidxs);
}

GLvoid APIENTRY __glImmedDrawElements_C4UB_V2F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_C4UB_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_C3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_N3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    GLuint64 elemSequence;
    GLuint retStatus;
    GLubyte *indexEnd, *indexPtr;

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            indexEnd = (GLubyte *)indices + count * sizeof(GLubyte);
            break;
        case GL_UNSIGNED_SHORT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLushort);
            break;
        case GL_UNSIGNED_INT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLuint);
            break;
        default:
            __glSetError(gc, GL_INVALID_ENUM);
            return;
    }
    if (count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_N3F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawElements_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawElements)(gc, mode, count, type, indices);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 6;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_N3F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_NORMAL | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.normal.offsetDW = 0;
        gc->input.normal.sizeDW = 3;
        gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.normal.pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.vertex.offsetDW = 3;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 3;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 3);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pNormal, *pVertex, *buf;
        GLuint64 *pteNorPtr, *pteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i, index = 0, pteExist;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)indices;
        vtxinfo->count = count;

        /* Save __GLvertexInfo of "indices" array in gc->input.vertexInfoBuffer */
        indexPtr = (GLubyte *)indices;
        while (indexPtr < indexEnd)
        {
            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_ARRAY_INDEX_TAG;
            vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)indexPtr;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, (GLfloat *)indexPtr);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_WEIGHT_INDEX);
            indexPtr += __GL_PAGE_SIZE;
        }

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = 0; i < count; i++)
        {
            switch (type)
            {
            case GL_UNSIGNED_BYTE:
                index = ((GLubyte *)indices)[i];
                break;
            case GL_UNSIGNED_SHORT:
                index = ((GLushort *)indices)[i];
                break;
            case GL_UNSIGNED_INT:
                index = ((GLuint *)indices)[i];
                break;
            }

            pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + index * pV->normal.stride);
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.normal.currentPtrDW;
            buf[0] = pNormal[0];
            buf[1] = pNormal[1];
            buf[2] = pNormal[2];
            pteNorPtr = __glGetPageTableEntryPointer(gc, pNormal);
            pteExist = __glPteInfoExistInHashTable(gc, pteNorPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_N3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->normal.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->normal.pointer;
                vtxinfo->ptePointer = pteNorPtr;
                __glClearPageTableEntryDirty(gc, pteNorPtr, __GL_INPUT_NORMAL_INDEX);
            }

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + index * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            pteExist = __glPteInfoExistInHashTable(gc, pteVerPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, pteVerPtr, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = count - i;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glClearPteInfoHashTable(gc, &gc->input.tempPteInfo, 0);
    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawElements_N3F_V3F_Cache(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint nextidx, remcount;
    GLvoid *remidxs = NULL;

    CHECK_VERTEX_COUNT();

    nextidx = 0;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawElements_N3F_V3F(gc, mode, count, type, indices);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->appDataPtr == indices && vtxinfo->count == count)
    {
        vtxinfo++;
        if (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
        {
            /* Check if "indices" array is changed */
            while (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
            {
                /* If the page is valid and the page dirty bit is not set */
                pteStatus = (GLuint)(*vtxinfo->ptePointer);
                if (__GL_PTE_NOT_DIRTY(pteStatus))
                {
                    /* Then index array on this page has not been changed */
                    vtxinfo++;
                }
                else goto SwitchToDefaultBuffer;
            }
        }
        else goto SwitchToDefaultBuffer;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_N3F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->normal.stride ||
        vtxinfo->appDataPtr != pV->normal.pointer ||
        (vtxinfo+1)->inputTag != __GL_ARRAY_V3F_TAG ||
        (vtxinfo+1)->offsetDW != (GLuint)pV->vertex.stride ||
        (vtxinfo+1)->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_N3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            nextidx = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            switch (vtxinfo->inputTag)
            {
            case __GL_ARRAY_N3F_TAG:
            case __GL_ARRAY_V3F_TAG:
            case __GL_BATCH_END_TAG:
            case __GL_DRAWARRAYS_END_TAG:
                break;
            default:
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            remidxs = &((GLubyte *)indices)[nextidx];
            break;
        case GL_UNSIGNED_SHORT:
            remidxs = &((GLushort *)indices)[nextidx];
            break;
        case GL_UNSIGNED_INT:
            remidxs = &((GLuint *)indices)[nextidx];
            break;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawElements_N3F_V3F(gc, mode, remcount, type, remidxs);
}

GLvoid APIENTRY __glImmedDrawElements_C4F_N3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T4F_V4F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_C4UB_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_C3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_N3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    GLuint64 elemSequence;
    GLuint retStatus;
    GLubyte *indexEnd, *indexPtr;

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            indexEnd = (GLubyte *)indices + count * sizeof(GLubyte);
            break;
        case GL_UNSIGNED_SHORT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLushort);
            break;
        case GL_UNSIGNED_INT:
            indexEnd = (GLubyte *)indices + count * sizeof(GLuint);
            break;
        default:
            __glSetError(gc, GL_INVALID_ENUM);
            return;
    }
    if (count < 0)
    {
        __glSetError(gc, GL_INVALID_VALUE);
        return;
    }
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT)
    {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    CHECK_VERTEX_COUNT();

    elemSequence = 0;
    __GL_PRIM_ELEMENT(elemSequence, __GL_TC2F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_N3F_TAG);
    __GL_PRIM_ELEMENT(elemSequence, __GL_V3F_TAG);

    retStatus = __glBegin_ImmedDrawArrays(gc, mode, count, elemSequence);

    if (retStatus == __GL_DRAWARRAYS_CACHE)
    {
        /* Jump to __glImmedDrawElements_*_Cache function if cache buffer is used */
        (*gc->currentImmediateDispatch->DrawElements)(gc, mode, count, type, indices);

        return;
    }
    else if (retStatus == __GL_DRAWARRAYS_NEW_BEGIN)
    {
        gc->input.vertTotalStrideDW = 8;
        gc->input.preVertexFormat = gc->input.primitiveFormat = (__GL_TC2F_BIT | __GL_N3F_BIT | __GL_V3F_BIT);
        gc->input.primInputMask = (__GL_INPUT_TEX0 | __GL_INPUT_NORMAL | __GL_INPUT_VERTEX);
        gc->input.primElemSequence = elemSequence;
        gc->input.texture[0].offsetDW = 0;
        gc->input.texture[0].sizeDW = 2;
        gc->input.texture[0].currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW;
        gc->input.texture[0].pointer = (GLubyte*)gc->input.currentDataBufPtr;
        gc->input.normal.offsetDW = 2;
        gc->input.normal.sizeDW = 3;
        gc->input.normal.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 2;
        gc->input.normal.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 2);
        gc->input.vertex.offsetDW = 5;
        gc->input.vertex.sizeDW = 3;
        gc->input.vertex.currentPtrDW = gc->input.currentDataBufPtr - gc->input.vertTotalStrideDW + 5;
        gc->input.vertex.pointer = (GLubyte*)(gc->input.currentDataBufPtr + 5);
    }

    {
        __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
        GLfloat *pTexture, *pNormal, *pVertex, *buf;
        GLuint64 *pteTexPtr, *pteNorPtr, *pteVerPtr;
        __GLvertexInfo *vtxinfo;
        GLint i, index = 0, pteExist;

        /* Save __GL_DRAWARRAYS_*_TAG in gc->input.vertexInfoBuffer.
        */
        vtxinfo = gc->input.currentInfoBufPtr++;
        vtxinfo->inputTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
        vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
        vtxinfo->appDataPtr = (GLuint *)indices;
        vtxinfo->count = count;

        /* Save __GLvertexInfo of "indices" array in gc->input.vertexInfoBuffer */
        indexPtr = (GLubyte *)indices;
        while (indexPtr < indexEnd)
        {
            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_ARRAY_INDEX_TAG;
            vtxinfo->offsetDW = (GLushort)(gc->input.currentDataBufPtr - gc->input.vertexDataBuffer);
            vtxinfo->appDataPtr = (GLuint *)indexPtr;
            vtxinfo->ptePointer = __glGetPageTableEntryPointer(gc, (GLfloat *)indexPtr);
            __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_WEIGHT_INDEX);
            indexPtr += __GL_PAGE_SIZE;
        }

        /* Copy vertex array data into the immediate mode vertex buffer */
        for (i = 0; i < count; i++)
        {
            switch (type)
            {
            case GL_UNSIGNED_BYTE:
                index = ((GLubyte *)indices)[i];
                break;
            case GL_UNSIGNED_SHORT:
                index = ((GLushort *)indices)[i];
                break;
            case GL_UNSIGNED_INT:
                index = ((GLuint *)indices)[i];
                break;
            }

            pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + index * pV->texture[0].stride);
            gc->input.texture[0].currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.texture[0].currentPtrDW;
            buf[0] = pTexture[0];
            buf[1] = pTexture[1];
            pteTexPtr = __glGetPageTableEntryPointer(gc, pTexture);
            pteExist = __glPteInfoExistInHashTable(gc, pteTexPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_TC2F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->texture[0].stride;
                vtxinfo->appDataPtr = (GLuint *)pV->texture[0].pointer;
                vtxinfo->ptePointer = pteTexPtr;
                __glClearPageTableEntryDirty(gc, vtxinfo->ptePointer, __GL_INPUT_TEX0_INDEX);
            }

            pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + index * pV->normal.stride);
            gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.normal.currentPtrDW;
            buf[0] = pNormal[0];
            buf[1] = pNormal[1];
            buf[2] = pNormal[2];
            pteNorPtr = __glGetPageTableEntryPointer(gc, pNormal);
            pteExist = __glPteInfoExistInHashTable(gc, pteNorPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_N3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->normal.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->normal.pointer;
                vtxinfo->ptePointer = pteNorPtr;
                __glClearPageTableEntryDirty(gc, pteNorPtr, __GL_INPUT_NORMAL_INDEX);
            }

            pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + index * pV->vertex.stride);
            gc->input.vertex.currentPtrDW += gc->input.vertTotalStrideDW;
            buf = gc->input.vertex.currentPtrDW;
            buf[0] = pVertex[0];
            buf[1] = pVertex[1];
            buf[2] = pVertex[2];
            pteVerPtr = __glGetPageTableEntryPointer(gc, pVertex);
            pteExist = __glPteInfoExistInHashTable(gc, pteVerPtr, &gc->input.tempPteInfo);
            if (pteExist == GL_FALSE) {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_ARRAY_V3F_TAG;
                vtxinfo->offsetDW = (GLushort)pV->vertex.stride;
                vtxinfo->appDataPtr = (GLuint *)pV->vertex.pointer;
                vtxinfo->ptePointer = pteVerPtr;
                __glClearPageTableEntryDirty(gc, pteVerPtr, __GL_INPUT_VERTEX_INDEX);
            }

            gc->input.vertex.index++;

            if (gc->input.vertex.index > __GL_MAX_VERTEX_NUMBER)
            {
                vtxinfo = gc->input.currentInfoBufPtr++;
                vtxinfo->inputTag = __GL_BATCH_END_TAG;
                vtxinfo->offsetDW = gc->input.vertex.index * gc->input.vertTotalStrideDW;
                vtxinfo->first = i + 1;
                vtxinfo->count = count - i;

                gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
                __glImmediateFlushBuffer(gc);
                gc->input.beginMode = __GL_IN_BEGIN;
            }
        }
    }

    gc->input.currentDataBufPtr = gc->input.vertexDataBuffer + gc->input.vertex.index * gc->input.vertTotalStrideDW;

    __glClearPteInfoHashTable(gc, &gc->input.tempPteInfo, 0);
    __glEnd_ImmedDrawArrays(gc);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_N3F_V3F_Cache(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint drawArraysTag = (mode | __GL_DRAWARRAYS_TAG_MASK);
    __GLvertexInfo *vtxinfo;
    GLuint pteStatus;
    GLint nextidx, remcount;
    GLvoid *remidxs = NULL;

    CHECK_VERTEX_COUNT();

    nextidx = 0;
    remcount = count;

    __glBegin_ImmedCache_Check(gc, mode);

    if (!gc->input.cacheBufferUsed) {
        __glImmedDrawElements_T2F_N3F_V3F(gc, mode, count, type, indices);
        return;
    }

    vtxinfo = gc->pCurrentInfoBufPtr;
    if (vtxinfo->inputTag == drawArraysTag &&
        vtxinfo->appDataPtr == indices && vtxinfo->count == count)
    {
        vtxinfo++;
        if (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
        {
            /* Check if "indices" array is changed */
            while (vtxinfo->inputTag == __GL_ARRAY_INDEX_TAG)
            {
                /* If the page is valid and the page dirty bit is not set */
                pteStatus = (GLuint)(*vtxinfo->ptePointer);
                if (__GL_PTE_NOT_DIRTY(pteStatus))
                {
                    /* Then index array on this page has not been changed */
                    vtxinfo++;
                }
                else goto SwitchToDefaultBuffer;
            }
        }
        else goto SwitchToDefaultBuffer;
    }
    else goto SwitchToDefaultBuffer;

    /* Check if vertex array data pointers are changed */
    if (vtxinfo->inputTag != __GL_ARRAY_TC2F_TAG ||
        vtxinfo->offsetDW != (GLuint)pV->texture[0].stride ||
        vtxinfo->appDataPtr != pV->texture[0].pointer ||
        (vtxinfo+1)->inputTag != __GL_ARRAY_N3F_TAG ||
        (vtxinfo+1)->offsetDW != (GLuint)pV->normal.stride ||
        (vtxinfo+1)->appDataPtr != pV->normal.pointer ||
        (vtxinfo+2)->inputTag != __GL_ARRAY_V3F_TAG ||
        (vtxinfo+2)->offsetDW != (GLuint)pV->vertex.stride ||
        (vtxinfo+2)->appDataPtr != pV->vertex.pointer)
    {
        goto SwitchToDefaultBuffer;
    }

    /* Check if vertex array data are changed */
    while (vtxinfo->inputTag != __GL_DRAWARRAYS_END_TAG)
    {
        if (vtxinfo->inputTag == __GL_ARRAY_TC2F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_N3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_ARRAY_V3F_TAG)
        {
            /* If the page is valid and the page dirty bit is not set */
            pteStatus = (GLuint)(*vtxinfo->ptePointer);
            if (__GL_PTE_NOT_DIRTY(pteStatus))
            {
                /* Then application data on this page has not been changed */
                vtxinfo++;
            }
            else goto SwitchToDefaultBuffer;
        }

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            /* We assume there is no TRIANGLE_FAN (>8190 vertices) batch connection case */
            nextidx = (GLint)(vtxinfo->first - gc->input.currentVertexCache->connectVertexCount);
            remcount = (GLint)(vtxinfo->count + gc->input.currentVertexCache->connectVertexCount);

            gc->pCurrentInfoBufPtr = vtxinfo;
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
            vtxinfo = gc->pCurrentInfoBufPtr;
            switch (vtxinfo->inputTag)
            {
            case __GL_ARRAY_TC2F_TAG:
            case __GL_ARRAY_N3F_TAG:
            case __GL_ARRAY_V3F_TAG:
            case __GL_BATCH_END_TAG:
            case __GL_DRAWARRAYS_END_TAG:
                break;
            default:
                goto SwitchToDefaultBuffer;
            }
        }
    }

    /* Check if vertex array end tag matches */
    /* if (vtxinfo->inputTag == __GL_DRAWARRAYS_END_TAG) */
    {
        vtxinfo++;
        gc->pCurrentInfoBufPtr = vtxinfo;
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        if (vtxinfo->inputTag == __GL_BATCH_END_TAG)
        {
            __glImmedFlushBuffer_Cache(gc, drawArraysTag);
        }
        return;
    }

SwitchToDefaultBuffer:

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, drawArraysTag);

    if (gc->input.currentDataBufPtr > gc->input.vertexDataBuffer) {
        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;
    }

    switch (type)
    {
        case GL_UNSIGNED_BYTE:
            remidxs = &((GLubyte *)indices)[nextidx];
            break;
        case GL_UNSIGNED_SHORT:
            remidxs = &((GLushort *)indices)[nextidx];
            break;
        case GL_UNSIGNED_INT:
            remidxs = &((GLuint *)indices)[nextidx];
            break;
    }

    /* Wirte vertex data into the default vertex buffer */
    __glImmedDrawElements_T2F_N3F_V3F(gc, mode, remcount, type, remidxs);
}

GLvoid APIENTRY __glImmedDrawElements_T2F_C4F_N3F_V3F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

GLvoid APIENTRY __glImmedDrawElements_T4F_C4F_N3F_V4F(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    __glim_DrawElements(gc, mode, count, type, indices);
}

/*********************** ArrayElement *************************************************************/

GLvoid APIENTRY __glImmedArrayElement_V2F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Vertex2fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_C4UB_V2F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLubyte *pColor = (GLubyte *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Color4ubv)(gc, pColor);
    (*gc->currentImmediateDispatch->Vertex2fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_C4UB_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLubyte *pColor = (GLubyte *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Color4ubv)(gc, pColor);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_C3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Color3fv)(gc, pColor);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_N3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Normal3fv)(gc, pNormal);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_C4F_N3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->Color4fv)(gc, pColor);
    (*gc->currentImmediateDispatch->Normal3fv)(gc, pNormal);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T2F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord2fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T4F_V4F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);
    (*gc->currentImmediateDispatch->TexCoord4fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Vertex4fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T2F_C4UB_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLubyte *pColor = (GLubyte *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord2fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Color4ubv)(gc, pColor);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T2F_C3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord2fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Color3fv)(gc, pColor);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T2F_N3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord2fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Normal3fv)(gc, pNormal);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T2F_C4F_N3F_V3F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord2fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Color4fv)(gc, pColor);
    (*gc->currentImmediateDispatch->Normal3fv)(gc, pNormal);
    (*gc->currentImmediateDispatch->Vertex3fv)(gc, pVertex);
}

GLvoid APIENTRY __glImmedArrayElement_T4F_C4F_N3F_V4F(__GLcontext *gc, GLint element)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLfloat *pTexture = (GLfloat *)((GLubyte *)pV->texture[0].pointer + element * pV->texture[0].stride);
    GLfloat *pColor = (GLfloat *)((GLubyte *)pV->color.pointer + element * pV->color.stride);
    GLfloat *pNormal = (GLfloat *)((GLubyte *)pV->normal.pointer + element * pV->normal.stride);
    GLfloat *pVertex = (GLfloat *)((GLubyte *)pV->vertex.pointer + element * pV->vertex.stride);

    (*gc->currentImmediateDispatch->TexCoord4fv)(gc, pTexture);
    (*gc->currentImmediateDispatch->Color4fv)(gc, pColor);
    (*gc->currentImmediateDispatch->Normal3fv)(gc, pNormal);
    (*gc->currentImmediateDispatch->Vertex4fv)(gc, pVertex);
}


/*      GL_V2F */
/*      GL_V3F */
/*      GL_C4UB_V2F */
/*      GL_C4UB_V3F */
/*      GL_C3F_V3F */
/*      GL_N3F_V3F */
/*      GL_C4F_N3F_V3F */
/*      GL_T2F_V3F */
/*      GL_T4F_V4F */
/*      GL_T2F_C4UB_V3F */
/*      GL_T2F_C3F_V3F */
/*      GL_T2F_N3F_V3F */
/*      GL_T2F_C4F_N3F_V3F */
/*      GL_T4F_C4F_N3F_V4F */

GLvoid __glSelectImmedDrawArraysFn(__GLcontext *gc)
{
    __GLvertexArrayState *arrayState = &gc->vertexArray.boundVAO->vertexArray;
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    vertexArray->drawArraysFunc = __glim_DrawArrays;
    vertexArray->drawElementsFunc = __glim_DrawElements;
    vertexArray->arrayElementFunc = __glim_ArrayElement;

    if (gc->input.cacheBufferUsed)
    {
        switch (arrayState->attribEnabled)
        {
        case __GL_VARRAY_VERTEX:
            if (arrayState->vertex.type == GL_FLOAT)
            {
                if (arrayState->vertex.size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_V2F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_V2F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_V2F;
                }
                else if (arrayState->vertex.size == 3)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_V3F_Select;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_V3F_Cache;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_V3F;
                }
            }
            break;
        case (__GL_VARRAY_DIFFUSE|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT)
            {
                if (arrayState->color.size == 4 && arrayState->color.type == GL_UNSIGNED_BYTE)
                {
                    if (arrayState->vertex.size == 2)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C4UB_V2F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C4UB_V2F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C4UB_V2F;
                    }
                    else if (arrayState->vertex.size == 3)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C4UB_V3F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C4UB_V3F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C4UB_V3F;
                    }
                }
                else if (arrayState->color.size == 3 && arrayState->color.type == GL_FLOAT)
                {
                    if (arrayState->vertex.size == 3)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C3F_V3F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C3F_V3F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C3F_V3F;
                    }
                }
            }
            break;
        case (__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_N3F_V3F_Cache;
                vertexArray->drawElementsFunc = __glImmedDrawElements_N3F_V3F_Cache;
                vertexArray->arrayElementFunc = __glImmedArrayElement_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_DIFFUSE|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->color.type == GL_FLOAT && arrayState->color.size == 4 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_C4F_N3F_V3F;
                vertexArray->drawElementsFunc = __glImmedDrawElements_C4F_N3F_V3F;
                vertexArray->arrayElementFunc = __glImmedArrayElement_C4F_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->texture[0].type == GL_FLOAT)
            {
                if (arrayState->vertex.size == 3 && arrayState->texture[0].size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_V3F;
                }
                else if (arrayState->vertex.size == 4 && arrayState->texture[0].size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T4F_V4F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T4F_V4F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T4F_V4F;
                }
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_DIFFUSE|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2)
            {
                if (arrayState->color.type == GL_UNSIGNED_BYTE && arrayState->color.size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C4UB_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C4UB_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C4UB_V3F;
                }
                else if (arrayState->color.type == GL_FLOAT && arrayState->color.size == 3)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C3F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C3F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C3F_V3F;
                }
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_N3F_V3F_Cache;
                vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_N3F_V3F_Cache;
                vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_DIFFUSE|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->color.type == GL_FLOAT && arrayState->color.size == 4 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                    arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C4F_N3F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C4F_N3F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C4F_N3F_V3F;
                }
                else if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 4 &&
                    arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T4F_C4F_N3F_V4F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T4F_C4F_N3F_V4F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T4F_C4F_N3F_V4F;
                }
            }
            break;
        }
    }
    else
    {
        switch (arrayState->attribEnabled)
        {
        case __GL_VARRAY_VERTEX:
            if (arrayState->vertex.type == GL_FLOAT)
            {
                if (arrayState->vertex.size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_V2F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_V2F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_V2F;
                }
                else if (arrayState->vertex.size == 3)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_V3F_Select;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_V3F;
                }
            }
            break;
        case (__GL_VARRAY_DIFFUSE|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT)
            {
                if (arrayState->color.size == 4 && arrayState->color.type == GL_UNSIGNED_BYTE)
                {
                    if (arrayState->vertex.size == 2)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C4UB_V2F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C4UB_V2F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C4UB_V2F;
                    }
                    else if (arrayState->vertex.size == 3)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C4UB_V3F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C4UB_V3F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C4UB_V3F;
                    }
                }
                else if (arrayState->color.size == 3 && arrayState->color.type == GL_FLOAT)
                {
                    if (arrayState->vertex.size == 3)
                    {
                        vertexArray->drawArraysFunc = __glImmedDrawArrays_C3F_V3F;
                        vertexArray->drawElementsFunc = __glImmedDrawElements_C3F_V3F;
                        vertexArray->arrayElementFunc = __glImmedArrayElement_C3F_V3F;
                    }
                }
            }
            break;
        case (__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_N3F_V3F;
                vertexArray->drawElementsFunc = __glImmedDrawElements_N3F_V3F;
                vertexArray->arrayElementFunc = __glImmedArrayElement_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_DIFFUSE|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->color.type == GL_FLOAT && arrayState->color.size == 4 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_C4F_N3F_V3F;
                vertexArray->drawElementsFunc = __glImmedDrawElements_C4F_N3F_V3F;
                vertexArray->arrayElementFunc = __glImmedArrayElement_C4F_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->texture[0].type == GL_FLOAT)
            {
                if (arrayState->vertex.size == 3 && arrayState->texture[0].size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_V3F;
                }
                else if (arrayState->vertex.size == 4 && arrayState->texture[0].size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T4F_V4F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T4F_V4F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T4F_V4F;
                }
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_DIFFUSE|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2)
            {
                if (arrayState->color.type == GL_UNSIGNED_BYTE && arrayState->color.size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C4UB_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C4UB_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C4UB_V3F;
                }
                else if (arrayState->color.type == GL_FLOAT && arrayState->color.size == 3)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C3F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C3F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C3F_V3F;
                }
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_N3F_V3F;
                vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_N3F_V3F;
                vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_N3F_V3F;
            }
            break;
        case (__GL_VARRAY_TEX0|__GL_VARRAY_DIFFUSE|__GL_VARRAY_NORMAL|__GL_VARRAY_VERTEX):
            if (arrayState->color.type == GL_FLOAT && arrayState->color.size == 4 &&
                arrayState->normal.type == GL_FLOAT && arrayState->normal.size == 3)
            {
                if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 3 &&
                    arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 2)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T2F_C4F_N3F_V3F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T2F_C4F_N3F_V3F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T2F_C4F_N3F_V3F;
                }
                else if (arrayState->vertex.type == GL_FLOAT && arrayState->vertex.size == 4 &&
                    arrayState->texture[0].type == GL_FLOAT && arrayState->texture[0].size == 4)
                {
                    vertexArray->drawArraysFunc = __glImmedDrawArrays_T4F_C4F_N3F_V4F;
                    vertexArray->drawElementsFunc = __glImmedDrawElements_T4F_C4F_N3F_V4F;
                    vertexArray->arrayElementFunc = __glImmedArrayElement_T4F_C4F_N3F_V4F;
                }
            }
            break;
        }
    }

    /*
    ** If there are enabled arrays sourcing data from buffer objects or vertex caching is disabled,
    ** then we do not use immediate draw arrays/elements functions.
    */
    if ((arrayState->arrayInBufObj & arrayState->attribEnabled) ||
        (gc->input.enableVertexCaching == GL_FALSE))
    {
        vertexArray->drawArraysFunc = __glim_DrawArrays;
        vertexArray->drawElementsFunc = __glim_DrawElements;
    }
}
