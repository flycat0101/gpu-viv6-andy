/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

#define _GC_OBJ_ZONE    __GLES3_ZONE_STATE

#if gcdFRAMEINFO_STATISTIC
extern GLint g_dbgDumpImagePerDraw;
#endif

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/
__GL_INLINE GLvoid
gcChipUtilConvertMinFilter(
    GLenum minFilter,
    gceTEXTURE_FILTER *halMin,
    gceTEXTURE_FILTER *halMip
    )
{
    gcmHEADER_ARG("minFilter=0x%04x halMin=0x%x halMip=0x%x", minFilter, halMin, halMip);

    GL_ASSERT(halMin && halMip);

    switch (minFilter)
    {
    case GL_NEAREST:
        *halMin = gcvTEXTURE_POINT;
        *halMip = gcvTEXTURE_NONE;
        break;
    case GL_LINEAR:
        *halMin = gcvTEXTURE_LINEAR;
        *halMip = gcvTEXTURE_NONE;
        break;
    case GL_NEAREST_MIPMAP_NEAREST:
        *halMin = gcvTEXTURE_POINT;
        *halMip = gcvTEXTURE_POINT;
        break;
    case GL_NEAREST_MIPMAP_LINEAR:
        *halMin = gcvTEXTURE_POINT;
        *halMip = gcvTEXTURE_LINEAR;
        break;
    case GL_LINEAR_MIPMAP_NEAREST:
        *halMin = gcvTEXTURE_LINEAR;
        *halMip = gcvTEXTURE_POINT;
        break;
    case GL_LINEAR_MIPMAP_LINEAR:
        *halMin = gcvTEXTURE_LINEAR;
        *halMip = gcvTEXTURE_LINEAR;
        break;
    default:
        GL_ASSERT(0);
        *halMin = gcvTEXTURE_POINT;
        *halMip = gcvTEXTURE_POINT;
        break;
    }

    gcmFOOTER_NO();
    return;
}

__GL_INLINE gceTEXTURE_ADDRESSING
gcChipUtilConvertWrapMode(
    GLenum mode
    )
{
    gceTEXTURE_ADDRESSING ret;

    gcmHEADER_ARG("mode=0x%04x", mode);
    switch (mode)
    {
        case GL_REPEAT:
            ret = gcvTEXTURE_WRAP;
            break;
        case GL_CLAMP_TO_EDGE:
            ret = gcvTEXTURE_CLAMP;
            break;
        case GL_MIRRORED_REPEAT:
            ret = gcvTEXTURE_MIRROR;
            break;
        case GL_CLAMP_TO_BORDER_EXT:
            ret = gcvTEXTURE_BORDER;
            break;
        default:
            ret = gcvTEXTURE_INVALID;
            break;
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

__GL_INLINE gceTEXTURE_SWIZZLE
gcChipUtilConvertTexSwizzle(
    GLenum swizzle
    )
{
    gceTEXTURE_SWIZZLE ret;
    gcmHEADER_ARG("swizzle=0x%04x", swizzle);
    switch (swizzle)
    {
    case GL_RED:
        ret = gcvTEXTURE_SWIZZLE_R;
        break;
    case GL_GREEN:
        ret = gcvTEXTURE_SWIZZLE_G;
        break;
    case GL_BLUE:
        ret = gcvTEXTURE_SWIZZLE_B;
        break;
    case GL_ALPHA:
        ret = gcvTEXTURE_SWIZZLE_A;
        break;
    case GL_ZERO:
        ret = gcvTEXTURE_SWIZZLE_0;
        break;
    case GL_ONE:
        ret = gcvTEXTURE_SWIZZLE_1;
        break;
    default:
        GL_ASSERT(0);
        ret = gcvTEXTURE_SWIZZLE_INVALID;
        break;
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}


__GL_INLINE gceCOMPARE
gcChipUtilConvertCompareFunc(
    GLenum compareFunc
    )
{
    gceCOMPARE ret;
    gcmHEADER_ARG("compareFunc=0x%04x", compareFunc);
    switch (compareFunc)
    {
    case GL_LEQUAL:
        ret = gcvCOMPARE_LESS_OR_EQUAL;
        break;
    case GL_GEQUAL:
        ret = gcvCOMPARE_GREATER_OR_EQUAL;
        break;
    case GL_LESS:
        ret = gcvCOMPARE_LESS;
        break;
    case GL_GREATER:
        ret = gcvCOMPARE_GREATER;
        break;
    case GL_EQUAL:
        ret = gcvCOMPARE_EQUAL;
        break;
    case GL_NOTEQUAL:
        ret = gcvCOMPARE_NOT_EQUAL;
        break;
    case GL_ALWAYS:
        ret = gcvCOMPARE_ALWAYS;
        break;
    case GL_NEVER:
        ret = gcvCOMPARE_NEVER;
        break;
    default:
        GL_ASSERT(0);
        ret = gcvCOMPARE_INVALID;
        break;
    }
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}


__GL_INLINE gceTEXTURE_COMPARE_MODE
gcChipUtilConvertCompareMode(
    GLenum compareMode
    )
{
    gceTEXTURE_COMPARE_MODE ret;
    gcmHEADER_ARG("compareMode=0x%04x", compareMode);
    switch (compareMode)
    {
    case GL_NONE:
        ret = gcvTEXTURE_COMPARE_MODE_NONE;
        break;

    case GL_COMPARE_REF_TO_TEXTURE:
        ret = gcvTEXTURE_COMPARE_MODE_REF;
        break;

    default:
        GL_ASSERT(0);
        ret = gcvTEXTURE_COMPARE_MODE_INVALID;
        break;
    }
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

__GL_INLINE gceTEXTURE_DS_MODE
gcChipUtilConvertDSMode(
    GLenum depthStencilMode
    )
{
    gceTEXTURE_DS_MODE ret;
    gcmHEADER_ARG("depthStencilMode=0x%04x", depthStencilMode);
    switch (depthStencilMode)
    {
    case GL_DEPTH_COMPONENT:
        ret = gcvTEXTURE_DS_MODE_DEPTH;
        break;

    case GL_STENCIL_INDEX:
        ret = gcvTEXTURE_DS_MODE_STENCIL;
        break;

    default:
        GL_ASSERT(0);
        ret = gcvTEXTURE_DS_MODE_INVALID;
        break;
    }
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

__GL_INLINE gceTEXTURE_SRGBDECODE
gcChipUtilConvertSRGB(
    GLenum mode
    )
{
    gceTEXTURE_SRGBDECODE ret;

    gcmHEADER_ARG("mode=0x%04x", mode);
    switch (mode)
    {
        case GL_DECODE_EXT:
            ret = gcvTEXTURE_DECODE;
            break;
        case GL_SKIP_DECODE_EXT:
            ret = gcvTEXTURE_SKIP_DECODE;
            break;
        default:
            ret = gcvTEXTURE_SRGB_INVALID;
            break;
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

gceLAYOUT_QUALIFIER
gcChipUtilConvertLayoutQualifier(
    GLenum blendEquation,
    GLboolean *blendInShader
    )
{
    gceLAYOUT_QUALIFIER layoutBit = gcvLAYOUT_QUALIFIER_NONE;
    GLboolean shaderBlend = gcvFALSE;
    gctBOOL advanceBlendPart0InHW = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_ADVANCED_BLEND_MODE_PART0);

    switch (blendEquation)
    {
    case GL_MULTIPLY_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_MULTIPLY;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_SCREEN_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_SCREEN;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_OVERLAY_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_OVERLAY;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_DARKEN_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_DARKEN;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_LIGHTEN_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_LIGHTEN;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_COLORDODGE_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORDODGE;
        shaderBlend = gcvTRUE;
        break;
    case GL_COLORBURN_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORBURN;
        shaderBlend = gcvTRUE;
        break;
    case GL_HARDLIGHT_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HARDLIGHT;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_SOFTLIGHT_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_SOFTLIGHT;
        shaderBlend = gcvTRUE;
        break;
    case GL_DIFFERENCE_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_DIFFERENCE;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_EXCLUSION_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_EXCLUSION;
        shaderBlend = !advanceBlendPart0InHW;
        break;
    case GL_HSL_HUE_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_HUE;
        shaderBlend = gcvTRUE;
        break;
    case GL_HSL_SATURATION_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_SATURATION;
        shaderBlend = gcvTRUE;
        break;
    case GL_HSL_COLOR_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_COLOR;
        shaderBlend = gcvTRUE;
        break;
    case GL_HSL_LUMINOSITY_KHR:
        layoutBit = gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_LUMINOSITY;
        shaderBlend = gcvTRUE;
        break;
    default:
        break;
    }

    if (blendInShader)
    {
        *blendInShader = shaderBlend;
    }

    return layoutBit;
}


__GL_INLINE GLenum
gcChipUtilConvertImageFormat(
    gceIMAGE_FORMAT imageFormat
    )
{
    GLenum glFormat = GL_RGBA32F;

    switch (imageFormat)
    {
    case gcIMAGE_FORMAT_RGBA32F:
        glFormat = GL_RGBA32F;
        break;

    case gcIMAGE_FORMAT_RGBA16F:
        glFormat = GL_RGBA16F;
        break;
    case gcIMAGE_FORMAT_R32F:
        glFormat = GL_R32F;
        break;
    case gcIMAGE_FORMAT_RGBA8:
        glFormat = GL_RGBA8;
        break;
    case gcIMAGE_FORMAT_RGBA8_SNORM:
        glFormat = GL_RGBA8_SNORM;
        break;
    case gcIMAGE_FORMAT_RGBA32I:
        glFormat = GL_RGBA32I;
        break;
    case gcIMAGE_FORMAT_RGBA16I:
        glFormat = GL_RGBA16I;
        break;
    case gcIMAGE_FORMAT_RGBA8I:
        glFormat = GL_RGBA8I;
        break;
    case gcIMAGE_FORMAT_R32I:
        glFormat = GL_R32I;
        break;
    case gcIMAGE_FORMAT_RGBA32UI:
        glFormat = GL_RGBA32UI;
        break;
    case gcIMAGE_FORMAT_RGBA16UI:
        glFormat = GL_RGBA16UI;
        break;
    case gcIMAGE_FORMAT_RGBA8UI:
        glFormat = GL_RGBA8UI;
        break;
    case gcIMAGE_FORMAT_R32UI:
        glFormat = GL_R32UI;
        break;
    case gcIMAGE_FORMAT_DEFAULT:
    default:
        gcmASSERT(0);
        break;
    }

    return glFormat;
}

__GL_INLINE gceSTATUS
gcChipSetBlendFuncSeparateIndexed(
    __GLchipContext *chipCtx,
    GLuint halRTIndex,
    GLenum SrcRGB,
    GLenum DstRGB,
    GLenum SrcAlpha,
    GLenum DstAlpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    static GLenum s_blendFuncNames[] =
    {
        GL_ZERO,                        /* glvBLENDZERO */
        GL_ONE,                         /* glvBLENDONE */
        GL_SRC_COLOR,                   /* glvBLENDSRCCOLOR */
        GL_ONE_MINUS_SRC_COLOR,         /* glvBLENDSRCCOLORINV */
        GL_SRC_ALPHA,                   /* glvBLENDSRCALPHA */
        GL_ONE_MINUS_SRC_ALPHA,         /* glvBLENDSRCALPHAINV */
        GL_DST_COLOR,                   /* glvBLENDDSTCOLOR */
        GL_ONE_MINUS_DST_COLOR,         /* glvBLENDDSTCOLORINV */
        GL_DST_ALPHA,                   /* glvBLENDDSTALPHA */
        GL_ONE_MINUS_DST_ALPHA,         /* glvBLENDDSTALPHAINV */
        GL_SRC_ALPHA_SATURATE,          /* glvBLENDSRCALPHASATURATE */
        GL_CONSTANT_COLOR,              /* gcvBLENDCONSTCOLOR */
        GL_ONE_MINUS_CONSTANT_COLOR,    /* glvBLENDCONSTCOLORINV */
        GL_CONSTANT_ALPHA,              /* glvBLENDCONSTALPHA */
        GL_ONE_MINUS_CONSTANT_ALPHA     /* glvBLENDCONSTALPHAINV */
    };

    static gceBLEND_FUNCTION s_blendFuncValues[] =
    {
        gcvBLEND_ZERO,                  /* glvBLENDZERO */
        gcvBLEND_ONE,                   /* glvBLENDONE */
        gcvBLEND_SOURCE_COLOR,          /* glvBLENDSRCCOLOR */
        gcvBLEND_INV_SOURCE_COLOR,      /* glvBLENDSRCCOLORINV */
        gcvBLEND_SOURCE_ALPHA,          /* glvBLENDSRCALPHA */
        gcvBLEND_INV_SOURCE_ALPHA,      /* glvBLENDSRCALPHAINV */
        gcvBLEND_TARGET_COLOR,          /* glvBLENDDSTCOLOR */
        gcvBLEND_INV_TARGET_COLOR,      /* glvBLENDDSTCOLORINV */
        gcvBLEND_TARGET_ALPHA,          /* glvBLENDDSTALPHA */
        gcvBLEND_INV_TARGET_ALPHA,      /* glvBLENDDSTALPHAINV */
        gcvBLEND_SOURCE_ALPHA_SATURATE, /* glvBLENDSRCALPHASATURATE */
        gcvBLEND_CONST_COLOR,           /* gcvBLENDCONSTCOLOR */
        gcvBLEND_INV_CONST_COLOR,       /* glvBLENDCONSTCOLORINV */
        gcvBLEND_CONST_ALPHA,           /* glvBLENDCONSTALPHA */
        gcvBLEND_INV_CONST_ALPHA        /* glvBLENDCONSTALPHAINV */
    };

    gcmHEADER_ARG("chipCtx=0x%x SrcRGB=0x%04x DstRGB=0x%04x SrcAlpha=0x%04x DstAlpha=0x%04x",
                   chipCtx, SrcRGB, DstRGB, SrcAlpha, DstAlpha);
    do
    {
        GLuint blendSrcRGB, blendSrcAlpha;
        GLuint blendDstRGB, blendDstAlpha;
        gceSTATUS status;

        gcmERR_BREAK(gcChipUtilConvertGLEnum(
                s_blendFuncNames,
                gcmCOUNTOF(s_blendFuncNames),
                &SrcRGB, glvINT,
                &blendSrcRGB
                ));

        gcmERR_BREAK(!gcChipUtilConvertGLEnum(
                s_blendFuncNames,
                gcmCOUNTOF(s_blendFuncNames),
                &DstRGB, glvINT,
                &blendDstRGB
                ));

        gcmERR_BREAK(gcChipUtilConvertGLEnum(
                s_blendFuncNames,
                gcmCOUNTOF(s_blendFuncNames),
                &SrcAlpha, glvINT,
                &blendSrcAlpha ));

        gcmERR_BREAK(gcChipUtilConvertGLEnum(
                s_blendFuncNames,
                gcmCOUNTOF(s_blendFuncNames),
                &DstAlpha, glvINT,
                &blendDstAlpha));

        do
        {
            gceBLEND_FUNCTION srcFunctionRGB   = s_blendFuncValues[blendSrcRGB];
            gceBLEND_FUNCTION dstFunctionRGB   = s_blendFuncValues[blendDstRGB];
            gceBLEND_FUNCTION srcFunctionAlpha = s_blendFuncValues[blendSrcAlpha];
            gceBLEND_FUNCTION dstFunctionAlpha = s_blendFuncValues[blendDstAlpha];

            gcmERR_BREAK(gco3D_SetBlendFunctionIndexed(chipCtx->engine,
                                                       halRTIndex,
                                                       gcvBLEND_SOURCE,
                                                       srcFunctionRGB,
                                                       srcFunctionAlpha));

            gcmERR_BREAK(gco3D_SetBlendFunctionIndexed(chipCtx->engine,
                                                       halRTIndex,
                                                       gcvBLEND_TARGET,
                                                       dstFunctionRGB,
                                                       dstFunctionAlpha));
        } while (gcvFALSE);

    } while (gcvFALSE);

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetBlendEquationSeparateIndexed(
    __GLchipContext *chipCtx,
    GLuint halRTIndex,
    GLenum modeRGB,
    GLenum modeAlpha
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    static GLenum s_blendModeNames[] =

    {

        GL_FUNC_ADD,                    /* glvBLEND_ADD */
        GL_FUNC_SUBTRACT,               /* glvBLEND_SUBTRACT */
        GL_FUNC_REVERSE_SUBTRACT,       /* glvBLEND_REVERSE_SUBTRACT */
        GL_MIN,                         /* glvBLEND_MIN */
        GL_MAX,                         /* glvBLEND_MAX */
        GL_MULTIPLY_KHR,                /* gcvBLEND_MULTIPLY */
        GL_SCREEN_KHR,                  /* gcvBLEND_SCREEN */
        GL_OVERLAY_KHR,                 /* gcvBLEND_OVERLAY */
        GL_DARKEN_KHR,                  /* gcvBLEND_DARKEN */
        GL_LIGHTEN_KHR,                 /* gcvBLEND_LIGHTEN */
        GL_COLORDODGE_KHR,              /* gcvBLEND_COLORDODGE */
        GL_COLORBURN_KHR,               /* gcvBLEND_COLORBURN */
        GL_HARDLIGHT_KHR,               /* gcvBLEND_HARDLIGHT */
        GL_SOFTLIGHT_KHR,               /* gcvBLEND_SOFTLIGHT */
        GL_DIFFERENCE_KHR,              /* gcvBLEND_DIFFERENCE */
        GL_EXCLUSION_KHR,               /* gcvBLEND_EXCLUSION */
        GL_HSL_HUE_KHR,                 /* gcvBLEND_HSL_HUE */
        GL_HSL_SATURATION_KHR,          /* gcvBLEND_HSL_SATURATION */
        GL_HSL_COLOR_KHR,               /* gcvBLEND_HSL_COLOR */
        GL_HSL_LUMINOSITY_KHR,          /* gcvBLEND_HSL_LUMINOSITY */
    };


    static gceBLEND_MODE s_blendModeValues[] =

    {

        gcvBLEND_ADD,                   /* glvBLEND_ADD */
        gcvBLEND_SUBTRACT,              /* glvBLEND_SUBTRACT */
        gcvBLEND_REVERSE_SUBTRACT,      /* glvBLEND_REVERSE_SUBTRACT */
        gcvBLEND_MIN,                   /* glvBLEND_MIN */
        gcvBLEND_MAX,                   /* glvBLEND_MAX */
        gcvBLEND_MULTIPLY,              /* gcvBLEND_MULTIPLY */
        gcvBLEND_SCREEN,                /* gcvBLEND_SCREEN */
        gcvBLEND_OVERLAY,               /* gcvBLEND_OVERLAY */
        gcvBLEND_DARKEN,                /* gcvBLEND_DARKEN */
        gcvBLEND_LIGHTEN,               /* gcvBLEND_LIGHTEN */
        gcvBLEND_COLORDODGE,            /* gcvBLEND_COLORDODGE */
        gcvBLEND_COLORBURN,             /* gcvBLEND_COLORBURN */
        gcvBLEND_HARDLIGHT,             /* gcvBLEND_HARDLIGHT */
        gcvBLEND_SOFTLIGHT,             /* gcvBLEND_SOFTLIGHT */
        gcvBLEND_DIFFERENCE,            /* gcvBLEND_DIFFERENCE */
        gcvBLEND_EXCLUSION,             /* gcvBLEND_EXCLUSION */
        gcvBLEND_HSL_HUE,               /* gcvBLEND_HSL_HUE */
        gcvBLEND_HSL_SATURATION,        /* gcvBLEND_HSL_SATURATION */
        gcvBLEND_HSL_COLOR,             /* gcvBLEND_HSL_COLOR */
        gcvBLEND_HSL_LUMINOSITY,        /* gcvBLEND_HSL_LUMINOSITY */
    };

    gcmHEADER_ARG("chipCtx=0x%x modeRGB=0x%04x modeAlpha=0x%04x",
                   chipCtx, modeRGB, modeAlpha);
    do
    {
        GLuint blendModeRGB;
        GLuint blendModeAlpha;

        gcmERR_BREAK(gcChipUtilConvertGLEnum(s_blendModeNames,
                              gcmCOUNTOF(s_blendModeNames),
                              &modeRGB, glvINT,
                              &blendModeRGB));

        gcmERR_BREAK(gcChipUtilConvertGLEnum(s_blendModeNames,
                              gcmCOUNTOF(s_blendModeNames),
                              &modeAlpha, glvINT,
                              &blendModeAlpha ));

        do {
            gceBLEND_MODE gceBlendModeRGB = s_blendModeValues[blendModeRGB];
            gceBLEND_MODE gceBlendModeAlpha = s_blendModeValues[blendModeAlpha];

            gcmERR_BREAK(gco3D_SetBlendModeIndexed(chipCtx->engine,
                                                   halRTIndex,
                                                   gceBlendModeRGB,
                                                   gceBlendModeAlpha));
        } while (gcvFALSE);

    } while (gcvFALSE);

    gcmFOOTER();

    return status;
}


__GL_INLINE gceSTATUS
gcChipValidateProgramTexBufferUniform(
    __GLcontext *gc,
    __GLtextureObject *texObj,
    __GLchipSLUniform *uniform,
    __GLimageUnitState *imageUnit
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo*)(texObj->bufObj->privateData);
    gctUINT32 physical;

    GLuint imageInfo = 0;
    gctUINT *data;
    __GLmipMapLevel *mipmap = &texObj->faceMipmap[0][0];
    GLuint width = 0;
    GLuint height = 0;
    GLuint size = 0;
    GLuint texelSize = 0;
    GLint format = 0;

    gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));

    size = texObj->bufSize ? texObj->bufSize : (GLint)texObj->bufObj->size;

    texelSize = size / texObj->bppPerTexel;

    /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, LINEAR); texBuffer always be linear.*/
    imageInfo = 0;

    /*
    imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TYPE, 2D);
    */
    imageInfo |= (1 << 12);

    /* When the format associated with an image unit does not exactly match the internal format of the texture bound to the image unit,
       image loads, stores and atomic operations re-interpret the memory holding the components of an accessed texel
       according to the format of the image unit.
    */
    if (imageUnit)
    {
        format = imageUnit->format;
    }
    else
    {
        format = mipmap->requestedFormat;
    }

    switch (format)
    {
    case GL_R8:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 0)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0xF << 6 | 0 | 1 << 14 | 0 << 3;
        break;
    case GL_R16F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x1 << 6 | 1 | 1 << 14 | 0 << 3;
        break;
    case GL_R32F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x0 << 6 | 2 | 1 << 14 | 0 << 3;
        break;
    case GL_R8I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 0)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x4 << 6 | 0 | 1 << 14 | 0 << 3;
        break;
    case GL_R16I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x3 << 6 | 1 | 1 << 14 | 0 << 3;
        break;
    case GL_R32I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x2 << 6 | 2 | 1 << 14 | 0 << 3;
        break;
    case GL_R8UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 0)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x7 << 6 | 0 | 1 << 14 | 0 << 3;
        break;
    case GL_R16UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x6 << 6 | 1 | 1 << 14 | 0 << 3;
        break;
    case GL_R32UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x5 << 6 | 2 | 1 << 14 | 0 << 3;
        break;
    case GL_RG8:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0xF << 6 | 1 | 2 << 14 | 0 << 3;
        break;
    case GL_RG16F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x1 << 6 | 2 | 2 << 14 | 0 << 3;
        break;
    case GL_RG32F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x0 << 6 | 3 | 2 << 14 | 0 << 3;
        break;
    case GL_RG8I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x4 << 6 | 1 | 2 << 14 | 0 << 3;
        break;
    case GL_RG16I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x3 << 6 | 2 | 2 << 14 | 0 << 3;
        break;
    case GL_RG32I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x2 << 6 | 3 | 2 << 14 | 0 << 3;
        break;
    case GL_RG8UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 1)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x7 << 6 | 1 | 2 << 14 | 0 << 3;
        break;
    case GL_RG16UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x6 << 6 | 2 | 2 << 14 | 0 << 3;
        break;
    case GL_RG32UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x5 << 6 | 3 | 2 << 14 | 0 << 3;
        break;
    case GL_RGB32F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 3) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, THREE);
        */
        imageInfo |= 0x0 << 6 | 2 | 3 << 14 | 1 << 3;
        break;
    case GL_RGB32I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 3) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, THREE);
        */
        imageInfo |= 0x2 << 6 | 2 | 3 << 14 | 1 << 3;
        break;
    case GL_RGB32UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 3) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, THREE);
        */
        imageInfo |= 0x5 << 6 | 2 | 3 << 14 | 1 << 3;
        break;
    case GL_RGBA8:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)           |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0xF << 6 | 2 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA16F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP16) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x1 << 6 | 3 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA32F:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 4)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x0 << 6 | 4 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA8I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x4 << 6 | 2 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA16I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S16)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x3 << 6 | 3 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA32I:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 4)         |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x2 << 6 | 4 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA8UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U8) |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)       |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x7 << 6 | 2 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA16UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U16)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
        gcmSETFIELDVALUE(), GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x6 << 6 | 3 | 0 << 14 | 0 << 3;
        break;
    case GL_RGBA32UI:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32)  |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 4)         |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0x5 << 6 | 4 | 0 << 14 | 0 << 3;
        break;

    case GL_RGBA8_SNORM:
        /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, SNORM8)|
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0) |
        gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, MULTIPLY, ONE);
        */
        imageInfo |= 0xC << 6 | 2 | 0 << 14 | 0 << 3;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
    }

    switch (format)
    {
    case GL_RGBA32F:
    case GL_RGBA32UI:
    case GL_RGBA32I:
    case GL_RGBA16F:
    case GL_RGBA16UI:
    case GL_RGBA16I:
    case GL_RGBA8UI:
    case GL_RGBA8I:
    case GL_RGBA8:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
        */
        imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);
        imageInfo |= 1 << 4;
        break;

    case GL_RG8:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8I:
    case GL_RG16I:
    case GL_RG32I:
    case GL_RG8UI:
    case GL_RG16UI:
    case GL_RG32UI:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, ZERO)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
        */
        imageInfo |= (0 << 16) | (1 << 20) | (4 << 24) | (5 << 28);
        imageInfo |= 1 << 4;
        break;

    case GL_RGB32F:
    case GL_RGB32UI:
    case GL_RGB32I:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
        */
        imageInfo |=  (0 << 16) | (1 << 20) | (2 << 24) | (5 << 28);
        imageInfo |= 1 << 4;
        break;

    case GL_R8:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16F:
    case GL_R16I:
    case GL_R16UI:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, ZERO)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, ZERO)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
        */
        imageInfo |=  (0 << 16) | (4 << 20) | (4 << 24) | (5 << 28);
        imageInfo |= 1 << 4;
        break;

    case GL_R32F:
    case GL_R32UI:
    case GL_R32I:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, ZERO)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, ZERO)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
        */
        imageInfo |=  (0 << 16) | (4 << 20) | (4 << 24) | (5 << 28);
        imageInfo |= 2 << 4;
        break;

    case GL_RGBA8_SNORM:
        /*
        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
        gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
        */
        imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);
        imageInfo |= 1 << 4;
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
    }

    /* compute texture with and height, our HW can only support 16K image size.*/
    if (((texelSize / 16384) < 1))
    {
        width = texelSize;
        height = 1;
    }
    else
    {
        width = 16384;
        height = (GLuint)gcoMATH_Ceiling(((GLfloat)texelSize / 16384));
    }

    data = (gctUINT*)uniform->data;
    /* baseAddress. */
    data[0] = (gctUINT)(physical + texObj->bufOffset);
    /* stride. */
    data[1] = (gctUINT)(16384 * texObj->bppPerTexel);
    data[2] = width | (height << 16);
    data[3] = imageInfo;

    /* Need to set user-defined uniforms dirty to toggle later flush */
    if (uniform->usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
    {
        chipCtx->chipDirty.uDefer.sDefer.activeUniform = 1;
    }
    uniform->dirty = GL_TRUE;

OnError:
    return status;
}


__GL_INLINE gceSTATUS
gcChipValidateAlphaBlend(
    __GLcontext *gc,
    GLbitfield localMask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x localMask=%u", gc, localMask);

    if (localMask & (__GL_BLEND_ENDISABLE_BIT | __GL_BLENDEQUATION_BIT | __GL_BLENDFUNC_BIT))
    {
        __GLchipSLProgramInstance *fsInstance = chipCtx->activePrograms[__GLSL_STAGE_FS]->curPgInstance;
        GLboolean advBlendInShader;
        gceLAYOUT_QUALIFIER layoutBit = gcChipUtilConvertLayoutQualifier(gc->state.raster.blendEquationRGB[0], &advBlendInShader);

        if (localMask & (__GL_BLENDEQUATION_BIT | __GL_BLEND_ENDISABLE_BIT))
        {
            chipCtx->advBlendMode = layoutBit;
            chipCtx->advBlendInShader = (advBlendInShader && gc->state.enables.colorBuffer.blend[0]);

            if (fsInstance->advBlendState)
            {
                fsInstance->advBlendState->dirty = gcvTRUE;
            }

            if (chipCtx->advBlendInShader)
            {
                __glChipBlendBarrier(gc);
            }
        }
        chipCtx->chipDirty.uDefer.sDefer.blend = 1;

    }

    if (localMask & __GL_BLENDCOLOR_BIT)
    {
        gcmONERROR(gco3D_SetBlendColorF(chipCtx->engine,
                                        gc->state.raster.blendColor.r,
                                        gc->state.raster.blendColor.g,
                                        gc->state.raster.blendColor.b,
                                        gc->state.raster.blendColor.a));
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipSetTextureParameters(
    __GLcontext *gc,
    __GLtextureObject *tex,
    GLuint unit,
    GLuint localMask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLsamplerObject *samplerObj = gc->texture.units[unit].boundSampler;
    __GLsamplerParamState *samplerStates;
    __GLchipTextureInfo *texInfo = gcvNULL;
    gcsTEXTURE * halTexture;
    GLuint value;
    gceSTATUS status = gcvSTATUS_OK;

    static gceTEXTURE_FILTER halMagFilter[] =
    {
        0,
        GL_NEAREST,         /* gcvTEXTURE_POINT  */
        GL_LINEAR,          /* gcvTEXTURE_LINEAR */
    };

    gcmHEADER_ARG("gc=0x%x tex=0x%x unit=%u localMask=%u", gc, tex, unit, localMask);

    if (tex == gcvNULL)
    {
        gcmFOOTER();
        return status;
    }

    texInfo = (__GLchipTextureInfo *)tex->privateData;
    if (!texInfo)
    {
        gcmFOOTER();
        return status;
    }

    /* (TBR) create gcoTEXTURE and its mipmaps as texture descriptor need them */
    if ((texInfo->eglImage.source) && chipCtx->chipFeature.hasTxDescriptor)
    {
        gcmONERROR(gcChipTexSyncEGLImage(gc, tex, gcvFALSE));
    }

    samplerStates = samplerObj ? &samplerObj->params : (__GLsamplerParamState*)&tex->params;
    halTexture = &chipCtx->texture.halTexture[unit];

    if (localMask & __GL_TEXPARAM_WRAP_S_BIT)
    {
        halTexture->s = gcChipUtilConvertWrapMode(samplerStates->sWrapMode);
    }

    if (localMask & __GL_TEXPARAM_WRAP_T_BIT)
    {
        halTexture->t = gcChipUtilConvertWrapMode(samplerStates->tWrapMode);
    }

    if (localMask & __GL_TEXPARAM_WRAP_R_BIT)
    {
        halTexture->r = gcChipUtilConvertWrapMode(samplerStates->rWrapMode);
    }

    if (localMask & __GL_TEXPARAM_SWIZZLE_R_BIT)
    {
        halTexture->swizzle[gcvTEXTURE_COMPONENT_R] = gcChipUtilConvertTexSwizzle(tex->params.swizzle[__GL_TEX_COMPONENT_R]);
    }

    if (localMask & __GL_TEXPARAM_SWIZZLE_G_BIT)
    {
        halTexture->swizzle[gcvTEXTURE_COMPONENT_G] = gcChipUtilConvertTexSwizzle(tex->params.swizzle[__GL_TEX_COMPONENT_G]);
    }

    if (localMask & __GL_TEXPARAM_SWIZZLE_B_BIT)
    {
        halTexture->swizzle[gcvTEXTURE_COMPONENT_B] = gcChipUtilConvertTexSwizzle(tex->params.swizzle[__GL_TEX_COMPONENT_B]);
    }

    if (localMask & __GL_TEXPARAM_SWIZZLE_A_BIT)
    {
        halTexture->swizzle[gcvTEXTURE_COMPONENT_A] = gcChipUtilConvertTexSwizzle(tex->params.swizzle[__GL_TEX_COMPONENT_A]);
    }

    if (localMask & __GL_TEXPARAM_D_ST_TEXMODE_BIT)
    {
        halTexture->dsMode = gcChipUtilConvertDSMode(tex->params.depthStTexMode);

    }

    if(localMask & __GL_TEXPARAM_SRGB_BIT)
    {
        halTexture->sRGB = gcChipUtilConvertSRGB(samplerStates->sRGB);
    }

    if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEX_BASELOD))
    {
        if (localMask & (__GL_TEXPARAM_MIN_LOD_BIT | __GL_TEXPARAM_MAX_LOD_BIT))
        {
            halTexture->lodMax = samplerStates->maxLod;
            /* Let lodMin <= lodMax to handle spec undefined behavior */
            halTexture->lodMin = __GL_MIN(samplerStates->minLod, samplerStates->maxLod);
        }

        if (localMask & __GL_TEXPARAM_BASE_LEVEL_BIT)
        {
            halTexture->baseLevel = tex->params.baseLevel;
        }

        if (localMask & __GL_TEXPARAM_MAX_LEVEL_BIT)
        {
            halTexture->maxLevel = gc->texture.units[unit].maxLevelUsed;
        }
    }
    else
    {
        /* Older hardware do not have support for base and max level. This implementation provides a
         * software approximation and may be revisited later. At least for now it complies with
         * ES30 sgis_texture_lod and full ES2.0 conformance tests
         */
        if (localMask &
            (__GL_TEXPARAM_MAX_LOD_BIT | __GL_TEXPARAM_MIN_LOD_BIT | __GL_TEXPARAM_BASE_LEVEL_BIT | __GL_TEXPARAM_MAX_LEVEL_BIT))
        {
            gctFLOAT base = (gctFLOAT) tex->params.baseLevel;
            base = __GL_MIN(samplerStates->maxLod, base);

            halTexture->lodMax = (gctFLOAT)gc->texture.units[unit].maxLevelUsed;

            if ((samplerStates->minLod != -1000) || (base == 0.0f))
            {
                halTexture->lodMin = samplerStates->minLod + base;
            }
            else
            {
                halTexture->lodMin = base;
            }
            /* Let lodMin <= lodMax to handle spec undefined behavior */
            halTexture->lodMin = __GL_MIN(halTexture->lodMin, halTexture->lodMax);

            halTexture->baseLevel = tex->params.baseLevel;
            halTexture->maxLevel = gc->texture.units[unit].maxLevelUsed;

            halTexture->lodBias = base;

            /* Fix for GL3Tests/texture_lod_bias test*/
            if ((samplerStates->minLod == samplerStates->maxLod) && (base == 0))
            {
                if ((gctFLOAT) gc->texture.units[unit].maxLevelUsed > samplerStates->maxLod)
                {
                    halTexture->lodBias = (samplerStates->maxLod - (gctFLOAT) gc->texture.units[unit].maxLevelUsed);
                }
            }
        }
    }

    if (localMask & (__GL_TEXPARAM_MIN_FILTER_BIT | __GL_TEXPARAM_MIP_HINT_BIT))
    {
        gceTEXTURE_FILTER halMin = gcvTEXTURE_NONE;
        gceTEXTURE_FILTER halMip = gcvTEXTURE_NONE;

        gcChipUtilConvertMinFilter(samplerStates->minFilter, &halMin, &halMip);

        switch (tex->params.mipHint)
        {
        case __GL_TEX_MIP_HINT_FORCE_ON:
            /* Force no-mipmap to point mipmap */
            if (halMip == gcvTEXTURE_NONE)
            {
                halMip = gcvTEXTURE_POINT;
            }
            break;
        case __GL_TEX_MIP_HINT_FORCE_OFF:
            halMip = gcvTEXTURE_NONE;
            break;
        default:
            break;
        }

        halTexture->minFilter = halMin;
        halTexture->mipFilter = halMip;
    }

    if (localMask & __GL_TEXPARAM_MAG_FILTER_BIT)
    {
        gcmVERIFY_OK(gcChipUtilConvertGLEnum((GLenum *)&halMagFilter[0],
                             gcmCOUNTOF(halMagFilter),
                             &samplerStates->magFilter, glvINT,
                             &value));
        halTexture->magFilter = value;
    }

    /* Hardware does not depth texture */
    if (localMask & __GL_TEXPARAM_COMPARE_MODE_BIT)
    {
        halTexture->compareMode = gcChipUtilConvertCompareMode(samplerStates->compareMode);
    }

    if (localMask & __GL_TEXPARAM_COMPARE_FUNC_BIT)
    {
        halTexture->compareFunc = gcChipUtilConvertCompareFunc(samplerStates->compareFunc);
    }

    if (localMask & __GL_TEXPARAM_MAX_ANISTROPY_BIT)
    {
        halTexture->anisoFilter = __GL_MIN(gc->constants.maxAnistropic, gcoMATH_Float2UInt(samplerStates->maxAnistropy));
    }

    if (localMask & __GL_TEXPARAM_BORDER_COLOR_BIT)
    {
        __GL_MEMCOPY(halTexture->borderColor, samplerStates->borderColor.fv, 4 * gcmSIZEOF(GLfloat));
    }

    if (chipCtx->chipFeature.hasTxDescriptor)
    {
        /* Update texture descriptors if object-only state dirties */
        if (tex->uObjStateDirty.objStateDirty)
        {
            gcmONERROR(gcoTEXTURE_SetDescDirty(texInfo->object));
        }
    }

OnError:

    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipSetRasterDiscard(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status;

    gcmONERROR(gco3D_SetRasterDiscard(chipCtx->engine, (gctBOOL)gc->state.enables.rasterizerDiscard));

OnError:
    return status;
}


__GL_INLINE gceSTATUS
gcChipValidateAttribGroup1(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1])
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_1];

        if (localMask & __GL_COLORMASK_BIT)
        {
            chipCtx->chipDirty.uDefer.sDefer.colorMask = 1;
        }

        /* The HAL face was affected by how the frontFace was defined */
        if (localMask & (__GL_STENCIL_ATTR_BITS | __GL_FRONTFACE_BIT))
        {
            gcmONERROR(gcChipSetStencilStates(gc, localMask));
            /* If stencil is enabled/disable, depth enable/disable state should be updated, too. */
            if (localMask & __GL_STENCILTEST_ENDISABLE_BIT)
            {
                localMask |= __GL_DEPTHTEST_ENDISABLE_BIT;
            }
        }

        if (localMask & __GL_DEPTHRANGE_BIT)
        {
            chipCtx->chipDirty.uDefer.sDefer.depthRange = 1;
        }

        if (localMask & __GL_DEPTHMASK_BIT)
        {
            chipCtx->chipDirty.uDefer.sDefer.depthMask = 1;
        }

        if (localMask & __GL_DEPTHBUF_ATTR_BITS)
        {
            gcmONERROR(gcChipSetDepthStates(gc, localMask));
        }

        if (localMask & __GL_ALPHABLEND_ATTR_BITS)
        {
            gcmONERROR(gcChipValidateAlphaBlend(gc, localMask));
        }

        if (localMask & (__GL_FRONTFACE_BIT | __GL_CULLFACE_BIT | __GL_CULLFACE_ENDISABLE_BIT))
        {
            chipCtx->chipDirty.uDefer.sDefer.culling = 1;
        }

        if (localMask & (__GL_POLYGONOFFSET_FILL_ENDISABLE_BIT | __GL_POLYGONOFFSET_BIT))
        {
            gcmONERROR(gcChipSetPolygonOffset(gc));
        }

        if (localMask & __GL_RASTERIZER_DISCARD_ENDISABLE_BIT)
        {
            gcmONERROR(gcChipSetRasterDiscard(gc, chipCtx));

            if (!chipCtx->chipFeature.hasHwTFB)
            {
                chipCtx->chipDirty.uDefer.sDefer.viewportScissor = 1;
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipValidateAttribGroup2(
    __GLcontext * gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (gc->globalDirtyState[__GL_DIRTY_ATTRS_2])
    {
        GLbitfield localMask = gc->globalDirtyState[__GL_DIRTY_ATTRS_2];

        if (localMask & (__GL_SCISSOR_BIT | __GL_SCISSORTEST_ENDISABLE_BIT | __GL_VIEWPORT_BIT))
        {
            /* set Chip layer dirty bit and combined with FBO changes at chip draw time */
            chipCtx->chipDirty.uDefer.sDefer.viewportScissor = 1;
            if (localMask & __GL_VIEWPORT_BIT)
            {
                chipCtx->viewportWidth  = gc->state.viewport.width;
                chipCtx->viewportHeight = gc->state.viewport.height;
#if __GL_CHIP_PATCH_ENABLED
                gcChipPatchValidateViewport(gc);
#endif
            }
        }

        if (localMask & __GL_DITHER_ENDISABLE_BIT)
        {
            gcmONERROR(gco3D_EnableDither(chipCtx->engine, gc->state.enables.colorBuffer.dither));
        }

        if (localMask & __GL_LINEWIDTH_BIT)
        {
            gcmONERROR(gco3D_SetAALineWidth(chipCtx->engine, (GLfloat)gc->state.line.aliasedWidth));
        }

        if(localMask & __GL_PRIMITIVE_RESTART_BIT)
        {
            gcmONERROR(gco3D_PrimitiveRestart(chipCtx->engine, gc->state.enables.primitiveRestart));
        }

        if (localMask & __GL_MULTISAMPLE_ATTR_BITS)
        {
            if (localMask & __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT)
            {
                gcmONERROR(gco3D_EnableAlphaToCoverage(chipCtx->engine, gc->state.enables.multisample.alphaToCoverage));
            }

            if (localMask & __GL_SAMPLE_COVERAGE_ENDISABLE_BIT)
            {
                gcmONERROR(gco3D_EnableSampleCoverage(chipCtx->engine, gc->state.enables.multisample.coverage));
            }

            if (localMask & __GL_SAMPLECOVERAGE_BIT)
            {
                gcmONERROR(gco3D_SetSampleCoverageValue(chipCtx->engine,
                                                        gc->state.multisample.coverageValue,
                                                        gc->state.multisample.coverageInvert));
            }

            if (localMask & __GL_SAMPLE_MASK_ENDISABLE_BIT)
            {
                gcmONERROR(gco3D_EnableSampleMask(chipCtx->engine, gc->state.enables.multisample.sampleMask));
            }

            if (localMask & __GL_SAMPLE_MASK_BIT)
            {
                gcmONERROR(gco3D_SetSampleMask(chipCtx->engine, gc->state.multisample.sampleMaskValue));
            }

            if(localMask & __GL_SAMPLE_SHADING_ENDISABLE_BIT)
            {
                gcmONERROR(gco3D_EnableSampleShading(chipCtx->engine, gc->state.enables.multisample.sampleShading));
            }

            if (localMask & __GL_SAMPLE_MIN_SHADING_VALUE_BIT)
            {
                gcmONERROR(gco3D_SetMinSampleShadingValue(chipCtx->engine, gc->state.multisample.minSampleShadingValue));
            }
        }

    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateShader(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLbitfield localMask = gc->globalDirtyState[__GL_PROGRAM_ATTRS];

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (localMask & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        if (localMask & __GL_DIRTY_GLSL_VS_SWITCH)
        {
            chipCtx->chipDirty.uDefer.sDefer.vsReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_FS_SWITCH)
        {
            __GLchipSLProgram *fsProgram = chipCtx->activePrograms[__GLSL_STAGE_FS];

            if (fsProgram)
            {
                gctBOOL enableEarlyTest = gcvFALSE;
                gcsHINT_PTR psHints = fsProgram->curPgInstance->programState.hints;
                gctBOOL psReadZ   = psHints->useFragCoord[2];
                gctBOOL psReadW   = psHints->useFragCoord[3];
                gctBOOL hasMemoryAccess = (psHints->memoryAccessFlags[gcvPROGRAM_STAGE_FRAGMENT] & gceMA_FLAG_READ)
                                          ||
                                          (psHints->memoryAccessFlags[gcvPROGRAM_STAGE_FRAGMENT] & gceMA_FLAG_WRITE);

                gcmASSERT(fsProgram->curPgInstance->binaries[__GLSL_STAGE_FS]);

                gcmONERROR(gcSHADER_GetEarlyFragTest(fsProgram->curPgInstance->binaries[__GLSL_STAGE_FS], &enableEarlyTest));
                gcmONERROR(gco3D_SetAllEarlyDepthModes(chipCtx->engine, enableEarlyTest ?
                                                                        gcvFALSE : psHints->psHasFragDepthOut || hasMemoryAccess));

                gcmONERROR(gco3D_SetSampleShading(chipCtx->engine, psHints->usedSampleIdOrSamplePosition,
                                                    psHints->psUsedSampleInput,
                                                    ((psHints->usedSampleIdOrSamplePosition || psHints->psUsedSampleInput) ? (gctFLOAT)chipCtx->drawRTSamples : 0)));

                gcmONERROR(gco3D_EnableSampleMaskOut(chipCtx->engine, psHints->sampleMaskOutWritten, psHints->sampleMaskLoc));

                if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_MSAA_SHADING))
                {
                    gcmONERROR(gco3D_SetEarlyDepthFromAPP(chipCtx->engine, enableEarlyTest));
                }

                if(gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_RA_DEPTH_WRITE))
                {
                    gcmONERROR(gco3D_SetRADepthWrite(chipCtx->engine, enableEarlyTest ?
                                                        gcvFALSE : psHints->psHasFragDepthOut || psHints->hasKill || hasMemoryAccess,
                                                        psReadZ, (psReadW || (psHints->rtArrayComponent != -1))));
                }

                gcmONERROR(gco3D_SetShading(chipCtx->engine, psHints->shaderMode));

                gcmONERROR(gco3D_SetShaderLayered(chipCtx->engine, (psHints->rtArrayComponent != -1)));
            }

            chipCtx->chipDirty.uDefer.sDefer.fsReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_CS_SWITCH)
        {
            chipCtx->chipDirty.uDefer.sDefer.csReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_TCS_SWITCH)
        {
            chipCtx->chipDirty.uDefer.sDefer.tcsReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_TES_SWITCH)
        {
            chipCtx->chipDirty.uDefer.sDefer.tesReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_GS_SWITCH)
        {
            chipCtx->chipDirty.uDefer.sDefer.gsReload = 1;
        }

        if (localMask & __GL_DIRTY_GLSL_PATCH_VERTICES)
        {
            gcmONERROR(gco3D_SetPatchVertices(chipCtx->engine, gc->shaderProgram.patchVertices));
        }

        /* Mark all uniforms dirty if program switched */
        gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipMarkUniformDirtyCB));
    }
#if gcdENABLE_APPCTXT_BLITDRAW
    else
    {
        if (gco3D_IsProgramSwitched(chipCtx->engine) == gcvTRUE)
        {
            GLuint index;
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_VS_SWITCH);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_FS_SWITCH);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_CS_SWITCH);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TCS_SWITCH);
            __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_TES_SWITCH);

            /* Mark all array binding points dirty when program switched */
            for (index = 0; index < __GL_MAX_BUFFER_INDEX; index++)
            {
                __glBitmaskSetAll(&gc->bufferObject.bindingDirties[index], GL_TRUE);
            }

            gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipMarkUniformDirtyCB));
            chipCtx->chipDirty.uDefer.sDefer.activeUniform = 1;
        }
    }
#endif
    if (localMask & __GL_DIRTY_GLSL_UNIFORM)
    {
        chipCtx->chipDirty.uDefer.sDefer.activeUniform = 1;
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipValidateProgramSamplersCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    __GLbitmask samplerMapDirty = gc->shaderProgram.samplerMapDirty;
    __GLbitmask samplerStateDirty = gc->shaderProgram.samplerStateDirty;
    __GLbitmask samplerDirtyMask;
    GLint sampler = -1;
    GLuint index;
    __GLtextureObject *texObj = gcvNULL;
    __GLchipSLUniform *uniform;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);

    __glBitmaskInitOR(&samplerDirtyMask, &samplerMapDirty, &samplerStateDirty);

    while (!__glBitmaskIsAllZero(&samplerDirtyMask))
    {
        GLuint unit;

        if (!__glBitmaskTestAndClear(&samplerDirtyMask, ++sampler))
        {
            continue;
        }

        if ((program->samplerMap[sampler].stage == __GLSL_STAGE_LAST) &&
            (pgInstance->extraSamplerMap[sampler].stage == __GLSL_STAGE_LAST))
        {
            gcmONERROR(gcoTEXTURE_Disable(chipCtx->hal, sampler, gcvFALSE));
            continue;
        }

        if (program->samplerMap[sampler].auxiliary ||
            pgInstance->extraSamplerMap[sampler].auxiliary)
        {
            continue;
        }

        if (program->samplerMap[sampler].uniform != gcvNULL &&
            !isUniformUsedInShader(program->samplerMap[sampler].uniform) &&
            !isUniformUsedInLTC(program->samplerMap[sampler].uniform) &&
            isUniformSamplerCalculateTexSize(program->samplerMap[sampler].uniform))
        {
            continue;
        }

        unit = program->samplerMap[sampler].unit;
        texObj = gc->texture.units[unit].currentTexture;
        uniform = program->samplerMap[sampler].slUniform;

        if (texObj && texObj->bufObj)
        {
            gcChipValidateProgramTexBufferUniform(gc, texObj, uniform, gcvNULL);
            continue;
        }

        if (program->samplerMap[sampler].uniform != gcvNULL &&
            ((isUniformUsedAsTexGatherSampler(program->samplerMap[sampler].uniform) &&
             !gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_GATHER)) ||
             (isUniformUsedAsTexGatherOffsetsSampler(program->samplerMap[sampler].uniform) &&
             !gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_TEXTURE_GATHER_OFFSETS))
            )
           )
        {
            chipCtx->texture.halTexture[unit].mipFilter = gcvTEXTURE_NONE;
            chipCtx->texture.halTexture[unit].minFilter =
                chipCtx->texture.halTexture[unit].magFilter = gcvTEXTURE_POINT;
        }

        if (pgInstance->extraSamplerMap[sampler].subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER
         || pgInstance->extraSamplerMap[sampler].subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_SAMPLER )
        {
            gcsTEXTURE texParams;
            if (gcvNULL == chipCtx->rtTexture)
            {
                gceSURF_FORMAT format;
                gcsSURF_VIEW texView = {gcvNULL, 0, 1};
                gcsSURF_VIEW *rtView0 = &chipCtx->drawRtViews[0];
                gcsSURF_RESOLVE_ARGS rlvArgs = {0};

                gcmASSERT(rtView0->surf);
                gcmONERROR(gcoSURF_GetFormat(rtView0->surf, gcvNULL, &format));
                gcmONERROR(gcoTEXTURE_ConstructSized(chipCtx->hal,
                                                     format,
                                                     (gctUINT)chipCtx->drawRTWidth,
                                                     (gctUINT)chipCtx->drawRTHeight,
                                                     1, 1, 1, gcvPOOL_DEFAULT, &chipCtx->rtTexture));

                gcmONERROR(gcoTEXTURE_GetMipMap(chipCtx->rtTexture, 0, &texView.surf));

                /* Flush all cache in pipe */
                gcmONERROR(gcoSURF_Flush(rtView0->surf));

                /* Sync texture surface from current RT */
                rlvArgs.version = gcvHAL_ARG_VERSION_V2;
                rlvArgs.uArgs.v2.yInverted  = gcoSURF_QueryFlags(rtView0->surf, gcvSURF_FLAG_CONTENT_YINVERTED);
                rlvArgs.uArgs.v2.rectSize.x = (gctINT)chipCtx->drawRTWidth;
                rlvArgs.uArgs.v2.rectSize.y = (gctINT)chipCtx->drawRTHeight;
                rlvArgs.uArgs.v2.numSlices  = 1;
                gcmONERROR(gcoSURF_ResolveRect(rtView0, &texView, &rlvArgs));
                gcmONERROR(gcoTEXTURE_Flush(chipCtx->rtTexture));
                gcmONERROR(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));
            }
            gcmASSERT(chipCtx->rtTexture);
            gcmONERROR(gcoTEXTURE_InitParams(chipCtx->hal, &texParams));
            texParams.s = texParams.t = texParams.r = gcvTEXTURE_CLAMP;
            texParams.minFilter = texParams.magFilter = gcvTEXTURE_POINT;
            if (chipCtx->chipFeature.hasTxDescriptor)
            {
                gcmONERROR(gcoTEXTURE_BindTextureDesc(chipCtx->rtTexture,
                                sampler, &texParams, 0));
            }
            else
            {
                gcmONERROR(gcoTEXTURE_BindTextureEx(
                                    chipCtx->rtTexture,
                                    0,
                                    sampler,
                                    &texParams,
                                    0));
            }

        }
        else if (__glBitmaskTest(&gc->texture.currentEnableMask, unit))
        {
            __GLtextureObject *texObj = gc->texture.units[unit].currentTexture;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;

#if __GL_CHIP_PATCH_ENABLED
            gcmONERROR(gcChipPatchTexture(gc, texObj, unit));
#endif

            if (texInfo->direct.source)
            {
                gcmONERROR(gcChipTexSyncDirectVIV(gc, texObj));
            }
            else if (texInfo->eglImage.source)
            {
                gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
                gcChipTexCheckDirtyStateKeep(gc, texObj, sampler);
            }
            else
            {
                /* If the tex was drawn before and will be sampled in this draw, sync it back from the rtSurface */
                gcmONERROR(gcChipTexSyncFromShadow(gc, unit, texObj));
            }

            if (pgInstance->recompilePatchInfo.recompilePatchDirectivePtr &&
                pgInstance->pgStateKeyMask.s.hasTexPatchFmt)
            {
                gctUINT samplers[gcdMAX_SURF_LAYERS], layers = 1, samplerBaseOffset = 0;
                gctBOOL swizzled = gcvFALSE;
                gceTEXTURE_SWIZZLE backedSwizzle[gcvTEXTURE_COMPONENT_NUM] = {gcvTEXTURE_SWIZZLE_INVALID};
                static const gceTEXTURE_SWIZZLE defaultSwizzle[gcvTEXTURE_COMPONENT_NUM] =
                {
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_G,
                    gcvTEXTURE_SWIZZLE_B,
                    gcvTEXTURE_SWIZZLE_A,
                };

                static const gcePROGRAM_STAGE shaderKindToProgStage[] =
                {
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_UNKNOWN = 0 */
                    gcvPROGRAM_STAGE_VERTEX,   /* gcSHADER_TYPE_VERTEX */
                    gcvPROGRAM_STAGE_FRAGMENT, /* gcSHADER_TYPE_FRAGMENT */
                    gcvPROGRAM_STAGE_COMPUTE,  /* gcSHADER_TYPE_COMPUTE */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_CL */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_PRECOMPILED */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_LIBRARY */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_VERTEX_DEFAULT_UBO */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO */
                    gcvPROGRAM_STAGE_TCS,      /* gcSHADER_TYPE_TCS */
                    gcvPROGRAM_STAGE_TES,      /* gcSHADER_TYPE_TES */
                    gcvPROGRAM_STAGE_GEOMETRY, /* gcSHADER_TYPE_GEOMETRY */
                    gcvPROGRAM_STAGE_LAST,     /* gcSHADER_KIND_COUNT */
                };

                gcSHADER_KIND shaderKind = GetUniformShaderKind(program->samplerMap[sampler].uniform);
                gcePROGRAM_STAGE progStage = shaderKindToProgStage[shaderKind];

                gcmASSERT(progStage != gcvPROGRAM_STAGE_LAST);

                samplerBaseOffset = pgInstance->programState.hints->samplerBaseOffset[progStage];

                /* Query samplers for extra sampler IDs. */
                if (gcmIS_ERROR(gcQueryFormatConvertionDirectiveSampler(
                        pgInstance->recompilePatchInfo.recompilePatchDirectivePtr,
                        program->samplerMap[sampler].uniform,
                        program->samplerMap[sampler].arrayIndex,
                        samplerBaseOffset,
                        (gctUINT*)&samplers,
                        &layers,
                        &swizzled)))
                {
                    layers = 1;
                    samplers[0] = sampler;
                }

                if (swizzled)
                {
                    gcoOS_MemCopy(backedSwizzle, chipCtx->texture.halTexture[unit].swizzle,
                                  gcvTEXTURE_COMPONENT_NUM * sizeof(gceTEXTURE_SWIZZLE));
                    gcoOS_MemCopy(chipCtx->texture.halTexture[unit].swizzle, defaultSwizzle,
                                  gcvTEXTURE_COMPONENT_NUM * sizeof(gceTEXTURE_SWIZZLE));
                }

                GL_ASSERT(layers >= 1 && layers <= gcdMAX_SURF_LAYERS);
                while (layers--)
                {
                    if (samplers[layers] != (gctUINT)-1)
                    {
                        if (chipCtx->chipFeature.hasTxDescriptor)
                        {
                            gcmERR_BREAK(
                                gcoTEXTURE_BindTextureDesc(
                                    texInfo->object,
                                    samplers[layers],
                                    &chipCtx->texture.halTexture[unit],
                                    layers));
                        }
                        else
                        {
                            /* Bind to the sampler. */
                            gcmERR_BREAK(gcoTEXTURE_BindTextureEx(
                                texInfo->object,
                                0,
                                samplers[layers],
                                &chipCtx->texture.halTexture[unit],
                                layers));
                        }
                    }
                }

                if (swizzled)
                {
                    gcoOS_MemCopy(chipCtx->texture.halTexture[unit].swizzle, backedSwizzle,
                                  gcvTEXTURE_COMPONENT_NUM * sizeof(gceTEXTURE_SWIZZLE));
                }

                gcmONERROR(status);
            }
            else
            {
                if (chipCtx->chipFeature.hasTxDescriptor)
                {
                    gcmONERROR(
                        gcoTEXTURE_BindTextureDesc(
                            texInfo->object,
                            sampler,
                            &chipCtx->texture.halTexture[unit],
                            0));
                }
                else
                {
                    /* Bind to the sampler. */
                    gcmONERROR(gcoTEXTURE_BindTextureEx(
                        texInfo->object,
                        0,
                        sampler,
                        &chipCtx->texture.halTexture[unit],
                        0));
                }
            }
            /* Clear object dirty as all changed states are flushed to descriptor memory */
            texObj->uObjStateDirty.objStateDirty = 0;

#if gcdFRAMEINFO_STATISTIC
            if (g_dbgDumpImagePerDraw & __GL_PERDRAW_DUMP_TEXTURE)
            {
                gcmVERIFY_OK(gcChipUtilsDumpTexture(gc, texObj));
            }
#endif

            if (gc->texUnitAttrState[unit] & __GL_TEX_IMAGE_CONTENT_CHANGED_BIT)
            {
                /* Invalidate texture cache once, based previous used stage. */
                if (chipCtx->texture.preStageMask & (1 << __GLSL_STAGE_VS))
                {
                    chipCtx->texture.preStageMask &= ~(1 << __GLSL_STAGE_VS);
                    gcmONERROR(gcoTEXTURE_FlushVS(texInfo->object));
                }
                if (chipCtx->texture.preStageMask & (1 << __GLSL_STAGE_FS))
                {
                    chipCtx->texture.preStageMask &= ~(1 <<__GLSL_STAGE_FS);
                    gcmONERROR(gcoTEXTURE_Flush(texInfo->object));
                }
            }

            if (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_UNIFIED_SAMPLERS))
            {
                /* Unified sampler need to mark both VS and PS used */
                chipCtx->texture.curStageMask |= ((1 << __GLSL_STAGE_VS) | (1 << __GLSL_STAGE_FS));
            }
            else
            {
                switch (program->samplerMap[sampler].stage)
                {
                case __GLSL_STAGE_FS:
                case __GLSL_STAGE_CS:
                    chipCtx->texture.curStageMask |= (1 << __GLSL_STAGE_FS);
                    break;
                default:
                    chipCtx->texture.curStageMask |= (1 << __GLSL_STAGE_VS);
                    break;
                }
            }
        }
        else
        {
            /* Disable the sampler */
            gcmONERROR(gcoTEXTURE_Disable(chipCtx->hal, sampler, (gctBOOL)program->samplerMap[sampler].isInteger));

            if (!chipCtx->chipFeature.txDefaultValueFix && program->samplerMap[sampler].isInteger)
            {
                __GLES_PRINT("ES30(todo): need recompilation for alpha channel.");
            }
        }
    } /* End of while */

    for (index = 0; index < gc->constants.shaderCaps.maxCombinedTextureImageUnits; index++)
    {
        __GLtexUnit2Sampler *texUnit2SamplerTmp = &gc->shaderProgram.texUnit2Sampler[index];
#if gcdSYNC
        gcoSURF map = gcvNULL;
#endif

        if((texUnit2SamplerTmp->numSamplers > 0) &&
            (pgInstance->extraSamplerMap[index].subUsage != __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER))
        {
            __GLtextureObject *texObjTmp = gc->texture.units[index].currentTexture;
            if(texObjTmp != gcvNULL)
            {
#if gcdSYNC
                if (texObjTmp->bufObj)
                {
                    __GLchipVertexBufferInfo *bufInfo = (__GLchipVertexBufferInfo *)(texObjTmp->bufObj->privateData);
                    if (bufInfo)
                    {
                        gcmONERROR(gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_READ));
                    }
                }
                else
                {
                    __GLchipTextureInfo *texInfoTmp = (__GLchipTextureInfo*)texObjTmp->privateData;
                    /* we don't send fence for egl image */
                    if ((texInfoTmp != gcvNULL) && (texInfoTmp->eglImage.image == gcvNULL))
                    {
                        /* Only Mip 0 now, latter I may add other mipmap base on the filter */
                        gcmONERROR(gcoTEXTURE_GetMipMap(texInfoTmp->object, texObjTmp->params.baseLevel, &map));
                        gcmONERROR(gcoSURF_GetFence(map, gcvFENCE_TYPE_READ));
                    }
                }
#endif
            }
        }
    }

    if (!__glBitmaskIsAllZero(&samplerStateDirty) ||
        !__glBitmaskIsAllZero(&samplerMapDirty))
    {
        gcsTEXTURE_BINDTEXTS_ARGS args;
        args.version = gcvHAL_ARG_VERSION_V1;
        gcmONERROR(gcoTEXTURE_BindTextureTS(&args));
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateTexture(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (!__glBitmaskIsAllZero(&gc->texUnitAttrDirtyMask))
    {
        __GLtextureObject *tex;
        __GLbitmask unitMask = gc->texUnitAttrDirtyMask;
        GLuint localMask;
        GLint  unit = -1;

        while (!__glBitmaskIsAllZero(&unitMask))
        {
            if (!__glBitmaskTestAndClear(&unitMask, ++unit))
            {
                continue;
            }

            localMask = gc->texUnitAttrState[unit];
            tex = gc->texture.units[unit].currentTexture;

            if (localMask & __GL_TEXPARAMETER_BITS)
            {
                gcChipSetTextureParameters(gc, tex, unit, localMask);
            }
        }
    }

    /* Clear curStageMask before validating samplers */
    chipCtx->texture.curStageMask = 0;

    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipValidateProgramSamplersCB));

    /* Mark curStageMask will be used in this draw */
    chipCtx->texture.preStageMask |= chipCtx->texture.curStageMask;


OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateXFB(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLxfbObject *xfbObj = gc->xfb.boundXfbObj;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (chipCtx->chipFeature.hasHwTFB && xfbObj->active)
    {
        GLuint i;
        __GLprogramObject *progObj = __glGetLastNonFragProgram(gc);
        __GLchipSLProgram *chipProg = (__GLchipSLProgram*)progObj->privateData;
        __GLbitmask bindingDirty = gc->bufferObject.bindingDirties[__GL_XFB_BUFFER_INDEX];
        gctUINT32 physical;
        gctUINT32 sizeRange;
        __GLchipVertexBufferInfo *bufInfo = gcvNULL;
        __GLBufBindPoint *pBindingPoints = xfbObj->boundBufBinding;
        __GLBufBindPoint *xfbBindingPoint;

        if (gc->xfb.dirtyState & __GL_XFB_DIRTY_OBJECT)
        {
            __GLchipXfbHeader *chipXfb = (__GLchipXfbHeader *)xfbObj->privateData;

            if (chipXfb->headerLocked == gcvNULL)
            {
                gcoSURF_LockNode(&chipXfb->headerNode, gcvNULL, (gctPOINTER *)&chipXfb->headerLocked);
            }

            gcmGETHARDWAREADDRESS(chipXfb->headerNode, physical);

            gcmONERROR(gco3D_SetXfbHeader(chipCtx->engine, physical));

            __glBitmaskSetAll(&bindingDirty, gcvTRUE);

            gc->xfb.dirtyState = 0;
        }

        if (progObj->bindingInfo.xfbMode == GL_INTERLEAVED_ATTRIBS)
        {
            if (__glBitmaskTest(&bindingDirty, 0))
            {
                xfbBindingPoint = &pBindingPoints[0];
                GL_ASSERT(xfbBindingPoint->boundBufObj);
                bufInfo = (__GLchipVertexBufferInfo*)(xfbBindingPoint->boundBufObj->privateData);
#if gcdSYNC
                gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_WRITE);
#endif
                gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));

                physical += (gctUINT32)xfbBindingPoint->bufOffset;

                sizeRange = (xfbBindingPoint->bufSize == 0) ?
                                (gctUINT32)(bufInfo->size - xfbBindingPoint->bufOffset) :
                                (gctUINT32)xfbBindingPoint->bufSize;

                gcmONERROR(gco3D_SetXfbBuffer(chipCtx->engine,
                        0, physical, chipProg->xfbStride, sizeRange));

                gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
            }
        }
        else
        {
            GL_ASSERT(progObj->bindingInfo.xfbMode == GL_SEPARATE_ATTRIBS);

            for (i = 0; i < progObj->bindingInfo.numActiveXFB; i++)
            {
                if (__glBitmaskTestAndClear(&bindingDirty, i))
                {
                    xfbBindingPoint = &pBindingPoints[i];
                    GL_ASSERT(xfbBindingPoint->boundBufObj);
                    bufInfo = (__GLchipVertexBufferInfo*)(xfbBindingPoint->boundBufObj->privateData);
#if gcdSYNC
                    gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_WRITE);
#endif
                    gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));

                    physical += (gctUINT32)xfbBindingPoint->bufOffset;

                    sizeRange = (xfbBindingPoint->bufSize == 0) ?
                                    (gctUINT32)(bufInfo->size - xfbBindingPoint->bufOffset) :
                                    (gctUINT32)xfbBindingPoint->bufSize;

                    gcmONERROR(gco3D_SetXfbBuffer(chipCtx->engine,
                            i, physical, chipProg->xfbVaryings[i].stride, sizeRange));

                    gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
                }

                if (__glBitmaskIsAllZero(&bindingDirty))
                {
                    break;
                }
            }
        }
    }
    else
    {
        __GLprogramObject *vsProgrObj = gc->shaderProgram.activeProgObjs[__GLSL_STAGE_VS];
        __GLchipSLProgram *vsProgram = (__GLchipSLProgram*)vsProgrObj->privateData;
        __GLchipSLProgramInstance* pgInstance = vsProgram->curPgInstance;
        gctINT enabled = ((xfbObj->active) && (!xfbObj->paused)) ? 1 : 0;

        if (pgInstance->xfbActiveUniform)
        {
            gcmONERROR(gcChipFlushSingleUniform(gc,
                                                vsProgram,
                                                pgInstance->xfbActiveUniform,
                                                (GLvoid*)&enabled));
            if (enabled)
            {
                gctUINT32 physical;
                __GLchipVertexBufferInfo *bufInfo = gcvNULL;
                __GLBufBindPoint *pBindingPoints = xfbObj->boundBufBinding;
                __GLBufBindPoint *xfbBindingPoint;
                GLuint vertexCount = gc->vertexArray.end - gc->vertexArray.start;

                if (pgInstance->xfbVertexCountPerInstance)
                {
                    gcmONERROR(gcChipFlushSingleUniform(gc,
                                                        vsProgram,
                                                        pgInstance->xfbVertexCountPerInstance,
                                                        (GLvoid*)&vertexCount
                                                        ));
                }

                if (vsProgrObj->bindingInfo.xfbMode == GL_INTERLEAVED_ATTRIBS)
                {
                    GL_ASSERT(pgInstance->xfbBufferCount == 1);

                    if (pgInstance->xfbBufferUniforms[0])
                    {
                        xfbBindingPoint = &pBindingPoints[0];

                        GL_ASSERT(xfbBindingPoint->boundBufObj);

                        bufInfo = (__GLchipVertexBufferInfo*)(xfbBindingPoint->boundBufObj->privateData);
#if gcdSYNC
                        gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_WRITE);
#endif
                        gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));
                        physical += xfbObj->offset * vsProgram->xfbStride;

                        gcmONERROR(gcChipFlushSingleUniform(gc,
                                                            vsProgram,
                                                            pgInstance->xfbBufferUniforms[0],
                                                            (GLvoid*)&physical));

                        gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
                    }
                }
                else
                {
                    GLuint index;

                    GL_ASSERT(vsProgrObj->bindingInfo.xfbMode == GL_SEPARATE_ATTRIBS);
                    GL_ASSERT(pgInstance->xfbBufferCount == vsProgram->xfbCount);

                    for (index = 0; index < vsProgram->xfbCount; ++index)
                    {
                        if (pgInstance->xfbBufferUniforms[index])
                        {
                            xfbBindingPoint = &pBindingPoints[index];

                            GL_ASSERT(xfbBindingPoint->boundBufObj);

                            bufInfo = (__GLchipVertexBufferInfo*)(xfbBindingPoint->boundBufObj->privateData);
#if gcdSYNC
                            gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_WRITE);
#endif
                            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));
                            physical += (gctUINT32)(xfbBindingPoint->bufOffset + xfbObj->offset * vsProgram->xfbVaryings[index].stride);

                            gcmONERROR(gcChipFlushSingleUniform(gc,
                                                                vsProgram,
                                                                pgInstance->xfbBufferUniforms[index],
                                                                (GLvoid*)&physical));

                            gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
                        }
                    }
                }

                xfbObj->offset += xfbObj->vertices;
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}


/* TODO: some misc states which are not covered by dirty bits, we can add it later*/
__GL_INLINE gceSTATUS
gcChipValidateMiscState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean pointPrimTypeSwitch = GL_FALSE;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    pointPrimTypeSwitch = (chipCtx->lastPrimitiveType != chipCtx->primitveType) &&
                          ((chipCtx->lastPrimitiveType == (GLenum)-1) ||
                           (chipCtx->lastPrimitiveType == GL_POINTS) ||
                           (chipCtx->primitveType == GL_POINTS));

    if (chipCtx->lastPrimitiveType != chipCtx->primitveType)
    {
        chipCtx->chipDirty.uDefer.sDefer.vsReload = 1;
    }

    if (pointPrimTypeSwitch || (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH))
    {
        gctBOOL enable = (GL_POINTS == chipCtx->primitveType) ? gcvTRUE : gcvFALSE;
        gcsHINT_PTR prePAhints = chipCtx->prePAProgram->curPgInstance->programState.hints;
        gcsHINT_PTR psHints = chipCtx->activePrograms[__GLSL_STAGE_FS]->curPgInstance->programState.hints;

        enable = enable && prePAhints->prePaShaderHasPointSize;

        /* Enable point size for points. */
        gcmONERROR(gco3D_SetPointSizeEnable(chipCtx->engine, enable));

        /* Enable point sprites for points. */
        gcmONERROR(gco3D_SetPointSprite(chipCtx->engine, enable && (psHints->usePointCoord[0] || psHints->usePointCoord[1])));

        /* Enable primtive-id. */
        gcmONERROR(gco3D_SetPrimitiveIdEnable(chipCtx->engine, ((psHints->primIdComponent != -1) && !prePAhints->prePaShaderHasPrimitiveId)));
    }

    /* Save current primitive type. */
    chipCtx->lastPrimitiveType = chipCtx->primitveType;

OnError:
    gcmFOOTER();
    return status;
}

/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/
gceSTATUS
gcChipSetCulling(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    gceCULL mode;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (gc->state.enables.polygon.cullFace)
    {
        if (gc->state.polygon.cullFace == GL_FRONT)
        {
            if (gc->state.polygon.frontFace == GL_CCW)
            {
                mode = gcvCULL_CW;
            }
            else
            {
                mode = gcvCULL_CCW;
            }
        }
        else if (gc->state.polygon.cullFace == GL_BACK)
        {
            if (gc->state.polygon.frontFace == GL_CCW)
            {
                mode = gcvCULL_CCW;
            }
            else
            {
                mode = gcvCULL_CW;
            }
        }
        else
        {
            mode = gcvCULL_NONE;
        }
    }
    else
    {
        mode = gcvCULL_NONE;
    }

    if ((chipCtx->drawYInverted) && (mode != gcvCULL_NONE))
    {
        mode = (mode == gcvCULL_CW)? gcvCULL_CCW : gcvCULL_CW;
    }

    gcmONERROR(gco3D_SetCulling(chipCtx->engine, mode));

OnError:
    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetAlphaBlend(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgramInstance *fsInstance = chipCtx->activePrograms[__GLSL_STAGE_FS]->curPgInstance;
    GLboolean shaderBlendPatch = (fsInstance->pgStateKeyMask.s.hasShaderBlend == 1)  ? GL_TRUE: GL_FALSE;
    gctUINT i, j;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        for (j = 0; j < chipCtx->rtHalMapping[i].numOfSlots; j++)
        {
            gctUINT halRTIndex = chipCtx->rtHalMapping[i].slots[j];

            GLboolean hwBlend = (!shaderBlendPatch &&
                                 !chipCtx->advBlendInShader &&
                                 gc->state.enables.colorBuffer.blend[i] &&
                                 (!fsInstance->pgStateKeyMask.s.hasAlphaBlend));

            gcmONERROR(gco3D_EnableBlendingIndexed(chipCtx->engine, halRTIndex, hwBlend));

            gcmONERROR(gcChipSetBlendEquationSeparateIndexed(chipCtx,
                                                             halRTIndex,
                                                             gc->state.raster.blendEquationRGB[i],
                                                             gc->state.raster.blendEquationAlpha[i]));

            gcmONERROR(gcChipSetBlendFuncSeparateIndexed(chipCtx,
                                                         halRTIndex,
                                                         gc->state.raster.blendSrcRGB[i],
                                                         gc->state.raster.blendDstRGB[i],
                                                         gc->state.raster.blendSrcAlpha[i],
                                                         gc->state.raster.blendDstAlpha[i]));

        }
    }

OnError:
    gcmFOOTER();
    return status;

}

gceSTATUS
gcChipSetColorMask(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLboolean Red, Green, Blue, Alpha;
    gctUINT8 enable;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i, j;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
    {
        Red = gc->state.raster.colorMask[i].redMask;
        Green = gc->state.raster.colorMask[i].greenMask;
        Blue = gc->state.raster.colorMask[i].blueMask;
        Alpha = gc->state.raster.colorMask[i].alphaMask;
        enable = (Red ? 1 : 0)
              | ((Green ? 1 : 0) << 1)
              | ((Blue ? 1 : 0)  << 2)
              | ((Alpha ? 1 : 0) << 3);

        for (j = 0; j < chipCtx->rtHalMapping[i].numOfSlots; j++)
        {
            status = gco3D_SetColorWriteIndexed(chipCtx->engine, chipCtx->rtHalMapping[i].slots[j], enable);
        }
    }

    gcmFOOTER();
    return status;
}


gceSTATUS
gcChipSetViewportScissor(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLviewport *pViewport = &(gc->state.viewport);
    GLint rtWidth  = (GLint)(chipCtx->drawRTWidth);
    GLint rtHeight = (GLint)(chipCtx->drawRTHeight);
    /* viewport size */
    GLint vpLeft, vpTop, vpRight, vpBottom;
    /* scissor size */
    GLint scLeft, scTop, scRight, scBottom;
    GLboolean scissorTest = gc->state.enables.scissorTest;
    __GLscissor *pScissor = &(gc->state.scissor);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (gc->state.enables.rasterizerDiscard && !chipCtx->chipFeature.hasHwTFB)
    {
        static __GLscissor zeroScissor = {0, 0, 0, 0};

        scissorTest = GL_TRUE;
        pScissor = &zeroScissor;
    }

    vpLeft   = pViewport->x;
    vpTop    = pViewport->y;
    vpRight  = pViewport->x + (GLint)chipCtx->viewportWidth;
    vpBottom = pViewport->y + (GLint)chipCtx->viewportHeight;

    if (scissorTest)
    {
        /* Intersect scissor with RT size, HW cannot handle out of range scissor */
        scLeft    = __GL_MIN(__GL_MAX(0, pScissor->scissorX), rtWidth);
        scTop     = __GL_MIN(__GL_MAX(0, pScissor->scissorY), rtHeight);
        scRight   = __GL_MIN(__GL_MAX(0, pScissor->scissorX + pScissor->scissorWidth), rtWidth);
        scBottom  = __GL_MIN(__GL_MAX(0, pScissor->scissorY + pScissor->scissorHeight), rtHeight);
    }
    else
    {
        scLeft    = 0;
        scTop     = 0;
        scRight   = rtWidth;
        scBottom  = rtHeight;
    }

    /* Intersect scissor with viewport. */
    scLeft   = __GL_MAX(scLeft, vpLeft);
    scTop    = __GL_MAX(scTop, vpTop);
    scRight  = __GL_MIN(scRight, vpRight);
    scBottom = __GL_MIN(scBottom, vpBottom);

    if (chipCtx->drawYInverted)
    {
        GLint temp;
        temp     = scTop;
        scTop    = rtHeight - scBottom;
        scBottom = rtHeight - temp;

        vpTop    = rtHeight - vpTop;
        vpBottom = rtHeight - vpBottom;
    }

    gcmONERROR(gco3D_SetViewport(chipCtx->engine, vpLeft, vpBottom, vpRight, vpTop));
    gcmONERROR(gco3D_SetScissors(chipCtx->engine, scLeft, scTop, scRight, scBottom));

OnError:
    gcmFOOTER();
    return status;
}

/********************************************************************
**
**  gcChipSetDrawBuffers
**
**  Set render target/depth/stencil surf into chipCtx.
**  Set chipDirty according to draw buffers change.
**
**  Parameters:      Describe the calling sequence
**                   rtOffset:
**                      Sometimes rt might from one face of cube texture,
**                      or one layer of 2DAarry/3D texture. HAL need the
**                      offset together with surface handle to determine
**                      rt address
**  Return Value:    Describe the returning sequence
**
********************************************************************/
gceSTATUS
gcChipSetDrawBuffers(
    __GLcontext * gc,
    GLuint        integerRT,
    GLuint        floatRT,
    gcsSURF_VIEW * rtViews,
    gcsSURF_VIEW * dView,
    gcsSURF_VIEW * sView,
    GLboolean     drawYInveted,
    GLuint        samples,
    GLboolean     useDefault,
    GLuint        width,
    GLuint        height,
    GLboolean     layered,
    GLuint        maxLayers
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLuint i;
    GLuint rtWidth = 0xFFFFFFFF, tmpWidth;
    GLuint rtHeight = 0xFFFFFFFF, tmpHeight;
    __GLchipDirty *chipDirty = &chipCtx->chipDirty;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint drawRTnum = 0;

    gcmHEADER_ARG("gc=0x%x integerRT=%d floatRT=%d rtViews=0x%x dView=0x%x sView=0x%x "
                  "useDefault=%d width=%d height=%d layered=%d maxLayers=%d",
                   gc, integerRT, floatRT, rtViews, dView, sView, useDefault,
                   width, height, layered, maxLayers);

    /* Set Integer for drawing */
    if (chipCtx->drawInteger != integerRT)
    {
        chipCtx->drawInteger = integerRT;
    }

    /* Set float for drawing */
    if (chipCtx->drawFloat ^ floatRT)
    {
        chipCtx->drawFloat = (GLboolean)floatRT;
    }

    /* set draw yInverted flag */
    if (chipCtx->drawYInverted ^ drawYInveted)
    {
        chipCtx->drawYInverted = drawYInveted;
        chipDirty->uDefer.sDefer.viewportScissor = 1;
        chipDirty->uDefer.sDefer.culling = 1;
    }

    if (samples != chipCtx->drawRTSamples)
    {
        chipCtx->drawRTSamples = samples;
        chipDirty->uBuffer.sBuffer.rtSamplesDirty = 1;
    }

    GL_ASSERT(rtViews && dView && sView);

    /* Reset state */
    chipCtx->drawRTRotation = gcvSURF_0_DEGREE;

    /* Set correct render target for drawing */
    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        if (gcoOS_MemCmp(&chipCtx->drawRtViews[i], &rtViews[i], sizeof(gcsSURF_VIEW)) != gcvSTATUS_OK)
        {
            chipCtx->drawRtViews[i] = rtViews[i];
            chipDirty->uBuffer.sBuffer.rtSurfDirty = GL_TRUE;
        }

        if (rtViews[i].surf)
        {
            gcsSURF_FORMAT_INFO_PTR formatInfo;
            gcmONERROR(gcoSURF_GetSize(rtViews[i].surf, &tmpWidth, &tmpHeight, gcvNULL));

            if(tmpWidth < rtWidth)
            {
                rtWidth = tmpWidth;
            }

            if(tmpHeight < rtHeight)
            {
                rtHeight = tmpHeight;
            }

            /* Count */
            gcmVERIFY_OK(gcoSURF_GetFormatInfo(rtViews[i].surf, &formatInfo));
            drawRTnum += formatInfo->layers;
        }
    }

    if (chipCtx->drawRTnum != drawRTnum)
    {
        chipDirty->uBuffer.sBuffer.rtNumberDirty = GL_TRUE;

        if ((chipCtx->drawRTnum == 0) || (drawRTnum == 0))
        {
            chipDirty->uBuffer.sBuffer.zeroRTDirty = GL_TRUE;
        }

        chipCtx->drawRTnum = drawRTnum;
    }

    if (chipCtx->drawDepthView.firstSlice != dView->firstSlice)
    {
        chipCtx->drawDepthView.firstSlice = dView->firstSlice;
        chipDirty->uBuffer.sBuffer.zOffsetDirty = GL_TRUE;
    }

    if (chipCtx->drawStencilView.firstSlice != sView->firstSlice)
    {
        chipCtx->drawStencilView.firstSlice = sView->firstSlice;
        chipDirty->uBuffer.sBuffer.sOffsetDirty = GL_TRUE;
    }

    if (chipCtx->drawDepthView.surf != dView->surf)
    {
        chipCtx->drawDepthView.surf = dView->surf;
        chipDirty->uBuffer.sBuffer.zSurfDirty = GL_TRUE;
    }

    if (chipCtx->drawStencilView.surf != sView->surf)
    {
        chipCtx->drawStencilView.surf = sView->surf;
        chipDirty->uBuffer.sBuffer.sSurfDirty = GL_TRUE;
    }

    if ((chipCtx->drawLayered != layered) ||
        (chipCtx->drawMaxLayers != maxLayers))
    {
        chipCtx->drawLayered = layered;
        chipCtx->drawMaxLayers = maxLayers;
        chipDirty->uBuffer.sBuffer.layeredDirty =  GL_TRUE;
    }

#if (defined(DEBUG) || defined(_DEBUG))
    if (gc->apiVersion == __GL_API_VERSION_ES30)
    {
        GL_ASSERT((chipCtx->drawDepthView.surf && chipCtx->drawStencilView.surf) ?
                  (chipCtx->drawDepthView.surf == chipCtx->drawStencilView.surf) :
                  GL_TRUE);
    }
#endif

    if (chipCtx->drawRTnum == 0)
    {
        gcoSURF zsSurf = dView->surf ? dView->surf : sView->surf;
        if (zsSurf)
        {
            gcmONERROR(gcoSURF_GetSize(zsSurf, &rtWidth, &rtHeight, gcvNULL));
        }
    }

    if (useDefault)
    {
        GL_ASSERT(chipCtx->drawRTnum == 0 && dView->surf == gcvNULL);
        rtWidth = width;
        rtHeight = height;
    }

    chipCtx->drawNoAttach = useDefault;

    if (chipCtx->drawRTWidth != rtWidth || chipCtx->drawRTHeight != rtHeight)
    {
        chipCtx->drawRTWidth = rtWidth;
        chipCtx->drawRTHeight = rtHeight;
        chipDirty->uDefer.sDefer.viewportScissor = 1;
    }

    if (chipCtx->rtTexture)
    {
        gcoSURF texSurface;
        gctUINT32 width, height;
        gceSURF_FORMAT format;
        gceSURF_FORMAT rtFormat = gcvSURF_UNKNOWN;
        gcmONERROR(gcoTEXTURE_GetMipMap(chipCtx->rtTexture, 0, &texSurface));
        gcmONERROR(gcoSURF_GetSize(texSurface, &width, &height, gcvNULL));
        gcmONERROR(gcoSURF_GetFormat(texSurface, gcvNULL, &format));
        if (chipCtx->drawRtViews[0].surf)
        {
            gcmONERROR(gcoSURF_GetFormat(chipCtx->drawRtViews[0].surf, gcvNULL, &rtFormat));
        }
        if ((width != chipCtx->drawRTWidth)   ||
            (height != chipCtx->drawRTHeight) ||
            (format != rtFormat))
        {
            gcmONERROR(gcoTEXTURE_Destroy(chipCtx->rtTexture));
            chipCtx->rtTexture = gcvNULL;
        }
    }

    /*
    ** Check if total RT number exceed hw support limit
    */
    if (chipDirty->uBuffer.sBuffer.rtNumberDirty)
    {
        if (drawRTnum > chipCtx->maxDrawRTs)
        {
            gc->flags |= __GL_CONTEXT_SKIP_DRAW_UNSPPORTED_MODE;
        }
        else
        {
            gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_UNSPPORTED_MODE;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipSetReadBuffers(
    __GLcontext * gc,
    GLuint        integerRT,
    gcsSURF_VIEW * rtView,
    gcsSURF_VIEW * dView,
    gcsSURF_VIEW * sView,
    GLboolean     readYInverted,
    GLboolean     layered
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x integerRT=%d rtView=0x%x dView=0x%x sView=0x%x readYInverted=%d layered=%d",
                  gc, integerRT, rtView, dView, sView, readYInverted, layered);

    GL_ASSERT(rtView && dView && sView);

    chipCtx->readInteger     = (GLboolean)integerRT;
    chipCtx->readRtView      = *rtView;
    chipCtx->readDepthView   = *dView;
    chipCtx->readStencilView = *sView;
    chipCtx->readYInverted   = readYInverted;
    chipCtx->readLayered     = layered;

    if (rtView->surf)
    {
        gctUINT width, height;
        gcmONERROR(gcoSURF_GetSize(rtView->surf, &width, &height, gcvNULL));
        chipCtx->readRTWidth  = (gctSIZE_T)width;
        chipCtx->readRTHeight = (gctSIZE_T)height;
    }
    else
    {
        chipCtx->readRTWidth  = 0;
        chipCtx->readRTHeight = 0;
    }

OnError:
    gcmFOOTER();
    return status;
}

/* Recompile shader if needed. */
static gceSTATUS
gcChipRecompileShader(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    __GLchipProgramStateKey* pgStateKey =  pgInstance->pgStateKey;
    __GLchipProgramStateKeyMask* pgStateKeyMask = &pgInstance->pgStateKeyMask;
    __GLchipRecompileInfo *recompileInfo = &pgInstance->recompilePatchInfo;
    gctBOOL recompileNoDirective = gcvFALSE;
    gctUINT32 recompileOptions = 0;
    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    /* Currently, we have two methods of recompiling, one is full-time compile and the other is
       machine code level modification which can not introduce any resources currently. We will
       merge 2nd one to 1st one later */

    /* 1, Tex format patch*/
    if (pgStateKeyMask->s.hasTexPatchFmt)
    {
        __GLtextureObject *texObj;
        gcsSURF_FORMAT_INFO_PTR txFormatInfo;
        GLuint unit, sampler;

        for (sampler = 0; sampler < gc->constants.shaderCaps.maxTextureSamplers; sampler++ )
        {
            if (program->samplerMap[sampler].stage< __GLSL_STAGE_LAST &&
                pgStateKey->texPatchInfo[sampler].format != gcvSURF_UNKNOWN)
            {
                gctBOOL depthStencilMode = gcvTRUE;

                unit = program->samplerMap[sampler].unit;

                texObj  = gc->texture.units[unit].currentTexture;

                gcmONERROR(gcChipTexGetFormatInfo(gc, texObj, &txFormatInfo));

                depthStencilMode = (texObj->params.depthStTexMode == GL_DEPTH_COMPONENT) ? gcvTRUE : gcvFALSE;

                if (!gcIsSameInputDirectiveExist(program->samplerMap[sampler].uniform,
                                                 (gctINT)program->samplerMap[sampler].arrayIndex,
                                                 recompileInfo->recompilePatchDirectivePtr))
                {
                    /* Create patch directives. */
                    gcCreateInputConversionDirective(program->samplerMap[sampler].uniform,
                                                     (gctINT)program->samplerMap[sampler].arrayIndex,
                                                     txFormatInfo,
                                                     pgStateKey->texPatchInfo[sampler].swizzle,
                                                     0,gcTEXTURE_MODE_NONE, gcTEXTURE_MODE_NONE, gcTEXTURE_MODE_NONE, 0.0,
                                                     0, 0, 0, 0, 0, 0, 0, gcvFALSE, gcvFALSE,
                                                     depthStencilMode,
                                                     pgStateKey->texPatchInfo[sampler].needFormatConvert,
                                                     &recompileInfo->recompilePatchDirectivePtr);
                }
            }
        }
    }

    /* 2, Shadow map patch */
    if (pgStateKeyMask->s.hasShadowMapCmp)
    {
        GLuint sampler;
        gcsSURF_FORMAT_INFO_PTR texFormatInfo;
        for (sampler = 0; sampler < gc->constants.shaderCaps.maxTextureSamplers; sampler++)
        {
            if (program->samplerMap[sampler].stage < __GLSL_STAGE_LAST &&
                pgStateKey->shadowMapCmpInfo[sampler].texFmt != gcvSURF_UNKNOWN)
            {
                gcmONERROR(gcoSURF_QueryFormat(pgStateKey->shadowMapCmpInfo[sampler].texFmt, &texFormatInfo));

                if (!gcIsSameDepthComparisonDirectiveExist(texFormatInfo,
                                                           program->samplerMap[sampler].uniform,
                                                           program->samplerMap[sampler].arrayIndex,
                                                           pgStateKey->shadowMapCmpInfo[sampler].cmp.cmpMode,
                                                           pgStateKey->shadowMapCmpInfo[sampler].cmp.cmpOp,
                                                           recompileInfo->recompilePatchDirectivePtr))
                {
                    gcmONERROR(gcCreateDepthComparisonDirective(texFormatInfo,
                                                                program->samplerMap[sampler].uniform,
                                                                program->samplerMap[sampler].arrayIndex,
                                                                pgStateKey->shadowMapCmpInfo[sampler].cmp.cmpMode,
                                                                pgStateKey->shadowMapCmpInfo[sampler].cmp.cmpOp,
                                                                &recompileInfo->recompilePatchDirectivePtr));
                }
            }
        }
    }


    /* 3, rt format patch */
    if (pgStateKeyMask->s.hasRtPatchFmt)
    {
        GLuint i;
        gcsSURF_FORMAT_INFO_PTR rtFormatInfo;
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (pgStateKey->staticKey->rtPatchFmt[i] != gcvSURF_UNKNOWN)
            {
                gcmONERROR(gcoSURF_QueryFormat(pgStateKey->staticKey->rtPatchFmt[i], &rtFormatInfo));
                gcmONERROR(gcCreateOutputConversionDirective(i,
                                                             rtFormatInfo,
                                                             0,
                                                             gcvFALSE,
                                                             &recompileInfo->recompilePatchDirectivePtr));
            }
        }
    }

    /* 4. Blend patch in shader */
    if (pgStateKeyMask->s.hasShaderBlend)
    {
        if (pgStateKey->staticKey->shaderBlendPatch)
        {
            gcmONERROR(gcCreateAlphaBlendingDirective(0,
                                                      gcvFALSE,
                                                      &recompileInfo->recompilePatchDirectivePtr));

            /* Add additional color factor to blend in 255 space. */
            if (chipCtx->chipFeature.haltiLevel > __GL_CHIP_HALTI_LEVEL_1)
            {
                gctFLOAT factor = 255.5f/255.0f;

                gcmONERROR(gcCreateColorFactoringDirective(0,
                                                           1,
                                                           &factor,
                                                           gcvFALSE,
                                                           &recompileInfo->recompilePatchDirectivePtr));
            }
        }
    }

#if gcdUSE_NPOT_PATCH
    /* 5. NPOT patch in shader. */
    if (pgStateKeyMask->s.hasNP2AddrMode)
    {
        GLuint sampler;
        GLuint count = 0;
        gcNPOT_PATCH_PARAM texParams[__GL_MAX_GLSL_SAMPLERS];

        gcoOS_ZeroMemory(&texParams, gcmSIZEOF(texParams));
        for (sampler = 0; sampler < gc->constants.shaderCaps.maxTextureSamplers; sampler++ )
        {
            if (program->samplerMap[sampler].stage == __GLSL_STAGE_LAST)
            {
                continue;
            }

            if ((pgStateKey->NP2AddrMode[sampler].add.addrModeS != gcvTEXTURE_INVALID) ||
                (pgStateKey->NP2AddrMode[sampler].add.addrModeT != gcvTEXTURE_INVALID) ||
                (pgStateKey->NP2AddrMode[sampler].add.addrModeR != gcvTEXTURE_INVALID)
               )
            {
                texParams[count].addressMode[0] = (NP2_ADDRESS_MODE)(pgStateKey->NP2AddrMode[sampler].add.addrModeS - 1);
                texParams[count].addressMode[1] = (NP2_ADDRESS_MODE)(pgStateKey->NP2AddrMode[sampler].add.addrModeT - 1);
                texParams[count].addressMode[2] = (NP2_ADDRESS_MODE)(pgStateKey->NP2AddrMode[sampler].add.addrModeR - 1);
                texParams[count].samplerSlot    = GetUniformIndex(program->samplerMap[sampler].uniform);
                texParams[count].texDimension   = 2;  /* So far only apply texture 2d. */
                count++;
            }
        }

        if (count > 0)
        {
            gcCreateNP2TextureDirective(count,
                                        texParams,
                                        &recompileInfo->recompilePatchDirectivePtr);
        }
    }
#endif

    /* 7. remove alpha assignement */
    if (pgStateKeyMask->s.hasRemoveAlpha)
    {
        gcmONERROR(gcCreateRemoveAssignmentForAlphaChannel(pgStateKey->staticKey->removeAlpha,
                                                           &recompileInfo->recompilePatchDirectivePtr));

    }

    /* 8. draw yInverted */
    if (pgStateKeyMask->s.hasYinverted)
    {
        gcmONERROR(gcCreateYFlippedShaderDirective(&recompileInfo->recompilePatchDirectivePtr));
    }

    /* 9. sample mask */
    if (pgStateKeyMask->s.hasSampleMask || pgStateKeyMask->s.hasSampleCov || pgStateKeyMask->s.hasAlpha2Cov)
    {
        gctBOOL sampleMask  = pgStateKeyMask->s.hasSampleMask;
        gctBOOL sampleCov   = pgStateKeyMask->s.hasSampleCov;
        gctBOOL alphaCov    = pgStateKeyMask->s.hasAlpha2Cov;

        gcmONERROR(gcCreateSampleMaskDirective(alphaCov, sampleCov, sampleMask, &recompileInfo->recompilePatchDirectivePtr));
    }

    /* 10. tcs patch in vertics adjust */
    if (pgStateKeyMask->s.hasTcsPatchInVertices)
    {
        gcmONERROR(gcCreateTcsInputMismatch(pgStateKey->staticKey->tcsPatchInVertices, &recompileInfo->recompilePatchDirectivePtr));
    }

    /* 11. color kill */
    if (pgStateKeyMask->s.hasColorKill)
    {
        if (pgStateKey->staticKey->colorKill)
        {
            gcmONERROR(gcCreateColorKillDirective(0, &pgInstance->recompilePatchInfo.recompilePatchDirectivePtr));
        }
    }

      /* 12, shader blend */
    if (pgStateKeyMask->s.hasAlphaBlend)
    {
        if (pgStateKey->staticKey->alphaBlend)
        {
            gcmONERROR(gcCreateAlphaBlendDirective(0, &pgInstance->recompilePatchInfo.recompilePatchDirectivePtr));
        }
    }

     /* 13, shader polygon offset */
    if (pgStateKeyMask->s.hasPolygonOffset)
    {
        if (pgStateKey->staticKey->ShaderPolygonOffset)
        {
            gcmONERROR(gcCreateDepthBiasDirective(&pgInstance->recompilePatchInfo.recompilePatchDirectivePtr));
        }
    }

    /* 14, output highp conversion */
    if (pgStateKeyMask->s.hasPSOutHighpConversion)
    {
        recompileNoDirective = gcvTRUE;
        recompileOptions |= __GL_CHIP_RO_OUTPUT_HIGHP_CONVERSION;
    }

    if (recompileInfo->recompilePatchDirectivePtr || recompileNoDirective)
    {
        /* Do first kind of recompile */
        gcmONERROR(gcChipDynamicPatchProgram(gc, progObj, recompileInfo->recompilePatchDirectivePtr, recompileOptions));
    }

OnError:
    gcmASSERT(status == gcvSTATUS_OK);
    gcmFOOTER();
    return status;
}

static GLboolean
gcChipNeedRecompile(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipSLProgram *program,
    __GLchipProgramStateKey *pgStateKey,
    __GLchipProgramStateKeyMask* pgStateKeyMask
    )
{
    GLuint i;
    GLboolean ret;
    __GLchipProgramStateKey *pgKeyState = chipCtx->pgKeyState;
    __GLchipSLProgramInstance *masterInstance = program->masterPgInstance;
    GLbitfield stageBits[] =
    {
        gcvPROGRAM_STAGE_VERTEX_BIT,
        gcvPROGRAM_STAGE_TCS_BIT,
        gcvPROGRAM_STAGE_TES_BIT,
        gcvPROGRAM_STAGE_GEOMETRY_BIT,
        gcvPROGRAM_STAGE_FRAGMENT_BIT,
        gcvPROGRAM_STAGE_COMPUTE_BIT,
        0
    };

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x pgStateKey=0x%x pgStateKeyMask=0x%x", gc, chipCtx, pgStateKey, pgStateKeyMask);

    /* Initialize statekey and its mask to unneeded state */
    pgStateKeyMask->value = 0;

    for (i = 0; i < gc->constants.shaderCaps.maxTextureSamplers; i++)
    {
        /* Only if the program have the shader stage */
        if (program->stageBits & stageBits[program->samplerMap[i].stage])
        {
            if (pgKeyState->shadowMapCmpInfo[i].texFmt != gcvSURF_UNKNOWN)
            {
                __GL_MEMCOPY(&pgStateKey->shadowMapCmpInfo[i], &pgKeyState->shadowMapCmpInfo[i], sizeof(__GLchipPgStateKeyShadowMapCmpInfo));
                pgStateKeyMask->s.hasShadowMapCmp = 1;
            }

            if (pgKeyState->texPatchInfo[i].format != gcvSURF_UNKNOWN)
            {
                __GL_MEMCOPY(&pgStateKey->texPatchInfo[i], &pgKeyState->texPatchInfo[i], sizeof(__GLchipPgStateKeyTexPatchInfo));
                pgStateKeyMask->s.hasTexPatchFmt = 1;
            }

#if gcdUSE_NPOT_PATCH
            if ((pgKeyState->NP2AddrMode[i].add.addrModeS != gcvTEXTURE_INVALID) ||
                (pgKeyState->NP2AddrMode[i].add.addrModeT != gcvTEXTURE_INVALID) ||
                (pgKeyState->NP2AddrMode[i].add.addrModeR != gcvTEXTURE_INVALID)
               )
            {
                pgStateKey->NP2AddrMode[i].value = pgKeyState->NP2AddrMode[i].value;
                pgStateKeyMask->s.hasNP2AddrMode = 1;
            }
#endif
        }
    }

    if (program->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
    {
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (pgKeyState->staticKey->rtPatchFmt[i] != gcvSURF_UNKNOWN)
            {
                pgStateKey->staticKey->rtPatchFmt[i] = pgKeyState->staticKey->rtPatchFmt[i];
                pgStateKeyMask->s.hasRtPatchFmt = 1;
            }

            if (pgKeyState->staticKey->removeAlpha[i])
            {
                pgStateKey->staticKey->removeAlpha[i] = pgKeyState->staticKey->removeAlpha[i];
                pgStateKeyMask->s.hasRemoveAlpha = 1;
            }
        }

        if (pgKeyState->staticKey->shaderBlendPatch)
        {
            pgStateKey->staticKey->shaderBlendPatch = GL_TRUE;
            pgStateKeyMask->s.hasShaderBlend = 1;
        }

        if (pgKeyState->staticKey->drawYInverted)
        {
            pgStateKey->staticKey->drawYInverted = gcvTRUE;
            pgStateKeyMask->s.hasYinverted = 1;
        }

        if (pgKeyState->staticKey->sampleMask)
        {
            pgStateKey->staticKey->sampleMask = gcvTRUE;
            pgStateKeyMask->s.hasSampleMask = 1;
        }

        if (pgKeyState->staticKey->sampleCoverage)
        {
            pgStateKey->staticKey->sampleCoverage = gcvTRUE;
            pgStateKeyMask->s.hasSampleCov = 1;
        }

        if (pgKeyState->staticKey->alpha2Coverage)
        {
            pgStateKey->staticKey->alpha2Coverage = gcvTRUE;
            pgStateKeyMask->s.hasAlpha2Cov = 1;
        }

        if (pgKeyState->staticKey->colorKill)
        {
            pgStateKey->staticKey->colorKill = gcvTRUE;
            pgStateKeyMask->s.hasColorKill = 1;
        }

        if (pgKeyState->staticKey->alphaBlend)
        {
            pgStateKey->staticKey->alphaBlend = GL_TRUE;
            pgStateKeyMask->s.hasAlphaBlend = 1;
        }

        if (pgKeyState->staticKey->ShaderPolygonOffset)
        {
            pgStateKey->staticKey->ShaderPolygonOffset = GL_TRUE;
            pgStateKeyMask->s.hasPolygonOffset = 1;
        }

        if (chipCtx->pgKeyState->staticKey->highpConversion)
        {
            pgStateKey->staticKey->highpConversion = GL_TRUE;
            pgStateKeyMask->s.hasPSOutHighpConversion = 1;
        }
    }

    if (program->stageBits & gcvPROGRAM_STAGE_TCS_BIT)
    {
        pgStateKey->staticKey->tcsPatchInVertices = masterInstance->tcsPatchInVertices;
        if (pgKeyState->staticKey->tcsPatchInVertices != masterInstance->tcsPatchInVertices )
        {
            pgStateKey->staticKey->tcsPatchInVertices = pgKeyState->staticKey->tcsPatchInVertices;
            pgStateKeyMask->s.hasTcsPatchInVertices = 1;
        }
    }

    ret = (pgStateKeyMask->value != 0);
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

/* Key states collection must be as fast as possible since if no dirty exist, such
** collection is CPU-time wasted
** TODO: we need find a way to clear in-stale state
*/
static gceSTATUS
gcChipRecompileEvaluateKeyStates(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipSLProgram *program
    )
{
    __GLchipProgramStateKey *pgKeyState = chipCtx->pgKeyState;
    GLboolean progSwitched = (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH) ? GL_TRUE : GL_FALSE;
    GLboolean pgKeyDirty = progSwitched;
    __GLbitmask samplerMapDirty = gc->shaderProgram.samplerMapDirty;
    __GLbitmask samplerStateDirty = gc->shaderProgram.samplerStateDirty;
    __GLbitmask samplerDirtyMask;
    __GLbitmask shadowSamplerMask = program->shadowSamplerMask;
    GLuint unit;
    GLint sampler = -1;
    __GLbitmask samplerDirtyMaskCopy;
    GLuint maxSamplers[__GLSL_STAGE_LAST] = {0};
    GLuint numSamplers[__GLSL_STAGE_LAST] = {0};
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    if (progSwitched)
    {
        __glBitmaskInitAllOne(&samplerDirtyMask, gc->constants.shaderCaps.maxTextureSamplers);
    }
    else
    {
        __glBitmaskInitOR(&samplerDirtyMask, &samplerMapDirty, &samplerStateDirty);
    }

    samplerDirtyMaskCopy = samplerDirtyMask;

    if (program->progFlags.skipRecompile)
    {
        status = gcvSTATUS_FALSE;
        goto OnError;
    }

    if (!__glBitmaskIsAllZero(&samplerDirtyMask))
    {
        __GLSLStage stage;
        gctBOOL useUnifiedSampler = gcvFALSE;
        /* There are only 5 bits for sampler index, so the max sampler count is 32. */
        gctUINT32 maxSamplerCountForOneShader = 32;

        /* Get already used sampler count */
        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (program->masterPgInstance->binaries[stage])
            {
                gcmONERROR(gcSHADER_GetSamplerCount(program->masterPgInstance->binaries[stage], &numSamplers[stage]));
                useUnifiedSampler = (GetShaderSamplerAllocStrategy(program->masterPgInstance->binaries[stage]) != gcSHADER_ALLOC_STRATEGY_FIXED_ADDR_OFFSET);
            }
        }

        if (useUnifiedSampler)
        {
            maxSamplers[__GLSL_STAGE_VS]    = __GL_MIN(maxSamplerCountForOneShader,
                                                       gc->constants.shaderCaps.maxVertTextureImageUnits + gc->constants.shaderCaps.maxFragTextureImageUnits);
            maxSamplers[__GLSL_STAGE_FS]    =
            maxSamplers[__GLSL_STAGE_CS]    =
            maxSamplers[__GLSL_STAGE_TCS]   =
            maxSamplers[__GLSL_STAGE_TES]   =
            maxSamplers[__GLSL_STAGE_GS]    = maxSamplers[__GLSL_STAGE_VS];
        }
        else
        {
            maxSamplers[__GLSL_STAGE_VS]    = gc->constants.shaderCaps.maxVertTextureImageUnits;
            maxSamplers[__GLSL_STAGE_FS]    = gc->constants.shaderCaps.maxFragTextureImageUnits;
            maxSamplers[__GLSL_STAGE_CS]    = gc->constants.shaderCaps.maxCmptTextureImageUnits;
            maxSamplers[__GLSL_STAGE_TCS]   = gc->constants.shaderCaps.maxTcsTextureImageUnits;
            maxSamplers[__GLSL_STAGE_TES]   = gc->constants.shaderCaps.maxTesTextureImageUnits;
            maxSamplers[__GLSL_STAGE_GS]    = gc->constants.shaderCaps.maxGsTextureImageUnits;
        }
    }

    while (!__glBitmaskIsAllZero(&samplerDirtyMask))
    {
        __GLchipPgStateKeyShadowMapCmpInfo shadowMapCmpInfo;
        __GLchipPgStateKeyTexPatchInfo texPatchInfo;

#if gcdUSE_NPOT_PATCH
        GLuint rMode = gcvTEXTURE_INVALID;
        GLuint sMode = gcvTEXTURE_INVALID;
        GLuint tMode = gcvTEXTURE_INVALID;
#endif
        if (!__glBitmaskTestAndClear(&samplerDirtyMask, ++sampler))
        {
            continue;
        }

        __GL_MEMZERO(&shadowMapCmpInfo, sizeof(shadowMapCmpInfo));
        shadowMapCmpInfo.cmp.cmpMode = gcvTEXTURE_COMPARE_MODE_NONE;
        shadowMapCmpInfo.cmp.cmpOp   = gcvCOMPARE_LESS_OR_EQUAL;
        shadowMapCmpInfo.texFmt  = gcvSURF_UNKNOWN;

        __GL_MEMZERO(&texPatchInfo, sizeof(texPatchInfo));
        texPatchInfo.format = gcvSURF_UNKNOWN;
        texPatchInfo.needFormatConvert = GL_FALSE;

        unit = program->samplerMap[sampler].unit;

        if ((program->samplerMap[sampler].stage < __GLSL_STAGE_LAST) &&
            (__glBitmaskTest(&gc->texture.currentEnableMask, unit)))
        {
            __GLtextureObject *texObj = gc->texture.units[unit].currentTexture;
            __GLchipTextureInfo *texInfo = (__GLchipTextureInfo*)texObj->privateData;
            gcsSURF_FORMAT_INFO_PTR txFormatInfo = gcvNULL;
            gceSURF_FORMAT texFmt = gcvSURF_UNKNOWN;

            if (!isUniformUsedInShader(program->samplerMap[sampler].uniform) &&
                isUniformSamplerCalculateTexSize(program->samplerMap[sampler].uniform))
            {
                continue;
            }

            if (gcmIS_SUCCESS(gcChipTexGetFormatInfo(gc, texObj, &txFormatInfo)))
            {
                texFmt = txFormatInfo->format;
            }

            /* 1 Tex fmt key state */
            if (
                   (texFmt >= gcvSURF_X16B16G16R16F_2_A8R8G8B8 && texFmt <= gcvSURF_B10G11R11F_1_A8R8G8B8) ||
                   (texFmt >= gcvSURF_G32R32I_2_A8R8G8B8 && texFmt <= gcvSURF_B32G32R32UI_3_A8R8G8B8) ||
                   ( !(__glBitmaskTest(&shadowSamplerMask, sampler)) &&
                     (texFmt == gcvSURF_S8D32F_2_A8R8G8B8 || texFmt == gcvSURF_D24S8_1_A8R8G8B8)
                   ) ||
                   (isUniformUsedAsTexGatherSampler(program->samplerMap[sampler].uniform) &&
                    (texFmt == gcvSURF_R32F  || texFmt == gcvSURF_G32R32F ||
                     texFmt == gcvSURF_R32I  || texFmt == gcvSURF_G32R32I ||
                     texFmt == gcvSURF_R32UI || texFmt == gcvSURF_G32R32UI ||
                     texFmt == gcvSURF_S8D32F_1_G32R32F)
                   ) ||
                   (!(__glBitmaskTest(&shadowSamplerMask, sampler)) &&
                     (texFmt == gcvSURF_S8D32F_1_G32R32F) &&
                     (texObj->params.depthStTexMode == GL_STENCIL_INDEX)
                   ) ||
                   (!gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_INTEGER_SIGNEXT_FIX) &&
                    ((texFmt == gcvSURF_R8I) || (texFmt == gcvSURF_R16I)||
                     (texFmt == gcvSURF_G8R8I) ||(texFmt == gcvSURF_G16R16I) ||
                     (texFmt == gcvSURF_X8B8G8R8I) || (texFmt == gcvSURF_X16B16G16R16I) ||
                     (texFmt == gcvSURF_A8B8G8R8I) || (texFmt == gcvSURF_A16B16G16R16I)
                    )
                   ) ||
                   (!gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_INTEGER32_FIX) &&
                    ((texFmt == gcvSURF_R32I) || (texFmt == gcvSURF_R32UI) ||
                     (texFmt == gcvSURF_G32R32I) || (texFmt == gcvSURF_G32R32UI)
                    )
                   ) ||
                   (!(__glBitmaskTest(&shadowSamplerMask, sampler)) &&
                     (texFmt == gcvSURF_D24S8) &&
                     (!gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_D24S8_SAMPLE_STENCIL)) &&
                     (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3) &&
                     (texObj->params.depthStTexMode == GL_STENCIL_INDEX)
                   ))
            {
                __GLSLStage stage = program->samplerMap[sampler].stage;
                numSamplers[stage] += (txFormatInfo->layers - 1);
                if (numSamplers[stage] > maxSamplers[stage])
                {
                    gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
                }

                texPatchInfo.format = texFmt;
                texPatchInfo.needFormatConvert = GL_TRUE;
                texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_R] = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_R]);
                texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_G] = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_G]);
                texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_B] = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_B]);
                texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_A] = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_A]);
            }

            if(texFmt >= gcvSURF_A32F_1_R32F && texFmt <= gcvSURF_A32L32F_1_G32R32F)
            {

                static const gceTEXTURE_SWIZZLE baseComponents_000r[] =
                {
                    gcvTEXTURE_SWIZZLE_0,
                    gcvTEXTURE_SWIZZLE_0,
                    gcvTEXTURE_SWIZZLE_0,
                    gcvTEXTURE_SWIZZLE_R
                };
                static const gceTEXTURE_SWIZZLE baseComponents_rrrg[] =
                {
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_G
                };
                static const gceTEXTURE_SWIZZLE baseComponents_rrr1[] =
                {
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_R,
                    gcvTEXTURE_SWIZZLE_1
                };
                  gceTEXTURE_SWIZZLE swizzle;
                 const  gceTEXTURE_SWIZZLE *swizzArray = gcvNULL;
                 __GLSLStage stage = program->samplerMap[sampler].stage;
                numSamplers[stage] += (txFormatInfo->layers - 1);
                if (numSamplers[stage] > maxSamplers[stage])
                {
                    gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
                }

                switch (texFmt)
                {
                case gcvSURF_A32F_1_R32F:
                    swizzArray = baseComponents_000r;
                    break;
                case gcvSURF_L32F_1_R32F:
                    swizzArray = baseComponents_rrr1;
                    break;
                case gcvSURF_A32L32F_1_G32R32F:
                    swizzArray = baseComponents_rrrg;
                    break;

                default:
                    break;
                }
                gcmASSERT(swizzArray != gcvNULL);
                texPatchInfo.format = texFmt;
                texPatchInfo.needFormatConvert = GL_FALSE;

                swizzle = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_R]);

                if(swizzle >= gcvTEXTURE_SWIZZLE_0)
                {
                    texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_R] = swizzle;
                }
                else
                {
                    texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_R] = swizzArray[swizzle];
                }

                swizzle = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_G]);

                if(swizzle >= gcvTEXTURE_SWIZZLE_0)
                {
                    texPatchInfo.swizzle[__GL_TEX_COMPONENT_G] = swizzle;
                }
                else
                {
                    texPatchInfo.swizzle[__GL_TEX_COMPONENT_G] = swizzArray[swizzle];
                }

                swizzle = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_B]);

                if(swizzle >= gcvTEXTURE_SWIZZLE_0)
                {
                    texPatchInfo.swizzle[__GL_TEX_COMPONENT_B] = swizzle;
                }
                else
                {
                    texPatchInfo.swizzle[gcvTEXTURE_COMPONENT_B] = swizzArray[swizzle];
                }
                swizzle = gcChipUtilConvertTexSwizzle(texObj->params.swizzle[__GL_TEX_COMPONENT_A]);

                if(swizzle >= gcvTEXTURE_SWIZZLE_0)
                {
                    texPatchInfo.swizzle[__GL_TEX_COMPONENT_A] = swizzle;
                }
                else
                {
                    texPatchInfo.swizzle[__GL_TEX_COMPONENT_A] = swizzArray[swizzle];
                }

            }

            /* 2. Shadow map key state */
            if ((__glBitmaskTest(&shadowSamplerMask, sampler)) &&
                ((chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3) ?
                 (texFmt != gcvSURF_UNKNOWN) :
                 (texFmt == gcvSURF_S8D32F_1_G32R32F)
                )
               )
            {
                __GLsamplerObject *samplerObj = gc->texture.units[unit].boundSampler;
                __GLsamplerParamState *samplerStates = samplerObj
                                                     ? &samplerObj->params
                                                     : (__GLsamplerParamState*)&texObj->params;

                shadowMapCmpInfo.cmp.cmpMode = gcChipUtilConvertCompareMode(samplerStates->compareMode);
                shadowMapCmpInfo.cmp.cmpOp   = gcChipUtilConvertCompareFunc(samplerStates->compareFunc);
                shadowMapCmpInfo.texFmt  = texFmt;
            }

#if gcdUSE_NPOT_PATCH
            /* 3. NPOT key state. Only for 2D texture for now */
            if ((texInfo->isNP2) &&
                (texObj->targetIndex == __GL_TEXTURE_2D_INDEX) &&
                (chipCtx->chipFeature.patchNP2Texture))
            {
                gcoTEXTURE tex = texInfo->object;
                gcoSURF     mip = gcvNULL;
                /* Just to check whether it has only 1 level of mipmap. */
                gcoTEXTURE_GetMipMap(tex, texObj->params.baseLevel + 1, &mip);
                if ((!mip) ||
                    (texObj->params.sampler.minFilter == GL_NEAREST ||
                     texObj->params.sampler.minFilter == GL_LINEAR)
                   )
                {
                    if ((texObj->params.sampler.sWrapMode != GL_CLAMP_TO_EDGE) ||
                        (texObj->params.sampler.tWrapMode != GL_CLAMP_TO_EDGE) ||
                        (texObj->params.sampler.rWrapMode != GL_CLAMP_TO_EDGE)
                       )
                    {
                        rMode = gcChipUtilConvertWrapMode(texObj->params.sampler.rWrapMode);
                        sMode = gcChipUtilConvertWrapMode(texObj->params.sampler.sWrapMode);
                        tMode = gcChipUtilConvertWrapMode(texObj->params.sampler.tWrapMode);
                    }
                }
                else
                {
                    rMode = gcChipUtilConvertWrapMode(texObj->params.sampler.rWrapMode);
                    sMode = gcChipUtilConvertWrapMode(texObj->params.sampler.sWrapMode);
                    tMode = gcChipUtilConvertWrapMode(texObj->params.sampler.tWrapMode);
                }
            }
#endif
        }

        if (__GL_MEMCMP(&pgKeyState->texPatchInfo[sampler], &texPatchInfo, sizeof(texPatchInfo)) != 0)
        {
            __GL_MEMCOPY(&pgKeyState->texPatchInfo[sampler], &texPatchInfo, sizeof(texPatchInfo));
            pgKeyDirty = GL_TRUE;
        }

        if (__GL_MEMCMP(&pgKeyState->shadowMapCmpInfo[sampler], &shadowMapCmpInfo, sizeof(shadowMapCmpInfo)) != 0)
        {
            __GL_MEMCOPY(&pgKeyState->shadowMapCmpInfo[sampler], &shadowMapCmpInfo, sizeof(shadowMapCmpInfo));
            pgKeyDirty = GL_TRUE;
        }

#if gcdUSE_NPOT_PATCH
        if ((pgKeyState->NP2AddrMode[sampler].add.addrModeR != rMode) ||
            (pgKeyState->NP2AddrMode[sampler].add.addrModeS != sMode) ||
            (pgKeyState->NP2AddrMode[sampler].add.addrModeT != tMode))
        {
            pgKeyState->NP2AddrMode[sampler].add.addrModeR = (GLubyte)rMode;
            pgKeyState->NP2AddrMode[sampler].add.addrModeS = (GLubyte)sMode;
            pgKeyState->NP2AddrMode[sampler].add.addrModeT = (GLubyte)tMode;
            pgKeyDirty = GL_TRUE;
        }
#endif
    }

    if (program->stageBits & gcvPROGRAM_STAGE_FRAGMENT_BIT)
    {
        __GLchipDirty *chipDirty = &chipCtx->chipDirty;

        /* TODO: Refine later. Detect blend process. */
        if ((chipCtx->patchId == gcvPATCH_GTFES30) &&
            (chipCtx->chipFeature.haltiLevel < __GL_CHIP_HALTI_LEVEL_3) &&
            (gcvSTATUS_FALSE == gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_SINGLE_PIPE_HALTI1))
           )
        {
            if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_ALPHABLEND_ATTR_BITS) ||
                !__glBitmaskIsAllZero(&samplerDirtyMaskCopy))
            {
                __GLbitmask localMask;
                GLboolean hasETCformat = GL_FALSE;
                GLboolean needShaderPatch = GL_FALSE;

                sampler = -1;

                if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_ALPHABLEND_ATTR_BITS)
                {
                    __glBitmaskInitAllOne(&localMask, gc->constants.shaderCaps.maxTextureSamplers);
                }
                else
                {
                    localMask = samplerDirtyMaskCopy;
                }

                while (!__glBitmaskIsAllZero(&localMask))
                {
                    if (!__glBitmaskTestAndClear(&localMask, ++sampler))
                    {
                        continue;
                    }

                    unit = program->samplerMap[sampler].unit;

                    if ((program->samplerMap[sampler].stage < __GLSL_STAGE_LAST) &&
                        (__glBitmaskTest(&gc->texture.currentEnableMask, unit)))
                    {
                        __GLtextureObject *texObj = gc->texture.units[unit].currentTexture;
                        gcsSURF_FORMAT_INFO_PTR txFormatInfo;
                        gceSURF_FORMAT texFmt = gcvSURF_UNKNOWN;

                        if (gcmIS_SUCCESS(gcChipTexGetFormatInfo(gc, texObj, &txFormatInfo)))
                        {
                            texFmt = txFormatInfo->format;
                        }

                        if (texFmt == gcvSURF_RGBA8_ETC2_EAC)
                        {
                            hasETCformat = GL_TRUE;
                            break;
                        }
                    }
                }

                if (hasETCformat &&
                    (gc->state.enables.colorBuffer.blend[0]) &&
                    (gc->state.raster.blendSrcRGB[0]   == GL_SRC_ALPHA) &&
                    (gc->state.raster.blendSrcAlpha[0] == GL_SRC_ALPHA) &&
                    (gc->state.raster.blendDstRGB[0]   == GL_ZERO) &&
                    (gc->state.raster.blendDstAlpha[0] == GL_ZERO) &&
                    (gc->state.raster.blendEquationRGB[0]   == GL_FUNC_ADD) &&
                    (gc->state.raster.blendEquationAlpha[0] == GL_FUNC_ADD))
                {
                    needShaderPatch = GL_TRUE;
                }

                if (pgKeyState->staticKey->shaderBlendPatch != needShaderPatch)
                {
                    pgKeyState->staticKey->shaderBlendPatch = needShaderPatch;
                    pgKeyDirty = GL_TRUE;
                }
            }
        }

        /* rt format relevant recompilation */
        if (progSwitched ||
            chipDirty->uBuffer.sBuffer.rtSurfDirty ||
            (gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_BLEND_ENDISABLE_BIT))
        {
            GLuint i;
            for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
            {
                gceSURF_FORMAT rtFormat = gcvSURF_UNKNOWN;
                gceSURF_FORMAT rtPatchFormat = gcvSURF_UNKNOWN;

                if (chipCtx->drawRtViews[i].surf && i < program->maxOutLoc && program->loc2Out[i])
                {
                    gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawRtViews[i].surf, gcvNULL, &rtFormat));
                    if ((rtFormat >= gcvSURF_X16B16G16R16F_2_A8R8G8B8 && rtFormat <= gcvSURF_B10G11R11F_1_A8R8G8B8) ||
                        (rtFormat >= gcvSURF_G32R32I_2_A8R8G8B8 && rtFormat <= gcvSURF_B32G32R32UI_3_A8R8G8B8)      ||
                        (rtFormat >= gcvSURF_R8_1_X8R8G8B8 && rtFormat <= gcvSURF_G8R8_1_X8R8G8B8 && !gc->state.enables.colorBuffer.blend[i])   ||
                        (rtFormat == gcvSURF_A8B12G12R12_2_A8R8G8B8))
                    {
                        rtPatchFormat = rtFormat;
                    }
                }

                if (pgKeyState->staticKey->rtPatchFmt[i] != rtPatchFormat)
                {
                    pgKeyState->staticKey->rtPatchFmt[i] = rtPatchFormat;
                    pgKeyDirty = GL_TRUE;
                }
            }
        }

        if (chipDirty->uBuffer.sBuffer.rtSurfDirty || progSwitched)
        {
            GLuint i;
            gctBOOL drawYInverted = program->masterPgInstance->programState.hints->yInvertAware &&
                                    chipCtx->drawYInverted;

            if (drawYInverted != pgKeyState->staticKey->drawYInverted)
            {
                pgKeyState->staticKey->drawYInverted = drawYInverted;
                pgKeyDirty = GL_TRUE;
            }

            if(!gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_PSIO_DUAL16_32bpc_FIX))
            {
                for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
                {
                    if (chipCtx->drawRtViews[i].surf && i < program->maxOutLoc && program->loc2Out[i])
                    {
                        gceSURF_FORMAT rtFormat = gcvSURF_UNKNOWN;

                        gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawRtViews[i].surf, gcvNULL, &rtFormat));

                        switch (rtFormat)
                        {
                        case gcvSURF_R32F:
                        case gcvSURF_R32I:
                        case gcvSURF_R32UI:
                        case gcvSURF_G32R32F:
                        case gcvSURF_G32R32I:
                        case gcvSURF_G32R32UI:
                        case gcvSURF_A32B32G32R32F_2_G32R32F:
                        case gcvSURF_A32B32G32R32I_2_G32R32F:
                        case gcvSURF_A32B32G32R32UI_2_G32R32F:
                            pgKeyState->staticKey->highpConversion = gcvTRUE;
                            pgKeyDirty = GL_TRUE;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        if (chipDirty->uBuffer.sBuffer.rtSurfDirty || progSwitched ||
            (gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & (__GL_BLENDFUNC_BIT | __GL_BLEND_ENDISABLE_BIT)))
        {
            GLuint i, j;
            for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; i++)
            {
                GLboolean srcAlphaRequired = GL_FALSE;
                gceSURF_FORMAT rtFormat = gcvSURF_UNKNOWN;
                gctBOOL removeAlpha = gcvFALSE;

                if (gc->state.enables.colorBuffer.blend[i])
                {
                    GLenum blendFuncs[4];

                    blendFuncs[0] = gc->state.raster.blendSrcRGB[i];
                    blendFuncs[1] = gc->state.raster.blendSrcAlpha[i];
                    blendFuncs[2] = gc->state.raster.blendDstRGB[i];
                    blendFuncs[3] = gc->state.raster.blendDstAlpha[i];

                    for (j = 0; j < 4; j++)
                    {
                        switch (blendFuncs[j])
                        {
                        case GL_SRC_ALPHA:
                        case GL_ONE_MINUS_SRC_ALPHA:
                        case GL_SRC_ALPHA_SATURATE:
                            srcAlphaRequired = GL_TRUE;
                            break;

                        default:
                            break;
                        }

                        if (srcAlphaRequired)
                        {
                            break;
                        }
                    }
                }


                if (chipCtx->drawRtViews[i].surf)
                {
                    gcmVERIFY_OK(gcoSURF_GetFormat(chipCtx->drawRtViews[i].surf, gcvNULL, &rtFormat));

                    switch (rtFormat)
                    {
                    /* We may add more later if needed */
                    case gcvSURF_R5G6B5:
                    case gcvSURF_R5G5B5X1:
                    case gcvSURF_X8B8G8R8:
                    case gcvSURF_X8R8G8B8:
                        removeAlpha = gcvTRUE;
                        break;
                    default:
                        break;
                    }
                }

                removeAlpha = removeAlpha &&
                              (!srcAlphaRequired) &&
                              program->masterPgInstance->programState.hints->removeAlphaAssignment;

                if (pgKeyState->staticKey->removeAlpha[i] != removeAlpha)
                {
                    pgKeyState->staticKey->removeAlpha[i] = removeAlpha;
                    pgKeyDirty = GL_TRUE;
                }
            }
        }

        if (!chipCtx->chipFeature.msaaFragmentOperation)
        {
            if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_SAMPLE_MASK_ENDISABLE_BIT) ||
                progSwitched || chipDirty->uBuffer.sBuffer.rtSurfDirty)
            {
                gctBOOL sampleMask = gc->state.enables.multisample.sampleMask &&
                                     (chipCtx->drawRTSamples > 1);

                if (pgKeyState->staticKey->sampleMask != sampleMask)
                {
                    pgKeyState->staticKey->sampleMask = sampleMask;
                    pgKeyDirty = GL_TRUE;
                }
            }

            if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_SAMPLE_COVERAGE_ENDISABLE_BIT) ||
                progSwitched || chipDirty->uBuffer.sBuffer.rtSurfDirty)
            {
                gctBOOL sampleCov = gc->state.enables.multisample.coverage &&
                                    (chipCtx->drawRTSamples > 1);

                if (pgKeyState->staticKey->sampleCoverage != sampleCov)
                {
                    pgKeyState->staticKey->sampleCoverage = sampleCov;
                    pgKeyDirty = GL_TRUE;
                }
            }

            if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_SAMPLE_ALPHA_TO_COVERAGE_ENDISABLE_BIT) ||
                progSwitched || chipDirty->uBuffer.sBuffer.rtSurfDirty)
            {
                gctBOOL alphaCov = gc->state.enables.multisample.alphaToCoverage &&
                                   (chipCtx->drawRTSamples > 1);

                if (pgKeyState->staticKey->alpha2Coverage != alphaCov)
                {
                    pgKeyState->staticKey->alpha2Coverage = alphaCov;
                    pgKeyDirty = GL_TRUE;
                }
            }
        }
    }

    if (program->stageBits & gcvPROGRAM_STAGE_TCS_BIT)
    {
        if ((gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PATCH_VERTICES) ||
            progSwitched)
        {
            if (pgKeyState->staticKey->tcsPatchInVertices != gc->shaderProgram.patchVertices)
            {
                pgKeyState->staticKey->tcsPatchInVertices = gc->shaderProgram.patchVertices;
                pgKeyDirty = GL_TRUE;
            }
        }
    }

    /*Check color kill optimization for COC and AngryBirds.*/
    if (chipCtx->patchId == gcvPATCH_CLASHOFCLAN ||
        chipCtx->patchId == gcvPATCH_ANGRYBIRDS)
    {
        if (gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_ALPHABLEND_ATTR_BITS)
        {
            if (gc->state.enables.colorBuffer.blend[0] &&
                gc->state.raster.blendSrcRGB[0] == GL_ONE &&
                gc->state.raster.blendSrcAlpha[0] == GL_ONE &&
                gc->state.raster.blendDstRGB[0] == GL_ONE_MINUS_SRC_ALPHA &&
                gc->state.raster.blendDstAlpha[0] == GL_ONE_MINUS_SRC_ALPHA &&
                gc->state.raster.blendEquationRGB[0] == GL_FUNC_ADD&&
                gc->state.raster.blendEquationAlpha[0] == GL_FUNC_ADD)
             {
                 pgKeyState->staticKey->colorKill = gcvTRUE;
             }
             else
             {
                 pgKeyState->staticKey->colorKill = gcvFALSE;
             }
        }
    }


    if ((chipCtx->patchId == gcvPATCH_DEQP) &&
        (gc->apiVersion == __GL_API_VERSION_ES20) &&
        (gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_ALPHABLEND_ATTR_BITS) &&
        (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_HALF_FLOAT_PIPE) == gcvFALSE)
        )
    {
        if (gc->state.enables.colorBuffer.blend[0] &&
            (!gc->frameBuffer.drawFramebufObj->name) &&
            (program->progFlags.alphaBlend == 1) &&
            (gc->vertexArray.indexCount == 6))
        {
            if(pgKeyState->staticKey->alphaBlend != gcvTRUE)
            {
                pgKeyDirty = gcvTRUE;
            }
            pgKeyState->staticKey->alphaBlend = gcvTRUE;

        }
        else
        {
            if(pgKeyState->staticKey->alphaBlend != gcvFALSE)
            {
                pgKeyDirty = gcvTRUE;
            }
            pgKeyState->staticKey->alphaBlend = gcvFALSE;

        }
    }



    if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_1] & __GL_POLYGONOFFSET_FILL_ENDISABLE_BIT) &&
        (gcoHAL_IsFeatureAvailable(chipCtx->hal, gcvFEATURE_DEPTH_BIAS_FIX) == gcvFALSE))
    {
        if (gc->state.enables.polygon.polygonOffsetFill)
        {
            if(pgKeyState->staticKey->ShaderPolygonOffset != gcvTRUE)
            {
                pgKeyDirty = gcvTRUE;
            }
            pgKeyState->staticKey->ShaderPolygonOffset = gcvTRUE;

        }
        else
        {
            if(pgKeyState->staticKey->ShaderPolygonOffset != gcvFALSE)
            {
                pgKeyDirty = gcvTRUE;
            }
            pgKeyState->staticKey->ShaderPolygonOffset = gcvFALSE;

        }
    }


    status = pgKeyDirty ? gcvSTATUS_TRUE : gcvSTATUS_FALSE;

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateRecompileStateCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgramInstance*  oldPgInstance = program->curPgInstance;
    __GLchipSLProgramInstance*  newPgInstance = gcvNULL;
    __GLchipProgramStateKey    *pgStateKey = gcvNULL;
    __GLchipProgramStateKeyMask pgStateKeyMask;
    __GLchipUtilsObject*        pgInstanceObj;
    GLboolean                   pgInstanceChanged = GL_FALSE;
    gceSTATUS                   status = gcvSTATUS_OK;
    GLuint                      key;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);

    /*
    ** Evaluate key states
    ** Note key states are generally designed to record states that may affect recompile when
    ** states are validating, but since we are 1-stage validation, so we can not collect key
    ** states everywhere, just put it one place, here.
    */
    gcmONERROR(gcChipRecompileEvaluateKeyStates(gc, chipCtx, program));

    if (status == gcvSTATUS_TRUE)
    {
        GLint i;
        gcmONERROR(gcChipPgStateKeyAlloc(gc, &pgStateKey));
        /* Convert key states to state key to check whether we really need a recompile */
        if (gcChipNeedRecompile(gc, chipCtx, program, pgStateKey, &pgStateKeyMask))
        {
            /* Ok, we need an instance against master instance, so try to find an instance in the
            ** hash by pgStateKey, if no instance is found, then create a new one and call recompiler
            ** to do recompiling job to generate this new instance
            */
            key = gcChipUtilsEvaluateCRC32(pgStateKey->data, pgStateKey->size);

            pgInstanceObj = gcChipUtilsHashFindObjectByKey(gc, program->pgInstaceCache, key);
            if (pgInstanceObj == gcvNULL)
            {
                pgInstanceObj = gcChipAddPgInstanceToCache(gc, program, key, GL_FALSE);

                gcmASSERT(pgInstanceObj);
            }

            newPgInstance = (__GLchipSLProgramInstance*)pgInstanceObj->pUserData;

            if (newPgInstance != oldPgInstance)
            {
                program->curPgInstance = newPgInstance;

                /* Increase non-master instance's ref and historyRef */
                gcChipUtilsObjectAddRef(pgInstanceObj);

                if ((newPgInstance->programState.stateBuffer == gcvNULL || newPgInstance->programState.stateBufferSize == 0) &&
                    newPgInstance->pgStateKeyMask.value == 0)
                {
                    /* deep copy.*/
                    gcChipPgStateKeyCopy(gc, newPgInstance->pgStateKey, pgStateKey);
                    newPgInstance->pgStateKeyMask.value = pgStateKeyMask.value;

                    /* Recompile to generate new instance */
                    gcmONERROR(gcChipRecompileShader(gc, chipCtx, progObj, program));
                }

                  /* If program change, the sampler dirty is full, but when only instance change
                recompile could have more sampler, which we need dirty the
                mapDirty to trigger the validate texture.
                In __glBuildTexEnableDim, we don't get new pgInstance yet.
                */
                if (newPgInstance->pgStateKey->staticKey->alphaBlend)
                {
                    __glBitmaskSet(&gc->shaderProgram.samplerMapDirty,  newPgInstance->rtSampler);
                }

                pgInstanceChanged = GL_TRUE;
            }
        }
        else
        {
            /* Pick master instance as current one */
            newPgInstance = program->masterPgInstance;
            if (oldPgInstance != newPgInstance)
            {
                program->curPgInstance = newPgInstance;

                /* Increase ref count of master instance. Note that for master instance, we don't need
                   to increase historic ref count since it has been set to be perpetual */
                gcChipUtilsObjectAddRef(newPgInstance->ownerCacheObj);

                pgInstanceChanged = GL_TRUE;
            }
        }

        if (pgInstanceChanged)
        {
            gcePROGRAM_STAGE halStage = gcvPROGRAM_STAGE_VERTEX;
            gcePROGRAM_STAGE_BIT stageBits = program->stageBits;

            /* set dirty.*/
            chipCtx->chipDirty.uDefer.sDefer.pgInsChanged = 1;

            /* Decrease ref of replaced one */
            gcChipUtilsObjectReleaseRef(oldPgInstance->ownerCacheObj);

            /* Program instance may be built failed before, so we need retrieve it and let post-recompile
               to trivial check */
            if (newPgInstance->programState.stateBuffer == gcvNULL || newPgInstance->programState.stateBufferSize == 0)
            {
                progObj->programInfo.invalidFlags |= __GL_INVALID_LINK_BIT;
            }
            else
            {
                progObj->programInfo.invalidFlags &= ~__GL_INVALID_LINK_BIT;
            }

            /* A program instance switch must trigger program reload */
            while (stageBits)
            {
                if (stageBits & 1)
                {
                    switch (halStage)
                    {
                    case gcvPROGRAM_STAGE_VERTEX:
                        chipCtx->chipDirty.uDefer.sDefer.vsReload = 1;
                        if (chipCtx->patchId == gcvPATCH_REALRACING)
                        {
                            gc->vertexArray.varrayDirty = 0x3F;
                        }
                        break;
                    case gcvPROGRAM_STAGE_FRAGMENT:
                        chipCtx->chipDirty.uDefer.sDefer.fsReload = 1;
                        break;
                    case gcvPROGRAM_STAGE_COMPUTE:
                        chipCtx->chipDirty.uDefer.sDefer.csReload = 1;
                        break;
                    case gcvPROGRAM_STAGE_TCS:
                        chipCtx->chipDirty.uDefer.sDefer.tcsReload = 1;
                        break;
                    case gcvPROGRAM_STAGE_TES:
                        chipCtx->chipDirty.uDefer.sDefer.tesReload = 1;
                        break;
                    case gcvPROGRAM_STAGE_GEOMETRY:
                        chipCtx->chipDirty.uDefer.sDefer.gsReload = 1;
                        break;
                    default:
                        break;
                    }
                }
                stageBits >>= 1;
                ++halStage;
            }

            /* If new instance has RT patch, we have to dirty RT surface to validate RT again.
            ** This will kill performance, whenever shader switch with multi-layer RT, we have to validate RT.
            */
            if (program->curPgInstance->pgStateKeyMask.s.hasRtPatchFmt)
            {
                chipCtx->chipDirty.uBuffer.sBuffer.rtSurfDirty = 1;
            }

            /* If any of the const base was changed due to recompile, need to reflush all active uniforms. */
            if (newPgInstance->programState.hints->vsConstBase  != oldPgInstance->programState.hints->vsConstBase  ||
                newPgInstance->programState.hints->tcsConstBase != oldPgInstance->programState.hints->tcsConstBase ||
                newPgInstance->programState.hints->tesConstBase != oldPgInstance->programState.hints->tesConstBase ||
                newPgInstance->programState.hints->gsConstBase  != oldPgInstance->programState.hints->gsConstBase  ||
                newPgInstance->programState.hints->psConstBase  != oldPgInstance->programState.hints->psConstBase)
            {
                for (i = 0; i < program->activeUniformCount; ++i)
                {
                    program->uniforms[i].dirty = GL_TRUE;
                }
                chipCtx->chipDirty.uDefer.sDefer.activeUniform = 1;
            }

            /* Dirty all private uniforms */
            for (i = 0; i < newPgInstance->privateUniformCount; ++i)
            {
                newPgInstance->privateUniforms[i].dirty = GL_TRUE;
            }
        }
        if (newPgInstance->pgStateKeyMask.s.hasTexPatchFmt)
        {
            for (i = 0; i < (GLint)gc->constants.shaderCaps.maxTextureSamplers; i++)
            {
                __GLchipPgStateKeyTexPatchInfo *texPatchInfo = &newPgInstance->pgStateKey->texPatchInfo[i];
                if (texPatchInfo->format != gcvSURF_UNKNOWN)
                {
                    gcsSURF_FORMAT_INFO_PTR formatInfo;
                    gcoSURF_QueryFormat(texPatchInfo->format, &formatInfo);
                    if (formatInfo->layers > 1)
                    {
                        __glBitmaskSet(&gc->shaderProgram.samplerMapDirty, i);

                    }
                }
            }
        }
    }

     if (program->curPgInstance->pgStateKey->staticKey->alphaBlend &&
        chipCtx->rtTexture)
    {
        gcoSURF rt = chipCtx->drawRtViews[0].surf;
        gcoSURF tex = gcvNULL;
        gcsSURF_VIEW mipView;
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};
        gcmONERROR(gcoTEXTURE_GetMipMap(chipCtx->rtTexture, 0, &tex));

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted  = gcoSURF_QueryFlags(rt, gcvSURF_FLAG_CONTENT_YINVERTED);
        rlvArgs.uArgs.v2.rectSize.x = (gctINT)chipCtx->drawRTWidth;
        rlvArgs.uArgs.v2.rectSize.y = (gctINT)chipCtx->drawRTHeight;
        rlvArgs.uArgs.v2.numSlices  = 1;
        mipView.surf = tex;
        mipView.firstSlice = 0;
        mipView.numSlices = 1;

        /* Flush all cache in pipe */
        gcmONERROR(gcoSURF_Flush(rt));

        /* Sync texture surface from current RT */
        gcmONERROR(gcoSURF_ResolveRect(&chipCtx->drawRtViews[0], &mipView, &rlvArgs));

        gcmONERROR(gcoTEXTURE_Flush(chipCtx->rtTexture));

        gcmONERROR(gco3D_Semaphore(chipCtx->engine, gcvWHERE_RASTER, gcvWHERE_PIXEL, gcvHOW_SEMAPHORE));
    }

    /* This draw does not need recompile, but previous recompile really failed */
    if (progObj->programInfo.invalidFlags)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

OnError:

    if (pgStateKey)
    {
        gcmVERIFY_OK(gcChipPgStateKeyFree(gc, &pgStateKey));
    }

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipValidateProgramImagesCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLbitmask imageUnitDirty = gc->imageUnitDirtyMask;
    GLint unit = -1;
    __GLchipSLProgramInstance *pInstance = program->curPgInstance;

    gcmHEADER_ARG("gc=0x%x  progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);


    if (!(pInstance->extraImageUniformCount + program->imageUniformCount))
    {
        gcmFOOTER();
        return status;
    }

    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        __glBitmaskSetAll(&imageUnitDirty, gcvTRUE);
    }

    while (!__glBitmaskIsAllZero(&imageUnitDirty))
    {
        GLuint i = 0;
        __GLchipImageUnit2Uniform *pImageUnit2Uniform, *pExtraImageUnit2Uniform;
        __GLimageUnitState *imageUnit;

        if (!__glBitmaskTestAndClear(&imageUnitDirty, ++unit))
        {
            continue;
        }

        pImageUnit2Uniform = &program->imageUnit2Uniform[unit];
        pExtraImageUnit2Uniform = &pInstance->extraImageUnit2Uniform[unit];
        imageUnit = &gc->state.image.imageUnit[unit];

        while (i < (pImageUnit2Uniform->numUniform + pExtraImageUnit2Uniform->numUniform))
        {
            GLuint width = 0, height = 0;
            GLuint baseAddress = 0xdeadbeaf;
            gctINT stride = 0;
            GLuint imageInfo = 0;
            gctUINT *data;
            GLenum formatQualifier;
            GLuint type, arrayIndex;
            GLuint layerIndex = 0;
            gctINT32 layerSize = 0;
            __GLchipSLUniform *uniform;
                gcsSURF_VIEW texView = {gcvNULL, 0, 1};
            GLboolean valid = GL_FALSE;
            __GLtextureObject *texObj = imageUnit->texObj;

            if (i < pImageUnit2Uniform->numUniform)
            {
                uniform = pImageUnit2Uniform->uniforms[i].slUniform;
                formatQualifier = gcChipUtilConvertImageFormat(pImageUnit2Uniform->uniforms[i].formatQualifier);
                type = pImageUnit2Uniform->uniforms[i].type;
                arrayIndex = pImageUnit2Uniform->uniforms[i].arrayIndex;
                layerIndex = pImageUnit2Uniform->uniforms[i].auxiliary ? 1 : 0;
            }
            else
            {
                GLuint index = i - pImageUnit2Uniform->numUniform;
                uniform = pExtraImageUnit2Uniform->uniforms[index].slUniform;
                formatQualifier = gcChipUtilConvertImageFormat(pExtraImageUnit2Uniform->uniforms[index].formatQualifier);
                type = pExtraImageUnit2Uniform->uniforms[index].type;
                arrayIndex = pExtraImageUnit2Uniform->uniforms[index].arrayIndex;
                layerIndex = pExtraImageUnit2Uniform->uniforms[index].auxiliary ? 1 : 0;
            }

            do
            {
                __GLchipTextureInfo *texInfo;
                __GLchipMipmapInfo *chipMipmap;
                gceSURF_FORMAT format;
                gceTILING tiling;
                GLint face;
                GLint numLayers;

                /* Invalid access if bound texture is incomplete */
                if (imageUnit->invalid)
                {
                    break;
                }


                /* Invalid access if image unit format is different than qualifier format */
                if (imageUnit->format != formatQualifier)
                {
                    break;
                }

                /* Invalid access if shader image type mismatched with API specified */
                if (imageUnit->singleLayered ? ((type != __GL_IMAGE_2D) && (type != __GL_IMAGE_BUFFER)) : (type != imageUnit->type))
                {
                    break;
                }

                /* Invalid access if bound image level out of the range [baseLevel, maxLevel] */
                if (imageUnit->level < texObj->params.baseLevel || imageUnit->level > texObj->params.maxLevel)
                {
                    break;
                }

                numLayers = (__GL_IMAGE_3D == imageUnit->type) ? texObj->faceMipmap[0][0].depth : texObj->arrays;
                /* Invalid access if selected layer doesn't exist */
                if (imageUnit->singleLayered && (imageUnit->actualLayer >= numLayers))
                {
                    break;
                }

                /* Invalid access if image unit format was incompatible with tex internal format */
                face = (__GL_IMAGE_CUBE == imageUnit->type ? imageUnit->actualLayer : 0);
                if (imageUnit->formatInfo->bitsPerPixel != texObj->faceMipmap[face][imageUnit->level].formatInfo->bitsPerPixel)
                {
                    break;
                }

                if (texObj->bufObj)
                {
                    gcChipValidateProgramTexBufferUniform(gc, texObj, uniform, imageUnit);
                    valid = gcvTRUE;
                    break;
                }

                texInfo = (__GLchipTextureInfo *)texObj->privateData;
                if (texInfo->direct.source)
                {
                    gcmONERROR(gcChipTexSyncDirectVIV(gc, texObj));
                }
                else if (texInfo->eglImage.source)
                {
                    gcmONERROR(gcChipTexSyncEGLImage(gc, texObj, gcvFALSE));
                }
                else
                {
                    /* If the tex was drawn before and will be sampled in this draw, sync it back from the rtSurface */
                    gcmONERROR(gcChipTexSyncFromShadow(gc, unit, texObj));
                }

                chipMipmap = &texInfo->mipLevels[imageUnit->level];
                if (imageUnit->singleLayered)
                {
                    chipMipmap->shadow[imageUnit->actualLayer].masterDirty = GL_TRUE;
                }
                else
                {
                    GLint slice;
                    GLint numSlices = (texObj->targetIndex == __GL_TEXTURE_3D_INDEX)
                                    ? texObj->faceMipmap[face][imageUnit->level].depth
                                    : texObj->arrays;

                    for (slice = 0; slice < numSlices; ++slice)
                    {
                        chipMipmap->shadow[slice].masterDirty = GL_TRUE;
                    }
                }

                texView = gcChipGetTextureSurface(chipCtx, texObj, imageUnit->level, imageUnit->actualLayer);

                if (type != __GL_IMAGE_2D)
                {
                    texView.firstSlice = 0;
                }

                /* (todo) check if image is written inside shader.*/
                if (1)
                {
                    CHIP_TEX_IMAGE_UPTODATE(texInfo, imageUnit->level);
                }

                gcmONERROR(gcoSURF_DisableTileStatus(texView.surf, gcvTRUE));

                gcmONERROR(gcoTEXTURE_LockMipMap(texInfo->object, imageUnit->level, &baseAddress, gcvNULL));

                gcmONERROR(gcoSURF_GetSize(texView.surf, &width, &height, gcvNULL));
                gcmONERROR(gcoSURF_GetFormat(texView.surf, gcvNULL, &format));
                gcmONERROR(gcoSURF_GetAlignedSize(texView.surf, gcvNULL, gcvNULL, &stride));
                gcmONERROR(gcoSURF_GetTiling(texView.surf, &tiling));
                gcmONERROR(gcoSURF_GetInfo(texView.surf, gcvSURF_INFO_LAYERSIZE, &layerSize));
                switch (tiling)
                {
                case gcvLINEAR:
                    /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, LINEAR); */
                    imageInfo = 0;
                    break;
                case gcvTILED:
                    /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, TILED); */
                    imageInfo = 1 << 10;
                    break;
                case gcvSUPERTILED:
                     /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, SUPER_TILED); */
                    imageInfo = 2 << 10;
                    break;
                case gcvYMAJOR_SUPERTILED:
                     /* imageInfo = gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TILING, SUPER_TILED_YMAJOR); */
                    imageInfo = 3 << 10;
                    break;
                default:
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    break;
                }
                /*
                imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, TYPE, 2D);
                */
                imageInfo |= (1 << 12);

                switch (imageUnit->format)
                {
                case GL_RGBA32F:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2);
                    */
                    imageInfo |= 0x0 << 6 | 3 | 2 << 14;
                    break;
                case GL_R32F:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP32) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1);
                    */
                    imageInfo |= 0x0 << 6 | 2 | 1 << 14;
                    break;
                case GL_R32UI:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1);
                    */
                    imageInfo |= 0x5 << 6 | 2 | 1 << 14;
                    break;
                case GL_RGBA32UI:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U32)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2);
                    */
                    imageInfo |= 0x5 << 6 | 3 | 2 << 14;
                    break;
                case GL_R32I:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 1);
                    */
                    imageInfo |= 0x2 << 6 | 2 | 1 << 14;
                    break;
                case GL_RGBA32I:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S32)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 2);
                    */
                    imageInfo |= 0x2 << 6 | 3 | 2 << 14;
                    break;
                case GL_RGBA16F:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, FP16) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0x1 << 6 | 3 | 0 << 14;
                    break;
                case GL_RGBA16UI:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U16)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)         |
                                    gcmSETFIELDVALUE(), GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0x6 << 6 | 3 | 0 << 14;
                    break;
                case GL_RGBA16I:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S16)  |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 3)
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0x3 << 6 | 3 | 0 << 14;
                    break;
                case GL_RGBA8:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, UNORM8) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)           |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0xF << 6 | 2 | 0 << 14;
                    break;
                case GL_RGBA8_SNORM:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, SNORM8)|
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0xC << 6 | 2 | 0 << 14;
                    break;

                case GL_RGBA8UI:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, U8) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)       |
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                    */
                    imageInfo |= 0x7 << 6 | 2 | 0 << 14;
                    break;

                case GL_RGBA8I:
                    /* imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, CONVERSION, S8) |
                                    gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SHIFT, 2)
                                    gcmSETFILEDVALUE(0, GCREG_SH_IMAGE, COMPONENT_COUNT, 0);
                     */
                    imageInfo |= 0x4 << 6 | 2 | 0 << 14;
                    break;

                default:
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                    break;
                }

                switch (imageUnit->format)
                {
                case GL_RGBA32F:
                case GL_RGBA32UI:
                case GL_RGBA32I:
                    /*
                    imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, ZERO)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
                    */
                    imageInfo |= (0 << 16) | (1 << 20) | (4 << 24) | (5 << 28);

                    imageInfo |= 1 << 4;
                    break;

                case GL_RGBA16F:
                case GL_RGBA16UI:
                case GL_RGBA16I:
                case GL_RGBA8_SNORM:
                    /*
                    imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                    */
                    imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);

                    imageInfo |= 1 << 4;
                    break;

                case GL_RGBA8UI:
                case GL_RGBA8I:
                    switch (format)
                    {
                    case gcvSURF_A8B8G8R8I_1_A8R8G8B8:
                    case gcvSURF_A8B8G8R8UI_1_A8R8G8B8:
                        /*
                        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, Z)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, X)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                        */
                        imageInfo |= (2 << 16) | (1 << 20) | (0 << 24) | (3 << 28);
                        break;

                    default:
                        /*
                        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                        */
                        imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);
                        break;
                    }
                    imageInfo |= 1 << 4;
                    break;

                case GL_RGBA8:
                    switch (format)
                    {
                    case gcvSURF_A8R8G8B8:
                        /*
                        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, Z)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, X)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                        */
                        imageInfo |= (2 << 16) | (1 << 20) | (0 << 24) | (3 << 28);
                        break;

                    default:
                        /*
                        imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
                                     gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                        */
                        imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);
                        break;
                    }
                    imageInfo |= 1 << 4;
                    break;

                case GL_R32F:
                case GL_R32UI:
                case GL_R32I:
                    /*
                    imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, ZERO)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, ZERO)    |
                                 gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, ONE);
                    */
                    imageInfo |=  (0 << 16) | (4 << 20) | (4 << 24) | (5 << 28);

                    imageInfo |= 2 << 4;
                    break;
                }
                valid = GL_TRUE;
                data = (gctUINT*)((GLubyte*)uniform->data + arrayIndex * sizeof(gctFLOAT) * 4);
                data[0] = (gctUINT)(baseAddress + gcChipGetSurfOffset(&texView) + (layerSize*layerIndex));
                data[1] = (gctUINT)stride;
                data[2] = width | (height << 16);
                data[3] = imageInfo;
            } while (GL_FALSE);

            if (!valid)
            {
                /* Set default XYZW swizzle for invalid images */
                /*
                imageInfo |= gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_R, X)    |
                             gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_G, Y)    |
                             gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_B, Z)    |
                             gcmSETFIELDVALUE(0, GCREG_SH_IMAGE, SWIZZLE_A, W);
                */
                imageInfo |= (0 << 16) | (1 << 20) | (2 << 24) | (3 << 28);

                switch (formatQualifier)
                {
                case GL_RGBA32F:
                case GL_RGBA32UI:
                case GL_RGBA32I:
                case GL_RGBA16F:
                case GL_RGBA16UI:
                case GL_RGBA16I:
                case GL_RGBA8:
                case GL_RGBA8_SNORM:
                case GL_RGBA8UI:
                case GL_RGBA8I:
                    imageInfo |= 1 << 4;
                    break;

                case GL_R32F:
                case GL_R32UI:
                case GL_R32I:
                    imageInfo |= 2 << 4;
                    break;
                }

                width = height = stride = 0;
                baseAddress = 0xdeadbeaf;
                data = (gctUINT*)((GLubyte*)uniform->data + arrayIndex * sizeof(gctFLOAT) * 4);
                data[0] = (gctUINT)(baseAddress + gcChipGetSurfOffset(&texView) + (layerSize*layerIndex));
                data[1] = (gctUINT)stride;
                data[2] = width | (height << 16);
                data[3] = imageInfo;
                __GLES_PRINT("ES30: some image uniform attach to invalid image back-store");
            }

            /* Need to set user-defined uniforms dirty to toggle later flush */
            if (uniform->usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
            {
                chipCtx->chipDirty.uDefer.sDefer.activeUniform = 1;
            }
            uniform->dirty = GL_TRUE;

            i++;
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateImage(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmONERROR(gcChipTraverseProgramStages(gc, chipCtx, gcChipValidateProgramImagesCB));
OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipValidateState(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    gcmONERROR(gcChipValidateAttribGroup1(gc, chipCtx));

    gcmONERROR(gcChipValidateAttribGroup2(gc, chipCtx));

    gcmONERROR(gcChipValidateTexture(gc, chipCtx));

    gcmONERROR(gcChipValidateImage(gc, chipCtx));

    gcmONERROR(gcChipValidateShader(gc, chipCtx));

    gcmONERROR(gcChipValidateMiscState(gc, chipCtx));

OnError:
    gcmFOOTER();
    return status;
}
