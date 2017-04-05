/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_glff_texture_
#define __gc_glff_texture_


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
************************** Define structure pointers. **************************
\******************************************************************************/

typedef struct _glsTEXTURE        * glsTEXTURE_PTR;
typedef struct _glsTEXTUREWRAPPER * glsTEXTUREWRAPPER_PTR;
typedef struct _glsTEXTURESAMPLER * glsTEXTURESAMPLER_PTR;
typedef struct _glsTEXTURELIST    * glsTEXTURELIST_PTR;

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
    glvCOMBINEDOT3RGBA
}
gleTEXCOMBINEFUNCTION, * gleTEXCOMBINEFUNCTION_PTR;

typedef enum _gleCOMBINESOURCE
{
    glvTEXTURE,
    glvCONSTANT,
    glvCOLOR,
    glvPREVIOUS
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
******** direct texture information structure (GL_VIV_direct_texture). *********
\******************************************************************************/

typedef struct _glsDIRECTTEXTURE * glsDIRECTTEXTURE_PTR;
typedef struct _glsDIRECTTEXTURE
{
    gctBOOL                 dirty;          /* Direct texture change flag. */
    gcoSURF                 source;         /* Surface exposed to the user. */
    gctBOOL                 directSample;   /* Can bypass uploading. */
    gceSURF_FORMAT          textureFormat;  /* Closet supported tx format. */
}
glsDIRECTTEXTURE;

/******************************************************************************\
**** EGL image texture information structure (GL_OES_egl_image[_external]). ****
\******************************************************************************/

typedef struct _glsEGLIMAGETEXTURE * glsEGLIMAGETEXTURE_PTR;
typedef struct _glsEGLIMAGETEXTURE
{
    gctBOOL                 dirty;          /* Direct texture change flag. */
    gcoSURF                 source;         /* Surface exposed to the user. */
    gctBOOL                 directSample;   /* Can bypass uploading. */
    gceSURF_FORMAT          textureFormat;  /* Closet supported tx format. */

    gctBOOL                 genMipmap;      /* If need generate mipap. */

    gctPOINTER              nativeBuffer;   /* EGL_ANDROID_image_native_buffer. */
    void *                  image;          /* Reference khr EGL Image. */
}
glsEGLIMAGETEXTURE;


/******************************************************************************\
*************************** Texture object wrapper. ****************************
\******************************************************************************/

typedef enum _gleTARGETTYPE
{
    glvTEXTURE2D       = 0,
    glvCUBEMAP         = 1,
    glvTEXTUREEXTERNAL = 2,
}
gleTARGETTYPE, * gleTARGETTYPE_PTR;



typedef struct _glsTEXTUREWRAPPER
{
    GLuint                  name;           /* Texture name. */
    gcoTEXTURE              object;         /* Texture object. */
    gctBOOL                 dirty;          /* Texture dirty flag. */
    GLsizei                 width;          /* Texture width at LOD 0. */
    GLsizei                 height;         /* Texture height at LOD 0. */
    GLint                   maxLevel;       /* The number of mipmaps. */
    GLint                   maxLevelUsed;   /* The max level that will be sampled by HW, was evaluated at draw time.*/
    GLenum                  format;         /* Texture base internal format */
    gleTARGETTYPE           targetType;     /* texture type */
    glsTEXTURESAMPLER_PTR   binding;        /* Pointer to the bound sampler. */
    gctBOOL                 boundAtLeastOnce;

    /* Format dependent texture combine data flow. */
    glsCOMBINEFLOW          combineFlow;

    GLint                   maxLOD;         /* Maximum LOD value. */
    gleTEXTUREFILTER        minFilter;      /* Texture min filter. */
    gleTEXTUREFILTER        magFilter;      /* Texture mag filter. */
    GLuint                  anisoFilter;    /* Texture anisotropic filter param. */
    gleTEXTUREWRAP          wrapS;          /* Texture wrap S. */
    gleTEXTUREWRAP          wrapT;          /* Texture wrap T. */
    GLboolean               genMipmap;      /* Automatic map generation. */

    /* GL_OES_draw_texture extension. */
    GLboolean               dirtyCropRect;
    GLint                   cropRect[4];
    GLfloat                 texCoordBuffer[4 * 2];

    /* GL_OES_egl_image[_external] extension. */
    glsEGLIMAGETEXTURE      image;

    /* GL_VIV_direct_texture extension. */
    glsDIRECTTEXTURE        direct;

    /* Is the mipmpa surface from an external render target? */
    glsTEXTUREWRAPPER_PTR   prev;
    glsTEXTUREWRAPPER_PTR   next;

    /* Number of texture units (could be from different contexts) that bound to the object.
    */
    GLuint bindCount;

    /* Internal flag for generic object management.
    */
    GLbitfield flag;
}
glsTEXTUREWRAPPER;


/******************************************************************************\
****************************** Texture List ***********************************
\******************************************************************************/
#define NAMED_TEXTURE_HASH 256

typedef struct _glsTEXTURELIST
{
    GLuint                       lastTexture;
    glsTEXTUREWRAPPER_PTR        objects[NAMED_TEXTURE_HASH];

#if gldSUPPORT_SHARED_CONTEXT
    gctPOINTER                   sharedLock;
    GLuint                       reference;
#endif
}
glsTEXTURELIST;

/******************************************************************************\
*************************** Texture combine states. ****************************
\******************************************************************************/

typedef struct _glsTEXTURECOMBINE * glsTEXTURECOMBINE_PTR;
typedef struct _glsTEXTURECOMBINE
{
    gleTEXCOMBINEFUNCTION   function;
    gleCOMBINESOURCE        source[3];
    gleCOMBINEOPERAND       operand[3];
    GLfloat                 scale;
    glsCOMBINEFLOW_PTR      combineFlow;
}
glsTEXTURECOMBINE;


/******************************************************************************\
************************* Texture sampler structure. ***************************
\******************************************************************************/

typedef struct _glsTEXTURESAMPLER
{
    /* Sampler index. */
    GLuint                  index;

    /* Currently bound texture (same as one of bindings[]). */
    glsTEXTUREWRAPPER_PTR   binding;

    /* Points to currently bound textures by target. */
    glsTEXTUREWRAPPER_PTR   bindings[3];

    /* Texturing enable flags. */
    GLboolean               enableTexturing;
    GLboolean               stageEnabled;

    /* Texture coordinate stream states. */
    glsATTRIBUTEINFO        aTexCoordInfo;
    GLboolean               coordReplace;
    gcSHADER_TYPE           coordType;
    gcSL_SWIZZLE            coordSwizzle;

    /* Texture coordinate constant states. */
    GLboolean               recomputeCoord;
    glsVECTOR               homogeneousCoord;
    glsVECTOR               queryCoord;

    /* GL_OES_draw_texture extension. */
    glsATTRIBUTEINFO        aTexCoordDrawTexInfo;

    /* 3D copy. */
    glsATTRIBUTEINFO        aTexCoordTexCopy3DInfo;

    /* Texture constant color. */
    glsVECTOR               constColor;

    GLfloat                 lodBias;

    /* Texture functions. */
    gleTEXTUREFUNCTION      function;       /* Top-level. */
    glsTEXTURECOMBINE       combColor;      /* Combine color. */
    glsTEXTURECOMBINE       combAlpha;      /* Combine alpha. */
    glsCOMBINEFLOW          colorDataFlow;  /* Color data flow. */
    glsCOMBINEFLOW          alphaDataFlow;  /* Alpha data flow. */

    /* Texture coordinate generation (GL_OES_texture_cube_map). */
    GLboolean               genEnable;
    GLboolean               enableCubeTexturing;
    gleTEXTUREGEN           genMode;

    /* Texturing enable flags(GL_OES_image_external). */
    GLboolean                enableExternalTexturing;
}
glsTEXTURESAMPLER;


/******************************************************************************\
*************************** Main texture structure. ****************************
\******************************************************************************/

typedef struct _glsTEXTURE
{
    /* List of allocated textures. */
    glsTEXTURELIST_PTR      textureList;
    glsTEXTUREWRAPPER       defaultTexture[3];

    /* List of logical samplers. */
    glsTEXTURESAMPLER_PTR   sampler;

    /* Active sampler. */
    glsTEXTURESAMPLER_PTR   activeSampler;
    GLuint                  activeSamplerIndex;

    /* Active sampler for client side texture parameters. */
    glsTEXTURESAMPLER_PTR   activeClientSampler;
    GLuint                  activeClientSamplerIndex;

    /* Texture caps. */
    GLuint                  maxWidth;
    GLuint                  maxHeight;
    GLuint                  maxDepth;
    GLboolean               cubic;
    GLboolean               nonPowerOfTwo;
    GLint                   pixelSamplers;
    GLenum                  generateMipmapHint;

    GLboolean               matrixDirty;
}
glsTEXTURE;


/******************************************************************************\
********************** State loading for draw primitives. **********************
\******************************************************************************/

gceSTATUS glfLoadTexture(
    glsCONTEXT_PTR Context
    );


/******************************************************************************\
********************** Texture Management init / cleanup. **********************
\******************************************************************************/

gceSTATUS glfInitializeTexture(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfPointTexture(
    glsTEXTURELIST_PTR * Pointer,
    glsTEXTURELIST_PTR List
    );

gceSTATUS glfFlushTexture(
    glsCONTEXT_PTR Context
    );

gceSTATUS glfDestroyTexture(
    glsCONTEXT_PTR Context
    );

glsTEXTUREWRAPPER_PTR glfFindTexture(
    glsCONTEXT_PTR Context,
    GLuint Texture
    );

gceSTATUS glfInitializeTempBitmap(
    IN glsCONTEXT_PTR Context,
    IN gceSURF_FORMAT Format,
    IN gctUINT Width,
    IN gctUINT Height
    );

gceSTATUS glfResolveDrawToTempBitmap(
    IN glsCONTEXT_PTR Context,
    IN gctINT SourceX,
    IN gctINT SourceY,
    IN gctINT Width,
    IN gctINT Height
    );

GLenum glfEnableTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLenum glfEnableCubeTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLenum glfEnableExternalTexturing(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

GLenum glfEnableCoordGen(
    glsCONTEXT_PTR Context,
    GLboolean Enable
    );

gceSTATUS glfUpdateTextureStates(
    glsCONTEXT_PTR Context
    );

GLboolean glfQueryTextureState(
    glsCONTEXT_PTR Context,
    GLenum Name,
    GLvoid* Value,
    gleTYPE Type
    );

EGLenum glfBindTexImage(
    void * Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    void ** Binder
    );

EGLenum glfCreateImageTexture(
    void* Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    );


#ifdef __cplusplus
}
#endif

#endif /* __gc_glff_texture_ */
