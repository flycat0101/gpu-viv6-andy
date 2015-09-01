/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "glcore/gc_gl_context.h"
#include "glcore/gc_gl_debug.h"
extern GLvoid __glInitMemUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glInitMemStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer);
extern GLint __glElementsPerGroup(GLenum format, GLenum type);
extern GLint __glBytesPerElement(GLenum type);
extern GLvoid __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf);

/*
** Helper function
*/
static GLenum __glBaseInternalFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        return GL_ALPHA;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        return GL_LUMINANCE;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        return GL_LUMINANCE_ALPHA;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
        return GL_INTENSITY;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        return GL_RGB;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        return GL_RGBA;
    default:
        GL_ASSERT(0);
        return 0;
    }
}

static GLenum __glCheckConvolutionFilterArgs(__GLcontext *gc, GLenum target,
                                         GLsizei width, GLsizei height,
                                         GLenum internalFormat,
                                         GLenum format, GLenum type)
{
    switch (target)
    {
    case GL_CONVOLUTION_1D:
        if (width > (GLint)gc->constants.maxConvolution1DWidth || width < 0)
        {
            return(GL_INVALID_VALUE);
        }
        break;
    case GL_CONVOLUTION_2D:
        if (width > (GLint)gc->constants.maxConvolution2DWidth || width < 0 ||
            height > (GLint)gc->constants.maxConvolution2DHeight || height < 0)
        {
            return(GL_INVALID_VALUE);
        }
        break;
    case GL_SEPARABLE_2D:
        if (width > (GLint)gc->constants.maxSeparable2DWidth || width < 0 ||
            height > (GLint)gc->constants.maxSeparable2DHeight || height < 0)
        {
            return(GL_INVALID_VALUE);
        }
        break;
    default:
        return(GL_INVALID_ENUM);
    }

    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        break;
    default:
        return(GL_INVALID_ENUM);
    }

    switch (format)
    {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_BGR_EXT:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_ABGR_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    default:
        return(GL_INVALID_ENUM);
    }

    switch (type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
        break;
#if GL_EXT_packed_pixels
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            break;
        default:
            return(GL_INVALID_OPERATION);
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_BGRA_EXT:
        case GL_ABGR_EXT:
            break;
        default:
            return(GL_INVALID_OPERATION);
        }
        break;
#endif
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
        if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
            return (GL_INVALID_ENUM);

        if(format != GL_RGB)
            return (GL_INVALID_OPERATION);
        break;

    case GL_HALF_FLOAT_ARB:
        if (!__glExtension[INDEX_ARB_half_float_pixel].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        if(!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }

        if(format != GL_RGB)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    default:
        return(GL_INVALID_ENUM);
    }

    return 0;
}

static __GLconvolutionFilter* LookupConvolutionFilter(__GLcontext* gc, GLenum target)
{
    switch (target)
    {
    case GL_CONVOLUTION_1D:
        return &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX];
    case GL_CONVOLUTION_2D:
        return &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX];
    case GL_SEPARABLE_2D:
        return &gc->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX];
    default:
        return NULL;
    }
}

/*
** Implemention function
*/
GLboolean __glConvolutionFilter1D(__GLcontext* gc, GLenum target, GLenum internalFormat,
                      GLsizei width, GLenum format,
                      GLenum type, const GLvoid* image)
{
    __GLconvolutionFilter *cf;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    /*Setup manage structure*/
    cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX];
    cf->width = width;
    cf->height = 1;
    cf->format = cf->baseFormat = __glBaseInternalFormat(internalFormat);
    cf->formatReturn = internalFormat;
    cf->type = GL_FLOAT;

    if(cf->filter)
        (*gc->imports.free)(gc, cf->filter);

    cf->filter = (*gc->imports.malloc)(gc, width * sizeof(GLfloat) * 4 );
    if(!cf->filter)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }

    /*cache load*/
    __RAW_INIT_SPANINFO();
    __glInitMemUnpack(gc, spanInfo, width, 1,
        0, format, type, image);
    __glInitMemStore(gc, spanInfo, cf->format, cf->type, cf->filter);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    if (__glNeedScaleBias(gc, &cf->state.scale, &cf->state.bias))
    {
        spanInfo->applyGenericScaleBias = GL_TRUE;
        spanInfo->scale = cf->state.scale;
        spanInfo->bias = cf->state.bias;
    }
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*Dp load*/
    (*gc->dp.convolutionFilter1D)(gc, target, internalFormat, width, format, type, (GLvoid *)image);
    return GL_TRUE;
}

GLboolean __glConvolutionFilter2D(__GLcontext* gc, GLenum target, GLenum internalFormat,
                      GLsizei width, GLsizei height,
                      GLenum format, GLenum type,
                      const GLvoid* image)
{
    __GLconvolutionFilter *cf;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    /*Setup manage structure*/
    cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX];
    cf->width = width;
    cf->height = height;
    cf->format = cf->baseFormat = __glBaseInternalFormat(internalFormat);
    cf->formatReturn = internalFormat;
    cf->type = GL_FLOAT;

    if(cf->filter)
        (*gc->imports.free)(gc, cf->filter);

    cf->filter = (*gc->imports.malloc)(gc, width * height * sizeof(GLfloat) * 4 );
    if(!cf->filter)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }

    /*cache load*/
    __RAW_INIT_SPANINFO();
    __glInitMemUnpack(gc, spanInfo, width, height,
        0, format, type, image);
    __glInitMemStore(gc, spanInfo, cf->format, cf->type, cf->filter);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    if (__glNeedScaleBias(gc, &cf->state.scale, &cf->state.bias))
    {
        spanInfo->applyGenericScaleBias = GL_TRUE;
        spanInfo->scale = cf->state.scale;
        spanInfo->bias = cf->state.bias;
    }
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*Dp load*/
    (*gc->dp.convolutionFilter2D)(gc, target, internalFormat, width, format, type, (GLvoid *)image);
    return GL_TRUE;
}

GLboolean __glSeparableFilter2D(__GLcontext* gc, GLenum target, GLenum internalFormat,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    const GLvoid* row, const GLvoid* column)
{
    __GLconvolutionFilter *cf;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    /*Setup manage structure*/
    cf = &gc->state.pixel.convolutionFilter[__GL_SEPARABLE_2D_INDEX];
    cf->width = width;
    cf->height = height;
    cf->format = cf->baseFormat = __glBaseInternalFormat(internalFormat);
    cf->formatReturn = internalFormat;
    cf->type = GL_FLOAT;

    if(cf->filter)
        (*gc->imports.free)(gc, cf->filter);

    cf->filter = (*gc->imports.malloc)(gc, width * height * sizeof(GLfloat) * 4 );
    if(!cf->filter)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }

    /* treat separable filter as two spans */
    __RAW_INIT_SPANINFO();
    /*row*/
    __glInitMemUnpack(gc, spanInfo, width, 1, 0,
        format, type, row);
    __glInitMemStore(gc, spanInfo, cf->format, cf->type, cf->filter);
    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    if (__glNeedScaleBias(gc, &cf->state.scale, &cf->state.bias))
    {
        spanInfo->applyGenericScaleBias = GL_TRUE;
        spanInfo->scale = cf->state.scale;
        spanInfo->bias = cf->state.bias;
    }
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*column*/
    __glInitMemUnpack(gc, spanInfo, height, 1, 0,
        format, type, column);
    __glInitMemStore(gc, spanInfo, cf->format, cf->type,
        (GLubyte *)cf->filter + (width * __glElementsPerGroup(cf->format, cf->type) *
                                 __glBytesPerElement(cf->type)));
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*dp load*/
    (*gc->dp.separableFilter2D)(gc, target, internalFormat, width, format, type, (GLvoid *)row, (GLvoid *)column);
    return GL_TRUE;
}

GLboolean __glCopyConvolutionFilter2D(__GLcontext* gc, GLenum target, GLenum internalFormat,
                      GLint x, GLint y, GLsizei width,
                      GLsizei height)
{
    __GLconvolutionFilter *cf;

    cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX];
    cf->width = width;
    cf->height = height;
    cf->format = cf->baseFormat = __glBaseInternalFormat(internalFormat);
    cf->formatReturn = internalFormat;
    cf->type = GL_FLOAT;

    if(cf->filter)
        (*gc->imports.free)(gc, cf->filter);

    cf->filter = (*gc->imports.malloc)(gc, width * height * sizeof(GLfloat) * 4 );
    if(!cf->filter)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }

    /*Dp load*/
    (*gc->dp.copyConvolutionFilter2D)(gc, target, internalFormat, x, y, width, height);

    return GL_TRUE;
}

GLboolean __glCopyConvolutionFilter1D(__GLcontext* gc, GLenum target, GLenum internalFormat,
                   GLint x, GLint y, GLsizei width)
{
    __GLconvolutionFilter *cf;

    cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX];
    cf->width = width;
    cf->height = 1;
    cf->format = cf->baseFormat = __glBaseInternalFormat(internalFormat);
    cf->formatReturn = internalFormat;
    cf->type = GL_FLOAT;

    if(cf->filter)
        (*gc->imports.free)(gc, cf->filter);

    cf->filter = (*gc->imports.malloc)(gc, width * sizeof(GLfloat) * 4 );
    if(!cf->filter)
    {
        __glSetError(GL_OUT_OF_MEMORY);
        return GL_FALSE;
    }
    /*Dp load*/
    (*gc->dp.copyConvolutionFilter1D)(gc, target, internalFormat, x, y, width);
    return GL_TRUE;
}

GLboolean __glGetConvolutionFilter(__GLcontext* gc, GLenum target, GLenum format,
                       GLenum type, GLvoid* image)
{
    __GLconvolutionFilter *cf = NULL;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    switch (target)
    {
    case GL_CONVOLUTION_1D:
        cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_1D_INDEX];
        break;
    case GL_CONVOLUTION_2D:
        cf = &gc->state.pixel.convolutionFilter[__GL_CONVOLUTION_2D_INDEX];
        break;
    }

    __RAW_INIT_SPANINFO();
    __glInitMemGet(gc, spanInfo, cf->width, cf->height, 0,
        cf->format, cf->type, cf->filter);
    __glInitMemPack(gc, spanInfo, format, type, image);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE;

    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    return GL_TRUE;
}

GLboolean __glGetSeparableFilter(__GLcontext* gc, GLenum target, GLenum format, GLenum type,
                     GLvoid* row, GLvoid* column)
{
    __glGetConvolutionFilter(gc, target, format, type, row);
    __glGetConvolutionFilter(gc, target, format, type, column);
    return GL_TRUE;
}

/*
** OpenGL GL_EXT_convolution APIs
*/
GLvoid APIENTRY __glim_ConvolutionFilter1D(GLenum target, GLenum internalFormat,
                                           GLsizei width, GLenum format,
                                           GLenum type, const GLvoid* image)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionFilter1D", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLsizei, width, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, image, DT_GLnull);
#endif

    if (target != GL_CONVOLUTION_1D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, width, 1, internalFormat, format, type)) )
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glConvolutionFilter1D(gc,
            target,
            internalFormat,
            width,
            format,
            type,
            image);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_1D_BIT);

}


GLvoid APIENTRY __glim_ConvolutionFilter2D(GLenum target, GLenum internalFormat,
                                           GLsizei width, GLsizei height,
                                           GLenum format, GLenum type,
                                           const GLvoid* image)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionFilter2D", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLsizei, width, DT_GLsizei, height, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, image, DT_GLnull);
#endif

    if (target != GL_CONVOLUTION_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, width, height,
                                        internalFormat, format, type)) )
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glConvolutionFilter2D(gc,
        target,
        internalFormat,
        width,
        height,
        format,
        type,
        image);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_2D_BIT);
}


GLvoid APIENTRY __glim_SeparableFilter2D(GLenum target, GLenum internalFormat,
                                         GLsizei width, GLsizei height,
                                         GLenum format, GLenum type,
                                         const GLvoid* row, const GLvoid* column)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_SeparableFilter2D", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLsizei, width, DT_GLsizei, height, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, row, DT_GLvoid_ptr, column, DT_GLnull);
#endif

    if (target != GL_SEPARABLE_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, width, height,
                                        internalFormat, format, type)) )
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glSeparableFilter2D(gc,
        target,
        internalFormat,
        width,
        height,
        format,
        type,
        row,
        column);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_SEPARABLE_2D_BIT);
}


GLvoid APIENTRY __glim_CopyConvolutionFilter1D(GLenum target, GLenum internalFormat,
                                               GLint x, GLint y, GLsizei width)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyConvolutionFilter1D", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLnull);
#endif

    if(READ_FRAMEBUFFER_BINDING_NAME != 0)
    {
        if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.readFramebufObj))
        {
            __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
            return;
        }
        else
        {
            if(gc->frameBuffer.readFramebufObj->fbSamples > 0)
            {
                __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
                return;
            }
        }
    }


    if (target != GL_CONVOLUTION_1D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, width, 1,
                                        internalFormat, GL_RGBA, GL_FLOAT)) )
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glCopyConvolutionFilter1D(gc,
            target,
            internalFormat,
            x,
            y,
            width);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_1D_BIT);
}


GLvoid APIENTRY __glim_CopyConvolutionFilter2D(GLenum target, GLenum internalFormat,
                                               GLint x, GLint y, GLsizei width,
                                               GLsizei height)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyConvolutionFilter2D", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif

    if(READ_FRAMEBUFFER_BINDING_NAME != 0)
    {
        if(!gc->dp.isFramebufferComplete(gc, gc->frameBuffer.drawFramebufObj))
        {
            __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
            return;
        }
        else
        {
            if(gc->frameBuffer.drawFramebufObj->fbSamples > 0)
            {
                __glSetError(GL_INVALID_FRAMEBUFFER_OPERATION_EXT);
                return;
            }
        }
    }


    if (target != GL_CONVOLUTION_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, width, height,
                                        internalFormat, GL_RGBA, GL_FLOAT)) )
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glCopyConvolutionFilter2D(gc,
            target,
            internalFormat,
            x,
            y,
            width,
            height);
    __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_CONVOLUTION_2D_BIT);
}


GLvoid APIENTRY __glim_GetConvolutionFilter(GLenum target, GLenum format,
                                            GLenum type, GLvoid* image)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetConvolutionFilter", DT_GLenum, target, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, image, DT_GLnull);
#endif

    if ((target != GL_CONVOLUTION_1D)&&(target != GL_CONVOLUTION_2D))
    { /* only valid target for this fun */
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, 0, 0,
                                        GL_RGBA, format, type)) )
    {
        __glSetError(rv);
        return;
    }

    __glGetConvolutionFilter(gc,
            target,
            format,
            type,
            image);

}


GLvoid APIENTRY __glim_GetSeparableFilter(GLenum target, GLenum format, GLenum type,
                                          GLvoid* row, GLvoid* column, GLvoid* span)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetSeparableFilter", DT_GLenum, target, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, row, DT_GLvoid_ptr, column, DT_GLvoid_ptr, span, DT_GLnull);
#endif

    if (target != GL_SEPARABLE_2D)
    { /* only valid target for this fun */
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if ( (rv = __glCheckConvolutionFilterArgs(gc, target, 0, 0,
                                        GL_RGBA, format, type)) )
    {
        __glSetError(rv);
        return;
    }

    __glGetSeparableFilter(gc,
            target,
            format,
            type,
            row,
            column);
}


GLvoid APIENTRY __glim_ConvolutionParameteriv(GLenum target, GLenum pname,
                                              const GLint* params)
{
    __GLconvolutionFilter *filter;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((filter = LookupConvolutionFilter(gc, target)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname)
    {
    case GL_CONVOLUTION_BORDER_MODE:
        switch ((GLint)*params)
        {
        case GL_REDUCE:
#if GL_HP_convolution_border_modes
        case GL_IGNORE_BORDER_HP:
#endif
        case GL_CONSTANT_BORDER:
        case GL_REPLICATE_BORDER:
            filter->state.borderMode = (GLint)*params;
            break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;
    case GL_CONVOLUTION_BORDER_COLOR:
        /*
        ** Integer color values are mapped [-1,1].
        */
        filter->state.borderColor.r = __GL_I_TO_FLOAT(params[0]);
        filter->state.borderColor.g = __GL_I_TO_FLOAT(params[1]);
        filter->state.borderColor.b = __GL_I_TO_FLOAT(params[2]);
        filter->state.borderColor.a = __GL_I_TO_FLOAT(params[3]);
        break;
    case GL_CONVOLUTION_FILTER_SCALE:
        filter->state.scale.r = (GLfloat)params[0];
        filter->state.scale.g = (GLfloat)params[1];
        filter->state.scale.b = (GLfloat)params[2];
        filter->state.scale.a = (GLfloat)params[3];
        break;
    case GL_CONVOLUTION_FILTER_BIAS:
        filter->state.bias.r = (GLfloat)params[0];
        filter->state.bias.g = (GLfloat)params[1];
        filter->state.bias.b = (GLfloat)params[2];
        filter->state.bias.a = (GLfloat)params[3];
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_ConvolutionParameteri(GLenum target, GLenum pname,
                                             GLint param)
{

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionParameteri", DT_GLenum, target, DT_GLenum, pname, DT_GLint, param, DT_GLnull);
#endif


    switch (pname)
    {
    case GL_CONVOLUTION_BORDER_MODE:
        __glim_ConvolutionParameteriv(target, pname, &param);
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    /* Don't need to validate with current architecture */
}

GLvoid APIENTRY __glim_ConvolutionParameterfv(GLenum target, GLenum pname,
                                              const GLfloat* params)
{
    __GLconvolutionFilter *filter;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((filter = LookupConvolutionFilter(gc, target)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname)
    {
    case GL_CONVOLUTION_BORDER_MODE:
        switch ((GLint)*params)
        {
        case GL_REDUCE:
#if GL_HP_convolution_border_modes
        case GL_IGNORE_BORDER_HP:
#endif
        case GL_CONSTANT_BORDER:
        case GL_REPLICATE_BORDER:
            filter->state.borderMode = (GLint)*params;
            break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        break;
    case GL_CONVOLUTION_BORDER_COLOR:
        filter->state.borderColor.r = params[0];
        filter->state.borderColor.g = params[1];
        filter->state.borderColor.b = params[2] ;
        filter->state.borderColor.a = params[3] ;
        break;
    case GL_CONVOLUTION_FILTER_SCALE:
        filter->state.scale.r = params[0];
        filter->state.scale.g = params[1];
        filter->state.scale.b = params[2];
        filter->state.scale.a = params[3];
        break;
    case GL_CONVOLUTION_FILTER_BIAS:
        filter->state.bias.r = params[0];
        filter->state.bias.g = params[1];
        filter->state.bias.b = params[2];
        filter->state.bias.a = params[3];
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}


GLvoid APIENTRY __glim_ConvolutionParameterf(GLenum target, GLenum pname,
                                             GLfloat param)
{

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ConvolutionParameterf", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat, param, DT_GLnull);
#endif

    switch (pname)
    {
    case GL_CONVOLUTION_BORDER_MODE:
        __glim_ConvolutionParameterfv(target, pname, &param);
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    /* Don't need to validate with current architecture */
}

GLvoid APIENTRY __glim_GetConvolutionParameteriv(GLenum target, GLenum pname,
                                                 GLint* params)
{
    __GLconvolutionFilter *filter;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetConvolutionParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((filter = LookupConvolutionFilter(gc, target)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_CONVOLUTION_FORMAT:
        *params = (GLint)filter->format;
        break;
    case GL_CONVOLUTION_WIDTH:
        *params = (GLint)filter->width;
        break;
    case GL_CONVOLUTION_HEIGHT:
        *params = (GLint)filter->height;
        break;
    case GL_MAX_CONVOLUTION_WIDTH:
        switch (target)
        {
        case GL_CONVOLUTION_1D:
            *params = (GLint)gc->constants.maxConvolution1DWidth;
            break;
        case GL_CONVOLUTION_2D:
            *params = (GLint)gc->constants.maxConvolution2DWidth;
            break;
        case GL_SEPARABLE_2D:
            *params = (GLint)gc->constants.maxSeparable2DWidth;
            break;
        }
        break;
    case GL_MAX_CONVOLUTION_HEIGHT:
        switch (target)
        {
        case GL_CONVOLUTION_1D:
            *params = 0;
            break;
        case GL_CONVOLUTION_2D:
            *params = (GLint)gc->constants.maxConvolution2DHeight;
            break;
        case GL_SEPARABLE_2D:
            *params = (GLint)gc->constants.maxSeparable2DHeight;
            break;
        }
        break;
    case GL_CONVOLUTION_BORDER_MODE:
        *params = (GLint)filter->state.borderMode;
        break;
    case GL_CONVOLUTION_BORDER_COLOR:
        params[0] = (GLint)(filter->state.borderColor.r);
        params[1] = (GLint)(filter->state.borderColor.g);
        params[2] = (GLint)(filter->state.borderColor.b);
        params[3] = (GLint)(filter->state.borderColor.a);
        break;
    case GL_CONVOLUTION_FILTER_SCALE:
        params[0] = (GLint)filter->state.scale.r;
        params[1] = (GLint)filter->state.scale.g;
        params[2] = (GLint)filter->state.scale.b;
        params[3] = (GLint)filter->state.scale.a;
        break;
    case GL_CONVOLUTION_FILTER_BIAS:
        params[0] = (GLint)filter->state.bias.r;
        params[1] = (GLint)filter->state.bias.g;
        params[2] = (GLint)filter->state.bias.b;
        params[3] = (GLint)filter->state.bias.a;
        break;
    default: /* bad argument: set error, don't change anything */
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetConvolutionParameterfv(GLenum target, GLenum pname,
                                                 GLfloat* params)
{
    __GLconvolutionFilter *filter;

    __GL_SETUP_NOT_IN_BEGIN();


#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetConvolutionParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((filter = LookupConvolutionFilter(gc, target)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_CONVOLUTION_FORMAT:
        *params = (GLfloat)filter->format;
        break;
    case GL_CONVOLUTION_WIDTH:
        *params = (GLfloat)filter->width;
        break;
    case GL_CONVOLUTION_HEIGHT:
        *params = (GLfloat)filter->height;
        break;
    case GL_MAX_CONVOLUTION_WIDTH:
        switch (target)
        {
        case GL_CONVOLUTION_1D:
            *params = (GLfloat)gc->constants.maxConvolution1DWidth;
            break;
        case GL_CONVOLUTION_2D:
            *params = (GLfloat)gc->constants.maxConvolution2DWidth;
            break;
        case GL_SEPARABLE_2D:
            *params = (GLfloat)gc->constants.maxSeparable2DWidth;
            break;
        }
        break;
    case GL_MAX_CONVOLUTION_HEIGHT:
        switch (target)
        {
        case GL_CONVOLUTION_1D:
            *params = 0;
            break;
        case GL_CONVOLUTION_2D:
            *params = (GLfloat)gc->constants.maxConvolution2DHeight;
            break;
        case GL_SEPARABLE_2D:
            *params = (GLfloat)gc->constants.maxSeparable2DHeight;
            break;
        }
        break;
    case GL_CONVOLUTION_BORDER_MODE:
        *params = (GLfloat)filter->state.borderMode;
        break;
    case GL_CONVOLUTION_BORDER_COLOR:
        params[0] = filter->state.borderColor.r;
        params[1] = filter->state.borderColor.g;
        params[2] = filter->state.borderColor.b;
        params[3] = filter->state.borderColor.a;
        break;
    case GL_CONVOLUTION_FILTER_SCALE:
        params[0] = filter->state.scale.r;
        params[1] = filter->state.scale.g;
        params[2] = filter->state.scale.b;
        params[3] = filter->state.scale.a;
        break;
    case GL_CONVOLUTION_FILTER_BIAS:
        params[0] = filter->state.bias.r;
        params[1] = filter->state.bias.g;
        params[2] = filter->state.bias.b;
        params[3] = filter->state.bias.a;
        break;
    default: /* bad argument: set error, don't change anything */
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/*
**End: GL_EXT_convolution APIs
*/
