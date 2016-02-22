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


#ifndef __chip_texture_h_
#define __chip_texture_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
************************** Define structure pointers. **************************
\******************************************************************************/

typedef struct _glsFSCONTROL * glsFSCONTROL_PTR;
typedef struct _glsTEXTURESAMPLER * glsTEXTURESAMPLER_PTR;

/******************************************************************************\
************************ Texture parameter enumerations. ***********************
\******************************************************************************/

typedef enum _gleTEXTUREFILTER
{
    glvNEAREST,
    glvLINEAR,
    glvNEAREST_MIPMAP_NEAREST,
    glvLINEAR_MIPMAP_NEAREST,
    glvNEAREST_MIPMAP_LINEAR,
    glvLINEAR_MIPMAP_LINEAR,
}
gleTEXTUREFILTER, * gleTEXTUREFILTER_PTR;

typedef enum _gleTEXTUREWRAP
{
    glvCLAMP,
    glvREPEAT,
    glvMIRROR,
}
gleTEXTUREWRAP, * gleTEXTUREWRAP_PTR;

typedef enum _gleTEXTUREGEN
{
    glvTEXNORMAL,
    glvREFLECTION,
    glvOBJECTLINEAR,
    glvEYELINEAR,
    glvSPHERE,
}
gleTEXTUREGEN, * gleTEXTUREGEN_PTR;


/******************************************************************************\
************************ Texture function enumerations. ************************
\******************************************************************************/

typedef enum _gleTEXTUREFUNCTION
{
    glvTEXREPLACE,
    glvTEXMODULATE,
    glvTEXDECAL,
    glvTEXBLEND,
    glvTEXADD,
    glvTEXCOMBINE
}
gleTEXTUREFUNCTION, * gleTEXTUREFUNCTION_PTR;

typedef enum _gleTEXCOMBINEFUNCTION
{
    glvCOMBINEREPLACE,
    glvCOMBINEMODULATE,
    glvCOMBINEADD,
    glvCOMBINEADDSIGNED,
    glvCOMBINEINTERPOLATE,
    glvCOMBINESUBTRACT,
    glvCOMBINEDOT3RGB,
    glvCOMBINEDOT3RGBA,
    glvCOMBINEDOT3RGBEXT,
    glvCOMBINEDOT3RGBAEXT,
    glvMODULATEADD,
    glvMODULATESIGNEDADD,
    glvMODULATESUBTRACTADD
}
gleTEXCOMBINEFUNCTION, * gleTEXCOMBINEFUNCTION_PTR;

typedef enum _gleCOMBINESOURCE
{
    glvCONSTANT,
    glvCOLOR,
    glvPREVIOUS,
    glvTEXTURE,
    glvTEXTURE0,
    glvTEXTURE1,
    glvTEXTURE2,
    glvTEXTURE3,
    glvTEXTURE4,
    glvTEXTURE5,
    glvTEXTURE6,
    glvTEXTURE7,
    glvPRIMARYCOLOR,
}
gleCOMBINESOURCE, * gleCOMBINESOURCE_PTR;

typedef enum _gleCOMBINEOPERAND
{
    glvSRCALPHA,
    glvSRCALPHAINV,
    glvSRCCOLOR,
    glvSRCCOLORINV
}
gleCOMBINEOPERAND, * gleCOMBINEOPERAND_PTR;


/******************************************************************************\
************************* Texture Combine data flow. ***************************
\******************************************************************************/

typedef struct _glsCOMBINEFLOW * glsCOMBINEFLOW_PTR;
typedef struct _glsCOMBINEFLOW
{
    gcSL_ENABLE             targetEnable;
    gcSL_ENABLE             tempEnable;
    gcSL_SWIZZLE            tempSwizzle;
    gcSL_SWIZZLE            argSwizzle;
}
glsCOMBINEFLOW;


/******************************************************************************\
****** YUV direct texture information structure (GL_VIV_direct_texture). *******
\******************************************************************************/

typedef struct _glsDIRECTTEXTURE * glsDIRECTTEXTURE_PTR;
typedef struct _glsDIRECTTEXTURE
{
    gctBOOL                 dirty;          /* Direct texture change flag. */
    gcoSURF                 source;         /* Surface exposed to the user. */
    gcoSURF                 temp;           /* Temporary result surface. */
    gcoSURF *               texture;        /* Final texture surface. */
}
glsDIRECTTEXTURE;

/******************************************************************************\
*************************** Texture combine states. ****************************
\******************************************************************************/

typedef struct _glsTEXTURECOMBINE * glsTEXTURECOMBINE_PTR;
typedef struct _glsCOMBINEFUNCTION * glsCOMBINEFUNCTION_PTR;

typedef gceSTATUS (* glfDOTEXTUREFUNCTION) (
    __GLcontext * gc,
    GLvoid * ShaderControl,
    glsTEXTURESAMPLER_PTR Sampler,
    gctUINT SamplerNumber
    );

typedef struct _glsTEXTURECOMBINE
{
    gleTEXCOMBINEFUNCTION   function;
    gleCOMBINESOURCE        source[3];
    gleCOMBINEOPERAND       operand[3];
    glsMUTANT               scale;
    glsCOMBINEFLOW_PTR      combineFlow;
}
glsTEXTURECOMBINE;


#define CHIP_TEX_IMAGE_UPTODATE(textureInfo, level) \
    textureInfo->imageUpToDate |= (1 << (level))

#define CHIP_TEX_IMAGE_OUTDATED(textureInfo, level) \
    textureInfo->imageUpToDate &= ~(1 << (level))

#define CHIP_TEX_IMAGE_OUTDATE_ALL(textureInfo) \
    textureInfo->imageUpToDate = 0;

#define CHIP_TEX_IMAGE_IS_UPTODATE(textureInfo, level) \
    (textureInfo->imageUpToDate & (1 << (level)))

#define CHIP_TEX_IMAGE_IS_ANY_LEVEL_DIRTY( textureInfo, baseLevel, maxLevel) \
    (~(textureInfo->imageUpToDate)) & ( (1 << (maxLevel + 1)) - (1 << baseLevel))

/******************************************************************************\
*************************** Main texture structure. ****************************
\******************************************************************************/
#define TEXT_STIPPLE_CACHE_NUM  24
typedef struct _glsTEXTUREINFO {
    gcoTEXTURE object;                     /* Texture object. */

    /* Format dependent texture combine data flow. */
    glsCOMBINEFLOW combineFlow;

    /* Bit fields telling if the texture image in device memory is uptodate, one bit controls one level.
       if dp is going to use this texture, we must make sure that the texture image is uptodate( in device memory)*/
    GLbitfield              imageUpToDate;

    /* The number of levels that are resident(from base level) */
    GLuint                  residentLevels;

    /* The number of faces that are resident (array texture) */
    GLuint                  residentFaces;

    /* Texture base format */
    GLenum format;

    gceSURF_FORMAT imageFormat;
    /* The format of the resident resource */
    gceSURF_FORMAT residentFormat;
    GLboolean   dirty;
    gcoSURF     texRenderTarget;
    gctBOOL     renderDirty;
} glsTEXTUREINFO;

/******************************************************************************\
************************* Texture sampler structure. ***************************
\******************************************************************************/

typedef struct _glsTEXTURESAMPLER
{
    /* Sampler index. */
    GLuint                  index;

    /* Currently bound texture (same as one of bindings[]). */
    glsTEXTUREINFO * binding;

    /* Texture coordinate stream states. */
    GLboolean               coordReplace;
    gcSHADER_TYPE           coordType;
    gcSL_SWIZZLE            coordSwizzle;

    /* Texture constant color. */
    glsVECTOR               constColor;

    /* Texture functions. */
    gleTEXTUREFUNCTION      function;       /* Top-level. */
    glsTEXTURECOMBINE       combColor;      /* Combine color. */
    glsTEXTURECOMBINE       combAlpha;      /* Combine alpha. */
    glsCOMBINEFLOW          colorDataFlow;  /* Color data flow. */
    glsCOMBINEFLOW          alphaDataFlow;  /* Alpha data flow. */

    GLuint                  genEnable;

    /* for s, t, r, q */
    gleTEXTUREGEN           genMode[4];

    glfDOTEXTUREFUNCTION doTextureFunction;
}
glsTEXTURESAMPLER;

typedef struct _glsTEXTURE
{
    GLuint    stageEnabledMask;
    gcsTEXTURE halTexture[glvMAX_TEXTURES];
    glsTEXTURESAMPLER * sampler;
}
glsTEXTURE;

extern GLvoid __glChipBindTexture(__GLcontext* gc, __GLtextureObject *texObj);
extern GLvoid __glChipDeleteTexture(__GLcontext* gc, __GLtextureObject *texObj);
extern GLvoid __glChipTexImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, const GLvoid *buf);
extern GLvoid __glChipTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, const GLvoid *buf);
extern GLvoid __glChipTexImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, const GLvoid *buf);
extern GLvoid __glChipTexSubImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint xoffset, GLint width, const GLvoid* buf);
extern GLvoid __glChipTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint xoffset, GLint yoffset, GLint width, GLint height, const GLvoid* buf);
extern GLvoid __glChipTexSubImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint width, GLint height, GLint depth, const GLvoid* buf);
extern GLboolean __glChipCopyTexImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y);
extern GLboolean __glChipCopyTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y);
extern GLboolean __glChipCopyTexSubImage1D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y, GLint width, GLint xoffset);
extern GLboolean __glChipCopyTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint x, GLint y, GLint width, GLint height, GLint xoffset, GLint yoffset);
extern GLboolean __glChipCopyTexSubImage3D(__GLcontext* gc, __GLtextureObject *texObj,  GLint level, GLint x, GLint y, GLint width, GLint height, GLint xoffset, GLint yoffset,GLint zoffset);
extern GLboolean __glChipGenerateMipMap(__GLcontext* gc, __GLtextureObject* texObj, GLint face, GLint maxLevel);
extern GLvoid __glChipCompressedTexImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, const GLvoid* buf);
extern GLvoid __glChipCompressedTexSubImage2D(__GLcontext* gc, __GLtextureObject *texObj,  GLint face, GLint level, GLint xoffset, GLint yoffset, GLint width, GLint height, const GLvoid* buf);

#ifdef __cplusplus
}
#endif

#endif /* __chip_texture_h_ */
