/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __chip_base_h__
#define __chip_base_h__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
********************************* Mutable Types ********************************
\******************************************************************************/

typedef enum __gleTYPE
{
    glvBOOL,
    glvINT,
    glvNORM,
    glvFIXED,
    glvFLOAT
}
gleTYPE, * gleTYPE_PTR;

typedef union __gluMUTABLE * gluMUTABLE_PTR;
typedef union __gluMUTABLE
{
    GLint                   i;
    GLuint                  ui;
    GLenum                  e;
    GLfixed                 x;
    GLfloat                 f;
    GLclampf                cf;
} gluMUTABLE;

typedef struct __glsMUTANT * glsMUTANT_PTR;
typedef struct __glsMUTANT
{
    /* Mutable value (must be at the beginning of the structure). */
    gluMUTABLE              value;

    /* Special value flags. */
    GLboolean               zero;
    GLboolean               one;

    /* Current type. */
    gleTYPE                 type;
} glsMUTANT;

typedef struct __glsVECTOR * glsVECTOR_PTR;
typedef struct __glsVECTOR
{
    /* Mutable values (must be at the beginning of the structure). */
    gluMUTABLE              value[4];

    /* Special value flags. */
    GLboolean               zero3;
    GLboolean               zero4;
    GLboolean               one3;
    GLboolean               one4;

    /* Current type. */
    gleTYPE                 type;
} glsVECTOR;

typedef struct __glsMATRIX * glsMATRIX_PTR;
typedef struct __glsMATRIX
{
    /* Matrix mutable values. */
    gluMUTABLE              value[16];

    /* Current type. */
    gleTYPE                 type;

    /* Special value flags. */
    GLboolean               identity;
} glsMATRIX;

#ifdef OPENGL40

/* Maximum number of matrix indices per vertex for matrix palette. */
#define glvMAX_VERTEX_UNITS                 3

/* Maximum number of palette matrices supported. */
#define glvMAX_PALETTE_MATRICES             9

/* Maximum number of textures. */
#define glvMAX_TEXTURES                     8

/* Maximum number of clip planes. */
#define glvMAX_CLIP_PLANES                  6

/* Maximum number of lights. */
#define glvMAX_LIGHTS                       8

/* PI/180 value. */
/* There is a PIOVER180 in ES core, see the next, make GL4 use ES ones */
/*
#define gldPIOVER180            0.017453292519943295769236907684886f
*/

#define glmUNIFORM_INDEX(Shader, Name) \
    glvUNIFORM_ ## Shader ## _ ## Name

/* Shader attribute index definition. */
#define glmATTRIBUTE_INDEX(Shader, Name) \
    glvATTRIBUTE_ ## Shader ## _ ## Name

#define glmTRANSLATEHALSTATUS(func) \
    ( \
        gcmIS_SUCCESS(func) \
            ? GL_NO_ERROR \
            : GL_INVALID_OPERATION \
    )

/* Vertex shader uniforms. */
typedef enum _gleVS_UNIFORMS
{
    glmUNIFORM_INDEX(VS, uColor),
    glmUNIFORM_INDEX(VS, uNormal),

    glmUNIFORM_INDEX(VS, uModelView),
    glmUNIFORM_INDEX(VS, uModelViewInverse3x3Transposed),
    glmUNIFORM_INDEX(VS, uModelViewProjection),
    glmUNIFORM_INDEX(VS, uProjection),

    glmUNIFORM_INDEX(VS, uEcm),
    glmUNIFORM_INDEX(VS, uAcm),
    glmUNIFORM_INDEX(VS, uDcm),
    glmUNIFORM_INDEX(VS, uAcs),
    glmUNIFORM_INDEX(VS, uSrm),
    glmUNIFORM_INDEX(VS, uScm),

    glmUNIFORM_INDEX(VS, uPpli),
    glmUNIFORM_INDEX(VS, uKi),
    glmUNIFORM_INDEX(VS, uSrli),
    glmUNIFORM_INDEX(VS, uCrli),
    glmUNIFORM_INDEX(VS, uAcli),
    glmUNIFORM_INDEX(VS, uDcli),
    glmUNIFORM_INDEX(VS, uScli),
    glmUNIFORM_INDEX(VS, uAcmAcli),
    glmUNIFORM_INDEX(VS, uAcmAcli2),
    glmUNIFORM_INDEX(VS, uVPpli),
    glmUNIFORM_INDEX(VS, uDcmDcli),
    glmUNIFORM_INDEX(VS, uDcmDcli2),
    glmUNIFORM_INDEX(VS, uCosCrli),
    glmUNIFORM_INDEX(VS, uNormedSdli),

    glmUNIFORM_INDEX(VS, uTexCoord),
    glmUNIFORM_INDEX(VS, uTexMatrix),
    glmUNIFORM_INDEX(VS, uClipPlane),
    glmUNIFORM_INDEX(VS, uTexGenEyePlane),
    glmUNIFORM_INDEX(VS, uTexGenObjectPlane),
    glmUNIFORM_INDEX(VS, uPointAttenuation),
    glmUNIFORM_INDEX(VS, uPointSize),
    glmUNIFORM_INDEX(VS, uViewport),
    glmUNIFORM_INDEX(VS, uMatrixPalette),
    glmUNIFORM_INDEX(VS, uMatrixPaletteInverse),
    glmUNIFORM_INDEX(VS, uFogCoord),

    glvUNIFORM_VS_COUNT
}
gleVS_UNIFORMS;

/* Fragment shader uniforms. */
typedef enum _gleFS_UNIFORMS
{
    glmUNIFORM_INDEX(FS, uColor),
    glmUNIFORM_INDEX(FS, uColor2),
    glmUNIFORM_INDEX(FS, uFogFactors),
    glmUNIFORM_INDEX(FS, uFogColor),
    glmUNIFORM_INDEX(FS, uTexColor),
    glmUNIFORM_INDEX(FS, uTexCombScale),
    glmUNIFORM_INDEX(FS, uTexSampler0),
    glmUNIFORM_INDEX(FS, uTexSampler1),
    glmUNIFORM_INDEX(FS, uTexSampler2),
    glmUNIFORM_INDEX(FS, uTexSampler3),
    glmUNIFORM_INDEX(FS, uTexSampler4),
    glmUNIFORM_INDEX(FS, uTexSampler5),
    glmUNIFORM_INDEX(FS, uTexSampler6),
    glmUNIFORM_INDEX(FS, uTexSampler7),
    glmUNIFORM_INDEX(FS, uTexCoord),
    glmUNIFORM_INDEX(FS, uAccum),
    glmUNIFORM_INDEX(FS, uPixelTransferScale),
    glmUNIFORM_INDEX(FS, uPixelTransferBias),
    glmUNIFORM_INDEX(FS, uTextureBorderColor),
    glmUNIFORM_INDEX(FS, uYmajor),
    glvUNIFORM_FS_COUNT
}
gleFS_UNIFORMS;

/* Vertex shader attributes. */
typedef enum _gleVS_ATTRIBUTES
{
    glmATTRIBUTE_INDEX(VS, aPosition),
    glmATTRIBUTE_INDEX(VS, aWeight),
    glmATTRIBUTE_INDEX(VS, aNormal),
    glmATTRIBUTE_INDEX(VS, aColor),
    glmATTRIBUTE_INDEX(VS, aColor2),
    glmATTRIBUTE_INDEX(VS, aFogCoord),
    glmATTRIBUTE_INDEX(VS, aTexCoord0),
    glmATTRIBUTE_INDEX(VS, aTexCoord1),
    glmATTRIBUTE_INDEX(VS, aTexCoord2),
    glmATTRIBUTE_INDEX(VS, aTexCoord3),
    glmATTRIBUTE_INDEX(VS, aTexCoord4),
    glmATTRIBUTE_INDEX(VS, aTexCoord5),
    glmATTRIBUTE_INDEX(VS, aTexCoord6),
    glmATTRIBUTE_INDEX(VS, aTexCoord7),
    glmATTRIBUTE_INDEX(VS, aPointSize),
    glmATTRIBUTE_INDEX(VS, aMatrixIndex),
    glmATTRIBUTE_INDEX(VS, aMatrixWeight),

    glvATTRIBUTE_VS_COUNT
}
gleVS_ATTRIBUTES;

/* Fragment shader varyings. */
typedef enum _gleFS_ATTRIBUTES
{
    glmATTRIBUTE_INDEX(FS, vPosition),
    glmATTRIBUTE_INDEX(FS, vEyePosition),
    glmATTRIBUTE_INDEX(FS, vColor0),
    glmATTRIBUTE_INDEX(FS, vColor1),
    glmATTRIBUTE_INDEX(FS, vColor20),
    glmATTRIBUTE_INDEX(FS, vColor21),
    glmATTRIBUTE_INDEX(FS, vTexCoord0),
    glmATTRIBUTE_INDEX(FS, vTexCoord1),
    glmATTRIBUTE_INDEX(FS, vTexCoord2),
    glmATTRIBUTE_INDEX(FS, vTexCoord3),
    glmATTRIBUTE_INDEX(FS, vTexCoord4),
    glmATTRIBUTE_INDEX(FS, vTexCoord5),
    glmATTRIBUTE_INDEX(FS, vTexCoord6),
    glmATTRIBUTE_INDEX(FS, vTexCoord7),
    glmATTRIBUTE_INDEX(FS, vClipPlane0),
    glmATTRIBUTE_INDEX(FS, vClipPlane1),
    glmATTRIBUTE_INDEX(FS, vClipPlane2),
    glmATTRIBUTE_INDEX(FS, vClipPlane3),
    glmATTRIBUTE_INDEX(FS, vClipPlane4),
    glmATTRIBUTE_INDEX(FS, vClipPlane5),
    glmATTRIBUTE_INDEX(FS, vPointFade),
    glmATTRIBUTE_INDEX(FS, vPointSmooth),
    glmATTRIBUTE_INDEX(FS, vFace),

    glvATTRIBUTE_FS_COUNT
}
gleFS_ATTRIBUTES;

/* Functions for alpha, depth and stencil tests. */
typedef enum _gleTESTFUNCTIONS
{
    glvNEVER,
    glvLESS,
    glvEQUAL,
    glvLESSOREQUAL,
    glvGREATER,
    glvNOTEQUAL,
    glvGREATEROREQUAL,
    glvALWAYS
}
gleTESTFUNCTIONS, * gleTESTFUNCTIONS_PTR;

/* Supported fog modes. */
typedef enum _gleFOGMODES
{
    glvLINEARFOG,
    glvEXPFOG,
    glvEXP2FOG
}
gleFOGMODES, * gleFOGMODES_PTR;

/* Alpha blending functions. */
typedef enum _gleBLENDFUNCTIONS
{
    glvBLENDZERO,
    glvBLENDONE,
    glvBLENDSRCCOLOR,
    glvBLENDSRCCOLORINV,
    glvBLENDSRCALPHA,
    glvBLENDSRCALPHAINV,
    glvBLENDDSTCOLOR,
    glvBLENDDSTCOLORINV,
    glvBLENDDSTALPHA,
    glvBLENDDSTALPHAINV,
    glvBLENDSRCALPHASATURATE
}
gleBLENDFUNCTIONS, * gleBLENDFUNCTIONS_PTR;

typedef enum _gleBLENDMODES
{
    gcvBLENDADD,
    gcvBLENDSUBTRACT,
    gcvBLENDREVERSESUBTRACT,
    gcvBLENDMIN,
    gcvBLENDMAX,
}
gleBLENDMODES, * gleBLENDMODES_PTR;

/* Supported stencil operations. */
typedef enum _gleSTENCILOPERATIONS
{
    glvSTENCILZERO,
    glvSTENCILKEEP,
    glvSTENCILREPLACE,
    glvSTENCILINC,
    glvSTENCILDEC,
    glvSTENCILINVERT
}
gleSTENCILOPERATIONS, * gleSTENCILOPERATIONS_PTR;

/******************************************************************************\
********************************* Lighting states. *****************************
\******************************************************************************/

typedef struct _glsLIGHTING * glsLIGHTING_PTR;
typedef struct _glsLIGHTING
{
    /* Do two-sided lighting when rendering. */
    GLboolean               doTwoSidedlighting;

    /* mask if light enable */
    GLuint                  prevLightEnabled;
    /* mask if light enable */
    GLuint                  lightEnabled;

    /* Generuc function genertion enable flag. */
    GLuint                  useFunction;
}
glsLIGHTING;

typedef struct _glsPOINT * glsPOINT_PTR;
typedef struct _glsPOINT
{
    /* Not zero if current primitive is a point. */
    GLboolean               pointPrimitive;

    /* Point sprite states. */
    GLboolean               spriteDirty;
    GLboolean               spriteEnable;
    GLboolean               spriteActive;

    /* Point smooth enable flag. */
    GLboolean               smooth;

    /* Lower bound to which the derived point size is clamped. */
    glsMUTANT               clampFrom;

    /* Upper bound to which the derived point size is clamped. */
    glsMUTANT               clampTo;

    /* Distance attenuation function coefficients. */
    glsVECTOR               attenuation;

    /* Point fade threshold size. */
    glsMUTANT               fadeThrdshold;

    GLenum                  hint;
}
glsPOINT;

typedef struct _glsLOGICOP * glsLOGICOP_PTR;
typedef struct _glsLOGICOP
{
    GLboolean               enabled;
    GLenum                  operation;
    GLboolean               perform;
    gctUINT8                rop;
}
glsLOGICOP;

#endif


/******************************************************************************\
***************************** glsMUTANT Definitions ****************************
\******************************************************************************/

#define glmFIXEDMUTANTZERO \
    { \
        { 0 }, \
        GL_TRUE,            /* Zero. */ \
        GL_FALSE,           /* One. */ \
        glvFIXED \
    }

#define glmFIXEDMUTANTONE \
    { \
        { gcvONE_X }, \
        GL_FALSE,           /* Zero. */ \
        GL_TRUE,            /* One. */ \
        glvFIXED \
    }


/******************************************************************************\
******************************* Common Definitions *****************************
\******************************************************************************/

#define glvFLOATPI              3.141592653589793238462643383279502f
#define glvFIXEDPI              0x0003243F
#define glvFLOATPIOVER180       0.017453292519943295769236907684886f
#define glvFIXEDPIOVER180       0x00000478
#define glvFLOATPITIMES2        6.283185307179586476925286766559f
#define glvFIXEDPITIMES2        0x0006487F
#define glvFLOATPIOVER2         1.5707963267948966192313216916398f
#define glvFIXEDPIOVER2         0x00019220

#define glvFLOATONEOVER65535    ((GLfloat) 1.0f / 65535.0f)
#define glvFIXEDONEOVER65535    ((GLfixed) 0x00000001)

#define glvMAXLONG              0x7fffffff
#define glvMINLONG              0x80000000

#define glmFIXEDCLAMP(_x) \
    (((_x) < gcvNEGONE_X) \
        ? gcvNEGONE_X \
        : (((_x) > gcvONE_X) \
            ? gcvONE_X \
            : (_x)))

#define glmFLOATCLAMP(_f) \
    (((_f) < -1.0f) \
        ? -1.0f \
        : (((_f) > 1.0f) \
            ? 1.0f \
            : (_f)))

#define glmBOOL2INT(_b)         ((GLint) (glmINT2BOOL(_b)))
#define glmBOOL2FIXED(_b)       ((GLfixed) (glmINT2BOOL(_b) << 16))
#define glmBOOL2FLOAT(_b)       ((GLfloat) (glmINT2BOOL(_b)))

#define glmINT2BOOL(_i)         ((GLboolean) ((_i)? GL_TRUE : GL_FALSE))
#define glmINT2FIXED(_i)        ((GLfixed) ((_i) << 16))
#define glmINT2FLOAT(_i)        gcoMATH_Int2Float(_i)

#define glmFIXED2BOOL(_x)       ((GLboolean) ((_x)? GL_TRUE : GL_FALSE))
#define glmFIXED2INT(_x)        ((GLint) (((_x) + 0x8000) >> 16))
#define glmFIXED2NORM(_x) \
    ((GLint) \
    ((((GLfixed) (_x)) < 0) \
        ? ((((gctINT64) (glmFIXEDCLAMP(_x))) * (GLint) glvMINLONG) >> 16) \
        : ((((gctINT64) (glmFIXEDCLAMP(_x))) * (GLint) glvMAXLONG) >> 16)))
#define glmFIXED2FLOAT(_x)      gcoMATH_Fixed2Float(_x)

#define glmFLOAT2BOOL(_f)       ((GLboolean) ((_f)? GL_TRUE : GL_FALSE))
#define glmFLOAT2NORM(_f)       ((GLint) (glmFLOATCLAMP(_f) * (GLint) glvMAXLONG))
#define glmFLOAT2INT(_f)        ((GLint) ((_f) + 0.5f))
#define glmFLOAT2FIXED(_f)      ((GLfixed) ((_f) * 65536.0f))


#define glmVEC(Vector, Component) \
    (Vector)->value[Component]

#define glmVECFIXED(Vector, Component) \
    glmVEC(Vector, Component).x

#define glmVECFLOAT(Vector, Component) \
    glmVEC(Vector, Component).f


#define glmMAT(Matrix, X, Y) \
    (Matrix)->value[(Y) + ((X) << 2)]

#define glmMATFIXED(Matrix, X, Y) \
    glmMAT(Matrix, X, Y).x

#define glmMATFLOAT(Matrix, X, Y) \
    glmMAT(Matrix, X, Y).f


#define glmFIXEDCLAMP_NEG1_TO_1(_x) \
    (((_x) < gcvNEGONE_X) \
        ? gcvNEGONE_X \
        : (((_x) > gcvONE_X) \
            ? gcvONE_X \
            : (_x)))

#define glmFLOATCLAMP_NEG1_TO_1(_f) \
    (((_f) < -1.0f) \
        ? -1.0f \
        : (((_f) > 1.0f) \
            ? 1.0f \
            : (_f)))


#define glmFIXEDCLAMP_0_TO_1(_x) \
    (((_x) < 0) \
        ? 0 \
        : (((_x) > gcvONE_X) \
            ? gcvONE_X \
            : (_x)))

#define glmFLOATCLAMP_0_TO_1(_f) \
    (((_f) < 0.0f) \
        ? 0.0f \
        : (((_f) > 1.0f) \
            ? 1.0f \
            : (_f)))


#define glmFIXEDMULTIPLY(x1, x2) \
    gcmXMultiply(x1, x2)

#define glmFLOATMULTIPLY(x1, x2) \
    ((x1) * (x2))


#define glmFIXEDDIVIDE(x1, x2) \
    gcmXDivide(x1, x2)

#define glmFLOATDIVIDE(x1, x2) \
    ((x1) / (x2))


#define glmFIXEDMULTIPLY3(x1, x2, x3) \
    gcmXMultiply(gcmXMultiply(x1, x2), x3)

#define glmFLOATMULTIPLY3(x1, x2, x3) \
    ((x1) * (x2) * (x3))

#define glmFIXEDMULTIPLYDIVIDE(x1, x2, x3) \
    gcmXMultiplyDivide(x1, x2, x3)


#define glmSWAP(Type, Value1, Value2) \
    { \
        Type temp; \
        temp = (Value1); \
        (Value1) = (Value2); \
        (Value2) = temp; \
    }

#define glmABS(Value) \
    (((Value) > 0)? (Value) : -(Value))


#   define glvFRACGLTYPEENUM            GL_FLOAT
#   define glvFRACTYPEENUM              glvFLOAT
#   define gltFRACTYPE                  GLfloat

#   define glvFRACZERO                  ((GLfloat)    0.0f)
#   define glvFRACPOINT2                ((GLfloat)    0.2f)
#   define glvFRACHALF                  ((GLfloat)    0.5f)
#   define glvFRACPOINT8                ((GLfloat)    0.8f)
#   define glvFRACONE                   ((GLfloat)    1.0f)
#   define glvFRACNEGONE                ((GLfloat)   -1.0f)
#   define glvFRACTWO                   ((GLfloat)    2.0f)
#   define glvFRAC90                    ((GLfloat)   90.0f)
#   define glvFRAC128                   ((GLfloat)  128.0f)
#   define glvFRAC180                   ((GLfloat)  180.0f)
#   define glvFRAC256                   ((GLfloat)  256.0f)
#   define glvFRAC4096                  ((GLfloat) 4096.0f)
#   define glvFRACONEOVER255            ((GLfloat) 1.0f / 255.0f)
#   define glvFRACONEOVER65535          glvFLOATONEOVER65535

#   define glvFRACPI                    ((GLfloat) glvFLOATPI)
#   define glvFRACPIOVER180             ((GLfloat) glvFLOATPIOVER180)
#   define glvFRACPITIMES2              ((GLfloat) glvFLOATPITIMES2)
#   define glvFRACPIOVER2               ((GLfloat) glvFLOATPIOVER2)

#   define gcSHADER_FRAC_X1             gcSHADER_FLOAT_X1
#   define gcSHADER_FRAC_X2             gcSHADER_FLOAT_X2
#   define gcSHADER_FRAC_X3             gcSHADER_FLOAT_X3
#   define gcSHADER_FRAC_X4             gcSHADER_FLOAT_X4

#   define glmBOOL2FRAC                 glmBOOL2FLOAT
#   define glmINT2FRAC                  glmINT2FLOAT
#   define glmFIXED2FRAC                glmFIXED2FLOAT
#   define glmFLOAT2FRAC
#   define glmFRAC2BOOL                 glmFLOAT2BOOL
#   define glmFRAC2INT                  glmFLOAT2INT
#   define glmFRAC2NORM                 glmFLOAT2NORM
#   define glmFRACVEC                   glmVECFLOAT
#   define glmFRACMAT                   glmMATFLOAT
#   define glmFRAC2FLOAT

#   define glmFRACMULTIPLY              glmFLOATMULTIPLY
#   define glmFRACDIVIDE                glmFLOATDIVIDE
#   define glmFRACMULTIPLY3             glmFLOATMULTIPLY3



/******************************************************************************\
***************** Value Conversion From/To OpenGL Enumerations *****************
\******************************************************************************/

gceSTATUS
gcChipUtilConvertGLEnum(
    const GLenum* Names,
    GLint NameCount,
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    );

gceSTATUS
gcChipUtilConvertGLboolean(
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    );


/******************************************************************************\
***************************** Value Type Converters ****************************
\******************************************************************************/

void
gcChipUtilGetFromBool(
    GLboolean Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromBoolArray(
    const GLboolean* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromInt(
    GLint Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromIntArray(
    const GLint* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromFixed(
    GLfixed Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromFixedArray(
    const GLfixed* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromFloat(
    GLfloat Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromEnum(
    GLenum Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromEnumArray(
    const GLenum* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromMutable(
    gluMUTABLE Variable,
    gleTYPE VariableType,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromMutableArray(
    const gluMUTABLE_PTR Variables,
    gleTYPE VariableType,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromMutant(
    const glsMUTANT_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromMutantArray(
    const glsMUTANT_PTR Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromVector3(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromVector4(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilGetFromMatrix(
    const glsMATRIX_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );


/******************************************************************************\
**************************** Get Values From Raw Input *************************
\******************************************************************************/

GLfixed
gcChipUtilFixedFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    );

GLfloat
gcChipUtilFloatFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    );


/******************************************************************************\
**************************** Get Values From Mutables **************************
\******************************************************************************/

GLboolean
gcChipUtilBoolFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    );

GLint
gcChipUtilIntFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    );

GLfixed
gcChipUtilFixedFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    );

GLfloat
gcChipUtilFloatFromMutable(
    gluMUTABLE Variable,
    gleTYPE Type
    );


/******************************************************************************\
**************************** Get Values From Mutants ***************************
\******************************************************************************/

GLboolean
gcChipUtilBoolFromMutant(
    const glsMUTANT_PTR Variable
    );

GLint
gcChipUtilIntFromMutant(
    const glsMUTANT_PTR Variable
    );

GLfixed
gcChipUtilFixedFromMutant(
    const glsMUTANT_PTR Variable
    );

GLfloat
gcChipUtilFloatFromMutant(
    const glsMUTANT_PTR Variable
    );


/******************************************************************************\
***************************** Set Values To Mutants ****************************
\******************************************************************************/

void
gcChipUtilSetIntMutant(
    glsMUTANT_PTR Variable,
    GLint Value
    );

void
gcChipUtilSetFixedMutant(
    glsMUTANT_PTR Variable,
    GLfixed Value
    );

void
gcChipUtilSetFloatMutant(
    glsMUTANT_PTR Variable,
    GLfloat Value
    );

void
gcChipUtilSetMutant(
    glsMUTANT_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilSetClampedMutant(
    glsMUTANT_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );


/******************************************************************************\
***************************** Set Values To Vectors ****************************
\******************************************************************************/

void
gcChipUtilSetIntVector4(
    glsVECTOR_PTR Variable,
    GLint X,
    GLint Y,
    GLint Z,
    GLint W
    );

void
gcChipUtilSetFixedVector4(
    glsVECTOR_PTR Variable,
    GLfixed X,
    GLfixed Y,
    GLfixed Z,
    GLfixed W
    );

void
gcChipUtilSetFloatVector4(
    glsVECTOR_PTR Variable,
    GLfloat X,
    GLfloat Y,
    GLfloat Z,
    GLfloat W
    );

void
gcChipUtilSetVector3(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilSetVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilSetClampedVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );

void
gcChipUtilSetHomogeneousVector4(
    glsVECTOR_PTR Variable,
    const GLvoid* Value,
    gleTYPE Type
    );

/******************************************************************************\
*********************** Operations on Mutants and Vectors **********************
\******************************************************************************/

void
gcChipUtilCosMutant(
    const glsMUTANT_PTR Variable,
    glsMUTANT_PTR Result
    );

void
gcChipUtilNorm3Vector4f(
    const GLfloat * Vector,
    GLfloat * Result
    );

void
gcChipUtilHomogeneousVector4(
    const glsVECTOR_PTR Variable,
    glsVECTOR_PTR Result
    );


#ifdef OPENGL40
GLboolean glfConvertGLEnum(
    const GLenum* Names,
    GLint NameCount,
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    );

GLboolean glfConvertGLboolean(
    const GLvoid* Value,
    gleTYPE Type,
    GLuint* Result
    );
#endif

/******************************************************************************\
**************************** Error checking macros. ****************************
\******************************************************************************/

#define glmIS_SUCCESS(func) \
    ((func) == GL_NO_ERROR)


#define glmTRANSLATEHALSTATUS(func) \
    ( \
        gcmIS_SUCCESS(func) \
            ? GL_NO_ERROR \
            : GL_INVALID_OPERATION \
    )

#define glmTRANSLATEGLRESULT(func) \
    ( \
        glmIS_SUCCESS(func) \
            ? gcvSTATUS_OK \
            : gcvSTATUS_GENERIC_IO \
    )



/******************************************************************************\
**************** Typedefs need to be refered by precursor defines **************
\******************************************************************************/
typedef struct __GLchipSLUniformRec __GLchipSLUniform;
typedef struct __GLchipSLProgramRec __GLchipSLProgram;
typedef struct __GLchipVertexBufferInfoRec  __GLchipVertexBufferInfo;

#ifdef __cplusplus
}
#endif

#endif /* __chip_base_h__ */
