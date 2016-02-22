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


#ifndef __gc_glsl_compiler_h_
#define __gc_glsl_compiler_h_

/* Dump Options */
#define slmDUMP_OPTIONS        ((sltDUMP_OPTIONS)slvDUMP_NONE)

#define MAX_SINGLE_LOG_LENGTH       1024*32

#include "gc_glsl_common.h"

#define slmIsLanguageVersion3_1(Compiler) \
    (sloCOMPILER_GetLanguageVersion(Compiler) >= _SHADER_ES31_VERSION)

#if gcmIS_DEBUG(gcdDEBUG_ASSERT)

#    define slmVERIFY_OBJECT(obj, objType) \
        do \
        { \
            if (((obj) == gcvNULL) || (((slsOBJECT *)(obj))->type != (objType))) \
            { \
                gcmASSERT(((obj) != gcvNULL) && (((slsOBJECT *)(obj))->type == (objType))); \
                gcmFOOTER_ARG("status=%d(gcvSTATUS_INVALID_OBJECT)", gcvSTATUS_INVALID_OBJECT); \
                return gcvSTATUS_INVALID_OBJECT; \
            } \
        } \
        while (gcvFALSE)

#    define slmASSERT_OBJECT(obj, objType) \
        do \
        { \
            if (((obj) == gcvNULL) || (((slsOBJECT *)(obj))->type != (objType))) \
            { \
                gcmASSERT(((obj) != gcvNULL) && (((slsOBJECT *)(obj))->type == (objType))); \
            } \
        } \
        while (gcvFALSE)

#else

#    define slmVERIFY_OBJECT(obj, objType) do {} while (gcvFALSE)
#    define slmASSERT_OBJECT(obj, objType) do {} while (gcvFALSE)

#endif

/* Optimization options. */
typedef enum _sleOPTIMIZATION_OPTION
{
    slvOPTIMIZATION_NONE                    = 0x0000,
    slvOPTIMIZATION_CALCULATION             = 0x0001,
    slvOPTIMIZATION_UNROLL_ITERATION        = 0x0002,
    slvOPTIMIZATION_DATA_FLOW               = 0x0004,
    slvOPTIMIZATION_SPECIAL                 = 0x0008,
    slvOPTIMIZATION_SHARE_VEC_CONSTANTS     = 0x0010,
    slvOPTIMIZATION_ALL                     = 0xFFFF,
    /* Extra options */
    slvOPTIMIZATION_EXPAND_NORM             = 0x10000
}
sleOPTIMIZATION_OPTION;

typedef gctUINT32                        sltOPTIMIZATION_OPTIONS;

/* Dump options. */
typedef enum _sleDUMP_OPTION
{
    slvDUMP_NONE        = 0x0000,
    slvDUMP_SOURCE        = 0x0001,
    slvDUMP_PREPROCESSOR    = 0x0010,
    slvDUMP_SCANNER        = 0x0100,
    slvDUMP_PARSER        = 0x0200,
    slvDUMP_IR        = 0x0400,
    slvDUMP_CODE_GENERATOR    = 0x1000,
    slvDUMP_CODE_EMITTER    = 0x2000,
    slvDUMP_COMPILER    = 0xff00,
    slvDUMP_ALL        = 0xffff
}
sleDUMP_OPTION;

typedef gctUINT16                sltDUMP_OPTIONS;

/* Type of objects. */
typedef enum _sleOBJECT_TYPE
{
    slvOBJ_UNKNOWN         = 0,

    slvOBJ_COMPILER        = gcmCC('C','M','P','L'),
    slvOBJ_CODE_GENERATOR    = gcmCC('C','G','E','N'),
    slvOBJ_OBJECT_COUNTER    = gcmCC('O','B','J','C'),
    slvOBJ_CODE_EMITTER    = gcmCC('C','E','M','T')
}
sleOBJECT_TYPE;

/* slsOBJECT object defintinon. */
typedef struct _slsOBJECT
{
    /* Type of an object. */
    sleOBJECT_TYPE                type;
}
slsOBJECT;

typedef enum _sleCOMPILER_FLAGS
{
    slvCOMPILER_HAS_UNSPECIFIED_UNIFORM_LOCATION                = 0x0001,
    slvCOMPILER_HAS_UNSPECIFIED_LOCATION                        = 0x0002,
    slvCOMPILER_HAS_EARLY_FRAG_TEST                             = 0x0004,
    slvCOMPILER_HAS_PATCH_FOR_CENTROID_VARYING                  = 0x0008,
}
sleCOMPILER_FLAGS;

#define slsCOMPILER_HasUnspecifiedUniformLocation(f)            (((f) & slvCOMPILER_HAS_UNSPECIFIED_UNIFORM_LOCATION) != 0)
#define slsCOMPILER_SetUnspecifiedUniformLocation(f)            ((f) |= slvCOMPILER_HAS_UNSPECIFIED_UNIFORM_LOCATION)
#define slsCOMPILER_ClrUnspecifiedUniformLocation(f)            ((f) &= ~slvCOMPILER_HAS_UNSPECIFIED_UNIFORM_LOCATION)
#define slsCOMPILER_HasUnspecifiedLocation(f)                   (((f) & slvCOMPILER_HAS_UNSPECIFIED_LOCATION) != 0)
#define slsCOMPILER_SetUnspecifiedLocation(f)                   ((f) |= slvCOMPILER_HAS_UNSPECIFIED_LOCATION)
#define slsCOMPILER_ClrUnspecifiedLocation(f)                   ((f) &= ~slvCOMPILER_HAS_UNSPECIFIED_LOCATION)
#define slsCOMPILER_HasEarlyFragText(f)                         (((f) & slvCOMPILER_HAS_EARLY_FRAG_TEST) != 0)
#define slsCOMPILER_SetEarlyFragText(f)                         ((f) |= slvCOMPILER_HAS_EARLY_FRAG_TEST)
#define slsCOMPILER_ClrEarlyFragText(f)                         ((f) &= ~slvCOMPILER_HAS_EARLY_FRAG_TEST)
#define slsCOMPILER_HasPatchForCentroidVarying(f)               (((f) & slvCOMPILER_HAS_PATCH_FOR_CENTROID_VARYING) != 0)
#define slsCOMPILER_SetPatchForCentroidVarying(f)               ((f) |= slvCOMPILER_HAS_PATCH_FOR_CENTROID_VARYING)
#define slsCOMPILER_ClrPatchForCentroidVarying(f)               ((f) &= ~slvCOMPILER_HAS_PATCH_FOR_CENTROID_VARYING)

struct _sloCOMPILER;
struct _sloIR_BASE;

typedef struct _sloCOMPILER *            sloCOMPILER;

struct _sloCODE_GENERATOR;
typedef struct _sloCODE_GENERATOR *sloCODE_GENERATOR;

struct _sloCODE_EMITTER;

typedef struct _sloCODE_EMITTER *        sloCODE_EMITTER;

/* Construct a new sloCOMPILER object. */
typedef enum _sleSHADER_TYPE
{
    slvSHADER_TYPE_UNKNOWN                   = gcSHADER_TYPE_UNKNOWN,
    slvSHADER_TYPE_VERTEX                    = gcSHADER_TYPE_VERTEX,
    slvSHADER_TYPE_FRAGMENT                  = gcSHADER_TYPE_FRAGMENT,
    slvSHADER_TYPE_COMPUTE                   = gcSHADER_TYPE_COMPUTE,
    slvSHADER_TYPE_PRECOMPILED               = gcSHADER_TYPE_PRECOMPILED,
    slvSHADER_TYPE_VERTEX_DEFAULT_UBO        = gcSHADER_TYPE_VERTEX_DEFAULT_UBO,
    slvSHADER_TYPE_FRAGMENT_DEFAULT_UBO      = gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO,
    slvSHADER_TYPE_TCS                       = gcSHADER_TYPE_TCS,
    slvSHADER_TYPE_TES                       = gcSHADER_TYPE_TES,
    slvSHADER_TYPE_GS                        = gcSHADER_TYPE_GEOMETRY,
    slvSHADER_TYPE_LIBRARY                   = gcSHADER_TYPE_LIBRARY
}
sleSHADER_TYPE;

gceSTATUS
sloCOMPILER_LoadBuiltIns(
    IN sloCOMPILER Compiler,
    IN gctBOOL LoadPrecisionOnly
    );

gctUINT32 *
sloCOMPILER_GetVersion(
    IN sloCOMPILER Compiler,
    IN sleSHADER_TYPE ShaderType
);

gceAPI
sloCOMPILER_GetClientApiVersion(
    IN sloCOMPILER Compiler
);

gceSTATUS
sloCOMPILER_SetLanguageVersion(
    IN sloCOMPILER Compiler,
    IN gctUINT32 LangVersion
    );

gctUINT32
sloCOMPILER_GetLanguageVersion(
    IN sloCOMPILER Compiler
    );

gctBOOL
sloCOMPILER_IsHaltiVersion(
    IN sloCOMPILER Compiler
);

gctBOOL
sloCOMPILER_IsES30Version(
    IN sloCOMPILER Compiler
);

gctBOOL
sloCOMPILER_IsES31VersionOrAbove(
    IN sloCOMPILER Compiler
);

gceSTATUS
sloCOMPILER_SetDebug(
    IN sloCOMPILER Compiler,
    IN gctBOOL    Debug
    );

gceSTATUS
sloCOMPILER_SetOptimize(
    IN sloCOMPILER Compiler,
    IN gctBOOL    Optimize
    );

gceSTATUS
sloCOMPILER_SetOutputInvariant(
    IN sloCOMPILER Compiler,
    IN gctBOOL Flag
    );

gceSTATUS
sloCOMPILER_Construct(
    IN gcoHAL Hal,
    IN sleSHADER_TYPE ShaderType,
    IN gceAPI ClientApiVersion,
    OUT sloCOMPILER * Compiler
    );

/* Destroy an sloCOMPILER object. */
gceSTATUS
sloCOMPILER_Destroy(
    IN sloCOMPILER Compiler
    );

/* Get the shader type */
gceSTATUS
sloCOMPILER_GetShaderType(
    IN sloCOMPILER Compiler,
    OUT sleSHADER_TYPE * ShaderType
    );

/* If this shader use default UBO. */
gceSTATUS
sloCOMPILER_IsCreateDefaultUBO(
    IN sloCOMPILER Compiler,
    OUT gctBOOL * IsCreateDefaultUBO
    );

/* Get the binary */
gceSTATUS
sloCOMPILER_GetBinary(
    IN sloCOMPILER Compiler,
    OUT gcSHADER * Binary
    );

gceSTATUS
sloCOMPILER_GetHAL(
    IN sloCOMPILER Compiler,
    OUT gcoHAL * Hal
    );

/* Compile the source strings */
gceSTATUS
sloCOMPILER_Compile(
    IN sloCOMPILER Compiler,
    IN sltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN sltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    );

/* Get the compiler options */
sltOPTIMIZATION_OPTIONS sloCOMPILER_GetOptions(sloCOMPILER Compiler);

gctBOOL sloCOMPILER_ExpandNorm(sloCOMPILER Compiler);

/* Preprocess the source strings only */
gceSTATUS
sloCOMPILER_Preprocess(
    IN sloCOMPILER Compiler,
    IN sltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN sltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    OUT gctSTRING * Log
    );

/* Set the current line no */
gceSTATUS
sloCOMPILER_SetCurrentLineNo(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo
    );

/* Set the current string no */
gceSTATUS
sloCOMPILER_SetCurrentStringNo(
    IN sloCOMPILER Compiler,
    IN gctUINT StringNo
    );

/* Get the current line no */
gctUINT
sloCOMPILER_GetCurrentLineNo(
    IN sloCOMPILER Compiler
    );

/* Get the current string no */
gctUINT
sloCOMPILER_GetCurrentStringNo(
    IN sloCOMPILER Compiler
    );

/* Allocate memory blocks for the compiler */
gceSTATUS
sloCOMPILER_Allocate(
    IN sloCOMPILER Compiler,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    );

/* Free memory blocks for the compiler */
gceSTATUS
sloCOMPILER_Free(
    IN sloCOMPILER Compiler,
    IN gctPOINTER Memory
    );

gceSTATUS
sloCOMPILER_InsertWClipList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT Data0,
    IN gctINT Data1
    );

gceSTATUS
sloCOMPILER_InsertWClipForUniformList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT Data0,
    IN gctINT Data1
    );

gceSTATUS
sloCOMPILER_FindWClipForUniformList(
    IN sloCOMPILER Compiler,
    IN gctINT Index,
    IN gctINT * UniformIndex1,
    IN gctINT * UniformIndex2
    );

gceSTATUS
sloCOMPILER_GetUniformIndex(
    IN sloCOMPILER Compiler,
    IN gcUNIFORM Uniform,
    IN gctUINT16 * Index
    );

gceSTATUS
sloCOMPILER_GetAttributeIndex(
    IN sloCOMPILER Compiler,
    IN gcATTRIBUTE Attribute,
    IN gctUINT16 * Index
    );

/* Report info */
typedef enum _sleREPORT_TYPE
{
    slvREPORT_FATAL_ERROR         = 0,
    slvREPORT_INTERNAL_ERROR,
    slvREPORT_ERROR,
    slvREPORT_WARN
}
sleREPORT_TYPE;

gceSTATUS
sloCOMPILER_VReport(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    );

gceSTATUS
sloCOMPILER_Report(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN sleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN ...
    );

gceSTATUS
sloCOMPILER_CheckErrorLog(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo
    );

/* Dump info */
gceSTATUS
sloCOMPILER_Dump(
    IN sloCOMPILER Compiler,
    IN sleDUMP_OPTION DumpOption,
    IN gctCONST_STRING Message,
    IN ...
    );

/* Allocate pool string */
typedef gctSTRING        sltPOOL_STRING;

gceSTATUS
sloCOMPILER_AllocatePoolString(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT sltPOOL_STRING * PoolString
    );

/* Extensions */
typedef enum _sleEXTENSION
{
    slvEXTENSION_NONE                                       = 0x00000000,

    slvEXTENSION_STANDARD_DERIVATIVES                       = 0x00000001,
    slvEXTENSION_TEXTURE_3D                                 = 0x00000002,
    slvEXTENSION_TEXTURE_ARRAY                              = 0x00000004,
    slvEXTENSION_FRAG_DEPTH                                 = 0x00000008,
    slvEXTENSION_EGL_IMAGE_EXTERNAL                         = 0x00000010,
    slvEXTENSION_HALTI                                      = 0x00000020,
    slvEXTENSION_NON_HALTI                                  = 0x00000040,
    slvEXTENSION_SHADOW_SAMPLER                             = 0x00000080,
    slvEXTENSION_BLEND_EQUATION_ADVANCED                    = 0x00000100,
    slvEXTENSION_ES_31                                      = 0x00000200,
    slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY       = 0x00000400,
    slvEXTENSION_VASM                                       = 0x00000800,
    slvEXTENSION_IMAGE_ATOMIC                               = 0x00001000,
    slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY                     = 0x00002000,
    slvEXTENSION_IO_BLOCKS                                  = 0x00004000,
    slvEXTENSION_GPU_SHADER5                                = 0x00008000,
    slvEXTENSION_TESSELLATION_SHADER                        = 0x00010000 | slvEXTENSION_IO_BLOCKS | slvEXTENSION_GPU_SHADER5,
    slvEXTENSION_TESSELLATION_POINT_SIZE                    = 0x00020000 | slvEXTENSION_IO_BLOCKS,
    slvEXTENSION_SAMPLE_VARIABLES                           = 0x00040000,
    slvEXTENSION_SHADER_MULTISAMPLE_INTERPOLATION           = 0x00080000,
    slvEXTENSION_EXT_GEOMETRY_SHADER                        = 0x00100000 | slvEXTENSION_IO_BLOCKS,
    slvEXTENSION_EXT_GEOMETRY_POINT_SIZE                    = 0x00200000 | slvEXTENSION_IO_BLOCKS,
    slvEXTENSION_EXT_SHADER_IMPLICIT_CONVERSIONS            = 0x00400000,
    slvEXTENSION_EXT_TEXTURE_BUFFER                         = 0x00800000,
    slvEXTENSION_EXT_PRIMITIVE_BOUNDING_BOX                 = 0x01000000,

    slvEXTENSION_ES_32                                      = 0x02000000,

    slvEXTENSION_ES_30_AND_ABOVE                            = slvEXTENSION_HALTI |
                                                              slvEXTENSION_ES_31 |
                                                              slvEXTENSION_ES_32,

    slvEXTENSION_ALL                                        = slvEXTENSION_STANDARD_DERIVATIVES |
                                                              slvEXTENSION_TEXTURE_3D |
                                                              slvEXTENSION_TEXTURE_ARRAY |
                                                              slvEXTENSION_FRAG_DEPTH |
                                                              slvEXTENSION_EGL_IMAGE_EXTERNAL |
                                                              slvEXTENSION_SHADOW_SAMPLER |
                                                              slvEXTENSION_BLEND_EQUATION_ADVANCED |
                                                              slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY |
                                                              slvEXTENSION_IMAGE_ATOMIC
}
sleEXTENSION;

typedef gctUINT32                sltEXTENSIONS;

/* Enable extension */
gceSTATUS
sloCOMPILER_EnableExtension(
    IN sloCOMPILER Compiler,
    IN sleEXTENSION Extension,
    IN gctBOOL Enable
    );

gceSTATUS
sloCOMPILER_BackPatch(
    IN sloCOMPILER Compiler,
    IN sloCODE_GENERATOR CodeGenerator
    );

gceSTATUS
sloCOMPILER_PackUniformsWithSharedOrStd140(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_PackSSBOWithSharedOrStd140OrStd430(
    IN sloCOMPILER Compiler
    );

gceSTATUS
sloCOMPILER_CheckAssignmentForGlFragData(
    IN sloCOMPILER Compiler
    );

gctBOOL
sloCOMPILER_IsShaderType(
   IN sloCOMPILER Compiler,
   IN sleSHADER_TYPE ShaderType
   );

gceSTATUS
sloCOMPILER_CompileBuiltinLibrary(
    IN sloCOMPILER Compiler,
    IN sltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN sltDUMP_OPTIONS DumpOptions,
    IN gcSHADER  Shader,
    OUT gcSHADER *Binary,
    OUT gctSTRING *Log
    );

gceSTATUS
sloCOMPILER_LinkBuiltinLibrary(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library
    );

gceSTATUS
sloCOMPILER_GetCurrentIterationCount(
    IN sloCOMPILER Compiler,
    OUT gctINT * CurrentIterationCount
    );

gceSTATUS
sloCOMPILER_SetCurrentIterationCount(
    IN sloCOMPILER Compiler,
    IN gctINT CurrentIterationCount
    );

gceSTATUS
sloCOMPILER_SetCompilerFlag(
    IN sloCOMPILER Compiler,
    IN sleCOMPILER_FLAGS Flag
    );

gceSTATUS
sloCOMPILER_GetCompilerFlag(
    IN sloCOMPILER Compiler,
    OUT sleCOMPILER_FLAGS * Flag
    );

gcePATCH_ID
sloCOMPILER_GetPatchID(
    IN sloCOMPILER Compiler
    );

#endif /* __gc_glsl_compiler_h_ */
