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
#include "g_lcfncs.h"

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

#define __GL_PRIM_V2F               (__GL_V2F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_V3F               (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_C4UB_V2F          ((__GL_C4UB_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V2F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_C4UB_V3F          ((__GL_C4UB_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_C3F_V3F           ((__GL_C3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_N3F_V3F           ((__GL_N3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_C4F_N3F_V3F       ((__GL_C4F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 2)) | \
                                    ((__GL_N3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T2F_V3F           ((__GL_TC2F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T4F_V4F           ((__GL_TC4F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V4F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T2F_C4UB_V3F      ((__GL_TC2F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 2)) | \
                                    ((__GL_C4UB_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T2F_C3F_V3F       ((__GL_TC2F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 2)) | \
                                    ((__GL_C3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T2F_N3F_V3F       ((__GL_TC2F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 2)) | \
                                    ((__GL_N3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T2F_C4F_N3F_V3F   ((__GL_TC2F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 3)) | \
                                    ((__GL_C4F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT *2)) | \
                                    ((__GL_N3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V3F_TAG & __GL_PRIM_ELEMENT_MASK)

#define __GL_PRIM_T4F_C4F_N3F_V4F   ((__GL_TC4F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT * 3)) | \
                                    ((__GL_C4F_TAG & __GL_PRIM_ELEMENT_MASK) << (__GL_PRIM_ELEMENT_SHIFT *2)) | \
                                    ((__GL_N3F_TAG & __GL_PRIM_ELEMENT_MASK) << __GL_PRIM_ELEMENT_SHIFT) | \
                                    (__GL_V4F_TAG & __GL_PRIM_ELEMENT_MASK)



extern GLvoid __glDlistFreePrimitive(__GLcontext *, GLubyte *);
extern GLvoid __glDlistFreePrivateData(__GLcontext *, GLubyte *);


GLvoid __glComputeArrayPrimBegin(__GLcontext *gc, GLenum mode, GLsizei count, __GLPrimBegin *primBegin)
{
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    __GLPrimBegin *prevPrimBegin;
    GLuint copyPrevPrimBegin;
    GLint i, offset, size = 0;
    GLuint64 mask;

    /* We can copy primBegin from prevPrimBegin if vertex format is not changed */
    copyPrevPrimBegin = GL_FALSE;
    if (gc->dlist.listData.lastPrimNode && !gc->vertexArray.formatChanged)
    {
        prevPrimBegin = (__GLPrimBegin *)(gc->dlist.listData.lastPrimNode + 1);
        __GL_MEMCOPY(primBegin, prevPrimBegin, sizeof(__GLPrimBegin));
        primBegin->edgeflagBuffer = NULL;
        copyPrevPrimBegin = GL_TRUE;
    }
    else
    {
        __GL_MEMZERO(primBegin, sizeof(__GLPrimBegin));
    }

    primBegin->primType = mode;
    primBegin->vertexCount = count;
    primBegin->primCount = 1;
    primBegin->primInputMask = pV->attribEnabled;

    /* Allocate edgeflagBuffer if it is needed */
    if ((mode == GL_TRIANGLES || mode == GL_QUADS || mode == GL_POLYGON || mode == GL_TRIANGLES_ADJACENCY_EXT) &&
        (pV->attribEnabled & __GL_VARRAY_EDGEFLAG))
    {
        primBegin->edgeflagBuffer = (GLubyte *)(*gc->imports.malloc)(gc, count * sizeof(GLubyte) );
        if (primBegin->edgeflagBuffer == NULL) {
            __glSetError(gc, GL_OUT_OF_MEMORY);
            return;
        }
    }
    else {
        /* Clear the edgeflag bit since it is not needed */
        primBegin->primInputMask &= ~(__GL_VARRAY_EDGEFLAG);
    }

    /* Early return if we can copy primBegin from prevPrimBegin */
    if (copyPrevPrimBegin) {
        return;
    }

    /* Clear the vertex array formatChanged flag */
    gc->vertexArray.formatChanged = GL_FALSE;

    /* Compute the number of elements */
    mask = primBegin->primInputMask & (~__GL_INPUT_EDGEFLAG);
    while (mask) {
        if (mask & 0x1)
            primBegin->elementCount += 1;
        mask >>= 1;
    }

    offset = 0;

    /*
    ** Note:
    ** The following array enable checks must follow the GL Spec code sequence
    ** for ArrayElement so that elemOffsetDW[] match the copied element data in buffer.
    */

    if (pV->attribEnabled & __GL_VARRAY_NORMAL) {
        size = 3;
        primBegin->primitiveFormat |= __GL_N3F_BIT;
        __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_N3F_TAG);
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_NORMAL_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_NORMAL_INDEX] = offset;
        offset += size;
    }

    if (pV->attribEnabled & __GL_VARRAY_DIFFUSE) {
        switch (pV->color.size) {
        case 3:
            if (pV->color.type == GL_BYTE || pV->color.type == GL_UNSIGNED_BYTE) {
                size = 1;
                primBegin->primitiveFormat |= __GL_C4UB_BIT;
                __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_C4UB_TAG);
            }
            else {
                size = 3;
                primBegin->primitiveFormat |= __GL_C3F_BIT;
                __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_C3F_TAG);
            }
            break;
        case 4:
            if (pV->color.type == GL_BYTE || pV->color.type == GL_UNSIGNED_BYTE) {
                size = 1;
                primBegin->primitiveFormat |= __GL_C4UB_BIT;
                __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_C4UB_TAG);
            }
            else {
                size = 4;
                primBegin->primitiveFormat |= __GL_C4F_BIT;
                __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_C4F_TAG);
            }
            break;
        default:
            GL_ASSERT(0);
        }
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_DIFFUSE_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_DIFFUSE_INDEX] = offset;
        offset += size;
    }

    if (pV->attribEnabled & __GL_VARRAY_SPECULAR) {
        size = 3;
        primBegin->primitiveFormat |= __GL_SC3F_BIT;
        __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_SC3F_TAG);
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_SPECULAR_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_SPECULAR_INDEX] = offset;
        offset += size;
    }

    if (pV->attribEnabled & __GL_VARRAY_FOGCOORD) {
        size = 1;
        primBegin->primitiveFormat |= __GL_FOG1F_BIT;
        __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_FOG1F_TAG);
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_FOGCOORD_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_FOGCOORD_INDEX] = offset;
        offset += size;
    }

    if (pV->attribEnabled & __GL_VARRAY_TEXTURES) {
        mask = (pV->attribEnabled & __GL_VARRAY_TEXTURES) >> __GL_VARRAY_TEX0_INDEX;
        i = 0;
        while (mask) {
            if (mask & 0x1) {
                switch (pV->texture[i].size) {
                case 1:
                    size = 2;
                    primBegin->primitiveFormat |= (__GL_ONE_64 << (__GL_TC2F_INDEX + i));
                    __GL_PRIM_ELEMENT(primBegin->primElemSequence, (__GL_TC2F_TAG + i));
                    break;
                case 2:
                    size = 2;
                    primBegin->primitiveFormat |= (__GL_ONE_64 << (__GL_TC2F_INDEX + i));
                    __GL_PRIM_ELEMENT(primBegin->primElemSequence, (__GL_TC2F_TAG + i));
                    break;
                case 3:
                    size = 3;
                    primBegin->primitiveFormat |= (__GL_ONE_64 << (__GL_TC3F_INDEX + i));
                    __GL_PRIM_ELEMENT(primBegin->primElemSequence, (__GL_TC3F_TAG + i));
                    break;
                case 4:
                    size = 4;
                    primBegin->primitiveFormat |= (__GL_ONE_64 << (__GL_TC4F_INDEX + i));
                    __GL_PRIM_ELEMENT(primBegin->primElemSequence, (__GL_TC4F_TAG + i));
                    break;
                }
                primBegin->totalStrideDW += size;
                primBegin->elemSizeDW[__GL_VARRAY_TEX0_INDEX + i] = size;
                primBegin->elemOffsetDW[__GL_VARRAY_TEX0_INDEX + i] = offset;
                offset += size;
            }
            mask >>= 1;
            i += 1;
        }
    }


    if (primBegin->edgeflagBuffer && (pV->attribEnabled & __GL_VARRAY_EDGEFLAG)) {
        primBegin->primitiveFormat |= __GL_EDGEFLAG_BIT;
        __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_EDGEFLAG_TAG);
    }

    if (pV->attribEnabled & __GL_VARRAY_ATTRIBUTES) {
        mask = (pV->attribEnabled & __GL_VARRAY_ATTRIBUTES) >> __GL_VARRAY_ATT0_INDEX;
        i = 0;
        while (mask) {
            if (mask & 0x1) {
                size = 4;
                primBegin->primitiveFormat |= (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + i));
                __GL_PRIM_ELEMENT(primBegin->primElemSequence, (__GL_AT4F_I0_TAG + i));
                primBegin->totalStrideDW += size;
                primBegin->elemSizeDW[__GL_VARRAY_ATT0_INDEX + i] = size;
                primBegin->elemOffsetDW[__GL_VARRAY_ATT0_INDEX + i] = offset;
                offset += size;
            }
            mask >>= 1;
            i += 1;
        }
    }

    if (pV->attribEnabled & __GL_VARRAY_ATT0) {
        switch (pV->attribute[0].size) {
        case 1:
        case 2:
            size = 2;
            primBegin->primitiveFormat |= __GL_V2F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V2F_TAG);
            break;
        case 3:
            size = 3;
            primBegin->primitiveFormat |= __GL_V3F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V3F_TAG);
            break;
        case 4:
            size = 4;
            primBegin->primitiveFormat |= __GL_V4F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V4F_TAG);
            break;
        default:
            GL_ASSERT(0);
        }
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_VERTEX_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_VERTEX_INDEX] = offset;
        offset += size;
    }
    else if (pV->attribEnabled & __GL_VARRAY_VERTEX)
    {
        switch (pV->vertex.size) {
        case 2:
            size = 2;
            primBegin->primitiveFormat |= __GL_V2F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V2F_TAG);
            break;
        case 3:
            size = 3;
            primBegin->primitiveFormat |= __GL_V3F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V3F_TAG);
            break;
        case 4:
            size = 4;
            primBegin->primitiveFormat |= __GL_V4F_BIT;
            __GL_PRIM_ELEMENT(primBegin->primElemSequence, __GL_V4F_TAG);
            break;
        default:
            GL_ASSERT(0);
        }
        primBegin->totalStrideDW += size;
        primBegin->elemSizeDW[__GL_VARRAY_VERTEX_INDEX] = size;
        primBegin->elemOffsetDW[__GL_VARRAY_VERTEX_INDEX] = offset;
        offset += size;
    }

    /* Skip optimized arrayElement function if there are varrays in bufferObject */

    gc->vertexArray.optdlArrayElement = NULL;
    if ((pV->arrayInBufObj & pV->attribEnabled) == 0)
    {
        switch (primBegin->primElemSequence)
        {
        case __GL_PRIM_V2F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_V2F;
            break;
        case __GL_PRIM_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_V3F;
            break;
        case __GL_PRIM_C4UB_V2F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_C4UB_V2F;
            break;
        case __GL_PRIM_C4UB_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_C4UB_V3F;
            break;
        case __GL_PRIM_C3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_C3F_V3F;
            break;
        case __GL_PRIM_N3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_N3F_V3F;
            break;
        case __GL_PRIM_C4F_N3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_C4F_N3F_V3F;
            break;
        case __GL_PRIM_T2F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T2F_V3F;
            break;
        case __GL_PRIM_T4F_V4F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T4F_V4F;
            break;
        case __GL_PRIM_T2F_C4UB_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T2F_C4UB_V3F;
            break;
        case __GL_PRIM_T2F_C3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T2F_C3F_V3F;
            break;
        case __GL_PRIM_T2F_N3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T2F_N3F_V3F;
            break;
        case __GL_PRIM_T2F_C4F_N3F_V3F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T2F_C4F_N3F_V3F;
            break;
        case __GL_PRIM_T4F_C4F_N3F_V4F:
            gc->vertexArray.optdlArrayElement = __glArrayElement_T4F_C4F_N3F_V4F;
            break;
        }
    }
}

GLvoid APIENTRY __gllc_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid *elemIndices)
{
    __GLdlistOp *dlop;
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint tagBuf[__GL_TOTAL_VERTEX_ATTRIBUTES];
    __GLbufferObject *bufObj = NULL;
    __GLPrimBegin primBegin, *prevPrimBegin = gcvNULL;
    GLvoid *indices = gcvNULL;
    GLfloat *bufptr;
    GLubyte *edgeptr;
    GLint i, vertexCount, startIndex;
    GLuint mergePrimNode;
    GLenum error = GL_NO_ERROR;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->immedModeDispatch.DrawElements)(gc, mode, count, type, elemIndices);
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT) {
        __gllc_InvalidEnum(gc);
        return;
    }

    switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_INT:
        break;
    default:
        __gllc_InvalidEnum(gc);
        return;
    }

    if (count < 0) {
        __gllc_InvalidValue(gc);
        return;
    }

    CHECK_VERTEX_COUNT();

    /*
    ** Just return if vertex array is not enabled since
    ** vertex state is undefined after DrawElements.
    */
    if (!(pV->attribEnabled & __GL_VARRAY_VERTEX)) {
        return;
    }

    /*
    ** Break a large triangle list into multiple smaller triangle lists.
    */
    vertexCount = count;
    if (mode == GL_TRIANGLES && count > __GL_MAX_DLIST_VERTEX_NUMBER) {
        vertexCount = __GL_MAX_DLIST_VERTEX_NUMBER;
    }

    startIndex = 0;
    while (startIndex < count)
    {
        /* Setup __GLPrimBegin header for the primitive.
        */
        __glComputeArrayPrimBegin(gc, mode, vertexCount, &primBegin);

        /* Check if the current primitive node can be merged with a previous PrimBegin node.
        */
        mergePrimNode = GL_FALSE;
        if (gc->dlist.listData.lastPrimNode)
        {
            prevPrimBegin = (__GLPrimBegin *)(gc->dlist.listData.lastPrimNode + 1);

            /* We don't have to compare elemOffsetDW[i] since __glComputeArrayPrimBegin function
            ** will always generate the same elemOffsetDW[i] for a primBegin.primitiveFormat.
            */
            if (((prevPrimBegin->vertexCount + primBegin.vertexCount) < __GL_MAX_DLIST_VERTEX_NUMBER) &&
                (prevPrimBegin->primitiveFormat == primBegin.primitiveFormat) &&
                (prevPrimBegin->edgeflagBuffer == NULL) && (primBegin.edgeflagBuffer == NULL))
            {
                if (prevPrimBegin->primType == primBegin.primType)
                {
                    mergePrimNode = GL_TRUE;
                }
                else
                {
                    if ((prevPrimBegin->primType >= GL_LINES && prevPrimBegin->primType <= GL_LINE_STRIP) &&
                        (primBegin.primType >= GL_LINES && primBegin.primType <= GL_LINE_STRIP))
                    {
                        prevPrimBegin->primType = GL_LINE_STRIP;
                        mergePrimNode = GL_TRUE;
                    }
                }
            }
        }

        /* Allocate dlop primitive node and insert the node into the list.
        */
        if (mergePrimNode)
        {
            dlop = __glDlistAllocOp(gc, vertexCount * primBegin.totalStrideDW * sizeof(GLfloat));
            if (dlop == NULL) return;
            dlop->opcode = __glop_PrimContinue;
            dlop->primType = mode;
            __glDlistAppendOp(gc, dlop);

            /* Add the current primBegin.vertexCount to vertexCount in prevPrimBegin */
            prevPrimBegin->vertexCount += primBegin.vertexCount;
            prevPrimBegin->primCount++;

            /* Setup vertex data buffer pointer and edgeflag buffer pointer */
            bufptr = (GLfloat *)(dlop + 1);
            edgeptr = primBegin.edgeflagBuffer;
        }
        else
        {
            dlop = __glDlistAllocOp(gc,
                (sizeof(__GLPrimBegin) + vertexCount * primBegin.totalStrideDW * sizeof(GLfloat)));
            if (dlop == NULL) return;
            dlop->opcode = __glop_Primitive;
            dlop->dlistFree = __glDlistFreePrimitive;
            dlop->dlistFreePrivateData = __glDlistFreePrivateData;
            dlop->primType = mode;
            __glDlistAppendOp(gc, dlop);

            /* Setup vertex data buffer pointer and edgeflag buffer pointer */
            bufptr = (GLfloat *)((GLubyte *)(dlop + 1) + sizeof(__GLPrimBegin));
            edgeptr = primBegin.edgeflagBuffer;
        }

        /*
        * If a buffer object is bound to the ELEMENT_ARRAY target,
        * then indices is an offset into this buffer object
        */
        /* to do */
         GL_ASSERT(0);
        /* bufObj = gc->bufferObject.boundTarget[__GL_ELEMENT_ARRAY_BUFFER_INDEX];*/

        if (bufObj) {
            /* to do */
            GL_ASSERT(0);
            /* indices = (GLubyte*)(*gc->dp.mapBufferRange)(gc, bufObj, __GL_ELEMENT_ARRAY_BUFFER_INDEX, 0, bufObj->size, GL_MAP_READ_BIT)
                       + (GLuint)(ULONG_PTR)(elemIndices); */
        }
        else {
            indices = (GLvoid *)elemIndices;
        }

        /* Copy the array elements into the primitive dlop buffer.
        */
        switch (type) {
        case GL_UNSIGNED_BYTE:
            if (gc->vertexArray.optdlArrayElement)
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    (*gc->vertexArray.optdlArrayElement)(gc, ((GLubyte *)indices)[i],
                                    &bufptr);
                }
            }
            else
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    error = __glArrayElement_Generic(gc, ((GLubyte *)indices)[i],
                                    &bufptr, &edgeptr, tagBuf);
                }
                if (error != GL_NO_ERROR) {
                    __gllc_Error(gc, error);
                    return;
                }
            }
            break;

        case GL_UNSIGNED_SHORT:
            if (gc->vertexArray.optdlArrayElement)
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    (*gc->vertexArray.optdlArrayElement)(gc, ((GLushort *)indices)[i],
                                    &bufptr);
                }
            }
            else
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    error = __glArrayElement_Generic(gc, ((GLushort *)indices)[i],
                                    &bufptr, &edgeptr, tagBuf);
                }
                if (error != GL_NO_ERROR) {
                    __gllc_Error(gc, error);
                    return;
                }
            }
            break;

        case GL_UNSIGNED_INT:
            if (gc->vertexArray.optdlArrayElement)
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    (*gc->vertexArray.optdlArrayElement)(gc, ((GLuint *)indices)[i],
                                    &bufptr);
                }
            }
            else
            {
                for (i = startIndex; i < startIndex + vertexCount; i++) {
                    error = __glArrayElement_Generic(gc, ((GLuint *)indices)[i],
                                    &bufptr, &edgeptr, tagBuf);
                }
                if (error != GL_NO_ERROR) {
                    __gllc_Error(gc, error);
                    return;
                }
            }
            break;
        }

        if (!mergePrimNode) {
            /* Copy primBegin data into the primitive dlop node */
            (*(__GLPrimBegin *)(dlop + 1)) = primBegin;
        }

        startIndex += vertexCount;
        if (startIndex + vertexCount > count) {
            vertexCount = count - startIndex;
        }
    }

    return;
}

GLvoid APIENTRY __gllc_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{

    __GLdlistOp *dlop;
    __GLvertexArrayState *pV = &gc->vertexArray.boundVAO->vertexArray;
    GLuint tagBuf[__GL_TOTAL_VERTEX_ATTRIBUTES];
    __GLPrimBegin primBegin, *prevPrimBegin = gcvNULL;
    GLfloat *bufptr;
    GLubyte *edgeptr;
    GLint i, vertexCount, startIndex;
    GLuint mergePrimNode;
    GLenum error = GL_NO_ERROR;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->immedModeDispatch.DrawArrays)(gc, mode, first, count);
    }

    if (mode > GL_TRIANGLE_STRIP_ADJACENCY_EXT) {
        __gllc_InvalidEnum(gc);
        return;
    }

    if (count < 0) {
        __gllc_InvalidValue(gc);
        return;
    }

    CHECK_VERTEX_COUNT();

    /*
    ** Just return if vertex array is not enabled since
    ** vertex state is undefined after DrawElements.
    */
    if (!(pV->attribEnabled & __GL_VARRAY_VERTEX)) {
        return;
    }

    /*
    ** Break a large triangle list into multiple smaller triangle lists.
    */
    vertexCount = count;
    if (mode == GL_TRIANGLES && count > __GL_MAX_DLIST_VERTEX_NUMBER) {
        vertexCount = __GL_MAX_DLIST_VERTEX_NUMBER;
    }

    startIndex = 0;
    while (startIndex < count)
    {
        /* Setup __GLPrimBegin header for the primitive.
        */
        __glComputeArrayPrimBegin(gc, mode, vertexCount, &primBegin);

        /* Check if the current primitive node can be merged with a previous PrimBegin node.
        */
        mergePrimNode = GL_FALSE;
        if (gc->dlist.listData.lastPrimNode)
        {
            prevPrimBegin = (__GLPrimBegin *)(gc->dlist.listData.lastPrimNode + 1);

            /* We don't have to compare elemOffsetDW[i] since __glComputeArrayPrimBegin function
            ** will always generate the same elemOffsetDW[i] for a primBegin.primitiveFormat.
            */
            if (((prevPrimBegin->vertexCount + primBegin.vertexCount) < __GL_MAX_DLIST_VERTEX_NUMBER) &&
                (prevPrimBegin->primitiveFormat == primBegin.primitiveFormat) &&
                (prevPrimBegin->edgeflagBuffer == NULL) && (primBegin.edgeflagBuffer == NULL))
            {
                if (prevPrimBegin->primType == primBegin.primType)
                {
                    mergePrimNode = GL_TRUE;
                }
                else
                {
                    if ((prevPrimBegin->primType >= GL_LINES && prevPrimBegin->primType <= GL_LINE_STRIP) &&
                        (primBegin.primType >= GL_LINES && primBegin.primType <= GL_LINE_STRIP))
                    {
                        prevPrimBegin->primType = GL_LINE_STRIP;
                        mergePrimNode = GL_TRUE;
                    }
                }
            }
        }

        /* A locate dlop primitive node and insert the node into the list.
        */
        if (mergePrimNode)
        {
            dlop = __glDlistAllocOp(gc, vertexCount * primBegin.totalStrideDW * sizeof(GLfloat));
            if (dlop == NULL) return;
            dlop->opcode = __glop_PrimContinue;
            dlop->primType = mode;
            __glDlistAppendOp(gc, dlop);

            /* Add the current primBegin.vertexCount to vertexCount in prevPrimBegin */
            prevPrimBegin->vertexCount += primBegin.vertexCount;
            prevPrimBegin->primCount++;

            /* Setup vertex data buffer pointer and edgeflag buffer pointer */
            bufptr = (GLfloat *)(dlop + 1);
            edgeptr = primBegin.edgeflagBuffer;
        }
        else
        {
            dlop = __glDlistAllocOp(gc,
                (sizeof(__GLPrimBegin) + vertexCount * primBegin.totalStrideDW * sizeof(GLfloat)));
            if (dlop == NULL) return;
            dlop->opcode = __glop_Primitive;
            dlop->dlistFree = __glDlistFreePrimitive;
            dlop->dlistFreePrivateData = __glDlistFreePrivateData;
            dlop->primType = mode;
            __glDlistAppendOp(gc, dlop);

            /* Setup vertex data buffer pointer and edgeflag buffer pointer */
            bufptr = (GLfloat *)((GLubyte *)(dlop + 1) + sizeof(__GLPrimBegin));
            edgeptr = primBegin.edgeflagBuffer;
        }

        /* Copy the array elements into the primitive dlop buffer.
        */
        if (gc->vertexArray.optdlArrayElement)
        {
            for (i = startIndex; i < startIndex + vertexCount; i++) {
                (*gc->vertexArray.optdlArrayElement)(gc, first + i,
                                &bufptr);
            }
        }
        else
        {
            for (i = startIndex; i < startIndex + vertexCount; i++) {
                error = __glArrayElement_Generic(gc, first + i,
                                &bufptr, &edgeptr, tagBuf);
            }
            if (error != GL_NO_ERROR) {
                __gllc_Error(gc, error);
                return;
            }
        }

        if (!mergePrimNode) {
            /* Copy primBegin data into the primitive dlop node */
            (*(__GLPrimBegin *)(dlop + 1)) = primBegin;
        }

        startIndex += vertexCount;
        if (startIndex + vertexCount > count) {
            vertexCount = count - startIndex;
        }
    }

    return;
}

GLvoid APIENTRY __gllc_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end,
                            GLsizei count, GLenum type, const GLvoid *elemIndices)
{

    if (end < start) {
        __gllc_InvalidValue(gc);
        return;
    }

    __gllc_DrawElements(gc, mode, count, type, elemIndices);
}

GLvoid APIENTRY __gllc_MultiDrawElements(__GLcontext *gc, GLenum mode, const GLsizei *count,
                            GLenum type, const GLvoid *const*indices, GLsizei primcount)
{
    GLint i;
    for (i = 0; i < primcount; i++) {
        if (count[i] > 0)
            __gllc_DrawElements(gc, mode, count[i], type, indices[i]);
    }
}

GLvoid APIENTRY __gllc_MultiDrawArrays(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    GLint i;
    for (i = 0; i < primcount; i++)
    {
        if (count[i] > 0)
            __gllc_DrawArrays(gc, mode, first[i], count[i]);
    }
}

GLvoid APIENTRY __gllc_ArrayElement(__GLcontext *gc, GLint element)
{

    GLfloat dataBuf[__GL_TOTAL_VERTEX_ATTRIBUTES * 4];
    GLuint tagBuf[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLboolean edgeflag;
    GLfloat *bufptr = (GLfloat *)&dataBuf[0];
    GLubyte *edgeptr = (GLubyte *)&edgeflag;
    GLint i, index, loop;
    GLenum error;

    if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE) {
        (*gc->immedModeDispatch.ArrayElement)(gc, element);
    }

    __GL_MEMZERO(tagBuf, __GL_TOTAL_VERTEX_ATTRIBUTES * sizeof(GLuint));

    /* Copy the arrayElement data into dataBuf[] */

    error = __glArrayElement_Generic(gc, element, &bufptr, &edgeptr, tagBuf);
    if (error != GL_NO_ERROR) {
        __gllc_Error(gc, error);
        return;
    }

    /* Dispatch the arrayElement data to the right API functions */

    i = 0;
    loop = GL_TRUE;
    bufptr = (GLfloat *)&dataBuf[0];
    while (loop) {
        switch (tagBuf[i])
        {
        case __GL_V2F_TAG:
            (*gc->dlCompileDispatch.Vertex2fv)(gc, bufptr);
            bufptr += 2;
            loop = GL_FALSE;
            break;
        case __GL_V3F_TAG:
            (*gc->dlCompileDispatch.Vertex3fv)(gc, bufptr);
            bufptr += 3;
            loop = GL_FALSE;
            break;
        case __GL_V4F_TAG:
            (*gc->dlCompileDispatch.Vertex4fv)(gc, bufptr);
            bufptr += 4;
            loop = GL_FALSE;
            break;
        case __GL_C3F_TAG:
            (*gc->dlCompileDispatch.Color3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_C4F_TAG:
            (*gc->dlCompileDispatch.Color4fv)(gc, bufptr);
            bufptr += 4;
            break;
        case __GL_C4UB_TAG:
            (*gc->dlCompileDispatch.Color4ubv)(gc, (GLubyte *)bufptr);
            bufptr += 1;
            break;
        case __GL_N3F_TAG:
            (*gc->dlCompileDispatch.Normal3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_TC2F_TAG:
            (*gc->dlCompileDispatch.TexCoord2fv)(gc, bufptr);
            bufptr += 2;
            break;
        case __GL_TC2F_U1_TAG:
        case __GL_TC2F_U2_TAG:
        case __GL_TC2F_U3_TAG:
        case __GL_TC2F_U4_TAG:
        case __GL_TC2F_U5_TAG:
        case __GL_TC2F_U6_TAG:
        case __GL_TC2F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC2F_TAG);
            (*gc->dlCompileDispatch.MultiTexCoord2fv)(gc, index, bufptr);
            bufptr += 2;
            break;
        case __GL_TC3F_TAG:
            (*gc->dlCompileDispatch.TexCoord3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_TC3F_U1_TAG:
        case __GL_TC3F_U2_TAG:
        case __GL_TC3F_U3_TAG:
        case __GL_TC3F_U4_TAG:
        case __GL_TC3F_U5_TAG:
        case __GL_TC3F_U6_TAG:
        case __GL_TC3F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC3F_TAG);
            (*gc->dlCompileDispatch.MultiTexCoord3fv)(gc, index, bufptr);
            bufptr += 3;
            break;
        case __GL_TC4F_TAG:
            (*gc->dlCompileDispatch.TexCoord4fv)(gc, bufptr);
            bufptr += 4;
            break;
        case __GL_TC4F_U1_TAG:
        case __GL_TC4F_U2_TAG:
        case __GL_TC4F_U3_TAG:
        case __GL_TC4F_U4_TAG:
        case __GL_TC4F_U5_TAG:
        case __GL_TC4F_U6_TAG:
        case __GL_TC4F_U7_TAG:
            index = GL_TEXTURE0 + (tagBuf[i] - __GL_TC4F_TAG);
            (*gc->dlCompileDispatch.MultiTexCoord4fv)(gc, index, bufptr);
            bufptr += 4;
            break;
        case __GL_EDGEFLAG_TAG:
            (*gc->dlCompileDispatch.EdgeFlag)(gc, edgeflag);
            break;
        case __GL_SC3F_TAG:
            (*gc->dlCompileDispatch.SecondaryColor3fv)(gc, bufptr);
            bufptr += 3;
            break;
        case __GL_FOG1F_TAG:
            (*gc->dlCompileDispatch.FogCoordfv)(gc, bufptr);
            bufptr += 1;
            break;
        case __GL_AT4F_I0_TAG:
        case __GL_AT4F_I1_TAG:
        case __GL_AT4F_I2_TAG:
        case __GL_AT4F_I3_TAG:
        case __GL_AT4F_I4_TAG:
        case __GL_AT4F_I5_TAG:
        case __GL_AT4F_I6_TAG:
        case __GL_AT4F_I7_TAG:
        case __GL_AT4F_I8_TAG:
        case __GL_AT4F_I9_TAG:
        case __GL_AT4F_I10_TAG:
        case __GL_AT4F_I11_TAG:
        case __GL_AT4F_I12_TAG:
        case __GL_AT4F_I13_TAG:
        case __GL_AT4F_I14_TAG:
        case __GL_AT4F_I15_TAG:
            index = (tagBuf[i] - __GL_AT4F_I0_TAG);
            (*gc->dlCompileDispatch.VertexAttrib4fv)(gc, index, bufptr);
            bufptr += 4;
            break;

        default:
            loop = GL_FALSE;
            break;
        }
        i++;
    }
}
