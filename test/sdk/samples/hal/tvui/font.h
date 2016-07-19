/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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




typedef struct _gcsGLYPH *        gcsGLYPH_PTR;
typedef struct _gcsFONTTABLE *    gcsFONTTABLE_PTR;

struct _gcsGLYPH
{
    gctUINT32            blackBoxX;
    gctUINT32            blackBoxY;
    gctINT32            glyphOriginX;
    gctINT32            glyphOriginY;
    gctINT16            cellIncX;
    gctINT16            cellIncY;
    gctUINT32            size;
    gctUINT32            offset;
};

struct _gcsFONTTABLE
{
    char  *                name;
    gctUINT8 *            data;
    gctINT32            size;
    gctINT32            height;
    gctBOOL                bold;
    gctBOOL                italic;
    gctBOOL                underline;
    struct _gcsGLYPH    glyph[256];
};

#include "fontArial11.h"
#include "fontArial12.h"
#include "fontArial14.h"
#include "fontArial16.h"
#include "fontArial18.h"

gcsFONTTABLE_PTR fontTableIndex[] =
{
    &fontTable_Arial_11,
    &fontTable_Arial_12,
    &fontTable_Arial_14,
    &fontTable_Arial_16,
    &fontTable_Arial_18,
};
