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


#ifndef __gc_glff_basic_types_h_
#define __gc_glff_basic_types_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
********************************* Mutable Types ********************************
\******************************************************************************/
#if defined(_WIN32)
#define __GL_INLINE static __forceinline
#else
#define __GL_INLINE static __inline
#endif

typedef enum _gleTYPE
{
    glvBOOL,
    glvINT,
    glvNORM,
    glvFIXED,
    glvFLOAT
}
gleTYPE, * gleTYPE_PTR;

typedef struct _glsVECTOR * glsVECTOR_PTR;
typedef struct _glsVECTOR
{
    /* Mutable values (must be at the beginning of the structure). */
    GLfloat                value[4];

    /* Special value flags. */
    GLboolean               zero3;
    GLboolean               zero4;
    GLboolean               one3;
    GLboolean               one4;
}
glsVECTOR;

typedef struct _glsMATRIX * glsMATRIX_PTR;
typedef struct _glsMATRIX
{
    /* Matrix mutable values. */
    GLfloat                    value[16];

    /* Special value flags. */
    GLboolean               identity;
}
glsMATRIX;


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
#define glvFLOATNEGONE          -1.0f
#define glvFLOATZERO            0.0f
#define glvFLOATPOINT2          0.2f
#define glvFLOATHALF            0.5f
#define glvFLOATPOINT8            0.8f
#define glvFLOATONE                1.0f
#define glvFLOATTWO                2.0f
#define glvFLOAT90              90.0f
#define glvFLOAT128                128.0f
#define glvFLOAT180                180.0f
#define glvFLOAT256                256.0f
#define glvFLOATONEOVER255      (1.0f / 255.0f)
#define glvFLOATONEOVER65535    (1.0f / 65535.0f)
#define glvFLOATPI              3.141592653589793238462643383279502f
#define glvFLOATPIOVER180       0.017453292519943295769236907684886f
#define glvFLOATPITIMES2        6.283185307179586476925286766559f
#define glvFLOATPIOVER2         1.5707963267948966192313216916398f

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

#define glfISZERO(_f)            (((_f) == glvFLOATZERO) ? GL_TRUE : GL_FALSE)
#define glfISONE(_f)            (((_f) == glvFLOATONE) ? GL_TRUE : GL_FALSE)

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
#define glmFLOAT2INT(_f)        (((_f) >= 0) ? ((GLint) ((_f) + 0.5f)) : ((GLint) ((_f) - 0.5f)))
#define glmFLOAT2FIXED(_f)      ((GLfixed) ((_f) * 65536.0f))


#define glmVEC(Vector, Component) \
    (Vector)->value[Component]

#define glmVECFIXED(Vector, Component) \
    glmVEC(Vector, Component).x

#define glmVECFLOAT(Vector, Component) \
    glmVEC(Vector, Component)


#define glmMAT(Matrix, X, Y) \
    (Matrix)->value[(Y) + ((X) << 2)]

#define glmMATFIXED(Matrix, X, Y) \
    glmMAT(Matrix, X, Y).x

#define glmMATFLOAT(Matrix, X, Y) \
    glmMAT(Matrix, X, Y)


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

#   define gco3D_SetClearDepth          gco3D_SetClearDepthF
#   define gco3D_SetDepthScaleBias      gco3D_SetDepthScaleBiasF
#   define gco3D_SetDepthRange          gco3D_SetDepthRangeF
#   define gco3D_SetClearColorFrac      gco3D_SetClearColorF
#   define gco3D_SetFragmentColor       gco3D_SetFragmentColorF
#   define gco3D_SetFogColor            gco3D_SetFogColorF
#   define gco3D_SetTetxureColor        gco3D_SetTetxureColorF
#   define gcUNIFORM_SetFracValue       gcUNIFORM_SetValueF_Ex

/******************************************************************************\
***************** Value Conversion From/To OpenGL Enumerations *****************
\******************************************************************************/

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


/******************************************************************************\
***************************** Value Type Converters ****************************
\******************************************************************************/

void glfGetFromBool(
    GLboolean Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromBoolArray(
    const GLboolean* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromInt(
    GLint Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromIntArray(
    const GLint* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromFixed(
    GLfixed Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromFixedArray(
    const GLfixed* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromFloat(
    GLfloat Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFloatFromFloatArray(
    const GLfloat* Variables,
    GLint Count,
    GLfloat* Value
    );

void glfGetFromEnum(
    GLenum Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromEnumArray(
    const GLenum* Variables,
    GLint Count,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFromVector3(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFloatFromVector3(
    const glsVECTOR_PTR Variable,
    GLfloat* Value
    );

void glfGetFromVector4(
    const glsVECTOR_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFloatFromVector4(
    const glsVECTOR_PTR Variable,
    GLfloat* Value
    );

void glfGetFromMatrix(
    const glsMATRIX_PTR Variable,
    GLvoid* Value,
    gleTYPE Type
    );

void glfGetFloatFromMatrix(
    const glsMATRIX_PTR Variable,
    GLfloat* Value
    );

/******************************************************************************\
**************************** Get Values From Raw Input *************************
\******************************************************************************/

GLfixed glfFixedFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    );

GLfloat glfFloatFromRaw(
    const GLvoid* Variable,
    gleTYPE Type
    );

/******************************************************************************\
***************************** Set Values To Vectors ****************************
\******************************************************************************/

void glfSetIntVector4(
    glsVECTOR_PTR Variable,
    GLint X,
    GLint Y,
    GLint Z,
    GLint W
    );

void glfSetFixedVector4(
    glsVECTOR_PTR Variable,
    GLfixed X,
    GLfixed Y,
    GLfixed Z,
    GLfixed W
    );

void glfSetFloatVector4(
    glsVECTOR_PTR Variable,
    GLfloat X,
    GLfloat Y,
    GLfloat Z,
    GLfloat W
    );

void glfSetVector3(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    );

void glfSetVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    );

void glfSetClampedVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    );

void glfSetHomogeneousVector4(
    glsVECTOR_PTR Variable,
    const GLfloat* Value
    );

/******************************************************************************\
*********************** Operations on Mutants and Vectors **********************
\******************************************************************************/

void glfCos(
    const GLfloat* Variable,
    GLfloat* Result
    );

void glfAddVector4(
    const glsVECTOR_PTR Variable1,
    const glsVECTOR_PTR Variable2,
    glsVECTOR_PTR Result
    );

void glfMulVector4(
    const glsVECTOR_PTR Variable1,
    const glsVECTOR_PTR Variable2,
    glsVECTOR_PTR Result
    );

void glfNorm3Vector4(
    const glsVECTOR_PTR Variable,
    glsVECTOR_PTR Result
    );

void glfNorm3Vector4f(
    const glsVECTOR_PTR Variable,
    glsVECTOR_PTR Result
    );

void glfHomogeneousVector4(
    const glsVECTOR_PTR Variable,
    glsVECTOR_PTR Result
    );


/******************************************************************************\
**************** Debug Printing for Mutants, Vectors and Matrices **************
\******************************************************************************/

#if gcmIS_DEBUG(gcdDEBUG_TRACE)

void glfPrintMatrix3x3(
    gctSTRING Name,
    glsMATRIX_PTR Matrix
    );

void glfPrintMatrix4x4(
    gctSTRING Name,
    glsMATRIX_PTR Matrix
    );

void glfPrintVector3(
    gctSTRING Name,
    glsVECTOR_PTR Vector
    );

void glfPrintVector4(
    gctSTRING Name,
    glsVECTOR_PTR Vector
    );

#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_glff_basic_types_h_ */
