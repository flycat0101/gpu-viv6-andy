/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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


#include "dds.h"

#include <stdlib.h>
#include <string.h>
#include <GLES2/gl2.h>

#include <rc/resource.h>
#include <math/misc.h>
#include "check.h"
#include "log.h"
#include "gl2extext.h"
#include "dds_internal.h"

Image* ImageConstruct()
{
    Image* image = (Image*)malloc(sizeof (Image));
    if (!image)
    {
        LogError("Can not allocate memory\n");
        return NULL;
    }

    image->width  = 0;
    image->height = 0;
    image->components = 0;
    image->component_format = 0;
    image->bytes_per_pixel = 0;
    image->compressed = 0; /* false */
    image->num_mipmaps = 0;
    image->cubemap = 0; /* false */
    image->format = 0;
    image->alpha = 0; /* false */
    image->data_block = NULL;

    {
        int i;
        for (i = 0; i < (MAX_MIPMAPS * NUM_CUBEMAP_FACES); ++i)
        {
            image->data[i] = NULL;
            image->size[i] = 0;
            image->mipwidth[i] = 0;
            image->mipheight[i] = 0;
        }
    }

    return image;
}


void ImageDestroy(Image* Img)
{
    assert(Img != NULL);

    if (Img->data_block != NULL)
    {
        free(Img->data_block);
    }

    if (Img->name != NULL)
    {
        free(Img->name);
    }

    free(Img);
}


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;


static void _SwapChar(uint8* A, uint8* B)
{
     uint8 tmp;
     tmp = *A;
     *A = *B;
     *A = tmp;
}


static void _SwapShort(uint16* A, uint16* B)
{
     uint16 tmp;
     tmp = *A;
     *A = *B;
     *B = tmp;
}


static int _GetSize(int W, int H, const Image* Img)
{
    if (Img->compressed)
    {
         int bpp = (Img->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
        return ((W+3)/4) * ((H+3)/4) * bpp;
    }

    return W * H * Img->bytes_per_pixel;
}


static int _TotalImageDataSize(Image* Img)
{
    int index = 0;
    int size=0;
    int i,j;
    int cubeCount = Img->cubemap ? 6 : 1;

    for (j = 0; j < cubeCount; ++j)
    {
        int w = Img->width;
        int h = Img->height;

        // account for base plus each mip
        for (i=0; i<Img->num_mipmaps; i++)
        {
            Img->size[index] = _GetSize(w, h, Img);
            Img->mipwidth[index] = w;
            Img->mipheight[index] = h;

            size += Img->size[index];

            if(w != 1) w >>= 1;
            if(h != 1) h >>= 1;

            ++index;
        }
    }

    return size;
}


static void _FlipBlocks_dxtc1(DXTColBlock *Line, int Num_blocks)
{
    int i;
    DXTColBlock *curblock = Line;

    for (i = 0; i < Num_blocks; i++)
    {
        _SwapChar(&curblock->row[0], &curblock->row[3]);
        _SwapChar(&curblock->row[1], &curblock->row[2]);
        ++curblock;
    }
}


static void _FlipBlocks_dxtc3(DXTColBlock *Line, int Num_blocks)
{
    int i;
    DXTColBlock *curblock = Line;
    DXT3AlphaBlock *alphablock;

    for (i = 0; i < Num_blocks; ++i)
    {
        alphablock = (DXT3AlphaBlock*)curblock;

        _SwapShort(&alphablock->row[0], &alphablock->row[3]);
        _SwapShort(&alphablock->row[1], &alphablock->row[2]);
        ++curblock;

        _SwapChar(&curblock->row[0], &curblock->row[3]);
        _SwapChar(&curblock->row[1], &curblock->row[2]);
        ++curblock;
    }
}


static void _Flip_dxt5_alpha(DXT5AlphaBlock *Block)
{
    int8 gBits[4][4];

    const uint32 mask = 0x00000007;     /* bits = 00 00 01 11 */
    uint32 bits = 0;
    memcpy(&bits, &Block->row[0], sizeof (int8) * 3);

    gBits[0][0] = (int8)(bits & mask);
    bits >>= 3;
    gBits[0][1] = (int8)(bits & mask);
    bits >>= 3;
    gBits[0][2] = (int8)(bits & mask);
    bits >>= 3;
    gBits[0][3] = (int8)(bits & mask);
    bits >>= 3;
    gBits[1][0] = (int8)(bits & mask);
    bits >>= 3;
    gBits[1][1] = (int8)(bits & mask);
    bits >>= 3;
    gBits[1][2] = (int8)(bits & mask);
    bits >>= 3;
    gBits[1][3] = (int8)(bits & mask);

    bits = 0;
    memcpy(&bits, &Block->row[3], sizeof (int8) * 3);

    gBits[2][0] = (int8)(bits & mask);
    bits >>= 3;
    gBits[2][1] = (int8)(bits & mask);
    bits >>= 3;
    gBits[2][2] = (int8)(bits & mask);
    bits >>= 3;
    gBits[2][3] = (int8)(bits & mask);
    bits >>= 3;
    gBits[3][0] = (int8)(bits & mask);
    bits >>= 3;
    gBits[3][1] = (int8)(bits & mask);
    bits >>= 3;
    gBits[3][2] = (int8)(bits & mask);
    bits >>= 3;
    gBits[3][3] = (int8)(bits & mask);

    bits =
        (gBits[3][0] << 0) |
        (gBits[3][1] << 3) |
        (gBits[3][2] << 6) |
        (gBits[3][3] << 9) |
        (gBits[2][0] << 12) |
        (gBits[2][1] << 15) |
        (gBits[2][2] << 18) |
        (gBits[2][3] << 21);

    memcpy(&Block->row[0], &bits, 3);

    bits =
        (gBits[1][0] << 0) |
        (gBits[1][1] << 3) |
        (gBits[1][2] << 6) |
        (gBits[1][3] << 9) |
        (gBits[0][0] << 12) |
        (gBits[0][1] << 15) |
        (gBits[0][2] << 18) |
        (gBits[0][3] << 21);

    memcpy(&Block->row[3], &bits, 3);
}



static void _FlipBlocks_dxtc5(DXTColBlock *Line, int Num_blocks)
{
    DXTColBlock *curblock = Line;
    DXT5AlphaBlock *alphablock;
    int i;

    for (i = 0; i < Num_blocks; ++i)
    {
        alphablock = (DXT5AlphaBlock*)curblock;

        _Flip_dxt5_alpha(alphablock);
        ++curblock;

        _SwapChar(&curblock->row[0], &curblock->row[3]);
        _SwapChar(&curblock->row[1], &curblock->row[2]);
        ++curblock;
    }
}


static void _FlipDataVertical(char* Img, int Width, int Height, Image* Info)
{
    int linesize;
    char* tmp;
    int j;

    if (Info->compressed)
    {
        void (*flipblocks)(DXTColBlock*, int) = NULL;
        int xblocks = Width / 4;
        int yblocks = Height / 4;
        int blocksize;

        switch (Info->format)
        {
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            blocksize = 8;
            flipblocks = &_FlipBlocks_dxtc1;
            break;

        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            blocksize = 16;
            flipblocks = &_FlipBlocks_dxtc3;
            break;

        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            blocksize = 16;
            flipblocks = &_FlipBlocks_dxtc5;
            break;

        default:
            return;
        }

        linesize = xblocks * blocksize;
        tmp = (char*)malloc(linesize);

        for (j = 0; j < (yblocks >> 1); j++)
        {
            char* raw_top = (Img + j * linesize);
            char* raw_bottom = Img + (((yblocks-j)-1) * linesize);
            DXTColBlock* top = (DXTColBlock*)raw_top;
            DXTColBlock* bottom = (DXTColBlock*)raw_bottom;

            (*flipblocks)(top, xblocks);
            (*flipblocks)(bottom, xblocks);

            memcpy(tmp,     bottom, linesize);
            memcpy(bottom, top,     linesize);
            memcpy(top,     tmp,     linesize);
        }

        // Catch the middle row of blocks if there is one
        // The loop above will skip the middle row
        if (yblocks & 0x01)
        {

            char* raw_middle = (Img + (yblocks >> 1) * linesize);
            DXTColBlock *middle = (DXTColBlock*)raw_middle;
            (*flipblocks)(middle, xblocks);
        }

        free(tmp);

    }
    else
    {
        int linesize = Width * Info->bytes_per_pixel;

        // much simpler - just compute the line length and swap each row
        char* tmp = (char*)malloc(linesize);

        int j;
        for (j = 0; j < (Height >> 1); j++)
        {

            int8* top     = (int8*)(Img + j * linesize);
            int8* bottom = (int8*)(Img + (((Height-j)-1) * linesize));

            memcpy(tmp,     bottom, linesize);
            memcpy(bottom, top,     linesize);
            memcpy(top,     tmp,     linesize);
        }

        free(tmp);
    }
}

static unsigned const char * _ReadFromResource(void * out,
                                              unsigned const char * in,
                                              int bytes)
{
    unsigned char * dest = (unsigned char *)out;
    int i;
    for (i = 0; i<bytes; ++i)
    {
        *dest++ = *in++;
    }

    return in;
}


static void _AllocData(Image* image)
{
    int size;
    int planes;
    int i;

    if (image == NULL)
    {
        LogError("dds::_AllocData: null image");
        return;
    }

    size = _TotalImageDataSize(image);
    image->data_block = malloc(size);
    if (image->data_block == NULL)
    {
        LogError("dds::_AllocData failed");
        return;
    }

    image->data[0] = image->data_block;

    planes = image->num_mipmaps * (image->cubemap ? 6 : 1);

    for (i = 1; i < planes; i++)
    { // account for base plus each mip
        image->data[i] = (void*)(((unsigned long)(image->data[i - 1]))
                                         + image->size[i - 1]);
    }
}


// Assumes texture ID is bound. Returns true upon success.
int LoadTextureFromImage(GLenum target,
                         int start_level,
                         const Image* image,
                         int use_mipmaps)
{
    GLenum base_target;
    GLsizei w;
    GLsizei h;
    int level;
    int power_of_two;
    int mip_levels_to_load;
    int max_level;
    int m;;

    if (image == NULL) return 0;

    base_target =
        (target == GL_TEXTURE_2D) ?
        GL_TEXTURE_2D :
        GL_TEXTURE_CUBE_MAP;

    w = image->width;
    h = image->height;
    level = start_level;


     /* Need to detect if the image has a non-pow-2 dimension */
    power_of_two = isPowerOfTwo(w) && isPowerOfTwo(h);

    /* Only pow-2 images that are tagged for using mipmaps get them */
    mip_levels_to_load =
        (power_of_two && use_mipmaps) ?
        image->num_mipmaps :
        1;

#ifdef DEBUG
    {
        LogError("width: %d\n", w);
        LogError("height: %d\n", h);
        LogError("power_of_two: %d\n", power_of_two);
        LogError("use_mipmaps: %d\n", use_mipmaps);
        LogError("mip_levels_to_load: %d\n", mip_levels_to_load);
    }
#endif

    max_level = level + mip_levels_to_load;

    for (m = 0; level < max_level; level++, m++ )
    {
        if (image->compressed)
        {
            glCompressedTexImage2D(target,
                                          m,
                                          image->format,
                                          w,
                                          h,
                                          0,
                                          image->size[level],
                                          image->data[level]);
            CheckGL(image->name);
        }
        else
        {
            glTexImage2D(target,
                             m,
                             image->format,
                             w,
                             h,
                             0,
                             image->format,
                             image->component_format,
                             image->data[level]);
            CheckGL(image->name);

        }

        w = (w == 1) ? w : w >> 1;
        h = (h == 1) ? h : h >> 1;
    }

    if (mip_levels_to_load > 1)
    {
        glTexParameterf(base_target,
                             GL_TEXTURE_MIN_FILTER,
                             GL_LINEAR_MIPMAP_LINEAR);
#ifdef DEBUG
        LogError("mipmapped\n");
#endif
    }
    else
    {
        glTexParameterf(base_target,
                             GL_TEXTURE_MIN_FILTER,
                             GL_LINEAR);
#ifdef DEBUG
        LogError("not mipmapped\n");
#endif
    }

    glTexParameterf(base_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

     /* For now, we clamp everyone - apps wanting wrapping for a
         texture can set it manually */
     glTexParameterf(base_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameterf(base_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     return 1;
}



Image* LoadFromResource(const char * rc_name)
{

    //    bool flip_vertical = true;    // True for OpenGL ?
    int flip_vertical = 0;    // True for OpenGL ?
    unsigned char const * file_src = 0;
    int file_size = 0;
    int needs_BGRA_swap = 0;
    int is_already_flipped = 0;
    char filecode[4];
    Image* image;
    int faces;
    int index;
    int j;
    DDS_HEADER ddsh;

    Resource* rc = GetResource(rc_name);
    if (rc == NULL)
    {
        LogError("dds::loadFromResource: %s: not found\n", rc_name);
        return NULL;
    }

    file_src = (unsigned const char *)(rc->data);
    file_size = rc->size;

    // read in file marker, make sure its a DDS file

    file_src = _ReadFromResource(filecode, file_src, 4);

    if (0 != strncmp(filecode, "DDS ", 4))
    {
        LogError("dds::loadFromResource: %s is not a DDS file\n", rc_name);
        return NULL;
    }

#ifdef DEBUG
    {
        LogError("\nnew Image: %s\n", rc_name);
    }
#endif

    // allocate our image base structure
    image = (Image*)malloc(sizeof (Image));
    // read in DDS header

#ifdef UNDER_CE
    image->name = _strdup(rc_name);
#else
    image->name = strdup(rc_name);
#endif

    file_src = _ReadFromResource(&ddsh, file_src, sizeof(ddsh));

    // check if image is a cubempap
    if (ddsh.dwCaps2 & DDS_CUBEMAP)
    {

        unsigned int const allFaces =
            DDS_CUBEMAP_POSITIVEX |
            DDS_CUBEMAP_POSITIVEY |
            DDS_CUBEMAP_POSITIVEZ |
            DDS_CUBEMAP_NEGATIVEX |
            DDS_CUBEMAP_NEGATIVEY |
            DDS_CUBEMAP_NEGATIVEZ;

        if ((ddsh.dwCaps2 & allFaces) != allFaces)
        {

            LogError("dds::loadFromResource: %s is an incomplete cubemap: not supported\n", rc_name);
            return NULL;
        }

        image->cubemap = 1;

    }
    else
    {

        image->cubemap = 0;
    }

    // check if image is a volume texture

    if ((ddsh.dwCaps2 & DDS_VOLUME) && (ddsh.dwDepth > 0))
    {

        LogError("dds::loadFromResource: %s is a volume image: not supported\n", rc_name);
        return NULL;
    }

    // figure out what the image format is
    if (ddsh.ddspf.dwFlags & DDS_FOURCC)
    {

        switch(ddsh.ddspf.dwFourCC)
        {
        case FOURCC_DXT1:
            image->format      = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            image->components = 3;
            image->compressed = 1;
            image->alpha = 0; // Ugh - for backwards compatibility
            break;

        case FOURCC_DXT3:
            image->format      = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            image->components = 4;
            image->compressed = 1;
            image->alpha = 1;
            break;

        case FOURCC_DXT5:
            image->format      = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            image->components = 4;
            image->compressed = 1;
            image->alpha = 1;
            break;

        default:
            {
                LogError("dds::loadFromResource: %s: Unsupported FOURCC code\n", rc_name);
                return NULL;
            }
        }

    }
    else
    {

        // Check for a supported pixel format

        if ((ddsh.ddspf.dwRGBBitCount == 32) &&
             (ddsh.ddspf.dwRBitMask == 0x000000FF) &&
             (ddsh.ddspf.dwGBitMask == 0x0000FF00) &&
             (ddsh.ddspf.dwBBitMask == 0x00FF0000) &&
             (ddsh.ddspf.dwABitMask == 0xFF000000))
        {

            // We support D3D's A8B8G8R8, which is actually RGBA in linear
            // memory, equivalent to GL's RGBA
            image->format      = GL_RGBA;
            image->components = 4;
            image->component_format = GL_UNSIGNED_BYTE;
            image->bytes_per_pixel = 4;
            image->alpha = 1;
            image->compressed = 0;

        }
        else if ((ddsh.ddspf.dwRGBBitCount == 32) &&
                      (ddsh.ddspf.dwRBitMask == 0x00FF0000) &&
                      (ddsh.ddspf.dwGBitMask == 0x0000FF00) &&
                      (ddsh.ddspf.dwBBitMask == 0x000000FF) &&
                      (ddsh.ddspf.dwABitMask == 0xFF000000))
        {
            // We support D3D's A8R8G8B8, which is actually BGRA in linear
            // memory, need to be
            image->format      = GL_RGBA;
            image->components = 4;
            image->component_format = GL_UNSIGNED_BYTE;
            image->bytes_per_pixel = 4;
            image->alpha = 1;
            image->compressed = 0;
            needs_BGRA_swap = 1;

        }
        else if ((ddsh.ddspf.dwRGBBitCount == 16) &&
                      (ddsh.ddspf.dwRBitMask == 0x0000F800) &&
                      (ddsh.ddspf.dwGBitMask == 0x000007E0) &&
                      (ddsh.ddspf.dwBBitMask == 0x0000001F) &&
                      (ddsh.ddspf.dwABitMask == 0x00000000))
        {
            // We support D3D's R5G6B5, which is actually RGB in linear
            // memory.  It is equivalent to GL's GL_UNSIGNED_SHORT_5_6_5
            image->format      = GL_RGB;
            image->components = 3;
            image->alpha = 0;
            image->component_format = GL_UNSIGNED_SHORT_5_6_5;
            image->bytes_per_pixel = 2;
            image->compressed = 0;

        }
        else if ((ddsh.ddspf.dwRGBBitCount == 8) &&
                      (ddsh.ddspf.dwRBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwGBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwBBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwABitMask == 0x000000FF))
        {
            // We support D3D's A8
            image->format      = GL_ALPHA;
            image->components = 1;
            image->alpha = 1;
            image->component_format = GL_UNSIGNED_BYTE;
            image->bytes_per_pixel = 1;
            image->compressed = 0;

        }
        else if ((ddsh.ddspf.dwRGBBitCount == 8) &&
                      (ddsh.ddspf.dwRBitMask == 0x000000FF) &&
                      (ddsh.ddspf.dwGBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwBBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwABitMask == 0x00000000))
        {

            // We support D3D's L8 (flagged as 8 bits of red only)
            image->format      = GL_LUMINANCE;
            image->components = 1;
            image->alpha = 1;
            image->component_format = GL_UNSIGNED_BYTE;
            image->bytes_per_pixel = 1;
            image->compressed = 0;

        }
        else if ((ddsh.ddspf.dwRGBBitCount == 16) &&
                      (ddsh.ddspf.dwRBitMask == 0x000000FF) &&
                      (ddsh.ddspf.dwGBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwBBitMask == 0x00000000) &&
                      (ddsh.ddspf.dwABitMask == 0x0000FF00))
        {
            // We support D3D's A8L8 (flagged as 8 bits of red and 8
            // bits of alpha)

            image->format      = GL_LUMINANCE_ALPHA;
            image->components = 2;
            image->alpha = 1;
            image->component_format = GL_UNSIGNED_BYTE;
            image->bytes_per_pixel = 2;
            image->compressed = 0;

        }
        else
        {

            LogError("dds::loadFromResource: %s: is not a DXTC or supported RGB(A) format\n", rc_name);
            return NULL;
        }
    }

    // detect flagging to indicate this texture was stored in a
    // y-inverted fashion

    if(!(ddsh.dwFlags&DDS_LINEARSIZE))
    {
        if(ddsh.dwPitchOrLinearSize == DDS_MAGIC_FLIPPED)
        {
            is_already_flipped = 1;
        }
    }

    flip_vertical = is_already_flipped != flip_vertical;

    // store primary surface width/height/num_mipmaps
    image->width        = ddsh.dwWidth;
    image->height      = ddsh.dwHeight;
    image->num_mipmaps = ddsh.dwFlags&DDS_MIPMAPCOUNT ? ddsh.dwMipMapCount : 1;

    if (image->num_mipmaps > MAX_MIPMAPS)
    {
        LogError("dds::loadFromResource: %s: Too many mipmaps\n", rc_name);
        return NULL;
    }

    // allocate the meta datablock for all mip storage.
    _AllocData(image);
    if (image->data_block==0)
    {
        LogError("dds::loadFromResource: %s: failed to allocate memory\n", rc_name);
        return NULL;
    }

    faces = image->cubemap ? 6 : 1;
    index = 0;

    for (j = 0; j < faces; j++)
    {
        // load all surfaces for the image
        int width  = image->width;
        int height = image->height;
        int i;

        for (i = 0; i < image->num_mipmaps; i++)
        {
            // Get the size, read in the data.
            file_src = _ReadFromResource(image->data[index],
                                                 file_src,
                                                 image->size[index]);

            // Flip in Y for OpenGL if needed
            if (flip_vertical)
            {
                _FlipDataVertical((char*)image->data[index],
                                      width, height, image);
            }

            // shrink to next power of 2
            width  >>= 1;
            height >>= 1;

            if (!width) width = 1;
            if (!height) height = 1;

            // make sure DXT isn't <4 on a side...
            if (image->compressed)
            {
                if (width < 4) width = 4;
                if (height < 4) height = 4;
            }

            ++index;
        }
    }

    if (needs_BGRA_swap)
    {
        int index = 0;
        int k;

        for (k = 0; k < faces; ++k)
        {
            int width  = image->width;
            int height = image->height;

            int i;
            for (i = 0; i < image->num_mipmaps; ++i)
            {
                char* data = (char*)(image->data[index]);
                int pixels = width * height;

                int j;
                for (j = 0; j < pixels; j++)
                {
                    char temp = data[0];
                    data[0] = data[2];
                    data[2] = temp;

                    data += 4;
                }

                // shrink to next power of 2
                width  >>= 1;
                height >>= 1;

                if (!width) width = 1;
                if (!height) height = 1;

                ++index;
            }
        }
    }

    return image;
}


Texture2D* CreateTexture2D(const Image* image, int use_mipmaps)
{
    GLuint tex = 0;

    if (image->cubemap)
    {
        LogError("CreateTexture: %s: is a cube map. Use CreateCubemap\n", image->name);
        return NULL;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    CheckGL(image->name);

    if (LoadTextureFromImage(GL_TEXTURE_2D, 0, image, use_mipmaps))
    {
        return Texture2DConstruct(tex,image->width,image->height);
    }

    return NULL;
}


TextureCube* CreateCubemap(const Image* image, int use_mipmaps)
{
    GLuint tex = 0;
    int base_level = 0;

    if (!image->cubemap)
    {
        LogError("createCubemap: %s: is not a cube map. Use dds::createTexture\n", image->name);
        return NULL;
    }

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    CheckGL(image->name);

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    LoadTextureFromImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, base_level, image, use_mipmaps);
    base_level += image->num_mipmaps ? image->num_mipmaps : 1;

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return TextureCubeConstruct(tex);
}

