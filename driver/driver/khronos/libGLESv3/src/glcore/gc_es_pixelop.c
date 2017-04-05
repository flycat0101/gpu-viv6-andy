/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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

#define _GC_OBJ_ZONE __GLES3_ZONE_CORE


extern GLboolean __glCheckPBO(__GLcontext *gc,
                              __GLpixelPackMode *packMode,
                              __GLbufferObject *bufObj,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              GLenum format,
                              GLenum type,
                              const GLvoid *buf);

extern  GLuint __glPixelSize(__GLcontext *gc, GLenum format, GLenum type);

GLvoid __glInitPixelState(__GLcontext *gc)
{
    __GLclientPixelState *cps = &gc->clientState.pixel;

    cps->packModes.alignment = 4;
    cps->packModes.lineLength = 0;
    cps->packModes.imageHeight = 0;
    cps->packModes.skipPixels = 0;
    cps->packModes.skipLines = 0;
    cps->packModes.skipImages = 0;

    cps->unpackModes.alignment = 4;
    cps->unpackModes.lineLength = 0;
    cps->unpackModes.imageHeight = 0;
    cps->unpackModes.skipPixels = 0;
    cps->unpackModes.skipLines = 0;
    cps->unpackModes.skipImages = 0;
}

GLboolean __glCheckUnpackArgs(__GLcontext *gc, GLenum format, GLenum type)
{
    switch (format)
    {
    case GL_RGBA:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGB:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RG:
    case GL_RED:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_HALF_FLOAT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGBA_INTEGER:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_RGB_INTEGER:
    case GL_RG_INTEGER:
    case GL_RED_INTEGER:
        switch (type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_DEPTH_STENCIL:
        switch (type)
        {
        case GL_UNSIGNED_INT_24_8:
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}

GLvoid GL_APIENTRY __gles_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum *bufs)
{
    GLuint i;
    GLboolean changed = GL_FALSE;
    GLenum *pDrawBuffers = gcvNULL;

    __GL_HEADER();

    if (DRAW_FRAMEBUFFER_BINDING_NAME)
    {
        __GLframebufferObject *drawFBO = gc->frameBuffer.drawFramebufObj;

        if ((gcvNULL == bufs) && !(gc->constants.majorVersion == 3 && gc->constants.minorVersion >= 1))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if (n > (GLsizei)gc->constants.shaderCaps.maxDrawBuffers || n < 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }

        for (i = 0; i < (GLuint)n; ++i)
        {
            /* An INVALID_ENUM error is generated if any value in bufs is not one of the
            ** values in tables 15.3, BACK, or NONE
            */
            if (bufs[i] != GL_NONE && bufs[i] != GL_BACK &&
                ((bufs[i] < GL_COLOR_ATTACHMENT0) || (bufs[i] > GL_COLOR_ATTACHMENT0 + gc->constants.shaderCaps.maxDrawBuffers)))
            {
                __GL_ERROR_EXIT(GL_INVALID_ENUM);
            }

            if ((bufs[i] != GL_NONE) && (bufs[i] != GL_COLOR_ATTACHMENT0 + i))
            {
                __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            }
        }

        drawFBO->drawBufferCount = n;
        pDrawBuffers = drawFBO->drawBuffers;
    }
    else
    {
        if (gcvNULL == bufs)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        /* Error check */
        if (n != 1 || !(GL_NONE == *bufs || GL_BACK == *bufs))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        pDrawBuffers = gc->state.raster.drawBuffers;
    }

    /* Redundancy check */
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        GLenum buf = (i < (GLuint)n) ? bufs[i] : GL_NONE;

        if (pDrawBuffers[i] != buf)
        {
            pDrawBuffers[i] = buf;
            changed = GL_TRUE;
        }
    }

    if (changed)
    {
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;

        if (DRAW_FRAMEBUFFER_BINDING_NAME)
        {
            /* __GL_FRAMEBUFFER_DRAWBUFFER_DIRTY */
            __GL_FRAMEBUFFER_COMPLETE_DIRTY(gc->frameBuffer.drawFramebufObj);
        }
    }

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ReadBuffer(__GLcontext *gc, GLenum mode)
{
    __GL_HEADER();

    if ((mode != GL_NONE) && (mode != GL_BACK) &&
        (mode < GL_COLOR_ATTACHMENT0 || mode > __GL_COLOR_ATTACHMENT31))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (READ_FRAMEBUFFER_BINDING_NAME)
    {
        if ((mode == GL_BACK) || (mode > __GL_COLOR_ATTACHMENTn))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        /* If a FBO is bound to gc, set the states in FBO instead of gc. */
        if (mode != gc->frameBuffer.readFramebufObj->readBuffer)
        {
            gc->frameBuffer.readFramebufObj->readBuffer = mode;
            gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;

            __GL_FRAMEBUFFER_COMPLETE_DIRTY(gc->frameBuffer.readFramebufObj);
        }
    }
    else
    {
        if (mode != GL_NONE && mode != GL_BACK)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        /* If gc binds to default FBO. */
        if (mode != gc->state.raster.readBuffer)
        {
            gc->state.raster.readBuffer = mode;
            gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
        }
    }

OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __gles_PixelStorei(__GLcontext *gc, GLenum pname, GLint param)
{
    __GLclientPixelState *ps = &gc->clientState.pixel;

    __GL_HEADER();

    /* All pname require non-negative param */
    if (param < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    switch (pname)
    {
    case GL_PACK_ALIGNMENT:
        switch (param)
        {
        case 1: case 2: case 4: case 8:
            ps->packModes.alignment = param;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
    case GL_UNPACK_ALIGNMENT:
        switch (param)
        {
        case 1: case 2: case 4: case 8:
            ps->unpackModes.alignment = param;
            break;
        default:
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
    case GL_PACK_ROW_LENGTH:
        ps->packModes.lineLength = param;
        break;
    case GL_PACK_SKIP_ROWS:
        ps->packModes.skipLines = param;
        break;
    case GL_PACK_SKIP_PIXELS:
        ps->packModes.skipPixels = param;
        break;
    case GL_UNPACK_ROW_LENGTH:
        ps->unpackModes.lineLength = param;
        break;
    case GL_UNPACK_SKIP_ROWS:
        ps->unpackModes.skipLines = param;
        break;
    case GL_UNPACK_SKIP_PIXELS:
        ps->unpackModes.skipPixels = param;
        break;
    case GL_UNPACK_SKIP_IMAGES:
        ps->unpackModes.skipImages = param;
        break;
    case GL_UNPACK_IMAGE_HEIGHT:
        ps->unpackModes.imageHeight = param;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

OnError:
    __GL_FOOTER();
}

GLboolean __glCheckReadPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    __GLformatInfo * formatInfo = gcvNULL;
    __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

    if (!gc->dp.isFramebufferComplete(gc, readFBO))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_FRAMEBUFFER_OPERATION, GL_FALSE);
    }

    /* Check if framebuffer is complete */
    if (READ_FRAMEBUFFER_BINDING_NAME == 0)
    {
        if (gc->state.raster.readBuffer == GL_NONE)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        formatInfo = gc->drawablePrivate->rtFormatInfo;
    }
    else
    {
        __GLfboAttachPoint *attachPoint;
        GLint attachIndex;

        if (readFBO->readBuffer == GL_NONE)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        attachIndex = __glMapAttachmentToIndex(readFBO->readBuffer);
        if (-1 == attachIndex)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        attachPoint = &readFBO->attachPoint[attachIndex];

        if (0 == attachPoint->objName)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* Disable multi-samples check for tex RT to support EXT_multisampled_render_to_texture */
        if(readFBO->fbSamples > 0 && !(attachPoint->isExtMode))
        {
             __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        formatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
    }

    if ((width < 0) || (height < 0) || (formatInfo == gcvNULL))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    /* Implementation-chosen format information */
    if ((formatInfo->dataType == type) && (formatInfo->dataFormat == format))
    {
        return GL_TRUE;
    }

    switch (formatInfo->category)
    {
    case GL_SIGNED_NORMALIZED:
    case GL_UNSIGNED_NORMALIZED:
        if (GL_RGBA != format && GL_BGRA_EXT != format)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        if (__GL_FMT_RGB10_A2 == formatInfo->drvFormat)
        {
            if (GL_UNSIGNED_BYTE != type && GL_UNSIGNED_INT_2_10_10_10_REV != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
        }
        else if (GL_BGRA_EXT == format)
        {
            if(GL_UNSIGNED_BYTE != type &&
                GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT != type &&
                GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
        }
        else
        {
            if (GL_UNSIGNED_BYTE != type)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
        }
        break;

    case GL_FLOAT:
        if (GL_RGBA != format || GL_FLOAT != type)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    case GL_UNSIGNED_INT:
        if (GL_RGBA_INTEGER != format || GL_UNSIGNED_INT != type)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    case GL_INT:
        if (GL_RGBA_INTEGER != format || GL_INT != type)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    return GL_TRUE;
}

__GL_INLINE GLboolean __glReadPixelsBegin(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, GLvoid* pixels)
{
    if (width == 0 || height == 0)
    {
        return GL_FALSE;
    }

    if (gc->flags & __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER)
    {
        return GL_FALSE;
    }
    else
    {
        return (*gc->dp.readPixelsBegin)(gc);
    }

}

__GL_INLINE GLvoid __glReadPixelsValidateState(__GLcontext *gc)
{
    (*gc->dp.readPixelsValidateState)(gc);
}

__GL_INLINE GLvoid __glReadPixelsEnd(__GLcontext *gc)
{
    GLboolean retVal;

    retVal = (*gc->dp.readPixelsEnd)(gc);

    if(!retVal)
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }
}


GLvoid GL_APIENTRY __gles_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                     GLenum format, GLenum type, GLvoid* pixels)
{
    GLboolean retVal;
    __GLbufferObject *packBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    if (!__glCheckReadPixelArgs(gc, width, height, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from pack buffer object? */
    if (packBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.packModes, packBufObj, width, height, 0, format, type, pixels))
        {
            __GL_EXIT();
        }
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (GL_TRUE == __glReadPixelsBegin(gc, x, y, width, height, format, type, pixels))
    {
        __glReadPixelsValidateState(gc);

        retVal = (*gc->dp.readPixels)(gc, x, y, width, height, format, type, (GLubyte*)pixels);

        __glReadPixelsEnd(gc);

        if (!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnExit:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __gles_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height,
                                         GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    GLint requiredsize;
    __GLclientPixelState *ps = &gc->clientState.pixel;
    GLuint lineLength = ps->packModes.lineLength ? ps->packModes.lineLength : (GLuint)width;
    GLuint imageHeight = ps->packModes.imageHeight ? ps->packModes.imageHeight : (GLuint)height;

    __GL_HEADER();

    requiredsize = lineLength * __glPixelSize(gc, format, type);

    requiredsize = gcmALIGN(requiredsize, ps->packModes.alignment);

    requiredsize *= imageHeight;

    if (requiredsize <= bufSize)
    {
        __gles_ReadPixels(gc, x, y, width, height, format, type, data);
    }
    else
    {
        __GL_ERROR(GL_INVALID_OPERATION);
    }

    __GL_FOOTER();
}
