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
#include "g_asmoff.h"
#include "api/gc_gl_api_inline.c"

GLint indexDelta[] = { -1, -2, +1, +2 };
GLint testSlotNum = 4;

extern GLvoid __glSwitchImmediateDispatch(__GLcontext *gc, __GLdispatchTable *pDispatch);

GLvoid __glUpdateDeferedAttributes(__GLcontext *gc)
{
    /*
     * Update defered attribute states according to the deferedAttribMask.
    */
    GLboolean temp;
    GLint i, shift;

    struct
    {
        GLboolean redMask;
        GLboolean greenMask;
        GLboolean blueMask;
        GLboolean alphaMask;
    }colorMask[__GL_MAX_DRAW_BUFFERS];

    /* __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT */
    temp = (gc->state.deferredAttribMask & __GL_ATTRIB_POLYGON_OFFSET_FILL_BIT) ? GL_TRUE : GL_FALSE;
    if ( gc->state.enables.polygon.polygonOffsetFill != temp)
    {
        gc->state.enables.polygon.polygonOffsetFill = temp;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT );
    }

    /* __GL_DEPTHTEST_ENDISABLE_BIT */
    temp = (gc->state.deferredAttribMask & __GL_ATTRIB_DEPTH_TEST_BIT) ? GL_TRUE : GL_FALSE;
    if (gc->state.enables.depthTest != temp )
    {
        gc->state.enables.depthTest = temp;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHTEST_ENDISABLE_BIT);
    }

    /* __GL_LINESTIPPLE_ENDISABLE_BIT */
    temp = (gc->state.deferredAttribMask & __GL_ATTRIB_LINE_STIPPLE_BIT) ? GL_TRUE : GL_FALSE;
    if (gc->state.enables.line.stipple != temp )
    {
        gc->state.enables.line.stipple = temp;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_LINESTIPPLE_ENDISABLE_BIT);
    }

    /* __GL_DEPTHMASK_BIT */
    temp = (gc->state.deferredAttribMask & __GL_ATTRIB_DEPTH_MASK_BIT) ? GL_TRUE : GL_FALSE;
    if (gc->state.depth.writeEnable != temp )
    {
        gc->state.depth.writeEnable = temp;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1,  __GL_DEPTHMASK_BIT );
    }

    /* __GL_COLORMASK_BIT */

    for(i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        shift = i<<2;
        colorMask[i].redMask = (gc->state.deferredColorMask & (__GL_ATTRIB_COLOR_MASK_R_BIT << shift) )? GL_TRUE : GL_FALSE;
        colorMask[i].greenMask = (gc->state.deferredColorMask & (__GL_ATTRIB_COLOR_MASK_G_BIT << shift) )? GL_TRUE : GL_FALSE;
        colorMask[i].blueMask = (gc->state.deferredColorMask & (__GL_ATTRIB_COLOR_MASK_B_BIT << shift) )? GL_TRUE : GL_FALSE;
        colorMask[i].alphaMask = (gc->state.deferredColorMask & (__GL_ATTRIB_COLOR_MASK_A_BIT << shift) )? GL_TRUE : GL_FALSE;
    }

    if (__GL_MEMCMP(gc->state.raster.colorMask, colorMask, 4*sizeof(GLboolean)*__GL_MAX_DRAW_BUFFERS))
    {
        __GL_MEMCOPY(gc->state.raster.colorMask, colorMask, 4*sizeof(GLboolean)*__GL_MAX_DRAW_BUFFERS);
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_COLORMASK_BIT);
    }

    /* Sync currentAttribMask and deferedAttribMask.
    */
    gc->state.currentColorMask = gc->state.deferredColorMask;
    gc->state.currentAttribMask = gc->state.deferredAttribMask;
    gc->input.deferredAttribDirty &= ~(__GL_DEFERED_ATTRIB_BIT | __GL_DEFERED_COLOR_MASK_BIT);
}

GLvoid __glCopyDeferedAttribToCurrent(__GLcontext *gc) {
    /* Copy Normal/Color in shadowCurrent state to current state.
     */
    if (gc->input.deferredAttribDirty & __GL_DEFERED_NORMAL_BIT) {
        gc->state.current.normal = gc->input.shadowCurrent.normal;
        gc->input.deferredAttribDirty &= ~(__GL_DEFERED_NORMAL_BIT);
    }
    if (gc->input.deferredAttribDirty & __GL_DEFERED_COLOR_BIT) {
        gc->state.current.color = gc->input.shadowCurrent.color;
        gc->input.deferredAttribDirty &= ~(__GL_DEFERED_COLOR_BIT);

        /* Use the current color to update material state if color material is enabled */
        if (gc->state.enables.lighting.colorMaterial) {
            __glUpdateMaterialfv(gc, gc->state.light.colorMaterialFace,
                    gc->state.light.colorMaterialParam,
                    (GLfloat*) &gc->state.current.color);
        }
    }

    /* Copy the defered attribute states to current attribute states.
     */
    if (gc->input.deferredAttribDirty
            & (__GL_DEFERED_ATTRIB_BIT | __GL_DEFERED_COLOR_MASK_BIT)) {
        __glUpdateDeferedAttributes(gc);
    }
}

__GL_INLINE GLvoid __glGetNextImmedVertexCacheSlot(__GLcontext *gc) {
    __GLvertexCacheBlock *newBlock;
    GLint idx;

    /* Increament gc->input.vtxCacheDrawIndex by 1 to select the next cache slot */
    gc->input.vtxCacheDrawIndex++;

    idx = gc->input.vtxCacheDrawIndex
            - gc->input.currentCacheBlock->indexOffset;
    if (idx < __GL_VERTEX_CACHE_BLOCK_SIZE) {
        /* Point to the next vertex cache in the current vertex cache block */
        gc->input.currentVertexCache = &gc->input.currentCacheBlock->cache[idx];

        if (idx > gc->input.currentCacheBlock->maxVertexCacheIdx) {
            gc->input.currentCacheBlock->maxVertexCacheIdx = idx;
        }
    } else {
        if (gc->input.currentCacheBlock->next) {
            /* Point to the first cache if there is next cache block in the link list */
            gc->input.currentCacheBlock = gc->input.currentCacheBlock->next;
            gc->input.currentVertexCache =
                    &gc->input.currentCacheBlock->cache[0];
        } else {
            /* Allocate a new cache block and append it to the end of the cache block link list */
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, sizeof(__GLvertexCacheBlock), (gctPOINTER*)&newBlock)))
            {
                return;
            }
            gcoOS_ZeroMemory(newBlock, sizeof(__GLvertexCacheBlock));

            newBlock->indexOffset = gc->input.currentCacheBlock->indexOffset
                    + __GL_VERTEX_CACHE_BLOCK_SIZE;
            newBlock->prev = gc->input.currentCacheBlock;

            gc->input.currentCacheBlock->next = newBlock;
            gc->input.currentCacheBlock = newBlock;
            gc->input.currentVertexCache = &newBlock->cache[0];
            gc->input.maxCacheDrawIndex =
                    gc->input.currentCacheBlock->indexOffset
                            + __GL_VERTEX_CACHE_BLOCK_SIZE;
        }

        gc->input.currentCacheBlock->maxVertexCacheIdx = 0;
    }
}

__GL_INLINE GLvoid __glGetVertexCacheFromDrawIndex(__GLcontext *gc,
        GLint drawIndex, __GLvertexDataCache **retVertexCachePtr,
        __GLvertexCacheBlock **retCacheBlockPtr) {
    GLint index;

    index = drawIndex - gc->input.currentCacheBlock->indexOffset;
    if (index >= 0 && index < __GL_VERTEX_CACHE_BLOCK_SIZE) {
        /* Return the vertex cache in the current vertex cache block */
        *retVertexCachePtr = &(gc->input.currentCacheBlock->cache[index]);
        *retCacheBlockPtr = gc->input.currentCacheBlock;
    } else if (index >= __GL_VERTEX_CACHE_BLOCK_SIZE
            && gc->input.currentCacheBlock->next) {
        index = drawIndex - gc->input.currentCacheBlock->next->indexOffset;

        /* Return the vertex cache in the next vertex cache block */
        *retVertexCachePtr = &(gc->input.currentCacheBlock->next->cache[index]);
        *retCacheBlockPtr = gc->input.currentCacheBlock->next;
    } else if (index < 0 && gc->input.currentCacheBlock->prev) {
        index = drawIndex - gc->input.currentCacheBlock->prev->indexOffset;

        /* Return the vertex cache in the previous vertex cache block */
        *retVertexCachePtr = &(gc->input.currentCacheBlock->prev->cache[index]);
        *retCacheBlockPtr = gc->input.currentCacheBlock->prev;
    } else {
        *retVertexCachePtr = NULL;
        *retCacheBlockPtr = NULL;
    }
}

__GL_INLINE GLvoid __glClearVertexCachePTEentries(__GLvertexDataCache *vdc) {
    __GLvertexInfo *vtxinfo, *vtxinfoEnd;

    vtxinfo = vdc->vertexInfoBuffer;
    vtxinfoEnd = (__GLvertexInfo*) ((GLubyte*) vdc->vertexInfoBuffer
            + vdc->infoBufSize);

    if (vtxinfo->inputTag & __GL_DRAWARRAYS_TAG_MASK) {
        while (vtxinfo < vtxinfoEnd) {
            switch (vtxinfo->inputTag) {
            case __GL_ARRAY_V2F_TAG:
            case __GL_ARRAY_V3F_TAG:
            case __GL_ARRAY_V4F_TAG:
            case __GL_ARRAY_N3F_V3F_TAG:
            case __GL_ARRAY_C4F_V3F_TAG:
            case __GL_ARRAY_C3F_TAG:
            case __GL_ARRAY_C4F_TAG:
            case __GL_ARRAY_C4UB_TAG:
            case __GL_ARRAY_N3F_TAG:
            case __GL_ARRAY_TC2F_TAG:
            case __GL_ARRAY_TC3F_TAG:
            case __GL_ARRAY_TC4F_TAG:
            case __GL_ARRAY_INDEX_TAG:
                /* Clear the page dirty bit (bit 6) */
                (*vtxinfo->ptePointer) &= 0xFFFFFFBF;
                break;
            }
            vtxinfo++;
        }
    }
}

__GLvertexDataCache* __glCheckCachedImmedPrimtive(__GLcontext *gc) {
    __GLvertexDataCache *vdc, *vdcNxt, *vdcCmp;
    __GLvertexCacheBlock *vcbNxt;
    GLuint *pCache, *pBuffer, mask;
    GLint index, drawIndex, dataSize, i;
    __GLvertexInfo *vtxinfo;

    /*
     ** __GL_MIN_CACHE_VERTEX_COUNT is used to skip small drawPrimitives inserted in Viewperf
     ** so that the same big drawPrimitives can have the same drawIndexs between frames.
     */
    if (gc->input.vertex.index < __GL_MIN_CACHE_VERTEX_COUNT) {
        return NULL;
    }

#if __GL_VERTEX_CACHE_STATISTIC
    gc->input.canCacheDraws++;
#endif

    /*
     ** gc->input.maxCacheDrawIndex is used to control the total system memory consumption by
     ** vertex cache. It can be dynamically set base on the available system memory.
     */
    if (gc->input.vtxCacheDrawIndex >= gc->input.maxCacheDrawIndex) {
#if __GL_VERTEX_CACHE_STATISTIC
        gc->input.disabledDraws++;
#endif
        return NULL;
    }

    /* Get the current vertex cache pointer and the current drawIndex */
    vdc = gc->input.currentVertexCache;
    drawIndex = gc->input.vtxCacheDrawIndex;

    /* set gc->input.currentVertexCache to the next vertex cache slot */
    __glGetNextImmedVertexCacheSlot(gc);

    index = gc->input.vertTotalStrideDW * (gc->input.vertex.index - 1) + 1;

    switch (vdc->cacheStatus) {
    case __GL_FILL_QUICK_VERTEX_CACHE:
        /*
         ** Cache the current draw information in quick vertex cache.
         */
        vdc->primMode = gc->input.primMode;
        vdc->vertexCount = gc->input.vertex.index;
        vdc->indexCount = gc->input.indexCount;
        vdc->primElemSequence = gc->input.primElemSequence;
        vdc->quickDataCache[0] = ((GLuint*) gc->input.vertexDataBuffer)[0];
        vdc->quickDataCache[1] = ((GLuint*) gc->input.vertexDataBuffer)[index];

        /* Enable the the quick primitive data comparision for the next frame */
        gc->input.vertexCacheStatus |= __GL_CHECK_QUICK_VERTEX_CACHE;
        vdc->cacheStatus = __GL_CHECK_QUICK_VERTEX_CACHE;
        vdc->frameIndex = gc->input.currentFrameIndex;
        break;

    case __GL_CHECK_QUICK_VERTEX_CACHE:
        /*
         ** Compare the current draw information with quick cache to see
         ** if the primitive data are the same.
         */
        if (vdc->primMode == gc->input.primMode
                && vdc->vertexCount == gc->input.vertex.index
                && vdc->indexCount == gc->input.indexCount
                && vdc->primElemSequence == gc->input.primElemSequence
                && vdc->quickDataCache[0]
                        == ((GLuint*) gc->input.vertexDataBuffer)[0]
                && vdc->quickDataCache[1]
                        == ((GLuint*) gc->input.vertexDataBuffer)[index]) {
            /* Copy the primitive element information to the vertex cache buffer */
            vdc->vertTotalStrideDW = gc->input.vertTotalStrideDW;
            vdc->primInputMask = gc->input.primInputMask;
            vdc->primitiveFormat = gc->input.primitiveFormat;
            vdc->numberOfElements = gc->input.numberOfElements;
            vdc->indexPrimEnabled = gc->input.indexPrimEnabled;
            i = 0;
            mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
            while (mask) {
                if (mask & 0x1) {
                    vdc->elemOffsetDW[i] = gc->input.currentInput[i].offsetDW;
                    vdc->elemSizeDW[i] = gc->input.currentInput[i].sizeDW;
                }
                mask >>= 1;
                i += 1;
            }

            /* Copy primitive batch connection information to the vertex cache buffer */
            vdc->connectPrimMode = gc->input.connectPrimMode;
            vdc->connectVertexCount = gc->input.connectVertexCount;
            vdc->connectVertexIndex[0] = gc->input.connectVertexIndex[0];
            vdc->connectVertexIndex[1] = gc->input.connectVertexIndex[1];
            vdc->connectVertexIndex[2] = gc->input.connectVertexIndex[2];
            vdc->connectVertexIndex[3] = gc->input.connectVertexIndex[3];

            /* Save a __GL_BATCH_END_TAG at the end of valid data in gc->input.vertexInfoBuffer */
            vtxinfo = gc->input.currentInfoBufPtr++;
            vtxinfo->inputTag = __GL_BATCH_END_TAG;
            vtxinfo->offsetDW = (GLuint)(gc->input.vertex.index
                    + gc->input.vertexTrimCount) * gc->input.vertTotalStrideDW;
            vtxinfo->appDataPtr = NULL;
            vtxinfo->ptePointer = NULL;

            /* Compute the required buffer size to cache the vertex info buffer */
            dataSize = (GLuint)(
                    (GLubyte*) gc->input.currentInfoBufPtr
                            - (GLubyte*) gc->input.vertexInfoBuffer);

            /* If the infoBufSize is not equal to the dataSize */
            if (dataSize != vdc->infoBufSize) {
                /* Free the old primitive info buffer */
                if (vdc->vertexInfoBuffer) {
                    gcmOS_SAFE_FREE(gcvNULL, vdc->vertexInfoBuffer);
                    gc->input.totalCacheMemSize -= vdc->infoBufSize;
                }

                /* Allocate a new vertex info buffer with the size "dataSize" */
                if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, dataSize, (gctPOINTER*)&vdc->vertexInfoBuffer)))
                {
                    vdc->infoBufSize = 0;
                    gc->input.totalCacheMemSize -= dataSize;
                    __glSetError(gc, GL_OUT_OF_MEMORY);
                    return NULL;
                }
                vdc->infoBufSize = dataSize;
                gc->input.totalCacheMemSize += dataSize;
            }

            /* Copy the vertex info buffer to the vertex cache buffer */
            __GL_MEMCOPY(vdc->vertexInfoBuffer, gc->input.vertexInfoBuffer,
                    dataSize);

            /* Compute the required buffer size to cache the primitive data */
            dataSize = (((gc->input.vertex.index + gc->input.vertexTrimCount)
                    * gc->input.vertTotalStrideDW) << 2);

            /* If the dataBufSize is not equal to the dataSize */
            if (dataSize != vdc->dataBufSize) {
                /* Free the old primitive data buffer */
                if (vdc->vertexDataBuffer) {
                    gcmOS_SAFE_FREE(gcvNULL, vdc->vertexDataBuffer);
                    gc->input.totalCacheMemSize -= vdc->dataBufSize;
                }

                /* Allocate a new buffer with the size "dataSize" */
                if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, dataSize, (gctPOINTER*)&vdc->vertexDataBuffer)))
                {
                    vdc->dataBufSize = 0;
                    gc->input.totalCacheMemSize -= dataSize;
                    __glSetError(gc, GL_OUT_OF_MEMORY);
                    return NULL;
                }
                vdc->dataBufSize = dataSize;
                gc->input.totalCacheMemSize += dataSize;
            }

            /* Copy the primitive date to the vertex cache buffer */
            __GL_MEMCOPY(vdc->vertexDataBuffer, gc->input.vertexDataBuffer,
                    dataSize);

            if (gc->input.indexCount) {
                /* If the indexBufSize is not equal to the dataSize */
                dataSize = gc->input.indexCount * sizeof(GLushort);
                if (dataSize != vdc->indexBufSize) {
                    /* Free the old primitive data buffer */
                    if (vdc->indexBuffer) {
                        gcmOS_SAFE_FREE(gcvNULL, vdc->indexBuffer);
                        gc->input.totalCacheMemSize -= vdc->indexBufSize;
                    }

                    /* Allocate a new index buffer with the size "dataSize" */
                    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, dataSize, (gctPOINTER*)&vdc->indexBuffer)))
                    {
                        vdc->indexBufSize = 0;
                        gc->input.totalCacheMemSize -= dataSize;
                        __glSetError(gc, GL_OUT_OF_MEMORY);
                        return NULL;
                    }

                    vdc->indexBufSize = dataSize;
                    gc->input.totalCacheMemSize += dataSize;
                }

                /* Copy the index buffer to the vertex cache */
                __GL_MEMCOPY(vdc->indexBuffer, gc->input.indexBuffer, dataSize);

            }

            /* Enable full primitive data comparision for the next frame */
            gc->input.vertexCacheStatus |= __GL_CHECK_FULL_VERTEX_CACHE;
            vdc->cacheStatus = __GL_CHECK_FULL_VERTEX_CACHE;
            vdc->frameIndex = gc->input.currentFrameIndex;

            /* Check the system memory consumption and set the MaxCacheDrawIndex
             ** to limit the vertex cache memory size if it's necessary.
             */
            /*
             if (
             (*gc->imports.getMemoryStatus)(__GL_MMUSAGE_PHYS_AVAIL) < 0x100000 ) */
            if (0)
            {
                if (gc->input.vtxCacheNeedReset) {
                    gc->input.maxCacheDrawIndex = gc->input.vtxCacheDrawIndex;
                    gc->input.vtxCacheNeedReset = GL_FALSE;
#if __GL_VERTEX_CACHE_STATISTIC
                    fprintf(gc->input.vtxCacheStatFile, "Frame:%3d, Vertex caches after drawIndex %d are disabled!\n",
                        gc->input.currentFrameIndex, gc->input.maxCacheDrawIndex - 1);
#endif
                }
#if __GL_VERTEX_CACHE_STATISTIC
                else {
                    fprintf(gc->input.vtxCacheStatFile, "Frame:%3d, Vertex cache of drawIndex %d is disabled!\n",
                        gc->input.currentFrameIndex, gc->input.vtxCacheDrawIndex);
                }
#endif

                /* Free the cached data buffers of this draw */
                if (vdc->vertexInfoBuffer) {
                    gcmOS_SAFE_FREE(gcvNULL, vdc->vertexInfoBuffer);
                    gc->input.totalCacheMemSize -= vdc->infoBufSize;
                    vdc->vertexInfoBuffer = NULL;
                    vdc->infoBufSize = 0;
                }
                if (vdc->vertexDataBuffer) {
                    gcmOS_SAFE_FREE(gcvNULL, vdc->vertexDataBuffer);
                    gc->input.totalCacheMemSize -= vdc->dataBufSize;
                    vdc->vertexDataBuffer = NULL;
                    vdc->dataBufSize = 0;
                }
                if (vdc->indexBuffer) {
                    gcmOS_SAFE_FREE(gcvNULL, vdc->indexBuffer);
                    gc->input.totalCacheMemSize -= vdc->indexBufSize;
                    vdc->indexBuffer = NULL;
                    vdc->indexBufSize = 0;
                }

                GL_ASSERT((vdc->privateData==NULL)&&(vdc->ibPrivateData==NULL));

                vdc->cacheStatus = __GL_VERTEX_CACHE_DISABLED;
            }

            /* Enable device pipeline to cache the primitive in device memory */
            return (vdc);
        } else {
            /* The current draw is different from the previous frame's draw,
             ** re-cache the brief draw information.
             */
            vdc->primMode = gc->input.primMode;
            vdc->vertexCount = gc->input.vertex.index;
            vdc->indexCount = gc->input.indexCount;
            vdc->primElemSequence = gc->input.primElemSequence;
            vdc->quickDataCache[0] = ((GLuint*) gc->input.vertexDataBuffer)[0];
            vdc->quickDataCache[1] =
                    ((GLuint*) gc->input.vertexDataBuffer)[index];

            /* Delete the cached primitive data in DP only. Re-use buffers in system memory */
            if (vdc->privateData) {
                (*gc->dp.deletePrimData)(gc, vdc->privateData);
                vdc->privateData = NULL;
            }
            if (vdc->ibPrivateData) {
                (*gc->dp.deletePrimData)(gc, vdc->ibPrivateData);
                vdc->ibPrivateData = NULL;
            }

            /* Keep doing quick primitive data comparision in the next frame */
            gc->input.vertexCacheStatus |= __GL_CHECK_QUICK_VERTEX_CACHE;
            vdc->cacheStatus = __GL_CHECK_QUICK_VERTEX_CACHE;
            vdc->frameIndex = gc->input.currentFrameIndex;
        }
        break;

    case __GL_CHECK_FULL_VERTEX_CACHE:
        /*
         ** If the current primitive is the same as the cached copy
         ** then enable DP to use the cached primitive in device memory directly.
         */
        if (gc->input.cacheBufferUsed) {
            gc->input.vertexCacheStatus |= __GL_IMMED_VERTEX_CACHE_HIT;
            gc->input.cacheHitFrameIndex = gc->input.currentFrameIndex;

            return (vdc);
        }

        /*
         ** If the current cache slot does not match the incoming primitive then look around the current
         ** cache slot (-2, -1, +1, + 2) to see if there is a matching slot with the incoming primitive.
         */
        vdcCmp = vdc;
        vdcNxt = NULL;
        if (vdc->vertexCount != gc->input.vertex.index
                || vdc->indexCount != gc->input.indexCount) {
            for (i = 0; i < testSlotNum; i++) {
                GLint nextIndex = drawIndex + indexDelta[i];
                __glGetVertexCacheFromDrawIndex(gc, nextIndex, &vdcNxt,
                        &vcbNxt);
                if (vdcNxt) {
                    if (vdcNxt->cacheStatus == __GL_CHECK_FULL_VERTEX_CACHE
                            && vdcNxt->vertexCount == gc->input.vertex.index
                            && vdcNxt->indexCount == gc->input.indexCount
                            && vdcNxt->quickDataCache[0]
                                    == ((GLuint*) gc->input.vertexDataBuffer)[0]
                            && vdcNxt->quickDataCache[1]
                                    == ((GLuint*) gc->input.vertexDataBuffer)[index]) {
                        vdcCmp = vdcNxt;
                        drawIndex = nextIndex;
                        break;
                    }
                }
            }
        }

        /*
         ** Compare data in the default vertex buffer with the cache buffer to see if they are the same.
         */
        if ((vdcCmp != vdc || !gc->input.cacheCompareFailed)
                && vdcCmp->primMode == gc->input.primMode
                && vdcCmp->vertexCount == gc->input.vertex.index
                && vdcCmp->indexCount == gc->input.indexCount
                && vdcCmp->primElemSequence == gc->input.primElemSequence) {
            GLint result = 0;
            dataSize = gc->input.vertex.index * gc->input.vertTotalStrideDW;
            pCache = (GLuint*) vdcCmp->vertexDataBuffer;
            pBuffer = (GLuint*) gc->input.vertexDataBuffer;

            for (i = 0; i < dataSize; i++) {
                if (*pCache++ ^ *pBuffer++) {
                    result = 1;
                    break;
                }
            }

            if (result == 0) {
                if (vdcCmp == vdcNxt) {
                    /* If one of adjacent cache slots matches the primitive then change vtxCacheDrawIndex
                     ** as well as gc->input.currentVertexCache, gc->input.currentCacheBlock, and contine
                     ** vertex cache comparison from that matching cache slot.
                     */
                    gc->input.vtxCacheDrawIndex = drawIndex + 1;
                    __glGetVertexCacheFromDrawIndex(gc,
                            gc->input.vtxCacheDrawIndex,
                            &gc->input.currentVertexCache,
                            &gc->input.currentCacheBlock);
                }

                /* Clear app data PTE entry dirty bits if app data is the same as cached data */
                __glClearVertexCachePTEentries(vdcCmp);

                gc->input.vertexCacheStatus |= __GL_IMMED_VERTEX_CACHE_HIT;
                gc->input.cacheHitFrameIndex = gc->input.currentFrameIndex;
                return (vdcCmp);
            }
        }

        /* The current draw is different from the previous frame's draw,
         ** re-cache the brief draw information.
         */
        vdc->primMode = gc->input.primMode;
        vdc->vertexCount = gc->input.vertex.index;
        vdc->indexCount = gc->input.indexCount;
        vdc->primElemSequence = gc->input.primElemSequence;
        vdc->quickDataCache[0] = ((GLuint*) gc->input.vertexDataBuffer)[0];
        vdc->quickDataCache[1] = ((GLuint*) gc->input.vertexDataBuffer)[index];

        /* Delete the cached primitive data in DP only. Re-use buffers in system memory */
        if (vdc->privateData) {
            (*gc->dp.deletePrimData)(gc, vdc->privateData);
            vdc->privateData = NULL;
        }
        if (vdc->ibPrivateData) {
            (*gc->dp.deletePrimData)(gc, vdc->ibPrivateData);
            vdc->ibPrivateData = NULL;
        }

#if __GL_VERTEX_CACHE_STATISTIC
        gc->input.cacheMissDraws++;
#endif
        /* Keep doing quick primitive data comparision in the next frame */
        gc->input.vertexCacheStatus |= __GL_CHECK_QUICK_VERTEX_CACHE;
        vdc->cacheStatus = __GL_CHECK_QUICK_VERTEX_CACHE;
        vdc->frameIndex = gc->input.currentFrameIndex;
        break;

    case __GL_VERTEX_CACHE_DISABLED:
#if __GL_VERTEX_CACHE_STATISTIC
        gc->input.disabledDraws++;
#endif
        return NULL;
    }

    return NULL;
}

__GL_INLINE GLvoid __glImmediateBegin_Cache(__GLcontext *gc, GLenum mode) {
    GLuint beginTag = (mode | 0x10);

    gc->input.currentPrimMode = mode;

    LoopBack:

    /* If the incoming glBegin matches __GL_BEGIN_PRIM_TAG in gc->input.vertexInfoBuffer
     ** then we don't need to do anything, just return.
     */
    if (gc->pCurrentInfoBufPtr->inputTag == beginTag) {
        gc->pCurrentInfoBufPtr++;
        gc->input.beginMode = __GL_IN_BEGIN;
        return;
    }

    /* If it is the end of vertex cache buffer then flush the vertex data */
    if (gc->pCurrentInfoBufPtr->inputTag == __GL_BATCH_END_TAG) {
        __glImmedFlushBuffer_Cache(gc, beginTag);
        (*gc->currentImmediateDispatch->Begin)(gc, mode);
        return;
    }

    /* Skip the empty __GL_BEGIN_*_TAG and __GL_END_TAG pair */
    if (gc->pCurrentInfoBufPtr->inputTag < __GL_END_TAG
            && (gc->pCurrentInfoBufPtr + 1)->inputTag == __GL_END_TAG) {
        gc->pCurrentInfoBufPtr += 2;
        goto LoopBack;
    }

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, beginTag);

    /* Call generic Begin function */
    (*gc->currentImmediateDispatch->Begin)(gc, mode);
}

GLvoid APIENTRY __glim_Begin_Cache_First(__GLcontext *gc, GLenum mode) {
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __glBegin_ImmedCache_Check(gc, mode);

    /* Jump to __glim_Begin_Info function if cache buffer is not used.
     */
    if (!gc->input.cacheBufferUsed) {
        (*gc->currentImmediateDispatch->Begin)(gc, mode);
        return;
    }

    /* Compute the required primitive input mask.
     */
    if (gc->input.inputMaskChanged) {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask
            & edgeFlagInputMask[mode];

    /* Switch to immediate cached dispatch table.
     */
    __glSwitchImmediateDispatch(gc, gc->input.pCurrentImmedModeDispatch);

    __glImmediateBegin_Cache(gc, mode);
}

GLvoid APIENTRY __glim_Begin_Cache(__GLcontext *gc, GLenum mode) {
    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT) {
        __glSetError(gc, GL_INVALID_ENUM);
        return;
    }

    __glImmediateBegin_Cache(gc, mode);
}

GLvoid APIENTRY __glim_End_Cache(__GLcontext *gc) {
    /* If the incoming glEnd matches __GL_END_TAG in gc->input.vertexInfoBuffer
     ** then we don't need to do anything, just return.
     */
    if (gc->pCurrentInfoBufPtr->inputTag == __GL_END_TAG) {
        gc->pCurrentInfoBufPtr++;

        gc->input.beginMode = __GL_SMALL_DRAW_BATCH;

        return;
    }

    /* If it is the end of vertex cache buffer then flush the vertex data */
    if (gc->pCurrentInfoBufPtr->inputTag == __GL_BATCH_END_TAG) {
        __glImmedFlushBuffer_Cache(gc, __GL_END_TAG);
        (*gc->currentImmediateDispatch->End)(gc);
        return;
    }

    /* Switch the current vertex buffer back to the default vertex buffer */
    __glSwitchToDefaultVertexBuffer(gc, __GL_END_TAG);

    /* Call generic End function */
    (*gc->currentImmediateDispatch->End)(gc);
}

GLvoid APIENTRY __glim_End_Error(__GLcontext *gc) {
    if (gc->input.beginMode != __GL_IN_BEGIN) {
        __glSetError(gc, GL_INVALID_OPERATION);
        return;
    }
}

GLvoid __glImmedFlushBuffer_Cache(__GLcontext *gc, GLuint inputTag) {
    /* Cache gc->input.currentVertexCache before __glDrawImmedPrimtive(gc) */
    __GLvertexDataCache *vtxCache = gc->input.currentVertexCache;
    GLuint inputMask;
    GLint i;

    /* Copy the global variable gc->pCurrentInfoBufPtr back to gc->input.currentInfoBufPtr.
     */
    gc->input.currentInfoBufPtr = gc->pCurrentInfoBufPtr;

    /* Draw primitives from the vertex cache */
    gc->input.vertex.index = vtxCache->vertexCount;
    gc->input.indexCount = vtxCache->indexCount;

    __glDrawImmedPrimitive(gc);

    __glImmedUpdateVertexState(gc);
    __glResetImmedVertexBuffer(gc, gc->input.enableVertexCaching);

    /* No need for primitive batch connection if next primitive batch uses cached buffer
     ** or the current input API is glEnd, or beginMode is __GL_NOT_IN_BEGIN.
     */
    if (gc->input.cacheBufferUsed || inputTag <= __GL_END_TAG
            || gc->input.beginMode == __GL_NOT_IN_BEGIN) {
        return;
    }

    /* Copy the connecting vertices to the beginnig of vertexDataBuffer.
     */
    for (i = 0; i < vtxCache->connectVertexCount; i++) {
        __GL_MEMCOPY(
                gc->input.defaultDataBuffer + i * vtxCache->vertTotalStrideDW,
                vtxCache->vertexDataBuffer
                        + vtxCache->connectVertexIndex[i]
                                * vtxCache->vertTotalStrideDW,
                (vtxCache->vertTotalStrideDW << 2));
    }

    /* Copy the connecting edgeflags to the beginnig of edgeflag buffer.
     */
    if (vtxCache->primInputMask & __GL_INPUT_EDGEFLAG) {
        GLboolean *edgeaddr = gc->input.edgeflag.pointer;
        for (i = 0; i < vtxCache->connectVertexCount; i++) {
            edgeaddr[i] = edgeaddr[vtxCache->connectVertexIndex[i]];
        }
    }

    gc->input.connectPrimMode = gc->input.currentPrimMode;
    gc->input.vertex.index = vtxCache->connectVertexCount;
    gc->input.primBeginAddr = gc->input.defaultDataBuffer;
    gc->input.currentDataBufPtr = gc->input.defaultDataBuffer
            + vtxCache->connectVertexCount * vtxCache->vertTotalStrideDW;
    gc->input.preVertexFormat = vtxCache->primitiveFormat;

    /* Reset all the input pointers to the new locations.
     */
    i = 0;
    inputMask = vtxCache->primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (inputMask) {
        if (inputMask & 0x1) {
            gc->input.currentInput[i].pointer =
                    (GLubyte*) (gc->input.defaultDataBuffer
                            + gc->input.currentInput[i].offsetDW);
            gc->input.currentInput[i].currentPtrDW =
                    (GLfloat*) gc->input.currentInput[i].pointer
                            + (vtxCache->connectVertexCount - 1)
                                    * gc->input.vertTotalStrideDW;
            gc->input.currentInput[i].index = vtxCache->connectVertexCount;
        }
        inputMask >>= 1;
        i += 1;
    }
}


