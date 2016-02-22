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


#include "gc_gl_context.h"
#include "chip_context.h"

extern GLvoid updateTextureStates(__GLcontext * gc);
extern gceSTATUS uploadTexture(__GLcontext* gc);

#define _GC_OBJ_ZONE    gcvZONE_API_GL

gceSTATUS
initializeDraw(
    glsCHIPCONTEXT_PTR  chipCtx
    )
{
    gceSTATUS status;
    gctSIZE_T i;
    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    for (i = 0; i < gcmCOUNTOF(chipCtx->attributeArray); ++i)
    {
        chipCtx->attributeArray[i].genericValue[0] = 0.0f;
        chipCtx->attributeArray[i].genericValue[1] = 0.0f;
        chipCtx->attributeArray[i].genericValue[2] = 0.0f;
        chipCtx->attributeArray[i].genericValue[3] = 1.0f;

        chipCtx->attributeArray[i].enable = gcvTRUE;
        chipCtx->attributeArray[i].divisor = 0;
    }

    /* Construct the vertex array. */
    status = gcoVERTEXARRAY_Construct(chipCtx->hal, &chipCtx->vertexArray);

    /* Initialize last primitive type. */
    chipCtx->lastPrimitiveType = -1;
    chipCtx->programDirty = GL_FALSE;

    gcmFOOTER();
    /* Return status. */
    return status;
}

gceSTATUS
deinitializeDraw(
    glsCHIPCONTEXT_PTR  chipCtx
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);
    status = gcoVERTEXARRAY_Destroy(chipCtx->vertexArray);
    gcmFOOTER();
    return status;
}

gceSTATUS vertexArrayBind(
    IN glsCHIPCONTEXT_PTR chipCtx,
    IN GLint First,
    IN GLsizei Count,
    IN gceINDEX_TYPE IndexType,
    IN gcoINDEX index,
    IN const void * Indices,
    IN OUT gcePRIMITIVE * PrimitiveType,
    IN OUT gctUINT * PrimitiveCount
    )
{
    gctSIZE_T i, count;
    gctUINT enableBits = 0;
    gceSTATUS status;
    gctUINT attr = 0, j;
    gctSIZE_T vertexCount;
    gctSIZE_T spilitCount;

    gcmHEADER();


    if (chipCtx->currGLSLProgram)
    {
        for (i = enableBits = 0; i < chipCtx->maxAttributes; ++i)
        {
            gctUINT link;

            /* Get linkage of vertex attribute. */
            link = chipCtx->currGLSLProgram->attributeLinkage[i];

            if (link == -1) {
                continue;
            }

            chipCtx->attributeArray[i].linkage = chipCtx->currGLSLProgram->attributeMap[link];

            if ((link < chipCtx->currGLSLProgram->attributeCount)
            &&  ((chipCtx->currGLSLProgram->attributeEnable >> link) & 1)
            &&  (chipCtx->attributeArray[i].size > 0)
            )
            {
                enableBits |= (1 << i);
            }
        }
    } else {
        /* Get number of attributes for the vertex shader. */
        gcmONERROR(gcSHADER_GetAttributeCount(chipCtx->currProgram->vs.shader,
                                              &count));

        /* Walk all vertex shader attributes. */
        for (i = 0, j = 0; i < count; ++i)
        {
            gctBOOL attributeEnabled;

            /* Get the attribute linkage. */
            attr = chipCtx->currProgram->vs.attributes[i].binding;

            gcmERR_BREAK(gcATTRIBUTE_IsEnabled(
                chipCtx->currProgram->vs.attributes[i].attribute,
                &attributeEnabled
                ));

            if (attributeEnabled)
            {
                /* Link the attribute to the vertex shader input. */
                chipCtx->attributeArray[attr].linkage = j++;

                /* Enable the attribute. */
                enableBits |= 1 << attr;
            }
        }
    }

    /* Bind the vertex array to the hardware. */
    vertexCount = Count;
    gcmONERROR(gcoVERTEXARRAY_Bind(chipCtx->vertexArray,
                                   enableBits, chipCtx->attributeArray,
                                   First, &vertexCount,
                                   IndexType, index, (gctPOINTER) Indices,
                                   PrimitiveType, gcvNULL, &spilitCount, gcvNULL,
                                   PrimitiveCount, gcvNULL, gcvNULL));
    Count = (GLsizei)vertexCount;


    /* Success. */
    gcmFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER();
    return status;
}

GLvoid configStream(__GLcontext* gc, glsCHIPCONTEXT_PTR chipCtx)
{
    glsATTRIBUTEINFO_PTR vertexStream;
    gceVERTEX_FORMAT format = gcvVERTEX_INT;
    GLsizei componentSize;
    GLboolean normalize;
    gcsVERTEXARRAY_PTR vertexPtr;
    glsVERTEXBUFFERINFO* bufInfo;

    static gcSHADER_TYPE attributeType[] =
    {
        (gcSHADER_TYPE) 0,
        gcSHADER_FRAC_X1,
        gcSHADER_FRAC_X2,
        gcSHADER_FRAC_X3,
        gcSHADER_FRAC_X4
    };

    static gcSHADER_TYPE varyingType[] =
    {
        (gcSHADER_TYPE) 0,
        gcSHADER_FLOAT_X1,
        gcSHADER_FLOAT_X2,
        gcSHADER_FLOAT_X3,
        gcSHADER_FLOAT_X4
    };

    static gcSL_SWIZZLE varyingSwizzle[] =
    {
        (gcSL_SWIZZLE) 0,
        gcSL_SWIZZLE_XXXX,
        gcSL_SWIZZLE_XYYY,
        gcSL_SWIZZLE_XYZZ,
        gcSL_SWIZZLE_XYZW
    };

    GLuint streamIdx;
    GLuint elementIdx;
    GLuint missingAttr, attrIndex = 0;

    for (streamIdx = 0; streamIdx < gcmCOUNTOF(chipCtx->attributeInfo); streamIdx++) {
        chipCtx->attributeInfo[streamIdx].streamEnabled = GL_FALSE;
        chipCtx->attributeInfo[streamIdx].dirty = GL_TRUE;
    }

    chipCtx->hashKey.hashPointSizeStreamEnabled = 0;
    chipCtx->hashKey.hashColorStreamEnabled = 0;
    chipCtx->hashKey.hashNormalStreamEnabled = 0;
    chipCtx->hashKey.hashTexCoordStreamEnabled = 0;
    chipCtx->hashKey.hashSecondColorStreamEnabled = 0;
    chipCtx->hashKey.hashFogCoordStreamEnabled = 0;

    for(streamIdx = 0; streamIdx < gc->vertexStreams.numStreams; streamIdx++)
    {
        __GLstreamDecl * stream = &(gc->vertexStreams.streams[streamIdx]);

        for(elementIdx = 0; elementIdx < stream->numElements; elementIdx++)
        {
            __GLvertexElement* element = NULL;
            element = &(stream->streamElement[elementIdx]);
            /* Get internal format and component size. */
            switch (element->type)
            {
            case GL_FLOAT:
                format = gcvVERTEX_FLOAT;
                componentSize = sizeof(GLfloat);
                break;

             case GL_HALF_FLOAT_ARB:
                format = gcvVERTEX_HALF;
                componentSize = sizeof(GLshort);
                break;

            case GL_FIXED_ONLY_ARB:
                format = gcvVERTEX_FIXED;
                componentSize = sizeof(GLfixed);
                break;

            case GL_UNSIGNED_INT:
                format = gcvVERTEX_UNSIGNED_INT;
                componentSize = sizeof(GLuint);
                break;

            case GL_INT:
                format = gcvVERTEX_INT;
                componentSize = sizeof(GLint);
                break;

            case GL_UNSIGNED_SHORT:
                format = gcvVERTEX_UNSIGNED_SHORT;
                componentSize = sizeof(GLushort);
                break;

            case GL_SHORT:
                format = gcvVERTEX_SHORT;
                componentSize = sizeof(GLshort);
                break;

            case GL_UNSIGNED_BYTE:
                format = gcvVERTEX_UNSIGNED_BYTE;
                componentSize = sizeof(GLubyte);
                break;

            case GL_BYTE:
                format = gcvVERTEX_BYTE;
                componentSize = sizeof(GLbyte);
                break;

            default:
                return;
            }

            normalize
                = element->normalized
                && (element->type != GL_FIXED_ONLY_ARB)
                && (element->type != GL_FLOAT)
                && (element->type != GL_HALF_FLOAT_ARB);

            vertexStream = gcvNULL;

            if (element->inputIndex < __GL_INPUT_ATT0_INDEX) {
                attrIndex = element->inputIndex;
            } else {
                attrIndex = element->inputIndex - __GL_INPUT_ATT0_INDEX;
            }

            /* Set hash key for enabled stream */
            switch (attrIndex) {
                case __GL_INPUT_DIFFUSE_INDEX:
                    glmSETHASH_1BIT(hashColorStreamEnabled, 1, 0);
                    break;
                case __GL_INPUT_SPECULAR_INDEX:
                    glmSETHASH_1BIT(hashSecondColorStreamEnabled, 1, 0);
                    break;
                case __GL_INPUT_FOGCOORD_INDEX:
                    glmSETHASH_1BIT(hashFogCoordStreamEnabled, 1, 0);
                    break;
                case __GL_INPUT_NORMAL_INDEX:
                    glmSETHASH_1BIT(hashNormalStreamEnabled, 1, 0);
                    break;
                case __GL_INPUT_TEX0_INDEX:
                case __GL_INPUT_TEX1_INDEX:
                case __GL_INPUT_TEX2_INDEX:
                case __GL_INPUT_TEX3_INDEX:
                case __GL_INPUT_TEX4_INDEX:
                case __GL_INPUT_TEX5_INDEX:
                case __GL_INPUT_TEX6_INDEX:
                case __GL_INPUT_TEX7_INDEX:
                    glmSETHASH_1BIT(hashTexCoordStreamEnabled, 1, (attrIndex - __GL_INPUT_TEX0_INDEX));
                    break;
            }

            vertexStream = &chipCtx->attributeInfo[attrIndex];
            vertexPtr = &chipCtx->attributeArray[attrIndex];

            if(vertexStream) {
                /* Mark stream dirty. */
                vertexStream->dirty = GL_TRUE;

                /* Set stream parameters. */
                vertexStream->format         = format;
                vertexStream->normalize      = normalize;
                vertexStream->components     = element->size;
                vertexStream->attributeType  = attributeType[element->size];
                vertexStream->varyingType    = varyingType[element->size];
                vertexStream->varyingSwizzle = varyingSwizzle[element->size];
                vertexStream->stride         = (stream->stride != 0) ? stream->stride
                                                   : (element->size * componentSize);
                /* for VBO streamAddr is zero, otherwise it is real stream address */
                vertexStream->pointer        = (GLbyte *)stream->streamAddr + element->offset;
                vertexStream->attributeSize  = element->size * componentSize;
                vertexStream->streamEnabled = GL_TRUE;
            }

            /* Get vertex array attribute information. */
            vertexPtr->size       = element->size;
            vertexPtr->format     = format;
            vertexPtr->normalized = normalize;
            vertexPtr->stride     = (stream->stride != 0) ? stream->stride
                                          : (element->size * componentSize);
            /* if it is VBO, this is offset from VBO stream buffer */
            vertexPtr->pointer    = vertexStream->pointer ;
            if (stream->privPtrAddr && (*stream->privPtrAddr)) {
                bufInfo = *stream->privPtrAddr;
                vertexPtr->stream     = bufInfo->bufObject;
            } else {
                vertexPtr->stream = gcvNULL;
            }
        }
    }

    missingAttr = gc->vertexStreams.missingAttribs;

    while (missingAttr) {
        if ((missingAttr & 1) && (attrIndex < __GL_INPUT_ATT0_INDEX)) {
            vertexStream = &chipCtx->attributeInfo[attrIndex];
            vertexStream->streamEnabled = GL_FALSE;
            vertexStream->dirty = GL_TRUE;
        }
        attrIndex++;
        missingAttr >>= 1;
    }
}

GLvoid updateDrawPath(__GLcontext *gc, glsCHIPCONTEXT_PTR  chipCtx)
{
    /* Filter out no effect draw first */
    if (gc->state.polygon.frontMode == GL_LINE && gc->state.polygon.backMode == GL_LINE &&
        !gc->state.current.edgeflag && !gc->vertexStreams.edgeflagStream &&
        (gc->vertexStreams.primMode == GL_TRIANGLES ||
         gc->vertexStreams.primMode == GL_QUADS ||
         gc->vertexStreams.primMode == GL_POLYGON))
    {
        gc->dp.ctx.drawPrimitive = __glChipDrawNothing;
    }
    else
    if (gc->state.enables.polygon.cullFace && gc->state.polygon.cullFace == GL_FRONT_AND_BACK &&
       ((gc->vertexStreams.primMode >= GL_TRIANGLES && gc->vertexStreams.primMode <= GL_POLYGON) ||
        (gc->vertexStreams.primMode == GL_TRIANGLES_ADJACENCY_EXT) ||
        (gc->vertexStreams.primMode == GL_TRIANGLE_STRIP_ADJACENCY_EXT)))
    {
        gc->dp.ctx.drawPrimitive = __glChipDrawNothing;
    }
    else
    {
        /* Pick Draw primitive function */
        if (gc->vertexStreams.indexCount != 0) {
            gc->dp.ctx.drawPrimitive = __glChipDrawIndexedPrimitive;
            switch (gc->vertexStreams.primMode) {
                case GL_QUADS:
                    gc->dp.ctx.drawPrimitive = __glChipDrawIndexedQuadListPrimitive;
                    break;
                case GL_QUAD_STRIP:
                    gc->dp.ctx.drawPrimitive = __glChipDrawIndexedQuadStripPrimitive;
                    break;
                case GL_POLYGON:
                    if (gc->state.light.shadingModel != GL_FLAT) {
                        gc->dp.ctx.drawPrimitive = __glChipDrawIndexedPrimitive;
                    } else {
                        gc->dp.ctx.drawPrimitive = __glChipDrawIndexedPolygonPrimitive;
                    }
                    break;
                default:
                    gc->dp.ctx.drawPrimitive = __glChipDrawIndexedPrimitive;
                    break;
            }
        } else {
            switch (gc->vertexStreams.primMode) {
                case GL_QUADS:
                    gc->dp.ctx.drawPrimitive = __glChipDrawQuadListPrimitive;
                    break;
                case GL_QUAD_STRIP:
                    gc->dp.ctx.drawPrimitive = __glChipDrawQuadStripPrimitive;
                    break;
                case GL_POLYGON:
                    if (gc->state.light.shadingModel != GL_FLAT) {
                        gc->dp.ctx.drawPrimitive = __glChipDrawPrimitive;
                    } else {
                        gc->dp.ctx.drawPrimitive = __glChipDrawPolygonPrimitive;
                    }
                    break;
                case GL_LINE_LOOP:
                    if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_LINE_LOOP)) {
                        gc->dp.ctx.drawPrimitive = __glChipDrawLineLoopPrimitive;
                    } else {
                        gc->dp.ctx.drawPrimitive = __glChipDrawPrimitive;
                    }
                    break;
                default:
                    gc->dp.ctx.drawPrimitive = __glChipDrawPrimitive;
            }
        }

        /* determine SW path or HW path below */
    }

#if __GL_ENABLE_HW_NULL
    gc->dp.ctx.drawPrimitive = __glChipDrawNothing;
#endif
}

/**********************************************************************/
/* Implementation for device pipeline draw APIs                       */
/**********************************************************************/
#define COPY_INDEX(dst, src, i, j, type)  \
{                                         \
    type * srcPtr = (type *)src;          \
    type * dstPtr = (type *)dst;          \
    dstPtr += i;                          \
    srcPtr += j;                          \
    *dstPtr = *srcPtr;                    \
}


GLvoid __glChipDrawNothing(__GLcontext* gc)
{
}

GLvoid __glChipDrawIndexedLineLoopPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexCount;
    GLvoid * indexBuf;
    GLvoid * indexSrcBuf;
    GLuint bytesPerIndex = 1;
    GLuint i;
    gceINDEX_TYPE indexType = gcvINDEX_8;
    glsVERTEXBUFFERINFO* bufInfo;

    gcmHEADER_ARG("gc=0x%x", gc);

    halPrimitive = gcvPRIMITIVE_LINE_LIST;
    primCount = gc->vertexStreams.indexCount;

    switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            indexType = gcvINDEX_8;
            break;

        case GL_UNSIGNED_SHORT:
            indexType = gcvINDEX_16;
            bytesPerIndex = 2;
            break;

        case GL_UNSIGNED_INT:
            indexType = gcvINDEX_32;
            bytesPerIndex = 4;
            break;
    }

    indexCount = gc->vertexStreams.indexCount + 1;

    indexBuf = (*gc->imports.malloc)(gc, indexCount * bytesPerIndex);
    if (!gc->vertexStreams.indexStream.ppIndexBufPriv ||
        (gc->vertexStreams.indexStream.ppIndexBufPriv &&
        (*gc->vertexStreams.indexStream.ppIndexBufPriv == gcvNULL)))
    {
        indexSrcBuf = gc->vertexStreams.indexStream.streamAddr;
    } else {
        /* it is buffer object */
        bufInfo = *gc->vertexStreams.indexStream.ppIndexBufPriv;
        gcoSTREAM_Lock(bufInfo->bufObject,
            &bufInfo->bufferMapPointer,
            gcvNULL);
        indexSrcBuf = bufInfo->bufferMapPointer;
    }

    if (indexBuf && indexSrcBuf) {
       switch (gc->vertexStreams.indexStream.type) {
           case GL_UNSIGNED_BYTE:
               for (i = 0; i < primCount; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, i, i, GLubyte);
               }
               COPY_INDEX(indexBuf, indexSrcBuf, primCount, 0, GLubyte);
               break;

           case GL_UNSIGNED_SHORT:
                   for (i = 0; i < primCount; i++) {
                       COPY_INDEX(indexBuf, indexSrcBuf, i, i, GLushort);
                   }
                   COPY_INDEX(indexBuf, indexSrcBuf, primCount, 0, GLushort);
                   break;

           case GL_UNSIGNED_INT:
                   for (i = 0; i < primCount; i++) {
                       COPY_INDEX(indexBuf, indexSrcBuf, i, i, GLuint);
                   }
                   COPY_INDEX(indexBuf, indexSrcBuf, primCount, 0, GLuint);
                   break;
        }
    }

    if (indexBuf) {
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexCount,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawIndexedPolygonPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexCount;
    GLvoid * indexBuf;
    GLvoid * indexSrcBuf;
    GLuint bytesPerIndex = 1;
    GLuint i;
    GLuint index = 0;
    gceINDEX_TYPE indexType = gcvINDEX_8;
    glsVERTEXBUFFERINFO* bufInfo;

    gcmHEADER_ARG("gc=0x%x", gc);

    halPrimitive = gcvPRIMITIVE_TRIANGLE_LIST;
    primCount = gc->vertexStreams.indexCount - 2;

    switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            indexType = gcvINDEX_8;
            break;

        case GL_UNSIGNED_SHORT:
            indexType = gcvINDEX_16;
            bytesPerIndex = 2;
            break;

        case GL_UNSIGNED_INT:
            indexType = gcvINDEX_32;
            bytesPerIndex = 4;
            break;
    }

    indexCount = primCount * 3;

    indexBuf = (*gc->imports.malloc)(gc, indexCount * bytesPerIndex);

    if (!gc->vertexStreams.indexStream.ppIndexBufPriv ||
        (gc->vertexStreams.indexStream.ppIndexBufPriv &&
        (*gc->vertexStreams.indexStream.ppIndexBufPriv == gcvNULL)))
    {
        indexSrcBuf = gc->vertexStreams.indexStream.streamAddr;
    } else {
        /* it is buffer object */
        bufInfo = *gc->vertexStreams.indexStream.ppIndexBufPriv;
        gcoSTREAM_Lock(bufInfo->bufObject,
            &bufInfo->bufferMapPointer,
            gcvNULL);
        indexSrcBuf = bufInfo->bufferMapPointer;
    }

    if (indexBuf && indexSrcBuf) {
        switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            for (i = 0; i < primCount; i++) {
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 1), GLubyte);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 2), GLubyte);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, 0, GLubyte);
                index++;
            }
            break;

        case GL_UNSIGNED_SHORT:
            for (i = 0; i < primCount; i++) {
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 1), GLushort);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 2), GLushort);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, 0, GLushort);
                index++;
            }
            break;

        case GL_UNSIGNED_INT:
            for (i = 0; i < primCount; i++) {
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 1), GLuint);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, (i + 2), GLuint);
                index++;
                COPY_INDEX(indexBuf, indexSrcBuf, index, 0, GLuint);
                index++;
            }
            break;
        }
    }

    if (indexBuf) {
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexCount,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawIndexedQuadListPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLushort * indexBuf;
    GLvoid * indexSrcBuf;
    GLuint i;
    GLuint index = 0;
    GLuint indexCount;
    gceINDEX_TYPE indexType = gcvINDEX_8;
    GLuint bytesPerIndex = 1;
    glsVERTEXBUFFERINFO* bufInfo;

    gcmHEADER_ARG("gc=0x%x", gc);
    /* triangle list list */
    if (gc->state.polygon.frontMode == GL_LINE)
    {
        primCount = gc->vertexStreams.indexCount;
        indexCount = primCount * 2;
        halPrimitive = gcvPRIMITIVE_LINE_LIST;
        gco3D_SetAntiAliasLine(chipCtx->hw, GL_TRUE);
        gco3D_SetAALineWidth(chipCtx->hw, 1);
    }
    else
    {
        primCount = gc->vertexStreams.indexCount / 2;
        indexCount = primCount * 3;
        halPrimitive = gcvPRIMITIVE_TRIANGLE_LIST;
    }

    switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            indexType = gcvINDEX_8;
            break;

        case GL_UNSIGNED_SHORT:
            indexType = gcvINDEX_16;
            bytesPerIndex = 2;
            break;

        case GL_UNSIGNED_INT:
            indexType = gcvINDEX_32;
            bytesPerIndex = 4;
            break;
    }

    indexBuf = (*gc->imports.malloc)(gc, indexCount * bytesPerIndex);

    if (!gc->vertexStreams.indexStream.ppIndexBufPriv ||
        (gc->vertexStreams.indexStream.ppIndexBufPriv &&
        (*gc->vertexStreams.indexStream.ppIndexBufPriv == gcvNULL)))
    {
        indexSrcBuf = gc->vertexStreams.indexStream.streamAddr;
    } else {
        /* it is buffer object */
        bufInfo = *gc->vertexStreams.indexStream.ppIndexBufPriv;
        gcoSTREAM_Lock(bufInfo->bufObject,
            &bufInfo->bufferMapPointer,
            gcvNULL);
        indexSrcBuf = bufInfo->bufferMapPointer;
    }

    if (indexBuf && indexSrcBuf) {
        if (gc->state.polygon.frontMode == GL_LINE)
        {
           switch (gc->vertexStreams.indexStream.type) {
           case GL_UNSIGNED_BYTE:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLubyte);
                   index++;
               }
               break;
           case GL_UNSIGNED_SHORT:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLushort);
                   index++;
               }
               break;
           case GL_UNSIGNED_INT:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLuint);
                   index++;
               }
               break;
           }
        }
        else
        {
           switch (gc->vertexStreams.indexStream.type) {
           case GL_UNSIGNED_BYTE:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLubyte);
                   index++;
               }
               break;
           case GL_UNSIGNED_SHORT:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLushort);
                   index++;
               }
               break;
           case GL_UNSIGNED_INT:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 4 + 3), GLuint);
                   index++;
               }
               break;
           }
        }
    }

    if (indexBuf) {
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexCount,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));
            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);

        (*gc->imports.free)(gc, indexBuf);
        if (gc->state.polygon.frontMode == GL_LINE)
        {
            gco3D_SetAntiAliasLine(chipCtx->hw, GL_FALSE);
        }
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawIndexedQuadStripPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLushort * indexBuf;
    GLvoid * indexSrcBuf;
    GLuint i;
    GLuint index = 0;
    GLuint indexCount;
    gceINDEX_TYPE indexType = gcvINDEX_8;
    GLuint bytesPerIndex = 1;
    glsVERTEXBUFFERINFO* bufInfo;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Primitive Quad list */
    primCount = gc->vertexStreams.indexCount / 2 -1;

    if (gc->state.polygon.frontMode == GL_LINE)
    {
        primCount *= 4;
        halPrimitive = gcvPRIMITIVE_LINE_LIST;
        indexCount = primCount * 2;
        gco3D_SetAntiAliasLine(chipCtx->hw, GL_TRUE);
        gco3D_SetAALineWidth(chipCtx->hw, 1);
    }
    else
    {
        halPrimitive = gcvPRIMITIVE_TRIANGLE_LIST;
        /* triangle list */
        primCount *= 2;
        indexCount = primCount * 3;
    }

    switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            indexType = gcvINDEX_8;
            break;

        case GL_UNSIGNED_SHORT:
            indexType = gcvINDEX_16;
            bytesPerIndex = 2;
            break;

        case GL_UNSIGNED_INT:
            indexType = gcvINDEX_32;
            bytesPerIndex = 4;
            break;
    }



    indexBuf = (*gc->imports.malloc)(gc, indexCount * bytesPerIndex);

    if (!gc->vertexStreams.indexStream.ppIndexBufPriv ||
        (gc->vertexStreams.indexStream.ppIndexBufPriv &&
        (*gc->vertexStreams.indexStream.ppIndexBufPriv == gcvNULL)))
    {
        indexSrcBuf = gc->vertexStreams.indexStream.streamAddr;
    } else {
        /* it is buffer object */
        bufInfo = *gc->vertexStreams.indexStream.ppIndexBufPriv;
        gcoSTREAM_Lock(bufInfo->bufObject,
            &bufInfo->bufferMapPointer,
            gcvNULL);
        indexSrcBuf = bufInfo->bufferMapPointer;
    }

    if (indexBuf && indexSrcBuf) {
        if (gc->state.polygon.frontMode == GL_LINE)
        {
           switch (gc->vertexStreams.indexStream.type) {
           case GL_UNSIGNED_BYTE:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLubyte);
                   index++;
               }
               break;
           case GL_UNSIGNED_SHORT:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLushort);
                   index++;
               }
               break;
           case GL_UNSIGNED_INT:
               for (i = 0; i < primCount / 4; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLuint);
                   index++;
               }
               break;
           }
        }
        else
        {
           switch (gc->vertexStreams.indexStream.type) {
           case GL_UNSIGNED_BYTE:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLubyte);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLubyte);
                   index++;
               }
               break;
           case GL_UNSIGNED_SHORT:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLushort);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLushort);
                   index++;
               }
               break;
           case GL_UNSIGNED_INT:
               for (i = 0; i < primCount / 2; i++) {
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 1), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 3), GLuint);
                   index++;
                   COPY_INDEX(indexBuf, indexSrcBuf, index, (i * 2 + 2), GLuint);
                   index++;
               }
               break;
           }
        }
    }


    if (indexBuf) {
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexCount,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
        if (gc->state.polygon.frontMode == GL_LINE)
        {
            gco3D_SetAntiAliasLine(chipCtx->hw, GL_FALSE);
        }
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawLineLoopPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexCount;
    GLushort * indexBuf;
    GLuint i;
    GLuint index = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    halPrimitive   = gcvPRIMITIVE_LINE_STRIP;
    /* Line loop */
    primCount = gc->vertexStreams.endVertex - gc->vertexStreams.startVertex;

    indexCount = primCount + 1;

    indexBuf = (*gc->imports.malloc)(gc, indexCount * 2);

    if (indexBuf) {
        for (i = 0; i < primCount; i++) {
            indexBuf[index++] = i;
        }

        indexBuf[index] = 0;

        /* now use 16-bit index buffer, consider index overflow later */
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexCount,
                gcvINDEX_16, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawPolygonPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexBufSize;
    GLushort * index16Buf;
    GLushort * index32Buf;
    GLvoid * indexBuf;
    GLuint i;
    GLuint index = 0;
    GLuint indexType = gcvINDEX_16;

    gcmHEADER_ARG("gc=0x%x", gc);

    halPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
    primCount = (gc->vertexStreams.endVertex - gc->vertexStreams.startVertex) - 2;

    indexBufSize = primCount * 3;

    if (indexBufSize > 65536) {
        indexBuf = (*gc->imports.malloc)(gc, indexBufSize * 4);
        indexType = gcvINDEX_32;
        index32Buf = indexBuf;
    } else {
        indexBuf = (*gc->imports.malloc)(gc, indexBufSize * 2);
        index16Buf = indexBuf;
    }

    switch (indexType) {
        case gcvINDEX_32:
        if (indexBuf) {
            for (i = 0; i < primCount; i++) {
                index32Buf[index++] = i + 1;
                index32Buf[index++] = i + 2;
                index32Buf[index++] = 0;
            }
        }
        break;
        case gcvINDEX_16:
        if (indexBuf) {
            for (i = 0; i < primCount; i++) {
                index16Buf[index++] = i + 1;
                index16Buf[index++] = i + 2;
                index16Buf[index++] = 0;
            }
        }
        break;
    }
    if (indexBuf) {
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexBufSize,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawQuadListPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexBufSize;
    GLushort * index16Buf;
    GLuint * index32Buf;
    GLvoid * indexBuf;
    GLuint i;
    GLuint index = 0;
    GLuint indexType = gcvINDEX_16;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (gc->state.polygon.frontMode == GL_LINE)
    {
        halPrimitive   = gcvPRIMITIVE_LINE_LIST;
        /* triangle list count */
        primCount = (gc->vertexStreams.endVertex - gc->vertexStreams.startVertex);
        indexBufSize = primCount * 2;
        gco3D_SetAntiAliasLine(chipCtx->hw, GL_TRUE);
        gco3D_SetAALineWidth(chipCtx->hw, 1);
    }
    else
    {
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
        /* triangle list count */
        primCount = (gc->vertexStreams.endVertex - gc->vertexStreams.startVertex) / 2;

        indexBufSize = primCount * 3;
    }

    if (indexBufSize > 65536) {
        indexBuf = index32Buf = (*gc->imports.malloc)(gc, indexBufSize * 4);
        indexType = gcvINDEX_32;
    } else {
        indexBuf = index16Buf = (*gc->imports.malloc)(gc, indexBufSize * 2);
    }

    if (indexBuf) {
        switch (indexType) {
            case gcvINDEX_16:
            if (index16Buf) {
                /* generate index */
                if (gc->state.polygon.frontMode == GL_LINE)
                {
                    for (i = 0; i < primCount / 4; i++) {
                        index16Buf[index++] = i * 4;
                        index16Buf[index++] = i * 4 + 1;
                        index16Buf[index++] = i * 4 + 1;
                        index16Buf[index++] = i * 4 + 2;
                        index16Buf[index++] = i * 4 + 2;
                        index16Buf[index++] = i * 4 + 3;
                        index16Buf[index++] = i * 4 + 3;
                        index16Buf[index++] = i * 4;
                    }
                }
                else
                {
                    for (i = 0; i < primCount / 2; i++) {
                        index16Buf[index++] = i * 4;
                        index16Buf[index++] = i * 4 + 1;
                        index16Buf[index++] = i * 4 + 3;
                        index16Buf[index++] = i * 4 + 1;
                        index16Buf[index++] = i * 4 + 2;
                        index16Buf[index++] = i * 4 + 3;
                    }
                }
            }
            break;
            case gcvINDEX_32:
            if (index32Buf) {
                /* generate index */
                if (gc->state.polygon.frontMode == GL_LINE)
                {
                    for (i = 0; i < primCount / 4; i++) {
                        index32Buf[index++] = i * 4;
                        index32Buf[index++] = i * 4 + 1;
                        index32Buf[index++] = i * 4 + 1;
                        index32Buf[index++] = i * 4 + 2;
                        index32Buf[index++] = i * 4 + 2;
                        index32Buf[index++] = i * 4 + 3;
                        index32Buf[index++] = i * 4 + 3;
                        index32Buf[index++] = i * 4;
                    }
                }
                else
                {
                    for (i = 0; i < primCount / 2; i++) {
                        index32Buf[index++] = i * 4;
                        index32Buf[index++] = i * 4 + 1;
                        index32Buf[index++] = i * 4 + 3;
                        index32Buf[index++] = i * 4 + 1;
                        index32Buf[index++] = i * 4 + 2;
                        index32Buf[index++] = i * 4 + 3;
                    }
                }
            }
            break;
        }
        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexBufSize,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
        if (gc->state.polygon.frontMode == GL_LINE)
        {
            gco3D_SetAntiAliasLine(chipCtx->hw, GL_FALSE);
        }
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawQuadStripPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcePRIMITIVE halPrimitive;
    GLuint primCount = 0;
    GLuint indexBufSize;
    GLushort * index16Buf = NULL;
    GLuint * index32Buf = NULL;
    GLvoid * indexBuf = NULL;
    GLuint i;
    GLuint index = 0;
    GLuint indexType = gcvINDEX_16;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Primitive Quad list */
    primCount = (gc->vertexStreams.endVertex - gc->vertexStreams.startVertex) / 2 - 1;

    if (gc->state.polygon.frontMode == GL_LINE)
    {
        halPrimitive   = gcvPRIMITIVE_LINE_LIST;
        /* triangle list count */
        primCount *= 4;
        indexBufSize = primCount * 2;
        gco3D_SetAntiAliasLine(chipCtx->hw, GL_TRUE);
        gco3D_SetAALineWidth(chipCtx->hw, 1);
    }
    else
    {
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
        /* Primitive trangle list */
        primCount *= 2;

        indexBufSize = primCount * 3;
    }

    if (indexBufSize > 65536) {
        indexBuf = (*gc->imports.malloc)(gc, indexBufSize * 4);
        index32Buf = indexBuf;
        indexType = gcvINDEX_16;
    } else {
        indexBuf = (*gc->imports.malloc)(gc, indexBufSize * 2);
        index16Buf = indexBuf;
    }

    if (indexBuf) {
        /* generate index */
        switch (indexType) {
            case gcvINDEX_32:
            if (gc->state.polygon.frontMode == GL_LINE)
            {
                for (i = 0; i < primCount / 4; i++) {
                    index32Buf[index++] = i * 2;
                    index32Buf[index++] = i * 2 + 1;
                    index32Buf[index++] = i * 2 + 1;
                    index32Buf[index++] = i * 2 + 3;
                    index32Buf[index++] = i * 2 + 3;
                    index32Buf[index++] = i * 2 + 2;
                    index32Buf[index++] = i * 2 + 2;
                    index32Buf[index++] = i * 2;
                }
            }
            else
            {
                for (i = 0; i < primCount / 2; i++) {
                    index32Buf[index++] = i * 2;
                    index32Buf[index++] = i * 2 + 1;
                    index32Buf[index++] = i * 2 + 2;
                    index32Buf[index++] = i * 2 + 1;
                    index32Buf[index++] = i * 2 + 3;
                    index32Buf[index++] = i * 2 + 2;
                }
            }
            break;
            case gcvINDEX_16:
            if (gc->state.polygon.frontMode == GL_LINE)
            {
                for (i = 0; i < primCount / 4; i++) {
                    index16Buf[index++] = i * 2;
                    index16Buf[index++] = i * 2 + 1;
                    index16Buf[index++] = i * 2 + 1;
                    index16Buf[index++] = i * 2 + 3;
                    index16Buf[index++] = i * 2 + 3;
                    index16Buf[index++] = i * 2 + 2;
                    index16Buf[index++] = i * 2 + 2;
                    index16Buf[index++] = i * 2;
                }
            }
            else
            {
                for (i = 0; i < primCount / 2; i++) {
                    index16Buf[index++] = i * 2;
                    index16Buf[index++] = i * 2 + 1;
                    index16Buf[index++] = i * 2 + 2;
                    index16Buf[index++] = i * 2 + 1;
                    index16Buf[index++] = i * 2 + 3;
                    index16Buf[index++] = i * 2 + 2;
                }
            }
            break;
        }

        do {
            gcmERR_BREAK(vertexArrayBind(chipCtx, 0, indexBufSize,
                indexType, gcvNULL, indexBuf, &halPrimitive, &primCount));

            /* Draw the primitives. */
            gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
        } while (GL_FALSE);
        (*gc->imports.free)(gc, indexBuf);
        if (gc->state.polygon.frontMode == GL_LINE)
        {
            gco3D_SetAntiAliasLine(chipCtx->hw, GL_FALSE);
        }
    } else {
        __glSetError(GL_OUT_OF_MEMORY);
    }

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gcePRIMITIVE halPrimitive;
    gceSTATUS status;
    GLuint vertexCount;
    GLuint primCount = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    vertexCount = gc->vertexStreams.endVertex - gc->vertexStreams.startVertex;
    switch (gc->vertexStreams.primMode)
    {
    case GL_TRIANGLE_STRIP:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_STRIP;
        primCount = vertexCount - 2;
        break;

    case GL_TRIANGLE_FAN:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_FAN;
        primCount = vertexCount - 2;
        break;

    case GL_TRIANGLES:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
        primCount = vertexCount / 3;
        break;

    case GL_POINTS:
        halPrimitive   = gcvPRIMITIVE_POINT_LIST;
        primCount = vertexCount;
        break;

    case GL_LINES:
        halPrimitive   = gcvPRIMITIVE_LINE_LIST;
        primCount = vertexCount / 2;
        break;

    case GL_LINE_LOOP:
        halPrimitive   = gcvPRIMITIVE_LINE_LOOP;
        primCount = vertexCount;
        break;

    case GL_LINE_STRIP:
        halPrimitive   = gcvPRIMITIVE_LINE_STRIP;
        primCount = vertexCount - 1;
        break;

    case GL_POLYGON:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_FAN;
        primCount = vertexCount - 2;
        break;

    case GL_LINES_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_LINE_STRIP_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_TRIANGLES_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_TRIANGLE_STRIP_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    default:
        gcmASSERT(0);
    }

    do {
        gcmERR_BREAK(vertexArrayBind(chipCtx, gc->vertexStreams.startVertex,
            vertexCount,
            0, gcvNULL, gcvNULL, &halPrimitive, &primCount));

        gcmERR_BREAK(gco3D_DrawPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     gc->vertexStreams.startVertex,
                     primCount));
    } while (GL_FALSE);

    gcmFOOTER_NO();
    return;
}

GLvoid __glChipDrawIndexedPrimitive(__GLcontext* gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gcePRIMITIVE halPrimitive;
    gceSTATUS status;
    gceINDEX_TYPE indexType = gcvINDEX_8;
    GLuint indexCount = gc->vertexStreams.indexCount;
    GLuint primCount = 0;
    glsVERTEXBUFFERINFO* bufInfo;

    gcmHEADER_ARG("gc=0x%x", gc);

    switch (gc->vertexStreams.primMode)
    {
    case GL_TRIANGLE_STRIP:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_STRIP;
        primCount = indexCount - 2;
        break;

    case GL_TRIANGLE_FAN:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_FAN;
        primCount = indexCount - 2;
        break;

    case GL_TRIANGLES:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_LIST;
        primCount = indexCount / 3;
        break;

    case GL_POINTS:
        halPrimitive   = gcvPRIMITIVE_POINT_LIST;
        primCount = indexCount;
        break;

    case GL_LINES:
        halPrimitive   = gcvPRIMITIVE_LINE_LIST;
        primCount = indexCount / 2;
        break;

        /* We need to patch line loop with multiple draws*/
    case GL_LINE_LOOP:
        halPrimitive   = gcvPRIMITIVE_LINE_LOOP;
        primCount = indexCount;
        break;

    case GL_LINE_STRIP:
        halPrimitive   = gcvPRIMITIVE_LINE_STRIP;
        primCount = indexCount - 1;
        break;

    case GL_POLYGON:
        halPrimitive   = gcvPRIMITIVE_TRIANGLE_FAN;
        primCount = indexCount - 2;
        break;

    /* Implement below primitives later */
    case GL_LINES_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_LINE_STRIP_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_TRIANGLES_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    case GL_TRIANGLE_STRIP_ADJACENCY_EXT:
        gcmASSERT(0);
        break;

    default:
        gcmASSERT(0);
    }

    switch (gc->vertexStreams.indexStream.type) {
        case GL_UNSIGNED_BYTE:
            indexType = gcvINDEX_8;
            break;
        case GL_UNSIGNED_SHORT:
            indexType = gcvINDEX_16;
            break;
        case GL_UNSIGNED_INT:
            indexType = gcvINDEX_32;
            break;

    }

    do {
        if (!gc->vertexStreams.indexStream.ppIndexBufPriv ||
            (gc->vertexStreams.indexStream.ppIndexBufPriv &&
            (*gc->vertexStreams.indexStream.ppIndexBufPriv == gcvNULL)))
        {
            gcmERR_BREAK(vertexArrayBind(chipCtx, gc->vertexStreams.startVertex, gc->vertexStreams.indexCount,
                indexType, gcvNULL, gc->vertexStreams.indexStream.streamAddr, &halPrimitive, &primCount));
        } else {
            /* Index buffer is from buffer object */
            bufInfo = *gc->vertexStreams.indexStream.ppIndexBufPriv;
            gcmERR_BREAK(vertexArrayBind(chipCtx, gc->vertexStreams.startVertex, gc->vertexStreams.indexCount,
                indexType,
                bufInfo->bufObject,
                (GLvoid *)gc->vertexStreams.indexStream.offset,
                &halPrimitive,
                &primCount));
        }

        /* Draw the primitives. */
        gcmERR_BREAK(gco3D_DrawIndexedPrimitives(
                     chipCtx->hw,
                     halPrimitive,
                     0,
                     0,
                     primCount,
                     gcvFALSE, 0, 0
                    ));
    } while (GL_FALSE);

    gcmFOOTER_NO();
    return;
}

extern gceSTATUS glshLoadShader( IN __GLcontext * gc );
extern gceSTATUS loadFixFunctionShader( IN __GLcontext * gc );
extern gceSTATUS bindTextureAndTextureState( __GLcontext* gc );

GLvoid sortAttributeByBuiltinAttr(glsCHIPCONTEXT_PTR chipCtx)
{

    glsATTRIBUTEINFO_PTR vertexStream;
    gcsVERTEXARRAY_PTR vertexPtr;
    glsATTRIBUTEINFO tempVStream;
    gcsVERTEXARRAY tempVertex;
    GLint i = 0;
    GLint index = 0;

    if ( (index = chipCtx->builtinAttributeIndex[_GL_VERTEX_INDEX]) >= 0)
    {

            vertexStream = &chipCtx->attributeInfo[__GL_INPUT_VERTEX_INDEX];
            vertexPtr = &chipCtx->attributeArray[__GL_INPUT_VERTEX_INDEX];

            memcpy((char *)&tempVStream, (char *)vertexStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&tempVertex, (char *)vertexPtr, sizeof(gcsVERTEXARRAY));

            memcpy((char *)vertexStream, (char *)&chipCtx->attributeInfo[index], sizeof(glsATTRIBUTEINFO));
            memcpy((char *)vertexPtr, (char *)&chipCtx->attributeArray[index], sizeof(gcsVERTEXARRAY));

            memcpy((char *)&chipCtx->attributeInfo[index], (char *)&tempVStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&chipCtx->attributeArray[index], (char *)&tempVertex, sizeof(gcsVERTEXARRAY));

    }

    if ( (index = chipCtx->builtinAttributeIndex[_GL_COLOR_INDEX]) >= 0)
    {

            vertexStream = &chipCtx->attributeInfo[__GL_INPUT_DIFFUSE_INDEX];
            vertexPtr = &chipCtx->attributeArray[__GL_INPUT_DIFFUSE_INDEX];

            memcpy((char *)&tempVStream, (char *)vertexStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&tempVertex, (char *)vertexPtr, sizeof(gcsVERTEXARRAY));

            memcpy((char *)vertexStream, (char *)&chipCtx->attributeInfo[index], sizeof(glsATTRIBUTEINFO));
            memcpy((char *)vertexPtr, (char *)&chipCtx->attributeArray[index], sizeof(gcsVERTEXARRAY));

            memcpy((char *)&chipCtx->attributeInfo[index], (char *)&tempVStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&chipCtx->attributeArray[index], (char *)&tempVertex, sizeof(gcsVERTEXARRAY));

    }

    for(i = _GL_MULTITEX0_INDEX; i< _GL_BT_INDEX_MAX; i++)
    {

        if ( (index = chipCtx->builtinAttributeIndex[i]) >= 0)
        {

            vertexStream = &chipCtx->attributeInfo[__GL_INPUT_TEX0_INDEX + i - _GL_MULTITEX0_INDEX];
            vertexPtr = &chipCtx->attributeArray[__GL_INPUT_TEX0_INDEX + i - _GL_MULTITEX0_INDEX];

            memcpy((char *)&tempVStream, (char *)vertexStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&tempVertex, (char *)vertexPtr, sizeof(gcsVERTEXARRAY));

            memcpy((char *)vertexStream, (char *)&chipCtx->attributeInfo[index], sizeof(glsATTRIBUTEINFO));
            memcpy((char *)vertexPtr, (char *)&chipCtx->attributeArray[index], sizeof(gcsVERTEXARRAY));

            memcpy((char *)&chipCtx->attributeInfo[index], (char *)&tempVStream, sizeof(glsATTRIBUTEINFO));
            memcpy((char *)&chipCtx->attributeArray[index], (char *)&tempVertex, sizeof(gcsVERTEXARRAY));

        }

    }

}

GLvoid __glChipBegin(__GLcontext* gc, GLenum mode)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    if (!chipCtx->drawRT[0]) {
        return;
    }

    configStream(gc, chipCtx);

    /* update texture some hash keys which are related to stream, so we have to call here */
    /* after streams are configured. */
    updateTextureStates(gc);

    /* pick up right draw function */
    updateDrawPath(gc, chipCtx);

    /* upload texture to device local memory */
    uploadTexture(gc);

    if (chipCtx->currGLSLProgram) {
        sortAttributeByBuiltinAttr(chipCtx);
        glshLoadShader(gc);
    } else {
        if (!chipCtx->useFragmentProcessor) {
            /* determine load fix function or programmable shader here */
            loadFixFunctionShader(gc);
        } else {
            /* Add code for none programmable fragment chip */
        }
    }

    bindTextureAndTextureState(gc);
}

GLvoid __glChipFlush(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    if (chipCtx->drawRT[0]) {
        /* Flush the cache. */
        if (gcmIS_ERROR(gcoSURF_Flush(chipCtx->drawRT[0])))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(chipCtx->hal, gcvFALSE)))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        if(gc->flags & __GL_DRAW_TO_FRONT)
        {
            (*gc->imports.internalSwapBuffers)(gc,GL_TRUE);
        }
    }
}

GLvoid __glChipEnd(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    if ((gc->flags & __GL_DRAW_TO_FRONT) && chipCtx->drawRT[0]) {
        /* Flush the cache. */
        if (gcmIS_ERROR(gcoSURF_Flush(chipCtx->drawRT[0])))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(chipCtx->hal, gcvFALSE)))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        (*gc->imports.internalSwapBuffers)(gc,GL_TRUE);
    }
}

GLvoid __glChipFinish(__GLcontext * gc)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    if (chipCtx->drawRT[0]) {
        /* Flush the cache. */
        if (gcmIS_ERROR(gcoSURF_Flush(chipCtx->drawRT[0])))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE)))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        if(gc->flags & __GL_DRAW_TO_FRONT)
        {
            (*gc->imports.internalSwapBuffers)(gc,GL_TRUE);
        }
    }
}


