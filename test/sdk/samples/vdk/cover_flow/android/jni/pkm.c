/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#pragma pack(1)
struct PKM_HEADER
{
    unsigned char Magic[4];
    unsigned char Version[2];
    unsigned char Type[2];
    unsigned char Width[2];
    unsigned char Height[2];
    unsigned char ActiveWidth[2];
    unsigned char ActiveHeight[2];
};
#pragma pack()

void *
LoadPKM(
    FILE * File,
    GLenum * Format,
    GLsizei * Width,
    GLsizei * Height,
    GLsizei * Bytes
    )
{
    struct PKM_HEADER pkm;
    size_t bytes;
    unsigned char * bits;
    unsigned short type;
    unsigned short width, height;
    unsigned short activeWidth, activeHeight;

    /* Read the PKM file header. */
    if (fread(&pkm, sizeof(pkm), 1, File) != 1)
    {
        return NULL;
    }

    /* Validate magic data. */
    if ((pkm.Magic[0] != 'P')
    ||  (pkm.Magic[1] != 'K')
    ||  (pkm.Magic[2] != 'M')
    ||  (pkm.Magic[3] != ' ')
    )
    {
        return NULL;
    }

    /* Convert from big endian to numbers. */
    type         = (pkm.Type[0]         << 8) | pkm.Type[1];
    width        = (pkm.Width[0]        << 8) | pkm.Width[1];
    height       = (pkm.Height[0]       << 8) | pkm.Height[1];
    activeWidth  = (pkm.ActiveWidth[0]  << 8) | pkm.ActiveWidth[1];
    activeHeight = (pkm.ActiveHeight[0] << 8) | pkm.ActiveHeight[1];

    /* For now we only support ETC1_RGB_NO_MIPMAPS. */
    if (type != 0)
    {
        return NULL;
    }

    /* ETC1 RGB texture format. */
    *Format = GL_ETC1_RGB8_OES;

    /* Return texture dimension. */
    *Width  = activeWidth;
    *Height = activeHeight;

    /* Compute number of bytes. */
    bytes = ((width + 3)/ 4) * ((height + 3) / 4) * 8;

    *Bytes = bytes;

    /* Allocate the bits. */
    bits = (unsigned char *) malloc(bytes);

    if (bits != NULL)
    {
        /* Read the bits from the PKM file. */
        if (fread(bits, 1, bytes, File) != bytes)
        {
            /* Error reading bits. */
            free(bits);
            bits = NULL;
        }
    }

    /* Return the bits. */
    return bits;
}
