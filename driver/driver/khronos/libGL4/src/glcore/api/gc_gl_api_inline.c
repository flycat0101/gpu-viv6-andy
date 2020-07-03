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


#ifdef OPENGL40
#include "gc_es_context.h"

extern GLuint fmtIndex2InputIndex[];

#if defined(_WIN32)
#pragma warning( disable : 4305)
#endif
/*
 * Will make them configurable later
 */
#define pageDirectoryMask 0xffe00000
#define pageDirectoryShift 21
#define pageTableMask 0x1ff000
#define pageTableShift 12
#define pageOffsetFactor 2

#define __GL_PDE_OFFSET(linearAddr)     (((((GLuint64)(linearAddr)) & pageDirectoryMask) >> pageDirectoryShift))
#define __GL_PAGE_DIR_ENTRY(gc, linearAddr) (GLuint64*)(*(gc->pageDirectoryBase + __GL_PDE_OFFSET(linearAddr)))
#define __GL_PTE_OFFSET(linearAddr)     (((((GLuint64)(linearAddr)) & pageTableMask) >> pageTableShift) * pageOffsetFactor)
#define __GL_PTE_NOT_DIRTY(pteStatus)   ((((pteStatus) ^ 0x5) & 0x45) == 0x0)
#define __GL_PAGE_SIZE                  0x1000; /* 4KB */

__GL_INLINE GLboolean __glPteInfoExistInHashTable(__GLcontext *gc, GLuint64 *ptePointer, __GLpteInfoHashTable *pteHashTable)
{
    __GLpageTableEntryInfo *pteInfo;
    GLuint index;

    index = __GL_PTE_HASH_INDEX(ptePointer);
    pteInfo = pteHashTable->hashTable[index];
    while (pteInfo)
    {
        if (pteInfo->ptePointer == ptePointer)
        {
            return GL_TRUE;
        }
        pteInfo = pteInfo->next;
    }

    /* Insert a new pteInfo record into the hash table slot */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLpageTableEntryInfo), (gctPOINTER*)&pteInfo)))
    {
        return GL_FALSE;
    }
    gcoOS_ZeroMemory(pteInfo, sizeof(__GLpageTableEntryInfo));

    pteInfo->ptePointer = ptePointer;
    pteInfo->next = pteHashTable->hashTable[index];
    pteInfo->index = index;
    pteHashTable->hashTable[index] = pteInfo;

    /* Insert the pteInfo record into pteInfo.freeList */
    pteInfo->link = pteHashTable->freeList;
    pteHashTable->freeList = pteInfo;

    return GL_FALSE;
}

__GL_INLINE GLvoid __glClearPageTableEntryDirty(__GLcontext *gc, GLuint64 *ptePointer, GLuint inputIdx)
{
    GLboolean pteExist;

    /* Quick compare of the current ptePointer with the previous ptePointer */
    if (ptePointer == (GLuint64 *)gc->input.pteInfo.lastPtePtr[inputIdx])
    {
        return;
    }

    gc->input.pteInfo.lastPtePtr[inputIdx] = (GLuint64 *)ptePointer;

    pteExist = __glPteInfoExistInHashTable(gc, ptePointer, &gc->input.pteInfo);

    if (pteExist == GL_FALSE)
    {
        /* Clear the page dirty bit (bit 6) */
        (*ptePointer) &= 0xFFFFFFBF;
    }
}

__GL_INLINE void __glClearPteInfoHashTable(__GLcontext *gc, __GLpteInfoHashTable *pteHashTable, GLint clearLast)
{
    __GLpageTableEntryInfo *pteInfo, *nextInfo;

    pteInfo = pteHashTable->freeList;
    while (pteInfo) {
        nextInfo = pteInfo->link;
        pteHashTable->hashTable[pteInfo->index] = NULL;
        gcmOS_SAFE_FREE(gcvNULL, pteInfo);
        pteInfo = nextInfo;
    }
    pteHashTable->freeList = NULL;

    if (clearLast) {
        __GL_MEMZERO(pteHashTable->lastPtePtr, __GL_TOTAL_VERTEX_ATTRIBUTES * sizeof(GLuint64*));
    }
}


__GL_INLINE GLuint64 * __glGetPageTableEntryPointer(__GLcontext *gc, GLfloat *v)
{
    GLuint64 *pageTableBase, *ptePointer;

    if (gc->flags & __GL_USE_FAKE_PAGE_TABLE_ENTRY )
    {
        ptePointer = &(gc->fakePageTableEntry);
    }
    else
    {
        pageTableBase = __GL_PAGE_DIR_ENTRY(gc, v);
        ptePointer = pageTableBase + __GL_PTE_OFFSET(v);
        if (pageTableBase == NULL || *ptePointer == 0)
        {
            /* Re-map page table if page directory entry or page table entry is invalid */
            __glClearPteInfoHashTable(gc, &gc->input.pteInfo, 1);
            /*
            ** Add code later to call kernel driver to map page table to this process
            */
            pageTableBase = __GL_PAGE_DIR_ENTRY(gc, v);
            ptePointer = pageTableBase + __GL_PTE_OFFSET(v);
            if (pageTableBase == NULL || *ptePointer == 0)
            {
                /* If we still could not map the page directroy entry or page table entry, return a fake ptePointer */
                gc->flags |= __GL_USE_FAKE_PAGE_TABLE_ENTRY;
                ptePointer = &(gc->fakePageTableEntry);
            }
        }
    }

    return ptePointer;
}

__GL_INLINE void __glBegin_ImmedCache_Check(__GLcontext *gc, GLenum mode)
{
    if (gc->input.deferredAttribDirty)
    {
        if (gc->input.beginMode == __GL_SMALL_DRAW_BATCH)
        {
            /* If there are defered attribute changes, we have to flush the vertex buffer
            ** and then copy the defered attribute states to current attribute state.
            */
            if (gc->input.deferredAttribDirty & (__GL_DEFERED_ATTRIB_BIT & __GL_DEFERED_COLOR_MASK_BIT))
            {
                __glPrimitiveBatchEnd(gc);
                __glUpdateDeferedAttributes(gc);
                return;
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
                    return;
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
                    return;
                }

                gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);
            }
        }
        else
        {
            __glCopyDeferedAttribToCurrent(gc);
        }
    }
}

__GL_INLINE GLvoid __glDuplicatePreviousAttrib(__GLcontext *gc)
{
    __GLvertexInput *input;
    GLuint64 missingMask;
    GLfloat *current, *prevData;
    GLuint index, inputIdx, col4ub;

    /* Fill in the missing attributes for the current vertex */
    missingMask = gc->input.preVertexFormat & ~gc->input.vertexFormat;

    switch (missingMask)
    {
    case __GL_C3F_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case __GL_C4F_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        current[3] = prevData[3];
        break;

    case __GL_C4UB_BIT:
        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            col4ub = *(GLuint *)gc->input.color.currentPtrDW;
        }
        else {
            GLubyte r = __GL_FLOAT_TO_UB(gc->state.current.color.r);
            GLubyte g = __GL_FLOAT_TO_UB(gc->state.current.color.g);
            GLubyte b = __GL_FLOAT_TO_UB(gc->state.current.color.b);
            GLubyte a = __GL_FLOAT_TO_UB(gc->state.current.color.a);
            col4ub = __GL_PACK_COLOR4UB(r, g, b, a);
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        *(GLuint *)gc->input.color.currentPtrDW = col4ub;
        break;

    case __GL_N3F_BIT:
        if (gc->input.normal.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.normal.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.normal;
        }
        gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.normal.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case __GL_SC3F_BIT:
        if (gc->input.color2.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color2.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color2;
        }
        gc->input.color2.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color2.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    case (__GL_N3F_BIT | __GL_C3F_BIT):
        if (gc->input.normal.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.normal.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.normal;
        }
        gc->input.normal.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.normal.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];

        if (gc->input.color.currentPtrDW >= gc->input.defaultDataBuffer) {
            prevData = gc->input.color.currentPtrDW;
        }
        else {
            prevData = (GLfloat *)&gc->state.current.color;
        }
        gc->input.color.currentPtrDW += gc->input.vertTotalStrideDW;
        current = gc->input.color.currentPtrDW;
        current[0] = prevData[0];
        current[1] = prevData[1];
        current[2] = prevData[2];
        break;

    default:
        if (missingMask & __GL_EDGEFLAG_BIT) {
            if (gc->input.vertex.index == 0)
                gc->input.edgeflag.pointer[gc->input.vertex.index] = gc->state.current.edgeflag;
            else
                gc->input.edgeflag.pointer[gc->input.vertex.index] = gc->input.edgeflag.pointer[gc->input.vertex.index - 1];
            missingMask &= ~__GL_EDGEFLAG_BIT;
        }

        index = 0;
        while (missingMask) {
            if (missingMask & 0x1) {
                inputIdx = fmtIndex2InputIndex[index];
                input = &gc->input.currentInput[inputIdx];
                if (input->currentPtrDW >= gc->input.defaultDataBuffer) {
                    prevData = input->currentPtrDW;
                }
                else {
                    prevData = (GLfloat *)&gc->state.current.currentState[inputIdx];
                }
                input->currentPtrDW += gc->input.vertTotalStrideDW;
                current = input->currentPtrDW;
                switch (input->sizeDW) {
                case 4:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    current[2] = prevData[2];
                    current[3] = prevData[3];
                    break;
                case 3:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    current[2] = prevData[2];
                    break;
                case 2:
                    current[0] = prevData[0];
                    current[1] = prevData[1];
                    break;
                case 1:
                    current[0] = prevData[0];
                    break;
                default:
                    GL_ASSERT(0);
                    break;
                }
            }
            index++;
            missingMask >>= 1;
        }
    }
}

#endif
