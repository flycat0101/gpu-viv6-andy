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


#include "gc_es_context.h"
#include "gc_es_device.h"
#include "gc_es_object_inline.c"

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE


GLboolean __glDeleteQueryObj(__GLcontext *gc, __GLqueryObject *queryObj);

/*
** Convert the results of a query from one type to another.
*/
GLvoid __glConvertResult(__GLcontext *gc, GLint fromType, const GLvoid *rawdata,
                         GLint toType, GLvoid *result, GLint size)
{
    GLint i;

    switch (fromType)
    {
    case __GL_FLOAT:
        switch (toType)
        {
        case __GL_FLOAT32:
            for (i=0; i < size; i++)
            {
                ((GLfloat *)result)[i] = ((const GLfloat *)rawdata)[i];
            }
            break;
        case __GL_INT32:
            for (i=0; i < size; i++)
            {
                ((GLint *)result)[i] = (GLint)(((const GLfloat *)rawdata)[i] >= 0.0
                                     ? ((const GLfloat *)rawdata)[i] + __glHalf
                                     : ((const GLfloat *)rawdata)[i] - __glHalf);
            }
            break;
        case __GL_INT64:
            for (i=0; i < size; i++)
            {
                ((GLint64 *)result)[i] = (GLint64)(((const GLfloat *)rawdata)[i] >= 0.0
                                       ? ((const GLfloat *)rawdata)[i] + __glHalf
                                       : ((const GLfloat *)rawdata)[i] - __glHalf);
            }
            break;
        case __GL_BOOLEAN:
            for (i=0; i < size; i++)
            {
                ((GLboolean *)result)[i] = ((const GLfloat *)rawdata)[i] ? 1 : 0;
            }
            break;
        }
        break;

    case __GL_COLOR:
        switch (toType)
        {
        case __GL_FLOAT32:
            for (i=0; i < size; i++)
            {
                ((GLfloat *)result)[i] = ((const GLfloat *)rawdata)[i];
            }
            break;
        case __GL_INT32:
            for (i=0; i < size; i++)
            {
                ((GLint *)result)[i] = __GL_FLOAT_TO_I(((const GLfloat *)rawdata)[i]);
            }
            break;
        case __GL_INT64:
            for (i=0; i < size; i++)
            {
                ((GLint64 *)result)[i] = (GLint64)__GL_FLOAT_TO_I(((const GLfloat *)rawdata)[i]);
            }
            break;
        case __GL_BOOLEAN:
            for (i=0; i < size; i++)
            {
                ((GLboolean *)result)[i] = ((const GLfloat *)rawdata)[i] ? 1 : 0;
            }
            break;
        }
        break;

    case __GL_INT32:
        switch (toType)
        {
        case __GL_FLOAT32:
            for (i=0; i < size; i++)
            {
                ((GLfloat *)result)[i] = (GLfloat)(((const GLint *)rawdata)[i]);
            }
            break;
        case __GL_INT32:
            for (i=0; i < size; i++)
            {
                ((GLint *)result)[i] = ((const GLint *)rawdata)[i];
            }
            break;
        case __GL_INT64:
            for (i=0; i < size; i++)
            {
                ((GLint64 *)result)[i] = ((const GLint *)rawdata)[i];
            }
            break;
        case __GL_BOOLEAN:
            for (i=0; i < size; i++)
            {
                ((GLboolean *)result)[i] = ((const GLint *)rawdata)[i] ? 1 : 0;
            }
            break;
        }
        break;

    case __GL_INT64:
        switch (toType)
        {
        case __GL_FLOAT32:
            for (i=0; i < size; i++)
            {
                ((GLfloat *)result)[i] = (GLfloat)(((const GLint64 *)rawdata)[i]);
            }
            break;
        case __GL_INT32:
            for (i=0; i < size; i++)
            {
                ((GLint *)result)[i] = (GLint)(((const GLint64 *)rawdata)[i]);
            }
            break;
        case __GL_INT64:
            for (i=0; i < size; i++)
            {
                ((GLint64 *)result)[i] = ((const GLint64 *)rawdata)[i];
            }
            break;
        case __GL_BOOLEAN:
            for (i=0; i < size; i++)
            {
                ((GLboolean *)result)[i] = ((const GLint64 *)rawdata)[i] ? 1 : 0;
            }
            break;
        }
        break;

    case __GL_BOOLEAN:
        switch (toType)
        {
        case __GL_FLOAT32:
            for (i=0; i < size; i++)
            {
                ((GLfloat *)result)[i] = ((const GLboolean *)rawdata)[i];
            }
            break;
        case __GL_INT32:
            for (i=0; i < size; i++)
            {
                ((GLint *)result)[i] = ((const GLboolean *)rawdata)[i];
            }
            break;
        case __GL_INT64:
            for (i=0; i < size; i++)
            {
                ((GLint64 *)result)[i] = ((const GLboolean *)rawdata)[i];
            }
            break;
        case __GL_BOOLEAN:
            for (i=0; i < size; i++)
            {
                ((GLboolean *)result)[i] = ((const GLboolean *)rawdata)[i] ? 1 : 0;
            }
            break;
        }
        break;
    }
}

__GL_INLINE GLint __glGetFboColorBits(__GLcontext *gc, GLenum bitType)
{
    __GLframebufferObject *framebufferObj = gc->frameBuffer.drawFramebufObj;
    __GLfboAttachPoint *attachPoint = gcvNULL;
    __GLformatInfo * formatInfo = gcvNULL;
    GLint i;
    GLint redBits = 0, greenBits = 0, blueBits = 0, alphaBits = 0;
    GLint depthBits = 0, stencilBits = 0;

    if ((bitType == GL_RED_BITS) || (bitType == GL_BLUE_BITS) ||
        (bitType == GL_GREEN_BITS) || (bitType == GL_ALPHA_BITS))
    {
        for (i = 0; i < __GL_MAX_COLOR_ATTACHMENTS; i++)
        {
            attachPoint = &framebufferObj->attachPoint[i];

            /* Attach point with type NONE is framebuffer attachment complete. */
            if (attachPoint->objType == GL_NONE)
            {
                continue;
            }

            formatInfo = __glGetFramebufferFormatInfo(gc, framebufferObj, GL_COLOR_ATTACHMENT0+i);
            if (formatInfo)
            {
                break;
            }
        }

        if (formatInfo)
        {
            redBits   = formatInfo->redSize;
            blueBits  = formatInfo->blueSize;
            greenBits = formatInfo->greenSize;
            alphaBits = formatInfo->alphaSize;
        }

        switch (bitType)
        {
        case GL_RED_BITS:
            return redBits;
        case GL_GREEN_BITS:
            return greenBits;
        case GL_BLUE_BITS:
            return blueBits;
        case GL_ALPHA_BITS:
            return alphaBits;
        default:
            GL_ASSERT(0);
        }
    }
    else if (bitType == GL_DEPTH_BITS)
    {
        formatInfo = __glGetFramebufferFormatInfo(gc, framebufferObj, GL_DEPTH_ATTACHMENT);
        if (formatInfo)
        {
            depthBits = formatInfo->depthSize;
        }
        return depthBits;
    }
    else if (bitType == GL_STENCIL_BITS)
    {
        formatInfo = __glGetFramebufferFormatInfo(gc, framebufferObj, GL_STENCIL_ATTACHMENT);
        if (formatInfo)
        {
            stencilBits = formatInfo->stencilSize;
        }
        return stencilBits;
    }

    GL_ASSERT(0); /*should not reach here*/
    return 0;
}

/*
** Fetch the data for a query in its internal type, then convert it to the
** type that the user asked for.
*/
__GL_INLINE GLvoid __glDoGet(__GLcontext *gc, GLenum sq, GLvoid *result, GLint type,
                             const GLchar *procName)
{
    GLint index;
    GLfloat   ftemp[100],   *fp   = ftemp;      /* NOTE: for floats */
    GLfloat   ctemp[100],   *cp   = ctemp;      /* NOTE: for colors */
    GLint     itemp[100],   *ip   = itemp;      /* NOTE: for ints */
    GLint64   i64temp[100], *i64p = i64temp;    /* NOTE: for int64 */
    GLboolean btemp[100],   *bp   = btemp;      /* NOTE: for boolean */

    switch (sq)
    {
    case GL_NUM_EXTENSIONS:
        *ip++ = gc->constants.numExtensions;
        break;
    case GL_MAJOR_VERSION:
        *ip++ = gc->constants.majorVersion;
        break;
    case GL_MINOR_VERSION:
        *ip++ = gc->constants.minorVersion;
        break;
    case GL_BLEND:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DITHER:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    case GL_POLYGON_OFFSET_FILL:
    case GL_SAMPLE_MASK:
    case GL_SAMPLE_SHADING_OES:
        *bp++ = __gles_IsEnabled(gc, sq);
        break;

    case GL_MAX_3D_TEXTURE_SIZE:
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
    case GL_MAX_TEXTURE_SIZE:
        *ip++ = gc->constants.maxTextureSize;
        break;
    case GL_SUBPIXEL_BITS:
        *ip++ = gc->constants.subpixelBits;
        break;
    case GL_LINE_WIDTH:
        *fp++ = gc->state.line.requestedWidth;
        break;
    case GL_ALIASED_LINE_WIDTH_RANGE:
        *fp++ = gc->constants.lineWidthMin;
        *fp++ = gc->constants.lineWidthMax;
        break;
    case GL_ALIASED_POINT_SIZE_RANGE:
        *fp++ = gc->constants.pointSizeMin;
        *fp++ = gc->constants.pointSizeMax;
        break;
    case GL_CULL_FACE_MODE:
        *ip++ = gc->state.polygon.cullFace;
        break;
    case GL_FRONT_FACE:
        *ip++ = gc->state.polygon.frontFace;
        break;
    case GL_DEPTH_RANGE:
        if (__GL_INT32 == type || __GL_INT64 == type)
        {
            *cp++ = (gc->state.depth.zNear);
            *cp++ = (gc->state.depth.zFar);
        }
        else
        {
            *fp++ = (gc->state.depth.zNear);
            *fp++ = (gc->state.depth.zFar);
        }
        break;
    case GL_DEPTH_WRITEMASK:
        *bp++ = gc->state.depth.writeEnable;
        break;
    case GL_DEPTH_CLEAR_VALUE:
        if (__GL_INT32 == type || __GL_INT64 == type)
        {
            *cp++ = gc->state.depth.clear;
        }
        else
        {
            *fp++ = gc->state.depth.clear;
        }
        break;
    case GL_DEPTH_FUNC:
        *ip++ = gc->state.depth.testFunc;
        break;
    case GL_STENCIL_CLEAR_VALUE:
        *ip++ = gc->state.stencil.clear;
        break;
    case GL_STENCIL_FUNC:
        *ip++ = gc->state.stencil.front.testFunc;
        break;
    case GL_STENCIL_BACK_FUNC:
        *ip++ = gc->state.stencil.back.testFunc;
        break;
    case GL_STENCIL_VALUE_MASK:
        *ip++ = gc->state.stencil.front.mask;
        break;
    case GL_STENCIL_BACK_VALUE_MASK:
        *ip++ = gc->state.stencil.back.mask;
        break;
    case GL_STENCIL_FAIL:
        *ip++ = gc->state.stencil.front.fail;
        break;
    case GL_STENCIL_BACK_FAIL:
        *ip++ = gc->state.stencil.back.fail;
        break;
    case GL_STENCIL_PASS_DEPTH_FAIL:
        *ip++ = gc->state.stencil.front.depthFail;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        *ip++ = gc->state.stencil.back.depthFail;
        break;
    case GL_STENCIL_PASS_DEPTH_PASS:
        *ip++ = gc->state.stencil.front.depthPass;
        break;
    case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        *ip++ = gc->state.stencil.back.depthPass;
        break;
    case GL_STENCIL_REF:
        *ip++ = gc->state.stencil.front.reference;
        break;
    case GL_STENCIL_BACK_REF:
        *ip++ = gc->state.stencil.back.reference;
        break;
    case GL_STENCIL_WRITEMASK:
        *ip++ = gc->state.stencil.front.writeMask;
        break;
    case GL_STENCIL_BACK_WRITEMASK:
        *ip++ = gc->state.stencil.back.writeMask;
        break;
    case GL_VIEWPORT:
        *ip++ = gc->state.viewport.x;
        *ip++ = gc->state.viewport.y;
        *ip++ = gc->state.viewport.width;
        *ip++ = gc->state.viewport.height;
        break;
    case GL_BLEND_DST_RGB:
        *ip++ = gc->state.raster.blendDstRGB[0];
        break;
    case GL_BLEND_DST_ALPHA:
        *ip++ = gc->state.raster.blendDstAlpha[0];
        break;
    case GL_BLEND_SRC_RGB:
        *ip++ = gc->state.raster.blendSrcRGB[0];
        break;
    case GL_BLEND_SRC_ALPHA:
        *ip++ = gc->state.raster.blendSrcAlpha[0];
        break;
    case GL_BLEND_EQUATION_RGB:
        *ip++ = gc->state.raster.blendEquationRGB[0];
        break;
    case GL_BLEND_EQUATION_ALPHA:
        *ip++ = gc->state.raster.blendEquationAlpha[0];
        break;
    case GL_BLEND_COLOR:
        {
            __GLcolor *blendColor = &gc->state.raster.blendColor;
            if (__GL_INT32 == type || __GL_INT64 == type)
            {
                *cp++ = blendColor->r;
                *cp++ = blendColor->g;
                *cp++ = blendColor->b;
                *cp++ = blendColor->a;
            }
            else
            {
                *fp++ = blendColor->r;
                *fp++ = blendColor->g;
                *fp++ = blendColor->b;
                *fp++ = blendColor->a;
            }
        }
        break;
    case GL_SCISSOR_BOX:
        *ip++ = gc->state.scissor.scissorX;
        *ip++ = gc->state.scissor.scissorY;
        *ip++ = gc->state.scissor.scissorWidth;
        *ip++ = gc->state.scissor.scissorHeight;
        break;
    case GL_COLOR_CLEAR_VALUE:
        {
            __GLcolor *clearColor = &gc->state.raster.clearColor.clear;
            if (__GL_INT32 == type || __GL_INT64 == type)
            {
                *cp++ = clearColor->r;
                *cp++ = clearColor->g;
                *cp++ = clearColor->b;
                *cp++ = clearColor->a;
            }
            else
            {
                *fp++ = clearColor->r;
                *fp++ = clearColor->g;
                *fp++ = clearColor->b;
                *fp++ = clearColor->a;
            }
        }
        break;
    case GL_COLOR_WRITEMASK:
        *bp++ = gc->state.raster.colorMask[0].redMask;
        *bp++ = gc->state.raster.colorMask[0].greenMask;
        *bp++ = gc->state.raster.colorMask[0].blueMask;
        *bp++ = gc->state.raster.colorMask[0].alphaMask;
        break;
    case GL_PACK_ROW_LENGTH:
        *ip++ = gc->clientState.pixel.packModes.lineLength;
        break;
    case GL_PACK_SKIP_ROWS:
        *ip++ = gc->clientState.pixel.packModes.skipLines;
        break;
    case GL_PACK_SKIP_PIXELS:
        *ip++ = gc->clientState.pixel.packModes.skipPixels;
        break;
    case GL_PACK_ALIGNMENT:
        *ip++ = gc->clientState.pixel.packModes.alignment;
        break;
    case GL_UNPACK_ROW_LENGTH:
        *ip++ = gc->clientState.pixel.unpackModes.lineLength;
        break;
    case GL_UNPACK_SKIP_ROWS:
        *ip++ = gc->clientState.pixel.unpackModes.skipLines;
        break;
    case GL_UNPACK_SKIP_PIXELS:
        *ip++ = gc->clientState.pixel.unpackModes.skipPixels;
        break;
    case GL_UNPACK_ALIGNMENT:
        *ip++ = gc->clientState.pixel.unpackModes.alignment;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        *ip++ = gc->clientState.pixel.unpackModes.skipImages;
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        *ip++ = gc->clientState.pixel.unpackModes.imageHeight;
        break;
    case GL_RED_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_RED_BITS) : gc->modes.redBits;
        break;
    case GL_GREEN_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_GREEN_BITS) : gc->modes.greenBits;
        break;
    case GL_BLUE_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_BLUE_BITS) : gc->modes.blueBits;
        break;
    case GL_ALPHA_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_ALPHA_BITS) : gc->modes.alphaBits;
        break;
    case GL_DEPTH_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_DEPTH_BITS) : gc->modes.depthBits;
        break;
    case GL_STENCIL_BITS:
        *ip++ = DRAW_FRAMEBUFFER_BINDING_NAME ? __glGetFboColorBits(gc, GL_STENCIL_BITS) : gc->modes.stencilBits;
        break;
    case GL_MAX_VIEWPORT_DIMS:
        *ip++ = gc->constants.maxViewportWidth;
        *ip++ = gc->constants.maxViewportHeight;
        break;
    case GL_POLYGON_OFFSET_FACTOR:
        *fp++ = gc->state.polygon.factor;
        break;
    case GL_POLYGON_OFFSET_UNITS:
        *fp++ = gc->state.polygon.units;
        break;
    case GL_TEXTURE_BINDING_2D:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_2D_ARRAY:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_3D:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_3D_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_CUBE_MAP:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_EXTERNAL_OES:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_EXTERNAL_INDEX]->name;
        break;
    case GL_TEXTURE_BINDING_CUBE_MAP_ARRAY_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX]->name;
        break;
    case GL_SAMPLER_BINDING:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundSampler
              ? gc->texture.units[index].boundSampler->name
              : 0;
        break;
    case GL_MAX_ELEMENTS_VERTICES:
        *ip++ = gc->constants.maxElementsVertices;
        break;
    case GL_MAX_ELEMENTS_INDICES:
        *ip++ = gc->constants.maxElementsIndices;
        break;
    case GL_MAX_ELEMENT_INDEX:
        *i64p++ = gc->constants.maxElementIndex;
        break;
    case GL_ACTIVE_TEXTURE:
        *ip++ = GL_TEXTURE0 + gc->state.texture.activeTexIndex;
        break;
    case GL_MAX_ARRAY_TEXTURE_LAYERS:
        *ip++ = gc->constants.maxTextureArraySize;
        break;
    case GL_MAX_TEXTURE_LOD_BIAS:
        *fp++ = (GLfloat)gc->constants.maxTextureLodBias;
        break;
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        *fp++ = (GLfloat)gc->constants.maxAnistropic;
        break;
    case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        *ip++ = gc->constants.numCompressedTextureFormats;
        break;
    case GL_COMPRESSED_TEXTURE_FORMATS:
        if (gc->constants.numCompressedTextureFormats > 0)
        {
            __GL_MEMCOPY(ip, gc->constants.pCompressedTexturesFormats, gc->constants.numCompressedTextureFormats * sizeof(GLint));
            ip += gc->constants.numCompressedTextureFormats;
        }
        break;
    case GL_SHADER_BINARY_FORMATS:
        __GL_MEMCOPY(ip, gc->constants.pShaderBinaryFormats, gc->constants.numShaderBinaryFormats * sizeof(GLint));
        ip += gc->constants.numShaderBinaryFormats;
        break;
    case GL_NUM_SHADER_BINARY_FORMATS:
        *ip++ = gc->constants.numShaderBinaryFormats;
        break;
    case GL_PROGRAM_BINARY_FORMATS:
        __GL_MEMCOPY(ip, gc->constants.pProgramBinaryFormats, gc->constants.numProgramBinaryFormats * sizeof(GLint));
        ip += gc->constants.numProgramBinaryFormats;
        break;
    case GL_NUM_PROGRAM_BINARY_FORMATS:
        *ip++ = gc->constants.numProgramBinaryFormats;
        break;
    case GL_SAMPLE_BUFFERS:
        if (gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj))
        {
            *ip++ = gc->frameBuffer.drawFramebufObj->fbSamples ? 1 : 0;
        }
        else
        {
            __GL_ERROR_RET(GL_INVALID_FRAMEBUFFER_OPERATION);
        }
        break;
    case GL_SAMPLES:
        if (gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj))
        {
            *ip++ = gc->frameBuffer.drawFramebufObj->fbSamples;
        }
        else
        {
            __GL_ERROR_RET(GL_INVALID_FRAMEBUFFER_OPERATION);
        }
        break;
    case GL_SAMPLE_COVERAGE_VALUE:
        *fp++ = gc->state.multisample.coverageValue;
        break;
    case GL_SAMPLE_COVERAGE_INVERT:
        *bp++ = gc->state.multisample.coverageInvert;
        break;
    case GL_MIN_SAMPLE_SHADING_VALUE_OES:
        *fp++ = gc->state.multisample.minSampleShadingValue;
        break;
    case GL_ARRAY_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_ARRAY_BUFFER_INDEX].boundBufName;
        break;
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        if (gc->vertexArray.boundVertexArray != 0)
        {
            *ip++ = gc->vertexArray.boundVAO->vertexArray.boundIdxName;
        }
        else
        {
            *ip++ = gc->bufferObject.generalBindingPoint[__GL_ELEMENT_ARRAY_BUFFER_INDEX].boundBufName;
        }
        break;
    case GL_CURRENT_PROGRAM:
        *ip++ = gc->shaderProgram.currentProgram
              ? gc->shaderProgram.currentProgram->objectInfo.id
              : 0;
        break;
    case GL_MAX_COLOR_ATTACHMENTS:
        *ip++ = __GL_MAX_COLOR_ATTACHMENTS;
        break;
    case GL_MAX_RENDERBUFFER_SIZE:
        *ip++ = gc->constants.maxRenderBufferSize;
        break;
    case GL_MAX_SAMPLES:
        *ip++ = gc->constants.maxSamples;
        break;
    case GL_DRAW_FRAMEBUFFER_BINDING:
        *ip++ = gc->frameBuffer.drawFramebufObj->name;
        break;
    case GL_IMPLEMENTATION_COLOR_READ_TYPE:
        {
            __GLformatInfo *formatInfo = gcvNULL;
            __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

            if (!gc->dp.isFramebufferComplete(gc, readFBO))
            {
                __GL_ERROR_RET(GL_INVALID_FRAMEBUFFER_OPERATION);
            }

            if (readFBO->name)
            {
                if (readFBO->readBuffer == GL_NONE)
                {
                    __GL_ERROR_RET(GL_INVALID_OPERATION);
                }

                formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
            }
            else
            {
                formatInfo = gc->readablePrivate->rtFormatInfo;
            }
            GL_ASSERT(formatInfo);

            *ip++ = formatInfo->dataType;
        }
        break;

    case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        {
            __GLformatInfo *formatInfo = gcvNULL;
            __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

            if (!gc->dp.isFramebufferComplete(gc, readFBO))
            {
                __GL_ERROR_RET(GL_INVALID_FRAMEBUFFER_OPERATION);
            }

            if (readFBO->name)
            {
                if (readFBO->readBuffer == GL_NONE)
                {
                    __GL_ERROR_RET(GL_INVALID_OPERATION);
                }

                formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
            }
            else
            {
                formatInfo = gc->readablePrivate->rtFormatInfo;
            }
            GL_ASSERT(formatInfo);

            *ip++ = formatInfo->dataFormat;
        }
        break;

    case GL_READ_FRAMEBUFFER_BINDING:
        *ip++ = gc->frameBuffer.readFramebufObj->name;
        break;
    case GL_RENDERBUFFER_BINDING:
        *ip++ = gc->frameBuffer.boundRenderbufObj->name;
        break;

    case GL_MIN_PROGRAM_TEXEL_OFFSET:
        *ip++ = gc->constants.shaderCaps.minProgramTexelOffset;
        break;
    case GL_MAX_PROGRAM_TEXEL_OFFSET:
        *ip++ = gc->constants.shaderCaps.maxProgramTexelOffset;
        break;

    case GL_MAX_DRAW_BUFFERS:
        *ip++ = gc->constants.shaderCaps.maxDrawBuffers;
        break;
    case GL_DRAW_BUFFER0:
    case GL_DRAW_BUFFER1:
    case GL_DRAW_BUFFER2:
    case GL_DRAW_BUFFER3:
    case GL_DRAW_BUFFER4:
    case GL_DRAW_BUFFER5:
    case GL_DRAW_BUFFER6:
    case GL_DRAW_BUFFER7:
    case GL_DRAW_BUFFER8:
    case GL_DRAW_BUFFER9:
    case GL_DRAW_BUFFER10:
    case GL_DRAW_BUFFER11:
    case GL_DRAW_BUFFER12:
    case GL_DRAW_BUFFER13:
    case GL_DRAW_BUFFER14:
    case GL_DRAW_BUFFER15:
        index = sq - GL_DRAW_BUFFER0;
        if (DRAW_FRAMEBUFFER_BINDING_NAME)
        {
            *ip++ = index < (GLint)gc->constants.shaderCaps.maxDrawBuffers
                  ? gc->frameBuffer.drawFramebufObj->drawBuffers[index]
                  : GL_NONE;
        }
        else
        {
            *ip++ = index < 1
                  ? gc->state.raster.drawBuffers[index]
                  : GL_NONE;
        }
        break;
    case GL_READ_BUFFER:
        *ip++ = READ_FRAMEBUFFER_BINDING_NAME
            ? gc->frameBuffer.readFramebufObj->readBuffer
            : gc->state.raster.readBuffer;
        break;

    case GL_SHADER_COMPILER:
        *bp++ = GL_TRUE;
        break;

    case GL_MAX_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.shaderCaps.maxFragTextureImageUnits;
        break;

    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.shaderCaps.maxVertTextureImageUnits;
        break;

    case GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.shaderCaps.maxCmptTextureImageUnits;
        break;

    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
        *ip++ = gc->constants.shaderCaps.maxCombinedTextureImageUnits;
        break;

    case GL_MAX_VERTEX_ATTRIBS:
        *ip++ = gc->constants.shaderCaps.maxUserVertAttributes;
        break;

    case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxVertOutVectors * 4;
        break;

    case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxFragInVectors * 4;
        break;

    case GL_MAX_VARYING_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxVaryingVectors * 4;
        break;

    case GL_MAX_VARYING_VECTORS:
        *ip++ = gc->constants.shaderCaps.maxVaryingVectors;
        break;

    case GL_MAX_VERTEX_UNIFORM_VECTORS:
        *ip++ = gc->constants.shaderCaps.maxVertUniformVectors;
        break;
    case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
        *ip++ = gc->constants.shaderCaps.maxFragUniformVectors;
        break;
    case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxVertUniformVectors * 4;
        break;
    case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxFragUniformVectors * 4;
        break;
    case GL_MAX_COMPUTE_UNIFORM_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxCmptUniformVectors * 4;
        break;

    case GL_MAX_VERTEX_UNIFORM_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxVertUniformBlocks;
        break;
    case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxFragUniformBlocks;
        break;
    case GL_MAX_COMPUTE_UNIFORM_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxCmptUniformBlocks;
        break;
    case GL_MAX_COMBINED_UNIFORM_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxCombinedUniformBlocks;
        break;
    case GL_MAX_UNIFORM_BUFFER_BINDINGS:
        *ip++ = gc->constants.shaderCaps.maxUniformBufferBindings;
        break;
    case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
        *ip++ = gc->constants.shaderCaps.uniformBufferOffsetAlignment;
        break;
    case GL_MAX_UNIFORM_BLOCK_SIZE:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxUniformBlockSize;
        break;
    case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxCombinedVertUniformComponents;
        break;
    case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxCombinedFragUniformComponents;
        break;
    case GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxCombinedCmptUniformComponents;
        break;

    case GL_MAX_UNIFORM_LOCATIONS:
        *ip++ = gc->constants.shaderCaps.maxUniformLocations;
        break;
    case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxXfbInterleavedComponents;
        break;
    case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
        *ip++ = gc->constants.shaderCaps.maxXfbSeparateComponents;
        break;
    case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
        *ip++ = gc->constants.shaderCaps.maxXfbSeparateAttribs;
        break;

    case GL_UNIFORM_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_UNIFORM_BUFFER_INDEX].boundBufName;
        break;

    case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        *ip++ = gc->xfb.boundXfbObj->boundBufName;
        break;

    case GL_TRANSFORM_FEEDBACK_PAUSED:
        *bp++ = gc->xfb.boundXfbObj->paused;
        break;

    case GL_TRANSFORM_FEEDBACK_ACTIVE:
        *bp++ = gc->xfb.boundXfbObj->active;
        break;

    case GL_TRANSFORM_FEEDBACK_BINDING:
        *ip++ = gc->xfb.boundXfbObj->name;
        break;

    case GL_COPY_READ_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_COPY_READ_BUFFER_INDEX].boundBufName;
        break;

    case GL_COPY_WRITE_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_COPY_WRITE_BUFFER_INDEX].boundBufName;
        break;

    case GL_VERTEX_ARRAY_BINDING:
        *ip++ = gc->vertexArray.boundVertexArray;
        break;

    case GL_GENERATE_MIPMAP_HINT:
        *ip++ = gc->state.hints.generateMipmap;
        break;

    case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
        *ip++ = gc->state.hints.fsDerivative;
        break;

    case GL_MAX_SERVER_WAIT_TIMEOUT:
        *i64p++ = (GLint64)gc->constants.maxServerWaitTimeout;
        break;

    case GL_PIXEL_PACK_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufName;
        break;

    case GL_PIXEL_UNPACK_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufName;
        break;

    case GL_DRAW_INDIRECT_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_DRAW_INDIRECT_BUFFER_INDEX].boundBufName;
        break;

    case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_DISPATCH_INDIRECT_BUFFER_INDEX].boundBufName;
        break;

    case GL_PRIMITIVE_RESTART_FIXED_INDEX:
        *bp++ = gc->state.enables.primitiveRestart;
        break;

    case GL_RASTERIZER_DISCARD:
        *bp++ = gc->state.enables.rasterizerDiscard;
        break;

    case GL_SAMPLE_ALPHA_TO_COVERAGE:
        *bp++ = gc->state.enables.multisample.alphaToCoverage;
        break;

    case GL_SAMPLE_COVERAGE:
        *bp++ = gc->state.enables.multisample.coverage;
        break;

    case GL_CONTEXT_ROBUST_ACCESS_EXT:
        *bp++ = (GLboolean)gc->imports.robustAccess;
        break;

    case GL_RESET_NOTIFICATION_STRATEGY_EXT:
        *ip++ = gc->imports.resetNotification;
        break;

    case GL_MAX_SAMPLE_MASK_WORDS:
        *ip++ = gc->constants.maxSampleMaskWords;
        break;

    case GL_MAX_COLOR_TEXTURE_SAMPLES:
    case GL_MAX_DEPTH_TEXTURE_SAMPLES:
        *ip++ = gc->constants.maxSamples;
        break;

    case GL_MAX_INTEGER_SAMPLES:
        *ip++ = gc->constants.maxSamplesInteger;
        break;

    case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_MS_INDEX]->name;
        break;

    case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY_OES:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_2D_MS_ARRAY_INDEX]->name;
        break;

    case GL_MAX_VERTEX_ATOMIC_COUNTERS:
    case GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS:
        *ip++ = gc->constants.shaderCaps.maxVertAtomicCounters;
        break;
    case GL_MAX_FRAGMENT_ATOMIC_COUNTERS:
    case GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS:
        *ip++ = gc->constants.shaderCaps.maxFragAtomicCounters;
        break;
    case GL_MAX_COMPUTE_ATOMIC_COUNTERS:
    case GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS:
        *ip++ = gc->constants.shaderCaps.maxCmptAtomicCounters;
        break;
    case GL_MAX_COMBINED_ATOMIC_COUNTERS:
    case GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS:
        *ip++ = gc->constants.shaderCaps.maxCombinedAtomicCounters;
        break;
    case GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS:
        *ip++ = gc->constants.shaderCaps.maxAtomicCounterBufferBindings;
        break;
    case GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxAtomicCounterBufferSize;
        break;
    case GL_ATOMIC_COUNTER_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_ATOMIC_COUNTER_BUFFER_INDEX].boundBufName;
        break;

    case GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT:
        *ip++ = gc->constants.shaderCaps.shaderStorageBufferOffsetAlignment;
        break;
    case GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxVertShaderStorageBlocks;
        break;
    case GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxFragShaderStorageBlocks;
        break;
    case GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxCmptShaderStorageBlocks;
        break;
    case GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS:
        *ip++ = gc->constants.shaderCaps.maxCombinedShaderStorageBlocks;
        break;
    case GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS:
        *ip++ = gc->constants.shaderCaps.maxShaderStorageBufferBindings;
        break;
    case GL_MAX_SHADER_STORAGE_BLOCK_SIZE:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxShaderBlockSize;
        break;
    case GL_SHADER_STORAGE_BUFFER_BINDING:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_SHADER_STORAGE_BUFFER_INDEX].boundBufName;
        break;
    case GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET:
        *ip++ = gc->constants.shaderCaps.minProgramTexGatherOffset;
        break;
    case GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET:
        *ip++ = gc->constants.shaderCaps.maxProgramTexGatherOffset;
        break;
    case GL_MAX_VERTEX_IMAGE_UNIFORMS:
        *ip++ = gc->constants.shaderCaps.maxVertexImageUniform;
        break;
    case GL_MAX_FRAGMENT_IMAGE_UNIFORMS:
        *ip++ = gc->constants.shaderCaps.maxFragImageUniform;
        break;
    case GL_MAX_COMPUTE_IMAGE_UNIFORMS:
        *ip++ = gc->constants.shaderCaps.maxCmptImageUniform;
        break;
    case GL_MAX_IMAGE_UNITS:
        *ip++ = gc->constants.shaderCaps.maxImageUnit;
        break;
    case GL_MAX_COMBINED_IMAGE_UNIFORMS:
        *ip++ = gc->constants.shaderCaps.maxCombinedImageUniform;
        break;
    case GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES:
        *ip++ = gc->constants.shaderCaps.maxCombinedShaderOutputResource;
        break;
    case GL_MAX_FRAMEBUFFER_WIDTH:
        *ip++ = gc->constants.maxTextureSize;
        break;
    case GL_MAX_FRAMEBUFFER_HEIGHT:
        *ip++ = gc->constants.maxTextureSize;
        break;
    case GL_MAX_FRAMEBUFFER_LAYERS_EXT:
        *ip++ = gc->constants.maxTextureDepthSize;
        break;
    case GL_MAX_FRAMEBUFFER_SAMPLES:
        *ip++ = gc->constants.maxSamples;
        break;
    case GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS:
        *ip++ = gc->constants.shaderCaps.maxWorkGroupInvocation;
        break;
    case GL_MAX_COMPUTE_SHARED_MEMORY_SIZE:
        *ip++ = gc->constants.shaderCaps.maxShareMemorySize;
        break;

    case GL_MAX_VERTEX_ATTRIB_BINDINGS:
        *ip++ = gc->constants.maxVertexAttribBindings;
        break;
    case GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET:
        *ip++ = gc->constants.maxVertexAttribRelativeOffset;
        break;
    case GL_MAX_VERTEX_ATTRIB_STRIDE:
        *ip++ = gc->constants.maxVertexAttribStride;
        break;

    case GL_PROGRAM_PIPELINE_BINDING:
        *ip++ = gc->shaderProgram.boundPPO ? gc->shaderProgram.boundPPO->name : 0;
        break;

    case GL_MAX_DEBUG_MESSAGE_LENGTH_KHR:
        *ip++ = gc->debug.maxMsgLen;
        break;
    case GL_MAX_DEBUG_LOGGED_MESSAGES_KHR:
        *ip++ = gc->debug.maxLogMsgs;
        break;
    case GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR:
        *ip++ = gc->debug.maxStackDepth;
        break;
    case GL_MAX_LABEL_LENGTH_KHR:
        *ip++ = gc->debug.maxMsgLen;
        break;
    case GL_DEBUG_LOGGED_MESSAGES_KHR:
        *ip++ = gc->debug.loggedMsgs;
        break;
    case GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR:
        *ip++ = gc->debug.msgLogHead ? gc->debug.msgLogHead->length : 0;
        break;
    case GL_DEBUG_GROUP_STACK_DEPTH_KHR:
        *ip++ = gc->debug.current + 1;
        break;
    case GL_DEBUG_OUTPUT_KHR:
        *bp++ = gc->debug.dbgOut;
        break;
    case GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR:
        *bp++ = gc->debug.dbgOutSync;
        break;

    /* TS */
    case GL_MAX_TESS_GEN_LEVEL_EXT:
        *ip++ = gc->constants.shaderCaps.maxTessGenLevel;
        break;
    case GL_MAX_PATCH_VERTICES_EXT:
        *ip++ = gc->constants.shaderCaps.maxTessPatchVertices;
        break;

    case GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsUniformVectors * 4;
        break;
    case GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsUniformBlocks;
        break;
    case GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS_EXT:
        *i64p++ = gc->constants.shaderCaps.maxCombinedTcsUniformComponents;
        break;
    case GL_MAX_TESS_CONTROL_INPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsInVectors * 4;
        break;
    case GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsOutVectors * 4;
        break;
    case GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsOutTotalVectors * 4;
        break;
    case GL_MAX_TESS_PATCH_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsOutPatchVectors * 4;
        break;
    case GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS_EXT:
    case GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsAtomicCounters;
        break;
    case GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsImageUniform;
        break;
    case GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsShaderStorageBlocks;
        break;
    case GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTcsTextureImageUnits;
        break;

    case GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesUniformVectors * 4;
        break;
    case GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesUniformBlocks;
        break;
    case GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS_EXT:
        *i64p++ = gc->constants.shaderCaps.maxCombinedTesUniformComponents;
        break;
    case GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesInVectors * 4;
        break;
    case GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesOutVectors * 4;
        break;
    case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS_EXT:
    case GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesAtomicCounters;
        break;
    case GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesImageUniform;
        break;
    case GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesShaderStorageBlocks;
        break;
    case GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS_EXT:
        *ip++ = gc->constants.shaderCaps.maxTesTextureImageUnits;
        break;

    case GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED:
        *bp++ = (GLboolean)gc->constants.shaderCaps.tessPatchPR;
        break;

    case GL_PATCH_VERTICES_EXT:
        *ip++ = gc->shaderProgram.patchVertices;
        break;

    /* GS only */
    case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsUniformVectors * 4;
        break;
    case GL_MAX_GEOMETRY_UNIFORM_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsUniformBlocks;
        break;
    case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS_EXT:
        *i64p++ = (GLint64)gc->constants.shaderCaps.maxCombinedGsUniformComponents;
        break;
    case GL_MAX_GEOMETRY_INPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsInVectors * 4;
        break;
    case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsOutVectors * 4;
        break;
    case GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsOutVertices;
        break;
    case GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsOutTotalVectors * 4;
        break;
    case GL_MAX_GEOMETRY_SHADER_INVOCATIONS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsInvocationCount;
        break;
    case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsTextureImageUnits;
        break;
    case GL_MAX_GEOMETRY_ATOMIC_COUNTERS_EXT:
    case GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsAtomicCounters;
        break;
    case GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsImageUniform;
        break;
    case GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT:
        *ip++ = gc->constants.shaderCaps.maxGsShaderStorageBlocks;
        break;

    case GL_LAYER_PROVOKING_VERTEX_EXT:
        *ip++ = gc->constants.gsLayerProvokingVertex;
        break;

    case GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES:
        *fp++ = gc->constants.minFragmentInterpolationOffset;
        break;

    case GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES:
        *fp++ = gc->constants.maxFragmentInterpolationOffset;
        break;

    case GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES:
        *ip++ = gc->constants.fragmentInterpolationOffsetBits;
        break;

    case GL_PRIMITIVE_BOUNDING_BOX:
        *fp++ = gc->state.primBound.minX;
        *fp++ = gc->state.primBound.minY;
        *fp++ = gc->state.primBound.minZ;
        *fp++ = gc->state.primBound.minW;
        *fp++ = gc->state.primBound.maxX;
        *fp++ = gc->state.primBound.maxY;
        *fp++ = gc->state.primBound.maxZ;
        *fp++ = gc->state.primBound.maxW;
        break;

    case GL_TEXTURE_BUFFER_BINDING_EXT:
        *ip++ = gc->bufferObject.generalBindingPoint[__GL_TEXTURE_BUFFER_BINDING_EXT].boundBufName;
        break;

    case GL_MAX_TEXTURE_BUFFER_SIZE_EXT:
        *ip++ = gc->constants.maxTextureBufferSize;
        break;

    case GL_TEXTURE_BINDING_BUFFER_EXT:
        index = gc->state.texture.activeTexIndex;
        *ip++ = gc->texture.units[index].boundTextures[__GL_TEXTURE_BINDING_BUFFER_EXT]->name;
        break;

    case GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_EXT:
        *ip++ = gc->constants.textureBufferOffsetAlignment;
        break;

    case GL_CONTEXT_FLAGS:
        *ip++ = gc->contextFlags;
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    /* Use the motion of the pointers to type convert the result */
    if (ip != itemp)
    {
        __glConvertResult(gc, __GL_INT32, itemp, type, result, (GLint)(ip - itemp));
    }
    else if (i64p != i64temp)
    {
        __glConvertResult(gc, __GL_INT64, i64temp, type, result, (GLint)(i64p - i64temp));
    }
    else if (fp != ftemp)
    {
        __glConvertResult(gc, __GL_FLOAT, ftemp, type, result, (GLint)(fp - ftemp));
    }
    else if (bp != btemp)
    {
        __glConvertResult(gc, __GL_BOOLEAN, btemp, type, result, (GLint)(bp - btemp));
    }
    else if (cp != ctemp)
    {
        __glConvertResult(gc, __GL_COLOR, ctemp, type, result, (GLint)(cp - ctemp));
    }
}

GLvoid GL_APIENTRY __gles_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params)
{
    __GL_HEADER();

    __glDoGet(gc, pname, params, __GL_FLOAT32, "glGetFloatv");

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params)
{
    __GL_HEADER();

    __glDoGet(gc, pname, params, __GL_INT32, "glGetIntegerv");

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params)
{
    __GL_HEADER();

    __glDoGet(gc, pname, params, __GL_BOOLEAN, "glGetBooleanv");

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params)
{
    __GL_HEADER();

    __glDoGet(gc, pname, params, __GL_INT64, "glGetInteger64v");

    __GL_FOOTER();
}

__GL_INLINE GLvoid __glDoIndexedGet(__GLcontext *gc, GLenum target, GLint type, GLuint index, GLvoid* data)
{
    GLuint targetIdx;
    __GLBufBindPoint *bufBinding;
    __GLvertexAttribBinding *attribBinding;
    GLint     itemp[100],   *ip   = itemp;      /* NOTE: for ints */
    GLint64   i64temp[100], *i64p = i64temp;    /* NOTE: for int64 */
    GLboolean btemp[100],   *bp   = btemp;      /* NOTE: for boolean */

    if (!data)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    switch (target)
    {
    case GL_COLOR_WRITEMASK:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.raster.colorMask[index].redMask;
        *ip++ = gc->state.raster.colorMask[index].greenMask;
        *ip++ = gc->state.raster.colorMask[index].blueMask;
        *ip++ = gc->state.raster.colorMask[index].alphaMask;
        break;

    case GL_BLEND:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.enables.colorBuffer.blend[index];
        break;

    case GL_BLEND_EQUATION_RGB:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.raster.blendEquationRGB[index];
        break;

    case GL_BLEND_EQUATION_ALPHA:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.raster.blendEquationAlpha[index];
        break;

    case GL_BLEND_SRC_RGB:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.raster.blendSrcRGB[index];
        break;

    case GL_BLEND_SRC_ALPHA:
        if (index >= gc->constants.shaderCaps.maxDrawBuffers)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.raster.blendSrcAlpha[index];
        break;

     case GL_BLEND_DST_RGB:
         if (index >= gc->constants.shaderCaps.maxDrawBuffers)
         {
             __GL_ERROR_RET(GL_INVALID_VALUE);
         }
         *ip++ = gc->state.raster.blendDstRGB[index];
         break;

     case GL_BLEND_DST_ALPHA:
         if (index >= gc->constants.shaderCaps.maxDrawBuffers)
         {
             __GL_ERROR_RET(GL_INVALID_VALUE);
         }
         *ip++ = gc->state.raster.blendDstAlpha[index];
         break;

    case GL_SAMPLE_MASK_VALUE:
        if (index >= gc->constants.maxSampleMaskWords)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.multisample.sampleMaskValue;
        break;

    case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        targetIdx = __GL_XFB_BUFFER_INDEX;
        if (index >= gc->constants.shaderCaps.maxXfbSeparateAttribs)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->xfb.boundXfbObj->boundBufBinding[index];
        *ip++ = bufBinding->boundBufName;
        break;
    case GL_TRANSFORM_FEEDBACK_BUFFER_START:
        targetIdx = __GL_XFB_BUFFER_INDEX;
        if (index >= gc->constants.shaderCaps.maxXfbSeparateAttribs)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->xfb.boundXfbObj->boundBufBinding[index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufOffset : 0;
        break;
    case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
        targetIdx = __GL_XFB_BUFFER_INDEX;
        if (index >= gc->constants.shaderCaps.maxXfbSeparateAttribs)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->xfb.boundXfbObj->boundBufBinding[index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufSize : 0;
        break;

    case GL_UNIFORM_BUFFER_BINDING:
        targetIdx = __GL_UNIFORM_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *ip++ = bufBinding->boundBufName;
        break;
    case GL_UNIFORM_BUFFER_START:
        targetIdx = __GL_UNIFORM_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufOffset : 0;
        break;
    case GL_UNIFORM_BUFFER_SIZE:
        targetIdx = __GL_UNIFORM_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufSize : 0;
        break;

    case GL_ATOMIC_COUNTER_BUFFER_BINDING:
        targetIdx = __GL_ATOMIC_COUNTER_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *ip++ = bufBinding->boundBufName;
        break;
    case GL_ATOMIC_COUNTER_BUFFER_START:
        targetIdx = __GL_ATOMIC_COUNTER_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufOffset : 0;
        break;
    case GL_ATOMIC_COUNTER_BUFFER_SIZE:
        targetIdx = __GL_ATOMIC_COUNTER_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufSize : 0;
        break;

    case GL_SHADER_STORAGE_BUFFER_BINDING:
        targetIdx = __GL_SHADER_STORAGE_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *ip++ = bufBinding->boundBufName;
        break;
    case GL_SHADER_STORAGE_BUFFER_START:
        targetIdx = __GL_SHADER_STORAGE_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufOffset : 0;
        break;
    case GL_SHADER_STORAGE_BUFFER_SIZE:
        targetIdx = __GL_SHADER_STORAGE_BUFFER_INDEX;
        if (index >= gc->bufferObject.maxBufBindings[targetIdx])
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        bufBinding = &gc->bufferObject.bindingPoints[targetIdx][index];
        *i64p++ = bufBinding->boundBufName ? bufBinding->bufSize : 0;
        break;

    case GL_VERTEX_BINDING_BUFFER:
        if (index >= gc->constants.maxVertexAttribBindings)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        attribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[index];
        *ip++ = attribBinding->boundArrayName;
        break;
    case GL_VERTEX_BINDING_STRIDE:
        if (index >= gc->constants.maxVertexAttribBindings)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        attribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[index];
        *ip++ = attribBinding->stride;
        break;
    case GL_VERTEX_BINDING_DIVISOR:
        if (index >= gc->constants.maxVertexAttribBindings)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        attribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[index];
        *ip++ = attribBinding->divisor;
        break;
    case GL_VERTEX_BINDING_OFFSET:
        if (index >= gc->constants.maxVertexAttribBindings)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        attribBinding = &gc->vertexArray.boundVAO->vertexArray.attributeBinding[index];
        *i64p++ = attribBinding->offset;
        break;

    case GL_IMAGE_BINDING_NAME:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.image.imageUnit[index].texObj ? gc->state.image.imageUnit[index].texObj->name : 0;
        break;
    case GL_IMAGE_BINDING_LEVEL:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.image.imageUnit[index].level;
        break;
    case GL_IMAGE_BINDING_LAYER:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.image.imageUnit[index].requestLayer;
        break;
    case GL_IMAGE_BINDING_LAYERED:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *bp++ = gc->state.image.imageUnit[index].layered;
        break;
    case GL_IMAGE_BINDING_ACCESS:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.image.imageUnit[index].access;
        break;
    case GL_IMAGE_BINDING_FORMAT:
        if (index >= gc->constants.shaderCaps.maxImageUnit)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->state.image.imageUnit[index].format;
        break;
    case GL_MAX_COMPUTE_WORK_GROUP_COUNT:
        if (index >= 3)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->constants.shaderCaps.maxWorkGroupCount[index];
        break;
    case GL_MAX_COMPUTE_WORK_GROUP_SIZE:
        if (index >= 3)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        *ip++ = gc->constants.shaderCaps.maxWorkGroupSize[index];
        break;

    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    /* Use the motion of the pointers to type convert the result */
    if (ip != itemp)
    {
        __glConvertResult(gc, __GL_INT32, itemp, type, data, (GLint)(ip - itemp));
    }
    else if (i64p != i64temp)
    {
        __glConvertResult(gc, __GL_INT64, i64temp, type, data, (GLint)(i64p - i64temp));
    }
    else if (bp != btemp)
    {
        __glConvertResult(gc, __GL_BOOLEAN, btemp, type, data, (GLint)(bp - btemp));
    }
}

GLvoid GL_APIENTRY __gles_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data)
{
    __GL_HEADER();

    __glDoIndexedGet(gc, target, __GL_INT32, index, data);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data)
{
    __GL_HEADER();

    __glDoIndexedGet(gc, target, __GL_INT64, index, data);

    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data)
{
    __GL_HEADER();

    __glDoIndexedGet(gc, target, __GL_BOOLEAN, index, data);

    __GL_FOOTER();
}

/*
** Return the current error code.
*/
GLenum GL_APIENTRY __gles_GetError(__GLcontext *gc)
{
    GLint error = gc->error;

    gc->error = 0;
    return error;
}

/*
** Return a pointer to the requested string
*/
const GLubyte * GL_APIENTRY __gles_GetString(__GLcontext *gc, GLenum name)
{
    switch (name)
    {
    case GL_VENDOR:
        return (GLubyte*)gc->constants.vendor;
    case GL_RENDERER:
        return (GLubyte*)gc->constants.renderer;
    case GL_VERSION:
        return (GLubyte*)gc->constants.version;
    case GL_EXTENSIONS:
        return(GLubyte*)gc->constants.extensions;
    case GL_SHADING_LANGUAGE_VERSION:
        return (GLubyte*)gc->constants.GLSLVersion;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, gcvNULL);
    }
}

const GLubyte* GL_APIENTRY __gles_GetStringi(__GLcontext *gc, GLenum name, GLuint index)
{
    __GLextension *curExt;
    GLuint num = 0;

    switch (name)
    {
    case GL_EXTENSIONS:
        if (index >= gc->constants.numExtensions)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, gcvNULL);
        }
        break;
    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, gcvNULL);
    }

    /* Go through the extension table again to construct the extension string */
    for (curExt = __glExtension; curExt->name; curExt++)
    {
        if (curExt->bEnabled)
        {
            if (num++ == index)
            {
                break;
            }
        }
    }

    return (const GLubyte*)curExt->name;
}

static GLvoid __glEndQuery(__GLcontext *gc, GLuint targetIndex)
{
    __GLqueryObject *queryObj;
    GLboolean retVal;

    /* If there is no active query */
    if (gc->query.currQuery[targetIndex] == gcvNULL)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    queryObj = gc->query.currQuery[targetIndex];
    if (!queryObj->active)
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    /* Notify DP to end query on the query object */
    retVal = (*gc->dp.endQuery)(gc, queryObj);

    if (!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    queryObj->active = GL_FALSE;
    gc->query.currQuery[targetIndex] = gcvNULL;

    if (queryObj->flag & __GL_OBJECT_IS_DELETED)
    {
        __glDeleteQueryObj(gc, queryObj);
    }
}

GLvoid GL_APIENTRY __gles_GenQueries(__GLcontext *gc, GLsizei n, GLuint *ids)
{
    GLint start, i;

    __GL_HEADER();

    if (gcvNULL == ids)
    {
        __GL_EXIT();
    }

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED] ||
        gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE] ||
        gc->query.currQuery[__GL_QUERY_XFB_PRIMITIVES_WRITTEN])
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    start = __glGenerateNames(gc, gc->query.noShare, n);

    for (i = 0; i < n; i++)
    {
        ids[i] = start + i;
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint *ids)
{
    GLint i;

    __GL_HEADER();

    if (n < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    for (i = 0; i < n; i++)
    {
        __glDeleteObject(gc, gc->query.noShare, ids[i]);
    }

OnError:
    __GL_FOOTER();
}

GLboolean GL_APIENTRY __gles_IsQuery(__GLcontext *gc, GLuint id)
{
    return (gcvNULL != __glGetObject(gc, gc->query.noShare, id));
}

GLvoid GL_APIENTRY __gles_BeginQuery(__GLcontext *gc, GLenum target, GLuint id)
{
    __GLqueryObject *queryObj;
    GLuint targetIndex, queryIdx;
    GLboolean retVal;

    __GL_HEADER();

    if (id == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (target)
    {
    case GL_ANY_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED;
        break;
    case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE;
        break;
    case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
        targetIndex = __GL_QUERY_XFB_PRIMITIVES_WRITTEN;
        break;
    case GL_PRIMITIVES_GENERATED_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            targetIndex = __GL_QUERY_PRIMITIVES_GENERATED;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /*
    ** Another query is already in progress with the same target.
    ** note: GL_ANY_SAMPLES_PASSED and GL_ANY_SAMPLES_PASSED_CONSERVATIVE alias to the same target for this purposes.
    */
    if (gc->query.currQuery[targetIndex] ||
        (targetIndex == __GL_QUERY_ANY_SAMPLES_PASSED && gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE]) ||
        (targetIndex == __GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE && gc->query.currQuery[__GL_QUERY_ANY_SAMPLES_PASSED])
       )
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* Where id is the name of a query currently in progress */
    for (queryIdx = __GL_QUERY_ANY_SAMPLES_PASSED; queryIdx < __GL_QUERY_LAST; ++queryIdx)
    {
        if (gc->query.currQuery[queryIdx] &&
            !(gc->query.currQuery[queryIdx]->flag & __GL_OBJECT_IS_DELETED) &&
            gc->query.currQuery[queryIdx]->name == id)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    if (!__glIsNameDefined(gc, gc->query.noShare, id))
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    queryObj = (__GLqueryObject *)__glGetObject(gc, gc->query.noShare, id);

    if (queryObj == gcvNULL)
    {
        /*
        ** If this is the first time this name has been bound,
        ** then create a new texture object and initialize it.
        */
        queryObj = (__GLqueryObject *)(*gc->imports.calloc)(gc, 1, sizeof(__GLqueryObject));
        if (queryObj == gcvNULL)
        {
            __GL_ERROR_EXIT(GL_OUT_OF_MEMORY);
        }

        queryObj->name = id;

        /* Add this __GLoccluQueryObject to the "gc->occluQuery.noShare" structure. */
        __glAddObject(gc, gc->query.noShare, id, queryObj);
    }

    /* If id refers to an existing query object whose type does not match target. */
    if (queryObj->target != 0 && queryObj->target != target)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    queryObj->target = target;
    queryObj->count = 0;
    queryObj->resultAvailable = GL_FALSE;
    queryObj->active = GL_TRUE;
    gc->query.currQuery[targetIndex] = queryObj;

    /* Notify DP to begin query on the query object */
    retVal = (*gc->dp.beginQuery)(gc, queryObj);

    if (!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_EndQuery(__GLcontext *gc, GLenum target)
{
    GLuint targetIndex;

    __GL_HEADER();

    switch (target)
    {
    case GL_ANY_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED;
        break;
    case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE;
        break;
    case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
        targetIndex = __GL_QUERY_XFB_PRIMITIVES_WRITTEN;
        break;
    case GL_PRIMITIVES_GENERATED_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            targetIndex = __GL_QUERY_PRIMITIVES_GENERATED;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    __glEndQuery(gc, targetIndex);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GLqueryObject *queryObj;
    GLuint targetIndex ;

    __GL_HEADER();

    switch (target)
    {
    case GL_ANY_SAMPLES_PASSED:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED;
        break;
    case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
        targetIndex = __GL_QUERY_ANY_SAMPLES_PASSED_CONSERVATIVE;
        break;
    case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
        targetIndex = __GL_QUERY_XFB_PRIMITIVES_WRITTEN;
        break;
    case GL_PRIMITIVES_GENERATED_EXT:
        if (__glExtension[__GL_EXTID_EXT_geometry_shader].bEnabled)
        {
            targetIndex = __GL_QUERY_PRIMITIVES_GENERATED;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    queryObj = gc->query.currQuery[targetIndex];

    switch (pname)
    {
    case GL_CURRENT_QUERY:
        if (queryObj && queryObj->active)
        {
            *params = queryObj->name;
        }
        else
        {
            *params = 0;
        }
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

__GL_INLINE GLboolean __glGetQueryObjectiv(__GLcontext *gc, GLuint id, GLenum pname, GLint64* params)
{
    __GLqueryObject *queryObj;

    GL_ASSERT(gc->query.noShare);
    queryObj = (__GLqueryObject *)__glGetObject(gc, gc->query.noShare, id);
    if (!queryObj || queryObj->active)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    switch (pname)
    {
    case GL_QUERY_RESULT:
        while (!queryObj->resultAvailable)
        {
            /* Query DP for the results of the query object */
            (*gc->dp.getQueryObject)(gc, pname, queryObj);
        }

        if ((queryObj->target == GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN) ||
            (queryObj->target == GL_PRIMITIVES_GENERATED_EXT))
        {
            *params = queryObj->count;
        }
        else
        {
            /* For occlusion query, the result is a boolean value. */
            *params = queryObj->count ? 1 : 0;
        }
        break;

    case GL_QUERY_RESULT_AVAILABLE:
        if (!queryObj->resultAvailable)
        {
            /* Query DP for the results of the query object */
            (*gc->dp.getQueryObject)(gc, pname, queryObj);
        }
        *params = queryObj->resultAvailable;
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}

GLvoid GL_APIENTRY __gles_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint *params)
{
    GLint64 result = 0;

    __GL_HEADER();

    switch (pname)
    {
    case GL_QUERY_RESULT:
    case GL_QUERY_RESULT_AVAILABLE:
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (__glGetQueryObjectiv(gc, id, pname, &result) == GL_FALSE)
    {
        __GL_EXIT();
    }

    /* Make sure result can convert from INT64 to UINT safely */
    GL_ASSERT((result >= 0) && (result <= __glMaxUint));

    *params = (GLuint)result;

OnExit:
OnError:
    __GL_FOOTER();
}

GLboolean __glDeleteQueryObj(__GLcontext *gc, __GLqueryObject *queryObj)
{
    if (queryObj->active)
    {
        /* Set the flag to indicate the object is marked for delete */
        queryObj->flag |= __GL_OBJECT_IS_DELETED;
        return GL_FALSE;
    }

    if (queryObj->label)
    {
        gc->imports.free(gc, queryObj->label);
    }

    (*gc->dp.deleteQuery)(gc, queryObj);

    (*gc->imports.free)(gc, queryObj);

    return GL_TRUE;
}

GLvoid __glInitQueryState(__GLcontext *gc)
{
    /* Query object cannot be shared between contexts */
    if (gc->query.noShare == gcvNULL)
    {
        gc->query.noShare = (__GLsharedObjectMachine *)
            (*gc->imports.calloc)(gc, 1, sizeof(__GLsharedObjectMachine));

        /* Initialize a linear lookup table for query object */
        gc->query.noShare->maxLinearTableSize = __GL_MAX_QUERYOBJ_LINEAR_TABLE_SIZE;
        gc->query.noShare->linearTableSize = __GL_DEFAULT_QUERYOBJ_LINEAR_TABLE_SIZE;
        gc->query.noShare->linearTable = (GLvoid **)
            (*gc->imports.calloc)(gc, 1, gc->query.noShare->linearTableSize * sizeof(GLvoid *));

        gc->query.noShare->hashSize = __GL_QUERY_HASH_TABLE_SIZE;
        gc->query.noShare->hashMask = __GL_QUERY_HASH_TABLE_SIZE - 1;
        gc->query.noShare->refcount = 1;
        gc->query.noShare->deleteObject = (__GLdeleteObjectFunc)__glDeleteQueryObj;
        gc->query.noShare->immediateInvalid = GL_TRUE;
    }
}

GLvoid __glFreeQueryState(__GLcontext *gc)
{
    GLuint targetIdx;
    for (targetIdx = 0; targetIdx < __GL_QUERY_LAST; ++targetIdx)
    {
        if (gc->query.currQuery[targetIdx])
        {
            __glEndQuery(gc, targetIdx);
        }
    }

    /* Free query object table */
    __glFreeSharedObjectState(gc, gc->query.noShare);
}

GLenum GL_APIENTRY __gles_GetGraphicsResetStatus(__GLcontext *gc)
{
    return (*gc->dp.getGraphicsResetStatus)(gc);
}
