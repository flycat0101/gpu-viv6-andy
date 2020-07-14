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

    if ( type >= __GL_MIN_TC_TYPE && type <= __GL_MAX_TC_TYPE)
    {
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

    if ( type >= __GL_MIN_TC_TYPE && type <= __GL_MAX_TC_TYPE)
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
        __GLpixelPackMode *unpackModes = &gc->clientState.pixel.unpackModes;

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
** textures and DrawPixels.
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

/* Get component and mask info from format */
GLboolean __glGetComponentAndMaskFromFormat(GLenum format, GLubyte* components, GLubyte* mask)
{
    switch (format)
    {
        case GL_RED:
            *components = 1;
            mask[0] = 1;
            break;
        case GL_GREEN:
            *components = 1;
            mask[1] = 2;
            break;
        case GL_BLUE:
            *components = 1;
            mask[0] = 3;
            break;
        case GL_ALPHA:
            *components = 1;
            mask[0] = 4;
            break;
        case GL_RG:
            *components = 2;
            mask[0] = 1;
            mask[1] = 2;
            break;
        case GL_LUMINANCE:
        case GL_RGB:
            *components = 3;
            mask[0] = 1;
            mask[1] = 2;
            mask[2] = 3;
            break;
        case GL_BGR:
            *components = 3;
            mask[0] = 3;
            mask[1] = 2;
            mask[2] = 1;
            break;
        case GL_LUMINANCE_ALPHA:
        case GL_INTENSITY:
        case GL_RGBA:
            *components = 4;
            mask[0] = 1;
            mask[1] = 2;
            mask[2] = 3;
            mask[3] = 4;
            break;
        case GL_BGRA:
            *components = 4;
            mask[0] = 3;
            mask[1] = 2;
            mask[2] = 1;
            mask[3] = 4;
            break;
        default:
            return GL_TRUE;
    }
    return GL_FALSE;
}

GLvoid __glCalculateMemoryAndNumOfComponents(__GLpixelTransferInfo *transferInfo)
{
    if ((transferInfo->dstSizeOfPixel * transferInfo->width) % transferInfo->alignment)
    {
        transferInfo->numOfAlign =  transferInfo->alignment - ((transferInfo->dstSizeOfPixel * transferInfo->width) %  transferInfo->alignment);
    }
    else
    {
        transferInfo->numOfAlign = 0;
    }
    transferInfo->sizeOfAlignMemory = transferInfo->width * transferInfo->height * transferInfo->depth * transferInfo->dstSizeOfPixel + transferInfo->height * transferInfo->numOfAlign;

    if ((transferInfo->srcSizeOfPixel * transferInfo->width) %  transferInfo->alignment)
    {
        transferInfo->numOfComponents = transferInfo->compNumber * transferInfo->numOfPixel;
        transferInfo->numOfAlignSrc= transferInfo->alignment - ((transferInfo->srcSizeOfPixel * transferInfo->width) %  transferInfo->alignment);

    }
    else
    {
        transferInfo->numOfComponents = transferInfo->compNumber * transferInfo->numOfPixel * transferInfo->width / transferInfo->widthAlign;
        transferInfo->numOfPixel =  transferInfo->width * transferInfo->height * transferInfo->depth;
    }
}

GLboolean __glInitTransferInfo(__GLcontext *gc,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth,
                                __GLpixelTransferInfo *transferInfo,
                                GLenum baseFmt,
                                GLenum srcType,
                                GLenum dstType,
                                GLenum pixelTransferOperations)
{
    transferInfo->scale.r = gc->state.pixel.transferMode.r_scale;
    transferInfo->scale.g = gc->state.pixel.transferMode.g_scale;
    transferInfo->scale.b = gc->state.pixel.transferMode.b_scale;
    transferInfo->scale.a = gc->state.pixel.transferMode.a_scale;
    transferInfo->bias.r = gc->state.pixel.transferMode.r_bias;
    transferInfo->bias.g = gc->state.pixel.transferMode.g_bias;
    transferInfo->bias.b = gc->state.pixel.transferMode.b_bias;
    transferInfo->bias.a = gc->state.pixel.transferMode.a_bias;

    transferInfo->width = width;
    transferInfo->height = height;
    transferInfo->depth = depth;

    transferInfo->alignment = gc->clientState.pixel.packModes.alignment;
    __glMemoryAlignment(baseFmt, srcType, dstType, transferInfo, pixelTransferOperations);
    transferInfo->numOfPixel =  transferInfo->widthAlign * height * depth;
    if (0 == transferInfo->numOfPixel)
    {
        __GL_EXIT();
    }

    if (gc->imports.conformGLSpec)
    {
        transferInfo->applyGenericScaleBias = __glNeedScaleBias(gc, &(transferInfo->scale), &(transferInfo->bias));
    }
    else
    {
        transferInfo->applyGenericScaleBias = GL_FALSE;
    }

    transferInfo->srcType = srcType;
    transferInfo->dstType = dstType;
    if ((GL_FALSE == transferInfo->applyGenericScaleBias) && (transferInfo->srcType == transferInfo->dstType))
    {
        /* No need do pixel operation */
        transferInfo->applyPixelTransfer = GL_FALSE;
        __GL_EXIT();
    }
    else
    {
        transferInfo->applyPixelTransfer = GL_TRUE;
    }

    transferInfo->baseFormat = baseFmt;
    transferInfo->srcSizeOfPixel = __glPixelSize(gc, transferInfo->baseFormat, transferInfo->srcType);
    GL_ASSERT(0 != transferInfo->srcSizeOfPixel);

    transferInfo->dstSizeOfPixel = __glPixelSize(gc, transferInfo->baseFormat, transferInfo->dstType);
    GL_ASSERT(0 != transferInfo->dstSizeOfPixel);

    if (GL_TRUE == __glGetComponentAndMaskFromFormat(transferInfo->baseFormat, &(transferInfo->compNumber), &(transferInfo->compMask[0])))
    {
        __GL_EXIT();
    }

    /* calculate dst alignment memory size and src numofComponents */
    __glCalculateMemoryAndNumOfComponents(transferInfo);

    return GL_TRUE;

OnExit:
    return GL_FALSE;
}


#define gcdUINT_MAX(n)                  ((gctUINT)(((gctUINT64)1 << (n)) - 1))
#define gcdGET_FIELD(data, start, bits) (((data) >> (start)) & (~(~0U << bits)))
#define gcdOFFSET_O_DOT_5(x)            (((x) < 0) ? ((x) - 0.5f) : ((x) + 0.5f))


#define __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(offset, start, bits) \
    {\
        *((GLfloat *)tmpBuf+i+offset) = (GLfloat)gcdGET_FIELD(tmpUint32, start, bits) / (GLfloat)gcdUINT_MAX(bits);\
    }

#define __GL_CONVERT_SIGNED_2_FLOAT(srcType, maxValue) \
    {\
        *((GLfloat *)tmpBuf+i) = (GLfloat)(*((srcType *)buf+i)) / (GLfloat)(maxValue);\
    }

#define __GL_CONVERT_UNSIGNED_2_FLOAT(srcType, maxValue) \
    {\
        *((GLfloat *)tmpBuf+i) = (GLfloat)(*((srcType *)buf+i)) / (GLfloat)(maxValue);\
    }

#define __GL_CONVERT_HALF_FLOAT_2_FLOAT() \
    {\
        tmpUint32 = gcoMATH_Float16ToFloat(*((GLushort *)buf+i));\
        *((GLfloat *)tmpBuf+i) = *(GLfloat *)&tmpUint32;\
    }

#define __GL_CONVERT_UNSIGNED_L_2_RGB(srcType, maxValue)\
    {\
        readOffset = (3 == components) ? i : 2*i;\
        if (3 == j)\
        {\
            *((GLfloat *)tmpBuf+components*i+j) =  (GLfloat)(*((srcType *)buf+readOffset+1)) / (GLfloat)(maxValue);\
        }\
        else\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)(*((srcType *)buf+readOffset)) / (GLfloat)(maxValue);\
        }\
    }

#define __GL_CONVERT_SIGNED_L_2_RGB(srcType, maxValue) \
    {\
        readOffset = (3 == components) ? i : 2*i;\
        if (3 == j)\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)(*((srcType *)buf+readOffset+1) * 2 + 1) / (GLfloat)(maxValue);\
        }\
        else\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)(*((srcType *)buf+readOffset) * 2 + 1) / (GLfloat)(maxValue);\
        }\
    }

#define __GL_CONVERT_FLOAT_L_2_RGB()\
    {\
        readOffset = (3 == components) ? i : 2*i;\
        if (3 == j)\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = *((GLfloat*)buf+readOffset+1);\
        }\
        else\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = *((GLfloat*)buf+readOffset);\
        }\
    }

#define __GL_CONVERT_HALF_FLOAT_L_2_RGB()\
    {\
        readOffset = (3 == components) ? i : 2*i;\
        if (3 == j)\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)gcoMATH_Float16ToFloat(*((GLushort *)buf+readOffset+1));\
        }\
        else\
        {\
            *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)gcoMATH_Float16ToFloat(*((GLushort *)buf+readOffset));\
        }\
    }

#define __GL_CONVERT_SIGNED_I_2_RGBA(srcType, maxValue) \
    {\
        *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)(*((srcType *)buf+i) * 2 + 1) / (GLfloat)(maxValue);\
    }

#define __GL_CONVERT_UNSIGNED_I_2_RGBA(srcType, maxValue)\
    {\
        *((GLfloat *)tmpBuf+components*i+j) =  (GLfloat)(*((srcType *)buf+i)) / (GLfloat)(maxValue);\
    }

#define __GL_CONVERT_FLOAT_I_2_RGBA()\
    {\
        *((GLfloat *)tmpBuf+components*i+j) = *((GLfloat*)buf+i);\
    }

#define __GL_CONVERT_HALF_FLOAT_I_2_RGBA()\
    {\
        *((GLfloat *)tmpBuf+components*i+j) = (GLfloat)gcoMATH_Float16ToFloat(*((GLushort *)buf+i));\
    }

#define __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(offset, start, bits) \
    {\
        tmpUint32 |= (GLuint)(((GLuint)(*((GLfloat *)tmpBuf+i+offset) * (GLfloat)(gcdUINT_MAX(bits)) + 0.5f) & gcdUINT_MAX(bits)) << start);\
    }

#define __GL_FINALCONVERSION_2_SIGNED(dstType, maxValue) \
    {\
        *((dstType *)outBuf+i) = (dstType)(gcdOFFSET_O_DOT_5(((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) - 1.0f) / 2.0f));\
    }

#define __GL_FINALCONVERSION_2_UNSIGNED(dstType, maxValue) \
    {\
        *((dstType *)outBuf+i) = (dstType)((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) + 0.5f);\
    }

#define __GL_FINALCONVERSION_2_HALF_FLOAT() \
    {\
        *((GLushort *)outBuf+i) = gcoMATH_FloatToFloat16(*((GLuint *)tmpBuf+i));\
    }

#define __GL_L_FINALCONVERSION_2_UNSIGNED(dstType, maxValue) \
    {\
        writeOffset = (3 == components) ? (i/components) : ((i/components)*2);\
        *((dstType *)outBuf+writeOffset) = (dstType)((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) + 0.5f);\
        if (4 == components)\
        {\
            *((dstType *)outBuf+writeOffset+1) = (dstType)((*((GLfloat *)tmpBuf+i+3)) * (GLfloat)(maxValue) + 0.5f);\
        }\
    }

#define __GL_L_FINALCONVERSION_2_SIGNED(dstType, maxValue) \
    {\
        writeOffset = (3 == components) ? (i/components) : ((i/components)*2);\
        *((dstType *)outBuf+writeOffset) = (dstType)(gcdOFFSET_O_DOT_5(((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) - 1.0f) / 2.0f));\
        if (4 == components)\
        {\
            *((dstType *)outBuf+writeOffset+1) = (dstType)(gcdOFFSET_O_DOT_5(((*((GLfloat *)tmpBuf+i+3)) * (GLfloat)(maxValue) - 1.0f) / 2.0f));\
        }\
    }

#define __GL_L_FINALCONVERSION_2_FLOAT() \
    {\
        writeOffset = (3 == components) ? (i/components) : ((i/components)*2);\
        *((GLfloat *)outBuf+writeOffset) = *((GLfloat *)tmpBuf+i);\
        if (4 == components)\
        {\
            *((GLfloat *)outBuf+writeOffset+1) = *((GLfloat *)tmpBuf+i+3);\
        }\
    }

#define __GL_L_FINALCONVERSION_2_HALF_FLOAT() \
    {\
        writeOffset = (3 == components) ? (i/components) : ((i/components)*2);\
        *((GLushort *)outBuf+writeOffset) = (GLushort)gcoMATH_FloatToFloat16(*((GLuint *)tmpBuf+i));\
        if (4 == components)\
        {\
            *((GLushort *)outBuf+writeOffset+1) = (GLushort)gcoMATH_FloatToFloat16(*((GLuint *)tmpBuf+i+3));\
        }\
    }

#define __GL_I_FINALCONVERSION_2_UNSIGNED(dstType, maxValue) \
    {\
        *((dstType *)outBuf+i/components) = (dstType)((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) + 0.5f);\
    }

#define __GL_I_FINALCONVERSION_2_SIGNED(dstType, maxValue) \
    {\
        *((dstType *)outBuf+i/components) = (dstType)(gcdOFFSET_O_DOT_5(((*((GLfloat *)tmpBuf+i)) * (GLfloat)(maxValue) - 1.0f) / 2.0f));\
    }

#define __GL_I_FINALCONVERSION_2_FLOAT() \
    {\
        *((GLfloat *)outBuf+i/components) = *((GLfloat *)tmpBuf+i);\
    }

#define __GL_I_FINALCONVERSION_2_HALF_FLOAT() \
    {\
        *((GLushort *)outBuf+i/components) = (GLushort)gcoMATH_FloatToFloat16(*((GLuint *)tmpBuf+i));\
    }

GLvoid __glCheckSpcecialType(GLenum baseFmt, GLenum *srcType, GLenum dstType)
{
    switch(*srcType)
    {
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
            if ((GL_RGB != baseFmt) && (GL_RGB_INTEGER != baseFmt) && (GL_BGR != baseFmt) && (GL_BGR_INTEGER != baseFmt))
            {
                *srcType = dstType;
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            if ((GL_RGBA != baseFmt) && (GL_RGBA_INTEGER != baseFmt) && (GL_BGRA != baseFmt) && (GL_BGRA_INTEGER != baseFmt))
            {
                *srcType = dstType;
            }
            break;
        default:
            break;
    }
}

GLboolean __glCheckSpecialFormat(GLenum internalFormat, GLenum format, GLenum* type)
{
    switch(format)
    {
        case GL_RED_INTEGER_EXT:
        case GL_BLUE_INTEGER_EXT:
        case GL_GREEN_INTEGER_EXT:
        case GL_ALPHA_INTEGER_EXT:
        case GL_RGB_INTEGER_EXT:
        case GL_RGBA_INTEGER_EXT:
        case GL_BGR_INTEGER_EXT:
        case GL_BGRA_INTEGER_EXT:
        case GL_LUMINANCE_INTEGER_EXT:
        case GL_LUMINANCE_ALPHA_INTEGER_EXT:
            switch (internalFormat)
            {
                case GL_RGBA32UI_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB10_A2UI:
                case GL_RGB8UI_EXT:
                case GL_RG32UI:
                case GL_RG16UI:
                case GL_RG8UI:
                case GL_R32UI:
                case GL_R16UI:
                case GL_R8UI:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY8UI_EXT:
                    //format is INTEGER
                    if (*type == GL_INT){
                        *type = GL_UNSIGNED_INT;
                    }
                    else if (*type == GL_SHORT){
                        *type = GL_UNSIGNED_SHORT;
                    }
                    else if (*type == GL_BYTE){
                        *type = GL_UNSIGNED_BYTE;
                    }
                    break;
                case GL_RGBA32I_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8I_EXT:
                case GL_RG32I:
                case GL_RG16I:
                case GL_RG8I:
                case GL_R32I:
                case GL_R16I:
                case GL_R8I:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8I_EXT:
                    //format is INTEGER
                    if (*type == GL_UNSIGNED_INT){
                        *type = GL_INT;
                    }
                    else if (*type == GL_UNSIGNED_SHORT){
                        *type = GL_SHORT;
                    }
                    else if (*type == GL_UNSIGNED_BYTE){
                        *type = GL_BYTE;
                    }
                    break;
                default:
                    break;
            }
            return GL_TRUE;
        case GL_COLOR_INDEX:
        case GL_STENCIL_INDEX:
        case GL_DEPTH_STENCIL:
        case GL_DEPTH_COMPONENT:
            return GL_TRUE;
        default:
            break;
    }
    return GL_FALSE;
}

GLboolean __glConvertL2RGB(GLenum type,GLsizei numOfPixel, GLubyte components,GLfloat* tmpBuf, const GLvoid* buf)
{
    GLsizei i = 0, j = 0, readOffset = 0;
    switch (type)
    {
        case GL_BYTE:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_L_2_RGB(GLbyte, __glMaxUbyte);
                }
            }
            break;
        case GL_SHORT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_L_2_RGB(GLshort, __glMaxUshort);
                }
            }
            break;
        case GL_INT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_L_2_RGB(GLint, __glMaxUint);
                }
            }
            break;
        case GL_UNSIGNED_BYTE:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_L_2_RGB(GLubyte, __glMaxUbyte);
                }
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_L_2_RGB(GLushort, __glMaxUshort);
                }
            }
            break;
        case GL_UNSIGNED_INT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_L_2_RGB(GLuint, __glMaxUint);
                }
            }
            break;
        case GL_FLOAT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_FLOAT_L_2_RGB();
                }
            }
            break;
        case GL_HALF_FLOAT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_HALF_FLOAT_L_2_RGB();
                }
            }
            break;

        default:
            return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean __glConvertI2RGBA(GLenum type,GLsizei numOfPixel, GLubyte components,GLfloat* tmpBuf, const GLvoid* buf)
{
    GLsizei i = 0, j = 0;
    switch (type)
    {
        case GL_BYTE:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_I_2_RGBA(GLbyte, __glMaxUbyte);
                }
            }
            break;
        case GL_SHORT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_I_2_RGBA(GLshort, __glMaxUshort);
                }
            }
            break;
        case GL_INT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_SIGNED_I_2_RGBA(GLint, __glMaxUint);
                }
            }
            break;
        case GL_UNSIGNED_BYTE:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_I_2_RGBA(GLubyte, __glMaxUbyte);
                }
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_I_2_RGBA(GLushort, __glMaxUshort);
                }
            }
            break;
        case GL_UNSIGNED_INT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_UNSIGNED_I_2_RGBA(GLuint, __glMaxUint);
                }
            }
            break;
        case GL_FLOAT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_FLOAT_I_2_RGBA();
                }
            }
            break;
        case GL_HALF_FLOAT:
            for(i=0; i<numOfPixel; i++)
            {
                for(j=0; j<components; j++)
                {
                    __GL_CONVERT_HALF_FLOAT_I_2_RGBA();
                }
            }
            break;

        default:
            return GL_TRUE;
    }
    return GL_FALSE;
}

GLboolean __glConvert2Float(GLenum type,GLsizei numOfComponent, GLubyte components,GLfloat* tmpBuf, const GLvoid* buf)
{
    GLsizei i = 0;
    GLuint tmpUint32 = 0;

    switch (type)
    {
        case GL_BYTE:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_SIGNED_2_FLOAT(GLbyte, __glMaxByte);
            }
            break;
        case GL_SHORT:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_SIGNED_2_FLOAT(GLshort, __glMaxShort);
            }
            break;
        case GL_INT:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_SIGNED_2_FLOAT(GLint, __glMaxInt);
            }
            break;
        case GL_UNSIGNED_BYTE:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_UNSIGNED_2_FLOAT(GLubyte, __glMaxUbyte);
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_UNSIGNED_2_FLOAT(GLushort, __glMaxUshort);
            }
            break;
        case GL_UNSIGNED_INT:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_UNSIGNED_2_FLOAT(GLuint, __glMaxUint);
            }
            break;
        case GL_FLOAT:
            gcoOS_MemCopy(tmpBuf, buf, numOfComponent * sizeof(GLfloat));
            break;
        case GL_HALF_FLOAT:
            for(i=0; i<numOfComponent; i++)
            {
                __GL_CONVERT_HALF_FLOAT_2_FLOAT();
            }
            break;
        case GL_UNSIGNED_BYTE_3_3_2:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLubyte*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 5, 3);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 2, 3);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 0, 2);
            }
            break;
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLubyte*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0, 3);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 3, 3);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 6, 2);
            }
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 11, 5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 5,  6);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 0,  5);
            }
            break;
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0,  5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 5,  6);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 11, 5);
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 12, 4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 8,  4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 4,  4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 0,  4);
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0,  4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 4,  4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 8,  4);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 12, 4);
            }
            break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 11, 5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 6,  5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 1,  5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 0,  1);
            }
            break;
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLushort*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0,  5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 5,  5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 10, 5);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 15, 1);
            }
            break;
        case GL_UNSIGNED_INT_8_8_8_8:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 24, 8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 16, 8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 8,  8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 0,  8);
            }
            break;
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0,  8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 8,  8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 16, 8);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 24, 8);
            }
            break;
        case GL_UNSIGNED_INT_10_10_10_2:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 22, 10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 12, 10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 2,  10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 0,  2);
            }
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            GL_ASSERT(4 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(0, 0,  10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(1, 10, 10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(2, 20, 10);
                __GL_CONVERT_UNSIGNED_2_FLOAT_SPECIAL(3, 30, 2);
            }
            break;
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                GLuint or=0, og=0, ob=0;
                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));

                or = gcoMATH_Float11ToFloat(gcdGET_FIELD(tmpUint32, 0, 11));
                og = gcoMATH_Float11ToFloat(gcdGET_FIELD(tmpUint32, 11, 11));
                ob = gcoMATH_Float10ToFloat(gcdGET_FIELD(tmpUint32, 22, 10));

                *((GLfloat *)tmpBuf+i) = *(GLfloat*)&or;
                *((GLfloat *)tmpBuf+i+1) = *(GLfloat*)&og;
                *((GLfloat *)tmpBuf+i+2) = *(GLfloat*)&ob;
            }
            break;
        case GL_UNSIGNED_INT_5_9_9_9_REV:
            GL_ASSERT(3 == components);
            for(i=0; i<numOfComponent; i+=components)
            {
                const GLuint mBits = 9;      /* mantissa bits */
                const GLuint eBias = 15;     /* max allowed bias exponent */
                GLfloat scale =0.0;

                tmpUint32 = (GLuint)(*(GLuint*)buf+(i/components));
                scale = gcoMATH_Power(2.0f, (GLfloat)gcdGET_FIELD(tmpUint32, 27, 5) - eBias - mBits);

                *((GLfloat *)tmpBuf+i) = (GLfloat)gcdGET_FIELD(tmpUint32, 0,  9) * scale;
                *((GLfloat *)tmpBuf+i+1) = (GLfloat)gcdGET_FIELD(tmpUint32, 9,  9) * scale;
                *((GLfloat *)tmpBuf+i+2) = (GLfloat)gcdGET_FIELD(tmpUint32, 18, 9) * scale;
             }
            break;

        default:
            return GL_TRUE;
    }
    return GL_FALSE;
}

GLvoid __glScaleAndBias(GLsizei numOfComponent, GLubyte components, GLfloat* tmpBuf, GLfloat rgbaScale[4], GLfloat rgbaBias[4], GLubyte mask[4])
{
    int i, j;

    for(i = 0; i < numOfComponent; ){
        for(j = 0; j < components; j++)
        {
            *(tmpBuf+i) = *(tmpBuf+i) * rgbaScale[mask[j]-1];
            *(tmpBuf+i) = *(tmpBuf+i) + rgbaBias[mask[j]-1];
            i++;
        }
    }
}

GLvoid __glClamp2ZeroOne(GLsizei numOfComponent, GLubyte components, GLfloat* tmpBuf)
{
    int i, j;
    for(i = 0; i < numOfComponent; ){
        for(j = 0; j < components; j++)
        {
            *(tmpBuf+i) = gcmCLAMP(*(tmpBuf+i), __glZero, __glOne);
            i++;
        }
    }
}

GLint _FloorLog2(GLfloat val)
{
    GLint exp = 0;
    gctUINT64 base = 1;

    while (val >= (GLfloat)base)
    {
        base = (gctUINT64)1 << (++exp);
    }

    return exp - 1;
}


GLvoid __glFinalConversionForL(GLenum interType, GLenum* type, GLsizei numOfComponent, GLubyte components, GLfloat* tmpBuf, GLvoid * outBuf)
{
    GLsizei i = 0, writeOffset = 0;

    switch (interType)
    {
        case GL_BYTE:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_SIGNED(GLbyte, __glMaxUbyte);
            }
            break;
        case GL_SHORT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_SIGNED(GLshort, __glMaxUshort);
            }
            break;
        case GL_INT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_SIGNED(GLint, __glMaxUint);
            }
        case GL_UNSIGNED_BYTE:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_UNSIGNED(GLubyte, __glMaxUbyte);
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_UNSIGNED(GLushort, __glMaxUshort);
            }
            break;
        case GL_UNSIGNED_INT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_UNSIGNED(GLuint, __glMaxUint);
            }
            break;
        case GL_FLOAT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_FLOAT();
            }
            break;
        case GL_HALF_FLOAT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_L_FINALCONVERSION_2_HALF_FLOAT();
            }
            break;

        default:
            return;
    }

    /* Change type after convert data from tmpBuf to outBuf */
    *type = interType;
}

GLvoid __glFinalConversionForI(GLenum interType, GLenum* type, GLsizei numOfComponent, GLubyte components, GLfloat* tmpBuf, GLvoid * outBuf)
{
    GLsizei i = 0;

    switch (interType)
    {
        case GL_BYTE:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_SIGNED(GLbyte, __glMaxUbyte);
            }
            break;
        case GL_SHORT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_SIGNED(GLshort, __glMaxUshort);
            }
            break;
        case GL_INT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_SIGNED(GLint, __glMaxUint);
            }
        case GL_UNSIGNED_BYTE:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_UNSIGNED(GLubyte, __glMaxUbyte);
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_UNSIGNED(GLushort, __glMaxUshort);
            }
            break;
        case GL_UNSIGNED_INT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_UNSIGNED(GLuint, __glMaxUint);
            }
            break;
        case GL_FLOAT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_FLOAT();
            }
            break;
        case GL_HALF_FLOAT:
            for(i=0; i<numOfComponent; i+=components)
            {
                __GL_I_FINALCONVERSION_2_HALF_FLOAT();
            }
            break;

        default:
            return;
    }

    /* Change type after convert data from tmpBuf to outBuf */
    *type = interType;
}

GLvoid __glFinalConversion(GLenum interType, GLenum* type, GLsizei numOfComponent, GLubyte components, GLfloat* tmpBuf, GLvoid * outBuf)
{
    int i;
    GLuint tmpUint32 = 0;

    /* Add convert to L */

    switch (interType)
    {
        case GL_BYTE:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_SIGNED(GLbyte, __glMaxUbyte);
            }
            break;
        case GL_SHORT:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_SIGNED(GLshort, __glMaxUshort);
            }
            break;
        case GL_INT:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_SIGNED(GLint, __glMaxUint);
            }
            break;
        case GL_UNSIGNED_BYTE:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_UNSIGNED(GLubyte, __glMaxUbyte);
            }
            break;
        case GL_UNSIGNED_SHORT:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_UNSIGNED(GLushort, __glMaxUshort);
            }
            break;
        case GL_UNSIGNED_INT:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_UNSIGNED(GLuint, __glMaxUint);
            }
            break;
        case GL_FLOAT:
            gcoOS_MemCopy(outBuf, tmpBuf, numOfComponent * sizeof(GLfloat));
            break;
        case GL_HALF_FLOAT:
            for(i = 0; i < numOfComponent; i++)
            {
                __GL_FINALCONVERSION_2_HALF_FLOAT();
            }
            break;
        case GL_UNSIGNED_BYTE_3_3_2:
            GL_ASSERT(3 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 5, 3);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 2, 3);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 0, 2);
                *((GLubyte *)outBuf + (i/components)) = (GLubyte)(tmpUint32 & gcdUINT_MAX(8));
            }
            break;
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            GL_ASSERT(3 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0, 3);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 3, 3);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 6, 2);
                *((GLubyte *)outBuf + (i/components)) = (GLubyte)(tmpUint32 & gcdUINT_MAX(8));
            }
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
            GL_ASSERT(3 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 11, 5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 5,  6);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 0,  5);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            GL_ASSERT(3 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0,  5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 5,  6);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 11, 5);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 12, 4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 8,  4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 4,  4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 0,  4);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0,  4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 4,  4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 8,  4);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 12, 4);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 11, 5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 6,  5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 1,  5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 0,  1);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0,  5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 5,  5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 10, 5);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 15, 1);
                *((GLushort *)outBuf + (i/components)) = (GLushort)(tmpUint32 & gcdUINT_MAX(16));
            }
            break;
        case GL_UNSIGNED_INT_8_8_8_8:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 24, 8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 16, 8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 8,  8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 0,  8);
                *((GLuint *)outBuf + (i/components)) = (GLuint)(tmpUint32 & gcdUINT_MAX(32));
            }
            break;
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0,  8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 8,  8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 16, 8);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 24, 8);
                *((GLuint *)outBuf + (i/components)) = (GLuint)(tmpUint32 & gcdUINT_MAX(32));
            }
            break;
        case GL_UNSIGNED_INT_10_10_10_2:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 22, 10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 12, 10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 2,  10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 0,  2);
                *((GLuint *)outBuf + (i/components)) = (GLuint)(tmpUint32 & gcdUINT_MAX(32));
            }
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            GL_ASSERT(4 == components);
            for(i = 0; i < numOfComponent; i+=components)
            {
                tmpUint32 = 0;
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(0, 0,  10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(1, 10, 10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(2, 20, 10);
                __GL_FINALCONVERSION_2_UNSIGNED_SPECIAL(3, 30, 2);
                *((GLuint *)outBuf + (i/components)) = (GLuint)(tmpUint32 & gcdUINT_MAX(32));
            }
            break;
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            GL_ASSERT(3 == components);
            for(i = 0; i<numOfComponent; i+=components)
            {
                GLshort r=0,g=0,b=0;

                r = (GLushort)gcoMATH_FloatToFloat11(*(GLuint*)((GLfloat*)tmpBuf+i));
                g = (GLushort)gcoMATH_FloatToFloat11(*(GLuint*)((GLfloat*)tmpBuf+i+1));
                b = (GLushort)gcoMATH_FloatToFloat10(*(GLuint*)((GLfloat*)tmpBuf+i+2));

                *((GLuint *)outBuf + (i/components)) = (b << 22) | (g << 11) | r;
            }
            break;
        case GL_UNSIGNED_INT_5_9_9_9_REV:
            GL_ASSERT(3 == components);
            for(i = 0; i<numOfComponent; i+=components)
            {
                const GLint mBits = 9;
                const GLint eBits = 5;
                const GLint eBias = 15;
                const GLint eMax  = gcdUINT_MAX(eBits);
                const GLfloat sharedExpMax = gcdUINT_MAX(mBits)* (1 << (eMax - eBias)) / (GLfloat)(1 << mBits);

                GLfloat rc   = gcmCLAMP(*((GLfloat *)tmpBuf+i), 0.0f, sharedExpMax);
                GLfloat gc   = gcmCLAMP(*((GLfloat *)tmpBuf+i+1), 0.0f, sharedExpMax);
                GLfloat bc   = gcmCLAMP(*((GLfloat *)tmpBuf+i+2), 0.0f, sharedExpMax);
                GLfloat maxc = gcoMATH_MAX(rc, gcoMATH_MAX(gc, bc));

                GLint expp  = gcoMATH_MAX(-eBias - 1, _FloorLog2(maxc)) + 1 + eBias;
                GLfloat scale = (gctFLOAT)gcoMATH_Power(2.0f, (gctFLOAT)(expp - eBias - mBits));
                GLint maxs  = (GLint)(maxc / scale + 0.5f);

                GLuint exps = (maxs == (1 << mBits)) ? (GLuint)(expp + 1) : (GLuint)expp;
                GLuint rs = gcmMIN((GLuint)(rc / scale + 0.5f), gcdUINT_MAX(9));
                GLuint gs = gcmMIN((GLuint)(gc / scale + 0.5f), gcdUINT_MAX(9));
                GLuint bs = gcmMIN((GLuint)(bc / scale + 0.5f), gcdUINT_MAX(9));

                *((GLuint *)outBuf + (i/components)) = rs | (gs << 9) | (bs << 18) | (exps << 27);
            }
            break;

        default:
            return;
    }

    /* Change type after convert data from tmpBuf to outBuf */
    *type = interType;
}

GLvoid __glClearAlignmentPlaceOfBuffer(__GLcontext *gc, __GLpixelTransferInfo *transferInfo, GLvoid **srcBuf)
{
    GLuint i, j;

    if ((transferInfo->srcSizeOfPixel * transferInfo->width) % transferInfo->alignment)
    {
        (*srcBuf) = (GLfloat *)(*gc->imports.malloc)(gc, transferInfo->numOfPixel * transferInfo->srcSizeOfPixel * sizeof(GLubyte));
        gcoOS_MemCopy((GLubyte *)(*srcBuf), (GLubyte *)transferInfo->srcImage, transferInfo->numOfPixel * transferInfo->srcSizeOfPixel);

        for (i=0, j=0; i < transferInfo->numOfPixel * transferInfo->srcSizeOfPixel; i++, j++)
        {
            while (j % (transferInfo->width * transferInfo->srcSizeOfPixel + transferInfo->numOfAlignSrc) == transferInfo->width * transferInfo->srcSizeOfPixel)
            {
                j = j + transferInfo->numOfAlignSrc;
            }

            if (j < transferInfo->numOfPixel * transferInfo->srcSizeOfPixel)
            {
                 *((GLubyte *)transferInfo->srcImage+i) = *((GLubyte *)transferInfo->srcImage + j);
            }
        }
        transferInfo->numOfComponents = transferInfo->numOfComponents / transferInfo->widthAlign * transferInfo->width;
    }
}

GLvoid __glAddAlignmentPlaceOfBuffer(__GLcontext *gc, __GLpixelTransferInfo *transferInfo, GLvoid *alignmentBuf)
{
    GLuint i, j;

    if (transferInfo->numOfAlign)
    {
        alignmentBuf = (*gc->imports.malloc)(gc, transferInfo->sizeOfAlignMemory * sizeof(GLubyte));

        for(i=0, j=0; j < transferInfo->sizeOfAlignMemory; i++, j++)
        {
            if (j % (transferInfo->dstWidthAlign * transferInfo->dstSizeOfPixel) == transferInfo->width * transferInfo->dstSizeOfPixel)
            {
                j = j + transferInfo->numOfAlign;
            }

            if (j < transferInfo->sizeOfAlignMemory)
            {
                *((GLubyte *)alignmentBuf + j) = *((GLubyte *)transferInfo->dstImage + i);
            }
        }

        gcoOS_MemCopy((GLubyte *)transferInfo->dstImage, (GLubyte *)alignmentBuf, transferInfo->sizeOfAlignMemory);
        if (alignmentBuf != gcvNULL)
        {
            (*gc->imports.free)(gc, (void*)alignmentBuf);
            alignmentBuf = gcvNULL;
        }
    }
}

GLvoid __glConvertToFloatOfBufferType(__GLcontext *gc, GLenum format, GLenum *type, const GLvoid* buf, __GLpixelTransferInfo *transferInfo, GLsizei width, GLsizei height, GLsizei depth)
{
    if (buf != gcvNULL)
    {
        GLuint i;
        GLfloat *tmpBuf = gcvNULL;
        GLuint tmpUint32;

        transferInfo->height = height;
        transferInfo->depth = depth;
        transferInfo->width = width;

        transferInfo->alignment = gc->clientState.pixel.packModes.alignment;
        __glMemoryAlignment(format, *type, *type, transferInfo, __GL_TexImage);
        transferInfo->numOfPixel = height * depth * transferInfo->widthAlign;
        transferInfo->numOfComponents = transferInfo->numOfPixel * transferInfo->compNumOfElement;
        tmpBuf = (GLfloat *)(*gc->imports.malloc)(gc, transferInfo->numOfComponents * sizeof(GLfloat));

        switch (*type)
        {
           case GL_UNSIGNED_BYTE:
                for (i=0; i<transferInfo->numOfComponents; i++)
                {
                    __GL_CONVERT_UNSIGNED_2_FLOAT(GLubyte, __glMaxUbyte);
                }
                break;
           case GL_BYTE:
                for (i=0; i<transferInfo->numOfComponents; i++)
                {
                    __GL_CONVERT_SIGNED_2_FLOAT(GLbyte, __glMaxUbyte);
                }
                break;
           case GL_SHORT:
                for (i=0; i<transferInfo->numOfComponents; i++)
                {
                    __GL_CONVERT_SIGNED_2_FLOAT(GLshort, __glMaxUshort);
                }
                break;
           case GL_INT:
                for (i=0; i<transferInfo->numOfComponents; i++)
                {
                    __GL_CONVERT_SIGNED_2_FLOAT(GLint, __glMaxUint);
                }
                break;
           case GL_HALF_FLOAT:
                for (i=0; i<transferInfo->numOfComponents; i++)
                {
                    tmpUint32=0;
                    __GL_CONVERT_HALF_FLOAT_2_FLOAT();
                }
                break;
           default:
                break;
        }

        (*type)=GL_FLOAT;
        transferInfo->srcImage = buf;
        transferInfo->dstImage = tmpBuf;
        transferInfo->dstNeedFree = GL_TRUE;
    }
    else
    {
        transferInfo->srcImage = buf;
        transferInfo->dstImage = buf;
    }
}

/* A GenericPixelTransfer function template*/
GLvoid __glGenericPixelTransferSub(__GLcontext *gc,
                                        __GLpixelTransferInfo *transferInfo,
                                        GLenum *type)
{
    GLfloat *tmpBuf = gcvNULL;
    GLvoid  *srcBuf = gcvNULL;
    GLvoid  *alignmentBuf = gcvNULL;
    GLenum baseFormat = transferInfo->baseFormat;

    if ((gcvNULL == transferInfo->srcImage) || (0 == transferInfo->numOfComponents))
    {
        __GL_EXIT();
    }

    /* clear alignment place in the buffer before conversion*/
    __glClearAlignmentPlaceOfBuffer(gc, transferInfo, &srcBuf);

    tmpBuf = (GLfloat *)(*gc->imports.malloc)(gc, transferInfo->numOfComponents * sizeof(GLfloat));
    if (gcvNULL == tmpBuf)
    {
        __GL_EXIT();
    }

    if ((baseFormat == GL_LUMINANCE) || (baseFormat == GL_LUMINANCE_ALPHA))
    {
        if (__glConvertL2RGB(transferInfo->srcType, transferInfo->numOfPixel, transferInfo->compNumber, tmpBuf, transferInfo->srcImage))
        {
            __GL_EXIT();
        }
    }
    else if (baseFormat == GL_INTENSITY)
    {
        if (__glConvertI2RGBA(transferInfo->srcType, transferInfo->numOfPixel, transferInfo->compNumber, tmpBuf, transferInfo->srcImage))
        {
            __GL_EXIT();
        }
    }
    else
    {
        if (__glConvert2Float(transferInfo->srcType, transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf, transferInfo->srcImage))
        {
            __GL_EXIT();
        }
    }

    /* Scale, Bias, Clamp */
    if (transferInfo->applyGenericScaleBias)
    {
        __glScaleAndBias(transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf, (GLfloat *)(&(transferInfo->scale)), (GLfloat *)(&(transferInfo->bias)), &(transferInfo->compMask[0]));
    }
    __glClamp2ZeroOne(transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf);


    if ((baseFormat == GL_LUMINANCE) || (baseFormat == GL_LUMINANCE_ALPHA))
    {
        __glFinalConversionForL(transferInfo->dstType, type, transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf, (GLvoid *)(transferInfo->dstImage));
    }
    else if (baseFormat == GL_INTENSITY)
    {
        __glFinalConversionForI(transferInfo->dstType, type, transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf, (GLvoid *)(transferInfo->dstImage));
    }
    else
    {
        __glFinalConversion(transferInfo->dstType, type, transferInfo->numOfComponents, transferInfo->compNumber, tmpBuf, (GLvoid *)(transferInfo->dstImage));
    }

    /* add alignment place in the buffer after conversion */
    __glAddAlignmentPlaceOfBuffer(gc, transferInfo, alignmentBuf);

OnExit:
    if (tmpBuf != gcvNULL){
        (*gc->imports.free)(gc, (void*)tmpBuf);
        tmpBuf = gcvNULL;
    }

    if (srcBuf != gcvNULL)
    {
         gcoOS_MemCopy((GLubyte *)transferInfo->srcImage, (GLubyte *)srcBuf, transferInfo->numOfPixel * transferInfo->srcSizeOfPixel);
        (*gc->imports.free)(gc, (void*)srcBuf);
        srcBuf = gcvNULL;
    }
}


/* A GenericPixelTransfer function template*/
GLvoid __glGenericPixelTransfer(__GLcontext *gc,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth,
                                __GLformatInfo *formatInfo,
                                GLenum format,
                                GLenum *type,
                                const GLvoid *buf,
                                __GLpixelTransferInfo *transferInfo,
                                GLenum pixelTransferOperations)
{
    GLint internalFormat = 0;
    GLvoid *interBuf = gcvNULL;
    GLenum baseFmt = 0;
    GLenum inType = 0;
    GLenum outType = 0;

    if(!gc->imports.conformGLSpec)
    {
        __GL_EXIT();
    }

    if ((gcvNULL == buf) || (gcvNULL == formatInfo) || (gcvNULL == transferInfo))
    {
        if ((gcvNULL != formatInfo) && (*type != formatInfo->dataType))
        {
            *type = formatInfo->dataType;
        }
        __GL_EXIT();
    }

    if (pixelTransferOperations == __GL_TexImage)// Tex
    {
        internalFormat = formatInfo->glFormat;
        baseFmt = format;
        inType = *type;
        outType = formatInfo->dataType;

        /* When format is integer/index/depth/stencil */
        if (__glCheckSpecialFormat(internalFormat, baseFmt, type))
        {
            __GL_EXIT();
        }

        if (GL_FALSE == __glInitTransferInfo(gc, width, height, depth, transferInfo, baseFmt, inType, outType, __GL_TexImage))
        {
            __GL_EXIT();
        }

        interBuf  = (gc->imports.malloc)(gc, transferInfo->sizeOfAlignMemory);
        if (gcvNULL == interBuf)
        {
            __GL_EXIT();
        }
        transferInfo->srcImage = buf;
        transferInfo->dstImage = interBuf;
        transferInfo->dstNeedFree = GL_TRUE;

        __glGenericPixelTransferSub(gc, transferInfo, type);
    }
    else if (pixelTransferOperations == __GL_ReadPixelsPre)// Read
    {
        internalFormat = formatInfo->glFormat;
        baseFmt = format;
        inType = formatInfo->dataType;
        outType = *type;

        /* When format is integer/index/depth/stencil */
        if (__glCheckSpecialFormat(internalFormat, baseFmt, type))
        {
            __GL_EXIT();
        }

        __glCheckSpcecialType(baseFmt, &inType, outType);

        if (GL_FALSE == __glInitTransferInfo(gc, width, height, depth, transferInfo, baseFmt, inType, outType, __GL_ReadPixelsPre))
        {
            __GL_EXIT();
        }

        interBuf = (gc->imports.malloc)(gc, transferInfo->numOfPixel * transferInfo->srcSizeOfPixel);
        if (gcvNULL == interBuf)
        {
            __GL_EXIT();
        }
        transferInfo->srcImage = interBuf;
        transferInfo->srcNeedFree = GL_TRUE;
        transferInfo->dstImage = buf;
        *type = transferInfo->srcType;
        return;
    }
    else if (pixelTransferOperations == __GL_ReadPixels)
    {
        if (GL_FALSE == transferInfo->applyPixelTransfer)
        {
            __GL_EXIT();
        }

        __glGenericPixelTransferSub(gc, transferInfo, type);
    }

OnExit:
    if (gcvNULL == interBuf)
    {
        if (pixelTransferOperations == __GL_TexImage)
        {
            if ((format==GL_DEPTH_COMPONENT)&&((*type)==GL_UNSIGNED_BYTE||(*type)==GL_BYTE||(*type)==GL_SHORT||(*type)==GL_INT||(*type)==GL_HALF_FLOAT))
            {
                __glConvertToFloatOfBufferType(gc, format, type, buf, transferInfo, width, height, depth);
            }
            else
            {
                transferInfo->srcImage = buf;
                transferInfo->dstImage = buf;
            }
        }
        else if (pixelTransferOperations == __GL_ReadPixelsPre)
        {
            transferInfo->srcImage = buf;
            transferInfo->dstImage = buf;
            transferInfo->applyPixelTransfer = GL_FALSE;
        }
    }
}

