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

#define _GC_OBJ_ZONE    __GLES3_ZONE_BUFFER

static GLint MSAASample0 = 0;
static GLint MSAASamples[4] = {0};
                                                    /*requestFormat,      readFormat,        writeFormat,     flags,  patchCase,  numSampler,    samples[4] */
#define __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(b)      {b,             gcvSURF_UNKNOWN,   gcvSURF_UNKNOWN,    0,         0,          0,        &MSAASample0 }
#define __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(b,f) {b,             gcvSURF_UNKNOWN,   gcvSURF_UNKNOWN,    f,         0,          0,        &MSAASample0 }


/* OGL format naming rules:
** 1, Non packed-format, from LSB to MSB. e.g RGBA8, R is LSB, A is MSB.
** 2, Packed-format(byte-acrossed), please check table 3.2/3.2 of OES spec 3.0.1.pdf
*/
static __GLchipFmtMapInfo __glChipFmtMapInfo[__GL_FMT_MAX + 1] =
{
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8),             /* __GL_FMT_A8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_L8),             /* __GL_FMT_L8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8L8),           /* __GL_FMT_LA8 */

    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R8),             /* __GL_FMT_R8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R8_SNORM),       /* __GL_FMT_R8_SNORM */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G8R8),           /* __GL_FMT_RG8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G8R8_SNORM),     /* __GL_FMT_RG8_SNORM */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B8G8R8),         /* __GL_FMT_RGB8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B8G8R8_SNORM),   /* __GL_FMT_RGB8_SNORM */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R5G6B5),         /* __GL_FMT_RGB565 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R4G4B4A4),       /* __GL_FMT_RGBA4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R5G5B5A1),       /* __GL_FMT_RGB5_A1 */

    /* RGBA8 format need be converted ALWAYS, as its resident format is always different with its request format */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ), /* __GL_FMT_RGBA8 */

    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8R8G8B8),       /* __GL_FMT_BGRA */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8B8G8R8_SNORM), /* __GL_FMT_RGBA8_SNORM */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A2B10G10R10),    /* __GL_FMT_RGB10_A2 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SBGR8),          /* __GL_FMT_SRGB8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8_SBGR8),       /* __GL_FMT_SRGB8_ALPHA8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R16F),           /* __GL_FMT_R16F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G16R16F),        /* __GL_FMT_RG16F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B16G16R16F),     /* __GL_FMT_RGB16F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A16B16G16R16F),  /* __GL_FMT_RGBA16F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R32F),           /* __GL_FMT_R32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G32R32F),        /* __GL_FMT_RG32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B32G32R32F),     /* __GL_FMT_RGB32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A32B32G32R32F),  /* __GL_FMT_RGBA32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B10G11R11F),     /* __GL_FMT_R11F_G11F_B10F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_E5B9G9R9),       /* __GL_FMT_RGB9_E5 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R8I),            /* __GL_FMT_R8I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R8UI),           /* __GL_FMT_R8UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R16I),           /* __GL_FMT_R16I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R16UI),          /* __GL_FMT_R16UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R32I),           /* __GL_FMT_R32I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R32UI),          /* __GL_FMT_R32UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G8R8I),          /* __GL_FMT_RG8I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G8R8UI),         /* __GL_FMT_RG8UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G16R16I),        /* __GL_FMT_RG16I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G16R16UI),       /* __GL_FMT_RG16UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G32R32I),        /* __GL_FMT_RG32I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_G32R32UI),       /* __GL_FMT_RG32UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B8G8R8I),        /* __GL_FMT_RGB8I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B8G8R8UI),       /* __GL_FMT_RGB8UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B16G16R16I),     /* __GL_FMT_RGB16I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B16G16R16UI),    /* __GL_FMT_RGB16UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B32G32R32I),     /* __GL_FMT_RGB32I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_B32G32R32UI),    /* __GL_FMT_RGB32UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8B8G8R8I),      /* __GL_FMT_RGBA8I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A8B8G8R8UI),     /* __GL_FMT_RGBA8UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A16B16G16R16I),  /* __GL_FMT_RGBA16I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A16B16G16R16UI), /* __GL_FMT_RGBA16UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A32B32G32R32I),  /* __GL_FMT_RGBA32I */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A32B32G32R32UI), /* __GL_FMT_RGBA32UI */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A2B10G10R10UI),  /* __GL_FMT_RGB10_A2UI */

    /* ECT format maybe need conversion or not, dependent on flags */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ETC1),                           /* __GL_FMT_ETC1_RGB8_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_R11_EAC),                        /* __GL_FMT_R11_EAC */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SIGNED_R11_EAC),                 /* __GL_FMT_SIGNED_R11_EAC */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_RG11_EAC),                       /* __GL_FMT_RG11_EAC */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SIGNED_RG11_EAC),                /* __GL_FMT_SIGNED_RG11_EAC */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_RGB8_ETC2),                      /* __GL_FMT_RGB8_ETC2 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SRGB8_ETC2),                     /* __GL_FMT_SRGB8_ETC2 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_RGB8_PUNCHTHROUGH_ALPHA1_ETC2),  /* __GL_FMT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2), /* __GL_FMT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_RGBA8_ETC2_EAC),                 /* __GL_FMT_RGBA8_ETC2_EAC */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_SRGB8_ALPHA8_ETC2_EAC),          /* __GL_FMT_SRGB8_ALPHA8_ETC2_EAC */

    /* DXT format maybe need conversion or not, dependent on flags */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_DXT1),           /* __GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_DXT1),           /* __GL_FMT_COMPRESSED_RGBA_S3TC_DXT1_EXT */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_DXT3),           /* __GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_DXT5),           /* __GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT */

    /* palette format need be converted ALWAYS, as its resident format is always different with its request format */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A4R4G4B4, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE4_RGBA4_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A1R5G5B5, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE4_RGB5_A1_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_R5G6B5,   __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE4_R5_G6_B5_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_X8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE4_RGB8_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE4_RGBA8_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A4R4G4B4, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE8_RGBA4_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A1R5G5B5, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE8_RGB5_A1_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_R5G6B5,   __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE8_R5_G6_B5_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_X8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE8_RGB8_OES */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_PALETTE8_RGBA8_OES */


    /* depth stencil formats */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_D16),                                                 /* __GL_FMT_Z16 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_D24X8,   __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_Z24 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_D32F),                                                /* __GL_FMT_Z32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_D24S8),                                               /* __GL_FMT_Z24S8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_S8D32F),                                              /* __GL_FMT_Z32FS8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_D24S8,   __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_S1 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_D24S8,   __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ),    /* __GL_FMT_S4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_S8),                                                  /* __GL_FMT_S8 */

    /* ASTC formats. */
#if defined(GL_KHR_texture_compression_astc_ldr)
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC4x4),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_4x4_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC5x4),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_5x4_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC5x5),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_5x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC6x5),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_6x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC6x6),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_6x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x5),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_8x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x6),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_8x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x8),        /* __GL_FMT_COMPRESSED_RGBA_ASTC_8x8_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x5),       /* __GL_FMT_COMPRESSED_RGBA_ASTC_10x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x6),       /* __GL_FMT_COMPRESSED_RGBA_ASTC_10x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x8),       /* __GL_FMT_COMPRESSED_RGBA_ASTC_10x8_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x10),      /* __GL_FMT_COMPRESSED_RGBA_ASTC_10x10_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC12x10),      /* __GL_FMT_COMPRESSED_RGBA_ASTC_12x10_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC12x12),      /* __GL_FMT_COMPRESSED_RGBA_ASTC_12x12_KHR */

    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC4x4_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC5x4_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC5x5_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC6x5_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC6x6_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x5_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x6_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC8x8_SRGB),   /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x5_SRGB),  /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x6_SRGB),  /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x8_SRGB),  /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC10x10_SRGB), /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC12x10_SRGB), /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_ASTC12x12_SRGB), /* __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR */
#endif

    /* RGB8 format need be converted ALWAYS, as its resident format is always different with its request format */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_X8R8G8B8, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ), /* __GL_FMT_RGBX8 */

    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_X8R8G8B8),      /* __GL_FMT_BGRX8 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A4R4G4B4),      /* __GL_FMT_ARGB4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_A4R4G4B4, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ), /* __GL_FMT_ABGR4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_X4R4G4B4),      /* __GL_FMT_XRGB4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_WITH_FLAGS(gcvSURF_X4R4G4B4, __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ), /* __GL_FMT_XBGR4 */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A32F),           /* __GL_FMT_A32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_L32F),           /* __GL_FMT_L32F */
    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_A32L32F),        /* __GL_FMT_LA32F */


    __GL_INITIALIZE_FORMAT_MAP_INFO_DEFAULT(gcvSURF_UNKNOWN)        /* __GL_FMT_MAX */
};


/*
** format map information for patch
*/
#define __GL_CHIP_PATCH_FMT_MAX         64
static __GLchipFmtMapInfo __glChipFmtMapInfo_patch[__GL_CHIP_PATCH_FMT_MAX];
extern __GLformatInfo __glFormatInfoTable[];



/************************************************************************/
/* Implementation for EXPORTED FUNCIONS                                 */
/************************************************************************/

__GL_INLINE GLvoid
gcChipSetFmtMapAttribs(
    __GLcontext *gc,
    __GLformat  glFormat,
    __GLchipFmtMapInfo *fmtMapInfo,
    GLint maxSamples
    )
{
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);

    if (fmtMapInfo->readFormat != fmtMapInfo->requestFormat)
    {
        fmtMapInfo->flags |= __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_READ;
    }

    if (fmtMapInfo->writeFormat != fmtMapInfo->requestFormat)
    {
        fmtMapInfo->flags |= __GL_CHIP_FMTFLAGS_FMT_DIFF_REQ_WRITE;
    }

    if (fmtMapInfo->writeFormat != fmtMapInfo->readFormat)
    {
        fmtMapInfo->flags |= __GL_CHIP_FMTFLAGS_FMT_DIFF_READ_WRITE;
    }

    if (chipCtx->chipFeature.hwFeature.indirectRTT)
    {
        fmtMapInfo->flags |= __GL_CHIP_FMTFLAGS_LAYOUT_DIFF_READ_WRITE;
    }

    if (fmtMapInfo->writeFormat != gcvSURF_UNKNOWN)
    {
        if (__glFormatInfoTable[glFormat].renderable)
        {
            if ((glFormat == __GL_FMT_RGBA32F) ||
                (glFormat == __GL_FMT_RGBA16F) ||
                (glFormat == __GL_FMT_RG32F)   ||
                (glFormat == __GL_FMT_R32F)    ||
                (glFormat == __GL_FMT_RGB16F))
            {
                fmtMapInfo->numSamples = 0;
                fmtMapInfo->samples = &MSAASample0;
            }
            else if (((__glFormatInfoTable[glFormat].dataFormat == GL_RED_INTEGER)    ||
                      (__glFormatInfoTable[glFormat].dataFormat == GL_RG_INTEGER)     ||
                      (__glFormatInfoTable[glFormat].dataFormat == GL_RGB_INTEGER)    ||
                      (__glFormatInfoTable[glFormat].dataFormat == GL_RGBA_INTEGER))  &&
                     (gc->apiVersion <= __GL_API_VERSION_ES30))
            {
                fmtMapInfo->numSamples = 0;
                fmtMapInfo->samples = &MSAASample0;
            }
            else
            {
                /* we only support 1 sample mode for renderable internal non-integer formats */
                fmtMapInfo->numSamples = chipCtx->numSamples;
                fmtMapInfo->samples = MSAASamples;
                gcoOS_MemCopy(fmtMapInfo->samples, chipCtx->samples, sizeof(chipCtx->samples));
            }
        }
        else
        {
            /* we only support 1 sample mode for renderable internal formats */
            fmtMapInfo->numSamples = 0;
            fmtMapInfo->samples = &MSAASample0;
        }
    }
    else
    {
        /* we don't any multisample configurations for non-renderable format */
        fmtMapInfo->numSamples = 0;
        fmtMapInfo->samples = &MSAASample0;
    }
}

gceSTATUS
gcChipInitFormatMapInfo(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx         = CHIP_CTXINFO(gc);
    GLint maxSamples                 = (GLint)gc->constants.maxSamples;
    GLuint i, j;
    gceSTATUS status                 = gcvSTATUS_OK;
    GLuint patch3DFormatCount        = 0;
    gceSURF_FORMAT patch3DFormat     = gcvSURF_UNKNOWN;
    GLint sRGBentry                  = -1;
    GLint MSAAsRGBentry              = -1;
    GLuint patchHalfFloatCount       = 0;
    GLuint patch8bitMsaaCount        = 0;
    GLuint patchD32FCount            = 0;
    GLuint patchAstcCount            = 0;
    GLuint patchAlpha8Count          = 0;
    GLuint patchFmtMapInfoCount      = 0;
    static __GLApiVersion initializedVer = __GL_API_VERSION_INVALID;

    typedef struct __GLChipPatchFmtRec
    {
        gceSURF_FORMAT requestFormat;
        gceSURF_FORMAT readFormat;
        gceSURF_FORMAT writeFormat;
        GLint entry;
    } __GLChipPatchFmt;

    __GLChipPatchFmt patchHalfFloatFormats[] =
    {
        {gcvSURF_R16F,          gcvSURF_R8_1_X8R8G8B8,   gcvSURF_UNKNOWN, -1},
        {gcvSURF_G16R16F,       gcvSURF_G8R8_1_X8R8G8B8, gcvSURF_UNKNOWN, -1},
        {gcvSURF_B16G16R16F,    gcvSURF_X8R8G8B8,        gcvSURF_UNKNOWN, -1},
        {gcvSURF_A16B16G16R16F, gcvSURF_A8R8G8B8,        gcvSURF_UNKNOWN, -1},
    };

    __GLChipPatchFmt patch8bitMsaaFmts[] =
    {
        {gcvSURF_R8I,  gcvSURF_R8I_1_A4R4G4B4,  gcvSURF_R8I_1_A4R4G4B4,  -1},
        {gcvSURF_R8UI, gcvSURF_R8UI_1_A4R4G4B4, gcvSURF_R8UI_1_A4R4G4B4, -1},
    };

    __GLChipPatchFmt patchZ32FFmts[] =
    {
        {gcvSURF_D32F,   gcvSURF_D24X8, gcvSURF_D24X8, -1},
        {gcvSURF_S8D32F, gcvSURF_D24S8, gcvSURF_D24S8, -1},
    };

    __GLChipPatchFmt patchAstcFmts[] =
    {
        {gcvSURF_ASTC4x4,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC5x4,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC5x5,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC6x5,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC6x6,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x5,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x6,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x8,           gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x5,          gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x6,          gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x8,          gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x10,         gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC12x10,         gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC12x12,         gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC4x4_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC5x4_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC5x5_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC6x5_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC6x6_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x5_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x6_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC8x8_SRGB,      gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x5_SRGB,     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x6_SRGB,     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x8_SRGB,     gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC10x10_SRGB,    gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC12x10_SRGB,    gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
        {gcvSURF_ASTC12x12_SRGB,    gcvSURF_A8R8G8B8, gcvSURF_A8R8G8B8, -1},
    };

    __GLChipPatchFmt patchAlpha8Formats[] =
    {
        {gcvSURF_A8, gcvSURF_A8_1_A8R8G8B8, gcvSURF_UNKNOWN, -1},
    };

    GLuint halfFloatTableSize = 0;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (chipCtx->chipFeature.hwFeature.supportMSAA2X)
    {
        chipCtx->numSamples = 2;
        chipCtx->samples[0] = 2;
        chipCtx->samples[1] = maxSamples;
    }
    else
    {
        chipCtx->numSamples = 1;
        chipCtx->samples[0] = maxSamples;
    }

    if (initializedVer == gc->apiVersion)
    {
        gcmFOOTER();
        return gcvSTATUS_OK;
    }

    if (chipCtx->patchId == gcvPATCH_GTFES30 &&
        !chipCtx->chipFeature.hwFeature.hasHalfFloatPipe)
    {
        halfFloatTableSize = gcmCOUNTOF(patchHalfFloatFormats);
    }

    for (i = 0; i < __GL_FMT_MAX; ++i)
    {
        gcmONERROR(gcoTEXTURE_GetClosestFormat(chipCtx->hal, __glChipFmtMapInfo[i].requestFormat, &__glChipFmtMapInfo[i].readFormat));
        gcmONERROR(gco3D_GetClosestRenderFormat(chipCtx->engine, __glChipFmtMapInfo[i].requestFormat, &__glChipFmtMapInfo[i].writeFormat));
        gcmONERROR(gcoTEXTURE_GetClosestFormatEx(chipCtx->hal, __glChipFmtMapInfo[i].requestFormat, gcvTEXTURE_3D, &patch3DFormat));

        gcChipSetFmtMapAttribs(gc, i, &__glChipFmtMapInfo[i], maxSamples);

        /* count 3d format patch */
        if (patch3DFormat != __glChipFmtMapInfo[i].readFormat)
        {
            patch3DFormatCount++;
        }

        /* count SRGB rendering */
        if(!chipCtx->chipFeature.hwFeature.hasSRGBRT)
        {
            if ((__glChipFmtMapInfo[i].requestFormat == gcvSURF_A8_SBGR8) &&
                (__glChipFmtMapInfo[i].writeFormat != gcvSURF_A16B16G16R16F))
            {
                sRGBentry = i;
            }
            else if ((__glChipFmtMapInfo[i].requestFormat == gcvSURF_A8_SBGR8) &&
                    (__glChipFmtMapInfo[i].writeFormat == gcvSURF_A16B16G16R16F))
            {
                MSAAsRGBentry = i;
            }
        }

        for (j = 0; j < halfFloatTableSize; ++j)
        {
            if (patchHalfFloatFormats[j].requestFormat == __glChipFmtMapInfo[i].requestFormat)
            {
                patchHalfFloatFormats[j].entry = i;
                patchHalfFloatCount++;
            }
        }

        for (j = 0; j < gcmCOUNTOF(patch8bitMsaaFmts); ++j)
        {
            if (patch8bitMsaaFmts[j].requestFormat == __glChipFmtMapInfo[i].requestFormat)
            {
                patch8bitMsaaFmts[j].entry = i;
                patch8bitMsaaCount++;
            }
        }

        for (j = 0; j < gcmCOUNTOF(patchZ32FFmts); ++j)
        {
            if (patchZ32FFmts[j].requestFormat == __glChipFmtMapInfo[i].requestFormat)
            {
                patchZ32FFmts[j].entry = i;
                patchD32FCount++;
            }
        }

        for (j = 0; j < gcmCOUNTOF(patchAstcFmts); ++j)
        {
            if (patchAstcFmts[j].requestFormat == __glChipFmtMapInfo[i].requestFormat)
            {
                patchAstcFmts[j].entry = i;
                patchAstcCount++;
            }
        }

        for (j = 0; j < gcmCOUNTOF(patchAlpha8Formats); ++j)
        {
            if (patchAlpha8Formats[j].requestFormat == __glChipFmtMapInfo[i].requestFormat)
            {
                patchAlpha8Formats[j].entry = i;
                patchAlpha8Count++;
            }
        }
    }

    /* case0 */
    patchFmtMapInfoCount = patch3DFormatCount;

    /* case1 */
    patchFmtMapInfoCount += (sRGBentry == -1) ? 0: 1;

    /* case2 */
    patchFmtMapInfoCount ++;

    /* case3 */
    patchFmtMapInfoCount += patchHalfFloatCount;

    /* case4 */
    patchFmtMapInfoCount += (MSAAsRGBentry == -1) ? 0 : 1;

    /* case5: 8 bit msaa */
    patchFmtMapInfoCount += patch8bitMsaaCount;

    /* case6: D32F */
    patchFmtMapInfoCount += patchD32FCount;

    /* case7: ASTC */
    patchFmtMapInfoCount += patchAstcCount;

    /* case8 : ALPHA8*/
    patchFmtMapInfoCount += patchAlpha8Count;

    GL_ASSERT(patchFmtMapInfoCount < __GL_CHIP_PATCH_FMT_MAX);

    if (patchFmtMapInfoCount)
    {
        GLuint index = 0;

        gcoOS_ZeroMemory(__glChipFmtMapInfo_patch, __GL_CHIP_PATCH_FMT_MAX * gcmSIZEOF(__GLchipFmtMapInfo));

        if (patch3DFormatCount)
        {
            /* 1. build format map information to patch 2d array/3d texture */
            for (i = 0; i < __GL_FMT_MAX; i++)
            {
                gcmONERROR(gcoTEXTURE_GetClosestFormatEx(chipCtx->hal, __glChipFmtMapInfo[i].requestFormat, gcvTEXTURE_3D, &patch3DFormat));

                if ((patch3DFormat != __glChipFmtMapInfo[i].readFormat) && (index < __GL_CHIP_PATCH_FMT_MAX))
                {
                    __glChipFmtMapInfo_patch[index].requestFormat = __glChipFmtMapInfo[i].requestFormat;
                    __glChipFmtMapInfo_patch[index].readFormat    = patch3DFormat;
                    __glChipFmtMapInfo_patch[index].writeFormat   = __glChipFmtMapInfo[i].writeFormat;
                    __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_CASE0;
                    __glChipFmtMapInfo_patch[index].flags         = (__glChipFmtMapInfo[i].flags & __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ);

                    gcChipSetFmtMapAttribs(gc, i, &__glChipFmtMapInfo_patch[index], maxSamples);

                    index++;
                }
            }
        }

        /* 2. build case1 mapping information */
        if ((sRGBentry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
        {
            __glChipFmtMapInfo_patch[index].requestFormat = __glChipFmtMapInfo[sRGBentry].requestFormat;
            __glChipFmtMapInfo_patch[index].readFormat    = __glChipFmtMapInfo[sRGBentry].readFormat;
            __glChipFmtMapInfo_patch[index].writeFormat   = gcvSURF_A8B12G12R12_2_A8R8G8B8;
            __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_CASE1;
            __glChipFmtMapInfo_patch[index].flags         = (__glChipFmtMapInfo[sRGBentry].flags & __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ);

            gcChipSetFmtMapAttribs(gc, sRGBentry, &__glChipFmtMapInfo_patch[index], maxSamples);

            index++;
        }

        /* 3. case2: add Z24 force mapping */
        if (index < __GL_CHIP_PATCH_FMT_MAX)
        {
            __glChipFmtMapInfo_patch[index].requestFormat = gcvSURF_D16;
            __glChipFmtMapInfo_patch[index].readFormat    = gcvSURF_D24X8;
            __glChipFmtMapInfo_patch[index].writeFormat   = gcvSURF_D24X8;
            __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_CASE2;
            __glChipFmtMapInfo_patch[index].flags         = 0;

            gcChipSetFmtMapAttribs(gc, __GL_FMT_Z16, &__glChipFmtMapInfo_patch[index], maxSamples);

            index++;
        }

        /* 4. case3: add half float force mapping */
        for (i = 0; i < halfFloatTableSize; ++i)
        {
            GLint entry = patchHalfFloatFormats[i].entry;

            GL_ASSERT(entry != -1)
            if ((entry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
            {
                __glChipFmtMapInfo_patch[index].requestFormat = patchHalfFloatFormats[i].requestFormat;
                __glChipFmtMapInfo_patch[index].readFormat    = patchHalfFloatFormats[i].readFormat;
                __glChipFmtMapInfo_patch[index].writeFormat   = __glChipFmtMapInfo[entry].writeFormat;
                __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_CASE3;
                __glChipFmtMapInfo_patch[index].flags         = __glChipFmtMapInfo[entry].flags;

                gcChipSetFmtMapAttribs(gc, entry, &__glChipFmtMapInfo_patch[index], maxSamples);
            }
            index++;
        }

        /* 5. case4: add MSAA sRGB force mapping */
        if ((MSAAsRGBentry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
        {
            __glChipFmtMapInfo_patch[index].requestFormat = __glChipFmtMapInfo[MSAAsRGBentry].requestFormat;
            __glChipFmtMapInfo_patch[index].readFormat    = __glChipFmtMapInfo[MSAAsRGBentry].readFormat;
            __glChipFmtMapInfo_patch[index].writeFormat   = gcvSURF_A16B16G16R16F_2_G16R16F;
            __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_CASE4;
            __glChipFmtMapInfo_patch[index].flags         = (__glChipFmtMapInfo[MSAAsRGBentry].flags & __GL_CHIP_FMTFLAGS_FMT_DIFF_CORE_REQ);

            gcChipSetFmtMapAttribs(gc, MSAAsRGBentry, &__glChipFmtMapInfo_patch[index], maxSamples);

            index++;
        }

        /* 6: 8 bit msaa */
        for (i = 0; i < gcmCOUNTOF(patch8bitMsaaFmts); ++i)
        {
            GLint entry = patch8bitMsaaFmts[i].entry;

            GL_ASSERT(entry != -1)
            if ((entry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
            {
                __glChipFmtMapInfo_patch[index].requestFormat = patch8bitMsaaFmts[i].requestFormat;
                __glChipFmtMapInfo_patch[index].readFormat    = patch8bitMsaaFmts[i].readFormat;
                __glChipFmtMapInfo_patch[index].writeFormat   = patch8bitMsaaFmts[i].writeFormat;
                __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_8BIT_MSAA;
                __glChipFmtMapInfo_patch[index].flags         = __glChipFmtMapInfo[entry].flags;

                gcChipSetFmtMapAttribs(gc, entry, &__glChipFmtMapInfo_patch[index], maxSamples);
            }
            index++;
        }

        /* 7: D32F */
        for (i = 0; i < gcmCOUNTOF(patchZ32FFmts); ++i)
        {
            GLint entry = patchZ32FFmts[i].entry;

            GL_ASSERT(entry != -1);
            if ((entry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
            {
                __glChipFmtMapInfo_patch[index].requestFormat = patchZ32FFmts[i].requestFormat;
                __glChipFmtMapInfo_patch[index].readFormat    = patchZ32FFmts[i].readFormat;
                __glChipFmtMapInfo_patch[index].writeFormat   = patchZ32FFmts[i].writeFormat;
                __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_D32F;
                __glChipFmtMapInfo_patch[index].flags         = 0;

                gcChipSetFmtMapAttribs(gc, entry, &__glChipFmtMapInfo_patch[index], maxSamples);
            }
            index++;
        }

        /* 8: ASTC */
        for (i = 0; i < gcmCOUNTOF(patchAstcFmts); ++i)
        {
            GLint entry = patchAstcFmts[i].entry;

            GL_ASSERT(entry != -1);
            if ((entry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
            {
                __glChipFmtMapInfo_patch[index].requestFormat = patchAstcFmts[i].requestFormat;
                __glChipFmtMapInfo_patch[index].readFormat    = patchAstcFmts[i].readFormat;
                __glChipFmtMapInfo_patch[index].writeFormat   = patchAstcFmts[i].writeFormat;
                __glChipFmtMapInfo_patch[index].patchCase     = __GL_CHIP_FMT_PATCH_ASTC;
                __glChipFmtMapInfo_patch[index].flags         = 0;

                gcChipSetFmtMapAttribs(gc, entry, &__glChipFmtMapInfo_patch[index], maxSamples);
            }
            index++;
        }

        for (i = 0; i < gcmCOUNTOF(patchAlpha8Formats); ++i)
        {
            GLint entry = patchAlpha8Formats[i].entry;

            GL_ASSERT(entry != -1);
            if ((entry != -1) && (index < __GL_CHIP_PATCH_FMT_MAX))
            {
                __glChipFmtMapInfo_patch[index].requestFormat = patchAlpha8Formats[i].requestFormat;
                __glChipFmtMapInfo_patch[index].readFormat = patchAlpha8Formats[i].readFormat;
                __glChipFmtMapInfo_patch[index].writeFormat = patchAlpha8Formats[i].writeFormat;
                __glChipFmtMapInfo_patch[index].patchCase = __GL_CHIP_FMT_PATCH_ALPHA8;
                __glChipFmtMapInfo_patch[index].flags = 0;

                gcChipSetFmtMapAttribs(gc, entry, &__glChipFmtMapInfo_patch[index], maxSamples);
            }
            index++;
        }

        GL_ASSERT(index <= patchFmtMapInfoCount);
    }

    initializedVer = gc->apiVersion;

OnError:
    gcmFOOTER();
    return status;
}


__GLchipFmtMapInfo*
gcChipGetFormatMapInfo(
    __GLcontext *gc,
    __GLformat  drvFormat,
    __GLchipFmtPatch patchCase
    )
{
    GLuint index;
    __GLchipFmtMapInfo *formatMapInfo = &__glChipFmtMapInfo[drvFormat];

    gcmHEADER_ARG("gc=0x%x drvFormat=%d patchCase=%d", gc, drvFormat, patchCase);

    if (patchCase != __GL_CHIP_FMT_PATCH_NONE)
    {
        for (index = 0; index < __GL_CHIP_PATCH_FMT_MAX; index++)
        {
            if ((__glChipFmtMapInfo_patch[index].requestFormat == formatMapInfo->requestFormat) &&
                (__glChipFmtMapInfo_patch[index].patchCase == patchCase))
            {
                formatMapInfo = &__glChipFmtMapInfo_patch[index];
                break;
            }
        }
    }

    gcmFOOTER_NO();
    return formatMapInfo;
}

__GL_INLINE gceBUFOBJ_TYPE
gcChipMapBufObjType(
    GLuint targetIdx
    )
{
    gceBUFOBJ_TYPE bufObjType = 0;

    switch (targetIdx)
    {
    case __GL_ARRAY_BUFFER_INDEX:
        bufObjType = gcvBUFOBJ_TYPE_ARRAY_BUFFER;
        break;
    case __GL_ELEMENT_ARRAY_BUFFER_INDEX:
        bufObjType = gcvBUFOBJ_TYPE_ELEMENT_ARRAY_BUFFER;
        break;
    default:
        bufObjType = gcvBUFOBJ_TYPE_GENERIC_BUFFER;
        break;
    }

    return bufObjType;
}


__GL_INLINE gceBUFOBJ_USAGE
gcChipMapBufObjUsage(
    GLenum usage
    )
{
    gceBUFOBJ_USAGE bufObjUsage = 0;

    switch (usage)
    {
    case GL_STATIC_DRAW:
        bufObjUsage = gcvBUFOBJ_USAGE_STATIC_DRAW;
        break;
    case GL_STATIC_READ:
        bufObjUsage = gcvBUFOBJ_USAGE_STATIC_READ;
        break;
    case GL_STATIC_COPY:
        bufObjUsage = gcvBUFOBJ_USAGE_STATIC_COPY;
        break;
    case GL_STREAM_DRAW:
        bufObjUsage = gcvBUFOBJ_USAGE_STREAM_DRAW;
        break;
    case GL_STREAM_READ:
        bufObjUsage = gcvBUFOBJ_USAGE_STREAM_READ;
        break;
    case GL_STREAM_COPY:
        bufObjUsage = gcvBUFOBJ_USAGE_STREAM_COPY;
        break;
    case GL_DYNAMIC_DRAW:
        bufObjUsage = gcvBUFOBJ_USAGE_DYNAMIC_DRAW;
        break;
    case GL_DYNAMIC_READ:
        bufObjUsage = gcvBUFOBJ_USAGE_DYNAMIC_READ;
        break;
    case GL_DYNAMIC_COPY:
        bufObjUsage = gcvBUFOBJ_USAGE_DYNAMIC_COPY;
        break;
    default:
        GL_ASSERT(0);
        break;
    }

    return bufObjUsage;
}


GLboolean
__glChipBindBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u",gc, bufObj, targetIndex);

    /* If this is the first time that the buffer object is bound */
    if (bufObj->privateData == gcvNULL)
    {
        bufInfo = (__GLchipVertexBufferInfo*)gc->imports.calloc(gcvNULL, 1, sizeof(__GLchipVertexBufferInfo));
        GL_ASSERT(bufInfo);
        bufObj->privateData = bufInfo;
    }
    else
    {
        bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    }

    if (!bufInfo->bufObj)
    {
        /* Construct bufobj */
        gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcChipMapBufObjType(targetIndex), &bufInfo->bufObj));
    }

    if (targetIndex == __GL_ELEMENT_ARRAY_BUFFER_INDEX)
    {
        bufInfo->boundAsIB = GL_TRUE;
    }

#if __GL_CHIP_PATCH_ENABLED
    chipCtx->patchInfo.bufBindDirty = GL_TRUE;
#endif

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipDeleteBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj
    )
{
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo *)(bufObj->privateData);

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x",gc, bufObj);

    GL_ASSERT(bufInfo);

    do
    {
        if (bufInfo->listIndexEven)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Destroy(bufInfo->listIndexEven));
            bufInfo->listIndexEven = gcvNULL;
        }

        if (bufInfo->listIndexOdd)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Destroy(bufInfo->listIndexOdd));
            bufInfo->listIndexOdd = gcvNULL;
        }

        if (bufInfo->shiftObj)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Destroy(bufInfo->shiftObj));
            bufInfo->shiftObj = gcvNULL;
        }

        if (bufInfo->bufObj)
        {
            gcmVERIFY_OK(gcoBUFOBJ_Destroy(bufInfo->bufObj));
            bufInfo->bufObj = gcvNULL;
        }

#if __GL_CHIP_PATCH_ENABLED
        /* Clean up the buffer. */
        gcChipPatchDeleteBuffer(gc, bufInfo);

        if (bufInfo->cache)
        {
            gc->imports.free(gc, bufInfo->cache);
            bufInfo->cache = gcvNULL;
        }
#endif

        gc->imports.free(gc, bufInfo);
        bufObj->privateData = gcvNULL;
    } while (GL_FALSE);



    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

GLvoid*
__glChipMapBufferRange(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length,
    GLbitfield access
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    GLvoid *retPtr = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u offset=%ld length=%ld access=0x%x",
                   gc, bufObj, targetIndex, offset, length, access);
    do
    {
        GL_ASSERT(gcvNULL == bufInfo->bufferMapPointer);

        if ((access & GL_MAP_UNSYNCHRONIZED_BIT) == 0)
        {
            __glChipFlush(gc, gcvFALSE);

            if (gcvSTATUS_FALSE == gcoBUFOBJ_IsFenceEnabled(bufInfo->bufObj))
            {
                __glChipFinish(gc);
            }
            else
            {
                gcmONERROR(gcoBUFOBJ_WaitFence(bufInfo->bufObj, gcvFENCE_TYPE_ALL));
            }
        }

        if (bufInfo->bufObj)
        {
            gctUINT32 physical = 0;
            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, &bufInfo->bufferMapPointer));
#if gcdDUMP
            if (access & GL_MAP_READ_BIT)
            {
                gcmDUMP(gcvNULL, "#[info: verify mapped bufobj");
                gcmDUMP_BUFFER(gcvNULL,
                               "verify",
                               physical,
                               (gctPOINTER)bufInfo->bufferMapPointer,
                               (gctUINT32)offset,
                               (gctSIZE_T)length);
            }
#endif


            GL_ASSERT(bufInfo->bufferMapPointer);
            if(!bufInfo->bufferMapPointer)
            {
                break;
            }
            gcmONERROR(gcoBUFOBJ_CPUCacheOperation_Range(bufInfo->bufObj, (gctSIZE_T)offset, (gctSIZE_T)length, gcvCACHE_INVALIDATE));
            bufInfo->isMapped = GL_TRUE;
        }

        /* Map success */
        bufObj->mapOffset    = offset;
        bufObj->mapLength    = length;
        bufObj->mapPointer   = (gctUINT8_PTR)bufInfo->bufferMapPointer + (gctSIZE_T)offset;
        bufObj->bufferMapped = GL_TRUE;
    }
    while (gcvFALSE);

    retPtr = (access & GL_MAP_BUFFER_OBJ_VIV) ? bufInfo->bufObj : bufObj->mapPointer;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=0x%x", retPtr);
    return retPtr;
}

GLboolean
__glChipFlushMappedBufferRange(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr length
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u offset=%ld length=%ld",
                   gc, bufObj, targetIndex, offset, length);

    gcmONERROR(gcoBUFOBJ_CPUCacheOperation_Range(bufInfo->bufObj,
                                                (gctSIZE_T)(bufObj->mapOffset + offset),
                                                (gctSIZE_T)length,
                                                 gcvCACHE_CLEAN));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipUnMapBufferObject(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u",gc, bufObj, targetIndex);

    GL_ASSERT(bufInfo);

    if (bufInfo->isMapped)
    {
        if (bufInfo->bufObj)
        {
            GLboolean mapWrite = GL_FALSE;

            switch (targetIndex)
            {
            case __GL_ARRAY_BUFFER_INDEX:
                mapWrite = (bufObj->accessFlags & GL_MAP_WRITE_BIT || bufObj->accessOES == GL_WRITE_ONLY_OES);
                break;

            case __GL_ELEMENT_ARRAY_BUFFER_INDEX:
                mapWrite = (bufObj->accessFlags & GL_MAP_WRITE_BIT || bufObj->accessOES == GL_WRITE_ONLY_OES);
                if (mapWrite)
                {
                    gcoBUFOBJ_SetDirty(bufInfo->bufObj);
                }
                break;

            default:
                mapWrite = (bufObj->accessFlags & GL_MAP_WRITE_BIT);
                break;
            }

            if (mapWrite)
            {
                if (bufInfo->boundAsIB)
                {
                    bufInfo->patchDirty = gcvTRUE;
                }

                gcmONERROR(gcoBUFOBJ_CPUCacheOperation_Range(bufInfo->bufObj,
                                                             (gctSIZE_T)bufObj->mapOffset,
                                                             (gctSIZE_T)bufObj->mapLength,
                                                             gcvCACHE_FLUSH));
                gcmONERROR(gcoBUFOBJ_GPUCacheOperation(bufInfo->bufObj));

                gcoBUFOBJ_Dump(bufInfo->bufObj);
            }
            gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
        }

        bufInfo->bufferMapPointer = gcvNULL;
        bufInfo->isMapped = GL_FALSE;
    }

    /* Reset the mapping state */
    bufObj->bufferMapped = GL_FALSE;
    bufObj->mapPointer   = gcvNULL;
    bufObj->mapOffset    = 0;
    bufObj->mapLength    = 0;
    bufObj->accessFlags  = 0;
    bufObj->accessOES    = GL_WRITE_ONLY_OES;

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipBufferData(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    const void *data
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u data=0x%x",gc, bufObj, targetIndex, data);

    GL_ASSERT(bufInfo);

    do
    {
        gctSIZE_T newSize = (gctSIZE_T)bufObj->size;
        GLboolean resized = (bufInfo->size == newSize) ? GL_FALSE : GL_TRUE;

        /* Set new size */
        bufInfo->size = newSize;
        bufInfo->usage = gcChipMapBufObjUsage(bufObj->usage);

        /* Unmap if it is mapped */
        if (bufObj->bufferMapped)
        {
            if (!__glChipUnMapBufferObject(gc, bufObj, targetIndex))
            {
                gcmONERROR(gcvSTATUS_INVALID_REQUEST);
            }
        }

        /* If resized, destroy the previous memory node first */
        if (bufInfo->bufObj && resized)
        {
            gcmERR_BREAK(gcoBUFOBJ_Destroy(bufInfo->bufObj));
            bufInfo->bufObj = gcvNULL;
        }

        /* If request size > 0 and the HAL does not exist  */
        if (newSize > 0 && !bufInfo->bufObj)
        {
            /* Construct HAL pbo object. */
            gcmERR_BREAK(gcoBUFOBJ_Construct(chipCtx->hal, gcChipMapBufObjType(targetIndex), &bufInfo->bufObj));
            if (!data)
            {
                gcmERR_BREAK(gcoBUFOBJ_Upload(bufInfo->bufObj, gcvNULL, 0, newSize, bufInfo->usage));
            }
        }

        if (newSize > 0 && data)
        {
            gcmERR_BREAK(gcoBUFOBJ_Upload(bufInfo->bufObj, data, 0, newSize, bufInfo->usage));
        }

        /* Set patch as dirty */
        if (bufInfo->boundAsIB)
        {
            bufInfo->patchDirty = gcvTRUE;
        }

#if __GL_CHIP_PATCH_ENABLED
        if (chipCtx->patchInfo.patchFlags.vertexPack && (bufObj->usage == GL_STATIC_DRAW))
        {
            if (resized && bufInfo->cache)
            {
                gc->imports.free(gc, bufInfo->cache);
                bufInfo->cache = gcvNULL;
            }

            if (newSize > 0 && !bufInfo->cache)
            {
                bufInfo->cache = gc->imports.malloc(gc, newSize);
            }

            if (data)
            {
                gcoOS_MemCopy(bufInfo->cache, data, newSize);
            }
        }
#endif

        /* Set buffer as dirty */
        if (bufInfo->bufObj != gcvNULL)
        {
            gcoBUFOBJ_SetDirty(bufInfo->bufObj);
        }

    } while (GL_FALSE);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLboolean
__glChipBufferSubData(
    __GLcontext *gc,
    __GLbufferObject *bufObj,
    GLuint targetIndex,
    GLintptr offset,
    GLsizeiptr size,
    const void* data
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(bufObj->privateData);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x bufObj=0x%x targetIndex=%u offset=%ld size=%ld data=0x%x",
                   gc, bufObj, targetIndex, offset, size, data);

    do
    {
        __GLimageUser *bindUser;

        gctBOOL bPatch = gcvFALSE;
        if(chipCtx->patchInfo.patchFlags.isNavi &&
           chipCtx->chipModel == gcv2000        &&
           chipCtx->chipRevision == 0x5108)
        {
            bPatch = gcvTRUE;
        }

        /* Upload data */
        gcmERR_BREAK(gcoBUFOBJ_Upload(bufInfo->bufObj,
                                      data,
                                      (gctSIZE_T)offset,
                                      (gctSIZE_T)size,
                                      bPatch ? (bufInfo->usage | gcvBUFOBJ_USAGE_DISABLE_FENCE_DYNAMIC_STREAM): bufInfo->usage ));

        if (bufInfo->boundAsIB)
        {
            bufInfo->patchDirty = gcvTRUE;
        }

#if __GL_CHIP_PATCH_ENABLED
        if (bufInfo->cache)
        {
            gcoOS_MemCopy((GLubyte*)bufInfo->cache + (gctSIZE_T)offset, data, (gctSIZE_T)size);

            if (__GL_ELEMENT_ARRAY_BUFFER_INDEX == targetIndex)
            {
                gcChipPatchDirtyClipInfo(gc, bufInfo, (gctSIZE_T)offset, (gctSIZE_T)size);
            }
        }
#endif

        /* Set buffer as dirty */
        gcoBUFOBJ_SetDirty(bufInfo->bufObj);

        /* From spec view, BufSubData will not realloc buffer storage and need not reprogram bufAddr.
        ** But actually our implementation did. So put setting dirty code here instead of GLcore layer.
        */
        bindUser = bufObj->bindList;
        while (bindUser)
        {
            GLuint binding = __GL_PTR2UINT(bindUser->imageUser);

            GLuint targetIdx = (binding >> 16);
            GLuint arrayIdx = (binding & 0xFFFF);

            GL_ASSERT(targetIdx < __GL_MAX_BUFFER_INDEX);

            /* If the bufObj is really bound to the point, mark dirty so validation will program
            ** the new allocated address to HW
            */
            if (arrayIdx < gc->bufferObject.maxBufBindings[targetIdx] &&
                gc->bufferObject.bindingPoints[targetIdx][arrayIdx].boundBufObj == bufObj)
            {
                __glBitmaskSet(&gc->bufferObject.bindingDirties[targetIdx], arrayIdx);
            }

            bindUser = bindUser->next;
        }
    } while (GL_FALSE);

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
__glChipCopyBufferSubData(
    __GLcontext* gc,
    GLuint readTargetIndex,
    __GLbufferObject* readBufObj,
    GLuint writeTargetIndex,
    __GLbufferObject* writeBufObj,
    GLintptr readOffset,
    GLintptr writeOffset,
    GLsizeiptr size
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipVertexBufferInfo *readBufInfo = (__GLchipVertexBufferInfo*)(readBufObj->privateData);
    gctPOINTER logical = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x readTargetIndex=%u readBufObj=0x%x writeTargetIndex=%u "
                  "writeBufObj=0x%x readOffset=%ld writeOffset=%ld size=%ld",
                   gc, readTargetIndex, readBufObj, writeTargetIndex, writeBufObj,
                   readOffset, writeOffset, size);

    /* Lock Buffer */
    GL_ASSERT(readBufInfo->bufObj);

    gcoBUFOBJ_WaitFence(readBufInfo->bufObj, gcvFENCE_TYPE_WRITE);

    gcmONERROR(gcoBUFOBJ_Lock(readBufInfo->bufObj, gcvNULL, &logical));
    /* Invalidate CPU cache for the buf may be written by GPU previously. */
    gcmONERROR(gcoBUFOBJ_CPUCacheOperation_Range(readBufInfo->bufObj, (gctSIZE_T)readOffset,
                                                 (gctSIZE_T)size, gcvCACHE_INVALIDATE));

    if (logical)
    {
        /* Adjust source offset */
        logical =(gctPOINTER)((GLubyte*)logical + readOffset);
        __glChipBufferSubData(gc, writeBufObj, writeTargetIndex, writeOffset, size, logical);
    }

    /* Unlock */
    gcoBUFOBJ_Unlock(readBufInfo->bufObj);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

