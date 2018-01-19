/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_types.h"
#include "gc_es_context.h"
#include "g_lcomp.h"
#include "memmgr.h"
#include "../gc_es_object_inline.c"

extern GLsizei minVertexNumber[];
extern GLuint fmtIndex2InputIndex[];

/*
** Display list execution function table
*/
extern const GLubyte * (*__glListExecFuncTable[])(__GLcontext *, const GLubyte *);


/*
** Display op end function.
*/
const GLubyte *__glle_Sentinel(__GLcontext *gc, const GLubyte *PC)
{
    return (NULL);
}

GLvoid __glDlistFreePrivateData(__GLcontext *gc, GLubyte *data)
{
    __GLPrimBegin *primBegin = (__GLPrimBegin * )data;

    if (primBegin->privateData || primBegin->ibPrivateData)
    {
        /* Notify Dp to delete cached vertex buffer in video memory */
        /* to do*/
        /*
        (*gc->dp.updatePrivateData)(gc, (GLvoid *)primBegin, __GL_DL_CACHE);
        */
    }
}

/*
** Free indexBuffer and primStartAddr in __GLPrimBegin
*/
GLvoid __glDlistFreePrimitive(__GLcontext *gc, GLubyte *data)
{
    __GLPrimBegin *primBegin = (__GLPrimBegin * )data;

    if (primBegin->primStartAddr)
    {
        (*gc->imports.free)(gc, primBegin->primStartAddr);
        primBegin->primStartAddr = NULL;
    }

    if (primBegin->primVertCount)
    {
        (*gc->imports.free)(gc, primBegin->primVertCount);
        primBegin->primVertCount = NULL;
    }

    if (primBegin->indexBuffer)
    {
        (*gc->imports.free)(gc, primBegin->indexBuffer);
        primBegin->indexBuffer = NULL;
    }

    if (primBegin->edgeflagBuffer)
    {
        (*gc->imports.free)(gc, primBegin->edgeflagBuffer);
        primBegin->edgeflagBuffer = NULL;
    }

    if (primBegin->privateData)
    {
        /* Notify Dp to delete cached vertex buffer in video memory */
        /* to do */
        /*(*gc->dp.deletePrimData)(gc, primBegin->privateData);*/
        primBegin->privateData = NULL;
    }

    if (primBegin->ibPrivateData)
    {
        /* Notify Dp to delete cached vertex buffer in video memory */
        /* to do */
        /*(*gc->dp.deletePrimData)(gc, primBegin->ibPrivateData);*/
        primBegin->ibPrivateData = NULL;
    }

    if (primBegin->privStreamInfo)
    {
        /* to do */
        /*(*gc->dp.deleteStreamInfo)(gc, primBegin->privStreamInfo);*/
        primBegin->privStreamInfo = NULL;
    }
}

/*
** Free all private data of display lists, this function is called when mode change
** or display device config changes if cache is in video memory.
*/
GLvoid __glFreeDlistVertexCache(__GLcontext *gc)
{
    __GLobjItem **buckets;
    __GLobjItem *hdr;
    __GLdlist *dlist;
    GLuint freeCount;
    __GLDlistFreeFns *freeRec;
    GLuint i;

    if (gc->dlist.shared->linearTable)
    {
        /* Free the privateData of all display lists.
        */
        for (i = 0; i < gc->dlist.shared->linearTableSize; i++)
        {
            dlist = gc->dlist.shared->linearTable[i];
            if (dlist)
            {
                if (dlist->freefunc)
                {
                    freeCount = *(GLuint *)dlist->freefunc;
                    freeRec = (__GLDlistFreeFns *)((GLuint *)dlist->freefunc + 1);
                    while (freeCount--)
                    {
                        (*freeRec->freePrivateDataFn)(gc, freeRec->data);
                        freeRec++;
                    }
                }
            }
        }
    }
    else
    {
        buckets = gc->dlist.shared->hashBuckets;
        if (buckets != NULL)
        {
            /* Free every list's privateData on each hash bucket chain.
            */
            for (i = 0; i < gc->dlist.shared->hashSize; i++)
            {
                hdr = buckets[i];
                while (hdr)
                {
                    dlist = (__GLdlist *)hdr->obj;
                    if (dlist->freefunc)
                    {
                        freeCount = *(GLuint *)dlist->freefunc;
                        freeRec = (__GLDlistFreeFns *)((GLuint *)dlist->freefunc + 1);
                        while (freeCount--)
                        {
                            (*freeRec->freePrivateDataFn)(gc, freeRec->data);
                            freeRec++;
                        }
                    }

                    hdr = hdr->next;
                }
            }
        }
    }
}

__GL_INLINE GLvoid __glBreakLargeTriangleList(__GLcontext *gc, __GLdlistOp *dlistop, __GLPrimBegin *prim)
{
    __GLdlistOp *dlBegin, *dlEnd;

    if (prim->vertexCount >= __GL_MAX_DLIST_VERTEX_NUMBER &&
        prim->primType == GL_TRIANGLES)
    {
        /* Insert a glEnd here to break the large triangle list */
        dlEnd = __glDlistAllocOp(gc, 0);
        dlEnd->opcode = __glop_End;
        dlEnd->next = dlistop->next;
        dlistop->next = dlEnd;

        /* Insert a glBegin after the glEnd to start another triangle list */
        dlBegin = __glDlistAllocOp(gc, sizeof(struct __gllc_Begin_Rec));
        dlBegin->opcode = __glop_Begin;
        ((struct __gllc_Begin_Rec *)(dlBegin + 1))->primType = GL_TRIANGLES;
        dlBegin->next = dlEnd->next;
        dlEnd->next = dlBegin;
    }
}

/*
** Display list optimization routine which optimizes consistent primitives.
*/
GLvoid __glOptimizeDisplaylist(__GLcontext *gc, __GLcompiledDlist *cdlist)
{
    __GLdlistOp *dlistop = cdlist->dlist;
    __GLdlistOp *beginOp, *endOp, *primOp, *primBeginOp = NULL;
    GLuint firstVertex, index, input, inputMask, datasize, startNewPrim, discardVertexNum;
    GLuint64 primFormat, fmtMask;
    __GLPrimBegin prim, *primBegin;
    GLfloat *currentPtr[__GL_TOTAL_VERTEX_ATTRIBUTES];
    GLfloat *startAddr, *v;

    primOp = NULL;

    while (dlistop)
    {
        /* Start checking and optimization on this primitive.
        */
        if (dlistop->opcode == __glop_Begin)
        {
            __GL_MEMZERO(&prim, sizeof(__GLPrimBegin));
            prim.primType = ((struct __gllc_Begin_Rec *)(dlistop + 1))->primType;
            prim.primCount = 1;

            /* Skip glBegin */
            beginOp = dlistop;
            dlistop = dlistop->next;
            firstVertex = GL_TRUE;

            /* Compute the primitive's format according to the first vertex */
            while (firstVertex) {
                switch (dlistop->opcode) {
                case __glop_Vertex2fv:
                    prim.primInputMask |= __GL_INPUT_VERTEX;
                    prim.primitiveFormat |= __GL_V2F_BIT;
                    __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_V2F_TAG);
                    prim.elemOffsetDW[__GL_INPUT_VERTEX_INDEX] = prim.totalStrideDW;
                    prim.elemSizeDW[__GL_INPUT_VERTEX_INDEX] = 2;
                    prim.totalStrideDW += 2;
                    prim.elementCount += 1;
                    firstVertex = GL_FALSE;
                    break;
                case __glop_Vertex3fv:
                    prim.primInputMask |= __GL_INPUT_VERTEX;
                    prim.primitiveFormat |= __GL_V3F_BIT;
                    __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_V3F_TAG);
                    prim.elemOffsetDW[__GL_INPUT_VERTEX_INDEX] = prim.totalStrideDW;
                    prim.elemSizeDW[__GL_INPUT_VERTEX_INDEX] = 3;
                    prim.totalStrideDW += 3;
                    prim.elementCount += 1;
                    firstVertex = GL_FALSE;
                    break;
                case __glop_Vertex4fv:
                    prim.primInputMask |= __GL_INPUT_VERTEX;
                    prim.primitiveFormat |= __GL_V4F_BIT;
                    __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_V4F_TAG);
                    prim.elemOffsetDW[__GL_INPUT_VERTEX_INDEX] = prim.totalStrideDW;
                    prim.elemSizeDW[__GL_INPUT_VERTEX_INDEX] = 4;
                    prim.totalStrideDW += 4;
                    prim.elementCount += 1;
                    firstVertex = GL_FALSE;
                    break;
                case __glop_Normal3fv:
                    if ((prim.primInputMask & __GL_INPUT_NORMAL) == 0) {
                        prim.primInputMask |= __GL_INPUT_NORMAL;
                        prim.primitiveFormat |= __GL_N3F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_N3F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_NORMAL_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_NORMAL_INDEX] = 3;
                        prim.totalStrideDW += 3;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_Color3fv:
                    if ((prim.primInputMask & __GL_INPUT_DIFFUSE) == 0) {
                        prim.primInputMask |= __GL_INPUT_DIFFUSE;
                        prim.primitiveFormat |= __GL_C3F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_C3F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_DIFFUSE_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_DIFFUSE_INDEX] = 3;
                        prim.totalStrideDW += 3;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_Color4fv:
                    if ((prim.primInputMask & __GL_INPUT_DIFFUSE) == 0) {
                        prim.primInputMask |= __GL_INPUT_DIFFUSE;
                        prim.primitiveFormat |= __GL_C4F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_V4F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_DIFFUSE_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_DIFFUSE_INDEX] = 4;
                        prim.totalStrideDW += 4;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_Color4ubv:
                    if ((prim.primInputMask & __GL_INPUT_DIFFUSE) == 0) {
                        prim.primInputMask |= __GL_INPUT_DIFFUSE;
                        prim.primitiveFormat |= __GL_C4UB_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_C4UB_TAG);
                        prim.elemOffsetDW[__GL_INPUT_DIFFUSE_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_DIFFUSE_INDEX] = 1; /* RGBA in 1 DWORD */
                        prim.totalStrideDW += 1;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_SecondaryColor3fv:
                    if ((prim.primInputMask & __GL_INPUT_SPECULAR) == 0) {
                        prim.primInputMask |= __GL_INPUT_SPECULAR;
                        prim.primitiveFormat |= __GL_SC3F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_SC3F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_SPECULAR_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_SPECULAR_INDEX] = 3;
                        prim.totalStrideDW += 3;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_FogCoordf:
                    if ((prim.primInputMask & __GL_INPUT_FOGCOORD) == 0) {
                        prim.primInputMask |= __GL_INPUT_FOGCOORD;
                        prim.primitiveFormat |= __GL_FOG1F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_FOG1F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_FOGCOORD_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_FOGCOORD_INDEX] = 1;
                        prim.totalStrideDW += 1;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_TexCoord2fv:
                    if ((prim.primInputMask & __GL_INPUT_TEX0) == 0) {
                        prim.primInputMask |= __GL_INPUT_TEX0;
                        prim.primitiveFormat |= __GL_TC2F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC2F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX] = 2;
                        prim.totalStrideDW += 2;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_TexCoord3fv:
                    if ((prim.primInputMask & __GL_INPUT_TEX0) == 0) {
                        prim.primInputMask |= __GL_INPUT_TEX0;
                        prim.primitiveFormat |= __GL_TC3F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC3F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX] = 3;
                        prim.totalStrideDW += 3;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_TexCoord4fv:
                    if ((prim.primInputMask & __GL_INPUT_TEX0) == 0) {
                        prim.primInputMask |= __GL_INPUT_TEX0;
                        prim.primitiveFormat |= __GL_TC4F_BIT;
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC4F_TAG);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX] = 4;
                        prim.totalStrideDW += 4;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_MultiTexCoord2fv:
                    index = ((struct __gllc_MultiTexCoord2fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    inputMask = (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + index));
                    if ((prim.primInputMask & inputMask) == 0) {
                        prim.primInputMask |= inputMask;
                        prim.primitiveFormat |= (__GL_ONE_64 << (__GL_TC2F_INDEX + index));
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC2F_TAG + index);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX + index] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX + index] = 2;
                        prim.totalStrideDW += 2;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_MultiTexCoord3fv:
                    index = ((struct __gllc_MultiTexCoord3fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    inputMask = (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + index));
                    if ((prim.primInputMask & inputMask) == 0) {
                        prim.primInputMask |= inputMask;
                        prim.primitiveFormat |= (__GL_ONE_64 << (__GL_TC3F_INDEX + index));
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC3F_TAG + index);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX + index] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX + index] = 3;
                        prim.totalStrideDW += 3;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_MultiTexCoord4fv:
                    index = ((struct __gllc_MultiTexCoord4fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    inputMask = (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + index));
                    if ((prim.primInputMask & inputMask) == 0) {
                        prim.primInputMask |= inputMask;
                        prim.primitiveFormat |= (__GL_ONE_64 << (__GL_TC4F_INDEX + index));
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_TC4F_TAG + index);
                        prim.elemOffsetDW[__GL_INPUT_TEX0_INDEX + index] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_TEX0_INDEX + index] = 4;
                        prim.totalStrideDW += 4;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_VertexAttrib4fv:
                    index = ((struct __gllc_VertexAttrib4fv_Rec *)(dlistop + 1))->index;
                    inputMask = (__GL_ONE_32 << (__GL_INPUT_ATT0_INDEX + index));
                    if ((prim.primInputMask & inputMask) == 0) {
                        prim.primInputMask |= inputMask;
                        prim.primitiveFormat |= (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + index));
                        __GL_PRIM_ELEMENT(prim.primElemSequence, __GL_AT4F_I0_TAG + index);
                        prim.elemOffsetDW[__GL_INPUT_ATT0_INDEX + index] = prim.totalStrideDW;
                        prim.elemSizeDW[__GL_INPUT_ATT0_INDEX + index] = 4;
                        prim.totalStrideDW += 4;
                        prim.elementCount += 1;
                    }
                    else {
                        /* Skip optimization if there is duplicated vertex attribute */
                        goto Next_dlistop;
                    }
                    break;
                case __glop_End:
                    /* It is an empty primitive so skip the begin, end dlistop */
                    beginOp->opcode = __glop_Skip;
                    dlistop->opcode = __glop_Skip;
                    goto Next_dlistop;
                default:
                    /* Skip optimization */
                    goto Next_dlistop;
                }

                dlistop = dlistop->next;
            }

            /* Compare the primFormat of following vertices with the first vertex */
            prim.vertexCount = 1;
            primFormat = 0;
            while (dlistop->opcode != __glop_End)
            {
                switch (dlistop->opcode) {
                case __glop_Vertex2fv:
                    primFormat |= __GL_V2F_BIT;
                    if ((primFormat & prim.primitiveFormat) != primFormat) {
                        /* Skip optimization if primFormat is not a subset of prim.primitiveFormat */
                        goto Next_dlistop;
                    }
                    primFormat = 0;
                    prim.vertexCount += 1;
                    __glBreakLargeTriangleList(gc, dlistop, &prim);
                    break;
                case __glop_Vertex3fv:
                    primFormat |= __GL_V3F_BIT;
                    if ((primFormat & prim.primitiveFormat) != primFormat) {
                        /* Skip optimization if primFormat is not a subset of prim.primitiveFormat */
                        goto Next_dlistop;
                    }
                    primFormat = 0;
                    prim.vertexCount += 1;
                    __glBreakLargeTriangleList(gc, dlistop, &prim);
                    break;
                case __glop_Vertex4fv:
                    primFormat |= __GL_V4F_BIT;
                    if ((primFormat & prim.primitiveFormat) != primFormat) {
                        /* Skip optimization if primFormat is not a subset of prim.primitiveFormat */
                        goto Next_dlistop;
                    }
                    primFormat = 0;
                    prim.vertexCount += 1;
                    __glBreakLargeTriangleList(gc, dlistop, &prim);
                    break;
                case __glop_Normal3fv:
                    primFormat |= __GL_N3F_BIT;
                    break;
                case __glop_Color3fv:
                    primFormat |= __GL_C3F_BIT;
                    break;
                case __glop_Color4fv:
                    primFormat |= __GL_C4F_BIT;
                    break;
                case __glop_Color4ubv:
                    primFormat |= __GL_C4UB_BIT;
                    break;
                case __glop_SecondaryColor3fv:
                    primFormat |= __GL_SC3F_BIT;
                    break;
                case __glop_FogCoordf:
                    primFormat |= __GL_FOG1F_BIT;
                    break;
                case __glop_TexCoord2fv:
                    primFormat |= __GL_TC2F_BIT;
                    break;
                case __glop_TexCoord3fv:
                    primFormat |= __GL_TC3F_BIT;
                    break;
                case __glop_TexCoord4fv:
                    primFormat |= __GL_TC4F_BIT;
                    break;
                case __glop_MultiTexCoord2fv:
                    index = ((struct __gllc_MultiTexCoord2fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    primFormat |= (__GL_ONE_64 << (__GL_TC2F_INDEX + index));
                    break;
                case __glop_MultiTexCoord3fv:
                    index = ((struct __gllc_MultiTexCoord3fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    primFormat |= (__GL_ONE_64 << (__GL_TC3F_INDEX + index));
                    break;
                case __glop_MultiTexCoord4fv:
                    index = ((struct __gllc_MultiTexCoord4fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    primFormat |= (__GL_ONE_64 << (__GL_TC4F_INDEX + index));
                    break;
                case __glop_VertexAttrib4fv:
                    index = ((struct __gllc_VertexAttrib4fv_Rec *)(dlistop + 1))->index;
                    primFormat |= (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + index));
                    break;
                default:
                    /* Skip optimization */
                    goto Next_dlistop;
                }

                dlistop = dlistop->next;
            }

            if(primFormat != 0){
                /* If there are attrib after last glvertex, skip opt. */
                goto Next_dlistop;
            }

            /* Cache the glEnd listop */
            endOp = dlistop;

            /* Make sure the vertexCount is valid for primType.
            */
            discardVertexNum = 0;
            if (prim.vertexCount < minVertexNumber[prim.primType]) {
                discardVertexNum = prim.vertexCount;
            }
            else {
                switch (prim.primType)
                {
                case GL_TRIANGLES:
                    discardVertexNum = prim.vertexCount % 3;
                    break;
                case GL_LINES:
                    discardVertexNum = prim.vertexCount % 2;
                    break;
                case GL_QUADS:
                    discardVertexNum = prim.vertexCount % 4;
                    break;
                case GL_QUAD_STRIP:
                    discardVertexNum = prim.vertexCount % 2;
                    break;
                }
            }
            /* Discard the extra vetices and set the extra dlistops to __glop_Skip.
            */
            if (discardVertexNum) {
                prim.vertexCount -= discardVertexNum;

                if (prim.vertexCount == 0) {
                    dlistop = beginOp;
                    while (dlistop != endOp) {
                        dlistop->opcode = __glop_Skip;
                        dlistop = dlistop->next;
                    }
                    dlistop->opcode = __glop_Skip;
                    /* Skip optimization */
                    goto Next_dlistop;
                }
                else {
                    GLint i = 0;
                    dlistop = beginOp;
                    while (dlistop != endOp) {
                        if (dlistop->opcode == __glop_Vertex2fv ||
                            dlistop->opcode == __glop_Vertex3fv ||
                            dlistop->opcode == __glop_Vertex4fv) {
                                i += 1;
                                if (i == prim.vertexCount) {
                                    break;
                                }
                            }
                            dlistop = dlistop->next;
                    }
                    dlistop = dlistop->next;
                    while (dlistop != endOp) {
                        dlistop->opcode = __glop_Skip;
                        dlistop = dlistop->next;
                    }
                    dlistop->opcode = __glop_Skip;
                }
            }

            /* If the current primitive is right after a primOp node and they have the same
            ** primType and primitiveFormat, or if the first primitive is Line primtive and
            ** followed by another Line primitive, then the current primitive will be coalesced
            ** with the prevoius primOp node.
            */
            startNewPrim = GL_TRUE;
            if (primOp && primOp->next == beginOp) {
                /* Set primBegin to the primitive begin node */
                primBegin = (__GLPrimBegin *)(primBeginOp + 1);

                /* Check the conditions and decide either to restart a new primitive or continue.
                */
                if ((primBegin->vertexCount + prim.vertexCount) < __GL_MAX_DLIST_VERTEX_NUMBER &&
                    primBegin->primitiveFormat == prim.primitiveFormat)
                {
                    if (primBegin->primType == prim.primType)
                    {
                        startNewPrim = GL_FALSE;
                    }
                    else
                    {
                        if ((primBegin->primType >= GL_LINES && primBegin->primType <= GL_LINE_STRIP) &&
                            (prim.primType >= GL_LINES && prim.primType <= GL_LINE_STRIP))
                        {
                            primBegin->primType = GL_LINE_STRIP;
                            startNewPrim = GL_FALSE;
                        }
                    }
                }
            }

            if (startNewPrim) {
                /* Allocate a new __GLdlistOp for the primitive */
                datasize = sizeof(__GLPrimBegin) + (prim.totalStrideDW << 2) * prim.vertexCount;
                primOp = __glDlistAllocOp(gc, datasize);
                if (primOp == NULL) return;
                primOp->opcode = __glop_Primitive;
                primOp->primType = prim.primType;
                startAddr = (GLfloat *)((GLubyte *)(primOp + 1) + sizeof(__GLPrimBegin));

                /* Cache the primOp as primBeginOp and initialize its __GLPrimBegin  */
                primBeginOp = primOp;
                primBeginOp->dlistFree = __glDlistFreePrimitive;
                primBeginOp->dlistFreePrivateData = __glDlistFreePrivateData;
                primBegin = (__GLPrimBegin *)(primBeginOp + 1);
                *primBegin = prim;
            }
            else {
                /* Allocate a new __GLdlistOp for the primitive */
                datasize = (prim.totalStrideDW << 2) * prim.vertexCount;
                primOp = __glDlistAllocOp(gc, datasize);
                if (primOp == NULL) return;
                primOp->opcode = __glop_PrimContinue;
                primOp->primType = prim.primType;
                startAddr = (GLfloat *)(primOp + 1);

                /* Add the current prim.vertexCount to vertexCount in primBeginOp */
                primBegin = (__GLPrimBegin *)(primBeginOp + 1);
                primBegin->primCount += 1;
                primBegin->vertexCount += prim.vertexCount;

                /* Make sure the current primitive use the same element offsets as
                the prevoius primitive */
                __GL_MEMCOPY(prim.elemOffsetDW, primBegin->elemOffsetDW,
                    sizeof(GLuint) * __GL_TOTAL_VERTEX_ATTRIBUTES);
            }

            /* Setup the currentPtr[] array according to elemOffset[] */
            input = 0;
            inputMask = prim.primInputMask;
            while (inputMask) {
                if (inputMask & 0x1) {
                    currentPtr[input] = startAddr - prim.totalStrideDW + prim.elemOffsetDW[input];
                }
                input += 1;
                inputMask >>= 1;
            }

            /* Now pack this consistent primitive into a single __GLdlistOp.
            ** At the same time, extract the bounding box from all vertices.
            */
            dlistop = beginOp;
            while (dlistop != endOp) {
                switch (dlistop->opcode) {
                case __glop_Begin:
                    break;

                case __glop_Vertex2fv:
                    /* Fill in the missing attributes */
                    primFormat |= __GL_V2F_BIT;
                    fmtMask = prim.primitiveFormat & ~primFormat;
                    index = 0;
                    while (fmtMask) {
                        if (fmtMask & 0x1) {
                            input = fmtIndex2InputIndex[index];
                            switch (index) {
                                case __GL_TC2F_INDEX:
                                case __GL_TC2F_U1_INDEX:
                                case __GL_TC2F_U2_INDEX:
                                case __GL_TC2F_U3_INDEX:
                                case __GL_TC2F_U4_INDEX:
                                case __GL_TC2F_U5_INDEX:
                                case __GL_TC2F_U6_INDEX:
                                case __GL_TC2F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord2fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord2fv_Rec *)v;
                                    break;
                                case __GL_N3F_INDEX:
                                case __GL_C3F_INDEX:
                                case __GL_SC3F_INDEX:
                                case __GL_TC3F_INDEX:
                                case __GL_TC3F_U1_INDEX:
                                case __GL_TC3F_U2_INDEX:
                                case __GL_TC3F_U3_INDEX:
                                case __GL_TC3F_U4_INDEX:
                                case __GL_TC3F_U5_INDEX:
                                case __GL_TC3F_U6_INDEX:
                                case __GL_TC3F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord3fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord3fv_Rec *)v;
                                    break;
                                case __GL_C4F_INDEX:
                                case __GL_TC4F_INDEX:
                                case __GL_TC4F_U1_INDEX:
                                case __GL_TC4F_U2_INDEX:
                                case __GL_TC4F_U3_INDEX:
                                case __GL_TC4F_U4_INDEX:
                                case __GL_TC4F_U5_INDEX:
                                case __GL_TC4F_U6_INDEX:
                                case __GL_TC4F_U7_INDEX:
                                case __GL_AT4F_I0_INDEX:
                                case __GL_AT4F_I1_INDEX:
                                case __GL_AT4F_I2_INDEX:
                                case __GL_AT4F_I3_INDEX:
                                case __GL_AT4F_I4_INDEX:
                                case __GL_AT4F_I5_INDEX:
                                case __GL_AT4F_I6_INDEX:
                                case __GL_AT4F_I7_INDEX:
                                case __GL_AT4F_I8_INDEX:
                                case __GL_AT4F_I9_INDEX:
                                case __GL_AT4F_I10_INDEX:
                                case __GL_AT4F_I11_INDEX:
                                case __GL_AT4F_I12_INDEX:
                                case __GL_AT4F_I13_INDEX:
                                case __GL_AT4F_I14_INDEX:
                                case __GL_AT4F_I15_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord4fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord4fv_Rec *)v;
                                    break;
                                case __GL_FOG1F_INDEX:
                                case __GL_C4UB_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_FogCoordf_Rec *)currentPtr[input] =
                                        *(struct __gllc_FogCoordf_Rec *)v;
                                    break;
                                default:
                                    GL_ASSERT(0);
                            }
                        }
                        index++;
                        fmtMask >>= 1;
                    }

                    /* Copy the vertex data */
                    input = __GL_INPUT_VERTEX_INDEX;
                    currentPtr[input] += prim.totalStrideDW;
                    *(struct __gllc_Vertex2fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Vertex2fv_Rec *)(dlistop + 1);
                    primFormat = 0;
                    break;

                case __glop_Vertex3fv:
                    /* Fill in the missing attributes */
                    primFormat |= __GL_V3F_BIT;
                    fmtMask = prim.primitiveFormat & ~primFormat;
                    index = 0;
                    while (fmtMask) {
                        if (fmtMask & 0x1) {
                            input = fmtIndex2InputIndex[index];
                            switch (index) {
                                case __GL_TC2F_INDEX:
                                case __GL_TC2F_U1_INDEX:
                                case __GL_TC2F_U2_INDEX:
                                case __GL_TC2F_U3_INDEX:
                                case __GL_TC2F_U4_INDEX:
                                case __GL_TC2F_U5_INDEX:
                                case __GL_TC2F_U6_INDEX:
                                case __GL_TC2F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord2fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord2fv_Rec *)v;
                                    break;
                                case __GL_N3F_INDEX:
                                case __GL_C3F_INDEX:
                                case __GL_SC3F_INDEX:
                                case __GL_TC3F_INDEX:
                                case __GL_TC3F_U1_INDEX:
                                case __GL_TC3F_U2_INDEX:
                                case __GL_TC3F_U3_INDEX:
                                case __GL_TC3F_U4_INDEX:
                                case __GL_TC3F_U5_INDEX:
                                case __GL_TC3F_U6_INDEX:
                                case __GL_TC3F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord3fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord3fv_Rec *)v;
                                    break;
                                case __GL_C4F_INDEX:
                                case __GL_TC4F_INDEX:
                                case __GL_TC4F_U1_INDEX:
                                case __GL_TC4F_U2_INDEX:
                                case __GL_TC4F_U3_INDEX:
                                case __GL_TC4F_U4_INDEX:
                                case __GL_TC4F_U5_INDEX:
                                case __GL_TC4F_U6_INDEX:
                                case __GL_TC4F_U7_INDEX:
                                case __GL_AT4F_I0_INDEX:
                                case __GL_AT4F_I1_INDEX:
                                case __GL_AT4F_I2_INDEX:
                                case __GL_AT4F_I3_INDEX:
                                case __GL_AT4F_I4_INDEX:
                                case __GL_AT4F_I5_INDEX:
                                case __GL_AT4F_I6_INDEX:
                                case __GL_AT4F_I7_INDEX:
                                case __GL_AT4F_I8_INDEX:
                                case __GL_AT4F_I9_INDEX:
                                case __GL_AT4F_I10_INDEX:
                                case __GL_AT4F_I11_INDEX:
                                case __GL_AT4F_I12_INDEX:
                                case __GL_AT4F_I13_INDEX:
                                case __GL_AT4F_I14_INDEX:
                                case __GL_AT4F_I15_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord4fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord4fv_Rec *)v;
                                    break;
                                case __GL_FOG1F_INDEX:
                                case __GL_C4UB_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_FogCoordf_Rec *)currentPtr[input] =
                                        *(struct __gllc_FogCoordf_Rec *)v;
                                    break;
                                default:
                                    GL_ASSERT(0);
                            }
                        }
                        index++;
                        fmtMask >>= 1;
                    }

                    /* Copy the vertex data */
                    input = __GL_INPUT_VERTEX_INDEX;
                    currentPtr[input] += prim.totalStrideDW;
                    *(struct __gllc_Vertex3fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Vertex3fv_Rec *)(dlistop + 1);
                    primFormat = 0;
                    break;

                case __glop_Vertex4fv:
                    /* Fill in the missing attributes */
                    primFormat |= __GL_V4F_BIT;
                    fmtMask = prim.primitiveFormat & ~primFormat;
                    index = 0;
                    while (fmtMask) {
                        if (fmtMask & 0x1) {
                            input = fmtIndex2InputIndex[index];
                            switch (index) {
                                case __GL_TC2F_INDEX:
                                case __GL_TC2F_U1_INDEX:
                                case __GL_TC2F_U2_INDEX:
                                case __GL_TC2F_U3_INDEX:
                                case __GL_TC2F_U4_INDEX:
                                case __GL_TC2F_U5_INDEX:
                                case __GL_TC2F_U6_INDEX:
                                case __GL_TC2F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord2fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord2fv_Rec *)v;
                                    break;
                                case __GL_N3F_INDEX:
                                case __GL_C3F_INDEX:
                                case __GL_SC3F_INDEX:
                                case __GL_TC3F_INDEX:
                                case __GL_TC3F_U1_INDEX:
                                case __GL_TC3F_U2_INDEX:
                                case __GL_TC3F_U3_INDEX:
                                case __GL_TC3F_U4_INDEX:
                                case __GL_TC3F_U5_INDEX:
                                case __GL_TC3F_U6_INDEX:
                                case __GL_TC3F_U7_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord3fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord3fv_Rec *)v;
                                    break;
                                case __GL_C4F_INDEX:
                                case __GL_TC4F_INDEX:
                                case __GL_TC4F_U1_INDEX:
                                case __GL_TC4F_U2_INDEX:
                                case __GL_TC4F_U3_INDEX:
                                case __GL_TC4F_U4_INDEX:
                                case __GL_TC4F_U5_INDEX:
                                case __GL_TC4F_U6_INDEX:
                                case __GL_TC4F_U7_INDEX:
                                case __GL_AT4F_I0_INDEX:
                                case __GL_AT4F_I1_INDEX:
                                case __GL_AT4F_I2_INDEX:
                                case __GL_AT4F_I3_INDEX:
                                case __GL_AT4F_I4_INDEX:
                                case __GL_AT4F_I5_INDEX:
                                case __GL_AT4F_I6_INDEX:
                                case __GL_AT4F_I7_INDEX:
                                case __GL_AT4F_I8_INDEX:
                                case __GL_AT4F_I9_INDEX:
                                case __GL_AT4F_I10_INDEX:
                                case __GL_AT4F_I11_INDEX:
                                case __GL_AT4F_I12_INDEX:
                                case __GL_AT4F_I13_INDEX:
                                case __GL_AT4F_I14_INDEX:
                                case __GL_AT4F_I15_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_TexCoord4fv_Rec *)currentPtr[input] =
                                        *(struct __gllc_TexCoord4fv_Rec *)v;
                                    break;
                                case __GL_FOG1F_INDEX:
                                case __GL_C4UB_INDEX:
                                    v = currentPtr[input];
                                    currentPtr[input] += prim.totalStrideDW;
                                    *(struct __gllc_FogCoordf_Rec *)currentPtr[input] =
                                        *(struct __gllc_FogCoordf_Rec *)v;
                                    break;
                                default:
                                    GL_ASSERT(0);
                            }
                        }
                        index++;
                        fmtMask >>= 1;
                    }

                    /* Copy the vertex data */
                    input = __GL_INPUT_VERTEX_INDEX;
                    currentPtr[input] += prim.totalStrideDW;
                    *(struct __gllc_Vertex4fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Vertex4fv_Rec *)(dlistop + 1);
                    primFormat = 0;
                    break;

                case __glop_Normal3fv:
                    input = __GL_INPUT_NORMAL_INDEX;
                    if ((primFormat & __GL_N3F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_Normal3fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Normal3fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_N3F_BIT;
                    break;
                case __glop_Color3fv:
                    input = __GL_INPUT_DIFFUSE_INDEX;
                    if ((primFormat & __GL_C3F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_Color3fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Color3fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_C3F_BIT;
                    break;
                case __glop_Color4fv:
                    input = __GL_INPUT_DIFFUSE_INDEX;
                    if ((primFormat & __GL_C4F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_Color4fv_Rec *)currentPtr[input] =
                        *(struct __gllc_Color4fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_C4F_BIT;
                    break;
                case __glop_Color4ubv:
                    input = __GL_INPUT_DIFFUSE_INDEX;
                    if ((primFormat & __GL_C4UB_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_Color4ubv_Rec *)currentPtr[input] =
                        *(struct __gllc_Color4ubv_Rec *)(dlistop + 1);
                    primFormat |= __GL_C4UB_BIT;
                    break;
                case __glop_SecondaryColor3fv:
                    input = __GL_INPUT_SPECULAR_INDEX;
                    if ((primFormat & __GL_SC3F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_SecondaryColor3fv_Rec *)currentPtr[input] =
                        *(struct __gllc_SecondaryColor3fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_SC3F_BIT;
                    break;
                case __glop_FogCoordf:
                    input = __GL_INPUT_FOGCOORD_INDEX;
                    if ((primFormat & __GL_FOG1F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_FogCoordf_Rec *)currentPtr[input] =
                        *(struct __gllc_FogCoordf_Rec *)(dlistop + 1);
                    primFormat |= __GL_FOG1F_BIT;
                    break;
                case __glop_TexCoord2fv:
                    input = __GL_INPUT_TEX0_INDEX;
                    if ((primFormat & __GL_TC2F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_TexCoord2fv_Rec *)currentPtr[input] =
                        *(struct __gllc_TexCoord2fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_TC2F_BIT;
                    break;
                case __glop_TexCoord3fv:
                    input = __GL_INPUT_TEX0_INDEX;
                    if ((primFormat & __GL_TC3F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_TexCoord3fv_Rec *)currentPtr[input] =
                        *(struct __gllc_TexCoord3fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_TC3F_BIT;
                    break;
                case __glop_TexCoord4fv:
                    input = __GL_INPUT_TEX0_INDEX;
                    if ((primFormat & __GL_TC4F_BIT) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(struct __gllc_TexCoord4fv_Rec *)currentPtr[input] =
                        *(struct __gllc_TexCoord4fv_Rec *)(dlistop + 1);
                    primFormat |= __GL_TC4F_BIT;
                    break;
                case __glop_MultiTexCoord2fv:
                    index = ((struct __gllc_MultiTexCoord2fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    fmtMask = (__GL_ONE_64 << (__GL_TC2F_INDEX + index));
                    input = __GL_INPUT_TEX0_INDEX + index;
                    if ((primFormat & fmtMask) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(__GLvertex2 *)currentPtr[input] = *(__GLvertex2 *)
                        &((struct __gllc_MultiTexCoord2fv_Rec *)(dlistop + 1))->v[0];
                    primFormat |= fmtMask;
                    break;
                case __glop_MultiTexCoord3fv:
                    index = ((struct __gllc_MultiTexCoord3fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    fmtMask = (__GL_ONE_64 << (__GL_TC3F_INDEX + index));
                    input = __GL_INPUT_TEX0_INDEX + index;
                    if ((primFormat & fmtMask) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(__GLvertex3 *)currentPtr[input] = *(__GLvertex3 *)
                        &((struct __gllc_MultiTexCoord3fv_Rec *)(dlistop + 1))->v[0];
                    primFormat |= fmtMask;
                    break;
                case __glop_MultiTexCoord4fv:
                    index = ((struct __gllc_MultiTexCoord4fv_Rec *)(dlistop + 1))->texture;
                    index -= GL_TEXTURE0;
                    fmtMask = (__GL_ONE_64 << (__GL_TC4F_INDEX + index));
                    input = __GL_INPUT_TEX0_INDEX + index;
                    if ((primFormat & fmtMask) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(__GLvertex4 *)currentPtr[input] = *(__GLvertex4 *)
                        &((struct __gllc_MultiTexCoord4fv_Rec *)(dlistop + 1))->v[0];
                    primFormat |= fmtMask;
                    break;
                case __glop_VertexAttrib4fv:
                    index = ((struct __gllc_VertexAttrib4fv_Rec *)(dlistop + 1))->index;
                    fmtMask = (__GL_ONE_64 << (__GL_AT4F_I0_INDEX + index));
                    input = __GL_INPUT_ATT0_INDEX + index;
                    if ((primFormat & fmtMask) == 0) {
                        currentPtr[input] += prim.totalStrideDW;
                    }
                    *(__GLvertex4 *)currentPtr[input] = *(__GLvertex4 *)
                        &((struct __gllc_VertexAttrib4fv_Rec *)(dlistop + 1))->v[0];
                    primFormat |= fmtMask;
                    break;
                case __glop_Skip:
                    /* Skip the node */
                    break;
                default:
                    /* Error here! */
                    GL_ASSERT(0);
                }

                /* Set this dlistop free */
                dlistop->opcode = __glop_Skip;
                dlistop = dlistop->next;
            }

            /* Set glEnd dlistop free */
            dlistop->opcode = __glop_Skip;

            /* Copy the bounding box of prim structure back to primBegin */
            primBegin = (__GLPrimBegin *)(primBeginOp + 1);

            /* Insert the new primitive primOp into the link list after glEnd */
            primOp->next = dlistop->next;
            dlistop->next = primOp;
            dlistop = primOp;
        }

Next_dlistop:
        dlistop = dlistop->next;
    }
}

/*****************************************************************************************/


__GLdlist * __glAllocateDlist(__GLcontext *gc, GLuint segsize, GLuint freeCount, GLuint name)
{
    __GLdlist *dlist;
    size_t memsize;
    GLuint *tmp;

    dlist = (__GLdlist *)__glGetObject(gc, gc->dlist.shared, name);

    /* If app wants to re-define an already defined DL, then we have to delete the old DL first.
    */
    if (dlist && (dlist->freefunc || dlist->segment))
    {
        __glDeleteObject(gc, gc->dlist.shared, dlist->name);
        dlist = NULL;
    }

    if (dlist == NULL)
    {
        dlist = (__GLdlist *)(*gc->imports.calloc)(gc, 1, sizeof(__GLdlist) );
        if (dlist == NULL)
        {
            __glSetError( gc, GL_OUT_OF_MEMORY);
            return (NULL);
        }
    }

    if (freeCount)
    {
        memsize = sizeof(__GLDlistFreeFns) * freeCount + sizeof(GLuint);
        dlist->freefunc = (GLubyte *)(*gc->imports.calloc)(gc, 1, memsize );
        if (dlist->freefunc == NULL)
        {
            __glSetError( gc, GL_OUT_OF_MEMORY);
            return (NULL);
        }
        tmp = (GLuint *)dlist->freefunc;
        *tmp = freeCount;
    }

    /* Calling function is 1st reference to dlist */
    dlist->refcount = 1;

    /* Initialize boundBox in dlist */
    dlist->name = name;
    dlist->concatenatable = GL_TRUE;

    if (segsize)
    {
        dlist->segment = (GLubyte *)(*gc->imports.malloc)(gc, segsize );
        if (dlist->segment == NULL)
        {
            (*gc->imports.free)(gc, dlist);
            __glSetError( gc, GL_OUT_OF_MEMORY);
            return (NULL);
        }
    }

    return (dlist);
}

GLvoid __glGeneratePrimIndexStream(__GLcontext *gc, GLuint vertexCount, GLuint currentIndex,
                                 __GLPrimBegin *primBegin, GLenum currentPrimType)
{
    GLuint index, startIndex, indexCount, i;
    GLushort *indexBuf;

    /* Generate vertex index stream */
    indexBuf = primBegin->indexBuffer;
    indexCount = primBegin->indexCount;
    index = startIndex = currentIndex;

    GL_ASSERT((index + 3)< __glMaxUshort);

    switch (currentPrimType)
    {
    case GL_TRIANGLES:
        for (i = 0; i < vertexCount; i += 3)
        {
            indexBuf[indexCount++] = (GLshort)index;
            indexBuf[indexCount++] = (GLshort)index + 1;
            indexBuf[indexCount++] = (GLshort)index + 2;
            index = index + 3;
        }
        break;
    case GL_TRIANGLE_STRIP:
        indexBuf[indexCount++] = (GLshort)index;
        indexBuf[indexCount++] = (GLshort)index + 1;
        indexBuf[indexCount++] = (GLshort)index + 2;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            if (i & 1)
            {
                indexBuf[indexCount++] = (GLshort)index - 1;
                indexBuf[indexCount++] = (GLshort)index - 2;
                indexBuf[indexCount++] = (GLshort)index;
                index++;
            }
            else
            {
                indexBuf[indexCount++] = (GLshort)index - 2;
                indexBuf[indexCount++] = (GLshort)index - 1;
                indexBuf[indexCount++] = (GLshort)index;
                index++;
            }
        }
        break;
    case GL_TRIANGLE_FAN:
        indexBuf[indexCount++] = (GLshort)index;
        indexBuf[indexCount++] = (GLshort)index + 1;
        indexBuf[indexCount++] = (GLshort)index + 2;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            indexBuf[indexCount++] = (GLshort)startIndex;
            indexBuf[indexCount++] = (GLshort)index - 1;
            indexBuf[indexCount++] = (GLshort)index;
            index++;
        }
        break;
    case GL_POLYGON:
        indexBuf[indexCount++] = (GLshort)index + 1;
        indexBuf[indexCount++] = (GLshort)index + 2;
        indexBuf[indexCount++] = (GLshort)index;
        index = index + 3;

        for (i = 3; i < vertexCount; i++)
        {
            indexBuf[indexCount++] = (GLshort)index - 1;
            indexBuf[indexCount++] = (GLshort)index;
            indexBuf[indexCount++] = (GLshort)startIndex;
            index++;
        }
        break;
    case GL_QUADS:
        for (i = 0; i < vertexCount; i += 4)
        {
            indexBuf[indexCount++] = (GLushort)index;
            indexBuf[indexCount++] = (GLushort)index + 1;
            indexBuf[indexCount++] = (GLushort)index + 3;
            indexBuf[indexCount++] = (GLushort)index + 1;
            indexBuf[indexCount++] = (GLushort)index + 2;
            indexBuf[indexCount++] = (GLushort)index + 3;
            index = index + 4;
        }
        break;
    case GL_QUAD_STRIP:
        for (i = 0; i < (vertexCount - 2); i += 2)
        {
            indexBuf[indexCount++] = (GLshort)index;
            indexBuf[indexCount++] = (GLshort)index + 1;
            indexBuf[indexCount++] = (GLshort)index + 3;
            indexBuf[indexCount++] = (GLshort)index + 2;
            indexBuf[indexCount++] = (GLshort)index;
            indexBuf[indexCount++] = (GLshort)index + 3;
            index = index + 2;
        }
        break;
    case GL_LINE_STRIP:
        indexBuf[indexCount++] = (GLshort)index;
        indexBuf[indexCount++] = (GLshort)index + 1;
        index = index + 2;

        for (i = 2; i < vertexCount; i++)
        {
            indexBuf[indexCount++] = (GLshort)index - 1;
            indexBuf[indexCount++] = (GLshort)index;
            index++;
        }
        break;
    case GL_LINE_LOOP:
        indexBuf[indexCount++] = (GLshort)index;
        indexBuf[indexCount++] = (GLshort)index + 1;
        index = index + 2;

        for (i = 2; i < vertexCount; i++)
        {
            indexBuf[indexCount++] = (GLshort)index - 1;
            indexBuf[indexCount++] = (GLshort)index;
            index++;
        }

        indexBuf[indexCount++] = (GLshort)index - 1;
        indexBuf[indexCount++] = (GLshort)startIndex;
        break;
    case GL_LINES:
        for (i = 0; i < vertexCount; i += 2)
        {
            indexBuf[indexCount++] = (GLshort)index;
            indexBuf[indexCount++] = (GLshort)index + 1;
            index += 2;
        }
        break;
    }
    primBegin->indexCount = indexCount;
}

GLvoid __glProcessPrimitiveOp(__GLcontext *gc, GLint primIndex, GLuint vertexCount, GLuint currentIndex,
                            __GLPrimBegin *primBegin, GLenum currentPrimType, GLubyte *data)
{
    /* For disjoint primitives, simply combine and return.
    */
    if (primBegin->primType == GL_TRIANGLES ||
        primBegin->primType == GL_QUADS ||
        primBegin->primType == GL_LINES ||
        primBegin->primType == GL_POINTS)
    {
        primBegin->primCount = 1;
        return;
    }

    /* Allocate vertex indexBuffer */
    if (primIndex == 0)
    {
        primBegin->indexBuffer = (GLushort *)
            (*gc->imports.malloc)(gc, 3 * primBegin->vertexCount * sizeof(GLushort) );
        if (primBegin->indexBuffer == NULL)
        {
            __glSetError( gc, GL_OUT_OF_MEMORY);
            return;
        }
    }

    /* Save the primitive begin addess and vertexCount for each primitive if necessary */
    if (primBegin->primCount > 1 && primBegin->primType >= GL_TRIANGLES)
    {
        if (primIndex == 0)
        {
            /* Allocate primStartAddr, primVertCount buffers */
            primBegin->primStartAddr = (GLfloat **)
                (*gc->imports.malloc)(gc, primBegin->primCount * sizeof(GLfloat*) );
            primBegin->primVertCount = (GLint *)
                (*gc->imports.malloc)(gc, primBegin->primCount * sizeof(GLuint) );
            if (primBegin->primStartAddr == NULL || primBegin->primVertCount == NULL)
            {
                __glSetError( gc, GL_OUT_OF_MEMORY);
                return;
            }

            primBegin->primStartAddr[primIndex] = (GLfloat *)(data + sizeof(__GLPrimBegin));
            primBegin->primVertCount[primIndex] = vertexCount;
        }
        else
        {
            primBegin->primStartAddr[primIndex] = (GLfloat *)(data);
            primBegin->primVertCount[primIndex] = vertexCount;
        }
    }

    /* Generate vertex index stream */
    __glGeneratePrimIndexStream(gc, vertexCount, currentIndex, primBegin, currentPrimType);
}

/*
** Now we call the implementation-specific compiler. It does the following:
**
** compute and allocate space for compiled dlist
**
** creates an implmentation-specific compiled dlist using information
** from linked list.
**
** creates a dlist node of type __GLdlist
**
** fills in all elements of __GLdlist structure appropriately;
**
**    attaching the compiled display list as one or more segments,
**
**    attaching an execute function to display the compiled list,
**
**    allocating and filling in any local data needed by the execute
**    or free functions,
**
**    attaching a free function that will delete the list, local data, and
**    __GLdlist node.
**
** returns a pointer to the decorated __GLdlist structure
*/
__GLdlist * __glCompileDisplayList(__GLcontext *gc, __GLcompiledDlist *compDlist)
{
    __GLdlistOp *dlistop;
    GLubyte *data;
    __GLlistExecFunc *fp;
    __GLDlistFreeFns *freeFnArray;
    GLuint totalSize, vertexCount, currentIndex = 0;
    GLint freeCount, primIndex = 0;
    __GLdlist *dlist;
    __GLPrimBegin *primBegin = NULL;

    /*
    ** Now we compress the chain of display list ops into an optimized dlist.
    */
    totalSize = 0;
    freeCount = 0;
    for (dlistop = compDlist->dlist; dlistop; dlistop = dlistop->next)
    {
        if(dlistop->opcode != __glop_Skip)
        {
            totalSize += dlistop->size + sizeof(__GLlistExecFunc *);

            if (dlistop->dlistFree)
            {
                freeCount++;
            }
        }
    }
    /* Space needed for sentinel function; __glle_Sentinel() */
    totalSize += sizeof(__GLlistExecFunc *);

    dlistop = compDlist->dlist;
    if (dlistop)
    {
        /* Allocate dlist structure */
        dlist = __glAllocateDlist(gc, totalSize, freeCount, gc->dlist.currentList);
        if (dlist == NULL)
        {
            /* No memory! */
            __glArenaFreeAll(gc->dlist.arena);
            compDlist->dlist     = NULL;
            compDlist->lastDlist = NULL;
            gc->dlist.currentList = 0;
            __glSetError( gc, GL_OUT_OF_MEMORY);
            return (NULL);
        }

        /* Local memory used to hold free functions, if any */
        freeFnArray = (__GLDlistFreeFns *)
            ((GLubyte *)(dlist->freefunc + sizeof(GLuint)));

        data = dlist->segment;
        for (; dlistop; dlistop = dlistop->next)
        {
            GLshort opcode = dlistop->opcode;

            if (opcode == __glop_Skip)
                continue;

            if (opcode != __glop_PrimContinue)
            {
                /* Look up list execute function based on opcode.
                */
                fp = (__GLlistExecFunc*)__glListExecFuncTable[opcode];

                *((__GLlistExecFunc **)data) = fp;

                data += sizeof(__GLlistExecFunc *);

                if (dlistop->dlistFree)
                {
                    freeFnArray->freeFn = dlistop->dlistFree;
                    freeFnArray->freePrivateDataFn = dlistop->dlistFreePrivateData;
                    freeFnArray->data = data;
                    freeFnArray++;
                }
            }

            /* Copy all the dlistop data to contiguous memory segment dlist->segment.
            */
            __GL_MEMCOPY(data, (dlistop + 1), dlistop->size);

            switch (opcode)
            {
            case __glop_Primitive:
                primBegin = (__GLPrimBegin *)data;
                dlist->vertexCount += primBegin->vertexCount;
                dlist->primitiveCount += 1;
                primIndex = 0;
                currentIndex = 0;
                vertexCount = (dlistop->size - sizeof(__GLPrimBegin)) / (primBegin->totalStrideDW << 2);
                /* Generate vertex indices */
                __glProcessPrimitiveOp(gc, primIndex, vertexCount, currentIndex,
                                        primBegin, dlistop->primType, data);
                currentIndex += vertexCount;
                break;
            case __glop_PrimContinue:
                primIndex += 1;
                vertexCount = dlistop->size / (primBegin->totalStrideDW << 2);
                /* Generate vertex indices */
                __glProcessPrimitiveOp(gc, primIndex, vertexCount, currentIndex,
                                        primBegin, dlistop->primType, data);
                currentIndex += vertexCount;
                break;
            case __glop_Frustum:
            case __glop_LoadIdentity:
            case __glop_LoadMatrixf:
            case __glop_LoadMatrixd:
            case __glop_MultMatrixf:
            case __glop_MultMatrixd:
            case __glop_LoadTransposeMatrixf:
            case __glop_LoadTransposeMatrixd:
            case __glop_MultTransposeMatrixf:
            case __glop_MultTransposeMatrixd:
            case __glop_Ortho:
            case __glop_PopMatrix:
            case __glop_Rotated:
            case __glop_Rotatef:
            case __glop_Scaled:
            case __glop_Scalef:
            case __glop_Translated:
            case __glop_Translatef:
            case __glop_CallLists:
                dlist->concatenatable = GL_FALSE;
                break;
            default:
                dlist->concatenatable = GL_FALSE;
                break;
            }

            data += dlistop->size;
        }

        /* Append sentinel function to end of list */
        *((__GLlistExecFunc **)data) =(__GLlistExecFunc *)__glle_Sentinel;

        if (dlist->concatenatable)
        {
            /* Disable list concatenation if there are more than one primitive node
            ** or the list has large enough vertex numbers.
            */
            if (dlist->primitiveCount > 1 || dlist->vertexCount > (__GL_MAX_VERTEX_NUMBER>>1))
            {
                dlist->concatenatable = GL_FALSE;
            }
        }
    }
    else /* This is an empty display list */
    {
        /* Create an empty dlist */
        dlist = __glAllocateDlist(gc, 0,  0, gc->dlist.currentList);
    }

    return (dlist);
}
