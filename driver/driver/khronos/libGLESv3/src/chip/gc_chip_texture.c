/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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
#include "gc_chip_misc.h"

#if defined(ANDROID)
#if ANDROID_SDK_VERSION >= 16
#      include <ui/ANativeObjectBase.h>
#   else
#      include <private/ui/android_natives_priv.h>
#   endif

#if gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
#      include <gc_gralloc_priv.h>
#   endif
#endif


#define _GC_OBJ_ZONE    __GLES3_ZONE_TEXTURE

gceTEXTURE_TYPE __glChipTexTargetToHAL[] =
{
    gcvTEXTURE_2D,            /* __GL_TEXTURE_2D_INDEX */
    gcvTEXTURE_3D,            /* __GL_TEXTURE_3D_INDEX */
    gcvTEXTURE_CUBEMAP,       /* __GL_TEXTURE_CUBEMAP_INDEX */
    gcvTEXTURE_2D_ARRAY,      /* __GL_TEXTURE_2D_ARRAY_INDEX */
    gcvTEXTURE_EXTERNAL,      /* __GL_TEXTURE_EXTERNAL_INDEX */
    gcvTEXTURE_2D_MS,         /* __GL_TEXTURE_2D_MS_INDEX */
    gcvTEXTURE_2D_MS_ARRAY,   /* __GL_TEXTURE_2D_MS_ARRAY_INDEX */
    gcvTEXTURE_CUBEMAP_ARRAY, /* __GL_TEXTURE_CUBEMAP_ARRAY_INDEX */
    gcvTEXTURE_UNKNOWN,       /* __GL_MAX_TEXTURE_BINDINGS */
};

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/
static gceENDIAN_HINT
gcChipUtilGetEndianHint(
    GLenum internalFormat,
    GLenum Type
    )
{
    gcmHEADER_ARG("internalFormat=0x%04x Type=0x%04x", internalFormat, Type);
    /* Dispatch on the type. */
    switch (Type)
    {
    case GL_UNSIGNED_BYTE:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;

    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_SWAP_WORD);
        return gcvENDIAN_SWAP_WORD;

    default:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;
    }
}


GLvoid
gcChipUtilGetImageFormat(
    GLenum format,
    GLenum type,
    gceSURF_FORMAT *pImageFormat,
    gctSIZE_T *pBpp
    )
{
    gceSURF_FORMAT imageFormat = gcvSURF_UNKNOWN;
    gctSIZE_T bpp = 0;

    gcmHEADER_ARG("format=0x%04x type=0x%04x pImageFormat=0x%x pBpp",
                   format, type, pImageFormat, pBpp);

    switch (format)
    {
    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_D16;
            break;

        case GL_DEPTH_COMPONENT24_OES:
            /* Some applications (Taiji) use DEPTH_COMPONENT24_OES, even though it's not part of the spec. */
            bpp = 32;
            imageFormat = gcvSURF_D24X8;
            break;

        case GL_DEPTH_COMPONENT32_OES:
            /* Some applications use DEPTH_COMPONENT32_OES, even though it's not part of the spec. */
        case GL_UNSIGNED_INT:
            bpp = 32;
            imageFormat = gcvSURF_D32;
            break;

        case GL_FLOAT:
            bpp = 32;
            imageFormat = gcvSURF_D32F;
            break;
        }
        break;

    case GL_ALPHA:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_A8;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_A16;
            break;

        case GL_UNSIGNED_INT:
            bpp = 32;
            imageFormat = gcvSURF_A32;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 16;
            imageFormat = gcvSURF_A16F;
            break;

        case GL_FLOAT:
            bpp = 32;
            imageFormat = gcvSURF_A32F;
            break;
        }
        break;

    case GL_RGB:
        switch (type)
        {
        case GL_UNSIGNED_SHORT_4_4_4_4:
            bpp = 16;
            imageFormat = gcvSURF_X4R4G4B4;
            break;

        case GL_UNSIGNED_SHORT_5_5_5_1:
            bpp = 16;
            imageFormat = gcvSURF_X1R5G5B5;
            break;

        case GL_UNSIGNED_SHORT_5_6_5:
            bpp = 16;
            imageFormat = gcvSURF_R5G6B5;
            break;

        case GL_UNSIGNED_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_B8G8R8;
            break;

        case GL_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_B8G8R8_SNORM;
            break;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bpp = 32;
            imageFormat = gcvSURF_X2B10G10R10;
            break;

        case GL_UNSIGNED_INT_10F_11F_11F_REV:
            bpp = 32;
            imageFormat = gcvSURF_B10G11R11F;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 48;
            imageFormat = gcvSURF_B16G16R16F;
            break;

        case GL_FLOAT:
            bpp = 96;
            imageFormat = gcvSURF_B32G32R32F;
             break;

        case GL_UNSIGNED_SHORT:
            bpp = 48;
            imageFormat = gcvSURF_B16G16R16;
            break;

        case GL_UNSIGNED_INT_5_9_9_9_REV:
            bpp = 32;
            imageFormat = gcvSURF_E5B9G9R9;
            break;
        }
        break;

    case GL_RGB_INTEGER:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_B8G8R8UI;
            break;

        case GL_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_B8G8R8I;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 48;
            imageFormat = gcvSURF_B16G16R16UI;
            break;

        case GL_SHORT:
            bpp = 48;
            imageFormat = gcvSURF_B16G16R16I;
            break;

        case GL_UNSIGNED_INT:
            bpp = 96;
            imageFormat = gcvSURF_B32G32R32UI;
            break;

        case GL_INT:
            bpp = 96;
            imageFormat = gcvSURF_B32G32R32I;
            break;
         }
         break;

    case GL_RGBA:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            bpp = 64;
            imageFormat = gcvSURF_A16B16G16R16;
            break;

        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8B8G8R8;
            break;

        case GL_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8B8G8R8_SNORM;
            break;

        case GL_UNSIGNED_SHORT_4_4_4_4:
            bpp = 16;
            imageFormat = gcvSURF_R4G4B4A4;
            break;

        case GL_UNSIGNED_SHORT_5_5_5_1:
            bpp = 16;
            imageFormat = gcvSURF_R5G5B5A1;
            break;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bpp = 32;
            imageFormat = gcvSURF_A2B10G10R10;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 64;
            imageFormat = gcvSURF_A16B16G16R16F;
            break;

        case GL_FLOAT:
            bpp = 128;
            imageFormat = gcvSURF_A32B32G32R32F;
            break;
        }
        break;

    case GL_RGBA_INTEGER:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8B8G8R8UI;
            break;

        case GL_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8B8G8R8I;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 64;
            imageFormat = gcvSURF_A16B16G16R16UI;
            break;

        case GL_SHORT:
            bpp = 64;
            imageFormat = gcvSURF_A16B16G16R16I;
            break;

        case GL_UNSIGNED_INT:
            bpp = 128;
            imageFormat = gcvSURF_A32B32G32R32UI;
            break;

        case GL_INT:
            bpp = 128;
            imageFormat = gcvSURF_A32B32G32R32I;
            break;

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bpp = 32;
            imageFormat = gcvSURF_A2B10G10R10UI;
            break;
         }
         break;

    case GL_BGRA_EXT:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8R8G8B8;
            break;
        }
        break;

    case GL_RED:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_R8;
            break;

        case GL_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_R8_SNORM;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 16;
            imageFormat = gcvSURF_R16F;
            break;

        case GL_FLOAT:
            bpp = 32;
            imageFormat = gcvSURF_R32F;
            break;

        default:
            break;
        }
        break;

    case GL_RED_INTEGER:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_R8UI;
            break;

        case GL_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_R8I;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_R16UI;
            break;

        case GL_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_R16I;
            break;

        case GL_UNSIGNED_INT:
            bpp = 32;
            imageFormat = gcvSURF_R32UI;
            break;

        case GL_INT:
            bpp = 32;
            imageFormat = gcvSURF_R32I;
            break;
        }
        break;

    case GL_RG:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 16;
            imageFormat = gcvSURF_G8R8;
            break;

        case GL_BYTE:
            bpp = 16;
            imageFormat = gcvSURF_G8R8_SNORM;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 32;
            imageFormat = gcvSURF_G16R16F;
            break;

        case GL_FLOAT:
            bpp = 64;
            imageFormat = gcvSURF_G32R32F;
            break;

        default:
            break;
        }
        break;

    case GL_RG_INTEGER:
        switch(type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 16;
            imageFormat = gcvSURF_G8R8UI;
            break;

        case GL_BYTE:
            bpp = 16;
            imageFormat = gcvSURF_G8R8I;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 32;
            imageFormat = gcvSURF_G16R16UI;
            break;

        case GL_SHORT:
            bpp = 32;
            imageFormat = gcvSURF_G16R16I;
            break;

        case GL_UNSIGNED_INT:
            bpp = 64;
            imageFormat = gcvSURF_G32R32UI;
            break;

        case GL_INT:
            bpp = 64;
            imageFormat = gcvSURF_G32R32I;
            break;
        }
        break;

    case GL_LUMINANCE:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_L8;
            break;
        case GL_UNSIGNED_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_L16;
            break;

        case GL_UNSIGNED_INT:
            bpp = 32;
            imageFormat = gcvSURF_L32;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 16;
            imageFormat = gcvSURF_L16F;
            break;

        case GL_FLOAT:
            bpp = 32;
            imageFormat = gcvSURF_L32F;
            break;
        }
        break;

    case GL_LUMINANCE_ALPHA:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 16;
            imageFormat = gcvSURF_A8L8;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 32;
            imageFormat = gcvSURF_A16L16;
            break;

        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            bpp = 32;
            imageFormat = gcvSURF_A16L16F;
            break;

        case GL_FLOAT:
            bpp = 64;
            imageFormat = gcvSURF_A32L32F;
            break;
        }
        break;

    case GL_DEPTH_STENCIL:
        switch (type)
        {
        case GL_UNSIGNED_INT_24_8:
            bpp = 32;
            imageFormat = gcvSURF_D24S8;
            break;

        case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            bpp = 64;
            imageFormat = gcvSURF_S8D32F;
            break;
        }
        break;

    case GL_STENCIL_INDEX:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 8;
            imageFormat = gcvSURF_S8;
            break;
        }
        break;

    case GL_SRGB_EXT:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_SBGR8;
            break;
        }
        break;

    case GL_SRGB_ALPHA_EXT:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8_SBGR8;
            break;
        }
        break;
    }

    if (pImageFormat)
    {
        /* We should find a valid Format-Type combination */
        GL_ASSERT(imageFormat != gcvSURF_UNKNOWN);
        *pImageFormat = imageFormat;
    }

    if (pBpp)
    {
        *pBpp = bpp;
    }
    gcmFOOTER_NO();
}

/* Reset the direct texture  */
static gceSTATUS
gcChipResetTextureWrapper(
    __GLcontext* gc,
    __GLtextureObject *texObj
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    do
    {
        texInfo->direct.dirty   = gcvFALSE;
        texInfo->eglImage.dirty = gcvFALSE;

        /* Remove existing external source. */
        if (texInfo->eglImage.source)
        {
            /* Destroy the source surface. */
            gcmERR_BREAK(gcoSURF_Destroy(texInfo->eglImage.source));
            texInfo->eglImage.source = gcvNULL;
        }

        if (texInfo->eglImage.image)
        {
            /* Orphan the texture if it's EGLimage sibling */
            if (texInfo->object)
            {
                gcmERR_BREAK(gcoTEXTURE_Destroy(texInfo->object));
                texInfo->object = gcvNULL;
            }

            /* Dereference EGLImageKHR. */
            gc->imports.dereferenceImage(texInfo->eglImage.image);
            texInfo->eglImage.image = gcvNULL;
        }

#if defined(ANDROID) && (ANDROID_SDK_VERSION >= 7)
        if (texInfo->eglImage.nativeBuffer)
        {
            android_native_buffer_t * nativeBuffer;

            /* Cast to android native buffer. */
            nativeBuffer = (android_native_buffer_t *) texInfo->eglImage.nativeBuffer;

            /* Decrease native buffer reference count. */
            nativeBuffer->common.decRef(&nativeBuffer->common);
            texInfo->eglImage.nativeBuffer = gcvNULL;
        }
#endif

        /* Remove existing YUV direct structure. */
        if (texInfo->direct.source != gcvNULL)
        {
            /* Unlock the source surface. */
            gcmERR_BREAK(gcoSURF_Unlock(texInfo->direct.source, gcvNULL));

            /* Destroy the source surface. */
            gcmERR_BREAK(gcoSURF_Destroy(texInfo->direct.source));
            texInfo->direct.source = gcvNULL;
        }
    } while (gcvFALSE);

    /* Return status. */
    gcmFOOTER();
    return status;
}


static gceSTATUS
gcChipResidentTextureLevel(
    __GLcontext* gc,
    __GLchipContext *chipCtx,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    __GLmipMapLevel *mipmap = gcvNULL;
    __GLformat drvFormat;
    gctSIZE_T width = 0, height = 0;
    GLuint lods;
    GLboolean paletted;
    GLboolean needClean = GL_FALSE;
    const GLvoid *pixels = gcvNULL;
    gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                            ? gcvFACE_POSITIVE_X + face : gcvFACE_NONE;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x texObj=0x%x face=%d level=%d buf=0x%x",
                   gc, chipCtx, texObj, face, level, buf);

    if (!texInfo)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    mipmap = &texObj->faceMipmap[face][level];
    drvFormat = mipmap->formatInfo->drvFormat;

#if gcdUSE_NPOT_PATCH
    if ((mipmap->width & (mipmap->width - 1)) ||
        (mipmap->height & (mipmap->height - 1)))
    {
        texInfo->isNP2 = GL_TRUE;
    }
#endif

    gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

    /* Construct the gcoTEXTURE object. */
    if (texInfo->object == gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));
    }

    gcmONERROR(gcoTEXTURE_SetEndianHint(texInfo->object,
        gcChipUtilGetEndianHint(mipmap->requestedFormat, mipmap->type)));

    gcmONERROR(gcoTEXTURE_SetDepthTextureFlag(texInfo->object,texObj->unsizedTexture));


    paletted = ((drvFormat >= __GL_FMT_PALETTE4_RGBA4_OES) &&
                (drvFormat <= __GL_FMT_PALETTE8_RGBA8_OES))
             ? GL_TRUE : GL_FALSE;

    if (paletted)
    {
        width = (gctSIZE_T)texObj->faceMipmap[face][0].width;
        height = (gctSIZE_T)texObj->faceMipmap[face][0].height;
        lods = level + 1;
        level = 0;
    }
    else
    {
        lods = 1;
    }

    /* The image is from unpack buffer object */
    if (unpackBufObj)
    {
        gcmONERROR(gcChipProcessPBO(gc, unpackBufObj, &buf));
    }

    while (lods-- > 0)
    {
        gceSURF_FORMAT srcImageFormat = gcvSURF_UNKNOWN;
        __GLchipFmtMapInfo *formatMapInfo = gcvNULL;
        GLboolean renderMipmap = GL_FALSE;
        gceSURF_COLOR_SPACE srcColorSpace = gcvSURF_COLOR_SPACE_LINEAR;
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
        __GLchipFmtPatch patchCase = __GL_CHIP_FMT_PATCH_NONE;
        GLint numSlices = mipmap->arrays > 1 ? mipmap->arrays : mipmap->depth;
        gceSURF_FORMAT format;
        GLint i;

        mipmap = &texObj->faceMipmap[face][level];

        GL_ASSERT(mipmap->formatInfo->drvFormat < __GL_FMT_MAX);

        if ((mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8) &&
            (chipCtx->patchId == gcvPATCH_GTFES30) &&
            ((mipmap->width == 0x1f4 && mipmap->height == 0x1f4) ||
             (mipmap->width == 2 && mipmap->height == 2)
            )
           )
        {
            patchCase = __GL_CHIP_FMT_PATCH_CASE1;
        }
        else if ((mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8) &&
                 (chipCtx->patchId == gcvPATCH_GTFES30) &&
                 (texObj->samples > 1)
                )
        {
            patchCase = __GL_CHIP_FMT_PATCH_CASE4;
        }
        else if ((chipCtx->patchId == gcvPATCH_GTFES30) &&
                 (mipmap->formatInfo->dataType == GL_HALF_FLOAT) &&
                 (texObj->immutable && texObj->immutableLevels == 1))
        {
            patchCase = __GL_CHIP_FMT_PATCH_CASE3;
        }
        else if ((mipmap->formatInfo->drvFormat == __GL_FMT_Z16) &&
                 chipCtx->chipFeature.hwFeature.hasCompressionV1 &&
                 (chipCtx->chipFeature.hwFeature.hasTxTileStatus ||
                  chipCtx->chipFeature.hwFeature.hasTileFiller ||
                  (chipCtx->patchId == gcvPATCH_GTFES30) || gcdPROC_IS_WEBGL(chipCtx->patchId)
                 ) &&
                 (texObj->targetIndex != __GL_TEXTURE_2D_ARRAY_INDEX) &&
                 (texObj->targetIndex != __GL_TEXTURE_3D_INDEX)
                )
        {
            patchCase = __GL_CHIP_FMT_PATCH_CASE2;
        }
        else if (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_6 &&
                 texObj->samples > 0 &&
                 mipmap->formatInfo->bitsPerPixel == 8)
        {
            patchCase = __GL_CHIP_FMT_PATCH_8BIT_MSAA;
        }
        else if ((chipCtx->patchId == gcvPATCH_GTFES30) &&
                 (texObj->arrays == 4) &&
                 (mipmap->width != 0x40) &&
                 (mipmap->formatInfo->drvFormat == __GL_FMT_Z32F || mipmap->formatInfo->drvFormat == __GL_FMT_Z32FS8) &&
                 (buf == gcvNULL))
        {
            patchCase = __GL_CHIP_FMT_PATCH_D32F;
        }
        else if ((((mipmap->requestedFormat >= GL_COMPRESSED_RGBA_ASTC_4x4_KHR) &&
                   (mipmap->requestedFormat <= GL_COMPRESSED_RGBA_ASTC_12x12_KHR)
                  ) ||
                  ((mipmap->requestedFormat >= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR) &&
                   (mipmap->requestedFormat <= GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR)
                  )
                 ) &&
                 !chipCtx->chipFeature.hwFeature.hasTxASTCMultiSliceFix &&
                 (__GL_IS_TEXTURE_CUBE(texObj->targetIndex) || __GL_IS_TEXTURE_ARRAY(texObj->targetIndex)))
        {
            patchCase = __GL_CHIP_FMT_PATCH_ASTC;
        }
        else if ((mipmap->formatInfo->drvFormat == __GL_FMT_A8) &&
            (chipCtx->chipModel == gcv600 && chipCtx->chipRevision == 0x4652))
        {
            patchCase = __GL_CHIP_FMT_PATCH_ALPHA8;
        }
        else if ((texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX) ||
                 (texObj->targetIndex == __GL_TEXTURE_3D_INDEX))
        {
            patchCase = __GL_CHIP_FMT_PATCH_CASE0;
        }

        formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, patchCase);

        chipMipLevel->formatMapInfo = formatMapInfo;

        GL_ASSERT((formatMapInfo->flags & __GL_CHIP_FMTFLAGS_CANT_FOUND_HAL_FORMAT) == GL_FALSE);
        GL_ASSERT(formatMapInfo->readFormat != gcvSURF_UNKNOWN);

        /* For non-compress source image, now we can construct its HAL format for late surface blit.
        ** For compressed format, it will be determined later.
        ** Either srcImageFormat will be determined by decompressed data format,
        ** or it's just same as resident format.
        */
        if (!mipmap->compressed)
        {
            gcChipUtilGetImageFormat(mipmap->format, mipmap->type, &srcImageFormat, gcvNULL);
        }

        if (texObj->samples > 0)
        {
            for (i = 0; i < formatMapInfo->numSamples; ++i)
            {
                if (formatMapInfo->samples[i] >= texObj->samples)
                {
                    break;
                }
            }

            texObj->samplesUsed = formatMapInfo->samples[i];
        }
        else
        {
            texObj->samplesUsed = texObj->samples;
        }

        format = formatMapInfo->readFormat;
#if __GL_CHIP_PATCH_ENABLED
        if (chipCtx->patchInfo.commitTexDelete)
        {
            format = gcvSURF_R8;
        }
#endif

        chipCtx->needTexRecompile = chipCtx->needTexRecompile
                                  || gcChipCheckRecompileEnable(gc, format);

        chipCtx->needRTRecompile = chipCtx->needRTRecompile || chipCtx->needTexRecompile;

        /* Add the mipmap. If it already exists, the call will be ignored. */
        gcmONERROR(gcoTEXTURE_AddMipMapEx(texInfo->object,
                                          level,
                                          mipmap->requestedFormat,
                                          format,
                                          (gctSIZE_T)mipmap->width,
                                          (gctSIZE_T)mipmap->height,
                                          __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ?
                                                      (gctSIZE_T)texObj->arrays : (gctSIZE_T)mipmap->depth,
                                          __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? 1 : texObj->arrays,
                                          gcvPOOL_DEFAULT,
                                          texObj->samplesUsed,
                                          (gctBOOL)texObj->params.contentProtected,
                                          gcvNULL));

        if (chipCtx->needStencilOpt)
        {
            if (mipmap->formatInfo->stencilSize > 0)
            {
                if (!chipMipLevel->stencilOpt)
                {
                    /* For cubemap, all 6 slice will be allocated once,
                    ** but only 1 face will be touched by the call.
                    */
                    GLint totalSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX)
                                      ? mipmap->depth
                                      : texObj->arrays;

                    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                              totalSlices * gcmSIZEOF(__GLchipStencilOpt),
                                              (gctPOINTER*)&chipMipLevel->stencilOpt));
                    gcoOS_ZeroMemory(chipMipLevel->stencilOpt, totalSlices * gcmSIZEOF(__GLchipStencilOpt));
                }

                for (i = 0; i < numSlices; ++i)
                {
                    gcChipPatchStencilOptReset(&chipMipLevel->stencilOpt[face + i],
                                               (gctSIZE_T)mipmap->width,
                                               (gctSIZE_T)mipmap->height,
                                               (gctSIZE_T)mipmap->formatInfo->stencilSize);
                }
            }
            else
            {
                if (chipMipLevel->stencilOpt)
                {
                    gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)chipMipLevel->stencilOpt));
                    chipMipLevel->stencilOpt = gcvNULL;
                }
            }
        }

        if (texObj->fboList)
        {
            GLuint attachIdx;
            __GLimageUser *iu = texObj->fboList;
            __GLframebufferObject *fbo = gcvNULL;

            /* Go through all attached fbos. */
            while (iu)
            {
                fbo = (__GLframebufferObject*)iu->imageUser;
                /* Texture can only be bound to user created FBO */
                if (fbo && fbo->name)
                {
                    /* Go through all attach points, not only the ones selected by drawBuffers.
                    ** Because we do not resize indirect RT surfaces when change drawBuffers.
                    */
                    for (attachIdx = 0; attachIdx < __GL_MAX_ATTACHMENTS; ++attachIdx)
                    {
                        __GLfboAttachPoint* attachPoint = &fbo->attachPoint[attachIdx];
                        /* If the attachment sourced from this face level */
                        if (attachPoint->objType == GL_TEXTURE &&
                            attachPoint->objName == texObj->name &&
                            attachPoint->level == level &&
                            attachPoint->face == face)
                        {
                            if (gcChipTexNeedShadow(gc,
                                                    texObj,
                                                    texInfo,
                                                    chipMipLevel->formatMapInfo,
                                                    attachPoint->samples,
                                                    &attachPoint->samplesUsed)
                               )
                            {
                                gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, level, attachPoint->slice);
                                gcmONERROR(gcChipRellocShadowResource(gc,
                                                                      texView.surf,
                                                                      attachPoint->samplesUsed,
                                                                      &chipMipLevel->shadow[attachPoint->slice],
                                                                      chipMipLevel->formatMapInfo,
                                                                      GL_TRUE));
                            }
                            else
                            {
                                __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

                                if(shadow && shadow->surface)
                                {
                                    gcoSURF_Destroy(shadow->surface);
                                    shadow->surface = gcvNULL;
                                    shadow->shadowDirty = GL_FALSE;
                                }

                                renderMipmap = GL_TRUE;
                            }

                            /* set update flag as it will be rendered later. not very precisely*/
                            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
                        }
                    }
                }
                iu = iu->next;
            }
        }

        if (buf && ((mipmap->width * mipmap->height * mipmap->depth) != 0))
        {
            gctSIZE_T rowStride = 0;
            gctSIZE_T imgHeight = (gctSIZE_T)mipmap->height;
            GLboolean compressed = mipmap->compressed;
            GLboolean needDecompress = GL_FALSE;
            gctSIZE_T compressedSize = mipmap->compressedSize;
            gctSIZE_T skipImgs = (gctSIZE_T)gc->clientState.pixel.unpackModes.skipImages;

            /*
            ** If core format indicate it's compressed AND there is mismatch within core->requestHAL->readHAL,
            ** which mean we have to do conversion.
            */
            if ((formatMapInfo->flags & (__GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ |
                                         __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_READ)) &&
                 mipmap->compressed)
            {
                needDecompress = GL_TRUE;
            }

            switch (mipmap->requestedFormat)
            {
            case GL_ETC1_RGB8_OES:
                if (needDecompress)
                {
                    GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                    /* Decompress ETC1 texture since hardware doesn't support it. */
                    pixels = gcChipDecompressETC1(gc,
                                                  (gctSIZE_T)mipmap->width,
                                                  (gctSIZE_T)mipmap->height,
                                                  (gctSIZE_T)mipmap->compressedSize,
                                                  buf,
                                                  &srcImageFormat,
                                                  &rowStride);
                    compressed = GL_FALSE;
                    needClean = GL_TRUE;
                }
                else
                {
                    pixels = buf;
                }
                break;

             case GL_COMPRESSED_R11_EAC:
             case GL_COMPRESSED_SIGNED_R11_EAC:
             case GL_COMPRESSED_RG11_EAC:
             case GL_COMPRESSED_SIGNED_RG11_EAC:
                if (needDecompress)
                {
                    GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                    /* Decompress. */
                    pixels = gcChipDecompress_EAC_11bitToR16F(gc,
                                                              (gctSIZE_T)mipmap->width,
                                                              (gctSIZE_T)mipmap->height,
                                                              (gctSIZE_T)mipmap->depth,
                                                              (gctSIZE_T)mipmap->compressedSize,
                                                              buf,
                                                              mipmap->requestedFormat,
                                                              &srcImageFormat,
                                                              &rowStride);
                    compressed = GL_FALSE;
                    needClean = GL_TRUE;
                }
                else
                {
                    pixels = buf;
                }
                break;

             case GL_COMPRESSED_RGB8_ETC2:
             case GL_COMPRESSED_SRGB8_ETC2:
             case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
             case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
             case GL_COMPRESSED_RGBA8_ETC2_EAC:
             case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
                if (needDecompress)
                {
                    GL_ASSERT(GL_FALSE);
                }
                else
                {
                    pixels = buf;
                }
                break;

            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                if (needDecompress)
                {
                    GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                    pixels = gcChipDecompressDXT(gc,
                                                 (gctSIZE_T)mipmap->width,
                                                 (gctSIZE_T)mipmap->height,
                                                 (gctSIZE_T)mipmap->compressedSize,
                                                 buf,
                                                 mipmap->requestedFormat,
                                                 &srcImageFormat,
                                                 &rowStride);
                    compressed = GL_FALSE;
                    needClean = GL_TRUE;
                }
                else
                {
                    pixels = buf;
                }
                break;

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
                {
                    GL_ASSERT(needDecompress);

                    GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                    pixels = gcChipDecompressPalette(gc,
                                                     mipmap->requestedFormat,
                                                     width,
                                                     height,
                                                     level,
                                                     (gctSIZE_T)mipmap->compressedSize,
                                                     buf,
                                                     &srcImageFormat,
                                                     &rowStride);
                    compressed = GL_FALSE;
                    needClean = GL_TRUE;
                    break;
                }

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
                /* if texture is 2d array or cube map and gcvFEATURE_TX_ASTC_MULTISLICE_FIX is false, should decompress, */
                if (needDecompress)
                {
                    gctUINT newDepth = (gctUINT)__GL_MAX(texObj->arrays, mipmap->depth);
                    gctSIZE_T astcBytes = mipmap->compressedSize * newDepth;

                    pixels = gcChipDecompressASTC(gc,
                                                  (gctSIZE_T)mipmap->width,
                                                  (gctSIZE_T)mipmap->height,
                                                  (gctSIZE_T)numSlices,
                                                  (gctSIZE_T)mipmap->compressedSize,
                                                  buf,
                                                  mipmap->formatInfo,
                                                  &srcImageFormat,
                                                  &rowStride);

                    compressed = GL_FALSE;
                    needClean = GL_TRUE;

                    /* If resized or formated was changed, need to destroy the surf. */
                    if (chipMipLevel->astcSurf)
                    {
                        gceSURF_FORMAT oldFormat = gcvSURF_UNKNOWN;
                        gctUINT oldWidth = 0, oldHeight = 0, oldDepth = 0;
                        __GLchipFmtMapInfo *astcFmtInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

                        gcmONERROR(gcoSURF_GetSize(chipMipLevel->astcSurf, &oldWidth, &oldHeight, &oldDepth));
                        gcmONERROR(gcoSURF_GetFormat(chipMipLevel->astcSurf, gcvNULL, &oldFormat));

                        if (oldWidth  != (gctUINT)mipmap->width  ||
                            oldHeight != (gctUINT)mipmap->height ||
                            oldDepth  != newDepth                ||
                            oldFormat != astcFmtInfo->readFormat)
                        {
                            gcmONERROR(gcoSURF_Unlock(chipMipLevel->astcSurf, (gctPOINTER)chipMipLevel->astcData));
                            gcmONERROR(gcoSURF_Destroy(chipMipLevel->astcSurf));
                            chipMipLevel->astcSurf = gcvNULL;
                            chipMipLevel->astcData = gcvNULL;
                            chipMipLevel->astcBytes = 0;
                        }
                    }

                    if (chipMipLevel->astcBytes != astcBytes)
                    {

                        /* If resized, previously astcSurf must be destroyed. */
                        GL_ASSERT(!chipMipLevel->astcSurf);

                        if (chipMipLevel->astcData)
                        {
                            gcoOS_Free(gcvNULL, (gctPOINTER)chipMipLevel->astcData);
                            chipMipLevel->astcData = gcvNULL;
                        }
                        gcmONERROR(gcoOS_Allocate(gcvNULL, astcBytes, (gctPOINTER*)&chipMipLevel->astcData));

                        chipMipLevel->astcBytes = astcBytes;
                    }

                    gcoOS_MemCopy(chipMipLevel->astcData + face * mipmap->compressedSize, buf, numSlices * mipmap->compressedSize);
                }
                else
                {
                    GL_ASSERT(needDecompress == GL_FALSE);
                    pixels = buf;
                }
                break;

            default:
                {
                    gctSIZE_T skipOffset = 0;

                    /* Only uncompressed textures need to take unpack mode into consideration */
                    gcChipProcessPixelStore(gc,
                                            &gc->clientState.pixel.unpackModes,
                                            (gctSIZE_T)mipmap->width,
                                            (gctSIZE_T)mipmap->height,
                                            mipmap->format,
                                            mipmap->type,
                                            skipImgs,
                                            &rowStride,
                                            &imgHeight,
                                            &skipOffset);

                    pixels = (gctPOINTER)((gctINT8_PTR)buf + skipOffset);

                    if ((mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8) ||
                        (mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8))
                    {
                        srcColorSpace = gcvSURF_COLOR_SPACE_NONLINEAR;
                    }
                }
                break;
            }

            for (i = 0; i < numSlices; ++i)
            {
                if (compressed)
                {
                    gcmONERROR(gcoTEXTURE_UploadCompressed(texInfo->object,
                                                           level,
                                                           halFace,
                                                           (gctSIZE_T)mipmap->width,
                                                           (gctSIZE_T)mipmap->height,
                                                           i,
                                                           (const GLvoid*)((GLubyte*)pixels + (i * compressedSize)),
                                                           compressedSize));
                }
                else
                {
                    gcmONERROR(gcoTEXTURE_Upload(texInfo->object,
                                                 level,
                                                 halFace,
                                                 (gctSIZE_T)mipmap->width,
                                                 (gctSIZE_T)mipmap->height,
                                                 i,
                                                 (const GLvoid*)((GLbyte*)pixels + rowStride * imgHeight * i),
                                                 rowStride, /* stride */
                                                 srcImageFormat,
                                                 srcColorSpace));
                }

                texInfo->mipLevels[level].shadow[face + i].masterDirty = GL_TRUE;
                texInfo->mipLevels[level].shadow[face + i].shadowDirty = GL_FALSE;
            }

            if (needClean)
            {
                gcoOS_Free(gcvNULL, (gctPOINTER)pixels);
                needClean = GL_FALSE;
            }

            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
        }

        if (renderMipmap)
        {
            /* Set up direct render into mipmap surface  */
            gcmONERROR(gcoTEXTURE_RenderIntoMipMap2(texInfo->object,
                                                    level,
                                                    texInfo->mipLevels[level].shadow[face].masterDirty));

            /* set update flag as it will be rendered later. not very precisely*/
            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
        }

        level++;
    }

    if (unpackBufObj) /* The image is from unpack buffer object */
    {
        gcmONERROR(gcChipPostProcessPBO(gc, unpackBufObj, GL_FALSE));
    }

OnError:
    if (needClean && pixels)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)pixels);
    }
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipCreateTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    GLuint level;
    __GLchipTextureInfo *texInfo;
    gctSIZE_T size;
    GLvoid *pointer = NULL;
    __GLchipResourceShadow *shadows = NULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    if (texObj->privateData)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    texInfo = (__GLchipTextureInfo *)gc->imports.calloc(gc, 1, sizeof(__GLchipTextureInfo));
    if (!texInfo)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
    texObj->privateData = texInfo;

    texInfo->rendered = GL_FALSE;

    size = texObj->maxLevels * sizeof(__GLchipMipmapInfo)
         + texObj->maxLevels * texObj->maxSlices * sizeof(__GLchipResourceShadow);

    pointer = gc->imports.calloc(gc, 1, size);
    if (!pointer)
    {
        gcmONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    texInfo->mipLevels = (__GLchipMipmapInfo*)pointer;
    shadows = (__GLchipResourceShadow*)(texInfo->mipLevels + texObj->maxLevels);

    for (level = 0; level < texObj->maxLevels; ++level)
    {
        texInfo->mipLevels[level].shadow = shadows;
        shadows += texObj->maxSlices;
    }

    texInfo->direct.dirty = GL_FALSE;
    texInfo->direct.source = gcvNULL;
    texInfo->direct.directSample = gcvFALSE;

    texInfo->eglImage.dirty = GL_FALSE;
    texInfo->eglImage.source = gcvNULL;
    texInfo->eglImage.directSample = gcvFALSE;
    texInfo->eglImage.image = gcvNULL;
    texInfo->eglImage.nativeBuffer = gcvNULL;

#if gcdUSE_NPOT_PATCH
    texInfo->isNP2 = GL_FALSE;
#endif

#if __GL_CHIP_PATCH_ENABLED
    texInfo->isFboRendered = GL_FALSE;
#endif

OnError:
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  initializeTempBitmap
**
**  Initialize the temporary bitmap image.
**
**  INPUT:
**
**      Context
**          Pointer to the context.
**
**      Format
**          Format of the image.
**
**      Width, Height
**          The size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

static gceSTATUS
gcChipInitializeTempBitmap(
    IN __GLchipContext *chipCtx,
    IN gceSURF_FORMAT Format,
    IN gctSIZE_T Width,
    IN gctSIZE_T Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF bitmap = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Format=%d Width=%u Height=%u",
                    chipCtx, Format, Width, Height);

    do
    {
        /* See if the existing surface can be reused. */
        if ((chipCtx->tempWidth  < Width)  ||
            (chipCtx->tempHeight < Height) ||
            (chipCtx->tempFormat != Format))
        {
            gctUINT width;
            gctUINT height;
            gctINT stride;
            gctPOINTER bits[3];
            gcsSURF_FORMAT_INFO_PTR info;

            /* Is there a surface allocated? */
            if (chipCtx->tempBitmap != gcvNULL)
            {
                /* Unlock the surface. */
                if (chipCtx->tempBits != gcvNULL)
                {
                    gcmERR_BREAK(gcoSURF_Unlock(chipCtx->tempBitmap, chipCtx->tempBits));
                    chipCtx->tempBits = gcvNULL;
                }

                /* Destroy the surface. */
                gcmERR_BREAK(gcoSURF_Destroy(chipCtx->tempBitmap));

                /* Reset temporary surface. */
                chipCtx->tempBitmap       = gcvNULL;
                chipCtx->tempFormat       = gcvSURF_UNKNOWN;
                chipCtx->tempBitsPerPixel = 0;
                chipCtx->tempWidth        = 0;
                chipCtx->tempHeight       = 0;
                chipCtx->tempStride       = 0;
            }

            /* Valid surface requested? */
            if (Format != gcvSURF_UNKNOWN)
            {
                /* Round up the size. */
                width  = gcmALIGN(Width,  256);
                height = gcmALIGN(Height, 256);

                /* Allocate a new surface. */
                gcmONERROR(gcoSURF_Construct(chipCtx->hal,
                                             width, height, 1,
                                             gcvSURF_BITMAP, Format,
                                             gcvPOOL_UNIFIED,
                                             &bitmap));

                /* Get the pointer to the bits. */
                gcmONERROR(gcoSURF_Lock(bitmap, gcvNULL, bits));

                /* Query the parameters back. */
                gcmONERROR(gcoSURF_GetAlignedSize(bitmap, &width, &height, &stride));

                /* Query format specifics. */
                gcmONERROR(gcoSURF_QueryFormat(Format, &info));

                /* Temp bitmap didn't support faked multi-layer formats */
                if (info->layers > 1)
                {
                    gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
                }

                /* Set information. */
                chipCtx->tempBitmap       = bitmap;
                chipCtx->tempBits         = bits[0];
                chipCtx->tempFormat       = Format;
                chipCtx->tempBitsPerPixel = info->bitsPerPixel;
                chipCtx->tempWidth        = (gctSIZE_T)width;
                chipCtx->tempHeight       = (gctSIZE_T)height;
                chipCtx->tempStride       = (gctSIZE_T)stride;
            }
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    } while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;

OnError:
    if (bitmap != gcvNULL)
    {
        gcoSURF_Destroy(bitmap);
        bitmap = gcvNULL;
    }
    gcmFOOTER();
    return status;
}


/*******************************************************************************
**
**  resolveDrawToTempBitmap
**
**  Resolve specified area of the drawing surface to the temporary bitmap.
**
**  INPUT:
**
**      chipCtx
**          Pointer to the context.
**
**      SourceX, SourceY, Width, Height
**          The origin and the size of the image.
**
** OUTPUT:
**
**      Nothing.
*/

static gceSTATUS
gcChipResolveDrawToTempBitmap(
    IN __GLchipContext *chipCtx,
    IN gcsSURF_VIEW *srcView,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height)
{
    gceSTATUS status;
    gceSURF_FORMAT format  = gcvSURF_UNKNOWN;
    GLuint  surfWidth    = 0;
    GLuint  surfHeight   = 0;

    gcmHEADER_ARG("chipCtx=0x%x srcView=0x%x SourceX=%d SourceY=%d Width=%d Height=%d",
                    chipCtx, srcView, SourceX, SourceY, Width, Height);

    do
    {
        gctUINT resX       = 0;
        gctUINT resY       = 0;
        gctUINT resW       = 0;
        gctUINT resH       = 0;

        gctUINT sourceX    = 0;
        gctUINT sourceY    = 0;

        gctINT left        = 0;
        gctINT top         = 0;
        gctINT right       = 0;
        gctINT bottom      = 0;

        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};
        GLboolean surfYInverted = (gcoSURF_QueryFlags(srcView->surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);

        gcmERR_BREAK(gcoSURF_GetSize(srcView->surf, &surfWidth, &surfHeight, gcvNULL));

        /* Clamp coordinates. */
        left   = gcmMAX(SourceX, 0);
        top    = gcmMAX(SourceY, 0);
        right  = gcmMIN(SourceX + Width,  (GLint) surfWidth);
        bottom = gcmMIN(SourceY + Height, (GLint) surfHeight);

        if ((right <= 0) || (bottom <= 0))
        {
            gcmERR_BREAK(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmERR_BREAK(gcoSURF_GetResolveAlignment(srcView->surf,
                                                 &resX,
                                                 &resY,
                                                 &resW,
                                                 &resH));

        /* Convert GL coordinates. */
        sourceX = left;

        /* Adjust to right rectangle */
        if (surfYInverted)
        {
            sourceY = (gctINT)(surfHeight - bottom);
        }
        else
        {
            sourceY = top;
        }

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted  = surfYInverted;
        rlvArgs.uArgs.v2.numSlices = 1;

        /* Determine the aligned source origin. */
        rlvArgs.uArgs.v2.srcOrigin.x = sourceX & ~(resX - 1);
        rlvArgs.uArgs.v2.srcOrigin.y = sourceY & ~(resY - 1);
        if ((rlvArgs.uArgs.v2.srcOrigin.x + (gctINT)resW > (GLint)surfWidth) &&
            (surfWidth > resW))
        {
            rlvArgs.uArgs.v2.srcOrigin.x = (surfWidth - resW) & ~(resX - 1);
        }

        if ((rlvArgs.uArgs.v2.srcOrigin.y + (gctINT)resH > (GLint)surfHeight) &&
            (surfHeight > resH))
        {
            rlvArgs.uArgs.v2.srcOrigin.y = (surfHeight - resH) & ~(resY - 1);
        }

        /* Determine the origin adjustment. */
        chipCtx->tempX = sourceX - rlvArgs.uArgs.v2.srcOrigin.x;
        chipCtx->tempY = sourceY - rlvArgs.uArgs.v2.srcOrigin.y;

        /* Determine the aligned area size. */
        rlvArgs.uArgs.v2.rectSize.x = (gctUINT) gcmALIGN(right  - left + chipCtx->tempX, resW);
        rlvArgs.uArgs.v2.rectSize.y = (gctUINT) gcmALIGN(bottom - top  + chipCtx->tempY, resH);

        gcmERR_BREAK(gcoSURF_GetPackedFormat(srcView->surf, &format));

        /* Initialize the temporary surface. */
        gcmERR_BREAK(gcChipInitializeTempBitmap(
            chipCtx,
            format,
            rlvArgs.uArgs.v2.rectSize.x,
            rlvArgs.uArgs.v2.rectSize.y
            ));

        tmpView.surf = chipCtx->tempBitmap;
        /* Resolve the aligned area. */
        gcmERR_BREAK(gcoSURF_ResolveRect(srcView, &tmpView, &rlvArgs));

        /* Make sure the operation is complete. */
        gcmERR_BREAK(gcoHAL_Commit( chipCtx->hal, gcvTRUE));

        if (surfYInverted)
        {
            /* Compute the pointer to the last line. */
            chipCtx->tempLastLine
                =  chipCtx->tempBits
                +  chipCtx->tempStride * (rlvArgs.uArgs.v2.rectSize.y - (chipCtx->tempY + (bottom - top)))
                + (chipCtx->tempX      * chipCtx->tempBitsPerPixel) / 8;
        }
        else
        {
            /* Compute the pointer to the last line. */
            chipCtx->tempLastLine
                =  chipCtx->tempBits
                +  chipCtx->tempStride * chipCtx->tempY
                + (chipCtx->tempX      * chipCtx->tempBitsPerPixel) / 8;
        }

    } while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/* Orphan this tex while keep content of base mipmap */
static gceSTATUS
gcChipOrphanTexMipmap(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLtextureObject *texObj
    )
{
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    gcoTEXTURE newHalTex = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x texObj=0x%x", gc, chipCtx, texObj);

    if (texInfo->eglImage.image && texInfo->object)
    {
        gcoSURF oldSurf = gcvNULL;
        gcoSURF newSurf = gcvNULL;
        GLint baseLevel = texObj->params.baseLevel;
        __GLmipMapLevel *baseMipmap = &texObj->faceMipmap[0][baseLevel];
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[baseLevel];

        gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, baseLevel, &oldSurf));

        /* Create new hal texture object */
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &newHalTex));
        gcmONERROR(gcoTEXTURE_SetEndianHint(newHalTex, gcChipUtilGetEndianHint(baseMipmap->requestedFormat, baseMipmap->type)));
        gcmONERROR(gcoTEXTURE_AddMipMap(newHalTex,
                                        baseLevel,
                                        baseMipmap->requestedFormat,
                                        chipMipLevel->formatMapInfo->readFormat,
                                        (gctSIZE_T)baseMipmap->width,
                                        (gctSIZE_T)baseMipmap->height,
                                        (texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX) ?
                                                    (gctSIZE_T)texObj->arrays : (gctSIZE_T)baseMipmap->depth,
                                        (texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX) ? 1 : texObj->arrays,
                                        gcvPOOL_DEFAULT,
                                        &newSurf));

        gcmONERROR(gcoSURF_Copy(newSurf, oldSurf));
        CHIP_TEX_IMAGE_UPTODATE(texInfo, 0);
        gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

        texInfo->object = newHalTex;
    }

OnError:
    if (gcmIS_ERROR(status) && newHalTex)
    {
        gcoTEXTURE_Destroy(newHalTex);
    }
    gcmFOOTER();
    return status;
}


static GLboolean
gcChipIsResolvable(
    gceSURF_FORMAT format
    )
{
    GLboolean result = GL_FALSE;

    switch (format)
    {
    case gcvSURF_X4R4G4B4:
    case gcvSURF_A4R4G4B4:
    case gcvSURF_X1R5G5B5:
    case gcvSURF_A1R5G5B5:
    case gcvSURF_X1B5G5R5:
    case gcvSURF_A1B5G5R5:
    case gcvSURF_R5G6B5:
    case gcvSURF_X8R8G8B8:
    case gcvSURF_A8R8G8B8:
    case gcvSURF_A2B10G10R10UI_1_A8R8G8B8:
    case gcvSURF_A8B8G8R8I_1_A8R8G8B8:
    case gcvSURF_A8B8G8R8UI_1_A8R8G8B8:
    case gcvSURF_X8B8G8R8:
    case gcvSURF_A8B8G8R8:
    case gcvSURF_YUY2:
    /* Fake 16-bit formats. */
    case gcvSURF_D16:
    /* Fake 32-bit formats. */
    case gcvSURF_D24S8:
    case gcvSURF_D24X8:
    case gcvSURF_D32:
    case gcvSURF_A2B10G10R10:
    case gcvSURF_A8_SBGR8:
    case gcvSURF_A8_SRGB8:
        result = GL_TRUE;
        break;

    default:
        break;
    }

    return result;
}

static gceSTATUS
gcChipCopyTexImage(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x,
    GLint y
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext*     chipCtx = CHIP_CTXINFO(gc);
    __GLmipMapLevel*     mipmap  = &texObj->faceMipmap[face][level];
    __GLchipTextureInfo* texInfo = (__GLchipTextureInfo*)texObj->privateData;
    __GLchipMipmapInfo*  chipMipLevel = &texInfo->mipLevels[level];
    gctBOOL              tryResolve = gcvFALSE;
    gctBOOL              tryShader = gcvFALSE;
    gcsSURF_VIEW         texView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         srcView = {gcvNULL, 0, 1};

    gcePATCH_ID patchId;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d x=%d y=%d",
                   gc, texObj, face, level, x, y);

    gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

    /* Construct the gcoTEXTURE object. */
    if (texInfo->object == gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));
    }

    chipMipLevel->formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

    if (texInfo->eglImage.source)
    {
        gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
    }
    else
    {
        /* Add the mipmap. If it already exists, the call will be ignored. */
        gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                                        level,
                                        mipmap->requestedFormat,
                                        chipMipLevel->formatMapInfo->readFormat,
                                        (gctSIZE_T)mipmap->width,
                                        (gctSIZE_T)mipmap->height,
                                        1,
                                        texObj->arrays,
                                        gcvPOOL_DEFAULT,
                                        &texView.surf));
    }

    texView.firstSlice = face;
    gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, level, &texView.surf));

    /* Whether resolve can handle the copy? Should be aligned and face==0 */
    if (__GL_TEXTURE_2D_INDEX == texObj->targetIndex)
    {
        tryResolve = gcvTRUE;
    }

    /* Whether shader draw can handle the copy */
    if ((__GL_TEXTURE_2D_INDEX == texObj->targetIndex) &&
        mipmap->formatInfo->renderable &&
        /* Currently no 3Dblit shader for INT sampling. No need check dst for GLcore guarantees. */
        (mipmap->formatInfo->category != GL_INT && mipmap->formatInfo->category != GL_UNSIGNED_INT) &&
        gcmIS_SUCCESS(gcoSURF_IsRenderable(texView.surf)) &&
        chipCtx->chipFeature.hwFeature.hasSupertiledTx
        )
    {
        tryShader = gcvTRUE;
    }

    /* TBD: Filter out unsupported cases later */
    patchId = chipCtx->patchId;

    if (patchId == gcvPATCH_GTFES30)
    {
        tryShader = GL_FALSE;
    }

    /* If chipCtx->readRT is a shadow surface, we should sync it to master resource
    ** And read pixels from master resource, as shadow resource may have different
    ** meaning (format) with master surface, such as SRGB encoding.
    ** Even for the same format between shadow and master, then we only sync once.
    */
    srcView = gcChipFboSyncFromShadowSurface(gc, &chipCtx->readRtView, GL_TRUE);

    if (srcView.surf && srcView.surf->isMsaa)
    {
        if ((!gcChipIsResolvable(srcView.surf->format) ||
             !gcChipIsResolvable(texView.surf->format)))
        {
            tryResolve = gcvFALSE;
        }

        tryShader = gcvFALSE;
    }

    if ((chipCtx->chipModel == gcv600 && chipCtx->chipRevision == 0x4652) &&
        mipmap->formatInfo->drvFormat == __GL_FMT_A8 &&
        srcView.surf->tiling == texView.surf->tiling)
    {
        tryShader = gcvTRUE;
    }

    do
    {
        gctINT width  = __GL_MIN(mipmap->width,  (gctINT32)chipCtx->readRTWidth  - x);
        gctINT height = __GL_MIN(mipmap->height, (gctINT32)chipCtx->readRTHeight - y);

        /* Skip copy for zero dim */
        if (width <= 0 || height <= 0)
        {
            goto OnExit;
        }

        if (tryResolve)
        {
            if (width > 0 && height > 0)
            {
                gcsSURF_RESOLVE_ARGS rlvArgs = {0};

                rlvArgs.version = gcvHAL_ARG_VERSION_V2;
                rlvArgs.uArgs.v2.srcOrigin.x = x;
                rlvArgs.uArgs.v2.srcOrigin.y = chipCtx->readYInverted
                                               ? (gctINT) (chipCtx->readRTHeight - y - height) : y;
                rlvArgs.uArgs.v2.rectSize.x  = width;
                rlvArgs.uArgs.v2.rectSize.y  = height;
                rlvArgs.uArgs.v2.yInverted = chipCtx->readYInverted;
                rlvArgs.uArgs.v2.numSlices = 1;
                rlvArgs.uArgs.v2.gpuOnly   = gcvTRUE;

                status = gcoSURF_ResolveRect(&srcView, &texView, &rlvArgs);

                if (gcmIS_SUCCESS(status))
                {
                    /* Done with resolve. */
                    break;
                }
            }
        }

        if (tryShader)
        {
            if (gcmIS_ERROR(gcoTEXTURE_RenderIntoMipMap2(texInfo->object,
                                                         level,
                                                         chipMipLevel->shadow[face].masterDirty)))
            {
                tryShader = gcvFALSE;
            }

            if (gcmIS_ERROR((gcoTEXTURE_GetMipMap(texInfo->object, level, &texView.surf))))
            {
                tryShader = gcvFALSE;
            }
        }

        if (tryShader)
        {
            gscSURF_BLITDRAW_BLIT blitArgs;

            __GL_MEMZERO(&blitArgs, sizeof(blitArgs));
            blitArgs.srcRect.left = x;
            blitArgs.srcRect.top = y;
            blitArgs.srcRect.right = x + width;
            blitArgs.srcRect.bottom = y + height;
            blitArgs.dstRect.left = 0;
            blitArgs.dstRect.top = 0;
            blitArgs.dstRect.right = width;
            blitArgs.dstRect.bottom = height;
            blitArgs.filterMode = gcvTEXTURE_POINT;

            blitArgs.yReverse = chipCtx->readYInverted;
            if (chipCtx->readYInverted)
            {
                blitArgs.srcRect.top = (gctINT32)(chipCtx->readRTHeight - (y + mipmap->height));
                blitArgs.srcRect.bottom = (gctINT32)(chipCtx->readRTHeight - y);
            }

            status = gcoSURF_DrawBlit(&srcView, &texView, &blitArgs);

            if (gcmIS_SUCCESS(status))
            {
                /* Done with shader. */
                break;
            }
        }

        /* Try resolve to temp bitmap then upload. */
        if (gcmIS_SUCCESS(gcChipResolveDrawToTempBitmap(chipCtx,
                                                        &srcView,
                                                        x, y,
                                                        width,
                                                        height))
           )
        {
            gceSURF_COLOR_SPACE srcColorSpace;

            gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                                    ? gcvFACE_POSITIVE_X + face
                                    : gcvFACE_NONE;

            gcmONERROR(gcoSURF_GetColorSpace(srcView.surf, &srcColorSpace));

            /* Upload the texture. */
            gcmONERROR(gcoTEXTURE_Upload(texInfo->object,
                                         level,
                                         halFace,
                                         (gctSIZE_T)width,
                                         (gctSIZE_T)height,
                                         0,
                                         chipCtx->tempLastLine,
                                         chipCtx->tempStride,
                                         chipCtx->tempFormat,
                                         srcColorSpace));
            if (gcmIS_SUCCESS(status))
            {
                /* Done with shader. */
                break;
            }
        }

        /* Finally, try CPU copy. */
        {
            gcsSURF_BLIT_ARGS blitArgs;

            gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
            blitArgs.srcSurface         = srcView.surf;
            blitArgs.srcX               = x;
            blitArgs.srcY               = y;
            blitArgs.srcZ               = srcView.firstSlice;
            blitArgs.srcWidth           = width;
            blitArgs.srcHeight          = height;
            blitArgs.srcDepth           = 1;
            blitArgs.dstSurface         = texView.surf;
            blitArgs.dstX               = 0;
            blitArgs.dstY               = 0;
            blitArgs.dstZ               = face;
            blitArgs.dstWidth           = width;
            blitArgs.dstHeight          = height;
            blitArgs.dstDepth           = 1;
            blitArgs.xReverse           = gcvFALSE;
            blitArgs.yReverse           = gcvFALSE;
            blitArgs.scissorTest        = gcvFALSE;
            blitArgs.srcNumSlice        = srcView.numSlices;
            blitArgs.dstNumSlice        = texView.numSlices;

            if (chipCtx->readYInverted)
            {
                blitArgs.yReverse       = GL_TRUE;
                blitArgs.srcY           = (gctINT)(chipCtx->readRTHeight - (y + mipmap->height));
            }

            gcmONERROR(gcoSURF_BlitCPU(&blitArgs));
        }
    }
    while (gcvFALSE);

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
    /*
     * If FBO master is from EGLImage-NativeBuffer, we need block access of the
     * native buffer from other hardware before GPU finishes read.
     */
    gcChipFBOSyncEGLImageNativeBuffer(gc, chipCtx->readRtView.surf, GL_TRUE);
#endif

    texInfo->mipLevels[level].shadow[face].masterDirty = GL_TRUE;
    CHIP_TEX_IMAGE_UPTODATE(texInfo, level);

OnExit:
OnError:
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipTexSubImage(
    __GLcontext        * gc,
    __GLtextureObject  * texObj,
    GLint                face,
    GLint                level,
    GLint                xoffset,
    GLint                yoffset,
    GLint                zoffset,
    GLint                width,
    GLint                height,
    GLint                depth,
    const GLvoid       * buf
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;
    __GLchipVertexBufferInfo *unpackBufInfo = gcvNULL;
    gctUINT32 physicalAddress = gcvINVALID_ADDRESS;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d xoffset=%d yoffset=%d"
                   "zoffset=%d width=%d height=%d depth=%d buf=0x%x",
                   gc, texObj, face, level, xoffset, yoffset, zoffset,
                   width, height, depth, buf);

    if (texInfo->eglImage.source)
    {
        if (texInfo->eglImage.directSample)
        {
            gceTILING tiling = gcvINVALIDTILED;

            gcmVERIFY_OK(gcoSURF_GetTiling(texInfo->eglImage.source, &tiling));

            if ((tiling != gcvTILED) && (tiling != gcvSUPERTILED) && (tiling != gcvYMAJOR_SUPERTILED))
            {
                /*
                 * Will later upload texture pixels by CPU below.
                 * There's only tiled and super-tiled path in CPU texture
                 * uploading for now. So here we force indirect sampling, ie,
                 * allocate a tiled texture surface.
                 */
                texInfo->eglImage.directSample = gcvFALSE;
                texInfo->eglImage.dirty = gcvTRUE;
            }
        }

        /* Sync from EGLImage source when it is changed, wait sync finish before CPU sub upload */
        gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvTRUE));
    }

    if (buf || unpackBufObj)
    {
        GLint i;
        gctSIZE_T rowStride = 0;
        gctSIZE_T imgHeight = 0;
        gctSIZE_T skipOffset = 0;
        __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
        gceSURF_FORMAT imageFormat = gcvSURF_UNKNOWN;
        gceSURF_COLOR_SPACE srcColorSpace = gcvSURF_COLOR_SPACE_LINEAR;
        __GLmipMapLevel *mipmap = &texObj->faceMipmap[face][level];
        __GLchipMipmapInfo *mipLevel = &texInfo->mipLevels[level];
        gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                                ? gcvFACE_POSITIVE_X + face : gcvFACE_NONE;
        gctSIZE_T skipImgs = (__GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ||
                             texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ?
                             (gctSIZE_T)gc->clientState.pixel.unpackModes.skipImages : 0;

        gcChipProcessPixelStore(gc,
                                &gc->clientState.pixel.unpackModes,
                                (gctSIZE_T)width,
                                (gctSIZE_T)height,
                                mipmap->format,
                                mipmap->type,
                                skipImgs,
                                &rowStride,
                                &imgHeight,
                                &skipOffset);

        if (unpackBufObj)
        {
            unpackBufInfo = (__GLchipVertexBufferInfo*)(unpackBufObj->privateData);

            skipOffset += __GL_PTR2SIZE(buf);
            GL_ASSERT(unpackBufInfo);
            gcmONERROR(gcoBUFOBJ_Lock(unpackBufInfo->bufObj, &physicalAddress, (gctPOINTER*)&buf));
            /* wait fence here becuase of the bufobj may comes from packbuffer obj which write in glReadPixel*/
            gcmONERROR(gcoBUFOBJ_WaitFence(unpackBufInfo->bufObj, gcvFENCE_TYPE_WRITE));
            /* get fence here becuase of the bufobj may will be used in glMapBufferRange for read */
            gcmONERROR(gcoBUFOBJ_GetFence(unpackBufInfo->bufObj, gcvFENCE_TYPE_READ));

            physicalAddress += (gctUINT32)skipOffset;
        }
        buf = (gctPOINTER)((gctINT8_PTR)buf + skipOffset);

        gcChipUtilGetImageFormat(mipmap->format, mipmap->type, &imageFormat, gcvNULL);

        if ((mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8) ||
            (mipmap->formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8))
        {
            srcColorSpace = gcvSURF_COLOR_SPACE_NONLINEAR;
        }

        for (i = 0; i < depth; ++i)
        {
            gctUINT32 slicePhyAddress;
            gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc, texObj, face, level, zoffset + i));

            slicePhyAddress = (physicalAddress == gcvINVALID_ADDRESS)
                            ? gcvINVALID_ADDRESS
                            : (physicalAddress + (gctUINT32)(rowStride * imgHeight * i));

            gcmONERROR(gcoTEXTURE_UploadSub(texInfo->object,
                                            level,
                                            halFace,
                                            (gctSIZE_T)xoffset, (gctSIZE_T)yoffset,
                                            (gctSIZE_T)width, (gctSIZE_T)height,
                                            zoffset + i,
                                            (const GLvoid*)((GLbyte*)buf + rowStride * imgHeight * i),
                                            rowStride,
                                            imageFormat,
                                            srcColorSpace,
                                            slicePhyAddress));

            mipLevel->shadow[face + zoffset + i].masterDirty = GL_TRUE;

            if (texInfo->eglImage.image)
            {
                gcsSURF_VIEW mipView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, level, face + zoffset + i);

                if (mipView.surf)
                {
                    gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, mipView.surf));
                }
            }

            if (chipCtx->needStencilOpt &&
                mipLevel->stencilOpt &&
                mipmap->formatInfo->stencilSize > 0)
            {
                gcsRECT rect;

                rect.left   = xoffset;
                rect.right  = xoffset + width - 1;
                rect.top    = yoffset;
                rect.bottom = yoffset + height - 1;

                gcChipPatchStencilOptWrite(gc, &mipLevel->stencilOpt[face + zoffset + i], &rect, 0, 0, GL_TRUE);
            }
        }

        CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
    }

    if (unpackBufObj) /* The image is from unpack buffer object */
    {
        __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo *)(unpackBufObj->privateData);

        gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
        gcmONERROR(gcChipPostProcessPBO(gc, unpackBufObj, GL_FALSE));
    }

OnError:
    /* The image is from unpack buffer object */
    if (unpackBufInfo && physicalAddress != gcvINVALID_ADDRESS)
    {
        gcmONERROR(gcoBUFOBJ_Unlock(unpackBufInfo->bufObj));
    }

    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipCopyTexSubImage(
    __GLcontext       * gc,
    __GLtextureObject * texObj,
    GLint               face,
    GLint               level,
    GLint               x,
    GLint               y,
    GLint               xoffset,
    GLint               yoffset,
    GLint               zoffset,
    GLint               width,
    GLint               height
    )
{
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    __GLchipTextureInfo * texInfo    = (__GLchipTextureInfo*)texObj->privateData;
    gctBOOL tryResolve               = gcvFALSE;
    gctBOOL tryShader                = gcvFALSE;
    __GLmipMapLevel    *mipmap       = &texObj->faceMipmap[face][level];
    __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
    GLint               slice        = face > 0 ? face : zoffset;
    gcsSURF_VIEW        texView      = {gcvNULL, 0, 1};
    gcsSURF_VIEW        srcView      = {gcvNULL, 0, 1};
    gceSTATUS status = gcvSTATUS_OK;
    gcePATCH_ID patchId = chipCtx->patchId;
    gcoSURF     mip = gcvNULL;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d x=%d y=%d xoffset=%d yoffset=%d zoffset=%d width=%d height=%d",
                   gc, texObj, face, level, x, y, xoffset, yoffset, zoffset, width, height);

    /* Check to construct the gcoTEXTURE object. */
    if (texInfo->object == gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal,
                                          __glChipTexTargetToHAL[texObj->targetIndex],
                                          &texInfo->object));
    }

    if (texInfo->eglImage.source)
    {
        gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
    }
    else
    {
        /* Add the mipmap. If it already exists, the call will be ignored. */
        gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                                        level,
                                        mipmap->requestedFormat,
                                        chipMipLevel->formatMapInfo->readFormat,
                                        (gctSIZE_T)mipmap->width,
                                        (gctSIZE_T)mipmap->height,
                                        __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ?
                                            (gctSIZE_T)texObj->arrays : (gctSIZE_T)mipmap->depth,
                                        __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? 1 : texObj->arrays,
                                        gcvPOOL_DEFAULT,
                                        &texView.surf));
    }

    /* Check what kind of "copy" we can use. */
    /* Whether resolve can handle the copy? Should be aligned and face==0 */
    if (__GL_TEXTURE_2D_INDEX == texObj->targetIndex)
    {
        tryResolve = gcvTRUE;
    }

    /* Werther shader draw can handle the copy */
    if ((__GL_TEXTURE_2D_INDEX == texObj->targetIndex) &&
        mipmap->formatInfo->renderable &&
        /* Currently no 3Dblit shader for INT sampling. No need check dst for GLcore guarantees. */
        (mipmap->formatInfo->category != GL_INT && mipmap->formatInfo->category != GL_UNSIGNED_INT) &&
        ((chipMipLevel->formatMapInfo->flags & __GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE) == 0) &&
        chipCtx->chipFeature.hwFeature.hasSupertiledTx
        )
    {
        tryShader = gcvTRUE;
    }

    /* Get mip map for specified level. */
    gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, level, &texView.surf));
    texView.firstSlice = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex) ? face : zoffset;
    texView.numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ? texObj->faceMipmap[0][level].depth : texObj->arrays;

    /* Sync RT from texture before use RT. */
    srcView = gcChipFboSyncFromShadowSurface(gc, &chipCtx->readRtView, GL_TRUE);

    if (srcView.surf && srcView.surf->isMsaa)
    {
        if ((!gcChipIsResolvable(srcView.surf->format) ||
             !gcChipIsResolvable(texView.surf->format)))
        {
            tryResolve = gcvFALSE;
        }
        tryShader = gcvFALSE;
    }

    if ((chipCtx->chipModel == gcv600 && chipCtx->chipRevision == 0x4652) &&
        mipmap->formatInfo->drvFormat == __GL_FMT_A8 &&
        srcView.surf->tiling == texView.surf->tiling)
    {
        tryShader = gcvTRUE;
    }

    /* Just to check whether it has only 1 level of mipmap. */
    gcoTEXTURE_GetMipMap(texInfo->object, texObj->params.baseLevel + 1, &mip);

    /* When texture owns mipmap, gcoSURF_DrawBlit path would modify the tiling mode
    ** which could cause mipmap level address and maxLevel incorrect when drawing.
    */
    if (patchId == gcvPATCH_DEQP &&
        gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_MULTI_PIXELPIPES) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SINGLE_BUFFER) &&
        (mip || (texObj->params.sampler.minFilter != GL_NEAREST &&
        texObj->params.sampler.minFilter != GL_LINEAR)))
    {
        tryShader = GL_FALSE;
    }

    /* If tex is bound to FBO and draw before calling CopyTexSubImage2D, shadow surface is dirty.
       Therefore, we should sync tex data from shadow surface. */
    gcmONERROR(gcChipTexSyncFromShadow(gc, gc->state.texture.activeTexIndex, texObj));

    do
    {
        width  = __GL_MIN(width,  (gctINT32)chipCtx->readRTWidth  - x);
        height = __GL_MIN(height, (gctINT32)chipCtx->readRTHeight - y);
        width  = __GL_MIN(width,  mipmap->width  - xoffset);
        height = __GL_MIN(height, mipmap->height - yoffset);

        if (width <= 0 || height <= 0)
        {
            goto OnExit;
        }

        if (tryResolve)
        {
            gcsSURF_RESOLVE_ARGS rlvArgs = {0};

            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.srcOrigin.x = x;
            rlvArgs.uArgs.v2.srcOrigin.y = chipCtx->readYInverted
                                         ? (gctINT) (chipCtx->readRTHeight - y - height) : y;
            rlvArgs.uArgs.v2.dstOrigin.x = xoffset;
            rlvArgs.uArgs.v2.dstOrigin.y = yoffset;
            rlvArgs.uArgs.v2.rectSize.x  = width;
            rlvArgs.uArgs.v2.rectSize.y  = height;
            rlvArgs.uArgs.v2.yInverted = chipCtx->readYInverted;
            rlvArgs.uArgs.v2.numSlices = 1;
            rlvArgs.uArgs.v2.gpuOnly   = gcvTRUE;

            status = gcoSURF_ResolveRect(&srcView, &texView, &rlvArgs);

            if (gcmIS_SUCCESS(status))
            {
                gcoTEXTURE_Flush(texInfo->object);
                break;
            }
        }

        if (tryShader)
        {
            if (gcmIS_ERROR(gcoTEXTURE_RenderIntoMipMap2(texInfo->object,
                                                         level,
                                                         chipMipLevel->shadow[slice].masterDirty)))
            {
                tryShader = gcvFALSE;
            }

            if (gcmIS_ERROR((gcoTEXTURE_GetMipMap(texInfo->object, level, &texView.surf))))
            {
                tryShader = gcvFALSE;
            }
        }

        if (tryShader)
        {
            gscSURF_BLITDRAW_BLIT blitArgs;

            __GL_MEMZERO(&blitArgs, sizeof(blitArgs));
            blitArgs.srcRect.left = x;
            blitArgs.srcRect.top = y;
            blitArgs.srcRect.right = x + width;
            blitArgs.srcRect.bottom = y + height;
            blitArgs.dstRect.left = xoffset;
            blitArgs.dstRect.top = yoffset;
            blitArgs.dstRect.right = xoffset + width;
            blitArgs.dstRect.bottom = yoffset + height;
            blitArgs.filterMode = gcvTEXTURE_POINT;

            blitArgs.yReverse = chipCtx->readYInverted;
            if (chipCtx->readYInverted)
            {
                blitArgs.srcRect.top = (gctINT32)(chipCtx->readRTHeight - (y + height));
                blitArgs.srcRect.bottom = (gctINT32)(chipCtx->readRTHeight - y);
            }

            status = gcoSURF_DrawBlit(&srcView, &texView, &blitArgs);

            if (gcmIS_SUCCESS(status))
            {
                break;
            }
        }

        /* The texture might be drawn before, resolve to texture surface before sub upload */
        gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc, texObj, face, level, zoffset));

        /* Try resolve to temp bitmap then upload. */
        if (gcmIS_SUCCESS(gcChipResolveDrawToTempBitmap(chipCtx,
                                                        &srcView,
                                                        x, y,
                                                        width,
                                                        height))
           )
        {
            gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                                    ? gcvFACE_POSITIVE_X + face
                                    : gcvFACE_NONE;

            /* Upload the texture. */
            gcmONERROR(gcoTEXTURE_UploadSub(texInfo->object,
                                            level,
                                            halFace,
                                            (gctSIZE_T)xoffset, (gctSIZE_T)yoffset,
                                            (gctSIZE_T)width, (gctSIZE_T)height,
                                            zoffset,
                                            chipCtx->tempLastLine,
                                            chipCtx->tempStride,
                                            chipCtx->tempFormat,
                                            gcvSURF_COLOR_SPACE_LINEAR,
                                            gcvINVALID_ADDRESS));

            if (gcmIS_SUCCESS(status))
            {
                break;
            }
        }

        /* Finally, try CPU copy. */
        {
            gcsSURF_BLIT_ARGS blitArgs;
            gcoOS_ZeroMemory(&blitArgs, sizeof(blitArgs));
            blitArgs.srcSurface         = srcView.surf;
            blitArgs.srcX               = x;
            blitArgs.srcY               = y;
            blitArgs.srcZ               = srcView.firstSlice;
            blitArgs.srcWidth           = width;
            blitArgs.srcHeight          = height;
            blitArgs.srcDepth           = 1;
            blitArgs.dstSurface         = texView.surf;
            blitArgs.dstX               = xoffset;
            blitArgs.dstY               = yoffset;
            blitArgs.dstZ               = texView.firstSlice;
            blitArgs.dstWidth           = width;
            blitArgs.dstHeight          = height;
            blitArgs.dstDepth           = 1;
            blitArgs.xReverse           = gcvFALSE;
            blitArgs.scissorTest        = gcvFALSE;
            blitArgs.srcNumSlice        = srcView.firstSlice;
            blitArgs.dstNumSlice        = texView.numSlices;

            if (chipCtx->readYInverted)
            {
                blitArgs.srcY = (gctINT)(chipCtx->readRTHeight - (y + height));
                blitArgs.yReverse = gcvTRUE;
            }

            gcmONERROR(gcoSURF_BlitCPU(&blitArgs));
        }
    }
    while (gcvFALSE);

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
    /*
     * If FBO master is from EGLImage-NativeBuffer, we need block access of the
     * native buffer from other hardware before GPU finishes read.
     */
    gcChipFBOSyncEGLImageNativeBuffer(gc, chipCtx->readRtView.surf, GL_TRUE);
#endif

    if (texInfo->eglImage.image)
    {
        gcmONERROR(gcChipSetImageSrc(texInfo->eglImage.image, texView.surf));
    }

    chipMipLevel->shadow[slice].masterDirty = GL_TRUE;
    CHIP_TEX_IMAGE_UPTODATE(texInfo, level);

OnExit:
OnError:
    gcmFOOTER();
    return status;
}

/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/

/*
** Check if we need shadow as a bridge between read/write
*/
GLboolean
gcChipTexNeedShadow(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    __GLchipTextureInfo *texInfo,
    __GLchipFmtMapInfo *fmtMapInfo,
    GLint samples,
    GLint *samplesUsed
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLboolean need = GL_FALSE;
    khrEGL_IMAGE * eglImage = gcvNULL;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x texInfo=0x%x fmtMapInfo=0x%x samples=%d samplesUsed=0x%x",
                   gc, texObj, texInfo, fmtMapInfo, samples, samplesUsed);

    if (texInfo->eglImage.image)
    {
        eglImage = (khrEGL_IMAGE*)(texInfo->eglImage.image);
    }

    /* For non-native msaa texture, we need shadow rendering/downsample before sampling */
    if ((samples > 1) && (texObj->samplesUsed <= 1))
    {
        need = GL_TRUE;
    }
    else if (eglImage && gcmIS_ERROR(gcoSURF_IsRenderable(eglImage->surface)))
    {
        need = GL_TRUE;
    }
    else if (texInfo->direct.source)
    {
        need = GL_TRUE;
    }
    else if (fmtMapInfo &&
             (fmtMapInfo->flags & (__GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE |
                                   __GL_CHIP_FMTFLAGS_LAYOUT_DIFF_READ_WRITE
                                  )
             )
            )
    {
        need = GL_TRUE;
    }
    else if (chipCtx->chipFeature.hwFeature.hasMultiPixelPipes &&
             !chipCtx->chipFeature.hwFeature.hasSingleBuffer)
    {
        /* If HW required multi tiled/supertiled rt, these rt surface can only
        ** be used as single mipmaped 2D texture.
        */
        if (texObj->targetIndex != __GL_TEXTURE_2D_INDEX &&
            texObj->targetIndex != __GL_TEXTURE_2D_MS_INDEX)
        {
            need = GL_TRUE;
        }
        else if (chipCtx->patchId == gcvPATCH_GTFES30 && texObj->immutable && texObj->immutableLevels > 1)
        {
            /* Always think immutable tex with >1 levels will enable mipmap filtering */
            need = GL_TRUE;
        }
    }

    if (samplesUsed)
    {
        if (samples > 1)
        {
            GLint i;
            for (i = 0; i < fmtMapInfo->numSamples; i++)
            {
                if (fmtMapInfo->samples[i] >= samples)
                {
                    break;
                }
            }

            *samplesUsed = fmtMapInfo->samples[i];
        }
        else
        {
            *samplesUsed = samples;
        }
    }

    gcmFOOTER_ARG("return=%d", need);
    return need;
}

gcsSURF_VIEW
gcChipGetTextureSurface(
    __GLchipContext *chipCtx,
    __GLtextureObject *texObj,
    GLboolean layered,
    GLint level,
    GLint slice
    )
{
    gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;

    static gceTEXTURE_FACE s_texFaceIndex2HAL[] =
    {
        gcvFACE_POSITIVE_X,     /* face 0 */
        gcvFACE_NEGATIVE_X,     /* face 1 */
        gcvFACE_POSITIVE_Y,     /* face 2 */
        gcvFACE_NEGATIVE_Y,     /* face 3 */
        gcvFACE_POSITIVE_Z,     /* face 4 */
        gcvFACE_NEGATIVE_Z,     /* face 5 */
    };

    gcmHEADER_ARG("chipCtx=0x%x texObj=0x%x level=%d slice=%d", chipCtx, texObj, level, slice);

    if (!texInfo || !texInfo->object)
    {
        goto OnError;
    }

    switch (texObj->targetIndex)
    {
    case __GL_TEXTURE_2D_INDEX:
    case __GL_TEXTURE_2D_MS_INDEX:
        if (slice != 0)
        {
            goto OnError;
        }
        gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, level, &surfView.surf));
        surfView.numSlices = 1;
        break;

    case __GL_TEXTURE_CUBEMAP_INDEX:
        if (slice < 6)
        {
            surfView.firstSlice = slice;
            surfView.numSlices = texObj->arrays;
            gcmONERROR(gcoTEXTURE_GetMipMapFace(texInfo->object,
                                                level,
                                                s_texFaceIndex2HAL[slice],
                                                &surfView.surf,
                                                gcvNULL));
        }
        break;
    case __GL_TEXTURE_3D_INDEX:
    case __GL_TEXTURE_2D_ARRAY_INDEX:
    case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
    case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
        surfView.firstSlice = slice;
        gcmONERROR(gcoTEXTURE_GetMipMapSlice(texInfo->object,
                                             level,
                                             slice,
                                             &surfView.surf,
                                             gcvNULL));
        surfView.numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ? texObj->faceMipmap[0][level].depth : texObj->arrays;
        break;
    default:
        GL_ASSERT(0);
    }

    if (!layered)
    {
        surfView.numSlices = 1;
    }
    else
    {
        surfView.firstSlice = 0;
    }

OnError:
    gcmFOOTER_ARG("return={0x%x, %d, %d}", surfView.surf, surfView.firstSlice, surfView.numSlices);
    return surfView;
}


GLvoid
__glChipBindTexture(
    __GLcontext* gc,
    __GLtextureObject *texObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    gcmONERROR(gcChipCreateTexture(gc, texObj));

    gcmFOOTER_NO();
    return;
OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_NO();
    return;

}

GLvoid
__glChipDeleteTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
#if __GL_CHIP_PATCH_ENABLED
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
#endif
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    GLuint level, slice;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    if (!texInfo)
    {
        gcmFOOTER_NO();
        return;
    }

    /* default texture does not create the array */
    GL_ASSERT(texInfo->mipLevels);
    for (level = 0; level < texObj->maxLevels; ++level)
    {
        __GLchipMipmapInfo *mipLevel = &texInfo->mipLevels[level];
        for (slice = 0; slice < texObj->maxSlices; ++slice)
        {
            if (mipLevel->shadow[slice].surface)
            {
                gcmVERIFY_OK(gcoSURF_Destroy(mipLevel->shadow[slice].surface));
                mipLevel->shadow[slice].surface = gcvNULL;
            }
        }

        if (mipLevel->stencilOpt)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)mipLevel->stencilOpt));
            mipLevel->stencilOpt = gcvNULL;
        }

        if (mipLevel->astcSurf)
        {
            gcmVERIFY_OK(gcoSURF_Unlock(mipLevel->astcSurf, (gctPOINTER)mipLevel->astcData));
            gcmVERIFY_OK(gcoSURF_Destroy(mipLevel->astcSurf));
            mipLevel->astcSurf = gcvNULL;
        }
        else if (mipLevel->astcData)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)mipLevel->astcData));
            mipLevel->astcData = gcvNULL;
        }
    }
    gc->imports.free(gc, texInfo->mipLevels);
    texInfo->mipLevels = gcvNULL;


#if __GL_CHIP_PATCH_ENABLED
    if (chipCtx->patchId == gcvPATCH_GTFES30 && texObj->immutable)
    {
        __GLmipMapLevel *baseMip = &texObj->faceMipmap[0][0];

        if (texObj->targetIndex == __GL_TEXTURE_2D_INDEX &&
            texObj->immutableLevels == 1 &&
            (GLuint)baseMip->width == gc->constants.maxTextureSize &&
            baseMip->height == 1 &&
            (baseMip->requestedFormat == GL_R8 || baseMip->requestedFormat == GL_RGB565)
           )
        {
            chipCtx->patchInfo.commitTexDelete = gcvTRUE;
        }
        else if (texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX &&
                 baseMip->height == 1 &&
                 ((texObj->immutableLevels == 1 &&
                   baseMip->requestedFormat == GL_R8 &&
                   baseMip->width == 1 &&
                   (GLuint)baseMip->arrays == gc->constants.maxTextureArraySize
                  ) ||
                  ((GLuint)texObj->immutableLevels == gc->constants.maxNumTextureLevels &&
                   baseMip->requestedFormat == GL_RGB565 &&
                   (GLuint)baseMip->width == gc->constants.maxTextureSize &&
                   baseMip->arrays == 1
                  )
                 )
                )
        {
            chipCtx->patchInfo.commitTexDelete = gcvFALSE;
        }
    }
#endif

    gcmVERIFY_OK(gcChipResetTextureWrapper(gc, texObj));

    /* Destroy the texture object. */
    if (texInfo->object != gcvNULL)
    {
        gcmVERIFY_OK(gcoTEXTURE_Destroy(texInfo->object));
        texInfo->object = gcvNULL;
    }

#if __GL_CHIP_PATCH_ENABLED
    /* Commit, so that the coming destroy can free memory immediately. */
    if (chipCtx->patchInfo.commitTexDelete)
    {
        gcmVERIFY_OK(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
    }
#endif

    gc->imports.free(gc, texObj->privateData);
    texObj->privateData = gcvNULL;

    gcmFOOTER_NO();
}

/*
** Detach texture chip/HAL object reference from context when application try to delete it.
** NOTE: texture probably still exist in other context.
*/
GLvoid
__glChipDetachTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    gcoSURF *surfList = gcvNULL;
    gcoSURF surf;
    GLuint  surfCount = 0;
    GLuint  level, slice;
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    if (!texInfo || !texInfo->object)
    {
        gcmFOOTER_NO();
        return;
    }

    surfList = (gcoSURF*)(*gc->imports.calloc)(gc, __GL_CHIP_SURF_COUNT, sizeof(GLuint*));

    /* collect all surface which could be RT surface*/
    /* step1: collect shadow surface if exist */
    GL_ASSERT(texInfo->mipLevels);
    for (level = 0; level < texObj->maxLevels; ++level)
    {
        __GLchipMipmapInfo *mipLevel = &texInfo->mipLevels[level];
        for (slice = 0; slice < texObj->maxSlices; ++slice)
        {
            if (mipLevel->shadow[slice].surface)
            {
                surfList[surfCount++] = mipLevel->shadow[slice].surface;
            }
        }
    }

    /* collect mipmap surface from HAL */
    for (level = 0; level < texObj->maxLevels; ++level)
    {
        status = gcoTEXTURE_GetMipMap(texInfo->object, level, &surf);
        if ((status == gcvSTATUS_OK) && surf)
        {
            surfList[surfCount++] = surf;
        }
    }

    GL_ASSERT(surfCount <= __GL_CHIP_SURF_COUNT);

    if (surfCount)
    {
        gcChipDetachSurface(gc, chipCtx, surfList, surfCount);
    }

    (*gc->imports.free)(gc, surfList);

    gcmFOOTER_NO();
    return;
}


GLboolean
__glChipTexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d buf=0x%x", gc, texObj, face, level, buf);

    gcmONERROR(gcChipResidentTextureLevel(gc, chipCtx, texObj, face, level, buf));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLboolean
__glChipTexImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid *buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x level=%d buf=0x%x", gc, texObj, level, buf);

    gcmONERROR(gcChipResidentTextureLevel(gc, chipCtx, texObj, 0, level, buf));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipTexSubImage2D(
        __GLcontext *gc,
        __GLtextureObject *texObj,
        GLint face,
        GLint level,
        GLint xoffset,
        GLint yoffset,
        GLint width,
        GLint height,
        const GLvoid* buf
        )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d xoffset=%d yoffset=%d "
                  "width=%d height=%d buf=0x%x",
                  gc, texObj, face, level, xoffset, yoffset, width, height, buf);

    gcmONERROR(gcChipTexSubImage(gc, texObj, face, level, xoffset, yoffset, 0, width, height, 1, buf));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipTexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid* buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x level=%d xoffset=%d yoffset=%d zoffset=%d"
                   "width=%d height=%d depth=%d buf=0x%x",
                   gc, texObj, level, xoffset, yoffset, zoffset, width, height, depth, buf);

    gcmONERROR(gcChipTexSubImage(gc,
                                   texObj,
                                   0,
                                   level,
                                   xoffset,
                                   yoffset,
                                   zoffset,
                                   width,
                                   height,
                                   depth,
                                   buf));


    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipCopyTexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x,
    GLint y
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d x=%x y=%d",
                   gc, texObj, face, level, x, y);

    gcmONERROR(gcChipCopyTexImage(gc, texObj, face, level, x, y));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipCopyTexSubImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint x, GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d x=%x y=%d width=%d hieght=%d xoffset=%d yoffset=%d",
                   gc, texObj, face, level, x, y, width, height, xoffset, yoffset);

    gcmONERROR(gcChipCopyTexSubImage(gc, texObj, face, level, x, y, xoffset, yoffset, 0, width, height));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipCopyTexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint x,
    GLint y,
    GLint width,
    GLint height,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x level=%d x=%x y=%d width=%d hieght=%d"
                  "xoffset=%d yoffset=%d zoffset=%d",
                   gc, texObj, level, x, y, width, height, xoffset, yoffset, zoffset);

    gcmONERROR(gcChipCopyTexSubImage(gc, texObj, 0, level, x, y, xoffset, yoffset, zoffset, width, height));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipCompressedTexImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    const GLvoid *buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d buf=0x%x", gc, texObj, face, level, buf);

    gcmONERROR(gcChipResidentTextureLevel(gc, chipCtx, texObj, face, level, buf));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipCompressedTexImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    const GLvoid *buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x level=%d buf=0x%x", gc, texObj, level, buf);

    gcmONERROR(gcChipResidentTextureLevel(gc, chipCtx, texObj, 0, level, buf));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

static gceSTATUS
gcChipCompressedTexSubImage(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid *buf,
    GLsizei size
    )
{
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    __GLmipMapLevel *mipmap = &texObj->faceMipmap[face][level];
    gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                            ? gcvFACE_POSITIVE_X + face : gcvFACE_NONE;
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF mipSurf;
    gctSIZE_T rowStride = 0;

    const GLvoid *pixels = gcvNULL;
    GLboolean needDecompress = GL_FALSE;
    gceSURF_FORMAT texImageFormat = gcvSURF_UNKNOWN;
    __GLbufferObject *unpackBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_UNPACK_BUFFER_INDEX].boundBufObj;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d xoffset=%d yoffset=%d"
                  "zoffset=%d width=%d height=%d depth=%d buf=0x%x size=%d",
                   gc, texObj, face, level, xoffset, yoffset, zoffset,
                   width, height, depth, buf, size);

    /* Get mip map for specified level. */
    gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, level, &mipSurf));

    if (unpackBufObj)
    {
        gcmONERROR(gcChipProcessPBO(gc, unpackBufObj, &buf));
    }

    if (buf)
    {
        GLint i, j;
        GLsizei sliceSize = size / depth;
        __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[level];
        __GLchipFmtMapInfo *formatMapInfo = chipMipLevel->formatMapInfo;

        GL_ASSERT((formatMapInfo->flags & __GL_CHIP_FMTFLAGS_CANT_FOUND_HAL_FORMAT) == GL_FALSE);
        GL_ASSERT(formatMapInfo->readFormat != gcvSURF_UNKNOWN);

        if ((formatMapInfo->flags & (__GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ|
                                     __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_READ))
            && mipmap->compressed)
        {
            needDecompress = GL_TRUE;
        }

        /* Check format support to see whether decompression is needed. */
        switch (mipmap->requestedFormat)
        {
        case GL_ETC1_RGB8_OES:
            if (needDecompress)
            {
                GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                /* Decompress ETC1 texture since hardware doesn't support it. */
                pixels = gcChipDecompressETC1(gc,
                                              (gctSIZE_T)width,
                                              (gctSIZE_T)height,
                                              (gctSIZE_T)size,
                                              buf,
                                              &texImageFormat,
                                              &rowStride);
            }
            break;

         case GL_COMPRESSED_R11_EAC:
         case GL_COMPRESSED_SIGNED_R11_EAC:
         case GL_COMPRESSED_RG11_EAC:
         case GL_COMPRESSED_SIGNED_RG11_EAC:
            if (needDecompress)
            {
                GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                /* Decompress. */
                pixels = gcChipDecompress_EAC_11bitToR16F(gc,
                                                          (gctSIZE_T)width,
                                                          (gctSIZE_T)height,
                                                          (gctSIZE_T)depth,
                                                          (gctSIZE_T)size,
                                                          buf,
                                                          mipmap->requestedFormat,
                                                          &texImageFormat,
                                                          &rowStride);
            }
            break;

         case GL_COMPRESSED_RGB8_ETC2:
         case GL_COMPRESSED_SRGB8_ETC2:
         case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
         case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
         case GL_COMPRESSED_RGBA8_ETC2_EAC:
         case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            if (needDecompress)
            {
                GL_ASSERT(GL_FALSE);
            }
            break;

        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            if (needDecompress)
            {
                GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                pixels = gcChipDecompressDXT(gc,
                                             (gctSIZE_T)width,
                                             (gctSIZE_T)height,
                                             (gctSIZE_T)size,
                                             buf,
                                             mipmap->requestedFormat,
                                             &texImageFormat,
                                             &rowStride);
            }
            break;
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
            {
                GL_ASSERT(needDecompress);

                GL_ASSERT(!__GL_IS_TEXTURE_ARRAY(texObj->targetIndex));

                pixels = gcChipDecompressPalette(gc,
                                                 mipmap->requestedFormat,
                                                 (gctSIZE_T)width,
                                                 (gctSIZE_T)height,
                                                 level,
                                                 (gctSIZE_T)size,
                                                 buf,
                                                 &texImageFormat,
                                                 &rowStride);
                break;
            }


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
            /* if texture is 2d array or cube map and gcvFEATURE_TX_ASTC_MULTISLICE_FIX is false, should decompress, */
            if (needDecompress)
            {
                GLubyte *src;
                GLubyte *dst;
                GLint blockWidth  = mipmap->formatInfo->blockWidth;
                GLint blockHeight = mipmap->formatInfo->blockHeight;
                GLuint xOffBlock, yOffBlock;
                GLuint xBlock, yBlock;
                GLuint widthBlock;
                GLuint srcStride, dstStride;

                pixels = gcChipDecompressASTC(gc,
                                              (gctSIZE_T)width,
                                              (gctSIZE_T)height,
                                              (gctSIZE_T)depth,
                                              (gctSIZE_T)sliceSize,
                                              buf,
                                              mipmap->formatInfo,
                                              &texImageFormat,
                                              &rowStride);

                xOffBlock   = (xoffset + blockWidth - 1) / blockWidth;
                yOffBlock   = (yoffset + blockHeight - 1) / blockHeight;
                widthBlock  = (mipmap->width + blockWidth - 1) / blockWidth;
                xBlock      = (width + blockWidth - 1) / blockWidth;
                yBlock      = (height + blockHeight - 1) / blockHeight;
                srcStride   = xBlock * 16;
                dstStride   = widthBlock * 16;

                if (chipMipLevel->astcData)
                {
                    for (i = 0; i < depth; ++i)
                    {
                        src = (GLubyte*) buf + sliceSize * i;
                        dst = chipMipLevel->astcData + (zoffset + face + i) * mipmap->compressedSize + (yOffBlock * widthBlock + xOffBlock) * 16;
                        for (j = 0; j < (GLint) yBlock; j++)
                        {
                            gcoOS_MemCopy(dst, src, srcStride);
                            /* Advance to next line. */
                            src += srcStride;
                            dst += dstStride;
                        }
                    }
                }
                else
                {
                    gcmONERROR(gcvSTATUS_INVALID_ADDRESS);
                }
            }
            break;

        default:
            pixels = gcvNULL;
            break;
        }

        for (i = 0; i < depth; ++i)
        {
            gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc, texObj, face, level, zoffset + i));

            if (pixels == gcvNULL)
            {
                /* Upload compressed if hw supports. */
                gcmONERROR(gcoTEXTURE_UploadCompressedSub(texInfo->object,
                                                          level,
                                                          halFace,
                                                          (gctSIZE_T)xoffset, (gctSIZE_T)yoffset,
                                                          (gctSIZE_T)width, (gctSIZE_T)height,
                                                          zoffset + i,
                                                          (const GLvoid*)((GLubyte*)buf + (i * sliceSize)),
                                                          sliceSize));
            }
            else
            {
                gcmASSERT(texImageFormat != gcvSURF_UNKNOWN);

                /* Upload uncompressed if hw does not support. */
                gcmONERROR(gcoTEXTURE_UploadSub(texInfo->object,
                                                level,
                                                halFace,
                                                (gctSIZE_T)xoffset, (gctSIZE_T)yoffset,
                                                (gctSIZE_T)width, (gctSIZE_T)height,
                                                zoffset + i,
                                                (const GLvoid*)((GLubyte*)pixels + rowStride * height * i),
                                                rowStride,
                                                texImageFormat,
                                                gcvSURF_COLOR_SPACE_LINEAR,
                                                gcvINVALID_ADDRESS));
            }

            chipMipLevel->shadow[face + zoffset + i].masterDirty = GL_TRUE;
        }

        CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
    }

    if (unpackBufObj) /* The image is from unpack buffer object */
    {
        gcmONERROR(gcChipPostProcessPBO(gc, unpackBufObj, GL_FALSE));
    }

OnError:
    if (pixels)
    {
        gcoOS_Free(gcvNULL, (gctPOINTER)pixels);
    }

    gcmFOOTER();
    return status;
}


GLboolean
__glChipCompressedTexSubImage2D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint width,
    GLint height,
    const GLvoid *buf,
    GLsizei size
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d level=%d xoffset=%d yoffset=%d"
                  "width=%d height=%d buf=0x%x size=%d",
                   gc, texObj, face, level, xoffset, yoffset, width, height, buf, size);

    gcmONERROR(gcChipCompressedTexSubImage(gc, texObj, face, level, xoffset, yoffset, 0, width, height, 1, buf, size));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipCompressedTexSubImage3D(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint level,
    GLint xoffset,
    GLint yoffset,
    GLint zoffset,
    GLint width,
    GLint height,
    GLint depth,
    const GLvoid *buf,
    GLsizei size
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x level=%d xoffset=%d yoffset=%d zoffset=%d"
                  "width=%d height=%d depth=%d buf=0x%x size=%d",
                   gc, texObj, level, xoffset, yoffset, zoffset, width, height, depth, buf, size);

    gcmONERROR(gcChipCompressedTexSubImage(gc, texObj, 0, level, xoffset, yoffset, zoffset, width, height, depth, buf, size));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}


GLboolean
__glChipGenerateMipMap(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint faces,
    GLint *maxLevel
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    GLint level, lastLevel = -1;
    GLint baseLevel = texObj->params.baseLevel;
    __GLmipMapLevel *baseMipmap = &texObj->faceMipmap[0][baseLevel];
    __GLchipMipmapInfo *chipMipLevel = gcvNULL;
    __GLchipFmtPatch patchCase = __GL_CHIP_FMT_PATCH_NONE;
    gcoSURF surface = gcvNULL;
    gctBOOL splitTexture = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x texObj=0x%x faces=%d maxLevel=0x%x",
                   gc, texObj, faces, maxLevel);

    if (__GL_TEXTURE_EXTERNAL_INDEX == texObj->targetIndex)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (!texInfo)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    if ((texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX) ||
        (texObj->targetIndex == __GL_TEXTURE_3D_INDEX))
    {
        patchCase = __GL_CHIP_FMT_PATCH_CASE0;
    }

    gcmONERROR(gcChipOrphanTexMipmap(gc, chipCtx, texObj));

    /* Construct the gcoTEXTURE object. */
    if (texInfo->object == gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal,
                                          __glChipTexTargetToHAL[texObj->targetIndex],
                                          &texInfo->object));

        gcmONERROR(gcoTEXTURE_SetEndianHint(texInfo->object,
                                            gcChipUtilGetEndianHint(baseMipmap->requestedFormat,
                                                                    baseMipmap->type)));
    }

    chipMipLevel = &texInfo->mipLevels[baseLevel];

    if (CHIP_TEX_IMAGE_IS_UPTODATE(texInfo, baseLevel) == 0)
    {
        chipMipLevel->formatMapInfo = gcChipGetFormatMapInfo(gc, baseMipmap->formatInfo->drvFormat, patchCase);

        /* Add the base level. If it already exists, the call will be ignored. */
        gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                                        baseLevel,
                                        baseMipmap->requestedFormat,
                                        chipMipLevel->formatMapInfo->readFormat,
                                        (gctSIZE_T)baseMipmap->width,
                                        (gctSIZE_T)baseMipmap->height,
                                        __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? texObj->arrays : baseMipmap->depth,
                                        __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? 1 : texObj->arrays,
                                        gcvPOOL_DEFAULT,
                                        gcvNULL));

        GL_ASSERT(chipCtx->needStencilOpt == GL_FALSE);

        __GLES_PRINT("ES30: generating mipmap but baselevel has no valid data. garbage is expected for other levels ");
    }

    /* When generate mips, we should sync texture surface from corresponding RT surface. */
    gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc, texObj, 0, baseLevel, 0));

    gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, baseLevel, &surface));

    if (surface)
    {
        gceTILING tiling = 0;

        gcmONERROR(gcoSURF_GetTiling(surface,&tiling));

        if (tiling & gcvTILING_SPLIT_BUFFER)
        {
            splitTexture = gcvTRUE;
        }
    }

    for (level = baseLevel + 1; level <= *maxLevel; ++level)
    {
        gcoSURF srcSurface = gcvNULL, dstSurface = gcvNULL;
        __GLmipMapLevel *mipmap = &texObj->faceMipmap[0][level];
        GLint numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ? mipmap->depth : texObj->arrays;
        __GLchipMipmapInfo *chipMipBaseLevel = &texInfo->mipLevels[baseLevel];
        GLint slice;

        chipMipLevel = &texInfo->mipLevels[level];

        chipMipLevel->formatMapInfo = gcChipGetFormatMapInfo(gc, baseMipmap->formatInfo->drvFormat, patchCase);

        /* Get the texture surface. */
        gcmONERROR(gcoTEXTURE_GetMipMap(texInfo->object, level - 1, &srcSurface));

        /* Create a new level. */
        gcmONERROR(gcoTEXTURE_AddMipMapEx(texInfo->object,
                                          level,
                                          baseMipmap->requestedFormat,
                                          chipMipBaseLevel->formatMapInfo->readFormat,
                                          (gctSIZE_T)mipmap->width,
                                          (gctSIZE_T)mipmap->height,
                                          __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? texObj->arrays : mipmap->depth,
                                          __GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ? 1 : texObj->arrays,
                                          gcvPOOL_DEFAULT,
                                          0,
                                          surface->hints & gcvSURF_PROTECTED_CONTENT ? gcvTRUE : gcvFALSE,
                                          &dstSurface));

        /* only base level has valid data, we need to generate data for other mips */
        /* For split texture, we don't need the real data of other mips. Fix bug #8768 */
        if (CHIP_TEX_IMAGE_IS_UPTODATE(texInfo, baseLevel) && !splitTexture)
        {
            /* For blit engine, we generate mipmap in one shot */
            if (chipCtx->chipFeature.hwFeature.hasBlitEngine)
            {
                if (level == *maxLevel)
                {
                    gcmONERROR(gcoTEXTURE_GenerateMipMap(texInfo->object, baseLevel, level));
                }
            }
            else
            {
                if (gcmIS_ERROR(gcoSURF_Resample(srcSurface, dstSurface)))
                {
                    /* record last succeeded level. */
                    if (lastLevel == -1)
                    {
                        lastLevel = level - 1;
                    }
                }
            }

#if gcdSYNC
            gcmONERROR(gcoSURF_GetFence(dstSurface, gcvFENCE_TYPE_ALL));
#endif
            for (slice = 0; slice < numSlices; ++slice)
            {
                chipMipLevel->shadow[slice].masterDirty = GL_TRUE;
            }

            CHIP_TEX_IMAGE_UPTODATE(texInfo, level);
        }

        if (chipCtx->needStencilOpt)
        {
            if (mipmap->formatInfo->stencilSize > 0)
            {
                if (!chipMipLevel->stencilOpt)
                {
                    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                              numSlices * gcmSIZEOF(__GLchipStencilOpt),
                                              (gctPOINTER*)&chipMipLevel->stencilOpt));
                }

                for (slice = 0; slice < numSlices; ++slice)
                {
                    gcChipPatchStencilOptReset(&chipMipLevel->stencilOpt[slice],
                                               (gctSIZE_T)mipmap->width,
                                               (gctSIZE_T)mipmap->height,
                                               (gctSIZE_T)mipmap->formatInfo->stencilSize);
                }
            }
            else
            {
                if (chipMipLevel->stencilOpt)
                {
                    gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)chipMipLevel->stencilOpt));
                    chipMipLevel->stencilOpt = gcvNULL;
                }
            }
        }
    }

    if (lastLevel != -1)
    {
        *maxLevel = lastLevel;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcoHAL_SetHardwareType(chipCtx->hal, gcvHARDWARE_3D);
    CHIP_TEX_IMAGE_OUTDATE_ALL(texInfo);

    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipGetTexImage(
    __GLcontext* gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLubyte *buf
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLclientPixelState *ps = &gc->clientState.pixel;
    gctUINT slice = face;
    __GLmipMapLevel *mipmap = &texObj->faceMipmap[face][level];
    gcsSURF_VIEW srcView = { gcvNULL, 0, 1 };
    gcsSURF_VIEW dstView = { gcvNULL, 0, 1 };
    __GLbufferObject *packBufObj = gcvNULL;
    __GLchipVertexBufferInfo *packBufInfo = gcvNULL;
    gctUINT32 physicalAddress = gcvINVALID_ADDRESS;
    gctPOINTER logicalAddress = buf;
    gcsSURF_RESOLVE_ARGS rlvArgs = { 0 };
    gctSIZE_T rowStride = 0, imgHeight = 0, skipOffset = 0;
    gctSIZE_T skipImgs = (__GL_IS_TEXTURE_ARRAY(texObj->targetIndex) ||
        texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ?
        (gctSIZE_T)gc->clientState.pixel.packModes.skipImages : 0;
    GLuint i;
    GLenum format = mipmap->format;
    gceSURF_FORMAT wrapformat = gcvSURF_UNKNOWN;
    __GLformatInfo *formatInfo = mipmap->formatInfo;
    gctUINT w, h;
    gctUINT dstWidth, dstHeight;
    gctUINT numSlice = 0;
    GLuint lineLength = ps->packModes.lineLength ? ps->packModes.lineLength : (GLuint)mipmap->width;
    GLuint imageHeight = ps->packModes.imageHeight ? ps->packModes.imageHeight : (GLuint)mipmap->height;
    gceSTATUS status = gcvSTATUS_OK;


    gcmHEADER_ARG("gc=0x%x, texObj=0x%x, face=%d, level=%d, buf=0x%x",
        gc, texObj, face, level, buf);

    srcView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, level, slice);

    switch (mipmap->type)
    {
    case GL_UNSIGNED_BYTE:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A8B8G8R8;
        }
        else if (format == GL_BGRA_EXT)
        {
            wrapformat = gcvSURF_A8R8G8B8;
        }
        break;
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A2B10G10R10;
        }
        break;
    case GL_FLOAT:
        if (format == GL_RGBA)
        {
            wrapformat = gcvSURF_A32B32G32R32F;
        }
        break;
    case GL_UNSIGNED_INT:
        if (format == GL_RGBA_INTEGER)
        {
            wrapformat = gcvSURF_A32B32G32R32UI;
        }
        break;
    case GL_INT:
        if (format == GL_RGBA_INTEGER)
        {
            wrapformat = gcvSURF_A32B32G32R32I;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
        {
            wrapformat = gcvSURF_A4R4G4B4;
        }
        break;
    case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
        {
            wrapformat = gcvSURF_A1R5G5B5;
        }
        break;
    default:
        break;
    }

    if (formatInfo == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (gcvSURF_UNKNOWN == wrapformat)
    {
        __GLchipFmtMapInfo *formatMapInfo = gcChipGetFormatMapInfo(gc, formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);
        wrapformat = formatMapInfo->requestFormat;
    }

    gcChipProcessPixelStore(gc,
        &ps->packModes,
        (gctSIZE_T)mipmap->width,
        (gctSIZE_T)mipmap->height,
        mipmap->format,
        mipmap->type,
        skipImgs,
        &rowStride,
        &imgHeight,
        &skipOffset);

    /* The image is from pack buffer object? */
    packBufObj = gc->bufferObject.generalBindingPoint[__GL_PIXEL_PACK_BUFFER_INDEX].boundBufObj;
    if (packBufObj)
    {
        packBufInfo = (__GLchipVertexBufferInfo *)(packBufObj->privateData);
        GL_ASSERT(packBufInfo);
        gcmONERROR(gcoBUFOBJ_Lock(packBufInfo->bufObj, &physicalAddress, &logicalAddress));
        gcmONERROR(gcoBUFOBJ_GetFence(packBufInfo->bufObj, gcvFENCE_TYPE_WRITE));

        skipOffset += __GL_PTR2SIZE(buf);
        physicalAddress += (gctUINT32)skipOffset;
    }
    logicalAddress = (gctPOINTER)((gctINT8_PTR)logicalAddress + skipOffset);

    switch (texObj->targetIndex)
    {
    case __GL_TEXTURE_2D_INDEX:
    case __GL_TEXTURE_2D_MS_INDEX:
    case __GL_TEXTURE_CUBEMAP_INDEX:
        numSlice = 1;
        break;
    case __GL_TEXTURE_3D_INDEX:
    case __GL_TEXTURE_2D_ARRAY_INDEX:
    case __GL_TEXTURE_2D_MS_ARRAY_INDEX:
    case __GL_TEXTURE_CUBEMAP_ARRAY_INDEX:
        numSlice = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ? texObj->faceMipmap[face][level].depth : texObj->arrays;
        break;
    default:
        GL_ASSERT(0);
    }

    for (i = 0; i < numSlice; ++i)
    {
        gctPOINTER dstLogicalAddress;
        srcView.firstSlice = (texObj->targetIndex == __GL_TEXTURE_CUBEMAP_INDEX) ? srcView.firstSlice : i;

        dstLogicalAddress = (gctPOINTER)((GLbyte*)logicalAddress + rowStride * imgHeight * i);
        physicalAddress = (physicalAddress == gcvINVALID_ADDRESS) ? gcvINVALID_ADDRESS : physicalAddress + (gctUINT32)(rowStride * imgHeight * i);

        /* Create the wrapper surface. */
        gcmONERROR(gcoSURF_Construct(gcvNULL, mipmap->width, mipmap->height, 1, gcvSURF_BITMAP,
            wrapformat, gcvPOOL_USER, &dstView.surf));
        gcmONERROR(gcoSURF_ResetSurWH(dstView.surf, mipmap->width, mipmap->height, lineLength, imageHeight, wrapformat));

        gcmONERROR(gcoSURF_WrapSurface(dstView.surf, ps->packModes.alignment, dstLogicalAddress, physicalAddress));
        gcmONERROR(gcoSURF_GetSize(srcView.surf, &w, &h, gcvNULL));
        gcmONERROR(gcoSURF_GetSize(dstView.surf, &dstWidth, &dstHeight, gcvNULL));

        /*
        ** Set Non-Linear space for SRGB8_ALPHA8
        */
        if (formatInfo->drvFormat == __GL_FMT_SRGB8_ALPHA8)
        {
            gcmONERROR(gcoSURF_SetColorSpace(dstView.surf, gcvSURF_COLOR_SPACE_NONLINEAR));
        }

        do {
            rlvArgs.version = gcvHAL_ARG_VERSION_V2;
            rlvArgs.uArgs.v2.yInverted = gcvFALSE;
            rlvArgs.uArgs.v2.srcOrigin.x = 0;
            rlvArgs.uArgs.v2.srcOrigin.y = 0;
            rlvArgs.uArgs.v2.dstOrigin.x = 0;
            rlvArgs.uArgs.v2.dstOrigin.y = 0;
            rlvArgs.uArgs.v2.rectSize.x = gcmMIN(mipmap->width, (gctINT)w);
            rlvArgs.uArgs.v2.rectSize.y = gcmMIN(mipmap->height, (gctINT)h);
            rlvArgs.uArgs.v2.numSlices = 1;
            rlvArgs.uArgs.v2.dump = gcvTRUE;

            if (packBufObj)
            {
                if (gcmIS_SUCCESS(gcoSURF_ResolveRect(&srcView, &dstView, &rlvArgs)))
                {
                    break;
                }
            }
            gcmERR_BREAK(gcoSURF_CopyPixels(&srcView, &dstView, &rlvArgs));
        } while (gcvFALSE);

        if (dstView.surf)
        {
            gcmONERROR(gcoSURF_Destroy(dstView.surf));
            dstView.surf = gcvNULL;
        }
    }

OnError:
    if (packBufInfo && gcvINVALID_ADDRESS != physicalAddress) /* The image is from pack buffer object */
    {
        /* CPU cache will not be flushed in HAL, bc HAL only see wrapped user pool surface.
        ** Instead it will be flushed when unlock the packed buffer as non-user pool node.
        */
        gcmVERIFY_OK(gcoBUFOBJ_Unlock(packBufInfo->bufObj));
        gcmVERIFY_OK(gcoBUFOBJ_CPUCacheOperation(packBufInfo->bufObj, gcvCACHE_CLEAN));
    }

    if (dstView.surf)
    {
        gcoSURF_Destroy(dstView.surf);
    }

    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }
    else
    {
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }
}

GLboolean
__glChipCopyTexBegin(
    __GLcontext* gc
    )
{
    return GL_TRUE;
}

GLvoid
__glChipCopyTexValidateState(
    __GLcontext* gc
    )
{

}

GLvoid
__glChipCopyTexEnd(
    __GLcontext* gc
    )
{

}

GLboolean
__glChipBindTexImage(
    IN  __GLcontext *gc,
    IN  __GLtextureObject *texObj,
    IN  GLint level,
    IN  void * surface,
    OUT void ** pBinder
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    gcsSURF_VIEW texView = {gcvNULL, 0, 1};
    GLboolean ret;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x, texObj=0x%x, level=%d, surface=0x%x, pBinder=0x%x",
                   gc, texObj, level, surface, pBinder);

    /* Destroy the direct source.*/
    gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

    /* Destroy the texture */
    if (texInfo->object)
    {
        gcmONERROR(gcoTEXTURE_Destroy(texInfo->object));
        texInfo->object = gcvNULL;
    }

    if (surface)
    {
        gcsSURF_VIEW surfView = {(gcoSURF)surface, 0, 1};
        __GLmipMapLevel *mipmap = &texObj->faceMipmap[0][level];
        __GLchipFmtMapInfo *formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

        /* Create a new texture object. */
        gcmONERROR(gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));

        /*
        ** Create the mipmap that the render target will resolve to.
        ** We should bind the incoming surface directly to the texture for the
        ** first time and and do a resolve when necessary to sync.
        */
        gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                                        level,
                                        mipmap->format,
                                        formatMapInfo->readFormat,
                                        (gctSIZE_T)mipmap->width,
                                        (gctSIZE_T)mipmap->height,
                                        1,
                                        1,
                                        gcvPOOL_DEFAULT,
                                        &texView.surf));

        /* Resolve surface to texture mipmap: no flip in the resolve. */
        gcmONERROR(gcoSURF_ResolveRect(&surfView, &texView, gcvNULL));
    }

    if (pBinder != gcvNULL)
    {
        *pBinder = texView.surf;
    }

OnError:
    ret = gcmIS_SUCCESS(status) ? GL_TRUE : GL_FALSE;
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLvoid
__glChipFreeTexImage(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level
    )
{

}

GLboolean
__glChipTexDirectVIV(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint width,
    GLint height,
    GLenum format,
    GLvoid **pixels
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    gctBOOL sourceYuv = gcvFALSE;
    gctBOOL planarYuv = gcvFALSE;
    __GLmipMapLevel     *mipmap;
    gceSURF_FORMAT sourceFormat = 0, textureFormat = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER memory[3] = {gcvNULL};

    gcmHEADER_ARG("gc=0x%x texObj=0x%x width=%d height=%d format=0x%04x pixels=0x%x",
                   gc, texObj, width, height, format, pixels);

    /* Translate format information. */
    switch (format)
    {
    case GL_VIV_YV12:
        sourceFormat = gcvSURF_YV12;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_I420:
        sourceFormat = gcvSURF_I420;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_NV12:
        sourceFormat = gcvSURF_NV12;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_NV21:
        sourceFormat = gcvSURF_NV21;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_YUY2:
        sourceFormat = gcvSURF_YUY2;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvFALSE;
        break;

    case GL_VIV_UYVY:
        sourceFormat = gcvSURF_UYVY;
        textureFormat = gcvSURF_UYVY;
        sourceYuv = gcvTRUE;
        planarYuv = gcvFALSE;
        break;

    case GL_VIV_YUV420_10_ST:
        sourceFormat  = gcvSURF_YUV420_10_ST;
        textureFormat = gcvSURF_YUV420_10_ST;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_YUV420_TILE_ST:
        sourceFormat  = gcvSURF_YUV420_TILE_ST;
        textureFormat = gcvSURF_YUV420_TILE_ST;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_YUV420_TILE_10_ST:
        sourceFormat  = gcvSURF_YUV420_TILE_10_ST;
        textureFormat = gcvSURF_YUV420_TILE_10_ST;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_RGBA:
        sourceFormat = gcvSURF_A8B8G8R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB:
        sourceFormat = gcvSURF_X8R8G8B8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        sourceYuv = gcvFALSE;
        planarYuv = gcvFALSE;
        break;

    case GL_BGRA_EXT:
        sourceFormat = gcvSURF_A8R8G8B8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB565_OES:
        sourceFormat = gcvSURF_R5G6B5;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB5_A1_OES:
        sourceFormat = gcvSURF_A1R5G5B5;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RG8_EXT:
        sourceFormat = gcvSURF_G8R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_R8_EXT:
        sourceFormat = gcvSURF_R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_ALPHA:
        sourceFormat = gcvSURF_A8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_LUMINANCE8_ALPHA8_EXT:
        sourceFormat = gcvSURF_A8L8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check whether the source can be handled. */
    if (sourceYuv)
    {
        /*
         * Planar YUV requires 420tiler (422 not supported)
         * or yuv-assembler.
         */
        if (planarYuv &&
            !chipCtx->chipFeature.hwFeature.hasYuv420Tiler &&
            !chipCtx->chipFeature.hwFeature.hasYuvAssembler)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if ((format == GL_VIV_YUV420_10_ST ||
             format == GL_VIV_YUV420_TILE_ST ||
             format == GL_VIV_YUV420_TILE_10_ST) &&
            (!chipCtx->chipFeature.hwFeature.hasYuvAssembler10bit))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    /* Test if can sample texture source directly. */
    texInfo->direct.directSample = gcvFALSE;

    if (sourceYuv)
    {
        /* Check direct yuv class texture:
         * YUV assembler for planar yuv.
         * Linear texture for interleaved yuv. */
        if (
            (planarYuv && chipCtx->chipFeature.hwFeature.hasYuvAssembler) ||
            (!planarYuv && chipCtx->chipFeature.hwFeature.hasLinearTx))
        {
            texInfo->direct.directSample = gcvTRUE;
        }
    }
    else
    {
        /* RGB class texture. */
        if ((chipCtx->chipFeature.hwFeature.hasLinearTx) &&
            (chipCtx->chipFeature.hwFeature.hasTxSwizzle || sourceFormat == textureFormat))
        {
            /* Direct if format supported. */
            texInfo->direct.directSample = gcvTRUE;
        }
    }

    /* Remove the existing texture. */
    gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

    /* Destroy the texture object. */
    if (texInfo->object != gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_Destroy(texInfo->object));
        texInfo->object = gcvNULL;
    }

    /* Construct texture object. */
    gcmONERROR(gcoTEXTURE_ConstructEx(
        chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));

    /* Reset the dirty flag. */
    texInfo->direct.dirty = gcvFALSE;
    texInfo->direct.textureFormat = textureFormat;

    mipmap = &texObj->faceMipmap[0][0];
    texInfo->mipLevels[0].formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

    /* Construct the source surface. */
    gcmONERROR(gcoSURF_Construct(
        chipCtx->hal, width, height, 1, gcvSURF_BITMAP, sourceFormat, gcvPOOL_DEFAULT, &texInfo->direct.source));

    /* Lock the source surface. */
    gcmONERROR(gcoSURF_Lock(texInfo->direct.source, gcvNULL, memory));

    /* Return back virtual address pointers. */
    switch (format)
    {
    case GL_VIV_YV12:
        /* Y plane, V plane, U plane. */
        pixels[0] = memory[0];
        pixels[1] = memory[2];
        pixels[2] = memory[1];
        break;

    case GL_VIV_I420:
        /* Y plane, U plane, V plane. */
        pixels[0] = memory[0];
        pixels[1] = memory[1];
        pixels[2] = memory[2];
        break;

    case GL_VIV_NV12:
    case GL_VIV_NV21:
    case GL_VIV_YUV420_10_ST:
    case GL_VIV_YUV420_TILE_ST:
    case GL_VIV_YUV420_TILE_10_ST:
        /* Y plane, UV (VU) plane. */
        pixels[0] = memory[0];
        pixels[1] = memory[1];
        break;

    default:
        /* Single plane. */
        pixels[0] = memory[0];
        break;
    }

OnError:
    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(gcChipResetTextureWrapper(gc, texObj));
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

GLboolean
__glChipTexDirectInvalidateVIV(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    gcmHEADER_ARG("Context=0x%x texObj=0x%x", gc, texObj);

    /* Has to be a planar-sourced texture. */
    if (texInfo->direct.source == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Set the quick-reference dirty flag. */
    texInfo->direct.dirty = gcvTRUE;
    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipTexDirectVIVMap(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLsizei width,
    GLsizei height,
    GLenum format,
    GLvoid **logical,
    const GLuint *physical,
    GLboolean tiled
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    gctBOOL sourceYuv = gcvFALSE;
    gctBOOL planarYuv = gcvFALSE;
    __GLmipMapLevel     *mipmap;
    gceSURF_TYPE        type;

    gceSURF_FORMAT  sourceFormat = 0, textureFormat = 0;
    gceSTATUS       status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x target=0x%04x width=%d height=%d "
                  "format=0x%04x logical=0x%x physical=0x%x tiled=%d",
                  gc, texObj, target, width, height, format, logical, physical, tiled);

    /* Translate format information. */
    switch (format)
    {
    case GL_VIV_YV12:
        sourceFormat = gcvSURF_YV12;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_I420:
        sourceFormat = gcvSURF_I420;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_NV12:
        sourceFormat = gcvSURF_NV12;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_NV21:
        sourceFormat = gcvSURF_NV21;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case GL_VIV_YUY2:
        sourceFormat = gcvSURF_YUY2;
        textureFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvFALSE;
        break;

    case GL_VIV_UYVY:
        sourceFormat = gcvSURF_UYVY;
        textureFormat = gcvSURF_UYVY;
        sourceYuv = gcvTRUE;
        planarYuv = gcvFALSE;
        break;

    case GL_RGBA:
        sourceFormat = gcvSURF_A8B8G8R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB:
        sourceFormat = gcvSURF_X8R8G8B8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        sourceYuv = gcvFALSE;
        planarYuv = gcvFALSE;
        break;

    case GL_BGRA_EXT:
        sourceFormat = gcvSURF_A8R8G8B8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB565_OES:
        sourceFormat = gcvSURF_R5G6B5;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RGB5_A1_OES:
        sourceFormat = gcvSURF_A1R5G5B5;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_RG8_EXT:
        sourceFormat = gcvSURF_G8R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_R8_EXT:
        sourceFormat = gcvSURF_R8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_ALPHA:
        sourceFormat = gcvSURF_A8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_LUMINANCE_ALPHA:
        sourceFormat = gcvSURF_A8L8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_DEPTH_COMPONENT16:
        sourceFormat = gcvSURF_D16;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    case GL_LUMINANCE8_ALPHA8_EXT:
        sourceFormat = gcvSURF_A8L8;
        gcoTEXTURE_GetClosestFormat(gcvNULL, sourceFormat, &textureFormat);
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (sourceYuv && planarYuv && !chipCtx->chipFeature.hwFeature.hasYuv420Tiler)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Test if can sample texture source directly. */
    texInfo->direct.directSample = gcvFALSE;

    if (sourceYuv)
    {
        /* Check direct yuv class texture:
         * YUV assembler for planar yuv.
         * Linear texture for interleaved yuv. */
        if (
            (planarYuv && chipCtx->chipFeature.hwFeature.hasYuvAssembler) ||
            (!planarYuv && chipCtx->chipFeature.hwFeature.hasLinearTx))
        {
            texInfo->direct.directSample = gcvTRUE;
        }
    }
    else
    {
        /* RGB class texture. */
        if ((chipCtx->chipFeature.hwFeature.hasLinearTx) &&
            (chipCtx->chipFeature.hwFeature.hasTxSwizzle || sourceFormat == textureFormat))
        {
            /* Direct if format supported. */
            texInfo->direct.directSample = gcvTRUE;
        }
    }

    if (!texInfo->direct.directSample)
    {
        gctINT32 tileWidth, tileHeight;

        gcoHAL_QueryTiled(chipCtx->hal, gcvNULL, gcvNULL, &tileWidth, &tileHeight);

        /* For linear targets we need to have 4 tiles worth of alignment because of resolve. */
        if (!tiled)
        {
            tileWidth *= 4;
        }

        /* Currently hardware only supports aligned width and height. */
        if (width & (tileWidth - 1))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (height & (tileHeight - 1))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    if (planarYuv && tiled)
    {
        /* Tiled planar yuv? No such format. */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Remove the existing texture. */
    gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

    /* Destroy the texture object. */
    if (texInfo->object != gcvNULL)
    {
        gcmONERROR(gcoTEXTURE_Destroy(texInfo->object));
        texInfo->object = gcvNULL;
    }

    /* Construct texture object. */
    gcmONERROR(gcoTEXTURE_ConstructEx(
        chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));

    type = tiled ? gcvSURF_TEXTURE : gcvSURF_BITMAP;

    /* Construct the source surface. */
    gcmONERROR(gcoSURF_Construct(
        chipCtx->hal, width, height, 1, type, sourceFormat, gcvPOOL_USER, &texInfo->direct.source));

    /* Set the user buffer to the surface. */
    gcmONERROR(gcoSURF_MapUserSurface(texInfo->direct.source, 0, (GLvoid *) *logical, (gctUINT32) *physical));

    gcmONERROR(gcoSURF_Lock(texInfo->direct.source, gcvNULL, gcvNULL));

    /* Reset the dirty flag. */
    texInfo->direct.dirty = gcvFALSE;
    texInfo->direct.textureFormat = textureFormat;

    mipmap = &texObj->faceMipmap[0][0];
    texInfo->mipLevels[0].formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

OnError:
    if (gcmIS_ERROR(status))
    {
        gcmVERIFY_OK(gcChipResetTextureWrapper(gc, texObj));
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

gcsSURF_VIEW
gcChipGetAstcSurf(
    __GLcontext *gc,
    __GLtextureObject * tex,
    GLint level,
    GLint slice
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)tex->privateData;
    __GLchipMipmapInfo* mipLevel = &texInfo->mipLevels[level];
    gcsSURF_VIEW surfView = {gcvNULL, 0, 1};
    gctPOINTER memory[3] = {gcvNULL};
    gceSTATUS status = gcvSTATUS_OK;

    if (!mipLevel->astcSurf)
    {
        __GLmipMapLevel *mipmap = &tex->faceMipmap[0][level];
        __GLchipFmtMapInfo *formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

        /* Construct the surface. */
        gcmONERROR(gcoSURF_Construct(gcvNULL,
                                     (gctUINT)mipmap->width,
                                     (gctUINT)mipmap->height,
                                     __GL_MAX(tex->arrays, mipmap->depth),
                                     gcvSURF_TEXTURE,
                                     formatMapInfo->readFormat,
                                     gcvPOOL_DEFAULT,
                                     &mipLevel->astcSurf));

        /* Lock the surface. */
        gcmONERROR(gcoSURF_Lock(mipLevel->astcSurf, gcvNULL, memory));

        /* Uploaded texture in linear fashion. */
        gcoOS_MemCopy(memory[0], mipLevel->astcData, mipLevel->astcBytes);

        /* Free the CPU cache memory */
        gcoOS_Free(gcvNULL, (gctPOINTER)mipLevel->astcData);

        mipLevel->astcData = (GLubyte*)memory[0];
    }
    surfView.surf = mipLevel->astcSurf;
    surfView.firstSlice = slice;

OnError:
    if (gcmIS_ERROR(status))
    {
        if (memory[0])
        {
            gcmVERIFY_OK(gcoSURF_Unlock(mipLevel->astcSurf, memory[0]));
        }

        if (mipLevel->astcSurf)
        {
            gcmVERIFY_OK(gcoSURF_Destroy(mipLevel->astcSurf));
            mipLevel->astcSurf = gcvNULL;
        }

        gcChipSetError(chipCtx, status);
    }

    return surfView;
}

GLboolean __glChipCopyImageSubData(
    __GLcontext *gc,
    GLvoid * srcObject,
    GLint srcType,
    GLint srcLevel,
    GLint srcX,
    GLint srcY,
    GLint srcZ,
    GLvoid * dstObject,
    GLint dstType,
    GLint dstLevel,
    GLint dstX,
    GLint dstY,
    GLint dstZ,
    GLsizei width,
    GLsizei height,
    GLsizei depth
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcsSURF_VIEW srcView = {gcvNULL, 0, 1};
    gcsSURF_VIEW dstView = {gcvNULL, 0, 1};
    gctINT i;
    __GLformatInfo * srcFormatInfo = gcvNULL;
    __GLformatInfo * dstFormatInfo = gcvNULL;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < depth; i++)
    {
        __GLmipMapLevel *dstMipmap = NULL;
        __GLchipTextureInfo *dstTexInfo = gcvNULL;
        __GLchipMipmapInfo *dstMipLevel = gcvNULL;

        if (srcType != GL_RENDERBUFFER)
        {
            __GLtextureObject *tex = (__GLtextureObject*)srcObject;
            __GLmipMapLevel *mipmap = &tex->faceMipmap[0][srcLevel];
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)tex->privateData;
            __GLchipMipmapInfo *mipLevel = &texInfo->mipLevels[srcLevel];

            srcFormatInfo = mipmap->formatInfo;
            srcView = mipLevel->astcData
                    ? gcChipGetAstcSurf(gc, tex, srcLevel, srcZ + i)
                    : gcChipGetTextureSurface(chipCtx, tex, gcvFALSE, srcLevel, srcZ + i);
        }
        else
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)srcObject;
            __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);

            srcView.surf = chipRBO->surface;
            srcFormatInfo = rbo->formatInfo;
        }

        if (dstType != GL_RENDERBUFFER)
        {
            __GLtextureObject *tex = (__GLtextureObject*)dstObject;
            dstMipmap = &tex->faceMipmap[0][dstLevel];
            dstTexInfo = (__GLchipTextureInfo*)tex->privateData;
            dstMipLevel = &dstTexInfo->mipLevels[dstLevel];

            dstFormatInfo = dstMipmap->formatInfo;
            dstView = dstMipLevel->astcData
                    ? gcChipGetAstcSurf(gc, tex, dstLevel, dstZ + i)
                    : gcChipGetTextureSurface(chipCtx, tex, gcvFALSE, dstLevel, dstZ + i);
        }
        else
        {
            __GLrenderbufferObject *rbo = (__GLrenderbufferObject*)dstObject;
            __GLchipRenderbufferObject *chipRBO = (__GLchipRenderbufferObject*)(rbo->privateData);

            dstView.surf = chipRBO->surface;
            dstFormatInfo = rbo->formatInfo;
        }

        if (srcView.surf && dstView.surf)
        {
            gctUINT srcWidth, srcHeight;
            gctUINT dstWidth, dstHeight;

            gcmONERROR(gcoSURF_GetAlignedSize(srcView.surf, &srcWidth, &srcHeight, gcvNULL));
            gcmONERROR(gcoSURF_GetAlignedSize(dstView.surf, &dstWidth, &dstHeight, gcvNULL));

            width  = __GL_MIN(width,  (gctINT32)srcWidth  - srcX);
            height = __GL_MIN(height, (gctINT32)srcHeight - srcY);
            if (srcFormatInfo->compressed && ! dstFormatInfo->compressed)
            {
                width  = __GL_MIN(width,  ((gctINT32)dstWidth  - dstX) * (gctINT32)srcView.surf->formatInfo.blockWidth);
                height = __GL_MIN(height, ((gctINT32)dstHeight - dstY) * (gctINT32)srcView.surf->formatInfo.blockHeight);
            }
            else if (!srcFormatInfo->compressed && dstFormatInfo->compressed)
            {
                width  = __GL_MIN(width,  ((gctINT32)dstWidth  - dstX) / (gctINT32)dstView.surf->formatInfo.blockWidth);
                height = __GL_MIN(height, ((gctINT32)dstHeight - dstY) / (gctINT32)dstView.surf->formatInfo.blockHeight);
            }
            else
            {
                width  = __GL_MIN(width,  (gctINT32)dstWidth  - dstX);
                height = __GL_MIN(height, (gctINT32)dstHeight - dstY);
            }

            if (width > 0 && height > 0)
            {
                gcsSURF_RESOLVE_ARGS rlvArgs = {0};

                rlvArgs.version = gcvHAL_ARG_VERSION_V2;
                rlvArgs.uArgs.v2.directCopy = gcvTRUE;
                rlvArgs.uArgs.v2.srcOrigin.x = srcX;
                rlvArgs.uArgs.v2.srcOrigin.y = srcY;
                rlvArgs.uArgs.v2.dstOrigin.x = dstX;
                rlvArgs.uArgs.v2.dstOrigin.y = dstY;
                rlvArgs.uArgs.v2.rectSize.x  = width;
                rlvArgs.uArgs.v2.rectSize.y  = height;
                rlvArgs.uArgs.v2.srcSwizzle = gcvFALSE;
                rlvArgs.uArgs.v2.dstSwizzle = gcvFALSE;
                rlvArgs.uArgs.v2.srcCompressed = srcFormatInfo->compressed;
                rlvArgs.uArgs.v2.dstCompressed = dstFormatInfo->compressed;

                if (srcView.surf->format !=dstView.surf->format)
                {
                    if((srcFormatInfo->glFormat == GL_RGBA8 &&
                        srcView.surf->format == gcvSURF_A8R8G8B8)  ||
                        (srcFormatInfo->glFormat == GL_RGB8&&
                        srcView.surf->format == gcvSURF_X8R8G8B8)  ||
                        srcFormatInfo->glFormat == GL_SRGB8_ALPHA8 ||
                        srcFormatInfo->glFormat == GL_SRGB8)
                    {
                        rlvArgs.uArgs.v2.srcSwizzle = gcvTRUE;
                    }

                    if((dstFormatInfo->glFormat == GL_RGBA8 &&
                        dstView.surf->format == gcvSURF_A8R8G8B8)  ||
                        (dstFormatInfo->glFormat == GL_RGB8&&
                        dstView.surf->format == gcvSURF_X8R8G8B8)  ||
                        dstFormatInfo->glFormat == GL_SRGB8_ALPHA8 ||
                        dstFormatInfo->glFormat == GL_SRGB8)
                    {
                        rlvArgs.uArgs.v2.dstSwizzle = gcvTRUE;
                    }
                }

                gcmONERROR(gcoSURF_ResolveRect(&srcView, &dstView, &rlvArgs));
            }

        }

        if (dstMipLevel && dstMipLevel->astcData)
        {
            gctSIZE_T rowStride = 0;
            gceSURF_FORMAT imageFormat = gcvSURF_UNKNOWN;

            GLvoid *pixels = gcChipDecompressASTC(gc,
                                                  (gctSIZE_T)dstMipmap->width,
                                                  (gctSIZE_T)dstMipmap->height,
                                                  (gctSIZE_T)1,
                                                  (gctSIZE_T)dstMipmap->compressedSize,
                                                  dstMipLevel->astcData + (dstZ + i) * dstMipmap->compressedSize,
                                                  dstMipmap->formatInfo,
                                                  &imageFormat,
                                                  &rowStride);

            gcmONERROR(gcoTEXTURE_Upload(dstTexInfo->object,
                                         dstLevel,
                                         gcvFACE_NONE,
                                         (gctSIZE_T)dstMipmap->width,
                                         (gctSIZE_T)dstMipmap->height,
                                         dstZ + i,
                                         (const GLvoid*)(GLbyte*)pixels,
                                         rowStride, /* stride */
                                         imageFormat,
                                         gcvSURF_COLOR_SPACE_LINEAR));

            if (pixels)
            {
                gcoOS_Free(gcvNULL, (gctPOINTER)pixels);
                pixels = gcvNULL;
            }
        }
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

gceSTATUS
gcChipTexSyncDirectVIV(
    __GLcontext *gc,
    __GLtextureObject *texObj
    )
{
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcoSURF mipmap = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gctBOOL dirty;
    gcoSURF source;
    gctBOOL directSample;
    gceSURF_FORMAT textureFormat;

    gcmHEADER_ARG("Context=0x%x texObj=0x%x", gc, texObj);

    dirty  = texInfo->direct.dirty;
    source = texInfo->direct.source;
    directSample  = texInfo->direct.directSample;
    textureFormat = texInfo->direct.textureFormat;

    if (!texInfo->object)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(
            chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));
    }

    if (!directSample)
    {
        status = gcoTEXTURE_GetMipMap(texInfo->object, 0, &mipmap);

        if (gcmIS_ERROR(status))
        {
            /* Allocate mipmap level 0. */
            gctUINT width;
            gctUINT height;

            status = gcvSTATUS_TRUE;

            /* Get source size. */
            gcmVERIFY_OK(gcoSURF_GetSize(source, &width, &height, gcvNULL));

            /* Create mipmap level 0. */
            gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                0, gcvUNKNOWN_MIPMAP_IMAGE_FORMAT, textureFormat, width, height, 1, 1, gcvPOOL_DEFAULT, &mipmap));

            /* Mipmap allocated, force dirty. */
            dirty = gcvTRUE;
        }
    }

    if (dirty)
    {
        if (directSample)
        {
            /* Directly add surface to mipmap. */
            gcmONERROR(gcoTEXTURE_AddMipMapFromClient(texInfo->object, 0, source));
        }
        else
        {
            gceSURF_FORMAT srcFormat;

            /* Get source format. */
            gcmVERIFY_OK(gcoSURF_GetFormat(source, gcvNULL, &srcFormat));

            /*
             * Android has following formats, but hw does not support
             * Need software upload for such formats.
             */
            if ((srcFormat == gcvSURF_NV16) || (srcFormat == gcvSURF_NV61))
            {
                gctUINT width;
                gctUINT height;
                gctPOINTER memory[3] = {gcvNULL};
                gctINT stride[3];

                gcmVERIFY_OK(gcoSURF_GetSize(source, &width, &height, gcvNULL));

                gcmVERIFY_OK(gcoSURF_GetAlignedSize(source, gcvNULL, gcvNULL, stride));

                /* Lock source surface for read. */
                gcmONERROR(gcoSURF_Lock(source, gcvNULL, memory));

                /* UV stride should be same as Y stride. */
                stride[1] = stride[0];

                /* Upload NV16/NV61 to YUY2 by software. */
                status = gcoTEXTURE_UploadYUV(texInfo->object,
                    gcvFACE_NONE, width, height, 0, memory, stride, srcFormat);

                /* Unlock. */
                gcmVERIFY_OK(gcoSURF_Unlock(source, memory[0]));

                /* Check status. */
                gcmONERROR(status);
            }
            else
            {
                gcsSURF_VIEW srcView = {source, 0, 1};
                gcsSURF_VIEW mipView = {mipmap, 0, 1};

                /* Use resolve to upload texture. */
                gcmONERROR(gcoSURF_ResolveRect(&srcView, &mipView, gcvNULL));

                /* Wait all the pixels done. */
                gcmVERIFY_OK(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));
            }
        }

        /* Set dirty flag (for later flush). */
        gcoTEXTURE_Flush(texInfo->object);

        /* Commit changes. */
        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, gcvFALSE));

        /* Reset dirty flag. */
        texInfo->direct.dirty = gcvFALSE;
    }

OnError:
    gcmFOOTER_ARG("return=%d", status);
    return status;
}


gceSTATUS
gcChipTexSyncEGLImage(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    gctBOOL stall
    )
{
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    khrEGL_IMAGE_PTR image;
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF mipmap = gcvNULL;
    gctBOOL dirty;
    gcoSURF source;
    gctBOOL directSample;
    gceSURF_FORMAT textureFormat;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    /* Always update for texture 2D target. */
    dirty  = texInfo->eglImage.dirty;

    source = texInfo->eglImage.source;
    directSample  = texInfo->eglImage.directSample;
    textureFormat = texInfo->eglImage.textureFormat;

    if (!texInfo->object)
    {
        gcmONERROR(gcoTEXTURE_ConstructEx(
            chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &texInfo->object));
    }

    if (!directSample)
    {
        status = gcoTEXTURE_GetMipMap(texInfo->object, 0, &mipmap);

        if (gcmIS_ERROR(status))
        {
            /* Allocate mipmap level 0. */
            gctUINT width;
            gctUINT height;

            status = gcvSTATUS_TRUE;

            /* Get source size. */
            gcmVERIFY_OK(gcoSURF_GetSize(source, &width, &height, gcvNULL));

            /* Create mipmap level 0. */
            gcmONERROR(gcoTEXTURE_AddMipMap(texInfo->object,
                0, gcvUNKNOWN_MIPMAP_IMAGE_FORMAT, textureFormat, width, height, 1, 1, gcvPOOL_DEFAULT, &mipmap));

            /* Mipmap allocated, force dirty. */
            dirty = gcvTRUE;
        }
    }

    image = (khrEGL_IMAGE_PTR) texInfo->eglImage.image;

    if (image->update)
    {
        /* Update source sibling pixels. */
        if (image->update(image))
        {
            /* Force dirty if updated. */
            dirty = gcvTRUE;
        }
    }
    else
    {
        /* Assume dirty for other EGL image types. */
        dirty = gcvTRUE;
    }

    if (dirty)
    {
        if (directSample)
        {
            /* Directly add surface to mipmap. */
            gcmONERROR(gcoTEXTURE_AddMipMapFromClient(texInfo->object, 0, source));
        }
        else
        {
            /* Lock the image mutex. */
            gcoOS_AcquireMutex(gcvNULL, image->mutex, gcvINFINITE);

            /* srcSurface should be the most up-to-date copy */
            if (image->srcSurface)
            {
                source = image->srcSurface;
            }

            /* Release the image mutex. */
            gcoOS_ReleaseMutex(gcvNULL, image->mutex);

            if (source != mipmap)
            {
                gceSURF_FORMAT srcFormat;

                /* Get source format. */
                gcmVERIFY_OK(gcoSURF_GetFormat(source, gcvNULL, &srcFormat));

                /*
                 * Android has following formats, but hw does not support
                 * Need software upload for such formats.
                 */
                if ((srcFormat == gcvSURF_NV16) || (srcFormat == gcvSURF_NV61) ||
                    (srcFormat == gcvSURF_R4G4B4A4) || (srcFormat == gcvSURF_R5G5B5A1))
                {
                    gctUINT width;
                    gctUINT height;
                    gctPOINTER memory[3] = {gcvNULL};
                    gctINT stride[3];

                    gcmVERIFY_OK(gcoSURF_GetSize(source, &width, &height, gcvNULL));

                    gcmVERIFY_OK(gcoSURF_GetAlignedSize(source, gcvNULL, gcvNULL, stride));

                    /* Lock source surface for read. */
                    gcmVERIFY_OK(gcoSURF_Lock(source, gcvNULL, memory));

                    if ((srcFormat == gcvSURF_NV16) || (srcFormat == gcvSURF_NV61))
                    {
                        /* UV stride should be same as Y stride. */
                        stride[1] = stride[0];

                        /* Upload NV16/NV61 to YUY2 by software. */
                        status = gcoTEXTURE_UploadYUV(texInfo->object,
                            gcvFACE_NONE, width, height, 0, memory, stride, srcFormat);
                    }
                    else
                    {
                        /* Upload by software. */
                        status = gcoTEXTURE_Upload(texInfo->object,
                            0, gcvFACE_NONE, width, height, 0, memory[0], stride[0], srcFormat, gcvSURF_COLOR_SPACE_LINEAR);
                    }

                    /* Unlock. */
                    gcmVERIFY_OK(gcoSURF_Unlock(source, memory[0]));

                    /* Check status. */
                    gcmONERROR(status);
                }
                else
                {
                    gcsSURF_VIEW srcView = {source, 0, 1};
                    gcsSURF_VIEW mipView = {mipmap, 0, 1};

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                    android_native_buffer_t * nativeBuffer;
                    struct private_handle_t * hnd = gcvNULL;
                    struct gc_native_handle_t * handle = gcvNULL;

                    /* Cast to android native buffer. */
                    nativeBuffer = (android_native_buffer_t *) texInfo->eglImage.nativeBuffer;

                    if (nativeBuffer != gcvNULL)
                    {
                        /* Get private handle. */
                        hnd = (struct private_handle_t *) nativeBuffer->handle;
                        handle = gc_native_handle_get(nativeBuffer->handle);
                    }

                    /* Check composition signal. */
                    if (handle != gcvNULL && handle->hwDoneSignal != 0)
                    {
                        gcmVERIFY_OK(gcoOS_Signal(gcvNULL, (gctSIGNAL) (gctUINTPTR_T) handle->hwDoneSignal, gcvFALSE));
                    }
#endif

                    /* Use resolve to upload texture. */
                    gcmONERROR(gcoSURF_ResolveRect(&srcView, &mipView, gcvNULL));

#if defined(ANDROID) && gcdANDROID_IMPLICIT_NATIVE_BUFFER_SYNC
                    if (handle != gcvNULL && handle->hwDoneSignal != 0)
                    {
                        /* Signal the signal, so CPU apps
                         * can lock again once resolve is done. */
                        gcsHAL_INTERFACE iface;

                        iface.command            = gcvHAL_SIGNAL;
                        iface.engine             = gcvENGINE_RENDER;
                        iface.u.Signal.signal    = handle->hwDoneSignal;
                        iface.u.Signal.auxSignal = 0;
                        /* Stuff the client's PID. */
                        iface.u.Signal.process   = handle->clientPID;
                        iface.u.Signal.fromWhere = gcvKERNEL_PIXEL;

                        /* Schedule the event. */
                        gcmVERIFY_OK(gcoHAL_ScheduleEvent(gcvNULL, &iface));
                    }
#endif

                    /* Wait all the pixels done. */
                    gcmVERIFY_OK(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));
                }
            }
            else
            {
                status = gcvSTATUS_SKIP;
            }
        }

        /* Set dirty flag (for later flush). */
        gcoTEXTURE_Flush(texInfo->object);

        /* Commit changes. */
        gcmVERIFY_OK(gcoHAL_Commit(gcvNULL, stall));

        /* Reset dirty flag. */
        texInfo->eglImage.dirty = gcvFALSE;
    }

OnError:
    gcmFOOTER();
    return status;
}

void
gcChipTexCheckDirtyStateKeep(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLuint samplerID
    )
{
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;
    khrEGL_IMAGE_PTR image;

    image = (khrEGL_IMAGE_PTR) texInfo->eglImage.image;

    if (image && image->magic == KHR_EGL_IMAGE_MAGIC_NUM && image->update)
    {
        /* If the image source sibling may be changed implicitly,
         * it should be dirty on next draw. */
        __glBitmaskSet(&gc->shaderProgram.samplerStateKeepDirty, samplerID);
    }
}

gceSTATUS
gcChipTexGetFormatInfo(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    gcsSURF_FORMAT_INFO_PTR * txFormatInfo
    )
{
    gceSTATUS status;
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    gcmHEADER_ARG("gc=0x%x texObj=0x%x", gc, texObj);

    /* For eglimage target texture and VIV_direct_texture, texture mipmap is
     * defered allocated. */
    if (texInfo->eglImage.source)
    {
        if (texInfo->eglImage.directSample)
        {
            status = gcoSURF_GetFormatInfo(texInfo->eglImage.source, txFormatInfo);
        }
        else
        {
            status = gcoSURF_QueryFormat(texInfo->eglImage.textureFormat, txFormatInfo);
        }
    }
    else if (texInfo->direct.source)
    {
        if (texInfo->direct.directSample)
        {
            status = gcoSURF_GetFormatInfo(texInfo->direct.source, txFormatInfo);
        }
        else
        {
            status = gcoSURF_QueryFormat(texInfo->direct.textureFormat, txFormatInfo);
        }
    }
    else if (texObj->bufObj)
    {
        __GLmipMapLevel *mipmap = &texObj->faceMipmap[0][0];
        __GLchipFmtMapInfo *formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);
        status = gcoSURF_QueryFormat(formatMapInfo->readFormat, txFormatInfo);
    }
    else
    {
        status = gcoTEXTURE_GetFormatInfo(texInfo->object, texObj->params.baseLevel, txFormatInfo);
    }

    gcmFOOTER();
    return status;
}

GLenum
__glChipCreateEglImageTexture(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLint face,
    GLint level,
    GLint depth,
    GLvoid * image
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo *texInfo;
    gcsSURF_VIEW texView;
    gctUINT width = 0;
    gctUINT height = 0;
    khrEGL_IMAGE * eglImage;
    __GLmipMapLevel *mipmap;
    gcmHEADER_ARG("gc=0x%x texObj=0x%x face=%d, level=%d depth=%d image=0x%x", gc, texObj, face, level, depth, image);

    mipmap = &texObj->faceMipmap[face][level];

    texInfo = (__GLchipTextureInfo*)texObj->privateData;

    /* Test if surface is a sibling of any eglImage. */
    if ((face == 0) && (texInfo->eglImage.image != gcvNULL))
    {
        gcmFOOTER_ARG("return=0x%04x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }

    texView = gcChipGetTextureSurface(chipCtx, texObj, gcvFALSE, level, face);

    if (texView.surf == gcvNULL)
    {
        gcmFOOTER_ARG("return=0x%04x", EGL_BAD_PARAMETER);
        return EGL_BAD_PARAMETER;
    }

    gcmONERROR(gcoSURF_GetSize(texView.surf, &width, &height, gcvNULL));

    /* If texture was drawn before, resolve to texture surface before used as EGLimage source */
    gcmONERROR(gcChipTexMipSliceSyncFromShadow(gc, texObj, face, level, depth));

    eglImage = (khrEGL_IMAGE*)image;

    if (texView.firstSlice != 0)
    {
        GLuint targetW, targetH, shadowW, shadowH;
        GLuint shadowSamples, targetSamples;
        GLboolean needAlloc = GL_TRUE;
        gceSURF_FORMAT targetFormat, shadowFormat;

        gcmONERROR(gcoSURF_GetSize(texView.surf, &targetW, &targetH, gcvNULL));
        targetFormat = texInfo->mipLevels[level].formatMapInfo->readFormat;
        targetSamples = 1;

        if (eglImage->u.texture.shadowSurface)
        {
            gcmONERROR(gcoSURF_GetSize(eglImage->u.texture.shadowSurface, &shadowW, &shadowH, gcvNULL));
            gcmONERROR(gcoSURF_GetFormat(eglImage->u.texture.shadowSurface, gcvNULL, &shadowFormat));
            gcmONERROR(gcoSURF_GetSamples(eglImage->u.texture.shadowSurface, &shadowSamples));
            if ((shadowW == targetW) &&
                (shadowH == targetH) &&
                (shadowFormat == targetFormat) &&
                (shadowSamples == targetSamples))
            {
                needAlloc = GL_FALSE;
            }
        }

        if (needAlloc)
        {
            gceSURF_TYPE surfType = gcvSURF_TYPE_UNKNOWN;

            if (texView.surf->formatInfo.fmtClass == gcvFORMAT_CLASS_DEPTH)
            {
                surfType = gcvSURF_DEPTH_TS_DIRTY;
            }
            else
            {
                surfType = gcvSURF_RENDER_TARGET_TS_DIRTY;
            }

            if (((VEGLImage)image)->protectedContent)
            {
                surfType |= gcvSURF_PROTECTED_CONTENT;
            }

            if (eglImage->u.texture.shadowSurface)
            {
                gcmONERROR(gcoSURF_Destroy(eglImage->u.texture.shadowSurface));
                eglImage->u.texture.shadowSurface = gcvNULL;
            }

            gcmONERROR(gcoSURF_Construct(chipCtx->hal, targetW, targetH, 1, surfType,
                targetFormat, gcvPOOL_DEFAULT, &eglImage->u.texture.shadowSurface));

            chipCtx->needRTRecompile = chipCtx->needRTRecompile
                                     || gcChipCheckRecompileEnable(gc, targetFormat);

            chipCtx->needTexRecompile = chipCtx->needTexRecompile
                                      || gcChipCheckRecompileEnable(gc, targetFormat);

            gcmONERROR(gcoSURF_SetSamples(eglImage->u.texture.shadowSurface, targetSamples));
        }

        eglImage->u.texture.masterDirty = gcvTRUE;
    }

    /* Set EGL Image info. */
    eglImage->magic             = KHR_EGL_IMAGE_MAGIC_NUM;
    eglImage->surface           = texView.surf;
    eglImage->u.texture.level   = level;
    eglImage->u.texture.face    = face;
    eglImage->u.texture.depth   = depth;
    eglImage->u.texture.sliceIndex = texView.firstSlice;
    eglImage->u.texture.width   = width;
    eglImage->u.texture.height  = height;
    eglImage->u.texture.object  = texInfo->object;
    eglImage->u.texture.format  = mipmap->format;
    eglImage->u.texture.internalFormat = mipmap->interalFormat;
    eglImage->u.texture.type = mipmap->type;

    gcmONERROR(gcoSURF_SetResolvability(texView.surf, gcvFALSE));

    /* Reference EGLImage. */
    if (texInfo->eglImage.image != eglImage)
    {
        if (texInfo->eglImage.image != gcvNULL)
        {
            /* Dereference old EGLImageKHR. */
            gc->imports.dereferenceImage(texInfo->eglImage.image);
        }

        /* Reference new EGLImageKHR. */
        texInfo->eglImage.image = eglImage;
        gc->imports.referenceImage(eglImage);
    }

    gcmFOOTER_ARG("return=0x%04x", EGL_SUCCESS);
    return EGL_SUCCESS;

OnError:
    gcmFOOTER_ARG("return=0x%04x", EGL_BAD_ACCESS);
    return EGL_BAD_ACCESS;
}

GLboolean
__glChipGetTextureAttribFromImage(
    __GLcontext     *gc,
    GLvoid          *eglImage,
    GLint           *width,
    GLint           *height,
    GLint           *stride,
    gceSURF_FORMAT  *format,
    GLint           *glFormat,
    GLint           *glInternalFormat,
    GLint           *glType,
    GLint           *level,
    GLuint          *sliceIndex,
    GLvoid          **pixel
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcoSURF   surface = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    khrEGL_IMAGE *image = (khrEGL_IMAGE_PTR)eglImage;
    gcmHEADER_ARG("gc=0x%x eglImage=0x%x width=0x%x height=0x%x stride=0x%x glFormat=0x%x"
                  "glInternalFormat=0x%x glType=0x%x level=0x%x sliceIndex=0x%x pixel=0x%x",
                   gc, eglImage, width, height, stride, format, glFormat,
                   glInternalFormat, glType, level, sliceIndex, pixel);

    /* Get texture attributes. */
    switch (image->type)
    {
    case KHR_IMAGE_TEXTURE_2D:
    case KHR_IMAGE_TEXTURE_CUBE:
    case KHR_IMAGE_RENDER_BUFFER:
    case KHR_IMAGE_ANDROID_NATIVE_BUFFER:
    case KHR_IMAGE_WAYLAND_BUFFER:
    case KHR_IMAGE_VIV_DEC:
    case KHR_IMAGE_PIXMAP:
    case KHR_IMAGE_LINUX_DMA_BUF:
        {
            surface = image->surface;

            if (!surface)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if ((width != gcvNULL) && (height != gcvNULL))
            {
                /* Query width and height from source. */
                gcmONERROR(
                    gcoSURF_GetSize(surface,
                                    (gctUINT_PTR) width,
                                    (gctUINT_PTR) height,
                                    gcvNULL));
            }

            if (format != gcvNULL)
            {
                /* Query source surface format. */
                gcmONERROR(gcoSURF_GetFormat(surface, gcvNULL, format));
            }

            if (stride != gcvNULL)
            {
                /* Query srouce surface stride. */
                gcmONERROR(gcoSURF_GetAlignedSize(surface, gcvNULL, gcvNULL, stride));
            }

            if (sliceIndex != gcvNULL)
            {
                if (image->type == KHR_IMAGE_TEXTURE_CUBE)
                {
                    *sliceIndex = image->u.texture.sliceIndex;
                }
                else
                {
                    *sliceIndex = 0;
                }
            }

            if (level != gcvNULL)
            {
                *level = 0;
            }

            if (pixel != gcvNULL)
            {
                *pixel = gcvNULL;
            }
        }
        break;
    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if ((glFormat != gcvNULL) || (glInternalFormat != gcvNULL) || (glType != gcvNULL))
    {
        gceSURF_FORMAT surfFormat;
        GLint format = GL_NONE;
        GLint internalFormat = GL_NONE;
        GLint type = GL_NONE;

        if (!surface)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        gcmONERROR(gcoSURF_GetFormat(surface, gcvNULL, &surfFormat));
        /* Convert surface format to GL format. */
        switch (surfFormat)
        {
        case gcvSURF_A8B8G8R8:
            format = internalFormat = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_A16B16G16R16F:
            format = GL_RGBA;
            internalFormat = GL_RGBA16F;
            type = GL_HALF_FLOAT;
            break;

        case gcvSURF_X8B8G8R8:
            format = internalFormat = __GL_RGBX8;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_X8R8G8B8:
            format = internalFormat = __GL_BGRX8;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_A8R8G8B8:
            format = internalFormat = GL_BGRA_EXT;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_R5G6B5:
            format = internalFormat = GL_RGB;
            type = GL_UNSIGNED_SHORT_5_6_5;
            break;

        case gcvSURF_R5G5B5A1:
            format = internalFormat = GL_RGBA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case gcvSURF_A1R5G5B5:
            format = internalFormat = GL_RGBA;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case gcvSURF_X1R5G5B5:
            format = internalFormat = GL_RGB;
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;

        case gcvSURF_R4G4B4A4:
            format = internalFormat = GL_RGBA;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;

        case gcvSURF_A4R4G4B4:
            format = internalFormat = __GL_ARGB4;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;

        case gcvSURF_X4R4G4B4:
            format = internalFormat = __GL_XRGB4;
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;

        case gcvSURF_R8:
            format = internalFormat = GL_RED;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_G8R8:
            format = internalFormat = GL_RG;
            type = GL_UNSIGNED_BYTE;
            break;

        case gcvSURF_YV12:
            format = GL_VIV_YV12;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_I420:
            format = GL_VIV_I420;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_NV12:
            format  = GL_VIV_NV12;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_NV21:
            format = GL_VIV_NV21;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_YUY2:
            format = GL_VIV_YUY2;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_UYVY:
            format = GL_VIV_UYVY;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_AYUV:
            format = GL_VIV_AYUV;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_NV16:
            format = GL_VIV_YUY2;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        case gcvSURF_NV61:
            format = GL_VIV_UYVY;
            internalFormat = GL_RGBA;
            type = GL_NONE;
            break;

        default:
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (glFormat)
        {
            *glFormat = format;
        }

        if (glInternalFormat)
        {
            *glInternalFormat = internalFormat;
        }

        if (glType)
        {
            *glType = type;
        }
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipEglImageTargetTexture2DOES(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    GLenum target,
    GLvoid *eglImage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipTextureInfo  *texInfo = (__GLchipTextureInfo *)texObj->privateData;

    khrEGL_IMAGE *image = (khrEGL_IMAGE_PTR)eglImage;

    gceSTATUS status = gcvSTATUS_OK;
    gctINT width = 0, height = 0;
    gctINT stride = 0;
    gctINT level = 0;
    gceSURF_FORMAT srcFormat = gcvSURF_UNKNOWN;
    gceSURF_FORMAT dstFormat = gcvSURF_UNKNOWN;
    gctPOINTER pixel = gcvNULL;
    gctBOOL sourceYuv = gcvFALSE;
    gctBOOL planarYuv = gcvFALSE;
    gceSURF_TYPE srcType = gcvSURF_TYPE_UNKNOWN;
    gctBOOL resetTexture = gcvTRUE;
    gctBOOL dirty = gcvFALSE;
    gcoSURF surface;
    __GLmipMapLevel *mipmap = &texObj->faceMipmap[0][0];
    __GLchipMipmapInfo *chipMipLevel = &texInfo->mipLevels[0];
    gcsSURF_VIEW texView = {gcvNULL, 0, 1};

    gcmHEADER_ARG("gc=0x%x texObj=0x%x target=0x%04x eglImage=0x%x", gc, texObj, target, eglImage);

    /* Get texture attributes from eglImage. */
    if (!__glChipGetTextureAttribFromImage(gc, eglImage, &width, &height, &stride, &srcFormat,
                                           gcvNULL, gcvNULL, gcvNULL, &level, gcvNULL, &pixel))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Check the texture size. */
    if ((width <= 0) || (height <= 0) ||
        ((gctUINT) width  > gc->constants.maxTextureSize) ||
        ((gctUINT) height > gc->constants.maxTextureSize))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if ((target == GL_TEXTURE_EXTERNAL_OES) &&
        (level != 0))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    chipMipLevel->formatMapInfo = gcChipGetFormatMapInfo(gc, mipmap->formatInfo->drvFormat, __GL_CHIP_FMT_PATCH_NONE);

    /* Validate the format. */
    switch (srcFormat)
    {
    case gcvSURF_YV12:
    case gcvSURF_I420:
    case gcvSURF_NV12:
    case gcvSURF_NV21:
    case gcvSURF_NV16:
    case gcvSURF_NV61:
        dstFormat = gcvSURF_YUY2;
        sourceYuv = gcvTRUE;
        planarYuv = gcvTRUE;
        break;

    case gcvSURF_YUY2:
    case gcvSURF_UYVY:
    case gcvSURF_YVYU:
    case gcvSURF_VYUY:
        dstFormat = srcFormat;
        sourceYuv = gcvTRUE;
        break;

    default:
        /* Get closest supported destination. */
        dstFormat = chipMipLevel->formatMapInfo->readFormat;
        break;
    }

    /* Make the compiler happy, disable warnings. */
#ifndef __clang__
    sourceYuv = sourceYuv;
    planarYuv = planarYuv;
#endif

    /* Save closest supported destination. */
    texInfo->eglImage.textureFormat = dstFormat;

    if (image->type == KHR_IMAGE_TEXTURE_CUBE && image->u.texture.face > 0)
    {
        if ((image->u.texture.shadowSurface != gcvNULL) && (image->u.texture.masterDirty))
        {
            gcsSURF_VIEW imageView  = {image->surface, image->u.texture.sliceIndex, 1};
            gcsSURF_VIEW shadowView = {image->u.texture.shadowSurface, 0, 1};

            gcmONERROR(gcoSURF_ResolveRect(&imageView, &shadowView, gcvNULL));
            image->u.texture.masterDirty = gcvFALSE;
        }
        surface = image->u.texture.shadowSurface;
    }
    else
    {
        /* As the srcSurface of image could any tiling/format, we are resolve for now, if we use the surface info to decide
            renderable and texturable, we could save this resolve if match.
        */
        if (image->srcSurface && image->srcSurface != image->surface)
        {
            gcsSURF_VIEW imgView = {image->surface, image->u.texture.sliceIndex, 1};
            gcsSURF_VIEW srcView = {image->srcSurface, 0, 1};

            gcmONERROR(gcoSURF_ResolveRect(&srcView, &imgView, gcvNULL));

            gcmONERROR(gcChipSetImageSrc(image, gcvNULL));
        }

        surface = image->surface;
    }

    if (image->update)
    {
        /* Update image source sibling. */
        dirty = image->update(image);
    }

    gcmONERROR(gcoSURF_GetFormat(surface, &srcType, gcvNULL));

    /* Test if can sample texture source directly. */
    texInfo->eglImage.directSample = gcvFALSE;

    if (sourceYuv)
    {
        /* YUV class, should be linear. */
        gcmASSERT(srcType == gcvSURF_BITMAP);

        /* Check direct yuv class texture:
         * YUV assembler for planar yuv.
         * Linear texture for interleaved yuv. */
        if (
            (planarYuv && chipCtx->chipFeature.hwFeature.hasYuvAssembler) ||
            (!planarYuv && chipCtx->chipFeature.hwFeature.hasLinearTx))
        {
            texInfo->eglImage.directSample = gcvTRUE;
        }
    }
    else
    {
        /* RGB class texture. */
        if ((srcType == gcvSURF_BITMAP && chipCtx->chipFeature.hwFeature.hasLinearTx) ||
            (srcType == gcvSURF_TEXTURE) ||
            (srcType == gcvSURF_RENDER_TARGET && chipCtx->chipFeature.hwFeature.hasSupertiledTx))
        {
            /* Direct if format supported. */
            if (srcFormat == dstFormat)
            {
                texInfo->eglImage.directSample = gcvTRUE;
            }
            else if (chipCtx->chipFeature.hwFeature.hasTxSwizzle)
            {
                switch (srcFormat)
                {
                case gcvSURF_A8B8G8R8:
                case gcvSURF_X8B8G8R8:
                case gcvSURF_A1B5G5R5:
                case gcvSURF_X1B5G5R5:
                case gcvSURF_A4B4G4R4:
                case gcvSURF_X4B4G4R4:
                case gcvSURF_B5G6R5:
                    /* Tx swizzle to support directly. */
                    texInfo->eglImage.directSample = gcvTRUE;
                    break;

                default:
                    break;
                }
            }
        }

        /* More checks. */
        if (texInfo->eglImage.directSample)
        {
            /* Check tile status. */
            texView.surf = surface;
            if (!chipCtx->chipFeature.hwFeature.hasTxTileStatus && gcoSURF_IsTileStatusEnabled(&texView))
            {
                texInfo->eglImage.directSample = gcvFALSE;
            }
            /* Check compression. */
            else if (!chipCtx->chipFeature.hwFeature.hasTxDecompressor && gcoSURF_IsCompressed(&texView))
            {
                texInfo->eglImage.directSample = gcvFALSE;
            }
            /* Check multi-sample. */
            else
            {
                gctUINT samples = 0;

                gcmVERIFY_OK(gcoSURF_GetSamples(surface, &samples));

                if (samples > 1)
                {
                    texInfo->eglImage.directSample = gcvFALSE;
                }
            }
        }
    }

    do
    {
        /* Test if need to reset texture wrapper. */
        gcoSURF mipmap = gcvNULL;

        gctUINT txWidth;
        gctUINT txHeight;
        gctUINT depth;
        gceSURF_FORMAT format;
        gceSURF_TYPE type;

        if (texInfo->object == gcvNULL)
        {
            /* No texture object, do nothing here. */
            resetTexture = gcvFALSE;

            break;
        }

        if (texInfo->direct.source != gcvNULL)
        {
            /* Formerly bound a GL_VIV_direct_texture. */
            break;
        }

        /* Mipmap level 1 is available, can not skip reset. */
        if (gcmIS_SUCCESS(gcoTEXTURE_GetMipMap(texInfo->object, 1, &mipmap)))
        {
            /* This texture object has more than 1 levels, need destroy
             * and re-create again. */
            break;
        }

        /* Get mipmap level 0. */
        if (gcmIS_ERROR(gcoTEXTURE_GetMipMap(texInfo->object, 0, &mipmap)))
        {
            /* Can not get mipmap level 0, which means this texture
             * object is clean. Destroy it. */
            break;
        }

        /* Query size and format. */
        gcmERR_BREAK(gcoSURF_GetSize(mipmap, &txWidth, &txHeight, &depth));

        gcmERR_BREAK(gcoSURF_GetFormat(mipmap, &type, &format));

        if ((txWidth != (gctUINT) width) || (txHeight != (gctUINT) height) || (depth != 1))
        {
            /* Has different size. */
            break;
        }

        if (texInfo->eglImage.directSample)
        {
            break;
        }
        else
        {
            if ((format != dstFormat) || (type != gcvSURF_TEXTURE))
            {
                /* Has different dst parameters. */
                break;
            }
        }

        /* All parameters are matched, we do not need re-create
         * another texture object. */
        resetTexture = gcvFALSE;
    }
    while (gcvFALSE);

    if (resetTexture)
    {
        /* Destroy the texture. */
        gcmONERROR(gcChipResetTextureWrapper(gc, texObj));

        /* Destroy the texture object. */
        if (texInfo->object != gcvNULL)
        {
            gcmONERROR(gcoTEXTURE_Destroy(texInfo->object));
            texInfo->object = gcvNULL;
        }

        /* Commit. */
        gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));

        /* Mark texture as dirty because it'll be re-constructed. */
        dirty = gcvTRUE;
    }

    /* Reference source surface. */
    if (texInfo->eglImage.source != surface)
    {
        if (texInfo->eglImage.source != gcvNULL)
        {
            /* Decrease reference count of old surface. */
            gcmONERROR(gcoSURF_Destroy(texInfo->eglImage.source));

            /* Commit the destroy. */
            gcmONERROR(gcoHAL_Commit(gcvNULL, gcvFALSE));
        }

        /* Reference new source surface. */
        texInfo->eglImage.source = surface;

        if (texInfo->eglImage.source != gcvNULL)
        {
            /* Increase reference count of new source. */
            gcmONERROR(gcoSURF_ReferenceSurface(texInfo->eglImage.source));
        }

        /* Mark texture as dirty because its source is changed. */
        dirty = gcvTRUE;
    }

    if (texInfo->eglImage.image != eglImage)
    {
        if (texInfo->eglImage.image != gcvNULL)
        {
            /* Dereference old EGLImageKHR. */
            gc->imports.dereferenceImage(texInfo->eglImage.image);
        }

        /* Reference new EGLImageKHR. */
        texInfo->eglImage.image = eglImage;
        gc->imports.referenceImage(eglImage);
    }

#if defined(ANDROID) && ANDROID_SDK_VERSION >= 7
    if (image->type == KHR_IMAGE_ANDROID_NATIVE_BUFFER)
    {
        /* Reference android native buffer. */
        if (texInfo->eglImage.nativeBuffer != image->u.ANativeBuffer.nativeBuffer)
        {
            android_native_buffer_t * nativeBuffer;

            if (texInfo->eglImage.nativeBuffer != gcvNULL)
            {
                /* Cast to android native buffer. */
                nativeBuffer = (android_native_buffer_t *) texInfo->eglImage.nativeBuffer;

                /* Decrease native buffer reference count. */
                nativeBuffer->common.decRef(&nativeBuffer->common);
            }

            /* Reference new android native buffer. */
            texInfo->eglImage.nativeBuffer = image->u.ANativeBuffer.nativeBuffer;

            if (texInfo->eglImage.nativeBuffer != gcvNULL)
            {
                /* Cast to android native buffer. */
                nativeBuffer = (android_native_buffer_t *) texInfo->eglImage.nativeBuffer;

                /* Increase reference count. */
                nativeBuffer->common.incRef(&nativeBuffer->common);
            }
        }
    }
#endif
    if (dirty)
    {
        /* Store dirty flag. */
        texInfo->eglImage.dirty = gcvTRUE;
    }

    /* If already bound to FBO, we have to create the texture object and check for shadow */
    if (texObj->fboList)
    {
        GLuint attachIdx;
        __GLimageUser *iu = texObj->fboList;
        __GLframebufferObject *fbo = gcvNULL;

        if (texInfo->eglImage.source)
        {
            /* For EGLImage target texture, base level may be not allocated yet. */
            gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
        }

        /* Go through all attached fbos. */
        while (iu)
        {
            fbo = (__GLframebufferObject*)iu->imageUser;
            /* Texture can only be bound to user created FBO */
            if (fbo && fbo->name)
            {
                /* Go through all attach points, not only the ones selected by drawBuffers.
                ** Because we do not resize indirect RT surfaces when change drawBuffers.
                */
                for (attachIdx = 0; attachIdx < __GL_MAX_ATTACHMENTS; ++attachIdx)
                {
                    __GLfboAttachPoint* attachPoint = &fbo->attachPoint[attachIdx];
                    /* If the attachment sourced from this face level */
                    if (attachPoint->objType == GL_TEXTURE &&
                        attachPoint->objName == texObj->name &&
                        attachPoint->level == level &&
                        attachPoint->face == 0)
                    {
                        if (gcChipTexNeedShadow(gc,
                                                texObj,
                                                texInfo,
                                                chipMipLevel->formatMapInfo,
                                                attachPoint->samples,
                                                &attachPoint->samplesUsed)
                           )
                        {
                            gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, attachPoint->layered, 0, attachPoint->slice);

                            gcmONERROR(gcChipRellocShadowResource(gc,
                                                                  texView.surf,
                                                                  attachPoint->samplesUsed,
                                                                  &chipMipLevel->shadow[attachPoint->slice],
                                                                  chipMipLevel->formatMapInfo,
                                                                  GL_TRUE));
                        }
                        else
                        {
                            __GLchipResourceShadow *shadow = &chipMipLevel->shadow[attachPoint->slice];

                            if(shadow && shadow->surface)
                            {
                                gcoSURF_Destroy(shadow->surface);
                                shadow->surface = gcvNULL;
                                shadow->shadowDirty = GL_FALSE;
                            }
                        }

                        /* set update flag as it will be rendered later. not very precisely*/
                        CHIP_TEX_IMAGE_UPTODATE(texInfo, 0);
                    }
                }
            }
            iu = iu->next;
        }
    }

    texInfo->mipLevels[0].shadow[0].masterDirty = GL_TRUE;
    CHIP_TEX_IMAGE_UPTODATE(texInfo, level);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

gceSTATUS
gcChipInitializeSampler(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    do
    {
        GLint i;
        GLint j;

        /* Make sure we have samplers. */
        if (gc->constants.shaderCaps.maxCombinedTextureImageUnits == 0)
        {
            gcmERR_BREAK(gcvSTATUS_INVALID_ARGUMENT);
        }

        for (i = 0; i < (GLint)gc->constants.shaderCaps.maxCombinedTextureImageUnits; ++i)
        {
            gcmONERROR(gcoTEXTURE_InitParams(chipCtx->hal, &chipCtx->texture.halTexture[i]));
        }

        for (i = 0; i < (GLint)gc->constants.shaderCaps.maxTextureSamplers; ++i)
        {
            /* Initialize related info of key state */
            chipCtx->pgKeyState->NP2AddrMode[i].add.addrModeR = gcvTEXTURE_INVALID;
            chipCtx->pgKeyState->NP2AddrMode[i].add.addrModeT = gcvTEXTURE_INVALID;
            chipCtx->pgKeyState->NP2AddrMode[i].add.addrModeS = gcvTEXTURE_INVALID;
            chipCtx->pgKeyState->shadowMapCmpInfo[i].cmp.cmpMode = gcvTEXTURE_COMPARE_MODE_NONE;
            chipCtx->pgKeyState->shadowMapCmpInfo[i].cmp.cmpOp = gcvCOMPARE_LESS_OR_EQUAL;
            chipCtx->pgKeyState->shadowMapCmpInfo[i].texFmt = gcvSURF_UNKNOWN;
            chipCtx->pgKeyState->texPatchInfo[i].format = gcvSURF_UNKNOWN;
            chipCtx->pgKeyState->texPatchInfo[i].needFormatConvert = GL_FALSE;

            for (j = 0; j < gcvTEXTURE_COMPONENT_NUM; j++)
            {
                chipCtx->pgKeyState->texPatchInfo[i].swizzle[j] = 0;
            }
        }
        chipCtx->texture.curStageMask =
        chipCtx->texture.preStageMask = 0;

    } while (GL_FALSE);

OnError:
    gcmFOOTER();
    /* Return result. */
    return status;
}

gceSTATUS
gcChipDeinitializeSampler(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    /* Destroy tempBitmap if it is allocated */
    if (chipCtx->tempBitmap != gcvNULL)
    {
        /* Unlock the surface. */
        if (chipCtx->tempBits != gcvNULL)
        {
            gcmVERIFY_OK(gcoSURF_Unlock(chipCtx->tempBitmap, chipCtx->tempBits));
            chipCtx->tempBits = gcvNULL;
        }

        /* Destroy the surface. */
        gcmVERIFY_OK(gcoSURF_Destroy(chipCtx->tempBitmap));
        chipCtx->tempBitmap = gcvNULL;
    }

    gcmFOOTER();
    /* Return result. */
    return status;
}

