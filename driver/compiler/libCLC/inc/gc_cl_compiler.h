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


#ifndef __gc_cl_compiler_h_
#define __gc_cl_compiler_h_

/* Dump Options */
#define clmDUMP_OPTIONS        ((cltDUMP_OPTIONS)clvDUMP_NONE)

#include "gc_cl_common.h"
#include "debug/gc_vsc_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define cldSupportOpenCLAtomicExtension 1
#define cldSupportOpenCLKhrFP16Extension  1

/* floating point capability */
#define cldFpDENORM                                (1 << 0)
#define cldFpINF_NAN                               (1 << 1)
#define cldFpROUND_TO_NEAREST                      (1 << 2)
#define cldFpROUND_TO_ZERO                         (1 << 3)
#define cldFpROUND_TO_INF                          (1 << 4)
#define cldFpFMA                                   (1 << 5)
#define cldFpSOFT_FLOAT                            (1 << 6)
#define cldFpFAST_RELAXED_MATH                     (1 << 7)
#define cldFpFINITE_MATH_ONLY                      (1 << 8)
#define cldFpCapsDefault    (cldFpROUND_TO_ZERO | cldFpROUND_TO_NEAREST)

/* all floating point flags */
#define cldFpAllFlags  (cldFpDENORM | \
                        cldFpINF_NAN | \
                        cldFpROUND_TO_NEAREST | \
                        cldFpROUND_TO_ZERO | \
                        cldFpROUND_TO_INF | \
                        cldFpFMA | \
                        cldFpSOFT_FLOAT | \
                        cldFpFAST_RELAXED_MATH | \
                        cldFpFINITE_MATH_ONLY)

#define cldVxImageArrayMaxLevelDefault  10

#if gcdDEBUG

#define clmVERIFY_OBJECT(obj, objType) \
    do { \
       if (((obj) == gcvNULL) || (((clsOBJECT *)(obj))->type != (objType))) { \
        gcmASSERT(((obj) != gcvNULL) && (((clsOBJECT *)(obj))->type == (objType))); \
        return gcvSTATUS_INVALID_OBJECT; \
       } \
    } \
    while (gcvFALSE)

#define clmASSERT_OBJECT(obj, objType) \
    do { \
       if (((obj) == gcvNULL) || (((clsOBJECT *)(obj))->type != (objType))) { \
        gcmASSERT(((obj) != gcvNULL) && (((clsOBJECT *)(obj))->type == (objType))); \
       } \
    } \
    while (gcvFALSE)

#else

#define clmVERIFY_OBJECT(obj, objType) do {} while (gcvFALSE)
#define clmASSERT_OBJECT(obj, objType) do {} while (gcvFALSE)

#endif

/* Optimization options. */
typedef enum _cleOPTIMIZATION_OPTION
{
    clvOPTIMIZATION_NONE         = 0x0000,
    clvOPTIMIZATION_CALCULATION    = 0x0001,
    clvOPTIMIZATION_UNROLL_ITERATION= 0x0010,
    clvOPTIMIZATION_DATA_FLOW    = 0x0100,
    clvOPTIMIZATION_SPECIAL        = 0x8000,
    clvOPTIMIZATION_ALL        = 0xFFFF,
}
cleOPTIMIZATION_OPTION;

typedef gctUINT32    cltOPTIMIZATION_OPTIONS;

#define cldOPTIMIZATION_OPTIONS_DEFAULT (clvOPTIMIZATION_CALCULATION | \
                                         clvOPTIMIZATION_UNROLL_ITERATION | \
                                         clvOPTIMIZATION_DATA_FLOW)

/* Dump options. */
typedef enum _cleDUMP_OPTION
{
    clvDUMP_NONE     = 0x0000,
    clvDUMP_SOURCE    = 0x0001,
    clvDUMP_PREPROCESSOR    = 0x0010,
    clvDUMP_SCANNER        = 0x0100,
    clvDUMP_PARSER        = 0x0200,
    clvDUMP_IR        = 0x0400,

    clvDUMP_CODE_GENERATOR    = 0x1000,
    clvDUMP_CODE_EMITTER    = 0x2000,

    clvDUMP_COMPILER    = 0xff00,
    clvDUMP_ALL        = 0xffff
}
cleDUMP_OPTION;

typedef gctUINT16    cltDUMP_OPTIONS;

/* Type of objects. */
typedef enum _cleOBJECT_TYPE
{
    clvOBJ_UNKNOWN         = 0,
    clvOBJ_COMPILER        = gcmCC('C','M','P','L'),
    clvOBJ_CODE_GENERATOR    = gcmCC('C','G','E','N'),
    clvOBJ_CODE_EMITTER    = gcmCC('C','E','M','T')
}
cleOBJECT_TYPE;

/* clsOBJECT object defintinon. */
typedef struct _clsOBJECT
{
    /* Type of an object. */
    cleOBJECT_TYPE                type;
}
clsOBJECT;

struct _cloCOMPILER;
struct _cloIR_BASE;

typedef struct _cloCOMPILER *            cloCOMPILER;

struct _cloCODE_GENERATOR;

typedef struct _cloCODE_GENERATOR *        cloCODE_GENERATOR;

struct _cloCODE_EMITTER;

typedef struct _cloCODE_EMITTER *        cloCODE_EMITTER;

/* Construct a new cloCOMPILER object. */
typedef enum _cleSHADER_TYPE
{
    clvSHADER_TYPE_VERTEX        = gcSHADER_TYPE_VERTEX,
    clvSHADER_TYPE_FRAGMENT        = gcSHADER_TYPE_FRAGMENT,
    clvSHADER_TYPE_CL            = gcSHADER_TYPE_CL
}
cleSHADER_TYPE;

gceSTATUS
cloCOMPILER_SetImageArrayMaxLevel(
    IN cloCOMPILER Compiler,
    IN gctSTRING MaxLevel
);

gctUINT32
cloCOMPILER_GetImageArrayMaxLevel(
    IN cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetLanguageVersion(
    IN cloCOMPILER Compiler,
    IN gctSTRING LangVersion
);

gctUINT32
cloCOMPILER_GetLanguageVersion(
    IN cloCOMPILER Compiler
);

void
cloCOMPILER_GetVersion(
    IN cloCOMPILER Compiler,
    IN cleSHADER_TYPE ShaderType,
    IN OUT gctUINT32 *CompilerVersion
);

gceSTATUS
cloCOMPILER_Load(
    void
    );

gceSTATUS
cloCOMPILER_Unload(
    void
    );

gceSTATUS
cloCOMPILER_Construct(
    INOUT cloCOMPILER Compiler
    );

gceSTATUS
cloCOMPILER_Construct_General(
    IN gctCONST_STRING Options,
    OUT cloCOMPILER * Compiler
    );

gceSTATUS
cloCOMPILER_ConstructByLangVersion(
    IN gctUINT32 LangVersion,
    OUT cloCOMPILER * Compiler
    );

/* Destroy an cloCOMPILER object. */
gceSTATUS
cloCOMPILER_Destroy(
    IN cloCOMPILER Compiler
    );

/* Destroy an cloCOMPILER object. */
gceSTATUS
cloCOMPILER_Destroy_General(
    IN cloCOMPILER Compiler
    );

/* Get the shader type */
gceSTATUS
cloCOMPILER_GetShaderType(
    IN cloCOMPILER Compiler,
    OUT cleSHADER_TYPE * ShaderType
    );

/* Get the binary */
gceSTATUS
cloCOMPILER_GetBinary(
    IN cloCOMPILER Compiler,
    OUT gcSHADER * Binary
    );

/* Compile the source strings */
gceSTATUS
cloCOMPILER_Compile(
    IN cloCOMPILER Compiler,
    IN cltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN cltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options,
    OUT gcSHADER * Binary,
    OUT gctSTRING * Log
    );

/* Preprocess the source strings only */
gceSTATUS
cloCOMPILER_Preprocess(
    IN cloCOMPILER Compiler,
    IN cltOPTIMIZATION_OPTIONS OptimizationOptions,
    IN cltDUMP_OPTIONS DumpOptions,
    IN gctUINT StringCount,
    IN gctCONST_STRING Strings[],
    IN gctCONST_STRING Options,
    OUT gctSTRING * Log
    );

/* Set the current line no */
gceSTATUS
cloCOMPILER_SetCurrentLineNo(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo
    );

/* Set the current string no */
gceSTATUS
cloCOMPILER_SetCurrentStringNo(
    IN cloCOMPILER Compiler,
    IN gctUINT StringNo
    );

/* Set the current file name */
gceSTATUS
cloCOMPILER_SetCurrentFileName(
    IN cloCOMPILER Compiler,
    IN gctSTRING Text
    );

gceSTATUS
cloCOMPILER_SetIsMainFile(
    IN cloCOMPILER Compiler,
    IN gctBOOL     isMainFile
    );

/* Get the current line no */
gctUINT
cloCOMPILER_GetCurrentLineNo(
    IN cloCOMPILER Compiler
    );

/* Get the current string no */
gctUINT
cloCOMPILER_GetCurrentStringNo(
    IN cloCOMPILER Compiler
    );

gctSTRING
cloCOMPILER_GetCurrentFileName(
IN cloCOMPILER Compiler
);

/* Allocate memory blocks for the compiler */
gceSTATUS
cloCOMPILER_Allocate(
    IN cloCOMPILER Compiler,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    );

/* Allocate memory blocks initialized to zero for the compiler */
gceSTATUS
cloCOMPILER_ZeroMemoryAllocate(
    IN cloCOMPILER Compiler,
    IN gctSIZE_T Bytes,
    OUT gctPOINTER * Memory
    );

/* Free memory blocks for the compiler */
gceSTATUS
cloCOMPILER_Free(
    IN cloCOMPILER Compiler,
    IN gctPOINTER Memory
    );

/* Allocate name for the compiler */
gceSTATUS
cloCOMPILER_AllocateName(
    IN cloCOMPILER Compiler,
    OUT gctPOINTER * Memory
    );

/* Report info */
typedef enum _cleREPORT_TYPE
{
    clvREPORT_FATAL_ERROR         = 0,
    clvREPORT_INTERNAL_ERROR,
    clvREPORT_ERROR,
    clvREPORT_WARN,
    clvREPORT_INFO
}
cleREPORT_TYPE;

gceSTATUS
cloCOMPILER_VReport(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN gctARGUMENTS Arguments
    );

gceSTATUS
cloCOMPILER_Report(
    IN cloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN cleREPORT_TYPE Type,
    IN gctCONST_STRING Message,
    IN ...
    );

gceSTATUS
cloCOMPILER_SetFpConfig(
IN cloCOMPILER Compiler,
IN gctUINT32 Flag
);

gctUINT32
cloCOMPILER_GetFpConfig(
IN cloCOMPILER Compiler
);

/* Dump info */
gceSTATUS
cloCOMPILER_Dump(
    IN cloCOMPILER Compiler,
    IN cleDUMP_OPTION DumpOption,
    IN gctCONST_STRING Message,
    IN ...
    );

gceSTATUS
cloCOMPILER_DumpDIE(
    IN cloCOMPILER Compiler,
    IN cleDUMP_OPTION DumpOption,
    IN gctUINT16 id
    );

/* Allocate pool string */
typedef gctSTRING        cltPOOL_STRING;

gceSTATUS
cloCOMPILER_AllocatePoolString(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT cltPOOL_STRING * PoolString
    );

gceSTATUS
cloCOMPILER_SetCurrentPMP(
IN cloCOMPILER  Compiler,
IN VSC_PRIMARY_MEM_POOL *CurrentPMP
);

VSC_PRIMARY_MEM_POOL *
cloCOMPILER_SetGeneralPMP(
IN cloCOMPILER  Compiler
);

gceSTATUS
cloCOMPILER_FindPrivatePoolString(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT cltPOOL_STRING * PoolString
    );

gceSTATUS
cloCOMPILER_FindGeneralPoolString(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT cltPOOL_STRING * PoolString
    );

gceSTATUS
cloCOMPILER_FindPoolString(
    IN cloCOMPILER Compiler,
    IN gctCONST_STRING String,
    OUT cltPOOL_STRING * PoolString
    );

typedef enum _cleOPENCLSetting {
  OPENCL_INVALID,
  OPENCL_ENABLE,
  OPENCL_DISABLE,
  OPENCL_ON,
  OPENCL_OFF,
  OPENCL_DEFAULT
}
cleOPENCLSetting;

/* Extensions */
typedef enum _cleEXTENSION
{
    clvEXTENSION_NONE                             = 0x00000000,
    clvEXTENSION_STANDARD_DERIVATIVES             = 0x00000001,
    clvEXTENSION_VASM                             = 0x00000002,
    clvEXTENSION_VIV_VX                           = 0x00000004,
    clvEXTENSION_VIV_BITFIELD                     = 0x00000008,
    clvEXTENSION_VIV_CMPLX                        = 0x00000010,
    clvEXTENSION_CL_KHR_FP16                      = 0x00000020,
    clvEXTENSION_CL_KHR_3D_IMAGE_WRITES           = 0x00000040,
    clvEXTENSION_CL_KHR_BYTE_ADDRESSABLE_STORE    = 0x00000080,
    clvEXTENSION_CL_KHR_GL_SHARING                = 0x00000100,
    clvEXTENSION_CL_KHR_FP64                      = 0x00000200,
    clvEXTENSION_ES_KHR_INT64                     = 0x00000400,
    clvEXTENSION_CL_KHR_GLOBAL_INT32_EXTENDED_ATOMICS = 0x00000800,
    clvEXTENSION_CL_KHR_GLOBAL_INT32_BASE_ATOMICS     = 0x00001000,
    clvEXTENSION_CL_KHR_LOCAL_INT32_EXTENDED_ATOMICS  = 0x00002000,
    clvEXTENSION_CL_KHR_LOCAL_INT32_BASE_ATOMICS  = 0x00004000,
    clvEXTENSION_CL_KHR_INT64_BASE_ATOMICS        = 0x00008000,
    clvEXTENSION_CL_KHR_INT64_EXTENDED_ATOMICS    = 0x00010000,
    clvEXTENSION_CL_KHR_GL_EVENT                  = 0x00020000,
    clvEXTENSION_CL_KHR_D3D10_SHARING             = 0x00040000,
    clvEXTENSION_ALL                              = clvEXTENSION_STANDARD_DERIVATIVES
}
cleEXTENSION;

typedef gctUINT32                cltEXTENSIONS;

/* Enable extension */
gceSTATUS
cloCOMPILER_EnableExtension(
    IN cloCOMPILER Compiler,
    IN cleEXTENSION Extension,
    IN gctBOOL Enable
    );

gceSTATUS
cloCOMPILER_SetBasicTypePacked(
IN cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetLongUlongPatch(
IN cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetGcslDriverImage(
IN cloCOMPILER Compiler
);

gceSTATUS
cloCOMPILER_SetFastRelaxedMath(
IN cloCOMPILER Compiler
);

gctBOOL
cloCOMPILER_IsLoadingBuiltin(
    IN cloCOMPILER Compiler
    );

cloCOMPILER *
gcGetKernelCompiler(void);

#ifdef __cplusplus
}
#endif
#endif /* __gc_cl_compiler_h_ */
