/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"
#include "gc_es_object_inline.c"
#include "gc_es_device.h"


#define _GC_OBJ_ZONE __GLES3_ZONE_CORE

extern GLboolean __glIsTextureComplete(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter,
                                       GLenum magFilter, GLenum compareMode, GLint maxLevelUsed);
extern GLint __glCalcTexMaxLevelUsed(__GLcontext *gc, __GLtextureObject *texObj, GLenum minFilter);

extern __GLformatInfo __glFormatInfoTable[];
extern __GLformatInfo* __glGetFormatInfo(GLenum internalFormat);
extern GLvoid __eglImageTargetRenderbufferStorageOES(__GLcontext* gc, GLenum target, GLvoid *eglImage);
extern GLenum __createEglImageRenderbuffer(__GLcontext* gc, GLenum renderbuffer, GLvoid* eglImage);
extern GLboolean __glDeleteTextureObject(__GLcontext *gc, __GLtextureObject *tex);

typedef struct __GLformatCombineRec
{
    GLenum internalformat;
    GLenum format;
    GLenum type;
    int size;
    GLboolean bRenderable;
} __GLformatCombine;

/* Valid combinations of format, type and (un)sized internal format */
static const __GLformatCombine canonicalformats[] = {
    /* Table 3.2, sized internal formats */
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_RGBA4, GL_RGBA, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, 4, GL_FALSE },
    { GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2, GL_TRUE },
    { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2, GL_TRUE },
    { GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 4, GL_TRUE },
    { GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 4, GL_TRUE },
    { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 8, GL_FALSE },
    { GL_RGBA32F, GL_RGBA, GL_FLOAT, 16, GL_FALSE },
    { GL_RGBA16F, GL_RGBA, GL_FLOAT, 16, GL_FALSE },
    { GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, 4, GL_TRUE },
    { GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 8, GL_TRUE },
    { GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, 8, GL_TRUE },
    { GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 16, GL_TRUE },
    { GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 16, GL_TRUE },
    { GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, 4, GL_TRUE },
    { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3, GL_TRUE },
    { GL_RGB565, GL_RGB, GL_UNSIGNED_BYTE, 3, GL_TRUE },
    { GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE, 3, GL_FALSE },
    { GL_RGB8_SNORM, GL_RGB, GL_BYTE, 3, GL_FALSE },
    { GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, GL_TRUE },
    { GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, 4, GL_FALSE },
    { GL_R11F_G11F_B10F, GL_RGB, GL_HALF_FLOAT, 6, GL_FALSE },
    { GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT, 12, GL_FALSE },
    { GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, 4, GL_FALSE },
    { GL_RGB9_E5, GL_RGB, GL_HALF_FLOAT, 6, GL_FALSE },
    { GL_RGB9_E5, GL_RGB, GL_FLOAT, 12, GL_FALSE },
    { GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 6, GL_FALSE },
    { GL_RGB32F, GL_RGB, GL_FLOAT, 12, GL_FALSE },
    { GL_RGB16F, GL_RGB, GL_FLOAT, 12, GL_FALSE },
    { GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, 3, GL_FALSE },
    { GL_RGB8I, GL_RGB_INTEGER, GL_BYTE, 3, GL_FALSE },
    { GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, 6, GL_FALSE },
    { GL_RGB16I, GL_RGB_INTEGER, GL_SHORT, 6, GL_FALSE },
    { GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 12, GL_FALSE },
    { GL_RGB32I, GL_RGB_INTEGER, GL_INT, 12, GL_FALSE },
    { GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 2, GL_TRUE },
    { GL_RG8_SNORM, GL_RG, GL_BYTE, 2, GL_FALSE },
    { GL_RG16F, GL_RG, GL_HALF_FLOAT, 4, GL_FALSE },
    { GL_RG32F, GL_RG, GL_FLOAT, 8, GL_FALSE },
    { GL_RG16F, GL_RG, GL_FLOAT, 8, GL_FALSE },
    { GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, 2, GL_TRUE },
    { GL_RG8I, GL_RG_INTEGER, GL_BYTE, 2, GL_TRUE },
    { GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, 4, GL_TRUE },
    { GL_RG16I, GL_RG_INTEGER, GL_SHORT, 4, GL_TRUE },
    { GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 8, GL_TRUE },
    { GL_RG32I, GL_RG_INTEGER, GL_INT, 8, GL_TRUE },
    { GL_R8, GL_RED, GL_UNSIGNED_BYTE, 1, GL_TRUE },
    { GL_R8_SNORM, GL_RED, GL_BYTE, 1, GL_FALSE },
    { GL_R16F, GL_RED, GL_HALF_FLOAT, 2, GL_FALSE },
    { GL_R32F, GL_RED, GL_FLOAT, 4, GL_FALSE },
    { GL_R16F, GL_RED, GL_FLOAT, 4, GL_FALSE },
    { GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1, GL_TRUE },
    { GL_R8I, GL_RED_INTEGER, GL_BYTE, 1, GL_TRUE },
    { GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 2, GL_TRUE },
    { GL_R16I, GL_RED_INTEGER, GL_SHORT, 2, GL_TRUE },
    { GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 4, GL_TRUE },
    { GL_R32I, GL_RED_INTEGER, GL_INT, 4, GL_TRUE },
    { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4, GL_TRUE },
    { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4, GL_TRUE },
    { GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 2, GL_TRUE },
    { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 4, GL_TRUE },
    { GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 4, GL_TRUE },
    { GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 8, GL_TRUE },
    { GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, 1, GL_TRUE},

    /* Table 3.3, unsized internal formats */
    { GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 4, GL_TRUE },
    { GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2, GL_TRUE },
    { GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 2, GL_TRUE },
    { GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, 3, GL_TRUE },
    { GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, GL_TRUE },
    { GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2, GL_FALSE },
    { GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 1, GL_FALSE },
    { GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, 1, GL_FALSE },

    /* To be compatible with OES2.0, unsize depth internal format */
    { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 2, GL_TRUE },
    { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4, GL_TRUE },
    { GL_DEPTH_STENCIL_OES, GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8, 4, GL_TRUE},

    /* required by GL_OES_required_internalformat */
    { GL_RGB8, GL_RGB, GL_UNSIGNED_INT_2_10_10_10_REV_EXT, 4, GL_TRUE },
    { GL_RGB565, GL_RGB, GL_UNSIGNED_INT_2_10_10_10_REV_EXT, 4, GL_TRUE },
    { GL_DEPTH_COMPONENT32_OES, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 4, GL_TRUE },
};


__GL_INLINE GLboolean __glIsCanonicalFormat(GLenum internalFormat, GLenum format, GLenum type)
{
    GLuint index;
    for (index = 0; index < __GL_TABLE_SIZE(canonicalformats); index++)
    {
        if ((canonicalformats[index].internalformat == internalFormat) &&
            (canonicalformats[index].format == format) &&
            (canonicalformats[index].type == type))
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

__GL_INLINE GLuint __glCheckMipHintDirty(__GLcontext *gc, __GLtextureObject* tex, GLint lod)
{
    GLuint dirty = 0;

    if (tex->params.mipHint != __GL_TEX_MIP_HINT_AUTO &&
        lod >= tex->mipBaseLevel && lod < tex->mipMaxLevel)
    {
        tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;
        dirty = __GL_TEXPARAM_MIP_HINT_BIT;
    }

    return dirty;
}

__GL_INLINE GLvoid __glSetTexImageDirtyBit(__GLcontext *gc, __GLtextureObject* tex, GLuint dirtyBit)
{
    GLuint i;

    for (i = 0; i < gc->constants.shaderCaps.maxCombinedTextureImageUnits; ++i)
    {
        if (tex->name == gc->texture.units[i].boundTextures[tex->targetIndex]->name)
        {
            __GL_SET_TEX_UNIT_BIT(gc, i, dirtyBit);
        }
    }
}

GLboolean __glCheckTexImgArgs(__GLcontext *gc,
                              __GLtextureObject *tex,
                              GLint   lod,
                              GLsizei width,
                              GLsizei height,
                              GLsizei depth,
                              GLint   border)
{
    GLint maxLod = (GLint)(gc->constants.maxNumTextureLevels - 1);
    GLint maxArray = (GLint)gc->constants.maxTextureArraySize;

    if (!tex)
    {
        return GL_FALSE;
    }

    if (tex->immutable)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    if (0 != border)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    /* Check lod, width, height */
    if (lod    < 0 || lod > maxLod ||
        width  < 0 || width  > (1 << (maxLod - lod)) ||
        height < 0 || height > (1 << (maxLod - lod)))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

#ifdef OPENGL40
    if(__GL_TEXTURE_1D_ARRAY_INDEX == tex->targetIndex )
    {
        if ((height < 0) || (height > maxArray))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
#endif

    /* Check depth */
    if (__GL_TEXTURE_2D_ARRAY_INDEX == tex->targetIndex)
    {
        /* depth in fact is the array size */
        if (depth < 0 || depth > maxArray)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
    else if (__GL_TEXTURE_CUBEMAP_ARRAY_INDEX == tex->targetIndex)
    {
        if ((((depth / 6) * 6) != depth) ||
            (depth / 6 > maxArray) ||
            (depth < 0))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
    else
    {
        if (depth < 0 || depth > (1 << (maxLod - lod)))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }

    if (__GL_IS_TEXTURE_CUBE(tex->targetIndex))
    {
        if (width != height)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }

    return GL_TRUE;
}

GLboolean __glCheckTexSubImgArgs(__GLcontext *gc,
                                 __GLtextureObject *tex,
                                 GLuint  face,
                                 GLint   lod,
                                 GLint   xoffset,
                                 GLint   yoffset,
                                 GLint   zoffset,
                                 GLsizei width,
                                 GLsizei height,
                                 GLsizei depth)
{
    __GLmipMapLevel *mipmap;
    GLint max_lod = (GLint)(gc->constants.maxNumTextureLevels - 1);
    GLint mipmapDepth;

    /* Check lod */
    if (lod < 0 || lod > max_lod)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    mipmap = &tex->faceMipmap[face][lod];

    if (width < 0 || height < 0 || depth < 0)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    mipmapDepth = ((__GL_TEXTURE_2D_ARRAY_INDEX == tex->targetIndex) ||
                   (__GL_TEXTURE_CUBEMAP_ARRAY_INDEX == tex->targetIndex))
                   ? mipmap->arrays : mipmap->depth;

    if (mipmap->requestedFormat >= GL_COMPRESSED_R11_EAC &&
        mipmap->requestedFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
    {
        if (((width  % 4) && (width  + xoffset != mipmap->width )) ||
            ((height % 4) && (height + yoffset != mipmap->height)) ||
            ((xoffset % 4) || (yoffset % 4)))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    /* Check xoffset, yoffset, zoffset, width, height, depth */
    if (xoffset < 0 || (xoffset + width)  > mipmap->width ||
        yoffset < 0 || (yoffset + height) > mipmap->height ||
        zoffset < 0 || (zoffset + depth)  > mipmapDepth)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    return GL_TRUE;
}

GLboolean __glCheckTexDirectFmt(__GLcontext *gc,
                                __GLtextureObject *tex,
                                GLenum target,
                                GLenum directFormat)
{
    if (!tex)
    {
        return GL_FALSE;
    }

    if (target != GL_TEXTURE_2D)
    {
        return GL_FALSE;
    }

    switch (directFormat)
    {
    case GL_VIV_YV12:
    case GL_VIV_I420:
    case GL_VIV_NV12:
    case GL_VIV_NV21:
    case GL_VIV_YUY2:
    case GL_VIV_UYVY:
    case GL_VIV_YUV420_10_ST:
    case GL_VIV_YUV420_TILE_ST:
    case GL_VIV_YUV420_TILE_10_ST:
    case GL_RGB565:
    case GL_RGB:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_RG8_EXT:
    case GL_ALPHA:
    case GL_LUMINANCE_ALPHA:
    case GL_DEPTH_COMPONENT16:
        return GL_TRUE;

    default:
        return GL_FALSE;
    }
}

GLboolean __glCheckTexImgTypeArg(__GLcontext *gc,
                                 __GLtextureObject *tex,
                                 GLenum type)
{
    GLboolean invalid = GL_FALSE;

    if (!tex)
    {
        return GL_FALSE;
    }

    switch (type)
    {
#ifdef OPENGL40

        case GL_BITMAP:
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        /* the same as GL_UNSIGNED_INT_2_10_10_10_REV_EXT */
        /*case GL_UNSIGNED_INT_2_10_10_10_REV:*/
        /* the same as GL_UNSIGNED_INT_5_9_9_9_REV */
        /*case GL_UNSIGNED_INT_5_9_9_9_REV_EXT:*/
        /* the same as GL_HALF_FLOAT */
        /*case GL_HALF_FLOAT_ARB:*/
        /* the same as GL_UNSIGNED_INT_10F_11F_11F_REV */
        /*case GL_UNSIGNED_INT_10F_11F_11F_REV_EXT:*/

#endif

        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
        case GL_FLOAT:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
        case GL_UNSIGNED_INT_5_9_9_9_REV:
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_DEPTH_COMPONENT24_OES:
        case GL_DEPTH_COMPONENT32_OES:
        case GL_UNSIGNED_INT_24_8:
        case GL_DEPTH24_STENCIL8:
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            break;

        default:
            invalid = GL_TRUE;
            break;
    }

    if (invalid)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}

GLboolean __glCheckTexImgInternalFmtArg(__GLcontext *gc,
                                        __GLtextureObject *tex,
                                        GLenum internalFormat)
{
    GLboolean invalid = GL_FALSE;

    if (!tex)
    {
        return GL_FALSE;
    }

    switch (internalFormat)
    {
#ifdef OPENGL40
        case 1:
        case 2:
        case 3:
        case 4:
        case GL_LUMINANCE4:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGBA2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_ALPHA4:
        case GL_ALPHA12:
        case GL_ALPHA16:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
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
        case GL_SRGB_ALPHA:
        case GL_SLUMINANCE_ALPHA:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_SLUMINANCE:
        case GL_SLUMINANCE8:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_SLUMINANCE:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_ALPHA32F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
            break;
#endif

        case GL_RED:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R16F:
        case GL_R32I:
        case GL_R32UI:
        case GL_R32F:

        case GL_RG:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG16F:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RG32F:

        case GL_RGB:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB16F:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGB32F:
        case GL_RGBA4:
        case GL_RGB565:
        case GL_RGB5_A1:
        case GL_RGB10_EXT:
        case GL_RGB9_E5:
        case GL_R11F_G11F_B10F:

        case GL_RGBA:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA16F:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_RGBA32F:
        case GL_BGRA_EXT:

        case GL_SRGB:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:

        case GL_LUMINANCE:
        case GL_LUMINANCE8_OES:
        case GL_ALPHA:
        case GL_ALPHA8_OES:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4_OES:
        case GL_LUMINANCE8_ALPHA8_OES:

        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32_OES:
        case GL_DEPTH_COMPONENT32F:
        case GL_STENCIL_INDEX8:
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:

            break;

        default:
            invalid = GL_TRUE;
            break;
    }

    if (invalid)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    return GL_TRUE;
}


GLboolean __glCheckTexImgFmtArg(__GLcontext *gc,
                                __GLtextureObject *tex,
                                GLenum format)
{
    GLboolean invalid = GL_FALSE;

    if (!tex)
    {
        return GL_FALSE;
    }

    switch (format)
    {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_RG:
        case GL_RG_INTEGER:
        case GL_RGB:
        case GL_RGB_INTEGER:
        case GL_RGBA:
        case GL_RGBA_INTEGER:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
        case GL_ALPHA:
        case GL_STENCIL_INDEX:
        case GL_VIV_YV12:
        case GL_VIV_I420:
        case GL_VIV_NV12:
        case GL_VIV_NV21:
        case GL_VIV_YUY2:
        case GL_VIV_UYVY:
        case GL_BGRA_EXT:
#ifdef OPENGL40
        case GL_BGR_EXT:
#endif
            break;
        default:
            invalid = GL_TRUE;
            break;
    }

    if (invalid)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}
#ifdef OPENGL40
GLboolean __glCheckTexImgFmtGL4(__GLcontext *gc,
                             __GLtextureObject *tex,
                             GLenum target,
                             GLint internalFormat,
                             GLenum format,
                             GLenum type)
{

    if (!tex)
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
        case GL_RGB565:
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
        case GL_RG:
        case GL_RG16:
        case GL_RG8:
        case GL_RED:
        case GL_R16:
        case GL_R8:
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
        case GL_RGBA8_SNORM:
        case GL_RGBA16_SNORM:
        case GL_RGB8_SNORM:
        case GL_RGB16_SNORM:
        case GL_RG8_SNORM:
        case GL_RG16_SNORM:
        case GL_R8_SNORM:
        case GL_R16_SNORM:
            switch (format)
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
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                    break;
                case GL_DEPTH_COMPONENT:
                case GL_DEPTH_STENCIL:
                    goto bad_operation;
                default:
                    break;
            }
            break;

            /*depth component*/
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
            if(!__glExtension[__GL_EXTID_ARB_depth_texture].bEnabled)
            {
    bad_enum:
                __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
            }
            if(format != GL_DEPTH_COMPONENT)
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

        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
            if(format != GL_DEPTH_STENCIL)
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
            if(!__glExtension[__GL_EXTID_EXT_texture_compression_s3tc].bEnabled)
                goto bad_enum;
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            if(!__glExtension[__GL_EXTID_EXT_texture_compression_latc].bEnabled)
                goto bad_enum;
            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;

        case GL_COMPRESSED_RED_RGTC1_EXT:
        case GL_COMPRESSED_SIGNED_RED_RGTC1_EXT:
        case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
            if(!__glExtension[__GL_EXTID_EXT_texture_compression_rgtc].bEnabled)
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
        case GL_RGB10_A2UI:
        case GL_RG32UI:
        case GL_RG32I:
        case GL_RG16UI:
        case GL_RG16I:
        case GL_RG8UI:
        case GL_RG8I:
        case GL_R32UI:
        case GL_R32I:
        case GL_R16UI:
        case GL_R16I:
        case GL_R8UI:
        case GL_R8I:
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
            if (!__glExtension[__GL_EXTID_EXT_texture_integer].bEnabled)
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
                if (type == GL_HALF_FLOAT || type == GL_FLOAT)
                    goto bad_operation;
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
            if (!__glExtension[__GL_EXTID_EXT_texture_sRGB].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;

        /* extension GL_EXT_texture_shared_exponent */
        case GL_RGB9_E5_EXT:
             if(!__glExtension[__GL_EXTID_EXT_texture_shared_exponent].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

             break;

        /* extension GL_ARB_texture_float */
        case GL_RGBA32F_ARB:
        case GL_RGB32F_ARB:
        case GL_RG32F_EXT:
        case GL_RG16F_EXT:
        case GL_R32F_EXT:
        case GL_R16F_EXT:
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
            if (!__glExtension[__GL_EXTID_ARB_texture_float].bEnabled)
                goto bad_enum;

            if(format == GL_DEPTH_COMPONENT)
                goto bad_operation;

            break;
        case GL_R11F_G11F_B10F_EXT:
            if (!__glExtension[__GL_EXTID_EXT_packed_float].bEnabled)
                goto bad_enum;

            if (format == GL_DEPTH_COMPONENT)
                goto bad_operation;
            break;

        default:
            goto bad_enum;
    }


/* Fix me for this werid style */
    if (!tex) {
bad_operation:
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    return GL_TRUE;

}
#endif

GLboolean __glCheckTexImgFmtES(__GLcontext *gc,
                             __GLtextureObject *tex,
                             GLenum target,
                             GLint internalFormat,
                             GLenum format,
                             GLenum type)
{
    GLboolean invalid = GL_FALSE;

    if (!tex)
    {
        return GL_FALSE;
    }

    switch (format)
    {
    case GL_BGRA_EXT:
        /* Fall through. */
    case GL_RGBA:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RGBA8 != internalFormat && GL_RGB5_A1 != internalFormat &&
                       GL_RGBA4 != internalFormat && GL_SRGB8_ALPHA8 != internalFormat &&
                       GL_RGBA != internalFormat && GL_BGRA_EXT != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RGBA8_SNORM != internalFormat);
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4:
            invalid = (GL_RGBA4 != internalFormat && GL_RGBA != internalFormat);
            break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
            invalid = (GL_RGB5_A1 != internalFormat && GL_RGBA != internalFormat);
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
            invalid = (GL_RGB10_A2 != internalFormat && GL_RGB5_A1 != internalFormat);
            if (__glExtension[__GL_EXTID_EXT_texture_type_2_10_10_10_REV].bEnabled)
            {
                invalid = invalid && (GL_RGBA != internalFormat);
            }
            break;
        case GL_HALF_FLOAT:
            invalid = (GL_RGBA16F != internalFormat);
            break;
        case GL_HALF_FLOAT_OES:
            invalid = (GL_RGBA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;
        case GL_FLOAT:
            invalid = (GL_RGBA32F != internalFormat && GL_RGBA16F != internalFormat && (GL_RGBA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_float].bEnabled));
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RGB:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RGB8 != internalFormat && GL_RGB565 != internalFormat &&
                       GL_SRGB8 != internalFormat && GL_RGB != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RGB8_SNORM != internalFormat);
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
            invalid = (GL_RGB565 != internalFormat && GL_RGB != internalFormat);
            break;
        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            invalid = (GL_R11F_G11F_B10F != internalFormat);
            break;
        case GL_UNSIGNED_INT_5_9_9_9_REV:
            invalid = (GL_RGB9_E5 != internalFormat);
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
            invalid = !(__glExtension[__GL_EXTID_EXT_texture_type_2_10_10_10_REV].bEnabled && GL_RGB == internalFormat) &&
                      !(__glExtension[__GL_EXTID_OES_required_internalformat].bEnabled && (GL_RGB10_EXT == internalFormat ||
                        GL_RGB == internalFormat || GL_RGB8 == internalFormat || GL_RGB565 == internalFormat));
            break;
        case GL_HALF_FLOAT:
            invalid = (GL_RGB16F != internalFormat && GL_R11F_G11F_B10F != internalFormat &&
                       GL_RGB9_E5 != internalFormat);
            break;
        case GL_HALF_FLOAT_OES:
            invalid = (GL_RGB != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;
        case GL_FLOAT:
            invalid = (GL_RGB32F != internalFormat && GL_RGB16F != internalFormat &&
                GL_R11F_G11F_B10F != internalFormat && GL_RGB9_E5 != internalFormat && (GL_RGB != internalFormat || !__glExtension[__GL_EXTID_OES_texture_float].bEnabled));
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RG:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RG8 != internalFormat && GL_RG_EXT != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RG8_SNORM != internalFormat);
            break;
        case GL_HALF_FLOAT:
            invalid = (GL_RG16F != internalFormat);
            break;
        case GL_HALF_FLOAT_OES:
            invalid = (GL_RG_EXT != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;
        case GL_FLOAT:
            invalid = (GL_RG32F != internalFormat && GL_RG16F != internalFormat && GL_RG_EXT != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RED:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = ((GL_R8 != internalFormat) && (GL_RED_EXT != internalFormat));
            break;
        case GL_BYTE:
            invalid = (GL_R8_SNORM != internalFormat);
            break;
        case GL_HALF_FLOAT:
            invalid = (GL_R16F != internalFormat);
            break;
        case GL_HALF_FLOAT_OES:
            invalid = (GL_RED_EXT != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;
        case GL_FLOAT:
            invalid = (GL_R32F != internalFormat && GL_R16F != internalFormat && GL_RED_EXT != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RGBA_INTEGER:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RGBA8UI != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RGBA8I != internalFormat);
            break;
        case GL_UNSIGNED_SHORT:
            invalid = (GL_RGBA16UI != internalFormat);
            break;
        case GL_SHORT:
            invalid = (GL_RGBA16I != internalFormat);
            break;
        case GL_UNSIGNED_INT:
            invalid = (GL_RGBA32UI != internalFormat);
            break;
        case GL_INT:
            invalid = (GL_RGBA32I != internalFormat);
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV_EXT:
            invalid = (GL_RGB10_A2UI != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RGB_INTEGER:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RGB8UI != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RGB8I != internalFormat);
            break;
        case GL_UNSIGNED_SHORT:
            invalid = (GL_RGB16UI != internalFormat);
            break;
        case GL_SHORT:
            invalid = (GL_RGB16I != internalFormat);
            break;
        case GL_UNSIGNED_INT:
            invalid = (GL_RGB32UI != internalFormat);
            break;
        case GL_INT:
            invalid = (GL_RGB32I != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RG_INTEGER:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_RG8UI != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_RG8I != internalFormat);
            break;
        case GL_UNSIGNED_SHORT:
            invalid = (GL_RG16UI != internalFormat);
            break;
        case GL_SHORT:
            invalid = (GL_RG16I != internalFormat);
            break;
        case GL_UNSIGNED_INT:
            invalid = (GL_RG32UI != internalFormat);
            break;
        case GL_INT:
            invalid = (GL_RG32I != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_RED_INTEGER:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_R8UI != internalFormat);
            break;
        case GL_BYTE:
            invalid = (GL_R8I != internalFormat);
            break;
        case GL_UNSIGNED_SHORT:
            invalid = (GL_R16UI != internalFormat);
            break;
        case GL_SHORT:
            invalid = (GL_R16I != internalFormat);
            break;
        case GL_UNSIGNED_INT:
            invalid = (GL_R32UI != internalFormat);
            break;
        case GL_INT:
            invalid = (GL_R32I != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            /* ES2 extension GL_OES_depth_texture requires internal format GL_DEPTH_COMPONENT */
            invalid = (GL_DEPTH_COMPONENT16 != internalFormat && GL_DEPTH_COMPONENT != internalFormat);
            break;
        case GL_UNSIGNED_INT:
            /* ES2 extension GL_OES_depth_texture requires internal format GL_DEPTH_COMPONENT */
            invalid = (GL_DEPTH_COMPONENT24 != internalFormat && GL_DEPTH_COMPONENT16 != internalFormat &&
                       GL_DEPTH_COMPONENT32_OES != internalFormat && GL_DEPTH_COMPONENT != internalFormat);
            break;
        case GL_FLOAT:
            invalid = (GL_DEPTH_COMPONENT32F != internalFormat);
            break;
        case GL_DEPTH_COMPONENT24_OES:
            /* Some applications (Taiji) use DEPTH_COMPONENT24_OES, even though it's not part of the spec. */
            invalid = (GL_DEPTH_COMPONENT24_OES != internalFormat && GL_DEPTH_COMPONENT != internalFormat);
            break;
        case GL_DEPTH_COMPONENT32_OES:
            /* Some applications use DEPTH_COMPONENT32_OES, even though it's not part of the spec. */
            invalid = (GL_DEPTH_COMPONENT32_OES != internalFormat && GL_DEPTH_COMPONENT != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_STENCIL_INDEX:
        if (GL_STENCIL_INDEX8 != internalFormat ||
            GL_UNSIGNED_BYTE != type)
        {
            invalid = GL_TRUE;
        }
        break;

    case GL_DEPTH_STENCIL_OES:
        switch (type)
        {
        case GL_UNSIGNED_INT_24_8:
        case GL_DEPTH24_STENCIL8:
            invalid = (GL_DEPTH24_STENCIL8 != internalFormat && GL_DEPTH_STENCIL_OES != internalFormat);
            break;
        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            invalid = (GL_DEPTH32F_STENCIL8 != internalFormat);
            break;
        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_LUMINANCE_ALPHA:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_LUMINANCE_ALPHA != internalFormat &&
                       GL_LUMINANCE4_ALPHA4_OES != internalFormat &&
                       GL_LUMINANCE8_ALPHA8_OES != internalFormat);
            break;
        case GL_FLOAT:
            invalid = (GL_LUMINANCE_ALPHA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_float].bEnabled);
            break;

        case GL_HALF_FLOAT_OES:
            invalid = (GL_LUMINANCE_ALPHA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;

        default:
            invalid = GL_TRUE;
            break;
        }
        break;
    case GL_LUMINANCE:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_LUMINANCE != internalFormat && GL_LUMINANCE8_OES != internalFormat);
            break;

        case GL_HALF_FLOAT_OES:
            invalid = (GL_LUMINANCE != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;
        case GL_FLOAT:
            invalid = (GL_LUMINANCE != internalFormat || !__glExtension[__GL_EXTID_OES_texture_float].bEnabled);
            break;

        default:
            invalid = GL_TRUE;
            break;
        }
        break;
    case GL_ALPHA:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            invalid = (GL_ALPHA != internalFormat && GL_ALPHA8_OES != internalFormat);
            break;

        case GL_FLOAT:
            invalid = (GL_ALPHA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_float].bEnabled);
            break;
        case GL_HALF_FLOAT_OES:
            invalid = (GL_ALPHA != internalFormat || !__glExtension[__GL_EXTID_OES_texture_half_float].bEnabled);
            break;

        default:
            invalid = GL_TRUE;
            break;
        }
        break;

    case GL_VIV_YV12:
    case GL_VIV_I420:
    case GL_VIV_NV12:
    case GL_VIV_NV21:
    case GL_VIV_YUY2:
    case GL_VIV_UYVY:
    case GL_VIV_AYUV:
        invalid = (GL_RGBA != internalFormat);
        break;

    case __GL_RGBX8:
        invalid = (__GL_RGBX8 != internalFormat);
        break;

    case __GL_BGRX8:
        invalid = (__GL_BGRX8 != internalFormat);
        break;

    default:
        invalid = GL_TRUE;
        break;
    }

    if (invalid)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    /*
    ** Textures with a base internal format of DEPTH_COMPONENT or DEPTH_-
    ** STENCIL are supported by texture image specification commands only if target is
    ** TEXTURE_2D, TEXTURE_2D_ARRAY, or TEXTURE_CUBE_MAP
    */
    if (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
    {
        switch (internalFormat)
        {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH_COMPONENT32_OES:
        case GL_DEPTH24_STENCIL8:
        case GL_DEPTH32F_STENCIL8:
        case GL_DEPTH_STENCIL_OES:
        case GL_STENCIL_INDEX8:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    switch (internalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_STENCIL_OES:
        tex->unsizedTexture = GL_TRUE;
        break;

    case GL_RGBA:
    case GL_RGB:
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE:
    case GL_ALPHA:
    case GL_BGRA_EXT:
        tex->unsizedTexture = GL_TRUE;
        break;

    default:
        tex->unsizedTexture = GL_FALSE;
        break;
    }

    tex->canonicalFormat = __glIsCanonicalFormat(internalFormat, format, type);

    return GL_TRUE;
}


GLboolean __glCheckTexImgFmt(__GLcontext *gc,
                             __GLtextureObject *tex,
                             GLenum target,
                             GLint internalFormat,
                             GLenum format,
                             GLenum type)
{

#ifdef OPENGL40
    return __glCheckTexImgFmtGL4(gc, tex, target, internalFormat, format, type);
#else
    return __glCheckTexImgFmtES(gc, tex, target, internalFormat, format, type);
#endif

}

GLuint __glGetNumberOfElement(GLenum format)
{
    switch(format)
    {
    case GL_DEPTH_COMPONENT:
    case GL_RED:
    case GL_LUMINANCE:
    case GL_ALPHA:
    case GL_RED_INTEGER:
        return 1;

    case GL_DEPTH_STENCIL:
    case GL_RG:
    case GL_LUMINANCE_ALPHA:
    case GL_RG_INTEGER:
        return 2;

    case GL_RGB:
    case GL_RGB_INTEGER:
        return 3;

    case GL_RGBA:
    case GL_RGBA_INTEGER:
    case GL_BGRA_EXT:
        return 4;
    }
    GL_ASSERT(0);
    return 0;
}

GLuint __glGetSizeOfType(GLenum type, GLboolean *packed)
{
    *packed = GL_FALSE;
    switch(type)
    {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
        return 1;

    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_HALF_FLOAT:
        return 2;

    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
        return 4;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        *packed = GL_TRUE;
        return 2;

    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_24_8:
    case GL_UNSIGNED_INT_10F_11F_11F_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV:
        *packed = GL_TRUE;
        return 4;

    case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        *packed = GL_TRUE;
        return 8;
    }
    GL_ASSERT(0);
    return 0;
}

GLboolean __glCheckPBO(__GLcontext *gc,
                       __GLpixelPackMode *packMode,
                       __GLbufferObject *bufObj,
                       GLsizei width,
                       GLsizei height,
                       GLsizei depth,
                       GLenum format,
                       GLenum type,
                       const GLvoid *buf)
{
    GLuint alignment = packMode->alignment;
    GLuint lineLength = packMode->lineLength ? packMode->lineLength : (GLuint)width;
    GLuint imageHeight = packMode->imageHeight ? packMode->imageHeight : (GLuint)height;
    GLuint skipPixels = packMode->skipPixels;
    GLuint skipLines = packMode->skipLines;
    GLuint skipImages = packMode->skipImages;

    GLuint numElement = __glGetNumberOfElement(format);
    GLboolean packed;
    GLuint sizeType = __glGetSizeOfType(type, &packed);
    GLuint bytePerPixel = packed ? sizeType : numElement * sizeType;

    GLuint rowStride = __GL_ALIGN(bytePerPixel * lineLength, alignment);
    GLuint imageStride = depth > 0 ? rowStride * imageHeight : 0;
    GLuint requireSize = 0;

    /* If a pixel unpack buffer object is bound and data is not evenly divisible by the number
     * of basic machine units needed to store in memory the corresponding GL data type
     */
    if (!sizeType || (__GL_PTR2UINT(buf) % sizeType) != 0)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    if (!bufObj || bufObj->bufferMapped)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    /* If a pixel unpack buffer object is bound and storing texture data would access
     * memory beyond the end of the pixel unpack buffer, an INVALID_OPERATION error results.
     */
    requireSize = __GL_PTR2UINT(buf)
                + (skipImages + depth - 1) * imageStride
                + (skipLines + height - 1) * rowStride
                + (skipPixels + width) * bytePerPixel;
    if (requireSize > (GLuint)bufObj->size)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    return GL_TRUE;
}

GLboolean __glCheckTexCopyImgFmt(__GLcontext *gc, __GLtextureObject * tex, GLint internalFormat, GLboolean compSizeMatch)
{
    __GLformatInfo* texFormatInfo;
    __GLformatInfo* rtFormatInfo;
    __GLframebufferObject *readFBO = gc->frameBuffer.readFramebufObj;

    if (!gc->dp.isFramebufferComplete(gc, readFBO))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_FRAMEBUFFER_OPERATION, GL_FALSE);
    }

    if (!((internalFormat == GL_DEPTH_COMPONENT) || (internalFormat == GL_DEPTH_COMPONENT16) || (internalFormat == GL_DEPTH_COMPONENT24) ||
        (internalFormat == GL_DEPTH_COMPONENT32) || (internalFormat == GL_DEPTH_COMPONENT32F) || (internalFormat == GL_STENCIL_INDEX) ||
        (internalFormat == GL_STENCIL_INDEX8) || (internalFormat == GL_DEPTH24_STENCIL8) || (internalFormat == GL_DEPTH32F_STENCIL8) ||
        (internalFormat == GL_DEPTH_STENCIL)))
    {
        if (READ_FRAMEBUFFER_BINDING_NAME)
        {
            __GLfboAttachPoint *attachPoint;
            GLint attachIndex;

            if (readFBO->readBuffer == GL_NONE)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            attachIndex = __glMapAttachmentToIndex(readFBO->readBuffer);
            if (-1 == attachIndex)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            attachPoint = &readFBO->attachPoint[attachIndex];

            if (0 == attachPoint->objName)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            if(readFBO->fbSamples > 0 && !(attachPoint->isExtMode))
            {
                 __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }

            rtFormatInfo = __glGetFramebufferFormatInfo(gc, readFBO, readFBO->readBuffer);
        }
        else
        {
#ifdef OPENGL40
            if (gc->state.pixel.readBuffer == GL_NONE)
#else
            if (gc->state.raster.readBuffer == GL_NONE)
#endif
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            rtFormatInfo = gc->drawablePrivate->rtFormatInfo;
        }

        if (!rtFormatInfo)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* According to spec3.1 table8.17, define effective internal format. */
        switch (internalFormat)
        {
        case GL_ALPHA:
            if (1 <= rtFormatInfo->redSize && rtFormatInfo->redSize <= 8)
            {
                texFormatInfo = __glGetFormatInfo(GL_ALPHA);
            }
            else
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_LUMINANCE:
            if (1 <= rtFormatInfo->redSize && rtFormatInfo->redSize <= 8)
            {
                texFormatInfo = __glGetFormatInfo(GL_LUMINANCE);
            }
            else
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_LUMINANCE_ALPHA:
            if ((1 <= rtFormatInfo->redSize && rtFormatInfo->redSize <= 8) &&
                (1 <= rtFormatInfo->alphaSize && rtFormatInfo->alphaSize <= 8))
            {
                texFormatInfo = __glGetFormatInfo(GL_LUMINANCE_ALPHA);
            }
            else
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_RGB:
            {
                if ((1 <= rtFormatInfo->redSize && rtFormatInfo->redSize <= 5) &&
                    (1 <= rtFormatInfo->greenSize && rtFormatInfo->greenSize <= 6) &&
                    (1 <= rtFormatInfo->blueSize && rtFormatInfo->blueSize <= 5))
                {
                    texFormatInfo = __glGetFormatInfo(GL_RGB565);
                }
                else if ((5 < rtFormatInfo->redSize && rtFormatInfo->redSize <= 8) &&
                         (6 < rtFormatInfo->greenSize && rtFormatInfo->greenSize <= 8) &&
                         (5 < rtFormatInfo->blueSize && rtFormatInfo->blueSize <= 8))
                {
                    texFormatInfo = __glGetFormatInfo(GL_RGB8);
                }
                else
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
                break;
            }
            break;
        case GL_RGBA:
            {
                if ((1 <= rtFormatInfo->redSize && rtFormatInfo->redSize <= 4) &&
                    (1 <= rtFormatInfo->greenSize && rtFormatInfo->greenSize <= 4) &&
                    (1 <= rtFormatInfo->blueSize && rtFormatInfo->blueSize <= 4) &&
                    (1 <= rtFormatInfo->alphaSize && rtFormatInfo->alphaSize <= 4))
                {
                    texFormatInfo = __glGetFormatInfo(GL_RGBA4);
                }
                else if ((4 < rtFormatInfo->redSize && rtFormatInfo->redSize <= 5) &&
                         (4 < rtFormatInfo->greenSize && rtFormatInfo->greenSize <= 5) &&
                         (4 < rtFormatInfo->blueSize && rtFormatInfo->blueSize <= 5) &&
                         (rtFormatInfo->alphaSize == 1))
                {
                    texFormatInfo = __glGetFormatInfo(GL_RGB5_A1);
                }
                else if ((4 < rtFormatInfo->redSize && rtFormatInfo->redSize <= 8) &&
                         (4 < rtFormatInfo->greenSize && rtFormatInfo->greenSize <= 8) &&
                         (4 < rtFormatInfo->blueSize && rtFormatInfo->blueSize <= 8) &&
                         (1 < rtFormatInfo->alphaSize && rtFormatInfo->alphaSize <= 8))
                {
                    texFormatInfo = __glGetFormatInfo(GL_RGBA8);
                }
                else
                {
                    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
                }
                break;
            }
            break;
        case GL_RGB9_E5:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            break;
        default:
            texFormatInfo = __glGetFormatInfo(internalFormat);
            break;
        }

        if (__GL_FMT_MAX == texFormatInfo->drvFormat)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        /*
        ** According to spec table 3.15, CopyTex(Sub) only supports A/RGB/RGBA.
        ** But I am not sure about the restrictions on sized format color buffer
        */
        switch (texFormatInfo->baseFormat)
        {
        case GL_RED:
        case GL_LUMINANCE:
            if (GL_RED != rtFormatInfo->baseFormat && GL_RG != rtFormatInfo->baseFormat &&
                GL_RGB != rtFormatInfo->baseFormat && GL_RGBA != rtFormatInfo->baseFormat)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_RG:
            if (GL_RG != rtFormatInfo->baseFormat && GL_RGB != rtFormatInfo->baseFormat &&
                GL_RGBA != rtFormatInfo->baseFormat)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_RGB:
            if (GL_RGBA != rtFormatInfo->baseFormat && GL_RGB != rtFormatInfo->baseFormat)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_RGBA:
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_BGRA_EXT:
            if (GL_RGBA != rtFormatInfo->baseFormat)
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* An INVALID_OPERATION error is generated if the component sizes of internalformat do not exactly match
        ** the corresponding component sizes of the source buffer’s effective internal format.
        */

        if ((internalFormat != GL_LUMINANCE) &&
            (internalFormat != GL_RG) &&
            (internalFormat != GL_RGB) &&
            (internalFormat != GL_RGBA) &&
            (internalFormat != GL_ALPHA) &&
            (internalFormat != GL_LUMINANCE_ALPHA) &&
            compSizeMatch
            )
        {
            if ((texFormatInfo->redSize && texFormatInfo->redSize != rtFormatInfo->redSize) ||
                (texFormatInfo->greenSize && texFormatInfo->greenSize != rtFormatInfo->greenSize) ||
                (texFormatInfo->blueSize && texFormatInfo->blueSize != rtFormatInfo->blueSize) ||
                (texFormatInfo->alphaSize && texFormatInfo->alphaSize != rtFormatInfo->alphaSize))
            {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
        }

        /* If integer RGBA data is required and color buffer is not integer
        */
        if ((GL_UNSIGNED_INT == texFormatInfo->category) != (GL_UNSIGNED_INT == rtFormatInfo->category))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        if ((GL_INT == texFormatInfo->category) != (GL_INT == rtFormatInfo->category))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* If floating- or fixed-point RGBA data is required and color buffer is integer
         */
        if ((GL_SIGNED_NORMALIZED == texFormatInfo->category || GL_UNSIGNED_NORMALIZED == texFormatInfo->category ||
             GL_FLOAT == texFormatInfo->category) &&
            (GL_UNSIGNED_INT == rtFormatInfo->category || GL_INT == rtFormatInfo->category))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* Signed normalized data must match with signed normalized or float source */
        if ((GL_SIGNED_NORMALIZED == texFormatInfo->category) &&
            (GL_FLOAT != rtFormatInfo->category) &&
            (GL_SIGNED_NORMALIZED != rtFormatInfo->category))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* unsigned normalized data must match with unsigned normalized or float source */
        if ((GL_UNSIGNED_NORMALIZED == texFormatInfo->category) &&
            (GL_FLOAT != rtFormatInfo->category) &&
            (GL_UNSIGNED_NORMALIZED != rtFormatInfo->category))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        /* If FBO encoding is SRGB and internalFormat is not one of SRGB, or
        ** FBO encoding is LINEAR and internalFormat is SRGB
        */
        if (rtFormatInfo->encoding != texFormatInfo->encoding)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }
    else
    {
        texFormatInfo = __glGetFormatInfo(internalFormat);
        if (__GL_FMT_MAX == texFormatInfo->drvFormat)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
        switch (texFormatInfo->baseFormat)
        {
        case GL_DEPTH_COMPONENT:
            if (READ_FRAMEBUFFER_BINDING_NAME ?
                !gc->frameBuffer.readFramebufObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX].objName : !gc->modes.haveDepthBuffer) {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_STENCIL:
            if (READ_FRAMEBUFFER_BINDING_NAME ?
                !gc->frameBuffer.readFramebufObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX].objName : !gc->modes.haveStencilBuffer) {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        case GL_DEPTH_STENCIL:
            if (READ_FRAMEBUFFER_BINDING_NAME ?
                (!gc->frameBuffer.readFramebufObj->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX].objName ||
                !gc->frameBuffer.readFramebufObj->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX].objName) :
                (!gc->modes.haveDepthBuffer || !gc->modes.haveStencilBuffer)) {
                __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
            }
            break;
        default:
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    tex->canonicalFormat = GL_TRUE;

    return GL_TRUE;
}

GLboolean __glCheckCompressedTexImgFmt(__GLcontext *gc, GLint internalFormat)
{
    switch (internalFormat)
    {
    case GL_ETC1_RGB8_OES:
    case GL_COMPRESSED_R11_EAC:
    case GL_COMPRESSED_SIGNED_R11_EAC:
    case GL_COMPRESSED_RG11_EAC:
    case GL_COMPRESSED_SIGNED_RG11_EAC:
    case GL_COMPRESSED_RGB8_ETC2:
    case GL_COMPRESSED_SRGB8_ETC2:
    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case GL_COMPRESSED_RGBA8_ETC2_EAC:
    case  GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        break;

#if defined(GL_KHR_texture_compression_astc_ldr)
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        if (__glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled)
        {
            break;
        }
#endif
#ifdef OPENGL40
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
            break;
#endif
        /* fall through */
    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    return GL_TRUE;
}

GLsizei __glCompressedTexImageSize(GLint lods, GLint internalFormat, GLint width, GLint height, GLint depth)
{
    struct astcblocksize
    {
        GLsizei width;
        GLsizei height;
    };

    static struct astcblocksize astcblocksizearray[] =
    {
        {  4,  4 },
        {  5,  4 },
        {  5,  5 },
        {  6,  5 },
        {  6,  6 },
        {  8,  5 },
        {  8,  6 },
        {  8,  8 },
        { 10,  5 },
        { 10,  6 },
        { 10,  8 },
        { 10, 10 },
        { 12, 10 },
        { 12, 12 }
    };

    GLsizei blockWidth = 0;
    GLsizei blockHeight = 0;
    GLsizei countX, countY;
    GLsizei blockSize = 0;
    GLsizei paletteSize = 0;
    GLsizei bitsPerIndex = 0;
#if defined(GL_KHR_texture_compression_astc_ldr)
    GLsizei index;
#endif

    switch (internalFormat)
    {
    case GL_ETC1_RGB8_OES:
    case GL_COMPRESSED_R11_EAC:
    case GL_COMPRESSED_SIGNED_R11_EAC:
    case GL_COMPRESSED_RGB8_ETC2:
    case GL_COMPRESSED_SRGB8_ETC2:
    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        blockWidth = 4;
        blockHeight = 4;
        blockSize = 8;
        break;

    case GL_COMPRESSED_RGBA8_ETC2_EAC:
    case  GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
    case GL_COMPRESSED_RG11_EAC:
    case GL_COMPRESSED_SIGNED_RG11_EAC:

    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        blockWidth = 4;
        blockHeight = 4;
        blockSize = 16;
        break;

    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE4_R5_G6_B5_OES:
        paletteSize = 16 * 2; /* entries * bytes per entry */
        bitsPerIndex = 4;
        break;
    case GL_PALETTE4_RGB8_OES:
        paletteSize = 16 * 3; /* entries * bytes per entry */
        bitsPerIndex = 4;
        break;
    case GL_PALETTE4_RGBA8_OES:
        paletteSize = 16 * 4; /* entries * bytes per entry */
        bitsPerIndex = 4;
        break;
    case GL_PALETTE8_RGBA4_OES:
    case GL_PALETTE8_RGB5_A1_OES:
    case GL_PALETTE8_R5_G6_B5_OES:
        paletteSize = 256 * 2; /* entries * bytes per entry */
        bitsPerIndex = 8;
        break;
    case GL_PALETTE8_RGB8_OES:
        paletteSize = 256 * 3; /* entries * bytes per entry */
        bitsPerIndex = 8;
        break;
    case GL_PALETTE8_RGBA8_OES:
        paletteSize = 256 * 4; /* entries * bytes per entry */
        bitsPerIndex = 8;
        break;

    /*
    ** How is the imageSize argument calculated for the CompressedTexImage2D and CompressedTexSubImage2D function.
    ** Resolution:
    ** For PVRTC 4BPP formats the imageSize is calculated as: ( max(width, 8) * max(height, 8) * 4 + 7) / 8
    ** For PVRTC 2BPP formats the imageSize is calculated as: ( max(width, 16) * max(height, 8) * 2 + 7) / 8
    */

#if defined(GL_KHR_texture_compression_astc_ldr)
    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
        index = internalFormat - GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
        blockWidth  = astcblocksizearray[index].width;
        blockHeight = astcblocksizearray[index].height;
        blockSize = 16;
        break;

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        index = internalFormat - GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
        blockWidth  = astcblocksizearray[index].width;
        blockHeight = astcblocksizearray[index].height;
        blockSize = 16;
        break;
#endif

    default:
        GL_ASSERT(0);
        return 0;
    }

    if (paletteSize)
    {
        GLint i;
        GLint w = width;
        GLint h = height;
        GLsizei indexSize = 0;
        for (i = 0; i <= lods; ++i)
        {
            indexSize += (GLsizei)(__GL_ALIGN(w * bitsPerIndex, 8) / 8) * h;
            w = __GL_MAX(1, w/2);
            h = __GL_MAX(1, h/2);
        }

        return paletteSize + indexSize;
    }

    countX = (width  + blockWidth  - 1) / blockWidth;
    countY = (height + blockHeight - 1) / blockHeight;

    return (GLsizei) (countX * countY * blockSize * depth);
}

GLvoid __glSetMipmapBorder(__GLcontext *gc, __GLtextureObject *tex, GLint face,
                                 GLint lod, GLint border)
{
#ifdef OPENGL40
    __GLmipMapLevel *mipmap;
    mipmap = &tex->faceMipmap[face][lod];
    mipmap->border = border;
#endif
}

#ifdef OPENGL40
GLint __glTextureBaseFormat(GLint internalFormat)
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

        case GL_DEPTH_STENCIL:
            return GL_DEPTH_STENCIL;

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
#endif

GLboolean __glSetMipmapLevelInfo(__GLcontext *gc, __GLtextureObject *tex, GLint face,
                              GLint lod, GLint internalFormat, GLenum format,
                              GLenum type, GLsizei width, GLsizei height, GLsizei depth)
{
    GLint arrays = 1;
    __GLmipMapLevel *mipmap;
    __GLmipMapLevel *currentMipmap;
    __GLformatInfo *formatInfo = NULL;
    GLboolean paletted;
    GLint requestedFormat = internalFormat;

/*
#ifdef OPENGL40
       internalFormat = __glTextureBaseFormat( internalFormat);
#endif
*/

    /* For some unsized internal formats, need consider corresponding "type" to
    ** get best matched chosen formats for better performance.
    */
    switch (internalFormat)
    {
    case GL_RED:
        switch (type)
        {
        case GL_HALF_FLOAT_OES:
            formatInfo = &__glFormatInfoTable[__GL_FMT_R16F];
            requestedFormat = GL_R16F;
            break;
        case GL_FLOAT:
            formatInfo = &__glFormatInfoTable[__GL_FMT_R32F];
            break;
        default:
            formatInfo = &__glFormatInfoTable[__GL_FMT_R8];
            break;
        }
        break;

    case GL_RG:
        switch (type)
        {
        case GL_HALF_FLOAT_OES:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RG16F];
            requestedFormat = GL_RG16F;
            break;
        case GL_FLOAT:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RG32F];
            break;
        default:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RG8];
            break;
        }
        break;

    case GL_RGB:
        switch (type)
        {
        case GL_UNSIGNED_SHORT_5_6_5:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB565];
            requestedFormat = GL_RGB565;
            break;
         case GL_FLOAT:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB32F];
            requestedFormat = GL_RGB32F;
            break;
        case GL_HALF_FLOAT_OES:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB16F];
            requestedFormat = GL_RGB16F;
            break;
#ifdef OPENGL40
        case GL_UNSIGNED_SHORT:
#if (defined(_DEBUG) || defined(DEBUG))
            gcoOS_Print("TO DO: transfer RGB16 to RGB8.\n");
#endif
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB8];
            requestedFormat = GL_RGB16;
            break;
#endif
        default:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB8];
            break;
        }
        break;

    case GL_RGBA:
        switch (type)
        {
        case GL_UNSIGNED_SHORT_4_4_4_4:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGBA4];
            requestedFormat = GL_RGBA4;
            break;
        case GL_UNSIGNED_SHORT_5_5_5_1:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB5_A1];
            requestedFormat = GL_RGB5_A1;
            break;
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGB10_A2];
            requestedFormat = GL_RGB10_A2;
            break;
         case GL_FLOAT:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGBA32F];
            requestedFormat = GL_RGBA32F;
            break;
        case GL_HALF_FLOAT_OES:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGBA16F];
            requestedFormat = GL_RGBA16F;
            break;
#ifdef OPENGL40
        case GL_UNSIGNED_SHORT:
#if (defined(_DEBUG) || defined(DEBUG))
            gcoOS_Print("TO DO: transfer RGBA16 to RGBA8.\n");
#endif
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGBA8];
            requestedFormat = GL_RGBA16;
            break;
#endif
        default:
            formatInfo = &__glFormatInfoTable[__GL_FMT_RGBA8];
            break;
        }
        break;

    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            formatInfo = &__glFormatInfoTable[__GL_FMT_Z16];
            break;
        default:
            formatInfo = &__glFormatInfoTable[__GL_FMT_Z24];
            break;
        }
        break;

    case GL_LUMINANCE:
        if(type == GL_FLOAT)
            formatInfo = & __glFormatInfoTable[__GL_FMT_L32F];
        else
            formatInfo = __glGetFormatInfo(internalFormat);
        break;

    case GL_ALPHA:
        if(type == GL_FLOAT)
            formatInfo = & __glFormatInfoTable[__GL_FMT_A32F];
        else
            formatInfo = __glGetFormatInfo(internalFormat);
        break;
    case GL_LUMINANCE_ALPHA:
        if(type == GL_FLOAT)
            formatInfo = & __glFormatInfoTable[__GL_FMT_LA32F];
        else
            formatInfo = __glGetFormatInfo(internalFormat);
        break;

    default:
        formatInfo = __glGetFormatInfo(internalFormat);
        break;
    }

    GL_ASSERT(formatInfo->drvFormat < __GL_FMT_MAX);

    paletted = (formatInfo->drvFormat >= __GL_FMT_PALETTE4_RGBA4_OES &&
                formatInfo->drvFormat <= __GL_FMT_PALETTE8_RGBA8_OES) ? GL_TRUE : GL_FALSE;

    if (__GL_IS_TEXTURE_ARRAY(tex->targetIndex))
    {
        arrays = depth;
        depth  = 1;
    }

    /* For texStorage call, use base format and type */
    if ((GL_NONE == format) && (GL_NONE == type))
    {
        format = formatInfo->dataFormat;
        type   = formatInfo->dataType;
    }

    /* For paletted texture, need to go through all levels, begin with level 0 */
    mipmap = paletted ? &tex->faceMipmap[face][0] : &tex->faceMipmap[face][lod];

    mipmap->formatInfo = formatInfo;
    mipmap->compressed = formatInfo->compressed;
    mipmap->baseFormat = formatInfo->baseFormat;
    mipmap->requestedFormat = requestedFormat;

    mipmap->width  = width;
    mipmap->height = height;
    mipmap->depth  = depth;
    mipmap->arrays = arrays;
    mipmap->format = format;
    mipmap->type   = type;

    if (mipmap->compressed)
    {
        mipmap->compressedSize = __glCompressedTexImageSize(lod, internalFormat, width, height, depth);
    }

    /* Initialize other lods if it is paletted texture */
    if (paletted)
    {
        GLint lodIdx;
        GLsizei w = width;
        GLsizei h = height;

        for (lodIdx = 1; lodIdx <= lod; ++lodIdx)
        {
            currentMipmap = &tex->faceMipmap[face][lodIdx];
            __GL_MEMCOPY(currentMipmap, mipmap, sizeof(__GLmipMapLevel));

            /* Half the size */
            w = __GL_MAX(1, w/2);
            h = __GL_MAX(1, h/2);
            currentMipmap->width = w;
            currentMipmap->height = h;
        }
    }

    return GL_TRUE;
}

GLvoid __glClearMipmapLevelInfo(__GLcontext *gc, __GLtextureObject *tex, GLint face, GLint level)
{
    __GLmipMapLevel *mipmap = &tex->faceMipmap[face][level];

    __GL_MEMZERO(mipmap, sizeof(__GLmipMapLevel));
    mipmap->requestedFormat = GL_RGBA;
    mipmap->formatInfo = NULL;
}

/*
 * Check cubemap texture for cube completeness.
 */
GLboolean __glIsCubeBaselevelConsistent(__GLcontext *gc, __GLtextureObject *tex)
{
    __GLmipMapLevel *level = NULL;
    GLint width, height, depth;
    GLint base, face;
    GLint requestedFormat;

    if (tex->targetIndex != __GL_TEXTURE_CUBEMAP_INDEX)
    {
        return GL_FALSE;
    }

    base = tex->params.baseLevel;
    width = tex->faceMipmap[0][base].width;
    height = tex->faceMipmap[0][base].height;
    depth = tex->faceMipmap[0][base].depth;
    requestedFormat = tex->faceMipmap[0][base].requestedFormat;

    /* If each dimension of the level[base] array is positive. */
    if (width == 0 || height == 0 || depth == 0)
    {
        return GL_FALSE;
    }

    /*
     * Check for CubeMap base level consistency for all six faces
     */

    /* If Cubemap baselevel has square dimension */
    if (width != height)
    {
        return GL_FALSE;
    }

    /* If the six baselevel images have identical dimension, internalformat */
    for (face = 1; face < 6; ++face)
    {
        level = &tex->faceMipmap[face][base];
        if (requestedFormat != level->requestedFormat ||
            width != level->width ||
            height != level->height)
        {
            return GL_FALSE;
        }
    }

    return GL_TRUE;
}

EGLenum __glCheckEglImageTexArg(__GLcontext *gc,
                               EGLenum target,
                               __GLtextureObject *texObj,
                               khrIMAGE_TYPE *type,
                               GLint *face)
{
    /* Test target parameter. */
    switch (target)
    {
    case EGL_GL_TEXTURE_2D_KHR:
        *face = 0;
        *type = KHR_IMAGE_TEXTURE_2D;
        break;

    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
    case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
        *face = target - EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR;
        *type = KHR_IMAGE_TEXTURE_CUBE;
        break;

    default:
        return EGL_BAD_PARAMETER;
    }

    if ((texObj == gcvNULL) ||
        (texObj->privateData == gcvNULL))
    {
        return EGL_BAD_PARAMETER;
    }

    return EGL_SUCCESS;
}

EGLenum __glCreateEglImageTexture(__GLcontext* gc,
                                  EGLenum target,
                                  GLint texture,
                                  GLint level,
                                  GLint depth,
                                  void * image)
{
    khrIMAGE_TYPE type;
    GLint face = 0;
    EGLenum result;
    __GLtextureObject *texObj = gcvNULL;

    if (gc->texture.shared == gcvNULL)
    {
        return EGL_BAD_PARAMETER;
    }
    /* Find the texture object by name. */
    texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, texture);

    result = __glCheckEglImageTexArg(gc, target, texObj, &type, &face);

    if (result != EGL_SUCCESS)
    {
        return result;
    }

    result = (*gc->dp.createEglImageTexture)(gc, texObj, face, level, depth, image);

    if (result != EGL_SUCCESS)
    {
        return result;
    }

    ((khrEGL_IMAGE*)image)->type = type;
    ((khrEGL_IMAGE*)image)->u.texture.texture = texture;

    return EGL_SUCCESS;
}

EGLenum __glCreateEglImageRenderbuffer(__GLcontext* gc, GLuint renderbuffer, void * image)
{
    EGLenum result;

    result = __createEglImageRenderbuffer(gc, renderbuffer, image);

    if (result != EGL_SUCCESS)
    {
        return result;
    }

    ((khrEGL_IMAGE*)image)->u.texture.texture = renderbuffer;

    return EGL_SUCCESS;
}

GLboolean
__glBindTexImage(
    __GLcontext *gc,
    GLenum format,
    GLboolean mipmap,
    GLint level,
    GLint width,
    GLint height,
    void * surface,
    void ** pBinder
    )
{
    __GLtextureObject *tex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[__GL_TEXTURE_2D_INDEX];

    __glSetMipmapBorder(gc, tex, 0, level, 0);

    if (__glSetMipmapLevelInfo(gc, tex, 0, level, format, format,
                               GL_UNSIGNED_BYTE, width, height, 1) == GL_FALSE)
    {
        return GL_FALSE;
    }

    if (gc->dp.bindTexImage(gc, tex, level, surface, pBinder) == GL_FALSE)
    {
        return GL_FALSE;
    }

    if (mipmap && level == 0)
    {
        gc->immedModeDispatch.GenerateMipmap(gc, GL_TEXTURE_2D);
    }

    return GL_TRUE;
}

__GL_INLINE GLboolean __glCopyTexBegin(__GLcontext *gc)
{
    if (gc->flags & __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER)
    {
        return GL_FALSE;
    }
    else
    {
        return (*gc->dp.copyTexBegin)(gc);
    }
}

__GL_INLINE GLvoid __glCopyTexValidateState(__GLcontext *gc)
{
    (*gc->dp.copyTexValidateState)(gc);
}

__GL_INLINE GLvoid __glCopyTexEnd(__GLcontext *gc, __GLtextureObject *tex, GLint lod)
{
    GLuint mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    (*gc->dp.copyTexEnd)(gc);
}

/*
** Check whether a tex miplevel was attached to the fbo.
** If level/face is "-1", it will not be checked. It's useful for TexStorage and palette texture.
*/
__GL_INLINE GLboolean __glFboIsTexAttached(__GLcontext *gc, __GLframebufferObject *fbo, __GLtextureObject *tex, GLint level, GLint face)
{
    /* Tex cannot be bound to default fbo */
    if (tex && fbo && fbo->name > 0)
    {
        GLuint i;

        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
        {
            if (fbo->attachPoint[i].objType == GL_TEXTURE &&
                fbo->attachPoint[i].objName == tex->name &&
                (fbo->attachPoint[i].level  == level || -1 == level) &&
                (fbo->attachPoint[i].face    == face || -1 == face))
            {
                return GL_TRUE;
            }
        }
    }

    return GL_FALSE;
}

__GL_INLINE GLvoid __glSetTexAttachedFboDirty(__GLcontext *gc, __GLtextureObject *tex, GLint level, GLint face)
{
    if (tex && tex->fboList)
    {
        __GLimageUser *imageUserList = tex->fboList;
        __GLframebufferObject *drawFbo = gc->frameBuffer.drawFramebufObj;
        __GLframebufferObject *readFbo = gc->frameBuffer.readFramebufObj;

        /* Mark dirty all the fbos this texture attached to */
        while (imageUserList)
        {
            __GL_FRAMEBUFFER_COMPLETE_DIRTY((__GLframebufferObject*)imageUserList->imageUser);
            imageUserList = imageUserList->next;
        }

        if (drawFbo == readFbo)
        {
            /* If draw and read FBO are same, check once. */
            if (__glFboIsTexAttached(gc, drawFbo, tex, level, face))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_DRAW_READ_BITS;
            }
        }
        else
        {
            if (__glFboIsTexAttached(gc, drawFbo, tex, level, face))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_DRAW_BIT;
            }

            if (__glFboIsTexAttached(gc, readFbo, tex, level, face))
            {
                gc->drawableDirtyMask |= __GL_BUFFER_READ_BIT;
            }
        }
    }
}

#if gcdFORCE_MIPMAP
__GL_INLINE GLvoid __glForceGenerateMipmap(__GLcontext* gc)
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if (patchId == gcvPATCH_BMGUI)
    {
        gc->immedModeDispatch.GenerateMipmap(gc,GL_TEXTURE_2D);
    }
}
#endif

GLvoid GL_APIENTRY __glim_TexImage3D(__GLcontext *gc,
                                     GLenum target,
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
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    __GL_TEXIMAGE3D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, lod, width, height, depth, border))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgInternalFmtArg(gc, tex, internalFormat))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, internalFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, height, depth, format, type, buf))
        {
            __GL_EXIT();
        }
    }

    __glSetMipmapBorder(gc, tex, 0, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, lod, internalFormat,
                               format, type, width, height, depth) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage3D)(gc, tex, lod, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, lod, 0);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexImage2D(__GLcontext *gc,
                                     GLenum target,
                                     GLint lod,
                                     GLint internalFormat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLint border,
                                     GLenum format,
                                     GLenum type,
                                     const GLvoid *buf)
{
    GLint face;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, lod, width, height, 1, border))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgInternalFmtArg(gc, tex, internalFormat))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, internalFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, height, 0, format, type, buf))
        {
            __GL_EXIT();
        }
    }
    __glSetMipmapBorder(gc, tex, face, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalFormat,
                               format, type, width, height, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage2D)(gc, tex, face, lod, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, lod, face);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

#if gcdFORCE_MIPMAP
    if (target == GL_TEXTURE_2D && lod == 0 && buf != gcvNULL)
    {
        __glForceGenerateMipmap(gc);
    }
#endif

    tex->seqNumber++;

OnExit:
    __GL_FOOTER();
}

#ifdef OPENGL40
GLboolean __glCheckTexMultisampleArgs(__GLcontext *gc,
                                  __GLtextureObject *tex,
                                  GLsizei levels,
                                  GLenum internalformat,
                                  GLsizei width,
                                  GLsizei height,
                                  GLsizei depth,
                                  GLsizei samples)
{
    GLsizei maxSize = __GL_MAX(width, height);

    if (!tex)
    {
        return GL_FALSE;
    }

    if (width < 0 || height < 0 || depth < 0)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    if (__GL_TEXTURE_2D_MS_ARRAY_INDEX == tex->targetIndex)
    {
         if (depth > (GLsizei)gc->constants.maxTextureArraySize)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
    else
    {
        maxSize = __GL_MAX(maxSize, depth);
    }

    if (maxSize > (GLsizei)gc->constants.maxTextureSize)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    switch (internalformat)
    {
    /* RGBA NORM */
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_BGRA_EXT:
    case GL_SRGB8_ALPHA8:
    case GL_RGB10_A2:
    case GL_RGBA16F:
    case GL_RGBA32F:
    /* RGB NORM */
    case GL_RGB565:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_SRGB8:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB16F:
    case GL_RGB32F:
    /* RG NORM */
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    /* R NORM */
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16F:
    case GL_R32F:
    /* unsized internal format */
    case GL_RED:
    case GL_RG:
    case GL_RGBA:
    case GL_RGB:
        if (samples > (GLsizei)gc->constants.maxSamplesInteger)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;
    /* RGBA INTEGER */
    case GL_RGBA8UI:
    case GL_RGBA8I:
    case GL_RGBA16UI:
    case GL_RGBA16I:
    case GL_RGBA32UI:
    case GL_RGBA32I:
    case GL_RGB10_A2UI:
    /* RGB INTEGER */
    case GL_RGB8UI:
    case GL_RGB8I:
    case GL_RGB16UI:
    case GL_RGB16I:
    case GL_RGB32UI:
    case GL_RGB32I:
    /* RG INTEGER */
    case GL_RG8UI:
    case GL_RG8I:
    case GL_RG16UI:
    case GL_RG16I:
    case GL_RG32UI:
    case GL_RG32I:
    /* R INTEGER */
    case GL_R8UI:
    case GL_R8I:
    case GL_R16UI:
    case GL_R16I:
    case GL_R32UI:
    case GL_R32I:
        if (samples > (GLsizei)gc->constants.maxSamplesInteger)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;


    /* DEPTH STENCIL */
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32F:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
    case GL_STENCIL_INDEX8:
        if (samples > (GLsizei)gc->constants.maxSamples)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;



    default:
    /*according to spec, an INVALID_ENUM error is generated if internalformat is one of the unsized base internal formats*/
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    if ((tex->targetIndex == __GL_TEXTURE_2D_MS_INDEX) ||
        (tex->targetIndex == __GL_TEXTURE_2D_MS_ARRAY_INDEX))
    {
        __GLformatInfo *formatInfo = __glGetFormatInfo(internalformat);
        GLint formatSamples = 0;

        (*gc->dp.queryFormatInfo)(gc, formatInfo->drvFormat, gcvNULL, &formatSamples, 1);

        /* Must be renderable formats */
        /*
        if (!formatInfo->renderable)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        */

        if (samples == 0 || samples > formatSamples)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }

    /* Always a canonical format. */
    tex->canonicalFormat = GL_TRUE;

    return GL_TRUE;
}

GLvoid GL_APIENTRY __glim_TexImage1D( __GLcontext *gc,
                                     GLenum target,
                                     GLint level,
                                     GLint internalFormat,
                                     GLsizei width,
                                     GLint border,
                                     GLenum format,
                                     GLenum type,
                                     const GLvoid *pixels )
{

    GLint lod = level;
    const GLvoid *buf = pixels;
    GLint face = 0;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE1D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, lod, width, 1 + border*2, 1 + border*2, border))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgInternalFmtArg(gc, tex, internalFormat))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, internalFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, 1, 0, format, type, buf))
        {
            __GL_EXIT();
        }
    }
    __glSetMipmapBorder(gc, tex, face, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalFormat,
                               format, type, width, 1, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage1D)(gc, tex, lod, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, lod, face);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
    __GL_FOOTER();

}

GLvoid GL_APIENTRY __glim_TexSubImage1D(__GLcontext *gc,
                                     GLenum target,
                                     GLint level,
                                     GLint xoffset,
                                     GLsizei width,
                                     GLenum format,
                                     GLenum type,
                                     const GLvoid *pixels )
{

    GLint lod = level;
    const GLvoid *buf = pixels;
    GLint face = 0;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXSUBIMAGE1D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, 0, 0, width, 1, 1))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[face][lod].requestedFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, 1, 0, format, type, buf))
        {
            __GL_EXIT();
        }
    }

    if (width * 1 == 0)
    {
        __GL_EXIT();
    }

    tex->faceMipmap[face][lod].format = format;
    tex->faceMipmap[face][lod].type   = type;
    if (!(*gc->dp.texSubImage1D)(gc, tex, lod, xoffset, width, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);
    tex->seqNumber++;

OnExit:
    __GL_FOOTER();
}

GLvoid APIENTRY __glim_CopyTexImage1D(__GLcontext *gc,
                                    GLenum target,
                                    GLint lod,
                                    GLenum internalFormat,
                                    GLint x,
                                    GLint y,
                                    GLsizei width,
                                    GLint border)
{
    GLint face = 0;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLboolean retVal;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE1D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexCopyImgFmt(gc, tex, internalFormat, GL_TRUE))
    {
        __GL_EXIT();
    }
    if (!__glCheckTexImgArgs(gc, tex, lod, width, 1, 1, border))
    {
        __GL_EXIT();
    }
    __glSetMipmapBorder(gc, tex, face, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalFormat,
                               0, 0, width, 1, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (__glCopyTexBegin(gc))
    {
        __glCopyTexValidateState(gc);

        retVal = (*gc->dp.copyTexImage1D)(gc, tex, lod, x, y);

        __glCopyTexEnd(gc, tex, lod);

        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }

        /* Set dirty for the texture attached FBOs */
        __glSetTexAttachedFboDirty(gc, tex, lod, face);

        tex->seqNumber++;
    }

OnExit:
    __GL_FOOTER();

}

GLvoid GL_APIENTRY __glim_CopyTexSubImage1D(__GLcontext *gc,
                                    GLenum target,
                                    GLint level,
                                    GLint xoffset,
                                    GLint x,
                                    GLint y,
                                    GLsizei width )
{
    GLint lod = level;
    GLint face = 0;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLboolean retVal;

    __GL_HEADER();

    /*get the texture object and face*/
    __GL_TEXIMAGE1D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, 0, 0, width, 1, 1))
    {
        __GL_EXIT();
    }
    if (!__glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[face][lod].requestedFormat, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (width * 1 == 0)
    {
        __GL_EXIT();
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (__glCopyTexBegin(gc))
    {
        __glCopyTexValidateState(gc);

        retVal = (*gc->dp.copyTexSubImage1D)(gc, tex, lod, x, y, width,xoffset);

        __glCopyTexEnd(gc, tex, lod);

        if (retVal)
        {
            tex->seqNumber++;
        }
        else
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnExit:
    __GL_FOOTER();

}

GLvoid GL_APIENTRY __glim_CompressedTexImage1D(__GLcontext *gc,
                                    GLenum target,
                                    GLint level,
                                    GLenum internalformat,
                                    GLsizei width,
                                    GLint border,
                                    GLsizei imageSize,
                                    const GLvoid *data )
{
    GLint lod = level;
    GLint face = 0;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    GLboolean isPalette = GL_FALSE;
    __GLtextureObject *tex;
    __GLmipMapLevel *mipmap;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE1D_GET_OBJECT();

    if (imageSize < 0)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    /* Check for paletted texture */
    switch (internalformat)
    {
    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE4_R5_G6_B5_OES:
    case GL_PALETTE4_RGB8_OES:
    case GL_PALETTE4_RGBA8_OES:
    case GL_PALETTE8_RGBA4_OES:
    case GL_PALETTE8_RGB5_A1_OES:
    case GL_PALETTE8_R5_G6_B5_OES:
    case GL_PALETTE8_RGB8_OES:
    case GL_PALETTE8_RGBA8_OES:
        if (lod > 0)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        lod = -lod;
        /* If number of lods exceeded the size allowed one */
        if (lod > (GLint)__glFloorLog2(__GL_MAX(width, 1)))
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        level = 0;
        isPalette = GL_TRUE;
        break;

    default:
        if (!__glCheckCompressedTexImgFmt(gc, internalformat))
        {
            __GL_EXIT();
        }
        level = lod;
        break;
    }

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, level, width, 1, 1, border))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if(unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    __glSetMipmapBorder(gc, tex, face, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalformat,
                               0, 0, width, 1, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[face][level];
    if (imageSize != mipmap->compressedSize)
    {
        __glSetMipmapBorder(gc, tex, 0, lod, 0);
        /* Clear level info to make texture incomplete */
        __glSetMipmapLevelInfo(gc, tex, 0, lod, internalformat, 0, 0, 0, 0, 0);
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!(*gc->dp.compressedTexImage1D)(gc, tex, lod, data))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, isPalette?-1:lod, face);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();

}


GLvoid APIENTRY __glim_CompressedTexSubImage1D(__GLcontext *gc,
                                               GLenum target,
                                               GLint lod,
                                               GLint xoffset,
                                               GLsizei width,
                                               GLenum format,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    GLint face = 0;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLmipMapLevel *mipmap;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXSUBIMAGE1D_GET_OBJECT();

    /* FIXME: compressed sub image has different requirement base on different compressed internal fomrat */

    /* Check arguments */

    if (imageSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, 0, 0, width, 1, 1))
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[face][lod];
    if((GLint)format != mipmap->requestedFormat)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    if (imageSize != __glCompressedTexImageSize(lod, format, width, 1, 1))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (width * 1 == 0)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.compressedTexSubImage1D)(gc, tex, lod, xoffset, width, imageSize, data))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GLAPIENTRY __glim_GetCompressedTexImage(__GLcontext *gc, GLenum target
, GLint level, GLvoid *img)
{
    __GLtextureObject *tex;
    GLuint activeUnit;
    GLuint face;
    __GLmipMapLevel *mipmap;

    __GL_HEADER();

    /* Get the texture object and face. */

    activeUnit = gc->state.texture.activeTexIndex;
    switch (target)
    {
    case GL_TEXTURE_1D:
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_INDEX];
        break;
    case GL_TEXTURE_2D:
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_1D_ARRAY_EXT:
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_1D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_RECTANGLE_ARB:
        if (level != 0)
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
            return;
        }
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_RECTANGLE_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP:
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        /* Check if this cubemap is cube complete */
        if (!__glIsCubeBaselevelConsistent(gc, tex))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;
    case GL_TEXTURE_3D:
        face = 0;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        face = 0;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            face = 0;
            break;
        }
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (level < 0 || level >= (GLint)gc->constants.maxNumTextureLevels )
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
        return;
    }

    mipmap = &tex->faceMipmap[face][level];

    /* Invalid to get a non-compressed image. */
    if (!mipmap->compressed)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if( (mipmap->width * mipmap->height ) == 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (img)
    {
        gc->dp.getCompressedTexImage(gc, tex, mipmap, level, img);
    }
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexImage2DMultisample(__GLcontext *gc,
                                     GLenum target,
                                     GLsizei samples,
                                     GLenum internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLboolean fixedsamplelocations)
{
    __GLtextureObject *tex;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D_MULTISAMPLE:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_MS_INDEX];
        tex->arrays = 1;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexMultisampleArgs(gc, tex, 1, internalformat, width, height, 1, samples))
    {
        __GL_EXIT();
    }

    tex->samples = samples;
    tex->fixedSampleLocations = fixedsamplelocations;
    tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, internalformat,
                               GL_NONE, GL_NONE, width, height, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage2D)(gc, tex, 0, 0, NULL))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, 0, 0);


    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexImage3DMultisample(__GLcontext *gc,
                                     GLenum target,
                                     GLsizei samples,
                                     GLenum internalformat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLsizei depth,
                                     GLboolean fixedsamplelocations)
{
    __GLtextureObject *tex;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_MS_ARRAY_INDEX];
        tex->arrays = depth;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexMultisampleArgs(gc, tex, 1, internalformat, width, height, depth, samples))
    {
        __GL_EXIT();
    }

    tex->samples = samples;
    tex->fixedSampleLocations = fixedsamplelocations;
    tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, internalformat,
                               GL_NONE, GL_NONE, width, height, depth) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage3D)(gc, tex, 0, NULL))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, 0, 0);


    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}
#endif

/*****************************************************************************
*
*  glTexSubImagexD
*
*****************************************************************************/
GLvoid GL_APIENTRY __glim_TexSubImage3D(__GLcontext *gc,
                                        GLenum target,
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
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_3D:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            break;
        }
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, 0, lod, xoffset, yoffset, zoffset, width, height, depth))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[0][lod].requestedFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, height, depth, format, type, buf))
        {
            __GL_EXIT();
        }
    }

    if (width * height * depth == 0)
    {
        __GL_EXIT();
    }

    tex->faceMipmap[0][lod].format = format;
    tex->faceMipmap[0][lod].type   = type;

    if (!(*gc->dp.texSubImage3D)(gc, tex, lod, xoffset, yoffset, zoffset, width, height, depth, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

OnExit:
    __GL_FOOTER();
    return;
}

GLvoid GL_APIENTRY __glim_TexSubImage2D(__GLcontext *gc,
                                        GLenum target,
                                        GLint lod,
                                        GLint xoffset,
                                        GLint yoffset,
                                        GLsizei width,
                                        GLsizei height,
                                        GLenum format,
                                        GLenum type,
                                        const GLvoid *buf)
{
    GLint face = 0;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXSUBIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, yoffset, 0, width, height, 1))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[face][lod].requestedFormat, format, type))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (!__glCheckPBO(gc, &gc->clientState.pixel.unpackModes, unpackBufObj, width, height, 0, format, type, buf))
        {
            __GL_EXIT();
        }
    }

    if (width * height == 0)
    {
        __GL_EXIT();
    }

    tex->faceMipmap[face][lod].format = format;
    tex->faceMipmap[face][lod].type   = type;

    if (!(*gc->dp.texSubImage2D)(gc, tex, face, lod, xoffset, yoffset, width, height, buf))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);
    tex->seqNumber++;

OnExit:
    __GL_FOOTER();
}

/*****************************************************************************
*
*  glCopyTexImagexD
*
*****************************************************************************/

GLvoid GL_APIENTRY __glim_CopyTexImage2D(__GLcontext *gc,
                                         GLenum target,
                                         GLint lod,
                                         GLenum internalFormat,
                                         GLint x,
                                         GLint y,
                                         GLsizei width,
                                         GLsizei height,
                                         GLint border)
{
    GLint face;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLboolean retVal;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexCopyImgFmt(gc, tex, internalFormat, GL_TRUE))
    {
        __GL_EXIT();
    }
    if (!__glCheckTexImgArgs(gc, tex, lod, width, height, 1, border))
    {
        __GL_EXIT();
    }

    __glSetMipmapBorder(gc, tex, face, lod, border);

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalFormat,
                               0, 0, width, height, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (__glCopyTexBegin(gc))
    {
        __glCopyTexValidateState(gc);

        retVal = (*gc->dp.copyTexImage2D)(gc, tex, face, lod, x, y);

        __glCopyTexEnd(gc, tex, lod);

        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }

        /* Set dirty for the texture attached FBOs */
        __glSetTexAttachedFboDirty(gc, tex, lod, face);

        tex->seqNumber++;
    }

OnExit:
    __GL_FOOTER();
}

/*****************************************************************************
*
*  glCopyTexSubImagexD
*
*****************************************************************************/
GLvoid GL_APIENTRY __glim_CopyTexSubImage3D(__GLcontext *gc,
                                            GLenum target,
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
    GLboolean retVal;
    GLuint activeUnit = gc->state.texture.activeTexIndex;

    __GL_HEADER();

    switch(target)
    {
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_3D:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
        break;
    default:
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, 0, lod, xoffset, yoffset, zoffset, width, height, 1))
    {
        __GL_EXIT();
    }
    if (!__glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[0][lod].requestedFormat, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (width * height == 0)
    {
        __GL_EXIT();
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (__glCopyTexBegin(gc))
    {
        __glCopyTexValidateState(gc);

        retVal =  (*gc->dp.copyTexSubImage3D)(gc, tex, lod, x, y, width, height, xoffset, yoffset, zoffset);

        __glCopyTexEnd(gc, tex, lod);

        if(!retVal)
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnExit:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_CopyTexSubImage2D(__GLcontext *gc,
                                            GLenum target,
                                            GLint lod,
                                            GLint xoffset,
                                            GLint yoffset,
                                            GLint x,
                                            GLint y,
                                            GLsizei width,
                                            GLsizei height)
{
    GLint face;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLboolean retVal;

    __GL_HEADER();

    /*get the texture object and face*/
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, yoffset, 0, width, height, 1))
    {
        __GL_EXIT();
    }
    if (!__glCheckTexCopyImgFmt(gc, tex, tex->faceMipmap[face][lod].requestedFormat, GL_FALSE))
    {
        __GL_EXIT();
    }

    if (width * height == 0)
    {
        __GL_EXIT();
    }

    __glEvaluateDrawableChange(gc, __GL_BUFFER_READ_BIT);

    if (__glCopyTexBegin(gc))
    {
        __glCopyTexValidateState(gc);

        retVal = (*gc->dp.copyTexSubImage2D)(gc, tex, face, lod, x, y, width, height, xoffset, yoffset);

        __glCopyTexEnd(gc, tex, lod);

        if (retVal)
        {
            tex->seqNumber++;
        }
        else
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }
    }

OnExit:
    __GL_FOOTER();
}

/*
*  All pixel storage and pixel transfer modes are ignored when decoding
*  a compressed texture image.
*/
GLvoid GL_APIENTRY __glim_CompressedTexImage3D(__GLcontext *gc,
                                               GLenum target,
                                               GLint lod,
                                               GLenum internalFormat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLsizei depth,
                                               GLint border,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex = NULL;
    __GLmipMapLevel *mipmap = NULL;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    switch (target)
    {
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        tex->arrays = depth;
        break;

    case GL_TEXTURE_3D:
        break;

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            tex->arrays = depth;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (imageSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    /* Check arguments */
    if (!__glCheckCompressedTexImgFmt(gc, internalFormat))
    {
        __GL_EXIT();
    }

    /* Now internalFormat is valid, only allows 2D_ARRAY target currently. */
    if (target != GL_TEXTURE_2D_ARRAY && target != GL_TEXTURE_CUBE_MAP_ARRAY_EXT)
    {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

    if (!__glCheckTexImgArgs(gc, tex, lod, width, height, depth, border))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    __glSetMipmapBorder(gc, tex, 0, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, lod, internalFormat,
                               0, 0, width, height, depth) == GL_FALSE)
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[0][lod];

    /* We may set mipmap->compressed to false because choose a uncompressed device format */
    if (mipmap->compressed && imageSize != (mipmap->compressedSize * tex->arrays))
    {
        __glSetMipmapBorder(gc, tex, 0, lod, border);
        /* Clear level info to make texture incomplete */
        __glSetMipmapLevelInfo(gc, tex, 0, lod, internalFormat, 0, 0, 0, 0, 0);
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!(*gc->dp.compressedTexImage3D)(gc, tex, lod, data))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, lod, 0);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_CompressedTexImage2D(__GLcontext *gc,
                                               GLenum target,
                                               GLint lod,
                                               GLenum internalFormat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLint border,
                                               GLsizei imageSize,
                                               const GLvoid *data)
{
    GLint face;
    GLint level;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    GLboolean isPalette = GL_FALSE;
    __GLtextureObject *tex;
    __GLmipMapLevel *mipmap;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    if (imageSize < 0)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    /* Check for paletted texture */
    switch (internalFormat)
    {
    case GL_PALETTE4_RGBA4_OES:
    case GL_PALETTE4_RGB5_A1_OES:
    case GL_PALETTE4_R5_G6_B5_OES:
    case GL_PALETTE4_RGB8_OES:
    case GL_PALETTE4_RGBA8_OES:
    case GL_PALETTE8_RGBA4_OES:
    case GL_PALETTE8_RGB5_A1_OES:
    case GL_PALETTE8_R5_G6_B5_OES:
    case GL_PALETTE8_RGB8_OES:
    case GL_PALETTE8_RGBA8_OES:
        if (lod > 0)
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        lod = -lod;
        /* If number of lods exceeded the size allowed one */
        if (lod > (GLint)__glFloorLog2(__GL_MAX(width, height)))
        {
            __GL_ERROR_RET(GL_INVALID_VALUE);
        }
        level = 0;
        isPalette = GL_TRUE;
        break;

    default:
        if (!__glCheckCompressedTexImgFmt(gc, internalFormat))
        {
            __GL_EXIT();
        }
        level = lod;
        break;
    }

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, level, width, height, 1, border))
    {
        __GL_EXIT();
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if(unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }
    __glSetMipmapBorder(gc, tex, face, lod, border);
    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalFormat,
                               0, 0, width, height, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[face][level];
    if (imageSize != mipmap->compressedSize)
    {
        __glSetMipmapBorder(gc, tex, 0, lod, border);
        /* Clear level info to make texture incomplete */
        __glSetMipmapLevelInfo(gc, tex, 0, lod, internalFormat, 0, 0, 0, 0, 0);
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!(*gc->dp.compressedTexImage2D)(gc, tex, face, lod, data))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    __glSetTexAttachedFboDirty(gc, tex, isPalette?-1:lod, face);

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_CompressedTexSubImage3D(__GLcontext *gc,
                                                  GLenum target,
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
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLmipMapLevel *mipmap;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    switch (target)
    {
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        break;
    case GL_TEXTURE_3D:
        switch (format)
        {
        case GL_COMPRESSED_R11_EAC:
        case GL_COMPRESSED_SIGNED_R11_EAC:
        case GL_COMPRESSED_RG11_EAC:
        case GL_COMPRESSED_SIGNED_RG11_EAC:
        case GL_COMPRESSED_RGB8_ETC2:
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
            break;

        default:
            break;
        }
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        tex = gc->texture.units[gc->state.texture.activeTexIndex].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
        break;
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    /* Check arguments */
    if (!__glCheckTexSubImgArgs(gc, tex, 0, lod, xoffset, yoffset, zoffset, width, height, depth))
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[0][lod];
    if ((GLint)format != mipmap->requestedFormat)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (imageSize != (__glCompressedTexImageSize(lod, format, width, height, depth)))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (width * height * depth == 0)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.compressedTexSubImage3D)(gc, tex, lod, xoffset, yoffset, zoffset, width, height, depth, data, imageSize))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_CompressedTexSubImage2D(__GLcontext *gc,
                                                  GLenum target,
                                                  GLint lod,
                                                  GLint xoffset,
                                                  GLint yoffset,
                                                  GLsizei width,
                                                  GLsizei height,
                                                  GLenum format,
                                                  GLsizei imageSize,
                                                  const GLvoid *data)
{
    GLint face;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;
    __GLmipMapLevel *mipmap;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXSUBIMAGE2D_GET_OBJECT();

    /* FIXME: compressed sub image has different requirement base on different compressed internal fomrat */

    /* Check arguments */

    if (imageSize < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!__glCheckTexSubImgArgs(gc, tex, face, lod, xoffset, yoffset, 0, width, height, 1))
    {
        __GL_EXIT();
    }

    mipmap = &tex->faceMipmap[face][lod];
    if((GLint)format != mipmap->requestedFormat)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    /* The image is from unpack buffer object? */
    if (unpackBufObj)
    {
        if (unpackBufObj->bufferMapped)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }

        if ((__GL_PTR2INT(data) + imageSize) > unpackBufObj->size)
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
    }

    if (imageSize != __glCompressedTexImageSize(lod, format, width, height, 1))
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (width * height == 0)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.compressedTexSubImage2D)(gc, tex, face, lod, xoffset, yoffset, width, height, data, imageSize))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    mipHintDirty = __glCheckMipHintDirty(gc, tex, lod);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_GenerateMipmap(__GLcontext *gc, GLenum target)
{
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLtextureObject *tex;
    __GLmipMapLevel *baseMipmap;
    GLint width, height, depth, maxSize;
    GLint baseLevel, maxLevel;
    GLint lod, faces, arrays;
    GLint border = 0;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D:
        faces = 1;
        arrays = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_CUBE_MAP:
        faces = 6;
        arrays = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        /* Check if this cubemap is cube complete */
        if (!__glIsCubeBaselevelConsistent(gc, tex))
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;
    case GL_TEXTURE_3D:
        faces = 1;
        arrays = 1;
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
        break;
    case GL_TEXTURE_2D_ARRAY:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
        faces = 1;
        arrays = tex->faceMipmap[0][tex->params.baseLevel].arrays;
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
        {
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
            faces = 1;
            arrays = tex->faceMipmap[0][tex->params.baseLevel].arrays;
            break;
        }
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    baseLevel = tex->params.baseLevel;
    baseMipmap = &tex->faceMipmap[0][baseLevel];

    switch (baseMipmap->requestedFormat)
    {
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_RGB5_A1:
    case GL_RGBA4:
    case GL_RGBA:
    case GL_RGB:
    case GL_RGB565:
        break;
    default:
        if (!baseMipmap->formatInfo->filterable ||
            !baseMipmap->formatInfo->renderable ||
            /* Only color-renderable format are allowed. */
            baseMipmap->formatInfo->baseFormat == GL_DEPTH_COMPONENT ||
            baseMipmap->formatInfo->baseFormat == GL_DEPTH_STENCIL ||
            baseMipmap->formatInfo->baseFormat == GL_STENCIL
           )
        {
            __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        }
        break;
    }

    width  = baseMipmap->width;
    height = baseMipmap->height;
    depth  = baseMipmap->depth;
    border = baseMipmap->border;

    maxSize = __GL_MAX(__GL_MAX(width, height), depth);

    maxLevel = __glFloorLog2(maxSize) + baseLevel;
    maxLevel = __GL_MIN(maxLevel, tex->params.maxLevel);
    if (maxLevel == baseLevel)
    {
        __GL_EXIT();
    }

    for (lod = baseLevel + 1; lod <= maxLevel; ++lod)
    {
        GLint face;

        __GL_HALVE_SIZE(width);
        __GL_HALVE_SIZE(height);
        __GL_HALVE_SIZE(depth);

        depth = __GL_IS_TEXTURE_ARRAY(tex->targetIndex) ? arrays : depth;

        for (face = 0; face < faces; ++face)
        {
            __glSetMipmapBorder(gc, tex, face, lod, border);
            __glSetMipmapLevelInfo(gc, tex, face, lod, baseMipmap->requestedFormat,
                                   baseMipmap->format, baseMipmap->type, width, height, depth);
        }
    }

    if (!(*gc->dp.generateMipmaps)(gc, tex, faces, &maxLevel))
    {
        tex->params.mipHint = __GL_TEX_MIP_HINT_FORCE_OFF;
        __GL_SET_TEX_UNIT_BIT(gc, gc->state.texture.activeTexIndex, __GL_TEXPARAM_MAX_LEVEL_BIT);
        __GL_ERROR((*gc->dp.getError)(gc));
    }
    else
    {
        if (tex->fboList == gcvNULL)
        {
            tex->params.mipHint = __GL_TEX_MIP_HINT_FORCE_ON;
        }
        else
        {
            tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO_MIP;
        }
    }
    tex->mipBaseLevel = baseLevel;
    tex->mipMaxLevel  = maxLevel;

    /* Set dirty for the texture attached FBOs */
    __glSetTexAttachedFboDirty(gc, tex, -1, -1);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | __GL_TEXPARAM_MIP_HINT_BIT);

OnExit:
OnError:
    __GL_FOOTER();
}

GLboolean __glCheckTexStorageArgs(__GLcontext *gc,
                                  __GLtextureObject *tex,
                                  GLsizei levels,
                                  GLenum internalformat,
                                  GLsizei width,
                                  GLsizei height,
                                  GLsizei depth,
                                  GLsizei samples)
{
    GLsizei maxSize = __GL_MAX(width, height);

    if (!tex)
    {
        return GL_FALSE;
    }

    /* If default texture, or immutable */
    if (0 == tex->name || tex->immutable)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    if (levels < 1 || width < 1 || height < 1 || depth < 1)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    if (__GL_IS_TEXTURE_CUBE(tex->targetIndex))
    {
        if (width != height)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }

    if (__GL_IS_2D_TEXTURE_ARRAY(tex->targetIndex))
    {
        if (depth > (GLsizei)gc->constants.maxTextureArraySize)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
    else if (__GL_TEXTURE_CUBEMAP_ARRAY_INDEX == tex->targetIndex)
    {
        if (depth > ((GLsizei)gc->constants.maxTextureArraySize * 6) || depth % 6 != 0)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
    }
    else
    {
        maxSize = __GL_MAX(maxSize, depth);
    }

    if (maxSize > (GLsizei)gc->constants.maxTextureSize)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    if (levels > (GLint)__glFloorLog2(maxSize) + 1)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    switch (internalformat)
    {
    /* RGBA NORM */
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_BGRA_EXT:
    case GL_SRGB8_ALPHA8:
    case GL_RGB10_A2:
    case GL_RGBA16F:
    case GL_RGBA32F:
    /* RGB NORM */
    case GL_RGB565:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_SRGB8:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB16F:
    case GL_RGB32F:
    /* RG NORM */
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    /* R NORM */
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16F:
    case GL_R32F:
    /* RGBA INTEGER */
    case GL_RGBA8UI:
    case GL_RGBA8I:
    case GL_RGBA16UI:
    case GL_RGBA16I:
    case GL_RGBA32UI:
    case GL_RGBA32I:
    case GL_RGB10_A2UI:
    /* RGB INTEGER */
    case GL_RGB8UI:
    case GL_RGB8I:
    case GL_RGB16UI:
    case GL_RGB16I:
    case GL_RGB32UI:
    case GL_RGB32I:
    /* RG INTEGER */
    case GL_RG8UI:
    case GL_RG8I:
    case GL_RG16UI:
    case GL_RG16I:
    case GL_RG32UI:
    case GL_RG32I:
    /* R INTEGER */
    case GL_R8UI:
    case GL_R8I:
    case GL_R16UI:
    case GL_R16I:
    case GL_R32UI:
    case GL_R32I:
        break;
    /* compressed */
    case GL_COMPRESSED_R11_EAC:
    case GL_COMPRESSED_SIGNED_R11_EAC:
    case GL_COMPRESSED_RG11_EAC:
    case GL_ETC1_RGB8_OES:
    case GL_COMPRESSED_SIGNED_RG11_EAC:
    case GL_COMPRESSED_RGB8_ETC2:
    case GL_COMPRESSED_SRGB8_ETC2:
    case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
    case GL_COMPRESSED_RGBA8_ETC2_EAC:
    case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        if (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
    case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
    case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
    case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:

    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
    case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
        if (!__glExtension[__GL_EXTID_KHR_texture_compression_astc_ldr].bEnabled)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }

        if (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        if (!__glExtension[__GL_EXTID_EXT_texture_compression_s3tc].bEnabled)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }

        if (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    /* DEPTH STENCIL */
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32F:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
    case GL_STENCIL_INDEX8:
        if (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
        break;

    /* unsized internal format */
    case GL_RGBA:
    case GL_RGB:
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    default:
    /*according to spec, an INVALID_ENUM error is generated if internalformat is one of the unsized base internal formats*/
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    if ((tex->targetIndex == __GL_TEXTURE_2D_MS_INDEX) ||
        (tex->targetIndex == __GL_TEXTURE_2D_MS_ARRAY_INDEX))
    {
        __GLformatInfo *formatInfo = __glGetFormatInfo(internalformat);
        GLint formatSamples = 0;

        (*gc->dp.queryFormatInfo)(gc, formatInfo->drvFormat, gcvNULL, &formatSamples, 1);

        /* Must be renderable formats */
        if (!formatInfo->renderable)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }

        if (samples == 0)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        if (samples > formatSamples)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }
    }

    /* Always a canonical format. */
    tex->canonicalFormat = GL_TRUE;

    return GL_TRUE;
}

GLvoid GL_APIENTRY __glim_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels,
                                       GLenum internalformat, GLsizei width, GLsizei height)
{
    GLsizei w, h;
    GLint face, lod;
    GLuint mipHintDirty = 0;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLtextureObject *tex;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        tex->arrays = 1;
        break;
    case GL_TEXTURE_CUBE_MAP:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
        tex->arrays = 6;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexStorageArgs(gc, tex, levels, internalformat, width, height, 1, 0))
    {
        __GL_EXIT();
    }

    tex->immutable = GL_TRUE;
    tex->immutableLevels = levels;

    for (face = 0; face < tex->arrays; ++face)
    {
        w = width;
        h = height;
        for (lod = 0; lod < levels; ++lod)
        {
            /* Init the mipmap info which will be queried by app */
            if (__glSetMipmapLevelInfo(gc, tex, face, lod, internalformat,
                                       GL_NONE, GL_NONE, w, h, 1) == GL_FALSE)
            {
                __GL_EXIT();
            }

            /* The dp function should also handle compressed format */
            if (!(*gc->dp.texImage2D)(gc, tex, face, lod, NULL))
            {
                __GL_ERROR((*gc->dp.getError)(gc));
            }

            mipHintDirty |= __glCheckMipHintDirty(gc, tex, lod);

            w = __GL_MAX(1, w/2);
            h = __GL_MAX(1, h/2);
        }

        /* Set the other level to default value */
        for (; lod < (GLint)gc->constants.maxNumTextureLevels; ++lod)
        {
            (*gc->dp.freeTexImage)(gc, tex, face, lod);
            __glClearMipmapLevelInfo(gc, tex, face, lod);
        }
    }

    __glSetTexAttachedFboDirty(gc, tex, -1, -1);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexStorage3D(__GLcontext *gc,
                                       GLenum target,
                                       GLsizei levels,
                                       GLenum internalformat,
                                       GLsizei width,
                                       GLsizei height,
                                       GLsizei depth)
{
    GLint lod;
    GLuint activeUnit;
    GLuint mipHintDirty = 0;
    __GLtextureObject *tex;

    __GL_HEADER();

    __GL_TEXIMAGE3D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexStorageArgs(gc, tex, levels, internalformat, width, height, depth, 0))
    {
        __GL_EXIT();
    }

    tex->immutable = GL_TRUE;
    tex->immutableLevels = levels;

    /* No need to go over all arrays(faces in CUBE) like TexStorage2D does, because all the arrays of
    ** 2D array textures will be specified together, while each face of CUBE texture was specified separately.
    */
    for (lod = 0; lod < levels; ++lod)
    {
        /* Init the mipmap info which will be queried by app */
        if (__glSetMipmapLevelInfo(gc, tex, 0, lod, internalformat,
                                   GL_NONE, GL_NONE, width, height, depth) == GL_FALSE)
        {
            __GL_EXIT();
        }

        if (!(*gc->dp.texImage3D)(gc, tex, lod, NULL))
        {
            __GL_ERROR((*gc->dp.getError)(gc));
        }

        mipHintDirty |= __glCheckMipHintDirty(gc, tex, lod);

        width  = __GL_MAX(1, width/2);
        height = __GL_MAX(1, height/2);
        if (GL_TEXTURE_3D == target)
        {
            depth = __GL_MAX(1, depth/2);
        }
    }

    /* Set the other level to default value */
    for (; lod < (GLint)gc->constants.maxNumTextureLevels; ++lod)
    {
        (*gc->dp.freeTexImage)(gc, tex, 0, lod);
        __glClearMipmapLevelInfo(gc, tex, 0, lod);
    }

    __glSetTexAttachedFboDirty(gc, tex, -1, 0);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT | mipHintDirty);

    tex->seqNumber++;

OnExit:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples,
                                                  GLenum internalformat, GLsizei width, GLsizei height,
                                                  GLboolean fixedsamplelocations)
{
    GLint lod;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLtextureObject *tex;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D_MULTISAMPLE:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_MS_INDEX];
        tex->arrays = 1;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexStorageArgs(gc, tex, 1, internalformat, width, height, 1, samples))
    {
        __GL_EXIT();
    }

    /* Mark it not as direct texture any more */
    tex->immutable = GL_TRUE;
    tex->immutableLevels = 1;
    tex->samples = samples;
    tex->fixedSampleLocations = fixedsamplelocations;
    tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, internalformat,
                               GL_NONE, GL_NONE, width, height, 1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    /* The dp function should also handle compressed format */
    if (!(*gc->dp.texImage2D)(gc, tex, 0, 0, NULL))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    /* Set the other level to default value */
    for (lod = 1; lod < (GLint)gc->constants.maxNumTextureLevels; ++lod)
    {
        (*gc->dp.freeTexImage)(gc, tex, 0, lod);
        __glClearMipmapLevelInfo(gc, tex, 0, lod);
    }

    __glSetTexAttachedFboDirty(gc, tex, -1, -1);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}


GLvoid GL_APIENTRY __glim_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat,
                                                     GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    GLint lod;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLtextureObject *tex;

    __GL_HEADER();

    switch (target)
    {
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_MS_ARRAY_INDEX];
        tex->arrays = depth;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check arguments */
    if (!__glCheckTexStorageArgs(gc, tex, 1, sizedinternalformat, width, height, depth, samples))
    {
        __GL_EXIT();
    }

    tex->immutable = GL_TRUE;
    tex->immutableLevels = 1;
    tex->samples = samples;
    tex->fixedSampleLocations = fixedsamplelocations;
    tex->params.mipHint = __GL_TEX_MIP_HINT_AUTO;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, sizedinternalformat,
                               GL_NONE, GL_NONE, width, height, depth) == GL_FALSE)
    {
        __GL_EXIT();
    }

    if (!(*gc->dp.texImage3D)(gc, tex, 0, NULL))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    /* Set the other level to default value */
    for (lod = 1; lod < (GLint)gc->constants.maxNumTextureLevels; ++lod)
    {
        (*gc->dp.freeTexImage)(gc, tex, 0, lod);
        __glClearMipmapLevelInfo(gc, tex, 0, lod);
    }

    __glSetTexAttachedFboDirty(gc, tex, -1, 0);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}


#if defined(GL_OES_EGL_image)

GLboolean __glGetAttribFromImage(__GLcontext *gc,
                                 khrEGL_IMAGE_PTR image,
                                 GLint *internalFormat,
                                 GLint *format,
                                 GLint *type,
                                 GLint *width,
                                 GLint *height)
{
    /* Get texture attributes. */
    switch (image->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
    case KHR_IMAGE_RENDER_BUFFER:
        if (internalFormat)
        {
            *internalFormat = image->u.texture.internalFormat;
        }

        if (format)
        {
            *format = image->u.texture.format;
        }

        if (type)
        {
            *type = image->u.texture.type;
        }

        if (width)
        {
            *width = image->u.texture.width;
        }

        if (height)
        {
            *height = image->u.texture.height;
        }
        break;

    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
    case KHR_IMAGE_WAYLAND_BUFFER:
    case KHR_IMAGE_LINUX_DMA_BUF:
        if (!(*gc->dp.getTextureAttribFromImage)(gc, image, width, height, gcvNULL, gcvNULL,
                                                 format, internalFormat, type, gcvNULL, gcvNULL, gcvNULL))
        {
            __GL_ERROR((*gc->dp.getError)(gc));
            return GL_FALSE;
        }
        break;

    case KHR_IMAGE_PIXMAP:
        if (!(*gc->dp.getTextureAttribFromImage)(gc, image, gcvNULL, gcvNULL, gcvNULL, gcvNULL,
                                                 format, internalFormat, type, gcvNULL, gcvNULL, gcvNULL))
        {
            __GL_ERROR((*gc->dp.getError)(gc));
            return GL_FALSE;
        }
        *width = image->u.pixmap.width;
        *height = image->u.pixmap.height;
        break;

    case KHR_IMAGE_VIV_DEC:
        if ((*gc->dp.getTextureAttribFromImage)(gc, image, gcvNULL, gcvNULL, gcvNULL, gcvNULL,
                                                 format, internalFormat, type, gcvNULL, gcvNULL, gcvNULL))
        {
            return GL_FALSE;
        }
        *width = image->u.decimage.width;
        *height = image->u.decimage.height;
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    return GL_TRUE;
}

GLboolean __glCheckEglImageTargetArg(__GLcontext *gc, GLenum target, khrEGL_IMAGE *eglImage)
{
    /* Test if image is valid */
    if ((eglImage == NULL)
    ||  (eglImage->magic != KHR_EGL_IMAGE_MAGIC_NUM)
    )
    {
        __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
    }

    /* Test image content. */
    if (eglImage->surface == NULL)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }

    return GL_TRUE;
}


GLvoid GL_APIENTRY __glim_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    __GLtextureObject *texObj;
    GLuint activeUnit;
    GLint width = 0, height = 0;
    GLint internalFormat, format;
    GLint type;
    khrEGL_IMAGE * eglImage = (khrEGL_IMAGE_PTR)image;

    __GL_HEADER();

    activeUnit = gc->state.texture.activeTexIndex;

    if (!__glCheckEglImageTargetArg(gc, target, eglImage))
    {
        __GL_EXIT();
    }

    /* Get the texture object and face */
    switch (target)
    {
    case GL_TEXTURE_2D:
        texObj = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
        break;
    case GL_TEXTURE_EXTERNAL_OES:
        texObj = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_EXTERNAL_INDEX];
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    texObj->arrays = 1;

    /* Get texture attributes from eglImage. */
    if (!__glGetAttribFromImage(gc, eglImage, &internalFormat, &format, &type, &width, &height))
    {
        __GL_EXIT();
    }

    /* Check corresponding texture arguments. */
    switch (eglImage->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
        if (!__glCheckTexImgArgs(gc, texObj, eglImage->u.texture.level, width, height, 1, 0))
        {
            __GL_EXIT();
        }
        if (!__glCheckTexImgFmt(gc, texObj, target, internalFormat, format, type))
        {
            __GL_EXIT();
        }

        break;

    case KHR_IMAGE_RENDER_BUFFER:
        if (!__glCheckTexImgArgs(gc, texObj, eglImage->u.texture.level, width, height, 1, 0))
        {
            __GL_EXIT();
        }
        if (!__glCheckTexImgInternalFmtArg(gc, texObj, internalFormat))
        {
            __GL_EXIT();
        }
        break;

    case KHR_IMAGE_VG_IMAGE:
    case KHR_IMAGE_PIXMAP:
    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
    case KHR_IMAGE_WAYLAND_BUFFER:
    case KHR_IMAGE_VIV_DEC:
    case KHR_IMAGE_LINUX_DMA_BUF:
        if (!__glCheckTexImgArgs(gc, texObj, 0, width, height, 1, 0))
        {
            __GL_EXIT();
        }
        if (!__glCheckTexImgFmt(gc, texObj, target, internalFormat, format, type))
        {
            __GL_EXIT();
        }

        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }
    texObj->canonicalFormat = GL_TRUE;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc,
                               texObj,
                               0,
                               0,
                               internalFormat,
                               format,
                               type,
                               width,
                               height,
                               1) == GL_FALSE)
    {
        __GL_EXIT();
    }

    /* When egl image's content changed, set the __GL_TEX_IMAGE_CONTENT_CHANGED_BIT dirty bit */
    __glSetTexImageDirtyBit(gc, texObj, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    if (!(*gc->dp.eglImageTargetTexture2DOES)(gc, texObj, target, eglImage))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    texObj->seqNumber++;

OnExit:
OnError:
    __GL_FOOTER();
}


GLboolean __glCheckEglImageTargetRbArg(__GLcontext *gc, GLenum target, khrEGL_IMAGE *eglImage)
{
    if (target != GL_RENDERBUFFER)
    {
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    if ((eglImage == NULL) ||
        (eglImage->magic != KHR_EGL_IMAGE_MAGIC_NUM))
    {
        __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
    }
    return GL_TRUE;
}

GLvoid GL_APIENTRY __glim_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    khrEGL_IMAGE *eglImage = (khrEGL_IMAGE_PTR)(image);

    __GL_HEADER();

    if (!__glCheckEglImageTargetRbArg(gc, target, eglImage))
    {
        __GL_EXIT();
    }

    __eglImageTargetRenderbufferStorageOES(gc, target, eglImage);

OnExit:
    __GL_FOOTER();
}

#endif


#if GL_VIV_direct_texture

GLvoid GL_APIENTRY __glim_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width,
                                       GLsizei height, GLenum format, GLvoid ** pixels)
{
    GLint face;
    GLuint activeUnit;
    __GLtextureObject *tex;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, 0, width, height, 1, 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check target and format. The format actually represents both external format and internal format*/
    if ( !__glCheckTexDirectFmt(gc, tex, target, format))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, 0, GL_RGBA8,
                               format, GL_NONE, width, height, 1) == GL_FALSE)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!(*gc->dp.texDirectVIV)(gc, tex, width, height, format, pixels))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

    tex->params.mipHint = __GL_TEX_MIP_HINT_FORCE_OFF;
    tex->mipBaseLevel = tex->mipMaxLevel = tex->params.baseLevel;

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT |
                                     __GL_TEXPARAM_MAX_LEVEL_BIT |
                                     __GL_TEXPARAM_MIP_HINT_BIT);

    tex->seqNumber++;

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target)
{
    GLint face = 0;
    GLuint activeUnit;
    __GLtextureObject *tex;
    GLsizei height = 1;

    __GL_HEADER();

    if (target != GL_TEXTURE_2D)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* A texture has to be bound. */
    if (tex == gcvNULL || face >= 6)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    if (!(*gc->dp.texDirectInvalidateVIV)(gc, tex))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glTexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height,
                              GLenum format, GLvoid ** logical, const GLuint * physical, GLboolean tiled)
{
    __GLtextureObject *tex;
    GLint face;
    GLuint activeUnit;

    __GL_HEADER();

    /* Get the texture object and face */
    __GL_TEXIMAGE2D_GET_OBJECT();

    /* Check arguments */
    if (!__glCheckTexImgArgs(gc, tex, 0, width, height, 1, 0))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Check target and format. The format actually represents both external format and internal format*/
    if ( !__glCheckTexDirectFmt(gc, tex, target, format))
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Validate the logical address */
    if ( (*logical) == gcvNULL || ((gctUINTPTR_T)(*logical) & 0x3F) != 0 )
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, face, 0, format,
                               format, GL_NONE, width, height, 1) == GL_FALSE)
    {
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (!(*gc->dp.texDirectVIVMap)(gc, tex, target, width, height, format, logical, physical, tiled))
    {
        __GL_ERROR_EXIT((*gc->dp.getError)(gc));
    }

    tex->params.mipHint = __GL_TEX_MIP_HINT_FORCE_OFF;
    tex->mipBaseLevel = tex->mipMaxLevel = tex->params.baseLevel;

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT |
                                     __GL_TEXPARAM_MAX_LEVEL_BIT |
                                     __GL_TEXPARAM_MIP_HINT_BIT);

    tex->seqNumber++;

OnError:
    __GL_FOOTER();
}

GLvoid GL_APIENTRY __glim_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height,
                                          GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __glTexDirectVIVMap(gc, target, width, height, format, logical, physical, GL_FALSE);
}

GLvoid GL_APIENTRY __glim_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height,
                                               GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __glTexDirectVIVMap(gc, target, width, height, format, logical, physical, GL_TRUE);
}

#endif

GLboolean __glGetTBOFmt(__GLcontext *gc, GLenum internalformat, GLenum *type, GLenum *format, GLuint *bppPerTexel)
{
    switch (internalformat)
    {
    case GL_R8:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 1;
            break;
        }
    case GL_R8UI:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RED;
            *bppPerTexel = 1;
            break;
        }
    case GL_RG8:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RG;
            *bppPerTexel = 2;
            break;
        }
    case GL_RG8UI:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 2;
            break;
        }
    case GL_RGBA8:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RGBA;
            *bppPerTexel = 4;
            break;
        }
    case GL_RGBA8UI:
        {
            *type = GL_UNSIGNED_BYTE;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 4;
            break;
        }

    case GL_R8I:
        {
            *type = GL_BYTE;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 1;
            break;
        }
    case GL_RG8I:
        {
            *type = GL_BYTE;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 2;
            break;
        }
    case GL_RGBA8I:
        {
            *type = GL_BYTE;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_R16I:
        {
            *type = GL_SHORT;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 2;
            break;
        }
    case GL_RG16I:
        {
            *type = GL_SHORT;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_RGBA16I:
        {
            *type = GL_SHORT;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 8;
            break;
        }
    case GL_R16UI:
        {
            *type = GL_UNSIGNED_SHORT;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 2;
            break;
        }
    case GL_RG16UI:
        {
            *type = GL_UNSIGNED_SHORT;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_RGBA16UI:
        {
            *type = GL_UNSIGNED_SHORT;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 8;
            break;
        }
    case GL_R16F:
        {
            *type = GL_HALF_FLOAT;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 2;
            break;
        }
    case GL_RG16F:
        {
            *type = GL_HALF_FLOAT;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_RGBA16F:
        {
            *type = GL_HALF_FLOAT;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 8;
            break;
        }
    case GL_R32I:
        {
            *type = GL_INT;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_RG32I:
        {
            *type = GL_INT;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 8;
            break;
        }
    case GL_RGB32I:
        {
            *type = GL_INT;
            *format = GL_RGB_INTEGER;
            *bppPerTexel = 12;
            break;
        }
    case GL_RGBA32I:
        {
            *type = GL_INT;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 16;
            break;
        }
    case GL_R32UI:
        {
            *type = GL_UNSIGNED_INT;
            *format = GL_RED_INTEGER;
            *bppPerTexel = 4;
            break;
        }
    case GL_RG32UI:
        {
            *type = GL_UNSIGNED_INT;
            *format = GL_RG_INTEGER;
            *bppPerTexel = 8;
            break;
        }
    case GL_RGB32UI:
        {
            *type = GL_UNSIGNED_INT;
            *format = GL_RGB_INTEGER;
            *bppPerTexel = 12;
            break;
        }
    case GL_RGBA32UI:
        {
            *type = GL_UNSIGNED_INT;
            *format = GL_RGBA_INTEGER;
            *bppPerTexel = 16;
            break;
        }
    case GL_R32F:
        {
            *type = GL_FLOAT;
            *format = GL_RED;
            *bppPerTexel = 4;
            break;
        }
    case GL_RG32F:
        {
            *type = GL_FLOAT;
            *format = GL_RG;
            *bppPerTexel = 8;
            break;
        }
    case GL_RGB32F:
        {
            *type = GL_FLOAT;
            *format = GL_RGB;
            *bppPerTexel = 12;
            break;
        }
    case GL_RGBA32F:
        {
            *type = GL_FLOAT;
            *format = GL_RGBA;
            *bppPerTexel = 16;
            break;
        }

    default:
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }
        break;
    }

    return GL_TRUE;
}

GLvoid __glUnBindTextureBuffer(__GLcontext *gc, __GLtextureObject *tex, __GLbufferObject *bufObj)
{
    if (tex->bufObj && tex->bufObj == bufObj)
    {
        __glRemoveImageUser(gc, &bufObj->texList, tex);

        if (!bufObj->bindCount && !bufObj->vaoList &&
            !bufObj->texList && (bufObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteBufferObject(gc, bufObj);
        }
    }

    tex->bufObj  = gcvNULL;
    tex->bufOffset = 0;
    tex->bufSize = 0;

    return;
}

GLvoid GL_APIENTRY __glim_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer)
{
    __GLbufferObject *bufObj;
    __GLtextureObject *tex;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    GLenum type;
    GLint size = 0;
    GLint offset = 0;
    GLenum format = 0;
    GLuint bppPerTexel = 0;
    GLuint texelSize = 0;

    __GL_HEADER();

    if (target != GL_TEXTURE_BUFFER_EXT)
    {
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    if (!__glGetTBOFmt(gc, internalformat, &type, &format, &bppPerTexel))
    {
        __GL_EXIT();
    }

    tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_BINDING_BUFFER_EXT];

    if(buffer == 0)
    {
        __glUnBindTextureBuffer(gc, tex, tex->bufObj);
        __GL_EXIT();
    }

    bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, buffer);

    if ((buffer != 0) && (!bufObj))
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    size = (GLint)bufObj->size;

    /* Already bound with same setting*/
    if((bufObj == tex->bufObj) &&
        (size == tex->bufSize) &&
        (offset == tex->bufOffset)
        )
    {
        return;
    }

    tex->arrays = 1;

    texelSize = size / bppPerTexel;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, internalformat,
        format, type, texelSize, 1, 1) == GL_FALSE)
    {
        return;
    }

    tex->bufObj = bufObj;
    tex->bufOffset = 0;
    tex->bufSize = 0;
    tex->bppPerTexel = bppPerTexel;

    /* Add into list */
    __glAddImageUser(gc, &bufObj->texList, tex);

    __glSetTexAttachedFboDirty(gc, tex, 0, 0);

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

    tex->seqNumber++;

OnExit:
     __GL_FOOTER();
     return;
}
GLvoid GL_APIENTRY __glim_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                            GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GLbufferObject *bufObj;
    __GLtextureObject *tex;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    GLenum type;
    GLenum format = 0;
    GLuint texelSize = 0;
    GLuint bppPerTexel = 0;
    __GL_HEADER();

    if (target != GL_TEXTURE_BUFFER_EXT)
    {
        __GL_ERROR_RET(GL_INVALID_ENUM);
    }

    if (!__glGetTBOFmt(gc, internalformat, &type, &format, &bppPerTexel))
    {
        __GL_EXIT();
    }

    tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_BINDING_BUFFER_EXT];

    if(buffer == 0)
    {
        __glUnBindTextureBuffer(gc, tex, tex->bufObj);
        __GL_EXIT();
    }

    bufObj = (__GLbufferObject*)__glGetObject(gc, gc->bufferObject.shared, buffer);

    if ((buffer != 0) && (!bufObj))
    {
        __GL_ERROR_RET(GL_INVALID_OPERATION);
    }

    if ((offset < 0) || (size <= 0) || ((offset + size) > bufObj->size))
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    if ((offset % gc->constants.textureBufferOffsetAlignment) != 0)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    /* Already bound with same setting*/
    if((bufObj == tex->bufObj) &&
        (size == tex->bufSize) &&
        (offset == tex->bufOffset)
        )
    {
        return;
    }

    tex->arrays = 1;

    texelSize = (GLint)size / bppPerTexel;

    /* Init the mipmap info which will be queried by app */
    if (__glSetMipmapLevelInfo(gc, tex, 0, 0, internalformat,
        format, type, texelSize, 1, 1) == GL_FALSE)
    {
        return;
    }

    tex->bufObj = bufObj;
    tex->bufOffset = (GLint)offset;
    tex->bufSize = (GLint)size;
    tex->bppPerTexel = bppPerTexel;

    /* Add into list */
    __glAddImageUser(gc, &bufObj->texList, tex);

    __glSetTexAttachedFboDirty(gc, tex, 0, 0);

    tex->seqNumber++;

    __glSetTexImageDirtyBit(gc, tex, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);

OnExit:
     __GL_FOOTER();
     return;
}

GLboolean __glCheckCopyImageSubDataArg(__GLcontext *gc, GLuint name, GLenum target, GLint level, GLint x, GLint y, GLint z,
                                       GLsizei width, GLsizei height, GLsizei depth, __GLformatInfo ** formatInfo,
                                       GLvoid ** object, GLuint *targetIndex, GLint *samples)
{
    __GLtextureObject * tex = NULL;
    __GLrenderbufferObject * rbo = NULL;
    GLint maxLevelUsed = 0;
    GLboolean isTex = GL_FALSE;

    switch (target)
    {
    case GL_RENDERBUFFER:
        rbo = (__GLrenderbufferObject *)__glGetObject(gc, gc->frameBuffer.rboShared, name);

        if (rbo == gcvNULL ||
            level != 0)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        *targetIndex = target;
        break;

    case GL_TEXTURE_2D:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_2D_INDEX;
        break;

    case GL_TEXTURE_3D:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_3D_INDEX;
        break;

    case GL_TEXTURE_CUBE_MAP:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_CUBEMAP_INDEX;
        break;

    case GL_TEXTURE_2D_ARRAY:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_2D_ARRAY_INDEX;
        break;

    case GL_TEXTURE_2D_MULTISAMPLE:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_2D_MS_INDEX;
        break;

    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
        break;

    case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
        isTex = GL_TRUE;
        *targetIndex = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
        break;

    default:
        __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
    }

    if (isTex)
    {
        __GLmipMapLevel *mipmap = NULL;
        GLint mipmapDepth;
        __GLsamplerParamState *samplerParam;

        tex =  (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, name);

        if (!tex)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        if (tex->targetIndex != *targetIndex)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_ENUM, GL_FALSE);
        }

        maxLevelUsed = __glCalcTexMaxLevelUsed(gc, tex, tex->params.sampler.minFilter);
        samplerParam = &tex->params.sampler;

        if (!__glIsTextureComplete(gc, tex, samplerParam->minFilter, samplerParam->magFilter,
                                   samplerParam->compareMode, maxLevelUsed))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
        }

        mipmap = &tex->faceMipmap[0][level];

        if (__GL_IS_TEXTURE_ARRAY(tex->targetIndex))
        {
            mipmapDepth = mipmap->arrays;
        }
        else if(__GL_IS_TEXTURE_CUBE(tex->targetIndex))
        {
            mipmapDepth = 6;
        }
        else
        {
            mipmapDepth = mipmap->depth;
        }

        if (x < 0 || (x + width)  > mipmap->width ||
            y < 0 || (y + height) > mipmap->height ||
            z < 0 || (z + depth)  > mipmapDepth)
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        if (mipmap->requestedFormat >= GL_COMPRESSED_R11_EAC &&
            mipmap->requestedFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)
        {
            if ((width  % 4) || (x % 4) || (height % 4) || (y % 4))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }
        }

        *formatInfo = mipmap->formatInfo;
        *object     = tex;
        *samples    = tex->samples;
    }
    else if (rbo)
    {
        if ((rbo->width < x + width) || (rbo->height < y + height))
        {
            __GL_ERROR_RET_VAL(GL_INVALID_VALUE, GL_FALSE);
        }

        *formatInfo = rbo->formatInfo;
        *object     = rbo;
        *samples    = rbo->samples;
    }

    return GL_TRUE;
}


/*For the purposes of CopyImageSubData, two internal formats
  are considered compatible if any of the following conditions are met:
 * the formats are the same,
 * the formats are both listed in the same entry of Table 4.X.2, or
 * one format is compressed and the other is uncompressed and
   Table 4.X.1 lists the two formats in the same row.
*/
GLboolean __glIsCopyImageSubDataCompatible(__GLcontext *gc, __GLformatInfo * srcFormatInfo, __GLformatInfo * dstFormatInfo)
{
    /*1. the formats are the same */
    if (srcFormatInfo == dstFormatInfo &&
        srcFormatInfo != &__glFormatInfoTable[__GL_FMT_MAX])
    {
        return GL_TRUE;
    }

    /*2. the formats are both listed in the same entry of Table 4.X.2 */
    if ((!srcFormatInfo->compressed && !dstFormatInfo->compressed) &&
        (srcFormatInfo->bitsPerPixel == dstFormatInfo->bitsPerPixel))
    {
        return GL_TRUE;
    }

    if (srcFormatInfo->compressed && dstFormatInfo->compressed )
    {
    }

    /*3. one format is compressed and the other is uncompressed and
      Table 4.X.1 lists the two formats in the same row.*/
    if ((srcFormatInfo->compressed && !dstFormatInfo->compressed) ||
        (!srcFormatInfo->compressed && dstFormatInfo->compressed))
    {
        if (srcFormatInfo->bitsPerPixel == dstFormatInfo->bitsPerPixel)
        {
            return GL_TRUE;
        }
    }

    __GL_ERROR_RET_VAL(GL_INVALID_OPERATION, GL_FALSE);
}

GLvoid GL_APIENTRY __glim_CopyImageSubData(__GLcontext *gc,
                                           GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                           GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                           GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
     __GLformatInfo *srcFormatInfo = NULL;
     __GLformatInfo *dstFormatInfo = NULL;
     GLvoid * srcObject = NULL;
     GLvoid * dstObject = NULL;
     GLuint srcTargetIndex = 0;
     GLuint dstTargetIndex = 0;
     GLint srcSamples = 0;
     GLint dstSamples = 0;

     __GL_HEADER();

     if (!__glCheckCopyImageSubDataArg(gc, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, srcWidth, srcHeight, srcDepth,
                                       &srcFormatInfo, &srcObject, &srcTargetIndex, &srcSamples))
     {
         __GL_EXIT();
     }

     if (!__glCheckCopyImageSubDataArg(gc, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth,
                                       &dstFormatInfo, &dstObject, &dstTargetIndex, &dstSamples))
     {
         __GL_EXIT();
     }

     if (!__glIsCopyImageSubDataCompatible(gc,srcFormatInfo, dstFormatInfo))
     {
         __GL_EXIT();
     }

     if (srcSamples != dstSamples)
     {
         __GL_ERROR_EXIT(GL_INVALID_OPERATION);
     }

    if (!(*gc->dp.copyImageSubData)(gc,srcObject, srcTargetIndex, srcLevel, srcX, srcY, srcZ,
                                       dstObject, dstTargetIndex, dstLevel, dstX, dstY, dstZ,
                                       srcWidth,srcHeight,srcDepth))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

    /* dstObject is texture */
    if (dstTargetIndex != GL_RENDERBUFFER)
    {
        ((__GLtextureObject *)dstObject)->seqNumber++;
    }

OnExit:
OnError:
    __GL_FOOTER();
}

GLvoid __glUnBindImageTexture(__GLcontext *gc, GLuint unit, __GLtextureObject *texObj)
{
    __GLimageUnitState *imageUnit = &gc->state.image.imageUnit[unit];

    if (imageUnit->texObj && imageUnit->texObj == texObj)
    {
        __glRemoveImageUser(gc, &texObj->imageList, (GLvoid*)(GLintptr)unit);

        if (!texObj->bindCount && !texObj->fboList && !texObj->imageList && (texObj->flag & __GL_OBJECT_IS_DELETED))
        {
            __glDeleteTextureObject(gc, texObj);
        }
    }

    imageUnit->texObj  = gcvNULL;
    imageUnit->level   = 0;
    imageUnit->layered = GL_FALSE;
    imageUnit->requestLayer = 0;
    imageUnit->access  = GL_READ_ONLY;
    imageUnit->format  = GL_R32UI;

    imageUnit->invalid = GL_TRUE;
    return;
}

GLvoid GL_APIENTRY __glim_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level,
                                           GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GLtextureObject *texObj;
    __GLimageUnitState *imageUnit;
    __GLmipMapLevel *texLevel;
    GLuint imageElemSize = 0;
    GLuint texelSize = 0;
    GLboolean singleLayered = GL_FALSE;
    GLint actualLayer = 0;
    GLuint type;
    __GLmipMapLevel *mipmap;

    __GL_HEADER();

    if (unit >= gc->constants.shaderCaps.maxImageUnit)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    imageUnit = &gc->state.image.imageUnit[unit];

    if (texture == 0)
    {
        __glUnBindImageTexture(gc, unit, imageUnit->texObj);
        __GL_EXIT();
    }

    if (level < 0 || layer < 0)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    texObj = (__GLtextureObject*)__glGetObject(gc, gc->texture.shared, texture);

    if (NULL == texObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
    }

    if (!texObj->immutable && !texObj->bufObj)
    {
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
    }

    switch (format)
    {
    case GL_RGBA32F:
    case GL_RGBA32UI:
    case GL_RGBA32I:
        imageElemSize = 128;
        break;
    case GL_RGBA16F:
    case GL_RGBA16UI:
    case GL_RGBA16I:
        imageElemSize = 64;
        break;
    case GL_R32F:
    case GL_R32UI:
    case GL_R32I:
    case GL_RGBA8:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA8_SNORM:
        imageElemSize = 32;
        break;
    default:
        __GL_ERROR_EXIT(GL_INVALID_VALUE);
        break;
    }

    switch (texObj->targetIndex)
    {
    case  __GL_TEXTURE_2D_INDEX:
        type = __GL_IMAGE_2D;
        singleLayered = GL_TRUE;
        actualLayer = 0;
        break;
    case  __GL_TEXTURE_3D_INDEX:
        type = __GL_IMAGE_3D;
        singleLayered = !layered;
        actualLayer = layered ? 0 : layer;
        break;
    case  __GL_TEXTURE_CUBEMAP_INDEX:
        type = __GL_IMAGE_CUBE;
        singleLayered = !layered;
        actualLayer = layered ? 0 : layer;
        break;
    case  __GL_TEXTURE_2D_ARRAY_INDEX:
        type = __GL_IMAGE_2D_ARRAY;
        singleLayered = !layered;
        actualLayer = layered ? 0 : layer;
        break;
    case  __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
        type = __GL_IMAGE_CUBE_ARRAY;
        singleLayered = !layered;
        actualLayer = layered ? 0 : layer;
        break;
    case __GL_TEXTURE_BINDING_BUFFER_EXT:
        type = __GL_IMAGE_BUFFER;
        singleLayered = GL_TRUE;
        actualLayer = 0;
        break;

    default:
        /* Spec didn't say other target is legal or not */
        __GL_ERROR_EXIT(GL_INVALID_OPERATION);
        break;
    }

    imageUnit->requestLayer = layer;

    /* Already bound with same setting*/
    if ((texObj == imageUnit->texObj) &&
        (layered == imageUnit->layered) &&
        (actualLayer == imageUnit->actualLayer) &&
        (format == imageUnit->format) &&
        (access == imageUnit->access) &&
        (level == imageUnit->level))
    {
        __GL_EXIT();
    }

    imageUnit->texObj = texObj;
    imageUnit->layered = layered;
    imageUnit->actualLayer = actualLayer;
    imageUnit->format = format;
    imageUnit->formatInfo = __glGetFormatInfo(format);
    imageUnit->level = level;
    imageUnit->access = access;

    __GL_SET_IMG_UNIT_BIT(gc, unit);

    /* Add into list */
    __glAddImageUser(gc, &texObj->imageList, (GLvoid*)(GLintptr)unit);

    texLevel = &texObj->faceMipmap[0][0];

    switch (texLevel->requestedFormat)
    {
    case GL_RGBA32F:
    case GL_RGBA32UI:
    case GL_RGBA32I:
        texelSize = 128;
        break;
    case GL_RGBA16F:
    case GL_RGBA16UI:
    case GL_RGBA16I:
        texelSize = 64;
        break;
    case GL_R32F:
    case GL_R32UI:
    case GL_R32I:
    case GL_RGBA8:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA8_SNORM:
        texelSize = 32;
        break;
    default:
        __GL_EXIT();
    }

    if (texelSize != imageElemSize)
    {
        __GL_EXIT();
    }

    mipmap = &texObj->faceMipmap[0][level];

    /* This unit has valid texture image */
    imageUnit->invalid = GL_FALSE;
    imageUnit->singleLayered = singleLayered;
    imageUnit->type = type;
    imageUnit->width = mipmap->width;
    imageUnit->height = mipmap->height;
    imageUnit->depth = __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ?
                           ((texObj->targetIndex == __GL_TEXTURE_CUBEMAP_ARRAY_INDEX) ?
                               mipmap->arrays / 6 : mipmap->arrays)
                           : mipmap->depth;


OnError:
OnExit:
     __GL_FOOTER();
     return;

}

GLvoid __glSetFBOAttachedTexDirty(__GLcontext *gc, GLbitfield mask, GLint drawbuffer)
{
    GLboolean depthChecked = GL_FALSE;
    __GLtextureObject *texObj = gcvNULL;
    __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;

    GL_ASSERT(gc->frameBuffer.drawFramebufObj->name);

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        if (drawbuffer == -1)
        {
            GLuint i;
            for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
            {
                GLint attachIndex = __glMapAttachmentToIndex(fbo->drawBuffers[i]);
                __GLfboAttachPoint *attachPoint = gcvNULL;

                if (-1 == attachIndex)
                {
                    continue;
                }

                attachPoint = &fbo->attachPoint[attachIndex];

                if (attachPoint->objType == GL_TEXTURE)
                {
                    texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                    if (texObj)
                    {
                        __glSetTexImageDirtyBit(gc, texObj, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
                    }
                }
            }
        }
        else
        {
            GLint attachIndex = __glMapAttachmentToIndex(fbo->drawBuffers[drawbuffer]);

            if (attachIndex != -1)
            {
                __GLfboAttachPoint *attachPoint = &fbo->attachPoint[attachIndex];

                if (attachPoint->objType == GL_TEXTURE)
                {
                    texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
                    if (texObj)
                    {
                        __glSetTexImageDirtyBit(gc, texObj, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
                    }
                }
            }
        }
    }

    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        __GLfboAttachPoint *attachPoint = &fbo->attachPoint[__GL_DEPTH_ATTACHMENT_POINT_INDEX];
        if (attachPoint->objType == GL_TEXTURE)
        {
            texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
            if (texObj)
            {
                __glSetTexImageDirtyBit(gc, texObj, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
            }
        }
        depthChecked = GL_TRUE;
    }

    if (!depthChecked && (mask & GL_STENCIL_BUFFER_BIT))
    {
        __GLfboAttachPoint *attachPoint = &fbo->attachPoint[__GL_STENCIL_ATTACHMENT_POINT_INDEX];
        if (attachPoint->objType == GL_TEXTURE)
        {
            texObj = (__GLtextureObject *)__glGetObject(gc, gc->texture.shared, attachPoint->objName);
            if (texObj)
            {
                __glSetTexImageDirtyBit(gc, texObj, __GL_TEX_IMAGE_CONTENT_CHANGED_BIT);
            }
        }
    }
}

GLvoid GL_APIENTRY __glim_GetTexImage(__GLcontext *gc, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    GLint face;
    GLuint activeUnit = gc->state.texture.activeTexIndex;
    __GLtextureObject *tex;
    GLint maxLod = (GLint)(gc->constants.maxNumTextureLevels - 1);
    __GLbufferObject *packBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufObj;

    __GL_HEADER();

    switch (target)
    {
        case GL_TEXTURE_2D:
            face = 0;
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_INDEX];
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_INDEX];
            break;
        case GL_TEXTURE_2D_ARRAY:
            face = 0;
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_2D_ARRAY_INDEX];
            break;
        case GL_TEXTURE_3D:
            face = 0;
            tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_3D_INDEX];
            break;
        case GL_TEXTURE_CUBE_MAP_ARRAY_EXT:
            if (__glExtension[__GL_EXTID_EXT_texture_cube_map_array].bEnabled)
            {
                face = 0;
                tex = gc->texture.units[activeUnit].boundTextures[__GL_TEXTURE_CUBEMAP_ARRAY_INDEX];
                break;
            }
        default:
            __GL_ERROR_RET(GL_INVALID_ENUM);
        }

    if (!tex)
    {
        __GL_EXIT();
    }

    /* Check lod, width, height */
    if (level < 0 || level > maxLod)
    {
        __GL_ERROR_RET(GL_INVALID_VALUE);
    }

    if (!__glCheckTexImgTypeArg(gc, tex, type))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmtArg(gc, tex, format))
    {
        __GL_EXIT();
    }

    if (!__glCheckTexImgFmt(gc, tex, target, tex->faceMipmap[face][level].requestedFormat, format, type))
    {
        __GL_EXIT();
    }
    /* The image is from unpack buffer object? */
    if (packBufObj)
    {
        __GLmipMapLevel *mipmap = &tex->faceMipmap[face][level];

        if (!__glCheckPBO(gc, &gc->clientState.pixel.packModes, packBufObj, mipmap->width, mipmap->height, mipmap->depth, format, type, pixels))
        {
            __GL_EXIT();
        }
    }

    if (!(*gc->dp.getTexImage)(gc, tex, face, level, type, pixels))
    {
        __GL_ERROR((*gc->dp.getError)(gc));
    }

OnExit:
    __GL_FOOTER();
}

