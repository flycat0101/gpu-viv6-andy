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



#ifndef __chip_basic_defines_h_
#define __chip_basic_defines_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
********************** Function parameter dumping macros. **********************
\******************************************************************************/

#define glmARGINT   "%d"
#define glmARGUINT  "%u"
#define glmARGENUM  "0x%04X"
#define glmARGFIXED "0x%08X"
#define glmARGHEX   "0x%08X"
#define glmARGPTR   "0x%x"
#define glmARGFLOAT "%1.4f"

#define glmERROR(result) \
{ \
    GLenum lastResult = result;  \
    \
    if (glmIS_ERROR(lastResult)) \
    { \
        glsCONTEXT * _context = GetCurrentContext(); \
        gcmTRACE( \
            gcvLEVEL_ERROR, \
            "glmERROR: result=0x%04X @ %s(%d)", \
            lastResult, __FUNCTION__, __LINE__ \
            ); \
        \
        if (_context && glmIS_SUCCESS(_context->error)) \
        {  \
            _context->error = lastResult; \
        } \
    } \
} \
do { } while (gcvFALSE)


#define __glmDOENTER() \
    do \
    { \
        context = GetCurrentContext(); \
        glmGetApiStartTime(); \
        \
        if (context == gcvNULL) \
        { \
            break; \
        }

#define glmENTER() \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER(); \
    \
    __glmDOENTER()

#define glmENTER1( \
    Type, Value \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value "=" Type, Value); \
    \
    __glmDOENTER()

#define glmENTER2( \
    Type1, Value1, \
    Type2, Value2 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2, \
                  Value1, Value2); \
    \
    __glmDOENTER()

#define glmENTER3( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3, \
                  Value1, Value2, Value3); \
    \
    __glmDOENTER()

#define glmENTER4( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4, \
                  Value1, Value2, Value3, Value4); \
    \
    __glmDOENTER()

#define glmENTER5( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4, \
    Type5, Value5 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4 \
                  " " #Value5 "=" Type5, \
                  Value1, Value2, Value3, Value4, Value5); \
    \
    __glmDOENTER()

#define glmENTER6( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4, \
    Type5, Value5, \
    Type6, Value6 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4 \
                  " " #Value5 "=" Type5 \
                  " " #Value6 "=" Type6, \
                  Value1, Value2, Value3, Value4, Value5, Value6); \
    \
    __glmDOENTER()

#define glmENTER7( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4, \
    Type5, Value5, \
    Type6, Value6, \
    Type7, Value7 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4 \
                  " " #Value5 "=" Type5 \
                  " " #Value6 "=" Type6 \
                  " " #Value7 "=" Type7, \
                  Value1, Value2, Value3, Value4, Value5, Value6, Value7); \
    \
    __glmDOENTER()

#define glmENTER8( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4, \
    Type5, Value5, \
    Type6, Value6, \
    Type7, Value7, \
    Type8, Value8 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4 \
                  " " #Value5 "=" Type5 \
                  " " #Value6 "=" Type6 \
                  " " #Value7 "=" Type7 \
                  " " #Value8 "=" Type8, \
                  Value1, Value2, Value3, Value4, Value5, Value6, Value7, \
                  Value8); \
    \
    __glmDOENTER()

#define glmENTER9( \
    Type1, Value1, \
    Type2, Value2, \
    Type3, Value3, \
    Type4, Value4, \
    Type5, Value5, \
    Type6, Value6, \
    Type7, Value7, \
    Type8, Value8, \
    Type9, Value9 \
    ) \
    glsCHIPCONTEXT_PTR  context; \
    glmDeclareTimeVar() \
    \
    gcmHEADER_ARG(#Value1 "=" Type1 \
                  " " #Value2 "=" Type2 \
                  " " #Value3 "=" Type3 \
                  " " #Value4 "=" Type4 \
                  " " #Value5 "=" Type5 \
                  " " #Value6 "=" Type6 \
                  " " #Value7 "=" Type7 \
                  " " #Value8 "=" Type8 \
                  " " #Value9 "=" Type9, \
                  Value1, Value2, Value3, Value4, Value5, Value6, Value7, \
                  Value8, Value9); \
    \
    __glmDOENTER()

#define glmLEAVE() \
    } \
    while (GL_FALSE); \
    glmGetApiEndTime(context);\
    gcmFOOTER_ARG("error=0x%04X", (context == gcvNULL) ? 0 \
                                                       : context->error) \


/******************************************************************************\
**************************** Error checking macros. ****************************
\******************************************************************************/

#define glmIS_ERROR(func) \
    ((func) != GL_NO_ERROR)

#define glmIS_SUCCESS(func) \
    ((func) == GL_NO_ERROR)

#define glmERR_BREAK(func) \
    result = func; \
    if (glmIS_ERROR(result)) \
    { \
        break; \
    }

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
************************ Debug zones (28 are available). ***********************
\******************************************************************************/

#define glvZONE_BUFFER          (gcvZONE_API_ES11 | (1 <<  0))
#define glvZONE_CLEAR           (gcvZONE_API_ES11 | (1 <<  1))
#define glvZONE_CLIP            (gcvZONE_API_ES11 | (1 <<  2))
#define glvZONE_CONTEXT         (gcvZONE_API_ES11 | (1 <<  3))
#define glvZONE_DRAW            (gcvZONE_API_ES11 | (1 <<  4))
#define glvZONE_ENABLE          (gcvZONE_API_ES11 | (1 <<  5))
#define glvZONE_EXTENTION       (gcvZONE_API_ES11 | (1 <<  6))
#define glvZONE_FOG             (gcvZONE_API_ES11 | (1 <<  7))
#define glvZONE_FRAGMENT        (gcvZONE_API_ES11 | (1 <<  8))
#define glvZONE_LIGHT           (gcvZONE_API_ES11 | (1 <<  9))
#define glvZONE_MATRIX          (gcvZONE_API_ES11 | (1 << 10))
#define glvZONE_PIXEL           (gcvZONE_API_ES11 | (1 << 11))
#define glvZONE_POLIGON         (gcvZONE_API_ES11 | (1 << 12))
#define glvZONE_LINE            (gcvZONE_API_ES11 | (1 << 13))
#define glvZONE_QUERY           (gcvZONE_API_ES11 | (1 << 14))
#define glvZONE_TEXTURE         (gcvZONE_API_ES11 | (1 << 15))
#define glvZONE_STATES          (gcvZONE_API_ES11 | (1 << 16))
#define glvZONE_STREAM          (gcvZONE_API_ES11 | (1 << 17))
#define glvZONE_VIEWPORT        (gcvZONE_API_ES11 | (1 << 18))
#define glvZONE_SHADER          (gcvZONE_API_ES11 | (1 << 19))
#define glvZONE_HASH            (gcvZONE_API_ES11 | (1 << 20))
#define glvZONE_TRACE           (gcvZONE_API_ES11 | (1 << 21))

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
#define gldPIOVER180            0.017453292519943295769236907684886f

/* Shader uniform index definition. */
#define glmUNIFORM_INDEX(Shader, Name) \
    glvUNIFORM_ ## Shader ## _ ## Name

/* Shader attribute index definition. */
#define glmATTRIBUTE_INDEX(Shader, Name) \
    glvATTRIBUTE_ ## Shader ## _ ## Name

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
**************************** Shader generation macros. *************************
\******************************************************************************/

#define glmUSING_UNIFORM(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_ATTRIBUTE(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_INDEXED_ATTRIBUTE(Name, Index) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl, Index))

#define glmUSING_VARYING(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_INDEXED_VARYING(Name, Index) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl, Index))

#define glmDEFINE_TEMP(Name) \
    gctUINT16 Name

#define glmALLOCATE_LOCAL_TEMP(Name) \
    glmDEFINE_TEMP(Name) = allocateTemp(ShaderControl)

#define glmALLOCATE_TEMP(Name) \
    Name = allocateTemp(ShaderControl)

#define glmDEFINE_LABEL(Name) \
    gctUINT Name

#define glmALLOCATE_LOCAL_LABEL(Name) \
    glmDEFINE_LABEL(Name) = allocateLabel(ShaderControl)

#define glmALLOCATE_LABEL(Name) \
    Name = allocateLabel(ShaderControl)

#define glmASSIGN_OUTPUT(Name) \
    if (ShaderControl->Name != 0) \
    { \
        gcmERR_BREAK(assign_##Name( \
            gc, \
            ShaderControl, \
            ShaderControl->Name \
            )); \
    }

#define glmASSIGN_INDEXED_OUTPUT(Name, Index) \
    if (ShaderControl->Name[Index] != 0) \
    { \
        gcmERR_BREAK(assign_##Name( \
            gc, \
            ShaderControl, \
            Index \
            )); \
    }

#define glmOPCODE(Opcode, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        TempRegister, \
        gcSL_ENABLE_##ComponentEnable, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmOPCODE_COND(Opcode, Condition, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode2( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        gcSL_##Condition, \
        TempRegister, \
        gcSL_ENABLE_##ComponentEnable, \
        gcSL_FLOAT \
        ))

#define glmOPCODEV(Opcode, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmASSERT((ComponentEnable & ~gcSL_ENABLE_XYZW) == 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        TempRegister, \
        ComponentEnable, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmOPCODE_BRANCH(Opcode, Condition, Target) \
    gcmERR_BREAK(gcSHADER_AddOpcodeConditional( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        gcSL_##Condition, \
        Target \
        ))

#define glmCONST(Value) \
    gcmERR_BREAK(gcSHADER_AddSourceConstant( \
        ShaderControl->i->shader, \
        (gctFLOAT) (Value) \
        ))

#define glmUNIFORM_WRAP(Shader, Name) \
        ShaderControl->uniforms[glmUNIFORM_INDEX(Shader, Name)]

#define glmUNIFORM_WRAP_INDEXED(Shader, Name, Index) \
        ShaderControl->uniforms[glmUNIFORM_INDEX(Shader, Name) + Index]

#define glmUNIFORM(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmUNIFORM_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP_INDEXED(Shader, Name, Index)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmUNIFORM_STATIC(Shader, Name, Swizzle, Index) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        Index \
        ))

#define glmUNIFORM_DYNAMIC(Shader, Name, Swizzle, IndexRegister) \
    gcmERR_BREAK(gcSHADER_AddSourceUniformIndexed( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0, \
        gcSL_INDEXED_X, \
        IndexRegister \
        ))

#define glmUNIFORM_DYNAMIC_MATRIX(Shader, Name, Swizzle, IndexRegister, \
                                  Offset, IndexMode) \
    gcmERR_BREAK(gcSHADER_AddSourceUniformIndexed( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        Offset, \
        IndexMode, \
        IndexRegister \
        ))

#define glmATTRIBUTE_WRAP(Shader, Name) \
        ShaderControl->attributes[glmATTRIBUTE_INDEX(Shader, Name)]

#define glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index) \
        ShaderControl->attributes[glmATTRIBUTE_INDEX(Shader, Name) + Index]

#define glmATTRIBUTE(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmATTRIBUTE_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmATTRIBUTEV(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        Swizzle, \
        0 \
        ))

#define glmVARYING(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmVARYING_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmVARYINGV_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        Swizzle, \
        0 \
        ))

#define glmTEMP(TempRegister, Swizzle) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddSource( \
        ShaderControl->i->shader, \
        gcSL_TEMP, \
        TempRegister, \
        gcSL_SWIZZLE_##Swizzle, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmTEMPV(TempRegister, Swizzle) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddSource( \
        ShaderControl->i->shader, \
        gcSL_TEMP, \
        TempRegister, \
        Swizzle, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmADD_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_AddFunction( \
        ShaderControl->i->shader, \
        #FunctionName, \
        &ShaderControl->FunctionName \
        ))

#define glmBEGIN_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_BeginFunction( \
        ShaderControl->i->shader, \
        ShaderControl->FunctionName \
        ))

#define glmEND_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_EndFunction( \
        ShaderControl->i->shader, \
        ShaderControl->FunctionName \
        ))

#define glmRET() \
    gcmERR_BREAK(gcSHADER_AddOpcodeConditional( \
        ShaderControl->i->shader, \
        gcSL_RET, \
        gcSL_ALWAYS, \
        0 \
        ))

#define glmLABEL(Label) \
    gcmERR_BREAK(gcSHADER_AddLabel( \
        ShaderControl->i->shader, \
        Label \
        ))

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

/******************************************************************************\
********************************** Point states. *******************************
\******************************************************************************/

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


/******************************************************************************\
******************************* Multisample states. ****************************
\******************************************************************************/

typedef struct _glsMULTISAMPLE * glsMULTISAMPLE_PTR;
typedef struct _glsMULTISAMPLE
{
    GLboolean               enabled;
    glsMUTANT               coverageValue;
    GLboolean               coverageInvert;
    GLboolean               coverage;
    GLboolean               alphaToCoverage;
    GLboolean               alphaToOne;
}
glsMULTISAMPLE;


/******************************************************************************\
******************************* Multisample states. ****************************
\******************************************************************************/

typedef struct _glsLOGICOP * glsLOGICOP_PTR;
typedef struct _glsLOGICOP
{
    GLboolean               enabled;
    GLenum                  operation;
    GLboolean               perform;
    gctUINT8                rop;
}
glsLOGICOP;


/******************************************************************************\
******************** Input attribute (stream) type definition. *****************
\******************************************************************************/

typedef struct _glsATTRIBUTEINFO * glsATTRIBUTEINFO_PTR;
typedef struct _glsATTRIBUTEINFO
{
    /* Current value. */
    glsVECTOR               currValue;
    GLboolean               dirty;

    /* Stream. */
    GLboolean               streamEnabled;
    gceVERTEX_FORMAT        format;
    GLboolean               normalize;
    GLuint                  components;
    gcSHADER_TYPE           attributeType;
    gcSHADER_TYPE           varyingType;
    gcSL_SWIZZLE            varyingSwizzle;
    GLsizei                 stride;
    GLsizei                 attributeSize;
    gctCONST_POINTER        pointer;
}
glsATTRIBUTEINFO;


/******************************************************************************\
************************** Shader container definitions. ***********************
\******************************************************************************/

typedef gceSTATUS (*glfUNIFORMSET) (
    __GLcontext *,
    gcUNIFORM
    );

typedef struct _glsUNIFORMWRAP * glsUNIFORMWRAP_PTR;
typedef struct _glsUNIFORMWRAP
{
    gcUNIFORM               uniform;
    glfUNIFORMSET           set;
}
glsUNIFORMWRAP;

typedef struct _glsATTRIBUTEWRAP * glsATTRIBUTEWRAP_PTR;
typedef struct _glsATTRIBUTEWRAP
{
    gcATTRIBUTE             attribute;
    glsATTRIBUTEINFO_PTR    info;
    gctINT                  binding;
}
glsATTRIBUTEWRAP;

typedef struct _glsSHADERCONTROL * glsSHADERCONTROL_PTR;
typedef struct _glsSHADERCONTROL
{
    gcSHADER                shader;
    glsUNIFORMWRAP_PTR      uniforms;
    glsATTRIBUTEWRAP_PTR    attributes;
    glsUNIFORMWRAP_PTR      texture[glvMAX_TEXTURES];
}
glsSHADERCONTROL;


/******************************************************************************\
*************************** Shader generation helpers. *************************
\******************************************************************************/

gceSTATUS glfUsingUniform(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN glfUNIFORMSET UniformSet,
    IN glsUNIFORMWRAP_PTR* UniformWrap
    );

gceSTATUS glfUsingAttribute(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEINFO_PTR AttributeInfo,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gctINT Binding,
    IN gcSHADER_SHADERMODE ShadingMode
    );

gceSTATUS glfUsingVarying(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gcSHADER_SHADERMODE ShadingMode
    );

gceSTATUS glfUsing_uTexCoord(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    );


/******************************************************************************\
***************************** Shader generation API. ***************************
\******************************************************************************/
gceSTATUS glfGenerateVSFixedFunction(
    IN __GLcontext * gc
    );

gceSTATUS glfGenerateFSFixedFunction(
    IN __GLcontext * gc
    );

#ifdef __cplusplus
}
#endif

#endif /* __chip_basic_defines_h_ */
