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
#include "chip_context.h"

#define _GC_OBJ_ZONE    glvZONE_TEXTURE
extern gceSURF_FORMAT __glVIVDevFormatToHWFormat[];
/******************************************************************************\
***************************** Texture GL Name Arrays ***************************
\******************************************************************************/
/* Possible combine texture function sources. */
GLenum combineFunctionSourceNames[] =
{
    GL_CONSTANT,                /* glvCONSTANT */
    GL_PRIMARY_COLOR,           /* glvCOLOR */
    GL_PREVIOUS,                /* glvPREVIOUS */
    GL_TEXTURE,                 /* glvTEXTURE */
    GL_TEXTURE0,                /* glvTEXTURE0 */
    GL_TEXTURE1,                /* glvTEXTURE1 */
    GL_TEXTURE2,                /* glvTEXTURE2 */
    GL_TEXTURE3,                /* glvTEXTURE3 */
    GL_TEXTURE4,                /* glvTEXTURE4 */
    GL_TEXTURE5,                /* glvTEXTURE5 */
    GL_TEXTURE6,                /* glvTEXTURE6 */
    GL_TEXTURE7,                /* glvTEXTURE7 */
};

/* Possible combine texture function RGB operands. */
GLenum combineFunctionColorOperandNames[] =
{
    GL_SRC_ALPHA,               /* glvSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvSRCALPHAINV */
    GL_SRC_COLOR,               /* glvSRCCOLOR */
    GL_ONE_MINUS_SRC_COLOR,     /* glvSRCCOLORINV */
};

/* Possible combine texture function alpha operands. */
GLenum combineFunctionAlphaOperandNames[] =
{
    GL_SRC_ALPHA,               /* glvSRCALPHA */
    GL_ONE_MINUS_SRC_ALPHA,     /* glvSRCALPHAINV */
};

/* Possible texture coordinate generation modes. */
GLenum textureGenModes[] =
{
    GL_NORMAL_MAP,              /* glvTEXNORMAL */
    GL_REFLECTION_MAP,          /* glvREFLECTION */
    GL_OBJECT_LINEAR,           /* glvOBJECTLINEAR */
    GL_EYE_LINEAR,              /* glvEYELINEAR */
    GL_SPHERE_MAP,              /* glvSPHERE */
};

gceTEXTURE_TYPE __glChipTexTargetToHAL[] =
{
    gcvTEXTURE_1D,            /* __GL_TEXTURE_1D_INDEX */
    gcvTEXTURE_2D,            /* __GL_TEXTURE_2D_INDEX */
    gcvTEXTURE_3D,            /* __GL_TEXTURE_3D_INDEX */
    gcvTEXTURE_CUBEMAP,       /* __GL_TEXTURE_CUBEMAP_INDEX */
    gcvTEXTURE_2D,            /* __GL_TEXTURE_RECTANGLE_INDEX */
    gcvTEXTURE_1D_ARRAY,      /* __GL_TEXTURE_1D_ARRAY_INDEX */
    gcvTEXTURE_2D_ARRAY,      /* __GL_TEXTURE_2D_ARRAY_INDEX */
    gcvTEXTURE_UNKNOWN,       /* __GL_TEXTURE_BUFFER_INDEX */
    gcvTEXTURE_UNKNOWN,       /* __GL_MAX_TEXTURE_BINDINGS */
};


/******************************************************************************\
********************** Individual State Setting Functions **********************
\******************************************************************************/
GLboolean setCombineColorSource(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Source=0x%04x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Source, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineFunctionSourceNames,
        gcmCOUNTOF(combineFunctionSourceNames),
        Value, Type,
        &value
        );

    if (result)
    {
        gctUINT source = Source - GL_SRC0_RGB;

        switch (source)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombColorSource0, value, Sampler->index);
            Sampler->combColor.source[0] = (gleCOMBINESOURCE) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombColorSource1, value, Sampler->index);
            Sampler->combColor.source[1] = (gleCOMBINESOURCE) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombColorSource2, value, Sampler->index);
            Sampler->combColor.source[2] = (gleCOMBINESOURCE) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLboolean setCombineAlphaSource(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Source,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Source=0x%04x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Source, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineFunctionSourceNames,
        gcmCOUNTOF(combineFunctionSourceNames),
        Value, Type,
        &value
        );

    if (result)
    {
        gctUINT source = Source - GL_SRC0_ALPHA;

        switch (source)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombAlphaSource0, value, Sampler->index);
            Sampler->combAlpha.source[0] = (gleCOMBINESOURCE) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombAlphaSource1, value, Sampler->index);
            Sampler->combAlpha.source[1] = (gleCOMBINESOURCE) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombAlphaSource2, value, Sampler->index);
            Sampler->combAlpha.source[2] = (gleCOMBINESOURCE) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLboolean setCombineColorOperand(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Operand=0x%04x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Operand, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineFunctionColorOperandNames,
        gcmCOUNTOF(combineFunctionColorOperandNames),
        Value, Type,
        &value
        );

    if (result)
    {
        GLuint operand = Operand - GL_OPERAND0_RGB;

        switch (operand)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombColorOperand0, value, Sampler->index);
            Sampler->combColor.operand[0] = (gleCOMBINEOPERAND) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombColorOperand1, value, Sampler->index);
            Sampler->combColor.operand[1] = (gleCOMBINEOPERAND) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombColorOperand2, value, Sampler->index);
            Sampler->combColor.operand[2] = (gleCOMBINEOPERAND) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLboolean setCombineAlphaOperand(
    glsCHIPCONTEXT_PTR chipCtx,
    GLenum Operand,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLboolean result;
    GLuint value;
    gcmHEADER_ARG("Context=0x%x Operand=0x%04x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Operand, Sampler, Value, Type);

    result = glfConvertGLEnum(
        combineFunctionAlphaOperandNames,
        gcmCOUNTOF(combineFunctionAlphaOperandNames),
        Value, Type,
        &value
        );

    if (result)
    {
        GLuint operand = Operand - GL_OPERAND0_ALPHA;

        switch (operand)
        {
        case 0:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand0, value, Sampler->index);
            Sampler->combAlpha.operand[0] = (gleCOMBINEOPERAND) value;
            break;

        case 1:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand1, value, Sampler->index);
            Sampler->combAlpha.operand[1] = (gleCOMBINEOPERAND) value;
            break;

        case 2:
            glmSETHASH_2BITS(hashTextureCombAlphaOperand2, value, Sampler->index);
            Sampler->combAlpha.operand[2] = (gleCOMBINEOPERAND) value;
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", result);
    return result;
}

GLboolean setCurrentColor(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    glfSetVector4(
        &Sampler->constColor,
        Value,
        Type
        );

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

GLboolean setColorScale(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLfloat scale;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    scale = glfFloatFromRaw(Value, Type);

    if ((scale != 1.0f) && (scale != 2.0f) && (scale != 4.0f))
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    glfSetMutant(
        &Sampler->combColor.scale,
        Value,
        Type
        );

    glmSETHASH_1BIT(
        hashTexCombColorScaleOne,
        Sampler->combColor.scale.one,
        Sampler->index
        );

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

GLboolean setAlphaScale(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    )
{
    GLfloat scale;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    scale = glfFloatFromRaw(Value, Type);

    if ((scale != 1.0f) && (scale != 2.0f) && (scale != 4.0f))
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    glfSetMutant(
        &Sampler->combAlpha.scale,
        Value,
        Type
        );

    glmSETHASH_1BIT(
        hashTexCombAlphaScaleOne,
        Sampler->combAlpha.scale.one,
        Sampler->index
        );

    gcmFOOTER_ARG("return=%d", GL_TRUE);

    return GL_TRUE;
}

GLboolean setTexCoordGenMode(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type,
    GLuint index
    )
{
    GLboolean result;
    gcmHEADER_ARG("Context=0x%x Sampler=0x%x Value=0x%x Type=0x%04x",
                    chipCtx, Sampler, Value, Type);

    do
    {
        GLuint value;

        /* Convert the value. */
        result = glfConvertGLEnum(
            textureGenModes,
            gcmCOUNTOF(textureGenModes),
            Value, Type,
            &value
            );

        /* Error? */
        if (result)
        {
            /* Update the generation mode. */
            Sampler->genMode[index] = (gleTEXTUREGEN) value;

            /* Update the hash key. */
            switch (Sampler->index) {
                case 0:
                case 1:
                    glmSETHASH_3BITS(hashTexCoordGenMode0, value, Sampler->index * 12 + index * 3);
                    break;
                case 2:
                case 3:
                    glmSETHASH_3BITS(hashTexCoordGenMode1, value, (Sampler->index - 2) * 12 + index * 3);
                    break;
                case 4:
                case 5:
                    glmSETHASH_3BITS(hashTexCoordGenMode2, value, (Sampler->index - 4) * 12 + index * 3);
                    break;
                case 6:
                case 7:
                    glmSETHASH_3BITS(hashTexCoordGenMode3, value, (Sampler->index - 6) * 12 + index * 3);
                    break;
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER_ARG("return=%d", result);

    /* Return result. */
    return result;
}


/******************************************************************************\
*********************** Support Functions and Definitions **********************
\******************************************************************************/

#define gcSENTINELTEXTURE ( (gcoTEXTURE) ~0 )

typedef struct _glsCOMPRESSEDTEXTURE * glsCOMPRESSEDTEXTURE_PTR;
typedef struct _glsCOMPRESSEDTEXTURE
{
    GLint       bits;
    GLint       bytes;
    GLenum      format;
    GLenum      type;
}
glsCOMPRESSEDTEXTURE;

GLenum compressedTextures[] =
{
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};


gceENDIAN_HINT getEndianHint(
    GLenum Format,
    GLenum Type
    )
{
    gcmHEADER_ARG("Format=0x%04x Type=0x%04x", Format, Type);
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

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;

    default:
        gcmFOOTER_ARG("result=0x%04x", gcvENDIAN_NO_SWAP);
        return gcvENDIAN_NO_SWAP;
    }
}

/*******************************************************************************
**
**  _GetCompressedTextureDetails
**
**  Return Find the closest power of two to the specified value.
**
**  INPUT:
**
**      Name
**          Enumerated compressed paletted texture name.
**
**  OUTPUT:
**
**      Pointer to the format information.
*/

glsCOMPRESSEDTEXTURE_PTR getCompressedTextureDetails(
    GLenum Name
    )
{
    static glsCOMPRESSEDTEXTURE compressedTextureDetails[] =
    {
        { 4, 3, GL_RGB,  GL_UNSIGNED_BYTE },            /* GL_PALETTE4_RGB8_OES  */
        { 4, 4, GL_RGBA, GL_UNSIGNED_BYTE },            /* GL_PALETTE4_RGBA8_OES */
        { 4, 2, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 },     /* GL_PALETTE4_R5_G6_B5_OES */
        { 4, 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },   /* GL_PALETTE4_RGBA4_OES */
        { 4, 2, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },   /* GL_PALETTE4_RGB5_A1_OES */
        { 8, 3, GL_RGB,  GL_UNSIGNED_BYTE },            /* GL_PALETTE8_RGB8_OES */
        { 8, 4, GL_RGBA, GL_UNSIGNED_BYTE },            /* GL_PALETTE8_RGBA8_OES */
        { 8, 2, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5 },     /* GL_PALETTE8_R5_G6_B5_OES */
        { 8, 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },   /* GL_PALETTE8_RGBA4_OES */
        { 8, 2, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },   /* GL_PALETTE8_RGB5_A1_OES */
    };

    gctINT index;

    gcmHEADER_ARG("Name=0x%04x", Name);

    /* Determine the info index. */
    index = Name - GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

    /* Out of ranage? */
    if ((index < 0) || (index > (gctINT) gcmCOUNTOF(compressedTextureDetails)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }
    else
    {
        gcmFOOTER_ARG("return=0x%x", &compressedTextureDetails[index]);
        return &compressedTextureDetails[index];
    }
}


/*******************************************************************************
**
**  uploadTexture
**
**  Upload texture image to device local memory.
**
**  INPUT:
**
**      chipCtx
**          Pointer to the glsCHIPCONTEXT structure.
**
**      texObj
**          Pointer to the __GLtextureObject.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS uploadTexture(__GLcontext* gc)
{
    gceSTATUS status = gcvSTATUS_OK;
#ifdef __GL_DELAY_TEX_UPLOAD_
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLtextureObject* texObj;
    glsTEXTUREINFO * textureInfo;
    GLuint stageEnableMask;
    GLuint unit;
    GLuint          numFaces;
    GLuint          numLevels;
    GLuint face;
    GLuint level;

    gcmHEADER_ARG("Context=0x%x", gc);

    stageEnableMask = chipCtx->texture.stageEnabledMask;
    unit = 0;
    while (stageEnableMask)
    {
        if (stageEnableMask & 1)
        {
            texObj = gc->texture.units[unit].currentTexture;
            textureInfo = texObj->privateData;
            /* Construct texture object. */
            if (textureInfo)
            {
                /* Construct the gcoTEXTURE object. */
                if (textureInfo->object == gcvNULL) {
                    status = gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &textureInfo->object);
                    if (gcmIS_ERROR(status)) {
                        gcmFOOTER_ARG("result=%d", status);
                        return status;
                    }
                    gcoTEXTURE_SetEndianHint(textureInfo->object,
                            getEndianHint(texObj->faceMipmap[0][0].baseFormat, texObj->faceMipmap[0][0].type));
                }

                numLevels = texObj->maxLevelUsed + 1;
                if (CHIP_TEX_IMAGE_IS_ANY_LEVEL_DIRTY(textureInfo, texObj->params.baseLevel, numLevels))
                {
                    numFaces = texObj->arrays;
                    for (level = 0; level < numLevels; level++)
                    {
                        for (face = 0; face < numFaces; face++)
                        {
                            if (CHIP_TEX_IMAGE_IS_UPTODATE(textureInfo,level) == 0) {
                                /* Add the mipmap. If it already exists, the call will be ignored. */
                                status = gcoTEXTURE_AddMipMap(
                                    textureInfo->object,
                                    level,
                                    texObj->faceMipmap[face][level].requestedFormat,
                                    textureInfo->residentFormat,
                                    texObj->faceMipmap[face][level].width,
                                    texObj->faceMipmap[face][level].height,
                                    0,
                                    numFaces,
                                    gcvPOOL_DEFAULT,
                                    gcvNULL
                                     );

                                if (gcmIS_ERROR(status)) {
                                    break;
                                }

                                if (texObj->faceMipmap[face][level].compressed)
                                {
                                    status = gcoTEXTURE_UploadCompressed(textureInfo->object,
                                                           level,
                                                           face,
                                                           texObj->faceMipmap[face][level].width,
                                                           texObj->faceMipmap[face][level].height,
                                                           0,
                                                           NULL, /* TODO: should from texture host cache */
                                                          ((texObj->faceMipmap[face][level].width + 3) / 4) * ((texObj->faceMipmap[face][level].height + 3) / 4) * 8);
                                } else {
                                    status = gcoTEXTURE_Upload(textureInfo->object,
                                        level,
                                        face + 1,
                                        texObj->faceMipmap[face][level].width,
                                        texObj->faceMipmap[face][level].height,
                                        0,
                                        NULL, /* TODO: should from texture host cache */
                                        0, /* stride */
                                        textureInfo->imageFormat,
                                        gcvSURF_COLOR_SPACE_LINEAR);
                                }

                                if (gcmIS_ERROR(status)) {
                                    break;
                                }
                            }
                        }
                        if (gcmIS_ERROR(status)) {
                            break;
                        }
                        CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
                    }
                }
                textureInfo->residentLevels = numLevels;
                textureInfo->residentFaces = numFaces;
            }
        }
        stageEnableMask >>= 1;
        unit++;
    }

    gcmFOOTER_ARG("result=%d", status);
#endif
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  resetTextureWrapper
**
**  Delete objects associated with the wrapper.
**
**  INPUT:
**
**      chipCtx
**          Pointer to the glsCHIPCONTEXT structure.
**
**      TextureInfo
**          Pointer to the glsTEXTUREINFO.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS resetTextureWrapper(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTUREINFO * TextureInfo
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x Texture=0x%x", chipCtx, TextureInfo);

    do
    {
        /* Destroy the texture object. */
        if (TextureInfo->object != gcvNULL)
        {
            gcmERR_BREAK(gcoTEXTURE_Destroy(TextureInfo->object));
            TextureInfo->object = gcvNULL;
            if (!chipCtx->renderToTexture && TextureInfo->texRenderTarget)
            {
                gcmVERIFY_OK(gcoSURF_Destroy(TextureInfo->texRenderTarget));
                TextureInfo->texRenderTarget = gcvNULL;
            }
        }
    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  setTextureWrapperFormat
**
**  Set texture format and associated fields.
**
**  INPUT:
**
**      chipCtx
**          Pointer to the current context.
**
**      Texture
**          Pointer to the texture wrapper object.
**
**      Format
**          Specifies the number of color components in the texture. Must be
**          GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, or GL_LUMINANCE_ALPHA.
**
**  OUTPUT:
**
**      Nothing.
*/

void setTextureWrapperFormat(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTUREINFO * TextureInfo,
    GLenum Format
    )
{
    gcmHEADER_ARG("Context=0x%x Texture=0x%x Format=0x%04x", chipCtx, TextureInfo, Format);

    /* Set the format. */
    TextureInfo->format = Format;

    /* Set target enable for FS. */
    switch (Format)
    {
    case GL_ALPHA:
        TextureInfo->combineFlow.targetEnable = gcSL_ENABLE_W;
        TextureInfo->combineFlow.tempEnable   = gcSL_ENABLE_X;
        TextureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XXXX;
        TextureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_WWWW;
        break;

    case GL_LUMINANCE:
    case GL_RGB:
    case GL_DEPTH_COMPONENT:
        TextureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZ;
        TextureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZ;
        TextureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZZ;
        TextureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZZ;
        break;

    case GL_LUMINANCE_ALPHA:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case GL_INTENSITY:
        TextureInfo->combineFlow.targetEnable = gcSL_ENABLE_XYZW;
        TextureInfo->combineFlow.tempEnable   = gcSL_ENABLE_XYZW;
        TextureInfo->combineFlow.tempSwizzle  = gcSL_SWIZZLE_XYZW;
        TextureInfo->combineFlow.argSwizzle   = gcSL_SWIZZLE_XYZW;
        break;

    default:
        /* Invalid request. */
        gcmASSERT(gcvFALSE);
    }
    gcmFOOTER_NO();
    return;
}


/*******************************************************************************
**
**  updateStageEnable
**
**  Update stage enable state.
**
**  INPUT:
**
**      Context
**          Pointer to the glsCONTEXT structure.
**
**      Sampler
**          Pointer to the texture sampler descriptor.
**
**  OUTPUT:
**
**      Nothing.
*/

GLvoid updateStageEnable(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    GLboolean Enabled
    )
{
    gctUINT formatIndex;
    gctUINT index;

    gcmHEADER_ARG("Context=0x%x Sampler=0x%x", chipCtx, Sampler);

    if (Enabled) {
        chipCtx->texture.stageEnabledMask |= (1 << Sampler->index);
    } else {
        chipCtx->texture.stageEnabledMask &= ~(1 << Sampler->index);
    }

    /* Get the sampler index. */
    index = Sampler->index;

    if (Enabled)
    {
        /* Determine the format. */
        switch (Sampler->binding->format)
        {
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            formatIndex = Sampler->binding->format - GL_ALPHA;
            break;
        case GL_BGRA_EXT:
            formatIndex = 5;
            break;
        case GL_INTENSITY:
            formatIndex = 6;
            break;
        default:
            /* Invalid request. */
            gcmASSERT(gcvFALSE);
            gcmFOOTER_NO();
            return;
        }

        /* Update the hash key. */
        glmSETHASH_1BIT (hashStageEnabled,  Enabled, index);
        glmSETHASH_3BITS(hashTextureFormat, formatIndex,  index);
    }
    else
    {
        /* Update the hash key. */
        glmSETHASH_1BIT(hashStageEnabled,  Enabled, index);

        /* Set to an invalid format. */
        glmCLEARHASH_3BITS(hashTextureFormat, index);
    }
    gcmFOOTER_NO();
}

/*******************************************************************************
**
**  initializeTexture
**
**  Constructs texture management object.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
*/
extern GLboolean setTextureFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineAlphaFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );
extern GLboolean setCombineColorFunction(
    glsCHIPCONTEXT_PTR chipCtx,
    glsTEXTURESAMPLER_PTR Sampler,
    const GLvoid* Value,
    gleTYPE Type
    );

gceSTATUS initializeSampler(
    __GLcontext* gc,
    glsCHIPCONTEXT_PTR chipCtx
    )
{
    gceSTATUS status;
    __GLtextureEnvState *tes;
    __GLtextureObject* texObj;

    gcmHEADER_ARG("Context=0x%x", gc);

    do
    {
        GLuint samplerSize;
        GLint i;
        gctPOINTER pointer = gcvNULL;
        __GLTextureEnableState * es;
        __GLtextureUnitState *tex;

        /* Make sure we have samplers. */
        if (gc->constants.numberOfTextureUnits == 0)
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            break;
        }

        /* Enable DrawTex position stream. */
        chipCtx->attributeInfo[gldATTRIBUTE_DRAWTEX_POSITION].streamEnabled = GL_TRUE;

        chipCtx->hwPointSprite = gcvFALSE;

        /* Allocate and init texture sampler structures. */
        samplerSize = gc->constants.numberOfTextureUnits * gcmSIZEOF(glsTEXTURESAMPLER);
        gcmERR_BREAK(gcoOS_Allocate(
            gcvNULL,
            samplerSize,
            &pointer
            ));

        chipCtx->texture.sampler = pointer;

        /* Reset to zero. */
        gcoOS_ZeroMemory(chipCtx->texture.sampler, samplerSize);

        for (i = 0; i < glvMAX_TEXTURES; ++i)
        {
            gcoTEXTURE_InitParams(chipCtx->hal, &chipCtx->texture.halTexture[i]);
        }

        /* Init the samplers. */
        for (i = 0; i < gc->constants.numberOfTextureUnits; i++)
        {
            /* Get a shortcut to the current sampler. */
            glsTEXTURESAMPLER_PTR sampler = &chipCtx->texture.sampler[i];

            /* Set the index. */
            sampler->index = i;

            /* Initialize default bindings. */
            sampler->combColor.combineFlow = &sampler->colorDataFlow;
            sampler->combAlpha.combineFlow = &sampler->alphaDataFlow;

            /* Set defaults for alapha data flow. */
            sampler->alphaDataFlow.targetEnable = gcSL_ENABLE_W;
            sampler->alphaDataFlow.tempEnable   = gcSL_ENABLE_X;
            sampler->alphaDataFlow.tempSwizzle  = gcSL_SWIZZLE_XXXX;
            sampler->alphaDataFlow.argSwizzle   = gcSL_SWIZZLE_WWWW;

            /* Set default states. */
            sampler->coordReplace = GL_FALSE;
            tes = &gc->state.texture.texUnits[i].env;
            es = &gc->state.enables.texUnits[i];
            tex = &gc->state.texture.texUnits[i];
            setTextureFunction(chipCtx, sampler, &tes->mode, glvINT);
            setCurrentColor(chipCtx, sampler, &tes->color, glvFLOAT);
            setCombineAlphaFunction(chipCtx, sampler, &tes->function.alpha, glvINT);
            setCombineColorFunction(chipCtx, sampler, &tes->function.rgb, glvINT);
            setCombineColorSource(chipCtx, GL_SRC0_RGB, sampler, &tes->source[0].rgb, glvINT);
            setCombineColorSource(chipCtx, GL_SRC1_RGB, sampler, &tes->source[1].rgb, glvINT);
            setCombineColorSource(chipCtx, GL_SRC2_RGB, sampler, &tes->source[2].rgb, glvINT);
            setCombineAlphaSource(chipCtx, GL_SRC0_ALPHA, sampler, &tes->source[0].alpha, glvINT);
            setCombineAlphaSource(chipCtx, GL_SRC1_ALPHA, sampler, &tes->source[1].alpha, glvINT);
            setCombineAlphaSource(chipCtx, GL_SRC2_ALPHA, sampler, &tes->source[2].alpha, glvINT);
            setCombineColorOperand(chipCtx, GL_OPERAND0_RGB, sampler, &tes->operand[0].rgb, glvINT);
            setCombineColorOperand(chipCtx, GL_OPERAND1_RGB, sampler, &tes->operand[1].rgb, glvINT);
            setCombineColorOperand(chipCtx, GL_OPERAND2_RGB, sampler, &tes->operand[2].rgb, glvINT);
            setCombineAlphaOperand(chipCtx, GL_OPERAND0_ALPHA, sampler, &tes->operand[0].alpha, glvINT);
            setCombineAlphaOperand(chipCtx, GL_OPERAND1_ALPHA, sampler, &tes->operand[1].alpha, glvINT);
            setCombineAlphaOperand(chipCtx, GL_OPERAND2_ALPHA, sampler, &tes->operand[2].alpha, glvINT);
            setColorScale(chipCtx, sampler, &tes->rgbScale, glvFLOAT);
            setAlphaScale(chipCtx, sampler, &tes->alphaScale, glvFLOAT);

            /* right now if one of them is enabled, texture gen will be enabled. */
            sampler->genEnable = (es->texGen[0] || (es->texGen[1] << 1) || (es->texGen[2] << 2) || (es->texGen[3] << 3));
            /* Update the hash key. */
            glmSETHASH_4BITS(hashTexCoordGenEnable, sampler->genEnable, i);

            setTexCoordGenMode(chipCtx, sampler, &tex->s.mode, glvINT, 0);
            setTexCoordGenMode(chipCtx, sampler, &tex->t.mode, glvINT, 1);
            setTexCoordGenMode(chipCtx, sampler, &tex->r.mode, glvINT, 2);
            setTexCoordGenMode(chipCtx, sampler, &tex->q.mode, glvINT, 3);
        }

        /* Initialize default and proxy texture binding */
        for (i = 0; i < __GL_MAX_TEXTURE_BINDINGS; i++) {
            texObj = &gc->texture.defaultTextures[i];
            if (texObj->privateData == NULL)
            {
                glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)gc->imports.calloc(gc, 1, sizeof(glsTEXTUREINFO));
                if(textureInfo) {
                    texObj->privateData = textureInfo;
                }
            }
            texObj = &gc->texture.proxyTextures[i];
            if (texObj->privateData == NULL)
            {
                glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)gc->imports.calloc(gc, 1, sizeof(glsTEXTUREINFO));
                if(textureInfo) {
                    texObj->privateData = textureInfo;
                }
            }
        }
    }
    while (GL_FALSE);

    gcmFOOTER();
    /* Return result. */
    return status;
}

gceSTATUS deinitializeSampler(
    __GLcontext* gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLtextureObject* texObj;
    GLint i;

    gcmHEADER_ARG("Context=0x%x", gc);

    for (i = 0; i < __GL_MAX_TEXTURE_BINDINGS; i++) {

            texObj = &gc->texture.defaultTextures[i];
            if (texObj->privateData != NULL)
                gc->imports.free(gc, texObj->privateData);
            texObj->privateData = NULL;

            texObj = &gc->texture.proxyTextures[i];
            if (texObj->privateData == NULL)
                gc->imports.free(gc, texObj->privateData);
            texObj->privateData = NULL;

    }

    gcoOS_Free(chipCtx->os, chipCtx->texture.sampler);

    gcmFOOTER();
    /* Return result. */
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

gceSTATUS initializeTempBitmap(
    IN glsCHIPCONTEXT_PTR chipCtx,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcoSURF bitmap = gcvNULL;
    gcmHEADER_ARG("Context=0x%x Format=0x%04x Width=%u Height=%u",
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
            gcsSURF_FORMAT_INFO_PTR info[2];

            /* Is there a surface allocated? */
            if (chipCtx->tempBitmap != gcvNULL)
            {
                /* Unlock the surface. */
                if (chipCtx->tempBits != gcvNULL)
                {
                    gcmERR_BREAK(gcoSURF_Unlock(
                        chipCtx->tempBitmap, chipCtx->tempBits
                        ));

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
                gcmONERROR(gcoSURF_Construct(
                    chipCtx->hal,
                    width, height, 1,
                    gcvSURF_BITMAP, Format,
                    gcvPOOL_UNIFIED,
                    &bitmap
                    ));

                /* Get the pointer to the bits. */
                gcmONERROR(gcoSURF_Lock(
                    bitmap, gcvNULL, bits
                    ));

                /* Query the parameters back. */
                gcmONERROR(gcoSURF_GetAlignedSize(
                    bitmap, &width, &height, &stride
                    ));

                /* Query format specifics. */
                gcmONERROR(gcoSURF_QueryFormat(
                    Format, info
                    ));

                /* Set information. */
                chipCtx->tempBitmap       = bitmap;
                chipCtx->tempBits         = bits[0];
                chipCtx->tempFormat       = Format;
                chipCtx->tempBitsPerPixel = info[0]->bitsPerPixel;
                chipCtx->tempWidth        = width;
                chipCtx->tempHeight       = height;
                chipCtx->tempStride       = stride;
            }
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    while (gcvFALSE);

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

gceSTATUS resolveDrawToTempBitmap(
    IN glsCHIPCONTEXT_PTR chipCtx,
    IN gcoSURF srcSurf,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height
    )
{
    gceSTATUS status;
    gceSURF_FORMAT format;
    GLuint  drawRTWidth;
    GLuint  drawRTHeight;

    gcmHEADER_ARG("Context=0x%x SourceX=%d SourceY=%d Width=%d Height=%d",
                    chipCtx, SourceX, SourceY, Width, Height);

    do
    {
        gctUINT resX;
        gctUINT resY;
        gctUINT resW;
        gctUINT resH;

        gctUINT sourceX;
        gctUINT sourceY;

        gctINT left;
        gctINT top;
        gctINT right;
        gctINT bottom;

        gcsSURF_VIEW srcView = {srcSurf, 0, 1};
        gcsSURF_VIEW tmpView = {gcvNULL, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcoSURF_GetSize(srcSurf, &drawRTWidth, &drawRTHeight, gcvNULL);

        /* Clamp coordinates. */
        left   = gcmMAX(SourceX, 0);
        top    = gcmMAX(SourceY, 0);
        right  = gcmMIN(SourceX + Width,  (GLint) drawRTWidth);
        bottom = gcmMIN(SourceY + Height, (GLint) drawRTHeight);

        if ((right <= 0) || (bottom <= 0))
        {
            status = gcvSTATUS_INVALID_ARGUMENT;
            gcmFOOTER();
            return status;
        }

        gcmERR_BREAK(gcoSURF_GetResolveAlignment(srcSurf,
                                                 &resX,
                                                 &resY,
                                                 &resW,
                                                 &resH));

        /* Convert GL coordinates. */
        sourceX = left;
        sourceY = top;

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.numSlices  = 1;

        /* Determine the aligned source origin. */
        rlvArgs.uArgs.v2.srcOrigin.x = sourceX & ~(resX - 1);
        rlvArgs.uArgs.v2.srcOrigin.y = sourceY & ~(resY - 1);
        if ((rlvArgs.uArgs.v2.srcOrigin.x + (gctINT) resW > (GLint) drawRTWidth)
        &&  (rlvArgs.uArgs.v2.srcOrigin.x > 0)
        )
        {
            rlvArgs.uArgs.v2.srcOrigin.x = (chipCtx->drawRTWidth - resW) & ~(resX - 1);
        }

        /* Determine the origin adjustment. */
        chipCtx->tempX = sourceX - rlvArgs.uArgs.v2.srcOrigin.x;
        chipCtx->tempY = sourceY - rlvArgs.uArgs.v2.srcOrigin.y;

        /* Determine the aligned area size. */
        rlvArgs.uArgs.v2.rectSize.x = (gctUINT) gcmALIGN(right  - left + chipCtx->tempX, resW);
        rlvArgs.uArgs.v2.rectSize.y = (gctUINT) gcmALIGN(bottom - top  + chipCtx->tempY, resH);

        gcmVERIFY_OK(gcoSURF_GetFormat(srcSurf, gcvNULL, &format));

        /* Initialize the temporary surface. */
        gcmERR_BREAK(initializeTempBitmap(
            chipCtx,
            format,
            rlvArgs.uArgs.v2.rectSize.x,
            rlvArgs.uArgs.v2.rectSize.y
            ));

        tmpView.surf = chipCtx->tempBitmap;
        gcmERR_BREAK(gcoSURF_ResolveRect_v2(&srcView, &tmpView, &rlvArgs));

        /* Make sure the operation is complete. */
        gcmERR_BREAK(gcoHAL_Commit(chipCtx->hal, gcvTRUE));

        /* Compute the pointer to the last line. */
        chipCtx->tempLastLine
            =  chipCtx->tempBits
            +  chipCtx->tempStride *  chipCtx->tempY
            + (chipCtx->tempX      *  chipCtx->tempBitsPerPixel) / 8;

    }
    while (gcvFALSE);

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  bindTextureAndTextureState / unBindTextureAndTextureState
**
**  Texture state and binding/unbinding functions.
**
**  INPUT:
**
**      Context
**          Pointer to the glsCONTEXT structure.
**
**  OUTPUT:
**
**      Nothing.
*/

gceSTATUS bindTextureAndTextureState(
    __GLcontext* gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLuint stageEnabledMask;
    glsTEXTURESAMPLER_PTR sampler;
    glsTEXTUREINFO * textureInfo;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("Context=0x%x", chipCtx);

    do
    {
        gctINT i = 0;

        if (chipCtx->currGLSLProgram) {
            GLint i;
            GLint unit;
            GLProgram program = chipCtx->currGLSLProgram;
            stageEnabledMask = chipCtx->texture.stageEnabledMask;
            for (i = 0; i < gcmCOUNTOF(program->sampleMap) ; i++) {
                if (!( chipCtx->samplerDirty & ( 1 << i ) ) )
                                continue;
                chipCtx->samplerDirty &= (~(1 << i));
                unit = program->sampleMap[i].unit;
                if (stageEnabledMask & (1 << unit)) {
                    /* Get a shortcut to the current sampler. */
                    sampler = &chipCtx->texture.sampler[unit];

                    /* Make sure the stage is valid. */
                    gcmASSERT(sampler->binding != gcvNULL);
                    gcmASSERT(sampler->binding->object != gcvNULL);

                    /* Get a shortcut to the current sampler's bound texture. */
                    textureInfo = sampler->binding;
                    /* Flush texture cache. */
                    if (textureInfo->dirty)
                    {
                        gcmERR_BREAK(gcoTEXTURE_Flush(textureInfo->object));
                        textureInfo->dirty = gcvFALSE;
                    }
                    /* Bind to the sampler. */
                    if (textureInfo->object) {
                        if (textureInfo->renderDirty)
                        {
                            gcsSURF_VIEW rtView  = {textureInfo->texRenderTarget, 0, 1};
                            gcsSURF_VIEW texView = {gcvNULL, 0, 1};

                            gcmERR_BREAK(gcoTEXTURE_GetMipMap(textureInfo->object,
                                                              0,
                                                              &texView.surf));

                            gcmERR_BREAK(gcoSURF_ResolveRect_v2(&rtView, &texView, gcvNULL));

                            gcmERR_BREAK(gcoTEXTURE_Flush(textureInfo->object));

                            gcmERR_BREAK(gco3D_Semaphore(chipCtx->hw,
                                                          gcvWHERE_RASTER,
                                                          gcvWHERE_PIXEL,
                                                          gcvHOW_SEMAPHORE_STALL));

                            textureInfo->renderDirty = gcvFALSE;
                        }

                        if (chipCtx->hasTxDescriptor)
                        {
                            gcmERR_BREAK(gcoTEXTURE_BindTextureDesc(
                                textureInfo->object,
                                i,
                                &chipCtx->texture.halTexture[unit],
                                0
                                ));

                        }
                        else
                        {
                            gcmERR_BREAK(gcoTEXTURE_BindTexture(
                                textureInfo->object,
                                0,
                                i,
                                &chipCtx->texture.halTexture[unit]
                                ));
                        }
                    }
                }
            }
        } else {
            if (chipCtx->currProgram) {
                /* Make a shortcut to the texture attribute array. */
                glsUNIFORMWRAP_PTR* attrTexture = chipCtx->currProgram->fs.texture;

                if (chipCtx->hashKey.accumMode != gccACCUM_UNKNOWN)
                {
                    stageEnabledMask = 3;
                }
                else
                {
                    stageEnabledMask = chipCtx->texture.stageEnabledMask;
                    if (chipCtx->hashKey.hasPolygonStippleEnabled) {
                        stageEnabledMask = (1 << chipCtx->polygonStippleTextureStage);
                    }
                    if (chipCtx->hashKey.hasLineStippleEnabled) {
                        stageEnabledMask = (1 << chipCtx->lineStippleTextureStage);
                    }
                }
                /* Iterate though the attributes. */
                while ((i < glvMAX_TEXTURES) && stageEnabledMask)
                {
                    gctUINT samplerNumber;
                    gctUINT samplerBase;

                    /* Skip if the texture is not used. */
                    if (((stageEnabledMask & 1) && (attrTexture[i] == gcvNULL)) || !(stageEnabledMask & 1))
                    {
                        i++;
                        stageEnabledMask >>= 1;
                        continue;
                    }

                    /* Get a shortcut to the current sampler. */
                    sampler = &chipCtx->texture.sampler[i];

                    /* Make sure the stage is valid. */
                    gcmASSERT(sampler->binding != gcvNULL);
                    gcmASSERT(sampler->binding->object != gcvNULL);

                    /* Get a shortcut to the current sampler's bound texture. */
                    textureInfo = sampler->binding;

                    /* Flush texture cache. */
                    if (textureInfo->dirty)
                    {
                        gcmERR_BREAK(gcoTEXTURE_Flush(textureInfo->object));
                        textureInfo->dirty = gcvFALSE;
                    }

                    samplerBase = GetShaderSamplerBaseOffset(chipCtx->currProgram->fs.shader);

                    /* Get the sampler number. */
                    gcmERR_BREAK(gcUNIFORM_GetSampler(
                        attrTexture[i]->uniform,
                        &samplerNumber
                        ));

                    samplerNumber += samplerBase;

                    if (i == chipCtx->polygonStippleTextureStage) {
                        if (chipCtx->hasTxDescriptor)
                        {
                            gcmERR_BREAK(gcoTEXTURE_BindTextureDesc(
                                textureInfo->object,
                                samplerNumber,
                                &chipCtx->polygonStippleTexture,
                                0
                               ));
                        }
                        else
                        {
                            gcmERR_BREAK(gcoTEXTURE_BindTexture(
                                textureInfo->object,
                                0,
                                samplerNumber,
                                &chipCtx->polygonStippleTexture
                               ));
                        }
                    } else {
                        if (i == chipCtx->lineStippleTextureStage) {
                            if (chipCtx->hasTxDescriptor)
                            {
                                gcmERR_BREAK(gcoTEXTURE_BindTextureDesc(
                                    textureInfo->object,
                                    samplerNumber,
                                    &chipCtx->lineStippleTexture,
                                    0
                                    ));

                            }
                            else
                            {
                                gcmERR_BREAK(gcoTEXTURE_BindTexture(
                                    textureInfo->object,
                                    0,
                                    samplerNumber,
                                    &chipCtx->lineStippleTexture
                                    ));
                            }
                        } else {
                             if (textureInfo->renderDirty)
                             {
                                gcsSURF_VIEW rtView  = {textureInfo->texRenderTarget, 0, 1};
                                gcsSURF_VIEW texView = {gcvNULL, 0, 1};

                                 gcmERR_BREAK(gcoTEXTURE_GetMipMap(textureInfo->object,
                                                                   0,
                                                                   &texView.surf));

                                 gcmERR_BREAK(gcoSURF_ResolveRect_v2(&rtView, &texView, gcvNULL));

                                 gcmERR_BREAK(gcoTEXTURE_Flush(textureInfo->object));

                                 gcmERR_BREAK(gco3D_Semaphore(chipCtx->hw,
                                                              gcvWHERE_RASTER,
                                                              gcvWHERE_PIXEL,
                                                              gcvHOW_SEMAPHORE_STALL));

                                 textureInfo->renderDirty = gcvFALSE;
                             }

                             if (chipCtx->hasTxDescriptor)
                             {
                                 /* Bind to the sampler. */
                                 gcmERR_BREAK(gcoTEXTURE_BindTextureDesc(
                                     textureInfo->object,
                                     samplerNumber,
                                     &chipCtx->texture.halTexture[i],
                                     0
                                    ));
                             }
                             else
                             {
                                 /* Bind to the sampler. */
                                 gcmERR_BREAK(gcoTEXTURE_BindTexture(
                                     textureInfo->object,
                                     0,
                                     samplerNumber,
                                     &chipCtx->texture.halTexture[i]
                                    ));
                             }
                        }
                    }
                    i++;
                    stageEnabledMask >>= 1;
                }
            }
        }
    }
    while (GL_FALSE);

    gcmFOOTER();

    /* Return status. */
    return status;
}

gceSTATUS unBindTextureAndTextureState(
    __GLcontext* gc
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLint i = 0;
    GLuint stageEnabledMask;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    /* Make a shortcut to the texture attribute array. */
    glsUNIFORMWRAP_PTR* attrTexture = chipCtx->currProgram->fs.texture;

    gcmHEADER_ARG("Context=0x%x", chipCtx);

    stageEnabledMask = chipCtx->texture.stageEnabledMask;
    /* Iterate though the attributes. */
    while ((i < glvMAX_TEXTURES) && stageEnabledMask)
    {
        /* Skip if the texture is not used. */
        if ((stageEnabledMask & 1) && (attrTexture[i] != gcvNULL))
        {
            glsTEXTURESAMPLER_PTR sampler;
            glsTEXTUREINFO * texture;

            /* Get a shortcut to the current sampler. */
            sampler = &chipCtx->texture.sampler[i];

            /* Make sure the stage is valid. */
            gcmASSERT(sampler->binding != gcvNULL);
            gcmASSERT(sampler->binding->object != gcvNULL);

            /* Get a shortcut to the current sampler's bound texture. */
            texture = sampler->binding;

            /* Unbind the texture. */
            gcmERR_BREAK(gcoTEXTURE_BindTexture(texture->object, 0, -1, gcvNULL));
        }
        i++;
        stageEnabledMask >>= 1;
    }

    gcmFOOTER();
    /* Return status. */
    return status;
}


/*******************************************************************************
**
**  updateTextureStates
**
**  Update texture states.
**
**  INPUT:
**
**      Context
**          Pointer to the current context.
**
**  OUTPUT:
**
**      Nothing.
**
*/

gceSTATUS updateTextureStates(
    __GLcontext* gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean coordReplace = GL_FALSE;
    GLint hashComponents;
    GLint lastAvailableSlot = -1;
    GLint i;

    gcmHEADER_ARG("Context=0x%x", gc);

    for (i = 0; i < gc->constants.numberOfTextureUnits; i++)
    {
        glsTEXTURESAMPLER_PTR sampler = &chipCtx->texture.sampler[i];

        /* Optimization. */
        if (!(chipCtx->texture.stageEnabledMask & (1 << i)))
        {
            glmCLEARHASH_2BITS(
                hashTexCoordComponentCount,
                i
                );
            if (lastAvailableSlot == -1) {
                lastAvailableSlot = i;
            }
            continue;
        }

        lastAvailableSlot = -1;

        /* Our hardware currently has only one global coordinate replacement
           state (not per unit as specified in OpenGL ES spec). Up until now
           it has not been a problem. Here we determine whether the state
           has been turned on for any of the enabled samplers and OR them
           into one state for later analysis. */
        if (chipCtx->pointStates.spriteDirty)
        {
            coordReplace |= sampler->coordReplace;
        }

        /* Determine the number of components in streamed texture coordinate. */
        if (chipCtx->drawTexOESEnabled)
        {
            /* Always 2 components for DrawTex extension. */
            sampler->coordType    = gcSHADER_FLOAT_X2;
            sampler->coordSwizzle = gcSL_SWIZZLE_XYYY;

            /* Set hash key component count. */
            hashComponents = 2;
        }
        else if (chipCtx->attributeInfo[i + __GL_INPUT_TEX0_INDEX].streamEnabled && !sampler->genEnable)
        {
            /* Copy stream component count. */
            sampler->coordType    = chipCtx->attributeInfo[i + __GL_INPUT_TEX0_INDEX].varyingType;
            sampler->coordSwizzle = chipCtx->attributeInfo[i + __GL_INPUT_TEX0_INDEX].varyingSwizzle;

            /* Set hash key component count. */
            hashComponents = chipCtx->attributeInfo[i + __GL_INPUT_TEX0_INDEX].components;
        }
        else
        {
            /* Constant texture coordinate allways has 4 components. */
            sampler->coordType    = gcSHADER_FLOAT_X4;
            sampler->coordSwizzle = gcSL_SWIZZLE_XYZW;

            /* Set hash key component count. */
            hashComponents = 4;
        }

        /* Update the hash key; only values in 2..4 range are possible,
           use the count reduced by 2 to save (1 * 4) = 4 hash bits. */
        glmSETHASH_2BITS(
            hashTexCoordComponentCount,
            (hashComponents - 2),
            i
            );
    }

    /* Update point sprite state. */
    if (chipCtx->pointStates.spriteDirty)
    {
        chipCtx->pointStates.spriteActive
            =  coordReplace
            && chipCtx->pointStates.pointPrimitive
            && chipCtx->pointStates.spriteEnable;

        if (chipCtx->hwPointSprite != chipCtx->pointStates.spriteActive)
        {
            status = gco3D_SetPointSprite(
                chipCtx->hw,
                chipCtx->hwPointSprite = chipCtx->pointStates.spriteActive
                );
        }

        chipCtx->pointStates.spriteDirty = GL_FALSE;
    }

    if ((chipCtx->hashKey.hasPolygonStippleEnabled) && (lastAvailableSlot != -1)) {
        chipCtx->texture.sampler[lastAvailableSlot] = chipCtx->polygonStippleSampler;
        chipCtx->polygonStippleTextureStage = lastAvailableSlot++;
    }

    if ((chipCtx->hashKey.hasLineStippleEnabled) && (lastAvailableSlot != -1)) {
        chipCtx->texture.sampler[lastAvailableSlot] = chipCtx->lineStippleSampler;
        chipCtx->lineStippleTextureStage = lastAvailableSlot;
    }
    gcmFOOTER();
    /* Return status. */
    return status;
}

static GLenum
getImageFormat(
    GLenum format,
    GLenum type,
    gceSURF_FORMAT* ImageFormat,
    GLsizei *Bpp
    )
{
    gceSURF_FORMAT imageFormat = gcvSURF_UNKNOWN;
    GLsizei bpp = 0;

    /* Check for a valid type. */
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_24_8_EXT:
        case GL_HALF_FLOAT_ARB:
        case GL_FLOAT:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            break;
        default:
            return GL_INVALID_ENUM;
    }

    switch (format)
    {
    case GL_DEPTH_COMPONENT:
        switch (type)
        {
        case GL_UNSIGNED_SHORT:
            bpp = 16;
            imageFormat = gcvSURF_D16;
            break;

        case GL_DEPTH_COMPONENT24:
            /* Some applications use DEPTH_COMPONENT24_OES,
               even though it's not part of the spec. */
            bpp = 32;
            imageFormat = gcvSURF_D24X8;
            break;

        case GL_DEPTH_COMPONENT32:
            /* Fall through */
        case GL_UNSIGNED_INT:
            bpp = 32;
            imageFormat = gcvSURF_D32;
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

        case GL_HALF_FLOAT_ARB:
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

        case GL_UNSIGNED_INT_2_10_10_10_REV:
            bpp = 32;
            imageFormat = gcvSURF_X2B10G10R10;
            break;

        case GL_HALF_FLOAT_ARB:
            bpp = 64;
            imageFormat = gcvSURF_X16B16G16R16F;
            break;

        case GL_UNSIGNED_SHORT:
            bpp = 48;
            imageFormat = gcvSURF_B16G16R16;
            break;

        case GL_FLOAT:
            bpp = 96;
            imageFormat = gcvSURF_B32G32R32F;
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

        case GL_UNSIGNED_INT_8_8_8_8:
            bpp = 32;
            imageFormat = gcvSURF_R8G8B8A8;
            break;
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8B8G8R8;
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

        case GL_FLOAT:
            bpp = 128;
            imageFormat = gcvSURF_A32B32G32R32F;
            break;

        case GL_HALF_FLOAT_ARB:
            bpp = 64;
            imageFormat = gcvSURF_A16B16G16R16F;
            break;
        }
        break;

    case GL_RED:
        switch (type)
        {
            case GL_FLOAT:
                bpp = 32;
                imageFormat = gcvSURF_R32F;
                break;
            case GL_UNSIGNED_BYTE:
                bpp = 8;
                imageFormat = gcvSURF_L8;
                break;
           default : ;
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

        case GL_HALF_FLOAT_ARB:
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

        case GL_HALF_FLOAT_ARB:
            bpp = 32;
            imageFormat = gcvSURF_A16L16F;
            break;

        case GL_FLOAT:
            bpp = 64;
            imageFormat = gcvSURF_A32L32F;
            break;
        }
        break;

    case GL_BGRA_EXT:
        switch (type)
        {
        case GL_UNSIGNED_INT_8_8_8_8:
            bpp = 32;
            imageFormat = gcvSURF_B8G8R8A8;
            break;
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_A8R8G8B8;
            break;
        }
        break;

    case GL_BGR_EXT:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 24;
            imageFormat = gcvSURF_R8G8B8;
            break;
        }
        break;
    case GL_ABGR_EXT:
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            bpp = 32;
            imageFormat = gcvSURF_R8G8B8A8;
            break;
        }
        break;
    case GL_DEPTH_STENCIL_EXT:
        switch (type)
        {
        case GL_UNSIGNED_INT_24_8_EXT:
            bpp = 32;
            imageFormat = gcvSURF_D24S8;
            break;
        }
        break;

    default:
        gcmBREAK();
        return GL_INVALID_ENUM;
    }

    /* Did we find a valid Format-Type combination?. */
    if (imageFormat == gcvSURF_UNKNOWN)
    {
        return GL_INVALID_OPERATION;
    }

    if (ImageFormat)
    {
        *ImageFormat = imageFormat;
    }

    if (Bpp)
    {
        *Bpp = bpp;
    }

    /* Success. */
    return GL_NO_ERROR;
}

GLvoid buildTextureInfo(glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj, GLint face, GLint level)
{
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

    gcmHEADER_ARG("chipCtx=0x%x", chipCtx);

    if (textureInfo != NULL) {
        if (level == 0) {
            setTextureWrapperFormat(chipCtx, (glsTEXTUREINFO *)texObj->privateData,
                texObj->faceMipmap[0][0].baseFormat);
            textureInfo->residentFormat =
                __glVIVDevFormatToHWFormat[texObj->faceMipmap[0][0].deviceFormat->devfmt];
        }
    }
    gcmFOOTER_ARG("result=%d", GL_TRUE);
}

#define ALLOC_TARGET(size,pointer) \
    do { \
        status = gcoOS_Allocate(gcvNULL,    \
            (size),    \
            (gctPOINTER)&pointer);    \
        if ( status < 0)    \
        {   \
                pointer = NULL;    \
                goto OnError;    \
        } \
    } while ( 0 )


static void cvtL32DataToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLfloat *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint j;
    GLubyte c;

    pfbuf = (GLfloat *)(psource);
    switch (internalformt) {
        case gcvSURF_L8:
            ALLOC_TARGET(width*height*sizeof(GLubyte),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                pbuf[j] = (GLubyte)(pfbuf[j] * 255.0);
            }
            *imgformt = gcvSURF_L8;
            break;
        case gcvSURF_A8B8G8R8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                c = (GLubyte)(pfbuf[j] * 255.0);
                *((GLuint *)pbuf) = c | (c<<8) | (c<<16) | (0xFF <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8B8G8R8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvtRGB32FDataToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLfloat *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte r, g, b;

    pfbuf = (GLfloat *)(psource);
    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                r = (GLubyte)(pfbuf[j*3] * 255.0);
                g = (GLubyte)(pfbuf[j*3 + 1] * 255.0);
                b = (GLubyte)(pfbuf[j*3 + 2] * 255.0);
                *((GLuint *)pbuf) = b | (g<<8) | (r<<16) | (0xFF <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvtRGBA32FDataToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLfloat *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte r, g, b,a;

    pfbuf = (GLfloat *)(psource);
    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                r = (GLubyte)(pfbuf[j*4 + 0] * 255.0);
                g = (GLubyte)(pfbuf[j*4 + 1] * 255.0);
                b = (GLubyte)(pfbuf[j*4 + 2] * 255.0);
                a = (GLubyte)(pfbuf[j*4 + 3] * 255.0);
                *((GLuint *)pbuf) = b | (g<<8) | (r<<16) | (a <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}


static void cvtRGB32ABGRToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLubyte *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte r, g, b,a;

    pfbuf = (GLubyte *)(psource);

    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                a = (GLubyte)(pfbuf[j*4]);
                b = (GLubyte)(pfbuf[j*4 + 1]);
                g = (GLubyte)(pfbuf[j*4 + 2]);
                r = (GLubyte)(pfbuf[j*4 + 3]);
                *((GLuint *)pbuf) = b | (g<<8) | (r<<16) | (a <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        case gcvSURF_A8B8G8R8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                a = (GLubyte)(pfbuf[j*4]);
                b = (GLubyte)(pfbuf[j*4 + 1]);
                g = (GLubyte)(pfbuf[j*4 + 2]);
                r = (GLubyte)(pfbuf[j*4 + 3]);
                *((GLuint *)pbuf) = r | (g<<8) | (b<<16) | (a <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8B8G8R8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvtRGB32ARGBToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLubyte *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte r, g, b,a;

    pfbuf = (GLubyte *)(psource);

    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                a = (GLubyte)(pfbuf[j*4]);
                r = (GLubyte)(pfbuf[j*4 + 1]);
                g = (GLubyte)(pfbuf[j*4 + 2]);
                b = (GLubyte)(pfbuf[j*4 + 3]);
                *((GLuint *)pbuf) = b | (g<<8) | (r<<16) | (a <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        case gcvSURF_A8B8G8R8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                a = (GLubyte)(pfbuf[j*4]);
                r = (GLubyte)(pfbuf[j*4 + 1]);
                g = (GLubyte)(pfbuf[j*4 + 2]);
                b = (GLubyte)(pfbuf[j*4 + 3]);
                *((GLuint *)pbuf) = r | (g<<8) | (b<<16) | (a <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8B8G8R8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvRGB32BGRToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLubyte *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte r, g, b;

    pfbuf = (GLubyte *)(psource);

    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                b = (GLubyte)(pfbuf[j*3]);
                g = (GLubyte)(pfbuf[j*3 + 1]);
                r = (GLubyte)(pfbuf[j*3 + 2]);
                *((GLuint *)pbuf) = b | (g<<8) | (r<<16) | (0xFF <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvtL32FToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLfloat *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte c;

    pfbuf = (GLfloat *)(psource);

    switch (internalformt) {
        case gcvSURF_L8:
            ALLOC_TARGET(width*height*sizeof(GLubyte),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                *pbuf = (GLubyte)(pfbuf[j]*255);
                pbuf++;
            }
            *imgformt = gcvSURF_L8;
            break;
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                c = (GLubyte)(pfbuf[j] * 255.0);
                *((GLuint *)pbuf) = c | c << 8 | c << 16 | (0xFF <<24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static void cvtL8ToTarget(GLsizei width, GLsizei height, gctPOINTER *ptarget, gctPOINTER psource, gceSURF_FORMAT internalformt, gceSURF_FORMAT *imgformt)
{
    GLubyte *pbuf = NULL;
    GLubyte *pobuf = NULL;
    GLubyte *pfbuf = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    GLuint  j;
    GLubyte c;

    pfbuf = (GLubyte *)(psource);

    switch (internalformt) {
        case gcvSURF_A8R8G8B8:
            ALLOC_TARGET(width*height*sizeof(GLuint),pbuf);
            pobuf = pbuf;
            for ( j = 0; j < height * width; j++ ) {
                c = (GLubyte)(pfbuf[j]);
                *((GLuint *)pbuf) = c | (c << 8) | (c << 16) | (c << 24);
                pbuf += 4;
            }
            *imgformt = gcvSURF_A8R8G8B8;
            break;
        default :
            break;
    }

OnError:
    *ptarget = (gctPOINTER)pobuf;
}

static GLvoid *convertToInternalTex(
                                __GLmipMapLevel *mipmap, gceSURF_FORMAT internalformt,
                                GLsizei width,
                                GLsizei height,
                                gceSURF_FORMAT *imgformt,
                                GLvoid **buf)
{

    GLubyte *pbuf = NULL;

    if ( mipmap->compressed )
       return NULL;

    switch (*imgformt)
    {
        case gcvSURF_R32F:
            if ( internalformt == gcvSURF_L8 || internalformt == gcvSURF_A8B8G8R8)
                cvtL32DataToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break ;
        case gcvSURF_B32G32R32F:
            cvtRGB32FDataToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_R8G8B8A8:
            cvtRGB32ABGRToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_B8G8R8A8:
            cvtRGB32ARGBToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_R8G8B8:
            cvRGB32BGRToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_L32F:
            cvtL32FToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_A32B32G32R32F:
            cvtRGBA32FDataToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        case gcvSURF_L8:
            cvtL8ToTarget(width, height, (gctPOINTER)&pbuf, (gctPOINTER)(*buf), internalformt, imgformt);
            break;
        default:
            break ;
    }

    if ( pbuf )
        *buf = (gctPOINTER)pbuf;
    return (GLvoid *)pbuf;
}

static void removeTempCVTData(gctPOINTER pbuf) {
    if ( pbuf )
        gcoOS_Free(NULL, pbuf);
}

static GLvoid
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


static GLvoid
gcChipProcessPixelStore(
    __GLpixelUnpackMode *packMode,
    gctSIZE_T width,
    gctSIZE_T height,
    GLenum format,
    GLenum type,
    gctSIZE_T skipImgs,
    gctSIZE_T *pRowStride,
    gctSIZE_T *pImgHeight,
    const GLvoid** pBuf
    )
{

    gctSIZE_T bpp = 0;
    gctSIZE_T rowStride = 0;
    gctSIZE_T imgStride = 0;
    gctSIZE_T imgLength = packMode->lineLength ? (gctSIZE_T)packMode->lineLength : width;
    gctSIZE_T imgHeight = packMode->imageHeight ? (gctSIZE_T)packMode->imageHeight : height;
    const GLbyte* buf = *pBuf;


    gcmHEADER_ARG("packMode=0x%x width=%u height=%u format=0x%04x type=0x%04x "
                  "skipImgs=%u pRowStride=0x%x pImgHeight=0x%x pBuf=0x%x",
                  packMode, width, height, format, type, skipImgs,
                  pRowStride, pImgHeight, pBuf);


    /* pixel store unpack parameters */
    gcChipUtilGetImageFormat(format, type, gcvNULL, &bpp);

    rowStride = gcmALIGN(bpp * imgLength / 8, packMode->alignment);
    imgStride = rowStride * imgHeight;
    *pBuf = (GLbyte*)buf
          + skipImgs * imgStride                /* skip images */
          + packMode->skipLines * rowStride     /* skip lines */
          + packMode->skipPixels * bpp / 8;     /* skip pixels */

    if (pRowStride)
    {
        *pRowStride = rowStride;
    }

    if (pImgHeight)
    {
        *pImgHeight = imgHeight;
    }
    gcmFOOTER_NO();
}

GLvoid residentTextureLevel(__GLclientPixelState *ps, glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj, GLint face, GLint level, const GLvoid* buf)
{
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;
    gctPOINTER pbuf;

    /* upload texture */
    if (textureInfo != NULL) {
        gceSTATUS status = gcvSTATUS_OK;
        __GLmipMapLevel *mipmap = &texObj->faceMipmap[face][level];

        setTextureWrapperFormat(chipCtx, (glsTEXTUREINFO *)texObj->privateData, mipmap->baseFormat);
        getImageFormat(mipmap->format, mipmap->type, &textureInfo->imageFormat, NULL);
        textureInfo->residentFormat = __glVIVDevFormatToHWFormat[mipmap->deviceFormat->devfmt];

        /* Construct the gcoTEXTURE object. */
        if (textureInfo->object == gcvNULL) {
            status = gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &textureInfo->object);
            if (gcmIS_ERROR(status)) {
                return;
            }
            gcoTEXTURE_SetEndianHint(textureInfo->object,
                                     getEndianHint(mipmap->baseFormat, mipmap->type));
            if ( mipmap->baseFormat == GL_DEPTH_COMPONENT)
            {
                gcoTEXTURE_SetDepthTextureFlag(textureInfo->object, gcvTRUE);
            } else {
                gcoTEXTURE_SetDepthTextureFlag(textureInfo->object, gcvFALSE);
            }
        }

        /* Add the mipmap. If it already exists, the call will be ignored. */
        status = gcoTEXTURE_AddMipMap(textureInfo->object,
                                      level,
                                      mipmap->requestedFormat,
                                      textureInfo->residentFormat,
                                      mipmap->width,
                                      mipmap->height,
                                      0,
                                      texObj->arrays,
                                      gcvPOOL_DEFAULT,
                                      gcvNULL);

        if (gcmIS_ERROR(status)) {
            return;
        }

        if (!chipCtx->renderToTexture && (textureInfo->texRenderTarget == gcvNULL))
        {
             status = gcoSURF_Construct(gcvNULL,
                          mipmap->width,
                          mipmap->height,
                          1,
                          gcvSURF_RENDER_TARGET_NO_TILE_STATUS,
                          textureInfo->residentFormat,
                          gcvPOOL_DEFAULT,
                          &textureInfo->texRenderTarget);

            if (gcmIS_ERROR(status)) {
                return;
            }
        }

        if (buf == gcvNULL)
        {
            return;
        }

        /* Possibly the texture is needed to be converted.*/
        /* If it can't convert the format or have not to do, change nothing */
        /* This function will perhaps generate a temp buf, if return NULL, no temp buf is existing */
        /* If temp buf is existing, please remove it after used */
        pbuf = convertToInternalTex(mipmap, textureInfo->residentFormat, mipmap->width, mipmap->height, &textureInfo->imageFormat, (GLvoid *)&buf);

        if (mipmap->compressed)
        {
            status = gcoTEXTURE_UploadCompressed(textureInfo->object,
                                                 level,
                                                 face,
                                                 mipmap->width,
                                                 mipmap->height,
                                                 0,
                                                 buf,
                                                 ((mipmap->width + 3) / 4) * ((mipmap->height + 3) / 4) * 8);
        }
        else
        {
            gceTEXTURE_FACE halFace = (__GL_TEXTURE_CUBEMAP_INDEX == texObj->targetIndex)
                            ? gcvFACE_POSITIVE_X + face : gcvFACE_NONE;
            gctSIZE_T skipImgs = (texObj->targetIndex == __GL_TEXTURE_2D_ARRAY_INDEX ||
                             texObj->targetIndex == __GL_TEXTURE_3D_INDEX) ?
                             (gctSIZE_T)ps->unpackModes.skipImages : 0;
            gctSIZE_T rowStride = 0;
            gctSIZE_T imgHeight = (gctSIZE_T)mipmap->height;

            gcChipProcessPixelStore(&ps->unpackModes,
                                        (gctSIZE_T)mipmap->width,
                                        (gctSIZE_T)mipmap->height,
                                        mipmap->format,
                                        mipmap->type,
                                        skipImgs,
                                        &rowStride,
                                        &imgHeight,
                                        &buf);

            status = gcoTEXTURE_Upload(textureInfo->object,
                                       level,
                                       halFace,
                                       mipmap->width,
                                       mipmap->height,
                                       0,
                                       (const GLvoid*)buf,
                                       rowStride,
                                       textureInfo->imageFormat,
                                       gcvSURF_COLOR_SPACE_LINEAR);

            if (gcmIS_ERROR(status)) {
                goto ENDFLAG;
            }

            if (!chipCtx->renderToTexture)
            {
                gcsSURF_VIEW rtView  = {textureInfo->texRenderTarget, 0, 1};
                gcsSURF_VIEW texView = {gcvNULL, 0, 1};

                status = gcoTEXTURE_GetMipMap(textureInfo->object, 0, &texView.surf);
                if (gcmIS_ERROR(status)) {
                    goto ENDFLAG;
                }

                status = gcoSURF_ResolveRect_v2(&texView, &rtView, gcvNULL);
                if (gcmIS_ERROR(status)) {
                    goto ENDFLAG;
                }
            }


        }

ENDFLAG:

        removeTempCVTData(pbuf);


        if (gcmIS_ERROR(status)) {
            return;
        }
        //textureInfo->residentLevels = numLevels;
        //textureInfo->residentFaces = numFaces;
    }
}

GLboolean copyTexImage(glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y)
{
    gceSTATUS status;
    __GLmipMapLevel* mipmap = &texObj->faceMipmap[face][level];
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

    buildTextureInfo(chipCtx, texObj, face, level);

    /* Construct the gcoTEXTURE object. */
    if (textureInfo->object == gcvNULL) {
        status = gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &textureInfo->object);
        if (gcmIS_ERROR(status)) {
            return GL_FALSE;
        }
    }

    /* Add the mipmap. If it already exists, the call will be ignored. */
    status = gcoTEXTURE_AddMipMap(textureInfo->object,
                                  level,
                                  mipmap->requestedFormat,
                                  textureInfo->residentFormat,
                                  mipmap->width,
                                  mipmap->height, 0,
                                  0,
                                  gcvPOOL_DEFAULT,
                                  gcvNULL);

    if (gcmIS_ERROR(status)) {
        return GL_FALSE;
    }

    /* Resolve the rectangle to the temporary surface. */
    status = resolveDrawToTempBitmap(chipCtx,
                                     chipCtx->readRT,
                                     x, y,
                                     mipmap->width,
                                     mipmap->height);

    if (gcmIS_ERROR(status))
    {
        return GL_FALSE;
    }

    /* Upload the texture. */
    status = gcoTEXTURE_Upload(textureInfo->object,
                               level,
                               face,
                               mipmap->width,
                               mipmap->height,
                               0,
                               chipCtx->tempLastLine,
                               chipCtx->tempStride,
                               chipCtx->tempFormat,
                               gcvSURF_COLOR_SPACE_LINEAR);

    if (gcmIS_ERROR(status))
    {
        return GL_FALSE;
    }

    return GL_TRUE;
}

GLvoid texSubImage(glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj,  GLint face, GLint level, GLint xoffset, GLint yoffset, GLint width, GLint height, const GLvoid* buf)
{
    gceSTATUS status;
    gcoSURF mipSurf;
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;
    __GLmipMapLevel *mipmap = &texObj->faceMipmap[face][level];
    gceSURF_FORMAT imageFormat;
    gctPOINTER pbuf;

    buildTextureInfo(chipCtx, texObj, face, level);

    /* Get mip map for specified level. */
    status = gcoTEXTURE_GetMipMap(textureInfo->object, level, &mipSurf);
    if (gcmIS_ERROR(status))
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    /* Upload the texture. */
    getImageFormat(mipmap->format, mipmap->type, &imageFormat, NULL);

    pbuf = convertToInternalTex(mipmap, textureInfo->residentFormat, mipmap->width, mipmap->height, &imageFormat, (GLvoid *)&buf);

    /* Upload the texture. */
    status = gcoTEXTURE_UploadSub(textureInfo->object,
                                  level, face,
                                  xoffset, yoffset,
                                  width, height, 0,
                                  buf,
                                  0,  // TODO: stride is not always 0
                                  imageFormat,
                                  gcvSURF_COLOR_SPACE_LINEAR,
                                  gcvINVALID_ADDRESS);

    removeTempCVTData(pbuf);

    if (gcmIS_ERROR(status))
    {
        return;
    }

    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
}

GLboolean copyTexSubImage(glsCHIPCONTEXT_PTR chipCtx, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y, GLint xoffset, GLint yoffset, GLint width, GLint height)
{
    gceSTATUS status;
    gcoSURF mipSurf;
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

    buildTextureInfo(chipCtx, texObj, face, level);

    /* Get mip map for specified level. */
    status = gcoTEXTURE_GetMipMap(textureInfo->object, level, &mipSurf);
    if (gcmIS_ERROR(status))
    {
        __glSetError(GL_INVALID_OPERATION);
        return GL_FALSE;
    }

    /* Resolve the rectangle to the temporary surface. */
    status = resolveDrawToTempBitmap(chipCtx,
        chipCtx->drawRT[0],
        x, y,
        width,
        height);

    if (gcmIS_ERROR(status))
    {
        return GL_FALSE;
    }

    /* Upload the texture. */
    status = gcoTEXTURE_UploadSub(textureInfo->object,
                                  level, face,
                                  xoffset, yoffset,
                                  width, height, 0,
                                  chipCtx->tempLastLine,
                                  chipCtx->tempStride,
                                  chipCtx->tempFormat,
                                  gcvSURF_COLOR_SPACE_LINEAR,
                                  gcvINVALID_ADDRESS);

    if (gcmIS_ERROR(status))
    {
        return GL_FALSE;
    }

    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
    return GL_TRUE;
}

/************************************************************************/
/* Implementation for device texture APIs                               */
/************************************************************************/

GLvoid __glChipBindTexture(__GLcontext* gc, __GLtextureObject *texObj)
{
    if (texObj->privateData == NULL)
    {
        glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)gc->imports.calloc(gc, 1, sizeof(glsTEXTUREINFO));
        if(textureInfo) {
            textureInfo->imageFormat    = gcvSURF_UNKNOWN;
            textureInfo->residentFormat = gcvSURF_UNKNOWN;
            texObj->privateData = textureInfo;
        }
    }
}

GLvoid __glChipDeleteTexture(__GLcontext* gc, __GLtextureObject *texObj)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    if (texObj->privateData != NULL)
    {
        resetTextureWrapper(chipCtx, texObj->privateData);
        gc->imports.free(gc, texObj->privateData);
        texObj->privateData = gcvNULL;
    }
}

GLvoid __glChipTexImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, const GLvoid *buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

#ifdef __GL_DELAY_TEX_UPLOAD_
    buildTextureInfo(chipCtx, texObj, 0, level);
    /* Save the client image and upload at draw time */
    CHIP_TEX_IMAGE_OUTDATED(textureInfo, level);
#else
    residentTextureLevel(&gc->clientState.pixel, chipCtx, texObj, 0, level, buf);
    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
#endif
}

GLvoid __glChipTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, const GLvoid *buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

#ifdef __GL_DELAY_TEX_UPLOAD_
    buildTextureInfo(chipCtx, texObj, face, level);
    /* Save the client image and upload at draw time */
    CHIP_TEX_IMAGE_OUTDATED(textureInfo, level);
#else
    residentTextureLevel(&gc->clientState.pixel, chipCtx, texObj, face, level, buf);
    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
#endif
}

GLvoid __glChipTexImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, const GLvoid *buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

#ifdef __GL_DELAY_TEX_UPLOAD_
    buildTextureInfo(chipCtx, texObj, 0, level);
    /* Save the client image and upload at draw time */
    CHIP_TEX_IMAGE_OUTDATED(textureInfo, level);
#else
    residentTextureLevel(&gc->clientState.pixel, chipCtx, texObj, 0, level, buf);
    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
#endif
}

GLvoid __glChipCompressedTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, const GLvoid* buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;

#ifdef __GL_DELAY_TEX_UPLOAD_
    buildTextureInfo(chipCtx, texObj, face, level);
    /* Save the client image and upload at draw time */
    CHIP_TEX_IMAGE_OUTDATED(textureInfo, level);
#else
    residentTextureLevel(&gc->clientState.pixel, chipCtx, texObj, face, level, buf);
    CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
#endif
}

GLvoid __glChipCompressedTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint xoffset, GLint yoffset, GLint width, GLint height, const GLvoid* buf)
{
    /* TODO */
}

GLvoid __glChipTexSubImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint xoffset, GLint width, const GLvoid* buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    texSubImage(chipCtx, texObj, 0, level, xoffset, 0, width, 1, buf);
}

GLvoid __glChipTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint xoffset, GLint yoffset, GLint width, GLint height, const GLvoid* buf)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    texSubImage(chipCtx, texObj, face, level, xoffset, yoffset, width, height, buf);
}

GLvoid __glChipTexSubImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint width, GLint height, GLint depth, const GLvoid* buf)
{
    /* TODO */
}

GLboolean __glChipCopyTexImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    return copyTexImage(chipCtx, texObj, 0, level, x, y);
}

GLboolean __glChipCopyTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    return copyTexImage(chipCtx, texObj, face, level, x, y);
}

GLboolean __glChipCopyTexSubImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y, GLint width, GLint xoffset)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    return copyTexSubImage(chipCtx, texObj, 0, level, x, y, xoffset, 0, width, 1);
}

GLboolean __glChipCopyTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y, GLint width, GLint height, GLint xoffset, GLint yoffset)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    return copyTexSubImage(chipCtx, texObj, face, level, x, y, xoffset, yoffset, width, height);
}

GLboolean __glChipCopyTexSubImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y, GLint width, GLint height,GLint xoffset, GLint yoffset, GLint zoffset)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    buildTextureInfo(chipCtx, texObj, 0, level);

    return GL_TRUE;
}

GLboolean __glChipGenerateMipMap(__GLcontext* gc, __GLtextureObject* texObj, GLint face, GLint maxLevel)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    GLint baseLevel;
    glsTEXTUREINFO * textureInfo = (glsTEXTUREINFO *)texObj->privateData;
    gcoSURF lod0, lod1;
    GLint level;

    gcmHEADER_ARG("Context=0x%x texObj=0x%x maxLevel=%d Faces=%u",
                    gc, texObj, maxLevel, face);

    textureInfo = texObj->privateData;
    /* Construct texture object. */
    if (textureInfo)
    {
        /* Construct the gcoTEXTURE object. */
        if (textureInfo->object == gcvNULL) {
            status = gcoTEXTURE_ConstructEx(chipCtx->hal, __glChipTexTargetToHAL[texObj->targetIndex], &textureInfo->object);
            if (gcmIS_ERROR(status))
            {
                gcmFOOTER_ARG("result=%d", status);
                return GL_FALSE;
            }
            gcoTEXTURE_SetEndianHint(textureInfo->object,
                    getEndianHint(texObj->faceMipmap[0][texObj->params.baseLevel].baseFormat,
                    texObj->faceMipmap[0][texObj->params.baseLevel].type));
        }

        baseLevel = texObj->params.baseLevel;

        if (CHIP_TEX_IMAGE_IS_UPTODATE(textureInfo, baseLevel) == 0)
        {
            /* Add the base leve. If it already exists, the call will be ignored. */
            status = gcoTEXTURE_AddMipMap(
                textureInfo->object,
                baseLevel,
                texObj->faceMipmap[face][baseLevel].requestedFormat,
                textureInfo->residentFormat,
                texObj->faceMipmap[face][baseLevel].width,
                texObj->faceMipmap[face][baseLevel].height, 0,
                texObj->arrays,
                gcvPOOL_DEFAULT,
                gcvNULL
                 );

            if (gcmIS_ERROR(status)) {
                gcoTEXTURE_Destroy(textureInfo->object);
                textureInfo->object = gcvNULL;
                gcmFOOTER_ARG("result=%d", status);
                return GL_FALSE;
            }

            /* upload the base level texture */
            status = gcoTEXTURE_Upload(textureInfo->object,
                                       baseLevel,
                                       face,
                                       texObj->faceMipmap[face][baseLevel].width,
                                       texObj->faceMipmap[face][baseLevel].height,
                                       0,
                                       /* TODO: need to save the image or map from device */
                                       NULL, /*texObj->faceMipmap[0][baseLevel].mipBuffer,*/
                                       0, /* stride */
                                       textureInfo->imageFormat,
                                       gcvSURF_COLOR_SPACE_LINEAR);

            if (gcmIS_ERROR(status))
            {
                gcoTEXTURE_Destroy(textureInfo->object);
                textureInfo->object = gcvNULL;
                CHIP_TEX_IMAGE_OUTDATE_ALL(textureInfo);
                gcmFOOTER_ARG("result=%d", status);
                return GL_FALSE;;
            }
            CHIP_TEX_IMAGE_UPTODATE(textureInfo, baseLevel);
        }

        for (level = baseLevel + 1; level <= maxLevel; level++)
        {
            /* Get the texture surface. */
            status = gcoTEXTURE_GetMipMap(
                textureInfo->object,
                level - 1, &lod0
                );

            if (gcmIS_ERROR(status))
            {
                gcoTEXTURE_Destroy(textureInfo->object);
                textureInfo->object = gcvNULL;
                gcmFOOTER_ARG("result=%d", status);
                CHIP_TEX_IMAGE_OUTDATE_ALL(textureInfo);
                return GL_FALSE;;
            }

            /* Create a new level. */
            status = gcoTEXTURE_AddMipMap(
                textureInfo->object,
                level,
                texObj->faceMipmap[face][baseLevel].requestedFormat,
                textureInfo->residentFormat,
                texObj->faceMipmap[face][level].width,
                texObj->faceMipmap[face][level].height,
                0,
                texObj->arrays,
                gcvPOOL_DEFAULT,
                &lod1
                );

            if (gcmIS_ERROR(status))
            {
                gcoTEXTURE_Destroy(textureInfo->object);
                textureInfo->object = gcvNULL;
                CHIP_TEX_IMAGE_OUTDATE_ALL(textureInfo);
                gcmFOOTER_ARG("result=%d", status);
                return GL_FALSE;;
            }

            /* For blit engine, we generate mipmap in one shot. */
            if (chipCtx->hasBlitEngine)
            {
                if (level == maxLevel)
                {
                    status = gcoTEXTURE_GenerateMipMap(textureInfo->object,
                                                       baseLevel, maxLevel);
                }
            }
            else
            {
                status = gcoSURF_Resample_v2(lod0, lod1);
            }
            if (gcmIS_ERROR(status))
            {
                gcoTEXTURE_Destroy(textureInfo->object);
                textureInfo->object = gcvNULL;
                CHIP_TEX_IMAGE_OUTDATE_ALL(textureInfo);
                gcmFOOTER_ARG("result=%d", status);
                return GL_FALSE;;
            }

            /* currently we always flush and wait for HW finish, otherwise */
            /* for small size mipmap is generated by CPU not HW, this could */
            /* cuase the problem, will fix later */
            gcoSURF_Flush(lod1);
            /* Commit command buffer. */
            gcoHAL_Commit(chipCtx->hal, gcvTRUE);

            CHIP_TEX_IMAGE_UPTODATE(textureInfo, level);
        }
        textureInfo->residentLevels = maxLevel + 1;
    }

    gcmFOOTER();

    /* Return status. */
    return GL_TRUE;
}

