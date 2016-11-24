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


#include <math.h>
#include "gc_es_context.h"
/*
** Return the number of bytes per element, based on the element type
** Supports packed pixels as described in the specification, i.e.
** if packed pixel type return bytes per group.
*/
GLint __glBytesPerElement(GLenum type)
{
    switch (type)
    {
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_HALF_FLOAT_ARB:

        return (2);
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
        return (1);
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case __GL_UNSIGNED_INT_HIGH24:
    case __GL_UNSIGNED_S8_D24:
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        return (4);
    case GL_BITMAP: /* XXX what should be done here? */
        return (1);
    default:
        return (0);
    }
}


/*
** This file contains routines to unpack data from the user's data space
** into a span of pixels which can then be rendered.
*/

/*
** Return the number of elements per group of a specified format
** Supports packed pixels as described in the specification, i.e.
** if the type is packed pixel, the number of elements per group
** is understood to be one.
*/
GLint __glElementsPerGroup(GLenum format, GLenum type)
{
    /*
    ** To make row length computation valid for image extraction,
    ** packed pixel types assume elements per group equals one.
    */
    switch (type)
    {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        return (1);
    default:
        break;
    }

    switch (format)
    {
    case GL_RGB:
    case GL_BGR:
        return (3);
    case GL_LUMINANCE_ALPHA:
        return (2);
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGRA:
        return (4);
    /*extension GL_EXT_texture_integer*/
    case GL_RED_INTEGER_EXT:
    case GL_GREEN_INTEGER_EXT:
    case GL_BLUE_INTEGER_EXT:
    case GL_ALPHA_INTEGER_EXT:
    case GL_LUMINANCE_INTEGER_EXT:
        return (1);
    case GL_LUMINANCE_ALPHA_INTEGER_EXT:
        return (2);
    case GL_RGB_INTEGER_EXT:
    case GL_BGR_INTEGER_EXT:
        return (3);
    case GL_RGBA_INTEGER_EXT:
    case GL_BGRA_INTEGER_EXT:
        return (4);
    default:
        return (1);
    }
}
/*
** This file contains span packers.  A span packer takes a span of source
** data, and packs its contents into the user's data space.
**
** The packer is expected to aquire information about store modes from
** the __GLpixelSpanInfo structure.
*/
GLvoid __glInitPacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint alignment;
    GLint components;
    GLint element_size;
    GLint rowsize;
    GLint padding;
    GLint group_size;
    GLint groups_per_line;
    GLint skip_pixels, skip_lines;
    GLint skip_images;
    GLint imagesize, imageheight;
    GLint swap_bytes;
    GLenum format, type;
    const GLvoid *pixels;

    format = spanInfo->dstFormat;
    type = spanInfo->dstType;
    pixels = spanInfo->dstImage;
    alignment = spanInfo->dstAlignment;
    swap_bytes = spanInfo->dstSwapBytes;

    if( type >= __GL_MIN_TC_TYPE && type <= __GL_MAX_TC_TYPE)
    {
        /* TODO: dstSkipPixels & dstSkipLines are not __GL_TCBLOCK_DIM aligned */
        GL_ASSERT(!(spanInfo->dstSkipPixels % __GL_TCBLOCK_DIM));
        GL_ASSERT(!(spanInfo->dstSkipLines % __GL_TCBLOCK_DIM));

        imageheight = (spanInfo->dstImageHeight + __GL_TCBLOCK_DIM - 1) / __GL_TCBLOCK_DIM;
        skip_pixels = (spanInfo->dstSkipPixels + __GL_TCBLOCK_DIM - 1) / __GL_TCBLOCK_DIM;
        skip_lines = (spanInfo->dstSkipLines + __GL_TCBLOCK_DIM - 1) / __GL_TCBLOCK_DIM;
        skip_images = spanInfo->dstSkipImages;
        groups_per_line = (spanInfo->dstLineLength + __GL_TCBLOCK_DIM - 1) / __GL_TCBLOCK_DIM;
    }
    else
    {
        imageheight = spanInfo->dstImageHeight;
        skip_pixels = spanInfo->dstSkipPixels;
        skip_lines = spanInfo->dstSkipLines;
        skip_images = spanInfo->dstSkipImages;
        groups_per_line = spanInfo->dstLineLength;
    }

    components = __glElementsPerGroup(format, type);

    element_size = __glBytesPerElement(type);
    if (element_size == 1) swap_bytes = 0;
    group_size = element_size * components;

    rowsize = groups_per_line * group_size;
    if (type == GL_BITMAP) {
        rowsize = (groups_per_line + 7)/8;
    }

    padding = (rowsize % alignment);
    if (padding) {
        rowsize += alignment - padding;
    }
    imagesize = imageheight * rowsize;
    if (((skip_pixels & 0x7) && type == GL_BITMAP) ||
            (swap_bytes && element_size > 1)) {
        spanInfo->dstPackedData = GL_FALSE;
    } else {
        spanInfo->dstPackedData = GL_TRUE;
    }

    if (type == GL_BITMAP) {
        spanInfo->dstCurrent = (GLvoid *) (((const GLubyte*) pixels) +
                skip_lines * rowsize + skip_pixels / 8);
        spanInfo->dstStartBit = skip_pixels % 8;
    } else {
        spanInfo->dstCurrent = (GLvoid *) (((const GLubyte*) pixels) +
                skip_images * imagesize +
                skip_lines * rowsize +
                skip_pixels * group_size);
    }
    spanInfo->dstRowIncrement = rowsize;
    spanInfo->dstGroupIncrement = group_size;
    spanInfo->dstComponents = components;
    spanInfo->dstElementSize = element_size;
    spanInfo->dstImageIncrement = imagesize;

    /* Set defaults for general span modifier selection control */
    spanInfo->zeroFillAlpha = GL_FALSE;
    spanInfo->applySrcClamp = GL_TRUE;
    spanInfo->applyDstClamp = GL_TRUE;
    spanInfo->nonColorComp = GL_FALSE;
}
/*
** Return the number of components per pixel of a specified format or data type
*/
GLint __glComponentsPerPixel(GLenum format, GLenum type)
{
    switch (type)
    {
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        return (3);
        break;

    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
    case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:
        return (4);
        break;

    default:
        break;
    }

    switch (format)
    {
    case GL_RGB:
    case GL_BGR:
        return (3);
    case GL_LUMINANCE_ALPHA:
        return (2);
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGRA:
        return (4);
    /*extension GL_EXT_texture_integer */
    case GL_RED_INTEGER_EXT:
    case GL_GREEN_INTEGER_EXT:
    case GL_BLUE_INTEGER_EXT:
    case GL_ALPHA_INTEGER_EXT:
    case GL_LUMINANCE_INTEGER_EXT:
        return (1);
    case GL_LUMINANCE_ALPHA_INTEGER_EXT:
        return (2);
    case GL_RGB_INTEGER_EXT:
    case GL_BGR_INTEGER_EXT:
       return (3);
    case GL_RGBA_INTEGER_EXT:
    case GL_BGRA_INTEGER_EXT:
        return (4);
    default:
        return (1);
    }
}
/* ARGSUSED */
GLvoid __glInitUnpacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLint alignment;
    GLint components;
    GLint element_size;
    GLint rowsize;
    GLint padding;
    GLint group_size;
    GLint groups_per_line;
    GLint skip_pixels, skip_lines;
    GLint skip_images;
    GLint imagesize, imageheight;
    GLint swap_bytes;
    GLenum format, type;
    const GLvoid *pixels;

    format = spanInfo->srcFormat;
    type = spanInfo->srcType;
    pixels = spanInfo->srcImage;
    skip_pixels = spanInfo->srcSkipPixels;
    skip_lines = spanInfo->srcSkipLines;
    skip_images = spanInfo->srcSkipImages;
    alignment = spanInfo->srcAlignment;
    swap_bytes = spanInfo->srcSwapBytes;

    components = __glElementsPerGroup(format, type);

    if( type >= __GL_MIN_TC_TYPE && type <= __GL_MAX_TC_TYPE)
    {
        groups_per_line = (spanInfo->srcLineLength + __GL_TCBLOCK_DIM - 1)
                                                                /__GL_TCBLOCK_DIM;
        imageheight = (spanInfo->srcImageHeight + __GL_TCBLOCK_DIM - 1)
                                                                /__GL_TCBLOCK_DIM;

    }
    else
    {
        groups_per_line = spanInfo->srcLineLength;
        imageheight = spanInfo->srcImageHeight;

    }

    element_size = __glBytesPerElement(type);
    if (element_size == 1) swap_bytes = 0;
    group_size = element_size * components;

    rowsize = groups_per_line * group_size;
    if (type == GL_BITMAP)
    {
        rowsize = (groups_per_line + 7)/8;
    }
    padding = (rowsize % alignment);
    if (padding)
    {
        rowsize += alignment - padding;
    }
    imagesize = imageheight * rowsize;

    if (((skip_pixels & 0x7) && type == GL_BITMAP) ||
        (swap_bytes && element_size > 1))
    {
        spanInfo->srcPackedData = GL_FALSE;
    }
    else
    {
        spanInfo->srcPackedData = GL_TRUE;
    }

    if (type == GL_BITMAP)
    {
        spanInfo->srcCurrent = (GLvoid *) (((const GLubyte *) pixels) +
                                           skip_lines * rowsize + skip_pixels / 8);
        spanInfo->srcStartBit = skip_pixels % 8;
    }
    else
    {
        spanInfo->srcCurrent = (GLvoid *) (((const GLubyte *) pixels) +
                                           skip_images * imagesize +
                                           skip_lines * rowsize +
                                           skip_pixels * group_size);
    }

    spanInfo->srcRowIncrement = rowsize;
    spanInfo->srcGroupIncrement = group_size;
    spanInfo->srcComponents = components;
    spanInfo->srcElementSize = element_size;
    spanInfo->srcImageIncrement = imagesize;
}

/*
** Used to set up Writing into sys mem. without doing any packing.
** Note: Dest address is exactly "buf". No x/y offset required.
**
*/
/* ARGSUSED */
GLvoid __glInitMemStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf)
{
    /*Generic dest info*/
    spanInfo->dstFormat    = format;
    spanInfo->dstType      = type;
    spanInfo->dstImage = buf;

    /*default pack state. no pack. no offset.*/
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSkipImages = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = spanInfo->width;
    spanInfo->dstImageHeight= spanInfo->height;
    spanInfo->dstAlignment = __glBytesPerElement(type);

    /*Init rest fields for writting data to sys mem.*/
    __glInitPacker(gc, spanInfo);

    /*
    ** Put here those states which not needed but readed by the
    ** generic pixel path.
    */
    spanInfo->dim = 0; /* don't convolve */
}

/*
** Used to set up Reading data from system mem. without doing any unpacking.
*/
/*ARGSUSED*/
GLvoid __glInitMemGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLenum format, GLenum type, const GLvoid *buf)
{
     /* General source info*/
    spanInfo->srcImage      = buf;
    spanInfo->srcFormat    = format;
    spanInfo->srcType      = type;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;

    /*Default unpack state. No unpacking required*/
    spanInfo->srcSkipPixels = 0;
    spanInfo->srcSkipLines  = 0;
#if GL_EXT_texture3D
    spanInfo->srcSkipImages = 0;
#endif
    spanInfo->srcSwapBytes  = GL_FALSE;
    spanInfo->srcLsbFirst   = GL_TRUE;
    spanInfo->srcLineLength = width; /*still need to set because is necessary for calcute further info*/
#if GL_EXT_texture3D
    spanInfo->srcImageHeight = height;/*still need to set because is necessary for calcute further info*/
#endif
    spanInfo->srcAlignment = __glBytesPerElement(type);

    /*Init rest fields for reading data from sys mem.*/
    __glInitUnpacker(gc, spanInfo);

    /*
    ** Put here those states which not needed but readed by the
    ** generic pixel path.
    */
    spanInfo->dim = 0; /* don't convolve */
}

/*
** Used to set up Writing into sub rect of sys mem. without doing any packing.
**
** buf: start address to offseted from.
** Tobe update address = buf + yoffset*line_length + xoffset*skip_pixels.
*/
GLvoid __glInitMemStoreSub(__GLcontext *gc,
                                         __GLpixelSpanInfo *spanInfo,
                                         GLint xoffset,
                                         GLint yoffset,
                                         GLenum format, GLenum type, const GLvoid *buf)
{
    /*Generic dest info*/
    spanInfo->dstFormat    = format;
    spanInfo->dstType      = type;
    spanInfo->dstImage = buf;

    /*default pack state. no pack. with offset.*/
    spanInfo->dstSkipPixels = xoffset;
    spanInfo->dstSkipLines = yoffset;
#if GL_EXT_texture3D
    spanInfo->dstSkipImages = 0;
#endif
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = spanInfo->width;
    spanInfo->dstImageHeight= spanInfo->height;
    spanInfo->dstAlignment = __glBytesPerElement(type);

    /*Init rest fields for writting data to sys mem.*/
    __glInitPacker(gc, spanInfo);

}

/*
** Used to set up APP mem as source. With unpacking.
** "dltex" is GL_TRUE if the mem object is a display-listed texture.
*/
GLvoid __glInitMemUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLsizei width, GLsizei height, GLsizei depth,
                       GLenum format, GLenum type, const GLvoid *buf)
{
    /*Generic src info*/
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;

    /* Setup unpack modes according to the gc pixel state */
    __glLoadUnpackModes(gc, spanInfo);

    /*Init rest fields for reading data from sys mem.*/
    __glInitUnpacker(gc, spanInfo);
    /*
    ** Put here those states which not needed but readed by the
    ** generic pixel path.
    */
}

/*
** Used to set up APP mem as destination. With packing.
*/
GLvoid __glInitMemPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLenum format, GLenum type, const GLvoid *buf)
{
    /*Generic dest info*/
    spanInfo->dstFormat = format;
    spanInfo->dstType = type;
    spanInfo->dstImage = buf;

    /* Set the remaining pack modes according to the pixel state */
    __glLoadPackModes(gc, spanInfo);

    /*Init rest fields for packing data to sys mem.*/
    __glInitPacker(gc, spanInfo);

    /*
    ** Put here those states which not needed but readed by the
    ** generic pixel path.
    */
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
}

GLboolean __glNeedScaleBias(__GLcontext *gc, __GLcolor *scale, __GLcolor *bias)
{
    return((scale->r != __glOne) ||
           (scale->g != __glOne) ||
           (scale->b != __glOne) ||
           (scale->a != __glOne) ||
           (bias->r != __glZero) ||
           (bias->g != __glZero) ||
           (bias->b != __glZero) ||
           (bias->a != __glZero));
}




/*
** Initialize the spanInfo unpack members.
** If "bForDL" is true, the structure is initialized for unpacking data from a display list.
** If "bForDL" is false, it is initialized for unpacking data from gc pixel unpack states.
*/
GLvoid __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    if (!gc->dlist.pCurrentDlist)
    {
        /*
        ** Data came straight from the application.
        */
        GLint lineLength, imageHeight;
        __GLpixelUnpackMode *unpackModes = &gc->clientState.pixel.unpackModes;

        lineLength = unpackModes->lineLength;
        spanInfo->srcAlignment  = unpackModes->alignment;
        spanInfo->srcSkipPixels = unpackModes->skipPixels;
        spanInfo->srcSkipLines  = unpackModes->skipLines;
        spanInfo->srcSkipImages = unpackModes->skipImages;
        spanInfo->srcLsbFirst   = unpackModes->lsbFirst;
        spanInfo->srcSwapBytes  = unpackModes->swapEndian;
        if (lineLength <= 0)
        {
            lineLength = spanInfo->width;
        }
        spanInfo->srcLineLength = lineLength;

        imageHeight = unpackModes->imageHeight;
        if (imageHeight <= 0) imageHeight = spanInfo->height;
        spanInfo->srcImageHeight = imageHeight;
        spanInfo->srcSkipImages  = unpackModes->skipImages;
    }
    else
    {
        /*
        ** Data came from a display list.
        */

        spanInfo->srcAlignment = 1;
        spanInfo->srcSkipPixels = 0;
        spanInfo->srcSkipLines = 0;
        spanInfo->srcSkipImages = 0;
        spanInfo->srcLsbFirst = GL_FALSE;
        spanInfo->srcSwapBytes = GL_FALSE;
        spanInfo->srcLineLength = spanInfo->width;
        spanInfo->srcImageHeight = spanInfo->height;
    }
}

/*
** Initialize the spanInfo with NO unpack members. Especially used for compressed Texture uploading.
** Special from the above:
** If "bForDL" is true, the structure is initialized for unpacking data from a display list.
** If "bForDL" is false, it is initialized for unpacking data from gc pixel unpack states.
*/
GLvoid __glLoadNoUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    if (!gc->dlist.pCurrentDlist)
    {
        /*
        ** Data came straight from the application.
        */
        spanInfo->srcAlignment = 1;
        spanInfo->srcSkipPixels = 0;
        spanInfo->srcSkipLines = 0;
        spanInfo->srcSkipImages = 0;
        spanInfo->srcLsbFirst = GL_FALSE;
        spanInfo->srcSwapBytes = GL_FALSE;
        spanInfo->srcLineLength = spanInfo->width;
        spanInfo->srcImageHeight = spanInfo->height;
    }
    else
    {
        /*
        ** Data came from a display list.
        */

        spanInfo->srcAlignment = 1;
        spanInfo->srcSkipPixels = 0;
        spanInfo->srcSkipLines = 0;
        spanInfo->srcSkipImages = 0;
        spanInfo->srcLsbFirst = GL_FALSE;
        spanInfo->srcSwapBytes = GL_FALSE;
        spanInfo->srcLineLength = spanInfo->width;
        spanInfo->srcImageHeight = spanInfo->height;
    }
}

/*
** Initialize the spanInfo structure for packing data into the user's data
** space.
*/
GLvoid __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    if (!gc->dlist.pCurrentDlist)
    {
        /*
        ** Data came straight from the application.
        */
        __GLpixelPackMode *packModes = &gc->clientState.pixel.packModes;
        GLint lineLength = packModes->lineLength;
        GLint imageHeight = packModes->imageHeight;

        spanInfo->dstAlignment = packModes->alignment;
        spanInfo->dstSkipPixels = packModes->skipPixels;
        spanInfo->dstSkipLines = packModes->skipLines;
        spanInfo->dstSkipImages = packModes->skipImages;
        spanInfo->dstLsbFirst = packModes->lsbFirst;
        spanInfo->dstSwapBytes = packModes->swapEndian;
        if (lineLength <= 0) lineLength = spanInfo->width;
        spanInfo->dstLineLength = lineLength;
        if (imageHeight <= 0) imageHeight = spanInfo->height;
        spanInfo->dstImageHeight = imageHeight;
    }
    else
    {
        /*
        ** Data came from a display list.
        */

        spanInfo->dstAlignment = 1;
        spanInfo->dstSkipPixels = 0;
        spanInfo->dstSkipLines = 0;
        spanInfo->dstSkipImages = 0;
        spanInfo->dstLsbFirst = GL_FALSE;
        spanInfo->dstSwapBytes = GL_FALSE;
        spanInfo->dstLineLength = spanInfo->width;
        spanInfo->dstImageHeight = spanInfo->height;
    }
}

/*
** Internal image processing routine.  Used by GetTexImage to transfer from
** internal texture image to the user.  Used by TexImage[12]D to transfer
** from the user to internal texture.  Used for display list optimization of
** textures and DrawPixels. Used by GetColorTable, GetConvolutionFilter,
** GetSeparableFilter, GetHistogram, GetMinmax, GetPolygonStipple,
**
** This routine also supports the pixel format mode __GL_RED_ALPHA which is
** basically a 2 component texture.
**
** If applyPixelTransfer is set to GL_TRUE, pixel transfer modes will be
** applied as necessary.
*/
GLvoid __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer)
{
}
