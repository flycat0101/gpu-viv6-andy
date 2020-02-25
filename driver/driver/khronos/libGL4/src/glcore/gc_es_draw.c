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
#include "gc_es_device.h"
#include "gc_es_object_inline.c"
#include "gc_es_utils.h"
#ifdef OPENGL40
#include "viv_lock.h"
#endif

extern GLboolean __glIsTextureComplete(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter,
                                       GLenum magFilter, GLenum compareMode, GLint maxLevelUsed);

#ifdef OPENGL40
GLvoid __glConfigArrayVertexStream(__GLcontext *gc, GLenum mode)
{
    GLuint indexBuffer = gc->bufferObject.generalBindingPoint[__GL_ELEMENT_ARRAY_BUFFER_INDEX].boundBufName;
    __GLvertexArrayState *vertexArrayState = &gc->vertexArray.boundVAO->vertexArray;
    GLuint64 arrayEnabled = vertexArrayState->attribEnabled & (~(__GL_VARRAY_EDGEFLAG));
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    __GLvertexAttrib *array;
    GLubyte arrayIdx, streamIdx, numStreams;
    GLuint64 mask;
    GLuint  i;
    GLboolean newStream;
    __GLbufferObject *bufObj;

    gc->vertexStreams.primElemSequence = 0;
    gc->vertexStreams.primElementMask = arrayEnabled;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~arrayEnabled) & (~__GL_VARRAY_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream = NULL;

    arrayEnabled = vertexArrayState->attribEnabled;
    if (arrayEnabled & (__GL_VARRAY_EDGEFLAG))
    {
        array = &vertexArrayState->attribute[__GL_VARRAY_EDGEFLAG_INDEX];;
        if ( vertexArrayState->attributeBinding[array->attribBinding].boundArrayName == 0 )
        {
            gc->vertexStreams.edgeflagStream = (GLubyte *)array->pointer;
        }
        else
        {
            GL_ASSERT(0);
        }
    }

    arrayEnabled &= ~(__GL_VARRAY_EDGEFLAG);
    gc->vertexStreams.indexCount = gc->vertexArray.indexCount;
    if (gc->vertexStreams.indexCount)
    {
        /*configure the index stream*/
        gc->vertexStreams.indexStream.type = gc->vertexArray.indexType;
        if (indexBuffer == 0 )
        {
            gc->vertexStreams.indexStream.ppIndexBufPriv = NULL;
            gc->vertexStreams.indexStream.streamAddr = (GLvoid *)gc->vertexArray.indices;
            gc->vertexStreams.indexStream.offset = 0;
        }
        else
        {
            gc->vertexStreams.indexStream.ppIndexBufPriv = &gc->bufferObject.generalBindingPoint[__GL_ELEMENT_ARRAY_BUFFER_INDEX].boundBufObj->privateData;
            gc->vertexStreams.indexStream.streamAddr = NULL;
            gc->vertexStreams.indexStream.offset = __GL_PTR2SIZE(gc->vertexArray.indices);
        }
    }

    gc->vertexStreams.startVertex = gc->vertexArray.start;
    gc->vertexStreams.endVertex = gc->vertexArray.end;

    /*
    ** Now gc->vertexStreams.streamMode is the stream mode of last draw,
    ** and will be set to VERTEXARRAY_STREAMMODE at the end of this function.
    */
    if (gc->vertexArray.fastStreamSetup && gc->vertexStreams.streamMode == VERTEXARRAY_STREAMMODE)
    {
#if defined (_DEBUG) || defined (DEBUG)
        GLuint elementIdx;
        GLuint arrayIdx_2;
#endif
        for (streamIdx = 0; streamIdx < gc->vertexStreams.numStreams; streamIdx++ )
        {
            stream = &gc->vertexStreams.streams[streamIdx];
            arrayIdx = stream->streamElement[0].inputIndex;
            bufObj =  vertexArrayState->attributeBinding[arrayIdx].boundArrayObj;
            if (bufObj && bufObj->size > 0)
            {
                GL_ASSERT(stream->privPtrAddr);
                GL_ASSERT(vertexArrayState->attributeBinding[vertexArrayState->attribute[arrayIdx].attribBinding].boundArrayName);
                stream->privPtrAddr = &bufObj->privateData;
                stream->streamAddr = NULL;
            }
            else
            {
                GL_ASSERT(stream->privPtrAddr == NULL);
                GL_ASSERT(vertexArrayState->attributeBinding[vertexArrayState->attribute[arrayIdx].attribBinding].boundArrayName == 0 ||
                    (bufObj && bufObj->size == 0) );
                stream->streamAddr = (GLvoid *)gc->clientState.vertexArray.attribute[arrayIdx].pointer;
#if defined (_DEBUG) || defined (DEBUG)
                for ( elementIdx = 0; elementIdx < stream->numElements; elementIdx++ )
                {
                    arrayIdx_2 = stream->streamElement[elementIdx].inputIndex;
                }
#endif
            }
        }

        return;
    }

    gc->vertexArray.fastStreamSetup = GL_FALSE;

    /* If both __GL_VARRAY_ATT0 and __GL_VARRAY_VERTEX enabled, according
    ** to spec, we prefer to send down the data of __GL_VARRAY_ATT0
    */
    if (arrayEnabled & __GL_VARRAY_ATT0)
    {
        mask = arrayEnabled & ~__GL_VARRAY_VERTEX;
    }else
    {
        mask = arrayEnabled;
    }

    arrayIdx = 0;
    numStreams = 0;

    while(mask)
    {
        if (mask & 0x1)
        {
            array = &vertexArrayState->attribute[arrayIdx];
            bufObj = vertexArrayState->attributeBinding[arrayIdx].boundArrayObj;

            /* Corner case: FarCry call SecondaryColorPointer with size 4. currentArrays will not updated.*/
            if (array->stride == 0)
            {
                gc->vertexStreams.missingAttribs |= (__GL_ONE_64 << arrayIdx);
                arrayIdx++;
                mask >>= 1;
                continue;
            }

            if (array->stride < array->size * __glSizeOfType(array->type))
            {
//                gc->vertexArray.immedFallback = GL_TRUE;
                return;
            }

            /*the first stage of checking whether we should configure another stream for this element*/
            newStream = GL_TRUE;/*default: YES, another stream*/
            for(streamIdx = 0; streamIdx < numStreams; streamIdx++)
            {
                stream = &gc->vertexStreams.streams[streamIdx];
                if ( bufObj == NULL || bufObj->size == 0 )/*normal vertex array*/
                {
                    if (stream->privPtrAddr == NULL)/*stream source from conventional vertex array*/
                    {
                        newStream = GL_FALSE;
                        break;
                    }
                    else/*stream source from buffer object*/
                    {
                        continue;
                    }
                }
                else/*from buffer object*/
                {
                    if (stream->privPtrAddr == NULL)/*stream source from conventional vertex array*/
                    {
                        continue;
                    }
                    else/*stream source from buffer object*/
                    {
                        GL_ASSERT(bufObj);
                        if (&bufObj->privateData == stream->privPtrAddr)
                        {
                            newStream = GL_FALSE;
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }

            /*the second stage of checking whether we should configure another stream for this element*/
            if (!newStream)
            {
                __GLvertexElement *prevElement, tempElement;
                __GLvertexAttrib *prevArray;
                stream = &gc->vertexStreams.streams[streamIdx];

                prevElement = &stream->streamElement[stream->numElements-1];
                prevArray = &vertexArrayState->attribute[prevElement->inputIndex];

                GL_ASSERT(prevArray->stride == (GLsizei)stream->stride);
                GL_ASSERT(streamIdx == prevElement->streamIndex);

                /*array->offset may be an offset or a pointer. No matter what it is,
                (array->offset - prevArray->offset) is always what we want here*/
                if ( (array->stride == (GLsizei)stream->stride) && ( (GLuint)abs((GLint)glALL_TO_UINT32(array->offset - prevArray->offset)) < stream->stride ))
                {
                    /*configure the next element in the stream*/
                    element = &stream->streamElement[stream->numElements];
                    element->streamIndex = streamIdx;
                    element->inputIndex = arrayIdx;
                    element->size = array->size;
                    element->type = array->type;
                    element->normalized = array->normalized;
                    element->integer = array->integer;
                    element->offset = array->offset;
                    /*for conventional vertex arrays, this offset could be seen as an offset from NULL*/

                    /*insert the element to the stream according to offset*/
                    while( (element > stream->streamElement) && (element->offset < prevElement->offset) )
                    {
                        GL_ASSERT( element == prevElement + 1 );
                        tempElement = *prevElement;
                        *prevElement = *element;
                        *element = tempElement;
                        element--;
                        prevElement--;
                    }

                    /*streamAddr may have changed*/
                    stream->streamAddr = __GL_OFFSET_TO_POINTER(stream->streamElement[0].offset);

                    /*incease the numElements of the stream*/
                    stream->numElements++;
                }
                else
                {
                    /*we need another stream for this element*/
                    newStream = GL_TRUE;
                }
            }

            if (newStream)
            {
                /*get a new stream*/
                stream = &gc->vertexStreams.streams[numStreams];

                /*configure the first element in the new stream*/
                element = &stream->streamElement[0];
                element->streamIndex = numStreams;
                element->inputIndex = arrayIdx;
                element->size = array->size;
                element->type = array->type;
                element->normalized = array->normalized;
                element->integer = array->integer;
                element->offset = array->offset;/*for normal vertex arrays, this offset could be seen as an offset from
                                                                NULL pointer*/

                /*configure the new stream*/
                stream->privPtrAddr = (bufObj && bufObj->size > 0) ? &bufObj->privateData: 0;
                stream->numElements = 1;
                stream->stride = array->stride;
                stream->streamAddr = __GL_OFFSET_TO_POINTER(element->offset);

                /* increase the numStream */
                numStreams++;
            }
        }

        arrayIdx++;
        mask >>= 1;
    }

    /*
    **For streams that do not source data from buffer objects, calculate the offsets
    **for all elements. For Streams that source data from buffer objects, calculate
    ** the streamAddr.
    */
    for(streamIdx = 0; streamIdx < numStreams; streamIdx++ )
    {
        stream = &gc->vertexStreams.streams[streamIdx];
        /*for streams in buffer object, offset is already an offset*/
        if (stream->privPtrAddr == NULL)
        {
            GL_ASSERT(stream->streamAddr == __GL_OFFSET_TO_POINTER(stream->streamElement[0].offset));
            for(i = 1; i < stream->numElements; i ++ )
            {
                stream->streamElement[i].offset -= stream->streamElement[0].offset;
            }
            stream->streamElement[0].offset = 0;
        }
        else
        {
            /*Map the buffer object, and set the streamAddr*/
            arrayIdx = stream->streamElement[0].inputIndex;
            bufObj = vertexArrayState->attributeBinding[arrayIdx].boundArrayObj;
            GL_ASSERT(&bufObj->privateData == stream->privPtrAddr);
            stream->streamAddr = NULL;
        }
    }

    gc->vertexStreams.numStreams = numStreams;
    gc->vertexStreams.streamMode = VERTEXARRAY_STREAMMODE;
}
#endif

#define _GC_OBJ_ZONE gcdZONE_GL40_CORE

/* Set all attribute dirty bits.
 */
GLvoid __glSetAttributeStatesDirty(__GLcontext *gc)
{
    GLuint index, unit;
    __GLSLStage stage;

    /* Initialize global dirty attribute bits */
    for (index = 0; index < __GL_DIRTY_ATTRS_END; index++)
    {
        gc->globalDirtyState[index] = (GLbitfield)(-1);
    }

    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_TRUE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_TRUE);

    for (unit = 0; unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits; unit++)
    {
        gc->texUnitAttrState[unit] = (GLuint64)(-1);
    }

    gc->drawableDirtyMask = __GL_BUFFER_DRAW_READ_BITS;

    /* Reset the last program/codeSeq to force toggling the dirty. */
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
        gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
    }

    for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
    {
        __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_TRUE);
    }
}

/* Clear all attribute dirty bits
 */
__GL_INLINE GLvoid __glClearAttributeStates(__GLcontext *gc)
{
    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_FALSE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_FALSE);

    __GL_MEMZERO(gc->texUnitAttrState, gc->constants.shaderCaps.maxCombinedTextureImageUnits * sizeof(GLuint64));
    __GL_MEMZERO(gc->globalDirtyState, __GL_DIRTY_ATTRS_END   * sizeof(GLbitfield));
}

GLvoid __glBuildTexEnableDim(__GLcontext * gc, __GLattribute* cs, __GLattribute* ds)
{
    GLint unit;

    (*gc->dp.buildTexEnableDim)(gc);

    /* Mark texture enable dimension changed */
    for (unit = 0; unit < (GLint)gc->shaderProgram.maxUnit; ++unit)
    {
        if (ds->texture.texUnits[unit].enableDim != cs->texture.texUnits[unit].enableDim)
        {
            ds->texture.texUnits[unit].enableDim = cs->texture.texUnits[unit].enableDim;
            __GL_SET_TEX_UNIT_BIT(gc, unit, __GL_TEX_ENABLE_DIM_CHANGED_BIT);
        }
    }
}

/*
** Compute the gc->texture.units[unit].maxLevelUsed, which will be used in texture consistency check
*/
GLint __glCalcTexMaxLevelUsed(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter)
{
    GLboolean useMipmap = GL_FALSE;
    GLint maxLevelUsed;
    GLint base = texObj->params.baseLevel;

    /* Rectangular texture target has only one mipmap level, can't do mipmap */
    if (texObj->targetIndex == __GL_TEXTURE_RECTANGLE_INDEX)
    {
        return base;
    }

    /* Check whether need mipmap */
    switch (texObj->params.mipHint)
    {
    case __GL_TEX_MIP_HINT_AUTO:
    case __GL_TEX_MIP_HINT_AUTO_MIP:
        useMipmap = (minFilter != GL_NEAREST && minFilter != GL_LINEAR) ? GL_TRUE : GL_FALSE;
        break;
    case __GL_TEX_MIP_HINT_FORCE_ON:
        useMipmap = GL_TRUE;
        break;
    case __GL_TEX_MIP_HINT_FORCE_OFF:
        useMipmap = GL_FALSE;
        break;
    default:
        GL_ASSERT(0);
    }

    if (useMipmap)
    {
        if (texObj->immutable)
        {
            maxLevelUsed = texObj->immutableLevels - 1;
        }
        else
        {
            __GLmipMapLevel *baseMipmap = &texObj->faceMipmap[0][base];
            GLint maxSize = __GL_MAX(__GL_MAX(baseMipmap->width, baseMipmap->height), baseMipmap->depth);
            maxLevelUsed = (GLint)__glFloorLog2((GLuint)maxSize) + base;
        }

        if (maxLevelUsed > texObj->params.maxLevel)
        {
            maxLevelUsed = texObj->params.maxLevel;
        }

        if (((texObj->params.mipHint == __GL_TEX_MIP_HINT_FORCE_ON) ||
             (texObj->params.mipHint == __GL_TEX_MIP_HINT_AUTO_MIP)
            )&&
            (maxLevelUsed > texObj->mipMaxLevel)
           )
        {
            maxLevelUsed = texObj->mipMaxLevel;
        }
    }
    else
    {
        maxLevelUsed = base;
    }

    return maxLevelUsed;
}

__GL_INLINE GLvoid __glEvaluateAttribGroup1(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

    __GL_CHECK_ATTR2(__GL_DEPTHRANGE_BIT, depth.zNear, depth.zFar);

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        __GL_CHECK_ATTR4(__GL_CLEARACCUM_BIT,
            accum.clear.r, accum.clear.g, accum.clear.b, accum.clear.a);
    }
#endif

    if (localMask & (__GL_COLORBUF_ATTR_BITS))
    {
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            __GL_CHECK_ATTR2(__GL_ALPHAFUNC_BIT,
                raster.alphaFunction, raster.alphaReference);

            __GL_CHECK_ATTR1(__GL_ALPHATEST_ENDISABLE_BIT,
                enables.colorBuffer.alphaTest);
        }
#endif

        __GL_CHECK_ATTR4(__GL_BLENDCOLOR_BIT,
            raster.blendColor.r, raster.blendColor.g,
            raster.blendColor.b, raster.blendColor.a);

        if (localMask & __GL_BLENDEQUATION_BIT)
        {
            if ((__GL_MEMCMP(ds->raster.blendEquationRGB,   cs->raster.blendEquationRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)) ||
                (__GL_MEMCMP(ds->raster.blendEquationAlpha, cs->raster.blendEquationAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)))
            {
                __GL_MEMCOPY(ds->raster.blendEquationRGB,   cs->raster.blendEquationRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendEquationAlpha, cs->raster.blendEquationAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLENDEQUATION_BIT);
            }
        }

        if (localMask & __GL_BLENDFUNC_BIT)
        {
            if ((__GL_MEMCMP(ds->raster.blendSrcRGB,   cs->raster.blendSrcRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers))     ||
                (__GL_MEMCMP(ds->raster.blendSrcAlpha, cs->raster.blendSrcAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)) ||
                (__GL_MEMCMP(ds->raster.blendDstRGB,   cs->raster.blendDstRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers))     ||
                (__GL_MEMCMP(ds->raster.blendDstAlpha, cs->raster.blendDstAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers)))
            {
                __GL_MEMCOPY(ds->raster.blendSrcRGB,   cs->raster.blendSrcRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendSrcAlpha, cs->raster.blendSrcAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendDstRGB,   cs->raster.blendDstRGB,   sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
                __GL_MEMCOPY(ds->raster.blendDstAlpha, cs->raster.blendDstAlpha, sizeof(GLenum) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLENDFUNC_BIT);
            }
        }

        if (localMask & __GL_BLEND_ENDISABLE_BIT)
        {
            if (__GL_MEMCMP(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers))
            {
                __GL_MEMCOPY(ds->enables.colorBuffer.blend, cs->enables.colorBuffer.blend, sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~(__GL_BLEND_ENDISABLE_BIT);
            }
        }

        if (localMask & __GL_COLORMASK_BIT)
        {
            if (__GL_MEMCMP(ds->raster.colorMask, cs->raster.colorMask, 4 * sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers))
            {
                __GL_MEMCOPY(ds->raster.colorMask, cs->raster.colorMask, 4 * sizeof(GLboolean) * gc->constants.shaderCaps.maxDrawBuffers);
            }
            else
            {
                localMask &= ~( __GL_COLORMASK_BIT);
            }
        }
    }

    if (localMask & (__GL_DEPTHBUF_ATTR_BITS))
    {
        __GL_CHECK_ATTR1(__GL_DEPTHFUNC_BIT, depth.testFunc);
        __GL_CHECK_ATTR1(__GL_DEPTHTEST_ENDISABLE_BIT, enables.depthTest);
        __GL_CHECK_ATTR1(__GL_DEPTHMASK_BIT, depth.writeEnable);
    }

    if (localMask & __GL_STENCIL_ATTR_BITS)
    {
        __GL_CHECK_ATTR3(__GL_STENCILFUNC_FRONT_BIT,
            stencil.front.testFunc, stencil.front.reference, stencil.front.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_FRONT_BIT,
            stencil.front.fail, stencil.front.depthFail, stencil.front.depthPass);

        __GL_CHECK_ATTR3(__GL_STENCILFUNC_BACK_BIT,
            stencil.back.testFunc, stencil.back.reference, stencil.back.mask);

        __GL_CHECK_ATTR3(__GL_STENCILOP_BACK_BIT,
            stencil.back.fail, stencil.back.depthFail, stencil.back.depthPass);

        __GL_CHECK_ATTR1(__GL_STENCILMASK_FRONT_BIT, stencil.front.writeMask);
        __GL_CHECK_ATTR1(__GL_STENCILMASK_BACK_BIT, stencil.back.writeMask);
        __GL_CHECK_ATTR1(__GL_STENCILTEST_ENDISABLE_BIT, enables.stencilTest);
    }

    if (localMask & __GL_POLYGON_ATTR_BITS)
    {
        __GL_CHECK_ATTR1(__GL_FRONTFACE_BIT, polygon.frontFace);
        __GL_CHECK_ATTR1(__GL_CULLFACE_BIT, polygon.cullFace);
        __GL_CHECK_ATTR1(__GL_CULLFACE_ENDISABLE_BIT, enables.polygon.cullFace);
        __GL_CHECK_ATTR2(__GL_POLYGONOFFSET_BIT, polygon.factor, polygon.units);
        __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_FILL_ENDISABLE_BIT, enables.polygon.polygonOffsetFill);
#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            __GL_CHECK_ATTR2(__GL_POLYGONMODE_BIT, polygon.frontMode, polygon.backMode);
            __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_POINT_ENDISABLE_BIT,
                enables.polygon.polygonOffsetPoint);

            __GL_CHECK_ATTR1(__GL_POLYGONOFFSET_LINE_ENDISABLE_BIT,
                enables.polygon.polygonOffsetLine);

            __GL_CHECK_ATTR1(__GL_POLYGONSTIPPLE_ENDISABLE_BIT,
                enables.polygon.stipple);
        }
#endif
    }

    __GL_CHECK_ATTR1(__GL_RASTERIZER_DISCARD_ENDISABLE_BIT, enables.rasterizerDiscard);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_1]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_1] = localMask;
    if (localMask == 0)
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_DIRTY_ATTRS_1);
    }
}

__GL_INLINE GLvoid __glEvaluateAttribGroup2(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

    __GL_CHECK_ATTR4(__GL_VIEWPORT_BIT, viewport.x, viewport.y,
                                        viewport.width, viewport.height);

    __GL_CHECK_ATTR4(__GL_SCISSOR_BIT, scissor.scissorX, scissor.scissorY,
                                       scissor.scissorWidth, scissor.scissorHeight);

    __GL_CHECK_ATTR1(__GL_SCISSORTEST_ENDISABLE_BIT, enables.scissorTest);
    __GL_CHECK_ATTR1(__GL_DITHER_ENDISABLE_BIT, enables.colorBuffer.dither);
    __GL_CHECK_ATTR1(__GL_LINEWIDTH_BIT, line.requestedWidth);
    __GL_CHECK_ATTR2(__GL_SAMPLECOVERAGE_BIT, multisample.coverageValue, multisample.coverageInvert);
    __GL_CHECK_ATTR1(__GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT, enables.multisample.alphaToCoverage);
    __GL_CHECK_ATTR1(__GL_SAMPLE_COVERAGE_ENDISABLE_BIT, enables.multisample.coverage);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MASK_ENDISABLE_BIT, enables.multisample.sampleMask);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MASK_BIT, multisample.sampleMaskValue);
    __GL_CHECK_ATTR1(__GL_SAMPLE_SHADING_ENDISABLE_BIT, enables.multisample.sampleShading);
    __GL_CHECK_ATTR1(__GL_SAMPLE_MIN_SHADING_VALUE_BIT, multisample.minSampleShadingValue);

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_2]
    */
#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
         if (localMask & __GL_FOG_ATTR_BITS) {

            __GL_CHECK_ATTR4(__GL_FOGCOLOR_BIT,
                fog.color.r, fog.color.g, fog.color.b, fog.color.a);

            __GL_CHECK_ATTR1(__GL_FOGINDEX_BIT,
                fog.index);

            __GL_CHECK_ATTR1(__GL_FOGDENSITY_BIT,
                fog.density);

            __GL_CHECK_ATTR1(__GL_FOGSTART_BIT,
                fog.start);

            __GL_CHECK_ATTR1(__GL_FOGEND_BIT,
                fog.end);

            __GL_CHECK_ATTR1(__GL_FOGMODE_BIT,
                fog.mode);

            __GL_CHECK_ATTR1(__GL_FOG_ENDISABLE_BIT,
                enables.fog);
        }

        if (localMask & __GL_COLORBUF_ATTR2_BITS) {

            __GL_CHECK_ATTR1(__GL_LOGICOP_BIT,
                raster.logicOp);

            __GL_CHECK_ATTR1(__GL_LOGICOP_ENDISABLE_BIT,
                enables.colorBuffer.colorLogicOp);

      //      __GL_CHECK_ATTR4(__GL_CLEARCOLOR_BIT,
      //          raster.clear.r, raster.clear.g, raster.clear.b, raster.clear.a);

            __GL_CHECK_ATTR1(__GL_DITHER_ENDISABLE_BIT,
                enables.colorBuffer.dither);
        }

        if (localMask & __GL_DEPTHBUF_ATTR2_BITS) {

            __GL_CHECK_ATTR2(__GL_DEPTHBOUNDTEST_BIT,
                depthBoundTest.zMin, depthBoundTest.zMax);

            __GL_CHECK_ATTR1(__GL_DEPTHBOUNDTESTENABLE_BIT,
                enables.depthBoundTest);
        }

        if (localMask & __GL_LINE_ATTR_BITS) {
            __GL_CHECK_ATTR1(__GL_LINESMOOTH_ENDISABLE_BIT,
                enables.line.smooth);

            __GL_CHECK_ATTR2(__GL_LINESTIPPLE_BIT,
                line.stippleRepeat, line.stipple);

            __GL_CHECK_ATTR1(__GL_LINESTIPPLE_ENDISABLE_BIT,
                enables.line.stipple);
        }
    }
#endif
    gc->globalDirtyState[__GL_DIRTY_ATTRS_2] = localMask;
    if (localMask == 0)
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_DIRTY_ATTRS_2);
    }
}

#ifdef OPENGL40
__GL_INLINE GLvoid __glEvaluateLightingAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
   GLbitfield localMask = gc->globalDirtyState[__GL_LIGHTING_ATTRS];

    __GL_CHECK_ATTR1(__GL_SHADEMODEL_BIT,
        light.shadingModel);

    __GL_CHECK_ATTR1(__GL_LIGHTING_ENDISABLE_BIT,
        enables.lighting.lighting);

    if (localMask & __GL_LIGHTMODEL_ATTR_BITS) {

        __GL_CHECK_ATTR4(__GL_LIGHTMODEL_AMBIENT_BIT,
            light.model.ambient.r, light.model.ambient.g,
            light.model.ambient.b, light.model.ambient.a);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_LOCALVIEWER_BIT,
            light.model.localViewer);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_TWOSIDE_BIT,
            light.model.twoSided);

        __GL_CHECK_ATTR1(__GL_LIGHTMODEL_COLORCONTROL_BIT,
            light.model.colorControl);
    }

    if (localMask & __GL_FRONT_MATERIAL_BITS) {
        __GL_CHECK_ATTR3(__GL_MATERIAL_COLORINDEX_FRONT_BIT,
            light.front.cmapa, light.front.cmapd, light.front.cmaps);

        __GL_CHECK_ATTR4(__GL_MATERIAL_EMISSION_FRONT_BIT,
            light.front.emissive.r, light.front.emissive.g,
            light.front.emissive.b, light.front.emissive.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_SPECULAR_FRONT_BIT,
            light.front.specular.r, light.front.specular.g,
            light.front.specular.b, light.front.specular.a);

        __GL_CHECK_ATTR1(__GL_MATERIAL_SHININESS_FRONT_BIT,
            light.front.specularExponent);

        __GL_CHECK_ATTR4(__GL_MATERIAL_AMBIENT_FRONT_BIT,
            light.front.ambient.r, light.front.ambient.g,
            light.front.ambient.b, light.front.ambient.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_DIFFUSE_FRONT_BIT,
            light.front.diffuse.r, light.front.diffuse.g,
            light.front.diffuse.b, light.front.diffuse.a);
    }

    if (localMask & __GL_BACK_MATERIAL_BITS) {
        __GL_CHECK_ATTR3(__GL_MATERIAL_COLORINDEX_BACK_BIT,
            light.back.cmapa, light.back.cmapd, light.back.cmaps);

        __GL_CHECK_ATTR4(__GL_MATERIAL_EMISSION_BACK_BIT,
            light.back.emissive.r, light.back.emissive.g,
            light.back.emissive.b, light.back.emissive.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_SPECULAR_BACK_BIT,
            light.back.specular.r, light.back.specular.g,
            light.back.specular.b, light.back.specular.a);

        __GL_CHECK_ATTR1(__GL_MATERIAL_SHININESS_BACK_BIT,
            light.back.specularExponent);

        __GL_CHECK_ATTR4(__GL_MATERIAL_AMBIENT_BACK_BIT,
            light.back.ambient.r, light.back.ambient.g,
            light.back.ambient.b, light.back.ambient.a);

        __GL_CHECK_ATTR4(__GL_MATERIAL_DIFFUSE_BACK_BIT,
            light.back.diffuse.r, light.back.diffuse.g,
            light.back.diffuse.b, light.back.diffuse.a);
    }

    __GL_CHECK_ATTR2(__GL_COLORMATERIAL_BIT,
        light.colorMaterialFace, light.colorMaterialParam);

    __GL_CHECK_ATTR1(__GL_COLORMATERIAL_ENDISABLE_BIT,
        enables.lighting.colorMaterial);

    /* Reassign localMask back to globalDirtyState[__GL_LIGHTING_ATTRS]
    */
    gc->globalDirtyState[__GL_LIGHTING_ATTRS] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_LIGHTING_ATTRS);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_LIGHTING_ATTRS]
        */
        gc->swpDirtyState[__GL_LIGHTING_ATTRS] |= localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateLightSrcAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)

{

    GLbitfield bitMask = gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];

    GLuint i = 0;



    while (bitMask) {

        if (bitMask & 0x1) {

            GLbitfield localMask = gc->lightAttrState[i];



            __GL_CHECK_ATTR1(__GL_LIGHT_ENDISABLE_BIT,

                enables.lighting.light[i]);



            __GL_CHECK_ATTR4(__GL_LIGHT_AMBIENT_BIT,

                light.source[i].ambient.r, light.source[i].ambient.g,

                light.source[i].ambient.b, light.source[i].ambient.a);



            __GL_CHECK_ATTR4(__GL_LIGHT_DIFFUSE_BIT,

                light.source[i].diffuse.r, light.source[i].diffuse.g,

                light.source[i].diffuse.b, light.source[i].diffuse.a);



            __GL_CHECK_ATTR4(__GL_LIGHT_SPECULAR_BIT,

                light.source[i].specular.r, light.source[i].specular.g,

                light.source[i].specular.b, light.source[i].specular.a);



            __GL_CHECK_ATTR4(__GL_LIGHT_POSITION_BIT,

                light.source[i].positionEye.f.x,

                light.source[i].positionEye.f.y,

                light.source[i].positionEye.f.z,

                light.source[i].positionEye.f.w);



            __GL_CHECK_ATTR1(__GL_LIGHT_CONSTANTATT_BIT,

                light.source[i].constantAttenuation);



            __GL_CHECK_ATTR1(__GL_LIGHT_LINEARATT_BIT,

                light.source[i].linearAttenuation);



            __GL_CHECK_ATTR1(__GL_LIGHT_QUADRATICATT_BIT,

                light.source[i].quadraticAttenuation);



            __GL_CHECK_ATTR4(__GL_LIGHT_SPOTDIRECTION_BIT,

                light.source[i].direction.f.x, light.source[i].direction.f.y,

                light.source[i].direction.f.z, light.source[i].direction.f.w);



            __GL_CHECK_ATTR1(__GL_LIGHT_SPOTEXPONENT_BIT,

                light.source[i].spotLightExponent);



            __GL_CHECK_ATTR1(__GL_LIGHT_SPOTCUTOFF_BIT,

                light.source[i].spotLightCutOffAngle);



            /* Reassign localMask back to gc->lightAttrState[i]

            */

            gc->lightAttrState[i] = localMask;

            if (localMask == 0) {

                gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] &= ~(1 << i);

            }

            else {

                /* Copy the dirty bits to swpLightAttrState[i].

                */

                gc->swpLightAttrState[i] |= localMask;

            }

        }



        bitMask = (bitMask >> 1);

        i++;

    }



    if (gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS] == 0) {

        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_LIGHT_SRC_ATTRS);

    }

    else {

        /* Copy the dirty bits to swpDirtyState[__GL_LIGHT_SRC_ATTRS]

        */

        gc->swpDirtyState[__GL_LIGHT_SRC_ATTRS] |=

            gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS];

    }

}

__GL_INLINE GLvoid __glEvaluateAttribGroup3(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_3];

    __GL_CHECK_ATTR1(__GL_NORMALIZE_ENDISABLE_BIT,
        enables.transform.normalize);

    __GL_CHECK_ATTR1(__GL_RESCALENORMAL_ENDISABLE_BIT,
        enables.transform.rescaleNormal);

    __GL_CHECK_ATTR1(__GL_MULTISAMPLE_ENDISABLE_BIT,
        enables.multisample.multisampleOn);

    __GL_CHECK_ATTR1(__GL_SAMPLE_ALPHA_TO_ONE_ENDISABLE_BIT,
        enables.multisample.alphaToOne);

    __GL_CHECK_ATTR1(__GL_COLORSUM_ENDISABLE_BIT,
        enables.colorSum);

     if (localMask & __GL_POINT_ATTR_BITS) {

        __GL_CHECK_ATTR1(__GL_POINTSIZE_BIT,
            point.requestedSize);

        __GL_CHECK_ATTR1(__GL_POINTSMOOTH_ENDISABLE_BIT,
            enables.pointSmooth);

        __GL_CHECK_ATTR1(__GL_POINT_SIZE_MIN_BIT,
            point.sizeMin);

        __GL_CHECK_ATTR1(__GL_POINT_SIZE_MAX_BIT,
            point.sizeMax);

        __GL_CHECK_ATTR1(__GL_POINT_FADE_THRESHOLD_SIZE_BIT,
            point.fadeThresholdSize);

        __GL_CHECK_ATTR3(__GL_POINT_DISTANCE_ATTENUATION_BIT,
            point.distanceAttenuation[0],
            point.distanceAttenuation[1],
            point.distanceAttenuation[2]);

        __GL_CHECK_ATTR1(__GL_POINTSPRITE_ENDISABLE_BIT,
            enables.pointSprite);

        __GL_CHECK_ATTR1(__GL_POINTSPRITE_COORD_ORIGIN_BIT,
            point.coordOrigin);

    }

    /* Reassign localMask back to globalDirtyState[__GL_DIRTY_ATTRS_3]
    */
    gc->globalDirtyState[__GL_DIRTY_ATTRS_3] = localMask;
    if (localMask == 0) {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(1 << __GL_DIRTY_ATTRS_3);
    }
    else {
        /* Copy the dirty bits to swpDirtyState[__GL_DIRTY_ATTRS_3]
        */
        gc->swpDirtyState[__GL_DIRTY_ATTRS_3] |= localMask;
    }
}
#endif

__GL_INLINE GLuint __glGetTextureDIM(__GLattribute* cs, GLint i)
{
#ifdef OPENGL40
    if (cs->enables.texUnits[i].enabledDimension > 0)
        return cs->enables.texUnits[i].enabledDimension - 1;
#endif
    return cs->texture.texUnits[i].enableDim;
}
__GL_INLINE GLvoid __glEvaluateTextureAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLint i = -1;
    GLuint64 localMask;
    GLuint enableDim, realEnableDim, lastRealEnableDim;
    __GLbitmask unitMask = gc->texUnitAttrDirtyMask;
    GLuint lastMaxLevelUsed = 0;
    __GLtextureObject *texObj = gcvNULL;

    while (!__glBitmaskIsAllZero(&unitMask))
    {
        if (!__glBitmaskTestAndClear(&unitMask, ++i))
        {
            continue;
        }

        localMask = gc->texUnitAttrState[i];
        lastMaxLevelUsed = gc->texture.units[i].maxLevelUsed;

        /* Save the last enabled texture dimension and last texture object. */
        if (gc->imports.conformGLSpec)
        {
            enableDim = __glGetTextureDIM(cs, i);
        }
        else
        {
            enableDim = cs->texture.texUnits[i].enableDim;
        }

        lastRealEnableDim = ds->texture.texUnits[i].realEnableDim;

        /* Please do only use "currentTexture" after the evaluation and in this draw.
        ** Otherwise the texobj might be destroyed in other context but "currentTexture" not cleared.
        */
        gc->texture.units[i].currentTexture = gcvNULL;
        __glBitmaskClear(&gc->texture.currentEnableMask, i);
        if (enableDim < __GL_MAX_TEXTURE_BINDINGS)
        {
            GLenum minFilter, magFilter, compareMode;

            texObj = gc->texture.units[i].boundTextures[enableDim];
            minFilter = gc->texture.units[i].boundSampler
                      ? gc->texture.units[i].boundSampler->params.minFilter
                      : texObj->params.sampler.minFilter;
            magFilter = gc->texture.units[i].boundSampler
                      ? gc->texture.units[i].boundSampler->params.magFilter
                      : texObj->params.sampler.magFilter;
            compareMode = gc->texture.units[i].boundSampler
                        ? gc->texture.units[i].boundSampler->params.compareMode
                        : texObj->params.sampler.compareMode;

            /* maxLevelUsed must be calculated before texture consistent check */
            if (localMask & (__GL_TEX_ENABLE_DIM_CHANGED_BIT |
                             __GL_TEX_IMAGE_CONTENT_CHANGED_BIT |
                             __GL_TEXPARAM_MIP_HINT_BIT |
                             __GL_TEXPARAM_MIN_FILTER_BIT |
                             __GL_TEXPARAM_BASE_LEVEL_BIT |
                             __GL_TEXPARAM_MAX_LEVEL_BIT))
            {

                gc->texture.units[i].maxLevelUsed = __glCalcTexMaxLevelUsed(gc, texObj, minFilter);

                /* Update and set object-only dirty */
                if (texObj->maxLevelUsed != gc->texture.units[i].maxLevelUsed)
                {
                    texObj->maxLevelUsed = gc->texture.units[i].maxLevelUsed;
                    texObj->uObjStateDirty.s.maxLevelUsedDirty = GL_TRUE;
                }
            }

            if (__glIsTextureComplete(gc, texObj, minFilter, magFilter, compareMode, gc->texture.units[i].maxLevelUsed))
            {
                realEnableDim = enableDim;
                gc->texture.units[i].currentTexture = texObj;
                __glBitmaskSet(&gc->texture.currentEnableMask, i);
            }
            else
            {
                /* Set texture to be disabled if inconsistent */
                realEnableDim = __GL_MAX_TEXTURE_BINDINGS;
                __GLES_PRINT("ES30:texture(id=%d) is incomplete", texObj->name);

            }
        }
        else
        {
            realEnableDim = enableDim;
        }

        /* Check if "enableDim" is really changed */
        cs->texture.texUnits[i].realEnableDim = realEnableDim;
        if (realEnableDim != ds->texture.texUnits[i].realEnableDim)
        {
            localMask |= __GL_TEX_ENABLE_DIM_CHANGED_BIT;
        }
        __GL_CHECK_ATTR1(__GL_TEX_ENABLE_DIM_CHANGED_BIT, texture.texUnits[i].realEnableDim);

        if (localMask & __GL_TEX_ENABLE_DIM_CHANGED_BIT)
        {
            /* If dim changed, only need to check param bit if enabled */
            if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS)
            {
                localMask |= __GL_TEXPARAMETER_BITS;
            }
            else
            {
                localMask &= ~__GL_TEXPARAMETER_BITS;
            }

            /* Texture image dirty bits need to be set when enableDim is changed */
            localMask |= __GL_TEXIMAGE_BITS;
        }

        if (localMask & __GL_TEXPARAMETER_BITS)
        {
            __GLsamplerObject *samplerObj = gc->texture.units[i].boundSampler;
            __GLtextureParamState *ds_params = &ds->texture.texUnits[i].commitParams;
            if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS &&
                lastRealEnableDim < __GL_MAX_TEXTURE_BINDINGS)
            {
                /* Parameters may come from either sampler objects or texture objects */
                if (localMask & __GL_SAMPLERPARAMETER_BITS)
                {
                    __GLsamplerParamState *cs_params = samplerObj
                                                     ? &samplerObj->params
                                                     : (__GLsamplerParamState*)&texObj->params;

                    if (__GL_MEMCMP(ds_params, cs_params, sizeof(__GLsamplerParamState)))
                    {
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_S_BIT, sWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_T_BIT, tWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_WRAP_R_BIT, rWrapMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MIN_FILTER_BIT, minFilter);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MAG_FILTER_BIT, magFilter);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MIN_LOD_BIT, minLod);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_MAX_LOD_BIT, maxLod);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_COMPARE_MODE_BIT, compareMode);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_COMPARE_FUNC_BIT, compareFunc);
                        __GL_CHECK_SAMPLER_PARAM1(__GL_TEXPARAM_SRGB_BIT, sRGB);

                        if (localMask & __GL_TEXPARAM_BORDER_COLOR_BIT)
                        {
                            /* Update and set object-only state dirty */
                            if (__GL_MEMCMP(cs_params->borderColor.fv, texObj->borderColorUsed.fv, 4 * sizeof(GLfloat)))
                            {
                                __GL_MEMCOPY(texObj->borderColorUsed.fv, cs_params->borderColor.fv, 4 * sizeof(GLfloat));

                                if(gc->imports.conformGLSpec)
                                {
                                    texObj->uObjStateDirty.s.borderColorDirty = GL_TRUE;
                                }
                            }
                            __GL_CHECK_SAMPLER_PARAM_ARRAY(__GL_TEXPARAM_BORDER_COLOR_BIT, borderColor.fv, 4, sizeof(GLfloat));

                        }
                    }
                    else
                    {
                        localMask &= ~__GL_SAMPLERPARAMETER_BITS;
                    }
                }

                /* Other parameters must come from texture objects */
                {
                    __GLtextureParamState *cs_params = &texObj->params;
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MIP_HINT_BIT, mipHint);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_BASE_LEVEL_BIT, baseLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_MAX_LEVEL_BIT, maxLevel);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_R_BIT, swizzle[0]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_G_BIT, swizzle[1]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_B_BIT, swizzle[2]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_SWIZZLE_A_BIT, swizzle[3]);
                    __GL_CHECK_TEX_PARAM1(__GL_TEXPARAM_DS_TEXMODE_BIT, dsTexMode);
                }
            }
            else if (realEnableDim < __GL_MAX_TEXTURE_BINDINGS &&
                     lastRealEnableDim == __GL_MAX_TEXTURE_BINDINGS)
            {
                __GL_MEMCOPY(ds_params, &texObj->params, sizeof(__GLtextureParamState));
                if (samplerObj)
                {
                    __GL_MEMCOPY(ds_params, &samplerObj->params, sizeof(__GLsamplerParamState));
                }
            }
        }

        if (lastMaxLevelUsed != gc->texture.units[i].maxLevelUsed)
        {
            localMask |= __GL_TEXPARAM_MAX_LEVEL_BIT;
        }

        /* Reassign localMask back to texUnitAttrState[i]
        */
        gc->texUnitAttrState[i] = localMask;
        if (localMask == 0)
        {
            __glBitmaskClear(&gc->texUnitAttrDirtyMask, i);
        }
        else
        {
            GLuint index;
            __GLtexUnit2Sampler *texUnit2Sampler = &gc->shaderProgram.texUnit2Sampler[i];
            /* Mark states of the samplers bound to this unit dirty */
            for (index = 0; index < texUnit2Sampler->numSamplers; ++index)
            {
                __glBitmaskSet(&gc->shaderProgram.samplerStateDirty, texUnit2Sampler->samplers[index]);
            }
        }
    }

    if (__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_TEX_UNIT_ATTRS);
    }
}


__GL_INLINE GLvoid __glEvaluateImageAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    __GLbitmask unitMask = gc->imageUnitDirtyMask;
    GLint i = -1;
    __GLtextureObject *texObj;
    __GLbitmask localMask = unitMask;

    while (!__glBitmaskIsAllZero(&unitMask))
    {
        if (!__glBitmaskTestAndClear(&unitMask, ++i))
        {
            continue;
        }

        texObj = cs->image.imageUnit[i].texObj;

        if (texObj)
        {
            GLint maxLevelUsed = __glCalcTexMaxLevelUsed(gc, texObj, texObj->params.sampler.minFilter);
            __GLsamplerParamState * samplerParam = &texObj->params.sampler;

            if (GL_FALSE == __glIsTextureComplete(gc, texObj, samplerParam->minFilter, samplerParam->magFilter,
                                                  samplerParam->compareMode, maxLevelUsed))
            {
                cs->image.imageUnit[i].invalid = GL_TRUE;
                break;
            }
        }

        if (!__GL_MEMCMP(&cs->image.imageUnit[i], &ds->image.imageUnit[i], sizeof(__GLimageUnitState)))
        {
            __glBitmaskClear(&localMask, i);
        }
        else
        {
            __GL_MEMCOPY(&ds->image.imageUnit[i], &cs->image.imageUnit[i], sizeof(__GLimageUnitState));
        }
    }

    if (__glBitmaskIsAllZero(&localMask))
    {
        gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_IMG_UNIT_ATTRS);
    }
    else
    {
        gc->imageUnitDirtyMask = localMask;
    }
}

__GL_INLINE GLvoid __glEvaluateProgramAttrib(__GLcontext* gc, __GLattribute* cs, __GLattribute* ds)
{
    GLbitfield localMask = gc->globalDirtyState[__GL_PROGRAM_ATTRS];
    __GLshaderProgramMachine *spMachine = &gc->shaderProgram;
    GLbitfield stageDirties[] =
    {
        __GL_DIRTY_GLSL_VS_SWITCH,
        __GL_DIRTY_GLSL_TCS_SWITCH,
        __GL_DIRTY_GLSL_TES_SWITCH,
        __GL_DIRTY_GLSL_GS_SWITCH,
        __GL_DIRTY_GLSL_FS_SWITCH,
        __GL_DIRTY_GLSL_CS_SWITCH
    };
    __GLSLStage stage;

    if (localMask & __GL_DIRTY_GLSL_MODE_SWITCH)
    {
        localMask |= __GL_DIRTY_GLSL_PROGRAM_SWITCH;

        /* Reset the last program/codeSeq to force toggling the dirty. */
        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            gc->shaderProgram.lastProgObjs[stage] = gcvNULL;
            gc->shaderProgram.lastCodeSeqs[stage] = 0xFFFFFFFF;
        }
    }

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        __GLprogramObject *progObj = spMachine->activeProgObjs[stage];

        if (progObj)
        {
            GLenum dirtyBit = stageDirties[stage];

            if (localMask & dirtyBit)
            {
                /* Check whether stage program and its code really changed */
                if (spMachine->lastProgObjs[stage] == progObj &&
                    spMachine->lastCodeSeqs[stage] == progObj->programInfo.codeSeq)
                {
                    localMask &= ~(dirtyBit);
                }
                else
                {
                    spMachine->lastProgObjs[stage] = progObj;
                    spMachine->lastCodeSeqs[stage] = progObj->programInfo.codeSeq;
                }
            }
        }
    }


    if (localMask & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        GLuint index;
        gcePATCH_ID patchId = gcvPATCH_INVALID;
        /* When program switch, even sampler->texUnit mapping sometimes may not change
        ** The sampler type might changed, need to set the flag to rebuild texEnableDim
        */
        localMask |= __GL_DIRTY_GLSL_SAMPLER;
        localMask |= __GL_DIRTY_GLSL_UNIFORM;

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (patchId == gcvPATCH_REALRACING)
        {
            gc->vertexArray.varrayDirty = 0x3F;
        }
        /* Mark all array binding points dirty when program switched */
        for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
        {
            __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_TRUE);
        }
    }

    if (localMask & __GL_DIRTY_GLSL_SAMPLER)
    {
        GLuint sampler;
        GLuint *newSampler2TexUnit = cs->program.sampler2TexUnit;
        GLuint *oldSampler2TexUnit = ds->program.sampler2TexUnit;

        /* Current sampler2TexUnit will be build in __glBuildTexEnableDim */
        __glBuildTexEnableDim(gc, cs, ds);

        for (sampler = 0; sampler < gc->shaderProgram.maxSampler; ++sampler)
        {
            if (oldSampler2TexUnit[sampler] != newSampler2TexUnit[sampler])
            {
                /* Program switch depend on the comparison to set samplerMapDirty */
                __glBitmaskSet(&gc->shaderProgram.samplerMapDirty, sampler);
                oldSampler2TexUnit[sampler] = newSampler2TexUnit[sampler];
            }
            else
            {
                __glBitmaskClear(&gc->shaderProgram.samplerMapDirty, sampler);
            }
        }
    }

    gc->globalDirtyState[__GL_PROGRAM_ATTRS] = localMask;
}

/******** Draw functions *****************/

#if __GL_TIMEING_DRAW
extern BOOLEAN logTimeEnable;
extern GLuint64 tTick;
extern FILE*  timeingLogFile ;
extern GLbyte *printBuffer ;
extern GLbyte* curOutput, *endSlot;

__GL_INLINE GLuint64 GetCycleCount()
{
    __asm _emit 0x0F
    __asm _emit 0x31
}

#define ENTERFUNC_TM() \
    if (logTimeEnable) tTick = GetCycleCount();

#define LEAVEFUNC_TM()                                  \
    if (logTimeEnable) {                                \
        tTick = GetCycleCount() - tTick;                \
        curOutput += sprintf(curOutput, "%d\t", tTick); \
        if (curOutput > endSlot ) {                     \
            if (timeingLogFile) {                       \
                sprintf(curOutput, "\0");               \
                fprintf(timeingLogFile, printBuffer);   \
                curOutput = printBuffer;                \
            }                                           \
        }                                               \
    }

#else
#define ENTERFUNC_TM()
#define LEAVEFUNC_TM()
#endif

__GL_INLINE GLboolean __glDrawBegin(__GLcontext *gc, GLenum mode)
{
    return (*gc->dp.drawBegin)(gc, mode);
}

__GL_INLINE GLboolean __glDrawValidateState(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    __GLattribute *ds = &gc->commitState;

    if (gc->invalidCommonCommit || gc->invalidDrawCommit)
    {
        /* Full dirty to flush all */
        __glOverturnCommitStates(gc);
        __glSetAttributeStatesDirty(gc);

        gc->shaderProgram.maxSampler = gc->constants.shaderCaps.maxTextureSamplers;
        gc->shaderProgram.maxUnit = gc->constants.shaderCaps.maxCombinedTextureImageUnits;
    }

    if (gc->globalDirtyState[__GL_ALL_ATTRS])
    {
        if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1])
        {
            __glEvaluateAttribGroup1(gc, cs, ds);
        }

        if (gc->globalDirtyState[__GL_DIRTY_ATTRS_2])
        {
            __glEvaluateAttribGroup2(gc, cs, ds);
        }
#ifdef OPENGL40
        if ((gc->imports.conformGLSpec) && gc->globalDirtyState[__GL_DIRTY_ATTRS_3])
        {
            __glEvaluateAttribGroup3(gc, cs, ds);
        }
#endif
        /* Program state must be checked before texture */
        if (gc->globalDirtyState[__GL_PROGRAM_ATTRS])
        {
            __glEvaluateProgramAttrib(gc, cs, ds);
        }

        if (!__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
        {
            __glEvaluateTextureAttrib(gc, cs, ds);
        }

        if (!__glBitmaskIsAllZero(&gc->imageUnitDirtyMask))
        {
            __glEvaluateImageAttrib(gc, cs, ds);
        }

#ifdef OPENGL40
        if ((gc->imports.conformGLSpec) && gc->globalDirtyState[__GL_LIGHTING_ATTRS])
        {
            __glEvaluateLightingAttrib(gc,cs,ds);
        }

        if ((gc->imports.conformGLSpec) && gc->globalDirtyState[__GL_LIGHT_SRC_ATTRS])
        {
            __glEvaluateLightSrcAttrib(gc,cs,ds);
        }
#endif
    }

    /*
    ** Note "drawValidateState" is designed to never fail. Otherwise GL states might still not
    ** be flush to HW before their dirty bits are cleared in the next draw's evaluations.
    ** If there are HW limitations that the draw must be skipped, set DrawNothing function in chip layer
    ** for those cases.
    */

    return gc->dp.drawValidateState(gc);
}

#ifdef OPENGL40

GLenum indexPrimMode[] = {
    GL_POINTS,                          /* GL_POINTS */
    GL_LINES,                           /* GL_LINES */
    GL_LINE_LOOP,                       /* GL_LINE_LOOP */
    GL_LINE_STRIP,                      /* GL_LINE_STRIP */
    GL_TRIANGLES,                       /* GL_TRIANGLES */
    GL_TRIANGLE_STRIP,                  /* GL_TRIANGLE_STRIP */
    GL_TRIANGLE_FAN,                    /* GL_TRIANGLE_FAN */
    GL_QUADS,                           /* GL_QUADS */
    GL_QUAD_STRIP,                      /* GL_QUAD_STRIP */
    GL_POLYGON,                         /* GL_POLYGON */
    GL_LINES_ADJACENCY_EXT,             /* GL_LINES_ADJACENCY_EXT */
    GL_LINE_STRIP_ADJACENCY_EXT,        /* GL_LINE_STRIP_ADJACENCY_EXT */
    GL_TRIANGLES_ADJACENCY_EXT,         /* GL_TRIANGLES_ADJACENCY_EXT */
    GL_TRIANGLE_STRIP_ADJACENCY_EXT,    /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};

/***** Vertex stream config functions ********/

GLvoid __glConfigImmedVertexStream(__GLcontext *gc, GLenum mode)
{
    __GLvertexInput *input = NULL;
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    GLuint offset;
    GLubyte inputIdx;
    GLuint mask, i;
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    /* Immediate mode vertex input always use single stream */
    gc->vertexStreams.numStreams = 1;
    gc->vertexStreams.endVertex = gc->input.vertex.index;
    gc->vertexStreams.indexCount = gc->input.indexCount;
    gc->vertexStreams.startVertex = 0;
    gc->vertexStreams.primElemSequence = gc->input.primElemSequence;
    gc->vertexStreams.primElementMask = gc->input.primInputMask;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~gc->input.primInputMask) & (~__GL_INPUT_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream =
        (gc->input.primInputMask & __GL_INPUT_EDGEFLAG) ? gc->input.edgeflag.pointer : NULL;

    stream = &gc->vertexStreams.streams[0];
    stream->numElements = gc->input.numberOfElements;
    stream->streamAddr = (GLuint *)gc->input.vertexDataBuffer;
    stream->stride = (gc->input.vertTotalStrideDW << 2);
    stream->privPtrAddr = NULL;

    offset = 0;
    for (i = 0; i < gc->input.numberOfElements; i++)
    {
        element = &stream->streamElement[i];

        inputIdx = 0;
        mask = gc->input.primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if ((mask & 0x1) && (gc->input.currentInput[inputIdx].offsetDW == offset))
            {
                input = &gc->input.currentInput[inputIdx];
                offset += input->sizeDW;
                break;
            }
            mask >>= 1;
            inputIdx += 1;
        }

        GL_ASSERT( ( __GL_ONE_64 << inputIdx ) & gc->input.primInputMask );

        element->inputIndex = inputIdx;
        element->streamIndex = 0;
        element->offset = (input->offsetDW << 2);
        element->size = input->sizeDW;
        element->type = GL_FLOAT;
        element->normalized = GL_FALSE;
        element->integer = GL_FALSE;
        if (element->inputIndex == __GL_INPUT_DIFFUSE_INDEX && element->size == 1)
        {
            /* Special case for RGBA ubyte format */
            element->size = 4;
            element->type = GL_UNSIGNED_BYTE;
            element->normalized = GL_TRUE;
        }
    }

    gc->vertexStreams.streamMode = IMMEDIATE_STREAMMODE;


    vertexArray->indexCount = 0;
    vertexArray->instanceCount  = 1;
    vertexArray->start = 0;
    vertexArray->end = gc->input.vertex.index;
    vertexArray->baseVertex = 0;
    vertexArray->drawIndirect = GL_FALSE;
    vertexArray->multidrawIndirect = GL_FALSE;
}
#endif

__GL_INLINE GLboolean __glDrawEnd(__GLcontext *gc)
{
    GLboolean ret = gc->dp.drawEnd(gc);

    if (ret)
    {
        GLuint index;
        __GLprogramObject *progObj;
        __GLSLStage stage;

        for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
        {
            __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_FALSE);
        }

        __glBitmaskSetAll(&gc->shaderProgram.samplerMapDirty, GL_FALSE);
        gc->shaderProgram.samplerStateDirty = gc->shaderProgram.samplerStateKeepDirty;
        __glBitmaskSetAll(&gc->shaderProgram.samplerStateKeepDirty, GL_FALSE);

        if (gc->globalDirtyState[__GL_ALL_ATTRS])
        {
            /* Clear all the attribute dirty bits */
            __glClearAttributeStates(gc);
        }

        gc->vertexArray.varrayDirty = 0;

        if (gc->invalidCommonCommit || gc->invalidDrawCommit)
        {
            gc->invalidCommonCommit = GL_FALSE;
            gc->invalidDrawCommit = GL_FALSE;

            if (gc->shaderProgram.mode == __GLSL_MODE_GRAPHICS)
            {
                /* reset valude.*/
                gc->shaderProgram.maxSampler = 0;
                gc->shaderProgram.maxUnit = 0;

                for (stage = __GLSL_STAGE_VS; stage <= __GLSL_STAGE_FS; ++stage)
                {
                    progObj = __glGetCurrentStageProgram(gc, stage);
                    if (progObj)
                    {
                        gc->shaderProgram.maxSampler = gcmMAX(gc->shaderProgram.maxSampler, progObj->maxSampler);
                        gc->shaderProgram.maxUnit = gcmMAX(gc->shaderProgram.maxUnit, progObj->maxUnit);
                    }
                }
            }
        }

        /* Temp disable the dirty in case to affect perf */
    }

    return ret;
}


__GL_INLINE GLboolean  __glDrawPattern(__GLcontext *gc)
{
    GLboolean matched = GL_FALSE;

    if (gc->pattern.enable)
    {
        gc->pattern.lastPattern = gc->pattern.matchPattern;
        gc->pattern.matchPattern = gcvNULL;

        if (gc->pattern.patternMatchMask == 0)
        {
            /* Reset if match nothing, */
            gc->pattern.patternMatchMask = ( 1 << GLES_PATTERN_GFX0 | 1 << GLES_PATTERN_GFX1);
            gc->pattern.matchCount = 0;
            gc->pattern.apiCount = 0;

            if (gc->pattern.lastPattern != gcvNULL)
            {
                /* Fail from last pattern*/
                /*gcmPRINT("Fail from last pattern");*/

                /* For this pattern, we need update blend color/func, and depth function commit state and dirty */
                gc->commitState.raster.blendColor.r = gc->state.raster.blendColor.r + 1;
                gc->commitState.raster.blendColor.g = gc->state.raster.blendColor.g + 1;
                gc->commitState.raster.blendColor.b = gc->state.raster.blendColor.b + 1;
                gc->commitState.raster.blendColor.a = gc->state.raster.blendColor.a + 1;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDCOLOR_BIT);

                gc->commitState.raster.blendSrcRGB[0] = gc->state.raster.blendSrcRGB[0] + 1;
                gc->commitState.raster.blendSrcAlpha[0] = gc->state.raster.blendSrcAlpha[0] + 1;
                gc->commitState.raster.blendDstRGB[0] = gc->state.raster.blendDstRGB[0] + 1;
                gc->commitState.raster.blendDstAlpha[0] = gc->state.raster.blendDstAlpha[0] + 1;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_BLENDFUNC_BIT);

                gc->commitState.depth.testFunc = gc->state.depth.testFunc;
                __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_1, __GL_DEPTHFUNC_BIT);
            }

            gc->pattern.state = GLES_PATTERN_STATE_CHECK;
        }
        else
        {
            GLuint i;

            for (i = 0; i < GLES_PATTERN_COUNT; i++)
            {
                /* get the match APIs, only could get one */
                if (gc->pattern.patternMatchMask == (gc->pattern.patternMatchMask & (1 << i)))
                {
                    if (gc->pattern.patterns[i]->apiCount == gc->pattern.matchCount)
                    {
                        gc->pattern.matchPattern = gc->pattern.patterns[i];
                        gc->pattern.state = GLES_PATTERN_STATE_MATCHED;
                        gc->pattern.matchCount = 0;
                        gc->pattern.apiCount = 0;

                        /* the same with the last one, send command */
                        if (gc->pattern.lastPattern == gc->pattern.matchPattern)
                        {
                            /* match! */
                            /*gcmPRINT("pattern found, run HW command, and go!");*/

                            if (i == GLES_PATTERN_GFX0 || i == GLES_PATTERN_GFX1)
                            {
                                matched = (*gc->dp.drawPattern)(gc);
                            }

                            return matched;
                        }
                        else
                        {
                            /* match! */
                            /*gcmPRINT("pattern found, get another round!");*/
                        }
                    }
                    else
                    {
                        /* Not full matched, continue check */
                        gc->pattern.state = GLES_PATTERN_STATE_CHECK;
                    }
                    break;
                }
            }
        }
    }

    return matched;
}


static GLboolean __glCheckVBOSize(__GLcontext *gc)
{
    GLboolean ret = GL_TRUE;
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;

    if(!gc->imports.conformGLSpec)
    {
        if (gc->imports.robustAccess && __glExtension[__GL_EXTID_KHR_robust_buffer_access_behavior].bEnabled)
        {
            return GL_TRUE;
        }
    }
    else
    {
        if (__glExtension[__GL_EXTID_KHR_robust_buffer_access_behavior].bEnabled)
        {
            return GL_TRUE;
        }
    }

    if (!(vertexArray->multidrawIndirect || vertexArray->drawIndirect))
    {
        GLsizeiptr endBytes;
        __GLbufferObject *boundIdxObj = __glGetBoundBufObj(gc, __GL_ELEMENT_ARRAY_BUFFER_INDEX);
        GLuint indexCount = vertexArray->indexCount;

        if (boundIdxObj && (indexCount != 0))
        {
            GLuint sizeofIndex = 0;
            switch (vertexArray->indexType)
            {
            case GL_UNSIGNED_BYTE:
                sizeofIndex = 1;
                break;
            case GL_UNSIGNED_SHORT:
                sizeofIndex = 2;
                break;
            case GL_UNSIGNED_INT:
                sizeofIndex = 4;
                break;
            }

            endBytes = (GLsizeiptr)(indexCount * sizeofIndex + __GL_PTR2UINT(vertexArray->indices));

            if (endBytes > boundIdxObj->size)
            {
                ret = GL_FALSE;
            }
        }
        else if (indexCount == 0)
        {
            GLuint index = 0;
            GLuint instanceCount = vertexArray->instanceCount;
            __GLvertexArrayState *curVertexArray = &gc->vertexArray.boundVAO->vertexArray;
            GLuint64 attribEnabled = curVertexArray->attribEnabled;
            __GLprogramObject *vsProgObj = __glGetCurrentStageProgram(gc,__GLSL_STAGE_VS);
            GLuint64 inputMask = 0;

            if (vsProgObj)
            {
#ifdef OPENGL40
                if (gc->imports.conformGLSpec)
                {
                    inputMask = vsProgObj->bindingInfo.vsInputMask;
                }
                else  /* Running OES api */
#endif
                {
                    inputMask = vsProgObj->bindingInfo.vsInputArrayMask;
                }
            }

            while (attribEnabled & inputMask)
            {
                if ((attribEnabled & inputMask) & 0x1)
                {
                    GLuint remain;
                    __GLvertexAttrib *pAttrib = &curVertexArray->attribute[index];
                    __GLvertexAttribBinding *pAttribBinding = &curVertexArray->attributeBinding[pAttrib->attribBinding];
                    __GLbufferObject *boundVBObj = (gc->imports.conformGLSpec) ? pAttribBinding->boundArrayObj:
                                                   __glGetCurrentVertexArrayBufObj(gc, pAttrib->attribBinding);

                    if (boundVBObj)
                    {
                        if (pAttribBinding->divisor)
                        {
                            remain = (instanceCount % pAttribBinding->divisor) ? 1 : 0;
                            endBytes = pAttrib->relativeOffset + pAttribBinding->offset + ((instanceCount / pAttribBinding->divisor) + remain - 1) *  pAttribBinding->stride +
                                __glUtilCalculateStride(pAttrib->size, pAttrib->type);
                            if (endBytes > boundVBObj->size)
                            {
                                ret = GL_FALSE;
                                break;
                            }
                        }
                        else
                        {
                            endBytes = pAttrib->relativeOffset + pAttribBinding->offset + (vertexArray->end - 1) * pAttribBinding->stride +
                                __glUtilCalculateStride(pAttrib->size, pAttrib->type);
                            if (endBytes > boundVBObj->size)
                            {
                                ret = GL_FALSE;
                                break;
                            }
                        }
                    }
                }

                index++;
                inputMask >>= 1;
                attribEnabled >>= 1;
            }
        }
    }

    return ret;
}

GLvoid  __glDrawPrimitive(__GLcontext *gc, GLenum mode)
{
    ENTERFUNC_TM()

    if (gc->conditionalRenderDiscard)
    {
        LEAVEFUNC_TM();
        return;
    }

    if (mode != gc->vertexArray.primMode)
    {
        gc->vertexArray.primMode = mode;
        __GL_SET_VARRAY_MODE_BIT(gc);
    }

#ifdef OPENGL40
    if(gc->imports.conformGLSpec)
    {
        if (gc->vertexStreams.primMode != mode)
        {
            gc->vertexStreams.primMode = mode;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
        }

        if ( gc->input.beginMode != __GL_IN_BEGIN && gc->input.beginMode != __GL_SMALL_LIST_BATCH )
#endif
        {
            if (!__glCheckVBOSize(gc))
           {
               __GL_ERROR_RET(GL_INVALID_OPERATION);
           }

           if (__GLSL_MODE_GRAPHICS != gc->shaderProgram.mode)
          {
               gc->shaderProgram.mode = __GLSL_MODE_GRAPHICS;
               __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_MODE_SWITCH);
           }
        }
    }
    else
    {
          if (!__glCheckVBOSize(gc))
          {
              __GL_ERROR_RET(GL_INVALID_OPERATION);
          }

           if (__GLSL_MODE_GRAPHICS != gc->shaderProgram.mode)
          {
               gc->shaderProgram.mode = __GLSL_MODE_GRAPHICS;
               __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_MODE_SWITCH);
          }
    }
#if gcdPATTERN_FAST_PATH
    if (__glDrawPattern(gc))
    {
        LEAVEFUNC_TM();
        return;
    }

#endif

    /* Realize render buffers change*/
    __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_BIT);


    if (__glDrawBegin(gc, mode))
    {
        GLboolean failed = GL_TRUE;

        do
        {
            /* Try to validate all states to HW */
            if (!__glDrawValidateState(gc))
            {
                break;
            }

            /* Try to issue draw command to HW */
            if (!gc->dp.drawPrimitive(gc))
            {
                break;
            }

            /* Try to clean up for this draw */
            if (!__glDrawEnd(gc))
            {
                break;
            }

            failed = GL_FALSE;
        } while (GL_FALSE);

        if (failed)
        {
            /* Mark commit states as invalid */
            gc->invalidCommonCommit = GL_TRUE;
            gc->invalidDrawCommit = GL_TRUE;
            gcmPRINT("ES30: some draw get error and skipped during validation");
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

    LEAVEFUNC_TM();
}

/***** End of draw functions *****************/

#ifdef OPENGL40
extern GLvoid configStream(__GLcontext *gc);

/* Immediate mode draw function */
GLvoid  __glDrawImmedPrimitive(__GLcontext *gc)
{
    GLenum mode;

    ENTERFUNC_TM();
    mode = (gc->input.indexCount ? indexPrimMode[gc->input.primMode] : gc->input.primMode);

    if (mode != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = mode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    /* Get the latest drawable information */
//   LINUX_LOCK_FRAMEBUFFER(gc);

 //   __glEvaluateAttribDrawableChange(gc);

    __glConfigImmedVertexStream(gc, mode);

//    configStream(gc);

    __glDrawPrimitive(gc,mode);

//    LINUX_UNLOCK_FRAMEBUFFER(gc);
    LEAVEFUNC_TM();
}

GLvoid __glEvaluateAttributeChange(__GLcontext *gc)
{
    __glDrawValidateState(gc);
}


GLenum indexPrimModeDL[] = {
    GL_POINTS,            /* GL_POINTS */
    GL_LINES,            /* GL_LINES */
    GL_LINES,            /* GL_LINE_LOOP */
    GL_LINES,            /* GL_LINE_STRIP */
    GL_TRIANGLES,        /* GL_TRIANGLES */
    GL_TRIANGLES,        /* GL_TRIANGLE_STRIP */
    GL_TRIANGLES,        /* GL_TRIANGLE_FAN */
    GL_QUADS,            /* GL_QUADS */
    GL_TRIANGLES,        /* GL_QUAD_STRIP */
    GL_TRIANGLES,        /* GL_POLYGON */
    GL_LINES_ADJACENCY_EXT,             /* GL_LINES_ADJACENCY_EXT */
    GL_LINE_STRIP_ADJACENCY_EXT,        /* GL_LINE_STRIP_ADJACENCY_EXT */
    GL_TRIANGLES_ADJACENCY_EXT,         /* GL_TRIANGLES_ADJACENCY_EXT */
    GL_TRIANGLE_STRIP_ADJACENCY_EXT,    /* GL_TRIANGLE_STRIP_ADJACENCY_EXT */
};
GLvoid __glConfigDlistVertexStream(__GLcontext *gc, __GLPrimBegin *primBegin, GLfloat *beginAddr,
        GLuint vertexCount, GLuint indexCount, GLushort *indexBuffer, GLvoid **privPtrAddr, GLvoid **ibPrivPtrAddr)
{
    __GLstreamDecl *stream;
    __GLvertexElement *element;
    __GLvertexArrayMachine *vertexArray = &gc->vertexArray;
    GLint i, offset;
    GLubyte inputIdx;
    GLuint mask;

    /* Display list consistent primitive always use single stream */
    gc->vertexStreams.numStreams = 1;
    gc->vertexStreams.endVertex = vertexCount;
    gc->vertexStreams.indexCount = indexCount;
    gc->vertexStreams.startVertex = 0;
    gc->vertexStreams.primElemSequence = primBegin->primElemSequence;
    gc->vertexStreams.primElementMask = primBegin->primInputMask;
    gc->vertexStreams.missingAttribs = gc->input.requiredInputMask & (~primBegin->primInputMask) & (~__GL_INPUT_EDGEFLAG) & (~__GL_INPUT_VERTEX);
    gc->vertexStreams.edgeflagStream = primBegin->edgeflagBuffer;
    if (indexCount)
    {   /* Configure the index stream */
        gc->vertexStreams.indexStream.streamAddr = (GLvoid *)indexBuffer;
        gc->vertexStreams.indexStream.type = GL_UNSIGNED_SHORT;
        gc->vertexStreams.indexStream.ppIndexBufPriv = ibPrivPtrAddr;
        gc->vertexStreams.indexStream.offset = 0;
    }

    stream = &gc->vertexStreams.streams[0];
    stream->numElements = primBegin->elementCount;
    stream->streamAddr = (GLuint *)beginAddr;
    stream->stride = (primBegin->totalStrideDW << 2);
    stream->privPtrAddr = privPtrAddr;

    offset = 0;
    for (i = 0; i < (GLint)stream->numElements; i++)
    {
        element = &stream->streamElement[i];
        inputIdx = 0;
        mask = primBegin->primInputMask & (~__GL_INPUT_EDGEFLAG);
        while (mask)
        {
            if ((mask & 0x1) && (primBegin->elemOffsetDW[inputIdx] == offset))
            {
                offset += primBegin->elemSizeDW[inputIdx];
                break;
            }

            mask >>= 1;
            inputIdx += 1;
        }

        element->inputIndex = inputIdx;
        element->streamIndex = 0;
        element->offset = (primBegin->elemOffsetDW[inputIdx] << 2);
        element->size = primBegin->elemSizeDW[inputIdx];
        element->type = GL_FLOAT;
        element->normalized = GL_FALSE;
        element->integer = GL_FALSE;
        if (element->inputIndex == __GL_INPUT_DIFFUSE_INDEX && element->size == 1)
        {
            /* Special case for RGBA ubyte format */
            element->size = 4;
            element->type = GL_UNSIGNED_BYTE;
            element->normalized = GL_TRUE;
        }
    }

    if (indexCount)
    {
        vertexArray->end = 0;
        vertexArray->indices = indexBuffer;
        vertexArray->indexType = GL_UNSIGNED_SHORT;
    }
    else
    {
        vertexArray->end = vertexCount;
    }
    gc->vertexStreams.streamMode = DLIST_STREAMMODE;
    vertexArray->instanceCount  = 1;
    vertexArray->start = 0;
    vertexArray->baseVertex = 0;
    vertexArray->indexCount = indexCount;
    vertexArray->drawIndirect = GL_FALSE;
    vertexArray->multidrawIndirect = GL_FALSE;
}
GLvoid __glDrawDlistPrimitive(__GLcontext *gc, __GLPrimBegin *primBegin)
{
    GLfloat *beginAddr;
    GLint vertexCount, indexCount, i;
    GLboolean bothFaceFill, indexedPrim;
    GLenum mode;

    ENTERFUNC_TM();

    /* Compute the required attribute mask */
    if (gc->input.inputMaskChanged)
    {
        __glComputeRequiredInputMask(gc);
        gc->input.inputMaskChanged = GL_FALSE;
    }
    gc->input.requiredInputMask = gc->input.currentInputMask & edgeFlagInputMask[primBegin->primType];

    bothFaceFill = (primBegin->primType >= GL_TRIANGLES && gc->state.polygon.bothFaceFill);
    indexedPrim = ((bothFaceFill || primBegin->primType <= GL_LINE_STRIP) && primBegin->indexCount > 0);

    mode = (indexedPrim) ? indexPrimModeDL[primBegin->primType] : primBegin->primType;
    mode = (!bothFaceFill) ? GL_LINE_LOOP : mode;
    if (mode != gc->vertexStreams.primMode)
    {
        gc->vertexStreams.primMode = mode;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_DIRTY_ATTRS_2, __GL_PRIMMODE_BIT);
    }

    if (indexedPrim || primBegin->primCount == 1)
    {
        indexCount = (indexedPrim || bothFaceFill) ? primBegin->indexCount : 0;

        beginAddr = (GLfloat *)(primBegin + 1);
         __glConfigDlistVertexStream(gc, primBegin, beginAddr, primBegin->vertexCount,
                indexCount, primBegin->indexBuffer, &primBegin->privateData, &primBegin->ibPrivateData);

        LEAVEFUNC_TM();
        /* Draw indexed primitive */
        __glDrawPrimitive(gc, mode);
        ENTERFUNC_TM();

    }
    else
    {
        for (i = 0; i < primBegin->primCount; i++) {
            beginAddr = primBegin->primStartAddr[i];
            vertexCount = primBegin->primVertCount[i];
            __glConfigDlistVertexStream(gc, primBegin, beginAddr, vertexCount, 0, NULL, NULL, NULL);

            LEAVEFUNC_TM();
            __glDrawPrimitive(gc, mode);
            ENTERFUNC_TM();
        }
    }

    LEAVEFUNC_TM();
}

#endif

/****  Start of dispatch compute */
__GL_INLINE GLboolean __glComputeBegin(__GLcontext *gc)
{
    return (*gc->dp.computeBegin)(gc);
}


__GL_INLINE GLboolean __glComputeValidateState(__GLcontext *gc)
{
    __GLattribute *cs = &gc->state;
    __GLattribute *ds = &gc->commitState;

    if (gc->invalidCommonCommit)
    {
        /* Full dirty to flush all */
        __glOverturnCommitStates(gc);
        __glSetAttributeStatesDirty(gc);

        gc->invalidCommonCommit = GL_FALSE;
    }

    /* Program state must be checked before texture */
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS])
    {
        __glEvaluateProgramAttrib(gc, cs, ds);
    }

    if (!__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
    {
        __glEvaluateTextureAttrib(gc, cs, ds);
    }

    if (!__glBitmaskIsAllZero(&gc->imageUnitDirtyMask))
    {
        __glEvaluateImageAttrib(gc, cs, ds);
    }

    /*
    ** Note "computeValidateState" is designed to never fail. Otherwise GL states might still not
    ** be flush to HW before their dirty bits are cleared in the next draw's evaluations.
    ** If there are HW limitations that the draw must be skipped, set DrawNothing function in chip layer
    ** for those cases.
    */

    return gc->dp.computeValidateState(gc);
}

__GL_INLINE GLboolean __glComputeEnd(__GLcontext *gc)
{
    (*gc->dp.computeEnd)(gc);

    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_UNIFORM_BUFFER_INDEX], GL_FALSE);
    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_ATOMIC_COUNTER_BUFFER_INDEX], GL_FALSE);
    __glBitmaskSetAll(&gc->bufferObject.bindingDirties[__GL_SHADER_STORAGE_BUFFER_INDEX], GL_FALSE);

    __glBitmaskSetAll(&gc->shaderProgram.samplerMapDirty, GL_FALSE);
    gc->shaderProgram.samplerStateDirty = gc->shaderProgram.samplerStateKeepDirty;
    __glBitmaskSetAll(&gc->shaderProgram.samplerStateKeepDirty, GL_FALSE);

    __glBitmaskSetAll(&gc->texUnitAttrDirtyMask, GL_FALSE);
    __glBitmaskSetAll(&gc->imageUnitDirtyMask, GL_FALSE);
    __GL_MEMZERO(gc->texUnitAttrState, gc->constants.shaderCaps.maxCombinedTextureImageUnits * sizeof(GLuint64));

    gc->globalDirtyState[__GL_PROGRAM_ATTRS] = 0;
    gc->globalDirtyState[__GL_ALL_ATTRS] &= ~(__GL_ONE_32 << __GL_PROGRAM_ATTRS |
                                              __GL_ONE_32 << __GL_TEX_UNIT_ATTRS |
                                              __GL_ONE_32 << __GL_IMG_UNIT_ATTRS);

    return GL_TRUE;
}

GLvoid __glDispatchCompute(__GLcontext *gc)
{
    if (__GLSL_MODE_COMPUTE != gc->shaderProgram.mode)
    {
        gc->shaderProgram.mode = __GLSL_MODE_COMPUTE;
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_MODE_SWITCH);
    }

    if (__glComputeBegin(gc))
    {
        GLboolean failed = GL_TRUE;

        do
        {
            /* Try to validate all states to HW */
            if (!__glComputeValidateState(gc))
            {
                break;
            }

            /* Try to issue compute command to HW */
            if (!gc->dp.dispatchCompute(gc))
            {
                break;
            }

            /* Try to clean up for this dispatch */
            if (!__glComputeEnd(gc))
            {
                break;
            }

            failed = GL_FALSE;
        } while (GL_FALSE);


        if (failed)
        {
            /* Mark commit states as invalid */
            gc->invalidCommonCommit = GL_TRUE;
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }
}

