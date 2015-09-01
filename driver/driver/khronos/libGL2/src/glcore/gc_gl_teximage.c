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


#include "gc_gl_context.h"
#include "gc_gl_names_inline.c"
#include "gc_gl_debug.h"
#include "gl/gl_device.h"
#include "dri/viv_lock.h"

extern GLvoid __glSetTexMaxLevelUsed(__GLtextureObject *);

/*macro to 2D texture target and get the texture object*/
#define TEXIMAGE2D_SUB_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D_ARRAY_EXT: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX]; \
        break; \
    case GL_TEXTURE_2D: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX]; \
        break; \
    case GL_TEXTURE_RECTANGLE_ARB: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX]; \
        break; \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: \
        if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled) \
        { \
            __glSetError(GL_INVALID_ENUM); \
            return; \
        } \
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX]; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }


#define TEXIMAGE2D_COPY_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D_ARRAY_EXT: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX]; \
        tex->arrays = height; \
        break; \
    case GL_TEXTURE_2D: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX]; \
        tex->arrays = 1; \
        break; \
    case GL_TEXTURE_RECTANGLE_ARB: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX]; \
        tex->arrays = 1; \
        break; \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: \
        if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled) \
        { \
            __glSetError(GL_INVALID_ENUM); \
            return; \
        } \
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX]; \
        tex->arrays = 6; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }


#define TEXIMAGE2D_CHECK_TARGET_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D_ARRAY_EXT: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX]; \
        tex->arrays = height; \
        proxy = GL_FALSE; \
        break; \
    case GL_TEXTURE_2D: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX]; \
        proxy = GL_FALSE; \
        tex->arrays = 1;   \
        break; \
    case GL_TEXTURE_RECTANGLE_ARB: \
        face = 0; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX]; \
        tex->arrays = 1;   \
        proxy = GL_FALSE; \
        break; \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y: \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z: \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: \
        if(!__glExtension[INDEX_ARB_texture_cube_map].bEnabled && !__glExtension[INDEX_EXT_texture_cube_map].bEnabled) \
        { \
            __glSetError(GL_INVALID_ENUM); \
            return; \
        } \
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X; \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX]; \
        tex->arrays = 6;   \
        proxy = GL_FALSE; \
        break; \
    case GL_PROXY_TEXTURE_1D_ARRAY_EXT: \
        face = 0;  \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_1D_ARRAY_INDEX]; \
        tex->arrays = height; \
        proxy = GL_TRUE; \
        break; \
    case GL_PROXY_TEXTURE_2D: \
        face = 0;  \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_2D_INDEX]; \
        proxy = GL_TRUE; \
        tex->arrays = 1;   \
        break; \
    case GL_PROXY_TEXTURE_RECTANGLE_ARB: \
        face = 0;  \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_RECTANGLE_INDEX]; \
        proxy = GL_TRUE; \
        tex->arrays = 1;   \
        break; \
    case GL_PROXY_TEXTURE_CUBE_MAP: \
        face = 0; \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_CUBEMAP_INDEX]; \
        proxy = GL_TRUE; \
        tex->arrays = 6;   \
        break;           \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }

/*macro to 1D texture target and get the texture object*/
#define TEXIMAGE1D_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX]; \
        tex->arrays = 1;   \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }

/*macro to 1D texture target and get the texture object*/
#define TEXIMAGE1D_SUB_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX]; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }


#define TEXIMAGE1D_CHECK_TARGET_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_1D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX]; \
        tex->arrays = 1;   \
        proxy = GL_FALSE; \
        break; \
    case GL_PROXY_TEXTURE_1D: \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_1D_INDEX]; \
        tex->arrays = 1;   \
        proxy = GL_TRUE; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }

/*macro to 3D texture target and get the texture object*/
#define TEXIMAGE3D_SUB_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_2D_ARRAY_EXT: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]; \
        break; \
    case GL_TEXTURE_3D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX]; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }

#define TEXIMAGE3D_COPY_CHECK_TARGET_NO_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_2D_ARRAY_EXT: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]; \
        startIndex = zoffset;\
        break; \
    case GL_TEXTURE_3D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX]; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }


#define TEXIMAGE3D_COPY_CHECK_TARGET_NO_PROXY_EXT() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_2D_ARRAY_EXT: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]; \
        break; \
    case GL_TEXTURE_3D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX]; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }


#define TEXIMAGE3D_CHECK_TARGET_PROXY() \
    activeUnit = gc->state.texture.activeTexIndex;\
    switch(target) \
    { \
    case GL_TEXTURE_2D_ARRAY_EXT: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX]; \
        tex->arrays = depth; \
        proxy = GL_FALSE;   \
        break; \
    case GL_TEXTURE_3D: \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX]; \
        proxy = GL_FALSE; \
        tex->arrays = 1; \
        break; \
    case GL_PROXY_TEXTURE_2D_ARRAY_EXT: \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_2D_ARRAY_INDEX]; \
        tex->arrays = depth; \
        proxy = GL_TRUE; \
        break; \
    case GL_PROXY_TEXTURE_3D: \
        tex = &gc->texture.proxyTextures[__GL_TEXTURE_3D_INDEX]; \
        proxy = GL_TRUE; \
        tex->arrays = 1; \
        break; \
    default: \
        __glSetError(GL_INVALID_ENUM); \
        return; \
    }

#define GETTEXIMAGE_CHECK_TARGET_LOD_FACE_NO_PROXY()        \
    activeUnit = gc->state.texture.activeTexIndex;  \
    switch(target) \
    { \
    case GL_TEXTURE_1D:     \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];   \
        break;              \
    case GL_TEXTURE_2D:     \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];   \
        break;              \
    case GL_TEXTURE_1D_ARRAY_EXT:     \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];   \
        break;              \
    case GL_TEXTURE_2D_ARRAY_EXT:     \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];   \
        break;              \
    case GL_TEXTURE_RECTANGLE_ARB:     \
        if (level != 0)       \
        {                                               \
            __glSetError(GL_INVALID_VALUE);             \
            return;                                     \
        }                   \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];   \
        break; \
    case GL_TEXTURE_3D:     \
        face = 0;           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];   \
        break;              \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:            \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:            \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:            \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:            \
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:            \
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:            \
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;                                           \
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];            \
        break;                                                                                    \
    default:                                                                                      \
        __glSetError(GL_INVALID_ENUM);              \
        return;                                     \
    }                                               \
    if (level < 0 || level >= (GLint)gc->constants.maxNumTextureLevels)       \
    {                                               \
        __glSetError(GL_INVALID_VALUE);             \
        return;                                     \
    }

/*s_texel.c*/
extern const __GLdeviceFormatInfo __glDevfmtInfo[];
extern const __GLdeviceFormatInfo __glNullDevfmt;

/*s_pixel.c*/
extern GLboolean __glCheckUnpackArgs(__GLcontext *gc, GLenum format, GLenum type);

GLvoid __glReleaseTexImageImplicit(__GLcontext *gc,GLvoid *hPbuffer,GLenum iBuffer,__GLtextureObject *tex);

__GL_INLINE GLvoid __glSetTexImageDirtyBit(__GLcontext *gc, __GLtextureObject* tex, GLuint dirtyBit)
{
    GLuint i;

    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++) {
        if (tex->name == gc->texture.units[i].boundTextures[tex->targetIndex]->name) {
            __GL_SET_TEX_UNIT_BIT(gc, i, dirtyBit);
        }
    }
}

GLboolean __glIsIntegerDataFormat(GLenum  srcFormat)
{
    switch (srcFormat)
    {
    case GL_RED_INTEGER_EXT:
    case GL_GREEN_INTEGER_EXT:
    case GL_BLUE_INTEGER_EXT:
    case GL_ALPHA_INTEGER_EXT:
    case GL_RGB_INTEGER_EXT:
    case GL_RGBA_INTEGER_EXT:
    case GL_BGR_INTEGER_EXT:
    case GL_BGRA_INTEGER_EXT:
    case GL_LUMINANCE_INTEGER_EXT:
    case GL_LUMINANCE_ALPHA_INTEGER_EXT:
        return GL_TRUE;
    default:
        return GL_FALSE;
    }
}

GLboolean __glIsInternalFormatCompressed( GLint internalFormat )
{
    switch (internalFormat)
    {
            /*generic compressed format*/
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:

            /*extension "GL_S3_s3tc"*/
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB_S3TC:
        case GL_COMPRESSED_RGB4_S3TC:
        case GL_COMPRESSED_RGBA_S3TC:
        case GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC:
        case GL_COMPRESSED_RGB4_ALPHA4_S3TC:
        case GL_COMPRESSED_LUMINANCE_S3TC:
        case GL_COMPRESSED_LUMINANCE4_S3TC:
        case GL_COMPRESSED_LUMINANCE_ALPHA_S3TC:
        case GL_COMPRESSED_LUMINANCE4_ALPHA_S3TC:
#endif

            /*GL_EXT_texture_compression_s3tc*/
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:

        /* GL_EXT_texture_compression_latc */
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:

        /* GL_EXT_texture_compression_rgtc */
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:

        /* GL_EXT_texture_sRGB */
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:

            return (GL_TRUE);



        default:
            return (GL_FALSE);
    }
}

GLint __glCompressedTexImageSize(GLint internalFormat, GLint width, GLint height, GLint * blockSize)
{
    /*
    * Note : If we change the map from generic compress format to
    *           specific compress format. We should change their block
    *           size accordingly.
    */
    switch( internalFormat )
    {
        case GL_RGB_S3TC:
        case GL_RGB4_S3TC:
        case GL_COMPRESSED_RGB_ARB:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            *blockSize = 8;
            break;
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB4_ALPHA4_S3TC:
#endif
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGBA_S3TC:
#endif
        case GL_COMPRESSED_RGBA_ARB:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC:
#endif
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            *blockSize = 16;
            break;

        default:
            GL_ASSERT(0);
            return 0;
    }
    return ((width + 3)/4) * ((height + 3)/4) * (*blockSize);
}

GLboolean __glCheckGetTexImageArgs(GLenum format, GLenum type)
{
    switch(format)
    {
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGR:
        case GL_BGRA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_DEPTH_COMPONENT:
            break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
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
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            switch (format)
            {
                case GL_RGB:
                case GL_BGR:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return GL_FALSE;
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
                case GL_ABGR_EXT:
                case GL_BGRA:
                break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return GL_FALSE;
            }
            break;
        case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:
            if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
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
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
    }
    return GL_TRUE;
}

GLboolean __glCheckCompressedTexImageFormat(GLint internalFormat, GLint border, GLboolean proxy)
{
    switch(internalFormat)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:

        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:

        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            if( border != 0)
            {
                if(proxy) return GL_FALSE;

                __glSetError(GL_INVALID_OPERATION);
                return GL_FALSE;
            }
            break;

        default:
            if(proxy) return GL_FALSE;
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean __glCheckTexImageArgs(__GLcontext *gc,
                                GLenum target,
                                GLint lod,
                                GLint internalFormat,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth,
                                GLint border,
                                GLenum format,
                                GLenum type)
{
    GLsizei wt = width - 2*border, ht = height - 2* border, dt = depth - 2*border;
    GLint max_lod = (GLint)(gc->constants.maxNumTextureLevels -1);
    GLint max_array = (GLint)gc->constants.maxTextureArraySize;

    if(target == GL_TEXTURE_1D_ARRAY_EXT)
    {
        ht = 1;
        if((height <= 0) || (height > max_array))
            goto bad_value;
    }

    if(target == GL_TEXTURE_2D_ARRAY_EXT)
    {
        dt = 1;
        if((depth <= 0) || (depth > max_array) )
            goto bad_value;
    }

    /*
    1. spec says texturing in color index mode is undefined, we treat it as an invalid operation
    2. GL_STENCIL_INDEX format is invalide for data format
    3. GL_COLOR_INDEX format is invalide for texturing,but can be upload as host memory data format
    */
    if(!gc->modes.rgbMode || format == GL_STENCIL_INDEX )
    {
bad_operation:
        __glSetError(GL_INVALID_OPERATION);
        return GL_FALSE;
    }

    /*check format and type*/
    if( GL_FALSE == __glCheckUnpackArgs(gc, format, type) )
    {
        return GL_FALSE;
    }

    /*check border, lod, width, height, depth*/
    if (border < 0 || border > 1 ||
        lod < 0 || lod > max_lod ||
        wt < 0 || wt > (1 << (max_lod -lod) ) ||
        ht < 0 || ht > (1 << (max_lod -lod)) ||
        dt < 0 || dt >(1 << (max_lod -lod) ) )

    {
bad_value:
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }

    if(wt * ht * dt == 0 )
    {
        return GL_FALSE;
    }

    /*check internalFormat*/
    switch (internalFormat)
    {
            /*color component*/
        case 1:
        case 2:
        case 3:
        case 4:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGB5_A1:
        case GL_RGB10_A2:
        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            if(format == GL_DEPTH_COMPONENT )
            {
                goto bad_operation;
            }
            break;

            /*depth component*/
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            if(!__glExtension[INDEX_ARB_depth_texture].bEnabled)
            {
    bad_enum:
                __glSetError(GL_INVALID_ENUM);
                return GL_FALSE;
            }
            if(format != GL_DEPTH_COMPONENT || !gc->modes.haveDepthBuffer)
            {
                goto bad_operation;
            }
            switch(target)
            {
            case GL_TEXTURE_1D:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_RECTANGLE_ARB:
            case GL_PROXY_TEXTURE_1D:
            case GL_PROXY_TEXTURE_2D:
            case GL_PROXY_TEXTURE_RECTANGLE_ARB:

            /* for layered framebuffer */
            case GL_TEXTURE_1D_ARRAY_EXT:
            case GL_TEXTURE_2D_ARRAY_EXT:
            case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
            case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            case GL_PROXY_TEXTURE_CUBE_MAP:

                break;
            default:
                goto  bad_operation;
            }
            break;

            /*the six generic compressed texture format*/
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;

            /*extension "GL_EXT_texture_compression_s3tc" */
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            if(!__glExtension[INDEX_EXT_texture_compression_s3tc].bEnabled)
                goto bad_enum;
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;

            /*extension "GL_S3_s3tc"*/

#ifdef GL_S3_S3TC_EX
        case GL_COMPRESSED_RGB_S3TC:
        case GL_COMPRESSED_RGB4_S3TC:
        case GL_COMPRESSED_RGBA_S3TC:
        case GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC:
        case GL_COMPRESSED_RGB4_ALPHA4_S3TC:
        case GL_COMPRESSED_LUMINANCE_S3TC:
        case GL_COMPRESSED_LUMINANCE4_S3TC:
        case GL_COMPRESSED_LUMINANCE_ALPHA_S3TC:
        case GL_COMPRESSED_LUMINANCE4_ALPHA_S3TC:
            if(!__glExtension[INDEX_S3_s3tc].bEnabled)
                goto bad_enum;
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            switch (format)
            {
                case 3:
                case 4:
                case GL_RGB:
                case GL_RGB8:
                case GL_RGBA:
                case GL_RGBA8:
                case GL_BGR_EXT:
                case GL_BGRA_EXT:
                case GL_RGB_S3TC:
                case GL_RGB4_S3TC:
                case GL_COMPRESSED_RGBA_S3TC:
                case GL_COMPRESSED_LUMINANCE_S3TC:
                case GL_COMPRESSED_LUMINANCE_ALPHA_S3TC:
                    break;
                default:
                    goto bad_value;
            }
            break;
#endif
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            if(!__glExtension[INDEX_EXT_texture_compression_latc].bEnabled)
                goto bad_enum;
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;

        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            if(!__glExtension[INDEX_EXT_texture_compression_rgtc].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;
        /* extension GL_EXT_texture_integer */
        case GL_RGBA32UI_EXT:
        case GL_RGBA32I_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA16I_EXT:
        case GL_RGBA8UI_EXT:
        case GL_RGBA8I_EXT:
        case GL_RGB32UI_EXT:
        case GL_RGB32I_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGB16I_EXT:
        case GL_RGB8UI_EXT:
        case GL_RGB8I_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_INTENSITY8I_EXT:
            if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
                goto bad_enum;

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
                break;
            default:
                goto bad_operation;
            }
            break;

        /* extension GL_EXT_texture_sRGB */
        case GL_SRGB:
        case GL_SRGB8:
        case GL_SRGB_ALPHA:
        case GL_SRGB8_ALPHA8:
        case GL_SLUMINANCE_ALPHA:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_SLUMINANCE:
        case GL_SLUMINANCE8:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_SLUMINANCE:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            if (!__glExtension[INDEX_EXT_texture_sRGB].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;

        /* extension GL_EXT_texture_shared_exponent */
        case GL_RGB9_E5_EXT:
             if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

             break;

        /* extension GL_ARB_texture_float */
        case GL_RGBA32F_ARB:
        case GL_RGB32F_ARB:
        case GL_ALPHA32F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_RGBA16F_ARB:
        case GL_RGB16F_ARB:
        case GL_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
            if (!__glExtension[INDEX_ARB_texture_float].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;
        case GL_R11F_G11F_B10F_EXT:
            if (!__glExtension[INDEX_EXT_packed_float].bEnabled)
                goto bad_enum;

            if (format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;

        default:
            goto bad_enum;
    }

    switch(format)
    {
        case GL_BLUE_INTEGER_EXT:
        case GL_GREEN_INTEGER_EXT:
        case GL_ALPHA_INTEGER_EXT:
        case GL_RGB_INTEGER_EXT:
        case GL_RGBA_INTEGER_EXT:
        case GL_BGR_INTEGER_EXT:
        case GL_BGRA_INTEGER_EXT:
        case GL_LUMINANCE_INTEGER_EXT:
        case GL_LUMINANCE_ALPHA_INTEGER_EXT:
            switch(internalFormat)
            {
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    goto bad_operation;

            }
            break;
        default:
            break;
    }

    return GL_TRUE;
}

GLboolean __glCheckTexBufferArgs(GLenum target, GLenum internalformat, GLint * components, GLint * baseSize)
{
    if(target != GL_TEXTURE_BUFFER_EXT)
    {
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }

    switch(internalformat)
    {
        case GL_ALPHA8:
        case GL_LUMINANCE8:
        case GL_INTENSITY8:
        case GL_ALPHA8I_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_INTENSITY8I_EXT:
        case GL_ALPHA8UI_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_INTENSITY8UI_EXT:
            *baseSize = sizeof(GLubyte);
            *components = 1;
            break;
        case GL_ALPHA16:
        case GL_LUMINANCE16:
        case GL_INTENSITY16:
        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_ALPHA16I_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_ALPHA16UI_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_INTENSITY16UI_EXT:
            *baseSize = sizeof(GLushort);
            *components = 1;
            break;
        case GL_ALPHA32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_INTENSITY32F_ARB:
            *baseSize = sizeof(GLfloat);
            *components = 1;
            break;
        case GL_ALPHA32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_ALPHA32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_INTENSITY32UI_EXT:
            *baseSize = sizeof(GLuint);
            *components = 1;
            break;


        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
            *components = 2;
            *baseSize = sizeof(GLubyte);
            break;

        case GL_LUMINANCE16_ALPHA16:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
            *components = 2;
            *baseSize = sizeof(GLushort);
            break;

        case GL_LUMINANCE_ALPHA32F_ARB:
            *components = 2;
            *baseSize = sizeof(GLfloat);
            break;
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
            *components = 2;
            *baseSize = sizeof(GLuint);
            break;


        case GL_RGBA8:
        case GL_RGBA8I_EXT:
        case GL_RGBA8UI_EXT:
            *components = 4;
            *baseSize = sizeof(GLubyte);
            break;
        case GL_RGBA16:
        case GL_RGBA16I_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA16F_ARB:
            *components = 4;
            *baseSize = sizeof(GLushort);
            break;
        case GL_RGBA32F_ARB:
            *components = 4;
            *baseSize = sizeof(GLfloat);
            break;
        case GL_RGBA32I_EXT:
        case GL_RGBA32UI_EXT:
            *components = 4;
            *baseSize = sizeof(GLuint);
            break;;

        default:
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
    }

    return GL_TRUE;
}

GLboolean __glCheckTexSubImageArgs(__GLcontext *gc,
                                   __GLtextureObject *tex,
                                   GLuint face,
                                   GLint lod,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLint zoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLenum format,
                                   GLenum type)
{
     __GLmipMapLevel *mipmap;
    GLint max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check texture array */
    if(tex->targetIndex == __GL_TEXTURE_1D_ARRAY_INDEX)
    {
        if((yoffset < 0) || ((GLint)(yoffset + height) > tex->arrays))
        {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
        height = 1;
        yoffset = 0;
    }

    if(tex->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX)
    {
        if((zoffset < 0) || ((GLint)(zoffset + depth) > tex->arrays))
        {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
        depth = 1;
        zoffset = 0;
    }


    /* check lod */
    if (lod < 0 || lod > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }

    mipmap = &tex->faceMipmap[face][lod];
    if(!gc->modes.rgbMode || format == GL_STENCIL_INDEX || format == GL_COLOR_INDEX)
    {
        __glSetError(GL_INVALID_OPERATION);
        return GL_FALSE;
    }

    if( width < 0 || height < 0 ||depth < 0 )
    {
        __glSetError( GL_INVALID_VALUE );
        return GL_FALSE;
    }

    /*check format and type*/
    if( GL_FALSE == __glCheckUnpackArgs(gc, format, type) )
    {
        return GL_FALSE;
    }
    switch(mipmap->requestedFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        if(format != GL_DEPTH_COMPONENT)
        {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
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
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;

        }
        break;

    case GL_RGB9_E5_EXT:
        if(!__glExtension[INDEX_EXT_texture_shared_exponent].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    case GL_R11F_G11F_B10F_EXT:
        if (!__glExtension[INDEX_EXT_packed_float].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;

    default:
        if(format == GL_DEPTH_COMPONENT)
        {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
    }

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
        switch(mipmap->requestedFormat)
        {
        case GL_RGBA32UI_EXT:
        case GL_RGBA32I_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA16I_EXT:
        case GL_RGBA8UI_EXT:
        case GL_RGBA8I_EXT:
        case GL_RGB32UI_EXT:
        case GL_RGB32I_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGB16I_EXT:
        case GL_RGB8UI_EXT:
        case GL_RGB8I_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_INTENSITY8I_EXT:
            break;
        default:
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;

        }
        break;

    default:
        break;
    }


    /*check xoffset, yoffset, zoffset, width, height, depth*/

    if( !mipmap->compressed )
    {
        if( xoffset < -mipmap->border || (xoffset + width) > (mipmap->width - mipmap->border) ||
            yoffset < -mipmap->border || (yoffset + height) > (mipmap->height - mipmap->border) ||
            zoffset < -mipmap->border || (zoffset + depth) > (mipmap->depth - mipmap->border) )
        {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
    }
    else
    {
        if( (xoffset & 0x3) || (yoffset & 0x3)  ||
            ( (width & 0x3) && width != mipmap->width) ||
            ((height & 0x3) && height != mipmap->height) )
        {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
    }

    if(width * height * depth == 0 )
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLenum __glTextureBaseFormat(GLint internalFormat)
{
    switch(internalFormat)
    {
        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            return GL_ALPHA;

        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            return GL_DEPTH_COMPONENT;

        case 1:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            return GL_LUMINANCE;

        case 2:
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

        case 3:
        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            return GL_RGB;

        case 4:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGB5_A1:
        case GL_RGB10_A2:
            return GL_RGBA;

            /*generic compressed format*/
        case GL_COMPRESSED_ALPHA:
            return GL_ALPHA;
        case GL_COMPRESSED_LUMINANCE:
            return GL_LUMINANCE;
        case GL_COMPRESSED_LUMINANCE_ALPHA:
            return GL_LUMINANCE_ALPHA;
        case GL_COMPRESSED_INTENSITY:
            return GL_INTENSITY;
        case GL_COMPRESSED_RGB:
            return GL_RGB;
        case GL_COMPRESSED_RGBA:
            return GL_RGBA;

            /*extension "GL_S3_s3tc"*/
#ifdef GL_S3_S3TC_EX

        case GL_COMPRESSED_RGB_S3TC:
        case GL_COMPRESSED_RGB4_S3TC:
            return GL_RGB;
        case GL_COMPRESSED_RGBA_S3TC:
        case GL_COMPRESSED_RGB4_COMPRESSED_ALPHA4_S3TC:
        case GL_COMPRESSED_RGB4_ALPHA4_S3TC:
            return GL_RGBA;
        case GL_COMPRESSED_LUMINANCE_S3TC:
        case GL_COMPRESSED_LUMINANCE4_S3TC:
            return GL_LUMINANCE;
        case GL_COMPRESSED_LUMINANCE_ALPHA_S3TC:
        case GL_COMPRESSED_LUMINANCE4_ALPHA_S3TC:
            return GL_LUMINANCE_ALPHA;
#endif
            /*GL_EXT_texture_compression_s3tc*/
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return GL_RGBA;
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return GL_RGB;

        /* GL_EXT_texture_compression_latc */
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            return GL_LUMINANCE;
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            return GL_LUMINANCE_ALPHA;

        /* GL_EXT_texture_compression_rgtc */
        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            return GL_RGB;

        /* GL_EXT_texture_integer */
        case GL_ALPHA32UI_EXT:
        case GL_ALPHA32I_EXT:
        case GL_ALPHA16UI_EXT:
        case GL_ALPHA16I_EXT:
        case GL_ALPHA8UI_EXT:
        case GL_ALPHA8I_EXT:
            return GL_ALPHA;

        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE8I_EXT:
            return GL_LUMINANCE;

        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
            return GL_LUMINANCE_ALPHA;

        case GL_INTENSITY32UI_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_INTENSITY8I_EXT:
            return GL_INTENSITY;

        case GL_RGB32UI_EXT:
        case GL_RGB32I_EXT:
        case GL_RGB16UI_EXT:
        case GL_RGB16I_EXT:
        case GL_RGB8UI_EXT:
        case GL_RGB8I_EXT:
            return GL_RGB;

        case GL_RGBA32UI_EXT:
        case GL_RGBA32I_EXT:
        case GL_RGBA16UI_EXT:
        case GL_RGBA16I_EXT:
        case GL_RGBA8UI_EXT:
        case GL_RGBA8I_EXT:
            return GL_RGBA;

        /*extension GL_EXT_texture_sRGB*/
        case GL_SRGB:
        case GL_SRGB8:
        case GL_COMPRESSED_SRGB:
            return GL_RGB;

        case GL_SRGB_ALPHA:
        case GL_SRGB8_ALPHA8:
        case GL_COMPRESSED_SRGB_ALPHA:
            return GL_RGBA;

        case GL_SLUMINANCE_ALPHA:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
            return GL_LUMINANCE_ALPHA;

        case GL_SLUMINANCE:
        case GL_SLUMINANCE8:
        case GL_COMPRESSED_SLUMINANCE:
            return GL_LUMINANCE;

        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
            return GL_RGB;

        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            return GL_RGBA;
        /*extension GL_EXT_texture_shared_exponent*/
        case GL_RGB9_E5_EXT:
            return GL_RGB;

        /*extension GL_ARB_texture_float*/
        case GL_RGBA32F_ARB:
        case GL_RGBA16F_ARB:
            return GL_RGBA;

        case GL_RGB32F_ARB:
        case GL_RGB16F_ARB:
            return GL_RGB;

        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE16F_ARB:
            return GL_LUMINANCE;

        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
            return GL_LUMINANCE_ALPHA;

        case GL_INTENSITY32F_ARB:
        case GL_INTENSITY16F_ARB:
            return GL_INTENSITY;

        case GL_ALPHA32F_ARB:
        case GL_ALPHA16F_ARB:
            return GL_ALPHA;

        /*extension GL_EXT_packed_float */
        case GL_R11F_G11F_B10F_EXT:
            return GL_RGB;


        default:
            /*a wrong internal format is passed here*/
            GL_ASSERT(0);
            return 0;
    }
}

GLboolean __glIsIntegerInternalFormat(GLenum internalFormat)
{
    switch(internalFormat)
    {
    case GL_ALPHA32UI_EXT:
    case GL_ALPHA32I_EXT:
    case GL_ALPHA16UI_EXT:
    case GL_ALPHA16I_EXT:
    case GL_ALPHA8UI_EXT:
    case GL_ALPHA8I_EXT:

    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:

    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:


    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:

    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:

    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
        return GL_TRUE;

    default:
        return GL_FALSE;

    }
}

GLboolean __glIsFloatInternalFormat(GLenum internalFormat)
{
    switch(internalFormat)
    {
    /* GL_ARB_texture_float*/
    case GL_RGBA32F_ARB:
    case GL_RGB32F_ARB:
    case GL_ALPHA32F_ARB:
    case GL_INTENSITY32F_ARB:
    case GL_LUMINANCE32F_ARB:
    case GL_LUMINANCE_ALPHA32F_ARB:
    case GL_RGBA16F_ARB:
    case GL_RGB16F_ARB:
    case GL_ALPHA16F_ARB:
    case GL_INTENSITY16F_ARB:
    case GL_LUMINANCE16F_ARB:
    case GL_LUMINANCE_ALPHA16F_ARB:
    /* GL_EXT_packed_float */
    case GL_R11F_G11F_B10F_EXT:
    /* GL_EXT_texture_shared_component*/
    case GL_RGB9_E5_EXT:

        return GL_TRUE;
    default:
        return GL_FALSE;
    }
}

GLboolean __glSetMipmapLevelInfo(__GLcontext *gc, __GLtextureObject *tex, GLint face, GLint level,
                                 __GLdeviceFormat chosenFormat, GLint internalFormat, GLenum format, GLenum type,
                                 GLsizei width, GLsizei height, GLsizei depth, GLint border ,GLboolean proxy)
{
    GLenum baseFormat = __glTextureBaseFormat( internalFormat);
    __GLmipMapLevel *mipmap = &tex->faceMipmap[face][level];
    GLint arrays = 1;
    GLint array_index;
    __GLmipMapLevel *currentMipmap;

    if( baseFormat != mipmap->baseFormat )
    {
        mipmap->baseFormat = baseFormat;
        __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_FORMAT_CHANGED_BIT);
    }
    mipmap->deviceFormat = &__glDevfmtInfo[chosenFormat];
    mipmap->compressed   = __glIsInternalFormatCompressed(internalFormat);

    /*if the requested internal format is compressed, but the chosen format is not compressed,
    that means we choose not to compress the texture src image. As spec says, the compressed
    internal format is replaced by the base format*/
    if( mipmap->compressed && !mipmap->deviceFormat->compressed )
    {
        mipmap->compressed = GL_FALSE;
        internalFormat = baseFormat;
    }
    mipmap->requestedFormat = internalFormat;

    mipmap->border = border;
    mipmap->width = width;
    mipmap->height = height;
    mipmap->depth = depth;
    mipmap->arrays = arrays;
    mipmap->format = format;
    mipmap->type = type;

    mipmap->width2 = width - border*2;

    switch( tex->targetIndex )
    {
    case __GL_TEXTURE_1D_INDEX:
    case __GL_TEXTURE_BUFFER_INDEX:
        mipmap->height2 = 1;
        mipmap->depth2 =  1;
        break;
    case __GL_TEXTURE_1D_ARRAY_INDEX:
        mipmap->height2 = 1;
        mipmap->depth2 =  1;
        arrays = height;
        height = 1;
        break;

    case __GL_TEXTURE_2D_INDEX:
    case __GL_TEXTURE_CUBEMAP_INDEX:
    case __GL_TEXTURE_RECTANGLE_INDEX:
        mipmap->height2 = height - border*2;
        mipmap->depth2 =  1;
        break;
    case __GL_TEXTURE_2D_ARRAY_INDEX:
        mipmap->height2 = height - border*2;
        mipmap->depth2 =  1;
        arrays = depth;
        depth = 1;
        break;

    case __GL_TEXTURE_3D_INDEX:
        mipmap->height2 = height - border*2;
        mipmap->depth2 =  depth - border*2;
        break;
    }

    mipmap->width2f  = (GLfloat)mipmap->width2;
    mipmap->height2f = (GLfloat)mipmap->height2;
    mipmap->depth2f = (GLfloat)mipmap->depth2;

    mipmap->widthLog2 = __glFloorLog2(mipmap->width2);
    mipmap->heightLog2 = __glFloorLog2(mipmap->height2);
    mipmap->depthLog2 = __glFloorLog2(mipmap->depth2);

    mipmap->seqNumber++;

    if (face==0 && level == tex->params.baseLevel)
    {
        __glSetTexMaxLevelUsed( tex);
    }

    /* Initialize other levels if it is 1d or 2d array */
    if (arrays > 1)
    {
        mipmap->height = height;
        mipmap->depth = depth;
        mipmap->arrays = arrays;
        for (array_index = 1; array_index < arrays; array_index++)
        {
            currentMipmap = &tex->faceMipmap[array_index][level];
            __GL_MEMCOPY(currentMipmap, mipmap, sizeof(__GLmipMapLevel));
        }
    }

    return GL_TRUE;
}

/* Set the image of this texture level to NULL */
GLvoid __glSetMipmapLevelNULL(__GLcontext *gc, __GLmipMapLevel *mipmap)
{
    /* Set dimensions to 0 */
    mipmap->width = 0;
    mipmap->height = 0;
    mipmap->depth = 0;
    mipmap->width2 = 0;
    mipmap->height2 = 0;
    mipmap->depth2 = 0;
    mipmap->width2f  = 0.f;
    mipmap->height2f = 0.f;
    mipmap->depth2f = 0.f;
    mipmap->widthLog2 = 0;
    mipmap->heightLog2 = 0;
    mipmap->depthLog2 = 0;

    /* Increase the sequence number */
    mipmap->seqNumber++;
}

GLvoid __glGenerateNextMipmap(__GLmipMapLevel *mipmap, __GLmipMapLevel* next, GLenum filterMode)
{
    /*Not implemented yet*/
    GL_ASSERT(0);
}

GLvoid __glGenerateMipmaps(__GLcontext *gc, __GLtextureObject *tex, GLint face, GLint baseLevel)
{
    __GLmipMapLevel *base = &tex->faceMipmap[face][baseLevel];
    GLint w = base->width2 >> 1;
    GLint h = base->height2 >> 1;
    GLint d = base->depth2 >> 1;
    GLint lod = baseLevel + 1;
    GLint maxLevel;

    while(w > 0 || h > 0 || d > 0)
    {
        if(w == 0) w =1;
        if( h == 0) h = 1;
        if(d == 0) d = 1;

        __glSetMipmapLevelInfo(gc, tex, face,lod,
                               base->deviceFormat->devfmt, base->requestedFormat,
                               base->format, base->type,
                               w +(base->width - base->width2),
                               h + (base->height - base->height2),
                               d + (base->depth - base->depth2),
                               base->border, GL_FALSE);

        w >>= 1;
        h >>= 1;
        d >>= 1;
        lod++;
    }
    maxLevel = lod -1;
    if(maxLevel == baseLevel )
    {
        return;
    }

    if(GL_FALSE == (*gc->dp.generateMipmaps)(gc, tex, face, maxLevel))
    {
        GLint faces = (tex->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX )?6:1;
        GLint i;
        tex->forceBaseLeve = GL_TRUE;
        __glSetTexMaxLevelUsed(tex);
        __GL_SET_TEX_UNIT_BIT(gc, gc->state.texture.activeTexIndex, __GL_TEXPARAM_MAX_LEVEL_BIT);

        /*software mipmap generation*/
        for(i = 0; i < faces; i++)
        {
            for(lod = baseLevel + 1; lod <= maxLevel; lod++ )
            {
                __glGenerateNextMipmap(&tex->faceMipmap[i][lod -1],
                    &tex->faceMipmap[i][lod], GL_NEAREST);
            }
        }
    }
}

__GL_INLINE GLvoid __glTextureCubeWorkAround(__GLcontext *gc,__GLtextureObject * tex, GLint face)
{

    if ((tex->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX) && (face != 0) && (tex->params.generateMipmap == 1))
    {
        if (tex->faceMipmap[0][tex->params.baseLevel].compressed)
        {
            tex->forceBaseLeve = GL_TRUE;
            __glSetTexMaxLevelUsed(tex);
            __GL_SET_TEX_UNIT_BIT(gc, gc->state.texture.activeTexIndex, __GL_TEXPARAM_MAX_LEVEL_BIT);
            tex->params.generateMipmap = 0;
        }
    }
}


/*
** Reference to const __GLdeviceFormatInfo __glDevfmtInfo[__GL_DEVFMT_MAX]
** for detail pixel format info.
*/
GLvoid __glQueryCompressedSrcFormatAndType(GLint internalFormat,
                                         GLenum *format,
                                         GLenum *type)
{
    switch(internalFormat)
    {
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            *format = GL_LUMINANCE;
            *type = __GL_LATC1_BLOCK;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            *format = GL_LUMINANCE;
            *type = __GL_SIGNED_LATC1_BLOCK;
            break;

        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            *format = GL_LUMINANCE_ALPHA;
            *type = __GL_LATC2_BLOCK;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            *format = GL_LUMINANCE_ALPHA;
            *type = __GL_SIGNED_LATC2_BLOCK;
            break;

        case GL_COMPRESSED_RED_RGTC1_EXT:
            *format = GL_RED;
            *type = __GL_RGTC1_BLOCK;
            break;

        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
            *format = GL_RED;
            *type = __GL_SIGNED_RGTC1_BLOCK;
            break;

        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
            *format = GL_RED_GREEN_VIVPRIV;
            *type = __GL_RGTC2_BLOCK;
            break;

        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            *format = GL_RED_GREEN_VIVPRIV;
            *type = __GL_SIGNED_RGTC2_BLOCK;
            break;
    }
}

GLvoid APIENTRY __glim_TexImage3D(GLenum target,
                                GLint lod,
                                GLint internalFormat,
                                GLsizei width,
                                GLsizei height,
                                GLsizei depth,
                                GLint border,
                                GLenum format,
                                GLenum type,
                                const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLboolean proxy;
    GLenum oldError;
    GLuint unpackBuffer;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexImage3D", DT_GLenum, target, DT_GLint, lod, DT_GLint, internalFormat, DT_GLsizei, width, DT_GLsizei, height,
        DT_GLsizei, depth, DT_GLint, border, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE3D_CHECK_TARGET_PROXY();

    if(proxy)/*if target is a proxy texture, no error code is generated*/
    {
        oldError = gc->error;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalFormat, width, height, depth, border, format, type) )
    {
        if(proxy)
        {
            __GL_MEMZERO(&tex->faceMipmap[0][lod], sizeof(__GLmipMapLevel));
            tex->faceMipmap[0][lod].deviceFormat = &__glNullDevfmt;
            __glSetError(oldError);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat,tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if(__glSetMipmapLevelInfo(gc, tex, 0, lod, chosenFormat, internalFormat,
                              format, type, width, height, depth, border, proxy) == GL_FALSE)
    {
          return;
    }

    if(proxy)
    {
        return;
    }

    /* The image is from unpack buffer object? */
    unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

    if (unpackBuffer > 0)
    {
        /* Go SW path directly. */
        __GLbufferObject *bufObj;

        bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
        if (bufObj == NULL)
        {
            GL_ASSERT(0);
            return;
        }

        /* Add the offset */
        buf = bufObj->systemMemCache + ((GLbyte *)buf - (GLbyte *)0);
    }

    LINUX_LOCK_FRAMEBUFFER(gc);
    (*gc->dp.texImage3D)(gc, tex, lod, buf);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);

#if GL_EXT_framebuffer_object
    if(tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        /* dirty all fbo this rbo attached to */
        while(imageUserList)
        {
            FRAMEBUFFER_COMPLETENESS_DIRTY((__GLframebufferObject *)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }
    }
#endif

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_TexImage2D(GLenum target,
                                GLint lod,
                                GLint internalFormat,
                                GLsizei width,
                                GLsizei height,
                                GLint border,
                                GLenum format,
                                GLenum type,
                                const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLboolean proxy;
    GLenum oldError;
    GLuint unpackBuffer;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexImage2D", DT_GLenum, target, DT_GLint, lod, DT_GLint, internalFormat, DT_GLsizei, width,
        DT_GLsizei, height, DT_GLint, border, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE2D_CHECK_TARGET_PROXY();

    if (proxy)/*if target is a proxy texture, no error code is generated*/
    {
        oldError = gc->error;
    }

    /* Check arguments */
    if (GL_FALSE == __glCheckTexImageArgs(gc, target, lod, internalFormat,
                                          width, height, 1+ border*2, border,
                                          format, type) )
    {
        if(proxy)
        {
            __GL_MEMZERO(&tex->faceMipmap[face][lod], sizeof(__GLmipMapLevel));
            tex->faceMipmap[face][lod].deviceFormat = &__glNullDevfmt;
            __glSetError(oldError);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* If this texture is bound to a colorbuffer. */
    if (tex->hPbuffer != NULL)
    {
        __glReleaseTexImageImplicit(gc,tex->hPbuffer,tex->colorBuffer,tex);
    }

    __glTextureCubeWorkAround(gc,tex, face);

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat,tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, chosenFormat, internalFormat,
                               format, type, width, height, 1, border, proxy) == GL_FALSE)
    {
        return;
    }

    if (proxy)
    {
        return;
    }

    /* The image is from unpack buffer object? */
    unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

    if (unpackBuffer > 0)
    {
        GLboolean bSupported;
        GLuint temp1,temp2;
        bSupported = (*gc->dp.queryPixelBufferFormat)(gc,format,type,&temp1,&temp2);

        /* If the format and type are not supported by HW directly, go SW path */
        if (!bSupported)
        {
            __GLbufferObject *bufObj;

            bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
            if (bufObj == NULL)
            {
                GL_ASSERT(0);
                return;
            }

            /* Add the offset */
            buf = bufObj->systemMemCache + ((GLbyte *)buf - (GLbyte *)0);
            unpackBuffer = 0;
        }
    }

    if (unpackBuffer != 0)
    {
        __GLmipMapLevel *mipmap = &tex->faceMipmap[face][lod];

        /*
        ** Just save the offset.
        ** Leave DP layer to judge how to copy the image.
        */
        tex->offsetInPBO = (GLvoid *)buf;
        tex->unpackBuffer = unpackBuffer;

        /* Save format and type */
        mipmap->format = format;
        mipmap->type   = type;
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.texImage2D)(gc, tex, face, lod, buf);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

#if GL_EXT_framebuffer_object
    if(tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        /* dirty all fbo this rbo attached to */
        while(imageUserList)
        {
            FRAMEBUFFER_COMPLETENESS_DIRTY((__GLframebufferObject *)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }
    }
#endif

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_TexImage1D(GLenum target,
                                GLint lod,
                                GLint internalFormat,
                                GLsizei width,
                                GLint border,
                                GLenum format,
                                GLenum type,
                                const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLboolean proxy;
    GLenum oldError;
    GLuint unpackBuffer;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexImage1D", DT_GLenum, target, DT_GLint, lod, DT_GLint, internalFormat,
        DT_GLsizei, width, DT_GLint, border, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE1D_CHECK_TARGET_PROXY();

    if(proxy)/*if target is a proxy texture, no error code is generated*/
    {
        oldError = gc->error;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalFormat, width, 1 + border*2, 1 + border*2, border, format, type) )
    {
        if(proxy)
        {
            __GL_MEMZERO(&tex->faceMipmap[0][lod], sizeof(__GLmipMapLevel));
            tex->faceMipmap[0][lod].deviceFormat = &__glNullDevfmt;
            __glSetError(oldError);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* If this texture is bound to a colorbuffer. */
    if (tex->hPbuffer != NULL)
    {
        __glReleaseTexImageImplicit(gc,tex->hPbuffer,tex->colorBuffer,tex);
    }

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat, tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if( GL_FALSE == __glSetMipmapLevelInfo(gc, tex, 0, lod, chosenFormat, internalFormat,
                                           format, type, width, 1, 1, border, proxy))
    {
          return;
    }

    if(proxy)
    {
        return;
    }

    /* The image is from unpack buffer object? */
    unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

    if (unpackBuffer > 0)
    {
        GLboolean bSupported;
        GLuint temp1,temp2;
        bSupported = (*gc->dp.queryPixelBufferFormat)(gc,format,type,&temp1,&temp2);

        /* If the format and type are not supported by HW directly, go SW path */
        if (!bSupported)
        {
            __GLbufferObject *bufObj;

            bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
            if (bufObj == NULL)
            {
                GL_ASSERT(0);
                return;
            }

            /* Add the offset */
            buf = bufObj->systemMemCache + ((GLbyte *)buf - (GLbyte *)0);
            unpackBuffer = 0;
        }
    }

    if (unpackBuffer != 0)
    {
        __GLmipMapLevel *mipmap = &tex->faceMipmap[0][lod];

        /*
        ** Just save the offset.
        ** Leave DP layer to judge how to copy the image.
        */
        tex->offsetInPBO = (GLvoid *)buf;
        tex->unpackBuffer = unpackBuffer;

        /* Save format and type */
        mipmap->format = format;
        mipmap->type   = type;
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.texImage1D)(gc, tex, lod, buf);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

#if GL_EXT_framebuffer_object
    if(tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        /* dirty all fbo this rbo attached to */
        while(imageUserList)
        {
            FRAMEBUFFER_COMPLETENESS_DIRTY((__GLframebufferObject *)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }
    }
#endif

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

/*****************************************************************************
*
*  glCopyTexImagexD
*
*****************************************************************************/

GLvoid APIENTRY __glim_CopyTexImage2D(GLenum target,
                                    GLint lod,
                                    GLenum internalFormat,
                                    GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLint border)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLenum format;
    GLenum type = GL_FLOAT;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyTexImage2D", DT_GLenum, target, DT_GLint, lod, DT_GLenum, internalFormat, DT_GLint, x, DT_GLint, y,
        DT_GLsizei, width, DT_GLsizei, height, DT_GLint, border, DT_GLnull);
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
            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(internalFormat)
                {
                /* extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return;
                }
            }
        }
    }

    /*get the texture object and face*/
    TEXIMAGE2D_COPY_CHECK_TARGET_NO_PROXY();

    /*get "format"*/
    switch (internalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        format = GL_DEPTH_COMPONENT;
        if(!gc->modes.haveDepthBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;

    case 1:
    case 2:
    case 3:
    case 4:
        __glSetError(GL_INVALID_ENUM);
        return;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
        if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        if (!gc->frameBuffer.readFramebufObj->fbInteger || (READ_FRAMEBUFFER_BINDING_NAME == 0))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        /*just set it to pass args checking*/
        format = GL_RGBA_INTEGER_EXT;
        type = GL_INT;
        break;

    default:
        format = GL_RGBA;
        break;
    }


    /* Check arguments: GL_FLOAT is just to pass the args check*/
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalFormat, width, height, 1 + border *2, border, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* If this texture is bound to a colorbuffer. */
    if (tex->hPbuffer != NULL)
    {
        __glReleaseTexImageImplicit(gc,tex->hPbuffer,tex->colorBuffer,tex);
    }

    __glTextureCubeWorkAround(gc,tex, face);
    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat, tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if(__glSetMipmapLevelInfo(gc, tex,face,lod,
          chosenFormat, internalFormat, format, type, width, height, 1, border, 0) == GL_FALSE)
    {
        return;
    }

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/
    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.copyTexImage2D)(gc, tex, face, lod, x, y);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_CopyTexImage1D(GLenum target,
                                    GLint lod,
                                    GLenum internalFormat,
                                    GLint x,
                                    GLint y,
                                    GLsizei width,
                                    GLint border)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLenum format,type = GL_FLOAT;


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyTexImage1D", DT_GLenum, target, DT_GLint, lod, DT_GLenum, internalFormat,DT_GLint, x, DT_GLint, y,
        DT_GLsizei, width, DT_GLint, border, DT_GLnull);
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

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(internalFormat)
                {
                /* extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return;
                }
            }
        }


    }

    /*get the texture object and face*/
    TEXIMAGE1D_CHECK_TARGET_NO_PROXY();

    /*get "format"*/
    switch (internalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        format = GL_DEPTH_COMPONENT;
        if(!gc->modes.haveDepthBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;

    case 1:
    case 2:
    case 3:
    case 4:
        __glSetError(GL_INVALID_ENUM);
        return;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
        if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        if (!gc->frameBuffer.readFramebufObj->fbInteger || (READ_FRAMEBUFFER_BINDING_NAME == 0))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        /*just set it to pass args checking*/
        format = GL_RGBA_INTEGER_EXT;
        type = GL_INT;
        break;
    default:
        format = GL_RGBA;
        break;
    }

    /* Check arguments: GL_FLOAT is just to pass the args check*/
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalFormat, width, 1 + border *2, 1 + border *2,
        border, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* If this texture is bound to a colorbuffer. */
    if (tex->hPbuffer != NULL)
    {
        __glReleaseTexImageImplicit(gc,tex->hPbuffer,tex->colorBuffer,tex);
    }

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat, tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if( GL_FALSE == __glSetMipmapLevelInfo(gc, tex, 0,lod,
        chosenFormat, internalFormat, format, type,
        width, 1, 1, border, GL_FALSE))
    {
        return;
    }

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/
    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.copyTexImage1D)(gc, tex, lod, x, y);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}


/*
*  All pixel storage and pixel transfer modes are ignored when decoding
*    a compressed texture image.
*/
GLvoid APIENTRY __glim_CompressedTexImage3D(GLenum target,
                                            GLint lod,
                                            GLenum internalFormat,
                                            GLsizei width,
                                            GLsizei height,
                                            GLsizei depth,
                                            GLint border,
                                            GLsizei imageSize,
                                            const GLvoid *data)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLboolean proxy;
    GLenum oldError = 0;
    __GLmipMapLevel *mipmap;



    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexImage3D", DT_GLenum, target, DT_GLint, lod, DT_GLenum, internalFormat,
        DT_GLsizei, width, DT_GLsizei, height, DT_GLsizei, depth, DT_GLint, border, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE3D_CHECK_TARGET_PROXY();

    if(proxy)/*if target is a proxy texture, no error code is generated*/
    {
        oldError = gc->error;
    }

    if(tex->targetIndex != __GL_TEXTURE_2D_ARRAY_INDEX)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(__glCheckCompressedTexImageFormat(internalFormat, border, proxy) == GL_FALSE)
    {
        if(proxy)
            goto proxy_return;
    }

    /* Check arguments, use GL_RGBA, GL_FLOAT just to pass the check*/
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalFormat, width, height, depth, border, GL_RGBA, GL_FLOAT) )
    {
        if(proxy)
        {
proxy_return:
            __GL_MEMZERO(&tex->faceMipmap[0][lod], sizeof(__GLmipMapLevel));
            tex->faceMipmap[0][lod].deviceFormat = &__glNullDevfmt;
            __glSetError(oldError);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalFormat,tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    if(__glSetMipmapLevelInfo(gc, tex, 0, lod,
          chosenFormat, internalFormat, 0, 0, width, height, depth, border, proxy) == GL_FALSE)
    {
        return;
    }

    mipmap = &tex->faceMipmap[0][lod];

    /*we may set mipmap->compressed to false because choose a uncompressed device format*/
    if (mipmap->compressed && imageSize != (mipmap->compressedSize * tex->arrays))
    {
        if(proxy) goto proxy_return;
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (proxy) return;

    if (data && !mipmap->compressed)
    {
        /*we chose an un-compressed format, need de-compressed*/
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.compressedTexImage3D)(gc, tex, lod, data);

    if (tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

#if GL_EXT_framebuffer_object
    if(tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        /* dirty all fbo this rbo attached to */
        while(imageUserList)
        {
            FRAMEBUFFER_COMPLETENESS_DIRTY((__GLframebufferObject *)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }
    }
#endif

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_CompressedTexImage2D(GLenum target,
                                            GLint lod,
                                            GLenum internalformat,
                                            GLsizei width,
                                            GLsizei height,
                                            GLint border,
                                            GLsizei imageSize,
                                            const GLvoid *data)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    GLboolean proxy;
    GLenum oldError = 0;
    __GLmipMapLevel *mipmap;
    GLuint unpackBuffer;



    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexImage2D", DT_GLenum, target, DT_GLint, lod, DT_GLenum, internalformat,
        DT_GLsizei, width, DT_GLsizei, height, DT_GLint, border, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE2D_CHECK_TARGET_PROXY();

    if(proxy)/*if target is a proxy texture, no error code is generated*/
    {
        oldError = gc->error;
    }

    if(__glCheckCompressedTexImageFormat(internalformat, border, proxy) == GL_FALSE)
    {
        if(proxy)
            goto proxy_return;
    }

    /* Check arguments, use GL_RGBA, GL_FLOAT just to pass the check*/
    if(GL_FALSE == __glCheckTexImageArgs(gc, target,lod,
        internalformat, width, height, 1 + border*2, border, GL_RGBA, GL_FLOAT) )
    {
        if(proxy)
        {
proxy_return:
            __GL_MEMZERO(&tex->faceMipmap[face][lod], sizeof(__GLmipMapLevel));
            tex->faceMipmap[face][lod].deviceFormat = &__glNullDevfmt;
            __glSetError(oldError);
        }
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glTextureCubeWorkAround(gc,tex,face);

    /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalformat,tex->params.generateMipmap,0);

    /*init the mipmap info which will be queried by app*/
    mipmap = &tex->faceMipmap[face][lod];

    /*init the mipmap info which will be queried by app*/
    if(__glSetMipmapLevelInfo(gc, tex,face,lod,
          chosenFormat, internalformat, 0, 0, width, height, 1, border, 0) == GL_FALSE)
    {
        return;
    }

    /*we may set mipmap->compressed to false because choose a uncompressed device format*/
    if(mipmap->compressed && imageSize != (mipmap->compressedSize * mipmap->arrays))
    {
        if(proxy) goto proxy_return;
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(proxy) return;

    /* The image is from unpack buffer object? */
    unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

    if (unpackBuffer > 0)
    {
        __GLbufferObject *bufObj;

        bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
        if (bufObj == NULL)
        {
            GL_ASSERT(0);
            return;
        }

        /* Add the offset */
        data = bufObj->systemMemCache + ((GLbyte *)data - (GLbyte *)0);
    }

    if(data && !mipmap->compressed)
    {
        /*we chose an un-compressed format, need de-compressed*/
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.compressedTexImage2D)(gc, tex, face, lod, data);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

#if GL_EXT_framebuffer_object
    if(tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        /* dirty all fbo this rbo attached to */
        while(imageUserList)
        {
            FRAMEBUFFER_COMPLETENESS_DIRTY((__GLframebufferObject *)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }
    }
#endif

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;
}

GLvoid APIENTRY __glim_CompressedTexImage1D(GLenum target,
                                            GLint lod,
                                            GLenum internalFormat,
                                            GLsizei width,
                                            GLint border,
                                            GLsizei imageSize,
                                            const GLvoid *data)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    GLboolean proxy;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexImage1D", DT_GLenum, target, DT_GLint, lod, DT_GLenum, internalFormat, DT_GLsizei, width,
        DT_GLint, border, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE1D_CHECK_TARGET_PROXY();

    if(proxy)
    {
        __GL_MEMZERO(&tex->faceMipmap[0][lod], sizeof(__GLmipMapLevel));
                tex->faceMipmap[0][lod].deviceFormat = &__glNullDevfmt;
    }
    else
    {
        __glSetError(GL_INVALID_ENUM);
    }
}


/*****************************************************************************
*
*  glTexSubImagexD
*
*****************************************************************************/
GLvoid APIENTRY __glim_TexSubImage3D(GLenum target,
                                   GLint lod,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLint zoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLsizei depth,
                                   GLenum format,
                                   GLenum type,
                                   const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexSubImage3D", DT_GLenum, target, DT_GLint, lod,DT_GLint, xoffset,DT_GLint, yoffset,DT_GLint, zoffset,
        DT_GLsizei, width,DT_GLsizei, height,DT_GLsizei, depth, DT_GLenum, format,DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE3D_SUB_CHECK_TARGET_NO_PROXY();

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, 0, lod,
        xoffset, yoffset, zoffset, width, height, depth, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->dp.syncTextureFromDeviceMemory(gc,tex,lod);

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/
    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.texSubImage3D)(gc, tex, lod, xoffset, yoffset, zoffset, width, height, depth, buf);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

GLvoid APIENTRY __glim_TexSubImage2D(GLenum target,
                                   GLint lod,
                                   GLint xoffset,
                                   GLint yoffset,
                                   GLsizei width,
                                   GLsizei height,
                                   GLenum format,
                                   GLenum type,
                                   const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;
    GLuint unpackBuffer;
    GLenum oldformat;
    GLenum oldtype;
    __GLmipMapLevel *mipmap;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexSubImage2D", DT_GLenum, target, DT_GLint, lod,DT_GLint, xoffset,DT_GLint, yoffset,
        DT_GLsizei, width,DT_GLsizei, height, DT_GLenum, format,DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE2D_SUB_CHECK_TARGET_NO_PROXY();

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, face, lod,
        xoffset, yoffset, 0, width, height, 1, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->dp.syncTextureFromDeviceMemory(gc,tex,lod);

    /* The image is from unpack buffer object? */
    unpackBuffer = gc->bufferObject.boundBuffer[__GL_PIXEL_UNPACK_BUFFER_INDEX];

    if (unpackBuffer > 0)
    {
        GLboolean bSupported;
        GLuint temp1,temp2;
        bSupported = (*gc->dp.queryPixelBufferFormat)(gc,format,type,&temp1,&temp2);

        /* If the format and type are not supported by HW directly, go SW path */
        if (!bSupported)
        {
            __GLbufferObject *bufObj;

            bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, unpackBuffer);
            if (bufObj == NULL)
            {
                GL_ASSERT(0);
                return;
            }

            /* Add the offset */
            buf = bufObj->systemMemCache + ((GLbyte *)buf - (GLbyte *)0);
            unpackBuffer = 0;
        }
    }

    mipmap = &tex->faceMipmap[face][lod];
    if (unpackBuffer != 0)
    {
        /*
        ** Just save the offset.
        ** Leave DP layer to judge how to copy the image.
        */
        tex->offsetInPBO = (GLvoid *)buf;
        tex->unpackBuffer = unpackBuffer;
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Save format and type in order to restore them*/
    if ( unpackBuffer==0 )
    {
        oldformat = mipmap->format;
        oldtype = mipmap->type;
    }

    /* TexImage2D sets the internal format and the format of the data deliverd by TexSubimage2D
    ** perhaps is different from internal format set by TexImage2D, so it is a must that low level
    ** should know format change to convert incoming data to internal format if possible.
    */

    mipmap->format = format;
    mipmap->type   = type;

    (*gc->dp.texSubImage2D)(gc, tex, face, lod, xoffset, yoffset, width, height, buf);

    /* Restore internal format to the original one */
    if ( unpackBuffer==0 )
    {
        mipmap->format = oldformat;
        mipmap->type   = oldtype;
    }

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

GLvoid APIENTRY __glim_TexSubImage1D(GLenum target,
                                   GLint lod,
                                   GLint xoffset,
                                   GLsizei width,
                                   GLenum format,
                                   GLenum type,
                                   const GLvoid *buf)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexSubImage1D", DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset, DT_GLsizei, width,
        DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, buf, DT_GLnull);
#endif

    activeUnit = gc->state.texture.activeTexIndex;

    /*TODO:add proxy texture*/
    /*get the texture object and face*/
    TEXIMAGE1D_SUB_CHECK_TARGET_NO_PROXY();

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, 0, lod,
        xoffset, 0, 0, width, 1, 1, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->dp.syncTextureFromDeviceMemory(gc,tex,lod);

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.texSubImage1D)(gc, tex, lod, xoffset, width, buf);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

/*****************************************************************************
*
*  glCopyTexSubImagexD
*
*****************************************************************************/
GLvoid APIENTRY __glim_CopyTexSubImage3D(GLenum target,
                                       GLint lod,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLint zoffset,
                                       GLint x,
                                       GLint y,
                                       GLsizei width,
                                       GLsizei height)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLmipMapLevel *mipmap;
    GLenum format, type = GL_FLOAT;
    GLint max_lod;


    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyTexSubImage3D", DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset,DT_GLint, yoffset,DT_GLint, zoffset,
        DT_GLint, x, DT_GLint, y, DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif


    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (lod < 0 || lod > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    /*get the texture object and face*/
    TEXIMAGE3D_COPY_CHECK_TARGET_NO_PROXY_EXT();

    mipmap = &tex->faceMipmap[0][lod];

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

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(mipmap->requestedFormat)
                {
                /* extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return;
                }
            }
        }
    }

    switch (mipmap->requestedFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        format = GL_DEPTH_COMPONENT;
        if(!gc->modes.haveDepthBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
        if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        if (!gc->frameBuffer.readFramebufObj->fbInteger || (READ_FRAMEBUFFER_BINDING_NAME == 0))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        /*just set it to pass args checking*/
        format = GL_RGBA_INTEGER_EXT;
        type = GL_INT;



        break;
    default:
        format = GL_RGBA;
        break;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, 0, lod,
        xoffset, yoffset, 0, width, height, 1, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.copyTexSubImage3D)(gc, tex, lod, x, y, width, height, xoffset, yoffset, zoffset);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}


GLvoid APIENTRY __glim_CopyTexSubImage2D(GLenum target,
                                       GLint lod,
                                       GLint xoffset,
                                       GLint yoffset,
                                       GLint x,
                                       GLint y,
                                       GLsizei width,
                                       GLsizei height)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;
    __GLmipMapLevel *mipmap;
    GLenum format, type = GL_FLOAT;
    GLint max_lod;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyTexSubImage2D", DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset, DT_GLint, yoffset, DT_GLint, x, DT_GLint, y,
        DT_GLsizei, width, DT_GLsizei, height, DT_GLnull);
#endif

    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (lod < 0 || lod > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    /*get the texture object and face*/
    TEXIMAGE2D_SUB_CHECK_TARGET_NO_PROXY();

    mipmap = &tex->faceMipmap[face][lod];

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

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(mipmap->requestedFormat)
                {
                /* extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return;
                }
            }
        }


    }

    switch (mipmap->requestedFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        format = GL_DEPTH_COMPONENT;
        if(!gc->modes.haveDepthBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
        if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        if (!gc->frameBuffer.readFramebufObj->fbInteger || (READ_FRAMEBUFFER_BINDING_NAME == 0))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        /*just set it to pass args checking*/
        format = GL_RGBA_INTEGER_EXT;
        type = GL_INT;
        break;
    default:
        format = GL_RGBA;
        break;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, face, lod,
        xoffset, yoffset, 0, width, height, 1, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.copyTexSubImage2D)(gc, tex, face, lod, x, y, width, height, xoffset, yoffset);

    if (tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

GLvoid APIENTRY __glim_CopyTexSubImage1D(GLenum target,
                                       GLint lod,
                                       GLint xoffset,
                                       GLint x,
                                       GLint y,
                                       GLsizei width)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    __GLmipMapLevel *mipmap;
    GLenum format,type = GL_FLOAT;
    GLint max_lod;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CopyTexSubImage1D" ,DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset, DT_GLint, x, DT_GLint, y,
        DT_GLsizei, width, DT_GLnull);
#endif


    max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);

    /* check lod */
    /*
    ** because switch (mipmap->requestedFormat) use the level and change the format
    ** so we do check here outside the __glCheckTexSubImageArgs
    */
    if (lod < 0 || lod > max_lod )
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    /*get the texture object and face*/
    TEXIMAGE1D_SUB_CHECK_TARGET_NO_PROXY();

    mipmap = &tex->faceMipmap[0][lod];

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

            if (gc->frameBuffer.readFramebufObj->fbInteger)
            {
                switch(mipmap->requestedFormat)
                {
                /* extension GL_EXT_texture_integer */
                case GL_RGBA32UI_EXT:
                case GL_RGBA32I_EXT:
                case GL_RGBA16UI_EXT:
                case GL_RGBA16I_EXT:
                case GL_RGBA8UI_EXT:
                case GL_RGBA8I_EXT:
                case GL_RGB32UI_EXT:
                case GL_RGB32I_EXT:
                case GL_RGB16UI_EXT:
                case GL_RGB16I_EXT:
                case GL_RGB8UI_EXT:
                case GL_RGB8I_EXT:
                case GL_LUMINANCE32UI_EXT:
                case GL_LUMINANCE32I_EXT:
                case GL_LUMINANCE16UI_EXT:
                case GL_LUMINANCE16I_EXT:
                case GL_LUMINANCE8UI_EXT:
                case GL_LUMINANCE8I_EXT:
                case GL_LUMINANCE_ALPHA32UI_EXT:
                case GL_LUMINANCE_ALPHA32I_EXT:
                case GL_LUMINANCE_ALPHA16UI_EXT:
                case GL_LUMINANCE_ALPHA16I_EXT:
                case GL_LUMINANCE_ALPHA8UI_EXT:
                case GL_LUMINANCE_ALPHA8I_EXT:
                case GL_INTENSITY32UI_EXT:
                case GL_INTENSITY32I_EXT:
                case GL_INTENSITY16UI_EXT:
                case GL_INTENSITY16I_EXT:
                case GL_INTENSITY8UI_EXT:
                case GL_INTENSITY8I_EXT:
                    break;
                default:
                    __glSetError(GL_INVALID_OPERATION);
                    return;
                }
            }
        }
    }
    switch (mipmap->requestedFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
        format = GL_DEPTH_COMPONENT;
        if(!gc->modes.haveDepthBuffer)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
    /* extension GL_EXT_texture_integer */
    case GL_RGBA32UI_EXT:
    case GL_RGBA32I_EXT:
    case GL_RGBA16UI_EXT:
    case GL_RGBA16I_EXT:
    case GL_RGBA8UI_EXT:
    case GL_RGBA8I_EXT:
    case GL_RGB32UI_EXT:
    case GL_RGB32I_EXT:
    case GL_RGB16UI_EXT:
    case GL_RGB16I_EXT:
    case GL_RGB8UI_EXT:
    case GL_RGB8I_EXT:
    case GL_LUMINANCE32UI_EXT:
    case GL_LUMINANCE32I_EXT:
    case GL_LUMINANCE16UI_EXT:
    case GL_LUMINANCE16I_EXT:
    case GL_LUMINANCE8UI_EXT:
    case GL_LUMINANCE8I_EXT:
    case GL_LUMINANCE_ALPHA32UI_EXT:
    case GL_LUMINANCE_ALPHA32I_EXT:
    case GL_LUMINANCE_ALPHA16UI_EXT:
    case GL_LUMINANCE_ALPHA16I_EXT:
    case GL_LUMINANCE_ALPHA8UI_EXT:
    case GL_LUMINANCE_ALPHA8I_EXT:
    case GL_INTENSITY32UI_EXT:
    case GL_INTENSITY32I_EXT:
    case GL_INTENSITY16UI_EXT:
    case GL_INTENSITY16I_EXT:
    case GL_INTENSITY8UI_EXT:
    case GL_INTENSITY8I_EXT:
        if (!__glExtension[INDEX_EXT_texture_integer].bEnabled)
        {
            __glSetError(GL_INVALID_ENUM);
            return;
        }
        if (!gc->frameBuffer.readFramebufObj->fbInteger || (READ_FRAMEBUFFER_BINDING_NAME == 0))
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        /*just set it to pass args checking*/
        format = GL_RGBA_INTEGER_EXT;
        type = GL_INT;
        break;
    default:
        format = GL_RGBA;
        break;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, 0, lod,
        xoffset, 0, 0, width, 1, 1, format, type) )
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* The dp function may want to flush the DMA buffer, so LOCK is needed*/

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.copyTexSubImage1D)(gc, tex, lod, x, y, width, xoffset);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

GLvoid APIENTRY __glim_CompressedTexSubImage3D(GLenum target,
                                               GLint lod,
                                               GLint xoffset,
                                               GLint yoffset,
                                               GLint zoffset,
                                               GLsizei width,
                                               GLsizei height,
                                               GLsizei depth,
                                               GLenum format,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    GLint blockSize;

    __GLmipMapLevel *mipmap;



    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexSubImage2D" ,DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset, DT_GLint, yoffset, DT_GLsizei, width,
        DT_GLsizei, height, DT_GLenum, format, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE3D_SUB_CHECK_TARGET_NO_PROXY();

    if(tex->targetIndex != __GL_TEXTURE_2D_ARRAY_INDEX)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if(__glCheckCompressedTexImageFormat(format, 0, 0) == GL_FALSE)
    {
        return;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, 0, lod,
        xoffset, yoffset, zoffset, width, height, depth, GL_RGBA, GL_FLOAT) )
    {
        return;
    }

    mipmap = &tex->faceMipmap[zoffset][lod];
    if (imageSize != (__glCompressedTexImageSize(format, width, height, &blockSize)))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->dp.syncTextureFromDeviceMemory(gc, tex, lod);

    if (data)
    {
        if (!mipmap->compressed)
        {
            /*we chose an un-compressed format, need de-compressed*/
        }
        /* copy the data directly into the retrieved buffer from device */
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.compressedTexSubImage3D)(gc, tex, 0, lod, xoffset, yoffset, zoffset, width, height, depth, data);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, 0, lod);
    }
    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}

GLvoid APIENTRY __glim_CompressedTexSubImage2D(GLenum target,
                                               GLint lod,
                                               GLint xoffset,
                                               GLint yoffset,
                                               GLsizei width,
                                               GLsizei height,
                                               GLenum format,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;

    GLint blockSize;

    __GLmipMapLevel *mipmap;


    GLubyte * imageSrc = (GLubyte *)data;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexSubImage2D" ,DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset, DT_GLint, yoffset, DT_GLsizei, width,
        DT_GLsizei, height, DT_GLenum, format, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif

    /*get the texture object and face*/
    TEXIMAGE2D_SUB_CHECK_TARGET_NO_PROXY();

    /* Check format */
    if(__glCheckCompressedTexImageFormat(format, 0, 0) == GL_FALSE)
    {
        return;
    }

    /* Check arguments */
    if(GL_FALSE == __glCheckTexSubImageArgs(gc, tex, face, lod,
        xoffset, yoffset, 0, width, height, 1, GL_RGBA, GL_FLOAT) )
    {
        return;
    }

    mipmap = &tex->faceMipmap[face][lod];
    if (imageSize != __glCompressedTexImageSize(format, width, height, &blockSize))
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    gc->dp.syncTextureFromDeviceMemory(gc,tex,lod);

    if (imageSrc)
    {
        if (!mipmap->compressed)
        {
            /*we chose an un-compressed format, need de-compressed*/
        }
        /* copy the data directly into the retrieved buffer from device */
    }

    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.compressedTexSubImage2D)(gc, tex, face, lod, xoffset, yoffset, width, height, data);

    if(tex->params.generateMipmap && lod == tex->params.baseLevel)
    {
        __glGenerateMipmaps(gc, tex, face, lod);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
}


GLvoid APIENTRY __glim_CompressedTexSubImage1D(GLenum target,
                                               GLint lod,
                                               GLint xoffset,
                                               GLsizei width,
                                               GLenum format,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_CompressedTexSubImage1D", DT_GLenum, target, DT_GLint, lod, DT_GLint, xoffset,
        DT_GLsizei, width,  DT_GLenum, format, DT_GLsizei, imageSize, DT_GLvoid_ptr, data, DT_GLnull);
#endif
}


GLvoid APIENTRY __glim_GetTexImage( GLenum target, GLint lod, GLenum format,GLenum type, GLvoid *img)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    GLuint face;
    __GLmipMapLevel *mipmap;
    GLint level = lod;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetTexImage", DT_GLenum, target, DT_GLint, lod, DT_GLenum, format, DT_GLenum, type, DT_GLvoid_ptr, img, DT_GLnull);
#endif

    /* Get the texture object and face. */
    GETTEXIMAGE_CHECK_TARGET_LOD_FACE_NO_PROXY();

    if(__glCheckGetTexImageArgs(format, type) == GL_FALSE)
        return;

    mipmap = &tex->faceMipmap[face][level];

    if(mipmap->width == 0) return;

    gc->dp.syncTextureFromDeviceMemory(gc,tex,lod);

    if (img) {
        __GLpixelSpanInfo *spanInfo = gc->pixel.spanInfo;
        spanInfo->transDirection = __GL_DRIVER_TO_APP;

        /* TODO Copy text image to user buffer */
    }

    return;
}

GLvoid APIENTRY __glim_GetCompressedTexImage(GLenum target, GLint level, GLvoid *img)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    GLuint face;
    __GLmipMapLevel *mipmap;
    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_GetCompressedTexImage", DT_GLenum, target, DT_GLint, level, DT_GLvoid_ptr, img, DT_GLnull);
#endif

    /* Get the texture object and face. */

    GETTEXIMAGE_CHECK_TARGET_LOD_FACE_NO_PROXY();

    mipmap = &tex->faceMipmap[face][level];

    /* Invalid to get a non-compressed image. */
    if (!mipmap->compressed)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    if(mipmap->width == 0) return;

    gc->dp.syncTextureFromDeviceMemory(gc,tex,level);

    if (img)
    {
        /* TODO Copy text image to user buffer */
    }
}

GLvoid APIENTRY __glim_TexBufferEXT(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GLbufferObject *bufObj = NULL;
    __GLbufferObject * attachedBufObj = NULL;
    GLuint activeUnit;
    __GLdeviceFormat chosenFormat;
    __GLtextureObject *tex;
    GLint components, baseSize;
    GLsizei width;

    __GL_SETUP_NOT_IN_BEGIN();

#if (defined(_DEBUG) || defined(DEBUG))
    if(dbg_logAPIFilter)
        dbgLogFullApi("__glim_TexBufferEXT", DT_GLenum, target, DT_GLenum, internalformat,  DT_GLuint, buffer, DT_GLnull);
#endif

    if(!__glCheckTexBufferArgs(target, internalformat, &components, &baseSize))
    {
        return;
    }

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get active texture unit */
    activeUnit = gc->state.texture.activeTexIndex;

    tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_BUFFER_INDEX];

    attachedBufObj = tex->bufferObj;

     /*query the hw-supported texture format*/
    chosenFormat = (*__glDevice->devQueryDeviceFormat)(internalformat, 0, 0);


    if(buffer)
    {
        /* Retrieve the buffer object from the "gc->bufferObject.shared" structure.*/
        bufObj = (__GLbufferObject *)__glGetObject(gc, gc->bufferObject.shared, buffer);

        if(!bufObj)
        {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }

        width = (GLsizei)__GL_FLOORD(((GLdouble)bufObj->size / (components * baseSize)));

        if (tex->bufferObj == bufObj)
        {
            return;
        }

        if(!__glSetMipmapLevelInfo(gc, tex, 0, 0, chosenFormat, internalformat, 0, 0, width, 1, 1, 0,0))
        {
            return;
        }

        /* unbind last attached point */
        if(attachedBufObj)
        {
            __glRemoveImageUser(gc, &(attachedBufObj->bufferObjData->bufferObjUserList), tex);
            tex->bufferObj = NULL;
        }

        if(!bufObj->bufferObjData)
        {
            bufObj->bufferObjData = (*gc->imports.malloc)(gc, sizeof(__GLbufferObjData) );
            if (!bufObj->bufferObjData)
            {
                __glSetError(GL_OUT_OF_MEMORY);
                return;
            }
        }

        /* bind texture buffer object to buffer object */
        __glAddImageUser(gc, &(bufObj->bufferObjData->bufferObjUserList), tex, NULL);

        tex->bufferObj = bufObj;

        (*gc->dp.texBufferEXT)(gc, tex, GL_TRUE);
    }
    else
    {
        /* unbind last attached point */
        if(attachedBufObj)
        {
            __glRemoveImageUser(gc, &(attachedBufObj->bufferObjData->bufferObjUserList), tex);
            (*gc->dp.texBufferEXT)(gc, tex, GL_FALSE);
            tex->bufferObj = NULL;
        }
    }
}


GLboolean __glReleaseTexImage(__GLcontext *gc,__GLtextureObject *tex)
{
    __GLmipMapLevel *mipmap;
    GLint lod,faceTotal,face;
    GLuint i;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Call dp to release the texture */
    gc->dp.releaseTexImageARB(gc,tex);

    /*
    ** Spec:
    ** After a color buffer is released from a texture,
    ** all texture images that were defined
    ** by the color buffer become NULL (it is as if TexImage was
    ** called with an image of zero width).
    */
    /* Clear all  mipmap levels for this texture object. */
    faceTotal = tex->arrays;

    for (face=0;face<faceTotal;face++)
    {
        lod = 0;
        for(lod=0;lod<(GLint)tex->pBufferNumLevels;lod++)
        {
            mipmap = &tex->faceMipmap[face][lod];

            __glSetMipmapLevelNULL(gc,mipmap);
        }
    }

    /* Clear the pointers. */
    tex->colorBuffer = __GL_NULL_COLORBUFFER;
    tex->hPbuffer = NULL;

    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++) {
        if (tex->name == gc->texture.units[i].boundTextures[tex->targetIndex]->name) {
            __GL_SET_TEX_UNIT_BIT(gc, i, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
        }
    }

    return GL_TRUE;
}

/*
** Bind a colorbuffer in the drawable to current texture object.
*/
GLboolean __glBindTexImageARB(__GLcontext *gc, __GLdrawablePrivate *srcDrawable, GLvoid *hPBuffer, GLenum iBuffer)
{
    __GLtextureObject *tex;
    GLenum target,internalFormat;
    GLboolean mipmap;
    GLuint activeUnit;

    __GLdeviceFormat chosenFormat;
    __GLpBufferTexture *pbufferTex;
    GLsizei width,height;
    GLint lod,faceTotal,face;

    __GLdrawableBuffer *colorBuffer = NULL;

    /* Get the target */
    pbufferTex = srcDrawable->pbufferTex;
    target = pbufferTex->target;
    mipmap = pbufferTex->mipmap;

    /* Get the current texture object */
    __GL_VERTEX_BUFFER_FLUSH(gc);
    activeUnit = gc->state.texture.activeTexIndex;

    switch(target)
    {
    case GL_TEXTURE_2D:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        faceTotal = 1;
        break;
    case GL_TEXTURE_1D:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];
        faceTotal = 1;
        break;
    case GL_TEXTURE_CUBE_MAP:
        faceTotal = 6;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        break;
    default:
        GL_ASSERT(0);
        return GL_FALSE;
    }

    internalFormat = pbufferTex->internalFormat;
    chosenFormat = pbufferTex->chosenFormat;

    /* Get the colorbuffer based on ibuffer */
    colorBuffer = &srcDrawable->drawBuffers[iBuffer];
    if (colorBuffer == NULL)
    {
        return GL_FALSE;
    }

    /* Is this texture is bound to the color buffer already? */
    if (colorBuffer->boundTex.gc != NULL )
    {
        if (colorBuffer->boundTex.name == tex->name)
        {
            /* Bound again. */
            /* For Knight of old Republic */
            return GL_TRUE;
        }
        else
        {
            /* Release texture */
            colorBuffer->boundTex.gc = NULL;
            colorBuffer->boundTex.name = 0;
            __glReleaseTexImage(gc,tex);
        }
    }

    /*
    ** Get width and height from drawable.
    ** The drawable shouldn't be changed even when non-zero level mipmap of texture is rendered.
    */
    width = colorBuffer->width;
    height = colorBuffer->height;

    /* Get the count of levels. */
    if (mipmap)
    {
        tex->pBufferNumLevels = __glComputeNumLevels(width,height,1,0);
    }
    else
    {
        tex->pBufferNumLevels = 1;
    }

    tex->arrays = faceTotal;
    /* Set mipmap level for this texture object. */
    for (face = 0;face < faceTotal; face++)
    {
        width = colorBuffer->width;
        height = colorBuffer->height;

        for(lod = 0;lod < (GLint)tex->pBufferNumLevels; lod++)
        {
            if( GL_FALSE == __glSetMipmapLevelInfo(gc, tex, face, lod,
                chosenFormat, internalFormat, 0, 0, width, height, 1 , 0, 0))
            {
                return GL_FALSE;
            }
            width >>= 1;
            height >>= 1;
        }
    }

    /* Go to dp bindTexImage */
    LINUX_LOCK_FRAMEBUFFER(gc);

    (*gc->dp.bindTexImageARB)(gc,srcDrawable,colorBuffer,tex);

    /* Save the colorbuffer pointer. */
    tex->colorBuffer = iBuffer;
    tex->hPbuffer = hPBuffer;

    /* Record the context and texture name for release. */
    colorBuffer->boundTex.gc = (__GLcontextInterface*)gc;
    colorBuffer->boundTex.name = tex->name;

    /* Mipmap generation is needed too. */
    //Check Pbuffer Is dirty or not!
    if(tex->params.generateMipmap && pbufferTex->needGenMipmap == GL_TRUE)
    {
        //Compute correct face index
        if(faceTotal == 1)
            face = 0;
        else
            face = pbufferTex->face - GL_TEXTURE_CUBE_MAP_POSITIVE_X;

        __glGenerateMipmaps(gc, tex, face, 0);

        pbufferTex->needGenMipmap = GL_FALSE;
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);
    /* Enable texture consistency check */
    ENABLE_TEXTURE_CONSISTENCY_CHECK(tex);

    /*set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit*/
    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

    return GL_TRUE;
}


GLboolean __glReleaseTexImageARB(__GLcontext *gc,__GLdrawablePrivate *srcDrawable,GLenum iBuffer)
{
    __GLdrawableBuffer *colorBuffer = NULL;
    __GLtextureObject *tex;
    GLuint name, target;

    /* Get the colorbuffer based on ibuffer */
    colorBuffer = &srcDrawable->drawBuffers[iBuffer];
    if (colorBuffer == NULL)
    {
        return GL_FALSE;
    }

    if (colorBuffer->boundTex.gc == NULL)
    {
        /* Not bound */
        return GL_FALSE;
    }

    /* Get the texture object. */
    name = colorBuffer->boundTex.name;
    if(name)
        tex = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, name);
    else
    {
        switch(srcDrawable->pbufferTex->target)
        {
        case GL_TEXTURE_2D:
            target = __GL_TEXTURE_2D_INDEX;
            break;
        case GL_TEXTURE_1D:
            target = __GL_TEXTURE_1D_INDEX;
            break;
        case GL_TEXTURE_CUBE_MAP:
            target = __GL_TEXTURE_CUBEMAP_INDEX;
            break;
        default:
            GL_ASSERT(0);
            return GL_FALSE;
        }
        tex = &gc->texture.defaultTextures[target];
    }

    if (tex == NULL)
    {
        /* Texture is deleted. */
        colorBuffer->boundTex.gc = NULL;
        return GL_TRUE;
    }

    colorBuffer->boundTex.gc = NULL;
    colorBuffer->boundTex.name = 0;

    __glReleaseTexImage(gc,tex);

    return GL_TRUE;
}

GLvoid __glReleaseTexImageImplicit(__GLcontext *gc,GLvoid *hPbuffer,GLenum iBuffer,__GLtextureObject *tex)
{
    __GLdrawableBuffer *cb;

    /* First get the colorbuffer from wgl. */
    cb = gc->imports.getColorbufferPBuffer(hPbuffer,iBuffer);
    if (cb != NULL)
    {
        cb->boundTex.gc = NULL;
        cb->boundTex.name = 0;
    }

    __glReleaseTexImage(gc,tex);
}


