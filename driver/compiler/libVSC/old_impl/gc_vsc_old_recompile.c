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


/*
**  Common gcSL compiler module.
*/

#include "gc_vsc.h"
#if DX_SHADER
#include <Windows.h>
#endif

#define _GC_OBJ_ZONE    gcvZONE_COMPILER

#if gcdENABLE_3D

#define __COMPILE_TEX_FORMAT_CONVERT_LIBRARY__  1
#define __COMPILE_CL_PATCH_LIBRARY__            1

#if __COMPILE_TEX_FORMAT_CONVERT_LIBRARY__
#include "patch_lib/gc_vsc_gl_patch_lib.h"
#endif

#if __COMPILE_CL_PATCH_LIBRARY__
#include "patch_lib/gc_vsc_cl_patch_lib.h"
#endif

/* library for gl built-in functions that are written in high level shader */
#include "old_impl/gc_vsc_gl_builtin_lib.h"

/* library for cl built-in functions that are written in high level shader */
#include "old_impl/gc_vsc_cl_builtin_lib.h"

extern gctGLSLCompiler gcGLSLCompiler;
gcSHADER gcTexFormatConvertLibrary = gcvNULL;
gctSTRING RecompilerShaderSource   = gcvNULL;

/* Builtin library for HW that can't support IMG instructions.*/
gcSHADER gcBuiltinLibrary0 = gcvNULL;
/* Builtin library for HW taht can support IMG instructions. */
gcSHADER gcBuiltinLibrary1 = gcvNULL;
gcSHADER gcBlendEquationLibrary = gcvNULL;

extern gctCLCompiler gcCLCompiler;
#if _SUPPORT_LONG_ULONG_DATA_TYPE
#define CL_LIB_COUNT    3
#else
#define CL_LIB_COUNT    2
#endif
gcSHADER gcCLPatchLibrary[CL_LIB_COUNT]  = {gcvNULL};

gcSHADER gcCLBuiltinLibrary = gcvNULL;

/* define the max builtin parameter count,
   should be the same as slmMAX_BUILT_IN_PARAMETER_COUNT*/
#define gcMAX_BUILT_IN_PARAMETER_COUNT        32

#if _SUPPORT_LONG_ULONG_DATA_TYPE
static gctBOOL isLogicOR = gcvFALSE;
static gctBOOL Patched1Dto2D = gcvFALSE;
#endif

gceSTATUS
gcLoadDXTexFormatConvertLibrary(
    void
    );

gceSTATUS
gcLoadESTexFormatConvertLibrary(
    void
    );

gctUINT16
_SelectSwizzle(
    IN gctUINT16 Swizzle,
    IN gctSOURCE_t Source
    );

gctUINT8
_Enable2Swizzle(
    IN gctUINT32 Enable
    );

static gceSTATUS
gcLoadTexFormatConvertLibrary()
{
#if DX_SHADER
    return gcLoadDXTexFormatConvertLibrary();
#else
    return gcLoadESTexFormatConvertLibrary();
#endif
}

/* forward declarations */
static gctINT
_insertNOP2Main(
    IN OUT gcSHADER          Shader,
    IN     gctUINT            Num
    );

/* call this function before using TempIndex in the shader */
static gceSTATUS
_ChangeAttribToTempForAllCodes(
    IN OUT gcSHADER Shader,
    IN gctUINT16 AttribIndex,
    IN gctUINT16 TempIndex
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i;
    gcSL_INSTRUCTION code;

    for (i = 0; i < Shader->codeCount; i++)
    {
        code = &Shader->code[i];

        if (code->tempIndex == TempIndex && code->opcode != gcSL_JMP) continue;

        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
            code->source0Index == AttribIndex)
        {
            code->source0 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_TEMP);
            code->source0Index = TempIndex;
            code->source0Indexed = 0;
        }

        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
            code->source1Index == AttribIndex)
        {
            code->source1 = gcmSL_SOURCE_SET(code->source1, Type, gcSL_TEMP);
            code->source1Index = TempIndex;
            code->source1Indexed = 0;
        }
    }

    return status;
}
extern gceSTATUS
_ExpandCode(
    IN gcSHADER Shader,
    IN gctUINT  CodeCount
    );

static gctBOOL
_patchApplys(
    IN OUT gcSHADER         Shader,
    IN gcPatchDirective  *  PatchDirective
    )
{
    /* check if the patch directive is applied to this Shader */

    return gcvTRUE;
}

typedef enum
{
    gcvSource0,
    gcvSource1,
    gcvIntConstant,
    gcvUIntConstant,
    gcvFloatConstant,
    gcvIntTempIndex,
    gcvUIntTempIndex,
    gcvFloatTempIndex,
    gcvIntUniformIndex,
    gcvUIntUniformIndex,
    gcvFloatUniformIndex,
    gcvDest
}
_sourceType;

typedef enum
{
    gcvTempReg,
    gcvCodeTemp
}
_targetType;

static gctINT
_getComponentsFromEnable(
    IN gcSL_ENABLE Enable
    )
{
    gctINT n = 0;
    if (0 != (Enable & gcSL_ENABLE_X))
        n++;

    if (0 != (Enable & gcSL_ENABLE_Y))
        n++;

    if (0 != (Enable & gcSL_ENABLE_Z))
        n++;

    if (0 != (Enable & gcSL_ENABLE_W))
        n++;

    return n;
}

/* Copy the source info from FromCode to ToCode */
static void
_copySource(
    IN OUT gcSL_INSTRUCTION  ToCode,
    IN     gctINT            ToSrcNo,
    IN     gcSL_INSTRUCTION  FromCode,
    IN     _sourceType       FromSrcType,
    IN     gcsValue *        SourceValue,
    IN     gcSL_SWIZZLE      srcSwizzle,
    IN     gcSHADER_PRECISION SourcePrecision
    )
{
    gctSOURCE_t source = 0;
    gctUINT16 sourceIndex = 0, sourceIndexed = 0;
    gctUINT8                 swizzle;
    gcSL_FORMAT              format;
    gctUINT8                 enable;
    gcSHADER_PRECISION       precision;
    gctINT                   components;

    switch (FromSrcType)
    {
    case gcvSource0:
        gcmASSERT(FromCode != gcvNULL);
        source        = FromCode->source0;
        sourceIndex   = FromCode->source0Index;
        sourceIndexed = FromCode->source0Indexed;
        break;

    case gcvSource1:
        gcmASSERT(FromCode != gcvNULL);
        source        = FromCode->source1;
        sourceIndex   = FromCode->source1Index;
        sourceIndexed = FromCode->source1Indexed;
        break;

    case gcvIntConstant:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_INTEGER);
        source = gcmSL_SOURCE_SET(source, Precision, gcSHADER_PRECISION_HIGH);
        sourceIndex = (gctUINT16) ((SourceValue->i32) & 0xFFFF);
        sourceIndexed = (gctUINT16) (((SourceValue->i32) & 0xFFFF0000) >> 16);
        break;

    case gcvUIntConstant:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_UINT32);
        source = gcmSL_SOURCE_SET(source, Precision, gcSHADER_PRECISION_HIGH);
        sourceIndex = (gctUINT16) ((SourceValue->u32) & 0xFFFF);
        sourceIndexed = (gctUINT16) (((SourceValue->u32) & 0xFFFF0000) >> 16);
        break;

    case gcvFloatConstant:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_FLOAT);
        source = gcmSL_SOURCE_SET(source, Precision, gcSHADER_PRECISION_HIGH);
        sourceIndex = (gctUINT16) ((SourceValue->u32) & 0xFFFF);
        sourceIndexed = (gctUINT16) (((SourceValue->u32) & 0xFFFF0000) >> 16);
        break;

    case gcvIntTempIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_INTEGER);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->i32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvUIntTempIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_UINT32);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->u32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvFloatTempIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_FLOAT);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->i32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvIntUniformIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_INT32);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->i32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvUIntUniformIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_UINT32);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->u32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvFloatUniformIndex:
        source = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM);
        source = gcmSL_SOURCE_SET(source, Format, gcSL_FLOAT);
        source = gcmSL_SOURCE_SET(source, Swizzle, srcSwizzle);
        source = gcmSL_SOURCE_SET(source, Precision, SourcePrecision);
        sourceIndex = (gctUINT16) ((SourceValue->i32) & 0xFFFF);
        sourceIndexed = 0;
        break;

    case gcvDest:
        gcmASSERT(FromCode != gcvNULL);
        format      = gcmSL_TARGET_GET(FromCode->temp, Format);
        enable      = gcmSL_TARGET_GET(FromCode->temp, Enable);
        precision   = gcmSL_TARGET_GET(FromCode->temp, Precision);
        /* get components from enable */
        components  = _getComponentsFromEnable(enable);

        swizzle     = (components == 1) ? gcSL_SWIZZLE_XXXX :
                      (components == 2) ? gcSL_SWIZZLE_XYYY :
                      (components == 3) ? gcSL_SWIZZLE_XYZZ :
                                          gcSL_SWIZZLE_XYZW;

        source = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP);
        source = gcmSL_SOURCE_SET(source, Format, format);
        source = gcmSL_SOURCE_SET(source, Swizzle, swizzle);
        source = gcmSL_SOURCE_SET(source, Precision, precision);

        sourceIndex   = FromCode->tempIndex;
        sourceIndexed = FromCode->tempIndexed;

        break;
    default:
        /* Invalid data. */
        gcmASSERT(gcvFALSE);
        break;
    }

    switch (ToSrcNo)
    {
    case 0:
        /* Update source0 operand. */
        ToCode->source0        = source;
        ToCode->source0Index   = sourceIndex;
        ToCode->source0Indexed = sourceIndexed;
        break;

    case 1:
        /* Update source1 operand. */
        ToCode->source1        = source;
        ToCode->source1Index   = sourceIndex;
        ToCode->source1Indexed = sourceIndexed;
        break;

    default:
        /* Invalid data. */
        gcmASSERT(gcvFALSE);
        break;
    }
}

static gceSTATUS
_addArgPassByAnotherArg(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         TargetFunction,
    IN gctUINT            TargetArgNo,
    IN gcFUNCTION         SourceFunction,
    IN gctUINT            SourceArgNo,
    IN gcSL_FORMAT        InstType
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR targetArg, sourceArg;
    gcSL_INSTRUCTION         curCode;

    gcmASSERT(TargetArgNo < TargetFunction->argumentCount &&
              SourceArgNo < SourceFunction->argumentCount);

    targetArg = &TargetFunction->arguments[TargetArgNo];
    sourceArg = &SourceFunction->arguments[SourceArgNo];

    gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_MOV, targetArg->index, targetArg->enable, InstType, targetArg->precision));
    curCode = Shader->code + Shader->lastInstruction;
    curCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                     | gcmSL_SOURCE_SET(0, Precision, sourceArg->precision)
                     | gcmSL_SOURCE_SET(0, Format, InstType)
                     | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW);
    curCode->source0Index = sourceArg->index;
    curCode->source0Indexed = 0;

OnError:
    return status;
}

static gceSTATUS
_addSamplerArgPassInst(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         Function,
    IN gctUINT            ArgNo,
    IN gcUNIFORM          Uniform,
    IN gctINT             Index
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR arg;
    gcSL_INSTRUCTION         code;

    gcmASSERT(ArgNo < Function->argumentCount);

    arg = &Function->arguments[ArgNo];

    gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_GET_SAMPLER_IDX, arg->index, arg->enable, gcSL_INTEGER, arg->precision));

    code = Shader->code + Shader->lastInstruction;

    code->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                  | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                  | gcmSL_SOURCE_SET(0, Precision, GetUniformPrecision(Uniform))
                  | gcmSL_SOURCE_SET(0, Format, GetUniformFormat(Uniform))
                  | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW);
    code->source0Index = gcmSL_INDEX_SET(0, Index, GetUniformIndex(Uniform))
                       | gcmSL_INDEX_SET(0, ConstValue, Index);
    code->source0Indexed = Index & ~3;

OnError:
    /* Return the status. */
    return status;
}

/* mov  arg, src */
static gceSTATUS
_addArgPassInst(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         Function,
    IN gcFUNCTION         StubFunction,
    IN gcSL_INSTRUCTION   Code,
    IN gctUINT            ArgNo,
    IN _sourceType        SourceType,
    IN gcsValue *         SourceValue,
    IN gcSL_SWIZZLE       srcSwizzle,
    IN gctBOOL            IsSampler,
    IN gcSHADER_PRECISION srcPrecision
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR arg;
    gcSL_INSTRUCTION         curCode;
    gcSL_FORMAT              srcFormat;

    gcmASSERT(ArgNo < Function->argumentCount);

    arg = &Function->arguments[ArgNo];
    switch (SourceType)
    {
    case gcvSource0:
        gcmASSERT(Code != gcvNULL && StubFunction != gcvNULL);
        srcFormat = gcmSL_SOURCE_GET(Code->source0, Format);
        if (gcmSL_SOURCE_GET(Code->source0, Indexed) != gcSL_NOT_INDEXED)
        {
            gcmONERROR(gcFUNCTION_AddArgument(StubFunction,
                      0xffff,
                      Code->source0Indexed,
                      gcSL_ENABLE_XYZW,
                      gcvFUNCTION_INPUT,
                      gcSHADER_PRECISION_HIGH, /* to be changed to more accurate one */
                      gcvTRUE));
        }
        break;

    case gcvSource1:
        gcmASSERT(Code != gcvNULL && StubFunction != gcvNULL);
        srcFormat = gcmSL_SOURCE_GET(Code->source1, Format);
        if (gcmSL_SOURCE_GET(Code->source1, Indexed) != gcSL_NOT_INDEXED)
        {
            gcmONERROR(gcFUNCTION_AddArgument(StubFunction,
                      0xffff,
                      Code->source1Indexed,
                      gcSL_ENABLE_XYZW,
                      gcvFUNCTION_INPUT,
                      gcSHADER_PRECISION_HIGH, /* to be changed to more accurate one */
                      gcvTRUE));
        }
        break;

    case gcvIntConstant:
    case gcvIntTempIndex:
    case gcvIntUniformIndex:
        srcFormat = gcSL_INTEGER;
        break;

    case gcvUIntConstant:
    case gcvUIntTempIndex:
    case gcvUIntUniformIndex:
        srcFormat = gcSL_UINT32;
        break;

    case gcvFloatConstant:
    case gcvFloatTempIndex:
    case gcvFloatUniformIndex:
        srcFormat = gcSL_FLOAT;
        break;
    case gcvDest:
        gcmASSERT(Code != gcvNULL);
        srcFormat = gcmSL_TARGET_GET(Code->temp, Format);
        break;
    default:
        gcmASSERT(gcvFALSE);
        srcFormat = gcSL_FLOAT;
        break;
    }

    gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_MOV, arg->index, arg->enable, srcFormat, arg->precision));

    curCode = Shader->code + Shader->lastInstruction;
    _copySource(curCode, 0 /* mov src0 */, Code, SourceType, SourceValue, srcSwizzle, srcPrecision);

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_addCallInst(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         Callee
    )
{
    return gcSHADER_AddOpcodeConditional(Shader,
                                         gcSL_CALL,
                                         gcSL_ALWAYS,
                                         Callee->label);
}

static gceSTATUS
_addRetInst(
    IN OUT gcSHADER Shader
    )
{
    return gcSHADER_AddOpcodeConditional(Shader,
                                         gcSL_RET,
                                         gcSL_ALWAYS,
                                         0);
}

/* the return value is in Function's output argument ArgNo, copy
 * the output value to temp register in SourceNo (-1 means dest)
 * of Shader->code[CodeIndex]
 */
static gceSTATUS
_addRetValueInst(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         Function,
    IN gcSL_INSTRUCTION   Code,
    IN gctUINT            ArgNo,
    IN _sourceType        SourceType,
    IN gctPOINTER         SourceValue
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR arg;
    gctSOURCE_t              source;
    /*gcSL_TYPE                type;*/
    gctUINT16                tempIndex;
    gctUINT16                tempIndexed;
    gctUINT8                 swizzle;
    gcSL_INDEXED             mode;
    gcSL_FORMAT              format;
    gctUINT8                 enable;
    gctBOOL                  isSource0;
    gcsValue *               val;

    gcmASSERT(ArgNo < Function->argumentCount);

    arg = &Function->arguments[ArgNo];
    /* get the temp register index for SourceNo */
    switch (SourceType)
    {
    case gcvSource0:
    case gcvSource1:
        isSource0   = (SourceType == gcvSource0) ? gcvTRUE : gcvFALSE;
        source      = isSource0 ? Code->source0 : Code->source1;
        /*type        = gcmSL_SOURCE_GET(source, Type);*/
        mode        = gcmSL_SOURCE_GET(source, Indexed);
        format      = gcmSL_SOURCE_GET(source, Format);
        swizzle     = gcmSL_SOURCE_GET(source, Swizzle);
        tempIndex   = isSource0 ? Code->source0Index
                                : Code->source1Index;
        tempIndexed = isSource0 ? Code->source0Indexed
                                : Code->source1Indexed;
        enable = gcSL_ConvertSwizzle2Enable((gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleX),
                                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleY),
                                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleZ),
                                            (gcSL_SWIZZLE) gcmSL_SOURCE_GET(source, SwizzleW));
       break;
    case gcvDest: /* dest */
        /*type        = gcSL_TEMP;*/
        mode        = gcmSL_TARGET_GET(Code->temp, Indexed);
        format      = gcmSL_TARGET_GET(Code->temp, Format);
        enable      = gcmSL_TARGET_GET(Code->temp, Enable);
        swizzle     = gcSL_SWIZZLE_XYZW;
        tempIndex   = Code->tempIndex;
        tempIndexed = Code->tempIndexed;
        break;
    case gcvIntTempIndex: /* temp index */
    case gcvUIntTempIndex:
    case gcvFloatTempIndex: /* temp index */
        /*type        = gcSL_TEMP;*/
        mode        = 0;
        format      = (SourceType == gcvIntTempIndex) ? gcSL_INTEGER :
                                                        ((SourceType == gcvUIntTempIndex) ? gcSL_UINT32 : gcSL_FLOAT);
        enable      = gcSL_ENABLE_XYZW; /* default enable, need to change it later if it's different */
        swizzle     = gcSL_SWIZZLE_XYZW;
        val         = (gcsValue *)SourceValue;
        gcmASSERT(val->i32 >= 0 && val->i32 < 0xFFFF);
        tempIndex   = (gctUINT16) val->i32;
        tempIndexed = 0;
        break;
    default:
        gcmASSERT(gcvFALSE);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmONERROR(gcSHADER_AddOpcodeIndexed(Shader, gcSL_MOV, tempIndex, enable,
                                         mode, tempIndexed, format, arg->precision));
    gcmONERROR(gcSHADER_AddSource(Shader, gcSL_TEMP, arg->index, swizzle, format, arg->precision));

OnError:
    /* Return the status. */
    return status;
}

#if _SUPPORT_LONG_ULONG_DATA_TYPE
static gceSTATUS
_addRetValue2NewTemp(
    IN OUT gcSHADER       Shader,
    IN gcFUNCTION         Function,
    IN gcSL_INSTRUCTION   Code,
    IN gctUINT            ArgNo,
    IN _targetType        TargetType,
    IN OUT gctUINT16 *      Index
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gcsFUNCTION_ARGUMENT_PTR arg;
    gctUINT16                tempIndex;
    gctUINT16                tempIndexed;
    gctUINT8                 swizzle;
    gcSL_INDEXED             mode;
    gcSL_FORMAT              format;
    gctUINT8                 enable;
    gctUINT32                rows;
    gcSHADER_TYPE            type;

    gcmASSERT(ArgNo < Function->argumentCount);

    arg = &Function->arguments[ArgNo];
    /* get the temp register index for SourceNo */
    switch (TargetType)
    {
    case gcvTempReg: /* dest */
        /*type        = gcSL_TEMP;*/
        mode        = gcmSL_TARGET_GET(Code->temp, Indexed);
        format      = gcmSL_TARGET_GET(Code->temp, Format);
        enable      = gcSL_ENABLE_X;
        swizzle     = gcSL_SWIZZLE_XYZW;

        gcTYPE_GetFormatInfo(format,
                             1,
                             &rows,
                             &type);

        tempIndex   = (gctUINT16)gcSHADER_NewTempRegs(Shader, rows, type);
        tempIndexed = 0;
        *Index = tempIndex;
        break;

    case gcvCodeTemp:
        mode        = gcmSL_TARGET_GET(Code->temp, Indexed);
        format      = gcmSL_SOURCE_GET(Code->source0, Format);
        enable      = gcmSL_TARGET_GET(Code->temp, Enable);
        swizzle     = gcSL_SWIZZLE_XYZW;
        tempIndex   = Code->tempIndex;
        tempIndexed = Code->tempIndexed;
        break;
    default:
        gcmASSERT(gcvFALSE);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    gcmONERROR(gcSHADER_AddOpcodeIndexed(Shader, gcSL_MOV, tempIndex, enable,
                                         mode, tempIndexed, format, arg->precision));
    gcmONERROR(gcSHADER_AddSource(Shader, gcSL_TEMP, arg->index, swizzle, format, arg->precision));

OnError:
    /* Return the status. */
    return status;
}
#endif

extern void
gcSL_SetInst2NOP(
    IN OUT gcSL_INSTRUCTION      Code
    );

gceSTATUS
gcGetConvertFunctionName(
    IN gcsInputConversion * FormatConversion,
    IN gceTexldFlavor       TexldFlavor,
    OUT gctSTRING *         ConvertFuncName
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctCHAR                 name[128] = "_txcvt_";
    gctCONST_STRING         txFormat;

    gcmASSERT(TexldFlavor < gceTF_COUNT);

#if DX_SHADER
    switch (FormatConversion->mipFilter)
    {
    case gcTEXTURE_MODE_NONE:
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "mip_none_"));
        break;
    case gcTEXTURE_MODE_POINT:
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "mip_point_"));
        break;
    case gcTEXTURE_MODE_LINEAR:
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "mip_linear_"));
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    if (FormatConversion->minFilter == FormatConversion->magFilter &&
        FormatConversion->minFilter == gcTEXTURE_MODE_POINT)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "point_"));
    }
#endif

    txFormat   = FormatConversion->samplerInfo.formatName;
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), txFormat));

    if (FormatConversion->samplerInfo.format == gcvSURF_D24S8)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_1_A8R8G8B8_stencilMode"));
    }
    /* depthStencilMode 1: use depth (vec4), 0: use stencil (uvec4)*/
    else if (FormatConversion->samplerInfo.format == gcvSURF_S8D32F_1_G32R32F
        && FormatConversion->depthStencilMode == 0)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_u"));
    }

    /* set flavor */
    if (TexldFlavor != gceTF_NONE)
    {
        gctCONST_STRING flavor = gcTexldFlavor[TexldFlavor];
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), flavor));
    }

    /* dup the name to ConvertFuncName */
    gcmONERROR(gcoOS_StrDup(gcvNULL, name, ConvertFuncName));

OnError:
    /* Return the status. */
    return status;
}

gceSTATUS
gcGetOutputConvertFunctionName(
    IN gcsOutputConversion * OutputConversion,
    OUT gctSTRING *          ConvertFuncName
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctCHAR                 name[128] = "_outputcvt_";
    gctCONST_STRING         formatName;

    formatName = OutputConversion->formatInfo.formatName;
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), formatName));

    /* dup the name to ConvertFuncName */
    gcmONERROR(gcoOS_StrDup(gcvNULL, name, ConvertFuncName));

OnError:
    /* Return the status. */
    return status;
}


static gceTexldFlavor
_getTexldFlavor(
    IN gcSL_OPCODE TexldOpcode,
    IN gcSL_OPCODE TexldStatusOpcode)
{
    switch (TexldStatusOpcode)
    {
    case gcSL_NOP:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_TEXLD;
        case gcSL_TEXLDPROJ:
            return gceTF_PROJ;
        case gcSL_TEXLDPCF:
            return gceTF_PCF;
        case gcSL_TEXLDPCFPROJ:
            return gceTF_PCFPROJ;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_NONE;
        }
        break;
    case gcSL_TEXBIAS:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_BIAS_TEXLD;
        case gcSL_TEXLDPROJ:
            return gceTF_BIAS_PROJ;
        case gcSL_TEXLDPCF:
            return gceTF_BIAS_PCF;
        case gcSL_TEXLDPCFPROJ:
            return gceTF_BIAS_PCFPROJ;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_BIAS_TEXLD;
        }
        break;
    case gcSL_TEXLOD:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_LOD_TEXLD;
        case gcSL_TEXLDPROJ:
            return gceTF_LOD_PROJ;
        case gcSL_TEXLDPCF:
            return gceTF_LOD_PCF;
        case gcSL_TEXLDPCFPROJ:
            return gceTF_LOD_PCFPROJ;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_LOD_TEXLD;
        }
        break;
    case gcSL_TEXGRAD:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_GRAD_TEXLD;
        case gcSL_TEXLDPROJ:
            return gceTF_GRAD_PROJ;
        case gcSL_TEXLDPCF:
            return gceTF_GRAD_PCF;
        case gcSL_TEXLDPCFPROJ:
            return gceTF_GRAD_PCFPROJ;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_GRAD_TEXLD;
        }
        break;
    case gcSL_TEXGATHER:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_GATHER_TEXLD;
        case gcSL_TEXLDPCF:
            return gceTF_GATHER_PCF;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_GATHER_TEXLD;
        }
        break;
    case gcSL_TEXFETCH_MS:
        switch (TexldOpcode)
        {
        case gcSL_TEXLD:
            return gceTF_FETCH_MS_TEXLD;
        case gcSL_TEXLDPROJ:
            return gceTF_FETCH_MS_PROJ;
        case gcSL_TEXLDPCF:
            return gceTF_FETCH_MS_PCF;
        case gcSL_TEXLDPCFPROJ:
            return gceTF_FETCH_MS_PCFPROJ;
        default:
            gcmASSERT(gcvFALSE);
            return gceTF_FETCH_MS_TEXLD;
        }
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }
    return gceTF_NONE;
}


static
gceSTATUS
_FindFunctionFromShaderOrLibrary(
    IN gcSHADER              Shader,
    IN gcSHADER              Library,
    gctSTRING                FuncName,
    OUT gcFUNCTION *         NewFunction
    )
{
    gceSTATUS       status;
    gcFUNCTION      function;

    /* Check if convertFunction already exists. */
    status = gcSHADER_GetFunctionByName(Shader,
                                          FuncName,
                                          &function);

    if (status == gcvSTATUS_NAME_NOT_FOUND)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            FuncName,
                                            &function));
        gcmASSERT(function != gcvNULL);
    }

OnError:
    if (NewFunction)
    {
        *NewFunction = function;
    }
    return status;
}

static
gceSTATUS
_createInputConvertFunction(
    IN gcSHADER              Shader,
    IN gcSHADER              Library,
    IN gcsInputConversion *  FormatConversion,
    IN gcSL_OPCODE           TexldStatusInstOpcode,
    OUT gcFUNCTION *         NewFunction
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gctSTRING       convertFuncName = gcvNULL;
    gcFUNCTION      convertFunction = gcvNULL;
    gceTexldFlavor  texldFlavor;

    gcmASSERT(Library != gcvNULL);
    if (TexldStatusInstOpcode != gcSL_TEXGRAD)
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, gcSL_NOP);
    }
    else
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, TexldStatusInstOpcode);
    }

    /* get convert function name according to sampler info and
       texld status instruction type */
    gcmONERROR(gcGetConvertFunctionName(FormatConversion,
                                        texldFlavor,
                                        &convertFuncName));

    gcmONERROR(_FindFunctionFromShaderOrLibrary(Shader,
                                                 Library,
                                                 convertFuncName,
                                                 &convertFunction));
    SetFunctionRecompiler(convertFunction);

OnError:
    if (convertFuncName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, convertFuncName));
    }

    *NewFunction = convertFunction;
    return status;
}

static gceSTATUS
_createSwizzleConvertFunction(
    IN gcSHADER              Shader,
    IN gcSHADER              Library,
    IN gcsInputConversion *  FormatConversion,
    OUT gcFUNCTION *         NewFunction
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gctSTRING       convertFuncName = gcvNULL;
    gcFUNCTION      convertFunction = gcvNULL;
    gctCHAR         name[128] = "_txcvt_swizzle_";

    if (FormatConversion->samplerInfo.fmtDataType == gcvFORMAT_DATATYPE_SIGNED_INTEGER)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "int"));
    }
    else if (FormatConversion->samplerInfo.fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_INTEGER)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "uint"));
    }
    else
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "float"));
    }

    gcmONERROR(gcoOS_StrDup(gcvNULL, name, &convertFuncName));

    gcmONERROR(_FindFunctionFromShaderOrLibrary(Shader,
                                                Library,
                                                convertFuncName,
                                                &convertFunction));
    SetFunctionRecompiler(convertFunction);

    *NewFunction = convertFunction;

OnError:
    if (convertFuncName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, convertFuncName));
    }

    if (NewFunction)
    {
        *NewFunction = convertFunction;
    }
    return status;
}

/*static*/ gceSTATUS
_createOutputConvertFunction(
    IN gcSHADER              Shader,
    IN gcSHADER              Library,
    IN gcsOutputConversion * OutputConversion,
    IN gcSL_ENABLE           Enable,
    OUT gcFUNCTION *         NewFunction
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gctSTRING       convertFuncName = gcvNULL;
    gcFUNCTION      convertFunction = gcvNULL;

    gcmASSERT(Library != gcvNULL);

    /* get convert function name according to sampler info and
       texld status instruction type */
    gcmONERROR(gcGetOutputConvertFunctionName(OutputConversion,
                                              &convertFuncName));

    /* Check if convertFunction already exists. */
    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          convertFuncName,
                                          &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            convertFuncName,
                                            &convertFunction));
    }

    SetFunctionRecompiler(convertFunction);

OnError:
    if (convertFuncName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, convertFuncName));
    }

    *NewFunction = convertFunction;
    return status;
}

#if _SUPPORT_LONG_ULONG_DATA_TYPE
gceSTATUS
gcGetLongULongFunctionName(
    IN  gcSL_INSTRUCTION    Instruction,
    OUT gctSTRING           *FunctionName,
    OUT gctBOOL             *isI2I
)
{
    /* Compose the name of the function to process 64-bit integer operations.
       The name is composed of:
       1. type: long/ulong;
       2. Operator: LShift/RShift;
       3. Target enable channel count;
    */
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     index;

    static const gctSTRING typeName[] = {"long", "ulong"};
    static const gctSTRING opName[] = /*{"_LeftShift", "_RightShift"};*/
    {
    "",/*gcSL_NOP, */                     /* 0x00 */
    "",/*gcSL_MOV, */                     /* 0x01 */
    "",/*gcSL_SAT, */                     /* 0x02 */
    "",/*gcSL_DP3, */                     /* 0x03 */
    "",/*gcSL_DP4, */                     /* 0x04 */
    "_abs",/*gcSL_ABS, */                     /* 0x05 */
    "_jmp",/*gcSL_JMP, */                     /* 0x06 */
    "_Add",/*gcSL_ADD, */                     /* 0x07 */
    "_Mul",/*gcSL_MUL, */                     /* 0x08 */
    "",/*gcSL_RCP, */                     /* 0x09 */
    "_Sub",/*gcSL_SUB, */                     /* 0x0A */
    "",/*gcSL_KILL, */                     /* 0x0B */
    "",/*gcSL_TEXLD, */                     /* 0x0C */
    "",/*gcSL_CALL, */                     /* 0x0D */
    "",/*gcSL_RET, */                     /* 0x0E */
    "",/*gcSL_NORM, */                     /* 0x0F */
    "_max",/*gcSL_MAX, */                     /* 0x10 */
    "_min",/*gcSL_MIN, */                     /* 0x11 */
    "",/*gcSL_POW, */                     /* 0x12 */
    "",/*gcSL_RSQ, */                     /* 0x13 */
    "",/*gcSL_LOG, */                     /* 0x14 */
    "",/*gcSL_FRAC, */                     /* 0x15 */
    "",/*gcSL_FLOOR, */                     /* 0x16 */
    "",/*gcSL_CEIL, */                     /* 0x17 */
    "",/*gcSL_CROSS, */                     /* 0x18 */
    "",/*gcSL_TEXLDPROJ,*/                     /* 0x19 */
    "",/*gcSL_TEXBIAS, */                     /* 0x1A */
    "",/*gcSL_TEXGRAD, */                     /* 0x1B */
    "",/*gcSL_TEXLOD, */                     /* 0x1C */
    "",/*gcSL_SIN, */                     /* 0x1D */
    "",/*gcSL_COS, */                     /* 0x1E */
    "",/*gcSL_TAN, */                     /* 0x1F */
    "",/*gcSL_EXP, */                     /* 0x20 */
    "",/*gcSL_SIGN, */                     /* 0x21 */
    "",/*gcSL_STEP, */                     /* 0x22 */
    "",/*gcSL_SQRT, */                   /* 0x23 */
    "",/*gcSL_ACOS, */                   /* 0x24 */
    "",/*gcSL_ASIN, */                   /* 0x25 */
    "",/*gcSL_ATAN, */                   /* 0x26 */
    "",/*gcSL_SET, */                   /* 0x27 */
    "",/*gcSL_DSX, */                   /* 0x28 */
    "",/*gcSL_DSY, */                   /* 0x29 */
    "",/*gcSL_FWIDTH, */                   /* 0x2A */
    "_Div",/*gcSL_DIV, */                   /* 0x2B */
    "_Mod",/*gcSL_MOD, */                   /* 0x2C */
    "",/*gcSL_AND_BITWISE,*/                   /* 0x2D */
    "",/*gcSL_OR_BITWISE, */                   /* 0x2E */
    "",/*gcSL_XOR_BITWISE,*/                   /* 0x2F */
    "",/*gcSL_NOT_BITWISE,*/                   /* 0x30 */
    "_LeftShift",/*gcSL_LSHIFT, */                   /* 0x31 */
    "_RightShift",/*gcSL_RSHIFT, */                   /* 0x32 */
    "_Rotate",/*gcSL_ROTATE, */                   /* 0x33 */
    "",/*gcSL_BITSEL, */                   /* 0x34 */
    "_clz",/*gcSL_LEADZERO, */                   /* 0x35 */
    "",/*gcSL_LOAD, */                   /* 0x36 */
    "",/*gcSL_STORE, */                   /* 0x37 */
    "",/*gcSL_BARRIER, */                   /* 0x38 */
    "",/*gcSL_STORE1, */                   /* 0x39 */
    "",/*gcSL_ATOMADD, */                   /* 0x3A */
    "",/*gcSL_ATOMSUB, */                   /* 0x3B */
    "",/*gcSL_ATOMXCHG, */                   /* 0x3C */
    "",/*gcSL_ATOMCMPXCHG,*/                   /* 0x3D */
    "",/*gcSL_ATOMMIN, */                   /* 0x3E */
    "",/*gcSL_ATOMMAX, */                   /* 0x3F */
    "",/*gcSL_ATOMOR, */                   /* 0x40 */
    "",/*gcSL_ATOMAND, */                   /* 0x41 */
    "",/*gcSL_ATOMXOR, */                   /* 0x42 */
    "",/*gcSL_TEXLDPCF, */                   /* 0x43 */
    "",/*gcSL_TEXLDPCFPROJ,*/                  /* 0x44 */
    "",/*gcSL_TEXLODQ, */                   /* 0x45  ES31 */
    "",/*gcSL_FLUSH, */                  /* 0x46  ES31 */
    "",/*gcSL_JMP_ANY, */                  /* 0x47  ES31 */
    "",/*gcSL_BITRANGE, */                  /* 0x48  ES31 */
    "",/*gcSL_BITRANGE1, */                  /* 0x49  ES31 */
    "",/*gcSL_BITEXTRACT, */                  /* 0x4A  ES31 */
    "",/*gcSL_BITINSERT, */                  /* 0x4B  ES31 */
    "",/*gcSL_FINDLSB, */                  /* 0x4C  ES31 */
    "",/*gcSL_FINDMSB, */                  /* 0x4D  ES31 */
    "",/*gcSL_IMAGE_OFFSET,*/                  /* 0x4E  ES31 */
    "",/*gcSL_IMAGE_ADDR, */                  /* 0x4F  ES31 */
    "",/*gcSL_SINPI, */                  /* 0x50 */
    "",/*gcSL_COSPI, */                  /* 0x51 */
    "",/*gcSL_TANPI, */                  /* 0x52 */
    "_AddLo",/*gcSL_ADDLO, */                  /* 0x53 */  /* Float only. */
    "_MulLo",/*gcSL_MULLO, */                  /* 0x54 */  /* Float only. */
    "",/*gcSL_CONV, */                  /* 0x55 */
    "",/*gcSL_GETEXP, */                  /* 0x56 */
    "",/*gcSL_GETMANT, */                  /* 0x57 */
    "_MulHi",/*gcSL_MULHI, */                  /* 0x58 */  /* Integer only. */
    "_cmp",/*gcSL_CMP, */                  /* 0x59 */
    "_I2F",/*gcSL_I2F, */                  /* 0x5A */
    "_F2I",/*gcSL_F2I, */                  /* 0x5B */
    "_AddSat",/*gcSL_ADDSAT, */                  /* 0x5C */  /* Integer only. */
    "_SubSat",/*gcSL_SUBSAT, */                  /* 0x5D */  /* Integer only. */
    "_MulSat",/*gcSL_MULSAT, */                  /* 0x5E */  /* Integer only. */
    "",/*gcSL_DP2, */                  /* 0x5F */
    "",/*gcSL_UNPACK, */                  /* 0x60 */
    "",/*gcSL_IMAGE_WR, */                  /* 0x61 */
    "",/*gcSL_SAMPLER_ADD, */                  /* 0x62 */
    "",/*gcSL_MOVA, */                  /* 0x63, HW MOVAR/MOVF/MOVI, VIRCG only */
    "",/*gcSL_IMAGE_RD, */                  /* 0x64 */
    "",/*gcSL_IMAGE_SAMPLER,*/                 /* 0x65 */
    "",/*gcSL_NORM_MUL, */                  /* 0x66  VIRCG only */
    "",/*gcSL_NORM_DP2, */                  /* 0x67  VIRCG only */
    "",/*gcSL_NORM_DP3, */                 /* 0x68  VIRCG only */
    "",/*gcSL_NORM_DP4, */                 /* 0x69  VIRCG only */
    "",/*gcSL_PRE_DIV, */                 /* 0x6A  VIRCG only */
    "",/*gcSL_PRE_LOG2, */                 /* 0x6B  VIRCG only */
    "",/*gcSL_TEXGATHER, */                 /* 0x6C  ES31 */
    "",/*gcSL_TEXFETCH_MS, */                 /* 0x6D  ES31 */
    "_Popcount",/*gcSL_POPCOUNT, */                 /* 0x6E  ES31(OCL1.2)*/
    "",/*gcSL_BIT_REVERSAL, */                 /* 0x6F  ES31 */
    "",/*gcSL_BYTE_REVERSAL,*/                 /* 0x70  ES31 */
    "",/*gcSL_TEXPCF, */                 /* 0x71  ES31 */
    "",/*gcSL_UCARRY, */                 /* 0x72  ES31 UCARRY is a condition op, while gcSL */
    "",/*gcSL_TEXU, */                 /* 0x73  paired with gcSL_TEXLD to implement HW texld_u_plain */
    "",/*gcSL_TEXU_LOD, */                 /* 0x74  paired with gcSL_TEXLD to implement HW texld_u_lod */
    "",/*gcSL_MEM_BARRIER, */                 /* 0x75  Memory Barrier. */
    "",/*gcSL_SAMPLER_ASSIGN,*/                /* 0x76  Sampler assignment as a parameter, only exist on FE. */
    "",/*gcSL_GET_SAMPLER_IDX,*/               /* 0x77  Get Image/Sampler index */
    "",/*gcSL_IMAGE_RD_3D, */                 /* 0x78 */
    "",/*gcSL_IMAGE_WR_3D, */                 /* 0x79 */
    "",/*gcSL_CLAMP0MAX, */                 /* 0x7A clamp0max dest, value, max */
    "",/*gcSL_FMA_MUL, */                 /* 0x7B FMA first part: MUL */
    "",/*gcSL_FMA_ADD, */                 /* 0x7C FMA second part: ADD */
    "",/*gcSL_ATTR_ST, */                 /* 0x7D ATTR_ST attribute(0+temp(1).x), InvocationIndex, val */
    "",/*gcSL_ATTR_LD, */                 /* 0x7E ATTR_LD dest, attribute(0+temp(1).x), InvocationIndex */
    "",/*gcSL_EMIT_VERTEX, */                 /* 0x7F For function "EmitVertex" */
    "",/*gcSL_END_PRIMITIVE,*/                 /* 0x80 For function "EndPrimitive" */
    "",/*gcSL_ARCTRIG0, */                 /* 0x81 For triangle functions */
    "",/*gcSL_ARCTRIG1, */                 /* 0x82 For triangle functions */
    "",/*gcSL_MUL_Z, */                 /* 0x83 special mul, resulting in 0 from inf * 0 */
    "",/*gcSL_NEG, */                 /* 0x84 neg(a) is similar to (0 - (a)) */
    "",/*gcSL_LONGLO, */                 /* 0x85 get the lower 4 bytes of a long/ulong integer */
    "",/*gcSL_LONGHI, */                 /* 0x86 get the upper 4 bytes of a long/ulong integer */
    "",/*gcSL_MOV_LONG, */                 /* 0x87 mov two 4 byte integers to the lower/upper 4 bytes of a long/ulong integer */
    "_MadSat",/*gcSL_MADSAT, */          /* 0x88 mad with saturation for integer only */
    "",/*gcSL_COPY, */                   /* 0x89 copy temp register data */
    };
static const gctSTRING conditionName[] = /*{"_LeftShift", "_RightShift"};*/
    {
    "_always", /*gcSL_ALWAYS*/                                                /* 0x0 */
    "_notEqual", /*gcSL_NOT_EQUAL*/                                             /* 0x1 */
    "_lessEqual", /*gcSL_LESS_OR_EQUAL*/                                         /* 0x2 */
    "_less", /*gcSL_LESS*/                                                  /* 0x3 */
    "_equal", /*gcSL_EQUAL*/                                                 /* 0x4 */
    "_greater", /*gcSL_GREATER*/                                               /* 0x5 */
    "_greaterEqual", /*gcSL_GREATER_OR_EQUAL*/                                      /* 0x6 */
    "_and", /*gcSL_AND*/                                                   /* 0x7 */
    "_or", /*gcSL_OR*/                                                    /* 0x8 */
    "_xor", /*gcSL_XOR*/                                                   /* 0x9 */
    "_notZero", /*gcSL_NOT_ZERO*/                                              /* 0xA */
    "_zero", /*gcSL_ZERO*/                                                  /* 0xB */
    "_greaterEqual0", /*gcSL_GREATER_OR_EQUAL_ZERO*/                                 /* 0xC */
    "_greater0", /*gcSL_GREATER_ZERO*/                                          /* 0xD */
    "_lessEqual0", /*gcSL_LESS_OREQUAL_ZERO*/                                     /* 0xE */
    "_less0", /*gcSL_LESS_ZERO*/                                             /* 0xF */
    };
    gctCHAR name[128] = {'\0'};

    gctUINT srcFormat;
    gctUINT opcode;
    gctUINT dstFormat;
    gcSL_CONDITION condition;

    srcFormat  = gcmSL_SOURCE_GET(GetInstSource0(Instruction), Format);
    dstFormat  = gcmSL_TARGET_GET(Instruction->temp, Format);
    opcode     = gcmSL_OPCODE_GET(Instruction->opcode, Opcode);
    condition  = gcmSL_TARGET_GET(GetInstTemp(Instruction), Condition);

    if (opcode == gcSL_CONV)
    {
        if (((dstFormat == gcSL_UINT64) || (dstFormat == gcSL_INT64)) && (srcFormat == gcSL_FLOAT))
        {
            srcFormat = dstFormat;
            opcode = gcSL_F2I;
        }
        else if (((srcFormat == gcSL_UINT64) || (srcFormat == gcSL_INT64)) && (dstFormat == gcSL_FLOAT))
        {
            opcode = gcSL_I2F;
        }
    }

    /* data type name: long/ulong, */
    index = srcFormat - gcSL_INT64;
    gcoOS_StrCatSafe(name, sizeof(name), typeName[index]);

    /* opcode name. */
    if((opcode == gcSL_CONV) && (srcFormat != gcSL_FLOAT || dstFormat != gcSL_FLOAT))
    {
        if(dstFormat == gcSL_INT8)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2charConvert_sat");
        }
        else if(dstFormat == gcSL_UINT8)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2ucharConvert_sat");
        }
        else if(dstFormat == gcSL_INT16)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2shortConvert_sat");
        }
        else if(dstFormat == gcSL_UINT16)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2ushortConvert_sat");
        }
        else if(dstFormat == gcSL_INT32)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2intConvert_sat");
        }
        else if(dstFormat == gcSL_UINT32)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2uintConvert_sat");
        }
        else if(dstFormat == gcSL_INT64)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2longConvert_sat");
        }
        else if(dstFormat == gcSL_UINT64)
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_2ulongConvert_sat");
        }
        *isI2I = gcvTRUE;
    }
    else if(opcode == gcSL_CMP)
    {
        gcSL_CONDITION condition = gcmSL_TARGET_GET(Instruction->temp, Condition);
        index = opcode - gcSL_NOP;
        gcoOS_StrCatSafe(name, sizeof(name), opName[index]);

        if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT)
        {
            index = condition - gcSL_ALWAYS;
            gcoOS_StrCatSafe(name, sizeof(name), conditionName[index]);
        }
        else if((condition == gcSL_NOT_EQUAL) && (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT))
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_nz");
        }
        else if((condition == gcSL_EQUAL) && (gcmSL_SOURCE_GET(Instruction->source1, Type) == gcSL_CONSTANT))
        {
            gcoOS_StrCatSafe(name, sizeof(name), "_z");
        }
    }
    else
    {
        index = opcode - gcSL_NOP;
        gcoOS_StrCatSafe(name, sizeof(name), opName[index]);
        *isI2I = gcvFALSE;
        if (opcode == gcSL_JMP)
        {
            if (condition != gcSL_NOT_EQUAL && condition != gcSL_EQUAL) /* gcSL_NOT_EQUAL gcSL_EQUAL use other name ruler */
            {
                index = condition - gcSL_ALWAYS;
                gcoOS_StrCatSafe(name, sizeof(name), conditionName[index]);
            }
            else if(gcmSL_SOURCE_GET(Instruction->source1, Type) != gcSL_CONSTANT)
            {
                index = condition - gcSL_ALWAYS;
                gcoOS_StrCatSafe(name, sizeof(name), conditionName[index]);
            }
        }
        else if(opcode == gcSL_F2I)
        {
            gctUINT round = gcmSL_OPCODE_GET((Instruction->opcode), Round);

            if(gcmSL_OPCODE_GET((Instruction->opcode), Sat) == gcSL_SATURATE)
            {
                gcoOS_StrCatSafe(name, sizeof(name), "_sat");
            }

            if(round == gcSL_ROUND_RTNE || round == gcSL_ROUND_RTP || round == gcSL_ROUND_RTN)
            {
                switch(round)
                {
                    case 2:
                        gcoOS_StrCatSafe(name, sizeof(name), "_rte");
                        break;
                    case 3:
                        gcoOS_StrCatSafe(name, sizeof(name), "_rtp");
                        break;
                    case 4:
                        gcoOS_StrCatSafe(name, sizeof(name), "_rtn");
                        break;
                    default:
                        break;
                }
            }
        }
    }

    gcoOS_StrDup(gcvNULL, name, FunctionName);

    return status;
}

static const gctSTRING _divFuncs[] =
{
    "viv_Mul64_32RShift",
    "viv_Mul64HiLo_32RShift",
    "viv_Mul64ThenNeg",
    "viv_Add64",
    "viv_Div_long",
    "viv_Div_ulong",
    "viv_Mod_ulong",
    "viv_Mod_long",
};

static const gctSTRING _mulFuncs[] =
{
    "viv_Mul_long",
    "viv_Mul_ulong",
    "long_Mul",
    "ulong_Mul",
};

static const gctSTRING _madsatFuncs[] =
{
    "viv_Mul_long",
    "viv_Mul_ulong",
    "viv_MulHi_long",
    "viv_MulHi_ulong",
    "long_MadSat",
    "ulong_MadSat",
    "viv_MadSat_long",
    "viv_MadSat_ulong",
};

static const gctSTRING _convFuncs[] =
{
    "viv_I2F_long",
    "viv_I2F_ulong",
    "viv_F2I_long",
    "viv_F2I_ulong",
    "viv_F2I_long_sat",
    "viv_F2I_ulong_sat",
    "viv_F2I_long_sat_rte",
    "viv_F2I_ulong_sat_rte",
    "viv_F2I_long_sat_rtp",
    "viv_F2I_ulong_sat_rtp",
    "viv_F2I_long_sat_rtn",
    "viv_F2I_ulong_sat_rtn",
    "viv_F2I_long_rte",
    "viv_F2I_ulong_rte",
    "viv_F2I_long_rtp",
    "viv_F2I_ulong_rtp",
    "viv_F2I_long_rtn",
    "viv_F2I_ulong_rtn",
};

static const gctSTRING _rotateFuncs[] =
{
    "viv_Rotate64"
};

static const gctSTRING _popcountFuncs[] =
{
    "viv_Popcount"
};

static gceSTATUS
_createLongULongFunction(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcsPatchLongULong *  Patch,
    OUT gcFUNCTION *        NewFunction
    )
{
    /*  What to do:
        1. Get the proper function name;
        2. Get the function from the lib;
        3. Link the function.
    */
    gceSTATUS   status   = gcvSTATUS_OK;
    gctSTRING   convertFuncName;
    gcFUNCTION  convertFunction = gcvNULL;
    gcSL_OPCODE opcode;
    gctBOOL isI2I = gcvFALSE;
    gctINT funcCount = 0;
    gctINT i;
    const gctSTRING *funcNames = gcvNULL;
    gcFUNCTION internalFunc = gcvNULL;

    gcmONERROR(gcGetLongULongFunctionName(&Shader->code[Patch->instructionIndex], &convertFuncName, &isI2I));
    gcmONERROR(gcSHADER_GetFunctionByName(Shader, convertFuncName, &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            convertFuncName,
                                            &convertFunction));
        gcmASSERT(convertFunction != gcvNULL);
    }

    SetFunctionRecompiler(convertFunction);

    opcode = (gcSL_OPCODE)(GetInstOpcode(&Shader->code[Patch->instructionIndex]) & 0xff);
    /* Here we specially handle some functions which call more other internal functions. */
    if ((opcode == gcSL_DIV) ||
        (opcode == gcSL_MOD))
    {
        funcCount = gcmSIZEOF(_divFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_divFuncs[0];
    }
    else if (opcode == gcSL_ROTATE)
    {
        funcCount = gcmSIZEOF(_rotateFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_rotateFuncs[0];
    }
    else if (opcode == gcSL_POPCOUNT)
    {
        funcCount = gcmSIZEOF(_popcountFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_popcountFuncs[0];
    }
    else if ((opcode == gcSL_I2F) ||
             (opcode == gcSL_F2I) ||
             (opcode == gcSL_CONV && isI2I == gcvFALSE))
    {
        funcCount = gcmSIZEOF(_convFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_convFuncs[0];
    }
    else if (opcode == gcSL_MADSAT)
    {
        funcCount = gcmSIZEOF(_madsatFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_madsatFuncs[0];
    }
    else if (opcode == gcSL_MUL)
    {
        funcCount = gcmSIZEOF(_mulFuncs) / gcmSIZEOF(gctSTRING);
        funcNames = &_mulFuncs[0];
    }

    for (i = 0; i < funcCount; i++)
    {
        gcmONERROR(gcSHADER_GetFunctionByName(Shader, funcNames[i], &internalFunc));

        if (internalFunc == gcvNULL)
        {
            /* Link the convert function from library */
            gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                                Library,
                                                funcNames[i],
                                                &internalFunc));
            gcmASSERT(internalFunc != gcvNULL);
        }

        SetFunctionRecompiler(internalFunc);
    }

OnError:
    *NewFunction = convertFunction;
    return status;
}

static gceSTATUS
_createLongULongFunction_jmp(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcsPatchLongULong *  Patch,
    OUT gcFUNCTION *        NewFunction
    )
{
    /*  What to do:
        1. Get the proper function name;
        2. Get the function from the lib;
        3. Link the function.
    */
    gceSTATUS   status   = gcvSTATUS_OK;
    gctSTRING   convertFuncName;
    gcFUNCTION  convertFunction = gcvNULL;
    gctBOOL isI2I = gcvFALSE;

    gcmASSERT(gcmSL_OPCODE_GET(Shader->code[Patch->instructionIndex+Shader->InsertCount].opcode, Opcode) == gcSL_JMP);

    gcmONERROR(gcGetLongULongFunctionName(&Shader->code[Patch->instructionIndex+Shader->InsertCount], &convertFuncName, &isI2I));
    gcmONERROR(gcSHADER_GetFunctionByName(Shader, convertFuncName, &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            convertFuncName,
                                            &convertFunction));
        gcmASSERT(convertFunction != gcvNULL);
    }

    SetFunctionRecompiler(convertFunction);

OnError:
    *NewFunction = convertFunction;
    return status;
}
#endif

gceSTATUS
gcGetReadImageFunctionName(
    IN gcsPatchReadImage *  ReadImage,
    IN gceTexldFlavor       TexldFlavor,
    IN gcSL_FORMAT          DataType,
    IN gcSL_FORMAT          CoordType,
    OUT gctSTRING *         ConvertFuncName
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT                 channelDataType;
    gctUINT                 channelOrder;
    gctUINT                 addressMode;
    gctUINT                 filterMode;
    gctUINT                 normalizeMode;
    gctUINT                 imageType;
    gctCHAR                 name[128] = "_read_image";

    static const gctSTRING  dataTypeName[] = {"_f", "_i", "_b", "_ui"};
    static const gctSTRING  coordTypeName[] = {"_floatcoord", "_intcoord"};
    static const gctSTRING  addressModeName[] = {"_none", "_clamp", "_border", "_wrap", "_mirror"};
    static const gctSTRING  filterModeName[] = {"_nearest", "_linear"};
    static const gctSTRING  normalizeModeName[] = {"_unnorm", "_norm"};
    static const gctSTRING  channelDataTypeName[] =
        {"_snorm8", "_snorm16", "_unorm8", "_unorm16", "_unorm565", "_unorm555", "_unorm101010",
         "_int8", "_int16", "_int32", "_uint8", "_uint16", "_uint32", "_half", "_float"};
    static const gctSTRING  imageTypeName[] = {"_buffer","_2d","_3d","_2DARRAY","_1d","_1darray","_1dbuffer"};
    static const gctSTRING  channelOrderName[] = {"_R", "_A", "_RG", "_RA", "_RGB", "_RGBA", "_BGRA", "_ARGB",
        "_INTENSITY", "_LUMINANCE", "_Rx", "_RGx", "_RGBx"};

    filterMode = (ReadImage->samplerValue >> 8) & 0xF;
    gcmASSERT(filterMode <= 1);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), filterModeName[filterMode]));

    normalizeMode = (ReadImage->samplerValue >> 16) & 0xF;
    gcmASSERT(normalizeMode <= 1);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), normalizeModeName[normalizeMode]));

    addressMode = ReadImage->samplerValue & 0xF;
    gcmASSERT(addressMode <= 4);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), addressModeName[addressMode]));

    gcmASSERT(CoordType <= gcSL_INT32);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), coordTypeName[CoordType]));

    gcmASSERT(DataType <= gcSL_UINT32);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), dataTypeName[DataType]));

    channelDataType = ReadImage->channelDataType;
    gcmASSERT(channelDataType <= 15);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), channelDataTypeName[channelDataType]));

    imageType = ReadImage->imageType & 0xF;
    gcmASSERT(imageType <= 6);
    gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), imageTypeName[imageType]));

    channelOrder = ReadImage->channelOrder & 0xF;
    gcmASSERT(channelOrder <= 12);
    if (ReadImage->channelOrder == 6)
    {
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), channelOrderName[channelOrder]));
    }

    /* dup the name to ConvertFuncName */
    gcmONERROR(gcoOS_StrDup(gcvNULL, name, ConvertFuncName));

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_createReadImageFunction(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcsPatchReadImage *  ReadImage,
    IN gcSL_FORMAT          DataType,
    IN gcSL_FORMAT          CoordType,
    IN gcSL_OPCODE          TexldStatusInstOpcode,
    OUT gcFUNCTION *        NewFunction
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gctSTRING       convertFuncName = gcvNULL;
    gcFUNCTION      convertFunction = gcvNULL;
    gceTexldFlavor  texldFlavor;

    gcmASSERT(Library != gcvNULL);
    if (TexldStatusInstOpcode != gcSL_TEXGRAD)
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, gcSL_NOP);
    }
    else
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, TexldStatusInstOpcode);
    }

    /* get convert function name according to sampler info and
       texld status instruction type */
    gcmONERROR(gcGetReadImageFunctionName(ReadImage,
                                          texldFlavor,
                                          DataType,
                                          CoordType,
                                          &convertFuncName));

    /* Check if convertFunction already exists. */
    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          convertFuncName,
                                          &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            convertFuncName,
                                            &convertFunction));
        gcmASSERT(convertFunction != gcvNULL);
    }

    SetFunctionRecompiler(convertFunction);

OnError:
    if (convertFuncName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, convertFuncName));
    }
    *NewFunction = convertFunction;
    return status;
}

static gceSTATUS
_createWriteImageFunction(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gctUINT              DataType,
    IN gcSL_FORMAT          ColorType,
    IN gctUINT              ImageType,
    IN gctUINT              ChannelOrder,
    OUT gcFUNCTION *        NewFunction
    )
{
    gceSTATUS       status                  = gcvSTATUS_OK;
    gctCHAR         convertFuncName[128]    = "_write_image_";
    gcFUNCTION      convertFunction         = gcvNULL;
    gctSTRING       dataTypeName, colorTypeName;
    gctUINT         imgType;
    gctUINT         channelOrder;
    static const gctSTRING  imageTypeName[] = {"_buffer","_2d","_3d","_2DARRAY","_1d","_1darray","_1dbuffer"};
    static const gctSTRING  swizzleName[] = {"", "", "", "", "", "", "_BGRA", "", "", "", "", "", "", "", "" };

    switch (ColorType)
    {
        case gcSL_FLOAT:
        {
            static const gctSTRING  channelDataTypeName[16] =
            {"char4", "short4", "uchar4", "ushort4", "", "", "",
            "char4", "short4", "", "uchar4", "ushort4", "", "half4", "float4"};
            dataTypeName = channelDataTypeName[DataType & 0xf];
            colorTypeName = "float4_";
            break;
        }
        case gcSL_INTEGER:
        {
            static const gctSTRING  channelDataTypeName[16] =
            {"char4", "short4", "char4", "short4", "", "", "",
            "char4", "short4", "int4", "", "", "", "", ""};
            dataTypeName = channelDataTypeName[DataType & 0xf];
            colorTypeName = "int4_";
            break;
        }
        case gcSL_UINT32:
        {
            static const gctSTRING  channelDataTypeName[16] =
            {"", "", "", "", "", "", "",
            "", "", "", "uchar4", "ushort4", "uint4", "", ""};
            dataTypeName = channelDataTypeName[DataType & 0xf];
            colorTypeName = "uint4_";
            break;
        }

    default:
        colorTypeName = "";
        dataTypeName  = "";
        break;
    }

    if (gcoOS_StrCmp(dataTypeName, "") == gcvSTATUS_OK)
    {
        gcmONERROR(gcoOS_StrCatSafe(convertFuncName,
                                gcmSIZEOF(convertFuncName),
                                "null"));
    }
    else
    {
        gcmONERROR(gcoOS_StrCatSafe(convertFuncName,
                                gcmSIZEOF(convertFuncName),
                                colorTypeName));

        gcmONERROR(gcoOS_StrCatSafe(convertFuncName,
                                gcmSIZEOF(convertFuncName),
                                dataTypeName));
    }

    channelOrder = ChannelOrder & 0xF;
    gcmONERROR(gcoOS_StrCatSafe(convertFuncName, gcmSIZEOF(convertFuncName), swizzleName[channelOrder]));

    imgType = ImageType & 0xF;
    gcmASSERT(imgType <= 6);
    gcmONERROR(gcoOS_StrCatSafe(convertFuncName, gcmSIZEOF(convertFuncName), imageTypeName[imgType]));

    gcmASSERT(Library != gcvNULL);


    /* Check if convertFunction already exists. */
    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          convertFuncName,
                                          &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            convertFuncName,
                                            &convertFunction));
        gcmASSERT(convertFunction != gcvNULL);
    }

    SetFunctionRecompiler(convertFunction);

OnError:
    *NewFunction = convertFunction;
    return status;
}

/* create extra sampler uniform for Multi-layer sampler uniform */
static gceSTATUS
_createMLSamplers(
    IN gcSHADER              Shader,
    IN gcsInputConversion * FormatConversion
    )
{
    gceSTATUS          status         = gcvSTATUS_OK;
    gcUNIFORM          samplerUniform = FormatConversion->samplers[0];
    gctCONST_STRING    samplerName    = samplerUniform->name;
    gctCHAR            name[256];
    gctINT             i;
    static gctINT      ordinal=0;


    for (i=1; i < FormatConversion->layers; i++)
    {
        gcSHADER_TYPE     type     = samplerUniform->u.type;
        gctUINT           offset   = 0;
        gcUNIFORM         uniform;

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(name, sizeof(name), &offset,
                               "#sh_multiLayerTex_%s_layer%d_%d", samplerName, i+1, ordinal++));  /* make it private uniform */

        gcmONERROR(gcSHADER_AddUniform(Shader, name, type, 1, samplerUniform->precision, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_IS_MULTI_LAYER);
        uniform->parent = (gctINT16)samplerUniform->index;
        FormatConversion->samplers[i] = uniform;
    }
OnError:
    /* Return the status. */
    return status;
}

/* create extra outputs for Multi-layer output */
static gceSTATUS
_createMLOutputs(
    IN gcSHADER              Shader,
    IN gcsOutputConversion * OutputConversion
    )
{
    gceSTATUS          status         = gcvSTATUS_OK;
    gctCHAR            name[256];
    gctINT             i;
    gctCONST_STRING    outputName;

    gcmASSERT(OutputConversion->outputs[0] != gcvNULL);
    gcmASSERT(gcSHADER_IsHaltiCompiler(Shader));

    outputName = OutputConversion->outputs[0]->name;

    for (i=1; i < OutputConversion->layers; i++)
    {
        gcSHADER_TYPE      type        = OutputConversion->outputs[0]->type;
        gcSHADER_PRECISION precision   = OutputConversion->outputs[0]->precision;
        gctUINT            offset      = 0;
        gctINT             rows;
        gctUINT            tempRegister;
        gcOUTPUT           newOutput;

        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(name, sizeof(name), &offset,
                               "#%s_layer%d", outputName, Shader->outputCount));  /* make it private uniform */

        /* allocate temp register for the new output */
        rows = gcmType_Rows(type);
        tempRegister = gcSHADER_NewTempRegs(Shader, rows, type);

        gcmONERROR(gcSHADER_AddOutputWithLocation(Shader,
                                                  name,
                                                  type,
                                                  precision,
                                                  gcvFALSE,
                                                  1,
                                                  (gctUINT16)tempRegister,
                                                  OutputConversion->outputs[0]->shaderMode,
                                                  Shader->outputCount,
                                                  -1,
                                                  gcvFALSE,
                                                  gcvFALSE,
                                                  &newOutput));
        /* newly added output is the last one in shader->outputs array */
        OutputConversion->outputs[i] = newOutput;
    }
OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_createSwizzleConvertStubFunction(
    IN gcSHADER              Shader,
    IN gcsInputConversion *  FormatConversion,
    IN gcFUNCTION            SwizzleConvertFunction,
    IN gcFUNCTION            RetFunction,
    IN gcSL_INSTRUCTION      Code,
    IN gcSL_INSTRUCTION      TexldStatusInst,
    IN OUT gctUINT *         RetArgNo
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gctUINT          argNo = 0;
    gctUINT          i;
    gcSL_FORMAT      valueType;
    gcsValue         val0;
    gcSL_INSTRUCTION currentCode;

    if (FormatConversion->samplerInfo.fmtDataType == gcvFORMAT_DATATYPE_SIGNED_INTEGER)
    {
        valueType = gcSL_INT32;
    }
    else if (FormatConversion->samplerInfo.fmtDataType == gcvFORMAT_DATATYPE_UNSIGNED_INTEGER)
    {
        valueType = gcSL_UINT32;
    }
    else
    {
        valueType = gcSL_FLOAT;
    }

    /* Insert argument assignments. */
    /*
    ** If the retFunction is not empty, then use the output of the retFunction;
    ** Otherwise copy the texld and its modifer instruction.
    */
    if (RetFunction != gcvNULL)
    {
        /* Insert the value of TEXLD. */
        _addArgPassByAnotherArg(Shader,
                                SwizzleConvertFunction,
                                argNo++,
                                RetFunction,
                                *RetArgNo,
                                valueType);
    }
    else
    {
        if (TexldStatusInst)
        {
            gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_MOV, 1, gcSL_ENABLE_X, valueType, gcSL_PRECISION_DEFAULT));
            currentCode = Shader->code + Shader->lastInstruction;
            gcoOS_MemCopy(currentCode, TexldStatusInst, sizeof(struct _gcSL_INSTRUCTION));
        }
        gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_MOV, 1, gcSL_ENABLE_X, valueType, gcSL_PRECISION_DEFAULT));
        currentCode = Shader->code + Shader->lastInstruction;
        gcoOS_MemCopy(currentCode, Code, sizeof(struct _gcSL_INSTRUCTION));
        /* Reset the enable. */
        currentCode->temp = gcmSL_TARGET_SET(currentCode->temp, Enable, gcSL_ENABLE_XYZW);

        _addArgPassInst(Shader,
                        SwizzleConvertFunction,
                        gcvNULL,
                        currentCode,
                        argNo++,
                        gcvDest,
                        gcvNULL,
                        gcSL_SWIZZLE_XYZW,
                        gcvFALSE,
                        gcSHADER_PRECISION_DEFAULT);
    }

    /* Insert four channel swizzle. */
    for (i = gcvTEXTURE_COMPONENT_R; i < gcvTEXTURE_COMPONENT_NUM; ++i)
    {
        val0.i32 = FormatConversion->swizzle[i];
        _addArgPassInst(Shader,
                        SwizzleConvertFunction,
                        gcvNULL,
                        gcvNULL,
                        argNo++,
                        gcvIntConstant,
                        &val0,
                        gcSL_SWIZZLE_INVALID,
                        gcvFALSE,
                        gcSHADER_PRECISION_HIGH);
    }
    /* Call the swizzle convert function. */
    _addCallInst(Shader, SwizzleConvertFunction);

    if (RetArgNo)
    {
        *RetArgNo = argNo;
    }

OnError:
    return status;
}

static gceSTATUS
_createFormatConvertStubFunction(
    IN gcSHADER              Shader,
    IN gcsInputConversion *  FormatConversion,
    IN gcFUNCTION            ConvertFunction,
    IN gcFUNCTION            StubFunction,
    IN gcSL_INSTRUCTION      Code,
    IN gcSL_INSTRUCTION      TexldStatusInst,
    OUT gctUINT *            RetArgNo
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gcFUNCTION       stubFunction = StubFunction;
    gctUINT          argNo;
    gctINT           i;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code = Code;
    gcSL_OPCODE      opcode;
    gctINT           type;
    gcSL_OPCODE      texldStatusOpcode = TexldStatusInst ?
                       gcmSL_OPCODE_GET(TexldStatusInst->opcode, Opcode): gcSL_NOP;
    gcsValue         val0;

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    opcode = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);

    /* check if multi-layer sampler uniform is created */
    if (FormatConversion->layers > 1 &&
        FormatConversion->samplers[FormatConversion->layers - 1] == gcvNULL)
    {
        _createMLSamplers(Shader, FormatConversion);
    }

    /* add arguments */
    /* we don't add new arguments to pass value to  stub, since there
       is only one caller per stub, we can reuse the temp variables for
       conversion function parameter directly in stub function:

          10  texld target, sampler, coord

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, sampler
              mov  arg1, coord
              call _convert_func_n
              mov  target, arg2
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    argNo = 0;

    /* create sampler argument:
     * get_sampler_idx  arg0, uniformIndex of sampler */
    _addSamplerArgPassInst(Shader,
        ConvertFunction,
        argNo++,
        FormatConversion->samplers[0],
        FormatConversion->arrayIndex);

    /* create coordinate argument:
     * mov  arg1, coord */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG1*/, gcvSource1, gcvNULL,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* handle other arguments: modifier and lod_bias, they are overloaded
     * with many meanings for different texture load modifiers */
    /* extract the texld status instruction value */
    switch (texldStatusOpcode)
    {
    case gcSL_TEXBIAS:
        val0.i32 = 0x01;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                        argNo++ /*ARG2*/, gcvIntConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        break;
    case gcSL_TEXLOD:
        val0.i32 = 0x02;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                        argNo++ /*ARG2*/, gcvIntConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        break;
    case gcSL_TEXGRAD:
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG2*/, gcvSource0, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        break;
    case gcSL_TEXGATHER:
        if (opcode == gcSL_TEXLDPCF)
        {
            /* vec4 textureGather (sampler2DShadow sampler,
             *                     vec2 P, float refZ)
             * is translated to TEXGATHER/TEXLDPCF pair, pass refZ in lod_bias
             */
            val0.i32 = 0;
            _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                            argNo++ /*ARG2*/, gcvIntConstant, &val0,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        }
        else
        {
            /* gvec4 textureGather (gsampler2D sampler,vec2 P[, int comp]);
             * gvec4 textureGather (gsampler2DArray sampler, vec3 P [, int comp]);
             * gvec4 textureGather (gsamplerCube sampler, vec3 P [, int comp]);
             * is translated to TEXGATHER/TEXLD pair, pass comp in modifier
             */
            /* the component must be interger constant expression */
            gctINT comp;
            gctFLOAT fval;
            gctUINT16 *ptr;
            if (gcmSL_SOURCE_GET(TexldStatusInst->source0, Type) == gcSL_CONSTANT)
            {
                if (gcmSL_SOURCE_GET(TexldStatusInst->source0, Format) == gcSL_FLOAT)
                {
                    ptr = (gctUINT16 *) (&fval);
                    ptr[0] = TexldStatusInst->source0Index;
                    ptr[1] = TexldStatusInst->source0Indexed;
                    comp = (gctINT)fval;
                }
                else if (gcmSL_SOURCE_GET(TexldStatusInst->source0, Format) == gcSL_INTEGER)
                {
                    ptr = (gctUINT16 *) (&comp);
                    ptr[0] = TexldStatusInst->source0Index;
                    ptr[1] = TexldStatusInst->source0Indexed;
                }
                else
                {
                    gcmASSERT(gcvFALSE);
                    return gcvSTATUS_INVALID_ARGUMENT;
                }
                gcmASSERT(comp >= 0 && comp < 4);
                val0.i32 = FormatConversion->swizzle[comp];

                _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                                argNo++ /*ARG2*/, gcvIntConstant, &val0,
                                gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
                val0.f32 = 0.0;
                _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                                argNo++ /*ARG3*/, gcvFloatConstant, &val0,
                                gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            }
            else
            {
                _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                                argNo++ /*ARG2*/, gcvSource0, gcvNULL,
                                gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
                val0.f32 = 0.0;
                _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                                argNo++ /*ARG3*/, gcvFloatConstant, &val0,
                                gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            }
        }
        break;
    case gcSL_TEXFETCH_MS:
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG2*/, gcvSource1, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
        break;
    case gcSL_NOP:
        /* no texld modifier */
        val0.i32 = 0;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                        argNo++ /*ARG2*/, gcvIntConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
        val0.f32 = 0.0;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                        argNo++ /*ARG3*/, gcvFloatConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
        break;
    default:
        gcmASSERT(gcvFALSE);
        break;
    }

    /* create "type" argument */
    if (texldStatusOpcode == gcSL_TEXFETCH_MS)
    {
        type = TEXLDTYPE_FETCHMS;
    }
    else if (texldStatusOpcode == gcSL_TEXGATHER)
    {
        /* special handle texgather */
        type = (opcode == gcSL_TEXLDPCF) ? TEXLDTYPE_GATHERPCF
                                         : TEXLDTYPE_GATHER;
    }
    else if (opcode == gcSL_TEXLDPROJ)
    {
        type = TEXLDTYPE_PROJ;
    }
    else if (opcode == gcSL_TEXLD_U)
    {
        type = TEXLDTYPE_U;
    }
    else
    {
        type = TEXLDTYPE_NORMAL;
    }
    val0.i32 = type;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                    argNo++ /*ARG4*/, gcvIntConstant, &val0,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);

#if DX_SHADER
    /* pass dimension */
    val0.i32 = FormatConversion->dimension;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 5*/, gcvIntConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    /* pass width */
    val0.f32 = (gctFLOAT)FormatConversion->width;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 6*/, gcvFloatConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    /* pass height */
    val0.f32 = (gctFLOAT)FormatConversion->height;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 7*/, gcvFloatConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    /* pass depth */
    val0.f32 = (gctFLOAT)FormatConversion->depth;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 8*/, gcvFloatConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    if (!(FormatConversion->minFilter == FormatConversion->magFilter &&
        FormatConversion->minFilter == gcTEXTURE_MODE_POINT))
    {
        val0.i32 = FormatConversion->magFilter;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
            argNo++ /*ARG 9*/, gcvIntConstant, &val0,
            gcSL_SWIZZLE_INVALID, gcvFALSE);

        val0.i32 = FormatConversion->minFilter;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
            argNo++ /*ARG 10*/, gcvIntConstant, &val0,
            gcSL_SWIZZLE_INVALID, gcvFALSE);
    }

    if (FormatConversion->mipFilter != gcTEXTURE_MODE_NONE)
    {
        val0.f32 = (gctFLOAT)FormatConversion->mipLevelMin;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
            argNo++ /*ARG 11*/, gcvFloatConstant, &val0,
            gcSL_SWIZZLE_INVALID, gcvFALSE);

        val0.f32 = (gctFLOAT)FormatConversion->mipLevelMax;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
            argNo++ /*ARG 12*/, gcvFloatConstant, &val0,
            gcSL_SWIZZLE_INVALID, gcvFALSE);

        val0.f32 = FormatConversion->LODBias;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
            argNo++ /*ARG 13*/, gcvFloatConstant, &val0,
            gcSL_SWIZZLE_INVALID, gcvFALSE);
    }

    val0.i32 = FormatConversion->srgb;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 14*/, gcvIntConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    val0.i32 = FormatConversion->projected;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
        argNo++ /*ARG 15*/, gcvIntConstant, &val0,
        gcSL_SWIZZLE_INVALID, gcvFALSE);

    for (i = gcvTEXTURE_COMPONENT_R; i < gcvTEXTURE_COMPONENT_NUM; ++i)
    {
        if (FormatConversion->swizzle[i] != i)
        {
            break;
        }
    }

    if (i != gcvTEXTURE_COMPONENT_NUM)
    {
        for (i = gcvTEXTURE_COMPONENT_R; i < gcvTEXTURE_COMPONENT_NUM; ++i)
        {
            val0.i32 = FormatConversion->swizzle[i];
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                argNo++, gcvIntConstant, &val0,
                gcSL_SWIZZLE_INVALID, gcvFALSE);
            gcmASSERT(argNo < ConvertFunction->argumentCount);
        }
    }
    else
    {
        for (i = gcvTEXTURE_COMPONENT_R; i < gcvTEXTURE_COMPONENT_NUM; ++i)
        {
            val0.i32 = (gctINT)_SelectSwizzle((gctUINT16)i, code->source0);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                argNo++, gcvIntConstant, &val0,
                gcSL_SWIZZLE_INVALID, gcvFALSE);
            gcmASSERT(argNo < ConvertFunction->argumentCount);
        }
    }
#endif

    for (i = 1; i < FormatConversion->layers; i++)
    {
        /* pass uniform index of corresponding extra multi-layer samplers */
        _addSamplerArgPassInst(Shader,
            ConvertFunction,
            argNo++,
            FormatConversion->samplers[i],
            0);
    }

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* save the ret arg no */
    if (RetArgNo)
    {
        *RetArgNo = argNo;
    }

    return status;
}

static gcSL_SWIZZLE _ConvertShaderTypeToSwizzle(gcSHADER_TYPE type)
{
    switch (type)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FIXED_X1:
    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_UINT_X1:
    case gcSHADER_INT64_X1:
    case gcSHADER_UINT64_X1:
        return gcSL_SWIZZLE_XXXX;
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FIXED_X2:
    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_UINT_X2:
    case gcSHADER_INT64_X2:
    case gcSHADER_UINT64_X2:
        return gcSL_SWIZZLE_XYYY;
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FIXED_X3:
    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_UINT_X3:
    case gcSHADER_INT64_X3:
    case gcSHADER_UINT64_X3:
        return gcSL_SWIZZLE_XYZZ;
    case gcSHADER_FLOAT_X4:
    case gcSHADER_FIXED_X4:
    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X4:
    case gcSHADER_INT64_X4:
    case gcSHADER_UINT64_X4:
    default:
        return gcSL_SWIZZLE_XYZW;
    }
}

static _sourceType _ConvertShaderTypeToSourceType(gcSHADER_TYPE DataType, gcSL_TYPE Type)
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        {
            if (Type == gcSL_TEMP)
            {
                return gcvFloatTempIndex;
            }
            else if (Type == gcSL_CONSTANT)
            {
                return gcvFloatConstant;
            }
            else
            {
                gcmASSERT(Type == gcSL_UNIFORM);
                return gcvFloatUniformIndex;
            }
        }
    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
        {
            if (Type == gcSL_TEMP)
            {
                return gcvIntTempIndex;
            }
            else if (Type == gcSL_CONSTANT)
            {
                return gcvIntConstant;
            }
            else
            {
                gcmASSERT(Type == gcSL_UNIFORM);
                return gcvIntUniformIndex;
            }
        }

    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        {
            if (Type == gcSL_TEMP)
            {
                return gcvUIntTempIndex;
            }
            else if (Type == gcSL_CONSTANT)
            {
                return gcvUIntConstant;
            }
            else
            {
                gcmASSERT(Type == gcSL_UNIFORM);
                return gcvUIntUniformIndex;
            }
        }

    default:
        return gcvFloatTempIndex;
    }
}

static gcFUNCTION
_createOutputConvertStubFunction(
    IN gcSHADER              Shader,
    IN gcsOutputConversion * OutputConversion,
    IN gcFUNCTION            ConvertFunction,
    IN gctUINT               CodeIndex
    )
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gctINT           i;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcSL_SWIZZLE     srcSwizzle;
    gcsValue         val0;
    _sourceType      sourceType;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "#outputConvert%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* check if multi-layer sampler uniform is created */
    if (OutputConversion->layers > 1 &&
        OutputConversion->outputs[OutputConversion->layers - 1] == gcvNULL)
    {
        _createMLOutputs(Shader, OutputConversion);
    }

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /*
          10  call stub_10

          stub_10:
              mov  arg0, outputs[0]'s tempIndex
              call _convert_func_n
              mov  outputs[0], arg1
              ...
              ret
     */
    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    /* mov  arg0, output */
    srcSwizzle = _ConvertShaderTypeToSwizzle(OutputConversion->outputs[0]->type);
    sourceType = _ConvertShaderTypeToSourceType(OutputConversion->outputs[0]->type, gcSL_TEMP);
    val0.i32   = OutputConversion->outputs[0]->tempIndex;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG0*/, sourceType, &val0,
                    srcSwizzle, gcvFALSE, OutputConversion->outputs[0]->precision);


    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);


    for (i = 0; i < OutputConversion->layers; i++)
    {
        /* mov  target, arg1 */
        val0.i32 = OutputConversion->outputs[i]->tempIndex;
        _addRetValueInst(Shader, ConvertFunction, code,
                         argNo++ /*ARG1*/, sourceType, &val0);

    }
    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);


    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

#if _SUPPORT_LONG_ULONG_DATA_TYPE
static gcFUNCTION
_createLongULongStubFunction_src2(
    IN gcSHADER             Shader,
    IN gcsPatchLongULong *  Patch,
    IN gcFUNCTION           ConvertFunction)
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcsValue         val;

    gcmASSERT(Patch);
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "longShift_%u", Patch->instructionIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[Patch->instructionIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);


    /* add arguments */
    /* see image functions, here the similiar logic

          10  LSHIFT target, E0, E1

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, count_of_target_channel
              mov  arg1, E0
              mov  arg2, E1
              call _convert_func_n
              mov  target, arg3
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    val.u32 = Patch->channelCountIndex;
    /* mov  arg0, count */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG0*/,
                    gcvUIntUniformIndex, &val, gcSL_SWIZZLE_INVALID, gcvFALSE, Shader->uniforms[Patch->channelCountIndex]->precision);

    /* mov  arg1, src0 */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG0*/,
                    gcvSource0, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* mov  arg2, src1 */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG1*/,
                    gcvSource1, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* mov  target, arg3
       target should have the same enable bits as the original instruction.
    */
    _addRetValueInst(Shader, ConvertFunction, code, argNo++ /*ARG3*/, gcvDest /*DEST*/, gcvNULL);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

static gcFUNCTION
_createLongULongStubFunction_src1(
    IN gcSHADER             Shader,
    IN gcsPatchLongULong *  Patch,
    IN gcFUNCTION           ConvertFunction)
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcsValue         val;

    gcmASSERT(Patch);
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "longConvert_%u", Patch->instructionIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[Patch->instructionIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);


    /* add arguments */
    /* see image functions, here the similiar logic

          10  LSHIFT target, E0, E1

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, count_of_target_channel
              mov  arg1, E0
              call _convert_func_n
              mov  target, arg2
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    val.u32 = Patch->channelCountIndex;
    /* mov  arg0, count */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG0*/,
                    gcvUIntUniformIndex, &val, gcSL_SWIZZLE_INVALID, gcvFALSE, Shader->uniforms[Patch->channelCountIndex]->precision);

    /* mov  arg1, src0 */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG0*/,
                    gcvSource0, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* mov  target, arg3
       target should have the same enable bits as the original instruction.
    */
    _addRetValueInst(Shader, ConvertFunction, code, argNo++ /*ARG2*/, gcvDest /*DEST*/, gcvNULL);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

static gcFUNCTION
_createLongULongStubFunction_jmp(
    IN gcSHADER             Shader,
    IN gcsPatchLongULong *  Patch,
    IN gcFUNCTION           ConvertFunction,
    IN OUT gctUINT16 *      Index)
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "longjmp_%u", Patch->instructionIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[Patch->instructionIndex+Shader->InsertCount], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    if(gcmSL_TARGET_GET(code->temp, Format) == gcSL_UINT64)
    {
        code->temp = gcmSL_TARGET_SET(code->temp, Format, gcSL_UINT32);
    }
    else if(gcmSL_TARGET_GET(code->temp, Format) == gcSL_INT64)
    {
        code->temp = gcmSL_TARGET_SET(code->temp, Format, gcSL_INT32);
    }

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);


    /* add arguments */
    /* see image functions, here the similiar logic

          10  JMP.ne label, src, 0

          ==>

          10  call stub_10
              jmp.ne label, target, 0

          stub_10:
              mov  arg0, src
              call _convert_func_n
              mov  target, arg1
              ret
     */

    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    /* mov  arg0, src0 */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG0*/,
                    gcvSource0, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* mov  target, arg3
       target should have the same enable bits as the original instruction.
    */
    _addRetValue2NewTemp(Shader, ConvertFunction, code, argNo++ /*ARG1*/, gcvTempReg /*dst*/, Index);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

static gcFUNCTION
_createLongULongStubFunction_jmp_src2(
    IN gcSHADER             Shader,
    IN gcsPatchLongULong *  Patch,
    IN gcFUNCTION           Function,
    IN OUT gctUINT16 *      Index)
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "longjmp_%u", Patch->instructionIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[Patch->instructionIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /*if(gcmSL_TARGET_GET(code->temp, Format) == gcSL_UINT64)
    {
        code->temp = ((code->temp & 0xfff) | (gcSL_UINT32 << 12));
    }
    else if(gcmSL_TARGET_GET(code->temp, Format) == gcSL_INT64)
    {
        code->temp = ((code->temp & 0xfff) | (gcSL_INT32 << 12));
    }*/

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);


    /* add arguments */
    /* see image functions, here the similiar logic

          10  JMP.xx label, src0, src1

          ==>

          10  call stub_10
              jmp.xx label, target, 0

          stub_10:
              mov  arg0, src0
              mov  arg1, src1
              call _convert_func_n
              mov  target, arg2
              ret
     */

    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    /* mov  arg0, src0 */
    _addArgPassInst(Shader, Function, stubFunction, code, argNo++ /*ARG0*/,
                    gcvSource0, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* mov  arg1, src1 */
    _addArgPassInst(Shader, Function, stubFunction, code, argNo++ /*ARG1*/,
                    gcvSource1, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, Function);

    /* mov  target, arg3
       target should have the same enable bits as the original instruction.
    */
    _addRetValue2NewTemp(Shader, Function, code, argNo++ /*ARG2*/, gcvTempReg /*dst*/, Index);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}


static gcFUNCTION
_createLongULongStubFunction(
    IN gcSHADER             Shader,
    IN gcsPatchLongULong *  Patch,
    IN gcFUNCTION           ConvertFunction)
{
    gcSL_INSTRUCTION code;
    gcSL_OPCODE opcode;
    gcFUNCTION  function = gcvNULL;

    gcmASSERT(Patch);
    code   = &Shader->code[Patch->instructionIndex];
    opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
    switch (opcode)
    {
    case gcSL_CMP:
        {
            gcSL_CONDITION condition = gcmSL_TARGET_GET(Shader->code[Patch->instructionIndex].temp, Condition);
            if ((condition == gcSL_EQUAL  || condition == gcSL_NOT_EQUAL) && (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_CONSTANT))
            {
                function = _createLongULongStubFunction_src1(Shader, Patch, ConvertFunction);
            }
            else if (condition == gcSL_LESS_OR_EQUAL ||
                      condition == gcSL_GREATER_OR_EQUAL ||
                      condition == gcSL_LESS ||
                      condition == gcSL_GREATER ||
                      condition == gcSL_EQUAL ||
                      condition == gcSL_NOT_EQUAL)
            {
                function = _createLongULongStubFunction_src2(Shader, Patch, ConvertFunction);
            }
            else
            {
                gcmASSERT(0); /* please make sure your CMP source count */
            }
            break;
        }

    case gcSL_RSHIFT:
    case gcSL_LSHIFT:
    case gcSL_ADD:
    case gcSL_ADDLO:
    case gcSL_ADDSAT:
    case gcSL_SUB:
    case gcSL_SUBSAT:
    case gcSL_MUL:
    case gcSL_MULHI:
    case gcSL_MULLO:
    case gcSL_MULSAT:
    case gcSL_MADSAT:
    case gcSL_DIV:
    case gcSL_MOD:
    case gcSL_ROTATE:
    case gcSL_MAX:
    case gcSL_MIN:
        function = _createLongULongStubFunction_src2(Shader, Patch, ConvertFunction);
        break;

    case gcSL_F2I:
    case gcSL_I2F:
    case gcSL_CONV:
    case gcSL_ABS:
    case gcSL_LEADZERO:
    case gcSL_POPCOUNT:
        function = _createLongULongStubFunction_src1(Shader, Patch, ConvertFunction);
        break;

    default:
        /* Unsupported instruction. */
        gcmASSERT(0);

        break;
    }

    return function;
}
#endif

static gcFUNCTION
_createReadImageStubFunction(
    IN gcSHADER             Shader,
    IN gcsPatchReadImage *  ReadImage,
    IN gcFUNCTION           ConvertFunction,
    IN gctUINT              CodeIndex,
    IN gcSL_INSTRUCTION     TexldStatusInst
    )
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcsValue         val0;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "_readImage_%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /* we don't add new arguments to pass value to  stub, since there
       is only one caller per stub, we can reuse the temp variables for
       conversion function parameter directly in stub function:

          10  texld target, sampler, coord

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, image_data
              mov  arg1, image_size
              mov  arg2, coord
              call _convert_func_n
              mov  target, arg3
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    /* mov  arg0, uniformIndex of image data */
    val0.u32 = ReadImage->imageDataIndex;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG0*/, gcvUIntUniformIndex, &val0,
                    gcSL_SWIZZLE_XYZW, gcvFALSE, Shader->uniforms[ReadImage->imageDataIndex]->precision);

    /* mov  arg1, uniformIndex of image size */
    val0.u32 = ReadImage->imageSizeIndex;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG1*/,
                    gcvUIntUniformIndex,
                    &val0,
                    gcSL_SWIZZLE_XYZW,
                    gcvFALSE,
                    Shader->uniforms[ReadImage->imageSizeIndex]->precision);

    /* mov  arg2, coord */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG3*/,
                    gcvSource1, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* mov  target, arg3 */
    _addRetValueInst(Shader, ConvertFunction, code, argNo++ /*ARG3*/, gcvDest /*DEST*/, gcvNULL);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    /* set TexldStatusInst to NOP, the convertFunction takes care of it now. */
    if (TexldStatusInst)
    {
        /* Shader->code may be resized to a new array, so need to use index. */
        code = &Shader->code[CodeIndex - 1];
        gcSL_SetInst2NOP(code);
    }

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}


static gcFUNCTION
_createWriteImageStubFunction(
    IN gcSHADER              Shader,
    IN gcsPatchWriteImage *  WriteImage,
    IN gcFUNCTION            ConvertFunction,
    IN gctUINT               CodeIndex,
    IN gcSL_INSTRUCTION      TexldStatusInst
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcsValue         val0;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "_writeImage_%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* Add stubFunction to Shader. */
    gcmONERROR(gcSHADER_AddFunction(Shader, funcName, &stubFunction));
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /* we don't add new arguments to pass value to  stub, since there
       is only one caller per stub, we can reuse the temp variables for
       conversion function parameter directly in stub function:

          10  image_wr color, target, coord

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, image_data
              mov  arg1, image_size
              mov  arg2, coord
              mov  arg3, color
              call _convert_func_n
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    gcmONERROR(gcSHADER_BeginFunction(Shader, stubFunction));

    argNo = 0;

    /* mov  arg0, uniformIndex of image data */
    val0.u32 = WriteImage->imageDataIndex;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG0*/,
                    gcvUIntUniformIndex,
                    &val0,
                    gcSL_SWIZZLE_XYZW, gcvFALSE, Shader->uniforms[WriteImage->imageDataIndex]->precision);

    /* mov  arg1, uniformIndex of image size */
    val0.u32 = WriteImage->imageSizeIndex;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code,
                    argNo++ /*ARG1*/,
                    gcvUIntUniformIndex,
                    &val0,
                    gcSL_SWIZZLE_XYZW, gcvFALSE, Shader->uniforms[WriteImage->imageSizeIndex]->precision);


    /* mov  arg2, coord */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG3*/,
                    gcvSource1, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* mov  arg3, color */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG3*/,
                    gcvDest, gcvNULL, gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* ret */
    _addRetInst(Shader);

    gcmONERROR(gcSHADER_EndFunction(Shader, stubFunction));

    /* set TexldStatusInst to NOP, the convertFunction takes care of it now. */
    if (TexldStatusInst)
    {
        /* Shader->code may be resized to a new array, so need to use index. */
        code = &Shader->code[CodeIndex - 1];
        gcSL_SetInst2NOP(code);
    }

OnError:
    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

gceSTATUS
gcGetComparisonFunctionName(
    IN gcsDepthComparison * DepthComparison,
    IN gctBOOL              Sampler2DCoord,
    IN gceTexldFlavor       TexldFlavor,
    OUT gctSTRING *         ConvertFuncName
    )
{
    gceSTATUS       status   = gcvSTATUS_OK;
    gctBOOL         isHalti0 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0);
    gctBOOL         isHalti1 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1);
    gctBOOL         isHalti2 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2);
    gctCHAR         name[128] = "_txpcfcvt";

    if (isHalti2)
    {
        /* do nothing */
    }
    else if (isHalti1 &&
        (DepthComparison->formatInfo.format == gcvSURF_S8D32F_1_G32R32F))
    {
        /* for Halti1, non D32F format needs to clamp the reference depth
           value */
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_S8D32F_1_G32R32F"));
        DepthComparison->convertD32F = gcvTRUE;
    }
    else if (isHalti1 &&
        (DepthComparison->formatInfo.format == gcvSURF_D24S8_1_A8R8G8B8))
    {
        /* for Halti1, non D32F format needs to clamp the reference depth
           value */
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_D24S8"));
        DepthComparison->convertD32F = gcvTRUE;
        gcmASSERT(!Sampler2DCoord); /* we should not see Sampler 2d here, it should only work for 2d array */
    }
    else if (isHalti1 &&
        (DepthComparison->formatInfo.format == gcvSURF_S8D32F_2_A8R8G8B8))
    {
        /* for Halti1, non D32F format needs to clamp the reference depth
           value */
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_S8D32F_2_A8R8G8B8"));
        DepthComparison->convertD32F = gcvTRUE;
        gcmASSERT(!Sampler2DCoord); /* we should not see Sampler 2d here, it should only work for 2d array */
    }
    else if (isHalti0 &&
        (DepthComparison->formatInfo.format == gcvSURF_S8D32F_2_A8R8G8B8))
    {
        /* for Halti0, D32F is not support natively, need to use different
           convert function */
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_D32F"));
        DepthComparison->convertD32F = gcvTRUE;
    }
    if (TexldFlavor != gceTF_GATHER_TEXLD)
    {
        if (Sampler2DCoord)
        {
            gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_2DCoord"));
        }
        else
        {
            gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_3DCoord"));
        }
    }

    /* set flavor */
    if (TexldFlavor != gceTF_NONE)
    {
        gctCONST_STRING flavor = gcTexldFlavor[TexldFlavor];
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), flavor));
    }

    /* dup the name to ConvertFuncName */
    gcmONERROR(gcoOS_StrDup(gcvNULL, name, ConvertFuncName));

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_createDepthComparisonFunction(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcsDepthComparison * DepthComparison,
    IN gcSL_OPCODE          TexldStatusInstOpcode,
    OUT gcFUNCTION *        NewFunction
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gctSTRING       convertFuncName = gcvNULL;
    gcFUNCTION      convertFunction = gcvNULL;
    gceTexldFlavor  texldFlavor;

    if ((TexldStatusInstOpcode != gcSL_TEXGRAD)
        && (TexldStatusInstOpcode != gcSL_TEXGATHER))
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, gcSL_NOP);
    }
    else
    {
        texldFlavor = _getTexldFlavor(gcSL_TEXLD, TexldStatusInstOpcode);
    }

    /* Get convert function name according to sampler type and projection. */
    gcmONERROR(gcGetComparisonFunctionName(DepthComparison,
                                           DepthComparison->sampler->u.type == gcSHADER_SAMPLER_2D_SHADOW,
                                           texldFlavor,
                                           &convertFuncName));

    /* Check if convertFunction already exists. */
    gcmONERROR(gcSHADER_GetFunctionByName(Shader, convertFuncName, &convertFunction));
    if (convertFunction)
    {
        goto OnError;
    }

    /* Link the convert function from library */
    if (Library == gcvNULL)
    {
        goto OnError;
    }
    gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                        Library,
                                        convertFuncName,
                                        &convertFunction));
    gcmASSERT(convertFunction != gcvNULL);

    SetFunctionRecompiler(convertFunction);

OnError:
    if (convertFuncName)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, convertFuncName));
    }

    *NewFunction = convertFunction;
    return status;
}

static gcFUNCTION
_createDepthComparisonStubFunction(
    IN gcSHADER             Shader,
    IN gcsDepthComparison * DepthComparison,
    IN gcUNIFORM            Uniform,
    IN gcFUNCTION           ConvertFunction,
    IN gctUINT              CodeIndex,
    IN gcSL_INSTRUCTION     TexldStatusInst
    )
{
    gceSTATUS        status = gcvSTATUS_OK;
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcSL_OPCODE      opcode;
    gcsValue         val0;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "#depthFormatConvert%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    /* TODO: we need to add indexed sampler support for recompiler. */
    gcmASSERT(gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED);

    opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);

    /* Add stubFunction to Shader. */
    gcmONERROR(gcSHADER_AddFunction(Shader, funcName, &stubFunction));
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /* we don't add new arguments to pass value to  stub, since there
       is only one caller per stub, we can reuse the temp variables for
       conversion function parameter directly in stub function:

          10  texld target, sampler, coord

          ==>

          10  call stub_10


          stub_10:
              mov  arg0, sampler
              mov  arg1, coord
              call _convert_func_n
              mov  target, arg2
              ret
     */

    /*
    gcFUNCTION_AddArgument(stubFunction,
          TempIndex,
          Enable,
          Qualifier);
     */
    gcmONERROR(gcSHADER_BeginFunction(Shader, stubFunction));

    argNo = 0;

    /* get_sampler_idx  arg0, uniformIndex of sampler */
    _addSamplerArgPassInst(Shader,
        ConvertFunction,
        argNo++,
        DepthComparison->sampler,
        DepthComparison->arrayIndex);

    /* mov  arg1, coord */
    _addArgPassInst(Shader, ConvertFunction, stubFunction, code, argNo++ /*ARG1*/, gcvSource1, gcvNULL,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* handle other arguments */
    /* float _txpcfcvt_2DCoord(sampler2D sampler,
                               vec4      coord,
                               int       modifier,
                               float     lod_bias,
                               int       type,
                               int       compareMode,
                               int       compareFunc
                               );
    */
    if (TexldStatusInst)
    {
        gcSL_OPCODE opcode = gcmSL_OPCODE_GET(TexldStatusInst->opcode, Opcode);
        /* extract the texld status instruction value */
        switch (opcode)
        {
        case gcSL_TEXBIAS:
            if (GetUniformType(Uniform) == gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW)
            {
                val0.i32 = 0x00;
            }
            else
            {
                val0.i32 = 0x01;
            }
            _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                            argNo++ /*ARG2*/, gcvIntConstant, &val0,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            break;
        case gcSL_TEXLOD:
            val0.i32 = 0x02;
            _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                            argNo++ /*ARG2*/, gcvIntConstant, &val0,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
            break;
        case gcSL_TEXGRAD:
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG2*/, gcvSource0, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
            break;

        case gcSL_TEXGATHER:
            val0.i32 = 0x03;
            _addArgPassInst(Shader, ConvertFunction, stubFunction, gcvNULL,
                            argNo++ /*ARG2*/, gcvIntConstant, &val0,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
            _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                            argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                            gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);
            break;
        case gcSL_TEXFETCH_MS:
        default:
            gcmASSERT(gcvFALSE);
            break;
        }
    }
    else
    {
        val0.i32 = 0x00;
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG2*/, gcvIntConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
        _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                        argNo++ /*ARG3*/, gcvIntConstant, &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
    }

    val0.i32 = (opcode == gcSL_TEXLDPCF) ? 0x00 : 0x01;
    if (GetUniformType(Uniform) == gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW)
    {
        val0.i32 |= (1 << 3);
    }
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                    argNo++ /*ARG4*/, gcvIntConstant,
                    &val0,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);

    /* pass comparison mode */
    val0.u32 = DepthComparison->compMode;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                    argNo++ /*ARG5*/,
                    gcvIntConstant, &val0,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);

    /* pass comparison function */
    val0.u32 = DepthComparison->compFunction;
    _addArgPassInst(Shader, ConvertFunction, stubFunction, TexldStatusInst,
                    argNo++ /*ARG5*/,
                    gcvIntConstant, &val0,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);

    /* call _convert_func_n */
    _addCallInst(Shader, ConvertFunction);

    /* mov  target, arg2 */
    _addRetValueInst(Shader, ConvertFunction, code, argNo++ /*ARG4*/, gcvDest /*DEST*/, 0);

    /* ret */
    _addRetInst(Shader);

    gcmONERROR(gcSHADER_EndFunction(Shader, stubFunction));

    /* set TexldStatusInst to NOP, the convertFunction takes care of it now. */
    if (TexldStatusInst)
    {
        /* Shader->code may be resized to a new array, so need to use index. */
        code = &Shader->code[CodeIndex - 1];
        gcSL_SetInst2NOP(code);
    }

OnError:
    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

/* supporting SW blend equations */
static
gceSTATUS
_createBlendEquationFunc(
    IN gcSHADER              Shader,
    IN gcSHADER              Library,
    IN gctCONST_STRING       FunctionName,
    OUT gcFUNCTION *         Function
    )
{
    gceSTATUS       status          = gcvSTATUS_OK;
    gcFUNCTION      convertFunction = gcvNULL;

    gcmASSERT(Library != gcvNULL);

    /* Check if blend equation function already exists. */
    gcmONERROR(gcSHADER_GetFunctionByName(Shader,
                                          FunctionName,
                                          &convertFunction));

    if (convertFunction == gcvNULL)
    {
        /* Link the convert function from library */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            FunctionName,
                                            &convertFunction));
        gcmASSERT(convertFunction != gcvNULL);
    }

OnError:

    *Function = convertFunction;
    return status;
}

static gctINT
_findLastDefine(
    IN     gcSHADER          Shader,
    IN     gctINT            outputTempIndex
    )
{

    gctINT     i;
    for (i = (gctINT)Shader->lastInstruction - 1; i >= 0; i--)
    {
        gcSL_INSTRUCTION code = &Shader->code[i];
        if (code->tempIndex == outputTempIndex)
        {
            return (gctINT) i;
        }
    }

    /* is no define legal? */
    return Shader->lastInstruction;
}

static gctBOOL
_shaderHasBuildinAttr(
    IN gcSHADER              Shader,
    IN gceBuiltinNameKind    BuiltinNameKind,
    INOUT  gcATTRIBUTE       *BuiltinAttr)
{
    gctUINT     i;

    for (i = 0; i < Shader->attributeCount; i++) {

        gcATTRIBUTE attr = Shader->attributes[i];

        if (attr == gcvNULL) continue;

        if (attr->nameLength == BuiltinNameKind)
        {
            *BuiltinAttr = attr;
            return gcvTRUE;
        }
    }

    return gcvFALSE;
}

static gctBOOL
_shaderHasPositionAttr(
    IN gcSHADER              Shader,
    INOUT  gcATTRIBUTE       *position)
{
    return _shaderHasBuildinAttr(Shader, gcSL_POSITION, position);
}

static gceSTATUS
_addRtWidthUniform(
    IN OUT gcSHADER             Shader,
    IN OUT gcUNIFORM          * RtWidth
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcUNIFORM    uniform = gcvNULL;
    gctCHAR      name[64];
    gctUINT      offset   = 0;
    gctUINT      i;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_rtWidth");

    /* search for uniform "#sh_rtWidth" in Shader */
    for(i = 0; i < Shader->uniformCount; i++)
    {
        uniform = Shader->uniforms[i];
        if(uniform && uniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, name)))
            {
                break;
            }
        }
    }
    /* uniform "#sh_rtWidth" not found, add it */
    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name, gcSHADER_FLOAT_X1, 1, gcSHADER_PRECISION_HIGH, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    }

    if(RtWidth)
    {
        *RtWidth = uniform;
    }
OnError:
    return status;
}

static gceSTATUS
_addRtHeightUniform(
    IN OUT gcSHADER             Shader,
    IN OUT gcUNIFORM          * RtHeight
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcUNIFORM    uniform = gcvNULL;
    gctCHAR      name[64];
    gctUINT      offset   = 0;
    gctUINT      i;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_rtHeight");

    /* search for uniform "#sh_rtHeight" in Shader */
    for(i = 0; i < Shader->uniformCount; i++)
    {
        uniform = Shader->uniforms[i];
        if(uniform && uniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, name)))
            {
                break;
            }
        }
    }
    /* uniform "#sh_rtHeight" not found, add it */
    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name, gcSHADER_FLOAT_X1, 1, gcSHADER_PRECISION_HIGH, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    }

    if(RtHeight)
    {
        *RtHeight = uniform;
    }
OnError:
    return status;
}

static gceSTATUS
_addBlendSamplerUniform(
    IN OUT gcSHADER             Shader,
    IN OUT gcUNIFORM          * BlendSampler
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcUNIFORM    uniform = gcvNULL;
    gctCHAR      name[64];
    gctUINT      offset   = 0;
    gctUINT      i;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_blend_sampler");

    /* search for uniform "#sh_blend_sampler" in Shader */
    for(i = 0; i < Shader->uniformCount; i++)
    {
        uniform = Shader->uniforms[i];
        if(uniform && uniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, name)))
            {
                break;
            }
        }
    }
    /* uniform "#sh_blend_sampler" not found, add it */
    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name, gcSHADER_SAMPLER_2D, 1, gcSHADER_PRECISION_HIGH, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    }

    if(BlendSampler)
    {
        *BlendSampler = uniform;
    }
OnError:
    return status;
}

static gceSTATUS
_addBlendModeUniform(
    IN OUT gcSHADER             Shader,
    IN OUT gcUNIFORM          * BlendMode
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcUNIFORM    uniform = gcvNULL;
    gctCHAR      name[64];
    gctUINT      offset   = 0;
    gctUINT      i;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_blend_enable_mode");

    /* search for uniform "#sh_blend_enable_mode" in Shader */
    for(i = 0; i < Shader->uniformCount; i++)
    {
        uniform = Shader->uniforms[i];
        if(uniform && uniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(uniform->name, name)))
            {
                break;
            }
        }
    }
    /* uniform "#sh_blend_enable_mode" not found, add it */
    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader, name, gcSHADER_INTEGER_X2, 1, gcSHADER_PRECISION_MEDIUM, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    }

    if(BlendMode)
    {
        *BlendMode = uniform;
    }
OnError:
    return status;
}

static gcFUNCTION
_createBlendStubFunction(
    IN gcSHADER              Shader,
    IN gcFUNCTION            BlendFunction,
    IN gctUINT               CodeIndex,
    IN gctUINT               outputIndex,
    INOUT gctBOOL            *createdUniforms,
    INOUT gcUNIFORM          *blend_sampler,
    INOUT gcUNIFORM          *rt_height,
    INOUT gcUNIFORM          *rt_width,
    INOUT gcUNIFORM          *blend_enable_mode
    )
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gctBOOL          isHalti4 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI4);

    gcSL_SWIZZLE     srcSwizzle;
    gcsValue         val0;
    gcATTRIBUTE      texCoord = gcvNULL;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "_blendEquation%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /*
          10  call _blendEquation1

          _blendEquation1:
              mov  arg0, outputs[i]'s tempIndex
              mov  arg1, new_sampler.x's phsical
              mov  arg2, new_uniform.xy
              call _blend_equation_func
              mov  outputs[i]'tempIndex, arg1
              ret
    */

    gcSHADER_BeginFunction(Shader, stubFunction);

    if (!_shaderHasPositionAttr(Shader, &texCoord))
    {
        gcSHADER_AddAttribute(Shader, "gl_Position", gcSHADER_FLOAT_X4,
                              1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &texCoord);
    }

    if (*createdUniforms == gcvFALSE)
    {
        _addBlendSamplerUniform(Shader, blend_sampler);
        _addBlendModeUniform(Shader, blend_enable_mode);
        if(!isHalti4)
        {
            _addRtWidthUniform(Shader, rt_width);
            _addRtHeightUniform(Shader, rt_height);
        }

        *createdUniforms = gcvTRUE;
    }

    argNo = 0;

    /* mov  arg0, output */
    srcSwizzle = _ConvertShaderTypeToSwizzle(Shader->outputs[outputIndex]->type);
    val0.i32   = Shader->outputs[outputIndex]->tempIndex;
    if (Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X1 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X2 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X3 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X4)
    {
        _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG0*/, gcvFloatTempIndex, &val0, srcSwizzle, gcvFALSE, Shader->outputs[outputIndex]->precision);
    }
    else if (Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X1 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X2 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X3 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X4)
    {
        _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG0*/, gcvIntTempIndex, &val0, srcSwizzle, gcvFALSE, Shader->outputs[outputIndex]->precision);
    }
    else if (Shader->outputs[outputIndex]->type == gcSHADER_UINT_X1 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X2 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X3 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X4)
    {
        _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG0*/, gcvUIntTempIndex, &val0, srcSwizzle, gcvFALSE, Shader->outputs[outputIndex]->precision);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* get_sampler_idx  arg1, sampler */
    _addSamplerArgPassInst(Shader,
        BlendFunction,
        argNo++,
        (*blend_sampler),
        0);

    if(!isHalti4)
    {
        /* mov  arg1, sampler's phsical */
        val0.i32 = (*rt_width)->index;
        srcSwizzle = gcSL_SWIZZLE_XXXX;
        _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG1*/, gcvFloatUniformIndex, &val0, srcSwizzle, gcvFALSE, (*rt_width)->precision);
        val0.i32 = (*rt_height)->index;
        srcSwizzle = gcSL_SWIZZLE_XXXX;
        _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG1*/, gcvFloatUniformIndex, &val0, srcSwizzle, gcvFALSE, (*rt_height)->precision);
    }

    /* mov  arg2, uniform */
    val0.i32 = (*blend_enable_mode)->index;
    srcSwizzle = gcSL_SWIZZLE_XYYY;
    _addArgPassInst(Shader, BlendFunction, stubFunction, gcvNULL, argNo++ /*ARG2*/, gcvIntUniformIndex, &val0, srcSwizzle, gcvTRUE, (*blend_enable_mode)->precision);

    /* call _blend_equation_func */
    _addCallInst(Shader, BlendFunction);

    /* mov  output, arg3 */
    val0.i32 = Shader->outputs[outputIndex]->tempIndex;
    if (Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X1 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X2 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X3 ||
        Shader->outputs[outputIndex]->type == gcSHADER_FLOAT_X4)
    {
        _addRetValueInst(Shader, BlendFunction, gcvNULL, argNo++ /*output*/, gcvFloatTempIndex, &val0);
    }
    else if (Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X1 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X2 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X3 ||
            Shader->outputs[outputIndex]->type == gcSHADER_INTEGER_X4)
    {
        _addRetValueInst(Shader, BlendFunction, gcvNULL, argNo++ /*output*/, gcvIntTempIndex, &val0);
    }
    else if (Shader->outputs[outputIndex]->type == gcSHADER_UINT_X1 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X2 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X3 ||
            Shader->outputs[outputIndex]->type == gcSHADER_UINT_X4)
    {
        _addRetValueInst(Shader, BlendFunction, gcvNULL, argNo++ /*output*/, gcvUIntTempIndex, &val0);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);


    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

#define __RECOMPILER_SHADER_LENGTH__ 131071


static gctSTRING
_getRecompilerShaderSource()
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSTRING * sourceArray = gcvNULL;
    gctPOINTER  pointer = gcvNULL;
    gctSTRING  Halti2FormatConvertLib[] =
    {
        gcLibTexFormatConvertHalti2_Header,
        gcLibTexFormatConvertHalti2_TexCvtUnifiedFunc,
        gcLibTexFormatConvertHalti2_TexCvtFunc,
        gcLibTexFormatConvertHalti2_TexCvtFunc1,
        gcLibTexFormatConvertHalti2_TexPcfCvtFunc,
        gcLibTexFormatConvertHalti2_OutputCvtUnifiedFunc,
        gcLibTexFormatConvertHalti2_OutputCvtFunc,
        gcLibTexFormatConvertHalti2_MainFunc,
        gcvNULL
    };

    gctSTRING  Halti1FormatConvertLib[] =
    {
        gcLibTexFormatConvertHalti1_Header,
        gcLibTexFormatConvertHalti1_TexCvtUnifiedFunc,
        gcLibTexFormatConvertHalti1_TexCvtFunc0,
        gcLibTexFormatConvertHalti1_TexCvtFunc1,
        gcLibTexFormatConvertHalti1_TexPcfCvtFunc,
        gcLibTexFormatConvertHalti1_OutputCvtUnifiedFunc,
        gcLibTexFormatConvertHalti1_OutputCvtFunc,
        gcLibTexFormatConvertHalti1_MainFunc,
        gcvNULL
    };

    gctSTRING  Halti1SinglePipeFormatConvertLib[] =
    {
        gcLibTexFormatConvertHalti1_SinglePipe_Header,
        gcLibTexFormatConvertHalti1_TexCvtUnifiedFunc,
        gcLibTexFormatConvertHalti1_TexCvtFunc0,
        gcLibTexFormatConvertHalti1_TexCvtFunc1,
        gcLibTexFormatConvertHalti1_TexCvtFunc_SinglePipe,
        gcLibTexFormatConvertHalti1_TexPcfCvtFunc,
        gcLibTexFormatConvertHalti1_OutputCvtUnifiedFunc,
        gcLibTexFormatConvertHalti1_OutputCvtFunc,
        gcLibTexFormatConvertHalti1_MainFunc,
        gcvNULL
    };

    gctSTRING  Halti0FormatConvertLib[] =
    {
        gcLibTexFormatConvertHalti0_Header,
        gcLibTexFormatConvertHalti0_TexCvtUnifiedFunc,
        gcLibTexFormatConvertHalti0_TexCvtFunc,
        gcLibTexFormatConvertHalti0_TexPcfCvtFunc,
        gcLibTexFormatConvertHalti0_OutputCvtUnifiedFunc,
        gcLibTexFormatConvertHalti0_OutputCvtFunc,
        gcLibTexFormatConvertHalti0_MainFunc,
        gcvNULL
    };

    /* Get library source based on hardware capability. */
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2))
    {
        sourceArray = Halti2FormatConvertLib;
    }
    else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SINGLE_PIPE_HALTI1))
    {
        sourceArray = Halti1SinglePipeFormatConvertLib;
    }
    else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1))
    {
        sourceArray = Halti1FormatConvertLib;
    }
    else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0))
    {
        sourceArray = Halti0FormatConvertLib;
    }

    if (sourceArray != gcvNULL)
    {
        gctSTRING str;
        gctSIZE_T length;
        gctINT i;
        /* caculate source length */

        length = 2; /* room for '\0' */
        for (i = 0; sourceArray[i] != gcvNULL; i++)
        {
            length += gcoOS_StrLen(sourceArray[i], gcvNULL);
        }
        /* allocate memory for all source strings */
        gcmONERROR(gcoOS_Allocate(gcvNULL, length, &pointer));
        /* cat them together */
        str = (gctSTRING)pointer;
        *str = '\0';
        for (i = 0; sourceArray[i] != gcvNULL; i++)
        {
            gcoOS_StrCatSafe(str, length, sourceArray[i]);
        }
    }

OnError:
    return (gctSTRING)pointer;
}

gceSTATUS
gcLoadESTexFormatConvertLibrary(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSTRING log    = gcvNULL;

    if (gcTexFormatConvertLibrary == gcvNULL)
    {
        gcSHADER library = gcvNULL;
#if __COMPILE_TEX_FORMAT_CONVERT_LIBRARY__
        gctSIZE_T sourceSize;

        if (gcGLSLCompiler == gcvNULL)
        {
            return gcvSTATUS_INVALID_ADDRESS;
        }

        /* Get library source based on hardware capability. */
        RecompilerShaderSource = _getRecompilerShaderSource();

        if (!RecompilerShaderSource)
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

        sourceSize = gcoOS_StrLen(RecompilerShaderSource, gcvNULL);
        status = (*gcGLSLCompiler)(gcvNULL,
                     gcSHADER_TYPE_LIBRARY,
                     sourceSize,
                     RecompilerShaderSource,
                     &library,
                     &log);

        if (status != gcvSTATUS_OK)
        {
            /* report error */
            gcoOS_Print("Compiler Error:\n%s\n", log);
            goto OnError;
        }
#else
        gctCHAR libName[32];
        gctFILE libFile;
        gctUINT32 fileSize;
        gctUINT8_PTR buffer;
        gctSIZE_T bufferSize;

        gcmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, &library));

        /* Load library based on hardware capability. */
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2))
        {
            gcoOS_StrCopySafe(libName, gcmSIZEOF(libName), "libtxcvt2.pgcSL");
        }
        else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1))
        {
            gcoOS_StrCopySafe(libName, gcmSIZEOF(libName), "libtxcvt1.pgcSL");
        }
        else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0))
        {
            gcoOS_StrCopySafe(libName, gcmSIZEOF(libName), "libtxcvt0.pgcSL");
        }
        else
        {
            return gcvSTATUS_NOT_SUPPORTED;
        }

        gcmONERROR(gcoOS_Open(gcvNULL, libName, gcvFILE_READ, &libFile));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_END));
        gcmONERROR(gcoOS_GetPos(gcvNULL, libFile, &fileSize));

        gcmONERROR(gcoOS_Allocate(gcvNULL, fileSize, (gctPOINTER *) &buffer));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_SET));
        gcmONERROR(gcoOS_Read(gcvNULL, libFile, fileSize, buffer, &bufferSize));
        gcmASSERT(fileSize == bufferSize);

        gcmONERROR(gcSHADER_Load(library, buffer, bufferSize));
#endif
        gcTexFormatConvertLibrary = library;
        return gcvSTATUS_OK;
    }

OnError:
    gcmOS_SAFE_FREE(gcvNULL, RecompilerShaderSource);
    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }
    /* Return the status. */
    return status;
}

gceSTATUS
gcSHADER_FreeRecompilerLibrary(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    if (gcTexFormatConvertLibrary)
    {
        gcSHADER_Destroy(gcTexFormatConvertLibrary);
        gcTexFormatConvertLibrary = gcvNULL;

        if (RecompilerShaderSource)
        {
            gcmOS_SAFE_FREE(gcvNULL, RecompilerShaderSource);
        }
    }

    if (gcBuiltinLibrary0)
    {
        gcSHADER_Destroy(gcBuiltinLibrary0);
        gcBuiltinLibrary0 = gcvNULL;
    }

    if (gcBuiltinLibrary1)
    {
        gcSHADER_Destroy(gcBuiltinLibrary1);
        gcBuiltinLibrary1 = gcvNULL;
    }

    if (gcCLBuiltinLibrary)
    {
        gcSHADER_Destroy(gcCLBuiltinLibrary);
        gcCLBuiltinLibrary = gcvNULL;
    }

    if (gcBlendEquationLibrary)
    {
        gcSHADER_Destroy(gcBlendEquationLibrary);
        gcBlendEquationLibrary = gcvNULL;
    }

    gcmFOOTER();
    return status;
}

#if DX_SHADER
static gceSTATUS
_getDXRecompilerShaderBinary(
    OUT gcSHADER *Shader
)
{
    gceSTATUS   status          = gcvSTATUS_OK;
    gctCHAR     libName[260]    = { 0 };
    gctUINT     libNameOffset   = 0;
    gctCHAR     winDir[260]     = { 0 };
    gctFILE     libFile         = gcvNULL;
    gctUINT32   fileSize        = 0;
    gctUINT8_PTR buffer         = gcvNULL;
    gctSIZE_T   bufferSize      = 0;;

    gcmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, Shader));

    /* Really should be in gcOS layer!!!  Since this functions works with char strings we need
       to explicitly call the ANSI version since Windows driver can build with Unicode as default */
    GetWindowsDirectoryA(winDir, 260);

    /* Load library based on hardware capability. */
    /*
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2))
    {
        gcoOS_PrintStrSafe(libName, gcmSIZEOF(libName), &libNameOffset,
            "%s\\system32\\libtxcvt2.pgcSL", winDir);
    }
    else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_SINGLE_PIPE_HALTI1))
    {
        gcoOS_PrintStrSafe(libName, gcmSIZEOF(libName), &libNameOffset,
            "%s\\system32\\libtxcvt1_pipe.pgcSL", winDir);
    }
    else if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI1))
    {
        gcoOS_PrintStrSafe(libName, gcmSIZEOF(libName), &libNameOffset,
            "%s\\system32\\libtxcvt1.pgcSL", winDir);
    }
    else
    */
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0))
    {
        gcoOS_PrintStrSafe(libName, gcmSIZEOF(libName), &libNameOffset,
            "%s\\system32\\libtxcvt0.pgcSL", winDir);
    }
    else
    {
        gcmASSERT(gcvFALSE);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    gcmONERROR(gcoOS_Open(gcvNULL, libName, gcvFILE_READ, &libFile));
    gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_END));
    gcmONERROR(gcoOS_GetPos(gcvNULL, libFile, &fileSize));

    gcmONERROR(gcoOS_Allocate(gcvNULL, fileSize, (gctPOINTER *) &buffer));
    gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_SET));
    gcmONERROR(gcoOS_Read(gcvNULL, libFile, fileSize, buffer, &bufferSize));
    gcmASSERT(fileSize == bufferSize);

    gcmONERROR(gcSHADER_Load(*Shader, buffer, bufferSize));

OnError:

    if (libFile != gcvNULL)
    {
        gcoOS_Close(gcvNULL, libFile);
    }

    if (buffer != gcvNULL)
    {
        gcoOS_Free(gcvNULL, buffer);
    }

    return status;
}

gceSTATUS
gcLoadDXTexFormatConvertLibrary(
    void
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSTRING log    = gcvNULL;

    if (gcTexFormatConvertLibrary == gcvNULL)
    {
        if ((status = _getDXRecompilerShaderBinary(&gcTexFormatConvertLibrary)) != gcvSTATUS_OK)
        {
            /* report error */
            gcoOS_Print("Compiler Error:\n%s\n", log);
            goto OnError;
        }

        return gcvSTATUS_OK;
    }

OnError:
    /* Return the status. */
    return status;
}
#endif

static gcsATOM_PTR  _RecompileLockRef = gcvNULL;
static gctPOINTER   _RecompileLock = gcvNULL;

#define _gcmLockLoadLibrary(Status)  do { \
    if (_RecompileLock == gcvNULL) { \
           if(_RecompileLockRef != gcvNULL) (Status) = gcvSTATUS_INVALID_OBJECT; \
           else (Status) = gcvSTATUS_OK; \
    } \
    else (Status) = gcoOS_AcquireMutex(gcvNULL, _RecompileLock, gcvINFINITE); \
   } while (gcvFALSE)

#define _gcmUnlockLoadLibrary(Status)  do { \
    if (_RecompileLock == gcvNULL) { \
           if(_RecompileLockRef != gcvNULL) (Status) = gcvSTATUS_INVALID_OBJECT; \
           else (Status) = gcvSTATUS_OK; \
    } \
    else (Status) = gcoOS_ReleaseMutex(gcvNULL, _RecompileLock); \
   } while (gcvFALSE)

/*******************************************************************************************
**  Initialize recompilation
*/
gceSTATUS
gcInitializeRecompilation(void)
{
    gctINT32 reference;
    gceSTATUS status;

    gcmHEADER();

    if (_RecompileLockRef == gcvNULL)
    {
        /* Create a new reference counter. */
        gcmONERROR(gcoOS_AtomConstruct(gcvNULL, &_RecompileLockRef));
    }

    /* Increment the reference counter */
    gcmONERROR(gcoOS_AtomIncrement(gcvNULL, _RecompileLockRef, &reference));

    if (reference == 0)
    {
        /* Create a global lock. */
        status = gcoOS_CreateMutex(gcvNULL, &_RecompileLock);

        if (gcmIS_ERROR(status))
        {
            _RecompileLock = gcvNULL;
        }
    }

OnError:
    gcmFOOTER_ARG("status=%d", status);
    return status;

}

/*******************************************************************************************
**  Finalize recompilation.
*/
gceSTATUS
gcFinalizeRecompilation(void)
{
    gctINT32 reference = 0;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER();

    /* _RecompileLockRef could be NULL when Construction failed. */
    if(_RecompileLockRef != gcvNULL)
    {
        /* Decrement the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, _RecompileLockRef, &reference));
    }

    if (reference == 1)
    {
        /* Delete the global lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, _RecompileLock));

        /* Destroy the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, _RecompileLockRef));

        _RecompileLockRef = gcvNULL;
    }

    gcmFOOTER_ARG("status=%d", status);
    return status;
}

gceSTATUS
gcLoadCLPatchLibrary(
    void
    )
{
    gceSTATUS   status          = gcvSTATUS_OK;
    gctSTRING   gcCLPatchSource[CL_LIB_COUNT] = {gcvNULL};
    gctINT      i, j;

    _gcmLockLoadLibrary(status);
    gcmONERROR(status);
    if (gcCLPatchLibrary[0] == gcvNULL)
    {
        gcSHADER    library;
#if __COMPILE_CL_PATCH_LIBRARY__
        gctSTRING   CLPatchLib[][24] =
        {
            {
                gcLibCLImage_ReadFunc_1D,
                gcLibCLImage_ReadFunc_1DARRAY,
                gcLibCLImage_ReadFunc_1DBUFFER,
                gcLibCLImage_ReadFunc_2D,
                gcLibCLImage_ReadFunc_2DARRAY,
                gcLibCLImage_ReadFunc_3D,
                gcLibCLImage_ReadFuncF_NORM_1D,
                gcLibCLImage_ReadFuncF_NORM_1DBUFFER,
                gcLibCLImage_ReadFuncF_NORM_1DARRAY1,
                gcLibCLImage_ReadFuncF_NORM_1DARRAY2,
                gcLibCLImage_ReadFuncF_NORM_2D,
                gcLibCLImage_ReadFuncF_NORM_2DARRAY1,
                gcLibCLImage_ReadFuncF_NORM_2DARRAY2,
                gcLibCLImage_ReadFuncF_NORM_3D0,
                gcLibCLImage_ReadFuncF_NORM_3D1,
                gcLibCLImage_ReadFuncF_UNNORM_1D,
                gcLibCLImage_ReadFuncF_UNNORM_1DARRAY,
                gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER,
                gcLibCLImage_ReadFuncF_UNNORM_2D,
                gcLibCLImage_ReadFuncF_UNNORM_2DARRAY,
                gcLibCLImage_ReadFuncF_UNNORM_3D,

                gcLibCLImage_WriteFunc,
                gcLibCLImage_WriteFunc_BGRA,
                gcLibCLPatch_MainFunc
            },
            {
                gcLibCLImage_ReadFunc_1D_BGRA,
                gcLibCLImage_ReadFunc_1DARRAY_BGRA,
                gcLibCLImage_ReadFunc_1DBUFFER_BGRA,
                gcLibCLImage_ReadFunc_2D_BGRA,
                gcLibCLImage_ReadFunc_2DARRAY_BGRA,
                gcLibCLImage_ReadFunc_3D_BGRA,
                gcLibCLImage_ReadFuncF_NORM_1D_BGRA,
                gcLibCLImage_ReadFuncF_NORM_1DBUFFER_BGRA,
                gcLibCLImage_ReadFuncF_NORM_1DARRAY1_BGRA,
                gcLibCLImage_ReadFuncF_NORM_1DARRAY2_BGRA,
                gcLibCLImage_ReadFuncF_NORM_2D_BGRA1,
                gcLibCLImage_ReadFuncF_NORM_2D_BGRA2,
                gcLibCLImage_ReadFuncF_NORM_2DARRAY1_BGRA,
                gcLibCLImage_ReadFuncF_NORM_2DARRAY2_BGRA,
                gcLibCLImage_ReadFuncF_NORM_3D0_BGRA,
                gcLibCLImage_ReadFuncF_NORM_3D1_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_1D_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_1DARRAY_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_2D_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_2DARRAY_BGRA,
                gcLibCLImage_ReadFuncF_UNNORM_3D_BGRA,
                "",
                ""
            },
#if _SUPPORT_LONG_ULONG_DATA_TYPE
            {
                gcLibCLLong_Func,
                gcLibCLLong_Func1,
                gcLibCLLong_Func2,
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
                "",
            }
#endif
        };
        gctSIZE_T   sourceSize, length;
        gctPOINTER  pointer = gcvNULL;
        gctSTRING   log       = gcvNULL;
        gctINT      stringNum;
        gctINT      patchLen;

        if (gcCLCompiler == gcvNULL)
        {
            _gcmUnlockLoadLibrary(status);
            return gcvSTATUS_INVALID_ADDRESS;
        }

        for (j = 0; j < CL_LIB_COUNT; j++)
        {
            stringNum = sizeof(CLPatchLib[j]) / sizeof(gctSTRING);
            patchLen = stringNum;
            for (i = 0; i < stringNum; i++)
            {
                patchLen += gcoOS_StrLen(CLPatchLib[j][i], gcvNULL);
            }
            gcmONERROR(gcoOS_Allocate(gcvNULL, patchLen, &pointer));
            gcCLPatchSource[j] = (gctSTRING)pointer;

            /* Get library source based on hardware capability. */
            length = gcoOS_StrLen(CLPatchLib[j][0], gcvNULL);
            gcoOS_StrCopySafe(gcCLPatchSource[j], length + 1, CLPatchLib[j][0]);

            for (i = 1; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(gcCLPatchSource[j],
                    patchLen, CLPatchLib[j][i]);
            }

            sourceSize = gcoOS_StrLen(gcCLPatchSource[j], gcvNULL);
            status = (*gcCLCompiler)(gcvNULL,
                                     sourceSize,
                                     gcCLPatchSource[j],
                                     "",
                                     &library,
                                     &log);

            if (status != gcvSTATUS_OK)
            {
                /* report error */
                gctSIZE_T strLen = gcoOS_StrLen(log, gcvNULL);
                gctSIZE_T outLen = 1024;
                char orig = '\0';
                char* start = log;
                gcoOS_Print("Compiler Error:\n");
                do {
                    if (strLen > outLen)
                    {
                        orig = start[outLen];
                        start[outLen] = '\0';
                    }
                    gcoOS_Print("%s\n", start);
                    if (strLen > outLen)
                    {
                        start[outLen] = orig;
                        start+=outLen;
                    }
                    else
                    {
                        break;
                    }
                    strLen = gcoOS_StrLen(start, gcvNULL);
                }while(strLen > 0);
                goto OnError;
            }

            gcCLPatchLibrary[j] = library;
        }

#else
        gctCHAR libName[32];
        gctFILE libFile;
        gctUINT32 fileSize;
        gctUINT8_PTR buffer;
        gctSIZE_T bufferSize;

        gcmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, &library));

        /* Load library based on hardware capability. */
        gcoOS_StrCopySafe(libName, gcmSIZEOF(libName), "libclpatch.pgcSL");

        gcmONERROR(gcoOS_Open(gcvNULL, libName, gcvFILE_READ, &libFile));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_END));
        gcmONERROR(gcoOS_GetPos(gcvNULL, libFile, &fileSize));

        gcmONERROR(gcoOS_Allocate(gcvNULL, fileSize, (gctPOINTER *) &buffer));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_SET));
        gcmONERROR(gcoOS_Read(gcvNULL, libFile, fileSize, buffer, &bufferSize));
        gcmASSERT(fileSize == bufferSize);

        gcmONERROR(gcSHADER_Load(library, buffer, bufferSize));
#endif
    }
    _gcmUnlockLoadLibrary(status);
    gcmONERROR(status);

OnError:
    for (i = 0; i < CL_LIB_COUNT; i++)
    {
        if (gcCLPatchSource[i])
        {
            gcmOS_SAFE_FREE(gcvNULL, gcCLPatchSource[i]);
        }
    }
    /* Return the status. */
    return status;
}

gceSTATUS
gcFreeCLPatchLibrary(
    void
    )
{
    gctINT i;
    for (i = 0; i < CL_LIB_COUNT; i++)
    {
        if (gcCLPatchLibrary[i])
        {
            /* Free library. */
            gcSHADER_Destroy(gcCLPatchLibrary[i]);
            gcCLPatchLibrary[i] = gcvNULL;
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_patchColorFactoring(
    IN OUT gcSHADER             Shader,
    IN gcsPatchColorFactoring * FormatConversion
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcOUTPUT    output = gcvNULL;
    gctINT      i;

    /* only need to patch fragment shader */
    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
        return status;

    /* find the output with specified location
     */
    for (i = 0; i < (gctINT)Shader->outputCount; ++i)
    {
        if (Shader->outputs[i] == gcvNULL) continue;
        if (Shader->outputs[i]->location == FormatConversion->outputLocation)
        {
            output = Shader->outputs[i];
            /* the output should not be gl_Depth */
            gcmASSERT(output->nameLength != gcSL_DEPTH);
            break;
        }
    }

    /* found output to patch */
    if (output != gcvNULL)
    {
        gcSL_FORMAT format      = gcGetFormatFromType(output->type);
        gctINT      components  = gcmType_Comonents(output->type);
        gcSL_ENABLE enable      = (components == 1) ? gcSL_ENABLE_X    :
                                  (components == 2) ? gcSL_ENABLE_XY   :
                                  (components == 3) ? gcSL_ENABLE_XYZ  :
                                                      gcSL_ENABLE_XYZW;
        gcSL_SWIZZLE swizzle    = (components == 1) ? gcSL_SWIZZLE_XXXX :
                                  (components == 2) ? gcSL_SWIZZLE_XYYY :
                                  (components == 3) ? gcSL_SWIZZLE_XYZZ :
                                                      gcSL_SWIZZLE_XYZW;
        gcUNIFORM    uniform                      = gcvNULL;
        gcsValue     constValue;
        gctINT                      tempCodeIndex = 0;
        gctUINT                     lastInstruction;
        gcSHADER_INSTRUCTION_INDEX  instrIndex    = 0;
        gctUINT16                   newTemp       = 0;

        /* set the const value */
        constValue.f32_v4[0] = constValue.f32_v4[1] =
            constValue.f32_v4[2] =constValue.f32_v4[3] = 0.0;
        if (components >= 1)
            constValue.f32_v4[0] = FormatConversion->value[0];
        if (components >= 2)
            constValue.f32_v4[1] = FormatConversion->value[1];
        if (components >= 3)
            constValue.f32_v4[2] = FormatConversion->value[2];
        if (components >= 4)
            constValue.f32_v4[3] = FormatConversion->value[3];

        /* create uniform and initialize it with constValue */
        gcSHADER_CreateConstantUniform(Shader,
                                       gcmType_ComonentType(output->type),
                                       &constValue,
                                       &uniform);

        /* create one new temp register */
        newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

        /* insert 2 NOPs to the end of main() */
        tempCodeIndex = _insertNOP2Main(Shader, 2);

        lastInstruction = Shader->lastInstruction;
        instrIndex = Shader->instrIndex;
        Shader->lastInstruction = tempCodeIndex;
        Shader->instrIndex = gcSHADER_OPCODE;

        /* create MUL newTemp, temp, factor */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MUL,
                           newTemp,
                           enable,
                           format,
                           gcSHADER_PRECISION_HIGH);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           output->tempIndex,
                           swizzle,
                           format,
                           output->precision);
        gcSHADER_AddSourceUniformFormatted(Shader,
                                           uniform,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           format);

        /* create MOV temp, newTemp */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MOV,
                           output->tempIndex,
                           enable,
                           format,
                           output->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           newTemp,
                           gcSL_SWIZZLE_XYZW,
                           format,
                           gcSHADER_PRECISION_HIGH);

        Shader->lastInstruction = lastInstruction;
        Shader->instrIndex = instrIndex;
    }

    return status;
}

static gceSTATUS
_patchAlphaBlending(
    IN OUT gcSHADER             Shader,
    IN gcsPatchAlphaBlending *  FormatConversion
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcOUTPUT    output = gcvNULL;
    gctINT      i;

    /* only need to patch fragment shader */
    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
        return status;

    /* find the output with specified location
     */
    for (i = 0; i < (gctINT)Shader->outputCount; ++i)
    {
        if (Shader->outputs[i] == gcvNULL) continue;
        if (Shader->outputs[i]->location == FormatConversion->outputLocation)
        {
            output = Shader->outputs[i];
            /* The output should not be gl_Depth */
            gcmASSERT(output->nameLength != gcSL_DEPTH);
            break;
        }
    }

    /* found output to patch */
    if (output != gcvNULL)
    {
        gcSL_FORMAT format      = gcGetFormatFromType(output->type);
        gctINT      components  = gcmType_Comonents(output->type);
        gcSL_ENABLE enable      = (components == 1) ? gcSL_ENABLE_X    :
                                  (components == 2) ? gcSL_ENABLE_XY   :
                                  (components == 3) ? gcSL_ENABLE_XYZ  :
                                                      gcSL_ENABLE_XYZW;
        gcSL_SWIZZLE swizzle    = (components == 1) ? gcSL_SWIZZLE_XXXX :
                                  (components == 2) ? gcSL_SWIZZLE_XYYY :
                                  (components == 3) ? gcSL_SWIZZLE_XYZZ :
                                                      gcSL_SWIZZLE_XYZW;
        gctINT                      tempCodeIndex;
        gctUINT                     lastInstruction;
        gcSHADER_INSTRUCTION_INDEX  instrIndex;
        gctUINT16 newTemp;

        /* create one new temp register */
        newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

        /* insert 2 NOPs to the end of main() */
        tempCodeIndex = _insertNOP2Main(Shader, 2);

        lastInstruction = Shader->lastInstruction;
        instrIndex = Shader->instrIndex;
        Shader->lastInstruction = tempCodeIndex;
        Shader->instrIndex = gcSHADER_OPCODE;

        /* create MUL newTemp, temp, temp.w */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MUL,
                           newTemp,
                           enable,
                           format,
                           output->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           output->tempIndex,
                           swizzle,
                           format,
                           output->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           output->tempIndex,
                           gcSL_SWIZZLE_WWWW,
                           format,
                           output->precision);

        /* create MOV temp, newTemp */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MOV,
                           output->tempIndex,
                           enable,
                           format,
                           output->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           newTemp,
                           gcSL_SWIZZLE_XYZW,
                           format,
                           output->precision);

        Shader->lastInstruction = lastInstruction;
        Shader->instrIndex = instrIndex;
    }

    return status;
}

gctINT
_insertNOP2Shader(
    IN OUT gcSHADER          Shader,
    IN     gctINT            InsertAtInst,
    IN     gctUINT           Num
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctINT      origCodeCount = (gctINT)Shader->lastInstruction;
    gctUINT     i;

    gcmASSERT(InsertAtInst >= 0 &&
              InsertAtInst <=  (gctINT)Shader->lastInstruction);

    /* force the shader to increase instruction count */
    Shader->instrIndex = gcSHADER_SOURCE1;

    for (i=0; i < Num; i++)
    {
        /* add NOP to the end of the shader */
        gcmONERROR(gcSHADER_AddOpcode(Shader, gcSL_NOP, 0, 0, 0, 0));
    }

    if (InsertAtInst < origCodeCount)
    {
        gcSHADER_LABEL label;
        /* main is no the at the end of Shader, need to patch
         * the code after main():
         *   1. move the code down by one code
         *   2. set the last Num instructions in main() to NOP
         *   3. increase resolved branch/call target after main's end by one
         *      for all code (including the code before main's end)
         *   4. adjust function start and end instruction
         *   5. adjust shader label's reference
         */

        /* 1. move the code down by Num code */
        for (i = 0; i < (gctUINT)(origCodeCount - InsertAtInst); i++)
        {
            Shader->code[origCodeCount + Num - 1 - i] = Shader->code[origCodeCount - 1 - i];
        }

        /* 2. set the Num instructions start from InsertAtInst to NOP */
        for (i = 0; i < Num; i++)
        {
            gcSL_SetInst2NOP(Shader->code + InsertAtInst + i);
        }

        /* 3. increase the branch/call target after main's end by one
         *    for all code (including the code before main's end)
         */
        for (i = 0 ; i < (gctUINT)Shader->codeCount; i++)
        {
            gcSL_INSTRUCTION  code = &Shader->code[i];
            gcSL_OPCODE  opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
            if ((opcode == gcSL_JMP || opcode == gcSL_CALL) &&
                (code->tempIndex >= InsertAtInst) &&
                (code->tempIndex < Shader->lastInstruction))
            {
                code->tempIndex += (gctINT16)Num;
            }
        }
        /* 4. adjust function start and end instruction */
        for (i = 0; i < (gctUINT)Shader->functionCount; ++i)
        {
            if (IsFunctionIntrinsicsBuiltIn(Shader->functions[i]))
                continue;

            if (Shader->functions[i]->codeStart >= (gctUINT) InsertAtInst)
                Shader->functions[i]->codeStart += Num;
        }
        for (i = 0; i < (gctUINT)Shader->kernelFunctionCount; ++i)
        {
            if (Shader->kernelFunctions[i]->codeStart >= (gctUINT)InsertAtInst)
                Shader->kernelFunctions[i]->codeStart += Num;
        }

        /* 5. adjust shader label's defined */
        for (label = Shader->labels; label != gcvNULL; label = label->next)
        {
            gcSHADER_LINK link;

            if (label->defined >= (gctUINT)InsertAtInst)
                label->defined += Num;
            link = label->referenced;
            while (link)
            {
                if (link->referenced >= (gctUINT)InsertAtInst)
                    link->referenced += Num;
                link = link->next;
            }
        }

    }

    Shader->instrIndex = gcSHADER_OPCODE;

OnError:
    /* Return the mainEnd. */
    return InsertAtInst;
}

/* insert Num of NOPs to the end of main() funciton, adjust shader
 * JMP/CALL instrucitons to right label after the insertion,
 * the index to start of added code (original end of main) is returned
 */
gctINT
_insertNOP2Main(
    IN OUT gcSHADER          Shader,
    IN     gctUINT            Num
    )
{
    gctINT      mainStart = -1;
    gctINT      mainEnd = -1;

    /* find the main function start and end code index */
    gcSHADER_FindMainFunction(Shader, &mainStart, &mainEnd);
    gcmASSERT(mainStart !=  -1 && mainEnd >= 0);

    return _insertNOP2Shader(Shader, mainEnd, Num);
}

/* insert Num of NOPs to the begin of main() funciton, adjust shader
 * JMP/CALL instrucitons to right label after the insertion,
 * the index to start of added code (original begin of main) is returned
 */
gctINT
_insertNOP2MainBegin(
    IN OUT gcSHADER          Shader,
    IN     gctUINT           Num
    )
{
    gctINT      mainStart = -1;
    gctINT      mainEnd = -1;

    /* find the main function start and end code index */
    gcSHADER_FindMainFunction(Shader, &mainStart, &mainEnd);
    gcmASSERT(mainStart !=  -1 && mainEnd >= 0);

    return _insertNOP2Shader(Shader, mainStart, Num);
}

static gceSTATUS
_addDepthBiasUniform(
    IN OUT gcSHADER             Shader,
    IN gcsPatchDepthBias *      DepthBias
    )
{
    gceSTATUS    status = gcvSTATUS_OK;
    gcUNIFORM    uniform;
    gctCHAR      name[64];
    gctUINT      offset   = 0;

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh%d_DepthBias",
                       Shader->_id, Shader->_constVectorId++);
    gcmONERROR(gcSHADER_AddUniform(Shader, name, gcSHADER_FLOAT_X1, 1, gcSHADER_PRECISION_HIGH, &uniform));
    SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
    DepthBias->depthBiasUniform = uniform;

OnError:
    return status;
}

static gceSTATUS
_patchDepthBias(
    IN OUT gcSHADER             Shader,
    IN gcsPatchDepthBias *      DepthBias
    )
{
    gceSTATUS                   status = gcvSTATUS_OK;
    gcOUTPUT                    position = gcvNULL;
    gctINT                      i;
    gctINT                      tempCodeIndex;
    gctUINT                     lastInstruction;
    gcSHADER_INSTRUCTION_INDEX  instrIndex;

    /* only need to patch vertex shader */
    if (Shader->type != gcSHADER_TYPE_VERTEX)
        return status;

    /* find the output with specified location
     */
    for (i = 0; i < (gctINT)Shader->outputCount; ++i)
    {
        if (Shader->outputs[i] == gcvNULL) continue;
        if (Shader->outputs[i]->nameLength == gcSL_POSITION)
        {
            position = Shader->outputs[i];
            break;
        }
    }

    gcmASSERT(position != gcvNULL);

    /* insert 3 NOPs to the end of main() */
    tempCodeIndex = _insertNOP2Main(Shader, 3);

    lastInstruction = Shader->lastInstruction;
    instrIndex = Shader->instrIndex;
    Shader->lastInstruction = tempCodeIndex;
    Shader->instrIndex = gcSHADER_OPCODE;

    _addDepthBiasUniform(Shader, DepthBias);

    /* found output to patch */
    if (DepthBias->depthBiasUniform != gcvNULL)
    {
        gctUINT16 newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 2, gcSHADER_FLOAT_X2);

        /* create MUL temp.x, position.w, bias */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MUL,
                           newTemp,
                           gcSL_ENABLE_X,
                           gcSL_FLOAT,
                           position->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           position->tempIndex,
                           gcSL_SWIZZLE_WWWW,
                           gcSL_FLOAT,
                           position->precision);
        gcSHADER_AddSourceUniformFormatted(Shader,
                                           DepthBias->depthBiasUniform,
                                           gcSL_SWIZZLE_XXXX,
                                           gcSL_NOT_INDEXED,
                                           gcSL_FLOAT);
        /* create ADD  temp1, temp, position.a */
        gcSHADER_AddOpcode(Shader,
                           gcSL_ADD,
                           (newTemp+1),
                           gcSL_ENABLE_X,
                           gcSL_FLOAT,
                           position->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           newTemp,
                           gcSL_SWIZZLE_XXXX,
                           gcSL_FLOAT,
                           position->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           position->tempIndex,
                           gcSL_SWIZZLE_ZZZZ,
                           gcSL_FLOAT,
                           position->precision);
        /* create MOV  position.z, temp1 */
        gcSHADER_AddOpcode(Shader,
                           gcSL_MOV,
                           position->tempIndex,
                           gcSL_ENABLE_Z,
                           gcSL_FLOAT,
                           position->precision);
        gcSHADER_AddSource(Shader,
                           gcSL_TEMP,
                           (newTemp+1),
                           gcSL_SWIZZLE_XXXX,
                           gcSL_FLOAT,
                           position->precision);
    }
    else
    {
        gcmASSERT(gcvFALSE);
    }

    Shader->lastInstruction = lastInstruction;
    Shader->instrIndex = instrIndex;

    return status;
}

extern gctUINT8
_Enable2SwizzleWShift(
    IN gctUINT32 Enable
    );

/* yInverted recompilation
 *
 * 1) driver will create directive to tell compile the pixel shader need
 *    yInverted for some inputs (gl_FragCoord.y, gl_FrontFacing, gl_PointCoord.y)
 *
 * 2) compiler add following code at begining of fragment shader
 *    If (position_yInverted == TRUE)
 *    {
 *      gl_FragCoord.y = rtHeight -gl_FragCoord.y ;
 *    }
 *    if (frontFacing_yInverted == TRUE)
 *    {
 *      gl_FrontFacing  = !gl_FrontFacing;
 *    }
 *    if (pointCoord_yInverted == TRUE)
 *    {
 *      gl_PointCoord.y = 1.0 -gl_PointCoord.y;
 *    }
 *
 * 2) compiler need have a hint to tell driver if this program need to be
 *    aware of yInverted:
 *       gctBOOL yInverted;
 *
 *    to indicate if PS is referring to any of above 3 variables.
 *
 * 3) compiler need to add a private uniform(rtHeight) and specify the
 *    special SUB uniform usage to tell driver where the uniform is if
 *    position_yInverted is true
 *
 */
static gceSTATUS
_patchYFlippedShader(
    IN OUT gcSHADER                 Shader,
    IN OUT gcsPatchYFilppedShader * YFilppedShader
    )
{
    gceSTATUS                   status      = gcvSTATUS_OK;
    gcATTRIBUTE                 position    = gcvNULL;
    gcATTRIBUTE                 frontFacing = gcvNULL;
    gcATTRIBUTE                 pointCoord  = gcvNULL;
    gctINT                      tempCodeIndex;
    gcSHADER_INSTRUCTION_INDEX  instrIndex = 0;
    gctINT                      i;
    gctUINT                     lastInstruction = Shader->lastInstruction;
    gctINT                      addNopCount = 0;
    gctUINT16                   newTemp;
    gcShaderCodeInfo            codeInfo;
    gctBOOL                     useDSY;
    const gctFLOAT              constantZero = 0.0;
    const gctFLOAT              constantOne  = 1.0;

    /* only apply the yFlip to fragment shader */
    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
        return status;

    /* find the attributes with specified location
     */
    for (i = 0; i < (gctINT)Shader->attributeCount; ++i)
    {
        if (Shader->attributes[i] == gcvNULL) continue;
        if (Shader->attributes[i]->nameLength == gcSL_POSITION)
        {
            position = Shader->attributes[i];
            addNopCount += 2;
        }
        if (Shader->attributes[i]->nameLength == gcSL_FRONT_FACING)
        {
            frontFacing = Shader->attributes[i];
            addNopCount += 1;
        }
        if (Shader->attributes[i]->nameLength == gcSL_POINT_COORD)
        {
            pointCoord = Shader->attributes[i];
            addNopCount += 2;
        }
    }
    gcoOS_ZeroMemory(&codeInfo, gcmSIZEOF(codeInfo));
    gcSHADER_CountCode(Shader, &codeInfo);
    useDSY = (codeInfo.codeCounter[gcSL_DSY] != 0);

    gcmASSERT(position     != gcvNULL ||
              frontFacing  != gcvNULL ||
              pointCoord   != gcvNULL ||
              useDSY       != gcvFALSE  );

    gcmASSERT(addNopCount != 0 || useDSY != gcvFALSE);

    if (position     != gcvNULL ||
        frontFacing  != gcvNULL ||
        pointCoord   != gcvNULL    )
    {
        /* insert NOPs to the begin of main() */
        tempCodeIndex = _insertNOP2MainBegin(Shader, addNopCount);

        lastInstruction = Shader->lastInstruction;
        instrIndex = Shader->instrIndex;
        Shader->lastInstruction = tempCodeIndex;
        Shader->instrIndex = gcSHADER_OPCODE;

        if (position)
        {
            _addRtHeightUniform(Shader, &YFilppedShader->rtHeight);
        }
        else
        {
            YFilppedShader->rtHeight = gcvNULL;
        }

        /* patch outputs */
        if (position)
        {
            /* gl_FragCoord.y = rtHeight - gl_FragCoord.y */
            /* create a new temp register */
            newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
            _ChangeAttribToTempForAllCodes(Shader, position->index, newTemp);

            /* MOV  temp.xyzw, gl_PointCoord.xyzw */
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                         gcSL_MOV,
                                         newTemp,
                                         gcSL_ENABLE_XYZW,
                                         gcSL_FLOAT,
                                         position->precision));

            gcSHADER_AddSourceAttributeFormatted(Shader,
                                                 position,
                                                 gcSL_SWIZZLE_XYZW,
                                                 0,
                                                 gcSL_FLOAT);
            /* SUB: temp.y, rtHeight.x, gl_FragCoord.y */
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                         gcSL_SUB,
                                         newTemp,
                                         gcSL_ENABLE_Y,
                                         gcSL_FLOAT,
                                         position->precision));
            gcSHADER_AddSourceUniformFormatted(Shader,
                                               YFilppedShader->rtHeight,
                                               gcSL_SWIZZLE_XXXX,
                                               0,
                                               gcSL_FLOAT);
            gcSHADER_AddSourceAttributeFormatted(Shader,
                                                 position,
                                                 gcSL_SWIZZLE_YYYY,
                                                 0,
                                                 gcSL_FLOAT);
        }
        if (frontFacing)
        {
            /* gl_FrontFacing  = !gl_FrontFacing */
            /* create a new temp register */
            newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X1);
            _ChangeAttribToTempForAllCodes(Shader, frontFacing->index, newTemp);
            /* SET.eq temp.x, gl_FrontFacing, 0.0 */
            gcmONERROR(gcSHADER_AddOpcodeConditionIndexed(Shader,
                                                           gcSL_SET,
                                                           gcSL_EQUAL,
                                                           newTemp,
                                                           gcSL_ENABLE_X,
                                                           gcSL_NOT_INDEXED,
                                                           0,
                                                           gcSL_FLOAT,
                                                           gcSHADER_PRECISION_MEDIUM));
            gcSHADER_AddSourceAttributeFormatted(Shader,
                                                 frontFacing,
                                                 gcSL_SWIZZLE_XXXX,
                                                 0,
                                                 gcSL_FLOAT);

            gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                                                            (void *)&constantZero,
                                                            gcSL_FLOAT));
        }

        if (pointCoord != gcvNULL)
        {
            /* gl_PointCoord.y = 1.0 -gl_PointCoord.y */
            /* create a new temp register */
            newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X2);
            _ChangeAttribToTempForAllCodes(Shader, pointCoord->index, newTemp);

            /* MOV  temp.xy, gl_PointCoord.xy */
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                         gcSL_MOV,
                                         newTemp,
                                         gcSL_ENABLE_XY,
                                         gcSL_FLOAT,
                                         pointCoord->precision));

            gcSHADER_AddSourceAttributeFormatted(Shader,
                                                 pointCoord,
                                                 gcSL_SWIZZLE_XYYY,
                                                 0,
                                                 gcSL_FLOAT);

            /* SUB: temp.y, 1.0, gl_PointCoord.y */
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                         gcSL_SUB,
                                         newTemp,
                                         gcSL_ENABLE_Y,
                                         gcSL_FLOAT,
                                         pointCoord->precision));
            gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                                                            (void *)&constantOne,
                                                            gcSL_FLOAT));
            gcSHADER_AddSourceAttributeFormatted(Shader,
                                                 pointCoord,
                                                 gcSL_SWIZZLE_YYYY,
                                                 0,
                                                 gcSL_FLOAT);
        }
        Shader->lastInstruction = lastInstruction;
        Shader->instrIndex = instrIndex;
    }
    if (useDSY)
    {
        gctUINT          instIdx;
        gcSL_INSTRUCTION code;
        gcSL_OPCODE      opcode;

        /* v = dsy(x) ==> v = -dsy(x) */
        for (instIdx = 0; instIdx < Shader->codeCount; instIdx ++)
        {
            code = &Shader->code[instIdx];
            opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);

            if (opcode == gcSL_DSY)
            {
                gctUINT dst = code->tempIndex;
                gcSL_INDEXED dst_indexed = (gcSL_INDEXED)gcmSL_TARGET_GET(code->temp, Indexed);
                gctUINT dst_indexedIndex = code->tempIndexed;
                gcSL_ENABLE enable = gcmSL_TARGET_GET(code->temp, Enable);
                gcSL_SWIZZLE swizzle = _Enable2SwizzleWShift((gctUINT32)enable);
                gcSHADER_PRECISION precision = gcmSL_TARGET_GET(code->temp, Precision);

                /* DSY dst, src
                 *    change dst to newTemp */
                newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
                code->tempIndex = newTemp;
                /* reset indexed */
                code->temp          = gcmSL_TARGET_SET(code->temp, Indexed, 0);
                code->tempIndexed   = 0;

                /* insert a NOP after instIdx */
                tempCodeIndex = _insertNOP2Shader(Shader, instIdx+1, 1);

                lastInstruction = Shader->lastInstruction;
                instrIndex = Shader->instrIndex;
                Shader->lastInstruction = tempCodeIndex;
                Shader->instrIndex = gcSHADER_OPCODE;

                /* SUB dst, 0.0, newTemp */
                gcmONERROR(gcSHADER_AddOpcodeIndexed(Shader,
                                                     gcSL_SUB,
                                                     (gctUINT16)dst,
                                                     enable,
                                                     dst_indexed,
                                                     (gctUINT16)dst_indexedIndex,
                                                     gcSL_FLOAT,
                                                     precision));

                gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                                                               (void *)&constantZero,
                                                               gcSL_FLOAT));

                gcmONERROR(gcSHADER_AddSource(Shader,
                                              gcSL_TEMP,
                                              newTemp,
                                              swizzle,
                                              gcSL_FLOAT,
                                              precision));
                Shader->lastInstruction = lastInstruction;
                Shader->instrIndex = instrIndex;
            }
        }
    }

OnError:
    return status;
}

static gcOUTPUT
_findFirstRTOutput(IN gcSHADER Shader)
{
    /* find the output which is the first render target */
    gcOUTPUT rt = gcvNULL;
    gctUINT i;

    for (i=0; i < Shader->outputCount; i++)
    {
        if (Shader->outputs[i] && GetOutput2RTIndex(Shader->outputs[i]) == 0)
        {
            rt = Shader->outputs[i];
            break;
        }
    }
    return rt;
}

/* find the temp register assigned to gl_vertexID variable */
gcOUTPUT
_findSubsampleDepthTemp(
    IN gcSHADER Shader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcOUTPUT    subsampleDepth  = gcvNULL;
    gctUINT     i;

    gcmASSERT(Shader->type == gcSHADER_TYPE_FRAGMENT);
    for (i = 0; i < Shader->outputCount; i++)
    {
        gcOUTPUT output = Shader->outputs[i];

        /* attributes could be null if it is not used */
        if (output == gcvNULL)
            continue;
        if (output->nameLength == (gctINT)gcSL_SUBSAMPLE_DEPTH)
        {
            subsampleDepth = output;
        }
    }

    if (subsampleDepth == gcvNULL)
    {
        gctUINT16 t = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_UINT_X4);
        gcmONERROR(gcSHADER_AddOutput(Shader, "#Subsample_Depth", gcSHADER_UINT_X4, 1, t, gcSHADER_PRECISION_MEDIUM));
        gcmONERROR(gcSHADER_AddOutputLocation(Shader, 0, 1));

        subsampleDepth = Shader->outputs[Shader->outputCount - 1];
    }

OnError:
    return subsampleDepth;
}


static gcFUNCTION
_createSampleMaskStubFunction(
    IN gcSHADER             Shader,
    IN gcsPatchSampleMask * SampleMask,
    IN gcFUNCTION           SampleMaskFunction,
    IN gctUINT              CodeIndex
    )
{
    gctCHAR          funcName[32];
    gctUINT          offset         = 0;
    gcFUNCTION       stubFunction    = gcvNULL;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode = gcvNULL;
    gctPOINTER       pointer = gcvNULL;
    gcSL_INSTRUCTION code;
    gcsValue         val0;
    gcOUTPUT         subsampleDepth;

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "_sampleMaskStub_%d", CodeIndex));

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[CodeIndex], sizeof(struct _gcSL_INSTRUCTION));
    code = tempCode;

    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);
    SetFunctionRecompilerStub(stubFunction);

    /* add arguments */
    /*  void _viv_sampleMask(in    float alphaValue,
     *                       in    int   sampleConverageEnabled,
     *                       in    vec2  sampleConverage_Invert,
     *                       in    uint  sampleMask,
     *                       inout vec4  subsampleDepthReg);
     *
     *
     *
     *    _sampleMaskStub_10:
     *         mov  arg0, alphaValue
     *         mov  arg1, sampleConverageEnabled
     *         mov  arg2, sampleConverage_Invert
     *         mov  arg3, sampleMask
     *         mov  arg4  subsampleDepthReg
     *         call _convert_func_n
     *         mov  implicitMaskReg, arg4
     *         ret
     *
     *  The implicit mask register was never accessed in shader program before,
     *  now the sample mask forced the register (subsample_depth) be exposed to
     *  the shader code, the register is the last regrister reported to the GPU
     *  for single 32 mode, or last 2 register for dual 16 mode.
     *  In V542 cmodel, the 4 inverted subsample mask bits are the MSB bits in
     *  <x, y, z, w> channels of the subsample_depth register.
     */

    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    /*  mov  arg0, alphaValue */
    if (SampleMask->alphaToConverageEnabled)
    {
        gcOUTPUT output = _findFirstRTOutput(Shader);
        gcmASSERT(output != gcvNULL);
        val0.i32 = GetOutputTempIndex(output);
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG0*/,
                        gcvFloatTempIndex,
                        &val0,
                        gcSL_SWIZZLE_WWWW, gcvFALSE, GetUniformPrecision(output));
    }
    else
    {
        /* if alpha converage is not enabled, same as alpha value is 1.0 */
        val0.f32 = 1.0 ;
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG0*/,
                        gcvFloatConstant,
                        &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
    }

    /* mov  arg1, sampleConverageEnabled */
    val0.i32 = SampleMask->sampleConverageEnabled ? 1 : 0;
    _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code, argNo++ /*ARG1*/,
                    gcvIntConstant, &val0,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);

    /* mov  arg2, sampleConverage_Invert */
    if (SampleMask->sampleConverageEnabled)
    {
        val0.i32 = GetUniformIndex(SampleMask->sampleCoverageValue_Invert);
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG2*/, gcvFloatUniformIndex, &val0,
                        gcSL_SWIZZLE_XYYY, gcvFALSE, GetUniformPrecision(SampleMask->sampleCoverageValue_Invert));
    }
    else
    {
        /* if sampl converage is not enabled, pass 0.0 */
        val0.f32 = 0.0 ;
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG2*/,
                        gcvFloatConstant,
                        &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
    }

    /* mov  arg3, sampleMask */
    if (SampleMask->sampleMaskEnabled)
    {

        val0.i32 = GetUniformIndex(SampleMask->sampleMaskValue);
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG3*/,
                        gcvUIntUniformIndex,
                        &val0,
                        gcSL_SWIZZLE_WWWW, gcvFALSE, GetUniformPrecision(SampleMask->sampleMaskValue));
    }
    else
    {
        /* if sampleMask is not enabled, same as sampleMask is 0x0f */
        val0.u32 = 0x0f ;
        _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                        argNo++ /*ARG3*/,
                        gcvUIntConstant,
                        &val0,
                        gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_HIGH);
    }

    /* add special variable subsample_depth, which later will be assgined to
     * laster register */
    subsampleDepth = _findSubsampleDepthTemp(Shader);
    gcmASSERT(subsampleDepth != gcvNULL);

    val0.i32 = GetOutputTempIndex(subsampleDepth);
    _addArgPassInst(Shader, SampleMaskFunction, stubFunction, code,
                    argNo /*ARG4*/,
                    gcvUIntTempIndex,
                    &val0,
                    gcSL_SWIZZLE_XYZW, gcvFALSE, GetOutputPrecision(subsampleDepth));

    /* call _convert_func_n */
    _addCallInst(Shader, SampleMaskFunction);


    /* mov  target, arg2 */
    _addRetValueInst(Shader, SampleMaskFunction, code,
                     argNo /*ARG4*/,
                     gcvFloatTempIndex,
                     &val0);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    return stubFunction;
}

/*
 * There 3 operations are orthogonal and can be turn on/off separately.
 * So we have one directive for
 *   o alpha_to_coverage, if it’s enabled, with final alpha value,
 *     shader should generate a bitmask, easiest way is just to scale
 *     the alpha floating value to 0 ~ 0xf.
 *   o sample_to_coverage, need a uniform to have a sample value
 *     (float, like a global alpha) and a flag indicate if need to
 *     invert the generated bitmask. The way to map sample value to
 *     bitmask can be same as the one used in alpha_to_coverage.
 *   o sampleMask, we need a uniform to have the 4 bit mask.
 *
 * When get the 4 bit mask, ps can access the last temp register’s
 * MSB to mask them out.
 */
static gceSTATUS
_patchSampleMask(
    IN OUT gcSHADER                 Shader,
    IN OUT gcsPatchSampleMask *     SampleMask
    )
{
    gceSTATUS       status      = gcvSTATUS_OK;
    gcUNIFORM       uniform;

    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
    {
        /* only need to patch fragment shader */
        return status;
    }
    /* sample coverage */
    if (SampleMask->sampleConverageEnabled)
    {
        /* creat sampleCoverageValue_Invert uniform:
         *  .x => float sampleCoverageValue;
         *  .y => float isInvert;
         */
        gcmONERROR(gcSHADER_FindAddUniform(Shader, "#sampleCoverageValue_Invert",
                                           gcSHADER_FLOAT_X2, 1, gcSHADER_PRECISION_HIGH, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SampleMask->sampleCoverageValue_Invert = uniform;
    }

    /* sample mask */
    if (SampleMask->sampleMaskEnabled)
    {
        /* creat sampleCoverageValue_Invert uniform:
         *  .x => uint sampleMaskValue;
         */
        gcmONERROR(gcSHADER_FindAddUniform(Shader, "#sampleMaskValue",
                                           gcSHADER_UINT_X1, 1, gcSHADER_PRECISION_HIGH, &uniform));
        SetUniformFlag(uniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SampleMask->sampleMaskValue = uniform;
    }

    /* create call to _viv_sampleMask(float alphaValue,
     *                                bool  sampleConverageEnabled,
     *                                vec2  sampleConverage_Invert,
     *                                uint  sampleMask);
     */

    {
        gcFUNCTION  sampleMaskFunction = gcvNULL;
        gcFUNCTION  stubFunction = gcvNULL;
        gctUINT     tempCodeIndex;
        gctUINT lastInstruction;
        gcSHADER_INSTRUCTION_INDEX instrIndex;

        if (gcTexFormatConvertLibrary == gcvNULL)
        {
            gcmONERROR(gcLoadTexFormatConvertLibrary());
        }

        /* insert NOPs to the end of main() */
        tempCodeIndex = _insertNOP2Main(Shader, 2);

        gcmONERROR(_FindFunctionFromShaderOrLibrary(Shader,
                                                 gcTexFormatConvertLibrary,
                                                 "_viv_sampleMask",
                                                 &sampleMaskFunction));

        SetFunctionRecompiler(sampleMaskFunction);

        /* Construct call stub function. */
        stubFunction = _createSampleMaskStubFunction(
                            Shader,
                            SampleMask,
                            sampleMaskFunction,
                            tempCodeIndex);


        lastInstruction = Shader->lastInstruction;
        instrIndex = Shader->instrIndex;
        Shader->lastInstruction = tempCodeIndex;
        Shader->instrIndex = gcSHADER_OPCODE;
        _addCallInst(Shader, stubFunction);
        Shader->lastInstruction = lastInstruction;
        Shader->instrIndex = instrIndex;
    }

OnError:
    return status;
}

static gceSTATUS
_patchSignExtent(
    IN OUT gcSHADER                 Shader,
    IN OUT gcsPatchSignExtent *     SignExtent
    )
{
    gceSTATUS       status      = gcvSTATUS_OK;

    return status;
}

static gceSTATUS
_patchTCSInputMismatch(
    IN OUT gcSHADER                            Shader,
    IN gcsPatchTCSInputCountMismatch*      InputMismatch
    )
{
    gceSTATUS       status      = gcvSTATUS_OK;

    if (GetShaderType(Shader) == gcSHADER_TYPE_TCS)
    {
        /* driver send in the number of registers that are used to save these remap
           two channels per component */
        Shader->shaderLayout.tcs.tcsPatchInputVertices = InputMismatch->inputVertexCount;
        Shader->useDriverTcsPatchInputVertices = gcvTRUE;
    }

    return status;
}

static gceSTATUS
_patchColorKill(
    IN OUT gcSHADER                 Shader,
    IN OUT gcsPatchColorKill  *     ColorKill
    )
{
    gceSTATUS                       status = gcvSTATUS_OK;
    gcOUTPUT                        output = gcvNULL;
    gctINT                          i;
    gctINT                          tempCodeIndex = 0;
    gctUINT                         lastInstruction;
    gcSHADER_INSTRUCTION_INDEX      instrIndex    = 0;
    gctFLOAT                        constZero = 0.0;
    gctUINT16                       newTempIndex;

    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
    {
        /* only need to patch fragment shader */
        return status;
    }

    for (i = 0; i < (gctINT)Shader->outputCount; ++i)
    {
        if (Shader->outputs[i] == gcvNULL ||
                Shader->outputs[i]->nameLength != gcSL_COLOR)
        {
            continue;
        }
        output = Shader->outputs[i];
        break;
    }

    if (output == gcvNULL)
        return status;

    tempCodeIndex = _insertNOP2Main(Shader, 2);
    lastInstruction = Shader->lastInstruction;
    instrIndex = Shader->instrIndex;
    Shader->lastInstruction = tempCodeIndex;
    Shader->instrIndex = gcSHADER_OPCODE;

    /* create one new temp register */
    newTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

    /* DP4: newTemp, output, output. */
    gcmONERROR(gcSHADER_AddOpcode(Shader,
                                  gcSL_DP4,
                                  newTempIndex,
                                  gcSL_ENABLE_X,
                                  gcSL_FLOAT,
                                  output->precision));
    gcSHADER_AddSource(Shader,
                       gcSL_TEMP,
                       output->tempIndex,
                       gcSL_SWIZZLE_XYZW,
                       gcSL_FLOAT,
                       output->precision);
    gcSHADER_AddSource(Shader,
                       gcSL_TEMP,
                       output->tempIndex,
                       gcSL_SWIZZLE_XYZW,
                       gcSL_FLOAT,
                       output->precision);

    /* TEXKILL.eq: newTemp, 0.0. */
    gcmONERROR(gcSHADER_AddOpcodeConditionIndexed(Shader,
                                                  gcSL_KILL,
                                                  gcSL_EQUAL,
                                                  newTempIndex,
                                                  gcSL_ENABLE_X,
                                                  gcSL_NOT_INDEXED,
                                                  0,
                                                  gcSL_FLOAT,
                                                  output->precision));
    gcmONERROR(gcSHADER_AddSource(Shader, gcSL_TEMP, newTempIndex, gcSL_SWIZZLE_XXXX, gcSL_FLOAT, output->precision));
    gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader, &constZero, gcSL_FLOAT));

    Shader->lastInstruction = lastInstruction;
    Shader->instrIndex = instrIndex;

OnError:
    return status;
}

static gcSL_SWIZZLE
_SingleEnable2Swizzle(
    IN gctUINT8 Enable
    )
{
    switch (Enable)
    {
    case gcSL_ENABLE_X:
        return gcSL_SWIZZLE_XXXX;
    case gcSL_ENABLE_Y:
        return gcSL_SWIZZLE_YYYY;
    case gcSL_ENABLE_Z:
        return gcSL_SWIZZLE_ZZZZ;
    case gcSL_ENABLE_W:
        return gcSL_SWIZZLE_WWWW;
    default:
        break;
    }

    gcmFATAL("Invalid single enable 0x%x.", Enable);
    return gcSL_SWIZZLE_INVALID;
}

static gceSTATUS
_ConvertRepeatNP2Mode(
    IN OUT gcSHADER             Shader,
    IN gcSL_INSTRUCTION Code,
    IN gctUINT16 TempIndex,
    IN gctINT ChannelIndex,
    IN gcSHADER_PRECISION Precision
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_SWIZZLE sourceSwizzle;
    gcSL_ENABLE targetEnable;
    gctUINT16 tempReg;

    targetEnable = gcSL_ENABLE_X << ChannelIndex;
    sourceSwizzle = _SingleEnable2Swizzle(targetEnable);

    tempReg = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

    /* frac r1.x, tempTexCoord */
    gcSHADER_AddOpcode(Shader, gcSL_FRAC, tempReg,
                        gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, TempIndex,
                        sourceSwizzle, gcSL_FLOAT, Precision);

    /* mov tempTexCoord, r1.x*/
    gcSHADER_AddOpcode(Shader, gcSL_MOV, TempIndex,
                        targetEnable, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg,
                        gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    return status;
}

static gceSTATUS
_ConvertMIRRORNP2Mode(
    IN OUT gcSHADER             Shader,
    IN gcSL_INSTRUCTION Code,
    IN gctUINT16 TempIndex,
    IN gctINT ChannelIndex,
    IN gcSHADER_PRECISION Precision
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_SWIZZLE sourceSwizzle;
    gcSL_ENABLE targetEnable;
    gctUINT16 tempReg1, tempReg2, tempReg3, tempReg4;
    gctFLOAT constZero = 0.0, constPointFive = 0.5, constOne = 1.0, constTwo = -2.0;
    gctUINT label  = gcSHADER_FindNextUsedLabelId(Shader);

    targetEnable = gcSL_ENABLE_X << ChannelIndex;
    sourceSwizzle = _SingleEnable2Swizzle(targetEnable);

    tempReg1 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
    tempReg2 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
    tempReg3 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
    tempReg4 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);

    /* r1.x would hole the result temporarily. */
    /* frc r1.x, tempTexCoord*/
    gcSHADER_AddOpcode(Shader, gcSL_FRAC, tempReg1,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, TempIndex,
                       sourceSwizzle, gcSL_FLOAT, Precision);

    /* floor r2.x, tempTexCoord*/
    gcSHADER_AddOpcode(Shader, gcSL_FLOOR, tempReg2,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, TempIndex,
                       sourceSwizzle, gcSL_FLOAT, Precision);

    /* abs r3.x, r2.x */
    gcSHADER_AddOpcode(Shader, gcSL_ABS, tempReg3,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg2,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    /* mul r1.z, r3.x, 0.5 */
    gcSHADER_AddOpcode(Shader, gcSL_MUL, tempReg1,
                      gcSL_ENABLE_Z, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg3,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constPointFive, gcSL_FLOAT);

    /* floor r2.y, r1.z */
    gcSHADER_AddOpcode(Shader, gcSL_FLOOR, tempReg2,
                      gcSL_ENABLE_Y, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg1,
                       gcSL_SWIZZLE_ZZZZ, gcSL_FLOAT, Precision);

    /* mul r4.x, r2.y, -2.0 */
    gcSHADER_AddOpcode(Shader, gcSL_MUL, tempReg4,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg2,
                       gcSL_SWIZZLE_YYYY, gcSL_FLOAT, Precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constTwo, gcSL_FLOAT);

    /* add r1.z, r4.x, r3.x */
    gcSHADER_AddOpcode(Shader, gcSL_ADD, tempReg1,
                      gcSL_ENABLE_Z, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg4,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg3,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    /* jmp r1.z, 0, 9*/
    gcSHADER_AddOpcodeConditional(Shader, gcSL_JMP, gcSL_EQUAL, label);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg1,
                       gcSL_SWIZZLE_ZZZZ, gcSL_FLOAT, Precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constZero, gcSL_FLOAT);

    /* mov r2.x, r1.x*/
    gcSHADER_AddOpcode(Shader, gcSL_MOV, tempReg2,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg1,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    /* sub r1.x, 1.0, r2.x*/
    gcSHADER_AddOpcode(Shader, gcSL_SUB, tempReg1,
                      gcSL_ENABLE_X, gcSL_FLOAT, Precision);
    gcSHADER_AddSourceConstantFormatted(Shader, &constOne, gcSL_FLOAT);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg2,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    gcSHADER_AddLabel(Shader, label);
    /* mov tempTexCoord, r1.x */
    gcSHADER_AddOpcode(Shader, gcSL_MOV, TempIndex,
                      targetEnable, gcSL_FLOAT, Precision);
    gcSHADER_AddSource(Shader, gcSL_TEMP, tempReg1,
                       gcSL_SWIZZLE_XXXX, gcSL_FLOAT, Precision);

    return status;
}

static gctINT
_ConvertNP2Textue(
    IN OUT gcSHADER             Shader,
    IN gcsPatchNP2Texture *     NP2Texture,
    IN gcSL_INSTRUCTION Code,
    IN gctUINT16 TextureIndex,
    IN gctBOOL CountAddCode
    )
{
    gctINT j;
    gctINT addCountNum = 0;
    gctUINT16 tempTexCoord = 0;
    gctUINT movIndex = 0;

    if (!CountAddCode)
    {
        movIndex = Shader->lastInstruction;
        /* mov tempTexCoord, texCoord*/
        /* tempTexCoord would hold the modified value of texCoord. */
        if (NP2Texture->np2Texture[TextureIndex].texDimension == 2)
        {
            tempTexCoord = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X2);
            gcSHADER_AddOpcode(Shader, gcSL_MOV, tempTexCoord,
                              gcSL_ENABLE_XY, gcSL_FLOAT, gcmSL_SOURCE_GET(Code->source1, Precision));
        }
        else
        {
            gcmASSERT(NP2Texture->np2Texture[TextureIndex].texDimension == 3);
            tempTexCoord = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X3);
            gcSHADER_AddOpcode(Shader, gcSL_MOV, tempTexCoord,
                               gcSL_ENABLE_XYZ, gcSL_FLOAT, gcmSL_SOURCE_GET(Code->source1, Precision));
        }
        gcSHADER_AddSourceIndexedWithPrecision(Shader,
                            gcmSL_SOURCE_GET(Code->source1, Type),
                            Code->source1Index,
                            gcmSL_SOURCE_GET(Code->source1, Swizzle),
                            gcmSL_SOURCE_GET(Code->source1, Indexed),
                            Code->source1Indexed,
                            gcSL_FLOAT,
                            gcmSL_SOURCE_GET(Code->source1, Precision));
    }

    /* fix all channel. */
    for (j = 0; j < NP2Texture->np2Texture[TextureIndex].texDimension; j++)
    {
        switch (NP2Texture->np2Texture[TextureIndex].addressMode[j])
        {
        case NP2_ADDRESS_MODE_REPEAT:
            if (CountAddCode)
            {
                addCountNum += 2;
            }
            else
            {
                _ConvertRepeatNP2Mode(Shader, Code, tempTexCoord, j, gcmSL_SOURCE_GET(Code->source1, Precision));
            }
            break;
        case NP2_ADDRESS_MODE_MIRROR:
            if (CountAddCode)
            {
                addCountNum += 11;
            }
            else
            {
                _ConvertMIRRORNP2Mode(Shader, Code, tempTexCoord, j, gcmSL_SOURCE_GET(Code->source1, Precision));
            }
            break;
        default:
            break;
        }
    }

    /* change the source0 of texld to tempTexCoord */
    if (!CountAddCode)
    {
        gctSOURCE_t source;
        source = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
               | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
               | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT);
        if (NP2Texture->np2Texture[TextureIndex].texDimension == 2)
        {
            source |= gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYYY);
        }
        else
        {
            gcmASSERT(NP2Texture->np2Texture[TextureIndex].texDimension == 3);
            source |= gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZZ);
        }
        Code->source1        = source;
        Code->source1Index   = Shader->code[movIndex].tempIndex;
        Code->source1Indexed = 0;
    }

    return addCountNum;
}

static gctINT
_isSamplerIDNeedNP2(
    IN gctINT SamplerID,
    IN gcsPatchNP2Texture *     NP2Texture
    )
{
    gctINT i;

    for (i = 0; i < NP2Texture->textureCount; i++)
    {
        if (SamplerID == NP2Texture->np2Texture[i].samplerSlot)
        {
            return i;
        }
    }
    return -1;
}

static gceSTATUS
_patchNP2Texture(
    IN OUT gcSHADER             Shader,
    IN gcsPatchNP2Texture *     NP2Texture
    )
{
    gceSTATUS                   status = gcvSTATUS_OK;
    gctINT     i;
    gctUINT lastInst = Shader->lastInstruction;

    for (i = Shader->lastInstruction; i >= 0; i--)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gctINT textureIndex = 0;

        /* Find the texld instruction. */
        if (opcode == gcSL_TEXLD ||
            opcode == gcSL_TEXLDPROJ)
        {
            gctINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gctINT addCodeCount = 0;
            gctUINT insertIndex = i;

            /* Find the texture index. */
            textureIndex = _isSamplerIDNeedNP2(samplerId, NP2Texture);
            if (textureIndex == -1)
                continue;

            if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
                continue;

            /* If there is texlod/texbias/texgrad/texldproj instruction before texld,
            ** we need to modify the insert index.
            */
            if (i)
            {
                opcode = gcmSL_OPCODE_GET(Shader->code[i - 1].opcode, Opcode);
                if (gcSL_isOpcodeTexldModifier(opcode ))
                {
                    insertIndex--;
                }
            }

            /* estimate how many instruction we need to add for this texture. */
            addCodeCount = _ConvertNP2Textue(Shader, NP2Texture, code, (gctUINT16)textureIndex, gcvTRUE);
            if (addCodeCount)
            {
                /* insert nops before texld or texlod instruction. */
                addCodeCount++;
                gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, insertIndex, (gctUINT)addCodeCount));
                Shader->lastInstruction = insertIndex;
                Shader->instrIndex = gcSHADER_OPCODE;
                /* Do NP2 texture patch. */
                _ConvertNP2Textue(Shader, NP2Texture, &Shader->code[i + addCodeCount],
                                            (gctUINT16)textureIndex, gcvFALSE);
            }
            /* update the last instruction. */
            lastInst +=addCodeCount;
            Shader->lastInstruction = lastInst;
        }
    }

OnError:
    return status;
}

static gceSTATUS
_patchTexldFormatConversion(
    IN OUT gcSHADER         Shader,
    IN gcsInputConversion * FormatConversion
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             origCodeCount = Shader->codeCount;
    gcFUNCTION          stubFunction = gcvNULL;
    gctUINT             i;
    gcSL_INSTRUCTION    tempCode = gcvNULL, tempCode1 = gcvNULL;
    gctPOINTER          pointer = gcvNULL;

    if (gcTexFormatConvertLibrary == gcvNULL)
    {
        gcmONERROR(gcLoadTexFormatConvertLibrary());
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    for (i = 0; i < origCodeCount; i++)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gcSL_INSTRUCTION prevCode   = gcvNULL;
        gcSL_OPCODE      prevOpcode = gcSL_NOP;
        gcFUNCTION       convertFunction = gcvNULL;
        gcFUNCTION       swizzleConvertFunction = gcvNULL;
        gcFUNCTION       retFunction = gcvNULL;
        gctUINT          retArgNo = (gctUINT)-1;

        /* Find the instruction which need to do format conversion */
        if (gcSL_isOpcodeTexld(opcode))
        {
            gctINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gctINT offset = gcmSL_INDEX_GET(code->source0Index, ConstValue);
            gctINT offset2 = code->source0Indexed;
            gctUINT lastInstruction;
            gcSHADER_INSTRUCTION_INDEX instrIndex;
            gceSURF_FORMAT format;
            gcSL_INDEXED indexed = gcmSL_SOURCE_GET(code->source0, Indexed);
            gctCHAR funcName[32];
            gctUINT funcNameOffset = 0;

            /* TODO - Need to add support to handle temp. */
            if (indexed != gcSL_NOT_INDEXED)
                continue;

            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_SAMPLER)
            {
                gcUNIFORM uniform;
                uniform = gcSHADER_GetUniformBySamplerIndex(Shader, samplerId, &offset);
                samplerId = uniform->index;
            }
            else
            {
                offset = offset + offset2;
            }

            if (FormatConversion->samplers[0]->index != samplerId ||
                FormatConversion->arrayIndex != offset)
            {
                continue;
            }

            /*
            ** Update the sampler on gcsInputConversion, make sure that
            ** we save the pointer from the recompiler shader.
            */
            gcmONERROR(gcSHADER_GetUniform(Shader,
                                           FormatConversion->samplers[0]->index,
                                           &FormatConversion->samplers[0]));

            /* Check if there is any texld modifier. */
            if (i > 0)
            {
                gcoOS_MemCopy(tempCode, &Shader->code[i - 1], sizeof(struct _gcSL_INSTRUCTION));
                prevCode   = tempCode;
                prevOpcode = gcmSL_OPCODE_GET(prevCode->opcode, Opcode);
                if (!gcSL_isOpcodeTexldModifier(prevOpcode))
                {
                    prevCode = gcvNULL;
                    prevOpcode = gcSL_NOP;
                }
            }
            /* check if it special 1 layer texgather recompilation */
            format = FormatConversion->samplerInfo.format;
            if (prevOpcode != gcSL_TEXGATHER &&
                (format == gcvSURF_G32R32F ||
                 format == gcvSURF_G32R32I ||
                 format == gcvSURF_G32R32UI ||
                 format == gcvSURF_R32F))
            {
                continue;
            }

            gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));
            tempCode1 = (gcSL_INSTRUCTION) pointer;
            gcoOS_MemCopy(tempCode1, &Shader->code[i], sizeof(struct _gcSL_INSTRUCTION));

            /* Construct format convert function. */
            if (FormatConversion->needFormatConvert)
            {
                gcmONERROR(_createInputConvertFunction(Shader,
                                                       gcTexFormatConvertLibrary,
                                                       FormatConversion,
                                                       prevOpcode,
                                                       &convertFunction));
            }

            /* Construct swizzle convert function. */
            if (FormatConversion->needSwizzle)
            {
                gcmONERROR(_createSwizzleConvertFunction(Shader,
                                                         gcTexFormatConvertLibrary,
                                                         FormatConversion,
                                                         &swizzleConvertFunction));

            }

            /* Construct stub function first. */
            gcmVERIFY_OK(gcoOS_PrintStrSafe(funcName, sizeof(funcName), &funcNameOffset, "#inputConvert%d", i));

            /* Add stubFunction to Shader. */
            gcSHADER_AddFunction(Shader, funcName, &stubFunction);
            SetFunctionRecompilerStub(stubFunction);
            gcSHADER_BeginFunction(Shader, stubFunction);

            /* Call format convert function. */
            if (convertFunction)
            {
                /* Construct call stub function. */
                gcmONERROR(_createFormatConvertStubFunction(Shader,
                                                            FormatConversion,
                                                            convertFunction,
                                                            stubFunction,
                                                            tempCode1,
                                                            prevCode,
                                                            &retArgNo));
                retFunction = convertFunction;
            }

            /* Call swizzle convert function. */
            if (swizzleConvertFunction)
            {
                /* Construct call stub function. */
                gcmONERROR(_createSwizzleConvertStubFunction(Shader,
                                                             FormatConversion,
                                                             swizzleConvertFunction,
                                                             retFunction,
                                                             tempCode1,
                                                             prevCode,
                                                             &retArgNo));
                retFunction = swizzleConvertFunction;
            }

            /* save the new value. */
            _addRetValueInst(Shader,
                             retFunction,
                             tempCode1,
                             retArgNo,
                             gcvDest,
                             gcvNULL);

            /* ret stub Function */
            _addRetInst(Shader);
            gcSHADER_EndFunction(Shader, stubFunction);

            /* Change the texld instruciton to call stub */
            code = &Shader->code[i];
            gcSL_SetInst2NOP(code);
            /* Change the texld modifier instruciton to call stub */
            if (prevCode)
            {
                code = &Shader->code[i - 1];
                gcSL_SetInst2NOP(code);
            }
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = i;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction);
            Shader->lastInstruction = lastInstruction;
            Shader->instrIndex = instrIndex;

            if (tempCode1)
            {
                /* Free the current code buffer. */
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode1));
            }
        }
    }

OnError:
    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    if (tempCode1)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode1));
    }

    /* Return the status. */
    return status;
}

/************************************************************************
**
** patch the unsupported output format to a format supported by the
** the hardware the shader is compiling for. The FormatConversion
** directive specifies which output location should be patched, and
** what supported format it should use.
**
** Algorithm:
**
**    o find the output to be patched by the location
**    o construct convert function
**    o insert a stub call to the convert function at the end
**      of main function
**    o patch the code after the main function
**/
static gceSTATUS
_patchOutputFormatConversion(
    IN OUT gcSHADER          Shader,
    IN gcsOutputConversion * OutputConversion
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcFUNCTION  convertFunction = gcvNULL;

    gcFUNCTION  stubFunction = gcvNULL;
    gcSL_INSTRUCTION  tempCode;
    gctINT      tempCodeIndex;
    gctINT      outputTempIndex = -1;
    gctUINT     i;
    gctUINT     lastInstruction;
    gcSHADER_INSTRUCTION_INDEX instrIndex;

    /* only fragment shader needs to patch output */
    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
        return gcvSTATUS_OK;

    /* load format conversion library if is not loaded yet */
    if (gcTexFormatConvertLibrary == gcvNULL)
    {
        gcmONERROR(gcLoadTexFormatConvertLibrary());
    }


    /* find the output variable's temp register index */
    for (i=0; i < Shader->outputCount; i++)
    {
        gcOUTPUT output = Shader->outputs[i];

        if (output == gcvNULL) continue;
        if (output->location == OutputConversion->outputLocation)
        {
            outputTempIndex = output->tempIndex;
            OutputConversion->outputs[0] = output;
            break;
        }
    }

    if (outputTempIndex == -1)
    {
        status = gcvSTATUS_NOT_FOUND;
        goto OnError;
    }

    /* insert a NOP to the end of main() */
    tempCodeIndex = _insertNOP2Main(Shader, 1);


    /* Construct convert function. */
    gcmONERROR(_createOutputConvertFunction(
                            Shader,
                            gcTexFormatConvertLibrary,
                            OutputConversion,
                            gcSL_ENABLE_XYZW, /* TODO: set enable */
                            &convertFunction));

    if (convertFunction == gcvNULL)
    {
        return gcvSTATUS_RECOMPILER_CONVERT_UNIMPLEMENTED;
    }
    /* Construct call stub function. */
    stubFunction = _createOutputConvertStubFunction(
                        Shader,
                        OutputConversion,
                        convertFunction,
                        tempCodeIndex);

    /* Change the texld instruciton to call stub */
    tempCode = &Shader->code[tempCodeIndex];
    if (gcmSL_OPCODE_GET(tempCode->opcode, Opcode) != gcSL_NOP)
    {
        gcmASSERT(gcvFALSE);
    }

    lastInstruction = Shader->lastInstruction;
    instrIndex = Shader->instrIndex;
    Shader->lastInstruction = tempCodeIndex;
    Shader->instrIndex = gcSHADER_OPCODE;
    _addCallInst(Shader, stubFunction);
    Shader->lastInstruction = lastInstruction;
    Shader->instrIndex = instrIndex;

    /*Shader->codeCount = Shader->lastInstruction;*/

OnError:


    /* Return the status. */
    return status;
}

static gceSTATUS
_patchDepthComparison(
    IN OUT gcSHADER         Shader,
    IN gcsDepthComparison * DepthComparison
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     origCodeCount = Shader->codeCount;
    gcFUNCTION  convertFunction = gcvNULL;
    gcFUNCTION  stubFunction = gcvNULL;
    gctUINT     i;
    gcSL_INSTRUCTION  tempCode = gcvNULL;
    gctPOINTER pointer = gcvNULL;

    if (gcTexFormatConvertLibrary == gcvNULL)
    {
        gcmONERROR(gcLoadTexFormatConvertLibrary());
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    tempCode = (gcSL_INSTRUCTION) pointer;

    for (i = 0; i < origCodeCount; i++)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gcSL_INSTRUCTION prevCode   = gcvNULL;
        gcSL_OPCODE      prevOpcode = gcSL_NOP;

        /* Find the instruction which need to do format conversion */
        if (opcode == gcSL_TEXLDPCF ||
            opcode == gcSL_TEXLDPCFPROJ)
        {
            gcUNIFORM uniform = gcvNULL;
            gctINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gctINT offset = gcmSL_INDEX_GET(code->source0Index, ConstValue);
            gctINT offset2 = code->source0Indexed;
            gcSL_TYPE type = gcmSL_SOURCE_GET(code->source0, Type);
            gctUINT lastInstruction;
            gcSHADER_INSTRUCTION_INDEX instrIndex;
            gctBOOL isSamplerMatch = gcvFALSE;
            gctINT minSamplerIndex, maxSamplerIndex;

            gcmASSERT(type == gcSL_SAMPLER || type == gcSL_UNIFORM);

            /* TODO: need to handle indexed sampler. */
            if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
                continue;

            if (type == gcSL_SAMPLER)
            {
                if (DepthComparison->sampler->arraySize > 1)
                {
                    minSamplerIndex = DepthComparison->sampler->physical;
                    maxSamplerIndex = DepthComparison->sampler->physical + DepthComparison->sampler->arraySize - 1;
                }
                else
                {
                    minSamplerIndex = maxSamplerIndex = DepthComparison->sampler->physical;
                }
                uniform = gcSHADER_GetUniformBySamplerIndex(Shader, samplerId, gcvNULL);
            }
            else
            {
                if (DepthComparison->sampler->arraySize > 1)
                {
                    minSamplerIndex = DepthComparison->sampler->index;
                    maxSamplerIndex = DepthComparison->sampler->index + DepthComparison->sampler->index - 1;
                }
                else
                {
                    minSamplerIndex = maxSamplerIndex = DepthComparison->sampler->index;
                }
                gcSHADER_GetUniform(Shader, samplerId, &uniform);
                offset = offset + offset2;
            }

            if (samplerId >= minSamplerIndex && samplerId <= maxSamplerIndex &&
                DepthComparison->arrayIndex == offset)
            {
                isSamplerMatch = gcvTRUE;
            }

            /* TODO - Need to add support to handle temp. */
            if (!isSamplerMatch)
            {
                continue;
            }

            SetUniformFlags(uniform, gcvUNIFORM_FLAG_IS_DEPTH_COMPARISON);

            /* Check if there is any texld modifier. */
            if (i > 0)
            {
                gcoOS_MemCopy(tempCode, &Shader->code[i - 1], sizeof(struct _gcSL_INSTRUCTION));
                prevCode   = tempCode;
                prevOpcode = gcmSL_OPCODE_GET(prevCode->opcode, Opcode);
                if (!gcSL_isOpcodeTexldModifier(prevOpcode))
                {
                    prevCode = gcvNULL;
                    prevOpcode = gcSL_NOP;
                }
            }

            /* Construct convert function. */
            gcmONERROR(_createDepthComparisonFunction(
                                    Shader,
                                    gcTexFormatConvertLibrary,
                                    DepthComparison,
                                    prevOpcode,
                                    &convertFunction));

            /* Construct call stub function. */
            stubFunction = _createDepthComparisonStubFunction(
                                Shader,
                                DepthComparison,
                                uniform,
                                convertFunction,
                                i,
                                prevCode);

            /* Change the texld instruciton to call stub */
            /* Shader->code may be resized to a new array, so need to use index. */
            code = &Shader->code[i];
            gcSL_SetInst2NOP(code);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = i;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction);
            Shader->lastInstruction = lastInstruction;
            Shader->instrIndex = instrIndex;
        }
    }

OnError:
    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

    /* Return the status. */
    return status;
}

static gceSTATUS
_patchGlobalWorkSizeCode(
    IN  gcSHADER           Shader,
    IN  gceBuiltinNameKind BuiltinName,
    IN  gcUNIFORM          UniormWidth
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T attribCount;
    gctINT mainStart, mainEnd;
    gctUINT lastInstruction;

    if (UniormWidth == gcvNULL)
    {
        return status;
    }

    /* Find the attribute that need to convert. */
    for (attribCount = 0; attribCount < Shader->attributeCount; attribCount++)
    {
        if (GetATTRNameLength(GetShaderAttribute(Shader, attribCount)) == BuiltinName)
        {
            break;
        }
    }

    gcmASSERT(attribCount < Shader->attributeCount);

    /* find the beginning of main function. */
    gcmONERROR(gcSHADER_FindMainFunction(Shader, &mainStart, &mainEnd));

    /* Insert nops at the beginning of main function. */
    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, mainStart, 3));
    lastInstruction = Shader->lastInstruction;
    Shader->lastInstruction = mainStart;
    Shader->instrIndex = gcSHADER_OPCODE;
#if _SUPPORT_LONG_ULONG_DATA_TYPE
    Patched1Dto2D = gcvTRUE;
#endif

    /* Insert instructions */
    {
        gctUINT16 newTemp = (gctUINT16)gcSHADER_NewTempRegs(
                                    Shader, 1,
                                    Shader->attributes[attribCount]->type);
        gctUINT32 zeroValue = 0;

        /* Replace the same attribute */
        gcmONERROR(_ChangeAttribToTempForAllCodes(Shader,
                            Shader->attributes[attribCount]->index,
                            newTemp));

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_MOV,
                            newTemp,
                            gcSL_ENABLE_XYZW,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                            &zeroValue,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_MUL,
                            newTemp,
                            gcSL_ENABLE_X,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceAttributeFormatted(Shader,
                            Shader->attributes[attribCount],
                            gcSL_SWIZZLE_YYYY,
                            0,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddSourceUniformFormatted(Shader,
                            UniormWidth,
                            gcSL_SWIZZLE_XXXX,
                            0,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_ADD,
                            newTemp,
                            gcSL_ENABLE_X,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSource(Shader,
                            gcSL_TEMP,
                            newTemp,
                            gcSL_SWIZZLE_XXXX,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceAttributeFormatted(Shader,
                            Shader->attributes[attribCount],
                            gcSL_SWIZZLE_XXXX,
                            0,
                            gcSL_UINT32));

        Shader->lastInstruction = lastInstruction + 3;
    }

OnError:
    return status;
}

static gceSTATUS
_patchRealGlobalWorkSizeCode(
    IN  gcSHADER           Shader,
    IN  gcUNIFORM          UniormWidth
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T attribCount;
    gctINT mainStart, mainEnd;
    gctUINT lastInstruction;
    gcATTRIBUTE attrGlobalWorkID = gcvNULL;

    if (UniormWidth == gcvNULL)
    {
        return status;
    }

    /* Find the attribute that need to convert. */
    for (attribCount = 0; attribCount < Shader->attributeCount; attribCount++)
    {
        if (GetATTRNameLength(GetShaderAttribute(Shader, attribCount)) == gcSL_GLOBAL_INVOCATION_ID)
        {
            attrGlobalWorkID = Shader->attributes[attribCount];
            break;
        }
    }

    if(!attrGlobalWorkID)
    {
        gcmONERROR(gcSHADER_AddAttribute(Shader,
                        "#global_id", gcSHADER_INTEGER_X4, 1,
                        gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH,
                        &attrGlobalWorkID));
    }

    /* find the beginning of main function. */
    gcmONERROR(gcSHADER_FindMainFunction(Shader, &mainStart, &mainEnd));

    /* Insert nops at the beginning of main function. */
    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, mainStart, 3));
    lastInstruction = Shader->lastInstruction;
    Shader->lastInstruction = mainStart;
    Shader->instrIndex = gcSHADER_OPCODE;

    /* Insert instructions */
    {
        gctUINT16 newTemp = (gctUINT16)gcSHADER_NewTempRegs(
                                    Shader, 1,
                                    attrGlobalWorkID->type);
        gctUINT32 zeroValue = 0;

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_MOV,
                            newTemp,
                            gcSL_ENABLE_XYZW,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                            &zeroValue,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_MUL,
                            newTemp,
                            gcSL_ENABLE_X,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceAttributeFormatted(Shader,
                            attrGlobalWorkID,
                            gcSL_SWIZZLE_YYYY,
                            0,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddSourceUniformFormatted(Shader,
                            UniormWidth,
                            gcSL_SWIZZLE_XXXX,
                            0,
                            gcSL_UINT32));

        gcmONERROR(gcSHADER_AddOpcode(Shader,
                            gcSL_ADD,
                            newTemp,
                            gcSL_ENABLE_X,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSource(Shader,
                            gcSL_TEMP,
                            newTemp,
                            gcSL_SWIZZLE_XXXX,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

        gcmONERROR(gcSHADER_AddSourceAttributeFormatted(Shader,
                            attrGlobalWorkID,
                            gcSL_SWIZZLE_XXXX,
                            0,
                            gcSL_UINT32));


        {
            gctUINT label  = gcSHADER_FindNextUsedLabelId(Shader);

            gcmONERROR(gcSHADER_AddOpcodeConditional(Shader,
                            gcSL_JMP,
                            gcSL_GREATER_OR_EQUAL,
                            label));

            gcmONERROR(gcSHADER_AddSource(Shader,
                            gcSL_TEMP,
                            newTemp,
                            gcSL_SWIZZLE_XXXX,
                            gcSL_UINT32,
                            gcSHADER_PRECISION_HIGH));

            /*UniformWidth.y is realGlobalSize*/
            gcmONERROR(gcSHADER_AddSourceUniformFormatted(Shader,
                            UniormWidth,
                            gcSL_SWIZZLE_YYYY,
                            0,
                            gcSL_UINT32));

            Shader->lastInstruction = lastInstruction + 4;
            gcmONERROR(gcSHADER_AddLabel(Shader, label));
        }
    }

OnError:
    return status;
}

static gceSTATUS
_patchGlobalWorkSize(
    IN OUT gcSHADER             Shader,
    IN gcsPatchGlobalWorkSize * globalWorkSize
    )
{
    gceSTATUS status;

    gcmONERROR(_patchGlobalWorkSizeCode(Shader,
                            gcSL_GLOBAL_INVOCATION_ID,
                            globalWorkSize->globalWidth));

    gcmONERROR(_patchGlobalWorkSizeCode(Shader,
                            gcSL_WORK_GROUP_ID,
                            globalWorkSize->groupWidth));

    if (globalWorkSize->patchRealGlobalWorkSize)
    {
        gcmONERROR(_patchRealGlobalWorkSizeCode(Shader,
                                globalWorkSize->globalWidth));
    }

OnError:
    return status;
}

#if _SUPPORT_LONG_ULONG_DATA_TYPE
static gctUINT tmpLabel1=0, tmpLabel2=0;
static gceSTATUS
_patchLongULong(
    IN OUT gcSHADER         Shader,
    IN OUT gcPatchDirective_PTR CurDir,
    IN gcsPatchLongULong *  Patch
    )
{
    /* How to do patch.
       1. Ensure the precompiled patch lib for long/ulong is ready;
       2. Create long/ulong handling function (find it in the lib);
       3. Create the calling function to the long/ulong function.
       See also: _patchReadImage().
    */

    gceSTATUS           status = gcvSTATUS_OK;

    /*  gcCLPatchLibrary[2] is used as long/ulong patch lib. */
    if (gcCLPatchLibrary[2] == gcvNULL)
    {
        gcmASSERT((gcCLPatchLibrary[1] == gcvNULL) &&
                  (gcCLPatchLibrary[0] == gcvNULL));
        gcmONERROR(gcLoadCLPatchLibrary());
    }

    gcmASSERT(Patch);
    {
        gctUINT lastInstruction;
        gcSHADER_INSTRUCTION_INDEX instrIndex;
        gcSL_CONDITION condition = gcmSL_TARGET_GET(Shader->code[Patch->instructionIndex].temp, Condition);

        /* Construct longULong processing function. */
        if(gcmSL_OPCODE_GET(Shader->code[Patch->instructionIndex + Shader->InsertCount].opcode, Opcode) == gcSL_JMP &&
           ((gcmSL_TARGET_GET(Shader->code[Patch->instructionIndex+Shader->InsertCount].temp, Condition) == gcSL_NOT_EQUAL) ||
            (gcmSL_TARGET_GET(Shader->code[Patch->instructionIndex+Shader->InsertCount].temp, Condition) == gcSL_EQUAL)) &&
           (gcmSL_SOURCE_GET(Shader->code[Patch->instructionIndex + Shader->InsertCount].source1, Type) == gcSL_CONSTANT) )
        {
            gctPOINTER pointer = gcvNULL;
            gcSL_INSTRUCTION tempCode = gcvNULL, curCode = gcvNULL;
            gctUINT i = Patch->instructionIndex + Shader->InsertCount;
            gcFUNCTION          convertFunction_jmp = gcvNULL;
            gcFUNCTION          stubFunction_jmp = gcvNULL;
            gctUINT16 jmpIndex = 0;

#if gcdDEBUG
            gcSL_INSTRUCTION code     = &Shader->code[Patch->instructionIndex+Shader->InsertCount];
            gcSL_FORMAT src0Format  = gcmSL_SOURCE_GET(GetInstSource0(code), Format);
            gcSL_FORMAT src1Format  = gcmSL_SOURCE_GET(GetInstSource1(code), Format);
            gcSL_FORMAT dstFormat   = gcmSL_TARGET_GET(GetInstTemp(code), Format);
#endif
            gcmASSERT((Patch->instructionIndex + Shader->InsertCount < Shader->codeCount) &&
                    (src0Format == gcSL_UINT64 ||
                     src0Format == gcSL_INT64  ||
                     src1Format == gcSL_UINT64 ||
                     src1Format == gcSL_INT64  ||
                     dstFormat  == gcSL_UINT64 ||
                     dstFormat  == gcSL_INT64));

            {
                /* Construct longULong processing function. */
                status = _createLongULongFunction_jmp(Shader,
                                                  gcCLPatchLibrary[2],
                                                  Patch,
                                                  &convertFunction_jmp);
                gcmONERROR(status);

                /* Construct call stub function. */
                stubFunction_jmp = _createLongULongStubFunction_jmp(Shader,
                                                            Patch,
                                                            convertFunction_jmp,
                                                            &jmpIndex);
            }

            /* Change the texld instruciton to call stub */
            gcmASSERT (gcmSL_OPCODE_GET(Shader->code[Patch->instructionIndex+Shader->InsertCount].opcode, Opcode) != gcSL_CALL);
            gcSL_SetInst2NOP(&Shader->code[Patch->instructionIndex+Shader->InsertCount]);
            Shader->code[Patch->instructionIndex+Shader->InsertCount].temp = gcmSL_TARGET_SET(Shader->code[Patch->instructionIndex+Shader->InsertCount].temp, Enable, gcSL_ENABLE_X);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = i;
            Shader->instrIndex = gcSHADER_OPCODE;
            if((Shader->InstNum == 1) && (isLogicOR == gcvTRUE))
            {
                gcSHADER_AddLabel(Shader, tmpLabel1);
                isLogicOR = gcvFALSE;
            }

            _addCallInst(Shader, stubFunction_jmp);
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                gcmSIZEOF(struct _gcSL_INSTRUCTION) * lastInstruction,
                                &pointer));
            tempCode = (gcSL_INSTRUCTION)pointer;

            /* Copy existing code. */
            gcoOS_MemCopy(tempCode,
                            Shader->code,
                            gcmSIZEOF(struct _gcSL_INSTRUCTION) * lastInstruction);

            if(Shader->InstNum == 0)
            {
                tmpLabel1  = gcSHADER_FindNextUsedLabelId(Shader);
                gcSHADER_AddOpcodeConditional(Shader,
                                              gcSL_JMP,
                                              (condition == gcSL_NOT_EQUAL ? gcSL_NOT_ZERO : gcSL_ZERO),
                                              tmpLabel1);

                gcSHADER_AddSource(Shader, gcSL_TEMP, jmpIndex,
                                   gcSL_SWIZZLE_XXXX, gcSL_INT32, convertFunction_jmp->arguments[1].precision);

                 if(condition == gcSL_NOT_EQUAL)
                 {
                     gcoOS_MemCopy(Shader->code+i+2, tempCode+i+1,
                            gcmSIZEOF(struct _gcSL_INSTRUCTION) * 2);
                    Shader->lastInstruction = i+3;
                    Shader->instrIndex = instrIndex;
                    gcSHADER_AddLabel(Shader, tmpLabel1);
                    Shader->lastInstruction = lastInstruction + 1;
                    gcoOS_MemCopy(Shader->code+i+4, tempCode+i+3, gcmSIZEOF(struct _gcSL_INSTRUCTION) * (lastInstruction - i - 2));
                    Shader->InsertCount++;
                }
                else if(condition == gcSL_EQUAL)
                {
                    Shader->lastInstruction = lastInstruction + 1;
                    Shader->instrIndex = instrIndex;

                     gcoOS_MemCopy(Shader->code+i+2, tempCode+i+1,
                                     gcmSIZEOF(struct _gcSL_INSTRUCTION));

                     gcoOS_MemCopy(Shader->code+i+3, tempCode+i+3,
                                     gcmSIZEOF(struct _gcSL_INSTRUCTION) * (lastInstruction - i - 1));

                     Shader->lastInstruction -= 1;
                    isLogicOR = gcvTRUE;
                 }
                Shader->InstNum++;
            }
            else if(Shader->InstNum == 1)
            {
                tmpLabel2  = gcSHADER_FindNextUsedLabelId(Shader);
                gcSHADER_AddOpcodeConditional(Shader,
                                            gcSL_JMP,
                                            gcSL_ZERO,
                                            tmpLabel2);
                gcSHADER_AddSource(Shader, gcSL_TEMP, jmpIndex,
                                gcSL_SWIZZLE_XXXX, gcSL_INT32, convertFunction_jmp->arguments[1].precision);
                gcoOS_MemCopy(Shader->code+i+2, tempCode+i+1,
                                gcmSIZEOF(struct _gcSL_INSTRUCTION) * 2);
                Shader->lastInstruction = i+3;
                Shader->instrIndex = instrIndex;
                gcSHADER_AddLabel(Shader, tmpLabel2);
                Shader->lastInstruction = lastInstruction + 1;
                gcoOS_MemCopy(Shader->code+i+4, tempCode+i+3,
                    gcmSIZEOF(struct _gcSL_INSTRUCTION) * (lastInstruction - i - 2));

                Shader->InsertCount++;
                Shader->InstNum++;
            }

            gcmOS_SAFE_FREE(gcvNULL, tempCode);
            Shader->codeCount = Shader->lastInstruction;

            if(Shader->InsertCount != 0)
            {
                gcSL_OPCODE  opcode;
                gcSHADER_LABEL label;
                gctINT j;

                /* 3. update the call and jump index. */
                for (j = 0; j < (gctINT)Shader->codeCount; j++)
                {
                    curCode = &Shader->code[j];
                    opcode = gcmSL_OPCODE_GET(curCode->opcode, Opcode);

                    if ((opcode == gcSL_JMP || opcode == gcSL_CALL) &&
                        (curCode->tempIndex >= i+1 ))
                    {
                        curCode->tempIndex += 1;
                    }
                }

                /* TODO: modify jump/call target index on function _insertNOP2BeforeCode. */
                for (j = 0; j < (gctINT)Shader->functionCount; ++j)
                {
                    /* If the code is inside the function, update the code count. */
                    if (Shader->functions[j]->codeStart <= i+1 &&
                        Shader->functions[j]->codeCount > 0 &&
                        Shader->functions[j]->codeStart + Shader->functions[j]->codeCount - 1 >= i+1)
                    {
                        Shader->functions[j]->codeCount += 1;
                    }

                    /* If the code is before the function, update the code start index. */
                    if (Shader->functions[j]->codeStart > i+1)
                        Shader->functions[j]->codeStart += 1;
                }

                for (j = 0; j < (gctINT)Shader->kernelFunctionCount; ++j)
                {
                    if (Shader->kernelFunctions[j]->codeStart <= i+1 &&
                        Shader->kernelFunctions[j]->codeEnd >= i+1 &&
                        Shader->kernelFunctions[j]->codeCount > 0)
                        Shader->kernelFunctions[j]->codeCount += 1;

                    if (Shader->kernelFunctions[j]->codeStart > i+1 &&
                        Shader->kernelFunctions[j]->codeCount > 0)
                    {
                        Shader->kernelFunctions[j]->codeStart += 1;
                        Shader->kernelFunctions[j]->codeEnd += 1;
                    }
                }

                /* 5. adjust shader label's defined */
                for (label = Shader->labels; label != gcvNULL; label = label->next)
                {
                    gcSHADER_LINK link;

                    if (label->defined > i+1)
                        label->defined += 1;
                    link = label->referenced;
                    while (link)
                    {
                        if (link->referenced > i+1)
                            link->referenced += 1;
                        link = link->next;
                    }
                }
            }
        }
        else if(Shader->code[Patch->instructionIndex].opcode == gcSL_JMP &&
            ((condition == gcSL_LESS_OR_EQUAL) ||
             (condition == gcSL_GREATER_OR_EQUAL) ||
             (condition == gcSL_LESS) ||
             (condition == gcSL_GREATER) ||
             (condition == gcSL_EQUAL) ||
             (condition == gcSL_NOT_EQUAL)) )
        {
            gctPOINTER pointer = gcvNULL;
            gcSL_INSTRUCTION tempCode = gcvNULL, curCode = gcvNULL;
            gctUINT patchInstrIndex = Patch->instructionIndex;
            gcFUNCTION          convertFunction_jmp = gcvNULL;
            gcFUNCTION          stubFunction_jmp = gcvNULL;
            gctUINT16 jmpIndex = 0;
            gctUINT   jmpLabel = 0;
            gcSL_INSTRUCTION code;
            gctUINT16   origJmpLabel = 0;
            /*1D to 2D Patch will add 3 instruction in the top of binary, we need add 3 offset in instructionIndex if Patch is set. */
            Patch->instructionIndex += (Patched1Dto2D?3:0);

            /* Construct longULong processing function. */
            status = _createLongULongFunction_jmp(Shader,
                                                gcCLPatchLibrary[2],
                                                Patch,
                                                &convertFunction_jmp);
            gcmONERROR(status);

            /* Construct call stub function. */
            stubFunction_jmp = _createLongULongStubFunction_jmp_src2(Shader,
                                                                        Patch,
                                                                        convertFunction_jmp,
                                                                        &jmpIndex);
            /* Change the instruciton to call stub */
            code = &Shader->code[patchInstrIndex];
            origJmpLabel = gcmSL_JMP_TARGET(code);
            gcmASSERT (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL);
            gcSL_SetInst2NOP(code);
            code->temp = gcmSL_TARGET_SET(code->temp, Enable, gcSL_ENABLE_X);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = patchInstrIndex;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction_jmp);
            /* add call stub done */


            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                gcmSIZEOF(struct _gcSL_INSTRUCTION) * lastInstruction,
                                &pointer));
            tempCode = (gcSL_INSTRUCTION)pointer;

            /* Copy existing code. */
            gcoOS_MemCopy(tempCode,
                            Shader->code,
                            gcmSIZEOF(struct _gcSL_INSTRUCTION) * lastInstruction);

            /* add JMP instruction label */
            Shader->lastInstruction = origJmpLabel;
            Shader->instrIndex = gcSHADER_OPCODE;
            jmpLabel  =  gcSHADER_FindNextUsedLabelId(Shader);
            gcSHADER_AddLabel(Shader, jmpLabel);
            /* add JMP instruction to jump same target like before */
            Shader->lastInstruction = patchInstrIndex + 1;
            code = &Shader->code[Shader->lastInstruction];
            gcSL_SetInst2NOP(code);
            gcSHADER_AddOpcodeConditional(Shader,
                                        gcSL_JMP,
                                        gcSL_NOT_ZERO,
                                        jmpLabel);

            gcSHADER_AddSource(Shader, gcSL_TEMP, jmpIndex,
                                gcSL_SWIZZLE_XXXX, gcSL_INT32, convertFunction_jmp->arguments[1].precision);


            Shader->lastInstruction = lastInstruction + 1;
            Shader->instrIndex = instrIndex;
            gcoOS_MemCopy(Shader->code+patchInstrIndex+2, tempCode+patchInstrIndex+1,
                        gcmSIZEOF(struct _gcSL_INSTRUCTION) * (lastInstruction - patchInstrIndex));

            gcmOS_SAFE_FREE(gcvNULL, tempCode);
            Shader->codeCount = Shader->lastInstruction;

            /* update jump index */
            {
                gcSL_OPCODE  opcode;
                gcSHADER_LABEL label;
                gctINT j;

                /* 3. update the call and jump index. */
                for (j = 0; j < (gctINT)Shader->codeCount; j++)
                {
                    curCode = &Shader->code[j];
                    opcode = gcmSL_OPCODE_GET(curCode->opcode, Opcode);

                    if ((opcode == gcSL_JMP || opcode == gcSL_CALL) &&
                        (curCode->tempIndex >= patchInstrIndex+1 ))
                    {
                        curCode->tempIndex += 1;
                    }
                }

                /* TODO: modify jump/call target index on function _insertNOP2BeforeCode. */
                for (j = 0; j < (gctINT)Shader->functionCount; ++j)
                {
                    /* If the code is inside the function, update the code count. */
                    if (Shader->functions[j]->codeStart <= patchInstrIndex+1 &&
                        Shader->functions[j]->codeCount > 0 &&
                        Shader->functions[j]->codeStart + Shader->functions[j]->codeCount - 1 >= patchInstrIndex+1)
                    {
                        Shader->functions[j]->codeCount += 1;
                    }

                    /* If the code is before the function, update the code start index. */
                    if (Shader->functions[j]->codeStart > patchInstrIndex+1)
                        Shader->functions[j]->codeStart += 1;
                }

                for (j = 0; j < (gctINT)Shader->kernelFunctionCount; ++j)
                {
                    if (Shader->kernelFunctions[j]->codeStart <= patchInstrIndex+1 &&
                        Shader->kernelFunctions[j]->codeEnd >= patchInstrIndex+1 &&
                        Shader->kernelFunctions[j]->codeCount > 0)
                        Shader->kernelFunctions[j]->codeCount += 1;

                    if (Shader->kernelFunctions[j]->codeStart > patchInstrIndex+1 &&
                        Shader->kernelFunctions[j]->codeCount > 0)
                    {
                        Shader->kernelFunctions[j]->codeStart += 1;
                        Shader->kernelFunctions[j]->codeEnd += 1;
                    }
                }

                /* 5. adjust shader label's defined */
                for (label = Shader->labels; label != gcvNULL; label = label->next)
                {
                    gcSHADER_LINK link;

                    if (label->defined > patchInstrIndex+1)
                        label->defined += 1;
                    link = label->referenced;
                    while (link)
                    {
                        if (link->referenced > patchInstrIndex+1)
                            link->referenced += 1;
                        link = link->next;
                    }
                }
            }

            /* a new inst is inserted, should update all the succssive references. */
            {
                gcPatchDirective_PTR pd = CurDir;
                while (pd != gcvNULL)
                {
                    if (pd->kind == gceRK_PATCH_CL_LONGULONG)
                    {
                        pd->patchValue.longULong->instructionIndex++;
                    }
                    pd = pd->next;
                }
            }
        }
        else
        {
            gcFUNCTION          convertFunction = gcvNULL;
            gcFUNCTION          stubFunction = gcvNULL;
            gcSL_INSTRUCTION code     = &Shader->code[Patch->instructionIndex];

#if gcdDEBUG
            gcSL_FORMAT src0Format  = gcmSL_SOURCE_GET(GetInstSource0(code), Format);
            gcSL_FORMAT src1Format  = gcmSL_SOURCE_GET(GetInstSource1(code), Format);
            gcSL_FORMAT dstFormat   = gcmSL_TARGET_GET(GetInstTemp(code), Format);
#endif
            gcmASSERT(code &&
                    (src0Format == gcSL_UINT64 ||
                     src0Format == gcSL_INT64  ||
                     src1Format == gcSL_UINT64 ||
                     src1Format == gcSL_INT64  ||
                     dstFormat  == gcSL_UINT64 ||
                     dstFormat  == gcSL_INT64));

            /* Construct longULong processing function. */
            status = _createLongULongFunction(Shader,
                                              gcCLPatchLibrary[2],
                                              Patch,
                                              &convertFunction);
            gcmONERROR(status);

            /* Construct call stub function. */
            stubFunction = _createLongULongStubFunction(Shader,
                                                        Patch,
                                                         convertFunction);

            /* Change the instruciton to call stub */
            code = &Shader->code[Patch->instructionIndex];
            gcmASSERT (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL);
            gcSL_SetInst2NOP(code);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = Patch->instructionIndex;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction);
            Shader->lastInstruction = lastInstruction;
            Shader->instrIndex = instrIndex;
        }
    }

OnError:

    return gcvSTATUS_OK;
}
#endif

static gceSTATUS
_patchReadImage(
    IN OUT gcSHADER         Shader,
    IN gcsPatchReadImage *  ReadImage
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             origCodeCount = Shader->codeCount;
    gcFUNCTION          convertFunction = gcvNULL;
    gcFUNCTION          stubFunction = gcvNULL;
    gctUINT             i, j;

    if (gcCLPatchLibrary[0] == gcvNULL)
    {
        gcmASSERT(gcCLPatchLibrary[1] == gcvNULL);
        gcmONERROR(gcLoadCLPatchLibrary());
    }

    /* Pre-allocate more space to reduce overhead. */
    if (Shader->lastInstruction + 64 >= Shader->codeCount)
    {
        /* Allocate 160 extra instruction slots. */
        gcmONERROR(_ExpandCode(Shader, 160));
    }

    for (i = 0; i < origCodeCount; i++)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);
        gcSL_INSTRUCTION prevCode   = gcvNULL;
        gcSL_OPCODE      prevOpcode = gcSL_NOP;

        /* Find the instruction which need to do format conversion */
        if (opcode == gcSL_TEXLD)
        {
            gctUINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gctUINT lastInstruction;
            gcSHADER_INSTRUCTION_INDEX instrIndex;

            /* Assume optimizer has inline all functions that use IMAGE_RD. */
            /* texld and samplers are added by compiler, so no indexed. */
            if (samplerId != ReadImage->samplerNum)
            {
                continue;
            }

            /* TODO - Check if there is any texld modifier. */
            /* Need this for mipmap extension. */

            /* Construct read image function. */
            for (j = 0; j < CL_LIB_COUNT; j++)
            {
                status = _createReadImageFunction(
                                        Shader,
                                        gcCLPatchLibrary[j],
                                        ReadImage,
                                        (gcSL_FORMAT)gcmSL_TARGET_GET(code->temp, Format),
                                        (gcSL_FORMAT)gcmSL_SOURCE_GET(code->source1, Format),
                                        prevOpcode,
                                        &convertFunction);

                if (!gcmIS_ERROR(status))
                {
                    break;
                }
            }

            gcmONERROR(status);

            /* Construct call stub function. */
            stubFunction = _createReadImageStubFunction(
                                    Shader,
                                    ReadImage,
                                    convertFunction,
                                    i,
                                    prevCode);

            /* Change the texld instruciton to call stub */
            code = &Shader->code[i];
            gcSL_SetInst2NOP(code);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = i;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction);
            Shader->lastInstruction = lastInstruction;
            Shader->instrIndex = instrIndex;
        }
    }

    /* delete useless sampler. */
    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, Shader->uniforms[ReadImage->samplerNum]));

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_patchWriteImage(
    IN OUT gcSHADER         Shader,
    IN gcsPatchWriteImage * WriteImage
    )
{
    gceSTATUS           status = gcvSTATUS_OK;
    gctUINT             origCodeCount = Shader->codeCount;
    gcFUNCTION          convertFunction = gcvNULL;
    gcFUNCTION          stubFunction = gcvNULL;
    gctUINT             i, j;

    if (gcCLPatchLibrary[0] == gcvNULL)
    {
        gcmASSERT(gcCLPatchLibrary[1] == gcvNULL);
        gcmONERROR(gcLoadCLPatchLibrary());
    }

    for (i = 0; i < origCodeCount; i++)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = (gcSL_OPCODE)gcmSL_OPCODE_GET(code->opcode, Opcode);
        gcSL_INSTRUCTION prevCode   = gcvNULL;

        /* Find the instruction which need to do format conversion */
        if (opcode == gcSL_IMAGE_WR || opcode == gcSL_IMAGE_WR_3D)
        {
            gctUINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gctUINT lastInstruction;
            gcSHADER_INSTRUCTION_INDEX instrIndex;

            /* Assume optimizer has inline all functions that use IMAGE_WR. */
            if (samplerId != WriteImage->samplerNum)
            {
                continue;
            }
            /* Construct write image function. */
            for (j = 0; j < CL_LIB_COUNT; j++)
            {
                status = _createWriteImageFunction(
                                    Shader,
                                    gcCLPatchLibrary[j],
                                    WriteImage->channelDataType,
                                    (gcSL_FORMAT)gcmSL_TARGET_GET(code->temp, Format),
                                    WriteImage->imageType,
                                    WriteImage->channelOrder,
                                    &convertFunction);

                if (!gcmIS_ERROR(status))
                {
                    break;
                }
            }

            gcmONERROR(status);

            /* Construct call stub function. */
            stubFunction = _createWriteImageStubFunction(
                                    Shader,
                                    WriteImage,
                                    convertFunction,
                                    i,
                                    prevCode);

            /* Change the texld instruciton to call stub */
            code = &Shader->code[i];
            gcSL_SetInst2NOP(code);
            lastInstruction = Shader->lastInstruction;
            instrIndex = Shader->instrIndex;
            Shader->lastInstruction = i;
            Shader->instrIndex = gcSHADER_OPCODE;
            _addCallInst(Shader, stubFunction);
            Shader->lastInstruction = lastInstruction;
            Shader->instrIndex = instrIndex;
        }
    }

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_patchYFlippedTexture(
    IN OUT gcSHADER         Shader,
    IN gcsPatchYFilppedTexture * YFilppedTexture
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gctUINT lastInst = Shader->lastInstruction;
    const gctFLOAT negativeOneConstant = -1.0;

    for (i = Shader->lastInstruction - 1; i >= 0; i--)
    {
        gcSL_INSTRUCTION code       = &Shader->code[i];
        gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);

        /* Find the texld instruction. */
        if (opcode == gcSL_TEXLD ||
            opcode == gcSL_TEXLDPROJ)
        {
            gctINT samplerId = gcmSL_INDEX_GET(code->source0Index, Index);
            gcSL_TYPE type = gcmSL_SOURCE_GET(code->source0, Type);
            gctBOOL isSamplerMatch = gcvFALSE;
            gctINT minSamplerIndex, maxSamplerIndex;
            gctUINT insertIndex = i;
            gctUINT16 newTemp;

            gcmASSERT(type == gcSL_SAMPLER || type == gcSL_UNIFORM);

            /* TODO: need to handle indexed sampler. */
            if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
                continue;

            if (type == gcSL_SAMPLER)
            {
                if (YFilppedTexture->yFlippedTexture->arraySize > 1)
                {
                    minSamplerIndex = YFilppedTexture->yFlippedTexture->physical;
                    maxSamplerIndex = YFilppedTexture->yFlippedTexture->physical + YFilppedTexture->yFlippedTexture->arraySize - 1;
                }
                else
                {
                    minSamplerIndex = maxSamplerIndex = YFilppedTexture->yFlippedTexture->physical;
                }
            }
            else
            {
                if (YFilppedTexture->yFlippedTexture->arraySize > 1)
                {
                    minSamplerIndex = YFilppedTexture->yFlippedTexture->index;
                    maxSamplerIndex = YFilppedTexture->yFlippedTexture->index + YFilppedTexture->yFlippedTexture->index - 1;
                }
                else
                {
                    minSamplerIndex = maxSamplerIndex = YFilppedTexture->yFlippedTexture->index;
                }
            }

            if (samplerId >= minSamplerIndex && samplerId <= maxSamplerIndex)
            {
                isSamplerMatch = gcvTRUE;
            }

            if (!isSamplerMatch)
            {
                continue;
            }

            /* If there is texlod/texbias/texgrad/texldproj instruction before texld,
            ** we need to modify the insert index.
            */
            if (i)
            {
                opcode = gcmSL_OPCODE_GET(Shader->code[i - 1].opcode, Opcode);
                if (gcSL_isOpcodeTexldModifier(opcode))
                {
                    insertIndex--;
                }
            }

            /* insert two instructions. */
            gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, insertIndex, 2));
            Shader->lastInstruction = insertIndex;
            Shader->instrIndex = gcSHADER_OPCODE;

            /* MOV: temp, coord */
            newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X3);
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                                           gcSL_MOV,
                                                           newTemp,
                                                           gcSL_ENABLE_XYZ,
                                                           gcSL_FLOAT,
                                                           gcmSL_SOURCE_GET(Shader->code[insertIndex + 2].source1, Precision)));
            Shader->code[insertIndex].source0 = Shader->code[insertIndex + 2].source1;
            Shader->code[insertIndex].source0Index = Shader->code[insertIndex + 2].source1Index;
            Shader->code[insertIndex].source0Indexed = Shader->code[insertIndex + 2].source1Indexed;
            Shader->instrIndex = gcSHADER_OPCODE;
            Shader->lastInstruction++;

            /* ADD: temp.y, temp.y, -1.0 */
            gcmONERROR(gcSHADER_AddOpcode(Shader,
                                                           gcSL_ADD,
                                                           newTemp,
                                                           gcSL_ENABLE_Y,
                                                           gcSL_FLOAT,
                                                           gcmSL_SOURCE_GET(Shader->code[insertIndex + 2].source1, Precision)));

            gcmONERROR(gcSHADER_AddSource(Shader,
                                                           gcSL_TEMP,
                                                           newTemp,
                                                           gcSL_SWIZZLE_YYYY,
                                                           gcSL_FLOAT,
                                                           gcmSL_SOURCE_GET(Shader->code[insertIndex + 2].source1, Precision)));

            gcmONERROR(gcSHADER_AddSourceConstantFormatted(Shader,
                                                           (void *)&negativeOneConstant,
                                                           gcSL_FLOAT));

            /* Change the coord to the new one.*/
            Shader->code[insertIndex + 2].source1 = gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZZ)
                                                                   | gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                                                                   | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                                                   | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT);
            Shader->code[insertIndex + 2].source1Index = newTemp;
            Shader->code[insertIndex + 2].source1Indexed = 0;

            /* update the last instruction. */
            lastInst +=2;
            Shader->lastInstruction = lastInst;
        }
    }

    /* Return the status. */
    return status;
OnError:
    return status;
}

static gceSTATUS
_IsTempOutputColor(
    IN gcSHADER Shader,
    IN gctUINT16 TempIndex,
    IN gctINT16 *Index,
    IN OUT gcOUTPUT *Output
    )
{
    gctSIZE_T i;
    gcOUTPUT output = gcvNULL;

    for (i = 0; i < 4; i++)
    {
        if (Index[i] != -1 && TempIndex == (gctUINT16)Index[i])
        {
            break;
        }
    }

    if (i == 4)
    {
        return gcvSTATUS_FALSE;
    }

    for (i = 0; i < Shader->outputCount; i++)
    {
        if (Shader->outputs[i] != gcvNULL &&
            Shader->outputs[i]->tempIndex == TempIndex)
        {
            output = Shader->outputs[i];
            break;
        }
    }

    gcmASSERT(output != gcvNULL);

    if (Output != gcvNULL)
    {
        *Output = output;
    }

    return gcvTRUE;
}

static gceSTATUS
_patchRemoveAssignmentForAlphaChannel(
    IN OUT gcSHADER         Shader,
    IN gcsPatchRemoveAssignmentForAlphaChannel * OutputAlphaChannel
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T i;
    gcSL_INSTRUCTION code = gcvNULL;
    /* For a es30 shader, it may have 4 output. */
    gctINT16 index[4] = {-1, -1, -1, -1};
    gctINT outputCount = 0;
    gcOUTPUT output;

    /* This patch only works for fragment shader. */
    if (Shader->type != gcSHADER_TYPE_FRAGMENT)
        return status;

    /* Skip empty shader. */
    if (Shader->codeCount == 0 || Shader->outputCount == 0)
        return status;

    /* Find the color output. */
    /* If this is a es20 shader, it only has one output with location 0. */
    if (!gcSHADER_IsHaltiCompiler(Shader))
    {
        for (i = 0; i < Shader->outputCount; i++)
        {
            if (Shader->outputs[i] != gcvNULL && Shader->outputs[i]->nameLength == gcSL_COLOR)
            {
                index[0] = (gctINT16)Shader->outputs[i]->tempIndex;
                outputCount++;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i < Shader->outputCount; i++)
        {
            if (Shader->outputs[i] != gcvNULL && Shader->outputs[i]->nameLength !=  gcSL_DEPTH)
            {
                gcmASSERT(Shader->outputLocations[i] < 4);
                index[Shader->outputLocations[i]] = (gctINT16)Shader->outputs[i]->tempIndex;
                outputCount++;
            }
        }
    }

    gcmASSERT(outputCount <= 4);

    /* No match output. */
    if (outputCount == 0)
        return status;

    for (i = 0; i < Shader->codeCount; i++)
    {
        code = &Shader->code[i];

        if (_IsTempOutputColor(Shader, code->tempIndex, index, &output) == gcvSTATUS_FALSE)
            continue;

        if (OutputAlphaChannel->removeOutputAlpha[output->location] == gcvFALSE)
            continue;

        if (output->type != gcSHADER_FLOAT_X4 &&
            output->type != gcSHADER_INTEGER_X4 &&
            output->type != gcSHADER_UINT_X4)
            continue;

        if (gcmSL_TARGET_GET(code->temp, Enable) != gcSL_ENABLE_W)
            continue;

        gcSL_SetInst2NOP(code);
    }

    /* Return the status. */
    return status;
}

gceSTATUS
_addInstNopToEndOfMainFunc(
    IN OUT gcSHADER         Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL addNOP = gcvFALSE;
    gctSIZE_T i;
    gcFUNCTION function;

    /* If there is not other function, we need to add a NOP. */
    if (Shader->functionCount == 0)
    {
        addNOP = gcvTRUE;
    }
    else
    {
        addNOP = gcvTRUE;
        for (i = 0; i < Shader->functionCount; i++)
        {
            function = Shader->functions[i];

            if (function == gcvNULL || IsFunctionIntrinsicsBuiltIn(function))
            {
                continue;
            }

            /* If there is a function at the bottom of shader, we don't need to add a NOP. */
            if (function->codeCount + function->codeStart == Shader->codeCount)
            {
                addNOP = gcvFALSE;
                break;
            }
        }
    }

    if (addNOP)
    {
        _insertNOP2Main(Shader, 1);
        Shader->codeCount = Shader->lastInstruction;
        Shader->instrIndex = gcSHADER_OPCODE;
    }
    return status;
}

gceSTATUS
gcSHADER_DynamicPatch(
    IN OUT gcSHADER         Shader,
    IN  gcPatchDirective *  PatchDirective
    )
{
    gceSTATUS           status       = gcvSTATUS_OK;
    gcPatchDirective *  curDirective = PatchDirective;

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        gcDump_Shader(gcvNULL, "Before Dynamic Patche (recompile) Shader", gcvNULL, Shader, gcvTRUE);
    }
    /* If main functions is at the bottom of shader source,
    ** we need to add a NOP to the end of main function.
    */
    if (curDirective)
        _addInstNopToEndOfMainFunc(Shader);

    for (; curDirective; curDirective = curDirective->next)
    {
        /* check if the PatchDirective affect the Shader */
        if (!_patchApplys(Shader, curDirective))
            return gcvSTATUS_OK;

        switch (curDirective->kind) {
        case gceRK_PATCH_TEXLD_FORMAT_CONVERSION:
            status = _patchTexldFormatConversion(Shader,
                           curDirective->patchValue.formatConversion);
            break;
        case gceRK_PATCH_OUTPUT_FORMAT_CONVERSION:
            status = _patchOutputFormatConversion(Shader,
                           curDirective->patchValue.outputConversion);
            break;
        case gceRK_PATCH_DEPTH_COMPARISON:
            status = _patchDepthComparison(Shader,
                           curDirective->patchValue.depthComparison);
            break;
        case gceRK_PATCH_COLOR_FACTORING:
            status = _patchColorFactoring(Shader,
                           curDirective->patchValue.colorFactoring);
            break;
        case gceRK_PATCH_ALPHA_BLENDING:
            status = _patchAlphaBlending(Shader,
                           curDirective->patchValue.alphaBlending);
            break;
        case gceRK_PATCH_DEPTH_BIAS:
            status = _patchDepthBias(Shader,
                           curDirective->patchValue.depthBias);
            break;
        case gceRK_PATCH_NP2TEXTURE:
            status = _patchNP2Texture(Shader,
                           curDirective->patchValue.np2Texture);
            break;
        case gceRK_PATCH_GLOBAL_WORK_SIZE:
            status = _patchGlobalWorkSize(Shader,
                           curDirective->patchValue.globalWorkSize);
            break;
        case gceRK_PATCH_READ_IMAGE:
            status = _patchReadImage(Shader,
                           curDirective->patchValue.readImage);
            break;
        case gceRK_PATCH_WRITE_IMAGE:
            status = _patchWriteImage(Shader,
                           curDirective->patchValue.writeImage);
            break;
        case gceRK_PATCH_Y_FLIPPED_TEXTURE:
            status = _patchYFlippedTexture(Shader,
                           curDirective->patchValue.yFilppedTexture);
            break;
        case gceRK_PATCH_REMOVE_ASSIGNMENT_FOR_ALPHA:
            status = _patchRemoveAssignmentForAlphaChannel(Shader,
                          curDirective->patchValue.removeOutputAlpha);
            break;
        case gceRK_PATCH_Y_FLIPPED_SHADER:
            status = _patchYFlippedShader(Shader,
                           curDirective->patchValue.yFilppedShader);
            break;
        case gceRK_PATCH_SAMPLE_MASK:
            status = _patchSampleMask(Shader,
                           curDirective->patchValue.sampleMask);
            break;
        case gceRK_PATCH_SIGNEXTENT:
            status = _patchSignExtent(Shader,
                           curDirective->patchValue.signExtent);
            break;

        case gceRK_PATCH_TCS_INPUT_COUNT_MISMATCH:
            status = _patchTCSInputMismatch(Shader,
                           curDirective->patchValue.inputMismatch);
            break;

#if _SUPPORT_LONG_ULONG_DATA_TYPE
        case gceRK_PATCH_CL_LONGULONG:
            status = _patchLongULong(Shader, curDirective,
                            curDirective->patchValue.longULong);
            break;
#endif

        case gceRK_PATCH_COLOR_KILL:
            status = _patchColorKill(Shader,
                           curDirective->patchValue.colorKill);
            break;

        default:
            gcmASSERT(gcvFALSE);  /* not implemented yet */
            break;
        }
    }

    /* Trim shader instruciton and resolve labels */
    gcSHADER_Pack(Shader);

    if (gcSHADER_DumpCodeGenVerbose(Shader))
    {
        gcDump_Shader(gcvNULL, "Dynamic Patched (recompiled) Shader", gcvNULL, Shader, gcvTRUE);
    }

    /* need to reset linked in library's tempRegister map after recompilation */
    gcSHADER_ResetLibraryMappingTable(Shader);

#if _SUPPORT_LONG_ULONG_DATA_TYPE
    /* Reinitialize these static variables. The existence of these static variables are not
       thread-safe. In a multi-threaded application environment, the outcoming is not predictable.
       It is advisable to remove these static variables.
    */
    Shader->InsertCount = 0;
    Shader->InstNum = 0;
#endif

    return status;
}

static gctBOOL
_IsIntOrUIntAttribute(
    IN gcSHADER_TYPE DataType,
    OUT gcSL_FORMAT *Format
    )
{
    if (DataType >= gcSHADER_INTEGER_X1 && DataType <= gcSHADER_INTEGER_X4)
    {
        *Format = gcSL_INTEGER;
        return gcvTRUE;
    }
    else if (DataType >= gcSHADER_UINT_X1 && DataType <= gcSHADER_UINT_X4)
    {
        *Format = gcSL_UINT32;
        return gcvTRUE;
    }
    return gcvFALSE;
}


static gceSTATUS
_ConvertIntOrUIntAttribute(
    IN OUT gcSHADER Shader,
    IN OUT gcSL_FORMAT *AttribArray
    )
{
    gceSTATUS status           = gcvSTATUS_OK;
    gctSIZE_T attribCount;
    gctUINT convertAttribCount = 0;
    gctINT mainStart           = 0;
    gctINT mainEnd             = 0;
    gctUINT lastInstruction    = 0;

    /* find the beginning of main function. */
    gcmONERROR(gcSHADER_FindMainFunction(Shader, &mainStart, &mainEnd));
    mainEnd -= 1;

    /* Count the attribute number that need to convert. */
    for (attribCount = 0; attribCount < Shader->attributeCount; attribCount++)
    {
        AttribArray[attribCount] = 0;

        if (_IsIntOrUIntAttribute(Shader->attributes[attribCount]->type, &AttribArray[attribCount]))
        {
            convertAttribCount++;
        }
    }

    if (convertAttribCount == 0)
        return status;

    /* Insert nops at the beginning of main function. */
    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, mainStart, convertAttribCount));
    lastInstruction = Shader->lastInstruction;
    Shader->lastInstruction = mainStart;
    Shader->instrIndex = gcSHADER_OPCODE;

    /* For vertex shader: insert gcSL_F2I at the beginning of main function. */
    for (attribCount = 0; attribCount < Shader->attributeCount; attribCount++)
    {
        if (AttribArray[attribCount])
        {
            gctUINT16 newTemp = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, Shader->attributes[attribCount]->type);

            /* Change all users of this attribute. */
            _ChangeAttribToTempForAllCodes(Shader, Shader->attributes[attribCount]->index, newTemp);
            /* F2I: newTemp, attribute */
            gcSHADER_AddOpcode(Shader,
                                gcSL_F2I,
                                newTemp,
                                gcSL_ENABLE_XYZW,
                                AttribArray[attribCount],
                                Shader->attributes[attribCount]->precision);

            gcSHADER_AddSourceAttributeFormatted(Shader,
                                Shader->attributes[attribCount],
                                gcSL_SWIZZLE_XYZW,
                                0, /* GLES3 only support signed and unsigned integers and integer vectors, don't need index. */
                                gcSL_FLOAT);
        }
    }

    Shader->lastInstruction = lastInstruction + convertAttribCount;

OnError:
    return status;
}

gceSTATUS
gcSHADER_ConvertIntOrUIntAttribute(
    IN OUT gcSHADER VertexShader,
    IN OUT gcSHADER FragmentShader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSL_FORMAT *convertAttribArray;
    gctPOINTER pointer;

    gcmHEADER_ARG("VertexShader=0x%x, FragmentShader = 0x%x", VertexShader, FragmentShader);

    status = gcoOS_Allocate(gcvNULL,
                           gcmSIZEOF(gcSL_FORMAT) * VertexShader->attributeCount,
                           &pointer);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }
    convertAttribArray = pointer;

    /* Currently we only convert attribute on vertex shader. */
    status = _ConvertIntOrUIntAttribute(VertexShader, convertAttribArray);

    if (gcmIS_ERROR(status))
    {
        gcmFOOTER();
        return status;
    }

    gcmOS_SAFE_FREE(gcvNULL, convertAttribArray);

    gcmFOOTER();
    return status;
}

gctBOOL isAppConformance()
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if (patchId == gcvPATCH_GTFES30 || patchId == gcvPATCH_DEQP)
        return gcvTRUE;

    return gcvFALSE;   /* TODO: patch confomance app */
}

/* The followings are for compiling/linking built-in functions in the library */
/* compile the intrinsic builtin functions library */

#if DX_SHADER
gceSTATUS
gcSHADER_CompileBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS    status   = gcvSTATUS_OK;
    gctSTRING    log      = gcvNULL;
    gctFILE      libFile  = gcvNULL;
    gctUINT8_PTR buffer   = gcvNULL;

    *Binary = gcvNULL;
    if (gcBuiltinLibrary0 == gcvNULL)
    {
        gctCHAR     libName[260]    = { 0 };
        gctUINT     libNameOffset   = 0;
        gctCHAR     winDir[260]     = { 0 };
        gctUINT32   fileSize        = 0;
        gctSIZE_T   bufferSize      = 0;

        gcmONERROR(gcSHADER_Construct(gcvNULL, gcSHADER_TYPE_FRAGMENT, &gcBuiltinLibrary0));

        /* Really should be in gcOS layer!!!  Since this functions works with char strings we need
           to explicitly call the ANSI version since Windows driver can build with Unicode as default */
        GetWindowsDirectoryA(winDir, 260);

        /* Load library based on hardware capability. */
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0))
        {
            gcoOS_PrintStrSafe(libName, gcmSIZEOF(libName), &libNameOffset,
                "%s\\system32\\libbuiltin0.pgcSL", winDir);
        }
        else
        {
            gcmASSERT(gcvFALSE);
            return gcvSTATUS_NOT_SUPPORTED;
        }

        gcmONERROR(gcoOS_Open(gcvNULL, libName, gcvFILE_READ, &libFile));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_END));
        gcmONERROR(gcoOS_GetPos(gcvNULL, libFile, &fileSize));

        gcmONERROR(gcoOS_Allocate(gcvNULL, fileSize, (gctPOINTER *) &buffer));
        gcmONERROR(gcoOS_Seek(gcvNULL, libFile, 0, gcvFILE_SEEK_SET));
        gcmONERROR(gcoOS_Read(gcvNULL, libFile, fileSize, buffer, &bufferSize));
        gcmASSERT(fileSize == bufferSize);

        gcmONERROR(gcSHADER_Load(gcBuiltinLibrary0, buffer, bufferSize));
    }

    if (gcBuiltinLibrary0 != gcvNULL)
    {
        *Binary = gcBuiltinLibrary0;
    }

OnError:
    if (libFile != gcvNULL)
    {
        gcoOS_Close(gcvNULL, libFile);
    }

    if (buffer != gcvNULL)
    {
        gcoOS_Free(gcvNULL, buffer);
    }

    if (status != gcvSTATUS_OK)
    {
        /* report error */
        gcoOS_Print("Compiler Error:\n%s\n", log);
    }

    /* Return the status. */
    return status;
}

#else

gceSTATUS
gcSHADER_CompileBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS   status          = gcvSTATUS_OK;
    gctSTRING   sloBuiltinSource = gcvNULL;

    gctSIZE_T   length;
    gctPOINTER  pointer = gcvNULL;
    gctINT      i, stringNum = 0;
    gctSTRING   log    = gcvNULL;

    gctBOOL     isHalti5 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI5);
    gctBOOL     isHalti4 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI4);
    gctBOOL     isHalti2 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2);
    gctBOOL     isHalti0 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI0);
    gctBOOL     isSupportTextureGather = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TEXTURE_GATHER);
    gctBOOL     isSupportImgAddr = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_IMG_INSTRUCTION);
    gctBOOL     isSupportImgInst = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI5) ?
                                    gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_USC_GOS_ADDR_FIX) :
                                    isSupportImgAddr;
    gctBOOL     isSupportTexelFetchForMSAA = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_MSAA_TEXTURE);
    /* Use extension string to check extension feature. */
    gctBOOL     isSupportTexMSAA2DArray = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_texture_storage_multisample_2d_array", gcvNULL);
    gctBOOL     isSupportCubeMapArray = gcoOS_StrStr(GetGLExtensionString(), "GL_EXT_texture_cube_map_array", gcvNULL);
    gctBOOL     isSupportTextureBuffer = gcoOS_StrStr(GetGLExtensionString(), "GL_EXT_texture_buffer", gcvNULL);
    gctBOOL     isSupportMSShading = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_shader_multisample_interpolation", gcvNULL);

    /* built-in function library */

    /* the following builtin functions have different implementation in gc3000/5000 and gc7000 */

    /* gc3000/5000 implementation */
    gctSTRING   BuiltinLib[] =
    {
        gcLibFindLSB_Func_1,
        gcLibFindLSB_Func_2,
        gcLibFindLSB_Func_3,
        gcLibFindLSB_Func_4,
        gcLibFindLSB_Func_5,
        gcLibFindLSB_Func_6,
        gcLibFindLSB_Func_7,
        gcLibFindLSB_Func_8,

        gcLibFindMSB_Func_1,
        gcLibFindMSB_Func_2,
        gcLibFindMSB_Func_3,
        gcLibFindMSB_Func_4,
        gcLibFindMSB_Func_5,
        gcLibFindMSB_Func_6,
        gcLibFindMSB_Func_7,
        gcLibFindMSB_Func_8,

        gcLibBitfieldReverse_Func_1,
        gcLibBitfieldReverse_Func_2,
        gcLibBitfieldReverse_Func_3,
        gcLibBitfieldReverse_Func_4,
        gcLibBitfieldReverse_Func_5,
        gcLibBitfieldReverse_Func_6,
        gcLibBitfieldReverse_Func_7,
        gcLibBitfieldReverse_Func_8,

        gcLibBitfieldExtract_Func_1,
        gcLibBitfieldExtract_Func_2,
        gcLibBitfieldExtract_Func_3,
        gcLibBitfieldExtract_Func_4,
        gcLibBitfieldExtract_Func_5,
        gcLibBitfieldExtract_Func_6,
        gcLibBitfieldExtract_Func_7,
        gcLibBitfieldExtract_Func_8,

        gcLibBitfieldInsert_Func_1,
        gcLibBitfieldInsert_Func_2,
        gcLibBitfieldInsert_Func_3,
        gcLibBitfieldInsert_Func_4,
        gcLibBitfieldInsert_Func_5,
        gcLibBitfieldInsert_Func_6,
        gcLibBitfieldInsert_Func_7,
        gcLibBitfieldInsert_Func_8,

        gcLibUaddCarry_Func_1,
        gcLibUaddCarry_Func_2,
        gcLibUaddCarry_Func_3,
        gcLibUaddCarry_Func_4,
    };

    /* gc7000 implementation */
    gctSTRING   BuiltinLib_hati4[] =
    {
        gcLibFindLSB_Func_1_hati4,
        gcLibFindLSB_Func_2_hati4,
        gcLibFindLSB_Func_3_hati4,
        gcLibFindLSB_Func_4_hati4,
        gcLibFindLSB_Func_5_hati4,
        gcLibFindLSB_Func_6_hati4,
        gcLibFindLSB_Func_7_hati4,
        gcLibFindLSB_Func_8_hati4,

        gcLibFindMSB_Func_1_hati4,
        gcLibFindMSB_Func_2_hati4,
        gcLibFindMSB_Func_3_hati4,
        gcLibFindMSB_Func_4_hati4,
        gcLibFindMSB_Func_5_hati4,
        gcLibFindMSB_Func_6_hati4,
        gcLibFindMSB_Func_7_hati4,
        gcLibFindMSB_Func_8_hati4,

        gcLibBitfieldReverse_Func_1_hati4,
        gcLibBitfieldReverse_Func_2_hati4,
        gcLibBitfieldReverse_Func_3_hati4,
        gcLibBitfieldReverse_Func_4_hati4,
        gcLibBitfieldReverse_Func_5_hati4,
        gcLibBitfieldReverse_Func_6_hati4,
        gcLibBitfieldReverse_Func_7_hati4,
        gcLibBitfieldReverse_Func_8_hati4,

        gcLibBitfieldExtract_Func_1_hati4,
        gcLibBitfieldExtract_Func_2_hati4,
        gcLibBitfieldExtract_Func_3_hati4,
        gcLibBitfieldExtract_Func_4_hati4,
        gcLibBitfieldExtract_Func_5_hati4,
        gcLibBitfieldExtract_Func_6_hati4,
        gcLibBitfieldExtract_Func_7_hati4,
        gcLibBitfieldExtract_Func_8_hati4,

        gcLibBitfieldInsert_Func_1_hati4,
        gcLibBitfieldInsert_Func_2_hati4,
        gcLibBitfieldInsert_Func_3_hati4,
        gcLibBitfieldInsert_Func_4_hati4,
        gcLibBitfieldInsert_Func_5_hati4,
        gcLibBitfieldInsert_Func_6_hati4,
        gcLibBitfieldInsert_Func_7_hati4,
        gcLibBitfieldInsert_Func_8_hati4,

        gcLibUaddCarry_Func_1_hati4,
        gcLibUaddCarry_Func_2_hati4,
        gcLibUaddCarry_Func_3_hati4,
        gcLibUaddCarry_Func_4_hati4,

    };

#define BUILTINLIB_MIX_IDX    0
    /* the following builtin functions have same implementation in gc3000/5000 and gc7000 */
    gctSTRING   BuiltinLib_Common[] =
    {
        gcLib_2instMixFunc, /* it can be replaced with 3 inst version for comformance */

        gcLibMODF_Func_1,
        gcLibMODF_Func_2,
        gcLibMODF_Func_3,
        gcLibMODF_Func_4,

        /* common functions */
        gcLibCommon_Func,
        gcLibACOSH_Funcs,

        gcLibLDEXP_Func_1,
        gcLibLDEXP_Func_2,
        gcLibLDEXP_Func_3,
        gcLibLDEXP_Func_4,

        gcLibFREXP_Func_1,
        gcLibFREXP_Func_2,
        gcLibFREXP_Func_3,
        gcLibFREXP_Func_4,

        gcLibUsubBorrow_Func_1,
        gcLibUsubBorrow_Func_2,
        gcLibUsubBorrow_Func_3,
        gcLibUsubBorrow_Func_4,

        gcLibPack_Func,
        gcLibUnpack_Func,

        gcLibUmulExtended_Func_1,
        gcLibUmulExtended_Func_2,
        gcLibUmulExtended_Func_3,
        gcLibUmulExtended_Func_4,
        gcLibImulExtended_Func_1,
        gcLibImulExtended_Func_2,
        gcLibImulExtended_Func_3,
        gcLibImulExtended_Func_4,

        gcLibFMA_Func_1,
        gcLibFMA_Func_2,
        gcLibFMA_Func_3,
        gcLibFMA_Func_4,

        /* textureSize functions. */
        gcLibTextureSize_Func_1,
        gcLibTextureSize_Func_2,
        gcLibTextureSize_Func_3,
        gcLibTextureSize_Func_4,
        gcLibTextureSize_Func_5,
        gcLibTextureSize_Func_6,
        gcLibTextureSize_Func_7,
        gcLibTextureSize_Func_8,
        gcLibTextureSize_Func_9,
        gcLibTextureSize_Func_10,
        gcLibTextureSize_Func_11,
        gcLibTextureSize_Func_12,
        gcLibTextureSize_Func_13,
        gcLibTextureSize_Func_14,
        gcLibTextureSize_Func_15,
        gcLibTextureSize_Func_16,
        gcLibTextureSize_Func_17,
        gcLibTextureSize_Func_18,

        gcLibTextureGatherCommon_Func_1,
    };

    gctSTRING   BuiltinLib_Reflect[] =
    {
        gcLibREFLECT_Func_1,
        gcLibREFLECT_Func_2,
        gcLibREFLECT_Func_3,
        gcLibREFLECT_Func_4,
    };
    gctSTRING   BuiltinLib_Reflect_halti5[] =
    {
        gcLibREFLECT_Func_1_halti5,
        gcLibREFLECT_Func_2_halti5,
        gcLibREFLECT_Func_3_halti5,
        gcLibREFLECT_Func_4_halti5,
    };

    /* advanced blend equation library */
    /* blend equations have different implementation on gc3000/5000 and gc7000,
       since on gc7000, hardware has some advanced blend equation support and
       texld_u instruction. While, on gc3000/5000, there are no such support. */

    /* gc3000/5000 implementation */
    gctSTRING   BlendEquationLib[] =
    {
        gcLibBlendEquation_Multiply,
        gcLibBlendEquation_Screen,
        gcLibBlendEquation_Overlay,
        gcLibBlendEquation_Darken,
        gcLibBlendEquation_Lighten,
        gcLibBlendEquation_Hardlight,
        gcLibBlendEquation_Difference,
        gcLibBlendEquation_Exclusion,

        gcLibBlendEquation_Colordodge,
        gcLibBlendEquation_Colorburn,
        gcLibBlendEquation_Softlight,
        gcLibBlendEquation_HSL_HUE,
        gcLibBlendEquation_HSL_SATURATION,
        gcLibBlendEquation_HSL_COLOR,
        gcLibBlendEquation_HSL_LUMINOSITY,
        gcLibBlendEquation_ALL,

    };

    /* gc7000 implementation */
    gctSTRING   BlendEquationLib_hati4[] =
    {
        gcLibBlendEquation_Colordodge_hati4,
        gcLibBlendEquation_Colorburn_hati4,
        gcLibBlendEquation_Softlight_hati4,
        gcLibBlendEquation_HSL_HUE_hati4,
        gcLibBlendEquation_HSL_SATURATION_hati4,
        gcLibBlendEquation_HSL_COLOR_hati4,
        gcLibBlendEquation_HSL_LUMINOSITY_hati4,
        gcLibBlendEquation_ALL_hati4,

    };

    /* HW that can directly support textureGather. */
    gctSTRING TextureGatherLib[] =
    {
        gcLibTextureGather_Func_1,
        gcLibTextureGather_Func_2,
        gcLibTextureGather_Func_3,
        gcLibTextureGather_Func_4,
        gcLibTextureGather_Func_5,
        gcLibTextureGather_Func_6,
        gcLibTextureGather_Func_7,
        gcLibTextureGather_Func_8,
        gcLibTextureGather_Func_9,
        gcLibTextureGather_Func_10,
        gcLibTextureGather_Func_11,
        gcLibTextureGather_Func_12,
        gcLibTextureGather_Func_13,
        gcLibTextureGather_Func_14,
        gcLibTextureGather_Func_15,
        gcLibTextureGather_Func_16,
        gcLibTextureGather_Func_17,
        gcLibTextureGather_Func_18,
        gcLibTextureGather_Func_19,
        gcLibTextureGather_Func_20,
        gcLibTextureGather_Func_21
    };

    /* HW that can't directly support textureGather. */
    gctSTRING TextureGatherLib_2[] =
    {
        gcLibTextureGather_Func_2_0,
        gcLibTextureGather_Func_2_1,
        gcLibTextureGather_Func_2_2,
        gcLibTextureGather_Func_2_3,
        gcLibTextureGather_Func_2_4,
        gcLibTextureGather_Func_2_5,
        gcLibTextureGather_Func_2_6,
        gcLibTextureGather_Func_2_7,
        gcLibTextureGather_Func_2_8,
        gcLibTextureGather_Func_2_9,
        gcLibTextureGather_Func_2_10,
        gcLibTextureGather_Func_2_11,
        gcLibTextureGather_Func_2_12,
        gcLibTextureGather_Func_2_13,
        gcLibTextureGather_Func_2_14,
        gcLibTextureGather_Func_2_15,
        gcLibTextureGather_Func_2_16,
        gcLibTextureGather_Func_2_17,
        gcLibTextureGather_Func_2_18,
        gcLibTextureGather_Func_2_19,
        gcLibTextureGather_Func_2_20,
        gcLibTextureGather_Func_2_21
    };

    gctSTRING TextureGatherOffsetLib[] =
    {
        gcLibTextureGatherOffset_Func_1,
        gcLibTextureGatherOffset_Func_2,
        gcLibTextureGatherOffset_Func_3,
        gcLibTextureGatherOffset_Func_4,
        gcLibTextureGatherOffset_Func_5,
        gcLibTextureGatherOffset_Func_6,
        gcLibTextureGatherOffset_Func_7,
        gcLibTextureGatherOffset_Func_8,
        gcLibTextureGatherOffset_Func_9,
        gcLibTextureGatherOffset_Func_10,
        gcLibTextureGatherOffset_Func_11,
        gcLibTextureGatherOffset_Func_12,
        gcLibTextureGatherOffset_Func_13,
        gcLibTextureGatherOffset_Func_14
    };

    gctSTRING TextureGatherOffsetsLib[] =
    {
        gcLibTextureGatherOffsets_Func_1,
        gcLibTextureGatherOffsets_Func_2,
        gcLibTextureGatherOffsets_Func_3,
        gcLibTextureGatherOffsets_Func_4,
        gcLibTextureGatherOffsets_Func_5,
        gcLibTextureGatherOffsets_Func_6,
        gcLibTextureGatherOffsets_Func_7,
        gcLibTextureGatherOffsets_Func_8,
        gcLibTextureGatherOffsets_Func_9,
        gcLibTextureGatherOffsets_Func_10,
        gcLibTextureGatherOffsets_Func_11,
        gcLibTextureGatherOffsets_Func_12,
        gcLibTextureGatherOffsets_Func_13,
        gcLibTextureGatherOffsets_Func_14
    };

    gctSTRING TexelFetchForMSAALib[] =
    {
        gcLibTexelFetchForMSAA_Func_1,
        gcLibTexelFetchForMSAA_Func_2,
        gcLibTexelFetchForMSAA_Func_3,
    };

    gctSTRING TexelFetchForMSAALib_2[] =
    {
        gcLibTexelFetchForMSAA_Func_2_1,
        gcLibTexelFetchForMSAA_Func_2_2,
        gcLibTexelFetchForMSAA_Func_2_3,
    };

    gctSTRING TexMS2DArrayLib[] =
    {
        gcLibTextureSize_Func_19,
        gcLibTextureSize_Func_20,
        gcLibTextureSize_Func_21,
        gcLibTexelFetchForMSAA_Func_4,
        gcLibTexelFetchForMSAA_Func_5,
        gcLibTexelFetchForMSAA_Func_6
    };

    gctSTRING TexMS2DArrayLib_2[] =
    {
        gcLibTextureSize_Func_19,
        gcLibTextureSize_Func_20,
        gcLibTextureSize_Func_21,
        gcLibTexelFetchForMSAA_Func_2_4,
        gcLibTexelFetchForMSAA_Func_2_5,
        gcLibTexelFetchForMSAA_Func_2_6
    };

    gctSTRING ImageLib_common[] =
    {
        gcLibImageSize,
        gcLibImageSize_2D_float,
        gcLibImageSize_3D_float,
        gcLibImageSize_CUBE_float,
        gcLibImageSize_2DArray_float,
        gcLibImageSize_2D_int,
        gcLibImageSize_3D_int,
        gcLibImageSize_CUBE_int,
        gcLibImageSize_2DArray_int,
        gcLibImageSize_2D_uint,
        gcLibImageSize_3D_uint,
        gcLibImageSize_CUBE_uint,
        gcLibImageSize_2DArray_uint,
    };

    /* image_load, image_store gc3000/5000 implementation */
    gctSTRING ImageLib[] =
    {
        /* gcLibImageAddr must be the first element. */
        gcLibImageAddr,
        gcLibImageSwizzle,
        gcLibImageStoreSwizzle,

        gcLibImageLoad_2D_int, /* 16i */
        gcLibImageLoad_2D_int_rgba32i,
        gcLibImageLoad_2D_int_rgba8i,
        gcLibImageLoad_2D_int_r32i,
        gcLibImageLoad_2D_uint, /* 16ui */
        gcLibImageLoad_2D_uint_rgba32ui,
        gcLibImageLoad_2D_uint_rgba8ui,
        gcLibImageLoad_2D_uint_r32ui,
        gcLibImageLoad_2D_float, /* 16f */
        gcLibImageLoad_2D_float_rgba8,
        gcLibImageLoad_2D_float_rgba8_snorm,
        gcLibImageLoad_2D_float_rgba32f,
        gcLibImageLoad_2D_float_r32f,

        gcLibImageLoad_3Dcommon,
        gcLibImageLoad_3D,
        gcLibImageLoad_cube,
        gcLibImageLoad_2DArray,

        gcLibImageStore_2D_float, /* 16f */
        gcLibImageStore_2D_float_rgba32f,
        gcLibImageStore_2D_float_r32f,
        gcLibImageStore_2D_float_rgba8,
        gcLibImageStore_2D_float_rgba8_snorm,
        gcLibImageStore_2D_int, /* 16i */
        gcLibImageStore_2D_int_rgba32i,
        gcLibImageStore_2D_int_r32i,
        gcLibImageStore_2D_int_rgba8i,
        gcLibImageStore_2D_uint, /* 16ui */
        gcLibImageStore_2D_uint_rgba32ui,
        gcLibImageStore_2D_uint_r32ui,
        gcLibImageStore_2D_uint_rgba8ui,

        gcLibImageStore_3Dcommon,
        gcLibImageStore_3D,
        gcLibImageStore_cube,
        gcLibImageStore_2DArray,

        gcLibImageAtomicAdd_2D_int,
        gcLibImageAtomicAdd_2D_uint,
        gcLibImageAtomicAdd_3D_int,
        gcLibImageAtomicAdd_3D_uint,
        gcLibImageAtomicAdd_CUBE_int,
        gcLibImageAtomicAdd_CUBE_uint,
        gcLibImageAtomicAdd_2DARRAY_int,
        gcLibImageAtomicAdd_2DARRAY_uint,
        gcLibImageAtomicMin_2D_int,
        gcLibImageAtomicMin_2D_uint,
        gcLibImageAtomicMin_3D_int,
        gcLibImageAtomicMin_3D_uint,
        gcLibImageAtomicMin_CUBE_int,
        gcLibImageAtomicMin_CUBE_uint,
        gcLibImageAtomicMin_2DARRAY_int,
        gcLibImageAtomicMin_2DARRAY_uint,
        gcLibImageAtomicMax_2D_int,
        gcLibImageAtomicMax_2D_uint,
        gcLibImageAtomicMax_3D_int,
        gcLibImageAtomicMax_3D_uint,
        gcLibImageAtomicMax_CUBE_int,
        gcLibImageAtomicMax_CUBE_uint,
        gcLibImageAtomicMax_2DARRAY_int,
        gcLibImageAtomicMax_2DARRAY_uint,
        gcLibImageAtomicAnd_2D_int,
        gcLibImageAtomicAnd_2D_uint,
        gcLibImageAtomicAnd_3D_int,
        gcLibImageAtomicAnd_3D_uint,
        gcLibImageAtomicAnd_CUBE_int,
        gcLibImageAtomicAnd_CUBE_uint,
        gcLibImageAtomicAnd_2DARRAY_int,
        gcLibImageAtomicAnd_2DARRAY_uint,
        gcLibImageAtomicOr_2D_int,
        gcLibImageAtomicOr_2D_uint,
        gcLibImageAtomicOr_3D_int,
        gcLibImageAtomicOr_3D_uint,
        gcLibImageAtomicOr_CUBE_int,
        gcLibImageAtomicOr_CUBE_uint,
        gcLibImageAtomicOr_2DARRAY_int,
        gcLibImageAtomicOr_2DARRAY_uint,
        gcLibImageAtomicXor_2D_int,
        gcLibImageAtomicXor_2D_uint,
        gcLibImageAtomicXor_3D_int,
        gcLibImageAtomicXor_3D_uint,
        gcLibImageAtomicXor_CUBE_int,
        gcLibImageAtomicXor_CUBE_uint,
        gcLibImageAtomicXor_2DARRAY_int,
        gcLibImageAtomicXor_2DARRAY_uint,
        gcLibImageAtomicXchg_2D_int,
        gcLibImageAtomicXchg_2D_uint,
        gcLibImageAtomicXchg_2D_float,
        gcLibImageAtomicXchg_3D_int,
        gcLibImageAtomicXchg_3D_uint,
        gcLibImageAtomicXchg_3D_float,
        gcLibImageAtomicXchg_CUBE_int,
        gcLibImageAtomicXchg_CUBE_uint,
        gcLibImageAtomicXchg_CUBE_float,
        gcLibImageAtomicXchg_2DARRAY_int,
        gcLibImageAtomicXchg_2DARRAY_uint,
        gcLibImageAtomicXchg_2DARRAY_float,
        gcLibImageAtomicCmpXchg_2D_int,
        gcLibImageAtomicCmpXchg_2D_uint,
        gcLibImageAtomicCmpXchg_3D_int,
        gcLibImageAtomicCmpXchg_3D_uint,
        gcLibImageAtomicCmpXchg_CUBE_int,
        gcLibImageAtomicCmpXchg_CUBE_uint,
        gcLibImageAtomicCmpXchg_2DARRAY_int,
        gcLibImageAtomicCmpXchg_2DARRAY_uint,
    };

    /* image_load, image_store gc7000 implementation */
    gctSTRING ImageLib_hati4[] =
    {
        gcLibImageLoad_2D_float_hati4,
        gcLibImageLoad_2D_float_1_hati4,
        gcLibImageLoad_2D_int_hati4,
        gcLibImageLoad_2D_int_1_hati4,
        gcLibImageLoad_2D_uint_hati4,
        gcLibImageLoad_2D_uint_1_hati4,

        gcLibImageLoad_3D_float_hati4,
        gcLibImageLoad_3D_float_1_hati4,
        gcLibImageLoad_3D_int_hati4,
        gcLibImageLoad_3D_int_1_hati4,
        gcLibImageLoad_3D_uint_hati4,
        gcLibImageLoad_3D_uint_1_hati4,

        gcLibImageLoad_cube_float_hati4,
        gcLibImageLoad_cube_float_1_hati4,
        gcLibImageLoad_cube_int_hati4,
        gcLibImageLoad_cube_int_1_hati4,
        gcLibImageLoad_cube_uint_hati4,
        gcLibImageLoad_cube_uint_1_hati4,

        gcLibImageLoad_2DArray_float_hati4,
        gcLibImageLoad_2DArray_float_1_hati4,
        gcLibImageLoad_2DArray_int_hati4,
        gcLibImageLoad_2DArray_int_1_hati4,
        gcLibImageLoad_2DArray_uint_hati4,
        gcLibImageLoad_2DArray_uint_1_hati4,

        gcLibImageStore_2D_float_hati4,
        gcLibImageStore_2D_float_1_hati4,
        gcLibImageStore_2D_int_hati4,
        gcLibImageStore_2D_int_1_hati4,
        gcLibImageStore_2D_uint_hati4,
        gcLibImageStore_2D_uint_1_hati4,

        gcLibImageStore_3D_float_hati4,
        gcLibImageStore_3D_float_1_hati4,
        gcLibImageStore_3D_int_hati4,
        gcLibImageStore_3D_int_1_hati4,
        gcLibImageStore_3D_uint_hati4,
        gcLibImageStore_3D_uint_1_hati4,

        gcLibImageStore_cube_float_hati4,
        gcLibImageStore_cube_float_1_hati4,
        gcLibImageStore_cube_int_hati4,
        gcLibImageStore_cube_int_1_hati4,
        gcLibImageStore_cube_uint_hati4,
        gcLibImageStore_cube_uint_1_hati4,

        gcLibImageStore_2DArray_float_hati4,
        gcLibImageStore_2DArray_float_1_hati4,
        gcLibImageStore_2DArray_int_hati4,
        gcLibImageStore_2DArray_int_1_hati4,
        gcLibImageStore_2DArray_uint_hati4,
        gcLibImageStore_2DArray_uint_1_hati4,

        gcLibImageAtomicAdd_2D_int_hati4,
        gcLibImageAtomicAdd_2D_uint_hati4,
        gcLibImageAtomicAdd_3D_int_hati4,
        gcLibImageAtomicAdd_3D_uint_hati4,
        gcLibImageAtomicAdd_CUBE_int_hati4,
        gcLibImageAtomicAdd_CUBE_uint_hati4,
        gcLibImageAtomicAdd_2DARRAY_int_hati4,
        gcLibImageAtomicAdd_2DARRAY_uint_hati4,
        gcLibImageAtomicMin_2D_int_hati4,
        gcLibImageAtomicMin_2D_uint_hati4,
        gcLibImageAtomicMin_3D_int_hati4,
        gcLibImageAtomicMin_3D_uint_hati4,
        gcLibImageAtomicMin_CUBE_int_hati4,
        gcLibImageAtomicMin_CUBE_uint_hati4,
        gcLibImageAtomicMin_2DARRAY_int_hati4,
        gcLibImageAtomicMin_2DARRAY_uint_hati4,
        gcLibImageAtomicMax_2D_int_hati4,
        gcLibImageAtomicMax_2D_uint_hati4,
        gcLibImageAtomicMax_3D_int_hati4,
        gcLibImageAtomicMax_3D_uint_hati4,
        gcLibImageAtomicMax_CUBE_int_hati4,
        gcLibImageAtomicMax_CUBE_uint_hati4,
        gcLibImageAtomicMax_2DARRAY_int_hati4,
        gcLibImageAtomicMax_2DARRAY_uint_hati4,
        gcLibImageAtomicAnd_2D_int_hati4,
        gcLibImageAtomicAnd_2D_uint_hati4,
        gcLibImageAtomicAnd_3D_int_hati4,
        gcLibImageAtomicAnd_3D_uint_hati4,
        gcLibImageAtomicAnd_CUBE_int_hati4,
        gcLibImageAtomicAnd_CUBE_uint_hati4,
        gcLibImageAtomicAnd_2DARRAY_int_hati4,
        gcLibImageAtomicAnd_2DARRAY_uint_hati4,
        gcLibImageAtomicOr_2D_int_hati4,
        gcLibImageAtomicOr_2D_uint_hati4,
        gcLibImageAtomicOr_3D_int_hati4,
        gcLibImageAtomicOr_3D_uint_hati4,
        gcLibImageAtomicOr_CUBE_int_hati4,
        gcLibImageAtomicOr_CUBE_uint_hati4,
        gcLibImageAtomicOr_2DARRAY_int_hati4,
        gcLibImageAtomicOr_2DARRAY_uint_hati4,
        gcLibImageAtomicXor_2D_int_hati4,
        gcLibImageAtomicXor_2D_uint_hati4,
        gcLibImageAtomicXor_3D_int_hati4,
        gcLibImageAtomicXor_3D_uint_hati4,
        gcLibImageAtomicXor_CUBE_int_hati4,
        gcLibImageAtomicXor_CUBE_uint_hati4,
        gcLibImageAtomicXor_2DARRAY_int_hati4,
        gcLibImageAtomicXor_2DARRAY_uint_hati4,
        gcLibImageAtomicXchg_2D_int_hati4,
        gcLibImageAtomicXchg_2D_uint_hati4,
        gcLibImageAtomicXchg_2D_float_hati4,
        gcLibImageAtomicXchg_3D_int_hati4,
        gcLibImageAtomicXchg_3D_uint_hati4,
        gcLibImageAtomicXchg_3D_float_hati4,
        gcLibImageAtomicXchg_CUBE_int_hati4,
        gcLibImageAtomicXchg_CUBE_uint_hati4,
        gcLibImageAtomicXchg_CUBE_float_hati4,
        gcLibImageAtomicXchg_2DARRAY_int_hati4,
        gcLibImageAtomicXchg_2DARRAY_uint_hati4,
        gcLibImageAtomicXchg_2DARRAY_float_hati4,
        gcLibImageAtomicCmpXchg_2D_int_hati4,
        gcLibImageAtomicCmpXchg_2D_uint_hati4,
        gcLibImageAtomicCmpXchg_3D_int_hati4,
        gcLibImageAtomicCmpXchg_3D_uint_hati4,
        gcLibImageAtomicCmpXchg_CUBE_int_hati4,
        gcLibImageAtomicCmpXchg_CUBE_uint_hati4,
        gcLibImageAtomicCmpXchg_2DARRAY_int_hati4,
        gcLibImageAtomicCmpXchg_2DARRAY_uint_hati4,
    };

    /*--------------------extension built-in support--------------------*/
    /* texture buffer related built-in functions. */
    gctSTRING TextureBuffer_general[] =
    {
        gcLibTextureSize_Func_26,
        gcLibTextureSize_Func_27,
        gcLibTextureSize_Func_28,
        gcLibImageSize_Buffer_float,
        gcLibImageSize_Buffer_int,
        gcLibImageSize_Buffer_uint,
    };

    gctSTRING TextureBuffer[] =
    {
        /* imageLoad/imageStore.*/
        gcLibImageLoad_Buffer_int, /* 16i */
        gcLibImageLoad_Buffer_int_rgba32i,
        gcLibImageLoad_Buffer_int_rgba8i,
        gcLibImageLoad_Buffer_int_r32i,
        gcLibImageLoad_Buffer_uint, /* 16ui */
        gcLibImageLoad_Buffer_uint_rgba32ui,
        gcLibImageLoad_Buffer_uint_rgba8ui,
        gcLibImageLoad_Buffer_uint_r32ui,
        gcLibImageLoad_Buffer_float, /* 16f */
        gcLibImageLoad_Buffer_float_rgba8,
        gcLibImageLoad_Buffer_float_rgba8_snorm,
        gcLibImageLoad_Buffer_float_rgba32f,
        gcLibImageLoad_Buffer_float_r32f,
        gcLibImageStore_Buffer_float, /* 16f */
        gcLibImageStore_Buffer_float_rgba32f,
        gcLibImageStore_Buffer_float_r32f,
        gcLibImageStore_Buffer_float_rgba8,
        gcLibImageStore_Buffer_float_rgba8_snorm,
        gcLibImageStore_Buffer_int, /* 16i */
        gcLibImageStore_Buffer_int_rgba32i,
        gcLibImageStore_Buffer_int_r32i,
        gcLibImageStore_Buffer_int_rgba8i,
        gcLibImageStore_Buffer_uint, /* 16ui */
        gcLibImageStore_Buffer_uint_rgba32ui,
        gcLibImageStore_Buffer_uint_r32ui,
        gcLibImageStore_Buffer_uint_rgba8ui,

        /* imageAtomicXXX. */
        gcLibImageAtomicAdd_buffer_int,
        gcLibImageAtomicAdd_buffer_uint,

        gcLibImageAtomicMin_buffer_int,
        gcLibImageAtomicMin_buffer_uint,

        gcLibImageAtomicMax_buffer_int,
        gcLibImageAtomicMax_buffer_uint,

        gcLibImageAtomicAnd_buffer_int,
        gcLibImageAtomicAnd_buffer_uint,

        gcLibImageAtomicOr_buffer_int,
        gcLibImageAtomicOr_buffer_uint,

        gcLibImageAtomicXor_buffer_int,
        gcLibImageAtomicXor_buffer_uint,

        gcLibImageAtomicXchg_buffer_int,
        gcLibImageAtomicXchg_buffer_uint,
        gcLibImageAtomicXchg_buffer_float,

        gcLibImageAtomicCmpXchg_buffer_int,
        gcLibImageAtomicCmpXchg_buffer_uint,
    };

    gctSTRING TextureBuffer_support_img_access[] =
    {
        /* imageLoad/imageStore.*/
        gcLibImageLoad_Buffer_float_img_access,
        gcLibImageLoad_Buffer_int_img_access,
        gcLibImageLoad_Buffer_uint_img_access,
        gcLibImageStore_Buffer_float_img_access,
        gcLibImageStore_Buffer_int_img_access,
        gcLibImageStore_Buffer_uint_img_access,

        /* imageAtomicXXX. */
        gcLibImageAtomicAdd_buffer_int_img_access,
        gcLibImageAtomicAdd_buffer_uint_img_access,

        gcLibImageAtomicMin_buffer_int_img_access,
        gcLibImageAtomicMin_buffer_uint_img_access,

        gcLibImageAtomicMax_buffer_int_img_access,
        gcLibImageAtomicMax_buffer_uint_img_access,

        gcLibImageAtomicAnd_buffer_int_img_access,
        gcLibImageAtomicAnd_buffer_uint_img_access,

        gcLibImageAtomicOr_buffer_int_img_access,
        gcLibImageAtomicOr_buffer_uint_img_access,

        gcLibImageAtomicXor_buffer_int_img_access,
        gcLibImageAtomicXor_buffer_uint_img_access,

        gcLibImageAtomicXchg_buffer_int_img_access,
        gcLibImageAtomicXchg_buffer_uint_img_access,
        gcLibImageAtomicXchg_buffer_float_img_access,

        gcLibImageAtomicCmpXchg_buffer_int_img_access,
        gcLibImageAtomicCmpXchg_buffer_uint_img_access,
    };

    /* cubeMap related built-in functions. */
    gctSTRING ImageLib_cubeMapArray_general[] =
    {
        gcLibTextureSize_Func_22,
        gcLibTextureSize_Func_23,
        gcLibTextureSize_Func_24,
        gcLibTextureSize_Func_25,

        gcLibImageSize_CubeArray_float,
        gcLibImageSize_CubeArray_int,
        gcLibImageSize_CubeArray_uint,
    };

    gctSTRING ImageLib_cubeMapArray[] =
    {
        gcLibImageLoad_CubeArray,
        gcLibImageStore_CubeArray,
    };

    gctSTRING ImageLib_cubeMapArray_img_access[] =
    {
        gcLibImageLoad_CubeArray_float_img_access,
        gcLibImageLoad_CubeArray_float_1_img_access,
        gcLibImageLoad_CubeArray_int_img_access,
        gcLibImageLoad_CubeArray_int_1_img_access,
        gcLibImageLoad_CubeArray_uint_img_access,
        gcLibImageLoad_CubeArray_uint_1_img_access,

        gcLibImageStore_CubeArray_float_img_access,
        gcLibImageStore_CubeArray_float_1_img_access,
        gcLibImageStore_CubeArray_int_img_access,
        gcLibImageStore_CubeArray_int_1_img_access,
        gcLibImageStore_CubeArray_uint_img_access,
        gcLibImageStore_CubeArray_uint_1_img_access,
    };

    gctSTRING TextureGatherLib_cubeMapArray_halti4[] =
    {
        gcLibTextureGather_Func_22,
        gcLibTextureGather_Func_23,
        gcLibTextureGather_Func_24,
        gcLibTextureGather_Func_25,
        gcLibTextureGather_Func_26,
        gcLibTextureGather_Func_27,
        gcLibTextureGather_Func_28,
    };

    /* MS shading related built-in functions. */
    gctSTRING MSShadingLib[] =
    {
        gcLibInterpolateCommon,
        gcLibInterpolateAtCentroid_float,
        gcLibInterpolateAtCentroid_vec2,
        gcLibInterpolateAtCentroid_vec3,
        gcLibInterpolateAtCentroid_vec4,

        gcLibInterpolateAtSample_float,
        gcLibInterpolateAtSample_vec2,
        gcLibInterpolateAtSample_vec3,
        gcLibInterpolateAtSample_vec4,

        gcLibInterpolateAtOffset_float,
        gcLibInterpolateAtOffset_vec2,
        gcLibInterpolateAtOffset_vec3,
        gcLibInterpolateAtOffset_vec4,
    };

    if (isSupportImgAddr && !isSupportImgInst &&
        (GetShaderType(Shader) == gcSHADER_TYPE_COMPUTE || GetShaderType(Shader) == gcSHADER_TYPE_CL))
    {
        isSupportImgInst = gcvTRUE;
    }

    if (isSupportImgAddr && !isSupportImgInst)
    {
        ImageLib[0] = gcLibImageAddr_halti4;
    }

    gcmASSERT((LibType == gcLIB_BUILTIN && GetShaderHasIntrinsicBuiltin(Shader)) ||
              (LibType == gcLIB_BLEND_EQUATION &&
               gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shader))));

    if (gcGLSLCompiler == gcvNULL)
    {
        return gcvSTATUS_LINK_LIB_ERROR;
    }

    if (LibType == gcLIB_BUILTIN)
    {
        if (isSupportImgInst)
        {
            if (gcBuiltinLibrary1 != gcvNULL)
            {
                *Binary = gcBuiltinLibrary1;
                return gcvSTATUS_OK;
            }
        }
        else
        {
            if (gcBuiltinLibrary0 != gcvNULL)
            {
                *Binary = gcBuiltinLibrary0;
                return gcvSTATUS_OK;
            }
        }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
        if (gcBlendEquationLibrary != gcvNULL)
        {
            * Binary = gcBlendEquationLibrary;
            return gcvSTATUS_OK;
        }
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, __BUILTIN_SHADER_LENGTH__, &pointer));
    sloBuiltinSource = pointer;

    /* add the extension source */
    length = gcoOS_StrLen(gcLibFunc_Extension, gcvNULL);
    gcoOS_StrCopySafe(sloBuiltinSource, length + 1, gcLibFunc_Extension);

    /* add the extension source */
    if (isSupportTexMSAA2DArray)
    {
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_TexMS2DArray);
    }

    if (isSupportCubeMapArray)
    {
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_CubeMapArray);
    }

    if (isSupportTextureBuffer)
    {
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_TextureBuffer);
    }

    if (isSupportMSShading)
    {
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_MSShading);
    }

    if (LibType == gcLIB_BUILTIN)
    {
        /* add the header source */
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_BuiltinHeader);

        if (isHalti4)
        {
            /* add the intrinsic builtin function source in gc7000*/
            stringNum = sizeof(BuiltinLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_hati4[i]);
            }
        }
        else
        {
            /* add the intrinsic builtin function source in gc3000/5000*/
            stringNum = sizeof(BuiltinLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib[i]);
            }
        }

        if (isHalti5 && isAppConformance())
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_halti5);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs_halti5);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs_halti5);
        }
        else if (isHalti2 && isAppConformance())
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_halti2);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs_halti2);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs_halti2);
        }
        else
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_Common);
            if (isHalti0)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_Funcs_halti0);
            }
            else
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_Funcs);
            }
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibACOS_Funcs);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs);
            gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs);
        }

        /* add common intrinsic builtin function source */
        gcmASSERT(BuiltinLib_Common[BUILTINLIB_MIX_IDX] == gcLib_2instMixFunc);
        if (isAppConformance())
        {
            BuiltinLib_Common[BUILTINLIB_MIX_IDX] = gcLib_3instMixFunc;
        }
        stringNum = sizeof(BuiltinLib_Common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, BuiltinLib_Common[i]);
        }

        if (isHalti5)
        {
            stringNum = sizeof(BuiltinLib_Reflect_halti5) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_Reflect_halti5[i]);
            }
        }
        else
        {
            stringNum = sizeof(BuiltinLib_Reflect) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_Reflect[i]);
            }
        }

        if (isSupportTextureGather)
        {
            stringNum = sizeof(TextureGatherLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib[i]);
            }
        }
        else
        {
            stringNum = sizeof(TextureGatherLib_2) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib_2[i]);
            }
        }

        /* add textureGatherOffset functions. */
        stringNum = sizeof(TextureGatherOffsetLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, TextureGatherOffsetLib[i]);
        }

        /* add textureGatherOffsets functions. */
        stringNum = sizeof(TextureGatherOffsetsLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, TextureGatherOffsetsLib[i]);
        }

        stringNum = sizeof(ImageLib_common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, ImageLib_common[i]);
        }
        /* hati4 support image instruction */
        if (isSupportImgInst)
        {
            stringNum = sizeof(ImageLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib_hati4[i]);
            }
        }
        else
        {
            stringNum = sizeof(ImageLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib[i]);
            }
        }

        /* texelFetch for MSAA. */
        if (isSupportTexelFetchForMSAA)
        {
            stringNum = sizeof(TexelFetchForMSAALib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TexelFetchForMSAALib[i]);
            }
        }
        else
        {
            stringNum = sizeof(TexelFetchForMSAALib_2) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TexelFetchForMSAALib_2[i]);
            }
        }

        /* MSAA Tex 2D Array */
        if (isSupportTexMSAA2DArray)
        {
            if (isSupportTexelFetchForMSAA)
            {
                stringNum = sizeof(TexMS2DArrayLib) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, TexMS2DArrayLib[i]);
                }
            }
            else
            {
                stringNum = sizeof(TexMS2DArrayLib_2) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, TexMS2DArrayLib_2[i]);
                }
            }
        }

        if (isSupportCubeMapArray)
        {
            stringNum = sizeof(ImageLib_cubeMapArray_general) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray_general[i]);
            }

            stringNum = sizeof(TextureGatherLib_cubeMapArray_halti4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib_cubeMapArray_halti4[i]);
            }

            if (isSupportImgInst)
            {
                stringNum = sizeof(ImageLib_cubeMapArray_img_access) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray_img_access[i]);
                }
            }
            else
            {
                stringNum = sizeof(ImageLib_cubeMapArray) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray[i]);
                }
            }
        }

        if (isSupportTextureBuffer)
        {
            stringNum = sizeof(TextureBuffer_general) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureBuffer_general[i]);
            }

            if (isSupportImgInst)
            {
                stringNum = sizeof(TextureBuffer_support_img_access) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, TextureBuffer_support_img_access[i]);
                }
            }
            else
            {
                stringNum = sizeof(TextureBuffer) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, TextureBuffer[i]);
                }
            }
        }

        if (isSupportMSShading)
        {
            stringNum = sizeof(MSShadingLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, MSShadingLib[i]);
            }
        }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
        /* add the header source */
        gcoOS_StrCatSafe(sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_BlendEquationHeader);

        if (isHalti4)
        {
            /* add the blend equation source that are not supported by HW in gc7000 */
            stringNum = sizeof(BlendEquationLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BlendEquationLib_hati4[i]);
            }
        }
        else
        {
            /* add the blend equation source that are not supported by HW in gc3000/5000*/
            stringNum = sizeof(BlendEquationLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BlendEquationLib[i]);
            }
        }
    }

    status = (*gcGLSLCompiler)(gcvNULL,
                             ShaderType,
                             gcoOS_StrLen(sloBuiltinSource, gcvNULL),
                             sloBuiltinSource,
                             Binary,
                             &log);

    if (status != gcvSTATUS_OK)
    {
        /* report error */
        gcoOS_Print("Compiler Error:\n%s\n", log);
        goto OnError;
    }
    if (gcSHADER_DumpCodeGenVerbose(*Binary))
    {
        gcOpt_Dump(gcvNULL, "Library Shader", gcvNULL, *Binary);
    }
    if (LibType == gcLIB_BUILTIN)
    {
        if (isSupportImgInst)
        {
            gcBuiltinLibrary1 = *Binary;
        }
        else
        {
            gcBuiltinLibrary0 = *Binary;
        }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
        gcBlendEquationLibrary = *Binary;
    }

OnError:
    if (sloBuiltinSource)
    {
        gcmOS_SAFE_FREE(gcvNULL, sloBuiltinSource);
    }

    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }

    return status;
}

gceSTATUS
gcSHADER_CompileCLBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS   status        = gcvSTATUS_OK;
    gctSTRING   builtinSource = gcvNULL;

    gctSIZE_T   length;
    gctPOINTER  pointer = gcvNULL;
    gctINT      i, stringNum = 0;
    gctSTRING   log    = gcvNULL;

    /* built-in function library */

    gctSTRING   builtinLib[] =
    {
        gcCLLibLongMADSAT_Funcs,
        gcCLLibLongNEXTAFTER_Funcs,
    };


    gcmASSERT((LibType == gcLIB_CL_BUILTIN && GetShaderHasIntrinsicBuiltin(Shader)));

    _gcmLockLoadLibrary(status);
    gcmONERROR(status);

    if (gcCLCompiler == gcvNULL)
    {
        return gcvSTATUS_LINK_LIB_ERROR;
    }

    if (LibType == gcLIB_CL_BUILTIN)
    {
        if (gcCLBuiltinLibrary != gcvNULL)
        {
            *Binary = gcCLBuiltinLibrary;
            return gcvSTATUS_OK;
        }
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL, __BUILTIN_SHADER_LENGTH__, &pointer));
    builtinSource = pointer;

    /* add the header source */
    length = gcoOS_StrLen(gcCLLibHeader, gcvNULL);
    gcoOS_StrCopySafe(builtinSource, length + 1, gcCLLibHeader);

    if (LibType == gcLIB_CL_BUILTIN)
    {
        /* add the intrinsic builtin function source */
        stringNum = sizeof(builtinLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(builtinSource,
                             __BUILTIN_SHADER_LENGTH__, builtinLib[i]);
        }
    }

    status = (*gcCLCompiler)(gcvNULL,
                             gcoOS_StrLen(builtinSource, gcvNULL),
                             builtinSource,
                             "",
                             Binary,
                             &log);
    if (gcmIS_ERROR(status))
    {
        /* report error */
        gcoOS_Print("Builtin library compile Error:\n%s\n", log);
        gcmONERROR(status);
    }

    if (gcSHADER_DumpCodeGenVerbose(*Binary))
    {
        gcOpt_Dump(gcvNULL, "Library Shader", gcvNULL, *Binary);
    }

    gcCLBuiltinLibrary = *Binary;

OnError:

    if (builtinSource)
    {
        gcmOS_SAFE_FREE(gcvNULL, builtinSource);
    }

    if (log)
    {
        gcmOS_SAFE_FREE(gcvNULL, log);
    }

    _gcmUnlockLoadLibrary(status);
    return status;
}
#endif /* DX_SHADER */

/* For a Call node which calls a function in Library, link the the callee
 * to the Shader, and NewFunction return linked function in Shader */
static gceSTATUS
_linkLibFunctionToShader(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gcSL_INSTRUCTION Call,
    OUT    gcFUNCTION *     NewFunction
    )
{
    gcFUNCTION  libFunction = gcvNULL;
    gcFUNCTION  function = gcvNULL;
    gceSTATUS   status = gcvSTATUS_OK;

    gcmONERROR(gcSHADER_GetFunctionByHeadIndex(Library, Call->tempIndex, &libFunction));

    /* check if the function is already linked to Shader */
    gcmASSERT((gctINT)libFunction->nameLength > 0);

    gcSHADER_GetFunctionByName(Shader, libFunction->name, &function);
    /* We need a link a unused function or a unlinked intrinsic function. */
    if ((function == gcvNULL) ||
        (function != gcvNULL && function->codeCount == 0 && IsFunctionIntrinsicsBuiltIn(function)))
    {
        /* not linked to Shader yet, link it */
        gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                 Library,
                                 libFunction->name,
                                 &function));
    }

OnError:
    *NewFunction = function;
    /* Return the status. */
    return status;
}

typedef struct _MappingInfo
{
    gcSHADER         Shader;
    gcLibraryList    *LibList;
    gctINT           TempOffset;
    gcFUNCTION       Function;

}
_MappingInfo;

/* check Library's temp register mapping table if *TempIndexPtr is
 * already mapped, if it is, change the temp index to mapped value,
 * otherwise if the temp index is in Function's temp register range,
 * then adjust the temp index by TempOffset and enter the mapping
 * to the table; otherwise, it must use some unseen variable (most
 * likely the global variable), mapping the global variable.
 */
static void
_fixTempIndexByMappingTable(
    IN     _MappingInfo *   MI,
    IN OUT gctUINT16 *      TempIndexPtr
    )
{
    gctINT libFunctionTempIndexStart = (gctINT)MI->Function->tempIndexStart - MI->TempOffset;

    gcmASSERT(*TempIndexPtr < MI->LibList->mappingTableEntries);
    if (MI->LibList->mappingTable[*TempIndexPtr] != -1)
    {
        /* already mapped, use the mapped temp index */
        *TempIndexPtr = (gctUINT16) MI->LibList->mappingTable[*TempIndexPtr];
        gcmASSERT(*TempIndexPtr < MI->Shader->_tempRegCount);
        return;
    }

    /* check if the *TempIndexPtr is in function's temp register range */
    if (*TempIndexPtr >= libFunctionTempIndexStart &&
        *TempIndexPtr < (libFunctionTempIndexStart + MI->Function->tempIndexCount))
    {
        gctINT newTempIndex = (gctINT)*TempIndexPtr + MI->TempOffset;
        gcmASSERT(newTempIndex > 0 && newTempIndex < (gctINT)MI->Shader->_tempRegCount);
        /* enter the mapping */
        MI->LibList->mappingTable[*TempIndexPtr] = newTempIndex;
        *TempIndexPtr = (gctUINT16)newTempIndex;
    }
    else
    {
        /* TODO: handling global variables */
        gcmASSERT(gcvFALSE);
    }
    return;
}

static void
_MapTempRegister(
    IN OUT gcLibraryList *  LibList,
    IN     gctINT           TempIndex,
    IN     gctINT           MappedTempRegIndex
    )
{
    gcmASSERT(TempIndex >= 0 &&
              TempIndex < (gctINT)LibList->mappingTableEntries);
    LibList->mappingTable[TempIndex] = MappedTempRegIndex;
    return;
}

static gcLibraryList *
_addToLibraryList(
    IN OUT gcSHADER Shader,
    IN     gcSHADER LibraryShader
    )
{
    gcLibraryList * libList       = Shader->libraryList;
    gctPOINTER      pointer       = gcvNULL;

    /* check if the LibraryShader is already in Shader's libList */
    while (libList != gcvNULL && libList->lib != LibraryShader)
        libList = libList->next;

    if (libList == gcvNULL)
    {
        gctINT i;

        /* create a new node if LibraryShader is not in the list */
        gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, gcmSIZEOF(gcLibraryList), &pointer));
        if (pointer == gcvNULL) return pointer;
        libList = (gcLibraryList *)pointer;

        libList->lib   = LibraryShader;
        libList->next  = Shader->libraryList;
        Shader->libraryList = libList;

        gcmVERIFY_OK(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(gctINT) * LibraryShader->_tempRegCount,
                                  &pointer));
        libList->mappingTableEntries = LibraryShader->_tempRegCount;
        libList->mappingTable = (gctINT *) pointer;

        for (i=0; i < (gctINT)libList->mappingTableEntries; i++)
        {
            libList->mappingTable[i] = -1;
        }
    }

    return libList;
}

/* initialize Shader's temp register mapping table for the LinkedToShader */
gcLibraryList *
gcSHADER_InitMappingTable(
    IN OUT gcSHADER LibShader,
    IN     gcSHADER LinkedToShader
    )
{
    return  _addToLibraryList(LinkedToShader, LibShader);
}

/* after the Shader linked with libraries, reset the temp register mapping
 * mapping table in all libraries the Shader links to, so the libraries
 * can be linked with other shaders
 */
gceSTATUS
gcSHADER_ResetLibraryMappingTable(
    IN OUT gcSHADER Shader
    )
{
    gceSTATUS status         = gcvSTATUS_OK;
    gcLibraryList * libList  = Shader->libraryList;

    while (libList != gcvNULL)
    {
        gcLibraryList * next = libList->next;

        if(libList->mappingTable)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, libList->mappingTable));
        }
        /* free the libList node */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, libList));

        /* go to next library */
        libList = next;
    }

    /* reset the library list */
    Shader->libraryList = gcvNULL;

    return status;
}

static gceSTATUS
_CreateSamplerSizeArgument(
    IN gcSHADER             Shader,
    IN gcFUNCTION           LibFunction,
    IN gcFUNCTION           Function,
    IN gctBOOL              CreateLod
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 tempIndex1 = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X3);
    gctUINT32 tempIndex2 = 0;
    gctUINT32 i, j;
    gctUINT32 argumentCount = 0;
    gctINT16 variableIndex1 = -1, variableIndex2 = -1;
    gctPOINTER pointer = gcvNULL;
    gcsFUNCTION_ARGUMENT_PTR newArguments = gcvNULL, oldArguments = Function->arguments;

    if (LibFunction == gcvNULL)
    {
        argumentCount = Function->argumentCount + 1;
        if (CreateLod)
            argumentCount++;
    }
    else
    {
        argumentCount = LibFunction->argumentCount;
    }

    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                        "_input_levelBaseSize",
                                        gcSHADER_FLOAT_X3,
                                        0,
                                        gcvNULL,
                                        (gctUINT16)tempIndex1,
                                        gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                        gcSHADER_PRECISION_MEDIUM,
                                        0,
                                        -1,
                                        -1,
                                        &variableIndex1));

    if (CreateLod)
    {
        tempIndex2 = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X3);
        gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                            "_input_lodMinMax",
                                            gcSHADER_FLOAT_X3,
                                            0,
                                            gcvNULL,
                                            (gctUINT16)tempIndex2,
                                            gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                            gcSHADER_PRECISION_MEDIUM,
                                            0,
                                            -1,
                                            -1,
                                            &variableIndex2));
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                                gcmSIZEOF(gcsFUNCTION_ARGUMENT) * argumentCount,
                                &pointer));
    gcoOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_ARGUMENT) * argumentCount);
    newArguments = pointer;

    /* Copy the sampler index. */
    newArguments[0].enable = oldArguments[0].enable;
    newArguments[0].index = oldArguments[0].index;
    newArguments[0].qualifier = oldArguments[0].qualifier;
    newArguments[0].precision = oldArguments[0].precision;
    newArguments[0].variableIndex = oldArguments[0].variableIndex;

    /* Insert the new arguments. */
    newArguments[1].enable = gcSL_ENABLE_XYZ;
    newArguments[1].index = (gctUINT16)tempIndex1;
    newArguments[1].qualifier = gcvFUNCTION_INPUT;
    newArguments[1].precision = gcSHADER_PRECISION_MEDIUM;
    newArguments[1].variableIndex = (gctUINT16)variableIndex1;

    if (CreateLod)
    {
        newArguments[2].enable = gcSL_ENABLE_XYZ;
        newArguments[2].index = (gctUINT16)tempIndex2;
        newArguments[2].qualifier = gcvFUNCTION_INPUT;
        newArguments[2].precision = gcSHADER_PRECISION_MEDIUM;
        newArguments[2].variableIndex = (gctUINT16)variableIndex2;
    }

    /* Copy the left arguments. */
    for (i = 1, j = CreateLod ? 3 : 2; i < Function->argumentCount; i++, j++)
    {
        newArguments[j].enable = oldArguments[i].enable;
        newArguments[j].index = oldArguments[i].index;
        newArguments[j].qualifier = oldArguments[i].qualifier;
        newArguments[j].precision = oldArguments[i].precision;
        newArguments[j].variableIndex = oldArguments[i].variableIndex;
    }

    /* Free the old arguments. */
    gcmOS_SAFE_FREE(gcvNULL, oldArguments);

    /* Set the new arguments. */
    Function->argumentCount = argumentCount;
    Function->argumentArrayCount = Function->argumentCount;
    Function->arguments = newArguments;

OnError:
    return status;
}

static gceSTATUS
_CreateInterpolationArgument(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcFUNCTION           LibFunction,
    IN gcFUNCTION           Function
    )
{
    gceSTATUS                status = gcvSTATUS_OK;
    gctUINT32                origArgumentCount;
    /* New function arguments temp register. */
    gctUINT32                interpolateTypeIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X1);
    gctUINT32                positionIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_FLOAT_X4);
    gctUINT32                multiSampleBufferIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_BOOLEAN_X1);
    gctUINT32                underSampleShadingIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_BOOLEAN_X1);
    gctUINT32                sampleIdIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X1);
    gctUINT32                sampleMaskIndex = gcSHADER_NewTempRegs(Shader, 1, gcSHADER_INTEGER_X1);
    gctUINT32                sampleLocationIndex = gcSHADER_NewTempRegs(Shader, 4, gcSHADER_FLOAT_X4);
    /* New function argments variable index. */
    gctINT16                 interpolateTypeVariableIndex = -1;
    gctINT16                 positionVariableIndex = -1;
    gctINT16                 multiSampleBufferVariableIndex = -1;
    gctINT16                 underSampleShadingVariableIndex = -1;
    gctINT16                 sampleIdVariableIndex = -1;
    gctINT16                 sampleMaskVariableIndex = -1;
    gctINT16                 sampleLocationVariableIndex = -1;

    gcsFUNCTION_ARGUMENT_PTR newArguments = gcvNULL, oldArguments = Function->arguments;
    gcVARIABLE               interpolateTypeVariable = gcvNULL;
    gcVARIABLE               positionVariable = gcvNULL;
    gcVARIABLE               multiSampleBufferVariable = gcvNULL;
    gcVARIABLE               underSampleShadingVariable = gcvNULL;
    gcVARIABLE               sampleIdVariable = gcvNULL;
    gcVARIABLE               sampleMaskVariable = gcvNULL;
    gcVARIABLE               sampleLocationVariable = gcvNULL;
    gctPOINTER               pointer = gcvNULL;
    gctINT                   i, currentArgumentIndex = 0;

    if (IsIntrinsicsKindInterpolateAtCentroid(Function->intrinsicsKind))
    {
        origArgumentCount = 1;
    }
    else if (IsIntrinsicsKindInterpolateAtSample(Function->intrinsicsKind))
    {
        origArgumentCount = 2;
    }
    else
    {
        origArgumentCount = 2;
    }

    /* Create agument "interpolateType". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &interpolateTypeVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      interpolateTypeVariable->name,
                                      gcSHADER_INTEGER_X1,
                                      0,
                                      gcvNULL,
                                      (gctUINT16)interpolateTypeIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      interpolateTypeVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &interpolateTypeVariableIndex));
    origArgumentCount++;

    /* Create argument "position". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &positionVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      positionVariable->name,
                                      gcSHADER_FLOAT_X4,
                                      0,
                                      gcvNULL,
                                      (gctUINT16)positionIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      positionVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &positionVariableIndex));
    origArgumentCount++;

    /* Create argument "multiSampleBuffer". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &multiSampleBufferVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      multiSampleBufferVariable->name,
                                      gcSHADER_BOOLEAN_X1,
                                      0,
                                      gcvNULL,
                                      (gctUINT16)multiSampleBufferIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      multiSampleBufferVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &multiSampleBufferVariableIndex));
    origArgumentCount++;

    /* Create argument "underSampleShading". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &underSampleShadingVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      underSampleShadingVariable->name,
                                      gcSHADER_BOOLEAN_X1,
                                      0,
                                      gcvNULL,
                                      (gctUINT16)underSampleShadingIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      underSampleShadingVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &underSampleShadingVariableIndex));
    origArgumentCount++;

    /* Create argument "sampleId". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &sampleIdVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      sampleIdVariable->name,
                                      gcSHADER_INTEGER_X1,
                                      0,
                                      gcvNULL,
                                      (gctUINT16)sampleIdIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      sampleIdVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &sampleIdVariableIndex));
    origArgumentCount++;

    /* Create argument "sampleMaskIn". */
    if (IsIntrinsicsKindInterpolateAtCentroid(Function->intrinsicsKind))
    {
        gcmONERROR(gcSHADER_GetVariable(Library,
                                        LibFunction->arguments[origArgumentCount].variableIndex,
                                        &sampleMaskVariable));
        gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                          sampleMaskVariable->name,
                                          gcSHADER_INTEGER_X1,
                                          0,
                                          gcvNULL,
                                          (gctUINT16)sampleMaskIndex,
                                          gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                          sampleMaskVariable->precision,
                                          0,
                                          -1,
                                          -1,
                                          &sampleMaskVariableIndex));
        origArgumentCount++;
    }

    /* Create argument "sampleLocation". */
    gcmONERROR(gcSHADER_GetVariable(Library,
                                    LibFunction->arguments[origArgumentCount].variableIndex,
                                    &sampleLocationVariable));
    gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                      sampleLocationVariable->name,
                                      gcSHADER_FLOAT_X4,
                                      1,
                                      sampleLocationVariable->arrayLengthList,
                                      (gctUINT16)sampleLocationIndex,
                                      gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                      sampleLocationVariable->precision,
                                      0,
                                      -1,
                                      -1,
                                      &sampleLocationVariableIndex));

    /* Copy arguments. */
    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gcmSIZEOF(gcsFUNCTION_ARGUMENT) * LibFunction->argumentCount,
                              &pointer));
    gcoOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_ARGUMENT) * LibFunction->argumentCount);
    newArguments = pointer;

    /* Copy old input arguments. */
    for (i = 0; i < (gctINT)(Function->argumentCount - 1); i++)
    {
        newArguments[currentArgumentIndex].enable         = oldArguments[i].enable;
        newArguments[currentArgumentIndex].index          = oldArguments[i].index;
        newArguments[currentArgumentIndex].qualifier      = oldArguments[i].qualifier;
        newArguments[currentArgumentIndex].precision      = oldArguments[i].precision;
        newArguments[currentArgumentIndex].variableIndex  = oldArguments[i].variableIndex;
        currentArgumentIndex++;
    }

    /* Insert argument "interpolateType". */
    newArguments[currentArgumentIndex].enable             = gcSL_ENABLE_X;
    newArguments[currentArgumentIndex].index              = (gctUINT16)interpolateTypeIndex;
    newArguments[currentArgumentIndex].qualifier          = gcvFUNCTION_INPUT;
    newArguments[currentArgumentIndex].precision          = interpolateTypeVariable->precision;
    newArguments[currentArgumentIndex].variableIndex      = (gctUINT16)interpolateTypeVariableIndex;
    currentArgumentIndex++;

    /* Insert argument "position". */
    newArguments[currentArgumentIndex].enable             = gcSL_ENABLE_XYZW;
    newArguments[currentArgumentIndex].index              = (gctUINT16)positionIndex;
    newArguments[currentArgumentIndex].qualifier          = gcvFUNCTION_INPUT;
    newArguments[currentArgumentIndex].precision          = positionVariable->precision;
    newArguments[currentArgumentIndex].variableIndex      = (gctUINT16)positionVariableIndex;
    currentArgumentIndex++;

    /* Insert argument "multiSampleBuffer". */
    newArguments[currentArgumentIndex].enable             = gcSL_ENABLE_X;
    newArguments[currentArgumentIndex].index              = (gctUINT16)multiSampleBufferIndex;
    newArguments[currentArgumentIndex].qualifier          = gcvFUNCTION_INPUT;
    newArguments[currentArgumentIndex].precision          = multiSampleBufferVariable->precision;
    newArguments[currentArgumentIndex].variableIndex      = (gctUINT16)multiSampleBufferVariableIndex;
    currentArgumentIndex++;

    /* Insert argument "underSampleShading". */
    newArguments[currentArgumentIndex].enable             = gcSL_ENABLE_X;
    newArguments[currentArgumentIndex].index              = (gctUINT16)underSampleShadingIndex;
    newArguments[currentArgumentIndex].qualifier          = gcvFUNCTION_INPUT;
    newArguments[currentArgumentIndex].precision          = underSampleShadingVariable->precision;
    newArguments[currentArgumentIndex].variableIndex      = (gctUINT16)underSampleShadingVariableIndex;
    currentArgumentIndex++;

    /* Insert argument "sampleId". */
    newArguments[currentArgumentIndex].enable             = gcSL_ENABLE_X;
    newArguments[currentArgumentIndex].index              = (gctUINT16)sampleIdIndex;
    newArguments[currentArgumentIndex].qualifier          = gcvFUNCTION_INPUT;
    newArguments[currentArgumentIndex].precision          = sampleIdVariable->precision;
    newArguments[currentArgumentIndex].variableIndex      = (gctUINT16)sampleIdVariableIndex;
    currentArgumentIndex++;

    /* Insert argument "sampleMaskIn". */
    if (IsIntrinsicsKindInterpolateAtCentroid(Function->intrinsicsKind))
    {
        newArguments[currentArgumentIndex].enable         = gcSL_ENABLE_X;
        newArguments[currentArgumentIndex].index          = (gctUINT16)sampleMaskIndex;
        newArguments[currentArgumentIndex].qualifier      = gcvFUNCTION_INPUT;
        newArguments[currentArgumentIndex].precision      = sampleMaskVariable->precision;
        newArguments[currentArgumentIndex].variableIndex  = (gctUINT16)sampleMaskVariableIndex;
        currentArgumentIndex++;
    }

    /* Insert argument "sampleLocation". */
    for (i = 0; i < 4; i++)
    {
        newArguments[currentArgumentIndex].enable         = gcSL_ENABLE_XYZW;
        newArguments[currentArgumentIndex].index          = (gctUINT16)(sampleLocationIndex + i);
        newArguments[currentArgumentIndex].qualifier      = gcvFUNCTION_INPUT;
        newArguments[currentArgumentIndex].precision      = sampleLocationVariable->precision;
        newArguments[currentArgumentIndex].variableIndex  = (gctUINT16)sampleLocationVariableIndex;
        currentArgumentIndex++;
    }

    /* Copy old output argument. */
    newArguments[currentArgumentIndex].enable             = oldArguments[Function->argumentCount - 1].enable;
    newArguments[currentArgumentIndex].index              = oldArguments[Function->argumentCount - 1].index;
    newArguments[currentArgumentIndex].qualifier          = oldArguments[Function->argumentCount - 1].qualifier;
    newArguments[currentArgumentIndex].precision          = oldArguments[Function->argumentCount - 1].precision;
    newArguments[currentArgumentIndex].variableIndex      = oldArguments[Function->argumentCount - 1].variableIndex;

    /* Free the old arguments. */
    gcmOS_SAFE_FREE(gcvNULL, oldArguments);

    /* Set the new arguments. */
    Function->argumentCount = LibFunction->argumentCount;
    Function->argumentArrayCount = Function->argumentCount;
    Function->arguments = newArguments;
OnError:
    return status;
}

static gctBOOL
_inputOutputArgument(
    IN gcFUNCTION Function,
    IN gctUINT    Index,
    IN gctBOOL    IsInput,
    OUT gctUINT   *argIndex
    )
{
    gctUINT     i;

    for (i = 0; i < Function->argumentCount; i++)
    {
        gcsFUNCTION_ARGUMENT_PTR argument = Function->arguments + i;

        if (IsInput)
        {
            if (argument->qualifier == gcvFUNCTION_INPUT ||
                argument->qualifier == gcvFUNCTION_INOUT)
            {
                if (argument->index == Index)
                {
                    if (argIndex)
                        *argIndex = i;
                    return gcvTRUE;
                }
            }
        }
        else
        {
            if (argument->qualifier == gcvFUNCTION_OUTPUT ||
                argument->qualifier == gcvFUNCTION_INOUT)
            {
                if (argument->index == Index)
                {
                    if (argIndex)
                        *argIndex = i;
                    return gcvTRUE;
                }
            }
        }
    }

    return gcvFALSE;
}

static gceSTATUS
_findFunctionByArgumentIndex(
    IN gcSHADER     Shader,
    IN gctUINT      Index,
    IN gcFUNCTION * Function
    )
{
    gctUINT32 i;
    gcFUNCTION function = gcvNULL;

    for (i = 0; i < Shader->functionCount; i++)
    {
        function = Shader->functions[i];
        if (_inputOutputArgument(function, Index, gcvTRUE, gcvNULL))
        {
            break;
        }
        if (_inputOutputArgument(function, Index, gcvFALSE, gcvNULL))
        {
            break;
        }
    }

    if (i < Shader->functionCount && Function)
        *Function = function;

    return gcvSTATUS_OK;
}

static gceSTATUS
_InsertAssignmentForSamplerSize(
    IN gcSHADER             Shader,
    IN gcFUNCTION           Function,
    IN gctBOOL              CreateLod,
    IN gctBOOL              BuiltIn,
    IN gceINTRINSICS_KIND   IntrinsicsKind,
    IN gctINT16 *           ArgIndex0,
    IN gctINT16 *           ArgIndex1
    );

static gceSTATUS
_InsertAssignmentForAllFuncStack(
    IN gcSHADER             Shader,
    IN gcFUNCTION           Function,
    IN gctBOOL              CreateLod,
    IN gceINTRINSICS_KIND   IntrinsicsKind,
    IN gctINT16 *           ArgIndex0,
    IN gctINT16 *           ArgIndex1
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT16 argIndex0 = 0, argIndex1 = 0;

    gcmONERROR(_CreateSamplerSizeArgument(Shader, gcvNULL, Function, CreateLod));

    gcmONERROR(_InsertAssignmentForSamplerSize(Shader,
                                               Function,
                                               CreateLod,
                                               gcvFALSE,
                                               IntrinsicsKind,
                                               &argIndex0,
                                               &argIndex1));

    if (ArgIndex0)
    {
        *ArgIndex0 = argIndex0;
    }

    if (ArgIndex1)
    {
        *ArgIndex1 = argIndex1;
    }

OnError:
    return status;
}

static gceSTATUS
_InsertAssignmentForSamplerSize(
    IN gcSHADER             Shader,
    IN gcFUNCTION           Function,
    IN gctBOOL              CreateLod,
    IN gctBOOL              BuiltIn,
    IN gceINTRINSICS_KIND   IntrinsicsKind,
    IN gctINT16 *           ArgIndex0,
    IN gctINT16 *           ArgIndex1
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 codeCount = Shader->codeCount, createCodeCount = CreateLod ? 2 : 1;
    gcSL_INSTRUCTION code, insertCode1, insertCode2;
    gctSIZE_T len;
    gctUINT offset;
    gcUNIFORM uniform, firstChild, lodMinMaxUniform, levelBaseSizeUniform;
    gctSTRING symbol;
    gctINT16 lodMinMaxIndex, levelBaseSizeIndex;
    gctINT16 argumentIndex0 = 0, argumentIndex1 = 0;
    gctINT32 i, j;
    gctBOOL fromUniform = gcvFALSE;
    gctPOINTER pointer = gcvNULL;

    for (i = 0; i < (gctINT32)codeCount; i++)
    {
        uniform = gcvNULL;
        lodMinMaxIndex = levelBaseSizeIndex = -1;

        code = Shader->code + i;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL ||
            (BuiltIn && (code->tempIndex != (gctUINT16)Function->label)) ||
            (!BuiltIn && (code->tempIndex != Function->codeStart)))
        {
            continue;
        }

        for (j = i; j >= 0; j--)
        {
            code = Shader->code + j;
            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_GET_SAMPLER_IDX ||
                gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_SAMPLER_ASSIGN)
                break;
        }

        /* This function is called by a user-defined function. */
        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_SAMPLER_ASSIGN)
        {
            gcFUNCTION function = gcvNULL;

            gcmONERROR(_findFunctionByArgumentIndex(Shader, (gctUINT)code->source0Index, &function));
            gcmASSERT(function != gcvNULL);

            gcmONERROR(_InsertAssignmentForAllFuncStack(Shader,
                                                        function,
                                                        CreateLod,
                                                        IntrinsicsKind,
                                                        &argumentIndex0,
                                                        &argumentIndex1));
        }
        /* This function is called by main function. */
        else
        {
            fromUniform = gcvTRUE;
            gcmONERROR(gcSHADER_GetUniform(Shader,
                                           gcmSL_INDEX_GET(code->source0Index, Index),
                                           &uniform));

            gcmASSERT(uniform);

            if (IsIntrinsicsKindTextureGather(IntrinsicsKind) ||
                IsIntrinsicsKindTextureGatherOffset(IntrinsicsKind))
            {
                SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_AS_TEXGATHER_SAMPLER);
            }

            if (IsIntrinsicsKindTextureGatherOffsets(IntrinsicsKind))
            {
                SetUniformFlag(uniform, gcvUNIFORM_FLAG_USED_AS_TEXGATHEROFFSETS_SAMPLER);
            }

            if (uniform->firstChild != -1)
            {
                gcmONERROR(gcSHADER_GetUniform(Shader, (gctUINT)uniform->firstChild, &firstChild));

                if (isUniformLodMinMax(firstChild))
                {
                    lodMinMaxIndex = firstChild->index;
                    if (firstChild->nextSibling != -1)
                    {
                        levelBaseSizeIndex = firstChild->nextSibling;
                    }
                }
                else
                {
                    gcmASSERT(isUniformLevelBaseSize(firstChild));
                    levelBaseSizeIndex = firstChild->index;
                    if (firstChild->nextSibling != -1)
                    {
                        lodMinMaxIndex = firstChild->nextSibling;
                    }
                }
            }

            /* Create levelBaseSize if needed. */
            if (levelBaseSizeIndex == -1)
            {
                len = uniform->nameLength + 16;
                gcmONERROR(gcoOS_Allocate(gcvNULL, len, &pointer));
                symbol = pointer;
                offset = 0;
                gcmONERROR(gcoOS_PrintStrSafe(symbol,
                                              len,
                                              &offset,
                                              "#%s$LevelBaseSize",
                                              uniform->name));

                gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                  symbol,
                                                  gcSHADER_INTEGER_X3,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  -1,
                                                  -1,
                                                  -1,
                                                  0,
                                                  gcvNULL,
                                                  gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE,
                                                  0,
                                                  uniform->index,
                                                  uniform->firstChild != -1 ? uniform->firstChild : -1,
                                                  gcIMAGE_FORMAT_DEFAULT,
                                                  &levelBaseSizeIndex,
                                                  &levelBaseSizeUniform));
                gcmOS_SAFE_FREE(gcvNULL, symbol);
            }

            /* Create lodMinMax if needed. */
            if (lodMinMaxIndex == -1)
            {
                len = uniform->nameLength + 12;
                gcmONERROR(gcoOS_Allocate(gcvNULL, len, &pointer));
                symbol = pointer;
                offset = 0;
                gcmONERROR(gcoOS_PrintStrSafe(symbol,
                                              len,
                                              &offset,
                                              "#%s$LodMinMax",
                                              uniform->name));

                gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                                  symbol,
                                                  gcSHADER_INTEGER_X3,
                                                  gcSHADER_PRECISION_MEDIUM,
                                                  -1,
                                                  -1,
                                                  -1,
                                                  0,
                                                  gcvNULL,
                                                  gcSHADER_VAR_CATEGORY_LOD_MIN_MAX,
                                                  0,
                                                  uniform->index,
                                                  uniform->firstChild != -1 ? uniform->firstChild : -1,
                                                  gcIMAGE_FORMAT_DEFAULT,
                                                  &lodMinMaxIndex,
                                                  &lodMinMaxUniform));
                gcmOS_SAFE_FREE(gcvNULL, symbol);
            }

            argumentIndex0 = levelBaseSizeIndex;
            argumentIndex1 = lodMinMaxIndex;
        }

        /* Insert two argument assignments. */
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, j + 1, createCodeCount));

        insertCode1 = &Shader->code[j + 1];
        insertCode1->opcode = gcSL_MOV;
        insertCode1->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                        | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZ)
                        | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                        | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_MEDIUM);
        insertCode1->tempIndex = Function->arguments[1].index;

        insertCode1->source0 = gcmSL_SOURCE_SET(0, Type, fromUniform ? gcSL_UNIFORM : gcSL_TEMP)
                            | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZZ)
                            | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_MEDIUM);
        insertCode1->source0Index = argumentIndex0;
        argumentIndex0 = (gctINT16)insertCode1->tempIndex;

        if (CreateLod)
        {
            insertCode2 = &Shader->code[j + 2];
            insertCode2->opcode = gcSL_MOV;
            insertCode2->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZ)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_MEDIUM);
            insertCode2->tempIndex = Function->arguments[2].index;

            insertCode2->source0 = gcmSL_SOURCE_SET(0, Type, fromUniform ? gcSL_UNIFORM : gcSL_TEMP)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZZ)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_MEDIUM);
            insertCode2->source0Index = argumentIndex1;
            argumentIndex1 = (gctINT16)insertCode2->tempIndex;
        }

        codeCount += createCodeCount;
        i += createCodeCount;
    }

    if (ArgIndex0)
    {
        *ArgIndex0 = argumentIndex0;
    }

    if (ArgIndex1)
    {
        *ArgIndex1 = argumentIndex1;
    }

OnError:
    return status;
}

static gceSTATUS
_InsertAssignmentForInterpolation(
    IN gcSHADER             Shader,
    IN gcFUNCTION           Function,
    IN gceINTRINSICS_KIND   IntrinsicsKind
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gcATTRIBUTE             sampleMaskIn = gcvNULL, position = gcvNULL, sampleId = gcvNULL, samplePosition = gcvNULL;
    gcATTRIBUTE             attribute = gcvNULL;
    gcUNIFORM               sampleLocationUniform = gcvNULL;
    gcUNIFORM               multiSampleBufferUniform = gcvNULL;
    gctUINT32               origArgumentCount, codeCount = Shader->codeCount, currentArg;
    gctUINT32               i, j, createCodeCount, currentPos;
    gcSL_INSTRUCTION        code, insertCode0;
    union
    {
        gctUINT16 hex[2];
        gctUINT32 hex32;
    }
    interpolateTypeValue, underSampleShadingValue;

    /* Check if this PS is running under sample shading. */
    gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                           gcvNULL,
                                           (gctUINT32)gcSL_SAMPLE_ID,
                                           &sampleId));
    gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                           gcvNULL,
                                           (gctUINT32)gcSL_SAMPLE_POSITION,
                                           &samplePosition));

    if ((sampleId != gcvNULL && !gcmATTRIBUTE_isCompilerGen(sampleId)) ||
        (samplePosition != gcvNULL && !gcmATTRIBUTE_isCompilerGen(samplePosition)))
    {
        underSampleShadingValue.hex32 = 1;
    }
    else
    {
        underSampleShadingValue.hex32 = 0;
    }

    /* Get attribute gl_FragCoord. */
    gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                           gcvNULL,
                                           (gctUINT32)gcSL_POSITION,
                                           &position));
    if (position == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddAttributeWithLocation(Shader,
                                                     "#Position",
                                                     gcSHADER_FLOAT_X4,
                                                     gcSHADER_PRECISION_HIGH,
                                                     1,
                                                     0,
                                                     gcvFALSE,
                                                     gcSHADER_SHADER_DEFAULT,
                                                     -1,
                                                     -1,
                                                     gcvFALSE,
                                                     gcvFALSE,
                                                     &position));

        gcmATTRIBUTE_SetIsStaticallyUsed(position, gcvTRUE);
    }

    /* Get uniform "multiSampleBuffer". */
    gcmONERROR(gcSHADER_GetUniformByName(Shader,
                                         "#EnableMultiSampleBuffer",
                                         (gctUINT32)gcoOS_StrLen("#EnableMultiSampleBuffer", gcvNULL),
                                         &multiSampleBufferUniform
                                         ));

    if (multiSampleBufferUniform == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader,
                                        "#EnableMultiSampleBuffer",
                                        gcSHADER_BOOLEAN_X1,
                                        1,
                                        gcSHADER_PRECISION_HIGH,
                                        &multiSampleBufferUniform));

        SetUniformFlag(multiSampleBufferUniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SetUniformFlag(multiSampleBufferUniform, gcvUNIFORM_FLAG_STATICALLY_USED);
        SetUniformCategory(multiSampleBufferUniform, gcSHADER_VAR_CATEGORY_ENABLE_MULTISAMPLE_BUFFERS);
    }

    /* Get attribute gl_SampleId. */
    if (sampleId == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddAttributeWithLocation(Shader,
                                                     "#SampleID",
                                                     gcSHADER_INTEGER_X1,
                                                     gcSHADER_PRECISION_LOW,
                                                     1,
                                                     0,
                                                     gcvFALSE,
                                                     gcSHADER_SHADER_DEFAULT,
                                                     -1,
                                                     -1,
                                                     gcvFALSE,
                                                     gcvFALSE,
                                                     &sampleId));

        gcmATTRIBUTE_SetIsCompilerGen(sampleId, gcvTRUE);
        gcmATTRIBUTE_SetIsStaticallyUsed(sampleId, gcvTRUE);
    }

    /* Get attribute gl_SampleMaskIn[(MaxSamples + 31) /32]. */
    if (IsIntrinsicsKindInterpolateAtCentroid(IntrinsicsKind))
    {
        gcmONERROR(gcSHADER_GetAttributeByName(Shader,
                                               gcvNULL,
                                               (gctUINT32)gcSL_SAMPLE_MASK_IN,
                                               &sampleMaskIn));
        if (sampleMaskIn == gcvNULL)
        {
            gcmONERROR(gcSHADER_AddAttributeWithLocation(Shader,
                                                         "#SampleMaskIn",
                                                         gcSHADER_INTEGER_X1,
                                                         gcSHADER_PRECISION_HIGH,
                                                         (GetGLMaxSamples() + 31) / 32,
                                                         1,
                                                         gcvFALSE,
                                                         gcSHADER_SHADER_DEFAULT,
                                                         -1,
                                                         -1,
                                                         gcvFALSE,
                                                         gcvFALSE,
                                                         &sampleMaskIn));

            gcmATTRIBUTE_SetIsStaticallyUsed(sampleMaskIn, gcvTRUE);
        }
    }

    /* Get private uniform #SampleLocation[4]. */
    gcmONERROR(gcSHADER_GetUniformByName(Shader,
                                         "#SampleLocation",
                                         (gctUINT32)gcoOS_StrLen("#SampleLocation", gcvNULL),
                                         &sampleLocationUniform
                                         ));

    if (sampleLocationUniform == gcvNULL)
    {
        gcmONERROR(gcSHADER_AddUniform(Shader,
                                       "#SampleLocation",
                                       gcSHADER_FLOAT_X4,
                                       4,
                                       gcSHADER_PRECISION_HIGH,
                                       &sampleLocationUniform));

        SetUniformFlag(sampleLocationUniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SetUniformFlag(sampleLocationUniform, gcvUNIFORM_FLAG_STATICALLY_USED);
        SetUniformCategory(sampleLocationUniform, gcSHADER_VAR_CATEGORY_SAMPLE_LOCATION);
    }

    if (IsIntrinsicsKindInterpolateAtCentroid(IntrinsicsKind))
    {
        origArgumentCount = 1;
        createCodeCount = 10;
    }
    else if (IsIntrinsicsKindInterpolateAtSample(IntrinsicsKind))
    {
        origArgumentCount = 2;
        createCodeCount = 9;
    }
    else
    {
        origArgumentCount = 2;
        createCodeCount = 9;
    }

    for (i = 0; i < codeCount; i++)
    {
        code = Shader->code + i;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL ||
            code->tempIndex != (gctUINT16)Function->label)
        {
            continue;
        }

        gcmASSERT(i >= origArgumentCount);
        code = code - origArgumentCount;
        gcmASSERT(gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV &&
                  gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE);
        gcmONERROR(gcSHADER_GetAttribute(Shader,
                                         (gctUINT)gcmSL_INDEX_GET(code->source0Index, Index),
                                         &attribute));
        gcmASSERT(attribute);

        /* Insert new arguments assignment codes. */
        gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, i, createCodeCount));
        currentPos = i;
        currentArg = origArgumentCount;

        /* Insert interpolateType assignment. */
        if (gcmATTRIBUTE_isCentroid(attribute))
        {
            interpolateTypeValue.hex32 = 0;
        }
        else if (gcmATTRIBUTE_isSample(attribute))
        {
            interpolateTypeValue.hex32 = 1;
        }
        else
        {
            interpolateTypeValue.hex32 = 2;
        }
        insertCode0 = &Shader->code[currentPos];
        insertCode0->opcode = gcSL_MOV;
        insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->tempIndex = Function->arguments[currentArg].index;

        insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->source0Index = interpolateTypeValue.hex[0];
        insertCode0->source0Indexed = interpolateTypeValue.hex[1];
        currentPos++;
        currentArg++;

        /* Insert position assignment. */
        insertCode0 = &Shader->code[currentPos];
        insertCode0->opcode = gcSL_MOV;
        insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->tempIndex = Function->arguments[currentArg].index;

        insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->source0Index = position->index;
        currentPos++;
        currentArg++;

        /* Insert multiSampleBuffer assignment. */
        insertCode0 = &Shader->code[currentPos];
        insertCode0->opcode = gcSL_MOV;
        insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->tempIndex = Function->arguments[currentArg].index;

        insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->source0Index = gcmSL_INDEX_SET(0, Index, multiSampleBufferUniform->index);
        currentPos++;
        currentArg++;

        /* Insert underSampleShading assignment. */
        insertCode0 = &Shader->code[currentPos];
        insertCode0->opcode = gcSL_MOV;
        insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_BOOLEAN)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->tempIndex = Function->arguments[currentArg].index;

        insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_CONSTANT)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_BOOLEAN)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->source0Index = underSampleShadingValue.hex[1];
        insertCode0->source0Indexed = underSampleShadingValue.hex[0];
        currentPos++;
        currentArg++;

        /* Insert sampleId assignment. */
        insertCode0 = &Shader->code[currentPos];
        insertCode0->opcode = gcSL_MOV;
        insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                            | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                            | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                            | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->tempIndex = Function->arguments[currentArg].index;

        insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
        insertCode0->source0Index = sampleId->index;
        currentPos++;
        currentArg++;

        /* Insert sampleMaskIn assignment. */
        if (IsIntrinsicsKindInterpolateAtCentroid(IntrinsicsKind))
        {
            insertCode0 = &Shader->code[currentPos];
            insertCode0->opcode = gcSL_MOV;
            insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_INTEGER)
                                | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_X)
                                | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            insertCode0->tempIndex = Function->arguments[currentArg].index;

            insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)
                                    | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                                    | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XXXX)
                                    | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                    | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            insertCode0->source0Index = sampleMaskIn->index;
            currentPos++;
            currentArg++;
        }

        /* Insert sampleLocation assignment. */
        for (j = 0; j < 4; j++)
        {
            insertCode0 = &Shader->code[currentPos + j];

            insertCode0->opcode = gcSL_MOV;
            insertCode0->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                                | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                                | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_TARGET_SET(0, Precision, gcSL_PRECISION_HIGH);
            insertCode0->tempIndex = Function->arguments[currentArg + j].index;

            insertCode0->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                                    | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                                    | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                    | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                    | gcmSL_SOURCE_SET(0, Precision, gcSL_PRECISION_HIGH);
            insertCode0->source0Index = gcmSL_INDEX_SET(0, Index, sampleLocationUniform->index)
                                        | gcmSL_INDEX_SET(0, ConstValue, j);
        }

        codeCount += createCodeCount;
        i += createCodeCount;
    }

OnError:
    return status;
}

/* Return the uniform's extra imageSize uniform. If not exist,
   create one imageSize uniform based on the Uniform's name. */
static gceSTATUS
_AddImageSizeUniform(
    IN  gcSHADER             Shader,
    IN  gcUNIFORM            Uniform,
    OUT gcUNIFORM            *SizeUniform
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T length = 0;
    gctUINT offset = 0;
    gctPOINTER pointer = gcvNULL;
    gctSTRING sizeUniformName = gcvNULL;

    gcmASSERT(isUniformImage(Uniform));

    length = gcoOS_StrLen(Uniform->name, gcvNULL) + 15;
    gcoOS_Allocate(gcvNULL, length, &pointer);
    gcoOS_ZeroMemory(pointer, length);
    sizeUniformName = pointer;

    gcoOS_PrintStrSafe(sizeUniformName,
                        length,
                        &offset,
                        "#sh_imageSize$%s",
                        Uniform->name);

    status = gcSHADER_GetUniformByName(Shader, sizeUniformName, length -1, SizeUniform);

    if (*SizeUniform == gcvNULL)
    {
        gcSHADER_AddUniform(Shader, sizeUniformName,
                            gcSHADER_INTEGER_X4, 1, gcSHADER_PRECISION_HIGH, SizeUniform);

        SetUniformFlag(*SizeUniform, gcvUNIFORM_FLAG_COMPILER_GEN);
        SetUniformCategory(*SizeUniform, gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE);

        (*SizeUniform)->parent = (gctINT16)Uniform->index;
        if (Uniform->firstChild == -1)
        {
            Uniform->firstChild = (gctINT16)((*SizeUniform)->index);
        }
        else
        {
            gcUNIFORM prev = gcvNULL;

            gcSHADER_GetUniform(Shader, (gctUINT)Uniform->firstChild, &prev);

            /* If this uniform is a 128 bpp image, then it already has a child. */
            while (prev->nextSibling != -1)
            {
                gcSHADER_GetUniform(Shader, (gctUINT)prev->nextSibling, &prev);
            }

            SetUniformNextSibling(prev, (gctINT16)((*SizeUniform)->index));
            SetUniformPrevSibling((*SizeUniform), (gctINT16)prev->index);
        }
    }

    if (pointer != gcvNULL)
    {
        gcmOS_SAFE_FREE(gcvNULL, pointer);
    }
    return status;
}

static gctBOOL
_isImageSizeFunc(
    IN gcSHADER     Shader,
    IN gctUINT      funcStart
    )
{
    gctSIZE_T i;
    for (i = 0; i < Shader->functionCount; i++)
    {
        if (Shader->functions[i] == gcvNULL)
            continue;

        if ((Shader->functions[i]->codeStart == funcStart) &&
            (Shader->functions[i]->nameLength == 15) &&
            (gcmIS_SUCCESS(gcoOS_MemCmp(Shader->functions[i]->name, "_viv_image_size", 15))))
        {
            return gcvTRUE;
        }
    }
    return gcvFALSE;
}

/* recursively find the call and check whether it is _image_size */
static gctBOOL
_HandleImageSizeFunc(
    IN gcSHADER     Shader,
    IN gcFUNCTION   Function,
    IN gcUNIFORM    Uniform
    )
{
    gctBOOL foundImageSize = gcvFALSE;
    gctSIZE_T i;
    gcSL_INSTRUCTION code;
    gcUNIFORM sizeUniform = gcvNULL;
    gcFUNCTION   function = gcvNULL;

    gcmASSERT(Uniform != gcvNULL);

    for (i = Function->codeStart; i < Function->codeStart + Function->codeCount ; i++)
    {
        code = Shader->code + i;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_CALL)
        {
            if (_isImageSizeFunc(Shader, code->tempIndex))
            {
                _AddImageSizeUniform(Shader, Uniform, &sizeUniform);

                code = Shader->code + i -1;

                code->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                                | gcmSL_SOURCE_SET(0, Format, gcSL_INTEGER)
                                | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                | gcmSL_SOURCE_SET(0, Precision, gcSHADER_PRECISION_HIGH);

                code->source0Index = gcmSL_INDEX_SET(0, Index, sizeUniform->index);

                foundImageSize = gcvTRUE;
            }
            else
            {
                gcSHADER_GetFunctionByHeadIndex(Shader, code->tempIndex, &function);
                gcmASSERT(function != gcvNULL);
                _HandleImageSizeFunc(Shader, function, Uniform);
            }
        }
    }

    return foundImageSize;
}

/*  This function is called when linking imageLoad/imageStore library function.
    It will link different image lib function based on the image format, since image
    format is known in the shader. This way will save the runtime switch for the
    image conversion.
*/
static gceSTATUS
_LinkImageLibFuc(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcFUNCTION           Function,
    IN gctCONST_STRING      FunctionName,
    IN gctBOOL              isSupportImgInst,
    IN gceINTRINSICS_KIND   IntrinsicsKind
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 codeCount = Shader->codeCount;
    gcSL_INSTRUCTION code, argCode;
    gctINT index;
    gcUNIFORM uniform = gcvNULL, extraUniform = gcvNULL;
    gctUINT32 i, j, movIndex = 0, callIndex = 0, index_constVal = 0;
    gcFUNCTION  imgLibFunc = Function;

    for (i = 0; i < codeCount; i++)
    {
        gctSIZE_T length = 0;
        gctUINT offset = 0;
        gctPOINTER pointer = gcvNULL;
        gctSTRING formatLibName = gcvNULL;
        gctBOOL extraArg = gcvFALSE;

        uniform = gcvNULL;
        index_constVal = 0;

        code = Shader->code + i;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL ||
            code->tempIndex != (gctUINT16)Function->codeStart)
        {
            continue;
        }

        callIndex = i;

        /* find the "mov temp, uniform" instruction:
            image load has 2 input and 1 output
                mov t1, img_desc
                mov t2, p
                call imgLoad
                mov t3, output
            image store has 3 input
                mov t1, img_desc
                mov t2, p
                mov t3, data
                call imgStore
            But it is possible that several instructions are used to
            pass one argument. Thus, we should not use code index,
            but the argument's temp. */

        for (j = 0; j <= i; j ++)
        {
            code = Shader->code + i - j;
            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV &&
                code->tempIndex == Function->arguments[0].index)
            {
                movIndex = i - j;
                break;
            }
        }
        gcmASSERT(gcmSL_SOURCE_GET(code->source0, Type) == gcSL_UNIFORM);
        gcmASSERT(gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED);

        index = gcmSL_INDEX_GET(code->source0Index, Index);
        index_constVal = gcmSL_INDEX_GET(code->source0Index, ConstValue);

        status  = gcSHADER_GetUniform(Shader, index, &uniform);
        gcmASSERT(uniform);

        if (!IsIntrinsicsKindImageAtomic(IntrinsicsKind))
        {
            imgLibFunc = gcvNULL;
            length = gcoOS_StrLen(FunctionName, gcvNULL);

            if (isSupportImgInst)
            {
                switch (uniform->imageFormat)
                {
                case gcIMAGE_FORMAT_RGBA32F:
                case gcIMAGE_FORMAT_RGBA32I:
                case gcIMAGE_FORMAT_RGBA32UI:
                    if (!isUniformImageBuffer(uniform))
                    {
                        length += 3;
                        gcoOS_Allocate(gcvNULL, length, &pointer);
                        gcoOS_ZeroMemory(pointer, length);
                        formatLibName = pointer;

                        gcoOS_PrintStrSafe(formatLibName,
                                            length,
                                            &offset,
                                            "%s_1",
                                            FunctionName);
                        extraArg = gcvTRUE;
                        break;
                    }

                default:
                    length += 1;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s",
                                        FunctionName);
                    break;
                }
            }
            else
            {
                switch (uniform->imageFormat)
                {
                case gcIMAGE_FORMAT_RGBA32F:
                    length += 9;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba32f",
                                        FunctionName);

                    extraArg = gcvTRUE;
                    break;
                case gcIMAGE_FORMAT_RGBA16F:
                case gcIMAGE_FORMAT_RGBA16I:
                case gcIMAGE_FORMAT_RGBA16UI:
                    length += 1;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_R32F:
                    length += 6;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_r32f",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_RGBA8:
                    length += 7;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba8",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_RGBA8_SNORM:
                    length += 13;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba8_snorm",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_RGBA32I:
                    length += 9;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba32i",
                                        FunctionName);
                    extraArg = gcvTRUE;
                    break;
                case gcIMAGE_FORMAT_RGBA8I:
                    length += 8;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba8i",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_R32I:
                    length += 6;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_r32i",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_RGBA32UI:
                    length += 10;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba32ui",
                                        FunctionName);
                    extraArg = gcvTRUE;
                    break;
                case gcIMAGE_FORMAT_RGBA8UI:
                    length += 9;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_rgba8ui",
                                        FunctionName);
                    break;
                case gcIMAGE_FORMAT_R32UI:
                    length += 7;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s_r32ui",
                                        FunctionName);
                    break;
                default:
                    length += 1;
                    gcoOS_Allocate(gcvNULL, length, &pointer);
                    gcoOS_ZeroMemory(pointer, length);
                    formatLibName = pointer;

                    gcoOS_PrintStrSafe(formatLibName,
                                        length,
                                        &offset,
                                        "%s",
                                        FunctionName);
                    break;
                }
            }

            /*
            ** If this image is a buffer, we can support 128 bpp, so we don't need to add this extra layer.
            */
            if (isUniformImageBuffer(uniform))
            {
                extraArg = gcvFALSE;
            }

            gcSHADER_GetFunctionByName(Shader, formatLibName, &imgLibFunc);
            if (imgLibFunc == gcvNULL)
            {
                /* not linked to Shader yet, link it */
                status = gcSHADER_LinkLibFunction(Shader,
                                            Library,
                                            formatLibName,
                                            &imgLibFunc);
            }

            gcmASSERT(imgLibFunc != gcvNULL);

            if (imgLibFunc != Function)
            {
                code = Shader->code + callIndex;

                /* change the call target */
                code->tempIndex = (gctUINT16)imgLibFunc->codeStart;

                /* change the input to be the new extraLayerLib's temp */
                argCode = &Shader->code[movIndex];
                argCode->tempIndex = imgLibFunc->arguments[0].index;

                /* insert an extra assigment to pass extraLayer uniform */
                if (extraArg)
                {
                    gcmASSERT(uniform->firstChild != -1);
                    gcmONERROR(gcSHADER_GetUniform(Shader, (gctUINT)uniform->firstChild, &extraUniform));
                    /* Insert one argument assignment */
                    gcmONERROR(gcSHADER_InsertNOP2BeforeCode(Shader, movIndex + 1, 1));
                    gcSHADER_Pack(Shader);

                    argCode = &Shader->code[movIndex + 1];
                    argCode->opcode = gcSL_MOV;
                    argCode->temp = gcmSL_TARGET_SET(0, Format, extraUniform->format)
                                    | gcmSL_TARGET_SET(0, Enable, gcSL_ENABLE_XYZW)
                                    | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                                    | gcmSL_TARGET_SET(0, Precision, uniform->precision);
                    argCode->tempIndex = imgLibFunc->arguments[1].index;

                    argCode->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_UNIFORM)
                                        | gcmSL_SOURCE_SET(0, Format, extraUniform->format)
                                        | gcmSL_SOURCE_SET(0, Swizzle, gcSL_SWIZZLE_XYZW)
                                        | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                                        | gcmSL_SOURCE_SET(0, Precision, uniform->precision);

                    argCode->source0Index = gcmSL_INDEX_SET(0, Index, extraUniform->index)
                                            | gcmSL_INDEX_SET(argCode->source0Index, ConstValue, index_constVal);

                    codeCount ++;
                    i ++;
                }

                /* change the argument 1 to be the new lib's temp */
                for (j = (movIndex + (extraArg ? 2 : 1)); j < (extraArg ? callIndex + 1 : callIndex); j++)
                {
                    argCode = &Shader->code[j];
                    if (argCode->tempIndex == Function->arguments[1].index)
                    {
                        argCode->tempIndex = imgLibFunc->arguments[(extraArg ? 2 : 1)].index;
                    }
                }

                if (IsIntrinsicsKindImageLoad(IntrinsicsKind))
                {
                    /* change the output to be the new lib's temp */
                    for (j = callIndex + (extraArg ? 2 : 1);
                            j < ((callIndex + 4 < codeCount) ? callIndex + 4: codeCount); j++)
                    {
                        argCode = &Shader->code[j];
                        if (argCode->source0Index == Function->arguments[2].index)
                        {
                            argCode->source0Index = imgLibFunc->arguments[(extraArg ? 3 : 2)].index;
                        }
                    }
                }
                else
                {
                    /* change the argument 2 to be the new lib's temp */
                    for (j = (movIndex + (extraArg ? 3 : 2)); j < (extraArg ? callIndex + 1: callIndex) ; j++)
                    {
                        argCode = &Shader->code[j];
                        if (argCode->tempIndex == Function->arguments[2].index)
                        {
                            argCode->tempIndex = imgLibFunc->arguments[(extraArg ? 3 : 2)].index;
                        }
                    }
                }
            }
        }

        _HandleImageSizeFunc(Shader, imgLibFunc, uniform);

        if (pointer != gcvNULL)
        {
            gcmOS_SAFE_FREE(gcvNULL, pointer);
        }
    }

OnError:
    return status;
}

static gceSTATUS
_LinkImageSizeLibFuc(
    IN gcSHADER             Shader,
    IN gcSHADER             Library,
    IN gcFUNCTION           Function
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 codeCount = Shader->codeCount;
    gcSL_INSTRUCTION code, argCode;
    gctINT index;
    gcUNIFORM uniform, sizeUniform = gcvNULL;
    gctUINT32 i, movIndex;

    for (i = 0; i < codeCount; i++)
    {
        uniform = gcvNULL;
        code = Shader->code + i;

        if (gcmSL_OPCODE_GET(code->opcode, Opcode) != gcSL_CALL ||
            code->tempIndex != (gctUINT16)Function->codeStart)
        {
            continue;
        }

        movIndex = i - (Function->argumentCount - 1);
        code = code - (Function->argumentCount - 1);

        /* find the mov temp, uniform instruction */
        gcmASSERT(gcmSL_SOURCE_GET(code->source0, Type) == gcSL_UNIFORM);
        gcmASSERT(gcmSL_SOURCE_GET(code->source0, Indexed) == gcSL_NOT_INDEXED);

        index = gcmSL_INDEX_GET(code->source0Index, Index);
        status  = gcSHADER_GetUniform(Shader, index, &uniform);
        gcmASSERT(uniform);

        _AddImageSizeUniform(Shader, uniform, &sizeUniform);

        /* change the input to be the new size uniform's index */
        argCode = &Shader->code[movIndex];
        argCode->source0Index = gcmSL_INDEX_SET(0, Index, sizeUniform->index);
    }

    return status;
}

/* Find the library function name as FunctionName in library Lib,
   if the name is exist in the Lib, add the function to Shader
   and return the pointer the the function.
 */
gceSTATUS
gcSHADER_LinkLibFunction(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gctCONST_STRING  FunctionName,
    OUT    gcFUNCTION *     Function
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcFUNCTION  libFunction = gcvNULL;
    gcFUNCTION  function = gcvNULL;
    gctPOINTER  pointer = gcvNULL;
    gcSL_INSTRUCTION newCode = gcvNULL;
    gcSL_INSTRUCTION code;
    gctUINT     tempIndexStart;
    gctINT      codeOffset;
    gctINT      tempOffset;
    gctUINT     lowTemp;
    gctUINT     i;
    _MappingInfo mi;
    gcLibraryList *libList;
    gctBOOL      funcInShader = gcvTRUE;
    gctBOOL *    checkVariable = gcvNULL;
    gceINTRINSICS_KIND intrinsicsKind;
    gctINT  paraMapping[gcMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL     isSupportTextureGather = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TEXTURE_GATHER);
    gctBOOL     isSupportTexelFetchForMSAA = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_MSAA_TEXTURE);
    gctBOOL     isSupportImgAddr = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_IMG_INSTRUCTION);
    gctBOOL     isSupportImgInst = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI5) ?
                                    gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_USC_GOS_ADDR_FIX) :
                                    isSupportImgAddr;
    gctSTRING   functionName = (gctSTRING)FunctionName;

    if (isSupportImgAddr && !isSupportImgInst &&
        (GetShaderType(Shader) == gcSHADER_TYPE_COMPUTE || GetShaderType(Shader) == gcSHADER_TYPE_CL))
    {
        isSupportImgInst = gcvTRUE;
    }

    gcmONERROR(gcSHADER_GetFunctionByName(Library, functionName, &libFunction));
    if (libFunction == gcvNULL)
    {
        gcoOS_Print("Error: Failed to link unsatified function %s to shader (id:%d)",
                         functionName, GetShaderID(Shader));
        *Function = gcvNULL;
        return gcvSTATUS_UNSAT_LIB_SYMBOL;
    }
    gcmONERROR(gcSHADER_GetFunctionByName(Shader, functionName, &function));
    if(function == gcvNULL)
    {
        if(libFunction->intrinsicsKind == gceINTRIN_source)
        {
            gcmASSERT(libFunction->codeCount == 0);
            gcoOS_Print("Error: Failed to link unsatified function %s to shader (id:%d)",
                        functionName, GetShaderID(Shader));
            *Function = gcvNULL;
            return gcvSTATUS_UNSAT_LIB_SYMBOL;
        }

        funcInShader = gcvFALSE;
        /* Add function. */
        gcmONERROR(gcSHADER_AddFunction(Shader, functionName, &function));

        /* Add label. */
        gcmONERROR(gcSHADER_AddLabel(Shader, function->label));
    }

    intrinsicsKind = function->intrinsicsKind;

    /* allocate temp registers in Shader */
    tempIndexStart = gcSHADER_NewTempRegs(Shader,
                                          libFunction->tempIndexCount,
                                          gcSHADER_FLOAT_X1);

    /*
    ** We need to create extra uniform(lodMinMax and levelBaseSize)
    ** and insert them after sampler index argument.
    */
    if (IsIntrinsicsKindCreateSamplerSize(intrinsicsKind))
    {
        gcmONERROR(_CreateSamplerSizeArgument(Shader, libFunction, function, gcvTRUE));
    }
    else if (IsIntrinsicsKindNeedTexSizeOnly(intrinsicsKind))
    {
        gcmONERROR(_CreateSamplerSizeArgument(Shader, libFunction, function, gcvFALSE));
    }
    else if (IsIntrinsicsKindMSInterpolation(intrinsicsKind))
    {
        gcmONERROR(_CreateInterpolationArgument(Shader, Library, libFunction, function));
    }

    function->tempIndexStart = tempIndexStart;
    function->tempIndexEnd   = function->tempIndexStart + (libFunction->tempIndexEnd - libFunction->tempIndexStart);
    function->tempIndexCount = libFunction->tempIndexCount;
    function->codeCount      = libFunction->codeCount;
    function->flags          = libFunction->flags;
    function->intrinsicsKind = libFunction->intrinsicsKind;
    function->isRecursion    = libFunction->isRecursion;

    /* Allocate and copy arguments and variables. */
    if (!funcInShader && libFunction->argumentArrayCount > 0)
    {
        gctUINT16 i, index, variableIndex;
        gcmONERROR(gcFUNCTION_ReallocateArguments(function, libFunction->argumentArrayCount));
        gcoOS_MemCopy(function->arguments,
                      libFunction->arguments,
                      gcmSIZEOF(gcsFUNCTION_ARGUMENT) * libFunction->argumentCount);
        function->argumentCount = libFunction->argumentCount;

        for (i = 0; i < function->argumentCount; i++)
        {
            variableIndex = function->arguments[i].variableIndex;

            if (variableIndex != 0xffff)
            {
                gcSHADER_CopyVariable(Shader,
                                      Library->variables[variableIndex],
                                      &index);
                function->arguments[i].variableIndex = index;
                Shader->variables[index]->tempIndex = index;
            }
        }
    }

    if (function->localVariableCount > 0)
    {
        gcmASSERT(function->localVariableCount == 0);
        function->localVariableCount = libFunction->localVariableCount;
    }

    /* Reallocate code array if no enough space. */
    if(libFunction->codeCount) {
        if (Shader->codeCount <= Shader->lastInstruction + libFunction->codeCount)
        {
            gcmONERROR(gcoOS_Allocate(gcvNULL,
                                      gcmSIZEOF(struct _gcSL_INSTRUCTION) * (Shader->lastInstruction + libFunction->codeCount),
                                      &pointer));
            newCode = pointer;

            /* Copy existing code. */
            gcoOS_MemCopy(newCode,
                          Shader->code,
                          gcmSIZEOF(struct _gcSL_INSTRUCTION) * Shader->lastInstruction);
            /* copy library funciton code to Shader */
            gcoOS_MemCopy(newCode + Shader->lastInstruction,
                          Library->code + libFunction->codeStart,
                          gcmSIZEOF(struct _gcSL_INSTRUCTION) * libFunction->codeCount);
            /* Free the current array of object pointers. */
            gcmOS_SAFE_FREE(gcvNULL, Shader->code);
            Shader->code = newCode;
        }
        else
        {
            /* copy library funciton code to Shader */
            gcoOS_MemCopy(Shader->code + Shader->lastInstruction,
                          Library->code + libFunction->codeStart,
                          gcmSIZEOF(struct _gcSL_INSTRUCTION) * libFunction->codeCount);
        }
    }

    /* adjust code count and last instruction pointer */
    Shader->codeCount = Shader->lastInstruction + libFunction->codeCount;
    function->codeStart = Shader->lastInstruction;
    Shader->lastInstruction += libFunction->codeCount;

    /* initialize Library's temp register mapping for the Shader */
    libList = gcSHADER_InitMappingTable(Library, Shader);
    if(libList == gcvNULL)
    {
        status = gcvSTATUS_INVALID_ARGUMENT;
        gcmONERROR(status);
    }

    /* Find temp register range used in the function. */
    lowTemp = libFunction->tempIndexStart;

    /* Relocate temp registers and labels. */
    tempOffset = tempIndexStart - lowTemp;

    /* Insert the assignment for the new arguments. */
    if (IsIntrinsicsKindCreateSamplerSize(intrinsicsKind))
    {
        gcmONERROR(_InsertAssignmentForSamplerSize(Shader,
                                                   function,
                                                   gcvTRUE,
                                                   gcvTRUE,
                                                   intrinsicsKind,
                                                   gcvNULL,
                                                   gcvNULL));
    }
    else if (IsIntrinsicsKindTextureGatherOffset(intrinsicsKind) ||
             IsIntrinsicsKindTextureGatherOffsets(intrinsicsKind))
    {
        gcmONERROR(_InsertAssignmentForSamplerSize(Shader,
                                                   function,
                                                   gcvFALSE,
                                                   gcvTRUE,
                                                   intrinsicsKind,
                                                   gcvNULL,
                                                   gcvNULL));
    }
    else if ((IsIntrinsicsKindTextureGather(intrinsicsKind) && !isSupportTextureGather) ||
             (IsIntrinsicsKindTexelFetchForMSAA(intrinsicsKind) && !isSupportTexelFetchForMSAA))
    {
        gcmONERROR(_InsertAssignmentForSamplerSize(Shader,
                                                   function,
                                                   gcvFALSE,
                                                   gcvTRUE,
                                                   intrinsicsKind,
                                                   gcvNULL,
                                                   gcvNULL));
    }
    else if (IsIntrinsicsKindMSInterpolation(intrinsicsKind))
    {
        gcmONERROR(_InsertAssignmentForInterpolation(Shader,
                                                     function,
                                                     intrinsicsKind));
    }

    /* record the arguments maps for*/
    for (i = 0; i < gcMAX_BUILT_IN_PARAMETER_COUNT; i++)
    {
        paraMapping[i] = -1;
    }

    for (i = 0; i < function->argumentCount; i++)
    {
        gctINT mappedTempRegIndex;

        mappedTempRegIndex = (gctINT)libFunction->arguments[i].index + tempOffset;

        _MapTempRegister(libList,
                         libFunction->arguments[i].index,
                         mappedTempRegIndex);

        paraMapping[i] = mappedTempRegIndex;
    }

    mi.Shader      = Shader;
    mi.LibList     = libList;
    mi.TempOffset  = tempOffset;
    mi.Function    = function;

    /* handle all the calls to this function
       1) change its tempIndex (to this function's start)
       2) change its input/output (to the mapped temp)
       The reason to change the input/output to let the function
       has consecutive temps, including its args and locals. */
    for (i = 0; i < Shader->codeCount; i++)
    {
        gcSL_OPCODE opcode;
        gcSL_INSTRUCTION code;
        gctINT j, endIndex;

        code = Shader->code + i;
        opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
        if (opcode == gcSL_CALL)
        {
            if (code->tempIndex == function->label)
            {
                code->tempIndex = (gctUINT16) function->codeStart;

                if (function->codeStart > i)
                {
                    endIndex = 0;
                }
                else
                {
                    endIndex = (gctINT)(function->codeStart + function->codeCount);
                }

                for (j = (gctINT)i - 1; j >= endIndex; j--)
                {
                    gctUINT argIndex = 0;

                    code = Shader->code + j;
                    opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                    if (opcode == gcSL_CALL || opcode == gcSL_RET || opcode == gcSL_JMP)
                        break;

                    if (_inputOutputArgument(function, code->tempIndex, gcvTRUE, &argIndex))
                    {
                        gcmASSERT(paraMapping[argIndex] != -1);
                        code->tempIndex = (gctUINT16) paraMapping[argIndex];
                    }
                }

                if (function->codeStart > i)
                {
                    endIndex = (gctINT)function->codeStart;
                }
                else
                {
                    endIndex = (gctINT)Shader->codeCount;
                }

                /* rename all the output arguments
                   It is better to let FE generate the same format for output,
                   e.g., MOV tmp, out_tmp*/
                for (j = (gctINT)i + 1; j < endIndex; j++)
                {
                    gctUINT argIndex = 0;

                    code = Shader->code + j;
                    opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
                    if (opcode == gcSL_CALL || opcode == gcSL_RET || opcode == gcSL_JMP)
                        break;

                    if ((gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP) &&
                        _inputOutputArgument(function, code->source0Index, gcvFALSE, &argIndex))
                    {
                        gcmASSERT(paraMapping[argIndex] != -1);
                        code->source0Index = (gctUINT16) paraMapping[argIndex];
                    }
                    if ((gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP) &&
                        _inputOutputArgument(function, code->source1Index, gcvFALSE, &argIndex))
                    {
                        gcmASSERT(paraMapping[argIndex] != -1);
                        code->source1Index = (gctUINT16) paraMapping[argIndex];
                    }
                }
            }
        }
    }

    if (Shader->variableCount > 0)
    {
        /* change arguments according to the map*/
        gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, Shader->variableCount * gcmSIZEOF(gctBOOL), &pointer));
        gcoOS_ZeroMemory(pointer, Shader->variableCount * gcmSIZEOF(gctBOOL));
        checkVariable = (gctBOOL *)pointer;
    }

    for (i = 0; i < function->argumentCount; i++)
    {
        gctINT mappedTempRegIndex;

        mappedTempRegIndex = (gctINT)libFunction->arguments[i].index + tempOffset;

        /*
        ** It is possible that more than one arguments map to one variable(if this variable is an array or a structure),
        ** so we only need to update this variable one time.
        */
        if (function->arguments[i].variableIndex != 0xffff &&
            checkVariable &&
            !checkVariable[function->arguments[i].variableIndex])
        {
            Shader->variables[function->arguments[i].variableIndex]->tempIndex = (gctUINT16) mappedTempRegIndex;
            checkVariable[function->arguments[i].variableIndex] = gcvTRUE;
        }

        function->arguments[i].index = (gctUINT16) mappedTempRegIndex;
    }

    if (checkVariable)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, checkVariable));
    }

    codeOffset = function->codeStart - libFunction->codeStart;

    /* handle all function calls in the lib funciton to link in callees  */
    for (i = 0; i < function->codeCount; i++)
    {
        gctUINT opcode;
        gcSL_INSTRUCTION code;

        code = Shader->code + function->codeStart + i;
        opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);
        if (opcode == gcSL_CALL)
        {
            gcFUNCTION  shaderFunction;
            gcSL_INSTRUCTION newCode;

            gcmONERROR(_linkLibFunctionToShader(Shader, Library, code, &shaderFunction));

            /* Shader->code may be changed after link lib function to shader */
            newCode = Shader->code + function->codeStart + i;
            newCode->tempIndex = (gctUINT16) shaderFunction->codeStart;
        }
    }

    /* fix temp register used in all code */
    code = Shader->code + function->codeStart;
    for (i = 0; i < function->codeCount; i++, code++)
    {
        gctUINT opcode = gcmSL_OPCODE_GET(code->opcode, Opcode);

        /* Adjust label temporary register number.
         * the assumption is no global variable is used in the
         * lib function, otherwise it will be renamed to wrong
         * temp index
         */
        switch (opcode)
        {
        case gcSL_NOP:
            /* fall through */
        case gcSL_RET:
            /* fall through */
        case gcSL_BARRIER:
        case gcSL_MEM_BARRIER:
            /* Skip instructions without destination and source. */
            break;

        default:

            /* fix tempIndex */
            if (opcode == gcSL_JMP)
            {
                /* Adjust label. */
                code->tempIndex += (gctUINT16) codeOffset;
            }
            else if (opcode == gcSL_CALL)
            {
                /* already processed, do nothing here */
                break;
            }
            else
            {
                if (!(opcode == gcSL_STORE1 && code->tempIndex == 0))
                {
                   _fixTempIndexByMappingTable(&mi, &code->tempIndex);
                }
            }

            /* fix target */
            if (gcmSL_TARGET_GET(code->temp, Indexed) != gcSL_NOT_INDEXED)
            {
                /* fix indexed register */
                _fixTempIndexByMappingTable(&mi, &code->tempIndexed);
            }

            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_TEMP)
            {
                /* Adjust temporary register count. */
                _fixTempIndexByMappingTable(&mi, &code->source0Index);
            }
            if (gcmSL_SOURCE_GET(code->source0, Indexed) != gcSL_NOT_INDEXED)
            {
                /* fix indexed register */
                _fixTempIndexByMappingTable(&mi, &code->source0Indexed);
            }
            if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE)
            {
                /* Adjust buildin attribute usage */
                gctUINT attr_index = gcmSL_INDEX_GET(code->source0Index, Index);
                gcATTRIBUTE lib_attr;
                gcATTRIBUTE attr;

                gcmASSERT(attr_index < Library->attributeCount);
                lib_attr = Library->attributes[attr_index];
                gcmASSERT(lib_attr->nameLength < 0);    /* this attribute has to be buildin attribute, because it's from library */
                if (!_shaderHasBuildinAttr(Shader, (gceBuiltinNameKind)lib_attr->nameLength, &attr))
                {
                    switch((gceBuiltinNameKind)lib_attr->nameLength)
                    {
                        case gcSL_POSITION:
                            gcSHADER_AddAttribute(Shader, "gl_Position", gcSHADER_FLOAT_X4,
                                                  1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &attr);
                            break;
                        default:
                            gcmASSERT(0);       /* we need to add this attribute here */
                    }
                }
                code->source0Index = gcmSL_INDEX_SET(code->source0Index, Index, attr->index);
            }

            /* fix source1 */
            if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_TEMP)
            {
                /* Adjust temporary register count. */
                _fixTempIndexByMappingTable(&mi, &code->source1Index);
            }
            if (gcmSL_SOURCE_GET(code->source1, Indexed) != gcSL_NOT_INDEXED)
            {
                /* fix indexed register */
                _fixTempIndexByMappingTable(&mi, &code->source1Indexed);
            }
            if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE)
            {
                /* Adjust buildin attribute usage */
                gctUINT attr_index = gcmSL_INDEX_GET(code->source1Index, Index);
                gcATTRIBUTE lib_attr;
                gcATTRIBUTE attr;

                gcmASSERT(attr_index < Library->attributeCount);
                lib_attr = Library->attributes[attr_index];
                gcmASSERT(lib_attr->nameLength < 0);    /* this attribute has to be buildin attribute, because it's from library */
                if (!_shaderHasBuildinAttr(Shader, (gceBuiltinNameKind)lib_attr->nameLength, &attr))
                {
                    switch((gceBuiltinNameKind)lib_attr->nameLength)
                    {
                        case gcSL_POSITION:
                            gcSHADER_AddAttribute(Shader, "gl_Position", gcSHADER_FLOAT_X4,
                                                  1, gcvFALSE, gcSHADER_SHADER_DEFAULT, gcSHADER_PRECISION_HIGH, &attr);
                            break;
                        default:
                            gcmASSERT(0);       /* we need to add this attribute here */
                    }
                }
                code->source1Index = gcmSL_INDEX_SET(code->source1Index, Index, attr->index);
            }
        }
    }

    if (IsIntrinsicsKindImageLoad(intrinsicsKind) ||
        IsIntrinsicsKindImageStore(intrinsicsKind) ||
        IsIntrinsicsKindImageAtomic(intrinsicsKind))
    {
        gcmONERROR(_LinkImageLibFuc(Shader,
                                    Library,
                                    function,
                                    functionName,
                                    isSupportImgInst,
                                    intrinsicsKind));
    }

    if (IsIntrinsicsKindImageSize(intrinsicsKind))
    {
        gcmONERROR(_LinkImageSizeLibFuc(Shader, Library, function));
    }

    if (Function)
    {
        *Function = function;
    }

OnError:
    /* Return the status. */
    return status;
}

static gceSTATUS
_addTextureGradUniform(
    IN OUT gcSHADER       Shader,
    IN     gcUNIFORM      Uniform,
    IN OUT gcUNIFORM     *RectUniform,
    IN OUT gcUNIFORM     *LodMinMaxUniform
    )
{
    gceSTATUS    status            = gcvSTATUS_OK;
    gcUNIFORM    lodMinMaxUniform  = gcvNULL;
    gcUNIFORM    rectUniform       = gcvNULL;
    gctCHAR      name[64];
    gctUINT      offset            = 0;
    gctUINT      i;

    gcmASSERT(Uniform != gcvNULL);

    /* construct const vector name, create and initialize constant uniform */
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_Rect_%d", Uniform->index);

    for(i = 0; i < Shader->uniformCount; i++)
    {
        rectUniform = Shader->uniforms[i];
        if(rectUniform && rectUniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(rectUniform->name, name)))
            {
                break;
            }
        }
    }

    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                            name,
                                            gcSHADER_INTEGER_X4,
                                            gcSHADER_PRECISION_MEDIUM,
                                            -1,
                                            -1,
                                            -1,
                                            0,
                                            gcvNULL,
                                            gcSHADER_VAR_CATEGORY_LEVEL_BASE_SIZE,
                                            0,
                                            Uniform->index,
                                            -1,
                                            gcIMAGE_FORMAT_DEFAULT,
                                            gcvNULL,
                                            &rectUniform));
    }

    if(RectUniform)
    {
        *RectUniform = rectUniform;
    }

    offset = 0;
    gcoOS_PrintStrSafe(name, sizeof(name), &offset, "#sh_LodMinMax_%d", Uniform->index);

    for(i = 0; i < Shader->uniformCount; i++)
    {
        lodMinMaxUniform = Shader->uniforms[i];
        if(lodMinMaxUniform && lodMinMaxUniform->name)
        {
            if(gcmIS_SUCCESS(gcoOS_StrCmp(lodMinMaxUniform->name, name)))
            {
                break;
            }
        }
    }

    if(i == Shader->uniformCount)
    {
        gcmONERROR(gcSHADER_AddUniformEx1(Shader,
                                            name,
                                            gcSHADER_INTEGER_X3,
                                            gcSHADER_PRECISION_MEDIUM,
                                            -1,
                                            -1,
                                            -1,
                                            0,
                                            gcvNULL,
                                            gcSHADER_VAR_CATEGORY_LOD_MIN_MAX,
                                            0,
                                            Uniform->index,
                                            -1,
                                            gcIMAGE_FORMAT_DEFAULT,
                                            gcvNULL,
                                            &lodMinMaxUniform));
    }

    if(RectUniform)
    {
        *LodMinMaxUniform = lodMinMaxUniform;
    }

OnError:
    return status;
}

typedef gceSTATUS (*create_builtin_func_ptr)(gcSHADER Shader, gcSHADER Library, gctINT Index, gcFUNCTION *Func);

typedef gctBOOL   (*check_builtin_func_prt)(gcSHADER Shader, gcSHADER Library, gctINT Index);

gctBOOL
_checkTexGradBuiltinFunc(
    IN gcSHADER     Shader,
    IN gcSHADER     Library,
    IN gctINT       Index
)
{
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI2))
    {
        return gcvFALSE;
    }

    return gcvTRUE;
}

gceSTATUS
_createTexGradBuiltinFunc(
    IN OUT gcSHADER     Shader,
    IN     gcSHADER     Library,
    IN     gctINT       Index,
    IN OUT gcFUNCTION  *OutFunction
)
{
    gceSTATUS        status        = gcvSTATUS_OK;
    gcFUNCTION       stubFunction  = gcvNULL;
    gcFUNCTION       gradFunction  = gcvNULL;
    gctCHAR          funcName[32];
    gctUINT          offset        = 0;
    gctUINT          argNo;
    gcSL_INSTRUCTION tempCode      = gcvNULL;
    gctPOINTER       pointer       = gcvNULL;
    gcSL_INSTRUCTION code;
    gcUNIFORM        rectUniform      = gcvNULL;
    gcUNIFORM        lodMinMaxUniform = gcvNULL;
    gcsValue         val0;
    gcSL_SWIZZLE     srcSwizzle;
    gceTexldFlavor   texldFlavor;
    gctCONST_STRING  flavor;
    gctINT           samplerId      = 0;

    if (OutFunction)
    {
        *OutFunction = gcvNULL;
    }

    gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, sizeof(struct _gcSL_INSTRUCTION), &pointer));

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "_viv_textureGrad"));

    texldFlavor = _getTexldFlavor((gcSL_OPCODE)Shader->code[Index + 1].opcode,
        gcSL_NOP);

    flavor = gcTexldFlavor[texldFlavor];
    gcmONERROR(gcoOS_StrCatSafe(funcName, gcmSIZEOF(funcName), flavor));

    gcmONERROR(_FindFunctionFromShaderOrLibrary(Shader,
                                                 Library,
                                                 funcName,
                                                 &gradFunction));

    tempCode = (gcSL_INSTRUCTION) pointer;

    gcoOS_MemCopy(tempCode, &Shader->code[Index], sizeof(struct _gcSL_INSTRUCTION));

    code = tempCode;

    offset = 0;
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(funcName, sizeof(funcName), &offset,
                           "#inputBuiltin_%d", Index));
    /* Add stubFunction to Shader. */
    gcSHADER_AddFunction(Shader, funcName, &stubFunction);

    /* add arguments */

    gcSHADER_BeginFunction(Shader, stubFunction);

    argNo = 0;

    gcmASSERT(gcmSL_SOURCE_GET(Shader->code[Index + 1].source0, Indexed) == gcSL_NOT_INDEXED);

    samplerId = gcmSL_INDEX_GET(Shader->code[Index + 1].source0Index, Index);

    if (gcmSL_SOURCE_GET(Shader->code[Index + 1].source0, Type) == gcSL_SAMPLER)
    {
        gcUNIFORM uniform;
        uniform = gcSHADER_GetUniformBySamplerIndex(Shader, samplerId, gcvNULL);
        gcmONERROR(_addTextureGradUniform(Shader, uniform, &rectUniform, &lodMinMaxUniform));
        samplerId = uniform->index;
    }
    else
    {
        gcmASSERT(samplerId < (gctINT)Shader->uniformCount);
        gcmONERROR(_addTextureGradUniform(Shader, Shader->uniforms[samplerId], &rectUniform, &lodMinMaxUniform));
    }

    /* mov  arg0, rectUniform */
    val0.i32 = rectUniform->index;
    srcSwizzle = gcSL_SWIZZLE_XYZW;
    _addArgPassInst(Shader, gradFunction, stubFunction, gcvNULL, argNo++ /*ARG0*/, gcvIntUniformIndex, &val0, srcSwizzle, gcvTRUE, rectUniform->precision);

    /* mov  arg1, lodMinMaxUniform */
    val0.i32 = lodMinMaxUniform->index;
    srcSwizzle = gcSL_SWIZZLE_XYZZ;
    _addArgPassInst(Shader, gradFunction, stubFunction, gcvNULL, argNo++ /*ARG1*/, gcvIntUniformIndex, &val0, srcSwizzle, gcvTRUE, lodMinMaxUniform->precision);

    /* mov arg2, sampler */
    /* create sampler argument:
     * mov  arg0, uniformIndex of sampler */
    val0.i32 = samplerId;
    _addArgPassInst(Shader, gradFunction, stubFunction, &Shader->code[Index + 1],
                    argNo++ /*ARG2*/, gcvIntConstant, &val0,
                    gcSL_SWIZZLE_INVALID, gcvTRUE, gcSHADER_PRECISION_HIGH);

     /* mov  arg3, coord */
    _addArgPassInst(Shader, gradFunction, stubFunction, &Shader->code[Index + 1],
                    argNo++ /*ARG3*/, gcvSource1, gcvNULL,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

     /* mov  arg4, pDx */
    _addArgPassInst(Shader, gradFunction, stubFunction, &Shader->code[Index],
                    argNo++ /*ARG4*/, gcvSource0, gcvNULL,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

     /* mov  arg5, pDy */
    _addArgPassInst(Shader, gradFunction, stubFunction, &Shader->code[Index],
                    argNo++ /*ARG5*/, gcvSource1, gcvNULL,
                    gcSL_SWIZZLE_INVALID, gcvFALSE, gcSHADER_PRECISION_ANY);

    /* call _convert_func_n */
    _addCallInst(Shader, gradFunction);

    /* mov  target, arg6 */
    _addRetValueInst(Shader, gradFunction, code, argNo++, gcvDest /*DEST*/, gcvNULL);

    /* ret */
    _addRetInst(Shader);

    gcSHADER_EndFunction(Shader, stubFunction);

    if (OutFunction)
    {
        *OutFunction = stubFunction;
    }

    if (tempCode)
    {
        /* Free the current code buffer. */
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tempCode));
    }

OnError:
    return status;
}

/* link the builtin function library into the shader */
gceSTATUS
gcSHADER_LinkBuiltinLibrary(
    IN OUT gcSHADER         Shader,
    IN     gcSHADER         Library,
    IN     gcLibType        LibType
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     i, j, notSupportedBlendNum;
    gceLAYOUT_QUALIFIER* notSupportedBlendModes;
    gctSTRING* notSupportedBlendModeFunctions;
    gctBOOL     isHalti4 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_HALTI4);

    gceLAYOUT_QUALIFIER   HWNotSupportedBlendEquatioins[] =
    {
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_MULTIPLY,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_OVERLAY,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_DARKEN,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_LIGHTEN,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORDODGE,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORBURN,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HARDLIGHT,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_SOFTLIGHT,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_DIFFERENCE,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_EXCLUSION,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_HUE,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_SATURATION,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_COLOR,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_LUMINOSITY,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_SCREEN,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_ALL_EQUATIONS
    };

    gctSTRING   HWNotSupportedBlendEquatioinsFunc[] =
    {
        "_blend_equation_advanced_multiply",
        "_blend_equation_advanced_overlay",
        "_blend_equation_advanced_darken",
        "_blend_equation_advanced_lighten",
        "_blend_equation_advanced_colordodge",
        "_blend_equation_advanced_colorburn",
        "_blend_equation_advanced_hardlight",
        "_blend_equation_advanced_softlight",
        "_blend_equation_advanced_difference",
        "_blend_equation_advanced_exclusion",
        "_blend_equation_advanced_hsl_hue",
        "_blend_equation_advanced_hsl_saturation",
        "_blend_equation_advanced_hsl_color",
        "_blend_equation_advanced_hsl_luminosity",
        "_blend_equation_advanced_screen",
        "_blend_equation_advanced_all",
    };

    gceLAYOUT_QUALIFIER   HWNotSupportedBlendEquatioins_halti4[] =
    {
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORDODGE,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_COLORBURN,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_SOFTLIGHT,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_HUE,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_SATURATION,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_COLOR,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_HSL_LUMINOSITY,
        gcvLAYOUT_QUALIFIER_BLEND_SUPPORT_ALL_EQUATIONS
    };

    gctSTRING   HWNotSupportedBlendEquatioinsFunc_halti4[] =
    {
        "_blend_equation_advanced_colordodge",
        "_blend_equation_advanced_colorburn",
        "_blend_equation_advanced_softlight",
        "_blend_equation_advanced_hsl_hue",
        "_blend_equation_advanced_hsl_saturation",
        "_blend_equation_advanced_hsl_color",
        "_blend_equation_advanced_hsl_luminosity",
        "_blend_equation_advanced_all",
    };

    notSupportedBlendNum = isHalti4 ? sizeof(HWNotSupportedBlendEquatioins_halti4) / sizeof(gceLAYOUT_QUALIFIER)
        : sizeof(HWNotSupportedBlendEquatioins) / sizeof(gceLAYOUT_QUALIFIER);
    notSupportedBlendModes = isHalti4 ? HWNotSupportedBlendEquatioins_halti4 : HWNotSupportedBlendEquatioins;
    notSupportedBlendModeFunctions = isHalti4 ? HWNotSupportedBlendEquatioinsFunc_halti4 : HWNotSupportedBlendEquatioinsFunc;

    gcmASSERT(GetShaderHasIntrinsicBuiltin(Shader) ||
        gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shader)));

    /* If main functions is at the bottom of shader source,
    ** we need to add a NOP to the end of main function.
    */
    if (Shader->type != gcSHADER_TYPE_CL)
    {
        _addInstNopToEndOfMainFunc(Shader);
    }

    if (LibType == gcLIB_BUILTIN ||
        LibType == gcLIB_CL_BUILTIN)
    {
        gcmASSERT(GetShaderHasIntrinsicBuiltin(Shader));
        for (i = 0; i < Shader->functionCount; i++)
        {
            gcFUNCTION function = Shader->functions[i];

            if (function == gcvNULL) {
                continue;
            }

            if (IsFunctionIntrinsicsBuiltIn(function))
            {
               gcmONERROR(gcSHADER_LinkLibFunction(Shader,
                                                   Library,
                                                   GetFunctionName(function),
                                                   &function));
               if (status == gcvSTATUS_TRUE)
               {
                   i--;
               }
            }
        }
    }
    else if (LibType == gcLIB_DX_BUILTIN)
    {
        typedef struct PATTERN
        {
            gctINT                  count;
            gcSL_OPCODE             opcode[2];
            check_builtin_func_prt  check_func_ptr;
            create_builtin_func_ptr create_func_ptr;
        } Pattern;

        gctUINT count = Shader->codeCount;

#define PATTERN_COUNT 2
        Pattern     pattern[PATTERN_COUNT] =
        {
            { 2, { gcSL_TEXGRAD, gcSL_TEXLD }, _checkTexGradBuiltinFunc, _createTexGradBuiltinFunc },
            { 2, { gcSL_TEXGRAD, gcSL_TEXLDPROJ }, _checkTexGradBuiltinFunc, _createTexGradBuiltinFunc }
        };

        gceSTATUS   status        = gcvSTATUS_OK;
        gcFUNCTION  stubFunction  = gcvNULL;
        gctUINT     i;

        for (i = 0; i < count; i++)
        {
            gctUINT j = 0;

            for (j = 0; j < PATTERN_COUNT; ++j)
            {
                gctINT   k = 0;
                Pattern  cur_pattern = pattern[j];
                gctBOOL  skip_pattern = gcvFALSE;
                gctUINT  lastInstruction;
                gcSL_INSTRUCTION code;
                gcSHADER_INSTRUCTION_INDEX instrIndex;

                for (k = 0; k < cur_pattern.count; ++k)
                {
                    gcSL_INSTRUCTION code       = &Shader->code[i + k];
                    gcSL_OPCODE      opcode     = gcmSL_OPCODE_GET(code->opcode, Opcode);

                    if (opcode != cur_pattern.opcode[k])
                    {
                        skip_pattern = gcvTRUE;
                        break;
                    }
                }

                if (skip_pattern)
                {
                    continue;
                }

                if (!cur_pattern.check_func_ptr(Shader, Library, i))
                {
                    continue;
                }

                gcmONERROR(cur_pattern.create_func_ptr(Shader, Library, i, &stubFunction));
                gcmASSERT(stubFunction != gcvNULL);

                for (k = 0; k < cur_pattern.count; ++k)
                {
                    gcSL_INSTRUCTION code       = &Shader->code[i + k];
                    gcSL_SetInst2NOP(code);
                }

                code = &Shader->code[i];
                gcSL_SetInst2NOP(code);
                lastInstruction = Shader->lastInstruction;
                instrIndex = Shader->instrIndex;
                Shader->lastInstruction = i;
                Shader->instrIndex = gcSHADER_OPCODE;
                _addCallInst(Shader, stubFunction);
                Shader->lastInstruction = lastInstruction;
                Shader->instrIndex = instrIndex;

                i += cur_pattern.count;

                break;
            }
        }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
        gcmASSERT(gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shader)));
        for (i = 0; i < Shader->outputCount; i++)
        {
            gcOUTPUT    output = Shader->outputs[i];
            gctINT      tempCodeIndex;
            gctINT      outputTempIndex = -1;
            gctUINT     lastInstruction;
            gcSHADER_INSTRUCTION_INDEX instrIndex;
            gcFUNCTION  convertFunction = gcvNULL;
            gcFUNCTION  stubFunction = gcvNULL;
            gcSL_INSTRUCTION  tempCode;
            gctBOOL      createdUniforms = gcvFALSE;
            gcUNIFORM    blend_enable_mode = gcvNULL;
            gcUNIFORM    blend_sampler = gcvNULL;
            gcUNIFORM    rt_height = gcvNULL;
            gcUNIFORM    rt_width = gcvNULL;

            if (output == gcvNULL) {
                continue;
            }

            if (gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(output->layoutQualifier))
            {
                for (j = 0; j < notSupportedBlendNum; j++)
                {
                    /* TO-Do to support OR (e.g., mode1 | mode2) cases */
                    if (output->layoutQualifier == notSupportedBlendModes[j])
                    {
                        outputTempIndex = output->tempIndex;

                        /* insert a NOP after the last define for output */
                        tempCodeIndex = _findLastDefine(Shader, outputTempIndex);

                        tempCodeIndex = _insertNOP2Shader(Shader, tempCodeIndex+1, 1);

                        _createBlendEquationFunc(Shader,
                                                  Library,
                                                  notSupportedBlendModeFunctions[j],
                                                  &convertFunction);

                        /* Construct call stub function. */
                        stubFunction = _createBlendStubFunction(
                                            Shader,
                                            convertFunction,
                                            tempCodeIndex,
                                            i,
                                            &createdUniforms,
                                            &blend_sampler,
                                            &rt_height,
                                            &rt_width,
                                            &blend_enable_mode);

                        /* Change the NOP instruciton to call stub */
                        tempCode = &Shader->code[tempCodeIndex];
                        if (gcmSL_OPCODE_GET(tempCode->opcode, Opcode) != gcSL_NOP)
                        {
                            gcmASSERT(gcvFALSE);
                        }

                        lastInstruction = Shader->lastInstruction;
                        instrIndex = Shader->instrIndex;
                        Shader->lastInstruction = tempCodeIndex;
                        Shader->instrIndex = gcSHADER_OPCODE;
                        _addCallInst(Shader, stubFunction);
                        Shader->lastInstruction = lastInstruction;
                        Shader->instrIndex = instrIndex;
                    }
                }
            }
        }
    }
    /* make sure the shader is ready to add new instruction */
    Shader->instrIndex = gcSHADER_OPCODE;

    gcSHADER_Pack(Shader);

OnError:
    /* Return the status. */
    return status;
}

gceSTATUS
gcSHADER_PatchCentroidVaryingAsCenter(
    IN OUT gcSHADER         Shader
    )
{
    gceSTATUS               status = gcvSTATUS_OK;
    gctUINT32               i;
    gctUINT16 *             attributeCheck = gcvNULL;
    gctINT *                attributeMapTempIndex = gcvNULL;
    gctPOINTER              pointer = gcvNULL;
    gcSL_INSTRUCTION        code;
    gcFUNCTION              convertFunc;
    gcFUNCTION              convertFuncList[] = {gcvNULL, gcvNULL, gcvNULL, gcvNULL};
    gctSTRING               convertFuncName;
    gctSTRING               convertFuncNameList[] =
    {
        "_viv_interpolateAtCentroid_float",
        "_viv_interpolateAtCentroid_vec2",
        "_viv_interpolateAtCentroid_vec3",
        "_viv_interpolateAtCentroid_vec4",
    };
    gctUINT8                enableList[] =
    {
        gcSL_ENABLE_X,
        gcSL_ENABLE_XY,
        gcSL_ENABLE_XYZ,
        gcSL_ENABLE_XYZW
    };
    gctUINT8                swizzleList[] =
    {
        gcSL_SWIZZLE_XXXX,
        gcSL_SWIZZLE_XYYY,
        gcSL_SWIZZLE_XYZZ,
        gcSL_SWIZZLE_XYZW
    };

    gcmASSERT(Shader->type == gcSHADER_TYPE_FRAGMENT);

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctUINT16) * Shader->attributeCount, &pointer));
    gcoOS_ZeroMemory(pointer, sizeof(gctUINT16) * Shader->attributeCount);
    attributeCheck = (gctUINT16 *)pointer;

    gcmONERROR(gcoOS_Allocate(gcvNULL, sizeof(gctINT) * Shader->attributeCount, &pointer));
    gcoOS_ZeroMemory(pointer, sizeof(gctINT) * Shader->attributeCount);
    attributeMapTempIndex = (gctINT *)pointer;

    /* Find the matched varyings, mark them as center and call function to convert them to centroid. */
    for (i = 0; i < Shader->attributeCount; i++)
    {
        gcATTRIBUTE         attribute = Shader->attributes[i];
        gctINT              funcIndex, codeIndex, j;
        gctUINT16           argumentTempIndex1, argumentTempIndex2, mapTempIndex;
        gctINT16            variableIndex1 = -1, variableIndex2 = -1;
        gctCHAR             name[128] = "";

        if (!attribute ||
            !gcmATTRIBUTE_isUseAsInterpolate(attribute) ||
            !gcmATTRIBUTE_isCentroid(attribute))
        {
            continue;
        }

        funcIndex = gcmType_Comonents(GetATTRType(attribute)) - 1;

        /* Find the matched convert function, add one if function is not existed. */
        convertFuncName = convertFuncNameList[funcIndex];
        convertFunc = convertFuncList[funcIndex];
        if (convertFunc == gcvNULL)
        {
            gcmONERROR(gcSHADER_GetFunctionByName(Shader, convertFuncName, &convertFunc));
            if (convertFunc == gcvNULL)
            {
                /* Add the function. */
                gcmONERROR(gcSHADER_AddFunction(Shader, convertFuncName, &convertFunc));
                /* Set the intrinsics kind. */
                SetFunctionIntrinsicsKind(convertFunc, gceINTRIN_MS_interpolate_at_centroid);
                /* Add the label. */
                gcmONERROR(gcSHADER_AddLabel(Shader, convertFunc->label));
                /* Add function arguments. */
                argumentTempIndex1 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, GetATTRType(attribute));
                argumentTempIndex2 = (gctUINT16)gcSHADER_NewTempRegs(Shader, 1, GetATTRType(attribute));
                gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                                  convertFuncName,
                                                  GetATTRType(attribute),
                                                  0,
                                                  gcvNULL,
                                                  argumentTempIndex1,
                                                  gcSHADER_VAR_CATEGORY_FUNCTION_INPUT_ARGUMENT,
                                                  gcSHADER_PRECISION_HIGH,
                                                  0,
                                                  -1,
                                                  -1,
                                                  &variableIndex1));
                gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                                  convertFuncName,
                                                  GetATTRType(attribute),
                                                  0,
                                                  gcvNULL,
                                                  argumentTempIndex2,
                                                  gcSHADER_VAR_CATEGORY_FUNCTION_OUTPUT_ARGUMENT,
                                                  gcSHADER_PRECISION_HIGH,
                                                  0,
                                                  -1,
                                                  -1,
                                                  &variableIndex2));
                gcmONERROR(gcFUNCTION_AddArgument(convertFunc,
                                                  (gctUINT16)variableIndex1,
                                                  argumentTempIndex1,
                                                  enableList[funcIndex],
                                                  gcvFUNCTION_INPUT,
                                                  gcSHADER_PRECISION_HIGH,
                                                  gcvFALSE));
                gcmONERROR(gcFUNCTION_AddArgument(convertFunc,
                                                  (gctUINT16)variableIndex2,
                                                  argumentTempIndex2,
                                                  enableList[funcIndex],
                                                  gcvFUNCTION_OUTPUT,
                                                  gcSHADER_PRECISION_HIGH,
                                                  gcvFALSE));
            }
            convertFuncList[funcIndex] = convertFunc;
        }
        /* Add a new variable to save the new varying value. */
        mapTempIndex = (gctUINT16)gcSHADER_NewTempRegs(Shader, GetATTRArraySize(attribute), GetATTRType(attribute));
        gcmONERROR(gcoOS_StrCopySafe(name, gcmSIZEOF(name), GetATTRName(attribute)));
        gcmONERROR(gcoOS_StrCatSafe(name, gcmSIZEOF(name), "_evaluated_as_centroid"));
        gcmONERROR(gcSHADER_AddVariableEx(Shader,
                                          name,
                                          GetATTRType(attribute),
                                          GetATTRIsArray(attribute) ? 1 : 0,
                                          GetATTRIsArray(attribute) ? &GetATTRArraySize(attribute) : gcvNULL,
                                          mapTempIndex,
                                          gcSHADER_VAR_CATEGORY_NORMAL,
                                          GetATTRPrecision(attribute),
                                          0,
                                          -1,
                                          -1,
                                          gcvNULL));
        /* Call function in the beginning of main function. */
        for (j = 0; j < GetATTRArraySize(attribute); j++)
        {
            codeIndex = _insertNOP2MainBegin(Shader, 3);
            /* Argument assignment. */
            code = &Shader->code[codeIndex];
            code->opcode = gcSL_MOV;
            code->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                       | gcmSL_TARGET_SET(0, Enable, enableList[funcIndex])
                       | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                       | gcmSL_TARGET_SET(0, Precision, GetATTRPrecision(attribute));
            code->tempIndex = convertFunc->arguments[0].index;
            code->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_ATTRIBUTE)
                          | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                          | gcmSL_SOURCE_SET(0, Swizzle, swizzleList[funcIndex])
                          | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                          | gcmSL_SOURCE_SET(0, Precision, GetATTRPrecision(attribute));
            code->source0Index = gcmSL_INDEX_SET(0, Index, GetATTRIndex(attribute))
                               | gcmSL_INDEX_SET(0, ConstValue, j);
            /* Call the function. */
            code = &Shader->code[codeIndex + 1];
            code->opcode = gcSL_CALL;
            code->temp = gcmSL_TARGET_SET(code->temp, Condition, gcSL_ALWAYS);
            code->tempIndex = (gctUINT16)convertFunc->label;
            /* Get the return value. */
            code = &Shader->code[codeIndex + 2];
            code->opcode = gcSL_MOV;
            code->temp = gcmSL_TARGET_SET(0, Format, gcSL_FLOAT)
                       | gcmSL_TARGET_SET(0, Enable, enableList[funcIndex])
                       | gcmSL_TARGET_SET(0, Indexed, gcSL_NOT_INDEXED)
                       | gcmSL_TARGET_SET(0, Precision, GetATTRPrecision(attribute));
            code->tempIndex = (gctUINT16)(mapTempIndex + j);
            code->source0 = gcmSL_SOURCE_SET(0, Type, gcSL_TEMP)
                          | gcmSL_SOURCE_SET(0, Format, gcSL_FLOAT)
                          | gcmSL_SOURCE_SET(0, Swizzle, swizzleList[funcIndex])
                          | gcmSL_SOURCE_SET(0, Indexed, gcSL_NOT_INDEXED)
                          | gcmSL_SOURCE_SET(0, Precision, GetATTRPrecision(attribute));
            code->source0Index = convertFunc->arguments[1].index;
        }

        /* Save the info. */
        attributeCheck[i] = 1;
        attributeMapTempIndex[i] = mapTempIndex;
        gcmATTRIBUTE_SetIsCentroid(attribute, gcvFALSE);
    }

    /* Replace the attribute index with the new temp register. */
    for (i = 0; i < Shader->codeCount; i++)
    {
        gcFUNCTION          function = gcvNULL;
        gctUINT16           attributeIndex;

        code = &Shader->code[i];

        if (!code) continue;

        /* Update the source0. */
        attributeIndex = gcmSL_INDEX_GET(code->source0Index, Index);
        if (gcmSL_SOURCE_GET(code->source0, Type) == gcSL_ATTRIBUTE &&
            attributeCheck[attributeIndex])
        {
            /* If this varying is used as the argument of a MS interpolation function, then skip. */
            if (gcmSL_OPCODE_GET(code->opcode, Opcode) == gcSL_MOV)
            {
                gcmONERROR(_findFunctionByArgumentIndex(Shader,
                                                        code->tempIndex,
                                                        &function));

                if (function != gcvNULL &&
                    IsIntrinsicsKindMSInterpolation(GetFunctionIntrinsicsKind(function)))
                {
                    continue;
                }
            }

            code->source0 = gcmSL_SOURCE_SET(code->source0, Type, gcSL_TEMP);
            code->source0Index = (gctUINT16)(attributeMapTempIndex[attributeIndex] + gcmSL_INDEX_GET(code->source0Index, ConstValue));
            code->source0Index = gcmSL_INDEX_SET(code->source0Index, ConstValue, 0);
        }

        /* Update the source1. */
        attributeIndex = gcmSL_INDEX_GET(code->source1Index, Index);
        if (gcmSL_SOURCE_GET(code->source1, Type) == gcSL_ATTRIBUTE &&
            attributeCheck[attributeIndex])
        {
            code->source1 = gcmSL_SOURCE_SET(code->source1, Type, gcSL_TEMP);
            code->source1Index = (gctUINT16)(attributeMapTempIndex[attributeIndex] + gcmSL_INDEX_GET(code->source1Index, ConstValue));
            code->source1Index = gcmSL_INDEX_SET(code->source1Index, ConstValue, 0);
        }
    }

    /* Pach the shader. */
    gcSHADER_Pack(Shader);

OnError:
    if (attributeCheck != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, attributeCheck));
    }
    if (attributeMapTempIndex != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, attributeMapTempIndex));
    }
    return status;
}

#endif /*gcdENABLE_3D*/

