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
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    __GLES3_ZONE_CODEC

/*************************************************************************
**
** SW Decompress Functions
**
*************************************************************************/

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/


/*************************************************************************
** ETC1 Decompress
*************************************************************************/

static void
gcChipDecodeETC1Block(
    GLubyte * Output,
    gctSIZE_T Stride,
    gctSIZE_T Width,
    gctSIZE_T Height,
    const GLubyte * Data
    )
{
    GLubyte base[2][3];
    GLboolean flip, diff;
    GLbyte index[2];
    gctSIZE_T i, j, x, y, offset;
    static GLubyte table[][2] =
    {
        {  2,   8 },
        {  5,  17 },
        {  9,  29 },
        { 13,  42 },
        { 18,  60 },
        { 24,  80 },
        { 33, 106 },
        { 47, 183 },
    };
    gcmHEADER_ARG("Output=0x%x Stride=%d Width=%d Height=%d Data=0x%x",
                  Output, Stride, Width, Height, Data);

    diff = Data[3] & 0x2;
    flip = Data[3] & 0x1;

    if (diff)
    {
        GLbyte delta[3];

        /* Need to extend sign bit to entire byte. */
        delta[0] = (GLbyte)((Data[0] & 0x7) << 5) >> 5;
        delta[1] = (GLbyte)((Data[1] & 0x7) << 5) >> 5;
        delta[2] = (GLbyte)((Data[2] & 0x7) << 5) >> 5;

        base[0][0] = Data[0] >> 3;
        base[0][1] = Data[1] >> 3;
        base[0][2] = Data[2] >> 3;

        base[1][0] = base[0][0] + delta[0];
        base[1][1] = base[0][1] + delta[1];
        base[1][2] = base[0][2] + delta[2];

        base[0][0] = (base[0][0] << 3) | (base[0][0] >> 2);
        base[0][1] = (base[0][1] << 3) | (base[0][1] >> 2);
        base[0][2] = (base[0][2] << 3) | (base[0][2] >> 2);

        base[1][0] = (base[1][0] << 3) | (base[1][0] >> 2);
        base[1][1] = (base[1][1] << 3) | (base[1][1] >> 2);
        base[1][2] = (base[1][2] << 3) | (base[1][2] >> 2);
    }
    else
    {
        base[0][0] = (Data[0] & 0xF0) | (Data[0] >> 4  );
        base[0][1] = (Data[1] & 0xF0) | (Data[1] >> 4  );
        base[0][2] = (Data[2] & 0xF0) | (Data[2] >> 4  );
        base[1][0] = (Data[0] << 4  ) | (Data[0] & 0x0F);
        base[1][1] = (Data[1] << 4  ) | (Data[1] & 0x0F);
        base[1][2] = (Data[2] << 4  ) | (Data[2] & 0x0F);
    }

    index[0] = (Data[3] & 0xE0) >> 5;
    index[1] = (Data[3] & 0x1C) >> 2;

    for (i = x = y = offset = 0; i < 2; ++i)
    {
        GLubyte msb = Data[5 - i];
        GLubyte lsb = Data[7 - i];

        for (j = 0; j < 8; ++j)
        {
            GLuint delta = 0;
            GLint r, g, b;
            GLint block = flip
                        ? (y < 2) ? 0 : 1
                        : (x < 2) ? 0 : 1;

            switch (((msb & 1) << 1) | (lsb & 1))
            {
            case 0x3: delta = -table[index[block]][1]; break;
            case 0x2: delta = -table[index[block]][0]; break;
            case 0x0: delta =  table[index[block]][0]; break;
            case 0x1: delta =  table[index[block]][1]; break;
            }

            r = base[block][0] + delta; r = __GL_MAX(0x00, __GL_MIN(r, 0xFF));
            g = base[block][1] + delta; g = __GL_MAX(0x00, __GL_MIN(g, 0xFF));
            b = base[block][2] + delta; b = __GL_MAX(0x00, __GL_MIN(b, 0xFF));

            if ((x < Width) && (y < Height))
            {
                Output[offset + 0] = (GLubyte) r;
                Output[offset + 1] = (GLubyte) g;
                Output[offset + 2] = (GLubyte) b;
            }

            offset += Stride;
            if (++y == 4)
            {
                y = 0;
                ++x;

                offset += 3 - 4 * Stride;
            }

            msb >>= 1;
            lsb >>= 1;
        }
    }
    gcmFOOTER_NO();
}

/*************************************************************************
** DXT Decompress
*************************************************************************/

#define __GL_DXT_RED(c)   ( (c) >> 11 )
#define __GL_DXT_GREEN(c) ( ((c) >> 5) & 0x3F )
#define __GL_DXT_BLUE(c)  ( (c) & 0x1F )

#define __GL_DXT_EXPAND_RED(c)   ( (((c) & 0xF800) << 8) | (((c) & 0xE000) << 3) )
#define __GL_DXT_EXPAND_GREEN(c) ( (((c) & 0x07E0) << 5) | (((c) & 0x0600) >> 1) )
#define __GL_DXT_EXPAND_BLUE(c)  ( (((c) & 0x001F) << 3) | (((c) & 0x001C) >> 2) )

/* Decode 64-bits of color information. */
static void
gcChipDecodeDXTColor16(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    OUT GLubyte * Output
    )
{
    GLushort c0, c1;
    GLushort color[4];
    GLushort r, g, b;
    gctSIZE_T x, y;
    gcmHEADER_ARG("Width=%d Height=%d Stride=%d Data=0x%x Output=0x%x",
                  Width, Height, Stride, Data, Output);

    /* Decode color 0. */
    c0 = *(GLushort*)Data;
    color[0] = 0x8000 | (c0 & 0x001F) | ((c0 & 0xFFC0) >> 1);

    /* Decode color 1. */
    c1 = *(GLushort*)(Data + 2);
    color[1] = 0x8000 | (c1 & 0x001F) | ((c1 & 0xFFC0) >> 1);

    if (c0 > c1)
    {
        /* Compute color 2: (c0 * 2 + c1) / 3. */
        r = (2 * __GL_DXT_RED  (c0) + __GL_DXT_RED  (c1)) / 3;
        g = (2 * __GL_DXT_GREEN(c0) + __GL_DXT_GREEN(c1)) / 3;
        b = (2 * __GL_DXT_BLUE (c0) + __GL_DXT_BLUE (c1)) / 3;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Compute color 3: (c0 + 2 * c1) / 3. */
        r = (__GL_DXT_RED  (c0) + 2 * __GL_DXT_RED  (c1)) / 3;
        g = (__GL_DXT_GREEN(c0) + 2 * __GL_DXT_GREEN(c1)) / 3;
        b = (__GL_DXT_BLUE (c0) + 2 * __GL_DXT_BLUE (c1)) / 3;
        color[3] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;
    }
    else
    {
        /* Compute color 2: (c0 + c1) / 2. */
        r = (__GL_DXT_RED  (c0) + __GL_DXT_RED  (c1)) / 2;
        g = (__GL_DXT_GREEN(c0) + __GL_DXT_GREEN(c1)) / 2;
        b = (__GL_DXT_BLUE (c0) + __GL_DXT_BLUE (c1)) / 2;
        color[2] = 0x8000 | (r << 10) | ((g & 0x3E) << 4) | b;

        /* Color 3 is opaque black. */
        color[3] = 0;
    }

    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 2)
        {
            /* Copy the color. */
            *(GLshort *) Output = color[bits & 3];
        }

        /* Next line. */
        Output += Stride - Width * 2;
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of color information. */
static void
gcChipDecodeDXTColor32(
    IN gctSIZE_T Width,
    IN gctSIZE_T Height,
    IN gctSIZE_T Stride,
    IN const GLubyte * Data,
    IN const GLubyte *Alpha,
    OUT GLubyte * Output
    )
{
    GLuint color[4];
    GLushort c0, c1;
    gctSIZE_T x, y;
    GLuint r, g, b;
    gcmHEADER_ARG("Width=%d Height=%d Stride=%d Data=0x%x Alpha=0x%x Output=0x%x",
                  Width, Height, Stride, Data, Alpha, Output);

    /* Decode color 0. */
    c0       = Data[0] | (Data[1] << 8);
    color[0] = __GL_DXT_EXPAND_RED(c0) | __GL_DXT_EXPAND_GREEN(c0) | __GL_DXT_EXPAND_BLUE(c0);

    /* Decode color 1. */
    c1       = Data[2] | (Data[3] << 8);
    color[1] = __GL_DXT_EXPAND_RED(c1) | __GL_DXT_EXPAND_GREEN(c1) | __GL_DXT_EXPAND_BLUE(c1);

    /* Compute color 2: (c0 * 2 + c1) / 3. */
    r = (2 * (color[0] & 0xFF0000) + (color[1] & 0xFF0000)) / 3;
    g = (2 * (color[0] & 0x00FF00) + (color[1] & 0x00FF00)) / 3;
    b = (2 * (color[0] & 0x0000FF) + (color[1] & 0x0000FF)) / 3;
    color[2] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Compute color 3: (c0 + 2 * c1) / 3. */
    r = ((color[0] & 0xFF0000) + 2 * (color[1] & 0xFF0000)) / 3;
    g = ((color[0] & 0x00FF00) + 2 * (color[1] & 0x00FF00)) / 3;
    b = ((color[0] & 0x0000FF) + 2 * (color[1] & 0x0000FF)) / 3;
    color[3] = (r & 0xFF0000) | (g & 0x00FF00) | (b & 0x0000FF);

    /* Walk all lines. */
    for (y = 0; y < Height; y++)
    {
        /* Get lookup for line. */
        GLubyte bits = Data[4 + y];
        gctSIZE_T a  = y << 2;

        /* Walk all pixels. */
        for (x = 0; x < Width; x++, bits >>= 2, Output += 4)
        {
            /* Combine the lookup color with the alpha value. */
            GLuint c = color[bits & 3] | (Alpha[a++] << 24);
            *(GLuint *) Output = c;
        }

        /* Next line. */
        Output += Stride - Width * 4;
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of alpha information. */
static void
gcChipDecodeDXT3Alpha(
    IN const GLubyte *Data,
    OUT GLubyte *Output
    )
{
    GLint i;
    GLubyte a;
    gcmHEADER_ARG("Data=0x%x Output=0x%x", Data, Output);

    /* Walk all alpha pixels. */
    for (i = 0; i < 8; i++, Data++)
    {
        /* Get even alpha and expand into 8-bit. */
        a = *Data & 0x0F;
        *Output++ = a | (a << 4);

        /* Get odd alpha and expand into 8-bit. */
        a = *Data >> 4;
        *Output++ = a | (a << 4);
    }
    gcmFOOTER_NO();
}

/* Decode 64-bits of alpha information. */
static void
gcChipDecodeDXT5Alpha(
    IN const GLubyte *Data,
    OUT GLubyte *Output
    )
{
    GLint i, j, n;
    GLubyte a[8];
    GLushort bits = 0;
    gcmHEADER_ARG("Data=0x%x Output=0x%x", Data, Output);

    /* Load alphas 0 and 1. */
    a[0] = Data[0];
    a[1] = Data[1];

    if (a[0] > a[1])
    {
        /* Interpolate alphas 2 through 7. */
        a[2] = (GLubyte) ((6 * a[0] + 1 * a[1]) / 7);
        a[3] = (GLubyte) ((5 * a[0] + 2 * a[1]) / 7);
        a[4] = (GLubyte) ((4 * a[0] + 3 * a[1]) / 7);
        a[5] = (GLubyte) ((3 * a[0] + 4 * a[1]) / 7);
        a[6] = (GLubyte) ((2 * a[0] + 5 * a[1]) / 7);
        a[7] = (GLubyte) ((1 * a[0] + 6 * a[1]) / 7);
    }
    else
    {
        /* Interpolate alphas 2 through 5. */
        a[2] = (GLubyte) ((4 * a[0] + 1 * a[1]) / 5);
        a[3] = (GLubyte) ((3 * a[0] + 2 * a[1]) / 5);
        a[4] = (GLubyte) ((2 * a[0] + 3 * a[1]) / 5);
        a[5] = (GLubyte) ((1 * a[0] + 4 * a[1]) / 5);

        /* Set alphas 6 and 7. */
        a[6] = 0;
        a[7] = 255;
    }

    /* Walk all pixels. */
    for (i = 0, j = 2, n = 0; i < 16; i++, bits >>= 3, n -= 3)
    {
        /* Test if we have enough bits in the accumulator. */
        if (n < 3)
        {
            /* Load another chunk of bits in the accumulator. */
            bits |= Data[j++] << n;
            n += 8;
        }

        /* Copy decoded alpha value. */
        Output[i] = a[bits & 0x7];
    }
    gcmFOOTER_NO();
}


/*************************************************************************
** PowerVR Decompress
*************************************************************************/

/* Decompress a DXT texture. */
GLvoid*
gcChipDecompressDXT(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T* pRowStride
    )
{
    GLubyte * pixels = gcvNULL;
    GLubyte * line;
    const GLubyte * data;
    gctSIZE_T x, y, stride;
    GLubyte alpha[16];
    gctSIZE_T bpp;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d ImageSize=%d Data=0x%x InternalFormat=0x%04x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, ImageSize, Data, InternalFormat, Format, pRowStride);

    /* Determine bytes per pixel. */
    bpp = ((InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) || (InternalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)) ? 4 : 2;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * bpp, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    /* Initialize the variables. */
    stride = Width * bpp;
    data   = Data;

    GL_ASSERT(Format && pRowStride);

    /* Walk all lines, 4 lines per block. */
    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        /* Walk all pixels, 4 pixels per block. */
        for (x = 0; x < Width; x += 4)
        {
            /* Dispatch on format. */
            switch (InternalFormat)
            {
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                GL_ASSERT(ImageSize >= 16);

                /* Decode DXT3 alpha. */
                gcChipDecodeDXT3Alpha(data, alpha);
                /* Decompress one color block. */
                gcChipDecodeDXTColor32(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data + 8, alpha, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_A8R8G8B8;
                *pRowStride = Width * 4;
                break;

            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                GL_ASSERT(ImageSize >= 16);

                /* Decode DXT5 alpha. */
                gcChipDecodeDXT5Alpha(data, alpha);
                /* Decompress one color block. */
                gcChipDecodeDXTColor32(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4),
                                         stride, data + 8, alpha, p);

                /* 16 bytes per block. */
                data      += 16;
                ImageSize -= 16;
                *Format = gcvSURF_A8R8G8B8;
                *pRowStride = Width * 4;
                break;

            default:
                GL_ASSERT(ImageSize >= 8);

                /* Decompress one color block. */
                gcChipDecodeDXTColor16(__GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4), stride, data, p);

                /* 8 bytes per block. */
                data      += 8;
                ImageSize -= 8;
                *Format = gcvSURF_A1R5G5B5;
                *pRowStride = Width * 2;
                break;
            }

            /* Next block. */
            p += 4 * bpp;
        }
    }

    /* Return pointer to decompressed data. */
    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

GLvoid*
gcChipDecompressPalette(
    IN  __GLcontext* gc,
    IN  GLenum InternalFormat,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  GLint Level,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    OUT gceSURF_FORMAT * Format,
    OUT gctSIZE_T *pRowStride)
{
    gctSIZE_T pixelBits = 0, paletteBytes = 0;
    const GLubyte * palette;
    const GLubyte * data;
    GLubyte * pixels = gcvNULL;
    gctSIZE_T x, y, bytes;
    gctSIZE_T offset;

    gcmHEADER_ARG("gc=0x%x InternalFormat=0x%04x Width=%d Height=%d Level=%d ImageSize=%d Data=0x%x Format=0x%x pRowStride=0x%x",
                   gc, InternalFormat, Width, Height, Level, ImageSize, Data, Format, pRowStride);

    GL_ASSERT(Format && pRowStride);

    switch (InternalFormat)
    {
    case GL_PALETTE4_RGBA4_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R4G4B4A4;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_RGB5_A1_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G5B5A1;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_R5_G6_B5_OES:
        pixelBits    = 4;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G6B5;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE4_RGB8_OES:
        pixelBits    = 4;
        paletteBytes = 24 / 8;
        *Format      = gcvSURF_B8G8R8;
        *pRowStride  = Width * 3;
        break;

    case GL_PALETTE4_RGBA8_OES:
        pixelBits    = 4;
        paletteBytes = 32 / 8;
        *Format      = gcvSURF_A8B8G8R8;
        *pRowStride  = Width * 4;
        break;

    case GL_PALETTE8_RGBA4_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R4G4B4A4;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_RGB5_A1_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G5B5A1;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_R5_G6_B5_OES:
        pixelBits    = 8;
        paletteBytes = 16 / 8;
        *Format      = gcvSURF_R5G6B5;
        *pRowStride  = Width * 2;
        break;

    case GL_PALETTE8_RGB8_OES:
        pixelBits    = 8;
        paletteBytes = 24 / 8;
        *Format      = gcvSURF_B8G8R8;
        *pRowStride  = Width * 3;
        break;

    case GL_PALETTE8_RGBA8_OES:
        pixelBits    = 8;
        paletteBytes = 32 / 8;
        *Format      = gcvSURF_A8B8G8R8;
        *pRowStride  = Width * 4;
        break;
    }

    palette = Data;

    bytes = paletteBytes << pixelBits;
    data  = (const GLubyte *) palette + bytes;

    gcmASSERT(ImageSize > bytes);
    ImageSize -= bytes;

    while (Level-- > 0)
    {
        bytes  = gcmALIGN(Width * pixelBits, 8) / 8 * Height;
        data  += bytes;

        GL_ASSERT(ImageSize > bytes);
        ImageSize -= bytes;

        Width  = Width / 2;
        Height = Height / 2;
    }

    bytes = gcmALIGN(Width * paletteBytes, (gctSIZE_T)gc->clientState.pixel.unpackModes.alignment) * Height;

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    for (y = 0, offset = 0; y < Height; ++y)
    {
        for (x = 0; x < Width; ++x)
        {
            GLubyte pixel;

            GL_ASSERT(ImageSize > 0);
            if (pixelBits == 4)
            {
                if (x & 1)
                {
                    pixel = *data++ & 0xF;
                    --ImageSize;
                }
                else
                {
                    pixel = *data >> 4;
                }
            }
            else
            {
                pixel = *data++;
                --ImageSize;
            }

            __GL_MEMCOPY(pixels + offset, palette + pixel * paletteBytes, paletteBytes);
            offset += paletteBytes;
        }

        offset = gcmALIGN(offset, (gctSIZE_T)gc->clientState.pixel.unpackModes.alignment);

        if (x & 1)
        {
            ++data;
            --ImageSize;
        }
    }

    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

GLvoid*
gcChipDecompressETC1(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    OUT gceSURF_FORMAT * Format,
    OUT gctSIZE_T *pRowStride
    )
{
    GLubyte * pixels = gcvNULL;
    GLubyte * line;
    const GLubyte * data;
    gctSIZE_T x, y, stride;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d ImageSize=%d Data=0x%x Format=0x%x pRowStride=0x%x",
                   gc, Width, Height, ImageSize, Data, Format, pRowStride);

    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, Width * Height * 3, (gctPOINTER*)&pixels)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    stride = Width * 3;
    data   = Data;

    for (y = 0, line = pixels; y < Height; y += 4, line += stride * 4)
    {
        GLubyte * p = line;

        for (x = 0; x < Width; x += 4)
        {
            GL_ASSERT(ImageSize >= 8);
            gcChipDecodeETC1Block(p, stride, __GL_MIN(Width - x, 4), __GL_MIN(Height - y, 4), data);
            p         += 4 * 3;
            data      += 8;
            ImageSize -= 8;
        }
    }

    GL_ASSERT(Format && pRowStride);
    *Format = gcvSURF_B8G8R8;
    *pRowStride = stride;

    gcmFOOTER_ARG("return=0x%x", pixels);
    return pixels;
}

/*************************************************************************
** EAC 11bit Decompress
*************************************************************************/

static GLint
__get3Bits(
    const GLubyte * Data,
    gctSIZE_T start
    )
{
    gctSIZE_T i, j;

    i = start / 8;
    j = start % 8;

    if (j < 6)
    {
        /* Grab 3 bits from Ith byte. */
        return (Data[i] >> j) & 0x7;
    }
    else
    {
        /* Grab j bits from Ith byte, and 8 - j bits from (I+1)th byte */
        return ((Data[i] >> j) | (Data[i+1] << (8 - j))) & 0x7;
    }
}

/* Decode 8 bytes of 16 compressed 11 bit pixels,
   into 16 16F float values. */
static void
gcChipDecodeEAC11Block(
    GLubyte * Output,
    gctSIZE_T Width,
    gctSIZE_T inX,
    gctSIZE_T inY,
    gctSIZE_T RequiredW,
    gctSIZE_T RequiredH,
    GLboolean signedFormat,
    GLboolean gPresent,
    const GLubyte * Data
    )
{
    gctSIZE_T i, x, y, offset, pixelStride;
    gctSIZE_T pixelIndex, fetchIndex;
    GLint modifier, A;
    GLfloat color;

    struct _eacBlock
    {
        GLbitfield skip1        : 8;
        GLbitfield skip2        : 8;
        GLbitfield skip3        : 8;
        GLbitfield skip4        : 8;
        GLbitfield skip5        : 8;
        GLbitfield skip6        : 8;

        GLbitfield tableIndex   : 4;
        GLbitfield multiplier   : 4;
        GLbitfield baseCodeWord : 8;
    } eacBlock;

    /* EAC modifier table. */
    static GLint EACModifierTable[] =
    {
         -3,  -6,  -9, -15,   2,   5,   8,  14,        /* codeword0. */
         -3,  -7, -10, -13,   2,   6,   9,  12,        /* codeword1. */
         -2,  -5,  -8, -13,   1,   4,   7,  12,        /* codeword2. */
         -2,  -4,  -6, -13,   1,   3,   5,  12,        /* codeword3. */
         -3,  -6,  -8, -12,   2,   5,   7,  11,        /* codeword4. */
         -3,  -7,  -9, -11,   2,   6,   8,  10,        /* codeword5. */
         -4,  -7,  -8, -11,   3,   6,   7,  10,        /* codeword6. */
         -3,  -5,  -8, -11,   2,   4,   7,  10,        /* codeword7. */
         -2,  -6,  -8, -10,   1,   5,   7,   9,        /* codeword8. */
         -2,  -5,  -8, -10,   1,   4,   7,   9,        /* codeword9. */
         -2,  -4,  -8, -10,   1,   3,   7,   9,        /* codeword10. */
         -2,  -5,  -7, -10,   1,   4,   6,   9,        /* codeword11. */
         -3,  -4,  -7, -10,   2,   3,   6,   9,        /* codeword12. */
         -1,  -2,  -3, -10,   0,   1,   2,   9,        /* codeword13. */
         -4,  -6,  -8,  -9,   3,   5,   7,   8,        /* codeword14. */
         -3,  -5,  -7,  -9,   2,   4,   6,   8,        /* codeword15. */
    };

    GLubyte *littleEndianArray = (GLubyte *) &eacBlock;

    /* Convert to little endian. */
    for (i = 0; i < 8; i++)
    {
        littleEndianArray[i] = Data[7 - i];
    }

    /* Stride in bytes. */
    pixelStride = gPresent ? 4 : 2;

    for (y = 0; y < RequiredH; y++)
    {
        offset = ((inY + y) * Width + inX) * pixelStride;

        for (x = 0; x < RequiredW; x++)
        {
            fetchIndex = (15 - (x << 2) - y) * 3;

            /* Get fetchIndex to fetchIndex + 2 from Data (64 bit block). */
            pixelIndex = __get3Bits(littleEndianArray, fetchIndex);

            modifier = EACModifierTable[(eacBlock.tableIndex << 3) + pixelIndex];

            if (signedFormat)
            {
                GLubyte baseCodeWord = (GLubyte)eacBlock.baseCodeWord;
                A = *((GLbyte*)&baseCodeWord);
                if (A == -128)
                {
                    A = -127;
                }
                A <<= 3;
                if (eacBlock.multiplier > 0)
                {
                    A += (eacBlock.multiplier * modifier) << 3;
                }
                else
                {
                    A += modifier;
                }

                /* Clamp to [-1023, 1023]. */
                A = (A < -1023) ? -1023 : ((A > 1023) ? 1023 : A);

                if ((A > 0) && (A < 1023))
                {
                    color = (A + 0.5f) / 1023.0f;
                }
                else
                {
                    color = A / 1023.0f;
                }
            }
            else
            {
                A = (eacBlock.baseCodeWord << 3) + 4;

                if (eacBlock.multiplier > 0)
                {
                    A += (eacBlock.multiplier * modifier) << 3;
                }
                else
                {
                    A += modifier;
                }

                /* Clamp to [0, 2047]. */
                A = (A < 0) ? 0 : ((A > 2047) ? 2047 : A);

                if ((A < 2047) && (A != 1023))
                {
                    color = (A + 0.5f) / 2047.0f;
                }
                else
                {
                    color = A / 2047.0f;
                }
            }

            /* Write the decoded color. */
            *(gctUINT16*)(Output + offset) = gcoMATH_FloatToFloat16(*(gctUINT32*)&color);
            offset += pixelStride;
        }
    }
}

/* Decompress EAC 11bit textures. */
GLvoid*
gcChipDecompress_EAC_11bitToR16F(
    IN  __GLcontext* gc,
    IN  gctSIZE_T Width,
    IN  gctSIZE_T Height,
    IN  gctSIZE_T Depth,
    IN  gctSIZE_T ImageSize,
    IN  const void * Data,
    IN  GLenum InternalFormat,
    OUT gceSURF_FORMAT *Format,
    OUT gctSIZE_T *pRowStride
    )
{
    GLubyte * pIn  = gcvNULL;
    GLubyte * pOut = gcvNULL;
    GLvoid  * decompressed = gcvNULL;
    gctSIZE_T offset, outputSize, x, y, slice;
    GLboolean gPresent = gcvFALSE;
    GLboolean signedFormat = gcvFALSE;

    gcmHEADER_ARG("gc=0x%x Width=%d Height=%d Depth=%d, ImageSize=%d Data=0x%x "
                  "InternalFormat=0x%04x Format=0x%x pRowStride=0x%x", gc, Width,
                  Height, Depth, ImageSize, Data, InternalFormat, Format, pRowStride);

    /* Determine bytes per pixel. */
    GL_ASSERT((ImageSize % 8) == 0 && (ImageSize % Depth) == 0);
    GL_ASSERT(Format && pRowStride);

    switch (InternalFormat)
    {
    case GL_COMPRESSED_SIGNED_R11_EAC:
        signedFormat = gcvTRUE;
        /* Fall through. */
    case GL_COMPRESSED_R11_EAC:
        *Format  = gcvSURF_R16F_1_A4R4G4B4;
        *pRowStride = Width * 2;
        break;

    case GL_COMPRESSED_SIGNED_RG11_EAC:
        signedFormat = gcvTRUE;
        /* Fall through. */
    case GL_COMPRESSED_RG11_EAC:
        gPresent = gcvTRUE;
        *Format  = gcvSURF_G16R16F_1_A8R8G8B8;
        *pRowStride = Width * 4;
        break;

    default:
        GL_ASSERT(GL_FALSE);
        return gcvNULL;
    }
    outputSize = (*pRowStride) * Height * Depth;

    /* Allocate the decompressed memory. */
    if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL, outputSize, (gctPOINTER*)&decompressed)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    pIn  = (GLubyte*)Data;
    pOut = (GLubyte*)decompressed;
    for (slice = 0; slice < Depth; ++slice)
    {
        offset = 0;
        for (y = 0; y < Height; y += 4)
        {
            for (x = 0; x < Width; x += 4)
            {
                /* For some mip level, not the whole blocks need to decompressed */
                gctSIZE_T w = __GL_MIN(Width - x, 4);
                gctSIZE_T h = __GL_MIN(Height - y, 4);

                /* Decompress 8 byte R block into 16 pixels. */
                gcChipDecodeEAC11Block(pOut,
                                       Width,
                                       x, y,
                                       w, h,
                                       signedFormat,
                                       gPresent,
                                       pIn + offset);
                offset += 8;

                if (gPresent)
                {
                    /* Decompress 8 byte G block into 16 pixels. */
                    gcChipDecodeEAC11Block(pOut + 2,
                                           Width,
                                           x, y,
                                           w, h,
                                           signedFormat,
                                           gPresent,
                                           pIn + offset);
                    offset += 8;
                }
            }
        }

        pOut += (*pRowStride) * Height;
        pIn  += ImageSize / Depth;
    }


    /* Return pointer to converted data. */
    gcmFOOTER_ARG("return=0x%x", decompressed);
    return decompressed;
}
