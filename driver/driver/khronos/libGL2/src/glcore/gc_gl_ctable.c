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


#include "glcore/gc_gl_context.h"
#include "glcore/gc_gl_debug.h"
extern GLint __glBytesPerElement(GLenum type);
extern GLint __glElementsPerGroup(GLenum format, GLenum type);
extern GLvoid __glInitPacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
extern GLvoid __glInitMemUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLsizei width, GLsizei height, GLsizei depth,
                       GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glInitMemStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer);
extern GLvoid __glInitMemStoreSub(__GLcontext *gc,
                                         __GLpixelSpanInfo *spanInfo,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLenum format, GLenum type, const GLvoid *buf);
extern GLvoid __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf);

/*
** Helper function to check glColorTable, glCopyColorTable, glGetColorTable
** arguments. Return non-zero if  any argument is invalid. As a side effect,
** set the base format of the internal format, and the number of components
** to the format, as well as the number of bits in each component category.
*/
GLenum __glCheckColorTableArgs(__GLcontext *gc, GLenum target,
                               GLenum internalformat, GLsizei width,
                               GLenum format, GLenum type)
{
    GLenum baseFormat;

    switch (target)
    {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
    case GL_SHARED_TEXTURE_PALETTE_EXT:
    case GL_COLOR_TABLE:
    case GL_POST_CONVOLUTION_COLOR_TABLE:
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
    case GL_PROXY_COLOR_TABLE:
    case GL_PROXY_POST_CONVOLUTION_COLOR_TABLE:
    case GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE:
        break;
    default:
        return (GL_INVALID_ENUM);
    }

    switch (internalformat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        baseFormat = GL_ALPHA;
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        baseFormat = GL_LUMINANCE;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        baseFormat = GL_LUMINANCE_ALPHA;
        break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
        baseFormat = GL_INTENSITY;
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        baseFormat = GL_RGB;
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        baseFormat = GL_RGBA;
        break;

    case GL_RGBA32F_ARB:
    case GL_RGBA16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_RGBA;
        break;

    case GL_RGB32F_ARB:
    case GL_RGB16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_RGB;
        break;

    case GL_ALPHA32F_ARB:
    case GL_ALPHA16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_ALPHA;
        break;

    case GL_INTENSITY32F_ARB:
    case GL_INTENSITY16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_INTENSITY;
        break;

    case GL_LUMINANCE32F_ARB:
    case GL_LUMINANCE16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_LUMINANCE;
        break;

    case GL_LUMINANCE_ALPHA32F_ARB:
    case GL_LUMINANCE_ALPHA16F_ARB:
        if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
            return (GL_INVALID_ENUM);

        return GL_LUMINANCE_ALPHA;
        break;

    default:
        return (GL_INVALID_ENUM);
    }

    /* width must be a positive power of two */
    if ((width < 0) || (width & (width - 1)))
    {
        return (GL_INVALID_VALUE);
    }

    /* Limit color table size so that pixel operations can be used */
    if ((GLuint) width > __GL_MAX_SPAN_SIZE /
        (__glBytesPerElement(type) * __glElementsPerGroup(baseFormat, type)))
    {
        return (GL_TABLE_TOO_LARGE_EXT);
    }

    switch (format)
    {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    default:
        return (GL_INVALID_ENUM);
    }

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
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
        return (GL_INVALID_ENUM);
    }

    return(GLenum) 0;
}

/*
** Helper function to check color subtable arguments. Return non-zero if
** any argument is invalid. As a side effect, set the base format of
** the internal format, and the number of components to the format,
** as well as the number of bits in each component category.
**
** Note: The soft implementation ignores all but the base format.
*/
GLenum __glCheckColorSubTableArgs(__GLcontext *gc, GLenum target,
                                  GLsizei start, GLsizei count,
                                  GLenum format, GLenum type)
{
    switch (target)
    {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
        break;
    default:
        return (GL_INVALID_ENUM);
    }

    if (start < 0 || count < 0)
    {
        return (GL_INVALID_VALUE);
    }

    switch (format)
    {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
        break;
    default:
        return (GL_INVALID_ENUM);
    }

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
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
        return (GL_INVALID_ENUM);
    }

    return(GLenum) 0;
}

/*
** Function to get __GLcolorTable manage struture for the dested target.
** Return NULL if this target is not supported.
** "isProxy" is OUTPUT, true if target is proxy.
*/
static __GLcolorTable *LookupColorTable(__GLcontext *gc, GLenum target,
                                        GLboolean *isProxy)
{
    *isProxy = GL_TRUE;

    switch (target)
    {
    case GL_COLOR_TABLE:
        *isProxy = GL_FALSE;
        return &gc->state.pixel.colorTable[__GL_COLOR_TABLE_INDEX];
    case GL_PROXY_COLOR_TABLE:
        *isProxy = GL_TRUE;
        return &gc->state.pixel.proxyColorTable[__GL_COLOR_TABLE_INDEX];
    case GL_POST_CONVOLUTION_COLOR_TABLE:
        *isProxy = GL_FALSE;
        return &gc->state.pixel.colorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX];
        break;
    case GL_PROXY_POST_CONVOLUTION_COLOR_TABLE:
        *isProxy = GL_TRUE;
        return &gc->state.pixel.proxyColorTable[__GL_POST_CONVOLUTION_COLOR_TABLE_INDEX];
        break;
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
        *isProxy = GL_FALSE;
        return &gc->state.pixel.colorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX];
        break;
    case GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE:
        *isProxy = GL_TRUE;
        return &gc->state.pixel.proxyColorTable[__GL_POST_COLOR_MATRIX_COLOR_TABLE_INDEX];
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return NULL;
    }
}

/*
** Helper function to setup __GLcolorTable states and alloc table memory if necessary.
*/
static GLboolean AllocColorTable(__GLcontext *gc, __GLcolorTable *ct,
                                 GLboolean isProxy, GLenum internalFormat, GLsizei width)
{
    GLenum baseFormat, type;
    GLint redSize, greenSize, blueSize, alphaSize, luminanceSize, intensitySize;
    GLint bufferSize;

    /*
    ** Map internal format to actual format and type
    */
    switch (internalFormat)
    {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        baseFormat = GL_ALPHA;
        type = GL_FLOAT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = 8;
        luminanceSize = 0;
        intensitySize = 0;
        break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        baseFormat = GL_LUMINANCE;
        type = GL_FLOAT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = 0;
        luminanceSize = 8;
        intensitySize = 0;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        baseFormat = GL_LUMINANCE_ALPHA;
        type = GL_FLOAT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = 8;
        luminanceSize = 8;
        intensitySize = 0;
        break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
        baseFormat = GL_INTENSITY;
        type = GL_FLOAT;
        redSize = 0;
        greenSize = 0;
        blueSize = 0;
        alphaSize = 0;
        luminanceSize = 0;
        intensitySize = 8;
        break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        baseFormat = GL_RGB;
        type = GL_FLOAT;
        redSize = 8;
        greenSize = 8;
        blueSize = 8;
        alphaSize = 0;
        luminanceSize = 0;
        intensitySize = 0;
        break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
        baseFormat = GL_RGBA;
        type = GL_FLOAT;
        redSize = 8;
        greenSize = 8;
        blueSize = 8;
        alphaSize = 8;
        luminanceSize = 0;
        intensitySize = 0;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        GL_ASSERT(0);
        return GL_FALSE;
    }

#if GL_EXT_paletted_texture
    /*
    ** Texture palettes are currently always stored as GL_UNSIGNED_BYTE
    */
    if ((ct->target == GL_TEXTURE_1D) ||
        (ct->target == GL_TEXTURE_2D) ||
        (ct->target == GL_TEXTURE_3D))
    {
        type = GL_UNSIGNED_BYTE;
    }
#endif

    bufferSize = width *
        __glElementsPerGroup(baseFormat, type) *
        __glBytesPerElement(type);

    /* Limit color table size so that pixel operations can be used */
    if (bufferSize > __GL_MAX_SPAN_SIZE)
    {
        ct->formatReturn = 0;
        ct->redSize = 0;
        ct->greenSize = 0;
        ct->blueSize = 0;
        ct->alphaSize = 0;
        ct->luminanceSize = 0;
        ct->intensitySize = 0;
        ct->width = 0;
        ct->format = 0;
        ct->baseFormat = 0;
        ct->type = 0;
        if (!isProxy)
        {
            __glSetError(GL_TABLE_TOO_LARGE);
        }
        return GL_FALSE;
    }

    /* Allocate color table buffer memory */
    if (!isProxy)
    {
        if(ct->table)
            (*gc->imports.free)(gc, ct->table);
        ct->table = (*gc->imports.malloc)(gc, bufferSize );
        if (!ct->table && bufferSize > 0)
        {
            __glSetError(GL_OUT_OF_MEMORY);
            return GL_FALSE;
        }
    }

    /* Update color table state */
    ct->width = width;

    ct->formatReturn = internalFormat;
    ct->format =
        ct->baseFormat = baseFormat;
    ct->type = type;

    ct->redSize = redSize;
    ct->greenSize = greenSize;
    ct->blueSize = blueSize;
    ct->alphaSize = alphaSize;
    ct->luminanceSize = luminanceSize;
    ct->intensitySize = intensitySize;

    return GL_TRUE;
}

/*
** Funtion to load color table, called by glColorTable,
** further call dp function to do actual load.
*/
GLboolean __glColorTable(__GLcontext* gc, GLenum target, GLenum internalFormat,
                         GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    __GLcolorTable *ct;
    GLboolean isProxy;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    /*Get CT manage structure*/
    ct = LookupColorTable(gc, target, &isProxy);
    if(!ct)
        return GL_FALSE;

    /*Malloc table mem and init CT states*/
    if (!AllocColorTable(gc, ct, isProxy, internalFormat, width))
    {
        return GL_FALSE;
    }

    if (isProxy || width == 0)
    {
        return GL_TRUE;
    }

    /*Load to system cache*/
    __RAW_INIT_SPANINFO();
    __glInitMemUnpack(gc, spanInfo, width, 1, 0,
        format, type, table);
    __glInitMemStore(gc, spanInfo, ct->format, ct->type, ct->table);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_TRUE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    if (__glNeedScaleBias(gc, &ct->state.scale, &ct->state.bias))
    {
        spanInfo->applyGenericScaleBias = GL_TRUE;
        spanInfo->scale = ct->state.scale;
        spanInfo->bias = ct->state.bias;
    }
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*Call dp to do dp related load */
    switch(target)
    {
    case GL_TEXTURE_1D:/*GL_EXT_paletted_texture*/
    case GL_TEXTURE_2D:
        //TODO: (*gc->dp.texColorTable)();
        break;
    case GL_COLOR_TABLE:
        (*gc->dp.colorTable)(gc, target, internalFormat, width, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_COLORTABLE_BIT);
        break;
    case GL_POST_CONVOLUTION_COLOR_TABLE:
        (*gc->dp.convColorTable)(gc, target, internalFormat, width, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_CONV_COLORTABLE_BIT);
        break;
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
        (*gc->dp.postColorMatrixColorTable)(gc, target, internalFormat, width, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_MATRIX_COLORTABLE_BIT);
        break;
    }
    return GL_TRUE;
}

/*
** Funtion called by glColorSubTable to load sub color table.
*/
GLboolean __glColorSubTable(__GLcontext* gc, GLenum target, GLsizei start,
                            GLsizei count, GLenum format, GLenum type, const GLvoid *table)
{
    __GLcolorTable *ct;
    GLboolean isProxy;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    ct = LookupColorTable(gc, target, &isProxy);
    if(!ct)
        return GL_FALSE;

    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    if (start + count > ct->width)
    {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }

    /*system cache update*/
    __RAW_INIT_SPANINFO();
    __glInitMemUnpack(gc, spanInfo, count, 1, 0,
        format, type, table);
    __glInitMemStoreSub(gc, spanInfo, start, 0, ct->format, ct->type, ct->table);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_TRUE;
    spanInfo->applyPixelTransfer = GL_FALSE;
    if (__glNeedScaleBias(gc, &ct->state.scale, &ct->state.bias))
    {
        spanInfo->applyGenericScaleBias = GL_TRUE;
        spanInfo->scale = ct->state.scale;
        spanInfo->bias = ct->state.bias;
    }
    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    /*dp update*/
    switch(target)
    {
    case GL_TEXTURE_1D:/*GL_EXT_paletted_texture*/
    case GL_TEXTURE_2D:
        //TODO: (*gc->dp.texColorSubTable)();
        break;
    case GL_COLOR_TABLE:
        (*gc->dp.colorSubTable)(gc, target, start, count, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_COLORTABLE_BIT);
        break;
    case GL_POST_CONVOLUTION_COLOR_TABLE:
        (*gc->dp.convColorSubTable)(gc, target, start, count, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_CONV_COLORTABLE_BIT);
        break;
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
        (*gc->dp.postColorMatrixColorSubTable)(gc, target, start, count, format, type, (GLubyte *)table);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_MATRIX_COLORTABLE_BIT);
        break;
    }

    return GL_TRUE;
}

/*
** Funtion called by glCopyColorTable to setup color table from framebuffer.
*/
GLboolean __glCopyColorTable(__GLcontext* gc, GLenum target, GLenum internalFormat,
                             GLint x, GLint y, GLsizei width)
{
    __GLcolorTable *ct;
    GLboolean isProxy;

    ct = LookupColorTable(gc, target, &isProxy);
    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    if (!AllocColorTable(gc, ct, isProxy, internalFormat, width))
    {
        return GL_FALSE;
    }

    if (width == 0)
    {
        return GL_TRUE;
    }

    /*Call dp to setup colortable from framebuffer.*/
    switch(target)
    {
    case GL_TEXTURE_1D:/*GL_EXT_paletted_texture*/
    case GL_TEXTURE_2D:
        //TODO: (*gc->dp.texColorSubTable)();
        break;
    case GL_COLOR_TABLE:
        (*gc->dp.copyColorTable)(gc, target, internalFormat, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_COLORTABLE_BIT);
        break;
    case GL_POST_CONVOLUTION_COLOR_TABLE:
        (*gc->dp.copyConvColorTable)(gc, target, internalFormat, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_CONV_COLORTABLE_BIT);
        break;
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
        (*gc->dp.copyPostColorMatrixColorTable)(gc, target, internalFormat, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_MATRIX_COLORTABLE_BIT);
        break;
    }

    return GL_TRUE;
}

/*
** Funtion called by glCopyColorSubTable to partly update color table from framebuffer.
*/
GLboolean __glCopyColorSubTable(__GLcontext* gc, GLenum target, GLsizei start,
                                GLint x, GLint y, GLsizei width)
{
    GLboolean isProxy;
    LookupColorTable(gc, target, &isProxy);
    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    /*Call dp to update colortable from framebuffer.*/
    switch(target)
    {
    case GL_TEXTURE_1D:/*GL_EXT_paletted_texture*/
    case GL_TEXTURE_2D:
        //TODO: (*gc->dp.texColorSubTable)();
        break;
    case GL_COLOR_TABLE:
        (*gc->dp.copyColorSubTable)(gc, target, start, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_COLORTABLE_BIT);
        break;
    case GL_POST_CONVOLUTION_COLOR_TABLE:
        (*gc->dp.copyConvColorSubTable)(gc, target, start, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_CONV_COLORTABLE_BIT);
        break;
    case GL_POST_COLOR_MATRIX_COLOR_TABLE:
        (*gc->dp.copyPostColorMatrixColorSubTable)(gc, target, start, x, y, width);
        __GL_SET_SWP_DIRTY_ATTR(gc, __GL_SWP_POST_MATRIX_COLORTABLE_BIT);
        break;
    }
    return GL_TRUE;
}

/*
** Function called by glGetColorTable. From  __GLcolorTable cache to APP.
*/
GLboolean __glGetColorTable(__GLcontext* gc, GLenum target, GLenum format, GLenum type,
                            GLvoid *table)
{
    __GLcolorTable *ct;
    GLboolean isProxy;
    __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;

    ct = LookupColorTable(gc, target, &isProxy);
    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    __RAW_INIT_SPANINFO();
    __glInitMemGet(gc, spanInfo, ct->width, 1, 0, ct->baseFormat, ct->type, ct->table);
    __glInitMemPack(gc, spanInfo, format, type, table);

    spanInfo->applySrcClamp = GL_FALSE;
    spanInfo->applyDstClamp = GL_FALSE;
    spanInfo->applyPixelTransfer = GL_FALSE; /*GetColorTable only affect by pixelStore.*/

    __glGenericPickCopyImage(gc, spanInfo, spanInfo->applyPixelTransfer);

    return GL_TRUE;
}

/*
** OpenGL GL_EXT_color_table APIs
*/

GLvoid APIENTRY __glim_ColorTable(GLenum target, GLenum internalFormat, GLsizei width,
                                  GLenum format, GLenum type, const GLvoid *table)
{
    GLenum rv;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorTable", DT_GLenum, target, DT_GLenum, internalFormat, DT_GLsizei, width, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, table, DT_GLnull);
#endif

    /*Params checking*/
    if ((rv = __glCheckColorTableArgs(gc, target,
        internalFormat, width, format, type)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glColorTable(gc,
        target,
        internalFormat,
        width,
        format,
        type,
        table);
}

GLvoid APIENTRY __glim_ColorSubTable(GLenum target, GLsizei start, GLsizei count,
                                     GLenum format, GLenum type, const GLvoid *table)
{
    GLenum rv;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorSubTable", DT_GLenum, target, DT_GLsizei, start, DT_GLsizei, count,DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, table, DT_GLnull);
#endif

    if ((rv = __glCheckColorSubTableArgs(gc, target,
        start, count, format, type)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glColorSubTable(gc,
        target,
        start,
        count,
        format,
        type,
        table);

}

GLvoid APIENTRY __glim_CopyColorTable(GLenum target, GLenum internalFormat,
                                      GLint x, GLint y, GLsizei width)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyColorTable", DT_GLenum, target, DT_GLenum_ptr, internalFormat,DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLnull);
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

    if ((rv = __glCheckColorTableArgs(gc, target,
        internalFormat, width, GL_RGBA, GL_FLOAT)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glCopyColorTable(gc,
        target,
        internalFormat,
        x,
        y,
        width);

}

GLvoid APIENTRY __glim_CopyColorSubTable(GLenum target, GLsizei start,
                                         GLint x, GLint y, GLsizei width)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyColorSubTable", DT_GLenum, target,DT_GLsizei, start, DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLnull);
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

    if ((rv = __glCheckColorSubTableArgs(gc, target, start, width, GL_RGBA, GL_FLOAT)))
    {
        __glSetError(rv);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glCopyColorSubTable(gc,
        target,
        start,
        x,
        y,
        width);

}

GLvoid APIENTRY __glim_GetColorTable(GLenum target, GLenum format, GLenum type,
                                     GLvoid *table)
{
    GLenum rv;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetColorTable", DT_GLenum, target, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, table, DT_GLnull);
#endif

    if ((rv = __glCheckColorTableArgs(gc, target, GL_RGB, 0, format, type)))
    {
        __glSetError(rv);
        return;
    }

    __glGetColorTable(gc,
        target,
        format,
        type,
        table);
}

GLvoid APIENTRY __glim_ColorTableParameteriv(GLenum target, GLenum pname,
                                             const GLint *params)
{
    __GLcolorTable *ct;
    GLboolean isProxy;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorTableParameteriv", DT_GLenum, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((ct = LookupColorTable(gc, target, &isProxy)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname)
    {
    case GL_COLOR_TABLE_SCALE:
        ct->state.scale.r = (__GLfloat)params[0];
        ct->state.scale.g = (__GLfloat)params[1];
        ct->state.scale.b = (__GLfloat)params[2];
        ct->state.scale.a = (__GLfloat)params[3];
        break;
    case GL_COLOR_TABLE_BIAS:
        ct->state.bias.r = (__GLfloat)params[0];
        ct->state.bias.g = (__GLfloat)params[1];
        ct->state.bias.b = (__GLfloat)params[2];
        ct->state.bias.a = (__GLfloat)params[3];
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_ColorTableParameterfv(GLenum target, GLenum pname,
                                             const GLfloat *params)
{
    __GLcolorTable *ct;
    GLboolean isProxy;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_ColorTableParameterfv", DT_GLenum, target, DT_GLenum, pname,DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((ct = LookupColorTable(gc, target, &isProxy)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    if (isProxy == GL_TRUE)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    switch (pname)
    {
    case GL_COLOR_TABLE_SCALE:
        ct->state.scale.r = (__GLfloat)params[0];
        ct->state.scale.g = (__GLfloat)params[1];
        ct->state.scale.b = (__GLfloat)params[2];
        ct->state.scale.a = (__GLfloat)params[3];
        break;
    case GL_COLOR_TABLE_BIAS:
        ct->state.bias.r = (__GLfloat)params[0];
        ct->state.bias.g = (__GLfloat)params[1];
        ct->state.bias.b = (__GLfloat)params[2];
        ct->state.bias.a = (__GLfloat)params[3];
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetColorTableParameteriv(GLenum target, GLenum pname,
                                                GLint *params)
{
    __GLcolorTable *ct;
    GLboolean isProxy;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetColorTableParameteriv", DT_GLenum_ptr, target, DT_GLenum, pname, DT_GLint_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((ct = LookupColorTable(gc, target, &isProxy)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_COLOR_TABLE_FORMAT:
        *params = (GLint)ct->formatReturn;
        break;
    case GL_COLOR_TABLE_WIDTH:
        *params = (GLint)ct->width;
        break;
    case GL_COLOR_TABLE_RED_SIZE:
        *params = (GLint)ct->redSize;
        break;
    case GL_COLOR_TABLE_GREEN_SIZE:
        *params = (GLint)ct->greenSize;
        break;
    case GL_COLOR_TABLE_BLUE_SIZE:
        *params = (GLint)ct->blueSize;
        break;
    case GL_COLOR_TABLE_ALPHA_SIZE:
        *params = (GLint)ct->alphaSize;
        break;
    case GL_COLOR_TABLE_LUMINANCE_SIZE:
        *params = (GLint)ct->luminanceSize;
        break;
    case GL_COLOR_TABLE_INTENSITY_SIZE:
        *params = (GLint)ct->intensitySize;
        break;
    case GL_COLOR_TABLE_SCALE:
        if (isProxy)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        params[0] = (GLint)ct->state.scale.r;
        params[1] = (GLint)ct->state.scale.g;
        params[2] = (GLint)ct->state.scale.b;
        params[3] = (GLint)ct->state.scale.a;
        break;
    case GL_COLOR_TABLE_BIAS:
        if (isProxy)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        params[0] = (GLint)ct->state.bias.r;
        params[1] = (GLint)ct->state.bias.g;
        params[2] = (GLint)ct->state.bias.b;
        params[3] = (GLint)ct->state.bias.a;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

GLvoid APIENTRY __glim_GetColorTableParameterfv(GLenum target, GLenum pname,
                                                GLfloat *params)
{
    __GLcolorTable *ct;
    GLboolean isProxy;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetColorTableParameterfv", DT_GLenum, target, DT_GLenum, pname, DT_GLfloat_ptr, params, DT_GLnull);
#endif

    if (params == NULL)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((ct = LookupColorTable(gc, target, &isProxy)) == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch (pname)
    {
    case GL_COLOR_TABLE_FORMAT:
        *params = (GLfloat)ct->formatReturn;
        break;
    case GL_COLOR_TABLE_WIDTH:
        *params = (GLfloat)ct->width;
        break;
    case GL_COLOR_TABLE_RED_SIZE:
        *params = (GLfloat)ct->redSize;
        break;
    case GL_COLOR_TABLE_GREEN_SIZE:
        *params = (GLfloat)ct->greenSize;
        break;
    case GL_COLOR_TABLE_BLUE_SIZE:
        *params = (GLfloat)ct->blueSize;
        break;
    case GL_COLOR_TABLE_ALPHA_SIZE:
        *params = (GLfloat)ct->alphaSize;
        break;
    case GL_COLOR_TABLE_LUMINANCE_SIZE:
        *params = (GLfloat)ct->luminanceSize;
        break;
    case GL_COLOR_TABLE_INTENSITY_SIZE:
        *params = (GLfloat)ct->intensitySize;
        break;
    case GL_COLOR_TABLE_SCALE:
        if (isProxy)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        params[0] = (GLfloat)ct->state.scale.r;
        params[1] = (GLfloat)ct->state.scale.g;
        params[2] = (GLfloat)ct->state.scale.b;
        params[3] = (GLfloat)ct->state.scale.a;
        break;
    case GL_COLOR_TABLE_BIAS:
        if (isProxy)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        params[0] = (GLfloat)ct->state.bias.r;
        params[1] = (GLfloat)ct->state.bias.g;
        params[2] = (GLfloat)ct->state.bias.b;
        params[3] = (GLfloat)ct->state.bias.a;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/*
** End: GL_EXT_color_table
*/

