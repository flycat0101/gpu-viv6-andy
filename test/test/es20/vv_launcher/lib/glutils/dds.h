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


#ifndef gutils_dds_INCLUDED
#define gutils_dds_INCLUDED

#include <GLES2/gl2.h>

#include "Texture2D.h"
#include "TextureCube.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  Support for DDS-file texture loading/saving and DXT compression

  Support for DXT texture loading/saving and compression.  Supports:

    DDS 2D texture loading with mipmaps
    DDS cube map texture loading with mipmaps
    Mipmap pyramid generation
    DDS 2D and cube map texture saving
    On-the-fly conpression to DXT formats

  The library can load DDS files of the following formats:

    DXT1, DXT3, DXT5
    A8B8G8R8
    A8R8G8B8
    R5G6B5
    A8, L8, A8L8
*/

#define MAX_MIPMAPS         16
#define NUM_CUBEMAP_FACES    6

typedef struct _Image
{
   char* name;

   int width;
   int height;

   /* Components per texel: 1-4 */
   int components;

   /* The GL type of each color component (noncompressed textures only) */
   int component_format;

   /* The number of bytes per pixel (noncompressed textures only) */
   int bytes_per_pixel;

   /* If true, it DXT-compressed */
   int compressed;

   /* The number of levels in the mipmap pyramid (including the base level) */
   int num_mipmaps;

   /* If true, then the file contains 6 cubemap faces */
   int cubemap;

   /* The GL format of the loaded texture data */
   int format;

   /* If true, the texture data includes alpha */
   int alpha;

   /* Base of the allocated block of all texel data */
   void * data_block;

   /* Pointers to the mipmap levels for the texture or each cubemap face */
   void * data[MAX_MIPMAPS * NUM_CUBEMAP_FACES];

   /* Array of sizes of the mipmap levels for the texture or each
      cubemap face */
   int size[MAX_MIPMAPS * NUM_CUBEMAP_FACES];

   /* Array of widths of the mipmap levels for the texture or each
      cubemap face */
   int mipwidth[MAX_MIPMAPS * NUM_CUBEMAP_FACES];

   /* Array of heights of the mipmap levels for the texture or each
      cubemap face */
   int mipheight[MAX_MIPMAPS * NUM_CUBEMAP_FACES];
} Image;


Image* ImageConstruct();

void ImageDestroy(Image*);


Image* LoadFromResource(const char * Name);

Texture2D* CreateTexture2D(const Image * , int UseMipmaps);

TextureCube* CreateCubemap(const Image *, int UseMipmaps);

#ifdef __cplusplus
}
#endif

#endif

